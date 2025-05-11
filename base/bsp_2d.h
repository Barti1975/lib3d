
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

  
#ifndef _BSP_2D_H_
#define _BSP_2D_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	ID2D_NODE	=	1,
	ID2D_IN		=	2,
	ID2D_OUT	=	3,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CBSP2DNode
{
public:
	unsigned char id;
	float a,b,c,d;
	CBSP2DNode *p,*m;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CSegment
{
public:
	CVector a,b;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CSegmenti
{
public:
	int a,b,tag;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! 2D BSP to process figures.
//! some functions need CSegmenti,CSegment data structure to work : see the include file.
class API3D_STUB CBSP2D
{
public:
	CBSP2DNode *root;
	CList <CSegmenti> seg_out;
	CList <CSegmenti> seg_in;

	CVector *points;
	int npoints;
	CVector N;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CBSP2D() {root=NULL;}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! build a 2d volumic bsp from a figure, specifying vertices and 3d normal of the figure
	void Build(CFigure *f,CVertex *vertices,CVector &Normal);
	//! build a 2d bsp (internal)
	CBSP2DNode *BuildNode(CList <CSegment> *lseg);
	//! apply a 2d bsp on segment list
	void Apply(CBSP2DNode *node,CList <CSegmenti> *lseg);
	//! free ressource (internal)
	void FreeNode(CBSP2DNode *node);
	//! test if a point is inside a bsp
	bool Inside(CBSP2DNode *node,CVector &v);
	//! free ressource
	void Free();
};

#endif
