
#include "StrategyManager.h"

// constructor
StrategyManager::StrategyManager()
	: firstAttackSent(false)
	, currentStrategy(0)
	, selfRace(BWAPI::Broodwar->self()->getRace())
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
	currentStrategyGoal = MutaliskRush;
	triggerMutaliskBuild = true;

	goalBuildingOrderInit();
	disableMutaliskHarass = false;
}


void StrategyManager::goalBuildingOrderInit()
{
	currentStrategyGoal = MutaliskRush;

	goalBuildingOrder[MutaliskRush] = getMetaVector("10 10 10 10 10 10 10 10 10 10 10 10");
	goalBuildingOrder[MutaHydraRush] = getMetaVector("9 9 9 9 9 9 9 9 9 9 9 9");
}


const std::vector<MetaType>& StrategyManager::getCurrentGoalBuildingOrder()
{
	int hydriskCount = 0;
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Hydralisk)
			{
				hydriskCount++;
			}
		}
	}

	if (currentStrategyGoal == MutaHydraRush && (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk) + hydriskCount) > 20 && triggerMutaliskBuild)
	{
		ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 12);
		triggerMutaliskBuild = false;
		return goalBuildingOrder[currentStrategyGoal];
	}
	else
		return goalBuildingOrder[currentStrategyGoal];
}


void StrategyManager::goalChange(strategyGoal changeGoal)
{
	//mutalisk fail, change to hydralisk
	if (changeGoal == MutaHydraRush)
	{
		ProductionManager::Instance().clearCurrentQueue();
		currentStrategyGoal = MutaHydraRush;

		//ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, 12);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) == 0)
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self()->getStartLocation(), 1);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Grooved_Spines);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Muscular_Augments);
		}
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == 0)
		{
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->self()->getStartLocation(), 2);

			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
			ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
		}
		//upgrade overlord
		ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Pneumatized_Carapace);
		ProductionManager::Instance().triggerUpgrade(BWAPI::UpgradeTypes::Ventral_Sacs);
	}
}

void StrategyManager::baseExpand()
{
	BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
	if (nextBase == BWAPI::TilePositions::None)
		return;

	ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hatchery, nextBase, 1);
	//if ((*BWTA::getRegion(nextBase)->getBaseLocations().begin())->getGeysers().size() > 0 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) < 3)
	if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) >= 48 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) < 3)
	{
		//extractor location is determined in building manager 
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self()->getStartLocation(), 1);
	}
}


//key function to adapt change
void StrategyManager::update()
{
	switch (gameStage)
	{
	case Start:
		break;
	case Mid:
	{
		std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& enemyBattle = InformationManager::Instance().getEnemyAllBattleUnit();
		int deadCount = BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
		int currentCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
		if (currentCount == 0)
			return;
		if ((currentCount * 10 / (currentCount + deadCount) <= 2)
			|| enemyBattle[BWAPI::UnitTypes::Protoss_Corsair].size() >= 8)
		{
			disableMutaliskHarass = true;
		}

		switch (currentStrategyGoal)
		{
		case BaseExpand:
			break; 
		case MutaliskRush:
		{
			/*
			std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>& enemyBattle = InformationManager::Instance().getEnemyAllBattleUnit();
			int deadCount = BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
			int currentCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
			if (currentCount == 0)
				return;
			if ((currentCount * 10 / (currentCount + deadCount) <= 2)
				|| (enemyBattle.find(BWAPI::UnitTypes::Protoss_Corsair) != enemyBattle.end() && enemyBattle[BWAPI::UnitTypes::Protoss_Corsair].size() >= 8))
			{
				currentStrategyGoal = MutaHydraRush;
				goalChange(MutaHydraRush);
			}*/
		}
			break;
		case HydraliskRush:
			break;
		case ZerglingRush:
			break;
		case MutaHydraRush:
			break;
		default:
			break;
		}
	}
		break;
	case End:
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
		return getMetaVector("0 0 0 0 0 1 3 0 0 0 2 4 4 4 0 5 4 0 0 0 6 0 0 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10 10 10 19 20 19 20 19 20");
	}
	else
		return getMetaVector("");
}



const double StrategyManager::getUCBValue(const size_t & strategy) const
{
	double totalTrials(0);
	for (size_t s(0); s < usableStrategies.size(); ++s)
	{
		totalTrials += results[usableStrategies[s]].first + results[usableStrategies[s]].second;
	}

	double C = 0.7;
	double wins = results[strategy].first;
	double trials = results[strategy].first + results[strategy].second;

	double ucb = (wins / trials) + C * sqrt(std::log(totalTrials) / trials);

	return ucb;
}

const int StrategyManager::getScore(BWAPI::Player * player) const
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


// when do we want to defend with our workers?
// this function can only be called if we have no fighters to defend with
const int StrategyManager::defendWithWorkers()
{
	if (!Options::Micro::WORKER_DEFENSE)
	{
		return false;
	}

	// our home nexus position
	BWAPI::Position homePosition = BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition();;

	// enemy units near our workers
	int enemyUnitsNearWorkers = 0;

	// defense radius of nexus
	int defenseRadius = 300;

	// fill the set with the types of units we're concerned about
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if it's a zergling or a worker we want to defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			if (unit->getDistance(homePosition) < defenseRadius)
			{
				enemyUnitsNearWorkers++;
			}
		}
	}

	// if there are enemy units near our workers, we want to defend
	return enemyUnitsNearWorkers;
}

// called by combat commander to determine whether or not to send an attack force
// freeUnits are the units available to do this attack
const bool StrategyManager::doAttack(const std::set<BWAPI::Unit *> & freeUnits)
{
	int ourForceSize = (int)freeUnits.size();

	int numUnitsNeededForAttack = 1;

	bool doAttack = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) >= 1
		|| ourForceSize >= numUnitsNeededForAttack;

	if (doAttack)
	{
		firstAttackSent = true;
	}

	return doAttack || firstAttackSent;
}


const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		return getZergBuildOrderGoal();
	}
	return getZergBuildOrderGoal();
}


const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMutas = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
	int numHydras = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);

	int mutasWanted = numMutas + 6;
	int hydrasWanted = numHydras + 6;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, 4));
	//goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Stim_Packs,	1));

	//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

void StrategyManager::changeGameStage(zergGameStage curStage)
{
	gameStage = curStage;
}

int	StrategyManager::getGameStage()
{
	return gameStage;
}
