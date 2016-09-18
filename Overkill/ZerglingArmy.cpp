#include "ZerglingArmy.h"
#include "InformationManager.h"


void ZerglingArmy::attackScoutWorker(BWAPI::Unit unit)
{
	int zerglingAttackRange = BWAPI::UnitTypes::Zerg_Zergling.groundWeapon().maxRange();

	BOOST_FOREACH(UnitState u, units)
	{
		int distance = u.unit->getDistance(unit);
		if (distance <= zerglingAttackRange)
		{
			smartAttackUnit(u.unit, unit);
		}
		else
		{
			double2 direc = unit->getPosition() - u.unit->getPosition();
			double2 direcNormal = direc / direc.len();

			int targetx = unit->getPosition().x + int(direcNormal.x * 32 * 2);
			int targety = unit->getPosition().y + int(direcNormal.y * 32 * 2);
			BWAPI::Position target(targetx, targety);
			smartMove(u.unit, target);
		}
	}
}


void ZerglingArmy::mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet)
{
	if (units.size() == 0)
		return;

	bool isWorkerScout = false;
	std::set<BWTA::Region *> & ourRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (BWAPI::Broodwar->getFrameCount() < 10000)
	{
		std::map<BWTA::Region*, std::set<BWAPI::Unit>> enemyUnitsInRegion;

		BOOST_FOREACH(BWAPI::Unit enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
		{
			// if we do not have anti-air army, ignore
			if (enemyUnit->getType().isFlyer())
				continue;

			if (ourRegions.find(BWTA::getRegion(enemyUnit->getPosition())) != ourRegions.end())
			{
				enemyUnitsInRegion[BWTA::getRegion(enemyUnit->getPosition())].insert(enemyUnit);
			}
		}

		if (enemyUnitsInRegion.size() == 1 && enemyUnitsInRegion.begin()->second.size() == 1 &&
			(*enemyUnitsInRegion.begin()->second.begin())->getType().isWorker())
		{
			isWorkerScout = true;
		}
	}

	int minTargetDistance = 99999;
	BWAPI::Unit firstZergling = NULL;
	for (auto u : units)
	{
		if (u.unit->getDistance(targetPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(targetPosition);
			firstZergling = u.unit;
		}
	}

	int zerglingSpeed = int(BWAPI::UnitTypes::Zerg_Zergling.topSpeed());
	std::vector<EnemyUnit> priorityEnemy;


	BWAPI::Broodwar->drawCircleMap(firstZergling->getPosition().x, firstZergling->getPosition().y, 8 * 32, BWAPI::Colors::Green, false);

	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (u->getType().isFlyer() || (!u->isDetected() && u->isVisible()))
				continue;

			//if (int(u->getType().topSpeed()) >= zerglingSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
				//continue;

			//do not attack scout worker
			if (isWorkerScout && u->getType().isWorker() && ourRegions.find(BWTA::getRegion(u->getPosition())) != ourRegions.end())
			{
				continue;
			}

			if (u->getType().groundWeapon() != BWAPI::WeaponTypes::None)
			{
				int enemyAttackRange = u->getType().groundWeapon().maxRange() + 1 * 32;
				BWAPI::Unitset percentUnits = u->getUnitsInRadius(enemyAttackRange, BWAPI::Filter::IsAlly);
				if (percentUnits.size() > 0)
				{
					priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstZergling->getDistance(u)));
				}
			}
			else
			{
				priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), firstZergling->getDistance(u)));
			}

			//priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), firstZergling->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Zergling, targetPosition);
}


void ZerglingArmy::harassAttack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	bool isWorkerScout = false;
	std::set<BWTA::Region *> & ourRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (BWAPI::Broodwar->getFrameCount() < 10000)
	{
		std::map<BWTA::Region*, std::set<BWAPI::Unit>> enemyUnitsInRegion;

		BOOST_FOREACH(BWAPI::Unit enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
		{
			// if we do not have anti-air army, ignore
			if (enemyUnit->getType().isFlyer())
				continue;

			if (ourRegions.find(BWTA::getRegion(enemyUnit->getPosition())) != ourRegions.end())
			{
				enemyUnitsInRegion[BWTA::getRegion(enemyUnit->getPosition())].insert(enemyUnit);
			}
		}

		if (enemyUnitsInRegion.size() == 1 && enemyUnitsInRegion.begin()->second.size() == 1 &&
			(*enemyUnitsInRegion.begin()->second.begin())->getType().isWorker())
		{
			isWorkerScout = true;
		}
	}
	
	int minTargetDistance = 99999;
	BWAPI::Unit firstZergling = NULL;
	for (auto u : units)
	{
		if (u.unit->getDistance(targetPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(targetPosition);
			firstZergling = u.unit;
		}
	}

	int zerglingSpeed = int(BWAPI::UnitTypes::Zerg_Zergling.topSpeed());
	std::vector<EnemyUnit> priorityEnemy;

	BWAPI::Unitset enemySet = firstZergling->getUnitsInRadius(4 * 32);
	for (int percent = 0; percent < 5; percent++)
	{
		BWAPI::Unitset percentUnits = units[int(units.size() * percent * 0.2)].unit->getUnitsInRadius(12 * 32);
		enemySet.insert(percentUnits.begin(), percentUnits.end());
	}


	BWAPI::Broodwar->drawCircleMap(firstZergling->getPosition().x, firstZergling->getPosition().y, 8 * 32, BWAPI::Colors::Green, false);

	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (u->getType().isFlyer() || (!u->isDetected() && u->isVisible()))
				continue;

			//if (int(u->getType().topSpeed()) >= zerglingSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
				//continue;

			//do not attack scout worker
			if (isWorkerScout && u->getType().isWorker() && ourRegions.find(BWTA::getRegion(u->getPosition())) != ourRegions.end())
			{
				continue;
			}
			priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), firstZergling->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Zergling, targetPosition);

}

void ZerglingArmy::attack(BWAPI::Position targetPosition)
{
	harassAttack(targetPosition);
}


void ZerglingArmy::defend(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	bool isWorkerScout = false;
	std::set<BWTA::Region *> & ourRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	int minTargetDistance = 99999;
	BWAPI::Unit firstZergling = units.front().unit;
	for (auto u : units)
	{
		if (u.unit->getDistance(targetPosition) < minTargetDistance)
		{
			minTargetDistance = u.unit->getDistance(targetPosition);
			firstZergling = u.unit;
		}
	}

	int zerglingSpeed = int(BWAPI::UnitTypes::Zerg_Zergling.topSpeed());
	std::vector<EnemyUnit> priorityEnemy;

	BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(targetPosition, 16 * 32, BWAPI::Filter::IsEnemy);
	int count = 0;
	for (auto u : units)
	{
		count += 1;
		if (count % 2 == 0)
		{
			BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(6 * 32, BWAPI::Filter::IsEnemy);
			enemySet.insert(percentUnits.begin(), percentUnits.end());
		}
	}

	BWAPI::Broodwar->drawCircleMap(firstZergling->getPosition().x, firstZergling->getPosition().y, 8 * 32, BWAPI::Colors::Green, false);

	for (auto u : enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (u->getType().isFlyer() || (!u->isDetected() && u->isVisible()))
				continue;

			if (int(u->getType().topSpeed()) >= zerglingSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(targetPosition))
				continue;

			//do not attack scout worker
			if (isWorkerScout && u->getType().isWorker() && ourRegions.find(BWTA::getRegion(u->getPosition())) != ourRegions.end())
			{
				continue;
			}
			priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), firstZergling->getDistance(u)));
		}
	}

	assignTarget(priorityEnemy, BWAPI::UnitTypes::Zerg_Zergling, targetPosition);

}


void ZerglingArmy::zerglingFSM(UnitState& myUnit, BWAPI::Unit target)
{
	switch (myUnit.state)
	{
	case Attack:
	{
		if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
		{
			myUnit.deltaDamge = myUnit.currentHealth - myUnit.unit->getHitPoints();
			myUnit.currentHealth = myUnit.unit->getHitPoints();
		}
		//only do retreat at necessary time, retreat interval is 30 seconds
		if (BWAPI::Broodwar->getFrameCount() > myUnit.nextRetreatFrame && myUnit.unit->getHitPoints() < myUnit.unit->getType().maxHitPoints() / 4 && myUnit.deltaDamge > 0)
		{
			double2 direc = myUnit.unit->getPosition() - target->getPosition();
			double2 direcNormal = direc / direc.len();
			 
			int targetx = myUnit.unit->getPosition().x + int(direcNormal.x * 32 * 5);
			int targety = myUnit.unit->getPosition().y + int(direcNormal.y * 32 * 5);

			myUnit.retreatPosition = BWAPI::Position(targetx < 0 ? 32 : (targetx > BWAPI::Broodwar->mapWidth() * 32 ? BWAPI::Broodwar->mapWidth() * 31 : targetx), targety < 0 ? 32 : (targety > BWAPI::Broodwar->mapHeight() * 32 ? BWAPI::Broodwar->mapHeight() * 31 : targety));
			if (BWAPI::Broodwar->isWalkable(myUnit.retreatPosition.x / 8, myUnit.retreatPosition.y / 8))
				myUnit.state = LOWHEALTHRetreat;
			else
			{
				myUnit.nextRetreatFrame = BWAPI::Broodwar->getFrameCount() + 25 * 30;
				myUnit.state = Attack;
			}
		}
		else
			smartAttackUnit(myUnit.unit, target);
		break;
	}
	case LOWHEALTHRetreat:
	{
		if (myUnit.unit->getDistance(myUnit.retreatPosition) < 32)
		{
			myUnit.currentHealth = myUnit.unit->getHitPoints();
			myUnit.deltaDamge = 0;
			myUnit.nextRetreatFrame = BWAPI::Broodwar->getFrameCount() + 25 * 30;
			myUnit.state = Attack;
		}
		else
			smartMove(myUnit.unit, myUnit.retreatPosition);
		break;
	}
	}
}

// get the attack priority of a type in relation to a zergling
int ZerglingArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	// highest priority is something that can attack us or aid in combat
	if (type.groundWeapon() != BWAPI::WeaponTypes::None || type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 11;
	}
	else if (type.isSpellcaster())
	{
		return 10;
	}
	else if (type.isRefinery())
	{
		return 8;
	}

	else if (type.isResourceDepot())
	{
		return 7;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 6;
	}
	else if (type.isBuilding())
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

int ZerglingArmy::harassAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	// highest priority is something that can attack us or aid in combat
	if ((type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker())
		|| unit->isRepairing()
		|| type == BWAPI::UnitTypes::Terran_Bunker
		|| type == BWAPI::UnitTypes::Protoss_High_Templar)
	{
		return 11;
	}
	else if (type.isWorker())
	{
		return 8;
	}
	else if (type.isRefinery())
	{
		return 7;
	}

	else if (type.isResourceDepot())
	{
		return 7;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 6;
	}
	else if (type.isBuilding())
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

