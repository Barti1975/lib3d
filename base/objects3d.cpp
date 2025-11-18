////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Canc� Francis, 10/08/1975>
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

////////////////////////////////////////////////////////////////////////
//	@file objects3d.cpp
//	@date 2012-12-03
////////////////////////////////////////////////////////////////////////
#include "params.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "triangularisation.h"
#include "edges.h"
#include "bsp_2d.h"

#if !defined(WIN32) && !defined(WINDOWS_PHONE)
#include <sys/types.h>
#include <sys/stat.h>
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intersections
#define MOLLER_TRUMBORE
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAXI_BOUND 10000000
////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void (*fonction_progress_traitment)(float pg)=NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// string with last '/' like "../AppDir/temp/"
char TEMPORARY_DIRECTORY_FOR_FILES_SLEEP_PACK[4096]={ 0 };
int ID_TEMPORARY_FILES=-1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitTemporaryFilesProtocol()
{
#if !defined(WINDOWS_PHONE) && !defined(WEBASM)
    ID_TEMPORARY_FILES=0;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FreeTemporaryFilesProtocol()
{
    ID_TEMPORARY_FILES=-1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TemporaryFilesProtocol()
{
    if ((strlen(TEMPORARY_DIRECTORY_FOR_FILES_SLEEP_PACK)>0)&&(ID_TEMPORARY_FILES>=0)) return true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SMALLFXXK (SMALLF)

float COEF_MUL_OPTIMIZEFROM=1.0f;
float COEF_MUL_OPTIMIZEFROM_LOCAL=1.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool __nanish(float x)
{
    return ((*(unsigned int *)&x) & 0x7fffffff) > 0x7f800000;
    /*
    char ss[64];
    sprintf(ss,"%3.3f",x);
    return (strcmp(ss,"nan")==0);
    /**/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeVB(CVertexBuffer *vb);

void naming(char *dst,char *src)
{
	int n=0;
	while ((n<128)&&(src[n]!='\0')) { dst[n]=src[n]; n++; }
	if (n==128) strcpy(dst,"OBJ");
	else dst[n]='\0';
}

int SIZE_LIGHTMAPS=16;

void CRGBA::Init(float R,float G,float B)
{
	r=R;g=G;b=B;a=1;
}
void CRGBA::Init(float R,float G,float B,float A)
{
	r=R;g=G;b=B;a=A;
}

void CVertex::SetDiffuse(float r,float g,float b,float a)
{
	Diffuse.r=r;
	Diffuse.g=g;
	Diffuse.b=b;
	Diffuse.a=a;
}


void CFace::Init(int n0,int n1,int n2)
{
	v0=n0;v1=n1;v2=n2;
}

void CFace::InitNorm(float x,float y,float z)
{
	Norm.Init(x,y,z);
}

void CFace::SetDiffuse(float r,float g,float b,float a)
{
	//Diffuse.r=r;
	//Diffuse.g=g;
	//Diffuse.b=b;
	//Diffuse.a=a;
}


void CShortVertex::InitStok(float x,float y,float z)
{
	Stok.Init(x,y,z);
}

void CShortVertex::InitNorm(float x,float y,float z)
{
	Norm.Init(x,y,z);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		this set lightmap array size

		void SetLightMapSize(int len)

	Usage:

		  use befor calling all other lightmap functions

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void SetLightMapSize(int len)
{
	SIZE_LIGHTMAPS=len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::Bounds()
{
    boundmini.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    boundmaxi.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

    for (int n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.x>boundmaxi.x) boundmaxi.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y>boundmaxi.y) boundmaxi.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z>boundmaxi.z) boundmaxi.z=Vertices[n].Stok.z;

        if (Vertices[n].Stok.x<boundmini.x) boundmini.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y<boundmini.y) boundmini.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z<boundmini.z) boundmini.z=Vertices[n].Stok.z;       
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector p,float r,CMatrix M)
{
	int n;

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if (p.x+r<mni.x) return false;
	if (p.y+r<mni.y) return false;
	if (p.z+r<mni.z) return false;
	if (p.x-r>mxi.x) return false;
	if (p.y-r>mxi.y) return false;
	if (p.z-r>mxi.z) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector p,CMatrix M)
{
	int n;

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if (p.x<mni.x) return false;
	if (p.y<mni.y) return false;
	if (p.z<mni.z) return false;
	if (p.x>mxi.x) return false;
	if (p.y>mxi.y) return false;
	if (p.z>mxi.z) return false;

	return true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector p,float r)
{
	int n;
	CMatrix M;

	M.Id();
	M.RotationDegre(Rot);
	M.Translation(Coo);

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if (p.x+r<mni.x) return false;
	if (p.y+r<mni.y) return false;
	if (p.z+r<mni.z) return false;
	if (p.x-r>mxi.x) return false;
	if (p.y-r>mxi.y) return false;
	if (p.z-r>mxi.z) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector p)
{
	int n;
	CMatrix M;

	M.Id();
	M.RotationDegre(Rot);
	M.Translation(Coo);

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if (p.x<mni.x) return false;
	if (p.y<mni.y) return false;
	if (p.z<mni.z) return false;
	if (p.x>mxi.x) return false;
	if (p.y>mxi.y) return false;
	if (p.z>mxi.z) return false;

	return true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector A,CVector B,CVector p,CMatrix M)
{
	int n;

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if ((p.x>mni.x)&&(p.y>mni.y)&&(p.z>mni.z)&&(p.x<mxi.x)&&(p.y<mxi.y)&&(p.z<mxi.z)) return true;

	if ((A.x>mni.x)&&(A.y>mni.y)&&(A.z>mni.z)&&(A.x<mxi.x)&&(A.y<mxi.y)&&(A.z<mxi.z)) return true;
	if ((B.x>mni.x)&&(B.y>mni.y)&&(B.z>mni.z)&&(B.x<mxi.x)&&(B.y<mxi.y)&&(B.z<mxi.z)) return true;

	if ((A.x<mni.x)&&(B.x>mxi.x)) return true;
	if ((A.y<mni.y)&&(B.y>mxi.y)) return true;
	if ((A.z<mni.z)&&(B.z>mxi.z)) return true;

	if ((B.x<mni.x)&&(A.x>mxi.x)) return true;
	if ((B.y<mni.y)&&(A.y>mxi.y)) return true;
	if ((B.z<mni.z)&&(A.z>mxi.z)) return true;

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CObject3D::InsideBounds(CVector A,CVector B,CVector p)
{
	int n;
	CMatrix M;

	M.Id();
	M.RotationDegre(Rot);
	M.Translation(Coo);

    CVector cube[8];
    cube[0].x=boundmini.x; cube[0].y=boundmini.y; cube[0].z=boundmini.z;
    cube[1].x=boundmini.x; cube[1].y=boundmini.y; cube[1].z=boundmaxi.z;
    cube[2].x=boundmini.x; cube[2].y=boundmaxi.y; cube[2].z=boundmini.z;
    cube[3].x=boundmini.x; cube[3].y=boundmaxi.y; cube[3].z=boundmaxi.z;
    cube[4].x=boundmaxi.x; cube[4].y=boundmini.y; cube[4].z=boundmini.z;
    cube[5].x=boundmaxi.x; cube[5].y=boundmini.y; cube[5].z=boundmaxi.z;
    cube[6].x=boundmaxi.x; cube[6].y=boundmaxi.y; cube[6].z=boundmini.z;
    cube[7].x=boundmaxi.x; cube[7].y=boundmaxi.y; cube[7].z=boundmaxi.z;	

	CVector mni(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector mxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<8;n++)
	{
		CVector v=cube[n]*M;
		if (v.x<mni.x) mni.x=v.x;
		if (v.y<mni.y) mni.y=v.y;
		if (v.z<mni.z) mni.z=v.z;
		if (v.x>mxi.x) mxi.x=v.x;
		if (v.y>mxi.y) mxi.y=v.y;
		if (v.z>mxi.z) mxi.z=v.z;
	}

	if ((p.x>mni.x)&&(p.y>mni.y)&&(p.z>mni.z)&&(p.x<mxi.x)&&(p.y<mxi.y)&&(p.z<mxi.z)) return true;

	if ((A.x>mni.x)&&(A.y>mni.y)&&(A.z>mni.z)&&(A.x<mxi.x)&&(A.y<mxi.y)&&(A.z<mxi.z)) return true;
	if ((B.x>mni.x)&&(B.y>mni.y)&&(B.z>mni.z)&&(B.x<mxi.x)&&(B.y<mxi.y)&&(B.z<mxi.z)) return true;

	if ((A.x<mni.x)&&(B.x>mxi.x)) return true;
	if ((A.y<mni.y)&&(B.y>mxi.y)) return true;
	if ((A.z<mni.z)&&(B.z>mxi.z)) return true;

	if ((B.x<mni.x)&&(A.x>mxi.x)) return true;
	if ((B.y<mni.y)&&(A.y>mxi.y)) return true;
	if ((B.z<mni.z)&&(A.z>mxi.z)) return true;

	return false;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		void CObject3D::TagCoplanarFaces2(int nf,int ref,int TAG)

			set .tag2 of all faces coplanar with ref and with tag2==0 to TAG

		void CObject3D::TagCoplanarFaces_nT(int nf,int ref,int TAG)

			set .tag of all faces coplanar with ref, with same .nT and with tag==0 to TAG

		void CObject3D::TagCoplanarFaces(int nf,int ref,int TAG)

			set .tag of all faces coplanar with ref and with tag==0 to TAG

		void CObject3D::TagObjetCoplanarFacesTag(int nf,int ref,int TAG,int BASETAG)

			set .tag2 of all faces coplanar with ref and with tag2==0 and same .tag to TAG

		void CObject3D::TagCoplanarOnlyFaces(int ref,int TAG)

			previous function worked with continuity of faces, this one set all coplanar faces to TAG

		void CObject3D::TagNearToNearFaces(int nf,int ref,int TAG)

		void CObject3D::TagCloseToCloseFaces(int nf,int ref,int TAG)

		void CObject3D::TagCloseToCloseFaces(int nf)

		void CObject3D::TagGroupFaces(int nf,int ref,int TAG)

		void CObject3D::TagHCoplanarFaces(int nf,int ref,int TAG)

		void CObject3D::Tag2CoplanarOnlyFaces(int ref,int TAG)

		void CObject3D::TagCoplanarFacesCool(int nf,int ref,int TAG)

		void CObject3D::TagCoplanarFaces_nTnLTag3(int nf,int ref,int TAG)

	Usage:

		object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::TagCoplanarFaces2(int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;

	if (Faces[nf].tag2==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

		if (ss>1.0f-SMALLF2)
		{
			Faces[nf].tag2=TAG;
			f0=Face_01(nf);
			f1=Face_12(nf);
			f2=Face_20(nf);

			if (f0!=-1) TagCoplanarFaces2(f0,ref,TAG);
			if (f1!=-1) TagCoplanarFaces2(f1,ref,TAG);
			if (f2!=-1) TagCoplanarFaces2(f2,ref,TAG);
		}
	}
}


void CObject3D::TagNearToNearFaces(int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;

	if (Faces[nf].tag2==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

		if (ss>1.0f-SMALLF2)
		{
			Faces[nf].tag2=TAG;
			f0=Face_01(nf);
			f1=Face_12(nf);
			f2=Face_20(nf);

			if (f0!=-1) TagNearToNearFaces(f0,ref,TAG);
			if (f1!=-1) TagNearToNearFaces(f1,ref,TAG);
			if (f2!=-1) TagNearToNearFaces(f2,ref,TAG);
		}
	}
}


void CObject3D::TagCloseToCloseFaces(int nf,int ref,int TAG)
{
	int n;
	CList <int> tags;

	SetF012();

	Faces[nf].tag=TAG;

	for (n=0;n<nFaces;n++)
		if (Faces[n].tag==TAG) tags.Add(n);

	int *pi=tags.GetFirst();
	while (pi)
	{
		n=*pi;

		if (Faces[n].f01>=0)
			if (Faces[Faces[n].f01].tag==0) { Faces[Faces[n].f01].tag=TAG; tags.Add(Faces[n].f01); }

		if (Faces[n].f12>=0)
			if (Faces[Faces[n].f12].tag==0) { Faces[Faces[n].f12].tag=TAG; tags.Add(Faces[n].f12); }

		if (Faces[n].f20>=0)
			if (Faces[Faces[n].f20].tag==0) { Faces[Faces[n].f20].tag=TAG; tags.Add(Faces[n].f20); }

		pi=tags.DeleteAndGetNext();
	}
}

void CObject3D::TagCloseToCloseFaces2(int nf,int ref,int TAG)
{
    if (Faces[nf].tag==0)
    {
        Faces[nf].tag=TAG;
        if (Faces[nf].f01>=0) TagCloseToCloseFaces2(Faces[nf].f01,ref,TAG);
        if (Faces[nf].f12>=0) TagCloseToCloseFaces2(Faces[nf].f12,ref,TAG);
        if (Faces[nf].f20>=0) TagCloseToCloseFaces2(Faces[nf].f20,ref,TAG);
    }
}

void CObject3D::TagCloseToCloseFaces(int nf)
{
	int n;
	CList <int> tags;

	SetF012();

	Faces[nf].tag=1;

	for (n=0;n<nFaces;n++)
		if (Faces[n].tag==1) tags.Add(n);

	int *pi=tags.GetFirst();
	while (pi)
	{
		n=*pi;

		if (Faces[n].f01>=0)
			if (Faces[Faces[n].f01].tag==0) { Faces[Faces[n].f01].tag=1; tags.Add(Faces[n].f01); }

		if (Faces[n].f12>=0)
			if (Faces[Faces[n].f12].tag==0) { Faces[Faces[n].f12].tag=1; tags.Add(Faces[n].f12); }

		if (Faces[n].f20>=0)
			if (Faces[Faces[n].f20].tag==0) { Faces[Faces[n].f20].tag=1; tags.Add(Faces[n].f20); }

		pi=tags.DeleteAndGetNext();
	}
}

void CObject3D::TagGroupFaces(int nf,int ref,int TAG)
{
    int f0,f1,f2;

    if (Faces[nf].tag2==0)
    {
        Faces[nf].tag2=TAG;
        f0=Face_01(nf);
        f1=Face_12(nf);
        f2=Face_20(nf);

        if (f0!=-1) TagNearToNearFaces(f0,ref,TAG);
        if (f1!=-1) TagNearToNearFaces(f1,ref,TAG);
        if (f2!=-1) TagNearToNearFaces(f2,ref,TAG);
    }
}

void CObject3D::TagHCoplanarFaces(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)
        if ((ss>1.0f-SMALLF)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Face_01(nf);
            f1=Face_12(nf);
            f2=Face_20(nf);
            if (f0!=-1) TagHCoplanarFaces(f0,ref,TAG);
            if (f1!=-1) TagHCoplanarFaces(f1,ref,TAG);
            if (f2!=-1) TagHCoplanarFaces(f2,ref,TAG);
        }
    }
}

void CObject3D::TagSetHCoplanarFaces(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)
        if ((ss>1.0f-SMALLF)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Faces[nf].f01;
			f1=Faces[nf].f12;
			f2=Faces[nf].f20;
            if (f0!=-1) TagSetHCoplanarFaces(f0,ref,TAG);
            if (f1!=-1) TagSetHCoplanarFaces(f1,ref,TAG);
            if (f2!=-1) TagSetHCoplanarFaces(f2,ref,TAG);
        }
    }
}

void CObject3D::TagCoplanarFaces(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

        if ((ss>1.0f-SMALLF2)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Face_01(nf);
            f1=Face_12(nf);
            f2=Face_20(nf);

            if (f0!=-1) TagCoplanarFaces(f0,ref,TAG);
            if (f1!=-1) TagCoplanarFaces(f1,ref,TAG);
            if (f2!=-1) TagCoplanarFaces(f2,ref,TAG);
        }
    }
}

void CObject3D::TagCoplanarFacesSet(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

        if ((ss>1.0f-SMALLF2)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Faces[nf].f01;
            f1=Faces[nf].f12;
            f2=Faces[nf].f20;

            if (f0!=-1) TagCoplanarFacesSet(f0,ref,TAG);
            if (f1!=-1) TagCoplanarFacesSet(f1,ref,TAG);
            if (f2!=-1) TagCoplanarFacesSet(f2,ref,TAG);
        }
    }
}


void CObject3D::TagCoplanarFacesCool(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)
        if ((ss>0.8f)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Face_01(nf);
            f1=Face_12(nf);
            f2=Face_20(nf);

            if (f0!=-1) TagCoplanarFacesCool(f0,ref,TAG);
            if (f1!=-1) TagCoplanarFacesCool(f1,ref,TAG);
            if (f2!=-1) TagCoplanarFacesCool(f2,ref,TAG);
        }
    }
}

void CObject3D::TagCoplanarFacesCool2(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if (Faces[nf].tag==0)
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)
        if ((ss>0.75f)||(VECTORNORM2(Faces[nf].Norm)<0.01f))
        {
            Faces[nf].tag=TAG;
            f0=Faces[nf].f01;
            f1=Faces[nf].f12;
            f2=Faces[nf].f20;

            if (f0!=-1) TagCoplanarFacesCool2(f0,nf,TAG);
            if (f1!=-1) TagCoplanarFacesCool2(f1,nf,TAG);
            if (f2!=-1) TagCoplanarFacesCool2(f2,nf,TAG);
        }
    }
}

void CObject3D::TagCoplanarFaces_nTnLTag3(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if ((Faces[nf].tag==0)&&(Faces[nf].nT==Faces[ref].nT)&&(Faces[nf].nL==Faces[ref].nL)&&(Faces[nf].tag3==Faces[ref].tag3))
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

        if (ss>1.0f-SMALLF2*0.1f)
        {
            Faces[nf].tag=TAG;
            f0=Face_01(nf);
            f1=Face_12(nf);
            f2=Face_20(nf);

            if (f0!=-1) TagCoplanarFaces_nTnLTag3(f0,ref,TAG);
            if (f1!=-1) TagCoplanarFaces_nTnLTag3(f1,ref,TAG);
            if (f2!=-1) TagCoplanarFaces_nTnLTag3(f2,ref,TAG);
        }
    }
}

void CObject3D::TagCoplanarFaces_nT(int nf,int ref,int TAG)
{
    float ss;
    int f0,f1,f2;

    if ((Faces[nf].tag==0)&&(Faces[nf].nT==Faces[ref].nT))
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

        if (ss>1.0f-SMALLF2)
        {
            Faces[nf].tag=TAG;
            f0=Face_01(nf);
            f1=Face_12(nf);
            f2=Face_20(nf);

            if (f0!=-1) TagCoplanarFaces_nT(f0,ref,TAG);
            if (f1!=-1) TagCoplanarFaces_nT(f1,ref,TAG);
            if (f2!=-1) TagCoplanarFaces_nT(f2,ref,TAG);
        }
    }
}

void CObject3D::TagObjetCoplanarFacesTag(int nf,int ref,int TAG,int BASETAG)
{
    int f0,f1,f2;
    float ss;

    if ((Faces[nf].tag2==0)&&(Faces[nf].tag==BASETAG))
    {
        DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)
        if ((ss>1.0f-SMALLF*COEF_MUL_OPTIMIZEFROM)||(Faces[nf].coef1<0.01f))
        {
            if (minnf>nf) minnf=nf;
            if (maxnf<nf) maxnf=nf;

            Faces[nf].tag2=TAG;
            f0=Faces[nf].f01;
            f1=Faces[nf].f12;
            f2=Faces[nf].f20;
            if (f0!=-1) TagObjetCoplanarFacesTag(f0,ref,TAG,BASETAG);
            if (f1!=-1) TagObjetCoplanarFacesTag(f1,ref,TAG,BASETAG);
            if (f2!=-1) TagObjetCoplanarFacesTag(f2,ref,TAG,BASETAG);
        }
    }
}


void CObject3D::TagCoplanarOnlyFaces(int ref,int TAG)
{
    float ss;
    int n;
    float d1,d2;

    DOTPRODUCT(d2,Faces[ref].Norm,Faces[ref].v[0]->Stok)
    for (n=0;n<nFaces;n++)
    {
        if ((n!=ref)&&(Faces[n].tag==0))
        {
            if (Faces[n].Norm.Norme()>0.5f)
            {
                DOTPRODUCT(ss,Faces[n].Norm,Faces[ref].Norm)
                if (ss>1.0f-SMALLF)
                {
                    DOTPRODUCT(d1,Faces[n].Norm,Faces[n].v[0]->Stok)
                    if (f_abs(d2-d1)<SMALLF2) Faces[n].tag=TAG;
                }
            }
            else
            {
                DOTPRODUCT(d1,Faces[ref].Norm,Faces[n].v[0]->Stok)
                if (f_abs(d2-d1)<SMALLF2) Faces[n].tag=TAG;
            }
        }
    }
    Faces[ref].tag=TAG;
}


void CObject3D::Tag2CoplanarOnlyFaces(int ref,int TAG)
{
    float ss;
    int n;
    float d1,d2;

    DOTPRODUCT(d2,Faces[ref].Norm,Faces[ref].v[0]->Stok)

    for (n=0;n<nFaces;n++)
    {
        if ((n!=ref)&&(Faces[n].tag2==0))
        {
            DOTPRODUCT(ss,Faces[n].Norm,Faces[ref].Norm)

            if (ss>1.0f-SMALLF)
            {
                DOTPRODUCT(d1,Faces[n].Norm,Faces[n].v[0]->Stok)
                if (f_abs(d2-d1)<SMALLF)
                {
                    Faces[n].tag2=TAG;
                }
            }
        }
    }
    Faces[ref].tag2=TAG;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:



    Usage:

        tag faces

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::TagGroupCloseToClose()
{
    int n,nn;
    int tag=1;
    
    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    
    bool end=false;
    while (!end)
    {
        nn=-1;
        n=0;
        while ((n<nFaces)&&(nn<0))
        {
            if (Faces[n].tag==0) nn=n;
            n++;
        }
        
        if (nn==-1) end=true;
        else
        {
            TagCloseToCloseFaces2(nn,nn,tag);
            tag++;
        }
    }
    
    return tag;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:



    Usage:

        object mapp coord traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CObject3D::ChecFaceMapCoord(int nf,CVector2 &map1,CVector2 &map0)
{
    CVector2 u;
    
    u=map0-Faces[nf].v[0]->Map;
    if (u.Norme()<SMALLF)
    {
        u=map1-Faces[nf].v[1]->Map;
        if (u.Norme()<SMALLF) return true;
    }

    u=map0-Faces[nf].v[1]->Map;
    if (u.Norme()<SMALLF)
    {
        u=map1-Faces[nf].v[2]->Map;
        if (u.Norme()<SMALLF) return true;
    }

    u=map0-Faces[nf].v[2]->Map;
    if (u.Norme()<SMALLF)
    {
        u=map1-Faces[nf].v[0]->Map;
        if (u.Norme()<SMALLF) return true;
    }
    
    return false;
}

void CObject3D::TagMappingCoordiantesFaces(int nf,int ref,int TAG)
{
    if (Faces[nf].tag==0)
    {
        Faces[nf].tag=TAG;
        
        int f01=Faces[nf].f01;
        int f12=Faces[nf].f12;
        int f20=Faces[nf].f20;
        
        if (f01>=0)
        {
            if (ChecFaceMapCoord(f01,Faces[nf].v[0]->Map,Faces[nf].v[1]->Map)) TagMappingCoordiantesFaces(f01,ref,TAG);
        }
        
        if (f12>=0)
        {
            if (ChecFaceMapCoord(f12,Faces[nf].v[1]->Map,Faces[nf].v[2]->Map)) TagMappingCoordiantesFaces(f12,ref,TAG);
        }
        
        if (f20>=0)
        {
            if (ChecFaceMapCoord(f20,Faces[nf].v[2]->Map,Faces[nf].v[0]->Map)) TagMappingCoordiantesFaces(f20,ref,TAG);
        }
    }
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    void CObject3D::SetWF012()

    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetWF012()
{
    /*
    int n;
    int ofs2=nVertices*2;
    
    CShortyList ** references_face_012Q=new CShortyList*[ofs2];
    
    for (n=0;n<ofs2;n++) references_face_012Q[n]=NULL;
    
    int sz=sizeof(CShortyList);
    int szel=sizeof(CSElement);
    
    newbieInit(3*nFaces*(sz+szel));
    
    for (n=0;n<nFaces;n++)
    {
        Faces[n].i0=Faces[n].v0+Faces[n].v1;
        Faces[n].i1=Faces[n].v1+Faces[n].v2;
        Faces[n].i2=Faces[n].v2+Faces[n].v0;
        
        if (references_face_012Q[Faces[n].i0]==NULL) { references_face_012Q[Faces[n].i0]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i0]->Closing=NULL; references_face_012Q[Faces[n].i0]->obj=this; }
        if (references_face_012Q[Faces[n].i1]==NULL) { references_face_012Q[Faces[n].i1]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i1]->Closing=NULL; references_face_012Q[Faces[n].i1]->obj=this; }
        if (references_face_012Q[Faces[n].i2]==NULL) { references_face_012Q[Faces[n].i2]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i2]->Closing=NULL; references_face_012Q[Faces[n].i2]->obj=this; }
        
        references_face_012Q[Faces[n].i0]->Add(n);
        references_face_012Q[Faces[n].i1]->Add(n);
        references_face_012Q[Faces[n].i2]->Add(n);
    }
    
    for (n=0;n<nFaces;n++)
    {
        Faces[n].f01=Face_01QW(n,references_face_012Q);
        Faces[n].f12=Face_12QW(n,references_face_012Q);
        Faces[n].f20=Face_20QW(n,references_face_012Q);
        
        Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
    }
    
    newbieKill();
    delete [] references_face_012Q;
    /**/
    
    SetF012();
    for (int n=0;n<nFaces;n++) Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    void CObject3D::SetF012Near()
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetF012Near()
{
    int n;

	if (nFaces<=0) return;
    
    f012=true;
    
    for (n=0;n<nFaces;n++) { Faces[n].tag=0; Faces[n].nL=n; }
    
    CObject3D *obj=Duplicate2();
	
	obj->SetF012();

    for (n=0;n<obj->nFaces;n++)
    {
		if ((obj->Faces[n].f01<0)||(obj->Faces[n].f12<0)||(obj->Faces[n].f20<0)) obj->Faces[n].tag=1;
		else obj->Faces[n].tag=0;
	}
    
    CObject3D *res=obj->ConvertLinearFull(SMALLF,100.0f);

    res->SetF012();
    
    for (n=0;n<res->nFaces;n++)
    {
        int nf=res->Faces[n].nL;
        
        if (res->Faces[n].f01>=0) Faces[nf].f01=res->Faces[res->Faces[n].f01].nL;
        else Faces[nf].f01=-1;
        
        if (res->Faces[n].f12>=0) Faces[nf].f12=res->Faces[res->Faces[n].f12].nL;
        else Faces[nf].f12=-1;
        
        if (res->Faces[n].f20>=0) Faces[nf].f20=res->Faces[res->Faces[n].f20].nL;
        else Faces[nf].f20=-1;
    }
    
    res->Free();
    delete res;
    
    obj->Free();
    delete obj;
    
    update=true;
}

void CObject3D::SetF012MapNear()
{
    int n;

	if (nFaces<=0) return;
    
    f012=true;
    
    for (n=0;n<nFaces;n++) { Faces[n].tag=0; Faces[n].nL=n; }
    
    CObject3D *obj=Duplicate2();
    
    CObject3D *res=obj->ConvertLinearFull(SMALLF,SMALLF2);

    res->SetF012();
    
    for (n=0;n<res->nFaces;n++)
    {
        int nf=res->Faces[n].nL;
        
        if (res->Faces[n].f01>=0) Faces[nf].f01=res->Faces[res->Faces[n].f01].nL;
        else Faces[nf].f01=-1;
        
        if (res->Faces[n].f12>=0) Faces[nf].f12=res->Faces[res->Faces[n].f12].nL;
        else Faces[nf].f12=-1;
        
        if (res->Faces[n].f20>=0) Faces[nf].f20=res->Faces[res->Faces[n].f20].nL;
        else Faces[nf].f20=-1;
    }
    
    res->Free();
    delete res;
    
    obj->Free();
    delete obj;
    
    update=true;
}


void CObject3D::SetF012NearInv()
{
    int n;

	if (nFaces<=0) return;
    
    f012=true;
    
    for (n=0;n<nFaces;n++) { Faces[n].tag=0; Faces[n].nL=n; }
    
    CObject3D *obj=Duplicate2();
    
    CObject3D *res=obj->ConvertLinearFull(SMALLF,100.0f);

    res->SetF012Inv();
    
    for (n=0;n<res->nFaces;n++)
    {
        int nf=res->Faces[n].nL;
        
        if (res->Faces[n].f01>=0) Faces[nf].f01=res->Faces[res->Faces[n].f01].nL;
        else Faces[nf].f01=-1;
        
        if (res->Faces[n].f12>=0) Faces[nf].f12=res->Faces[res->Faces[n].f12].nL;
        else Faces[nf].f12=-1;
        
        if (res->Faces[n].f20>=0) Faces[nf].f20=res->Faces[res->Faces[n].f20].nL;
        else Faces[nf].f20=-1;
    }
    
    res->Free();
    delete res;
    
    obj->Free();
    delete obj;
    
    update=true;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    void CObject3D::SetF012()

    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetF012()
{
    int n,a,b,nf;

	if (nFaces<=0) return;
    
    f012=true;
    
    CList<int>*corres=new CList<int>[nVertices];

    for (n=0;n<nFaces;n++)
    {
        corres[Faces[n].v0].Add(n);
        corres[Faces[n].v1].Add(n);
        corres[Faces[n].v2].Add(n);
    }
    
    int *pi1,*pi2;
    
    for (n=0;n<nFaces;n++)
    {
        a=Faces[n].v0;
        b=Faces[n].v1;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f01=nf;
        
        a=Faces[n].v1;
        b=Faces[n].v2;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f12=nf;

        a=Faces[n].v2;
        b=Faces[n].v0;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f20=nf;

    }

    for (n=0;n<nVertices;n++) corres[n].Free();
    
    delete [] corres;

    /*
	int n;
	int * references_face_012Q=new int[nVertices*4];
	
	int ofs2=nVertices*2;

	for (n=0;n<ofs2;n++)
	{
		references_face_012Q[n]=nFaces;
		references_face_012Q[ofs2+n]=0;
	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].i0=Faces[n].v0+Faces[n].v1;
		Faces[n].i1=Faces[n].v1+Faces[n].v2;
		Faces[n].i2=Faces[n].v2+Faces[n].v0;

		if (n<references_face_012Q[Faces[n].i0]) references_face_012Q[Faces[n].i0]=n;
		if (n<references_face_012Q[Faces[n].i1]) references_face_012Q[Faces[n].i1]=n;
		if (n<references_face_012Q[Faces[n].i2]) references_face_012Q[Faces[n].i2]=n;

		if (n>references_face_012Q[ofs2+Faces[n].i0]) references_face_012Q[ofs2+Faces[n].i0]=n;
		if (n>references_face_012Q[ofs2+Faces[n].i1]) references_face_012Q[ofs2+Faces[n].i1]=n;
		if (n>references_face_012Q[ofs2+Faces[n].i2]) references_face_012Q[ofs2+Faces[n].i2]=n;
	}
	
	for (n=0;n<nFaces;n++)
	{
		Faces[n].f01=Face_01Q(n,references_face_012Q);
		Faces[n].f12=Face_12Q(n,references_face_012Q);
		Faces[n].f20=Face_20Q(n,references_face_012Q);
	}

	delete [] references_face_012Q;
    /**/
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		F012

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetF012Inv()
{
    int n,a,b,nf;

	if (nFaces<=0) return;
    
    f012=true;
    
    CList<int>*corres=new CList<int>[nVertices];

    for (n=0;n<nFaces;n++)
    {
        corres[Faces[n].v0].Add(n);
        corres[Faces[n].v1].Add(n);
        corres[Faces[n].v2].Add(n);
    }
    
    int *pi1,*pi2;
    
    for (n=0;n<nFaces;n++)
    {
        a=Faces[n].v0;
        b=Faces[n].v1;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f01=nf;
		if (nf<0)
		{
			a=Faces[n].v1;
			b=Faces[n].v0;
        
			pi1=corres[a].GetFirst();
			while ((pi1)&&(nf<0))
			{
				pi2=corres[b].GetFirst2();
				while ((pi2)&&(nf<0))
				{
					if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
					pi2=corres[b].GetNext2();
				}
				pi1=corres[a].GetNext();
			}

			Faces[n].f01=nf;
		}
        
        a=Faces[n].v1;
        b=Faces[n].v2;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f12=nf;
		if (nf<0)
		{
			a=Faces[n].v2;
			b=Faces[n].v1;
        
			pi1=corres[a].GetFirst();
			while ((pi1)&&(nf<0))
			{
				pi2=corres[b].GetFirst2();
				while ((pi2)&&(nf<0))
				{
					if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
					pi2=corres[b].GetNext2();
				}
				pi1=corres[a].GetNext();
			}

			Faces[n].f12=nf;
		}

        a=Faces[n].v2;
        b=Faces[n].v0;
        
        nf=-1;
        pi1=corres[a].GetFirst();
        while ((pi1)&&(nf<0))
        {
            pi2=corres[b].GetFirst2();
            while ((pi2)&&(nf<0))
            {
                if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
                pi2=corres[b].GetNext2();
            }
            pi1=corres[a].GetNext();
        }
        
        Faces[n].f20=nf;
		if (nf<0)
		{
			a=Faces[n].v0;
			b=Faces[n].v2;
        
			pi1=corres[a].GetFirst();
			while ((pi1)&&(nf<0))
			{
				pi2=corres[b].GetFirst2();
				while ((pi2)&&(nf<0))
				{
					if ((*pi1!=n)&&(*pi1==*pi2)) nf=*pi1;
					pi2=corres[b].GetNext2();
				}
				pi1=corres[a].GetNext();
			}

			Faces[n].f20=nf;
		}

    }

    for (n=0;n<nVertices;n++) corres[n].Free();
    
    delete [] corres;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		F012

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

class followings
{
public:
	int nb;
	int ab[8];
	followings() { nb=0; }

	void add(int nf)
	{
		if (nb<8) ab[nb++]=nf;
	}
};

void CObject3D::SetF012Cull(float val)
{
    int n,a,b,nf,k,ed;
	CVector m,dir;

	f012=true;

    CList<int>*corres=new CList<int>[nVertices];
	followings *linked=new followings[nFaces*3];

    for (n=0;n<nFaces;n++)
    {
		Faces[n].f01=-1;
		Faces[n].f12=-1;
		Faces[n].f20=-1;
        corres[Faces[n].v0].Add(n);
        corres[Faces[n].v1].Add(n);
        corres[Faces[n].v2].Add(n);
    }
    
    int *pi1,*pi2;

    for (n=0;n<nFaces;n++)
    {
		for (ed=0;ed<3;ed++)
		{
			if (ed==0)
			{
				a=Faces[n].v0;
				b=Faces[n].v1;
			}

			if (ed==1)
			{
				a=Faces[n].v1;
				b=Faces[n].v2;
			}

			if (ed==2)
			{
				a=Faces[n].v2;
				b=Faces[n].v0;
			}
		
			pi1=corres[a].GetFirst();
			while (pi1)
			{
				pi2=corres[b].GetFirst2();
				while (pi2)
				{				
					if ((*pi1!=n)&&(*pi1==*pi2)) linked[n*3+ed].add(*pi1);
					pi2=corres[b].GetNext2();
				}
				pi1=corres[a].GetNext();
			}
		}
	}

	for (n=0;n<nFaces;n++)
    {
		for (ed=0;ed<3;ed++)
		{
			int nn=-1;
			if (ed==0) nn=Faces[n].f01;
			if (ed==1) nn=Faces[n].f12;
			if (ed==2) nn=Faces[n].f20;

			if (nn<0)
			{
				nf=-1;
				float smax=val;
				for (k=0;k<linked[n*3+ed].nb;k++)
				{
					float s=DOT(Faces[n].Norm,Faces[linked[n*3+ed].ab[k]].Norm);
					if (s>smax)
					{
						smax=s;
						nf=linked[n*3+ed].ab[k];					
					}
				}
				if (nf>=0)
				{
					if (ed==0)
					{
						Faces[n].f01=nf;
						if ((Faces[nf].v0==Faces[n].v1)&&(Faces[nf].v1==Faces[n].v0)) Faces[nf].f01=n;
						if ((Faces[nf].v1==Faces[n].v1)&&(Faces[nf].v2==Faces[n].v0)) Faces[nf].f12=n;
						if ((Faces[nf].v2==Faces[n].v1)&&(Faces[nf].v0==Faces[n].v0)) Faces[nf].f20=n;
					}
					if (ed==1)
					{
						Faces[n].f12=nf;
						if ((Faces[nf].v0==Faces[n].v2)&&(Faces[nf].v1==Faces[n].v1)) Faces[nf].f01=n;
						if ((Faces[nf].v1==Faces[n].v2)&&(Faces[nf].v2==Faces[n].v1)) Faces[nf].f12=n;
						if ((Faces[nf].v2==Faces[n].v2)&&(Faces[nf].v0==Faces[n].v1)) Faces[nf].f20=n;
					}
					if (ed==2)
					{
						Faces[n].f20=nf;
						if ((Faces[nf].v0==Faces[n].v0)&&(Faces[nf].v1==Faces[n].v2)) Faces[nf].f01=n;
						if ((Faces[nf].v1==Faces[n].v0)&&(Faces[nf].v2==Faces[n].v2)) Faces[nf].f12=n;
						if ((Faces[nf].v2==Faces[n].v0)&&(Faces[nf].v0==Faces[n].v2)) Faces[nf].f20=n;
					}
				}
			}
		}
	}

    for (n=0;n<nVertices;n++) corres[n].Free();
    
	delete [] linked;
    delete [] corres;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate an object

		CObject3D * CObject3D::Duplicate2()
		CObject3D CObject3D::Duplicate()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::Duplicate()
{
	return (*Duplicate2());
}


int anim_stringlen(CObject3D *obj)
{
	int nz=0;
	int n;

	if (obj->anim_strings)
	{
		if (obj->nKeys>0)
		{
			for (n=0;n<obj->nKeys;n++)
			{
				while (obj->anim_strings[nz]!='\0') nz++;
				nz++;
			}

			return (nz-1);
		}
		else return ((int) strlen(obj->anim_strings));
	}
	else
	{
		return 0;
	}
}

CObject3D * CObject3D::Duplicate2()
{
	CObject3D *duplicated;
	int n,k;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	duplicated->Init(nVertices,nFaces);

	for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];
	for (n=0;n<nFaces;n++) duplicated->Faces[n]=Faces[n];

	if (nKeys>0)
	{
		duplicated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			duplicated->VerticesKeys[k]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++) duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
		}
	}

	if (anim_strings)
	{
		duplicated->anim_strings=(char*) malloc(anim_stringlen(this)+1);
		memset(duplicated->anim_strings,0,anim_stringlen(this)+1);
		memcpy(duplicated->anim_strings,anim_strings,anim_stringlen(this));
	}

	duplicated->SetFaces();
	duplicated->Rot=Rot;
	duplicated->Coo=Coo;
	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;
	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];
	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;
	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;
	duplicated->impostor=impostor;

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        dissociate coplanars an object
        CObject3D * CObject3D::Triangles()

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::Triangles()
{
    int n;
    for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2=0;
    
    CVertex * vertices=new CVertex[nFaces*3];
    CFace * faces=new CFace[nFaces];
    
    CObject3D *res=new CObject3D;

	for (n=0;n<nFaces;n++)
	{
		vertices[3*n+0]=*Faces[n].v[0];
		vertices[3*n+1]=*Faces[n].v[1];
		vertices[3*n+2]=*Faces[n].v[2];

		faces[n]=Faces[n];
		faces[n].v0=n*3+0;
		faces[n].v1=n*3+1;
		faces[n].v2=n*3+2;
	}
    
    res->Vertices=vertices;
    res->Faces=faces;
    
    res->nVertices=nFaces*3;
    res->nFaces=nFaces;

    res->SetFaces();
    res->CalculateNormals(-1);
    res->Rot=Rot;
    res->Coo=Coo;
    for (n=0;n<6;n++) res->P[n]=P[n];
    res->nP=nP;
    for (n=0;n<16;n++) res->Tab[n]=Tab[n];
    res->Status=Status;
    res->nurbs=nurbs;
    res->Status2=Status2;
    res->Attribut=Attribut;
    res->Tag=Tag;
    res->env_mapping=env_mapping;
    res->group=group;

    return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        dissociate coplanars an object
        CObject3D * CObject3D::PlanarOptimize()

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::PlanarOptimize()
{
    int n;
    for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2=0;
    
    int nn=0;
    bool end=false;
    int tag=1;
    
    SetF012();
    
    CVertex * vertices=new CVertex[nFaces*3];
    CFace * faces=new CFace[nFaces];
    
    int * corres=(int *) malloc(4*nVertices);
    int nf=0;
    int np=0;

    CObject3D *res=new CObject3D;
    
    while (!end)
    {
        n=nn;
        nn=-1;
        while (n<nFaces)
        {
            if (Faces[n].tag==0) { nn=n; break; }
            else n++;
        }

        if (nn==-1) end=true;
        else
        {
            TagCoplanarFacesSet(nn,nn,tag);
            
            for (n=0;n<nVertices;n++) Vertices[n].tag=0;
            
            for (n=0;n<nFaces;n++)
            {
                if (Faces[n].tag==tag)
                {
                    Vertices[Faces[n].v0].tag=1;
                    Vertices[Faces[n].v1].tag=1;
                    Vertices[Faces[n].v2].tag=1;
                }
            }
            
            for (n=0;n<nVertices;n++)
            {
                corres[n]=-1;
                if (Vertices[n].tag==1)
                {
                    corres[n]=np;
                    vertices[np]=Vertices[n];
                    np++;
                }
            }
            
            for (n=0;n<nFaces;n++)
            if (Faces[n].tag==tag)
            {
                faces[nf]=Faces[n];
                faces[nf].v0=corres[Faces[n].v0];
                faces[nf].v1=corres[Faces[n].v1];
                faces[nf].v2=corres[Faces[n].v2];
                nf++;
            }
            
            tag++;
        }
    }
    
    res->Vertices=vertices;
    res->Faces=faces;
    
    free(corres);
    
    res->nVertices=np;
    res->nFaces=nf;

    res->SetFaces();
    res->CalculateNormals(-1);
    res->Rot=Rot;
    res->Coo=Coo;
    for (n=0;n<6;n++) res->P[n]=P[n];
    res->nP=nP;
    for (n=0;n<16;n++) res->Tab[n]=Tab[n];
    res->Status=Status;
    res->nurbs=nurbs;
    res->Status2=Status2;
    res->Attribut=Attribut;
    res->Tag=Tag;
    res->env_mapping=env_mapping;
    res->group=group;

    return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        bevel an object

        CObject3D * CObject3D::ApplyBevel2()
        CObject3D CObject3D::ApplyBevel()

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::ApplyBevel(float R)
{
    return (*ApplyBevel2(R,0));
}

CObject3D * CObject3D::ApplyBevel2(float R)
{
	return ApplyBevel2(R,0);
}

CObject3D * CObject3D::ApplyBevel2(float R,int proc)
{
    int n,k;
    EdgeListD ED;
    CObject3D *dup;
    
    for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2=0;
    for (n=0;n<nVertices;n++) Vertices[n].Map2.Init(0,0);
    for (n=0;n<nVertices;n++) Vertices[n].Map.Init(0,0);

    ConvertLinear(SMALLF);
        
    CObject3D *obj=PlanarOptimize();
    if (obj)
    {
        delete [] Faces;
        delete [] Vertices;
        nFaces=obj->nFaces;
        Faces=obj->Faces;
        nVertices=obj->nVertices;
        Vertices=obj->Vertices;
        
        obj->Faces=NULL;
        obj->Vertices=NULL;
        obj->Free();
        delete obj;
    }
    
    dup=new CObject3D;
    dup->Init(nVertices*16,nFaces*16);
    
    ED.Init(nFaces*3);
    
    int nt=Faces[0].nT;
    
    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    for (n=0;n<nVertices;n++) Vertices[n].tag2=-1;
    
    CalculateNormals2a(-1);
    
    int tag=0;
    bool end=false;
    
    int nv=0;
    int nf=0;
    
    while (!end)
    {
        int nn=0;
        while ((Faces[nn].tag!=0)&&(nn<nFaces)) nn++;
        
        if (nn==nFaces) end=true;
        else
        {
            tag++;
            TagCoplanarFaces(nn,nn,tag);
            
            CVector N=Faces[nn].Norm;
            
            EdgeListD ed;
            int nb=0;
            for (n=0;n<nFaces;n++)
                if (Faces[n].tag==tag) nb++;

            int donot=0;
            
            float RR=R;
            
            for (n=0;n<nFaces;n++)
            if (Faces[n].tag==tag)
            {
                CVector u1=Faces[n].v[0]->Stok-Faces[n].v[1]->Stok;
                CVector u2=Faces[n].v[1]->Stok-Faces[n].v[2]->Stok;
                CVector u3=Faces[n].v[2]->Stok-Faces[n].v[0]->Stok;
                
                int cnt=0;
                while (((VECTORNORM(u1)<RR*2)||(VECTORNORM(u2)<RR*2)||(VECTORNORM(u3)<RR*2))&&(cnt<4)) RR=RR/2;
                if (cnt==4) donot=1;
            }

            ed.Init(nb*3);
            
            for (n=0;n<nVertices;n++) Vertices[n].temp=0;
            
            for (n=0;n<nFaces;n++)
            if (Faces[n].tag==tag)
            {
                ed.AddEdge(Faces[n].v0,Faces[n].v1);
                ed.AddEdge(Faces[n].v1,Faces[n].v2);
                ed.AddEdge(Faces[n].v2,Faces[n].v0);
                
                Faces[n].v[0]->temp=1;
                Faces[n].v[1]->temp=1;
                Faces[n].v[2]->temp=1;
            }
            
            for (n=0;n<nVertices;n++) Vertices[n].tag=0;
            
            for (n=0;n<ed.nList;n++)
            {
                ED.List[ED.nList].a=ed.List[n].a;
                ED.List[ED.nList].b=ed.List[n].b;
                ED.nList++;
                
                Vertices[ed.List[n].a].tag=1;
                Vertices[ed.List[n].b].tag=1;
            }
            
            /*
            for (n=0;n<nVertices;n++)
                if (Vertices[n].tag) Vertices[n].Stok0.Init(0,0,0);
            
            for (n=0;n<ed.nList;n++)
            {
                CVector A=Vertices[ed.List[n].a].Stok;
                CVector B=Vertices[ed.List[n].b].Stok;
                CVector uu=A-B;
                uu.Norme();
                CVector vv=uu^N;
                
                Vertices[ed.List[n].a].Stok0+=vv;
                Vertices[ed.List[n].b].Stok0+=vv;
            }
            /**/
            
            ed.Free();
            
            nb=0;
            CVector G(0,0,0);
            for (n=0;n<nVertices;n++)
                if (Vertices[n].tag) { G+=Vertices[n].Stok; nb++; }

            G/=nb;
            CVector u;

			CMatrix M;
			M.RotationAngleVecteur(N,-PI/32);

            for (n=0;n<nVertices;n++)
            if (Vertices[n].temp)
            {
                if (Vertices[n].tag)
                {
                    Vertices[n].tag2=nv;
                    u=Vertices[n].Stok-G;
                    //u=Vertices[n].Stok0;
                    u.Normalise();
                    
                    float s=DOT(Vertices[n].Norm,u);
//                    u=Vertices[n].Norm-s*N;
//                    u.Normalise();
                    
                    if (donot==0)
                    {
                        if ((s>0)||(proc==0)) dup->Vertices[nv].Stok=Vertices[n].Stok-u*RR;
						else
						{
							dup->Vertices[nv].Stok=Vertices[n].Stok*M+u*RR;
						}
                        dup->Vertices[nv].Map=Vertices[n].Map;
                        dup->Vertices[nv].tag2=n;
                        nv++;
                    }
                    else
                    {
                        dup->Vertices[nv].Stok=Vertices[n].Stok;
                        dup->Vertices[nv].Map=Vertices[n].Map;
                        dup->Vertices[nv].tag2=n;
                        nv++;
                    }
                }
                else
                {
                    Vertices[n].tag2=nv;
                    dup->Vertices[nv].Stok=Vertices[n].Stok;
                    dup->Vertices[nv].Map=Vertices[n].Map;
                    dup->Vertices[nv].tag2=n;
                    nv++;
                }
            }

            for (n=0;n<nFaces;n++)
            if (Faces[n].tag==tag)
            {
                dup->Faces[nf].v0=Faces[n].v[0]->tag2;
                dup->Faces[nf].v1=Faces[n].v[2]->tag2;
                dup->Faces[nf].v2=Faces[n].v[1]->tag2;
                dup->Faces[nf].nT=Faces[n].nT;
                dup->Faces[nf].ref=Faces[n].ref;
                nf++;
            }
        }
    }
    
    EdgeListD TRI;
    
    TRI.Init(ED.nList*3);
    
    for (n=0;n<ED.nList;n++) ED.List[n].tag=0;
    
    for (n=0;n<ED.nList;n++)
    {
        ED.List[n].tag=1;
        int nn=-1;
        
        for (k=0;k<ED.nList;k++)
        if (ED.List[k].tag==0)
        {
            CVector a=Vertices[ED.List[n].a].Stok-Vertices[ED.List[k].b].Stok;
            CVector b=Vertices[ED.List[n].b].Stok-Vertices[ED.List[k].a].Stok;
            if (VECTORNORM2(a)<SMALLF)
                if (VECTORNORM2(b)<SMALLF) nn=k;
        }
        
        if (nn>=0)
        {
            ED.List[nn].tag=1;
            
            dup->Vertices[nv+0].Stok=dup->Vertices[Vertices[ED.List[n].a].tag2].Stok;
            dup->Vertices[nv+1].Stok=dup->Vertices[Vertices[ED.List[n].b].tag2].Stok;
            
            dup->Vertices[nv+2].Stok=dup->Vertices[Vertices[ED.List[nn].a].tag2].Stok;
            dup->Vertices[nv+3].Stok=dup->Vertices[Vertices[ED.List[nn].b].tag2].Stok;
            
            TRI.AddEdge(nv+1,nv+2);
            TRI.AddEdge(nv+3,nv+0);

            dup->Faces[nf].v0=nv+0;
            dup->Faces[nf].v1=nv+1;
            dup->Faces[nf].v2=nv+2;
            dup->Faces[nf].nT=nt;
            dup->Faces[nf].ref=0;
            nf++;

            dup->Faces[nf].v0=nv+0;
            dup->Faces[nf].v1=nv+2;
            dup->Faces[nf].v2=nv+3;
            dup->Faces[nf].nT=nt;
            dup->Faces[nf].ref=0;
            nf++;
            
            nv+=4;
        }
    }

    ED.Free();
    
    for (n=0;n<TRI.nList;n++) TRI.List[n].tag=0;
    
    for (n=0;n<TRI.nList;n++)
    {
        TRI.List[n].tag=1;
        CVector aa;
        int res=0;
        int list[128];
        int nlist=0;

        bool stop=false;
        
        CVector AC=dup->Vertices[TRI.List[n].b].Stok;
        
        list[0]=TRI.List[n].a;
        nlist=1;
        while (!stop)
        {
            stop=true;
            
            for (k=0;k<TRI.nList;k++)
            if (TRI.List[k].tag==0)
            {
                aa=AC-dup->Vertices[TRI.List[k].a].Stok;
                if (VECTORNORM2(aa)<SMALLF)
                {
                    TRI.List[k].tag=1;
                    list[nlist++]=TRI.List[k].a;
                    list[nlist]=TRI.List[k].b;
                    
                    AC=dup->Vertices[TRI.List[k].b].Stok;
                    
                    aa=AC-dup->Vertices[TRI.List[n].a].Stok;
                    if (VECTORNORM2(aa)>SMALLF) stop=false;
                    break;
                }
            }
        }
        
        if (nlist>=3)
        for (k=1;k<nlist;k++)
        {
            dup->Vertices[nv+0].Stok=dup->Vertices[list[0]].Stok;
            dup->Vertices[nv+1].Stok=dup->Vertices[list[k]].Stok;
            dup->Vertices[nv+2].Stok=dup->Vertices[list[k+1]].Stok;
            
            dup->Faces[nf].v0=nv+0;
            dup->Faces[nf].v2=nv+1;
            dup->Faces[nf].v1=nv+2;
            dup->Faces[nf].nT=nt;
            dup->Faces[nf].ref=0;
            nf++;

            nv+=3;
        }
    }
    
    TRI.Free();
    
    dup->nVertices=nv;
    dup->nFaces=nf;
        
    dup->SetFaces();
    dup->CalculateNormals(-1);
    dup->Rot=Rot;
    dup->Coo=Coo;
    for (n=0;n<6;n++) dup->P[n]=P[n];
    dup->nP=nP;
    for (n=0;n<16;n++) dup->Tab[n]=Tab[n];
    dup->Status=Status;
    dup->nurbs=nurbs;
    dup->Status2=Status2;
    dup->Attribut=Attribut;
    dup->Tag=Tag;
    dup->env_mapping=env_mapping;
    dup->group=group;
    
    CObject3D *tmp=dup->Duplicate2();
    dup->Free();
    delete dup;
    
    for (n=0;n<tmp->nVertices;n++) tmp->Vertices[n].tag=tmp->Vertices[n].tag2=0;
    for (n=0;n<tmp->nFaces;n++) tmp->Faces[n].tag=tmp->Faces[n].tag2=0;
    
    return tmp;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate an object

		CObject3D CObject3D::Dedouble()
		CObject3D * CObject3D::Dedouble2()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::Dedouble()
{
	return (*Dedouble2());
}

CObject3D * CObject3D::Dedouble2()
{
	CObject3D *duplicated;
	int n;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	duplicated->Init(nVertices*2,nFaces*2);

	for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];
	for (n=0;n<nVertices;n++) duplicated->Vertices[nVertices + n]=Vertices[n];

	for (n=0;n<nFaces;n++)
	{
		duplicated->Faces[2*n+0]=Faces[n];
		duplicated->Faces[2*n+1]=Faces[n];
		duplicated->Faces[2*n+1].v0=Faces[n].v0+nVertices;
		duplicated->Faces[2*n+1].v1=Faces[n].v2+nVertices;
		duplicated->Faces[2*n+1].v2=Faces[n].v1+nVertices;
	}

	duplicated->SetFaces();

	duplicated->CalculateNormals(-1);

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;
	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;
	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];
	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;
	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		attrib pointers

		void CObject3D::SetFaces()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetFaces()
{
	int n;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]=&Vertices[Faces[n].v0];
		Faces[n].v[1]=&Vertices[Faces[n].v1];
		Faces[n].v[2]=&Vertices[Faces[n].v2];
	}
}

void CObject3D::Init(int nv,int nf)
{
	Vertices=NULL;
	Faces=NULL;

	nVertices=0;
	nFaces=0;

	if ((nv<=0)||(nf<0)) return;

	Vertices=new CVertex[nv];
	if (nf>0) Faces=new CFace[nf];

	nVertices=nv;
	nFaces=nf;

	Status=0;
    for (int n=0;n<nf;n++)
    {
        Faces[n].ref=0;
        Faces[n].tag4=0;
    }
}

void CObject3D::Init2(int nv,int nf,int nT)
{
	Vertices=NULL;
	Faces=NULL;

	nVertices=0;
	nFaces=0;

	if ((nv<=0)||(nf<0)) return;

	Vertices=new CVertex[nv];
	if (nf>0) Faces=new CFace[nf];

	nVertices=nv;
	nFaces=nf;

    for (int n=0;n<nf;n++)
    {
        Faces[n].nT=0;
        Faces[n].ref=0;
        Faces[n].tag4=0;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		compressed storage function

		read()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::read(CDataReader *dat)
{
	int n,k;
	int nv,nf,nk;
	int tagve,tagac;
	int version=dat->readInt();

	if (version==1001)
	{
		nv=dat->readInt();
		nf=dat->readInt();
		nk=dat->readInt();

		tagve=dat->readInt();
		tagac=dat->readInt();

		if (tagve)
		{
			velocities=new CVector[nv];
			for (n=0;n<nv;n++) velocities[n].Init(0,0,0);
		}

		if (tagac)
		{
			accelerations=new CVector[nv];
			for (n=0;n<nv;n++) accelerations[n].Init(0,0,0);
		}

		Init(nv,nf);
		nKeys=nk;

		Coo=dat->readVector();
		Rot=dat->readVector();

		int tt=dat->readInt();
		if (tt) env_mapping=true; else env_mapping=false;

		for (n=0;n<128;n++) Name[n]=dat->readChar();
		nP=dat->readInt();

		for (n=0;n<6;n++)
		{
			P[n].a=dat->readFloat();
			P[n].b=dat->readFloat();
			P[n].c=dat->readFloat();
			P[n].d=dat->readFloat();
		}

        for (n=0;n<16;n++) Tab[n]=dat->readInt();
        for (n=0;n<16;n++) Tab2[n]=dat->readInt();
        
        Radius=dat->readFloat();
        
        Status=dat->readInt();
        Status2=dat->readInt();
		Attribut=dat->readInt();
        Tag=dat->readInt();
        Tag2=dat->readInt();

		int len=dat->readInt();
		if (len>0)
		{
			anim_strings=(char*) malloc(len+1);
			memset(anim_strings,0,len+1);
			for (n=0;n<len;n++) anim_strings[n]=dat->readChar();
		}

		for (n=0;n<nVertices;n++)
		{
            Vertices[n].Stok=dat->readVector();
			Vertices[n].Map=dat->readMap();
            
			if (tag_stuffs_in_sleep_data)
			{
                Vertices[n].Norm=dat->readVector();
                Vertices[n].coef1=dat->readFloat();
                
                //Vertices[n].Map2=dat->readMap();
                
                Vertices[n].Index[0]=dat->readWord();
                Vertices[n].Index[1]=dat->readWord();
                Vertices[n].Index[2]=dat->readWord();
                Vertices[n].Index[3]=dat->readWord();
                
                Vertices[n].Weight[0]=dat->readFloat();
                Vertices[n].Weight[1]=dat->readFloat();
                Vertices[n].Weight[2]=dat->readFloat();
                Vertices[n].Weight[3]=dat->readFloat();
                
				Vertices[n].tag=dat->readWord();
				Vertices[n].tag2=dat->readWord();
			}
		}

		for (n=0;n<nFaces;n++)
		{
			Faces[n].v0=dat->readInt();
			Faces[n].v1=dat->readInt();
			Faces[n].v2=dat->readInt();
			if (tag_stuffs_in_sleep_data)
			{
                Faces[n].Norm=dat->readVector();
                Faces[n].g=dat->readVector();
                Faces[n].coef1=dat->readFloat();
                
                Faces[n].mp0=dat->readMap();
                Faces[n].mp1=dat->readMap();
                Faces[n].mp2=dat->readMap();
                
				Faces[n].tag=dat->readWord();
				Faces[n].tag2=dat->readWord();
				Faces[n].nT=dat->readWord();
				Faces[n].nT2=dat->readWord();
				Faces[n].nL=dat->readInt();
				Faces[n].nLVectors=dat->readInt();
				Faces[n].ref=dat->readWord();
				Faces[n].lock=dat->readWord();
                //Faces[n].coef1=dat->readFloat();
			}
		}

		if (nk>0)
		{
			for (k=0;k<nKeys;k++)
			{
				VerticesKeys[k]=new CShortVertex[nv];
				for (n=0;n<nVertices;n++)
				{
					VerticesKeys[k][n].Stok=dat->readVector();
					if (tag_stuffs_in_sleep_data) VerticesKeys[k][n].Norm=dat->readVector();
				}
			}
		}

		SetFaces();
		CalculateNormals(-1);
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		compressed storage function

		store()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::store(CDataContainer *dat)
{
	int n,k;
	int len;
	int version=1001;

	dat->store(version);

	dat->store(nVertices);
	dat->store(nFaces);
	dat->store(nKeys);

	if (velocities) dat->storeInt(1); else dat->storeInt(0);
	if (accelerations) dat->storeInt(1); else dat->storeInt(0);

	dat->store(Coo);
	dat->store(Rot);

	if (env_mapping) dat->storeInt(1); else dat->storeInt(0);

	dat->store(Name,128);
	dat->store(nP);

	for (n=0;n<6;n++)
	{
		dat->store(P[n].a);
		dat->store(P[n].b);
		dat->store(P[n].c);
		dat->store(P[n].d);
	}

    for (n=0;n<16;n++) dat->store(Tab[n]);
    for (n=0;n<16;n++) dat->store(Tab2[n]);

    dat->store(Radius);
    
    dat->store(Status);
    dat->store(Status2);
	dat->store(Attribut);
    dat->store(Tag);
    dat->store(Tag2);

	if (anim_strings)
	{
		len=anim_stringlen(this);
		dat->store(len);
		for (n=0;n<len;n++) dat->storeChar(anim_strings[n]);
	}
	else
	{
		len=-1;
		dat->store(len);
	}

	for (n=0;n<nVertices;n++)
	{
        dat->store(Vertices[n].Stok);
		dat->store(Vertices[n].Map);
        
		if (tag_stuffs_in_sleep_data)
		{
            dat->store(Vertices[n].Norm);
            dat->store(Vertices[n].coef1);
            //dat->store(Vertices[n].Map2);
            
            dat->storeWord(Vertices[n].Index[0]);
            dat->storeWord(Vertices[n].Index[1]);
            dat->storeWord(Vertices[n].Index[2]);
            dat->storeWord(Vertices[n].Index[3]);
            
            dat->store(Vertices[n].Weight[0]);
            dat->store(Vertices[n].Weight[1]);
            dat->store(Vertices[n].Weight[2]);
            dat->store(Vertices[n].Weight[3]);
            
			dat->storeWord(Vertices[n].tag);
			dat->storeWord(Vertices[n].tag2);
		}
	}

	for (n=0;n<nFaces;n++)
	{
		dat->storeInt(Faces[n].v0);
		dat->storeInt(Faces[n].v1);
		dat->storeInt(Faces[n].v2);
		if (tag_stuffs_in_sleep_data)
		{
            dat->storeVector(Faces[n].Norm);
            dat->storeVector(Faces[n].g);
            dat->storeFloat(Faces[n].coef1);
            
            dat->store(Faces[n].mp0);
            dat->store(Faces[n].mp1);
            dat->store(Faces[n].mp2);
            
            dat->storeWord(Faces[n].tag);
			dat->storeWord(Faces[n].tag2);
			dat->storeWord(Faces[n].nT);
			dat->storeWord(Faces[n].nT2);
			dat->store(Faces[n].nL);
			dat->store(Faces[n].nLVectors);
			dat->storeWord(Faces[n].ref);
			dat->storeWord(Faces[n].lock);
            //dat->storeFloat(Faces[n].coef1);
		}
		else
		{
		}
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++)
			{
				dat->store(VerticesKeys[k][n].Stok);
				if (tag_stuffs_in_sleep_data) dat->store(VerticesKeys[k][n].Norm);
			}
		}
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		compressed storage function
		directly change storage of the object in compressed or normal form

		SetForm(int flag)

		this function is mainly internal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::SetForm(int flag)
{
	int tmp;
	int tag1,tag2,tag3;
	int res=-1;

    if ((strlen(TEMPORARY_DIRECTORY_FOR_FILES_SLEEP_PACK)>0)&&(ID_TEMPORARY_FILES>=0))
    {
        if ((flag==OBJECT3D_COMPRESSED_FORM)||(flag==OBJECT3D_PACKED_FORM))
        {
            if (temporaryID<0)
            {
                CDataContainer dat;

                dat.clean();

                if (Double1) dat.storeInt(1);
                else dat.storeInt(0);

                if (Double2) dat.storeInt(1);
                else dat.storeInt(0);

                if (lod) dat.storeInt(1);
                else dat.storeInt(0);
                
                store(&dat);

                if (Double1) Double1->store(&dat);
                if (Double2) Double2->store(&dat);
                if (lod) lod->store(&dat);
                
                char *ptr=dat.getData();
                int len=dat.getLength();

                temporaryID=ID_TEMPORARY_FILES++;
                
                char fname[4096];
                sprintf(fname,"%sfile_%d.temp",TEMPORARY_DIRECTORY_FOR_FILES_SLEEP_PACK,temporaryID);

                FILE *f=fopen(fname,"wb");
                fwrite(ptr,len,1,f);
                fclose(f);

                res=len;
                
                dat.clean();
                
                Free();
            }
        }
        else
        {
            if (temporaryID>=0)
            {
                char fname[4096];
                sprintf(fname,"%sfile_%d.temp",TEMPORARY_DIRECTORY_FOR_FILES_SLEEP_PACK,temporaryID);
                
                temporaryID=-1;

                FILE *f=fopen(fname,"rb");
                if (f)
                {
                    fseek(f,0,SEEK_END);
                    int len=ftell(f);
                    fseek(f,0,SEEK_SET);
                    char *ptr=(char*)malloc(len);
                    fread(ptr,len,1,f);
                    fclose(f);
                    
                    remove(fname);
                
                    CDataReader dat;

                    dat.clean();
                    
                    dat.data=ptr;
                    dat.data_length=len;

                    tag1=dat.readInt();
                    tag2=dat.readInt();
                    tag3=dat.readInt();

                    read(&dat);

                    if (tag1)
                    {
                        Double1=new CObject3D;
                        Double1->read(&dat);
                    }

                    if (tag2)
                    {
                        Double2=new CObject3D;
                        Double2->read(&dat);
                    }
                    
                    if (tag3)
                    {
                        lod=new CObject3D;
                        lod->read(&dat);
                    }
                    
                    dat.clean();
                }
            }
        }
    }
    else
    {
        // 1 seul niveau de r�cursivit� dans les doubles des objets
        if (flag==OBJECT3D_COMPRESSED_FORM)
        {
            if (form_compressed_buffer==NULL)
            {
                CDataContainer dat;

                dat.clean();

                if (Double1) dat.storeInt(1);
                else dat.storeInt(0);

                if (Double2) dat.storeInt(1);
                else dat.storeInt(0);

                if (lod) dat.storeInt(1);
                else dat.storeInt(0);
                
                store(&dat);

                if (Double1) Double1->store(&dat);
                if (Double2) Double2->store(&dat);
                if (lod) lod->store(&dat);
                dat.compressFast();

                res=dat.getLZLength()+8;
                char * buffer=(char*) malloc(dat.getLZLength()+8);

                tmp=dat.getLZLength();
                memcpy(&buffer[0],&tmp,4);
                tmp=dat.getLength();
                memcpy(&buffer[4],&tmp,4);
                memcpy(&buffer[8],dat.getLZData(),dat.getLZLength());

                dat.clean();
                
                Free();
                
                form_compressed_buffer=buffer;
            }
        }
        else
        if (flag==OBJECT3D_PACKED_FORM)
        {
            if (form_compressed_buffer==NULL)
            {
                CDataContainer dat;

                dat.clean();

                if (Double1) dat.storeInt(1);
                else dat.storeInt(0);

                if (Double2) dat.storeInt(1);
                else dat.storeInt(0);

                if (lod) dat.storeInt(1);
                else dat.storeInt(0);
                
                store(&dat);

                if (Double1) Double1->store(&dat);
                if (Double2) Double2->store(&dat);
                if (lod) lod->store(&dat);
                
                char *ptr=dat.getData();
                int len=dat.getLength();

                res=len+8;
                char * buffer=(char*) malloc(len+8);

                tmp=-1;
                memcpy(&buffer[0],&tmp,4);
                tmp=len;
                memcpy(&buffer[4],&tmp,4);
                memcpy(&buffer[8],ptr,len);

                dat.clean();
                
                Free();
                
                form_compressed_buffer=buffer;
            }
        }
        else
        {
            if (form_compressed_buffer)
            {
                CDataReader dat;

                dat.clean();
                
                int *pack=(int*)form_compressed_buffer;
                
                if (*pack==-1)
                {
                    int len=((int*)form_compressed_buffer)[1];
                    dat.data=(char*) malloc(len);
                    memcpy(dat.data,&form_compressed_buffer[8],len);
                    dat.data_length=len;
                }
                else
                {
                    dat.getLZ(form_compressed_buffer);
                }

                free(form_compressed_buffer);
                form_compressed_buffer=NULL;

                tag1=dat.readInt();
                tag2=dat.readInt();
                tag3=dat.readInt();

                read(&dat);

                if (tag1)
                {
                    Double1=new CObject3D;
                    Double1->read(&dat);
                }

                if (tag2)
                {
                    Double2=new CObject3D;
                    Double2->read(&dat);
                }
                
                if (tag3)
                {
                    lod=new CObject3D;
                    lod->read(&dat);
                }
                
                dat.clean();
            }
        }
    }
	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		compressed storage function
		directly change storage of the object in compressed or normal form
		sleep() compress the object that will be unusable until awake() is called

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


int CObject3D::sleep()
{
	if (nFaces==0) return 0;
	state=1;
    return SetForm(OBJECT3D_COMPRESSED_FORM);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        compressed storage function
        directly change storage of the object in compressed or normal form
        pack() pack the object that will be unusable until awake() is called

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


int CObject3D::pack()
{
	state=1;
    return SetForm(OBJECT3D_PACKED_FORM);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		compressed storage function
		directly change storage of the object in compressed or normal form

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::awake()
{
	if (state==1)
	{
		state=0;
		SetForm(OBJECT3D_NORMAL_FORM);
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Free ressources allocated for the object

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Free()
{
	if (form_compressed_buffer)
	{
		free(form_compressed_buffer);
		form_compressed_buffer=NULL;
	}
	else
	{
		if (velocities) { delete [] velocities; }
		if (accelerations) { delete [] accelerations; }

		velocities=NULL;
		accelerations=NULL;
        
        if (Silhouette)
        {
            Silhouette->Free();
            delete Silhouette;
            Silhouette=NULL;
        }

		if (Double1)
		{
			Double1->Free();
			delete Double1;
			Double1=NULL;
		}

		if (Double2)
		{
			Double2->Free();
			delete Double2;
			Double2=NULL;
		}

        if (lod)
        {
            lod->Free();
            delete lod;
            lod=NULL;
        }
        
		if ((Vertices)&&(nVertices>0)) delete[] Vertices;
		if ((Faces)&&(nFaces>0)) delete[] Faces;

		nVertices=0;
		nFaces=0;

		if (nKeys>0)
		{
			for (int n=0;n<nKeys;n++)
			{
				delete [] VerticesKeys[n];
			}
			nKeys=0;
		}

		if (edges)
		{
			edges->Free();
			delete edges;
			edges=NULL;
		}

		if (anim_strings) free(anim_strings);
		anim_strings=NULL;

		Faces=NULL;
		Vertices=NULL;

	}
    
    if (vbedges)
    {
        FreeVB(vbedges);
        vbedges=NULL;
    }
    
    if (vbedges2)
    {
        FreeVB(vbedges2);
        vbedges2=NULL;
    }

    
#ifdef INCLUDE_PHYSIC
	Proprietes.Free();
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::FillTexture(int NT)
{
	int n;
	for (n=0;n<nFaces;n++) Faces[n].nT=NT;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CutPlane(CObject3D obj,float a,float b,float c,float d)
{
	CObject3D dest;
	int n,nn,np,nf,cc;
	CFace f, f2;
	int * tags;
	int * corres;
	int * tagsf;
	int v0,v1,v2;
	float s1,s2,s3,s,ss,t1,t2;
	CVector A,B;
	CVector NA,NB;
	CVector2 mA,mB,map0,map1,map2;
	int res,res2;
	CRGBA dA,dB,d0,d1,d2;


	tags=(int *) malloc(obj.nVertices*4);
	corres=(int *) malloc(obj.nVertices*4);

	for (n=0;n<obj.nVertices;n++)
	{
		tags[n]=0;
	}


	tagsf=(int *) malloc(obj.nFaces*4);

	for (n=0;n<obj.nFaces;n++)
	{
		tagsf[n]=0;
	}


#define PLAN(n) (a*obj.Vertices[n].Stok.x + b*obj.Vertices[n].Stok.y + c*obj.Vertices[n].Stok.z +d)

	np=0;
	nf=0;

	for (n=0;n<obj.nFaces;n++)
	{
		v0=obj.Faces[n].v0;
		v1=obj.Faces[n].v1;
		v2=obj.Faces[n].v2;

		s1=PLAN(v0);
		s2=PLAN(v1);
		s3=PLAN(v2);

		cc=(s1>0)+(s2>0)+(s3>0);

		if (cc==3)
		{
			// face totalement en dehors
		}
		else
			if (cc==0)
			{
				// face totalement en dedans
				tagsf[n]=1;
				tags[v0]=1;
				tags[v1]=1;
				tags[v2]=1;
			}
			else
			{
				// face partiellement en dehors

				tagsf[n]=0;
				np+=2;


				if (cc==2)	// 2 vertex dehors
				{
					nf++;
					res=0;

					if (s1<0)
					{
						tags[v0]=1;
						if ((f_abs(s2)<SMALLF)&&(f_abs(s3)<SMALLF)) res=1;

					}

					if (s2<0)
					{
						tags[v1]=1;
						if ((f_abs(s1)<SMALLF)&&(f_abs(s3)<SMALLF)) res=1;
					}
					if (s3<0)
					{
						tags[v2]=1;
						if ((f_abs(s1)<SMALLF)&&(f_abs(s2)<SMALLF)) res=1;
					}

					if (res==1)
					{
						tags[v0]=1;
						tags[v1]=1;
						tags[v2]=1;
					}

				}
				else
				{


					res=2;

					if (s1>0)
					{
						tags[v1]=1;
						tags[v2]=1;
						if ((f_abs(s3)<SMALLF)&&(f_abs(s2)<SMALLF)) res=1;
					}

					if (s2>0)
					{
						tags[v0]=1;
						tags[v2]=1;
						if ((f_abs(s1)<SMALLF)&&(f_abs(s3)<SMALLF)) res=1;
					}

					if (s3>0)
					{
						tags[v0]=1;
						tags[v1]=1;

						if ((f_abs(s1)<SMALLF)&&(f_abs(s2)<SMALLF)) res=1;
					}

					nf+=res;

					if (res==1)
					{
						tags[v0]=1;
						tags[v1]=1;
						tags[v2]=1;
					}

				}
			}
	}


	for (n=0;n<obj.nFaces;n++)
	{
		if (tagsf[n]==1) nf++;
	}


	// pour creer face lorsque trop proche plan !(res=res2=0) => stocker tous les sommets

	nn=0;
	for (n=0;n<obj.nVertices;n++)
	{
		if (tags[n]==1)
		{
			np++;
			corres[n]=nn;
			nn++;
		}
	}

	dest.Init(np,nf*2);

	np=0;
	nf=0;

	for (n=0;n<obj.nVertices;n++)
	{
		if (tags[n]==1)
		{
			dest.Vertices[np].Stok=obj.Vertices[n].Stok;
			dest.Vertices[np].Norm=obj.Vertices[n].Norm;
			dest.Vertices[np].tag2=obj.Vertices[n].tag2;
			dest.Vertices[np].tag=obj.Vertices[n].tag;
			dest.Vertices[np].temp=obj.Vertices[n].temp;
			dest.Vertices[np].Diffuse=obj.Vertices[n].Diffuse;
			np++;
		}
	}




	for (n=0;n<obj.nFaces;n++)
	{
		v0=obj.Faces[n].v0;
		v1=obj.Faces[n].v1;
		v2=obj.Faces[n].v2;

		s1=PLAN(v0);
		s2=PLAN(v1);
		s3=PLAN(v2);

		cc=(s1>0)+(s2>0)+(s3>0);

		if (cc==3)
		{
			// face totalement en dehors
		}
		else
			if (cc==0)
			{
				// face totalement en dedans

				dest.Faces[nf].Init(corres[v0],corres[v1],corres[v2]);
				dest.Faces[nf].mp0=obj.Faces[n].mp0;
				dest.Faces[nf].mp1=obj.Faces[n].mp1;
				dest.Faces[nf].mp2=obj.Faces[n].mp2;
				dest.Faces[nf].nT=obj.Faces[n].nT;
				dest.Faces[nf].tag=obj.Faces[n].tag;
				dest.Faces[nf].tag2=obj.Faces[n].tag2;
				nf++;
			}
			else
			{
				// face partiellement en dehors

				if (cc==2) s=-1;
				else s=1;

				res=0;

				res2=1;

				if (s*s1>0)
				{
					v0=obj.Faces[n].v0;
					v1=obj.Faces[n].v1;
					v2=obj.Faces[n].v2;

					d0=obj.Vertices[v0].Diffuse;
					d1=obj.Vertices[v1].Diffuse;
					d2=obj.Vertices[v2].Diffuse;

					ss=s1;
					map0=obj.Faces[n].mp0;
					map1=obj.Faces[n].mp1;
					map2=obj.Faces[n].mp2;

					if ((f_abs(s2)<SMALLF)&&(f_abs(s3)<SMALLF)) res=1;
					res2=0;

				}

				if (s*s2>0)
				{
					v0=obj.Faces[n].v1;
					v1=obj.Faces[n].v2;
					v2=obj.Faces[n].v0;
					ss=s2;

					d0=obj.Vertices[v1].Diffuse;
					d1=obj.Vertices[v2].Diffuse;
					d2=obj.Vertices[v0].Diffuse;


					map0=obj.Faces[n].mp1;
					map1=obj.Faces[n].mp2;
					map2=obj.Faces[n].mp0;

					if ((f_abs(s3)<SMALLF)&&(f_abs(s1)<SMALLF)) res=1;
					res2=0;

				}

				if (s*s3>0)
				{
					v0=obj.Faces[n].v2;
					v1=obj.Faces[n].v0;
					v2=obj.Faces[n].v1;
					ss=s3;

					d0=obj.Vertices[v2].Diffuse;
					d1=obj.Vertices[v0].Diffuse;
					d2=obj.Vertices[v1].Diffuse;


					map0=obj.Faces[n].mp2;
					map1=obj.Faces[n].mp0;
					map2=obj.Faces[n].mp1;

					if ((f_abs(s1)<SMALLF)&&(f_abs(s2)<SMALLF)) res=1;
					res2=0;

				}

				if ((res==0)&&(res2==0))
				{

					t1=-ss/(a*(obj.Vertices[v1].Stok.x - obj.Vertices[v0].Stok.x) +
							b*(obj.Vertices[v1].Stok.y - obj.Vertices[v0].Stok.y) +
							c*(obj.Vertices[v1].Stok.z - obj.Vertices[v0].Stok.z));


					t2=-ss/(a*(obj.Vertices[v2].Stok.x - obj.Vertices[v0].Stok.x) +
							b*(obj.Vertices[v2].Stok.y - obj.Vertices[v0].Stok.y) +
							c*(obj.Vertices[v2].Stok.z - obj.Vertices[v0].Stok.z));


					A=obj.Vertices[v0].Stok + t1*(obj.Vertices[v1].Stok - obj.Vertices[v0].Stok);
					NA=obj.Vertices[v0].Norm + t1*(obj.Vertices[v1].Norm - obj.Vertices[v0].Norm);
					NA.Normalise();
					mA=map0 + t1*(map1 - map0);
					dA=d0 + t1*(d1-d0);


					B=obj.Vertices[v0].Stok + t2*(obj.Vertices[v2].Stok - obj.Vertices[v0].Stok);
					NB=obj.Vertices[v0].Norm + t2*(obj.Vertices[v2].Norm - obj.Vertices[v0].Norm);
					NB.Normalise();
					mB=map0 + t2*(map2 - map0);
					dB=d0 + t2*(d2-d0);



					if (cc==2)		// v0 point en dedans
					{
						dest.Vertices[np].Stok=A;
						dest.Vertices[np].Norm=NA;
						dest.Vertices[np].tag2=obj.Vertices[v0].tag2;
						dest.Vertices[np].tag=obj.Vertices[v0].tag;
						dest.Vertices[np].temp=obj.Vertices[v0].temp;
						dest.Vertices[np].Diffuse=dA;

						dest.Vertices[np+1].Stok=B;
						dest.Vertices[np+1].Norm=NB;
						dest.Vertices[np+1].tag2=obj.Vertices[v0].tag2;
						dest.Vertices[np+1].tag=obj.Vertices[v0].tag;
						dest.Vertices[np+1].temp=obj.Vertices[v0].temp;
						dest.Vertices[np+1].Diffuse=dB;

						dest.Faces[nf].Init(corres[v0],np,np+1);

						dest.Faces[nf].mp0=map0;
						dest.Faces[nf].mp1=mA;
						dest.Faces[nf].mp2=mB;
						dest.Faces[nf].nT=obj.Faces[n].nT;

						dest.Faces[nf].tag=obj.Faces[n].tag;
						dest.Faces[nf].tag2=obj.Faces[n].tag2;

						nf++;
						np+=2;
					}
					else
					{
						dest.Vertices[np].Stok=A;
						dest.Vertices[np].Norm=NA;
						dest.Vertices[np].tag2=obj.Vertices[v0].tag2;
						dest.Vertices[np].tag=obj.Vertices[v0].tag;
						dest.Vertices[np].temp=obj.Vertices[v0].temp;
						dest.Vertices[np].Diffuse=dA;

						dest.Vertices[np+1].Stok=B;
						dest.Vertices[np+1].Norm=NB;
						dest.Vertices[np+1].tag2=obj.Vertices[v0].tag2;
						dest.Vertices[np+1].tag=obj.Vertices[v0].tag;
						dest.Vertices[np+1].temp=obj.Vertices[v0].temp;
						dest.Vertices[np+1].Diffuse=dB;

						dest.Faces[nf].Init(corres[v2],np+1,corres[v1]);

						dest.Faces[nf].mp0=map2;
						dest.Faces[nf].mp1=mB;
						dest.Faces[nf].mp2=map1;
						dest.Faces[nf].nT=obj.Faces[n].nT;

						dest.Faces[nf].tag=obj.Faces[n].tag;
						dest.Faces[nf].tag2=obj.Faces[n].tag2;

						nf++;

						dest.Faces[nf].Init(corres[v1],np+1,np);

						dest.Faces[nf].mp0=map1;
						dest.Faces[nf].mp1=mB;
						dest.Faces[nf].mp2=mA;

						dest.Faces[nf].nT=obj.Faces[n].nT;
						dest.Faces[nf].tag=obj.Faces[n].tag;
						dest.Faces[nf].tag2=obj.Faces[n].tag2;

						nf++;
						np+=2;
					}

				}
				else
				{

					//faire correspondre les sommets

/*
					v0=obj.Faces[n].v0;
					v1=obj.Faces[n].v1;
					v2=obj.Faces[n].v2;

					dest.Faces[nf].Init(corres[v0],corres[v1],corres[v2]);
					dest.Faces[nf].mp0=obj.Faces[n].mp0;
					dest.Faces[nf].mp1=obj.Faces[n].mp1;
					dest.Faces[nf].mp2=obj.Faces[n].mp2;
					dest.Faces[nf].nT=obj.Faces[n].nT;
					nf++;
*/

				}

			}
	}

	dest.nFaces=nf;
	dest.nVertices=np;

	free(corres);
	free(tags);
	free(tagsf);

	return dest;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::PortionCarree(float x1,float y1,float x2,float y2,int nT,float dx,float dy)
{
    float A[4],B[4],C[4],D[4];
    int n;
    CObject3D nouveau[4];
    A[0]=1;
    B[0]=0;
    C[0]=0;
    D[0]=-x2;
    
    A[1]=0;
    B[1]=1;
    C[1]=0;
    D[1]=-y2;
    
    A[2]=-1;
    B[2]=0;
    C[2]=0;
    D[2]=+x1;
    
    A[3]=0;
    B[3]=-1;
    C[3]=0;
    D[3]=+y1;
    
    nouveau[0]=CutPlane((*this),A[0],B[0],C[0],D[0]);
    for (n=1;n<4;n++)
    {
        nouveau[n]=CutPlane(nouveau[n-1],A[n],B[n],C[n],D[n]);
    }
    
    nouveau[0].Free();
    nouveau[1].Free();
    nouveau[2].Free();
    
    for (n=0;n<nouveau[3].nFaces;n++)
    {
        nouveau[3].Faces[n].nT=nT;
        nouveau[3].Faces[n].mp0.Mod1(dx,dy);
        nouveau[3].Faces[n].mp1.Mod1(dx,dy);
        nouveau[3].Faces[n].mp2.Mod1(dx,dy);
    }
    
    return (nouveau[3]);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		geometry smoothing group

		void CObject3D::SmoothingGroupFaces();

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SmoothingGroupFaces()
{
	int n;
	CObject3D * obj1=DuplicateSelected2(0);
	CObject3D * obj2=DuplicateSelected2(1);
	CGenerator *G;

	G=new CGenerator(nVertices*2,nFaces*2);
    
    int bakStatus=Status;

	delete [] Vertices;
	delete [] Faces;

	G->Reset();
	if (obj1) G->Add(obj1);
	if (obj2) G->Add(obj2);
	
	Init(G->nVertices,G->nFaces);

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok=G->Vertices[n].Stok;
		Vertices[n].Map=G->Vertices[n].Map;
		//Vertices[n].Map2=G->Vertices[n].Map2;
		//Vertices[n].Diffuse=G->Vertices[n].Diffuse;
		Vertices[n].Index[0]=G->Vertices[n].Index[0];
		Vertices[n].Index[1]=G->Vertices[n].Index[1];
		Vertices[n].Index[2]=G->Vertices[n].Index[2];
		Vertices[n].Index[3]=G->Vertices[n].Index[3];
		Vertices[n].Weight[0]=G->Vertices[n].Weight[0];
		Vertices[n].Weight[1]=G->Vertices[n].Weight[1];
		Vertices[n].Weight[2]=G->Vertices[n].Weight[2];
		Vertices[n].Weight[3]=G->Vertices[n].Weight[3];
	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].v0=G->Faces[n].v0;
		Faces[n].v1=G->Faces[n].v1;
		Faces[n].v2=G->Faces[n].v2;
		Faces[n].Norm=G->Faces[n].Norm;
		Faces[n].tag=0;
		if (n>=obj1->nFaces) Faces[n].tag2=1;
		else Faces[n].tag2=0;
		Faces[n].nT=G->Faces[n].nT;
		Faces[n].nT2=G->Faces[n].nT2;
		Faces[n].ref=G->Faces[n].ref;
		Faces[n].nL=G->Faces[n].nL;
		Faces[n].nLVectors=G->Faces[n].nLVectors;
		//Faces[n].Diffuse=G->Faces[n].Diffuse;
		Faces[n].mp0=G->Faces[n].mp0;
		Faces[n].mp1=G->Faces[n].mp1;
		Faces[n].mp2=G->Faces[n].mp2;
		Faces[n].size_lightmap=G->Faces[n].size_lightmap;
	}

	SetFaces();
	CalculateNormals(-1);
    
    Status=bakStatus;

	update=true;

    if (obj1)
    {
        obj1->Free();
        delete obj1;
    }
	
    if (obj1)
    {
        obj2->Free();
        delete obj2;
    }
	delete G;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		merge vertices

		CObject3D CObject3D::Regroupe()
		CObject3D* CObject3D::Regroupe2()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */



CObject3D CObject3D::Regroupe()
{
	return (*Regroupe2());
}

CObject3D* CObject3D::Regroupe2()
{
	return Regroupe2(SMALLF2);
}

CObject3D* CObject3D::Regroupe2(float d)
{
	int nf;
	CVector D;
	CObject3D* simplificated;
	int n,k,nn,nnn,res;
	float K=(float)((int) (1.0f/d));

	simplificated=new CObject3D;
	naming(simplificated->Name,Name);

	nnn=0;
	for (n=0;n<nVertices;n++)
	{
		Vertices[n].tag=1;
		int x=(int)(Vertices[n].Stok.x*K);
		int y=(int)(Vertices[n].Stok.y*K);
		int z=(int)(Vertices[n].Stok.z*K);
		res=0;
		nn=0;
		while ((nn<n)&&(res==0))
		{
			int xx=(int)(Vertices[nn].Stok.x*K);
			if (xx==x)
			{
				int yy=(int)(Vertices[nn].Stok.y*K);
				if (yy==y)
				{
					int zz=(int)(Vertices[nn].Stok.z*K);
					if (zz==z)
					{
						Vertices[n].tag=0;
						Vertices[n].tag2=Vertices[nn].tag2;
						res=1;
					}
				}
			}
			nn++;
		}

		if (res==0)
		{
			Vertices[n].tag2=nnn;
			nnn++;
		}
	}

	simplificated->Init(nnn,nFaces);

	if (nKeys>0)
	{
		simplificated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			simplificated->VerticesKeys[k]=new CShortVertex[nVertices];
		}
	}

	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==1)
		{
			simplificated->Vertices[nn].Stok=Vertices[n].Stok;
			simplificated->Vertices[nn].Norm=Vertices[n].Norm;

			if (nKeys>0)
			{
				simplificated->nKeys=nKeys;
				for (k=0;k<nKeys;k++)
					simplificated->VerticesKeys[k][nn]=VerticesKeys[k][n];
			}

			simplificated->Vertices[nn].Map=Vertices[n].Map;
			//simplificated->Vertices[nn].Map2=Vertices[n].Map2;
			//simplificated->Vertices[nn].Diffuse=Vertices[n].Diffuse;
			simplificated->Vertices[nn].Index[0]=Vertices[n].Index[0];
			simplificated->Vertices[nn].Index[1]=Vertices[n].Index[1];
			simplificated->Vertices[nn].Index[2]=Vertices[n].Index[2];
			simplificated->Vertices[nn].Index[3]=Vertices[n].Index[3];
			simplificated->Vertices[nn].Weight[0]=Vertices[n].Weight[0];
			simplificated->Vertices[nn].Weight[1]=Vertices[n].Weight[1];
			simplificated->Vertices[nn].Weight[2]=Vertices[n].Weight[2];
			simplificated->Vertices[nn].Weight[3]=Vertices[n].Weight[3];
			nn++;
		}
	}

	nf=0;
	for (n=0;n<nFaces;n++)
	{
		if ((Vertices[Faces[n].v0].tag2!=Vertices[Faces[n].v1].tag2)||
			(Vertices[Faces[n].v1].tag2!=Vertices[Faces[n].v2].tag2)||
			(Vertices[Faces[n].v2].tag2!=Vertices[Faces[n].v0].tag2))
		{
			simplificated->Faces[nf].v0=Vertices[Faces[n].v0].tag2;
			simplificated->Faces[nf].v1=Vertices[Faces[n].v1].tag2;
			simplificated->Faces[nf].v2=Vertices[Faces[n].v2].tag2;

			simplificated->Faces[nf].Norm=Faces[n].Norm;
			simplificated->Faces[nf].tag=Faces[n].tag;
			simplificated->Faces[nf].nT=Faces[n].nT;
			simplificated->Faces[nf].nT2=Faces[n].nT2;
			simplificated->Faces[nf].ref=Faces[n].ref;
			simplificated->Faces[nf].tag2=(short int)n;

			//simplificated->Faces[nf].Diffuse=Faces[n].Diffuse;
			simplificated->Faces[nf].mp0=Faces[n].mp0;
			simplificated->Faces[nf].mp1=Faces[n].mp1;
			simplificated->Faces[nf].mp2=Faces[n].mp2;
			nf++;
		}
	}

	simplificated->nFaces=nf;
	simplificated->SetFaces();

	simplificated->Rot=Rot;
	simplificated->Coo=Coo;
	for (n=0;n<6;n++) simplificated->P[n]=P[n];
	simplificated->nP=nP;
	for (n=0;n<16;n++) simplificated->Tab[n]=Tab[n];
	simplificated->Status=Status;
	simplificated->Attribut=Attribut;
	simplificated->Tag=Tag;
	simplificated->nurbs=nurbs;
	simplificated->env_mapping=env_mapping;
	simplificated->group=group;
	return simplificated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        void CObject3D::Minimal()

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Minimal()
{
    int n;
    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    
    CObject3D * tmp=DuplicateSelected2(0);
    if (tmp)
    {
        delete [] Vertices;
        delete [] Faces;
        
        Vertices=tmp->Vertices;
        nVertices=tmp->nVertices;
        
        Faces=tmp->Faces;
        nFaces=tmp->nFaces;

        tmp->Vertices=NULL;
        tmp->Faces=NULL;
        
        tmp->Free();
        delete tmp;
    }
    update=true;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate selected faces (accordingly to .tag)

		CObject3D CObject3D::DuplicateSelected(int tag)
		CObject3D* CObject3D::DuplicateSelected2(int tag)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D* CObject3D::DuplicateVerticesSelected(int tag)
{
	int n;
	CObject3D *duplicated;
	int nb=0;

	for (n=0;n<nVertices;n++)
		if (Vertices[n].tag==tag) nb++;

	if (nb>0)
	{
		duplicated=new CObject3D;

		naming(duplicated->Name,Name);

		duplicated->Init(nb,0);

		duplicated->Vertices=new CVertex[nb];
								
		int p=0;
		for (n=0;n<nVertices;n++)
			if (Vertices[n].tag==tag) duplicated->Vertices[p++]=Vertices[n];

		duplicated->nVertices=nb;

		if (anim_strings)
		{
			duplicated->anim_strings=(char*) malloc(anim_stringlen(this)+1);
			memset(duplicated->anim_strings,0,anim_stringlen(this)+1);
			memcpy(duplicated->anim_strings,anim_strings,anim_stringlen(this));
		}

		duplicated->Rot=Rot;
		duplicated->Coo=Coo;

		for (n=0;n<6;n++)
			duplicated->P[n]=P[n];

		duplicated->nP=nP;

		for (n=0;n<16;n++)
			duplicated->Tab[n]=Tab[n];

		duplicated->Status=Status;
		duplicated->nurbs=nurbs;
		duplicated->Status2=Status2;
		duplicated->Attribut=Attribut;

		duplicated->Tag=Tag;
		duplicated->env_mapping=env_mapping;
		duplicated->group=group;

		return duplicated;
	}
	else return NULL;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate selected faces (accordingly to .tag)

		CObject3D CObject3D::DuplicateSelected(int tag)
		CObject3D* CObject3D::DuplicateSelected2(int tag)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CObject3D::DuplicateSelected(int tag)
{
	return (*DuplicateSelected2(tag));
}

CObject3D* CObject3D::DuplicateSelected2(int tag)
{
	int * corres;
	CObject3D *duplicated;
	int n,k,nn;
	int nf,np;

	corres=(int *) malloc(4*nVertices);

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;

	nf=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag==tag)
		{
			Vertices[Faces[n].v0].tag=1;
			Vertices[Faces[n].v1].tag=1;
			Vertices[Faces[n].v2].tag=1;
			nf++;
		}
	}

	np=0;
	for (n=0;n<nVertices;n++)
	{
		corres[n]=-1;
		if (Vertices[n].tag==1)
		{
			corres[n]=np;
			np++;
		}
	}
    
    if (np==0) { free(corres); return NULL; }
    if (nf==0) { free(corres); return NULL; }
    
    duplicated=new CObject3D;

    naming(duplicated->Name,Name);

	duplicated->Init(np,nf);

	for (k=0;k<nKeys;k++) duplicated->VerticesKeys[k]=new CShortVertex[np];
	duplicated->nKeys=nKeys;

	if (anim_strings)
	{
		duplicated->anim_strings=(char*) malloc(anim_stringlen(this)+1);
		memset(duplicated->anim_strings,0,anim_stringlen(this)+1);
		memcpy(duplicated->anim_strings,anim_strings,anim_stringlen(this));
	}

	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==1)
		{
			duplicated->Vertices[nn].Stok=Vertices[n].Stok;
			duplicated->Vertices[nn].Norm=Vertices[n].Norm;
			duplicated->Vertices[nn].Map=Vertices[n].Map;
			//duplicated->Vertices[nn].Map2=Vertices[n].Map2;
			duplicated->Vertices[nn].tag2=Vertices[n].tag2;
			//duplicated->Vertices[nn].Diffuse=Vertices[n].Diffuse;
			duplicated->Vertices[nn].Index[0]=Vertices[n].Index[0];
			duplicated->Vertices[nn].Index[1]=Vertices[n].Index[1];
			duplicated->Vertices[nn].Index[2]=Vertices[n].Index[2];
			duplicated->Vertices[nn].Index[3]=Vertices[n].Index[3];
			duplicated->Vertices[nn].Weight[0]=Vertices[n].Weight[0];
			duplicated->Vertices[nn].Weight[1]=Vertices[n].Weight[1];
			duplicated->Vertices[nn].Weight[2]=Vertices[n].Weight[2];
			duplicated->Vertices[nn].Weight[3]=Vertices[n].Weight[3];
			for (k=0;k<nKeys;k++)
			{
				duplicated->VerticesKeys[k][nn].Stok=VerticesKeys[k][n].Stok;
				duplicated->VerticesKeys[k][nn].Norm=VerticesKeys[k][n].Norm;
			}
			nn++;
		}
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag==tag)
		{
			duplicated->Faces[nn].v0=corres[Faces[n].v0];
			duplicated->Faces[nn].v1=corres[Faces[n].v1];
			duplicated->Faces[nn].v2=corres[Faces[n].v2];
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=Faces[n].mp0;
			duplicated->Faces[nn].mp1=Faces[n].mp1;
			duplicated->Faces[nn].mp2=Faces[n].mp2;
			nn++;
		}
	}


	duplicated->SetFaces();

	free(corres);

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

CObject3D CObject3D::DuplicateSelectedNot(int tag)
{
    return (*DuplicateSelectedNot2(tag));
}

CObject3D* CObject3D::DuplicateSelectedNot2(int tag)
{
    int * corres;
    CObject3D *duplicated;
    int n,k,nn;
    int nf,np;
    
    corres=(int *) malloc(4*nVertices);
    
    duplicated=new CObject3D;
    
    naming(duplicated->Name,Name);
    
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    
    nf=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag!=tag)
        {
            Vertices[Faces[n].v0].tag=1;
            Vertices[Faces[n].v1].tag=1;
            Vertices[Faces[n].v2].tag=1;
            nf++;
        }
    }
    
    np=0;
    for (n=0;n<nVertices;n++)
    {
        corres[n]=-1;
        if (Vertices[n].tag==1)
        {
            corres[n]=np;
            np++;
        }
    }
    
    duplicated->Init(np,nf);
    
    for (k=0;k<nKeys;k++) duplicated->VerticesKeys[k]=new CShortVertex[np];
    duplicated->nKeys=nKeys;
    
    if (anim_strings)
    {
        duplicated->anim_strings=(char*) malloc(anim_stringlen(this)+1);
        memset(duplicated->anim_strings,0,anim_stringlen(this)+1);
        memcpy(duplicated->anim_strings,anim_strings,anim_stringlen(this));
    }
    
    nn=0;
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].tag==1)
        {
            duplicated->Vertices[nn].Stok=Vertices[n].Stok;
            duplicated->Vertices[nn].Norm=Vertices[n].Norm;
            duplicated->Vertices[nn].Map=Vertices[n].Map;
            //duplicated->Vertices[nn].Map2=Vertices[n].Map2;
            duplicated->Vertices[nn].tag2=Vertices[n].tag2;
            //duplicated->Vertices[nn].Diffuse=Vertices[n].Diffuse;
            duplicated->Vertices[nn].Index[0]=Vertices[n].Index[0];
            duplicated->Vertices[nn].Index[1]=Vertices[n].Index[1];
            duplicated->Vertices[nn].Index[2]=Vertices[n].Index[2];
            duplicated->Vertices[nn].Index[3]=Vertices[n].Index[3];
            duplicated->Vertices[nn].Weight[0]=Vertices[n].Weight[0];
            duplicated->Vertices[nn].Weight[1]=Vertices[n].Weight[1];
            duplicated->Vertices[nn].Weight[2]=Vertices[n].Weight[2];
            duplicated->Vertices[nn].Weight[3]=Vertices[n].Weight[3];
            for (k=0;k<nKeys;k++)
            {
                duplicated->VerticesKeys[k][nn].Stok=VerticesKeys[k][n].Stok;
                duplicated->VerticesKeys[k][nn].Norm=VerticesKeys[k][n].Norm;
            }
            nn++;
        }
    }
    
    nn=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag!=tag)
        {
            duplicated->Faces[nn].v0=corres[Faces[n].v0];
            duplicated->Faces[nn].v1=corres[Faces[n].v1];
            duplicated->Faces[nn].v2=corres[Faces[n].v2];
            duplicated->Faces[nn].Norm=Faces[n].Norm;
            duplicated->Faces[nn].tag=Faces[n].tag;
            duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].nT=Faces[n].nT;
            duplicated->Faces[nn].nT2=Faces[n].nT2;
            duplicated->Faces[nn].ref=Faces[n].ref;
            duplicated->Faces[nn].nL=Faces[n].nL;
            duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
            //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].mp0=Faces[n].mp0;
            duplicated->Faces[nn].mp1=Faces[n].mp1;
            duplicated->Faces[nn].mp2=Faces[n].mp2;
            nn++;
        }
    }
    
    
    duplicated->SetFaces();
    
    free(corres);
    
    duplicated->Rot=Rot;
    duplicated->Coo=Coo;
    
    for (n=0;n<6;n++)
        duplicated->P[n]=P[n];
    
    duplicated->nP=nP;
    
    
    for (n=0;n<16;n++)
        duplicated->Tab[n]=Tab[n];
    
    duplicated->Status=Status;
    duplicated->nurbs=nurbs;
    duplicated->Status2=Status2;
    duplicated->Attribut=Attribut;
    
    duplicated->Tag=Tag;
    duplicated->env_mapping=env_mapping;
    duplicated->group=group;
    
    return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate selected faces (accordingly to .tag)

		CObject3D CObject3D::DuplicateSelected_nT(int tag)
		CObject3D* CObject3D::DuplicateSelected_nT2(int tag)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CObject3D::DuplicateSelected_nT(int nT)
{
	return (*DuplicateSelected_nT2(nT));
}

CObject3D* CObject3D::DuplicateSelected_nT2(int nT)
{
	int * corres;
	CObject3D *duplicated;
	int n,nn;
	int nf,np;


	if (nT>=0)
	{
		corres=(int *) malloc(4*nVertices);
		duplicated=new CObject3D;
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;

		nf=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].nT==nT)
			{
				Vertices[Faces[n].v0].tag=1;
				Vertices[Faces[n].v1].tag=1;
				Vertices[Faces[n].v2].tag=1;
				nf++;
			}
		}

		np=0;
		for (n=0;n<nVertices;n++)
		{
			corres[n]=-1;
			if (Vertices[n].tag==1)
			{
				corres[n]=np;
				np++;
			}
		}

		duplicated->Init(np,nf);
		nn=0;
		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag==1)
			{
				duplicated->Vertices[nn].Stok=Vertices[n].Stok;
				duplicated->Vertices[nn].Norm=Vertices[n].Norm;
				duplicated->Vertices[nn].Map=Vertices[n].Map;
				//duplicated->Vertices[nn].Map2=Vertices[n].Map2;
				//duplicated->Vertices[nn].Diffuse=Vertices[n].Diffuse;
				nn++;
			}
		}

		nn=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].nT==nT)
			{
				duplicated->Faces[nn].v0=corres[Faces[n].v0];
				duplicated->Faces[nn].v1=corres[Faces[n].v1];
				duplicated->Faces[nn].v2=corres[Faces[n].v2];
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
				duplicated->Faces[nn].ref=Faces[n].ref;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=Faces[n].mp0;
				duplicated->Faces[nn].mp1=Faces[n].mp1;
				duplicated->Faces[nn].mp2=Faces[n].mp2;
				nn++;
			}
		}
		duplicated->SetFaces();
		free(corres);
	}
	else
	{
		corres=(int *) malloc(4*nVertices);
		duplicated=new CObject3D;
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;

		nf=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].nT!=-nT-1)
			{
				Vertices[Faces[n].v0].tag=1;
				Vertices[Faces[n].v1].tag=1;
				Vertices[Faces[n].v2].tag=1;
				nf++;
			}
		}

		np=0;
		for (n=0;n<nVertices;n++)
		{
			corres[n]=-1;
			if (Vertices[n].tag==1)
			{
				corres[n]=np;
				np++;
			}
		}

		duplicated->Init(np,nf);
		nn=0;
		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag==1)
			{
				duplicated->Vertices[nn].Stok=Vertices[n].Stok;
				duplicated->Vertices[nn].Norm=Vertices[n].Norm;
				duplicated->Vertices[nn].Map=Vertices[n].Map;
				//duplicated->Vertices[nn].Map2=Vertices[n].Map2;
				//duplicated->Vertices[nn].Diffuse=Vertices[n].Diffuse;
				nn++;
			}
		}

		nn=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].nT!=-nT-1)
			{
				duplicated->Faces[nn].v0=corres[Faces[n].v0];
				duplicated->Faces[nn].v1=corres[Faces[n].v1];
				duplicated->Faces[nn].v2=corres[Faces[n].v2];
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
				duplicated->Faces[nn].ref=Faces[n].ref;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=Faces[n].mp0;
				duplicated->Faces[nn].mp1=Faces[n].mp1;
				duplicated->Faces[nn].mp2=Faces[n].mp2;
				nn++;
			}
		}

		duplicated->SetFaces();
		free(corres);
	}

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		duplicate and inverse orientation

		CObject3D CObject3D::Inverse()
		CObject3D* CObject3D::Inverse2()


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CObject3D::Inverse()
{
	return (*Inverse2());
}


CObject3D* CObject3D::Inverse2()
{
	CObject3D* duplicated;
	int n,k;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	duplicated->Init(nVertices,nFaces);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
		duplicated->Vertices[n].Norm.x=-Vertices[n].Norm.x;
		duplicated->Vertices[n].Norm.y=-Vertices[n].Norm.y;
		duplicated->Vertices[n].Norm.z=-Vertices[n].Norm.z;
		duplicated->Vertices[n].Map=Vertices[n].Map;
		//duplicated->Vertices[n].Map2=Vertices[n].Map2;
		//duplicated->Vertices[n].Diffuse=Vertices[n].Diffuse;
        
        duplicated->Vertices[n].tag=Vertices[n].tag;
        duplicated->Vertices[n].tag2=Vertices[n].tag2;
        
        duplicated->Vertices[n].Index[0]=Vertices[n].Index[0];
        duplicated->Vertices[n].Index[1]=Vertices[n].Index[1];
        duplicated->Vertices[n].Index[2]=Vertices[n].Index[2];
        duplicated->Vertices[n].Index[3]=Vertices[n].Index[3];
        
        duplicated->Vertices[n].Weight[0]=Vertices[n].Weight[0];
        duplicated->Vertices[n].Weight[1]=Vertices[n].Weight[1];
        duplicated->Vertices[n].Weight[2]=Vertices[n].Weight[2];
        duplicated->Vertices[n].Weight[3]=Vertices[n].Weight[3];
	}

	for (n=0;n<nFaces;n++)
	{
		duplicated->Faces[n]=Faces[n];

		duplicated->Faces[n].v0=Faces[n].v0;
		duplicated->Faces[n].v1=Faces[n].v2;
		duplicated->Faces[n].v2=Faces[n].v1;
        
        duplicated->Faces[n].tag=Faces[n].tag;
        duplicated->Faces[n].tag2=Faces[n].tag2;
        
        duplicated->Faces[n].mp0=Faces[n].mp0;
        duplicated->Faces[n].mp1=Faces[n].mp2;
        duplicated->Faces[n].mp2=Faces[n].mp1;

		duplicated->Faces[n].Norm.x=-Faces[n].Norm.x;
		duplicated->Faces[n].Norm.y=-Faces[n].Norm.y;
		duplicated->Faces[n].Norm.z=-Faces[n].Norm.z;
	}
    
    if (nKeys>0)
    {
        duplicated->nKeys=nKeys;
        for (k=0;k<nKeys;k++)
        {
            duplicated->VerticesKeys[k]=new CShortVertex[nVertices];
            for (n=0;n<nVertices;n++) duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
        }
    }
    
    if (anim_strings)
    {
        duplicated->anim_strings=(char*) malloc(anim_stringlen(this)+1);
        memset(duplicated->anim_strings,0,anim_stringlen(this)+1);
        memcpy(duplicated->anim_strings,anim_strings,anim_stringlen(this));
    }

	duplicated->SetFaces();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;

	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;

	duplicated->group=group;

	return duplicated;

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		normalize vertex corrdinate to 1 from center (0,0,0)

		CObject3D CObject3D::Normalize()
		CObject3D * CObject3D::Normalize2()

	Usage:

		after tesselation on a cube, transformation to sphere

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D * CObject3D::Normalize2()
{
	CObject3D *duplicated;
	int n;
	float r=CalculateRadius();

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	duplicated->Init(nVertices,nFaces);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
		duplicated->Vertices[n].Stok*=r/duplicated->Vertices[n].Stok.Norme();
	}

	for (n=0;n<nFaces;n++)
	{
		duplicated->Faces[n]=Faces[n];
	}


	duplicated->SetFaces();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

CObject3D CObject3D::Normalize()
{
	return (*Normalize2());
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four

		CObject3D CObject3D::Tesselate()
		CObject3D * CObject3D::Tesselate2()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CObject3D::Tesselate()
{
	return (*Tesselate2());
}



CObject3D * CObject3D::Tesselate2full()
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
    QuickEdges ed;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

    ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
        ed.Add(Faces[n].v0,Faces[n].v1);
        ed.Add(Faces[n].v1,Faces[n].v2);
        ed.Add(Faces[n].v2,Faces[n].v0);
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nFaces*4);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	for (n=0;n<ed.Nb();n++)
	{
		int a=ed.a(n);
		int b=ed.b(n);
		duplicated->Vertices[nVertices+n].Stok=(Vertices[a].Stok + Vertices[b].Stok)*0.5f;
		duplicated->Vertices[nVertices+n].Calc=(Vertices[a].Calc + Vertices[b].Calc)*0.5f;
        duplicated->Vertices[nVertices+n].Norm=(Vertices[a].Norm + Vertices[b].Norm);
        duplicated->Vertices[nVertices+n].Norm.Normalise();
        duplicated->Vertices[nVertices+n].NormCalc=(Vertices[a].NormCalc + Vertices[b].NormCalc);
        duplicated->Vertices[nVertices+n].NormCalc.Normalise();
        
		duplicated->Vertices[nVertices+n].Map=(Vertices[a].Map+Vertices[b].Map)*0.5f;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)*0.5f;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)*0.5f;

		duplicated->Vertices[nVertices+n].Weight[0]=Vertices[a].Weight[0];
		duplicated->Vertices[nVertices+n].Weight[1]=Vertices[a].Weight[1];
		duplicated->Vertices[nVertices+n].Weight[2]=Vertices[a].Weight[2];
		duplicated->Vertices[nVertices+n].Weight[3]=Vertices[a].Weight[3];

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[a].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[a].Index[1];
		duplicated->Vertices[nVertices+n].Index[2]=Vertices[a].Index[2];
		duplicated->Vertices[nVertices+n].Index[3]=Vertices[a].Index[3];
        
        if ((Vertices[a].tag2)&&(Vertices[b].tag2)) duplicated->Vertices[nVertices+n].tag2=1;
        else duplicated->Vertices[nVertices+n].tag2=0;
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		m0=nVertices+ed.Witch(v0,v1);
		m1=nVertices+ed.Witch(v1,v2);
		m2=nVertices+ed.Witch(v2,v0);

		mpv0=Faces[n].mp0;
		mpv1=Faces[n].mp1;
		mpv2=Faces[n].mp2;

		mpm0=(mpv0+mpv1)/2;
		mpm1=(mpv1+mpv2)/2;
		mpm2=(mpv2+mpv0)/2;

		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v0;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m2;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].NormCalc=Faces[n].NormCalc;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
        duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv0;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm2;
		nn++;


		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=m2;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].NormCalc=Faces[n].NormCalc;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpm2;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;


		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v1;
		duplicated->Faces[nn].v1=m1;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].NormCalc=Faces[n].NormCalc;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv1;
		duplicated->Faces[nn].mp1=mpm1;
		duplicated->Faces[nn].mp2=mpm0;
		nn++;

		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v2;
		duplicated->Faces[nn].v1=m2;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].NormCalc=Faces[n].NormCalc;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv2;
		duplicated->Faces[nn].mp1=mpm2;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;

	}

	duplicated->SetFaces();

	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;

	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];
	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

CObject3D * CObject3D::Tesselate2()
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
    QuickEdges ed;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

    ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
        ed.Add(Faces[n].v0,Faces[n].v1);
        ed.Add(Faces[n].v1,Faces[n].v2);
        ed.Add(Faces[n].v2,Faces[n].v0);
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nFaces*4);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	for (n=0;n<ed.Nb();n++)
	{
		int a=ed.a(n);
		int b=ed.b(n);
		duplicated->Vertices[nVertices+n].Stok=(Vertices[a].Stok + Vertices[b].Stok)*0.5f;
        duplicated->Vertices[nVertices+n].Norm=(Vertices[a].Norm + Vertices[b].Norm);
        duplicated->Vertices[nVertices+n].Norm.Normalise();
        
		duplicated->Vertices[nVertices+n].Map=(Vertices[a].Map+Vertices[b].Map)*0.5f;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)*0.5f;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)*0.5f;

		duplicated->Vertices[nVertices+n].Weight[0]=Vertices[a].Weight[0];
		duplicated->Vertices[nVertices+n].Weight[1]=Vertices[a].Weight[1];
		duplicated->Vertices[nVertices+n].Weight[2]=Vertices[a].Weight[2];
		duplicated->Vertices[nVertices+n].Weight[3]=Vertices[a].Weight[3];

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[a].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[a].Index[1];
		duplicated->Vertices[nVertices+n].Index[2]=Vertices[a].Index[2];
		duplicated->Vertices[nVertices+n].Index[3]=Vertices[a].Index[3];
        
        if ((Vertices[a].tag2)&&(Vertices[b].tag2)) duplicated->Vertices[nVertices+n].tag2=1;
        else duplicated->Vertices[nVertices+n].tag2=0;
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		m0=nVertices+ed.Witch(v0,v1);
		m1=nVertices+ed.Witch(v1,v2);
		m2=nVertices+ed.Witch(v2,v0);

		mpv0=Faces[n].mp0;
		mpv1=Faces[n].mp1;
		mpv2=Faces[n].mp2;

		mpm0=(mpv0+mpv1)/2;
		mpm1=(mpv1+mpv2)/2;
		mpm2=(mpv2+mpv0)/2;

		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v0;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m2;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
        duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv0;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm2;
		nn++;


		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=m2;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpm2;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;


		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v1;
		duplicated->Faces[nn].v1=m1;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv1;
		duplicated->Faces[nn].mp1=mpm1;
		duplicated->Faces[nn].mp2=mpm0;
		nn++;

		duplicated->Faces[nn]=Faces[n];
		duplicated->Faces[nn].v0=v2;
		duplicated->Faces[nn].v1=m2;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
		duplicated->Faces[nn].tag2=Faces[n].tag2;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv2;
		duplicated->Faces[nn].mp1=mpm2;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;

	}

	duplicated->SetFaces();

	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;

	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];
	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four

		CObject3D CObject3D::SplitEdges()
		CObject3D * CObject3D::SplitEdges2()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::SplitEdges()
{
	return (*SplitEdges2());
}

CObject3D * CObject3D::SplitEdges2()
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int cc,nf;
	int e[3];
	QuickEdges ed;
	int a,b,c,d,f;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
		if ((Faces[n].v[0]->tag)&&(Faces[n].v[1]->tag)) ed.Add(Faces[n].v0,Faces[n].v1);
		if ((Faces[n].v[1]->tag)&&(Faces[n].v[2]->tag)) ed.Add(Faces[n].v1,Faces[n].v2);
		if ((Faces[n].v[2]->tag)&&(Faces[n].v[0]->tag)) ed.Add(Faces[n].v2,Faces[n].v0);
	}
    
    ed.Fix();

	nf=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		cc=0;
		e[0]=e[1]=e[2]=-1;

		if ((Faces[n].v[0]->tag)&&(Faces[n].v[1]->tag)) { e[0]=nVertices+ed.Witch(v0,v1); cc++; }
		if ((Faces[n].v[1]->tag)&&(Faces[n].v[2]->tag)) { e[1]=nVertices+ed.Witch(v1,v2); cc++; }
		if ((Faces[n].v[2]->tag)&&(Faces[n].v[0]->tag)) { e[2]=nVertices+ed.Witch(v2,v0); cc++; }

		switch (cc)
		{
		case 0:
			nf++;
			break;
		case 1:
			nf+=2;
			break;
		case 2:
			nf+=3;
			break;
		case 3:
			nf+=4;
			break;
		};
	}

	duplicated->Init(nVertices+ed.Nb(),nf);

	for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)*0.5f;
        duplicated->Vertices[nVertices+n].Stok0=(Vertices[ed.a(n)].Stok0 + Vertices[ed.b(n)].Stok0)*0.5f;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)*0.5f;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)*0.5f;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)*0.5f;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)*0.5f;


		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[2]=0;
		duplicated->Vertices[nVertices+n].Index[3]=0;
		duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
		duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
		duplicated->Vertices[nVertices+n].Weight[2]=0;
		duplicated->Vertices[nVertices+n].Weight[3]=0;
	}

    CVector2 mp0,mp1,mp2,mp01,mp12,mp20;
    
	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		cc=0; e[0]=e[1]=e[2]=-1;
		if ((Faces[n].v[0]->tag)&&(Faces[n].v[1]->tag)) { e[0]=nVertices+ed.Witch(v0,v1); cc++; }
		if ((Faces[n].v[1]->tag)&&(Faces[n].v[2]->tag)) { e[1]=nVertices+ed.Witch(v1,v2); cc++; }
		if ((Faces[n].v[2]->tag)&&(Faces[n].v[0]->tag)) { e[2]=nVertices+ed.Witch(v2,v0); cc++; }

		switch (cc)
		{
		case 0:
			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
                
                duplicated->Faces[nn].mp0=Faces[n].mp0;
                duplicated->Faces[nn].mp1=Faces[n].mp1;
                duplicated->Faces[nn].mp2=Faces[n].mp2;

            //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                
            duplicated->Faces[nn].coef1=Faces[n].coef1;
			nn++;
			break;

		case 1:
            if (e[0]>=0)
            { a=v0; b=v1; c=v2; d=e[0]; mp0=Faces[n].mp0; mp0=Faces[n].mp1; mp2=Faces[n].mp2; mp01=(Faces[n].mp0+Faces[n].mp1)/2; }
			if (e[1]>=0)
            { a=v1; b=v2; c=v0; d=e[1]; mp0=Faces[n].mp1; mp0=Faces[n].mp2; mp2=Faces[n].mp0; mp01=(Faces[n].mp1+Faces[n].mp2)/2; }
			if (e[2]>=0)
            { a=v2; b=v0; c=v1; d=e[2]; mp0=Faces[n].mp2; mp0=Faces[n].mp0; mp2=Faces[n].mp1; mp01=(Faces[n].mp2+Faces[n].mp0)/2; }

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=a;
			duplicated->Faces[nn].v1=d;
			duplicated->Faces[nn].v2=c;
                
                duplicated->Faces[nn].mp0=mp0;
                duplicated->Faces[nn].mp1=mp01;
                duplicated->Faces[nn].mp2=mp2;

			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=b;
			duplicated->Faces[nn].v1=c;
			duplicated->Faces[nn].v2=d;
                
                duplicated->Faces[nn].mp0=mp1;
                duplicated->Faces[nn].mp1=mp2;
                duplicated->Faces[nn].mp2=mp01;

			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;
			break;

		case 2:

			if (e[2]<0)
            { a=v0; b=v1; c=v2; d=e[0]; f=e[1]; mp0=Faces[n].mp0; mp0=Faces[n].mp1; mp2=Faces[n].mp2; mp01=(Faces[n].mp0+Faces[n].mp1)/2; mp12=(Faces[n].mp1+Faces[n].mp2)/2; }
			if (e[0]<0)
            { a=v1; b=v2; c=v0; d=e[1]; f=e[2];  mp0=Faces[n].mp1; mp0=Faces[n].mp2; mp2=Faces[n].mp0; mp01=(Faces[n].mp1+Faces[n].mp2)/2; mp12=(Faces[n].mp2+Faces[n].mp0)/2; }
			if (e[1]<0)
            { a=v2; b=v0; c=v1; d=e[2]; f=e[0];  mp0=Faces[n].mp2; mp0=Faces[n].mp0; mp2=Faces[n].mp1; mp01=(Faces[n].mp2+Faces[n].mp0)/2; mp12=(Faces[n].mp0+Faces[n].mp1)/2; }

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=b;
			duplicated->Faces[nn].v1=f;
			duplicated->Faces[nn].v2=d;
                
                duplicated->Faces[nn].mp0=mp1;
                duplicated->Faces[nn].mp1=mp12;
                duplicated->Faces[nn].mp2=mp01;

			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=d;
			duplicated->Faces[nn].v1=f;
			duplicated->Faces[nn].v2=c;

                duplicated->Faces[nn].mp0=mp01;
                duplicated->Faces[nn].mp1=mp12;
                duplicated->Faces[nn].mp2=mp2;
                
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=d;
			duplicated->Faces[nn].v1=c;
			duplicated->Faces[nn].v2=a;
                
                duplicated->Faces[nn].mp0=mp01;
                duplicated->Faces[nn].mp1=mp2;
                duplicated->Faces[nn].mp2=mp0;
                
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;
			break;

		case 3:
			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=e[0];
			duplicated->Faces[nn].v2=e[2];
                
                duplicated->Faces[nn].mp0=Faces[n].mp0;
                duplicated->Faces[nn].mp1=(Faces[n].mp0+Faces[n].mp1)/2;
                duplicated->Faces[nn].mp2=(Faces[n].mp2+Faces[n].mp0)/2;
                
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=e[2];
			duplicated->Faces[nn].v1=e[0];
			duplicated->Faces[nn].v2=e[1];
                
                duplicated->Faces[nn].mp0=(Faces[n].mp2+Faces[n].mp0)/2;
                duplicated->Faces[nn].mp1=(Faces[n].mp0+Faces[n].mp1)/2;
                duplicated->Faces[nn].mp2=(Faces[n].mp1+Faces[n].mp2)/2;
                
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=e[1];
			duplicated->Faces[nn].v2=e[0];
                
                duplicated->Faces[nn].mp0=Faces[n].mp1;
                duplicated->Faces[nn].mp1=(Faces[n].mp1+Faces[n].mp2)/2;
                duplicated->Faces[nn].mp2=(Faces[n].mp0+Faces[n].mp1)/2;

			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=e[2];
			duplicated->Faces[nn].v2=e[1];
                
                duplicated->Faces[nn].mp0=Faces[n].mp2;
                duplicated->Faces[nn].mp1=(Faces[n].mp2+Faces[n].mp0)/2;
                duplicated->Faces[nn].mp2=(Faces[n].mp1+Faces[n].mp2)/2;

			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].coef1=-1;
			nn++;
			break;
        };
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	//for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to .tag)

		CObject3D CObject3D::TesselateSelected(int tag)
		CObject3D* CObject3D::TesselateSelected2(int tag)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CObject3D::TesselateSelected(int tag)
{
	return (*TesselateSelected2(tag));
}

CObject3D* CObject3D::TesselateSelected2(int tag)
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	int n1,n2;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
    QuickEdges ed;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

    ed.Init(nVertices);

	n1=n2=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag==tag)
		{
            ed.Add(Faces[n].v0,Faces[n].v1);
            ed.Add(Faces[n].v1,Faces[n].v2);
            ed.Add(Faces[n].v2,Faces[n].v0);
			n1++;
		}
		else n2++;
	}

    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),n1*4+n2);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
        duplicated->Vertices[nVertices+n].Stok0=(Vertices[ed.a(n)].Stok0 + Vertices[ed.b(n)].Stok0)/2;
        duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm);
        duplicated->Vertices[nVertices+n].Norm.Normalise();
        
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a].Map2+Vertices[ed.b].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a].Diffuse + Vertices[ed.b].Diffuse)/2;
        
        if ((Vertices[ed.a(n)].tag2)||(Vertices[ed.b(n)].tag2)) duplicated->Vertices[nVertices+n].tag2=1;
        else duplicated->Vertices[nVertices+n].tag2=1;

        duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
        duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.a(n)].Index[1];
        duplicated->Vertices[nVertices+n].Index[2]=Vertices[ed.a(n)].Index[2];
        duplicated->Vertices[nVertices+n].Index[3]=Vertices[ed.a(n)].Index[3];
        
        duplicated->Vertices[nVertices+n].Weight[0]=Vertices[ed.a(n)].Weight[0];
        duplicated->Vertices[nVertices+n].Weight[1]=Vertices[ed.a(n)].Weight[1];
        duplicated->Vertices[nVertices+n].Weight[2]=Vertices[ed.a(n)].Weight[2];
        duplicated->Vertices[nVertices+n].Weight[3]=Vertices[ed.a(n)].Weight[3];
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag==tag)
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			m0=nVertices+ed.Witch(v0,v1);
			m1=nVertices+ed.Witch(v1,v2);
			m2=nVertices+ed.Witch(v2,v0);


			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			mpm0=(mpv0+mpv1)/2;
			mpm1=(mpv1+mpv2)/2;
			mpm2=(mpv2+mpv0)/2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm2;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			nn++;


			duplicated->Faces[nn].v0=m2;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpm2;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm1;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;

			nn++;


			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=m1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;


			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv1;
			duplicated->Faces[nn].mp1=mpm1;
			duplicated->Faces[nn].mp2=mpm0;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=m2;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv2;
			duplicated->Faces[nn].mp1=mpm2;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
            duplicated->Faces[nn].tag4=Faces[n].tag4;
            
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;

		}
	}

	duplicated->SetFaces();
	/*
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;
	/**/
	ed.Free();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->nurbs=nurbs;
	duplicated->Status=Status;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to .tag)

		CObject3D CObject3D::TesselateSelected(int tag)
		CObject3D* CObject3D::TesselateSelected2(int tag)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::RTPatch(float sign)
{
	return (*RTPatchSelected2(-1,sign));
}

CObject3D* CObject3D::RTPatch2(float sign)
{
	return RTPatchSelected2(-1,sign);
}

CObject3D CObject3D::RTPatchSelected(int tag,float sign)
{
	return (*RTPatchSelected2(tag,sign));
}

int EDGEN(CObject3D *obj,int a,int b)
{
	int res=-1;
	int n=0;
	while ((n<obj->nFaces)&&(res==-1))
	{
		if ((obj->Faces[n].v0==b)&&(obj->Faces[n].v1==a)) res=n;
		if ((obj->Faces[n].v1==b)&&(obj->Faces[n].v2==a)) res=n;
		if ((obj->Faces[n].v2==b)&&(obj->Faces[n].v0==a)) res=n;
		n++;
	}
	return res;
}

CObject3D* CObject3D::RTPatchSelected2(int tag,float sign)
{
    CObject3D *duplicated;
    
    SetMappingFacesFromVertices();
    
    CObject3D *tmp=ConvertLinearFull(SMALLF,100.0f);
    
    duplicated=tmp->RTPatchSelected2Basic(tag,sign);
    
    tmp->Free();
    delete tmp;
    
    duplicated->SetMappingVerticesFromFaces();
    
    return duplicated;
}

CObject3D* CObject3D::RTPatchSelected2Basic(int tag,float sign)
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	int n1,n2;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
	float r=CalculateRadius();

	duplicated=new CObject3D;
	naming(duplicated->Name,Name);

    QuickEdges ed;
    ed.Init(nVertices);

	n1=n2=0;
	for (n=0;n<nFaces;n++)
	{
		if ((Faces[n].tag==tag)||(tag==-1))
		{
            ed.Add(Faces[n].v0,Faces[n].v1);
            ed.Add(Faces[n].v1,Faces[n].v2);
            ed.Add(Faces[n].v2,Faces[n].v0);
			n1++;
		}
		else n2++;
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),n1*4+n2);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
		duplicated->Vertices[nVertices+n].Norm.Normalise();
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a].Map2+Vertices[ed.b].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a].Diffuse + Vertices[ed.b].Diffuse)/2;

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[2]=0;
		duplicated->Vertices[nVertices+n].Index[3]=0;
		duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
		duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
		duplicated->Vertices[nVertices+n].Weight[2]=0;
		duplicated->Vertices[nVertices+n].Weight[3]=0;

		if (EDGEN(this,ed.a(n),ed.b(n))>=0)
		{
			CVector u=Vertices[ed.b(n)].Stok - Vertices[ed.a(n)].Stok;
			float uu=u.Norme();
			u.Normalise();
			float t=(Vertices[ed.a(n)].Norm||u)*sign*uu/r;
			duplicated->Vertices[nVertices+n].Stok+=t*duplicated->Vertices[nVertices+n].Norm;
		}
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		if ((Faces[n].tag==tag)||(tag==-1))
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			m0=nVertices+ed.Witch(v0,v1);
			m1=nVertices+ed.Witch(v1,v2);
			m2=nVertices+ed.Witch(v2,v0);

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			mpm0=(mpv0+mpv1)/2;
			mpm1=(mpv1+mpv2)/2;
			mpm2=(mpv2+mpv0)/2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm2;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			nn++;


			duplicated->Faces[nn].v0=m2;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpm2;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm1;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;

			nn++;


			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=m1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;


			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv1;
			duplicated->Faces[nn].mp1=mpm1;
			duplicated->Faces[nn].mp2=mpm0;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=m2;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].tag2=Faces[n].tag2;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv2;
			duplicated->Faces[nn].mp1=mpm2;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].tag2=Faces[n].tag2;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;

		}
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;

	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to face length)

		CObject3D CObject3D::TesselateLength(float len)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
CObject3D CObject3D::TesselateLength(float len)
{
	return (*TesselateLength2(len));
}

CObject3D* CObject3D::TesselateLength2(float len)
{
	CObject3D *duplicated;
	int n,k,nn,nnn;
	int v0,v1,v2;
	int m0,m1,m2,a,b,c;
	CVector u1,u2,u3;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2,mpa,mpb,mpc;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

    QuickEdges ed;
    ed.Init(nVertices);

	nn=0;
	nnn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

		if ((u1.Norme()>len)||(u2.Norme()>len)||(u3.Norme()>len))
		{
			if ((u1.Norme()<len)||(u2.Norme()<len)||(u3.Norme()<len))
			{
				nnn++;
				Faces[n].tag2=1;

				if (u1.Norme()<len)
				{
                    ed.Add(Faces[n].v1,Faces[n].v2);
                    ed.Add(Faces[n].v2,Faces[n].v0);
				}

				if (u2.Norme()<len)
				{
                    ed.Add(Faces[n].v0,Faces[n].v1);
                    ed.Add(Faces[n].v2,Faces[n].v0);
				}

				if (u3.Norme()<len)
				{
                    ed.Add(Faces[n].v1,Faces[n].v2);
                    ed.Add(Faces[n].v0,Faces[n].v1);
				}
			}
			else
			{				
				nn++;
				Faces[n].tag2=2;
                ed.Add(Faces[n].v0,Faces[n].v1);
                ed.Add(Faces[n].v1,Faces[n].v2);
                ed.Add(Faces[n].v2,Faces[n].v0);
			}
		}
		else Faces[n].tag2=0;
	}

    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nnn*3+nn*4+(nFaces-nn-nnn));

	if (nKeys>0)
	{
		duplicated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			duplicated->VerticesKeys[k]=new CShortVertex[nVertices+ed.Nb()];
		}
	}


	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];

		if (nKeys>0)
		{
			duplicated->nKeys=nKeys;
			for (k=0;k<nKeys;k++)
				duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
		}
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[2]=0;
		duplicated->Vertices[nVertices+n].Index[3]=0;
		duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
		duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
		duplicated->Vertices[nVertices+n].Weight[2]=0;
		duplicated->Vertices[nVertices+n].Weight[3]=0;

		if (nKeys>0)
		{
			duplicated->nKeys=nKeys;
			for (k=0;k<nKeys;k++)
				duplicated->VerticesKeys[k][nVertices+n]=VerticesKeys[k][ed.a(n)];
		}
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

		if (Faces[n].tag2!=0)
		{
			if (Faces[n].tag2==1)
			{

				mpv0=Faces[n].mp0;
				mpv1=Faces[n].mp1;
				mpv2=Faces[n].mp2;

				if (u1.Norme()<len)
				{
					m0=nVertices+ed.Witch(v1,v2);
					m1=nVertices+ed.Witch(v2,v0);

					mpm0=(mpv1+mpv2)/2;
					mpm1=(mpv2+mpv0)/2;

					a=v1;
					b=v2;
					c=v0;
					mpa=mpv1;
					mpb=mpv2;
					mpc=mpv0;


				}

				if (u2.Norme()<len)
				{
					m0=nVertices+ed.Witch(v2,v0);
					m1=nVertices+ed.Witch(v0,v1);

					mpm0=(mpv2+mpv0)/2;
					mpm1=(mpv0+mpv1)/2;

					a=v2;
					b=v0;
					c=v1;

					mpa=mpv2;
					mpb=mpv0;
					mpc=mpv1;
				}

				if (u3.Norme()<len)
				{
					m0=nVertices+ed.Witch(v0,v1);
					m1=nVertices+ed.Witch(v1,v2);

					mpm0=(mpv0+mpv1)/2;
					mpm1=(mpv1+mpv2)/2;

					a=v0;
					b=v1;
					c=v2;
					mpa=mpv0;
					mpb=mpv1;
					mpc=mpv2;

				}

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=a;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=c;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpa;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpc;
				nn++;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=c;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpc;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=m1;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=b;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpm1;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpb;
				nn++;

			}
			else
			{
				m0=nVertices+ed.Witch(v0,v1);
				m1=nVertices+ed.Witch(v1,v2);
				m2=nVertices+ed.Witch(v2,v0);

				mpv0=Faces[n].mp0;
				mpv1=Faces[n].mp1;
				mpv2=Faces[n].mp2;

				mpm0=(mpv0+mpv1)/2;
				mpm1=(mpv1+mpv2)/2;
				mpm2=(mpv2+mpv0)/2;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v0;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m2;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;


				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv0;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm2;
				nn++;


				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=m2;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpm2;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;


				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v1;
				duplicated->Faces[nn].v1=m1;
				duplicated->Faces[nn].v2=m0;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv1;
				duplicated->Faces[nn].mp1=mpm1;
				duplicated->Faces[nn].mp2=mpm0;
				nn++;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v2;
				duplicated->Faces[nn].v1=m2;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv2;
				duplicated->Faces[nn].mp1=mpm2;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;
			}
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;

		}
	}

	duplicated->SetFaces();
	//for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	ed.Free();
    
	return duplicated;
}

CObject3D* CObject3D::TesselateLength2Map2(float len)
{
    CObject3D *duplicated;
    int n,k,nn,nnn;
    int v0,v1,v2;
    int m0,m1,m2,a,b,c;
    CVector u1,u2,u3;
    CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2,mpa,mpb,mpc;

    duplicated=new CObject3D;

    naming(duplicated->Name,Name);

    QuickEdges ed;
    ed.Init(nVertices);

    nn=0;
    nnn=0;
    for (n=0;n<nFaces;n++)
    {
        v0=Faces[n].v0;
        v1=Faces[n].v1;
        v2=Faces[n].v2;

        u1=Vertices[v1].Stok-Vertices[v0].Stok;
        u2=Vertices[v2].Stok-Vertices[v1].Stok;
        u3=Vertices[v0].Stok-Vertices[v2].Stok;

        if ((u1.Norme()>len)||(u2.Norme()>len)||(u3.Norme()>len))
        {
            
            if ((u1.Norme()<len)||(u2.Norme()<len)||(u3.Norme()<len))
            {
                nnn++;
                Faces[n].tag2=1;

                if (u1.Norme()<len)
                {
                    ed.Add(Faces[n].v1,Faces[n].v2);
                    ed.Add(Faces[n].v2,Faces[n].v0);
                }

                if (u2.Norme()<len)
                {
                    ed.Add(Faces[n].v0,Faces[n].v1);
                    ed.Add(Faces[n].v2,Faces[n].v0);
                }

                if (u3.Norme()<len)
                {
                    ed.Add(Faces[n].v1,Faces[n].v2);
                    ed.Add(Faces[n].v0,Faces[n].v1);
                }
            }
            else
            {
                nn++;
                Faces[n].tag2=2;
                ed.Add(Faces[n].v0,Faces[n].v1);
                ed.Add(Faces[n].v1,Faces[n].v2);
                ed.Add(Faces[n].v2,Faces[n].v0);
            }
        }
        else Faces[n].tag2=0;
    }
    
    ed.Fix();

    duplicated->Init(nVertices+ed.Nb(),nnn*3+nn*4+(nFaces-nn-nnn));

    if (nKeys>0)
    {
        duplicated->nKeys=nKeys;
        for (k=0;k<nKeys;k++)
        {
            duplicated->VerticesKeys[k]=new CShortVertex[nVertices+ed.Nb()];
        }
    }


    for (n=0;n<nVertices;n++)
    {
        duplicated->Vertices[n]=Vertices[n];

        if (nKeys>0)
        {
            duplicated->nKeys=nKeys;
            for (k=0;k<nKeys;k++)
                duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
        }
    }

    for (n=0;n<ed.Nb();n++)
    {
        duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
        duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
        duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
        duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
        duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
        //duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

        if (nKeys>0)
        {
            duplicated->nKeys=nKeys;
            for (k=0;k<nKeys;k++)
                duplicated->VerticesKeys[k][nVertices+n]=VerticesKeys[k][ed.a(n)];
        }
    }

    nn=0;
    for (n=0;n<nFaces;n++)
    {
        v0=Faces[n].v0;
        v1=Faces[n].v1;
        v2=Faces[n].v2;

        u1=Vertices[v1].Stok-Vertices[v0].Stok;
        u2=Vertices[v2].Stok-Vertices[v1].Stok;
        u3=Vertices[v0].Stok-Vertices[v2].Stok;

        if (Faces[n].tag2!=0)
        {

            if (Faces[n].tag2==1)
            {

                mpv0=Faces[n].mp0;
                mpv1=Faces[n].mp1;
                mpv2=Faces[n].mp2;

                if (u1.Norme()<len)
                {
                    m0=nVertices+ed.Witch(v1,v2);
                    m1=nVertices+ed.Witch(v2,v0);

                    mpm0=(mpv1+mpv2)/2;
                    mpm1=(mpv2+mpv0)/2;

                    a=v1;
                    b=v2;
                    c=v0;
                    mpa=mpv1;
                    mpb=mpv2;
                    mpc=mpv0;


                }

                if (u2.Norme()<len)
                {
                    m0=nVertices+ed.Witch(v2,v0);
                    m1=nVertices+ed.Witch(v0,v1);

                    mpm0=(mpv2+mpv0)/2;
                    mpm1=(mpv0+mpv1)/2;

                    a=v2;
                    b=v0;
                    c=v1;

                    mpa=mpv2;
                    mpb=mpv0;
                    mpc=mpv1;
                }

                if (u3.Norme()<len)
                {
                    m0=nVertices+ed.Witch(v0,v1);
                    m1=nVertices+ed.Witch(v1,v2);

                    mpm0=(mpv0+mpv1)/2;
                    mpm1=(mpv1+mpv2)/2;

                    a=v0;
                    b=v1;
                    c=v2;
                    mpa=mpv0;
                    mpb=mpv1;
                    mpc=mpv2;

                }

                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=a;
                duplicated->Faces[nn].v1=m0;
                duplicated->Faces[nn].v2=c;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;

                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpa;
                duplicated->Faces[nn].mp1=mpm0;
                duplicated->Faces[nn].mp2=mpc;
                nn++;


                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=c;
                duplicated->Faces[nn].v1=m0;
                duplicated->Faces[nn].v2=m1;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;

                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpc;
                duplicated->Faces[nn].mp1=mpm0;
                duplicated->Faces[nn].mp2=mpm1;
                nn++;

                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=m1;
                duplicated->Faces[nn].v1=m0;
                duplicated->Faces[nn].v2=b;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;

                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpm1;
                duplicated->Faces[nn].mp1=mpm0;
                duplicated->Faces[nn].mp2=mpb;
                nn++;

            }
            else
            {


                m0=nVertices+ed.Witch(v0,v1);
                m1=nVertices+ed.Witch(v1,v2);
                m2=nVertices+ed.Witch(v2,v0);


                mpv0=Faces[n].mp0;
                mpv1=Faces[n].mp1;
                mpv2=Faces[n].mp2;

                mpm0=(mpv0+mpv1)/2;
                mpm1=(mpv1+mpv2)/2;
                mpm2=(mpv2+mpv0)/2;

                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=v0;
                duplicated->Faces[nn].v1=m0;
                duplicated->Faces[nn].v2=m2;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;


                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpv0;
                duplicated->Faces[nn].mp1=mpm0;
                duplicated->Faces[nn].mp2=mpm2;
                nn++;


                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=m2;
                duplicated->Faces[nn].v1=m0;
                duplicated->Faces[nn].v2=m1;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;
                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpm2;
                duplicated->Faces[nn].mp1=mpm0;
                duplicated->Faces[nn].mp2=mpm1;
                nn++;


                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=v1;
                duplicated->Faces[nn].v1=m1;
                duplicated->Faces[nn].v2=m0;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;
                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpv1;
                duplicated->Faces[nn].mp1=mpm1;
                duplicated->Faces[nn].mp2=mpm0;
                nn++;

                duplicated->Faces[nn]=Faces[n];
                duplicated->Faces[nn].v0=v2;
                duplicated->Faces[nn].v1=m2;
                duplicated->Faces[nn].v2=m1;
                duplicated->Faces[nn].Norm=Faces[n].Norm;
                duplicated->Faces[nn].tag=Faces[n].tag;
                duplicated->Faces[nn].ref=Faces[n].ref;
                duplicated->Faces[nn].nT=Faces[n].nT;
                duplicated->Faces[nn].nT2=Faces[n].nT2;
                duplicated->Faces[nn].nL=Faces[n].nL;
                //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
                duplicated->Faces[nn].mp0=mpv2;
                duplicated->Faces[nn].mp1=mpm2;
                duplicated->Faces[nn].mp2=mpm1;
                nn++;
            }
        }
        else
        {
            v0=Faces[n].v0;
            v1=Faces[n].v1;
            v2=Faces[n].v2;

            mpv0=Faces[n].mp0;
            mpv1=Faces[n].mp1;
            mpv2=Faces[n].mp2;

            duplicated->Faces[nn]=Faces[n];
            duplicated->Faces[nn].v0=v0;
            duplicated->Faces[nn].v1=v1;
            duplicated->Faces[nn].v2=v2;
            duplicated->Faces[nn].Norm=Faces[n].Norm;
            duplicated->Faces[nn].tag=Faces[n].tag;
            duplicated->Faces[nn].ref=Faces[n].ref;
            duplicated->Faces[nn].nT=Faces[n].nT;
            duplicated->Faces[nn].nT2=Faces[n].nT2;
            duplicated->Faces[nn].nL=Faces[n].nL;
            //duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
            duplicated->Faces[nn].mp0=mpv0;
            duplicated->Faces[nn].mp1=mpv1;
            duplicated->Faces[nn].mp2=mpv2;
            nn++;

        }
    }

    duplicated->SetFaces();
    //for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
    for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

    duplicated->Rot=Rot;
    duplicated->Coo=Coo;

    for (n=0;n<6;n++)
        duplicated->P[n]=P[n];

    duplicated->nP=nP;


    for (n=0;n<16;n++)
        duplicated->Tab[n]=Tab[n];

    duplicated->Status=Status;
    duplicated->nurbs=nurbs;
    duplicated->Status2=Status2;
    duplicated->Attribut=Attribut;

    duplicated->Tag=Tag;
    duplicated->env_mapping=env_mapping;
    duplicated->group=group;

    ed.Free();
    return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to face length)

		CObject3D CObject3D::TesselateLengthSelected(int tag,float len)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
CObject3D CObject3D::TesselateLengthSelected(int tag,float len)
{
	return (*TesselateLengthSelected2(tag,len));
}

CObject3D* CObject3D::TesselateLengthSelected2(int tag,float len)
{
	CObject3D *duplicated;
	int n,k,nn,nnn;
	int v0,v1,v2;
	int m0,m1,m2,a,b,c;
	CVector u1,u2,u3;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2,mpa,mpb,mpc;
	QuickEdges ed;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	ed.Init(nVertices);

	nn=0;
	nnn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

		if (((u1.Norme()>len)||(u2.Norme()>len)||(u3.Norme()>len))&&(tag==Faces[n].tag))
		{
			
			if ((u1.Norme()<len)||(u2.Norme()<len)||(u3.Norme()<len))
			{
				nnn++;
				Faces[n].tag2=1;

				if (u1.Norme()<len)
				{
					ed.Add(Faces[n].v1,Faces[n].v2);
					ed.Add(Faces[n].v2,Faces[n].v0);
				}

				if (u2.Norme()<len)
				{
					ed.Add(Faces[n].v0,Faces[n].v1);
					ed.Add(Faces[n].v2,Faces[n].v0);
				}

				if (u3.Norme()<len)
				{
					ed.Add(Faces[n].v1,Faces[n].v2);
					ed.Add(Faces[n].v0,Faces[n].v1);
				}
			}
			else
			{
				nn++;
				Faces[n].tag2=2;
				ed.Add(Faces[n].v0,Faces[n].v1);
				ed.Add(Faces[n].v1,Faces[n].v2);
				ed.Add(Faces[n].v2,Faces[n].v0);
			}
		}
		else Faces[n].tag2=0;
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nnn*3+nn*4+(nFaces-nn));

	if (nKeys>0)
	{
		duplicated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			duplicated->VerticesKeys[k]=new CShortVertex[nVertices+ed.Nb()];
		}
	}


	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];

		if (nKeys>0)
		{
			duplicated->nKeys=nKeys;
			for (k=0;k<nKeys;k++)
				duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
		}
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[2]=0;
		duplicated->Vertices[nVertices+n].Index[3]=0;
		duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
		duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
		duplicated->Vertices[nVertices+n].Weight[2]=0;
		duplicated->Vertices[nVertices+n].Weight[3]=0;

		if (nKeys>0)
		{
			duplicated->nKeys=nKeys;
			for (k=0;k<nKeys;k++)
				duplicated->VerticesKeys[k][nVertices+n]=VerticesKeys[k][ed.a(n)];
		}
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

		if (Faces[n].tag2!=0)
		{

			if (Faces[n].tag2==1)
			{

				mpv0=Faces[n].mp0;
				mpv1=Faces[n].mp1;
				mpv2=Faces[n].mp2;

				if (u1.Norme()<len)
				{
					m0=nVertices+ed.Witch(v1,v2);
					m1=nVertices+ed.Witch(v2,v0);

					mpm0=(mpv1+mpv2)/2;
					mpm1=(mpv2+mpv0)/2;

					a=v1;
					b=v2;
					c=v0;
					mpa=mpv1;
					mpb=mpv2;
					mpc=mpv0;


				}

				if (u2.Norme()<len)
				{
					m0=nVertices+ed.Witch(v2,v0);
					m1=nVertices+ed.Witch(v0,v1);

					mpm0=(mpv2+mpv0)/2;
					mpm1=(mpv0+mpv1)/2;

					a=v2;
					b=v0;
					c=v1;

					mpa=mpv2;
					mpb=mpv0;
					mpc=mpv1;
				}

				if (u3.Norme()<len)
				{
					m0=nVertices+ed.Witch(v0,v1);
					m1=nVertices+ed.Witch(v1,v2);

					mpm0=(mpv0+mpv1)/2;
					mpm1=(mpv1+mpv2)/2;

					a=v0;
					b=v1;
					c=v2;
					mpa=mpv0;
					mpb=mpv1;
					mpc=mpv2;

				}

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=a;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=c;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpa;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpc;
				nn++;


				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=c;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpc;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=m1;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=b;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;

				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpm1;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpb;
				nn++;

			}
			else
			{


				m0=nVertices+ed.Witch(v0,v1);
				m1=nVertices+ed.Witch(v1,v2);
				m2=nVertices+ed.Witch(v2,v0);


				mpv0=Faces[n].mp0;
				mpv1=Faces[n].mp1;
				mpv2=Faces[n].mp2;

				mpm0=(mpv0+mpv1)/2;
				mpm1=(mpv1+mpv2)/2;
				mpm2=(mpv2+mpv0)/2;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v0;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m2;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;


				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv0;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm2;
				nn++;


				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=m2;
				duplicated->Faces[nn].v1=m0;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpm2;
				duplicated->Faces[nn].mp1=mpm0;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;


				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v1;
				duplicated->Faces[nn].v1=m1;
				duplicated->Faces[nn].v2=m0;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv1;
				duplicated->Faces[nn].mp1=mpm1;
				duplicated->Faces[nn].mp2=mpm0;
				nn++;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v2;
				duplicated->Faces[nn].v1=m2;
				duplicated->Faces[nn].v2=m1;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv2;
				duplicated->Faces[nn].mp1=mpm2;
				duplicated->Faces[nn].mp2=mpm1;
				nn++;
			}
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn]=Faces[n];
			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].nL=Faces[n].nL;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;

		}
	}

    duplicated->nFaces=nn;
    
	duplicated->SetFaces();

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status2=Status2;
	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	ed.Free();
	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to face length)

		CObject3D CObject3D::TesselateLength(float len)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::TesselateLengthNb(float len,int NB)
{
	return (*TesselateLength2(len));
}

CObject3D* CObject3D::TesselateLengthNb2(float len,int NB)
{
	CObject3D *duplicated;
	int n,nn,nnn;
	int v0,v1,v2;
	int m0,m1,m2,a,b,c;
	CVector u1,u2,u3;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2,mpa,mpb,mpc;
	QuickEdges ed;

	duplicated=new CObject3D;

	if (nFaces>NB)
	{
		naming(duplicated->Name,Name);
		duplicated->Init(nVertices,nFaces);

		for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];
		for (n=0;n<nFaces;n++) duplicated->Faces[n]=Faces[n];
	}
	else
	{
		naming(duplicated->Name,Name);

		ed.Init(nVertices);

		nn=0;
		nnn=0;
		for (n=0;n<nFaces;n++)
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			u1=Vertices[v1].Stok-Vertices[v0].Stok;
			u2=Vertices[v2].Stok-Vertices[v1].Stok;
			u3=Vertices[v0].Stok-Vertices[v2].Stok;

			if ((u1.Norme()>len)||(u2.Norme()>len)||(u3.Norme()>len))
			{
				/*
				if ((u1.Norme()<len)||(u2.Norme()<len)||(u3.Norme()<len))
				{
					nnn++;
					Faces[n].tag2=1;

					if (u1.Norme()<len)
					{
						ed.Add(Faces[n].v1,Faces[n].v2);
						ed.Add(Faces[n].v2,Faces[n].v0);
					}

					if (u2.Norme()<len)
					{
						ed.Add(Faces[n].v0,Faces[n].v1);
						ed.Add(Faces[n].v2,Faces[n].v0);
					}

					if (u3.Norme()<len)
					{
						ed.Add(Faces[n].v1,Faces[n].v2);
						ed.Add(Faces[n].v0,Faces[n].v1);
					}
				}
				else
				{
					/**/
					nn++;
					Faces[n].tag2=2;
					ed.Add(Faces[n].v0,Faces[n].v1);
					ed.Add(Faces[n].v1,Faces[n].v2);
					ed.Add(Faces[n].v2,Faces[n].v0);
				//}


			}
			else Faces[n].tag2=0;
		}

        ed.Fix();

		duplicated->Init(nVertices+ed.Nb(),nnn*3+nn*4+(nFaces-nn));

		for (n=0;n<nVertices;n++)
		{
			duplicated->Vertices[n]=Vertices[n];
		}

		for (n=0;n<ed.Nb();n++)
		{
			duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
			duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
			duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
			duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
			//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
			//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

			duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
			duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
			duplicated->Vertices[nVertices+n].Index[2]=0;
			duplicated->Vertices[nVertices+n].Index[3]=0;
			duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
			duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
			duplicated->Vertices[nVertices+n].Weight[2]=0;
			duplicated->Vertices[nVertices+n].Weight[3]=0;
		}


		nn=0;
		for (n=0;n<nFaces;n++)
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			u1=Vertices[v1].Stok-Vertices[v0].Stok;
			u2=Vertices[v2].Stok-Vertices[v1].Stok;
			u3=Vertices[v0].Stok-Vertices[v2].Stok;

			if (Faces[n].tag2!=0)
			{

				if (Faces[n].tag2==1)
				{

					mpv0=Faces[n].mp0;
					mpv1=Faces[n].mp1;
					mpv2=Faces[n].mp2;

					if (u1.Norme()<len)
					{
						m0=nVertices+ed.Witch(v1,v2);
						m1=nVertices+ed.Witch(v2,v0);

						mpm0=(mpv1+mpv2)/2;
						mpm1=(mpv2+mpv0)/2;

						a=v1;
						b=v2;
						c=v0;
						mpa=mpv1;
						mpb=mpv2;
						mpc=mpv0;


					}

					if (u2.Norme()<len)
					{
						m0=nVertices+ed.Witch(v2,v0);
						m1=nVertices+ed.Witch(v0,v1);

						mpm0=(mpv2+mpv0)/2;
						mpm1=(mpv0+mpv1)/2;

						a=v2;
						b=v0;
						c=v1;

						mpa=mpv2;
						mpb=mpv0;
						mpc=mpv1;
					}

					if (u3.Norme()<len)
					{
						m0=nVertices+ed.Witch(v0,v1);
						m1=nVertices+ed.Witch(v1,v2);

						mpm0=(mpv0+mpv1)/2;
						mpm1=(mpv1+mpv2)/2;

						a=v0;
						b=v1;
						c=v2;
						mpa=mpv0;
						mpb=mpv1;
						mpc=mpv2;

					}

					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=a;
					duplicated->Faces[nn].v1=m0;
					duplicated->Faces[nn].v2=c;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;

					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpa;
					duplicated->Faces[nn].mp1=mpm0;
					duplicated->Faces[nn].mp2=mpc;
					nn++;


					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=c;
					duplicated->Faces[nn].v1=m0;
					duplicated->Faces[nn].v2=m1;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;

					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpc;
					duplicated->Faces[nn].mp1=mpm0;
					duplicated->Faces[nn].mp2=mpm1;
					nn++;

					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=m1;
					duplicated->Faces[nn].v1=m0;
					duplicated->Faces[nn].v2=b;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;

					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpm1;
					duplicated->Faces[nn].mp1=mpm0;
					duplicated->Faces[nn].mp2=mpb;
					nn++;

				}
				else
				{


					m0=nVertices+ed.Witch(v0,v1);
					m1=nVertices+ed.Witch(v1,v2);
					m2=nVertices+ed.Witch(v2,v0);


					mpv0=Faces[n].mp0;
					mpv1=Faces[n].mp1;
					mpv2=Faces[n].mp2;

					mpm0=(mpv0+mpv1)/2;
					mpm1=(mpv1+mpv2)/2;
					mpm2=(mpv2+mpv0)/2;

					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=v0;
					duplicated->Faces[nn].v1=m0;
					duplicated->Faces[nn].v2=m2;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;


					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpv0;
					duplicated->Faces[nn].mp1=mpm0;
					duplicated->Faces[nn].mp2=mpm2;
					nn++;


					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=m2;
					duplicated->Faces[nn].v1=m0;
					duplicated->Faces[nn].v2=m1;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;
					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpm2;
					duplicated->Faces[nn].mp1=mpm0;
					duplicated->Faces[nn].mp2=mpm1;
					nn++;


					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=v1;
					duplicated->Faces[nn].v1=m1;
					duplicated->Faces[nn].v2=m0;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;
					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpv1;
					duplicated->Faces[nn].mp1=mpm1;
					duplicated->Faces[nn].mp2=mpm0;
					nn++;

					duplicated->Faces[nn]=Faces[n];
					duplicated->Faces[nn].v0=v2;
					duplicated->Faces[nn].v1=m2;
					duplicated->Faces[nn].v2=m1;
					duplicated->Faces[nn].Norm=Faces[n].Norm;
					duplicated->Faces[nn].tag=Faces[n].tag;
					duplicated->Faces[nn].ref=Faces[n].ref;
					duplicated->Faces[nn].nT=Faces[n].nT;
					duplicated->Faces[nn].nT2=Faces[n].nT2;
					duplicated->Faces[nn].nL=Faces[n].nL;
					//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
					duplicated->Faces[nn].mp0=mpv2;
					duplicated->Faces[nn].mp1=mpm2;
					duplicated->Faces[nn].mp2=mpm1;
					nn++;
				}
			}
			else
			{
				v0=Faces[n].v0;
				v1=Faces[n].v1;
				v2=Faces[n].v2;

				mpv0=Faces[n].mp0;
				mpv1=Faces[n].mp1;
				mpv2=Faces[n].mp2;

				duplicated->Faces[nn]=Faces[n];
				duplicated->Faces[nn].v0=v0;
				duplicated->Faces[nn].v1=v1;
				duplicated->Faces[nn].v2=v2;
				duplicated->Faces[nn].Norm=Faces[n].Norm;
				duplicated->Faces[nn].tag=Faces[n].tag;
				duplicated->Faces[nn].ref=Faces[n].ref;
				duplicated->Faces[nn].nT=Faces[n].nT;
				duplicated->Faces[nn].nT2=Faces[n].nT2;
				duplicated->Faces[nn].nL=Faces[n].nL;
				//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
				duplicated->Faces[nn].mp0=mpv0;
				duplicated->Faces[nn].mp1=mpv1;
				duplicated->Faces[nn].mp2=mpv2;
				nn++;

			}
		}
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	ed.Free();
	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in four (accordingly to face length)

		CObject3D CObject3D::TesselateLength4(float len)

		(variante)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::TesselateLength4(float len)
{
	return (*TesselateLength42(len));
}

CObject3D* CObject3D::TesselateLength42(float len)
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector u1,u2,u3;
    float llen=len*len;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2,mpa,mpb,mpc;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	QuickEdges ed;

    ed.Init(nVertices);

	nn=0;
    
	for (n=0;n<nFaces;n++)
	{
        Faces[n].tag=0;
        
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

        if ((VECTORNORM2(u1)>llen)||(VECTORNORM2(u2)>llen)||(VECTORNORM2(u3)>llen))
		{
            Faces[n].tag=1;
			nn++;
            
            ed.Add(v0,v1);
            ed.Add(v1,v2);
            ed.Add(v2,v0);
		}
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nn*4+(nFaces-nn));

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.b(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[2]=0;
		duplicated->Vertices[nVertices+n].Index[3]=0;
		duplicated->Vertices[nVertices+n].Weight[0]=0.8f;
		duplicated->Vertices[nVertices+n].Weight[1]=0.2f;
		duplicated->Vertices[nVertices+n].Weight[2]=0;
		duplicated->Vertices[nVertices+n].Weight[3]=0;
	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
        if (Faces[n].tag)
		{
            v0=Faces[n].v0;
            v1=Faces[n].v1;
            v2=Faces[n].v2;

			m0=nVertices+ed.Witch(v0,v1);
			m1=nVertices+ed.Witch(v1,v2);
			m2=nVertices+ed.Witch(v2,v0);

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			mpm0=(mpv0+mpv1)/2;
			mpm1=(mpv1+mpv2)/2;
			mpm2=(mpv2+mpv0)/2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm2;
			nn++;


			duplicated->Faces[nn].v0=m2;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpm2;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;


			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=m1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv1;
			duplicated->Faces[nn].mp1=mpm1;
			duplicated->Faces[nn].mp2=mpm0;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=m2;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv2;
			duplicated->Faces[nn].mp1=mpm2;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;
		}
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	ed.Free();
	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		each faces is subdivided in three (accordingly to face length)

		CObject3D CObject3D::TesselateLength3(float len)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::TesselateLength3(float len)
{
	return (*TesselateLength32(len));
}

CObject3D* CObject3D::TesselateLength32(float len)
{
	CObject3D* duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0;
	int cmp;
    float llen=len*len;
	CVector u1,u2,u3;
	CVector2 mpv0,mpv1,mpv2,mpm,mpa,mpb,mpc;

	duplicated=new CObject3D;

	naming(duplicated->Name,Name);

	nn=0;

	for (n=0;n<nFaces;n++)
	{
        Faces[n].tag=0;
        
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

        if ((VECTORNORM2(u1)>llen)||(VECTORNORM2(u2)>llen)||(VECTORNORM2(u3)>llen))
        {
            Faces[n].tag=1;
			nn++;
		}
	}


	duplicated->Init(nVertices+nn,nn*3+(nFaces-nn));

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	cmp=0;
	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		//u1=Vertices[v1].Stok-Vertices[v0].Stok;
		//u2=Vertices[v2].Stok-Vertices[v1].Stok;
		//u3=Vertices[v0].Stok-Vertices[v2].Stok;


		if (Faces[n].tag)
		{

			m0=nVertices+cmp;

			duplicated->Vertices[nVertices+cmp].Stok=(Vertices[v0].Stok+Vertices[v1].Stok+Vertices[v2].Stok)/3;
			duplicated->Vertices[nVertices+cmp].Norm=(Vertices[v0].Norm+Vertices[v1].Norm+Vertices[v2].Norm)/3;
			duplicated->Vertices[nVertices+cmp].Map=(Vertices[v0].Map+Vertices[v1].Map+Vertices[v2].Map)/3;
			//duplicated->Vertices[nVertices+cmp].Map2=(Vertices[v0].Map2+Vertices[v1].Map2+Vertices[v2].Map2)/3;

			duplicated->Vertices[nVertices+cmp].Index[0]=Vertices[v0].Index[0];
			duplicated->Vertices[nVertices+cmp].Index[1]=0;
			duplicated->Vertices[nVertices+cmp].Index[2]=0;
			duplicated->Vertices[nVertices+cmp].Index[3]=0;

			duplicated->Vertices[nVertices+cmp].Weight[0]=1;
			duplicated->Vertices[nVertices+cmp].Weight[1]=0;
			duplicated->Vertices[nVertices+cmp].Weight[2]=0;
			duplicated->Vertices[nVertices+cmp].Weight[3]=0;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			mpm=(mpv0+mpv1+mpv2)/3;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpm;
			nn++;

			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=v2;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv1;
			duplicated->Faces[nn].mp1=mpv2;
			duplicated->Faces[nn].mp2=mpm;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=v0;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv2;
			duplicated->Faces[nn].mp1=mpv0;
			duplicated->Faces[nn].mp2=mpm;
			nn++;

			cmp++;
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;

		}
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;


	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}


CObject3D* CObject3D::TesselateSelected3(int tag)
{
	CObject3D* duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0;
	int cmp;
	CVector u1,u2,u3;

	duplicated=new CObject3D;
	naming(duplicated->Name,Name);

	nn=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag==tag) nn++; }

	duplicated->Init(nVertices+nn,nn*3+(nFaces-nn));

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	cmp=0;
	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		u1=Vertices[v1].Stok-Vertices[v0].Stok;
		u2=Vertices[v2].Stok-Vertices[v1].Stok;
		u3=Vertices[v0].Stok-Vertices[v2].Stok;

		if (Faces[n].tag==tag)
		{
			m0=nVertices+cmp;

			duplicated->Vertices[nVertices+cmp].Stok=(Vertices[v0].Stok+Vertices[v1].Stok+Vertices[v2].Stok)/3;
			duplicated->Vertices[nVertices+cmp].Norm=(Vertices[v0].Norm+Vertices[v1].Norm+Vertices[v2].Norm)/3;
			duplicated->Vertices[nVertices+cmp].Map=(Vertices[v0].Map+Vertices[v1].Map+Vertices[v2].Map)/3;
			//duplicated->Vertices[nVertices+cmp].Map2=(Vertices[v0].Map2+Vertices[v1].Map2+Vertices[v2].Map2)/3;

			duplicated->Vertices[nVertices+cmp].Index[0]=Vertices[v0].Index[0];
			duplicated->Vertices[nVertices+cmp].Index[1]=0;
			duplicated->Vertices[nVertices+cmp].Index[2]=0;
			duplicated->Vertices[nVertices+cmp].Index[3]=0;

			duplicated->Vertices[nVertices+cmp].Weight[0]=1;
			duplicated->Vertices[nVertices+cmp].Weight[1]=0;
			duplicated->Vertices[nVertices+cmp].Weight[2]=0;
			duplicated->Vertices[nVertices+cmp].Weight[3]=0;


			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].mp0.x=-1.0f;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			nn++;

			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=v2;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].mp0.x=-1.0f;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=v0;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			duplicated->Faces[nn].mp0.x=-1.0f;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			nn++;

			cmp++;
		}
		else
		{
			v0=Faces[n].v0;
			v1=Faces[n].v1;
			v2=Faces[n].v2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0.x=Faces[n].mp0.x;
			nn++;

		}
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;

	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

CObject3D* CObject3D::Tesselate3()
{
	CObject3D* duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0;
	int cmp;
	CVector u1,u2,u3;
	CVector2 mpv0,mpv1,mpv2,mpm,mpa,mpb,mpc;

	duplicated=new CObject3D;
	naming(duplicated->Name,Name);

	duplicated->Init(nVertices+nFaces,nFaces*3);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n]=Vertices[n];
	}

	cmp=0;
	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		m0=nVertices+cmp;

		duplicated->Vertices[nVertices+cmp].Stok=(Vertices[v0].Stok+Vertices[v1].Stok+Vertices[v2].Stok)/3;
		duplicated->Vertices[nVertices+cmp].Norm=(Vertices[v0].Norm+Vertices[v1].Norm+Vertices[v2].Norm)/3;
		duplicated->Vertices[nVertices+cmp].Map=(Vertices[v0].Map+Vertices[v1].Map+Vertices[v2].Map)/3;
		//duplicated->Vertices[nVertices+cmp].Map2=(Vertices[v0].Map2+Vertices[v1].Map2+Vertices[v2].Map2)/3;

		duplicated->Vertices[nVertices+cmp].Index[0]=Vertices[v0].Index[0];
		duplicated->Vertices[nVertices+cmp].Index[1]=0;
		duplicated->Vertices[nVertices+cmp].Index[2]=0;
		duplicated->Vertices[nVertices+cmp].Index[3]=0;

		duplicated->Vertices[nVertices+cmp].Weight[0]=1;
		duplicated->Vertices[nVertices+cmp].Weight[1]=0;
		duplicated->Vertices[nVertices+cmp].Weight[2]=0;
		duplicated->Vertices[nVertices+cmp].Weight[3]=0;


		mpv0=Faces[n].mp0;
		mpv1=Faces[n].mp1;
		mpv2=Faces[n].mp2;

		mpm=(mpv0+mpv1+mpv2)/3;

		duplicated->Faces[nn].v0=v0;
		duplicated->Faces[nn].v1=v1;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nL=Faces[n].nL;
		duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
        duplicated->Faces[nn].ref=Faces[n].ref;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv0;
		duplicated->Faces[nn].mp1=mpv1;
		duplicated->Faces[nn].mp2=mpm;
		nn++;

		duplicated->Faces[nn].v0=v1;
		duplicated->Faces[nn].v1=v2;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nL=Faces[n].nL;
		duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv1;
		duplicated->Faces[nn].mp1=mpv2;
		duplicated->Faces[nn].mp2=mpm;
		nn++;

		duplicated->Faces[nn].v0=v2;
		duplicated->Faces[nn].v1=v0;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nL=Faces[n].nL;
		duplicated->Faces[nn].nLVectors=Faces[n].nLVectors;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;
        duplicated->Faces[nn].tag4=Faces[n].tag4;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv2;
		duplicated->Faces[nn].mp1=mpv0;
		duplicated->Faces[nn].mp2=mpm;
		nn++;

		cmp++;
	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	duplicated->Rot=Rot;
	duplicated->Coo=Coo;

	for (n=0;n<6;n++)
		duplicated->P[n]=P[n];

	duplicated->nP=nP;

	for (n=0;n<16;n++)
		duplicated->Tab[n]=Tab[n];

	duplicated->Status=Status;
	duplicated->nurbs=nurbs;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;

	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        do stuffs to easy calculation of lightmap


        CVector * CObject3D::CalculateLightmapCoordinates()

    Usage:

        returns an array of four vector components


          Array[n*4 + 0]=A;
        Array[n*4 + 1]=B;
        Array[n*4 + 2]=C;
        Array[n*4 + 3]=N;

        n=0..max(Faces[i].tag i=0..nFaces)

        A,B,C the 3 point to define a paralellogram accordingly to coplanar faces of the object
        N : normal


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

float CObject3D::MinimalDistance()
{
    float fmin=100000;
    CVector u;
    float ff;

    for (int n=0;n<nFaces;n++)
    {
        u=Faces[n].v[1]->Stok-Faces[n].v[0]->Stok;
        ff=VECTORNORM2(u);
        if (ff<fmin) fmin=ff;
        u=Faces[n].v[2]->Stok-Faces[n].v[1]->Stok;
        ff=VECTORNORM2(u);
        if (ff<fmin) fmin=ff;
        u=Faces[n].v[0]->Stok-Faces[n].v[2]->Stok;
        ff=VECTORNORM2(u);
        if (ff<fmin) fmin=ff;
    }
    
    return sqrtf(fmin);
}

float CObject3D::MaximalDistance()
{
    float fmax=0;
    CVector u;
    float ff;

    for (int n=0;n<nFaces;n++)
    {
        u=Faces[n].v[1]->Stok-Faces[n].v[0]->Stok;
        ff=VECTORNORM2(u);
        if (ff>fmax) fmax=ff;
        u=Faces[n].v[2]->Stok-Faces[n].v[1]->Stok;
        ff=VECTORNORM2(u);
        if (ff>fmax) fmax=ff;
        u=Faces[n].v[0]->Stok-Faces[n].v[2]->Stok;
        ff=VECTORNORM2(u);
        if (ff>fmax) fmax=ff;
    }
    
    return sqrtf(fmax);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		do stuffs to easy calculation of lightmap


		CVector * CObject3D::CalculateLightmapCoordinates()

	Usage:

		returns an array of four vector components


  		Array[n*4 + 0]=A;
		Array[n*4 + 1]=B;
		Array[n*4 + 2]=C;
		Array[n*4 + 3]=N;

		n=0..max(Faces[i].tag i=0..nFaces)

		A,B,C the 3 point to define a paralellogram accordingly to coplanar faces of the object
		N : normal


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CVector * CObject3D::CalculateLightmapCoordinates()
{
	return CalculateLightmapCoordinates(0);
}


CVector * CObject3D::CalculateLightmapCoordinates(int size)
{
	int n,nn;
	int res,tag,d,res2;
	CVector N,A,B,Na,O;
	CVector tmp,vect,u1,u2,u,m1,m2;
	CVector2 mm1,mm2;
	int a,b,c;
	EdgeListD E;
	int ne;
	int i;
	float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;
	CVector * Array;

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	if (size==0) Array=new CVector[4*(tag+1)];
	else Array=new CVector[6*(tag+1)];

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	res=0;
	tag=1;

	while (res==0)
	{

		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;

			N=Faces[d].Norm;

			nn=0;
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					nn++;
					if (VECTORNORM(Faces[n].Norm)>0.5f) N=Faces[n].Norm;
				}
			}

			E.Init(nn*3+3);
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					E.AddEdge(Faces[n].v0,Faces[n].v1);
					E.AddEdge(Faces[n].v1,Faces[n].v2);
					E.AddEdge(Faces[n].v2,Faces[n].v0);
				}
			}

			nn=0;
			res2=0;
			if (E.nList>=3)
			{

				float l_max=0.0f;
				float ll=0.0f;
				int sva=-1;
				int svb=-1;
				int svc=-1;
				CVector uu1,uu2;


				while ((res2==0)&&(nn<E.nList))
				{
					a=E.List[nn].a;
					b=E.List[nn].b;
					E.List[nn].tag=1;

					ne=E.Next(b);
					if (E.List[ne].a==b) c=E.List[ne].b;
					else c=E.List[ne].a;

					E.List[ne].tag=1;

					res2=1;

					VECTORSUB(vect,Vertices[b].Stok,Vertices[a].Stok);
					CROSSPRODUCT(Na,N,vect);

					VECTORNORMALISE(Na);
					aa=Na.x;
					bb=Na.y;
					cc=Na.z;
					INVDOTPRODUCT(dd,Vertices[a].Stok,Na);

					for (i=0;i<E.nList;i++)
					{
						if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
						{
							A=Vertices[E.List[i].a].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
						if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
						{
							A=Vertices[E.List[i].b].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
					}

					if (res2)
					{
						VECTORSUB(uu1,Vertices[b].Stok,Vertices[a].Stok);
						VECTORSUB(uu2,Vertices[c].Stok,Vertices[a].Stok);
						ll=VECTORNORM(uu1)+VECTORNORM(uu2);
						if (ll>l_max)
						{
							l_max=ll;
							sva=a;
							svb=b;
							svc=c;
						}
						res2=0;
					}

					E.List[ne].tag=0;
					E.List[nn].tag=0;

					nn++;
				}

				if (sva!=-1) res2=1;

				if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
				{

					a=sva; b=svb; c=svc;

					VECTORSUB(u1,Vertices[a].Stok,Vertices[b].Stok);
					VECTORSUB(u2,Vertices[c].Stok,Vertices[b].Stok);

					mm1.x=Vertices[a].Map.x-Vertices[b].Map.x;
					mm1.y=Vertices[a].Map.y-Vertices[b].Map.y;
					mm2.x=Vertices[c].Map.x-Vertices[b].Map.x;
					mm2.y=Vertices[c].Map.y-Vertices[b].Map.y;

					m1.Init(mm1.x,mm1.y,0);
					m2.Init(mm2.x,mm2.y,0);

					VECTORNORMALISE(u1);
					VECTORNORMALISE(u2);

					O=Vertices[b].Stok;

					CROSSPRODUCT(u2,N,u1);
					VECTORNORMALISE(u2);

					s1=0;
					s2=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						VECTORSUB(tmp,A,O);
						DOTPRODUCT(ss,tmp,u1);
						if (s1>ss) s1=ss;
						DOTPRODUCT(ss,tmp,u2);
						if (s2>ss) s2=ss;
						VECTORSUB(tmp,B,O);
						DOTPRODUCT(ss,tmp,u1);
						if (s1>ss) s1=ss;
						DOTPRODUCT(ss,tmp,u2);
						if (s2>ss) s2=ss;
					}

					O=O+s1*u1+s2*u2;

					u1max=0;
					u2max=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						VECTORSUB(tmp,A,O);
						DOTPRODUCT(s1,tmp,u1); if (s1<0) s1=-s1;
						DOTPRODUCT(s2,tmp,u2); if (s2<0) s2=-s2;
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
						VECTORSUB(tmp,B,O);
						DOTPRODUCT(s1,tmp,u1); if (s1<0) s1=-s1;
						DOTPRODUCT(s2,tmp,u2); if (s2<0) s2=-s2;
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
					}

					for (n=0;n<nVertices;n++) Vertices[n].tag=0;

					for (n=0;n<nFaces;n++)
					{
						if (Faces[n].tag==tag)
						{
							Faces[n].v[0]->tag=1;
							Faces[n].v[1]->tag=1;
							Faces[n].v[2]->tag=1;
						}
					}

					if (size==0)
					{
						Array[tag*4 + 0]=O;
						Array[tag*4 + 1]=O + u1*u1max;
						Array[tag*4 + 2]=O + u2*u2max;
						Array[tag*4 + 3]=N;
					}
					else
					{
						Array[tag*6 + 0]=O;
						Array[tag*6 + 1]=O + u1*u1max;
						Array[tag*6 + 2]=O + u2*u2max;
						Array[tag*6 + 3]=N;
						Array[tag*6 + 4]=m1;
						Array[tag*6 + 5]=m2;
					}

					if (size==0)
					{
						#define am ((float) 0.5f/SIZE_LIGHTMAPS)
						#define bm (((float) SIZE_LIGHTMAPS-0.5f)/SIZE_LIGHTMAPS)

						for (n=0;n<nVertices;n++)
						{
							if (Vertices[n].tag==1)
							{
								VECTORSUB(B,Vertices[n].Stok,O);
								DOTPRODUCT(s1,B,u1);s1=s1/u1max;
								DOTPRODUCT(s2,B,u2);s2=s2/u2max;
								Vertices[n].Map2.Init(am+s1*(bm-am),am+s2*(bm-am));
							}
						}
					}
					else
					{
						for (n=0;n<nVertices;n++)
						{
							if (Vertices[n].tag==1)
							{
								VECTORSUB(B,Vertices[n].Stok,O);
								DOTPRODUCT(s1,B,u1);s1=s1/u1max;
								DOTPRODUCT(s2,B,u2);s2=s2/u2max;
								Vertices[n].Map2.Init(s1,s2);
							}
						}

					}
				}
			}


			E.Free();

			tag++;

		}
	}
	return Array;
}

CVector * CObject3D::CalculateLightmapCoordinatesAvg(int size)
{
    int n,nn;
    int res,tag,d,res2;
    CVector N,A,B,Na,O;
    CVector tmp,vect,u1,u2,u,m1,m2;
    CVector2 mm1,mm2;
    int a,b,c;
    EdgeListD E;
    int ne;
    int i;
    float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;
    CVector * Array;

    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    res=0;
    tag=1;

    SetFaces();
    CalculateNormals(-1);
    
    SetF012();

    while (res==0)
    {
        n=0;
        d=-1;
        while ((n<nFaces)&&(d==-1))
        {
            if (Faces[n].tag==0) d=n;
            n++;
        }

        if (d==-1) res=1;
        else
        {
            TagCoplanarFacesCool2(d,d,tag);
            Faces[d].tag=tag;
            tag++;
        }
    }

    if (size==0) Array=new CVector[4*(tag+1)];
    else Array=new CVector[6*(tag+1)];

    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    res=0;
    tag=1;

    while (res==0)
    {

        n=0;
        d=-1;
        while ((n<nFaces)&&(d==-1))
        {
            if (Faces[n].tag==0) d=n;
            n++;
        }

        if (d==-1) res=1;
        else
        {
            TagCoplanarFacesCool2(d,d,tag);
            Faces[d].tag=tag;

            N.Init(0,0,0);

            nn=0;
            for (n=0;n<nFaces;n++)
            {
                if (Faces[n].tag==tag)
                {
                    nn++;
                    if (VECTORNORM(Faces[n].Norm)>0.5f) N+=Faces[n].Norm;
                }
            }
            
            N.Normalise();

            E.Init(nn*3+3);
            for (n=0;n<nFaces;n++)
            {
                if (Faces[n].tag==tag)
                {
                    E.AddEdge(Faces[n].v0,Faces[n].v1);
                    E.AddEdge(Faces[n].v1,Faces[n].v2);
                    E.AddEdge(Faces[n].v2,Faces[n].v0);
                }
            }

            nn=0;
            res2=0;
            if (E.nList>=3)
            {
                float l_max=0.0f;
                float ll=0.0f;
                int sva=-1;
                int svb=-1;
                int svc=-1;
                CVector uu1,uu2;

                while ((res2==0)&&(nn<E.nList))
                {
                    a=E.List[nn].a;
                    b=E.List[nn].b;
                    E.List[nn].tag=1;

                    ne=E.Next(b);
                    if (E.List[ne].a==b) c=E.List[ne].b;
                    else c=E.List[ne].a;

                    E.List[ne].tag=1;

                    res2=1;

                    VECTORSUB(vect,Vertices[b].Stok,Vertices[a].Stok);
                    CROSSPRODUCT(Na,N,vect);

                    VECTORNORMALISE(Na);
                    aa=Na.x;
                    bb=Na.y;
                    cc=Na.z;
                    INVDOTPRODUCT(dd,Vertices[a].Stok,Na);

                    for (i=0;i<E.nList;i++)
                    {
                        if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
                        {
                            A=Vertices[E.List[i].a].Stok;
                            s1=A.x*aa+A.y*bb+A.z*cc+dd;
                            if (s1>SMALLF) res2=0;
                        }
                        if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
                        {
                            A=Vertices[E.List[i].b].Stok;
                            s1=A.x*aa+A.y*bb+A.z*cc+dd;
                            if (s1>SMALLF) res2=0;
                        }
                    }

                    if (res2)
                    {
                        VECTORSUB(uu1,Vertices[b].Stok,Vertices[a].Stok);
                        VECTORSUB(uu2,Vertices[c].Stok,Vertices[a].Stok);
                        ll=VECTORNORM(uu1)+VECTORNORM(uu2);
                        if (ll>l_max)
                        {
                            l_max=ll;
                            sva=a;
                            svb=b;
                            svc=c;
                        }
                        res2=0;
                    }

                    E.List[ne].tag=0;
                    E.List[nn].tag=0;

                    nn++;
                }

                if (sva!=-1) res2=1;

                if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
                {

                    a=sva; b=svb; c=svc;

                    VECTORSUB(u1,Vertices[a].Stok,Vertices[b].Stok);
                    VECTORSUB(u2,Vertices[c].Stok,Vertices[b].Stok);
                    
                    u1.Base(N,0);
                    u2.Base(N,1);

                    mm1.x=Vertices[a].Map.x-Vertices[b].Map.x;
                    mm1.y=Vertices[a].Map.y-Vertices[b].Map.y;
                    mm2.x=Vertices[c].Map.x-Vertices[b].Map.x;
                    mm2.y=Vertices[c].Map.y-Vertices[b].Map.y;

                    m1.Init(mm1.x,mm1.y,0);
                    m2.Init(mm2.x,mm2.y,0);

                    VECTORNORMALISE(u1);
                    VECTORNORMALISE(u2);

                    O=Vertices[b].Stok;

                    CROSSPRODUCT(u2,N,u1);
                    VECTORNORMALISE(u2);

                    s1=0;
                    s2=0;
                    for (n=0;n<E.nList;n++)
                    {
                        A=Vertices[E.List[n].a].Stok;
                        B=Vertices[E.List[n].b].Stok;
                        VECTORSUB(tmp,A,O);
                        DOTPRODUCT(ss,tmp,u1);
                        if (s1>ss) s1=ss;
                        DOTPRODUCT(ss,tmp,u2);
                        if (s2>ss) s2=ss;
                        VECTORSUB(tmp,B,O);
                        DOTPRODUCT(ss,tmp,u1);
                        if (s1>ss) s1=ss;
                        DOTPRODUCT(ss,tmp,u2);
                        if (s2>ss) s2=ss;
                    }

                    O=O+s1*u1+s2*u2;

                    u1max=0;
                    u2max=0;
                    for (n=0;n<E.nList;n++)
                    {
                        A=Vertices[E.List[n].a].Stok;
                        B=Vertices[E.List[n].b].Stok;
                        VECTORSUB(tmp,A,O);
                        DOTPRODUCT(s1,tmp,u1); if (s1<0) s1=-s1;
                        DOTPRODUCT(s2,tmp,u2); if (s2<0) s2=-s2;
                        if (s1>u1max) u1max=s1;
                        if (s2>u2max) u2max=s2;
                        VECTORSUB(tmp,B,O);
                        DOTPRODUCT(s1,tmp,u1); if (s1<0) s1=-s1;
                        DOTPRODUCT(s2,tmp,u2); if (s2<0) s2=-s2;
                        if (s1>u1max) u1max=s1;
                        if (s2>u2max) u2max=s2;
                    }

                    for (n=0;n<nVertices;n++) Vertices[n].tag=0;

                    for (n=0;n<nFaces;n++)
                    {
                        if (Faces[n].tag==tag)
                        {
                            Faces[n].v[0]->tag=1;
                            Faces[n].v[1]->tag=1;
                            Faces[n].v[2]->tag=1;
                        }
                    }

                    if (size==0)
                    {
                        Array[tag*4 + 0]=O;
                        Array[tag*4 + 1]=O + u1*u1max;
                        Array[tag*4 + 2]=O + u2*u2max;
                        Array[tag*4 + 3]=N;
                    }
                    else
                    {
                        Array[tag*6 + 0]=O;
                        Array[tag*6 + 1]=O + u1*u1max;
                        Array[tag*6 + 2]=O + u2*u2max;
                        Array[tag*6 + 3]=N;
                        Array[tag*6 + 4]=m1;
                        Array[tag*6 + 5]=m2;
                    }

                    if (size==0)
                    {
                        #define am ((float) 0.5f/SIZE_LIGHTMAPS)
                        #define bm (((float) SIZE_LIGHTMAPS-0.5f)/SIZE_LIGHTMAPS)

                        for (n=0;n<nVertices;n++)
                        {
                            if (Vertices[n].tag==1)
                            {
                                VECTORSUB(B,Vertices[n].Stok,O);
                                DOTPRODUCT(s1,B,u1);s1=s1/u1max;
                                DOTPRODUCT(s2,B,u2);s2=s2/u2max;
                                Vertices[n].Map2.Init(am+s1*(bm-am),am+s2*(bm-am));
                            }
                        }
                    }
                    else
                    {
                        for (n=0;n<nVertices;n++)
                        {
                            if (Vertices[n].tag==1)
                            {
                                VECTORSUB(B,Vertices[n].Stok,O);
                                DOTPRODUCT(s1,B,u1);s1=s1/u1max;
                                DOTPRODUCT(s2,B,u2);s2=s2/u2max;
                                Vertices[n].Map2.Init(s1,s2);
                            }
                        }

                    }
                }
            }


            E.Free();

            tag++;

        }
    }
    return Array;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::SetRefTexture()
{
	int n;

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].ref=0;
		Faces[n].tag=0;
		Faces[n].tag2=0;
		Faces[n].nT=0;
		Faces[n].nT2=0;
		Faces[n].nL=0;
		Faces[n].nLVectors=0;
	}
}


void CObject3D::SetTexture(int nt)
{
	int n;
	for (n=0;n<nFaces;n++) Faces[n].nT=nt;
}

void CObject3D::SetTexture2(int nt)
{
	int n;
	for (n=0;n<nFaces;n++) Faces[n].nT2=nt;
}

void CObject3D::ApplyBlending(int num,CMatrix M1,CMatrix M2)
{
	int n;
	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok=Vertices[n].Weight[0]*(Vertices[n].Stok*M1)+
						 (1.0f-Vertices[n].Weight[0])*(Vertices[n].Stok*M2);
	}
}


void CObject3D::SetBlend(int num,char c)
{
	int n;
	CVector min,max;
	float minr,maxr;

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	minr=MAXI_BOUND;
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
	maxr=-MAXI_BOUND;

	for (n=0;n<nVertices;n++)
	{
		float rad=Vertices[n].Stok.Norme();

		if (rad>maxr) maxr=rad;
		if (rad<minr) minr=rad;

		if (Vertices[n].Stok.x>max.x) max.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y>max.y) max.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z>max.z) max.z=Vertices[n].Stok.z;

		if (Vertices[n].Stok.x<min.x) min.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y<min.y) min.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z<min.z) min.z=Vertices[n].Stok.z;
	}

	switch(c)
	{
	case 'X':
		for (n=0;n<nVertices;n++) Vertices[n].Weight[num]=(Vertices[n].Stok.x-min.x)/(max.x - min.x);
		break;
	case 'Y':
		for (n=0;n<nVertices;n++) Vertices[n].Weight[num]=(Vertices[n].Stok.y-min.y)/(max.y - min.y);
		break;
	case 'Z':
		for (n=0;n<nVertices;n++) Vertices[n].Weight[num]=(Vertices[n].Stok.z-min.z)/(max.z - min.z);
		break;
	case 'R':
		for (n=0;n<nVertices;n++) Vertices[n].Weight[num]=(Vertices[n].Stok.Norme()-minr)/(maxr - minr);
		break;
	}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cube

		void CObject3D::Cube(float Lx,float Ly,float Lz)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Cube(float Lx,float Ly,float Lz)
{
	int n;
	float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	float map[8][2]={ {0,0},{1,0},{1,1},{0,1},{0,0},{1,0},{1,1},{0,1} };

	Init(8,12);
	sprintf(Name,"OBJ");

	for (n=0;n<8;n++)
	{
		Vertices[n].Stok.Init(Lx*sss[n][0]/2,Ly*sss[n][1]/2,Lz*sss[n][2]/2);
		Vertices[n].Map.Init(map[n][0],map[n][1]);
	}

	Carre(0		,0,1,2,3);
	Carre(2		,3,2,6,7);
	Carre(4		,0,3,7,4);
	Carre(6		,1,0,4,5);
	Carre(8		,2,1,5,6);
	Carre(10    ,4,7,6,5);

	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cube

		void CObject3D::Cube(float Lx,float Ly,float Lz)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Pyramid(float Lx,float Ly,float Lz)
{
	int n;
	float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	float map[8][2]={ {0,0},{1,0},{1,1},{0,1},{0,0},{1,0},{1,1},{0,1} };

	Init(8,12);
	sprintf(Name,"OBJ");

	for (n=0;n<8;n++)
	{
		if (sss[n][2]>0) Vertices[n].Stok.Init(0,0,Lz*sss[n][2]/2);
		else Vertices[n].Stok.Init(Lx*sss[n][0]/2,Ly*sss[n][1]/2,Lz*sss[n][2]/2);
		Vertices[n].Map.Init(map[n][0],map[n][1]);
	}

	Carre(0		,0,1,2,3);
	Carre(2		,3,2,6,7);
	Carre(4		,0,3,7,4);
	Carre(6		,1,0,4,5);
	Carre(8		,2,1,5,6);
	Carre(10    ,4,7,6,5);

	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cube

		void CObject3D::Cube(float Lx,float Ly,float Lz)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::CubeWorld(float Ax,float Ay,float Az,float Bx,float By,float Bz)
{
	int n;
	float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	float map[8][2]={ {0,0},{1,0},{1,1},{0,1},{0,0},{1,0},{1,1},{0,1} };

	Init(8,12);
	sprintf(Name,"OBJ");

	for (n=0;n<8;n++)
	{
		if (sss[n][0]>0) Vertices[n].Stok.x=Bx;
		else Vertices[n].Stok.x=Ax;

		if (sss[n][1]>0) Vertices[n].Stok.y=By;
		else Vertices[n].Stok.y=Ay;

		if (sss[n][2]>0) Vertices[n].Stok.z=Bz;
		else Vertices[n].Stok.z=Az;

		Vertices[n].Map.Init(map[n][0],map[n][1]);
	}

	Carre(0		,0,1,2,3);
	Carre(2		,3,2,6,7);
	Carre(4		,0,3,7,4);
	Carre(6		,1,0,4,5);
	Carre(8		,2,1,5,6);
	Carre(10    ,4,7,6,5);

	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cube

		void CObject3D::Grid(float Lx,float Lz,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Grid(float Lx,float Lz,int def)
{
	int n;
	int x,y;
	int i0,i1,i2,i3;

	Init((def+1)*(def+1),def*def*2);
	sprintf(Name,"OBJ");

	n=0;
	for (y=0;y<=def;y++)
		for (x=0;x<=def;x++)
		{
			Vertices[n].Stok.Init(-Lx/2 + x*Lx/def,0,-Lz/2 + y*Lz/def);
			Vertices[n].Map.Init((float) x/def,(float) y/def);
			n++;
		}

	n=0;
	for (y=0;y<def;y++)
		for (x=0;x<def;x++)
		{
			i0=y*(def+1)+x;
			i1=y*(def+1)+x+1;
			i2=(y+1)*(def+1)+x+1;
			i3=(y+1)*(def+1)+x;
			Carre(n,i0,i1,i2,i3);
			n+=2;
		}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen disk

		void CObject3D::Disk(float R,int Rdef,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Disk(float R,int Rdef,int def)
{
	int n;
	int x,y;
	int i0,i1,i2,i3;

	Init((def+1)*(Rdef+1),Rdef*def*2);
	sprintf(Name,"OBJ");

	n=0;
	for (y=0;y<=def;y++)
	{
		float r=R*((float)y)/def;
		for (x=0;x<=Rdef;x++)
		{
			float q=2*PI*((float)x)/Rdef;			
			Vertices[n].Stok.Init(r*cosf(q),0,r*sinf(q));
			Vertices[n].Map.Init((float) x/def,(float) y/def);
			n++;
		}
	}

	n=0;
	for (y=0;y<def;y++)
		for (x=0;x<Rdef;x++)
		{
			i0=y*(Rdef+1)+x;
			i1=y*(Rdef+1)+x+1;
			i2=(y+1)*(Rdef+1)+x+1;
			i3=(y+1)*(Rdef+1)+x;
			Carre(n,i0,i1,i2,i3);
			n+=2;
		}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen dome

		void CObject3D::Dome(float R,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Dome(float R,int def)
{
	int n,nn,n0,n1,n2,n3;
	float h,r,x,y;

	Init(1+((def/2))*(def+1),((def/2)-1)*def*2+def);
	sprintf(Name,"OBJ");

	for (n=0;n<(def/2);n++)
	{
		r=(float) (R*cosf((PI/2)*(n)/(def/2)));
		h=(float) (-R*sinf((PI/2)*(n)/(def/2)));

		for (nn=0;nn<def+1;nn++)
		{
			x=(float) (r*cosf(2*PI*(nn+0.5f*(n&1))/def));
			y=(float) (r*sinf(2*PI*(nn+0.5f*(n&1))/def));
			Vertices[1+n*(def+1)+nn].Stok.Init(x,h,y);

			if (nn<def/2)
			{
				x=(float) nn/(def/2);
			}
			else
			{
				x=1-(float) (nn-def/2)/(def/2);
			}

			x=(float) nn/(def);


			Vertices[1+n*(def+1)+nn].Map.Init(h/(2*R) + 0.5f,x);

			x=(h/(2*R)+0.5f);
			x=x*x*x;
			Vertices[1+n*(def+1)+nn].Diffuse.Init(x,x,x);
		}
	}
	Vertices[0].Stok.Init(0,-R,0);
	Vertices[0].Map.Init(-2+0.5f,0);
	Vertices[0].Diffuse.Init(0,0,0);

	for (n=0;n<(def/2)-1;n++)
	{
		for (nn=0;nn<def;nn++)
		{
			n0=nn+1;
			n1=nn;

			n2=n1+def+1;
			n3=n0+def+1;

			if ((n&1)==0)
			{
				Faces[n*def*2 + nn*2+0].Init(1+n*(def+1)+n0,1+n*(def+1)+n2,1+n*(def+1)+n1);
				Faces[n*def*2 + nn*2+1].Init(1+n*(def+1)+n0,1+n*(def+1)+n3,1+n*(def+1)+n2);
			}
			else
			{
				Faces[n*def*2 + nn*2+0].Init(1+n*(def+1)+n1,1+n*(def+1)+n0,1+n*(def+1)+n3);
				Faces[n*def*2 + nn*2+1].Init(1+n*(def+1)+n1,1+n*(def+1)+n3,1+n*(def+1)+n2);
			}

			Faces[n*def*2 + nn*2+0].tag=0;
			Faces[n*def*2 + nn*2+1].tag=0;
		}
	}

	for (nn=0;nn<def;nn++)
	{
		n0=nn;
		n1=nn+1;

		n2=n1+((def/2)-1)*(def+1);
		n3=n0+((def/2)-1)*(def+1);

		Faces[((def/2)-1)*def*2 + nn].Init(0,1+n3,1+n2);
		Faces[((def/2)-1)*def*2 + nn].tag=0;
	}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen conics

		void CObject3D::Cone(float L,float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Cone(float L,float R,int def)
{
	int nf=0;
	int n;
	float x,y;

	Init((def*2)+2,2*def);
	sprintf(Name,"OBJ");

	for (n=0;n<def;n++)
	{
		x=(float) (cosf((2*PI*n)/def));
		y=(float) (sinf((2*PI*n)/def));
		Vertices[2 + n].Stok.Init(R*x,L/2,R*y);
		Vertices[2 + def + n].Stok.Init(R*x,L/2,R*y);
	}

	Vertices[0].Stok.Init(0,-L/2,0);
	Vertices[1].Stok.Init(0,L/2,0);

	for (n=0;n<def;n++)
	{
		Triangle(nf, 0,2+(n+1)%def,2+n);
		nf++;
	}

	for (n=0;n<def;n++)
	{
		Triangle(nf,1,2+def+n,2+def+(n+1)%def);
		nf++;
	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].tag=0;
		Faces[n].tag2=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetTag(SET_FACES_TAG+0);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cylindre

		void CObject3D::Cylindre(float L,float R,int def)
		void CObject3D::Cylindre2(float L,float R,int def)
		void CObject3D::Cylindre2pair(float L,float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Cylindre2(float L,float R,int def)
{
	int nf;
	int n;
	float x,y;

	Init(def*2+def*4,(def-2)*2 + def*2);
	sprintf(Name,"OBJ");

	for (n=0;n<def;n++)
	{
		// vertex des 2 disques

		x=(float) (cosf((2*PI*n)/def));
		y=(float) (sinf((2*PI*n)/def));
		Vertices[def*4 + n].Stok.Init(R*x,L/2,R*y);
		Vertices[def*4 + def+ n].Stok.Init(R*x,-L/2,R*y);

		Vertices[def*4 + n].Map.Init(0.5f+x/2,0.5f+y/2);
		Vertices[def*4 + def+ n].Map.Init(0.5f+x/2,0.5f+y/2);

		// vertex des faces verticales

		x=(float) (R*cosf((2*PI*n)/def));
		y=(float) (R*sinf((2*PI*n)/def));

		Vertices[n*4+0].Stok.Init(x,L/2,y);
		Vertices[n*4+3].Stok.Init(x,-L/2,y);

		x=(float) (R*cosf((2*PI*(n+1))/def));
		y=(float) (R*sinf((2*PI*(n+1))/def));
		Vertices[n*4+1].Stok.Init(x,L/2,y);
		Vertices[n*4+2].Stok.Init(x,-L/2,y);

	}

	nf=def*2;
	for (n=1;n<def-1;n++)
	{
		Triangle(nf, def*4 + 0,def*4+n+1,def*4+n);
		nf++;
	}
	for (n=1;n<def-1;n++)
	{
		Triangle(nf, def*4 +def+ 0,def+def*4+n,def+def*4+n+1);
		nf++;
	}

	for (n=0;n<def;n++)
	{
		Carre2(n*2,n*4+0,n*4+1,n*4+2,n*4+3);

	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetTag(SET_FACES_TAG+0);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

void CObject3D::Cylindre2pair(float L,float R,int def)
{
	int nf;
	int n;
	float x,y;
	int NB=(def/2)/2;
	/*
	int aA1,aB1;
	int aA2,aB2;
	int bA1,bB1;
	int bA2,bB2;
	/**/

	nf=def*2;
	/*
	if (((def/2)&1)==0) nf+=4;
	else NB++;
	aA1=bA1=0; aB1=bB1=def/2;
	bA2=def-1; bB2=aB1+1;
	aA2=aA1+1; aB2=aB1-1;
	for (n=0;n<NB-1;n++)
	{
		nf+=8;
		aA1=(def+aA1+1)%def; aA2=(def+aA2+1)%def;
		aB1=(def+aB1-1)%def; aB2=(def+aB2-1)%def;
		bA1=(def+bA1-1)%def; bA2=(def+bA2-1)%def;
		bB1=(def+bB1+1)%def; bB2=(def+bB2+1)%def;
	}
	/**/

	Init(2+def*2+def*4,nf*2);
	sprintf(Name,"OBJ");

	Vertices[0].Stok.Init(0,L/2,0);
	Vertices[1].Stok.Init(0,-L/2,0);

	for (n=0;n<def;n++)
	{
		// vertex des 2 disques
		x=(float) (cosf((2*PI*n)/def));
		y=(float) (sinf((2*PI*n)/def));
		Vertices[2+def*4 + n].Stok.Init(R*x,L/2,R*y);
		Vertices[2+def*4 + def+ n].Stok.Init(R*x,-L/2,R*y);

		Vertices[2+def*4 + n].Map.Init(0.5f+x/2,0.5f+y/2);
		Vertices[2+def*4 + def+ n].Map.Init(0.5f+x/2,0.5f+y/2);

		// vertex des faces verticales

		x=(float) (R*cosf((2*PI*n)/def));
		y=(float) (R*sinf((2*PI*n)/def));

		Vertices[2+n*4+0].Stok.Init(x,L/2,y);
		Vertices[2+n*4+3].Stok.Init(x,-L/2,y);

		x=(float) (R*cosf((2*PI*(n+1))/def));
		y=(float) (R*sinf((2*PI*(n+1))/def));
		Vertices[2+n*4+1].Stok.Init(x,L/2,y);
		Vertices[2+n*4+2].Stok.Init(x,-L/2,y);

	}

	nf=def*2;
	for (n=0;n<def;n++)
	{
		Triangle(nf,0,2+def*4 + (n+1)%def,2+def*4 + n); nf++;
		Triangle(nf,1,2+def*4 + def+ n,2+def*4 + def+ (n+1)%def); nf++;
	}

/*
	aA1=bA1=0; aB1=bB1=def/2;
	bA2=def-1; bB2=aB1+1;
	aA2=aA1+1; aB2=aB1-1;

	for (n=0;n<NB-1;n++)
	{
		Triangle(nf, def*4+aA1,def*4+aB1,def*4+aB2); nf++;
		Triangle(nf, def*4+aA1,def*4+aB2,def*4+aA2); nf++;
		Triangle(nf, def*4+bA1,def*4+bB2,def*4+bB1); nf++;
		Triangle(nf, def*4+bA1,def*4+bA2,def*4+bB2); nf++;
		Triangle(nf, def*5+aA1,def*5+aB2,def*5+aB1); nf++;
		Triangle(nf, def*5+aA1,def*5+aA2,def*5+aB2); nf++;
		Triangle(nf, def*5+bA1,def*5+bB1,def*5+bB2); nf++;
		Triangle(nf, def*5+bA1,def*5+bB2,def*5+bA2); nf++;
		aA1=(def+aA1+1)%def; aA2=(def+aA2+1)%def;
		aB1=(def+aB1-1)%def; aB2=(def+aB2-1)%def;
		bA1=(def+bA1-1)%def; bA2=(def+bA2-1)%def;
		bB1=(def+bB1+1)%def; bB2=(def+bB2+1)%def;
	}

	if (((def/2)&1)==0)
	{
		Triangle(nf, def*4+aA1,def*4+aB1,def*4+aA2); nf++;
		Triangle(nf, def*4+bA1,def*4+bA2,def*4+bB1); nf++;
		Triangle(nf, def*5+aA1,def*5+aA2,def*5+aB1); nf++;
		Triangle(nf, def*5+bA1,def*5+bB1,def*5+bA2); nf++;
	}
/**/
	for (n=0;n<def;n++) Carre2(n*2,2+n*4+0,2+n*4+1,2+n*4+2,2+n*4+3);
	for (n=0;n<nFaces;n++) { Faces[n].nL=0; Faces[n].nT=0; }
	SetTag(SET_FACES_TAG+0);
	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
    
    InvertYMap();
    InvertXMap();
}




void CObject3D::Cylindre(float L,float R,int def)
{
	int Def;
	int n;
	float x,y;

	Init(2+4*(def+1),def*2+def*2);
	sprintf(Name,"OBJ");

	Def=def+1;

	Vertices[0].Stok.Init(0,0,L/2);
	Vertices[0].Map.Init(0.5f,0.5f);
	Vertices[1].Stok.Init(0,0,-L/2);
	Vertices[1].Map.Init(0.5f,0.5f);

	for (n=0;n<=def;n++)
	{
		x=(float) (R*cosf((2*PI*n)/def));
		y=(float) (R*sinf((2*PI*n)/def));
		Vertices[2+Def*0+n].Stok.Init(x,y,L/2);
		Vertices[2+Def*1+n].Stok.Init(x,y,L/2);

		Vertices[2+Def*2+n].Stok.Init(x,y,-L/2);
		Vertices[2+Def*3+n].Stok.Init(x,y,-L/2);


		x=(float) cosf((2*PI*n)/def)/2 + 0.5f;
		y=(float) sinf((2*PI*n)/def)/2 + 0.5f;

		Vertices[2+Def*0+n].Map.Init(x,y);
		Vertices[2+Def*3+n].Map.Init(x,y);

		x=(float) n/def;

		Vertices[2+Def*1+n].Map.Init(x,0);
		Vertices[2+Def*2+n].Map.Init(x,1);

	}

	for (n=0;n<def;n++)
	{
		Carre(def*2+n*2,2+Def+n,2+Def+(n+1),2+Def*2+(n+1),2+Def*2+n);



		Triangle(def*0+n, 0,2+(n+1)%def,2+n);
		Triangle(def*1+n, 1,2+Def*3+n,2+Def*3+(n+1)%def);
	}

	for (n=0;n<def;n++)
	{
		Faces[n].tag=0;
		Faces[n+def*1].tag=1;
		Faces[0+2*n+def*2].tag=2;
		Faces[1+2*n+def*2].tag=2;
	}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

void CObject3D::Cylindre3(float L,float R1,float R2,int def)
{
	int Def;
	int n;
	float x,y;

	Init(2*(def+1),def*2);
	sprintf(Name,"OBJ");

	Def=def+1;

	for (n=0;n<def;n++)
	{
		float c=cosf((2*PI*n)/def);
		float s=sinf((2*PI*n)/def);

		x=(float) (R1*c);
		y=(float) (R1*s);
		Vertices[Def*0+n].Stok.Init(x,y,L/2);

		x=(float) (R2*c);
		y=(float) (R2*s);
		Vertices[Def*1+n].Stok.Init(x,y,-L/2);


		x=(float) n/def;
		Vertices[Def*0+n].Map.Init(x,0);
		Vertices[Def*1+n].Map.Init(x,1);
	}

	Vertices[Def*0+def].Stok=Vertices[Def*0+0].Stok;
	Vertices[Def*1+def].Stok=Vertices[Def*1+0].Stok;
	Vertices[Def*0+def].Map=Vertices[Def*0+0].Map;
	Vertices[Def*1+def].Map=Vertices[Def*1+0].Map;

	for (n=0;n<def;n++)
	{
		Carre((n<<1),n,(n+1),Def+(n+1),Def+n);
	}

	for (n=0;n<def*2;n++) Faces[n].tag=0;


	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen arrow

		void CObject3D::Arrow(float L,float R,int def)
		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Arrow(float L,float R,int def)
{
	int nf;
	int n,nn;
	float x,y;
	float scale[4]={ 0,0,0.8f,0.8f };
	float radius[4]={ 1.0f,1.0f,1.0f,2.0f };
	float color[4]={ 1.0f,1.0f,1.0f,0.75f };

	Init(def*4 + 1,(def-2) + def + def*2*3);
	sprintf(Name,"OBJ");

	for (nn=0;nn<4;nn++)
	{
		for (n=0;n<def;n++)
		{
			x=(float) (cosf((2*PI*n)/def));
			y=(float) (sinf((2*PI*n)/def));
			Vertices[1 + n + nn*def].Stok.Init(radius[nn]*R*x,radius[nn]*R*y,L*scale[nn]);
			Vertices[1 + n + nn*def].Map.Init(0.5f+x/2,0.5f+y/2);
			Vertices[1 + n + nn*def].Diffuse.Init(color[nn],color[nn],color[nn]);
		}
	}

	Vertices[0].Stok.Init(0,0,L);
	Vertices[0].Diffuse.Init(1,1,1);

	nf=0;

	for (n=0;n<def;n++)
	{
		Triangle(nf,1 + 3*def + n,0,1 + 3*def + (n+1)%def);
		nf++;
	}

	for (nn=0;nn<3;nn++)
	{
		for (n=0;n<def;n++)
		{
			Carre2(nf,nn*def + 1 + n,(nn+1)*def + 1 + n,(nn+1)*def + 1 + (n+1)%def,nn*def + 1 + (n+1)%def);
			nf+=2;
		}
	}

	for (n=1;n<def-1;n++)
	{
		Triangle(nf,1,1+n,1+(n+1)%def);
		nf++;
	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0; Faces[n].nT=0; Faces[n].ref=0; Faces[n].tag=0;
	}

	SetFaces();
	CalculateNormals(-1);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cordechasse

		void CObject3D::CorDeChasse(float L,float R1,float R2,int def)
		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::CorDeChasse(float L,float R1,float R2,int def)
{
	int Def;
	int n;
	float x,y;
	float K1=0.6f;
	float K2=0.8f;
	float kf;

	Init(4*(def+1),def*6);
	sprintf(Name,"OBJ");

	Def=def+1;

	for (n=0;n<def;n++)
	{
		float c=cosf((2*PI*n)/def);
		float s=sinf((2*PI*n)/def);

		x=(float) (R1*c);
		y=(float) (R1*s);
		Vertices[n].Stok.Init(x,y,(L/2));

		x=(float) ((0.7f*R1+0.3f*R2)*c);
		y=(float) ((0.7f*R1+0.3f*R2)*s);
		Vertices[Def+n].Stok.Init(x,y,(L/2)-K1*L);

		x=(float) (((R1+R2)/2)*c);
		y=(float) (((R1+R2)/2)*s);
		Vertices[(Def<<1)+n].Stok.Init(x,y,(L/2)-K2*L);

		if (def==3) kf=((((float) (rand()%15))/15.0f) + 2.0f)/2.0f;
		else kf=1.5f;

		x=(float) (kf*R2*c);
		y=(float) (kf*R2*s);
		Vertices[Def+(Def<<1)+n].Stok.Init(x,y,-L/2);

		x=(float) n/def;
		Vertices[n].Map.Init(x,0.0f);
		Vertices[Def+n].Map.Init(x,K1);
		Vertices[(Def<<1)+n].Map.Init(x,K2);
		Vertices[Def+(Def<<1)+n].Map.Init(x,1.0f);
	}

	Vertices[Def*0+def].Stok=Vertices[Def*0+0].Stok;
	Vertices[Def*1+def].Stok=Vertices[Def*1+0].Stok;
	Vertices[Def*2+def].Stok=Vertices[Def*2+0].Stok;
	Vertices[Def*3+def].Stok=Vertices[Def*3+0].Stok;

	Vertices[Def*0+def].Map=Vertices[Def*0+0].Map;
	Vertices[Def*1+def].Map=Vertices[Def*1+0].Map;
	Vertices[Def*2+def].Map=Vertices[Def*2+0].Map;
	Vertices[Def*3+def].Map=Vertices[Def*3+0].Map;

	for (n=0;n<def;n++)
	{
		Carre((n<<1),n,(n+1),Def+(n+1),Def+n);
		Carre((def<<1)+(n<<1),Def+n,Def+(n+1),(Def<<1)+(n+1),2*Def+n);
		Carre((def<<2)+(n<<1),(Def<<1)+n,(Def<<1)+(n+1),(Def<<1)+Def+(n+1),(Def<<1)+Def+n);
	}

	for (n=0;n<def*6;n++) Faces[n].tag=0;

	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen tore

		void CObject3D::Tore(float R1,float R2,int def)
		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Tore(float R1,float R2,int def)
{
	int n,nn;
	float x,y;
	float X,Y;
	int nv=0;
	int nf=0;
	int def2=(3*def)/2;

	Init(def2*def,def2*def*2);
	sprintf(Name,"OBJ");

	for (n=0;n<def2;n++)
	{
		X=cosf((2*PI*n)/def2);
		Y=sinf((2*PI*n)/def2);

		for (nn=0;nn<def;nn++)
		{
			x=R2*cosf((2*PI*nn)/def);
			y=R2*sinf((2*PI*nn)/def);
			Vertices[nv].Stok.Init(X*(R1+x),y,Y*(R1+x));
			Vertices[nv].Map.Init((float)n/def2,(float)nn/def);
			Vertices[nv].Diffuse.Init(1,1,1,1);
			nv++;
		}
	}

	for (n=0;n<def2;n++)
	{
		int b0=n;
		int b1=(n+1)%def2;
		b0*=def;
		b1*=def;
		for (nn=0;nn<def;nn++)
		{
			Carre(nf,b0+nn,b0+((nn+1)%def),b1+((nn+1)%def),b1+nn);
			nf+=2;
		}
	}

	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen sphere

		void CObject3D::Sphere(float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
void CObject3D::Cylindre3void(float L,float R1,float R2,int def)
{
	int Def;
	int n;
	float x,y;

	Init(2*(def+1),def*2);
	sprintf(Name,"OBJ");

	Def=def+1;

	for (n=0;n<def;n++)
	{
		float c=cosf((2*PI*n)/def);
		float s=sinf((2*PI*n)/def);

		x=(float) (R1*c);
		y=(float) (R1*s);
		Vertices[Def*0+n].Stok.Init(x,y,L/2);

		x=(float) (R2*c);
		y=(float) (R2*s);
		Vertices[Def*1+n].Stok.Init(x,y,-L/2);


		x=(float) n/def;
		Vertices[Def*0+n].Map.Init(x,0);
		Vertices[Def*1+n].Map.Init(x,1);
	}

	Vertices[Def*0+def].Stok=Vertices[Def*0+0].Stok;
	Vertices[Def*1+def].Stok=Vertices[Def*1+0].Stok;
	Vertices[Def*0+def].Map=Vertices[Def*0+0].Map;
	Vertices[Def*1+def].Map=Vertices[Def*1+0].Map;

	for (n=0;n<def;n++)
	{
		Carre((n<<1),n,(n+1),Def+(n+1),Def+n);
	}

	for (n=0;n<def*2;n++) Faces[n].tag=0;
}

void CObject3D::CorDeChassevoid(float L,float R1,float R2,int def)
{
	int Def;
	int n;
	float x,y;
	float K1=0.6f;
	float K2=0.8f;
	float kf;

	Init(4*(def+1),def*6);
	sprintf(Name,"OBJ");

	Def=def+1;

	for (n=0;n<def;n++)
	{
		float c=cosf((2*PI*n)/def);
		float s=sinf((2*PI*n)/def);

		x=(float) (R1*c);
		y=(float) (R1*s);
		Vertices[n].Stok.Init(x,y,(L/2));

		x=(float) ((0.7f*R1+0.3f*R2)*c);
		y=(float) ((0.7f*R1+0.3f*R2)*s);
		Vertices[Def+n].Stok.Init(x,y,(L/2)-K1*L);

		x=(float) (((R1+R2)/2)*c);
		y=(float) (((R1+R2)/2)*s);
		Vertices[(Def<<1)+n].Stok.Init(x,y,(L/2)-K2*L);

		if (def==3) kf=((((float) (rand()%15))/15.0f) + 2.0f)/2.0f;
		else kf=1.5f;

		x=(float) (kf*R2*c);
		y=(float) (kf*R2*s);
		Vertices[Def+(Def<<1)+n].Stok.Init(x,y,-L/2);

		x=(float) n/def;
		Vertices[n].Map.Init(x,0.0f);
		Vertices[Def+n].Map.Init(x,K1);
		Vertices[(Def<<1)+n].Map.Init(x,K2);
		Vertices[Def+(Def<<1)+n].Map.Init(x,1.0f);
	}

	Vertices[Def*0+def].Stok=Vertices[Def*0+0].Stok;
	Vertices[Def*1+def].Stok=Vertices[Def*1+0].Stok;
	Vertices[Def*2+def].Stok=Vertices[Def*2+0].Stok;
	Vertices[Def*3+def].Stok=Vertices[Def*3+0].Stok;

	Vertices[Def*0+def].Map=Vertices[Def*0+0].Map;
	Vertices[Def*1+def].Map=Vertices[Def*1+0].Map;
	Vertices[Def*2+def].Map=Vertices[Def*2+0].Map;
	Vertices[Def*3+def].Map=Vertices[Def*3+0].Map;

	for (n=0;n<def;n++)
	{
		Carre((n<<1),n,(n+1),Def+(n+1),Def+n);
		Carre((def<<1)+(n<<1),Def+n,Def+(n+1),(Def<<1)+(n+1),2*Def+n);
		Carre((def<<2)+(n<<1),(Def<<1)+n,(Def<<1)+(n+1),(Def<<1)+Def+(n+1),(Def<<1)+Def+n);
	}

	for (n=0;n<def*6;n++) Faces[n].tag=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen sphere

		void CObject3D::Sphere(float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Sphere(float R,int def)
{
	int n,nn,n0,n1,n2,n3;
	float h,r,x,y;

	Init(2+(def-1)*def,(def-2)*def*2+2*def);
	sprintf(Name,"OBJ");

	for (n=0;n<def-1;n++)
	{
		r=(float) (R*sinf(PI*(1+n)/def));
		h=(float) (-R*cosf(PI*(1+n)/def));

		for (nn=0;nn<def;nn++)
		{
			x=(float) (r*cosf(2*PI*nn/def));
			y=(float) (r*sinf(2*PI*nn/def));
			Vertices[2+n*def+nn].Stok.Init(x,y,h);

			if (nn<def/2)
			{
				x=(float) nn/(def/2);
			}
			else
			{
				x=1-(float) (nn-def/2)/(def/2);
			}

			x=(float) nn/(def);


			Vertices[2+n*def+nn].Map.Init(h/(2*R) + 0.5f,x);

			x=(h/(2*R)+0.5f);
			x=x*x*x;
			Vertices[2+n*def+nn].Diffuse.Init(x,x,x);
		}
	}
	Vertices[0].Stok.Init(0,0,-R);
	Vertices[0].Map.Init(-2+0.5f,0);
	Vertices[0].Diffuse.Init(0,0,0);

	Vertices[1].Stok.Init(0,0,R);
	Vertices[1].Map.Init(2+0.5f,0);
	Vertices[1].Diffuse.Init(1,1,1);


	for (n=0;n<def-2;n++)
	{
		for (nn=0;nn<def;nn++)
		{
			n0=(nn+1)%def;
			n1=nn;

			n2=n1+def;
			n3=n0+def;

			Faces[n*def*2 + nn*2+0].Init(2+n*def+n0,2+n*def+n1,2+n*def+n2);
			Faces[n*def*2 + nn*2+1].Init(2+n*def+n0,2+n*def+n2,2+n*def+n3);

			//Faces[n*def*2 + nn*2+0].Diffuse.Init(1,1,1);
			//Faces[n*def*2 + nn*2+1].Diffuse.Init(1,1,1);

			Faces[n*def*2 + nn*2+0].tag=0;
			Faces[n*def*2 + nn*2+1].tag=0;

		}

	}

	for (nn=0;nn<def;nn++)
	{
		n0=nn;
		n1=(nn+1)%def;


		n2=n1+(def-2)*def;
		n3=n0+(def-2)*def;

		Faces[(def-2)*def*2 + nn].Init(0,2+n0,2+n1);
		//Faces[(def-2)*def*2 + nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + nn].tag=0;

		Faces[(def-2)*def*2 + def+nn].Init(1,2+n2,2+n3);
		//Faces[(def-2)*def*2 + def+nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + def+nn].tag=0;
	}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen bone

		void CObject3D::Bone(float R)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Bone(float R)
{
	float R2=R/2;

	Init(2+4,4+4);
	sprintf(Name,"OBJ");

	Vertices[0].Stok.Init(0,0,-R);
	Vertices[1].Stok.Init(0,0,R);

	Vertices[2+0].Stok.Init(-R2,-R2,0);
	Vertices[2+1].Stok.Init(R2,-R2,0);
	Vertices[2+2].Stok.Init(R2,R2,0);
	Vertices[2+3].Stok.Init(-R2,R2,0);

	Faces[0+0].Init(0,2+0,2+1);
	Faces[0+1].Init(0,2+1,2+2);
	Faces[0+2].Init(0,2+2,2+3);
	Faces[0+3].Init(0,2+3,2+0);

	Faces[4+0].Init(1,2+0,2+3);
	Faces[4+1].Init(1,2+1,2+0);
	Faces[4+2].Init(1,2+2,2+1);
	Faces[4+3].Init(1,2+3,2+2);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen opened sphere

		void CObject3D::OpenSphere(float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::OpenSphere(float R,int def)
{
	int n,nn,n0,n1,n2,n3;
	float h,r,x,y;

	Init(2+(def-1)*(def+1),(def-2)*def*2+2*def);
	sprintf(Name,"OBJ");

	for (n=0;n<def-1;n++)
	{
		r=(float) (R*sinf(PI*(1+n)/def));
		h=(float) (-R*cosf(PI*(1+n)/def));

		for (nn=0;nn<def+1;nn++)
		{
			x=(float) (r*cosf(2*PI*nn/def));
			y=(float) (r*sinf(2*PI*nn/def));
			Vertices[2+n*(def+1)+nn].Stok.Init(x,y,h);

			if (nn<def/2)
			{
				x=(float) nn/(def/2);
			}
			else
			{
				x=1-(float) (nn-def/2)/(def/2);
			}

			x=(float) nn/(def);


			Vertices[2+n*(def+1)+nn].Map.Init(h/(2*R) + 0.5f,x);

			x=(h/(2*R)+0.5f);
			x=x*x*x;
			Vertices[2+n*(def+1)+nn].Diffuse.Init(x,x,x);
		}
	}
	Vertices[0].Stok.Init(0,0,-R);
	Vertices[0].Map.Init(-2+0.5f,0);
	Vertices[0].Diffuse.Init(0,0,0);

	Vertices[1].Stok.Init(0,0,R);
	Vertices[1].Map.Init(2+0.5f,0);
	Vertices[1].Diffuse.Init(1,1,1);


	for (n=0;n<def-2;n++)
	{
		for (nn=0;nn<def;nn++)
		{
			n0=nn+1;
			n1=nn;

			n2=n1+def+1;
			n3=n0+def+1;

			Faces[n*def*2 + nn*2+0].Init(2+n*(def+1)+n0,2+n*(def+1)+n2,2+n*(def+1)+n1);
			Faces[n*def*2 + nn*2+1].Init(2+n*(def+1)+n0,2+n*(def+1)+n3,2+n*(def+1)+n2);

			//Faces[n*def*2 + nn*2+0].Diffuse.Init(1,1,1);
			//Faces[n*def*2 + nn*2+1].Diffuse.Init(1,1,1);

			Faces[n*def*2 + nn*2+0].tag=0;
			Faces[n*def*2 + nn*2+1].tag=0;

		}

	}

	for (nn=0;nn<def;nn++)
	{
		n0=nn;
		n1=nn+1;


		n2=n1+(def-2)*(def+1);
		n3=n0+(def-2)*(def+1);

		Faces[(def-2)*def*2 + nn].Init(0,2+n1,2+n0);
		//Faces[(def-2)*def*2 + nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + nn].tag=0;

		Faces[(def-2)*def*2 + def+nn].Init(1,2+n3,2+n2);
		//Faces[(def-2)*def*2 + def+nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + def+nn].tag=0;
	}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen opened sphere

		void CObject3D::OpenSphereTri(float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::OpenSphereTri(float R,int def)
{
	int n,nn,n0,n1,n2,n3;
	float h,r,x,y;

	Init(2+(def-1)*(def+1),(def-2)*def*2+2*def);
	sprintf(Name,"OBJ");

	for (n=0;n<def-1;n++)
	{
		r=(float) (R*sinf(PI*(1+n)/def));
		h=(float) (-R*cosf(PI*(1+n)/def));

		for (nn=0;nn<def+1;nn++)
		{
			x=(float) (r*cosf(2*PI*(nn+0.5f*(n&1))/def));
			y=(float) (r*sinf(2*PI*(nn+0.5f*(n&1))/def));
			Vertices[2+n*(def+1)+nn].Stok.Init(x,y,h);

			if (nn<def/2)
			{
				x=(float) nn/(def/2);
			}
			else
			{
				x=1-(float) (nn-def/2)/(def/2);
			}

			x=(float) nn/(def);


			Vertices[2+n*(def+1)+nn].Map.Init(h/(2*R) + 0.5f,x);

			x=(h/(2*R)+0.5f);
			x=x*x*x;
			Vertices[2+n*(def+1)+nn].Diffuse.Init(x,x,x);
		}
	}
	Vertices[0].Stok.Init(0,0,-R);
	Vertices[0].Map.Init(-2+0.5f,0);
	Vertices[0].Diffuse.Init(0,0,0);

	Vertices[1].Stok.Init(0,0,R);
	Vertices[1].Map.Init(2+0.5f,0);
	Vertices[1].Diffuse.Init(1,1,1);


	for (n=0;n<def-2;n++)
	{
		for (nn=0;nn<def;nn++)
		{
			n0=nn+1;
			n1=nn;

			n2=n1+def+1;
			n3=n0+def+1;

			if ((n&1)==0)
			{
				Faces[n*def*2 + nn*2+0].Init(2+n*(def+1)+n0,2+n*(def+1)+n2,2+n*(def+1)+n1);
				Faces[n*def*2 + nn*2+1].Init(2+n*(def+1)+n0,2+n*(def+1)+n3,2+n*(def+1)+n2);
			}
			else
			{
				Faces[n*def*2 + nn*2+0].Init(2+n*(def+1)+n1,2+n*(def+1)+n0,2+n*(def+1)+n3);
				Faces[n*def*2 + nn*2+1].Init(2+n*(def+1)+n1,2+n*(def+1)+n3,2+n*(def+1)+n2);
			}

			//Faces[n*def*2 + nn*2+0].Diffuse.Init(1,1,1);
			//Faces[n*def*2 + nn*2+1].Diffuse.Init(1,1,1);

			Faces[n*def*2 + nn*2+0].tag=0;
			Faces[n*def*2 + nn*2+1].tag=0;

		}

	}

	for (nn=0;nn<def;nn++)
	{
		n0=nn;
		n1=nn+1;


		n2=n1+(def-2)*(def+1);
		n3=n0+(def-2)*(def+1);

		Faces[(def-2)*def*2 + nn].Init(0,2+n1,2+n0);
		//Faces[(def-2)*def*2 + nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + nn].tag=0;

		Faces[(def-2)*def*2 + def+nn].Init(1,2+n3,2+n2);
		//Faces[(def-2)*def*2 + def+nn].Diffuse.Init(1,1,1);
		Faces[(def-2)*def*2 + def+nn].tag=0;
	}

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen opened sphere

		void CObject3D::OpenSphere2(float R,int def)

		def : parameter on number of shapes

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::OpenSphere2(float R,int def)
{
    OpenSphere2(R,def,0);
}

void CObject3D::OpenSphere2(float R,int def,int tag)
{
    int n,nn,n0,n1,n2,n3;
    float h,r,x,y,d;
    float decalage=PI/(2*def);

    Init(2+(def-1)*(def+1),(def-2)*def*2+2*def);
    sprintf(Name,"OBJ");

    for (n=0;n<def-1;n++)
    {
        r=(float) (R*sinf(PI*(n)/(def-2)));
        h=(float) (-R*cosf(PI*(n)/(def-2)));

        for (nn=0;nn<=def;nn++)
        {
            if ((nn&1)) d=decalage;
            else d=0;
            x=(float) (r*cosf(d+(2*PI*nn/def)));
            y=(float) (r*sinf(d+(2*PI*nn/def)));
            Vertices[2+n*(def+1)+nn].Stok.Init(x,y,h);
            if (nn<def/2) x=(float) nn/(def/2);
            else x=1-(float) (nn-def/2)/(def/2);
            if (tag==0) x=(float) nn/(def);
            Vertices[2+n*(def+1)+nn].Map.Init(x,((float) asinf(h/R)/(PI)) + 0.5f);
            Vertices[2+n*(def+1)+nn].Diffuse.Init(1.0f,1.0f,1.0f);
        }
    }
    Vertices[0].Stok.Init(0,0,-R);
    Vertices[0].Map.Init(0,0);
    Vertices[0].Diffuse.Init(1,1,1);

    Vertices[1].Stok.Init(0,0,R);
    Vertices[1].Map.Init(0,0);
    Vertices[1].Diffuse.Init(1,1,1);

    for (n=0;n<def-2;n++)
    {
        for (nn=0;nn<def;nn++)
        {
            n0=nn+1;
            n1=nn;

            n2=n1+def+1;
            n3=n0+def+1;

            Faces[n*def*2 + nn*2+0].Init(2+n*(def+1)+n0,2+n*(def+1)+n2,2+n*(def+1)+n1);
            Faces[n*def*2 + nn*2+1].Init(2+n*(def+1)+n0,2+n*(def+1)+n3,2+n*(def+1)+n2);

            //Faces[n*def*2 + nn*2+0].Diffuse.Init(1,1,1);
            //Faces[n*def*2 + nn*2+1].Diffuse.Init(1,1,1);

            Faces[n*def*2 + nn*2+0].tag=0;
            Faces[n*def*2 + nn*2+1].tag=0;

        }

    }

    for (nn=0;nn<def;nn++)
    {
        n0=nn;
        n1=nn+1;


        n2=n1+(def-2)*(def+1);
        n3=n0+(def-2)*(def+1);

        Faces[(def-2)*def*2 + nn].Init(0,2+n1,2+n0);
        //Faces[(def-2)*def*2 + nn].Diffuse.Init(1,1,1);
        Faces[(def-2)*def*2 + nn].tag=0;

        Faces[(def-2)*def*2 + def+nn].Init(1,2+n3,2+n2);
        //Faces[(def-2)*def*2 + def+nn].Diffuse.Init(1,1,1);
        Faces[(def-2)*def*2 + def+nn].tag=0;
    }


    SetFaces();
    CalculateNormals2(-1);

    SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        gen opened sphere

        void CObject3D::OpenSphere3(float R,int def)

        def : parameter on number of shapes

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::OpenSphere3(float R,int def)
{
    OpenSphere3(R,def,0);
}

void CObject3D::OpenSphere3(float R,int def,int tag)
{
    int n,nn,n0,n1,n2,n3;
    float h,r,x,y,d;
    float decalage=PI/(2*def);

    Init((def)*(def+1),(def-1)*def*2);
    sprintf(Name,"OBJ");

    for (n=0;n<def;n++)
    {
        r=(float) (R*sinf(PI*(n)/(def-1)));
        h=(float) (-R*cosf(PI*(n)/(def-1)));

        for (nn=0;nn<=def;nn++)
        {
            if ((nn&1)) d=decalage;
            else d=0;
            x=(float) (r*cosf((2*PI*nn/def)));
            y=(float) (r*sinf((2*PI*nn/def)));
            Vertices[n*(def+1)+nn].Stok.Init(x,h,y);
            if (nn<def/2) x=(float) nn/(def/2);
            else x=1-(float) (nn-def/2)/(def/2);
            if (tag==0) x=(float) nn/(def);
            Vertices[n*(def+1)+nn].Map.Init(x,((float) asinf(h/R)/(PI)) + 0.5f);
            Vertices[n*(def+1)+nn].Diffuse.Init(1.0f,1.0f,1.0f);
        }
    }

    for (n=0;n<def-1;n++)
    {
        for (nn=0;nn<def;nn++)
        {
            n0=nn+1;
            n1=nn;

            n2=n1+def+1;
            n3=n0+def+1;

            Faces[n*def*2 + nn*2+0].Init(n*(def+1)+n0,n*(def+1)+n2,n*(def+1)+n1);
            Faces[n*def*2 + nn*2+1].Init(n*(def+1)+n0,n*(def+1)+n3,n*(def+1)+n2);

            //Faces[n*def*2 + nn*2+0].Diffuse.Init(1,1,1);
            //Faces[n*def*2 + nn*2+1].Diffuse.Init(1,1,1);

            Faces[n*def*2 + nn*2+0].tag=0;
            Faces[n*def*2 + nn*2+1].tag=0;
        }
    }

    SetFaces();
    
    InverseOrientation();
    
    CalculateNormals2(-1);

    SetRefTexture();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		for generation of object

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Carre2(int nf,int n0,int n1,int n2,int n3)
{

	Faces[nf+0].tag=0;
	Faces[nf+1].tag=0;

	Faces[nf+0].v0=n0;
	Faces[nf+0].v1=n1;
	Faces[nf+0].v2=n2;

	Faces[nf+1].v0=n0;
	Faces[nf+1].v1=n2;
	Faces[nf+1].v2=n3;

	Vertices[n0].Map.Init(0,0);
	Vertices[n1].Map.Init(1,0);
	Vertices[n2].Map.Init(1,1);
	Vertices[n3].Map.Init(0,1);

}


void CObject3D::Carre(int nf,int n0,int n1,int n2,int n3)
{
	Faces[nf+0].tag=0;
	Faces[nf+1].tag=0;

	Faces[nf+0].v0=n0;
	Faces[nf+0].v1=n1;
	Faces[nf+0].v2=n2;

	Faces[nf+1].v0=n0;
	Faces[nf+1].v1=n2;
	Faces[nf+1].v2=n3;


	Faces[nf+0].mp0.Init(0,0);
	Faces[nf+0].mp1.Init(1,0);
	Faces[nf+0].mp2.Init(1,1);

	Faces[nf+1].mp0.Init(0,0);
	Faces[nf+1].mp1.Init(1,1);
	Faces[nf+1].mp2.Init(0,1);
}


void CObject3D::Carrei(int nf,int n3,int n2,int n1,int n0)
{
	Faces[nf+0].tag=0;
	Faces[nf+1].tag=0;

	Faces[nf+0].v0=n0;
	Faces[nf+0].v1=n1;
	Faces[nf+0].v2=n2;

	Faces[nf+1].v0=n0;
	Faces[nf+1].v1=n2;
	Faces[nf+1].v2=n3;


	Faces[nf+0].mp0.Init(0,0);
	Faces[nf+0].mp1.Init(1,0);
	Faces[nf+0].mp2.Init(1,1);

	Faces[nf+1].mp0.Init(0,0);
	Faces[nf+1].mp1.Init(1,1);
	Faces[nf+1].mp2.Init(0,1);
}


void CObject3D::Triangle(int nf,int n0,int n1,int n2)
{
	Faces[nf+0].v0=n0;
	Faces[nf+0].v1=n1;
	Faces[nf+0].v2=n2;
	Faces[nf+0].tag=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        vars to occlusion optimizations

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::OptimizedOcc()
{
    optimizedG=Center();
    float R=0.0f;
    CVector u;
    for (int n=0;n<nVertices;n++)
    {
        VECTORSUB(u,Vertices[n].Stok,optimizedG);
        float r=VECTORNORM2(u);
        if (r>R) R=r;
    }
    optimizedRadius=sqrtf(R);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		apply transformation on "Calc" vertex data

		void CObject3D::Calculate(CMatrix *Mobj,CMatrix *Mrot)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Calculate(CMatrix *Mobj,CMatrix *Mrot)
{
	int n;

	for (n=0;n<nVertices;n++)
	{
		VECTOR3PMATRIX(Vertices[n].Calc,Vertices[n].Stok,Mobj)
		VECTOR3PMATRIX3X3(Vertices[n].NormCalc,Vertices[n].Norm,Mrot)
	}

	for (n=0;n<nFaces;n++)
	{
		VECTOR3PMATRIX3X3(Faces[n].NormCalc,Faces[n].Norm,Mrot)
	}

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		apply transformation on "Calc" vertex data
		takes matrix from .Coo and .Rot

		void CObject3D::Calculate()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Calculate()
{
	int n;
	CMatrix Mobj;

	Mobj.Id();
	Mobj.RotationDegre(Rot.x,Rot.y,Rot.z);
	Mobj.Translation(Coo.x,Coo.y,Coo.z);

	for (n=0;n<nVertices;n++)
	{
		VECTOR3MATRIX(Vertices[n].Calc,Vertices[n].Stok,Mobj)
		VECTOR3MATRIX3X3(Vertices[n].NormCalc,Vertices[n].Norm,Mobj)
	}

	for (n=0;n<nFaces;n++)
	{
		VECTOR3MATRIX3X3(Faces[n].NormCalc,Faces[n].Norm,Mobj)
	}

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::AddSelected(CObject3D *objbase,int tag)
{
    if (objbase)
    {
        CObject3D *obj=objbase->DuplicateSelected2(tag);
        if (obj)
        {
            Add(obj);
            obj->Free();
            delete obj;
        }
        update=true;
    }
}

void CObject3D::AddOnSelected(CObject3D *obj,int tag)
{
    int n;
    int nv=0;
    int nf=0;
    int p,k;
    CVector u;
    
    if (obj==NULL) return;
    
    float r;
    float rmax=0;
    for (n=0;n<nVertices;n++)
    {
        r=VECTORNORM2(Vertices[n].Stok);
        if (r>rmax) rmax=r;
    }
    
    if (rmax==0.0f) r=16.0f; else r=256.0f/sqrtf(rmax);
    for (n=0;n<nVertices;n++)
        Vertices[n].value=((int)((256+Vertices[n].Stok.x*r)))+512*(((int)((256+Vertices[n].Stok.y*r))) + 512*((int)((256+Vertices[n].Stok.z*r))));

    int * corres=new int[nVertices];
    
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    
    for (n=0;n<nFaces;n++)
    if (Faces[n].tag==tag)
    {
        nf++;
        Faces[n].v[0]->tag=1;
        Faces[n].v[1]->tag=1;
        Faces[n].v[2]->tag=1;
    }
    
    for (n=0;n<nVertices;n++)
        if (Vertices[n].tag) nv++;
    
    CVertex * vertices=new CVertex[nv+obj->nVertices];
    CFace * faces=new CFace[nf+obj->nFaces];
    
    p=0;
    for (n=0;n<nVertices;n++)
        if (Vertices[n].tag)
        {
            corres[n]=p;
            vertices[p++]=Vertices[n];
        }
    
    for (n=0;n<obj->nVertices;n++)
    {
        vertices[nv+n]=obj->Vertices[n];
        
        int val=((int)((256+obj->Vertices[n].Stok.x*r)))+512*(((int)((256+obj->Vertices[n].Stok.y*r))) + 512*((int)((256+obj->Vertices[n].Stok.z*r))));
        
        for (k=0;k<nVertices;k++)
        if ((Vertices[k].value==val)&&(Vertices[k].tag==0))
        {
            VECTORSUB(u,obj->Vertices[n].Stok,Vertices[k].Stok);
            if (VECTORNORM2(u)<SMALLF)
            {
                vertices[nv+n].Index[0]=Vertices[k].Index[0];
                vertices[nv+n].Index[1]=Vertices[k].Index[1];
                vertices[nv+n].Index[2]=Vertices[k].Index[2];
                vertices[nv+n].Index[3]=Vertices[k].Index[3];
                
                vertices[nv+n].Weight[0]=Vertices[k].Weight[0];
                vertices[nv+n].Weight[1]=Vertices[k].Weight[1];
                vertices[nv+n].Weight[2]=Vertices[k].Weight[2];
                vertices[nv+n].Weight[3]=Vertices[k].Weight[3];
            }
        }
    }
    
    p=0;
    for (n=0;n<nFaces;n++)
        if (Faces[n].tag==tag)
        {
            faces[p]=Faces[n];
            faces[p].v0=corres[faces[p].v0];
            faces[p].v1=corres[faces[p].v1];
            faces[p].v2=corres[faces[p].v2];
            p++;
        }
    
    for (n=0;n<obj->nFaces;n++)
    {
        faces[nf+n]=obj->Faces[n];
        faces[nf+n].v0+=nv;
        faces[nf+n].v1+=nv;
        faces[nf+n].v2+=nv;
    }
    
    delete [] Vertices;
    delete [] Faces;
    delete [] corres;
    
    Vertices=vertices;
    Faces=faces;
    
    nVertices=nv+obj->nVertices;
    nFaces=nf+obj->nFaces;
    
    SetFaces();
    CalculateNormals(-1);
    
    update=true;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
    
void CObject3D::AddTri(int a,int b,int c)
{
    int n;

    CFace * faces=new CFace[nFaces+1];

    for (n=0;n<nFaces;n++) faces[n]=Faces[n];
    faces[nFaces].v0=a;
    faces[nFaces].v1=b;
    faces[nFaces].v2=c;

    delete [] Faces;

    Faces=faces;
    nFaces++;

    SetFaces();
    CalculateNormals(-1);

    update=true;
}

void CObject3D::AddTriV(CVector &A,CVector &B,CVector &C)
{
    int n;

    CVertex * vertices=new CVertex[nVertices+3];
    CFace * faces=new CFace[nFaces+1];

    for (n=0;n<nVertices;n++) vertices[n]=Vertices[n];
    vertices[nVertices+0].Stok=A;
    vertices[nVertices+1].Stok=B;
    vertices[nVertices+2].Stok=C;
    
    vertices[nVertices+0].Map.Init(0,0);
    vertices[nVertices+1].Map.Init(0,0);
    vertices[nVertices+2].Map.Init(0,0);

    for (n=0;n<nFaces;n++) faces[n]=Faces[n];
    faces[nFaces].v0=nVertices+0;
    faces[nFaces].v1=nVertices+1;
    faces[nFaces].v2=nVertices+2;

    delete [] Vertices;
    delete [] Faces;

    Vertices=vertices;
    Faces=faces;

    nVertices+=3;
    nFaces++;

    SetFaces();
    CalculateNormals(-1);

    update=true;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
	
void CObject3D::Add(CObject3D *obj)
{
	int n;
    
    if (obj)
    {
        CVertex * vertices=new CVertex[nVertices+obj->nVertices];
        CFace * faces=new CFace[nFaces+obj->nFaces];

        for (n=0;n<nVertices;n++) vertices[n]=Vertices[n];
        for (n=0;n<obj->nVertices;n++) vertices[nVertices+n]=obj->Vertices[n];

        for (n=0;n<nFaces;n++) faces[n]=Faces[n];
        for (n=0;n<obj->nFaces;n++)
        {
            faces[nFaces+n]=obj->Faces[n];
            faces[nFaces+n].v0+=nVertices;
            faces[nFaces+n].v1+=nVertices;
            faces[nFaces+n].v2+=nVertices;
        }

        delete [] Vertices;
        delete [] Faces;

        Vertices=vertices;
        Faces=faces;

        nVertices+=obj->nVertices;
        nFaces+=obj->nFaces;

        SetFaces();
        CalculateNormals(-1);

        update=true;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
	
void CObject3D::AddAsKey(CObject3D *obj)
{
	int n;
    
    if (obj)
    {
		if (nKeys==0)
		{
			nKeys=2;

			VerticesKeys[0]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[0][n].Stok=Vertices[n].Stok;
				VerticesKeys[0][n].Norm=Vertices[n].Norm;
			}
			VerticesKeys[1]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[1][n].Stok=obj->Vertices[n].Stok;
				VerticesKeys[1][n].Norm=obj->Vertices[n].Norm;
			}
		}
		else
		{
			VerticesKeys[nKeys]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[nKeys][n].Stok=obj->Vertices[n].Stok;
				VerticesKeys[nKeys][n].Norm=obj->Vertices[n].Norm;
			}
			nKeys++;
		}

        SetFaces();
        CalculateNormals(-1);

        update=true;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
	
void CObject3D::AddCalcAsKey(CObject3D *obj)
{
	int n;
    
    if (obj)
    {
		if (nKeys==0)
		{
			nKeys=2;

			VerticesKeys[0]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[0][n].Stok=Vertices[n].Stok;
				VerticesKeys[0][n].Norm=Vertices[n].Norm;
			}
			VerticesKeys[1]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[1][n].Stok=obj->Vertices[n].Calc;
				VerticesKeys[1][n].Norm=obj->Vertices[n].NormCalc;
			}
		}
		else
		{
			VerticesKeys[nKeys]=new CShortVertex[nVertices];
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[nKeys][n].Stok=obj->Vertices[n].Calc;
				VerticesKeys[nKeys][n].Norm=obj->Vertices[n].NormCalc;
			}
			nKeys++;
		}

        SetFaces();
        CalculateNormals(-1);

        update=true;
    }
}
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::AddFirst(CObject3D *obj)
{
    int n;
    
    if (obj)
    {
        CVertex * vertices=new CVertex[nVertices+obj->nVertices];
        CFace * faces=new CFace[nFaces+obj->nFaces];
        
        for (n=0;n<nVertices;n++) vertices[n]=Vertices[n];
        for (n=0;n<obj->nVertices;n++) vertices[nVertices+n]=obj->Vertices[n];
        
        for (n=0;n<nFaces;n++) faces[obj->nFaces+n]=Faces[n];
        for (n=0;n<obj->nFaces;n++)
        {
            faces[n]=obj->Faces[n];
            faces[n].v0+=nVertices;
            faces[n].v1+=nVertices;
            faces[n].v2+=nVertices;
        }
        
        delete [] Vertices;
        delete [] Faces;
        
        Vertices=vertices;
        Faces=faces;
        
        nVertices+=obj->nVertices;
        nFaces+=obj->nFaces;
        
        SetFaces();
        CalculateNormals(-1);
        
        update=true;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		apply transformation and erase old data

		void CObject3D::Apply(CMatrix M,CMatrix M2)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Apply(CMatrix &M,CMatrix &M2)
{
	int n,k;

	for (n=0;n<nVertices;n++)
	{
		VECTOR3MATRIX(Vertices[n].Stok,Vertices[n].Stok,M);
		VECTOR3MATRIX3X3(Vertices[n].Norm,Vertices[n].Norm,M2);
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++)
			{
				VECTOR3MATRIX(VerticesKeys[k][n].Stok,VerticesKeys[k][n].Stok,M);
				VECTOR3MATRIX3X3(VerticesKeys[k][n].Norm,VerticesKeys[k][n].Norm,M2);
			}
		}
	}

	for (n=0;n<nFaces;n++)
	{
		VECTOR3MATRIX3X3(Faces[n].Norm,Faces[n].Norm,M2);
	}

	update=true;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		apply transformation and erase old data
		takes matrix from .Coo and .Rot

		void CObject3D::Set()


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Set()
{
	int n,k;
	CMatrix Mobj,Mrot;
    
    update=true;
    if ((__nanish(Coo.x))||(__nanish(Coo.y))||(__nanish(Coo.z))) return;
    
	Mobj.Id();
	Mobj.RotationDegre(Rot.x,Rot.y,Rot.z);
	Mrot=Mobj;
	Mobj.Translation(Coo.x,Coo.y,Coo.z);

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok=Vertices[n].Stok*Mobj;
		Vertices[n].Norm=Vertices[n].Norm*Mrot;
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++)
			{
				VerticesKeys[k][n].Stok=VerticesKeys[k][n].Stok*Mobj;
				VerticesKeys[k][n].Norm=VerticesKeys[k][n].Norm*Mrot;
			}
		}
	}

	for (n=0;n<nFaces;n++)
		Faces[n].Norm=Faces[n].Norm*Mrot;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen plane

		void CObject3D::Plane(float Dim)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Plane(float Dim)
{

	Init(4,2);
	sprintf(Name,"OBJ");

	Vertices[0].Stok.Init(-Dim,0,-Dim);
	Vertices[1].Stok.Init(Dim,0,-Dim);
	Vertices[2].Stok.Init(Dim,0,Dim);
	Vertices[3].Stok.Init(-Dim,0,Dim);
	Vertices[0].Map.Init(-10,-10);
	Vertices[1].Map.Init(10,-10);
	Vertices[2].Map.Init(10,10);
	Vertices[3].Map.Init(-10,10);


	Carre(0		,3,2,1,0);

	Faces[0].tag=0;
	Faces[1].tag=0;

	SetFaces();
	CalculateNormals(-1);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen equi-Triangle 

		void CObject3D::EquiTriangle(float Dim)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::EquiTriangle(float Dim)
{
	float x1,y1,x2,y2,x3,y3;

	Init(3,1);
	sprintf(Name,"OBJ");

	x1=cosf(90.0f*PI/180.0f); y1=sinf(90.0f*PI/180.0f);
	x2=cosf(210.0f*PI/180.0f); y2=sinf(210.0f*PI/180.0f);
	x3=cosf(-30.0f*PI/180.0f); y3=sinf(-30.0f*PI/180.0f);

	Vertices[0].Stok.Init(Dim*x1,0,Dim*y1);
	Vertices[1].Stok.Init(Dim*x2,0,Dim*y2);
	Vertices[2].Stok.Init(Dim*x3,0,Dim*y3);

	x1=(1+cosf(90.0f*PI/180.0f))/2; y1=(1+sinf(90.0f*PI/180.0f))/2;
	x2=(1+cosf(210.0f*PI/180.0f))/2; y2=(1+sinf(210.0f*PI/180.0f))/2;
	x3=(1+cosf(-30.0f*PI/180.0f))/2; y3=(1+sinf(-30.0f*PI/180.0f))/2;

	Vertices[0].Map.Init(x1,y1);
	Vertices[1].Map.Init(x2,y2);
	Vertices[2].Map.Init(x3,y3);

	Triangle(0,0,1,2);
	Faces[0].tag=0;

	SetFaces();
	CalculateNormals(-1);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen cube

		void CObject3D::Cube2(float Lx,float Ly,float Lz)
		void CObject3D::Cube2i(float Lx,float Ly,float Lz)

		gen prism

		void CObject3D::Prism(float D,float L)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Prism(float D,float L)
{
	int n,m;
	float xy[3][2];
	float h[4]={-1,-1,1,1};

	for (n=0;n<3;n++)
	{
		xy[n][0]=(float) (D*cosf(n*2*PI/3));
		xy[n][1]=(float) (D*sinf(n*2*PI/3));
	}

	Init(3*4,2+6);
	sprintf(Name,"OBJ");

	for (n=0;n<4;n++)
	{
		for (m=0;m<3;m++)
		{
			Vertices[n*3+m].Stok.Init(xy[m][0],h[n]*L/2,xy[m][1]);
			Vertices[n*3+m].Map.Init((float) m/3,(1+h[n])/2);
		}
	}

	Triangle(0,0,1,2);
	Triangle(1,9+0,9+2,9+1);

	Carrei(2,3+0,3+1,6+1,6+0);
	Carrei(4,3+1,3+2,6+2,6+1);
	Carrei(6,3+2,3+0,6+0,6+2);

	SetFaces();
	CalculateNormals(-1);

	for (n=0;n<8;n++)
	{
		Faces[n].tag=0;
        Faces[n].tag2=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}
    
	SetRefTexture();
}


void CObject3D::Cube2(float Lx,float Ly,float Lz)
{
	int n;
	float sss[24][3]={
		{1,-1,-1},{-1,-1,-1},{-1,-1,1},{1,-1,1},
		{-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1},

		{1,-1,-1},{1,-1,1},{1,1,1},{1,1,-1},
		{1,-1,1},{-1,-1,1},{-1,1,1},{1,1,1},
		{-1,-1,1},{-1,-1,-1},{-1,1,-1},{-1,1,1},
		{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1}

	};

	Init(24,12);
	sprintf(Name,"OBJ");

	for (n=0;n<6;n++)
	{
		Vertices[4*n+0].Stok.Init(Lx*sss[4*n+0][0]/2,Ly*sss[4*n+0][1]/2,Lz*sss[4*n+0][2]/2);
		Vertices[4*n+1].Stok.Init(Lx*sss[4*n+1][0]/2,Ly*sss[4*n+1][1]/2,Lz*sss[4*n+1][2]/2);
		Vertices[4*n+2].Stok.Init(Lx*sss[4*n+2][0]/2,Ly*sss[4*n+2][1]/2,Lz*sss[4*n+2][2]/2);
		Vertices[4*n+3].Stok.Init(Lx*sss[4*n+3][0]/2,Ly*sss[4*n+3][1]/2,Lz*sss[4*n+3][2]/2);
		Vertices[4*n+0].Map.Init(0,0);
		Vertices[4*n+1].Map.Init(1,0);
		Vertices[4*n+2].Map.Init(1,1);
		Vertices[4*n+3].Map.Init(0,1);
	}

	Carre(0 , 0,1,2,3);
	Carre(2 , 4,5,6,7);
	Carre(4 , 8,9,10,11);
	Carre(6 , 12,13,14,15);
	Carre(8 , 16,17,18,19);
	Carre(10 , 20,21,22,23);

	SetFaces();
	CalculateNormals(-1);

	for (n=0;n<12;n++)
	{
		Faces[n].tag=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetRefTexture();
}


void CObject3D::Quad(CVector A,CVector B,CVector C,CVector D)
{
	CVector G=(A+B+C+D)/4;

	Init(4,2);
	sprintf(Name,"QUAD");

	Vertices[0].Stok=A-G;
	Vertices[1].Stok=B-G;
	Vertices[2].Stok=C-G;
	Vertices[3].Stok=D-G;

	Vertices[0].Calc=A;
	Vertices[1].Calc=B;
	Vertices[2].Calc=C;
	Vertices[3].Calc=D;

	Vertices[0].Map.Init(0,0);
	Vertices[1].Map.Init(1,0);
	Vertices[2].Map.Init(1,1);
	Vertices[3].Map.Init(0,1);

	Vertices[0].Diffuse.Init(1,1,1);
	Vertices[1].Diffuse.Init(1,1,1);
	Vertices[2].Diffuse.Init(1,1,1);
	Vertices[3].Diffuse.Init(1,1,1);

	Carre(0 , 0,1,2,3);

	SetFaces();
	CalculateNormals(-1);

	for (int n=0;n<2;n++)
	{
		Faces[n].tag=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetRefTexture();

	Coo=G;
	Rot.Init(0,0,0);
}

void CObject3D::DoubleQuad(CVector A,CVector B,CVector C,CVector D)
{
	Init(8,4);
	sprintf(Name,"QUAD");

	Vertices[0].Stok=A;
	Vertices[1].Stok=B;
	Vertices[2].Stok=C;
	Vertices[3].Stok=D;

	Vertices[4].Stok=D;
	Vertices[5].Stok=C;
	Vertices[6].Stok=B;
	Vertices[7].Stok=A;


	Vertices[0].Map.Init(0,0);
	Vertices[1].Map.Init(1,0);
	Vertices[2].Map.Init(1,1);
	Vertices[3].Map.Init(0,1);

	Vertices[7].Map.Init(0,0);
	Vertices[6].Map.Init(1,0);
	Vertices[5].Map.Init(1,1);
	Vertices[4].Map.Init(0,1);

	Vertices[0].Diffuse.Init(1,1,1);
	Vertices[1].Diffuse.Init(1,1,1);
	Vertices[2].Diffuse.Init(1,1,1);
	Vertices[3].Diffuse.Init(1,1,1);

	Vertices[4].Diffuse.Init(1,1,1);
	Vertices[5].Diffuse.Init(1,1,1);
	Vertices[6].Diffuse.Init(1,1,1);
	Vertices[7].Diffuse.Init(1,1,1);

	Carre(0 , 0,1,2,3);
	Carre(2 , 4,5,6,7);

	SetFaces();
	CalculateNormals(-1);

	for (int n=0;n<4;n++)
	{
		Faces[n].tag=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetRefTexture();

	Coo.Init(0,0,0);
	Rot.Init(0,0,0);
}



void CObject3D::Cube2i(float Lx,float Ly,float Lz)
{
	int n;
	float sss[24][3]={
		{1,-1,-1},{-1,-1,-1},{-1,-1,1},{1,-1,1},
		{-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1},

		{1,-1,-1},{1,-1,1},{1,1,1},{1,1,-1},
		{1,-1,1},{-1,-1,1},{-1,1,1},{1,1,1},
		{-1,-1,1},{-1,-1,-1},{-1,1,-1},{-1,1,1},
		{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1}

	};

	Init(24,12);
	sprintf(Name,"OBJ");

	for (n=0;n<6;n++)
	{
		Vertices[4*n+0].Stok.Init(Lx*sss[4*n+0][0]/2,Ly*sss[4*n+0][1]/2,Lz*sss[4*n+0][2]/2);
		Vertices[4*n+1].Stok.Init(Lx*sss[4*n+1][0]/2,Ly*sss[4*n+1][1]/2,Lz*sss[4*n+1][2]/2);
		Vertices[4*n+2].Stok.Init(Lx*sss[4*n+2][0]/2,Ly*sss[4*n+2][1]/2,Lz*sss[4*n+2][2]/2);
		Vertices[4*n+3].Stok.Init(Lx*sss[4*n+3][0]/2,Ly*sss[4*n+3][1]/2,Lz*sss[4*n+3][2]/2);
		Vertices[4*n+0].Map.Init(0,0);
		Vertices[4*n+1].Map.Init(1,0);
		Vertices[4*n+2].Map.Init(1,1);
		Vertices[4*n+3].Map.Init(0,1);
	}

	//Carrei(0 , 0,1,2,3);
	//Carrei(2 , 4,5,6,7);
	Carrei(0 , 3,2,1,0);
	Carrei(2 , 7,6,5,4);
    InverseOrientation();
	Carrei(4 , 8,9,10,11);
	Carrei(6 , 12,13,14,15);
	Carrei(8 , 16,17,18,19);
	Carrei(10 , 20,21,22,23);

	SetFaces();
	CalculateNormals(-1);

	for (n=0;n<12;n++)
	{
		Faces[n].tag=0;
        Faces[n].tag2=0;
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetRefTexture();
    
    InvertXMap();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		calculate center of an object

		CVector CObject3D::BoundingCenter()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CVector CObject3D::BoundingCenter()
{
	int n;
	CVector G,v,min,max;

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		v=Vertices[n].Stok;
		if (v.x>max.x) max.x=v.x;
		if (v.y>max.y) max.y=v.y;
		if (v.z>max.z) max.z=v.z;
		if (v.x<min.x) min.x=v.x;
		if (v.y<min.y) min.y=v.y;
		if (v.z<min.z) min.z=v.z;
	}

	G=(min+max)/2;

	return G;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		calculate center of an object

		CVector CObject3D::Bounding()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


float CObject3D::Bounding(int axe)
{
	int n;
	CVector v,min,max;

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		v=Vertices[n].Stok;
		if (v.x>max.x) max.x=v.x;
		if (v.y>max.y) max.y=v.y;
		if (v.z>max.z) max.z=v.z;
		if (v.x<min.x) min.x=v.x;
		if (v.y<min.y) min.y=v.y;
		if (v.z<min.z) min.z=v.z;
	}

	switch (axe)
	{
	case 0:
		return (max.x - min.x);
		break;
	case 1:
		return (max.y - min.y);
		break;
	case 2:
		return (max.z - min.z);
		break;
	};

	return -1.0f;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		center object

		void CObject3D::Centered()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CVector CObject3D::BoundingCentered()
{
	int n,k;
	CVector G;

    update=true;
	G=BoundingCenter();
    
    if ((__nanish(G.x))||(__nanish(G.y))||(__nanish(G.z))) return CVector(0,0,0);
    
	for (n=0;n<nVertices;n++) VECTORSUB(Vertices[n].Stok,Vertices[n].Stok,G);
	if (nKeys>0) { for (k=0;k<nKeys;k++) { for (n=0;n<nVertices;n++) VECTORSUB(VerticesKeys[k][n].Stok,VerticesKeys[k][n].Stok,G); } }

	return G;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
void CObject3D::BoundingCenteredCoo()
{
	int n,k;
	CVector G;

    update=true;
	G=BoundingCenter();
    
    if ((__nanish(G.x))||(__nanish(G.y))||(__nanish(G.z))) return;
    
	Coo=G;
	for (n=0;n<nVertices;n++) VECTORSUB(Vertices[n].Stok,Vertices[n].Stok,G);
	if (nKeys>0) { for (k=0;k<nKeys;k++) { for (n=0;n<nVertices;n++) VECTORSUB(VerticesKeys[k][n].Stok,VerticesKeys[k][n].Stok,G); } }
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		calculate center of an object

		CVector CObject3D::Center()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CVector CObject3D::Center()
{
	int n;
	CVector G;

	G.Init(0,0,0);
	for (n=0;n<nVertices;n++) VECTORADD(G,G,Vertices[n].Stok);
	VECTORDIV(G,G,(float)nVertices);

	return G;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		calculate center of an object

		CVector CObject3D::CenterCalc()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CVector CObject3D::CenterCalc()
{
	int n;
	CVector G;

	G.Init(0,0,0);
	for (n=0;n<nVertices;n++) VECTORADD(G,G,Vertices[n].Calc);
	VECTORDIV(G,G,(float)nVertices);

	return G;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		center object

		void CObject3D::Centered()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CVector CObject3D::Centered()
{
	int n,k;
	CVector G;

    update=true;
	G=Center();
    if ((__nanish(G.x))||(__nanish(G.y))||(__nanish(G.z))) return CVector(0,0,0);
    
	for (n=0;n<nVertices;n++) VECTORSUB(Vertices[n].Stok,Vertices[n].Stok,G);
	if (nKeys>0) { for (k=0;k<nKeys;k++) { for (n=0;n<nVertices;n++) VECTORSUB(VerticesKeys[k][n].Stok,VerticesKeys[k][n].Stok,G); } }
	
	return G;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CObject3D::Yup()
{
    int n;
    float y=1000000;
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.y<y) y=Vertices[n].Stok.y;
    }
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CObject3D::Ydown()
{
    int n;
    float y=-1000000;
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.y>y) y=Vertices[n].Stok.y;
    }
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CObject3D::CenteredVertical(bool up)
{
    int n,k;
    CVector G;

    update=true;
    G=Center();
    if (up) G.y=Yup(); else G.y=Ydown();
    if ((__nanish(G.x))||(__nanish(G.y))||(__nanish(G.z))) return CVector(0,0,0);
    
    for (n=0;n<nVertices;n++) VECTORSUB(Vertices[n].Stok,Vertices[n].Stok,G);
    if (nKeys>0) { for (k=0;k<nKeys;k++) { for (n=0;n<nVertices;n++) VECTORSUB(VerticesKeys[k][n].Stok,VerticesKeys[k][n].Stok,G); } }
    
    return G;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen arc

		void CObject3D::Arc(float r,float e,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Arc(float r,float e,int def)
{
	int n,nf;
	float angle,x;
	CVector X,Y,Z;

	X.Init(1,0,0);
	Y.Init(0,1,0);
	Z.Init(0,0,1);

	Init((def+1)*8+8,def*8+4);
	sprintf(Name,"OBJ");


	for (n=0;n<def+1;n++)
	{
		angle=-PI + PI*n/def;

		x=(float) n/def;

		Vertices[8*n+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
		Vertices[8*n+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

		Vertices[8*n+3].Stok.Init((float)((r+e)*cosf(angle)),(float)((r+e)*(sinf(angle))),e);
		Vertices[8*n+2].Stok.Init((float) ((r+e)*cosf(angle)),(float)((r+e)*(sinf(angle))),-e);


		Vertices[8*n+1+4].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
		Vertices[8*n+0+4].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

		Vertices[8*n+3+4].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),e);
		Vertices[8*n+2+4].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),-e);


		Vertices[8*n+1].Map.Init(x,0);
		Vertices[8*n+0].Map.Init(x,0.5f);

		Vertices[8*n+3].Map.Init(x,1);
		Vertices[8*n+2].Map.Init(x,0.5f);

		Vertices[8*n+1+4].Map.Init(x,0);
		Vertices[8*n+0+4].Map.Init(x,0.5f);

		Vertices[8*n+3+4].Map.Init(x,1);
		Vertices[8*n+2+4].Map.Init(x,0.5f);


	}

	angle=-PI;

	Vertices[8*(def+1)+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
	Vertices[8*(def+1)+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

	Vertices[8*(def+1)+3].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),e);
	Vertices[8*(def+1)+2].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),-e);

	angle=0;

	Vertices[4+8*(def+1)+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
	Vertices[4+8*(def+1)+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

	Vertices[4+8*(def+1)+3].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),e);
	Vertices[4+8*(def+1)+2].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),-e);

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok+=((2*r+e)/2)*Y;
	}


	nf=0;
	for (n=0;n<def;n++)
	{
		Carre(nf,n*8 +0,(n+1)*8+0,(n+1)*8 + 1, n*8+1);
		nf+=2;

		Carre(nf,n*8 +3,(n+1)*8+3,(n+1)*8 + 2, n*8+2);
		nf+=2;

		Carre(nf,n*8 +5,(n+1)*8+5,(n+1)*8 + 7, n*8+7);
		nf+=2;
		Carre(nf,n*8 +6,(n+1)*8+6,(n+1)*8 + 4, n*8+4);
		nf+=2;
	}


	//Carre(nf,0,1,3,2);
	Carre(nf,(def+1)*8+0,(def+1)*8+1,(def+1)*8+3,(def+1)*8+2);
	nf+=2;
	Carre(nf,(def+1)*8+2+4,(def+1)*8+3+4,(def+1)*8+1+4,(def+1)*8+0+4);

	for (n=0;n<nf+2;n++) Faces[n].tag=0;


//	Centre();
	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen arc

		void CObject3D::Arc1(float r,float r2,float e,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Arc1(float r,float r2,float e,int def)
{
	int n,nf;
	float angle,x;
	CVector X,Y,Z;

	X.Init(1,0,0);
	Y.Init(0,1,0);
	Z.Init(0,0,1);

	Init((def+1)*8+8,def*8+4);
	sprintf(Name,"OBJ");


	for (n=0;n<def+1;n++)
	{
		angle=-PI + PI*n/def;

		x=(float) n/def;

		Vertices[8*n+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
		Vertices[8*n+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

		Vertices[8*n+3].Stok.Init((float)((r2)*cosf(angle)),(float)((r2)*(sinf(angle))),e);
		Vertices[8*n+2].Stok.Init((float) ((r2)*cosf(angle)),(float)((r2)*(sinf(angle))),-e);


		Vertices[8*n+1+4].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
		Vertices[8*n+0+4].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

		Vertices[8*n+3+4].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),e);
		Vertices[8*n+2+4].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),-e);


		Vertices[8*n+1].Map.Init(x,0);
		Vertices[8*n+0].Map.Init(x,0.5f);

		Vertices[8*n+3].Map.Init(x,1);
		Vertices[8*n+2].Map.Init(x,0.5f);

		Vertices[8*n+1+4].Map.Init(x,0);
		Vertices[8*n+0+4].Map.Init(x,0.5f);

		Vertices[8*n+3+4].Map.Init(x,1);
		Vertices[8*n+2+4].Map.Init(x,0.5f);


	}

	angle=-PI;

	Vertices[8*(def+1)+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
	Vertices[8*(def+1)+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

	Vertices[8*(def+1)+3].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),e);
	Vertices[8*(def+1)+2].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),-e);

	angle=0;

	Vertices[4+8*(def+1)+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
	Vertices[4+8*(def+1)+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

	Vertices[4+8*(def+1)+3].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),e);
	Vertices[4+8*(def+1)+2].Stok.Init((float) ((r2)*cosf(angle)),(float) ((r2)*(sinf(angle))),-e);

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok+=((2*r+e)/2)*Y;
	}


	nf=0;
	for (n=0;n<def;n++)
	{
		Carre(nf,n*8 +0,(n+1)*8+0,(n+1)*8 + 1, n*8+1);
		nf+=2;

		Carre(nf,n*8 +3,(n+1)*8+3,(n+1)*8 + 2, n*8+2);
		nf+=2;

		Carre(nf,n*8 +5,(n+1)*8+5,(n+1)*8 + 7, n*8+7);
		nf+=2;
		Carre(nf,n*8 +6,(n+1)*8+6,(n+1)*8 + 4, n*8+4);
		nf+=2;
	}


	//Carre(nf,0,1,3,2);
	Carre(nf,(def+1)*8+0,(def+1)*8+1,(def+1)*8+3,(def+1)*8+2);
	nf+=2;
	Carre(nf,(def+1)*8+2+4,(def+1)*8+3+4,(def+1)*8+1+4,(def+1)*8+0+4);

	for (n=0;n<nf+2;n++) Faces[n].tag=0;


//	Centre();
	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		gen arc

		void CObject3D::Arc0(float r,float e,int def)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Arc0(float r,float e,int def)
{
	int n,nf;
	float angle,x;
	CVector X,Y,Z;

	X.Init(1,0,0);
	Y.Init(0,1,0);
	Z.Init(0,0,1);

	Init((def+1)*4,def*8+4);
	sprintf(Name,"OBJ");

	for (n=0;n<def+1;n++)
	{
		angle=-PI + PI*n/def;

		x=(float) n/def;

		Vertices[4*n+1].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),e);
		Vertices[4*n+0].Stok.Init((float) (r*cosf(angle)),(float) (r*(sinf(angle))),-e);

		Vertices[4*n+3].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),e);
		Vertices[4*n+2].Stok.Init((float) ((r+e)*cosf(angle)),(float) ((r+e)*(sinf(angle))),-e);



		Vertices[4*n+1].Map.Init(x,0);
		Vertices[4*n+0].Map.Init(x,0.5f);

		Vertices[4*n+3].Map.Init(x,1);
		Vertices[4*n+2].Map.Init(x,0.5f);

	}

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Stok+=((2*r+e)/2)*Y;
	}


	nf=0;
	for (n=0;n<def;n++)
	{
		Carre(nf,n*4 +0,(n+1)*4+0,(n+1)*4 + 1, n*4+1);
		nf+=2;

		Carre(nf,n*4 +3,(n+1)*4+3,(n+1)*4 + 2, n*4+2);
		nf+=2;

		Carre(nf,n*4 +1,(n+1)*4+1,(n+1)*4 + 3, n*4+3);
		nf+=2;
		Carre(nf,n*4 +2,(n+1)*4+2,(n+1)*4 + 0, n*4+0);
		nf+=2;
	}


	Carre(nf,0,1,3,2);
	nf+=2;
	Carre(nf,(def)*4+2,(def)*4+3,(def)*4+1,(def)*4+0);

	for (n=0;n<nf+2;n++) Faces[n].tag=0;


//	Centre();
	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		face length
 
        float CObject3D::CalculateAverageFaceLength()
        float CObject3D::CalculateShortestFaceLength()
        float CObject3D::CalculateLargestFaceLength()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

float CObject3D::CalculateAverageFaceLength()
{
    CVector u;
    float r=0;
    int n;
    
    for (n=0;n<nFaces;n++)
    {
        VECTORSUB(u,Faces[n].v[0]->Stok,Faces[n].v[1]->Stok)
        r+=VECTORNORM(u);
        VECTORSUB(u,Faces[n].v[1]->Stok,Faces[n].v[2]->Stok)
        r+=VECTORNORM(u);
        VECTORSUB(u,Faces[n].v[2]->Stok,Faces[n].v[0]->Stok)
        r+=VECTORNORM(u);
    }
    
    if (nFaces>0) r/=nFaces*3;
    
    return r;
}

float CObject3D::CalculateShortestFaceLength()
{
    CVector u;
    float fn;
    float r=MAXI_BOUND;
    int n;
    
    for (n=0;n<nFaces;n++)
    {
        VECTORSUB(u,Faces[n].v[0]->Stok,Faces[n].v[1]->Stok)
        VECTORNORME2(fn,u);
        if (fn<r) r=fn;
        VECTORSUB(u,Faces[n].v[1]->Stok,Faces[n].v[2]->Stok)
        VECTORNORME2(fn,u);
        if (fn<r) r=fn;
        VECTORSUB(u,Faces[n].v[2]->Stok,Faces[n].v[0]->Stok)
        VECTORNORME2(fn,u);
        if (fn<r) r=fn;
    }
    return ((float) sqrtf(r));
}

float CObject3D::CalculateLargestFaceLength()
{
    CVector u;
    float fn;
    float r=0;
    int n;
    
    for (n=0;n<nFaces;n++)
    {
        VECTORSUB(u,Faces[n].v[0]->Stok,Faces[n].v[1]->Stok)
        VECTORNORME2(fn,u);
        if (fn>r) r=fn;
        VECTORSUB(u,Faces[n].v[1]->Stok,Faces[n].v[2]->Stok)
        VECTORNORME2(fn,u);
        if (fn>r) r=fn;
        VECTORSUB(u,Faces[n].v[2]->Stok,Faces[n].v[0]->Stok)
        VECTORNORME2(fn,u);
        if (fn>r) r=fn;
    }
    return ((float) sqrtf(r));
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        radius
        float CObject3D::CalculateRadius()

        shortest radius
        float CObject3D::CalculateMinimalRadius()

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

float CObject3D::CalculateRadius()
{
    float fn;
    float r;
    int n;
    
    r=0;
    for (n=0;n<nVertices;n++)
    {
        VECTORNORME2(fn,Vertices[n].Stok);
        if (fn>r) r=fn;
    }
    return ((float) sqrtf(r));
}

float CObject3D::CalculatePlanarRadius()
{
    float fn;
    float r;
    int n;
    
    r=0;
    for (n=0;n<nVertices;n++)
    {
        CVector p=Vertices[n].Stok;
        p.y=0.0f;
        VECTORNORME2(fn,p);
        if (fn>r) r=fn;
    }
    return ((float) sqrtf(r));
}


float CObject3D::CalculateOptimalRadius()
{
	float r;
	int n;
	CVector u;

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
	
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].Stok.x>max.x) max.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y>max.y) max.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z>max.z) max.z=Vertices[n].Stok.z;
		
		if (Vertices[n].Stok.x<min.x) min.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y<min.y) min.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z<min.z) min.z=Vertices[n].Stok.z;
	}

	world_pos.x=(min.x+max.x)/2;
	world_pos.y=(min.y+max.y)/2;
	world_pos.z=(min.z+max.z)/2;

	r=sqrtf((max.x-world_pos.x)*(max.x-world_pos.x)+(max.y-world_pos.y)*(max.y-world_pos.y)+(max.z-world_pos.z)*(max.z-world_pos.z));

	return r;
}


float CObject3D::CalculateVolum()
{
	float x0,x1,y0,y1,z0,z1;
	int n;

	x0=MAXI_BOUND;
	x1=-MAXI_BOUND;
	y0=MAXI_BOUND;
	y1=-MAXI_BOUND;
	z0=MAXI_BOUND;
	z1=-MAXI_BOUND;

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].Stok.x>x1) x1=Vertices[n].Stok.x;
		if (Vertices[n].Stok.x<x0) x0=Vertices[n].Stok.x;

		if (Vertices[n].Stok.y>y1) y1=Vertices[n].Stok.y;
		if (Vertices[n].Stok.y<y0) y0=Vertices[n].Stok.y;

		if (Vertices[n].Stok.z>z1) z1=Vertices[n].Stok.z;
		if (Vertices[n].Stok.z<z0) z0=Vertices[n].Stok.z;
	}

	return ((x1-x0)*(y1-y0)*(z1-z0));
}


float CObject3D::CalculateMinimalRadius()
{
	float r,a,b,c,ss;
	int n;
	CVector Nn;

	r=65536*256;
	for (n=0;n<nFaces;n++)
	{
		Nn=Faces[n].Norm;
		a=Nn.x;
		b=Nn.y;
		c=Nn.z;
		ss=f_abs(a*Faces[n].v[0]->Stok.x + b*Faces[n].v[0]->Stok.y + c*Faces[n].v[0]->Stok.z);
		if (ss<r) r=ss;
	}
	return r;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		CObject3D CObject3D::SphereDerivate()



	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::SphereDerivate()
{
	return (*SphereDerivate2());
}

CObject3D* CObject3D::SphereDerivate2()
{
	CObject3D *duplicated;
	int n,nn;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
	float f;

	duplicated=new CObject3D;

	f=CalculateRadius();

	QuickEdges ed;
    ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
		ed.Add(Faces[n].v0,Faces[n].v1);
		ed.Add(Faces[n].v1,Faces[n].v2);
		ed.Add(Faces[n].v2,Faces[n].v0);
	}

    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nFaces*4);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n].Stok=Vertices[n].Stok+Vertices[n].Norm*f;
		duplicated->Vertices[n].Norm=Vertices[n].Norm;
		duplicated->Vertices[n].Map=Vertices[n].Map;
		duplicated->Vertices[n].Map2=Vertices[n].Map2;
		duplicated->Vertices[n].Diffuse=Vertices[n].Diffuse;
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[ed.a(n)].Norm + Vertices[ed.b(n)].Norm)/2;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;
	}


	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		m0=nVertices+ed.Witch(v0,v1);
		m1=nVertices+ed.Witch(v1,v2);
		m2=nVertices+ed.Witch(v2,v0);

		mpv0=Faces[n].mp0;
		mpv1=Faces[n].mp1;
		mpv2=Faces[n].mp2;

		mpm0=(mpv0+mpv1)/2;
		mpm1=(mpv1+mpv2)/2;
		mpm2=(mpv2+mpv0)/2;

		duplicated->Faces[nn].v0=v0;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m2;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv0;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm2;
		nn++;


		duplicated->Faces[nn].v0=m2;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpm2;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;


		duplicated->Faces[nn].v0=v1;
		duplicated->Faces[nn].v1=m1;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv1;
		duplicated->Faces[nn].mp1=mpm1;
		duplicated->Faces[nn].mp2=mpm0;
		nn++;

		duplicated->Faces[nn].v0=v2;
		duplicated->Faces[nn].v1=m2;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv2;
		duplicated->Faces[nn].mp1=mpm2;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;

	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		displacement of vertices

		void CObject3D::Displace(float d)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::Displace(float d)
{
	int n;
	CVector Nn;

	for (n=0;n<nVertices;n++)
	{
		Nn=Vertices[n].Norm;
		Nn.Normalise();
		Vertices[n].Stok+=d*Nn;
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		smoothing of object form du to normals (similar to RT patch)

		CObject3D CObject3D::TesselateInf(float Coef)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::TesselateInf(float Coef)
{
	return (*TesselateInf2(Coef));
}

CObject3D* CObject3D::TesselateInf2(float Coef)
{
	CVector u;
	CObject3D *obj;
	CObject3D *duplicated;
	int n,nn,k;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector Norm;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
	float f;
	int * corres;

	duplicated=new CObject3D;

	f=CalculateRadius();
	//CalculateVertexMasses();

	QuickEdges ed;

	corres=new int[nVertices];
	for (n=0;n<nVertices;n++)
	{
		corres[n]=n;
		Vertices[n].tag=1;

		for (nn=0;nn<n;nn++)
		{
			if (Vertices[nn].tag==1)
			{
				u=Vertices[n].Stok - Vertices[nn].Stok;
				if (u.Norme()<SMALLF2)
				{
					corres[n]=nn;
					Vertices[n].tag=0;
				}
			}
		}
	}

    ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
		ed.Add(Faces[n].v0,Faces[n].v1);
		ed.Add(Faces[n].v1,Faces[n].v2);
		ed.Add(Faces[n].v2,Faces[n].v0);
	}
    
    ed.Fix();

	duplicated->Init(nVertices+ed.Nb(),nFaces*4);

	for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];

	if (nKeys>0)
	{
		duplicated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			duplicated->VerticesKeys[k]=new CShortVertex[nVertices+ed.Nb()];
			for (n=0;n<nVertices;n++) duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
		}
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;

		u=Vertices[ed.a(n)].Stok - Vertices[ed.b(n)].Stok;
		f=u.Norme();

		duplicated->Vertices[nVertices+n].Norm=(Vertices[corres[ed.a(n)]].Norm + Vertices[corres[ed.b(n)]].Norm)/2;

		Norm=duplicated->Vertices[nVertices+n].Norm;
		Norm=f*Coef*Norm;

		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2 - Norm;

		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;

		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.a(n)].Index[1];
		duplicated->Vertices[nVertices+n].Index[2]=Vertices[ed.a(n)].Index[2];
		duplicated->Vertices[nVertices+n].Index[3]=Vertices[ed.a(n)].Index[3];

		duplicated->Vertices[nVertices+n].Weight[0]=Vertices[ed.a(n)].Weight[0];
		duplicated->Vertices[nVertices+n].Weight[1]=Vertices[ed.a(n)].Weight[1];
		duplicated->Vertices[nVertices+n].Weight[2]=Vertices[ed.a(n)].Weight[2];
		duplicated->Vertices[nVertices+n].Weight[3]=Vertices[ed.a(n)].Weight[3];
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			obj=new CObject3D;
			obj->Init(nVertices,nFaces);
			for (n=0;n<nVertices;n++)
			{
				obj->Vertices[n].Stok=VerticesKeys[k][n].Stok;
				obj->Vertices[n].Norm=VerticesKeys[k][n].Norm;
			}

			for (n=0;n<nFaces;n++) obj->Faces[n]=Faces[n];

			obj->SetFaces();

			for (n=0;n<ed.Nb();n++)
			{
				f=0.2f*(Vertices[ed.a(n)].coef1 + Vertices[ed.b(n)].coef1)/2;
				u=Vertices[ed.a(n)].Stok - Vertices[ed.b(n)].Stok;
				f=u.Norme();


				Norm=(obj->Vertices[corres[ed.a(n)]].Norm + obj->Vertices[corres[ed.b(n)]].Norm)/2;
				duplicated->VerticesKeys[k][nVertices+n].Norm=Norm;
				Norm=f*Coef*Norm;

				duplicated->VerticesKeys[k][nVertices+n].Stok=(obj->Vertices[ed.a(n)].Stok + obj->Vertices[ed.b(n)].Stok)/2 - Norm;
			}

			obj->Free();
			delete obj;
		}

	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		m0=nVertices+ed.Witch(v0,v1);
		m1=nVertices+ed.Witch(v1,v2);
		m2=nVertices+ed.Witch(v2,v0);

		mpv0=Faces[n].mp0;
		mpv1=Faces[n].mp1;
		mpv2=Faces[n].mp2;

		mpm0=(mpv0+mpv1)/2;
		mpm1=(mpv1+mpv2)/2;
		mpm2=(mpv2+mpv0)/2;

		duplicated->Faces[nn].v0=v0;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m2;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv0;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm2;
		nn++;


		duplicated->Faces[nn].v0=m2;
		duplicated->Faces[nn].v1=m0;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpm2;
		duplicated->Faces[nn].mp1=mpm0;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;


		duplicated->Faces[nn].v0=v1;
		duplicated->Faces[nn].v1=m1;
		duplicated->Faces[nn].v2=m0;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv1;
		duplicated->Faces[nn].mp1=mpm1;
		duplicated->Faces[nn].mp2=mpm0;
		nn++;

		duplicated->Faces[nn].v0=v2;
		duplicated->Faces[nn].v1=m2;
		duplicated->Faces[nn].v2=m1;
		duplicated->Faces[nn].Norm=Faces[n].Norm;
		duplicated->Faces[nn].tag=Faces[n].tag;
		duplicated->Faces[nn].nT=Faces[n].nT;
		duplicated->Faces[nn].nT2=Faces[n].nT2;
		duplicated->Faces[nn].ref=Faces[n].ref;

		//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[nn].mp0=mpv2;
		duplicated->Faces[nn].mp1=mpm2;
		duplicated->Faces[nn].mp2=mpm1;
		nn++;

	}

	duplicated->SetFaces();
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	delete [] corres;


	duplicated->Rot=Rot;
	duplicated->Coo=Coo;
	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;
	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];

	duplicated->nurbs=nurbs;
	duplicated->Status=Status;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;
	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;


	return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		smoothing of object form du to normals (similar to RT patch)

		CObject3D CObject3D::TesselateInf(float Coef)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D CObject3D::TesselateInfSelected(float Coef)
{
	return (*TesselateInfSelected2(Coef));
}

CObject3D* CObject3D::TesselateInfSelected2(float Coef)
{
	CVector u;
	CObject3D *obj;
	CObject3D *duplicated;
	int n,nn,k;
	int v0,v1,v2;
	int m0,m1,m2;
	CVector Norm;
	CVector2 mpv0,mpv1,mpv2,mpm0,mpm1,mpm2;
	float f;
	int * corres;

	duplicated=new CObject3D;

	f=CalculateRadius();
	//CalculateVertexMasses();

	QuickEdges ed;

	corres=new int[nVertices];
	for (n=0;n<nVertices;n++)
	{
		corres[n]=n;
		Vertices[n].tag=1;
		/*
		for (nn=0;nn<n;nn++)
		{
			if (Vertices[nn].tag==1)
			{
				u=Vertices[n].Stok - Vertices[nn].Stok;
				if (u.Norme()<SMALLF2)
				{
					corres[n]=nn;
					Vertices[n].tag=0;
				}
			}
		}
		/**/
	}

	ed.Init(nVertices);

	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			ed.Add(Faces[n].v0,Faces[n].v1);
			ed.Add(Faces[n].v1,Faces[n].v2);
			ed.Add(Faces[n].v2,Faces[n].v0);
		}
	}
    
    ed.Fix();

	int nf=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag) nf+=4;
		else nf++;
	}

	duplicated->Init(nVertices+ed.Nb(),nf);

	for (n=0;n<nVertices;n++) duplicated->Vertices[n]=Vertices[n];

	if (nKeys>0)
	{
		duplicated->nKeys=nKeys;
		for (k=0;k<nKeys;k++)
		{
			duplicated->VerticesKeys[k]=new CShortVertex[nVertices+ed.Nb()];
			for (n=0;n<nVertices;n++) duplicated->VerticesKeys[k][n]=VerticesKeys[k][n];
		}
	}

	for (n=0;n<ed.Nb();n++)
	{
		duplicated->Vertices[nVertices+n].tag=Vertices[ed.a(n)].tag;
		u=Vertices[ed.a(n)].Stok - Vertices[ed.b(n)].Stok;
		f=u.Norme();
		if (ed.tag(n)==0) f=0;
		duplicated->Vertices[nVertices+n].Norm=(Vertices[corres[ed.a(n)]].Norm + Vertices[corres[ed.b(n)]].Norm)/2;
		Norm=duplicated->Vertices[nVertices+n].Norm;
		Norm=f*Coef*Norm;
		duplicated->Vertices[nVertices+n].Stok=(Vertices[ed.a(n)].Stok + Vertices[ed.b(n)].Stok)/2 - Norm;
		duplicated->Vertices[nVertices+n].Map=(Vertices[ed.a(n)].Map+Vertices[ed.b(n)].Map)/2;
		//duplicated->Vertices[nVertices+n].Map2=(Vertices[ed.a(n)].Map2+Vertices[ed.b(n)].Map2)/2;
		//duplicated->Vertices[nVertices+n].Diffuse=(Vertices[ed.a(n)].Diffuse + Vertices[ed.b(n)].Diffuse)/2;
		duplicated->Vertices[nVertices+n].Index[0]=Vertices[ed.a(n)].Index[0];
		duplicated->Vertices[nVertices+n].Index[1]=Vertices[ed.a(n)].Index[1];
		duplicated->Vertices[nVertices+n].Index[2]=Vertices[ed.a(n)].Index[2];
		duplicated->Vertices[nVertices+n].Index[3]=Vertices[ed.a(n)].Index[3];
		duplicated->Vertices[nVertices+n].Weight[0]=Vertices[ed.a(n)].Weight[0];
		duplicated->Vertices[nVertices+n].Weight[1]=Vertices[ed.a(n)].Weight[1];
		duplicated->Vertices[nVertices+n].Weight[2]=Vertices[ed.a(n)].Weight[2];
		duplicated->Vertices[nVertices+n].Weight[3]=Vertices[ed.a(n)].Weight[3];
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			obj=new CObject3D;
			obj->Init(nVertices,nFaces);
			for (n=0;n<nVertices;n++)
			{
				obj->Vertices[n].Stok=VerticesKeys[k][n].Stok;
				obj->Vertices[n].Norm=VerticesKeys[k][n].Norm;
			}

			for (n=0;n<nFaces;n++) obj->Faces[n]=Faces[n];
			obj->SetFaces();

			for (n=0;n<ed.Nb();n++)
			{
				f=0.2f*(Vertices[ed.a(n)].coef1 + Vertices[ed.b(n)].coef1)/2;
				u=Vertices[ed.a(n)].Stok - Vertices[ed.b(n)].Stok;
				f=u.Norme();
				if (ed.tag(n)==0) f=0;
				Norm=(obj->Vertices[corres[ed.a(n)]].Norm + obj->Vertices[corres[ed.b(n)]].Norm)/2;
				duplicated->VerticesKeys[k][nVertices+n].Norm=Norm;
				Norm=f*Coef*Norm;
				duplicated->VerticesKeys[k][nVertices+n].Stok=(obj->Vertices[ed.a(n)].Stok + obj->Vertices[ed.b(n)].Stok)/2 - Norm;
			}

			obj->Free();
			delete obj;
		}

	}

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Faces[n].v0;
		v1=Faces[n].v1;
		v2=Faces[n].v2;

		if (Faces[n].tag)
		{
			m0=nVertices+ed.Witch(v0,v1);
			m1=nVertices+ed.Witch(v1,v2);
			m2=nVertices+ed.Witch(v2,v0);

			mpv0=Faces[n].mp0;
			mpv1=Faces[n].mp1;
			mpv2=Faces[n].mp2;

			mpm0=(mpv0+mpv1)/2;
			mpm1=(mpv1+mpv2)/2;
			mpm2=(mpv2+mpv0)/2;

			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm2;
			nn++;

			duplicated->Faces[nn].v0=m2;
			duplicated->Faces[nn].v1=m0;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpm2;
			duplicated->Faces[nn].mp1=mpm0;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;

			duplicated->Faces[nn].v0=v1;
			duplicated->Faces[nn].v1=m1;
			duplicated->Faces[nn].v2=m0;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv1;
			duplicated->Faces[nn].mp1=mpm1;
			duplicated->Faces[nn].mp2=mpm0;
			nn++;

			duplicated->Faces[nn].v0=v2;
			duplicated->Faces[nn].v1=m2;
			duplicated->Faces[nn].v2=m1;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;
			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv2;
			duplicated->Faces[nn].mp1=mpm2;
			duplicated->Faces[nn].mp2=mpm1;
			nn++;
		}
		else
		{
			duplicated->Faces[nn].v0=v0;
			duplicated->Faces[nn].v1=v1;
			duplicated->Faces[nn].v2=v2;
			duplicated->Faces[nn].Norm=Faces[n].Norm;
			duplicated->Faces[nn].tag=Faces[n].tag;
			duplicated->Faces[nn].nT=Faces[n].nT;
			duplicated->Faces[nn].nT2=Faces[n].nT2;
			duplicated->Faces[nn].ref=Faces[n].ref;

			//duplicated->Faces[nn].Diffuse=Faces[n].Diffuse;
			duplicated->Faces[nn].mp0=mpv0;
			duplicated->Faces[nn].mp1=mpv1;
			duplicated->Faces[nn].mp2=mpv2;
			nn++;
		}
	}

	duplicated->SetFaces();

	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag=0;
	for (n=0;n<duplicated->nFaces;n++) duplicated->Faces[n].tag2=0;

	ed.Free();

	delete [] corres;


	duplicated->Rot=Rot;
	duplicated->Coo=Coo;
	for (n=0;n<6;n++) duplicated->P[n]=P[n];
	duplicated->nP=nP;
	for (n=0;n<16;n++) duplicated->Tab[n]=Tab[n];

	duplicated->nurbs=nurbs;
	duplicated->Status=Status;
	duplicated->Status2=Status2;
	duplicated->Attribut=Attribut;
	duplicated->Tag=Tag;
	duplicated->env_mapping=env_mapping;
	duplicated->group=group;


	return duplicated;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		set tag to an object accordingly to Filtre

		int CObject3D::SetTag(unsigned int Filtre)

		SET_FACES_TAG
		SET_REF
		SET_FACES_TAG2
		SET_VERTICES_TAG
		SET_FACES_COPLANAR_RECURS_TAG

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::SetTag(unsigned int Filtre)
{
	unsigned int filtre;
	int tag;
	int n,nn;
	bool test;
	int res;


	res=0;

	tag=Filtre&0x00FFFFFF;
	filtre=Filtre&0xFF000000;

	if (filtre&SET_FACES_TAG)
	{
		for (n=0;n<nFaces;n++)
			Faces[n].tag=tag;
	}

	if (filtre&SET_REF)
	{
		for (n=0;n<nFaces;n++)
			Faces[n].ref=tag&0xFF;
	}

	if (filtre&SET_FACES_TAG2)
	{
		for (n=0;n<nFaces;n++)
			Faces[n].tag2=tag;
	}

	if (filtre&SET_VERTICES_TAG)
	{
		for (n=0;n<nVertices;n++)
			Vertices[n].tag=tag;
	}


	if (filtre&SET_FACES_COPLANAR_RECURS_TAG)
	{
		for (n=0;n<nFaces;n++) Faces[n].tag=0;

		tag=1;
		test=true;
		while (test)
		{
			n=0;
			nn=-1;
			while ((n<nFaces)&&(nn==-1))
			{
				if (Faces[n].tag==0) nn=n;
				else
					n++;
			}

			if (nn==-1) test=false;
			else
			{
				TagCoplanarFaces(nn,nn,tag);
				Faces[nn].tag=tag;
				tag++;
				test=true;
			}
		}

		res=tag;
	}

	if (filtre&SET_FACES_COPLANAR_RECURS_TAG2)
	{
		for (n=0;n<nFaces;n++) Faces[n].tag=0;

		tag=1;
		test=true;
		while (test)
		{
			n=0;
			nn=-1;
			while ((n<nFaces)&&(nn==-1))
			{
				if (Faces[n].tag==0) nn=n;
				else
					n++;
			}

			if (nn==-1) test=false;
			else
			{
				TagCoplanarOnlyFaces(nn,tag);
				Faces[nn].tag=tag;
				tag++;
				test=true;
			}
		}

		res=tag;
	}

	return res;
}




/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		see previious def

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool ABClose(CVector &a,CVector &b)
{
	CVector u;
	VECTORSUB(u,a,b);
	if (VECTORNORM(u)<SMALLF2) return true;
	return false;
}


int CObject3D::FaceClose_01(int nf)
{
	int n,res;
	CFace * f;
	f=&Faces[nf];

	n=0;
	res=-1;
	while ((n<nFaces)&&(res==-1))
	{
		if (n!=nf)
		{
			if ((ABClose(Faces[n].v[0]->Stok,f->v[1]->Stok))&&(ABClose(Faces[n].v[1]->Stok,f->v[0]->Stok))) res=n;
			if ((ABClose(Faces[n].v[1]->Stok,f->v[1]->Stok))&&(ABClose(Faces[n].v[2]->Stok,f->v[0]->Stok))) res=n;
			if ((ABClose(Faces[n].v[2]->Stok,f->v[1]->Stok))&&(ABClose(Faces[n].v[0]->Stok,f->v[0]->Stok))) res=n;
		}
		n++;
	}
	return res;
}

int CObject3D::FaceClose_12(int nf)
{
	int n,res;
	CFace * f;
	f=&Faces[nf];

	n=0;
	res=-1;
	while ((n<nFaces)&&(res==-1))
	{
		if (n!=nf)
		{
			if ((ABClose(Faces[n].v[0]->Stok,f->v[2]->Stok))&&(ABClose(Faces[n].v[1]->Stok,f->v[1]->Stok))) res=n;
			if ((ABClose(Faces[n].v[1]->Stok,f->v[2]->Stok))&&(ABClose(Faces[n].v[2]->Stok,f->v[1]->Stok))) res=n;
			if ((ABClose(Faces[n].v[2]->Stok,f->v[2]->Stok))&&(ABClose(Faces[n].v[0]->Stok,f->v[1]->Stok))) res=n;
		}
		n++;
	}
	return res;
}

int CObject3D::FaceClose_20(int nf)
{
	int n,res;
	CFace * f;
	f=&Faces[nf];

	n=0;
	res=-1;
	while ((n<nFaces)&&(res==-1))
	{
		if (n!=nf)
		{
			if ((ABClose(Faces[n].v[0]->Stok,f->v[0]->Stok))&&(ABClose(Faces[n].v[1]->Stok,f->v[2]->Stok))) res=n;
			if ((ABClose(Faces[n].v[1]->Stok,f->v[0]->Stok))&&(ABClose(Faces[n].v[2]->Stok,f->v[2]->Stok))) res=n;
			if ((ABClose(Faces[n].v[2]->Stok,f->v[0]->Stok))&&(ABClose(Faces[n].v[0]->Stok,f->v[2]->Stok))) res=n;
		}
		n++;
	}
	return res;
}




int CObject3D::Face_01(int nf)
{
	int n;
	CFace * f=&Faces[nf];

	n=0;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n;
			if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n;
			if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n;
		}
		n++;
	}
	return -1;
}

int CObject3D::Face_12(int nf)
{
	int n;
	CFace * f=&Faces[nf];
	n=0;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n;
			if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n;
			if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n;
		}
		n++;
	}
	return -1;
}

int CObject3D::Face_20(int nf)
{
	int n;
	CFace * f=&Faces[nf];
	n=0;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n;
			if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n;
			if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n;
		}
		n++;
	}
	return -1;
}

/*
int CObject3D::Face_01Q(int nf,int *references_face_012Q)
{
	int n;
	CFace * f=&Faces[nf];
	int tag=f->i0;

	int minf=references_face_012Q[tag];
	int maxf=references_face_012Q[nVertices*2+tag];

	for (n=minf;n<=maxf;n++)
	{
		if (tag==Faces[n].i0) { if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n; }
		else if (tag==Faces[n].i1) { if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n; }
		else if (tag==Faces[n].i2) { if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n; }
	}

	return -1;
}

int CObject3D::Face_12Q(int nf,int *references_face_012Q)
{
	int n;
	CFace * f=&Faces[nf];
	int tag=f->i1;
	
	int minf=references_face_012Q[tag];
	int maxf=references_face_012Q[nVertices*2+tag];

	for (n=minf;n<=maxf;n++)
	{
		if (tag==Faces[n].i0) { if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n; }
		else if (tag==Faces[n].i1) { if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n; }
		else if (tag==Faces[n].i2) { if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n; }
	}

	return -1;
}

int CObject3D::Face_20Q(int nf,int *references_face_012Q)
{
	int n;
	CFace * f=&Faces[nf];
	int tag=f->i2;
	
	int minf=references_face_012Q[tag];
	int maxf=references_face_012Q[nVertices*2+tag];

	for (n=minf;n<=maxf;n++)
	{
		if (tag==Faces[n].i0) { if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n; }
		else if (tag==Faces[n].i1) { if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n; }
		else if (tag==Faces[n].i2) { if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n; }
	}

	return -1;
}
/**/

int CObject3D::Face_01QZ(int nf,int minf,int maxf)
{
	int n;
	CFace * f=&Faces[nf];
	CVector u1,u2;

	for (n=nf-1;n>=minf;n--)
	{
		if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n;
		if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n;
		if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	
		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	for (n=nf+1;n<maxf;n++)
	{
		if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n;
		if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n;
		if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	
		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[1]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[0]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	return -1;
}

int CObject3D::Face_12QZ(int nf,int minf,int maxf)
{
	int n;
	CFace * f=&Faces[nf];
	CVector u1,u2;

	for (n=nf-1;n>=minf;n--)
	{	
		if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n;
		if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n;
		if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
		
		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
			
		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	for (n=nf+1;n<maxf;n++)
	{	
		if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n;
		if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n;
		if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
		
		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
			
		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[2]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[1]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	return -1;
}

int CObject3D::Face_20QZ(int nf,int minf,int maxf)
{
	int n;
	CFace * f=&Faces[nf];
	CVector u1,u2;
	
	for (n=nf-1;n>=minf;n--)
	{
		if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n;
		if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n;
		if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[2]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[2]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[2]->Stok);		
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	for (n=nf+1;n<maxf;n++)
	{
		if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n;
		if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n;
		if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n;

		VECTORSUB(u1,Faces[n].v[0]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[1]->Stok,f->v[2]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[1]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[2]->Stok,f->v[2]->Stok);
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;

		VECTORSUB(u1,Faces[n].v[2]->Stok,f->v[0]->Stok);
		VECTORSUB(u2,Faces[n].v[0]->Stok,f->v[2]->Stok);		
		if ((VECTORNORM2(u1)<SMALLF)&&(VECTORNORM2(u2)<SMALLF)) return n;
	}

	return -1;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

	copy figure:
		void CFigure::Copy(CFigure *figure)

	add two figures:
		void CFigure::Add(int position,CFigure *figure,int index)

	optimization of the figure shape:
		void CFigure::OptimizeFrom(CVertex * vertices)

	determining if figure define volume or hole:
		bool CFigure::Sens(int ss,CVertex *vertices,CVector N)

	retreiving convex envelop of a figure:
		void CFigure::ConvexHull(CVertex *vertices,CVector N)


	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CFigure::Disjoint(CVertex *vertices,CVector N,CFigure *fig)
{
	CBSP2D bsp;
	int *i;
	CVector v;
	bool res=true;
/*
	bsp.Build(fig,vertices,N);
	i=List.GetFirst();
	while (i)
	{
		v=vertices[*i].Stok;
		if (bsp.Inside(bsp.root,v)) res=false;
		i=List.GetNext();
	}
	bsp.Free();
/**/
	bsp.Build(this,vertices,N);
	i=fig->List.GetFirst();
	while (i)
	{
		v=vertices[*i].Stok;
		if (bsp.Inside(bsp.root,v)) res=false;
		i=fig->List.GetNext();
	}
	bsp.Free();

	return res;
}


bool CFigure::Sens(int ss,CVertex *vertices,CVector N)
{
	bool res;
	int *i0,*i1,*i2;
	CFigure fig;
	CVector u1,u2,N0;

	fig.Copy(this);
	fig.ConvexHull(vertices,N);

	i0=fig.List[(fig.List.Length()-1)%fig.List.Length()];
	i1=fig.List[0];
	i2=fig.List[1];

	u1=vertices[*i1].Stok - vertices[*i0].Stok;
	u2=vertices[*i2].Stok - vertices[*i1].Stok;

	N0=u1^u2;
	res=(((N||N0)*ss)>0);
	fig.List.Free();
	return res;
}

void CFigure::ConvexHull(CVertex *vertices,CVector N)
{
	int *i1,*i2;
	int n;
	bool tmp;

	tmp=true;
	n=0;
	while (tmp)
	{
		i1=List[n];
		i2=List[(n+1)%List.Length()];

		if (TRIANGULARISATION_PlanSecant2(vertices,this,N,*i1,*i2)) List.Del(n);
		else n++;

		if (n<List.Length()) tmp=true;
		else tmp=false;
	}

}

void CFigure::Copy(CFigure *figure)
{
	int *i;
	List.Free();

	nT=figure->nT;

	i=figure->List.GetFirst();
	while (i)
	{
		List.Add(*i);
		i=figure->List.GetNext();
	}
}

void CFigure::InvCopy(CFigure *figure)
{
	int *i;
	List.Free();

	i=figure->List.GetFirst();
	while (i)
	{
		List.InsertFirst(*i);
		i=figure->List.GetNext();
	}
}

void CFigure::Add(int position,CFigure *figure,int index)
{
	int n,i;

	i=*(List[position]);
	List.SetCurrent(position);
	for (n=index;n<figure->List.Length();n++) List.InsertCurrent(*(figure->List[n]));
	for (n=0;n<=index;n++) List.InsertCurrent(*(figure->List[n]));
	List.InsertCurrent(i);
}

void CFigure::OptimizeFrom(CVertex * vertices)
{
	int i0,i1,i2;
	int n;
	CVector u1,u2;
//	float scale;
	float n1,n2,s;
	float SMALLFK=SMALLF*COEF_MUL_OPTIMIZEFROM_LOCAL;
	float SMALLFKSC=SMALLF*COEF_MUL_OPTIMIZEFROM;

	if (List.Length()<3) return;

	n=0;
	while (n<List.Length())
	{
		CElement <int>*el=List(n);
		if (el->prev) i0=el->prev->data; else i0=List.Closing->data;
		i1=el->data;
		if (el->next) i2=el->next->data; else i2=List.First->data;
		
		VECTORSUB(u2,vertices[i2].Stok,vertices[i1].Stok);		
		n2=VECTORNORM(u2);

		if (n2<SMALLFK) List.DelEl(el);
		else
		{
			VECTORSUB(u1,vertices[i0].Stok,vertices[i1].Stok);
			n1=VECTORNORM(u1);
			if (n1>SMALLFK)
			{
				//scale=1.0f/(n1+n2);
				//if (scale>1.0f) scale=1.0f;
				VECTORDIV(u1,u1,n1);
				VECTORDIV(u2,u2,n2);
				DOTPRODUCT(s,u1,u2);
				if (s<0) s=-s;
				//if (s>1.0f-(SMALLF*scale)) List.DelEl(el);
				if (s>(1.0f-SMALLFKSC)) List.DelEl(el);
				else n++;
			}
			else n++;
		}
	}

}

void CFigure::OptimizeFromPreserve(CVertex * vertices)
{
    int i0,i1,i2;
    int n;
    CVector u1,u2;
    //    float scale;
    float n1,n2,s;
    float SMALLFK=SMALLF*COEF_MUL_OPTIMIZEFROM_LOCAL;
    float SMALLFKSC=SMALLF*COEF_MUL_OPTIMIZEFROM;
    
    if (List.Length()<3) return;
    
    n=0;
    while (n<List.Length())
    {
        CElement <int>*el=List(n);
        if (el->prev) i0=el->prev->data; else i0=List.Closing->data;
        i1=el->data;
        if (el->next) i2=el->next->data; else i2=List.First->data;
        
        int tag=0;
        if ((vertices[i0].tag2==0)&&(vertices[i1].tag2==0)&&(vertices[i2].tag2==0)) tag=1;
        if ((vertices[i0].tag2==1)&&(vertices[i1].tag2==1)&&(vertices[i2].tag2==1)) tag=1;
        
        if (tag)
        {
            VECTORSUB(u2,vertices[i2].Stok,vertices[i1].Stok);
            n2=VECTORNORM(u2);
            
            if (n2<SMALLFK) List.DelEl(el);
            else
            {
                VECTORSUB(u1,vertices[i0].Stok,vertices[i1].Stok);
                n1=VECTORNORM(u1);
                if (n1>SMALLFK)
                {
                    //scale=1.0f/(n1+n2);
                    //if (scale>1.0f) scale=1.0f;
                    VECTORDIV(u1,u1,n1);
                    VECTORDIV(u2,u2,n2);
                    DOTPRODUCT(s,u1,u2);
                    if (s<0) s=-s;
                    //if (s>1.0f-(SMALLF*scale)) List.DelEl(el);
                    if (s>(1.0f-SMALLFKSC)) List.DelEl(el);
                    else n++;
                }
                else n++;
            }
        }
        else n++;
    }
    
}

void CFigure::preOptimizeFrom(CVertex * vertices)
{
	int *i0,*i1,*i2;
	int n;
	CVector u,u1,u2;
	bool tmp;
	float scale,s;

	if (List.Length()==0) return;

	tmp=true;
	n=0;
	while ((tmp)&&(List.Length()!=0))
	{
		i0=List[(List.Length()+n-1)%List.Length()];
		i1=List[n];
		i2=List[(n+1)%List.Length()];

		VECTORSUB(u1,vertices[*i0].Stok,vertices[*i1].Stok);
		VECTORSUB(u2,vertices[*i2].Stok,vertices[*i1].Stok);

		if (VECTORNORM2(u2)<SMALLFXXK*SMALLFXXK) List.Del(n);
		else n++;

		if (n<List.Length()) tmp=true;
		else tmp=false;
	}

	if (List.Length()==0) return;

	tmp=true;
	n=0;
	while ((tmp)&&(List.Length()!=0))
	{
		i0=List[(List.Length()+n-1)%List.Length()];
		i1=List[n];
		i2=List[(n+1)%List.Length()];

		VECTORSUB(u1,vertices[*i0].Stok,vertices[*i1].Stok);
		VECTORSUB(u2,vertices[*i2].Stok,vertices[*i1].Stok);

		float f1=VECTORNORM(u1);
		float f2=VECTORNORM(u2);

		scale=1.0f/(f1+f2);
		if (scale>1.0f) scale=1.0f;

		if (f2<SMALLFXXK) List.Del(n);
		else
		{
			VECTORDIV(u1,u1,f1)
			VECTORDIV(u2,u2,f2)
			s=DOT(u1,u2);
			if (s<0) s=-s;
			if (s>1.0f-(SMALLF*scale)) List.Del(n);
			else n++;
		}

		if (n<List.Length()) tmp=true;
		else tmp=false;
	}

}


void CFigure::OptimizeHugeFrom(CVertex * vertices)
{
	int *i0,*i1,*i2;
	int n;
	CVector u0,u1,u2;
	bool tmp;
	float scale;

	if (List.Length()==0) return;

	tmp=true;
	n=0;
	while (tmp)
	{
		i0=List[(List.Length()+n-1)%List.Length()];
		i1=List[n];
		i2=List[(n+1)%List.Length()];

		u1=vertices[*i0].Stok - vertices[*i1].Stok;
		u2=vertices[*i2].Stok - vertices[*i1].Stok;
		u0=vertices[*i0].Stok - vertices[*i2].Stok;

		scale=1.0f/(u1.Norme()+u2.Norme());

		if (u2.Norme()<SMALLF2) List.Del(n);
		else
		{
			u1.Normalise();
			u2.Normalise();
			if ((u1||u2)>1.0f-SMALLF3*scale) List.Del(n);
			else
			if (((u1||u2)<-1.0f+SMALLF3*scale)&&(u0.Norme()<SMALLF2)) List.Del(n);
			else n++;
		}

		if (n<List.Length()) tmp=true;
		else tmp=false;
	}

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		optimize geometry

		CObject3D * CObject3D::OptimizeMeshBooleanTag(int THISTAG)

	Usage:

		merge coplanar face in figures and retriangularisation
		.nT not implemented ?
		.tag define smoothing groups

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::OptimizeMeshBooleanTag(int THISTAG,bool map2)
{
#ifdef TRIANGULARISATION
	CVector normal;
	CObject3D *res;
	int n,nn,nnn,n0,tag,nb;
	bool test;
	_EdgeListD Edges;
	CList <CFigure> Figures;
	CFigure tmpfig;
	CFigure *f,tmpf;
	CVertex* vertices;
	CShortFace* faces;
	CShortFace *ff;
	CVector u;
	int na;
	int nv,nf;
	int thistag;
	bool b=true;
	int ed;
	int array_tmp[8192];
	int narray_tmp=0;
	CVector u_prev,u_next;
	CVector v_prev,v_next;
	float mini=2*PI;
	int found=0;
	int nd=0;
	float x,y;
	float ang=0.0f;

	Figures.Free();
    
    SetF012();    
    for (n=0;n<nFaces;n++) Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
    
    /*
	int ofs2=nVertices*2;
	
	CShortyList ** references_face_012Q=new CShortyList*[ofs2];

	for (n=0;n<ofs2;n++) references_face_012Q[n]=NULL;

	int sz=sizeof(CShortyList);
	int szel=sizeof(CSElement);

	newbieInit(3*nFaces*(sz+szel));

	for (n=0;n<nFaces;n++)
	{		
		Faces[n].i0=Faces[n].v0+Faces[n].v1;
		Faces[n].i1=Faces[n].v1+Faces[n].v2;
		Faces[n].i2=Faces[n].v2+Faces[n].v0;

		if (references_face_012Q[Faces[n].i0]==NULL) { references_face_012Q[Faces[n].i0]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i0]->Closing=NULL; references_face_012Q[Faces[n].i0]->obj=this; }
		if (references_face_012Q[Faces[n].i1]==NULL) { references_face_012Q[Faces[n].i1]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i1]->Closing=NULL; references_face_012Q[Faces[n].i1]->obj=this; }
		if (references_face_012Q[Faces[n].i2]==NULL) { references_face_012Q[Faces[n].i2]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i2]->Closing=NULL; references_face_012Q[Faces[n].i2]->obj=this; }
		
		references_face_012Q[Faces[n].i0]->Add(n);
		references_face_012Q[Faces[n].i1]->Add(n);
		references_face_012Q[Faces[n].i2]->Add(n);
	}
	
	for (n=0;n<nFaces;n++)
	{
		Faces[n].f01=Face_01QW(n,references_face_012Q);
		Faces[n].f12=Face_12QW(n,references_face_012Q);
		Faces[n].f20=Face_20QW(n,references_face_012Q);

		Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
	}

	newbieKill();
	delete [] references_face_012Q;
    /**/

	faces=new CShortFace[nFaces*16];
	vertices=new CVertex[nVertices*16];
	nv=0;
	nf=0;
	for (thistag=0;thistag<THISTAG;thistag++)
	{
		for (n=0;n<nFaces;n++) Faces[n].tag2=0;

		int basenn=0;

		tag=1;
		test=true;
		while (test)
		{
			n=basenn;
			nn=-1;
			while (n<nFaces)
			{
				if (Faces[n].tag2==0)
				{
					if ((Faces[n].tag==thistag)&&(Faces[n].coef1>0.25f)) { nn=n; break; }
					else n++;
				}
				else n++;
			}

			if (nn==-1) test=false;
			else
			{
				basenn=nnn=nn;
				minnf=maxnf=nn;
				TagObjetCoplanarFacesTag(nn,nn,-1,thistag);

				Faces[nn].tag2=-1;

				nb=0;
				for (n=minnf;n<=maxnf;n++)
					if (Faces[n].tag2<0)
					{
						nb+=3;
						if (Faces[n].coef1>0.25f) normal=Faces[n].Norm;
					}

				Edges.Init(nb);

				for (n=minnf;n<=maxnf;n++)
				{
					if (Faces[n].tag2<0)
					{
						Edges.AddEdge(Faces[n].v0,Faces[n].v1);
						Edges.AddEdge(Faces[n].v1,Faces[n].v2);
						Edges.AddEdge(Faces[n].v2,Faces[n].v0);
					}
				}

				test=true;

				while (test)
				{
					n=0;
					nn=-1;
					while (n<Edges.nList)
					{
						if (Edges.List[n].tag==0) { nn=n; break; }
						n++;
					}

					if (nn==-1) test=false;
					else
					{
						tmpfig.List.Free();
						f=&tmpfig;
						n0=nn;
						
						f->List.Add(Edges.List[n0].a);
						Edges.List[n0].tag=1;
						n=Edges.Suivant(Edges.List[n0].b);
						Edges.List[n0].tag=0;

						while ((n!=n0)&&(n!=-1))
						{
							if (n!=-1)
							{
								Edges.List[n].tag=1;
								f->List.Add(Edges.List[n].a);

								narray_tmp=0;

								ed=Edges.Suivant(Edges.List[n].b);
								while (ed!=-1)
								{
									Edges.List[ed].tag=1;
									array_tmp[narray_tmp]=ed;
									narray_tmp++;

									ed=Edges.Suivant(Edges.List[n].b);
								}

								if (narray_tmp==0) n=-1;
								else
								{
									for (ed=0;ed<narray_tmp;ed++) Edges.List[array_tmp[ed]].tag=0;

									if (narray_tmp==1) n=array_tmp[0];
									else
									{
										mini=2*PI;
										found=0;

										VECTORSUB(u_prev,Vertices[Edges.List[n].a].Stok,Vertices[Edges.List[n].b].Stok);
										VECTORNORMALISE(u_prev);
										CROSSPRODUCT(v_prev,u_prev,normal);
										VECTORNORMALISE(v_prev);

										for (ed=0;ed<narray_tmp;ed++)
										{
											nd=array_tmp[ed];
											VECTORSUB(u_next,Vertices[Edges.List[nd].b].Stok,Vertices[Edges.List[nd].a].Stok);
											VECTORNORMALISE(u_next);
											CROSSPRODUCT(v_next,u_next,normal);
											VECTORNORMALISE(v_next);

											x=DOT(u_prev,u_next);
											y=DOT(v_prev,v_next);
											ang=0.0f;

											if (x<-SMALLF) ang=PI - (float) atanf(-y/x);
											else
											if (x>SMALLF)
											{
												ang=(float) atanf(y/x);
												if (ang<0)
												{
													ang=2*PI - ang;
												}
											}
											else
											{
												if (y>0) ang=PI/2;
												else ang=3*PI/2;
											}

											if (ang<mini)
											{
												mini=ang;
												found=ed;
											}
										}

										n=array_tmp[found];
									}
								}
							}
						}

						Edges.List[n0].tag=1;

						if (n!=-1)
						{
							Figures.InsertLast()->Copy(f);
						}

					}
				}

				tmpfig.List.Free();

				f=Figures.GetFirst();
				while (f)
				{
					f->OptimizeFrom(Vertices);

					if (f->List.Length()>2) f=Figures.GetNext();
					else
					{
						f->List.Free();
						f=Figures.DeleteAndGetNext();
					}
				}

				if (Figures.Length()>0)
				{
					na=nf;
					if (!Triangularisation2(Vertices,&Figures,normal,vertices,faces,&nv,&nf,Faces[nnn].nT,Faces[nnn].nT2,Faces[nnn].nL,Faces[nnn].nLVectors,Faces[nnn].ref,thistag)) b=false;
				}

				f=Figures.GetFirst();
				while (f)
				{
					f->List.Free();
					f=Figures.GetNext();
				}

				Figures.Free();
					
				Edges.Free();

				for (n=minnf;n<=maxnf;n++)
					if (Faces[n].tag2<0) Faces[n].tag2=1;

				test=true;
				tag++;
			}
		}
	}

	if (b)
	{
		res=new CObject3D;
		res->Init(nv,nf);

		for (n=0;n<nv;n++)
		{
			res->Vertices[n].Stok=vertices[n].Stok;
			res->Vertices[n].Calc=vertices[n].Calc;
			res->Vertices[n].Map=vertices[n].Map;
            if (map2) res->Vertices[n].Map2=vertices[n].Map2;
            
            res->Vertices[n].tag=0;
            res->Vertices[n].tag2=0;
            
            if (!map2)
            {
                res->Vertices[n].Index[0]=vertices[n].Index[0];
                res->Vertices[n].Index[1]=vertices[n].Index[1];
                res->Vertices[n].Index[2]=vertices[n].Index[2];
                res->Vertices[n].Index[3]=vertices[n].Index[3];
                
                res->Vertices[n].Weight[0]=vertices[n].Weight[0];
                res->Vertices[n].Weight[1]=vertices[n].Weight[1];
                res->Vertices[n].Weight[2]=vertices[n].Weight[2];
                res->Vertices[n].Weight[3]=vertices[n].Weight[3];
            }
		}

		for (n=0;n<nf;n++)
		{
			ff=&faces[n];
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nT2=ff->nT2;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].tag2=0;
			res->Faces[n].nLVectors=ff->nLVectors;
			res->Faces[n].ref=ff->ref;
		}

		delete [] vertices;
		delete [] faces;
	
		res->Rot=Rot;
		res->Coo=Coo;
		for (n=0;n<6;n++) res->P[n]=P[n];
		res->nP=nP;
		for (n=0;n<16;n++) res->Tab[n]=Tab[n];

		res->Status=Status;
		res->Attribut=Attribut;
		res->env_mapping=env_mapping;
		res->Tag=Tag;
		res->nurbs=nurbs;
		res->SetFaces();

		res->CalculateNormals(-1);

		res->env_mapping=env_mapping;
		res->group=group;

		return res;
	}
	else
	{
		delete [] vertices;
		delete [] faces;

		return NULL;
	}
#else

	return this->Duplicate2();

#endif
}
/**/

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 optimize geometry
 
 CObject3D * CObject3D::OptimizeMeshBooleanTag(int THISTAG)
 
 Usage:
 
 merge coplanar face in figures and retriangularisation
 .nT not implemented ?
 .tag define smoothing groups
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

extern int OPTIMIZE_FORDEGEN;

CObject3D * CObject3D::OptimizeMeshBooleanPreserveTag2()
{
#ifdef TRIANGULARISATION
    CVector normal;
    CObject3D *res;
    int n,nn,nnn,n0,tag,nb;
    bool test;
    _EdgeListD Edges;
    CList <CFigure> Figures;
    CFigure tmpfig;
    CFigure *f,tmpf;
    CVertex* vertices;
    CShortFace* faces;
    CShortFace *ff;
    CVector u;
    int na;
    int nv,nf;
    bool b=true;
    int ed;
    int array_tmp[8192];
    int narray_tmp=0;
    CVector u_prev,u_next;
    CVector v_prev,v_next;
    float mini=2*PI;
    int found=0;
    int nd=0;
    float x,y;
    float ang=0.0f;
    
    Figures.Free();
    
    SetF012();
    for (n=0;n<nFaces;n++) Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
    
    /*
    int ofs2=nVertices*2;
    
    CShortyList ** references_face_012Q=new CShortyList*[ofs2];
    
    for (n=0;n<ofs2;n++) references_face_012Q[n]=NULL;
    
    int sz=sizeof(CShortyList);
    int szel=sizeof(CSElement);
    
    newbieInit(3*nFaces*(sz+szel));
    
    for (n=0;n<nFaces;n++)
    {
        Faces[n].i0=Faces[n].v0+Faces[n].v1;
        Faces[n].i1=Faces[n].v1+Faces[n].v2;
        Faces[n].i2=Faces[n].v2+Faces[n].v0;
        
        if (references_face_012Q[Faces[n].i0]==NULL) { references_face_012Q[Faces[n].i0]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i0]->Closing=NULL; references_face_012Q[Faces[n].i0]->obj=this; }
        if (references_face_012Q[Faces[n].i1]==NULL) { references_face_012Q[Faces[n].i1]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i1]->Closing=NULL; references_face_012Q[Faces[n].i1]->obj=this; }
        if (references_face_012Q[Faces[n].i2]==NULL) { references_face_012Q[Faces[n].i2]=(CShortyList*)newbie(sizeof(CShortyList)); references_face_012Q[Faces[n].i2]->Closing=NULL; references_face_012Q[Faces[n].i2]->obj=this; }
        
        references_face_012Q[Faces[n].i0]->Add(n);
        references_face_012Q[Faces[n].i1]->Add(n);
        references_face_012Q[Faces[n].i2]->Add(n);
    }
    
    for (n=0;n<nFaces;n++)
    {
        Faces[n].f01=Face_01QW(n,references_face_012Q);
        Faces[n].f12=Face_12QW(n,references_face_012Q);
        Faces[n].f20=Face_20QW(n,references_face_012Q);
        
        Faces[n].coef1=VECTORNORM2(Faces[n].Norm);
    }
    
    newbieKill();
    delete [] references_face_012Q;
    /**/
    
    faces=new CShortFace[nFaces*16];
    vertices=new CVertex[nVertices*16];
    nv=0;
    nf=0;
    for (n=0;n<nFaces;n++) Faces[n].tag2=0;
    
    int basenn=0;
    
    tag=1;
    test=true;
    while (test)
    {
        n=basenn;
        nn=-1;
        while (n<nFaces)
        {
            if (Faces[n].tag2==0)
            {
                if ((Faces[n].tag==0)&&(Faces[n].coef1>0.25f)) { nn=n; break; }
                else n++;
            }
            else n++;
        }
        
        if (nn==-1) test=false;
        else
        {
            basenn=nnn=nn;
            minnf=maxnf=nn;
            TagObjetCoplanarFacesTag(nn,nn,-1,0);
            
            Faces[nn].tag2=-1;
            
            nb=0;
            for (n=minnf;n<=maxnf;n++)
                if (Faces[n].tag2<0)
                {
                    nb+=3;
                    if (Faces[n].coef1>0.25f) normal=Faces[n].Norm;
                }
            
            Edges.Init(nb);
            
            for (n=minnf;n<=maxnf;n++)
            {
                if (Faces[n].tag2<0)
                {
                    Edges.AddEdge(Faces[n].v0,Faces[n].v1);
                    Edges.AddEdge(Faces[n].v1,Faces[n].v2);
                    Edges.AddEdge(Faces[n].v2,Faces[n].v0);
                }
            }
            
            test=true;
            
            while (test)
            {
                n=0;
                nn=-1;
                while (n<Edges.nList)
                {
                    if (Edges.List[n].tag==0) { nn=n; break; }
                    n++;
                }
                
                if (nn==-1) test=false;
                else
                {
                    tmpfig.List.Free();
                    f=&tmpfig;
                    n0=nn;
                    
                    f->List.Add(Edges.List[n0].a);
                    Edges.List[n0].tag=1;
                    n=Edges.Suivant(Edges.List[n0].b);
                    Edges.List[n0].tag=0;
                    
                    while ((n!=n0)&&(n!=-1))
                    {
                        if (n!=-1)
                        {
                            Edges.List[n].tag=1;
                            f->List.Add(Edges.List[n].a);
                            
                            narray_tmp=0;
                            
                            ed=Edges.Suivant(Edges.List[n].b);
                            while (ed!=-1)
                            {
                                Edges.List[ed].tag=1;
                                array_tmp[narray_tmp]=ed;
                                narray_tmp++;
                                
                                ed=Edges.Suivant(Edges.List[n].b);
                            }
                            
                            if (narray_tmp==0) n=-1;
                            else
                            {
                                for (ed=0;ed<narray_tmp;ed++) Edges.List[array_tmp[ed]].tag=0;
                                
                                if (narray_tmp==1) n=array_tmp[0];
                                else
                                {
                                    mini=2*PI;
                                    found=0;
                                    
                                    VECTORSUB(u_prev,Vertices[Edges.List[n].a].Stok,Vertices[Edges.List[n].b].Stok);
                                    VECTORNORMALISE(u_prev);
                                    CROSSPRODUCT(v_prev,u_prev,normal);
                                    VECTORNORMALISE(v_prev);
                                    
                                    for (ed=0;ed<narray_tmp;ed++)
                                    {
                                        nd=array_tmp[ed];
                                        VECTORSUB(u_next,Vertices[Edges.List[nd].b].Stok,Vertices[Edges.List[nd].a].Stok);
                                        VECTORNORMALISE(u_next);
                                        CROSSPRODUCT(v_next,u_next,normal);
                                        VECTORNORMALISE(v_next);
                                        
                                        x=DOT(u_prev,u_next);
                                        y=DOT(v_prev,v_next);
                                        ang=0.0f;
                                        
                                        if (x<-SMALLF) ang=PI - (float) atanf(-y/x);
                                        else
                                            if (x>SMALLF)
                                            {
                                                ang=(float) atanf(y/x);
                                                if (ang<0)
                                                {
                                                    ang=2*PI - ang;
                                                }
                                            }
                                            else
                                            {
                                                if (y>0) ang=PI/2;
                                                else ang=3*PI/2;
                                            }
                                        
                                        if (ang<mini)
                                        {
                                            mini=ang;
                                            found=ed;
                                        }
                                    }
                                    
                                    n=array_tmp[found];
                                }
                            }
                        }
                    }
                    
                    Edges.List[n0].tag=1;
                    
                    if (n!=-1)
                    {
                        Figures.InsertLast()->Copy(f);
                    }
                    
                }
            }
            
            tmpfig.List.Free();
            
            f=Figures.GetFirst();
            while (f)
            {
                f->OptimizeFromPreserve(Vertices);
                
                if (f->List.Length()>2) f=Figures.GetNext();
                else
                {
                    f->List.Free();
                    f=Figures.DeleteAndGetNext();
                }
            }
            
            if (Figures.Length()>0)
            {
                OPTIMIZE_FORDEGEN=1;
                na=nf;
                if (!Triangularisation2(Vertices,&Figures,normal,vertices,faces,&nv,&nf,Faces[nnn].nT,Faces[nnn].nT2,Faces[nnn].nL,Faces[nnn].nLVectors,Faces[nnn].ref,0)) b=false;
                
                OPTIMIZE_FORDEGEN=0;
            }
            
            f=Figures.GetFirst();
            while (f)
            {
                f->List.Free();
                f=Figures.GetNext();
            }
            
            Figures.Free();
            
            Edges.Free();
            
            for (n=minnf;n<=maxnf;n++)
                if (Faces[n].tag2<0) Faces[n].tag2=1;
            
            test=true;
            tag++;
        }
    }
    
    if (b)
    {
        res=new CObject3D;
        res->Init(nv,nf);
        
        for (n=0;n<nv;n++)
        {
            res->Vertices[n].Stok=vertices[n].Stok;
            res->Vertices[n].Calc=vertices[n].Calc;
            res->Vertices[n].Map=vertices[n].Map;
            //res->Vertices[n].Map2=vertices[n].Map2;
            
            res->Vertices[n].tag=0;
            res->Vertices[n].tag2=vertices[n].tag2;
            
            res->Vertices[n].Index[0]=vertices[n].Index[0];
            res->Vertices[n].Index[1]=vertices[n].Index[1];
            res->Vertices[n].Index[2]=vertices[n].Index[2];
            res->Vertices[n].Index[3]=vertices[n].Index[3];
            
            res->Vertices[n].Weight[0]=vertices[n].Weight[0];
            res->Vertices[n].Weight[1]=vertices[n].Weight[1];
            res->Vertices[n].Weight[2]=vertices[n].Weight[2];
            res->Vertices[n].Weight[3]=vertices[n].Weight[3];
        }
        
        for (n=0;n<nf;n++)
        {
            ff=&faces[n];
            res->Faces[n].v0=ff->n0;
            res->Faces[n].v1=ff->n1;
            res->Faces[n].v2=ff->n2;
            res->Faces[n].nT=ff->nT;
            res->Faces[n].nT2=ff->nT2;
            res->Faces[n].nL=ff->nL;
            res->Faces[n].tag=ff->tag;
            res->Faces[n].tag2=0;
            res->Faces[n].nLVectors=ff->nLVectors;
            res->Faces[n].ref=ff->ref;
        }
        
        delete [] vertices;
        delete [] faces;
        
        res->Rot=Rot;
        res->Coo=Coo;
        for (n=0;n<6;n++) res->P[n]=P[n];
        res->nP=nP;
        for (n=0;n<16;n++) res->Tab[n]=Tab[n];
        
        res->Status=Status;
        res->Attribut=Attribut;
        res->env_mapping=env_mapping;
        res->Tag=Tag;
        res->nurbs=nurbs;
        res->SetFaces();
        
        res->CalculateNormals(-1);
        
        res->env_mapping=env_mapping;
        res->group=group;
        
        return res;
    }
    else
    {
        delete [] vertices;
        delete [] faces;
        
        return NULL;
    }
#else
    
    return this->Duplicate2();
    
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		optimize geometry

		CObject3D * CObject3D::OptimizeMeshBoolean()
		CObject3D * CObject3D::OptimizeMeshBoolean_simple()

	Usage:

		merge coplanar face in figures and retriangularisation
		.nT not implemented
		.tag define smoothing groups

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
CObject3D * CObject3D::OptimizeMeshBoolean()
{
	CObject3D *tmp=ConvertLinear(1.0f);
	CObject3D *res=tmp->OptimizeMeshBoolean_simple();
	tmp->Free();
	delete tmp;

	res->group=group;
	res->env_mapping=env_mapping;

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CObject3D::OptimizeMeshBoolean_simple()
{
#ifdef TRIANGULARISATION
	CObject3D * res;
	int n,nn,nnn,n0,N,tag,nb;
	bool test;
	_EdgeListD Edges;
	CList <CFigure> Figures;
	CFigure *f;
	CVertex* vertices;
	CShortFace* faces;
	CFigure tmpfig;
	CShortFace *ff;
	CMatrix M;
	CVector u;
	int na;
	int nv,nf;
	CVector normal;

	M.Id();
	M.RotationDegre(Rot.x,Rot.y,Rot.z);
	M.Translation(Coo.x,Coo.y,Coo.z);

	Calculate();

	faces=new CShortFace[nFaces*16];
	vertices=new CVertex[nVertices*16];

	nv=0;
	nf=0;

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	tag=1;
	test=true;
	while (test)
	{
		n=0;
		nn=-1;
		while ((n<nFaces)&&(nn==-1))
		{
			if ((Faces[n].tag==0)&&(Faces[n].Norm.Norme()>0.5f)) nn=n;
			else
				n++;
		}

		if (nn==-1) test=false;
		else
		{
			nnn=nn;
			TagCoplanarFaces(nn,nn,tag);

			Faces[nn].tag=tag;

			// calcule de l'ensemble des figures form�es par les faces tagg�es
			nb=0;
			for (n=0;n<nFaces;n++)
				if (Faces[n].tag==tag)
					{
						nb+=3;
						if (Faces[n].Norm.Norme()>0.5f)
							normal=Faces[n].Norm;
					}


			if (nb==3)
			{
				vertices[nv+0].Stok=Faces[nn].v[0]->Stok;
				vertices[nv+0].Map=Faces[nn].v[0]->Map;
				vertices[nv+0].Map2=Faces[nn].v[0]->Map2;
				vertices[nv+0].Index[0]=Faces[nn].v[0]->Index[0];
				vertices[nv+0].Index[1]=Faces[nn].v[0]->Index[1];
				vertices[nv+0].Index[2]=Faces[nn].v[0]->Index[2];
				vertices[nv+0].Index[3]=Faces[nn].v[0]->Index[3];
				vertices[nv+0].Weight[0]=Faces[nn].v[0]->Weight[0];
				vertices[nv+0].Weight[1]=Faces[nn].v[0]->Weight[1];
				vertices[nv+0].Weight[2]=Faces[nn].v[0]->Weight[2];
				vertices[nv+0].Weight[3]=Faces[nn].v[0]->Weight[3];

				vertices[nv+1].Stok=Faces[nn].v[1]->Stok;
				vertices[nv+1].Map=Faces[nn].v[1]->Map;
				vertices[nv+1].Map2=Faces[nn].v[1]->Map2;
				vertices[nv+1].Index[0]=Faces[nn].v[1]->Index[0];
				vertices[nv+1].Index[1]=Faces[nn].v[1]->Index[1];
				vertices[nv+1].Index[2]=Faces[nn].v[1]->Index[2];
				vertices[nv+1].Index[3]=Faces[nn].v[1]->Index[3];
				vertices[nv+1].Weight[0]=Faces[nn].v[1]->Weight[0];
				vertices[nv+1].Weight[1]=Faces[nn].v[1]->Weight[1];
				vertices[nv+1].Weight[2]=Faces[nn].v[1]->Weight[2];
				vertices[nv+1].Weight[3]=Faces[nn].v[1]->Weight[3];

				vertices[nv+2].Stok=Faces[nn].v[2]->Stok;
				vertices[nv+2].Map=Faces[nn].v[2]->Map;
				vertices[nv+2].Map2=Faces[nn].v[2]->Map2;
				vertices[nv+2].Index[0]=Faces[nn].v[2]->Index[0];
				vertices[nv+2].Index[1]=Faces[nn].v[2]->Index[1];
				vertices[nv+2].Index[2]=Faces[nn].v[2]->Index[2];
				vertices[nv+2].Index[3]=Faces[nn].v[2]->Index[3];
				vertices[nv+2].Weight[0]=Faces[nn].v[2]->Weight[0];
				vertices[nv+2].Weight[1]=Faces[nn].v[2]->Weight[1];
				vertices[nv+2].Weight[2]=Faces[nn].v[2]->Weight[2];
				vertices[nv+2].Weight[3]=Faces[nn].v[2]->Weight[3];

				faces[nf].n0=nv++;
				faces[nf].n1=nv++;
				faces[nf].n2=nv++;
				faces[nf].nL=Faces[nn].nL;
				faces[nf].nT=Faces[nn].nT;
				faces[nf].nT2=Faces[nn].nT2;
				faces[nf].ref=Faces[nn].ref;

				nf++;

			}
			else
			{

				Edges.Init(nb*2);

				for (n=0;n<nFaces;n++)
				{
					if (Faces[n].tag==tag)
					{
						Edges.AddEdge(Faces[n].v0,Faces[n].v1);
						Edges.AddEdge(Faces[n].v1,Faces[n].v2);
						Edges.AddEdge(Faces[n].v2,Faces[n].v0);
					}
				}

				//Edges.DelDoubleEdges();

				test=true;

				while (test)
				{
					n=0;
					nn=-1;
					while ((n<Edges.nList)&&(nn==-1))
					{
						if (Edges.List[n].tag==0) nn=n;
						n++;
					}

					if (nn==-1) test=false;
					else
					{
						n0=nn;
						f=Figures.InsertLast();
						f->List.Add(Edges.List[n0].a);
						Edges.List[n0].tag=1;
						n=Edges.Next(Edges.List[n0].b);
						Edges.List[n0].tag=0;
						N=0;
						while ((n!=n0)&&(N<250))
						{
							if (n!=-1)
							{
								Edges.List[n].tag=1;
								f->List.Add(Edges.List[n].a);
								n=Edges.Next(Edges.List[n].b);
							}
							N++;
						}

						Edges.List[n0].tag=1;
					}
				}

				tmpfig.List.Free();
				f=Figures.GetFirst();
				while (f)
				{
					//f->preOptimizeFrom(Vertices);					
					tmpfig.Copy(f);
					tmpfig.OptimizeFrom(Vertices);
					if (tmpfig.List.Length()>2) f=Figures.GetNext();
					else
					{
						f->List.Free();
						f=Figures.DeleteAndGetNext();
					}
					tmpfig.List.Free();
				}

				if (Figures.Length()>0)
				{
					na=nf;
					Triangularisation(Vertices,&Figures,normal,vertices,faces,&nv,&nf,Faces[nnn].nT,Faces[nnn].nT2,Faces[nnn].nL,Faces[nnn].ref);
				}

				f=Figures.GetFirst();
				while (f)
				{
					f->List.Free();
					f=Figures.GetNext();
				}

				Figures.Free();
				
				Edges.Free();
			}

			test=true;
			tag++;
		}
	}

	res=new CObject3D;
	res->Init(nv,nf);

	for (n=0;n<nv;n++)
	{
		res->Vertices[n].Stok=vertices[n].Stok;
		res->Vertices[n].Map=vertices[n].Map;
		res->Vertices[n].Map2=vertices[n].Map2;

		res->Vertices[n].Index[0]=vertices[n].Index[0];
		res->Vertices[n].Index[1]=vertices[n].Index[1];
		res->Vertices[n].Index[2]=vertices[n].Index[2];
		res->Vertices[n].Index[3]=vertices[n].Index[3];
		res->Vertices[n].Weight[0]=vertices[n].Weight[0];
		res->Vertices[n].Weight[1]=vertices[n].Weight[1];
		res->Vertices[n].Weight[2]=vertices[n].Weight[2];
		res->Vertices[n].Weight[3]=vertices[n].Weight[3];
	}

	for (n=0;n<nf;n++)
	{
		ff=&faces[n];
		res->Faces[n].v0=ff->n0;
		res->Faces[n].v1=ff->n1;
		res->Faces[n].v2=ff->n2;
		res->Faces[n].nT=ff->nT;
		res->Faces[n].nT2=ff->nT2;
		res->Faces[n].nL=ff->nL;
		res->Faces[n].tag=0;
		res->Faces[n].ref=ff->ref;
	}
	delete [] vertices;
	delete [] faces;



	res->Rot=Rot;
	res->Coo=Coo;
	for (n=0;n<6;n++)
		res->P[n]=P[n];

	res->nP=nP;
	for (n=0;n<16;n++)
		res->Tab[n]=Tab[n];

	res->Status=Status;
	res->Attribut=Attribut;
	res->env_mapping=env_mapping;
	res->Tag=Tag;
	res->nurbs=nurbs;
	res->SetFaces();
	res->CalculateNormals(-1);

	res->group=group;
	res->env_mapping=env_mapping;

	return res;
#else
	return this->Duplicate2();
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 reorder faces and vertices / Y
 
 void CObject3D::ReorderY()
 
 Usage:
 
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::ReorderY()
{
    int n,nf;
    int y;
    
    if (nVertices<256) return nVertices;
    
    CVector maxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
    CVector mini(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.x<mini.x) mini.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y<mini.y) mini.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z<mini.z) mini.z=Vertices[n].Stok.z;
        if (Vertices[n].Stok.x>maxi.x) maxi.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y>maxi.y) maxi.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z>maxi.z) maxi.z=Vertices[n].Stok.z;
    }
    
    int XYZ=0;
    CVector bound=maxi-mini;
    if ((bound.x>=bound.y)&&(bound.x>=bound.z)) XYZ=0;
    else if ((bound.y>=bound.x)&&(bound.y>=bound.z)) XYZ=1; else XYZ=2;
    
    float dd;
    if (XYZ==0) dd=bound.x;
    if (XYZ==1) dd=bound.y;
    if (XYZ==2) dd=bound.z;
    
    int ZDEF=512;
    CList<int>* tempsort=new CList<int>[ZDEF];
    int *pi;
    
    float ymin=MAXI_BOUND;
    float ymax=-MAXI_BOUND;
    
    if (XYZ==0)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=(Faces[n].v[0]->Stok.x+Faces[n].v[1]->Stok.x+Faces[n].v[2]->Stok.x)/3;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
        }
    }

    if (XYZ==1)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=(Faces[n].v[0]->Stok.y+Faces[n].v[1]->Stok.y+Faces[n].v[2]->Stok.y)/3;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
        }
    }

    if (XYZ==2)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=(Faces[n].v[0]->Stok.z+Faces[n].v[1]->Stok.z+Faces[n].v[2]->Stok.z)/3;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
        }
    }

	int NB=0;

	if (ymax-ymin>SMALLF2)
	{
        ymin-=SMALLF2;
        ymax+=SMALLF2;

		for (n=0;n<nFaces;n++)
		{
			y=(int)((ZDEF-1)*(Faces[n].coef1-ymin)/(ymax-ymin));
            if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
            tempsort[y].Add(n);
		}

		CFace * fcs=Faces;
		Faces=new CFace[nFaces];
		nf=0;
		for (y=0;y<ZDEF;y++)
		{
            pi=tempsort[y].GetFirst();
            while (pi)
            {
                Faces[nf++]=fcs[*pi];
                pi=tempsort[y].GetNext();
            }
            
            tempsort[y].Free();
		}
    
		delete [] fcs;
		delete [] tempsort;
	}
	else
	{
		delete [] tempsort;

		return nVertices;
	}
    
    tempsort=new CList<int>[ZDEF];
    
	if (ymax-ymin>SMALLF2)
	{
		ymin=MAXI_BOUND;
		ymax=-MAXI_BOUND;
        
        if (XYZ==0)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.x<ymin) ymin=Vertices[n].Stok.x;
                if (Vertices[n].Stok.x>ymax) ymax=Vertices[n].Stok.x;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.x-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }

        if (XYZ==1)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.y<ymin) ymin=Vertices[n].Stok.y;
                if (Vertices[n].Stok.y>ymax) ymax=Vertices[n].Stok.y;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.y-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }

        if (XYZ==2)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.z<ymin) ymin=Vertices[n].Stok.z;
                if (Vertices[n].Stok.z>ymax) ymax=Vertices[n].Stok.z;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.z-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }
    
		CVertex * vtx=Vertices;
		Vertices=new CVertex[nVertices];
    
		int * corres=new int[nVertices];
    
		nf=0;
		for (y=0;y<ZDEF;y++)
		{
            if (tempsort[y].Length()>NB) NB=tempsort[y].Length();
            
            pi=tempsort[y].GetFirst();
            while (pi)
            {
                Vertices[nf]=vtx[*pi];
                corres[*pi]=nf;
                nf++;
                pi=tempsort[y].GetNext();
            }
            
            tempsort[y].Free();
		}
        
        if (nKeys>0)
        {
            for (int k=0;k<nKeys;k++)
            {
                CShortVertex *oldVerticesKeys=VerticesKeys[k];
                VerticesKeys[k]=new CShortVertex[nVertices];
                for (n=0;n<nVertices;n++) VerticesKeys[k][corres[n]]=oldVerticesKeys[n];
                delete [] oldVerticesKeys;
            }
        }
    
		for (n=0;n<nFaces;n++)
		{
			Faces[n].v0=corres[Faces[n].v0];
			Faces[n].v1=corres[Faces[n].v1];
			Faces[n].v2=corres[Faces[n].v2];
		}
    
		delete [] corres;
    
		delete [] vtx;
		delete [] tempsort;
	}
	else
	{
		delete [] tempsort;

		return nVertices;
	}
    
	SetFaces();
    
    return NB;
}

int CObject3D::ReorderYFaces()
{
    int n,nf;
    int y;
    
    if (nVertices<256) return nVertices;
    
    CVector maxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
    CVector mini(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.x<mini.x) mini.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y<mini.y) mini.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z<mini.z) mini.z=Vertices[n].Stok.z;
        if (Vertices[n].Stok.x>maxi.x) maxi.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y>maxi.y) maxi.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z>maxi.z) maxi.z=Vertices[n].Stok.z;
    }
    
    int XYZ=0;
    CVector bound=maxi-mini;
    if ((bound.x>=bound.y)&&(bound.x>=bound.z)) XYZ=0;
    else if ((bound.y>=bound.x)&&(bound.y>=bound.z)) XYZ=1; else XYZ=2;
    
    float dd;
    if (XYZ==0) dd=bound.x;
    if (XYZ==1) dd=bound.y;
    if (XYZ==2) dd=bound.z;
    
    int ZDEF=512;
    CList<int>* tempsort=new CList<int>[ZDEF];
    int *pi;
    
    float ymin=MAXI_BOUND;
    float ymax=-MAXI_BOUND;
    
    if (XYZ==0)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=Faces[n].v[0]->Stok.x;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[1]->Stok.x;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[2]->Stok.x;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=(Faces[n].v[0]->Stok.x+Faces[n].v[1]->Stok.x+Faces[n].v[2]->Stok.x)/3;
        }
        
        for (n=0;n<nVertices;n++) Vertices[n].coef1=Vertices[n].Stok.x;
    }

    if (XYZ==1)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=Faces[n].v[0]->Stok.y;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[1]->Stok.y;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[2]->Stok.y;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=(Faces[n].v[0]->Stok.y+Faces[n].v[1]->Stok.y+Faces[n].v[2]->Stok.y)/3;
        }
        
        for (n=0;n<nVertices;n++) Vertices[n].coef1=Vertices[n].Stok.y;
    }

    if (XYZ==2)
    {
        for (n=0;n<nFaces;n++)
        {
            Faces[n].coef1=Faces[n].v[0]->Stok.z;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[1]->Stok.z;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=Faces[n].v[2]->Stok.z;
            if (Faces[n].coef1<ymin) ymin=Faces[n].coef1;
            if (Faces[n].coef1>ymax) ymax=Faces[n].coef1;
            Faces[n].coef1=(Faces[n].v[0]->Stok.z+Faces[n].v[1]->Stok.z+Faces[n].v[2]->Stok.z)/3;
        }
        
        for (n=0;n<nVertices;n++) Vertices[n].coef1=Vertices[n].Stok.z;
    }

    int NB=0;

    if (ymax-ymin>SMALLF2)
    {
        ymin-=SMALLF2;
        ymax+=SMALLF2;
        
        int *maxcount=new int[ZDEF];
        
        for (n=0;n<ZDEF;n++) maxcount[n]=0;
        
        float areamin=100000;
        float areamax=0;
        CVector u1,u2,u3;

        for (n=0;n<nFaces;n++)
        {
            VECTORSUB(u1,Faces[n].v[1]->Stok,Faces[n].v[0]->Stok);
            VECTORSUB(u2,Faces[n].v[2]->Stok,Faces[n].v[0]->Stok);
            VECTORSUB(u3,Faces[n].v[2]->Stok,Faces[n].v[1]->Stok);
            float ar=VECTORNORM2(u1);
            if (ar<areamin) areamin=ar;
            if (ar>areamax) areamax=ar;
            ar=VECTORNORM2(u2);
            if (ar<areamin) areamin=ar;
            if (ar>areamax) areamax=ar;
            ar=VECTORNORM2(u3);
            if (ar<areamin) areamin=ar;
            if (ar>areamax) areamax=ar;

            y=(int)((ZDEF-1)*(Faces[n].coef1-ymin)/(ymax-ymin));
            if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
            maxcount[y]++;
            tempsort[y].Add(n);
            
            y=(int)((ZDEF-1)*(Faces[n].v[0]->coef1-ymin)/(ymax-ymin));
            if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
            maxcount[y]++;
            y=(int)((ZDEF-1)*(Faces[n].v[1]->coef1-ymin)/(ymax-ymin));
            if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
            maxcount[y]++;
            y=(int)((ZDEF-1)*(Faces[n].v[2]->coef1-ymin)/(ymax-ymin));
            if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
            maxcount[y]++;
        }

        CFace * fcs=Faces;
        Faces=new CFace[nFaces];
        nf=0;
        for (y=0;y<ZDEF;y++)
        {
            if (tempsort[y].Length()>NB) NB=tempsort[y].Length();
            if (maxcount[y]>NB) NB=maxcount[y];
            
            pi=tempsort[y].GetFirst();
            while (pi)
            {
                Faces[nf++]=fcs[*pi];
                pi=tempsort[y].GetNext();
            }
            
            tempsort[y].Free();
        }
        
        int MUL=(int)((sqrtf(areamax)/sqrtf(areamin))/100);
        if (MUL<1) MUL=1;
        NB=(int)(NB*MUL);
        
        delete [] maxcount;
    
        delete [] fcs;
        delete [] tempsort;
    }
    else
    {
        delete [] tempsort;
        return nVertices;
    }
    
    tempsort=new CList<int>[ZDEF];
    
    if (ymax-ymin>SMALLF2)
    {
        ymin=MAXI_BOUND;
        ymax=-MAXI_BOUND;
        
        if (XYZ==0)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.x<ymin) ymin=Vertices[n].Stok.x;
                if (Vertices[n].Stok.x>ymax) ymax=Vertices[n].Stok.x;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.x-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }

        if (XYZ==1)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.y<ymin) ymin=Vertices[n].Stok.y;
                if (Vertices[n].Stok.y>ymax) ymax=Vertices[n].Stok.y;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.y-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }

        if (XYZ==2)
        {
            for (n=0;n<nVertices;n++)
            {
                if (Vertices[n].Stok.z<ymin) ymin=Vertices[n].Stok.z;
                if (Vertices[n].Stok.z>ymax) ymax=Vertices[n].Stok.z;
            }

            ymin-=SMALLF2;
            ymax+=SMALLF2;
        
            for (n=0;n<nVertices;n++)
            {
                y=(int)((ZDEF-1)*(Vertices[n].Stok.z-ymin)/(ymax-ymin));
                if ((y<0)||(y>ZDEF-1)) y=ZDEF/2;
                tempsort[y].Add(n);
            }
        }
    
        CVertex * vtx=Vertices;
        Vertices=new CVertex[nVertices];
    
        int * corres=new int[nVertices];
    
        nf=0;
        for (y=0;y<ZDEF;y++)
        {
            pi=tempsort[y].GetFirst();
            while (pi)
            {
                Vertices[nf]=vtx[*pi];
                corres[*pi]=nf;
                nf++;
                pi=tempsort[y].GetNext();
            }
            
            tempsort[y].Free();
        }
    
        if (nKeys>0)
        {
            for (int k=0;k<nKeys;k++)
            {
                CShortVertex *oldVerticesKeys=VerticesKeys[k];
                VerticesKeys[k]=new CShortVertex[nVertices];
                for (n=0;n<nVertices;n++) VerticesKeys[k][corres[n]]=oldVerticesKeys[n];
                delete [] oldVerticesKeys;
            }
        }

        for (n=0;n<nFaces;n++)
        {
            Faces[n].v0=corres[Faces[n].v0];
            Faces[n].v1=corres[Faces[n].v1];
            Faces[n].v2=corres[Faces[n].v2];
        }
    
        delete [] corres;
    
        delete [] vtx;
        delete [] tempsort;
    }
    else
    {
        delete [] tempsort;
        return nVertices;
    }
    
    SetFaces();
    
    return NB;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 for ray intersections

 void CObject3D::PrecomputeSides()
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PrecomputeSides()
{
    int n;
    CVector a,c;
    
    for (n=0;n<nFaces;n++)
    {
        CFace *F=&(Faces[n]);
        VECTORSUB(a,F->v[1]->Calc,F->v[0]->Calc);
        CROSSPRODUCT(c,F->NormCalc,a);
        F->g=c;
        VECTORSUB(a,F->v[2]->Calc,F->v[1]->Calc);
        CROSSPRODUCT(c,F->NormCalc,a);
        F->mp0.x=c.x;
        F->mp0.y=c.y;
        F->mp1.x=c.z;
        VECTORSUB(a,F->v[0]->Calc,F->v[2]->Calc);
        CROSSPRODUCT(c,F->NormCalc,a);
        F->mp1.y=c.x;
        F->mp2.x=c.y;
        F->mp2.y=c.z;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 for ray intersections
 
 bool CFace::Inside(CVector &I)
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CFace::Inside(CVector &I)
{
    CVector u;
    
    VECTORSUB(u,I,v[0]->Calc)
    if (DOT(u,g)<0)
    {
        VECTORSUB(u,I,v[1]->Calc)
        if (u.x*mp0.x+u.y*mp0.y+u.z*mp1.x<0)
        {
            VECTORSUB(u,I,v[2]->Calc)
            if (u.x*mp1.y+u.y*mp2.x+u.z*mp2.y<0) return true;
        }
    }
    
    return false;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectFace(CVector A,CVector B,CFace * F)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CObject3D::FaceApprox(CVector &I,CFace * F)
{
	float sss=SMALLF2;
	int res;
	float s1,s2,s3;
	CVector a1,a2,a3,O,b1,b2,b3,c1,c2,c3;

	res=0;

	VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc)
	VECTORSUB(b1,I,F->v[0]->Calc)
	CROSSPRODUCT(c1,b1,a1)
	DOTPRODUCT(s1,F->NormCalc,c1)
	if (s1>=-sss)
	{
		VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc)
		VECTORSUB(b2,I,F->v[1]->Calc)
		CROSSPRODUCT(c2,b2,a2)
		DOTPRODUCT(s2,F->NormCalc,c2)
		if (s2>=-sss)
		{
			VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc)
			VECTORSUB(b3,I,F->v[2]->Calc)
			CROSSPRODUCT(c3,b3,a3)
			DOTPRODUCT(s3,F->NormCalc,c3)
			if (s3>=-sss)
			{
				res=1;
			}
		}
	}

	return (res==1);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectFace(CVector A,CVector B,CFace * F)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CObject3D::IntersectFaceApprox(CVector &A,CVector &B,CFace * F)
{
#ifdef MOLLER_TRUMBORE
    CVector dir,v0v1,v0v2,pvec,tvec,qvec;
    
    VECTORSUB(dir,B,A);
    float len=VECTORNORM(dir);
    VECTORDIV(dir,dir,len);
    VECTORSUB(v0v1,F->v[1]->Calc,F->v[0]->Calc);
    VECTORSUB(v0v2,F->v[2]->Calc,F->v[0]->Calc);
    
    CROSSPRODUCT(pvec,dir,v0v2);
    float det = DOT(v0v1,pvec);
    if (f_abs(det) < SMALLF) return false;
    float invDet = 1 / det;
 
    VECTORSUB(tvec,A,F->v[0]->Calc);
    float u = DOT(tvec,pvec) * invDet;
    if (u < 0 || u > 1) return false;
 
    CROSSPRODUCT(qvec,tvec,v0v1);
    float v = DOT(dir,qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
 
    t = DOT(v0v2,qvec) * invDet;
    if ((t>=-SMALLF)&&(t<=len+SMALLF))
    {
        PointI=A+t*dir;
        PointI_t=t/len;
        PointINorm=F->NormCalc;
        return true;
    }
    else return false;
#else

	float sss=SMALLF2;
	int res;
	float t;
	float a,b,c,d;
	float s1,s2,s3;
	CVector I,tmp,a1,a2,a3,O,b1,b2,b3,c1,c2,c3;

	res=0;
	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	DOTPRODUCT(d,F->v[0]->Calc,F->NormCalc)
	d=-d;

	s1=a*A.x+b*A.y+c*A.z+d;
	s2=a*B.x+b*B.y+c*B.z+d;

	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		VECTORINTERPOL(I,t,B,A);
		VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc)
		VECTORSUB(b1,I,F->v[0]->Calc)
		CROSSPRODUCT(c1,b1,a1)
		DOTPRODUCT(s1,F->NormCalc,c1)
		if (s1>=-sss)
		{
			VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc)
			VECTORSUB(b2,I,F->v[1]->Calc)
			CROSSPRODUCT(c2,b2,a2)
			DOTPRODUCT(s2,F->NormCalc,c2)
			if (s2>=-sss)
			{
				VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc)
				VECTORSUB(b3,I,F->v[2]->Calc)
				CROSSPRODUCT(c3,b3,a3)
				DOTPRODUCT(s3,F->NormCalc,c3)
				if (s3>=-sss)
				{
					res=1;
					PointI=I;
					PointI_t=t;
					PointINorm=F->NormCalc;
				}
			}
		}
	}

	return (res==1);
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectFace(CVector A,CVector B,CFace * F)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */



bool CObject3D::IntersectFace(CVector &A,CVector &B,CFace * F)
{
#ifdef MOLLER_TRUMBORE
    CVector dir,v0v1,v0v2,pvec,tvec,qvec;
    
    VECTORSUB(dir,B,A);
    float len=VECTORNORM(dir);
    VECTORDIV(dir,dir,len);
    VECTORSUB(v0v1,F->v[1]->Calc,F->v[0]->Calc);
    VECTORSUB(v0v2,F->v[2]->Calc,F->v[0]->Calc);
    
    CROSSPRODUCT(pvec,dir,v0v2);
    float det = DOT(v0v1,pvec);
    if (f_abs(det) < SMALLF) return false;
    float invDet = 1 / det;
 
    VECTORSUB(tvec,A,F->v[0]->Calc);
    float u = DOT(tvec,pvec) * invDet;
    if (u < 0 || u > 1) return false;
 
    CROSSPRODUCT(qvec,tvec,v0v1);
    float v = DOT(dir,qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
 
    t = DOT(v0v2,qvec) * invDet;
    if ((t>=0.0f)&&(t<=len))
    {
        PointI=A+t*dir;
        PointI_t=t/len;
        PointINorm=F->NormCalc;
        return true;
    }
    else return false;
#else
	int res;
	float t;
	float a,b,c,d;
	float s1,s2,s3;
	CVector I,tmp,a1,a2,a3,O,b1,b2,b3,c1,c2,c3;

	res=0;
	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	DOTPRODUCT(d,F->v[0]->Calc,F->NormCalc)
	d=-d;

	s1=a*A.x+b*A.y+c*A.z+d;
	s2=a*B.x+b*B.y+c*B.z+d;

	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
        VECTORINTERPOL(I,t,B,A);
		VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc)
		VECTORSUB(b1,I,F->v[0]->Calc)
		CROSSPRODUCT(c1,b1,a1)
		DOTPRODUCT(s1,F->NormCalc,c1)
		if (s1>=0)
		{
			VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc)
			VECTORSUB(b2,I,F->v[1]->Calc)
			CROSSPRODUCT(c2,b2,a2)
			DOTPRODUCT(s2,F->NormCalc,c2)
			if (s2>=0)
			{
				VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc)
				VECTORSUB(b3,I,F->v[2]->Calc)
				CROSSPRODUCT(c3,b3,a3)
				DOTPRODUCT(s3,F->NormCalc,c3)
				if (s3>=0)
				{
					res=1;
					PointI=I;
					PointI_t=t;
					PointINorm=F->NormCalc;
				}
			}
		}
	}

	return (res==1);
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectFaceMatrix(CVector A,CVector B,CFace * F,CMatrix M)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


bool CObject3D::IntersectFaceMatrix(CVector &A,CVector &B,CFace * F,CVector &N)
{
#ifdef MOLLER_TRUMBORE
    CVector dir,v0v1,v0v2,pvec,tvec,qvec;
    
    VECTORSUB(dir,B,A);
    float len=VECTORNORM(dir);
    VECTORDIV(dir,dir,len);
    VECTORSUB(v0v1,F->v[1]->Stok0,F->v[0]->Stok0);
    VECTORSUB(v0v2,F->v[2]->Stok0,F->v[0]->Stok0);
    
    CROSSPRODUCT(pvec,dir,v0v2);
    float det = DOT(v0v1,pvec);
    if (f_abs(det) < SMALLF) return false;
    float invDet = 1 / det;
 
    VECTORSUB(tvec,A,F->v[0]->Stok0);
    float u = DOT(tvec,pvec) * invDet;
    if (u < 0 || u > 1) return false;
 
    CROSSPRODUCT(qvec,tvec,v0v1);
    float v = DOT(dir,qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
 
    t = DOT(v0v2,qvec) * invDet;
    if ((t>=0.0f)&&(t<=len))
    {
        PointI=A+t*dir;
        PointI_t=t/len;
        PointINorm=N;
        return true;
    }
    else return false;
#else

	int res;
	float t;
	float a,b,c,d;
	float s1,s2,s3;
	CVector I,tmp,a1,a2,a3,O,b1,b2,b3,c1,c2,c3;

	res=0;
	a=N.x;
	b=N.y;
	c=N.z;
	DOTPRODUCT(d,F->v[0]->Stok0,N)
	d=-d;

	s1=a*A.x+b*A.y+c*A.z+d;
	s2=a*B.x+b*B.y+c*B.z+d;

	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		VECTORINTERPOL(I,t,B,A);
		VECTORSUB(a1,F->v[1]->Stok0,F->v[0]->Stok0)
		VECTORSUB(b1,I,F->v[0]->Stok0)
		CROSSPRODUCT(c1,b1,a1)
		DOTPRODUCT(s1,N,c1)
		if (s1>=0)
		{
			VECTORSUB(a2,F->v[2]->Stok0,F->v[1]->Stok0)
			VECTORSUB(b2,I,F->v[1]->Stok0)
			CROSSPRODUCT(c2,b2,a2)
			DOTPRODUCT(s2,N,c2)
			if (s2>=0)
			{
				VECTORSUB(a3,F->v[0]->Stok0,F->v[2]->Stok0)
				VECTORSUB(b3,I,F->v[2]->Stok0)
				CROSSPRODUCT(c3,b3,a3)
				DOTPRODUCT(s3,N,c3)
				if (s3>=0)
				{
					res=1;
					PointI=I;
					PointI_t=t;
					PointINorm=N;
				}
			}
		}
	}

	return (res==1);
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectObjet(CVector A,CVector B,CVector Base)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
bool CObject3D::IntersectObjet(CVector &A,CVector &B,CVector &Base)
{
	int n,res;
	CMatrix M;
	CVector p,u1,u2;

	res=0;
	n=0;
	while ((res==0)&&(n<nFaces))
	{
		if (IntersectFace(A,B,&Faces[n]))
		{
			VECTORSUB(u1,PointI,A)
			VECTORSUB(u2,Base,A)
			if (VECTORNORM(u1)>VECTORNORM(u2))
            {
                res=1;
                FaceI=n;
            }
		}
		n++;
	}

	return (res==1);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		ray intersection
		calc values token

		bool CObject3D::IntersectObjet(CVector A,CVector B,CVector Base)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CObject3D::IntersectObjetApprox(CVector &A,CVector &B,CVector &Base)
{
	int n,res;
	CMatrix M;
	CVector p,u1,u2;

	res=0;
	n=0;
	while ((res==0)&&(n<nFaces))
	{
		if (IntersectFaceApprox(A,B,&Faces[n]))
		{
			FaceI=n;
			VECTORSUB(u1,PointI,A)
			VECTORSUB(u2,Base,A)
			if (VECTORNORM2(u1)>VECTORNORM2(u2)) res=1;

		}
		n++;
	}

	return (res==1);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        coo mapping
 
        CVector2 CObject3D::CalculateMappingFromPointAndFace(int nf,CVector I)

    Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CVector2 CObject3D::CalculateMappingFromPointAndFace(int FACE,CVector &Pointer)
{
    CMatrix M,MM;
    CVector u;
    float s1,s2,s3;
    CVector2 m;
    
    M.Id();
    M.a[0][0]=Faces[FACE].v[0]->Calc.x;
    M.a[0][1]=Faces[FACE].v[0]->Calc.y;
    M.a[0][2]=Faces[FACE].v[0]->Calc.z;
    M.a[1][0]=Faces[FACE].v[1]->Calc.x;
    M.a[1][1]=Faces[FACE].v[1]->Calc.y;
    M.a[1][2]=Faces[FACE].v[1]->Calc.z;
    M.a[2][0]=Faces[FACE].v[2]->Calc.x;
    M.a[2][1]=Faces[FACE].v[2]->Calc.y;
    M.a[2][2]=Faces[FACE].v[2]->Calc.z;
    MM.Transpose(M);
    M.Inverse3x3(MM);
    u=Pointer*M;
    s1=u.x;
    s2=u.y;
    s3=u.z;
    m=(Faces[FACE].v[0]->Map*s1 + Faces[FACE].v[1]->Map*s2 + Faces[FACE].v[2]->Map*s3)/(s1+s2+s3);
    
    return m;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		  normals calculation

			same index vertex

			void CObject3D::CalculateNormals(int tag)
			void CObject3D::CalculateNormalsSmoothingGroups()

			close vertex

			void CObject3D::CalculateNormals2(int tag)
			void CObject3D::CalculateNormalsSmoothingGroups2()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define SMALLFNP SMALLF

bool FaceNormalePrecalc(CFace * f,CObject3D *obj)
{
	CVector vec1,vec2,vec3;
	bool res=false;
	float n1,n2;

	VECTORINIT(vec3,0,0,0);

	VECTORSUB(vec1,obj->Vertices[f->v1].Stok, obj->Vertices[f->v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f->v2].Stok, obj->Vertices[f->v0].Stok);
	n1=VECTORNORM(vec1);
	n2=VECTORNORM(vec2);

	if ((n1>SMALLFNP)&&(n2>SMALLFNP))
	{
		VECTORDIV(vec1,vec1,n1);
		VECTORDIV(vec2,vec2,n2);

		float d=DOT(vec1,vec2);
		if (d<0) d=-d;
		if (d<1.0f-SMALLF)
		{
			CROSSPRODUCT(vec3,vec2,vec1);
			VECTORNORMALISE(vec3);
			res=true;
		}
	}

	if (!res)
	{
		VECTORSUB(vec1,obj->Vertices[f->v2].Stok,obj->Vertices[f->v1].Stok);
		VECTORSUB(vec2,obj->Vertices[f->v0].Stok,obj->Vertices[f->v1].Stok);
		n1=VECTORNORM(vec1);
		n2=VECTORNORM(vec2);
		if ((n1>SMALLFNP)&&(n2>SMALLFNP))
		{
			VECTORDIV(vec1,vec1,n1);
			VECTORDIV(vec2,vec2,n2);

			float d=DOT(vec1,vec2);
			if (d<0) d=-d;
			if (d<1.0f-SMALLF)
			{
				CROSSPRODUCT(vec3,vec2,vec1);
				VECTORNORMALISE(vec3);
				res=true;
			}
		}
	}

	if (!res)
	{
		VECTORSUB(vec1,obj->Vertices[f->v0].Stok,obj->Vertices[f->v2].Stok);
		VECTORSUB(vec2,obj->Vertices[f->v1].Stok,obj->Vertices[f->v2].Stok);
		n1=VECTORNORM(vec1);
		n2=VECTORNORM(vec2);
		if ((n1>SMALLFNP)&&(n2>SMALLFNP))
		{
			VECTORDIV(vec1,vec1,n1);
			VECTORDIV(vec2,vec2,n2);

			float d=DOT(vec1,vec2);
			if (d<0) d=-d;
			if (d<1.0f-SMALLF)
			{
				CROSSPRODUCT(vec3,vec2,vec1);
				VECTORNORMALISE(vec3);
				res=true;
			}
		}
	}

	f->Norm=vec3;

	VECTORADD(obj->Vertices[f->v0].Norm,obj->Vertices[f->v0].Norm, vec3);
	VECTORADD(obj->Vertices[f->v1].Norm,obj->Vertices[f->v1].Norm, vec3);
	VECTORADD(obj->Vertices[f->v2].Norm,obj->Vertices[f->v2].Norm, vec3);

	return res;
}


bool FaceNormalePrecalcTag(CFace * f,CObject3D *obj)
{
	CVector vec1,vec2,vec3;
	bool res=false;
	float n1,n2;

	VECTORINIT(vec3,0,0,0);

	VECTORSUB(vec1,obj->Vertices[f->v1].Stok, obj->Vertices[f->v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f->v2].Stok, obj->Vertices[f->v0].Stok);
	n1=VECTORNORM(vec1);
	n2=VECTORNORM(vec2);

	if ((n1>SMALLFNP)&&(n2>SMALLFNP))
	{
		VECTORDIV(vec1,vec1,n1);
		VECTORDIV(vec2,vec2,n2);

		float d=DOT(vec1,vec2);
		if (d<0) d=-d;
		if (d<1.0f-SMALLF)
		{
			CROSSPRODUCT(vec3,vec2,vec1);
			VECTORNORMALISE(vec3);
			res=true;
		}
	}


	if (!res)
	{
		VECTORSUB(vec1,obj->Vertices[f->v2].Stok,obj->Vertices[f->v1].Stok);
		VECTORSUB(vec2,obj->Vertices[f->v0].Stok,obj->Vertices[f->v1].Stok);
		n1=VECTORNORM(vec1);
		n2=VECTORNORM(vec2);
		if ((n1>SMALLFNP)&&(n2>SMALLFNP))
		{
			VECTORDIV(vec1,vec1,n1);
			VECTORDIV(vec2,vec2,n2);

			float d=DOT(vec1,vec2);
			if (d<0) d=-d;
			if (d<1.0f-SMALLF)
			{
				CROSSPRODUCT(vec3,vec2,vec1);
				VECTORNORMALISE(vec3);
				res=true;
			}
		}
	}

	if (!res)
	{
		VECTORSUB(vec1,obj->Vertices[f->v0].Stok,obj->Vertices[f->v2].Stok);
		VECTORSUB(vec2,obj->Vertices[f->v1].Stok,obj->Vertices[f->v2].Stok);
		n1=VECTORNORM(vec1);
		n2=VECTORNORM(vec2);
		if ((n1>SMALLFNP)&&(n2>SMALLFNP))
		{
			VECTORDIV(vec1,vec1,n1);
			VECTORDIV(vec2,vec2,n2);

			float d=DOT(vec1,vec2);
			if (d<0) d=-d;
			if (d<1.0f-SMALLF)
			{
				CROSSPRODUCT(vec3,vec2,vec1);
				VECTORNORMALISE(vec3);
				res=true;
			}
		}
	}

	f->Norm=vec3;

	if (obj->Vertices[f->v0].tag==0) { VECTORADD(obj->Vertices[f->v0].Norm,obj->Vertices[f->v0].Norm, vec3); }
	if (obj->Vertices[f->v1].tag==0) { VECTORADD(obj->Vertices[f->v1].Norm,obj->Vertices[f->v1].Norm, vec3); }
	if (obj->Vertices[f->v2].tag==0) { VECTORADD(obj->Vertices[f->v2].Norm,obj->Vertices[f->v2].Norm, vec3); }

	return res;
}

bool FaceNormalePrecalcBase(CFace * f,CObject3D *obj)
{
	CVector vec1,vec2;
	VECTORSUB(vec1,obj->Vertices[f->v1].Stok, obj->Vertices[f->v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f->v2].Stok, obj->Vertices[f->v0].Stok);
	CROSSPRODUCT(f->Norm,vec2,vec1);
	VECTORNORMALISE(f->Norm);
	return true;
}

bool FaceNormalePrecalcBase2(CFace * f,CObject3D *obj)
{
	CVector vec1,vec2;
	VECTORSUB(vec1,obj->Vertices[f->v1].Calc, obj->Vertices[f->v0].Calc);
	VECTORSUB(vec2,obj->Vertices[f->v2].Calc, obj->Vertices[f->v0].Calc);
	CROSSPRODUCT(f->NormCalc,vec2,vec1);
    VECTORNORMALISE(f->NormCalc);
	return true;
}

void FaceNormalePrecalc2(CFace f,CObject3D *obj)
{
	CVector vec1,vec2,vec3;

	VECTORSUB(vec1,obj->Vertices[f.v1].Stok,obj->Vertices[f.v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f.v2].Stok,obj->Vertices[f.v0].Stok);

	CROSSPRODUCT(vec3,vec2,vec1)

	VECTORADD(obj->Vertices[f.v0].NormCalc,obj->Vertices[f.v0].NormCalc,vec3)
	VECTORADD(obj->Vertices[f.v1].NormCalc,obj->Vertices[f.v1].NormCalc,vec3)
	VECTORADD(obj->Vertices[f.v2].NormCalc,obj->Vertices[f.v2].NormCalc,vec3)
}


void FaceNormalePrecalcXX(CFace &f,CObject3D *obj)
{
    CVector vec1,vec2,vec3,vec4;

    VECTORSUB(vec1,obj->Vertices[f.v1].Stok,obj->Vertices[f.v0].Stok);
    VECTORSUB(vec2,obj->Vertices[f.v2].Stok,obj->Vertices[f.v0].Stok);

    CROSSPRODUCT(vec3,vec2,vec1)

    VECTORSUB(vec1,obj->Vertices[f.v2].Stok,obj->Vertices[f.v1].Stok);
    VECTORSUB(vec2,obj->Vertices[f.v0].Stok,obj->Vertices[f.v1].Stok);

    CROSSPRODUCT(vec4,vec2,vec1)

    float v3=VECTORNORM(vec3);
    float v4=VECTORNORM(vec4);

    if (v4<v3) { VECTORDIV(f.Norm,vec3,v3); }
    else { VECTORDIV(f.Norm,vec4,v4); }

    VECTORADD(obj->Vertices[f.v0].Norm,obj->Vertices[f.v0].Norm,vec3)
    VECTORADD(obj->Vertices[f.v1].Norm,obj->Vertices[f.v1].Norm,vec3)
    VECTORADD(obj->Vertices[f.v2].Norm,obj->Vertices[f.v2].Norm,vec3)
}

void FaceNormalePrecalcXX2(CFace &f,CObject3D *obj)
{
    CVector vec1,vec2;

    VECTORSUB(vec1,obj->Vertices[f.v1].Stok,obj->Vertices[f.v0].Stok);
    VECTORSUB(vec2,obj->Vertices[f.v2].Stok,obj->Vertices[f.v0].Stok);

    CROSSPRODUCT(f.Norm,vec2,vec1)

    VECTORADD(obj->Vertices[f.v0].Norm,obj->Vertices[f.v0].Norm,f.Norm)
    VECTORADD(obj->Vertices[f.v1].Norm,obj->Vertices[f.v1].Norm,f.Norm)
    VECTORADD(obj->Vertices[f.v2].Norm,obj->Vertices[f.v2].Norm,f.Norm)
}

void CObject3D::InvertNormals()
{
	int n;
	for (n=0;n<nFaces;n++) VECTORNEG(Faces[n].Norm);
	for (n=0;n<nVertices;n++) VECTORNEG(Vertices[n].Norm);
}

void CObject3D::CalculateNormalsQuick()
{
	int n;
	
	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Norm.x=0.0f;
		Vertices[n].Norm.y=0.0f;
		Vertices[n].Norm.z=0.0f;
	}

    for (n=0;n<nFaces;n++) FaceNormalePrecalcXX2(this->Faces[n],this);

    for (n=0;n<nVertices;n++) VECTORNORMALISE(Vertices[n].Norm);
}

void CObject3D::CalculateNormals(int tag)
{
	int n;

	if (nFaces==0) return;
	if (Status==88) return;

	if (nKeys>0)
	{
		for (n=0;n<nVertices;n++)
		{
			Vertices[n].Calc=Vertices[n].Stok;
			Vertices[n].NormCalc=Vertices[n].Norm;
		}

		if (tag==-2)
		{
			//CreateMCV();
			for (int k=0;k<nKeys;k++)
			{
				for (n=0;n<nVertices;n++) Vertices[n].Stok=VerticesKeys[k][n].Stok;
				CalculateNormalsBasic(-1);
				for (n=0;n<nVertices;n++) VerticesKeys[k][n].Norm=Vertices[n].Norm;
			}
			//FreeMCV();
		}
		else
		{
			for (int k=0;k<nKeys;k++)
			{
				for (n=0;n<nVertices;n++) Vertices[n].Stok=VerticesKeys[k][n].Stok;
				CalculateNormalsSimple(-1);
				for (n=0;n<nVertices;n++) VerticesKeys[k][n].Norm=Vertices[n].Norm;
			}
		}

		for (n=0;n<nVertices;n++)
		{
			Vertices[n].Stok=Vertices[n].Calc;
			Vertices[n].Norm=Vertices[n].NormCalc;
		}
	}

	if (tag==-1)
	{
		for (n=0;n<nVertices;n++)
		{
			Vertices[n].Norm.x=0.0f;
			Vertices[n].Norm.y=0.0f;
			Vertices[n].Norm.z=0.0f;
		}

		for (n=0;n<nFaces;n++) FaceNormalePrecalc(&this->Faces[n],this);

		for (n=0;n<nVertices;n++)
		{
			VECTORNORMALISE(Vertices[n].Norm);
		}
	}
	else
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=-1;

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=0;
				Faces[n].v[1]->tag=0;
				Faces[n].v[2]->tag=0;
			}
		}

		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag==0)
			{
				Vertices[n].Norm.x=0.0f;
				Vertices[n].Norm.y=0.0f;
				Vertices[n].Norm.z=0.0f;
			}
		}

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag) FaceNormalePrecalcTag(&this->Faces[n],this);
		}

		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag==0) { VECTORNORMALISE(Vertices[n].Norm); }
		}

	}

}

void CObject3D::CalculateNormalsSmoothingGroups()
{
	CMap <unsigned int,int> CM;
	int *pi;
	int n,tag;

	if (nKeys>0)
	{
		//CreateMCV();
		for (int k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++) Vertices[n].Stok=VerticesKeys[k][n].Stok;
			CalculateNormalsBasic(-1);
			for (n=0;n<nVertices;n++) VerticesKeys[k][n].Norm=Vertices[n].Norm;
		}
		//FreeMCV();
	}

	for (n=0;n<nFaces;n++) CM[Faces[n].tag];

	pi=CM.GetFirst();
	while (pi)
	{
		tag=CM.GetId();
		CalculateNormals(tag);
		pi=CM.GetNext();
	}

	CM.Free();
}

void CObject3D::CalculateNormalsSmoothingGroups2()
{
	CMap <unsigned int,int> CM;
	int *pi;
	int n,tag;

	if (Status==88) return;
    
    //nbrow=ReorderY();

	if (nKeys>0)
	{
		CreateMCV();
		for (int k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++) Vertices[n].Stok=VerticesKeys[k][n].Stok;
			CalculateNormalsBasic(-1);			
			for (n=0;n<nVertices;n++) VerticesKeys[k][n].Norm=Vertices[n].Norm;
		}
		FreeMCV();
	}

	for (n=0;n<nFaces;n++) CM[Faces[n].tag];

	pi=CM.GetFirst();
	while (pi)
	{
		tag=CM.GetId();
		CalculateNormals2a(tag);
		pi=CM.GetNext();
	}

	CM.Free();
}


void FaceNormalePrecalc_allsmoothing(CFace *f,CObject3D *obj)
{
	CVector vec1,vec2,vec3;
	CVector v1,v2,v3,u;
	int n;
	int *tab;

	VECTORSUB(vec1,obj->Vertices[f->v1].Stok,obj->Vertices[f->v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f->v2].Stok,obj->Vertices[f->v0].Stok);
	CROSSPRODUCT(vec3,vec2,vec1);
	VECTORNORMALISE(vec3);
	f->Norm=vec3;

	if (obj->map_corres_vertices)
	{
		tab=(int*) (&(obj->map_corres_vertices[f->v0*32*4]));
		n=0;
		while (tab[n]!=-1)
		{
			int nv=tab[n];
			if (obj->Vertices[nv].tag2) { VECTORADD(obj->Vertices[nv].Norm,obj->Vertices[nv].Norm,vec3); obj->Vertices[nv].tag++;	}
			n++;
		}

		tab=(int*) (&(obj->map_corres_vertices[f->v1*32*4]));
		n=0;
		while (tab[n]!=-1)
		{
			int nv=tab[n];
			if (obj->Vertices[nv].tag2) { VECTORADD(obj->Vertices[nv].Norm,obj->Vertices[nv].Norm,vec3); obj->Vertices[nv].tag++;	}
			n++;
		}

		tab=(int*) (&(obj->map_corres_vertices[f->v2*32*4]));
		n=0;
		while (tab[n]!=-1)
		{
			int nv=tab[n];
			if (obj->Vertices[nv].tag2) { VECTORADD(obj->Vertices[nv].Norm,obj->Vertices[nv].Norm,vec3); obj->Vertices[nv].tag++;	}
			n++;
		}
	}
	else
	{
		v1=obj->Vertices[f->v0].Stok;
		v2=obj->Vertices[f->v1].Stok;
		v3=obj->Vertices[f->v2].Stok;

        for (n=0;n<obj->nVertices;n++)
        {
            if (obj->Vertices[n].tag2!=0)
            {
                int tag=0;
                if (obj->Vertices[f->v0].value==obj->Vertices[n].value)
                {
                    VECTORSUB(u,v1,obj->Vertices[n].Stok);
                    if (VECTORNORM2(u)<SMALLF)
                    {
                        VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                        obj->Vertices[n].tag++;
                        tag=1;
                    }
                }
                
                if (tag==0)
                {
                    if (obj->Vertices[f->v1].value==obj->Vertices[n].value)
                    {
                        VECTORSUB(u,v2,obj->Vertices[n].Stok);
                        if (VECTORNORM2(u)<SMALLF)
                        {
                            VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                            obj->Vertices[n].tag++;
                            tag=1;
                        }
                    }
                    
                    if (tag==0)
                    {
                        if (obj->Vertices[f->v2].value==obj->Vertices[n].value)
                        {
                            VECTORSUB(u,v3,obj->Vertices[n].Stok);
                            if (VECTORNORM2(u)<SMALLF)
                            {
                                VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                                obj->Vertices[n].tag++;
                            }
                        }
                    }
                }
            }
        }
/*
		for (n=0;n<obj->nVertices;n++)
		{
			if (obj->Vertices[n].tag2!=0)
			{
				VECTORSUB(u,v1,obj->Vertices[n].Stok);
				if (VECTORNORM(u)<SMALLF)
				{
					obj->Vertices[n].Norm=obj->Vertices[n].Norm+vec3;
					obj->Vertices[n].tag++;
				}
				else
				{
					VECTORSUB(u,v2,obj->Vertices[n].Stok);
					if (VECTORNORM(u)<SMALLF)
					{
						obj->Vertices[n].Norm=obj->Vertices[n].Norm+vec3;
						obj->Vertices[n].tag++;
					}
					else
					{
						VECTORSUB(u,v3,obj->Vertices[n].Stok);
						if (VECTORNORM(u)<SMALLF)
						{
							obj->Vertices[n].Norm=obj->Vertices[n].Norm+vec3;
							obj->Vertices[n].tag++;
						}
					}
				}
			}
		}
/**/
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::MCVSet(int x,int y)
{
	int *tab=(int*) map_corres_vertices;
	int n;

	n=0;
	while (tab[x*32 + n]!=-1) n++;
	tab[x*32 + n]=y;
	tab[x*32 + n + 1]=-1;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CObject3D::MCVGet(int x,int y)
{
	int *tab=(int*) map_corres_vertices;

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::FreeMCV()
{
	free(map_corres_vertices);
	map_corres_vertices=NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::CreateMCV()
{
	int n1,n2;
	CVector u;
	int * tab;

	map_corres_vertices=(unsigned char*) malloc(nVertices*32*4);
	memset(map_corres_vertices,0,nVertices*32*4);

	tab=(int*) map_corres_vertices;

	for (n1=0;n1<nVertices;n1++) tab[n1*32 + 0]=-1;

	for (n1=0;n1<nVertices;n1++)
	{
		for (n2=n1;n2<nVertices;n2++)
		{
			VECTORSUB(u,Vertices[n1].Stok,Vertices[n2].Stok);
			if (VECTORNORM2(u)<SMALLF)
			{
				MCVSet(n1,n2);
				if (n1!=n2) MCVSet(n2,n1);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateFaceNormals()
{
	CVector vec1,vec2,vec0;
	CFace *f;
	int n;
	float a0,a1,a2;

	for (n=0;n<nFaces;n++)
	{
		f=&(Faces[n]);
		VECTORSUB(vec0,Vertices[f->v1].Stok,Vertices[f->v0].Stok);
		VECTORSUB(vec1,Vertices[f->v2].Stok,Vertices[f->v0].Stok);
		VECTORSUB(vec2,Vertices[f->v2].Stok,Vertices[f->v1].Stok);
		a0=VECTORNORM2(vec0);
		a1=VECTORNORM2(vec1);
		a2=VECTORNORM2(vec2);
		if ((a0<a1)&&(a0<a2))
		{
			CROSSPRODUCT(f->Norm,vec2,vec1); //1:-vec1 2:-vec2 2^1
		}
		else
		{
			if ((a1<a0)&&(a1<a2))
			{
				CROSSPRODUCT(f->Norm,vec2,vec0); //1:vec2 2:-vec0 2^1
			}
			else
			{
				CROSSPRODUCT(f->Norm,vec1,vec0);
			}
		}
		VECTORNORMALISE(f->Norm);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateFaceNormalsCalc()
{
	CVector vec1,vec2,vec0;
	CFace *f;
	int n;
	float a0,a1,a2;

	for (n=0;n<nFaces;n++)
	{
		f=&(Faces[n]);
		VECTORSUB(vec0,Vertices[f->v1].Calc,Vertices[f->v0].Calc);
		VECTORSUB(vec1,Vertices[f->v2].Calc,Vertices[f->v0].Calc);
		VECTORSUB(vec2,Vertices[f->v2].Calc,Vertices[f->v1].Calc);
		a0=VECTORNORM2(vec0);
		a1=VECTORNORM2(vec1);
		a2=VECTORNORM2(vec2);
		if ((a0<a1)&&(a0<a2))
		{
			CROSSPRODUCT(f->NormCalc,vec2,vec1); //1:-vec1 2:-vec2 2^1
		}
		else
		{
			if ((a1<a0)&&(a1<a2))
			{
				CROSSPRODUCT(f->NormCalc,vec2,vec0); //1:vec2 2:-vec0 2^1
			}
			else
			{
				CROSSPRODUCT(f->NormCalc,vec1,vec0);
			}
		}
		VECTORNORMALISE(f->NormCalc);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CObject3D::CalculateNormals2(int tag)
{
	int n;

	if (Status==88) return;

	if (nKeys>0)
	{
		//CreateMCV();
		for (int k=0;k<nKeys;k++)
		{
			for (n=0;n<nVertices;n++) Vertices[n].Stok=VerticesKeys[k][n].Stok;
			CalculateNormalsBasic(-1);
			for (n=0;n<nVertices;n++) VerticesKeys[k][n].Norm=Vertices[n].Norm;
		}
		//FreeMCV();
	}

	if (tag!=-1)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=-1;

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=0;
				Faces[n].v[1]->tag=0;
				Faces[n].v[2]->tag=0;
			}
		}
	}
	else
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	}
    
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==0) Vertices[n].Norm.Init(0,0,0);
	}

	if (tag==-1)
	{
        // RADIUS and APPROX
        
        CVector MINI(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
        CVector MAXI(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
        for (n=0;n<nVertices;n++)
        {
            if (Vertices[n].Stok.x<MINI.x) MINI.x=Vertices[n].Stok.x;
            if (Vertices[n].Stok.y<MINI.y) MINI.y=Vertices[n].Stok.y;
            if (Vertices[n].Stok.z<MINI.z) MINI.z=Vertices[n].Stok.z;
            if (Vertices[n].Stok.x>MAXI.x) MAXI.x=Vertices[n].Stok.x;
            if (Vertices[n].Stok.y>MAXI.y) MAXI.y=Vertices[n].Stok.y;
            if (Vertices[n].Stok.z>MAXI.z) MAXI.z=Vertices[n].Stok.z;
        }
        
        int KK=1024;
        CVector DELTA=MAXI-MINI;
        DELTA.x=KK/DELTA.x;
        DELTA.y=KK/DELTA.y;
        DELTA.z=KK/DELTA.z;
        for (n=0;n<nVertices;n++)
        {
            int xx=(int)((Vertices[n].Stok.x-MINI.x)*DELTA.x);
            int yy=(int)((Vertices[n].Stok.y-MINI.y)*DELTA.y);
            int zz=(int)((Vertices[n].Stok.z-MINI.z)*DELTA.z);
            Vertices[n].value=xx+KK*(yy+KK*zz);
            
            Vertices[n].tag2=1;
        }
        
        // NORMALS
        
		for (n=0;n<nVertices;n++) Vertices[n].tag2=1;
		for (n=0;n<nFaces;n++) FaceNormalePrecalc_allsmoothing(&Faces[n],this);
	}

    for (n=0;n<nVertices;n++) VECTORNORMALISE(Vertices[n].Norm);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateNormalsBasic(int tag)
{
	int n;

	if (tag!=-1)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=-1;

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=0;
				Faces[n].v[1]->tag=0;
				Faces[n].v[2]->tag=0;
			}
		}
	}
	else
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	}

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==0) VECTORINIT(Vertices[n].Norm,0,0,0);
	}

	if (tag==-1)
	{
        // RADIUS and APPROX
        
        CVector MINI(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
        CVector MAXI(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
        for (n=0;n<nVertices;n++)
        {
            if (Vertices[n].Stok.x<MINI.x) MINI.x=Vertices[n].Stok.x;
            if (Vertices[n].Stok.y<MINI.y) MINI.y=Vertices[n].Stok.y;
            if (Vertices[n].Stok.z<MINI.z) MINI.z=Vertices[n].Stok.z;
            if (Vertices[n].Stok.x>MAXI.x) MAXI.x=Vertices[n].Stok.x;
            if (Vertices[n].Stok.y>MAXI.y) MAXI.y=Vertices[n].Stok.y;
            if (Vertices[n].Stok.z>MAXI.z) MAXI.z=Vertices[n].Stok.z;
        }
        
        int KK=1024;
        CVector DELTA=MAXI-MINI;
        DELTA.x=KK/DELTA.x;
        DELTA.y=KK/DELTA.y;
        DELTA.z=KK/DELTA.z;
        for (n=0;n<nVertices;n++)
        {
            int xx=(int)((Vertices[n].Stok.x-MINI.x)*DELTA.x);
            int yy=(int)((Vertices[n].Stok.y-MINI.y)*DELTA.y);
            int zz=(int)((Vertices[n].Stok.z-MINI.z)*DELTA.z);
            Vertices[n].value=xx+KK*(yy+KK*zz);
            
            Vertices[n].tag2=1;
        }

        // NORMALS
        
		for (n=0;n<nFaces;n++) FaceNormalePrecalc_allsmoothing(&Faces[n],this);
	}

    for (n=0;n<nVertices;n++) VECTORNORMALISE(Vertices[n].Norm);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CObject3D::CalculateNormalsSimple(int tag)
{
	int n;

	if (tag!=-1)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=-1;

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=0;
				Faces[n].v[1]->tag=0;
				Faces[n].v[2]->tag=0;
			}
		}
	}
	else
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	}

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==0) VECTORINIT(Vertices[n].Norm,0,0,0);
	}

	if (tag==-1)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag2=1;
		for (n=0;n<nFaces;n++) FaceNormalePrecalc(&Faces[n],this);
	}

    for (n=0;n<nVertices;n++) VECTORNORMALISE(Vertices[n].Norm);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FaceNormalePrecalc_allsmoothing_a(CFace *f,CObject3D *obj)
{
	CVector vec1,vec2,vec3;
	CVector u;
	int n;

	VECTORSUB(vec1,obj->Vertices[f->v1].Stok,obj->Vertices[f->v0].Stok);
	VECTORSUB(vec2,obj->Vertices[f->v2].Stok,obj->Vertices[f->v0].Stok);

	CROSSPRODUCT(vec3,vec2,vec1);

	VECTORNORMALISE(vec3);

	f->Norm=vec3;
    
    int val1=obj->Vertices[f->v0].value;
    int val2=obj->Vertices[f->v1].value;
    int val3=obj->Vertices[f->v2].value;
    
    int st=0;
    int nd=obj->nVertices;
    
    if (obj->nbrow>=0)
    {
        st=f->v0;
        if (f->v1<st) st=f->v1;
        if (f->v2<st) st=f->v2;

        nd=f->v0;
        if (f->v1>nd) nd=f->v1;
        if (f->v2>nd) nd=f->v2;

        nd+=obj->nbrow;
        if (nd>obj->nVertices) nd=obj->nVertices;
        
        st-=obj->nbrow;
        if (st<0) st=0;
    }

	for (n=st;n<nd;n++)
	{
		if (obj->Vertices[n].tag2!=0)
		{
            int tag=0;
            if (val1==obj->Vertices[n].value)
            {
                VECTORSUB(u,obj->Vertices[f->v0].Stok,obj->Vertices[n].Stok);
                if (VECTORNORM2(u)<SMALLF)
                {
                    VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                    obj->Vertices[n].tag++;
                    tag=1;
                }
            }
			
            if (tag==0)
			{
                if (val2==obj->Vertices[n].value)
                {
                    VECTORSUB(u,obj->Vertices[f->v1].Stok,obj->Vertices[n].Stok);
                    if (VECTORNORM2(u)<SMALLF)
                    {
                        VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                        obj->Vertices[n].tag++;
                        tag=1;
                    }
                }
                
				if (tag==0)
				{
                    if (val3==obj->Vertices[n].value)
                    {
                        VECTORSUB(u,obj->Vertices[f->v2].Stok,obj->Vertices[n].Stok);
                        if (VECTORNORM2(u)<SMALLF)
                        {
                            VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                            obj->Vertices[n].tag++;
                        }
                    }
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FaceNormalePrecalc_allsmoothing_a_grid(CFace *f,CObject3D *obj)
{
    CVector vec1,vec2,vec3;
    CVector u;
    int n;

    VECTORSUB(vec1,obj->Vertices[f->v1].Stok,obj->Vertices[f->v0].Stok);
    VECTORSUB(vec2,obj->Vertices[f->v2].Stok,obj->Vertices[f->v0].Stok);

    CROSSPRODUCT(vec3,vec2,vec1);

    VECTORNORMALISE(vec3);

    f->Norm=vec3;
    
    int val1=obj->Vertices[f->v0].value;
    int val2=obj->Vertices[f->v1].value;
    int val3=obj->Vertices[f->v2].value;
    
    int st=0;
    int nd=obj->nVertices;
    
    if (obj->nbrow>=0)
    {
        st=f->v0;
        if (f->v1<st) st=f->v1;
        if (f->v2<st) st=f->v2;

        nd=f->v0;
        if (f->v1>nd) nd=f->v1;
        if (f->v2>nd) nd=f->v2;

        nd+=obj->nbrow;
        if (nd>obj->nVertices) nd=obj->nVertices;
        
        st-=obj->nbrow;
        if (st<0) st=0;
    }

    
    CVector g;
    VECTORADDDIV3(g,obj->Vertices[f->v0].Stok,obj->Vertices[f->v1].Stok,obj->Vertices[f->v2].Stok);
    
    int ofs=obj->grid->Offset(g);
    
    int *pi=obj->grid->grid[ofs].GetFirst();
    while (pi)
    {
        n=*pi;
        
        if ((n>=st)&&(n<nd))
        if (obj->Vertices[n].tag2!=0)
        {
            int tag=0;
            if (val1==obj->Vertices[n].value)
            {
                VECTORSUB(u,obj->Vertices[f->v0].Stok,obj->Vertices[n].Stok);
                if (VECTORNORM2(u)<SMALLF)
                {
                    VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                    obj->Vertices[n].tag++;
                    tag=1;
                }
            }
            
            if (tag==0)
            {
                if (val2==obj->Vertices[n].value)
                {
                    VECTORSUB(u,obj->Vertices[f->v1].Stok,obj->Vertices[n].Stok);
                    if (VECTORNORM2(u)<SMALLF)
                    {
                        VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                        obj->Vertices[n].tag++;
                        tag=1;
                    }
                }
                
                if (tag==0)
                {
                    if (val3==obj->Vertices[n].value)
                    {
                        VECTORSUB(u,obj->Vertices[f->v2].Stok,obj->Vertices[n].Stok);
                        if (VECTORNORM2(u)<SMALLF)
                        {
                            VECTORADD(obj->Vertices[n].Norm,obj->Vertices[n].Norm,vec3);
                            obj->Vertices[n].tag++;
                        }
                    }
                }
            }
        }

        pi=obj->grid->grid[ofs].GetNext();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::CalculateNormals2a(int tag)
{
	int n,nn;
    
	if (tag!=-1)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=-1;

		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=0;
				Faces[n].v[1]->tag=0;
				Faces[n].v[2]->tag=0;
			}
		}
	}
	else
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	}
    
    // RADIUS and APPROX
    
    CVector MINI(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    CVector MAXI(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
    for (n=0;n<nVertices;n++)
    {
        if (Vertices[n].Stok.x<MINI.x) MINI.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y<MINI.y) MINI.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z<MINI.z) MINI.z=Vertices[n].Stok.z;
        if (Vertices[n].Stok.x>MAXI.x) MAXI.x=Vertices[n].Stok.x;
        if (Vertices[n].Stok.y>MAXI.y) MAXI.y=Vertices[n].Stok.y;
        if (Vertices[n].Stok.z>MAXI.z) MAXI.z=Vertices[n].Stok.z;
    }
    
    int KK=1024;
    CVector DELTA=MAXI-MINI;
    DELTA.x=KK/DELTA.x;
    DELTA.y=KK/DELTA.y;
    DELTA.z=KK/DELTA.z;
    for (n=0;n<nVertices;n++)
    {
        int xx=(int)((Vertices[n].Stok.x-MINI.x)*DELTA.x);
        int yy=(int)((Vertices[n].Stok.y-MINI.y)*DELTA.y);
        int zz=(int)((Vertices[n].Stok.z-MINI.z)*DELTA.z);
        Vertices[n].value=xx+KK*(yy+KK*zz);
    }

    // CALCULZTE NORMALS SMOOTHING
    
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag==0) Vertices[n].Norm.Init(0,0,0);
	}

    for (nn=0;nn<nVertices;nn++) Vertices[nn].tag2=0;

    for (nn=0;nn<nFaces;nn++)
    {
        if ((Faces[nn].tag==tag)||(tag==-1))
        {
            Faces[nn].v[0]->tag2=1;
            Faces[nn].v[1]->tag2=1;
            Faces[nn].v[2]->tag2=1;
        }
    }
    
    if (nFaces<32768)
    {
        for (n=0;n<nFaces;n++)
        {
            if ((Faces[n].tag==tag)||(tag==-1))
            {
                FaceNormalePrecalc_allsmoothing_a(&Faces[n],this);
            }
        }
    }
    else
    {
        grid=new GridObject3D;
        
        grid->Affect(this);

        for (n=0;n<nFaces;n++)
        {
            if ((Faces[n].tag==tag)||(tag==-1))
            {
                FaceNormalePrecalc_allsmoothing_a_grid(&Faces[n],this);
            }
        }
        
        delete grid;
    }

    for (n=0;n<nVertices;n++)
	{
		VECTORNORMALISE(Vertices[n].Norm);
	}

}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		CSphereTree* CObject3D::CalculateSphereTree()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


#ifdef _DEFINES_API_CODE_PHYSIC_
CSphereTree* CObject3D::CalculateSphereTree()
{
	CObject3D *loaded=this;
	CSphereTree *sph;
	int n;
	int res;
	CVector p;
	CMatrix M,Mobj;
	CMatrix Ms,I;
	int GROS;
	int nobj;
	float RayonMinimal;
	CObject3D * o;
	CObject3D *otmp1;
	int n1,n2,np;
	int t0,t1,t2;
	CVector p0,p1,p2,p3,c,D;
	float Rayon;
	int algo;
	float K=1.4f;
	float ep=0.1f;
	float GH=0.8f;
	CBSPVolumic bsp;
	CObject3D objet[16],obj,obj0,obj1,objet_a_calculer,tmpobj,obj00;
	float ss;
	float r,rr;
	bool exist;
	float fp;
	float RayonMax;
	float r1,r2;
	int i;
	CVector c1,c2,c3,c4,c5,c6;
	int NINTERVALLES=10;
	bool optimizated;

	sph=new CSphereTree,

	sph->Spheres=(Ball*) malloc(sizeof(Ball)*15000);

	obj=(loaded->Duplicate());//.Inverse();
	obj.Centered();
	for (n=0;n<obj.nFaces;n++) obj.Faces[n].tag=0;
	objet_a_calculer=obj.Regroupe();


	r=objet_a_calculer.CalculateRadius();

	for (n=0;n<objet_a_calculer.nFaces;n++) objet_a_calculer.Faces[n].tag=0;

	GROS=0;

	objet_a_calculer.CalculateNormals(-1);
	RayonMax=objet_a_calculer.CalculateRadius();
	GROS=objet_a_calculer.SetTag(SET_FACES_COPLANAR_RECURS_TAG);

	rr=objet_a_calculer.CalculateRadius()*0.05f;
	RayonMinimal=objet_a_calculer.CalculateRadius()/70;

	if (objet_a_calculer.nVertices<=100)
	{
		objet[4]=objet_a_calculer.TesselateLength(rr);
		nobj=0;
		for (n=4;n>0;n--)
		{
			if (objet[n].nVertices>500)
			{
				if (n<=3) nobj=n;
				else nobj=-1;
				break;
			}

			objet[n-1]=objet[n].TesselateLength(rr);
		}
	}
	sph->nB=0;


	optimizated=false;
	if ((objet_a_calculer.nVertices>100)||(nobj==-1))
	{
		algo=2;
		otmp1=o=&objet_a_calculer;
		while ((o->nVertices>650)&&(otmp1!=NULL))
		{
			optimizated=true;
			otmp1=o->Duplicate2();
			if ((o!=&objet_a_calculer)&&(otmp1!=NULL))
			{
				o->Free();
				delete o;
			}
			if (otmp1!=NULL) o=otmp1;
		}
	}
	else
	{
		o=&objet[nobj];
		algo=1;
		if (nobj!=0) algo=3;
	}


	bsp.Build(&objet_a_calculer,1,1);
	o->CalculateNormals(-1);

	GROS=o->SetTag(SET_FACES_COPLANAR_RECURS_TAG);
	np=o->nVertices;

	for (n=0;n<np;n++) o->Vertices[n].tag=0;



	for (n=0;n<o->nFaces;n++)
	{
		if (o->Faces[n].v[0]->tag!=o->Faces[n].tag)
		{
			if (o->Faces[n].v[0]->tag!=0) o->Faces[n].v[0]->tag=o->Faces[n].v[0]->tag<<8;
			o->Faces[n].v[0]->tag+=o->Faces[n].tag;
		}

		if (o->Faces[n].v[1]->tag!=o->Faces[n].tag)
		{
			if (o->Faces[n].v[1]->tag!=0) o->Faces[n].v[1]->tag=o->Faces[n].v[1]->tag<<8;
			o->Faces[n].v[1]->tag+=o->Faces[n].tag;
		}

		if (o->Faces[n].v[2]->tag!=o->Faces[n].tag)
		{
			if (o->Faces[n].v[2]->tag!=0) o->Faces[n].v[2]->tag=o->Faces[n].v[2]->tag<<8;
			o->Faces[n].v[2]->tag+=o->Faces[n].tag;
		}
	}


	for (i=NINTERVALLES-1;i>=0;i--)
	{
		r1=RayonMinimal+i*(RayonMax-RayonMinimal)/NINTERVALLES;
		r2=RayonMinimal+(i+1)*(RayonMax-RayonMinimal)/NINTERVALLES;
		for (n1=0;n1<np-1;n1++)
		{
			p0=o->Vertices[n1].Stok;
			t0=o->Vertices[n1].tag;

			if ((t0<GROS)||(algo>1))
			for (n2=n1+1;n2<np;n2++)
			{
				p1=o->Vertices[n2].Stok;
				D=p0-p1;

				t1=o->Vertices[n2].tag;

				if ((t1<GROS)||(algo==2))
				{

					if (t0!=t1)
					{

						c=(p0+p1)/2;
						D=c-p0;
						Rayon=D.Norme();

						if ((bsp.Inside(c))&&(Rayon>r1)&&(Rayon<r2))//&&
						{
							c1=c2=c3=c4=c5=c6=c;
							{

								Rayon=Rayon*Rayon;

								for (n=0;n<np;n++)
								{
									t2=o->Vertices[n].tag;
									if ((t2<GROS)||(algo==2))
									{
										D=c-o->Vertices[n].Stok;
										ss=D.x*D.x + D.y*D.y + D.z*D.z;
										if (ss<Rayon) Rayon=ss;
									}
								}

								Rayon=(float) sqrt(Rayon);

								c1.x+=Rayon*GH;
								c2.x+=-Rayon*GH;
								c3.y+=Rayon*GH;
								c4.y+=-Rayon*GH;
								c5.z+=Rayon*GH;
								c6.z+=-Rayon*GH;


								if ((Rayon>RayonMinimal))
								{
									if (bsp.Inside(c1))
									if (bsp.Inside(c2))
									if (bsp.Inside(c3))
									if (bsp.Inside(c4))
									if (bsp.Inside(c5))
									if (bsp.Inside(c6))
									{
										exist=true;
										res=-1;
										ss=(RayonMinimal)*(RayonMinimal)*16;

										for (n=0;n<sph->nB;n++)
										{
											fp=(float) ((sph->Spheres[n].x-c.x)*(sph->Spheres[n].x-c.x) + (sph->Spheres[n].y-c.y)*(sph->Spheres[n].y-c.y) + (sph->Spheres[n].z-c.z)*(sph->Spheres[n].z-c.z));
											if (fp<ss)
											{
												exist=false;
												break;
											}
										}

										if (exist)
										{
											sph->Spheres[sph->nB].x=c.x;
											sph->Spheres[sph->nB].y=c.y;
											sph->Spheres[sph->nB].z=c.z;
											sph->Spheres[sph->nB].r=Rayon;
											sph->Spheres[sph->nB].b0=-1;
											sph->Spheres[sph->nB].b1=-1;
											sph->Spheres[sph->nB].b2=-1;
											sph->Spheres[sph->nB].tag=0;
											sph->nB++;
										}
									}
								}
							}
						}

					}
				}

				if (sph->nB>15000) break;

			}
		}

	}
	bsp.Free();

	sph->Nmax=sph->Recurs(0,1);

	obj.Free();
	objet_a_calculer.Free();

	if (nobj>=0)
	for (n=nobj;n<4;n++) objet[n].Free();

	if (optimizated)
	{
		o->Free();
		delete o;
	}

	return sph;
}

#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::TagFacesClose()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::TagSelectingFacesClose()
{
	int n,res,tag,d;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	SetF012();

	if (TAG)
	{
        for (n=0;n<nFaces;n++)
            if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=0;
	}

	return tag;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::TagFacesClose()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int CObject3D::TagSelectingFacesCloseNear()
{
	int n,res,tag,d;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	SetF012Near();

	if (TAG)
	{
        for (n=0;n<nFaces;n++)
            if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=0;
	}

	return tag;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		  void CObject3D::SphericalMapping()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::SphericalMapping()
{
	float x,y,z;
	float a,b;
	float aa[3],bb[3];
	int n;
	int tag=0;

	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) tag=1; }
	if (tag) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (int nf=0;nf<nFaces;nf++)
	{
		if (Faces[nf].tag)
		{
			n=Faces[nf].v0;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			b=(float) asinf(y/sqrtf(x*x+y*y+z*z));
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI + a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a/=2*PI;
			b=(b+PI/2)/PI;
			aa[0]=a; bb[0]=b;

			n=Faces[nf].v1;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			b=(float) asinf(y/sqrtf(x*x+y*y+z*z));
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI + a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a/=2*PI;
			b=(b+PI/2)/PI;
			aa[1]=a; bb[1]=b;

			n=Faces[nf].v2;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			b=(float) asinf(y/sqrtf(x*x+y*y+z*z));
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI + a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a/=2*PI;
			b=(b+PI/2)/PI;
			aa[2]=a; bb[2]=b;

			if ((aa[0]>0.85f)||(aa[1]>0.85f)||(aa[2]>0.85f))
			{
				if (aa[0]<0.15f) aa[0]+=1.0f;
				if (aa[1]<0.15f) aa[1]+=1.0f;
				if (aa[2]<0.15f) aa[2]+=1.0f;
			}

			Vertices[Faces[nf].v0].Map.Init(aa[0],bb[0]);
			Vertices[Faces[nf].v1].Map.Init(aa[1],bb[1]);
			Vertices[Faces[nf].v2].Map.Init(aa[2],bb[2]);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::MappingBoundsFixed(0..1..2)
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::MappingBoundsFixed(int tag)
{
    int n;
    CVector p,mini,maxi;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    maxi.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
    mini.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        p=Vertices[n].Stok;
        if (p.x<mini.x) mini.x=p.x;
        if (p.y<mini.y) mini.y=p.y;
        if (p.z<mini.z) mini.z=p.z;
        if (p.x>maxi.x) maxi.x=p.x;
        if (p.y>maxi.y) maxi.y=p.y;
        if (p.z>maxi.z) maxi.z=p.z;
    }
    
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        p=Vertices[n].Stok;

        if (tag==0)
        {
            Vertices[n].Map.x=(p.x-mini.x)/(maxi.x-mini.x);
            Vertices[n].Map.y=(p.z-mini.z)/(maxi.z-mini.z);
        }
        
        if (tag==1)
        {
            Vertices[n].Map.x=(p.y-mini.y)/(maxi.y-mini.y);
            Vertices[n].Map.y=(p.z-mini.z)/(maxi.z-mini.z);
        }

        if (tag==2)
        {
            Vertices[n].Map.x=(p.x-mini.x)/(maxi.x-mini.x);
            Vertices[n].Map.y=(p.y-mini.y)/(maxi.y-mini.y);
        }
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::RotateLeftMap()
 void CObject3D::RotateRightMap()
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::RotateLeftMap()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        CVector2 m=Vertices[n].Map;
        
        Vertices[n].Map.x=m.y;
        Vertices[n].Map.y=1.0f-m.x;
    }
}

void CObject3D::RotateRightMap()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        CVector2 m=Vertices[n].Map;
        
        Vertices[n].Map.x=1.0f-m.y;
        Vertices[n].Map.y=m.x;
    }
}

void CObject3D::RotateLeftMapL()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    CVector2 mini,maxi;
    maxi.Init(-MAXI_BOUND,-MAXI_BOUND);
    mini.Init(MAXI_BOUND,MAXI_BOUND);
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
	{
        if (mini.x>Vertices[n].Map.x) mini.x=Vertices[n].Map.x;
		if (mini.y>Vertices[n].Map.y) mini.y=Vertices[n].Map.y;
        if (maxi.x<Vertices[n].Map.x) maxi.x=Vertices[n].Map.x;
		if (maxi.y<Vertices[n].Map.y) maxi.y=Vertices[n].Map.y;
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        CVector2 m;
        m.x=(Vertices[n].Map.x-mini.x)/(maxi.x-mini.x);
        m.y=(Vertices[n].Map.y-mini.y)/(maxi.y-mini.y);
	    Vertices[n].Map.x=mini.x+(maxi.x-mini.x)*m.y;
        Vertices[n].Map.y=mini.y+(maxi.y-mini.y)*(1.0f-m.x);
    }
}

void CObject3D::RotateRightMapL()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    CVector2 mini,maxi;
    maxi.Init(-MAXI_BOUND,-MAXI_BOUND);
    mini.Init(MAXI_BOUND,MAXI_BOUND);

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
	{
        if (mini.x>Vertices[n].Map.x) mini.x=Vertices[n].Map.x;
		if (mini.y>Vertices[n].Map.y) mini.y=Vertices[n].Map.y;
        if (maxi.x<Vertices[n].Map.x) maxi.x=Vertices[n].Map.x;
		if (maxi.y<Vertices[n].Map.y) maxi.y=Vertices[n].Map.y;
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        CVector2 m;
        m.x=(Vertices[n].Map.x-mini.x)/(maxi.x-mini.x);
        m.y=(Vertices[n].Map.y-mini.y)/(maxi.y-mini.y);
	    Vertices[n].Map.x=mini.x+(maxi.x-mini.x)*(1.0f-m.y);
        Vertices[n].Map.y=mini.y+(maxi.y-mini.y)*m.x;
    }
}

void CObject3D::RotateMapL(float deg)
{
    int n;
	float c=cosf(deg*PI/180.0f);
	float s=sinf(deg*PI/180.0f);
	float sq2=sqrtf(2.0f);

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    CVector2 mini,maxi;
    maxi.Init(-MAXI_BOUND,-MAXI_BOUND);
    mini.Init(MAXI_BOUND,MAXI_BOUND);
    
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
	{
        if (mini.x>Vertices[n].Map.x) mini.x=Vertices[n].Map.x;
		if (mini.y>Vertices[n].Map.y) mini.y=Vertices[n].Map.y;
        if (maxi.x<Vertices[n].Map.x) maxi.x=Vertices[n].Map.x;
		if (maxi.y<Vertices[n].Map.y) maxi.y=Vertices[n].Map.y;
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
    {
        CVector2 tm,m;
        tm.x=(Vertices[n].Map.x-(maxi.x+mini.x)/2)/(maxi.x-mini.x);
        tm.y=(Vertices[n].Map.y-(maxi.y+mini.y)/2)/(maxi.y-mini.y);

		m.x=(c*tm.x + s*tm.y)/sq2;
		m.y=(-s*tm.x + c*tm.y)/sq2;

	    Vertices[n].Map.x=(maxi.x+mini.x)/2+m.x*(maxi.x-mini.x);
        Vertices[n].Map.y=(maxi.y+mini.y)/2+m.y*(maxi.y-mini.y);
    }
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::InvertXMap()
 void CObject3D::InvertYMap()
 
 void CObject3D::InvertXMapL()
 void CObject3D::InvertYMapL()
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::InvertXMap()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
        Vertices[n].Map.x=1.0f-Vertices[n].Map.x;
}

void CObject3D::InvertYMap()
{
    int n;
    
    int TAG=0;
    for (n=0;n<nFaces;n++) Faces[n].tag=1;
    for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
    if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }
    
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag)
        {
            Faces[n].v[0]->tag=1;
            Faces[n].v[1]->tag=1;
            Faces[n].v[2]->tag=1;
        }
    }
    
    for (n=0;n<nVertices;n++)
        if (Vertices[n].tag)
            Vertices[n].Map.y=1.0f-Vertices[n].Map.y;
}

void CObject3D::InvertXMapL()
{
    int n;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

	float xpd=-MAXI_BOUND;
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
        if (xpd<Vertices[n].Map.x) xpd=Vertices[n].Map.x;

    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
        Vertices[n].Map.x=xpd-Vertices[n].Map.x;
}

void CObject3D::InvertYMapL()
{
    int n;
    
    int TAG=0;
    for (n=0;n<nFaces;n++) Faces[n].tag=1;
    for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
    if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }
    
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag)
        {
            Faces[n].v[0]->tag=1;
            Faces[n].v[1]->tag=1;
            Faces[n].v[2]->tag=1;
        }
    }

	float ypd=-MAXI_BOUND;
    for (n=0;n<nVertices;n++)
	if (Vertices[n].tag)
        if (ypd<Vertices[n].Map.y) ypd=Vertices[n].Map.y;
    
    for (n=0;n<nVertices;n++)
        if (Vertices[n].tag)
            Vertices[n].Map.y=ypd-Vertices[n].Map.y;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::MappingScale(float Kx,float Ky)
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::MappingScale(float Kx,float Ky)
{
    int n;
    
    int TAG=0;
    for (n=0;n<nFaces;n++) Faces[n].tag=1;
    for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
    if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }
    
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag)
        {
            Faces[n].v[0]->tag=1;
            Faces[n].v[1]->tag=1;
            Faces[n].v[2]->tag=1;
        }
    }
    
    for (n=0;n<nVertices;n++)
        if (Vertices[n].tag)
        {
            Vertices[n].Map.x*=Kx;
            Vertices[n].Map.y*=Ky;
        }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBoundXYZ(CVector min,CVector max)
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingBoundXYZ(CVector min,CVector max)
{
	float bx=max.x-min.x;
	float by=max.y-min.y;
	float bz=max.z-min.z;
	float a,b;
	int n;
	CVector v;

	if (bx<0) bx=-bx;
	if (by<0) by=-by;
	if (bz<0) bz=-bz;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag)
		{
            a=(Vertices[n].Calc.x-min.x)/(max.x-min.x);
            b=(Vertices[n].Calc.z-min.z)/(max.z-min.z);

            if ((bx<by)&&(bx<bz))
			{
				a=(Vertices[n].Calc.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Calc.y-min.y)/(max.y-min.y);
			}

			if ((by<bx)&&(by<bz))
			{
				a=(Vertices[n].Calc.x-min.x)/(max.x-min.x);
				b=1.0f-(Vertices[n].Calc.z-min.z)/(max.z-min.z);
			}

			if ((bz<by)&&(bz<bx))
			{
				a=(Vertices[n].Calc.x-min.x)/(max.x-min.x);
				b=(Vertices[n].Calc.y-min.y)/(max.y-min.y);
			}

			Vertices[n].Map.Init(a,b);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBound()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingBound()
{
	float x,y,z;
	float a,b;
	int n;
	CVector min,max,v;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag)
		{
			v=Vertices[n].Stok;

			if (v.x>max.x) max.x=v.x;
			if (v.y>max.y) max.y=v.y;
			if (v.z>max.z) max.z=v.z;

			if (v.x<min.x) min.x=v.x;
			if (v.y<min.y) min.y=v.y;
			if (v.z<min.z) min.z=v.z;
		}
	}

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag)
		{
			x=f_abs(Vertices[n].Norm.x);
			y=f_abs(Vertices[n].Norm.y);
			z=f_abs(Vertices[n].Norm.z);

			if ((x>y)&&(x>z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
			}

			if ((y>x)&&(y>z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
			}

			if ((z>y)&&(z>x))
			{
				a=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
			}

			Vertices[n].Map.Init(a,b);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	//for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	//for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBoundOrtho()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingBoundOrtho()
{
	float a,b;
	int n;
	CVector min,max,v;
	CPOBGenerator GENPOB;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }
	if (TAG) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag)
		{
			Faces[n].v[0]->tag=1;
			Faces[n].v[1]->tag=1;
			Faces[n].v[2]->tag=1;
		}
	}

	CPOB *pob=GENPOB.POBFrom2(this,1,0);
	CVector u=pob->Ux;
	CVector u1=pob->Uy;
	CVector u2=pob->Uz;
	delete pob;

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag)
		{
			v.x=DOT(Vertices[n].Stok,u);
			v.y=DOT(Vertices[n].Stok,u1);
			v.z=DOT(Vertices[n].Stok,u2);

			if (v.x>max.x) max.x=v.x;
			if (v.y>max.y) max.y=v.y;
			if (v.z>max.z) max.z=v.z;

			if (v.x<min.x) min.x=v.x;
			if (v.y<min.y) min.y=v.y;
			if (v.z<min.z) min.z=v.z;
		}
	}

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].tag)
		{
			v.x=DOT(Vertices[n].Stok,u);
			v.y=DOT(Vertices[n].Stok,u1);
			v.z=DOT(Vertices[n].Stok,u2);

			a=(v.x-min.x)/(max.x-min.x);

			if (u1.y>u2.y) b=(v.y-min.y)/(max.y-min.y);
			else b=(v.z-min.z)/(max.z-min.z);

			Vertices[n].Map.Init(a,b);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	//for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	//for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void setTEX(unsigned char *grid,int x,int y,int sx,int sy,int dim)
{
	int adr=x+y*dim;

	switch (sx)
	{
	case 1:
		grid[adr]=1;
		break;
	case 2:
		*(unsigned short int *)(&grid[adr])=0x0101;
		*(unsigned short int *)(&grid[adr+dim])=0x0101;
		break;
	case 4:
		*(unsigned int *)(&grid[adr])=0x01010101;
		*(unsigned int *)(&grid[adr+dim])=0x01010101;
		*(unsigned int *)(&grid[adr+(dim<<1)])=0x01010101;
		*(unsigned int *)(&grid[adr+dim+(dim<<1)])=0x01010101;
		break;

	}
}

unsigned int seekTEX(unsigned char *grid,int sx,int sy,int dim)
{
	unsigned int adr=0xFFFFFFFF;
	unsigned int pos;
	int x,y;
	int add=sy*dim;

	pos=0;
	y=0;
	while ((y<dim)&&(adr==0xFFFFFFFF))
	{
		x=0;
		while ((x<dim)&&(adr==0xFFFFFFFF))
		{
			if (grid[pos+x]==0) adr=x+(y<<16);
			x+=sx;
		}
		y+=sy;
		pos+=add;
	}
	return adr;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBoundPainting()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingBoundPainting(int texw,int texh)
{
	int tag;
	float size=0.0f;
	float K1=1.0f;
	float K2=2.0f;
	int n;
	int ntag=0;
	CVector u0,u1,u2;

	nT=1;

	SetFaces();
	CVector *array=CalculateLightmapCoordinates(1);

	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag>ntag) ntag=Faces[n].tag;
	}

	float moy=0.0f;
	n=0;
	for (tag=1;tag<=ntag;tag++)
	{
		u0=array[tag*6+0];
		VECTORSUB(u1,array[tag*6+1],u0);
		VECTORSUB(u2,array[tag*6+2],u0);
		size=(u1.Norme()+u2.Norme())/2;
		moy+=size;
		n++;
	}
	if (n>0) moy/=n;

	int nb1=0;
	int nb2=0;
	int nb4=0;

	for (tag=1;tag<=ntag;tag++)
	{
		u0=array[tag*6+0];
		VECTORSUB(u1,array[tag*6+1],u0);
		VECTORSUB(u2,array[tag*6+2],u0);
		size=(u1.Norme()+u2.Norme())/2;
		if (size>moy*K2) nb4++;
		else
		if (size>moy*K1) nb2++;
		else nb1++;
	}

	float aire=1.0f/(nb1 + nb2*4 + nb4*16);
	float d=(float) sqrt(aire);
	float x=0.0f;
	float y=0.0f;
	int x0,y0;
	unsigned int adr;
	int dim=1 + (int) (1.0f/d);
	dim=dim +(dim&1);
	float dx,dy;
	unsigned char *grid=(unsigned char *) malloc(dim*dim);
	memset(grid,0,dim*dim);

	d=1.0f/dim;

	for (tag=1;tag<=ntag;tag++)
	{
		u0=array[tag*6+0];
		VECTORSUB(u1,array[tag*6+1],u0);
		VECTORSUB(u2,array[tag*6+2],u0);
		size=(u1.Norme()+u2.Norme())/2;
		if (size>moy*K2)
		{
			adr=seekTEX(grid,4,4,dim);
			x0=(adr&0xFFFF);
			y0=((adr>>16)&0xFFFF);
			setTEX(grid,x0,y0,4,4,dim);
			x=d*x0;
			y=d*y0;
			dx=dy=d*4;
		}
		else
		if (size>moy*K1)
		{
			adr=seekTEX(grid,2,2,dim);
			x0=(adr&0xFFFF);
			y0=((adr>>16)&0xFFFF);
			setTEX(grid,x0,y0,2,2,dim);
			x=d*x0;
			y=d*y0;
			dx=dy=d*2;
		}
		else
		{
			adr=seekTEX(grid,1,1,dim);
			x0=(adr&0xFFFF);
			y0=((adr>>16)&0xFFFF);
			setTEX(grid,x0,y0,1,1,dim);
			x=d*x0;
			y=d*y0;
			dx=dy=d;
		}

		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=1;
				Faces[n].v[1]->tag=1;
				Faces[n].v[2]->tag=1;
			}
		}
        
        float xs=1.0f/texw;
        float ys=1.0f/texh;

		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag)
			{
				if (texw>0) Vertices[n].Map.x=Vertices[n].Map2.x*(dx-xs*2) + x + xs;
                else Vertices[n].Map.x=Vertices[n].Map2.x*dx + x;
				if (texh>0) Vertices[n].Map.y=Vertices[n].Map2.y*(dy-ys*2) + y + ys;
                else Vertices[n].Map.y=Vertices[n].Map2.y*dy + y;
			}
		}

	}

	free(grid);
	delete [] array;

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

	update=true;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBoundPaintingAvg()
 
 
    Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

char* CObject3D::PlanarMappingBoundPaintingAvg(int texw,int texh)
{
    int tag;
    float size=0.0f;
    float K1=1.0f;
    float K2=2.0f;
    int n;
    int ntag=0;
    CVector u0,u1,u2;

    nT=1;

    SetFaces();
    CVector *array=CalculateLightmapCoordinatesAvg(1);

    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag>ntag) ntag=Faces[n].tag;
    }

    float moy=0.0f;
    n=0;
    for (tag=1;tag<=ntag;tag++)
    {
        u0=array[tag*6+0];
        VECTORSUB(u1,array[tag*6+1],u0);
        VECTORSUB(u2,array[tag*6+2],u0);
        size=(u1.Norme()+u2.Norme())/2;
        moy+=size;
        n++;
    }
    moy/=n;

    int nb1=0;
    int nb2=0;
    int nb4=0;

    for (tag=1;tag<=ntag;tag++)
    {
        u0=array[tag*6+0];
        VECTORSUB(u1,array[tag*6+1],u0);
        VECTORSUB(u2,array[tag*6+2],u0);
        size=(u1.Norme()+u2.Norme())/2;
        nb1++;
        /*
        if (size>moy*K2) nb4++;
        else
        if (size>moy*K1) nb2++;
        else nb1++;
        /**/
    }

    float aire=1.0f/(nb1 + nb2*4 + nb4*16);
    float d=(float) sqrt(aire);
    float x=0.0f;
    float y=0.0f;
    int x0,y0;
    unsigned int adr;
    int dim=1 + (int) (1.0f/d);
    //dim=dim +(dim&1);
    float dx,dy;
    unsigned char *grid=(unsigned char *) malloc(dim*dim);
    memset(grid,0,dim*dim);

    d=1.0f/dim;
    
    char * ptrimg=(char*)malloc(2048*2048*4);
    memset(ptrimg,255,2048*2048*4);

    for (tag=1;tag<=ntag;tag++)
    {
        u0=array[tag*6+0];

        VECTORSUB(u1,array[tag*6+1],u0);
        VECTORSUB(u2,array[tag*6+2],u0);
        size=(u1.Norme()+u2.Norme())/2;
        /*
        if (size>moy*K2)
        {
            adr=seekTEX(grid,4,4,dim);
            x0=(adr&0xFFFF);
            y0=((adr>>16)&0xFFFF);
            setTEX(grid,x0,y0,4,4,dim);
            x=d*x0;
            y=d*y0;
            dx=dy=d*4;
        }
        else
        if (size>moy*K1)
        {
            adr=seekTEX(grid,2,2,dim);
            x0=(adr&0xFFFF);
            y0=((adr>>16)&0xFFFF);
            setTEX(grid,x0,y0,2,2,dim);
            x=d*x0;
            y=d*y0;
            dx=dy=d*2;
        }
        else
        {/**/
            adr=seekTEX(grid,1,1,dim);
            x0=(adr&0xFFFF);
            y0=((adr>>16)&0xFFFF);
            setTEX(grid,x0,y0,1,1,dim);
            x=d*x0;
            y=d*y0;
            dx=dy=d;
        //}

        for (n=0;n<nVertices;n++) Vertices[n].tag=0;
        for (n=0;n<nFaces;n++)
        {
            if (Faces[n].tag==tag)
            {
                Faces[n].v[0]->tag=1;
                Faces[n].v[1]->tag=1;
                Faces[n].v[2]->tag=1;
            }
        }
        
        float xs=1.0f/texw;
        float ys=1.0f/texh;
        
        CVector2 mini,maxi;
        maxi.Init(-MAXI_BOUND,-MAXI_BOUND);
        mini.Init(MAXI_BOUND,MAXI_BOUND);

        for (n=0;n<nVertices;n++)
        {
            if (Vertices[n].tag)
            {
                if (texw>0) Vertices[n].Map.x=Vertices[n].Map2.x*(dx-xs*2) + x + xs;
                else Vertices[n].Map.x=Vertices[n].Map2.x*dx + x;
                if (texh>0) Vertices[n].Map.y=Vertices[n].Map2.y*(dy-ys*2) + y + ys;
                else Vertices[n].Map.y=Vertices[n].Map2.y*dy + y;
                
                if (Vertices[n].Map.x<mini.x) mini.x=Vertices[n].Map.x;
                if (Vertices[n].Map.y<mini.y) mini.y=Vertices[n].Map.y;

                if (Vertices[n].Map.x>maxi.x) maxi.x=Vertices[n].Map.x;
                if (Vertices[n].Map.y>maxi.y) maxi.y=Vertices[n].Map.y;
            }
        }
        
        if (mini.x<0) mini.x=0;
        if (mini.y<0) mini.y=0;
        if (mini.x>1) mini.x=1;
        if (mini.y>1) mini.y=1;
        
        if (maxi.x<0) maxi.x=0;
        if (maxi.y<0) maxi.y=0;
        if (maxi.x>1) maxi.x=1;
        if (maxi.y>1) maxi.y=1;
        
        int x1=(int)(mini.x*2048);
        int y1=(int)(mini.y*2048);
        
        int x2=(int)(maxi.x*2048);
        int y2=(int)(maxi.y*2048);
        
        int r=(rand()%8)*32;
        int g=(rand()%8)*32;
        int b=(rand()%8)*32;
        
        for (int xx=x1;xx<x2;xx++)
            for (int yy=y1;yy<y2;yy++)
            {
                if ((xx>=0)&&(xx<2048)&&(yy>=0)&&(yy<2048))
                {
                    ptrimg[4*(xx+yy*2048)+0]=r;
                    ptrimg[4*(xx+yy*2048)+1]=g;
                    ptrimg[4*(xx+yy*2048)+2]=b;
                    ptrimg[4*(xx+yy*2048)+3]=(char)255;
                }
            }
    }

    free(grid);
    delete [] array;

    for (n=0;n<nFaces;n++) Faces[n].tag=0;
    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
    for (n=0;n<nFaces;n++) Faces[n].tag2=0;
    for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

    update=true;
    
    return ptrimg;
}

/*
void CObject3D::PlanarMappingBoundPainting()
{
	int n;
	int ntag=0;

	SetFaces();
	CVector *array=CalculateLightmapCoordinates();

	for (n=0;n<nFaces;n++)
	{
		if (Faces[n].tag>ntag) ntag=Faces[n].tag;
	}

	float aire=1.0f/(ntag*2);
	float d=(float) sqrt(aire);
	float x=0.0f;
	float y=0.0f;

	for (int tag=1;tag<=ntag;tag++)
	{
		for (n=0;n<nVertices;n++) Vertices[n].tag=0;
		for (n=0;n<nFaces;n++)
		{
			if (Faces[n].tag==tag)
			{
				Faces[n].v[0]->tag=1;
				Faces[n].v[1]->tag=1;
				Faces[n].v[2]->tag=1;
			}
		}

		for (n=0;n<nVertices;n++)
		{
			if (Vertices[n].tag)
			{
				Vertices[n].Map.x=Vertices[n].Map2.x*d +x;
				Vertices[n].Map.y=Vertices[n].Map2.y*d +y;
			}
		}

		x+=d;
		if (x+d>1.0f)
		{
			x=0.0f;
			y+=d;
		}

	}

	delete [] array;

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/**/

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingBound2()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingBound2()
{
	float a,b;
	float K=0.1f;
	int n;
	CVector min,max,v,d;
	int tag=0;

	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) tag=1; }
	if (tag) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	min.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	max.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		v=Vertices[n].Stok;
		if (v.x>max.x) max.x=v.x;
		if (v.y>max.y) max.y=v.y;
		if (v.z>max.z) max.z=v.z;
		if (v.x<min.x) min.x=v.x;
		if (v.y<min.y) min.y=v.y;
		if (v.z<min.z) min.z=v.z;
	}

	d=max-min;

	for (int nf=0;nf<nFaces;nf++)
	{
		if (Faces[nf].tag)
		{
			n=Faces[nf].v0;
			if ((d.x<d.y)&&(d.x<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.x<K) a+=1.0f;
			}

			if ((d.y<d.x)&&(d.y<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				if (Faces[nf].Norm.y<K) a+=1.0f;
			}

			if ((d.z<d.y)&&(d.z<d.x))
			{
				a=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.z<K) a+=1.0f;
			}

			Vertices[n].Map.Init(a/2,b);

			n=Faces[nf].v1;
			if ((d.x<d.y)&&(d.x<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.x<K) a+=1.0f;
			}

			if ((d.y<d.x)&&(d.y<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				if (Faces[nf].Norm.y<K) a+=1.0f;
			}

			if ((d.z<d.y)&&(d.z<d.x))
			{
				a=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.z<K) a+=1.0f;
			}

			Vertices[n].Map.Init(a/2,b);


			n=Faces[nf].v2;
			if ((d.x<d.y)&&(d.x<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.x<K) a+=1.0f;
			}

			if ((d.y<d.x)&&(d.y<d.z))
			{
				a=(Vertices[n].Stok.z-min.z)/(max.z-min.z);
				b=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				if (Faces[nf].Norm.y<K) a+=1.0f;
			}

			if ((d.z<d.y)&&(d.z<d.x))
			{
				a=(Vertices[n].Stok.x-min.x)/(max.x-min.x);
				b=(Vertices[n].Stok.y-min.y)/(max.y-min.y);
				if (Faces[nf].Norm.z<K) a+=1.0f;
			}

			Vertices[n].Map.Init(a/2,b);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nFaces;n++) Faces[n].tag2=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMapping()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMapping()
{
	int n,nn,res,tag,d,res2;
	CVector N,A,B,Na,O;
	CVector tmp,vect,u1,u2,u,m1,m2;
	CVector2 mm1,mm2;
	int a,b,c;
	EdgeListD E;
	int ne;
	int i;
	float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
    
	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	res=0;
	tag=1;
	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;

			N=Faces[d].Norm;

			nn=0;
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					nn++;
					if (Faces[n].Norm.Norme()>0.5f) N=Faces[n].Norm;
				}
			}

			E.Init(nn*3+3);
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					E.AddEdge(Faces[n].v0,Faces[n].v1);
					E.AddEdge(Faces[n].v1,Faces[n].v2);
					E.AddEdge(Faces[n].v2,Faces[n].v0);
				}
			}

			nn=0;
			res2=0;
			if (E.nList>=3)
			{
				while ((res2==0)&&(nn<E.nList))
				{
					a=E.List[nn].a;
					b=E.List[nn].b;
					E.List[nn].tag=1;

					ne=E.Next(b);
					if (E.List[ne].a==b) c=E.List[ne].b;
					else c=E.List[ne].a;

					E.List[ne].tag=1;

					res2=1;

					vect=Vertices[b].Stok-Vertices[a].Stok;
					Na=N^vect;

					Na.Normalise();
					aa=Na.x;
					bb=Na.y;
					cc=Na.z;
					dd=-(Vertices[a].Stok||Na);

					for (i=0;i<E.nList;i++)
					{
						if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
						{
							A=Vertices[E.List[i].a].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
						if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
						{
							A=Vertices[E.List[i].b].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
					}

					E.List[ne].tag=0;
					E.List[nn].tag=0;

					nn++;
				}

				if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
				{

					u1=Vertices[a].Stok-Vertices[b].Stok;
					u2=Vertices[c].Stok-Vertices[b].Stok;

					mm1=Vertices[a].Map-Vertices[b].Map;
					mm2=Vertices[c].Map-Vertices[b].Map;

					m1.Init(mm1.x,mm1.y,0);
					m2.Init(mm2.x,mm2.y,0);

					u1.Normalise();
					u2.Normalise();

					O=Vertices[b].Stok;

					u2=N^u1;
					u2.Normalise();

					s1=0;
					s2=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
						tmp=B-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
					}

					O=O+s1*u1+s2*u2;

					u1max=0;
					u2max=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
						tmp=B-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
					}

					for (n=0;n<nVertices;n++) Vertices[n].tag=0;

					for (n=0;n<nFaces;n++)
					{
						if (Faces[n].tag==tag)
						{
							Faces[n].v[0]->tag=1;
							Faces[n].v[1]->tag=1;
							Faces[n].v[2]->tag=1;
						}
					}
					for (n=0;n<nVertices;n++)
					{
						if (Vertices[n].tag==1)
						{
							B=Vertices[n].Stok-O;
							s1=(B||u1)/u1max;
							s2=(B||u2)/u2max;
							Vertices[n].Map.Init(s1,s2);
						}
					}
				}
			}
			E.Free();
			tag++;
		}
	}

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

    for (n=0;n<nFaces;n++)
    {
        if (TAG)
        {
            if (Faces[n].tag==-666) Faces[n].tag2=0;
            else Faces[n].tag2=1;
        }
        else Faces[n].tag2=0;
        
        Faces[n].tag=0;
    }
    
    InvertYMap();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingRadial()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingRadial()
{
	int n,nn,res,tag,d,res2;
	CVector N,A,B,Na,O;
	CVector tmp,vect,u1,u2,u,m1,m2;
	CVector2 mm1,mm2;
	int a,b,c;
	EdgeListD E;
	int ne;
	int i;
	float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
    
	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	res=0;
	tag=1;
	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCoplanarFaces(d,d,tag);
			Faces[d].tag=tag;

			N=Faces[d].Norm;

			nn=0;
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					nn++;
					if (Faces[n].Norm.Norme()>0.5f) N=Faces[n].Norm;
				}
			}

			E.Init(nn*3+3);
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					E.AddEdge(Faces[n].v0,Faces[n].v1);
					E.AddEdge(Faces[n].v1,Faces[n].v2);
					E.AddEdge(Faces[n].v2,Faces[n].v0);
				}
			}

			nn=0;
			res2=0;
			if (E.nList>=3)
			{
				while ((res2==0)&&(nn<E.nList))
				{
					a=E.List[nn].a;
					b=E.List[nn].b;
					E.List[nn].tag=1;

					ne=E.Next(b);
					if (E.List[ne].a==b) c=E.List[ne].b;
					else c=E.List[ne].a;

					E.List[ne].tag=1;

					res2=1;

					vect=Vertices[b].Stok-Vertices[a].Stok;
					Na=N^vect;

					Na.Normalise();
					aa=Na.x;
					bb=Na.y;
					cc=Na.z;
					dd=-(Vertices[a].Stok||Na);

					for (i=0;i<E.nList;i++)
					{
						if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
						{
							A=Vertices[E.List[i].a].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
						if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
						{
							A=Vertices[E.List[i].b].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
					}

					E.List[ne].tag=0;
					E.List[nn].tag=0;

					nn++;
				}

				if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
				{

					u1=Vertices[a].Stok-Vertices[b].Stok;
					u2=Vertices[c].Stok-Vertices[b].Stok;

					mm1=Vertices[a].Map-Vertices[b].Map;
					mm2=Vertices[c].Map-Vertices[b].Map;

					m1.Init(mm1.x,mm1.y,0);
					m2.Init(mm2.x,mm2.y,0);

					u1.Normalise();
					u2.Normalise();

					O=Vertices[b].Stok;

					//u2=N^u1;
					//u2.Normalise();
                    
                    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
                    
                    for (n=0;n<nFaces;n++)
                    {
                        if (Faces[n].tag==tag)
                        {
                            Faces[n].v[0]->tag=1;
                            Faces[n].v[1]->tag=1;
                            Faces[n].v[2]->tag=1;
                        }
                    }
                    
                    CVector uu(0,0,0);
                    CVector NN(0,0,0);
                    for (n=0;n<nVertices;n++)
                        if (Vertices[n].tag==1)
                        {
                            uu+=Vertices[n].Stok;
                            NN+=Vertices[n].Norm;
                        }
                    
                    NN.Normalise();
                    
                    u2=NN^u1;
                    u2.Normalise();
                    
                    
                    float d1,d2;
                    
                    uu.y=0.0f;
                    uu.Normalise();
                    d1=DOT(uu,u1);
                    d2=DOT(uu,u2);
                    
                    if (f_abs(d1)>f_abs(d2))
                    {
                        CMatrix M;
                        CVector uu1,uu2;
                        float sd=-10000;
                        CVector base_u1=u1;
                        CVector base_u2=u2;
                        
                        for (n=0;n<4;n++)
                        {
                            M.Id();
                            M.RotationAngleVecteur(NN,n*PI/2);
                            uu1=base_u1*M;
                            uu2=base_u2*M;
                            
                            float d=f_abs(DOT(uu,uu2));
                            if (d>sd)
                            {
                                sd=d;
                                u1=uu1;
                                u2=uu2;
                            }
                        }
                    }
                    
                    d2=DOT(uu,u2);
                    if (d2<0)
                    {
                        u2=-1*u2;
                    }


					s1=0;
					s2=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
						tmp=B-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
					}

					O=O+s1*u1+s2*u2;

					u1max=0;
					u2max=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
						tmp=B-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
					}

					for (n=0;n<nVertices;n++)
					{
						if (Vertices[n].tag==1)
						{
							B=Vertices[n].Stok-O;
							s1=(B||u1)/u1max;
							s2=(B||u2)/u2max;
							Vertices[n].Map.Init(s1,s2);
						}
					}
				}
			}
			E.Free();
			tag++;
		}
	}

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

    for (n=0;n<nFaces;n++)
    {
        if (TAG)
        {
            if (Faces[n].tag==-666) Faces[n].tag2=0;
            else Faces[n].tag2=1;
        }
        else Faces[n].tag2=0;
        
        Faces[n].tag=0;
    }
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingClose()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingClose()
{
	int n,nn,res,tag,d,res2;
	CVector N,A,B,Na,O;
	CVector tmp,vect,u1,u2,u,m1,m2;
	CVector2 mm1,mm2;
	int a,b,c;
	EdgeListD E;
	int ne;
	int i;
	float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	SetF012();

	if (TAG)
	{
        for (n=0;n<nFaces;n++)
            if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	res=0;
	tag=1;
	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;

			N=Faces[d].Norm;

			nn=0;
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					nn++;
					if (Faces[n].Norm.Norme()>0.5f) N=Faces[n].Norm;
				}
			}

			E.Init(nn*3+3);
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					E.AddEdge(Faces[n].v0,Faces[n].v1);
					E.AddEdge(Faces[n].v1,Faces[n].v2);
					E.AddEdge(Faces[n].v2,Faces[n].v0);
				}
			}

			nn=0;
			res2=0;
			if (E.nList>=3)
			{
				while ((res2==0)&&(nn<E.nList))
				{
					a=E.List[nn].a;
					b=E.List[nn].b;
					E.List[nn].tag=1;

					ne=E.Next(b);
					if (E.List[ne].a==b) c=E.List[ne].b;
					else c=E.List[ne].a;

					E.List[ne].tag=1;

					res2=1;

					vect=Vertices[b].Stok-Vertices[a].Stok;
					Na=N^vect;

					Na.Normalise();
					aa=Na.x;
					bb=Na.y;
					cc=Na.z;
					dd=-(Vertices[a].Stok||Na);

					for (i=0;i<E.nList;i++)
					{
						if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
						{
							A=Vertices[E.List[i].a].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
						if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
						{
							A=Vertices[E.List[i].b].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
					}

					E.List[ne].tag=0;
					E.List[nn].tag=0;

					nn++;
				}

				if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
				{

					u1=Vertices[a].Stok-Vertices[b].Stok;
					u2=Vertices[c].Stok-Vertices[b].Stok;

					mm1=Vertices[a].Map-Vertices[b].Map;
					mm2=Vertices[c].Map-Vertices[b].Map;

					m1.Init(mm1.x,mm1.y,0);
					m2.Init(mm2.x,mm2.y,0);

					u1.Normalise();
					u2.Normalise();

					O=Vertices[b].Stok;

					u2=N^u1;
					u2.Normalise();

					s1=0;
					s2=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
						tmp=B-O;
						ss=(tmp||u1);
						if (s1>ss) s1=ss;
						ss=(tmp||u2);
						if (s2>ss) s2=ss;
					}

					O=O+s1*u1+s2*u2;

					u1max=0;
					u2max=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
						tmp=B-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
					}

					for (n=0;n<nVertices;n++) Vertices[n].tag=0;

					for (n=0;n<nFaces;n++)
					{
						if (Faces[n].tag==tag)
						{
							Faces[n].v[0]->tag=1;
							Faces[n].v[1]->tag=1;
							Faces[n].v[2]->tag=1;
						}
					}
					for (n=0;n<nVertices;n++)
					{
						if (Vertices[n].tag==1)
						{
							B=Vertices[n].Stok-O;
							s1=(B||u1)/u1max;
							s2=(B||u2)/u2max;
							Vertices[n].Map.Init(s1,s2);
						}
					}
				}
			}
			E.Free();
			tag++;
		}
	}

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

	for (n=0;n<nFaces;n++)
	{
        if (TAG)
        {
            if (Faces[n].tag==-666) Faces[n].tag2=0;
            else Faces[n].tag2=1;
        }
        else Faces[n].tag2=0;
        
		Faces[n].tag=0;
	}
    
    InvertYMap();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::PlanarMappingCloseRadial()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::PlanarMappingCloseRadial()
{
	int n,nn,res,tag,d,res2;
	CVector N,A,B,Na,O;
	CVector tmp,vect,u1,u2,u,m1,m2;
	CVector2 mm1,mm2;
	int a,b,c;
	EdgeListD E;
	int ne;
	int i;
	float ss,aa,bb,cc,dd,s1,s2,u1max,u2max;

	int TAG=0;
	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) TAG=1; }

	res=0;
	tag=1;

	SetFaces();
	CalculateNormals(-1);

	SetF012();

	if (TAG)
	{
        for (n=0;n<nFaces;n++)
            if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;
			tag++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	if (TAG)
	{
		for (n=0;n<nFaces;n++)
			if (Faces[n].tag2==0) Faces[n].tag=-666;
	}

	res=0;
	tag=1;
	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<nFaces)&&(d==-1))
		{
			if (Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagCloseToCloseFaces2(d,d,tag);
			Faces[d].tag=tag;

			N=Faces[d].Norm;

			nn=0;
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					nn++;
					if (Faces[n].Norm.Norme()>0.5f) N=Faces[n].Norm;
				}
			}

			E.Init(nn*3+3);
			for (n=0;n<nFaces;n++)
			{
				if (Faces[n].tag==tag)
				{
					E.AddEdge(Faces[n].v0,Faces[n].v1);
					E.AddEdge(Faces[n].v1,Faces[n].v2);
					E.AddEdge(Faces[n].v2,Faces[n].v0);
				}
			}

			nn=0;
			res2=0;
			if (E.nList>=3)
			{
				while ((res2==0)&&(nn<E.nList))
				{
					a=E.List[nn].a;
					b=E.List[nn].b;
					E.List[nn].tag=1;

					ne=E.Next(b);
					if (E.List[ne].a==b) c=E.List[ne].b;
					else c=E.List[ne].a;

					E.List[ne].tag=1;

					res2=1;

					vect=Vertices[b].Stok-Vertices[a].Stok;
					Na=N^vect;

					Na.Normalise();
					aa=Na.x;
					bb=Na.y;
					cc=Na.z;
					dd=-(Vertices[a].Stok||Na);

					for (i=0;i<E.nList;i++)
					{
						if ((E.List[i].a!=a)&&(E.List[i].a!=b)&&(E.List[i].a!=c))
						{
							A=Vertices[E.List[i].a].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
						if ((E.List[i].b!=a)&&(E.List[i].b!=b)&&(E.List[i].b!=c))
						{
							A=Vertices[E.List[i].b].Stok;
							s1=A.x*aa+A.y*bb+A.z*cc+dd;
							if (s1>SMALLF) res2=0;
						}
					}

					E.List[ne].tag=0;
					E.List[nn].tag=0;

					nn++;
				}

				if (res2==1) // 2 arr�tes cons�cutives qui ne coupent pas les autres arr�tes
				{

					u1=Vertices[a].Stok-Vertices[b].Stok;
					u2=Vertices[c].Stok-Vertices[b].Stok;

					mm1=Vertices[a].Map-Vertices[b].Map;
					mm2=Vertices[c].Map-Vertices[b].Map;

					m1.Init(mm1.x,mm1.y,0);
					m2.Init(mm2.x,mm2.y,0);

					u1.Normalise();
					u2.Normalise();

					O=Vertices[b].Stok;

//					u2=N^u1;
//					u2.Normalise();
                    
                    for (n=0;n<nVertices;n++) Vertices[n].tag=0;
                    
                    for (n=0;n<nFaces;n++)
                    {
                        if (Faces[n].tag==tag)
                        {
                            Faces[n].v[0]->tag=1;
                            Faces[n].v[1]->tag=1;
                            Faces[n].v[2]->tag=1;
                        }
                    }
                    
                    CVector uu(0,0,0);
                    CVector NN(0,0,0);
                    for (n=0;n<nVertices;n++)
                        if (Vertices[n].tag==1)
                        {
                            uu+=Vertices[n].Stok;
                            NN+=Vertices[n].Norm;
                        }
                    
                    NN.Normalise();
                    
                    u2=NN^u1;
                    u2.Normalise();

                    
                    float d1,d2;
                    
                    uu.y=0.0f;
                    uu.Normalise();
                    d1=DOT(uu,u1);
                    d2=DOT(uu,u2);
                    
                    if (f_abs(d1)>f_abs(d2))
                    {
                        CMatrix M;
                        CVector uu1,uu2;
                        float sd=-10000;
                        CVector base_u1=u1;
                        CVector base_u2=u2;
                        
                        for (n=0;n<4;n++)
                        {
                            M.Id();
                            M.RotationAngleVecteur(NN,n*PI/2);
                            uu1=base_u1*M;
                            uu2=base_u2*M;
                        
                            float d=f_abs(DOT(uu,uu2));
                            if (d>sd)
                            {
                                sd=d;
                                u1=uu1;
                                u2=uu2;
                            }
                        }
                    }
                    
                    d2=DOT(uu,u2);
                    if (d2<0)
                    {
                        u2=-1*u2;
                    }

                    s1=0;
                    s2=0;
                    for (n=0;n<E.nList;n++)
                    {
                        A=Vertices[E.List[n].a].Stok;
                        B=Vertices[E.List[n].b].Stok;
                        tmp=A-O;
                        ss=(tmp||u1);
                        if (s1>ss) s1=ss;
                        ss=(tmp||u2);
                        if (s2>ss) s2=ss;
                        tmp=B-O;
                        ss=(tmp||u1);
                        if (s1>ss) s1=ss;
                        ss=(tmp||u2);
                        if (s2>ss) s2=ss;
                    }
                    
                    O=O+s1*u1+s2*u2;
                    
					u1max=0;
					u2max=0;
					for (n=0;n<E.nList;n++)
					{
						A=Vertices[E.List[n].a].Stok;
						B=Vertices[E.List[n].b].Stok;
						tmp=A-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
						tmp=B-O;
						s1=f_abs(tmp||u1);
						s2=f_abs(tmp||u2);
						if (s1>u1max) u1max=s1;
						if (s2>u2max) u2max=s2;
					}
                    
					for (n=0;n<nVertices;n++)
					{
						if (Vertices[n].tag==1)
						{
							B=Vertices[n].Stok-O;
							s1=(B||u1)/u1max;
							s2=(B||u2)/u2max;
							Vertices[n].Map.Init(s1,s2);
						}
					}
                    
				}
			}
			E.Free();
			tag++;
		}
	}

	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;

	for (n=0;n<nFaces;n++)
	{
        if (TAG)
        {
            if (Faces[n].tag==-666) Faces[n].tag2=0;
            else Faces[n].tag2=1;
        }
        else Faces[n].tag2=0;
        
		Faces[n].tag=0;
	}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::CylindricalMapping()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::CylindricalMappingCenter()
{
	CVector G=Center();
	Centered();
	CylindricalMapping();
	for (int n=0;n<nVertices;n++) Vertices[n].Stok+=G;
}

void CObject3D::CylindricalMapping()
{
	float x,y,z;
	float a,b;
	float aa[3],bb[3];
	int n;
	float h1,h2;
	int tag=0;

	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) tag=1; }
	if (tag) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	h1=MAXI_BOUND;
	h2=-MAXI_BOUND;
	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].Stok.y>h2) h2=Vertices[n].Stok.y;
		if (Vertices[n].Stok.y<h1) h1=Vertices[n].Stok.y;
	}

	for (int nf=0;nf<nFaces;nf++)
	{
		if (Faces[nf].tag)
		{
			n=Faces[nf].v0;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(Vertices[n].Stok.y-h1)/(h2-h1);
			aa[0]=a; bb[0]=b;

			n=Faces[nf].v1;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(Vertices[n].Stok.y-h1)/(h2-h1);
			aa[1]=a; bb[1]=b;

			n=Faces[nf].v2;
			x=Vertices[n].Stok.x;
			y=Vertices[n].Stok.y;
			z=Vertices[n].Stok.z;
			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(Vertices[n].Stok.y-h1)/(h2-h1);
			aa[2]=a; bb[2]=b;

			if ((aa[0]>0.85f)||(aa[1]>0.85f)||(aa[2]>0.85f))
			{
				if (aa[0]<0.15f) aa[0]+=1.0f;
				if (aa[1]<0.15f) aa[1]+=1.0f;
				if (aa[2]<0.15f) aa[2]+=1.0f;
			}

			Vertices[Faces[nf].v0].Map.Init(aa[0],bb[0]);
			Vertices[Faces[nf].v1].Map.Init(aa[1],bb[1]);
			Vertices[Faces[nf].v2].Map.Init(aa[2],bb[2]);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::CylindricalMappingOrtho()
 
 
	Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::CylindricalMappingOrtho()
{
	float x,y,z;
	float a,b;
	float aa[3],bb[3];
	int n;
	float h1,h2;
	int tag=0;
	CVector mini(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	CVector maxi(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<nVertices;n++)
	{
		if (Vertices[n].Stok.x>maxi.x) maxi.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y>maxi.y) maxi.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z>maxi.z) maxi.z=Vertices[n].Stok.z;
			
		if (Vertices[n].Stok.x<mini.x) mini.x=Vertices[n].Stok.x;
		if (Vertices[n].Stok.y<mini.y) mini.y=Vertices[n].Stok.y;
		if (Vertices[n].Stok.z<mini.z) mini.z=Vertices[n].Stok.z;			
	}

	int axe=0;
	CVector d=maxi-mini;
	if ((d.y>d.x)&&(d.y>d.z)) axe=1;
	if ((d.z>d.x)&&(d.z>d.y)) axe=2;

	for (n=0;n<nFaces;n++) Faces[n].tag=1;
	for (n=0;n<nFaces;n++) { if (Faces[n].tag2) tag=1; }
	if (tag) { for (n=0;n<nFaces;n++) Faces[n].tag=Faces[n].tag2; }

	h1=MAXI_BOUND;
	h2=-MAXI_BOUND;
	for (n=0;n<nVertices;n++)
	{
		switch (axe)
		{
		case 0:
			if (Vertices[n].Stok.x>h2) h2=Vertices[n].Stok.x;
			if (Vertices[n].Stok.x<h1) h1=Vertices[n].Stok.x;
			break;
		case 1:
			if (Vertices[n].Stok.y>h2) h2=Vertices[n].Stok.y;
			if (Vertices[n].Stok.y<h1) h1=Vertices[n].Stok.y;
			break;
		case 2:
			if (Vertices[n].Stok.z>h2) h2=Vertices[n].Stok.z;
			if (Vertices[n].Stok.z<h1) h1=Vertices[n].Stok.z;
			break;
		};
	}

	for (int nf=0;nf<nFaces;nf++)
	{
		if (Faces[nf].tag)
		{
			n=Faces[nf].v0;

			x=Vertices[n].Stok.v[(3-1+axe)%3];
			y=Vertices[n].Stok.v[axe];
			z=Vertices[n].Stok.v[(1+axe)%3];

			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(y-h1)/(h2-h1);
			aa[0]=a; bb[0]=b;

			n=Faces[nf].v1;

			x=Vertices[n].Stok.v[(3-1+axe)%3];
			y=Vertices[n].Stok.v[axe];
			z=Vertices[n].Stok.v[(1+axe)%3];

			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(y-h1)/(h2-h1);
			aa[1]=a; bb[1]=b;

			n=Faces[nf].v2;
			
			x=Vertices[n].Stok.v[(3-1+axe)%3];
			y=Vertices[n].Stok.v[axe];
			z=Vertices[n].Stok.v[(1+axe)%3];

			if (z>0)
			{
				a=(float) atanf(x/z);
				if (a<0) a=2*PI+a;
			}
			else
			{
				if (z<0) a=PI - (float) atanf(x/-z);
				else
				{
					if (x>0) a=PI/2;
					else a=3*PI/2;
				}
			}
			a=a/(2*PI);
			b=(y-h1)/(h2-h1);
			aa[2]=a; bb[2]=b;

			if ((aa[0]>0.85f)||(aa[1]>0.85f)||(aa[2]>0.85f))
			{
				if (aa[0]<0.15f) aa[0]+=1.0f;
				if (aa[1]<0.15f) aa[1]+=1.0f;
				if (aa[2]<0.15f) aa[2]+=1.0f;
			}

			Vertices[Faces[nf].v0].Map.Init(aa[0],bb[0]);
			Vertices[Faces[nf].v1].Map.Init(aa[1],bb[1]);
			Vertices[Faces[nf].v2].Map.Init(aa[2],bb[2]);
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag=0;
	for (n=0;n<nVertices;n++) Vertices[n].tag2=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		  void CObject3D::ScaleMappingCoordinate(float sx,float sy)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::ScaleMappingCoordinate(float sx,float sy)
{
	int n;

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].Map.x*=sx;
		Vertices[n].Map.y*=sy;
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Mass distribution

		void CObject3D::CalculateVertexMasses()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::CalculateVertexMasses()
{
	int n;
	CVector u0,u1,u2;

	for (n=0;n<nVertices;n++) Vertices[n].coef1=0.0f;

	for (n=0;n<nFaces;n++)
	{
		u0=Faces[n].v[1]->Stok - Faces[n].v[0]->Stok;
		u1=Faces[n].v[2]->Stok - Faces[n].v[1]->Stok;
		u2=Faces[n].v[0]->Stok - Faces[n].v[2]->Stok;
		Faces[n].v[0]->coef1+=u0.Norme() + u2.Norme();
		Faces[n].v[1]->coef1+=u0.Norme() + u1.Norme();
		Faces[n].v[2]->coef1+=u1.Norme() + u2.Norme();
	}

	CalculateNormals(-1);

	/*
	for (n=0;n<nVertices;n++) Vertices[n].NormCalc.Init(0,0,0);
	for (n=0;n<nFaces;n++) FaceNormalePrecalc2((*this).Faces[n],this);
    for (n=0;n<nVertices;n++) Vertices[n].coef1=Vertices[n].NormCalc.Norme();
	/**/
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		From mass distribution

		void CObject3D::GravityCenter()

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CObject3D::GravityCenter()
{
	CVector G;
	int n;
	float masse;

	G.Init(0,0,0);
	masse=0;
	for (n=0;n<nVertices;n++)
	{
		G+=Vertices[n].Stok*Vertices[n].coef1;
		masse+=Vertices[n].coef1;
	}
	G=G/masse;


	for (n=0;n<nVertices;n++) Vertices[n].Stok-=G;
}



void CObject3D::Escalier(float L,float H,float L2,float hmm,int def,float R,float dw)
{
	int n,nf;
	float x,z,hm;
	int pos;

	hm=(hmm/(def-1));

	Init(20+(def-2)*22 + 26,10+14+12*(def-2));
	sprintf(Name,"OBJ");

	nf=0;

	for (n=0;n<def;n++)
	{
		if (n==0)
		{
			pos=0;

			// face de fermeture
			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 0].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 1].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 2].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 3].Stok.Init(x,H/2 + hm*n,z);


			Carre2(nf,pos+1,pos+3,pos+2,pos+0);
			nf+=2;

			// cot� <0


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 4].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 5].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 6].Stok.Init(x,H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 7].Stok.Init(x,-H/2 + hm*(1+n),z);

			Carre2(nf,pos+4,pos+5,pos+6,pos+7);
			nf+=2;



			// cot� >0


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 8].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 9].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 10].Stok.Init(x,H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 11].Stok.Init(x,-H/2 + hm*(1+n),z);

			Carre2(nf,pos+11,pos+10,pos+9,pos+8);
			nf+=2;


			// dessus

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 12].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 13].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 14].Stok.Init(x,-H/2 + hm*(n+1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 15].Stok.Init(x,-H/2 + hm*(n+1),z);

			Carre2(nf,pos+12,pos+13,pos+14,pos+15);
			nf+=2;

			// dessous

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 16].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 17].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 18].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 19].Stok.Init(x,H/2 + hm*(n),z);

			Carre2(nf,pos+16,pos+17,pos+18,pos+19);
			nf+=2;
		}
		else
		{
			pos=20+(n-1)*22;

			// dessus

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 0].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 1].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 2].Stok.Init(x,-H/2 + hm*(n+1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 3].Stok.Init(x,-H/2 + hm*(n+1),z);

			Carre2(nf,pos+0,pos+1,pos+2,pos+3);
			nf+=2;


			// dessous

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 4].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 5].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 6].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 7].Stok.Init(x,H/2 + hm*(n),z);

			Carre2(nf,pos+4,pos+5,pos+6,pos+7);
			nf+=2;

			// cote <0

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 8].Stok.Init(x,-H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 9].Stok.Init(x,H/2 + hm*(n-1),z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 10].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 11].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 12].Stok.Init(x,-H/2 + hm*(n+1),z);


			Carre2(nf,pos+8,pos+9,pos+11,pos+12);
			nf+=2;
			Triangle(nf,pos+9,pos+10,pos+11);
			nf++;
			// cote >0

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 13].Stok.Init(x,-H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 14].Stok.Init(x,H/2 + hm*(n-1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 15].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 16].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 17].Stok.Init(x,-H/2 + hm*(n+1),z);


			Carre2(nf,pos+17,pos+16,pos+14,pos+13);
			nf+=2;
			Triangle(nf,pos+16,pos+15,pos+14);
			nf++;

			// marche

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 18].Stok.Init(x,H/2 + hm*(n-1),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 19].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos+ 20].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos+ 21].Stok.Init(x,H/2 + hm*(n-1),z);

			Carre2(nf,pos+21,pos+20,pos+19,pos+18);
			nf+=2;

			if (n==def-1)
			{
				// face de fermeture
				if (R>0)
				{
					x=(float) (R+L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R+L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=L2/2;
				}

				Vertices[pos+ 22].Stok.Init(x,-H/2 + hm*(n+1),z);

				if (R>0)
				{
					x=(float) (R+L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R+L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=L2/2;
				}

				Vertices[pos+ 23].Stok.Init(x,H/2 + hm*(n),z);

				if (R>0)
				{
					x=(float) (R-L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R-L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=-L2/2;
				}

				Vertices[pos+ 24].Stok.Init(x,H/2 + hm*(n),z);


				if (R>0)
				{
					x=(float) (R-L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R-L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=-L2/2;
				}

				Vertices[pos+ 25].Stok.Init(x,-H/2 + hm*(n+1),z);

				Carre2(nf,pos+25,pos+24,pos+23,pos+22);
				nf+=2;
			}
		}
	}


	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0;
		Faces[n].nT=0;
	}


	SetTag(SET_FACES_TAG+0);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


void CObject3D::Escalier2(float L,float H,float L2,float hmm,int def,float R,float dw)
{
	int n,nf;
	float x,z,hm;
	int pos,posm1;

	hm=(hmm/(def-1));


	Init(20+(def-2)*20 + 24,10+14+12*(def-2));
	sprintf(Name,"OBJ");
	nf=0;

	for (n=0;n<def;n++)
	{


		if (n==0)
		{
			pos=0;

			// face de fermeture
			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 0].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 1].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 2].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 3].Stok.Init(x,H/2 + hm*n,z);


			Carre2(nf,pos+1,pos+3,pos+2,pos+0);
			nf+=2;

			// cot� <0


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 4].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 5].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 6].Stok.Init(x,H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 7].Stok.Init(x,-H/2 + hm*(1+n),z);

			Carre2(nf,pos+4,pos+5,pos+6,pos+7);
			nf+=2;



			// cot� >0


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 8].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 9].Stok.Init(x,H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 10].Stok.Init(x,H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 11].Stok.Init(x,-H/2 + hm*(1+n),z);

			Carre2(nf,pos+11,pos+10,pos+9,pos+8);
			nf+=2;


			// dessus

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 12].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 13].Stok.Init(x,-H/2 + hm*n,z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 14].Stok.Init(x,-H/2 + hm*(n+1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 15].Stok.Init(x,-H/2 + hm*(n+1),z);

			Carre2(nf,pos+12,pos+13,pos+14,pos+15);
			nf+=2;


			// dessous

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 16].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 17].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 18].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 19].Stok.Init(x,H/2 + hm*(n),z);

			Carre2(nf,pos+16,pos+17,pos+18,pos+19);
			nf+=2;


		}
		else
		{

			pos=20+(n-1)*20;
			if (n>1)
			{
				posm1=20+(n-2)*20;
			}

			// dessus
/*
			if (R>0)
			{
				x=(float) (R+L2/2)*sinf(n*dw*PI/180);
				z=(float) (R+L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=L2/2;
			}
			Vertices[pos + 0].Stok.Init(x,-H/2 + hm*n,z);
			if (R>0)
			{
				x=(float) (R-L2/2)*sinf(n*dw*PI/180);
				z=(float) (R-L2/2)*cosf(n*dw*PI/180);
			}
			else
			{
				x=n*L/def;
				z=-L2/2;
			}
			Vertices[pos + 1].Stok.Init(x,-H/2 + hm*n,z);
*/

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 0].Stok.Init(x,-H/2 + hm*(n+1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 1].Stok.Init(x,-H/2 + hm*(n+1),z);

			if (n>1)
				Carre2(nf,posm1+1,posm1+0,pos+0,pos+1);
			else
				Carre2(nf,15,14,pos+0,pos+1);
			nf+=2;


			// dessous

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 2].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=L2/2;
			}
			Vertices[pos + 3].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 4].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos + 5].Stok.Init(x,H/2 + hm*(n),z);

			Carre2(nf,pos+2,pos+3,pos+4,pos+5);
			nf+=2;

			// cote <0

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 6].Stok.Init(x,-H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 7].Stok.Init(x,H/2 + hm*(n-1),z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 8].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 9].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 10].Stok.Init(x,-H/2 + hm*(n+1),z);


			if (n==1)
			{

				Carre2(nf,7,6,pos+9,pos+10);
				nf+=2;
				Triangle(nf,6,pos+8,pos+9);
				nf++;
			}
			else
			{
				Carre2(nf,posm1+10,posm1+9,pos+9,pos+10);
				nf+=2;
				Triangle(nf,posm1+9,pos+8,pos+9);
				nf++;
			}

			// cote >0

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 11].Stok.Init(x,-H/2 + hm*n,z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 12].Stok.Init(x,H/2 + hm*(n-1),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 13].Stok.Init(x,H/2 + hm*(n),z);

			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 14].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((1+n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((1+n)*dw*PI/180);
			}
			else
			{
				x=(1+n)*L/def;
				z=+L2/2;
			}
			Vertices[pos+ 15].Stok.Init(x,-H/2 + hm*(n+1),z);


			if (n==1)
			{
				Carre2(nf,pos+15,pos+14,10,11);
				nf+=2;
				Triangle(nf,pos+14,pos+13,10);
				nf++;
			}
			else
			{

				Carre2(nf,pos+15,pos+14,posm1+14,posm1+15);
				nf+=2;
				Triangle(nf,pos+14,pos+13,posm1+14);
				nf++;
			}

			// marche

			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 16].Stok.Init(x,H/2 + hm*(n-1),z);


			if (R>0)
			{
				x=(float) (R-L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R-L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=-L2/2;
			}
			Vertices[pos+ 17].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos+ 18].Stok.Init(x,H/2 + hm*(n),z);


			if (R>0)
			{
				x=(float) (R+L2/2)*sinf((n)*dw*PI/180);
				z=(float) (R+L2/2)*cosf((n)*dw*PI/180);
			}
			else
			{
				x=(n)*L/def;
				z=L2/2;
			}
			Vertices[pos+ 19].Stok.Init(x,H/2 + hm*(n-1),z);

			Carre2(nf,pos+19,pos+18,pos+17,pos+16);
			nf+=2;

			if (n==def-1)
			{
				// face de fermeture
				if (R>0)
				{
					x=(float) (R+L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R+L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=L2/2;
				}

				Vertices[pos+ 20].Stok.Init(x,-H/2 + hm*(n+1),z);

				if (R>0)
				{
					x=(float) (R+L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R+L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=L2/2;
				}

				Vertices[pos+ 21].Stok.Init(x,H/2 + hm*(n),z);

				if (R>0)
				{
					x=(float) (R-L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R-L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=-L2/2;
				}

				Vertices[pos+ 22].Stok.Init(x,H/2 + hm*(n),z);


				if (R>0)
				{
					x=(float) (R-L2/2)*sinf((n+1)*dw*PI/180);
					z=(float) (R-L2/2)*cosf((n+1)*dw*PI/180);
				}
				else
				{
					x=(n+1)*L/def;
					z=-L2/2;
				}

				Vertices[pos+ 23].Stok.Init(x,-H/2 + hm*(n+1),z);

				Carre2(nf,pos+23,pos+22,pos+21,pos+20);
				nf+=2;
			}
		}
	}


	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0;
		Faces[n].nT=0;
	}


	SetTag(SET_FACES_TAG+0);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}


void CObject3D::Tunnel(float L,float H,float L2,int def)
{
	float bas,milieu;
	int n,m,n1,n2,nf;
	float x,y,x2,y2,h;

	Init((def+1)*2*2 + (def+1)*2*4,def*4 +(def+1)*4);
	sprintf(Name,"OBJ");

	h=(H-L/2)*0.9f;

	bas=H/2;
	milieu=bas - h;

	for (n=0;n<(def+1)*2;n++)
	{

		if (n==0)
		{
			x=L/2;
			y=bas;
		}
		else
		if (n==(def+1)*2 - 1)
		{
			x=-L/2;
			y=bas;
		}
		else
		{
			x=(float) ((L/2)*cosf((PI*(n-1))/(def*2 -1)));
			y=milieu-(float) ((L/2)*sinf((PI*(n-1))/(def*2 -1)));
		}

		m=(n+1)%((def+1)*2);

		if (m==0)
		{
			x2=L/2;
			y2=bas;
		}
		else
		if (m==(def+1)*2 - 1)
		{
			x2=-L/2;
			y2=bas;
		}
		else
		{
			x2=(float) ((L/2)*cosf((PI*(m-1))/(def*2 -1)));
			y2=milieu-(float) ((L/2)*sinf((PI*(m-1))/(def*2 -1)));
		}

		Vertices[n].Stok.Init(x,y,-L2/2);
		Vertices[(def+1)*2+n].Stok.Init(x,y,L2/2);

		Vertices[n].Map.Init((x+(L/2))/L,(y+(h/2)+(L/2))/H);
		Vertices[(def+1)*2+n].Map.Init((x+(L/2))/L,(y+(h/2)+(L/2))/H);

		Vertices[((def+1)*2*2) + n*4 +0].Stok.Init(x,y,-L2/2);
		Vertices[((def+1)*2*2) + n*4 +1].Stok.Init(x2,y2,-L2/2);
		Vertices[((def+1)*2*2) + n*4 +2].Stok.Init(x2,y2,L2/2);
		Vertices[((def+1)*2*2) + n*4 +3].Stok.Init(x,y,L2/2);
	}


	n1=0;
	n2=(def+1)*2 -1;
	m=(def+1)*2;
	for (n=0;n<def;n++)
	{
		Carre(n*4+0,n1,n1+1,n2-1,n2);
		Carre(n*4+2,m+n2,m+n2-1,m+n1+1,m+n1);
		n1++;
		n2--;
	}

	nf=def*4;
	m=((def+1)*2*2);
	for (n=0;n<(def+1)*2;n++)
	{
		Carre2(nf,m+n*4+3,m+n*4+2,m+n*4+1,m+n*4+0);
		nf+=2;
	}


	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0;
		Faces[n].nT=0;
	}


	SetTag(SET_FACES_TAG+0);

	SetFaces();
	CalculateNormals(-1);

	SetRefTexture();
}



void CObject3D::Tunnel2(float L,float H,float L2,int def)
{
	float bas,milieu;
	int n,m,nf;
	float x,y,x2,y2,h;

	Init((def+1)*2 + (def+1)*4,(def-1)*2+(def+1)*2);


	h=(H-L/2)*0.9f;

	bas=H/2;
	milieu=bas - h;

	for (n=0;n<=def;n++)
	{

		if (n==0)
		{
			x=L/2;
			y=bas;
		}
		else
		if (n==def)
		{
			x=-L/2;
			y=bas;
		}
		else
		{
			x=(float) ((L/2)*cosf((PI*(n-1))/(def -2)));
			y=milieu-(float) ((L/2)*sinf((PI*(n-1))/(def -2)));
		}

		m=(n+1)%((def+1));

		if (m==0)
		{
			x2=L/2;
			y2=bas;
		}
		else
		if (m==def)
		{
			x2=-L/2;
			y2=bas;
		}
		else
		{
			x2=(float) ((L/2)*cosf((PI*(m-1))/(def -2)));
			y2=milieu-(float) ((L/2)*sinf((PI*(m-1))/(def -2)));
		}

		Vertices[n].Stok.Init(x,y,-L2/2);
		Vertices[(def+1)+n].Stok.Init(x,y,L2/2);

		Vertices[n].Map.Init((x+(L/2))/L,(y+(h/2)+(L/2))/H);
		Vertices[(def+1)+n].Map.Init((x+(L/2))/L,(y+(h/2)+(L/2))/H);

		Vertices[((def+1)*2) + n*4 +0].Stok.Init(x,y,-L2/2);
		Vertices[((def+1)*2) + n*4 +1].Stok.Init(x2,y2,-L2/2);
		Vertices[((def+1)*2) + n*4 +2].Stok.Init(x2,y2,L2/2);
		Vertices[((def+1)*2) + n*4 +3].Stok.Init(x,y,L2/2);
	}


	m=(def+1);
	for (n=1;n<def;n++)
	{
		Triangle((n-1)*2+0,0,n,n+1);
		Triangle((n-1)*2+1,m+0,m+n+1,m+n);
	}

	nf=(def-1)*2;
	m=(def+1)*2;
	for (n=0;n<=def;n++)
	{
		Carre2(nf,m+n*4+3,m+n*4+2,m+n*4+1,m+n*4+0);
		nf+=2;
	}

	for (n=0;n<nFaces;n++)
	{
		Faces[n].nL=0;
		Faces[n].nT=0;
	}

	SetTag(SET_FACES_TAG+0);
	SetFaces();
	CalculateNormals(-1);
	SetRefTexture();
}


bool CObject3D::EdgeConfondue(CVector a1,CVector b1,CVector a2,CVector b2)
{
	int res;
	CVector u1,u2,i1,i2,u;
	float d;
	float sa1,sb1,sa2,sb2;
	float t1,t2;

	res=0;

	u1=b1 - a1;
	u2=b2 - a2;

	u1.Normalise();
	u2.Normalise();

	if (f_abs(u1||u2)>0.9f)
	{
		d=-(u1||a1);

		sa1=(u1||a1) + d;
		sb1=(u1||b1) + d;

		sa2=(u1||a2) + d;
		sb2=(u1||b2) + d;

		t1=-sa1/(sb1-sa1);
		t2=-sa2/(sb2-sa2);

		i1=a1+t1*(b1-a1);
		i2=a2+t2*(b2-a2);

		u=i2-i1;

		if (u.Norme()<0.1f) res=1;
	}

	return (res==1);
}

bool CObject3D::Linked(CObject3D * obj)
{
	float K=0.1f;
	int n1,n2;
	int res=1;
	CVector u;

	if ((obj->x0>x0-K)&&(obj->x0<x1+K)) res=0;
	if ((obj->y0>y0-K)&&(obj->y0<y1+K)) res=0;
	if ((obj->z0>z0-K)&&(obj->z0<z1+K)) res=0;

	if ((x0>obj->x0-K)&&(x0<obj->x1+K)) res=0;
	if ((y0>obj->y0-K)&&(y0<obj->y1+K)) res=0;
	if ((z0>obj->z0-K)&&(z0<obj->z1+K)) res=0;

	if (res==0)
	{
		n1=0;
		while ((res==0)&&(n1<nVertices))
		{
			n2=0;
			while ((res==0)&&(n2<obj->nVertices))
			{
				VECTORSUB(u,Vertices[n1].Calc,obj->Vertices[n2].Calc);
				if (VECTORNORM(u)<0.1f) res=1;
				n2++;
			}
			n1++;
		}
	}
	else res=0;

	return (res==1);
}

CObject3D * ConvertConnerie(CObject3D *obj)
{
	float scale=0.0001f;
	CObject3D * res;
	int n,nn;
	int c,cc;
	int * corres;
	CVector u;
	CVector2 m;

	res=new CObject3D;

	res->Init(50000,50000);
	res->nVertices=0;
	res->nFaces=0;

	corres=(int *) malloc(obj->nVertices*4);

	for (n=0;n<obj->nVertices;n++)
	{

		obj->Vertices[n].tag=1;
		corres[n]=-1;
		nn=0;
		while ((nn<n)&&(corres[n]==-1))
		{
			if (obj->Vertices[nn].tag==1)
			{
				u=obj->Vertices[n].Stok - obj->Vertices[nn].Stok;
				if (u.Norme()<scale)
				{
					m=obj->Vertices[n].Map - obj->Vertices[nn].Map;

					if ((m.x*m.x + m.y*m.y)<0.0001f)
					{
						obj->Vertices[n].tag=0;
						corres[n]=nn;

					}
				}
			}
			nn++;
		}

	}

	nn=0;
	for (n=0;n<obj->nVertices;n++)
	{
		if (corres[n]==-1)
		{
			res->Vertices[res->nVertices+nn].Stok=obj->Vertices[n].Stok;
			res->Vertices[res->nVertices+nn].Norm=obj->Vertices[n].Norm;
			res->Vertices[res->nVertices+nn].Map=obj->Vertices[n].Map;
			res->Vertices[res->nVertices+nn].Map2=obj->Vertices[n].Map2;
			res->Vertices[res->nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
			obj->Vertices[n].tag2=res->nVertices+nn;
			nn++;
		}

	}
	res->nVertices+=nn;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{

		c=obj->Faces[n].v0;
		if (corres[c]==-1)
		{
			res->Faces[res->nFaces+nn].v0=obj->Faces[n].v[0]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				res->Faces[res->nFaces+nn].v0=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					res->Faces[res->nFaces+nn].v0=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					res->Faces[res->nFaces+nn].v0=(cc&0xFFFF);
				}
			}
		}

		c=obj->Faces[n].v1;
		if (corres[c]==-1)
		{
			res->Faces[res->nFaces+nn].v1=obj->Faces[n].v[1]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				res->Faces[res->nFaces+nn].v1=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					res->Faces[res->nFaces+nn].v1=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					res->Faces[res->nFaces+nn].v1=(cc&0xFFFF);
				}
			}
		}

		c=obj->Faces[n].v2;
		if (corres[c]==-1)
		{
			res->Faces[res->nFaces+nn].v2=obj->Faces[n].v[2]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				res->Faces[res->nFaces+nn].v2=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					res->Faces[res->nFaces+nn].v2=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					res->Faces[res->nFaces+nn].v2=(cc&0xFFFF);
				}
			}
		}

		res->Faces[res->nFaces+nn].Norm=obj->Faces[n].Norm;
		res->Faces[res->nFaces+nn].tag=0;
		res->Faces[res->nFaces+nn].nT=obj->Faces[n].nT;
		res->Faces[res->nFaces+nn].ref=obj->Faces[n].ref;
		res->Faces[res->nFaces+nn].nL=0;

		//res->Faces[res->nFaces+nn].Diffuse=obj->Faces[n].Diffuse;
		res->Faces[res->nFaces+nn].mp0.Init(0,0);
		res->Faces[res->nFaces+nn].mp1.Init(0,0);
		res->Faces[res->nFaces+nn].mp2.Init(0,0);
		nn++;
	}

	res->nFaces+=nn;

	free(corres);

	res->SetFaces();
	res->CalculateNormals(-1);

	res->Rot=obj->Rot;
	res->Coo=obj->Coo;
	for (n=0;n<6;n++)
		res->P[n]=obj->P[n];

	res->nP=obj->nP;
	for (n=0;n<16;n++)
		res->Tab[n]=obj->Tab[n];

	res->Status=obj->Status;
	res->Attribut=obj->Attribut;
	res->Tag=obj->Tag;


	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearNTLV(float coef)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearNTLV(float coef)
{
//	float scale=SMALLF;
	CObject3D * res;
	int n,nn;
	int c;//,cc;
	int * corres;
	CVector u;
	CVector2 m;
	int v1,v2,v0;
	int * tags;
	int tag;
    int ROW=ReorderY();

	res=new CObject3D;

	res->Init(nVertices*2,nFaces*2);
	res->nVertices=0;
	res->nFaces=0;

	corres=(int *) malloc(nVertices*4);
	tags=(int *) malloc(nFaces*4);

	for (n=0;n<nFaces;n++) tags[n]=Faces[n].tag;

	for (n=0;n<nVertices;n++)
    {
        Vertices[n].tag2=0;
        Vertices[n].value=-1;
    }
    
	tag=0;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]->tag=Faces[n].tag;
		Faces[n].v[1]->tag=Faces[n].tag;
		Faces[n].v[2]->tag=Faces[n].tag;
		if (tag<Faces[n].tag) tag=Faces[n].tag;

		Faces[n].v[0]->tag2=Faces[n].v[0]->tag2<<10;
		Faces[n].v[1]->tag2=Faces[n].v[1]->tag2<<10;
		Faces[n].v[2]->tag2=Faces[n].v[2]->tag2<<10;

		Faces[n].v[0]->tag2+=Faces[n].nL;
		Faces[n].v[1]->tag2+=Faces[n].nL;
		Faces[n].v[2]->tag2+=Faces[n].nL;
	}
    
    float sc=SMALLF*coef*SMALLF*coef;

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].tag=1;
		corres[n]=-1;
		nn=n-ROW*2;
        if (nn<0) nn=0;
		while ((nn<n)&&(corres[n]==-1))
		{
			if ((Vertices[nn].tag==1)&&(Vertices[nn].tag2==Vertices[n].tag2))
			{
				VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
				if (VECTORNORM2(u)<sc)
				{
					m=Vertices[n].Map - Vertices[nn].Map;
					if ((m.x*m.x + m.y*m.y)<SMALLF2)
					{
						m=Vertices[n].Map2 - Vertices[nn].Map2;
						if ((m.x*m.x + m.y*m.y)<SMALLF2)
						{
							Vertices[n].tag=0;
							//corres[n]=nn;
                            if (Vertices[nn].value<0)
                            {
                                Vertices[n].value=nn;
                                corres[n]=nn;
                            }
                            else
                            {
                                corres[n]=Vertices[nn].value;
                                Vertices[n].value=corres[n];
                            }
                            
                            break;
						}
					}
				}
			}
			nn++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=tags[n];
	free(tags);

	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (corres[n]==-1)
		{
			res->Vertices[res->nVertices+nn]=Vertices[n];
			Vertices[n].tag2=res->nVertices+nn;
			nn++;
		}
	}

	res->nVertices+=nn;

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		c=Faces[n].v0;
		if (corres[c]==-1) v0=Vertices[c].tag2;
		else v0=Vertices[corres[c]].tag2;

		c=Faces[n].v1;
		if (corres[c]==-1) v1=Vertices[c].tag2;
		else v1=Vertices[corres[c]].tag2;

		c=Faces[n].v2;
		if (corres[c]==-1) v2=Vertices[c].tag2;
		else v2=Vertices[corres[c]].tag2;


		if ((v0==v1)||(v2==v1)||(v0==v2))
		{
		}
		else
		{
			res->Faces[res->nFaces+nn]=Faces[n];

			res->Faces[res->nFaces+nn].v0=v0;
			res->Faces[res->nFaces+nn].v1=v1;
			res->Faces[res->nFaces+nn].v2=v2;

			res->Faces[res->nFaces+nn].mp0.Init(0,0);
			res->Faces[res->nFaces+nn].mp1.Init(0,0);
			res->Faces[res->nFaces+nn].mp2.Init(0,0);
			nn++;
		}
	}

	res->nFaces+=nn;

	free(corres);

	res->SetFaces();
	res->CalculateNormals(-1);

	res->Rot=Rot;
	res->Coo=Coo;
	for (n=0;n<6;n++) res->P[n]=P[n];
	res->nP=nP;
	for (n=0;n<16;n++) res->Tab[n]=Tab[n];
	res->Status=Status;
	res->Attribut=Attribut;
	res->Tag=Tag;
	res->Affect=0;
	res->nurbs=nurbs;

	return res;
}

float CObject3D::SetTagTopo()
{
    int n;
    float r;
    float rmax=0;
    
    for (n=0;n<nVertices;n++)
    {
        r=VECTORNORM2(Vertices[n].Stok);
        if (r>rmax) rmax=r;
    }

    r=256.0f/sqrtf(rmax);
    for (n=0;n<nVertices;n++)
    {
        Vertices[n].tag2=((int)((256+Vertices[n].Stok.x*r)))+512*(((int)((256+Vertices[n].Stok.y*r))) + 512*((int)((256+Vertices[n].Stok.z*r))));
    }
    
    return r;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearFull(float scale,float scalem)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearFull(float scale,float scalem)
{
    CObject3D * res;
    int n,nn;
    int c;
    int * corres;
    int v0,v1,v2;
    CVector u;
    CVector2 m;
    
    int ROW=nVertices;

    ROW=ReorderY();
    
    if (ROW>nVertices) ROW=nVertices;
    
    int partial=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag!=0) partial=1;
    }

    //if (scalem>=0) for (n=0;n<nVertices;n++) Vertices[n].Map2.Init(0,0);

    if (partial)
    {
        for (n=0;n<nVertices;n++) Vertices[n].tag=0;
        for (n=0;n<nFaces;n++)
        if (Faces[n].tag!=0)
        {
            Faces[n].v[0]->tag=1;
            Faces[n].v[1]->tag=1;
            Faces[n].v[2]->tag=1;
        }
    }
    else
    {
        for (n=0;n<nVertices;n++) Vertices[n].tag=1;
    }
    res=new CObject3D;
    res->Init(nVertices*2,nFaces*2);
    res->nVertices=0;
    res->nFaces=0;
    corres=(int *) malloc(nVertices*4);

    for (n=0;n<nVertices;n++)
    {
        corres[n]=-1;
        Vertices[n].value=-1;
    }

    float sc=scale*scale;
    float scm=scalem*scalem;
    
    if (scalem<0) scm=1000*1000;

    for (n=0;n<nVertices;n++)
    if (Vertices[n].tag==1)
    {
        nn=n-ROW*2;
        if (nn<0) nn=0;
        while ((nn<n)&&(corres[n]==-1))
        {
            VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
            if (VECTORNORM2(u)<sc)  //<scale
            {
                m.x=Vertices[n].Map.x - Vertices[nn].Map.x;
                m.y=Vertices[n].Map.y - Vertices[nn].Map.y;
            
                if ((m.x*m.x + m.y*m.y)<scm) //scalem
                {
                    Vertices[n].tag=0;
                    if (Vertices[nn].value<0)
                    {
                        Vertices[n].value=nn;
                        corres[n]=nn;
                    }
                    else
                    {
                        corres[n]=Vertices[nn].value;
                        Vertices[n].value=corres[n];
                    }
                    break;
                }
            }
            nn++;
        }
    }

    nn=0;
    for (n=0;n<nVertices;n++)
    {
        if (corres[n]==-1)
        {
            res->Vertices[res->nVertices+nn].Stok=Vertices[n].Stok;
            res->Vertices[res->nVertices+nn].Norm=Vertices[n].Norm;
            res->Vertices[res->nVertices+nn].Map=Vertices[n].Map;
            //res->Vertices[res->nVertices+nn].Map2=Vertices[n].Map2;
            res->Vertices[res->nVertices+nn].tag=Vertices[n].tag;
            res->Vertices[res->nVertices+nn].tag2=Vertices[n].tag2;
            res->Vertices[res->nVertices+nn].temp=Vertices[n].temp;
            res->Vertices[res->nVertices+nn].Diffuse=Vertices[n].Diffuse;
            res->Vertices[res->nVertices+nn].Index[0]=Vertices[n].Index[0];
            res->Vertices[res->nVertices+nn].Index[1]=Vertices[n].Index[1];
            res->Vertices[res->nVertices+nn].Index[2]=Vertices[n].Index[2];
            res->Vertices[res->nVertices+nn].Index[3]=Vertices[n].Index[3];
            res->Vertices[res->nVertices+nn].Weight[0]=Vertices[n].Weight[0];
            res->Vertices[res->nVertices+nn].Weight[1]=Vertices[n].Weight[1];
            res->Vertices[res->nVertices+nn].Weight[2]=Vertices[n].Weight[2];
            res->Vertices[res->nVertices+nn].Weight[3]=Vertices[n].Weight[3];
            Vertices[n].tag2=res->nVertices+nn;
            nn++;
        }
    }
    
    if (nKeys>0)
    {
        for (int k=0;k<nKeys;k++)
        {
            res->VerticesKeys[k]=new CShortVertex[nn];
            int nk=0;
            for (n=0;n<nVertices;n++)
            {
                if (corres[n]==-1)
                {
                    res->VerticesKeys[k][res->nVertices+nk].Stok=VerticesKeys[k][n].Stok;
                    res->VerticesKeys[k][res->nVertices+nk].Norm=VerticesKeys[k][n].Norm;
                    nk++;
                }
            }
        }
        res->nKeys=nKeys;
    }

    res->nVertices+=nn;

    nn=0;
    for (n=0;n<nFaces;n++)
    {
        c=Faces[n].v0;
        if (corres[c]==-1) v0=Faces[n].v[0]->tag2;
        else v0=Vertices[corres[c]].tag2;

        c=Faces[n].v1;
        if (corres[c]==-1) v1=Faces[n].v[1]->tag2;
        else v1=Vertices[corres[c]].tag2;

        c=Faces[n].v2;
        if (corres[c]==-1) v2=Faces[n].v[2]->tag2;
        else v2=Vertices[corres[c]].tag2;

        res->Faces[res->nFaces+nn].v0=v0;
        res->Faces[res->nFaces+nn].v1=v1;
        res->Faces[res->nFaces+nn].v2=v2;

        res->Faces[res->nFaces+nn].Norm=Faces[n].Norm;
        res->Faces[res->nFaces+nn].tag=Faces[n].tag;
        res->Faces[res->nFaces+nn].tag2=Faces[n].tag2;
        res->Faces[res->nFaces+nn].tag3=Faces[n].tag3;
        res->Faces[res->nFaces+nn].ref=Faces[n].ref;
        res->Faces[res->nFaces+nn].lock=Faces[n].lock;
        res->Faces[res->nFaces+nn].nT=Faces[n].nT;
        res->Faces[res->nFaces+nn].nT2=Faces[n].nT2;
        res->Faces[res->nFaces+nn].nL=Faces[n].nL;
        res->Faces[res->nFaces+nn].nLVectors=Faces[n].nLVectors;

        //res->Faces[res->nFaces+nn].Diffuse=Faces[n].Diffuse;
        res->Faces[res->nFaces+nn].mp0=Faces[n].mp0;
        res->Faces[res->nFaces+nn].mp1=Faces[n].mp1;
        res->Faces[res->nFaces+nn].mp2=Faces[n].mp2;
        
        res->Faces[res->nFaces+nn].coef1=Faces[n].coef1;
        nn++;
    }

    res->nFaces+=nn;

    free(corres);
    
    if (res->nFaces==0)
    {
        res->Free();
        delete res;
        return NULL;
    }
    
    res->SetFaces();
    res->CalculateNormals(-1);
    res->Rot=Rot;
    res->Coo=Coo;
    for (n=0;n<6;n++) res->P[n]=P[n];
    res->nP=nP;
    for (n=0;n<16;n++) res->Tab[n]=Tab[n];
    res->Status=Status;
    res->Status2=Status2;
    res->Attribut=Attribut;
    res->Tag=Tag;
    res->env_mapping=env_mapping;
    memcpy(res->Name,Name,128);
    res->anim_strings=anim_strings;
    anim_strings=NULL;
    res->group=group;
    res->nurbs=nurbs;

    return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearFullNorm(float scale,float scalem)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearFullNorm(float scale,float scalem)
{
    CObject3D * res;
    int n,nn;
    int c;
    int * corres;
    int v0,v1,v2;
    CVector u;
    CVector2 m;
    
    int ROW=nVertices;

    ROW=ReorderY();
    
    if (ROW>nVertices) ROW=nVertices;
    
    int partial=0;
    for (n=0;n<nFaces;n++)
    {
        if (Faces[n].tag!=0) partial=1;
    }

    //if (scalem>=0) for (n=0;n<nVertices;n++) Vertices[n].Map2.Init(0,0);

    if (partial)
    {
        for (n=0;n<nVertices;n++) Vertices[n].tag=0;
        for (n=0;n<nFaces;n++)
        if (Faces[n].tag!=0)
        {
            Faces[n].v[0]->tag=1;
            Faces[n].v[1]->tag=1;
            Faces[n].v[2]->tag=1;
        }
    }
    else
    {
        for (n=0;n<nVertices;n++) Vertices[n].tag=1;
    }
    res=new CObject3D;
    res->Init(nVertices*2,nFaces*2);
    res->nVertices=0;
    res->nFaces=0;
    corres=(int *) malloc(nVertices*4);

    for (n=0;n<nVertices;n++)
    {
        corres[n]=-1;
        Vertices[n].value=-1;
    }

    float sc=scale*scale;
    float scm=scalem*scalem;
    
    if (scalem<0) scm=1000*1000;

    for (n=0;n<nVertices;n++)
    if (Vertices[n].tag==1)
    {
        nn=n-ROW*2;
        if (nn<0) nn=0;
        while ((nn<n)&&(corres[n]==-1))
        {
            VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
            if (VECTORNORM2(u)<sc)  //<scale
            {
                if (DOT(Vertices[n].Norm,Vertices[nn].Norm)>0.7f)
                {
                    m.x=Vertices[n].Map.x - Vertices[nn].Map.x;
                    m.y=Vertices[n].Map.y - Vertices[nn].Map.y;
                
                    if ((m.x*m.x + m.y*m.y)<scm) //scalem
                    {
                        Vertices[n].tag=0;
                        if (Vertices[nn].value<0)
                        {
                            Vertices[n].value=nn;
                            corres[n]=nn;
                        }
                        else
                        {
                            corres[n]=Vertices[nn].value;
                            Vertices[n].value=corres[n];
                        }
                        break;
                    }
                }
            }
            nn++;
        }
    }

    nn=0;
    for (n=0;n<nVertices;n++)
    {
        if (corres[n]==-1)
        {
            res->Vertices[res->nVertices+nn].Stok=Vertices[n].Stok;
            res->Vertices[res->nVertices+nn].Norm=Vertices[n].Norm;
            res->Vertices[res->nVertices+nn].Map=Vertices[n].Map;
            //res->Vertices[res->nVertices+nn].Map2=Vertices[n].Map2;
            res->Vertices[res->nVertices+nn].tag=Vertices[n].tag;
            res->Vertices[res->nVertices+nn].tag2=Vertices[n].tag2;
            res->Vertices[res->nVertices+nn].temp=Vertices[n].temp;
            res->Vertices[res->nVertices+nn].Diffuse=Vertices[n].Diffuse;
            res->Vertices[res->nVertices+nn].Index[0]=Vertices[n].Index[0];
            res->Vertices[res->nVertices+nn].Index[1]=Vertices[n].Index[1];
            res->Vertices[res->nVertices+nn].Index[2]=Vertices[n].Index[2];
            res->Vertices[res->nVertices+nn].Index[3]=Vertices[n].Index[3];
            res->Vertices[res->nVertices+nn].Weight[0]=Vertices[n].Weight[0];
            res->Vertices[res->nVertices+nn].Weight[1]=Vertices[n].Weight[1];
            res->Vertices[res->nVertices+nn].Weight[2]=Vertices[n].Weight[2];
            res->Vertices[res->nVertices+nn].Weight[3]=Vertices[n].Weight[3];
            Vertices[n].tag2=res->nVertices+nn;
            nn++;
        }
    }
    
    if (nKeys>0)
    {
        for (int k=0;k<nKeys;k++)
        {
            res->VerticesKeys[k]=new CShortVertex[nn];
            int nk=0;
            for (n=0;n<nVertices;n++)
            {
                if (corres[n]==-1)
                {
                    res->VerticesKeys[k][res->nVertices+nk].Stok=VerticesKeys[k][n].Stok;
                    res->VerticesKeys[k][res->nVertices+nk].Norm=VerticesKeys[k][n].Norm;
                    nk++;
                }
            }
        }
        res->nKeys=nKeys;
    }

    res->nVertices+=nn;

    nn=0;
    for (n=0;n<nFaces;n++)
    {
        c=Faces[n].v0;
        if (corres[c]==-1) v0=Faces[n].v[0]->tag2;
        else v0=Vertices[corres[c]].tag2;

        c=Faces[n].v1;
        if (corres[c]==-1) v1=Faces[n].v[1]->tag2;
        else v1=Vertices[corres[c]].tag2;

        c=Faces[n].v2;
        if (corres[c]==-1) v2=Faces[n].v[2]->tag2;
        else v2=Vertices[corres[c]].tag2;

        res->Faces[res->nFaces+nn].v0=v0;
        res->Faces[res->nFaces+nn].v1=v1;
        res->Faces[res->nFaces+nn].v2=v2;

        res->Faces[res->nFaces+nn].Norm=Faces[n].Norm;
        res->Faces[res->nFaces+nn].tag=Faces[n].tag;
        res->Faces[res->nFaces+nn].tag2=Faces[n].tag2;
        res->Faces[res->nFaces+nn].tag3=Faces[n].tag3;
        res->Faces[res->nFaces+nn].ref=Faces[n].ref;
        res->Faces[res->nFaces+nn].lock=Faces[n].lock;
        res->Faces[res->nFaces+nn].nT=Faces[n].nT;
        res->Faces[res->nFaces+nn].nT2=Faces[n].nT2;
        res->Faces[res->nFaces+nn].nL=Faces[n].nL;
        res->Faces[res->nFaces+nn].nLVectors=Faces[n].nLVectors;

        //res->Faces[res->nFaces+nn].Diffuse=Faces[n].Diffuse;
        res->Faces[res->nFaces+nn].mp0=Faces[n].mp0;
        res->Faces[res->nFaces+nn].mp1=Faces[n].mp1;
        res->Faces[res->nFaces+nn].mp2=Faces[n].mp2;
        
        res->Faces[res->nFaces+nn].coef1=Faces[n].coef1;
        nn++;
    }

    res->nFaces+=nn;

    free(corres);
    
    if (res->nFaces==0)
    {
        res->Free();
        delete res;
        return NULL;
    }
    
    res->SetFaces();
    res->CalculateNormals(-1);
    res->Rot=Rot;
    res->Coo=Coo;
    for (n=0;n<6;n++) res->P[n]=P[n];
    res->nP=nP;
    for (n=0;n<16;n++) res->Tab[n]=Tab[n];
    res->Status=Status;
    res->Status2=Status2;
    res->Attribut=Attribut;
    res->Tag=Tag;
    res->env_mapping=env_mapping;
    memcpy(res->Name,Name,128);
    res->anim_strings=anim_strings;
    anim_strings=NULL;
    res->group=group;
    res->nurbs=nurbs;

    return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinear(float coef)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinear(float coef)
{
	CObject3D * res;
	int n,nn;
	int * corres;
	CVector u,v;
	CVector2 m;
	int v1,v2,v0;
	int * tags;
	int tag;
	float xx=SMALLF*coef;
    int ROW=ReorderY();
    if (ROW>nVertices) ROW=nVertices;

	res=new CObject3D;

	res->Init(nVertices*2,nFaces*2);
	res->nVertices=0;
	res->nFaces=0;

	corres=(int *) malloc(nVertices*4);
	tags=(int *) malloc(nFaces*4);

	for (n=0;n<nFaces;n++) tags[n]=Faces[n].tag;

	tag=0;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]->temp=Faces[n].tag;
		Faces[n].v[1]->temp=Faces[n].tag;
		Faces[n].v[2]->temp=Faces[n].tag;
		if (tag<Faces[n].tag) tag=Faces[n].tag;
	}

	for (n=0;n<nVertices;n++) Vertices[n].value=-1;

	for (n=0;n<nVertices;n++)
	{
		int temp=Vertices[n].temp;
		v=Vertices[n].Stok;
		Vertices[n].tag=1;
		corres[n]=-1;
		nn=n-2*ROW;
        if (nn<0) nn=0;
		while (nn<n)
		{
			if (Vertices[nn].temp==temp)
			{
				VECTORSUB(u,v,Vertices[nn].Stok);
				if (VECTORNORM2(u)<xx)
				{
					VECTOR2SUB(m,Vertices[n].Map,Vertices[nn].Map);
					if ((m.x*m.x + m.y*m.y)<SMALLF2)
					{
                        Vertices[n].tag=0;
//							corres[n]=nn;
                        if (Vertices[nn].value<0)
                        {
                            Vertices[n].value=nn;
                            corres[n]=nn;
                        }
                        else
                        {
                            corres[n]=Vertices[nn].value;
                            Vertices[n].value=corres[n];
                        }

                        break;
					}
				}
			}
			nn++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=tags[n];
	free(tags);

	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (corres[n]==-1)
		{
			res->Vertices[res->nVertices+nn]=Vertices[n];
			Vertices[n].tag2=res->nVertices+nn;
			corres[n]=n;
			nn++;			
		}
	}

	res->nVertices+=nn;

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		v0=Vertices[corres[Faces[n].v0]].tag2;
		v1=Vertices[corres[Faces[n].v1]].tag2;
		v2=Vertices[corres[Faces[n].v2]].tag2;

		if ((v0!=v1)&&(v2!=v1)&&(v0!=v2))
		{
			res->Faces[nn]=Faces[n];
			res->Faces[nn].v0=v0;
			res->Faces[nn].v1=v1;
			res->Faces[nn].v2=v2;
			nn++;
		}
	}

	res->nFaces=nn;

	free(corres);

	res->SetFaces();
	res->CalculateNormals(-1);

	res->Rot=Rot;
	res->Coo=Coo;
	for (n=0;n<6;n++) res->P[n]=P[n];
	res->nP=nP;
	for (n=0;n<16;n++) res->Tab[n]=Tab[n];
	res->Status=Status;
	res->Attribut=Attribut;
	res->nurbs=nurbs;
	res->Tag=Tag;
	res->env_mapping=env_mapping;

	naming(res->Name,Name);

	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearMap2(float coef)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearMap2(float coef)
{
    CObject3D * res;
    int n,nn;
    int * corres;
    CVector u,v;
    CVector2 m;
    int v1,v2,v0;
    int * tags;
    int tag;
    float xx=SMALLF*coef;
    int ROW=ReorderY();
    if (ROW>nVertices) ROW=nVertices;

    res=new CObject3D;

    res->Init(nVertices*2,nFaces*2);
    res->nVertices=0;
    res->nFaces=0;

    corres=(int *) malloc(nVertices*4);
    tags=(int *) malloc(nFaces*4);

    for (n=0;n<nFaces;n++) tags[n]=Faces[n].tag;

    tag=0;
    for (n=0;n<nFaces;n++)
    {
        Faces[n].v[0]->temp=Faces[n].tag;
        Faces[n].v[1]->temp=Faces[n].tag;
        Faces[n].v[2]->temp=Faces[n].tag;
        if (tag<Faces[n].tag) tag=Faces[n].tag;
    }

    for (n=0;n<nVertices;n++) Vertices[n].value=-1;

    for (n=0;n<nVertices;n++)
    {
        int temp=Vertices[n].temp;
        v=Vertices[n].Stok;
        Vertices[n].tag=1;
        corres[n]=-1;
        nn=n-2*ROW;
        if (nn<0) nn=0;
        while (nn<n)
        {
            if (Vertices[nn].temp==temp)
            {
                VECTORSUB(u,v,Vertices[nn].Stok);
                if (VECTORNORM2(u)<xx)
                {
                    VECTOR2SUB(m,Vertices[n].Map,Vertices[nn].Map);
                    if ((m.x*m.x + m.y*m.y)<SMALLF2)
                    {
                        VECTOR2SUB(m,Vertices[n].Map2,Vertices[nn].Map2);
                        if ((m.x*m.x + m.y*m.y)<SMALLF2)
                        {
                            Vertices[n].tag=0;
//                            corres[n]=nn;
                            if (Vertices[nn].value<0)
                            {
                                Vertices[n].value=nn;
                                corres[n]=nn;
                            }
                            else
                            {
                                corres[n]=Vertices[nn].value;
                                Vertices[n].value=corres[n];
                            }

                            break;
                        }
                    }
                }
            }
            nn++;
        }
    }

    for (n=0;n<nFaces;n++) Faces[n].tag=tags[n];
    free(tags);

    nn=0;
    for (n=0;n<nVertices;n++)
    {
        if (corres[n]==-1)
        {
            res->Vertices[res->nVertices+nn]=Vertices[n];
            Vertices[n].tag2=res->nVertices+nn;
            corres[n]=n;
            nn++;
        }
    }

    res->nVertices+=nn;

    nn=0;
    for (n=0;n<nFaces;n++)
    {
        v0=Vertices[corres[Faces[n].v0]].tag2;
        v1=Vertices[corres[Faces[n].v1]].tag2;
        v2=Vertices[corres[Faces[n].v2]].tag2;

        if ((v0!=v1)&&(v2!=v1)&&(v0!=v2))
        {
            res->Faces[nn]=Faces[n];
            res->Faces[nn].v0=v0;
            res->Faces[nn].v1=v1;
            res->Faces[nn].v2=v2;
            nn++;
        }
    }

    res->nFaces=nn;

    free(corres);

    res->SetFaces();
    res->CalculateNormals(-1);

    res->Rot=Rot;
    res->Coo=Coo;
    for (n=0;n<6;n++) res->P[n]=P[n];
    res->nP=nP;
    for (n=0;n<16;n++) res->Tab[n]=Tab[n];
    res->Status=Status;
    res->Attribut=Attribut;
    res->nurbs=nurbs;
    res->Tag=Tag;
    res->env_mapping=env_mapping;

    naming(res->Name,Name);

    return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearSimple(float coef)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearSimple(float coef)
{
	CObject3D * res;
	int n,nn;
	int c;
	int * corres;
	CVector u;
	CVector2 m;
	int v1,v2,v0;
	int * tags;
	int tag,k;
    int ROW=ReorderY();

	res=new CObject3D;

	res->Init(nVertices*2,nFaces*2);
	res->nVertices=0;
	res->nFaces=0;

	corres=(int *) malloc(nVertices*4);
	tags=(int *) malloc(nFaces*4);

	for (n=0;n<nFaces;n++) tags[n]=Faces[n].tag;

	tag=0;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]->tag=Faces[n].tag;
		Faces[n].v[1]->tag=Faces[n].tag;
		Faces[n].v[2]->tag=Faces[n].tag;
		if (tag<Faces[n].tag) tag=Faces[n].tag;
	}
    
    for (n=0;n<nVertices;n++) Vertices[n].value=-1;

    for (n=0;n<nVertices;n++)
	{
		Vertices[n].tag=1;
		corres[n]=-1;
		nn=n-ROW*2;
        if (nn<0) nn=0;
		while ((nn<n)&&(corres[n]==-1))
		{
			if (Vertices[nn].tag==1)
			{
				VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
				if (VECTORNORM(u)<SMALLF*coef)
				{
					Vertices[n].tag=0;
					//corres[n]=nn;
                    if (Vertices[nn].value<0)
                    {
                        Vertices[n].value=nn;
                        corres[n]=nn;
                    }
                    else
                    {
                        corres[n]=Vertices[nn].value;
                        Vertices[n].value=corres[n];
                    }
                    
                    break;
				}
			}
			nn++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=tags[n];
	free(tags);


	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (corres[n]==-1)
		{
			res->Vertices[res->nVertices+nn]=Vertices[n];
			Vertices[n].tag2=res->nVertices+nn;
			nn++;
		}
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			res->VerticesKeys[k]=new CShortVertex[nn];

			nn=0;
			for (n=0;n<nVertices;n++)
			{
				if (corres[n]==-1)
				{
					res->VerticesKeys[k][res->nVertices+nn]=VerticesKeys[k][n];
					nn++;
				}
			}
		}
	}

	res->nVertices+=nn;

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		c=Faces[n].v0;
		if (corres[c]==-1) v0=Vertices[c].tag2;
		else v0=Vertices[corres[c]].tag2;

		c=Faces[n].v1;
		if (corres[c]==-1) v1=Vertices[c].tag2;
		else v1=Vertices[corres[c]].tag2;

		c=Faces[n].v2;
		if (corres[c]==-1) v2=Vertices[c].tag2;
		else v2=Vertices[corres[c]].tag2;


		if ((v0==v1)||(v2==v1)||(v0==v2))
		{
		}
		else
		{

			res->Faces[res->nFaces+nn]=Faces[n];

			res->Faces[res->nFaces+nn].v0=v0;
			res->Faces[res->nFaces+nn].v1=v1;
			res->Faces[res->nFaces+nn].v2=v2;

			res->Faces[res->nFaces+nn].mp0.Init(0,0);
			res->Faces[res->nFaces+nn].mp1.Init(0,0);
			res->Faces[res->nFaces+nn].mp2.Init(0,0);
			nn++;
		}
	}

	res->nFaces+=nn;

	free(corres);

	res->SetFaces();
	res->CalculateNormals(-1);

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			for (n=0;n<res->nVertices;n++) res->Vertices[n].Stok=res->VerticesKeys[k][n].Stok;
			res->CalculateNormals(-1);
			for (n=0;n<res->nVertices;n++) res->VerticesKeys[k][n].Norm=res->Vertices[n].Norm;
		}

		for (n=0;n<res->nVertices;n++) res->Vertices[n].Stok=res->VerticesKeys[0][n].Stok;
		res->CalculateNormals(-1);

		res->nKeys=nKeys;
		res->anim_strings=anim_strings;
	}


	res->Rot=Rot;
	res->Coo=Coo;

	for (n=0;n<6;n++)
		res->P[n]=P[n];

	res->nP=nP;

	for (n=0;n<16;n++)
		res->Tab[n]=Tab[n];

	res->Status=Status;
	res->Attribut=Attribut;
	res->nurbs=nurbs;
	res->Tag=Tag;
	res->env_mapping=env_mapping;

	naming(res->Name,Name);

	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

    CObject3D * ConvertLinearSimple_m(float coef)
 
    Usage:

        object traitment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CObject3D::ConvertLinearSimple_m(float coef,float coefm)
{
//	float scale=SMALLF;
	CObject3D * res;
	int n,nn;
	int c;//,cc;
	int * corres;
	CVector u;
	CVector2 m;
	int v1,v2,v0;
	int * tags;
	int tag,k;
    int ROW=ReorderY();

	res=new CObject3D;

	res->Init(nVertices*2,nFaces*2);
	res->nVertices=0;
	res->nFaces=0;

	corres=(int *) malloc(nVertices*4);
	tags=(int *) malloc(nFaces*4);

	for (n=0;n<nFaces;n++) tags[n]=Faces[n].tag;

	tag=0;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]->tag=Faces[n].tag;
		Faces[n].v[1]->tag=Faces[n].tag;
		Faces[n].v[2]->tag=Faces[n].tag;
		if (tag<Faces[n].tag) tag=Faces[n].tag;
	}

    
    float sc=coef*coef;
    float scm=coefm*coefm;
    
    for (n=0;n<nVertices;n++) Vertices[n].value=-1;

	for (n=0;n<nVertices;n++)
	{
		Vertices[n].tag=1;
		corres[n]=-1;
		nn=n-2*ROW;
        if (nn<0) nn=0;
		while ((nn<n)&&(corres[n]==-1))
		{
			if (Vertices[nn].tag==1)
			{
				VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
				if (VECTORNORM2(u)<sc)
				{
                    m.x=Vertices[n].Map.x - Vertices[nn].Map.x;
                    m.y=Vertices[n].Map.y - Vertices[nn].Map.y;
                    
                    if ((m.x*m.x + m.y*m.y)<scm)
					{
						Vertices[n].tag=0;
						//corres[n]=nn;
                        if (Vertices[nn].value<0)
                        {
                            Vertices[n].value=nn;
                            corres[n]=nn;
                        }
                        else
                        {
                            corres[n]=Vertices[nn].value;
                            Vertices[n].value=corres[n];
                        }
                        
                        break;
					}
				}
			}
			nn++;
		}
	}

	for (n=0;n<nFaces;n++) Faces[n].tag=tags[n];
	free(tags);

	nn=0;
	for (n=0;n<nVertices;n++)
	{
		if (corres[n]==-1)
		{
			res->Vertices[res->nVertices+nn]=Vertices[n];
			Vertices[n].tag2=res->nVertices+nn;
			nn++;
		}
	}

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			res->VerticesKeys[k]=new CShortVertex[nn];

			nn=0;
			for (n=0;n<nVertices;n++)
			{
				if (corres[n]==-1)
				{
					res->VerticesKeys[k][res->nVertices+nn]=VerticesKeys[k][n];
					nn++;
				}
			}
		}
	}

	res->nVertices+=nn;

	nn=0;
	for (n=0;n<nFaces;n++)
	{
		c=Faces[n].v0;
		if (corres[c]==-1) v0=Vertices[c].tag2;
		else v0=Vertices[corres[c]].tag2;

		c=Faces[n].v1;
		if (corres[c]==-1) v1=Vertices[c].tag2;
		else v1=Vertices[corres[c]].tag2;

		c=Faces[n].v2;
		if (corres[c]==-1) v2=Vertices[c].tag2;
		else v2=Vertices[corres[c]].tag2;


		if ((v0==v1)||(v2==v1)||(v0==v2))
		{
		}
		else
		{

			res->Faces[res->nFaces+nn]=Faces[n];

			res->Faces[res->nFaces+nn].v0=v0;
			res->Faces[res->nFaces+nn].v1=v1;
			res->Faces[res->nFaces+nn].v2=v2;

			res->Faces[res->nFaces+nn].mp0.Init(0,0);
			res->Faces[res->nFaces+nn].mp1.Init(0,0);
			res->Faces[res->nFaces+nn].mp2.Init(0,0);
			nn++;
		}
	}

	res->nFaces+=nn;

	free(corres);

	res->SetFaces();
	res->CalculateNormals(-1);

	if (nKeys>0)
	{
		for (k=0;k<nKeys;k++)
		{
			for (n=0;n<res->nVertices;n++) res->Vertices[n].Stok=res->VerticesKeys[k][n].Stok;
			res->CalculateNormals(-1);
			for (n=0;n<res->nVertices;n++) res->VerticesKeys[k][n].Norm=res->Vertices[n].Norm;
		}

		for (n=0;n<res->nVertices;n++) res->Vertices[n].Stok=res->VerticesKeys[0][n].Stok;
		res->CalculateNormals(-1);

		res->nKeys=nKeys;
		res->anim_strings=anim_strings;
	}


	res->Rot=Rot;
	res->Coo=Coo;

	for (n=0;n<6;n++)
		res->P[n]=P[n];

	res->nP=nP;

	for (n=0;n<16;n++)
		res->Tab[n]=Tab[n];

	res->Status=Status;
	res->Attribut=Attribut;
	res->nurbs=nurbs;
	res->Tag=Tag;
	res->env_mapping=env_mapping;

	naming(res->Name,Name);

	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		BSP creation from an object.
		Collision meshes contain both object and bsp.

		void CCollisionMesh::Init(CObject3D * objet,float r,float f,int NITER)

		NITER : profondeur de l'arbre

		collisionmesh for physic

			r : restitution
			f : friction


	Usage:

		void LightShadowsMeshMapping(int nLight,CObject3D * obj,CCollisionMesh * mesh);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CCollisionMesh::Init(CObject3D * objet,float r,float f,int NITER)
{
	initialised=true;
	Restitution=r;
	Friction=f;
	obj=objet;
	quad.Build(NITER,obj);
	obj->Calculate();
	M.Id();
	M.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


		BSP creation from an object.
		Collision meshes contain both object and bsp.
		This function, merge coplanar,same .nT faces in figures.


		void CCollisionMesh::Init2(CObject3D * objet,float r,float f,int NITER)
		void CCollisionMesh::Init3(CObject3D * objet,float r,float f,int NITER)

		NITER : profondeur de l'arbre

		collisionmesh for physic

			r : restitution
			f : friction

		init3() pour tag!=0

	Usage:

		void LightShadowsMeshMapping2(int nLight,CObject3D * objb,CCollisionMesh * mesh);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CCollisionMesh::Init2(CObject3D * objet,float r,float f,int NITER)
{
	initialised=true;
	Restitution=r;
	Friction=f;
	obj=objet;
	quad.BuildWithFigures(NITER,obj);
	obj->Calculate();
	M.Id();
	M.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
}


void CCollisionMesh::Init3(CObject3D * objet,float r,float f,int NITER)
{
	initialised=true;
	Restitution=r;
	Friction=f;
	obj=objet;
	quad.BuildWithFigures2(NITER,obj);
	obj->Calculate();
	M.Id();
	M.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		The functionnality of collision mesh is to set a user defined matrix instead of object one

		void CCollisionMesh::OwnerMatrix(CMatrix m)

	Usage:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CCollisionMesh::OwnerMatrix(CMatrix m)
{
	ownermatrix=true;
	M=m;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		The functionnality of collision mesh is to get a dynamic bsp according to object moving.

		void CCollisionMesh::Actualise(int tag)

	Usage:

		update matrices

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CCollisionMesh::Actualise(int tag)
{
	int n;
	float mx,my,mz;
	float Mx,My,Mz;
	CVector g;
	CMatrix MR;

    if (obj->nKeys>0)
    {
        if (ownermatrix)
        {
            MR=M;
            MR.a[3][0]=0;
            MR.a[3][1]=0;
            MR.a[3][2]=0;
        }
        else
        {
            MR.Id();
            MR.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
            M=MR;
            M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
        }

        CVector u;
        for (n=0;n<obj->nVertices;n++)
        {
            u=obj->VerticesKeys[obj->key1][n].Stok+obj->t*(obj->VerticesKeys[obj->key2][n].Stok-obj->VerticesKeys[obj->key1][n].Stok);
            obj->Vertices[n].Calc=u*M;
            u=obj->VerticesKeys[obj->key1][n].Norm+obj->t*(obj->VerticesKeys[obj->key2][n].Norm-obj->VerticesKeys[obj->key1][n].Norm);
            obj->Vertices[n].NormCalc=u*MR;
        }
        
        for (n=0;n<obj->nFaces;n++)
        {
            obj->Faces[n].NormCalc=obj->Faces[n].v[0]->NormCalc+obj->Faces[n].v[1]->NormCalc+obj->Faces[n].v[2]->NormCalc;
            obj->Faces[n].NormCalc.Normalise();
        }
    }
    else
    {
        if (ownermatrix)
        {
            MR=M;
            MR.a[3][0]=0;
            MR.a[3][1]=0;
            MR.a[3][2]=0;
            obj->Calculate(&M,&MR);
        }
        else
        {
            obj->Calculate();
            M.Id();
            M.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
            M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
        }
    }

	bounds.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);

	for (n=0;n<obj->nVertices;n++)
	{
		if (f_abs(obj->Vertices[n].Stok.x)>bounds.x) bounds.x=f_abs(obj->Vertices[n].Stok.x);
		if (f_abs(obj->Vertices[n].Stok.y)>bounds.y) bounds.y=f_abs(obj->Vertices[n].Stok.y);
		if (f_abs(obj->Vertices[n].Stok.z)>bounds.z) bounds.z=f_abs(obj->Vertices[n].Stok.z);
	}

	Radius=sqrtf(bounds.x*bounds.x +bounds.y*bounds.y +bounds.z*bounds.z);
	if (bounds.x<SMALLF2) bounds.x=SMALLF2;
	if (bounds.y<SMALLF2) bounds.y=SMALLF2;
	if (bounds.z<SMALLF2) bounds.z=SMALLF2;

	boundsinit=true;

	for (n=0;n<obj->nFaces;n++)
	{
		VECTORADDDIV3(g,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[2]->Calc);

		obj->Faces[n].mp0.x=g.x;
		obj->Faces[n].mp0.y=g.y;
		obj->Faces[n].mp1.x=g.z;

		mx=g.x; my=g.y; mz=g.z;
		Mx=g.x; My=g.y; Mz=g.z;

		if (mx>obj->Faces[n].v[0]->Calc.x) mx=obj->Faces[n].v[0]->Calc.x;
		if (my>obj->Faces[n].v[0]->Calc.y) my=obj->Faces[n].v[0]->Calc.y;
		if (mz>obj->Faces[n].v[0]->Calc.z) mz=obj->Faces[n].v[0]->Calc.z;
		if (Mx<obj->Faces[n].v[0]->Calc.x) Mx=obj->Faces[n].v[0]->Calc.x;
		if (My<obj->Faces[n].v[0]->Calc.y) My=obj->Faces[n].v[0]->Calc.y;
		if (Mz<obj->Faces[n].v[0]->Calc.z) Mz=obj->Faces[n].v[0]->Calc.z;

		if (mx>obj->Faces[n].v[1]->Calc.x) mx=obj->Faces[n].v[1]->Calc.x;
		if (my>obj->Faces[n].v[1]->Calc.y) my=obj->Faces[n].v[1]->Calc.y;
		if (mz>obj->Faces[n].v[1]->Calc.z) mz=obj->Faces[n].v[1]->Calc.z;
		if (Mx<obj->Faces[n].v[1]->Calc.x) Mx=obj->Faces[n].v[1]->Calc.x;
		if (My<obj->Faces[n].v[1]->Calc.y) My=obj->Faces[n].v[1]->Calc.y;
		if (Mz<obj->Faces[n].v[1]->Calc.z) Mz=obj->Faces[n].v[1]->Calc.z;

		if (mx>obj->Faces[n].v[2]->Calc.x) mx=obj->Faces[n].v[2]->Calc.x;
		if (my>obj->Faces[n].v[2]->Calc.y) my=obj->Faces[n].v[2]->Calc.y;
		if (mz>obj->Faces[n].v[2]->Calc.z) mz=obj->Faces[n].v[2]->Calc.z;
		if (Mx<obj->Faces[n].v[2]->Calc.x) Mx=obj->Faces[n].v[2]->Calc.x;
		if (My<obj->Faces[n].v[2]->Calc.y) My=obj->Faces[n].v[2]->Calc.y;
		if (Mz<obj->Faces[n].v[2]->Calc.z) Mz=obj->Faces[n].v[2]->Calc.z;

		if (g.x-mx>Mx-g.x) obj->Faces[n].mp1.y=g.x-mx; else obj->Faces[n].mp1.y=Mx-g.x;
		if (g.y-my>My-g.y) obj->Faces[n].mp2.x=g.y-my; else obj->Faces[n].mp2.x=My-g.y;
		if (g.z-mz>Mz-g.z) obj->Faces[n].mp2.y=g.z-mz; else obj->Faces[n].mp2.y=Mz-g.z;
	}
    
    mini.Init(mx,my,mz);
    maxi.Init(Mx,My,Mz);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 void CObject3D::CalculateFaceCenter()
 void CObject3D::CalculateFaceCenterCalc()
 
 Usage:
 
 .g = center of face
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::CalculateFaceCenter()
{
    for (int n=0;n<nFaces;n++)
    {
        VECTORADDDIV3(Faces[n].g,Faces[n].v[0]->Stok,Faces[n].v[1]->Stok,Faces[n].v[2]->Stok);
    }
}

void CObject3D::CalculateFaceCenterCalc()
{
    for (int n=0;n<nFaces;n++)
    {
        VECTORADDDIV3(Faces[n].g,Faces[n].v[0]->Calc,Faces[n].v[1]->Calc,Faces[n].v[2]->Calc);
    }
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

  void CObject3D::CalculateSphereFaceOnCoef1()

	Usage:

	to call before settings the object as a envelop for physic collision
	CPhysicObject::IsAMesh() or CPhysicObject::IsAFullMesh()
	CPhysicObject::OBJECT=this;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::CalculateSphereFaceOnCoef1()
{
	int n,nn;
	int a,b;
	CVector g,u;
	float d,nrm;
	float mx,my,mz,Mx,My,Mz;
	int res;

	Calculate();

	for (n=0;n<nFaces;n++)
	{
		VECTORADDDIV3(g,Faces[n].v[0]->Stok,Faces[n].v[1]->Stok,Faces[n].v[2]->Stok);

		VECTORSUB(u,Faces[n].v[0]->Stok,g);
		d=VECTORNORM2(u);
		VECTORSUB(u,Faces[n].v[1]->Stok,g);
		nrm=VECTORNORM2(u);
		if (nrm>d) d=nrm;
		VECTORSUB(u,Faces[n].v[2]->Stok,g);
		nrm=VECTORNORM2(u);
		if (nrm>d) d=nrm;
		Faces[n].coef1=sqrtf(d);

		VECTORADDDIV3(g,Faces[n].v[0]->Calc,Faces[n].v[1]->Calc,Faces[n].v[2]->Calc);

		Faces[n].mp0.x=g.x;
		Faces[n].mp0.y=g.y;
		Faces[n].mp1.x=g.z;

		mx=g.x; my=g.y; mz=g.z;
		Mx=g.x; My=g.y; Mz=g.z;

		if (mx>Faces[n].v[0]->Calc.x) mx=Faces[n].v[0]->Calc.x;
		if (my>Faces[n].v[0]->Calc.y) my=Faces[n].v[0]->Calc.y;
		if (mz>Faces[n].v[0]->Calc.z) mz=Faces[n].v[0]->Calc.z;
		if (Mx<Faces[n].v[0]->Calc.x) Mx=Faces[n].v[0]->Calc.x;
		if (My<Faces[n].v[0]->Calc.y) My=Faces[n].v[0]->Calc.y;
		if (Mz<Faces[n].v[0]->Calc.z) Mz=Faces[n].v[0]->Calc.z;

		if (mx>Faces[n].v[1]->Calc.x) mx=Faces[n].v[1]->Calc.x;
		if (my>Faces[n].v[1]->Calc.y) my=Faces[n].v[1]->Calc.y;
		if (mz>Faces[n].v[1]->Calc.z) mz=Faces[n].v[1]->Calc.z;
		if (Mx<Faces[n].v[1]->Calc.x) Mx=Faces[n].v[1]->Calc.x;
		if (My<Faces[n].v[1]->Calc.y) My=Faces[n].v[1]->Calc.y;
		if (Mz<Faces[n].v[1]->Calc.z) Mz=Faces[n].v[1]->Calc.z;

		if (mx>Faces[n].v[2]->Calc.x) mx=Faces[n].v[2]->Calc.x;
		if (my>Faces[n].v[2]->Calc.y) my=Faces[n].v[2]->Calc.y;
		if (mz>Faces[n].v[2]->Calc.z) mz=Faces[n].v[2]->Calc.z;
		if (Mx<Faces[n].v[2]->Calc.x) Mx=Faces[n].v[2]->Calc.x;
		if (My<Faces[n].v[2]->Calc.y) My=Faces[n].v[2]->Calc.y;
		if (Mz<Faces[n].v[2]->Calc.z) Mz=Faces[n].v[2]->Calc.z;

		if (g.x-mx>Mx-g.x) Faces[n].mp1.y=g.x-mx; else Faces[n].mp1.y=Mx-g.x;
		if (g.y-my>My-g.y) Faces[n].mp2.x=g.y-my; else Faces[n].mp2.x=My-g.y;
		if (g.z-mz>Mz-g.z) Faces[n].mp2.y=g.z-mz; else Faces[n].mp2.y=Mz-g.z;
	}

	if (edges)
	{
		edges->Free();
		delete edges;
		edges=NULL;
	}

	if (edges==NULL)
	{
		edges=new EdgeListD;
		edges->Init(nFaces*3);
		int *corres=new int[nVertices];
		for (n=0;n<nVertices;n++)
		{
			corres[n]=n;
			for (nn=0;nn<n;nn++)
			{
				if (corres[nn]==nn)
				{
					VECTORSUB(u,Vertices[n].Stok,Vertices[nn].Stok);
					if (VECTORNORM2(u)<SMALLF) corres[n]=nn;
				}
			}
		}

		for (n=0;n<nFaces;n++)
		{
			a=Faces[n].v0;
			b=Faces[n].v1;
			edges->AddEdge3(corres[a],corres[b]);
			a=Faces[n].v1;
			b=Faces[n].v2;
			edges->AddEdge3(corres[a],corres[b]);
			a=Faces[n].v2;
			b=Faces[n].v0;
			edges->AddEdge3(corres[a],corres[b]);
		}

		for (nn=0;nn<nFaces;nn++)
		{
			res=0;
			a=corres[Faces[nn].v0];
			b=corres[Faces[nn].v1];
			for (n=0;n<edges->nList;n++) { if ((edges->List[n].a==a)&&(edges->List[n].b==b)) { res=1;break; } }
			Faces[nn].f01=res;
			res=0;
			a=corres[Faces[nn].v1];
			b=corres[Faces[nn].v2];
			for (n=0;n<edges->nList;n++) { if ((edges->List[n].a==a)&&(edges->List[n].b==b)) { res=1;break; } }
			Faces[nn].f12=res;
			res=0;
			a=corres[Faces[nn].v2];
			b=corres[Faces[nn].v0];
			for (n=0;n<edges->nList;n++) { if ((edges->List[n].a==a)&&(edges->List[n].b==b)) { res=1;break; } }
			Faces[nn].f20=res;
		}		

		delete [] corres;
	}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


	Usage:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

int search_value_in_tab(int tab[],int len,int value)
{
	int n;
	int res;

	res=-1;
	n=0;
	while ((n<len)&&(res==-1))
	{
		if (value==tab[n]) res=n;
		n++;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D* CObject3D::ReArr()
{
	CObject3D* obj=new CObject3D;
	int nts[128];
	int nnts=0;
	int n,s;
	int n1,n2;
	unsigned int tmp;
	unsigned int * array;
	unsigned int * corres;

	obj->Init(nVertices,nFaces);

	for (n=0;n<nFaces;n++)
	{
		s=search_value_in_tab(nts,nnts,Faces[n].nT);
		if (s<0)
		{
			nts[nnts]=Faces[n].nT;
			s=nnts;
			nnts++;
		}

		Vertices[Faces[n].v0].tag=s;
		Vertices[Faces[n].v1].tag=s;
		Vertices[Faces[n].v2].tag=s;
	}

	array=(unsigned int*) malloc(sizeof(unsigned int)*nVertices);
	corres=(unsigned int*) malloc(sizeof(unsigned int)*nVertices);

	for (n=0;n<nVertices;n++)
	{
		tmp=Vertices[n].tag;
		array[n]=n+(tmp<<16);
	}

	for (n1=0;n1<nVertices;n1++)
		for (n2=n1+1;n2<nVertices;n2++)
		{
			if (array[n2]<array[n1])
			{
				tmp=array[n1];
				array[n1]=array[n2];
				array[n2]=tmp;
			}
		}

	for (n=0;n<nVertices;n++) obj->Vertices[n]=Vertices[array[n]&0xFFFF];

	for (n=0;n<nVertices;n++) corres[array[n]&0xFFFF]=n;

	for (n=0;n<nFaces;n++)
	{
		obj->Faces[n]=Faces[n];
		obj->Faces[n].v0=corres[Faces[n].v0];
		obj->Faces[n].v1=corres[Faces[n].v1];
		obj->Faces[n].v2=corres[Faces[n].v2];
	}

	free(corres);
	free(array);

	obj->SetFaces();
	obj->CalculateNormals(-1);

	return obj;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

  void CObject3D::InverseOrientation()

	Usage:

	inverse face orientation

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::InverseOrientation()
{
	InverseOrientation(1);
}

void CObject3D::InverseOrientation(int tag)
{
	int n,k;

	for (n=0;n<nFaces;n++)
	{
		k=Faces[n].v1;
		Faces[n].v1=Faces[n].v2;
		Faces[n].v2=k;
	}

	SetFaces();

	if (tag)
	{
		for (n=0;n<nFaces;n++)
		{
			Faces[n].Norm.x=-Faces[n].Norm.x;
			Faces[n].Norm.y=-Faces[n].Norm.y;
			Faces[n].Norm.z=-Faces[n].Norm.z;
		}

		for (n=0;n<nVertices;n++)
		{
			Vertices[n].Norm.x=-Vertices[n].Norm.x;
			Vertices[n].Norm.y=-Vertices[n].Norm.y;
			Vertices[n].Norm.z=-Vertices[n].Norm.z;
		}
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

  CSpherePartition * CObject3D::CreateSpherePartition(int NB)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#ifdef _DEFINES_API_CODE_PHYSIC_
CSpherePartition * CObject3D::CreateSpherePartition(int NB)
{
	CSpherePartition *SP=new CSpherePartition;
	int res,n,n1,n2,n3;
	float r,lmax,dmin;
	CVector v,min,max,d;

	VECTORINIT(min,MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
	VECTORINIT(max,-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
	for (n=0;n<nVertices;n++)
	{
		v=Vertices[n].Stok;
		if (v.x<min.x) min.x=v.x; if (v.y<min.y) min.y=v.y; if (v.z<min.z) min.z=v.z;
		if (v.x>max.x) max.x=v.x; if (v.y>max.y) max.y=v.y; if (v.z>max.z) max.z=v.z;
	}

	d=max-min;

	lmax=d.x;
	if (d.y>lmax) lmax=d.y;
	if (d.z>lmax) lmax=d.z;

	dmin=lmax/NB;

	//r=(float) (2.0f*dmin/sqrtf(2.0f));


	SP->nbx=(int) (d.x / dmin);
	SP->nby=(int) (d.y / dmin);
	SP->nbz=(int) (d.z / dmin);

	if (SP->nbx<1) SP->nbx=1;
	if (SP->nby<1) SP->nby=1;
	if (SP->nbz<1) SP->nbz=1;

	if (SP->nbx>4) SP->nbx=4;
	if (SP->nby>4) SP->nby=4;
	if (SP->nbz>4) SP->nbz=4;

	int l1,l2,l3;
	l1=SP->nbx;
	l2=SP->nby;
	l3=SP->nbz;
	float _d=d.x;
	if (l1<l2) { l1=l2; _d=d.y; }
	if (l1<l3) { l1=l3; _d=d.z; }
	
	r=2.0f*_d/l1;


	for (n=0;n<64;n++) SP->balls[n].tag=0;

	for (n1=0;n1<SP->nbx;n1++)
		for (n2=0;n2<SP->nby;n2++)
			for (n3=0;n3<SP->nbz;n3++)
			{
				n=n1 + 4*( n2 + 4 * n3);
				SP->balls[n].x=min.x + d.x*(0.5f + (float) n1)/SP->nbx;
				SP->balls[n].y=min.y + d.y*(0.5f + (float) n2)/SP->nby;
				SP->balls[n].z=min.z + d.z*(0.5f + (float) n3)/SP->nbz;
				SP->balls[n].r=r;
				SP->balls[n].tag=1;
			}

	for (n=0;n<nVertices;n++)
	{
		v=Vertices[n].Stok;
		v=v-min;
		n1=(int) (v.x*(SP->nbx-1)/d.x);
		n2=(int) (v.y*(SP->nby-1)/d.y);
		n3=(int) (v.z*(SP->nbz-1)/d.z);

		if (n1<0) n1=0; if (n2<0) n2=0; if (n3<0) n3=0;
		if (n1>3) n1=3; if (n2>3) n2=3; if (n3>3) n3=3;

		Vertices[n].temp=n1 + 4*( n2 + 4 * n3);
	}

	for (n=0;n<nFaces;n++)
	{
		res=0;
		n1=Faces[n].v[0]->temp;
		n2=Faces[n].v[1]->temp;
		n3=Faces[n].v[2]->temp;
		if ((n1==n2)||(n1==n3)) res=0;
		if ((n2==n1)||(n2==n3)) res=1;
		if ((n3==n1)||(n3==n2)) res=2;
		Faces[n].var=(unsigned char)Faces[n].v[res]->temp;
	}

	return SP;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::TagCoplanarFacesMC(int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;
	int mf0,mf1,mf2;
	CMatrix M,MM;
	float s1,s2,s3;
	int pnt;
	CVector u;
	CVector2 m,Dm;

	if (Faces[nf].tag==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm)

		if ((ss>1.0f-SMALLF)||(VECTORNORM(Faces[nf].Norm)<0.1f))
		{
			Faces[nf].tag=TAG;
			f0=Face_01(nf);
			f1=Face_12(nf);
			f2=Face_20(nf);

			if ((f0!=-1)||(f1!=-1)||(f2!=-1))
			{
				M.Id();
				M.a[0][0]=Faces[nf].v[0]->Stok.x;
				M.a[0][1]=Faces[nf].v[0]->Stok.y;
				M.a[0][2]=Faces[nf].v[0]->Stok.z;
				M.a[1][0]=Faces[nf].v[1]->Stok.x;
				M.a[1][1]=Faces[nf].v[1]->Stok.y;
				M.a[1][2]=Faces[nf].v[1]->Stok.z;
				M.a[2][0]=Faces[nf].v[2]->Stok.x;
				M.a[2][1]=Faces[nf].v[2]->Stok.y;
				M.a[2][2]=Faces[nf].v[2]->Stok.z;
				MM.Transpose(M);
				M.Inverse3x3(MM);

				if (f0!=-1)
				{
					mf0=Face_01(f0);
					mf1=Face_12(f0);
					mf2=Face_20(f0);
					if (mf0==nf) pnt=Faces[f0].v2;
					if (mf1==nf) pnt=Faces[f0].v0;
					if (mf2==nf) pnt=Faces[f0].v1;
					u=Vertices[pnt].Stok*M;
					s1=u.x;
					s2=u.y;
					s3=u.z;
					m=(Faces[nf].v[0]->Map*s1 + Faces[nf].v[1]->Map*s2 + Faces[nf].v[2]->Map*s3)/(s1+s2+s3);
					Dm=m - Vertices[pnt].Map;
					if (Dm.Norme()<SMALLF3) TagCoplanarFacesMC(f0,ref,TAG);
				}

				if (f1!=-1)
				{
					mf0=Face_01(f1);
					mf1=Face_12(f1);
					mf2=Face_20(f1);
					if (mf0==nf) pnt=Faces[f1].v2;
					if (mf1==nf) pnt=Faces[f1].v0;
					if (mf2==nf) pnt=Faces[f1].v1;
					u=Vertices[pnt].Stok*M;
					s1=u.x;
					s2=u.y;
					s3=u.z;
					m=(Faces[nf].v[0]->Map*s1 + Faces[nf].v[1]->Map*s2 + Faces[nf].v[2]->Map*s3)/(s1+s2+s3);
					Dm=m - Vertices[pnt].Map;
					if (Dm.Norme()<SMALLF3) TagCoplanarFacesMC(f1,ref,TAG);
				}

				if (f2!=-1)
				{
					mf0=Face_01(f2);
					mf1=Face_12(f2);
					mf2=Face_20(f2);
					if (mf0==nf) pnt=Faces[f2].v2;
					if (mf1==nf) pnt=Faces[f2].v0;
					if (mf2==nf) pnt=Faces[f2].v1;
					u=Vertices[pnt].Stok*M;
					s1=u.x;
					s2=u.y;
					s3=u.z;
					m=(Faces[nf].v[0]->Map*s1 + Faces[nf].v[1]->Map*s2 + Faces[nf].v[2]->Map*s3)/(s1+s2+s3);
					Dm=m - Vertices[pnt].Map;
					if (Dm.Norme()<SMALLF3) TagCoplanarFacesMC(f2,ref,TAG);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CObject3D::ReMeshByMapping()
{
	int tag=1;
	int n,nn;
	int count=0;
	int countf=0;
	bool res=false;
	CObject3D *obj;

	for (n=0;n<nFaces;n++) Faces[n].tag=0;

	while (!res)
	{
		n=0;
		nn=-1;
		while ((n<nFaces)&&(nn==-1))
		{
			if (Faces[n].tag==0) nn=n;
			else n++;
		}

		if (nn==-1) res=true;
		else
		{
			TagCoplanarFacesMC(nn,nn,tag);
			Faces[nn].tag=tag;
			tag++;
		}
	}

	for (n=1;n<tag;n++)
	{
		for (nn=0;nn<nVertices;nn++) Vertices[nn].tag=0;

		for (nn=0;nn<nFaces;nn++)
		{
			if (Faces[nn].tag==n)
			{
				Faces[nn].v[0]->tag=1;
				Faces[nn].v[1]->tag=1;
				Faces[nn].v[2]->tag=1;
			}
		}

		for (nn=0;nn<nVertices;nn++) if (Vertices[nn].tag) count++;
	}

	obj=new CObject3D;
	obj->Init(count,nFaces);

	count=0;

	for (n=1;n<tag;n++)
	{
		for (nn=0;nn<nVertices;nn++) Vertices[nn].tag=0;

		for (nn=0;nn<nFaces;nn++)
		{
			if (Faces[nn].tag==n)
			{
				Faces[nn].v[0]->tag=1;
				Faces[nn].v[1]->tag=1;
				Faces[nn].v[2]->tag=1;
			}
		}

		for (nn=0;nn<nVertices;nn++)
		{
			if (Vertices[nn].tag)
			{
				obj->Vertices[count]=Vertices[nn];
				Vertices[nn].temp=count;
				count++;
			}
		}

		for (nn=0;nn<nFaces;nn++)
		{
			if (Faces[nn].tag==n)
			{
				obj->Faces[countf]=Faces[nn];
				obj->Faces[countf].v0=Vertices[Faces[nn].v0].temp;
				obj->Faces[countf].v1=Vertices[Faces[nn].v1].temp;
				obj->Faces[countf].v2=Vertices[Faces[nn].v2].temp;
				countf++;
			}
		}
	}

	obj->SetFaces();
	obj->CalculateNormals(-1);

	obj->Rot=Rot;
	obj->Coo=Coo;
	for (n=0;n<6;n++) obj->P[n]=P[n];
	obj->nP=nP;
	for (n=0;n<16;n++) obj->Tab[n]=Tab[n];
	obj->Status=Status;
	obj->Attribut=Attribut;
	obj->Tag=Tag;
	obj->nurbs=nurbs;
	obj->group=group;
	obj->env_mapping=env_mapping;

	return obj;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::Extend()
 
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::Extend()
{
    int n;
    CVertex * vtx=new CVertex[nFaces*3];
    
    for (n=0;n<nFaces;n++)
    {
        vtx[3*n+0]=*Faces[n].v[0];
        vtx[3*n+1]=*Faces[n].v[1];
        vtx[3*n+2]=*Faces[n].v[2];
        
        Faces[n].v0=3*n+0;
        Faces[n].v1=3*n+1;
        Faces[n].v2=3*n+2;
    }
    
    delete [] Vertices;
    Vertices=vtx;
    nVertices=nFaces*3;
    
    SetFaces();
    CalculateNormals(-1);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::SetMappingFacesFromVertices()
 
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetMappingFacesFromVertices()
{
    int n;
    
    for (n=0;n<nFaces;n++)
    {
        Faces[n].mp0=Faces[n].v[0]->Map;
        Faces[n].mp1=Faces[n].v[1]->Map;
        Faces[n].mp2=Faces[n].v[2]->Map;
    }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 functions:
 
 void CObject3D::SetMappingVerticesFromFaces()
 
 
 Usage:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CObject3D::SetMappingVerticesFromFaces()
{
    int n;
    int v0,v1,v2;
    float SMALLFZ=SMALLF*SMALLF;
    CObject3D *result=new CObject3D;
    
    result->Init(nVertices*16,nFaces);
    
    for (n=0;n<nVertices;n++)
    {
        result->Vertices[n].Stok=Vertices[n].Stok;
        for (int k=0;k<4;k++)
        {
            result->Vertices[n].Index[k]=Vertices[n].Index[k];
            result->Vertices[n].Weight[k]=Vertices[n].Weight[k];
        }
        
        result->Vertices[n].tag=0;
        result->Vertices[n].tag2=Vertices[n].tag2;
    }
    
    result->nVertices=nVertices;
    
    for (n=0;n<nFaces;n++)
    {
        v0=Faces[n].v0;
        v1=Faces[n].v1;
        v2=Faces[n].v2;
        
        if (result->Vertices[v0].tag>0)
        {
            CVector2 m=Faces[n].mp0;
            CVector2 u=result->Vertices[v0].Map-m;
            if (u.x*u.x+u.y*u.y>SMALLFZ)
            {
                result->Vertices[result->nVertices].Stok=result->Vertices[v0].Stok;
                for (int k=0;k<4;k++)
                {
                    result->Vertices[result->nVertices].Index[k]=result->Vertices[v0].Index[k];
                    result->Vertices[result->nVertices].Weight[k]=result->Vertices[v0].Weight[k];
                }
                result->Vertices[result->nVertices].tag=0;
                result->Vertices[result->nVertices].tag2=result->Vertices[v0].tag2;
                
                v0=result->nVertices;
                result->nVertices++;
            }
        }
        result->Vertices[v0].Map=Faces[n].mp0;
        result->Vertices[v0].tag++;
        
        if (result->Vertices[v1].tag>0)
        {
            CVector2 m=Faces[n].mp1;
            CVector2 u=result->Vertices[v1].Map-m;
            if (u.x*u.x+u.y*u.y>SMALLFZ)
            {
                result->Vertices[result->nVertices].Stok=result->Vertices[v1].Stok;
                for (int k=0;k<4;k++)
                {
                    result->Vertices[result->nVertices].Index[k]=result->Vertices[v1].Index[k];
                    result->Vertices[result->nVertices].Weight[k]=result->Vertices[v1].Weight[k];
                }
                result->Vertices[result->nVertices].tag=0;
                result->Vertices[result->nVertices].tag2=result->Vertices[v1].tag2;
                
                v1=result->nVertices;
                result->nVertices++;
            }
        }
        result->Vertices[v1].Map=Faces[n].mp1;
        result->Vertices[v1].tag++;
        
        if (result->Vertices[v2].tag>0)
        {
            CVector2 m=Faces[n].mp2;
            CVector2 u=result->Vertices[v2].Map-m;
            if (u.x*u.x+u.y*u.y>SMALLFZ)
            {
                result->Vertices[result->nVertices].Stok=result->Vertices[v2].Stok;
                for (int k=0;k<4;k++)
                {
                    result->Vertices[result->nVertices].Index[k]=result->Vertices[v2].Index[k];
                    result->Vertices[result->nVertices].Weight[k]=result->Vertices[v2].Weight[k];
                }
                result->Vertices[result->nVertices].tag=0;
                result->Vertices[result->nVertices].tag2=result->Vertices[v2].tag2;
                
                v2=result->nVertices;
                result->nVertices++;
            }
        }
        result->Vertices[v2].Map=Faces[n].mp2;
        result->Vertices[v2].tag++;
        
        result->Faces[n].v0=v0;
        result->Faces[n].v1=v1;
        result->Faces[n].v2=v2;

		result->Faces[n].mp0=Faces[n].mp0;
		result->Faces[n].mp1=Faces[n].mp1;
		result->Faces[n].mp2=Faces[n].mp2;
        
        result->Faces[n].f01=Faces[n].f01;
        result->Faces[n].f12=Faces[n].f12;
        result->Faces[n].f20=Faces[n].f20;
        
        result->Faces[n].ref=Faces[n].ref;
        result->Faces[n].nT=Faces[n].nT;
        result->Faces[n].nT2=Faces[n].nT2;
		result->Faces[n].nL=Faces[n].nL;
        
        result->Faces[n].tag=Faces[n].tag;
        result->Faces[n].tag2=Faces[n].tag2;
        result->Faces[n].tag3=Faces[n].tag3;
        result->Faces[n].tag4=Faces[n].tag4;
        
        result->Faces[n].coef1=Faces[n].coef1;
    }
    
    result->SetFaces();
    
    CObject3D * tmp=result->Duplicate2();
    
    result->Free();
    delete result;
    
    delete [] Vertices;
    delete [] Faces;
    
    nVertices=tmp->nVertices;
    nFaces=tmp->nFaces;

    Vertices=tmp->Vertices;
    Faces=tmp->Faces;

    tmp->Vertices=NULL;
    tmp->Faces=NULL;

    tmp->Free();
    delete tmp;
    
    SetFaces();
    CalculateNormals(-1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::Scaling(float sc)
{
    for (int n=0;n<nVertices;n++) Vertices[n].Stok*=sc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::Scaling(float sx,float sy,float sz)
{
    for (int n=0;n<nVertices;n++)
    {
        Vertices[n].Stok.x*=sx;
        Vertices[n].Stok.y*=sy;
        Vertices[n].Stok.z*=sz;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObject3D::Rotation(float rx,float ry,float rz)
{
    CMatrix M;
    M.RotationDegre(rx,ry,rz);
    for (int n=0;n<nVertices;n++) Vertices[n].Stok=Vertices[n].Stok*M;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///     Misc
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IntersectABCD(CVector &P,CVector &Q,CVector &A,CVector &B,CVector &C,CVector &D,CVector &I,CVector &N)
{
    float t;
    float a,b,c,d;
    float s1,s2;
    CVector a1,a2,a3,a4,b1,b2,b3,b4,c1,c2,c3,c4;

    VECTORSUB(a1,B,A);
    VECTORSUB(a2,C,A);

    CROSSPRODUCT(N,a2,a1);
    VECTORNORMALISE(N);

    a=N.x;
    b=N.y;
    c=N.z;
    d=-DOT(A,N);

    s1=a*P.x+b*P.y+c*P.z+d;
    s2=a*Q.x+b*Q.y+c*Q.z+d;

    if (s1*s2<0)
    {
        t=-s1/(s2-s1);
        I=P+t*(Q-P);

        VECTORSUB(a1,B,A);
        VECTORSUB(b1,I,A);
        CROSSPRODUCT(c1,b1,a1);
        if (DOT(N,c1)>0)
        {
            VECTORSUB(a2,C,B);
            VECTORSUB(b2,I,B);
            CROSSPRODUCT(c2,b2,a2);
            if (DOT(N,c2)>0)
            {
                VECTORSUB(a3,D,C);
                VECTORSUB(b3,I,C);
                CROSSPRODUCT(c3,b3,a3);
                if (DOT(N,c3)>0)
                {
                    VECTORSUB(a4,A,D);
                    VECTORSUB(b4,I,D);
                    CROSSPRODUCT(c4,b4,a4);
                    if (DOT(N,c4)>0) return true;
                }
            }
        }
    }

    return false;
}

float IntersectionABCD(CVector &A,CVector &B,CVector &AA,CVector &BB,CVector &CC,CVector &DD,float &xm,float &ym)
{
    CVector u,u1,u2;
    CVector N,I;

    if (IntersectABCD(A,B,AA,BB,CC,DD,I,N))
    {
        VECTORSUB(u,I,A);
        float f=VECTORNORM(u);

        VECTORSUB(u1,BB,AA);
        VECTORSUB(u2,DD,AA);

        VECTORSUB(u,I,AA);

        float l1=u1.Normalise2();
        float l2=u2.Normalise2();

        float s1=DOT(u,u1);
        float s2=DOT(u,u2);

        xm=s1/l1;
        ym=s2/l2;

        if ((xm>=0)&&(xm<1)&&(ym>0)&&(ym<1)) return f;
    }
    return 100000.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
