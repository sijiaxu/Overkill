
#include "StrategyManager.h"

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
	gameStage = Start;
	triggerMutaliskBuild = true;

	goalBuildingOrderInit();
	disableMutaliskHarass = false;

	openingStrategyName.push_back("TwelveHatchMuta");
	openingStrategyName.push_back("NinePoolling");
	openingStrategyName.push_back("TenHatchMuta");

	lairTrigger = true;
	spireTrigger = true;

	mutaUpgradeTrigger = true;
	hydraUpgradeTrigger = true;
	overlordUpgradeTrigger = true;
}

void StrategyManager::setOpeningStrategy(openingStrategy opening)
{
	currentopeningStrategy = opening;
	switch (currentopeningStrategy)
	{
	case TwelveHatchMuta:
		currentStrategy = MutaPush;
		break;
	case NinePoolling:
		currentStrategy = ZerglingPush;
		break;
	case TenHatchMuta:
		currentStrategy = MutaPush;
		break;
	default:
		break;
	}
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


void StrategyManager::goalBuildingOrderInit()
{
	goalBuildingOrder[HydraPush] = getMetaVector("9");
	goalBuildingOrder[ZerglingPush] = getMetaVector("4");
	goalBuildingOrder[MutaPush] = getMetaVector("10");
}

const std::vector<MetaType>& StrategyManager::getCurrentGoalBuildingOrder()
{
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();

	if ((currentStrategy == HydraPush || currentStrategy == MutaPush)
		&& BWAPI::Broodwar->self()->minerals() > 500 && BWAPI::Broodwar->self()->gas() < 200 
		&& BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva) >= 2 )
	{
		return goalBuildingOrder[ZerglingPush];
	}
	else if (currentStrategy == HydraPush && (selfAllBuilding[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() == 0 ||
		(selfAllBuilding[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() > 0 && !(*selfAllBuilding[BWAPI::UnitTypes::Zerg_Hydralisk_Den].begin())->isCompleted())))
	{
		return goalBuildingOrder[ZerglingPush];
	}
	else
		return goalBuildingOrder[currentStrategy];
}


void StrategyManager::goalChange(zergStrategy changeStrategy)
{
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();

	if (changeStrategy == HydraPush)
	{
		//change to hydra is quick, so do not wait
		ProductionManager::Instance().clearCurrentQueue();

		currentStrategy = HydraPush;

		//ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 12);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) == 0)
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self()->getStartLocation(), 1);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Grooved_Spines);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Muscular_Augments);
		}
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == 0)
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->self()->getStartLocation(), 1);
		}

	}
	else if (changeStrategy == MutaPush)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0)
		{
			if (lairTrigger)
			{
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self()->getStartLocation(), 1);
				lairTrigger = false;
			}
		}

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Spire) == 0)
		{
			if (spireTrigger && selfAllBuilding[BWAPI::UnitTypes::Zerg_Lair].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Lair].begin())->isCompleted())
			{
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self()->getStartLocation(), 1);
				spireTrigger = false;
			}
		}
		//if all prerequisite building has complete, change strategy
		if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].begin())->isCompleted())
		{
			currentStrategy = MutaPush;
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 6, true, true);
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


//key function to adapt change
void StrategyManager::update()
{
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
		//return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 4 6 1 11 11 0 0 0 0 12 12 8 0 5 1 1 0 0 10 10 10 10 10 10 10 1 10 10 10 10 19");

		//overPool opening
		//return getMetaVector("0 0 0 0 0 1 3 0 0 0 2 4 4 4 0 5 0 0 0 6 0 0 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10 10 10 19 20 19 20 19 20");
		
		if (currentopeningStrategy == TwelveHatchMuta)
		{
			//12 hatch mutalisk
			return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 4 4 4 6 15");
		}
		else if (currentopeningStrategy == NinePoolling)
		{
			//9 pool zergling
			return getMetaVector("0 0 0 0 0 3 0 5 0 1 4 4 4 4 15");
		}
		else if (currentopeningStrategy == TenHatchMuta)
		{
			// 10 hatch counter 2 gate zealot
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4 5");
		}
		else
			return getMetaVector("");
	}
	else
		return getMetaVector("");
}


int StrategyManager::getScore(BWAPI::Player player)
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
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


void StrategyManager::changeGameStage(zergGameStage curStage)
{
	gameStage = curStage;
}

int	StrategyManager::getGameStage()
{
	return gameStage;
}
