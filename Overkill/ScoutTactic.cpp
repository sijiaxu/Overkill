#pragma once
#include "ScoutTactic.h"
#include "InformationManager.h"


ScoutTactic::ScoutTactic()
{
	if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
		state = scoutForEnemyBase;
	else
		state = scoutForEnemyInfo;

	if (state == scoutForEnemyBase)
	{
		BOOST_FOREACH(BWAPI::TilePosition  startLocation, BWAPI::Broodwar->getStartLocations())
		{
			if (startLocation != BWAPI::Broodwar->self()->getStartLocation()){
				scoutLocation.push_back(scoutTarget(startLocation, 0 - int(startLocation.getDistance(BWAPI::Broodwar->self()->getStartLocation()))));
			}
		}

		if (scoutLocation.size() == 1)
		{
			InformationManager::Instance().setLocationEnemyBase(scoutLocation.front().location);
			scoutLocation.clear();
		}
	}

	endFlag = false;

	double2 direct1(1, 0);
	moveDirections.push_back(direct1);
	double2 direct2(1, 1);
	moveDirections.push_back(direct2);
	double2 direct3(0, 1);
	moveDirections.push_back(direct3);
	double2 direct4(-1, 1);
	moveDirections.push_back(direct4);

	double2 direct5(-1, 0);
	moveDirections.push_back(direct5);
	double2 direct6(-1, -1);
	moveDirections.push_back(direct6);
	double2 direct7(0, -1);
	moveDirections.push_back(direct7);
	double2 direct8(1, -1);
	moveDirections.push_back(direct8);

	//set different type of unit to the same scout array
	for (auto army : tacticArmy)
	{
		if (army.second->getUnits().size() > 0)
		{
			for (auto armyUnit : army.second->getUnits())
			{
				overLordIdle.push_back(Scout(armyUnit.unit));
			}
		}
	}
}

void ScoutTactic::onUnitDestroy(BWAPI::Unit unit)
{

	for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
	{
		if (it->scoutUnit == unit)
		{
			overLordIdle.erase(it);
			break;
		}
	}

	for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end(); it++)
	{
		if (it->scoutUnit == unit)
		{
			overLordScouts.erase(it);
			break;
		}
	}
}

bool ScoutTactic::HasZergling()
{
	for (auto u : overLordIdle)
	{
		if (u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			return true;
		}
	}

	for (auto u : overLordScouts)
	{
		if (u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			return true;
		}
	}

	return false;
}


void ScoutTactic::assignScoutWork()
{
	if (overLordIdle.size() > 0)
	{
		for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
		{
			if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
			{
				it->TileTarget = BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition());
				overLordScouts.push_back(*it);
				it = overLordIdle.erase(it);
			}
			else
				it++;
		}
	}

	if (scoutLocation.size() == 0 || overLordIdle.size() == 0)
		return;

	

	while (scoutLocation.size() > 0)
	{
		if (overLordIdle.size() == 0)
			break;

		std::sort(scoutLocation.begin(), scoutLocation.end());
		BWAPI::TilePosition maxPriorityLocation = scoutLocation.back().location;
		BWAPI::TilePosition minTile = scoutLocation.back().location;
		scoutLocation.pop_back();

		int minDistance = 999999;
		std::vector<Scout>::iterator minOverlord;
		for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
		{
			if (it->scoutUnit->getDistance(BWAPI::Position(maxPriorityLocation)) < minDistance)
			{
				minDistance = it->scoutUnit->getDistance(BWAPI::Position(maxPriorityLocation));
				minOverlord = it;
			}
		}
		minOverlord->TileTarget = maxPriorityLocation;

		//for natural detect at scoutForEnemyBase stage
		double closest = 999999999;
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(minTile)) < 90)
			{
				continue;
			}
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(minTile)) < closest)
			{
				closest = base->getGroundDistance(BWTA::getNearestBaseLocation(minTile));
				minOverlord->naturalTileTarget = base->getTilePosition();
			}
		}

		overLordScouts.push_back(*minOverlord);
		overLordIdle.erase(minOverlord);
	}
}


void ScoutTactic::update()
{
	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& enemyBuildings = InformationManager::Instance().getEnemyOccupiedDetail();

	switch (state)
	{
	case scoutForEnemyBase:
	{
		if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
		{
			assignScoutWork();

			// update for scout info
			for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end();)
			{
				smartMove(it->scoutUnit, BWAPI::Position(it->TileTarget));

				bool depot = false;
				BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->enemy()->getUnits())
				{
					if (u->getType().isBuilding() && ((BWTA::getRegion(u->getPosition()) == BWTA::getRegion(it->TileTarget))
						|| (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(it->naturalTileTarget))))
						depot = true;
				}
				if (depot)
				{
					InformationManager::Instance().setLocationEnemyBase(it->TileTarget);
					//find the enemy start base
					scoutLocation.clear();
					break;
				}
				else if (BWAPI::Broodwar->isVisible(it->TileTarget) && !depot)
				{
					overLordIdle.push_back(*it);
					it = overLordScouts.erase(it);

					//unknown base is one, set it to enemy
					if ((scoutLocation.size() + overLordScouts.size()) == 1)
					{
						BWAPI::TilePosition t = scoutLocation.size() == 1 ? scoutLocation[0].location : overLordScouts[0].TileTarget;
						InformationManager::Instance().setLocationEnemyBase(t);
						scoutLocation.clear();
						break;
					}
				}
				else
					it++;
			}
		}
		else
		{
			overLordIdle.insert(overLordIdle.end(), overLordScouts.begin(), overLordScouts.end());
			overLordScouts.clear();
			BOOST_FOREACH(Scout& u, overLordIdle)
			{
				u.scoutUnit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
				//u.scoutUnit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			}

			for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
			{
				if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone)
				{
					WorkerManager::Instance().finishedWithWorker(it->scoutUnit);
					overLordIdle.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			/*
			if (BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Terran)
			{
				int minDistance = 99999;
				BWAPI::Unit minOverlord = NULL;
				for (auto u : overLordIdle)
				{
					if (u.scoutUnit->getDistance(InformationManager::Instance().GetEnemyBasePosition()) < minDistance)
					{
						minDistance = u.scoutUnit->getDistance(InformationManager::Instance().GetEnemyBasePosition());
						minOverlord = u.scoutUnit;
					}
				}
				if (minOverlord != NULL)
				{

					//minOverlord->move(InformationManager::Instance().GetEnemyBasePosition());
				}
			}*/
			generateScoutLocation();
			//endFlag = true;
			state = scoutForEnemyInfo;
		}
	
		break;
	}
	case scoutForEnemyInfo:
	{
		assignScoutWork();

		for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end(); it++)
		{
			if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
			{
				BattleArmy::smartMove(it->scoutUnit, BWAPI::Position(it->TileTarget));
			}
			else
			{
				overlordMove(*it);
			}
			/*
			if (it->overLord->getDistance(BWAPI::Position(it->TileTarget)) < 32 * 5)//BWAPI::Broodwar->isVisible(it->TileTarget))
			{
			overLordIdle.push_back(*it);
			it = overLordScouts.erase(it);
			}
			else
			it++;*/
		}

		break;
	}
	}

}

void ScoutTactic::generateScoutLocation()
{
	BWTA::Region* nextBase = BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation());
	double maxDist = 0;
	BWAPI::Position maxChokeCenter;
	BWTA::Chokepoint* maxChoke = nullptr;
	BOOST_FOREACH(BWTA::Chokepoint* p, nextBase->getChokepoints())
	{
		if (InformationManager::Instance().GetOurBaseUnit()->getDistance(p->getCenter()) > maxDist)
		{
			maxDist = InformationManager::Instance().GetOurBaseUnit()->getDistance(p->getCenter());
			maxChokeCenter = p->getCenter();
			maxChoke = p;
		}
	}

	BWTA::Region* reachRegion = maxChoke->getRegions().first == nextBase ? maxChoke->getRegions().second : maxChoke->getRegions().first;

	if (BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Terran)
	{
		//scoutLocation.push_back(BWAPI::TilePosition(InformationManager::Instance().GetEnemyNaturalPosition()));
		scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()), 100));

		//if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
		//scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(reachRegion->getCenter()), 80));
	}

	BWAPI::Position baseChoke = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();

	scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(maxChokeCenter), 50));
	scoutLocation.push_back(scoutTarget(InformationManager::Instance().getOurNatrualLocation(), 30));
	scoutLocation.push_back(scoutTarget(InformationManager::Instance().getOurNatrualLocation(), 30));
}

void ScoutTactic::overlordMove(Scout& overlord)
{
	//select the move target every 4 position step
	if (overlord.nextMovePosition == BWAPI::TilePositions::None)
	{
		int walkRate = 4;
		std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
		int lowestAirForce = 99999;
		int cloest = 99999;
		BWAPI::TilePosition nextMovePosition = BWAPI::TilePositions::None;
		for (int i = 0; i < int(moveDirections.size()); i++)
		{
			BWAPI::TilePosition nextTilePosition(overlord.scoutUnit->getTilePosition().x + int(moveDirections[i].x * walkRate), overlord.scoutUnit->getTilePosition().y + int(moveDirections[i].y * walkRate));
			if (nextTilePosition.x > BWAPI::Broodwar->mapWidth() - 1 || nextTilePosition.x < 0
				|| nextTilePosition.y > BWAPI::Broodwar->mapHeight() - 1 || nextTilePosition.y < 0)
			{
				continue;
			}
			int nextAirForce = int(influnceMap[nextTilePosition.x][nextTilePosition.y].airForce) + int(influnceMap[nextTilePosition.x][nextTilePosition.y].decayAirForce);
			if (nextAirForce < lowestAirForce || (nextAirForce == lowestAirForce && nextTilePosition.getDistance(overlord.TileTarget) < cloest))
			{
				nextMovePosition = nextTilePosition;
				lowestAirForce = nextAirForce;
				cloest = int(nextTilePosition.getDistance(overlord.TileTarget));
			}
		}
		overlord.nextMovePosition = nextMovePosition;
	}
	else
	{
		smartMove(overlord.scoutUnit, BWAPI::Position(overlord.nextMovePosition));
		if (overlord.scoutUnit->getDistance(BWAPI::Position(overlord.nextMovePosition)) < 32)
		{
			overlord.nextMovePosition = BWAPI::TilePositions::None;
		}
	}
}



bool ScoutTactic::isTacticEnd()
{
	if (overLordIdle.size() == 0 && overLordScouts.size() == 0)
	{
		return true;
	}

	if (endFlag)
	{
		for (auto u : overLordIdle)
		{
			tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u.scoutUnit);
		}

		for (auto u : overLordScouts)
		{
			tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u.scoutUnit);
		}
		return true;
	}
	else
	{
		return false;
	}
}



void ScoutTactic::smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition) const
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		if (attacker->isSelected())
		{
			return;
		}
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move)
		&& (currentCommand.getTargetPosition() == targetPosition)
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 5)
		&& attacker->isMoving())
	{
		if (attacker->isSelected())
		{
			return;
			//BWAPI::Broodwar->printf("Previous Command Frame=%d Pos=(%d, %d)", attacker->getLastCommandFrame(), currentCommand.getTargetPosition().x, currentCommand.getTargetPosition().y);
		}
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
	{
		BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y,
			targetPosition.x, targetPosition.y, BWAPI::Colors::Orange);
	}
}


