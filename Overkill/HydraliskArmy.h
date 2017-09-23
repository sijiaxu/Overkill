#pragma once
#include "BattleArmy.h"



class HydraliskArmy : public BattleArmy
{
public:
	HydraliskArmy() {}
	int getAttackPriority(BWAPI::Unit unit);
};