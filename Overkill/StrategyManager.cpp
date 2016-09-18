
#include "StrategyManager.h"
#include "ModelWeightInit.h"

// constructor
StrategyManager::StrategyManager()
	: selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone)); //0
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Overlord)); //1
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hatchery)); //2
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool)); //3
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Zergling)); //4
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Extractor)); //5
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Lair)); //6
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den)); //7
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Spire)); //8
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk)); //9
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk)); //10
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony)); //11
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony)); //12
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Evolution_Chamber)); //13
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den)); //14


		actions.push_back(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost)); // 15
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Carapace)); // 16
		//Hydralisk
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Grooved_Spines)); // 17
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Muscular_Augments)); // 18

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)); // 19
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)); // 20

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)); // 21
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)); // 22
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Carapace)); // 23
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Pneumatized_Carapace)); // 24
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Ventral_Sacs)); // 25
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Antennae)); // 26
	}

	triggerMutaliskBuild = true;

	disableMutaliskHarass = false;

	openingStrategyName.push_back("TwelveHatchMuta");
	openingStrategyName.push_back("NinePoolling");
	openingStrategyName.push_back("TenHatchMuta");

	lairTrigger = true;
	spireTrigger = true;

	mutaUpgradeTrigger = true;
	hydraUpgradeTrigger = true;
	overlordUpgradeTrigger = true;


	//zergling build is the default strategy
	stateActions = {"zerglingBuild", "hydraBuild", "mutaBuild"};

	actionBuildingTarget["hydraBuild"] = BWAPI::UnitTypes::Zerg_Hydralisk;
	actionBuildingTarget["mutaBuild"] = BWAPI::UnitTypes::Zerg_Mutalisk;
	actionBuildingTarget["zerglingBuild"] = BWAPI::UnitTypes::Zerg_Zergling;

	//all feature is binary
	//continue feature is discretized to seven category
	//the category range is define by its vector 
	//it is important that action feature represent the action effect on current state, not the action it self.
	featureNames = decltype(featureNames) {
		//state feature
		{"state_general_feature", { 
			{ "time_", { "6", "12", "20", "30", "max" } },
			{ "enemyRace_", { "z", "t", "p", "unknow" } },
			{ "ourSupplyUsed_", { "9", "18", "30", "50", "80", "120", "150", "max" } },
			//multiply 10
			{ "enemyWinPercent_", {"0", "2", "4", "6", "8", "9", "max"} },
			{ "airDistance_", { "64", "96", "128", "160", "196", "228", "max" } },
			{ "mapWidth_", { "64", "96", "128", "196", "max" } },
			{ "mapHeight_", { "64", "96", "128", "196", "max" } },
			{ "ourLarvaCount_", { "0", "1", "3", "6", "9", "12", "16", "20", "max" } },
			{ "openingStrategy_", { "0", "1", "2" } }
		}},

		{ "state_tech_feature", {
			{ "ourTechLevel_", { "hatchery", "lair", "hive" } },
			{ "ourKeyUpgrade_", { "hasLurker", "hasLordSpeed", "hasLordLoad", "zerglingsAttackSpeed" } }
		} },

		{ "state_building_feature", {
			{ "ourKeyBuilding_", {"hasSpire"} },
			{ "enemyKeyBuilding_", { "hasZ_Spire", "hasZ_hive", \
			"hasT_starPort", "hasT_engineerBay", \
			"hasP_stargate", "hasP_robotics_facility", "hasP_temple", "hasP_fleet_beacon", "hasP_Citadel" } },
			{ "enemyP_Gateway_", { "0", "1", "2", "3", "4", "5", "max" } },
			{ "enemyT_Barracks_", { "0", "1", "2", "3", "4", "5", "max" } },
			{ "enemyT_Factory_", { "0", "1", "2", "3", "4", "5", "max" } }

		} },

		{ "state_economy_feature", {
			{ "ourMineral_", { "100", "200", "300", "500", "800", "1200", "2000","max" } },
			{ "ourGas_", { "100", "200", "300", "500", "800", "1200", "2000","max" } },
			{ "ourExpandBase_", { "0","1", "2", "3", "4", "5", "6", "max" } },
			{ "ourWorkers_", { "6", "12", "20", "30", "50", "70", "max" } },
			{ "enemyExpandBase_", { "0", "1", "2", "3", "4", "5", "6", "max" } },
			{ "enemyWorkers_", { "6", "12", "20", "30", "50", "70", "max" } }
		} },

		{ "state_our_army", {
			{ "ourSunken_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "ourSpore_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "ourZergling_", { "6", "12", "24", "40", "60", "80", "100", "140", "max" } },
			{ "ourMutalisk_", { "3", "6", "9", "12", "18", "24", "30", "36", "max" } },
			{ "ourHydra_", { "6", "12", "18", "24", "36", "48", "60", "max" } }
		} },

		{ "state_battle_feature", {

			{ "enemyP_cannon_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "enemyT_missile_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "enemyT_Bunker_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "enemyZ_Sunken_", { "1", "2", "3", "5", "8", "12", "max" } },
			{ "enemyZ_Spore_", { "1", "2", "3", "5", "8", "12", "max" } },

			
			{ "enemyZ_Zergling_", { "6", "12", "24", "40", "60", "80", "100", "140", "max" } },
			{ "enemyZ_Mutalisk_", { "3", "6", "9", "12", "18", "24", "30", "36", "max" } },
			{ "enemyZ_Hydra_", { "6", "12", "18", "24", "36", "48", "60", "max" } },
			{ "enemyZ_Lurker_", { "6", "12", "18", "24", "30", "36", "max" } },

			{ "enemyP_Zealot_", { "3", "6", "12", "18", "24", "36", "max" } },
			{ "enemyP_Dragon_", { "3", "6", "12", "18", "24", "36", "max" } },
			{ "enemyP_High_temple_", { "1", "3", "5", "8", "10", "12", "max" } },
			{ "enemyP_Carrier_", { "1", "3", "6", "12", "18", "24", "max" } },
			{ "enemyP_Corsair_", { "3", "6", "12", "18", "24", "30", "max" } },
			{ "enemyP_Shuttle_", { "1", "2", "3", "5", "7", "9", "max" } },

			{ "enemyT_Goliath_", { "3", "6", "12", "18", "24", "30", "max" } },
			{ "enemyT_Marine_", { "3", "6", "12", "18", "24", "30", "36", "max" } },
			{ "enemyT_Tank_", { "3", "6", "12", "18", "24", "30", "max" } },
			{ "enemyT_Vulture_", { "3", "6", "12", "18", "24", "30", "max" } },
			{ "enemyT_Dropship_", { "1", "2", "3", "5", "7", "9", "max" } },
			{ "enemyT_Valkyrie_", { "1", "3", "6", "9", "12", "18", "max" } },
			{ "enemyT_Science_", { "1", "2", "3", "5", "7", "9", "max" } },
			{ "enemyT_Firebat_", { "3", "6", "12", "18", "24", "30", "max" } },
			{ "enemyT_Terran_Medic_", { "1", "2", "3", "5", "7", "9", "12","max" } }
		} },

		{ "state_raw_combine_feature", {} },
		{ "action_battle_combine_state_battle_feature", {} }
	};

	featureWeightInit();
	featureCumulativeGradientInit();
	 
	//TimerManager::Instance().startTimer(TimerManager::strategy);
	experienceDataInit();
	//TimerManager::Instance().stopTimer(TimerManager::strategy);

	previousAction = "";

	//add upgrade callback check
	setArmyUpgrade();
	discountRate = 0.99;
	
	muteBuilding = true;

	isInBuildingMutalisk = false;

}


void StrategyManager::setOpeningStrategy(openingStrategy opening)
{
	currentopeningStrategy = opening;
}

int StrategyManager::getStrategyByName(std::string strategy)
{
	for (int i = 0; i <int(openingStrategyName.size()); i++)
	{
		if (openingStrategyName[i] == strategy)
		{
			return i;
		}
	}
	return -1;
}

std::string	StrategyManager::getStrategyName(openingStrategy strategy)
{
	return openingStrategyName[int(strategy)];
}




void StrategyManager::baseExpand()
{
	BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
	if (nextBase == BWAPI::TilePositions::None)
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self()->getStartLocation(), 1);
	else
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hatchery, nextBase, 1);
}


void StrategyManager::experienceDataSave()
{	
	string filePath;
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/RL_data";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/write/RL_data";
		filePath += enemyName;
	}
	
	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::out);
	std::vector<std::vector<std::string>> subData;

	//only use the recent 300 experience data.
	if (experienceData.size() > 3000)
	{
		experienceData.erase(experienceData.begin(), experienceData.end() -= 3000);
	}

	//store the (St-1, action, r, St) info
	//scheme Q(St-1) | action | Q(st) | reward |
	for (auto rlData : experienceData)
	{
		for (int i = 0; i < 4; i++)
		{
			historyModel << rlData[i];
			if (i != 3)
			{
				historyModel << "|";
			}
		}
		historyModel << endl;
	}
	historyModel.close();
}

void StrategyManager::featureWeightSave()
{
	string filePath;
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/feature_value";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/write/feature_value";
		filePath += enemyName;
	}
	
	fstream historyModel;

	historyModel << std::fixed << std::setprecision(5);

	historyModel.open(filePath.c_str(), ios::out);

	bool featureValid = true;

	for (std::map<std::string, std::map<std::string, double>> ::iterator it = parameterValue.begin(); it != parameterValue.end();)
	{
		if (featureNames.find(it->first) == featureNames.end())
		{
			parameterValue.erase(it++);
			featureValid = false;
		}
		else
		{
			it++;
		}
	}
	if (featureValid == false)
	{
		fstream lossFile;
		string lossfilePath = "./bwapi-data/write/debug_file_parweight";
		lossFile.open(lossfilePath.c_str(), ios::app);
		lossFile << "1" << endl;
		lossFile.close();
		//return maxAction;
	}

	
	// file format:
	// feature_name1:value
	// feature_name2:value
	for (auto feature_categoy : parameterValue)
	{
		for (auto& feature : feature_categoy.second)
		{
			//weird bug...
			if (isnan(feature.second))
			{
				fstream lossFile;
				string lossfilePath = "./bwapi-data/write/debug_file";
				lossFile.open(lossfilePath.c_str(), ios::app);
				lossFile << feature.first << ":" << feature.second << endl;
				lossFile.close();

				continue;
			}
			historyModel << feature_categoy.first << ":" << feature.first << ":" << feature.second;
			historyModel << endl;
		}
	}
	historyModel.close();
}


void StrategyManager::featureGradientSave()
{
	string filePath;
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/feature_gradient";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/write/feature_gradient";
		filePath += enemyName;
	}
	fstream historyModel;

	historyModel << std::fixed << std::setprecision(5);

	historyModel.open(filePath.c_str(), ios::out);

	bool featureValid = true;

	for (std::map<std::string, std::map<std::string, double>> ::iterator it = parameterCumulativeGradient.begin(); it != parameterCumulativeGradient.end();)
	{
		if (featureNames.find(it->first) == featureNames.end())
		{
			parameterCumulativeGradient.erase(it++);
			featureValid = false;
		}
		else
		{
			it++;
		}
	}
	if (featureValid == false)
	{
		fstream lossFile;
		string lossfilePath = "./bwapi-data/write/debug_file_pargad";
		lossFile.open(lossfilePath.c_str(), ios::app);
		lossFile << "1" << endl;
		lossFile.close();
		//return maxAction;
	}

	// file format:
	// feature_name1:value
	// feature_name2:value
	for (auto feature_categoy : parameterCumulativeGradient)
	{
		for (auto feature : feature_categoy.second)
		{
			historyModel << feature_categoy.first << ":" << feature.first << ":" << feature.second;
			historyModel << endl;
		}
	}
	historyModel.close();
}

void StrategyManager::experienceDataInit()
{
	string filePath;
	//recent RL data to learn
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/RL_data";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/read/RL_data";
		filePath += enemyName;
	}
	
	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);

	if (historyModel.is_open())
	{
		string content;
		//store the (St-1, action, r, St) info
		//scheme Q(St-1) | action | Q(st) | reward
		while (getline(historyModel, content))
		{
			if (content == "")
				continue;
			std::stringstream ss(content);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, '|'))
			{
				itemList.push_back(item);
			}
			experienceData.push_back(itemList);
		}
	}
	else
	{
		BWAPI::Broodwar->printf("do not has experience data !");
	}
	historyModel.close();

	//get the total match count have player
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/match_count";
	}
	else
	{
		filePath = "./bwapi-data/read/match_count";
	}

	historyModel.open(filePath.c_str(), ios::in);
	if (historyModel.is_open())
	{
		string content;
		while (getline(historyModel, content))
		{
			if (content == "")
				continue;
			playMatchCount = std::stoi(content);
			break;
		}
	}
	else
	{
		BWAPI::Broodwar->printf("do not has match count data !");
		playMatchCount = 0;
	}
	historyModel.close();
}


void StrategyManager::featureWeightInit()
{
	string filePath;
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/feature_value";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/read/feature_value";
		filePath += enemyName;
	}

	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);

	//file do not exist, first match to the enemy
	if (!historyModel.is_open())
	{
		BWAPI::Broodwar->printf("no model find! first match");
		parameterValue = initWeightValue;
	}
	else
	{
		BWAPI::Broodwar->printf("reading previous model");
		string modelFeatureValue;
		// file format:
		// feature_categy:feature_name:value
		while (getline(historyModel, modelFeatureValue))
		{
			std::stringstream ss(modelFeatureValue);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, ':'))
			{
				if (item != "")
					itemList.push_back(item);
			}
			parameterValue[itemList[0]][itemList[1]] = std::stof(itemList[2]);
		}
	}
	historyModel.close();
}


void StrategyManager::featureCumulativeGradientInit()
{
	string filePath;
	if (curMode == Develop)
	{
		filePath = "./bwapi-data/write/feature_gradient";
	}
	else
	{
		string enemyName = BWAPI::Broodwar->enemy()->getName();
		filePath = "./bwapi-data/read/feature_gradient";
		filePath += enemyName;
	}

	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);

	//file do not exist, first match to the enemy
	if (!historyModel.is_open())
	{
		BWAPI::Broodwar->printf("no gradient find! first match");
		parameterCumulativeGradient = initWeightCount;
	}
	else
	{
		BWAPI::Broodwar->printf("reading previous gradient");
		string modelFeatureValue;
		// file format:
		// feature_categy:feature_name:value
		while (getline(historyModel, modelFeatureValue))
		{
			std::stringstream ss(modelFeatureValue);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, ':'))
			{
				if (item != "")
					itemList.push_back(item);
			}
			parameterCumulativeGradient[itemList[0]][itemList[1]] = std::stof(itemList[2]);
		}
	}
	historyModel.close();
}


std::vector<MetaType> StrategyManager::getStrategyBuildingOrder(std::string stateAction)
{
	std::vector<MetaType> buildingOrder;
	BWAPI::UnitType targetType = actionBuildingTarget[stateAction];

	if (targetType == BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		int a = 0;
	}

	bool needCheck = false;
	BWAPI::UnitType checkType = targetType;
	while (true)
	{
		std::map<BWAPI::UnitType, int> requireUnits = checkType.requiredUnits();
		for (auto unit : requireUnits)
		{
			if (unit.first.isBuilding() && BWAPI::Broodwar->self()->allUnitCount(unit.first) == 0)
			{
				buildingOrder.push_back(MetaType(unit.first));
				needCheck = true;
				checkType = unit.first;
			}
		}

		if (needCheck == false)
		{
			break;
		}
		else
		{
			needCheck = false;
		}
	}
	//if (targetType.gasPrice() > 0 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) == 0)
		//buildingOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Extractor));


	if (stateAction == "mutaBuild" && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spire) == 0 && muteBuilding)
	{
		muteBuilding = false;
		isInBuildingMutalisk = true;
		BWAPI::Broodwar->printf("muta building!!!!!");

		if (BWAPI::Broodwar->getFrameCount() < (10 * 24 * 60))
		{
			InformationManager::Instance().setDefend(true);
		}

		const std::function<void(BWAPI::Game*)> closeAction = [=](BWAPI::Game* g)->void
		{
			isInBuildingMutalisk = false;
			InformationManager::Instance().setDefend(false);
		};
		const std::function<bool(BWAPI::Game*)> closeCondition = [=](BWAPI::Game* g)->bool
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spire) == 1)
				return true;
			else
				return false;
		};
		BWAPI::Broodwar->registerEvent(closeAction, closeCondition, 1, 48);
		
		
		const std::function<void(BWAPI::Game*)> lairAction = [=](BWAPI::Game* g)->void
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0)
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self()->getStartLocation(), 1);
			
			const std::function<void(BWAPI::Game*)> spireAction = [=](BWAPI::Game* g)->void
			{
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self()->getStartLocation(), 1);
			};
			const std::function<bool(BWAPI::Game*)> spireCondition = [=](BWAPI::Game* g)->bool
			{
				if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 1 && BWAPI::Broodwar->self()->minerals() >= 150)
					return true;
				else
					return false;
			};
			BWAPI::Broodwar->registerEvent(spireAction, spireCondition, 1, 48);
		};
		const std::function<bool(BWAPI::Game*)> lairCondition = [=](BWAPI::Game* g)->bool
		{
			if (BWAPI::Broodwar->self()->gas() >= 100 && BWAPI::Broodwar->self()->minerals() >= 150)
				return true;
			else
				return false;
		};
		BWAPI::Broodwar->registerEvent(lairAction, lairCondition, 1, 48);
		
		
	}

	//std::reverse(buildingOrder.begin(), buildingOrder.end());

	int buildCount = 0;
	if (stateAction == "hydraBuild")
	{
		buildCount = 1;
	}
	else if (stateAction == "mutaBuild")
	{
		buildingOrder.clear();
		buildCount = 1;
	}
	else if (stateAction == "zerglingBuild")
	{
		buildCount = 1;
	}

	buildingOrder.insert(buildingOrder.end(), buildCount, targetType);
	
	return buildingOrder;
}


void StrategyManager::setArmyUpgrade()
{
	//overlord speed
	
	const std::function<void(BWAPI::Game*)> overlordAction = [=](BWAPI::Game* g)->void
	{
		int triggerTime = BWAPI::Broodwar->getFrameCount() + 24 * 10;
		const std::function<void(BWAPI::Game*)> tAction = [=](BWAPI::Game* g)->void
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Pneumatized_Carapace);
		};
		const std::function<bool(BWAPI::Game*)> tCondition = [=](BWAPI::Game* g)->bool
		{
			if (BWAPI::Broodwar->getFrameCount() > triggerTime)
				return true;
			else
				return false;
		};
		BWAPI::Broodwar->registerEvent(tAction, tCondition, 1, 48);
	};
	const std::function<bool(BWAPI::Game*)> overlordCondition = [=](BWAPI::Game* g)->bool
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0
			&& InformationManager::Instance().isEnemyHasInvisibleUnit())
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(overlordAction, overlordCondition, 1, 48);
	

	//hrdrisk attack range and speed
	const std::function<void(BWAPI::Game*)> HydraliskBaseAction = [=](BWAPI::Game* g)->void
	{
		ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Grooved_Spines);
		ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Muscular_Augments, true);
	};
	const std::function<bool(BWAPI::Game*)> HydraliskBaseCondition = [=](BWAPI::Game* g)->bool
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk) >= 12)
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(HydraliskBaseAction, HydraliskBaseCondition, 1, 48);

	//zergling speed
	const std::function<void(BWAPI::Game*)> ZerglingBaseAction = [=](BWAPI::Game* g)->void
	{
		int maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost);
		int currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost);
		bool isInQueue = ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Metabolic_Boost);
		if (!isInQueue && !BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Metabolic_Boost) && currentLvl < maxLvl)
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Metabolic_Boost);
	};
	const std::function<bool(BWAPI::Game*)> ZerglingBaseCondition = [=](BWAPI::Game* g)->bool
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Zergling) >= 12)
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(ZerglingBaseAction, ZerglingBaseCondition, 1, 48);

	//zergling attack
	const std::function<void(BWAPI::Game*)> ZerglingFastAttackAction = [=](BWAPI::Game* g)->void
	{
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Queens_Nest, BWAPI::Broodwar->self()->getStartLocation(), 1);

		const std::function<bool(BWAPI::Game*)> buildHiveCondition = [=](BWAPI::Game* g)->bool
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Queens_Nest) > 0)
				return true;
			else
				return false;
		};

		const std::function<void(BWAPI::Game*)> buildHiveAction = [=](BWAPI::Game* g)->void
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hive, BWAPI::Broodwar->self()->getStartLocation(), 1);

			const std::function<bool(BWAPI::Game*)> upZAttackCondition = [=](BWAPI::Game* g)->bool
			{
				if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
					return true;
				else
					return false;
			};

			const std::function<void(BWAPI::Game*)> upZAttackAction = [=](BWAPI::Game* g)->void
			{
				ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Adrenal_Glands, true);
			};

			BWAPI::Broodwar->registerEvent(upZAttackAction, upZAttackCondition, 1, 48);

		};
		BWAPI::Broodwar->registerEvent(buildHiveAction, buildHiveCondition, 1, 48);

	};
	const std::function<bool(BWAPI::Game*)> ZerglingFastAttackCondition = [=](BWAPI::Game* g)->bool
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Zergling) >= 36 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Lair) > 0)
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(ZerglingFastAttackAction, ZerglingFastAttackCondition, 1, 48);


	//hydrisk/lurker attack && defend upgrade
	const std::function<void(BWAPI::Game*)> GroundRangeAction = [=](BWAPI::Game* g)->void
	{
		BWAPI::Unit chamber = NULL;
		std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
		if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == ourBuildings.end())
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->self()->getStartLocation(), 1);
		}

		int maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
		int currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Missile_Attacks) && currentLvl < maxLvl
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Missile_Attacks))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
		}

		maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Carapace) && currentLvl < maxLvl
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Carapace))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
		}
	};
	const std::function<bool(BWAPI::Game*)> GroundRangeCondition = [=](BWAPI::Game* g)->bool
	{
		int maxAttackLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
		int currenAttacktLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);

		int maxDefendLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		int currentDefendLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk) >= 18 && (currenAttacktLvl < maxAttackLvl || currentDefendLvl < maxDefendLvl))
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(GroundRangeAction, GroundRangeCondition, -1, 24 * 20);


	//zergling attack && defend
	const std::function<void(BWAPI::Game*)> GroundMeleeAction = [=](BWAPI::Game* g)->void
	{
		BWAPI::Unit chamber = NULL;
		std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
		if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == ourBuildings.end())
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->self()->getStartLocation(), 1);
		}

		int maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
		int currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Melee_Attacks) && currentLvl < maxLvl
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Melee_Attacks))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
		}

		maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Carapace) && currentLvl < maxLvl
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Carapace))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
		}
	};
	const std::function<bool(BWAPI::Game*)> GroundMeleeCondition = [=](BWAPI::Game* g)->bool
	{
		int maxAttackLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
		int currenAttacktLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);

		int maxDefendLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
		int currentDefendLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Zergling) >= 24 && (currenAttacktLvl < maxAttackLvl || currentDefendLvl < maxDefendLvl))
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(GroundMeleeAction, GroundMeleeCondition, -1, 24 * 20);


	//mutalisk attack && defend
	const std::function<void(BWAPI::Game*)> AirAction = [=](BWAPI::Game* g)->void
	{
		int maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
		int currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks) && currentLvl < maxLvl
			//&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks.whatsRequired(currentLvl + 1)) > 0
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
		}

		maxLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
		currentLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
		if (!BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace) && currentLvl < maxLvl
			&& !ProductionManager::Instance().IsUpgradeInQueue(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace))
		{
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
		}
	};
	const std::function<bool(BWAPI::Game*)> AirCondition = [=](BWAPI::Game* g)->bool
	{
		int maxAttackLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
		int currenAttacktLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);

		int maxDefendLvl = BWAPI::Broodwar->self()->getMaxUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
		int currentDefendLvl = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) >= 12 && (currenAttacktLvl < maxAttackLvl || currentDefendLvl < maxDefendLvl))
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(AirAction, AirCondition, -1, 24 * 20);

}


std::string	StrategyManager::getCategory(std::vector<std::string>& categoryRange, int curValue, std::string prefix)
{
	std::string innerString = "";
	for (auto item : categoryRange)
	{
		if (item == "max")
		{
			innerString = "max";
			break;
		}
		if (curValue <= std::stoi(item))
		{
			innerString = item;
			break;
		}
	}

	return prefix + innerString;
}


void StrategyManager::calCurrentStateFeature()
{
	//state general
	int curTimeMinuts = BWAPI::Broodwar->getFrameCount() / (24 * 60);
	std::string timeName = getCategory(featureNames["state_general_feature"]["time_"], curTimeMinuts, "time_");
	featureValue["state_general_feature"][timeName] = 1;
	BWAPI::UnitType enemyWork = BWAPI::UnitTypes::None;
	int featureCount = 0;

	std::string enemyRaceName;
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		enemyRaceName = "enemyRace_z";
		featureValue["state_general_feature"]["enemyRace_z"] = 1;
		enemyWork = BWAPI::UnitTypes::Zerg_Drone;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		enemyRaceName = "enemyRace_t";
		featureValue["state_general_feature"]["enemyRace_t"] = 1;
		enemyWork = BWAPI::UnitTypes::Terran_SCV;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		enemyRaceName = "enemyRace_p";
		featureValue["state_general_feature"]["enemyRace_p"] = 1;
		enemyWork = BWAPI::UnitTypes::Protoss_Probe;
	}
	else
	{
		enemyRaceName = "enemyRace_unknow";
		featureValue["state_general_feature"]["enemyRace_unknow"] = 1;
	}
		

	int supplyUsed = BWAPI::Broodwar->self()->supplyUsed() / 2;
	featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["ourSupplyUsed_"], supplyUsed, "ourSupplyUsed_")] = 1;

	if (opponentWinrate != "-1")
		featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["enemyWinPercent_"], stoi(opponentWinrate), "enemyWinPercent_")] = 1;

	int mapWidth = BWAPI::Broodwar->mapWidth();
	featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["mapWidth_"], mapWidth, "mapWidth_")] = 1;

	int mapHeight = BWAPI::Broodwar->mapHeight();
	featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["mapHeight_"], mapHeight, "mapHeight_")] = 1;
	if (InformationManager::Instance().GetEnemyBasePosition() != BWAPI::Positions::None)
	{
		BWAPI::TilePosition enemyBase = BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition());
		double airDistance = enemyBase.getDistance(BWAPI::Broodwar->self()->getStartLocation());
		featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["airDistance_"], int(airDistance), "airDistance_")] = 1;
	}
	std::string mapStr = BWAPI::Broodwar->mapFileName();
	mapStr.erase(std::remove(mapStr.begin(), mapStr.end(), ' '), mapStr.end());
	std::string mapName = "mapName_" + mapStr; //BWAPI::Broodwar->mapFileName();
	featureValue["state_general_feature"][mapName] = 1;

	int ourLarvaCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva);
	featureValue["state_general_feature"][getCategory(featureNames["state_general_feature"]["ourLarvaCount_"], ourLarvaCount, "ourLarvaCount_")] = 1;

	std::string openingName = "openingStrategy_" + std::to_string(int(currentopeningStrategy));
	featureValue["state_general_feature"][openingName] = 1;


	//state tech
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Hive) != ourBuildings.end() && ourBuildings[BWAPI::UnitTypes::Zerg_Hive].size() > 0)
		featureValue["state_tech_feature"]["ourTechLevel_hive"] = 1;
	else if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Lair) != ourBuildings.end() && ourBuildings[BWAPI::UnitTypes::Zerg_Lair].size() > 0)
		featureValue["state_tech_feature"]["ourTechLevel_lair"] = 1;
	else
		featureValue["state_tech_feature"]["ourTechLevel_hatchery"] = 1;

	if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect))
		featureValue["state_tech_feature"]["ourKeyUpgrade_hasLurker"] = 1;
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Pneumatized_Carapace) > 0)
		featureValue["state_tech_feature"]["ourKeyUpgrade_hasLordSpeed"] = 1;
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Ventral_Sacs) > 0)
		featureValue["state_tech_feature"]["ourKeyUpgrade_hasLordLoad"] = 1;
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands) > 0)
		featureValue["state_tech_feature"]["ourKeyUpgrade_zerglingsAttackSpeed"] = 1;


	//staet buildings
	if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Spire) != ourBuildings.end() && ourBuildings[BWAPI::UnitTypes::Zerg_Spire].size() > 0)
		featureValue["state_building_feature"]["ourKeyBuilding_hasSpire"] = 1;
		/*
	if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Hydralisk_Den) != ourBuildings.end() && ourBuildings[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() > 0)
		featureValue["state_building_feature"]["ourKeyBuilding_hasHydraDen"] = 1;
	if (ourBuildings.find(BWAPI::UnitTypes::Zerg_Spawning_Pool) != ourBuildings.end() && ourBuildings[BWAPI::UnitTypes::Zerg_Spawning_Pool].size() > 0)
		featureValue["state_building_feature"]["ourKeyBuilding_hasPool"] = 1;
	if (InformationManager::Instance().GetOurBaseUnit() != NULL)
		featureValue["state_building_feature"]["ourKeyBuilding_hasStartBase"] = 1;
		*/

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& enemyBuildings = InformationManager::Instance().getEnemyAllBuildingUnit();
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spire) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Spire].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasZ_Spire"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hive) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Hive].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasZ_hive"] = 1;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Starport) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Starport].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasT_starPort"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Engineering_Bay) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Engineering_Bay].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasT_engineerBay"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Barracks) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Barracks].size() : 0;
		if (featureCount > 0) featureValue["state_building_feature"][getCategory(featureNames["state_building_feature"]["enemyT_Barracks_"], featureCount, "enemyT_Barracks_")] = 1;
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Factory) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Factory].size() : 0;
		if (featureCount > 0) featureValue["state_building_feature"][getCategory(featureNames["state_building_feature"]["enemyT_Factory_"], featureCount, "enemyT_Factory_")] = 1;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Stargate) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Stargate].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasP_stargate"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Robotics_Facility) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Robotics_Facility].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasP_robotics_facility"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Templar_Archives) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Templar_Archives].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasP_temple"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Fleet_Beacon) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Fleet_Beacon].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasP_fleet_beacon"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Citadel_of_Adun].size() > 0)
			featureValue["state_building_feature"]["enemyKeyBuilding_hasP_Citadel"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Gateway) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Gateway].size() : 0;
		if (featureCount > 0) featureValue["state_building_feature"][getCategory(featureNames["state_building_feature"]["enemyP_Gateway_"], featureCount, "enemyP_Gateway_")] = 1;
	}

	//state economy
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourUnits = InformationManager::Instance().getOurAllBattleUnit();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& enemyUnits = InformationManager::Instance().getEnemyAllBattleUnit();
	std::string mineralFeature = getCategory(featureNames["state_economy_feature"]["ourMineral_"], BWAPI::Broodwar->self()->minerals(), "ourMineral_");
	std::string gasFeature = getCategory(featureNames["state_economy_feature"]["ourGas_"], BWAPI::Broodwar->self()->gas(), "ourGas_");

	featureValue["state_economy_feature"][getCategory(featureNames["state_economy_feature"]["ourMineral_"], BWAPI::Broodwar->self()->minerals(), "ourMineral_")] = 1;
	featureValue["state_economy_feature"][getCategory(featureNames["state_economy_feature"]["ourGas_"], BWAPI::Broodwar->self()->gas(), "ourGas_")] = 1;
	featureValue["state_economy_feature"][getCategory(featureNames["state_economy_feature"]["ourWorkers_"], ourUnits[BWAPI::UnitTypes::Zerg_Drone].size(), "ourWorkers_")] = 1;
	int enemyWorkerCount = enemyUnits.find(enemyWork) != enemyUnits.end() ? enemyUnits[enemyWork].size() : 0;
	if (enemyWorkerCount > 0) featureValue["state_economy_feature"][getCategory(featureNames["state_economy_feature"]["enemyWorkers_"], enemyWorkerCount, "enemyWorkers_")] = 1;
	std::set<BWAPI::Unit>& ourBases = InformationManager::Instance().getOurAllBaseUnit();
	std::set<BWAPI::Unit>& enemyBases = InformationManager::Instance().getEnemyAllBaseUnit();

	std::string ourExpandName = getCategory(featureNames["state_economy_feature"]["ourExpandBase_"], ourBases.size() - 1, "ourExpandBase_");
	std::string enemyExpandName = getCategory(featureNames["state_economy_feature"]["enemyExpandBase_"], enemyBases.size() - 1, "enemyExpandBase_");
	featureValue["state_economy_feature"][ourExpandName] = 1;
	featureValue["state_economy_feature"][enemyExpandName] = 1;


	//state battle
	std::map<BWAPI::UnitType, int> morphUnits;
	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			if (unit->getBuildType() != BWAPI::UnitTypes::Zerg_Overlord && unit->getBuildType() != BWAPI::UnitTypes::Zerg_Drone)
			{
				if (morphUnits.find(unit->getBuildType()) != morphUnits.end())
				{
					morphUnits[unit->getBuildType()] += 1;
				}
				else
				{
					morphUnits[unit->getBuildType()] = 1;
				}
			}
		}
	}

	// state_our_army
	featureCount = ourBuildings.find(BWAPI::UnitTypes::Zerg_Sunken_Colony) != ourBuildings.end() ? ourBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() : 0;
	if (featureCount > 0) featureValue["state_our_army"][getCategory(featureNames["state_our_army"]["ourSunken_"], featureCount, "ourSunken_")] = 1;
	featureCount = ourBuildings.find(BWAPI::UnitTypes::Zerg_Spore_Colony) != ourBuildings.end() ? ourBuildings[BWAPI::UnitTypes::Zerg_Spore_Colony].size() : 0;
	if (featureCount > 0) featureValue["state_our_army"][getCategory(featureNames["state_our_army"]["ourSpore_"], featureCount, "ourSpore_")] = 1;

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Zergling].size() : 0) + (morphUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Zergling] * 2 : 0);
	if (featureCount > 0) featureValue["state_our_army"][getCategory(featureNames["state_our_army"]["ourZergling_"], featureCount, "ourZergling_")] = 1;
	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Mutalisk].size() : 0) + (morphUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Mutalisk] : 0);
	if (featureCount > 0) featureValue["state_our_army"][getCategory(featureNames["state_our_army"]["ourMutalisk_"], featureCount, "ourMutalisk_")] = 1;
	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Hydralisk].size() : 0) + (morphUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Hydralisk] : 0);
	if (featureCount > 0) featureValue["state_our_army"][getCategory(featureNames["state_our_army"]["ourHydra_"], featureCount, "ourHydra_")] = 1;

	// state_enemy
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Zerg_Sunken_Colony) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Sunken_"], featureCount, "enemyZ_Sunken_")] = 1;
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spore_Colony) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Zerg_Spore_Colony].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Spore_"], featureCount, "enemyZ_Spore_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Zergling].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Zergling_"], featureCount, "enemyZ_Zergling_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Mutalisk].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Mutalisk_"], featureCount, "enemyZ_Mutalisk_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Hydralisk].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Hydra_"], featureCount, "enemyZ_Hydra_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Lurker) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Lurker].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyZ_Lurker_"], featureCount, "enemyZ_Lurker_")] = 1;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Missile_Turret) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Missile_Turret].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_missile_"], featureCount, "enemyT_missile_")] = 1;
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Bunker) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Bunker].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Bunker_"], featureCount, "enemyT_Bunker_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Goliath) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Goliath].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Goliath_"], featureCount, "enemyT_Goliath_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Marine) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Marine].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Marine_"], featureCount, "enemyT_Marine_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Vulture) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Vulture].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Vulture_"], featureCount, "enemyT_Vulture_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Dropship) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Dropship].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Dropship_"], featureCount, "enemyT_Dropship_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Valkyrie) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Valkyrie].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Valkyrie_"], featureCount, "enemyT_Valkyrie_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Science_Vessel) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Science_Vessel].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Science_"], featureCount, "enemyT_Science_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Tank_"], featureCount, "enemyT_Tank_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Firebat) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Firebat].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Firebat_"], featureCount, "enemyT_Firebat_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Medic) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Medic].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyT_Terran_Medic_"], featureCount, "enemyT_Terran_Medic_")] = 1;

	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Photon_Cannon) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Photon_Cannon].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_cannon_"], featureCount, "enemyP_cannon_")] = 1;

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Zealot) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Zealot].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_Zealot_"], featureCount, "enemyP_Zealot_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Dragoon) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Dragoon].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_Dragon_"], featureCount, "enemyP_Dragon_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_High_Templar) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_High_Templar].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_High_temple_"], featureCount, "enemyP_High_temple_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Carrier) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Carrier].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_Carrier_"], featureCount, "enemyP_Carrier_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Corsair) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Corsair].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_Corsair_"], featureCount, "enemyP_Corsair_")] = 1;
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Shuttle) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Shuttle].size() : 0;
		if (featureCount > 0) featureValue["state_battle_feature"][getCategory(featureNames["state_battle_feature"]["enemyP_Shuttle_"], featureCount, "enemyP_Shuttle_")] = 1;
	}


	//state combine feature

	for (auto enemyBattleFeature : featureValue["state_battle_feature"])
	{
		for (auto ourBattleFeature : featureValue["state_our_army"])
		{
			std::string combineFeatureName = enemyBattleFeature.first + "*" + ourBattleFeature.first;
			featureValue["state_raw_combine_feature"][combineFeatureName] = 1;
		}
	}

	std::string economyCombine = mineralFeature + "*" + gasFeature;
	featureValue["state_raw_combine_feature"][economyCombine] = 1;

	std::string mapRaceCombine = mapName + "*" + enemyRaceName;
	featureValue["state_raw_combine_feature"][mapRaceCombine] = 1;

	std::string expandBaseCombine = ourExpandName + "*" + enemyExpandName + "*" + mapName;
	featureValue["state_raw_combine_feature"][expandBaseCombine] = 1;

	/*

	for (auto enemyBattleFeature : featureValue["state_battle_feature"])
	{
		std::string combineFeatureName = enemyBattleFeature.first + "*" + timeName;
		featureValue["state_raw_combine_feature"][combineFeatureName] = 1;
	}

	for (auto ourBattleFeature : featureValue["state_our_army"])
	{
		std::string combineFeatureName = ourBattleFeature.first + "*" + timeName;
		featureValue["state_raw_combine_feature"][combineFeatureName] = 1;
	}

	std::string expandBaseCombine = ourExpandName + "*" + enemyExpandName;
	featureValue["state_raw_combine_feature"][expandBaseCombine] = 1;

	std::string openingRaceCombine = openingName + "*" + enemyRaceName;
	featureValue["state_raw_combine_feature"][openingRaceCombine] = 1;*/

}



double StrategyManager::calActionFeature(std::string curAction, std::map<std::string, std::map<std::string, int>>& features)
{
	for (auto categoryStateFeature : features)
	{
		if (categoryStateFeature.first == "state_raw_combine_feature" || categoryStateFeature.first == "state_building_feature")
		{
			for (auto stateFeature : categoryStateFeature.second)
			{
				std::string combineFeatureName = stateFeature.first + "*" + curAction;
				features["action_battle_combine_state_battle_feature"][combineFeatureName] = 1;
			}
		}
	}

	if (features["state_tech_feature"].find("ourKeyUpgrade_zerglingsAttackSpeed") != features["state_tech_feature"].end())
	{
		std::string combineFeatureName = std::string("ourKeyUpgrade_zerglingsAttackSpeed") + "*" + curAction;
		features["action_battle_combine_state_battle_feature"][combineFeatureName] = 1;
	}

	double curQValue = 0;
	for (auto categoryFeature : features)
	{
		for (auto curfeature : categoryFeature.second)
		{
			int curfeatureValue = curfeature.second;
			if (parameterValue.find(categoryFeature.first) != parameterValue.end() && parameterValue[categoryFeature.first].find(curfeature.first) != parameterValue[categoryFeature.first].end())
			{
				double curParameterValue = parameterValue[categoryFeature.first][curfeature.first];
				curQValue += curParameterValue * curfeatureValue;
			}
		}
	}
	return curQValue;

}


//backup value is full return

string	StrategyManager::strategyChange(int reward)
{
	//non-terminal state
	if (reward == 0)
	{
		featureValue.clear();

		//get current state features
		calCurrentStateFeature();

		double maxQValue = -9999999;
		string maxAction = "";
		std::map<std::string, std::map<std::string, int>> maxFeatureValue;
		//select next action
		for (auto action : stateActions)
		{
			std::map<std::string, std::map<std::string, int>> actionFeatureValue = featureValue;
			double curQValue = calActionFeature(action, actionFeatureValue);

			if (curQValue > maxQValue)
			{
				maxQValue = curQValue;
				maxAction = action;
				maxFeatureValue = actionFeatureValue;
			}
		}

		//explore rate start at 70%, grow 5 percent with 50 match, and grow 1 percent with 100 match.
		
		int exploreRate = 0;
		if (curMode == Develop)
		{
			if (playMatchCount > 25 * 8)
			{
				exploreRate = 90;
			}
			else
			{
				exploreRate = 50 + int((float(playMatchCount) / 25) * 5);
			}
		}
		else
		{
			exploreRate = 90;
		}

		//do more explore at training stage to get a reasonable score for all Q(s,a)
		if (std::rand() % 100 > exploreRate)
		{
			std::vector<std::string> actionlist;
			for (auto item : stateActions)
			{
				if (item != maxAction)
				{
					actionlist.push_back(item);
				}
			}
			int randomIndex = std::rand() % actionlist.size();

			//set current features to this explore action feature
			calActionFeature(actionlist[randomIndex], featureValue);

			maxAction = actionlist[randomIndex];
		}
		else
		{
			featureValue = maxFeatureValue;
		}

		bool featureValid = true;

		for (std::map<std::string, std::map<std::string, int>>::iterator it = featureValue.begin(); it != featureValue.end();)
		{
			if (featureNames.find(it->first) == featureNames.end())
			{
				featureValue.erase(it++);
				featureValid = false;
			}
			else
			{
				it++;
			}
		}
		if (featureValid == false)
		{
			fstream lossFile;
			string lossfilePath = "./bwapi-data/write/debug_file_featurevalue";
			lossFile.open(lossfilePath.c_str(), ios::app);
			lossFile << "1" << endl;
			lossFile.close();
			//return maxAction;
		}

		// save current to episode data
		// Q(st-1) | action | '' | reward
		// reward is gathered at the end of episode
		std::vector<std::string> dataVector;
		std::stringstream dataString;
		for (auto categoryField : featureValue)
		{
			for (auto field : categoryField.second)
			{
				if (field.second == 1)
				{
					dataString << categoryField.first << ":" << field.first << ":" << field.second << " ";
				}
			}
		}
		dataVector.push_back(dataString.str());
		dataVector.push_back(maxAction);

		dataString.str("");
		dataVector.push_back(dataString.str());
		dataVector.push_back("");
		curEpisodeData.push_back(dataVector);

		curEpisodeData[curEpisodeData.size() - 1][2] = std::to_string(BWAPI::Broodwar->getFrameCount() / (24 * 60));

		/*
		if (curEpisodeData.size() > 1)
		{
			int curTime = BWAPI::Broodwar->getFrameCount();
			int actionCostTime = curTime - curActionTime;
			curActionTime = curTime;
			double curReward = -1 * float(actionCostTime) / (24 * 60);

			curEpisodeData[curEpisodeData.size() - 2][3] = std::to_string(curReward);
		}
		else
		{
			curActionTime = BWAPI::Broodwar->getFrameCount();
		}*/

		//debug
		//save current predict Q(s,a) to check the model's training status
		if (curMode == Develop)
		{
			fstream actionFile;
			std::string filePath = "./bwapi-data/write/action_data";
			actionFile.open(filePath.c_str(), ios::app);
			double Qvalue = 0;
			for (auto categoryFeature : featureValue)
			{
				for (auto curfeature : categoryFeature.second)
				{
					if (curfeature.second == 1)
					{
						int curfeatureValue = curfeature.second;
						if (parameterValue.find(categoryFeature.first) != parameterValue.end() && parameterValue[categoryFeature.first].find(curfeature.first) != parameterValue[categoryFeature.first].end())
						{
							double curParameterValue = parameterValue[categoryFeature.first][curfeature.first];
							Qvalue += curParameterValue * curfeatureValue;
						}
					}
				}
			}
			actionFile << Qvalue << endl;
			actionFile.close();
		}


		return maxAction;
	}
	else
	{
		if (curEpisodeData.size() <= 1)
		{
			return "";
		}
		curEpisodeData.erase(curEpisodeData.end() - 1);
		
		int endMinut = BWAPI::Broodwar->getFrameCount() / (24 * 60);
		//calculate the discount reward for each action
		/*
		float cumulativeReward = 0;
		for (int i = int(curEpisodeData.size()) - 1; i >= 0; i--)
		{
			//if (i != int(curEpisodeData.size()) - 1)
				//cumulativeReward += std::stof(curEpisodeData[i][3]);
			//double dReward = reward + cumulativeReward;
			int deltaMinutes = endMinut - std::stol(curEpisodeData[i][2]);
			double dReward = int(std::pow(0.95, deltaMinutes) * reward * 100) / 100.0;
			curEpisodeData[i][3] = std::to_string(dReward);
		}*/

		for (int i = 0; i < int(curEpisodeData.size()); i++)
		{
			double dReward = int(std::pow(discountRate, curEpisodeData.size() - 1 - i) * reward * 100000) / 100000.0;
			curEpisodeData[i][3] = std::to_string(dReward);
		}

		/*
		if (curEpisodeData.size() > 50)
		{
			
			int eraseTime = curEpisodeData.size() - 50;
			while (eraseTime > 0)
			{
				int randomIndex = std::rand() % curEpisodeData.size();
				curEpisodeData.erase(curEpisodeData.begin() + randomIndex);
				eraseTime--;
			}
			//curEpisodeData.erase(curEpisodeData.begin(), curEpisodeData.end() -= 30);
		}*/

		experienceData.insert(experienceData.end(), curEpisodeData.begin(), curEpisodeData.end());

		//SGD
		//minibatchsize = 10, doing multi times
		int sgdTime = experienceData.size() < 500 ? 1 : (curEpisodeData.size() > 30 ? 30 : curEpisodeData.size());
		for (int i = 0; i < sgdTime; i++)
		{
			std::map<std::string, std::map<std::string, double>> parameterGradient;
			double loss = 0;
			int miniBatchSize = 5;
			if (experienceData.size() < 50)
			{
				miniBatchSize = 1;//experienceData.size();
			}
			for (int i = 0; i < miniBatchSize; i++)
			{
				int randomIndex = std::rand() % experienceData.size();
				vector<std::string> randomData = experienceData[randomIndex];
				std::map<std::string, std::map<std::string, int>> predictData;
				double delta = 0;
				if (randomData[3] != "0")
				{
					delta = calQValue(randomData[0], predictData) - std::stof(randomData[3]);
					loss += delta * delta;
				}
				else
				{
					std::map<std::string, std::map<std::string, int>> realData;
					delta = calQValue(randomData[0], predictData) - calQValue(randomData[2], realData);
					loss += delta * delta;
				}

				for (auto categoryFeature : predictData)
				{
					for (auto curfeature : categoryFeature.second)
					{
						if (parameterCumulativeGradient.find(categoryFeature.first) != parameterCumulativeGradient.end()
							&& parameterCumulativeGradient[categoryFeature.first].find(curfeature.first) != parameterCumulativeGradient[categoryFeature.first].end())
						{
							parameterCumulativeGradient[categoryFeature.first][curfeature.first] += 1;//delta * delta;
						}
						else
						{
							parameterCumulativeGradient[categoryFeature.first][curfeature.first] = 1;//delta * delta;
						}

						if (parameterGradient.find(categoryFeature.first) != parameterGradient.end()
							&& parameterGradient[categoryFeature.first].find(curfeature.first) == parameterGradient[categoryFeature.first].end())
						{
							parameterGradient[categoryFeature.first][curfeature.first] += delta;
						}
						else
						{
							parameterGradient[categoryFeature.first][curfeature.first] = delta;
						}
					}
				}
			}


			bool featureValid = true;

			for (std::map<std::string, std::map<std::string, double>> ::iterator it = parameterGradient.begin(); it != parameterGradient.end();)
			{
				if (featureNames.find(it->first) == featureNames.end())
				{
					parameterGradient.erase(it++);
					featureValid = false;
				}
				else
				{
					it++;
				}
			}
			if (featureValid == false)
			{
				fstream lossFile;
				string lossfilePath = "./bwapi-data/write/debug_file_featureweight";
				lossFile.open(lossfilePath.c_str(), ios::app);
				lossFile << "1" << endl;
				lossFile.close();
				//return maxAction;
			}

			//Q-learning is a off-policy method, which means the predicted value is always the max Q(s,a)
			//independent of the the chosen action Q(s,a)
			if (curMode == Develop)
			{
				fstream lossFile;
				string lossfilePath = "./bwapi-data/write/loss_file";
				lossFile.open(lossfilePath.c_str(), ios::app);
				lossFile << loss << endl;
				lossFile.close();
			}

			//float learningRate = 0.001 * 1 / float(std::sqrt(playMatchCount));

			for (auto categoryParameter : parameterGradient)
			{
				for (auto parameter : categoryParameter.second)
				{
					//per-coordinate adaptive learning rate
					int featureGdCount = int(parameterCumulativeGradient[categoryParameter.first][parameter.first]);
					double discountLearning = std::pow(4, int(featureGdCount / 500)) * featureGdCount;
					double learningRate = 0.001 / std::sqrt(discountLearning);
					
					//weird bug...
					if (isnan(learningRate))
					{
						fstream lossFile;
						string lossfilePath = "./bwapi-data/write/debug_file";
						lossFile.open(lossfilePath.c_str(), ios::app);
						lossFile << parameter.first << ":" << featureGdCount << ":" << discountLearning << ":" << learningRate << endl;
						lossFile.close();

						//parameterCumulativeGradient[categoryParameter.first][parameter.first] += 1;
						continue;
					}
					
					if (parameterValue.find(categoryParameter.first) != parameterValue.end() && parameterValue[categoryParameter.first].find(parameter.first) != parameterValue[categoryParameter.first].end())
					{
						parameterValue[categoryParameter.first][parameter.first] -= learningRate * parameter.second;
					}
					else
					{
						//parameter is initialized to zero
						parameterValue[categoryParameter.first][parameter.first] = 0 - learningRate * parameter.second;
					}
				}
			}
		}
		
		return "";
	}
}



	


	


//backup value in one step
/*
string	StrategyManager::strategyChange(int reward)
{
	std::map<std::string, std::map<std::string, int>> previousStateFeature = featureValue;
	featureValue.clear();

	double maxQValue = -9999999;
	string maxAction = "";
	std::map<std::string, std::map<std::string, int>> maxFeatureValue;
	//non-terminal state
	if (reward == 0)
	{
		//get current state features
		calCurrentStateFeature();

		//select next action
		for (auto action : stateActions)
		{
			std::map<std::string, std::map<std::string, int>> actionFeatureValue = featureValue;
			double curQValue = calActionFeature(action, actionFeatureValue);
			
			if (curQValue > maxQValue)
			{
				maxQValue = curQValue;
				maxAction = action;
				maxFeatureValue = actionFeatureValue;
			}
		}

		//explore rate start at 70%, grow 5 percent with 50 match, and grow 1 percent with 100 match.
		int exploreRate = 0;
		if (playMatchCount > 50 * 5)
		{
			if (playMatchCount > 50 * 5 + 100 * 4)
			{
				exploreRate = 99;
			}
			else
			{
				exploreRate = 95 + (playMatchCount - 50 * 5) / 100;
			}
		}
		else
		{
			exploreRate = 70 + int((float(playMatchCount) / 50) * 5);
		}

		//do more explore at training stage to get a reasonable score for all Q(s,a)
		if (std::rand() % 100 > exploreRate)
		{
			std::vector<std::string> actionlist;
			for (auto item : stateActions)
			{
				if (item != maxAction)
				{
					actionlist.push_back(item);
				}
			}
			int randomIndex = std::rand() % actionlist.size();

			//set current features to this explore action feature
			calActionFeature(actionlist[randomIndex], featureValue);

			maxAction = actionlist[randomIndex];
		}
		else
		{
			featureValue = maxFeatureValue;
		}
	}

	//for first strategyChange
	if (previousStateFeature.size() == 0)
	{
		previousAction = maxAction;
		return maxAction;
	}

	// save current to experience data
	std::vector<std::string> dataVector;
	std::stringstream dataString;
	for (auto categoryField : previousStateFeature)
	{
		for (auto field : categoryField.second)
		{
			if (field.second == 1)
			{
				dataString << categoryField.first << ":" << field.first << ":" << field.second << " ";
			}
		}
	}
	dataVector.push_back(dataString.str());
	dataVector.push_back(previousAction);

	dataString.str("");
	for (auto categoryField : maxFeatureValue)
	{
		for (auto field : categoryField.second)
		{
			if (field.second == 1)
			{
				dataString << categoryField.first << ":" << field.first << ":" << field.second << " ";
			}
		}
	}
	dataVector.push_back(dataString.str());
	dataVector.push_back(std::to_string(reward));
	experienceData.push_back(dataVector);

	previousAction = maxAction;
	

	//debug
	//save current predict Q(s,a) to check the model's training status
	fstream actionFile;
	std::string filePath = "./bwapi-data/write/action_data";
	actionFile.open(filePath.c_str(), ios::app);
	double Qvalue = 0;
	for (auto categoryFeature : featureValue)
	{
		for (auto curfeature : categoryFeature.second)
		{
			if (curfeature.second == 1)
			{
				int curfeatureValue = curfeature.second;
				if (parameterValue.find(categoryFeature.first) != parameterValue.end() && parameterValue[categoryFeature.first].find(curfeature.first) != parameterValue[categoryFeature.first].end())
				{
					double curParameterValue = parameterValue[categoryFeature.first][curfeature.first];
					Qvalue += curParameterValue * curfeatureValue;
				}
			}
		}
	}
	actionFile << Qvalue << endl;
	actionFile.close();



	//SGD
	//minibatch = 10
	std::map<std::string, std::map<std::string, double>> parameterGradient;
	double loss = 0;
	int miniBatchSize = 10;
	if (experienceData.size() < 10)
	{
		miniBatchSize = 1;
	}
	for (int i = 0; i < miniBatchSize; i++)
	{
		int randomIndex = std::rand() % experienceData.size();
		vector<std::string> randomData = experienceData[randomIndex];
		std::map<std::string, std::map<std::string, int>> predictData;
		double delta = 0;
		if (randomData[3] != "0")
		{
			delta = calQValue(randomData[0], predictData) - std::stoi(randomData[3]);
			loss += delta * delta;
		}
		else
		{
			std::map<std::string, std::map<std::string, int>> realData;
			delta = calQValue(randomData[0], predictData) - calQValue(randomData[2], realData);
			loss += delta * delta;
		}

		for (auto categoryFeature : predictData)
		{
			for (auto curfeature : categoryFeature.second)
			{
				if (parameterGradient[categoryFeature.first].find(curfeature.first) == parameterGradient[categoryFeature.first].end())
				{
					parameterGradient[categoryFeature.first][curfeature.first] = delta;
				}
				else
				{
					parameterGradient[categoryFeature.first][curfeature.first] += delta;
				}
			}
		}
	}

	//Q-learning is a off-policy method, which means the predicted value is always the max Q(s,a)
	//independent of the the chosen action Q(s,a)
	fstream lossFile;
	string lossfilePath = "./bwapi-data/write/loss_file";
	lossFile.open(lossfilePath.c_str(), ios::app);
	lossFile << loss << endl;
	lossFile.close();


	for (auto categoryParameter : parameterGradient)
	{
		for (auto parameter : categoryParameter.second)
		{
			if (parameterValue.find(categoryParameter.first) != parameterValue.end() && parameterValue[categoryParameter.first].find(parameter.first) != parameterValue[categoryParameter.first].end())
			{
				parameterValue[categoryParameter.first][parameter.first] -= 0.001 * parameter.second;
			}
			else
			{
				//parameter is initialized to zero
				parameterValue[categoryParameter.first][parameter.first] = 0 - 0.001 * parameter.second;
			}
		}
	}

	return maxAction;
}*/


double StrategyManager::calQValue(std::string stringData, std::map<std::string, std::map<std::string, int>>& stringValue)
{
	double QValue = 0;
	std::stringstream sd(stringData);
	std::string feature;
	while (getline(sd, feature, ' '))
	{
		if (feature == "")
			continue;
		std::stringstream ss(feature);
		std::vector<string> itemList;
		string item;
		while (getline(ss, item, ':'))
		{
			if (item != "")
				itemList.push_back(item);
		}

		if (parameterValue.find(itemList[0]) != parameterValue.end() && parameterValue[itemList[0]].find(itemList[1]) != parameterValue[itemList[0]].end())
		{
			double curParameterValue = parameterValue[itemList[0]][itemList[1]];
			QValue += curParameterValue;
		}
		stringValue[itemList[0]][itemList[1]] = 1;
	}

	return QValue;
}


void StrategyManager::update()
{
	/*
	if (overlordUpgradeTrigger && BWAPI::Broodwar->getFrameCount() > 12000)
	{
		//overlord speed
		ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Pneumatized_Carapace);
		overlordUpgradeTrigger = false;
	}

	switch (currentStrategy)
	{
	case HydraPush:
	{
		ProductionManager::Instance().setExtractorBuildSpeed(0);
		ProductionManager::Instance().setDroneProductionSpeed(250);

		if (hydraUpgradeTrigger && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk) >= 12)
		{
			hydraUpgradeTrigger = false;
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
		}
	}
		break;
	case MutaPush:
	{
		ProductionManager::Instance().setExtractorBuildSpeed(0);
		ProductionManager::Instance().setDroneProductionSpeed(250);

		if (mutaUpgradeTrigger && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) >= 8)
		{
			mutaUpgradeTrigger = false;
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
		}
		
		std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& enemyBattle = InformationManager::Instance().getEnemyAllBattleUnit();
		int enemyAntiAirSupply = 0;
		for (std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>::iterator it = enemyBattle.begin(); it != enemyBattle.end(); it++)
		{
			if (it->first.airWeapon() != BWAPI::WeaponTypes::None)
			{
				enemyAntiAirSupply += it->first.supplyRequired() * it->second.size();
			}
		}

		// if lost too many mutalisk , and enemy still have many anti-air army, change to hydrisk
		if (enemyAntiAirSupply >= 2 * 1.5 * 12 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk) * 3 < enemyAntiAirSupply)
		{
			goalChange(HydraPush);
		}

	}
		break;
	case ZerglingPush:
	{
		int hatchCompleteCount = 0;
		BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
		{
			if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && u->isCompleted())
			{
				hatchCompleteCount++;
			}
			else if (u->getType() == BWAPI::UnitTypes::Zerg_Lair)
			{
				hatchCompleteCount++;
			}
			else
				continue;
		}
		// slow the drone production speed when only one hatch
		if (hatchCompleteCount == 1)
		{
			ProductionManager::Instance().setDroneProductionSpeed(750);
		}
		else
		{
			ProductionManager::Instance().setDroneProductionSpeed(250);
		}

		ProductionManager::Instance().setExtractorBuildSpeed(15);

		if (BWAPI::Broodwar->self()->gas() >= 100)
		{
			goalChange(MutaPush);
		}
	}
		break;
	default:

		break;
	}*/
}


// get an instance of this
StrategyManager & StrategyManager::Instance()
{
	static StrategyManager instance;
	return instance;
}

std::vector<MetaType> StrategyManager::getOpeningBook()
{
	// according to the game progress, return start/mid/end strategy
	if (selfRace == BWAPI::Races::Zerg)
	{	//12hatch opening 
		//return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 4 6 1 11 11 0 0 0 0 12 12 8 0 5 1 1 0 0 10 10 10 10 10 10 10 1 10 10 10 10 19");

		//overPool opening
		//return getMetaVector("0 0 0 0 0 1 3 0 0 0 2 4 4 4 0 5 0 0 0 6 0 0 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10 10 10 19 20 19 20 19 20");
		
		if (currentopeningStrategy == TwelveHatchMuta)
		{
			//12 hatch mutalisk
			return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 4 4 4");//getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 4 4 4 6");
		}
		else if (currentopeningStrategy == NinePoolling)
		{
			//9 pool zergling
			return getMetaVector("0 0 0 0 0 3 0 5 0 1 4 4 4");
		}
		else if (currentopeningStrategy == TenHatchMuta)
		{
			// 10 hatch counter 2 gate zealot
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4");
		}
		else
		{
			return std::vector<MetaType>();
		}
	}
	else
	{
		return std::vector<MetaType>();
	}
}


int StrategyManager::getScore()
{
	int unitScore = BWAPI::Broodwar->self()->getUnitScore();
	int buildingScore = BWAPI::Broodwar->self()->getBuildingScore();
	int resourceScore = BWAPI::Broodwar->self()->gatheredMinerals() + BWAPI::Broodwar->self()->gatheredGas();
	return unitScore + buildingScore + resourceScore;
}


// need add the upgrade and tech unit type
std::vector<MetaType> StrategyManager::getMetaVector(std::string buildString)
{
	std::stringstream ss;
	ss << buildString;
	std::vector<MetaType> meta;

	int action(0);
	while (ss >> action)
	{
		meta.push_back(actions[action]);
	}
	return meta;
}


