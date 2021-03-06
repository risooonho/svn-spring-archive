#pragma once
#include "script.h"
#include <string>
#include <list>
#include <vector>
#include "filehandler.h"

using namespace std;
class CSpawnScript :
	public CScript
{
public:
	CSpawnScript(void);
	~CSpawnScript(void);
	void Update(void);

	void LoadSpawns(void);

	struct Spawn {
		int frame;
		string name;
	};

	std::list<Spawn> spawns;
	std::list<Spawn>::iterator curSpawn;
	int frameOffset;

	std::vector<float3> spawnPos;

	struct Unit{
		int id;
		int target;
		float3 lastTargetPos;
	};
	std::list<Unit> myUnits;
	std::list<Unit>::iterator curUnit;
	std::string LoadToken(CFileHandler& file);
};
