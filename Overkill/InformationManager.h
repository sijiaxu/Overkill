#pragma once

#include "Common.h"
#include "BuildingData.h"
#include "WorkerManager.h"
#include "ProductionManager.h"
#include "StrategyManager.h"


//gather enemy building/unit/base and self building/unit/base info
//for further decision making

struct buildingInfo
{
	BWAPI::Unit	unit;
	BWAPI::UnitType unitType;
	BWAPI::TilePosition initPosition;
	//TODO: check for cheating build
	bool			isComplete;

	buildingInfo(BWAPI::Unit u, BWAPI::TilePosition p, bool completeFlag, BWAPI::UnitType ut)
	{
		unit = u;
		initPosition = p;
		isComplete = completeFlag;
		unitType = ut;
	}
	buildingInfo()
	{}
};

//for enemy influence map
struct gridInfo
{
	double airForce;
	double groundForce;

	//indicate the distance from the attack building, used for path finding
	double decayAirForce;
	double decayGroundForce;

	double enemyUnitAirForce;
	double enemyUnitGroundForce;

	double enemyUnitDecayAirForce;
	double enemyUnitDecayGroundForce;
	gridInfo()
	{
		airForce = 0;
		groundForce = 0;

		decayAirForce = 0;
		decayGroundForce = 0;

		enemyUnitAirForce = 0;
		enemyUnitGroundForce = 0;

		enemyUnitDecayAirForce = 0;
		enemyUnitDecayGroundForce = 0;
	}
};


class InformationManager {

	InformationManager();

	//self info;
	BWAPI::Unit						selfBaseUnit;
	BWAPI::Position						selfNaturalChokePoint;

	BWAPI::TilePosition					selfStartBaseLocation;
	BWAPI::TilePosition					selfNaturalBaseLocation;
	std::set<BWAPI::Unit>				selfAllBase;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>		selfAllBuilding;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>		selfAllBattleUnit;

	//enemy info
	BWAPI::TilePosition					enemyStartBaseLocation;
	BWAPI::TilePosition					enemyNaturalBaseLocation;
	std::set<BWAPI::Unit>				enemyAllBase;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>			enemyAllBuilding;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>			enemyAllBattleUnit;

	//occupied region
	std::set<BWTA::Region *>			occupiedRegions[2];

	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>> selfOccupiedDetail;
	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>> enemyOccupiedDetail;

	//first Creep_Colony location
	BWAPI::TilePosition					firstColonyLocation;

	void							updateUnit(BWAPI::Unit unit);
	bool							defendTrig;
	bool							depotBalanceFlag;
	int								waitforDepotTime;

	bool							natrualFlag;
	int								waitTime;

	void							checkSelfNewDepotFinish();
	std::map<BWAPI::Unit, bool>	depotTrigMap;

	void							checkEarlyRush();
	bool							zealotRushFlag;
	bool							zealotRushTrig;

	void							checkMidRush();
	bool							midRushFlag;

	BWAPI::TilePosition				myFirstSunkerLocation;

	void							checkVeryEarlyRush();
	bool							earlyRush;

	void							checkEarlyRushDefend();
	int								defendAddSupply;
	
	std::vector<std::vector<gridInfo>>			enemyInfluenceMap;
	void							addUnitInfluenceMap(BWAPI::Unit unit, bool addOrdestroy);
	void							updateEnemyUnitInfluenceMap();
	void							setInfluenceMap(BWAPI::Position initPosition, int attackRange, int groundDamage, int airDamage, bool addOrdestroy);

	int								waitToBuildSunker;

	BWAPI::TilePosition				baseSunkenBuildingPosition;
	BWAPI::TilePosition				natrualSunkenBuildingPosition;

	bool							enemyEarlyRushSuccess;
	void							checkAirDrop();
	bool							airDropTrigger;

	void							checkAirDefend();
	bool							airDefendTrigger;
	bool							chamberTrigger;

	bool							needDefendCheck;
	int								buildZerglingChekcTime;
	int								defendBuildZerglingsCount;

public:
	void							setLocationEnemyBase(BWAPI::TilePosition Here);

	void							updateOccupiedRegions(BWTA::Region * region, BWAPI::Player player);
	void							addOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player player, BWAPI::Unit building);
	void							destroyOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player player, BWAPI::Unit building);
	void							checkOccupiedDetail();

	BWAPI::Unit						GetOurBaseUnit();
	BWAPI::Position					GetEnemyBasePosition();
	BWAPI::Position					GetEnemyNaturalPosition();

	BWAPI::TilePosition				GetNextExpandLocation();
	std::set<BWTA::Region *> &		getOccupiedRegions(BWAPI::Player player);
	
	BWAPI::TilePosition				getOurNatrualLocation();
	BWAPI::TilePosition				getOurFirstColonyLocation() { return firstColonyLocation; }

	std::set<BWAPI::Unit>&			getOurAllBaseUnit() { return selfAllBase; }
	std::set<BWAPI::Unit>&			getEnemyAllBaseUnit() { return enemyAllBase; }
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getOurAllBattleUnit() { return selfAllBattleUnit; }
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getOurAllBuildingUnit() { return selfAllBuilding; }

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getEnemyAllBattleUnit() { return enemyAllBattleUnit; }
	int								getEnemyGroundBattleUnitSupply();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getEnemyAllBuildingUnit() { return enemyAllBuilding; }
	std::vector<std::vector<gridInfo>>& getEnemyInfluenceMap() { return enemyInfluenceMap; }

	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& getEnemyOccupiedDetail() { return enemyOccupiedDetail; }
	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& getSelfOccupiedDetail() { return selfOccupiedDetail; }

	BWAPI::TilePosition				getSunkenBuildingPosition();
	void							addWaitBuildSunkun(int count) { waitToBuildSunker += count; }
	bool							getEnemyEarlyRushSuccess() { return enemyEarlyRushSuccess; }

	void							onUnitShow(BWAPI::Unit unit);
	void							onUnitMorph(BWAPI::Unit unit);
	void							onUnitDestroy(BWAPI::Unit unit);

	bool							isEarlyRush() { return earlyRush; }
	bool							isEnemyHasInvisibleUnit();
	bool							isEnemyHasFlyerAttacker();

	static InformationManager&		Instance();
	void							setDefend(bool status) { needDefendCheck = status;  if (needDefendCheck == true) BWAPI::Broodwar->printf("defend check open!!!!"); }

	void							update();
};


