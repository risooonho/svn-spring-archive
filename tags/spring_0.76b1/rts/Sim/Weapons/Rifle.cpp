// Rifle.cpp: implementation of the CRifle class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Game/GameHelper.h"
#include "LogOutput.h"
#include "Map/Ground.h"
#include "myMath.h"
#include "Rifle.h"
#include "Sim/Projectiles/Unsynced/HeatCloudProjectile.h"
#include "Sim/Projectiles/Unsynced/SmokeProjectile.h"
#include "Sim/Projectiles/Unsynced/TracerProjectile.h"
#include "Sim/Units/Unit.h"
#include "Sim/Weapons/WeaponDefHandler.h"
#include "Sound.h"
#include "Sync/SyncTracer.h"
#include "mmgr.h"

CR_BIND_DERIVED(CRifle, CWeapon, (NULL));

CR_REG_METADATA(CRifle,(
	CR_RESERVED(8)
	));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRifle::CRifle(CUnit* owner)
: CWeapon(owner)
{
}

CRifle::~CRifle()
{

}

void CRifle::Update()
{
	if(targetType!=Target_None){
		weaponPos=owner->pos+owner->frontdir*relWeaponPos.z+owner->updir*relWeaponPos.y+owner->rightdir*relWeaponPos.x;
		weaponMuzzlePos=owner->pos+owner->frontdir*relWeaponMuzzlePos.z+owner->updir*relWeaponMuzzlePos.y+owner->rightdir*relWeaponMuzzlePos.x;
		wantedDir=targetPos-weaponPos;
		wantedDir.Normalize();
	}
	CWeapon::Update();
}

bool CRifle::TryTarget(const float3 &pos,bool userTarget,CUnit* unit)
{
	if(!CWeapon::TryTarget(pos,userTarget,unit))
		return false;

	if(unit){
		if(unit->isUnderWater)
			return false;
	} else {
		if(pos.y<0)
			return false;
	}

	float3 dir=pos-weaponMuzzlePos;
	float length=dir.Length();
	if(length==0)
		return true;

	dir/=length;

	float g=ground->LineGroundCol(weaponMuzzlePos,pos);
	if(g>0 && g<length*0.9f)
		return false;

	if(helper->TestCone(weaponMuzzlePos,dir,length,(accuracy+sprayangle)*(1-owner->limExperience*0.9f),owner->allyteam,owner)){
		return false;
	}
	return true;
}

void CRifle::Fire(void)
{
	float3 dir=targetPos-weaponMuzzlePos;
	dir.Normalize();
	dir+=(gs->randVector()*sprayangle+salvoError)*(1-owner->limExperience*0.9f);
	dir.Normalize();
#ifdef TRACE_SYNC
	tracefile << "Rifle fire: ";
	tracefile << owner->pos.x << " " << dir.x << " " << targetPos.x << " " << targetPos.y << " " << targetPos.z << "\n";
#endif
	CUnit* hit;
	float length=helper->TraceRay(weaponMuzzlePos, dir, range, weaponDef->damages[0], owner, hit, collisionFlags);
	if(hit) {
		hit->DoDamage(weaponDef->damages, owner, ZeroVector, weaponDef->id);
		SAFE_NEW CHeatCloudProjectile(weaponMuzzlePos + dir * length, hit->speed*0.9f, 30, 1, owner);
	}
	SAFE_NEW CTracerProjectile(weaponMuzzlePos,dir*projectileSpeed,length,owner);
	SAFE_NEW CSmokeProjectile(weaponMuzzlePos,float3(0,0.0f,0),70,0.1f,0.02f,owner,0.6f);
	if(fireSoundId)
		sound->PlaySample(fireSoundId,owner,fireSoundVolume);
}
