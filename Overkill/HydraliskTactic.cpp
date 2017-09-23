#pragma once
#include "HydraliskTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"


//TODO: put in the battle tactic class
bool HydraliskTactic::needRetreat(BWAPI::Unit firstUnit, bool hasDetector)
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

	if (ourArmySupply <= maxIM / 5)
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

	if (tacticArmy[BWAPI::UnitTypes::Zerg_Lurker]->getUnits().size() > 0)
	{
		if (hasDetector && ourArmySupply * 1.2 < enemySupply && BWAPI::Broodwar->self()->supplyUsed() < 150 * 2)
			return true;
	}
	else
	{
		if (ourArmySupply * 1.2 < enemySupply && BWAPI::Broodwar->self()->supplyUsed() < 150 * 2)
			return true;
	}
	
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


	BWAPI::Unit firstUnit = NULL;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord && army.second->getUnits().size() > 0)
		{
			firstUnit = army.second->getUnits().front().unit;
		}
	}


	if (BWAPI::Broodwar->getFrameCount() % 25 * 5 == 0)
	{
		newAddMovePositions.clear();
		if (firstUnit->getPosition().isValid())
		{
			newAddMovePositions.push_back(firstUnit->getPosition());
		}
		
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
	int checkMod = 0;
	
	if (ourArmyCount <= 50)
		checkMod = 1;
	else
		checkMod = 2;
		
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
		}
	}

	bool hasDetector = false;
	for (std::set<BWAPI::Unit>::iterator it = nearbyUnits.begin(); it != nearbyUnits.end();)
	{
		if ((*it)->getType() == BWAPI::UnitTypes::Zerg_Overlord
			|| (*it)->getType() == BWAPI::UnitTypes::Terran_Science_Vessel
			|| (*it)->getType() == BWAPI::UnitTypes::Protoss_Observer
			|| (*it)->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
			|| (*it)->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
			|| (*it)->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			hasDetector = true;
		}

		if (!(*it)->isDetected() && (*it)->isVisible())
		{
			it = nearbyUnits.erase(it);
		}
		else
		{
			it++;
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

	if (!hasEnemy())
	{
		retreatSet();
	}

	switch (state)
	{
	case GROUPARMY:
	{
		//BWAPI::Broodwar->drawCircleMap(groupPosition.x, groupPosition.y, 16, BWAPI::Colors::Red, true);

		//groupStatus = checkGroupStatus(groupPosition);

		if (nearbyUnits.size() > 0)
		{
			//BWAPI::Broodwar->printf("change to attack");
			state = ATTACK;
			nextGroupTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
			break;
		}

		//for (auto army : tacticArmy)
		//{
			//army.second->armyMove(groupPosition);
		//}

		//flockingMove(attackPosition);
		togetherMove(attackPosition);

		//hydralisks->armyMove(groupPosition);
	}
	break;

	case ATTACK:
	{
		
		if (nearbyUnits.size() == 0 && BWAPI::Broodwar->getFrameCount() > nextGroupTime)
		{
			//BWAPI::Broodwar->printf("change to groupby");

			state = GROUPARMY;
			break;
		}

		if (needRetreat(firstUnit, hasDetector) && enemyInCircle.size() == 0 && sunkenNearbyEnemy.size() == 0)
		{

			//BWAPI::Broodwar->printf("hydrlisk retreat");

			retreatSet();

			break;
		}

		armyAttack(attackPosition, nearbyUnits);

		
		//hydralisks->attack(attackPosition);
	}
	break;

	case RETREAT:
	{

		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				if (army.first == BWAPI::UnitTypes::Zerg_Lurker)
				{
					for (auto u : army.second->getUnits())
					{
						if (u.unit->isBurrowed())
							u.unit->unburrow();
						else
							BattleArmy::smartMove(u.unit, nextRetreatPosition);
					}
				}
				else
				{
					army.second->armyMove(nextRetreatPosition);
				}
			}
		}

		// if no enemy nearby, wait to attack again
		if ((nearbyUnits.size() == 0 && !isUnderAttack) || BWAPI::Broodwar->getFrameCount() >= retreatTime)
		{
			//when current is not under attack, end this tactic.
			state = END;
			break;

			/*
			if (retreatCount >= 2)
			{
				state = END;
				break;
			}
			else
			{
				state = GROUPARMY;
			}*/
		}

		/*
		//if we reach the retreat position and still under attack, change to attack mode to choose the next action
		if (BWAPI::Broodwar->getFrameCount() >= retreatTime && (isUnderAttack || nearbyUnits.size() > 0))
		{
			//BWAPI::Broodwar->printf("retreat to attack");
			state = ATTACK;
			break;
		}
		*/
	}
	break;

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