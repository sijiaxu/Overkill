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

	if (tactic == MutaliskHarassTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new MutaliskHarassTactic();
	}
	else if (tactic == HydraliskPushTactic)
	{
		myTactic[tacKey(tactic, attackPosition)] = new HydraliskTactic();
	}
	else if (tactic == DefendTactic)
	{
		//BWAPI::Broodwar->printf("trigger defend!!!!");
		myTactic[tacKey(tactic, attackPosition)] = new ArmyDefendTactic();
	}
	else if (tactic == ScoutTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new ScoutTactic();
	}

	myTactic[tacKey(tactic, attackPosition)]->setAttackPosition(attackPosition, tactic);
}


void TacticManager::addTacticUnit(tacticType tactic, BWAPI::Position attackPosition, BWAPI::Unit unit)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end() || !unit->exists())
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

	for (std::vector<UnitState>::iterator it = sourceArmy.begin(); it != sourceArmy.end();)
	{
		if (!it->unit->exists())
		{
			it = sourceArmy.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (int(sourceArmy.size()) >= count)
	{
		targetArmy.insert(targetArmy.end(), sourceArmy.end() -= count, sourceArmy.end());
		sourceArmy.erase(sourceArmy.end() -= count, sourceArmy.end());

		for (auto a : tacticArmy)
		{
			if (a.first != BWAPI::UnitTypes::Zerg_Overlord && a.second->getUnits().size() > 0)
			{
				BWAPI::Position armyStartP = a.second->getUnits().front().unit->getPosition();
				myTactic[tacKey(tactic, attackPosition)]->setStartPosition(armyStartP);
				break;
			}
		}
		
		if (tactic == MutaliskHarassTac)
		{
			myTactic[tacKey(tactic, attackPosition)]->checkStartPositionEnemyInfo();
		}

		//myTactic[tacKey(tactic, attackPosition)]->setGroupPosition(unitType, tactic);
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
		if (tac.first.tacName != DefendTactic && tac.first.tacName != ScoutTac)
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

void TacticManager::onLurkerMorph()
{
	for (auto t : myTactic)
	{
		t.second->onLurkerMorph();
	}
}


void TacticManager::checkTacticEnd()
{
	for (std::map<tacKey, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end();)
	{
		if (it->second->isTacticEnd())
		{
			//BWAPI::Broodwar->printf("tactic %d end", int(it->first.tacName));
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


void TacticManager::assignScoutZergling()
{
	BattleTactic* scoutTac = myTactic.find(tacKey(ScoutTac, BWAPI::Positions::None))->second;
	for (std::map<tacKey, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end(); it++)
	{
		if (it->first.tacName == HydraliskPushTactic )
		{
			int zerglingRemain = it->second->getArmy()[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
			if (zerglingRemain > 0)
			{
				std::vector<UnitState>& targetArmy = it->second->getArmy()[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				BWAPI::Unit z = targetArmy.back().unit;
				addTacticUnit(ScoutTac, BWAPI::Positions::None, z);
				targetArmy.pop_back();
			}
			break;
		}
	}
}


bool TacticManager::isAssignScoutZergling()
{
	ScoutTactic* scoutTac = dynamic_cast<ScoutTactic*>(myTactic.find(tacKey(ScoutTac, BWAPI::Positions::None))->second);
	return scoutTac->HasZergling();
}

int TacticManager::addDefendArmyType(BattleTactic* tac, BWAPI::UnitType addArmyType, int needArmy, BWAPI::Position defendPosition)
{
	if (needArmy <= 0)
	{
		return needArmy;
	}
	int Remain = 0;
	int Send = 0;
	Remain = tac->getArmy()[addArmyType]->getUnits().size();
	int armySupply = addArmyType.supplyRequired();
	Send = needArmy / armySupply < Remain ? needArmy / armySupply : Remain;

	addTacticArmy(DefendTactic, defendPosition, tac->getArmy(), addArmyType, Send);
	needArmy -= Send * armySupply;
	return needArmy;
}


void TacticManager::assignDefendArmy(BWAPI::Position defendPosition, int needSupply, bool allAirEnemy)
{
	int originSupply = needSupply;
	BattleTactic* defendTac = myTactic.find(tacKey(DefendTactic, defendPosition))->second;
	for (std::map<tacKey, BattleTactic*>::iterator it = myTactic.begin(); it != myTactic.end(); it++)
	{
		if (needSupply <= 0)
			break;
		if (it->first.tacName != DefendTactic && it->first.tacName != ScoutTac)
		{
			if (it->first.tacName == MutaliskHarassTac)
			{
				needSupply = addDefendArmyType(it->second, BWAPI::UnitTypes::Zerg_Mutalisk, needSupply, defendPosition);
			}
			else if (it->first.tacName == HydraliskPushTactic)
			{
				needSupply = addDefendArmyType(it->second, BWAPI::UnitTypes::Zerg_Mutalisk, needSupply, defendPosition);
				needSupply = addDefendArmyType(it->second, BWAPI::UnitTypes::Zerg_Hydralisk, needSupply, defendPosition);
				if (allAirEnemy == false)
				{
					needSupply = addDefendArmyType(it->second, BWAPI::UnitTypes::Zerg_Lurker, needSupply, defendPosition);
					needSupply = addDefendArmyType(it->second, BWAPI::UnitTypes::Zerg_Zergling, needSupply, defendPosition);
				}
			}
			else
				continue;

			//if assign army to defend, the remain army maybe not enough, so we need to attack this position again
			if (originSupply - needSupply > 0)
			{
				it->second->setDefendEnd();
			}
			originSupply = needSupply;
		}

	}
}
