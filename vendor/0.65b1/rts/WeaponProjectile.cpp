#include "stdafx.h"
#include ".\weaponprojectile.h"
#include "weapondefhandler.h"
#include "sound.h"
#include "3doparser.h"
#include "mygl.h"
#include "TextureHandler.h"
#include "ground.h"
#include "gamehelper.h"
#include "laserprojectile.h"
#include "fireballprojectile.h"
#include "matrix44f.h"
#include "feature.h"
#include "unit.h"
//#include "mmgr.h"

CWeaponProjectile::CWeaponProjectile(const float3& pos,const float3& speed,CUnit* owner, CUnit* target,const float3 &targetPos, WeaponDef *weaponDef,CWeaponProjectile* interceptTarget) : 
	CProjectile(pos,speed,owner),
	weaponDef(weaponDef),
	target(target),
	targetPos(targetPos),
	startpos(pos),
	targeted(false),
	interceptTarget(interceptTarget)
{
	if(interceptTarget){
		interceptTarget->targeted=true;
		AddDeathDependence(interceptTarget);
	}
	if(!weaponDef->visuals.modelName.empty()){
		S3DOModel* model = unit3doparser->Load3DO(string("objects3d\\")+weaponDef->visuals.modelName+".3do",1,0);
		if(model){
			modelDispList= model->rootobject->displist;
			isUnitPart=true;
		}
	}
}

CWeaponProjectile::~CWeaponProjectile(void)
{
}

CWeaponProjectile *CWeaponProjectile::CreateWeaponProjectile(const float3& pos,const float3& speed, CUnit* owner, CUnit *target, const float3 &targetPos, WeaponDef *weaponDef)
{
	switch(weaponDef->visuals.renderType)
	{
	case WEAPON_RENDERTYPE_LASER:
		return new CLaserProjectile(pos, speed, owner, weaponDef->damages, 30, weaponDef->visuals.color, weaponDef->intensity, weaponDef, weaponDef->range/weaponDef->projectilespeed);
		break;
	case WEAPON_RENDERTYPE_PLASMA:
		break;
	case WEAPON_RENDERTYPE_FIREBALL:
		return new CFireBallProjectile(pos,speed,owner,target,targetPos,weaponDef);
		break;
	}

	//gniarf!
	return NULL;
}

void CWeaponProjectile::Collision()
{
	if(!weaponDef->noExplode || gs->frameNum&1)
		helper->Explosion(pos,weaponDef->damages,weaponDef->areaOfEffect,owner,true,weaponDef->noExplode? 0.3:1,weaponDef->noExplode);

	if(weaponDef->soundhit.id)
		sound->PlaySound(weaponDef->soundhit.id,this,weaponDef->soundhit.volume);

	if(!weaponDef->noExplode)
		CProjectile::Collision();
	else
	{
		if(TraveledRange())
			CProjectile::Collision();
	}

}

void CWeaponProjectile::Collision(CFeature* feature)
{
	if(gs->randFloat()>weaponDef->fireStarter)
		feature->StartFire();

	Collision();
}

void CWeaponProjectile::Collision(CUnit* unit)
{
	if(!weaponDef->noExplode || gs->frameNum&1)
		helper->Explosion(pos,weaponDef->damages,weaponDef->areaOfEffect,owner,true,weaponDef->noExplode? 0.3:1,weaponDef->noExplode);

	if(weaponDef->soundhit.id)
		sound->PlaySound(weaponDef->soundhit.id,this,weaponDef->soundhit.volume);

	if(!weaponDef->noExplode)
		CProjectile::Collision(unit);
	else
	{
		if(TraveledRange())
			CProjectile::Collision();
	}
}

void CWeaponProjectile::Update()
{
	//pos+=speed;

	//if(weaponDef->gravityAffected)
	//	speed.y+=gs->gravity;


	//if(weaponDef->noExplode)
	//{
 //       if(TraveledRange())
	//		CProjectile::Collision();
	//}

	//if(speed.Length()<weaponDef->maxvelocity)
	//	speed += dir*weaponDef->weaponacceleration

	CProjectile::Update();
}

bool CWeaponProjectile::TraveledRange()
{
	float trange = (pos-startpos).Length();
	if(trange>weaponDef->range)
		return true;
	return false;
}

void CWeaponProjectile::DrawUnitPart()
{
	float3 interPos=pos+speed*gu->timeOffset;
	float3 dir(speed);
	dir.Normalize();
	glPushMatrix();
	float3 rightdir;
	if(dir.y!=1)
		rightdir=dir.cross(UpVector);
	else
		rightdir=float3(1,0,0);
	rightdir.Normalize();
	float3 updir(rightdir.cross(dir));

	CMatrix44f transMatrix;
	transMatrix[0]=-rightdir.x;
	transMatrix[1]=-rightdir.y;
	transMatrix[2]=-rightdir.z;
	transMatrix[4]=updir.x;
	transMatrix[5]=updir.y;
	transMatrix[6]=updir.z;
	transMatrix[8]=dir.x;
	transMatrix[9]=dir.y;
	transMatrix[10]=dir.z;
	transMatrix[12]=interPos.x;
	transMatrix[13]=interPos.y;
	transMatrix[14]=interPos.z;
	glMultMatrixf(&transMatrix[0]);		

	glCallList(modelDispList);
	glPopMatrix();
}

void CWeaponProjectile::DependentDied(CObject* o)
{
	if(o==interceptTarget)
		interceptTarget=0;

	if(o==target)
		target=0;

	CProjectile::DependentDied(o);
}
