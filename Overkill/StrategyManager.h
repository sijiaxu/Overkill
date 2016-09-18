#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>

typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;



class StrategyManager
{
	StrategyManager();
	~StrategyManager() {}

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	std::vector<MetaType> 		actions;
	std::vector<MetaType>		getMetaVector(std::string buildString);

	openingStrategy				currentopeningStrategy;

	bool						triggerMutaliskBuild;

	bool						disableMutaliskHarass;

	std::vector<std::string>	openingStrategyName;

	bool						lairTrigger;
	bool						spireTrigger;

	bool						overlordUpgradeTrigger;
	bool						mutaUpgradeTrigger;
	bool						hydraUpgradeTrigger;

	std::vector<std::string>							stateActions;
	std::map<std::string, BWAPI::UnitType>				actionBuildingTarget;
	std::map<std::string, std::map<std::string, std::vector<std::string>>>		featureNames;
	int													previousScore;
	std::string											previousAction;
	std::map<std::string, std::map<std::string, double>>						parameterValue;
	std::map<std::string, std::map<std::string, int>>						featureValue;
	int							curActionTime;
	void						featureWeightInit();
	void						experienceDataInit();

	std::map<std::string, std::map<std::string, double>>						parameterCumulativeGradient;
	void						featureCumulativeGradientInit();


	std::vector<std::vector<std::string>> experienceData;
	std::vector<std::vector<std::string>>	curEpisodeData;
	double						discountRate;
	double						calQValue(std::string stringData, std::map<std::string, std::map<std::string, int>>& stringValue);
	std::string					getCategory(std::vector<std::string>& categoryRange, int curValue, std::string prefix);
	void						calCurrentStateFeature();
	double						calActionFeature(std::string curAction, std::map<std::string, std::map<std::string, int>>& features);
	std::string					opponentWinrate;
	void						setArmyUpgrade();

	int							playMatchCount;
	bool 						muteBuilding;
	bool						isInBuildingMutalisk;
	

public:

	static	StrategyManager &	Instance();

	std::vector<MetaType>		getOpeningBook();
	void						setOpeningStrategy(openingStrategy opening);
	
	openingStrategy				getCurrentopeningStrategy() { return currentopeningStrategy; }

	bool						mutaliskHarassFlag() { return disableMutaliskHarass; }
	void						update();
	void						baseExpand();

	int							getStrategyByName(std::string strategy);
	std::string					getStrategyName(openingStrategy strategy);
	std::vector<std::string>	getStrategyNameArray() { return openingStrategyName; }
	int							getScore();

	std::string					strategyChange(int reward);
	std::vector<MetaType>		getStrategyBuildingOrder(std::string stateAction);

	void						featureWeightSave();
	void						featureGradientSave();
	void						experienceDataSave();

	void						setOpponentWinrate(std::string rate) { opponentWinrate = rate; }
	int							getPlayeMatchCount() { return playMatchCount; }
	bool						isBuildingMutaliskBuilding() { return isInBuildingMutalisk; }

};
