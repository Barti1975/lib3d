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

#ifndef _TGA_H_
#define _TGA_H_

	extern int TGA_Opacity;
	extern int TGA_Color;
	extern int TGA_r;
	extern int TGA_g;
	extern int TGA_b;

	#define	C_RED 0
	#define C_GREEN 1
	#define C_BLUE 2
	#define C_ALPHA 3
	#define C_GRAYSCALE 4

	#define MINIMUM 0
	#define MAXIMUM 1

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	#ifndef _DEFINE_IMAGE_DATAS_STRUCT_
	#define _DEFINE_IMAGE_DATAS_STRUCT_

		#define ALPHA1 0
		#define ALPHA0 1
		#define ALPHAINT 2
		#define ALPHATRANSPARENT0 3
		#define ALPHATRANSPARENT1 4
		#define ALPHATRANSPARENT2 5
		#define ALPHACHOISI 6
		#define ALPHACOLORKEY 7


	#endif


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void CNV_COMPONENT(struct IMAGE_DATAS *im,int cp);
	void CNV_SQUARED(struct IMAGE_DATAS *im,int flags);
	void READ_TGA(char * name,struct IMAGE_DATAS *im,unsigned char alpha);
	void READ_TGA_FROM_MEM(char * buf,int size,struct IMAGE_DATAS *im,unsigned char alpha);
	void SAVE_TGA(char * name,struct IMAGE_DATAS *im);

	void TGA_SetNonNativeReadWrite();
	void TGA_SetNativeReadWrite();


#endif
