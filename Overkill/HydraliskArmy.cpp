#include "HydraliskArmy.h"



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

