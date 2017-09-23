#include "InformationManager.h"

#define SELF_INDEX 0
#define ENEMY_INDEX 1

// constructor
InformationManager::InformationManager()
{

	enemyInfluenceMap.resize(BWAPI::Broodwar->mapWidth(), std::vector<gridInfo>(BWAPI::Broodwar->mapHeight(), gridInfo()));

	selfNaturalBaseLocation = BWAPI::TilePositions::None;
	selfStartBaseLocation = BWAPI::Broodwar->self()->getStartLocation();
	BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			selfAllBase.insert(u);
	}
	enemyStartBaseLocation = BWAPI::TilePositions::None;
	enemyNaturalBaseLocation = BWAPI::TilePositions::None;
	firstColonyLocation = BWAPI::TilePositions::None;

	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Zergling] = std::set<BWAPI::Unit>();
	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Hydralisk] = std::set<BWAPI::Unit>();
	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Mutalisk] = std::set<BWAPI::Unit>();

	selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony] = std::set<BWAPI::Unit>();
	selfAllBuilding[BWAPI::UnitTypes::Zerg_Spore_Colony] = std::set<BWAPI::Unit>();
	selfAllBuilding[BWAPI::UnitTypes::Zerg_Creep_Colony] = std::set<BWAPI::Unit>();

	defendTrig = false;

	BOOST_FOREACH(BWAPI::Unit u, selfAllBase)
	{
		if (u->getTilePosition() == BWAPI::Broodwar->self()->getStartLocation())
		{
			selfBaseUnit = u;
			break;
		}
	}

	depotBalanceFlag = true;
	waitforDepotTime = 0;

	natrualFlag = true;
	waitTime = 0;

	zealotRushFlag = true;
	zealotRushTrig = true;

	depotTrigMap[selfBaseUnit] = true;

	midRushFlag = true;

	earlyRush = true;

	defendAddSupply = 0;
	waitToBuildSunker = 0;

	baseSunkenBuildingPosition = BWAPI::TilePositions::None;
	natrualSunkenBuildingPosition = BWAPI::TilePositions::None;

	enemyEarlyRushSuccess = false;
	airDropTrigger = true;

	airDefendTrigger = true;
	chamberTrigger = true;

	needDefendCheck = false;
	buildZerglingChekcTime = 0;
	defendBuildZerglingsCount = 0;
}


int	InformationManager::getEnemyTotalAntiGroundBattleForce()
{
	int totalSupply = 0;
	for (auto unitCategory : enemyAllBattleUnit)
	{
		if (unitCategory.first.groundWeapon() != BWAPI::WeaponTypes::None && !unitCategory.first.isWorker())
		{
			totalSupply += unitCategory.first.supplyRequired() * unitCategory.second.size();
		}
	}
	return totalSupply;
}


int	InformationManager::getOurTotalBattleForce()
{
	int totalSupply = 0;
	for (auto unitCategory : selfAllBattleUnit)
	{
		if (!unitCategory.first.isWorker())
		{
			totalSupply += unitCategory.first.supplyRequired() * unitCategory.second.size();
		}
	}
	return totalSupply;
}

void InformationManager::checkSelfNewDepotFinish()
{
	BOOST_FOREACH(BWAPI::Unit depot, selfAllBase)
	{
		//new depot finish
		if (BWAPI::Broodwar->getFrameCount() > 300 && depotTrigMap.find(depot) == depotTrigMap.end() && depot->isCompleted())
		{
			//waiting for the creep and vision expand
			if (depotBalanceFlag)
			{
				depotBalanceFlag = false;
				waitforDepotTime = BWAPI::Broodwar->getFrameCount() + 24 * 10;
			}
			if (BWAPI::Broodwar->getFrameCount() > waitforDepotTime)
			{
				if (BWTA::getRegion(selfNaturalBaseLocation) == BWTA::getRegion(depot->getPosition()))
				{
					continue;
					//WorkerManager::Instance().balanceWorkerOnDepotComplete(depot);
				}
				else
				{
					if (BWTA::getRegion(selfNaturalBaseLocation) != BWTA::getRegion(depot->getPosition()))
					{
					int buildingTime = BWAPI::Broodwar->getFrameCount() + 24 * 20;
					const std::function<void(BWAPI::Game*)> sunkenAction = [=](BWAPI::Game* g)->void
					{
					ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, depot->getTilePosition(), 1);
					};
					const std::function<bool(BWAPI::Game*)> sunkenCondition = [=](BWAPI::Game* g)->bool
					{
					if (BWAPI::Broodwar->getFrameCount() > buildingTime)
					return true;
					else
					return false;
					};
					BWAPI::Broodwar->registerEvent(sunkenAction, sunkenCondition, 1, 48);
					}
					
				}
				


				depotBalanceFlag = true;
				depotTrigMap[depot] = true;
			}
		}
	}
}


void InformationManager::checkEarlyRushDefend()
{

	if (needDefendCheck == false)
	{
		return;
	}

	//if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].begin())->isCompleted())
		//return;

	if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
		return;

	//zergling and sunken is just for early game defend
	//if (BWAPI::Broodwar->getFrameCount() > 12000 && BWAPI::Broodwar->getFrameCount() % 50 == 0)
		//return;



	int enemySupply = 0;
	for (std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>::iterator it = enemyAllBattleUnit.begin(); it != enemyAllBattleUnit.end(); it++)
	{
		if (it->first.canAttack() && !it->first.isWorker())
		{
			if (it->first == BWAPI::UnitTypes::Protoss_Zealot)
			{
				enemySupply += int(it->first.supplyRequired() * int(it->second.size()) * 0.9);
			}
			else
			{
				enemySupply += it->first.supplyRequired() * int(it->second.size());
			}
		}
	}

	int ourSupply = 0;
	int zerglingsCount = 0;
	for (std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>::iterator it = selfAllBattleUnit.begin(); it != selfAllBattleUnit.end(); it++)
	{
		if (it->first.canAttack() && !it->first.isWorker())
		{
			ourSupply += it->first.supplyRequired() * int(it->second.size());
			if (it->first == BWAPI::UnitTypes::Zerg_Zergling)
			{
				zerglingsCount += int(it->second.size());
			}
		}
	}

	//waiting to produce unit
	int waitingProductSupply = 0;
	int waitToBuildZergling = 0;
	bool hasDefendItem = false;
	std::vector<BWAPI::UnitType> waitingProduct = ProductionManager::Instance().getWaitingProudctUnit();
	BOOST_FOREACH(BWAPI::UnitType u, waitingProduct)
	{
		if (u.canAttack() && !u.isWorker())
		{
			if (u == BWAPI::UnitTypes::Zerg_Zergling)
			{
				waitToBuildZergling = 0;
				zerglingsCount += 2;
				waitingProductSupply += u.supplyRequired() * 2;
			}
			else
			{
				waitingProductSupply += u.supplyRequired();
			}
			
			hasDefendItem = true;
		}
		else if (u == BWAPI::UnitTypes::Zerg_Sunken_Colony || u == BWAPI::UnitTypes::Zerg_Creep_Colony)
		{
			hasDefendItem = true;
		}
		else
			continue;
			//waitingProductSupply += u.supplyRequired();
	}
	ourSupply += waitingProductSupply;

	
	int sunkenSupply = 0;
	int sunkenSize = selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() + waitToBuildSunker;

	int sunkenFactor = BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg ? 4 : 6;
	
	if (sunkenSize <= 4)
		sunkenSupply = int(sunkenSize * sunkenFactor * 0.75);
	else if (sunkenSize > 4 && sunkenSize <= 8)
		sunkenSupply = sunkenSize * sunkenFactor;
	else
		sunkenSupply = int(sunkenSize * sunkenFactor * 1.3);
	ourSupply += sunkenSupply;


	if (ourSupply < enemySupply)
	{

		int productionSupply = enemySupply - ourSupply;

		/*
		//if spire is morphing more than 2 / 3, do not trigger other unit morphing
		if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 &&
			(*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].begin())->getRemainingBuildTime() < BWAPI::UnitTypes::Zerg_Spire.buildTime() / 5)
			return;*/
		/*
		if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spire].begin())->isCompleted())
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Mutalisk, productionSupply / 4);
		else if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Hydralisk_Den].begin())->isCompleted())
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Hydralisk, productionSupply / 2);
		else*/ 
		if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Spawning_Pool].size() > 0 && (*selfAllBuilding[BWAPI::UnitTypes::Zerg_Spawning_Pool].begin())->isCompleted())
		{
			openingStrategy opening = StrategyManager::Instance().getCurrentopeningStrategy();

			//is natural complete
			bool isNatrualComplete = false;
			BOOST_FOREACH(BWAPI::Unit base, selfAllBuilding[BWAPI::UnitTypes::Zerg_Hatchery])
			{
				if (BWTA::getRegion(base->getPosition()) == BWTA::getRegion(getOurNatrualLocation()))
				{
					if (base->isCompleted())
						isNatrualComplete = true;
				}
			}

			if (sunkenSize >= 10)
				return;

			int sunkenBuildSupply = productionSupply;
			int count;
			if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
				count = sunkenBuildSupply / 6 + 1 >= 2 ? 2 : sunkenBuildSupply / 6 + 1;
			else
				count = sunkenBuildSupply / 8 + 1 >= 2 ? 2 : sunkenBuildSupply / 6 + 1;
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Drone, count);
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, getSunkenBuildingPosition(), count);
			waitToBuildSunker += count;

		}
		else
			return;
	}
}

void InformationManager::checkAirDrop()
{
	if (BWAPI::Broodwar->getFrameCount() > 12000)
		return;

	if (airDropTrigger && enemyAllBattleUnit[BWAPI::UnitTypes::Protoss_Shuttle].size() > 0 || enemyAllBattleUnit[BWAPI::UnitTypes::Terran_Dropship].size() > 0)
	{
		//ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Zergling, 3);
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::Broodwar->self()->getStartLocation(), 2);
		airDropTrigger = false;
	}
}


void InformationManager::checkAirDefend()
{
	if (chamberTrigger && (enemyAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0 || isEnemyHasFlyerAttacker()) 
		&& BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == 0)
	{
		BWAPI::Broodwar->printf("air defend trigger !!!");

		chamberTrigger = false;
		ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Drone, 3);
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->self()->getStartLocation(), 1);
	}

	if (airDefendTrigger && 
		((enemyAllBuilding[BWAPI::UnitTypes::Zerg_Spire].size() > 0)
		|| isEnemyHasFlyerAttacker()))
	{
		airDefendTrigger = false;
		
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Spore_Colony, BWAPI::Broodwar->self()->getStartLocation(), 1);
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Spore_Colony, selfNaturalBaseLocation, 1);
	}
}

void InformationManager::update()
{
	checkSelfNewDepotFinish();

	//checkAirDefend();

	checkAirDrop();

	checkEarlyRushDefend();


	updateEnemyUnitInfluenceMap();

	//for enemy buildings not destroy by us
	checkOccupiedDetail();
}


void InformationManager::checkOccupiedDetail()
{
	for (std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>::iterator it = enemyOccupiedDetail.begin(); it != enemyOccupiedDetail.end();)
	{
		for (std::map<BWAPI::Unit, buildingInfo>::iterator detailIt = it->second.begin(); detailIt != it->second.end();)
		{
			if (BWAPI::Broodwar->isVisible(BWAPI::TilePosition(detailIt->second.initPosition)))
			{
				bool isExist = false;
				for (auto u : BWAPI::Broodwar->getUnitsOnTile(detailIt->second.initPosition))
				{
					if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType() == detailIt->second.unitType)
					{
						isExist = true;
						break;
					}
				}
				if (isExist == false)
				{
					it->second.erase(detailIt++);
				}
				else
				{
					detailIt++;
				}
			}
			else
				detailIt++;
		}
		if (it->second.size() == 0)
		{
			enemyOccupiedDetail.erase(it++);
			occupiedRegions[1].erase(it->first);
		}
		else
			it++;
	}
}

BWAPI::TilePosition	InformationManager::GetNextExpandLocation()
{
	std::set<BWTA::Region*> enemyRegions;
	for (auto u : BWAPI::Broodwar->getAllUnits())
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy())
		{
			enemyRegions.insert(BWTA::getRegion(u->getPosition()));
		}
	}

	double closest = 999999999;
	BWAPI::TilePosition nextBase = BWAPI::TilePositions::None;
	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		/*
		//check if it is not our base
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
		{
			continue;
		}*/
		//already expand
		if (occupiedRegions[0].find(base->getRegion()) != occupiedRegions[0].end() || occupiedRegions[1].find(base->getRegion()) != occupiedRegions[1].end())
		{
			continue;
		}
		if (enemyNaturalBaseLocation != BWAPI::TilePositions::None && base->getTilePosition().getDistance(enemyNaturalBaseLocation) < 10)
		{
			continue;
		}
		if (enemyRegions.find(base->getRegion()) != enemyRegions.end())
		{
			continue;
		}

		if (base->getTilePosition().getDistance(selfStartBaseLocation) < closest && base->getGeysers().size() > 0 && !base->isIsland())
		{
			closest = base->getTilePosition().getDistance(selfStartBaseLocation);
			nextBase = base->getTilePosition();
		}
	}

	if (nextBase == BWAPI::TilePositions::None)
	{
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			/*
			//check if it is not our base
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
			{
				continue;
			}*/
			//already expand
			if (enemyRegions.find(base->getRegion()) != enemyRegions.end())
			{
				continue;
			}
			if (occupiedRegions[0].find(base->getRegion()) != occupiedRegions[0].end() || occupiedRegions[1].find(base->getRegion()) != occupiedRegions[1].end())
			{
				continue;
			}
			if (base->getTilePosition().getDistance(selfStartBaseLocation) < closest && !base->isIsland())
			{
				closest = base->getTilePosition().getDistance(selfStartBaseLocation);
				nextBase = base->getTilePosition();
			}
		}
	}

	return nextBase;
}


BWAPI::TilePosition	InformationManager::getOurNatrualLocation()
{
	if (selfNaturalBaseLocation != BWAPI::TilePositions::None)
		return selfNaturalBaseLocation;

	// first call
	double closest = 999999999;
	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		//check if it is not our base
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
		{
			continue;
		}
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < closest && base->getGeysers().size() > 0)
		{
			closest = base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation));
			selfNaturalBaseLocation = base->getTilePosition();
		}
	}

	double maxDist = 0;
	BWAPI::Position maxChokeCenter;
	BOOST_FOREACH(BWTA::Chokepoint* p, BWTA::getRegion(selfNaturalBaseLocation)->getChokepoints())
	{
		if (selfBaseUnit->getDistance(p->getCenter()) > maxDist)
		{
			maxDist = selfBaseUnit->getDistance(p->getCenter());
			maxChokeCenter = p->getCenter();
		}
	}
	selfNaturalChokePoint = maxChokeCenter;
	return selfNaturalBaseLocation;
}


BWAPI::TilePosition	InformationManager::getSunkenBuildingPosition()
{
	//generate selfNaturalChokePoint
	getOurNatrualLocation();

	if (selfOccupiedDetail.find(BWTA::getRegion(selfNaturalBaseLocation)) == selfOccupiedDetail.end() && baseSunkenBuildingPosition != BWAPI::TilePositions::None)
	{
		return baseSunkenBuildingPosition;
	}
	if (selfOccupiedDetail.find(BWTA::getRegion(selfNaturalBaseLocation)) != selfOccupiedDetail.end() && natrualSunkenBuildingPosition != BWAPI::TilePositions::None)
	{
		return natrualSunkenBuildingPosition;
	}


	if (selfOccupiedDetail.find(BWTA::getRegion(selfNaturalBaseLocation)) == selfOccupiedDetail.end())
	{
		BWAPI::Position baseChoke = BWTA::getNearestChokepoint(BWAPI::Broodwar->self()->getStartLocation())->getCenter();
		double2 direc = baseChoke - BWAPI::Position(selfStartBaseLocation);
		double2 direcNormal = direc / direc.len();

		/*
		BWAPI::Position desireLocation = BWAPI::Position(selfStartBaseLocation);
		while (desireLocation.getDistance(baseChoke) > 32 * 4)
		{
			if (BWAPI::Broodwar->hasCreep(desireLocation.x / 32, desireLocation.y / 32))
			{
				int targetx = desireLocation.x + int(direcNormal.x * 32 * 1);
				int targety = desireLocation.y + int(direcNormal.y * 32 * 1);
				desireLocation = BWAPI::Position(targetx, targety);
			}
			else
				break;
		}

		//int targetx = BWAPI::Position(selfStartBaseLocation).x + int(direcNormal.x * 32 * 8);
		//int targety = BWAPI::Position(selfStartBaseLocation).y + int(direcNormal.y * 32 * 8);

		baseSunkenBuildingPosition = BWAPI::TilePosition(desireLocation.x / 32, desireLocation.y / 32);*/

		int targetx = BWAPI::Position(selfStartBaseLocation).x + int(direcNormal.x * 32 * 8);
		int targety = BWAPI::Position(selfStartBaseLocation).y + int(direcNormal.y * 32 * 8);

		baseSunkenBuildingPosition = BWAPI::TilePosition(targetx / 32, targety / 32);
		return baseSunkenBuildingPosition;
	}
	else
	{
		double2 direc = selfNaturalChokePoint - BWAPI::Position(selfNaturalBaseLocation);
		double2 direcNormal = direc / direc.len();

		BWAPI::Position desireLocation = BWAPI::Position(selfNaturalBaseLocation);
		while (desireLocation.getDistance(selfNaturalChokePoint) > 32 * 2)
		{
			if (BWAPI::Broodwar->hasCreep(desireLocation.x / 32, desireLocation.y / 32))
			{
				int targetx = desireLocation.x + int(direcNormal.x * 32 * 1);
				int targety = desireLocation.y + int(direcNormal.y * 32 * 1);
				desireLocation = BWAPI::Position(targetx, targety);
			}
			else
				break;
		}

		//int targetx = BWAPI::Position(selfNaturalBaseLocation).x + int(direcNormal.x * 32 * 8);
		//int targety = BWAPI::Position(selfNaturalBaseLocation).y + int(direcNormal.y * 32 * 8);

		natrualSunkenBuildingPosition = BWAPI::TilePosition(desireLocation.x / 32, desireLocation.y / 32);
		return natrualSunkenBuildingPosition;
	}

	return BWAPI::TilePositions::None;
}


std::map<BWTA::Region*, int>& InformationManager::getBaseGroudDistance()
{
	if (baseGroundDistance.size() > 0)
		return baseGroundDistance;

	for (auto b : BWTA::getStartLocations())
	{
		if (b->getGroundDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())) < 90 )
			continue;
		baseGroundDistance[b->getRegion()] = int(b->getGroundDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())) / 32);
	}

	return baseGroundDistance;
}


std::map<BWTA::Region*, int>& InformationManager::getBaseAirDistance()
{
	if (baseAirDistance.size() > 0)
		return baseAirDistance;

	for (auto b : BWTA::getStartLocations())
	{
		if (b->getAirDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())) < 90)
			continue;
		baseAirDistance[b->getRegion()] = int(b->getTilePosition().getDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition()));
	}
	return baseAirDistance;
}


void InformationManager::setLocationEnemyBase(BWAPI::TilePosition Here)
{
	enemyStartBaseLocation = Here;
	//for we only detect enemy natural, we also update enemy base info
	updateOccupiedRegions(BWTA::getRegion(Here), BWAPI::Broodwar->enemy());

	double closest = 999999999;
	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		//check if it is not base
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(enemyStartBaseLocation)) < 90)
		{
			continue;
		}
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(enemyStartBaseLocation)) < closest && base->getGeysers().size() > 0)
		{
			closest = base->getGroundDistance(BWTA::getNearestBaseLocation(enemyStartBaseLocation));
			enemyNaturalBaseLocation = BWAPI::TilePosition(base->getPosition());
		}
	}
}


void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	updateUnit(unit);
}

void InformationManager::onUnitMorph(BWAPI::Unit unit)
{
 	updateUnit(unit);
}


void InformationManager::updateEnemyUnitInfluenceMap()
{
	//reset enemy unit's influence
	for (int i = 0; i < BWAPI::Broodwar->mapWidth(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapHeight(); j++)
		{
			enemyInfluenceMap[i][j].enemyUnitAirForce = 0;
			enemyInfluenceMap[i][j].enemyUnitGroundForce = 0;
			enemyInfluenceMap[i][j].enemyUnitDecayAirForce = 0;
			enemyInfluenceMap[i][j].enemyUnitDecayGroundForce = 0;
			
			if (int(enemyInfluenceMap[i][j].airForce) != 0 || int(enemyInfluenceMap[i][j].decayAirForce) != 0)
			{
				if (int(enemyInfluenceMap[i][j].airForce) != 0)
					BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(enemyInfluenceMap[i][j].airForce));
				//else
					//BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(enemyInfluenceMap[i][j].decayAirForce));
			}
		}
	}

	BOOST_FOREACH(BWAPI::Unit enemy, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (!enemy->getType().isBuilding() && enemy->getType().canAttack() && !enemy->getType().isWorker())
		{

			int attackRange = 0;
			if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
				attackRange = (enemy->getType().groundWeapon().maxRange() / 32) + 2;
			else
				attackRange = (enemy->getType().airWeapon().maxRange() / 32) + 2;
			
			int y_start = enemy->getTilePosition().y - attackRange > 0 ? enemy->getTilePosition().y - attackRange : 0;
			int y_end = enemy->getTilePosition().y + attackRange > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : enemy->getTilePosition().y + attackRange;
			
			int x_start = enemy->getTilePosition().x - attackRange > 0 ? enemy->getTilePosition().x - attackRange : 0;
			int x_end = enemy->getTilePosition().x + attackRange > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : enemy->getTilePosition().x + attackRange;
			
			for (int i = x_start; i <= x_end; i++)
			{
				for (int j = y_start; j <= y_end; j++)
				{
					if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].enemyUnitGroundForce += enemy->getType().groundWeapon().damageAmount();
					if (enemy->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].enemyUnitAirForce += enemy->getType().airWeapon().damageAmount();
				}
			}
		}
	}
	
	for (int i = 0; i < BWAPI::Broodwar->mapWidth(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapHeight(); j++)
		{
			if (int(enemyInfluenceMap[i][j].enemyUnitGroundForce) != 0 || int(enemyInfluenceMap[i][j].enemyUnitDecayGroundForce) != 0)
			{
				if (int(enemyInfluenceMap[i][j].enemyUnitGroundForce) != 0)
					BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(enemyInfluenceMap[i][j].enemyUnitGroundForce));
				//else
					//BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(enemyInfluenceMap[i][j].enemyUnitDecayGroundForce));
			}
		}
	}
}


void InformationManager::setInfluenceMap(BWAPI::Position initPosition, int attackRange, int groundDamage, int airDamage, bool addOrdestroy)
{
	// get the approximate center of the building
	
	double2 normalLength = double2(1, 0);
	std::set<BWAPI::TilePosition> alreadySetPosition;
	int startDegree = 0;
	while (startDegree < 360)
	{
		double2 rotateNormal(normalLength.rotateReturn(startDegree));
		for (int length = 1; length <= attackRange; length++)
		{
			double2 rotateVector(rotateNormal * length + initPosition);
			BWAPI::TilePosition tmp(int(rotateVector.x), int(rotateVector.y));
			if (int(tmp.x) >= 0 && int(tmp.x) < BWAPI::Broodwar->mapWidth() && int(tmp.y) >= 0 && int(tmp.y) < BWAPI::Broodwar->mapHeight()
				&& alreadySetPosition.find(tmp) == alreadySetPosition.end())
			{
				alreadySetPosition.insert(tmp);
				if (addOrdestroy)
				{
					enemyInfluenceMap[tmp.x][tmp.y].groundForce += groundDamage;
					enemyInfluenceMap[tmp.x][tmp.y].airForce += airDamage;
				}
				else
				{
					enemyInfluenceMap[tmp.x][tmp.y].groundForce -= groundDamage;
					enemyInfluenceMap[tmp.x][tmp.y].airForce -= airDamage;
				}
			}
		}
		startDegree += 5;
	}

	int decayRange = 4;
	startDegree = 0;
	double decayGroundValue = 0;
	double decayAirValue = 0;
	while (startDegree < 360)
	{
		double2 rotateNormal(normalLength.rotateReturn(startDegree));
		for (int length = attackRange + 1; length <= attackRange + decayRange; length++)
		{
			if (groundDamage > 0)
				decayGroundValue = groundDamage * (decayRange - (length - attackRange)) / double(decayRange);
			if (airDamage > 0)
				decayAirValue = airDamage * (decayRange - (length - attackRange)) / double(decayRange);
			double2 rotateVector(rotateNormal * length + initPosition);
			BWAPI::TilePosition tmp(int(rotateVector.x), int(rotateVector.y));
			if (int(tmp.x) >= 0 && int(tmp.x) < BWAPI::Broodwar->mapWidth() && int(tmp.y) >= 0 && int(tmp.y) < BWAPI::Broodwar->mapHeight()
				&& alreadySetPosition.find(tmp) == alreadySetPosition.end())
			{
				alreadySetPosition.insert(tmp);
				if (addOrdestroy)
				{
					enemyInfluenceMap[tmp.x][tmp.y].decayGroundForce += decayGroundValue;
					enemyInfluenceMap[tmp.x][tmp.y].decayAirForce += decayAirValue;
				}
				else
				{
					enemyInfluenceMap[tmp.x][tmp.y].decayGroundForce -= decayGroundValue;
					enemyInfluenceMap[tmp.x][tmp.y].decayAirForce -= decayAirValue;
				}
			}
		}
		startDegree += 5;
	}
}

void InformationManager::addUnitInfluenceMap(BWAPI::Unit unit, bool addOrdestroy)
{
	if (addOrdestroy && enemyAllBuilding[unit->getType()].find(unit) != enemyAllBuilding[unit->getType()].end())
		return;

	if (!addOrdestroy && enemyAllBuilding[unit->getType()].find(unit) == enemyAllBuilding[unit->getType()].end())
		return;

	if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony
		|| unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony
		|| unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{		
		int attackRange = 0;
		int airDamage = 0;
		int groundDamage = 0;
		int buildingWidth = unit->getType().tileWidth();
		int buildingHeight = unit->getType().tileHeight();
		int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;

		if (unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
		{
			attackRange = (BWAPI::UnitTypes::Terran_Marine.groundWeapon().maxRange() / 32) + maxSize + 2;

			groundDamage = BWAPI::UnitTypes::Terran_Marine.groundWeapon().damageAmount() * 3;
			airDamage = BWAPI::UnitTypes::Terran_Marine.airWeapon().damageAmount() * 3;
		}

		if (unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
		{
			attackRange = (unit->getType().groundWeapon().maxRange() / 32) + maxSize + 1;
			groundDamage = unit->getType().groundWeapon().damageAmount();
		}
		if (unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
		{
			attackRange = (unit->getType().airWeapon().maxRange() / 32) + maxSize + 1;
			airDamage = unit->getType().airWeapon().damageAmount();
		}

		double2 initPosition(unit->getTilePosition().x + buildingWidth / 2, unit->getTilePosition().y + buildingHeight / 2);
		setInfluenceMap(initPosition, attackRange, groundDamage, airDamage, addOrdestroy);
	}
} 



void InformationManager::updateUnit(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType().isBuilding())
		{
			if ((unit->getType().canAttack() || unit->getType() == BWAPI::UnitTypes::Terran_Bunker) && !unit->isCompleted())
				return;

			addUnitInfluenceMap(unit, true);
			//updateOccupiedRegions(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->enemy());
			addOccupiedRegionsDetail(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->enemy(), unit);

			if (unit->getType().isResourceDepot())
			{
				// update base unit for zerg
				BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
				{
					//check if it is not our base
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 3)
					{
						enemyAllBase.insert(unit);
						break;
					}
				}
			}

			if (enemyAllBuilding.find(unit->getType()) == enemyAllBuilding.end())
			{
				enemyAllBuilding[unit->getType()] = std::set<BWAPI::Unit>();
				enemyAllBuilding[unit->getType()].insert(unit);
			}
			else
				enemyAllBuilding[unit->getType()].insert(unit);
		}
		else
		{
			if (enemyAllBattleUnit.find(unit->getType()) == enemyAllBattleUnit.end())
			{
				enemyAllBattleUnit[unit->getType()] = std::set<BWAPI::Unit>();
				enemyAllBattleUnit[unit->getType()].insert(unit);
			}
			else
				enemyAllBattleUnit[unit->getType()].insert(unit);
		}
	}
	else if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		if (unit->getType().isBuilding())
		{
			selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Drone].erase(unit);

			addOccupiedRegionsDetail(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->self(), unit);
			if (unit->getType().isResourceDepot())
			{
				BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
				{
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 3)
					{
						selfAllBase.insert(unit);
						StrategyManager::Instance().setReward(unit->getType(), true);
						break;
					}
				}
			}

			if (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony && waitToBuildSunker > 0)
			{
				waitToBuildSunker--;
			}

			if (selfAllBuilding.find(unit->getType()) == selfAllBuilding.end())
			{
				selfAllBuilding[unit->getType()] = std::set<BWAPI::Unit>();
				selfAllBuilding[unit->getType()].insert(unit);
			}
			else
				selfAllBuilding[unit->getType()].insert(unit);
		}
		else
		{
			if (selfAllBattleUnit.find(unit->getType()) == selfAllBattleUnit.end())
			{
				selfAllBattleUnit[unit->getType()] = std::set<BWAPI::Unit>();
				selfAllBattleUnit[unit->getType()].insert(unit);
			}
			else
				selfAllBattleUnit[unit->getType()].insert(unit);
		}
	}
}


bool InformationManager::isEnemyHasInvisibleUnit()
{
	for (auto eU : enemyAllBattleUnit)
	{
		if (eU.first.hasPermanentCloak() 
			|| eU.first == BWAPI::UnitTypes::Zerg_Lurker
			|| eU.first == BWAPI::UnitTypes::Terran_Wraith)
		{
			return true;
		}
	}
	return false;
}


bool InformationManager::isEnemyHasFlyerAttacker()
{
	for (auto eU : enemyAllBattleUnit)
	{
		if (eU.first.isFlyer() && 
			(eU.first.airWeapon() != BWAPI::WeaponTypes::None))
		{
			return true;
		}
	}
	return false;
}

// TODO: update enemy occupied region
void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType().isBuilding())
		{
			addUnitInfluenceMap(unit, false);

			destroyOccupiedRegionsDetail(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->enemy(), unit);

			if (unit->getType().isResourceDepot())
			{
				BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
				{
					//check if it is not our base
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 3)
					{
						enemyAllBase.erase(unit);
						StrategyManager::Instance().setReward(unit->getType(), true);
						break;
					}
				}
			}
			if (enemyAllBuilding.find(unit->getType()) != enemyAllBuilding.end())
				enemyAllBuilding[unit->getType()].erase(unit);
		}
		else
		{
			if (enemyAllBattleUnit.find(unit->getType()) != enemyAllBattleUnit.end())
				enemyAllBattleUnit[unit->getType()].erase(unit);
		}
	}
	else if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		if (unit->getType().isBuilding())
		{
			destroyOccupiedRegionsDetail(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->self(), unit);

			if (unit->getType().isResourceDepot())
			{
				BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
				{
					//check if it is not our base
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 3)
					{
						//for ucb1 simulation
						if (BWTA::getRegion(unit->getTilePosition()) == BWTA::getRegion(getOurNatrualLocation())
							&& BWAPI::Broodwar->getFrameCount() < 8000)
						{
							enemyEarlyRushSuccess = true;
						}

						selfAllBase.erase(unit);
						StrategyManager::Instance().setReward(unit->getType(), false);
						break;
					}
				}
			}

			if (selfAllBuilding.find(unit->getType()) != selfAllBuilding.end())
				selfAllBuilding[unit->getType()].erase(unit);
		}
		else
		{
			if (selfAllBattleUnit.find(unit->getType()) != selfAllBattleUnit.end())
				selfAllBattleUnit[unit->getType()].erase(unit);
		}
	}
}


BWAPI::Unit InformationManager::GetOurBaseUnit()
{
	BWAPI::Unit minBase = NULL;
	int minDistance = 9999;
	for(auto u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType() == BWAPI::UnitTypes::Zerg_Lair
			|| u->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			minBase = u;
			break;
		}

		if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			if (u->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) < minDistance)
			{
				minDistance = u->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
				minBase = u;
			}
		}
	}
	return minBase;

}

BWAPI::Position InformationManager::GetEnemyBasePosition()
{
	if (enemyStartBaseLocation == BWAPI::TilePositions::None)
		return BWAPI::Positions::None;
	else
		return BWAPI::Position(enemyStartBaseLocation);
}

BWAPI::Position	InformationManager::GetEnemyNaturalPosition()
{
	if (enemyNaturalBaseLocation == BWAPI::TilePositions::None)
		return BWAPI::Positions::None;
	else
		return BWAPI::Position(enemyNaturalBaseLocation);
}

InformationManager& InformationManager::Instance()
{
	static InformationManager instance;
	return instance;
}


void InformationManager::updateOccupiedRegions(BWTA::Region * region, BWAPI::Player player)
{
	// if the region is valid (flying buildings may be in NULL regions)
	if (region)
	{
		// add it to the list of occupied regions
		if (player == BWAPI::Broodwar->self())
			occupiedRegions[0].insert(region);
		else
			occupiedRegions[1].insert(region);
	}
}

void InformationManager::addOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player player, BWAPI::Unit building)
{
	if (region)
	{
		if (player == BWAPI::Broodwar->self())
		{
			occupiedRegions[0].insert(region);

			if (selfOccupiedDetail.find(region) != selfOccupiedDetail.end())
				selfOccupiedDetail[region][building] = buildingInfo(building, building->getTilePosition(), building->isCompleted(), building->getType());
			else
			{
				selfOccupiedDetail[region] = std::map<BWAPI::Unit, buildingInfo>();
				selfOccupiedDetail[region][building] = buildingInfo(building, building->getTilePosition(), building->isCompleted(), building->getType());
			}
		}
		else
		{
			occupiedRegions[1].insert(region);
			if (enemyOccupiedDetail.find(region) != enemyOccupiedDetail.end())
				enemyOccupiedDetail[region][building] = buildingInfo(building, building->getTilePosition(), building->isCompleted(), building->getType());
			else
			{
				enemyOccupiedDetail[region] = std::map<BWAPI::Unit, buildingInfo>();
				enemyOccupiedDetail[region][building] = buildingInfo(building, building->getTilePosition(), building->isCompleted(), building->getType());
			}
		}
	}
}


void InformationManager::destroyOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player player, BWAPI::Unit building)
 {
	if (region)
	{
		if (player == BWAPI::Broodwar->self())
		{
			if (selfOccupiedDetail.find(region) != selfOccupiedDetail.end())
			{	
				selfOccupiedDetail[region].erase(building);
				if (selfOccupiedDetail[region].size() == 0)
				{
					occupiedRegions[0].erase(region);
					selfOccupiedDetail.erase(region);
				}
				/*
				if (selfOccupiedDetail[region].size() == 1 && (*selfOccupiedDetail[region].begin()).second.unitType.isRefinery())
				{
					selfOccupiedDetail[region].erase(selfOccupiedDetail[region].begin());
					occupiedRegions[0].erase(region);
					selfOccupiedDetail.erase(region);
				}*/
			}
		}
		else
		{
			if (enemyOccupiedDetail.find(region) != enemyOccupiedDetail.end())
			{
				enemyOccupiedDetail[region].erase(building);
				if (enemyOccupiedDetail[region].size() == 0)
				{
					occupiedRegions[1].erase(region);
					enemyOccupiedDetail.erase(region);
				} 

				/*
				if (enemyOccupiedDetail[region].size() == 1 && (*enemyOccupiedDetail[region].begin()).second.unitType.isRefinery())
				{
					enemyOccupiedDetail[region].erase(enemyOccupiedDetail[region].begin());
					occupiedRegions[1].erase(region);
					enemyOccupiedDetail.erase(region);
				}*/
			}
		}
	}
}


std::set<BWTA::Region *> & InformationManager::getOccupiedRegions(BWAPI::Player player)
{
	if (player == BWAPI::Broodwar->self())
	{
		//use the base as occupied flag
		occupiedRegions[0].clear();
		BOOST_FOREACH(BWAPI::Unit base, selfAllBase)
		{
			occupiedRegions[0].insert(BWTA::getRegion(base->getPosition()));
		}
		for (auto r : fakeOccupiedRegions)
		{
			occupiedRegions[0].insert(r);
		}

		return occupiedRegions[0];
	}
		
	else
		return occupiedRegions[1];
}


