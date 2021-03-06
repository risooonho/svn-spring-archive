// ScriptHandler.cpp: implementation of the CScriptHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ScriptHandler.h"
#include "Game/Game.h"
#include "mmgr.h"
#include "FileSystem/FileHandler.h"
#include "LoadScript.h"
#include "CommanderScript.h"
#include "CommanderScript2.h"
#include "AirScript.h"
#include "GlobalAITestScript.h"
#include "SpawnScript.h"
#include "EmptyScript.h"
#include "TestScript.h"
#ifndef NO_LUA
#include "System/Platform/errorhandler.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScriptHandler::CScriptHandler() : chosenScript(0), callback(0)
{
}

/** Load all scripts. */
void CScriptHandler::LoadScripts() {

	loaded_scripts.push_back( new CCommanderScript() );
	loaded_scripts.push_back( new CCommanderScript2() );
	loaded_scripts.push_back( new CAirScript() );
	loaded_scripts.push_back( new CEmptyScript() );
	loaded_scripts.push_back( new CSpawnScript(false) );
	loaded_scripts.push_back( new CSpawnScript(true) );
	loaded_scripts.push_back( new CTestScript() );

#ifdef WIN32
	std::vector<std::string> f = CFileHandler::FindFiles("AI\\Bot-libs\\", "*.dll");
#elif defined(__APPLE__)
	std::vector<std::string> f = CFileHandler::FindFiles("AI/Bot-libs/", "*.dylib");
#else
	std::vector<std::string> f = CFileHandler::FindFiles("AI/Bot-libs/", "*.so");
#endif
	for(std::vector<std::string>::iterator fi = f.begin(), e = f.end(); fi != e; ++fi) {
		string name = fi->substr(fi->find_last_of('\\') + 1);
		loaded_scripts.push_back(new CGlobalAITestScript(name));
	}

	f = CFileHandler::FindFiles("", "*.ssf");
	for(std::vector<std::string>::iterator fi = f.begin(), e = f.end(); fi != e; ++fi) {
		loaded_scripts.push_back(new CLoadScript(*fi));
	}
}

void CScriptHandler::StartLua()
{
#ifndef NO_LUA
	std::vector<string> files = CFileHandler::FindFiles("startscripts/", "*.lua");
	for (std::vector<string>::iterator i = files.begin(); i != files.end(); ++i) {
		CLuaBinder* lua = new CLuaBinder();
		if (!lua->LoadScript(*i)) 
			handleerror(NULL, lua->lastError.c_str(), "Lua", MBF_OK|MBF_EXCL);
		lua_binders.push_back(lua);
	}
#endif
}

/** @Return a reference to the only CScriptHandler instance */
CScriptHandler& CScriptHandler::Instance()
{
	static bool created = false;
	static CScriptHandler instance;
	if( !created ) {
		created = true;
		instance.LoadScripts();
	}
	return instance;
}

CScriptHandler::~CScriptHandler()
{
	while(!loaded_scripts.empty()) {
		delete loaded_scripts.back();
		loaded_scripts.pop_back();
	}

#ifndef NO_LUA
	while (!lua_binders.empty()) {
		delete lua_binders.back();
		lua_binders.pop_back();
	}
#endif
}

/** Called by the CScript constructors to add themselves to the CScriptHandler. */
void CScriptHandler::AddScript(string name, CScript *s)
{
	scripts[name] = s;
}

/** Called by the CglList generated by CScriptHandler::GenList on selection of a script. */
void CScriptHandler::SelectScript(std::string s)
{
	CScriptHandler::Instance().chosenName = s;
	CScriptHandler::Instance().chosenScript = CScriptHandler::Instance().scripts[s];
	if (CScriptHandler::Instance().callback)
		CScriptHandler::Instance().callback(s);
}

/** Generate a CglList with all available scripts. */
CglList* CScriptHandler::GenList(ListSelectCallback callb)
{
	CglList* list = new CglList("Select script", SelectScript, 1);
	for (std::map<std::string,CScript*>::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
		list->AddItem(it->first.c_str(), it->first.c_str());
	callback = callb;
	return list;
}
