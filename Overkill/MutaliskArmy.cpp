#include "MutaliskArmy.h"



void MutaliskArmy::defend(BWAPI::Position targetPosition)
{
	//attack(targetPosition);

	
	if (units.size() == 0)
		return;

	BWAPI::Unit* unit = (*units.begin()).unit;
	BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 8 * 32, BWAPI::Colors::Purple, false);

	std::set<BWAPI::Unit*> enemySetNearBy = unit->getUnitsInRadius(8 * 32);
	std::set<BWAPI::Unit*> enemySetAtPosition = BWAPI::Broodwar->getUnitsInRadius(targetPosition, 8 * 32);
	std::set<BWAPI::Unit*> enemySet;

	//attack the unit in target circle first
	if (enemySetAtPosition.size() > 0)
		enemySet = enemySetAtPosition;
	else
		enemySet = enemySetNearBy;

	int closetDist = 99999;
	int highPriority = 0;
	BWAPI::Unit* closet = NULL;

	//get the target unit
	BOOST_FOREACH(BWAPI::Unit* u, enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType() != BWAPI::UnitTypes::Zerg_Larva)
		{
			int priority = getAttackPriority(u);
			int distance = unit->getDistance(u);

			// attack high priority first, if have multiple high priority unit, attack closet first 
			if (!closet || (priority > highPriority) || (priority == highPriority && distance < closetDist))
			{
				closetDist = distance;
				highPriority = priority;
				closet = u;
			}
		}
	}

	if (closet != NULL)
	{
		BOOST_FOREACH(UnitState& u, units)
		{
			mutaliskFSM(u, closet);
		}
	}
	else
	{
		BOOST_FOREACH(UnitState& u, units)
		{
			smartAttackMove(u.unit, targetPosition);
		}
	}
}

void MutaliskArmy::mixArmyAttack(BWAPI::Position priorityPosition)
{
	//get the group attack target
	if (units.size() == 0)
		return;

	BWAPI::Unit* unit = (*units.begin()).unit;

	std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 12 * 32);
	std::set<BWAPI::Unit*> enemySet = enemySetInCircle;

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

	std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(8 * 32);
	std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 8 * 32);

	enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

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

	int x = target->getTilePosition().x();//myUnit.unit->getTilePosition().x();
	int y = target->getTilePosition().y();//myUnit.unit->getTilePosition().y();
	int length = 1;
	int j = 0;
	bool first = true;
	int dx = 0;
	int dy = 1;

	while (length < 20)//BWAPI::Broodwar->mapWidth())
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
		{
			if (!avoidCannon && influnceMap[y][x].airForce == 0 && influnceMap[y][x].enemyUnitAirForce == 0)
			{
				return BWAPI::Position(x * 32, y * 32);
			}

			if (avoidCannon && influnceMap[y][x].airForce == 0)
			{
				int buildingWidth = target->getType().tileWidth();
				int buildingHeight = target->getType().tileHeight();
				int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;
				if (BWAPI::TilePosition(x, y).getDistance(BWAPI::TilePosition(target->getPosition())) <= mutaliskAttackRange + maxSize)
					return BWAPI::Position(x * 32, y * 32);
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

		if (target->getType() != BWAPI::UnitTypes::Protoss_Photon_Cannon 
			&& target->getType() != BWAPI::UnitTypes::Terran_Missile_Turret
			&& target->getType() != BWAPI::UnitTypes::Zerg_Spore_Colony
			&& target->getType() != BWAPI::UnitTypes::Terran_Bunker
			&& influnceMap[unitCenter.y()][unitCenter.x()].airForce > 0
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
		}

		//only do retreat at necessary time, retreat interval is 30 seconds
		if (BWAPI::Broodwar->getFrameCount() > myUnit.nextRetreatFrame && myUnit.unit->getHitPoints() < myUnit.unit->getType().maxHitPoints() / 4 && myUnit.deltaDamge > 0)
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
			myUnit.nextRetreatFrame = BWAPI::Broodwar->getFrameCount() + 25 * 1;
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
bool MutaliskArmy::harassAttack(BWAPI::Position priorityPosition)
{
	//get the group attack target
	if (units.size() == 0)
		return true;

	BWAPI::Unit* unit = (*units.begin()).unit;
	BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 8 * 32, BWAPI::Colors::Purple, false);
	BWAPI::Broodwar->drawCircleMap(priorityPosition.x(), priorityPosition.y(), 8 * 32, BWAPI::Colors::Purple, false);

	std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(8 * 32);
	std::set<BWAPI::Unit*> enemySetInCircle = BWAPI::Broodwar->getUnitsInRadius(priorityPosition, 8 * 32);
	enemySet.insert(enemySetInCircle.begin(), enemySetInCircle.end());

	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
	int mutaliskAttackRange = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().maxRange() / 32;
	int mutaliskAttackDamage = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().damageAmount();
	
	// loop through enemySet to get the top n target for n mutalisk group
	std::vector<EnemyUnit> priorityEnemy;
	priorityEnemy.reserve(100);

	BOOST_FOREACH(BWAPI::Unit* u, enemySet)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy()) // && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(priorityPosition))
		{
			if (u->getType() == BWAPI::UnitTypes::Terran_Missile_Turret ||
				u->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
				u->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony ||
				u->getType() == BWAPI::UnitTypes::Terran_Bunker)
			{
				if (imInfo[u->getTilePosition().y() + 1][u->getTilePosition().x() + 1].airForce / 20 <= units.size() / 4)
				{
					priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
					BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 4, BWAPI::Colors::Red, true);
					continue;
				}
			}
			else
			{
				//TODO:: invisiable bug 
				if (u->getType() == BWAPI::UnitTypes::Protoss_Observer)
					continue;
				BWAPI::Position tmp = findSafePlace(u, true);
				if (tmp != BWAPI::Positions::None)
				{
					priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
					BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 4, BWAPI::Colors::Red, true);
				}

				/*
				double2 initPosition(u->getTilePosition().x(), u->getTilePosition().y());
				double2 length = double2(1, 0) * mutaliskAttackRange;
				if (u->getType().isBuilding())
				{
					int buildingWidth = u->getType().tileWidth();
					int buildingHeight = u->getType().tileHeight();
					initPosition.x += buildingWidth / 2;
					initPosition.y += buildingHeight / 2;
					int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;
					length = double2(1, 0) * (mutaliskAttackRange + maxSize);
				}

				int startDegree = 0;
				while (startDegree < 360)
				{
					double2 rotateVector(length.rotateReturn(startDegree) + initPosition);
					// if enemy has at least one position to attack, add to priority list
					if (int(rotateVector.x) >= 0 && int(rotateVector.x) < BWAPI::Broodwar->mapWidth() && int(rotateVector.y) >= 0 && int(rotateVector.y) < BWAPI::Broodwar->mapHeight())
					{
						if (imInfo[int(rotateVector.y)][int(rotateVector.x)].airForce == 0)
						{
							priorityEnemy.push_back(EnemyUnit(u, harassAttackPriority(u), unit->getDistance(u)));
							BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 4, BWAPI::Colors::Red, true);
							break;
						}
					}
					startDegree += 30;
				}*/
			}
		}
	}

	if (priorityEnemy.size() == 0)
	{
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
	int targetHp = priorityEnemy.back().unit->getType().maxHitPoints() + priorityEnemy.back().unit->getType().maxShields();
	int groupNum = (targetHp / BWAPI::UnitTypes::Zerg_Mutalisk.airWeapon().damageAmount()) + 1;
	int MutaliskGroupCount = units.size() / groupNum; //units.size() / 12;

	for (int i = 0; i <= MutaliskGroupCount; i++)
	{
		BWAPI::Unit* u = priorityEnemy.back().unit;
		priorityEnemy.pop_back();

		if (priorityEnemy.size() > 0)
		{
			//the last mutalisk group 
			if (i == MutaliskGroupCount)
			{
				for (unsigned j = groupNum * i; j < units.size(); j++)
				{
					mutaliskFSM(units[j], u);
				}
			}
			else
			{
				for (int j = groupNum * i; j < groupNum * (i + 1); j++)
				{
					mutaliskFSM(units[j], u);
				}
			}
		}
		else
		{
			//if target enemy is only one
			for (unsigned j = groupNum * i; j < units.size(); j++)
			{
				mutaliskFSM(units[j], u);
			}
			break;
		}
	}
}

int MutaliskArmy::harassAttackPriority(BWAPI::Unit * unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Terran_Missile_Turret ||
		type == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
		type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
		type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 12;
	}
	else if ((type.airWeapon() != BWAPI::WeaponTypes::None) ||
		type == BWAPI::UnitTypes::Protoss_Carrier ||
		type == BWAPI::UnitTypes::Terran_Bunker)
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
		return 13;
	}
	else if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
	{
		return 12;
	}
	else if ((type.airWeapon() != BWAPI::WeaponTypes::None) ||
		type == BWAPI::UnitTypes::Protoss_Carrier ||
		type == BWAPI::UnitTypes::Terran_Bunker)
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

