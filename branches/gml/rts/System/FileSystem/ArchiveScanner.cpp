#include "ArchiveScanner.h"

#include <list>
#include <algorithm>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "StdAfx.h"
#include "Lua/LuaParser.h"
#include "System/LogOutput.h"
#include "System/FileSystem/ArchiveFactory.h"
#include "System/FileSystem/ArchiveBuffered.h"
#include "System/FileSystem/CRC.h"
#include "System/FileSystem/FileFilter.h"
#include "System/FileSystem/FileHandler.h"
#include "System/Platform/FileSystem.h"
#include "mmgr.h"

using std::string;
using std::vector;


// fix for windows
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & 0170000) == 0040000) /* directory */
#endif

/*
 * The archive scanner is used to find stuff in archives that are needed before building the virtual
 * filesystem. This currently includes maps and mods. It uses caching to speed up the process.
 *
 * It only retrieves info that is used in an initial listing.. For detailed info when selecting a map
 * for example the more specialised parsers will be used. (mapping one archive when selecting a map
 * is not slow, but mapping them all every time to make the list is)
 */

#define INTERNAL_VER	7


CArchiveScanner* archiveScanner = NULL;


CArchiveScanner::CArchiveScanner(void)
: isDirty(false)
{
}


CArchiveScanner::~CArchiveScanner(void)
{
	if (isDirty) {
		WriteCacheData(filesystem.LocateFile(GetFilename(), FileSystem::WRITE));
	}
}


string CArchiveScanner::GetFilename()
{
	char buf[32];
	sprintf(buf, "ArchiveCacheV%i.lua", INTERNAL_VER);
	return string(buf);
}


CArchiveScanner::ModData CArchiveScanner::GetModData(const LuaTable& modTable)
{
	ModData md;
	md.name = "";

	if (!modTable.IsValid()) {
		return md;
	}

	md.name        = modTable.GetString("name",        "");
	md.shortName   = modTable.GetString("shortName",   "");
	md.version     = modTable.GetString("version",     "");
	md.mutator     = modTable.GetString("mutator",     "");
	md.game        = modTable.GetString("game",        "");
	md.shortGame   = modTable.GetString("shortGame",   "");
	md.description = modTable.GetString("description", "");
	md.modType     = modTable.GetInt("modType", 0);

	const LuaTable dependencies = modTable.SubTable("depend");
	for (int dep = 1; dependencies.KeyExists(dep); ++dep) {
		md.dependencies.push_back(dependencies.GetString(dep, ""));
	}

	const LuaTable replaces = modTable.SubTable("replace");
	for (int rep = 1; replaces.KeyExists(rep); ++rep) {
		md.replaces.push_back(replaces.GetString(rep, ""));
	}

	// HACK needed until lobbies, lobbyserver and unitsync are sorted out
	// so they can uniquely identify different versions of the same mod.
	// (at time of this writing they use name only)

	// NOTE when changing this, this function is used both by the code that
	// reads ArchiveCacheV#.lua and the code that reads modinfo.lua from the mod.
	// so make sure it doesn't keep adding stuff to the name everytime
	// Spring/unitsync is loaded.

	if (md.name.find(md.version) == string::npos) {
		md.name += " " + md.version;
	}

	return md;
}


void CArchiveScanner::PreScan(const string& curPath)
{
	const int flags = (FileSystem::INCLUDE_DIRS | FileSystem::RECURSE);
	vector<string> found = filesystem.FindFiles(curPath, "springcontent.sdz", flags);
	if (!found.empty()) {
		CArchiveBase* ar = CArchiveFactory::OpenArchive(found[0]);
		if (ar) {
			string name;
			int size;
			for (int cur = 0; cur = ar->FindFiles(cur, &name, &size); /* no-op */) {
				if (name == "gamedata/parse_tdf.lua") {
					const int fh = ar->OpenFile(name);
					if (fh != 0) {
						parse_tdf_path = found[0];
						ar->CloseFile(fh);
					}
				}
				else if (name == "gamedata/scanutils.lua") {
					const int fh = ar->OpenFile(name);
					if (fh != 0) {
						scanutils_path = found[0];
						ar->CloseFile(fh);
					}
				}
			}
			delete ar;
		}
	}
}


static bool LoadSourceFile(const string& archive,
                           const string& fileName, string& source)
{
	CArchiveBase* ar = CArchiveFactory::OpenArchive(archive);
	if (ar == NULL) {
		return false;
	}

	const int fh = ar->OpenFile(fileName);
	if (!fh) {
		delete ar;
		return false;
	}

	const int fsize = ar->FileSize(fh);
	char* buf = SAFE_NEW char[fsize];
	ar->ReadFile(fh, buf, fsize);
	ar->CloseFile(fh);
	source.clear();
	source.append(buf, fsize);
	delete [] buf;

	delete ar;

	return true;
}


void CArchiveScanner::ScanDirs(const vector<string>& scanDirs, bool checksum)
{
	// pre-scan for the modinfo utils
	for (unsigned int d = 0; d < scanDirs.size(); d++) {
		PreScan(scanDirs[d]);
	}

	LoadSourceFile(parse_tdf_path, "gamedata/parse_tdf.lua", parse_tdf_code);
	if (parse_tdf_code.empty()) {
		throw content_error("could not find 'gamedata/parse_tdf.lua' code");
	}

	LoadSourceFile(scanutils_path, "gamedata/scanutils.lua", scanutils_code);
	if (scanutils_code.empty()) {
		throw content_error("could not find 'gamedata/scanutils.lua' code");
	}

	// we don't want to return the parse_tdf table
	parse_tdf_code.erase(parse_tdf_code.find_last_of("}") + 1);
	// NOTE: this is a dangerous game to play,
	//       better to use a tag in the source file

	// add the archives
	for (unsigned int d = 0; d < scanDirs.size(); d++) {
		logOutput.Print("Scanning: %s\n", scanDirs[d].c_str());
		Scan(scanDirs[d], checksum);
	}
}


void CArchiveScanner::Scan(const string& curPath, bool checksum)
{
	isDirty = true;

	const int flags = (FileSystem::INCLUDE_DIRS | FileSystem::RECURSE);
	vector<string> found = filesystem.FindFiles(curPath, "*", flags);

	for (vector<string>::iterator it = found.begin(); it != found.end(); ++it) {
		string fullName = *it;

		// Strip
		const char lastFullChar = fullName[fullName.size() - 1];
		if ((lastFullChar == '/') || (lastFullChar == '\\')) {
			fullName = fullName.substr(0, fullName.size() - 1);
		}

		const string fn    = filesystem.GetFilename(fullName);
		const string fpath = filesystem.GetDirectory(fullName);
		const string lcfn    = StringToLower(fn);
		const string lcfpath = StringToLower(fpath);

		// Exclude archivefiles found inside directory archives (.sdd)
		if (lcfpath.find(".sdd") != string::npos) {
			continue;
		}

		// Exclude archivefiles found inside hidden directories
		if ((lcfpath.find("/hidden/")   != string::npos) ||
		    (lcfpath.find("\\hidden\\") != string::npos)) {
			continue;
		}

		// Is this an archive we should look into?
		if (CArchiveFactory::IsArchive(fullName)) {
			ScanArchive(fullName, checksum);
		}
	}

	// Now we'll have to parse the replaces-stuff found in the mods
	for (std::map<string, ArchiveInfo>::iterator aii = archiveInfo.begin(); aii != archiveInfo.end(); ++aii) {
		for (vector<string>::iterator i = aii->second.modData.replaces.begin(); i != aii->second.modData.replaces.end(); ++i) {

			const string lcname = StringToLower(*i);
			std::map<string, ArchiveInfo>::iterator ar = archiveInfo.find(lcname);

			// If it's not there, we will create a new entry
			if (ar == archiveInfo.end()) {
				ArchiveInfo tmp;
				archiveInfo[lcname] = tmp;
				ar = archiveInfo.find(lcname);
			}

			// Overwrite the info for this archive with a replaced pointer
			ar->second.path = "";
			ar->second.origName = lcname;
			ar->second.modified = 1;
			ar->second.mapData.clear();
			ar->second.modData.name = "";
			ar->second.modData.replaces.clear();
			ar->second.updated = true;
			ar->second.replaced = aii->first;
		}
	}
}


void CArchiveScanner::ScanArchive(const string& fullName, bool checksum)
{
	struct stat info;

	stat(fullName.c_str(), &info);

	const string fn    = filesystem.GetFilename(fullName);
	const string fpath = filesystem.GetDirectory(fullName);
	const string lcfn    = StringToLower(fn);
	const string lcfpath = StringToLower(fpath);

	// Determine whether to rely on the cached info or not
	bool cached = false;

	std::map<string, ArchiveInfo>::iterator aii = archiveInfo.find(lcfn);
	if (aii != archiveInfo.end()) {

		// This archive may have been obsoleted, do not process it if so
		if (aii->second.replaced.length() > 0) {
			return;
		}

		/*
			For truely correct updating of .sdd archives, this code should
			be enabled. Unfortunately it has as side effect that all files
			in all .sdd's always need to be stat()'ed, which really slows
			down program startup.

			An update can be forced anyway by removing ArchiveCacheV*.lua
			or renaming the archive.
		*/

		/*if (S_ISDIR(info.st_mode)) {
			struct stat info2;
			vector<string> sddfiles = filesystem.FindFiles(fpath, "*", FileSystem::RECURSE | FileSystem::INCLUDE_DIRS);
			for (vector<string>::iterator sddit = found.begin(); sddit != found.end(); ++sddit) {
				stat(sddit->c_str(), &info2);
				if (info.st_mtime < info2.st_mtime) {
					info.st_mtime = info2.st_mtime;
				}
			}
		}*/

		if ((unsigned)info.st_mtime == aii->second.modified && fpath == aii->second.path) {
			cached = true;
			aii->second.updated = true;
		}

		// If we are here, we could have invalid info in the cache
		// Force a reread if it's a directory archive, as st_mtime only
		// reflects changes to the directory itself, not the contents.
		if (!cached) {
			archiveInfo.erase(aii);
		}
	}

	// Time to parse the info we are interested in
	if (cached) {
		// If cached is true, aii will point to the archive
		if ((checksum) && (aii->second.checksum == 0)) {
			aii->second.checksum = GetCRC(fullName);
		}
	}
	else {
		CArchiveBase* ar = CArchiveFactory::OpenArchive(fullName);
		if (ar) {
			ArchiveInfo ai;

			string name;
			int size;
			for (int cur = 0; cur = ar->FindFiles(cur, &name, &size); /* no-op */) {
				//printf("found %s %d\n", name.c_str(), size);
				string ext = StringToLower(name.substr(name.find_last_of('.') + 1));

				// only accept new format maps
				if ((ext == "smf") || (ext == "sm3")) {
					ScanMap(ar, name, ai);
				}
				else if (name == "modinfo.lua") {
					ScanModLua(ar, name, ai);
				}
				else if (name == "modinfo.tdf") {
					ScanModTdf(ar, name, ai);
				}
			}

			ai.path = fpath;
			ai.modified = info.st_mtime;
			ai.origName = fn;
			ai.updated = true;

			delete ar;

			// Optionally calculate a checksum for the file
			// To prevent reading all files in all directory (.sdd) archives
			// every time this function is called, directory archive checksums
			// are calculated on the fly.
			if (checksum) {
				ai.checksum = GetCRC(fullName);
			} else {
				ai.checksum = 0;
			}

			archiveInfo[lcfn] = ai;
		}
	}
}


void CArchiveScanner::ScanMap(CArchiveBase* ar, const string& fileName,
                              ArchiveInfo& ai)
{
	MapData md;
	if ((fileName.find_last_of('\\') == string::npos) &&
			(fileName.find_last_of('/') == string::npos)) {
		md.name = fileName;
		md.virtualPath = "/";
	}
	else {
		if (fileName.find_last_of('\\') == string::npos) {
			md.name = fileName.substr(fileName.find_last_of('/') + 1);
			// include the backslash
			md.virtualPath = fileName.substr(0, fileName.find_last_of('/') + 1);
		}
		else {
			md.name = fileName.substr(fileName.find_last_of('\\') + 1);
			// include the backslash
			md.virtualPath = fileName.substr(0, fileName.find_last_of('\\') + 1);
		}
	}
	ai.mapData.push_back(md);
}


void CArchiveScanner::ScanModLua(CArchiveBase* ar, const string& fileName,
                                 ArchiveInfo& ai)
{
	const int fh = ar->OpenFile(fileName);
	if (fh) {
		const int fsize = ar->FileSize(fh);

		char* buf = SAFE_NEW char[fsize];
		ar->ReadFile(fh, buf, fsize);
		ar->CloseFile(fh);
		
		string cleanbuf;
		cleanbuf.append(buf, fsize);
		delete [] buf;
		LuaParser p(cleanbuf, SPRING_VFS_MOD);
		if (!p.Execute()) {
			logOutput.Print(p.GetErrorLog());
		}
		const LuaTable modTable = p.GetRoot();
		ai.modData = GetModData(modTable);
	}
}


void CArchiveScanner::ScanModTdf(CArchiveBase* ar, const string& fileName,
                                 ArchiveInfo& ai)
{
	const int fh = ar->OpenFile(fileName);
	if (fh) {
		const int fsize = ar->FileSize(fh);

		char* buf = SAFE_NEW char[fsize];
		ar->ReadFile(fh, buf, fsize);
		ar->CloseFile(fh);
		string cleanbuf;
		cleanbuf.append(buf, fsize);
		delete [] buf;
		const string luaFile =
				parse_tdf_code + "\n\n"
			+ scanutils_code + "\n\n"
			+ "local tdfModinfo, err = TDFparser.ParseText([[\n" 
			+ cleanbuf + "]])\n\n"
			+ "if (tdfModinfo == nil) then\n"
			+ "    error('Error parsing modinfo.tdf: ' .. err)\n"
			+ "end\n\n"
			+ "tdfModinfo.mod.depend  = MakeArray(tdfModinfo.mod, 'depend')\n"
			+ "tdfModinfo.mod.replace = MakeArray(tdfModinfo.mod, 'replace')\n\n"
			+ "return tdfModinfo.mod\n";
		LuaParser p(luaFile, SPRING_VFS_MOD);
		if (!p.Execute()) {
			logOutput.Print(p.GetErrorLog());
		}
		const LuaTable modTable = p.GetRoot();
		ai.modData = GetModData(modTable);
	}
}


IFileFilter* CArchiveScanner::CreateIgnoreFilter(CArchiveBase* ar)
{
	IFileFilter* ignore = IFileFilter::Create();
	int fh = ar->OpenFile("springignore.txt");

	if (fh) {
		int fsize = ar->FileSize(fh);
		char* buf = SAFE_NEW char[fsize];

		ar->ReadFile(fh, buf, fsize);
		ar->CloseFile(fh);

		// this automatically splits lines
		ignore->AddRule(string(buf, fsize));

		delete[] buf;
	}
	return ignore;
}


/** Get CRC of the data in the specified archive.
    Returns 0 if file could not be opened. */
unsigned int CArchiveScanner::GetCRC(const string& arcName)
{
	CRC crc;
	unsigned int digest;
	CArchiveBase* ar;
	std::list<string> files;

	// Try to open an archive
	ar = CArchiveFactory::OpenArchive(arcName);
	if (!ar) {
		return 0; // It wasn't an archive
	}

	// Load ignore list.
	IFileFilter* ignore = CreateIgnoreFilter(ar);

	string name;
	int size;
	// Sort all file paths for deterministic behaviour
	for (int cur = 0; cur = ar->FindFiles(cur, &name, &size); /* no-op */) {
		if (ignore->Match(name)) {
			continue;
		}
		const string lower = StringToLower(name); // case insensitive hash
		files.push_back(lower);
	}
	files.sort();

	// Add all files in sorted order
	for (std::list<string>::iterator i = files.begin(); i != files.end(); i++ ) {
		digest = CRC().Update(i->data(), i->size()).GetDigest();
		crc.Update(digest);
		crc.Update(ar->GetCrc32(*i));
	}

	delete ignore;
	delete ar;

	digest = crc.GetDigest();

	// A value of 0 is used to indicate no crc.. so never return that
	// Shouldn't happen all that often
	if (digest == 0) {
		return 4711;
	} else {
		return digest;
	}
}


void CArchiveScanner::ReadCacheData(const string& filename)
{
  LuaParser p(filename, SPRING_VFS_RAW, SPRING_VFS_ALL);
	
	if (!p.Execute()) {
		logOutput.Print(p.GetErrorLog());
	}
	const LuaTable archiveCache = p.GetRoot();
	const LuaTable archives = archiveCache.SubTable("archives");
	
	// Do not load old version caches
	const int ver = archiveCache.GetInt("internalVer", 0);
	if (ver != INTERNAL_VER) {
		return;
	}

	for (int i = 1; archives.KeyExists(i); ++i) {
	  const LuaTable curArchive = archives.SubTable(i);
		const LuaTable maps = curArchive.SubTable("maps");
		const LuaTable mod = curArchive.SubTable("modData");
		ArchiveInfo ai;

		ai.origName = curArchive.GetString("name", "");
		ai.path = curArchive.GetString("path", "");
		// don't use GetInt for modified and checksum as lua uses 32bit ints, no longs
		ai.modified = strtoul(curArchive.GetString("modified", "0").c_str(), 0, 10);
		ai.checksum = strtoul(curArchive.GetString("checksum", "0").c_str(), 0, 10);
		ai.updated = false;

		for (int m = 1; maps.KeyExists(m); ++m) {
			const LuaTable curMap = maps.SubTable(m);

			MapData md;
			md.name = curMap.GetString("name", "");
			md.virtualPath = curMap.GetString("virtualPath", "");

			ai.mapData.push_back(md);
		}

		ai.modData = GetModData(mod);

		string lcname = StringToLower(ai.origName);

		archiveInfo[lcname] = ai;
	}

	isDirty = false;
}


static inline void SafeStr(FILE* out, const char* prefix, const string& str)
{
	if (str.empty()) {
		return;
	}
	if (str.find_first_of("\\\"") == string::npos) {
		fprintf(out, "%s\"%s\",\n", prefix, str.c_str());
	} else {
		fprintf(out, "%s[[%s]],\n", prefix, str.c_str());
	}
}


void CArchiveScanner::WriteCacheData(const string& filename)
{
	if (!isDirty) {
		return;
	}

	FILE* out = fopen(filename.c_str(), "wt");
	if (!out) {
		return;
	}

	// First delete all outdated information
	for (std::map<string, ArchiveInfo>::iterator i = archiveInfo.begin(); i != archiveInfo.end(); ) {
		std::map<string, ArchiveInfo>::iterator next = i;
		next++;
		if (!i->second.updated) {
			archiveInfo.erase(i);
		}
		i = next;
	}

	fprintf(out, "local archiveCache = {\n\n");
	fprintf(out, "\tinternalver = %d,\n\n", INTERNAL_VER);
	fprintf(out, "\tarchives = {\n");
	std::map<string, ArchiveInfo>::const_iterator arcIt;
	for (arcIt = archiveInfo.begin(); arcIt != archiveInfo.end(); ++arcIt) {
		fprintf(out, "\t\t{\n");
		SafeStr(out, "\t\t\tname = ",            arcIt->second.origName);
		SafeStr(out, "\t\t\tpath = ",            arcIt->second.path); 
		fprintf(out, "\t\t\tmodified = \"%u\",\n", arcIt->second.modified);
		fprintf(out, "\t\t\tchecksum = \"%u\",\n", arcIt->second.checksum);
		SafeStr(out, "\t\t\treplaced = ",        arcIt->second.replaced);

		const vector<MapData>& mapData = arcIt->second.mapData;
		if (!mapData.empty()) {
			fprintf(out, "\t\t\tmaps = {\n");
			vector<MapData>::const_iterator mapIt;
			for (mapIt = mapData.begin(); mapIt != mapData.end(); ++mapIt) {
				fprintf(out, "\t\t\t\t{\n");
				SafeStr(out, "\t\t\t\t\tname = ",        mapIt->name);
				SafeStr(out, "\t\t\t\t\tvirtualpath = ", mapIt->virtualPath);
				fprintf(out, "\t\t\t\t},\n");
			}
			fprintf(out, "\t\t\t},\n");
		}

		// Any mod info? or just a map archive?
		const ModData& modData = arcIt->second.modData;
		if (modData.name != "") {
			fprintf(out, "\t\t\tmoddata = {\n");
			SafeStr(out, "\t\t\t\tname = ",         modData.name);
			SafeStr(out, "\t\t\t\tshortname = ",    modData.shortName);
			SafeStr(out, "\t\t\t\tversion = ",      modData.version);
			SafeStr(out, "\t\t\t\tmutator = ",      modData.mutator);
			SafeStr(out, "\t\t\t\tgame = ",         modData.game);
			SafeStr(out, "\t\t\t\tshortgame = ",    modData.shortGame);
			SafeStr(out, "\t\t\t\tdescription = ",  modData.description);
			fprintf(out, "\t\t\t\tmodtype = %d,\n", modData.modType);

			const vector<string>& modDeps = modData.dependencies;
			if (!modDeps.empty()) {
				fprintf(out, "\t\t\t\tdepend = {\n");
				vector<string>::const_iterator depIt;
				for (depIt = modDeps.begin(); depIt != modDeps.end(); ++depIt) {
					SafeStr(out, "\t\t\t\t\t", *depIt);
				}
				fprintf(out, "\t\t\t\t},\n");
			}
			
			const vector<string>& modReps = modData.replaces;
			if (!modReps.empty())  {
				fprintf(out, "\t\t\t\treplace = {\n");
				vector<string>::const_iterator repIt;
				for (repIt = modReps.begin(); repIt != modReps.end(); ++repIt) {
					SafeStr(out, "\t\t\t\t\t", *repIt);
				}
				fprintf(out, "\t\t\t\t},\n");
			}

			fprintf(out, "\t\t\t},\n");
		}

		fprintf(out, "\t\t},\n");
	}

	fprintf(out, "\t},\n"); // close 'archives'
	fprintf(out, "}\n\n"); // close 'archiveCache'
	fprintf(out, "return archiveCache\n");

	fclose(out);

	isDirty = false;
}


vector<CArchiveScanner::ModData> CArchiveScanner::GetPrimaryMods() const
{
	vector<ModData> ret;

	for (std::map<string, ArchiveInfo>::const_iterator i = archiveInfo.begin(); i != archiveInfo.end(); ++i) {
		if (i->second.modData.name != "") {

			if (i->second.modData.modType != 1) {
				continue;
			}

			// Add the archive the mod is in as the first dependency
			ModData md = i->second.modData;
			md.dependencies.insert(md.dependencies.begin(), i->second.origName);
			ret.push_back(md);
		}
	}

	return ret;
}


vector<CArchiveScanner::ModData> CArchiveScanner::GetAllMods() const
{
	vector<ModData> ret;

	for (std::map<string, ArchiveInfo>::const_iterator i = archiveInfo.begin(); i != archiveInfo.end(); ++i) {
		if (i->second.modData.name != "") {
			// Add the archive the mod is in as the first dependency
			ModData md = i->second.modData;
			md.dependencies.insert(md.dependencies.begin(), i->second.origName);
			ret.push_back(md);
		}
	}

	return ret;
}


vector<string> CArchiveScanner::GetArchives(const string& root, int depth)
{
	// Protect against circular dependencies
	// (worst case depth is if all archives form one huge dependency chain)
	if ((unsigned)depth > archiveInfo.size()) {
		throw content_error("Circular dependency");
	}

	vector<string> ret;
	string lcname = StringToLower(ModNameToModArchive(root));
	std::map<string, ArchiveInfo>::iterator aii = archiveInfo.find(lcname);
	if (aii == archiveInfo.end()) {
		return ret;
	}

	// Check if this archive has been replaced
	while (aii->second.replaced.length() > 0) {
		aii = archiveInfo.find(aii->second.replaced);
		if (aii == archiveInfo.end()) {
			return ret;
		}
	}

	ret.push_back(aii->second.path + aii->second.origName);

	if (aii->second.modData.name == "") {
		return ret;
	}

	// add depth-first
	for (vector<string>::iterator i = aii->second.modData.dependencies.begin(); i != aii->second.modData.dependencies.end(); ++i) {
		vector<string> dep = GetArchives(*i, depth + 1);
		for (vector<string>::iterator j = dep.begin(); j != dep.end(); ++j) {
			ret.push_back(*j);
		}
	}

	// add springcontent.sdz for primary mod archives
	if ((depth == 0) && (aii->second.modData.modType == 1)) {
		const string springContentPath = GetArchivePath("springcontent.sdz");
		if (springContentPath.empty()) {
			throw content_error("missing springcontent.sdz");
		} else {
			printf("Added springcontent.sdz for %s\n", root.c_str());
			ret.push_back(springContentPath + "springcontent.sdz");
		}
	}

	return ret;
}


vector<string> CArchiveScanner::GetMaps()
{
	vector<string> ret;

	for (std::map<string, ArchiveInfo>::iterator aii = archiveInfo.begin(); aii != archiveInfo.end(); ++aii) {
		for (vector<MapData>::iterator i = aii->second.mapData.begin(); i != aii->second.mapData.end(); ++i) {
			ret.push_back((*i).name);
		}
	}

	return ret;
}


vector<string> CArchiveScanner::GetArchivesForMap(const string& mapName)
{
	vector<string> ret;

	for (std::map<string, ArchiveInfo>::iterator aii = archiveInfo.begin(); aii != archiveInfo.end(); ++aii) {
		for (vector<MapData>::iterator i = aii->second.mapData.begin(); i != aii->second.mapData.end(); ++i) {
			if (mapName == (*i).name) {
				ret = GetArchives(aii->first);
				const string mapHelperPath = GetArchivePath("maphelper.sdz");
				if (mapHelperPath.empty()) {
					throw content_error("missing maphelper.sdz");
				} else {
					ret.push_back(mapHelperPath + "maphelper.sdz");
				}
				break;
			}
		}
	}

	return ret;
}


unsigned int CArchiveScanner::GetArchiveChecksum(const string& name)
{
	string lcname = name;

	// Strip path-info if present
	if (lcname.find_last_of('\\') != string::npos) {
		lcname = lcname.substr(lcname.find_last_of('\\') + 1);
	}
	if (lcname.find_last_of('/') != string::npos) {
		lcname = lcname.substr(lcname.find_last_of('/') + 1);
	}

	StringToLowerInPlace(lcname);

	std::map<string, ArchiveInfo>::iterator aii = archiveInfo.find(lcname);
	if (aii == archiveInfo.end()) {
		return 0;
	}

	return aii->second.checksum;
}


string CArchiveScanner::GetArchivePath(const string& name)
{
	string lcname = name;

	// Strip path-info if present
	if (lcname.find_last_of('\\') != string::npos) {
		lcname = lcname.substr(lcname.find_last_of('\\') + 1);
	}
	if (lcname.find_last_of('/') != string::npos) {
		lcname = lcname.substr(lcname.find_last_of('/') + 1);
	}

	StringToLowerInPlace(lcname);

	std::map<string, ArchiveInfo>::iterator aii = archiveInfo.find(lcname);
	if (aii == archiveInfo.end()) {
		return 0;
	}

	return aii->second.path;
}


/** Get checksum of all required archives depending on selected mod. */
unsigned int CArchiveScanner::GetModChecksum(const string& root)
{
	unsigned int checksum = 0;
	vector<string> ars = GetArchives(root);

	for (vector<string>::iterator i = ars.begin(); i != ars.end(); ++i) {
		unsigned tmp = GetArchiveChecksum(*i);
		logOutput.Print("mod checksum %s: %u/%d", i->c_str(), tmp, (int)tmp);
		checksum ^= tmp;
	}
	return checksum;
}


/** Get checksum of all required archives depending on selected map. */
unsigned int CArchiveScanner::GetMapChecksum(const string& mapName)
{
	unsigned int checksum = 0;
	vector<string> ars = GetArchivesForMap(mapName);

	for (vector<string>::iterator i = ars.begin(); i != ars.end(); ++i) {
		unsigned tmp = GetArchiveChecksum(*i);
		logOutput.Print("map checksum %s: %u/%d", i->c_str(), tmp, (int)tmp);
		checksum ^= tmp;
	}
	return checksum;
}


/** Check if calculated mod checksum equals given checksum. Throws content_error if not equal. */
void CArchiveScanner::CheckMod(const string& root, unsigned checksum)
{
	unsigned localChecksum = GetModChecksum(root);
	if (localChecksum != checksum) {
		char msg[1024];
		sprintf(
			msg,
			"Your mod (checksum 0x%x) differs from the host's mod (checksum 0x%x).\n"
			"This may be caused by a missing archive, a corrupted download, or there may even\n"
			"be 2 different versions in circulation. Make sure you and the host have installed\n"
			"the chosen mod and its dependencies and consider redownloading the mod.\n",
			localChecksum, checksum);

		throw content_error(msg);
	}
}


/** Check if calculated map checksum equals given checksum. Throws content_error if not equal. */
void CArchiveScanner::CheckMap(const string& mapName, unsigned checksum)
{
	unsigned localChecksum = GetMapChecksum(mapName);
	if (localChecksum != checksum) {
		char msg[1024];
		sprintf(
			msg,
			"Your map (checksum 0x%x) differs from the host's map (checksum 0x%x).\n"
			"This may be caused by a missing archive, a corrupted download, or there may even\n"
			"be 2 different versions in circulation. Make sure you and the host have installed\n"
			"the chosen map and its dependencies and consider redownloading the mod.\n",
			localChecksum, checksum);

		throw content_error(msg);
	}
}


/** Convert mod name to mod primary archive, e.g. ModNameToModArchive("XTA v8.1") returns "xtape.sd7". */
string CArchiveScanner::ModNameToModArchive(const string& s) const
{
	// Convert mod name to mod archive
	vector<ModData> found = GetAllMods();
	for (vector<ModData>::iterator it = found.begin(); it != found.end(); ++it) {
		if (it->name == s) {
			return it->dependencies.front();
		}
	}
	return s;
}


/** The reverse of ModNameToModArchive() */
string CArchiveScanner::ModArchiveToModName(const string& s) const
{
	// Convert mod archive to mod name
	vector<ModData> found = GetAllMods();
	for (vector<ModData>::iterator it = found.begin(); it != found.end(); ++it) {
		if (it->dependencies.front() == s) {
			return it->name;
		}
	}
	return s;
}


/** Convert mod name to mod data struct, can return empty ModData */
CArchiveScanner::ModData CArchiveScanner::ModNameToModData(const string& s) const
{
	// Convert mod name to mod archive
	vector<ModData> found = GetAllMods();
	for (vector<ModData>::iterator it = found.begin(); it != found.end(); ++it) {
		const ModData& md = *it;
		if (md.name == s) {
			return md;
		}
	}
	return ModData();
}


/** Convert mod archive to mod data struct, can return empty ModData */
CArchiveScanner::ModData CArchiveScanner::ModArchiveToModData(const string& s) const
{
	// Convert mod archive to mod name
	vector<ModData> found = GetAllMods();
	for (vector<ModData>::iterator it = found.begin(); it != found.end(); ++it) {
		const ModData& md = *it;
		if (md.dependencies.front() == s) {
			return md;
		}
	}
	return ModData();
}
