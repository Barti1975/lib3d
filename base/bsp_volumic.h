
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

#ifndef _VOLUMIC_BSP_
#define _VOLUMIC_BSP_

#include "params.h"


#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Volumic BSP tree
//	used in:
//	 object boolean operations
//	 shadow cutting
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


enum
{
	BSP_NODE	= 1,
	BSP_OUT		= 2,
	BSP_IN		= 3
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// operations booléennes sur les objets 3D

#define BOOLEAN_IN			1
#define BOOLEAN_OUT			2


#define INTERIEUR BOOLEAN_IN
#define EXTERIEUR BOOLEAN_OUT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTriangle2
{
public:
	CVector A,B,C;
	CVector Norm;
	int tag;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTriangle
{
public:
	CVector A,B,C;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTriangles
{
public:
	CList <CTriangle> List;
	CVector Norm;
	CVector Base;
	int tag;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CBSPNode
{
public:
	CBSPNode *plus,*moins;
	float a,b,c,d;
	int type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! BSP volumic processing unit
//! see include definitions
class API3D_STUB CBSPVolumic
{
public:
	CBSPNode *root;
	CList <CBSPNode> roots;
	float VALUEBUILD;
	CList <CShortVertexMap> Vertices;
	CList <CShortFaceMap> interior_faces,exterior_faces;
	CList <CShortFaceMap> cutted_faces;
	
	int nfaces;
	int nVertices;
	
	CList<CShortVertexMap> VerticesS;
	CList<CShortFaceMap> faces;

    int * VerticesSIndex;
	CList <CPlane> Plans;
	bool full;
	bool grouped;
	int _max_nb_faces;
	int _max_nb_vertices;

	float _x,_y,_z;

	CBSPVolumic() { _max_nb_vertices=_max_nb_faces=0; root=NULL; VALUEBUILD=1.0f; full=false; grouped=false;}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// params

		void SetGrouped() { grouped=true; }
		void SetUnGrouped() { grouped=false; }

		void SetFull() { full=true; }
		void SetFast() { full=false; }

		void SetValueBuild(float v) { VALUEBUILD=v; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// build

		//! build a volumic bsp tree
		//! @param obj : object in use for volum calculation
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		void Build(CObject3D *obj,float mul,float mul2);

		//! build a volumic bsp tree
		//! @param obj : object in use for volum calculation
		//! @param vg : facegroup
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : reference vector
		void BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L);

		//! build a volumic bsp tree from an object simple,fast,not complete
		//! @param obj : object in use for volum calculation
		//! @param nf : face in object
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : diffuse point
		void BuildVolume(CObject3D *obj,int nf,float mul,float mul2,CVector L,int tag);
		//! build a volumic bsp tree from a face, usefull to clip projected objects
		//! @param obj : object in use for volum calculation
		//! @param nf : face in object
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : diffuse point
		void BuildVolumeFace(CObject3D *obj,int nf,float mul,float mul2,CVector L);
    
        //! build a volumic bsp tree from an object simple,fast,not complete
        //! @param obj : object in use for volum calculation
        //! @param nf : face in object
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sign
        void BuildVolumeFaceNormal(CObject3D *obj,int nf,float mul,float mul2);
        
        //! build a volumic bsp tree from an object simple,fast,not complete
        //! @param obj : object in use for volum calculation
        //! @param N : normal of the face
        //! @param A,B,C,D : quad
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sign
        void BuildVolumeQuadNormal(CVector &N,CVector &A,CVector &B,CVector &C,CVector &D,float mul,float mul2);

		//! build a volumic bsp tree from a face, usefull to clip projected objects
		//! @param obj : object in use for volum calculation
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		void BuildObjectVolume(CObject3D *obj,float mul,float mul2);

		//! free (internal)
		void FreeNode(CBSPNode * node);

		//! free ressource
		void Free();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

        int NumberOfPlanes(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2);
		
		void ListePlans(CBSPNode * node);

		//! build the bsp from a list of faces reordered in CTriangles from coplanarity
		//! internal
		CBSPNode * BuildNode(CList <CTriangles> *Triangles,float mul);

		//! build the bsp from a list of faces reduced to plane information
		//! internal
		CBSPNode * BuildNode2(CTriangle2 *Triangles,int nt,float mul,int N,int mask);
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// application
	
		//! test if inside
		//! @param p : point
		bool Inside(CVector &p);
		//! test if outside
		//! @param p : point
		bool Outside(CVector &p);
		
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

		//! test if inside
		bool FaceInside(CBSPNode *node,CFace *f,float mul);
		//! test if inside
		bool ShortFaceInside(CBSPNode *node,CShortFaceMap *f);
		
		//! Cut and Sort from BSP
		void InOutNoMap(int mask,int N,CBSPNode * node,float mul);
		void InOut(int mask,int N,CBSPNode * node,float mul);

		// Cut Faces following merged bsp planes
		int CutFaces(int nP,CList<CShortFaceMap> &faces,int NF,float mul);
		int CutFacesMap(int nP,CList<CShortFaceMap> &faces,int NF,float mul);
			
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// fast boolean

		//! boolean operation
		//! @param obj,ref: objects
		//!	@param tag: value of BOOLEAN_IN,BOOLEAN_OUT
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sig		
		CObject3D * Boolean(CObject3D * obj,CObject3D * ref,int tag,float mul,float mul2,bool TAG);
    
        //! boolean operation
        //! @param obj,ref: objects
        //!    @param tag: value of BOOLEAN_IN,BOOLEAN_OUT
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sig
        bool tagBoolean(CObject3D * obj,CObject3D * ref,float mul);
    
		//! boolean operation between 2 objects (full,object is optimized)
		//! @param obj,ref: objects
		//!	@param tag: value of BOOLEAN_IN,BOOLEAN_OUT
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param pobj1
		//! @param pobj2
		void Boolean2Parts(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,CObject3D **pobj1,CObject3D **pobj2);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! BSP volumic processing unit
//! see include definitions
class API3D_STUB CBSPVolumicOld
{
public:
	CBSPNode *root;
	CList <CBSPNode> roots;
	float VALUEBUILD;
	CList <CShortVertexMap> Vertices;
	CList <CShortFaceMap> interior_faces,exterior_faces;
	CList <CShortFaceMap> cutted_faces;
	CShortFaceMap *faces;
	int nfaces;
	int nVertices;
	CShortVertexMap * VerticesS;
    int * VerticesSIndex;
	CList <CPlane> Plans;
	bool full;
	bool grouped;
	int _max_nb_faces;
	int _max_nb_vertices;

	float _x,_y,_z;

	CBSPVolumicOld() { _max_nb_vertices=_max_nb_faces=0; root=NULL; VALUEBUILD=1.0f; full=false; grouped=false;}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// params

		void SetGrouped() { grouped=true; }
		void SetUnGrouped() { grouped=false; }

		void SetFull() { full=true; }
		void SetFast() { full=false; }

		void SetValueBuild(float v) { VALUEBUILD=v; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// build

		//! build a volumic bsp tree
		//! @param obj : object in use for volum calculation
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		void Build(CObject3D *obj,float mul,float mul2);

		//! build a volumic bsp tree
		//! @param obj : object in use for volum calculation
		//! @param vg : facegroup
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : reference vector
		void BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L);

		//! build a volumic bsp tree from an object simple,fast,not complete
		//! @param obj : object in use for volum calculation
		//! @param nf : face in object
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : diffuse point
		void BuildVolume(CObject3D *obj,int nf,float mul,float mul2,CVector L,int tag);
		//! build a volumic bsp tree from a face, usefull to clip projected objects
		//! @param obj : object in use for volum calculation
		//! @param nf : face in object
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param L : diffuse point
		void BuildVolumeFace(CObject3D *obj,int nf,float mul,float mul2,CVector L);
    
        //! build a volumic bsp tree from an object simple,fast,not complete
        //! @param obj : object in use for volum calculation
        //! @param nf : face in object
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sign
        void BuildVolumeFaceNormal(CObject3D *obj,int nf,float mul,float mul2);
        
        //! build a volumic bsp tree from an object simple,fast,not complete
        //! @param obj : object in use for volum calculation
        //! @param N : normal of the face
        //! @param A,B,C,D : quad
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sign
        void BuildVolumeQuadNormal(CVector &N,CVector &A,CVector &B,CVector &C,CVector &D,float mul,float mul2);

		//! build a volumic bsp tree from a face, usefull to clip projected objects
		//! @param obj : object in use for volum calculation
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		void BuildObjectVolume(CObject3D *obj,float mul,float mul2);

		//! free (internal)
		void FreeNode(CBSPNode * node);

		//! free ressource
		void Free();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

        int NumberOfPlanes(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2);
		
		void ListePlans(CBSPNode * node);

		//! build the bsp from a list of faces reordered in CTriangles from coplanarity
		//! internal
		CBSPNode * BuildNode(CList <CTriangles> *Triangles,float mul);

		//! build the bsp from a list of faces reduced to plane information
		//! internal
		CBSPNode * BuildNode2(CTriangle2 *Triangles,int nt,float mul,int N,int mask);
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// application
	
		//! test if inside
		//! @param p : point
		bool Inside(CVector &p);
		//! test if outside
		//! @param p : point
		bool Outside(CVector &p);
		
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

		//! test if inside
		bool FaceInside(CBSPNode *node,CFace *f,float mul);
		//! test if inside
		bool ShortFaceInside(CBSPNode *node,CShortFaceMap *f);
		
		//! Cut and Sort from BSP
		void InOutNoMap(int mask,int N,CBSPNode * node,float mul);
		void InOut(int mask,int N,CBSPNode * node,float mul);

		// Cut Faces following merged bsp planes
		int CutFaces(int nP,CShortFaceMap *faces,int NF,float mul);
		int CutFacesMap(int nP,CShortFaceMap *faces,int NF,float mul);
			
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// fast boolean

		//! boolean operation
		//! @param obj,ref: objects
		//!	@param tag: value of BOOLEAN_IN,BOOLEAN_OUT
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sig		
		CObject3D * Boolean(CObject3D * obj,CObject3D * ref,int tag,float mul,float mul2,bool TAG);
    
        //! boolean operation
        //! @param obj,ref: objects
        //!    @param tag: value of BOOLEAN_IN,BOOLEAN_OUT
        //! @param mul : orientation sign
        //! @param mul2 : bsp comparison sig
        bool tagBoolean(CObject3D * obj,CObject3D * ref,float mul);
    
		//! boolean operation between 2 objects (full,object is optimized)
		//! @param obj,ref: objects
		//!	@param tag: value of BOOLEAN_IN,BOOLEAN_OUT
		//! @param mul : orientation sign
		//! @param mul2 : bsp comparison sign
		//! @param pobj1
		//! @param pobj2
		void Boolean2Parts(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,CObject3D **pobj1,CObject3D **pobj2);

};


#endif
#endif
