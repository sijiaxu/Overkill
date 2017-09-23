#pragma once
#include "BattleArmy.h"



class OverLordArmy : public BattleArmy
{
public:
	OverLordArmy() {}
	std::vector<EnemyUnit>	mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);
	void follow(BWAPI::Unit target);
};