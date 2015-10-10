#include "MutaliskArmy.h"



void MutaliskArmy::defend(BWAPI::Position priorityPosition)
{
	if (units.size() == 0)
		return;

	//attack the target region, enemy's priority is decided by defend tactic
	BWAPI::Unit* unit = (*units.begin()).unit;
	std::set<BWAPI::Unit*> enemySet = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 8 * 32);
	int mutaliskSpeed = int(BWAPI::UnitTypes::Zerg_Mutalisk.topSpeed());

	// loop through enemySet to get the top n target for n mutalisk group
	std::vector<EnemyUnit> priorityEnemy;
	priorityEnemy.reserve(100);

	BOOST_FOREACH(BWAPI::Unit* u, enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (u->getType() == BWAPI::UnitTypes::Protoss_Observer)
				continue;
			if (int(u->getType().topSpeed()) >= mutaliskSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(priorityPosition))
				continue;

			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), u->getDistance(unit)));
		}
	}

	if (priorityEnemy.size() == 0)
	{
		BOOST_FOREACH(UnitState& u, units)
		{
			smartAttackMove(u.unit, priorityPosition);
		}
	}
	else
	{
		std::sort(priorityEnemy.begin(), priorityEnemy.end());
		mutaliskAssignTarget(priorityEnemy);
	}
}

void MutaliskArmy::mixArmyAttack(BWAPI::Position priorityPosition)
{
	//get the group attack target
	if (units.size() == 0)
		return;

	BWAPI::Unit* unit = (*units.begin()).unit;

	std::set<BWAPI::Unit*> enemySet = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 12 * 32);

	// loop through enemySet to get the top n target for n mutalisk group
	std::vector<EnemyUnit> priorityEnemy;
	priorityEnemy.reserve(100);

	BOOST_FOREACH(BWAPI::Unit* u, enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{	//get the high priority and closed to priorityPosition unit
			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), int(priorityPosition.getDistance(u->getPosition()))));
		}
	}

	if (priorityEnemy.size() == 0)
	{
		BOOST_FOREACH(UnitState& u, units)
		{
			smartAttackMove(u.unit, priorityPosition);
		}
	}
	else
	{
		std::sort(priorityEnemy.begin(), priorityEnemy.end());
		mutaliskAssignTarget(priorityEnemy);
	}
}

//attack strategy for mutalisk. 
//unlike the ground unit, mutalisk attack the high priority target first(in the unit's circle and target circle),
void MutaliskArmy::attack(BWAPI::Position priorityPosition)
{
	//get the group attack target
	if (units.size() == 0)
		return;

	BWAPI::Unit* unit = (*units.begin()).unit;
	BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 8 * 32, BWAPI::Colors::Purple, false);
	BWAPI::Broodwar->drawCircleMap(priorityPosition.x(), priorityPosition.y(), 8 * 32, BWAPI::Colors::Purple, false);

	std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(12 * 32);
	//std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 8 * 32);

	//enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

	// loop through enemySet to get the top n target for n mutalisk group
	std::vector<EnemyUnit> priorityEnemy;
	priorityEnemy.reserve(100);
	
	BOOST_FOREACH(BWAPI::Unit* u, enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{	
			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), u->getDistance(unit)));
		}
	}

	if (priorityEnemy.size() == 0)
	{
		BOOST_FOREACH(UnitState& u, units)
		{
			smartAttackMove(u.unit, priorityPosition);
		}
	}
	else
	{
		std::sort(priorityEnemy.begin(), priorityEnemy.end());
		mutaliskAssignTarget(priorityEnemy);
	}
}


BWAPI::Position MutaliskArmy::findSafePlace(BWAPI::Unit* target, bool avoidCannon)
{
	int mutaliskAttackRange = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().maxRange() / 32;
	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();

	double2 origin(target->getTilePosition().x(), target->getTilePosition().y());
	int x = target->getTilePosition().x();
	int y = target->getTilePosition().y();
	int length = 1;
	int j = 0;
	bool first = true;
	int dx = 0;
	int dy = 1;

	int buildingWidth = target->getType().tileWidth();
	int buildingHeight = target->getType().tileHeight();
	int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;

	while (length < 20)//BWAPI::Broodwar->mapWidth())
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
		{
			if (!avoidCannon && influnceMap[x][y].airForce == 0 && influnceMap[x][y].enemyUnitAirForce == 0)
			{
				if (target->getTilePosition().getDistance(BWAPI::TilePosition(x, y)) < 6)
				{
					return BWAPI::Position(x * 32, y * 32);
				}
			}

			if (avoidCannon && influnceMap[x][y].airForce == 0)
			{
				double2 normalDirect = (double2(x, y) - origin).normal();
				double2 safePosition(double2(x, y) + normalDirect * 2);
				//BWAPI::TilePosition(target->getPosition()) get the center of the building
				if (BWAPI::TilePosition(int(safePosition.x), int(safePosition.y)).getDistance(BWAPI::TilePosition(target->getPosition())) <= mutaliskAttackRange + maxSize)
					return BWAPI::Position(int(safePosition.x * 32), int(safePosition.y * 32));
			}
		}

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;

		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first = !first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return BWAPI::Positions::None;
}


void MutaliskArmy::mutaliskFSM(UnitState& myUnit, BWAPI::Unit* target)
{
	std::vector<std::vector<gridInfo>>& influnceMap = InformationManager::Instance().getEnemyInfluenceMap();
	int deltaMove = 0;
	char jobCode;
	if (myUnit.state == Attack)
	{
		jobCode = 'A';
	}
	else if (myUnit.state == MoveToSafePlace)
	{
		jobCode = 'M';
	}

	BWAPI::Broodwar->drawTextMap(myUnit.unit->getPosition().x(), myUnit.unit->getPosition().y() - 5, "\x07%c", jobCode);
	BWAPI::TilePosition unitCenter = BWAPI::TilePosition(myUnit.unit->getPosition());
	
	switch (myUnit.state)
	{
	case Attack:
	{
		if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
		{
			myUnit.deltaDamge = myUnit.currentHealth - myUnit.unit->getHitPoints();
			myUnit.currentHealth = myUnit.unit->getHitPoints();
		}

		/*
		//if my unit under enemy cannon's range, seek a safe place to attack other unit
		if (target->getType() != BWAPI::UnitTypes::Protoss_Photon_Cannon 
			&& target->getType() != BWAPI::UnitTypes::Terran_Missile_Turret
			&& target->getType() != BWAPI::UnitTypes::Zerg_Spore_Colony
			&& target->getType() != BWAPI::UnitTypes::Terran_Bunker
			&& influnceMap[unitCenter.x()][unitCenter.y()].airForce > 0
			&& BWAPI::Broodwar->getFrameCount() > myUnit.nextRetreatFrame)
		{
			myUnit.retreatPosition = findSafePlace(target, true);
			if (myUnit.retreatPosition != BWAPI::Positions::None)
			{
				myUnit.unit->move(myUnit.retreatPosition);
				myUnit.unit->stop(true);
				myUnit.state = MoveToSafePlace;
				//BWAPI::Broodwar->printf("from (%d,%d) move to (%d,%d)", unitCenter.x(), unitCenter.y(), BWAPI::TilePosition(myUnit.retreatPosition).x(), BWAPI::TilePosition(myUnit.retreatPosition).y());
				break;
				//myUnit.unit->stop(true);
				//myUnit.unit->attack(target->getPosition(), true);
			}
		}*/

		//only do retreat at necessary time, retreat interval is 30 seconds
		if (BWAPI::Broodwar->getFrameCount() > myUnit.nextRetreatFrame && myUnit.unit->getHitPoints() <= myUnit.unit->getType().maxHitPoints() / 2 && myUnit.deltaDamge > 0)
		{
			/*
			double2 direc = myUnit.unit->getPosition() - target->getPosition();
			double2 direcNormal = direc / direc.len();

			int targetx = myUnit.unit->getPosition().x() + int(direcNormal.x * 32 * 4);
			int targety = myUnit.unit->getPosition().y() + int(direcNormal.y * 32 * 4);

			myUnit.retreatPosition = BWAPI::Position(targetx < 0 ? 32 * 2 : (targetx > BWAPI::Broodwar->mapWidth() * 32 ? BWAPI::Broodwar->mapWidth() * 30 : targetx), targety < 0 ? 32 * 2 : (targety > BWAPI::Broodwar->mapHeight() * 32 ? BWAPI::Broodwar->mapHeight() * 30 : targety));
			*/
			myUnit.retreatPosition = findSafePlace(myUnit.unit, false);
			if (myUnit.retreatPosition != BWAPI::Positions::None)
			{
				myUnit.unit->move(myUnit.retreatPosition);
				myUnit.unit->stop(true);
				//myUnit.unit->stop(true);
				//myUnit.unit->attack(target->getPosition(), true);
				myUnit.state = MoveToSafePlace;
				break;
			}
		}
		else
			smartAttackUnit(myUnit.unit, target);
		break;
	}
	case MoveToSafePlace:
	{
		BWAPI::Broodwar->drawCircleMap(myUnit.retreatPosition.x(), myUnit.retreatPosition.y(), 8, BWAPI::Colors::Green, true);
		BWAPI::Broodwar->drawLineMap(BWAPI::Position(unitCenter).x(), BWAPI::Position(unitCenter).y(), myUnit.retreatPosition.x(), myUnit.retreatPosition.y(), BWAPI::Colors::Green);

		if (myUnit.unit->getTarget() != NULL && myUnit.unit->getTarget()->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon)
		{
			break;
		}

		if (!myUnit.unit->isMoving())//myUnit.unit->isAttacking())
		{
			myUnit.currentHealth = myUnit.unit->getHitPoints();
			myUnit.deltaDamge = 0;
			myUnit.nextRetreatFrame = BWAPI::Broodwar->getFrameCount() + 25 * 10;
			myUnit.state = Attack;
		}
		break;
	}
	}
}


bool MutaliskArmy::flyGroup(BWAPI::Position targetPosition)
{
	int count = 0;
	// direct move and attack when enter in group circle
	BOOST_FOREACH(UnitState u, units)
	{
		// if the unit is outside the regroup area
		if (u.unit->getDistance(targetPosition) < 32 * 4)
		{
			count++;
		}
		else
		{
			u.unit->rightClick(targetPosition);
		}
	}

	if (units.size() > 0 && count * 10 / units.size() >= 9 )
		return true;
	else
		return false;
}

//attack the priorityPosition region only with high priority to attack worker
bool MutaliskArmy::harassAttack(BWAPI::Position priorityPosition, int attackMode)
{
	//get the group attack target
	if (units.size() == 0)
		return true;

	int mutaliskAttackRange = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().maxRange() / 32;
	int mutaliskAttackDamage = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().damageAmount();
	int mutaliskSpeed = int(BWAPI::UnitTypes::Zerg_Mutalisk.topSpeed());

	BWAPI::Unit* unit = units.front().unit;
	BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 12 * 32, BWAPI::Colors::Green, false);

	std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(12 * 32);
	std::set<BWAPI::Unit*> middleUnitnearby = units[units.size() / 2].unit->getUnitsInRadius(12 * 32);
	enemySet.insert(middleUnitnearby.begin(), middleUnitnearby.end());
	std::set<BWAPI::Unit*> backUnitnearby = units.back().unit->getUnitsInRadius(12 * 32);
	enemySet.insert(backUnitnearby.begin(), backUnitnearby.end());

	//std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 12 * 32);
	//enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
	
	// loop through enemySet to get the top n target for n mutalisk group
	std::vector<EnemyUnit> priorityEnemy;
	priorityEnemy.reserve(100);

	//normal mode
	if (attackMode == 1)
	{
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			//TODO: BWAPI bugs, addon's getplayer is not enemy
			if (u->getPlayer() == BWAPI::Broodwar->enemy() || u->getType().isAddon())
			{
				if (u->getType() == BWAPI::UnitTypes::Protoss_Observer)
					continue;
				//ignore unit with faster speed , and not attacking us outside target region
				if (int(u->getType().topSpeed()) >= mutaliskSpeed && !u->isAttacking() && BWTA::getRegion(u->getPosition()) != BWTA::getRegion(priorityPosition))
					continue;

				//first mutalisk always attack highest priority target, so enemy's priority is relative fixed in a short time
				priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
				BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 8, BWAPI::Colors::Purple, true);
			}
		}
	}
	// only attack has-air-weapon unit
	else if (attackMode == 3)
	{
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && (u->getType().airWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker)
				&& BWTA::getRegion(priorityPosition) == BWTA::getRegion(u->getPosition()))
			{
				if (u->getType() == BWAPI::UnitTypes::Protoss_Observer)
					continue;
				priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
				BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 8, BWAPI::Colors::Purple, true);
			}
		}
	}
	/*
	//special attack, when facing large cannon
	else
	{
		BOOST_FOREACH(BWAPI::Unit* u, enemySet)
		{
			//TODO: BWAPI bugs, addon's getplayer is not enemy
			if (u->getPlayer() == BWAPI::Broodwar->enemy() || u->getType().isAddon()) // && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(priorityPosition))
			{
				if ((u->getType().isBuilding() && u->getType().canAttack()) || u->getType() == BWAPI::UnitTypes::Terran_Bunker)
					continue;

				if (u->isInvincible())
					continue;

				BWAPI::Position tmp = findSafePlace(u, true);
				if (tmp != BWAPI::Positions::None)
				{
					priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
					BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 4, BWAPI::Colors::Red, true);
				}
			}
		}
	}*/


	if (priorityEnemy.size() == 0)
	{
		armyMove(priorityPosition);
		return true;
	}
	else
	{
		std::sort(priorityEnemy.begin(), priorityEnemy.end());
		mutaliskAssignTarget(priorityEnemy);
		return false;
	}
}

void MutaliskArmy::mutaliskAssignTarget(std::vector<EnemyUnit>& priorityEnemy)
{
	int mutaIndex = 0;
	for (int i = priorityEnemy.size() - 1; i >= 0; i--)
	{
		int targetHp = priorityEnemy[i].unit->getHitPoints() + priorityEnemy[i].unit->getShields();
		if (i > 0)
		{
			int needMutaCount;
			//terran building can be repair..
			if (priorityEnemy[i].unit->getType() == BWAPI::UnitTypes::Terran_Bunker || priorityEnemy[i].unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)
				needMutaCount = (targetHp / BWAPI::UnitTypes::Zerg_Mutalisk.airWeapon().damageAmount()) + 10;
			else if (priorityEnemy[i].unit->getType().isBuilding() && priorityEnemy[i].unit->getType().canAttack())
				needMutaCount = (targetHp / BWAPI::UnitTypes::Zerg_Mutalisk.airWeapon().damageAmount()) + 5;
			else
				needMutaCount = (targetHp / BWAPI::UnitTypes::Zerg_Mutalisk.airWeapon().damageAmount()) + 2;
			
			if (needMutaCount <= int(units.size()) - mutaIndex)
			{
				while (needMutaCount > 0)
				{
					mutaliskFSM(units[mutaIndex], priorityEnemy[i].unit);
					needMutaCount--;
					mutaIndex++;
				}
			}
			else
			{
				while (mutaIndex < int(units.size()))
				{
					mutaliskFSM(units[mutaIndex], priorityEnemy[i].unit);
					mutaIndex++;
				}
				break;
			}
		}
		// the last enemy 
		else
		{
			while (mutaIndex < int(units.size()))
			{
				mutaliskFSM(units[mutaIndex], priorityEnemy[i].unit);
				mutaIndex++;
			}
			break;
		}
	}
}

int MutaliskArmy::harassAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if (unit->isRepairing())
	{
		return 14;
	}
	else if (type.airWeapon() != BWAPI::WeaponTypes::None ||
		type == BWAPI::UnitTypes::Protoss_Carrier)
	{
		return 13;
	}
	//not sure the bunker's inside info
	else if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 12;
	}
	else if (type.isWorker())
	{
		return 11;
	}
	else if (type == BWAPI::UnitTypes::Zerg_Overlord)
	{
		return 10;
	}
	else if (type.gasPrice() > 0)
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
	
	else if (type.groundWeapon() != BWAPI::WeaponTypes::None)
	{
		return 6;
	}



	else if (type.isBuilding())
	{
		return 5;
	}
	// next is buildings that cost gas

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


int MutaliskArmy::getAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat

	if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode
		|| type == BWAPI::UnitTypes::Protoss_High_Templar)
	{
		return 14;
	}
	else if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
	{
		return 13;
	}
	else if ((type.airWeapon() != BWAPI::WeaponTypes::None) ||
		type == BWAPI::UnitTypes::Protoss_Carrier)
	{
		return 12;
	}
	else if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 11;
	}
	else if (type.groundWeapon() != BWAPI::WeaponTypes::None)
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

