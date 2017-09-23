#pragma once
#include "MutaliskHarassTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"


void MutaliskHarassTactic::onUnitShow(BWAPI::Unit unit)
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

			if (state == MOVE)
			{
				state = LOCATIONASSIGN;
			}

			for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
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

void MutaliskHarassTactic::addArmyUnit(BWAPI::Unit unit)
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

	BWAPI::Unit beginUnit = (*mutalisk->getUnits().begin()).unit;
	std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(beginUnit->getTilePosition(), BWAPI::TilePosition(endPosition), true, nearPosition);
	BWAPI::TilePosition targetPosition = pathFind.back();
	moveTarget = BWAPI::Position(targetPosition);
	BOOST_FOREACH(UnitState u, mutalisk->getUnits())
	{
		if (u.state == Irradiated)
		{
			continue;
		}

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
	/*
	if (moveComplete.size() > 0)
	{
		BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				BWAPI::Broodwar->printf("move change to attack!!");
				state = ATTACK;
				return;
			}
		}
	}*/

	int avgX = 0;
	int avgY = 0;
	for (auto u : mutalisk->getUnits())
	{
		avgX += u.unit->getPosition().x;
		avgY += u.unit->getPosition().y;
	}
	avgX = avgX / mutalisk->getUnits().size();
	avgY = avgY / mutalisk->getUnits().size();
	//BWAPI::Position avgP(avgX, avgY);
	BWAPI::Position avgP = mutalisk->getUnits().front().unit->getPosition();

	/*
	if ((startEnemyCanAttack == true && nearbyUnits.size() > 0)
		|| (startEnemyCanAttack == false && avgP.getDistance(armyStartPosition) > 30 * 32 && BWTA::getRegion(avgP) != BWTA::getRegion(armyStartPosition) && nearbyUnits.size() > 0))
	{
		BWAPI::Broodwar->printf("move change to attack!!");
		state = ATTACK;
		return;
	}
	*/
	
	if (unitMovePath.size() / double(mutalisk->getUnits().size()) <= 0.1)
	{
		state = nextState;
		//BWAPI::Broodwar->printf("move end, change to %d", int(nextState));
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
		for (std::map<BWAPI::Unit, std::list<BWAPI::TilePosition>>::iterator it = unitMovePath.begin(); it != unitMovePath.end();)
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
					//BattleArmy::smartMove(it->first, BWAPI::Position(*it->second.begin()));
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
		for (std::map<BWAPI::Unit, std::vector<BWAPI::Position>>::iterator it = newAddArmy.begin(); it != newAddArmy.end(); it++)
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

	std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& regionDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	int antiairBuildingCount = 0;
	if (regionDetail.find(BWTA::getRegion(attackPosition)) != regionDetail.end())
	{
		std::map<BWAPI::Unit, buildingInfo> buildinginfo = regionDetail[BWTA::getRegion(attackPosition)];
		for (std::map<BWAPI::Unit, buildingInfo>::iterator it = buildinginfo.begin(); it != buildinginfo.end(); it++)
		{
			if (it->second.unitType.isBuilding() && (it->second.unitType.airWeapon() != BWAPI::WeaponTypes::None || it->second.unitType == BWAPI::UnitTypes::Terran_Bunker))
			{
				antiairBuildingCount++;
			}
		}
	}

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());
	if (ourSupply * 0.8 < antiairBuildingCount * 10)
	{
		retreatSet();
	}

	nearbyUnits.clear();

	for (auto u : tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		BWAPI::Unitset percentUnits = u.unit->getUnitsInRadius(12 * 32, BWAPI::Filter::IsEnemy);
		nearbyUnits.insert(percentUnits.begin(), percentUnits.end());

		if (u.unit->isIrradiated())
		{
			u.state = Irradiated;
		}
	}
	
	friendUnitNearBy.clear();
	
	BWAPI::Unitset tmp = (*mutalisk->getUnits().begin()).unit->getUnitsInRadius(12 * 32);
	// if enemy can attack us, add in nearby enemies count
	BOOST_FOREACH(BWAPI::Unit unit, tmp)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding()
			&& unit->getType() != BWAPI::UnitTypes::Zerg_Mutalisk && !unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
		{
			friendUnitNearBy.insert(unit);
		}
	}

	TimerManager::Instance().startTimer(TimerManager::MutaliskTac);

	for (auto u : tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		if (u.unit->isSelected())
		{
			BWAPI::Broodwar->printf("Previous Command Frame=%d type=%d state=%d", u.unit->getLastCommandFrame(), u.unit->getLastCommand().getType(), int(state));
		}
	}

	bool isUnderAttack = false;
	for (auto army : tacticArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto u : army.second->getUnits())
			{
				if (u.unit->isUnderAttack())
				{
					isUnderAttack = true;
					break;
				}
			}
		}

		if (isUnderAttack == true)
		{
			break;
		}
	}

	// do tactic FSM update
	switch (state)
	{
	case LOCATIONASSIGN:
	{
		//BWAPI::Broodwar->printf("LOCATIONASSIGN");

		attackPosition = originAttackBase;
		locationAssign(mutalisk, originAttackBase, MOVE, true);
		break;
	}
	case MOVE:
	{
		locationMove(mutalisk, ATTACK);
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
				{
					retreatSet();
				}
			}

			TimerManager::Instance().stopTimer(TimerManager::MutaAttack);
		}
		else
		{
			retreatSet();
		}
		break;
	} 
	case RETREAT:
	{
		locationMove(mutalisk, END);

		if ((nearbyUnits.size() == 0 && !isUnderAttack) || BWAPI::Broodwar->getFrameCount() >= retreatTime)
		{
			//when current is not under attack, end this tactic.
			state = END;
			break;
		}

	}
	break;

	}

	TimerManager::Instance().stopTimer(TimerManager::MutaliskTac);
}


bool MutaliskHarassTactic::hasEnemy()
{
	if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
		return true;

	//BWAPI::Unitset& units = BWAPI::Broodwar->getUnitsInRadius(attackPosition, 12 * 32);
	bool needAttack = false;
	BOOST_FOREACH(BWAPI::Unit u, nearbyUnits)
	{
		if (!u->isDetected() && u->isVisible())
			continue;

		if (u->getPlayer() == BWAPI::Broodwar->enemy() && BWTA::getRegion(u->getPosition()) == BWTA::getRegion(attackPosition) && u->getType() != BWAPI::UnitTypes::Protoss_Observer)
		{
			return true;
		}
	}

	if (needAttack == false)
	{
		std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
		std::map<BWTA::Region*, std::map<BWAPI::Unit, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
		BWAPI::TilePosition curPosition = (*tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().begin()).unit->getTilePosition();
		BWAPI::Position maxAttackPosition;
		int maxDistance = 0;

		if (occupiedDetail.find(BWTA::getRegion(originAttackBase)) != occupiedDetail.end())
		{
			std::map<BWAPI::Unit, buildingInfo >& buildingsDetail = occupiedDetail[BWTA::getRegion(originAttackBase)];
			for (std::map<BWAPI::Unit, buildingInfo >::iterator it = buildingsDetail.begin(); it != buildingsDetail.end(); it++)
			{
				//if building can attack, return
				if (imInfo[BWAPI::TilePosition(it->second.initPosition).x][BWAPI::TilePosition(it->second.initPosition).y].airForce / 20 <= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() / 4)
				{
					attackPosition = BWAPI::Position(it->second.initPosition);
					return true;
					/*
					//TODO: for refinery bugs..
					if (it->second.unitType.isRefinery() && (BWAPI::Broodwar->isVisible(it->second.initPosition)))
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

					/*
					if ((!it->second.unitType.isRefinery() && buildingsDetail.size() > 1) ||
						(buildingsDetail.size() == 1))
					{
						attackPosition = BWAPI::Position(it->second.initPosition);
						return true;
					}*/

					//maxAttackPosition = BWAPI::Position(it->second.initPosition);
					//maxDistance = int(it->second.initPosition.getDistance(BWAPI::TilePosition(curPosition)));
				}
			}
		}
	}
	return false;
}


void MutaliskHarassTactic::retreatSet()
{
	if (state == RETREAT)
		return;

	state = RETREAT;
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
	retreatTime = BWAPI::Broodwar->getFrameCount() + 20 * 25;

	if (myRegions.size() > 1)
		nextRetreatPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		nextRetreatPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	MutaliskArmy* mutalisk = dynamic_cast<MutaliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]);
	locationAssign(mutalisk, nextRetreatPosition, RETREAT, false);
}

int MutaliskHarassTactic::needRetreat()
{
	bool retreat = false;
	int mutaliskCount = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size();
	BWAPI::Unit firstUnit = tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().front().unit;

	int ourSupply = int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired());

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
	
	//std::vector<std::vector<gridInfo>>& enemyIm = InformationManager::Instance().getEnemyInfluenceMap();
	int inRegionEnemySupply = 0;
	int outRegionEnemySupply = 0;
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
	
	int enemySupply = inRegionEnemySupply + outRegionEnemySupply ;
	if (ourSupply < enemySupply
	|| (lowHealthCount >= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * 0.7 && ourSupply * 0.3 <= enemySupply))
	{
		return 0;
	}
	else
		return 1;
}


bool MutaliskHarassTactic::isTacticEnd()
{
	if (state == END || state == WIN)
	{
		return true;
	}
		
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

