#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "network.h"
#include "LinearRegression.h"
#include "SumTree.h"

typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;


struct sampleParsedInfo
{
	vector<double> modelInputs;
	int actualChosen;
	double targetQValue;
};


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
	std::map<std::string, std::pair<BWAPI::UnitType, BWAPI::UnitType>>			actionBuildingTarget;
	std::map<std::string, std::map<std::string, std::vector<std::string>>>		featureNames;
	int													previousScore;
	std::string											previousAction;
	std::map<std::string, std::map<std::string, double>>						parameterValue;
	std::map<std::string, std::map<std::string, int>>						featureValue;



	Network						policyModel;
	Network						QValueModel;
	Network						targetQValueModel;

	Network						stateValueModel;

	std::map<std::string, MetaType> highLevelActions;
	std::map<std::string, std::vector<std::string>>	features;
	std::vector<std::string>	NNInputAction;
	std::vector<std::string>	NNOutputAction;
	std::map<std::string, int>	actionsOutputIndexMapping;

	std::map<std::string, double> featureValues;
	std::vector<std::vector<std::string>> trainingData;
	int							lastExpandTime;

	std::map<BWAPI::UnitType, std::pair<int, int>>		buildingsUnderProcess;
	std::map<BWAPI::UnitType, std::string> buildingsName;
	std::map<BWAPI::TechType, std::string> techsName;
	std::map<BWAPI::UpgradeType, std::string> upgradesName;
	std::map<BWAPI::TechType, std::string> techUnderProcess;
	std::map<BWAPI::UpgradeType, std::string> upgradeUnderProcess;
	std::string					previousChosenAction;
	double						iterationTrain(int miniBatch);
	double						tmpReward;
	int							tdStep;
	int							replayLength;


	int							curActionTime;
	void						featureWeightInit();
	void						experienceDataInit();


	SumTree						replayData;
	double						getSamplePriority(std::vector<std::string>& sample);
	sampleParsedInfo			sampleParse(std::vector<std::string>& sample);

	std::map<BWAPI::UnitType, int> morphUnits;



	std::vector<std::vector<std::string>> testSetData;
	double						testSetAvgQValue;
	void						tesetSetPerformance();

	std::map<std::string, std::map<std::string, double>>						parameterCumulativeGradient;
	void						featureCumulativeGradientInit();


	
	std::vector<std::vector<std::string>>	curEpisodeData;
	double						discountRate;
	std::string					getCategory(std::vector<std::string>& categoryRange, int curValue, std::string prefix);
	double						getNormalizedValue(std::vector<std::string>& categoryRange, int curValue);
	void						calCurrentStateFeature();
	std::string					opponentWinrate;
	void						setArmyUpgrade();

	int							playMatchCount;
	bool 						muteBuilding;
	bool						isInBuildingMutalisk;

	std::string					curBuildingAction;
	int							nextDefenseBuildTime;
	
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

	std::string					getCurrentBuildingAction() { return curBuildingAction; }

	MetaType					getTargetUnit(BWAPI::UnitType exploreUnit);

	MetaType					getTargetDQN();
	void						trainModels(int reward);
	void						buildingFinish(BWAPI::UnitType u);


	void						getStrategyBuildingOrder();
	std::vector<MetaType>		getCurrentStrategyUnit();

	void						featureWeightSave();
	void						featureGradientSave();
	void						experienceDataSave();

	void						setOpponentWinrate(std::string rate) { opponentWinrate = rate; }
	int							getPlayeMatchCount() { return playMatchCount; }
	bool						isBuildingMutaliskBuilding() { return isInBuildingMutalisk; }

	bool						isGasRequireMeet(int price);
	bool						isMineRequireMeet(int price);

	void						setReward(BWAPI::UnitType u, bool killEnemy);

};
