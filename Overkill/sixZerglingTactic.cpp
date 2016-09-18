#pragma once
#include "sixZerglingTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"



void sixZerglingTactic::update()
{
	ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]);
	if (zerglings->getUnits().size() == 0)
	{
		state = END;
		return;
	}

	/*
	if ((*BWTA::getRegion(attackPosition)->getBaseLocations().begin())->isIsland())
	{
		state = END;
		return;
	}*/

	TimerManager::Instance().startTimer(TimerManager::ZerglingTac);

	int minTargetDistance = 99999;
	BWAPI::Unit firstZergling = NULL;
	for (auto u : zerglings->getUnits())
	{
		if (u.unit->getDistance(attackPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(attackPosition);
			firstZergling = u.unit;
		}
	}


	if (BWAPI::Broodwar->getFrameCount() % 25 * 5 == 0)
	{
		newAddMovePositions.clear();
		newAddMovePositions.push_back(firstZergling->getPosition());
		for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
		{
			it->second = newAddMovePositions;
		}
	}


	newArmyRally();

	nearbyUnits.clear();
	nearbySunkens.clear();
	friendUnitNearBy.clear();
	//BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits())
	//{

	BWAPI::Unitset tmp = firstZergling->getUnitsInRadius(12 * 32);
	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (unit->getType().isFlyer())
				continue;

			nearbyUnits.insert(unit);
		}

		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Zergling && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}
	//}

	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	BWAPI::Position moveBackBase;
	if (myRegions.size() > 1)
		moveBackBase = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		moveBackBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	std::set<BWAPI::Unit> sunkenNearbyEnemy;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	int sunkunRange = BWAPI::UnitTypes::Zerg_Sunken_Colony.groundWeapon().maxRange();
	int minDistance = 99999;
	BWAPI::Unit minDistanceSunker = NULL;
	BOOST_FOREACH(BWAPI::Unit sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
	{
		if (BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion(moveBackBase)) //(BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion(attackPosition))
		{
			if (!minDistanceSunker || sunker->getDistance(moveBackBase) < minDistance)
			{
				minDistanceSunker = sunker;
				minDistance = sunker->getDistance(moveBackBase);
			}
		}
	}

	if (minDistanceSunker != NULL)
	{
		BWAPI::Unitset tmp = minDistanceSunker->getUnitsInRadius(sunkunRange);
		BWAPI::Broodwar->drawCircleMap(minDistanceSunker->getPosition().x, minDistanceSunker->getPosition().y, 8 * 32, BWAPI::Colors::Blue, false);

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
	if (minDistanceSunker != NULL)
	{
		unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(moveBackBase, 8 * 32);
		BWAPI::Broodwar->drawCircleMap(moveBackBase.x, moveBackBase.y, 6 * 32, BWAPI::Colors::Blue, false);
	}
	else
	{
		unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(moveBackBase, 12 * 32);
		BWAPI::Broodwar->drawCircleMap(moveBackBase.x, moveBackBase.y, 12 * 32, BWAPI::Colors::Blue, false);
	}
	std::set<BWAPI::Unit> enemyInCircle;
	BOOST_FOREACH(BWAPI::Unit enemy, unitsInCircle)
	{
		if (enemy->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (enemy->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;

			enemyInCircle.insert(enemy);
		}
	}

	bool isUnderAttack = false;
	BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits())
	{
		if (u.unit->isUnderAttack())
		{
			isUnderAttack = true;
			break;
		}
	}

	switch (state)
	{
	case GROUPARMY:
	{
		BWAPI::Broodwar->drawCircleMap(groupPosition.x, groupPosition.y, 16, BWAPI::Colors::Red, true);

		groupStatus = checkGroupStatus(groupPosition);

		if (isUnderAttack || nearbyUnits.size() > 0 || groupStatus == true || groupPosition == attackPosition)
		{
			state = ATTACK;
			nextGroupTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
			break;
		}
		
		zerglings->armyMove(groupPosition);
	}
	break;

	case ATTACK:
	{
		//if don't have enemy nearby and attack position is occupied by enemy, and we have not grouped army, change to group status
		if (nearbyUnits.size() == 0 && !isUnderAttack && groupStatus == false && groupPosition != attackPosition && BWAPI::Broodwar->getFrameCount() > nextGroupTime)
		{
			state = GROUPARMY;
			break;
		}

		if (!hasEnemy())
		{
			state = END;
			break;
		}

		//if enemy evade our base, do not retreat
		if (needRetreat() && enemyInCircle.size() == 0 && sunkenNearbyEnemy.size() == 0)
		{

			state = RETREAT;
			
			//at early game stage do not change attack position
			if (BWAPI::Broodwar->getFrameCount() > 12000)
			{
				retreatCount++;
				if (retreatCount >= 5)
				{
					state = END;
					break;
				}
			}

			retreatTime = BWAPI::Broodwar->getFrameCount() + 3 * 25;
			bool needGroup = false;
			BWAPI::Position tmpRetreat = groupArmyCheck(needGroup);

			if (needGroup = false || firstZergling->getDistance(tmpRetreat) < 32 * 12)
			{
				if (myRegions.size() > 1)
					nextRetreatPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
				else
					nextRetreatPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
			}
			else
			{
				nextRetreatPosition = tmpRetreat;
			}

			break;
		}

		zerglings->harassAttack(attackPosition);
	}
	break;

	case RETREAT:
	{
		//BWAPI::Broodwar->drawCircleMap(nextRetreatPosition.x, nextRetreatPosition.y, 8, BWAPI::Colors::Red, true);
		
		// if no enemy nearby, wait to attack again
		if (nearbyUnits.size() == 0 && !isUnderAttack)
		{
			state = WAIT;
			nextAttackTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
		}

		zerglings->armyMove(nextRetreatPosition);

		//if we reach the retreat position and still under attack, change to attack mode to choose the next action
		if ((BWAPI::Broodwar->getFrameCount() >= retreatTime) && (isUnderAttack || nearbyUnits.size() > 0))
		{
			state = ATTACK;
			break;
		}

		break;
	}

	case WAIT:
	{
		if (nearbyUnits.size() == 0)
		{
			// do regroup 
			zerglings->armyMove(firstZergling->getPosition());

			// no enemy nearby, and can attack the front cannon, wait some seconds to attack again 
			if (BWAPI::Broodwar->getFrameCount() > nextAttackTime)
				state = ATTACK;
		}
		// has nearby enemy
		else
		{
			state = ATTACK;
		}

		/*
		if (zerglings->getUnits().size() >= 6 || nearbyUnits.size() > 0 || BWAPI::Broodwar->getFrameCount() > nextAttackTime)
		{
		double2 centerPoint(firstZergling->getTilePosition().x, firstZergling->getTilePosition().y);
		int maxIM = 0;
		for (int x = centerPoint.x - 8 < 0 ? 0 : centerPoint.x - 8; x <= (centerPoint.x + 8 > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1: centerPoint.x + 8); x++)
		{
		for (int y = centerPoint.y - 8 < 0 ? 0 : centerPoint.y - 8; y <= (centerPoint.y + 8 > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : centerPoint.y + 8); y++)
		{
		if (enemyIm[x][y].groundForce > maxIM)
		{
		maxIM = enemyIm[x][y].groundForce;
		}
		}
		}
		// approximate 10 zergling can take down one cannon
		if (zerglings->getUnits().size() >= maxIM / 2)
		{
		state = ATTACK;
		}
		}*/
		break;
	}
	}

	TimerManager::Instance().stopTimer(TimerManager::ZerglingTac);
}



bool sixZerglingTactic::isTacticEnd()
{
	if (state == END)
		return true;
	else
		return false;
}


sixZerglingTactic::sixZerglingTactic()
{
	state = GROUPARMY;
	retreatCount = 0;
}


bool sixZerglingTactic::needRetreat()
{
	int ourArmySupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size());
	BOOST_FOREACH(BWAPI::Unit u, friendUnitNearBy)
	{
		ourArmySupply += u->getType().supplyRequired();
	}

	BWAPI::Unit firstZergling = (*tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().begin()).unit;

	std::vector<std::vector<gridInfo>>& enemyIm = InformationManager::Instance().getEnemyInfluenceMap();
	double2 centerPoint(firstZergling->getTilePosition().x, firstZergling->getTilePosition().y);
	int maxIM = 0;
	for (int x = int(centerPoint.x - 8 < 0 ? 0 : centerPoint.x - 8); x <= int(centerPoint.x + 8 > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : centerPoint.x + 8); x++)
	{
		for (int y = int(centerPoint.y - 8 < 0 ? 0 : centerPoint.y - 8); y <= int(centerPoint.y + 8 > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : centerPoint.y + 8); y++)
		{
			if (enemyIm[x][y].groundForce > maxIM)
			{
				maxIM = int(enemyIm[x][y].groundForce);
			}
		}
	}
	// approximate 4 zerglings can take down one cannon
	if (ourArmySupply <= maxIM / 5)
	{
		return true;
	}

	int enemySupply = 0;
	BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
	{
		if (u->getType().isFlyer())
		{
			continue;
		}

		if (u->getType().isBuilding() && u->isCompleted() && (u->getType().groundWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			if (u->getType() == BWAPI::UnitTypes::Terran_Bunker)
				enemySupply += 6;
			else
				enemySupply += 6;
		}
		else if (u->getType().groundWeapon() != BWAPI::WeaponTypes::None)
		{
			if (u->getType().isWorker() && !u->isAttacking())
				continue;

			if (u->getType().isWorker())
				enemySupply += int(u->getType().supplyRequired() * 0.5);
			else
				enemySupply += u->getType().supplyRequired();
		}
		else
			continue;
	}

	if (ourArmySupply * 0.8 < enemySupply)
		return true;
	else
		return false;
}