#pragma once
#include "Common.h"
#include "BattleTactic.h"

#include "ZerglingArmy.h"



struct Scout{
	Scout(BWAPI::Unit u)
	{
		scoutUnit = u;
		TileTarget = BWAPI::Broodwar->self()->getStartLocation();
		nextMovePosition = BWAPI::TilePositions::None;
	}
	BWAPI::Unit scoutUnit;
	BWAPI::TilePosition TileTarget;
	BWAPI::TilePosition nextMovePosition;
	BWAPI::TilePosition naturalTileTarget;
};


struct scoutTarget
{
	BWAPI::TilePosition location;
	int					priority;

	scoutTarget(BWAPI::TilePosition t, int p)
	{
		location = t;
		priority = p;
	}

	bool operator < (const scoutTarget & u)
	{
		if (priority < u.priority)
			return true;
		else
			return false;
	}
};



class ScoutTactic : public BattleTactic
{
	enum ScoutState { scoutForEnemyBase = 0, scoutForEnemyInfo = 1 };
	ScoutState state;
	bool endFlag;

	std::vector<scoutTarget>	scoutLocation;
	std::vector<double2>		moveDirections;
	std::vector<Scout>			overLordIdle;
	std::vector<Scout>			overLordScouts;

	void						assignScoutWork();
	void						smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition) const;
	void						overlordMove(Scout& overlord);
	void						generateScoutLocation();

public:
	ScoutTactic();

	virtual void				update();
	virtual bool				isTacticEnd();
	virtual void				onUnitDestroy(BWAPI::Unit unit);
	virtual void				addArmyUnit(BWAPI::Unit unit) { overLordIdle.push_back(Scout(unit)); }
	bool						HasZergling();

};



