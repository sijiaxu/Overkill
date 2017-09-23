#include "Overkill.h"


bool analyzed;
bool analysis_just_finished;


void Overkill::readHistoryFile()
{
	historyInfo.clear();
	fstream historyFile;
	string enemyName = BWAPI::Broodwar->enemy()->getName();

	string filePath;
	filePath = readResourceFolder;
	filePath += enemyName;
	
	//for each enemy, create a file
	historyFile.open(filePath.c_str(), ios::in);
	
	//file do not exist, first match to the enemy
	if (!historyFile.is_open())
	{
		BWAPI::Broodwar->printf("first match to:   %s", enemyName.c_str());
		historyFile.close();

		//default opening strategy
		chooseOpeningStrategy = TenHatchMuta; //TwelveHatchMuta;//TenHatchMuta;//NinePoolling;
		StrategyManager::Instance().setOpeningStrategy(chooseOpeningStrategy);

		StrategyManager::Instance().setOpponentWinrate("-1");

		/*
		if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
		{
			chooseOpeningStrategy = TenHatchMuta;
			StrategyManager::Instance().setOpeningStrategy(TenHatchMuta);
		}
		else
		{
			chooseOpeningStrategy = TwelveHatchMuta;
			StrategyManager::Instance().setOpeningStrategy(TwelveHatchMuta);
			//StrategyManager::Instance().setOpeningStrategy(NinePoolling);
		}*/
	}
	else
	{
		BWAPI::Broodwar->printf("read match records with :   %s", enemyName.c_str());
		string playerResult;
		// file format:
		// enemy_name | choose strategy | result
		while (getline(historyFile, playerResult))
		{
			std::stringstream ss(playerResult);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, '|'))
			{
				if (item != "")
					itemList.push_back(item);
			}
			historyInfo.push_back(itemList);
		}

		//do UCB1 calculate, and set the opening strategy
		std::map<std::string, std::pair<int, int>> strategyResults;
		int winCount = 0;
		for (auto record : historyInfo)
		{
			//win
			if (record[2] == "1")
			{
				strategyResults[record[1]].first += 1;
				winCount += 1;
			}
			else
			{
				strategyResults[record[1]].second += 1;
			}
		}
		std::string rate = std::to_string(int(10 * float(winCount) / historyInfo.size()));
		StrategyManager::Instance().setOpponentWinrate(rate);

		double experimentCount = historyInfo.size();
		std::map<std::string, double> strategyUCB;
		for(auto opening : StrategyManager::Instance().getStrategyNameArray())
		{
			strategyUCB[opening] = 99999;
		}

		//for (std::map<std::string, std::pair<int, int>>::iterator it = strategyResults.begin(); it != strategyResults.end(); it++)
		for (auto it : strategyResults)
		{
			double strategyExpectation = double(it.second.first) / (it.second.first + it.second.second);
			double uncertainty = 0.7 * std::sqrt(std::log(experimentCount) / (it.second.first + it.second.second));
			strategyUCB[it.first] = strategyExpectation + uncertainty;
		}
		
		std::string maxUCBStrategy;
		double maxUCB = 0;
		//for (std::map<std::string, double>::iterator it = strategyUCB.begin(); it != strategyUCB.end(); it++)
		for (auto it : strategyUCB)
		{
			if (it.second > maxUCB)
			{
				maxUCBStrategy = it.first;
				maxUCB = it.second;
			}
			BWAPI::Broodwar->printf("%s , UCB: %.4f", it.first.c_str(), it.second);
		}

		BWAPI::Broodwar->printf("choose %s opening", maxUCBStrategy.c_str());
		int openingId = StrategyManager::Instance().getStrategyByName(maxUCBStrategy);
		if (openingId != -1)
		{
			chooseOpeningStrategy = openingStrategy(openingId);
		}
		 
		//for test
		//chooseOpeningStrategy = TenHatchMuta;

		StrategyManager::Instance().setOpeningStrategy(chooseOpeningStrategy);
		historyFile.close();
	}
}


void Overkill::writeCurrentPlay(bool isWin)
{
	fstream historyFile;
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::string filePath = writeResourceFolder;
	filePath += enemyName;

	//for each enemy, create a file
	historyFile.open(filePath.c_str(), ios::out);

	std::vector<string> currentPlay;
	currentPlay.push_back(enemyName);
	currentPlay.push_back(StrategyManager::Instance().getStrategyName(chooseOpeningStrategy));
	if (isWin)
		currentPlay.push_back("1");
	else
		currentPlay.push_back("0");
	historyInfo.push_back(currentPlay);

	bool enemyEarlyRushFlag = InformationManager::Instance().getEnemyEarlyRushSuccess();
	//defeated by early Rush
	if (enemyEarlyRushFlag && !isWin)
	{
		//12 hatch is slower than 10 hatch, so it may definitely loss in this match
		//add this simulated record to improved ucb learning speed.
		if (chooseOpeningStrategy == TenHatchMuta || chooseOpeningStrategy == TwelveHatchMuta)
		{
			std::vector<string> simulatedPlay;
			simulatedPlay.push_back(enemyName);
			openingStrategy simulateOpening = chooseOpeningStrategy == TenHatchMuta ? TwelveHatchMuta : TenHatchMuta;
			simulatedPlay.push_back(StrategyManager::Instance().getStrategyName(simulateOpening));
			simulatedPlay.push_back("0");
			historyInfo.push_back(simulatedPlay);
		}
	}

	for (auto record : historyInfo)
	{
		for (auto field : record)
		{
			historyFile << field << "|";
		}
		historyFile << endl;
	}
	
	historyFile.close();
}


void Overkill::onStart()
{
	Broodwar->setLocalSpeed(0);
	BWAPI::Broodwar->sendText("gl hf :)");

	//Broodwar->printf("The map is %s, a %d player map", Broodwar->mapName().c_str(), Broodwar->getStartLocations().size());
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//read map information into BWTA so terrain analysis can be done in another thread
	BWTA::readMap();

	BWTA::analyze();

	for (auto & u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			InformationManager::Instance().addOccupiedRegionsDetail(BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Broodwar->self(), u);
	}

	analyzed = true;
	analysis_just_finished = true;

	show_bullets = false;
	show_visibility_data = false;
	show_paths = true;
	readHistoryFile();
}


void Overkill::onEnd(bool isWinner)
{
	int frameElapse = BWAPI::Broodwar->getFrameCount();
	int currentSupply = BWAPI::Broodwar->self()->supplyUsed();

	bool win = false;

	if (frameElapse >= 80000 && BWAPI::Broodwar->self()->supplyUsed() >= 180 * 2)
	{
		win = true;
	}
	else
	{
		win = isWinner;
	}

	writeCurrentPlay(win);
	int reward = win == true ? 10 : -10;

	//TimerManager::Instance().startTimer(TimerManager::strategy);
	
	/*
	StrategyManager::Instance().strategyChange(reward);
	StrategyManager::Instance().featureWeightSave();
	StrategyManager::Instance().featureGradientSave();
	StrategyManager::Instance().experienceDataSave();
	*/
	StrategyManager::Instance().trainModels(reward);


	//TimerManager::Instance().stopTimer(TimerManager::strategy);

	int maxFrameCount = TimerManager::Instance().getMaxFrameCount();
	int gamePlayLength = frameElapse / (24 * 60);
	std::string enemyRace;
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		enemyRace = "z";
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		enemyRace = "T";
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		enemyRace = "p";
	}
	else
		enemyRace = "unknow";

	std::string mapName = BWAPI::Broodwar->mapFileName();
	std::string maxTimeItem = TimerManager::Instance().getMaxItem();
	double maxValue = TimerManager::Instance().getMaxFrameValue();
	//save match result
	
	
	fstream historyFile;
	string filePath = writeResourceFolder + "match_result_" + BWAPI::Broodwar->enemy()->getName();
	historyFile.open(filePath.c_str(), ios::app);
	historyFile << win << ":" << maxFrameCount << ":" << gamePlayLength << ":" << enemyRace << ":" << mapName << ":" << maxTimeItem << ":" << maxValue << endl;
	historyFile.close();
	

	//save match count data
	int matchCount = StrategyManager::Instance().getPlayeMatchCount();
	filePath = writeResourceFolder + "match_count_" + BWAPI::Broodwar->enemy()->getName();
	historyFile.open(filePath.c_str(), ios::out);
	historyFile << (matchCount + 1) << endl;
	historyFile.close();

}


void Overkill::onFrame()
{
	if (Broodwar->isReplay())
		return;


	if (BWAPI::Broodwar->getFrameCount() >= 90000)
	{
		BWAPI::Broodwar->leaveGame();
	}

	/*
	if (WorkerManager::Instance().getNumMineralWorkers() == 0 && BWAPI::Broodwar->self()->minerals() < 100 
		&& BWAPI::Broodwar->self()->supplyUsed() < 100 * 2 && BWAPI::Broodwar->getFrameCount() >= (24 * 60 * 15))
	{
		BWAPI::Broodwar->leaveGame();
	}
	*/


	drawStats();


	/*
	BOOST_FOREACH(BWAPI::Unit enemy, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (!enemy->isGatheringMinerals())
		{
			BWAPI::Broodwar->drawCircleMap(enemy->getPosition().x, enemy->getPosition().y, 8, BWAPI::Colors::Blue, true);
		}
		else
			BWAPI::Broodwar->drawCircleMap(enemy->getPosition().x, enemy->getPosition().y, 8, BWAPI::Colors::Red, true);
	}*/


	TimerManager::Instance().startTimer(TimerManager::All);

	TimerManager::Instance().startTimer(TimerManager::Worker);
	WorkerManager::Instance().update();
	TimerManager::Instance().stopTimer(TimerManager::Worker);

	//self start location only available if the map has base locations
	if (analyzed)
	{
		drawTerrainData();

		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().startTimer(TimerManager::Production);
		ProductionManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Production);

		TimerManager::Instance().startTimer(TimerManager::Building);
		BuildingManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Building);

		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		//TimerManager::Instance().startTimer(TimerManager::Scout);
		//ScoutManager::Instance().update();
		//TimerManager::Instance().stopTimer(TimerManager::Scout);

		TimerManager::Instance().startTimer(TimerManager::strategy);
		StrategyManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::strategy);
	}

	TimerManager::Instance().stopTimer(TimerManager::All);

	TimerManager::Instance().displayTimers(490, 180);
	
}

void Overkill::onSendText(std::string text)
{
	if (text == "/show bullets")
	{
		show_bullets = !show_bullets;
	}
	else if (text == "/show players")
	{
		showPlayers();
	}
	else if (text == "/show forces")
	{
		showForces();
	}
	else if (text == "/show visibility")
	{
		show_visibility_data = !show_visibility_data;
	}
	else if (text == "/analyze")
	{
		if (analyzed == false)
		{
			Broodwar->printf("Analyzing map... this may take a minute");
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
		}
	}
	else
	{
		Broodwar->printf("You typed '%s'!", text.c_str());
		Broodwar->sendText("%s", text.c_str());
	}
}


void Overkill::drawPaths()
{
	//for (set<Unit*>::const_iterator it = BWAPI::Broodwar->self()->getUnits().begin(); it != Broodwar->self()->getUnits().end(); it++) 
	for (auto unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() != UnitTypes::Protoss_Interceptor)
		{
			BWAPI::Position point1 = unit->getPosition();
			BWAPI::Position point2 = unit->getTargetPosition();

			Color color = Colors::Cyan;
			if (unit->getOrder() == Orders::AttackUnit)
			{
				color = Colors::Red;
			}

			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, color);

			/*
			if ((*it)->getType().isWorker() && (*it)->getOrder() == Orders::PlaceBuilding)
			{
				int x = (*it)->getTargetPosition().x - ((*it)->getBuildType().tileWidth()*TILE_SIZE) / 2;
				int y = (*it)->getTargetPosition().y - ((*it)->getBuildType().tileHeight()*TILE_SIZE) / 2;

				Broodwar->drawBoxMap(x, y, x + (*it)->getBuildType().tileWidth()*TILE_SIZE, y + (*it)->getBuildType().tileHeight()*TILE_SIZE, Colors::Green);
			}*/
		}
	}
}


void Overkill::onNukeDetect(BWAPI::Position target)
{
	if (target != Positions::Unknown)
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)", target.x, target.y);
	else
		Broodwar->printf("Nuclear Launch Detected");
}

void Overkill::onUnitDiscover(BWAPI::Unit unit)
{

}

void Overkill::onUnitEvade(BWAPI::Unit unit)
{

}

void Overkill::onUnitShow(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	TacticManager::Instance().onUnitShow(unit);
	InformationManager::Instance().onUnitShow(unit);
	//for initial 
	//ScoutManager::Instance().onUnitShow(unit);
	WorkerManager::Instance().onUnitShow(unit);
	//for first overlord
	if (unit->getPlayer() == BWAPI::Broodwar->self() && 
		(!unit->getType().isBuilding() && !unit->getType().isWorker()))
	{
		AttackManager::Instance().onUnitMorph(unit);
	}
}

void Overkill::onUnitHide(BWAPI::Unit unit)
{
	
}

void Overkill::onUnitCreate(BWAPI::Unit unit)
{

}

void Overkill::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;
		
	InformationManager::Instance().onUnitDestroy(unit);
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		ProductionManager::Instance().onUnitDestroy(unit);
		WorkerManager::Instance().onUnitDestroy(unit);
		//ScoutManager::Instance().onUnitDestroy(unit);
		if (!unit->getType().isBuilding() && !unit->getType().isWorker())
		{
			AttackManager::Instance().onUnitDestroy(unit);
			TacticManager::Instance().onUnitDestroy(unit);
		}

		//trigger morph
		if (!unit->isUnderAttack() && unit->getType().isWorker())
		{
			return;
		}
		else
		{
			StrategyManager::Instance().setReward(unit->getType(), false);
		}
	}
	else if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		AttackManager::Instance().onEnemyUnitDestroy(unit);
		StrategyManager::Instance().setReward(unit->getType(), true);
	}
}

void Overkill::onUnitMorph(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg && unit->getBuildType() == BWAPI::UnitTypes::Zerg_Lurker)
	{
		AttackManager::Instance().onLurkerMorph();
		TacticManager::Instance().onLurkerMorph();
	}

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	InformationManager::Instance().onUnitMorph(unit);
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		WorkerManager::Instance().onUnitMorph(unit);
		//ScoutManager::Instance().onUnitMorph(unit);
		if (!unit->getType().isBuilding() && !unit->getType().isWorker())
		{
			AttackManager::Instance().onUnitMorph(unit);
		}

		
	}
}


void Overkill::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		//change the during production features
		if (unit->getType().isBuilding())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				for (auto b : BWTA::getBaseLocations())
				{
					if (unit->getTilePosition().getDistance(b->getTilePosition()) < 10)
					{
						StrategyManager::Instance().buildingFinish(unit->getType());
						break;
					}
				}
			}
			else
			{
				StrategyManager::Instance().buildingFinish(unit->getType());
			}
		}
	}
}


DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	BOOST_FOREACH(BWAPI::Unit u, BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			InformationManager::Instance().addOccupiedRegionsDetail(BWTA::getRegion(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Broodwar->self(), u);
	}

	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

void Overkill::drawStats()
{
	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5, 0, "I have %d units:", myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	//for (std::set<Unit*>::iterator i = myUnits.begin(); i != myUnits.end(); i++)
	for (auto unit : myUnits)
	{
		if (!unit->getType().isBuilding() && unit->getType().canAttack())
		{
			int unitWidth = unit->getType().tileWidth() * 32;
			int unitHeight = unit->getType().tileHeight() * 32 / 8;
			BWAPI::Position unitPosition = unit->getPosition();
			BWAPI::Broodwar->drawBoxMap(unitPosition.x - unitWidth / 2, unitPosition.y + unitHeight / 2, unitPosition.x + unitWidth / 2, unitPosition.y - unitHeight / 2, BWAPI::Colors::Red, true);
			double healthPercent = double(unit->getHitPoints()) / unit->getType().maxHitPoints();
			BWAPI::Broodwar->drawBoxMap(unitPosition.x - unitWidth / 2, unitPosition.y + unitHeight / 2, unitPosition.x - unitWidth / 2 + int(unitWidth * healthPercent), unitPosition.y - unitHeight / 2, BWAPI::Colors::Green, true);
			/*
			if (unit->isSelected())
			{
				BWAPI::Broodwar->printf("Previous Command Frame=%d Pos=(%d, %d) type=%d", unit->getLastCommandFrame(), unit->getLastCommand().getTargetPosition().x / 32, unit->getLastCommand().getTargetPosition().y / 32, unit->getLastCommand().getType());
				BWAPI::Broodwar->drawLineMap(unit->getPosition().x, unit->getPosition().y, unit->getLastCommand().getTargetPosition().x, unit->getLastCommand().getTargetPosition().y, BWAPI::Colors::Green);
				BWAPI::Broodwar->drawCircleMap(unit->getLastCommand().getTargetPosition().x, unit->getLastCommand().getTargetPosition().y, 8, BWAPI::Colors::Green, true);
			}*/

		}

		if (unitTypeCounts.find(unit->getType()) == unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair(unit->getType(), 0));
		}
		unitTypeCounts.find(unit->getType())->second++;
	}
	int line = 1;
	for (std::map<UnitType, int>::iterator i = unitTypeCounts.begin(); i != unitTypeCounts.end(); i++)
	{
		Broodwar->drawTextScreen(5, 16 * line, "- %d %ss", (*i).second, (*i).first.getName().c_str());
		line++;
	}
	// draw the map tile position at map
	Broodwar->drawTextScreen(BWAPI::Broodwar->getMousePosition().x + 20, BWAPI::Broodwar->getMousePosition().y + 20, "%d %d", (BWAPI::Broodwar->getScreenPosition().x + BWAPI::Broodwar->getMousePosition().x) / TILE_SIZE, (BWAPI::Broodwar->getScreenPosition().y + BWAPI::Broodwar->getMousePosition().y) / TILE_SIZE);
}

void Overkill::drawBullets()
{
	BWAPI::Bulletset bullets = Broodwar->getBullets();
	//for (std::set<Bullet*>::iterator i = bullets.begin(); i != bullets.end(); i++)
	for (auto bullet : bullets)
	{
		Position p = bullet->getPosition();
		double velocityX = bullet->getVelocityX();
		double velocityY = bullet->getVelocityY();
		if (bullet->getPlayer() == Broodwar->self())
		{
			Broodwar->drawLineMap(p.x, p.y, p.x + (int)velocityX, p.y + (int)velocityY, Colors::Green);
			Broodwar->drawTextMap(p.x, p.y, "\x07%s", bullet->getType().getName().c_str());
		}
		else
		{
			Broodwar->drawLineMap(p.x, p.y, p.x + (int)velocityX, p.y + (int)velocityY, Colors::Red);
			Broodwar->drawTextMap(p.x, p.y, "\x06%s", bullet->getType().getName().c_str());
		}
	}
}


void Overkill::drawVisibilityData()
{
	for (int x = 0; x<Broodwar->mapWidth(); x++)
	{
		for (int y = 0; y<Broodwar->mapHeight(); y++)
		{
			if (Broodwar->isExplored(x, y))
			{
				if (Broodwar->isVisible(x, y))
					Broodwar->drawDotMap(x * 32 + 16, y * 32 + 16, Colors::Green);
				else
					Broodwar->drawDotMap(x * 32 + 16, y * 32 + 16, Colors::Blue);
			}
			else
				Broodwar->drawDotMap(x * 32 + 16, y * 32 + 16, Colors::Red);
		}
	}
}


void Overkill::drawTerrainData()
{
	Broodwar->drawTextScreen(5, 0, "\x04Starts: %d Bases: %d Enemy: %s Frames: %d (%d:%s%d) Map: %s", BWTA::getStartLocations().size(), BWTA::getBaseLocations().size(), Broodwar->enemy()->getRace().getName().c_str(), Broodwar->getFrameCount(), Broodwar->getFrameCount() / (24 * 60), (Broodwar->elapsedTime() % 60 < 10) ? "0" : "", Broodwar->elapsedTime() % 60, Broodwar->mapName().c_str());

	//we will iterate through all the base locations, and draw their outlines.
	//for (std::set<BWTA::BaseLocation*>::const_iterator i = BWTA::getBaseLocations().begin(); i != BWTA::getBaseLocations().end(); i++)
	for (auto i : BWTA::getBaseLocations())
	{
		TilePosition p = i->getTilePosition();
		Position c = i->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map, p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, Colors::Blue, false);

		//draw a circle at each mineral patch
		for (auto j : i->getStaticMinerals())
		{
			Position q = j->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map, q.x, q.y, 30, Colors::Cyan, false);
		}

		//draw the outlines of vespene geysers
		//for (std::set<BWAPI::Unit>::const_iterator j = (*i)->getGeysers().begin(); j != (*i)->getGeysers().end(); j++)
		for (auto j : i->getGeysers())
		{
			TilePosition q = j->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map, q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, Colors::Orange, false);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if (i->isIsland())
			Broodwar->drawCircle(CoordinateType::Map, c.x, c.y, 80, Colors::Yellow, false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	//for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	for (auto r : BWTA::getRegions())
	{
		BWTA::Polygon p = r->getPolygon();
		for (int j = 0; j<(int)p.size(); j++)
		{
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	//for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	for (auto r : BWTA::getRegions())
	{
		//for (std::set<BWTA::Chokepoint*>::const_iterator c = (*r)->getChokepoints().begin(); c != (*r)->getChokepoints().end(); c++)
		for (auto c : r->getChokepoints())
		{
			Position point1 = c->getSides().first;
			Position point2 = c->getSides().second;
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Red);
		}
	}
}


void Overkill::showPlayers()
{
	BWAPI::Playerset players = Broodwar->getPlayers();
	//for (std::set<Player*>::iterator i = players.begin(); i != players.end(); i++)
	for (auto i : players)
	{
		Broodwar->printf("Player [%d]: %s is in force: %s", i->getID(), i->getName().c_str(), i->getForce()->getName().c_str());
	}
}

void Overkill::showForces()
{
	BWAPI::Forceset forces = Broodwar->getForces();
	for (auto i : forces)
	{
		BWAPI::Playerset players = i->getPlayers();
		Broodwar->printf("Force %s has the following players:", i->getName().c_str());
		for (auto j : players)
		{
			Broodwar->printf("  - Player [%d]: %s", j->getID(), j->getName().c_str());
		}
	}
}

