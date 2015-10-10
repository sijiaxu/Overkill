#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"
#include "AstarPath.h"
#include "TimeManager.cpp"


class sixZerglingTactic: public BattleTactic
{
	tacticState state;

	std::vector<BWAPI::Position> 		movePositions;

	std::set<BWAPI::Unit*>				nearbyUnits;
	std::set<BWAPI::Unit*>				nearbySunkens;
	std::set<BWAPI::Unit*>				friendUnitNearBy;
	int									nextAttackTime;
	int									retreatTime;
	BWAPI::Position						nextRetreatPosition;

	int									retreatCount;

public:
	sixZerglingTactic();
	
	virtual void			update();
	bool					isTacticEnd();
	bool					needRetreat();

};



