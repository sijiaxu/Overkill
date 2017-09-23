#pragma once
#include "BattleArmy.h"



class ScourgeArmy : public BattleArmy
{
public:
	ScourgeArmy() {}
	std::vector<EnemyUnit> mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);
};

