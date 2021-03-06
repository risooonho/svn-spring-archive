#ifndef TERRAIN_TEXTURES_H
#define TERRAIN_TEXTURES_H

class TdfParser;
#include "Rendering/Textures.h"

namespace terrain {

	struct ILoadCallback;
	struct Heightmap;
	class TQuad;

	struct float2
	{
		float2() { x=y=0.0f; }
		float2(float X,float Y) : x(X), y(Y) {}

		float x,y;
	};


//-----------------------------------------------------------------------
// AlphaImage - a single channel image class, used for storing blendmaps
//-----------------------------------------------------------------------

	struct AlphaImage {
		AlphaImage ();
		~AlphaImage ();

		enum AreaTestResult {
			AREA_ONE, AREA_ZERO, AREA_MIXED 
		};

		void Alloc (int W,int H);
		float& at(int x,int y) { return data[y*w+x]; }
		bool CopyFromBitmap(const MemTexture &bm);
		AlphaImage* CreateMipmap ();
		void Blit (AlphaImage *dst, int x,int y,int dstx, int dsty, int w ,int h);
		bool Save (const char *fn);
		AreaTestResult TestArea (int xstart,int ystart,int xend,int yend);

		int w,h;
		float *data;

		AlphaImage *lowDetail, *highDetail; // geomipmap chain links
	};


//-----------------------------------------------------------------------
// BaseTexture
//-----------------------------------------------------------------------

	struct BaseTexture
	{
		BaseTexture ();
		virtual ~BaseTexture() {};
		virtual BaseTexture* GetBumpMap() { return 0; }
		virtual BaseTexture* GetSpecularMap() { return 0; }

		virtual bool ShareTexCoordUnit () { return true; }

		std::string name;
		Texture id;
		float2 tilesize;

		// the "texgen vector" is defined as:
		// { Xfactor, Zfactor, Xoffset, Zoffset }
		virtual void CalcTexGenVector (float *v4);
		void SetupTexGen();
	};

	struct TiledTexture : public BaseTexture
	{
		TiledTexture ();
		~TiledTexture ();

		BaseTexture* GetBumpMap () { return bumpMap; }
		void Load(const std::string& name, const std::string& section, ILoadCallback *cb, TdfParser *tdf);

		std::string bumpMapName;
		BaseTexture* bumpMap;
	};

    struct Blendmap : public BaseTexture
	{
		Blendmap ();
		~Blendmap ();

		void Generate (Heightmap *rootHm, int lodLevel);
		void Load(const std::string& name, const std::string& section, Heightmap *heightmap, ILoadCallback *cb, TdfParser *tdf);

		struct GeneratorInfo {
			float coverage, noise;
			float minSlope, maxSlope;
			float minSlopeFuzzy, maxSlopeFuzzy;
			float minHeight, maxHeight;
			float minHeightFuzzy, maxHeightFuzzy;
		};
		GeneratorInfo *generatorInfo;
		AlphaImage *image;  // WIP image pointer

		AlphaImage::AreaTestResult curAreaResult;
	};

	// A bumpmap generated from a heightmap, used to increase detail on distant nodes
	struct DetailBumpmap : public BaseTexture
	{
		~DetailBumpmap();
		void CalcTexGenVector (float *v4);
		bool ShareTexCoordUnit () { return false; }

		TQuad *node;
	};

	Texture LoadTexture (const std::string& fn, bool isBumpmap=false);
	void SaveImage(const char *fn, int components, GLenum type, int w,int h, void *data);

};


#endif
