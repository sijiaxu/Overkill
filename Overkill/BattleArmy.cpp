#include "BattleArmy.h"


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
		if (enemySize == BWAPI::UnitSizeTypes::Small)
		{
			return 6;
		}
		else if (enemySize == BWAPI::UnitSizeTypes::Medium)
		{
			return 10;
		}
		else if (enemySize == BWAPI::UnitSizeTypes::Large)
		{
			return 16;
		}
		else
		{
			return 6;
		}
	}
	else if (ourType == BWAPI::UnitTypes::Zerg_Hydralisk)
	{
		ourDamage = BWAPI::UnitTypes::Zerg_Hydralisk.groundWeapon().damageAmount();
		int maxCount = (targetHp / ourDamage) + 6 > 20 ? 20 : (targetHp / ourDamage) + 6;
		return maxCount;
	}
	else if (ourType == BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		ourDamage = BWAPI::UnitTypes::Zerg_Mutalisk.airWeapon().damageAmount() + 3;
	}
	else
	{
		return -1;
	}

	return -1;
}


void BattleArmy::assignTarget(std::vector<EnemyUnit>& priorityEnemy, BWAPI::UnitType armyType, BWAPI::Position targetPosition)
{
	moveAbnormalUnits();

	//if there are too many enemy exist, select top 30 enemy.
	if (priorityEnemy.size() >= 30)
	{
		std::sort(priorityEnemy.begin(), priorityEnemy.end());
		priorityEnemy.erase(priorityEnemy.begin(), priorityEnemy.end() - 30);
	}

	for (auto u : units)
	{
		if (u.state == Irradiated)
		{
			continue;
		}

		int closetDist = 99999;
		int highPriority = 0;
		int notFullAssignUnitIndex = -1;
		int maxPriorityUnitIndex = -1;
		for (int i = 0; i < int(priorityEnemy.size()); i++)
		{
			int distance = u.unit->getDistance(priorityEnemy[i].unit);

			if ((priorityEnemy[i].priority > highPriority)
				|| (priorityEnemy[i].priority == highPriority && distance < closetDist))
			{
				if (priorityEnemy[i].assignCount <= calMaxAssign(priorityEnemy[i].unit, armyType))
				{
					notFullAssignUnitIndex = i;
					closetDist = distance;
					highPriority = priorityEnemy[i].priority;
				}
				else if (BWTA::getRegion(targetPosition) == BWTA::getRegion(priorityEnemy[i].unit->getPosition()))
				{
					closetDist = distance;
					highPriority = priorityEnemy[i].priority;
					maxPriorityUnitIndex = i;
				}
				else
					continue;
			}
		}
		
		if (maxPriorityUnitIndex == -1 && notFullAssignUnitIndex == -1)//(priorityEnemy.size() == 0)
		{
			smartMove(u.unit, targetPosition);
		}
		else
		{
			//if all enemy unit have been assign, attack the highest priority enemy
			if (notFullAssignUnitIndex == -1)
			{
				smartAttackUnit(u.unit, priorityEnemy[maxPriorityUnitIndex].unit);
			}
			else
			{
				smartAttackUnit(u.unit, priorityEnemy[notFullAssignUnitIndex].unit);
				priorityEnemy[notFullAssignUnitIndex].assignCount += 1;
			}
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
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 10)
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
		if (u.state == Irradiated)
		{
			smartMove(u.unit, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		}
	}
}


