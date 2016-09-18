#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "AstarPath.h"
#include "ZerglingArmy.h"





class HydraliskTactic : public BattleTactic
{
	BWAPI::Position			movePosition;
	int						mutaliskAttackTime;
	int						ourArmySupply;
	int						ourArmyCount;

	std::set<BWAPI::Unit>				nearbyUnits;
	std::set<BWAPI::Unit>				nearbySunkens;
	std::set<BWAPI::Unit>				friendUnitNearBy;
	int									nextAttackTime;
	int									retreatTime;
	BWAPI::Position						nextRetreatPosition;

	int									retreatCount;
public:
	HydraliskTactic();

	virtual void			update();
	bool					isTacticEnd();
	bool					needRetreat(BWAPI::Unit firstUnit);
	void					generateAttackPath();
};



