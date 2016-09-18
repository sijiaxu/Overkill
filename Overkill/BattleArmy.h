#pragma once
#include "Common.h"
#include "UnitState.h"


struct EnemyUnit
{
	BWAPI::Unit unit;
	int priority;
	int distance;
	int assignCount;

	EnemyUnit()
	{
		unit = NULL;
		priority = 0;
		distance = 0;
		assignCount = 0;
	}

	EnemyUnit(BWAPI::Unit u, int p, int d)
	{
		unit = u;
		priority = p;
		distance = d;
		assignCount = 0;
	}

	bool operator < (const EnemyUnit& u) const
	{
		if (priority < u.priority)
			return true;
		else if (priority > u.priority)
			return false;
		//priority == u.priority
		else
		{
			//distance less, priority higher
			if (distance > u.distance)
				return true;
			else
				return false;
		}
	}
};

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

	int							calMaxAssign(BWAPI::Unit enemyUnit, BWAPI::UnitType ourType);

	void						moveAbnormalUnits();

public:
	BattleArmy() { units.reserve(100); }
	virtual void				defend(BWAPI::Position targetPosition) = 0;
	virtual void				attack(BWAPI::Position priorityPosition) = 0;
	virtual void				mixAttack(BWAPI::Position priorityPosition, std::set<BWAPI::Unit> enemySet) {};
	void						assignTarget(std::vector<EnemyUnit>& priorityEnemy, BWAPI::UnitType armyType, BWAPI::Position targetPosition);

	void						addUnit(BWAPI::Unit u);
	std::vector<UnitState>&		getUnits() { return units; }
	void						armyAttackMove(BWAPI::Position targetPosition);
	void						armyMove(BWAPI::Position targetPosition);
	bool						reGroup(const BWAPI::Position & regroupPosition);

	bool						preciseReGroup(const BWAPI::Position & regroupPosition);
	bool						isInDanger(BWAPI::Unit u);

	void						removeUnit(BWAPI::Unit u);

	static void					smartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
	static void					smartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);
	static void					smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);

};


 

