#include "Common.h"
#include "ScoutManager.h"
#include "InformationManager.h"


ScoutManager::ScoutManager()
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
}

void ScoutManager::addScoutUnit(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		Scout scout(unit);
		overLordIdle.push_back(scout);
	}
}


void ScoutManager::onUnitMorph(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		Scout scout(unit);
		overLordIdle.push_back(scout);
	}
}


void ScoutManager::onUnitShow(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		Scout scout(unit);
		overLordIdle.push_back(scout);
	}
}

std::vector<BWAPI::Unit> ScoutManager::getOverLordArmy(int count)
{
	if (int(overLordIdle.size()) < count)
		return std::vector<BWAPI::Unit>();

	std::vector<BWAPI::Unit> overlordArmy;
	for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
	{
		if (count > 0)
		{
			overlordArmy.push_back(it->overLord);
			it = overLordIdle.erase(it);
			count--;
		}
		else
			break;
	}
	return overlordArmy;
}

void ScoutManager::giveBackOverLordArmy(BattleArmy* army)
{
	BOOST_FOREACH(UnitState u, army->getUnits())
	{
		overLordIdle.push_back(Scout(u.unit));
	}
}


void ScoutManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end(); it++)
		{
			if (it->overLord == unit)
			{
				if (state == scoutForEnemyBase)
					scoutLocation.push_back(scoutTarget(it->TileTarget, 0 - int(it->TileTarget.getDistance(BWAPI::Broodwar->self()->getStartLocation()))));
				overLordScouts.erase(it);
				return;
			}
		}

		for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
		{
			if (it->overLord == unit)
			{
				overLordIdle.erase(it);
				return;
			}
		}
	}
}

void ScoutManager::assignScoutWork()
{
	if (scoutLocation.size() == 0 || overLordIdle.size() == 0)
		return;

	for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
	{
		if (it->overLord->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			it->TileTarget = BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition());
			overLordScouts.push_back(*it);
			overLordIdle.erase(it++);
		}
		else
			it++;
	}

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
			if (it->overLord->getDistance(BWAPI::Position(maxPriorityLocation)) < minDistance)
			{
				minDistance = it->overLord->getDistance(BWAPI::Position(maxPriorityLocation));
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


	/*
	// set scout Position
	for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
	{
		if (scoutLocation.size() > 0)
		{
			std::sort(scoutLocation.begin(), scoutLocation.end());
			it->TileTarget = scoutLocation.back().location;
			BWAPI::TilePosition minTile = scoutLocation.back().location;
			scoutLocation.pop_back();
			
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
					it->naturalTileTarget = base->getTilePosition();
				}
			}

			overLordScouts.push_back(*it);
			it = overLordIdle.erase(it);
		}
		else
			it++;
	}*/
}

void ScoutManager::overlordMove(Scout& overlord)
{
	if (overlord.nextMovePosition == BWAPI::TilePositions::None)
	{
		int walkRate = 4;
		std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
		int lowestAirForce = 99999;
		int cloest = 99999;
		BWAPI::TilePosition nextMovePosition = BWAPI::TilePositions::None;
		for (int i = 0; i < int(moveDirections.size()); i++)
		{
			BWAPI::TilePosition nextTilePosition(overlord.overLord->getTilePosition().x + int(moveDirections[i].x * walkRate), overlord.overLord->getTilePosition().y + int(moveDirections[i].y * walkRate));
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
		smartMove(overlord.overLord, BWAPI::Position(overlord.nextMovePosition));
		if (overlord.overLord->getDistance(BWAPI::Position(overlord.nextMovePosition)) < 32)
		{
			overlord.nextMovePosition = BWAPI::TilePositions::None;
		}
	}
}


void ScoutManager::update()
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
				smartMove(it->overLord, BWAPI::Position(it->TileTarget));

				bool depot = false;
				BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->enemy()->getUnits())
				{
					if (u->getType().isBuilding() && ((BWTA::getRegion(u->getPosition()) == BWTA::getRegion(it->TileTarget)) 
						|| (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(it->naturalTileTarget))) )
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
				u.overLord->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			}
			for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
			{
				if (it->overLord->getType() == BWAPI::UnitTypes::Zerg_Drone)
				{
					WorkerManager::Instance().finishedWithWorker(it->overLord);
					overLordIdle.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			generateScoutLocation();
			state = scoutForEnemyInfo;
		}
		break;
	}
	case scoutForEnemyInfo:
	{
		assignScoutWork();
		
		for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end(); it++)
		{
			if (it->overLord->getType() == BWAPI::UnitTypes::Zerg_Zergling)
			{
				BattleArmy::smartMove(it->overLord, BWAPI::Position(it->TileTarget));
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

	//TODO: arrange overlord to stay at all enemy expand base 
	//TODO: add overlord flee function
}

void ScoutManager::addScoutLocation(BWAPI::TilePosition location)
{

}


std::vector<scoutTarget>& ScoutManager::getPossibleEnemyBase()
{
	return scoutLocation;
}


void ScoutManager::generateScoutLocation()
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

		if (StrategyManager::Instance().getCurrentopeningStrategy() != NinePoolling)
		{
			scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(maxChokeCenter), 50));
			scoutLocation.push_back(scoutTarget(InformationManager::Instance().getOurNatrualLocation(), 30));
		}
	}
}


void ScoutManager::smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition) const
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
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

ScoutManager& ScoutManager::Instance()
{
	static ScoutManager s;
	return s;
}
