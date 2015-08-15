#include "AstarPath.h"


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

	std::set<wayPoint> openPoints;
	std::set<wayPoint> closePoints;

	wayPoint startPoint(startPosition, endPosition);
	openPoints.insert(startPoint);

	BWAPI::TilePosition backPosition;

	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();

	double farest = 0;
	BWTA::Polygon p = BWTA::getRegion(endPosition)->getPolygon();
	for (int j = 0; j < (int)p.size(); j++)
	{
		double distance = endPosition.getDistance(BWAPI::TilePosition(p[j]));
		if (distance > farest)
		{
			farest = distance;
		}
	}
	int expandRate;

	TimerManager::Instance().startTimer(TimerManager::Astar1);

	int loop_time1 = 0;
	int loop_time2 = 0;
	int loop_time3 = 0;
	int canNotFind = 0;

	// if we expand the end, we find the optimal path
	while (openPoints.size() > 0)
	{
		loop_time1++;

		std::set<wayPoint>::iterator minPoint;
		int minFvalue = 9999999;
		double minEnemyForce = 9999;
		double enemyForce;
		for (std::set<wayPoint>::iterator it = openPoints.begin(); it != openPoints.end(); it++)
		{
			loop_time2++;

			if (isFlyer)
				enemyForce = influnceMap[it->wayPosition.y()][it->wayPosition.x()].airForce + influnceMap[it->wayPosition.y()][it->wayPosition.x()].decayAirForce + influnceMap[it->wayPosition.y()][it->wayPosition.x()].enemyUnitAirForce;
			else
				enemyForce = influnceMap[it->wayPosition.y()][it->wayPosition.x()].groundForce + influnceMap[it->wayPosition.y()][it->wayPosition.x()].decayGroundForce + influnceMap[it->wayPosition.y()][it->wayPosition.x()].enemyUnitGroundForce;
			if ((enemyForce < minEnemyForce) || (enemyForce == minEnemyForce && it->cumulativeValue + it->heuristicValue < minFvalue))
			{
				minEnemyForce = enemyForce;
				minFvalue = it->cumulativeValue + it->heuristicValue;
				minPoint = it;
			}
		}

		wayPoint currentMinPoint = *minPoint;
		openPoints.erase(minPoint);
		closePoints.insert(currentMinPoint);

		if (!nearEndPosition)
		{
			if (BWTA::getRegion(currentMinPoint.wayPosition) == BWTA::getRegion(endPosition))
			{
				backPosition = currentMinPoint.wayPosition;
				break;
			}
		}
		else 
		{
			if (currentMinPoint.wayPosition.getDistance(endPosition) < 20)
			{
				backPosition = currentMinPoint.wayPosition;
				break;
			}
		}
		
		
		if (BWAPI::Broodwar->mapWidth() <= 128 && BWAPI::Broodwar->mapHeight() <= 128)
		{
			expandRate = 4;
		}
		else
		{
			expandRate = 8;
		}
		
		for (int i = 0; i < int(directions.size()); i++)
		{
			loop_time3++;

			BWAPI::TilePosition nextTilePosition(currentMinPoint.wayPosition.x() + int(directions[i].x * expandRate), currentMinPoint.wayPosition.y() + int(directions[i].y * expandRate));

			if (nextTilePosition.x() > BWAPI::Broodwar->mapWidth() - 1 || nextTilePosition.x() < 0 
				|| nextTilePosition.y() > BWAPI::Broodwar->mapHeight() - 1 || nextTilePosition.y() < 0)
			{
				continue;
			}

			wayPoint nextPoint(nextTilePosition, endPosition);

			//if point have already been expand, skip it 
			if (closePoints.find(nextPoint) != closePoints.end())
				continue;
			// if point have already in open point, check the f value
			if (openPoints.find(nextPoint) != openPoints.end())
			{
				openPoints.find(nextPoint)->checkParent(currentMinPoint);
			}
			else
			{
				nextPoint.setParent(currentMinPoint);
				openPoints.insert(nextPoint);
			}
		}

		//can not find the solution path
		if (openPoints.size() == 0)
		{
			canNotFind = 1;
			return std::list<BWAPI::TilePosition>();
		}
	}
	/*
	BWAPI::Broodwar->printf("loop1 : %d", loop_time1);
	BWAPI::Broodwar->printf("loop2 : %d", loop_time2);
	BWAPI::Broodwar->printf("loop3 : %d", loop_time3);
	BWAPI::Broodwar->printf("can not find : %d", canNotFind);
	*/


	std::list<BWAPI::TilePosition> pathFind;

	//pathFind.push_front(endPosition);

	while (backPosition != startPosition)
	{
		wayPoint temp(backPosition);
		std::set<wayPoint>::iterator parent = closePoints.find(temp);
		pathFind.push_front(parent->wayPosition);
		backPosition = parent->parentPosition;
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
					double2 firstVector(it2->x() - it->x(), it2->y() - it->y());
					double2 secondVector(it3->x() - it2->x(), it3->y() - it2->y());
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

	TimerManager::Instance().stopTimer(TimerManager::Astar1);

	return pathFind;
}


