// This file contains implementations of various functions exposed through lua
// The goal is to include as few headers in LuaBinder.cpp as possible, and include
// them here instead to avoid recompiling LuaBinder too often.

#include "StdAfx.h"
#include "LuaFunctions.h"
#include "Game/StartScripts/Script.h"
#include "float3.h"
#include "Game/UI/InfoConsole.h"
#include "GlobalStuff.h"
#include "Sim/Units/Unit.h"
#include "Sim/Units/UnitTypes/TransportUnit.h"
#include "Sim/Units/UnitLoader.h"
#include "TdfParser.h"
#include "Game/command.h"
#include "Sim/Units/CommandAI/CommandAI.h"
#include "Sim/Misc/QuadField.h"
#include "Game/Game.h"
#include "Game/UI/EndGameBox.h"
#include "Sim/Units/UnitHandler.h"
#include "Map/ReadMap.h"
#include "Game/SelectedUnits.h"

using namespace std;
using namespace luabind;

extern std::string stupidGlobalMapname;

namespace luafunctions 
{

	// This should use net stuff instead of duplicating code here
	void EndGame()
	{
		new CEndGameBox();
		game->gameOver = true;
	}

	void UnitGiveCommand(CObject_pointer<CUnit>* u, Command* c)
	{
		if (!u->held)
			return;
	
		u->held->commandAI->GiveCommand(*c);
	}

	void CommandAddParam(Command* c, float p)
	{
		c->params.push_back(p);
	}

	CObject_pointer<CUnit>* UnitLoaderLoadUnit(string name, float3 pos, int team, bool buil)
	{
		CUnit* x = unitLoader.LoadUnit(name, pos, team, buil);
		return new CObject_pointer<CUnit>(x);
	}

	CObject_pointer<CUnit>* UnitGetTransporter(CObject_pointer<CUnit>* u)
	{
		CUnit* x = u->held;
		if (x->transporter)
			return new CObject_pointer<CUnit>(x->transporter);
		else 
			return NULL;
	}

	//	vector<CUnit*> GetUnits(const float3& pos,float radius);
	//	vector<CUnit*> GetUnitsExact(const float3& pos,float radius);

	int GetNumUnitsAt(const float3& pos, float radius)
	{
		vector<CUnit*> x = qf->GetUnits(pos, radius);
		return x.size();
	}

	object GetUnitsAt(lua_State* L, const float3& pos, float radius)
	{
		vector<CUnit*> x = qf->GetUnits(pos, radius);
		object o = newtable(L);

		int count = 1;
		for (vector<CUnit*>::iterator i = x.begin(); i != x.end(); ++i)
			o[count++] = new CObject_pointer<CUnit>(*i);
		
		return o;
	}

	object GetSelectedUnits(lua_State* L, int player)
	{
		object o = newtable(L);

		for (int i = 0; i < selectedUnits.netSelected[player].size(); ++i)
			o[i+1] = new CObject_pointer<CUnit>(uh->units[selectedUnits.netSelected[player][i]]);

		return o;
	}

	void SendSelectedUnits()
	{
		if (selectedUnits.selectionChanged)
			selectedUnits.SendSelection();
	}

	string MapGetTDFName()
	{
		return CReadMap::GetTDFName(stupidGlobalMapname);
	}
}
