#include "HydraliskArmy.h"

void HydraliskArmy::attack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	int HydraliskRange = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().maxRange();
	int hydraSpeed = int(BWAPI::UnitTypes::Zerg_Hydralisk.topSpeed());

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
}


void HydraliskArmy::defend(BWAPI::Position targetPosition)
{
	attack(targetPosition);
}


int HydraliskArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat

	if (type.canAttack() ||
		type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 10;
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

