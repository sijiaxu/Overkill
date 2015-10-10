#include "AttackManager.h"


AttackManager::AttackManager()
{
	// do init first
	myArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();

	enemyInvadeSet = std::set<BWAPI::Unit *>();

	isNeedDefend = false;
	isNeedTacticDefend = false;
	zerglingHarassFlag = true;
	unRallyArmy.reserve(100);

	defendState = noEnemy;
	recoverLostTime = 0;
	updatPositionTime = 0;

	//get the natural choke 
	BWAPI::Position natrualLocation = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	BWAPI::Position baseChoke = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();

	double2 direc = baseChoke - natrualLocation;
	double2 direcNormal = direc / direc.len();

	int targetx = natrualLocation.x() + int(direcNormal.x * 32 * 3);
	int targety = natrualLocation.y() + int(direcNormal.y * 32 * 3);

	rallyPosition = BWAPI::Position(targetx, targety);

	triggerZerglingBuilding = true;

	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		attackPosition.push_back(AttackEnemyBase(base->getPosition(), 0));
	}

	lastAttackPosition = BWAPI::Positions::None;
	defendAddSupply = 0;
	hasWorkerScouter = false;
	isFirstMutaAttack = true;
	isFirstHydraAttack = true;
}


void AttackManager::groupArmy()
{
	std::set<BWTA::Region *>& myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (myRegions.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
		rallyPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		rallyPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	//battle unit rally to the natural choke center
	for (std::vector<BWAPI::Unit*>::iterator it = unRallyArmy.begin(); it != unRallyArmy.end();)
	{
		if ((*it)->canIssueCommand(BWAPI::UnitCommand(*it, BWAPI::UnitCommandTypes::Move, NULL, rallyPosition.x(), rallyPosition.y(), 0)))
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
}


// attack manger trigger for two situation:defend and attack
void AttackManager::update()
{
	/*
	if (InformationManager::Instance().GetEnemyBasePosition() != BWAPI::TilePositions::None)
	{
		std::list<BWAPI::TilePosition> tmp = aStarPathFinding(InformationManager::Instance().getOurNatrualLocation(), BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()), false, true);
		
		//std::vector<BWAPI::TilePosition> tmp = BWTA::getShortestPath(InformationManager::Instance().getOurNatrualLocation(), BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()));
		for (std::list<BWAPI::TilePosition>::iterator it = tmp.begin(); it != tmp.end(); it++)
		{
			std::list<BWAPI::TilePosition>::iterator it2 = it;
			it2++;
			if (it2 != tmp.end())
			{
				BWAPI::Broodwar->drawLineMap(it->x() * 32, it->y() * 32, it2->x() * 32, it2->y() * 32, BWAPI::Colors::Purple);
			}	
		}
	}*/


	if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
		groupArmy();

	DefendUpdate();

	BWAPI::Position attackPosition;
	//if position has been eliminate or enemy is too strong at the position, change attack position
	for (int i = 0; i < int(tactictypeEnd); i++)
	{
		if (tacticType(i) == MutaPush)
			attackPosition = getNextAttackPosition(false);
		else
			attackPosition = getNextAttackPosition(true);
		if (attackPosition == BWAPI::Positions::None)
			return;
		triggerTactic(tacticType(i), attackPosition);
	}

}


void AttackManager::triggerTactic(tacticType tacType, BWAPI::Position attackPosition)
{
	bool isTacticRunning;

	switch (tacType)
	{
	case ZerglingHarassTac:
	{
		if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
			return;

		isTacticRunning = TacticManager::Instance().isOneTacticRun(ZerglingHarassTac);
		if (isTacticRunning)
		{
			if (hasWorkerScouter)
			{
				std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				int addCount = army.size() - 1 >= 0 ? army.size() - 1 : 0;
				if (addCount >= 0)
				{
					BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(ZerglingHarassTac);
					for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
					{
						if (addCount <= 0)
							break;
						addCount--;
						//can generate safe attack path
						TacticManager::Instance().addTacticUnit(ZerglingHarassTac, nowAttack, it->unit);
						it = army.erase(it);
					}
				}
				
				return;
			}
				
			std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
			BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(ZerglingHarassTac);
			for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
			{
				//can generate safe attack path
				TacticManager::Instance().addTacticUnit(ZerglingHarassTac, nowAttack, it->unit);
				it = army.erase(it);
			}
		}
		else
		{
			if (tacticTrigCondition(ZerglingHarassTac, attackPosition))
			{
				//popAttackPosition(attackPosition);
				TacticManager::Instance().addTactic(ZerglingHarassTac, attackPosition);
				//int triggerCount = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 36 ? 36 : myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
				TacticManager::Instance().addTacticArmy(ZerglingHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Zergling, 1);//myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size());
			}
		}
	}
		break;
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
			std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits();
			BWAPI::Position nowAttack = TacticManager::Instance().getTacticPosition(HydraliskPushTactic);
			for (std::vector<UnitState>::iterator it = army.begin(); it != army.end();)
			{
				//can generate safe attack path 
				TacticManager::Instance().addTacticUnit(HydraliskPushTactic, nowAttack, it->unit);
				it = army.erase(it);
			}
		}
		else
		{
			if (tacticTrigCondition(HydraliskPushTactic, attackPosition))
			{
				TacticManager::Instance().addTactic(HydraliskPushTactic, attackPosition);
				TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Hydralisk, myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size());
				std::vector<BWAPI::Unit*> overlordArmy = ScoutManager::Instance().getOverLordArmy(2);
				BOOST_FOREACH(BWAPI::Unit* u, overlordArmy)
				{
					myArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u);
				}
				TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Overlord, myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits().size());
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
		if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran && isFirstMutaAttack)
		{
			if (myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 6)
				isFirstMutaAttack = false;
			return myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 6;
		}
		else
			return myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() > 0;
	}
	else if (tac == int(ZerglingHarassTac))
	{
		return myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 0;
	}
	else if (tac = int(HydraliskPushTactic))
	{
		if (isFirstHydraAttack)
		{
			if (myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 24)
				isFirstHydraAttack = false;
			return myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 24;
		}
		else
			return  myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 0;
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
		if (myRegion.find(BWTA::getRegion(it->base)) != myRegion.end()
			&& enemyRegion.find(BWTA::getRegion(it->base)) == enemyRegion.end())
		{
			it->priority = -1;
		}
		else if (enemyRegion.find(BWTA::getRegion(it->base)) != enemyRegion.end())
		{
			if (BWTA::getRegion(it->base) == BWTA::getRegion(enemyBase))
			{
				it->priority = 10;
			}
			else if (BWTA::getRegion(it->base) == BWTA::getRegion(enemyNatural))
			{
				it->priority = 9;
			}
			else
			{
				it->priority = 7;
			}
		}
	}
	std::sort(attackPosition.begin(), attackPosition.end());

	int i = int(attackPosition.size()) - 1;
	while (i >= 0)
	{
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
	if (isGround)
	{
		return attackPosition.back().base;
	}
	return BWAPI::Positions::None;
}



int AttackManager::addTacArmy(int needArmySupply, tacticType tacType, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, bool addAll)
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
				zerglingSend = needArmySupply < zerglingRemain ? needArmySupply : zerglingRemain;
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


void AttackManager::DefendUpdate()
{
	std::map<BWTA::Region*, std::set<BWAPI::Unit*>> enemyUnitsInRegion;
	std::map<BWTA::Region*, int> enemyUnitsInRegionSupply;
	std::map<BWTA::Region*, int> enemyUnitsInRegionFlySupply;

	std::map<BWTA::Region*, std::set<BWAPI::Unit*>> enemyUnitsInNeighborRegion;
	std::map<BWTA::Region*, int> enemyUnitsInNeighborRegionSupply;

	std::set<BWTA::Region *> myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	//calculate nearby region
	std::set<BWTA::Region *> myReachableRegions;
	std::map<BWTA::Region *, BWTA::Region *> reachRegionsBelongsTo;
	BOOST_FOREACH(BWTA::Region* r, myRegion)
	{
		if (r != BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
			&& r != BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()))
			continue;

		const std::set<BWTA::Chokepoint*>& chokes = r->getChokepoints();
		std::set<BWTA::Region*> reachRegions;
		BOOST_FOREACH(const BWTA::Chokepoint* c, chokes)
		{
			reachRegions.insert(c->getRegions().first);
			reachRegions.insert(c->getRegions().second);
		}

		for (std::set<BWTA::Region*>::iterator it = reachRegions.begin(); it != reachRegions.end();)
		{
			if (myRegion.find(*it) != myRegion.end())
			{
				reachRegions.erase(it++);
			}
			else
				it++;
		}
		
		BOOST_FOREACH(BWTA::Region* reach, reachRegions)
		{
			reachRegionsBelongsTo[reach] = r;
		}

		myReachableRegions.insert(reachRegions.begin(), reachRegions.end());
	}

	BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if we do not have anti-air army, ignore
		if (enemyUnit->getType().isFlyer() && myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
			&& myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
			continue;

		//TODO: invisible unit can be get from api
		if (enemyUnit->getType() == BWAPI::UnitTypes::Protoss_Observer)
			continue;
		
		if (myRegion.find(BWTA::getRegion(enemyUnit->getPosition())) != myRegion.end())
		{
			enemyUnitsInRegion[BWTA::getRegion(enemyUnit->getPosition())].insert(enemyUnit);

			if (!enemyUnit->getType().isBuilding())
			{
				if (enemyUnit->getType().isFlyer())
					enemyUnitsInRegionFlySupply[BWTA::getRegion(enemyUnit->getPosition())] += enemyUnit->getType().supplyRequired();
				enemyUnitsInRegionSupply[BWTA::getRegion(enemyUnit->getPosition())] += enemyUnit->getType().supplyRequired();
			}
			else
			{
				if (enemyUnit->getType().canAttack())
				{
					enemyUnitsInRegionSupply[BWTA::getRegion(enemyUnit->getPosition())] += 8;
				}
				else if (enemyUnit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					enemyUnitsInRegionSupply[BWTA::getRegion(enemyUnit->getPosition())] += 12;
				}
				else
					enemyUnitsInRegionSupply[BWTA::getRegion(enemyUnit->getPosition())] += 1;
			}
		}

		// add enemy to their belonged region
		else if (myReachableRegions.find(BWTA::getRegion(enemyUnit->getPosition())) != myReachableRegions.end())
		{
			enemyUnitsInRegion[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]].insert(enemyUnit);
			if (!enemyUnit->getType().isBuilding())
			{
				if (enemyUnit->getType().isFlyer())
					enemyUnitsInRegionFlySupply[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]] += enemyUnit->getType().supplyRequired();
				enemyUnitsInRegionSupply[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]] += enemyUnit->getType().supplyRequired();
			}
			else
			{
				if (enemyUnit->getType().canAttack())
				{
					enemyUnitsInRegionSupply[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]] += 8;
				}
				else if (enemyUnit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					enemyUnitsInRegionSupply[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]] += 12;
				}
				else
					enemyUnitsInRegionSupply[reachRegionsBelongsTo[BWTA::getRegion(enemyUnit->getPosition())]] += 4;
			}
		}
		else
			continue;
	}

	/*
	//only trigger defend when enemy envade our regions
	for (std::map<BWTA::Region*, int>::iterator it = enemyUnitsInRegionSupply.begin(); it != enemyUnitsInRegionSupply.end(); it++)
	{
		if (it->second != 0)
		{
			it->second += enemyUnitsInNeighborRegionSupply[it->first];
		}
	}*/

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	bool hasSunken = false;
	BOOST_FOREACH(BWAPI::Unit* sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
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
	isNeedDefend = true;

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& myUnits = InformationManager::Instance().getOurAllBattleUnit();
	int myTotalArmySupply = 0;
	typedef std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>::value_type metaType;
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
		BWAPI::Unit * enemyWorker = *enemyUnitsInRegion.begin()->second.begin();

		if (myRegion.find(BWTA::getRegion(enemyWorker->getPosition())) == myRegion.end()) //(enemyWorker->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) > 32 * 10)
		{
			WorkerManager::Instance().finishedWithCombatWorkers();
		}
		else
		{
			// get our worker unit that is mining that is closest to it
			BWAPI::Unit * workerDefender = WorkerManager::Instance().getClosestMineralWorkerTo(enemyWorker);

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
	/*
	else if (myTotalArmySupply == 0 && !hasSunken)
	{
		WorkerManager::Instance().setCombatWorkerArmy(BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone));
	}*/
	else
	{
		//trig defend tactic
		std::set<AttackEnemyBase> enemyAttackRegionsPriority;
		BWAPI::Position myBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
		for (std::map<BWTA::Region*, std::set<BWAPI::Unit*>>::iterator it = enemyUnitsInRegion.begin(); it != enemyUnitsInRegion.end(); it++)
		{
			if (it->second.size() == 0)
				continue;

			std::set<BWTA::BaseLocation*> enemyBases = it->first->getBaseLocations();
			if (enemyBases.size() > 0)
				enemyAttackRegionsPriority.insert(AttackEnemyBase((*enemyBases.begin())->getPosition(), int(myBase.getDistance((*enemyBases.begin())->getPosition()))));
			else
				enemyAttackRegionsPriority.insert(AttackEnemyBase(it->first->getCenter(), int(myBase.getDistance(it->first->getCenter()))));
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

			int enemySupply = enemyUnitsInRegionSupply[BWTA::getRegion(it->base)];
			
			std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();

			int sunkenSupply = 0;
			BOOST_FOREACH(BWAPI::Unit* sunker, myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
			{
				if (BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion((it->base)) && sunker->isCompleted())
				{
					sunkenSupply += 8;
				}
			}

			int sporeSupply = 0;
			BOOST_FOREACH(BWAPI::Unit* sunker, myBuildings[BWAPI::UnitTypes::Zerg_Spore_Colony])
			{
				if (BWTA::getRegion(sunker->getPosition()) == BWTA::getRegion((it->base)) && sunker->isCompleted())
				{
					sporeSupply += 8;
				}
			}
			
			if (enemyUnitsInRegionFlySupply[BWTA::getRegion(it->base)] > 0)
			{
				int needAirDefend = enemyUnitsInRegionFlySupply[BWTA::getRegion(it->base)] - sporeSupply <= 0 ? 0 : enemyUnitsInRegionFlySupply[BWTA::getRegion(it->base)];
				int needGroundDefend = enemySupply - enemyUnitsInRegionFlySupply[BWTA::getRegion(it->base)] - sunkenSupply <= 0 ? 0 : enemySupply - enemyUnitsInRegionFlySupply[BWTA::getRegion(it->base)] - sunkenSupply;
				enemySupply = needAirDefend + needGroundDefend;
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
				enemySupply = int(enemySupply * 2);
			else
				enemySupply = int(enemySupply * 1.2);


			//do not have any unit to defend
			if (myRemainSupply == 0 && !TacticManager::Instance().isHaveNoneDefendTactic())
				continue;

			// if current army force is less than enemy, add more army, currently can only add mutalisk and hydralisk
			if (TacticManager::Instance().isTacticRun(DefendTactic, it->base))
			{
				int defendTacArmySupply = TacticManager::Instance().getTacArmyForce(DefendTactic, it->base);
				if (defendTacArmySupply < enemySupply)
				{
					int needDefendSupply = enemySupply - defendTacArmySupply;
					int remainSupply = addTacArmy(needDefendSupply, DefendTactic, it->base, myArmy);
					// check if attack tactic need retreat some army to defend
					if (remainSupply > 0)
					{
						TacticManager::Instance().assignDefendArmy(it->base, remainSupply);
					}
				}
			}
			else
			{
				TacticManager::Instance().addTactic(DefendTactic, it->base);
				int remainSupply = addTacArmy(enemySupply, DefendTactic, it->base, myArmy);
				// check if attack tactic can retreat some army to defend
				if (remainSupply > 0)
				{
					TacticManager::Instance().assignDefendArmy(it->base, remainSupply);
				}
			}
		}
	}

	
	




	/*
	//enemy detect 
	std::set<BWAPI::Unit *> enemyUnitsInRegion;
	std::set<BWTA::Region *>& myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	unsigned enemyTotalSupply = 0;
	BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
	{
		//TODO: invisible unit can be get from api
		if (myRegion.find(BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition()))) != myRegion.end() 
			&& enemyUnit->isVisible())
		{
			// if we do not have anti-air army, ignore
			if (enemyUnit->getType().isFlyer() && myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;
			enemyUnitsInRegion.insert(enemyUnit);
			enemyInvadeSet.insert(enemyUnit);
			enemyTotalSupply += enemyUnit->getType().supplyRequired();
		}
	}

	//enemy defend FSM
	switch (defendState)
	{
	case AttackManager::noEnemy:
	{
		if (enemyUnitsInRegion.size() > 0)
			defendState = enemyInvade;
		else
			//reset enemyInregion
			enemyInvadeSet.clear();
	}
		break;
	case AttackManager::enemyInvade:
	{
		ProductionManager::Instance().clearDefendVector();
		defendState = defendBattle;
	}
		break;
	case AttackManager::defendBattle:
	{
		if (enemyUnitsInRegion.size() == 0)
			defendState = enemyKilled;
		else
			DefendEnemy(enemyUnitsInRegion, enemyTotalSupply);
	}
		break;
	case AttackManager::enemyKilled:
	{
		//group army back to natrual
		typedef std::map<BWAPI::UnitType, BattleArmy*>::value_type mapType;
		BOOST_FOREACH(mapType& p, myArmy)
		{
			if (p.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				p.second->armyMove(BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
			}
		}

		//wait up to ten seconds to see if enemy retreat
		recoverLostTime = BWAPI::Broodwar->getFrameCount() + 30 * 10;
		defendState = end;
	}
		break;
	case AttackManager::end:
	{

		if (enemyInvadeSet.size() == 0 || BWAPI::Broodwar->getFrameCount() > recoverLostTime)
		{
			
			DefendOver();
			defendState = noEnemy;
		}
		else if (enemyUnitsInRegion.size() > 0)
		{
			defendState = defendBattle;
		}
	}
		break;
	default:
		break;
	}*/
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

void AttackManager::onUnitMorph(BWAPI::Unit* unit)
{
	if (unit == NULL)
	{
		return;
	}
	unRallyArmy.push_back(unit);

	/*
	//if mutalisk harass is executing, mutalisk add to the tactic directly
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk && TacticManager::Instance().isTacticRun(MutaliskHarassTac))
	{
		TacticManager::Instance().addTacticUnit(MutaliskHarassTac, unit);
	}
	else if ((unit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk  \
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk) \
		&& TacticManager::Instance().isTacticRun(HydraliskPushTactic))
	{
		TacticManager::Instance().addTacticUnit(HydraliskPushTactic, unit);
	}
	else
	{
		unRallyArmy.push_back(unit);
	}*/

	/*
	else if (unit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk && TacticManager::Instance().isTacticRun(HydraliskPushTactic))
	{
		TacticManager::Instance().addTacticUnit(HydraliskPushTactic, unit);
	}
	else
	{
		myArmy[unit->getType()]->addUnit(unit);
		unRallyArmy.push_back(unit);
	}*/
}

void AttackManager::onUnitDestroy(BWAPI::Unit * unit)
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
}

void AttackManager::onEnemyUnitDestroy(BWAPI::Unit* unit)
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
		if (p.first == BWAPI::UnitTypes::Zerg_Overlord)
		{
			ScoutManager::Instance().giveBackOverLordArmy(p.second);
		}
		else
		{
			std::vector<UnitState>& t = myArmy[p.first]->getUnits();
			t.insert(t.end(), p.second->getUnits().begin(), p.second->getUnits().end());
		}
	}

	//tactic is canceled by defend
	if (tacType == DefendTactic || endByDefend)
		return;

	//for kill scout
	if (BWAPI::Broodwar->getFrameCount() <= 5000)
		return;

	std::set<BWTA::Region *> & enemyRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
	
	if (tacType == MutaPush)
	{
		BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
		{
			if (BWTA::getRegion(attackTarget) == BWTA::getRegion(b.base))
			{
				// still our target
				if (enemyRegions.find(BWTA::getRegion(attackTarget)) != enemyRegions.end())
				{
					b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 20 * 25;
				}
				else
				{
					b.airNextAttackTime = BWAPI::Broodwar->getFrameCount() + 180 * 25;
					b.groundNextAttackTime = BWAPI::Broodwar->getFrameCount() + 180 * 25;
				}
				break;
			}
		}
	}
	else
	{
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
		}
		
	}
}


