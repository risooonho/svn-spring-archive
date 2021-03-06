#pragma once

#include "aidef.h"

class AAIBuildTable;
class AAISector;
class AAI;

class AAIMap
{
public:
	AAIMap(AAI *ai);
	~AAIMap(void);

	void Init();
	
	// sets cells of the builmap to value
	bool SetBuildMap(int xPos, int yPos, int xSize, int ySize, int value, int ignore_value = -1);

	// converts map-pos to unit-pos and vice versa
	void Pos2BuildMapPos(float3 *pos, const UnitDef* def);
	void BuildMapPos2Pos(float3 *pos, const UnitDef* def);
	void Pos2FinalBuildPos(float3 *pos, const UnitDef *def);

	// true if x/y are a valid sector
	bool ValidSector(int x, int y);

	// returns sector (0 if out of sector map -> e.g. aircraft flying outside of the map) of a position
	AAISector* GetSectorOfPos(float3 *pos);
	
	// returns buildsites for normal and defence buildings
	float3 GetHighestBuildsite(const UnitDef *def, int xStart, int xEnd, int yStart, int yEnd);
	float3 GetCenterBuildsite(const UnitDef *def, int xStart, int xEnd, int yStart, int yEnd, bool water = false);
	float3 GetRandomBuildsite(const UnitDef *def, int xStart, int xEnd, int yStart, int yEnd, int tries, bool water = false);
	float3 GetBuildSiteInRect(const UnitDef *def, int xStart, int xEnd, int yStart, int yEnd, bool water = false);

	// return rating of a the best buidliste fpr a def. building vs category within specified rect (and stores pos in pointer)
	float GetDefenceBuildsite(float3 *best_pos, const UnitDef *def, int xStart, int xEnd, int yStart, int yEnd, UnitCategory category,  float terrain_modifier, bool water);
	
	float3 GetClosestBuildsite(const UnitDef *def, float3 pos, int max_distance, bool water);

	// returns footprint size of a building
	void GetSize(const UnitDef *def, int *xSize, int *ySize);

	// prevents ai from building too many buildings in a row
	void CheckRows(int xPos, int yPos, int xSize, int ySize, bool add, bool water);

	// adds or removes blocked cells (to prevent aai from packing buildings too close to each other)
	void BlockCells(int xPos, int yPos, int width, int height, bool block, bool water);

	// returns number of cells with big slope
	int GetCliffyCells(int xPos, int yPos, int xSize, int ySize);
	int GetCliffyCellsInSector(AAISector *sector);

	// updates spotted ennemy/ally buildings/units on the map
	void UpdateRecon();

	void UpdateSectors();

	// increases/decreases usefulness of the category of the killer/killed unit 
	void UpdateCategoryUsefulness(const UnitDef *killer_def, int killer, const UnitDef *killed_def, int killed);

	char* GetMapTypeTextString(int mapType);
	char* GetMapTypeString(int mapType);

	// return next cell in direction with a certain value
	int GetNextX(int direction, int xPos, int yPos, int value);	// 0 means left, other right; returns -1 if not found 
	int GetNextY(int direction, int xPos, int yPos, int value);	// 0 means up, other down; returns -1 if not found
	
	// returns true if buildmap allows construction
	bool CanBuildAt(int xPos, int yPos, int xSize, int ySize, bool water = false);

	// reads map cahce file (and creates new one if necessary)
	void ReadCacheFile();

	// if auto_set == true, the loaded values are assigned to the current sectordata as well 
	void ReadMapLearnFile(bool auto_set);

	// loads mex spots, cliffs etc. from file or creates new one
	void GetMapData();

	// calculates learning effect
	void Learn();

	// get water, high slopes, defence map
	void AnalyseMap();

	// adds/removes a defence buidling to the defence map (air == true -> add to air defence map)
	void AddDefence(float3 *pos, int defence);
	void RemoveDefence(float3 *pos, int defence);

	void SearchMetalSpots();


public:
	vector<vector<AAISector> > sector;			// sectors

	bool initialized;

	// defence maps
	vector<float> defence_map;	// defence map has 1/2 of resolution of blockmap/buildmap
	vector<float> air_defence_map; // air defence map has 1/2 of resolution of blockmap/buildmap

	// temp for scouting
	vector<float> units_spotted;

private:
	AAI *ai;
	IAICallback *cb;
	AAIBuildTable *bt;

	// used for scouting
	vector<int> unitsInLos;
	vector<int> unitsInSector;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// static (shared with other ai players)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	static int aai_instances;	// how many aai instances have been initialized

	static int xMapSize, yMapSize;				// x and y size of the map
	static int xDefMapSize, yDefMapSize;		// x and y size of the defence maps (1/2 resolution of map)
	static int xSectors, ySectors;				// number of sectors
	static int xSectorSize, ySectorSize;		// size of sectors (in unit pos coordinates)
	static int xSectorSizeMap, ySectorSizeMap;	// size of sectors (in map coodrinates = 1/8 xSize)

	static list<AAIMetalSpot> metal_spots;

	static bool metalMap;
	static MapType mapType;	// 0 -> unknown ,1-> land map (default), 2 -> air map, 
							// 3 -> water map with land connections 
							// 4 -> "full water map

	static vector< vector<int> > team_sector_map;	// stores the number of ai player which has taken that sector (-1 if none)
											// this helps preventing aai from expanding into sectors of other aai players

	
	static vector<int> buildmap;	// map of the cells in the sector; 
							// 0 unoccupied, flat terrain 
							// 1 occupied flat terrain, 
							// 2 spaces between buildings 
							// 3 terrain not suitable for constr.
							// 4 water
							// 5 occupied water
	static vector<int> blockmap;		// number of buildings which ordered a cell to blocked
	static vector<float> plateau_map;	// positive values indicate plateaus, 1/2 of resolution of blockmap/buildmap

	static list<UnitCategory> map_categories;
	static list<int> map_categories_id;
	static vector<vector<float> > map_usefulness;
};
