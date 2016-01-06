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
#include "DefendTactic.h"


struct tacKey
{
	tacticType tacName;
	BWAPI::Position attackPosition;

	tacKey(tacticType t, BWAPI::Position a)
	{
		tacName = t;
		attackPosition = a;
	}

	bool operator < (const tacKey& t) const
	{
		if (tacName < t.tacName)
		{
			return true;
		}
		else if (tacName > t.tacName)
		{
			return false;
		}
		else
		{
			if (attackPosition.getLength() < t.attackPosition.getLength())
				return true;
			else
				return false;
		}
	}
};

class TacticManager
{
	
public:
	
	void				update();
	void				onUnitDestroy(BWAPI::Unit unit);
	void				addTactic(tacticType tactic, BWAPI::Position attackPosition);
	void				addTacticUnit(tacticType tactic, BWAPI::Position attackPosition, BWAPI::Unit unit);
	void				addTacticArmy(tacticType tactic, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count);

	bool				isTacticRun(tacticType tactic, BWAPI::Position attackPosition);
	bool				isOneTacticRun(tacticType tactic);
	bool				isHaveNoneDefendTactic();
	BWAPI::Position		getTacticPosition(tacticType tactic);
	void				checkTacticEnd();

	static TacticManager&		Instance();
	int					getTacArmyForce(tacticType tactic, BWAPI::Position attackPosition);
	void				assignDefendArmy(BWAPI::Position defendPosition, int needSupply);

	void				onUnitShow(BWAPI::Unit unit);

private:
	TacticManager() {}
	std::map<tacKey, BattleTactic*> myTactic;
};


