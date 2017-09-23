
#include "WorkerData.h"

WorkerData::WorkerData()
{
	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field))
		{
			workersOnMineralPatch[unit] = 0;
		}
	}

	depotBalanceFlag = true;
	waitforDepotTime = 0;
}

void WorkerData::workerDestroyed(BWAPI::Unit unit)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workers.erase(unit);
}

void WorkerData::addWorker(BWAPI::Unit unit)
{
	if (!unit) { return; }

	workers.insert(unit);
	workerJobMap[unit] = Default;
}

void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit || !jobUnit) { return; }

	assert(workers.find(unit) == workers.end());

	workers.insert(unit);
	setWorkerJob(unit, job, jobUnit);
}

void WorkerData::addWorker(BWAPI::Unit unit, enum WorkerJob job, BWAPI::UnitType jobUnitType)
{
	if (!unit) { return; }

	assert(workers.find(unit) == workers.end());
	workers.insert(unit);
	setWorkerJob(unit, job, jobUnitType);
}

void WorkerData::addDepot(BWAPI::Unit unit)
{
	if (!unit) { return; }

	if (depots.find(unit) == depots.end())
	{
		depots.insert(unit);
		depotWorkerCount[unit] = 0;
	}
}

void WorkerData::removeDepot(BWAPI::Unit unit)
{
	if (!unit) { return; }

	depots.erase(unit);
	depotWorkerCount.erase(unit);

	// re-balance workers in here
	BOOST_FOREACH(BWAPI::Unit worker, workers)
	{
		// if a worker was working at this depot
		if (workerDepotMap[worker] == unit)
		{
			setWorkerJob(worker, Idle, NULL);
		}
	}
}

void WorkerData::addToMineralPatch(BWAPI::Unit unit, int num)
{
	if (workersOnMineralPatch.find(unit) == workersOnMineralPatch.end())
	{
		workersOnMineralPatch[unit] = num;
	}
	else
	{
		workersOnMineralPatch[unit] = workersOnMineralPatch[unit] + num;
	}
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Minerals)
	{
		// increase the number of workers assigned to this nexus
		depotWorkerCount[jobUnit] += 1;

		// set the mineral the worker is working on
		workerDepotMap[unit] = jobUnit;

		// get the mineral in the mineral patch closet to depot and have minimum worker
		BWAPI::Unit mineralToMine = getMineralToMine(unit);
		workerMineralAssignment[unit] = mineralToMine;
		addToMineralPatch(mineralToMine, 1);

		// right click the mineral to start mining
		unit->gather(mineralToMine);
	}
	else if (job == Gas)
	{
		// increase the count of workers assigned to this refinery
		refineryWorkerCount[jobUnit] += 1;

		// set the refinery the worker is working on
		workerRefineryMap[unit] = jobUnit;

		// right click the refinery to start harvesting
		unit->gather(jobUnit);
	}
	else if (job == Repair)
	{
		// only SCVs can repair
		assert(unit->getType() == BWAPI::UnitTypes::Terran_SCV);

		// set the building the worker is to repair
		workerRepairMap[unit] = jobUnit;

		// start repairing 
		unit->repair(jobUnit);
	}
	else if (job == Combat)
	{

	}
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::UnitType jobUnitType)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Build)
	{
		workerBuildingTypeMap[unit] = jobUnitType;
	}
}

void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, WorkerMoveData wmd)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workerJobMap[unit] = job;

	if (job == Move)
	{
		workerMoveMap[unit] = wmd;
	}

	if (workerJobMap[unit] != Move)
	{
		BWAPI::Broodwar->printf("Something went horribly wrong");
	}
}


void WorkerData::clearPreviousJob(BWAPI::Unit unit)
{
	if (!unit) { return; }

	WorkerJob previousJob = getWorkerJob(unit);

	if (previousJob == Minerals)
	{
		depotWorkerCount[workerDepotMap[unit]] -= 1;

		workerDepotMap.erase(unit);

		// remove a worker from this unit's assigned mineral patch
		addToMineralPatch(workerMineralAssignment[unit], -1);

		// erase the association from the map
		workerMineralAssignment.erase(unit);
	}
	else if (previousJob == Gas)
	{
		refineryWorkerCount[workerRefineryMap[unit]] -= 1;
		workerRefineryMap.erase(unit);
	}
	else if (previousJob == Build)
	{
		workerBuildingTypeMap.erase(unit);
	}
	else if (previousJob == Repair)
	{
		workerRepairMap.erase(unit);
	}
	else if (previousJob == Move)
	{
		workerMoveMap.erase(unit);
	}

	workerJobMap.erase(unit);
}

int WorkerData::getNumWorkers()
{
	return workers.size();
}

int WorkerData::getNumMineralWorkers()
{
	size_t num = 0;
	BOOST_FOREACH(BWAPI::Unit unit, workers)
	{
		if (workerJobMap[unit] == WorkerData::Minerals)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumGasWorkers()
{
	size_t num = 0;
	BOOST_FOREACH(BWAPI::Unit unit, workers)
	{
		if (workerJobMap[unit] == WorkerData::Gas)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumIdleWorkers()
{
	size_t num = 0;
	BOOST_FOREACH(BWAPI::Unit unit, workers)
	{
		if (workerJobMap[unit] == WorkerData::Idle)
		{
			num++;
		}
	}
	return num;
}


enum WorkerData::WorkerJob WorkerData::getWorkerJob(BWAPI::Unit unit)
{
	if (!unit) { return Default; }

	std::map<BWAPI::Unit, enum WorkerJob>::iterator it = workerJobMap.find(unit);

	if (it != workerJobMap.end())
	{
		return it->second;
	}

	return Default;
}

bool WorkerData::depotIsFull(BWAPI::Unit depot)
{
	if (!depot) { return false; }

	int assignedWorkers = getNumAssignedWorkers(depot);
	int mineralsNearDepot = getMineralsNearDepot(depot);

	//one mineral have three worker is maximum
	if (assignedWorkers >= int(mineralsNearDepot * 2.5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<BWAPI::Unit> WorkerData::getMineralPatchesNearDepot(BWAPI::Unit depot)
{
	// if there are minerals near the depot, add them to the set
	std::vector<BWAPI::Unit> mineralsNearDepot;
	int radius = 300;

	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && unit->getDistance(depot) < radius)
		{
			mineralsNearDepot.push_back(unit);
		}
	}

	const std::set<BWTA::Chokepoint*>& depotChokes = BWTA::getRegion(depot->getPosition())->getChokepoints();
	std::set<BWAPI::Unit> mineralsAtChoke;
	for (auto m : BWAPI::Broodwar->getMinerals())
	{
		for (auto r : depotChokes)
		{
			if (m->getInitialPosition().getDistance(r->getCenter()) < 2 * 32)
			{
				mineralsAtChoke.insert(m);
			}
		}
	}
	mineralsNearDepot.insert(mineralsNearDepot.end(), mineralsAtChoke.begin(), mineralsAtChoke.end());

	// if we didn't find any, use the whole map
	if (mineralsNearDepot.empty())
	{
		for (auto m : BWAPI::Broodwar->getMinerals())
		{
			mineralsNearDepot.push_back(m);
		}
	}

	return mineralsNearDepot;
}


int WorkerData::getMineralsNearDepot(BWAPI::Unit depot)
{
	if (!depot) { return 0; }

	std::vector<BWAPI::Unit> mineralsNearDepot;
	int radius = 300;

	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && unit->getDistance(depot) < radius)
		{
			mineralsNearDepot.push_back(unit);
		}
	}

	const std::set<BWTA::Chokepoint*>& depotChokes = BWTA::getRegion(depot->getPosition())->getChokepoints();
	std::set<BWAPI::Unit> mineralsAtChoke;
	for (auto m : BWAPI::Broodwar->getMinerals())
	{
		for (auto r : depotChokes)
		{
			if (m->getInitialPosition().getDistance(r->getCenter()) < 2 * 32)
			{
				mineralsAtChoke.insert(m);
			}
		}
	}
	mineralsNearDepot.insert(mineralsNearDepot.end(), mineralsAtChoke.begin(), mineralsAtChoke.end());


	return mineralsNearDepot.size();
}

BWAPI::Unit WorkerData::getWorkerResource(BWAPI::Unit unit)
{
	if (!unit) { return NULL; }

	// create the iterator
	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it;

	// if the worker is mining, set the iterator to the mineral map
	if (getWorkerJob(unit) == Minerals)
	{
		it = workerMineralMap.find(unit);
		if (it != workerMineralMap.end())
		{
			return it->second;
		}
	}
	else if (getWorkerJob(unit) == Gas)
	{
		it = workerRefineryMap.find(unit);
		if (it != workerRefineryMap.end())
		{
			return it->second;
		}
	}

	return NULL;
}

int	WorkerData::getDepotWorker(BWAPI::Unit depot)
{
	if (depotWorkerCount.find(depot) != depotWorkerCount.end())
		return depotWorkerCount[depot];
	else
		return 0;
}


void WorkerData::balanceWorker(BWAPI::Unit depot)
{
	int currentMineralWorker = 0;
	typedef std::map<BWAPI::Unit, int>::value_type countValue;
	BOOST_FOREACH(countValue count, depotWorkerCount)
	{
		currentMineralWorker += count.second;
	}
	int balanceWorkerCount = currentMineralWorker / depotWorkerCount.size();

	BOOST_FOREACH(BWAPI::Unit worker, workers)
	{
		if (workerJobMap[worker] == Minerals && workerDepotMap[worker] != depot)
		{
			setWorkerJob(worker, WorkerData::Minerals, depot);
			balanceWorkerCount--;
			if (balanceWorkerCount == 0)
				break;
		}
	}
}

BWAPI::Unit WorkerData::getMineralToMine(BWAPI::Unit worker)
{
	if (!worker) { return NULL; }

	// get the depot associated with this unit
	BWAPI::Unit depot = getWorkerDepot(worker);
	BWAPI::Unit bestMineral = NULL;
	double bestDist = 100000;
	double bestNumAssigned = 10000;

	if (depot)
	{
		std::vector<BWAPI::Unit> mineralPatches = getMineralPatchesNearDepot(depot);

		BOOST_FOREACH(BWAPI::Unit mineral, mineralPatches)
		{
			double dist = mineral->getDistance(depot);
			double numAssigned = workersOnMineralPatch[mineral];

			if (numAssigned < bestNumAssigned || (numAssigned == bestNumAssigned && dist < bestDist))
			{
				bestMineral = mineral;
				bestDist = dist;
				bestNumAssigned = numAssigned;
			}
		}
	}

	return bestMineral;
}

BWAPI::Unit WorkerData::getWorkerRepairUnit(BWAPI::Unit unit)
{
	if (!unit) { return NULL; }

	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it = workerRepairMap.find(unit);

	if (it != workerRepairMap.end())
	{
		return it->second;
	}

	return NULL;
}

BWAPI::Unit WorkerData::getWorkerDepot(BWAPI::Unit unit)
{
	if (!unit) { return NULL; }

	std::map<BWAPI::Unit, BWAPI::Unit>::iterator it = workerDepotMap.find(unit);

	if (it != workerDepotMap.end())
	{
		return it->second;
	}

	return NULL;
}

BWAPI::UnitType	WorkerData::getWorkerBuildingType(BWAPI::Unit unit)
{
	if (!unit) { return BWAPI::UnitTypes::None; }

	std::map<BWAPI::Unit, BWAPI::UnitType>::iterator it = workerBuildingTypeMap.find(unit);

	if (it != workerBuildingTypeMap.end())
	{
		return it->second;
	}

	return BWAPI::UnitTypes::None;
}

WorkerMoveData WorkerData::getWorkerMoveData(BWAPI::Unit unit)
{
	std::map<BWAPI::Unit, WorkerMoveData>::iterator it = workerMoveMap.find(unit);

	assert(it != workerMoveMap.end());

	return (it->second);
}

int WorkerData::getNumAssignedWorkers(BWAPI::Unit unit)
{
	if (!unit) { return 0; }

	std::map<BWAPI::Unit, int>::iterator it;

	// if the worker is mining, set the iterator to the mineral map
	if (unit->getType().isResourceDepot())
	{
		it = depotWorkerCount.find(unit);

		// if there is an entry, return it
		if (it != depotWorkerCount.end())
		{
			return it->second;
		}
	}
	else if (unit->getType().isRefinery())
	{
		it = refineryWorkerCount.find(unit);

		// if there is an entry, return it
		if (it != refineryWorkerCount.end())
		{
			return it->second;
		}
		// otherwise, we are only calling this on completed refineries, so set it
		else
		{
			refineryWorkerCount[unit] = 0;
		}
	}

	// when all else fails, return 0
	return 0;
}

char WorkerData::getJobCode(BWAPI::Unit unit)
{
	if (!unit) { return 'X'; }

	WorkerData::WorkerJob j = getWorkerJob(unit);

	if (j == WorkerData::Build) return 'B';
	if (j == WorkerData::Combat) return 'C';
	if (j == WorkerData::Default) return 'D';
	if (j == WorkerData::Gas) return 'G';
	if (j == WorkerData::Idle) return 'I';
	if (j == WorkerData::Minerals) return 'M';
	if (j == WorkerData::Repair) return 'R';
	if (j == WorkerData::Move) return 'O';
	if (j == WorkerData::Scout) return 'S';
	return 'X';
}

void WorkerData::drawDepotDebugInfo()
{
	BOOST_FOREACH(BWAPI::Unit depot, depots)
	{
		int x = depot->getPosition().x - 64;
		int y = depot->getPosition().y - 32;

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x - 2, y - 1, x + 75, y + 14, BWAPI::Colors::Black, true);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(x, y, "\x04 Workers: %d", getNumAssignedWorkers(depot));

		std::vector<BWAPI::Unit> minerals = getMineralPatchesNearDepot(depot);

		for (size_t m(0); m<minerals.size(); ++m)
		{
			BWAPI::Unit mineral = minerals[m];

			int x = mineral->getPosition().x;
			int y = mineral->getPosition().y;

			if (workersOnMineralPatch.find(mineral) != workersOnMineralPatch.end())
			{
				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x - 2, y - 1, x + 75, y + 14, BWAPI::Colors::Black, true);
				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(x, y, "\x04 Workers: %d", workersOnMineralPatch[mineral]);
			}
		}
	}
}