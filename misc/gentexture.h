
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Cancé Francis, 10/08/1975>
//	  laurent.francis.cance@outlook.fr
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "../base/params.h"

#ifdef _DEFINES_API_CODE_GENTEXTURE_

class API3D_STUB CTextureGeneration
{
public:
	unsigned char *image;
	int w,h;
	struct IMAGE_DATAS mask;
	char mask_name[512];

	CTextureGeneration() { w=h=0; image=0; }

	char * getImage() { return (char*) image; }
	int getWidth() { return w; }
	int getHeight() { return h; }
	void readScript(char * script,char * image1,char * image2,char * image3,char * image4);
	void readScript(char * script,char * image1,char * image2,char * image3) { readScript(script,image1,image2,image3,NULL); }
	void readScript(char * script,char * image1,char * image2) { readScript(script,image1,image2,NULL,NULL); }
	void readScript(char * script,char * image1) { readScript(script,image1,NULL,NULL,NULL); }
	void readScript(char * script) { readScript(script,NULL,NULL,NULL,NULL); }
	void smooth(unsigned char * ptr,int sx,int sy);
	char * loadScript(const char *name);
	struct IMAGE_DATAS * getMask();
	char *getMaskName();
};
#endif
