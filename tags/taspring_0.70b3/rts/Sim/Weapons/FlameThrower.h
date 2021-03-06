#ifndef __FLAME_THROWER_H__
#define __FLAME_THROWER_H__

#include "Weapon.h"

class CFlameThrower :
	public CWeapon
{
public:
	CFlameThrower(CUnit* owner);
	~CFlameThrower(void);
	void Fire(void);
	bool TryTarget(const float3 &pos,bool userTarget,CUnit* unit);
	void Update(void);
};

#endif // __FLAME_THROWER_H__
