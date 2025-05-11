
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

#ifndef _PNG_H_
#define _PNG_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines, structures

extern int PNG_Opacity;
extern int PNG_Color;
extern int PNG_r;
extern int PNG_g;
extern int PNG_b;

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

struct IMAGE_DATAS
{
	char * ptrImageDATAS;	// ptr
	unsigned int X,Y;		// dimension
	int	nTexture;			// gestion dynamique...
	int Alpha;				// 0-1
	int Grayscale;
};

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fonctions principales

void READ_PNG(char * nomPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha);
void READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha);
#if defined(_DEFINES_OPENGL_NO_SDL_INIT_)&&!defined(API3D_VR)
void _READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha,int Opacity);
#else
void READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha,int Opacity);
#endif

void SAVE_PNG(char * nomPNG,struct IMAGE_DATAS * im,int filter);
void QUICKSAVE_PNG(char * nomPNG,struct IMAGE_DATAS * im,int filter);
void COMPRESS_PNG(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest);
void COMPRESS_PNG_ALPHA(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest);
void COMPRESS_PNG_GRAYSCALE(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest);

int ALPHA_READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,int Opacity);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	FILTER_NONE			=	0,
	FILTER_SUB			=	1,
	FILTER_UP			=	2,
	FILTER_AVERAGE		=	3
};

#endif
