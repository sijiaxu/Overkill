#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"


class sixZerglingTactic: public BattleTactic
{
	enum tacticState { LOCATIONASSIGN, MOVE, ATTACK, RETREAT, CIRCLE, END };
	tacticState state;

	std::vector<BWAPI::Position> 		movePositions;

public:
	sixZerglingTactic();
	
	virtual void			update();
	bool					isTacticEnd();

	void			generateAttackPath();
};



