#pragma once
#include "BattleTactic.h"
#include "InformationManager.h"


bool BattleTactic::reGroup(BWAPI::Position & regroupPosition) const
{
	typedef std::pair<BWAPI::UnitType, BattleArmy*> mapType;

	bool isGroup = true;
	BOOST_FOREACH(mapType p, tacticArmy)
	{
		if (p.second->getUnits().size() == 0)
			continue;
		if (!p.second->reGroup(regroupPosition))
			isGroup = false;
	}
	return isGroup;
}


void BattleTactic::checkStartPositionEnemyInfo()
{
	int mutaliskCount = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
	
	if (mutaliskCount == 0)
	{
		startEnemyCanAttack = false;
	}
	
	BWAPI::Unit firstUnit = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit;

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());

	std::set<BWAPI::Unit> friendUnitNearBy;
	
	BWAPI::Unitset tmp = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit->getUnitsInRadius(12 * 32);
	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Mutalisk && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}
	
	BOOST_FOREACH(BWAPI::Unit u, friendUnitNearBy)
	{
		ourSupply += u->getType().supplyRequired();
	}

	if (ourSupply <= 24)
	{
		ourSupply = int(ourSupply * 0.6);
	}
	else if (ourSupply > 24 && ourSupply <= 48)
	{
		ourSupply = int(ourSupply * 0.7);
	}
	else if (ourSupply > 48 && ourSupply <= 100)
	{
		ourSupply = int(ourSupply * 0.9);
	}
	else
	{
		ourSupply = int(ourSupply * 1.1);
	}
	
	
	std::set<BWAPI::Unit> nearbyUnits;

	for (auto u : tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
		nearbyUnits.insert(percentUnits.begin(), percentUnits.end());
	}
	
	int inRegionEnemySupply = 0;
	int cannonSupply = 0;
	BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && (u->getType().airWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			if (u->getType().isBuilding() && u->isCompleted())
			{
				if (u->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					inRegionEnemySupply += 12;
					cannonSupply += 12;
				}
				else
				{
					inRegionEnemySupply += 8;
					cannonSupply += 8;
				}
			}
			else
			{
				inRegionEnemySupply += u->getType().supplyRequired();
			}
		}
	}

	int lowHealthCount = 0;
	BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		if (u.unit->getHitPoints() < u.unit->getType().maxHitPoints() / 3)
		{
			lowHealthCount++;
		}
	}
	
	int enemySupply = inRegionEnemySupply;
	
	if (ourSupply * 0.9 < enemySupply
	|| (lowHealthCount >= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * 0.7 && ourSupply * 0.2 <= enemySupply))
	{
		startEnemyCanAttack = false;
	}
	else
	{
		startEnemyCanAttack = true;
	}
		
	
}


void BattleTactic::setGroupPosition(BWAPI::UnitType armyType, tacticType tactic)
{
	if (tactic == HydraliskPushTactic)
	{
		BWAPI::Unit ourUnit = (*tacticArmy[armyType]->getUnits().begin()).unit;
		std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& enemyRegions = InformationManager::Instance().getEnemyOccupiedDetail();

		/*
		if (enemyRegions.find(BWTA::getRegion(attackPosition)) == enemyRegions.end())
		{
			groupPosition = attackPosition;
			return;
		}

		groupPosition = attackPosition;
		return;
		*/
		/*
		std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
		if (myRegions.size() > 1)
		{
			groupPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
		}
			
		else
			groupPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

		return;
		*/


		bool getGroupPosition = false;

		std::set<BWTA::Region*> candidateRegion;
		for (auto c : BWTA::getRegion(attackPosition)->getChokepoints())
		{
			const std::pair<BWTA::Region*, BWTA::Region*>& adjcentR = c->getRegions();
			candidateRegion.insert(adjcentR.first);
			candidateRegion.insert(adjcentR.second);
		}
		candidateRegion.erase(BWTA::getRegion(attackPosition));

		int searchCount = 0;
		while (true)
		{
			double minDistance = 99999;
			for (auto r : candidateRegion)
			{
				if (enemyRegions.find(r) == enemyRegions.end()) //|| enemyRegions[r].size() <= 1)
				{
					if (ourUnit->getDistance(BWAPI::Position(r->getCenter().x, r->getCenter().y)) < minDistance)
					{
						groupPosition = r->getCenter();
						minDistance = ourUnit->getDistance(BWAPI::Position(r->getCenter().x, r->getCenter().y));
					}
					getGroupPosition = true;
				}
			}
			searchCount += 1;

			//regroup at the first not enemy region
			if (getGroupPosition)
			{
				break;
			}
			else
			{
				if (searchCount >= 5)
				{
					groupPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
					break;
				}
				else
				{
					std::set<BWTA::Region*> tmp;
					for (auto r : candidateRegion)
					{
						for (auto c : r->getChokepoints())
						{
							tmp.insert(c->getRegions().first);
							tmp.insert(c->getRegions().second);
						}
					}

					for (auto r : candidateRegion)
					{
						tmp.erase(r);
					}

					candidateRegion = tmp;
				}
			}
		}
	}
}

void BattleTactic::setAttackPosition(BWAPI::Position targetPosition, tacticType tactic)
{
	attackPosition = originAttackBase = targetPosition;
	if (tactic == HydraliskPushTactic)
	{
		if (BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation())->isReachable(BWTA::getRegion(targetPosition)) == false)
		{
			state = END;
			return;
		}
	}
}


bool BattleTactic::hasEnemy()
{
	if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
		return true;

	// if current attack position has target, do not change original attack position
	BWAPI::Unitset& units = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 6 * 32);
	bool needAttack = false;
	BOOST_FOREACH(BWAPI::Unit u, units)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(attackPosition) && u->getType() != BWAPI::UnitTypes::Protoss_Observer)
		{
			if (u->getType().isFlyer() && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0
				&& tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits().size() == 0)
				continue;
			return true;
		}
	}

	// no enemy in attackPosition circle, looking for remaining building in the region
	if (needAttack == false)
	{
		std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
		if (occupiedDetail.find(BWTA::getRegion(originAttackBase)) != occupiedDetail.end())
		{
			std::map<BWAPI::Unit, buildingInfo >& buildingsDetail = occupiedDetail[BWTA::getRegion(originAttackBase)];
			for (std::map<BWAPI::Unit, buildingInfo >::iterator it = buildingsDetail.begin(); it != buildingsDetail.end(); it++)
			{
				attackPosition = BWAPI::Position(it->second.initPosition);
				return true;
				/*
				//TODO: for occupiedDetail refinery bugs..
				//if can see the refinery and it really exsit, attack it 
				if (it->second.unitType.isRefinery())
				{
					BWAPI::Unitset enemySet = BWAPI::Broodwar->getUnitsOnTile(it->second.initPosition);
					for (auto u : enemySet)
					{
						if (u->getPlayer() == BWAPI::Broodwar->enemy())
						{
							attackPosition = BWAPI::Position(it->second.initPosition);
							return true;
						}
					}
				}
				else
				{
					attackPosition = BWAPI::Position(it->second.initPosition);
					return true;
				}*/
			}
		}
	}

	return false;
}



bool BattleTactic::unitNearChokepoint(BWAPI::Unit unit) const
{
	BOOST_FOREACH(BWTA::Chokepoint * choke, BWTA::getChokepoints())
	{
		if (unit->getDistance(choke->getCenter()) < 80)
		{
			return true;
		}
	}

	return false;
}

void BattleTactic::addArmyUnit(BWAPI::Unit unit)	
{
	newAddArmy[unit] = std::vector<BWAPI::Position>();
}


void BattleTactic::newArmyRally()
{
	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end();)
	{
		if (it->second.size() == 0)
		{
			tacticArmy[it->first->getType()]->addUnit(it->first);
			newAddArmy.erase(it++);
		}
		else
		{
			BattleArmy::smartAttackMove(it->first, *it->second.begin());
			if (it->first->getDistance((*it->second.begin())) < 32 * 12)
			{
				it->second.erase(it->second.begin());
			}
			it++;
		}
		/*
		if (state != RETREAT)
		{
			if (it->second.size() == 0)
			{
				tacticArmy[it->first->getType()]->addUnit(it->first);
				newAddArmy.erase(it++);
			}
			else
			{
				it->first->attack((*it->second.begin()));
				if (it->first->getDistance((*it->second.begin())) < 32 * 4)
				{
					it->second.erase(it->second.begin());
				}
				it++;
			}
		}
		else
			it++;*/
	}
}


BattleTactic::BattleTactic()
{
	tacticArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();

	endByDefend = false;
	groupStartTime = 0;
	groupStatus = false;

	BWAPI::Position rallyPosition;
	std::set<BWTA::Region *>& myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	if (myRegions.find(BWTA::getRegion(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
		rallyPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		rallyPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	groupPosition = rallyPosition;
}

BattleTactic::~BattleTactic()
{
	typedef std::pair<BWAPI::UnitType, BattleArmy*> mapType;
	BOOST_FOREACH(mapType p, tacticArmy)
	{
		delete p.second;
	}
	//tactic end
}


BWAPI::Position BattleTactic::groupArmyCheck(bool& needGroup)
{
	int avgX = 0;
	int avgY = 0;
	int totalCount = 0;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				totalCount += 1;
				avgX += u.unit->getPosition().x;
				avgY += u.unit->getPosition().y;
			}
		}
	}

	avgX = avgX / totalCount;
	avgY = avgY / totalCount;
	BWAPI::Position centerPoint(avgX, avgY);

	int distancethreshhold = ((totalCount / 12) + 3) * 32;
	int groupCount = 0;
	int minDistance = 9999999;
	BWAPI::Position minPosition = BWAPI::Positions::None;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (u.unit->getDistance(centerPoint) <= distancethreshhold)
				{
					groupCount += 1;
				}

				if (u.unit->getDistance(centerPoint) < minDistance)
				{
					minDistance = u.unit->getDistance(centerPoint);
					minPosition = u.unit->getPosition();
				}
			}
		}
	}

	BWAPI::Broodwar->drawCircleMap(minPosition.x, minPosition.y, distancethreshhold, BWAPI::Colors::Red, false);

	if (groupCount * 10 / totalCount >= 9)
	{
		needGroup = false;
	}
	else
	{
		needGroup = true;
	}

	return minPosition;
}


bool BattleTactic::checkGroupStatus(BWAPI::Position target)
{
	int totalCount = 0;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				totalCount += 1;
			}
		}
	}
	int groupCount = 0;

	if (totalCount <= 50)
	{
		int distancethreshhold = ((totalCount / 12) + 3) * 32;
		BWAPI::Broodwar->drawCircleMap(target.x, target.y, distancethreshhold, BWAPI::Colors::Red, false);

		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				for (auto u : army.second->getUnits())
				{
					if (u.unit->getDistance(groupPosition) <= distancethreshhold)
					{
						groupCount += 1;
					}
				}
			}
		}

		if (groupCount * 10 / totalCount >= 8)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		for (auto army : tacticArmy)
		{
			if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
			{
				for (auto u : army.second->getUnits())
				{
					if (BWTA::getRegion(u.unit->getPosition()) == BWTA::getRegion(target))
					{
						groupCount += 1;
					}
				}
			}
		}
		if (groupCount * 10 / totalCount >= 7)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void BattleTactic::addAllNewArmy()
{
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	BWAPI::Position moveBackBase;
	if (myRegions.size() > 1)
		moveBackBase = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		moveBackBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end();)
	{
		it->first->move(moveBackBase);
		tacticArmy[it->first->getType()]->addUnit(it->first);
		newAddArmy.erase(it++);
	}
}


void BattleTactic::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL || tacticArmy.find(unit->getType()) == tacticArmy.end())
		return;

	tacticArmy[unit->getType()]->removeUnit(unit);
	/*
	std::vector<UnitState>& army = tacticArmy[unit->getType()]->getUnits();
	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit == unit)
		{
			army.erase(it);
			break;
		}
	}*/

	for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
	{
		if (it->first == unit)
		{
			newAddArmy.erase(it);
			break;
		}
	}
}



