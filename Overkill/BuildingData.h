#pragma once

#include "Common.h"
#include "MetaType.h"

class Building {

public:
	enum BuildingState { initBuilderAndLocation, exploreMove, issueBuildOrder, refinerySpecial, buildingOrderCheck, end };

	BuildingState buildingState;

	BWAPI::TilePosition desiredPosition;
	BWAPI::TilePosition finalPosition;
	BWAPI::Position position;
	BWAPI::UnitType type;
	BWAPI::Unit buildingUnit;
	BWAPI::Unit builderUnit;
	int lastOrderFrame;
	bool buildCommandGiven;
	bool underConstruction;
	std::vector<MetaType> waitingBuildType;

	Building()
		: desiredPosition(0, 0), finalPosition(BWAPI::TilePositions::None), position(0, 0),
		type(BWAPI::UnitTypes::Unknown), buildingUnit(NULL),
		builderUnit(NULL), lastOrderFrame(0), buildCommandGiven(false), underConstruction(false) 
	{
		buildingState = initBuilderAndLocation;
		waitingBuildType = std::vector<MetaType>();
	}

	// constructor we use most often
	Building(BWAPI::UnitType t, BWAPI::TilePosition desired)
		: desiredPosition(desired), finalPosition(0, 0), position(0, 0),
		type(t), buildingUnit(NULL), builderUnit(NULL),
		lastOrderFrame(0), buildCommandGiven(false), underConstruction(false) 
	{
		buildingState = initBuilderAndLocation;
		waitingBuildType = std::vector<MetaType>();
	}

	Building(BWAPI::UnitType t, BWAPI::TilePosition desired, std::vector<MetaType> w)
		: desiredPosition(desired), finalPosition(0, 0), position(0, 0),
		type(t), buildingUnit(NULL), builderUnit(NULL),
		lastOrderFrame(0), buildCommandGiven(false), underConstruction(false)
	{
		buildingState = initBuilderAndLocation;
		waitingBuildType = w;
	}

	// equals operator
	bool operator==(const Building & b) {
		// buildings are equal if their worker unit or building unit are equal
		return (b.buildingUnit == buildingUnit) || (b.builderUnit == builderUnit);
	}
};

class ConstructionData {

public:

	typedef enum BuildingState_t { Unassigned = 0, Assigned = 1, UnderConstruction = 2, NumBuildingStates = 3 } BuildingState;

private:

	int							reservedMinerals;				// minerals reserved for planned buildings
	int							reservedGas;					// gas reserved for planned buildings
	int							buildingSpace;					// how much space we want between buildings

	std::vector< size_t >						buildingIndex;
	std::vector< std::vector<Building> >		buildings;			// buildings which do not yet have builders assigned

	std::set<BWAPI::Unit>		buildingUnitsConstructing;		// units which have been recently detected as started construction

public:

	ConstructionData();

	Building &					getNextBuilding(BuildingState bs);
	bool						hasNextBuilding(BuildingState bs);
	void						begin(BuildingState bs);
	void						addBuilding(BuildingState bs, const Building & b);
	void						removeCurrentBuilding(BuildingState bs);
	void						removeBuilding(BuildingState bs, Building & b);

	int							getNumBuildings(BuildingState bs);

	bool						isBeingBuilt(BWAPI::UnitType type);
};
