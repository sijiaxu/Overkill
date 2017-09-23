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
#include "LurkerArmy.h"
#include "ScourgeArmy.h"
#include "UltraliskArmy.h"
#include "DevourerArmy.h"
#include "GuardianArmy.h"

#include "TacticManager.h"
#include "UnitState.h"
#include "AstarPath.h"


struct AttackEnemyBase
{
	BWAPI::Position base;
	bool nowAttacking;
	int priority;
	int groundNextAttackTime;
	int airNextAttackTime;

	AttackEnemyBase(BWAPI::Position b, int p)
	{
		base = b;
		priority = p;
		groundNextAttackTime = 0;
		airNextAttackTime = 0;
		nowAttacking = false;
	}

	bool operator < (const AttackEnemyBase& u) const
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
	BWAPI::Position							lastAttackPosition;
	int						updatPositionTime;
	int						defendAddSupply;

	AttackManager();
	bool					triggerZerglingBuilding;

	bool					isNeedDefend;
	bool					isNeedTacticDefend;
	bool					hasWorkerScouter;

	bool					zerglingHarassFlag;
	std::vector<BWAPI::Unit> unRallyArmy;
	BWAPI::Position			rallyPosition;

	std::set<BWAPI::Unit> enemyInvadeSet;

	BWAPI::Position			naturalChokePosition;
	BWAPI::Position			baseChokePosition;

	void					groupArmy();
	bool					isArmyHealthy(BWAPI::UnitType unitType);

	enum DefendState {noEnemy ,enemyInvade, defendBattle, enemyKilled, end};
	DefendState				defendState;
	int						recoverLostTime;

	void					DefendEnemy(std::set<BWAPI::Unit>& enemyUnitsInRegion, int enemyTotalSupply);
	void					DefendOver();
	void					DefendUpdate();
	void					DefendProductionStrategy(int myTotalArmySupply, int enemyTotalSupply);

	void					popAttackPosition(BWAPI::Position popPosition);
	
	bool					tacticTrigCondition(int tac, BWAPI::Position attackPosition);
	void					triggerTactic(tacticType tacType, BWAPI::Position attackPosition);

	int						addTacArmy(int needArmySupply, tacticType tacType, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, bool allAirEnemy, bool addAll = false);
	bool					isFirstMutaAttack;
	bool					isFirstHydraAttack;

	void					ScoutUpdate();
	int						nextScoutTime;

	bool					isHaveAntiAirUnit();

public:
	
	void					update();
	void					issueAttackCommand(tacticType type, BWAPI::Position attackPosition = BWAPI::Positions::None);
	BWAPI::Position			getNextAttackPosition(bool isGround);

	void					onUnitMorph(BWAPI::Unit unit);
	void					onUnitDestroy(BWAPI::Unit unit);
	void					onLurkerMorph();
	void					onEnemyUnitDestroy(BWAPI::Unit unit);

	void					addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy, tacticType tacType, BWAPI::Position attackTarget, bool endByDefend);
	bool					isNeedRetreatDefend() { return isNeedTacticDefend; }
	bool					isUnderAttack() { return isNeedDefend; }

	std::map<BWAPI::UnitType, int> reaminArmy();
	
	static AttackManager&	Instance();

};

