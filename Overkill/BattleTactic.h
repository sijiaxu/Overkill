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

	std::map<BWAPI::Unit, std::vector<BWAPI::Position>> newAddArmy;
	std::vector<BWAPI::Position> 			newAddMovePositions;
	bool									endByDefend;

public:
	BattleTactic();
	~BattleTactic();

	virtual void		update() = 0;
	std::map<BWAPI::UnitType, BattleArmy*>& getArmy() { return tacticArmy; }
	virtual bool		isTacticEnd() = 0;
	virtual void		onUnitShow(BWAPI::Unit unit) {}
	virtual void		setAttackPosition(BWAPI::Position targetPosition) { attackPosition = originAttackBase = targetPosition; }
	virtual bool		hasEnemy();

	virtual void		addArmyUnit(BWAPI::Unit unit) { newAddArmy[unit] = newAddMovePositions; }
	void				addAllNewArmy();
	void				newArmyRally();
	void				setDefendEnd() { endByDefend = true; }
	bool				getDefendEnd() { return endByDefend; }

	bool				reGroup(BWAPI::Position & regroupPosition) const;
	bool				unitNearChokepoint(BWAPI::Unit unit) const;
	void				onUnitDestroy(BWAPI::Unit unit);
	BWAPI::Position		getOriginAttackPosition() { return originAttackBase; }
	
};



