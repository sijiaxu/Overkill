#pragma once
#include "Common.h"
#include "InformationManager.h"
#include "TimeManager.cpp"


struct wayPoint
{
	BWAPI::TilePosition wayPosition;
	BWAPI::TilePosition targetPosition;
	BWAPI::TilePosition parentPosition;

	int cumulativeValue;
	int heuristicValue;

	wayPoint(BWAPI::TilePosition position, BWAPI::TilePosition target)
	{
		wayPosition = position;
		targetPosition = target;
		cumulativeValue = 0;
		heuristicValue = int(wayPosition.getDistance(targetPosition));
	}

	wayPoint(BWAPI::TilePosition position)
	{
		wayPosition = position;
	}

	wayPoint(const wayPoint& p)
	{
		wayPosition = p.wayPosition;
		targetPosition = p.targetPosition;
		parentPosition = p.parentPosition;
		cumulativeValue = p.cumulativeValue;
		heuristicValue = p.heuristicValue;
	}

	void setParent(wayPoint& p)
	{
		cumulativeValue = p.cumulativeValue + int(wayPosition.getDistance(p.wayPosition));
		parentPosition = p.wayPosition;
	}

	void checkParent(wayPoint& p)
	{
		int curCumulativeValue = p.cumulativeValue + int(wayPosition.getDistance(p.wayPosition));
		if (curCumulativeValue < cumulativeValue)
		{
			cumulativeValue = curCumulativeValue;
			parentPosition = p.wayPosition;
		}
	}

	bool operator < (const wayPoint& u) const
	{
		if (wayPosition.y() < u.wayPosition.y())
			return true;
		else if (wayPosition.y() > u.wayPosition.y())
			return false;
		else
			if (wayPosition.x() < u.wayPosition.x())
				return true;
			else
				return false;
	}
};


std::list<BWAPI::TilePosition> aStarPathFinding(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition, bool isFlyer, bool nearEndPosition = false);
