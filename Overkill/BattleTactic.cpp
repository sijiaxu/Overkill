#pragma once
#include "BattleTactic.h"
#include "InformationManager.h"


void BattleTactic::armyAttack(BWAPI::Position attackPosition, std::set<BWAPI::Unit> nearbyUnits)
{
	BWAPI::Unit pathLeader = chooseLeader(attackPosition, false);
	std::list<BWAPI::Position> path = aStarGroundPathFinding(pathLeader->getPosition(), attackPosition);
	BWAPI::Position moveDestination = path.front();
	double2 backVector = (double2(pathLeader->getPosition()) - double2(moveDestination)).normal();
	BWAPI::Position gatherPosition = double2(pathLeader->getPosition()) + backVector * 32 * 3;

	//just assign the new add enemy
	for (auto u : currentAttackUnits)
	{
		nearbyUnits.erase(u);
	}
	currentAttackUnits.clear();

	for (auto armyType : attackOrder)
	{
		std::vector<EnemyUnit>& priorityEnemy = tacticArmy[armyType]->mixAttack(attackPosition, nearbyUnits);
		if (priorityEnemy.size() >= 50)
		{
			priorityEnemy.erase(priorityEnemy.begin(), priorityEnemy.end() - 50);
		}
		std::sort(priorityEnemy.begin(), priorityEnemy.end());

		int armyIndex = 0;
		for (int i = priorityEnemy.size() - 1; i >= 0; i--)
		{
			int targetPriority = priorityEnemy[i].priority;
			int targetHp = priorityEnemy[i].unit->getHitPoints() + priorityEnemy[i].unit->getShields();
			int needCount = BattleArmy::calMaxAssign(priorityEnemy[i].unit, armyType);

			for (; armyIndex < int(tacticArmy[armyType]->getUnits().size()); armyIndex++)
			{
				EnemyUnit originalTarget = tacticArmy[armyType]->getUnits()[armyIndex].target;
				//keep our unit attack the unchanged enemy
				if (originalTarget.unit == NULL || !originalTarget.unit->exists() || !originalTarget.unit->isVisible()
					|| originalTarget.priority < targetPriority)
				{
					tacticArmy[armyType]->getUnits()[armyIndex].target = priorityEnemy[i];
					needCount--;
					if (needCount == 0)
					{
						currentAttackUnits.insert(priorityEnemy[i].unit);
						nearbyUnits.erase(priorityEnemy[i].unit);
						armyIndex++;
						break;
					}
				}
				else
				{
					currentAttackUnits.insert(tacticArmy[armyType]->getUnits()[armyIndex].target.unit);
				}

				tacticArmy[armyType]->attackMoveLowHealth(tacticArmy[armyType]->getUnits()[armyIndex].unit,
					tacticArmy[armyType]->getUnits()[armyIndex].target.unit->getPosition(), tacticArmy[armyType]->getUnits()[armyIndex].target.unit);
			}
		}

		//enemy is too little or already assigned
		if (armyIndex < int(tacticArmy[armyType]->getUnits().size()))
		{
			for (; armyIndex < int(tacticArmy[armyType]->getUnits().size()); armyIndex++)
			{
				EnemyUnit originalTarget = tacticArmy[armyType]->getUnits()[armyIndex].target;
				if (originalTarget.unit == NULL || !originalTarget.unit->exists() || !originalTarget.unit->isVisible())
				{
					tacticArmy[armyType]->getUnits()[armyIndex].target = EnemyUnit();
					if (tacticArmy[armyType]->getUnits()[armyIndex].unit == pathLeader || !armyType.isFlyer())
						tacticArmy[armyType]->attackMoveLowHealth(tacticArmy[armyType]->getUnits()[armyIndex].unit, attackPosition);
					else
						tacticArmy[armyType]->attackMoveLowHealth(tacticArmy[armyType]->getUnits()[armyIndex].unit, gatherPosition);
				}
				else
				{
					currentAttackUnits.insert(originalTarget.unit);
					tacticArmy[armyType]->attackMoveLowHealth(tacticArmy[armyType]->getUnits()[armyIndex].unit, originalTarget.unit->getPosition(), originalTarget.unit);
				}
			}
		}
	}
}


BWAPI::Unit BattleTactic::chooseLeader(BWAPI::Position destination, bool onlyFlyer)
{
	if (leader != NULL && leader->exists() && BWAPI::Broodwar->getFrameCount() % 100 != 0)
	{
		return leader;
	}

	BWAPI::Unit firstUnit = NULL;
	std::map<BWTA::Region*, BWAPI::Position> regionPath;
	std::map<BWTA::Region*, double> regionPathLength;
	BWAPI::Unit groundLeader = NULL;
	int minGroundDistance = 999999;
	BWAPI::Unit airLeader = NULL;
	int minAirDistance = 9999999;

	for (auto army : tacticArmy)
	{
		if (army.first == BWAPI::UnitTypes::Zerg_Overlord)
			continue;

		for (auto u : army.second->getUnits())
		{
			firstUnit = u.unit;
			
			if (regionPath.find(BWTA::getRegion(u.unit->getPosition())) == regionPath.end() &&
				BWTA::getRegion(u.unit->getPosition()) != NULL)
			{
				std::list<BWAPI::Position> path = aStarGroundPathFinding(BWTA::getRegion(u.unit->getPosition())->getCenter(), destination);
				regionPath[BWTA::getRegion(u.unit->getPosition())] = path.front();

				double pathLength = 0;
				if (path.size() > 1)
				{
					BWAPI::Position previous = path.front();
					for (auto it = std::next(path.begin(), 1); it != path.end(); it++)
					{
						pathLength += (*it).getDistance(previous);
						previous = *it;
					}
				}
				regionPathLength[BWTA::getRegion(u.unit->getPosition())] = pathLength;
			}
			
			int distance = 0;
			
			if (onlyFlyer)
			{
				distance = int(u.unit->getDistance(destination));
			}
			else
			{
				distance = int(u.unit->getDistance(regionPath[BWTA::getRegion(u.unit->getPosition())]) + regionPathLength[BWTA::getRegion(u.unit->getPosition())]);
			}

			if (u.unit->getType().isFlyer() && distance < minAirDistance)
			{
				minAirDistance = distance;
				airLeader = u.unit;
			}

			if (!u.unit->getType().isFlyer() && distance < minGroundDistance)
			{
				minGroundDistance = distance;
				groundLeader = u.unit;
			}
		}
	}

	if (groundLeader == NULL)
	{
		leader = airLeader;
	}
	else
	{
		leader = groundLeader;
	}
	
	if (leader == NULL)
	{
		return firstUnit;
	}
	else
	{
		return leader;
	}
}

void BattleTactic::togetherMove(BWAPI::Position destination)
{
	bool onlyFlyer = true; 
	BWAPI::Unit goundUnit = NULL;
	for (auto army : tacticArmy)
	{
		if (army.first == BWAPI::UnitTypes::Zerg_Overlord)
			continue;
		if (!army.first.isFlyer() && army.second->getUnits().size() > 0)
		{
			onlyFlyer = false;
			goundUnit = army.second->getUnits().front().unit;
			break;
		}
	}

	BWAPI::Unit pathLeader = chooseLeader(destination, onlyFlyer);
	BWAPI::Broodwar->drawCircleMap(BWAPI::Position(int(pathLeader->getPosition().x), int(pathLeader->getPosition().y)), 5, BWAPI::Colors::Red, true);
	
	double2 centerPosition;
	int count = 0;
	int groundTotalSize = 0;
	int airTotalSize = 0;

	for (auto army : tacticArmy)
	{
		if (army.first == BWAPI::UnitTypes::Zerg_Overlord)
			continue;
		for (auto u : army.second->getUnits())
		{
			if (u.unit->isBurrowed())
			{
				u.unit->unburrow();
			}
			if (u.unit == pathLeader)
				continue;

			if (!army.first.isFlyer())
			{
				groundTotalSize += army.first.width() * army.first.height();
			}
			else
			{
				airTotalSize += army.first.width() * army.first.height();
			}
			centerPosition = centerPosition + double2(u.unit->getPosition());
			count++;
		}
	} 

	//have more than one unit
	if (count > 0)
	{
		centerPosition = centerPosition / count;
		int totalSize = groundTotalSize + airTotalSize; //groundTotalSize >= airTotalSize ? groundTotalSize : airTotalSize;
		int checkRadius = int(sqrt((totalSize * 5) / 3.1415926)) < 32 * 3 ? 32 * 3 : int(sqrt((totalSize * 5) / 3.1415926));

		int inRadiusCount = 0;
		int totalCount = 0;
		for (auto army : tacticArmy)
		{
			if (army.first == BWAPI::UnitTypes::Zerg_Overlord)
				continue;
			for (auto u : army.second->getUnits())
			{
				if (u.unit == pathLeader)
					continue;

				totalCount += 1;
				if (int((double2(u.unit->getPosition()) - centerPosition).len()) < checkRadius)
				{
					inRadiusCount += 1;
				}
			}
		}

		std::list<BWAPI::Position> path = aStarGroundPathFinding(pathLeader->getPosition(), destination);
		BWAPI::Position moveDestination;
		if (onlyFlyer)
		{
			moveDestination = destination;
		}
		else
		{
			moveDestination = path.front();
			if (pathLeader->getDistance(moveDestination) < 32 * 3)
			{
				if (path.size() > 1)
				{
					moveDestination = *(std::next(path.begin(), 1));
				}
				else
				{
					moveDestination = destination;
				}
			}
		}


		//for army stuck situation...
		if (BWAPI::Broodwar->getFrameCount() % 100 < 30)
		{
			for (auto a : tacticArmy)
			{
				if (!a.first.isFlyer())
				{
					a.second->armyMove(destination);
				}
				else
				{
					if (onlyFlyer)
					{
						a.second->armyMove(destination);
					}
					else
					{
						a.second->armyMove(goundUnit->getPosition());
					}
				}
			}

			return;
		}


		if (inRadiusCount * 10 / totalCount >= 8 || needMove)
		{
			BattleArmy::smartMove(pathLeader, moveDestination);
			
			curStopTime = 0;
			curMoveTime += 1;
			if (curMoveTime > 24 * 5)
			{
				needMove = false;
			}
		}
		else
		{
			pathLeader->stop();

			curMoveTime = 0;
			curStopTime += 1;
			if (curStopTime > 24 * 10)
			{
				needMove = true;
			}
		}
		

		double2 backVector = (double2(pathLeader->getPosition()) - double2(moveDestination)).normal();
		double2 rotateVector = backVector.rotateReturn(90).normal();
		BWAPI::Position baseMoveTarget = double2(pathLeader->getPosition()) + backVector * 32 * 2;

		BWAPI::Position ta = double2(baseMoveTarget) + rotateVector * 32 * 5;
		BWAPI::Position te = double2(baseMoveTarget) - rotateVector * 32 * 5;
		BWAPI::Broodwar->drawLineMap(pathLeader->getPosition().x, pathLeader->getPosition().y, baseMoveTarget.x, baseMoveTarget.y, BWAPI::Colors::Red);
		BWAPI::Broodwar->drawLineMap(ta.x, ta.y, te.x, te.y, BWAPI::Colors::Red);

		double maxLineLength = 5 * 32;
		double curLineLength = maxLineLength;
		BWAPI::Position lineLeftStart;
		BWAPI::Position lineRightStart;
		BWAPI::Position curPosition;

		int atPositionCount = 0;

		for (auto armyType : movePositionOrder)
		{
			int armySize = armyType.width() > armyType.height() ? armyType.width() : armyType.height();
			bool isFlyer = armyType.isFlyer();
			for (auto u : tacticArmy[armyType]->getUnits())
			{
				if (u.unit == pathLeader)
					continue;

				bool backPositionValid = true;
				//find next line position
				if (curLineLength >= lineLeftStart.getDistance(lineRightStart))
				{
					while (true)
					{
						baseMoveTarget += backVector * armySize;
						if (!isPositionValid(baseMoveTarget))
						{
							backPositionValid = false;
							break;
						}
						if (BWAPI::Broodwar->isWalkable(BWAPI::WalkPosition(baseMoveTarget.x / 8, baseMoveTarget.y / 8)) || isFlyer)
						{
							break;
						}
					}
					if (backPositionValid)
					{
						BWAPI::Position rightEnd = baseMoveTarget;
						int tmpLength = 0;
						int checkSpan = 8;
						for (BWAPI::Position start = baseMoveTarget; tmpLength < maxLineLength; start = start + rotateVector * checkSpan)
						{
							if (isPositionValid(start) && (BWAPI::Broodwar->isWalkable(BWAPI::WalkPosition(start.x / 8, start.y / 8)) || onlyFlyer))
							{
								rightEnd = start;
								tmpLength += checkSpan;
							}
							else
								break;
						}

						BWAPI::Position leftEnd = baseMoveTarget;
						tmpLength = 0;
						for (BWAPI::Position start = baseMoveTarget; tmpLength < maxLineLength; start = start - rotateVector * checkSpan)
						{
							if (isPositionValid(start) && (BWAPI::Broodwar->isWalkable(BWAPI::WalkPosition(start.x / 8, start.y / 8)) || onlyFlyer))
							{
								leftEnd = start;
								tmpLength += checkSpan;
							}
							else
								break;
						}

						lineLeftStart = leftEnd;
						lineRightStart = rightEnd;

						curPosition = lineLeftStart;
						curLineLength = 0;
						
					}
				}

				if (backPositionValid)
				{
					curPosition += rotateVector * armySize;
					curLineLength += armySize;
					BattleArmy::smartMove(u.unit, curPosition);
				}
				else
				{
					BattleArmy::smartMove(u.unit, pathLeader->getPosition());
				}

			}
		}
	}
	else
	{
		BattleArmy::smartMove(pathLeader, destination);
	}

	
}


void BattleTactic::flockingMove(BWAPI::Position destination)
{
	if (BWAPI::Broodwar->getFrameCount() % 10 != 0)
		return;

	BWAPI::Unit firstUnit = NULL;
	double lowestSpeed = 9999;
	std::map<BWTA::Region*, BWAPI::Position> regionPath;
	double2 centerPosition;
	int count = 0;
	for (auto army : tacticArmy)
	{
		for (auto u : army.second->getUnits())
		{
			centerPosition = centerPosition + double2(u.unit->getPosition());
			count++;
			if (regionPath.find(BWTA::getRegion(u.unit->getPosition())) == regionPath.end())
			{
				std::list<BWAPI::Position> path = aStarGroundPathFinding(u.unit->getPosition(), destination);
				regionPath[BWTA::getRegion(u.unit->getPosition())] = path.front();
			}

			if (army.first != BWAPI::UnitTypes::Zerg_Overlord && army.second->getUnits().size() > 0)
			{
				firstUnit = army.second->getUnits().front().unit;
				if (army.first.topSpeed() < lowestSpeed)
				{
					lowestSpeed = army.first.topSpeed();
				}
			}
		}
	}

	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();

	double2 cohesion;
	double2 seperation;
	double2 alignment;

	double2 goal;
	double2 enemy;

	for (auto army : tacticArmy)
	{
		for (auto u : army.second->getUnits())
		{
			double2 distanceToTarget = regionPath[BWTA::getRegion(u.unit->getPosition())] - u.unit->getPosition();
			double2 currentVelocity = double2(u.unit->getVelocityX(), u.unit->getVelocityY()); //distanceToTarget.normal();

			BWAPI::Unitset nearByUnits = u.unit->getUnitsInRadius(32 * 6, BWAPI::Filter::IsOwned);
			cohesion = getUnitCohesion(u.unit, nearByUnits, currentVelocity);
			//seperation = getUnitSeperation(u.unit, currentVelocity);
			//alignment = getUnitAlignment(u.unit, nearByUnits, currentVelocity);

			goal = getUnitGoal(u.unit, regionPath[BWTA::getRegion(u.unit->getPosition())], currentVelocity);
			//enemy = getUnitNearEnemy(u.unit, influnceMap, currentVelocity);

			double2 targetVelocity =  goal;
			targetVelocity = targetVelocity.normal();
			double currentSpeed = 0;//currentVelocity.len();

			double curLowSpeed = lowestSpeed > currentVelocity.len() ? currentVelocity.len() : lowestSpeed;

			if (army.first == BWAPI::UnitTypes::Zerg_Ultralisk)
				currentSpeed = lowestSpeed * 1.3;
			else if (army.first == BWAPI::UnitTypes::Zerg_Zergling)
				currentSpeed = lowestSpeed * 1.2;
			else if (army.first == BWAPI::UnitTypes::Zerg_Mutalisk)
				currentSpeed = lowestSpeed * 1.1;
			else
				currentSpeed = lowestSpeed;
			currentSpeed = currentSpeed > army.first.topSpeed() ? army.first.topSpeed() : currentSpeed;
			currentSpeed = currentSpeed * 20;
			BWAPI::Position nextMovePosition = u.unit->getPosition() + targetVelocity * currentSpeed;

			nextMovePosition.x = nextMovePosition.x > BWAPI::Broodwar->mapWidth() * 32 - 1 ? BWAPI::Broodwar->mapWidth() * 32 - 1 : nextMovePosition.x;
			nextMovePosition.x = nextMovePosition.x < 0 ? 0 : nextMovePosition.x;
			nextMovePosition.y = nextMovePosition.y > BWAPI::Broodwar->mapHeight() * 32 - 1 ? BWAPI::Broodwar->mapHeight() * 32 - 1 : nextMovePosition.y;
			nextMovePosition.y = nextMovePosition.y < 0 ? 0 : nextMovePosition.y;
			BattleArmy::smartMove(u.unit, nextMovePosition);
		}
	}
}


double2	BattleTactic::getUnitCohesion(BWAPI::Unit u, BWAPI::Unitset nearbyUnits, double2 currentVelocity)
{
	double2 totalPosition;
	for (auto unit : nearbyUnits)
	{
		if (unit != u)
		{
			totalPosition = totalPosition + double2(unit->getPosition());
		}
		else
		{
			continue;
		}
	}
	if (nearbyUnits.size() > 0)
	{
		totalPosition = totalPosition / nearbyUnits.size();
		double2 targetVelocity = double2(totalPosition - double2(u->getPosition())).normal() * 2;
		return targetVelocity - currentVelocity;
	}
	else
	{
		return double2(0, 0);
	}
}


double2	BattleTactic::getUnitSeperation(BWAPI::Unit u)
{
	BWAPI::Unitset nearByUnits = u->getUnitsInRadius(8, BWAPI::Filter::IsOwned);
	double2 totalSepration(0, 0);
	for (auto unit : nearByUnits)
	{
		if (unit != u)
		{
			double2 distance = double2(u->getPosition()) - double2(unit->getPosition());
			totalSepration = totalSepration + distance.normal() / distance.len();
		}
	}

	return totalSepration;
}


double2	BattleTactic::getUnitAlignment(BWAPI::Unit u, BWAPI::Unitset nearbyUnits, double2 currentVelocity)
{
	double2 totalAlignment;
	for (auto unit : nearbyUnits)
	{
		if (unit != u)
		{
			totalAlignment = totalAlignment + double2(unit->getVelocityX(), unit->getVelocityY());
		}
	}

	if (nearbyUnits.size() > 0)
	{
		totalAlignment = totalAlignment.normal() * 2;
		return totalAlignment - currentVelocity;
	}
	else
	{
		return double2(0, 0);
	}
}


double2	BattleTactic::getUnitGoal(BWAPI::Unit u, BWAPI::Position goal, double2 currentVelocity)
{
	//BWAPI::Position chokePosition = BWTA::getNearestChokepoint(u->getPosition())->getCenter();
	double2 targetVelocity = double2(goal) - double2(u->getPosition());
	targetVelocity = targetVelocity.normal() / targetVelocity.len();

	return targetVelocity;
}


double2	BattleTactic::getUnitNearEnemy(BWAPI::Unit u, std::vector<std::vector<gridInfo>>& influnceMap, double2 currentVelocity)
{
	double2 totalEnemy;
	BWAPI::TilePosition tp = u->getTilePosition();
	int searchRadius = 3;
	for (int i = -1 * searchRadius; i <= searchRadius; i++)
	{
		for (int j = -1 * searchRadius; j <= searchRadius; j++)
		{
			if (tp.x + i > BWAPI::Broodwar->mapWidth() * 32 - 1 || tp.x + i < 0
				|| tp.y + j > BWAPI::Broodwar->mapHeight() * 32 - 1 || tp.y + j < 0)
			{
				continue;
			}

			if (influnceMap[tp.x + i][tp.y + j].groundForce > 0 || influnceMap[tp.x + i][tp.y + j].airForce > 0)
			{
				totalEnemy = totalEnemy + (double2(u->getPosition()) - double2((tp.x + i) * 32, (tp.y + j) * 32));
			}
		}
	}

	return totalEnemy.normal();
}



bool BattleTactic::reGroup(BWAPI::Position & regroupPosition) const
{
	typedef std::pair<BWAPI::UnitType, BattleArmy*> mapType;

	bool isGroup = true;
	BOOST_FOREACH(mapType p, tacticArmy)
	{
		if (p.second->getUnits().size() == 0)
			continue;
		if (!p.second->reGroup(regroupPosition))
			isGroup = false;
	}
	return isGroup;
}

void BattleTactic::retreatSet()
{
	if (state == RETREAT)
		return;

	state = RETREAT;
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	retreatTime = BWAPI::Broodwar->getFrameCount() + 10 * 25;
	if (myRegions.size() > 1)
		nextRetreatPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		nextRetreatPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
}

void BattleTactic::checkStartPositionEnemyInfo()
{
	int mutaliskCount = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
	
	if (mutaliskCount == 0)
	{
		startEnemyCanAttack = false;
	}
	
	BWAPI::Unit firstUnit = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit;

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());

	std::set<BWAPI::Unit> friendUnitNearBy;
	
	BWAPI::Unitset tmp = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit->getUnitsInRadius(12 * 32);
	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Mutalisk && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}
	
	BOOST_FOREACH(BWAPI::Unit u, friendUnitNearBy)
	{
		ourSupply += u->getType().supplyRequired();
	}

	if (ourSupply <= 24)
	{
		ourSupply = int(ourSupply * 0.6);
	}
	else if (ourSupply > 24 && ourSupply <= 48)
	{
		ourSupply = int(ourSupply * 0.7);
	}
	else if (ourSupply > 48 && ourSupply <= 100)
	{
		ourSupply = int(ourSupply * 0.9);
	}
	else
	{
		ourSupply = int(ourSupply * 1.1);
	}
	
	
	std::set<BWAPI::Unit> nearbyUnits;

	for (auto u : tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
		nearbyUnits.insert(percentUnits.begin(), percentUnits.end());
	}
	
	int inRegionEnemySupply = 0;
	int cannonSupply = 0;
	BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && (u->getType().airWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			if (u->getType().isBuilding() && u->isCompleted())
			{
				if (u->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					inRegionEnemySupply += 12;
					cannonSupply += 12;
				}
				else
				{
					inRegionEnemySupply += 8;
					cannonSupply += 8;
				}
			}
			else
			{
				inRegionEnemySupply += u->getType().supplyRequired();
			}
		}
	}

	int lowHealthCount = 0;
	BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		if (u.unit->getHitPoints() < u.unit->getType().maxHitPoints() / 3)
		{
			lowHealthCount++;
		}
	}
	
	int enemySupply = inRegionEnemySupply;
	
	if (ourSupply * 0.9 < enemySupply
	|| (lowHealthCount >= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * 0.7 && ourSupply * 0.2 <= enemySupply))
	{
		startEnemyCanAttack = false;
	}
	else
	{
		startEnemyCanAttack = true;
	}
		
	
}


void BattleTactic::setGroupPosition(BWAPI::UnitType armyType, tacticType tactic)
{
	return;
}

void BattleTactic::setAttackPosition(BWAPI::Position targetPosition, tacticType tactic)
{
	attackPosition = originAttackBase = targetPosition;
	if (tactic == HydraliskPushTactic)
	{
		if (BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation())->isReachable(BWTA::getRegion(targetPosition)) == false)
		{
			state = END;
			return;
		}
	}

	else if (tactic == DefendTactic)
	{
		if (BWTA::getRegion(attackPosition) != BWTA::getRegion(InformationManager::Instance().getOurBaseLocation())
			&& BWTA::getRegion(attackPosition) != BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation()))
		{
			state = GROUPARMY;
		}
		else
		{
			state = BASEGROUP;
		}
	}
}


bool BattleTactic::hasEnemy()
{
	if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
		return true;

	bool isAllAntiAirUnit = true;
	bool isAllAntiGroundUnit = true;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord && army.second->getUnits().size() > 0)
		{
			if (army.first.groundWeapon() != BWAPI::WeaponTypes::None)
			{
				isAllAntiAirUnit = false;
			}
			if (army.first.airWeapon() != BWAPI::WeaponTypes::None)
			{
				isAllAntiGroundUnit = false;
			}
		}
	}

	// if current attack position has target, do not change original attack position
	BWAPI::Unitset& units = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 12 * 32, BWAPI::Filter::IsEnemy);
	bool needAttack = false;
	BOOST_FOREACH(BWAPI::Unit u, units)
	{
		if (BWTA::getRegion(u->getPosition()) == BWTA::getRegion(attackPosition) && u->getType() != BWAPI::UnitTypes::Protoss_Observer)
		{
			if (u->getType().isFlyer() && isAllAntiGroundUnit)
				continue;
			if (!u->getType().isFlyer() && isAllAntiAirUnit)
				continue;

			if (!u->isDetected() && u->isVisible())
				continue;
			return true;
		}
	}

	// no enemy in attackPosition circle, looking for remaining building in the region
	if (needAttack == false && !isAllAntiAirUnit)
	{
		std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
		if (occupiedDetail.find(BWTA::getRegion(originAttackBase)) != occupiedDetail.end())
		{
			std::map<BWAPI::Unit, buildingInfo >& buildingsDetail = occupiedDetail[BWTA::getRegion(originAttackBase)];
			for (std::map<BWAPI::Unit, buildingInfo >::iterator it = buildingsDetail.begin(); it != buildingsDetail.end(); it++)
			{
				attackPosition = BWAPI::Position(it->second.initPosition);
				return true; 
			}
		}
	}

	return false;
}



bool BattleTactic::unitNearChokepoint(BWAPI::Unit unit) const
{
	BOOST_FOREACH(BWTA::Chokepoint * choke, BWTA::getChokepoints())
	{
		if (unit->getDistance(choke->getCenter()) < 80)
		{
			return true;
		}
	}

	return false;
}

void BattleTactic::addArmyUnit(BWAPI::Unit unit)	
{
	newAddArmy[unit] = std::vector<BWAPI::Position>();
}


void BattleTactic::newArmyRally()
{
	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end();)
	{
		if (it->second.size() == 0)
		{
			if (tacticArmy.find(it->first->getType()) == tacticArmy.end())
			{
				it++;
				continue;
			}

			tacticArmy[it->first->getType()]->addUnit(it->first);
			newAddArmy.erase(it++);
		}
		else
		{
			if (it->first->getType() == BWAPI::UnitTypes::Zerg_Lurker)
			{
				BattleArmy::smartMove(it->first, *it->second.begin());
			}
			else
			{
				BattleArmy::smartAttackMove(it->first, *it->second.begin());
			}
			if (it->first->getDistance((*it->second.begin())) < 32 * 12)
			{
				it->second.erase(it->second.begin());
			}
			it++;
		}
		/*
		if (state != RETREAT)
		{
			if (it->second.size() == 0)
			{
				tacticArmy[it->first->getType()]->addUnit(it->first);
				newAddArmy.erase(it++);
			}
			else
			{
				it->first->attack((*it->second.begin()));
				if (it->first->getDistance((*it->second.begin())) < 32 * 4)
				{
					it->second.erase(it->second.begin());
				}
				it++;
			}
		}
		else
			it++;*/
	}
}


BattleTactic::BattleTactic()
{
	tacticArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Lurker] = new LurkerArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Scourge] = new ScourgeArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Ultralisk] = new UltraliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Devourer] = new DevourerArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Guardian] = new GuardianArmy();

	attackOrder = { BWAPI::UnitTypes::Zerg_Scourge, BWAPI::UnitTypes::Zerg_Devourer, BWAPI::UnitTypes::Zerg_Ultralisk, 
		BWAPI::UnitTypes::Zerg_Lurker, BWAPI::UnitTypes::Zerg_Zergling, BWAPI::UnitTypes::Zerg_Guardian, 
		BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::UnitTypes::Zerg_Mutalisk };

	movePositionOrder = { BWAPI::UnitTypes::Zerg_Ultralisk, BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Lurker, BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::UnitTypes::Zerg_Devourer,
		BWAPI::UnitTypes::Zerg_Guardian, BWAPI::UnitTypes::Zerg_Scourge };

	endByDefend = false;
	groupStartTime = 0;
	groupStatus = false;

	BWAPI::Position rallyPosition;
	std::set<BWTA::Region *>& myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (myRegions.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
		rallyPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		rallyPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	groupPosition = rallyPosition;
	leader = NULL;

	needMove = true;
	curStopTime = 0;
	curMoveTime = 0;
}

BattleTactic::~BattleTactic()
{
	typedef std::pair<BWAPI::UnitType, BattleArmy*> mapType;
	BOOST_FOREACH(mapType p, tacticArmy)
	{
		delete p.second;
	}
	//tactic end
}


BWAPI::Position BattleTactic::groupArmyCheck(bool& needGroup)
{
	int avgX = 0;
	int avgY = 0;
	int totalCount = 0;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				totalCount += 1;
				avgX += u.unit->getPosition().x;
				avgY += u.unit->getPosition().y;
			}
		}
	}

	avgX = avgX / totalCount;
	avgY = avgY / totalCount;
	BWAPI::Position centerPoint(avgX, avgY);

	int distancethreshhold = ((totalCount / 12) + 3) * 32;
	int groupCount = 0;
	int minDistance = 9999999;
	BWAPI::Position minPosition = BWAPI::Positions::None;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (u.unit->getDistance(centerPoint) <= distancethreshhold)
				{
					groupCount += 1;
				}

				if (u.unit->getDistance(centerPoint) < minDistance)
				{
					minDistance = u.unit->getDistance(centerPoint);
					minPosition = u.unit->getPosition();
				}
			}
		}
	}

	BWAPI::Broodwar->drawCircleMap(minPosition.x, minPosition.y, distancethreshhold, BWAPI::Colors::Red, false);

	if (groupCount * 10 / totalCount >= 9)
	{
		needGroup = false;
	}
	else
	{
		needGroup = true;
	}

	return minPosition;
}


bool BattleTactic::checkGroupStatus(BWAPI::Position target)
{
	int totalCount = 0;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				totalCount += 1;
			}
		}
	}
	int groupCount = 0;

	if (totalCount <= 50)
	{
		int distancethreshhold = ((totalCount / 12) + 3) * 32;
		BWAPI::Broodwar->drawCircleMap(target.x, target.y, distancethreshhold, BWAPI::Colors::Red, false);

		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				for (auto u : army.second->getUnits())
				{
					if (u.unit->getDistance(groupPosition) <= distancethreshhold)
					{
						groupCount += 1;
					}
				}
			}
		}

		if (groupCount * 10 / totalCount >= 8)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				for (auto u : army.second->getUnits())
				{
					if (BWTA::getRegion(u.unit->getPosition()) == BWTA::getRegion(target))
					{
						groupCount += 1;
					}
				}
			}
		}
		if (groupCount * 10 / totalCount >= 7)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void BattleTactic::addAllNewArmy()
{
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	BWAPI::Position moveBackBase;
	if (myRegions.size() > 1)
		moveBackBase = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		moveBackBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end();)
	{
		if (tacticArmy.find(it->first->getType()) == tacticArmy.end())
		{
			it++;
			continue;
		}

		it->first->move(moveBackBase);
		tacticArmy[it->first->getType()]->addUnit(it->first);
		newAddArmy.erase(it++);
	}
}


void BattleTactic::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL || tacticArmy.find(unit->getType()) == tacticArmy.end())
		return;

	tacticArmy[unit->getType()]->removeUnit(unit);

	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
	{
		if (it->first == unit)
		{
			newAddArmy.erase(it);
			break;
		}
	}
}

void BattleTactic::onLurkerMorph()
{
	std::vector<UnitState>& army = tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits();
	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit->isMorphing())
		{
			army.erase(it);
			break;
		}
	}

	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
	{
		if (it->first->isMorphing() && it->first->getBuildType() == BWAPI::UnitTypes::Zerg_Lurker)
		{
			newAddArmy.erase(it);
			break;
		}
	}
}



