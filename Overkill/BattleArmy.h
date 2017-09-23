#pragma once
#include "Common.h"
#include "UnitState.h"


struct unitDistance
{
	BWAPI::Unit unit;
	int distance;

	unitDistance(BWAPI::Unit u, int d)
	{
		unit = u;
		distance = d;
	}

	bool operator < (const unitDistance & u)
	{
		if (distance < u.distance)
			return true;
		else
			return false;
	}
};



class BattleArmy
{
protected:
	std::vector<UnitState>	units;
	
	void						moveAbnormalUnits();
	
	int							onlyGroundAttackPriority(BWAPI::Unit unit);
	int							onlyAirAttackPriority(BWAPI::Unit unit);

public:
	BattleArmy() { units.reserve(100); }
	virtual std::vector<EnemyUnit>				mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet);
	virtual int					getAttackPriority(BWAPI::Unit unit);
	void						assignTarget(std::vector<EnemyUnit>& priorityEnemy, BWAPI::UnitType armyType,
		BWAPI::Position targetPosition, std::set<BWAPI::Unit>& enemySet, std::set<BWAPI::Unit>& attackingUnits);

	void						addUnit(BWAPI::Unit u);
	std::vector<UnitState>&		getUnits() { return units; }
	void						armyAttackMove(BWAPI::Position targetPosition);
	void						armyMove(BWAPI::Position targetPosition);
	bool						reGroup(const BWAPI::Position & regroupPosition);

	bool						preciseReGroup(const BWAPI::Position & regroupPosition);
	bool						isInDanger(BWAPI::Unit u);

	void						removeUnit(BWAPI::Unit u);
	
	void						attackMoveLowHealth(BWAPI::Unit attacker, BWAPI::Position targetPosition, BWAPI::Unit targetUnit = NULL);
	static void					smartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
	static void					smartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);
	static void					smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);

	static int					calMaxAssign(BWAPI::Unit enemyUnit, BWAPI::UnitType ourType);
	
};


 

