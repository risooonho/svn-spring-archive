#ifndef PIECEPROJECTILE_H
#define PIECEPROJECTILE_H

#include "Projectile.h"
#include "Sim/Misc/DamageArray.h"

const int PP_Fall = 1;
const int PP_Smoke = 2;		//smoke and fire is turned off when there is to many projectiles so make sure they are unsycned
const int PP_Fire = 4;
const int PP_Explode = 8;
class CSmokeTrailProjectile;
struct LocalS3DO;
struct S3DO;
struct SS3O;

class CPieceProjectile : public CProjectile
{
	CR_DECLARE(CPieceProjectile);

	void creg_Serialize(creg::ISerializer& s);

	int flags;
	int dispList;
	S3DO* piece3do;
	SS3O* pieces3o;
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
	
	int team;
public:
	CPieceProjectile(const float3& pos,const float3& speed, LocalS3DO* piece, int flags,CUnit* owner,float radius);
	virtual ~CPieceProjectile(void);
	void Update();
	void Draw();
	void Collision();
	void Collision(CUnit* unit);

	void DrawUnitPart(void);
	void DrawCallback(void);
	
	void DrawS3O(void);
private:
	bool HasVertices();
	float3 RandomVertexPos();
};


#endif /* PIECEPROJECTILE_H */
