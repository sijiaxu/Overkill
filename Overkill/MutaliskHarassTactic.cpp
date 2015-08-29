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

		if ((newCannonCount == 3 || newAntiAirAnmyCount == 6) && tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() > 0)
		{
			if (state == MOVE || state == RETREAT)
			{
				BWAPI::TilePosition curPosition = (*tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().begin()).unit->getTilePosition();
				generateAttackPath(curPosition, aStarTargetPosition);
				targetPosition = BWAPI::Positions::None;
			}
			newCannonCount = 0;
			newAntiAirAnmyCount = 0;
		}
	}
}



void MutaliskHarassTactic::armyQueuedMove(BattleArmy* myArmy, tacticState nextState)
{
	if (targetPosition != BWAPI::Positions::None)
	{
		BOOST_FOREACH(UnitState u, myArmy->getUnits())
		{
			if (u.unit->getDistance(targetPosition) < 32 * 4)
			{
				targetPosition = BWAPI::Positions::None;
				if (movePositions.size() == 0)
				{
					state = nextState;
				}
				break;
			}
			else
			{
				// for stuck situation
				double timeElapse = t.getNoStopElapsedTimeInMicroSec() * 0.000001;
				if (timeElapse >= 30)
				{
					BWAPI::Broodwar->printf("mutalisk group stuck !!!");
					targetPosition = BWAPI::Positions::None;
					if (movePositions.size() == 0)
					{
						state = nextState;
					}
					break;
				}
			}
		}
	}
	else
	{
		int count = 0;
		bool first = true;
		for (std::vector<BWAPI::Position>::iterator it = movePositions.begin(); it != movePositions.end(); it++)
		{
			count += 1;
			BOOST_FOREACH(UnitState u, myArmy->getUnits())
			{
				if (first)
					u.unit->move(*it, false);
				else
					u.unit->move(*it, true);
					
			}
			first = false;
			if (count == 6)
			{
				targetPosition = *it;
				movePositions.erase(movePositions.begin(), ++it);
				t.start();
				break;
			}

			std::vector<BWAPI::Position>::iterator it2 = it;
			if (++it2 == movePositions.end())
			{
				t.start();
				targetPosition = *it;
				movePositions.erase(movePositions.begin(), movePositions.end());
				break;
			}
		}
	}
}

 
void MutaliskHarassTactic::update()
{
	if (BWAPI::Broodwar->getFrameCount() % 25 == 0)
	{
		if (AttackManager::Instance().isNeedRetreatDefend())
			state = END;
	}
	
	if (BWAPI::Broodwar->getFrameCount() % (25 * 20) == 0 && aStarTargetPosition != BWAPI::TilePositions::None)
	{
		std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(InformationManager::Instance().getOurNatrualLocation(), aStarTargetPosition, true, true);
		newAddMovePositions.clear();
		BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
		{
			newAddMovePositions.push_back(BWAPI::Position(p));
		}
	}

	newArmyRally();

	MutaliskArmy* mutalisk = dynamic_cast<MutaliskArmy*>(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]);
	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();

	// do tactic FSM update
	switch (state)
	{
	case GROUPARMY:
	{
		//if enemy is nearby, do not do the regroup
		std::set<BWAPI::Unit*> units;

		BOOST_FOREACH(UnitState u, mutalisk->getUnits())
		{
			std::set<BWAPI::Unit*> tmp = u.unit->getUnitsInRadius(8 * 32);
			units.insert(tmp.begin(), tmp.end());
		}
		BOOST_FOREACH(BWAPI::Unit* u, units)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				state = LOCATIONASSIGN;
				return;
			}
		}

		//need to regroup our total army first
		bool isGroup = mutalisk->flyGroup((*mutalisk->getUnits().begin()).unit->getPosition());
		if (isGroup)
			state = LOCATIONASSIGN;
	}
	break;
	case LOCATIONASSIGN:
	{
		generateAttackPath((*mutalisk->getUnits().begin()).unit->getTilePosition(), BWAPI::TilePosition(attackPosition));
		aStarTargetPosition = BWAPI::TilePosition(attackPosition);
		state = MOVE;
		break;
	}
	case MOVE:
	{
		if (mutalisk->getUnits().size() == 0)
		{
			state = END;
			return;
		}
		armyQueuedMove(mutalisk, MOVEATTACK);
		break;
	}
	case MOVEATTACK:
	{
		if (mutalisk->getUnits().size() == 0)
		{
			state = END;
			return;
		}
		/*
		std::set<BWAPI::Unit*>& units = (*mutalisk->getUnits().begin()).unit->getUnitsInRadius(8 * 32);
		BOOST_FOREACH(BWAPI::Unit* u, units)
		{
			if (u->getPlayer() == BWAPI::Broodwar->enemy() 
				&& u->getType().airWeapon() != BWAPI::WeaponTypes::None)
			{
				state = ATTACK;
				return;
			}
		}*/

		mutalisk->armyMove(attackPosition);

		if (BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
			state = ATTACK;

		break;
	}
	case ATTACK:
	{
		if (mutalisk->getUnits().size() <= 2)
		{
			generateAttackPath((*mutalisk->getUnits().begin()).unit->getTilePosition(), BWAPI::Broodwar->self()->getStartLocation());
			aStarTargetPosition = BWAPI::Broodwar->self()->getStartLocation();
			state = RETREAT;
			return;
		}
		
		// if too many anti-air army exist, retreat
		if (needRetreat())
		{
			generateAttackPath((*mutalisk->getUnits().begin()).unit->getTilePosition(), BWAPI::Broodwar->self()->getStartLocation());
			aStarTargetPosition = BWAPI::Broodwar->self()->getStartLocation();
			state = RETREAT;
			return;
		}
			
		bool noTarget = mutalisk->harassAttack(attackPosition);
		if (noTarget)
		{
			if (hasEnemy())
			{
				state = MOVEATTACK;
			}
			else
			{
				std::set<BWAPI::Unit*>& units = (*mutalisk->getUnits().begin()).unit->getUnitsInRadius(8 * 32);
				bool hasArmy = false;
				BOOST_FOREACH(BWAPI::Unit* u, units)
				{
					if (u->getPlayer() == BWAPI::Broodwar->enemy()
						&& u->getType().airWeapon() != BWAPI::WeaponTypes::None)
					{
						hasArmy = true;
						break;
					}
				}

				if (mutalisk->getUnits().size() < 12 || hasArmy)
				{
					generateAttackPath((*mutalisk->getUnits().begin()).unit->getTilePosition(), BWAPI::Broodwar->self()->getStartLocation());
					aStarTargetPosition = BWAPI::Broodwar->self()->getStartLocation();
					state = RETREAT;
				}	
				else
					state = END;
			}
				
		}
		break;
	} 
	case RETREAT:
	{
		if (mutalisk->getUnits().size() == 0)
		{
			state = END;
			return;
		}
		armyQueuedMove(mutalisk, END);

	}
	break;
	}
}


bool MutaliskHarassTactic::hasEnemy()
{
	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
	std::map<BWTA::Region*, std::map<BWAPI::Unit*, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	int mutaliskAttackRange = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().maxRange() / 32;
	int mutaliskAttackDamage = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().damageAmount();
	double2 length = double2(1, 0) * mutaliskAttackRange;
	if (occupiedDetail.find(BWTA::getRegion(originAttackBase)) != occupiedDetail.end())
	{
		std::map<BWAPI::Unit*, buildingInfo >& buildingsDetail = occupiedDetail[BWTA::getRegion(originAttackBase)];
		for (std::map<BWAPI::Unit*, buildingInfo >::iterator it = buildingsDetail.begin(); it != buildingsDetail.end(); it++)
		{
			if (it->second.unitType == BWAPI::UnitTypes::Terran_Missile_Turret ||
				it->second.unitType == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
				it->second.unitType == BWAPI::UnitTypes::Zerg_Spore_Colony ||
				it->second.unitType == BWAPI::UnitTypes::Terran_Bunker)
			{
				if (imInfo[BWAPI::TilePosition(it->second.initPosition).x() + 1][BWAPI::TilePosition(it->second.initPosition).y() + 1].airForce / 20
					<= tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() / 4)
				{
					attackPosition = BWAPI::Position(it->second.initPosition);
					return true;
				}
			}
			else
			{
				BWAPI::Position tmp = MutaliskArmy::findSafePlace(it->first, true);
				if (tmp != BWAPI::Positions::None)
				{
					attackPosition = BWAPI::Position(it->second.initPosition);
					return true;
				}
			}
		}
	}
	return false;
}

  
//TODO: justify if we can defeat opponent's army 
bool MutaliskHarassTactic::needRetreat()
{
	std::vector<std::vector<gridInfo>>& imInfo = InformationManager::Instance().getEnemyInfluenceMap();
	int mutaliskAttackDamage = BWAPI::UnitTypes::Zerg_Mutalisk.groundWeapon().damageAmount();
	bool retreat = false;
	BOOST_FOREACH(UnitState u, tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits())
	{
		if (imInfo[u.unit->getTilePosition().x()][u.unit->getTilePosition().y()].enemyUnitAirForce > 
			tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() * mutaliskAttackDamage)
		{
			retreat = true;
			break;
		}
	}
	return retreat;
	
	/*
	BWAPI::Unit* unit = (*tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().begin()).unit;
	std::set<BWAPI::Unit*> enemySet = unit->getUnitsInRadius(8 * 32);
	int defendBuildingCount = 0;
	BOOST_FOREACH(BWAPI::Unit* unit, enemySet)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
				|| unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony
				|| unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
				|| unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
			{
				defendBuildingCount++;
			}
		}
	}
	if (int(tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size()) < defendBuildingCount * 3)
		return true;
	else
		return false;*/
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
	state = GROUPARMY;
	targetPosition = BWAPI::Positions::None;
	newCannonCount = 0;
	newAntiAirAnmyCount = 0;
	aStarTargetPosition = BWAPI::TilePositions::None;
}

void MutaliskHarassTactic::setAttackPosition(BWAPI::Position targetPosition)
{
	attackPosition = targetPosition;
	originAttackBase = targetPosition;
	std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(InformationManager::Instance().getOurNatrualLocation(), BWAPI::TilePosition(attackPosition), true, true);
	newAddMovePositions.clear();
	BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
	{
		newAddMovePositions.push_back(BWAPI::Position(p));
	}
}

void MutaliskHarassTactic::generateAttackPath(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition)
{
	std::list<BWAPI::TilePosition> pathFind = aStarPathFinding(startPosition, endPosition, true);

	movePositions.clear();
	BOOST_FOREACH(BWAPI::TilePosition p, pathFind)
	{
		movePositions.push_back(BWAPI::Position(p));
	}

	if (pathFind.size() == 0)
	{
		movePositions.push_back(BWAPI::Position(endPosition));
	}

	/*
	//if not the main base, attack directly
	if (attackPosition != InformationManager::Instance().GetEnemyBasePosition())
	{
		movePositions.clear();
		movePositions.push_back(attackPosition);
		return;
	}

	BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
	BWAPI::Position enemyNatural = InformationManager::Instance().GetEnemyNaturalPosition();
	BWAPI::Position myBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	int xDistance = std::abs(enemyBase.x() - enemyNatural.x());
	int yDistance = std::abs(enemyBase.y() - enemyNatural.y());

	BWAPI::Position enterPositionTwo;
	BWAPI::Position enterPositionOne;

	//base and natural lines horizontal
	if (xDistance > yDistance)
	{
		if (enemyBase.x() > enemyNatural.x())
		{
			enterPositionTwo = BWAPI::Position(BWAPI::Broodwar->mapWidth() * 30, enemyBase.y());
			if (myBase.y() > enemyBase.y())
				enterPositionOne = BWAPI::Position(BWAPI::Broodwar->mapWidth() * 30, BWAPI::Broodwar->mapHeight() * 30);
			else
				enterPositionOne = BWAPI::Position(BWAPI::Broodwar->mapWidth() * 30, 30 * 2);
		}
		else
		{
			enterPositionTwo = BWAPI::Position(30 * 2, enemyBase.y());
			if (myBase.y() > enemyBase.y())
				enterPositionOne = BWAPI::Position(0, BWAPI::Broodwar->mapHeight() * 30);
			else
				enterPositionOne = BWAPI::Position(0, 30 * 2);
		}
	}
	else
	{
		if (enemyBase.y() > enemyNatural.y())
		{
			enterPositionTwo = BWAPI::Position(enemyBase.x(), BWAPI::Broodwar->mapHeight() * 30);
			if (myBase.x() > enemyBase.x())
				enterPositionOne = BWAPI::Position(BWAPI::Broodwar->mapWidth() * 30, BWAPI::Broodwar->mapHeight() * 30);
			else
				enterPositionOne = BWAPI::Position(30 * 2, BWAPI::Broodwar->mapHeight() * 30);
		}
		else
		{
			enterPositionTwo = BWAPI::Position(enemyBase.x(), 30 * 2);
			if (myBase.x() > enemyBase.x())
				enterPositionOne = BWAPI::Position(BWAPI::Broodwar->mapWidth() * 30, 30 * 2);
			else
				enterPositionOne = BWAPI::Position(30 * 2, 30 * 2);
		}
	}

	movePositions.clear();
	movePositions.push_back(enterPositionOne);
	movePositions.push_back(enterPositionTwo);
	movePositions.push_back(attackPosition);

	*/
	/*
	BWTA::Region* enemyBaseRegion = BWTA::getNearestBaseLocation(attackPosition)->getRegion();

	movePositions.clear();

	double2 direc = enterPosition - enemyBase;
	double2 direcNormal = direc / direc.len();
	int enterx = enterPosition.x() + int(direcNormal.x * 32 * 30);
	int entery = enterPosition.y() + int(direcNormal.y * 32 * 30);

	int targetx = enemyBase.x() + int(direcNormal.x * 32 * 8);
	int targety = enemyBase.y() + int(direcNormal.y * 32 * 8);

	// fly to the edge first
	movePositions.push_back(BWAPI::Position(enterx < 0 ? 32 * 4 : (enterx > BWAPI::Broodwar->mapWidth() * 32 ? BWAPI::Broodwar->mapWidth() * 32 : enterx), entery < 0 ? 32 * 4 : (entery > BWAPI::Broodwar->mapHeight() * 32 ? BWAPI::Broodwar->mapHeight() * 32 : entery)));
	// fly to near the base
	movePositions.push_back(BWAPI::Position(targetx < 0 ? 32 * 4 : (targetx > BWAPI::Broodwar->mapWidth() * 32 ? BWAPI::Broodwar->mapWidth() * 32 : targetx), targety < 0 ? 32 * 4 : (targety > BWAPI::Broodwar->mapHeight() * 32 ? BWAPI::Broodwar->mapHeight() * 32 : targety)));

	//attackedEnemyRegions.insert(BWTA::getRegion(InformationManager::Instance().GetEnemyBasePosition()));
	
	*/
}