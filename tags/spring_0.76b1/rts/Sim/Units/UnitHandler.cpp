// UnitHandler.cpp: implementation of the CUnitHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UnitHandler.h"
#include "Unit.h"
#include "Rendering/GL/myGL.h"
#include "Game/Team.h"
#include "TimeProfiler.h"
#include "myMath.h"
#include "Map/Ground.h"
#include "Map/ReadMap.h"
#include "Platform/ConfigHandler.h"
#include "Rendering/FartextureHandler.h"
#include "UnitDefHandler.h"
#include "Sim/Misc/QuadField.h"
#include "CommandAI/BuilderCAI.h"
#include "Game/SelectedUnits.h"
#include "FileSystem/FileHandler.h"
#include "LogOutput.h"
#include "Game/SelectedUnits.h"
#include "Sim/Misc/Feature.h"
#include "Sim/Misc/FeatureHandler.h"
#include "Sim/Units/Unit.h"
#include "LoadSaveInterface.h"
#include "UnitLoader.h"
#include "Sync/SyncTracer.h"
#include "Game/GameSetup.h"
#include "Sim/Units/CommandAI/Command.h"
#include "Sim/Misc/AirBaseHandler.h"
#include "creg/STL_List.h"
#include "creg/STL_Deque.h"
#include "creg/STL_Set.h"
#include "mmgr.h"

BuildInfo::BuildInfo(const std::string& name, const float3& p, int facing)
{
	def = unitDefHandler->GetUnitByName(name);
	pos = p;
	buildFacing = facing;
}

void BuildInfo::FillCmd(Command& c) const
{
	c.id=-def->id;
	c.params.resize(4);
	c.params[0]=pos.x;
	c.params[1]=pos.y;
	c.params[2]=pos.z;
	c.params[3]=(float)buildFacing;
}

bool BuildInfo::Parse(const Command& c)
{
	if (c.params.size() >= 3) {
		pos = float3(c.params[0],c.params[1],c.params[2]);

		if(c.id < 0) {
			def = unitDefHandler->GetUnitByID(-c.id);

			buildFacing = 0;
			if (c.params.size()==4)
				buildFacing = int(c.params[3]);

			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHandler* uh;
using namespace std;

CR_BIND(CUnitHandler, (true));
CR_REG_METADATA(CUnitHandler, (
	CR_MEMBER(activeUnits),
	CR_MEMBER(units),
	CR_MEMBER(freeIDs),
	CR_MEMBER(waterDamage),
	CR_MEMBER(maxUnits),
	CR_MEMBER(maxUnitRadius),
	CR_MEMBER(lastDamageWarning),
	CR_MEMBER(lastCmdDamageWarning),
	CR_MEMBER(limitDgun),
	CR_MEMBER(dgunRadius),
	CR_MEMBER(diminishingMetalMakers),
	CR_MEMBER(metalMakerIncome),
	CR_MEMBER(metalMakerEfficiency),
	CR_MEMBER(toBeRemoved),
	CR_MEMBER(morphUnitToFeature),
	CR_MEMBER(toBeRemoved),
	CR_MEMBER(builderCAIs),
	CR_MEMBER(unitsByDefs),
	CR_POSTLOAD(PostLoad),
	CR_SERIALIZER(Serialize)
	));

void CUnitHandler::Serialize(creg::ISerializer& s)
{
}

void CUnitHandler::PostLoad()
{
	// reset any synced stuff that is not saved
	slowUpdateIterator = activeUnits.end();
}

CUnitHandler::CUnitHandler(bool serializing)
:
	maxUnits(500),
	maxUnitRadius(0.0f),
	lastDamageWarning(0),
	lastCmdDamageWarning(0),
	metalMakerIncome(0),
	metalMakerEfficiency(1),
	diminishingMetalMakers(false),
	limitDgun(false),
	morphUnitToFeature(true)
{
	//unitModelLoader=SAFE_NEW CUnit3DLoader;
	for (int a = 1; a < MAX_UNITS; a++) {
		freeIDs.push_back(a);
		units[a] = 0;
	}
	units[0] = 0;

	slowUpdateIterator = activeUnits.end();

	waterDamage=atof(readmap->mapDefParser.SGetValueDef("0","MAP\\WATER\\WaterDamage").c_str())*(16.0f/30.0f);

	if (gameSetup) {
		maxUnits = gameSetup->maxUnits;
	}
	if (maxUnits > ((MAX_UNITS / gs->activeTeams) - 5)) {
		maxUnits = (MAX_UNITS / gs->activeTeams) -5;
	}

	if (gameSetup) {
		if (gameSetup->limitDgun) {
			limitDgun = true;
			dgunRadius = gs->mapx * 3;
		}
		if (gameSetup->diminishingMMs) {
			diminishingMetalMakers = true;
		}
	}

	if (!serializing) {
		airBaseHandler = SAFE_NEW CAirBaseHandler;

		for (int i = 0; i < MAX_TEAMS; i++) {
			unitsByDefs[i].resize(unitDefHandler->numUnitDefs + 1);
		}
	}
}


CUnitHandler::~CUnitHandler()
{
	list<CUnit*>::iterator usi;
	for (usi = activeUnits.begin(); usi != activeUnits.end(); usi++) {
		delete (*usi);
	}

	delete airBaseHandler;

}

int CUnitHandler::AddUnit(CUnit *unit)
{
	ASSERT_SYNCED_MODE;
	int num=(int)(gs->randFloat())*((int)activeUnits.size()-1);
	std::list<CUnit*>::iterator ui=activeUnits.begin();
	for(int a=0;a<num;++a){
		++ui;
	}
	activeUnits.insert(ui,unit);		//randomize this to make the order in slowupdate random (good if one build say many buildings at once and then many mobile ones etc)

	unit->id=freeIDs.front();
	freeIDs.pop_front();

	units[unit->id]=unit;
	gs->Team(unit->team)->AddUnit(unit,CTeam::AddBuilt);
	unitsByDefs[unit->team][unit->unitDef->id].insert(unit);

	maxUnitRadius = max(unit->radius, maxUnitRadius);

	return unit->id;
}

void CUnitHandler::DeleteUnit(CUnit* unit)
{
	ASSERT_SYNCED_MODE;
	toBeRemoved.push_back(unit);
}

void CUnitHandler::Update()
{
	ASSERT_SYNCED_MODE;

	SCOPED_TIMER("Unit handler");

	while(!toBeRemoved.empty()){
		CUnit* delUnit=toBeRemoved.back();
		toBeRemoved.pop_back();

		int delTeam = 0;
		int delType = 0;
		list<CUnit*>::iterator usi;
		for(usi=activeUnits.begin();usi!=activeUnits.end();++usi){
			if(*usi==delUnit){
				if (slowUpdateIterator!=activeUnits.end() && *usi==*slowUpdateIterator) {
					slowUpdateIterator++;
				}
				delTeam = delUnit->team;
				delType = delUnit->unitDef->id;

				activeUnits.erase(usi);
				units[delUnit->id] = 0;
				freeIDs.push_back(delUnit->id);
				gs->Team(delTeam)->RemoveUnit(delUnit, CTeam::RemoveDied);

				unitsByDefs[delTeam][delType].erase(delUnit);

				delete delUnit;

				break;
			}
		}
		//debug
		for (usi = activeUnits.begin(); usi != activeUnits.end(); /* no post-op */) {
			if (*usi == delUnit){
				logOutput.Print("Error: Duplicated unit found in active units on erase");
				usi = activeUnits.erase(usi);
			} else {
				++usi;
			}
		}
	}

	list<CUnit*>::iterator usi;
	for (usi = activeUnits.begin(); usi != activeUnits.end(); usi++) {
		(*usi)->Update();
	}

	{
		SCOPED_TIMER("Unit slow update");
		if (!(gs->frameNum & 15)) {
			slowUpdateIterator=activeUnits.begin();
		}

		int numToUpdate=activeUnits.size()/16+1;
		for(;slowUpdateIterator!=activeUnits.end() && numToUpdate!=0;++slowUpdateIterator){
			(*slowUpdateIterator)->SlowUpdate();
			numToUpdate--;
		}
	} // for timer destruction

	if(!(gs->frameNum&15)){
		if(diminishingMetalMakers)
			metalMakerEfficiency=8.0f/(8.0f+max(0.0f,sqrtf(metalMakerIncome/gs->activeTeams)-4));
		metalMakerIncome=0;
	}
}

float CUnitHandler::GetBuildHeight(float3 pos, const UnitDef* unitdef)
{
	float minh=-5000;
	float maxh=5000;
	int numBorder=0;
	float borderh=0;
	float* heightmap=readmap->GetHeightmap();

	int xsize=1;
	int ysize=1;

	float maxDif=unitdef->maxHeightDif;
	int x1 = (int)max(0.f,(pos.x-(xsize*0.5f*SQUARE_SIZE))/SQUARE_SIZE);
	int x2 = min(gs->mapx,x1+xsize);
	int z1 = (int)max(0.f,(pos.z-(ysize*0.5f*SQUARE_SIZE))/SQUARE_SIZE);
	int z2 = min(gs->mapy,z1+ysize);

	if (x1 > gs->mapx) x1 = gs->mapx;
	if (x2 < 0) x2 = 0;
	if (z1 > gs->mapy) z1 = gs->mapy;
	if (z2 < 0) z2 = 0;

	for(int x=x1; x<=x2; x++){
		for(int z=z1; z<=z2; z++){
			float orgh=readmap->orgheightmap[z*(gs->mapx+1)+x];
			float h=heightmap[z*(gs->mapx+1)+x];
			if(x==x1 || x==x2 || z==z1 || z==z2){
				numBorder++;
				borderh+=h;
			}
			if(minh<min(h,orgh)-maxDif)
				minh=min(h,orgh)-maxDif;
			if(maxh>max(h,orgh)+maxDif)
				maxh=max(h,orgh)+maxDif;
		}
	}
	float h=borderh/numBorder;

	if(h<minh && minh<maxh)
		h=minh+0.01f;
	if(h>maxh && maxh>minh)
		h=maxh-0.01f;

	return h;
}

int CUnitHandler::TestUnitBuildSquare(const BuildInfo& buildInfo, CFeature *&feature, int allyteam)
{
	feature=0;
	int xsize=buildInfo.GetXSize();
	int ysize=buildInfo.GetYSize();
	float3 pos=buildInfo.pos;

	int x1 = (int) (pos.x-(xsize*0.5f*SQUARE_SIZE));
	int x2 = x1+xsize*SQUARE_SIZE;
	int z1 = (int) (pos.z-(ysize*0.5f*SQUARE_SIZE));
	int z2 = z1+ysize*SQUARE_SIZE;
	float h=GetBuildHeight(pos,buildInfo.def);

	int canBuild=2;

	if(buildInfo.def->needGeo){
		canBuild=0;
		std::vector<CFeature*> features=qf->GetFeaturesExact(pos,max(xsize,ysize)*6);

		for(std::vector<CFeature*>::iterator fi=features.begin();fi!=features.end();++fi){
			if((*fi)->def->geoThermal && fabs((*fi)->pos.x-pos.x)<xsize*4-4 && fabs((*fi)->pos.z-pos.z)<ysize*4-4){
				canBuild=2;
				break;
			}
		}
	}

	for(int x=x1; x<x2; x+=SQUARE_SIZE){
		for(int z=z1; z<z2; z+=SQUARE_SIZE){
			int tbs=TestBuildSquare(float3(x,h,z),buildInfo.def,feature,allyteam);
			canBuild=min(canBuild,tbs);
			if(canBuild==0)
				return 0;
		}
	}

	return canBuild;
}

int CUnitHandler::TestBuildSquare(const float3& pos, const UnitDef *unitdef, CFeature *&feature, int allyteam)
{
	int ret=2;
	if(pos.x<0 || pos.x>=gs->mapx*SQUARE_SIZE || pos.z<0 || pos.z>=gs->mapy*SQUARE_SIZE)
		return 0;

	int yardxpos=int(pos.x+4)/SQUARE_SIZE;
	int yardypos=int(pos.z+4)/SQUARE_SIZE;
	CSolidObject* s;
	if ((s = readmap->GroundBlocked(yardypos*gs->mapx+yardxpos))) {
		if (dynamic_cast<CFeature*>(s)) {
			feature = (CFeature*)s;
		}
		else if (!dynamic_cast<CUnit*>(s) || (allyteam < 0) ||
		         (((CUnit*)s)->losStatus[allyteam] & LOS_INLOS)) {
			if (s->immobile) {
				return 0;
			} else {
				ret = 1;
			}
		}
	}

	if(!unitdef->floater)
	{
		float* heightmap=readmap->GetHeightmap();
		int x=(int) (pos.x/SQUARE_SIZE);
		int z=(int) (pos.z/SQUARE_SIZE);
		float orgh=readmap->orgheightmap[z*(gs->mapx+1)+x];
		float h=heightmap[z*(gs->mapx+1)+x];
		float hdif=unitdef->maxHeightDif;
		if(pos.y>orgh+hdif && pos.y>h+hdif)
			return 0;
		if(pos.y<orgh-hdif && pos.y<h-hdif)
			return 0;
	}
	float groundheight = ground->GetHeight2(pos.x,pos.z);
	if(!unitdef->floater && groundheight<-unitdef->maxWaterDepth)
		return 0;
	if(groundheight>-unitdef->minWaterDepth)
		return 0;

	return ret;
}

int CUnitHandler::ShowUnitBuildSquare(const BuildInfo& buildInfo)
{
	return ShowUnitBuildSquare(buildInfo, std::vector<Command>());
}

int CUnitHandler::ShowUnitBuildSquare(const BuildInfo& buildInfo, const std::vector<Command> &cv)
{
	glDisable(GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);

	int xsize=buildInfo.GetXSize();
	int ysize=buildInfo.GetYSize();
	const float3& pos = buildInfo.pos;

	int x1 = (int) (pos.x-(xsize*0.5f*SQUARE_SIZE));
	int x2 = x1+xsize*SQUARE_SIZE;
	int z1 = (int) (pos.z-(ysize*0.5f*SQUARE_SIZE));
	int z2 = z1+ysize*SQUARE_SIZE;
	float h=GetBuildHeight(pos,buildInfo.def);

	int canbuild=2;

	if(buildInfo.def->needGeo)
	{
		canbuild=0;
		std::vector<CFeature*> features=qf->GetFeaturesExact(pos,max(xsize,ysize)*6);

		for(std::vector<CFeature*>::iterator fi=features.begin();fi!=features.end();++fi){
			if((*fi)->def->geoThermal && fabs((*fi)->pos.x-pos.x)<xsize*4-4 && fabs((*fi)->pos.z-pos.z)<ysize*4-4){
				canbuild=2;
				break;
			}
		}
	}
	std::vector<float3> canbuildpos;
	std::vector<float3> featurepos;
	std::vector<float3> nobuildpos;

	for(int x=x1; x<x2; x+=SQUARE_SIZE){
		for(int z=z1; z<z2; z+=SQUARE_SIZE){

			CFeature* feature=0;
			int tbs=TestBuildSquare(float3(x,pos.y,z),buildInfo.def,feature,gu->myAllyTeam);
			if(tbs){
				std::vector<Command>::const_iterator ci = cv.begin();
				for(;ci != cv.end() && tbs; ci++){
					BuildInfo bc(*ci);
					if(max(bc.pos.x-x-SQUARE_SIZE,x-bc.pos.x)*2 < bc.GetXSize()*SQUARE_SIZE
						&& max(bc.pos.z-z-SQUARE_SIZE,z-bc.pos.z)*2 < bc.GetYSize()*SQUARE_SIZE){
						tbs=0;
					}
				}
				if(!tbs){
					nobuildpos.push_back(float3(x,h,z));
					canbuild = 0;
				} else if(feature || tbs==1)
					featurepos.push_back(float3(x,h,z));
				else
					canbuildpos.push_back(float3(x,h,z));
				canbuild=min(canbuild,tbs);
			} else {
				nobuildpos.push_back(float3(x,h,z));
				//glColor4f(0.8f,0.0f,0,0.4f);
				canbuild = 0;
			}
		}
	}

	if(canbuild)
		glColor4f(0,0.8f,0,1.0f);
	else
		glColor4f(0.5f,0.5f,0,1.0f);

	for(unsigned int i=0; i<canbuildpos.size(); i++)
	{
		glVertexf3(canbuildpos[i]);
		glVertexf3(canbuildpos[i]+float3(SQUARE_SIZE,0,0));
		glVertexf3(canbuildpos[i]+float3(SQUARE_SIZE,0,SQUARE_SIZE));
		glVertexf3(canbuildpos[i]+float3(0,0,SQUARE_SIZE));
	}
	glColor4f(0.5f,0.5f,0,1.0f);
	for(unsigned int i=0; i<featurepos.size(); i++)
	{
		glVertexf3(featurepos[i]);
		glVertexf3(featurepos[i]+float3(SQUARE_SIZE,0,0));
		glVertexf3(featurepos[i]+float3(SQUARE_SIZE,0,SQUARE_SIZE));
		glVertexf3(featurepos[i]+float3(0,0,SQUARE_SIZE));
	}

	glColor4f(0.8f,0.0f,0,1.0f);
	for(unsigned int i=0; i<nobuildpos.size(); i++)
	{
		glVertexf3(nobuildpos[i]);
		glVertexf3(nobuildpos[i]+float3(SQUARE_SIZE,0,0));
		glVertexf3(nobuildpos[i]+float3(SQUARE_SIZE,0,SQUARE_SIZE));
		glVertexf3(nobuildpos[i]+float3(0,0,SQUARE_SIZE));
	}

	glEnd();

	if (h < 0.0f) {
		const float s[4] = { 0.0f, 0.0f, 1.0f, 0.5f }; // start color
		const float e[4] = { 0.0f, 0.5f, 1.0f, 1.0f }; // end color

		glBegin(GL_LINES);
		glColor4fv(s); glVertex3f(x1, h, z1); glColor4fv(e); glVertex3f(x1, 0.0f, z1);
		glColor4fv(s); glVertex3f(x2, h, z1); glColor4fv(e); glVertex3f(x2, 0.0f, z1);
		glColor4fv(s); glVertex3f(x1, h, z2); glColor4fv(e); glVertex3f(x1, 0.0f, z2);
		glColor4fv(s); glVertex3f(x2, h, z2); glColor4fv(e); glVertex3f(x2, 0.0f, z2);
		glEnd();
		// using the last end color
		glBegin(GL_LINE_LOOP);
		glVertex3f(x1, 0.0f, z1);
		glVertex3f(x1, 0.0f, z2);
		glVertex3f(x2, 0.0f, z2);
		glVertex3f(x2, 0.0f, z1);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_BLEND);

	return canbuild;
}

void CUnitHandler::PushNewWind(float x, float z, float strength)
{
	ASSERT_SYNCED_MODE;
	//todo: fixa en lista med enbart windgenerators kanske blir lite snabbare
	list<CUnit*>::iterator usi;
	for(usi=activeUnits.begin();usi!=activeUnits.end();usi++)
	{
		if((*usi)->unitDef->windGenerator)
			(*usi)->PushWind(x,z,strength);
	}
}

void CUnitHandler::AddBuilderCAI(CBuilderCAI* b)
{
	builderCAIs.insert(builderCAIs.end(),b);
}

void CUnitHandler::RemoveBuilderCAI(CBuilderCAI* b)
{
	ListErase<CBuilderCAI*>(builderCAIs, b);
}

void CUnitHandler::LoadSaveUnits(CLoadSaveInterface* file, bool loading)
{
/*	for(int a=0;a<MAX_UNITS;++a){
		bool exists=!!units[a];
		file->lsBool(exists);
		if(exists){
			if(loading){
				overrideId=a;
				float3 pos;
				file->lsFloat3(pos);
				string name;
				file->lsString(name);
				int team;
				file->lsInt(team);
				bool build;
				file->lsBool(build);
				unitLoader.LoadUnit(name,pos,team,build);
			} else {
				file->lsFloat3(units[a]->pos);
				file->lsString(units[a]->unitDef->name);
				file->lsInt(units[a]->team);
				file->lsBool(units[a]->beingBuilt);
			}
		} else {
			if(loading)
				freeIDs.push_back(a);
		}
	}
	for(int a=0;a<MAX_UNITS;++a){
		if(units[a])
			units[a]->LoadSave(file,loading);
	}
	overrideId=-1;*/
}

bool CUnitHandler::CanCloseYard(CUnit* unit)
{
	for(int z=unit->mapPos.y;z<unit->mapPos.y+unit->ysize;++z){
		for(int x=unit->mapPos.x;x<unit->mapPos.x+unit->xsize;++x){
			CSolidObject* c=readmap->groundBlockingObjectMap[z*gs->mapx+x];
			if(c!=0 && c!=unit)
				return false;
		}
	}
	return true;
}

/**
* returns a build Command that intersects the ray described by pos and dir from the command queues of the
* units units on team number team
* @breif returns a build Command that intersects the ray described by pos and dir
* @return the build Command, or 0 if one is not found
*/

Command CUnitHandler::GetBuildCommand(float3 pos, float3 dir){
	float3 tempF1 = pos;
	std::list<CUnit*>::iterator ui = this->activeUnits.begin();
	CCommandQueue::iterator ci;
	for(; ui != this->activeUnits.end(); ui++){
		if((*ui)->team == gu->myTeam){
			ci = (*ui)->commandAI->commandQue.begin();
			for(; ci != (*ui)->commandAI->commandQue.end(); ci++){
				if((*ci).id < 0 && (*ci).params.size() >= 3){
					BuildInfo bi(*ci);
					tempF1 = pos + dir*((bi.pos.y - pos.y)/dir.y) - bi.pos;
					if(bi.def && bi.GetXSize()/2*SQUARE_SIZE > fabs(tempF1.x) && bi.GetYSize()/2*SQUARE_SIZE > fabs(tempF1.z)){
						return (*ci);
					}
				}
			}
		}
	}
	Command c;
	c.id = 0;
	return c;
}

bool CUnitHandler::CanBuildUnit(const UnitDef* unitdef, int team)
{
	if (gs->Team(team)->units.size() >= uh->maxUnits) {
		return false;
	}
	if (unitsByDefs[team][unitdef->id].size() >= unitdef->maxThisUnit) {
		return false;
	}

	return true;
}
