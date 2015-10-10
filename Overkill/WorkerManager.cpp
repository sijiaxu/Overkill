
#include "WorkerManager.h"

WorkerManager::WorkerManager()
	: workersPerRefinery(3)
{
	previousClosestWorker = NULL;
	assignScout = false;
	depotTriggerTime = 0;
}

void WorkerManager::update()
{

	handleWorkerReact();

	handleScoutWorker();

	// worker bookkeeping
	updateWorkerStatus();

	// set the gas workers
	handleGasWorkers();

	// handle idle workers
	handleIdleWorkers();

	// handle move workers
	handleMoveWorkers();

	// handle combat workers
	handleCombatWorkers();

	drawResourceDebugInfo();
	//drawWorkerInformation(450,20);

	workerData.drawDepotDebugInfo();
}


void WorkerManager::handleWorkerReact()
{
	if (BWAPI::Broodwar->getFrameCount() > 12000)
		return;

	int droneAttackRange = BWAPI::UnitTypes::Zerg_Drone.groundWeapon().maxRange();
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Gas || workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			std::set<BWAPI::Unit*> unitsNearby = worker->getUnitsInRadius(droneAttackRange * 3);

			int closeDistance = 99999;
			BWAPI::Unit* close = NULL;
			BOOST_FOREACH(BWAPI::Unit* enemy, unitsNearby)
			{
				if (enemy->getPlayer() == BWAPI::Broodwar->enemy() && !enemy->getType().isFlyer() 
					&& enemy->getType().canAttack() && enemy->isAttacking() && worker->getDistance(enemy->getPosition()) < closeDistance)
				{
					closeDistance = worker->getDistance(enemy->getPosition());
					close = enemy;
				}
			}

			if (close != NULL)
			{
				smartAttackUnit(worker, close);
			}
			else
			{
				if (!worker->isGatheringMinerals() && !worker->isGatheringGas())
				{
					if (workerData.getWorkerJob(worker) == WorkerData::Minerals)
						worker->rightClick(workerData.getMineralWorkerMine(worker));
					else
						worker->rightClick(workerData.getGasWorkerRefinery(worker));
				}
			}
		}
	}
}



void WorkerManager::handleScoutWorker()
{
	//send the 10th worker to scout if we have not find the enemy base
	if (workerData.getWorkers().size() == 9 && InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None &&
		ScoutManager::Instance().getPossibleEnemyBase().size() > 0 && !assignScout)
	{
		BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
		{
			if (workerData.getWorkerJob(worker) == WorkerData::Minerals)
			{
				setScoutWorker(worker);
				ScoutManager::Instance().addScoutUnit(worker);
				assignScout = true;
				break;
			}
		}
	}
}


void WorkerManager::balanceWorkerOnDepotComplete(BWAPI::Unit* depot)
{
	workerData.balanceWorker(depot);
}


void WorkerManager::updateWorkerStatus()
{
	// for each of our Workers
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}

		// if it's idle
		if (worker->isIdle() &&
			(workerData.getWorkerJob(worker) != WorkerData::Build) &&
			(workerData.getWorkerJob(worker) != WorkerData::Move) &&
			(workerData.getWorkerJob(worker) != WorkerData::Scout))
		{
			//printf("Worker %d set to idle", worker->getID());
			// set its job to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}

		// if its job is gas
		if (workerData.getWorkerJob(worker) == WorkerData::Gas)
		{
			BWAPI::Unit * refinery = workerData.getWorkerResource(worker);

			// if the refinery doesn't exist anymore
			if (!refinery || !refinery->exists() || refinery->getHitPoints() <= 0)
			{
				setMineralWorker(worker);
			}
		}
	}
}


void WorkerManager::handleGasWorkers()
{
	//if in the middle game stage, we have worker less than 10(means most worker have been killed), mine first
	if (workerData.getNumWorkers() <= 10 && ProductionManager::Instance().getTopProductionNeed() && BWAPI::Broodwar->getFrameCount() > 5000)
	{
		BOOST_FOREACH(BWAPI::Unit * unit, workerData.getWorkers())
		{
			if (workerData.getWorkerJob(unit) == WorkerData::Gas)
			{
				setMineralWorker(unit);
			}
		}
		return;
	}

	// for each unit we have
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		// if that unit is a refinery
		if (unit->getType().isRefinery() && unit->isCompleted())
		{
			// get the number of workers currently assigned to it
			int numAssigned = workerData.getNumAssignedWorkers(unit);

			// if it's less than we want it to be, fill 'er up
			for (int i = 0; i < (workersPerRefinery - numAssigned); ++i)
			{
				BWAPI::Unit * gasWorker = getGasWorker(unit);
				if (gasWorker)
				{
					workerData.setWorkerJob(gasWorker, WorkerData::Gas, unit);
				}
			}
		}
	}
}


void WorkerManager::handleIdleWorkers()
{
	// for each of our workers
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is idle
		if (workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			// send it to the nearest mineral patch
			setMineralWorker(worker);
		}
	}
}

// bad micro for combat workers
void WorkerManager::handleCombatWorkers()
{
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			BWAPI::Broodwar->drawCircleMap(worker->getPosition().x(), worker->getPosition().y(), 4, BWAPI::Colors::Yellow, true);
			BWAPI::Unit * target = getClosestEnemyUnit(worker);

			if (target)
			{
				smartAttackUnit(worker, target);
			}
		}
	}
}

void WorkerManager::setCombatWorkerArmy(int combatWorkerNum)
{
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Minerals || (workerData.getWorkerJob(worker) == WorkerData::Gas))
		{
			workerData.setWorkerJob(worker, WorkerData::Combat, NULL);
			combatWorkerNum--;
			if (combatWorkerNum == 0)
				break;
		}
	}
	
}

BWAPI::Unit * WorkerManager::getClosestEnemyUnit(BWAPI::Unit * worker)
{
	BWAPI::Unit * closestUnit = NULL;
	double closestDist = 10000;

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(worker);
		//if enemy is too far away, do not attack,keep mining
		if ((dist < 400) && (!closestUnit || (dist < closestDist)))
		{
			closestUnit = unit;
			closestDist = dist;
		}
	}

	return closestUnit;
}

void WorkerManager::finishedWithCombatWorkers()
{
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			setMineralWorker(worker);
		}
	}
}

BWAPI::Unit * WorkerManager::getClosestMineralWorkerTo(BWAPI::Unit * enemyUnit)
{
	BWAPI::Unit * closestMineralWorker = NULL;
	double closestDist = 100000;

	if (previousClosestWorker)
	{
		if (previousClosestWorker->getHitPoints() > 20)
		{
			return previousClosestWorker;
		}
		else
		{
			workerData.setWorkerJob(previousClosestWorker, WorkerData::Idle, NULL);
		}

	}

	// for each of our workers
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			double dist = worker->getDistance(enemyUnit);

			if (!closestMineralWorker || dist < closestDist)
			{
				closestMineralWorker = worker;
				dist = closestDist;
			}
		}
	}

	previousClosestWorker = closestMineralWorker;
	return closestMineralWorker;
}

void WorkerManager::handleMoveWorkers()
{
	// for each of our workers
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Move)
		{
			WorkerMoveData data = workerData.getWorkerMoveData(worker);

			worker->move(data.position);
		}
	}
}

// set a worker to mine minerals
void WorkerManager::setMineralWorker(BWAPI::Unit * unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// check if there is a mineral available to send the worker to
	BWAPI::Unit * depot = getClosestDepot(unit);

	// if there is a valid mineral
	if (depot)
	{
		if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
			return;
		else
			// update workerData with the new job
			workerData.setWorkerJob(unit, WorkerData::Minerals, depot);
	}
	/*
	else
	{
		if (BWAPI::Broodwar->getFrameCount() > depotTriggerTime)
		{
			StrategyManager::Instance().baseExpand();
			depotTriggerTime = BWAPI::Broodwar->getFrameCount() + 60 * 30;
		}
		//BWAPI::Broodwar->printf("No valid depot for mineral worker");
	}*/
}

BWAPI::Unit * WorkerManager::getClosestDepot(BWAPI::Unit * worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	BWAPI::Unit * closestDepot = NULL;
	double closestDistance = 9999999;

	BOOST_FOREACH(BWAPI::Unit* myBase, InformationManager::Instance().getOurAllBaseUnit())
	{
		if ((myBase->isCompleted() || myBase->getType() != BWAPI::UnitTypes::Zerg_Hatchery) && !workerData.depotIsFull(myBase))
		{
			double distance = worker->getDistance(myBase);
			if (!closestDepot || distance < closestDistance)
			{
				closestDepot = myBase;
				closestDistance = distance;
			}
		}
	}
		

	return closestDepot;
}


// other managers that need workers call this when they're done with a unit
void WorkerManager::finishedWithWorker(BWAPI::Unit * unit)
{
	if (!unit->exists())
	{
		BWAPI::Broodwar->printf("finishedWithWorker() called with NULL unit");
		return;
	}

	workerData.setWorkerJob(unit, WorkerData::Idle, NULL);
}

BWAPI::Unit * WorkerManager::getGasWorker(BWAPI::Unit * refinery)
{
	if (refinery == NULL)
	{
		assert(false);
	}

	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 0;

	BOOST_FOREACH(BWAPI::Unit * unit, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			double distance = unit->getDistance(refinery);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	return closestWorker;
}

// gets a builder for BuildingManager to use
// if setJobAsBuilder is true (default), it will be flagged as a builder unit
// set 'setJobAsBuilder' to false if we just want to see which worker will build a building
BWAPI::Unit * WorkerManager::getBuilder(Building & b, bool setJobAsBuilder)
{
	BWAPI::Unit * closestMovingWorker = NULL;
	BWAPI::Unit * closestMiningWorker = NULL;
	double closestMovingWorkerDistance = 0;
	double closestMiningWorkerDistance = 0;

	// look through each worker that had moved there first
	BOOST_FOREACH(BWAPI::Unit * unit, workerData.getWorkers())
	{
		// mining worker check
		if (unit->isCompleted() && (workerData.getWorkerJob(unit) == WorkerData::Minerals))
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMiningWorker || distance < closestMiningWorkerDistance)
			{
				closestMiningWorker = unit;
				closestMiningWorkerDistance = distance;
			}
		}

		// moving worker check
		if (unit->isCompleted() && (workerData.getWorkerJob(unit) == WorkerData::Move))
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMovingWorker || distance < closestMovingWorkerDistance)
			{
				closestMovingWorker = unit;
				closestMovingWorkerDistance = distance;
			}
		}
	}

	BWAPI::Unit * chosenWorker = NULL;
	if (closestMovingWorker != NULL)
	{
		if (closestMiningWorker != NULL)
			chosenWorker = closestMiningWorkerDistance < closestMovingWorkerDistance ? closestMiningWorker : closestMovingWorker;
		else
			chosenWorker = closestMovingWorker;
	}
	else
	{
		if (closestMiningWorker != NULL)
			chosenWorker = closestMiningWorker;
	}
		

	// if we found a moving worker, use it, otherwise using a mining worker
	//BWAPI::Unit * chosenWorker = closestMovingWorker ? closestMovingWorker : closestMiningWorker;

	// if the worker exists (one may not have been found in rare cases)
	if (chosenWorker && setJobAsBuilder)
	{
		//workerData.setWorkerJob(chosenWorker, WorkerData::Build, b.type);
		workerData.setWorkerJob(chosenWorker, WorkerData::Build, NULL);
	}

	// return the worker
	return chosenWorker;
}

// sets a worker as a scout
void WorkerManager::setScoutWorker(BWAPI::Unit * worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Scout, NULL);
}

// gets a worker which will move to a current location
BWAPI::Unit * WorkerManager::getMoveWorker(BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 99999;

	// for each worker we currently have
	BOOST_FOREACH(BWAPI::Unit * unit, workerData.getWorkers())
	{
		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	// return the worker
	return closestWorker;
}

// sets a worker to move to a given location
void WorkerManager::setMoveWorker(int mineralsNeeded, int gasNeeded, BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 0;

	// for each worker we currently have
	BOOST_FOREACH(BWAPI::Unit * unit, workerData.getWorkers())
	{
		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	if (closestWorker)
	{
		//BWAPI::Broodwar->printf("Setting worker job Move for worker %d", closestWorker->getID());
		workerData.setWorkerJob(closestWorker, WorkerData::Move, WorkerMoveData(mineralsNeeded, gasNeeded, p));
		//workerData.setWorkerJob(closestWorker, WorkerData::Move, NULL);
	}
	else
	{
		//BWAPI::Broodwar->printf("Error, no worker found");
	}
}

// will we have the required resources by the time a worker can travel a certain distance
bool WorkerManager::willHaveResources(int mineralsRequired, int gasRequired, double distance)
{
	// if we don't require anything, we will have it
	if (mineralsRequired <= 0 && gasRequired <= 0)
	{
		return true;
	}

	// the speed of the worker unit
	double speed = BWAPI::Broodwar->self()->getRace().getWorker().topSpeed();

	// how many frames it will take us to move to the building location
	// add a second to account for worker getting stuck. better early than late
	double framesToMove = (distance / speed) + 100;

	// magic numbers to predict income rates
	double mineralRate = getNumMineralWorkers() * 0.045;
	double gasRate = getNumGasWorkers() * 0.07;

	// calculate if we will have enough by the time the worker gets there
	if (mineralRate * framesToMove >= mineralsRequired &&
		gasRate * framesToMove >= gasRequired)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WorkerManager::setCombatWorker(BWAPI::Unit * worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Combat, NULL);
}

void WorkerManager::onUnitMorph(BWAPI::Unit * unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		workerData.addWorker(unit);
	}

	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			if (base->getTilePosition().getDistance(unit->getTilePosition()) < 3)
			{
				workerData.addDepot(unit);
				break;
			}
		}
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getPlayer()->getRace() == BWAPI::Races::Zerg)
	{
		//BWAPI::Broodwar->printf("A Drone started building");
		workerData.workerDestroyed(unit);
	}
}

void WorkerManager::onUnitShow(BWAPI::Unit * unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// add the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.addDepot(unit);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
		workerData.addWorker(unit);
	}
}


void WorkerManager::rebalanceWorkers()
{
	// for each worker
	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers())
	{
		// we only care to rebalance mineral workers
		if (!workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			continue;
		}

		// get the depot this worker works for
		BWAPI::Unit * depot = workerData.getWorkerDepot(worker);

		// if there is a depot and it's full
		if (depot && workerData.depotIsFull(depot))
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
		// if there's no depot
		else if (!depot)
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
	}
}

void WorkerManager::onUnitDestroy(BWAPI::Unit * unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// remove the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.removeDepot(unit);
	}

	// if the unit that was destroyed is a worker
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		// tell the worker data it was destroyed
		workerData.workerDestroyed(unit);
	}

	if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field)
	{
		//BWAPI::Broodwar->printf("A mineral died, rebalancing workers");

		rebalanceWorkers();
	}
}

void WorkerManager::smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target)
{
	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
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
}

void WorkerManager::drawResourceDebugInfo() {

	BOOST_FOREACH(BWAPI::Unit * worker, workerData.getWorkers()) {

		char job = workerData.getJobCode(worker);

		BWAPI::Position pos = worker->getTargetPosition();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(worker->getPosition().x(), worker->getPosition().y() - 5, "\x07%c", job);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x(), worker->getPosition().y(), pos.x(), pos.y(), BWAPI::Colors::Cyan);

		BWAPI::Unit * depot = workerData.getWorkerDepot(worker);
		if (depot)
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x(), worker->getPosition().y(), depot->getPosition().x(), depot->getPosition().y(), BWAPI::Colors::Orange);
		}
	}
}


bool WorkerManager::isFree(BWAPI::Unit * worker)
{
	return workerData.getWorkerJob(worker) == WorkerData::Minerals || workerData.getWorkerJob(worker) == WorkerData::Idle;
}

bool WorkerManager::isWorkerScout(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Scout);
}

bool WorkerManager::isBuilder(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Build);
}

int WorkerManager::getNumMineralWorkers()
{
	return workerData.getNumMineralWorkers();
}

int WorkerManager::getNumIdleWorkers()
{
	return workerData.getNumIdleWorkers();
}

int WorkerManager::getNumGasWorkers()
{
	return workerData.getNumGasWorkers();
}


WorkerManager & WorkerManager::Instance() {

	static WorkerManager instance;
	return instance;
}