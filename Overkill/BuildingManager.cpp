#include "BuildingManager.h"


BuildingManager::BuildingManager()
	: debugMode(false)
	, reservedMinerals(0)
	, reservedGas(0)
	, buildingSpace(1)
	, totalBuildTasks(0)
{

}


// gets called every frame from GameCommander
void BuildingManager::update()
{
	int i = 0;
	
	for (std::vector<Building>::iterator it = buildingData.begin(); it != buildingData.end();)
	{
		switch (it->buildingState)
		{
		case Building::initBuilderAndLocation:
		{
			assignWorkersToUnassignedBuildings(*it);
		}
			break;
		case Building::exploreMove:
		{
			exploreUnseenPosition(*it);
		}
			break;
		case Building::issueBuildOrder:
		{
			constructAssignedBuildings(*it);
		}
			break;
		case Building::refinerySpecial:
		{
			//for refinery bug...
			if (!it->builderUnit->exists())
			{
				it->buildingState = Building::end;
				return;
			}

			if (!it->builderUnit->isConstructing())
			{
				WorkerManager::Instance().finishedWithWorker(it->builderUnit);
				it->builderUnit = NULL;
				it->buildingState = Building::initBuilderAndLocation;
			}
			// issue the build order!
			//it->builderUnit->build(it->finalPosition, it->type);
		}
		break;
		case Building::buildingOrderCheck:
		{
			buildingOrderCheck(*it);
		}
			break;
		default:
			break;
		}

		if (it->buildingState == Building::end)
		{
			/*
			if (it->type == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				for (auto b : BWTA::getBaseLocations())
				{
					if (it->desiredPosition == b->getTilePosition())
					{
						StrategyManager::Instance().buildingFinish(it->type);
						break;
					}
				}
			}
			else
			{
				StrategyManager::Instance().buildingFinish(it->type);
			}
			*/

			BWAPI::Broodwar->printf("building manger end: %s\n", it->type.c_str());


			//execute the callback function and data
			ProductionManager::Instance().buildingCallback(it->builderUnit, it->waitingBuildType);
			reservedMinerals -= it->type.mineralPrice();
			reservedGas -= it->type.gasPrice();
			if (it->type == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				InformationManager::Instance().removeFakeOccupiedBase(it->finalPosition);
			}
			it = buildingData.erase(it);
		}
		else
			it++;
	}
}


void BuildingManager::builderNotExist(Building& b)
{
	if(!b.builderUnit->exists())
	{
		b.builderUnit = NULL;
		b.buildingState = Building::initBuilderAndLocation;
		return;

		//b.buildingState = Building::end;
		//StrategyManager::Instance().buildingFinish(b.type);
	}
}


// STEP 2: ASSIGN WORKERS TO BUILDINGS WITHOUT THEM
void BuildingManager::assignWorkersToUnassignedBuildings(Building& b)
{
	b.finalPosition = b.desiredPosition;
	BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b);
	if (workerToAssign)
	{
		// set the worker we have assigned
		b.builderUnit = workerToAssign;
		// get the building location
		BWAPI::TilePosition testLocation = getBuildingLocation(b);
		// if we can't find a valid build location, do not build at this frame
		if (!testLocation.isValid())
		{
			WorkerManager::Instance().finishedWithWorker(b.builderUnit);
			return;
		}
		// set the final position of the building as this location
		b.finalPosition = testLocation;

		if (!isBuildingPositionExplored(b))
		{
			b.buildingState = Building::exploreMove;
		}
		else
		{
			if (b.type.isRefinery())
			{
				b.buildingState = Building::refinerySpecial;
				b.builderUnit->build(b.type, b.finalPosition);
			}
			else
				b.buildingState = Building::issueBuildOrder;
		}

		/*
		// grab a worker unit from WorkerManager which is closest to this final position
		BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b);

		// if the worker exists
		if (workerToAssign)
		{
			// set the worker we have assigned
			b.builderUnit = workerToAssign;
			if (!isBuildingPositionExplored(b))
			{
				b.buildingState = Building::exploreMove;
			}
			else
			{
				if (b.type.isRefinery())
					b.buildingState = Building::refinerySpecial;
				else
					b.buildingState = Building::issueBuildOrder;
			}

		}*/
	}
	
}

void BuildingManager::exploreUnseenPosition(Building& b)
{
	if (!b.builderUnit->exists())
	{
		if (b.type == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			b.buildingState = Building::end;
			StrategyManager::Instance().buildingFinish(b.type);
		}
		else
		{
			b.builderUnit = NULL;
			b.buildingState = Building::initBuilderAndLocation;
		}
		return;
	}

	if (!isBuildingPositionExplored(b))
	{
		BattleArmy::smartMove(b.builderUnit, BWAPI::Position(b.finalPosition));
		//b.builderUnit->move(BWAPI::Position(b.finalPosition));
	}
	else
	{
		/*
		// get the building location
		BWAPI::TilePosition testLocation = getBuildingLocation(b);

		if (!testLocation.isValid())
		{
			return;
		}

		b.finalPosition = testLocation;
		*/
		b.buildingState = Building::issueBuildOrder;
	}
}

// STEP 3: ISSUE CONSTRUCTION ORDERS TO ASSIGN BUILDINGS AS NEEDED
void BuildingManager::constructAssignedBuildings(Building& b)
{
	if (!b.builderUnit->exists())
	{
		if (b.type == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			b.buildingState = Building::end;
			StrategyManager::Instance().buildingFinish(b.type);
		}
		else
		{
			b.builderUnit = NULL;
			b.buildingState = Building::initBuilderAndLocation;
		}
		return;
	}

	// issue the build order!
	b.builderUnit->build(b.type, b.finalPosition);
	b.buildingState = Building::buildingOrderCheck;
}


void BuildingManager::buildingOrderCheck(Building& b)
{
	if (!b.builderUnit->exists())
	{
		if (b.type == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			b.buildingState = Building::end;
			StrategyManager::Instance().buildingFinish(b.type);
		}
		else
		{
			b.builderUnit = NULL;
			b.buildingState = Building::initBuilderAndLocation;
		}
		return;
	}

	if (b.builderUnit->isMorphing())
	{
		b.buildingState = Building::end;
	}
	//if build command fail, something is wrong
	else if (!b.builderUnit->isConstructing())
	{
		WorkerManager::Instance().finishedWithWorker(b.builderUnit);
		b.builderUnit = NULL;
		b.buildingState = Building::initBuilderAndLocation;
	}
}


BWAPI::TilePosition BuildingManager::getBuildingLocation( Building & b)
{
	BWAPI::TilePosition testLocation = BWAPI::TilePositions::None;

	int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);

	if (b.type.isRefinery())
	{
		return getRefineryPosition();
	}

	if (b.type == BWAPI::UnitTypes::Zerg_Creep_Colony) // b.type == BWAPI::UnitTypes::Zerg_Hatchery || 
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony) <= 3)
		{
			testLocation = getBuildLocationNear(b, 0, false);
		}
		else
		{
			testLocation = getBuildLocationNear(b, 1, false);
		}
		

	}
	else
		testLocation = getBuildLocationNear(b, 0, false);

	// send back the location
	return testLocation;
}


BWAPI::TilePosition BuildingManager::getRefineryPosition()
{
	int closest = 999999;
	BWAPI::TilePosition position = BWAPI::TilePositions::None;
	std::set<BWTA::Region *> & myRegions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());

	//BWAPI::Broodwar->getGeysers() return all accessable gaysers that can not build extractor
	BOOST_FOREACH(BWAPI::Unit geyser, BWAPI::Broodwar->getGeysers())
	{	
		//not my region
		if (myRegions.find(BWTA::getRegion(geyser->getPosition())) == myRegions.end())
		{
			continue;
		}

		if (geyser->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) < closest)
		{
			closest = geyser->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			position = geyser->getTilePosition();
		}
	}
	return position;
}


BWAPI::TilePosition BuildingManager::getBuildLocationNear(Building & b, int buildDist, bool inRegionPriority, bool horizontalOnly) const
{
	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x = b.desiredPosition.x;
	int y = b.desiredPosition.y;
	int length = 1;
	int j = 0;
	bool first = true;
	int dx = 0;
	int dy = 1;

	int iter = 0;

	while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
		{
			iter++;

			if (iter > 1000)
			{
				std::set<BWTA::Region *> & ourRegion = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->self());
				for (auto r : ourRegion)
				{
					if (BWTA::getRegion(b.desiredPosition) != r)
					{
						b.desiredPosition = BWAPI::TilePosition(r->getCenter());
						break;
					}
				}
				return BWAPI::TilePositions::None;
			}

			// can we build this building at this location
			bool canBuild = this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), b, buildDist, horizontalOnly);

			if (canBuild)
			{
				return BWAPI::TilePosition(x, y);
			}
		}

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;

		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first = !first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return  BWAPI::TilePositions::None;
}


//returns true if we can build this type of unit here with the specified amount of space.
//space value is stored in this->buildDistance.
bool BuildingManager::canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const
{
	
	//if we can't build here, we of course can't build here with space
	if (!BWAPI::Broodwar->canBuildHere(position, b.type, b.builderUnit))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	// define the rectangle of the building spot
	int startx = position.x - buildDist;
	int starty = position.y - buildDist;
	int endx = position.x + width + buildDist;
	int endy = position.y + height + buildDist;


	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() - 1 || endy > BWAPI::Broodwar->mapHeight() - 1)
	{
		return false;
	}

	// if we can't build here, or space is reserved, or it's in the resource box, we can't build here
	for (int x = startx; x < endx; x++)
	{
		for (int y = starty; y < endy; y++)
		{
			if (!b.type.isRefinery())
			{
				if (!buildable(x, y, b))
				{
					return false;
				}
			}
		}
	}

	return true;
}



bool BuildingManager::buildable(int x, int y, const Building & b) const
{
	//returns true if this tile is currently buildable, takes into account units on tile
	if (!BWAPI::Broodwar->isBuildable(x, y)) // &&|| b.type == BWAPI::UnitTypes::Zerg_Hatchery
	{
		return false;
	}

	if (!(BWAPI::Broodwar->hasCreep(x, y) || b.type == BWAPI::UnitTypes::Zerg_Hatchery))
	{
		return false;
	}

	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->getUnitsOnTile(x, y))
	{
		if (unit->getType().isBuilding())
		{
			return false;
		}
	}

	return true;
}



// COMPLETED
bool BuildingManager::isEvolvedBuilding(BWAPI::UnitType type) {

	if (type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
		type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
		type == BWAPI::UnitTypes::Zerg_Lair ||
		type == BWAPI::UnitTypes::Zerg_Hive ||
		type == BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		return true;
	}

	return false;
}

// add a new building to be constructed
void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, std::vector<MetaType> waitingBuildType) 
{

	if (debugMode) { BWAPI::Broodwar->printf("Issuing addBuildingTask: %s", type.getName().c_str()); }

	totalBuildTasks++;

	// reserve the resources for this building
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();

	// set it up to receive a worker
	//buildingData.addBuilding(ConstructionData::Unassigned, Building(type, desiredLocation));
	buildingData.push_back(Building(type, desiredLocation, waitingBuildType));
}


bool BuildingManager::isBuildingPositionExplored(const Building & b) const
{
	BWAPI::TilePosition tile = b.finalPosition;

	// for each tile where the building will be built
	for (int x = 0; x < b.type.tileWidth(); ++x)
	{
		for (int y = 0; y < b.type.tileHeight(); ++y)
		{
			if (!BWAPI::Broodwar->isExplored(tile.x + x, tile.y + y))
			{
				return false;
			}
		}
	}

	return true;
}


char BuildingManager::getBuildingWorkerCode(const Building & b) const
{
	if (b.builderUnit == NULL)	return 'X';
	else						return 'W';
}

int BuildingManager::getReservedMinerals() {
	return reservedMinerals;
}

int BuildingManager::getReservedGas() {
	return reservedGas;
}

/*
void BuildingManager::drawBuildingInformation(int x, int y) {

	BOOST_FOREACH(BWAPI::Unit unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "\x07%d", unit->getID());
			//BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "%s", unit->getOrder().getName().c_str());
	}

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Building Information:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + 20, "\x04 Name");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x + 150, y + 20, "\x04 State");

	int yspace = 0;

	buildingData.begin(ConstructionData::Unassigned);
	while (buildingData.hasNextBuilding(ConstructionData::Unassigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Unassigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s", b.type.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 Need %c", getBuildingWorkerCode(b));
	}

	buildingData.begin(ConstructionData::Assigned);
	while (buildingData.hasNextBuilding(ConstructionData::Assigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Assigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s %d", b.type.getName().c_str(), b.builderUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 A %c (%d,%d)", getBuildingWorkerCode(b), b.finalPosition.x, b.finalPosition.y);

		int x1 = b.finalPosition.x * 32;
		int y1 = b.finalPosition.y * 32;
		int x2 = (b.finalPosition.x + b.type.tileWidth()) * 32;
		int y2 = (b.finalPosition.y + b.type.tileHeight()) * 32;

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition().x, b.builderUnit->getPosition().y, (x1 + x2) / 2, (y1 + y2) / 2, BWAPI::Colors::Orange);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Red, false);
	}

	buildingData.begin(ConstructionData::UnderConstruction);
	while (buildingData.hasNextBuilding(ConstructionData::UnderConstruction)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::UnderConstruction);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s %d", b.type.getName().c_str(), b.buildingUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 Const %c", getBuildingWorkerCode(b));
	}
}
*/
BuildingManager & BuildingManager::Instance()
{
	static BuildingManager instance;
	return instance;
}