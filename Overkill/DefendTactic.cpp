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

	std::set<EnemyUnit> enemyUnitsInRegion;
	int enemyInRegionSupply = 0;
	int enemyAntiAirSupply = 0;

	if (BWTA::getRegion(attackPosition) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
		|| BWTA::getRegion(attackPosition) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
	{
		for (auto enemyUnit : BWAPI::Broodwar->enemy()->getUnits())
		{
			if (BWTA::getRegion(enemyUnit->getPosition()) == BWTA::getRegion(attackPosition))
			{
				enemyUnitsInRegion.insert(EnemyUnit(enemyUnit, 10, enemyUnit->getDistance(attackPosition)));
			}
		}

		BWAPI::Unitset enemySet;
		if (BWTA::getRegion(attackPosition) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
		{
			BWAPI::Position baseChokePosition = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
			enemySet = BWAPI::Broodwar->getUnitsInRadius(baseChokePosition, 12 * 32, BWAPI::Filter::IsEnemy);
			for (auto e : enemySet)
			{
				enemyUnitsInRegion.insert(EnemyUnit(e, 10, e->getDistance(attackPosition)));
			}
		}
		else
		{
			enemySet = BWAPI::Broodwar->getUnitsInRadius(naturalChokePosition, 12 * 32, BWAPI::Filter::IsEnemy);
			for (auto e : enemySet)
			{
				enemyUnitsInRegion.insert(EnemyUnit(e, 10, e->getDistance(attackPosition)));
			}
		}
	}
	else
	{
		BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 16 * 32, BWAPI::Filter::IsEnemy);
		for (auto enemyUnit : enemySet)
		{
			enemyUnitsInRegion.insert(EnemyUnit(enemyUnit, 10, enemyUnit->getDistance(attackPosition)));
		}
	}
	
	for (std::set<EnemyUnit>::iterator it = enemyUnitsInRegion.begin(); it != enemyUnitsInRegion.end();)
	{
		if (!(*it).unit->isDetected() && (*it).unit->isVisible())
		{
			enemyUnitsInRegion.erase(it++);
		}
		else if (tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0 
			&& tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
			&& (*it).unit->getType().isFlyer())
		{
			enemyUnitsInRegion.erase(it++);
		}
		else
		{
			enemyInRegionSupply += (*it).unit->getType().supplyRequired();
			if ((*it).unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				enemyAntiAirSupply += (*it).unit->getType().supplyRequired();
			}
			it++;
		}
		
	}

	// if enemy has been eliminated or retreat from our defend adjacent regions, defend tactic is over
	if (enemyUnitsInRegion.size() == 0)
	{
		state = END;
	}

	reachRegions.insert(BWTA::getRegion(attackPosition));

	int myArmySupply = 0;
	int notAtRegionsSupply = 0;
	int myFlySupply = 0;
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
				if (reachRegions.find(BWTA::getRegion(u.unit->getPosition())) == reachRegions.end())
				{
					notAtRegionsSupply += u.unit->getType().supplyProvided();
				}
			}
		}
	}

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
		}
	}

	if (minDistanceSunker != NULL)
	{
		BWAPI::Unitset tmp = minDistanceSunker->getUnitsInRadius(sunkunRange);
		BWAPI::Broodwar->drawCircleMap(minDistanceSunker->getPosition().x, minDistanceSunker->getPosition().y, 8 * 32, BWAPI::Colors::Green, false);

		BOOST_FOREACH(BWAPI::Unit enemy, tmp)
		{
			if (enemy->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;

			if (enemy->getPlayer() == BWAPI::Broodwar->enemy())
				sunkenNearbyEnemy.insert(enemy);
		}
	}



	groupPosition = attackPosition;

	switch (state)
	{
	case GROUPARMY:
	{
		// if enemy near sunken || near base || our army outweigh enemy || some part of our army is outsides defend regions || enemy has less anti-air unit, start to attack
		if ((sunkens.size() > 0 && sunkenNearbyEnemy.size() > 0) || sunkens.size() == 0
			||  myFlySupply * 0.5 >= enemyAntiAirSupply)
		{
			state = ATTACK;
			break;
		}

		for (std::map<BWAPI::UnitType, BattleArmy*>::iterator it = tacticArmy.begin(); it != tacticArmy.end(); it++)
		{
			if (it->second->getUnits().size() > 0)
			{
				it->second->armyMove(groupPosition);
			}
		}
	}
	break;	

	case ATTACK:
	{
		// only none of the groupby -> attack condition match, return to groupby state, otherwise, it may return to attack again
		if (sunkens.size() > 0 && sunkenNearbyEnemy.size() == 0 && myFlySupply * 0.5 < enemyAntiAirSupply)
		{
			state = GROUPARMY;
			break;
		}

		// attack the unit at defend region in priority descending order
		for (std::map<BWAPI::UnitType, BattleArmy*>::iterator it = tacticArmy.begin(); it != tacticArmy.end(); it++)
		{
			if (it->second->getUnits().size() > 0)
			{
				
				// if enemy do not have much anti-air unit,only send mutalisks to attack
				if ((sunkens.size() > 0 && sunkenNearbyEnemy.size() == 0) &&
					myFlySupply * 0.5 >= enemyAntiAirSupply && 
					myArmySupply * 0.8 < enemyInRegionSupply && it->first != BWAPI::UnitTypes::Zerg_Mutalisk)
				{
					it->second->armyMove(groupPosition);
					continue;
				}
				std::set<EnemyUnit>::iterator unitIt = enemyUnitsInRegion.end();
				//it->second->defend((*(--unitIt)).unit->getPosition());
				BWAPI::Position p = (*(--unitIt)).unit->getPosition();
				it->second->defend(p);
			}
		}
	}
	break;
	/*
	case MOVE:
	{
		if (enemyInRegionSupply > 0)
		{
			state = ATTACK;
			break;
		}

		typedef std::pair<BWAPI::UnitType, BattleArmy*> mapType;
		bool isAllInRegion = true;
		BOOST_FOREACH(mapType p, tacticArmy)
		{
			if (p.second->getUnits().size() == 0)
				continue;
			p.second->armyMove(attackPosition);
			BOOST_FOREACH(UnitState u, p.second->getUnits())
			{
				if (BWTA::getRegion(u.unit->getPosition()) != BWTA::getRegion(attackPosition))
				{
					isAllInRegion = false;
					break;
				}
			}
		}

		//wait some seconds to check enemy is not exist
		if (isAllInRegion)
		{
			//state = END;
			retreatJustifyTime = BWAPI::Broodwar->getFrameCount() + 1 * 30;
			state = WAIT;
		}
	}
	break;

	case WAIT:
	{
		if (enemyInRegionSupply > 0)
		{
			state = ATTACK;
		}

		if (BWAPI::Broodwar->getFrameCount() > retreatJustifyTime && enemyInRegionSupply == 0)
		{
			state = END;
		}
	}
	break;*/
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