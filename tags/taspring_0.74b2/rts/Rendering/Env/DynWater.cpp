#include "StdAfx.h"
#include "DynWater.h"
#include "Game/Game.h"
//#include <windows.h>
#include "Rendering/GL/myGL.h"
#include "Game/Camera.h"
#include "Rendering/GL/VertexArray.h"
#include "Map/ReadMap.h"
#include "LogOutput.h"
#include "Map/BaseGroundDrawer.h"
#include "Rendering/Env/BaseSky.h"
#include "Rendering/UnitModels/UnitDrawer.h"
#include "Sim/Projectiles/ProjectileHandler.h"
#include "Sim/Misc/FeatureHandler.h"
#include "Game/UI/MouseHandler.h"
#include "Game/GameHelper.h"
#include "Rendering/ShadowHandler.h"
#include "Rendering/Textures/Bitmap.h"
#include "Sim/Units/UnitHandler.h"
#include "Sim/Units/Unit.h"
#include "Sim/Units/UnitDef.h"

extern GLfloat FogLand[]; 

#define W_SIZE 5
#define WF_SIZE 5120
#define WH_SIZE 2560
/*/
#define W_SIZE 4
#define WF_SIZE 4096
#define WH_SIZE 2048
/**/
CDynWater::CDynWater(void)
{
	lastWaveFrame=0;
	noWakeProjectiles=true;
	firstDraw=true;
	drawSolid=true;
	camPosBig=float3(2048,0,2048);
	refractSize=gu->viewSizeY>=1024 ? 1024:512;

	glGenTextures(1, &reflectTexture);
	glBindTexture(GL_TEXTURE_2D, reflectTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,512, 512, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenTextures(1, &refractTexture);
	glBindTexture(GL_TEXTURE_2D, refractTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,refractSize, refractSize, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenTextures(1, &detailNormalTex);
	glBindTexture(GL_TEXTURE_2D, detailNormalTex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,256, 256, 0,GL_RGBA, GL_FLOAT, 0);
	glGenerateMipmapEXT(GL_TEXTURE_2D);

	float* temp=SAFE_NEW float[1024*1024*4];

	for(int y=0;y<64;++y){
		for(int x=0;x<64;++x){
			temp[(y*64+x)*4+0]=(sin(x*PI*2.0f/64.0f)) + (x<32 ? -1:1)*0.3f;
			temp[(y*64+x)*4+1]=temp[(y*64+x)*4+0];
			temp[(y*64+x)*4+2]=(cos(x*PI*2.0f/64.0f)) + (x<32 ? 16-x:x-48)/16.0f*0.3f;
			temp[(y*64+x)*4+3]=0;
		}
	}
	glGenTextures(3, rawBumpTexture);
	glBindTexture(GL_TEXTURE_2D, rawBumpTexture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,64, 64, 0,GL_RGBA, GL_FLOAT, temp);

	unsigned char* scrap=SAFE_NEW unsigned char[256*256*4];
	CBitmap foam;
	if (!foam.Load("bitmaps/foam.jpg") || foam.xsize != 256 || foam.ysize != 256)
		throw content_error("Could not load foam from file bitmaps/foam.jpg");
	for(int a=0;a<256*256;++a)
		scrap[a]=foam.mem[a*4];

	glGenTextures(1, &foamTex);
	glBindTexture(GL_TEXTURE_2D, foamTex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE,256, 256, GL_LUMINANCE, GL_UNSIGNED_BYTE, scrap);

	delete[] scrap;

	if(ProgramStringIsNative(GL_VERTEX_PROGRAM_ARB,"waterDyn.vp"))
		waterVP=LoadVertexProgram("waterDyn.vp");
	else
		waterVP=LoadVertexProgram("waterDynNT.vp");

	waterFP=LoadFragmentProgram("waterDyn.fp");
	waveFP=LoadFragmentProgram("waterDynWave.fp");
	waveVP=LoadVertexProgram("waterDynWave.vp");
	waveFP2=LoadFragmentProgram("waterDynWave2.fp");
	waveVP2=LoadVertexProgram("waterDynWave2.vp");
	waveNormalFP=LoadFragmentProgram("waterDynNormal.fp");
	waveNormalVP=LoadVertexProgram("waterDynNormal.vp");
	waveCopyHeightFP=LoadFragmentProgram("waterDynWave3.fp");
	waveCopyHeightVP=LoadVertexProgram("waterDynWave3.vp");
	dwGroundRefractVP=LoadVertexProgram("dwgroundrefract.vp");
	dwGroundReflectIVP=LoadVertexProgram("dwgroundreflectinverted.vp");
	dwDetailNormalFP=LoadFragmentProgram("dwDetailNormal.fp");
	dwDetailNormalVP=LoadVertexProgram("dwDetailNormal.vp");
	dwAddSplashFP=LoadFragmentProgram("dwAddSplash.fp");
	dwAddSplashVP=LoadVertexProgram("dwAddSplash.vp");

	waterSurfaceColor = readmap->waterSurfaceColor;

	for(int y=0;y<1024;++y){
		for(int x=0;x<1024;++x){
			temp[(y*1024+x)*4+0]=0;
			temp[(y*1024+x)*4+1]=0;
			temp[(y*1024+x)*4+2]=0;
			temp[(y*1024+x)*4+3]=0;
		}
	}
	glGenTextures(1, &waveHeight32);
	glBindTexture(GL_TEXTURE_2D, waveHeight32);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F_ARB ,256, 256, 0,GL_RGBA, GL_FLOAT, temp);

	glGenTextures(1, &waveTex1);
	glBindTexture(GL_TEXTURE_2D, waveTex1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,1024, 1024, 0,GL_RGBA, GL_FLOAT, temp);

	for(int y=0;y<1024;++y){
		for(int x=0;x<1024;++x){
			float dist=(x-500)*(x-500)+(y-450)*(y-450);
			temp[(y*1024+x)*4+0]=0;//max(0.0f,15-sqrt(dist));//sin(y*PI*2.0f/64.0f)*0.5f+0.5f;
			temp[(y*1024+x)*4+1]=0;
			temp[(y*1024+x)*4+2]=0;
			temp[(y*1024+x)*4+3]=0;
		}
	}
	glGenTextures(1, &waveTex2);
	glBindTexture(GL_TEXTURE_2D, waveTex2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,1024, 1024, 0,GL_RGBA, GL_FLOAT, temp);

	for(int y=0;y<1024;++y){
		for(int x=0;x<1024;++x){
			float dist=(x-500)*(x-500)+(y-450)*(y-450);
			temp[(y*1024+x)*4+0]=0;
			temp[(y*1024+x)*4+1]=1;
			temp[(y*1024+x)*4+2]=0;
			temp[(y*1024+x)*4+3]=0;
		}
	}
	glGenTextures(1, &waveTex3);
	glBindTexture(GL_TEXTURE_2D, waveTex3);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,1024, 1024, 0,GL_RGBA, GL_FLOAT, temp);

	for(int y=0;y<64;++y){
		float dy=y-31.5f;
		for(int x=0;x<64;++x){
			float dx=x-31.5f;
			float dist=sqrt(dx*dx+dy*dy);
			temp[(y*64+x)*4+0]=max(0.0f,1-dist/30.f)*max(0.0f,1-dist/30.f);
			temp[(y*64+x)*4+1]=max(0.0f,1-dist/30.f);
			temp[(y*64+x)*4+2]=max(0.0f,1-dist/30.f)*max(0.0f,1-dist/30.f);
			temp[(y*64+x)*4+3]=0;
		}
	}

	glGenTextures(1, &splashTex);
	glBindTexture(GL_TEXTURE_2D, splashTex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR/*_MIPMAP_NEAREST*/);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	//gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA32F_ARB ,64, 64, GL_RGBA, GL_FLOAT, temp);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F_ARB ,64, 64, 0,GL_RGBA, GL_FLOAT, temp);

	unsigned char temp2[]={0,0,0,0};
	glGenTextures(1, &zeroTex);
	glBindTexture(GL_TEXTURE_2D, zeroTex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR/*_MIPMAP_NEAREST*/);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,1, 1, 0,GL_RGBA, GL_UNSIGNED_BYTE, temp2);

	unsigned char temp3[]={0,255,0,0};
	glGenTextures(1, &fixedUpTex);
	glBindTexture(GL_TEXTURE_2D, fixedUpTex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR/*_MIPMAP_NEAREST*/);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,1, 1, 0,GL_RGBA, GL_UNSIGNED_BYTE, temp3);

	CBitmap bm;
	if (!bm.Load("bitmaps/boatshape.bmp"))
		throw content_error("Could not load boatshape from file bitmaps/boatshape.bmp");
	for(int a=0;a<bm.xsize*bm.ysize;++a){
		bm.mem[a*4+2]=bm.mem[a*4];
		bm.mem[a*4+3]=bm.mem[a*4];
	}
	boatShape=bm.CreateTexture();

	CBitmap bm2;
	if (!bm.Load("bitmaps/hovershape.bmp"))
		throw content_error("Could not load hovershape from file bitmaps/hovershape.bmp");
	for(int a=0;a<bm2.xsize*bm2.ysize;++a){
		bm2.mem[a*4+2]=bm2.mem[a*4];
		bm2.mem[a*4+3]=bm2.mem[a*4];
	}
	hoverShape=bm2.CreateTexture();

	delete[] temp;
	glGenFramebuffersEXT(1,&frameBuffer);
}

CDynWater::~CDynWater(void)
{
	glDeleteTextures (1, &reflectTexture);
	glDeleteTextures (1, &refractTexture);
	glDeleteTextures (3, rawBumpTexture);
	glDeleteTextures (1, &detailNormalTex);
	glDeleteTextures (1, &waveTex1);
	glDeleteTextures (1, &waveTex2);
	glDeleteTextures (1, &waveTex3);
	glDeleteTextures (1, &waveHeight32);
	glDeleteTextures (1, &splashTex);
	glDeleteTextures (1, &foamTex);
	glDeleteTextures (1, &boatShape);
	glDeleteTextures (1, &hoverShape);
	glDeleteTextures (1, &zeroTex);
	glDeleteTextures (1, &fixedUpTex);
	glSafeDeleteProgram( waterFP );
	glSafeDeleteProgram( waterVP );
	glSafeDeleteProgram( waveFP );
	glSafeDeleteProgram( waveVP );
	glSafeDeleteProgram( waveFP2 );
	glSafeDeleteProgram( waveVP2 );
	glSafeDeleteProgram( waveNormalFP );
	glSafeDeleteProgram( waveNormalVP );
	glSafeDeleteProgram( waveCopyHeightFP );
	glSafeDeleteProgram( waveCopyHeightVP );
	glSafeDeleteProgram( dwGroundReflectIVP );
	glSafeDeleteProgram( dwGroundRefractVP );
	glSafeDeleteProgram( dwDetailNormalVP );
	glSafeDeleteProgram( dwDetailNormalFP );
	glSafeDeleteProgram( dwAddSplashVP );
	glSafeDeleteProgram( dwAddSplashFP );
	glDeleteFramebuffersEXT(1,&frameBuffer);
}

void CDynWater::Draw()
{
	if(readmap->minheight>10)
		return;

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_FOG);

	glBindTexture(GL_TEXTURE_2D, waveTex3);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,reflectTexture);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,waveHeight32);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,refractTexture);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,readmap->GetShadingTexture ());
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,foamTex);
	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D,detailNormalTex);
	glActiveTextureARB(GL_TEXTURE7_ARB);
	glBindTexture(GL_TEXTURE_2D,shadowHandler->shadowTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glColor4f(1,1,1,0.5f);
	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, waterFP );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, waterVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );

	float dx=float(gu->viewSizeX)/gu->viewSizeY*tan(camera->fov/180/2*PI);
	float dy=float(gu->viewSizeY)/gu->viewSizeY*tan(camera->fov/180/2*PI);

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 10, 1.0f/(W_SIZE*256),1.0f/(W_SIZE*256), 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 11, -camPosX/256.0f+0.5f,-camPosZ/256.0f+0.5f, 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 12, 1.0f/WF_SIZE,1.0f/WF_SIZE, 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 13, -(camPosBig.x-WH_SIZE)/WF_SIZE,-(camPosBig.z-WH_SIZE)/WF_SIZE, 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 14, 1.0f/(gs->pwr2mapx*SQUARE_SIZE),1.0f/(gs->pwr2mapy*SQUARE_SIZE), 0, 0);
	//	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0, 1.0f/4096.0f,1.0f/4096.0f,0,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1, camera->pos.x,camera->pos.y,camera->pos.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,2, reflectRight.x,reflectRight.y,reflectRight.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,3, reflectUp.x,reflectUp.y,reflectUp.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,4, 0.5f/dx,0.5f/dy,1,1);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,5, reflectForward.x,reflectForward.y,reflectForward.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,6, 0.05f, 1-0.05f, 0, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,7, 0.2f, 0, 0, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,8, 0.5f, 0.6f, 0.8f, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,9, gs->sunVector.x, gs->sunVector.y, gs->sunVector.z, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,10, readmap->sunColor.x, readmap->sunColor.y, readmap->sunColor.z, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,11, readmap->ambientColor.x, readmap->ambientColor.y, readmap->ambientColor.z, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,12, refractRight.x,refractRight.y,refractRight.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,13, refractUp.x,refractUp.y,refractUp.z,0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,14, 0.5f/dx,0.5f/dy,1,1);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,15, refractForward.x,refractForward.y,refractForward.z,0);

	DrawWaterSurface();

	glBindTexture(GL_TEXTURE_2D, fixedUpTex);

	DrawOuterSurface();

	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	glDisable( GL_VERTEX_PROGRAM_ARB );

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE7_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void CDynWater::UpdateWater(CGame* game)
{
	if(readmap->minheight>10)
		return;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(0);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	DrawHeightTex();

	if(firstDraw){
		firstDraw=false;
		DrawDetailNormalTex();
	}
	glEnable(GL_DEPTH_TEST);
	glDepthMask(1);

	CBaseGroundDrawer *gd = readmap->GetGroundDrawer();
	DrawRefraction(game);
	DrawReflection(game);
}

void CDynWater::Update()
{
	if(readmap->minheight>10)
		return;

	oldCamPosBig=camPosBig;

	camPosBig.x=floor(max((float)WH_SIZE, min((float)gs->mapx*SQUARE_SIZE-WH_SIZE, (float)camera->pos.x))/(W_SIZE*16))*(W_SIZE*16);
	camPosBig.z=floor(max((float)WH_SIZE, min((float)gs->mapy*SQUARE_SIZE-WH_SIZE, (float)camera->pos.z))/(W_SIZE*16))*(W_SIZE*16);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(0);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	/*	if(mouse->buttons[0].pressed){
	float3 pos=camera->pos+mouse->dir*(-camera->pos.y/mouse->dir.y);
	AddSplash(pos,20,1);
	}*/
	AddShipWakes();
	AddExplosions();
	DrawDetailNormalTex();
	DrawWaves();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(1);
}

void CDynWater::DrawReflection(CGame* game)
{
	CCamera realCam=*camera;

	camera->up.x=0;
	camera->up.y=1;
	camera->up.z=0;
	camera->forward.y*=-1;
	camera->pos.y*=-1;
	camera->pos.y+=0.2f;
	camera->Update(false);
	reflectRight=camera->right;
	reflectUp=camera->up;
	reflectForward=camera->forward;

	glViewport(0,0,512,512);

	glClearColor(0.5f,0.6f,0.8f,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sky->Draw();

	glEnable(GL_CLIP_PLANE2);
	double plane2[4]={0,-1,0,1.0f};
	glClipPlane(GL_CLIP_PLANE2 ,plane2);
	drawReflection=true;
	bool drawShadows=shadowHandler->drawShadows;
	shadowHandler->drawShadows=false;

	CBaseGroundDrawer *gd = readmap->GetGroundDrawer ();
	gd->Draw(true,false,dwGroundReflectIVP);

	double plane[4]={0,1,0,1.0f};
	glClipPlane(GL_CLIP_PLANE2 ,plane);

	gd->Draw(true);

	shadowHandler->drawShadows=drawShadows;

	unitDrawer->Draw(true);
	featureHandler->Draw();
	ph->Draw(true);

	sky->DrawSun();

	drawReflection=false;
	glDisable(GL_CLIP_PLANE2);

	glColorMask(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, reflectTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,512,512);

	glViewport(gu->viewPosX,0,gu->viewSizeX,gu->viewSizeY);
	glClearColor(FogLand[0],FogLand[1],FogLand[2],1);

	*camera=realCam;
	camera->Update(false);
}

void CDynWater::DrawRefraction(CGame* game)
{
	drawRefraction=true;
	camera->Update(false);

	refractRight=camera->right;
	refractUp=camera->up;
	refractForward=camera->forward;

	glViewport(0,0,refractSize,refractSize);

	glClearColor(FogLand[0],FogLand[1],FogLand[2],1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float3 oldsun=unitDrawer->unitSunColor;
	float3 oldambient=unitDrawer->unitAmbientColor;

	unitDrawer->unitSunColor*=float3(0.5f,0.7f,0.9f);
	unitDrawer->unitAmbientColor*=float3(0.6f,0.8f,1.0f);

	CBaseGroundDrawer *gd = readmap->GetGroundDrawer();
	gd->Draw(false,false,dwGroundRefractVP);

	glEnable(GL_CLIP_PLANE2);
	double plane[4]={0,-1,0,2};
	glClipPlane(GL_CLIP_PLANE2 ,plane);
	drawReflection=true;
	unitDrawer->Draw(false,true);
	featureHandler->Draw();
	drawReflection=false;
	ph->Draw(false,true);
	glDisable(GL_CLIP_PLANE2);

	drawRefraction=false;

	glBindTexture(GL_TEXTURE_2D, refractTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,refractSize,refractSize);

	glViewport(gu->viewPosX,0,gu->viewSizeX,gu->viewSizeY);
	glClearColor(FogLand[0],FogLand[1],FogLand[2],1);

	unitDrawer->unitSunColor=oldsun;
	unitDrawer->unitAmbientColor=oldambient;
}

void CDynWater::DrawWaves(void)
{
	float dx=camPosBig.x-oldCamPosBig.x;
	float dy=camPosBig.z-oldCamPosBig.z;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE0_ARB);


	GLenum status;
	float start=0.1f/1024;
	float end=1023.9f/1024;

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,8,  -1.0f/1024, 1.0f/1024, 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,9,  0, 1.0f/1024, 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,10, 1.0f/1024, 1.0f/1024, 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,11, 1.0f/1024, 0, 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,12, float(WF_SIZE)/(gs->pwr2mapx*SQUARE_SIZE), float(WF_SIZE)/(gs->pwr2mapy*SQUARE_SIZE), 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,13, (camPosBig.x-WH_SIZE)/(gs->pwr2mapx*SQUARE_SIZE), (camPosBig.z-WH_SIZE)/(gs->pwr2mapy*SQUARE_SIZE), 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,14, dx/WF_SIZE, dy/WF_SIZE, 0,0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,15, (camPosBig.x-WH_SIZE)/WF_SIZE*4, (camPosBig.x-WH_SIZE)/WF_SIZE*4, 0,0);

	//////////////////////////////////////

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveTex3, 0);

	glViewport(0,0,1024,1024);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready2");

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D, readmap->GetShadingTexture ());
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, waveFP2 );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, waveVP2 );
	glEnable( GL_VERTEX_PROGRAM_ARB );

	//update flows pass
	int resetTexs[]={0,1,2,3,4,5,-1};
	DrawUpdateSquare(dx,dy,resetTexs);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glFlush();


	///////////////////////////////////////

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveTex2, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready1");


	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex3);
	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D,detailNormalTex);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, waveFP );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, waveVP );

	//update height pass
	int resetTexs2[]={0,-1};
	DrawUpdateSquare(dx,dy,resetTexs2);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glFlush();

	////////////////////////////////

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex2);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex2);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex2);
	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveTex1, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready3");

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, waveNormalFP );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, waveNormalVP );

	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0, 0, 0, W_SIZE*2, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1, W_SIZE*2, 0, 0, 0);

	//update normals pass
	glBegin(GL_QUADS);
	glTexCoord2f(start,start);glVertex3f(0,0,0);
	glTexCoord2f(start,end);glVertex3f(0,1,0);
	glTexCoord2f(end,end);glVertex3f(1,1,0);
	glTexCoord2f(end,start);glVertex3f(1,0,0);
	glEnd();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glFlush();

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	unsigned int temp=waveTex1;
	waveTex1=waveTex2;
	waveTex2=waveTex3;
	waveTex3=temp;
}

void CDynWater::DrawHeightTex(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveHeight32, 0);

	glViewport(0,0,256,256);

	int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready4");

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, waveCopyHeightFP );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, waveCopyHeightVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );

	camPosX=int(camera->pos.x/W_SIZE);
	camPosZ=int(camera->pos.z/W_SIZE);

	float startx=(camPosX-120)/1024.0f-(camPosBig.x-WH_SIZE)/WF_SIZE;
	float startz=(camPosZ-120)/1024.0f-(camPosBig.z-WH_SIZE)/WF_SIZE;
	float endx=(camPosX+120)/1024.0f-(camPosBig.x-WH_SIZE)/WF_SIZE;
	float endz=(camPosZ+120)/1024.0f-(camPosBig.z-WH_SIZE)/WF_SIZE;
	float startv=8.0f/256;
	float endv=248.0f/256;
	//update 32 bit height map
	glBegin(GL_QUADS);
	glTexCoord2f(startx,startz);glVertex3f(startv,startv,0);
	glTexCoord2f(startx,endz);glVertex3f(startv,endv,0);
	glTexCoord2f(endx,endz);glVertex3f(endv,endv,0);
	glTexCoord2f(endx,startz);glVertex3f(endv,startv,0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D,waveTex1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	glDisable( GL_VERTEX_PROGRAM_ARB );
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glFlush();
}

void CDynWater::AddFrustumRestraint(float3 side)
{
	fline temp;
	float3 up(0,1,0);

	float3 b=up.cross(side);		//get vector for collision between frustum and horizontal plane
	if(fabs(b.z)<0.0001f)
		b.z=0.0001f;
	{
		temp.dir=b.x/b.z;				//set direction to that
		float3 c=b.cross(side);			//get vector from camera to collision line
		float3 colpoint;				//a point on the collision line

		if(side.y>0)								
			colpoint=cam2->pos-c*((cam2->pos.y-(-10))/c.y);
		else
			colpoint=cam2->pos-c*((cam2->pos.y-(10))/c.y);


		temp.base=colpoint.x-colpoint.z*temp.dir;	//get intersection between colpoint and z axis
		if(b.z>0){
			left.push_back(temp);			
		}else{
			right.push_back(temp);
		}
	}	
}

void CDynWater::UpdateCamRestraints(void)
{
	left.clear();
	right.clear();

	//Add restraints for camera sides
	AddFrustumRestraint(cam2->bottom);
	AddFrustumRestraint(cam2->top);
	AddFrustumRestraint(cam2->rightside);
	AddFrustumRestraint(cam2->leftside);

	//Add restraint for maximum view distance
	fline temp;
	float3 up(0,1,0);
	float3 side=cam2->forward;
	float3 camHorizontal=cam2->forward;
	camHorizontal.y=0;
	camHorizontal.Normalize();
	float3 b=up.cross(camHorizontal);			//get vector for collision between frustum and horizontal plane
	if(fabs(b.z)>0.0001f){
		temp.dir=b.x/b.z;				//set direction to that
		float3 c=b.cross(camHorizontal);			//get vector from camera to collision line
		float3 colpoint;				//a point on the collision line

		if(side.y>0)								
			colpoint=cam2->pos+camHorizontal*gu->viewRange*1.05f-c*(cam2->pos.y/c.y);
		else
			colpoint=cam2->pos+camHorizontal*gu->viewRange*1.05f-c*((cam2->pos.y-255/3.5f)/c.y);


		temp.base=colpoint.x-colpoint.z*temp.dir;	//get intersection between colpoint and z axis
		if(b.z>0){
			left.push_back(temp);			
		}else{
			right.push_back(temp);
		}
	}
}

#define WSQUARE_SIZE W_SIZE

static CVertexArray* va;
static inline void DrawVertexA(int x,int y)
{
	va->AddVertex0(float3(x*WSQUARE_SIZE,0,y*WSQUARE_SIZE));
}

void CDynWater::DrawWaterSurface(void)
{
	int viewRadius=40;
	bool inStrip=false;

	va=GetVertexArray();
	va->Initialize();

	camPosBig2.x=floor(max((float)WH_SIZE, min((float)gs->mapx*SQUARE_SIZE-WH_SIZE, (float)camera->pos.x))/(W_SIZE*16))*(W_SIZE*16);
	camPosBig2.z=floor(max((float)WH_SIZE, min((float)gs->mapy*SQUARE_SIZE-WH_SIZE, (float)camera->pos.z))/(W_SIZE*16))*(W_SIZE*16);

	for(int lod=1;lod<(2<<5);lod*=2){
		int cx=(int)(cam2->pos.x/(WSQUARE_SIZE));
		int cy=(int)(cam2->pos.z/(WSQUARE_SIZE));

		cx=(cx/lod)*lod;
		cy=(cy/lod)*lod;
		int hlod=lod>>1;
		int ysquaremod=((cy)%(2*lod))/lod;
		int xsquaremod=((cx)%(2*lod))/lod;

		int minty=int(camPosBig2.z/WSQUARE_SIZE-512);
		int maxty=int(camPosBig2.z/WSQUARE_SIZE+512);
		int mintx=int(camPosBig2.x/WSQUARE_SIZE-512);
		int maxtx=int(camPosBig2.x/WSQUARE_SIZE+512);

		int minly=cy+(-viewRadius+2-ysquaremod)*lod;
		int maxly=cy+(viewRadius-ysquaremod)*lod;
		int minlx=cx+(-viewRadius+2-xsquaremod)*lod;
		int maxlx=cx+(viewRadius-xsquaremod)*lod;

		int xstart=max(minlx,mintx);
		int xend=min(maxlx,maxtx);
		int ystart=max(minly,minty);
		int yend=min(maxly,maxty);

		for(int y=ystart;y<yend;y+=lod){
			int xs=xstart;
			int xe=xend;
			int xtest,xtest2;
			std::vector<fline>::iterator fli;
			for(fli=left.begin();fli!=left.end();fli++){
				xtest=((int)(fli->base/(WSQUARE_SIZE)+fli->dir*y))/lod*lod-lod;
				xtest2=((int)(fli->base/(WSQUARE_SIZE)+fli->dir*(y+lod)))/lod*lod-lod;
				if(xtest>xtest2)
					xtest=xtest2;
				if(xtest>xs)
					xs=xtest;
			}
			for(fli=right.begin();fli!=right.end();fli++){
				xtest=((int)(fli->base/(WSQUARE_SIZE)+fli->dir*y))/lod*lod+lod;
				xtest2=((int)(fli->base/(WSQUARE_SIZE)+fli->dir*(y+lod)))/lod*lod+lod;
				if(xtest<xtest2)
					xtest=xtest2;
				if(xtest<xe)
					xe=xtest;
			}

			for(int x=xs;x<xe;x+=lod){
				if((lod==1) || 
					(x>(cx)+viewRadius*hlod) || (x<(cx)-viewRadius*hlod) ||
					(y>(cy)+viewRadius*hlod) || (y<(cy)-viewRadius*hlod)){  //normal terrain
						if(!inStrip){
							DrawVertexA(x,y);
							DrawVertexA(x,y+lod);
							inStrip=true;
						}
						DrawVertexA(x+lod,y);
						DrawVertexA(x+lod,y+lod);
					} else {  //inre begr�sning mot f�eg�nde lod
						if((x>=(cx)+viewRadius*hlod)){
							if(inStrip){
								va->EndStrip();	
								inStrip=false;
							}
							DrawVertexA(x,y);                                            
							DrawVertexA(x,y+hlod);
							DrawVertexA(x+hlod,y);
							DrawVertexA(x+hlod,y+hlod);
							va->EndStrip();	
							DrawVertexA(x,y+hlod);
							DrawVertexA(x,y+lod);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+hlod,y+lod);
							va->EndStrip();	
							DrawVertexA(x+hlod,y+lod);
							DrawVertexA(x+lod,y+lod);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+lod,y);
							DrawVertexA(x+hlod,y);
							va->EndStrip();	
						}     
						else if((x<=(cx)-viewRadius*hlod)){
							if(inStrip){
								va->EndStrip();	
								inStrip=false;
							}
							DrawVertexA(x+lod,y+hlod);
							DrawVertexA(x+lod,y);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+hlod,y);
							va->EndStrip();	
							DrawVertexA(x+lod,y+lod);
							DrawVertexA(x+lod,y+hlod);
							DrawVertexA(x+hlod,y+lod);
							DrawVertexA(x+hlod,y+hlod);
							va->EndStrip();	
							DrawVertexA(x+hlod,y);
							DrawVertexA(x,y);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x,y+lod);
							DrawVertexA(x+hlod,y+lod);
							va->EndStrip();	
						} 
						else if((y>=(cy)+viewRadius*hlod)){
							if(inStrip){
								va->EndStrip();	
								inStrip=false;
							}
							DrawVertexA(x,y);                                            
							DrawVertexA(x,y+hlod);
							DrawVertexA(x+hlod,y);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+lod,y);
							DrawVertexA(x+lod,y+hlod);
							va->EndStrip();	
							DrawVertexA(x,y+hlod);
							DrawVertexA(x,y+lod);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+lod,y+lod);
							DrawVertexA(x+lod,y+hlod);
							va->EndStrip();	
						}
						else if((y<=(cy)-viewRadius*hlod)){
							if(inStrip){
								va->EndStrip();	
								inStrip=false;
							}
							DrawVertexA(x,y+hlod);
							DrawVertexA(x,y+lod);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x+hlod,y+lod);
							DrawVertexA(x+lod,y+hlod);
							DrawVertexA(x+lod,y+lod);
							va->EndStrip();	
							DrawVertexA(x+lod,y+hlod);
							DrawVertexA(x+lod,y);
							DrawVertexA(x+hlod,y+hlod);
							DrawVertexA(x,y);
							DrawVertexA(x,y+hlod);
							va->EndStrip();	
						}
					}
			}
			if(inStrip){
				va->EndStrip();	
				inStrip=false;
			}
		}
	}
	va->DrawArray0(GL_TRIANGLE_STRIP);
}

void CDynWater::DrawDetailNormalTex(void)
{
	for(int a=0;a<8;++a){
		glActiveTextureARB(GL_TEXTURE0_ARB+a);
		glBindTexture(GL_TEXTURE_2D,rawBumpTexture[0]);
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, detailNormalTex, 0);

	glViewport(0,0,256,256);

	int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready5");

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, dwDetailNormalFP );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, dwDetailNormalVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );

	float swh=0.05f;	//height of detail normal waves
	float lwh=1.0f;	//height of larger ambient waves

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 9, gs->frameNum, 0, 0, 0);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 10, 5, 0, 0, 1.0f/120);			//controls the position and speed of the waves
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 11, 14, 0, 0, 1.0f/90);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 12, 29, 0, 0, 1.0f/55);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 13, 9, 4, 0, 1.0f/100);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 14, -5, 14, 0, 1.0f/90);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 15, 27, 27, 0, 1.0f/75);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 16, -3, -5, 0, 1.0f/100);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 17, -10, 24, 0, 1.0f/60);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0, 0.2f*swh, 0.0f*swh, 0.7f*lwh, 0);		//controls the height of the waves
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1, 0.2f*swh, 0.0f*swh, 0.7f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,2, 0.2f*swh, 0.0f*swh, 0.7f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,3, 0.2f*swh, 0.01f*swh, 0.4f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,4, 0.07f*swh, 0.2f*swh, 0.7f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,5, 0.2f*swh, 0.2f*swh, 0.7f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,6, 0.12f*swh, 0.2f*swh, 0.7f*lwh, 0);
	glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,7, 0.08f*swh, 0.2f*swh, 0.7f*lwh, 0);

	//update detail normals
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);glVertex3f(0,0,0);
	glTexCoord2f(0,1);glVertex3f(0,1,0);
	glTexCoord2f(1,1);glVertex3f(1,1,0);
	glTexCoord2f(1,0);glVertex3f(1,0,0);
	glEnd();


	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	glDisable( GL_VERTEX_PROGRAM_ARB );

	glFlush();

	glBindTexture(GL_TEXTURE_2D,detailNormalTex);
	glGenerateMipmapEXT(GL_TEXTURE_2D);

}

void CDynWater::AddShipWakes()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveTex1, 0);

	glViewport(0,0,1024,1024);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready6");

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, dwAddSplashFP );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, dwAddSplashVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glDisable(GL_CULL_FACE);

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 10, 1.0f/WF_SIZE, 1.0f/WF_SIZE, 0, 1);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 11, -(oldCamPosBig.x-WH_SIZE)/WF_SIZE, -(oldCamPosBig.z-WH_SIZE)/WF_SIZE, 0, 0);

	CVertexArray* va=GetVertexArray();
	va->Initialize();
	CVertexArray* va2=GetVertexArray();		//never try to get more than 2 at once
	va2->Initialize();

	for(std::list<CUnit*>::iterator ui=uh->activeUnits.begin(); ui!=uh->activeUnits.end();++ui){
		CUnit* unit=*ui;
		if(unit->moveType && unit->floatOnWater && unit->mobility && !unit->unitDef->canhover){	//boat
			float speedf=unit->speed.Length2D();
			float3 pos=unit->pos;
			if(fabs(pos.x-camPosBig.x)>WH_SIZE-50 || fabs(pos.z-camPosBig.z)>WH_SIZE-50)
				continue;
			if(!(unit->losStatus[gu->myAllyTeam] & LOS_INLOS) && !gu->spectatingFullView)
				continue;
			if(pos.y>-4 && pos.y<1){
				float3 frontAdd=unit->frontdir*unit->radius*0.75f;
				float3 sideAdd=unit->rightdir*unit->radius*0.18f;
				float depth=sqrt(sqrt(unit->mass));
				float3 n(depth, 0.04f*speedf*depth, depth);

				va->AddVertexTN(pos+frontAdd+sideAdd,0,0,n);
				va->AddVertexTN(pos+frontAdd-sideAdd,1,0,n);
				va->AddVertexTN(pos-frontAdd-sideAdd,1,1,n);
				va->AddVertexTN(pos-frontAdd+sideAdd,0,1,n);
			}
		} else if(unit->moveType && unit->unitDef->canhover && unit->mobility){		//hover
			float speedf=unit->speed.Length2D();
			float3 pos=unit->pos;
			if(fabs(pos.x-camPosBig.x)>WH_SIZE-50 || fabs(pos.z-camPosBig.z)>WH_SIZE-50)
				continue;
			if(!(unit->losStatus[gu->myAllyTeam] & LOS_INLOS) && !gu->spectatingFullView)
				continue;
			if(pos.y>-4 && pos.y<4){
				float3 frontAdd=unit->frontdir*unit->radius*0.75f;
				float3 sideAdd=unit->rightdir*unit->radius*0.75f;
				float depth=sqrt(sqrt(unit->mass))*0.4f;
				float3 n(depth, 0.05f*depth, depth);

				va2->AddVertexTN(pos+frontAdd+sideAdd,0,0,n);
				va2->AddVertexTN(pos+frontAdd-sideAdd,1,0,n);
				va2->AddVertexTN(pos-frontAdd-sideAdd,1,1,n);
				va2->AddVertexTN(pos-frontAdd+sideAdd,0,1,n);
			}
		}
	}

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,boatShape);

	va->DrawArrayTN(GL_QUADS);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,hoverShape);

	va2->DrawArrayTN(GL_QUADS);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDisable(GL_BLEND);

	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	glDisable( GL_VERTEX_PROGRAM_ARB );

	glFlush();
}

void CDynWater::AddExplosions()
{
	if(explosions.empty())
		return;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, waveTex1, 0);

	glViewport(0,0,1024,1024);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,splashTex);

	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		logOutput.Print("FBO not ready7");

	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, dwAddSplashFP );
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, dwAddSplashVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glDisable(GL_CULL_FACE);

	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 10, 1.0f/WF_SIZE, 1.0f/WF_SIZE, 0, 1);
	glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 11, -(oldCamPosBig.x-WH_SIZE)/WF_SIZE, -(oldCamPosBig.z-WH_SIZE)/WF_SIZE, 0, 0);

	CVertexArray* va=GetVertexArray();
	va->Initialize();

	for(std::vector<Explosion>::iterator ei=explosions.begin(); ei!=explosions.end();++ei){
		Explosion& explo=*ei;
		float3 pos=explo.pos;
		if(fabs(pos.x-camPosBig.x)>WH_SIZE-50 || fabs(pos.z-camPosBig.z)>WH_SIZE-50)
			continue;
		float inv=1.01f;
		if(pos.y<0){
			if(pos.y<-explo.radius*0.5f)
				inv=0.99f;
			pos.y=pos.y*-0.5f;
		}

		float size=explo.radius-pos.y;
		if(size<8)
			continue;
		float strength=explo.strength * (size/explo.radius)*0.5f;

		float3 n(strength, strength*0.005f, strength*inv);

		va->AddVertexTN(pos+float3(1,0,1)*size,0,0,n);
		va->AddVertexTN(pos+float3(-1,0,1)*size,1,0,n);
		va->AddVertexTN(pos+float3(-1,0,-1)*size,1,1,n);
		va->AddVertexTN(pos+float3(1,0,-1)*size,0,1,n);
	}
	explosions.clear();

	va->DrawArrayTN(GL_QUADS);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDisable(GL_BLEND);

	glDisable( GL_FRAGMENT_PROGRAM_ARB );
	glDisable( GL_VERTEX_PROGRAM_ARB );

	glFlush();
}

void CDynWater::AddExplosion(const float3& pos, float strength, float size)
{
	if(pos.y>size || size < 8)
		return;
	explosions.push_back(Explosion(pos,min(size*20,strength),size));
}

void CDynWater::DrawUpdateSquare(float dx,float dy, int* resetTexs)
{
	float startx=max(0.f, -dx/WF_SIZE);
	float starty=max(0.f, -dy/WF_SIZE);
	float endx=min(1.f, 1-dx/WF_SIZE);
	float endy=min(1.f, 1-dy/WF_SIZE);

	DrawSingleUpdateSquare(startx,starty,endx,endy);

	int a=0;
	while(resetTexs[a]>=0){
		glActiveTextureARB(GL_TEXTURE0_ARB+resetTexs[a]);
		glBindTexture(GL_TEXTURE_2D, zeroTex);
		++a;
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);

	if(startx>0){
		DrawSingleUpdateSquare(0,0,startx,1);
	} else if(endx<1){
		DrawSingleUpdateSquare(endx,0,1,1);
	}

	if(starty>0){
		DrawSingleUpdateSquare(startx,0,endx,starty);
	} else if (endy<1){
		DrawSingleUpdateSquare(startx,endy,endx,1);
	}
}

void CDynWater::DrawSingleUpdateSquare(float startx,float starty,float endx,float endy)
{
	float texstart=0.1f/1024;
	float texend=1023.9f/1024;
	float texdif=texend-texstart;

	CVertexArray* va=GetVertexArray();
	va->Initialize();

	va->AddVertexT(float3(startx,starty,0),texstart + startx*texdif,texstart + starty*texdif);
	va->AddVertexT(float3(startx,endy,0),texstart + startx*texdif,texstart + endy*texdif  );
	va->AddVertexT(float3(endx,endy,0),texstart + endx*texdif,texstart + endy*texdif  );
	va->AddVertexT(float3(endx,starty,0),texstart + endx*texdif,texstart + starty*texdif);

	va->DrawArrayT(GL_QUADS);
}

void CDynWater::DrawOuterSurface(void)
{
	CVertexArray* va=GetVertexArray();
	va->Initialize();

	float size=WF_SIZE;
	float size2=WF_SIZE/16;
	float posx=camPosBig2.x-WH_SIZE;
	float posy=camPosBig2.z-WH_SIZE;

	for(int y=-1;y<=1;++y){
		for(int x=-1;x<=1;++x){
			if(x==0 && y==0)
				continue;

			for(int y2=0;y2<16;++y2){
				for(int x2=0;x2<16;++x2){
					va->AddVertex0(float3(posx+x*size+(x2+0)*size2,0,posy+y*size+(y2+0)*size2));
					va->AddVertex0(float3(posx+x*size+(x2+1)*size2,0,posy+y*size+(y2+0)*size2));
					va->AddVertex0(float3(posx+x*size+(x2+1)*size2,0,posy+y*size+(y2+1)*size2));
					va->AddVertex0(float3(posx+x*size+(x2+0)*size2,0,posy+y*size+(y2+1)*size2));
				}
			}
		}
	}

	va->DrawArray0(GL_QUADS);
}
