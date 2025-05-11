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


#ifndef _BSP_ORTHO_H_
#define _BSP_ORTHO_H_

#include "bsp_volumic.h"

enum
{
	ORTHOBSP_NODE	=	1,
	ORTHOBSP_TERM	=	2,
	AXE_x			=	0,
	AXE_y			=	1,
	AXE_z			=	2

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CFaceGROUP
{
public:
	CList <int> *faces;
	CList <CFigure> *figures;

	unsigned short int id;
	unsigned short int nT;
	unsigned short int nL;
	unsigned short int tag;
#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_
	CMap <int,CBSPVolumicOld> *bsps;
#endif	

	CFaceGROUP();
	~CFaceGROUP();

	void clean();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CBSPOrthoNode
{
public:
	unsigned char type;
	float a,b,c,d;
	CBSPOrthoNode *plus,*moins;
	CList <int> *faces;
	CList <CFaceGROUP> *groups;

	CBSPOrthoNode();
	void clean();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CBSPOrtho
{
public:

	CBSPOrthoNode *root;
	CList <int> *ListeFaces;
	CList <int> *ListeFacesTH[64];
	CList <CFaceGROUP> *Groups;
	CList <CFaceGROUP*> *tmpGroups;

	CBSPOrtho();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// calculate an orthogonal planes based BSP

		CBSPOrthoNode * BuildNode(int Niter,CList <int> *faces,CObject3D * obj,int axe);
		CBSPOrthoNode * BuildNode2D(int Niter,CList <int> *faces,CObject3D * obj,int axe);
		void Build(int Niter,CObject3D * obj);

		CBSPOrthoNode * BuildNodeWithFigures2D(int Niter,CList <CFaceGROUP> *groups,CObject3D * obj,int axe);
		CBSPOrthoNode * BuildNodeWithFigures(int Niter,CList <CFaceGROUP> *groups,CObject3D * obj,int axe);
		void BuildWithFigures(int Niter,CObject3D * obj);
		void BuildWithFigures2(int Niter,CObject3D * obj);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// retrieve face list inside a sphere of radius R and center P
	// M is the actual tranformation for the object
	
		CList <int> * GetFaceList(CVector &P,float R,CMatrix &M);
		CList <int> * GetFaceList(CVector &P,float R,CMatrix &M,int buf);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internal
		
		void RecursGetFaceList(CBSPOrthoNode *node,CVector &P,float R);
		void RecursGetFaceList(CBSPOrthoNode *node,CVector &P,float R,int buf);

		void RecursFrustumGetFaceList(CBSPOrthoNode *node,CVector * frustum);
		void RecursFrustumGetGroupList(CBSPOrthoNode *node,CVector * frustum);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// retrieves faces/groups inside frustum[5] 
	
		CList <int> * FrustumGetFaceList(CVector * frustum,CMatrix &M);
		CList <CFaceGROUP*> * FrustumGetGroupList(CVector * frustum,CMatrix &M);

		CList <CFaceGROUP*> * GetGroupList();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// misc

		void FreeNode(CBSPOrthoNode * node);
		void Free();

		void cleanNode(CBSPOrthoNode * node);
		void clean();
};

#endif
