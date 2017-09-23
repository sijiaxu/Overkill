
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
	tmpReward = 0;
	tdStep = 30;

	highLevelActions = decltype(highLevelActions)
	{
		//BattleUnit
		{ "Unit_Zergling", MetaType(BWAPI::UnitTypes::Zerg_Zergling)},
		{ "Unit_Hydralisk", MetaType(BWAPI::UnitTypes::Zerg_Hydralisk) },
		{ "Unit_Mutalisk", MetaType(BWAPI::UnitTypes::Zerg_Mutalisk) },
		{ "Unit_Lurker", MetaType(BWAPI::UnitTypes::Zerg_Lurker) },
		{ "Unit_Scourage", MetaType(BWAPI::UnitTypes::Zerg_Scourge) },
		{ "Unit_Ultralisk", MetaType(BWAPI::UnitTypes::Zerg_Ultralisk) },
		//{ "Unit_Devourer", MetaType(BWAPI::UnitTypes::Zerg_Devourer) },
		//{ "Unit_Guardian", MetaType(BWAPI::UnitTypes::Zerg_Guardian) },

		//Defend
		{ "Defense_Sunken", MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony) },
		//{ "Defense_Spore", MetaType(BWAPI::UnitTypes::Zerg_Spore_Colony) },

		//TechBuildings
		{ "Building_Chamber", MetaType(BWAPI::UnitTypes::Zerg_Evolution_Chamber) },
		{ "Building_Lair", MetaType(BWAPI::UnitTypes::Zerg_Lair) },
		{ "Building_GreaterSpire", MetaType(BWAPI::UnitTypes::Zerg_Greater_Spire) },
		{ "Building_Hive", MetaType(BWAPI::UnitTypes::Zerg_Hive) },
		{ "Building_HydraliskDen", MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den) },
		{ "Building_QueenNest", MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest) },
		{ "Building_SpawningPool", MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool) },
		{ "Building_Spire", MetaType(BWAPI::UnitTypes::Zerg_Spire) },
		{ "Building_UltraliskCavern", MetaType(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern) },

		//TechResearch
		{ "Tech_LurkerTech", MetaType(BWAPI::TechTypes::Lurker_Aspect) },

		//Upgrade
		{ "Upgrade_ZerglingsSpeed", MetaType(BWAPI::UpgradeTypes::Metabolic_Boost) },
		{ "Upgrade_ZerglingsAttackSpeed", MetaType(BWAPI::UpgradeTypes::Adrenal_Glands) },
		{ "Upgrade_OverlordSpeed", MetaType(BWAPI::UpgradeTypes::Pneumatized_Carapace) },
		{ "Upgrade_HydraliskSpeed", MetaType(BWAPI::UpgradeTypes::Muscular_Augments) },
		{ "Upgrade_HydraliskRange", MetaType(BWAPI::UpgradeTypes::Grooved_Spines) },
		{ "Upgrade_UltraliskArmor", MetaType(BWAPI::UpgradeTypes::Chitinous_Plating) },
		{ "Upgrade_UltraliskSpeed", MetaType(BWAPI::UpgradeTypes::Anabolic_Synthesis) },

		//Expand
		{ "Expand_BaseExpand", MetaType(BWAPI::UnitTypes::Zerg_Hatchery) },

		//AttackCommand
		{ "Attack_AllInAttack", MetaType() },
		{ "Attack_MutaliskHarassAttack", MetaType() },

		//Wait
		{ "Wait_doNothing", MetaType() }
	};

	for (auto entry : highLevelActions)
	{
		NNOutputAction.push_back(entry.first);
	}

	for (size_t i = 0; i < NNOutputAction.size(); i++)
	{
		actionsOutputIndexMapping[NNOutputAction[i]] = i;
	}

	for (auto entry : highLevelActions)
	{
		if (entry.first.find("Building_") != std::string::npos
			|| entry.first.find("Expand_") != std::string::npos
			|| entry.first.find("Defense_") != std::string::npos)
		{
			buildingsName[entry.second.unitType] = "building_during_production_" + entry.first;
		}
		else if (entry.first.find("Tech_") != std::string::npos)
		{
			techsName[entry.second.techType] = "building_during_production_" + entry.first;
		}
		else if (entry.first.find("Upgrade_") != std::string::npos)
		{
			upgradesName[entry.second.upgradeType] = "building_during_production_" + entry.first;
		}
		else
		{
			continue;
		}
	}

	lastExpandTime = 0;

	//state feature
	//represent the model input order
	features = decltype(features)
	{
		//state feature
		{ "time_", { "60" } },
		{ "enemyRace_z", { "1" } },
		{ "enemyRace_t", { "1" } },
		{ "enemyRace_p", { "1" } },
		{ "enemyRace_unknow", { "1" } },

		{ "ourSupplyUsed_", { "200" } },
		{ "airDistance_", { "196" } },
		{ "airDistance_unknow", { "1" } },
		{ "groundDistance_", { "196" } },
		{ "groundDistance_unknow", { "1" } },
		{ "mapMinAirDistance_", { "196" } },
		{ "mapMinGroundDistance_", { "196" } },
		{ "mapBaseCount_", { "16" } },

		{ "ourLarvaCount_", { "30" } },
		{ "openingStrategy_0", { "1" } },
		{ "openingStrategy_1", { "1" } },
		{ "openingStrategy_2", { "1" } },


		//tech
		{ "ourKeyUpgrade_LurkerResearch", { "1" } },
		{ "ourKeyUpgrade_ZerglingsAttackSpeed", { "1" } },

		//building
		{ "ourKeyBuilding_Spire", { "1" } },
		{ "ourKeyBuilding_GreaterSpire", { "1" } },
		{ "ourKeyBuilding_HydraliskDen", { "1" } },
		{ "ourKeyBuilding_QueenNest", { "1" } },
		{ "ourKeyBuilding_SpawningPool", { "1" } },
		{ "ourKeyBuilding_UltraliskCavern", { "1" } },
		{ "ourKeyBuilding_Lair", { "1" } },
		{ "ourKeyBuilding_Hive", { "1" } },
		{ "ourHatchery_", { "10" } },
		{ "ourExtractor_", { "8" } },

		{ "enemyKeyBuilding_hasZ_Spire", { "1" } },
		{ "enemyKeyBuilding_hasZ_hive", { "1" } },
		{ "enemyKeyBuilding_hasZ_Lair", { "1" } },
		{ "enemyKeyBuilding_hasZ_HydraliskDen", { "1" } },
		{ "enemyKeyBuilding_hasZ_QueenNest", { "1" } },
		{ "enemyKeyBuilding_hasZ_SpawningPool", { "1" } },

		{ "enemyKeyBuilding_hasT_starPort", { "1" } },
		{ "enemyKeyBuilding_hasT_engineerBay", { "1" } },
		{ "enemyKeyBuilding_hasT_Academy", { "1" } },
		{ "enemyKeyBuilding_hasT_Armory", { "1" } },
		{ "enemyKeyBuilding_hasT_scienceFacility", { "1" } },

		{ "enemyKeyBuilding_hasP_robotics_facility", { "1" } },
		{ "enemyKeyBuilding_hasP_temple", { "1" } },
		{ "enemyKeyBuilding_hasP_fleet_beacon", { "1" } },
		{ "enemyKeyBuilding_hasP_Citadel", { "1" } },
		{ "enemyKeyBuilding_hasP_cybernetics", { "1" } },
		{ "enemyKeyBuilding_hasP_forge", { "1" } },
		{ "enemyKeyBuilding_hasP_observatory", { "1" } },

		{ "enemyZ_Hatchery_", { "8" } },
		{ "enemyP_Gateway_", { "5" } },
		{ "enemyP_Stargate_", { "5" } },
		{ "enemyT_Barracks_", { "5" } },
		{ "enemyT_Factory_", { "5" } },

		//economy
		{ "ourMineral_", { "2000" } },
		{ "ourGas_", { "2000" } },
		{ "ourExpandBase_", { "6" } },
		{ "ourWorkers_", { "80" } },
		{ "ourMineralWorkers_", { "60" } },
		{ "ourMineralLeft_", { "60" } },
		{ "enemyExpandBase_", { "6" } },
		{ "enemyWorkers_", { "80" } },


		//battleUnit
		//self
		{ "ourSunken_", { "12" } },
		{ "ourSpore_", { "12" } },
		{ "ourZergling_", { "120" } },
		{ "ourMutalisk_", { "36" } },
		{ "ourHydra_", { "60" } },
		{ "ourLurker_", { "20" } },
		{ "ourScourage_", { "36" } },
		{ "ourUltralisk_", { "20" } },

		//self unit in egg
		{ "ourEggZergling_", { "40" } },
		{ "ourEggMutalisk_", { "24" } },
		{ "ourEggHydra_", { "24" } },
		{ "ourEggLurker_", { "12" } },
		{ "ourEggScourage_", { "24" } },
		{ "ourEggUltralisk_", { "12" } },

		//enemy
		{ "enemyP_cannon_", { "12" } },
		{ "enemyT_missile_", { "12" } },
		{ "enemyT_Bunker_", { "12" } },
		{ "enemyZ_Sunken_", { "12" } },
		{ "enemyZ_Spore_", { "12" } },

		{ "enemyZ_Zergling_", { "120" } },
		{ "enemyZ_Mutalisk_", { "36" } },
		{ "enemyZ_Hydra_", { "48" } },
		{ "enemyZ_Lurker_", { "20" } },
		{ "enemyZ_Scourage", { "36" } },
		{ "enemyZ_Ultralisk", { "20" } },
		{ "enemyZ_Defiler", { "12" } },

		{ "enemyP_Zealot_", { "36" } },
		{ "enemyP_Dragon_", { "36" } },
		{ "enemyP_High_temple_", { "12" } },
		{ "enemyP_Carrier_", { "24" } },
		{ "enemyP_Corsair_", { "30" } },
		{ "enemyP_Shuttle_", { "9" } },

		{ "enemyT_Goliath_", { "30" } },
		{ "enemyT_Marine_", { "36" } },
		{ "enemyT_Tank_", { "30" } },
		{ "enemyT_Vulture_", { "30" } },
		{ "enemyT_Dropship_", { "9" } },
		{ "enemyT_Valkyrie_", { "18" } },
		{ "enemyT_Science_", { "9" } },
		{ "enemyT_Firebat_", { "30" } },
		{ "enemyT_Terran_Medic_", { "12" } },

		//during production features
		{ "building_during_production_Defense_Sunken", { "10" } },
		//{ "building_during_production_Defense_Spore", { "10" } },
		{ "execute_Attack_AllInAttack", { "1" } },
		{ "execute_Attack_MutaliskHarassAttack", { "1" } }
	};

	for (auto entry : highLevelActions)
	{
		if (entry.first.find("Building_") != std::string::npos
			|| entry.first.find("Expand_") != std::string::npos
			|| entry.first.find("Tech_") != std::string::npos
			|| entry.first.find("Upgrade_") != std::string::npos)
		{
			features["building_during_production_" + entry.first] = { "1" };
		}
	}

	for (auto entry : features)
	{
		NNInputAction.push_back(entry.first);
	}

	/*
	if (policyModel.deserialize("policy") == -1)
	{
		BWAPI::Broodwar->printf("policy model first init!");
		// neural network initial
		policyModel.setDepth(3);
		policyModel.setNodeCountAtLevel(200, 0, Relu);
		policyModel.setNodeCountAtLevel(100, 1, Relu);
		policyModel.setNodeCountAtLevel(NNOutputAction.size(), 2, SOFTMAX);
		policyModel.setNumInputs(NNInputAction.size());
		policyModel.linkNeurons();
		policyModel.setLearningRate(0.001);
		policyModel.initNetwork();
	}
	*/
	string enemyName = BWAPI::Broodwar->enemy()->getName();
	if (QValueModel.deserialize(enemyName + "_QValue", readResourceFolder) == -1)
	{
		if (QValueModel.deserialize(enemyName + "_QValue", initReadResourceFolder) == -1)
		{
			BWAPI::Broodwar->printf("value model first init!");
			// linear model initial
			QValueModel.setDepth(3);
			QValueModel.setNodeCountAtLevel(100, 0, Relu);
			QValueModel.setNodeCountAtLevel(80, 1, Relu);
			QValueModel.setNodeCountAtLevel(NNOutputAction.size(), 2, SUM);
			QValueModel.setNumInputs(NNInputAction.size());
			QValueModel.linkNeurons();
			QValueModel.setLearningRate(0.00025);
			QValueModel.initNetwork();
		}
	}
	if (targetQValueModel.deserialize(enemyName + "_targetQValueModel", readResourceFolder) == -1)
	{
		if (targetQValueModel.deserialize(enemyName + "_targetQValueModel", initReadResourceFolder) == -1)
		{
			BWAPI::Broodwar->printf("target value model first init!");
			targetQValueModel.copy(QValueModel);
		}
	}

	//QValueModel.setLearningRate(0.00025);
	
	

	previousAction = "";

	//add upgrade callback check
	//setArmyUpgrade();

	discountRate = 0.99;
	
	muteBuilding = true;

	isInBuildingMutalisk = false;
	nextDefenseBuildTime = 0;
	replayLength = 6;
	replayData.setCapacity(10000);

	experienceDataInit();
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

void StrategyManager::setReward(BWAPI::UnitType u, bool addReward) 
{
	if (u == BWAPI::UnitTypes::Zerg_Lair || u == BWAPI::UnitTypes::Zerg_Hive)
	{
		return;
	}

	if (addReward)
	{
		if (u.isWorker())
		{
			tmpReward += double(u.supplyRequired()) / 20;
		}
		//expand reward
		else if (u.isResourceDepot())
		{
			tmpReward += 0.5;
		}
		else if (u == BWAPI::UnitTypes::Zerg_Overlord)
		{
			tmpReward += 0.1;
		}
		else if (u.isBuilding() && (u.groundWeapon() != BWAPI::WeaponTypes::None || u.airWeapon() != BWAPI::WeaponTypes::None ||
			u == BWAPI::UnitTypes::Terran_Bunker))
		{
			tmpReward += 0.15;
		}
		//encourage kill unit
		else
		{
			tmpReward += double(u.supplyRequired()) / 40;
		}
	}
	
	else
	{
		if (u.isWorker())
		{
			tmpReward -= double(u.supplyRequired()) / 20;
		}
		//more penalty on lost base
		else if (u.isResourceDepot())
		{
			tmpReward -= 0.5;
		}
		else if (u == BWAPI::UnitTypes::Zerg_Overlord)
		{
			tmpReward -= 0.1;
		}
		
		//less penalty on loss
		else
		{
			tmpReward -= double(u.supplyRequired()) / 80;
		}
		
	}
	
	
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
	string enemyName = BWAPI::Broodwar->enemy()->getName();
	filePath = writeResourceFolder + "RL_data_";
	filePath += enemyName;

	fstream historyModel;
	historyModel.open(filePath.c_str(), ios::out);
	std::vector<std::vector<std::string>> subData;

	vector<treeReturnData> experienceData = replayData.getData();

	/*
	//save test dataSet
	if (experienceData.size() > 1000)
	{
		filePath = "./bwapi-data/write/testSetData";
		fstream test;
		test.open(filePath.c_str(), ios::in);
		//if do not has this file, init
		if (!test.is_open())
		{
			fstream testSetData;
			testSetData.open(filePath.c_str(), ios::out);
			int count = 0;
			for (auto tmp : experienceData)
			{
				vector<string> rlData = tmp.second;
				count += 1;
				if (count % 5 != 0)
				{
					continue;
				}
				for (size_t i = 0; i < rlData.size(); i++)
				{
					testSetData << rlData[i];
					if (i != rlData.size() - 1)
					{
						testSetData << "|";
					}
				}
				testSetData << endl;
			}
			testSetData.close();
		}
	}
	*/


	for (auto tmp : experienceData)
	{
		vector<string> rlData = tmp.data;
		for (size_t i = 0; i < rlData.size(); i++)
		{
			historyModel << rlData[i];
			if (i != rlData.size() - 1)
			{
				historyModel << "|";
			}
		}
		historyModel << "|" << tmp.priority << endl;
	}
	historyModel.close();

	/*
	//save for offline test training
	if (trainingData.size() > 0)
	{
		fstream historyFile;
		string filePath = "./bwapi-data/total_match";
		historyFile.open(filePath.c_str(), ios::app);
		for (size_t instance = 0; instance < trainingData.size(); instance++)
		{
			for (size_t i = 0; i < trainingData[instance].size(); i++)
			{
				historyFile << trainingData[instance][i];
				if (i != trainingData.size() - 1)
				{
					historyFile << "|";
				}
			}
			historyFile << "&";
		}
		historyFile << endl;
		historyFile.close();
	}

	// save test set performance data
	if (testSetData.size() > 0)
	{
		fstream historyFile;
		string filePath = "./bwapi-data/write/performace_data";
		historyFile.open(filePath.c_str(), ios::app);
		historyFile << testSetAvgQValue;
		historyFile << endl;
		historyFile.close();
	}
	*/
}


void StrategyManager::featureWeightSave()
{
	/*
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
	*/
}


void StrategyManager::featureGradientSave()
{
	/*
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
	*/
}


void StrategyManager::experienceDataInit()
{
	string filePath;
	string enemyName = BWAPI::Broodwar->enemy()->getName();
	filePath = readResourceFolder + "RL_data_";
	filePath += enemyName;
	
	fstream historyModel;

	historyModel.open(filePath.c_str(), ios::in);

	if (historyModel.is_open())
	{
		string content;
		while (getline(historyModel, content))
		{
			if (content == "")
				continue;
			std::stringstream instance(content);
			string entry;
			std::vector<string> itemList;
			while (getline(instance, entry, '|'))
			{
				itemList.push_back(entry);
			}
			double priority = std::stod(itemList.back());
			itemList.pop_back();

			if (itemList.size() != replayLength)
			{
				continue;
			}

			replayData.add(priority, itemList);
		}
	}
	else
	{
		BWAPI::Broodwar->printf("do not has experience data !");
	}
	historyModel.close();

	//get the total match count have player
	filePath = readResourceFolder + "match_count_";
	filePath += enemyName;

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
	/*
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
	*/
}


void StrategyManager::featureCumulativeGradientInit()
{
	/*
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
	*/
}


void StrategyManager::getStrategyBuildingOrder()
{
	if (curBuildingAction == "lurkerBuild")
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) == false && 
			BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect) == false)
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) == 0 &&
				BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) == 0)
				ProductionManager::Instance().triggerBuildingOrder(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self()->getStartLocation(), curBuildingAction);

			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0 &&
				BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0)
				ProductionManager::Instance().triggerBuildingOrder(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self()->getStartLocation(), curBuildingAction);

			const std::function<void(BWAPI::Game*)> lurkerAction = [=](BWAPI::Game* g)->void
			{
				ProductionManager::Instance().triggerTech(BWAPI::TechTypes::Lurker_Aspect);
			};
			const std::function<bool(BWAPI::Game*)> luerkerCondition = [=](BWAPI::Game* g)->bool
			{
				if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 1
					&& BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) == 1)
					return true;
				else
					return false;
			};
			BWAPI::Broodwar->registerEvent(lurkerAction, luerkerCondition, 1, 48);
		}
	}
	else
	{
		BWAPI::UnitType targetBuilding = actionBuildingTarget[curBuildingAction].second;
		if (BWAPI::Broodwar->self()->completedUnitCount(targetBuilding) == 0 &&
			BWAPI::Broodwar->self()->incompleteUnitCount(targetBuilding) == 0)
		{
			ProductionManager::Instance().triggerBuildingOrder(targetBuilding, BWAPI::Broodwar->self()->getStartLocation(), curBuildingAction);
		}
	}
}


std::vector<MetaType> StrategyManager::getCurrentStrategyUnit()
{
	std::vector<MetaType> buildingOrder;
	BWAPI::UnitType targetType = actionBuildingTarget[curBuildingAction].first;
	buildingOrder.insert(buildingOrder.end(), 1, MetaType(targetType, curBuildingAction));
	/*
	
	if (targetType == BWAPI::UnitTypes::Zerg_Lurker)
	{
		buildingOrder.insert(buildingOrder.end(), 1, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk, curBuildingAction));
		buildingOrder.insert(buildingOrder.end(), 1, MetaType(targetType, curBuildingAction));
	}
	else if (targetType == BWAPI::UnitTypes::Zerg_Devourer || targetType == BWAPI::UnitTypes::Zerg_Guardian)
	{
		buildingOrder.insert(buildingOrder.end(), 1, MetaType(BWAPI::UnitTypes::Zerg_Mutalisk, curBuildingAction));
		buildingOrder.insert(buildingOrder.end(), 1, MetaType(targetType, curBuildingAction));
	}
	else
	{
		buildingOrder.insert(buildingOrder.end(), 1, MetaType(targetType, curBuildingAction));
	}
	*/
	return buildingOrder;
}


void StrategyManager::setArmyUpgrade()
{
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


double StrategyManager::getNormalizedValue(std::vector<std::string>& categoryRange, int curValue)
{
	int rangeValue = std::stoi(categoryRange[0]);
	//scale to [-1, 1] range
	return (curValue / double(rangeValue)) * 2 - 1;
}


void StrategyManager::calCurrentStateFeature()
{
	//state general
	int curTimeMinuts = BWAPI::Broodwar->getFrameCount() / (24 * 60);
	featureValues["time_"] = getNormalizedValue(features["time_"], curTimeMinuts);
	BWAPI::UnitType enemyWork = BWAPI::UnitTypes::None;
	int featureCount = 0;

	std::string enemyRaceName;
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		enemyRaceName = "enemyRace_z";
		featureValues["enemyRace_z"] = 1;
		enemyWork = BWAPI::UnitTypes::Zerg_Drone;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		enemyRaceName = "enemyRace_t";
		featureValues["enemyRace_t"] = 1;
		enemyWork = BWAPI::UnitTypes::Terran_SCV;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		enemyRaceName = "enemyRace_p";
		featureValues["enemyRace_p"] = 1;
		enemyWork = BWAPI::UnitTypes::Protoss_Probe;
	}
	else
	{
		enemyRaceName = "enemyRace_unknow";
		featureValues["enemyRace_unknow"] = 1;
	}
	int supplyUsed = BWAPI::Broodwar->self()->supplyUsed() / 2;
	featureValues["ourSupplyUsed_"] = getNormalizedValue(features["ourSupplyUsed_"], supplyUsed);

	std::map<BWTA::Region*, int>& baseGroundDistanceInfo = InformationManager::Instance().getBaseGroudDistance();
	std::map<BWTA::Region*, int>& baseAirDistanceInfo = InformationManager::Instance().getBaseAirDistance();

	if (InformationManager::Instance().GetEnemyBasePosition() != BWAPI::Positions::None)
	{
		int airDistance = baseAirDistanceInfo[BWTA::getRegion(InformationManager::Instance().GetEnemyBasePosition())];
		featureValues["airDistance_"] = getNormalizedValue(features["airDistance_"], airDistance);
		int groundDistance = baseGroundDistanceInfo[BWTA::getRegion(InformationManager::Instance().GetEnemyBasePosition())];
		featureValues["groundDistance_"] = getNormalizedValue(features["groundDistance_"], groundDistance);
	}
	else
	{
		featureValues["airDistance_unknow"] = 1;
		featureValues["groundDistance_unknow"] = 1;
		int minDistance = 99999;
		for (auto entry : baseAirDistanceInfo)
		{
			if (entry.second < minDistance)
			{
				minDistance = entry.second;
			}
		}
		featureValues["mapMinAirDistance_"] = getNormalizedValue(features["mapMinAirDistance_"], minDistance);
		minDistance = 99999;
		for (auto entry : baseGroundDistanceInfo)
		{
			if (entry.second < minDistance)
			{
				minDistance = entry.second;
			}
		}
		featureValues["mapMinGroundDistance_"] = getNormalizedValue(features["mapMinGroundDistance_"], minDistance);
	}

	featureValues["mapBaseCount_"] = getNormalizedValue(features["mapBaseCount_"], int(BWTA::getBaseLocations().size()));
	int ourLarvaCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva);
	featureValues["ourLarvaCount_"] = getNormalizedValue(features["ourLarvaCount_"], ourLarvaCount);
	std::string openingName = "openingStrategy_" + std::to_string(int(currentopeningStrategy));
	featureValues[openingName] = 1;


	//state tech
	if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect))
		featureValues["ourKeyUpgrade_LurkerResearch"] = 1;
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands) > 0)
		featureValues["ourKeyUpgrade_ZerglingsAttackSpeed"] = 1;


	//state buildings
	//self buildings
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spire) > 0)
		featureValues["ourKeyBuilding_Spire"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
	{
		featureValues["ourKeyBuilding_GreaterSpire"] = 1;
		featureValues["ourKeyBuilding_Spire"] = 1;
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) > 0)
		featureValues["ourKeyBuilding_HydraliskDen"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Queens_Nest) > 0)
		featureValues["ourKeyBuilding_QueenNest"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) > 0)
		featureValues["ourKeyBuilding_SpawningPool"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern) > 0)
		featureValues["ourKeyBuilding_UltraliskCavern"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) > 0)
		featureValues["ourKeyBuilding_Lair"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
	{
		featureValues["ourKeyBuilding_Hive"] = 1;
		featureValues["ourKeyBuilding_Lair"] = 1;
	}
	int hatcheryCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) +
		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Lair) +
		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	featureValues["ourHatchery_"] = getNormalizedValue(features["ourHatchery_"], hatcheryCount);
	featureValues["ourExtractor_"] = getNormalizedValue(features["ourExtractor_"], BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Extractor));

	//enemy buildings
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& enemyBuildings = InformationManager::Instance().getEnemyAllBuildingUnit();
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		int enemyHatcheryCount = 0;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spire) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Spire].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_Spire"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hive) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Hive].size() > 0)
		{
			featureValues["enemyKeyBuilding_hasZ_hive"] = 1;
			featureValues["enemyKeyBuilding_hasZ_Lair"] = 1;
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Hive].size();
		}
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Lair) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Lair].size() > 0)
		{
			featureValues["enemyKeyBuilding_hasZ_Lair"] = 1;
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Lair].size();
		}
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hydralisk_Den) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_HydraliskDen"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Queens_Nest) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Queens_Nest].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_QueenNest"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spawning_Pool) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Spawning_Pool].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_SpawningPool"] = 1;

		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hatchery) != enemyBuildings.end())
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Hatchery].size();
		featureValues["enemyZ_Hatchery_"] = getNormalizedValue(features["enemyZ_Hatchery_"], enemyHatcheryCount);

	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Starport) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Starport].size() > 0)
			featureValues["enemyKeyBuilding_hasT_starPort"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Engineering_Bay) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Engineering_Bay].size() > 0)
			featureValues["enemyKeyBuilding_hasT_engineerBay"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Academy) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Academy].size() > 0)
			featureValues["enemyKeyBuilding_hasT_Academy"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Armory) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Armory].size() > 0)
			featureValues["enemyKeyBuilding_hasT_Armory"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Science_Facility) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Science_Facility].size() > 0)
			featureValues["enemyKeyBuilding_hasT_scienceFacility"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Barracks) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Barracks].size() : 0;
		featureValues["enemyT_Barracks_"] = getNormalizedValue(features["enemyT_Barracks_"], featureCount);

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Factory) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Factory].size() : 0;
		featureValues["enemyT_Factory_"] = getNormalizedValue(features["enemyT_Factory_"], featureCount);
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Robotics_Facility) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Robotics_Facility].size() > 0)
			featureValues["enemyKeyBuilding_hasP_robotics_facility"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Templar_Archives) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Templar_Archives].size() > 0)
			featureValues["enemyKeyBuilding_hasP_temple"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Fleet_Beacon) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Fleet_Beacon].size() > 0)
			featureValues["enemyKeyBuilding_hasP_fleet_beacon"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Citadel_of_Adun].size() > 0)
			featureValues["enemyKeyBuilding_hasP_Citadel"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Cybernetics_Core) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Cybernetics_Core].size() > 0)
			featureValues["enemyKeyBuilding_hasP_cybernetics"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Forge) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Forge].size() > 0)
			featureValues["enemyKeyBuilding_hasP_forge"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Observatory) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Observatory].size() > 0)
			featureValues["enemyKeyBuilding_hasP_observatory"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Stargate) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Stargate].size() : 0;
		featureValues["enemyP_Stargate_"] = getNormalizedValue(features["enemyP_Stargate_"], featureCount);
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Gateway) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Gateway].size() : 0;
		featureValues["enemyP_Gateway_"] = getNormalizedValue(features["enemyP_Gateway_"], featureCount);
	}

	//state economy
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourUnits = InformationManager::Instance().getOurAllBattleUnit();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& enemyUnits = InformationManager::Instance().getEnemyAllBattleUnit();

	featureValues["ourMineral_"] = getNormalizedValue(features["ourMineral_"], BWAPI::Broodwar->self()->minerals());
	featureValues["ourGas_"] = getNormalizedValue(features["ourGas_"], BWAPI::Broodwar->self()->gas());
	featureValues["ourWorkers_"] = getNormalizedValue(features["ourWorkers_"], ourUnits[BWAPI::UnitTypes::Zerg_Drone].size());
	int enemyWorkerCount = enemyUnits.find(enemyWork) != enemyUnits.end() ? enemyUnits[enemyWork].size() : 0;
	featureValues["enemyWorkers_"] = getNormalizedValue(features["enemyWorkers_"], enemyWorkerCount);

	std::set<BWAPI::Unit>& ourBases = InformationManager::Instance().getOurAllBaseUnit();
	std::set<BWAPI::Unit>& enemyBases = InformationManager::Instance().getEnemyAllBaseUnit();

	featureValues["ourExpandBase_"] = getNormalizedValue(features["ourExpandBase_"], ourBases.size() - 1);
	featureValues["enemyExpandBase_"] = getNormalizedValue(features["enemyExpandBase_"], enemyBases.size() - 1);

	int mineralWorkerCount = WorkerManager::Instance().getNumMineralWorkers();
	featureValues["ourMineralWorkers_"] = getNormalizedValue(features["ourMineralWorkers_"], mineralWorkerCount);

	std::set<BWTA::Region *>& ourRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	int mineralLeft = 0;
	for (auto m : BWAPI::Broodwar->getMinerals())
	{
		if (ourRegions.find(BWTA::getRegion(m->getPosition())) != ourRegions.end())
		{
			mineralLeft++;
		}
	}
	featureValues["ourMineralLeft_"] = getNormalizedValue(features["ourMineralLeft_"], mineralLeft);

	//state battle
	morphUnits.clear();
	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		{
			if (unit->getBuildType() != BWAPI::UnitTypes::Zerg_Overlord && unit->getBuildType() != BWAPI::UnitTypes::Zerg_Drone)
			{
				if (morphUnits.find(unit->getBuildType()) == morphUnits.end())
				{
					morphUnits[unit->getBuildType()] = 0;
				}

				if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Zergling || unit->getBuildType() == BWAPI::UnitTypes::Zerg_Scourge)
				{
					morphUnits[unit->getBuildType()] += 2;
				}
				else
				{
					morphUnits[unit->getBuildType()] += 1;
				}
			}
		}
	}

	// state_our_army
	featureCount = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony);
	featureValues["ourSunken_"] = getNormalizedValue(features["ourSunken_"], featureCount);
	featureCount = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spore_Colony);
	featureValues["ourSpore_"] = getNormalizedValue(features["ourSpore_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Zergling].size() : 0);
	featureValues["ourZergling_"] = getNormalizedValue(features["ourZergling_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Zergling] * 2 : 0;
	featureValues["ourEggZergling_"] = getNormalizedValue(features["ourEggZergling_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Mutalisk].size() : 0);
	featureValues["ourMutalisk_"] = getNormalizedValue(features["ourMutalisk_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Mutalisk] : 0;
	featureValues["ourEggMutalisk_"] = getNormalizedValue(features["ourEggMutalisk_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Hydralisk].size() : 0);
	featureValues["ourHydra_"] = getNormalizedValue(features["ourHydra_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Hydralisk] : 0;
	featureValues["ourEggHydra_"] = getNormalizedValue(features["ourEggHydra_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Lurker) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Lurker].size() : 0);
	featureValues["ourLurker_"] = getNormalizedValue(features["ourLurker_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Lurker) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Lurker] : 0;
	featureValues["ourEggLurker_"] = getNormalizedValue(features["ourEggLurker_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Scourge) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Scourge].size() : 0);
	featureValues["ourScourage_"] = getNormalizedValue(features["ourScourage_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Scourge) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Scourge] : 0;
	featureValues["ourEggScourage_"] = getNormalizedValue(features["ourEggScourage_"], featureCount);

	featureCount = (ourUnits.find(BWAPI::UnitTypes::Zerg_Ultralisk) != ourUnits.end() ? ourUnits[BWAPI::UnitTypes::Zerg_Ultralisk].size() : 0);
	featureValues["ourUltralisk_"] = getNormalizedValue(features["ourUltralisk_"], featureCount);
	featureCount = morphUnits.find(BWAPI::UnitTypes::Zerg_Ultralisk) != morphUnits.end() ? morphUnits[BWAPI::UnitTypes::Zerg_Ultralisk] : 0;
	featureValues["ourEggUltralisk_"] = getNormalizedValue(features["ourEggUltralisk_"], featureCount);

	// state_enemy
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Zerg_Sunken_Colony) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() : 0;
		featureValues["enemyZ_Sunken_"] = getNormalizedValue(features["enemyZ_Sunken_"], featureCount);
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spore_Colony) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Zerg_Spore_Colony].size() : 0;
		featureValues["enemyZ_Spore_"] = getNormalizedValue(features["enemyZ_Spore_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Zergling) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Zergling].size() : 0;
		featureValues["enemyZ_Zergling_"] = getNormalizedValue(features["enemyZ_Zergling_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Mutalisk) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Mutalisk].size() : 0;
		featureValues["enemyZ_Mutalisk_"] = getNormalizedValue(features["enemyZ_Mutalisk_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Hydralisk) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Hydralisk].size() : 0;
		featureValues["enemyZ_Hydra_"] = getNormalizedValue(features["enemyZ_Hydra_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Lurker) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Lurker].size() : 0;
		featureValues["enemyZ_Lurker_"] = getNormalizedValue(features["enemyZ_Lurker_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Scourge) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Scourge].size() : 0;
		featureValues["enemyZ_Scourage"] = getNormalizedValue(features["enemyZ_Scourage"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Ultralisk) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Ultralisk].size() : 0;
		featureValues["enemyZ_Ultralisk"] = getNormalizedValue(features["enemyZ_Ultralisk"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Zerg_Defiler) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Zerg_Defiler].size() : 0;
		featureValues["enemyZ_Defiler"] = getNormalizedValue(features["enemyZ_Defiler"], featureCount);
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Missile_Turret) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Missile_Turret].size() : 0;
		featureValues["enemyT_missile_"] = getNormalizedValue(features["enemyT_missile_"], featureCount);
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Bunker) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Bunker].size() : 0;
		featureValues["enemyT_Bunker_"] = getNormalizedValue(features["enemyT_Bunker_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Goliath) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Goliath].size() : 0;
		featureValues["enemyT_Goliath_"] = getNormalizedValue(features["enemyT_Goliath_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Marine) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Marine].size() : 0;
		featureValues["enemyT_Marine_"] = getNormalizedValue(features["enemyT_Marine_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Vulture) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Vulture].size() : 0;
		featureValues["enemyT_Vulture_"] = getNormalizedValue(features["enemyT_Vulture_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Dropship) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Dropship].size() : 0;
		featureValues["enemyT_Dropship_"] = getNormalizedValue(features["enemyT_Dropship_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Valkyrie) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Valkyrie].size() : 0;
		featureValues["enemyT_Valkyrie_"] = getNormalizedValue(features["enemyT_Valkyrie_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Science_Vessel) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Science_Vessel].size() : 0;
		featureValues["enemyT_Science_"] = getNormalizedValue(features["enemyT_Science_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode].size() : 0;
		featureValues["enemyT_Tank_"] = getNormalizedValue(features["enemyT_Tank_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Firebat) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Firebat].size() : 0;
		featureValues["enemyT_Firebat_"] = getNormalizedValue(features["enemyT_Firebat_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Terran_Medic) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Terran_Medic].size() : 0;
		featureValues["enemyT_Terran_Medic_"] = getNormalizedValue(features["enemyT_Terran_Medic_"], featureCount);

	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Photon_Cannon) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Photon_Cannon].size() : 0;
		featureValues["enemyP_cannon_"] = getNormalizedValue(features["enemyP_cannon_"], featureCount);

		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Zealot) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Zealot].size() : 0;
		featureValues["enemyP_Zealot_"] = getNormalizedValue(features["enemyP_Zealot_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Dragoon) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Dragoon].size() : 0;
		featureValues["enemyP_Dragon_"] = getNormalizedValue(features["enemyP_Dragon_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_High_Templar) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_High_Templar].size() : 0;
		featureValues["enemyP_High_temple_"] = getNormalizedValue(features["enemyP_High_temple_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Carrier) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Carrier].size() : 0;
		featureValues["enemyP_Carrier_"] = getNormalizedValue(features["enemyP_Carrier_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Corsair) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Corsair].size() : 0;
		featureValues["enemyP_Corsair_"] = getNormalizedValue(features["enemyP_Corsair_"], featureCount);
		featureCount = enemyUnits.find(BWAPI::UnitTypes::Protoss_Shuttle) != enemyUnits.end() ? enemyUnits[BWAPI::UnitTypes::Protoss_Shuttle].size() : 0;
		featureValues["enemyP_Shuttle_"] = getNormalizedValue(features["enemyP_Shuttle_"], featureCount);
	}

	for (auto entry : buildingsUnderProcess)
	{
		if (entry.first != BWAPI::UnitTypes::Zerg_Sunken_Colony
			&& entry.first != BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			featureValues[buildingsName[entry.first]] = 1;
		}
	}
	if (buildingsUnderProcess.find(BWAPI::UnitTypes::Zerg_Sunken_Colony) != buildingsUnderProcess.end())
	{
		featureValues["building_during_production_Defense_Sunken"] = getNormalizedValue(features["building_during_production_Defense_Sunken"], buildingsUnderProcess[BWAPI::UnitTypes::Zerg_Sunken_Colony].first);
	}

	//if (buildingsUnderProcess.find(BWAPI::UnitTypes::Zerg_Spore_Colony) != buildingsUnderProcess.end())
	//{
		//featureValues["building_during_production_Defense_Spore"] = getNormalizedValue(features["building_during_production_Defense_Spore"], buildingsUnderProcess[BWAPI::UnitTypes::Zerg_Spore_Colony].first);
	//}

	for (auto entry : techUnderProcess)
	{
		featureValues[techsName[entry.first]] = 1;
	}
	for (auto entry : upgradeUnderProcess)
	{
		featureValues[upgradesName[entry.first]] = 1;
	}

	if (TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
	{
		featureValues["execute_Attack_MutaliskHarassAttack"] = 1;
	}
	if (TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
	{
		featureValues["execute_Attack_AllInAttack"] = 1;
	}
}


void StrategyManager::buildingFinish(BWAPI::UnitType u)
{
	if (buildingsUnderProcess.find(u) != buildingsUnderProcess.end())
	{
		buildingsUnderProcess[u].first = buildingsUnderProcess[u].first - 1;
		if (buildingsUnderProcess[u].first == 0)
		{
			buildingsUnderProcess.erase(u);
		}
	}
}

bool StrategyManager::isGasRequireMeet(int price)
{
	if (price > 0
		&& WorkerManager::Instance().getNumGasWorkers() == 0
		&& BWAPI::Broodwar->self()->gas() < price)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool StrategyManager::isMineRequireMeet(int price)
{
	if (price > 0
		&& WorkerManager::Instance().getNumMineralWorkers() == 0
		&& BWAPI::Broodwar->self()->minerals() < price)
	{
		return false;
	}
	else
	{
		return true;
	}
}




MetaType StrategyManager::getTargetUnit(BWAPI::UnitType exploreUnit = BWAPI::UnitTypes::None)
{
	//if (true)
	//{
		//exploreUnit = BWAPI::UnitTypes::Zerg_Hydralisk;
	//}

	featureValues.clear();
	//get current state features
	calCurrentStateFeature();

	vector<double> nnInput(NNInputAction.size());
	for (size_t i = 0; i < NNInputAction.size(); i++)
	{
		if (featureValues.find(NNInputAction[i]) != featureValues.end())
		{
			nnInput[i] = featureValues[NNInputAction[i]];
			featureValues.erase(NNInputAction[i]);
		}
		else
		{
			nnInput[i] = -1;
		}
	}

	if (featureValues.size() != 0)
	{
		BWAPI::Broodwar->printf("feature error!!!");
	}

	/*
	//softmax output is raw data, not the probability
	vector<double> nnOutput = policyModel.feedForward(nnInput);
	double softmaxTotal = 0;
	for (auto s : nnOutput)
	{
		softmaxTotal += s;
	}
	for (auto& s : nnOutput)
	{
		s = s / softmaxTotal;
	}
	*/
	vector<double> nnOutput = QValueModel.feedForward(nnInput);

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	map<string, double> filteredActionResult;
	for (size_t i = 0; i < nnOutput.size(); i++)
	{
		if (NNOutputAction[i].find("Building_") != std::string::npos)
		{
			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].unitType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].unitType.mineralPrice()))
			{
				continue;
			}

			//tech buildings just need one instance
			if (BWAPI::Broodwar->self()->allUnitCount(highLevelActions[NNOutputAction[i]].unitType) > 0
				|| buildingsUnderProcess.find(highLevelActions[NNOutputAction[i]].unitType) != buildingsUnderProcess.end())
			{
				continue;
			}
			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0
				&& highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Lair)
			{
				continue;
			}
			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0
				&& highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Spire)
			{
				continue;
			}
			if (highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Hive)
			{
				BWAPI::Unit baseUnit = InformationManager::Instance().GetOurBaseUnit();
				if (baseUnit != NULL && (baseUnit->isResearching() || baseUnit->isUpgrading()))
				{
					continue;
				}
			}

			std::map<BWAPI::UnitType, int> requireUnits = highLevelActions[NNOutputAction[i]].unitType.requiredUnits();
			bool isValid = true;
			for (auto u : requireUnits)
			{
				if (u.first.isBuilding() && BWAPI::Broodwar->self()->completedUnitCount(u.first) == 0)
				{
					isValid = false;
					break;
				}
			}

			if (isValid)
			{
				filteredActionResult[NNOutputAction[i]] = nnOutput[i];
			}
		}

		else if (NNOutputAction[i].find("Defense_") != std::string::npos)
		{
			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].unitType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].unitType.mineralPrice()))
			{
				continue;
			}

			int underBuildCount = 0;
			if (buildingsUnderProcess.find(highLevelActions[NNOutputAction[i]].unitType) != buildingsUnderProcess.end())
			{
				underBuildCount = buildingsUnderProcess[highLevelActions[NNOutputAction[i]].unitType].first;
			}
			if (BWAPI::Broodwar->self()->allUnitCount(highLevelActions[NNOutputAction[i]].unitType) + underBuildCount >= 10
				|| BWAPI::Broodwar->getFrameCount() < nextDefenseBuildTime)
				continue;

			if (NNOutputAction[i] == "Defense_Sunken" && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) > 0)
			{
				filteredActionResult[NNOutputAction[i]] = nnOutput[i];
			}
			else if (NNOutputAction[i] == "Defense_Spore" && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber) > 0)
			{
				filteredActionResult[NNOutputAction[i]] = nnOutput[i];
			}
			else
			{
				continue;
			}
		}

		else if (NNOutputAction[i].find("Unit_") != std::string::npos)
		{
			//if (exploreUnit != BWAPI::UnitTypes::None && highLevelActions[NNOutputAction[i]].unitType != exploreUnit)
			//{
				//continue;
			//}
			int morphCount = 0;
			if (morphUnits.find(BWAPI::UnitTypes::Zerg_Scourge) != morphUnits.end())
			{
				morphCount = morphUnits[BWAPI::UnitTypes::Zerg_Scourge];
			}
			if (highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Scourge
				&& BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Scourge) + morphCount > 12)
			{
				continue;
			}

			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].unitType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].unitType.mineralPrice()))
			{
				continue;
			}

			if (BWAPI::Broodwar->self()->gas() < highLevelActions[NNOutputAction[i]].unitType.gasPrice() * 3
				&& BWAPI::Broodwar->self()->minerals() > 5000 )
			{
				continue;
			}

			int supplyRequired = highLevelActions[NNOutputAction[i]].unitType.supplyRequired();
			if (highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Zergling 
				|| highLevelActions[NNOutputAction[i]].unitType == BWAPI::UnitTypes::Zerg_Scourge)
			{
				supplyRequired = supplyRequired * 2;
			}
			if (BWAPI::Broodwar->self()->supplyUsed() + supplyRequired > 200 * 2)
			{
				continue;
			}

			std::map<BWAPI::UnitType, int> requireUnits = highLevelActions[NNOutputAction[i]].unitType.requiredUnits();
			bool isValid = true;
			for (auto u : requireUnits)
			{
				if (u.first != BWAPI::UnitTypes::Zerg_Larva && BWAPI::Broodwar->self()->completedUnitCount(u.first) == 0)
				{
					if (u.first == BWAPI::UnitTypes::Zerg_Spire && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
					{
						continue;
					}
					if (u.first == BWAPI::UnitTypes::Zerg_Lair && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
					{
						continue;
					}
					isValid = false;
					break;
				}
			}

			if (isValid)
			{
				if (NNOutputAction[i] == "Unit_Lurker")
				{
					if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect))
					{
						filteredActionResult[NNOutputAction[i]] = nnOutput[i];
					}
				}
				else
				{
					filteredActionResult[NNOutputAction[i]] = nnOutput[i];
				}
			}
		}

		//only the lurker tech
		else if (NNOutputAction[i].find("Tech_") != std::string::npos)
		{
			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].techType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].techType.mineralPrice()))
			{
				continue;
			}

			if (techUnderProcess.find(highLevelActions[NNOutputAction[i]].techType) != techUnderProcess.end())
			{
				continue;
			}

			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) == false &&
				BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect) == false &&
				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) > 0 &&
				(BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) > 0 || 
				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0))
			{
				BWAPI::Unit hydraliskDen = *ourBuildings[BWAPI::UnitTypes::Zerg_Hydralisk_Den].begin();
				if (!hydraliskDen->isResearching() && !hydraliskDen->isUpgrading())
				{
					filteredActionResult[NNOutputAction[i]] = nnOutput[i];
				}
			}
		}

		else if (NNOutputAction[i].find("Upgrade_") != std::string::npos)
		{
			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].upgradeType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].upgradeType.mineralPrice()))
			{
				continue;
			}

			if (upgradeUnderProcess.find(highLevelActions[NNOutputAction[i]].upgradeType) != upgradeUnderProcess.end())
			{
				continue;
			}
			BWAPI::UnitType researchingUnit = highLevelActions[NNOutputAction[i]].upgradeType.whatUpgrades();
			BWAPI::UnitType requireUnit = highLevelActions[NNOutputAction[i]].upgradeType.whatsRequired();

			if (BWAPI::Broodwar->self()->getUpgradeLevel(highLevelActions[NNOutputAction[i]].upgradeType) == 0
				&& BWAPI::Broodwar->self()->isUpgrading(highLevelActions[NNOutputAction[i]].upgradeType) == false)
			{
				if (highLevelActions[NNOutputAction[i]].upgradeType == BWAPI::UpgradeTypes::Pneumatized_Carapace)
				{
					BWAPI::Unit baseUnit = InformationManager::Instance().GetOurBaseUnit();
					if (baseUnit == NULL)
					{
						continue;
					}

					//unit maybe researching other upgrades
					if (baseUnit->isCompleted() && (baseUnit->getType() == BWAPI::UnitTypes::Zerg_Lair || baseUnit->getType() == BWAPI::UnitTypes::Zerg_Hive)
						&& !baseUnit->isUpgrading() && !baseUnit->isResearching())
					{
						filteredActionResult[NNOutputAction[i]] = nnOutput[i];
					}
				}
				else
				{
					if (BWAPI::Broodwar->self()->completedUnitCount(researchingUnit) != 0)
					{
						BWAPI::Unit builder = *ourBuildings[researchingUnit].begin();
						if (!builder->isResearching() && !builder->isUpgrading()
							&& (requireUnit == BWAPI::UnitTypes::None || BWAPI::Broodwar->self()->completedUnitCount(requireUnit) != 0))
						{
							filteredActionResult[NNOutputAction[i]] = nnOutput[i];
						}
					}
				}
			}
		}

		else if (NNOutputAction[i].find("Expand_") != std::string::npos)
		{
			if (!isGasRequireMeet(highLevelActions[NNOutputAction[i]].unitType.gasPrice()) ||
				!isMineRequireMeet(highLevelActions[NNOutputAction[i]].unitType.mineralPrice()))
			{
				continue;
			}

			if (buildingsUnderProcess.find(highLevelActions[NNOutputAction[i]].unitType) != buildingsUnderProcess.end()
				|| InformationManager::Instance().GetNextExpandLocation() == BWAPI::TilePositions::None)
			{
				continue;
			}

			filteredActionResult[NNOutputAction[i]] = nnOutput[i];
		}
		
		else if (NNOutputAction[i].find("Attack_") != std::string::npos)
		{
			std::map<BWAPI::UnitType, int> remain = AttackManager::Instance().reaminArmy();

			if (NNOutputAction[i] == "Attack_MutaliskHarassAttack" && remain[BWAPI::UnitTypes::Zerg_Mutalisk] > 0
				&& AttackManager::Instance().getNextAttackPosition(false) != BWAPI::Positions::None)
			{
				filteredActionResult[NNOutputAction[i]] = nnOutput[i];
			}
			else if (NNOutputAction[i] == "Attack_AllInAttack" && AttackManager::Instance().getNextAttackPosition(true) != BWAPI::Positions::None)
			{
				int total = 0;
				for (auto army : remain)
				{
					total += army.second;
				}
				if (total > 0)
				{
					filteredActionResult[NNOutputAction[i]] = nnOutput[i];
				}
			}
		}
		else if (NNOutputAction[i].find("Wait_") != std::string::npos)
		{
			if (BWAPI::Broodwar->getFrameCount() > 24 * 60 * 15)
			{
				continue;
			}

			//only wait when some key building/tech is morphing
			bool isUnderConstruct = false;
			for (auto b : buildingsUnderProcess)
			{
				if (b.first != BWAPI::UnitTypes::Zerg_Sunken_Colony && b.first != BWAPI::UnitTypes::Zerg_Spore_Colony
					&& b.first != BWAPI::UnitTypes::Zerg_Hatchery)
				{
					isUnderConstruct = true;
					break;
				}
			}
			if (BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect))
			{
				isUnderConstruct = true;
			}

			if (isUnderConstruct)
			{
				filteredActionResult[NNOutputAction[i]] = nnOutput[i];
			}
		}

		else
		{
			BWAPI::Broodwar->printf("error type filter!!");
		}
	}

	////no valid action available
	if (filteredActionResult.size() == 0)
	{
		return MetaType();
	}

	/*
	double filteredTotalProb = 0.0f;
	for (auto f : filteredActionResult)
	{
		filteredTotalProb += f.second;
	}

	LARGE_INTEGER cpuTime;
	QueryPerformanceCounter(&cpuTime);
	unsigned int startTimeInMicroSec = cpuTime.QuadPart % 1000000;

	std::srand(startTimeInMicroSec);
	int randomNum = std::rand() % 100;
	double accumulatedProb = 0.0;
	string chosedAction;
	double chosedActionProb;
	for (auto f : filteredActionResult)
	{
		accumulatedProb += f.second / filteredTotalProb;
		if (accumulatedProb * 100 > randomNum)
		{
			chosedAction = f.first;
			chosedActionProb = f.second / filteredTotalProb;
			break;
		}
	}
	
	vector<double> valueOutput = stateValueModel.feedForward(nnInput);

	//save data for training
	//schema: inputFeature, curStateValue, reward, curChosedActionIndex, curOutputChosen, fromStartIndex, nextStateInputFeature
	if (previousChosenAction != chosedAction)
	{
	std::vector<std::string> dataVector;
	std::stringstream dataString;
	dataString.precision(15);
	dataString << std::fixed;
	for (auto in : nnInput)
	{
	dataString << in << ":";
	}
	dataVector.push_back(dataString.str());
	dataVector.push_back(std::to_string(valueOutput[0]));
	dataVector.push_back(std::to_string(tmpReward));
	tmpReward = 0;
	dataVector.push_back(std::to_string(actionsOutputIndexMapping[chosedAction]));
	std::vector<int> filterOutput;
	filterOutput.reserve(filteredActionResult.size());
	for (auto f : filteredActionResult)
	{
	filterOutput.push_back(actionsOutputIndexMapping[f.first]);
	}
	dataString.str("");
	dataString.clear();
	for (auto f : filterOutput)
	{
	dataString << f << ":";
	}
	dataVector.push_back(dataString.str());
	dataVector.push_back(std::to_string(trainingData.size()));
	dataVector.push_back("");
	trainingData.push_back(dataVector);

	previousChosenAction = chosedAction;
	}

	*/

	double maxValue = -999999;
	string maxAction;
	double secondValue = -99999;
	string secondAction;
	for (auto f : filteredActionResult)
	{
		if (f.second > secondValue)
		{
			secondAction = f.first;
			secondValue = f.second;
			if (secondValue > maxValue)
			{
				string tmpAction = maxAction;
				double tmpValue = maxValue;
				maxValue = secondValue;
				maxAction = secondAction;
				secondAction = tmpAction;
				secondValue = tmpValue;
			}
		}
	}
	double diff = (maxValue - secondValue) / maxValue;

	int exploreRate = 0;
	exploreRate = 100 - int(((float(playMatchCount) / 50)) * 90)  < 10 ? 10 : 100 - int(((float(playMatchCount) / 50)) * 90);

	string chosedAction;
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_int_distribution<int>  distr(0, 100);
	int randomNum = distr(generator);
	bool isExplore = false;
	//do explore
	if (randomNum < exploreRate)
	{
		isExplore = true;
		std::uniform_int_distribution<int>  dist2(0, filteredActionResult.size() - 1);
		int randomActionNum = dist2(generator);
		int count = 0;
		for (auto f : filteredActionResult)
		{
			if (count == randomActionNum)
			{
				chosedAction = f.first;
				break;
			}
			count++;
		}

	}
	else
	{
		chosedAction = maxAction;
	}
	
	BWAPI::Broodwar->printf("current:%s,max:%s,reward:%.2f,\n diff:%f, time: %d", chosedAction.c_str(), maxAction.c_str(), tmpReward, diff, BWAPI::Broodwar->getFrameCount());


	if (filteredActionResult.size() > 1)
	{
		//save data for training
		//schema: curState, action, reward, nextState, currentStateValidActions, nextStateValidActions
		if (trainingData.size() > 0)
		{
			trainingData.back()[2] = std::to_string(tmpReward);
		}
		tmpReward = 0;
		std::vector<std::string> dataVector;
		std::stringstream dataString;
		dataString.precision(15);
		dataString << std::fixed;
		for (auto in : nnInput)
		{
			dataString << in << ":";
		}
		dataVector.push_back(dataString.str());
		dataVector.push_back(std::to_string(actionsOutputIndexMapping[chosedAction]));
		dataVector.push_back("0");
		dataVector.push_back("");
		std::vector<int> filterOutput;
		filterOutput.reserve(filteredActionResult.size());
		for (auto f : filteredActionResult)
		{
			filterOutput.push_back(actionsOutputIndexMapping[f.first]);
		}
		dataString.str("");
		dataString.clear();
		for (auto f : filterOutput)
		{
			dataString << f << ":";
		}
		dataVector.push_back(dataString.str());
		dataVector.push_back("");
		trainingData.push_back(dataVector);
	}
	

	if (chosedAction.find("Attack_") != std::string::npos)
	{
		if (chosedAction == "Attack_MutaliskHarassAttack")
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac);
		}
		else
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic);
		}

		return MetaType();
	}
	else if (chosedAction.find("Expand_") != std::string::npos)
	{
		BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
		if (nextBase == BWAPI::TilePositions::None)
			nextBase = BWAPI::Broodwar->self()->getStartLocation();

		buildingsUnderProcess[highLevelActions[chosedAction].unitType].first = 1;
		InformationManager::Instance().addFakeOccupiedBase(nextBase);
		return MetaType(BWAPI::UnitTypes::Zerg_Hatchery, nextBase);
	}
	else if (chosedAction.find("Building_") != std::string::npos)
	{
		buildingsUnderProcess[highLevelActions[chosedAction].unitType].first = 1;
		return highLevelActions[chosedAction];
	}
	else if (chosedAction.find("Defense_") != std::string::npos)
	{
		if (buildingsUnderProcess.find(highLevelActions[chosedAction].unitType) == buildingsUnderProcess.end())
			buildingsUnderProcess[highLevelActions[chosedAction].unitType].first = 0;
		buildingsUnderProcess[highLevelActions[chosedAction].unitType].first += 1;
		nextDefenseBuildTime = BWAPI::Broodwar->getFrameCount() + 24 * 3;

		ProductionManager::Instance().triggerBuilding(highLevelActions[chosedAction].unitType, InformationManager::Instance().getSunkenBuildingPosition(), 1);
		return MetaType();
	}
	else if (chosedAction.find("Tech_") != std::string::npos)
	{
		ProductionManager::Instance().triggerTech(highLevelActions[chosedAction].techType);
		techUnderProcess[highLevelActions[chosedAction].techType] = chosedAction;
		return MetaType();
	}
	else if (chosedAction.find("Upgrade_") != std::string::npos)
	{
		ProductionManager::Instance().triggerUpgrade(highLevelActions[chosedAction].upgradeType);
		upgradeUnderProcess[highLevelActions[chosedAction].upgradeType] = chosedAction;
		return MetaType();
	}
	else if (chosedAction.find("Wait_") != std::string::npos)
	{
		return MetaType();
	}
	else
	{
		return highLevelActions[chosedAction];
	}
}


sampleParsedInfo StrategyManager::sampleParse(std::vector<std::string>& sample)
{
	sampleParsedInfo result;

	std::stringstream ss;
	ss.str("");
	ss.clear();
	ss << sample[0];
	string item;
	while (getline(ss, item, ':'))
	{
		if (item == "")
			continue;
		result.modelInputs.push_back(std::stod(item));
	}
	result.actualChosen = std::stoi(sample[1]);

	double targetQValue = 0;
	//terminal state-- next state value is empty
	if (sample[3] == "")
	{
		targetQValue = std::stod(sample[2]);
	}
	else
	{
		ss.str("");
		ss.clear();
		ss << sample[3];
		std::vector<double> nextItemList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			nextItemList.push_back(std::stod(item));
		}
		//double Q learning
		vector<double> allActionValues = QValueModel.feedForward(nextItemList);
		vector<double> targetAllActionValues = targetQValueModel.feedForward(nextItemList);

		ss.str("");
		ss.clear();
		ss << sample[5];
		std::set<int> outList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			outList.insert(std::stoi(item));
		}
		double maxValue = -999999;
		int	maxActionIndex = 0;
		for (size_t i = 0; i < allActionValues.size(); i++)
		{
			if (outList.find(i) != outList.end() && allActionValues[i] > maxValue)
			{
				maxValue = allActionValues[i];
				maxActionIndex = i;
			}
		}

		double targetMaxValue = targetAllActionValues[maxActionIndex];
		targetQValue = std::stod(sample[2]) + std::pow(discountRate, tdStep) * targetMaxValue;
	}
	result.targetQValue = targetQValue;

	return result;
}


double StrategyManager::getSamplePriority(std::vector<std::string>& sample)
{
	sampleParsedInfo result = sampleParse(sample);
	vector<double> allActionValues = QValueModel.feedForward(result.modelInputs);
	double TDError = allActionValues[result.actualChosen] - result.targetQValue;
	return  std::pow(std::abs(TDError) + 0.01, 0.6);
}


void StrategyManager::trainModels(int reward)
{
	if (trainingData.size() > 0)
	{
		std::map<std::string, int> actionExecutionCount;
		trainingData[trainingData.size() - 1][2] = std::to_string(reward + tmpReward);
		for (int i = 0; i < int(trainingData.size()) - 1; i++)
		{
			int nStepIndex = i + tdStep;
			//schema: curState, action, reward, nextState, currentStateValidActions, nextStateValidActions
			if (nStepIndex <= int(trainingData.size()) - 1)
			{
				trainingData[i][3] = trainingData[nStepIndex][0];
				trainingData[i][5] = trainingData[nStepIndex][4];
			}

			double totalReward = 0;
			int step = 0;
			if (nStepIndex > int(trainingData.size()) - 1)
			{
				nStepIndex = int(trainingData.size());
			}
			for (int j = i; j < nStepIndex; j++)
			{
				totalReward += std::pow(discountRate, step) * std::stod(trainingData[j][2]);
				step++;
			}
			trainingData[i][2] = std::to_string(totalReward);
		}

		for (auto sample : trainingData)
		{
			double priority = getSamplePriority(sample);
			replayData.add(priority, sample);
		}
	}

	if (replayData.getData().size() == 0){
		return;
	}

	int trainingRound = 200;//trainingData.size() < 200 ? 200 : trainingData.size();
	int miniBatch = 8;
	vector<double> loss;
	
	for (int round = 0; round < trainingRound; round++)
	{
		double curError = iterationTrain(miniBatch);
		loss.push_back(curError);
	}

	/*
	fstream historyFile;
	string filePath = "./bwapi-data/write/loss_data";
	historyFile.open(filePath.c_str(), ios::app);
	double total = 0;
	historyFile << "round:" << playMatchCount <<endl;
	for (size_t i = 0; i < loss.size();)
	{
		total += loss[i];
		i++;
		if (i % 10 == 0 && i != 0)
		{
			historyFile << total / 10;
			historyFile << endl;
			total = 0;
		}
	}
	historyFile << endl;
	historyFile.close();
	*/

	//tesetSetPerformance();
	experienceDataSave();
	string enemyName = BWAPI::Broodwar->enemy()->getName();
	QValueModel.serialize(enemyName + "_QValue", writeResourceFolder);

	int targetUpdateFrequence = 10;
	if (playMatchCount >= 50)
	{
		targetUpdateFrequence = 20;
	}
	else if (playMatchCount >= 100)
	{
		targetUpdateFrequence = 40;
	}
	//target network delay update
	if (playMatchCount % targetUpdateFrequence == 0)
	{
		targetQValueModel.copy(QValueModel);
	}
	targetQValueModel.serialize(enemyName + "_targetQValueModel", writeResourceFolder);


	/*
		std::map<std::string, int> actionExecutionCount;
		for (int i = 0; i < int(trainingData.size()); i++)
		{
			double dReward = std::pow(discountRate, trainingData.size() - 1 - i) * reward;
			trainingData[i][2] = std::to_string(dReward);

			//std::size_t pos = NNOutputAction[std::stoi(trainingData[i][3])].find("_");
			//std::string categoty = NNOutputAction[std::stoi(trainingData[i][3])].substr(0, pos);
			std::string categoty = NNOutputAction[std::stoi(trainingData[i][3])];
			if (actionExecutionCount.find(categoty) == actionExecutionCount.end())
			{
				actionExecutionCount[categoty] = 0;
			}
			actionExecutionCount[categoty] += 1;
		}

		int maxCount = 0;
		for (auto e : actionExecutionCount)
		{
			if (e.second > maxCount)
			{
				maxCount = e.second;
			}
		}

		std::vector<std::vector<std::string>> tmpAdd;
		for (int i = 0; i < int(trainingData.size()); i++)
		{
			std::string categoty = NNOutputAction[std::stoi(trainingData[i][3])];
			if (categoty.find("Upgrade_") == std::string::npos)
			{
				int addCopyRatio = (int(maxCount / actionExecutionCount[categoty]) - 1);
				tmpAdd.insert(tmpAdd.end(), addCopyRatio, trainingData[i]);
			}
		}

		trainingData.insert(trainingData.end(), tmpAdd.begin(), tmpAdd.end());
		if (trainingData.size() != 0)
		{
			experienceData.push_back(trainingData);
		}
	}
	
	if (experienceData.size() == 0)
		return;

	vector<int> loseMatchIndex;
	vector<int> winMatchIndex;
	for (size_t i = 0; i < experienceData.size(); i++)
	{
		if (std::stod(experienceData[i][0][2]) < 0)
		{
			loseMatchIndex.push_back(i);
		}
		else
		{
			winMatchIndex.push_back(i);
		}
	}

	std::srand((unsigned int)std::time(0));
	int GDRound = 100;
	int eachGdRoundSampleMatch = 10;
	if (experienceData.size() < 10)
	{
		GDRound = experienceData.size();
		eachGdRoundSampleMatch = 3;
	}

	for (int i = 0; i < GDRound; i++)
	{
		std::vector<int> sampleMatches;
		for (int sample = 0; sample < eachGdRoundSampleMatch; sample++)
		{
			//int matchIndex = std::rand() % experienceData.size();
			int matchIndex = 0;
			if (loseMatchIndex.size() > 0 && winMatchIndex.size() > 0)
			{
				if (sample % 3 == 0)
				{
					matchIndex = loseMatchIndex[std::rand() % loseMatchIndex.size()];
				}
				else
				{
					matchIndex = winMatchIndex[std::rand() % winMatchIndex.size()];
				}
			}
			else
			{
				matchIndex = std::rand() % experienceData.size();
			}

			sampleMatches.push_back(matchIndex);
		}

		double curError = iterationTrain(sampleMatches);
	}

	//repeat training instance with largest error
	//iterationTrain(maxMatchIndex);

	experienceDataSave();
	policyModel.serialize("policy");
	stateValueModel.serialize("stateValue");
	stateValueTargetModel.serialize("stateValueTarget");
	*/
}


double StrategyManager::iterationTrain(int miniBatch)
{
	map<int, vector<string>> sampleDatas = replayData.sample(miniBatch);

	vector<vector<double>> modelInputs;
	vector<int> actualChosen;
	vector<double> targetValues;
	for (auto instance : sampleDatas)
	{
		double priority = getSamplePriority(instance.second);
		replayData.updatePriority(instance.first, priority);

		//importance sampling weight

		sampleParsedInfo parseData = sampleParse(instance.second);
		modelInputs.push_back(parseData.modelInputs);
		actualChosen.push_back(parseData.actualChosen);
		targetValues.push_back(parseData.targetQValue);
	}

	double curError = 0;
	curError = QValueModel.train(modelInputs, MSE, targetValues, actualChosen);
	return curError;


	/*
	vector<vector<double>> modelInputs;
	vector<double> expectValue;
	vector<int> actualChosen;
	vector<vector<int>> outputChosen;
	vector<double> qValues;
	vector<int> indexValue;
	std::stringstream ss;
	for (int i = 0; i < miniBatch; i++)
	{
		int index = std::rand() % experienceData.size();
		ss.str("");
		ss.clear();
		ss << experienceData[index][0];
		std::vector<double> itemList;
		string item;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			itemList.push_back(std::stod(item));
		}
		modelInputs.push_back(itemList);
		ss.str("");
		ss.clear();
		ss << experienceData[index][4];
		std::vector<int> outList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			outList.push_back(std::stoi(item));
		}
		outputChosen.push_back(outList);
		//expectValue.push_back(std::stod(experienceData[matchIndex][index][2]));
		actualChosen.push_back(std::stoi(experienceData[index][3]));
		indexValue.push_back(std::stoi(experienceData[index][5]));

		double qValue = 0;
		double targetQValue = 0;
		//terminal state
		if (experienceData[index][6] == "")
		{
			qValue = targetQValue = std::stod(experienceData[index][2]);
		}
		else
		{
			ss.str("");
			ss.clear();
			ss << experienceData[index][6];
			std::vector<double> nextItemList;
			while (getline(ss, item, ':'))
			{
				if (item == "")
					continue;
				nextItemList.push_back(std::stod(item));
			}
			qValue = std::stod(experienceData[index][2]) + std::pow(discountRate, tdStep) * stateValueModel.feedForward(nextItemList)[0];
			targetQValue = std::stod(experienceData[index][2]) + std::pow(discountRate, tdStep) * stateValueTargetModel.feedForward(nextItemList)[0];
		}

		double baseline = stateValueModel.feedForward(itemList)[0];
		//reinforce with baseline
		qValues.push_back(qValue - baseline);
		expectValue.push_back(targetQValue);
	}

	double curError = policyModel.train(modelInputs, POLICY_GRADIENT, vector<double>(), actualChosen, 
		outputChosen, qValues, indexValue, miniBatch);

	stateValueModel.train(modelInputs, MSE, expectValue);

	stateValueTargetModel.targetNetworkUpdate(stateValueModel);

	return curError;
	*/
}


void StrategyManager::tesetSetPerformance()
{
	vector<double> maxQValues;
	vector<vector<double>> modelInputs;
	vector<int> actualChosen;
	vector<vector<int>> outputChosen;
	vector<double> targetValues;
	std::stringstream ss;
	for (size_t index = 0; index < testSetData.size(); index++)
	{
		string item;
		ss.str("");
		ss.clear();
		ss << testSetData[index][0];
		std::vector<double> nextItemList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			nextItemList.push_back(std::stod(item));
		}
		vector<double> allActionValues = QValueModel.feedForward(nextItemList);

		ss.str("");
		ss.clear();
		ss << testSetData[index][4];
		std::set<int> outList;
		while (getline(ss, item, ':'))
		{
			if (item == "")
				continue;
			outList.insert(std::stoi(item));
		}
		double maxValue = -999999;
		for (size_t i = 0; i < allActionValues.size(); i++)
		{
			if (outList.find(i) != outList.end() && allActionValues[i] > maxValue)
			{
				maxValue = allActionValues[i];
			}
		}
		maxQValues.push_back(maxValue);
	}

	double sumValue = 0;
	for (auto m : maxQValues)
	{
		sumValue += m;
	}
	testSetAvgQValue = sumValue / maxQValues.size();
}


void StrategyManager::update()
{
	if (BWAPI::Broodwar->getFrameCount() % 25  == 0)
	{
		for (std::map<BWAPI::TechType, std::string>::iterator it = techUnderProcess.begin(); it != techUnderProcess.end();)
		{
			if (BWAPI::Broodwar->self()->hasResearched(it->first))
			{
				it = techUnderProcess.erase(it);
			}
			else
			{
				it++;
			}
		}

		for (std::map<BWAPI::UpgradeType, std::string>::iterator it = upgradeUnderProcess.begin(); it != upgradeUnderProcess.end();)
		{
			if (BWAPI::Broodwar->self()->getUpgradeLevel(it->first) > 0)
			{
				it = upgradeUnderProcess.erase(it);
			}
			else
			{
				it++;
			}
		}

		for (std::map<BWAPI::UnitType, std::pair<int, int>>::iterator it = buildingsUnderProcess.begin(); it != buildingsUnderProcess.end();)
		{
			it->second.second += 25;
			if (it->first != BWAPI::UnitTypes::Zerg_Hatchery && it->second.second > 25 * 60 
				&& BWAPI::Broodwar->self()->allUnitCount(it->first) == 0)
			{
				it = buildingsUnderProcess.erase(it);
				BWAPI::Broodwar->printf("buildingsUnderProcess building not build!!!!!!!");
			}
			else
			{
				it++;
			}
		}

		

	}
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
		//return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 4 6 1 11 11 0 0 0 0 12 12 8 0 5 1 1 0 0 10 10 10 10 10 10 10 1 10 10 10 10");

		//overPool opening
		//return getMetaVector("0 0 0 0 0 1 3 0 0 0 2 4 4 4 0 5 0 0 0 6 0 0 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10 10 10 19 20 19 20 19 20");
		
		if (currentopeningStrategy == TwelveHatchMuta)
		{
			//12 hatch mutalisk
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4 5 0 0 0 0 6 0 1 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10");
		}
		else if (currentopeningStrategy == NinePoolling)
		{
			//9 pool zergling
			return getMetaVector("0 0 0 0 0 3 0 5 0 1 4 4 4 0");
		}
		else if (currentopeningStrategy == TenHatchMuta)
		{
			// 10 hatch counter 2 gate zealot
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4 0 0");
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


