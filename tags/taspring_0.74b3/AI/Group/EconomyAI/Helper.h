#include "MetalMap.h"

struct UnitDef;

class CHelper
{
	public:
		CHelper(IAICallback* aicb);
		virtual ~CHelper();

		pair<int,int> BuildNameToId(string name, int unit);
		string	BuildIdToName(int id, int unit);
		float3	FindBuildPos(string name, bool isMex, bool isGeo, float distance, int builder);
		void	DrawBuildArea();
		void	NewLocation(float3 centerPos, float radius);
		void	ResetLocations();
		void	AssignMetalMakerAI();
		void	SendTxt(const char *fmt, ...);
		void	ParseBuildOptions(map<string,const UnitDef*> &targetBO, const UnitDef* unitDef, bool recursive);

		float3	errorPos;
		CMetalMap* metalMap;
		int* friendlyUnits;
		int myTeam;
		float extractorRadius;
		float mmkrME;						// metalmaker M / E ratio
		float maxPartitionRadius;
	private:
		bool	IsMetalSpotAvailable(float3 spot,float extraction);
		int		FindMetalSpots(float3 pos, float radius, vector<float3>* mexSpots);

		IAICallback* aicb;
		struct partition
		{
			float3 pos;
			string name;
			bool taken;
			bool empty;
		};
		struct location
		{
			float3 centerPos;
			float radius;
			float partitionRadius;
			int numPartitions;
			int squarePartitions;
			vector<float3> mexSpots;
			vector<partition> partitions;
		};
		vector<location*> locations;
		int metalMakerAIid;
		const UnitDef* geoDef;
		float drawColor[4];
};

