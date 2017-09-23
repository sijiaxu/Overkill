#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "BuildingManager.h"
#include "StrategyManager.h"
#include "InformationManager.h"


typedef unsigned char Action;

class CompareWhenStarted
{

public:

	CompareWhenStarted() {}

	// the sorting operator
	bool operator() (BWAPI::Unit u1, BWAPI::Unit u2)
	{
		int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
		int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
		return startedU1 > startedU2;
	}
};


struct RecoverBuildingInfo
{
	BWAPI::UnitType buildingType;
	BWAPI::TilePosition buildingPosition;

	RecoverBuildingInfo(BWAPI::UnitType bt, BWAPI::TilePosition bp)
	{
		buildingType = bt;
		buildingPosition = bp;
	}
};

class ProductionManager
{
	ProductionManager();

	enum ProductionState {fixBuildingOrder, goalOriented};

	ProductionState				productionState;

	int							HatcheryProductionCheckTime;
	int							extractorProductionCheckTime;

//	BuildLearner				buildLearner;
	bool						initialBuildSet;

	std::map<char, MetaType>	typeCharMap;
	std::vector< std::pair<MetaType, UnitCountType> > searchGoal;

	bool						assignedWorkerForThisBuilding;
	bool						haveLocationForThisBuilding;
	int							reservedMinerals, reservedGas;
	bool						enemyCloakedDetected;
	bool						rushDetected;

	BWAPI::TilePosition			predictedTilePosition;
	BWAPI::Unit				selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, bool leastTrainingTimeRemaining = true, BWAPI::Position closestTo = BWAPI::Position(0, 0));
	BuildOrderQueue				queue;

	int							recoverDroneCount;
	std::vector<RecoverBuildingInfo> recoverBuilding;

	bool						checkProductionDeadLock(BWAPI::UnitType targetType);
	BWAPI::UnitType				getProducer(MetaType t);

	bool						contains(UnitVector & units, BWAPI::Unit unit);
	void						populateTypeCharMap();
	bool						hasResources(BWAPI::UnitType type);
	bool						canMake(BWAPI::UnitType type);
	bool						hasNumCompletedUnitType(BWAPI::UnitType type, int num);
	bool						meetsReservedResources(MetaType type);
	
	void						createMetaType(BWAPI::Unit producer, BuildOrderItem<PRIORITY_TYPE> currentItem);
	void						manageBuildOrderQueue();
	void						performCommand(BWAPI::UnitCommandType t);
	bool						canMakeNow(BWAPI::Unit producer, MetaType t);
	void						predictWorkerMovement( Building & b);

	bool						detectNeedMorphOverLord();

	int							getFreeMinerals();
	int							getFreeGas();
	int							OverlordIsBeingBuilt();
	int							nextSupplyDeadlockDetectTime;

	BWAPI::TilePosition			getNextHatcheryLocation();
	BWAPI::TilePosition			getNextColonyLocation();

	void						onHatcheryProduction();
	void						onExtractorProduction();
	void						onDroneProduction();
	void						onGoalProduction();

	bool						lairTrigger;
	bool						spireTrigger;
	bool						thirdBaseTrigger;
	bool						secondExtractorTrigger;
	bool						firstMutaTrigger;
	bool						sunkenBuilderTrigger;
	bool						sunkenWorkerTrigger;
	bool						sunkenCanBuild;
	int							sunkenBuilderTime;

	int							zerglingsKilledCount;

	void						buildExtractorTrick();


	int							droneProductionSpeed;
	int							extractorBuildSpeed;

	int							nextStrategyCheckTime;
	std::string					curStrategyAction;

	MetaType					curBuildingTarget;
	int							maxOneSecondProduction;
	int							curOneSecondProduction;

	bool						isDepotNearlyFull();



public:

	static ProductionManager &	Instance();

	void						drawQueueInformation(std::map<BWAPI::UnitType, int> & numUnits, int x, int y, int index);
	void						update();

	void						onGameEnd();
	void						onUnitMorph(BWAPI::Unit unit);
	void						onUnitDestroy(BWAPI::Unit unit);
	
	void						drawProductionInformation(int x, int y);

	bool						isStrategyComplete();
	int							getTopProductionNeed();
	std::vector<BWAPI::UnitType>	getWaitingProudctUnit();

	void						RushDefend(BWAPI::UnitType defendBuilding, int buildingCount, BWAPI::UnitType defendUnit, int unitCount);
	void						defendLostRecover();
	void						clearDefendVector();

	void						buildingCallback(BWAPI::Unit curBuildingUnit, std::vector<MetaType> buildingOrders);
	void						addItemInQueue(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType) { queue.queueAsHighestPriority(m, blocking, waitingBuildType); }

	void						triggerBuildingOrder(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, std::string unitSourceBuildingAction);
	void						triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count, bool isBlocking = true, bool needBuildWorker = true);
	void						triggerUnit(BWAPI::UnitType unitType, int unitCount, bool isHighPriority = true, bool isBlocking = false);
	void						triggerUpgrade(BWAPI::UpgradeType upgrade, bool isBlocking = false) { queue.queueAsHighestPriority(MetaType(upgrade), isBlocking); }
	void						triggerTech(BWAPI::TechType tech, bool isBlocking = true) { queue.queueAsHighestPriority(MetaType(tech), isBlocking); }

	void						clearCurrentQueue(){ queue.clearAllUnit(); }
	void						setBuildOrder(const std::vector<MetaType> & buildOrder, bool isBlock);

	void						setDroneProductionSpeed(int productionSpeed) { droneProductionSpeed = productionSpeed; }
	void						setExtractorBuildSpeed(int buildSpeed) { extractorBuildSpeed = buildSpeed; }
	bool						IsUpgradeInQueue(BWAPI::UpgradeType up);
	int							getProductionStage() { return int(productionState); }
};
