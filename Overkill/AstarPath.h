#pragma once
#include "Common.h"
#include "InformationManager.h"
#include "TimeManager.cpp"
#include <unordered_map>
#include <queue>


struct fValueGridPoint
{
	BWAPI::TilePosition position;
	double fValue;

	fValueGridPoint(BWAPI::TilePosition p, double f)
	{
		position = p;
		fValue = f;
	}

	bool operator < (const fValueGridPoint& g) const
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
