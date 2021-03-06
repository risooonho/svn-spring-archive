// -------------------------------------------------------------------------
// AAI
//
// A skirmish AI for the TA Spring engine.
// Copyright Alexander Seizinger
// 
// Released under GPL license: see LICENSE.html for more information.
// -------------------------------------------------------------------------

#include "AAIAttack.h"
#include "AAI.h"
#include "AAISector.h"
#include "AAIGroup.h"

AAIAttack::AAIAttack(AAI *ai)
{
	this->ai = ai;

	lastAttack = 0;

	dest = 0;
}

AAIAttack::~AAIAttack(void)
{
	for(set<AAIGroup*>::iterator group = combat_groups.begin(); group != combat_groups.end(); ++group)
		(*group)->attack = 0;
	
	for(set<AAIGroup*>::iterator group = aa_groups.begin(); group != aa_groups.end(); ++group)
		(*group)->attack = 0;

	for(set<AAIGroup*>::iterator group = arty_groups.begin(); group != arty_groups.end(); ++group)
		(*group)->attack = 0;
}

bool AAIAttack::Failed()
{
	if(!combat_groups.empty())
	{	
		// check if still enough power to attack target sector
		if(ai->am->SufficientAttackPowerVS(dest, &combat_groups, 1.3))
		{
			// check if sufficient power to combat enemy units
			float3 pos = (*combat_groups.begin())->GetGroupPos();
			AAISector *sector = ai->map->GetSectorOfPos(pos);

			if(sector && ai->am->SufficientCombatPowerAt(sector, &combat_groups, 2))
				return false;
		}		
	}
	
	return true;
}

void AAIAttack::StopAttack()
{
	float3 pos;
	AAISector *sector; 
	
	for(set<AAIGroup*>::iterator group = combat_groups.begin(); group != combat_groups.end(); ++group)
	{	
		// get rally point somewhere between current pos an base
		sector = ai->map->GetSectorOfPos((*group)->GetGroupPos());

		if(sector)
		{
			int max_dist = sector->distance_to_base/2;
			int min_dist = max_dist - 1;

			if(min_dist < 1) 
				min_dist = 1;
		
			pos = ai->execute->GetRallyPointCloseTo((*group)->category, ((*group)->GetGroupPos() + ai->brain->base_center) / 2.0f, min_dist, max_dist);

			if(pos.x > 0)
				(*group)->rally_point = pos;	
		}
		
		(*group)->Retreat((*group)->rally_point);
		(*group)->attack = 0;
	}

	for(set<AAIGroup*>::iterator group = aa_groups.begin(); group != aa_groups.end(); ++group)
	{
		// get rally point somewhere between current pos an base
		sector = ai->map->GetSectorOfPos((*group)->GetGroupPos());

		if(sector)
		{
			int max_dist = sector->distance_to_base/2;
			int min_dist = max_dist - 1;

			if(min_dist < 1) 
				min_dist = 1;
		
			pos = ai->execute->GetRallyPointCloseTo((*group)->category, ((*group)->GetGroupPos() + ai->brain->base_center) / 2.0f, min_dist, max_dist);

			if(pos.x > 0)
				(*group)->rally_point = pos;	
		}

		(*group)->Retreat((*group)->rally_point);
		(*group)->attack = 0;
	}

	for(set<AAIGroup*>::iterator group = arty_groups.begin(); group != arty_groups.end(); ++group)
	{
		// todo
	}

	combat_groups.clear();
	aa_groups.clear();
	arty_groups.clear();
}

void AAIAttack::AttackSector(AAISector *sector, AttackType type)
{
	int unit;
	float importance = 110;
	
	dest = sector;
	this->type = type;	

	lastAttack = ai->cb->GetCurrentFrame();

	for(set<AAIGroup*>::iterator group = combat_groups.begin(); group != combat_groups.end(); ++group)
	{
		(*group)->AttackSector(dest, importance);	
	}

	// order aa groups to guard combat units
	if(!combat_groups.empty())
	{
		for(set<AAIGroup*>::iterator group = aa_groups.begin(); group != aa_groups.end(); ++group)
		{	
			unit = (*combat_groups.begin())->GetRandomUnit();

			if(unit >= 0)
			{
				Command c;
				c.id = CMD_GUARD;
				c.params.push_back(unit);

				(*group)->GiveOrder(&c, 110, GUARDING);
			}
		}
	}

	for(set<AAIGroup*>::iterator group = arty_groups.begin(); group != arty_groups.end(); ++group)
	{
		(*group)->AttackSector(dest, importance);
	}
}

void AAIAttack::AddGroup(AAIGroup *group)
{
	if(group->group_type == ASSAULT_UNIT)
	{
		combat_groups.insert(group);
		group->attack = this;
	}
	else if(group->group_type == ANTI_AIR_UNIT)
	{
		aa_groups.insert(group);
		group->attack = this;
	}
	else
	{
		arty_groups.insert(group);
		group->attack = this;
	} 
}

void AAIAttack::RemoveGroup(AAIGroup *group)
{
	if(group->group_type == ASSAULT_UNIT)
	{
		group->attack = 0;
		combat_groups.erase(group);
	}
	else if(group->group_type == ANTI_AIR_UNIT)	
	{
		group->attack = 0;
		aa_groups.erase(group);		
	}
	else
	{
		group->attack = 0;
		arty_groups.erase(group);
	}

	ai->am->CheckAttack(this);
}
