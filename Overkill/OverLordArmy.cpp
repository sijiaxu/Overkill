#include "OverLordArmy.h"


std::vector<EnemyUnit> OverLordArmy::mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet)
{
	armyMove(targetPosition);
	return std::vector<EnemyUnit>();
}

void OverLordArmy::follow(BWAPI::Unit target)
{
	BOOST_FOREACH(UnitState u, units)
	{
		u.unit->follow(target);
	}
}
