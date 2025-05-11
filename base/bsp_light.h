
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


#ifndef _BSP_LIGHT_H_
#define _BSP_LIGHT_H_

#include "params.h"

#ifdef _DEFINES_API_CODE_BSP_LIGHTS_

#include "maths.h"
#include "objects3d.h"
#include "bsp_volumic.h"
#include "edges.h"

#define NODE -1
#define SHADOW 0
#define LIGHT 1


#define SHADOWFACE 1
#define NORMALFACE 0

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <memory.h>

#include "../data/datacontainer.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CBSPLightNode
{
public:
	signed char type;
	unsigned char tag;
	short int index;
	CBSPLightNode *plus, *moins;
	float a,b,c,d;
	CVector p;
	
	CBSPLightNode() {plus=moins=NULL;}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Lighting and shadowing processing unit
class API3D_STUB CBSPLight
{
public:
	int compteur;
	CBSPLightNode* root;
	int nb;
	CList <CBSPLightNode*> roots;
	int nobj;
	int nobj_base;
	CList <CVertex> Vertices;
	CList <CShortFace> faces_inlight,faces_inshadow,cutted_faces,facing;
	int nVertices;
	CList <CPlane> Plans;
	float tess_value;
	int faces_in_shadows_tag;
	bool dup;

	CBSPLight() { root=NULL; tess_value=1.0f; faces_in_shadows_tag=0; dup=false; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// application
	
	//! parameter
	void SetTesselateValue(float v);
	//! test if a point is lighted from a node
	bool RootLight(CBSPLightNode * node,CVector &v);
	//! test if a point is in shadow from a node
	bool RootShadow(CBSPLightNode * node,CVector &v);
	//! test if a point is lighted
	bool Lighted(CVector &v);

	//! basic duplicate
	void Duplicate(CBSPLight * bsp);

	//! test if a object is lighted/shadowed
	bool TestIfShadows(CObject3D * obj,CBSPLightNode *r,CVector &L);
	bool TestIfShadowsExt(CObject3D * obj,CBSPLightNode *r,CVector &Ldir);

	//! test if a face is lighted from a node
	bool RootShadowedFace(CBSPLightNode * node,CShortFace * f);
	//! test if a point is in shadow volume from a node
	bool RootExtShadowVolume(CBSPLightNode * node,CVector &v);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// routine avec optimisation de faces

	//! apply a bsp light on an object
	//! @param obj : the object to calculate
	//! @param tag : the value of tag in faces (1,2,4)
	//! @param L : vector defining lightsource
	CObject3D * BspApply(CObject3D * obj,int tag,CVector &L);

	//! apply a bsp light directionnal on an object
	//! @param obj : the object to calculate
	//! @param tag : the value of tag in faces (1,2,4)
	//! @param Ldir : vector defining lightsource
	CObject3D * BspDirectionnalApply(CObject3D * obj,int tag,CVector &Ldir);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// build

	//! build a bsp light from a list of faces due to light L
	//! faces represent shadow volume faces and shadowed faces tagged due to their property (SHADOWFACE,NORMALFACE)
	//! @param faces: list of triangles
	//! @param L: lightsource point
	void Build(CList <CTriangles> * faces,CVector &L);

	//! build a bsp light from a list of faces due to light L, and add it to the list of nodes CList(roots)
	//! faces represent shadow volume faces and shadowed faces tagged due to their property (SHADOWFACE,NORMALFACE)
	//! @param faces: list of triangles
	//! @param L: lightsource point
	void Add(CList <CTriangles> * faces,CVector &L);

	//! build a bsp light and return root node from a list of faces due to light L
	//! faces represent shadow volume faces and shadowed faces tagged due to their property (SHADOWFACE,NORMALFACE)
	//! @param faces: list of triangles
	//! @param L: lightsource point	
	CBSPLightNode * BuildNode(CList <CTriangles> * faces,CVector &L);
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// routine brute


	//! apply a sub bsp on an object due to light vector Ldir
	//! @param thisroot : nodes root
	//! @param obj : the object to calculate
	//! @param tag : the value of tag in faces (1,2,4)
	//! @param Ldir : vector defining lightsource
	CObject3D * RootApplyDirectionnal(CBSPLightNode *thisroot,CObject3D * obj,int tag,CVector &Ldir);
	//! apply a sub bsp on an object due to light L
	//! @param thisroot : nodes root
	//! @param obj : the object to calculate
	//! @param tag : the value of tag in faces (1,2,4)
	//! @param Ldir : vector defining lightsource
	CObject3D * RootApply(CBSPLightNode *thisroot,CObject3D * obj,int tag,CVector &L);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals
	
	void ShadowTraitment(CBSPLightNode * node,CList <CShortFace> *faces,float mul,int tag);

	bool FaceShadowTraitment(CBSPLightNode * node,CShortFace *f,float mul0,int tag);
	
	void CutFaces(int nP,CList <CShortFace> *faces);
	
	void CutFacesFast(int nP,CList <CShortFace> *faces,int NF,float mul);
	
	void PlaneList(CBSPLightNode * node);
	
	bool RootShadowedQuad(CBSPLightNode * node,CVector &v0,CVector &v1,CVector &v2,CVector &v3);
	
	bool ObjectInShadow(CBSPLightNode * node,CObject3D *obj);	
	
	CObject3D * Apply(CObject3D * obj,int tag,CVector &L);
	
	void ApplyToFaces(CBSPLightNode * node,CList <CShortFace> *faces);

	void ApplyToFacesGruged(CBSPLightNode * node,CList <CShortFace> *faces);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// free

	//! free ressource
	void Free();
	//! free ressource (internal)
	void FreeNode(CBSPLightNode * n);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// misc

	//! numbering nodes (internal)
	void NodeIndice(CBSPLightNode * n);
	//! get a node by number (internal)
	void getNode(CBSPLightNode * n,int index,CBSPLightNode **result);

	//! save a bsp light to data container
	void SaveBsp(CDataContainer *data);
	//! load a bsp light from data container
	void LoadBsp(CDataReader * data);
};

#endif
#endif
