#include "StdAfx.h"
#include "TorpedoLauncher.h"
#include "Sound.h"
#include "Sim/Projectiles/TorpedoProjectile.h"
#include "Game/GameHelper.h"
#include "Sim/Units/Unit.h"
#include "Map/Ground.h"
#include "Sim/Units/UnitDef.h"
#include "WeaponDefHandler.h"
#include "mmgr.h"

CTorpedoLauncher::CTorpedoLauncher(CUnit* owner)
: CWeapon(owner),
	tracking(0)
{
	owner->hasUWWeapons=true;
}

CTorpedoLauncher::~CTorpedoLauncher(void)
{
}


void CTorpedoLauncher::Update(void)
{
	if(targetType!=Target_None){
		weaponPos=owner->pos+owner->frontdir*relWeaponPos.z+owner->updir*relWeaponPos.y+owner->rightdir*relWeaponPos.x;
//		if(!onlyForward){		
			wantedDir=targetPos-weaponPos;
			float dist=wantedDir.Length();
			predict=dist/projectileSpeed;
			wantedDir/=dist;
//		}
	}
	CWeapon::Update();
}

void CTorpedoLauncher::Fire(void)
{
	float3 dir;
//	if(onlyForward){
//		dir=owner->frontdir;
//	} else {
		dir=targetPos-weaponPos;
		dir.Normalize();
		if(weaponDef->trajectoryHeight>0){
			dir.y+=weaponDef->trajectoryHeight;
			dir.Normalize();
		}
//	}
	float3 startSpeed=dir*weaponDef->startvelocity;

//	if(onlyForward)
//		startSpeed+=owner->speed*0.5f;
	SAFE_NEW CTorpedoProjectile(weaponPos,startSpeed,owner,areaOfEffect,projectileSpeed,tracking,(int)(range/projectileSpeed+15),targetUnit, weaponDef);
	if(fireSoundId && (!weaponDef->soundTrigger || salvoLeft==salvoSize-1))
		sound->PlaySample(fireSoundId,owner,fireSoundVolume);
}

bool CTorpedoLauncher::TryTarget(const float3& pos,bool userTarget,CUnit* unit)
{
	if(!CWeapon::TryTarget(pos,userTarget,unit))
		return false;

	if(unit){
		if(unit->unitDef->canhover)
			return false;
		if(unit->unitDef->canfly && unit->pos.y>0)
			return false;
	}
	if(ground->GetHeight2(pos.x,pos.z)>0)
		return 0;

	float3 dir=pos-weaponPos;
	float length=dir.Length();
	if(length==0)
		return true;

	dir/=length;

	if(avoidFriendly && helper->TestCone(weaponPos,dir,length,(accuracy+sprayangle)+0.05f,owner->allyteam,owner))	//+0.05f since torpedoes has an unfortunate tendency to hit own ships due to movement
		return false;
	return true;
}
