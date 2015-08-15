#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"


typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;


enum zergGameStage{Start = 0, Mid = 1, End = 2};
enum strategyGoal { BaseExpand, MutaliskRush, HydraliskRush, ZerglingRush, MutaHydraRush };

class StrategyManager
{
	StrategyManager();
	~StrategyManager() {}

	std::vector<std::string>	zergStartStrategyBook;
	std::vector<std::string>	zergMidStrategyBook;
	std::vector<std::string>	zergEndStrategyBook;

	std::string					readDir;
	std::string					writeDir;
	std::vector<IntPair>		results;
	std::vector<int>			usableStrategies;
	int							currentStrategy;
	zergGameStage				gameStage;


	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	bool						firstAttackSent;

	void	readResults();
	void	writeResults();

	const	int					getScore(BWAPI::Player * player) const;
	const	double				getUCBValue(const size_t & strategy) const;

	// protoss strategy
	const	MetaPairVector		getZergBuildOrderGoal() const;

	const	MetaPairVector		getZergOpeningBook() const;

	std::vector<MetaType> 		actions;
	std::vector<MetaType>		getMetaVector(std::string buildString);

	void						goalBuildingOrderInit();
	
	std::map<strategyGoal, std::vector<MetaType>> goalBuildingOrder;
	strategyGoal				currentStrategyGoal;
	bool						triggerMutaliskBuild;

	bool						disableMutaliskHarass;

public:

	static	StrategyManager &	Instance();

	void						onEnd(const bool isWinner);

	const	bool				regroup(int numInRadius);
	const	bool				doAttack(const std::set<BWAPI::Unit *> & freeUnits);
	const	int				    defendWithWorkers();
	const	bool				rushDetected();

	const	int					getCurrentStage() { return gameStage; }

	void						changeGameStage(zergGameStage curStage);
	int							getGameStage();
	const	MetaPairVector		getBuildOrderGoal();
	std::vector<MetaType>		getOpeningBook();

	
	const std::vector<MetaType>&		getCurrentGoalBuildingOrder();
	int							getCurrentStrategyGoal() { return currentStrategyGoal; }
	bool						mutaliskHarassFlag() { return disableMutaliskHarass; }
	void						update();
	void						goalChange(strategyGoal changeGoal);
	void						baseExpand();
};
