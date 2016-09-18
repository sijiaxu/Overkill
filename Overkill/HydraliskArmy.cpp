#include "HydraliskArmy.h"


void HydraliskArmy::mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();
	int hydraSpeed = int(BWAPI::UnitTypes::Zerg_Hydralisk.topSpeed() * 1.2);

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

	std::vector<EnemyUnit> priorityEnemy;

	int attackRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange() + 2 * 32;

	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (!u->isDetected() && u->isVisible())
				continue;

			if (u->getType().groundWeapon() != BWAPI::WeaponTypes::None)
			{
				int enemyAttackRange = u->getType().groundWeapon().maxRange() + 1 * 32;
				BWAPI::Unitset percentUnits = u->getUnitsInRadius(enemyAttackRange, BWAPI::Filter::IsAlly);
				if (percentUnits.size() > 0)
				{
					priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstUnit->getDistance(u)));
				}
			}
			else
			{
				priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstUnit->getDistance(u)));
			}

			//priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstUnit->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Hydralisk, targetPosition);
}


void HydraliskArmy::attack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();
	int hydraSpeed = int(BWAPI::UnitTypes::Zerg_Hydralisk.topSpeed());

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

	std::vector<EnemyUnit> priorityEnemy;

	int attackRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange() + 2 * 32;
	BWAPI::Unitset enemySet = firstUnit->getUnitsInRadius(attackRange);
	for (int percent = 0; percent < 5; percent++)
	{
		BWAPI::Unitset percentUnits = units[int(units.size() * percent * 0.2)].unit->getUnitsInRadius(12 * 32);
		enemySet.insert(percentUnits.begin(), percentUnits.end());
	}


	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (!u->isDetected() && u->isVisible())
				continue;

			//if (int(u->getType().topSpeed()) >= hydraSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
				//continue;

			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstUnit->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Hydralisk, targetPosition);

	/*
	if (priorityEnemy.size() == 0)
	{
		for (auto u : units)
		{
			smartMove(u.unit, targetPosition);
		}
	}
	else
	{
		//std::sort(priorityEnemy.begin(), priorityEnemy.end());
		//BWAPI::Position nearEnemy = priorityEnemy.back().unit->getPosition();
		//assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Hydralisk);

		for (auto u : units)
		{
			int closetDist = 99999;
			int highPriority = 0;
			BWAPI::Unit closet = NULL;
			for (auto e : priorityEnemy)
			{
				int distance = u.unit->getDistance(e.unit);

				if (!closet || (e.priority > highPriority) || (e.priority == highPriority && distance < closetDist))
				{
					closetDist = distance;
					highPriority = e.priority;
					closet = e.unit;
				}
			}
			if (closet != NULL)
				smartAttackUnit(u.unit, closet);
			else
				smartMove(u.unit, targetPosition);
		}
	}*/

	/*
	//for saving CPU compute time...
	if (units.size() > 24)
	{
		std::vector<EnemyUnit> priorityEnemy;
		BWAPI::Unitset enemySet = units.front().unit->getUnitsInRadius(6 * 32);
		for (auto u : enemySet)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy())
			{
				if (int(u->getType().topSpeed()) >= hydraSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
					continue;

				priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), units.front().unit->getDistance(u)));
			}
		}

		if (priorityEnemy.size() == 0)
		{
			for (auto u : units)
			{
				smartAttackMove(u.unit, targetPosition);
			}
		}
		else
		{
			std::sort(priorityEnemy.begin(), priorityEnemy.end());
			BWAPI::Position nearEnemy = priorityEnemy.back().unit->getPosition();
			for (auto u : units)
			{
				smartAttackMove(u.unit, nearEnemy);
			}
		}
	}
	else
	{
		BOOST_FOREACH(UnitState u, units)
		{
			BWAPI::Unit unit = u.unit;
			BWAPI::Unitset enemySet = unit->getUnitsInRadius(12 * 32);

			int closetDist = 99999;
			int highPriority = 0;
			BWAPI::Unit closet = NULL;

			//get the target unit nearby and in target circle
			BOOST_FOREACH(BWAPI::Unit u, enemySet)
			{
				if (u->getPlayer() == BWAPI::Broodwar->enemy())
				{
					if (int(u->getType().topSpeed()) >= hydraSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
						continue;

					int priority = getAttackPriority(u);
					int distance = unit->getDistance(u);

					// if it's a higher priority, or it's closer, set it
					if (!closet || (priority > highPriority) || (priority == highPriority && distance < closetDist))
					{
						closetDist = distance;
						highPriority = priority;
						closet = u;
					}
				}
			}
			if (closet != NULL)
				smartAttackUnit(unit, closet);
			else
				smartAttackMove(unit, targetPosition);
		}
	}*/
}


void HydraliskArmy::defend(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();
	int hydraSpeed = int(BWAPI::UnitTypes::Zerg_Hydralisk.topSpeed());

	int minTargetDistance = 99999;
	BWAPI::Unit firstUnit = units.front().unit;
	for (auto u : units)
	{
		if (u.unit->getDistance(targetPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(targetPosition);
			firstUnit = u.unit;
		}
	}

	std::vector<EnemyUnit> priorityEnemy;

	int attackRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange() + 2 * 32;
	BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(targetPosition, 16 * 32, BWAPI::Filter::IsEnemy);

	for (auto u : units)
	{
		BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(8 * 32, BWAPI::Filter::IsEnemy);
		enemySet.insert(percentUnits.begin(), percentUnits.end());
	}

	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (!u->isDetected() && u->isVisible())
				continue;

			//if (int(u->getType().topSpeed()) >= hydraSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
				//continue;

			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstUnit->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Hydralisk, targetPosition);

}


int HydraliskArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Protoss_High_Templar)
	{
		return 12;
	}
	else if ((type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) || type == BWAPI::UnitTypes::Terran_Bunker || unit->isRepairing())
	{
		return 11;
	}

	else if (type.airWeapon() != BWAPI::WeaponTypes::None && !type.isBuilding())
	{
		return 9;
	}
	else if (type.isWorker())
	{
		return 8;
	}

	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}

