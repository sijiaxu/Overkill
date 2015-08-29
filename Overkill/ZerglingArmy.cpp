#include "ZerglingArmy.h"



void ZerglingArmy::attackScoutWorker(BWAPI::Unit* unit)
{
	int zerglingAttackRange = BWAPI::UnitTypes::Zerg_Zergling.groundWeapon().maxRange();

	BOOST_FOREACH(UnitState u, units)
	{
		int distance = u.unit->getDistance(unit);
		if (distance < zerglingAttackRange)
		{
			smartAttackUnit(u.unit, unit);
		}
		else
		{
			double2 direc = unit->getPosition() - u.unit->getPosition();
			double2 direcNormal = direc / direc.len();

			int targetx = unit->getPosition().x() + int(direcNormal.x * 32 * 2);
			int targety = unit->getPosition().y() + int(direcNormal.y * 32 * 2);
			BWAPI::Position target(targetx, targety);
			smartAttackMove(u.unit, target);

		}
	}
}


void ZerglingArmy::harassAttack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;
	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
	std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(targetPosition, 8 * 32);

	BOOST_FOREACH(UnitState u, units)
	{
		BWAPI::Unit* unit = u.unit;
		std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(6 * 32);	

		enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

		int closetDist = 99999;
		int highPriority = 0;
		BWAPI::Unit* closet = NULL;

		//get the target unit nearby and in target circle
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && influnceMap[u->getTilePosition().x()][u->getTilePosition().y()].groundForce == 0)
			{
				//ignore cannon, kill worker first
				int priority = harassAttackPriority(u);
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

		// do not attack building alone the way
		if (closet != NULL && closet->getType().groundWeapon() != BWAPI::WeaponTypes::None && !closet->getType().isBuilding()) 
			zerglingFSM(u, closet);
		else if (closet != NULL && BWAPI::Broodwar->isVisible(BWAPI::TilePosition(targetPosition)))
			zerglingFSM(u, closet);
		else
			smartMove(unit, targetPosition);
	}
}

void ZerglingArmy::attack(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	BOOST_FOREACH(UnitState u, units)
	{
		BWAPI::Unit* unit = u.unit;
		std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(8 * 32);

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
		if (closet != NULL )
			zerglingFSM(u, closet);
		else
			smartMove(unit, targetPosition);
	}
}

void ZerglingArmy::defend(BWAPI::Position targetPosition)
{
	if (units.size() == 0)
		return;

	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
	std::set<BWAPI::Unit *> enemySet;
	std::set<BWTA::Region *>& myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (myRegion.find(BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition()))) != myRegion.end())
		{
			enemySet.insert(enemyUnit);
		}
	}

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& myBuilding = InformationManager::Instance().getOurAllBuildingUnit();
	int closeSunkenDist = 99999;
	BWAPI::Unit* closeSunker = NULL;
	BOOST_FOREACH(BWAPI::Unit* sunken, myBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony])
	{
		int distance = units[0].unit->getDistance(sunken);
		if (distance < closeSunkenDist && distance < 800)
		{
			closeSunkenDist = distance;
			closeSunker = sunken;
		}
	}

	BOOST_FOREACH(UnitState u, units)
	{
		BWAPI::Unit* unit = u.unit;
		int closetDist = 99999;
		int highPriority = 0;
		BWAPI::Unit* closet = NULL;

		//get the target unit
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
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
		
		if (closet != NULL && closeSunker != NULL)
		{
			//TODO:: add self influence map to justify our 
			// if target is an can-attack unit with weapon force large than ours
			int targetForce = int(influnceMap[closet->getTilePosition().x()][closet->getTilePosition().y()].enemyUnitGroundForce) + int(influnceMap[closet->getTilePosition().x()][closet->getTilePosition().y()].groundForce);
			if (int(units.size() * BWAPI::UnitTypes::Zerg_Zergling.groundWeapon().damageAmount() / 2) < targetForce &&
				(closet->getType().canAttack() || closet->getType() == BWAPI::UnitTypes::Terran_Bunker) && 
				!closet->getType().isWorker() && closet->getDistance(closeSunker->getPosition()) > BWAPI::UnitTypes::Zerg_Sunken_Colony.groundWeapon().maxRange())
			{
				smartMove(unit, closeSunker->getPosition());
			}
			else
			{
				zerglingFSM(u, closet);
			}
		}
		else if (closet != NULL)
		{
			zerglingFSM(u, closet);
		}
		else
			smartAttackMove(unit, targetPosition);
	}
}


void ZerglingArmy::zerglingFSM(UnitState& myUnit, BWAPI::Unit* target)
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

			int targetx = myUnit.unit->getPosition().x() + int(direcNormal.x * 32 * 5);
			int targety = myUnit.unit->getPosition().y() + int(direcNormal.y * 32 * 5);

			myUnit.retreatPosition = BWAPI::Position(targetx < 0 ? 32 : (targetx > BWAPI::Broodwar->mapWidth() * 32 ? BWAPI::Broodwar->mapWidth() * 31 : targetx), targety < 0 ? 32 : (targety > BWAPI::Broodwar->mapHeight() * 32 ? BWAPI::Broodwar->mapHeight() * 31 : targety));
			if (BWAPI::Broodwar->isWalkable(myUnit.retreatPosition.x() / 8, myUnit.retreatPosition.y() / 8))
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
int ZerglingArmy::getAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if ((type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker())||
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

int ZerglingArmy::harassAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if (type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker() && !type.isBuilding())
	{
		return 10;
	}
	// next priority is worker
	else if (type.isWorker())
	{
		return 9;
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

