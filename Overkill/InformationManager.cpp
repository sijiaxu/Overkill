#include "InformationManager.h"

#define SELF_INDEX 0
#define ENEMY_INDEX 1

// constructor
InformationManager::InformationManager()
{

	enemyInfluenceMap.resize(BWAPI::Broodwar->mapHeight(), std::vector<gridInfo>(BWAPI::Broodwar->mapWidth(), gridInfo()));

	selfNaturalBaseLocation = BWAPI::TilePositions::None;
	selfStartBaseLocation = BWAPI::Broodwar->self()->getStartLocation();
	BOOST_FOREACH(BWAPI::Unit* u, BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			selfAllBase.insert(u);
	}
	enemyStartBaseLocation = BWAPI::TilePositions::None;
	firstColonyLocation = BWAPI::TilePositions::None;

	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Zergling] = std::set<BWAPI::Unit*>();
	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Hydralisk] = std::set<BWAPI::Unit*>();
	selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Mutalisk] = std::set<BWAPI::Unit*>();

	selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony] = std::set<BWAPI::Unit*>();
	selfAllBuilding[BWAPI::UnitTypes::Zerg_Spore_Colony] = std::set<BWAPI::Unit*>();
	selfAllBuilding[BWAPI::UnitTypes::Zerg_Creep_Colony] = std::set<BWAPI::Unit*>();

	defendTrig = false;

	BOOST_FOREACH(BWAPI::Unit* u, selfAllBase)
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
	
}


void InformationManager::checkSelfNewDepotFinish()
{
	BOOST_FOREACH(BWAPI::Unit* depot, selfAllBase)
	{
		//new depot finish
		if (BWAPI::Broodwar->getFrameCount() > 300 && depotTrigMap.find(depot) == depotTrigMap.end() && depot->isCompleted())
		{
			//waiting for the creep and vision expand
			if (depotBalanceFlag)
			{
				depotBalanceFlag = false;
				waitforDepotTime = BWAPI::Broodwar->getFrameCount() + 30 * 5;
			}
			if (BWAPI::Broodwar->getFrameCount() > waitforDepotTime)
			{
				WorkerManager::Instance().balanceWorkerOnDepotComplete(depot);
				if (BWTA::getRegion(selfNaturalBaseLocation) == BWTA::getRegion(depot->getPosition()))
					ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::TilePosition(selfNaturalChokePoint), 1);
				else
					ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, depot->getTilePosition(), 1);

				depotBalanceFlag = true;
				depotTrigMap[depot] = true;
			}
		}
	}

	//trig for defend rush building when natural complete
	BOOST_FOREACH(BWAPI::Unit* depot, selfAllBase)
	{
		if (depot->isCompleted() && depot->getDistance(BWAPI::Position(getOurNatrualLocation())) < 5 * 32)
		{
			if (natrualFlag)
			{
				natrualFlag = false;
				waitTime = BWAPI::Broodwar->getFrameCount() + 30 * 3;
			}
			if (BWAPI::Broodwar->getFrameCount() > waitTime && !zealotRushFlag && zealotRushTrig)
			{
				ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Drone, 2);
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::TilePosition(selfNaturalChokePoint), 2);
				
				zealotRushTrig = false;
			}
		}
	}
}


void InformationManager::checkEarlyRush()
{
	if (BWAPI::Broodwar->getFrameCount() < 6000 && zealotRushFlag)
	{
		if (enemyAllBuilding[BWAPI::UnitTypes::Protoss_Gateway].size() >= 2 || enemyAllBattleUnit[BWAPI::UnitTypes::Protoss_Zealot].size() >= 3
			|| enemyAllBuilding[BWAPI::UnitTypes::Terran_Marine].size() >= 6)
		{
			BWAPI::Broodwar->printf("detect early zealot rush");
			zealotRushFlag = false;
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Zergling, 3);
		}
	}
}


void InformationManager::checkMidRush()
{
	if (BWAPI::Broodwar->getFrameCount() > 6000 && BWAPI::Broodwar->getFrameCount() < 12000 && midRushFlag)
	{
		int enemySupply = 0;
		typedef std::map<BWAPI::UnitType, std::set<BWAPI::Unit*>>::value_type battleType;
		BOOST_FOREACH(battleType units, enemyAllBattleUnit)
		{
			if (units.first.canAttack() && !units.first.isWorker())
			{
				enemySupply += units.first.supplyRequired() * units.second.size();
			}
		}

		int ourSupply = 0;
		BOOST_FOREACH(battleType units, selfAllBattleUnit)
		{
			if (units.first.canAttack() && !units.first.isWorker())
			{
				ourSupply += units.first.supplyRequired() * units.second.size();
			}
		}

		if (enemySupply > (ourSupply + int(selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony].size()) * 4 * 2) * 1.2)
		{
			ProductionManager::Instance().triggerUnit(BWAPI::UnitTypes::Zerg_Drone, 1);

			if (selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony].size() > 0)
			{
				BWAPI::TilePosition buildingLocation = BWAPI::TilePositions::None;
				BOOST_FOREACH(BWAPI::Unit* sunken, selfAllBuilding[BWAPI::UnitTypes::Zerg_Sunken_Colony])
				{
					if (BWTA::getRegion(sunken->getTilePosition()) == BWTA::getRegion(selfNaturalBaseLocation))
					{
						buildingLocation = sunken->getTilePosition();
						break;
					}
				}
				if (buildingLocation != BWAPI::TilePositions::None)
					ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, buildingLocation, 1);
				else
					ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::TilePosition(selfNaturalChokePoint), 1);
			}
			else
			{
				ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::TilePosition(selfNaturalChokePoint), 1);
			}
			midRushFlag = false;
		}
	}
}


void InformationManager::checkVeryEarlyRush()
{
	if (BWAPI::Broodwar->getFrameCount() < 5000 && earlyRush)
	{
		std::set<BWTA::Region *> myOccupied = occupiedRegions[0];
		myOccupied.insert(BWTA::getRegion(getOurNatrualLocation()));

		BWAPI::TilePosition trueNatrual;
		double closest = 999999999;
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			//check if it is not our base
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
			{
				continue;
			}
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < closest)
			{
				closest = base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation));
				trueNatrual = base->getTilePosition();
			}
		}
		myOccupied.insert(BWTA::getRegion(trueNatrual));

		bool hasEnemy = false;
		BWAPI::TilePosition enemyPosition = BWAPI::TilePositions::None;
		BOOST_FOREACH(BWAPI::Unit * enemyUnit, BWAPI::Broodwar->enemy()->getUnits())
		{
			if (enemyUnit->getType().isWorker() || enemyUnit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
				continue;
			//TODO: invisible unit can be get from api, bug
			if (myOccupied.find(BWTA::getRegion(BWAPI::TilePosition(enemyUnit->getPosition()))) != myOccupied.end()
				&& (enemyUnit->getType().canAttack() || enemyUnit->getType() == BWAPI::UnitTypes::Terran_Bunker))
			{
				hasEnemy = true;
				enemyPosition = enemyUnit->getTilePosition();
			}
		}

		if (hasEnemy) /*|| 
			(enemyAllBuilding.find(BWAPI::UnitTypes::Terran_Barracks) != enemyAllBuilding.end()
			&& (BWTA::getRegion((*enemyAllBuilding[BWAPI::UnitTypes::Terran_Barracks].begin())->getPosition()) == BWTA::getRegion(selfNaturalBaseLocation)
			|| BWTA::getRegion((*enemyAllBuilding[BWAPI::UnitTypes::Terran_Barracks].begin())->getPosition()) == BWTA::getRegion(trueNatrual))))*/
		{
			BWAPI::Broodwar->printf("detect early rush");
			earlyRush = false;
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Sunken_Colony, enemyPosition, 1);
		}
	}
}

void InformationManager::update()
{
	checkSelfNewDepotFinish();

	checkEarlyRush();

	checkMidRush();

	checkVeryEarlyRush();

	//if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
	updateEnemyUnitInfluenceMap();

	checkOccupiedDetail();

}


void InformationManager::checkOccupiedDetail()
{
	for (std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>::iterator it = enemyOccupiedDetail.begin(); it != enemyOccupiedDetail.end();)
	{
		for (std::map<BWAPI::Unit*, buildingInfo>::iterator detailIt = it->second.begin(); detailIt != it->second.end();)
		{
			if (BWAPI::Broodwar->isVisible(BWAPI::TilePosition(detailIt->second.initPosition))
				&& !detailIt->first->exists())
			{
				it->second.erase(detailIt++);
			}
			else
				detailIt++;
		}
		if (it->second.size() == 0)
		{
			enemyOccupiedDetail.erase(it++);
		}
		else
			it++;
	}
}

BWAPI::TilePosition	InformationManager::GetNextExpandLocation()
{
	double closest = 999999999;
	BWAPI::TilePosition nextBase = BWAPI::TilePositions::None;
	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		//check if it is not our base
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
		{
			continue;
		}
		//already expand
		if (occupiedRegions[0].find(base->getRegion()) != occupiedRegions[0].end() || occupiedRegions[1].find(base->getRegion()) != occupiedRegions[1].end())
		{
			continue;
		}
		if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < closest && base->getGeysers().size() > 0)
		{
			closest = base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation));
			nextBase = base->getTilePosition();
		}
	}

	if (nextBase == BWAPI::TilePositions::None)
	{
		BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
		{
			//check if it is not our base
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < 90)
			{
				continue;
			}
			//already expand
			if (occupiedRegions[0].find(base->getRegion()) != occupiedRegions[0].end() || occupiedRegions[1].find(base->getRegion()) != occupiedRegions[1].end())
			{
				continue;
			}
			if (base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation)) < closest )
			{
				closest = base->getGroundDistance(BWTA::getNearestBaseLocation(selfStartBaseLocation));
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


void InformationManager::setLocationEnemyBase(BWAPI::TilePosition Here)
{
	enemyStartBaseLocation = Here;
	//for we only detect enemy natural, we also update enemy base info
	updateOccupiedRegions(BWTA::getRegion(Here), BWAPI::Broodwar->enemy());

	double closest = 999999999;
	BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
	{
		//check if it is not our base
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


void InformationManager::onUnitShow(BWAPI::Unit * unit)
{
	updateUnit(unit);
}

void InformationManager::onUnitMorph(BWAPI::Unit * unit)
{
 	updateUnit(unit);
}

 

void InformationManager::updateEnemyUnitInfluenceMap()
{
	//reset enemy unit's influence
	for (int i = 0; i < BWAPI::Broodwar->mapHeight(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapWidth(); j++)
		{
			enemyInfluenceMap[i][j].enemyUnitAirForce = 0;
			enemyInfluenceMap[i][j].enemyUnitGroundForce = 0;
			
			if (int(enemyInfluenceMap[i][j].airForce) != 0 || int(enemyInfluenceMap[i][j].decayAirForce) != 0)
			{
				//if (int(enemyInfluenceMap[i][j].airForce) != 0)
					//BWAPI::Broodwar->drawTextMap(j * 32, i * 32, "%d", int(enemyInfluenceMap[i][j].airForce));
				//else
					//BWAPI::Broodwar->drawTextMap(j * 32, i * 32, "%d", int(enemyInfluenceMap[i][j].decayAirForce));
			}
		}
	}

	BOOST_FOREACH(BWAPI::Unit* enemy, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (!enemy->getType().isBuilding() && enemy->getType().canAttack() && !enemy->getType().isWorker())
		{
			int attackRange = 0;
			if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
				attackRange = (enemy->getType().groundWeapon().maxRange() / 32) + 2;
			else
				attackRange = (enemy->getType().airWeapon().maxRange() / 32) + 2;

			int y_start = enemy->getTilePosition().y() - attackRange > 0 ? enemy->getTilePosition().y() - attackRange : 0;
			int y_end = enemy->getTilePosition().y() + attackRange > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : enemy->getTilePosition().y() + attackRange;

			int x_start = enemy->getTilePosition().x() - attackRange > 0 ? enemy->getTilePosition().x() - attackRange : 0;
			int x_end = enemy->getTilePosition().x() + attackRange > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : enemy->getTilePosition().x() + attackRange;

			for (int i = y_start; i <= y_end; i++)
			{
				for (int j = x_start; j <= x_end; j++)
				{
					if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].enemyUnitGroundForce += enemy->getType().groundWeapon().damageAmount();
					if (enemy->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].enemyUnitAirForce += enemy->getType().airWeapon().damageAmount();
				}
			}
		}
	}
	/*
	for (int i = 0; i < BWAPI::Broodwar->mapHeight(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapWidth(); j++)
		{
			if (enemyInfluenceMap[i][j].enemyUnitAirForce > 0)
				BWAPI::Broodwar->drawTextMap(j * 32, i * 32, "%d", int(enemyInfluenceMap[i][j].enemyUnitGroundForce));
		}
	}*/
}


void InformationManager::addUnitInfluenceMap(BWAPI::Unit * unit, bool addOrdestroy)
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
			attackRange = (BWAPI::UnitTypes::Terran_Marine.groundWeapon().maxRange() / 32) + maxSize + 1;
			groundDamage = BWAPI::UnitTypes::Terran_Marine.groundWeapon().damageAmount() * 4;
			airDamage = BWAPI::UnitTypes::Terran_Marine.airWeapon().damageAmount() * 4;
		}
		else
		{
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
		}

		// get the approximate center of the building
		double2 initPosition(unit->getTilePosition().x() + buildingWidth / 2, unit->getTilePosition().y() + buildingHeight / 2);
		double2 normalLength = double2(1, 0);
		std::set<BWAPI::TilePosition> alreadySetPosition;
		for (int length = 1; length <= attackRange; length++)
		{
			int startDegree = 0;
			while (startDegree < 360)
			{
				double2 rotateVector((normalLength * length).rotateReturn(startDegree) + initPosition);
				BWAPI::TilePosition tmp(int(rotateVector.x), int(rotateVector.y));
				if (int(tmp.x()) >= 0 && int(tmp.x()) < BWAPI::Broodwar->mapWidth() && int(tmp.y()) >= 0 && int(tmp.y()) < BWAPI::Broodwar->mapHeight()
					&& alreadySetPosition.find(tmp) == alreadySetPosition.end())
				{
					alreadySetPosition.insert(tmp);
					if (addOrdestroy)
					{
						enemyInfluenceMap[tmp.y()][tmp.x()].groundForce += groundDamage;
						enemyInfluenceMap[tmp.y()][tmp.x()].airForce += airDamage;
					}
					else
					{
						enemyInfluenceMap[tmp.y()][tmp.x()].groundForce -= groundDamage;
						enemyInfluenceMap[tmp.y()][tmp.x()].airForce -= airDamage;
					}
					/*
					if (addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].groundForce += unit->getType().groundWeapon().damageAmount();

					if (addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].airForce += unit->getType().airWeapon().damageAmount();

					if (!addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].groundForce -= unit->getType().groundWeapon().damageAmount();
					if (!addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].airForce -= unit->getType().airWeapon().damageAmount();*/
				}
				startDegree += 5;
			}
		}
		
		int decayRange = 4;
		for (int length = attackRange + 1; length <= attackRange + decayRange; length++)
		{
			int startDegree = 0;
			while (startDegree < 360)
			{
				double decayValue = (decayRange - (length - attackRange)) / double(decayRange);
				double2 rotateVector((normalLength * length).rotateReturn(startDegree) + initPosition);
				BWAPI::TilePosition tmp(int(rotateVector.x), int(rotateVector.y));
				if (int(tmp.x()) >= 0 && int(tmp.x()) < BWAPI::Broodwar->mapWidth() && int(tmp.y()) >= 0 && int(tmp.y()) < BWAPI::Broodwar->mapHeight()
					&& alreadySetPosition.find(tmp) == alreadySetPosition.end())
				{
					alreadySetPosition.insert(tmp);
					if (addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].decayGroundForce += decayValue;
					if (addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].decayAirForce += decayValue;

					if (!addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].decayGroundForce -= decayValue;
					if (!addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[tmp.y()][tmp.x()].decayAirForce -= decayValue;
				}
				startDegree += 5;
			}
		}



		/*
		int buildingWidth = unit->getType().tileWidth();
		int buildingHeight = unit->getType().tileHeight();
		int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;
		int attackRange = 0;
		if (unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
			attackRange = (unit->getType().groundWeapon().maxRange() / 32) + maxSize;
		else
			attackRange = (unit->getType().airWeapon().maxRange() / 32) + maxSize;

		int decayRange = 5;
		// get the approximate center of the building
		BWAPI::TilePosition initPosition(unit->getTilePosition().x() + buildingWidth / 2, unit->getTilePosition().y() + buildingHeight / 2);

		int y_start = initPosition.y() - attackRange - decayRange > 0 ? initPosition.y() - attackRange - decayRange : 0;
		int y_end = initPosition.y() + attackRange + decayRange > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : initPosition.y() + attackRange + decayRange;

		int x_start = initPosition.x() - attackRange - decayRange > 0 ? initPosition.x() - attackRange - decayRange : 0;
		int x_end = initPosition.x() + attackRange + decayRange > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : initPosition.x() + attackRange + decayRange;

		for (int i = y_start; i <= y_end; i++)
		{
			for (int j = x_start; j <= x_end; j++)
			{
				int xDiff = std::abs(j - initPosition.x());
				int yDiff = std::abs(i - initPosition.y());
				int distance = std::max(xDiff, yDiff);

				if (distance <= attackRange)
				{
					if (addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].groundForce += unit->getType().groundWeapon().damageAmount();

					if (addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].airForce += unit->getType().airWeapon().damageAmount();

					if (!addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].groundForce -= unit->getType().groundWeapon().damageAmount();
					if (!addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].airForce -= unit->getType().airWeapon().damageAmount();
				}
				else
				{
					double decayValue = (decayRange - (distance - attackRange)) * 10 / double(decayRange) ;
					if (addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].decayGroundForce += decayValue;
					if (addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].decayAirForce += decayValue;

					if (!addOrdestroy && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].decayGroundForce -= decayValue;
					if (!addOrdestroy && unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
						enemyInfluenceMap[i][j].decayAirForce -= decayValue;
				}
			}
		}*/
	}
} 



void InformationManager::updateUnit(BWAPI::Unit * unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType().isBuilding())
		{
			if (unit->getType().canAttack() && !unit->isCompleted())
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
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 5)
					{
						enemyAllBase.insert(unit);
						break;
					}
				}
			}

			if (enemyAllBuilding.find(unit->getType()) == enemyAllBuilding.end())
			{
				enemyAllBuilding[unit->getType()] = std::set<BWAPI::Unit*>();
				enemyAllBuilding[unit->getType()].insert(unit);
			}
			else
				enemyAllBuilding[unit->getType()].insert(unit);
		}
		else
		{
			if (enemyAllBattleUnit.find(unit->getType()) == enemyAllBattleUnit.end())
			{
				enemyAllBattleUnit[unit->getType()] = std::set<BWAPI::Unit*>();
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
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 5)
					{
						selfAllBase.insert(unit);
						break;
					}
				}
			}

			if (selfAllBuilding.find(unit->getType()) == selfAllBuilding.end())
			{
				selfAllBuilding[unit->getType()] = std::set<BWAPI::Unit*>();
				selfAllBuilding[unit->getType()].insert(unit);
			}
			else
				selfAllBuilding[unit->getType()].insert(unit);
		}
		else
		{
			if (selfAllBattleUnit.find(unit->getType()) == selfAllBattleUnit.end())
			{
				selfAllBattleUnit[unit->getType()] = std::set<BWAPI::Unit*>();
				selfAllBattleUnit[unit->getType()].insert(unit);
			}
			else
				selfAllBattleUnit[unit->getType()].insert(unit);
		}
	}
}

// TODO: update enemy occupied region
void InformationManager::onUnitDestroy(BWAPI::Unit * unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType().isBuilding())
		{
			if (unit->getType().canAttack() && !unit->isCompleted())
				return;

			addUnitInfluenceMap(unit, false);

			destroyOccupiedRegionsDetail(BWTA::getRegion(unit->getPosition()), BWAPI::Broodwar->enemy(), unit);

			if (unit->getType().isResourceDepot())
			{
				BOOST_FOREACH(BWTA::BaseLocation* base, BWTA::getBaseLocations())
				{
					//check if it is not our base
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 5)
					{
						enemyAllBase.erase(unit);
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
					if (base->getTilePosition().getDistance(unit->getTilePosition()) < 5)
					{
						selfAllBase.erase(unit);
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

BWAPI::Unit* InformationManager::GetOurBaseUnit()
{
	if (selfBaseUnit->exists())
		return selfBaseUnit;
	else
		return NULL;
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


void InformationManager::updateOccupiedRegions(BWTA::Region * region, BWAPI::Player * player)
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

void InformationManager::addOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player * player, BWAPI::Unit* building)
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
				selfOccupiedDetail[region] = std::map<BWAPI::Unit*, buildingInfo>();
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
				enemyOccupiedDetail[region] = std::map<BWAPI::Unit*, buildingInfo>();
				enemyOccupiedDetail[region][building] = buildingInfo(building, building->getTilePosition(), building->isCompleted(), building->getType());
			}
		}
	}
}


void InformationManager::destroyOccupiedRegionsDetail(BWTA::Region * region, BWAPI::Player * player, BWAPI::Unit* building)
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
					
			}
		}
	}
}


std::set<BWTA::Region *> & InformationManager::getOccupiedRegions(BWAPI::Player * player)
{
	if (player == BWAPI::Broodwar->self())
		return occupiedRegions[0];
	else
		return occupiedRegions[1];
}


