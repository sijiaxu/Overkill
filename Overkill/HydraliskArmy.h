#pragma once
#include "BattleArmy.h"



class HydraliskArmy : public BattleArmy
{
public:
	HydraliskArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position targetPosition);
	void mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);
	int getAttackPriority(BWAPI::Unit unit);
};