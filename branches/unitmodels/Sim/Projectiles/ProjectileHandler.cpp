#include "StdAfx.h"
// ProjectileHandler.cpp: implementation of the CProjectileHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "ProjectileHandler.h"
#include "Rendering/GL/myGL.h"
#include <GL/glu.h>			// Header File For The GLu32 Library
#include "Projectile.h"
#include "Game/Camera.h"
#include "Rendering/GL/VertexArray.h"
#include "Sim/Misc/QuadField.h"
#include "Sim/Units/Unit.h"
#include "TimeProfiler.h"
#include "Rendering/Textures/Bitmap.h"
#include "Rendering/GroundFlash.h"
#include "Sim/Misc/LosHandler.h"
#include "Map/Ground.h"
#include "Rendering/Textures/TextureHandler.h"
#include "Sim/Misc/Feature.h"
#include "Platform/ConfigHandler.h"
#include "Rendering/ShadowHandler.h"
#include "Rendering/UnitModels/UnitDrawer.h"
#include "Rendering/UnitModels/UnitModel.h"
#include "Game/UI/InfoConsole.h"
#include <algorithm>
#include "Rendering/GL/IFramebuffer.h"
#include "mmgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectileHandler* ph;
using namespace std;
extern GLfloat FogBlack[]; 
extern GLfloat FogLand[]; 

CProjectileHandler::CProjectileHandler()
{
	PrintLoadMsg("Creating projectile texture");

	maxParticles=configHandler.GetInt("MaxParticles",4000);

	currentParticles=0;
	particleSaturation=0;
	numPerlinProjectiles=0;

	// This will cause a stack overflow if not made static
	static unsigned char tex[512][512][4];
	static unsigned char tex2[512*256];
	static float dotAlpha[256][256];

	for(int y=0;y<512;y++)
		for(int x=0;x<512;x++){
			tex[y][x][0]=0;
			tex[y][x][1]=0;
			tex[y][x][2]=0;
			tex[y][x][3]=0;
		}

	for(int y=0;y<64;y++){	//circular thingy
		for(int x=0;x<64;x++){
			float dist=sqrt((float)(x-32)*(x-32)+(y-32)*(y-32));
			if(dist>31.875)
				dist=31.875;
			tex[y][x][0]=255-(unsigned char) (dist*8);
			tex[y][x][1]=255-(unsigned char) (dist*8);
			tex[y][x][2]=255-(unsigned char) (dist*8);
			tex[y][x][3]=255-(unsigned char) (dist*8);
		}
	}

	for(int y=0;y<64;y++){	//linear falloff for lasers
		for(int x=0;x<64;x++){
			float dist=abs(y-32);
			if(dist>31.5)
				dist=31.5;
			tex[y][x+320][0]=255-(unsigned char) (dist*8);
			tex[y][x+320][1]=255-(unsigned char) (dist*8);
			tex[y][x+320][2]=255-(unsigned char) (dist*8);
			tex[y][x+320][3]=255;
		}
	}
	for(int y=0;y<64;y++){	//laser endings
		for(int x=0;x<64;x++){
			float dist=sqrt((float)(x-32)*(x-32)+(y-32)*(y-32));
			if(dist>31.875)
				dist=31.875;
			tex[y][x+448][0]=255-(unsigned char) (dist*8);
			tex[y][x+448][1]=255-(unsigned char) (dist*8);
			tex[y][x+448][2]=255-(unsigned char) (dist*8);
			if(tex[y][x+448][0]!=0)
				tex[y][x+448][3]=255;
		}
	}

	TdfParser resources("gamedata/resources.tdf");
	LoadSmoke(tex,64,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0000.bmp","resources\\graphics\\smoke\\smoke00")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0000.bmp","resources\\graphics\\smoke\\smoke00alpha")).c_str());
	LoadSmoke(tex,96,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0001.bmp","resources\\graphics\\smoke\\smoke01")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0001.bmp","resources\\graphics\\smoke\\smoke01alpha")).c_str());
	LoadSmoke(tex,128,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0002.bmp","resources\\graphics\\smoke\\smoke02")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0002.bmp","resources\\graphics\\smoke\\smoke02alpha")).c_str());
	LoadSmoke(tex,160,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0003.bmp","resources\\graphics\\smoke\\smoke03")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0003.bmp","resources\\graphics\\smoke\\smoke03alpha")).c_str());
	LoadSmoke(tex,192,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0004.bmp","resources\\graphics\\smoke\\smoke04")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0004.bmp","resources\\graphics\\smoke\\smoke04alpha")).c_str());
	LoadSmoke(tex,224,0,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0005.bmp","resources\\graphics\\smoke\\smoke05")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0005.bmp","resources\\graphics\\smoke\\smoke05alpha")).c_str());

	LoadSmoke(tex,64,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0006.bmp","resources\\graphics\\smoke\\smoke06")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0006.bmp","resources\\graphics\\smoke\\smoke06alpha")).c_str());
	LoadSmoke(tex,96,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0007.bmp","resources\\graphics\\smoke\\smoke07")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0007.bmp","resources\\graphics\\smoke\\smoke07alpha")).c_str());
	LoadSmoke(tex,128,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0008.bmp","resources\\graphics\\smoke\\smoke08")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0008.bmp","resources\\graphics\\smoke\\smoke08alpha")).c_str());
	LoadSmoke(tex,160,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0009.bmp","resources\\graphics\\smoke\\smoke09")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0009.bmp","resources\\graphics\\smoke\\smoke09alpha")).c_str());
	LoadSmoke(tex,192,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0010.bmp","resources\\graphics\\smoke\\smoke10")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0010.bmp","resources\\graphics\\smoke\\smoke10alpha")).c_str());
	LoadSmoke(tex,224,32,
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke0011.bmp","resources\\graphics\\smoke\\smoke11")).c_str(),
			(char*)("bitmaps/"+resources.SGetValueDef("smoke/smoke_Alpha0011.bmp","resources\\graphics\\smoke\\smoke11alpha")).c_str());

	for(int y=0;y<64;y++){		//fix smoke
		for(int x=64;x<256;x++){
			int a=(255-tex[y][x][3])/3;
			tex[y][x][0]-=a;
			tex[y][x][1]-=a;
			tex[y][x][2]-=a;
		}
	}

	for(int y=64;y<96;y++){	//smoke trail
		for(int x=0;x<256;x++){
			tex[y][x][0]=128;
			tex[y][x][1]=128;
			tex[y][x][2]=128;
			tex[y][x][3]=0;
		}
	}

	for(int a=0;a<16;++a){//smoke trail
		int xnum=(rand()%6)*32;
		int	ynum=(rand()&1)*32;
		for(int by=0;by<32;by++){
			int y=by+64;
			for(int bx=0;bx<32;bx++){
				int x=bx+a*8;
				if(x>127)
					x-=128;
				if(tex[by+ynum][bx+xnum+64][3]==0)
					continue;
				int totalAlpha=tex[y][x+16][3]+tex[by+ynum][bx+xnum+64][3];
				float alpha=(tex[by+ynum][bx+xnum+64][3]/255.0)/(totalAlpha/255.0);
				tex[y][x+16][0]=(unsigned char) (tex[y][x+16][0]*(1-alpha)+tex[by+ynum][bx+xnum+64][0]*alpha);
				tex[y][x+16][1]=(unsigned char) (tex[y][x+16][1]*(1-alpha)+tex[by+ynum][bx+xnum+64][1]*alpha);
				tex[y][x+16][2]=(unsigned char) (tex[y][x+16][2]*(1-alpha)+tex[by+ynum][bx+xnum+64][2]*alpha);
				tex[y][x+16][3]=min(255,totalAlpha);
			}
		}
	}
	for(int y=0;y<32;y++){//smoke trail
		for(int x=0;x<16;x++){
			for(int c=0;c<4;++c){
				tex[y+64][x][c]=tex[y+64][112+x][c];
				tex[y+64][144+x][c]=tex[y+64][16+x][c];			
			}
		}
	}
	for(int y=0;y<32;y++){//smoke trail
		float dist=1-fabs(float(y-16))/16.0;
		float amod=sqrt(dist);
		for(int x=0;x<160;x++){
			tex[y+64][x][3]=(unsigned char) (tex[y+64][x][3]*amod);
		}
	}
	for(int y2=0;y2<2;++y2){		//make alpha fall of a bit radially for the smoke
		int yoffs=y2*32;
		for(int x2=0;x2<6;++x2){
			int xoffs=64+x2*32;
			for(int y=0;y<32;y++){
				for(int x=0;x<32;x++){
					float xd=(x-16)/16.0;
					float yd=(y-16)/16.0;
					float dist=xd*xd+yd*yd;
					tex[yoffs+y][xoffs+x][3]=(unsigned char)( max(0.0,(1-dist*0.7)) * tex[yoffs+y][xoffs+x][3]);
				}
			}
		}
	}
	ConvertTex(tex,64,0,256,64,1);
	ConvertTex(tex,0,64,256,128,1);

	CBitmap explo((char*)("bitmaps/"+resources.SGetValueDef("explo.bmp","resources\\graphics\\explosions\\explo")).c_str());
	for(int y=0;y<128;y++){
		for(int x=0;x<128;x++){
			tex[y+128][x+128][0]=explo.mem[(y*128+x)*4];
			tex[y+128][x+128][1]=explo.mem[(y*128+x)*4+1];
			tex[y+128][x+128][2]=explo.mem[(y*128+x)*4+2];
			if(explo.mem[(y*128+x)*4]!=0)
				tex[y+128][x+128][3]=255;
		}
	}
	for(int y=0;y<128;y++){	//explo med fadeande alpha
		for(int x=0;x<128;x++){
			tex[y+128][x][0]=explo.mem[(y*128+x)*4];
			tex[y+128][x][1]=explo.mem[(y*128+x)*4+1];
			tex[y+128][x][2]=explo.mem[(y*128+x)*4+2];
			tex[y+128][x][3]=(explo.mem[(y*128+x)*4]+explo.mem[(y*128+x)*4+1]+explo.mem[(y*128+x)*4+2])/3;
		}
	}

	CBitmap flare((char*)("bitmaps/"+resources.SGetValueDef("flare.bmp","resources\\graphics\\flares\\flare")).c_str());
	for(int y=0;y<128;y++){
		for(int x=0;x<256;x++){
			tex[y+64][x+256][0]=flare.mem[(y*256+x)*4+0];
			tex[y+64][x+256][1]=flare.mem[(y*256+x)*4+1];
			tex[y+64][x+256][2]=flare.mem[(y*256+x)*4+2];
			tex[y+64][x+256][3]=255;
			if(!tex[y+64][x+256][0] && !tex[y+64][x+256][1] && !tex[y+64][x+256][2])
				tex[y+64][x+256][3]=0;
		}
	}

	for(int y=0;y<256;y++){//random dots
		for(int x=0;x<256;x++){
			tex[y+256][x][0]=205+(unsigned char) (gs->randFloat()*50);
			tex[y+256][x][1]=205+(unsigned char) (gs->randFloat()*50);
			tex[y+256][x][2]=205+(unsigned char) (gs->randFloat()*50);
			tex[y+256][x][3]=0;
			dotAlpha[y][x]=gu->usRandFloat()*30;
		}
	}
	for(int a=0;a<3;++a){//random dots
		float mag=(60<<a);
		float size=(2<<a);
		for(int y=(int)size;y<255-(int)size;y+=(int)size){
			for(int x=(int)size;x<255-(int)size;x+=(int)size){
				float p=gu->usRandFloat()*mag;
				for(int y2=y-(int)size;y2<=y+(int)size;y2++){
					float ym=float(size-abs(y2-y))/size;
					for(int x2=x-(int)size;x2<=x+(int)size;x2++){
						float xm=float(size-abs(x2-x))/size;
						dotAlpha[y2][x2]+=p*xm*ym;
					}
				}
			}
		}
	}

	for(int a=0;a<20;++a){//random dots
		int bx=(int) (gu->usRandFloat()*228);
		int by=(int) (gu->usRandFloat()*228)+256;
		for(int y=0;y<16;y++){
			for(int x=0;x<16;x++){
				float dist=sqrt((float)(x-8)*(x-8)+(y-8)*(y-8));
				if(dist>8)
					continue;
				int alpha=60+(int)(-dist*35+dotAlpha[by-256+y][bx+x]);
				if(tex[by+y][bx+x][3]<alpha){
					tex[by+y][bx+x][3]=max(0,min(255,alpha));
				}
			}
		}
	}
	ConvertTex(tex,0,256,256,512,1);

	for(int y=256;y<256+128;++y){//wake
		for(int x=256;x<256+128;++x){
			tex[y][x][0]=220;
			tex[y][x][1]=230;
			tex[y][x][2]=255;
		}
	}
	for(int a=0;a<40;++a){//wake
		float3 r(0,0,0);
		do{
			r.x=(gu->usRandFloat()-0.5)*2;
			r.y=(gu->usRandFloat()-0.5)*2;
		} while(r.Length()>1);
		int bx=(int)(r.x*52)+256+64-12;
		int by=(int)(r.y*52)+256+64-12;
		for(int y=0;y<24;y++){
			for(int x=0;x<24;x++){
				float dist=sqrt((float)(x-12)*(x-12)+(y-12)*(y-12));
				if(dist>12)
					continue;
				float alpha=255-dist*20;
				float alpha2=tex[by+y][bx+x][3];
				alpha=1-((1-alpha/255)*(1-alpha2/255));
				tex[by+y][bx+x][3]=(unsigned char) max((float)0,min((float)255,alpha*255));
			}
		}
	}
	ConvertTex(tex,256,256,256+128,256+128,1);

	for(int y=256;y<256+128;++y){//shield
		for(int x=384;x<512;++x){
			tex[y][x][0]=70;
			tex[y][x][1]=70;
			tex[y][x][2]=70;
			tex[y][x][3]=70;
		}
	}

	glGenTextures(1, CProjectile::textures);
	glBindTexture(GL_TEXTURE_2D, CProjectile::textures[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8,512, 512, GL_RGBA, GL_UNSIGNED_BYTE, tex[0]);

	CBitmap b(tex[0][0],512,512);
	b.Save("proj.tga");

	for(int y=0;y<256;y++){
		for(int x=0;x<256;x++){
			float dist=sqrt((float)(x-127)*(x-127)+(y-127)*(y-127))/128.0;
			float alpha;
			if(dist<0.5){
				alpha=0.0;
			}else {
				alpha=1-4.0*fabs(dist-0.75);
				if(alpha<0)
					alpha=0;
			}
			tex2[y*256+x]=(unsigned char)(alpha*255);
			alpha=1-dist;
			if(alpha<0)
				alpha=0;
			tex2[(y+256)*256+x]=(unsigned char)(alpha*255);
		}
	}
	/*
	unsigned char *tempdata = new unsigned char[256*512*4];
	for (int y=0;y<512*256;y++) {
		tempdata[y*4+0] = tempdata[y*4+1] = tempdata[y*4+2] = tex2[y];
		tempdata[y*4+3] = 255;
	}
	CBitmap tmp(tempdata, 256,512);
	tmp.Save ("groundflash.bmp");
	delete[] tempdata;*/

	glGenTextures(1,&CGroundFlash::texture);
	glBindTexture(GL_TEXTURE_2D, CGroundFlash::texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 1,256, 512, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex2);

	if(shadowHandler->drawShadows){
		projectileShadowVP=LoadVertexProgram("projectileshadow.vp");
	}

	for(int a=0;a<4;++a){
		perlinBlend[a]=0;
	}
	unsigned char tempmem[4*16*16];
	for(int a=0;a<4*16*16;++a)
		tempmem[a]=0;
	for(int a=0;a<8;++a){
		glGenTextures(1, &perlinTex[a]);
		glBindTexture(GL_TEXTURE_2D, perlinTex[a]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8,16, 16, GL_RGBA, GL_UNSIGNED_BYTE, tempmem);
	}

	drawPerlinTex=false;

	if(shadowHandler && shadowHandler->drawShadows){
		perlinFB = instantiate_fb(128);
		if (perlinFB && perlinFB->valid()){
			drawPerlinTex=true;
			perlinFB->attachTexture(CProjectile::textures[0], GL_TEXTURE_2D, FBO_ATTACH_COLOR);
			perlinFB->checkFBOStatus();
		}
	} else
		perlinFB = 0;
}

CProjectileHandler::~CProjectileHandler()
{
	glDeleteTextures (1, CProjectile::textures);
	glDeleteTextures (1, &CGroundFlash::texture);
	for(int a=0;a<8;++a)
		glDeleteTextures (1, &perlinTex[a]);

	Projectile_List::iterator psi;
	for(psi=ps.begin();psi!=ps.end();++psi)
		delete *psi;
	std::vector<CGroundFlash*>::iterator gfi;
	for(gfi=groundFlashes.begin();gfi!=groundFlashes.end();++gfi)
		delete *gfi;
	distlist.clear();

	if(shadowHandler->drawShadows){
		glDeleteProgramsARB( 1, &projectileShadowVP );
	}

	ph=0;
	if (perlinFB)
	{
		delete perlinFB;
	}
}

void CProjectileHandler::Update()
{
START_TIME_PROFILE
	Projectile_List::iterator psi=ps.begin();
	while(psi!= ps.end()){
		CProjectile *p = *psi;
		if(p->deleteMe){
			Projectile_List::iterator prev=psi++;
			ps.erase(prev);
			delete p;
		} else {
			(*psi)->Update();
			++psi;
		}
	}

	for(unsigned int i = 0; i < groundFlashes.size();)
	{
		CGroundFlash *gf = groundFlashes[i];
		if (!gf->Update ())
		{
			// swap gf with the groundflash at the end of the list, so pop_back() can be used to erase it
			if ( i < groundFlashes.size()-1 )
				std::swap (groundFlashes.back(), groundFlashes[i]);
			groundFlashes.pop_back();
			delete gf;
		} else i++;
	}

END_TIME_PROFILE("Projectile handler");
}

int CompareProjDist(CProjectileHandler::projdist const &arg1, CProjectileHandler::projdist const &arg2){
	if (arg1.dist >= arg2.dist)
	   return 0;
   return 1;
}

void CProjectileHandler::Draw(bool drawReflection,bool drawRefraction)
{
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(1);

	CVertexArray* va=GetVertexArray();

	int numFlyingPieces=0;
	int drawnPieces=0;


	/* Putting in, say, viewport culling will deserve refactoring. */
/*	
	unitDrawer->SetupForUnitDrawing();

	va->Initialize();
	numFlyingPieces += flying3doPieces->size();
	for(std::list<FlyingPiece*>::iterator pi=flying3doPieces->begin();pi!=flying3doPieces->end();++pi){
		CMatrix44f m;
		m.Rotate((*pi)->rot,(*pi)->rotAxis);
		float3 interPos=(*pi)->pos+(*pi)->speed*gu->timeOffset;
		CTextureHandler::UnitTexture* tex=(*pi)->prim->texture;

		C3DOVertex* v=&(*pi)->object->vertices[(*pi)->prim->vertices[0]];
		float3 tp=m.Mul(v->pos);
		float3 tn=m.Mul(v->normal);
		tp+=interPos;
		va->AddVertexTN(tp,tex->xstart,tex->ystart,tn);

		v=&(*pi)->object->vertices[(*pi)->prim->vertices[1]];
		tp=m.Mul(v->pos);
		tn=m.Mul(v->normal);
		tp+=interPos;
		va->AddVertexTN(tp,tex->xend,tex->ystart,tn);

		v=&(*pi)->object->vertices[(*pi)->prim->vertices[2]];
		tp=m.Mul(v->pos);
		tn=m.Mul(v->normal);
		tp+=interPos;
		va->AddVertexTN(tp,tex->xend,tex->yend,tn);

		v=&(*pi)->object->vertices[(*pi)->prim->vertices[3]];
		tp=m.Mul(v->pos);
		tn=m.Mul(v->normal);
		tp+=interPos;
		va->AddVertexTN(tp,tex->xstart,tex->yend,tn);
	}
	drawnPieces+=va->drawIndex/32;
	va->DrawArrayTN(GL_QUADS);

	unitDrawer->CleanUpUnitDrawing();

	unitDrawer->SetupForS3ODrawing();

	for (int textureType = 1; textureType < flyings3oPieces.size(); textureType++){
		
		texturehandler->SetS3oTexture(textureType);
		
		for (int team = 0; team < flyings3oPieces[textureType].size(); team++){
			FlyingPiece_List * fpl = flyings3oPieces[textureType][team];
		
			unitDrawer->SetS3OTeamColour(team);
			
			va->Initialize();
			
			numFlyingPieces += fpl->size();
		
			for(std::list<FlyingPiece*>::iterator pi=fpl->begin();pi!=fpl->end();++pi){
				CMatrix44f m;
				m.Rotate((*pi)->rot,(*pi)->rotAxis);
				float3 interPos=(*pi)->pos+(*pi)->speed*gu->timeOffset;
				
				S3OVertex * verts = (*pi)->verts;
				
				float3 tp, tn;
				
				for (int i = 0; i < 4; i++){
					tp=m.Mul(verts[i].pos);
					tn=m.Mul(verts[i].normal);
					tp+=interPos;
					va->AddVertexTN(tp,verts[i].textureX,verts[i].textureY,tn);
				}
			}
			drawnPieces+=va->drawIndex/32;
			va->DrawArrayTN(GL_QUADS);
		}
	}
	unitDrawer->CleanUpS3ODrawing();
	*/
	
	/*
	 * TODO Nearly cut here.
	 */
/*
	unitDrawer->SetupForUnitDrawing();
	Projectile_List::iterator psi;
	distlist.clear();
	for(psi=ps.begin();psi != ps.end();++psi){
		if(camera->InView((*psi)->pos,(*psi)->drawRadius) && (loshandler->InLos(*psi,gu->myAllyTeam) || gu->spectating || ((*psi)->owner && gs->Ally((*psi)->owner->allyteam,gu->myAllyTeam)))){
			if(drawReflection){
				if((*psi)->pos.y < -(*psi)->drawRadius)
					continue;
				float dif=(*psi)->pos.y-camera->pos.y;
				float3 zeroPos=camera->pos*((*psi)->pos.y/dif) + (*psi)->pos*(-camera->pos.y/dif);
				if(ground->GetApproximateHeight(zeroPos.x,zeroPos.z)>3+0.5*(*psi)->drawRadius)
					continue;
			}
			if(drawRefraction && (*psi)->pos.y>(*psi)->drawRadius)
				continue;
			if((*psi)->s3domodel){
				if((*psi)->s3domodel->textureType){
					unitDrawer->QueS3ODraw(*psi,(*psi)->s3domodel->textureType);
				} else {
					(*psi)->DrawUnitPart();
				}
			}
			struct projdist tmp;
			tmp.proj=*psi;
			tmp.dist=(*psi)->pos.dot(camera->forward);
			distlist.push_back(tmp);
		}
	}
	
	unitDrawer->CleanUpUnitDrawing();
	unitDrawer->DrawQuedS3O();
*/
	sort(distlist.begin(), distlist.end(), CompareProjDist);

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, CProjectile::textures[0]);
	glEnable(GL_BLEND);
	glDepthMask(0);
	glColor4f(1,1,1,0.2f);
	glAlphaFunc(GL_GREATER,0.0);
	glEnable(GL_ALPHA_TEST);
//	glFogfv(GL_FOG_COLOR,FogLand);
	glDisable(GL_FOG);

	currentParticles=0;
	CProjectile::inArray=false;
	CProjectile::va=GetVertexArray();
	CProjectile::va->Initialize();
	for(int a=0;a<distlist.size();a++){
		distlist.at(a).proj->Draw();
	}
	if(CProjectile::inArray)
		CProjectile::DrawArray();
	glDisable(GL_TEXTURE_2D);
	glDepthMask(1);
	currentParticles=(int)(ps.size()*0.8+currentParticles*0.2);
	currentParticles+=(int)(0.2*drawnPieces+0.3*numFlyingPieces);
	particleSaturation=(float)currentParticles/(float)maxParticles;
//	glFogfv(GL_FOG_COLOR,FogLand);
}

void CProjectileHandler::DrawShadowPass(void)
{
	distlist.clear();
	Projectile_List::iterator psi;
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, projectileShadowVP );
	glEnable( GL_VERTEX_PROGRAM_ARB );
	glDisable(GL_TEXTURE_2D);
/*	for(psi=ps.begin();psi != ps.end();++psi){
		if((loshandler->InLos(*psi,gu->myAllyTeam) || gu->spectating || ((*psi)->owner && gs->Ally((*psi)->owner->allyteam,gu->myAllyTeam)))){
			if((*psi)->s3domodel)
				(*psi)->DrawUnitPart();
			if((*psi)->castShadow){
				struct projdist tmp;
				tmp.proj = *psi;
				distlist.push_back(tmp);
			}
		}
	}*/

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, CProjectile::textures[0]);
	glColor4f(1,1,1,1);
	glAlphaFunc(GL_GREATER,0.3);
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);

	CProjectile::inArray=false;
	CProjectile::va=GetVertexArray();
	CProjectile::va->Initialize();
	for(int b=0;b<distlist.size();b++){
		distlist.at(b).proj->Draw();
	}
	if(CProjectile::inArray)
		CProjectile::DrawArray();

	glShadeModel(GL_FLAT);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable( GL_VERTEX_PROGRAM_ARB );
}

void CProjectileHandler::AddProjectile(CProjectile* p)
{
	ps.push_back(p);
//	toBeAddedProjectile.push(p);
}

void CProjectileHandler::LoadSmoke(unsigned char tex[512][512][4],int xoffs,int yoffs,char* filename,char* alphafile)
{
	CBitmap TextureImage(filename);
	CBitmap AlphaImage(alphafile);
	for(int y=0;y<32;y++){
		for(int x=0;x<32;x++){
			tex[yoffs+y][xoffs+x][0]=TextureImage.mem[(y*32+x)*4];
			tex[yoffs+y][xoffs+x][1]=TextureImage.mem[(y*32+x)*4+1];
			tex[yoffs+y][xoffs+x][2]=TextureImage.mem[(y*32+x)*4+2];
			tex[yoffs+y][xoffs+x][3]=AlphaImage.mem[(y*32+x)*4];
		}
	}
}

void CProjectileHandler::CheckUnitCol()
{
	Projectile_List::iterator psi;
	for(psi=ps.begin();psi != ps.end();++psi){
		CProjectile* p=(*psi);
		if(p->checkCol && !p->deleteMe){
			CUnit* owner=p->owner;
			float speedf=p->speed.Length();
			float ispeedf=1.0f/speedf;
			float3 normSpeed=p->speed*ispeedf;

			vector<CUnit*> units=qf->GetUnitsExact(p->pos,p->radius+speedf);
			for(vector<CUnit*>::iterator ui(units.begin());ui!=units.end();++ui){
				CUnit* unit=*ui;
				if(owner == unit)
					continue;
				float totalRadius=unit->radius+p->radius;
				float3 dif(unit->midPos-p->pos);
				float closeTime=dif.dot(normSpeed)*ispeedf;
				if(closeTime<0)
					closeTime=0;
				if(closeTime>1)
					closeTime=1;
				float3 closeVect=dif-(p->speed*closeTime);
				if(dif.SqLength() < totalRadius*totalRadius){
					if(unit->isMarkedOnBlockingMap && unit->physicalState != CSolidObject::Flying){
						float3 closePos(p->pos+p->speed*closeTime);
						int square=(int)max(0.,min((double)(gs->mapSquares-1),closePos.x*(1./8.)+int(closePos.z*(1./8.))*gs->mapx));
						if(readmap->groundBlockingObjectMap[square]!=unit)
							continue;
					}
					p->Collision(*ui);
					break;
				}
			}

			vector<CFeature*> features=qf->GetFeaturesExact(p->pos,p->radius+speedf);
			for(vector<CFeature*>::iterator fi=features.begin();fi!=features.end();++fi){
				if(!(*fi)->blocking)
					continue;
				float totalRadius=(*fi)->radius+p->radius;
				float3 dif=(*fi)->midPos-p->pos;
				float closeTime=dif.dot(normSpeed)/speedf;
				if(closeTime<0)
					closeTime=0;
				if(closeTime>1)
					closeTime=1;
				float3 closeVect=dif-(p->speed*closeTime);
				if(dif.SqLength() < totalRadius*totalRadius){
					if((*fi)->isMarkedOnBlockingMap){
						float3 closePos(p->pos+p->speed*closeTime);
						int square=(int)max(0.,min((double)(gs->mapSquares-1),closePos.x*(1./8.)+int(closePos.z*(1./8.))*gs->mapx));
						if(readmap->groundBlockingObjectMap[square]!=(*fi))
							continue;
					}
					p->Collision();
					break;
				}
			}
		}
	}	
}

void CProjectileHandler::AddGroundFlash(CGroundFlash* flash)
{
	groundFlashes.push_back(flash);
}


void CProjectileHandler::DrawGroundFlashes(void)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, CGroundFlash::texture);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glPolygonOffset(-20,-1000);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glFogfv(GL_FOG_COLOR,FogBlack);

	CGroundFlash::va=GetVertexArray();
	CGroundFlash::va->Initialize();

	std::vector<CGroundFlash*>::iterator gfi;
	for(gfi=groundFlashes.begin();gfi!=groundFlashes.end();++gfi){
		(*gfi)->Draw();
	}
	CGroundFlash::va->DrawArrayTC(GL_QUADS);

	glFogfv(GL_FOG_COLOR,FogLand);
	glDepthMask(1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}

void CProjectileHandler::ConvertTex(unsigned char tex[512][512][4], int startx, int starty, int endx, int endy, float absorb)
{
	for(int y=starty;y<endy;++y){
		for(int x=startx;x<endx;++x){
			float alpha=tex[y][x][3];
			float mul=alpha/255.0;
			tex[y][x][0]=(unsigned char)(mul * (float)tex[y][x][0]);
			tex[y][x][1]=(unsigned char)(mul * (float)tex[y][x][1]);
			tex[y][x][2]=(unsigned char)(mul * (float)tex[y][x][2]);
		}
	}
}

/*
void CProjectileHandler::AddFlyingPiece(float3 pos,float3 speed,C3DOPiece* object,C3DOPrimitive* piece)
{
	FlyingPiece* fp=new FlyingPiece;
	fp->pos=pos;
	fp->speed=speed;
	fp->prim=piece;
	fp->object=object;
	fp->verts=NULL;

	fp->rotAxis=gu->usRandVector();
	fp->rotAxis.Normalize();
	fp->rotSpeed=gu->usRandFloat()*0.1;
	fp->rot=0;

	flying3doPieces->push_back(fp);
}

void CProjectileHandler::AddFlyingPiece(int textureType, int team, float3 pos, float3 speed, S3OVertex * verts){
	FlyingPiece_List * pieceList = NULL;
	
	while(flyings3oPieces.size()<=textureType)
		flyings3oPieces.push_back(std::vector<FlyingPiece_List*>());
	
	while(flyings3oPieces[textureType].size()<=team){
		printf("Creating piece list %d %d.\n", textureType, flyings3oPieces[textureType].size());
		
		FlyingPiece_List * fpl = new FlyingPiece_List;
		flyings3oPieces[textureType].push_back(fpl);
		flyingPieces.push_back(fpl);
	}
	
	pieceList=flyings3oPieces[textureType][team];

	FlyingPiece* fp=new FlyingPiece;
	fp->pos=pos;
	fp->speed=speed;
	fp->prim=NULL;
	fp->object=NULL;
	fp->verts=verts;

	fp->rotAxis=gu->usRandVector();
	fp->rotAxis.Normalize();
	fp->rotSpeed=gu->usRandFloat()*0.1;
	fp->rot=0;

	pieceList->push_back(fp);
}*/

void CProjectileHandler::UpdateTextures()
{
	if(numPerlinProjectiles && drawPerlinTex)
		UpdatePerlin();
}

void CProjectileHandler::UpdatePerlin()
{
	perlinFB->select();
	glViewport(384,256,128,128);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);

	unsigned char col[4];	
	float time=gu->lastFrameTime*gs->speedFactor*3;
	float speed=1;
	float size=1;
	for(int a=0;a<4;++a){
		perlinBlend[a]+=time*speed;
		if(perlinBlend[a]>1){
			unsigned int temp=perlinTex[a*2];
			perlinTex[a*2]=perlinTex[a*2+1];
			perlinTex[a*2+1]=temp;
			GenerateNoiseTex(perlinTex[a*2+1],16);
			perlinBlend[a]-=1;
		}

		float tsize=8/size;

		if(a==0)
			glDisable(GL_BLEND);

		CVertexArray* va=GetVertexArray();
		va->Initialize();
		for(int b=0;b<4;++b)
			col[b]=int((1-perlinBlend[a])*16*size);
		glBindTexture(GL_TEXTURE_2D, perlinTex[a*2]);
		va->AddVertexTC(float3(0,0,0),0,0,col);
		va->AddVertexTC(float3(0,1,0),0,tsize,col);
		va->AddVertexTC(float3(1,1,0),tsize,tsize,col);
		va->AddVertexTC(float3(1,0,0),tsize,0,col);
		va->DrawArrayTC(GL_QUADS);

		if(a==0)
			glEnable(GL_BLEND);

		va=GetVertexArray();
		va->Initialize();
		for(int b=0;b<4;++b)
			col[b]=int(perlinBlend[a]*16*size);
		glBindTexture(GL_TEXTURE_2D, perlinTex[a*2+1]);
		va->AddVertexTC(float3(0,0,0),0,0,col);
		va->AddVertexTC(float3(0,1,0),0,tsize,col);
		va->AddVertexTC(float3(1,1,0),tsize,tsize,col);
		va->AddVertexTC(float3(1,0,0),tsize,0,col);
		va->DrawArrayTC(GL_QUADS);

		speed*=0.6;
		size*=2;
	}
/*
	glBindTexture(GL_TEXTURE_2D, CProjectile::textures[0]);
	glCopyTexSubImage2D(GL_TEXTURE_2D,0,384,256,0,0,128,128);
*/
	perlinFB->deselect();
	glViewport(0,0,gu->screenx,gu->screeny);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void CProjectileHandler::GenerateNoiseTex(unsigned int tex,int size)
{
	unsigned char* mem=new unsigned char[4*size*size];

	for(int a=0;a<size*size;++a){
		unsigned char rnd=int(max(0.f,gu->usRandFloat()*555-300));
		mem[a*4+0]=rnd;
		mem[a*4+1]=rnd;
		mem[a*4+2]=rnd;
		mem[a*4+3]=rnd;
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,size,size,GL_RGBA,GL_UNSIGNED_BYTE,mem);
	delete[] mem;
}
