#pragma once
#include "DefendTactic.h"
#include "InformationManager.h"


void ArmyDefendTactic::update()
{
	newArmyRally();
	
	//get all adjacent regions
	std::set<BWTA::Region *> myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	std::set<BWTA::Region*> reachRegions;

	//if defend place have been destroyed, end tactic
	if (myRegion.find(BWTA::getRegion(attackPosition)) == myRegion.end())
	{
		state = END;
		return;
	}

	bool hasDetector = false;
	std::set<BWAPI::Unit> enemyUnitsInRegion;
	int enemyInRegionSupply = 0;
	int enemyAntiAirSupply = 0;

	if (BWTA::getRegion(attackPosition) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
		|| BWTA::getRegion(attackPosition) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
	{
		for (auto enemyUnit : BWAPI::Broodwar->enemy()->getUnits())
		{
			if (BWTA::getRegion(enemyUnit->getPosition()) == BWTA::getRegion(attackPosition))
			{
				enemyUnitsInRegion.insert(enemyUnit);
				if (enemyUnit->getType() == BWAPI::UnitTypes::Zerg_Overlord
					|| enemyUnit->getType() == BWAPI::UnitTypes::Terran_Science_Vessel
					|| enemyUnit->getType() == BWAPI::UnitTypes::Protoss_Observer
					|| enemyUnit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
					|| enemyUnit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
					|| enemyUnit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
				{
					hasDetector = true;
				}
			}
		}

		// add enemy at region choke
		BWAPI::Unitset enemySet;
		if (BWTA::getRegion(attackPosition) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
		{
			BWAPI::Position baseChokePosition = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
			enemySet = BWAPI::Broodwar->getUnitsInRadius(baseChokePosition, 12 * 32, BWAPI::Filter::IsEnemy);
			enemyUnitsInRegion.insert(enemySet.begin(), enemySet.end());
		}
		else
		{
			enemySet = BWAPI::Broodwar->getUnitsInRadius(naturalChokePosition, 16 * 32, BWAPI::Filter::IsEnemy);
			enemyUnitsInRegion.insert(enemySet.begin(), enemySet.end());
		}
	}
	else
	{
		BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 16 * 32, BWAPI::Filter::IsEnemy);
		enemyUnitsInRegion.insert(enemySet.begin(), enemySet.end());
	}

	std::set<BWAPI::Unit> nearByEnemy;
	int count = 0;
	int checkMod = 3;
	for (auto army : tacticArmy)
	{
		for (auto u : army.second->getUnits())
		{
			if (army.first == BWAPI::UnitTypes::Zerg_Zergling)
			{
				count += 1;
				if (count % checkMod == 0)
				{
					BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
					nearByEnemy.insert(percentUnits.begin(), percentUnits.end());
				}
			}
			else
			{
				BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
				nearByEnemy.insert(percentUnits.begin(), percentUnits.end());
			}
		}
	}

	
	double2 enemyAvgPosition;
	for (std::set<BWAPI::Unit>::iterator it = enemyUnitsInRegion.begin(); it != enemyUnitsInRegion.end();)
	{
		enemyAvgPosition = enemyAvgPosition + (*it)->getPosition();
		enemyInRegionSupply += (*it)->getType().supplyRequired();
		if ((*it)->getType().airWeapon() != BWAPI::WeaponTypes::None)
		{
			enemyAntiAirSupply += (*it)->getType().supplyRequired();
		}
		it++;
	}
	enemyAvgPosition = enemyAvgPosition / enemyUnitsInRegion.size();

	// if enemy has been eliminated or retreat from our defend adjacent regions, defend tactic is over
	if (enemyUnitsInRegion.size() == 0)
	{
		state = END;
		return;
	}

	if (enemyInRegionSupply * armyForceMultiply > InformationManager::Instance().getOurTotalBattleForce()
		&& BWTA::getRegion(attackPosition) != BWTA::getRegion(InformationManager::Instance().getOurBaseLocation())
		&& BWTA::getRegion(attackPosition) != BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation()))
	{
		state = END;
		return;
	}

	std::set<BWAPI::Unit> totalEnemy;
	totalEnemy.insert(enemyUnitsInRegion.begin(), enemyUnitsInRegion.end());
	totalEnemy.insert(nearByEnemy.begin(), nearByEnemy.end());

	reachRegions.insert(BWTA::getRegion(attackPosition));

	int myArmySupply = 0;
	int notAtRegionsSupply = 0;
	int myFlySupply = 0;
	double2 avgPosition;
	int armyCount = 0;
	for (std::map<BWAPI::UnitType, BattleArmy*>::iterator it = tacticArmy.begin(); it != tacticArmy.end(); it++)
	{
		if (it->second->getUnits().size() > 0)
		{
			myArmySupply += it->first.supplyRequired() * it->second->getUnits().size();
			if (it->first == BWAPI::UnitTypes::Zerg_Mutalisk)
			{
				myFlySupply += it->first.supplyRequired() * it->second->getUnits().size();
			}
			BOOST_FOREACH(UnitState u, it->second->getUnits())
			{
				avgPosition = avgPosition + u.unit->getPosition();
				armyCount += 1;
				if (reachRegions.find(BWTA::getRegion(u.unit->getPosition())) == reachRegions.end())
				{
					notAtRegionsSupply += u.unit->getType().supplyProvided();
				}
			}
		}
	}

	avgPosition = avgPosition / armyCount;

	if (myArmySupply == 0)
	{
		state = END;
		return;
	}

	//get unit near our sunken defend and unit in range to decide when to attack
	std::set<BWAPI::Unit> sunkens;
	std::set<BWAPI::Unit> sunkenNearbyEnemy;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	int sunkunRange = BWAPI::UnitTypes::Zerg_Sunken_Colony.groundWeapon().maxRange();
	int minDistance = 99999;
	BWAPI::Unit minDistanceSunker = NULL;
	BOOST_FOREACH(BWAPI::Unit sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
	{
		if (reachRegions.find(BWTA::getRegion(sunker->getPosition())) != reachRegions.end()) //(BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion(attackPosition))
		{
			sunkens.insert(sunker);
			if (!minDistanceSunker || sunker->getDistance(attackPosition) < minDistance)
			{
				minDistanceSunker = sunker;
				minDistance = sunker->getDistance(attackPosition);
			}

			BWAPI::Unitset tmp = sunker->getUnitsInRadius(sunkunRange, BWAPI::Filter::IsEnemy);
			sunkenNearbyEnemy.insert(tmp.begin(), tmp.end());
		}
	}


	if (minDistanceSunker != NULL)
	{
		groupPosition = minDistanceSunker->getPosition();
	}
	else
	{
		groupPosition = attackPosition;
	}

	bool isUnderAttack = false;
	std::set<BWAPI::Unit> nearbyUnits;
	for (auto army : tacticArmy)
	{
		for (auto u : army.second->getUnits())
		{
			count += 1;
			if (count % checkMod == 0)
			{
				BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
				nearbyUnits.insert(percentUnits.begin(), percentUnits.end());
			}

			if (u.unit->isUnderAttack())
			{
				isUnderAttack = true;
			}
		}
	}
	BWAPI::Position target = (*enemyUnitsInRegion.begin())->getPosition();

	switch (state)
	{
	case BASEGROUP:
	{
		//our army is outside
		if ((enemyAvgPosition - attackPosition).len() < (avgPosition - attackPosition).len())
		{
			state = GROUPARMY;
			break;
		}

		if ((sunkens.size() > 0 && sunkenNearbyEnemy.size() > 0) || sunkens.size() == 0
			|| myFlySupply * 0.5 > enemyAntiAirSupply
			|| myArmySupply * 1.2 > enemyInRegionSupply
			|| (hasDetector == false && tacticArmy[BWAPI::UnitTypes::Zerg_Lurker]->getUnits().size() > 0))
		{
			state = BASEATTACK;
			break;
		}

		for (auto a : tacticArmy)
		{
			a.second->armyMove(groupPosition);
		}

	}
	break;

	case BASEATTACK:
	{
		if (sunkens.size() > 0 && sunkenNearbyEnemy.size() == 0 && myFlySupply * 0.5 <= enemyAntiAirSupply
			&& myArmySupply * 1.2 <= enemyInRegionSupply
			&& (hasDetector || tacticArmy[BWAPI::UnitTypes::Zerg_Lurker]->getUnits().size() == 0))
		{
			state = BASEGROUP;
			break;
		}

		armyAttack(target, totalEnemy);
	}
	break;

	case GROUPARMY:
	{
		if (isUnderAttack || nearbyUnits.size() > 0)
		{
			state = ATTACK;
			nextGroupTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
			break;
		}

		BWAPI::Broodwar->drawCircleMap(groupPosition, 10, BWAPI::Colors::Blue, true);

		togetherMove(groupPosition);
	}
	break;	

	case ATTACK:
	{
		if (nearbyUnits.size() == 0 && !isUnderAttack && BWAPI::Broodwar->getFrameCount() > nextGroupTime)
		{
			state = GROUPARMY;
			break;
		}

		BWAPI::Broodwar->drawCircleMap(attackPosition, 10, BWAPI::Colors::Blue, true);
		armyAttack(attackPosition, nearbyUnits);
	}
	break;
	
	}
}


bool ArmyDefendTactic::isTacticEnd()
{
	if (state == END)
		return true;
	else
		return false;
}


ArmyDefendTactic::ArmyDefendTactic()
{
	state = GROUPARMY;
	retreatJustifyTime = 0;
	
	BWTA::Region* nextBase = BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation());
	double maxDist = 0;
	BWTA::Chokepoint* maxChoke = nullptr;
	BOOST_FOREACH(BWTA::Chokepoint* p, nextBase->getChokepoints())
	{
		if (BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()).getDistance(p->getCenter()) > maxDist)
		{
			maxDist = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()).getDistance(p->getCenter());
			naturalChokePosition = p->getCenter();
			maxChoke = p;
		}
	}
}

void ArmyDefendTactic::generateAttackPath()
{

}