#ifndef SCRIPTHANDLER_H
#define SCRIPTHANDLER_H
// ScriptHandler.h: interface for the CScriptHandler class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#include <string>
#include <map>
#include <list>
#include "Script.h"
#include "Rendering/GL/glList.h"

using namespace std;

class CScriptHandler  
{
public:
	void AddScript(string name,CScript* s);
	static void SelectScript(std::string s);

	static CScriptHandler& Instance();
//	static void UnloadInstance();

	CScript* chosenScript;
	CglList* list;
	string chosenName;
	std::map<std::string,CScript*> scripts;
private:
  std::list<CScript*> loaded_scripts; ///< Scripts loaded and owned by CScriptHandler 
	CScriptHandler();
	CScriptHandler(CScriptHandler const&);
	CScriptHandler& operator=(CScriptHandler const&);
  void LoadScripts();
	virtual ~CScriptHandler();
};

#endif /* SCRIPTHANDLER_H */
