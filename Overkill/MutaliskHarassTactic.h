#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"
#include "AstarPath.h"
#include "Time.cpp"




class MutaliskHarassTactic : public BattleTactic
{


	std::vector<BWAPI::Position> 		movePositions;

	//std::set<BWTA::Region *>			attackedEnemyRegions;

	BWAPI::Position targetPosition;
	void					armyQueuedMove(BattleArmy* myArmy, tacticState nextState);
	int						newCannonCount;
	int						newAntiAirAnmyCount;

	BWAPI::TilePosition		aStarTargetPosition;
	Timer					t;

public:
	MutaliskHarassTactic();

	virtual void			update();
	bool					isTacticEnd();
	virtual void			onUnitShow(BWAPI::Unit* unit);
	virtual void			setAttackPosition(BWAPI::Position targetPosition);
	virtual bool			hasEnemy();

	void					generateAttackPath(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition);
	bool					needRetreat();

	
};



