#include "AttackManager.h"



AttackManager::AttackManager()
{
	// do init first
	myArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Lurker] = new LurkerArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Scourge] = new ScourgeArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Ultralisk] = new UltraliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Devourer] = new DevourerArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Guardian] = new GuardianArmy();

	enemyInvadeSet = std::set<BWAPI::Unit>();

	isNeedDefend = false;
	isNeedTacticDefend = false;
	zerglingHarassFlag = true;
	unRallyArmy.reserve(1000);

	defendState = noEnemy;
	recoverLostTime = 0;
	updatPositionTime = 0;

	//get the natural choke 
	BWAPI::Position natrualLocation = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	
	baseChokePosition = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
	double2 direc = baseChokePosition - natrualLocation;
	double2 direcNormal = direc / direc.len();

	int targetx = natrualLocation.x + int(direcNormal.x * 32 * 3);
	int targety = natrualLocation.y + int(direcNormal.y * 32 * 3);

	rallyPosition = BWAPI::Position(targetx, targety);

	triggerZerglingBuilding = true;

	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		if (base->isIsland())
		{
			continue;
		}
		attackPosition.push_back(AttackEnemyBase(base->getPosition(), 0));
	}

	lastAttackPosition = BWAPI::Positions::None;
	defendAddSupply = 0;
	hasWorkerScouter = false;
	isFirstMutaAttack = true;
	isFirstHydraAttack = true;

	BWTA::Region* nextBase = BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation());
	double maxDist = 0;
	BWTA::Chokepoint* maxChoke = nullptr;
	BOOST_FOREACH(BWTA::Chokepoint* p, nextBase->getChokepoints())
	{
		if (InformationManager::Instance().GetOurBaseUnit()->getDistance(p->getCenter()) > maxDist)
		{
			maxDist = InformationManager::Instance().GetOurBaseUnit()->getDistance(p->getCenter());
			naturalChokePosition = p->getCenter();
			maxChoke = p;
		}
	}
}


std::map<BWAPI::UnitType, int> AttackManager::reaminArmy()
{
	std::map<BWAPI::UnitType, int> result;
	for (auto army : myArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			result[army.first] = army.second->getUnits().size();
		}
	}
	return result;
}


void AttackManager::groupArmy()
{
	std::set<BWTA::Region *>& myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (myRegions.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
		rallyPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		rallyPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	//battle unit rally to the natural choke center
	for (std::vector<BWAPI::Unit>::iterator it = unRallyArmy.begin(); it != unRallyArmy.end();)
	{
		if ((*it)->canIssueCommand(BWAPI::UnitCommand(*it, BWAPI::UnitCommandTypes::Move, NULL, rallyPosition.x, rallyPosition.y, 0)))
		{
			(*it)->attack(rallyPosition);
			myArmy[(*it)->getType()]->addUnit(*it);
			it = unRallyArmy.erase(it);
		}
		else
		{
			it++;
		}
	}

	int count = 0;
	bool isArmyUnderAttack = false;
	BWAPI::Position underAttackPosition;
	//group by outside unit
	for (auto armyUnits : myArmy)
	{
		if (armyUnits.first == BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : armyUnits.second->getUnits())
			{
				u.unit->stop();
			}
			continue;
		}

		for (auto u : armyUnits.second->getUnits())
		{
			if (BWTA::getRegion(u.unit->getPosition()) == BWTA::getRegion(rallyPosition))
			{
				continue;
			}
			BattleArmy::smartMove(u.unit, rallyPosition);
			BWAPI::Broodwar->drawLineMap(u.unit->getPosition().x, u.unit->getPosition().y, rallyPosition.x, rallyPosition.y, BWAPI::Colors::Orange);

			if (u.unit->isUnderAttack())
			{
				isArmyUnderAttack = true;
				underAttackPosition = u.unit->getPosition();
			}
		}
	}

	if (isArmyUnderAttack)
	{
		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			triggerTactic(HydraliskPushTactic, underAttackPosition);
			BWAPI::Broodwar->printf("trigger new all-in attack");
		}
	}


}


// attack manger trigger for two situation:defend and attack
void AttackManager::update()
{
	/*
	BWAPI::TilePosition target;
	for (auto t : BWAPI::Broodwar->getStartLocations())
	{
		if (t != BWAPI::Broodwar->self()->getStartLocation())
		{
			target = t;
			break;
		}
	}
	
	std::list<BWAPI::Position> tmp = aStarGroundPathFinding(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Position(target));

	for (std::list<BWAPI::Position>::iterator it = tmp.begin(); it != tmp.end(); it++)
	{
		std::list<BWAPI::Position>::iterator it2 = it;
		it2++;
		if (it2 != tmp.end())
		{
			BWAPI::Broodwar->drawLineMap(it->x, it->y, it2->x, it2->y, BWAPI::Colors::Purple);
		}
	}
	*/

	if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
		groupArmy();

	DefendUpdate();
	ScoutUpdate();

	//add new morphed unit to tactic
	if (TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
	{
		triggerTactic(MutaliskHarassTac, BWAPI::Positions::None);
	}
	if (TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
	{
		triggerTactic(HydraliskPushTactic, BWAPI::Positions::None);
	}


	/*
	//if position has been eliminate or enemy is too strong at the position, change attack position
	BWAPI::Position attackPosition;
	for (int i = 0; i < int(tactictypeEnd); i++)
	{

		if (tacticType(i) == MutaliskHarassTac)
			attackPosition = getNextAttackPosition(false);
		else
			attackPosition = getNextAttackPosition(true);
		if (attackPosition == BWAPI::Positions::None)
			return;
		triggerTactic(tacticType(i), attackPosition);
	}
	*/
	
}


void AttackManager::issueAttackCommand(tacticType type, BWAPI::Position attackPosition)
{
	if (attackPosition == BWAPI::Positions::None)
	{
		if (type == MutaliskHarassTac)
			attackPosition = getNextAttackPosition(false);
		else
			attackPosition = getNextAttackPosition(true);
	}
	
	if (attackPosition == BWAPI::Positions::None)
		return;
	triggerTactic(type, attackPosition);
}


void AttackManager::ScoutUpdate()
{
	int curOverlordCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Overlord);
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();

	bool isTacticRunning = TacticManager::Instance().isOneTacticRun(ScoutTac);
	if (!isTacticRunning)
	{
		TacticManager::Instance().addTactic(ScoutTac, BWAPI::Positions::None);
		std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
		for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
		{
			TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
			it = army.erase(it);
		}
	}
	else
	{
		if (curOverlordCount - army.size() < 4)
		{
			for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
			{
				//can generate safe attack path 
				TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
				it = army.erase(it);
			}
		}

		if (TacticManager::Instance().isAssignScoutZergling())
			return;

		int checkInterval = 0;
		if (BWAPI::Broodwar->getFrameCount() >= 4 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 7 * 24 * 60)
			checkInterval = 24 * 40;
		else if (BWAPI::Broodwar->getFrameCount() >= 7 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 11 * 24 * 60)
			checkInterval = 24 * 70;
		else
			checkInterval = 24 * 100;

		if (BWAPI::Broodwar->getFrameCount() > nextScoutTime)
		{
			nextScoutTime = BWAPI::Broodwar->getFrameCount() + checkInterval;

			int armyZerglingCount = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
			if (armyZerglingCount == 0)
			{
				TacticManager::Instance().assignScoutZergling();
			}
			else
			{
				std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
				{
					TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
					it = army.erase(it);
					break;
				}
			}
		}
	}

}


void AttackManager::triggerTactic(tacticType tacType, BWAPI::Position attackPosition)
{
	bool isTacticRunning;

	switch (tacType)
	{
		
	case MutaliskHarassTac:
	{
		//trigger mytalisk harass
		isTacticRunning = TacticManager::Instance().isOneTacticRun(MutaliskHarassTac);
		if (isTacticRunning)
		{
			std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits();
			BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(MutaliskHarassTac);
			for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
			{
				//can generate safe attack path 
				TacticManager::Instance().addTacticUnit(MutaliskHarassTac, nowAttack, it->unit);
				it = army.erase(it);
			}
		}
		else
		{
			if (tacticTrigCondition(MutaliskHarassTac, attackPosition))
			{
				//popAttackPosition(attackPosition);

				TacticManager::Instance().addTactic(MutaliskHarassTac, attackPosition);
				TacticManager::Instance().addTacticArmy(MutaliskHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Mutalisk, myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
			}
		}
	}
		break;
		
	case HydraliskPushTactic:
	{
		// trigger hydrisk attack
		isTacticRunning = TacticManager::Instance().isOneTacticRun(HydraliskPushTactic);
		if (isTacticRunning)
		{
			if (hasWorkerScouter)
			{
				std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				int addCount = army.size() - 2 >= 0 ? army.size() - 2 : 0;
				if (addCount >= 0)
				{
					BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(HydraliskPushTactic);
					for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
					{
						if (addCount <= 0)
							break;
						addCount--;

						TacticManager::Instance().addTacticUnit(HydraliskPushTactic, nowAttack, it->unit);
						it = army.erase(it);
					}
				}
				return;
			}

			BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(HydraliskPushTactic);
			for (auto army : myArmy)
			{
				if (army.first == BWAPI::UnitTypes::Zerg_Overlord)
					continue;
				for (std::vector<UnitState>::iterator it = army.second->getUnits().begin(); it != army.second->getUnits().end();)
				{
					TacticManager::Instance().addTacticUnit(HydraliskPushTactic, nowAttack, it->unit);
					it = army.second->getUnits().erase(it);
				}
			}

		}
		else
		{
			if (tacticTrigCondition(HydraliskPushTactic, attackPosition))
			{
				TacticManager::Instance().addTactic(HydraliskPushTactic, attackPosition);
				for (auto army : myArmy)
				{
					if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
					{
						if (army.first == BWAPI::UnitTypes::Zerg_Zergling && hasWorkerScouter && army.second->getUnits().size() > 2)
							TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size() - 2);
						else
							TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size());
					}
				}

				if (InformationManager::Instance().isEnemyHasInvisibleUnit() && BWAPI::Broodwar->getFrameCount() > 5 * 24 * 60)
				{
					TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Overlord, 2);
				}
			}
		}
	}
		break;
	case DefendTactic:
		break;

	default:
		break;
	}
}


bool AttackManager::tacticTrigCondition(int tac, BWAPI::Position attackPosition)
{
	if (TacticManager::Instance().isTacticRun(tacticType(tac), attackPosition))
		return false;

	if (tac == int(MutaliskHarassTac))
	{
		//return !StrategyManager::Instance().mutaliskHarassFlag() && myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 12; //&& isArmyHealthy(BWAPI::UnitTypes::Zerg_Mutalisk);
		/*
		if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran && isFirstMutaAttack)
		{
			if (myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 6)
				isFirstMutaAttack = false;
			return myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 6;
		}
		else */
		return myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() > 0;
	}
	//else if (tac == int(ZerglingHarassTac))
	//{
		//return myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 0;
	//}
	else if (tac == int(HydraliskPushTactic))
	{
		bool hasArmy = false;
		for (auto army : myArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord && army.second->getUnits().size() > 0)
			{
				hasArmy = true;
				break;
			}
		}

		return hasArmy;

		/*if (isFirstHydraAttack)
		{
			if (myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 12)
				isFirstHydraAttack = false;
			return myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 12;
		}
		else
			return  myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 0;*/
	}
	return false;
}


BWAPI::Position	AttackManager::getNextAttackPosition(bool isGround)
{
	std::set<BWTA::Region *> enemyRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
	std::set<BWTA::Region *> myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
	BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();

	for (std::vector<AttackEnemyBase>::iterator it = attackPosition.begin(); it != attackPosition.end(); it++)
	{
		//our occupied base
		if (myRegion.find(BWTA::getRegion(it->base)) != myRegion.end()
			&& enemyRegion.find(BWTA::getRegion(it->base)) == enemyRegion.end())
		{
			it->priority = -1;
		}
		else if (enemyRegion.find(BWTA::getRegion(it->base)) != enemyRegion.end())
		{
			if (BWTA::getRegion(it->base) == BWTA::getRegion(enemyBase))
			{
				it->priority = 7;
			}
			else if (BWTA::getRegion(it->base) == BWTA::getRegion(enemyNatural))
			{
				it->priority = 6;
			}
			else
			{
				it->priority = 10;
			}
		}
		else
		{
			it->priority = 1;
		}
	}
	std::sort(attackPosition.begin(), attackPosition.end());

	int i = int(attackPosition.size()) - 1;
	while (i >= 0)
	{
		if (attackPosition[i].priority == -1)
		{
			i--;
			continue;
		}
		if (isGround && BWAPI::Broodwar->getFrameCount() > attackPosition[i].groundNextAttackTime)
		{
			return attackPosition[i].base;
		}
		if (!isGround && BWAPI::Broodwar->getFrameCount() > attackPosition[i].airNextAttackTime)
		{
			return attackPosition[i].base;
		}

		i--;
	}
	return BWAPI::Positions::None;
}



int AttackManager::addTacArmy(int needArmySupply, tacticType tacType, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, bool allAirEnemy, bool addAll)
{
	if (addAll)
	{
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Mutalisk, Army[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Hydralisk, Army[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size());
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Zergling, Army[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size());

		needArmySupply -= BWAPI::UnitTypes::Zerg_Mutalisk.spaceRequired() * Army[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
		needArmySupply -= BWAPI::UnitTypes::Zerg_Hydralisk.spaceRequired() * Army[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size();
		needArmySupply -= BWAPI::UnitTypes::Zerg_Zergling.spaceRequired() * Army[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
	}
	else
	{
		for (std::map<BWAPI::UnitType, BattleArmy*>::iterator it = myArmy.begin(); it != myArmy.end(); it++)
		{
			if (it->second->getUnits().size() > 0 && it->first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				if (allAirEnemy == true && it->first == BWAPI::UnitTypes::Zerg_Zergling)
				{
					continue;
				}
				for (std::vector<UnitState>::iterator itArmy = it->second->getUnits().begin(); itArmy != it->second->getUnits().end();)
				{
					if (BWTA::getRegion(itArmy->unit->getPosition()) == BWTA::getRegion(attackPosition) && !itArmy->unit->getType().isBuilding()
						&& !itArmy->unit->getType().isWorker() && itArmy->unit->getType().canAttack())
					{
						TacticManager::Instance().addTacticUnit(DefendTactic, attackPosition, itArmy->unit);
						needArmySupply -= itArmy->unit->getType().supplyRequired();
						itArmy = it->second->getUnits().erase(itArmy);
					}
					else
						itArmy++;
				}
			}
		}

		if (needArmySupply <= 0)
			return 0;

		int mutaliskRemain = 0;
		int hydraliskRemain = 0;
		int zerglingRemain = 0;
		int mutaliskSend = 0;
		int hydraliskSend = 0;
		int zerglingSend = 0;
		//send the mutalisk to defend first, since mutalisk move fast
		mutaliskRemain = Army[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
		hydraliskRemain = Army[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size();
		zerglingRemain = Army[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
		mutaliskSend = needArmySupply / 4 < mutaliskRemain ? needArmySupply / 4 : mutaliskRemain;
		needArmySupply -= mutaliskSend * 4;
		if (needArmySupply > 0)
		{
			hydraliskSend = needArmySupply / 2 < hydraliskRemain ? needArmySupply / 2 : hydraliskRemain;
			needArmySupply -= hydraliskSend * 2;
			if (needArmySupply > 0)
			{
				if (allAirEnemy == true)
				{
					zerglingSend = 0;
				}
				else
				{
					zerglingSend = needArmySupply < zerglingRemain ? needArmySupply : zerglingRemain;
				}
			}
		}
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Mutalisk, mutaliskSend);
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Hydralisk, hydraliskSend);
		TacticManager::Instance().addTacticArmy(tacType, attackPosition, Army, BWAPI::UnitTypes::Zerg_Zergling, zerglingSend);
	}

	if (needArmySupply > 0)
		return needArmySupply;
	else
		return 0;
}


bool AttackManager::isHaveAntiAirUnit()
{
	for (auto a : myArmy)
	{
		if (a.first.airWeapon() != BWAPI::WeaponTypes::None && a.second->getUnits().size() > 0)
		{
			return true;
		}
	}
	return false;
}


void AttackManager::DefendUpdate()
{
	//each base corresponding to defend enemy
	std::map<BWAPI::Position, std::set<BWAPI::Unit>> enemyUnitsInRegion;

	std::map<BWAPI::Position, int> enemyUnitsInRegionSupply;
	std::map<BWAPI::Position, int> enemyUnitsInRegionFlySupply;

	std::map<BWTA::Region*, std::set<BWAPI::Unit>> enemyUnitsInNeighborRegion;
	std::map<BWTA::Region*, int> enemyUnitsInNeighborRegionSupply;

	std::set<BWTA::Region *> myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	std::set<BWAPI::Unit> ourAllBases = InformationManager::Instance().getOurAllBaseUnit();

	BWAPI::Position curP = BWAPI::Positions::None;
	for (auto enemyUnit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (BWTA::getRegion(enemyUnit->getPosition()) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
		{
			curP = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
			if (enemyUnit->getType().isFlyer() && !isHaveAntiAirUnit())
				continue;

			enemyUnitsInRegion[curP].insert(enemyUnit);
		}

		if (BWTA::getRegion(enemyUnit->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
			&& myRegion.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegion.end())
		{
			curP = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());

			if (enemyUnit->getType().isFlyer() && !isHaveAntiAirUnit())
				continue;

			enemyUnitsInRegion[curP].insert(enemyUnit);
		}
	}

	//has natural base
	if (myRegion.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegion.end())
	{
		BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(naturalChokePosition, 16 * 32, BWAPI::Filter::IsEnemy);
		for (auto enemyUnit : enemySet)
		{
			if (enemyUnit->getType().isFlyer() && !isHaveAntiAirUnit())
				continue;

			BWAPI::Position curP = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
			enemyUnitsInRegion[curP].insert(enemyUnit);
		}
	}
	else
	{
		BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(baseChokePosition, 12 * 32, BWAPI::Filter::IsEnemy);
		for (auto enemyUnit : enemySet)
		{
			if (enemyUnit->getType().isFlyer() && !isHaveAntiAirUnit())
				continue;

			BWAPI::Position curP = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
			enemyUnitsInRegion[curP].insert(enemyUnit);
		}
	}


	for (auto b : ourAllBases)
	{
		//base region has special enemy detection policy
		if (BWTA::getRegion(b->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
			|| BWTA::getRegion(b->getPosition()) == BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
		{
			continue;
		}

		BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsInRadius(b->getPosition(), 16 * 32, BWAPI::Filter::IsEnemy);
		for (auto enemyUnit : enemySet)
		{
			// if we do not have anti-air army, ignore
			if (enemyUnit->getType().isFlyer() && !isHaveAntiAirUnit())
				continue;

			if (!enemyUnit->isDetected() && enemyUnit->isVisible())
				continue;

			enemyUnitsInRegion[b->getPosition()].insert(enemyUnit);
		}
	}

	for (auto baseEnemy : enemyUnitsInRegion)
	{
		for (auto enemyUnit : baseEnemy.second)
		{
			if (!enemyUnit->getType().isBuilding())
			{
				if (enemyUnit->getType().isFlyer())
					enemyUnitsInRegionFlySupply[baseEnemy.first] += enemyUnit->getType().supplyRequired();
				enemyUnitsInRegionSupply[baseEnemy.first] += enemyUnit->getType().supplyRequired();
			}
			else
			{
				if (enemyUnit->getType().canAttack())
				{
					enemyUnitsInRegionSupply[baseEnemy.first] += 8;
				}
				else if (enemyUnit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					enemyUnitsInRegionSupply[baseEnemy.first] += 12;
				}
				else
					enemyUnitsInRegionSupply[baseEnemy.first] += 1;
			}
		}
	}

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	bool hasSunken = false;
	BOOST_FOREACH(BWAPI::Unit sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
	{
		if (sunker->isCompleted())
		{
			hasSunken = true;
			break;
		}
	}


	if (enemyUnitsInRegion.size() == 0)
	{
		defendAddSupply = 0;
		isNeedDefend = false;
		hasWorkerScouter = false;
		ProductionManager::Instance().defendLostRecover();
		WorkerManager::Instance().finishedWithCombatWorkers();
		return;
	}
	

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myUnits = InformationManager::Instance().getOurAllBattleUnit();
	int myTotalArmySupply = 0;

	typedef std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>::value_type metaType;
	BOOST_FOREACH(metaType& p, myUnits)
	{
		if (p.first.canAttack() && !p.first.isWorker())
			myTotalArmySupply += ((p.first).supplyRequired() * p.second.size());
	}

	//special case for scouter
	if (enemyUnitsInRegion.size() == 1 && enemyUnitsInRegion.begin()->second.size() == 1 &&
		(*enemyUnitsInRegion.begin()->second.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() == 0)
	{
		hasWorkerScouter = true;
		// the enemy worker that is attacking us
		BWAPI::Unit enemyWorker = *enemyUnitsInRegion.begin()->second.begin();

		if (myRegion.find(BWTA::getRegion(enemyWorker->getPosition())) == myRegion.end()) //(enemyWorker->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) > 32 * 10)
		{
			WorkerManager::Instance().finishedWithCombatWorkers();
		}
		else
		{
			// get our worker unit that is mining that is closest to it
			BWAPI::Unit workerDefender = WorkerManager::Instance().getClosestMineralWorkerTo(enemyWorker);

			// grab it from the worker manager
			WorkerManager::Instance().setCombatWorker(workerDefender);
		}
	}
	//use zergling to kill scout worker
	else if (enemyUnitsInRegion.size() == 1 && enemyUnitsInRegion.begin()->second.size() == 1 &&
		(*enemyUnitsInRegion.begin()->second.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 0)
	{
		hasWorkerScouter = true;
		WorkerManager::Instance().finishedWithCombatWorkers();
		ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(myArmy[BWAPI::UnitTypes::Zerg_Zergling]);
		zerglings->attackScoutWorker(*enemyUnitsInRegion.begin()->second.begin());
	}
	else
	{
		isNeedDefend = true;
		//trig defend tactic
		std::set<AttackEnemyBase> enemyAttackRegionsPriority;
		BWAPI::Position myBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
		for (std::map<BWAPI::Position, std::set<BWAPI::Unit>>::iterator it = enemyUnitsInRegion.begin(); it != enemyUnitsInRegion.end(); it++)
		{
			if (it->second.size() == 0)
				continue;

			enemyAttackRegionsPriority.insert(AttackEnemyBase(it->first, int(myBase.getDistance(it->first))));
		}

		typedef std::map<BWAPI::UnitType, BattleArmy*>::value_type mapType;
		// defend priority is defined by attack region's distance to start base, we traverse to assign defend army from high priority to low 
		for (std::set<AttackEnemyBase>::iterator it = enemyAttackRegionsPriority.begin(); it != enemyAttackRegionsPriority.end(); it++)
		{
			int myRemainSupply = 0;
			BOOST_FOREACH(mapType& p, myArmy)
			{
				myRemainSupply += ((p.first).supplyRequired() * p.second->getUnits().size());
			}

			int enemySupply = enemyUnitsInRegionSupply[it->base];
			
			std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();

			int sunkenSupply = 0;
			BOOST_FOREACH(BWAPI::Unit sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
			{
				if (BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion((it->base)) && sunker->isCompleted())
				{
					sunkenSupply += 3;
				}
			}

			int sporeSupply = 0;
			BOOST_FOREACH(BWAPI::Unit sunker, myBuildings[BWAPI::UnitTypes::Zerg_Spore_Colony])
			{
				if (BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion((it->base)) && sunker->isCompleted())
				{
					sporeSupply += 6;
				}
			}

			bool allAirEnemy = false;
			if (enemyUnitsInRegionFlySupply[it->base] > 0)
			{
				int needAirDefend = enemyUnitsInRegionFlySupply[it->base] - sporeSupply <= 0 ? 0 : enemyUnitsInRegionFlySupply[it->base] - sporeSupply;
				int needGroundDefend = enemySupply - enemyUnitsInRegionFlySupply[it->base] - sunkenSupply <= 0 ? 0 : enemySupply - enemyUnitsInRegionFlySupply[it->base] - sunkenSupply;
				enemySupply = needAirDefend + needGroundDefend;
				
				if (enemySupply > 0 && (needAirDefend * 10 / enemySupply >= 8))
				{
					allAirEnemy = true;
				}
			}
			else
			{
				enemySupply -= sunkenSupply;
			}


			if (enemySupply <= 0)
			{
				continue;
			}

			//early game defend need army, no need to split army
			if (BWAPI::Broodwar->getFrameCount() < 10000)
				enemySupply = int(enemySupply * armyForceMultiply);
			else
				enemySupply = int(enemySupply * armyForceMultiply);

			int totalSupply = InformationManager::Instance().getOurTotalBattleForce();
			//do not have any unit to defend
			if (totalSupply == 0)
				continue;

			//if we do not have enough force, do not trigger defend outside our base
			if (BWTA::getRegion(it->base) != BWTA::getRegion(InformationManager::Instance().getOurBaseLocation())
				&& BWTA::getRegion(it->base) != BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
				&& totalSupply < enemySupply)
			{
				continue;
			}


			// if current army force is less than enemy, add more army, currently can only add mutalisk and hydralisk
			if (TacticManager::Instance().isTacticRun(DefendTactic, it->base))
			{
				int defendTacArmySupply = TacticManager::Instance().getTacArmyForce(DefendTactic, it->base);
				if (defendTacArmySupply < enemySupply)
				{
					int needDefendSupply = enemySupply - defendTacArmySupply;
					int remainSupply = addTacArmy(needDefendSupply, DefendTactic, it->base, myArmy, allAirEnemy);
					// check if attack tactic need retreat some army to defend
					if (remainSupply > 0)
					{
						TacticManager::Instance().assignDefendArmy(it->base, remainSupply, allAirEnemy);
					}
				}
			}
			else
			{
				TacticManager::Instance().addTactic(DefendTactic, it->base);
				int remainSupply = addTacArmy(enemySupply, DefendTactic, it->base, myArmy, allAirEnemy);
				// check if attack tactic can retreat some army to defend
				if (remainSupply > 0)
				{
					TacticManager::Instance().assignDefendArmy(it->base, remainSupply, allAirEnemy);
				}
			}
		}
	}

	

}

bool AttackManager::isArmyHealthy(BWAPI::UnitType unitType)
{
	int lowHealthCount = 0;
	BOOST_FOREACH(UnitState u, myArmy[unitType]->getUnits())
	{
		if (u.unit->getHitPoints() < u.unit->getType().maxHitPoints() / 3)
		{
			lowHealthCount++;
		}
	}

	if (lowHealthCount > myArmy[unitType]->getUnits().size() * 0.25)
		return false;
	else
		return true;
}

void AttackManager::onUnitMorph(BWAPI::Unit unit)
{
	if (unit == NULL)
	{
		return;
	}

	if (myArmy.find(unit->getType()) == myArmy.end())
	{
		BWAPI::UnitType u = unit->getType();
		return;
	}

	unRallyArmy.push_back(unit);
}

void AttackManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL || myArmy.find(unit->getType()) == myArmy.end())
		return;

	std::vector<UnitState>& army = myArmy[unit->getType()]->getUnits();

	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit == unit)
		{
			army.erase(it);
			break;
		}
	}

	for (std::vector<BWAPI::Unit>::iterator it = unRallyArmy.begin(); it != unRallyArmy.end(); it++)
	{
		if (*it == unit)
		{
			unRallyArmy.erase(it);
			break;
		}
	}
}

void AttackManager::onLurkerMorph()
{
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits();

	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit->isMorphing())
		{
			army.erase(it);
			break;
		}
	}
}

void AttackManager::onEnemyUnitDestroy(BWAPI::Unit unit)
{
	if (enemyInvadeSet.find(unit) != enemyInvadeSet.end())
		enemyInvadeSet.erase(unit);
}


AttackManager& AttackManager::Instance()
{
	static AttackManager a;
	return a;
}


void AttackManager::addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy, tacticType tacType, BWAPI::Position attackTarget, bool endByDefend)
{
	typedef std::map<BWAPI::UnitType, BattleArmy*>::value_type mapType;
	BOOST_FOREACH(mapType& p, tacticArmy)
	{
		std::vector<UnitState>& t = myArmy[p.first]->getUnits();
		t.insert(t.end(), p.second->getUnits().begin(), p.second->getUnits().end());
	}

	//tactic is canceled by defend
	if (tacType == DefendTactic || endByDefend)
		return;

	//for kill scout
	//if (BWAPI::Broodwar->getFrameCount() <= 5000)
		//return;

	std::set<BWTA::Region *> & enemyRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());

	BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
	BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();
	
	if (tacType == MutaliskHarassTac)
	{
		BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
		{
			if (attackTarget == b.base)//(BWTA::getRegion(attackTarget) == BWTA::getRegion(b.base))
			{
				// still our target
				if (enemyRegions.find(BWTA::getRegion(attackTarget)) != enemyRegions.end())
				{
					b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 90 * 25;
				}
				else
				{
					b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 60 * 25;
					b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 60 * 25;
				}
				break;
			}
		}
	}
	else
	{
		if ((BWTA::getRegion(attackTarget) == BWTA::getRegion(enemyBase) || BWTA::getRegion(attackTarget) == BWTA::getRegion(enemyNatural)) && enemyRegions.find(BWTA::getRegion(attackTarget)) != enemyRegions.end())
		{
			BWAPI::Broodwar->printf("do not attack enemy base && natural");
			BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
			{
				if (BWTA::getRegion(b.base) == BWTA::getRegion(enemyBase) || BWTA::getRegion(b.base) == BWTA::getRegion(enemyNatural))
				{
					b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 120 * 25;
				}
			}
		}
		else
		{
			BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
			{
				if (attackTarget == b.base)//(BWTA::getRegion(attackTarget) == BWTA::getRegion(b.base))
				{
					// still our target
					if (enemyRegions.find(BWTA::getRegion(attackTarget)) != enemyRegions.end())
					{
						b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 120 * 25;
					}
					else
					{
						b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 120 * 25;
						b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 120 * 25;
					}
					break;
				}
			}
		}
		

		/*
		BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
		BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();

		std::set<BWTA::Region*> attackVector;
		attackVector.insert(BWTA::getRegion(attackTarget));
		if (tacType != MutaPush && (BWTA::getRegion(attackTarget) == BWTA::getRegion(enemyBase) || BWTA::getRegion(attackTarget) == BWTA::getRegion(enemyNatural)))
		{
			attackVector.insert(BWTA::getRegion(enemyBase));
			attackVector.insert(BWTA::getRegion(enemyNatural));
		}

		for (std::set<BWTA::Region*>::iterator it = attackVector.begin(); it != attackVector.end(); it++)
		{
			BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
			{
				if (*it == BWTA::getRegion(b.base))
				{
					// still our target
					if (enemyRegions.find(*it) != enemyRegions.end())
					{
						b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 40 * 25;
					}
					else
					{
						b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 90 * 25;
						b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 90 * 25;
					}
				}
			}
		}*/
		
	}
}


