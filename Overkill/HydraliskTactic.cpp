#pragma once
#include "HydraliskTactic.h"


void HydraliskTactic::update()
{
	HydraliskArmy* hydralisks = dynamic_cast<HydraliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]);
	MutaliskArmy* mutalisks = dynamic_cast<MutaliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]);
	OverLordArmy* overlords = dynamic_cast<OverLordArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]);

	if ((*BWTA::getRegion(attackPosition)->getBaseLocations().begin())->isIsland())
	{
		state = END;
		return;
	}

	newArmyRally();

	switch (state)
	{
	case GROUPARMY:
	{
		/*
		BWAPI::Unit* unit = (*tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().begin()).unit;

		std::set<BWTA::Region *> & enemyRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
		std::set<BWTA::Region*> nearRegions = BWTA::getRegion(attackPosition)->getReachableRegions();
		std::list<BWTA::Region*> needFindRegions;
		std::set<BWTA::Region*> checkedRegions;
		BWTA::Region* nextRegion = BWTA::getRegion(attackPosition);
		checkedRegions.insert(BWTA::getRegion(attackPosition));

		while (true)
		{
			for (std::set<BWTA::Region*>::iterator it = nearRegions.begin(); it != nearRegions.end(); it++)
			{
				double newDistance = BWTA::getGroundDistance(unit->getTilePosition(), BWAPI::TilePosition((*it)->getCenter()));
				double oldDistance = BWTA::getGroundDistance(unit->getTilePosition(), BWAPI::TilePosition(nextRegion->getCenter()));
				if (newDistance > oldDistance)
					continue;

				if (enemyRegions.find(*it) == enemyRegions.end())
				{
					movePosition = (*it)->getCenter();
					break;
				}
				else
				{
					if (checkedRegions.find(*it) == checkedRegions.end())
					{
						needFindRegions.push_back(*it);
					}
				}
			}
			if (movePosition != BWAPI::Positions::None)
			{
				state = MOVE;
				break;
			}
			if (needFindRegions.size() == 0)
			{
				state = ATTACK;
				break;
			}
			nextRegion = needFindRegions.front();
			needFindRegions.pop_front();
			nearRegions = nextRegion->getReachableRegions();
			checkedRegions.insert(nextRegion);
		}*/

		state = ATTACK;
	}
	break;

	case ATTACK:
	{
		if (hydralisks->getUnits().size() <= 2 && mutalisks->getUnits().size() == 0)
		{
			state = END;
			return;
		}
			
		if (!hasEnemy())
		{
			state = END;
			return;
		}

		std::vector<hydrliskDistance> armyDistance;
		BOOST_FOREACH(UnitState u, hydralisks->getUnits())
		{
			armyDistance.push_back(hydrliskDistance(u.unit, u.unit->getDistance(attackPosition)));
		}
		std::sort(armyDistance.begin(), armyDistance.end());
		
		hydralisks->attack(attackPosition);

		bool enemyExsit = false;
		int antiAirBuildingCount = 0;
		std::set<BWAPI::Unit*> enemySet = armyDistance[0].unit->getUnitsInRadius(12 * 32);

		BWAPI::Broodwar->drawCircleMap(armyDistance[0].unit->getPosition().x(), armyDistance[0].unit->getPosition().y(), 12 * 32, BWAPI::Colors::Green, false);
		
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			BWAPI::UnitType unitType = u->getType();
			if (u->getPlayer() == BWAPI::Broodwar->enemy()
				&& (unitType == BWAPI::UnitTypes::Terran_Missile_Turret
				|| unitType == BWAPI::UnitTypes::Zerg_Spore_Colony))
			{
				antiAirBuildingCount++;
			}
			
			if (u->getPlayer() == BWAPI::Broodwar->enemy())
			{
				enemyExsit = true;
			}
		}

		// overlord follow
		std::vector<UnitState>&	 overlordUnits = overlords->getUnits();
		for (int i = 0; i < int(overlordUnits.size()); i++)
		{
			double splitPercent = i / double(overlordUnits.size());
			overlordUnits[i].unit->follow(armyDistance[int(armyDistance.size() * splitPercent)].unit);
		}

		// mutalisk attack 
		if (enemyExsit && antiAirBuildingCount <= 3)
			//snipe the high priority enemy unit in hydralisk's range
			mutalisks->mixArmyAttack((*hydralisks->getUnits().begin()).unit->getPosition());
		else
		{
			//if no enemy exsit, move to hydralisk's back
			BWAPI::Unit* unit = armyDistance[int(armyDistance.size() / 2)].unit;

			double2 direc = movePosition - unit->getPosition();
			double2 direcNormal = direc / direc.len();

			int targetx = unit->getPosition().x() + int(direcNormal.x * 32 * 6);
			int targety = unit->getPosition().y() + int(direcNormal.y * 32 * 6);

			BWAPI::Position p(targetx, targety);
			mutalisks->armyMove(p);
		}
	}
	break;

	case RETREAT:
	{
		hydralisks->armyMove(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		state = END;
	}
	break;
	}
}

bool HydraliskTactic::isTacticEnd()
{
	if (state == END)
	{
		OverLordArmy* overlords = dynamic_cast<OverLordArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]);
		BOOST_FOREACH(UnitState u, overlords->getUnits())
		{
			u.unit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		}
		return true;
	}
	else
		return false;
}


HydraliskTactic::HydraliskTactic()
{
	state = GROUPARMY;
	movePosition = BWAPI::Positions::None;
}

void HydraliskTactic::generateAttackPath()
{

}