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
	bool operator() (BWAPI::Unit * u1, BWAPI::Unit * u2)
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
	BWAPI::Unit *				selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, bool leastTrainingTimeRemaining = true, BWAPI::Position closestTo = BWAPI::Position(0, 0));
	BuildOrderQueue				queue;

	int							recoverDroneCount;
	std::vector<RecoverBuildingInfo> recoverBuilding;

	BWAPI::UnitType				getProducer(MetaType t);

	bool						contains(UnitVector & units, BWAPI::Unit * unit);
	void						populateTypeCharMap();
	bool						hasResources(BWAPI::UnitType type);
	bool						canMake(BWAPI::UnitType type);
	bool						hasNumCompletedUnitType(BWAPI::UnitType type, int num);
	bool						meetsReservedResources(MetaType type);
	
	void						createMetaType(BWAPI::Unit * producer, MetaType type);
	void						manageBuildOrderQueue();
	void						performCommand(BWAPI::UnitCommandType t);
	bool						canMakeNow(BWAPI::Unit * producer, MetaType t);
	void						predictWorkerMovement(const Building & b);

	bool						detectBuildOrderDeadlock();

	int							getFreeMinerals();
	int							getFreeGas();
	int							OverlordIsBeingBuilt();
	int							nextSupplyDeadlockDetectTime;

	BWAPI::TilePosition			getNextHatcheryLocation();
	BWAPI::TilePosition			getNextColonyLocation();

	void						onHatcheryProduction();
	void						onDroneProduction();
	void						onGoalProduction();

public:

	static ProductionManager &	Instance();

	void						drawQueueInformation(std::map<BWAPI::UnitType, int> & numUnits, int x, int y, int index);
	void						update();

	void						onGameEnd();
	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitDestroy(BWAPI::Unit * unit);
	
	void						drawProductionInformation(int x, int y);

	bool						isStrategyComplete();

	void						RushDefend(BWAPI::UnitType defendBuilding, int buildingCount, BWAPI::UnitType defendUnit, int unitCount);
	void						defendLostRecover();
	void						clearDefendVector();

	void						triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count);
	void						triggerUnit(BWAPI::UnitType unitType, int unitCount);
	void						triggerUpgrade(BWAPI::UpgradeType upgrade) { queue.queueAsHighestPriority(MetaType(upgrade), false); }

	void						clearCurrentQueue(){ queue.clearAll(); }
	void						setBuildOrder(const std::vector<MetaType> & buildOrder);
};
