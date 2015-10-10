#include "BattleArmy.h"


void BattleArmy::addUnit(BWAPI::Unit* u)
{
	if (u == NULL)
		return;

	units.push_back(UnitState(u));
}


bool BattleArmy::isInDanger(BWAPI::Unit* u)
{
	if (!u->isUnderAttack())
	{
		return false;
	}

	std::set<BWAPI::Unit*>& enemyUnits = u->getUnitsInRadius(u->getType().sightRange());
	BOOST_FOREACH(BWAPI::Unit* unit, enemyUnits)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy() &&
			!unit->getType().isBuilding())
		{
			return true;
		}
	}
	return false;
}

void BattleArmy::smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target) const
{
	if (attacker == NULL || target == NULL)
		return;

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x(), attacker->getPosition().y(),
		target->getPosition().x(), target->getPosition().y(),
		BWAPI::Colors::Red);

}

void BattleArmy::smartAttackMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition) const
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move &&	currentCommand.getTargetPosition() == targetPosition)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x(), attacker->getPosition().y(),
		targetPosition.x(), targetPosition.y(),
		BWAPI::Colors::Orange);
}

void BattleArmy::smartMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition) const
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		if (attacker->isSelected())
		{
			return;
		}
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move)
		&& (currentCommand.getTargetPosition() == targetPosition)
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 5)
		&& attacker->isMoving()) 
	{
		if (attacker->isSelected())
		{
			return;
			//BWAPI::Broodwar->printf("Previous Command Frame=%d Pos=(%d, %d)", attacker->getLastCommandFrame(), currentCommand.getTargetPosition().x(), currentCommand.getTargetPosition().y());
		}
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
	{
		BWAPI::Broodwar->drawLineMap(attacker->getPosition().x(), attacker->getPosition().y(),
			targetPosition.x(), targetPosition.y(), BWAPI::Colors::Orange);
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
		smartAttackMove(u.unit, targetPosition);
	}
}

void BattleArmy::armyMove(BWAPI::Position targetPosition)
{
	BOOST_FOREACH(UnitState u, units)
	{
		smartMove(u.unit, targetPosition);
	}
}


