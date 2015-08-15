#pragma once
#include "TacticManager.h"


TacticManager& TacticManager::Instance()
{
	static TacticManager a;
	return a;
}


void TacticManager::addTactic(tacticType tactic, BWAPI::Position attackPosition)
{
	// already have the same tactic
	if (myTactic.find(tactic) != myTactic.end())
		return;

	if (tactic == ZerglingHarassTac)
	{
		myTactic[tactic] = new sixZerglingTactic();
	}
	else if (tactic == MutaliskHarassTac)
	{
		myTactic[tactic] = new MutaliskHarassTactic();
	}
	else if (tactic == HydraliskPushTactic)
	{
		myTactic[tactic] = new HydraliskTactic();
	}

	myTactic[tactic]->setAttackPosition(attackPosition);
}


void TacticManager::addTacticUnit(tacticType tactic, BWAPI::Unit * unit)
{
	if (myTactic.find(tactic) == myTactic.end())
		return;
	myTactic[tactic]->addArmyUnit(unit);

}

void TacticManager::initTacticArmy(tacticType tactic, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count)
{
	if (myTactic.find(tactic) == myTactic.end())
		return;

	std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy = myTactic[tactic]->getArmy();
	std::vector<UnitState>& targetArmy = tacticArmy[unitType]->getUnits();
	std::vector<UnitState>& sourceArmy = Army[unitType]->getUnits();

	targetArmy.insert(targetArmy.begin(), sourceArmy.begin(), sourceArmy.begin() += count);
	sourceArmy.erase(sourceArmy.begin(), sourceArmy.begin() += count);
}


void TacticManager::update()
{
	typedef std::pair<tacticType, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->update();
	}

	checkTacticEnd();

}


bool TacticManager::isTacticRun(tacticType tactic)
{
	if (myTactic.find(tactic) != myTactic.end())
		return true;
	else
		return false;
}


void TacticManager::onUnitDestroy(BWAPI::Unit * unit)
{
	typedef std::pair<tacticType, BattleTactic*> mytype;

	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->onUnitDestroy(unit);
	}
}


void TacticManager::checkTacticEnd()
{
	for (std::map<tacticType, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end();)
	{
		if (it->second->isTacticEnd())
		{
			BWAPI::Broodwar->printf("tactic %d end", int(it->first));
			it->second->addAllNewArmy();
			AttackManager::Instance().addTacticRemainArmy(it->second->getArmy(), it->second->getOriginAttackPosition());
			delete it->second;
			myTactic.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

void TacticManager::onUnitShow(BWAPI::Unit* unit)
{
	typedef std::pair<tacticType, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->onUnitShow(unit);
	}
}