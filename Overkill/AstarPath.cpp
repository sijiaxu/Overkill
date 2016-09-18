#include "AstarPath.h"


double diag_distance(double2 pos)
{
	double d = std::abs(pos.x) > std::abs(pos.y) ? std::abs(pos.y) : std::abs(pos.x);
	double s = std::abs(pos.x) + std::abs(pos.y);

	return d*1.4142135623730950488016887242097 + (s - d * 2);
}


// for attack path finding
std::list<BWAPI::TilePosition> aStarPathFinding(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition, bool isFlyer, bool nearEndPosition)
{
	std::vector<double2> directions;
	directions.reserve(8);

	double2 direct1(1, 0);
	directions.push_back(direct1);
	double2 direct2(1, 1);
	directions.push_back(direct2);
	double2 direct3(0, 1);
	directions.push_back(direct3);
	double2 direct4(-1, 1);
	directions.push_back(direct4);

	double2 direct5(-1, 0);
	directions.push_back(direct5);
	double2 direct6(-1, -1);
	directions.push_back(direct6);
	double2 direct7(0, -1);
	directions.push_back(direct7);
	double2 direct8(1, -1);
	directions.push_back(direct8);

	int width = BWAPI::Broodwar->mapWidth();
	int heigth = BWAPI::Broodwar->mapHeight();
 

	std::priority_queue<fValueGridPoint> openList;
	//std::tr1::unordered_map<BWAPI::TilePosition, double> openListIndex;
	std::vector<std::vector<double>> openListIndex;
	openListIndex.resize(BWAPI::Broodwar->mapWidth(), std::vector<double>(BWAPI::Broodwar->mapHeight(), -1));

	double hValue = diag_distance(double2(endPosition.x - startPosition.x, endPosition.y - startPosition.y));
	openList.push(fValueGridPoint(startPosition, hValue));
	openListIndex[startPosition.x][startPosition.y] = 0;

	//std::tr1::unordered_map<BWAPI::TilePosition, BWAPI::TilePosition> backtraceList;
	std::vector<std::vector<BWAPI::TilePosition>> backtraceList;
	backtraceList.resize(BWAPI::Broodwar->mapWidth(), std::vector<BWAPI::TilePosition>(BWAPI::Broodwar->mapHeight(), BWAPI::TilePositions::None));

	//std::tr1::unordered_map<BWAPI::TilePosition, double> closeListIndex;
	std::vector<std::vector<double>> closeListIndex;
	closeListIndex.resize(BWAPI::Broodwar->mapWidth(), std::vector<double>(BWAPI::Broodwar->mapHeight(), -1));

	//backtraceList[startPosition] = startPosition;
	backtraceList[startPosition.x][startPosition.y] = startPosition;

	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
	int expandRate;
	BWAPI::TilePosition backPosition = BWAPI::TilePositions::None;

	//TimerManager::Instance().startTimer(TimerManager::Astar1);

	int canNotFind = 0;
	if (BWAPI::Broodwar->mapWidth() <= 128 && BWAPI::Broodwar->mapHeight() <= 128)
	{
		if (isFlyer)
			expandRate = 4;
		else
			expandRate = 1;
	}
	else
	{
		if (isFlyer)
			expandRate = 8;
		else
			expandRate = 1;
	}

	int loop1 = 0;
	int loop2 = 0;
	

	// if we expand the end, we find the optimal path
	while (openList.size() > 0)
	{
		//TimerManager::Instance().stopTimer(TimerManager::Astar1);
		double elapseTime = TimerManager::Instance().getElapseTime(TimerManager::Astar1);
		if (elapseTime > 50)
		{
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}
		loop1++;

		fValueGridPoint current = openList.top();
		closeListIndex[current.position.x][current.position.y] = openListIndex[current.position.x][current.position.y];
		openList.pop();
		openListIndex[current.position.x][current.position.y] = -1;
		
		if (!nearEndPosition)
		{
			if (BWTA::getRegion(current.position) == BWTA::getRegion(endPosition))
			{
				backPosition = current.position;
				break;
			}
		}
		else 
		{
			if (current.position.getDistance(endPosition) < 6)
			{
				backPosition = current.position;
				break;
			}
		}
		
		for (int i = 0; i < int(directions.size()); i++)
		{
			BWAPI::TilePosition nextTilePosition(current.position.x + int(directions[i].x * expandRate), current.position.y + int(directions[i].y * expandRate));

			if (nextTilePosition.x > BWAPI::Broodwar->mapWidth() - 1 || nextTilePosition.x < 0 
				|| nextTilePosition.y > BWAPI::Broodwar->mapHeight() - 1 || nextTilePosition.y < 0)
			{
				continue;
			}

			if (!isFlyer)
			{
				if (!BWAPI::Broodwar->isWalkable(nextTilePosition.x * 4, nextTilePosition.y * 4))
					continue;
				/*
				if (nextTilePosition.getDistance(BWAPI::TilePosition(BWTA::getNearestChokepoint(nextTilePosition)->getCenter())) <= 5
					&& BWTA::getNearestChokepoint(nextTilePosition)->getWidth() <= 5 * 32)
				{
					if (!BWAPI::Broodwar->isWalkable(nextTilePosition.x * 4, nextTilePosition.y * 4))
						continue;
				}
				else
				{
					//TODO can not justify the mineral and gas position
					bool isNearbyUnwalkable = false;
					for (int x = nextTilePosition.x - 2; x <= nextTilePosition.x + 2; x++)
					{
						for (int y = nextTilePosition.y - 2; y <= nextTilePosition.y + 2; y++)
						{
							if (!BWAPI::Broodwar->isWalkable(x * 4, y * 4))
							{
								isNearbyUnwalkable = true;
								break;
							}
						}
					}

					if (isNearbyUnwalkable)
						continue;
				}*/
			}
			

			double newCost = 0;
			if (isFlyer)
				newCost = openListIndex[current.position.x][current.position.y] + expandRate + (influnceMap[nextTilePosition.x][nextTilePosition.y].airForce +
				influnceMap[nextTilePosition.x][nextTilePosition.y].decayAirForce + influnceMap[nextTilePosition.x][nextTilePosition.y].enemyUnitAirForce +
				influnceMap[nextTilePosition.x][nextTilePosition.y].enemyUnitDecayAirForce) * expandRate * 100;
			else
				newCost = openListIndex[current.position.x][current.position.y] + expandRate; /*+ (influnceMap[nextTilePosition.x][nextTilePosition.y].groundForce +
				influnceMap[nextTilePosition.x][nextTilePosition.y].decayGroundForce + influnceMap[nextTilePosition.x][nextTilePosition.y].enemyUnitGroundForce +
				influnceMap[nextTilePosition.x][nextTilePosition.y].enemyUnitDecayGroundForce) * expandRate;*/

			//if point have already been expand(if our heuristic function is admissible, this will not happen)
			if (closeListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
				closeListIndex[nextTilePosition.x][nextTilePosition.y] = -1;
			// if point have already in open point, check the cost value
			// since std::priority_queue do not have increase-priority interface, so we push the same position twice into the open list 
			if (openListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && openListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
				openListIndex[nextTilePosition.x][nextTilePosition.y] = -1;

			if (openListIndex[nextTilePosition.x][nextTilePosition.y] == -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] == -1)
			{
				loop2++;
				double fvalue;
				fvalue = newCost + diag_distance(double2(endPosition.x - nextTilePosition.x, endPosition.y - nextTilePosition.y));

				openList.push(fValueGridPoint(nextTilePosition, fvalue));
				openListIndex[nextTilePosition.x][nextTilePosition.y] = newCost;
				backtraceList[nextTilePosition.x][nextTilePosition.y] = current.position;
			}
		}
		
		//can not find the solution path
		if (openList.size() == 0)
		{
			canNotFind = 1;
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}
	}
	//BWAPI::Broodwar->printf("loop1 %d", loop1);
	//BWAPI::Broodwar->printf("loop2 %d", loop2);


	std::list<BWAPI::TilePosition> pathFind;
	pathFind.push_front(backPosition);
	while (backPosition != startPosition)
	{
		pathFind.push_front(backtraceList[backPosition.x][backPosition.y]);
		backPosition = backtraceList[backPosition.x][backPosition.y];
	}

	if (isFlyer)
	{
		for (std::list<BWAPI::TilePosition>::iterator it = pathFind.begin(); it != pathFind.end();)
		{
			std::list<BWAPI::TilePosition>::iterator it2 = it;
			if (++it2 != pathFind.end())
			{
				std::list<BWAPI::TilePosition>::iterator it3 = it2;
				if (++it3 != pathFind.end())
				{
					double2 firstVector(it2->x - it->x, it2->y - it->y);
					double2 secondVector(it3->x - it2->x, it3->y - it2->y);
					double cosValue = (firstVector.x * secondVector.x + firstVector.y * secondVector.y) / (firstVector.len() * secondVector.len());
					double divergeDegree = std::acos(cosValue) * 180.0 / 3.14159265;

					//if three point are nearly in the same line, remove the middle point, making the flying unit move faster.
					if (divergeDegree < 10)
					{
						pathFind.erase(it2);
					}
					else
						it++;
				}
				else
					it++;
			}
			else
				it++;
		}
	}

	
	/*
	else
	{
		BWTA::Region* preRegion = BWTA::getRegion(*pathFind.begin());
		for (std::list<BWAPI::TilePosition>::iterator it = pathFind.begin(); it != pathFind.end();)
		{
			if (BWTA::getRegion(*it) != preRegion)
			{
				preRegion = BWTA::getRegion(*it);
				it++;
			}
			else
			{
				it = pathFind.erase(it);
			}
		}
	}*/

	//TimerManager::Instance().stopTimer(TimerManager::Astar1);

	return pathFind;
}


std::list<BWAPI::TilePosition> aStarGroundPathFinding(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition)
{
	TimerManager::Instance().startTimer(TimerManager::Astar1);

	if (BWTA::getRegion(startPosition) == BWTA::getRegion(endPosition) || BWTA::getRegion(startPosition) == NULL)
	{
		std::list<BWAPI::TilePosition> pathFind;
		pathFind.push_back(endPosition);
		return pathFind;
	}

	std::set<BWTA::Chokepoint*> allChokePoints = BWTA::getChokepoints();
	std::map<BWAPI::TilePosition, BWTA::Chokepoint*> positionAtChoke;
	for (auto c : allChokePoints)
	{
		positionAtChoke[BWAPI::TilePosition(c->getCenter())] = c;
	}

	std::priority_queue<fValueGridPoint> openList;
	std::vector<std::vector<double>> openListIndex;
	openListIndex.resize(BWAPI::Broodwar->mapWidth(), std::vector<double>(BWAPI::Broodwar->mapHeight(), -1));

	double hValue = diag_distance(double2(endPosition.x - startPosition.x, endPosition.y - startPosition.y));
	openList.push(fValueGridPoint(startPosition, hValue));
	openListIndex[startPosition.x][startPosition.y] = 0;

	std::vector<std::vector<BWAPI::TilePosition>> backtraceList;
	backtraceList.resize(BWAPI::Broodwar->mapWidth(), std::vector<BWAPI::TilePosition>(BWAPI::Broodwar->mapHeight(), BWAPI::TilePositions::None));

	std::vector<std::vector<double>> closeListIndex;
	closeListIndex.resize(BWAPI::Broodwar->mapWidth(), std::vector<double>(BWAPI::Broodwar->mapHeight(), -1));

	backtraceList[startPosition.x][startPosition.y] = startPosition;

	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
	BWAPI::TilePosition backPosition = BWAPI::TilePositions::None;



	int loop1 = 0;
	int loop2 = 0;

	TimerManager::Instance().startTimer(TimerManager::Astar1);
	// if we expand the end, we find the optimal path
	while (openList.size() > 0)
	{
		TimerManager::Instance().stopTimer(TimerManager::Astar1);
		double elapseTime = TimerManager::Instance().getElapseTime(TimerManager::Astar1);
		if (elapseTime > 2)
		{
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}

		loop1++;

		fValueGridPoint current = openList.top();
		closeListIndex[current.position.x][current.position.y] = openListIndex[current.position.x][current.position.y];
		openList.pop();
		openListIndex[current.position.x][current.position.y] = -1;

		std::vector<BWAPI::TilePosition> reachablePositions;
		//if current position is not at choke point
		if (positionAtChoke.find(current.position) == positionAtChoke.end())
		{
			const std::set<BWTA::Chokepoint*>& curRegionChokes = BWTA::getRegion(current.position)->getChokepoints();
			for (auto c : curRegionChokes)
			{
				reachablePositions.push_back(BWAPI::TilePosition(c->getCenter()));
			}
		}
		else
		{
			std::set<BWTA::Chokepoint*> candidateChoke;
			const std::pair<BWTA::Region*, BWTA::Region*>& connectedRegions = positionAtChoke[current.position]->getRegions();
			candidateChoke.insert(connectedRegions.first->getChokepoints().begin(), connectedRegions.first->getChokepoints().end());
			candidateChoke.insert(connectedRegions.second->getChokepoints().begin(), connectedRegions.second->getChokepoints().end());
			candidateChoke.erase(positionAtChoke[current.position]);

			for (auto c : candidateChoke)
			{
				reachablePositions.push_back(BWAPI::TilePosition(c->getCenter()));
			}

			if (connectedRegions.first == BWTA::getRegion(endPosition) || connectedRegions.second == BWTA::getRegion(endPosition))
			{
				backPosition = current.position;
				break;
			}
		}

		for (auto nextTilePosition : reachablePositions)
		{
			if (nextTilePosition.x > BWAPI::Broodwar->mapWidth() - 1 || nextTilePosition.x < 0
				|| nextTilePosition.y > BWAPI::Broodwar->mapHeight() - 1 || nextTilePosition.y < 0)
			{
				continue;
			}

			double newCost = 0;
			newCost = openListIndex[current.position.x][current.position.y] + current.position.getDistance(nextTilePosition);

			//if point have already been expand(if our heuristic function is admissible, this will not happen)
			//if (closeListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
				//closeListIndex[nextTilePosition.x][nextTilePosition.y] = -1;

			// if point have already in open point, check the cost value
			// since std::priority_queue do not have increase-priority interface, so we push the same position twice into the open list 
			if (openListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && openListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
				openListIndex[nextTilePosition.x][nextTilePosition.y] = -1;

			if (openListIndex[nextTilePosition.x][nextTilePosition.y] == -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] == -1)
			{
				loop2++;
				double fvalue;
				fvalue = newCost + diag_distance(double2(endPosition.x - nextTilePosition.x, endPosition.y - nextTilePosition.y));

				openList.push(fValueGridPoint(nextTilePosition, fvalue));
				openListIndex[nextTilePosition.x][nextTilePosition.y] = newCost;
				backtraceList[nextTilePosition.x][nextTilePosition.y] = current.position;
			}
		}

		//can not find the solution path
		if (openList.size() == 0)
		{
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}
	}
	
	std::list<BWAPI::TilePosition> pathFind;
	pathFind.push_front(backPosition);
	
	while (backPosition != startPosition)
	{
		TimerManager::Instance().stopTimer(TimerManager::Astar1);
		double elapseTime = TimerManager::Instance().getElapseTime(TimerManager::Astar1);
		if (elapseTime > 2)
		{
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}

		pathFind.push_front(backtraceList[backPosition.x][backPosition.y]);
		backPosition = backtraceList[backPosition.x][backPosition.y];
	}
	pathFind.push_back(endPosition);
	pathFind.erase(pathFind.begin());

	return pathFind;
}