#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"


typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;


enum zergGameStage{Start = 0, Mid = 1, End = 2};

enum openingStrategy { TwelveHatchMuta, NinePoolling, TenHatchMuta };
enum zergStrategy { HydraPush, MutaPush, ZerglingPush };


class StrategyManager
{
	StrategyManager();
	~StrategyManager() {}
	zergGameStage				gameStage;

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	std::vector<MetaType> 		actions;
	std::vector<MetaType>		getMetaVector(std::string buildString);

	void						goalBuildingOrderInit();
	
	std::map<zergStrategy, std::vector<MetaType>> goalBuildingOrder;
	openingStrategy				currentopeningStrategy;
	zergStrategy				currentStrategy;

	bool						triggerMutaliskBuild;

	bool						disableMutaliskHarass;

	std::vector<std::string>	openingStrategyName;

	bool						lairTrigger;
	bool						spireTrigger;

	bool						overlordUpgradeTrigger;
	bool						mutaUpgradeTrigger;
	bool						hydraUpgradeTrigger;

public:

	static	StrategyManager &	Instance();

	void						onEnd(const bool isWinner);

	const	int					getCurrentStage() { return gameStage; }

	void						changeGameStage(zergGameStage curStage);
	int							getGameStage();
	std::vector<MetaType>		getOpeningBook();
	void						setOpeningStrategy(openingStrategy opening);

	
	const std::vector<MetaType>&		getCurrentGoalBuildingOrder();
	openingStrategy				getCurrentopeningStrategy() { return currentopeningStrategy; }
	zergStrategy				getCurrentStrategy() { return currentStrategy; }

	bool						mutaliskHarassFlag() { return disableMutaliskHarass; }
	void						update();
	void						goalChange(zergStrategy changeStrategy);
	void						baseExpand();

	int							getStrategyByName(std::string strategy);
	std::string					getStrategyName(openingStrategy strategy);
	std::vector<std::string>	getStrategyNameArray() { return openingStrategyName; }
	int							getScore(BWAPI::Player player);
};
