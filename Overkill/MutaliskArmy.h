#pragma once
#include "BattleArmy.h"




class MutaliskArmy : public BattleArmy
{
	void mutaliskFSM(UnitState& myUnit, BWAPI::Unit target);
	void mutaliskAssignTarget(std::vector<EnemyUnit>& priorityEnemy);

public:
	MutaliskArmy() {}

	bool harassAttack(BWAPI::Position priorityPosition, int attackMode);
	int harassAttackPriority(BWAPI::Unit unit);

	int getAttackPriority(BWAPI::Unit unit);

	bool flyGroup(BWAPI::Position targetPosition);

	static BWAPI::Position findSafePlace(BWAPI::Unit target, bool avoidCannon);

};