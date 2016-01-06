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
	if (myTactic.find(tacKey(tactic, attackPosition)) != myTactic.end())
		return;

	if (tactic == ZerglingHarassTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new sixZerglingTactic();
	}
	else if (tactic == MutaliskHarassTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new MutaliskHarassTactic();
	}
	else if (tactic == HydraliskPushTactic)
	{
		myTactic[tacKey(tactic, attackPosition)] = new HydraliskTactic();
	}
	else if (tactic == DefendTactic)
	{
		myTactic[tacKey(tactic, attackPosition)] = new ArmyDefendTactic();
	}

	myTactic[tacKey(tactic, attackPosition)]->setAttackPosition(attackPosition);
}


void TacticManager::addTacticUnit(tacticType tactic, BWAPI::Position attackPosition, BWAPI::Unit unit)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end())
		return;
	myTactic[tacKey(tactic, attackPosition)]->addArmyUnit(unit);
}

void TacticManager::addTacticArmy(tacticType tactic, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end() || count == 0)
		return;

	std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy = myTactic[tacKey(tactic, attackPosition)]->getArmy();
	std::vector<UnitState>& targetArmy = tacticArmy[unitType]->getUnits();
	std::vector<UnitState>& sourceArmy = Army[unitType]->getUnits();

	if (int(sourceArmy.size()) >= count)
	{
		targetArmy.insert(targetArmy.end(), sourceArmy.end() -= count, sourceArmy.end());
		sourceArmy.erase(sourceArmy.end() -= count, sourceArmy.end());
	}
}


void TacticManager::update()
{
	typedef std::pair<tacKey, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->update();
	}

	checkTacticEnd();

}


bool TacticManager::isTacticRun(tacticType tactic, BWAPI::Position attackPosition)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) != myTactic.end())
		return true;
	else
		return false;
}

bool TacticManager::isOneTacticRun(tacticType tactic)
{
	typedef std::pair<tacKey, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tac, myTactic)
	{
		if (tac.first.tacName == tactic)
			return true;
	}
	return false;
}

bool TacticManager::isHaveNoneDefendTactic()
{
	typedef std::pair<tacKey, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tac, myTactic)
	{
		if (tac.first.tacName != DefendTactic)
			return true;
	}
	return false;
}

BWAPI::Position TacticManager::getTacticPosition(tacticType tactic)
{
	typedef std::pair<tacKey, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tac, myTactic)
	{
		if (tac.first.tacName == tactic)
			return tac.first.attackPosition;
	}
	return BWAPI::Positions::None;
}

void TacticManager::onUnitDestroy(BWAPI::Unit unit)
{
	typedef std::pair<tacKey, BattleTactic*> mytype;

	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->onUnitDestroy(unit);
	}
}


void TacticManager::checkTacticEnd()
{
	for (std::map<tacKey, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end();)
	{
		if (it->second->isTacticEnd())
		{
			BWAPI::Broodwar->printf("tactic %d end", int(it->first.tacName));
			bool endByDefend = it->second->getDefendEnd();
			it->second->addAllNewArmy();
			AttackManager::Instance().addTacticRemainArmy(it->second->getArmy(), it->first.tacName, it->second->getOriginAttackPosition(), endByDefend);
			delete it->second;
			myTactic.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

void TacticManager::onUnitShow(BWAPI::Unit unit)
{
	typedef std::pair<tacKey, BattleTactic*> mytype;
	BOOST_FOREACH(mytype tactic, myTactic)
	{
		tactic.second->onUnitShow(unit);
	}
}

int TacticManager::getTacArmyForce(tacticType tactic, BWAPI::Position attackPosition)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end())
		return 0;
	std::map<BWAPI::UnitType, BattleArmy*>& tacArmy = myTactic.find(tacKey(tactic, attackPosition))->second->getArmy();
	int defendTacArmySupply = 0;
	for (std::map<BWAPI::UnitType, BattleArmy*>::iterator it = tacArmy.begin(); it != tacArmy.end(); it++)
	{
		defendTacArmySupply += it->first.supplyRequired() * it->second->getUnits().size();
	}
	return defendTacArmySupply;
}


void TacticManager::assignDefendArmy(BWAPI::Position defendPosition, int needSupply)
{
	BattleTactic* defendTac = myTactic.find(tacKey(DefendTactic, defendPosition))->second;
	for (std::map<tacKey, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end(); it++)
	{
		if (needSupply <= 0)
			break;
		if (it->first.tacName != DefendTactic)
		{
			if (it->first.tacName == MutaliskHarassTac)
			{
				//int mapWidth = BWAPI::Broodwar->mapWidth() * 32;
				// not too far away from defend position
				//if (currentArmyPosition.getDistance(defendPosition) < mapWidth / 2)
				//{
				int mutaliskRemain = 0;
				int mutaliskSend = 0;
				mutaliskRemain = it->second->getArmy()[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
				mutaliskSend = needSupply / 4 < mutaliskRemain ? needSupply / 4 : mutaliskRemain;
				if (mutaliskSend == mutaliskRemain)
					it->second->setDefendEnd();

				addTacticArmy(DefendTactic, defendPosition, it->second->getArmy(), BWAPI::UnitTypes::Zerg_Mutalisk, mutaliskSend);
				needSupply -= mutaliskSend * 4;
				//}
			}
			else if (it->first.tacName == HydraliskPushTactic)
			{
				int hydraliskRemain = 0;
				int hydraliskSend = 0;
				//send the mutalisk to defend first, since mutalisk move fast
				hydraliskRemain = it->second->getArmy()[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size();
				hydraliskSend = needSupply / 2 < hydraliskRemain ? needSupply / 2 : hydraliskRemain;
				needSupply -= hydraliskSend * 2;
				addTacticArmy(DefendTactic, defendPosition, it->second->getArmy(), BWAPI::UnitTypes::Zerg_Hydralisk, hydraliskSend);
			}
			else if (it->first.tacName == ZerglingHarassTac)
			{
				int zerglingRemain = 0;
				int zerglingSend = 0;
				zerglingRemain = it->second->getArmy()[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
				zerglingSend = needSupply < zerglingRemain ? needSupply : zerglingRemain;
				if (zerglingSend == zerglingRemain)
					it->second->setDefendEnd();

				needSupply -= zerglingSend;
				addTacticArmy(DefendTactic, defendPosition, it->second->getArmy(), BWAPI::UnitTypes::Zerg_Zergling, zerglingSend);
			}

			else
				continue;
		}
	}
}
