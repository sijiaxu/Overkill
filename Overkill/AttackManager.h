#pragma once
#include "Common.h"
#include "BattleArmy.h"
#include "ProductionManager.h"
#include "StrategyManager.h"
#include "InformationManager.h"
#include "ZerglingArmy.h"
#include "MutaliskArmy.h"
#include "HydraliskArmy.h"
#include "OverLordArmy.h"
#include "TacticManager.h"
#include "UnitState.h"
#include "AstarPath.h"


struct AttackEnemyBase
{
	BWAPI::Position base;
	bool nowAttacking;
	int priority;
	int nextAttackTime;

	AttackEnemyBase(BWAPI::Position b, int p)
	{
		base = b;
		priority = p;
		nextAttackTime = 0;
		nowAttacking = false;
	}

	void resetAttackTime()
	{
		nextAttackTime = BWAPI::Broodwar->getFrameCount() + 25 * 90;
	}

	bool operator < (const AttackEnemyBase& u)
	{
		if (priority < u.priority)
			return true;
		else
			return false;
	}
};


class AttackManager{

	std::map<BWAPI::UnitType, BattleArmy*>	myArmy;

	std::vector<AttackEnemyBase>			attackPosition;
	void					updateAttackPosition();
	int						updatPositionTime;

	AttackManager();
	bool					triggerZerglingBuilding;

	bool					isNeedDefend;
	bool					isNeedTacticDefend;

	bool					zerglingHarassFlag;
	std::vector<BWAPI::Unit*> unRallyArmy;
	BWAPI::Position			rallyPosition;

	std::set<BWAPI::Unit *> enemyInvadeSet;
	void					groupArmy();
	bool					isArmyHealthy(BWAPI::UnitType unitType);

	enum DefendState {noEnemy ,enemyInvade, defendBattle, enemyKilled, end};
	DefendState				defendState;
	int						recoverLostTime;

	void					DefendEnemy(std::set<BWAPI::Unit *>& enemyUnitsInRegion, int enemyTotalSupply);
	void					DefendOver();
	void					DefendUpdate();
	void					DefendProductionStrategy(unsigned enemyTotalSupply);

	BWAPI::Position			generateAttackPosition();
	bool					tacticTrigCondition(int tac);

public:
	
	void					update();
	void					onUnitMorph(BWAPI::Unit* unit);
	void					onUnitDestroy(BWAPI::Unit * unit);
	void					onEnemyUnitDestroy(BWAPI::Unit* unit);

	void					addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy, BWAPI::Position attackTarget);
	bool					isNeedRetreatDefend() { return isNeedTacticDefend; }
	bool					isUnderAttack() { return isNeedDefend; }
	
	static AttackManager&	Instance();

};

