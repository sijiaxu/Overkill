
#include "ProductionManager.h"


#define BOADD(N, T, B) for (int i=0; i<N; ++i) { queue.queueAsLowestPriority(MetaType(T), B); }

#define GOAL_ADD(G, M, N) G.push_back(std::pair<MetaType, int>(M, N))

ProductionManager::ProductionManager()
	: initialBuildSet(false)
	, reservedMinerals(0)
	, reservedGas(0)
	, assignedWorkerForThisBuilding(false)
	, haveLocationForThisBuilding(false)
	, enemyCloakedDetected(false)
	, rushDetected(false)
{
	populateTypeCharMap();

	setBuildOrder(StrategyManager::Instance().getOpeningBook());

	nextSupplyDeadlockDetectTime = 0;

	productionState = fixBuildingOrder;

	HatcheryProductionCheckTime = 0;
}

void ProductionManager::setBuildOrder(const std::vector<MetaType> & buildOrder)
{
	// clear the current build order
	//queue.clearAll();

	// for each item in the results build order, add it
	for (size_t i(0); i < buildOrder.size(); ++i)
	{
		// queue the item
		if (buildOrder[i].isUpgrade() || buildOrder[i].isTech())
			queue.queueAsLowestPriority(buildOrder[i], false);
		else
			queue.queueAsLowestPriority(buildOrder[i], true);
	}
}


void ProductionManager::triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (buildingType == BWAPI::UnitTypes::Zerg_Sunken_Colony || buildingType == BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			queue.queueAsHighestPriority(MetaType(buildingType), false);
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, buildingLocation), true);
		}
		else if (buildingType == BWAPI::UnitTypes::Zerg_Extractor)
		{
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
		}
		else
		{
			queue.queueAsHighestPriority(MetaType(buildingType, buildingLocation), true);
		}
	}
}

void ProductionManager::triggerUnit(BWAPI::UnitType unitType, int unitCount)
{
	for (int i = 0; i < unitCount; i++)
	{
		queue.queueAsHighestPriority(MetaType(unitType), true);
	}
}


void ProductionManager::RushDefend(BWAPI::UnitType defendBuilding, int buildingCount, BWAPI::UnitType defendUnit, int unitCount)
{
	BWAPI::Position chokePosition = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
	BWAPI::Position basePositon = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	double2 direc = chokePosition - basePositon;
	double2 direcNormal = direc / direc.len();
	int targetx = (basePositon.x() + int(direcNormal.x * 32 * 10)) / 32;
	int targety = (basePositon.y() + int(direcNormal.y * 32 * 10)) / 32;

	for (int i = 0; i < unitCount; i++)
	{
		queue.queueAsHighestPriority(defendUnit, false);
	}

	for (int i = 0; i < buildingCount; i++)
	{
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::TilePosition(targetx, targety)), true);
		queue.queueAsHighestPriority(defendBuilding, false);
	}
}


void ProductionManager::defendLostRecover()
{
	if (recoverBuilding.size() == 0 && recoverDroneCount == 0)
		return;

	std::vector<RecoverBuildingInfo> recoverSunker;

	BOOST_FOREACH(RecoverBuildingInfo u, recoverBuilding)
	{
		if (u.buildingType == BWAPI::UnitTypes::Zerg_Sunken_Colony)
		{
			recoverSunker.push_back(u);
		}
		else
		{
			queue.queueAsHighestPriority(MetaType(u.buildingType, u.buildingPosition), true);
		}
	}

	for (int i = 0; i < int(recoverSunker.size()) / 2; i++)
	{
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), false);
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, recoverSunker[i].buildingPosition), false);
	}

	recoverDroneCount += recoverBuilding.size();

	//reproduce worker first
	for (int i = 0; i < recoverDroneCount; i++)
	{
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Drone), false);
	}

	recoverBuilding.clear();
	recoverDroneCount = 0;

}

void ProductionManager::clearDefendVector()
{
	recoverBuilding.clear();
	recoverDroneCount = 0;
}

void ProductionManager::update()
{
	//// detect if we need to morph overlord first.
	//note check too quick may lead to build more supply
	// do not trigger for early game
	if (BWAPI::Broodwar->getFrameCount() > nextSupplyDeadlockDetectTime && detectBuildOrderDeadlock())
	{
		BWAPI::Broodwar->printf("Supply deadlock detected, building OverLord!");
		queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
	}

	// check the queue for stuff we can build
	manageBuildOrderQueue();


	// production FSM
	switch (productionState)
	{
	case ProductionManager::fixBuildingOrder:
	{
		bool isAllUpgrade = true;
		for (int i = 0; i < int(queue.size()); i++)
		{
			if (!queue[i].metaType.isTech() && !queue[i].metaType.isUpgrade())
				isAllUpgrade = false;
		}

		if (isAllUpgrade && (BWAPI::Broodwar->getFrameCount() > 5000))
		{
			BWAPI::Broodwar->drawTextScreen(150, 10, "fix building order finish, start new goal");
			productionState = goalOriented;
			
			StrategyManager::Instance().goalChange(MutaHydraRush);

			StrategyManager::Instance().baseExpand();
		}
		/*
		else if (queue.size() <= 6 )
		{
			bool isAllSunker = true;
			for (int i = 0; i < int(queue.size()); i++)
			{
				if (queue[i].metaType.unitType != BWAPI::UnitTypes::Zerg_Sunken_Colony
					|| queue[i].metaType.unitType != BWAPI::UnitTypes::Zerg_Creep_Colony)
					isAllSunker = false;
			}
			if (isAllSunker)
			{
				queue.clearAll();
				BWAPI::Broodwar->drawTextScreen(150, 10, "fix building order finish, start new goal");
				productionState = goalOriented;
			}
		}*/
	}
	break;
	case ProductionManager::goalOriented:
	{
		onGoalProduction();
		onDroneProduction();
		onHatcheryProduction();
	}
	break;
	default:
		break;
	}
}

void ProductionManager::onGoalProduction()
{
	bool isAllUpgrade = true;
	for (int i = 0; i < int(queue.size()); i++)
	{
		if (!queue[i].metaType.isTech() && !queue[i].metaType.isUpgrade())
			isAllUpgrade = false;
	}
	if (isAllUpgrade)
	{
		BWAPI::Broodwar->drawTextScreen(150, 10, "get current building Battle unit");
		setBuildOrder(StrategyManager::Instance().getCurrentGoalBuildingOrder());
	}
}


void ProductionManager::onDroneProduction()
{
	if (productionState != goalOriented)
		return;

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& allUnits = InformationManager::Instance().getOurAllBattleUnit();
	//keep the drone production interval the same as one base's larva production rate
	if (allUnits[BWAPI::UnitTypes::Zerg_Drone].size() <= 60 && BWAPI::Broodwar->getFrameCount() % 250 == 0)//BWAPI::Broodwar->getFrameCount() % 350 == 0)
	{
		int highPriority = queue.getHighestPriorityValue();
		int lowPriority = queue.getLowestPriorityValue();
		int itemPriority = lowPriority + int((highPriority - lowPriority) * 0.8);
		queue.queueItem(BuildOrderItem<PRIORITY_TYPE>(MetaType(BWAPI::UnitTypes::Zerg_Drone), itemPriority, true));
		//queue.queueAsLowestPriority(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	}



}


void ProductionManager::onHatcheryProduction()
{
	if (productionState != goalOriented)
		return;

	bool notHaveLarva = true;
	BOOST_FOREACH(BWAPI::Unit* u, BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType() == BWAPI::UnitTypes::Zerg_Larva)
		{
			notHaveLarva = false;
			break;
		}
	}

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();
	if (BWAPI::Broodwar->getFrameCount() > HatcheryProductionCheckTime && notHaveLarva && getFreeMinerals() >= 300 && selfAllBuilding[BWAPI::UnitTypes::Zerg_Hatchery].size() <= 10)
	{
		HatcheryProductionCheckTime = BWAPI::Broodwar->getFrameCount() + 30 * 30;
		std::set<BWTA::Region *> & ourRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

		int workingDepots = 0;
		BOOST_FOREACH(BWTA::Region * r, ourRegion)
		{
			int mineralsNearDepot = 0;
			BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
			{
				if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && BWTA::getRegion(unit->getPosition()) == r)
				{
					mineralsNearDepot++;
				}
			}
			if (mineralsNearDepot > 0)
				workingDepots++;
		}

		if (workingDepots <= 5 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) > 12 * workingDepots)
		{
			StrategyManager::Instance().baseExpand();
		}
		else
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self()->getStartLocation()), true);
	}
}


// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit * unit)
{
	if (unit->getType().isWorker())
		recoverDroneCount++;
	if (unit->getType().isBuilding()) //&& unit->getType() != BWAPI::UnitTypes::Zerg_Sunken_Colony
		//&& unit->getType() != BWAPI::UnitTypes::Zerg_Creep_Colony && unit->getType() != BWAPI::UnitTypes::Zerg_Spore_Colony)
	{
		recoverBuilding.push_back(RecoverBuildingInfo(unit->getType(), unit->getTilePosition()));
	}
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() <= 9 * 2)
	{
		BWAPI::Broodwar->printf("trigger overlord!!!!!!!!!!!!!!!!!!!");
		triggerUnit(BWAPI::UnitTypes::Zerg_Overlord, 1);
	}
	// keep the total mutalisk's number
	
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		if (!StrategyManager::Instance().mutaliskHarassFlag() && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) < 24)
			triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 1);
		else if (StrategyManager::Instance().mutaliskHarassFlag() && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) < 12)
			triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 1);
		else
			return;
	}
		
}


void ProductionManager::manageBuildOrderQueue()
{
	// if there is nothing in the queue, oh well
	if (queue.isEmpty())
	{
		return;
	}

	// the current item to be used
	BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();

	// while there is still something left in the queue, we can skip something and continue build
	while (!queue.isEmpty())
	{
		// this is the unit which can produce the currentItem
		BWAPI::Unit * producer = selectUnitOfType(currentItem.metaType.whatBuilds(), currentItem.metaType.unitType);

		// check to see if we can make it right now, meet the required resource, tech, building
		// do not check if have legal building place
		bool canMake = canMakeNow(producer, currentItem.metaType);

		// if the next item in the list is Hatchery and we can't yet make it, 
		//we want to predict the right time for worker to move
		if (currentItem.metaType.unitType == BWAPI::UnitTypes::Zerg_Hatchery && !canMake)
		{
			// construct a fake building object to make a predict moving worker
			Building b;
			b.type = currentItem.metaType.unitType;
			if (currentItem.metaType.unitType == BWAPI::UnitTypes::Zerg_Hatchery)
				b.desiredPosition = getNextHatcheryLocation();

			// set the producer as the closest worker, but do not set its job yet
			producer = WorkerManager::Instance().getBuilder(b, false);

			// predict the worker movement to that building location
			predictWorkerMovement(b);
		}

		// if we can make the current item
		if (producer && canMake)
		{
			// create it
			createMetaType(producer, currentItem.metaType);
			assignedWorkerForThisBuilding = false;
			haveLocationForThisBuilding = false;

			// and remove it from the queue

			queue.removeCurrentHighestPriorityItem();

			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (queue.canSkipItem())
		{
			// skip it
			queue.skipItem();

			// and get the next one
			currentItem = queue.getNextHighestPriorityItem();
		}
		else
		{
			// so break out
			break;
		}
	}
}

bool ProductionManager::canMakeNow(BWAPI::Unit * producer, MetaType t)
{
	bool canMake = meetsReservedResources(t);
	if (canMake)
	{
		if (t.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(producer, t.unitType);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(producer, t.techType);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(producer, t.upgradeType);
		}
		else
		{
			assert(false);
		}
	}

	return canMake;
}

bool ProductionManager::detectBuildOrderDeadlock()
{
	// if the queue is empty there is no deadlock
	if (queue.size() == 0 || BWAPI::Broodwar->self()->supplyTotal() >= 390)
	{
		return false;
	}

	// check if zerg lord is morph
	int supplyInProgress = OverlordIsBeingBuilt();

	BWAPI::UnitType nextProductionType = queue.getHighestPriorityItem().metaType.unitType;
	int supplyneed = nextProductionType.supplyRequired();
	//int supplyAvailable = std::max(0, BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed());
	int supplyAvailable = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();

	//TODO: BWAPI Lan play bug, supplyused jump to 9*2
	if (supplyAvailable + supplyInProgress - supplyneed < 2 && nextProductionType != BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() < 42 * 2 && BWAPI::Broodwar->self()->supplyTotal() > 9 * 2)
		return true;
	else if (supplyAvailable + supplyInProgress - supplyneed < 8 && nextProductionType != BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() >= 42 * 2)
		return true;
	else
		return false;
}


int ProductionManager::OverlordIsBeingBuilt()
{
	int supplyMorph = 0;
	int minRemainBuildTime = 999999;

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				if (unit->getRemainingBuildTime() < minRemainBuildTime)
					minRemainBuildTime = unit->getRemainingBuildTime();
				supplyMorph += 8;
			}
		}
	}


	// for morphing info delay several frame bug..
	if (minRemainBuildTime < 50)
	{
		nextSupplyDeadlockDetectTime = BWAPI::Broodwar->getFrameCount() + 100;
	}

	return supplyMorph;
}

// When the next item in the queue is a building, this checks to see if we should move to it
// This function is here as it needs to access production manager's reserved resources info
void ProductionManager::predictWorkerMovement(const Building & b)
{
	// get a possible building location for the building
	if (!haveLocationForThisBuilding)
	{
		predictedTilePosition = BuildingManager::Instance().getBuildingLocation(b);
	}

	if (predictedTilePosition != BWAPI::TilePositions::None)
	{
		haveLocationForThisBuilding = true;
	}
	else
	{
		return;
	}

	// draw a box where the building will be placed
	int x1 = predictedTilePosition.x() * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = predictedTilePosition.y() * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);

	// where we want the worker to walk to
	BWAPI::Position walkToPosition = BWAPI::Position(x1 + (b.type.tileWidth() / 2) * 32, y1 + (b.type.tileHeight() / 2) * 32);

	// compute how many resources we need to construct this building
	int mineralsRequired = std::max(0, b.type.mineralPrice() - getFreeMinerals());
	int gasRequired = std::max(0, b.type.gasPrice() - getFreeGas());

	// get a candidate worker to move to this location
	BWAPI::Unit * moveWorker = WorkerManager::Instance().getMoveWorker(walkToPosition);

	// Conditions under which to move the worker: 
	//		- there's a valid worker to move
	//		- we haven't yet assigned a worker to move to this location
	//		- the build position is valid
	//		- we will have the required resources by the time the worker gets there
	if (moveWorker && haveLocationForThisBuilding && !assignedWorkerForThisBuilding && (predictedTilePosition != BWAPI::TilePositions::None) &&
		WorkerManager::Instance().willHaveResources(mineralsRequired, gasRequired, moveWorker->getDistance(walkToPosition)))
	{
		// we have assigned a worker
		assignedWorkerForThisBuilding = true;

		// tell the worker manager to move this worker
		WorkerManager::Instance().setMoveWorker(mineralsRequired, gasRequired, walkToPosition);
	}
}

void ProductionManager::performCommand(BWAPI::UnitCommandType t) {

	// if it is a cancel construction, it is probably the extractor trick
	if (t == BWAPI::UnitCommandTypes::Cancel_Construction)
	{
		BWAPI::Unit * extractor = NULL;
		BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor)
			{
				extractor = unit;
			}
		}

		if (extractor)
		{
			extractor->cancelConstruction();
		}
	}
}

int ProductionManager::getFreeMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - BuildingManager::Instance().getReservedMinerals();
}

int ProductionManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - BuildingManager::Instance().getReservedGas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(MetaType type)
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getFreeMinerals()) && (type.gasPrice() <= getFreeGas());
}


BWAPI::TilePosition	ProductionManager::getNextHatcheryLocation()
{
	std::set<BWAPI::Unit*>& selfAllbase = InformationManager::Instance().getOurAllBaseUnit();
	//the third hatchery build near the choke
	if (selfAllbase.size() == 1 && InformationManager::Instance().isEarlyRush())
	{
		return InformationManager::Instance().getOurNatrualLocation();
	}
	else
	{
		return BWAPI::Broodwar->self()->getStartLocation();
	}
}


// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::createMetaType(BWAPI::Unit * producer, MetaType t)
{
	if (!producer)
	{
		return;
	}

	// if we're dealing with a not upgrade building
	if (t.isUnit() && t.unitType.isBuilding()
		&& t.unitType != BWAPI::UnitTypes::Zerg_Lair
		&& t.unitType != BWAPI::UnitTypes::Zerg_Hive
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire
		&& t.unitType != BWAPI::UnitTypes::Zerg_Sunken_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Spore_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		// send the building task to the building manager
		BWAPI::Broodwar->printf("produce building %s", t.unitType.getName().c_str());

		//predefined building order
		if (t.buildingPosition == BWAPI::TilePositions::None)
		{
			if (t.unitType == BWAPI::UnitTypes::Zerg_Hatchery)
				BuildingManager::Instance().addBuildingTask(t.unitType, getNextHatcheryLocation());
			//build defend colony near chokepoint
			else
				BuildingManager::Instance().addBuildingTask(t.unitType, BWAPI::Broodwar->self()->getStartLocation());
		}
		//dynamic add building location
		else
		{
			BuildingManager::Instance().addBuildingTask(t.unitType, t.buildingPosition);
		}


	}
	// if we're dealing with a non-building unit
	else if (t.isUnit())
	{
		// if the race is zerg, morph the unit
		if (t.unitType.getRace() == BWAPI::Races::Zerg) {
			//BWAPI::Broodwar->printf("produce unit %s", t.unitType.getName().c_str());
			bool result = producer->morph(t.unitType);
			if (!result)
			{
				BWAPI::Broodwar->printf("morph fail!!");
			}

			// if not, train the unit
		}
		else {
			producer->train(t.unitType);
		}
	}
	// if we're dealing with a tech research
	else if (t.isTech())
	{
		BWAPI::Broodwar->printf("produce research  %s", t.techType.getName().c_str());
		producer->research(t.techType);
	}
	else if (t.isUpgrade())
	{
		BWAPI::Broodwar->printf("produce upgrade  %s", t.upgradeType.getName().c_str());
		producer->upgrade(t.upgradeType);
	}
	else
	{
		// critical error check
		//		assert(false);

		//Logger::Instance().log("createMetaType error: " + t.getName() + "\n");
	}

}

// selects a unit of a given type
BWAPI::Unit * ProductionManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, bool leastTrainingTimeRemaining, BWAPI::Position closestTo) {

	// if we have none of the unit type, return NULL right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0)
	{
		return NULL;
	}

	// we only want our start base to upgrade
	if (type == BWAPI::UnitTypes::Zerg_Hive ||
		type == BWAPI::UnitTypes::Zerg_Lair ||
		type == BWAPI::UnitTypes::Zerg_Hatchery)
	{
		return InformationManager::Instance().GetOurBaseUnit();
	}

	BWAPI::Unit * unit = NULL;

	// if we are concerned about the position of the unit, that takes priority
	if (closestTo != BWAPI::Position(0, 0)) {

		double minDist(1000000);

		BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type) {

				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

		// if it is a building and we are worried about selecting the unit with the least
		// amount of training time remaining
	}
	else if (type.isBuilding() && leastTrainingTimeRemaining) {

		BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type && u->isCompleted() && !u->isResearching() &&!u->isTraining() && !u->isLifted() && !u->isUnpowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	}
	else {

		BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits())
		{
			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() && !u->isUnpowered())
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return NULL;
}

void ProductionManager::populateTypeCharMap()
{
	typeCharMap['p'] = MetaType(BWAPI::UnitTypes::Protoss_Probe);
	typeCharMap['z'] = MetaType(BWAPI::UnitTypes::Protoss_Zealot);
	typeCharMap['d'] = MetaType(BWAPI::UnitTypes::Protoss_Dragoon);
	typeCharMap['t'] = MetaType(BWAPI::UnitTypes::Protoss_Dark_Templar);
	typeCharMap['c'] = MetaType(BWAPI::UnitTypes::Protoss_Corsair);
	typeCharMap['e'] = MetaType(BWAPI::UnitTypes::Protoss_Carrier);
	typeCharMap['h'] = MetaType(BWAPI::UnitTypes::Protoss_High_Templar);
	typeCharMap['n'] = MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	typeCharMap['a'] = MetaType(BWAPI::UnitTypes::Protoss_Arbiter);
	typeCharMap['r'] = MetaType(BWAPI::UnitTypes::Protoss_Reaver);
	typeCharMap['o'] = MetaType(BWAPI::UnitTypes::Protoss_Observer);
	typeCharMap['s'] = MetaType(BWAPI::UnitTypes::Protoss_Scout);
	typeCharMap['l'] = MetaType(BWAPI::UpgradeTypes::Leg_Enhancements);
	typeCharMap['v'] = MetaType(BWAPI::UpgradeTypes::Singularity_Charge);
}

void ProductionManager::drawProductionInformation(int x, int y)
{
	// fill prod with each unit which is under construction
	std::vector<BWAPI::Unit *> prod;
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isBeingConstructed())
		{
			prod.push_back(unit);
		}
	}

	// sort it based on the time it was started
	std::sort(prod.begin(), prod.end(), CompareWhenStarted());

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Build Order Info:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + 20, "\x04UNIT NAME");

	size_t reps = prod.size() < 10 ? prod.size() : 10;

	y += 40;
	int yy = y;

	// for each unit in the queue
	for (size_t i(0); i < reps; i++) {

		std::string prefix = "\x07";

		yy += 10;

		BWAPI::UnitType t = prod[i]->getType();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, yy, "%s%s", prefix.c_str(), t.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x + 150, yy, "%s%6d", prefix.c_str(), prod[i]->getRemainingBuildTime());
	}

	queue.drawQueueInformation(x, yy + 10);
}

ProductionManager & ProductionManager::Instance() {

	static ProductionManager instance;
	return instance;
}

void ProductionManager::onGameEnd()
{

}

// when the current strategy building item complete, start new strategy and attack mode. 
bool ProductionManager::isStrategyComplete()
{
	return queue.isEmpty();
}