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
}


void AttackManager::groupArmy()
{

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
		std::list<BWAPI::TilePosition> tmp = aStarPathFinding(InformationManager::Instance().getOurNatrualLocation(), BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()), true);

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

	//if enemy invade, do not trig tactic
	if (isNeedDefend)
		return;


	zergGameStage stage = zergGameStage(StrategyManager::Instance().getCurrentStage());

	//FSM for different stage
	switch (stage)
	{
	case Start:
		
		if (tacticTrigCondition(ZerglingHarassTac))
		{
			if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
				return;
			//only trig once
			zerglingHarassFlag = false;
			TacticManager::Instance().addTactic(ZerglingHarassTac, InformationManager::Instance().GetEnemyBasePosition());
			TacticManager::Instance().initTacticArmy(ZerglingHarassTac, myArmy, BWAPI::UnitTypes::Zerg_Zergling, 6);
		}
		break;
	case Mid:
		
		if (tacticTrigCondition(HydraliskPushTactic))
		{
			BWAPI::Position attackPosition = generateAttackPosition();
			if (attackPosition == BWAPI::Positions::None)
				return;
			TacticManager::Instance().addTactic(HydraliskPushTactic, attackPosition);
			TacticManager::Instance().initTacticArmy(HydraliskPushTactic, myArmy, BWAPI::UnitTypes::Zerg_Hydralisk, myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size());
			TacticManager::Instance().initTacticArmy(HydraliskPushTactic, myArmy, BWAPI::UnitTypes::Zerg_Mutalisk, 12);//myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
			std::vector<BWAPI::Unit*> overlordArmy = ScoutManager::Instance().getOverLordArmy(3);
			BOOST_FOREACH(BWAPI::Unit* u, overlordArmy)
			{
				myArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u);
			}
			TacticManager::Instance().initTacticArmy(HydraliskPushTactic, myArmy, BWAPI::UnitTypes::Zerg_Overlord, myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits().size());
		}
		 
		if (tacticTrigCondition(MutaliskHarassTac))
		{
			BWAPI::Position attackPosition = generateAttackPosition();
			if (attackPosition == BWAPI::Positions::None)
				return;
			TacticManager::Instance().addTactic(MutaliskHarassTac, attackPosition);
			TacticManager::Instance().initTacticArmy(MutaliskHarassTac, myArmy, BWAPI::UnitTypes::Zerg_Mutalisk, myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
		}
		break;
	case End:
		break;
	default:
		break;
	}
}


bool AttackManager::tacticTrigCondition(int tac)
{
	if (TacticManager::Instance().isTacticRun(tacticType(tac)))
		return false;

	if (tac == int(MutaliskHarassTac))
	{
		return !StrategyManager::Instance().mutaliskHarassFlag() && myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 12; //&& isArmyHealthy(BWAPI::UnitTypes::Zerg_Mutalisk);
	}
	else if (tac == int(ZerglingHarassTac))
	{
		return myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() >= 6 && zerglingHarassFlag;
	}
	else if (tac = int(HydraliskPushTactic))
	{
		return  myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 36 && myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() >= 12 \
			&& ScoutManager::Instance().getIdleOverlordNum() >= 3;
	}

	return false;
}


BWAPI::Position	AttackManager::generateAttackPosition()
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
				it->priority = 10;
			else if (BWTA::getRegion(it->base) == BWTA::getRegion(enemyNatural))
				it->priority = 9;
			else
				it->priority = 7;
		}
	}
	std::sort(attackPosition.begin(), attackPosition.end());

	int i = int(attackPosition.size()) - 1;
	while (i >= 0)
	{
		if (BWAPI::Broodwar->getFrameCount() > attackPosition[i].nextAttackTime
			&& attackPosition[i].nowAttacking == false)
		{
			//base has attack interval
			attackPosition[i].nowAttacking = true;
			return attackPosition[i].base;
		}
		i--;
	}
	return BWAPI::Positions::None;
	

	/*
	// only trig when all the possible base have been attacked
	if (attackPosition.size() == 0)
		updateAttackPosition();

	if (attackPosition.size() == 0)
		return BWAPI::Positions::None;

	BWAPI::Position p = attackPosition.back().base;
	attackPosition.pop_back();
	return p;*/
}


void AttackManager::updateAttackPosition()
{
	if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
		return;

	unsigned myTotalArmySupply = 0;
	typedef std::map<BWAPI::UnitType, BattleArmy*>::value_type armyType;
	BOOST_FOREACH(armyType u, myArmy)
	{
		if (u.first.canAttack() && !u.first.isWorker())
		{
			myTotalArmySupply += u.first.supplyRequired() * u.second->getUnits().size();
		}
	}

	attackPosition.clear();
	std::set<BWTA::Region *> enemyRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());

	std::set<BWTA::Region *> eraseRegion;
	
	//find the place we can attack 
	std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>& enemyRegionDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	std::set<BWTA::Region *> & ourRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	typedef std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>::value_type regionDetail;
	BOOST_FOREACH(regionDetail r, enemyRegionDetail)
	{
		int defendCount = 0;
		for (std::map<BWAPI::Unit*, buildingInfo>::iterator it = r.second.begin(); it != r.second.end(); it++)
		{
			if (it->second.unitType == BWAPI::UnitTypes::Protoss_Photon_Cannon
				|| it->second.unitType == BWAPI::UnitTypes::Zerg_Spore_Colony
				|| it->second.unitType == BWAPI::UnitTypes::Terran_Missile_Turret
				|| it->second.unitType == BWAPI::UnitTypes::Terran_Bunker)
			{
				defendCount++;
			}
		}
		if (int(myTotalArmySupply) < defendCount * 6 * 2)
		{
			enemyRegion.erase(r.first);
			eraseRegion.insert(r.first);
		}
	}

	//enemy's main base have been destroyed, looking for other possible base
	if (enemyRegion.size() == 0)
	{
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			if (eraseRegion.find(base->getRegion()) == eraseRegion.end() && ourRegions.find(base->getRegion()) == ourRegions.end())
			{
				attackPosition.push_back(AttackEnemyBase(base->getPosition(), 0));
			}
		}
	}
	//attack the knowned enemy occupied base
	else
	{
		//if the only enemy region is start base, attack the natural as well
		BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
		BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();

		if (enemyRegion.size() == 1 && BWTA::getRegion(enemyBase) == *enemyRegion.begin())
		{
			if (eraseRegion.find(BWTA::getRegion(enemyNatural)) == eraseRegion.end())
				attackPosition.push_back(AttackEnemyBase(enemyNatural, 9));
			attackPosition.push_back(AttackEnemyBase(enemyBase, 10));
		}
		else
		{
			BOOST_FOREACH(BWTA::Region* region, enemyRegion)
			{
				if (region == BWTA::getRegion(enemyBase))
					attackPosition.push_back(AttackEnemyBase(enemyBase, 10));
				else if (region == BWTA::getRegion(enemyNatural))
					attackPosition.push_back(AttackEnemyBase(enemyNatural, 9));
				else
				{
					std::set<BWTA::BaseLocation*> enemyBases = region->getBaseLocations();
					if (enemyBases.size() > 0)
						attackPosition.push_back(AttackEnemyBase((*enemyBases.begin())->getPosition(), 7));
					else
						attackPosition.push_back(AttackEnemyBase(region->getCenter(), 6));
				}
			}
		}

		std::sort(attackPosition.begin(), attackPosition.end());
	}
}


void AttackManager::DefendUpdate()
{
	//enemy detect 
	std::set<BWAPI::Unit *> enemyUnitsInRegion;
	std::set<BWTA::Region *>& myRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	unsigned enemyTotalSupply = 0;
	BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
	{
		//TODO: invisible unit can be get from api
		if (myRegion.find(BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition()))) != myRegion.end())
		{
			if (enemyUnit->getType() == BWAPI::UnitTypes::Protoss_Observer)
				continue;
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
		/*
		if (enemyUnitsInRegion.size() > 1 || !((*enemyUnitsInRegion.begin())->getType().isWorker()))
		{
			DefendProductionStrategy(enemyTotalSupply);
		}*/
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
			ProductionManager::Instance().defendLostRecover();
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

void AttackManager::DefendProductionStrategy(unsigned enemyTotalSupply)
{
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& selfAllBattleUnit = InformationManager::Instance().getOurAllBattleUnit();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();

	unsigned myTotalArmySupply = 0;
	typedef std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>::value_type metaType;
	BOOST_FOREACH(metaType& u, selfAllBattleUnit)
	{
		if (u.first.canAttack() && !u.first.isWorker())
		{
			myTotalArmySupply += u.first.supplyRequired() * u.second.size();
		}
	}

	//check if need to produce Battle units/buildings to defend
	if (BWAPI::Broodwar->getFrameCount() < 5000)
	{	//need to know the next proper trig time
		if (enemyTotalSupply > selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Zergling].size() && selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() < 2)
			ProductionManager::Instance().RushDefend(BWAPI::UnitTypes::Zerg_Sunken_Colony, 0, BWAPI::UnitTypes::Zerg_Zergling, 2);
	}
	else
	{
		if (myTotalArmySupply < enemyTotalSupply)
		{
			if (selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Hydralisk].size() > 0)
				ProductionManager::Instance().RushDefend(BWAPI::UnitTypes::Zerg_Sunken_Colony, 0, BWAPI::UnitTypes::Zerg_Hydralisk, (enemyTotalSupply - myTotalArmySupply) * 2);
			else
				ProductionManager::Instance().RushDefend(BWAPI::UnitTypes::Zerg_Sunken_Colony, 0, BWAPI::UnitTypes::Zerg_Zergling, (enemyTotalSupply - myTotalArmySupply) * 2);
		}
	}
}

void AttackManager::DefendOver()
{
	WorkerManager::Instance().finishedWithCombatWorkers();
	isNeedDefend = false;
	isNeedTacticDefend = false;
}

//TODO: cope with multiply enemy attack at the same time
void AttackManager::DefendEnemy(std::set<BWAPI::Unit *>& enemyUnitsInRegion, int enemyTotalSupply)
{

	if (enemyUnitsInRegion.size() == 1 && (*enemyUnitsInRegion.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() == 0)
	{
		// the enemy worker that is attacking us
		BWAPI::Unit * enemyWorker = *enemyUnitsInRegion.begin();

		//if (enemyWorker->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) > 32 * 10)
		//{
			//WorkerManager::Instance().finishedWithCombatWorkers();
		//}
		//else
		//{
		// get our worker unit that is mining that is closest to it
		BWAPI::Unit * workerDefender = WorkerManager::Instance().getClosestMineralWorkerTo(enemyWorker);

		// grab it from the worker manager
		WorkerManager::Instance().setCombatWorker(workerDefender);
		//}
		/*
		if (triggerZerglingBuilding)
		{
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Zergling, 1);
			triggerZerglingBuilding = false;
		}*/
	}
	//use zergling to kill scout worker
	else if (enemyUnitsInRegion.size() == 1 && (*enemyUnitsInRegion.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 0)
	{
		WorkerManager::Instance().finishedWithCombatWorkers();
		ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(myArmy[BWAPI::UnitTypes::Zerg_Zergling]);
		zerglings->attackScoutWorker((*enemyUnitsInRegion.begin()));
	}
	else
	{
		typedef std::map<BWAPI::UnitType, BattleArmy*>::value_type mapType;
		int armyCount = 0;
		int myArmyTotalSupply = 0;

		BOOST_FOREACH(mapType& p, myArmy)
		{
			if (p.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				p.second->defend((*enemyUnitsInRegion.begin())->getPosition());
				armyCount += p.second->getUnits().size();
				myArmyTotalSupply += ((p.first).supplyRequired() * p.second->getUnits().size());
			}
		}

		isNeedDefend = true;

		std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& myBuilding = InformationManager::Instance().getOurAllBuildingUnit();
		std::set<BWTA::Region*> enemyRegion;
		BOOST_FOREACH(BWAPI::Unit* unit, enemyUnitsInRegion)
		{
			enemyRegion.insert(BWTA::getRegion(unit->getPosition()));
		}

		std::set<BWAPI::Unit*> sunkens = myBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony];
		std::set<BWAPI::Unit*> creeps = myBuilding[BWAPI::UnitTypes::Zerg_Creep_Colony];
		sunkens.insert(creeps.begin(), creeps.end());
		bool hasSunken = false;
		BOOST_FOREACH(BWAPI::Unit* unit, sunkens)
		{
			if (enemyRegion.find(BWTA::getRegion(unit->getPosition())) != enemyRegion.end())
			{
				hasSunken = true;
				break;
			}
		}

		//need tactic army to retreat
		if (myArmyTotalSupply < enemyTotalSupply)
		{
			isNeedTacticDefend = true;
		}
		
		if (armyCount == 0 && sunkens.size() == 0)
		{
			if (WorkerManager::Instance().getNumMineralWorkers() > 5)
			{
				WorkerManager::Instance().setCombatWorkerArmy(BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) - 5);
			}
		}
		else
		{
			WorkerManager::Instance().finishedWithCombatWorkers();
		}
	}
}


void AttackManager::onUnitMorph(BWAPI::Unit* unit)
{
	if (unit == NULL)
	{
		return;
	}
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
	}

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


void AttackManager::addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy, BWAPI::Position attackTarget)
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

	//do not attack the same place within short time
	BOOST_FOREACH(AttackEnemyBase& b, attackPosition)
	{
		if (attackTarget == b.base)
		{
			b.resetAttackTime();
			b.nowAttacking = false;
			break;
		}
	}
}


