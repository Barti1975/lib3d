
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

#ifdef _DEFINES_API_CODE_METABALLS_

#ifndef _METABALLS_H_
#define _METABALLS_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../base/maths.h"
#include "../base/objects3d.h"
#include "../base/3d_api_base.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Ball
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMetaball
{
public:
	CVector v;
	float p;
	int tag;

	CMetaball() { tag=0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Tube
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMetaballTube
{
public:
	CVector a,b;
	float pa,pb;
	int tag;

	CMetaballTube() { tag=0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Very short face
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CShortF
{
public:
	int n0,n1,n2;

	CShortF() {}

	inline void Init(int i1,int i2,int i3);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Metaballs mesh generator
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMetaballs
{
public:
	float small_float;
	float equi;
	CMetaball Metaballs[4096];
	int nMetaballs;
	CMetaballTube MetaballsTube[4096];
	int nMetaballsTube;
    int METABALLS_MAXDATA;
	bool gravitycenter;
	bool vertexbuffer_update;
	float * potentiel_xyz;

	bool ENVMAPPING;
	CObject3D OBJ;
	CObject3D TMP;
	CVertexBuffer VB;

	CVector Pos;
	int K_ITERATIONS;
	int posVertices;
	int minx,miny,minz,maxx,maxy,maxz;
	float minx0,miny0,minz0,maxx0,maxy0,maxz0;  

	int Segments[8][8];

	int impair;
	int Orientation;

	int pnca0[8];
	int pnca1[8];
	int pnca2[8];

	bool wait;

	int tag[8];

	CVector pts[8];
	float e[8];

	CVector *Vertices;
	CVector *Norms;
	int nVertices;

	CShortF *Faces;
	int nFaces;

	float SCALING;
	float DIVISION;

	int segm0[12];
	int segm1[12];
	int pnc0[8];
	int pnc1[8];
	int pnc2[8];

	int status;

	short int * opt;
	short int actual;

    float LIMITEMETA;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// constructor

	CMetaballs() 
	{
        LIMITEMETA=100000.0f;
		vertexbuffer_update=true;
		gravitycenter=true;
		nMetaballs=0;
		nMetaballsTube=0;
		nFaces=0;nVertices=0;Orientation=1;
		status=0;
		ENVMAPPING=true;
		wait=true;
        potentiel_xyz=NULL;
        Vertices=NULL;
        opt=NULL;
        Faces=NULL;
        METABALLS_MAXDATA=-1;
        K_ITERATIONS=4;
	}

	~CMetaballs()
	{
        if (potentiel_xyz) free(potentiel_xyz);
        if (Vertices) delete [] Vertices;
        if (opt) delete [] opt;
        if (Faces) delete [] Faces;
        potentiel_xyz=NULL;
        Vertices=NULL;
        opt=NULL;
        Faces=NULL;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// interaction with procédures

		void Init();
		void Free();
		void AddMetaball(CVector pos,float pot);
		void SubMetaball(CVector pos,float pot);
		void AddMetaballTube(CVector a,CVector b,float pota,float potb);
		void SubMetaballTube(CVector a,CVector b,float pota,float potb);
		void SetEquipotentielle(float e,float precision);
		int Calculate();
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals
	
		int UploadVertices(CVector &A);
		CVector Gradiant(CVector &A);
		float Potentiel(CVector &u);
		int EquiPotentielle(CVector &A,CVector &B,float pA,float pB);
		void UploadQuad(int ref,int A,int B,int C,int D);
		void CalcTetrahedron(int n0,int n1,int n2,int n3);
		void CalcTetrahedronTri(int n0,int n1,int n2,int n3);
		int CalculateIntersectionSegment(int n0,int n1);
		void UploadTriangle(int ref,int A,int B,int C);
		inline void chg(int n,int m);	
};

typedef CMetaballs CLASS_NAME_METABALLS;

#endif
#endif
