#ifndef INCLUDE_H
#define INCLUDE_H
/*pragma once removed*/





// STL	

//#include <cmath>		// Math
//#include <cstdarg>	// Variable-Argument Lists
//#include <cstdio>		// I/O
//#include <cstdlib>	// Various Conversions
#include <ctime>		// Time
#include <fstream>		// File Streams
#include <algorithm>
#include <deque>
//#include <cctype>
//#include <list>
//#include <queue>
//#include <set>
//#include <string>		// String container
//#include <vector>		// Vector containers
#include <sstream>
//#include <math.h>
#include <stdarg.h>
//#include <map>
#include <functional>
#include <memory.h>
#include <stdio.h>
//#include <list>
#include <assert.h>
#include <float.h>

#ifdef WIN32
#include <direct.h>		// Folder manipulation
#include <io.h>
#include <windows.h>
#else
#include <sys/time.h>
#define LARGE_INTEGER struct timeval
static inline void itoa(int i, char* buf, int size) {
	snprintf(buf, size, "%d", i);
}
#endif


// Spring Standard Header
#include "./System/StdAfx.h"

// Spring Engine
#include "./Game/command.h"                     // Commands
#include "./Sim/Units/UnitDef.h"                // Unit Definitions
#include "./Sim/Misc/FeatureDef.h"            // Feature Definitions
#include "./Sim/MoveTypes/MoveInfo.h"           // Types of Movement units can have
#include "./Sim/Weapons/WeaponDefHandler.h"		// Weapon Definitions
//#include "float3.h"								// Float3 container and operators

// Spring AI
#include "ExternalAI/aibase.h"					// DLL exports and definitions
#include "ExternalAI/IGlobalAI.h"				// Main AI file
#include "ExternalAI/IAICallback.h"				// Callback functions
#include "ExternalAI/IGlobalAICallback.h"		// AI Interface
#include "ExternalAI/IAICheats.h"				// AI Cheat Interface

// KAI
#include "Containers.h"							// All KAI containers
#include "Definitions.h"						// Definition declarations
#include "mtrand.h"								// Mersenne Twister RNG
#include "SpotFinder.h"
#include "micropather.h"
#include "Maths.h"
#include "SunParser.h"
#include "MetalMap.h"
#include "Debug.h"
#include "PathFinder.h"
#include "UnitTable.h"
#include "ThreatMap.h"
#include "UnitHandler.h"
#include "UNIT.h"
#include "EconomyTracker.h"						// Temp only
#include "DefenseMatrix.h"
#include "BuildUp.h"
#include "AttackHandler.h"
#include "AttackGroup.h"
#include "EconomyManager.h"




            



#endif /* INCLUDE_H */
