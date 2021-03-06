/*
	Copyright (c) 2008 Robin Vobruba <hoijui.quaero@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SAIINTERFACELIBRARY_H
#define	_SAIINTERFACELIBRARY_H
#if !defined BUILDING_AI

#include "creg/creg_cond.h"

#ifdef	__cplusplus
extern "C" {
#endif

#include "ELevelOfSupport.h"
#include "SSAILibrary.h"
//#include "SGAILibrary.h"
#include "System/exportdefines.h"

struct SStaticGlobalData;

#define AI_INTERFACE_PROPERTY_DATA_DIR               "dataDir"               // [string] absolute data dir containing the AIs AIInfo.lua file. this property is by the engine, not read from any file.
//#define AI_INTERFACE_PROPERTY_FILE_NAME              "fileName"              // [string] when the library file is "libC-0.1.so" or "C-0.1.dll", this value should be "C-0.1"
#define AI_INTERFACE_PROPERTY_SHORT_NAME             "shortName"             // [string: [a-zA-Z0-9_.]*]
#define AI_INTERFACE_PROPERTY_VERSION                "version"               // [string: [a-zA-Z0-9_.]*]
#define AI_INTERFACE_PROPERTY_NAME                   "name"                  // [string]
#define AI_INTERFACE_PROPERTY_DESCRIPTION            "description"           // [string]
#define AI_INTERFACE_PROPERTY_URL                    "url"                   // [string]
#define AI_INTERFACE_PROPERTY_SUPPORTED_LANGUAGES    "supportedLanguages"    // [string]
#define AI_INTERFACE_PROPERTY_ENGINE_VERSION         "engineVersion"         // [int] the engine version number the AI was compiled, but may work with newer or older ones too

///**
// * @brief struct Artificial Intelligence Interface Specifier
// */
//struct SAIInterfaceSpecifier {
//	const char* shortName; // [may not contain: spaces, '_', '#']
//	const char* version; // [may not contain: spaces, '_', '#']
//};

//struct SAIInterfaceSpecifier copySAIInterfaceSpecifier(
//		const struct SAIInterfaceSpecifier* const orig);
//void deleteSAIInterfaceSpecifier(
//		const struct SAIInterfaceSpecifier* const spec);


///**
// * @brief struct Skirmish Artificial Intelligence Key
// * Compleetly specifies a skirmish AI together with an interface.
// */
//struct SSAIKey {
//	CR_DECLARE_STRUCT(SSAIKey);
//	struct SAIInterfaceSpecifier interface;
//	struct SSAISpecifier ai;
//};

/**
 * @brief struct Group Artificial Intelligence Key
 * Compleetly specifies a group AI together with an interface.
 */
//struct SGAIKey {
//	CR_DECLARE_STRUCT(SGAIKey);
//	struct SAIInterfaceSpecifier interface;
//	struct SGAISpecifier ai;
//};

/**
 * @brief struct Artificial Intelligence Interface
 */
struct SAIInterfaceLibrary {

	// static AI interface library functions

	/**
	 * This function is called right after the library is dynamically loaded.
	 * It can be used to initialize variables and to check or prepare
	 * the environment (os, engine, filesystem, ...).
	 * See also releaseStatic().
	 *
	 * NOTE: param staticGlobalData is guaranteed to be valid till
	 * releaseStatic() is called.
	 *
	 * NOTE: this method is optional. An AI Interface not exporting this
	 * function is still valid.
	 *
	 * @param	staticGlobalData	contains global data about hte engine
	 *								and the environment
	 * @return	ok: 0, error: != 0
	 */
	int (CALLING_CONV *initStatic)(
			unsigned int infoSize,
			const char** infoKeys, const char** infoValues,
			const struct SStaticGlobalData* staticGlobalData);

	/**
	 * This function is called right right before the library is unloaded.
	 * It can be used to deinitialize variables and to cleanup the environment,
	 * for example the filesystem.
	 *
	 * See also initStatic().
	 *
	 * NOTE: this method is optional. An AI Interface not exporting this
	 * function is still valid.
	 *
	 * @return	ok: 0, error: != 0
	 */
	int (CALLING_CONV *releaseStatic)();

	/**
	 * Level of Support for a specific engine version.
	 *
	 * NOTE: this method is optional. An AI Interface not exporting this
	 * function is still valid.
	 */
	enum LevelOfSupport (CALLING_CONV *getLevelOfSupportFor)(
			const char* engineVersionString, int engineVersionNumber);
	/**
	 * Returns static properties with info about this AI Interface library.
	 *
	 * NOTE: this method is optional. An AI Interface not exporting this
	 * function is still valid.
	 *
	 * @return number of elements stored into parameter info
	 */
//	unsigned int (CALLING_CONV *getInfo)(struct InfoItem info[],
//			unsigned int maxInfoItems);


	// skirmish AI methods

	/**
	 * Loads the specified Skirmish AI.
	 *
	 * @return	ok: 0, error: != 0
	 */
	//int (CALLING_CONV *getSkirmishAISpecifiers)(struct SSAISpecifier* sAISpecifiers, int max);
	//const struct SSAILibrary* (CALLING_CONV *loadSkirmishAILibrary)(const struct SSAISpecifier* const sAISpecifier);
	const struct SSAILibrary* (CALLING_CONV *loadSkirmishAILibrary)(
			unsigned int infoSize,
			const char** infoKeys, const char** infoValues);

	/**
	 * Unloads the specified Skirmish AI.
	 *
	 * @return	ok: 0, error: != 0
	 */
	int (CALLING_CONV *unloadSkirmishAILibrary)(
			unsigned int infoSize,
			const char** infoKeys, const char** infoValues);

	/**
	 * Unloads all Skirmish AI libraries currently loaded by this interface.
	 *
	 * @return	ok: 0, error: != 0
	 */
	int (CALLING_CONV *unloadAllSkirmishAILibraries)();


	// group AI methods

	/**
	 * Loads the specified Group AI.
	 *
	 * @return	ok: 0, error: != 0
	 */
	//int (CALLING_CONV *getGroupAISpecifiers)(struct SGAISpecifier* gAISpecifiers, int max);
	//const struct SGAILibrary* (CALLING_CONV *loadGroupAILibrary)(const struct SGAISpecifier* const gAISpecifier);
//	const struct SGAILibrary* (CALLING_CONV *loadGroupAILibrary)(
//			const struct InfoItem info[], unsigned int numInfoItems);

	/**
	 * Unloads the specified Group AI.
	 *
	 * @return	ok: 0, error: != 0
	 */
//	int (CALLING_CONV *unloadGroupAILibrary)(
//			const struct SGAISpecifier* const gAISpecifier);

	/**
	 * Unloads all Group AI libraries currently loaded by this interface.
	 *
	 * @return	ok: 0, error: != 0
	 */
//	int (CALLING_CONV *unloadAllGroupAILibraries)();
};

#ifdef	__cplusplus
}	// extern "C"
#endif


#ifdef	__cplusplus
//struct SAIInterfaceSpecifier_Comparator {
//	/**
//	 * The key comparison function, a Strict Weak Ordering;
//	 * it returns true if its first argument is less
//	 * than its second argument, and false otherwise.
//	 * This is also defined as map::key_compare.
//	 */
//	bool operator()(const struct SAIInterfaceSpecifier& a,
//			const struct SAIInterfaceSpecifier& b) const;
//	static bool IsEmpty(const struct SAIInterfaceSpecifier& spec);
//};
//
//struct SSAIKey_Comparator {
//	/**
//	 * The key comparison function, a Strict Weak Ordering;
//	 * it returns true if its first argument is less
//	 * than its second argument, and false otherwise.
//	 * This is also defined as map::key_compare.
//	 */
//	bool operator()(const struct SSAIKey& a, const struct SSAIKey& b) const;
//	static bool IsEmpty(const struct SSAIKey& key);
//};

//struct SGAIKey_Comparator {
//	/**
//	 * The key comparison function, a Strict Weak Ordering;
//	 * it returns true if its first argument is less
//	 * than its second argument, and false otherwise.
//	 * This is also defined as map::key_compare.
//	 */
//	bool operator()(const struct SGAIKey& a, const struct SGAIKey& b) const;
//	static bool IsEmpty(const struct SGAIKey& key);
//};
#endif // __cplusplus

#endif // !defined BUILDING_AI
#endif // _SAIINTERFACELIBRARY_H
