#include "StdAfx.h"
#include "EmgProjectile.h"
#include "Game/Camera.h"
#include "Rendering/GL/VertexArray.h"
#include "Sync/SyncTracer.h"
#include "mmgr.h"
#include "ProjectileHandler.h"
#include "Sim/Weapons/WeaponDefHandler.h"

CR_BIND_DERIVED(CEmgProjectile, CWeaponProjectile, (float3(0,0,0),float3(0,0,0),NULL,float3(0,0,0),0,0,NULL));

CR_REG_METADATA(CEmgProjectile,(        
    CR_MEMBER(ttl),
    CR_MEMBER(intensity),
    CR_MEMBER(color)
    ));

CEmgProjectile::CEmgProjectile(const float3& pos,const float3& speed,CUnit* owner,const float3& color,float intensity, int ttl, WeaponDef *weaponDef)
: CWeaponProjectile(pos,speed,owner,0,ZeroVector, weaponDef,0),
	ttl(ttl),
	color(color),
	intensity(intensity)
{
	SetRadius(weaponDef->collisionSize);
	drawRadius=weaponDef->size;
#ifdef TRACE_SYNC
	tracefile << "New emg: ";
	tracefile << pos.x << " " << pos.y << " " << pos.z << " " << speed.x << " " << speed.y << " " << speed.z << "\n";
#endif
}

CEmgProjectile::~CEmgProjectile(void)
{
}

void CEmgProjectile::Update(void)
{
	pos+=speed;
	ttl--;

	if(ttl<0){
		intensity-=0.1f;
		if(intensity<=0){
			deleteMe=true;
			intensity=0;
		}
	}
}

void CEmgProjectile::Collision(CUnit* unit)
{
//	unit->DoDamage(damages,owner);

	CWeaponProjectile::Collision(unit);
}

void CEmgProjectile::Draw(void)
{
	inArray=true;
	unsigned char col[4];
	col[0]=(unsigned char) (color.x*intensity*255);
	col[1]=(unsigned char) (color.y*intensity*255);
	col[2]=(unsigned char) (color.z*intensity*255);
	col[3]=5;//intensity*255;
	float3 interPos=pos+speed*gu->timeOffset;
	va->AddVertexTC(interPos-camera->right*drawRadius-camera->up*drawRadius,weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->ystart,col);
	va->AddVertexTC(interPos+camera->right*drawRadius-camera->up*drawRadius,weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->ystart,col);
	va->AddVertexTC(interPos+camera->right*drawRadius+camera->up*drawRadius,weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->yend,col);
	va->AddVertexTC(interPos-camera->right*drawRadius+camera->up*drawRadius,weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->yend,col);
}

int CEmgProjectile::ShieldRepulse(CPlasmaRepulser* shield,float3 shieldPos, float shieldForce, float shieldMaxSpeed)
{
	float3 dir=pos-shieldPos;
	dir.Normalize();
	if(dir.dot(speed)<shieldMaxSpeed){
		speed+=dir*shieldForce;
		return 2;
	}
	return 0;
}
