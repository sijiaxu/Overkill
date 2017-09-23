#include "BattleArmy.h"


std::vector<EnemyUnit> BattleArmy::mixAttack(BWAPI::Position targetPosition, std::set<BWAPI::Unit> enemySet)
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

			if (armyType.airWeapon() == BWAPI::WeaponTypes::None)
			{
				if (u->getType().isFlyer())
					continue;
			}

			if (armyType.groundWeapon() == BWAPI::WeaponTypes::None)
			{
				if (!u->getType().isFlyer())
					continue;
			}
			priorityEnemy.push_back(EnemyUnit(u, getAttackPriority(u), u->getID() ));
		}
	}

	return priorityEnemy;
}


int	BattleArmy::onlyGroundAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	// highest priority is something that can attack us or aid in combat
	if (type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isFlyer())
	{
		return 11;
	}
	else if (type.isSpellcaster() && !type.isFlyer())
	{
		return 11;
	}
	else if (type.airWeapon() != BWAPI::WeaponTypes::None && !type.isFlyer())
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
		return 7;
	}
	else if (type.isRefinery())
	{
		return 6;
	}
	else if (type.isResourceDepot())
	{
		return 5;
	}
	else if (type.isBuilding())
	{
		return 4;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 3;
	}
	else if (type.mineralPrice() > 0)
	{
		return 2;
	}
	// then everything else
	else
	{
		return 1;
	}
}


int	BattleArmy::onlyAirAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if ((type.airWeapon() != BWAPI::WeaponTypes::None) && type.isFlyer())
	{
		return 12;
	}
	else if ((type.groundWeapon() != BWAPI::WeaponTypes::None) && type.isFlyer())
	{
		return 11;
	}
	else if (type.isFlyer())
	{
		return 10;
	}
	else
	{
		return 1;
	}
}


int	BattleArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType curType = units.front().unit->getType();
	if (curType.groundWeapon() == BWAPI::WeaponTypes::None)
	{
		return onlyAirAttackPriority(unit);
	}
	else if (curType.airWeapon() == BWAPI::WeaponTypes::None)
	{
		return onlyGroundAttackPriority(unit);
	}
	else
	{
		return onlyGroundAttackPriority(unit);
	}
}


void BattleArmy::addUnit(BWAPI::Unit u)
{
	if (u == NULL)
		return;

	units.push_back(UnitState(u));
}


int	BattleArmy::calMaxAssign(BWAPI::Unit enemyUnit, BWAPI::UnitType ourType)
{
	BWAPI::UnitSizeType enemySize = enemyUnit->getType().size();
	int targetHp = enemyUnit->getHitPoints() + enemyUnit->getShields();
	int ourDamage = 0;

	if (ourType == BWAPI::UnitTypes::Zerg_Zergling)
	{
		if (!enemyUnit->getType().isBuilding())
		{
			if (enemySize == BWAPI::UnitSizeTypes::Small)
			{
				return 4;
			}
			else if (enemySize == BWAPI::UnitSizeTypes::Medium)
			{
				return 6;
			}
			else if (enemySize == BWAPI::UnitSizeTypes::Large)
			{
				return 8;
			}
		}
		else
		{
			return 20;
		}
	}
	//ground unit do not assign too many
	else if (ourType == BWAPI::UnitTypes::Zerg_Hydralisk)
	{
		ourDamage = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().damageAmount();
		int maxCount = (targetHp / (ourDamage * 2)) > 20 ? 20 : (targetHp / ourDamage) + 3;
		return maxCount;
	}
	else if (ourType == BWAPI::UnitTypes::Zerg_Mutalisk
		|| ourType == BWAPI::UnitTypes::Zerg_Devourer)
	{
		ourDamage = ourType.airWeapon().damageAmount();
		int maxCount = (targetHp / ourDamage) + 2 > 10 ? 10 : (targetHp / ourDamage) + 2;
		return maxCount;
	}
	else if (ourType == BWAPI::UnitTypes::Zerg_Scourge)
	{
		ourDamage = BWAPI::UnitTypes::Zerg_Scourge.airWeapon().damageAmount();
		int maxCount = (targetHp / ourDamage) + 1;
		return maxCount;
	}

	else if (ourType == BWAPI::UnitTypes::Zerg_Guardian
		|| ourType == BWAPI::UnitTypes::Zerg_Ultralisk
		|| ourType == BWAPI::UnitTypes::Zerg_Lurker)
	{
		ourDamage = ourType.groundWeapon().damageAmount();
		int maxCount = (targetHp / ourDamage) + 1;
		return maxCount;
	}
	else
	{
		return 1;
	}

	return -1;
}

//assign enemy target. mainly for air unit to attack, ground unit is attack reactively
void BattleArmy::assignTarget(std::vector<EnemyUnit>& priorityEnemy, BWAPI::UnitType armyType, BWAPI::Position targetPosition, std::set<BWAPI::Unit>& enemySet, std::set<BWAPI::Unit>& attackingUnits)
{
	/*
	//if there are too many enemy exist, select top 30 enemy.
	if (priorityEnemy.size() >= 50)
	{
		priorityEnemy.erase(priorityEnemy.begin(), priorityEnemy.end() - 50);
	}
	std::sort(priorityEnemy.begin(), priorityEnemy.end());

	int armyIndex = 0;
	for (int i = priorityEnemy.size() - 1; i >= 0; i--)
	{
		int targetPriority = priorityEnemy[i].priority;
		int targetHp = priorityEnemy[i].unit->getHitPoints() + priorityEnemy[i].unit->getShields();
		int needCount = calMaxAssign(priorityEnemy[i].unit, armyType);

		for (; armyIndex < int(units.size()); armyIndex++)
		{
			EnemyUnit originalTarget = units[armyIndex].target;
			if (originalTarget.unit == NULL || !originalTarget.unit->exists() || !originalTarget.unit->isVisible()
				|| originalTarget.priority < targetPriority)
			{
				units[armyIndex].target = priorityEnemy[i];
				needCount--;
			}
			if (units[armyIndex].target.unit != NULL)
				attackingUnits.insert(units[armyIndex].target.unit);
			attackMoveLowHealth(units[armyIndex].unit, units[armyIndex].target.unit->getPosition(), units[armyIndex].target.unit);
			if (needCount == 0)
			{
				enemySet.erase(priorityEnemy[i].unit);
				armyIndex++;
				break;
			}
		}
	}

	//enemy is too little or already assigned
	if (armyIndex < int(units.size()))
	{
		for (; armyIndex < int(units.size()); armyIndex++)
		{
			EnemyUnit originalTarget = units[armyIndex].target;
			if (units[armyIndex].target.unit != NULL)
				attackingUnits.insert(units[armyIndex].target.unit);
			if (originalTarget.unit == NULL || !originalTarget.unit->exists() || !originalTarget.unit->isVisible())
			{
				units[armyIndex].target = EnemyUnit();
				attackMoveLowHealth(units[armyIndex].unit, targetPosition);
			}
			else
			{
				attackMoveLowHealth(units[armyIndex].unit, units[armyIndex].target.unit->getPosition(), units[armyIndex].target.unit);
			}
		}
	}*/
}


void BattleArmy::attackMoveLowHealth(BWAPI::Unit attacker, BWAPI::Position targetPosition, BWAPI::Unit targetUnit)
{
	if (attacker->isIrradiated() || attacker->isUnderStorm())
	{
		smartMove(attacker, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		return;
	}

	bool isUnderAttack = attacker->isUnderAttack();
	BWAPI::UnitType armyType = attacker->getType();
	int attackRange = 0;
	//flyer unit can attack enemy more flexible
	if (armyType.isFlyer() && armyType != BWAPI::UnitTypes::Zerg_Guardian)
	{
		if (armyType == BWAPI::UnitTypes::Zerg_Scourge)
			attackRange = 12 * 32;
		else
			attackRange = armyType.sightRange();
	}
	else
	{
		attackRange = armyType.groundWeapon().maxRange();
	}

	BWAPI::Unit target = NULL;
	// if target is in attack range, attack it first
	if (targetUnit != NULL && attacker->getDistance(targetUnit) < attackRange)
	{
		target = targetUnit;
	}
	// scourge only attack the enemy by assign function
	else if (armyType == BWAPI::UnitTypes::Zerg_Scourge && targetUnit == NULL)
	{
		target = NULL;
	}
	else
	{
		BWAPI::Unitset nearbyEnemys = BWAPI::Broodwar->getUnitsInRadius(attacker->getPosition(), attackRange, BWAPI::Filter::IsEnemy);
		int hightPriority = -99999;
		int lowHealth = 99999;

		for (auto u : nearbyEnemys)
		{
			if (!u->isDetected() && u->isVisible())
				continue;

			if (armyType.airWeapon() == BWAPI::WeaponTypes::None)
			{
				if (u->getType().isFlyer())
					continue;
			}

			if (armyType.groundWeapon() == BWAPI::WeaponTypes::None)
			{
				if (!u->getType().isFlyer())
					continue;
			}

			//if has target, do not attack the building first
			if (targetUnit != NULL && (targetUnit->getType().canAttack() || targetUnit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				&& (!u->getType().canAttack() && u->getType() != BWAPI::UnitTypes::Terran_Bunker))
			{
				continue;
			}

			int enemyPriority = getAttackPriority(u);
			if (enemyPriority > hightPriority
				|| (enemyPriority == hightPriority && u->getHitPoints() < lowHealth)
				|| target == NULL)
			{
				hightPriority = enemyPriority;
				lowHealth = u->getHitPoints();
				target = u;
			}
		}
	}


	if (target == NULL)
	{
		if (armyType == BWAPI::UnitTypes::Zerg_Lurker)
		{
			if (attacker->isBurrowed())
				attacker->unburrow();
			else
				smartMove(attacker, targetPosition);
		}
		else
			smartMove(attacker, targetPosition);
	}
	else
	{
		if (armyType == BWAPI::UnitTypes::Zerg_Lurker)
		{
			int lurkerRange = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange();
			if (attacker->getDistance(target) < lurkerRange - 1 * 32)
			{
				if (attacker->isBurrowed())
					smartAttackUnit(attacker, target);
				else
					attacker->burrow();
			}
			//for lurker in range(lurkerRange - 1 * 32, lurkerRange)
			else
			{
				if (attacker->isBurrowed())
					smartAttackUnit(attacker, target);
				else
					smartMove(attacker, target->getPosition());
			}
		}
		else
		{
			smartAttackUnit(attacker, target);
		}
	}
}



bool BattleArmy::isInDanger(BWAPI::Unit u)
{
	if (!u->isUnderAttack())
	{
		return false;
	}

	BWAPI::Unitset enemyUnits = u->getUnitsInRadius(u->getType().sightRange());
	BOOST_FOREACH(BWAPI::Unit unit, enemyUnits)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy() &&
			!unit->getType().isBuilding())
		{
			return true;
		}
	}
	return false;
}


void BattleArmy::removeUnit(BWAPI::Unit u)
{
	for (std::vector<UnitState>::iterator it = units.begin(); it != units.end(); it++)
	{
		if (it->unit == u)
		{
			units.erase(it);
			break;
		}
	}
}



void BattleArmy::smartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
{
	if (attacker == NULL || target == NULL)
		return;

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 5)
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// for
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 10))
	{
		return;
	}

	// if nothing prevents it, attack the target
	bool re = attacker->attack(target);
	BWAPI::Color c;
	if (re)
	{
		c = BWAPI::Colors::Red;
	}
	else
	{
		c = BWAPI::Colors::Blue;
	}

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y,
		target->getPosition().x, target->getPosition().y, c);

}

void BattleArmy::smartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition)
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 10)
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've recently already told this unit to attack this target, ignore
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move &&	currentCommand.getTargetPosition() == targetPosition
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 10))
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y,
		targetPosition.x, targetPosition.y,
		BWAPI::Colors::Orange);
}

void BattleArmy::smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition)
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 10)
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move)
		&& (currentCommand.getTargetPosition() == targetPosition)
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 10)
		&& (attacker->isMoving()))
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
	{
		BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y, targetPosition.x, targetPosition.y, BWAPI::Colors::Orange);
		BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y, attacker->getOrderTargetPosition().x, attacker->getOrderTargetPosition().y, BWAPI::Colors::Orange);
	}
}


bool BattleArmy::reGroup(const BWAPI::Position & regroupPosition)
{
	int count = 0;
	// for each of the units we have
	BOOST_FOREACH(UnitState u, units)
	{
		// if the unit is outside the regroup area
		if (u.unit->getDistance(regroupPosition) < 32 * 8)
		{
			count++;
		}
	}

	this->armyMove(regroupPosition);

	if (units.size() > 0 && count * 10 / units.size() >= 8)
		return true;
	else
		return false;
}


bool BattleArmy::preciseReGroup(const BWAPI::Position & regroupPosition)
{
	bool isregroup = true;
	// for each of the units we have
	BOOST_FOREACH(UnitState u, units)
	{
		// if the unit is outside the regroup area
		if (u.unit->getDistance(regroupPosition) > 32 * 5)
		{
			smartAttackMove(u.unit, regroupPosition);
			isregroup = false;
			//smartAttackMove(u.unit, regroupPosition);
		}
		else
		{
			smartAttackMove(u.unit, regroupPosition);
		}
	}

	return isregroup;
}

void BattleArmy::armyAttackMove(BWAPI::Position targetPosition)
{
	BOOST_FOREACH(UnitState u, units)
	{
		if (u.state == Irradiated)
		{
			continue;
		}
		smartAttackMove(u.unit, targetPosition);
	}
}

void BattleArmy::armyMove(BWAPI::Position targetPosition)
{
	BOOST_FOREACH(UnitState u, units)
	{
		if (u.state == Irradiated)
		{
			continue;
		}

		smartMove(u.unit, targetPosition);
	}
}

void BattleArmy::moveAbnormalUnits()
{
	for (auto u : units)
	{
		if (u.unit->isIrradiated() || u.unit->isUnderStorm())
		{
			smartMove(u.unit, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		}
	}
}


