#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "ZerglingArmy.h"
#include "AstarPath.h"
#include "Time.cpp"
#include "TimeManager.cpp"




class MutaliskHarassTactic : public BattleTactic
{


	std::vector<BWAPI::Position> 		movePositions;

	//std::set<BWTA::Region *>			attackedEnemyRegions;

	BWAPI::Position targetPosition;
	void					armyQueuedMove(BattleArmy* myArmy, tacticState nextState);
	int						newCannonCount;
	int						newAntiAirAnmyCount;
	BWAPI::Position			groupPosition;

	Timer					t;
	std::map<BWAPI::Unit, std::list<BWAPI::TilePosition>> unitMovePath;
	std::set<BWAPI::Unit>	moveComplete;
	BWAPI::Position			moveTarget;
	int						moveFinishCount;
	int						triggerChangeTime;
	std::set<BWAPI::Unit>	friendUnitNearBy;
	std::set<BWAPI::Unit>	nearbyUnits;

	void					locationAssign(MutaliskArmy* mutalisk, BWAPI::Position endPosition, tacticState nextState, bool nearPosition);
	void					locationMove(MutaliskArmy* mutalisk, tacticState nextState);

	
public:
	MutaliskHarassTactic();

	virtual void			update();
	bool					isTacticEnd();
	virtual void			onUnitShow(BWAPI::Unit unit);
	virtual bool			hasEnemy();
	virtual void			addArmyUnit(BWAPI::Unit unit);

	void					generateAttackPath(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition);
	int						needRetreat();
	virtual void			retreatSet();

	
};



