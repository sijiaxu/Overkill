#pragma once
#include "HydraliskTactic.h"

//TODO: put in the battle tactic class
bool HydraliskTactic::needRetreat()
{
	int ourArmySupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size());
	BOOST_FOREACH(BWAPI::Unit* u, friendUnitNearBy)
	{
		ourArmySupply += u->getType().supplyRequired();
	}

	BWAPI::Unit* firstHydra = (*tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().begin()).unit;

	std::vector<std::vector<gridInfo>>& enemyIm = InformationManager::Instance().getEnemyInfluenceMap();
	double2 centerPoint(firstHydra->getTilePosition().x(), firstHydra->getTilePosition().y());
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

	if (ourArmySupply <= maxIM / 4)
	{
		return true;
	}

	int enemySupply = 0;
	BOOST_FOREACH(BWAPI::Unit* u, nearbyUnits)
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
			enemySupply += u->getType().supplyRequired();
		}
		else
			continue;
	}

	if (ourArmySupply * 1.2 < enemySupply)
		return true;
	else
		return false;
}


void HydraliskTactic::update()
{

	HydraliskArmy* hydralisks = dynamic_cast<HydraliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]);
	OverLordArmy* overlords = dynamic_cast<OverLordArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]);


	if (hydralisks->getUnits().size() == 0)
	{
		state = END;
		return;
	}

	BWAPI::Unit* firstHydra = hydralisks->getUnits().front().unit;

	if ((*BWTA::getRegion(attackPosition)->getBaseLocations().begin())->isIsland())
	{
		state = END;
		return;
	}

	if (BWAPI::Broodwar->getFrameCount() % 25 * 5 == 0)
	{
		newAddMovePositions.clear();
		newAddMovePositions.push_back((*hydralisks->getUnits().begin()).unit->getPosition());
		for (std::map<BWAPI::Unit*, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
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
			if (i == 0)
			{
				overlordUnits[i].unit->follow(hydralisks->getUnits().front().unit);
			}
			else
			{
				overlordUnits[i].unit->follow(hydralisks->getUnits().back().unit);
			}
		}
	}


	newArmyRally();

	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	nearbyUnits.clear();
	friendUnitNearBy.clear();
	std::set<BWAPI::Unit*> tmp = firstHydra->getUnitsInRadius(12 * 32);
	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit* unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			nearbyUnits.insert(unit);
		}

		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Hydralisk && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}


	BWAPI::Position moveBackBase;
	if (myRegions.size() > 1)
		moveBackBase = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		moveBackBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	std::set<BWAPI::Unit*> unitsInCircle;
	unitsInCircle = BWAPI::Broodwar->getUnitsInRadius(moveBackBase, 12 * 32);

	std::set<BWAPI::Unit*> enemyInCircle;
	BOOST_FOREACH(BWAPI::Unit* enemy, unitsInCircle)
	{
		if (enemy->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (enemy->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;

			enemyInCircle.insert(enemy);
		}
	}

	switch (state)
	{
	case ATTACK:
	{
		if (!hasEnemy())
		{
			state = END;
			break;
		}

		if (needRetreat() && enemyInCircle.size() == 0)
		{
			state = RETREAT;
			
			retreatCount++;
			if (retreatCount >= 5)
			{
				state = END;
				break;
			}

			retreatTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
			if (myRegions.size() > 1)
				nextRetreatPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
			else
				nextRetreatPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

			break;
		}
		
		hydralisks->attack(attackPosition);
	}
	break;

	case RETREAT:
	{
		bool isUnderAttack = false;
		BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits())
		{
			if (u.unit->isUnderAttack())
			{
				isUnderAttack = true;
				break;
			}
		}
		// if no enemy nearby, wait to attack again
		if (nearbyUnits.size() == 0 && !isUnderAttack)
		{
			state = WAIT;
			nextAttackTime = BWAPI::Broodwar->getFrameCount() + 5 * 25;
		}

		hydralisks->armyMove(nextRetreatPosition);

		//if we reach the retreat position and still under attack, change to attack mode to choose the next action
		if (BWAPI::Broodwar->getFrameCount() >= retreatTime && (isUnderAttack || nearbyUnits.size() > 0))
		{
			state = ATTACK;
			break;
		}
	}
	break;

	case WAIT:
	{
		if (nearbyUnits.size() == 0)
		{
			// do regroup 
			hydralisks->armyMove(firstHydra->getPosition());

			// waiting for hydralisks
			if (BWAPI::Broodwar->getFrameCount() > nextAttackTime && hydralisks->getUnits().size() >= 24)
				state = ATTACK;
		}
		// has nearby enemy
		else
		{
			state = ATTACK;
		}
		break;
	}

	case GROUPARMY:
	{
		hydralisks->armyMove(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));

		BOOST_FOREACH(UnitState u, hydralisks->getUnits())
		{
			if (u.unit->getPosition().getDistance(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation())) < 32 * 10)
			{
				state = END;
				break;
			}
		}
		break;
	}
	}
}


bool HydraliskTactic::isTacticEnd()
{
	if (state == END)
	{
		if (tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits().size() > 0)
			tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->armyMove(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
		return true;
	}
	else
		return false;
}


HydraliskTactic::HydraliskTactic()
{
	state = ATTACK;
	movePosition = BWAPI::Positions::None;
	newAddMovePositions.push_back(attackPosition);

	retreatCount = 0;
}

void HydraliskTactic::generateAttackPath()
{


}