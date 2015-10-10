#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"




class HydraliskTactic : public BattleTactic
{
	BWAPI::Position			movePosition;
	int						mutaliskAttackTime;

	std::set<BWAPI::Unit*>				nearbyUnits;
	std::set<BWAPI::Unit*>				nearbySunkens;
	std::set<BWAPI::Unit*>				friendUnitNearBy;
	int									nextAttackTime;
	int									retreatTime;
	BWAPI::Position						nextRetreatPosition;

	int									retreatCount;
public:
	HydraliskTactic();

	virtual void			update();
	bool					isTacticEnd();
	bool					needRetreat();
	void					generateAttackPath();
};



