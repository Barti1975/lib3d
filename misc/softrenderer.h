
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

#ifndef _SOFTRENDERER_H_
#define _SOFTRENDERER_H_

#include "../base/params.h"

#ifdef _DEFINES_API_CODE_SOFTRENDERER_

#include <stdio.h>
#include "../base/maths.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	unsigned int value;
	float z;

} SRpixel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	CSR_ZBUFFER			=	1,

	CSR_NORMAL			=	0,
	CSR_INVERSE			=	1,
	CSR_DISABLED		=	2,

	CSR_FILL_TEX0		=	1,
	CSR_FILL_TEX1		=	2,
	CSR_FILL_TEX2		=	4,
	CSR_FILL_COLOR		=	8,
	CSR_FILL_Z			=	16,	// zbuffer rendering
	CSR_FILL_ALPHA		=	32,  // alpha component for shadowing
    
    CSR_FILL_BLEND      =   64,


	CSR_FILL_DZ			=   128,

	CSR_FILL_GOURAUD    =   256
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SR_COLOR(r,g,b) ((r<<16)+(g<<8)+b)
#define SR_COLORf(r,g,b) ((((int)(255*r))<<16)+(((int)(255*g))<<8)+((int)(255*b)))
#define SR_ALPHA(a) (a<<24)
#define SR_GET_R(rgb) (((float)((rgb>>16)&0xFF))/255.0f)
#define SR_GET_G(rgb) (((float)((rgb>>8)&0xFF))/255.0f)
#define SR_GET_B(rgb) (((float)((rgb)&0xFF))/255.0f)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSoftwareRenderer
{
public:
	SRpixel * surfaces[16];
	SRpixel * surface;
    CVector2 * surfacemap;
	int wx,wy;
	int zfunc;
	float front,back;
	unsigned char * tex_ptr1;
	unsigned char * tex_ptr2;
	unsigned char * tex_ptr3;
	int szx1,szy1;
	int szx2,szy2;
	int szx3,szy3;
	float Zmin,Zmax;

    CSoftwareRenderer() { tex_ptr1=tex_ptr2=tex_ptr3=NULL; surfacemap=NULL; front=0.01f; back=100.0f; wx=wy=0;for (int n=0;n<16;n++) surfaces[n]=NULL;surface=NULL;zfunc=CSR_NORMAL;}

	void SetParams(int q,int c);
	void SetScreen(int x,int y);
	void SetRenderTarget(int n);
	int AddSurface();
	void Free();
	void Clear(int cc,float zz);
	float GetZ(int x,int y);
	float GetZaround(int x,int y);
    CVector2 GetCoo(int x,int y);
	unsigned int GetValue(int x,int y);
	void SetValue(int x,int y,unsigned int c);
	void Fill(int line,int xd0,int xf0,int c,float z);
	void Fill(int line,int xd0,int xf0,int cd,int cf,float zd,float zf,float izd,float izf,CVector2 &t0d,CVector2 &t0f,CVector2 &t1d,CVector2 &t1f,CVector2 &t2d,CVector2 &t2f,int flag);

	void borders();

	////////////////////////////////////////////////////////////////////////

	void Flat(CVector2 &p0,CVector2 &p1,CVector2 &p2,int cc,float z);
	void PrimitiveFlatZ(CVector &p0,CVector &p1,CVector &p2,int cc);
	void PrimitiveFlatMapping(CVector &p0,CVector &p1,CVector &p2,CVector2 &m0,CVector2 &m1,CVector2 &m2,int cc);
    void PrimitiveFlatMappingRGB(CVector &p0,CVector &p1,CVector &p2,CVector2 &m0,CVector2 &m1,CVector2 &m2,int cc);
    void PrimitiveFlatMappingRGBBlend(CVector &p0,CVector &p1,CVector &p2,CVector2 &m0,CVector2 &m1,CVector2 &m2,int cc);
	void PrimitiveFlatMappingRGBBlendDZ(CVector &p0,CVector &p1,CVector &p2,CVector2 &m0,CVector2 &m1,CVector2 &m2,int cc);
	void PrimitiveRGBBlendDZ(CVector &p0,CVector &p1,CVector &p2,CVector &c0,CVector &c1,CVector &c2,int cc);
	void PrimitiveFlatDot(CVector &p,unsigned int color,int size);
    
    void Print(int xc,int yc,char *str,int r,int g,int b);
    void Line(int x1,int y1,int x2,int y2,int r,int g,int b);
};

#endif
#endif
