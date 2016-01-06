#pragma once
#include "DefendTactic.h"



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

	if (BWTA::getRegion(attackPosition) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
		|| BWTA::getRegion(attackPosition) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
	{
		const std::set<BWTA::Chokepoint*>& chokes = BWTA::getRegion(attackPosition)->getChokepoints();
		BOOST_FOREACH(const BWTA::Chokepoint* c, chokes)
		{
			reachRegions.insert(c->getRegions().first);
			reachRegions.insert(c->getRegions().second);
		}
		for (std::set<BWTA::Region*>::iterator it = reachRegions.begin(); it != reachRegions.end();)
		{
			if (myRegion.find(*it) != myRegion.end())
			{
				reachRegions.erase(it++);
			}
			else
				it++;
		}
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

	//get unit in defend region
	std::set<EnemyUnit> enemyUnitsInRegion;
	int enemyInRegionSupply = 0;
	int enemyAntiAirSupply = 0;
	BOOST_FOREACH(BWAPI::Unit enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (reachRegions.find(BWTA::getRegion(enemyUnit->getPosition())) != reachRegions.end()) //(BWTA::getRegion(enemyUnit->getPosition()) == BWTA::getRegion(attackPosition))
		{
			if (enemyUnit->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;

			enemyUnitsInRegion.insert(EnemyUnit(enemyUnit, 10, enemyUnit->getDistance(attackPosition)));
			enemyInRegionSupply += enemyUnit->getType().supplyRequired();
			if (enemyUnit->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				enemyAntiAirSupply += enemyUnit->getType().supplyRequired();
			}
		}
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

	BWAPI::Unitset unitsInCircle;
	BWAPI::Unitset enemyInCircle;
	//if has sunken, only care about enemy around base
	if (sunkens.size() > 0)
	{
		unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 8 * 32);
		BWAPI::Broodwar->drawCircleMap(attackPosition.x, attackPosition.y, 6 * 32, BWAPI::Colors::Green, false);
	}
	else
	{
		unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 12 * 32);
		BWAPI::Broodwar->drawCircleMap(attackPosition.x, attackPosition.y, 12 * 32, BWAPI::Colors::Green, false);
	}
		
	

	BOOST_FOREACH(BWAPI::Unit enemy, unitsInCircle)
	{
		if (enemy->getPlayer() == BWAPI::Broodwar->enemy() && reachRegions.find(BWTA::getRegion(enemy->getPosition())) != reachRegions.end())
		{
			if (enemy->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;
			enemyInCircle.insert(enemy);
		}
	}

	// if enemy has been eliminated or retreat from our defend adjacent regions, defend tactic is over
	if (enemyUnitsInRegion.size() == 0)
	{
		state = END;
	}

	//group at defend region first
	BWAPI::Position groupPosition;
	if (sunkens.size() > 0)
	{
		groupPosition = minDistanceSunker->getPosition();
	}
	else
	{
		groupPosition = attackPosition;
	}

	if (notAtRegionsSupply / double(myArmySupply) >= 0.1)
	{
		int a = 1;
	}

	switch (state)
	{
	case GROUPARMY:
	{
		// if enemy near sunken || near base || our army outweigh enemy || some part of our army is outsides defend regions || enemy has less anti-air unit, start to attack
		if (sunkenNearbyEnemy.size() > 0 || enemyInCircle.size() > 0
			|| myArmySupply * 0.5 >= enemyInRegionSupply || notAtRegionsSupply / double(myArmySupply) >= 0.1 || (myFlySupply > 0 && myFlySupply * 0.6 >= enemyAntiAirSupply))
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
		if (sunkenNearbyEnemy.size() == 0 && enemyInCircle.size() == 0 
			&& myArmySupply * 0.5 < enemyInRegionSupply && (myFlySupply == 0 || myFlySupply * 0.6 < enemyAntiAirSupply) && notAtRegionsSupply / double(myArmySupply) < 0.1)
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
				if (myFlySupply > 0 && myFlySupply * 0.6 >= enemyAntiAirSupply && myArmySupply * 0.5 < enemyInRegionSupply
					&& sunkenNearbyEnemy.size() == 0 && enemyInCircle.size() == 0
					&& it->first != BWAPI::UnitTypes::Zerg_Mutalisk)
				{
					it->second->armyMove(groupPosition);
					continue;
				}
				std::set<EnemyUnit>::iterator unitIt = enemyUnitsInRegion.end();
				it->second->defend((*(--unitIt)).unit->getPosition());
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
}

void ArmyDefendTactic::generateAttackPath()
{

}