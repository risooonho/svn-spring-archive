// Bitmap.cpp: implementation of the CBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include <ostream.h>
#include <fstream.h>
#include "FileHandler.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include "Bitmap.h"
#include <assert.h> 
#include <iostream.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct InitializeOpenIL {
	InitializeOpenIL() {
		ilInit();
		iluInit();
	}
	~InitializeOpenIL() {
		ilShutDown();
	}
}static initOpenIL;

CBitmap::CBitmap()
  : xsize(1),
  ysize(1)
{
	mem=new unsigned char[4];
}

CBitmap::~CBitmap()
{
	delete[] mem;
}

CBitmap::CBitmap(const CBitmap& old)
{
	xsize=old.xsize;
	ysize=old.ysize;
	mem=new unsigned char[xsize*ysize*4];
	memcpy(mem,old.mem,xsize*ysize*4);
}

CBitmap::CBitmap(unsigned char *data, int xsize, int ysize)
  : xsize(xsize),
  ysize(ysize)
{
	mem=new unsigned char[xsize*ysize*4];	
	memcpy(mem,data,xsize*ysize*4);
}

CBitmap::CBitmap(string const& filename,int x,int y,bool add,int type,bool tex,bool tex2,bool geotherm)
: mem(0),
  xsize(0),
  ysize(0)
{
	Load(filename,255,x,y,2,add,type,tex,tex2,geotherm);
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

void CBitmap::Load(string const& filename, unsigned char defaultAlpha,int x,int y,int filter,bool add,int type,bool tex,bool typemap,bool geotherm)
{	 
	delete[] mem;
	mem = NULL;
	
	xsize = 1;
	ysize = 1;

	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);

	if(mem != NULL) delete [] mem;

	CFileHandler file(filename);
	if(file.FileExists() == false)
	{
		cout << "file " << filename << " not found" << endl;
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
		cout << "failed to load " << filename << endl;
		xsize = 1;
		ysize = 1;
		mem=new unsigned char[4];
		memset(mem, 0, 4);
		return;   
	}

	bool noAlpha=ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL)!=4;
	
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!x == 1 && !x < 1 || !y == 1 && !y < 1){
	   if (filter == 1){
		  cout << "using ilu_nearest for rescaling" << endl; 
          iluImageParameter(ILU_FILTER,ILU_NEAREST);
	   }   
	   if (filter == 3){
	      cout << "using ilu_scale_mitchel for rescaling" << endl; 
	      iluImageParameter(ILU_FILTER,ILU_SCALE_MITCHELL);
	   }   
	   if (!filter == 1 && !filter == 3){
		  cout << "using ilu_scale_triangle for rescaling" << endl;
	      iluImageParameter(ILU_FILTER,ILU_SCALE_TRIANGLE); 
	   }   
	}        
	   
	int both = 0;
	
	if (geotherm == false){
	 if (!(x == 1 || y == 1)){
	   both = 1;
	   if (tex == true){
		  cout << "scaling texture in both xy axis..." << endl;  
          iluScale(x*512,y*512,1);
	   }
	   if (tex == false){
	      if (typemap == false){
		     cout << "scaling something else in both xy axis..." << endl;
		     if (add == true)
	             iluScale((x*64)+1,(y*64)+1,1); 
	         if (add == false)
	             iluScale(x*64,y*64,1);   
	      }   
	      if (typemap == true){
		     cout << "scaling typemap in both xy axis..." << endl; 
	         iluScale(x*32,y*32,1);
	      }   
	   }    
       if (both == 0){
    	  if (x > 1){  
    	     if (tex == true){
    		     cout << "scaling texture in both xy axis..." << endl;  
                 iluScale(x*512,IL_IMAGE_HEIGHT,1);
    	     }
    	     if (tex == false){
    	         if (typemap == false){
    		        cout << "scaling something else in both xy axis..." << endl;
    		        if (add == true)
    	                iluScale((x*64)+1,ilGetInteger(IL_IMAGE_HEIGHT),1);
    	            if (add == false)
    	                iluScale(x*64,ilGetInteger(IL_IMAGE_HEIGHT),1); 
    	         }
    	         if (typemap == true){
    		        cout << "scaling typemap in both xy axis..." << endl; 
    	            iluScale(x*32,ilGetInteger(IL_IMAGE_HEIGHT),1);
    	         }   
    	     }
    	  }
    	  if (y > 1){  
    	     if (tex == true){
    		     cout << "scaling texture in both xy axis..." << endl;  
                 iluScale(ilGetInteger(IL_IMAGE_WIDTH),y*512,1);
    	     }
    	     if (tex == false){
    	         if (typemap == false){
    		        cout << "scaling something else in both xy axis..." << endl;
    		        if (add == true)
    	                iluScale(ilGetInteger(IL_IMAGE_WIDTH),(y*64)+1,1);
    	            if (add == false)
    	                iluScale(ilGetInteger(IL_IMAGE_WIDTH),y*64,1); 
    	         }
    	         if (typemap == true){
    		        cout << "scaling typemap in both xy axis..." << endl; 
    	            iluScale(ilGetInteger(IL_IMAGE_WIDTH),y*512,1);
    	         }   
    	     }
    	  }
	   }  
	 }
	
	 switch (type){
	      case 1: cout << "current texture width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
	              cout << "current texture height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
	              break;
	      case 2: cout << "current heightmap width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
	              cout << "current heightmap height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
	              break;
	      case 3: cout << "current metalmap width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
                  cout << "current metalmap height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
                  break;
	      case 4: cout << "current feature map width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
                  cout << "current feature map height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
                  break;
	      case 5: cout << "current feature map width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
                  cout << "current feature map height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
                  break;
	      case 6: cout << "current typemap width:" << ilGetInteger(IL_IMAGE_WIDTH) << endl; 
	              cout << "current typemap height:" << ilGetInteger(IL_IMAGE_HEIGHT) << endl;
	              break;
	 }              
	}
	
	xsize = ilGetInteger(IL_IMAGE_WIDTH);
	ysize = ilGetInteger(IL_IMAGE_HEIGHT);
	
	if (geotherm == false){
	 if (tex == true){
		if (!(x == 1)) 
	       xsize = x*512;
	    if (!(y == 1))   
		   ysize = y*512;
	 }
	 if (tex == false){
	   if (typemap == false){	
	      if (add == true){	
	  	     if (!(x == 1))   
	            xsize = (x*64) + 1;
	 	     if (!(y == 1))      
	  	        ysize = (y*64) + 1;
	      }   
	      if (add == false){
	  	     if (!(x == 1))    
	            xsize = x*64; 
	 	     if (!(y == 1))   
	  	        ysize = y*64;
	      }
	   }
	   if (typemap == true){
		  if (!(y == 1))   
		     xsize = x*32;
		  if (!(y == 1))   
		     ysize = y*32;  
	   }
	 }
	}
	 
	mem = new unsigned char[xsize * ysize * 4];
	//ilCopyPixels(0,0,0,xsize,ysize,1,IL_RGBA,IL_UNSIGNED_BYTE,mem) ; 
	memcpy(mem, ilGetData(),xsize * ysize * 4);
		
	ilDeleteImages(1, &ImageName); 

	if(noAlpha){
		for(int y=0;y<ysize;++y){
			for(int x=0;x<xsize;++x){
			mem[(y*xsize+x)*4+3]=defaultAlpha;
			}
		}
	}
}

void CBitmap::Save(string const& filename,int quality)
{
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	ilEnable(IL_FILE_OVERWRITE);

	unsigned char* buf=new unsigned char[xsize*ysize*4];
	/* HACK Flip the image so it saves the right way up.
		(Fiddling with ilOriginFunc didn't do anything?)/
		Duplicated with ReverseYAxis.  */
	for(int y=0;y<ysize;++y){
		for(int x=0;x<xsize;++x){
			buf[((ysize-1-y)*xsize+x)*4+0]=mem[((y)*xsize+x)*4+0];
			buf[((ysize-1-y)*xsize+x)*4+1]=mem[((y)*xsize+x)*4+1];
			buf[((ysize-1-y)*xsize+x)*4+2]=mem[((y)*xsize+x)*4+2];
			buf[((ysize-1-y)*xsize+x)*4+3]=mem[((y)*xsize+x)*4+3];
		}
	}

	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger (IL_JPG_QUALITY, quality);

	ILuint ImageName = 0;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);

	ilTexImage(xsize,ysize,1,4,IL_RGBA,IL_UNSIGNED_BYTE,NULL);
	ilSetData(buf);
	ilSaveImage((char*)filename.c_str());
	ilDeleteImages(1,&ImageName);
	delete[] buf;
}


// Depreciated (Only used by GUI which will be replaced by CEGUI anyway)
void CBitmap::SetTransparent( unsigned char red, unsigned char green, unsigned char blue )
{
	for ( int y = 0; y < xsize; y++ )
	{
		for ( int x = 0; x < xsize; x++ )
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

CBitmap CBitmap::Rescale(int newx, int newy)
{
	CBitmap bm;

	delete[] bm.mem;
	
	bm.mem=new unsigned char[newx*newy*4];
	
	iluImageParameter(ILU_FILTER,ILU_SCALE_MITCHELL); 
	iluScale(newx,newy,1); 
	
	bm.xsize=ilGetInteger(IL_IMAGE_WIDTH);
	bm.ysize=ilGetInteger(IL_IMAGE_HEIGHT);

	memcpy(mem, ilGetData(),xsize * ysize * 4);

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

#define RM	0x0000F800
#define GM  0x000007E0
#define BM  0x0000001F

#define RED_RGB565(x) ((x&RM)>>11)
#define GREEN_RGB565(x) ((x&GM)>>5)
#define BLUE_RGB565(x) (x&BM)
#define PACKRGB(r, g, b) (((r<<11)&RM) | ((g << 5)&GM) | (b&BM) )

void CBitmap::CreateFromDXT1(unsigned char* buf, int xsize, int ysize)
{
	delete[] mem;

	mem=new unsigned char[xsize*ysize*4];
	memset(mem,0,xsize*ysize*4);

	this->xsize=xsize;
	this->ysize=ysize;

	int numblocks = ((xsize+3)/4)*((ysize+3)/4);

	unsigned char* temp = buf;

	for ( int i = 0; i < numblocks; i++ ){

		unsigned short color0 = (*(unsigned short*)&temp[0]);
		unsigned short color1 = (*(unsigned short*)&temp[2]);

		int r0=RED_RGB565(color0)<<3;
		int g0=GREEN_RGB565(color0)<<2;
		int b0=BLUE_RGB565(color0)<<3;

		int r1=RED_RGB565(color1)<<3;
		int g1=GREEN_RGB565(color1)<<2;
		int b1=BLUE_RGB565(color1)<<3;

		unsigned int bits = (*(unsigned int*)&temp[4]);
		
		for ( int a = 0; a < 4; a++ )
		{
			for ( int b = 0; b < 4; b++ )
			{
				int x = 4*(i % ((xsize+3)/4))+b;
				int y = 4*(i / ((xsize+3)/4))+a;
				bits >>= 2;
				unsigned char code = bits & 0x3;
				
				if ( color0 > color1 )
				{
					if ( code == 0 )
					{
						mem[(y*xsize+x)*4+0]=r0;
						mem[(y*xsize+x)*4+1]=g0;
						mem[(y*xsize+x)*4+2]=b0;
					}
					else if ( code == 1 )
					{
						mem[(y*xsize+x)*4+0]=r1;
						mem[(y*xsize+x)*4+1]=g1;
						mem[(y*xsize+x)*4+2]=b1;
					}
					else if ( code == 2 )
					{
						mem[(y*xsize+x)*4+0]=(r0*2+r1)/3;
						mem[(y*xsize+x)*4+1]=(g0*2+g1)/3;
						mem[(y*xsize+x)*4+2]=(b0*2+b1)/3;
					}
					else
					{	
						mem[(y*xsize+x)*4+0]=(r0+r1*2)/3;
						mem[(y*xsize+x)*4+1]=(g0+g1*2)/3;
						mem[(y*xsize+x)*4+2]=(b0+b1*2)/3;
					}
				}
				else
				{
					if ( code == 0 )
					{
						mem[(y*xsize+x)*4+0]=r0;
						mem[(y*xsize+x)*4+1]=g0;
						mem[(y*xsize+x)*4+2]=b0;
					}
					else if ( code == 1 )
					{
						mem[(y*xsize+x)*4+0]=r1;
						mem[(y*xsize+x)*4+1]=g1;
						mem[(y*xsize+x)*4+2]=b1;
					}
					else if ( code == 2 )
					{
						mem[(y*xsize+x)*4+0]=(r0+r1)/2;
						mem[(y*xsize+x)*4+1]=(g0+g1)/2;
						mem[(y*xsize+x)*4+2]=(b0+b1)/2;
					}
					else
					{	
						mem[(y*xsize+x)*4+0]=0;
						mem[(y*xsize+x)*4+1]=0;
						mem[(y*xsize+x)*4+2]=0;
					}
				}
			}
		}
		temp += 8;
	}
}
