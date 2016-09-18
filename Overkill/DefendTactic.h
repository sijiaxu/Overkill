#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "BattleArmy.h"



class ArmyDefendTactic : public BattleTactic
{
	int retreatJustifyTime;
	BWAPI::Position naturalChokePosition;
public:
	ArmyDefendTactic();

	virtual void			update();
	bool					isTacticEnd();

	void					generateAttackPath();
};



