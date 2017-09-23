#include "ScourgeArmy.h"

std::vector<EnemyUnit> ScourgeArmy::mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet)
{

	if (units.size() == 0)
		return std::vector<EnemyUnit>();

	int minTargetDistance = 99999;
	BWAPI::Unit firstUnit = NULL;
	for (auto u : units)
	{
		if (u.unit->getDistance(targetPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(targetPosition);
			firstUnit = u.unit;
		}
	}

	BWAPI::UnitType armyType = units.front().unit->getType();
	std::vector<EnemyUnit> priorityEnemy;
	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (!u->isDetected() && u->isVisible())
				continue;

			if (!u->getType().isFlyer())
				continue;

			if (u->getType().isBuilding() || u->getType() == BWAPI::UnitTypes::Protoss_Interceptor)
				continue;
			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), u->getID()));
		}
	}

	return priorityEnemy;
}