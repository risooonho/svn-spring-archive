#include "StdAfx.h"
// Bitmap.cpp: implementation of the CBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "Rendering/GL/myGL.h"
#include <ostream>
#include <fstream>
#include "FileSystem/FileHandler.h"
#include <IL/il.h>
#include <boost/filesystem/path.hpp>
#include "Rendering/Textures/Bitmap.h"
#include "bitops.h"
#include "mmgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct InitializeOpenIL {
	InitializeOpenIL() {
		ilInit();
	}
	~InitializeOpenIL() {
		ilShutDown();
	}
} static initOpenIL;

CBitmap::CBitmap()
  : xsize(1),
  ysize(1)
{
	mem=new unsigned char[4];
	ddsimage = 0;
	type = BitmapTypeStandard;
}

CBitmap::~CBitmap()
{
	delete[] mem;
	delete ddsimage;
}

CBitmap::CBitmap(const CBitmap& old)
{
	assert(old.type == BitmapTypeStandard);
	ddsimage = 0;
	xsize=old.xsize;
	ysize=old.ysize;
	mem=new unsigned char[xsize*ysize*4];
	memcpy(mem,old.mem,xsize*ysize*4);
	type = BitmapTypeStandard;
}

CBitmap::CBitmap(unsigned char *data, int xsize, int ysize)
  : xsize(xsize),
  ysize(ysize)
{
	type = BitmapTypeStandard;
	ddsimage = 0;
	mem=new unsigned char[xsize*ysize*4];	
	memcpy(mem,data,xsize*ysize*4);
}

CBitmap::CBitmap(string const& filename)
: mem(0),
  xsize(0),
  ysize(0)
{
	type = BitmapTypeStandard;
	ddsimage = 0;
	Load(filename);
}

CBitmap& CBitmap::operator=(const CBitmap& bm)
{
	if( this != &bm ){
		delete[] mem;
		xsize=bm.xsize;
		ysize=bm.ysize;
		mem=new unsigned char[xsize*ysize*4];

		memcpy(mem,bm.mem,xsize*ysize*4);
	}
	return *this;
}

void CBitmap::Load(string const& filename, unsigned char defaultAlpha)
{
	delete[] mem;
	mem = NULL;

	if(filename.find(".dds")!=string::npos){
		ddsimage = new nv_dds::CDDSImage();
		ddsimage->load(filename);
		type = BitmapTypeDDS;
		return;
	}
	type = BitmapTypeStandard;

	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);

	if(mem != NULL) delete [] mem;

	CFileHandler file(filename);
	if(file.FileExists() == false)
	{
		xsize = 1;
		ysize = 1;
		mem=new unsigned char[4];
		memset(mem, 0, 4);
		return;
	}

	unsigned char *buffer = new unsigned char[file.FileSize()];
	file.Read(buffer, file.FileSize());

	ILuint ImageName = 0;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);

	const bool success = ilLoadL(IL_TYPE_UNKNOWN, buffer, file.FileSize());
	delete [] buffer;

	if(success == false)
	{
		xsize = 1;
		ysize = 1;
		mem=new unsigned char[4];
		memset(mem, 0, 4);
		return;   
	}

	bool noAlpha=ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL)!=4;
#if !defined(__APPLE__) // Temporary fix to allow testing of everything
						// else until i get a quicktime image loader written
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	xsize = ilGetInteger(IL_IMAGE_WIDTH);
	ysize = ilGetInteger(IL_IMAGE_HEIGHT);

	mem = new unsigned char[xsize * ysize * 4];
	//	ilCopyPixels(0,0,0,xsize,ysize,0,IL_RGBA,IL_UNSIGNED_BYTE,mem);
	memcpy(mem, ilGetData(), xsize * ysize * 4);
#else
	xsize = 4;
	ysize = 4;

	mem = new unsigned char[xsize * ysize * 4];
#endif

	ilDeleteImages(1, &ImageName); 

	if(noAlpha){
		for(int y=0;y<ysize;++y){
			for(int x=0;x<xsize;++x){
			mem[(y*xsize+x)*4+3]=defaultAlpha;
			}
		}
	}
}


void CBitmap::Save(string const& filename)
{
	if (type == BitmapTypeDDS) {
		ddsimage->save(filename);
		return;
	}

	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);

	unsigned char* buf=new unsigned char[xsize*ysize*4];
	/* HACK Flip the image so it saves the right way up.
		(Fiddling with ilOriginFunc didn't do anything?)
		Duplicated with ReverseYAxis. */
	for(int y=0;y<ysize;++y){
		for(int x=0;x<xsize;++x){
			buf[((ysize-1-y)*xsize+x)*4+0]=mem[((y)*xsize+x)*4+0];
			buf[((ysize-1-y)*xsize+x)*4+1]=mem[((y)*xsize+x)*4+1];
			buf[((ysize-1-y)*xsize+x)*4+2]=mem[((y)*xsize+x)*4+2];
			buf[((ysize-1-y)*xsize+x)*4+3]=mem[((y)*xsize+x)*4+3];
		}
	}

	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger (IL_JPG_QUALITY, 80);

	ILuint ImageName = 0;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);

	ilTexImage(xsize,ysize,1,4,IL_RGBA,IL_UNSIGNED_BYTE,NULL);
	ilSetData(buf);
	ilSaveImage((char*)filename.c_str());
	ilDeleteImages(1,&ImageName);
	delete[] buf;
}

unsigned int CBitmap::CreateTexture(bool mipmaps)
{
	if(type == BitmapTypeDDS)
	{
		return CreateDDSTexture();
	}

	if(mem==NULL)
		return 0;

	// jcnossen: ATI drivers appear to support OpenGL 2.0, but switch to software rendering when non-power-of-two textures are used.
	if ((xsize != next_power_of_2(xsize) || ysize != next_power_of_2(ysize)))
		 //&& strcmp(reinterpret_cast<const char*>(glGetString(GL_VERSION)), "2.0") < 0 )
	{
		CBitmap bm = CreateRescaled(next_power_of_2(xsize), next_power_of_2(ysize));
		return bm.CreateTexture(mipmaps);
	}

	unsigned int texture;

	glGenTextures(1, &texture);			
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

		// create mipmapped texture
		if (strcmp(reinterpret_cast<const char*>(glGetString(GL_VERSION)), "1.4") >= 0) {
			// This required GL-1.4
			// instead of using glu, we rely on glTexImage2D to create the Mipmaps.
			glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,true);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,xsize, ysize, 0,GL_RGBA, GL_UNSIGNED_BYTE, mem);
		} else
			gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA8 ,xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, mem);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		//glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,xsize, ysize, 0,GL_RGBA, GL_UNSIGNED_BYTE, mem);
		//gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA8 ,xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, mem);

		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8 ,xsize, ysize, 0,GL_RGBA, GL_UNSIGNED_BYTE, mem);
	}
	return texture;
}

unsigned int CBitmap::CreateDDSTexture()
{
	GLuint texobj;
	glPushAttrib(GL_TEXTURE_BIT);

	glGenTextures(1, &texobj);

	switch(ddsimage->get_type())
	{
	case nv_dds::TextureNone:
		glDeleteTextures(1, &texobj);
		texobj = 0;
		break;
	case nv_dds::TextureFlat:    // 1D, 2D, and rectangle textures
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texobj);
		if(!ddsimage->upload_texture2D())
		{
		glDeleteTextures(1, &texobj);
		texobj = 0;
		}
		break;
	case nv_dds::Texture3D:
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, texobj);
		if(!ddsimage->upload_texture3D())
		{
		glDeleteTextures(1, &texobj);
		texobj = 0;
		}
		break;
	case nv_dds::TextureCubemap:
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texobj);
		if(!ddsimage->upload_textureCubemap())
		{
		glDeleteTextures(1, &texobj);
		texobj = 0;
		}
		break;
	}

	glPopAttrib();
	return texobj;
}

void CBitmap::CreateAlpha(unsigned char red,unsigned char green,unsigned char blue)
{
	float3 aCol;
	for(int a=0;a<3;++a)
	{
		int cCol=0;
		int numCounted=0;
		for(int y=0;y<ysize;++y){
			for(int x=0;x<xsize;++x){
				if(mem[(y*xsize+x)*4+3]!=0 && !(mem[(y*xsize+x)*4+0]==red && mem[(y*xsize+x)*4+1]==green && mem[(y*xsize+x)*4+2]==blue)){
					cCol+=mem[(y*xsize+x)*4+a];
					++numCounted;
				}
			}
		}
		if ( numCounted != 0 )
		{
			aCol.xyz[a]=cCol/255.0f/numCounted;
		}
	}
	for(int y=0;y<ysize;++y)
	{
		for(int x=0;x<xsize;++x)
		{
			if(mem[(y*xsize+x)*4+0]==red && mem[(y*xsize+x)*4+1]==green && mem[(y*xsize+x)*4+2]==blue) {
				mem[(y*xsize+x)*4+0]= (unsigned char) (aCol.x*255);
				mem[(y*xsize+x)*4+1]= (unsigned char) (aCol.y*255);
				mem[(y*xsize+x)*4+2]= (unsigned char) (aCol.z*255);
				mem[(y*xsize+x)*4+3]=0;
			}
		}
	}
}

// Depreciated (Only used by GUI which will be replaced by CEGUI anyway)
void CBitmap::SetTransparent( unsigned char red, unsigned char green, unsigned char blue )
{
	for ( unsigned int y = 0; y < xsize; y++ )
	{
		for ( unsigned int x = 0; x < xsize; x++ )
		{
			unsigned int index = (y*xsize + x)*4;
			if ( mem[index+0] == red &&
				mem[index+1] == green &&
				mem[index+2] == blue )
			{
				// set transparent
				mem[index+3] = 0;
			}
		}
	}
}

void CBitmap::Renormalize(float3 newCol)
{
	float3 aCol;
	//	float3 aSpread;

	float3 colorDif;
	//	float3 spreadMul;
	for(int a=0;a<3;++a){
		int cCol=0;
		int numCounted=0;
		for(int y=0;y<ysize;++y){
			for(int x=0;x<xsize;++x){
				if(mem[(y*xsize+x)*4+3]!=0){
					cCol+=mem[(y*xsize+x)*4+a];
					++numCounted;
				}
			}
		}
		aCol.xyz[a]=cCol/255.0f/numCounted;
		cCol/=xsize*ysize;
		colorDif.xyz[a]=newCol[a]-aCol[a];

/*		int spread=0;
        for(int y=0;y<ysize;++y){
        for(int x=0;x<xsize;++x){
        if(mem[(y*xsize+x)*4+3]!=0){
        int dif=mem[(y*xsize+x)*4+a]-cCol;
        spread+=abs(dif);
        }
        }
        }
        aSpread.xyz[a]=spread/255.0f/numCounted;
        spreadMul.xyz[a]=(float)(newSpread[a]/aSpread[a]);*/
	}
	for(int a=0;a<3;++a){
		for(int y=0;y<ysize;++y){
			for(int x=0;x<xsize;++x){
				float nc=float(mem[(y*xsize+x)*4+a])/255.0f+colorDif.xyz[a];
				/*				float r=newCol.xyz[a]+(nc-newCol.xyz[a])*spreadMul.xyz[a];*/
				mem[(y*xsize+x)*4+a]=(unsigned char)(std::min(255.f,std::max(0.f,nc*255)));
			}
		}
	}
}

// Unused
CBitmap CBitmap::GetRegion(int startx, int starty, int width, int height)
{
	CBitmap bm;

	delete[] bm.mem;
	bm.mem=new unsigned char[width*height*4];
	bm.xsize=width;
	bm.ysize=height;

	for(int y=0;y<height;++y){
		for(int x=0;x<width;++x){
			bm.mem[(y*width+x)*4]=mem[((starty+y)*xsize+startx+x)*4];
			bm.mem[(y*width+x)*4+1]=mem[((starty+y)*xsize+startx+x)*4+1];
			bm.mem[(y*width+x)*4+2]=mem[((starty+y)*xsize+startx+x)*4+2];
			bm.mem[(y*width+x)*4+3]=mem[((starty+y)*xsize+startx+x)*4+3];
		}
	}

	return bm;
}

CBitmap CBitmap::CreateMipmapLevel(void)
{
	CBitmap bm;

	delete[] bm.mem;
	bm.xsize=xsize/2;
	bm.ysize=ysize/2;
	bm.mem=new unsigned char[bm.xsize*bm.ysize*4];

	for(int y=0;y<ysize/2;++y){
		for(int x=0;x<xsize/2;++x){
			float r=0,g=0,b=0,a=0;
			for(int y2=0;y2<2;++y2){
				for(int x2=0;x2<2;++x2){
					r+=mem[((y*2+y2)*xsize+x*2+x2)*4+0];
					g+=mem[((y*2+y2)*xsize+x*2+x2)*4+1];
					b+=mem[((y*2+y2)*xsize+x*2+x2)*4+2];
					a+=mem[((y*2+y2)*xsize+x*2+x2)*4+3];
				}
			}
			bm.mem[(y*bm.xsize+x)*4]=(unsigned char)(r/4);
			bm.mem[(y*bm.xsize+x)*4+1]=(unsigned char)(g/4);
			bm.mem[(y*bm.xsize+x)*4+2]=(unsigned char)(b/4);
			bm.mem[(y*bm.xsize+x)*4+3]=(unsigned char)(a/4);
		}
	}

	return bm;

}

CBitmap CBitmap::CreateRescaled(int newx, int newy)
{
	CBitmap bm;

	delete[] bm.mem;
	bm.xsize=newx;
	bm.ysize=newy;
	bm.mem=new unsigned char[bm.xsize*bm.ysize*4];

	float dx=float(xsize)/newx;
	float dy=float(ysize)/newy;

	float cy=0;
	for(int y=0;y<newy;++y){
		int sy=(int)cy;
		cy+=dy;
		int ey=(int)cy;
		if(ey==sy)
			ey=sy+1;

		float cx=0;
		for(int x=0;x<newx;++x){
			int sx=(int)cx;
			cx+=dx;
			int ex=(int)cx;
			if(ex==sx)
				ex=sx+1;

			int r=0,g=0,b=0,a=0;
			for(int y2=sy;y2<ey;++y2){
				for(int x2=sx;x2<ex;++x2){
					r+=mem[(y2*xsize+x2)*4+0];
					g+=mem[(y2*xsize+x2)*4+1];
					b+=mem[(y2*xsize+x2)*4+2];
					a+=mem[(y2*xsize+x2)*4+3];
				}
			}
			bm.mem[(y*bm.xsize+x)*4+0]=r/((ex-sx)*(ey-sy));
			bm.mem[(y*bm.xsize+x)*4+1]=g/((ex-sx)*(ey-sy));
			bm.mem[(y*bm.xsize+x)*4+2]=b/((ex-sx)*(ey-sy));
			bm.mem[(y*bm.xsize+x)*4+3]=a/((ex-sx)*(ey-sy));
		}	
	}
	return bm;
}

void CBitmap::ReverseYAxis(void)
{
	unsigned char* buf=new unsigned char[xsize*ysize*4];

	for(int y=0;y<ysize;++y){
		for(int x=0;x<xsize;++x){
			buf[((ysize-1-y)*xsize+x)*4+0]=mem[((y)*xsize+x)*4+0];
			buf[((ysize-1-y)*xsize+x)*4+1]=mem[((y)*xsize+x)*4+1];
			buf[((ysize-1-y)*xsize+x)*4+2]=mem[((y)*xsize+x)*4+2];
			buf[((ysize-1-y)*xsize+x)*4+3]=mem[((y)*xsize+x)*4+3];
		}
	}
	delete[] mem;
	mem=buf;
}
