#pragma once
#include "BattleArmy.h"



class ZerglingArmy : public BattleArmy
{

	void zerglingFSM(UnitState& myUnit, BWAPI::Unit target);

public:
	ZerglingArmy() {}

	std::vector<EnemyUnit> mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);

	int getAttackPriority(BWAPI::Unit unit);

	void attackScoutWorker(BWAPI::Unit unit);
};