#include "GlobalAI.h"

std::set<IGlobalAI*> ais;


DLL_EXPORT int GetGlobalAiVersion() {
	return GLOBAL_AI_INTERFACE_VERSION;
}

DLL_EXPORT void GetAiName(char* name) {
	strcpy(name, AI_NAME);
}

DLL_EXPORT IGlobalAI* GetNewAI() {
	CGlobalAI* ai = new CGlobalAI();
	ais.insert(ai);
	return ai;
}

DLL_EXPORT void ReleaseAI(IGlobalAI* i) {
	ais.erase(i);
	delete (CGlobalAI*) i;
}

DLL_EXPORT int IsCInterface(void) {
	return 0;
}
