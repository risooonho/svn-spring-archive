#include "StdAfx.h"
#include "mmgr.h"

#include "FeatureHandler.h"
#include "Feature.h"
#include "Game/Camera.h"
#include "Game/Game.h"
#include "LoadSaveInterface.h"
#include "LogOutput.h"
#include "Sim/Misc/LosHandler.h"
#include "Sim/Misc/QuadField.h"
#include "Lua/LuaParser.h"
#include "Lua/LuaRules.h"
#include "Map/Ground.h"
#include "Map/ReadMap.h"
#include "myMath.h"
#include "Rendering/Env/BaseTreeDrawer.h"
#include "Rendering/Env/BaseWater.h"
#include "Rendering/FartextureHandler.h"
#include "Rendering/GL/myGL.h"
#include "Rendering/GL/VertexArray.h"
#include "Rendering/ShadowHandler.h"
#include "Rendering/UnitModels/3DOParser.h"
#include "Rendering/UnitModels/UnitDrawer.h"
#include "Sim/Misc/CollisionVolume.h"
#include "Sim/Units/UnitHandler.h"
#include "Sim/Units/CommandAI/BuilderCAI.h"
#include "GlobalUnsynced.h"
#include "EventHandler.h"
#include "TimeProfiler.h"
#include "ConfigHandler.h"
#include "creg/STL_List.h"
#include "creg/STL_Set.h"
#include "Exceptions.h"

using namespace std;


CFeatureHandler* featureHandler = NULL;


/******************************************************************************/

CR_BIND(FeatureDef, );

CR_REG_METADATA(FeatureDef, (
		CR_MEMBER(myName),
		CR_MEMBER(description),
		CR_MEMBER(metal),
		CR_MEMBER(id),
		CR_MEMBER(energy),
		CR_MEMBER(maxHealth),
		CR_MEMBER(reclaimTime),
		CR_MEMBER(mass),
		CR_MEMBER(upright),
		CR_MEMBER(drawType),
		//CR_MEMBER(model), FIXME
		CR_MEMBER(modelname),
		CR_MEMBER(modelType),
		CR_MEMBER(resurrectable),
		CR_MEMBER(destructable),
		CR_MEMBER(blocking),
		CR_MEMBER(burnable),
		CR_MEMBER(floating),
		CR_MEMBER(geoThermal),
		CR_MEMBER(deathFeature),
		CR_MEMBER(smokeTime),
		CR_MEMBER(xsize),
		CR_MEMBER(zsize)
		));


CR_BIND_DERIVED(CFeatureHandler,CObject, );

CR_REG_METADATA(CFeatureHandler, (

//	CR_MEMBER(featureDefs),
//	CR_MEMBER(featureDefsVector),

	CR_MEMBER(nextFreeID),
	CR_MEMBER(freeIDs),
	CR_MEMBER(toBeFreedIDs),
	CR_MEMBER(activeFeatures),

	CR_MEMBER(toBeRemoved),
	CR_MEMBER(updateFeatures),

//	CR_MEMBER(drawQuads),
//	CR_MEMBER(drawQuadsX),
//	CR_MEMBER(drawQuadsY),

	CR_RESERVED(128),
	CR_SERIALIZER(Serialize),
	CR_POSTLOAD(PostLoad)
));

CR_BIND(CFeatureHandler::DrawQuad, );

CR_REG_METADATA_SUB(CFeatureHandler,DrawQuad,(
	CR_MEMBER(features)
));


/******************************************************************************/

CFeatureHandler::CFeatureHandler() : nextFreeID(0)
{
	PrintLoadMsg("Loading feature definitions");

	drawQuadsX = gs->mapx/DRAW_QUAD_SIZE;
	drawQuadsY = gs->mapy/DRAW_QUAD_SIZE;
	drawQuads.resize(drawQuadsX * drawQuadsY);

	treeDrawer = CBaseTreeDrawer::GetTreeDrawer();

	const LuaTable rootTable = game->defsParser->GetRoot().SubTable("FeatureDefs");
	if (!rootTable.IsValid()) {
		throw content_error("Error loading FeatureDefs");
	}

	// get most of the feature defs (missing trees and geovent from the map)
	vector<string> keys;
	rootTable.GetKeys(keys);
	for (int i = 0; i < (int)keys.size(); i++) {
		const string& name = keys[i];
		const LuaTable fdTable = rootTable.SubTable(name);
		CreateFeatureDef(fdTable, name);
	}
}


CFeatureHandler::~CFeatureHandler()
{
	for (CFeatureSet::iterator fi = activeFeatures.begin(); fi != activeFeatures.end(); ++fi) {
		// unsavory, but better than a memleak
		FeatureDef* fd = (FeatureDef*) (*fi)->def;

		if (fd->collisionVolume) {
			delete fd->collisionVolume;
			fd->collisionVolume = 0;
		}

		delete *fi;
	}

	activeFeatures.clear();

	while (!featureDefs.empty()) {
		std::map<std::string, const FeatureDef*>::iterator fi = featureDefs.begin();

		FeatureDef* fd = (FeatureDef*) fi->second;

		if (fd->collisionVolume) {
			delete fd->collisionVolume;
			fd->collisionVolume = 0;
		}

		delete fi->second;
		featureDefs.erase(fi);
	}

	delete treeDrawer;
}

void CFeatureHandler::Serialize(creg::ISerializer *s)
{
}

void CFeatureHandler::PostLoad()
{
	drawQuadsX = gs->mapx/DRAW_QUAD_SIZE;
	drawQuadsY = gs->mapy/DRAW_QUAD_SIZE;
	drawQuads.clear();
	drawQuads.resize(drawQuadsX * drawQuadsY);

	for (CFeatureSet::const_iterator it = activeFeatures.begin(); it != activeFeatures.end(); ++it)
		if ((*it)->drawQuad >= 0)
			drawQuads[(*it)->drawQuad].features.insert(*it);
}

void CFeatureHandler::AddFeatureDef(const std::string& name, FeatureDef* fd)
{
	std::map<std::string, const FeatureDef*>::const_iterator it = featureDefs.find(name);

	if (it != featureDefs.end()) {
		featureDefsVector[it->second->id] = fd;
	} else {
		fd->id = featureDefsVector.size();
		featureDefsVector.push_back(fd);
	}
	featureDefs[name] = fd;
}


const FeatureDef* CFeatureHandler::CreateFeatureDef(const LuaTable& fdTable,
                                                    const string& mixedCase)
{
	const string name = StringToLower(mixedCase);
	std::map<std::string, const FeatureDef*>::iterator fi = featureDefs.find(name);

	if (fi != featureDefs.end()) {
		return fi->second;
	}

	FeatureDef* fd = SAFE_NEW FeatureDef;

	fd->myName = name;

	fd->filename = fdTable.GetString("filename", "unknown");

	fd->description = fdTable.GetString("description", "");

	fd->blocking      =  fdTable.GetBool("blocking",       true);
	fd->burnable      =  fdTable.GetBool("flammable",      false);
	fd->destructable  = !fdTable.GetBool("indestructible", false);
	fd->reclaimable   =  fdTable.GetBool("reclaimable",    fd->destructable);
	fd->resurrectable =  fdTable.GetInt("resurrectable",   -1);

	//this seem to be the closest thing to floating that ta wreckage contains
	fd->floating = fdTable.GetBool("nodrawundergray", true);
	if (fd->floating && !fd->blocking) {
		fd->floating = false;
	}

	fd->noSelect = fdTable.GetBool("noselect", false);

	fd->deathFeature = fdTable.GetString("featureDead", "");

	fd->metal       = fdTable.GetFloat("metal",  0.0f);
	fd->energy      = fdTable.GetFloat("energy", 0.0f);
	fd->maxHealth   = fdTable.GetFloat("damage", 0.0f);
	fd->reclaimTime = fdTable.GetFloat("reclaimTime", (fd->metal + fd->energy));

	fd->smokeTime = fdTable.GetInt("smokeTime", 300);

	fd->drawType = DRAWTYPE_3DO;
	fd->modelname = fdTable.GetString("object", "");
	if (!fd->modelname.empty()) {
		if (fd->modelname.find(".") == std::string::npos) {
			fd->modelname += ".3do";
		}
		fd->modelname=string("objects3d/") + fd->modelname;
	}


	/*
	fd->collisionSphereScale = fdTable.GetFloat("collisionSphereScale", 1.0f);
	fd->collisionSphereOffset = fdTable.GetFloat3("collisionSphereOffset", ZeroVector);
	fd->useCSOffset = (fd->collisionSphereOffset != ZeroVector);
	*/

	// these take precedence over the old sphere tags as well as
	// feature->radius (for feature <--> projectile interactions)
	fd->collisionVolumeType    = fdTable.GetString("collisionVolumeType", "");
	fd->collisionVolumeScales  = fdTable.GetFloat3("collisionVolumeScales", ZeroVector);
	fd->collisionVolumeOffsets = fdTable.GetFloat3("collisionVolumeOffsets", ZeroVector);
	fd->collisionVolumeTest    = fdTable.GetInt("collisionVolumeTest", COLVOL_TEST_CONT);

	// initialize the (per-featuredef) collision-volume,
	// all CFeature instances hold a copy of this object
	fd->collisionVolume = SAFE_NEW CollisionVolume(fd->collisionVolumeType,
	fd->collisionVolumeScales, fd->collisionVolumeOffsets, fd->collisionVolumeTest);

	fd->upright = fdTable.GetBool("upright", false);

	// our resolution is double TA's
	fd->xsize = fdTable.GetInt("footprintX", 1) * 2;
	fd->zsize = fdTable.GetInt("footprintZ", 1) * 2;

	const float defMass = (fd->metal * 0.4f) + (fd->maxHealth * 0.1f);
	fd->mass = fdTable.GetFloat("mass", defMass);
	fd->mass = max(0.001f, fd->mass);

	// custom parameters table
	fdTable.SubTable("customParams").GetMap(fd->customParams);

	AddFeatureDef(name, fd);

	fi = featureDefs.find(name);

	return fi->second;
}


const FeatureDef* CFeatureHandler::GetFeatureDef(const std::string mixedCase)
{
	if (mixedCase.empty())
		return NULL;

	const string name = StringToLower(mixedCase);
	std::map<std::string, const FeatureDef*>::iterator fi = featureDefs.find(name);

	if (fi != featureDefs.end()) {
		return fi->second;
	}

	logOutput.Print("Couldnt find wreckage info %s", name.c_str());

	return NULL;
}


const FeatureDef* CFeatureHandler::GetFeatureDefByID(int id)
{
	if ((id < 0) || (id >= (int) featureDefsVector.size())) {
		return NULL;
	}
	return featureDefsVector[id];
}


void CFeatureHandler::LoadFeaturesFromMap(bool onlyCreateDefs)
{
	PrintLoadMsg("Initializing map features");

	int numType = readmap->GetNumFeatureTypes ();

	for (int a = 0; a < numType; ++a) {
		const string name = StringToLower(readmap->GetFeatureType(a));

		if (name.find("treetype") != string::npos) {
			FeatureDef* fd = SAFE_NEW FeatureDef;
			fd->blocking = 1;
			fd->burnable = true;
			fd->destructable = 1;
			fd->reclaimable = true;
			fd->drawType = DRAWTYPE_TREE;
			fd->modelType = atoi(name.substr(8).c_str());
			fd->energy = 250;
			fd->metal = 0;
			fd->reclaimTime = 250;
			fd->maxHealth = 5;
			fd->xsize = 2;
			fd->zsize = 2;
			fd->myName = name;
			fd->description = "Tree";
			fd->mass = 20;
			// trees by default have spherical collision volumes of fixed radius <TREE_RADIUS>
			fd->collisionVolume = SAFE_NEW CollisionVolume("", ZeroVector, ZeroVector, COLVOL_TEST_DISC);
			AddFeatureDef(name, fd);
		}
		else if (name.find("geovent") != string::npos) {
			FeatureDef* fd = SAFE_NEW FeatureDef;
			fd->blocking = 0;
			fd->burnable = 0;
			fd->destructable = 0;
			fd->reclaimable = false;
			fd->geoThermal = true;
			// geos are drawn into the ground texture and emit smoke to be visible
			fd->drawType = DRAWTYPE_NONE;
			fd->modelType = 0;
			fd->energy = 0;
			fd->metal = 0;
			fd->reclaimTime = 0;
			fd->maxHealth = 0;
			fd->xsize = 0;
			fd->zsize = 0;
			fd->myName = name;
			fd->mass = 100000;
			// geothermals have no collision volume at all
			fd->collisionVolume = 0;
			AddFeatureDef(name, fd);
		}
		else {
			if (GetFeatureDef(name) == NULL) {
				logOutput.Print("Unknown map feature type %s", name.c_str());
			}
		}
	}

	if (!onlyCreateDefs) {
		const int numFeatures = readmap->GetNumFeatures();
		MapFeatureInfo* mfi = SAFE_NEW MapFeatureInfo[numFeatures];
		readmap->GetFeatureInfo(mfi);

		for(int a = 0; a < numFeatures; ++a) {
			const string name = StringToLower(readmap->GetFeatureType(mfi[a].featureType));
			std::map<std::string, const FeatureDef*>::iterator def = featureDefs.find(name);

			if (def == featureDefs.end()) {
				logOutput.Print("Unknown feature named '%s'", name.c_str());
				continue;
			}

			const float ypos = ground->GetHeight2(mfi[a].pos.x, mfi[a].pos.z);
			(SAFE_NEW CFeature)->Initialize (float3(mfi[a].pos.x, ypos, mfi[a].pos.z),
			                                 featureDefs[name], (short int)mfi[a].rotation,
			                                 0, -1, "");
		}
		delete[] mfi;
	}
}


int CFeatureHandler::AddFeature(CFeature* feature)
{
	GML_RECMUTEX_LOCK(feat); // AddFeature

	ASSERT_SYNCED_MODE;

	// FIXME -- randomize me, pretty please
	//          (could be done in blocks, if (empty) { add 5000 freeIDs } ?)
	if (freeIDs.empty()) {
		feature->id = nextFreeID++;
	} else {
		feature->id = freeIDs.front();
		freeIDs.pop_front();
	}
	activeFeatures.insert(feature);
	SetFeatureUpdateable(feature);

	if (feature->def->drawType == DRAWTYPE_3DO) {
		int quad = int(feature->pos.z / DRAW_QUAD_SIZE / SQUARE_SIZE) * drawQuadsX +
		           int(feature->pos.x / DRAW_QUAD_SIZE / SQUARE_SIZE);
		DrawQuad* dq = &drawQuads[quad];
		dq->features.insert(feature);
		feature->drawQuad = quad;
	}

	eventHandler.FeatureCreated(feature);

	return feature->id ;
}


void CFeatureHandler::DeleteFeature(CFeature* feature)
{
	GML_RECMUTEX_LOCK(feat); // DeleteFeature, maybe superfluous

	ASSERT_SYNCED_MODE;
	toBeRemoved.push_back(feature->id);

	eventHandler.FeatureDestroyed(feature);
}


CFeature* CFeatureHandler::CreateWreckage(const float3& pos, const std::string& name,
	float rot, int facing, int iter, int team, int allyteam, bool emitSmoke, std::string fromUnit,
	const float3& speed)
{
	ASSERT_SYNCED_MODE;
	const FeatureDef* fd;
	const std::string* defname = &name;

	int i = iter;
	do {
		if (name.empty()) return NULL;
		fd = GetFeatureDef(*defname);
		if (!fd) return NULL;
		defname = &(fd->deathFeature);
	}while (--i > 0);

	if (luaRules && !luaRules->AllowFeatureCreation(fd, team, pos))
		return NULL;

	if (!fd->modelname.empty()) {
		if (fd->resurrectable==0 || (iter>1 && fd->resurrectable<0))
			fromUnit = "";

		CFeature* f = SAFE_NEW CFeature;
		f->Initialize(pos, fd, (short int) rot, facing, team, fromUnit, speed);

		// allow area-reclaiming wrecks of all units, including your own (they set allyteam = -1)
		f->allyteam = allyteam;
		if (emitSmoke && fd->blocking)
			f->emitSmokeTime = fd->smokeTime;

		return f;
	}
	return NULL;
}



void CFeatureHandler::Update()
{
	ASSERT_SYNCED_MODE;
	SCOPED_TIMER("Feature::Update");

	if ((gs->frameNum & 31) == 0) {
		// let all areareclaimers choose a target with a different id
		bool dontClear = false;
		for (std::list<int>::iterator it = toBeFreedIDs.begin(); it != toBeFreedIDs.end(); ++it) {
			if (CBuilderCAI::IsFeatureBeingReclaimed(*it)) {
				// postpone recycling
				dontClear = true;
				break;
			}
		}
		if (!dontClear)
			freeIDs.splice(freeIDs.end(), toBeFreedIDs, toBeFreedIDs.begin(), toBeFreedIDs.end());
	}

	if(!toBeRemoved.empty()) {
		GML_RECMUTEX_LOCK(feat); // Update
		GML_RECMUTEX_LOCK(quad); // Update
		
		while (!toBeRemoved.empty()) {
			CFeatureSet::iterator it = activeFeatures.find(toBeRemoved.back());
			toBeRemoved.pop_back();
			if (it != activeFeatures.end()) {
				CFeature* feature = *it;
				toBeFreedIDs.push_back(feature->id);
				activeFeatures.erase(feature);
				
				if (feature->drawQuad >= 0) {
					DrawQuad* dq = &drawQuads[feature->drawQuad];
					dq->features.erase(feature);
				}
				
				if (feature->inUpdateQue) {
					updateFeatures.erase(feature);
				}
				
				delete feature;
			}
		}
	}

	CFeatureSet::iterator fi = updateFeatures.begin();
	while (fi != updateFeatures.end()) {
		CFeature* feature = *fi;
		++fi;

		if (!feature->Update()) {
			// remove it
			feature->inUpdateQue = false;
			updateFeatures.erase(feature);
		}
	}
}


void CFeatureHandler::UpdateDrawQuad(CFeature* feature, const float3& newPos)
{
	GML_RECMUTEX_LOCK(feat); // UpdateDrawQuad

	const int oldDrawQuad = feature->drawQuad;
	if (oldDrawQuad >= 0) {
		const int newDrawQuad =
			int(newPos.z / DRAW_QUAD_SIZE / SQUARE_SIZE) * drawQuadsX +
			int(newPos.x / DRAW_QUAD_SIZE / SQUARE_SIZE);
		if (oldDrawQuad != newDrawQuad) {
			DrawQuad* oldDQ = &drawQuads[oldDrawQuad];
			oldDQ->features.erase(feature);
			DrawQuad* newDQ = &drawQuads[newDrawQuad];
			newDQ->features.insert(feature);
			feature->drawQuad = newDrawQuad;
		}
	}
}


void CFeatureHandler::SetFeatureUpdateable(CFeature* feature)
{
	if (feature->inUpdateQue) {
		return;
	}
	updateFeatures.insert(feature);
	feature->inUpdateQue = true;
}


void CFeatureHandler::TerrainChanged(int x1, int y1, int x2, int y2)
{
	ASSERT_SYNCED_MODE;
	std::vector<int> quads = qf->GetQuadsRectangle(float3(x1 * SQUARE_SIZE, 0, y1 * SQUARE_SIZE),
		float3(x2 * SQUARE_SIZE, 0, y2 * SQUARE_SIZE));

	for (std::vector<int>::iterator qi = quads.begin(); qi != quads.end(); ++qi) {
		std::list<CFeature*>::const_iterator fi;
		const list<CFeature*>& features = qf->GetQuad(*qi).features;

		for (fi = features.begin(); fi != features.end(); ++fi) {
			CFeature* feature = *fi;
			float3& fpos = feature->pos;
			if (fpos.y > ground->GetHeight(fpos.x, fpos.z)) {
				SetFeatureUpdateable(feature);

				if (feature->def->floating){
					feature->finalHeight = ground->GetHeight(fpos.x, fpos.z);
				} else {
					feature->finalHeight = ground->GetHeight2(fpos.x, fpos.z);
				}

				feature->CalculateTransform ();
			}
		}
	}
}


void CFeatureHandler::Draw()
{
	ASSERT_UNSYNCED_MODE;
	drawFar.clear();

	GML_RECMUTEX_LOCK(feat); // Draw

	unitDrawer->SetupForUnitDrawing();
	unitDrawer->SetupFor3DO();
	DrawRaw(0, &drawFar);
	unitDrawer->CleanUp3DO();
	unitDrawer->DrawQuedS3O();
	unitDrawer->CleanUpUnitDrawing();

	if (drawFar.size()>0) {
		CVertexArray* va = GetVertexArray();
		va->Initialize();
		va->EnlargeArrays(drawFar.size()*4,0,VA_SIZE_TN);
		glAlphaFunc(GL_GREATER, 0.8f);
		glEnable(GL_ALPHA_TEST);
		glBindTexture(GL_TEXTURE_2D, fartextureHandler->GetTextureID());
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_FOG);
		for (vector<CFeature*>::iterator usi = drawFar.begin(); usi != drawFar.end(); usi++) {
			DrawFar(*usi, va);
		}
		va->DrawArrayTN(GL_QUADS);
	}
}


void CFeatureHandler::DrawShadowPass()
{
	ASSERT_UNSYNCED_MODE;
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, unitDrawer->unitShadowGenVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glPolygonOffset(1,1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	GML_RECMUTEX_LOCK(feat); // DrawShadowPass

	DrawRaw(1, NULL);
	unitDrawer->DrawQuedS3O();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable( GL_VERTEX_PROGRAM_ARB );
}

class CFeatureDrawer : public CReadMap::IQuadDrawer
{
public:
	void DrawQuad(int x,int y);

	CFeatureHandler* fh;
	std::vector<CFeatureHandler::DrawQuad>* drawQuads;
	int drawQuadsX;
	bool drawReflection, drawRefraction;
	float unitDrawDist;
	std::vector<CFeature*>* farFeatures;
};


void CFeatureDrawer::DrawQuad(int x, int y)
{
	CFeatureHandler::DrawQuad* dq = &(*drawQuads)[y * drawQuadsX + x];

	for (CFeatureSet::iterator fi = dq->features.begin(); fi != dq->features.end(); ++fi) {
		CFeature* f = (*fi);
		const FeatureDef* def = f->def;

		if ((f->allyteam == -1 || f->allyteam == gu->myAllyTeam || gu->spectatingFullView ||
			loshandler->InLos(f->pos, gu->myAllyTeam)) && def->drawType == DRAWTYPE_3DO) {

			if (drawReflection) {
				float3 zeroPos;
				if (f->midPos.y < 0) {
					zeroPos = f->midPos;
				} else {
					float dif = f->midPos.y - camera->pos.y;
					zeroPos = camera->pos * (f->midPos.y / dif) + f->midPos * (-camera->pos.y / dif);
				}
				if (ground->GetApproximateHeight(zeroPos.x, zeroPos.z) > f->radius) {
					continue;
				}
			}
			if (drawRefraction) {
				if (f->pos.y > 0)
					continue;
			}

			float sqDist = (f->pos - camera->pos).SqLength2D();
			float farLength = f->sqRadius * unitDrawDist * unitDrawDist;

			if (sqDist<farLength) {
				if (f->model->type==MODELTYPE_3DO) {
					unitDrawer->DrawFeatureStatic(f);
				} else {
					unitDrawer->QueS3ODraw(f, f->model->textureType);
				}
			} else {
				if (farFeatures)
					farFeatures->push_back(f);
			}
		}
	}
}


void CFeatureHandler::DrawRaw(int extraSize, std::vector<CFeature*>* farFeatures)
{
	float featureDist = 3000;
	if (!extraSize) {
		featureDist = 6000; //farfeatures wont be drawn for shadowpass anyway
	}
	CFeatureDrawer drawer;
	drawer.drawQuads = &drawQuads;
	drawer.fh = this;
	drawer.drawQuadsX = drawQuadsX;
	drawer.drawReflection=water->drawReflection;
	drawer.drawRefraction=water->drawRefraction;
	drawer.unitDrawDist=unitDrawer->unitDrawDist;
	drawer.farFeatures = farFeatures;

	readmap->GridVisibility(camera, DRAW_QUAD_SIZE, featureDist, &drawer, extraSize);
}


void CFeatureHandler::DrawFar(CFeature* feature, CVertexArray* va)
{
	float3 interPos=feature->pos+UpVector*feature->model->height*0.5f;
	int snurr=-feature->heading+GetHeadingFromVector(camera->pos.x-feature->pos.x,camera->pos.z-feature->pos.z)+(0xffff>>4);
	if(snurr<0)
		snurr+=0xffff;
	if(snurr>0xffff)
		snurr-=0xffff;
	snurr=snurr>>13;
	float tx=(feature->model->farTextureNum%8)*(1.0f/8.0f)+snurr*(1.0f/64);
	float ty=(feature->model->farTextureNum/8)*(1.0f/64.0f);
	float offset=0;

	float3 curad=camera->up*feature->radius*1.4f;
	float3 crrad=camera->right*feature->radius;
	va->AddVertexQTN(interPos-(curad-offset)+crrad,tx,ty,unitDrawer->camNorm);
	va->AddVertexQTN(interPos+(curad+offset)+crrad,tx,ty+(1.0f/64.0f),unitDrawer->camNorm);
	va->AddVertexQTN(interPos+(curad+offset)-crrad,tx+(1.0f/64.0f),ty+(1.0f/64.0f),unitDrawer->camNorm);
	va->AddVertexQTN(interPos-(curad-offset)-crrad,tx+(1.0f/64.0f),ty,unitDrawer->camNorm);
}


S3DModel* FeatureDef::LoadModel()
{
	if (model==NULL)
		model = modelParser->Load3DModel(modelname);
	return model;
}
