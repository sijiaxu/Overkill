#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <cassert>

#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

#include <boost/foreach.hpp>
#include <boost/progress.hpp>
#include <boost/timer.hpp>

#include <BWAPI.h>
#include <BWTA.h>

#include "Options.h"


// type definitions for storing data
typedef		unsigned char		IDType;
typedef		unsigned char		UnitCountType;
typedef		unsigned char		ChildCountType;
typedef 	int					PositionType;
typedef 	int					TimeType;
typedef		short				HealthType;
typedef		int					ScoreType;
typedef		unsigned int		HashType;
typedef     int                 UCTValue;



typedef std::vector<BWAPI::Unit> UnitVector;

BWAPI::AIModule * __NewAIModule();


enum tacticType { HydraliskPushTactic, MutaliskHarassTac, DefendTactic, ScoutTac, tactictypeEnd };// ZerglingHarassTac = 1
enum openingStrategy { TwelveHatchMuta, NinePoolling, TenHatchMuta };


enum developMode { Develop, Release };
extern developMode	curMode;

extern double armyForceMultiply;

extern std::string initReadResourceFolder;
extern std::string readResourceFolder;
extern std::string writeResourceFolder;




struct double2
{
	double x, y;

	double2() : x(0), y(0) {}
	double2(const double2& p) : x(p.x), y(p.y) {}
	double2(double x, double y) : x(x), y(y) {}
	double2(const BWAPI::Position & p) : x(p.x), y(p.y) {}

	operator BWAPI::Position()				const { return BWAPI::Position(static_cast<int>(x), static_cast<int>(y)); }

	double2 operator + (const double2 & v)	const { return double2(x + v.x, y + v.y); }
	double2 operator - (const double2 & v)	const { return double2(x - v.x, y - v.y); }
	double2 operator * (double s)			const { return double2(x*s, y*s); }
	double2 operator / (double s)			const { return double2(x / s, y / s); }

	double dot(const double2 & v)			const { return x*v.x + y*v.y; }
	double lenSq()							const { return x*x + y*y; }
	double len()							const { return sqrt(lenSq()); }
	double2 normal()						const 
	{
		if (len() == 0)
			return *this;
		else
			return *this / len(); 
	}

	void normalise() { double s(len()); x /= s; y /= s; }
	void rotate(double angle)
	{
		angle = angle*M_PI / 180.0;
		*this = double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
	}

	double2 rotateReturn(double angle)
	{
		angle = angle*M_PI / 180.0;
		return double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
	}
};



//for enemy influence map
struct gridInfo
{
	double airForce;
	double groundForce;

	//indicate the distance from the attack building, used for path finding
	double decayAirForce;
	double decayGroundForce;

	double enemyUnitAirForce;
	double enemyUnitGroundForce;

	double enemyUnitDecayAirForce;
	double enemyUnitDecayGroundForce;
	gridInfo()
	{
		airForce = 0;
		groundForce = 0;

		decayAirForce = 0;
		decayGroundForce = 0;

		enemyUnitAirForce = 0;
		enemyUnitGroundForce = 0;

		enemyUnitDecayAirForce = 0;
		enemyUnitDecayGroundForce = 0;
	}
};


bool isPositionValid(BWAPI::Position p);