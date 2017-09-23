#include "Common.h"


BWAPI::AIModule * __NewAIModule();

developMode curMode = Develop;

double armyForceMultiply = 1.5;

std::string initReadResourceFolder = "./bwapi-data/AI/";
std::string readResourceFolder = "./bwapi-data/read/";
std::string writeResourceFolder = "./bwapi-data/write/";

bool isPositionValid(BWAPI::Position p)
{
	if (p.x > BWAPI::Broodwar->mapWidth() * 32 - 1 || p.x < 0
		|| p.y > BWAPI::Broodwar->mapHeight() * 32 - 1 || p.y < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}



