// -------------------------------------------------------------------------
// AAI
//
// A skirmish AI for the TA Spring engine.
// Copyright Alexander Seizinger
// 
// Released under GPL license: see LICENSE.html for more information.
// -------------------------------------------------------------------------

#pragma once

#include "aidef.h"

class AAI;
class AAIBrain;
class AAIBuildTable;
class AAIMap;
class AAIAttack;
class AAISector;


class AAIAttackManager
{
public:
	AAIAttackManager(AAI *ai, IAICallback *cb, AAIBuildTable *bt);
	~AAIAttackManager(void);

	void Update();

	void GetNextDest(AAIAttack *attack);

	void LaunchAttack(); 

	void StopAttack(AAIAttack *attack);

	void CheckAttack(AAIAttack *attack);

	// sends all groups to a rallypoint
	void RallyGroups(AAIAttack *attack);

	// true if combat groups have suffiecient attack power to face stationary defences
	bool SufficientAttackPowerVS(AAISector *dest, set<AAIGroup*> *combat_groups, float aggressiveness);

	// true if units have sufficient combat power to face mobile units in dest 
	bool SufficientCombatPowerAt(AAISector *dest, set<AAIGroup*> *combat_groups, float aggressiveness);

	// true if defences have sufficient combat power to push back mobile units in dest 
	bool SufficientDefencePowerAt(AAISector *dest, float aggressiveness);

	list<AAIAttack*> attacks;

	// array stores number of combat groups per category (for SufficientAttackPowerVS(..) )
	int *available_combat_cat;


private:

	IAICallback *cb;
	AAI *ai;
	AAIBrain *brain;
	AAIBuildTable *bt;
	AAIMap *map;
};
