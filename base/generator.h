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


#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#ifdef _DEFINES_API_CODE_GENERATOR_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(GLES)||defined(GLES20)||defined(IOS)
#define GENERATOR_MAX					20000
#define GENERATOR_VERTICES_MAX			(GENERATOR_MAX*3)
#else
#define GENERATOR_MAX					50000
#define GENERATOR_VERTICES_MAX			(GENERATOR_MAX*3)
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CGenerator
{
public:
	int nVertices;
	int nFaces;
	CVertex *Vertices;
	CShortVertex *VerticesKeys[4096];
	CFace *Faces;
    int maxnv,maxnf;
	int nkeys;
    int level;

    CGenerator()
	{
		level=-1; Vertices=new CVertex[GENERATOR_VERTICES_MAX]; Faces=new CFace[GENERATOR_MAX]; nVertices=0; nFaces=0; maxnv=GENERATOR_VERTICES_MAX; maxnf=GENERATOR_MAX;
		for (int n=0;n<4096;n++) VerticesKeys[n]=NULL;
		nkeys=0;
	}
    CGenerator(int nv,int nf)
	{
		level=-1; Vertices=new CVertex[nv]; Faces=new CFace[nf]; nVertices=0; nFaces=0; maxnv=nv; maxnf=nf;
		for (int n=0;n<4096;n++) VerticesKeys[n]=NULL;
		nkeys=0;
	}
	~CGenerator()
	{
		delete [] Vertices;
		delete [] Faces;
		for (int n=0;n<4096;n++)
		{
			if (VerticesKeys[n])
			{
				delete [] VerticesKeys[n];
				VerticesKeys[n]=NULL;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// usefull functions

		int Faces_01(int nf,int * faces_array,int *nb);
		int Faces_12(int nf,int * faces_array,int *nb);
		int Faces_20(int nf,int * faces_array,int *nb);
		bool EdgeConfondue(int na1,int nb1,int na2,int nb2);
		void ReadScript(char * script);
		void ReadScript(char * script,CObject3D *base1);
		void ReadScript(char * script,CObject3D *base1,CObject3D *base2);
		void ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3);
		void ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3,CObject3D *base4);
		void SetFaces();
		void Tric(CVector L);
		bool FacePresent(int v0,int v1,int v2);
		void Add(CObject3D * obj);
		void AddCalc(CObject3D * obj);
		void AddCalcTess(CObject3D * objbase);
		void AddMatrix(CObject3D * obj,CMatrix M);
		void AddPlanar(CObject3D * obj);
		void AddPlanarMod(CObject3D * obj);
		void AddInverse(CObject3D * obj);
		void AddSelected(CObject3D * obj,int tag);
		void AddSelectedCalc(CObject3D * obj,int tag);
		CObject3D ResObjet();
		void AddConvertingFaceMappingToPerVertexMapping(CObject3D * obj);
		void AddMinimizedVerticesIndexing(CObject3D * obj,float scale);
		void AddMinimized(CObject3D * obj);
		CObject3D * pResObjet();
        CObject3D * pResObjetMap2();
		void TagCoplanarFaces(int nf,int ref,int TAG);
		void TagAllCoplanarFaces(int ref,int TAG,int limit);
		void TagObjetCoplanarFaces(CObject3D * obj,int nf,int ref,int TAG);
		int Face_01(int nf);
		int Face_12(int nf);
		int Face_20(int nf);
		int iFace_01(int nf);
		int iFace_12(int nf);
		int iFace_20(int nf);
		void TagALLRecCoplanarFaces(int nf,int ref,int TAG,int limit);
		void Tri(CVector L);
		void TagRecursNearToNear(int nf,int ref,int TAG);
		void AddWithOptimization(CObject3D * obj);
        void Reset() { nVertices=nFaces=0; }

		void SetTopo();
		void TagCoplanarFacesTopo(int nf,int ref,int TAG);
    
        int FaceWithEdges(int a,int b);
        int FaceWithEdges(int a,int b,int nf);
        int nbFaceWithEdges(int a,int b);
    
        void AddTri(CVector &A,CVector &B,CVector &C);
        void AddTriNorm(CVector &A,CVector &NA,CVector &B,CVector &NB,CVector &C,CVector &NC);
};

#endif
#endif
