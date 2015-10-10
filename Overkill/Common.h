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


typedef std::vector<BWAPI::Unit*> UnitVector;

BWAPI::AIModule * __NewAIModule();

enum tacticType { ZerglingHarassTac = 0, MutaliskHarassTac = 1, HydraliskPushTactic = 2, DefendTactic = 3, tactictypeEnd};

struct double2
{
	double x, y;

	double2() {}
	double2(const double2& p) : x(p.x), y(p.y) {}
	double2(double x, double y) : x(x), y(y) {}
	double2(const BWAPI::Position & p) : x(p.x()), y(p.y()) {}

	operator BWAPI::Position()				const { return BWAPI::Position(static_cast<int>(x), static_cast<int>(y)); }

	double2 operator + (const double2 & v)	const { return double2(x + v.x, y + v.y); }
	double2 operator - (const double2 & v)	const { return double2(x - v.x, y - v.y); }
	double2 operator * (double s)			const { return double2(x*s, y*s); }
	double2 operator / (double s)			const { return double2(x / s, y / s); }

	double dot(const double2 & v)			const { return x*v.x + y*v.y; }
	double lenSq()							const { return x*x + y*y; }
	double len()							const { return sqrt(lenSq()); }
	double2 normal()						const { return *this / len(); }

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


