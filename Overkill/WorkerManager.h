#pragma once

#include "Common.h"
#include "BuildingManager.h"
#include "WorkerData.h"
#include "ScoutManager.h"
#include "InformationManager.h"
#include "StrategyManager.h"

class Building;

class WorkerManager {

	//static WorkerManager *		instance;

	WorkerData					workerData;
	BWAPI::Unit *               previousClosestWorker;

	int							workersPerRefinery;
	bool						assignScout;

	void						setMineralWorker(BWAPI::Unit * unit);
	int							depotTriggerTime;

	WorkerManager();

	void						handleWorkerReact();

public:

	void						update();
	void						balanceWorkerOnDepotComplete(BWAPI::Unit* depot);
	void						onUnitDestroy(BWAPI::Unit * unit);
	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitShow(BWAPI::Unit * unit);
	void						onUnitRenegade(BWAPI::Unit * unit);
	void						addWoker(BWAPI::Unit* unit) { workerData.addWorker(unit); }
	void						finishedWithWorker(BWAPI::Unit * unit);

	void						handleIdleWorkers();
	void						handleGasWorkers();
	void						handleMoveWorkers();
	void						handleCombatWorkers();
	void						finishedWithCombatWorkers();

	void						drawResourceDebugInfo();
	void						updateWorkerStatus();

	int							getNumMineralWorkers();
	int							getNumGasWorkers();
	int							getNumIdleWorkers();
	void						setScoutWorker(BWAPI::Unit * worker);
	void						handleScoutWorker();


	bool						isWorkerScout(BWAPI::Unit * worker);
	bool						isFree(BWAPI::Unit * worker);
	bool						isBuilder(BWAPI::Unit * worker);

	BWAPI::Unit *				getBuilder(Building & b, bool setJobAsBuilder = true);
	BWAPI::Unit *				getMoveWorker(BWAPI::Position p);
	BWAPI::Unit *				getClosestDepot(BWAPI::Unit * worker);
	BWAPI::Unit *				getGasWorker(BWAPI::Unit * refinery);
	BWAPI::Unit *				getClosestEnemyUnit(BWAPI::Unit * worker);
	BWAPI::Unit *               getClosestMineralWorkerTo(BWAPI::Unit * enemyUnit);
	int							getDepotMineralWorkerNum(BWAPI::Unit* depot) { return workerData.getDepotWorker(depot); }

	void						setMoveWorker(int m, int g, BWAPI::Position p);
	void						setCombatWorker(BWAPI::Unit * worker);
	void						setCombatWorkerArmy(int combatWorkerNum);
	void						smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target);

	bool						willHaveResources(int mineralsRequired, int gasRequired, double distance);
	void						rebalanceWorkers();

	static WorkerManager &		Instance();

};
