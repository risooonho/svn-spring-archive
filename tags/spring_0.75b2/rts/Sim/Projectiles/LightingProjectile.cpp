#include "StdAfx.h"
#include "LightingProjectile.h"
#include "Game/Camera.h"
#include "Rendering/GL/VertexArray.h"
#include "Sim/Weapons/Weapon.h"
#include "ProjectileHandler.h"
#include "Sim/Weapons/WeaponDefHandler.h"
#include "mmgr.h"

CR_BIND_DERIVED(CLightingProjectile, CWeaponProjectile, (float3(0,0,0),float3(0,0,0),NULL,float3(0,0,0),NULL,0,NULL));

CR_REG_METADATA(CLightingProjectile,(
	CR_MEMBER(ttl),
	CR_MEMBER(color),
	CR_MEMBER(endPos),
	CR_MEMBER(weapon),
	CR_MEMBER(displacements),
	CR_MEMBER(displacements2),
	CR_RESERVED(16)
	));

CLightingProjectile::CLightingProjectile(const float3& pos,const float3& end,CUnit* owner,const float3& color, WeaponDef *weaponDef,int ttl,CWeapon* weap)
:	CWeaponProjectile(pos,ZeroVector, owner, 0, ZeroVector, weaponDef,0, true), //CProjectile(pos,ZeroVector,owner),
	ttl(ttl),
	color(color),
	endPos(end),
	weapon(weap)
{
	checkCol=false;
	drawRadius=pos.distance(endPos);

	displacements[0]=0;
	for(int a=1;a<10;++a)
		displacements[a]=(gs->randFloat()-0.5f)*drawRadius*0.05f;

	displacements2[0]=0;
	for(int a=1;a<10;++a)
		displacements2[a]=(gs->randFloat()-0.5f)*drawRadius*0.05f;

	if(weapon)
		AddDeathDependence(weapon);

#ifdef TRACE_SYNC
	tracefile << "New lighting: ";
	tracefile << pos.x << " " << pos.y << " " << pos.z << " " << end.x << " " << end.y << " " << end.z << "\n";
#endif
}

CLightingProjectile::~CLightingProjectile(void)
{
}

void CLightingProjectile::Update(void)
{
	ttl--;

	if(ttl<0){
		deleteMe=true;
	}

	if(weapon){
		pos=weapon->weaponPos;
	}
	for(int a=1;a<10;++a)
		displacements[a]+=(gs->randFloat()-0.5f)*0.3f;
	for(int a=1;a<10;++a)
		displacements2[a]+=(gs->randFloat()-0.5f)*0.3f;
}

void CLightingProjectile::Draw(void)
{
	inArray=true;
	unsigned char col[4];
	col[0]=(unsigned char) (color.x*255);
	col[1]=(unsigned char) (color.y*255);
	col[2]=(unsigned char) (color.z*255);
	col[3]=1;//intensity*255;

	float3 dir=(endPos-pos).Normalize();
	float3 dif(pos-camera->pos);
	float camDist=dif.Length();
	dif/=camDist;
	float3 dir1(dif.cross(dir));
	dir1.Normalize();
	float3 tempPos=pos;

	for(int a=0;a<9;++a){
		float f=(a+1)*0.111f;
		va->AddVertexTC(tempPos+dir1*(displacements[a]+weaponDef->thickness),weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->ystart,    col);
		va->AddVertexTC(tempPos+dir1*(displacements[a]-weaponDef->thickness),weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->yend,col);
		tempPos=pos*(1-f)+endPos*f;
		va->AddVertexTC(tempPos+dir1*(displacements[a+1]-weaponDef->thickness),weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->yend,col);
		va->AddVertexTC(tempPos+dir1*(displacements[a+1]+weaponDef->thickness),weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->ystart    ,col);
	}

	tempPos=pos;
	for(int a=0;a<9;++a){
		float f=(a+1)*0.111f;
		va->AddVertexTC(tempPos+dir1*(displacements2[a]+weaponDef->thickness),weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->ystart,    col);
		va->AddVertexTC(tempPos+dir1*(displacements2[a]-weaponDef->thickness),weaponDef->visuals.texture1->xstart,weaponDef->visuals.texture1->yend,col);
		tempPos=pos*(1-f)+endPos*f;
		va->AddVertexTC(tempPos+dir1*(displacements2[a+1]-weaponDef->thickness),weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->yend,col);
		va->AddVertexTC(tempPos+dir1*(displacements2[a+1]+weaponDef->thickness),weaponDef->visuals.texture1->xend,weaponDef->visuals.texture1->ystart    ,col);
	}
}

void CLightingProjectile::DependentDied(CObject* o)
{
	if(o==weapon)
		weapon=0;
	CProjectile::DependentDied(o);
}
