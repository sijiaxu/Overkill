#pragma once
#include "BattleArmy.h"



class HydraliskArmy : public BattleArmy
{
public:
	HydraliskArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position targetPosition);
	int getAttackPriority(BWAPI::Unit unit);
};