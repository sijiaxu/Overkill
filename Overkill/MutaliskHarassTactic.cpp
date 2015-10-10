#pragma once
#include "MutaliskHarassTactic.h"


void MutaliskHarassTactic::onUnitShow(BWAPI::Unit* unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
			|| unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony
			|| unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)
		{
			newCannonCount++;
		}
		else if (unit->getType().airWeapon() != BWAPI::WeaponTypes::None && !unit->getType().isBuilding())
		{
			newAntiAirAnmyCount++;
		}

		if (BWAPI::Broodwar->getFrameCount() > triggerChangeTime && (newCannonCount >= 3 || newAntiAirAnmyCount >= 5) && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() > 0)
		{
			triggerChangeTime = BWAPI::Broodwar->getFrameCount() + 25 * 4;

			if (state == MOVE || state == BACKMOVE)
			{
				state = LOCATIONASSIGN;
			}

			for (std::map<BWAPI::Unit*, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
			{
				it->second.clear();
				std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(it->first->getTilePosition(), tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit->getTilePosition(), true, true);
				std::vector<BWAPI::Position> movePath;
				movePath.reserve(pathFind.size());
				BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
				{
					movePath.push_back(BWAPI::Position(p));
				}
				it->second = movePath;
			}

			newCannonCount = 0;
			newAntiAirAnmyCount = 0;
		}
	}
}

void MutaliskHarassTactic::addArmyUnit(BWAPI::Unit* unit)
{
	std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(unit->getTilePosition(), tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit->getTilePosition(), true, true);
	std::vector<BWAPI::Position> movePath;
	movePath.reserve(pathFind.size());
	BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
	{
		movePath.push_back(BWAPI::Position(p));
	}
	newAddArmy[unit] = movePath;
}

void MutaliskHarassTactic::locationAssign(MutaliskArmy* mutalisk, BWAPI::Position endPosition, tacticState nextState, bool nearPosition)
{
	unitMovePath.clear();
	moveComplete.clear();

	BWAPI::Unit* beginUnit = (*mutalisk->getUnits().begin()).unit;
	std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(beginUnit->getTilePosition(), BWAPI::TilePosition(endPosition), true, nearPosition);
	BWAPI::TilePosition targetPosition = pathFind.back();
	moveTarget = BWAPI::Position(targetPosition);
	BOOST_FOREACH(UnitState u, mutalisk->getUnits())
	{
		if (beginUnit->getDistance(u.unit) < 5 * 32)
		{
			unitMovePath[u.unit] = pathFind;
		}
		else
		{
			unitMovePath[u.unit] = aStarPathFinding(u.unit->getTilePosition(), targetPosition, true, true);
		}
	}

	state = nextState;
}


void MutaliskHarassTactic::locationMove(MutaliskArmy* mutalisk, tacticState nextState)
{

	if (moveComplete.size() > 0)
	{
		std::set<BWAPI::Unit*> nearbyUnits = (*moveComplete.begin())->getUnitsInRadius(12 * 32);
		BOOST_FOREACH(BWAPI::Unit* u, nearbyUnits)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				state = ATTACK;
				return;
			}
		}
	}
	
	if (unitMovePath.size() / double(mutalisk->getUnits().size()) <= 0.1)
	{
		state = nextState;
		return;
		/*
		if (mutalisk->flyGroup((*mutalisk->getUnits().begin()).unit->getPosition()))
		{
			state = nextState;
			return;
		}*/
	}
	else
	{
		for (std::map<BWAPI::Unit*, std::list<BWAPI::TilePosition>>::iterator it = unitMovePath.begin(); it != unitMovePath.end();)
		{
			if (!it->first->exists())
			{
				unitMovePath.erase(it++);
			}
			else
			{
				if (it->second.size() == 0)
				{
					moveComplete.insert(it->first);
					unitMovePath.erase(it++);
				}
				else
				{
					it->first->move(BWAPI::Position(*it->second.begin()));
					if (it->first->getDistance(BWAPI::Position(*it->second.begin())) < 32 * 4)
					{
						it->second.erase(it->second.begin());
					}
					it++;
				}
			}
		}
	}
}



void MutaliskHarassTactic::update()
{
	if (tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() == 0)
	{
		state = END;
		return;
	}

	newArmyRally();

	if (BWAPI::Broodwar->getFrameCount() % 100 == 0)
	{
		for (std::map<BWAPI::Unit*, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
		{
			it->second.clear();
			std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(it->first->getTilePosition(), tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit->getTilePosition(), true, true);
			std::vector<BWAPI::Position> movePath;
			movePath.reserve(pathFind.size());
			BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
			{
				movePath.push_back(BWAPI::Position(p));
			}
			it->second = movePath;
		}
	}

	MutaliskArmy* mutalisk = dynamic_cast<MutaliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]);
	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();

	std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>& regionDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	int antiairBuildingCount = 0;
	if (regionDetail.find(BWTA::getRegion(attackPosition)) != regionDetail.end())
	{
		std::map<BWAPI::Unit*, buildingInfo> buildinginfo = regionDetail[BWTA::getRegion(attackPosition)];
		for (std::map<BWAPI::Unit*, buildingInfo>::iterator it = buildinginfo.begin(); it != buildinginfo.end(); it++)
		{
			if (it->second.unitType.isBuilding() && (it->second.unitType.airWeapon() != BWAPI::WeaponTypes::None || it->second.unitType == BWAPI::UnitTypes::Terran_Bunker))
			{
				antiairBuildingCount++;
			}
		}
	}

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());
	if (ourSupply < antiairBuildingCount * 12)
	{
		state = END;
	}

	TimerManager::Instance().startTimer(TimerManager::MutaliskTac);

	// do tactic FSM update
	switch (state)
	{
	case LOCATIONASSIGN:
	{
		attackPosition = originAttackBase;
		locationAssign(mutalisk, originAttackBase, MOVE, true);
		break;
	}
	case MOVE:
	{
		locationMove(mutalisk, ATTACK);
		break;
	}
	
	case BACKLOCATIONASSIGN:
	{
		locationAssign(mutalisk, groupPosition, BACKMOVE, true);
		break;
	}
	case BACKMOVE:
	{
		locationMove(mutalisk, LOCATIONASSIGN);
		break;
	}

	case ATTACK:
	{	
		TimerManager::Instance().startTimer(TimerManager::MutaliskRetreat);
		int retreatFlag = needRetreat();
		TimerManager::Instance().stopTimer(TimerManager::MutaliskRetreat);

		if (retreatFlag != 0)
		{
			TimerManager::Instance().startTimer(TimerManager::MutaAttack);
			
			bool noTarget = mutalisk->harassAttack(attackPosition, 1);
			if (noTarget)
			{
				if (!hasEnemy())
					state = END;
			}

			/*
			if (retreatFlag == 1 && !BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
			{
				//only attack has-air-weapon enemy
				bool noTarget = mutalisk->harassAttack(attackPosition, 3);
			}
			else
			{
				bool noTarget = mutalisk->harassAttack(attackPosition, 1);
				if (noTarget)
				{
					if (!hasEnemy())
						state = END;
				}
			}*/
			TimerManager::Instance().stopTimer(TimerManager::MutaAttack);
		}
		break;
	} 
	}

	TimerManager::Instance().stopTimer(TimerManager::MutaliskTac);
}


bool MutaliskHarassTactic::hasEnemy()
{
	if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
		return true;

	std::set<BWAPI::Unit*>& units = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 12 * 32);
	bool needAttack = false;
	BOOST_FOREACH(BWAPI::Unit* u, units)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(attackPosition) && u->getType() != BWAPI::UnitTypes::Protoss_Observer)
		{
			return true;
		}
	}

	if (needAttack == false)
	{
		std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
		std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
		BWAPI::TilePosition curPosition = (*tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().begin()).unit->getTilePosition();
		BWAPI::Position maxAttackPosition;
		int maxDistance = 0;

		if (occupiedDetail.find(BWTA::getRegion(originAttackBase)) != occupiedDetail.end())
		{
			std::map<BWAPI::Unit*, buildingInfo >& buildingsDetail = occupiedDetail[BWTA::getRegion(originAttackBase)];
			for (std::map<BWAPI::Unit*, buildingInfo >::iterator it = buildingsDetail.begin(); it != buildingsDetail.end(); it++)
			{
				//if building can attack, return
				if (imInfo[BWAPI::TilePosition(it->second.initPosition).x()][BWAPI::TilePosition(it->second.initPosition).y()].airForce / 20 <= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() / 4)
					//&& it->second.initPosition.getDistance(BWAPI::TilePosition(curPosition)) > maxDistance)
				{
					if ((!it->second.unitType.isRefinery() && buildingsDetail.size() > 1) ||
						(buildingsDetail.size() == 1))
					{
						attackPosition = BWAPI::Position(it->second.initPosition);
						return true;
					}

					//maxAttackPosition = BWAPI::Position(it->second.initPosition);
					//maxDistance = int(it->second.initPosition.getDistance(BWAPI::TilePosition(curPosition)));
				}
			}
		}
	}
	return false;
}



int MutaliskHarassTactic::needRetreat()
{
	bool retreat = false;
	std::set<BWAPI::Unit*> nearbyUnits;
	int mutaliskCount = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
	BWAPI::Unit* firstUnit = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit;

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());
	
	//enemy unit is picked in the same range
	nearbyUnits = firstUnit->getUnitsInRadius(12 * 32);
	std::set<BWAPI::Unit*> middleUnitnearby = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits()[mutaliskCount / 2].unit->getUnitsInRadius(12 * 32);
	nearbyUnits.insert(middleUnitnearby.begin(), middleUnitnearby.end());
	std::set<BWAPI::Unit*> backUnitnearby = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().back().unit->getUnitsInRadius(12 * 32);
	nearbyUnits.insert(backUnitnearby.begin(), backUnitnearby.end());


	//std::vector<std::vector<gridInfo>>& enemyIm = InformationManager::Instance().getEnemyInfluenceMap();
	int inRegionEnemySupply = 0;
	int outRegionEnemySupply = 0;
	int cannonSupply = 0;
	BOOST_FOREACH(BWAPI::Unit* u, nearbyUnits)
	{
		if (u->getPlayer() == BWAPI::Broodwar->enemy() && (u->getType().airWeapon() != BWAPI::WeaponTypes::None || u->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			if (BWTA::getRegion(originAttackBase) == BWTA::getRegion(u->getPosition()))
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
			else
			{
				if (u->getType().isBuilding() && u->isCompleted())
				{
					if (u->getType() == BWAPI::UnitTypes::Terran_Bunker)
					{
						outRegionEnemySupply += 12;
						cannonSupply += 12;
					}
					else
					{
						outRegionEnemySupply += 8;
						cannonSupply += 8;
					}
				}
				else
				{
					outRegionEnemySupply += u->getType().supplyRequired();
				}
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
	 
	int enemySupply = inRegionEnemySupply + outRegionEnemySupply;

	//if has new army and 
	// our army less than enemy  and our army are not severely damaged
	//regroup at the middle position
	if (newAddArmy.size() > 0 &&
		(int(ourSupply * 0.7) < enemySupply
		|| lowHealthCount >= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * 0.7))
	{
		double2 distance((*tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().begin()).unit->getPosition() - (*newAddArmy.begin()).first->getPosition());
		int distanceLength = int(distance.len());
		groupPosition = double2((*newAddArmy.begin()).first->getPosition()) + distance.normal() * (distanceLength / 3);
		//groupPosition = (*newAddArmy.begin()).first->getPosition();
		addAllNewArmy();
		state = BACKLOCATIONASSIGN;
		return 0;
	}
	else if (int(ourSupply * 0.7) < enemySupply
		|| (lowHealthCount >= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * 0.7 && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired() * 0.3 <= enemySupply))
	{
		state = END;
		return 0;
	}
	else
		return 1;
}


bool MutaliskHarassTactic::isTacticEnd()
{
	if (state == END || state == WIN)
		return true;
	else
		return false;
}

MutaliskHarassTactic::MutaliskHarassTactic()
{
	state = LOCATIONASSIGN;
	targetPosition = BWAPI::Positions::None;
	newCannonCount = 0;
	newAntiAirAnmyCount = 0;
	groupPosition = BWAPI::Positions::None;
	triggerChangeTime = 0;
}

void MutaliskHarassTactic::setAttackPosition(BWAPI::Position targetPosition)
{
	attackPosition = targetPosition;
	originAttackBase = targetPosition;
}
