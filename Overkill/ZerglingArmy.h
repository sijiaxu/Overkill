#pragma once
#include "BattleArmy.h"



class ZerglingArmy : public BattleArmy
{

	void zerglingFSM(UnitState& myUnit, BWAPI::Unit target);

public:
	ZerglingArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position targetPosition);

	void mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);

	int getAttackPriority(BWAPI::Unit unit);

	void harassAttack(BWAPI::Position targetPosition);
	int harassAttackPriority(BWAPI::Unit unit);

	void attackScoutWorker(BWAPI::Unit unit);
};