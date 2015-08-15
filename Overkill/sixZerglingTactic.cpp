#pragma once
#include "sixZerglingTactic.h"



void sixZerglingTactic::update()
{
	if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
	{
		if (AttackManager::Instance().isNeedRetreatDefend())
			state = END;
	}

	ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Zergling]);

	switch (state)
	{
	case LOCATIONASSIGN:
	{
		generateAttackPath();
		state = MOVE;
		break;
	}
	break;

	case MOVE:
	{
		for (std::vector<BWAPI::Position>::iterator it = movePositions.begin(); it != movePositions.end();)
		{
			bool isGroupComplete = zerglings->preciseReGroup(*it);
			if (isGroupComplete)
			{
				it = movePositions.erase(it);
			}
			else
			{
				break;
			}
		}
		if (movePositions.size() == 0)
		{
			state = ATTACK;
		}
	}
	break;

	case ATTACK:
	{
		if (zerglings->getUnits().size() == 0)
		{
			state = END;
			break;
		}
			
		//if (zerglings->getUnits().size() == 1)
			//state = CIRCLE;

		if (!hasEnemy())
		{
			state = RETREAT;
			break;
		}

		std::set<BWAPI::Unit*>& units = (*zerglings->getUnits().begin()).unit->getUnitsInRadius(8 * 32);
		int cannonCount = 0;
		int zealotCount = 0;
		int marineCount = 0;
		BOOST_FOREACH(BWAPI::Unit* u, units)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->isCompleted())
			{
				if (u->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
					|| u->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony)
					cannonCount++;
				else if (u->getType() == BWAPI::UnitTypes::Protoss_Zealot)
					zealotCount++;
				else if (u->getType() == BWAPI::UnitTypes::Terran_Marine)
					marineCount++;
			}
		}
		if (cannonCount >= 2 || zealotCount >= 2 || marineCount >= 4)
		{
			state = RETREAT;
		}

		zerglings->harassAttack(attackPosition);
	}
	break;

	case RETREAT:
	{
		BWAPI::Position natrualLocation = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
		BWAPI::Position baseChoke = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();

		double2 direc = baseChoke - natrualLocation;
		double2 direcNormal = direc / direc.len();

		int targetx = natrualLocation.x() + int(direcNormal.x * 32 * 5);
		int targety = natrualLocation.y() + int(direcNormal.y * 32 * 5);

		zerglings->armyMove(BWAPI::Position(targetx, targety));
		state = END;
	}
	break;
	}
}

bool sixZerglingTactic::isTacticEnd()
{
	if (state == END)
		return true;
	else
		return false;
}


sixZerglingTactic::sixZerglingTactic()
{
	state = LOCATIONASSIGN;
}


void sixZerglingTactic::generateAttackPath()
{
	movePositions.push_back(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));

}