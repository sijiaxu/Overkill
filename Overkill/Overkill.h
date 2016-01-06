#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>

#include <string>
#include <sstream>
#include <ctime>
#include <list>
#include <vector>
#include <map>


#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>

#include "WorkerManager.h"
#include "ProductionManager.h"
#include "BuildingManager.h"
#include "InformationManager.h"
#include "AttackManager.h"
#include "StrategyManager.h"
#include "TacticManager.h"
#include "ScoutManager.h"

#include "Common.h"
#include "TimeManager.cpp"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();


using namespace BWAPI;
using namespace std;



class Overkill : public BWAPI::AIModule
{
	void			readHistoryFile();
	void			writeCurrentPlay(bool isWin);
	
	openingStrategy						chooseOpeningStrategy;
	std::vector<std::vector<string>>	historyInfo;
	

public:


	void	onStart();
	void	onFrame();
	void	onEnd(bool isWinner);
	void	onUnitDestroy(BWAPI::Unit unit);
	void	onUnitMorph(BWAPI::Unit unit);
	void	onSendText(std::string text);
	void	onUnitCreate(BWAPI::Unit unit);
	void	onUnitShow(BWAPI::Unit unit);
	void	onUnitHide(BWAPI::Unit unit);
	void    onNukeDetect(BWAPI::Position target);
	void    onUnitComplete(BWAPI::Unit unit);
	void    onUnitDiscover(BWAPI::Unit unit);
	void	onUnitEvade(BWAPI::Unit unit);

	void    drawTerrainData();
	void	drawStats(); //not part of BWAPI::AIModule
	void	drawBullets();
	void	drawVisibilityData();
	void	drawPaths();

	void	showPlayers();
	void	showForces();

	bool show_bullets;
	bool show_visibility_data;
	bool show_paths;
};



