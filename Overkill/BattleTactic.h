#pragma once
#include "Common.h"
#include "BattleArmy.h"
#include "UnitState.h"
#include "ZerglingArmy.h"
#include "MutaliskArmy.h"
#include "HydraliskArmy.h"
#include "OverLordArmy.h"

 
class BattleTactic
{

protected:
	enum tacticState { GROUPARMY, LOCATIONASSIGN, MOVE, MOVEATTACK, ATTACK, RETREAT, BACKLOCATIONASSIGN, BACKMOVE, WAIT, WIN, END};
	tacticState							state;

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



