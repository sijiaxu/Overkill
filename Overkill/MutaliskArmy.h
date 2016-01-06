#pragma once
#include "BattleArmy.h"
#include "InformationManager.h"



class MutaliskArmy : public BattleArmy
{
	void mutaliskFSM(UnitState& myUnit, BWAPI::Unit target);
	void mutaliskAssignTarget(std::vector<EnemyUnit>& priorityEnemy);

public:
	MutaliskArmy() {}
	void defend(BWAPI::Position targetPosition);
	void attack(BWAPI::Position priorityPosition);

	bool harassAttack(BWAPI::Position priorityPosition, int attackMode);
	int harassAttackPriority(BWAPI::Unit unit);

	void mixArmyAttack(BWAPI::Position priorityPosition);

	int getAttackPriority(BWAPI::Unit unit);

	bool flyGroup(BWAPI::Position targetPosition);

	static BWAPI::Position findSafePlace(BWAPI::Unit target, bool avoidCannon);

};