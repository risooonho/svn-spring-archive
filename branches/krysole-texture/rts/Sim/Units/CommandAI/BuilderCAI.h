#ifndef __BUILDER_CAI_H__
#define __BUILDER_CAI_H__

#include <map>
#include "MobileCAI.h"

class CBuilderCAI :
	public CMobileCAI
{
public:
	CBuilderCAI(CUnit* owner);
	~CBuilderCAI(void);
	int GetDefaultCmd(CUnit* unit,CFeature* feature);
	void SlowUpdate();

	void DrawCommands(void);
	void GiveCommand(Command& c);
	void DrawQuedBuildingSquares(void);

	bool FindReclaimableFeatureAndReclaim(float3 pos, float radius,unsigned char options, bool recAny);
	bool FindResurrectableFeatureAndResurrect(float3 pos, float radius,unsigned char options);
	void FinishCommand(void);
	bool FindRepairTargetAndRepair(float3 pos, float radius,unsigned char options,bool attackEnemy);
	bool FindCaptureTargetAndCapture(float3 pos, float radius,unsigned char options);

	map<int,string> buildOptions;
	bool building;
	float3 buildPos;

	float cachedRadius;
	int cachedRadiusId;

	int buildRetries;

	int lastPC1; //helps avoid infinite loops
	int lastPC2;
};

#endif // __BUILDER_CAI_H__
