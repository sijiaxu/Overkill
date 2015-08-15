#include "HydraliskArmy.h"

void HydraliskArmy::attack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();
	std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(targetPosition, 8 * 32);

	BOOST_FOREACH(UnitState u, units)
	{
		BWAPI::Unit* unit = u.unit;
		std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(HydraliskRange);
		enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

		int closetDist = 99999;
		int highPriority = 0;
		BWAPI::Unit* closet = NULL;

		//get the target unit nearby and in target circle
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy())
			{
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
}

void HydraliskArmy::defend(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();

	BOOST_FOREACH(UnitState u, units)
	{
		BWAPI::Unit* unit = u.unit;
		std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(HydraliskRange);

		int closetDist = 99999;
		int highPriority = 0;
		BWAPI::Unit* closet = NULL;

		//get the target unit nearby and in target circle
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy())
			{
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
}


int HydraliskArmy::getAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if ((type.canAttack() && !type.isWorker()) ||
		type == BWAPI::UnitTypes::Terran_Bunker ||
		type == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
		type == BWAPI::UnitTypes::Zerg_Sunken_Colony)
	{
		return 10;
	}
	// next priority is worker
	else if (type.isWorker())
	{
		return 9;
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

