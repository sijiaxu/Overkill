#pragma once
#include "Common.h"

enum microState { Attack = 0, LOWHEALTHRetreat = 1, Kite = 2, InCannonRange = 3, MoveToSafePlace = 4};

struct UnitState
{
	UnitState(BWAPI::Unit* u)
	{
		state = Attack;
		unit = u;
		currentHealth = u->getType().maxHitPoints();
		deltaDamge = 0;
		nextRetreatFrame = 0;
	}
	microState state;
	BWAPI::Unit* unit;
	BWAPI::Position retreatPosition;

	int currentHealth;
	int deltaDamge;
	int nextRetreatFrame;
};
