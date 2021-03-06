//#ifndef GLOBALAIHANDLER_H
//#define GLOBALAIHANDLER_H
//
//#include "IAILibraryManager.h"
//#include "ISkirmishAI.h"
//#include "Object.h"
//#include "Platform/ConfigHandler.h"
//#include "float3.h"
//#include "Game/GlobalConstants.h"
//#include <map>
//#include <string>
//
//class CUnit;
//class CGlobalAI;
//class CSkirmishAIWrapper;
//struct WeaponDef;
//struct Command;
//
//class CGlobalAIHandler :
//	public CObject
//{
//public:
//	CR_DECLARE(CGlobalAIHandler);
////	CR_DECLARE_SUB(AIMemBuffer);
//	static bool CatchException();
//
//	CGlobalAIHandler();
//	~CGlobalAIHandler();
//
//	void PostLoad();
//	void PreDestroy();
//
//	void Update();
//	void UnitEnteredLos(CUnit* unit, int allyTeam);
//	void UnitLeftLos(CUnit* unit, int allyTeam);
//	void UnitEnteredRadar(CUnit* unit, int allyTeam);
//	void UnitLeftRadar(CUnit* unit, int allyTeam);
//	void SeismicPing(int allyteam, CUnit *unit, const float3 &pos, float strength);
//
//	void UnitIdle(CUnit* unit);
//	void UnitCreated(CUnit* unit);
//	void UnitFinished(CUnit* unit);
//	void UnitDestroyed(CUnit* unit, CUnit *attacker);
//	void UnitDamaged(CUnit* attacked, CUnit* attacker, float damage);
//	void UnitMoveFailed(CUnit* unit);
//	void UnitTaken(CUnit* unit, int newTeam);
//	void UnitGiven(CUnit* unit, int oldTeam);
//	void WeaponFired(CUnit* unit, const WeaponDef* def);
//	void PlayerCommandGiven(std::vector<int>& selectedunits,Command& c,int player);
//	void Load(std::istream *s);
//	void Save(std::ostream *s);
//	void GotChatMsg(const char* msg, int player);
//
//	bool CreateSkirmishAI(int teamId, const SSAIKey& skirmishAIKey,
//			const std::map<std::string, std::string>& skirmishAIOptions);
//	bool IsSkirmishAI(int teamId);
//	void DestroySkirmishAI(int teamId);
//	const CSkirmishAIWrapper* GetSkirmishAI(int teamId);
//
//private:
//	CSkirmishAIWrapper* ais[MAX_TEAMS];
//	bool hasAI;
//};
//
//extern CGlobalAIHandler* globalAI;
//
//#endif
