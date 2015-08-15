#pragma once
#include "Common.h"
#include "InformationManager.h"
#include "BattleArmy.h"
#include "BattleTactic.h"
#include "sixZerglingTactic.h"
#include "MutaliskHarassTactic.h"
#include "AttackManager.h"
#include "HydraliskTactic.h"
#include "TimeManager.cpp"

enum tacticType { ZerglingHarassTac = 0, MutaliskHarassTac = 1, HydraliskPushTactic = 2,};


class TacticManager
{
	
public:
	
	void				update();
	void				onUnitDestroy(BWAPI::Unit * unit);
	void				addTactic(tacticType tactic, BWAPI::Position attackPosition);
	void				addTacticUnit(tacticType tactic, BWAPI::Unit * unit);
	void				initTacticArmy(tacticType tactic, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count);

	bool				isTacticRun(tacticType tactic);
	void				checkTacticEnd();

	static TacticManager&		Instance();

	void				onUnitShow(BWAPI::Unit* unit);

private:
	TacticManager() {}

	// one type of tactic can only be executed once at the same time
	std::map<tacticType, BattleTactic*> myTactic;
};


