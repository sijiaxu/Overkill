#pragma once

#include "Common.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BattleArmy.h"


struct Scout{
	Scout(BWAPI::Unit* u)
	{
		overLord = u;
		TileTarget = BWAPI::Broodwar->self()->getStartLocation();
		nextMovePosition = BWAPI::TilePositions::None;
	}
	BWAPI::Unit* overLord;
	BWAPI::TilePosition TileTarget;
	BWAPI::TilePosition nextMovePosition;
	BWAPI::TilePosition naturalTileTarget;
};

class ScoutManager {

	enum ScoutState {scoutForEnemyBase = 0, scoutForEnemyInfo = 1};
	ScoutState state;
	std::vector<Scout> overLordIdle;
	std::vector<Scout> overLordScouts;
	ScoutManager();

	std::vector<BWAPI::TilePosition>   scoutLocation;

	void						smartMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition) const;
	void						assignScoutWork();
	void						generateScoutLocation();
	void						idleRally();

	std::vector<double2>		moveDirections;
	void						overlordMove(Scout& overlord);

public:
	void						update();

	void						onUnitDestroy(BWAPI::Unit * unit);
	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitShow(BWAPI::Unit* unit);
	static ScoutManager&		Instance();

	void						addScoutLocation(BWAPI::TilePosition location);
	void						addScoutUnit(BWAPI::Unit* unit);

	int							getIdleOverlordNum() { return int(overLordIdle.size()); }
	std::vector<BWAPI::Unit*>	getOverLordArmy(int count);
	void						giveBackOverLordArmy(BattleArmy* army);


	std::vector<BWAPI::TilePosition>& getPossibleEnemyBase();

};