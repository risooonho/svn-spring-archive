#pragma once
#include "projectile.h"
#include "damagearray.h"

const PP_Fall = 1;
const PP_Smoke = 2;		//smoke and fire is turned off when there is to many projectiles so make sure they are unsycned
const PP_Fire = 4;
const PP_Explode = 8;
class CSmokeTrailProjectile;
struct S3DO;

class CPieceProjectile : public CProjectile
{
	int flags;
	int dispList;
	S3DO* piece;
	float3 spinVec;
	float spinSpeed;
	float spinPos;

	float3 oldSmoke,oldSmokeDir;
	CUnit* target;
	bool drawTrail;
	CSmokeTrailProjectile* curCallback;
	int* numCallback;
	int age;

	struct OldInfo{
		float3 pos;
		float size;
	};
	OldInfo* oldInfos[8];
public:
	CPieceProjectile(const float3& pos,const float3& speed, S3DO* piece, int flags,CUnit* owner,float radius);
	virtual ~CPieceProjectile(void);
	void Update();
	void Draw();
	void Collision();
	void Collision(CUnit* unit);

	void DrawUnitPart(void);
	void DrawCallback(void);
};
