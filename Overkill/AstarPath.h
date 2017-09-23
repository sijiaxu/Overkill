#pragma once
#include "TimeManager.cpp"
#include <unordered_map>
#include <queue>


template <typename pType>
struct fValueGridPoint
{
	pType position;
	double fValue;

	fValueGridPoint(pType p, double f)
	{
		position = p;
		fValue = f;
	}

	bool operator < (const fValueGridPoint<pType> & g) const
	{
		if (fValue > g.fValue)
			return true;
		else
			return false;
	}
};



struct costGridPoint
{
	BWAPI::TilePosition position;
	double costValue;

	costGridPoint(BWAPI::TilePosition p, double f)
	{
		position = p;
		costValue = f;
	}
};


std::list<BWAPI::TilePosition> aStarPathFinding(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition, bool isFlyer, bool nearEndPosition = false);

std::list<BWAPI::Position> aStarGroundPathFinding(BWAPI::Position startPosition, BWAPI::Position endPosition);


