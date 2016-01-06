#pragma once
#include "BattleArmy.h"



class OverLordArmy : public BattleArmy
{
public:
	OverLordArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position targetPosition);

	void follow(BWAPI::Unit target);
};