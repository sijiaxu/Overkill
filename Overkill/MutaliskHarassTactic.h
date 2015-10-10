#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"
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
	std::map<BWAPI::Unit*, std::list<BWAPI::TilePosition>> unitMovePath;
	std::set<BWAPI::Unit*>	moveComplete;
	BWAPI::Position			moveTarget;
	int						moveFinishCount;
	int						triggerChangeTime;

	void					locationAssign(MutaliskArmy* mutalisk, BWAPI::Position endPosition, tacticState nextState, bool nearPosition);
	void					locationMove(MutaliskArmy* mutalisk, tacticState nextState);
public:
	MutaliskHarassTactic();

	virtual void			update();
	bool					isTacticEnd();
	virtual void			onUnitShow(BWAPI::Unit* unit);
	virtual void			setAttackPosition(BWAPI::Position targetPosition);
	virtual bool			hasEnemy();
	virtual void			addArmyUnit(BWAPI::Unit* unit);

	void					generateAttackPath(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition);
	int						needRetreat();

	
};



