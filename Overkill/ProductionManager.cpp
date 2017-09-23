
#include "ProductionManager.h"
#include "AttackManager.h"


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

	setBuildOrder(StrategyManager::Instance().getOpeningBook(), true);

	nextSupplyDeadlockDetectTime = 0;

	productionState = fixBuildingOrder;

	HatcheryProductionCheckTime = 0;
	extractorProductionCheckTime = 0;

	spireTrigger = true;
	thirdBaseTrigger = true;
	secondExtractorTrigger = true;
	firstMutaTrigger = true;
	lairTrigger = true;
	sunkenBuilderTrigger = true;
	sunkenCanBuild = false;

	droneProductionSpeed = 120;
	extractorBuildSpeed = 0;

	sunkenWorkerTrigger = true;

	nextStrategyCheckTime = 0;

	curStrategyAction = "";

	curBuildingTarget = MetaType();

	maxOneSecondProduction = 6;
}


void ProductionManager::setBuildOrder(const std::vector<MetaType> & buildOrder, bool isBlock)
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
			queue.queueAsLowestPriority(buildOrder[i], isBlock);
	}
}


void ProductionManager::buildingCallback(BWAPI::Unit curBuildingUnit, std::vector<MetaType> buildingOrders)
{
	if (buildingOrders.size() == 0)
		return;

	MetaType targetBuilding = buildingOrders[0];
	buildingOrders.erase(buildingOrders.begin());

	const std::function<void(BWAPI::Game*)> buildingAction = [=](BWAPI::Game* g)->void
	{
		std::string curBuildingaction = StrategyManager::Instance().getCurrentBuildingAction();
		ProductionManager::Instance().addItemInQueue(targetBuilding, true, buildingOrders);
	};

	const std::function<bool(BWAPI::Game*)> buildingCondition = [=](BWAPI::Game* g)->bool
	{
 		if (curBuildingUnit->isCompleted())
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(buildingAction, buildingCondition, 1, 24);
}


void ProductionManager::triggerBuildingOrder(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, std::string unitSourceBuildingAction)
{
	std::vector<MetaType> buildingOrder(1, MetaType(buildingType, buildingLocation, unitSourceBuildingAction));
	std::vector<BWAPI::UnitType> candidateType(1, buildingType);
	bool needGas = false;
	while (true)
	{
		std::vector<BWAPI::UnitType> tmp;
		for (auto c : candidateType)
		{
			std::map<BWAPI::UnitType, int> requireUnits = c.requiredUnits();
			for (auto u : requireUnits)
			{
				if (u.first.isBuilding() && BWAPI::Broodwar->self()->allUnitCount(u.first) == 0)
				{
					buildingOrder.push_back(MetaType(u.first, buildingLocation, unitSourceBuildingAction));
					tmp.push_back(u.first);
					if (u.first.gasPrice() > 0)
						needGas = true;
				}
			}
		}

		if (tmp.size() == 0)
		{
			break;
		}
		else
		{
			candidateType = tmp;
		}
	}

	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Extractor) == 0 &&
		BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Extractor) == 0 && needGas)
	{
		buildingOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self()->getStartLocation(), unitSourceBuildingAction));
	}

	std::reverse(buildingOrder.begin(), buildingOrder.end());
	//remove the redundant item
	for (std::vector<MetaType>::iterator it = buildingOrder.begin(); it != buildingOrder.end(); it++)
	{
		std::vector<MetaType>::iterator nextIt = std::next(it, 1);
		BWAPI::UnitType curType = it->unitType;
		buildingOrder.erase(std::remove_if(nextIt, buildingOrder.end(), [=](MetaType m)->bool
		{
			if (m.unitType == curType)
				return true;
			else
				return false;
		}), buildingOrder.end());
	}

	MetaType buildingTarget = buildingOrder.front();
	buildingOrder.erase(buildingOrder.begin());

	queue.queueAsHighestPriority(buildingTarget, true, buildingOrder);
}


void ProductionManager::triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count, bool isBlocking, bool needBuildWorker)
{
	std::vector<MetaType> buildingOrder;
	if (buildingType == BWAPI::UnitTypes::Zerg_Sunken_Colony || buildingType == BWAPI::UnitTypes::Zerg_Spore_Colony)
	{
		buildingOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, buildingLocation));
		buildingOrder.push_back(MetaType(buildingType, buildingLocation));
	}
	else
	{
		buildingOrder.push_back(MetaType(buildingType, buildingLocation));
	}
	
	MetaType buildingTarget = buildingOrder.front();
	buildingOrder.erase(buildingOrder.begin());

	for (int i = 0; i < count; i++)
	{
		queue.queueAsHighestPriority(buildingTarget, isBlocking, buildingOrder);
	}
}


void ProductionManager::triggerUnit(BWAPI::UnitType unitType, int unitCount, bool isHighPriority, bool isBlocking)
{
	for (int i = 0; i < unitCount; i++)
	{
		if (isHighPriority)
			queue.queueAsHighestPriority(MetaType(unitType), isBlocking);
		else
			queue.queueAsLowestPriority(MetaType(unitType), isBlocking);
	}

	//if there is too many zerglings waiting to produce and we do not have enough larvas, building sunken instead
	if (BWAPI::Broodwar->getFrameCount() < 12000)
	{
		int waitingBuildZergling = 0;
		for (int i = queue.size() - 1; i >= 0; i--)
		{
			if (queue[i].metaType.unitType == BWAPI::UnitTypes::Zerg_Zergling)
			{
				waitingBuildZergling++;
			}
			else
				break;
		}

		if (waitingBuildZergling >= 3 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva) <= 1)
		{
			for (int i = 0; i < (waitingBuildZergling / 3) * 3; i++)
			{
				queue.removeHighestPriorityItem();
			}
			triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition(), waitingBuildZergling / 3);
			InformationManager::Instance().addWaitBuildSunkun(waitingBuildZergling / 3);
		}
	}
}


void ProductionManager::RushDefend(BWAPI::UnitType defendBuilding, int buildingCount, BWAPI::UnitType defendUnit, int unitCount)
{
	BWAPI::Position chokePosition = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
	BWAPI::Position basePositon = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	double2 direc = chokePosition - basePositon;
	double2 direcNormal = direc / direc.len();
	int targetx = (basePositon.x + int(direcNormal.x * 32 * 10)) / 32;
	int targety = (basePositon.y + int(direcNormal.y * 32 * 10)) / 32;

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

	/*
	std::vector<RecoverBuildingInfo> recoverSunker;
	std::vector<RecoverBuildingInfo> recoverSporer;

	BOOST_FOREACH(RecoverBuildingInfo u, recoverBuilding)
	{
		if (u.buildingType == BWAPI::UnitTypes::Zerg_Sunken_Colony)
		{
			recoverSunker.push_back(u);
		}
		else if (u.buildingType == BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			recoverSporer.push_back(u);
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

	for (int i = 0; i < int(recoverSporer.size()) / 2; i++)
	{
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Spore_Colony), false);
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, recoverSporer[i].buildingPosition), false);
	}

	recoverDroneCount += recoverBuilding.size();

	//reproduce worker first
	for (int i = 0; i < recoverDroneCount; i++)
	{
		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Drone), false);
	}
	*/
	recoverBuilding.clear();
	recoverDroneCount = 0;
	
}

void ProductionManager::clearDefendVector()
{
	recoverBuilding.clear();
	recoverDroneCount = 0;
}


std::vector<BWAPI::UnitType> ProductionManager::getWaitingProudctUnit()
{
	std::vector<BWAPI::UnitType> topWaitingProduct;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	for (int i = queue.size() - 1; i >= 0; i--)
	{
		if (queue[i].metaType.isUnit() && !queue[i].metaType.unitType.isWorker() && queue[i].metaType.unitType.canAttack() && !queue[i].metaType.unitType.isBuilding())
		{
			const std::map<BWAPI::UnitType, int>& unitRequiredBuilding = queue[i].metaType.unitType.requiredUnits();
			bool preBuildingComplete = true;
			for (std::map<BWAPI::UnitType, int>::const_iterator it = unitRequiredBuilding.begin(); it != unitRequiredBuilding.end(); it++)
			{
				if (it->first != BWAPI::UnitTypes::Zerg_Larva && (ourBuildings[it->first].size() == 0 || (ourBuildings[it->first].size() > 0 && !(*ourBuildings[it->first].begin())->isCompleted())))
				{
					preBuildingComplete = false;
					break;
				}
			}
			if (preBuildingComplete)
				topWaitingProduct.push_back(queue[i].metaType.unitType);
		}
	}
	return topWaitingProduct;
}

void ProductionManager::buildExtractorTrick()
{
	//extractor trick
	BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType() == BWAPI::UnitTypes::Zerg_Extractor && u->getRemainingBuildTime() / double(BWAPI::UnitTypes::Zerg_Extractor.buildTime()) < 0.7
			&& BWAPI::Broodwar->self()->supplyUsed() == 9 * 2)
		{
			u->cancelMorph();
		}
	}
}

void ProductionManager::update()
{
	//// detect if we need to morph overlord first.
	//note check too quick may lead to build more supply

	
	// do not trigger for early game
	if (BWAPI::Broodwar->getFrameCount() > nextSupplyDeadlockDetectTime && detectNeedMorphOverLord())
	{
		//BWAPI::Broodwar->printf("Supply deadlock detected, building OverLord!");
		queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
	}
	

	// check the queue for stuff we can build
	manageBuildOrderQueue();

	openingStrategy opening = StrategyManager::Instance().getCurrentopeningStrategy();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuilding = InformationManager::Instance().getOurAllBuildingUnit();

	// production FSM
	switch (productionState)
	{
	case ProductionManager::fixBuildingOrder:
	{
		//if (AttackManager::Instance().isUnderAttack())
		//{
			//productionState = goalOriented;
		//}

		switch (opening)
		{
		case TwelveHatchMuta:
		{
			buildExtractorTrick();

			BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
			{
				if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
					&& u->isCompleted() && sunkenBuilderTrigger)
				{
					sunkenBuilderTrigger = false;
					sunkenCanBuild = true;
					sunkenBuilderTime = BWAPI::Broodwar->getFrameCount() + 2 * 25;
				}

				if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
					&& u->getRemainingBuildTime() / double(BWAPI::UnitTypes::Zerg_Hatchery.buildTime()) < 0.2 && sunkenWorkerTrigger)
				{
					sunkenWorkerTrigger = false;

					WorkerManager::Instance().setMoveWorker(75, 0, BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
					WorkerManager::Instance().setMoveWorker(75, 0, BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
					//InformationManager::Instance().setDefend(false);
				}
			}
			if (BWAPI::Broodwar->getFrameCount() > sunkenBuilderTime && sunkenCanBuild)
			{
				sunkenCanBuild = false;
				int buildCount = 0;
				buildCount = 1;
				triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition(), 2, true, true);
				InformationManager::Instance().setDefend(true);
			}

			int waitMutaliskCount = 0;
			for (int i = 0; i < int(queue.size()); i++)
			{
				if (queue[i].metaType.unitType == BWAPI::UnitTypes::Zerg_Mutalisk)
					waitMutaliskCount++;
			}

			if (waitMutaliskCount == 0)
			{
				AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
				InformationManager::Instance().setDefend(false);
				productionState = goalOriented;
			}
		}
			break;
		case NinePoolling:
		{
			buildExtractorTrick();

			if (sunkenBuilderTrigger && ourBuilding[BWAPI::UnitTypes::Zerg_Spawning_Pool].size() > 0 && (*ourBuilding[BWAPI::UnitTypes::Zerg_Spawning_Pool].begin())->isCompleted())
			{
				sunkenBuilderTrigger = false;
				int buildCount = 0;
				buildCount = 1;
				triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition(), 1, true, true);

				const std::function<void(BWAPI::Game*)> sunkenAction = [=](BWAPI::Game* g)->void
				{
					BWAPI::Broodwar->drawTextScreen(150, 10, "fix building order finish");
					productionState = goalOriented;
				};
				const std::function<bool(BWAPI::Game*)> sunkenCondition = [=](BWAPI::Game* g)->bool
				{
					if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony) == buildCount)
						return true;
					else
						return false;
				};
				BWAPI::Broodwar->registerEvent(sunkenAction, sunkenCondition, 1, 48);
			}
			
		}
			break;
		case TenHatchMuta:
		{
			buildExtractorTrick();

			//build sunken at natural first
			BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
			{
				if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
					&& u->isCompleted() && sunkenBuilderTrigger)
				{
					sunkenBuilderTrigger = false;
					sunkenCanBuild = true;
					sunkenBuilderTime = BWAPI::Broodwar->getFrameCount() + 2 * 25;

					//WorkerManager::Instance().balanceWorkerOnDepotComplete(u);
				}

				if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())
					&& u->getRemainingBuildTime() / double(BWAPI::UnitTypes::Zerg_Hatchery.buildTime()) < 0.2 && sunkenWorkerTrigger)
				{
					sunkenWorkerTrigger = false;
					
					WorkerManager::Instance().setMoveWorker(75, 0, BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
					WorkerManager::Instance().setMoveWorker(75, 0, BWAPI::Position(InformationManager::Instance().getOurNatrualLocation()));
					//InformationManager::Instance().setDefend(false);
				}
			}
			if (BWAPI::Broodwar->getFrameCount() > sunkenBuilderTime && sunkenCanBuild)
			{
				sunkenCanBuild = false;
				int buildCount = 0;
				buildCount = 2;
				triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition(), 2, true, true);
				//triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition(), 1, true, false);
				//InformationManager::Instance().addWaitBuildSunkun(2);

				int nextCheckTime = BWAPI::Broodwar->getFrameCount() + 24 * 5;

				const std::function<void(BWAPI::Game*)> sunkenAction = [=](BWAPI::Game* g)->void
				{
					//InformationManager::Instance().setDefend(true);

					BWAPI::Broodwar->drawTextScreen(150, 10, "fix building order finish");
					productionState = goalOriented;
				};
				const std::function<bool(BWAPI::Game*)> sunkenCondition = [=](BWAPI::Game* g)->bool
				{
					if (BWAPI::Broodwar->getFrameCount() > nextCheckTime)
						return true;
					else
						return false;
				};
				BWAPI::Broodwar->registerEvent(sunkenAction, sunkenCondition, 1, 48);

			}

		}
			break;
		default:
			break;
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
		onExtractorProduction();
	}
	break;
	default:
		break;
	}
}


void ProductionManager::onGoalProduction()
{
	if (queue.isEmpty())
	{
		//continue previous unit production
		if (BWAPI::Broodwar->getFrameCount() < nextStrategyCheckTime)
		{
			if (curBuildingTarget.isUnit() && !curBuildingTarget.isBuilding() &&
				curOneSecondProduction < maxOneSecondProduction)
			{
				curOneSecondProduction += 1;
				queue.queueAsHighestPriority(curBuildingTarget, true);
			}
		}
		//for every second get the command
		else
		{
			curOneSecondProduction = 0;
			curBuildingTarget = StrategyManager::Instance().getTargetUnit(BWAPI::UnitTypes::None);
			if (curBuildingTarget.type != MetaType::Default)
			{
				queue.queueAsHighestPriority(curBuildingTarget, true);
			}

			nextStrategyCheckTime = BWAPI::Broodwar->getFrameCount() + 24 * 2;
		}
	}
}


void ProductionManager::onDroneProduction()
{
	if (productionState != goalOriented)
		return;

	int waitDroneCount = 0;
	for (int i = 0; i < int(queue.size()); i++)
	{
		if (queue[i].metaType.unitType == BWAPI::UnitTypes::Zerg_Drone)
			waitDroneCount++;
	}

	int maxWaitingBuildCount = 2;
	if (BWAPI::Broodwar->getFrameCount() < 7000)
	{
		maxWaitingBuildCount = 1;
	}

	bool isWorkerFull = isDepotNearlyFull();
	int expandCount = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self()).size() - 1;

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& allUnits = InformationManager::Instance().getOurAllBattleUnit();
	//keep the drone production interval the same as one base's larva production rate
	if (isWorkerFull && waitDroneCount < maxWaitingBuildCount && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) <= 70
		&& BWAPI::Broodwar->getFrameCount() % (int(droneProductionSpeed * 1.5) / (expandCount + 1)) == 0
		&& BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva) > 0)//BWAPI::Broodwar->getFrameCount() % 350 == 0)
	{
		
		int highPriority = queue.getHighestPriorityValue();
		int lowPriority = queue.getLowestPriorityValue();
		int itemPriority = lowPriority + int((highPriority - lowPriority) * 0.8);
		queue.queueItem(MetaType(BWAPI::UnitTypes::Zerg_Drone), true, itemPriority);
		
		//queue.queueAsLowestPriority(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	}
}

void ProductionManager::onExtractorProduction()
{
	bool hasGasPosition = false;
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	BOOST_FOREACH(BWAPI::Unit geyser, BWAPI::Broodwar->getGeysers())
	{
		//not my region
		if (myRegions.find(BWTA::getRegion(geyser->getPosition())) == myRegions.end())
		{
			continue;
		}

		hasGasPosition = true;
		break;
	}

	/*
	//if all the occupied region's gas has built and still short for gas, do another expand for gas
	if (!hasGasPosition && getFreeGas() <= 100 && getFreeMinerals() >= 2000)
	{
		StrategyManager::Instance().baseExpand();
		HatcheryProductionCheckTime = BWAPI::Broodwar->getFrameCount() + BWAPI::UnitTypes::Zerg_Hatchery.buildTime();
	}
	*/

	//if has extractor building location && we do not have much gas && (have enough worker || mineral is too many), build one
	if (BWAPI::Broodwar->getFrameCount() > extractorProductionCheckTime && hasGasPosition && getFreeGas() <= 500)
	{
		double factor = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) == 0 ? 0.7 : 0.8;
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) >= int((BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) + factor) * 15)
			|| getFreeMinerals() >= 1000)
		{
			//extractor location is determined in building manager 
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self()->getStartLocation(), 1);
			extractorProductionCheckTime = BWAPI::Broodwar->getFrameCount() + BWAPI::UnitTypes::Zerg_Extractor.buildTime() + 30 * 25;
		}
	}
}


void ProductionManager::onHatcheryProduction()
{
	if (productionState != goalOriented)
		return;

	for (int i = queue.size() - 1; i >= 0; i--)
	{
		if (queue[i].metaType.unitType == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			return;
		}
	}

	int larvaCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva);

	//check if base is full of worker
	std::set<BWTA::Region *> & ourRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();
	//if we do not have much idle larva and has enough mineral, build a hatchery 
	if (BWAPI::Broodwar->getFrameCount() > HatcheryProductionCheckTime &&
		((larvaCount <= 3 && getFreeMinerals() >= 500 && selfAllBuilding[BWAPI::UnitTypes::Zerg_Hatchery].size() <= 15))) //|| !hasIdleDepot))
	{
		HatcheryProductionCheckTime = BWAPI::Broodwar->getFrameCount() + 25 * 40;
		
		int workingDepots = ourRegion.size();

		/*
		if (!hasIdleDepot || (workingDepots <= 5 && (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) >= 15 * workingDepots)))
		{
			StrategyManager::Instance().baseExpand();
		}
		else
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self()->getStartLocation()), true);
			*/

		queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self()->getStartLocation()), true);
	}
}


bool ProductionManager::isDepotNearlyFull()
{
	//check if base is full of worker
	std::set<BWTA::Region *> & ourRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	std::map<BWTA::Region*, std::pair<int, int>> regionMineralWorker;
	BOOST_FOREACH(BWAPI::Unit mineral, BWAPI::Broodwar->getMinerals())
	{
		if (ourRegion.find(BWTA::getRegion(mineral->getPosition())) != ourRegion.end())
		{
			regionMineralWorker[BWTA::getRegion(mineral->getPosition())].first += 1;
		}
	}
	BOOST_FOREACH(BWAPI::Unit worker, BWAPI::Broodwar->self()->getUnits())
	{
		if (worker->getType().isWorker() && ourRegion.find(BWTA::getRegion(worker->getPosition())) != ourRegion.end())
		{
			regionMineralWorker[BWTA::getRegion(worker->getPosition())].second += 1;
		}
	}

	//if depot is near full of worker, do expand
	//worker can be auto balance between depots
	bool hasIdleDepot = false;
	for (std::map<BWTA::Region*, std::pair<int, int>>::iterator it = regionMineralWorker.begin(); it != regionMineralWorker.end(); it++)
	{
		if (it->second.second <= it->second.first * 2.3)
		{
			hasIdleDepot = true;
		}
	}

	return hasIdleDepot;
}


// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (AttackManager::Instance().isUnderAttack())
	{
		if (unit->getType().isWorker())
		{
			recoverDroneCount++;
		}
		if (unit->getType().isBuilding()) //&& unit->getType() != BWAPI::UnitTypes::Zerg_Sunken_Colony
			//&& unit->getType() != BWAPI::UnitTypes::Zerg_Creep_Colony && unit->getType() != BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			recoverBuilding.push_back(RecoverBuildingInfo(unit->getType(), unit->getTilePosition()));
		}

		openingStrategy opening = StrategyManager::Instance().getCurrentopeningStrategy();

		//for early defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling && BWAPI::Broodwar->getFrameCount() <= 12000)
		{
			std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();
			if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].begin())->isCompleted())
				return;

			if (opening != NinePoolling)
			{
				zerglingsKilledCount++;
				if (zerglingsKilledCount >= 4)
				{
					triggerUnit(BWAPI::UnitTypes::Zerg_Zergling, 1);
					zerglingsKilledCount = 0;
				}
			}
			else
			{
				zerglingsKilledCount++;
				if (zerglingsKilledCount >= 2)
				{
					triggerUnit(BWAPI::UnitTypes::Zerg_Zergling, 1);
					zerglingsKilledCount = 0;
				}
			}
		}
	}

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() <= 20 * 2)
	{
		BWAPI::Broodwar->printf("trigger overlord!!!!!!!!!!!!!!!!!!!");
		triggerUnit(BWAPI::UnitTypes::Zerg_Overlord, 1);
	}
	
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
	{
		BWAPI::Broodwar->printf("remove sunken or spornse!!!!!!!!!!!!!!!!!!!");
		int removeCount = queue.removeUnitType(BWAPI::UnitTypes::Zerg_Sunken_Colony);
		if (removeCount == 0)
		{
			int removeCount = queue.removeUnitType(BWAPI::UnitTypes::Zerg_Spore_Colony);
		}
	}
	// keep the total mutalisk's number
	
	/*
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) < 12)
			triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 1);
		else
			return;
	}*/
		
}


int ProductionManager::getTopProductionNeed()
{
	BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();
	if (currentItem.metaType.mineralPrice() > getFreeMinerals())
		return 1;
	else
		return 0;
}

bool ProductionManager::IsUpgradeInQueue(BWAPI::UpgradeType up)
{
	return queue.isUpgradeInQueue(up);
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

	//the waiting to build item's builder or required tech is not exist
	if (productionState == goalOriented && ((currentItem.metaType.whatBuilds() != BWAPI::UnitTypes::Zerg_Larva
		&& BWAPI::Broodwar->self()->completedUnitCount(currentItem.metaType.whatBuilds()) == 0)
		|| (currentItem.metaType.isUnit() && checkProductionDeadLock(currentItem.metaType.unitType))))
	{
		BWAPI::Broodwar->printf("checkProductionDeadLock !!!!!!");
		queue.removeHighestPriorityItem();
		return;
	}
	
	// while there is still something left in the queue, we can skip something and continue build
	while (!queue.isEmpty())
	{
		// this is the unit which can produce the currentItem
		BWAPI::Unit producer = selectUnitOfType(currentItem.metaType.whatBuilds(), currentItem.metaType.unitType);


		// check to see if we can make it right now, meet the required resource, tech, building
		// do not check if have legal building place
		bool canMake = canMakeNow(producer, currentItem.metaType);

		//if the next item in the list is Hatchery and we can't yet make it, 
		//and we in predefined building order
		if (currentItem.metaType.unitType == BWAPI::UnitTypes::Zerg_Hatchery && !canMake && currentItem.metaType.buildingPosition == BWAPI::TilePositions::None)
		{
			// construct a fake building object to make a predict moving worker
			Building b;
			b.type = currentItem.metaType.unitType;
			b.desiredPosition = getNextHatcheryLocation();

			// set the producer as the closest worker, but do not set its job yet
			//producer = WorkerManager::Instance().getBuilder(b, false);

			// predict the worker movement to that building location
			predictWorkerMovement(b);
		}

		// if we can make the current item
		if (producer && canMake)
		{
			// create it
			createMetaType(producer, currentItem);
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

bool ProductionManager::canMakeNow(BWAPI::Unit producer, MetaType t)
{
	bool canMake = meetsReservedResources(t);
	if (canMake)
	{
		if (t.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(t.unitType, producer);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(t.techType, producer);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(t.upgradeType, producer);
		}
		else
		{
			assert(false);
		}
	}

	return canMake;
}

bool ProductionManager::detectNeedMorphOverLord()
{
	// if the queue is empty there is no deadlock
	if (queue.size() == 0 || BWAPI::Broodwar->self()->supplyTotal() >= 400)
	{
		return false;
	}

	// check if zerg lord is morph
	int supplyInProgress = OverlordIsBeingBuilt();

	BWAPI::UnitType nextProductionType = queue.getHighestPriorityItem().metaType.unitType;
	int supplyneed = nextProductionType.supplyRequired();
	//int supplyAvailable = std::max(0, BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed());
	int supplyAvailable = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();


	if (supplyAvailable + supplyInProgress - supplyneed <= 1 && nextProductionType != BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() < 42 * 2 && BWAPI::Broodwar->self()->supplyTotal() > 10 * 2)
		return true;
	else if (BWAPI::Broodwar->self()->supplyTotal() != 200 * 2 && supplyAvailable + supplyInProgress - supplyneed <= 8 && nextProductionType != BWAPI::UnitTypes::Zerg_Overlord && BWAPI::Broodwar->self()->supplyTotal() >= 42 * 2)
		return true;
	else
		return false;
}


int ProductionManager::OverlordIsBeingBuilt()
{
	int supplyMorph = 0;
	int minRemainBuildTime = 999999;

	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
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
void ProductionManager::predictWorkerMovement( Building & b)
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
	int x1 = predictedTilePosition.x * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = predictedTilePosition.y * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);

	// where we want the worker to walk to
	BWAPI::Position walkToPosition = BWAPI::Position(x1 + (b.type.tileWidth() / 2) * 32, y1 + (b.type.tileHeight() / 2) * 32);

	// compute how many resources we need to construct this building
	int mineralsRequired = b.type.mineralPrice() - getFreeMinerals() > 0 ? b.type.mineralPrice() - getFreeMinerals() : 0;
	int gasRequired = b.type.gasPrice() - getFreeGas() > 0 ? b.type.gasPrice() - getFreeGas() : 0;

	// get a candidate worker to move to this location
	BWAPI::Unit moveWorker = WorkerManager::Instance().getMoveWorker(walkToPosition);

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
		BWAPI::Unit extractor = NULL;
		BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->self()->getUnits())
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
	std::set<BWAPI::Unit>& selfAllbase = InformationManager::Instance().getOurAllBaseUnit();
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
void ProductionManager::createMetaType(BWAPI::Unit producer, BuildOrderItem<PRIORITY_TYPE> currentItem)
{
	if (!producer)
	{
		return;
	}

	MetaType t = currentItem.metaType;

	// if we're dealing with a not upgrade building
	if (t.isUnit() && t.unitType.isBuilding()
		&& t.unitType != BWAPI::UnitTypes::Zerg_Lair
		&& t.unitType != BWAPI::UnitTypes::Zerg_Hive
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire
		&& t.unitType != BWAPI::UnitTypes::Zerg_Sunken_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Spore_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire)
	{

		BWAPI::Broodwar->printf("proudction manger: %s\n", t.unitType.c_str());


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
			BuildingManager::Instance().addBuildingTask(t.unitType, t.buildingPosition, currentItem.waitingBuildType);
		}
	}
	// if we're dealing with a non-building unit
	else if (t.isUnit())
	{
		// if the race is zerg, morph the unit
		if (t.unitType.getRace() == BWAPI::Races::Zerg) {
			//BWAPI::Broodwar->printf("produce unit %s", t.unitType.getName().c_str());
			bool result = producer->morph(t.unitType);

			
			if (currentItem.waitingBuildType.size() > 0)
			{
				const std::function<void(BWAPI::Game*)> triggerAction = [=](BWAPI::Game* g)->void
				{
					buildingCallback(producer, currentItem.waitingBuildType);
				};
				const std::function<bool(BWAPI::Game*)> triggerCondition = [=](BWAPI::Game* g)->bool
				{
					if (producer->isMorphing())
						return true;
					else
						return false;
				};
				BWAPI::Broodwar->registerEvent(triggerAction, triggerCondition, 1, 25);
			}
			

			//for bug: do not have enough resource to produce overlord
			if (BWAPI::Broodwar->self()->supplyTotal() < 15 * 2 && t.unitType == BWAPI::UnitTypes::Zerg_Overlord)
			{
				int checkTime = BWAPI::Broodwar->getFrameCount() + 24 * 20;
				const std::function<void(BWAPI::Game*)> triggerAction = [=](BWAPI::Game* g)->void
				{
					if (producer->exists() && !producer->isMorphing())
					{
						BWAPI::Broodwar->printf("morph again!!!");
						producer->morph(BWAPI::UnitTypes::Zerg_Overlord);
					}
				};
				const std::function<bool(BWAPI::Game*)> triggerCondition = [=](BWAPI::Game* g)->bool
				{
					int cur = BWAPI::Broodwar->getFrameCount();
					return cur > checkTime;
				};
				BWAPI::Broodwar->registerEvent(triggerAction, triggerCondition, 1, 25);
			}
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


bool ProductionManager::checkProductionDeadLock(BWAPI::UnitType targetType)
{
	if (!StrategyManager::Instance().isMineRequireMeet(targetType.mineralPrice()) || !StrategyManager::Instance().isGasRequireMeet(targetType.gasPrice()))
	{
		return true;
	}

	//requiredUnits api include the builder and the required tech building 
	std::map<BWAPI::UnitType, int> requireUnits = targetType.requiredUnits();
	for (auto u : requireUnits)
	{
		if (u.first != BWAPI::UnitTypes::Zerg_Larva && BWAPI::Broodwar->self()->allUnitCount(u.first) == 0)
		{
			if (u.first == BWAPI::UnitTypes::Zerg_Spire && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
			{
				continue;
			}
			if (u.first == BWAPI::UnitTypes::Zerg_Lair && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
			{
				continue;
			}
			return true;
		}
	}
	int supplyRequired = targetType.supplyRequired();
	if (targetType == BWAPI::UnitTypes::Zerg_Zergling || targetType == BWAPI::UnitTypes::Zerg_Scourge)
	{
		supplyRequired = supplyRequired * 2;
	}

	if (BWAPI::Broodwar->self()->supplyUsed() + supplyRequired > 200 * 2)
	{
		return true;
	}

	return false;
}

// selects a unit of a given type
BWAPI::Unit ProductionManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, bool leastTrainingTimeRemaining, BWAPI::Position closestTo) {

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

	BWAPI::Unit unit = NULL;

	// if we are concerned about the position of the unit, that takes priority
	if (closestTo != BWAPI::Position(0, 0)) {

		double minDist(1000000);

		BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits()) {

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

		BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type && u->isCompleted() && !u->isResearching()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	}
	else {

		BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
		{
			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0)
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
	std::vector<BWAPI::Unit> prod;
	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->self()->getUnits())
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