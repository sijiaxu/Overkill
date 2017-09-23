#pragma once
#include "Common.h"
#include "BattleArmy.h"
#include "UnitState.h"
#include "ZerglingArmy.h"
#include "MutaliskArmy.h"
#include "HydraliskArmy.h"
#include "OverLordArmy.h"
#include "LurkerArmy.h"
#include "ScourgeArmy.h"
#include "UltraliskArmy.h"
#include "DevourerArmy.h"
#include "GuardianArmy.h"

 
class BattleTactic
{

protected:
	enum tacticState { GROUPARMY, LOCATIONASSIGN, MOVE, MOVEATTACK, ATTACK, RETREAT, BACKLOCATIONASSIGN, BACKMOVE, WAIT, 
		BASEGROUP, BASEATTACK, WIN, END};
	tacticState							state;
	std::vector<BWAPI::UnitType>			attackOrder;
	std::vector<BWAPI::UnitType>			movePositionOrder;

	std::map<BWAPI::UnitType, BattleArmy*>	tacticArmy;
	BWAPI::Position							attackPosition;
	BWAPI::Position							originAttackBase;

	BWAPI::Position							armyStartPosition;

	std::map<BWAPI::Unit, std::vector<BWAPI::Position>> newAddArmy;
	std::vector<BWAPI::Position> 			newAddMovePositions;
	bool									endByDefend;
	int										groupStartTime;
	int										nextGroupTime;
	BWAPI::Position							groupPosition;
	bool									groupStatus;
	bool 									startEnemyCanAttack;
	std::set<BWAPI::Unit>					currentAttackUnits;

	double2									getUnitCohesion(BWAPI::Unit u, BWAPI::Unitset nearbyUnits, double2 currentVelocity);
	double2									getUnitSeperation(BWAPI::Unit u);
	double2									getUnitAlignment(BWAPI::Unit u, BWAPI::Unitset nearbyUnits, double2 currentVelocity);
	double2									getUnitGoal(BWAPI::Unit u, BWAPI::Position goal, double2 currentVelocity);
	double2									getUnitNearEnemy(BWAPI::Unit u, std::vector<std::vector<gridInfo>>& influnceMap, double2 currentVelocity);

	BWAPI::Unit								leader;
	BWAPI::Unit								chooseLeader(BWAPI::Position destination, bool onlyFlyer);

	int										curStopTime;
	int										curMoveTime;
	bool									needMove;
	int										retreatTime;
	BWAPI::Position							nextRetreatPosition;

public:
	BattleTactic();
	~BattleTactic();

	virtual void		update() = 0;
	std::map<BWAPI::UnitType, BattleArmy*>& getArmy() { return tacticArmy; }
	virtual bool		isTacticEnd() = 0;
	virtual void		onUnitShow(BWAPI::Unit unit) {}
	virtual void		setAttackPosition(BWAPI::Position targetPosition, tacticType tactic);
	virtual bool		hasEnemy();

	virtual void		addArmyUnit(BWAPI::Unit unit);
	virtual void		onUnitDestroy(BWAPI::Unit unit);
	virtual void		retreatSet();
	void				onLurkerMorph();
	void				flockingMove(BWAPI::Position destination);
	void				togetherMove(BWAPI::Position destination);

	void				armyAttack(BWAPI::Position attackPosition, std::set<BWAPI::Unit> nearbyUnits);

	void				addAllNewArmy();
	void				newArmyRally();
	void				setDefendEnd() { endByDefend = true; }
	bool				getDefendEnd() { return endByDefend; }

	bool				reGroup(BWAPI::Position & regroupPosition) const;
	bool				unitNearChokepoint(BWAPI::Unit unit) const;
	
	BWAPI::Position		getOriginAttackPosition() { return originAttackBase; }
	
	BWAPI::Position		groupArmyCheck(bool& needGroup);
	bool				checkGroupStatus(BWAPI::Position target);
	void				setGroupPosition(BWAPI::UnitType armyType, tacticType tactic);

	void				setStartPosition(BWAPI::Position p) { armyStartPosition = p; }
	void 				checkStartPositionEnemyInfo();

	
};



