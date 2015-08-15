#pragma once
#include "BattleArmy.h"
#include "InformationManager.h"


class ZerglingArmy : public BattleArmy
{

	void zerglingFSM(UnitState& myUnit, BWAPI::Unit* target);

public:
	ZerglingArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position targetPosition);
	int getAttackPriority(BWAPI::Unit * unit);

	void harassAttack(BWAPI::Position targetPosition);
	int harassAttackPriority(BWAPI::Unit * unit);

	void attackScoutWorker(BWAPI::Unit* unit);
};