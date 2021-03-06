/* Author: Teake Nutma */

#include "StdAfx.h"
#include "GlobalStuff.h"
#include "LogOutput.h"
#include "IconHandler.h"
#include "TdfParser.h"
#include "Rendering/Textures/Bitmap.h"
#include "mmgr.h"
#include <algorithm>
#include <assert.h>
#include <locale>
#include <cctype>


CIconHandler* iconHandler;


CIconHandler::CIconHandler()
{
	standardTextureGenerated = false;
	
	LoadTDFicons("usericons.tdf"); 
	LoadTDFicons("gamedata/icontypes.tdf");

	// If the default icon doesn't exist we'll have to create one
	// (as unitdef->iconType defaults to "default").
	if (icons.find("default") == icons.end()) {
		icons["default"] = SAFE_NEW CIcon(*GetStandardTexture(), 1, 1, false);
	}
}


bool CIconHandler::LoadTDFicons(const std::string& filename)
{
	try {
		PrintLoadMsg("Parsing unit icons");
		TdfParser tdfparser(filename);
		std::vector<std::string> iconList = tdfparser.GetSectionList("icontypes");
		CBitmap bitmap;

		for (std::vector<std::string>::const_iterator it = iconList.begin(); it != iconList.end(); ++it) {
			if (icons.find(*it) != icons.end()) {
				continue; // first come, first served  (also avoids possible memory leaks)
			}
			//Parse the bitmap location, the size, and the unit radius adjustment.
			float size=atof(tdfparser.SGetValueDef("1", "icontypes\\" + *it + "\\size").c_str());
			float distance=atof(tdfparser.SGetValueDef("1", "icontypes\\" + *it + "\\distance").c_str());
			bool radiusAdjust=!!atoi(tdfparser.SGetValueDef("0", "icontypes\\" + *it + "\\radiusadjust").c_str());
			// If we can't load the bitmap replace it with the default one.
			std::string bitmapLocation=tdfparser.SGetValueDef("", "icontypes\\" + *it + "\\bitmap");
			unsigned int texture;
			if(!bitmapLocation.empty () && bitmap.Load(bitmapLocation)){
				texture = bitmap.CreateTexture(true);
				glBindTexture(GL_TEXTURE_2D, texture);
				if (GLEW_EXT_texture_edge_clamp) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				} else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				}
			} else {
				texture = *GetStandardTexture();
			}
			icons[*it] = SAFE_NEW CIcon(texture,size,distance,radiusAdjust);
		}
	}
	catch (const TdfParser::parse_error& e) {
		// Show parse errors in the infolog.
		logOutput.Print("%s:%d: %s", e.get_filename().c_str(), e.get_line(), e.what());
	}
	catch (const content_error& e) {
		// Ignore non-existant file.
	}

	return true;
}


CIconHandler::~CIconHandler()
{
}


CIcon * CIconHandler::GetIcon(const std::string& iconName)
{
	std::map<std::string, CIcon*>::const_iterator it=icons.find(iconName);
	if(it==icons.end()){
		return icons["default"];
	} else {
		return it->second;
	}
}


float CIconHandler::GetDistance(const std::string& iconName)
{
	std::map<std::string, CIcon*>::const_iterator it=icons.find(iconName);
	if(it==icons.end()){
		return 1;
	} else {
		return it->second->distance;
	}
}


unsigned int *CIconHandler::GetStandardTexture()
{
	if(!standardTextureGenerated){
		unsigned char si[128*128*4];
		for(int y=0;y<128;++y){
			for(int x=0;x<128;++x){
				float r=sqrtf((y-64)*(y-64)+(x-64)*(x-64))/64.0f;
				if(r>1){
					si[(y*128+x)*4+0]=0;
					si[(y*128+x)*4+1]=0;
					si[(y*128+x)*4+2]=0;
					si[(y*128+x)*4+3]=0;
				} else {
					si[(y*128+x)*4+0]=(unsigned char)(255-r*r*r*255);
					si[(y*128+x)*4+1]=(unsigned char)(255-r*r*r*255);
					si[(y*128+x)*4+2]=(unsigned char)(255-r*r*r*255);
					si[(y*128+x)*4+3]=255;
				}
			}
		}
		CBitmap standardIcon(si,128,128);
		standardTexture=standardIcon.CreateTexture(false);
		standardTextureGenerated=true;

		glBindTexture(GL_TEXTURE_2D, standardTexture);
		if (GLEW_EXT_texture_edge_clamp) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
	}
	return &standardTexture;
}
