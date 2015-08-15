#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "AttackManager.h"


struct hydrliskDistance
{
	BWAPI::Unit* unit;
	int distance;

	hydrliskDistance(BWAPI::Unit* u, int d)
	{
		unit = u;
		distance = d;
	}

	bool operator < (const hydrliskDistance& u)
	{
		if (distance < u.distance)
			return true;
		else
			return false;
	}
};

class HydraliskTactic : public BattleTactic
{
	BWAPI::Position			movePosition;
	int						mutaliskAttackTime;

public:
	HydraliskTactic();

	virtual void			update();
	bool					isTacticEnd();

	void					generateAttackPath();
};



