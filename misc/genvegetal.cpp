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

#define MAXVERTICESVEG 50000
#define MAXFACESVEG 50000
////////////////////////////////////////////////////////////////////////
//	@file genvegetal.h 
//	@created September 2004
////////////////////////////////////////////////////////////////////////
#include "../base/params.h"
#include "../data/lzcompress.h"
#include "../data/png.h"
#include "../base/maths.h"
#include "../base/objects3d.h"
#include "../base/3d_api_base.h"
#include "../base/3d_api.h"
#include "../base/edges.h"
#include "../base/bsp_light.h"
#include "../data/scene.h"
#include "trees.h"
#include "softrenderer.h"

#if defined(GLES)||defined(GLES20)
#include <stdlib.h>
#endif

#include <math.h>

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

#include "genvegetal.h"

#ifdef _DEFINES_API_CODE_GENVEGETAUX_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "random_values.c"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRandomManager::CRandomManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRandomManager::~CRandomManager()
{ 
	delete [] aleatoire; 
	naleatoire=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRandomManager::CRandomManager(int ize,int size)
{
	int n;
	aleatoire=new unsigned int[size];

#ifdef RANDOM_PROCESS
	int tmp;
	for (n=0;n<ize;n++) tmp=rand();		
	for (n=0;n<size;n++) aleatoire[n]=rand();
#else
	int nb=SIZE_random_values/4;
	int * values=(int*)FILE_random_values;
	for (n=0;n<size;n++) aleatoire[n]=values[n%nb];
#endif
	naleatoire=size;
	pos=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CRandomManager::init(int seed)
{
	pos=seed;
	pos=pos%naleatoire;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CRandomManager::getRand()
{
	int n=aleatoire[pos];
	pos++;
	pos=pos%naleatoire;
	return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CRandomManager::getRand(int mod)
{
	int n=(aleatoire[pos]%(mod*2)) - mod;
	pos+=10;
	pos=pos%naleatoire;
	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(GLES)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "tronc.c"
#include "feuilles.c"
#include "tronc_blend.c"
#include "feuilles_blend.c"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVertexBuffer *_gen_vegetal_effect_tronc=NULL;
CVertexBuffer *_gen_vegetal_effect_feuilles=NULL;
CVertexBuffer *_gen_vegetal_effect_tronc_blend=NULL;
CVertexBuffer *_gen_vegetal_effect_feuilles_blend=NULL;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRandomManager hazard(1500,1000);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::setTex(int n1,int n2,int n3,int n4,int n5)
{
	tex[0]=n1;
	tex[1]=n2;
	tex[2]=n3;
	tex[3]=n4;
	tex[4]=n5;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::setBump(int n1,int n2,int n3,int n4,int n5)
{
	bump[0]=n1;
	bump[1]=n2;
	bump[2]=n3;
	bump[3]=n4;
	bump[4]=n5;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::setMode(int m)
{
	mode=m;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::draw(CVector camera,CVector light)
{
	CVector color,ambient;

	color.Init(1,1,1);
	ambient.Init(0.2f,0.2f,0.2f);
	draw(camera,light,ambient,color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::draw(CVector camera,CVector light,CVector ambient,CVector color)
{
	int n;
	int start=0;
	CMatrix M;
	CVertexBuffer *tronc;
	CVertexBuffer *feuilles;

	if (onlyleafs) start=params->depth;

#if !defined(GLES)

	if (_gen_vegetal_effect_tronc==NULL)
	{
		LOG("_gen_vegetal_effect_tronc");
		_gen_vegetal_effect_tronc=new CVertexBuffer;
		_gen_vegetal_effect_tronc->SetAPI(Render);
		_gen_vegetal_effect_tronc->BasicEffect();
		_gen_vegetal_effect_tronc->SetVertexProgram(script_tronc);
	}

	if (_gen_vegetal_effect_feuilles==NULL)
	{
		LOG("_gen_vegetal_effect_feuilles");
		_gen_vegetal_effect_feuilles=new CVertexBuffer;
		_gen_vegetal_effect_feuilles->SetAPI(Render);
		_gen_vegetal_effect_feuilles->BasicEffect();
		_gen_vegetal_effect_feuilles->SetVertexProgram(script_feuilles);
	}

	if (_gen_vegetal_effect_tronc_blend==NULL)
	{
		LOG("_gen_vegetal_effect_tronc_blend");
		_gen_vegetal_effect_tronc_blend=new CVertexBuffer;
		_gen_vegetal_effect_tronc_blend->SetAPI(Render);
		_gen_vegetal_effect_tronc_blend->BasicEffect();
		_gen_vegetal_effect_tronc_blend->SetVertexProgram(script_tronc_blend);
	}

	if (_gen_vegetal_effect_feuilles_blend==NULL)
	{
		LOG("_gen_vegetal_effect_feuilles_blend");
		_gen_vegetal_effect_feuilles_blend=new CVertexBuffer;
		_gen_vegetal_effect_feuilles_blend->SetAPI(Render);
		_gen_vegetal_effect_feuilles_blend->BasicEffect();
		_gen_vegetal_effect_feuilles_blend->SetVertexProgram(script_feuilles_blend);
	}

	if (mode==1) feuilles=_gen_vegetal_effect_feuilles_blend;
	else feuilles=_gen_vegetal_effect_feuilles;

	if (mode==1) tronc=_gen_vegetal_effect_tronc_blend;
	else tronc=_gen_vegetal_effect_tronc;
    
    Render->SetParams(API3D_CULL,CCW);

	if (mode==1)
	{
		M.Id();
		M.RotationDegre(rx,ry,rz);
		M=M*Render->WORLD;
		M=M*Render->REFLECTION;

#ifdef ANDROID
		Render->SetParams(API3D_BLEND,OFF);
		CVertexBuffer *vb;
		vb=tronc;
		vb->setTexture("Tex",tex[0]);
		vb->setVector("Camera",camera);
		vb->setVector("Light",-1*light);
		vb->setVector("Color",color);
		vb->setVector("Ambient",ambient);
		vb->setMatrixViewProj("VIEWPROJ");
		vb->setMatrixWorld("WORLD");
		vb->setMatrix("DISTORT",M);
		Render->SetEffect(vb);
		Render->SetVertexBuffer(vb_level[0]);
		Render->DrawVertexBuffer();
		Render->SetEffect(NULL);

		vb=feuilles;
		vb->setTexture("Tex",tex[params->depth]);
		vb->setVector("Camera",camera);
		vb->setVector("Light",-1*light);
		vb->setVector("Color",color);
		vb->setVector("Ambient",ambient);
		vb->setMatrixViewProj("VIEWPROJ");
		vb->setMatrixWorld("WORLD");
		vb->setMatrix("DISTORT",M);
		Render->SetEffect(vb);
		Render->SetVertexBuffer(vb_level[1]);
		Render->DrawVertexBuffer();
		Render->SetEffect(NULL);

#else
		Render->SetParams(API3D_BLEND,OFF);
		for (n=start;n<=params->depth;n++)
		{
			if (vb_level[n])
			{				
				CVertexBuffer *vb;
				if (tex[n]==tex[params->depth]) vb=feuilles; else vb=tronc;
				vb->setTexture("Tex",tex[n]);
				vb->setVector("Camera",camera);
				vb->setVector("Light",-1*light);
				vb->setVector("Color",color);
				vb->setVector("Ambient",ambient);
				vb->setMatrixViewProj("VIEWPROJ");
				vb->setMatrixWorld("WORLD");
				vb->setMatrix("DISTORT",M);
				Render->SetEffect(vb);
				Render->SetVertexBuffer(vb_level[n]);
				Render->DrawVertexBuffer();
				Render->SetEffect(NULL);
			}
		}
#endif
	}
	else
	{
		Render->SetParams(API3D_BLEND,OFF);
		for (n=start;n<=params->depth;n++)
		{
			if (vb_level[n])
			{
				CVertexBuffer *vb;
				if (tex[n]==tex[params->depth]) vb=feuilles; else vb=tronc;
				vb->setTexture("Tex",tex[n]);
				vb->setVector("Camera",camera);
				vb->setVector("Light",-1*light);
				vb->setVector("Color",color);
				vb->setVector("Ambient",ambient);
				vb->setMatrixViewProj("VIEWPROJ");
				vb->setMatrixWorld("WORLD");
				Render->SetEffect(vb);
				Render->SetVertexBuffer(vb_level[n]);
				Render->DrawVertexBuffer();
				Render->SetEffect(NULL);
			}
		}
	}
#else

	Render->SetParams(API3D_ALPHATEST,DISABLE);

	Render->SetParams(API3D_LIGHTING,ON);
	Render->SetLightPoint(0,light,color.x,color.y,color.z,0,0,0,0.01f,150);
	Render->EnableLight(0);

	for (n=start;n<=params->depth;n++)
	{
		if (vb_level[n])
		{
			if (n==params->depth)
			{
				Render->SetParams(API3D_ALPHATEST,ENABLE);
				Render->SetParams(API3D_ALPHATEST,128);
				Render->SetParams(API3D_ALPHATEST,GREATER);
				Render->SetParams(API3D_BLEND,OFF);
			}
			else Render->SetParams(API3D_BLEND,OFF);

			Render->SetTexture(tex[n]);
			Render->SetVertexBuffer(vb_level[n]);
			Render->DrawVertexBuffer();
		}
	}
	Render->SetParams(API3D_LIGHTING,OFF);
	Render->DisableLight(0);
#endif

	Render->SetParams(API3D_ALPHATEST,DISABLE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::clean()
{
	int n;

	if (cloned)
	{
		for (n=0;n<10;n++)
		{
			level[n]=NULL;
			vb_level[n]=NULL;
		}
	}
	else
	{
		for (n=0;n<10;n++)
		{
			if (vb_level[n])
			{
				vb_level[n]->Release();
				delete vb_level[n];
				vb_level[n]=NULL;
			}

			if (level[n])
			{
				level[n]->Free();
				delete level[n];
				level[n]=NULL;
			}
		}
	}
	cloned=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::setDistort(float x,float y,float z)
{
	rx=x; ry=y; rz=z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::gen(CVegetalParams *p,float size,int seed,int details)
{
	int n,k;
	CVector base,u,v;
	float maxy,miny,t;
	CObject3D * obj;
	
	p->hazard->init(seed);

	nfaces=0;
	nvertices=0;
	params=p;

	for (n=0;n<10;n++)
	{
		glevel[n]=new CGenerator(MAXVERTICESVEG,MAXFACESVEG);
		glevel[n]->Reset();
		level[n]=NULL;
	}

	base.Init(0,0,0);
	u.Init(SMALLF,-1,0);
	u.Normalise();
#ifdef _DEFINES_API_CODE_METABALLS_
	if (metaactive)
	{
		meta=new CMetaballs;
		meta->vertexbuffer_update=false;
		meta->nMetaballs=0;
		meta->nMetaballsTube=0;
		meta->ENVMAPPING=false;
		meta->gravitycenter=false;

		recursMeta(0,base,u,size);
		
		meta->SetEquipotentielle(3.8f,0.0001f);
		
		meta->nFaces=0;
		meta->nVertices=0;

		if (details==0) meta->DIVISION=0.23f;
		else meta->DIVISION=0.45f;

		meta->Calculate();

		if (meta->OBJ.nFaces>0)
		{
			for (n=0;n<meta->OBJ.nFaces;n++)
			{
				if ((meta->OBJ.Faces[n].v[0]->Stok.y<-SMALLF2)||(meta->OBJ.Faces[n].v[1]->Stok.y<-SMALLF2)||(meta->OBJ.Faces[n].v[2]->Stok.y<-SMALLF2)) meta->OBJ.Faces[n].tag=1;
				else meta->OBJ.Faces[n].tag=0;
			}

			level[0]=meta->OBJ.DuplicateSelected2(1);
		}
	
		meta->Free();
	}
	else
	{
#endif
		if (details==0) recurs(0,base,u,size);
		else recurs2(0,base,u,size);
#ifdef _DEFINES_API_CODE_METABALLS_
	}
#endif

#if !defined(GLES)
	if (_gen_vegetal_effect_tronc==NULL)
	{
		LOG("_gen_vegetal_effect_tronc");
		_gen_vegetal_effect_tronc=new CVertexBuffer;
		_gen_vegetal_effect_tronc->SetAPI(Render);
		_gen_vegetal_effect_tronc->BasicEffect();
		_gen_vegetal_effect_tronc->SetVertexProgram(script_tronc);
	}

	if (_gen_vegetal_effect_feuilles==NULL)
	{
		LOG("_gen_vegetal_effect_feuilles");
		_gen_vegetal_effect_feuilles=new CVertexBuffer;
		_gen_vegetal_effect_feuilles->SetAPI(Render);
		_gen_vegetal_effect_feuilles->BasicEffect();
		_gen_vegetal_effect_feuilles->SetVertexProgram(script_feuilles);
	}

	if (_gen_vegetal_effect_tronc_blend==NULL)
	{
		LOG("_gen_vegetal_effect_tronc_blend");
		_gen_vegetal_effect_tronc_blend=new CVertexBuffer;
		_gen_vegetal_effect_tronc_blend->SetAPI(Render);
		_gen_vegetal_effect_tronc_blend->BasicEffect();
		_gen_vegetal_effect_tronc_blend->SetVertexProgram(script_tronc_blend);
	}

	if (_gen_vegetal_effect_feuilles_blend==NULL)
	{
		LOG("_gen_vegetal_effect_feuilles_blend");
		_gen_vegetal_effect_feuilles_blend=new CVertexBuffer;
		_gen_vegetal_effect_feuilles_blend->SetAPI(Render);
		_gen_vegetal_effect_feuilles_blend->BasicEffect();
		_gen_vegetal_effect_feuilles_blend->SetVertexProgram(script_feuilles_blend);
	}
#endif

	if (mode==1)
	{
#ifdef _DEFINES_API_CODE_METABALLS_
		if (metaactive)
		{
			for (n=0;n<=params->depth;n++)
				if (level[n])
				{
					for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
					nvertices+=level[n]->nVertices;
					nfaces+=level[n]->nFaces;
				}
				else
				{
					level[n]=glevel[n]->pResObjet();
					for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
					nvertices+=level[n]->nVertices;
					nfaces+=level[n]->nFaces;
				}
		}
		else
		{
#endif
			for (n=0;n<=params->depth;n++)
			{
				level[n]=glevel[n]->pResObjet();
				for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
				nvertices+=level[n]->nVertices;
				nfaces+=level[n]->nFaces;
			}
#ifdef _DEFINES_API_CODE_METABALLS_
		}
#endif
		miny=100000;
		maxy=-100000;

		for (n=0;n<=params->depth;n++)
		{
			for (k=0;k<level[n]->nVertices;k++)
			{
				if (level[n]->Vertices[k].Stok.y<miny) miny=level[n]->Vertices[k].Stok.y;
				if (level[n]->Vertices[k].Stok.y>maxy) maxy=level[n]->Vertices[k].Stok.y;
			}
		}

		for (n=0;n<=params->depth;n++)
		{
			for (k=0;k<level[n]->nVertices;k++)
			{
				t=(level[n]->Vertices[k].Stok.y-miny)/(maxy-miny);
				if (t<SMALLF) t=SMALLF;
				if (t>1.0f-SMALLF) t=1.0f-SMALLF;
				t=-0.5f+1.5f*exp(0.333f*log(t));
				if (t<SMALLF) t=SMALLF;
				if (t>1.0f-SMALLF) t=1.0f-SMALLF;
				level[n]->Vertices[k].Weight[0]=t;
				level[n]->Vertices[k].Weight[1]=1.0f-t;
			}
		}

#ifdef ANDROID

		CGenerator * g=new CGenerator(MAXVERTICESVEG,MAXFACESVEG);
		g->Reset();

		CGenerator * g2=new CGenerator(MAXVERTICESVEG,MAXFACESVEG);
		g2->Reset();

		for (n=0;n<=params->depth;n++)
		if (level[n])
		{
			if (level[n]->nFaces>0)
			{
				level[n]->CalculateNormals(-1);
				if (tex[n]==tex[params->depth]) g2->Add(level[n]);
				else g->Add(level[n]);
			}
		}

		CObject3D *tronc=g->pResObjet();
		vb_level[0]=new CVertexBuffer;
		vb_level[0]->SetAPI(Render);
		vb_level[0]->CreateFrom2(tronc,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_BLENDING|API3D_TEXPERVERTEX);
		tronc->Free();
		delete tronc;

		CObject3D *feuilles=g2->pResObjet();
		vb_level[1]=new CVertexBuffer;
		vb_level[1]->SetAPI(Render);
		vb_level[1]->CreateFrom2(feuilles,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_BLENDING|API3D_TEXPERVERTEX);
		feuilles->Free();
		delete feuilles;

		delete g;
		delete g2;
#else
		for (n=0;n<=params->depth;n++)
		if (level[n])
		{
			if (level[n]->nFaces>0)
			{
				level[n]->CalculateNormals(-1);
				vb_level[n]=new CVertexBuffer;
				vb_level[n]->SetAPI(Render);
				vb_level[n]->CreateFrom2(level[n],API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_BLENDING|API3D_TEXPERVERTEX);
			}
		}
#endif
	}
	else
	{
#ifdef _DEFINES_API_CODE_METABALLS_
		if (metaactive)
		{
			for (n=0;n<=params->depth;n++)
			if (level[n])
			{				
				for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
				nvertices+=level[n]->nVertices;
				nfaces+=level[n]->nFaces;

				if (level[n]->nFaces>0)
				{
					level[n]->CalculateNormals(-1);
					vb_level[n]=new CVertexBuffer;
					vb_level[n]->SetAPI(Render);
					vb_level[n]->CreateFrom2(level[n],API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
				}
			}
			else
			{
				level[n]=glevel[n]->pResObjet();
				for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
				nvertices+=level[n]->nVertices;
				nfaces+=level[n]->nFaces;

				if (level[n]->nFaces>0)
				{
					level[n]->CalculateNormals(-1);
					vb_level[n]=new CVertexBuffer;
					vb_level[n]->SetAPI(Render);
					vb_level[n]->CreateFrom2(level[n],API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
				}
				/**/
			}
		}
		else
		{
#endif
			for (n=0;n<=params->depth;n++)
			{
				level[n]=glevel[n]->pResObjet();
				for (k=0;k<level[n]->nVertices;k++) level[n]->Vertices[k].Diffuse.Init(1,1,1);
				nvertices+=level[n]->nVertices;
				nfaces+=level[n]->nFaces;

				if (level[n]->nFaces>0)
				{
					level[n]->CalculateNormals(-1);
					vb_level[n]=new CVertexBuffer;
					vb_level[n]->SetAPI(Render);
					vb_level[n]->CreateFrom2(level[n],API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
				}
			}
#ifdef _DEFINES_API_CODE_METABALLS_
		}
#endif
	}

	for (n=0;n<10;n++)
	{
		delete glevel[n];
		glevel[n]=NULL;
	}

#ifdef _DEFINES_API_CODE_METABALLS_
	if (metaactive) delete meta;
	meta=NULL;
#endif

	_mini.Init(10000,10000,10000);
	_maxi.Init(-10000,-10000,-10000);

	for (int p=0;p<=params->depth;p++)
	if (level[p])
	{
		obj=level[p];

		for (n=0;n<obj->nVertices;n++)
		{
			v=obj->Vertices[n].Stok;
			if (v.x<_mini.x) _mini.x=v.x;
			if (v.y<_mini.y) _mini.y=v.y;
			if (v.z<_mini.z) _mini.z=v.z;
			if (v.x>_maxi.x) _maxi.x=v.x;
			if (v.y>_maxi.y) _maxi.y=v.y;
			if (v.z>_maxi.z) _maxi.z=v.z;
		}

	}

	mini.Init(10000,10000,10000);
	maxi.Init(-10000,-10000,-10000);

	obj=level[params->depth];
	if (obj)
	for (n=0;n<obj->nVertices;n++)
	{
		v=obj->Vertices[n].Stok;
		if (v.x<mini.x) mini.x=v.x;
		if (v.y<mini.y) mini.y=v.y;
		if (v.z<mini.z) mini.z=v.z;
		if (v.x>maxi.x) maxi.x=v.x;
		if (v.y>maxi.y) maxi.y=v.y;
		if (v.z>maxi.z) maxi.z=v.z;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVegetal::isClone(CVegetalParams *p1,float size1,CVegetalParams *p2,float size2)
{
	int res=0;

	if ((p1->depth==p2->depth)&&(p1->distort==p2->distort)&&(p1->height==p2->height)&&(p1->leaf==p2->leaf)&&
		(p1->lod==p2->lod)&&(p1->noding==p2->noding)&&(p1->scale_level==p2->scale_level)&&(p1->size_leafs==p2->size_leafs)&&
		(p1->strength==p2->strength)&&(size1==size2)&&(p1->values0==p2->values0)) res=1;

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::clone(CVegetalParams *p,CVegetal * vg)
{
	int n;

	params=p;

	cloned=1;
	for (n=0;n<10;n++)
	{
		level[n]=vg->level[n];
		vb_level[n]=vg->vb_level[n];
		tex[n]=vg->tex[n];
		bump[n]=vg->bump[n];
	}

	mini=vg->mini;
	maxi=vg->maxi;

	_mini=vg->_mini;
	_maxi=vg->_maxi;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::freeUnusefullStuffs()
{
	int n;

	if (cloned)
	{
		for (n=0;n<=params->depth;n++) level[n]=NULL;
	}
	else
	{
		for (n=0;n<=params->depth;n++)
		{
			if (level[n])
			{
				level[n]->Free();
				delete level[n];
				level[n]=NULL;
			}
		}
	}

	if (Eng)
	{
		Eng->Free();
		delete Eng;
		Eng=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::recurs(int N,CVector base,CVector vect,float size)
{
	CObject3D obj;
	float l,l2,r,r2,t;
	int n,def;
	float rr;
	CMatrix M,Mr;
	CVector p,u,v,zero;
	float rrx,rry,rrz;

	zero.Init(0,0,0);
	def=params->lod - N;
	def=def/2;
	if (def<3) def=3;
	l=size;
	r=l*params->strength;
	l2=size*params->scale_level;
	r2=l2*params->strength;
	u=vect;
	VECTORNORMALISE(u);
	VECTORMUL(p,u,0.5f*size);
	VECTORADD(p,p,base);

	if (N==params->depth)
	{
		obj.Plane(params->size_leafs);
		obj.ScaleMappingCoordinate(0.1f,0.1f);
		M.Id();
		M.Orientation_y(p,u);
		Mr.Id();
		Mr.Orientation_y(zero,u);
		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		M.Id();
		M.Orientation_y(p,-1*u);
		Mr.Id();
		Mr.Orientation_y(zero,-1*u);
		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		obj.Free();
	}
	else
	{
		if (N>=params->values0)
		{
			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;

			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;

			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;
		}
		else
		{
			if (N==0) obj.CorDeChassevoid(l,r2,2*r,def);
			else obj.Cylindre3void(l,r2,r,def);
			M.Id();
			M.Orientation_z(p,u);
			Mr.Id();
			Mr.Orientation_z(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();
		}

		if (N==params->depth-1) def=2*params->leaf;
		else def=5*params->leaf;

		for (n=0;n<def+1;n++)
		{
			t=((float) (params->hazard->getRand()%100))/100;

			float sz=size*(params->noding*(1-t) + t);
			VECTORMUL(p,u,sz);
			VECTORADD(p,p,base);

			rrx=params->hazard->getRand(params->distort);
			rry=params->hazard->getRand(params->distort);
			rrz=params->hazard->getRand(params->distort);
			M.Id();
			M.RotationDegre(rrx,rry,rrz);
			VECTOR3MATRIX(v,u,M);

			if ((N<params->depth-1)||((N==params->depth-1)&&((n%4)!=0)&&(params->size_leafs>0))) recurs(N+1,p,v,size*params->scale_level);
		}
		VECTORMUL(p,u,size);
		VECTORADD(p,p,base);

		rrx=params->hazard->getRand(params->distort/5);
		rry=params->hazard->getRand(params->distort/5);
		rrz=params->hazard->getRand(params->distort/5);
		M.Id();
		M.RotationDegre(rrx,rry,rrz);

		VECTOR3MATRIX(v,u,M);
		if ((N==params->depth-1)&&(params->size_leafs>0)) recurs(N+1,p,v,size*params->scale_level);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVegetal::recurs2(int N,CVector base,CVector vect,float size)
{
	CObject3D obj;
	float l,l2,r,r2,t;
	int n,def;
	float rr;
	CMatrix M,Mr;
	CVector p,u,v,zero;
	float rrx,rry,rrz;

	zero.Init(0,0,0);

	def=params->lod-N;
	def=def/2;
	if (def<3) def=3;

	l=size;
	r=l*params->strength;
	l2=size*params->scale_level;
	r2=l2*params->strength;

	u=vect;
	VECTORNORMALISE(u);
	VECTORMUL(p,u,0.5f*size);
	VECTORADD(p,p,base);

	if (N==params->depth)
	{
		obj.Plane(params->size_leafs);
		obj.ScaleMappingCoordinate(0.1f,0.1f);
		M.Id();
		M.Orientation_y(p,u);
		Mr.Id();
		Mr.Orientation_y(zero,u);

		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		M.Id();
		M.Orientation_y(p,-1*u);
		Mr.Id();
		Mr.Orientation_y(zero,-1*u);

		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		obj.Free();
	}
	else
	{
		if (N<params->depth-1)
		{
			if (N>=params->values0)
			{
				obj.Plane(params->size_leafs);
				obj.ScaleMappingCoordinate(0.1f,0.1f);
				M.Id();
				M.Orientation_y(p,u);
				Mr.Id();
				Mr.Orientation_y(zero,u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				M.Id();
				M.Orientation_y(p,-1*u);
				Mr.Id();
				Mr.Orientation_y(zero,-1*u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				obj.Free();

				rr=u.x;
				u.x=u.y; u.y=u.z; u.z=rr;

				obj.Plane(params->size_leafs);
				obj.ScaleMappingCoordinate(0.1f,0.1f);
				M.Id();
				M.Orientation_y(p,u);
				Mr.Id();
				Mr.Orientation_y(zero,u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				M.Id();
				M.Orientation_y(p,-1*u);
				Mr.Id();
				Mr.Orientation_y(zero,-1*u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				obj.Free();

				rr=u.x;
				u.x=u.y; u.y=u.z; u.z=rr;

				obj.Plane(params->size_leafs);
				obj.ScaleMappingCoordinate(0.1f,0.1f);
				M.Id();
				M.Orientation_y(p,u);
				Mr.Id();
				Mr.Orientation_y(zero,u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				M.Id();
				M.Orientation_y(p,-1*u);
				Mr.Id();
				Mr.Orientation_y(zero,-1*u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				obj.Free();

				rr=u.x;
				u.x=u.y; u.y=u.z; u.z=rr;

			}
			else
			{
				if (N==0) obj.CorDeChassevoid(l,r2,2.4f*r,3); else obj.Cylindre3void(l,r2,r,3);
				M.Id();
				M.Orientation_z(p,u);
				Mr.Id();
				Mr.Orientation_z(zero,u);
				obj.Calculate(&M,&Mr);
				glevel[N]->AddCalc(&obj);
				obj.Free();
			}
		}
		
		if (N==params->depth-1) def=2*params->leaf;
		else def=5*params->leaf;

		for (n=0;n<def+1;n++)
		{
			t=((float) (params->hazard->getRand()%100))/100;
			VECTORMUL(p,u,size*(params->noding*(1-t) + t));
			VECTORADD(p,p,base);
			rrx=params->hazard->getRand(params->distort);
			rry=params->hazard->getRand(params->distort);
			rrz=params->hazard->getRand(params->distort);
			M.Id();
			M.RotationDegre(rrx,rry,rrz);
			VECTOR3MATRIX(v,u,M);
			if ((N<params->depth-1)||((N==params->depth-1)&&((n%6)!=0)&&(params->size_leafs>0))) recurs2(N+1,p,v,size*params->scale_level);
		}
		VECTORMUL(p,u,size);
		VECTORADD(p,p,base);

		rrx=params->hazard->getRand(params->distort/5);
		rry=params->hazard->getRand(params->distort/5);
		rrz=params->hazard->getRand(params->distort/5);
		M.Id();
		M.RotationDegre(rrx,rry,rrz);
		VECTOR3MATRIX(v,u,M);
		if ((N==params->depth-1)&&(params->size_leafs>0)) recurs2(N+1,p,v,size*params->scale_level);

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEFINES_API_CODE_METABALLS_
void CVegetal::recursMeta(int N,CVector base,CVector vect,float size)
{
	CObject3D obj;
	float l,l2,r,r2,t;
	int n,def;
	float rr;
	CMatrix M,Mr;
	CVector p,u,v,zero;
	float rrx,rry,rrz;
	static float angles[10] = { 0.0f,67.0f,125.0f,245.0f,133.0f,14.0f,177.0f,329.0f,276.0f,89.0f };

	zero.Init(0,0,0);
	def=params->lod - N;
	def=def/2;
	if (def<3) def=3;
	l=size;
	r=l*params->strength;
	l2=size*params->scale_level;
	r2=l2*params->strength;
	u=vect;
	VECTORNORMALISE(u);
	VECTORMUL(p,u,0.5f*size);
	VECTORADD(p,p,base);

	if (N==params->depth)
	{
		obj.Plane(params->size_leafs);
		obj.ScaleMappingCoordinate(0.1f,0.1f);
		M.Id();
		M.Orientation_y(p,u);
		Mr.Id();
		Mr.Orientation_y(zero,u);
		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		M.Id();
		M.Orientation_y(p,-1*u);
		Mr.Id();
		Mr.Orientation_y(zero,-1*u);
		obj.Calculate(&M,&Mr);
		glevel[N]->AddCalc(&obj);
		obj.Free();
	}
	else
	{
		if (N>=params->values0)
		{
			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;

			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;

			obj.Plane(params->size_leafs);
			obj.ScaleMappingCoordinate(0.1f,0.1f);
			M.Id();
			M.Orientation_y(p,u);
			Mr.Id();
			Mr.Orientation_y(zero,u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			M.Id();
			M.Orientation_y(p,-1*u);
			Mr.Id();
			Mr.Orientation_y(zero,-1*u);
			obj.Calculate(&M,&Mr);
			glevel[N]->AddCalc(&obj);
			obj.Free();

			rr=u.x;
			u.x=u.y; u.y=u.z; u.z=rr;
		}
		else
		{
			int nb=1+(2*params->depth);
			int k;
			float KH=0.13f;
			int SC=5;
			float sl=0.7f;
			if (N==0) sl=0.5f;
			int poz=params->hazard->pos;

			if ((params->values1>0)&&(N>0))
			{
				sl/=0.35f*(params->values1*10+1);

				for (int i=0;i<params->values1*10+1;i++)
				{
					float t1=((float)i)/((float)(params->values1*10+1));
					float t2=((float)(i+1))/((float)(params->values1*10+1));

					for (k=0;k<nb;k++)
					{
						CVector rnd;
						rnd.x=params->hazard->getRand(params->distort/SC);
						rnd.y=params->hazard->getRand(params->distort/SC);
						rnd.z=params->hazard->getRand(params->distort/SC);
						VECTORNORMALISE(rnd);
						if (N==0) { VECTORMUL(rnd,rnd,(params->depth-N)*0.35f*r); }
						else { VECTORMUL(rnd,rnd,(params->depth-N)*0.5f*r); }

						CVector rnd2;
						rnd2.x=params->hazard->getRand(params->distort/SC);
						rnd2.y=params->hazard->getRand(params->distort/SC);
						rnd2.z=params->hazard->getRand(params->distort/SC);
						VECTORNORMALISE(rnd2);
						VECTORMUL(rnd2,rnd2,(params->depth-N)*0.35f*r2);

						CVector cc,vv,uu,p1,p2,pp,pp2;
						
						p1=base;
						if (N==0) p2=base+size*u; else p2=base+1.5f*size*u;
						uu=(p2-p1);
						vv=uu;
						vv.Normalise();
						cc.Base(vv,0);

						float xx1=(2.5f+N)*sinf(t1*PI);
						float xx2=(2.5f+N)*sinf(t2*PI);

						pp=p1+t1*uu +2.0f*xx1*r*cc;
						pp2=p1+t2*uu +2.0f*xx2*r*cc;

						if (N==0)
						{
							if (i==0) meta->AddMetaballTube(pp+rnd*sl,pp2+rnd2*sl,KH*3.2f*r*sl,KH*r2*sl);
							else meta->AddMetaballTube(pp+rnd*sl,pp2+rnd2*sl,KH*r*sl,KH*r2*sl);
						}
						else meta->AddMetaballTube(pp+rnd*sl,pp2+rnd2*sl,KH*r*sl,KH*r2*sl);
					}

				}
			}
			else
			{
				if (N==0)
				for (k=0;k<2+nb*params->scale_level*2;k++)
				{
					CVector rnd;
					rnd.x=params->hazard->getRand(params->distort/5);
					rnd.y=params->hazard->getRand(params->distort/5);
					rnd.z=params->hazard->getRand(params->distort/5);
					VECTORNORMALISE(rnd);
					VECTORMUL(rnd,rnd,1.5f*r);
					float t=(params->hazard->getRand(50)+50)/100.0f;
					if (t<0.2f) t=0.2f;
					if (t>1) t=1;
					meta->AddMetaball(base+rnd+t*size*u,0.6f*r*sl);
				}

				if (N==0)
				for (k=0;k<4*params->depth;k++)
				{
					CVector rpos,basepos;
					float ang=angles[(params->hazard->getRand(10)+10)%10]*PI/180.0f;
					float cf=(params->hazard->getRand(100)+120)/50.0f;
					rpos.Init(cos(ang),r2*cf,sin(ang));
					basepos.Init(0,-2*r2,0);
					meta->AddMetaballTube(base+basepos,base+basepos+rpos*r2*12*sl,12*r2*KH*sl,8*r2*KH*sl);
				}

				for (k=0;k<nb;k++)
				{
					CVector rnd;
					rnd.x=params->hazard->getRand(params->distort/SC);
					rnd.y=params->hazard->getRand(params->distort/SC);
					rnd.z=params->hazard->getRand(params->distort/SC);
					VECTORNORMALISE(rnd);
					if (N==0) { VECTORMUL(rnd,rnd,(params->depth-N)*0.35f*r); }
					else { VECTORMUL(rnd,rnd,(params->depth-N)*0.5f*r); }

					CVector rnd2;
					rnd2.x=params->hazard->getRand(params->distort/SC);
					rnd2.y=params->hazard->getRand(params->distort/SC);
					rnd2.z=params->hazard->getRand(params->distort/SC);
					VECTORNORMALISE(rnd2);
					VECTORMUL(rnd2,rnd2,(params->depth-N)*0.35f*r2);

					if (N==0) meta->AddMetaballTube(base+rnd*sl,base+rnd2*sl+size*u,KH*3.2f*r*sl,KH*r2*sl);
					else meta->AddMetaballTube(base+rnd*sl,base+rnd2*sl+1.5f*size*u,KH*r*sl,KH*r2*sl);
				}
			}

			params->hazard->pos=poz;
		}

		if (N==params->depth-1) def=2*params->leaf;
		else def=5*params->leaf;

		for (n=0;n<def+1;n++)
		{
			t=((float) (params->hazard->getRand()%100))/100;

			float sz=size*(params->noding*(1-t) + t);
			VECTORMUL(p,u,sz);
			VECTORADD(p,p,base);

			rrx=params->hazard->getRand(params->distort);
			rry=params->hazard->getRand(params->distort);
			rrz=params->hazard->getRand(params->distort);
			M.Id();
			M.RotationDegre(rrx,rry,rrz);
			VECTOR3MATRIX(v,u,M);

			if ((N<params->depth-1)||((N==params->depth-1)&&((n%4)!=0)&&(params->size_leafs>0))) recursMeta(N+1,p,v,size*params->scale_level);
		}
		VECTORMUL(p,u,size);
		VECTORADD(p,p,base);

		rrx=params->hazard->getRand(params->distort/5);
		rry=params->hazard->getRand(params->distort/5);
		rrz=params->hazard->getRand(params->distort/5);
		M.Id();
		M.RotationDegre(rrx,rry,rrz);

		VECTOR3MATRIX(v,u,M);
		if ((N==params->depth-1)&&(params->size_leafs>0)) recursMeta(N+1,p,v,size*params->scale_level);
	}
}
#endif


#endif
