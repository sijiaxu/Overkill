#pragma once
#include "HydraliskTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"


//TODO: put in the battle tactic class
bool HydraliskTactic::needRetreat(BWAPI::Unit firstUnit)
{
	/*
	if (ourArmySupply >= 36 && BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Protoss)
	{
		return false;
	}*/

	BOOST_FOREACH(BWAPI::Unit u, friendUnitNearBy)
	{
		ourArmySupply += u->getType().supplyRequired();
	}

	if (ourArmySupply <= 36)
	{
		ourArmySupply = int(ourArmySupply * 0.7);
	}
	else if (ourArmySupply > 36 && ourArmySupply <= 60)
	{
		ourArmySupply = int(ourArmySupply * 0.9);
	}
	else if (ourArmySupply > 60 && ourArmySupply <= 100)
	{
		ourArmySupply = int(ourArmySupply * 1.1);
	}
	else
	{
		ourArmySupply = int(ourArmySupply * 1.3);
	}


	std::vector<std::vector<gridInfo>>& enemyIm = InformationManager::Instance().getEnemyInfluenceMap();
	double2 centerPoint(firstUnit->getTilePosition().x, firstUnit->getTilePosition().y);
	int maxIM = 0;
	for (int x = int(centerPoint.x - 16 < 0 ? 0 : centerPoint.x - 16); x <= int(centerPoint.x + 16 > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : centerPoint.x + 16); x++)
	{
		for (int y = int(centerPoint.y - 16 < 0 ? 0 : centerPoint.y - 16); y <= int(centerPoint.y + 16 > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : centerPoint.y + 16); y++)
		{
			if (enemyIm[x][y].groundForce > maxIM)
			{
				maxIM = int(enemyIm[x][y].groundForce);
			}
		}
	}

	if (ourArmySupply <= maxIM / 4)
	{
		return true;
	}

	double enemySupply = 0;
	BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
	{
		if (u->getType().isBuilding() && u->isCompleted() && (u->getType().groundWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			if (u->getType() == BWAPI::UnitTypes::Terran_Bunker)
				enemySupply += 8;
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
				enemySupply += u->getType().supplyRequired() == 0 ? 0.5 : u->getType().supplyRequired();
		}
		else
			continue;
	}

	//BWAPI::Broodwar->printf("our army: %d, enemy:%d", ourArmySupply, enemySupply);

	if (ourArmySupply < enemySupply)
		return true;
	else
		return false;
}


void HydraliskTactic::update()
{

	HydraliskArmy* hydralisks = dynamic_cast<HydraliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]);
	OverLordArmy* overlords = dynamic_cast<OverLordArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]);
	ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]);

	ourArmyCount = 0;
	BWAPI::Unit armyUnit = NULL;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits() )
			{
				ourArmyCount += 1;
				armyUnit = u.unit;

				if (u.unit->isIrradiated())
				{
					u.state = Irradiated;
				}
			}
		}
	}

	if (ourArmyCount == 0)
	{
		state = END;
		return;
	}

	ourArmySupply = 0;

	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			ourArmySupply += army.second->getUnits().size() * army.first.supplyRequired();
		}
	}

	std::set<BWTA::Region *> & enemyRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
	BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
	BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();
	if ((BWTA::getRegion(attackPosition) == BWTA::getRegion(enemyBase) || BWTA::getRegion(attackPosition) == BWTA::getRegion(enemyNatural)) && enemyRegions.find(BWTA::getRegion(attackPosition)) != enemyRegions.end())
	{
		int enemySupply = InformationManager::Instance().getEnemyGroundBattleUnitSupply();
		if (ourArmySupply * 0.8 < enemySupply && ourArmySupply < 120)
		{
			BWAPI::Broodwar->printf("supply less then enemy, do not attack base!");
			state = END;
			return;
		}
	}

	/*
	std::list<BWAPI::TilePosition> groundPath = aStarGroundPathFinding(armyUnit->getTilePosition(), BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()));
	BWAPI::Position frontChoke = BWAPI::Position(groundPath.front());

	BWAPI::Unit firstUnit = NULL;
	int minTargetDistance = 99999;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (u.unit->getDistance(frontChoke) < minTargetDistance)
				{
					minTargetDistance = u.unit->getDistance(frontChoke);
					firstUnit = u.unit;
				}
			}
		}
	}
	BWAPI::Broodwar->drawCircleMap(firstUnit->getPosition().x, firstUnit->getPosition().y, 8, BWAPI::Colors::Black, true);
	BWAPI::Broodwar->drawCircleMap(frontChoke.x, frontChoke.y, 16, BWAPI::Colors::Blue, true);
	*/

	BWAPI::Unit firstUnit = NULL;
	if (zerglings->getUnits().size() > 0)
	{
		firstUnit = zerglings->getUnits().front().unit;
	}
	else 
	{
		firstUnit = hydralisks->getUnits().front().unit;
	}


	if (BWAPI::Broodwar->getFrameCount() % 25 * 5 == 0)
	{
		newAddMovePositions.clear();
		newAddMovePositions.push_back(firstUnit->getPosition());
		for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
		{
			it->second = newAddMovePositions;
		}
	}

	// overlord follow
	if (overlords->getUnits().size() > 0)
	{
		std::vector<UnitState>&	 overlordUnits = overlords->getUnits();
		for (int i = 0; i < int(overlordUnits.size()); i++)
		{
			overlordUnits[i].unit->follow(firstUnit);
		}
	}


	newArmyRally();

	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	nearbyUnits.clear();
	friendUnitNearBy.clear();
	BWAPI::Unitset tmp; //= firstUnit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
	int count = 0;

	int checkMod = 2;
	if (zerglings->getUnits().size() > 100)
	{
		checkMod = 5;
	}
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (army.first == BWAPI::UnitTypes::Zerg_Zergling)
				{
					count += 1;
					if (count % checkMod == 0)
					{
						BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
						tmp.insert(percentUnits.begin(), percentUnits.end());
					}
				}
				else
				{
					BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
					tmp.insert(percentUnits.begin(), percentUnits.end());
				}
			}
		}
	}

	/*
	for (int percent = 0; percent < 20; percent++)
	{
		if (zerglings->getUnits().size() > 0)
		{
			BWAPI::Unitset percentUnits = zerglings->getUnits()[int(zerglings->getUnits().size() * percent * 0.05)].unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
			tmp.insert(percentUnits.begin(), percentUnits.end());
		}
		
		if (hydralisks->getUnits().size() > 0)
		{
			int attackRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange() + 2 * 32;
			BWAPI::Unitset percentUnits = hydralisks->getUnits()[int(hydralisks->getUnits().size() * percent * 0.05)].unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
			tmp.insert(percentUnits.begin(), percentUnits.end());
		}
	}*/

	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			nearbyUnits.insert(unit);
		}

		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Hydralisk && unit->getType() != BWAPI::UnitTypes::Zerg_Zergling && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}


	BWAPI::Position moveBackBase;
	if (myRegions.size() > 1)
		moveBackBase = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		moveBackBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	BWAPI::Unitset unitsInCircle;
	unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(moveBackBase, 12 * 32);

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

	bool isUnderAttack = false;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (u.unit->isUnderAttack())
				{
					isUnderAttack = true;
					break;
				}
			}
		}

		if (isUnderAttack == true)
		{
			break;
		}
	}


	switch (state)
	{
	case GROUPARMY:
	{
		BWAPI::Broodwar->drawCircleMap(groupPosition.x, groupPosition.y, 16, BWAPI::Colors::Red, true);

		groupStatus = checkGroupStatus(groupPosition);

		if (isUnderAttack || nearbyUnits.size() > 0 || groupStatus == true)
		{
			//BWAPI::Broodwar->printf("change to attack");
			state = ATTACK;
			nextGroupTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
			break;
		}

		for (auto army : tacticArmy)
		{
			army.second->armyMove(groupPosition);
		}

		//hydralisks->armyMove(groupPosition);
	}
	break;

	case ATTACK:
	{
		
		if (nearbyUnits.size() == 0 && !isUnderAttack && groupStatus == false && BWAPI::Broodwar->getFrameCount() > nextGroupTime)
		{
			//BWAPI::Broodwar->printf("change to groupby");

			state = GROUPARMY;
			break;
		}

		if (!hasEnemy())
		{
			//BWAPI::Broodwar->printf("hydralisk end");
			state = END;
			break;

		}

		if (needRetreat(firstUnit) && enemyInCircle.size() == 0 && sunkenNearbyEnemy.size() == 0)
		{

			//BWAPI::Broodwar->printf("hydrlisk retreat");

			state = RETREAT;
			
			retreatCount++;
			if (retreatCount >= 2)
			{
				state = END;
				break;
			}

			retreatTime = BWAPI::Broodwar->getFrameCount() + 10 * 25;
			if (myRegions.size() > 1)
				nextRetreatPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
			else
				nextRetreatPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

			break;
		}

		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				army.second->mixAttack(attackPosition, nearbyUnits);
			}
		}

		
		//hydralisks->attack(attackPosition);
	}
	break;

	case RETREAT:
	{

		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				army.second->armyMove(nextRetreatPosition);
			}
		}

		// if no enemy nearby, wait to attack again
		if (nearbyUnits.size() == 0 && !isUnderAttack && BWAPI::Broodwar->getFrameCount() >= retreatTime)
		{
			//BWAPI::Broodwar->printf("wait");
			state = WAIT;
			nextAttackTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
		}

		//hydralisks->armyMove(nextRetreatPosition);

		//if we reach the retreat position and still under attack, change to attack mode to choose the next action
		if (BWAPI::Broodwar->getFrameCount() >= retreatTime && (isUnderAttack || nearbyUnits.size() > 0))
		{
			//BWAPI::Broodwar->printf("retreat to attack");
			state = ATTACK;
			break;
		}
	}
	break;

	case WAIT:
	{
		if (nearbyUnits.size() == 0)
		{
			for (auto army : tacticArmy)
			{
				if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
				{
					army.second->armyMove(firstUnit->getPosition());
				}
			}

			// do regroup 
			//hydralisks->armyMove(firstHydra->getPosition());

			// waiting for hydralisks
			if (BWAPI::Broodwar->getFrameCount() > nextAttackTime)
				state = ATTACK;
		}
		// has nearby enemy
		else
		{
			state = ATTACK;
		}
		break;
	}
	}
}


bool HydraliskTactic::isTacticEnd()
{
	if (state == END)
	{
		//if (tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits().size() > 0)
			//tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->armyMove(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
		return true;
	}
	else
		return false;
}


HydraliskTactic::HydraliskTactic()
{
	state = GROUPARMY;
	//movePosition = BWAPI::Positions::None;
	//newAddMovePositions.push_back(attackPosition);

	retreatCount = 0;
	ourArmySupply = 0;

}

void HydraliskTactic::generateAttackPath()
{


}