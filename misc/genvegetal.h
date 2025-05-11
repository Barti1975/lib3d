
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

#ifndef _GENVEGETAL_H_
#define _GENVEGETAL_H_

#include "../base/params.h"

#ifdef _DEFINES_API_CODE_GENVEGETAUX_

#ifdef _DEFINES_API_CODE_METABALLS_
#include "metaballs.h"
#endif

#if defined(GLES)||defined(GLES20)
#include <stdlib.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CRandomManager
{
public:
	unsigned int *aleatoire;
	int naleatoire;
	int pos;

	CRandomManager();
	~CRandomManager();
	CRandomManager(int ize,int size);
	
	int init(int seed);
	int getRand();
	int getRand(int mod);
};

/////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CVegetalParams
{
public:

	int depth;
	float height;
	float scale_level;
	float strength;
	float leaf;
	float size_leafs;
	int lod;
	int distort;
	float noding;
	CRandomManager *hazard;
	int values0;
	float values1;
	float values2;
	float values3;
	float values4;
	float values5;
	float values6;
	float values7;
	
	CVegetalParams() {}

	void init(int dep,CRandomManager *hasard,float p1,float p2,float p3,float p4,float p5,int p6,int p7,float p8)
	{
		hazard=hasard;
		depth=dep;
		height=p1;
		scale_level=p2;
		strength=p3;
		leaf=p4;
		size_leafs=p5;
		lod=p6;
		distort=p7;
		noding=p8;
	}

	void initm(int m1,float m2,float m3,float m4,float m5,float m6,float m7,float m8)
	{
		values0=m1;
		values1=m2;
		values2=m3;
		values3=m4;
		values4=m5;
		values5=m6;
		values6=m7;
		values7=m8;
	}

};

/////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	LOW_DETAILS=1,
	HIGH_DETAILS=0
};

/////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CVegetal
{
public:
	int cloned;
	C3DAPI* Render;
	CObject3D * level[10];
	CVertexBuffer *vb_level[10];
	CGenerator *glevel[10];
	CVegetalParams *params;
	int tex[10];
	int bump[10];
	int nfaces,nvertices;
	CVector Coo;
	CObject3D *Eng;
	int mode;
	float rx,ry,rz;
	float x0,y0,z0;
	float x1,y1,z1;
	float cx,cy,cz;
	bool box_calc;
	CVector mini,maxi;
	CVector _mini,_maxi;
	int onlyleafs;
#ifdef _DEFINES_API_CODE_METABALLS_
	CMetaballs *meta;
	int metaactive;
#endif

	CVegetal() 
	{
		int n;
#ifdef _DEFINES_API_CODE_METABALLS_
		meta=NULL;
#if defined(GLES)||defined(GLES20)
		metaactive=0;
#else
		metaactive=1;
#endif
#endif
		mini.Init(100000,100000,100000);
		maxi.Init(-100000,-100000,-100000);
		onlyleafs=0;
		box_calc=false;
		rx=ry=rz=0;
		mode=0;
		cloned=0;
		for (n=0;n<10;n++) level[n]=NULL;
		for (n=0;n<10;n++) vb_level[n]=NULL;
		for (n=0;n<10;n++) glevel[n]=NULL;
		Eng=NULL;
	}

	void setRender(C3DAPI *r) {Render=r;}
	void setMode(int m);

	void gen(CVegetalParams *p,float size,int seed,int details);
	void recurs(int N,CVector base,CVector vect,float size);
	void recurs2(int N,CVector base,CVector vect,float size);
#ifdef _DEFINES_API_CODE_METABALLS_
	void recursMeta(int N,CVector base,CVector vect,float size);
#endif
	void draw(CVector camera,CVector light);
	void draw(CVector camera,CVector light,CVector ambient,CVector color);
	
	void setTex(int n1,int n2,int n3,int n4,int n5);
	void setBump(int n1,int n2,int n3,int n4,int n5);
	
	void clean();
	int isClone(CVegetalParams *p1,float size1,CVegetalParams *p2,float size2);
	void clone(CVegetalParams *p,CVegetal * vg);
	void freeUnusefullStuffs();
	void setDistort(float x,float y,float z);
};

/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int dep;
	float p1,p2,p3,p4,p5,p6,p7,p8;
	float size;
	int m1;
	float m2,m3,m4,m5,m6,m7,m8;
	int tex[5];
	int bump[5];
	
} sVegetal;

/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int dep;
	float p1,p2,p3,p4,p5,p6,p7,p8;
	float size;
	int tex[5];
	int bump[5];
	
} sOldVegetal;

///////////////////////////////////////////////////////////////////////////////////////////// 
extern CRandomManager hazard;
///////////////////////////////////////////////////////////////////////////////////////////// 
/*

  Usage:

	CVegetalParams vg_p;
	CVegetal vg;
	CVegetal vg2;

	vg_p.init(3,&hazard,1,0.5f,0.05f,0.7f,1.5f,12,50,0.6f);
	vg.gen(&vg_p,5,SEED,HIGH_DETAILS);
	vg.setTex(1,1,1,0,0);
	vg.setBump(2,2,2,0,0);

	vg2.gen(&vg_p,5,SEED,LOW_DETAILS);
	vg2.setTex(1,1,1,0,0);
	vg2.setBump(2,2,2,0,0);

*/ 
/////////////////////////////////////////////////////////////////////////////////////////////

#endif
#endif
