#pragma once
#include "Common.h"

enum microState { Attack = 0, LOWHEALTHRetreat = 1, Kite = 2, InCannonRange = 3, MoveToSafePlace = 4, Irradiated = 5};


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


struct UnitState
{
	UnitState(BWAPI::Unit u)
	{
		state = Attack;
		unit = u;
		currentHealth = u->getType().maxHitPoints();
		deltaDamge = 0;
		nextRetreatFrame = 0;
		target = EnemyUnit();
		previousTimeFrame = 0;
	}

	bool isAbnormal()
	{
		if (state == Irradiated)
			return true;
		else
			return false;
	}

	microState state;
	BWAPI::Unit unit;
	BWAPI::Position retreatPosition;
	EnemyUnit target;

	int previousTimeFrame;
	BWAPI::Position previousPosition;
	int totalWalkDistance;

	int currentHealth;
	int deltaDamge;
	int nextRetreatFrame;
};
