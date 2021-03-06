#include "StdAfx.h"
#include "BeamLaser.h"
#include "Game/GameHelper.h"
#include "Game/Team.h"
#include "LogOutput.h"
#include "Map/Ground.h"
#include "Matrix44f.h"
#include "Rendering/UnitModels/3DOParser.h"
#include "Sim/Misc/InterceptHandler.h"
#include "Sim/MoveTypes/AirMoveType.h"
#include "Sim/Projectiles/WeaponProjectiles/BeamLaserProjectile.h"
#include "Sim/Projectiles/WeaponProjectiles/LargeBeamLaserProjectile.h"
#include "Sim/Projectiles/WeaponProjectiles/LaserProjectile.h"
#include "Sim/Units/COB/CobFile.h"
#include "Sim/Units/COB/CobInstance.h"
#include "Sim/Units/Unit.h"
#include "Sim/Weapons/PlasmaRepulser.h"
#include "Sound.h"
#include "WeaponDefHandler.h"
#include "mmgr.h"

CR_BIND_DERIVED(CBeamLaser, CWeapon, (NULL));

CR_REG_METADATA(CBeamLaser,(
	CR_MEMBER(color),
	CR_MEMBER(oldDir),
	CR_MEMBER(damageMul),
	CR_RESERVED(16)
	));

CBeamLaser::CBeamLaser(CUnit* owner)
: CWeapon(owner),
	oldDir(ZeroVector),
	lastFireFrame(0)
{
}

CBeamLaser::~CBeamLaser(void)
{
}

void CBeamLaser::Update(void)
{
	if(targetType!=Target_None){
		weaponPos=owner->pos+owner->frontdir*relWeaponPos.z+owner->updir*relWeaponPos.y+owner->rightdir*relWeaponPos.x;
		weaponMuzzlePos=owner->pos+owner->frontdir*relWeaponMuzzlePos.z+owner->updir*relWeaponMuzzlePos.y+owner->rightdir*relWeaponMuzzlePos.x;
		if(!onlyForward){
			wantedDir=targetPos-weaponPos;
			wantedDir.Normalize();
		}

		if (!weaponDef->beamburst) {
			predict = salvoSize / 2;
		} else {
 			// beamburst tracks the target during the burst so there's no need to lead
			predict = 0;
		}
	}
	CWeapon::Update();

	if(lastFireFrame > gs->frameNum - 18 && lastFireFrame != gs->frameNum  && weaponDef->sweepFire)
	{
		if (gs->Team(owner->team)->metal>=metalFireCost && gs->Team(owner->team)->energy>=energyFireCost)
		{
			owner->UseEnergy(energyFireCost / salvoSize);
			owner->UseMetal(metalFireCost / salvoSize);

			std::vector<int> args;
			args.push_back(0);
			owner->cob->Call(COBFN_QueryPrimary+weaponNum,args);
			CMatrix44f weaponMat = owner->localmodel->GetPieceMatrix(args[0]);

			float3 relWeaponPos = weaponMat.GetPos();
			weaponPos=owner->pos+owner->frontdir*-relWeaponPos.z+owner->updir*relWeaponPos.y+owner->rightdir*-relWeaponPos.x;

			float3 dir = owner->frontdir * weaponMat[10] + owner->updir * weaponMat[6] + -owner->rightdir * weaponMat[2];
			FireInternal(dir, true);
		}
	}
}

bool CBeamLaser::TryTarget(const float3& pos, bool userTarget,CUnit* unit)
{
	if (!CWeapon::TryTarget(pos, userTarget, unit))
		return false;

	if (!weaponDef->waterweapon) {
		if (unit) {
			if (unit->isUnderWater)
				return false;
		} else {
			if (pos.y < 0)
				return false;
		}
	}

	float3 dir = pos - weaponMuzzlePos;
	float length = dir.Length();

	if (length == 0)
		return true;

	dir /= length;

	if (!onlyForward) {
		// skip ground col testing for aircraft
		float g = ground->LineGroundCol(weaponMuzzlePos, pos);
		if (g > 0 && g < length * 0.9f)
			return false;
	}

	float spread = (accuracy + sprayAngle) * (1 - owner->limExperience * 0.7f);

	if (avoidFeature && helper->LineFeatureCol(weaponMuzzlePos, dir, length)) {
		return false;
	}
	if (avoidFriendly) {
		if (helper->TestAllyCone(weaponMuzzlePos, dir, length, spread, owner->allyteam, owner))
			return false;
	}
	if (avoidNeutral) {
		if (helper->TestNeutralCone(weaponMuzzlePos, dir, length, spread, owner))
			return false;
	}

	return true;
}

void CBeamLaser::Init(void)
{
	if (!weaponDef->beamburst) {
		salvoDelay = 0;
		salvoSize = (int) (weaponDef->beamtime * 30);

		if (salvoSize <= 0)
			salvoSize = 1;

		// multiply damage with this on each shot so the total damage done is correct
		damageMul = 1.0f / (float) salvoSize;
	}
	else {
		damageMul = 1.0f;
	}

	CWeapon::Init();
	muzzleFlareSize = 0;
}

void CBeamLaser::Fire(void)
{
	float3 dir;
	if (onlyForward && dynamic_cast<CAirMoveType*>(owner->moveType)) {
		// the taairmovetype can't align itself properly, change back when that is fixed
		dir = owner->frontdir;
	} else {
		if (salvoLeft == salvoSize - 1) {
			if (fireSoundId)
				sound->PlaySample(fireSoundId, owner, fireSoundVolume);

			dir = targetPos - weaponMuzzlePos;
			dir.Normalize();
			oldDir = dir;
		} else if (weaponDef->beamburst) {
			if (fireSoundId && !weaponDef->soundTrigger)
				sound->PlaySample(fireSoundId, owner, fireSoundVolume);

			dir = targetPos-weaponMuzzlePos;
			dir.Normalize();
		} else {
			dir = oldDir;
		}
	}

	dir += (salvoError) * (1 - owner->limExperience * 0.7f);
	dir.Normalize();

	FireInternal(dir, false);
}

void CBeamLaser::FireInternal(float3 dir, bool sweepFire)
{
	float rangeMod=1.3f;
#ifdef DIRECT_CONTROL_ALLOWED
	if(owner->directControl)
		rangeMod=0.95f;
#endif

	float maxLength=range*rangeMod;
	float curLength=0;
	float3 curPos=weaponMuzzlePos;
	float3 hitPos;
	dir += gs->randVector() * sprayAngle * (1 - owner->limExperience * 0.7f);
	dir.Normalize();

	bool tryAgain=true;
	CUnit* hit;

	// increase range if targets are searched for in a cylinder
	if (cylinderTargetting > 0.01) {
		//const float3 up(0, owner->radius*cylinderTargetting, 0);
		//const float uplen = up.dot(dir);
		const float uplen = owner->radius*cylinderTargetting * dir.y;
		maxLength = sqrt(maxLength*maxLength + uplen*uplen);
	}

	// increase range if targetting edge of hitsphere
	if (targetType == Target_Unit && targetUnit && targetBorder != 0) {
		maxLength += targetUnit->radius*targetBorder;
	}

	for(int tries=0;tries<5 && tryAgain;++tries){
		tryAgain=false;
		hit=0;
		float length = helper->TraceRay(curPos, dir, maxLength - curLength,
			weaponDef->damages[0], owner, hit, collisionFlags);

		if(hit && hit->allyteam == owner->allyteam && sweepFire){	//never damage friendlies with sweepfire
			lastFireFrame = 0;
			return;
		}

		float3 newDir;
		CPlasmaRepulser* shieldHit;
		float shieldLength=interceptHandler.AddShieldInterceptableBeam(this,curPos,dir,length,newDir,shieldHit);
		if(shieldLength<length){
			length=shieldLength;
			bool repulsed=shieldHit->BeamIntercepted(this, damageMul);
			if(repulsed){
				tryAgain=true;
			}
		}

		hitPos=curPos+dir*length;

		float baseAlpha=weaponDef->intensity*255;
		float startAlpha=(1-curLength/(range*1.3f))*baseAlpha;
		float endAlpha=(1-(curLength+length)/(range*1.3f))*baseAlpha;

		if (weaponDef->largeBeamLaser)
			SAFE_NEW CLargeBeamLaserProjectile(curPos, hitPos, color,
			weaponDef->visuals.color2, owner, weaponDef);
		else
			SAFE_NEW CBeamLaserProjectile(curPos, hitPos, startAlpha, endAlpha,
				color, weaponDef->visuals.color2, owner, weaponDef->thickness,
				weaponDef->corethickness, weaponDef->laserflaresize, weaponDef,
				weaponDef->visuals.beamttl, weaponDef->visuals.beamdecay);

		curPos = hitPos;
		curLength += length;
		dir = newDir;
	}

	// fix negative damage when hitting big spheres
	float actualRange = range;
	if (hit && targetBorder > 0) {
		actualRange += hit->radius*targetBorder;
	}
	// make it possible to always hit with some minimal intensity (melee weapons have use for that)
	float intensity = std::max(minIntensity, 1-(curLength)/(actualRange*2));

	if(curLength<maxLength) {
		// Dynamic Damage
		DamageArray dynDamages;
		if (weaponDef->dynDamageExp > 0)
			dynDamages = weaponDefHandler->DynamicDamages(weaponDef->damages, weaponMuzzlePos, curPos, weaponDef->dynDamageRange>0?weaponDef->dynDamageRange:weaponDef->range, weaponDef->dynDamageExp, weaponDef->dynDamageMin, weaponDef->dynDamageInverted);
		helper->Explosion(hitPos, weaponDef->dynDamageExp>0?dynDamages*(intensity*damageMul):weaponDef->damages*(intensity*damageMul), areaOfEffect, weaponDef->edgeEffectiveness, weaponDef->explosionSpeed,owner, true, 1.0f, false, weaponDef->explosionGenerator, hit, dir, weaponDef->id);
	}

	if(targetUnit)
		lastFireFrame = gs->frameNum;
}
