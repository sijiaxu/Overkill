#pragma once

#include "Common.h"
#include "WorkerManager.h"
#include "BuildingData.h"
#include "InformationManager.h"

class BuildingManager {

	BuildingManager();

	//ConstructionData			buildingData;

	std::vector<Building>		buildingData;

	bool						debugMode;
	int							totalBuildTasks;

	int							reservedMinerals;				// minerals reserved for planned buildings
	int							reservedGas;					// gas reserved for planned buildings
	int							buildingSpace;					// how much space we want between buildings

	std::vector<BWAPI::Unit *>	builders;						// workers which have been assigned to buildings
	std::vector<Building>		buildingsNeedingBuilders;		// buildings which do not yet have builders assigned
	std::vector<Building>		buildingsAssigned;				// buildings which have workers but not yet under construction
	std::vector<Building>		buildingsUnderConstruction;		// buildings which are under construction
	std::vector<BWAPI::Unit *>	buildingUnitsConstructing;		// units which have been recently detected as started construction

	// functions
	bool						isEvolvedBuilding(BWAPI::UnitType type);
	bool						isBuildingPositionExplored(const Building & b) const;

	// the update() functions
	void						assignWorkersToUnassignedBuildings(Building& b);	// STEP 2
	void						exploreUnseenPosition(Building& b);
	void						constructAssignedBuildings(Building& b);			// STEP 3
	void						buildingOrderCheck(Building& b);

	//building placement info
	BWAPI::TilePosition			getRefineryPosition();
	bool						canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const;
	bool						buildable(int x, int y, const Building & b) const;
	BWAPI::TilePosition			getBuildLocationNear(const Building & b, int buildDist, bool inRegion = false, bool horizontalOnly = false) const;

	// functions for performing tedious vector tasks

	char						getBuildingWorkerCode(const Building & b) const;

public:

	void						update();
	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitDestroy(BWAPI::Unit * unit);
	void						addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation);

	//for predict move 
	BWAPI::TilePosition			getBuildingLocation(const Building & b);

	int							getReservedMinerals();
	int							getReservedGas();

	static BuildingManager &	Instance();

	void						drawBuildingInformation(int x, int y);



};
