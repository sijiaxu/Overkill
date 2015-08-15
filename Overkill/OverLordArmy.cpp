#include "OverLordArmy.h"

void OverLordArmy::attack(BWAPI::Position targetPosition)
{
	armyMove(targetPosition);
}

void OverLordArmy::defend(BWAPI::Position targetPosition)
{
	armyMove(targetPosition);
}


void OverLordArmy::follow(BWAPI::Unit* target)
{
	BOOST_FOREACH(UnitState u, units)
	{
		u.unit->follow(target);
	}
}
