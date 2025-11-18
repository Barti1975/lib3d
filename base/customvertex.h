
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


#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)

#pragma warning (disable:4359)

#define FLOAT float
#define DWORD unsigned int

#endif

__declspec( align( 1 ) ) struct FVFFLAT
{
	FLOAT x,y,z,rhw;
	DWORD color;
};

#define D3DFVF_FVFFLAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
__declspec( align( 1 ) ) struct CUSTOMVERTEX0
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm,ym;
};
#else
__declspec( align( 1 ) ) struct CUSTOMVERTEX0
{
	FLOAT x,y,z,rhw;
	DWORD color;
	FLOAT xm,ym;
};

#define D3DFVF_CUSTOMVERTEX0 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)

__declspec( align( 1 ) ) struct CUSTOMVERTEXBLUR
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
	FLOAT xm3,ym3;
};

#else
__declspec( align( 1 ) ) struct CUSTOMVERTEXBLUR
{
	FLOAT x,y,z,rhw;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
	FLOAT xm3,ym3;
};

#define D3DFVF_CUSTOMVERTEXBLUR (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX4)
#endif

__declspec( align( 1 ) ) struct CUSTOMVERTEX01
{
	FLOAT x,y,z,rhw;
	DWORD color;
};

#define D3DFVF_CUSTOMVERTEX01 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

__declspec( align( 1 ) ) struct CUSTOMVERTEX02
{
	FLOAT x,y,z,rhw;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
};

#define D3DFVF_CUSTOMVERTEX02 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2)


__declspec( align( 1 ) ) struct CUSTOMVERTEX1
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm,ym;
};

#define D3DFVF_CUSTOMVERTEX1 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)


__declspec( align( 1 ) ) struct CUSTOMVERTEX2
{
	FLOAT x,y,z;
	DWORD color;
};

#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZ | D3DFVF_DIFFUSE)




__declspec( align( 1 ) ) struct XYZTEX
{
	FLOAT x,y,z;
	FLOAT xm,ym;
};

#define D3DFVF_XYZTEX (D3DFVF_XYZ | D3DFVF_TEX1)


__declspec( align( 1 ) ) struct XYZCOLOR
{
	FLOAT x,y,z;
	DWORD color;
};

#define D3DFVF_XYZCOLOR (D3DFVF_XYZ | D3DFVF_DIFFUSE)


__declspec( align( 1 ) ) struct XYZCOLORTEX
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm,ym;
};

#define D3DFVF_XYZCOLORTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)


__declspec( align( 1 ) ) struct XYZCOLOR2TEX
{
	FLOAT x,y,z;
	DWORD color;
	DWORD color2;
	FLOAT xm,ym;
};

#define D3DFVF_XYZCOLOR2TEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)


__declspec( align( 1 ) ) struct XYZCOLORTEX2
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
};

#define D3DFVF_XYZCOLORTEX2 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)


__declspec( align( 1 ) ) struct XYZCOLORTEX3
{
	FLOAT x,y,z;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
};

#define D3DFVF_XYZCOLORTEX3 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3)


__declspec( align( 1 ) ) struct XYZNORMALCOLORTEX4
{
	FLOAT x,y,z;
	FLOAT Nx,Ny,Nz;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
	FLOAT xm3,ym3;
};

#define D3DFVF_XYZNORMALCOLORTEX4 (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX4)




__declspec( align( 1 ) ) struct XYZNORMALTEX
{
	FLOAT x,y,z;
	FLOAT Nx,Ny,Nz;
	FLOAT xm,ym;
};
#define D3DFVF_XYZNORMALTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)




__declspec( align( 1 ) ) struct XYZNORMALTEX2
{
	FLOAT x,y,z;
	FLOAT Nx,Ny,Nz;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
};

#define D3DFVF_XYZNORMALTEX2 (D3DFVF_XYZ | D3DFVF_DIFFUSE| D3DFVF_NORMAL | D3DFVF_TEX2)

//--------------------------------------------------------------------------------------------------------- BLENDING WEIGHTS

__declspec( align( 1 ) ) struct BLENDXYZTEX
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	FLOAT xm,ym;
};

#define D3DFVF_BLENDXYZTEX (D3DFVF_XYZB5 | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4 )


__declspec( align( 1 ) ) struct BLENDXYZCOLOR
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	DWORD color;
};

#define D3DFVF_BLENDXYZCOLOR (D3DFVF_XYZB5 | D3DFVF_DIFFUSE| D3DFVF_LASTBETA_UBYTE4)


__declspec( align( 1 ) ) struct BLENDXYZCOLORTEX
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	DWORD color;
	FLOAT xm,ym;
};

#define D3DFVF_BLENDXYZCOLORTEX (D3DFVF_XYZB5 | D3DFVF_DIFFUSE | D3DFVF_TEX1| D3DFVF_LASTBETA_UBYTE4)


__declspec( align( 1 ) ) struct BLENDXYZCOLORTEX2
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
};

#define D3DFVF_BLENDXYZCOLORTEX2 (D3DFVF_XYZB5 | D3DFVF_DIFFUSE | D3DFVF_TEX2| D3DFVF_LASTBETA_UBYTE4)


__declspec( align( 1 ) ) struct BLENDXYZNORMALTEX
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	FLOAT Nx,Ny,Nz;
	FLOAT xm,ym;
};
#define D3DFVF_BLENDXYZNORMALTEX (D3DFVF_XYZB5 | D3DFVF_NORMAL | D3DFVF_TEX1| D3DFVF_LASTBETA_UBYTE4)

__declspec( align( 1 ) ) struct BLENDXYZCOLORTEX3
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
};

#define D3DFVF_BLENDXYZCOLORTEX3 (D3DFVF_XYZB5 | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_LASTBETA_UBYTE4)


__declspec( align( 1 ) ) struct BLENDXYZNORMALCOLORTEX4
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	FLOAT Nx,Ny,Nz;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
	FLOAT xm2,ym2;
	FLOAT xm3,ym3;
};

#define D3DFVF_BLENDXYZNORMALCOLORTEX4 (D3DFVF_XYZB5 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX4 | D3DFVF_LASTBETA_UBYTE4)


__declspec( align( 1 ) ) struct BLENDXYZNORMALTEX2
{
	FLOAT x,y,z;
	FLOAT blend1,blend2,blend3,blend4;
#if defined(API3D_DIRECT3D9)||defined(API3D_DIRECT3D)
	DWORD indices;
#endif
	FLOAT Nx,Ny,Nz;
	DWORD color;
	FLOAT xm0,ym0;
	FLOAT xm1,ym1;
};

#define D3DFVF_BLENDXYZNORMALTEX2 (D3DFVF_XYZB5 | D3DFVF_DIFFUSE| D3DFVF_NORMAL | D3DFVF_TEX2| D3DFVF_LASTBETA_UBYTE4)
