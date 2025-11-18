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
//	@file 3d_api.cpp 
////////////////////////////////////////////////////////////////////////

#include "params.h"

#include <math.h>
#include "stdio.h"
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "bsp_volumic.h"
#include "bsp_ortho.h"
#include "edges.h"
#include "3d_api.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_METAL)||defined(API3D_DIRECT3D12)
#if defined(API3D_DIRECT3D12)
#define MAX_VBQUADS 1024
#else
#define MAX_VBQUADS 512
#endif
#else
#if defined(GLESFULL)||defined(WINDOWS_PHONE)
#define MAX_VBQUADS 512
#else
#define MAX_VBQUADS 1024
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define K_t (-SMALLF3)
#define K_t2 (1.0f-SMALLF)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D10

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#define D3DCOLOR_RGBA(r,g,b,a) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(GLES)||defined(GLES20)
#define NB_DATA_PROJECTED_SHADOWS 5000
#else
#define NB_DATA_PROJECTED_SHADOWS 20000
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
	
		shaders for 3d quads
	  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


// COLORTEX
extern CVertexBuffer			effect_hll_diffuse_model;
extern CVertexBuffer			effect_hll_diffuse_alphatest_model;
// COLOR
extern CVertexBuffer			effect_hll_diffuse_nomap;

void MatrixOrthoLHS00(CMatrix *m,float w,float h,float zn,float zf);

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Shadows : plane flat

		void C3DAPI::LightShadowsPlaneFlat(int nLight,CObject3D * obj,float a,float b,float c,float d)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw flat alpha transparency shadow
	  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightShadowsPlaneFlat(int nLight,CObject3D * obj,float a,float b,float c,float d)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P;
	float t,s1,s2;
	int n,nn;
	M.Id();

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
	}


	LoadWorldMatrix(M);
	
	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].Norm*Mrot;

		P=(obj->Faces[n].v[0]->Stok+obj->Faces[n].v[1]->Stok+obj->Faces[n].v[2]->Stok)/3;
		P=P*Mobj;
		P=P - Lights[nLight].Pos;
		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	for (n=0;n<obj->nVertices;n++)
	{
		P=obj->Vertices[n].Stok*Mobj;
		s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;
		s2=a*P.x+b*P.y+c*P.z+d;
		t=-s1/(s2-s1);
		if (t<0)
			obj->Vertices[n].tag=1;
		else
		{
			obj->Vertices[n].tag=0;
			obj->Vertices[n].Calc=Lights[nLight].Pos +  t*(P-Lights[nLight].Pos);
		}
	}


	SetParams(API3D_TEXTURING,OFF);
	SetParams(API3D_BLEND,ON);
	SetParams(API3D_LIGHTING,OFF);


	SHADOW_FLAT.LockVertices();

	for (n=0;n<obj->nVertices;n++)
	{
		SHADOW_FLAT.SetColor(n,0,0,0,0.6f);
		SHADOW_FLAT.SetVertex(n,obj->Vertices[n].Calc.x,-obj->Vertices[n].Calc.y,obj->Vertices[n].Calc.z);
	}
	
	SHADOW_FLAT.UnlockVertices();


	SHADOW_FLAT.LockIndices();
	nn=0;

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==1) 
		{
			if ((obj->Faces[n].v[0]->tag==0)&&(obj->Faces[n].v[1]->tag==0)&&(obj->Faces[n].v[2]->tag==0))
			{

				SHADOW_FLAT.SetIndices(nn,obj->Faces[n].v0,obj->Faces[n].v2,obj->Faces[n].v1);
				nn++;
			}
		}
	}

	SHADOW_FLAT.nVerticesActif=obj->nVertices;
	SHADOW_FLAT.nIndicesActif=nn*3;

    SHADOW_FLAT.UnlockIndices();

	SetVertexBuffer(&SHADOW_FLAT);
	DrawVertexBuffer();

	SetParams(API3D_BLEND,OFF);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : plane mapping

		void C3DAPI::LightShadowsPlaneMapping(int nLight,CObject3D * obj,float a,float b,float c,float d,
												CVector A,CVector B,CVector C,
												float xm,float ym)
		

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture

		(obsolete)
	  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::LightShadowsPlaneMapping(int nLight,CObject3D * obj,float a,float b,float c,float d,
												CVector A,CVector B,CVector C,
												float xm,float ym)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P;
	float t,s1,s2;
	int n,nn;

	CVector u1,u2,u;
	float scalex,scaley;

	u1=B-A;
	scalex=xm/u1.Norme();
	u1.Normalise();
	u2=C-A;
	scaley=ym/u2.Norme();
	u2.Normalise();
	

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
	}


	LoadWorldMatrix(M);
	
	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].Norm*Mrot;

		P=(obj->Faces[n].v[0]->Stok+obj->Faces[n].v[1]->Stok+obj->Faces[n].v[2]->Stok)/3;
		P=P*Mobj;
		P=P - Lights[nLight].Pos;
		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	for (n=0;n<obj->nVertices;n++)
	{
		P=obj->Vertices[n].Stok*Mobj;
		s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;
		s2=a*P.x+b*P.y+c*P.z+d;
		t=-s1/(s2-s1);
		if (t<0)
			obj->Vertices[n].tag=1;
		else
		{
			obj->Vertices[n].tag=0;
			obj->Vertices[n].Calc=Lights[nLight].Pos +  t*(P-Lights[nLight].Pos);

		}
	}
	
	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);

	ShadowVertexBuffer->LockVertices();

	for (n=0;n<obj->nVertices;n++)
	{
		u=obj->Vertices[n].Calc;

		ShadowVertexBuffer->SetVertex(n,u.x,-u.y,u.z);
		ShadowVertexBuffer->SetTexCoo(n,((u-A)||u1)*scalex,((u-A)||u2)*scaley);
		ShadowVertexBuffer->SetColor(n,Ambient.r,Ambient.g,Ambient.b);
	}

	ShadowVertexBuffer->UnlockVertices();

	ShadowVertexBuffer->LockIndices();
	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==1) 
		{
			if ((obj->Faces[n].v[0]->tag==0)&&(obj->Faces[n].v[1]->tag==0)&&(obj->Faces[n].v[2]->tag==0))
			{
				ShadowVertexBuffer->SetIndices(nn,obj->Faces[n].v0,obj->Faces[n].v2,obj->Faces[n].v1);
				nn++;
			}
		}
	}

	ShadowVertexBuffer->nVerticesActif=obj->nVertices;
	ShadowVertexBuffer->nIndicesActif=nn*3;

    ShadowVertexBuffer->UnlockIndices();

	if (states(SHADOW_BUFFERING))
	{
	SetVertexBuffer(ShadowVertexBuffer);
	DrawVertexBuffer();
	}
	
}

#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : object mapping

		void C3DAPI::LightShadowsObjectMapping(int nLight,CObject3D * obj,CObject3D * ref)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture

	  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::LightShadowsObjectMapping(int nLight,CObject3D * obj,CObject3D * ref)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn;
	float xm,ym;
	int np,nf;
	CVector u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d;
	float d1;
	float d2;
	CVector2 um0,um1,um2,um;
	int nbre;

	ShadowVertexBuffer->ListeSE.Free();
	ShadowVertexBuffer->Grouped=false;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;
	}

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (obj->Output&PROPRIETES_MATRICES)
		{
			Mobj=obj->M;
			Mrot=obj->MR;
		}
		else
		{

			Mobj.Id();
			Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
		}
	}

	LoadWorldMatrix(M);
	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Calc=obj->Vertices[n].Stok*Mobj;
		
	}

	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].Norm*Mrot;

		P=(obj->Faces[n].v[0]->Stok+obj->Faces[n].v[1]->Stok+obj->Faces[n].v[2]->Stok)/3;
		P=P*Mobj;
		P=P - Lights[nLight].Pos;
		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	CBSPVolumicOld bsp;

	CVector L,A,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	L=Lights[nLight].Pos;
	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);


	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];

	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);

	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);

	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}
	
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	np=nf=0;

	for (n0=0;n0<ref->nFaces;n0++)
	{
		N=ref->Faces[n0].NormCalc;

		P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
		P=P - Lights[nLight].Pos;

		a=ref->Faces[n0].NormCalc.x;
		b=ref->Faces[n0].NormCalc.y;
		c=ref->Faces[n0].NormCalc.z;
		d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);


		s1=a*L.x+b*L.y+c*L.z+d;

		
		for (n=0;n<obj->nVertices;n++)
		{
			A=obj->Vertices[n].Calc;
			s2=a*A.x+b*A.y+c*A.z+d;

			t=-s1/(s2-s1);

			if (t>1) break;

		}


		if (((N||P)<0)&&(t>1))
		{
			for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;
				
			bsp.BuildVolumeFace(ref,n0,-1,-1,L);
									   // N
										  // cote +-		
			nn=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==1)
				{
					faces[nn].n0=obj->Faces[n].v0;
					faces[nn].n1=obj->Faces[n].v1;
					faces[nn].n2=obj->Faces[n].v2;
					faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
					faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
					faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
					nn++;
				}
			}

			nbre=0;
			if (nn>0)
			{
		
				bsp.ListePlans(bsp.root);
			
				bsp.nVertices=obj->nVertices;
				nbre=bsp.CutFaces(0,faces,nn,-1);
			}

			if (nbre>0)
			{
				nn=0;

				for (n=0;n<nbre;n++)
				{
					if (bsp.ShortFaceInside(bsp.root,&faces[n]))
					{
						faces[n].tag=1;
						faces[n].s0->tag=1;
						faces[n].s1->tag=1;
						faces[n].s2->tag=1;
						nn++;
					}
					else faces[n].tag=0;
				}

				if (nn>0)
				{
				
					v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
					v2=ref->Faces[n0].NormCalc;
					u1=v1^v2;
					u1.Normalise();

					v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
					v2=ref->Faces[n0].NormCalc;
					u2=v1^v2;
					u2.Normalise();

					um0=ref->Faces[n0].v[0]->Map;
					um1=ref->Faces[n0].v[1]->Map;
					um2=ref->Faces[n0].v[2]->Map;

					for (n=0;n<bsp.nVertices;n++)
					{
						s=&(bsp.VerticesS[n]);
						if (s->tag==1)
						{
							P=s->Calc;
							s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;
							s2=a*P.x+b*P.y+c*P.z+d;
							t=-s1/(s2-s1);
							u=Lights[nLight].Pos +  t*(P-Lights[nLight].Pos);

							xm=ym=0;

							d1=-(u1||u);
							d2=-(u2||u);
							s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
							s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;

							t1=-s1/(s2-s1);

							s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
							s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;

							t2=-s1/(s2-s1);

							um=um0+t1*(um1-um0) + t2*(um2-um0);


							xm=um.x;
							ym=um.y;

							u+=0.2f*SMALLF2*ref->Faces[n0].NormCalc;

							ShadowVertexBuffer->SetVertex(np,u.x,u.y,u.z);
							ShadowVertexBuffer->SetTexCoo(np,xm,ym);
							ShadowVertexBuffer->SetColor(np,Ambient.r,Ambient.g,Ambient.b);
							corres[n]=np;
							np++;
							
						}
					}

					for (n=0;n<nbre;n++)
					{
						if (faces[n].tag==1)
						{
							ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
							nf++;
						}
					}								
				}
			}

			bsp.Plans.Free();
			bsp.Free();
		}
	}

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		DrawVertexBuffer();
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);

}

class CTex
{
public:
	CList <int> Faces;
};


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : object mapping

		void C3DAPI::LightShadowsObjectMapping(int nLight,CObject3D * obj,CObject3D * ref)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture with multi texture support


	  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightShadowsObjectMapping_nT(int nLight,CObject3D * obj,CObject3D * ref)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn;
	float xm,ym;
	int np,nf,anf;
	CVector u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d;
	float d1;
	float d2;
	CVector2 um0,um1,um2,um;
	int nbre;
	CMap <unsigned int,CTex> CMtex;
	CTex * tex;
	int *i;
	CIndicesSE nfo;
	CVertexBuffer * vb=ShadowVertexBuffer;

	vb->ListeSE.Free();
	vb->Grouped=false;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;
	}	

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (obj->Output&PROPRIETES_MATRICES)
		{
			Mobj=obj->M;
			Mrot=obj->MR;
		}
		else
		{

			Mobj.Id();
			Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
		}
	}

	LoadWorldMatrix(M);
	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Calc=obj->Vertices[n].Stok*Mobj;
		
	}


	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].Norm*Mrot;

		P=(obj->Faces[n].v[0]->Stok+obj->Faces[n].v[1]->Stok+obj->Faces[n].v[2]->Stok)/3;
		P=P*Mobj;
		P=P - Lights[nLight].Pos;
		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	CBSPVolumicOld bsp;

	CVector L,A,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	L=Lights[nLight].Pos;
	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);

	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];
	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);

	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);	

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		tex=CMtex[ref->Faces[n].nT];
		tex->Faces.Add(n);
	}

	vb->LockVertices();
	vb->LockIndices();

	np=nf=0;

	anf=0;

	tex=CMtex.GetFirst();
	while (tex)
	{
		i=tex->Faces.GetFirst();
		while (i)
		{
			n0=*i;

			N=ref->Faces[n0].NormCalc;

			P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
			P=P - Lights[nLight].Pos;

			a=ref->Faces[n0].NormCalc.x;
			b=ref->Faces[n0].NormCalc.y;
			c=ref->Faces[n0].NormCalc.z;
			d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

			s1=a*L.x+b*L.y+c*L.z+d;
			
			for (n=0;n<obj->nVertices;n++)
			{
				A=obj->Vertices[n].Calc;
				s2=a*A.x+b*A.y+c*A.z+d;

				t=-s1/(s2-s1);

				if (t>1) break;
			}

			if (((N||P)<0)&&(t>1))
			{
				for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;
				
				bsp.BuildVolumeFace(ref,n0,-1,-1,L);
										   // N
											  // cote +-			
				nn=0;
				for (n=0;n<obj->nFaces;n++)
				{
					if (obj->Faces[n].tag==1)
					{
						faces[nn].n0=obj->Faces[n].v0;
						faces[nn].n1=obj->Faces[n].v1;
						faces[nn].n2=obj->Faces[n].v2;
						faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
						faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
						faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
						nn++;
					}
				}

				nbre=0;
				if (nn>0)
				{
			
					bsp.ListePlans(bsp.root);
				
					bsp.nVertices=obj->nVertices;
					nbre=bsp.CutFaces(0,faces,nn,-1);
				}

				if (nbre>0)
				{

					nn=0;

					for (n=0;n<nbre;n++)
					{
						if (bsp.ShortFaceInside(bsp.root,&faces[n]))
						{
							faces[n].tag=1;
							faces[n].s0->tag=1;
							faces[n].s1->tag=1;
							faces[n].s2->tag=1;
							nn++;
						}
						else faces[n].tag=0;
					}

					if (nn>0)
					{
					
						v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u1=v1^v2;
						u1.Normalise();

						v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u2=v1^v2;
						u2.Normalise();

						um0=ref->Faces[n0].v[0]->Map;
						um1=ref->Faces[n0].v[1]->Map;
						um2=ref->Faces[n0].v[2]->Map;

						for (n=0;n<bsp.nVertices;n++)
						{
							s=&(bsp.VerticesS[n]);
							if (s->tag==1)
							{
								P=s->Calc;
								s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;
								s2=a*P.x+b*P.y+c*P.z+d;
								t=-s1/(s2-s1);
								u=Lights[nLight].Pos +  t*(P-Lights[nLight].Pos);

								xm=ym=0;

								d1=-(u1||u);
								d2=-(u2||u);
								s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
								s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;

								t1=-s1/(s2-s1);

								s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
								s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;

								t2=-s1/(s2-s1);

								um=um0+t1*(um1-um0) + t2*(um2-um0);


								xm=um.x;
								ym=um.y;

								//u+=0.2f*SMALLF2*ref->Faces[n0].NormCalc;

								vb->SetVertex(np,u.x,u.y,u.z);
								vb->SetTexCoo(np,xm,ym);
								vb->SetColor(np,Ambient.r,Ambient.g,Ambient.b);
								corres[n]=np;
								np++;
								
							}
						}

						for (n=0;n<nbre;n++)
						{
							if (faces[n].tag==1)
							{
								vb->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
								nf++;
							}
						}				
					}
				}

				bsp.Plans.Free();
				bsp.Free();			
			}
			i=tex->Faces.GetNext();
		}

		if (anf<nf)
		{

		nfo.iStart=anf*3;
		nfo.ref=0;
		nfo.nLV=0;
		nfo.nL=0;
		nfo.nT2=0;
		nfo.nT=CMtex.GetId();
		nfo.iEnd=nf*3;
		vb->ListeSE.Add(nfo);
		}
		anf=nf;

		tex=CMtex.GetNext();
	}

	vb->Grouped=true;

	tex=CMtex.GetFirst();
	while (tex)
	{
		tex->Faces.Free();
		tex=CMtex.GetNext();
	}
	CMtex.Free();

    vb->nVerticesActif=np;
    vb->nIndicesActif=nf*3;

	vb->UnlockVertices();
	vb->UnlockIndices();

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(vb);
		DrawVertexBuffer();
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);	
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : mesh mapping

		
		void C3DAPI::LightShadowsMeshMapping(int nLight,CObject3D * objb,CCollisionMesh * mesh)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture with CCollisionMesh optimization

	Usage:

			CCollisioMesh M;

			M.Init(CObject3D * objet,0,0,int NITER);

			Render.LightShadowsMeshMapping(int nLight,CObject3D * obj_to_project,CCollisionMesh * mesh)
  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightShadowsMeshMapping(int nLight,CObject3D * objb,CCollisionMesh * mesh)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn;
	float xm,ym;
	int np,nf;
	CVector u1,u2,u,v1,v2,g;
	int n0;
	float a,b,c,d;
	float d1;
	CObject3D *obj;
	float d2;
	CVector2 um0,um1,um2,um;
	int nbre;
	float r;
	CObject3D * ref;

	ShadowVertexBuffer->ListeSE.Free();
	ShadowVertexBuffer->Grouped=false;

	ref=mesh->obj;
	mesh->Actualise(0);

	if (objb->Double2)
	{
		obj=objb->Double2;
	}
	else obj=objb;

	if (obj->Radius<0)
		obj->Radius=obj->CalculateRadius();
	if (ref->Radius<0)
		ref->Radius=ref->CalculateRadius();


	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (objb->Output&PROPRIETES_MATRICES)
		{
			Mobj=objb->M;
			Mrot=objb->MR;
		}
		else
		{

			Mobj.Id();
			Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
		}
	}

	ref->Calculate(&Mobj,&Mrot);

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((objb->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=objb->Proprietes.ActualMatrix();
		Mrot=objb->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(objb->Rot.x,objb->Rot.y,objb->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(objb->Coo.x,objb->Coo.y,objb->Coo.z);
	}


	LoadWorldMatrix(M);
	obj->Calculate(&Mobj,&Mrot);

	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].NormCalc;

		P=(obj->Faces[n].v[0]->Calc+obj->Faces[n].v[1]->Calc+obj->Faces[n].v[2]->Calc)/3;
		P=P - Lights[nLight].Pos;
		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	CBSPVolumicOld bsp;

	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	CList <int> *list=NULL;
	float f1,f2;
	int *pi;

	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);

	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);

	CVector A,L,frustum[5];

	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1.Base(u,0);
	u2.Base(u,1);

	r=10;

	d=1;

	frustum[0]=A+obj->Radius*u;
	u=A+obj->Radius*(u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[1]=A+r*u;

	u=A+obj->Radius*(u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[2]=A+r*u;

	u=A+obj->Radius*(-1*u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[3]=A+r*u;

	u=A+obj->Radius*(-1*u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[4]=A+r*u;

	list=mesh->quad.FrustumGetFaceList(frustum,mesh->M);


	np=nf=0;
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();	

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}	

	if (list->Length()>0)
	{
		pi=list->GetFirst();
		while (pi)
		{
			n0=*pi;

			pi=list->GetNext();
			N=ref->Faces[n0].NormCalc;

			P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
			P=P - Lights[nLight].Pos;

			a=ref->Faces[n0].NormCalc.x;
			b=ref->Faces[n0].NormCalc.y;
			c=ref->Faces[n0].NormCalc.z;
			d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

			s1=a*L.x+b*L.y+c*L.z+d;

			for (n=0;n<obj->nVertices;n++)
			{
				A=obj->Vertices[n].Calc;
				s2=a*A.x+b*A.y+c*A.z+d;

				t=-s1/(s2-s1);

				if (t>1) break;
			}

			if (((N||P)<0)&&(t>1))
			{

				for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;
				bsp.BuildVolumeFace(ref,n0,-1,-1,Lights[nLight].Pos);
										   // N
											  // cote +-
				nn=0;
				for (n=0;n<obj->nFaces;n++)
				{
					if (obj->Faces[n].tag==1)
					{

						faces[nn].n0=obj->Faces[n].v0;
						faces[nn].n1=obj->Faces[n].v1;
						faces[nn].n2=obj->Faces[n].v2;
						faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
						faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
						faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
						nn++;
					}
				}

				nbre=0;
				if (nn>0)
				{
			
					bsp.ListePlans(bsp.root);
				
					bsp.nVertices=obj->nVertices;
					nbre=bsp.CutFaces(0,faces,nn,-1);
				}

				if (nbre>0)
				{

					nn=0;
					
					for (n=0;n<nbre;n++)
					{
						if (bsp.ShortFaceInside(bsp.root,&faces[n]))
						{
							faces[n].tag=1;
							faces[n].s0->tag=1;
							faces[n].s1->tag=1;
							faces[n].s2->tag=1;
							nn++;
						}
						else faces[n].tag=0;
					}

					if (nn>0)
					{
						v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u1=v1^v2;
						u1.Normalise();

						v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u2=v1^v2;
						u2.Normalise();

						um0=ref->Faces[n0].v[0]->Map;
						um1=ref->Faces[n0].v[1]->Map;
						um2=ref->Faces[n0].v[2]->Map;
						
						v1.Base(ref->Faces[n0].NormCalc,0);
						v2.Base(ref->Faces[n0].NormCalc,1);

						g.Init(0,0,0);
						nn=0;

						s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;

						for (n=0;n<bsp.nVertices;n++)
						{
							s=&(bsp.VerticesS[n]);
							if (s->tag==1)
							{
								P=s->Calc;
								
								s2=a*P.x+b*P.y+c*P.z+d;
								t=-s1/(s2-s1);
								u=Lights[nLight].Pos +  t*(P-Lights[nLight].Pos);
								nn++;
								g+=u;
								s->Calc=u;
							}
						}
						
						g=g/(float) nn;

						for (n=0;n<bsp.nVertices;n++)
						{
							s=&(bsp.VerticesS[n]);
							if (s->tag==1)
							{
								u=s->Calc;

								u=u-g;
								f1=(u||v1);
								f2=(u||v2);
								
								if (f1>=0) f1+=0.5f*SMALLF2;
								else f1-=0.5f*SMALLF2;
								if (f2>=0) f2+=0.5f*SMALLF2;
								else f2-=0.5f*SMALLF2;
								
								u=g+f1*v1 + f2*v2;
						
								xm=ym=0;

								d1=-(u1||u);
								d2=-(u2||u);
								s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
								s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;

								t1=-s1/(s2-s1);

								s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
								s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;

								t2=-s1/(s2-s1);

								um=um0+t1*(um1-um0) + t2*(um2-um0);

								xm=um.x;
								ym=um.y;

								u+=0.2f*SMALLF2*ref->Faces[n0].NormCalc;

								ShadowVertexBuffer->SetVertex(np,u.x,u.y,u.z);
								ShadowVertexBuffer->SetTexCoo(np,xm,ym);
								ShadowVertexBuffer->SetColor(np,Ambient.r,Ambient.g,Ambient.b);
								corres[n]=np;
								np++;
								
							}
						}
						
						for (n=0;n<nbre;n++)
						{
							if (faces[n].tag==1)
							{
								ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
								nf++;
							}
						}				
					}
				}

				bsp.Plans.Free();
				bsp.Free();
			}
		}
	}

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{

		SetParams(API3D_ZBIAS,1);
		SetVertexBuffer(ShadowVertexBuffer);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}

	delete [] bsp.VerticesS;
	delete [] faces;
	free(corres);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CnTGroup
{
public:
	int nT;
	int nL;
	CList <CFaceGROUP*> ntgroup;

	CnTGroup() {}
	~CnTGroup() {ntgroup.Free();}

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : mesh mapping

		
		void C3DAPI::LightShadowsMeshMapping(int nLight,CObject3D * objb,CCollisionMesh * mesh)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture with CCollisionMesh optimization
		multitexturing support

		! finalised routine !

	Usage:

			CCollisioMesh M;

			M.Init2(CObject3D * objet,0,0,int NITER);

			Render.LightShadowsMeshMapping2(int nLight,CObject3D * obj_to_project,CCollisionMesh * mesh)
			void C3DAPI::LightShadowsMeshMapping2b(int nLight,CObject3D * objb,CCollisionMesh * mesh)
  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightShadowsMeshMapping2(int nLight,CObject3D * objb,CCollisionMesh * mesh)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M,Mp,invMp;
	CVector4 v4;
	CPlane plan;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn,bob;
	unsigned int id;
	float xm,ym;
	int np,nf,anf;
	CVector u1,u2,u,v1,v2,g;
	int n0;
	float a,b,c,d;
	float d1;
	CObject3D *obj;
	float d2;
	CVector2 um0,um1,um2,um;
	int nbre;
	float r;
	CObject3D * ref;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	CIndicesSE nfo;
	CBSPVolumicOld bsp;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;
	CVector A,L,frustum[5];
	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;

	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);
	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	ShadowVertexBuffer->ListeSE.Free();
	ShadowVertexBuffer->Grouped=false;


	ref=mesh->obj;
	mesh->Actualise(0);

	
	if (objb->Double2)
	{
		obj=objb->Double2;
	}
	else obj=objb;

	if (obj->Radius<0)
		obj->Radius=obj->CalculateRadius();

	if (ref->Radius<0)
		ref->Radius=ref->CalculateRadius();
	
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	//ref->Calculate(&Mobj,&Mrot);

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((objb->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=objb->Proprietes.ActualMatrix();
		Mrot=objb->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (objb->Output&PROPRIETES_MATRICES)
		{
			Mobj=objb->M;
			Mrot=objb->MR;
		}
		else
		{
			Mobj.Id();
			Mobj.RotationDegre(objb->Rot.x,objb->Rot.y,objb->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(objb->Coo.x,objb->Coo.y,objb->Coo.z);
		}
	}

	M.Id();
	LoadWorldMatrix(M);
	obj->Calculate(&Mobj,&Mrot);


	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].NormCalc;

		P=(obj->Faces[n].v[0]->Calc+obj->Faces[n].v[1]->Calc+obj->Faces[n].v[2]->Calc)/3;
		P=P - Lights[nLight].Pos;

		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;
	}

	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);

	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1.Base(u,0);
	u2.Base(u,1);

	r=7;
	d=2;

	frustum[0]=A+obj->Radius*u;
	u=A+obj->Radius*(u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[1]=A+r*u;

	u=A+obj->Radius*(u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[2]=A+r*u;

	u=A+obj->Radius*(-1*u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[3]=A+r*u;

	u=A+obj->Radius*(-1*u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[4]=A+r*u;

	list=mesh->quad.FrustumGetGroupList(frustum,mesh->M);

	np=nf=0;
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}

	anf=0;
	if (list->Length()>0)
	{		
		pg=list->GetFirst();
		while (pg)
		{
			id=((*pg)->nT)+(((*pg)->nL)<<16);
			ntg=CM[id];
			ntg->nT=(*pg)->nT;
			ntg->nL=(*pg)->nL;
			ntg->ntgroup.Add(*pg);
			pg=list->GetNext();
		}

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{

				n0=*((*pg)->faces->GetFirst());
				
				
				N=ref->Faces[n0].NormCalc;

				P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
				P=P - Lights[nLight].Pos;

				a=ref->Faces[n0].NormCalc.x;
				b=ref->Faces[n0].NormCalc.y;
				c=ref->Faces[n0].NormCalc.z;
				d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

				plan.Init(a,b,c,d);
				plan.Normalise();

				v4.Init(Lights[nLight].Pos);
				Mp.ShadowProjection(v4,plan);


				s1=a*L.x+b*L.y+c*L.z+d;

				bob=0;

				if ((N||P)<0)
				for (n=0;n<obj->nVertices;n++)
				{
					A=obj->Vertices[n].Calc;
					s2=a*A.x+b*A.y+c*A.z+d;

					t=-s1/(s2-s1);

					if (t>K_t2) 
					{
						bob=1;
						obj->Vertices[n].tag2=1;
					}
					else obj->Vertices[n].tag2=0;

				}

				if (((N||P)<0)&&(bob))
				{

					for (n=0;n<obj->nFaces;n++)
					{
						if (obj->Faces[n].tag==1)
						{
							bob=(obj->Faces[n].v[0]->tag2==1)+(obj->Faces[n].v[1]->tag2==1)+(obj->Faces[n].v[2]->tag2==1);
							if (bob>0) obj->Faces[n].tag2=1;
							else obj->Faces[n].tag2=0;
						}
					}

					for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;
					
					bsp.BuildGroupVolume(ref,*pg,-1,1,Lights[nLight].Pos);
				
					nn=0;
					for (n=0;n<obj->nFaces;n++)
					{
						if ((obj->Faces[n].tag==1)&&(obj->Faces[n].tag2==1))
						{
							faces[nn].n0=obj->Faces[n].v0;
							faces[nn].n1=obj->Faces[n].v1;
							faces[nn].n2=obj->Faces[n].v2;
							faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
							faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
							faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
							nn++;
						}
					}

					nbre=0;
					if (nn>0)
					{
						bsp.ListePlans(bsp.root);
					
						bsp.nVertices=obj->nVertices;
						nbre=bsp.CutFaces(0,faces,nn,-1);  
					}

					if (nbre>0)
					{

						nn=0;
						
						for (n=0;n<nbre;n++)
						{
							if (bsp.ShortFaceInside(bsp.root,&faces[n]))
							{
								faces[n].tag=1;
								faces[n].s0->tag=1;
								faces[n].s1->tag=1;
								faces[n].s2->tag=1;
								nn++;
							}
							else faces[n].tag=0;
						}

						if (nn>0)
						{
							v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
							v2=ref->Faces[n0].NormCalc;
							u1=v1^v2;
							u1.Normalise();

							v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
							v2=ref->Faces[n0].NormCalc;
							u2=v1^v2;
							u2.Normalise();

							um0=ref->Faces[n0].v[0]->Map;
							um1=ref->Faces[n0].v[1]->Map;
							um2=ref->Faces[n0].v[2]->Map;
							
							v1.Base(ref->Faces[n0].NormCalc,0);
							v2.Base(ref->Faces[n0].NormCalc,1);

							g.Init(0,0,0);
							nn=0;

							s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;

							CVector pu1,pu2;

							for (n=0;n<bsp.nVertices;n++)
							{
								s=&(bsp.VerticesS[n]);
								if (s->tag==1)
								{
									v4.Init(s->Calc);
									v4=v4*Mp;
									u.Init(v4.x,v4.y,v4.z);
									u/=v4.w;
									
									pu1=u-Lights[nLight].Pos;
									pu2=s->Calc-Lights[nLight].Pos;

									s->coef1=pu1.Norme()-pu2.Norme();

									nn++;
									g+=u;
									s->Calc=u;
								}
							}
							

							if (nn>0)
							{

							for (n=0;n<bsp.nVertices;n++)
							{
								s=&(bsp.VerticesS[n]);
								if (s->tag==1)
								{
									u=s->Calc;
				
									xm=ym=0;

									d1=-(u1||u);
									d2=-(u2||u);
									s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
									s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;

									t1=-s1/(s2-s1);

									s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
									s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;

									t2=-s1/(s2-s1);

									um=um0+t1*(um1-um0) + t2*(um2-um0);

									xm=um.x;
									ym=um.y;

									//u+=0.2f*SMALLF2*ref->Faces[n0].NormCalc;
									ShadowVertexBuffer->SetVertex(np,u.x,u.y,u.z);
									ShadowVertexBuffer->SetTexCoo(np,xm,ym);
									ShadowVertexBuffer->SetColor(np,Ambient.r,Ambient.g,Ambient.b);
									corres[n]=np;
									np++;								
								}
							}
								
							for (n=0;n<nbre;n++)
							{
								if (faces[n].tag==1)
								{
									if ((faces[n].s0->coef1>K_t)+(faces[n].s1->coef1>K_t)+(faces[n].s2->coef1>K_t)==3)
									{
										ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
										nf++;
									}
								}
							}				

							}						
						}
					}

					bsp.Plans.Free();
					bsp.Free();
				}

				if (anf<nf)
				{
					nfo.vEnd=-1;
					nfo.vStart=-1;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT2=0;
					id=CM.GetId();
					nfo.nT=id&0xFFFF;	
					nfo.nL=(id>>16)&0xFFFF;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
				}
				anf=nf;

				pg=ntg->ntgroup.GetNext();
				
			}

			ntg=CM.GetNext();
		}
    }

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	ShadowVertexBuffer->Grouped=true;

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{

		SetParams(API3D_ZBIAS,4);
		SetVertexBuffer(ShadowVertexBuffer);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}

	CM.Free();

	delete [] bsp.VerticesS;
	delete [] faces;
	free(corres);
}


void C3DAPI::LightProjectTextureMesh(int nLight,CVector Pos,float sizex,float sizey,CCollisionMesh *mesh,int DEF,int reset)
{
    if (!Active) return;
    
	float radius=((float) sqrtf(sizex*sizex+sizey*sizey));
	CIndicesSE nfo;
	CMatrix Mobj,Mrot,M;
	CVector N,P,Nf;
	int n,nn;
	int np,nf,anf,anp;
	CVector U1,U2,u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d,r,s1,s2,t;
	CVector2 um0,um1,um2,um;
	int nbre;
	CPlane plan;
	CMatrix Mproj;
	CVector4 v4;
	CVector A,B,C,D;
	CVector2 Am,Bm,Cm,Dm;
	int n1,n2;
	int a0,a1,a2,a3;
	CBSPVolumicOld bsp;
	CObject3D *ref=mesh->obj;
	CVector frustum[5];
	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;
	CVector L,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	unsigned int id;
	CVector pu1,pu2;
	int * corres;

	mesh->Actualise(0);

	u=Pos-Lights[nLight].Pos;
	u.Normalise();

	N=u;

	U1.Base(u,0); U2.Base(u,1);
	U1*=sizex; U2*=sizey;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		VECTOR3MATRIX(ref->Vertices[n].Calc,ref->Vertices[n].Stok,Mobj);
		VECTOR3MATRIX(ref->Vertices[n].NormCalc,ref->Vertices[n].Norm,Mrot);
	}

	for (n=0;n<ref->nFaces;n++)
	{
		VECTOR3MATRIX(ref->Faces[n].NormCalc,ref->Faces[n].Norm,Mrot);
	}

	L=Lights[nLight].Pos;
	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);
	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	A=Pos;

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1=U1; u2=U2;
	radius=r=sqrtf(sizex*sizex+sizey*sizey);
	u1.Normalise(); u2.Normalise();

	if (reset&PROJECT_BASE_GROUND)
	{		
		u=A-L;
		u.Normalise();

		frustum[0]=L;

		if (f_abs(N.y)>0.9f)
		{			
			P=A+r*(-1*u1+u2);
			frustum[1]=P+radius*u;

			P=A+r*(-1*u1-u2);
			frustum[2]=P+radius*u;

			P=A+r*(u1-u2);
			frustum[3]=P+radius*u;

			P=A+r*(u1+u2);
			frustum[4]=P+radius*u;

		}
		else
		{
			P=A+r*(0*u1+u2);
			frustum[1]=P+radius*u;

			P=A+r*(0*u1-u2);
			frustum[2]=P+radius*u;

			P=A+r*(2*u1-u2);
			frustum[3]=P+radius*u;

			P=A+r*(2*u1+u2);
			frustum[4]=P+radius*u;
		}
		list=mesh->quad.FrustumGetGroupList(frustum,mesh->M);
	}
	else /**/list=mesh->quad.GetGroupList();

	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	if (reset&COMMON_RESET)
	{
		anp=anf=np=nf=0;
		ShadowVertexBuffer->ListeSE.Free();
		ShadowVertexBuffer->Grouped=true;
	}
	else
	{
		anp=np=ShadowVertexBuffer->nVerticesActif;
		anf=nf=ShadowVertexBuffer->nIndicesActif/3;
		ShadowVertexBuffer->Grouped=true;
	}

	if (list->Length()>0)
	{	
		pg=list->GetFirst();
		while (pg)
		{
			id=(*pg)->nT;
			ntg=CM[id];
			ntg->nT=(*pg)->nT;
			ntg->ntgroup.Add((*pg));
			pg=list->GetNext();
		}

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{
				n0=*((*pg)->faces->GetFirst());

				Nf=ref->Faces[n0].NormCalc;

				a=ref->Faces[n0].NormCalc.x;
				b=ref->Faces[n0].NormCalc.y;
				c=ref->Faces[n0].NormCalc.z;
				d=-DOT(ref->Faces[n0].NormCalc,ref->Faces[n0].v[0]->Calc);

				VECTORADDDIV3(P,ref->Faces[n0].v[0]->Calc,ref->Faces[n0].v[1]->Calc,ref->Faces[n0].v[2]->Calc);
				VECTORSUB(P,P,Lights[nLight].Pos);
				VECTORNORMALISE(P);

				if ((DOT(N,Nf)<0.0f)&&(DOT(Nf,P)<0.0f))
				{
					bsp.BuildGroupVolume(ref,*pg,-1,-1,Lights[nLight].Pos);

					if (reset&PROJECT_BASE_GROUND)
					{
						if (f_abs(N.y)>0.9f)
						{
							A=Pos-U1-U2;
							Am.Init(0,0);
							B=Pos+U1-U2;
							Bm.Init(1,0);
							C=Pos+U1+U2;
							Cm.Init(1,1);
							D=Pos-U1+U2;
							Dm.Init(0,1);
						}
						else
						{
							A=Pos-0*U1-U2;
							Am.Init(0,0);
							B=Pos+2*U1-U2;
							Bm.Init(1,0);
							C=Pos+2*U1+U2;
							Cm.Init(1,1);
							D=Pos-0*U1+U2;
							Dm.Init(0,1);
						}
					}
					else
					{
						A=Pos-U1-U2;
						Am.Init(0,0);
						B=Pos+U1-U2;
						Bm.Init(1,0);
						C=Pos+U1+U2;
						Cm.Init(1,1);
						D=Pos-U1+U2;
						Dm.Init(0,1);
					}

					s1=a*L.x+b*L.y+c*L.z+d;
					int bob=0;

					for (n1=0;n1<=DEF;n1++)
						for (n2=0;n2<=DEF;n2++)
						{
							n=n1+n2*(DEF+1);
							u=A+((float)n1)*(B-A)/((float)DEF)+((float)n2)*(D-A)/((float)DEF);
							um0=Am+((float)n1)*(Bm-Am)/((float)DEF)+((float)n2)*(Dm-Am)/((float)DEF);
							bsp.VerticesS[n].Stok=u;
							bsp.VerticesS[n].Calc=u;
							bsp.VerticesS[n].Map=um0;
							s2=a*u.x+b*u.y+c*u.z+d;
							t=-s1/(s2-s1);
							if (t>0.9f) 
							{
								bob++;
								bsp.VerticesS[n].tag2=1;
							}
							else bsp.VerticesS[n].tag2=0;
						}

					bsp.nVertices=(DEF+1)*(DEF+1);

					if (bob==bsp.nVertices)
					{
						nn=0;
						for (n1=0;n1<DEF;n1++)
							for (n2=0;n2<DEF;n2++)
							{
								a0=n1+n2*(DEF+1);
								a1=n1+1+n2*(DEF+1);
								a2=n1+1+(n2+1)*(DEF+1);
								a3=n1+(n2+1)*(DEF+1);
								faces[nn].tag=0;
								faces[nn].n0=a0;
								faces[nn].n1=a2;
								faces[nn].n2=a1;
								faces[nn].s0=&(bsp.VerticesS[a0]);
								faces[nn].s1=&(bsp.VerticesS[a2]);
								faces[nn].s2=&(bsp.VerticesS[a1]);
								if (((faces[nn].s0->tag2==1)+(faces[nn].s1->tag2==1)+(faces[nn].s2->tag2==1))>2) nn++;
							
								faces[nn].tag=0;
								faces[nn].n0=a0;
								faces[nn].n1=a3;
								faces[nn].n2=a2;
								faces[nn].s0=&(bsp.VerticesS[a0]);
								faces[nn].s1=&(bsp.VerticesS[a3]);
								faces[nn].s2=&(bsp.VerticesS[a2]);
								if (((faces[nn].s0->tag2==1)+(faces[nn].s1->tag2==1)+(faces[nn].s2->tag2==1))>2) nn++;								
							}

						nbre=0;										
						if (nn>0)
						{
							bsp.faces=faces;
							bsp.nfaces=nn;
							bsp.InOut(0,0,bsp.root,-1);
							nbre=bsp.nfaces;
						}

						if (nbre>0)
						{
							nn=0;
							for (n=0;n<nbre;n++)
							{
								if (faces[n].tag==-1)
								{
									faces[n].tag=1;
									faces[n].s0->tag=1;
									faces[n].s1->tag=1;
									faces[n].s2->tag=1;
									nn++;
								}
								else faces[n].tag=0;
							}

							if (nn>0)
							{
								float t1,t2,d1,d2,s1,s2,xm,ym;

								plan.Init(a,b,c,d);
								v4.Init(L);
								Mproj.ShadowProjection(v4,plan);

								if (reset&COMMON_MULTITEXTURE)
								{
									VECTORSUB(v1,ref->Faces[n0].v[1]->Calc,ref->Faces[n0].v[0]->Calc);
									CROSSPRODUCT(u1,v1,ref->Faces[n0].NormCalc);
									VECTORNORMALISE(u1);
									VECTORSUB(v1,ref->Faces[n0].v[2]->Calc,ref->Faces[n0].v[0]->Calc);
									CROSSPRODUCT(u2,v1,ref->Faces[n0].NormCalc);
									VECTORNORMALISE(u2);

									um0.x=ref->Faces[n0].v[0]->Map.x;
									um1.x=ref->Faces[n0].v[1]->Map.x;
									um2.x=ref->Faces[n0].v[2]->Map.x;

									um0.y=ref->Faces[n0].v[0]->Map.y;
									um1.y=ref->Faces[n0].v[1]->Map.y;
									um2.y=ref->Faces[n0].v[2]->Map.y;
									
									v1.Base(ref->Faces[n0].NormCalc,0);
									v2.Base(ref->Faces[n0].NormCalc,1);

									for (n=0;n<bsp.nVertices;n++)
									{
										s=&(bsp.VerticesS[n]);
										if (s->tag==1)
										{				
											v4.x=s->Stok.x;
											v4.y=s->Stok.y;
											v4.z=s->Stok.z;
											v4.w=1.0f;

											VECTOR4MATRIX(v4,v4,Mproj);
											u.x=v4.x;
											u.y=v4.y;
											u.z=v4.z;

											VECTORSUB(pu1,u,Lights[nLight].Pos);
											VECTORSUB(pu2,s->Stok,Lights[nLight].Pos);
											s->coef1=VECTORNORM(pu1) - VECTORNORM(pu2);

											xm=ym=0;
											d1=-DOT(u1,u);
											d2=-DOT(u2,u);
											s1=DOT(ref->Faces[n0].v[0]->Calc,u2) + d2;
											s2=DOT(ref->Faces[n0].v[1]->Calc,u2) + d2;
											t1=-s1/(s2-s1);
											s1=DOT(ref->Faces[n0].v[0]->Calc,u1) + d1;
											s2=DOT(ref->Faces[n0].v[2]->Calc,u1) + d1;
											t2=-s1/(s2-s1);
											um.x=um0.x + t1*(um1.x-um0.x) + t2*(um2.x-um0.x);
											um.y=um0.y + t1*(um1.y-um0.y) + t2*(um2.y-um0.y);
											xm=um.x;
											ym=um.y;
											if (np<(int)ShadowVertexBuffer->nVertices)
											{
												ShadowVertexBuffer->SetVertex(np,u);
												ShadowVertexBuffer->SetTexCoo(np,xm,ym);
												ShadowVertexBuffer->SetTexCoo2(np,s->Map);
												//ShadowVertexBuffer->SetColor(np,1,1,1);
												corres[n]=np;
												np++;
											}
										}
									}

								}
								else
								{
									for (n=0;n<bsp.nVertices;n++)
									{
										s=&(bsp.VerticesS[n]);
										if (s->tag==1)
										{
											v4.Init(s->Stok);
											VECTOR4MATRIX(v4,v4,Mproj);
											u.Init(v4.x,v4.y,v4.z);
											if (np<(int)ShadowVertexBuffer->nVertices)
											{
												ShadowVertexBuffer->SetVertex(np,u);
												ShadowVertexBuffer->SetTexCoo(np,s->Map);
												ShadowVertexBuffer->SetColor(np,1,1,1);
												corres[n]=np;
												np++;
											}
										}
									}
								}

								for (n=0;n<nbre;n++)
								{
									if (faces[n].tag==1)
									{
										if (nf<(int)ShadowVertexBuffer->nIndices/3)
										{
											if ((faces[n].s0->coef1>-r)+(faces[n].s1->coef1>-r)+(faces[n].s2->coef1>-r)==3)
											{
												ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
												nf++;
											}
										}
									}
								}		
							}
						}
					}
					bsp.Plans.Free();
					bsp.Free();
				}
				pg=ntg->ntgroup.GetNext();				
			}

			if (reset&COMMON_MULTITEXTURE)
			{
				if (nf>anf)
				{
					nfo.vEnd=np;
					nfo.vStart=anp;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT=ntg->nT;
					nfo.nT2=aT;
					nfo.nL=-1;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
					anf=nf;
					anp=np;
				}
			}


			ntg=CM.GetNext();
		}
	}

	LoadWorldMatrix(M);

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();
	
	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		SetParams(API3D_ZBIAS,4);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}
	else
	{
		if ((reset&COMMON_MULTITEXTURE)==0)
		{
			if (nf>0)
			{
				nfo.vEnd=np;
				nfo.vStart=anp;
				nfo.iStart=anf*3;
				nfo.ref=1;
				nfo.nLV=0;
				nfo.nT2=0;
				nfo.nT=aT;					
				nfo.nL=-1;
				nfo.iEnd=nf*3;
				ShadowVertexBuffer->ListeSE.Add(nfo);
			}
		}
	}

	delete [] bsp.VerticesS;
	delete [] faces;
	free(corres);
}



void C3DAPI::LightShadowsMeshMapping2b(int nLight,CObject3D * objb,CCollisionMesh * mesh)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M,Mp,invMp;
	CVector4 v4;
	CPlane plan;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn,bob;
	unsigned int id;
	float xm,ym;
	int np,nf,anf;
	CVector u1,u2,u,v1,v2,g;
	int n0;
	float a,b,c,d;
	float d1;
	CObject3D *obj;
	float d2;
	CVector2 um0,um1,um2,um;
	int nbre;
	float r;
	CObject3D * ref;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	CIndicesSE nfo;
	CBSPVolumicOld bsp;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;
	CVector A,L,frustum[5];
	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;

	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];
	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);

	ShadowVertexBuffer->ListeSE.Free();
	ShadowVertexBuffer->Grouped=false;

	ref=mesh->obj;
	//mesh->Actualise(0);
	
	if (objb->Double2)
	{
		obj=objb->Double2;
	}
	else obj=objb;

	if (obj->Radius<0)
		obj->Radius=obj->CalculateRadius();

	if (ref->Radius<0)
		ref->Radius=ref->CalculateRadius();
	
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	//ref->Calculate(&Mobj,&Mrot);

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((objb->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=objb->Proprietes.ActualMatrix();
		Mrot=objb->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (objb->Output&PROPRIETES_MATRICES)
		{
			Mobj=objb->M;
			Mrot=objb->MR;
		}
		else
		{
			Mobj.Id();
			Mobj.RotationDegre(objb->Rot.x,objb->Rot.y,objb->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(objb->Coo.x,objb->Coo.y,objb->Coo.z);
		}
	}

	M.Id();
	LoadWorldMatrix(M);
	obj->Calculate(&Mobj,&Mrot);

	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].NormCalc;
		VECTORADDDIV3(P,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[2]->Calc);
		VECTORSUB(P,P,Lights[nLight].Pos);
		if (DOT(N,P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;

		obj->Faces[n].lock=0;
	}

	SetParams(API3D_BLEND,OFF);
	SetParams(API3D_LIGHTING,OFF);

	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);

	L=Lights[nLight].Pos;
	VECTORSUB(u,L,A);
	VECTORNORMALISE(u);

	u1.Base(u,0);
	u2.Base(u,1);

	r=13;

	d=1.0f;

	frustum[0]=A+0.5f*obj->Radius*u;
	u=A+obj->Radius*(u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[1]=A+r*u;

	u=A+obj->Radius*(u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[2]=A+r*u;

	u=A+obj->Radius*(-1*u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[3]=A+r*u;

	u=A+obj->Radius*(-1*u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[4]=A+r*u;

	list=mesh->quad.FrustumGetGroupList(frustum,mesh->M);		

	np=nf=0;
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}

	anf=0;
	if (list->Length()>0)
	{		
		pg=list->GetFirst();
		while (pg)
		{
			id=((*pg)->nT)+(((*pg)->nL)<<16);
			ntg=CM[id];
			ntg->nT=(*pg)->nT;
			ntg->nL=(*pg)->nL;
			ntg->ntgroup.Add(*pg);
			pg=list->GetNext();
		}

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{
				n0=*((*pg)->faces->GetFirst());
					
				N=ref->Faces[n0].NormCalc;

				VECTORADDDIV3(P,ref->Faces[n0].v[0]->Calc,ref->Faces[n0].v[1]->Calc,ref->Faces[n0].v[2]->Calc);
				VECTORSUB(P,P,Lights[nLight].Pos);

				a=ref->Faces[n0].NormCalc.x;
				b=ref->Faces[n0].NormCalc.y;
				c=ref->Faces[n0].NormCalc.z;
				d=-DOT(ref->Faces[n0].NormCalc,ref->Faces[n0].v[0]->Calc);

				plan.Init(a,b,c,d);
				plan.Normalise();

				v4.Init(Lights[nLight].Pos);
				Mp.ShadowProjection(v4,plan);

				s1=a*L.x+b*L.y+c*L.z+d;

				bob=0;

				if (DOT(N,P)<0)
				for (n=0;n<obj->nVertices;n++)
				{
					A=obj->Vertices[n].Calc;
					s2=a*A.x+b*A.y+c*A.z+d;

					t=-s1/(s2-s1);

					if (t>K_t2) 
					{
						bob=1;
						obj->Vertices[n].tag2=1;
					}
					else
					{
						obj->Vertices[n].tag2=0;
						
					}
				}
				
				if ((DOT(N,P)<0)&&(bob))
				{
					for (n=0;n<obj->nFaces;n++)
					{
						if (obj->Faces[n].tag==1)
						{
							bob=(obj->Faces[n].v[0]->tag2==1)+(obj->Faces[n].v[1]->tag2==1)+(obj->Faces[n].v[2]->tag2==1);
							if (bob>0) obj->Faces[n].tag2=1;
							else obj->Faces[n].tag2=0;
						}
					}

					for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;
					
					bsp.BuildGroupVolume(ref,*pg,-1,-1,Lights[nLight].Pos);

					nn=0;
					for (n=0;n<obj->nFaces;n++)
					{
						if ((obj->Faces[n].tag==1)&&(obj->Faces[n].tag2==1)&&(obj->Faces[n].lock==0))
						{
							faces[nn].n0=obj->Faces[n].v0;
							faces[nn].n1=obj->Faces[n].v1;
							faces[nn].n2=obj->Faces[n].v2;
							faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
							faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
							faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
							faces[nn].tag=0;
							faces[nn].tag2=n;
							faces[nn].lock=0;

							nn++;
						}
					}

					nbre=0;										
					if (nn>0)
					{
						bsp.nVertices=obj->nVertices;
						bsp.faces=faces;
						bsp.nfaces=nn;
						bsp.InOutNoMap(0,0,bsp.root,-1);
						nbre=bsp.nfaces;
					}

					if (nbre>0)
					{
						nn=0;	
						for (n=0;n<nbre;n++)
						{	
							if (faces[n].tag==-1)
							{
								if (faces[n].lock==0) { obj->Faces[faces[n].tag2].lock=1; }
								faces[n].tag=1;
								faces[n].s0->tag=1;
								faces[n].s1->tag=1;
								faces[n].s2->tag=1;
								nn++;

							}
							else faces[n].tag=0;
/*
							if (bsp.ShortFaceInside(bsp.root,&faces[n]))
							{
								faces[n].tag=1;
								faces[n].s0->tag=1;
								faces[n].s1->tag=1;
								faces[n].s2->tag=1;
								nn++;
							}
							else faces[n].tag=0;
/**/
						}

						if (nn>0)
						{
							VECTORSUB(v1,ref->Faces[n0].v[1]->Calc,ref->Faces[n0].v[0]->Calc);
							v2=ref->Faces[n0].NormCalc;
							CROSSPRODUCT(u1,v1,v2);
							VECTORNORMALISE(u1);

							VECTORSUB(v1,ref->Faces[n0].v[2]->Calc,ref->Faces[n0].v[0]->Calc);
							v2=ref->Faces[n0].NormCalc;
							CROSSPRODUCT(u2,v1,v2);
							VECTORNORMALISE(u2);

							um0=ref->Faces[n0].v[0]->Map;
							um1=ref->Faces[n0].v[1]->Map;
							um2=ref->Faces[n0].v[2]->Map;
							
							v1.Base(ref->Faces[n0].NormCalc,0);
							v2.Base(ref->Faces[n0].NormCalc,1);

							nn=0;
							s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;

							CVector pu1,pu2;

							for (n=0;n<bsp.nVertices;n++)
							{
								s=&(bsp.VerticesS[n]);
								if (s->tag==1)
								{
									v4.Init(s->Calc);
									VECTOR4MATRIX(v4,v4,Mp);
									VECTORINIT(u,v4.x,v4.y,v4.z);
									VECTORSUB(pu1,u,Lights[nLight].Pos);
									VECTORSUB(pu2,s->Calc,Lights[nLight].Pos);
									s->coef1=VECTORNORM(pu1) - VECTORNORM(pu2);
									nn++;
									s->Calc=u;
								}
							}
							
							if (nn>0)
							{
								for (n=0;n<bsp.nVertices;n++)
								{
									s=&(bsp.VerticesS[n]);
									if (s->tag==1)
									{
										u=s->Calc;
										xm=ym=0;
										d1=-DOT(u1,u);
										d2=-DOT(u2,u);
										s1=DOT(ref->Faces[n0].v[0]->Calc,u2) + d2;
										s2=DOT(ref->Faces[n0].v[1]->Calc,u2) + d2;
										t1=-s1/(s2-s1);
										s1=DOT(ref->Faces[n0].v[0]->Calc,u1) + d1;
										s2=DOT(ref->Faces[n0].v[2]->Calc,u1) + d1;
										t2=-s1/(s2-s1);
										um.x=um0.x + t1*(um1.x-um0.x) + t2*(um2.x-um0.x);
										um.y=um0.y + t1*(um1.y-um0.y) + t2*(um2.y-um0.y);
										xm=um.x;
										ym=um.y;
										ShadowVertexBuffer->SetVertex(np,u.x,u.y,u.z);
										ShadowVertexBuffer->SetTexCoo(np,xm,ym);
										ShadowVertexBuffer->SetColor(np,Ambient.r,Ambient.g,Ambient.b,1);
										corres[n]=np;
										np++;
										
									}
								}

								for (n=0;n<nbre;n++)
								{
									if (faces[n].tag==1)
									{
//MODIF
//										if ((faces[n].s0->coef1>K_t)+(faces[n].s1->coef1>K_t)+(faces[n].s2->coef1>K_t)==3)
										{
											ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
											nf++;
										}
									}
								}				
							}						
						}
					}

					bsp.Plans.Free();
					bsp.Free();
				}

				if (anf<nf)
				{
					nfo.vEnd=-1;
					nfo.vStart=-1;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT2=0;
					id=CM.GetId();
					nfo.nT=id&0xFFFF;	
					nfo.nL=(id>>16)&0xFFFF;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
				}
				anf=nf;
				pg=ntg->ntgroup.GetNext();				
			}
			ntg=CM.GetNext();
		}
	}

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();
	ShadowVertexBuffer->Grouped=true;

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetParams(API3D_ZBIAS,4);
		SetVertexBuffer(ShadowVertexBuffer);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}

	CM.Free();

	delete [] bsp.VerticesS;	
	delete [] faces;	
	free(corres);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		
		Shadows : mesh mapping

		void C3DAPI::LightShadowsMeshMappingBL(int nLight,CObject3D * objb,CCollisionMesh * mesh,unsigned int flags)
		void C3DAPI::LightShadowsMeshMappingBLTags(int nLight,CObject3D * objb,CCollisionMesh * mesh,unsigned int flags)

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

		draw shadow using texture with CCollisionMesh optimization
		multitexturing support, bump mapping lightmap

		! finalised routine !

	Usage:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */




void C3DAPI::LightShadowsMeshMappingBL(int nLight,CObject3D * objb,CCollisionMesh * mesh,unsigned int flags)
{
    if (!Active) return;
    
	int Corres[8]={ -1,0,2,3,4,5,6,7};
	float size_texture_lightmap=(((float) SIZE_LIGHTMAPS)/256);
	CMatrix Mobj,Mrot,M,Mp,invMp;
	CVector4 v4;
	CPlane plan;
	CVector N,P;
	float t,s1,s2,t1,t2;
	int n,nn,bob;
	unsigned int id;
	float xm,ym;
	float x0,y0;
	float xml,yml;
	int np,nf,anf;
	CVector u1,u2,u,v1,v2,g;
	CVector u1l,u2l,ul,v1l,v2l,gl;
	int n0;
	float a,b,c,d;
	float d1;
	CObject3D *obj;
	float d2;
	CVector2 um0,um1,um2,um;
	CVector2 um0l,um1l,um2l,uml;
	int nbre;
	float r;
	CObject3D * ref;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	CIndicesSE nfo;
	int nLT;
	int nLTV;
	int REF;
	unsigned char * taglight;
	int startvertex;

	CM.Free();

	if (flags&MMBL_RESET)
	{
		ShadowVertexBuffer->ListeSE.Free();
		ShadowVertexBuffer->Grouped=false;
	}

	ref=mesh->obj;
	mesh->Actualise(0);

	if (objb->Double2)
	{
		obj=objb->Double2;
	}
	else obj=objb;

	if (obj->Radius<0)
		obj->Radius=obj->CalculateRadius();

	if (ref->Radius<0)
		ref->Radius=ref->CalculateRadius();
	
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

//	ref->Calculate(&Mobj,&Mrot);

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((objb->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=objb->Proprietes.ActualMatrix();
		Mrot=objb->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		if (objb->Output&PROPRIETES_MATRICES)
		{
			Mobj=objb->M;
			Mrot=objb->MR;
		}
		else
		{
			Mobj.Id();
			Mobj.RotationDegre(objb->Rot.x,objb->Rot.y,objb->Rot.z);
			Mrot=Mobj;
			Mobj.Translation(objb->Coo.x,objb->Coo.y,objb->Coo.z);
		}
	}

	M.Id();
	LoadWorldMatrix(M);
	obj->Calculate(&Mobj,&Mrot);

	for (n=0;n<obj->nFaces;n++)
	{
		N=obj->Faces[n].NormCalc;

		P=(obj->Faces[n].v[0]->Calc+obj->Faces[n].v[1]->Calc+obj->Faces[n].v[2]->Calc)/3;
		P=P - Lights[nLight].Pos;

		if ((N||P)<0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=1;

		obj->Faces[n].lock=0;
	}

	CBSPVolumicOld * bsp;
	CBSPVolumicOld tmp_bsp;

	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;

	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);
	tmp_bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	CVector A,L,frustum[5];

	A.Init(Mobj.a[3][0],Mobj.a[3][1],Mobj.a[3][2]);

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1.Base(u,0);
	u2.Base(u,1);

	r=5;
	d=4;

	frustum[0]=A+obj->Radius*u;
	u=A+obj->Radius*(u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[1]=A+r*u;

	u=A+obj->Radius*(u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[2]=A+r*u;

	u=A+obj->Radius*(-1*u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[3]=A+r*u;

	u=A+obj->Radius*(-1*u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[4]=A+r*u;
	
	list=mesh->quad.FrustumGetGroupList(frustum,mesh->M);


	if (flags&MMBL_RESET)
	{
		np=nf=0;
	}
	else
	{
		np=ShadowVertexBuffer->nVerticesActif;
		nf=ShadowVertexBuffer->nIndicesActif;
	}

	startvertex=np;

	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	for (n=0;n<obj->nVertices;n++)
	{
		tmp_bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		tmp_bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
	}

	anf=nf;

//	int nLV=0;
	int nL=0;
	int TAG=0;

	if (list->Length()>0)
	{

		pg=list->GetFirst();
		while (pg)
		{
			id=(*pg)->nT+(((*pg)->nL)<<12) + ((unsigned int) (*pg)->tag<<28);
			ntg=CM[id];
			ntg->ntgroup.Add(*pg);
			pg=list->GetNext();
		}

		int SIZE;

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			id=CM.GetId();

			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{			
				n0=*((*pg)->faces->GetFirst());

				taglight=(unsigned char *) &(ref->Faces[n0].tag2);

				SIZE=ref->Faces[n0].size_lightmap;

				nL=(id>>12)&0xFFFF;
				TAG=(id>>28)&0x7;

				bob=-1;
				TAG=ref->Faces[n0].tag;
				
				if ((taglight[0]==nLight)&&(TAG&1)) { TAG-=1;bob=0;}
				if ((taglight[1]==nLight)&&(TAG&2)) { TAG-=2;bob=0;}
				if ((taglight[2]==nLight)&&(TAG&4)) { TAG-=4;bob=0;}


				CLightmap *LMV=(*Lightmaps)[nL+1];

				if (Corres[TAG]<0) nL=0;
				else nL=nL+Corres[TAG];

				CLightmap *LM=(*Lightmaps)[nL];

				if ((LM)&&(LMV))
				{

					nLT=LM->nt;
					nLTV=LMV->nt;
					
					N=ref->Faces[n0].NormCalc;

					P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
					P=P - Lights[nLight].Pos;

					REF=ref->Faces[n0].ref;

					a=ref->Faces[n0].NormCalc.x;
					b=ref->Faces[n0].NormCalc.y;
					c=ref->Faces[n0].NormCalc.z;
					d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

					plan.Init(a,b,c,d);
					plan.Normalise();

					v4.Init(Lights[nLight].Pos);
					Mp.ShadowProjection(v4,plan);

					s1=a*L.x+b*L.y+c*L.z+d;
					

					if (((N||P)<0)&&(bob==0))
					for (n=0;n<obj->nVertices;n++)
					{
						A=obj->Vertices[n].Calc;
						s2=a*A.x+b*A.y+c*A.z+d;

						t=-s1/(s2-s1);

						if (t>K_t2) 
						{
							bob=1;
							obj->Vertices[n].tag2=1;
						}
						else obj->Vertices[n].tag2=0;
					}

					if (bob==-1) bob=0;

					if ((taglight[0]!=nLight)&&(taglight[1]!=nLight)&&(taglight[2]!=nLight)) bob=0;
				}
				else bob=0;

				CFigure *f=(*pg)->figures->GetFirst();
				while (f)
				{
					if (f->List.Length()==0) bob=0;
					f=(*pg)->figures->GetNext();
				}

				if (((N||P)<0)&&(bob))
				{
					
					for (n=0;n<obj->nFaces;n++)
					{
						if (obj->Faces[n].tag==1)
						{
							bob=(obj->Faces[n].v[0]->tag2==1)+(obj->Faces[n].v[1]->tag2==1)+(obj->Faces[n].v[2]->tag2==1);
							if (bob>0) obj->Faces[n].tag2=1;
							else obj->Faces[n].tag2=0;
						}
					}

					bsp=(*(*pg)->bsps)(nLight);
					if (!bsp) 
					{
						bsp=(*(*pg)->bsps)[nLight];
						bsp->VerticesS=tmp_bsp.VerticesS;

						bsp->BuildGroupVolume(ref,*pg,-1,1,Lights[nLight].Pos);						
					}
					else
					{
						bsp->VerticesS=tmp_bsp.VerticesS;
					}

					for (n=0;n<obj->nVertices;n++) bsp->VerticesS[n].tag=0;
					bsp->nVertices=obj->nVertices;
					
					nn=0;
					for (n=0;n<obj->nFaces;n++)
					{
						if ((obj->Faces[n].tag==1)&&(obj->Faces[n].tag2==1)&&(obj->Faces[n].lock==0))
						{
							faces[nn].n0=obj->Faces[n].v0;
							faces[nn].n1=obj->Faces[n].v1;
							faces[nn].n2=obj->Faces[n].v2;
							faces[nn].s0=&(bsp->VerticesS[faces[nn].n0]);
							faces[nn].s1=&(bsp->VerticesS[faces[nn].n1]);
							faces[nn].s2=&(bsp->VerticesS[faces[nn].n2]);
							faces[nn].tag=0;
							faces[nn].tag2=n;
							faces[nn].lock=0;

							nn++;
						}
					}

					nbre=0;
										
					if (nn>0)
					{
						
						bsp->faces=faces;
						bsp->nfaces=nn;
						
						bsp->InOutNoMap(0,0,bsp->root,-1);
						nbre=bsp->nfaces;
					}

					if (nbre>0)
					{

						nn=0;
						for (n=0;n<nbre;n++)
						{
							//if (bsp.ShortFaceInside(bsp.root,&faces[n]))
							if (faces[n].tag==-1)
							{
								if (faces[n].lock==0)
								{
									obj->Faces[faces[n].tag2].lock=1;
								}
								faces[n].tag=1;
								faces[n].s0->tag=1;
								faces[n].s1->tag=1;
								faces[n].s2->tag=1;
								nn++;
							}
							else faces[n].tag=0;
						}

						if (nn>0)
						{
							
							v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
							v2=ref->Faces[n0].NormCalc;
							u1=v1^v2;
							u1.Normalise();

							v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
							v2=ref->Faces[n0].NormCalc;
							u2=v1^v2;
							u2.Normalise();

							um0=ref->Faces[n0].v[0]->Map;
							um1=ref->Faces[n0].v[1]->Map;
							um2=ref->Faces[n0].v[2]->Map;
	
							um0l=ref->Faces[n0].v[0]->Map2;
							um1l=ref->Faces[n0].v[1]->Map2;
							um2l=ref->Faces[n0].v[2]->Map2;
							
							v1.Base(ref->Faces[n0].NormCalc,0);
							v2.Base(ref->Faces[n0].NormCalc,1);

							g.Init(0,0,0);
							nn=0;

							s1=a*Lights[nLight].Pos.x + b*Lights[nLight].Pos.y + c*Lights[nLight].Pos.z + d;

							CVector pu1,pu2;
							
							for (n=0;n<bsp->nVertices;n++)
							{
								s=&(bsp->VerticesS[n]);
								if (s->tag==1)
								{
									v4.Init(s->Calc);
									v4=v4*Mp;
									u.Init(v4.x,v4.y,v4.z);
									if (v4.w!=0.0f)	
									{
										u/=v4.w;
										pu1=u-Lights[nLight].Pos;
										pu2=s->Calc-Lights[nLight].Pos;
										s->coef1=pu1.Norme()-pu2.Norme();

										nn++;
									
										s->Calc=u;
									}
									else
									{
										s->tag=0;
										s->coef1=-1;
									}
								}
							}							
							
							if (nn>0)
							{
								for (n=0;n<bsp->nVertices;n++)
								{
									s=&(bsp->VerticesS[n]);
									if ((s->tag==1)&&(s->coef1>K_t))
									{
										u=s->Calc;

										xm=ym=0;
										xml=yml=0;

										d1=-(u1||u);
										d2=-(u2||u);
										s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
										s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;

										t1=-s1/(s2-s1);

										s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
										s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;

										t2=-s1/(s2-s1);

										um=um0+t1*(um1-um0) + t2*(um2-um0);
										uml=um0l+t1*(um1l-um0l) + t2*(um2l-um0l);

										xm=um.x;
										ym=um.y;

										xml=uml.x;
										yml=uml.y;

										if (np<(int)ShadowVertexBuffer->nVertices)
										{
											ShadowVertexBuffer->SetVertex(np,u.x,u.y,u.z);
											ShadowVertexBuffer->SetTexCoo(np,xm,ym);

											x0=LM->x*size_texture_lightmap+(LM->offx*size_texture_lightmap)/4;
											y0=LM->y*size_texture_lightmap+(LM->offy*size_texture_lightmap)/4;
											ShadowVertexBuffer->SetTexCoo2(np,x0 + xml*(SIZE*size_texture_lightmap)/4,y0 + yml*(SIZE*size_texture_lightmap)/4);

											if (ShadowVertexBuffer->Type&API3D_TEXCOO3BUMP)
											{
												x0=LMV->x*size_texture_lightmap + (LMV->offx*size_texture_lightmap)/4;
												y0=LMV->y*size_texture_lightmap + (LMV->offy*size_texture_lightmap)/4;
												ShadowVertexBuffer->SetTexCoo3(np,x0 + xml*(SIZE*size_texture_lightmap)/4,y0 + yml*(SIZE*size_texture_lightmap)/4);
											}

											//ShadowVertexBuffer->SetColor(np,0.7f,0.7f,0.7f);
//											ShadowVertexBuffer->SetColor(np,1.0f,1.0f,1.0f);
											corres[n]=np;
											np++;
										}										
									}
								}
														
								for (n=0;n<nbre;n++)
								{
									if (faces[n].tag==1)
									{
										if ((faces[n].s0->coef1>K_t)+(faces[n].s1->coef1>K_t)+(faces[n].s2->coef1>K_t)==3)
										{
											if ((nf+(nf<<1))<(int)ShadowVertexBuffer->nIndices)
											{
												ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
												nf++;
											}											
										}
									}
								}
							}
						}
					}
				}

				pg=ntg->ntgroup.GetNext();
			}

			if (anf<nf)
			{
				nfo.iStart=anf*3;
				nfo.ref=REF;
				nfo.nLV=nLTV;
				nfo.vEnd=np;
				nfo.vStart=startvertex;
				startvertex=np;
				nfo.nT2=0;
				id=CM.GetId();
				nfo.nT=id&((1<<12)-1);
				nfo.nL=nLT;
				nfo.iEnd=nf*3;
				ShadowVertexBuffer->ListeSE.Add(nfo);
			}
			anf=nf;
			
			ntg=CM.GetNext();
		}
	}

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	ShadowVertexBuffer->Grouped=true;

	CM.Free();

	delete [] tmp_bsp.VerticesS;
	delete [] faces;
	free(corres);
}

#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		draw slow shadowvolume on stencil buffer		
  
		void C3DAPI::DrawShadowVolumeExotic(int nL,CObject3D *obj,CMatrix M,CMatrix Mr)

		nL : light

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::DrawShadowVolumeExotic(int nL,CObject3D *obj,CMatrix M,CMatrix Mr,CVector Camera,float len)
{
    if (!Active) return;
    
	CVector O,A,B,C,D,A0,B0;
	CVector u0,u1,u2,u3,u;
	CVector N;
	CVector pos=Lights[nL].Pos;
	float ss,s[3];
	float K=f_abs(len);
	CMatrix mc;
	int n;

#define DRAW_EDGES(n) { VECTORSUB(u1,A,pos);VECTORSUB(u2,B,pos);VECTORNORMALISE(u1);VECTORNORMALISE(u2);VECTORSUB(u,O,pos);VECTORNORMALISE(u);DOTPRODUCT(ss,obj->Faces[n].NormCalc,u);VECTORMUL(u1,u1,(ss>0)*K);VECTORMUL(u2,u2,(ss>0)*K);VECTORADD(C,B,u2);VECTORADD(D,A,u1);Quad_Flat(A,B,C,D,1,1,1); }

	obj->Calculate(&M,&Mr);

	mc.Id();
	LoadWorldMatrix(mc);
	
	for (n=0;n<obj->nFaces;n++)
	{
		obj->Faces[n].tag=0;

		VECTORSUB(u1,obj->Faces[n].v[0]->Calc,pos);
		VECTORSUB(u2,obj->Faces[n].v[1]->Calc,pos);
		VECTORSUB(u3,obj->Faces[n].v[2]->Calc,pos);

		DOTPRODUCT(s[0],u1,obj->Faces[n].NormCalc);
		DOTPRODUCT(s[1],u2,obj->Faces[n].NormCalc);
		DOTPRODUCT(s[2],u3,obj->Faces[n].NormCalc);

		if ((s[0]>=0)&&(s[1]>=0)&&(s[2]>=0))
		if (obj->IntersectFaceApprox(pos,Camera,&obj->Faces[n])) obj->Faces[n].tag=1;
	}


	SetParams(API3D_ZBUFFER,ON);
	SetParams(API3D_ZBUFFER,WRITEOFF);
	SetParams(API3D_RENDERTARGET,STENCIL);
	SetParams(API3D_STENCIL,INCREMENT);
	SetParams(API3D_CULL,CW);

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==1)
		{
			VECTORADD(O,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc);
			VECTORADD(O,O,obj->Faces[n].v[2]->Calc);
			VECTORDIV(O,O,3.0f);
			A=obj->Faces[n].v[0]->Calc;
			B=obj->Faces[n].v[1]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[1]->Calc;
			B=obj->Faces[n].v[2]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[2]->Calc;
			B=obj->Faces[n].v[0]->Calc;
			DRAW_EDGES(n);
		}
	}

	SetParams(API3D_ZBUFFER,ON);
	SetParams(API3D_ZBUFFER,WRITEOFF);
	SetParams(API3D_RENDERTARGET,STENCIL);
	SetParams(API3D_STENCIL,DECREMENT);
	SetParams(API3D_CULL,CCW);

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==1)
		{
			VECTORADD(O,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc);
			VECTORADD(O,O,obj->Faces[n].v[2]->Calc);
			VECTORDIV(O,O,3.0f);
			A=obj->Faces[n].v[0]->Calc;
			B=obj->Faces[n].v[1]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[1]->Calc;
			B=obj->Faces[n].v[2]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[2]->Calc;
			B=obj->Faces[n].v[0]->Calc;
			DRAW_EDGES(n);
		}
	}

	SetParams(API3D_ZBUFFER,OFF);
	SetParams(API3D_RENDERTARGET,STENCIL);
	SetParams(API3D_STENCIL,INVERT);
	Quad(0,0,(float)GetWidth(),(float)GetHeight(),0,0,0,0,1,1,1);

	SetParams(API3D_ZBUFFER,ON);
	SetParams(API3D_ZBUFFER,WRITEOFF);
	SetParams(API3D_RENDERTARGET,STENCIL);
	SetParams(API3D_STENCIL,INCREMENT);
	SetParams(API3D_CULL,CW);
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			VECTORADD(O,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc);
			VECTORADD(O,O,obj->Faces[n].v[2]->Calc);
			VECTORDIV(O,O,3.0f);
			A=obj->Faces[n].v[0]->Calc;
			B=obj->Faces[n].v[1]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[1]->Calc;
			B=obj->Faces[n].v[2]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[2]->Calc;
			B=obj->Faces[n].v[0]->Calc;
			DRAW_EDGES(n);
		}
	}

	SetParams(API3D_ZBUFFER,ON);
	SetParams(API3D_ZBUFFER,WRITEOFF);
	SetParams(API3D_RENDERTARGET,STENCIL);
	SetParams(API3D_STENCIL,DECREMENT);
	SetParams(API3D_CULL,CCW);

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			VECTORADD(O,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc);
			VECTORADD(O,O,obj->Faces[n].v[2]->Calc);
			VECTORDIV(O,O,3.0f);
			A=obj->Faces[n].v[0]->Calc;
			B=obj->Faces[n].v[1]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[1]->Calc;
			B=obj->Faces[n].v[2]->Calc;
			DRAW_EDGES(n);
			A=obj->Faces[n].v[2]->Calc;
			B=obj->Faces[n].v[0]->Calc;
			DRAW_EDGES(n);
		}
	}

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		draw slow shadowvolume on stencil buffer		
  
		void C3DAPI::DrawShadowVolume(int nL,CObject3D *obj)

		nL : light

		obj->Output=0 => .Coo .Rot will be used$
		obj->Output=PROPRIETES_PHYSIQUES => .ActualMatrix()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::DrawShadowVolume(int nL,CObject3D *obj)
{
    if (!Active) return;
    
	CMatrix M,Mobj,Mrot;
	CVector u,P,L,A,B,C,D;
	int n,nf,np;
	EdgeList Edges;
	int c;

	if (StencilBuffer)
	{
		M.Id();
		LoadWorldMatrix(M);

#ifdef _DEFINES_API_CODE_PHYSIC_
		if (obj->Output&PROPRIETES_PHYSIQUES)
		{
			Mobj=obj->Proprietes.ActualMatrix();
			Mrot=obj->Proprietes.ActualOrientationMatrix();
		}
		else
#endif
		{

			if (obj->Output&PROPRIETES_MATRICES)
			{
				Mobj=obj->M;
				Mrot=obj->MR;
			}
			else
			{
				Mobj.Id();
				Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
				Mrot=Mobj;
				Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
			}
		}
		
		obj->Calculate(&Mobj,&Mrot);

		L=Lights[nL].Pos;

		nf=0;
		for (n=0;n<obj->nFaces;n++)
		{
			obj->Faces[n].tag=1;


			P=(obj->Faces[n].v[0]->Calc+obj->Faces[n].v[1]->Calc+obj->Faces[n].v[2]->Calc)/3;
			u=L-P;
			if ((u||obj->Faces[n].NormCalc)<0)
			{
				obj->Faces[n].tag=0;
				nf++;
			}
			
		}

		Edges.Init(10000);

		for (n=0;n<obj->nFaces;n++)
		{
			if (obj->Faces[n].tag==1)
			{
				nf=obj->Face_01(n);
				if (nf!=-1)
				{
					if (obj->Faces[nf].tag==0)
							Edges.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
				}
				nf=obj->Face_12(n);
				if (nf!=-1)
				{
					if (obj->Faces[nf].tag==0)
							Edges.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
				}
				nf=obj->Face_20(n);
				if (nf!=-1)
				{
					if (obj->Faces[nf].tag==0)
							Edges.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
				}
			}
		}

		c=*states['C'];

		SHADOW_VOLUME.LockVertices();
		SHADOW_VOLUME.LockIndices();

		np=nf=0;

		L=Lights[nL].Pos;
		for (n=0;n<Edges.nList;n++)
		{
			A=obj->Vertices[Edges.List[n].a].Calc;
			B=obj->Vertices[Edges.List[n].b].Calc;

			C=B+5*(B-L);
			D=A+5*(A-L);

			SHADOW_VOLUME.SetVertex(np+0,A);
			SHADOW_VOLUME.SetVertex(np+1,B);
			SHADOW_VOLUME.SetVertex(np+2,C);
			SHADOW_VOLUME.SetVertex(np+3,D);
			
			SHADOW_VOLUME.SetColor(np+0,1,1,1);
			SHADOW_VOLUME.SetColor(np+1,1,1,1);
			SHADOW_VOLUME.SetColor(np+2,1,1,1);
			SHADOW_VOLUME.SetColor(np+3,1,1,1);
						
			SHADOW_VOLUME.SetIndices(nf+0,np+0,np+1,np+2);
			SHADOW_VOLUME.SetIndices(nf+1,np+0,np+2,np+3);
			nf+=2;
			np+=4;
		}

        SHADOW_VOLUME.nVerticesActif=np;
        SHADOW_VOLUME.nIndicesActif=nf*3;

		SHADOW_VOLUME.UnlockVertices();
		SHADOW_VOLUME.UnlockIndices();

		if (nf>0)
		{
			c=*states['C'];
			SetParams(API3D_RENDERTARGET,STENCIL);
			SetParams(API3D_STENCIL,INCREMENT);
			SetParams(API3D_CULL,CW);
			SetVertexBuffer(&SHADOW_VOLUME);
			DrawVertexBuffer();
			
			SetParams(API3D_STENCIL,DECREMENT);
			SetParams(API3D_CULL,CCW);
			SetVertexBuffer(&SHADOW_VOLUME);
			DrawVertexBuffer();
			
			if (c==0) SetParams(API3D_CULL,CW);
			else SetParams(API3D_CULL,CCW);
		}


		Edges.Free();
	}
}

#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		project a texture to an object

		void C3DAPI::LightProjectTexture(int nLight,CVector Pos,float sizex,float sizey,CObject3D * ref,int DEF)

		Pos : center of the texture
		sizex,sizey : lengths

		DEF :  subdivision to avoid projective mapping lack 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightProjectTexture(int nLight,CVector Pos,float sizex,float sizey,CObject3D * ref,int DEF)
{
    if (!Active) return;
	LightProjectTexture(nLight,Pos,sizex,sizey,ref,DEF,COMMON_RESET);
}

void C3DAPI::LightProjectTexture(int nLight,CVector Pos,float sizex,float sizey,CObject3D * ref,int DEF,int reset)
{
    if (!Active) return;
    
	CIndicesSE nfo;
	CMatrix Mobj,Mrot,M;
	CVector N,P,Nf;
	int n,nn;
	int np,nf,anf;
	CVector U1,U2,u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d;
	CVector2 um0,um1,um2,um;
	int nbre;
	CPlane plan;
	CMatrix Mproj;
	CVector4 v4;
	CVector A,B,C,D;
	CVector2 Am,Bm,Cm,Dm;
	int n1,n2;
	int a0,a1,a2,a3;

	CBSPVolumicOld bsp;

	CVector L,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	u=Pos-Lights[nLight].Pos;
	u.Normalise();

	N=u;

	U1.Base(u,0);
	U2.Base(u,1);

	U1*=sizex;
	U2*=sizey;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;
	}

	L=Lights[nLight].Pos;
	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];

	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);

	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];
	
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	if (reset&COMMON_RESET)
	{
		anf=np=nf=0;
		ShadowVertexBuffer->ListeSE.Free();
		ShadowVertexBuffer->Grouped=true;
	}
	else
	{
		np=ShadowVertexBuffer->nVerticesActif;
		anf=nf=ShadowVertexBuffer->nIndicesActif/3;
		ShadowVertexBuffer->Grouped=true;
	}

	for (n0=0;n0<ref->nFaces;n0++)
	{
		Nf=ref->Faces[n0].NormCalc;

		a=ref->Faces[n0].NormCalc.x;
		b=ref->Faces[n0].NormCalc.y;
		c=ref->Faces[n0].NormCalc.z;
		d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);
		
		if ((N||Nf)<0)
		{
			for (n=0;n<4;n++) bsp.VerticesS[n].tag=0;
			for (n=0;n<4;n++) bsp.VerticesS[n].tag2=1;
	
			bsp.BuildVolumeFace(ref,n0,-1,-1,L);
									   // N
										  // cote +-
			A=Pos-U1-U2;
			Am.Init(0,0);
			B=Pos+U1-U2;
			Bm.Init(1,0);
			C=Pos+U1+U2;
			Cm.Init(1,1);
			D=Pos-U1+U2;
			Dm.Init(0,1);

			for (n1=0;n1<=DEF;n1++)
				for (n2=0;n2<=DEF;n2++)
				{
					n=n1+n2*(DEF+1);
					u=A+((float)n1)*(B-A)/((float)DEF)+((float)n2)*(D-A)/((float)DEF);
					um0=Am+((float)n1)*(Bm-Am)/((float)DEF)+((float)n2)*(Dm-Am)/((float)DEF);
					bsp.VerticesS[n].Stok=u;
					bsp.VerticesS[n].Calc=u;
					bsp.VerticesS[n].Map=um0;
				}

			bsp.nVertices=(DEF+1)*(DEF+1);

			nn=0;
			for (n1=0;n1<DEF;n1++)
				for (n2=0;n2<DEF;n2++)
				{

					a0=n1+n2*(DEF+1);
					a1=n1+1+n2*(DEF+1);
					a2=n1+1+(n2+1)*(DEF+1);
					a3=n1+(n2+1)*(DEF+1);

					faces[nn].tag=0;
					faces[nn].n0=a0;
					faces[nn].n1=a2;
					faces[nn].n2=a1;
					faces[nn].s0=&(bsp.VerticesS[a0]);
					faces[nn].s1=&(bsp.VerticesS[a2]);
					faces[nn].s2=&(bsp.VerticesS[a1]);
					nn++;
					faces[nn].tag=0;
					faces[nn].n0=a0;
					faces[nn].n1=a3;
					faces[nn].n2=a2;
					faces[nn].s0=&(bsp.VerticesS[a0]);
					faces[nn].s1=&(bsp.VerticesS[a3]);
					faces[nn].s2=&(bsp.VerticesS[a2]);
					nn++;
				}

			nbre=0;										
			if (nn>0)
			{
				bsp.faces=faces;
				bsp.nfaces=nn;
				bsp.InOut(0,0,bsp.root,1);
				nbre=bsp.nfaces;
			}

			if (nbre>0)
			{

				nn=0;
				for (n=0;n<nbre;n++)
				{
					if (faces[n].tag==-1)
					{
						faces[n].tag=1;
						faces[n].s0->tag=1;
						faces[n].s1->tag=1;
						faces[n].s2->tag=1;
						nn++;
					}
					else faces[n].tag=0;
				}

				if (nn>0)
				{
					float t1,t2,d1,d2,s1,s2,xm,ym;

					plan.Init(a,b,c,d);
					v4.Init(L);
					Mproj.ShadowProjection(v4,plan);

					if (reset&COMMON_MULTITEXTURE)
					{
						v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u1=v1^v2;
						u1.Normalise();

						v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
						v2=ref->Faces[n0].NormCalc;
						u2=v1^v2;
						u2.Normalise();

						um0=ref->Faces[n0].v[0]->Map;
						um1=ref->Faces[n0].v[1]->Map;
						um2=ref->Faces[n0].v[2]->Map;
						
						v1.Base(ref->Faces[n0].NormCalc,0);
						v2.Base(ref->Faces[n0].NormCalc,1);

						for (n=0;n<bsp.nVertices;n++)
						{
							s=&(bsp.VerticesS[n]);
							if (s->tag==1)
							{
								v4.Init(s->Stok);
								v4=v4*Mproj;
								u.Init(v4.x,v4.y,v4.z);

								xm=ym=0;
								d1=-(u1||u);
								d2=-(u2||u);
								s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
								s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;
								t1=-s1/(s2-s1);
								s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
								s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;
								t2=-s1/(s2-s1);
								um=um0 + t1*(um1-um0) + t2*(um2-um0);
								xm=um.x;
								ym=um.y;

								ShadowVertexBuffer->SetVertex(np,u);
								ShadowVertexBuffer->SetTexCoo(np,xm,ym);
								ShadowVertexBuffer->SetTexCoo2(np,s->Map);
								ShadowVertexBuffer->SetColor(np,1,1,1);
								corres[n]=np;
								np++;							
							}
						}

					}
					else
					{
						for (n=0;n<bsp.nVertices;n++)
						{
							s=&(bsp.VerticesS[n]);
							if (s->tag==1)
							{
								v4.Init(s->Stok);
								v4=v4*Mproj;
								u.Init(v4.x,v4.y,v4.z);
								ShadowVertexBuffer->SetVertex(np,u);
								ShadowVertexBuffer->SetTexCoo(np,s->Map);
								ShadowVertexBuffer->SetColor(np,1,1,1);
								corres[n]=np;
								np++;							
							}
						}
					}					
					
					for (n=0;n<nbre;n++)
					{
						if (faces[n].tag==1)
						{
							ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
							nf++;
						}
					}				
				}
			}

			if (reset&COMMON_MULTITEXTURE)
			{
				if (nf>anf)
				{
					nfo.vEnd=-1;
					nfo.vStart=-1;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT=ref->Faces[n0].nT;
					nfo.nT2=aT;
					nfo.nL=-1;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
					anf=nf;
				}
			}

			bsp.Plans.Free();
			bsp.Free();
			
		}
	}

	LoadWorldMatrix(M);

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();
	
	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		SetParams(API3D_ZBIAS,4);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}
	else
	{
		if ((reset&COMMON_MULTITEXTURE)==0)
		{
			if (nf>0)
			{
				nfo.vEnd=-1;
				nfo.vStart=-1;
				nfo.iStart=anf*3;
				nfo.ref=1;
				nfo.nLV=0;
				nfo.nT2=0;
				nfo.nT=aT;					
				nfo.nL=-1;
				nfo.iEnd=nf*3;
				ShadowVertexBuffer->ListeSE.Add(nfo);
			}
		}
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		project an object to another object

		void C3DAPI::LightProjectObject(int nLight,CObject3D *obj,CObject3D * ref)
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::LightProjectObject(int nLight,CObject3D *obj,CObject3D * ref)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P,Nf;
	int n,nn;
	float s1,s2,t;
	int np,nf;
	CVector U1,U2,u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d;
	CVector2 um0,um1,um2,um;
	int nbre;
	CPlane plan;
	CMatrix Mproj;
	CVector4 v4;
	CVector A,B,C,D;
	CVector2 Am,Bm,Cm,Dm;
	CBSPVolumicOld bsp;
	CVector L,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;

	ShadowVertexBuffer->ListeSE.Free();
	ShadowVertexBuffer->Grouped=false;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;
	}

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
	}

	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Calc=obj->Vertices[n].Stok*Mobj;
		
	}

	L=Lights[nLight].Pos;

	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);
	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
		bsp.VerticesS[n].Map=obj->Vertices[n].Map;
	}
	
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	np=nf=0;

	for (n0=0;n0<ref->nFaces;n0++)
	{

		N=ref->Faces[n0].NormCalc;

		P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
		P=P - Lights[nLight].Pos;

		a=ref->Faces[n0].NormCalc.x;
		b=ref->Faces[n0].NormCalc.y;
		c=ref->Faces[n0].NormCalc.z;
		d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

		s1=a*L.x+b*L.y+c*L.z+d;
		
		for (n=0;n<obj->nVertices;n++)
		{
			A=obj->Vertices[n].Calc;
			s2=a*A.x+b*A.y+c*A.z+d;

			t=-s1/(s2-s1);

			if (t>1) break;
		}

		if (((N||P)<0)&&(t>1))
		{
			for (n=0;n<obj->nVertices;n++) bsp.VerticesS[n].tag=0;

			plan.Init(a,b,c,d);
			v4.Init(L);
			Mproj.ShadowProjection(v4,plan);
			
			bsp.BuildVolumeFace(ref,n0,-1,-1,L);
									   // N
										  // cote +-
			nn=0;
			for (n=0;n<obj->nFaces;n++)
			{
				faces[nn].n0=obj->Faces[n].v0;
				faces[nn].n1=obj->Faces[n].v1;
				faces[nn].n2=obj->Faces[n].v2;
				faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
				faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
				faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
				nn++;
			}

			bsp.nVertices=obj->nVertices;
					
			nbre=0;
			if (nn>0)
			{
				bsp.ListePlans(bsp.root);
				
				nbre=bsp.CutFacesMap(0,faces,nn,-1);
			}

			if (nbre>0)
			{
				nn=0;
				for (n=0;n<nbre;n++)
				{
					if (bsp.ShortFaceInside(bsp.root,&faces[n]))
					{
						faces[n].tag=1;
						faces[n].s0->tag=1;
						faces[n].s1->tag=1;
						faces[n].s2->tag=1;
						nn++;
					}
					else faces[n].tag=0;
				}

				if (nn>0)
				{
		
					for (n=0;n<bsp.nVertices;n++)
					{
						s=&(bsp.VerticesS[n]);
						if (s->tag==1)
						{
							v4.Init(s->Calc);
							v4=v4*Mproj;
							u.Init(v4.x,v4.y,v4.z);
			
							ShadowVertexBuffer->SetVertex(np,u);
							ShadowVertexBuffer->SetTexCoo(np,s->Map);
							ShadowVertexBuffer->SetColor(np,1,1,1);
							corres[n]=np;
							np++;							
						}
					}

					for (n=0;n<nbre;n++)
					{
						if (faces[n].tag==1)
						{
							ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
							nf++;
						}
					}				
				}
			}

			bsp.Plans.Free();
			bsp.Free();
		}
	}

	LoadWorldMatrix(M);

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		SetParams(API3D_ZBIAS,2);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		project an object to a collision mesh (type 2)

		void C3DAPI::LightProjectMesh(int nLight,CObject3D *obj,CCollisionMesh * mesh)
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*	float radius=1.0f*((float) sqrt(sizex*sizex+sizey*sizey));
	CIndicesSE nfo;
	CMatrix Mobj,Mrot,M;
	CVector N,P,Nf;
	int n,nn;
	int np,nf,anf,anp;
	CVector U1,U2,u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d,r,s1,s2,t;
	CVector2 um0,um1,um2,um;
	int nbre;
	CPlane plan;
	CMatrix Mproj;
	CVector4 v4;
	CVector A,B,C,D;
	CVector2 Am,Bm,Cm,Dm;
	int n1,n2;
	int a0,a1,a2,a3;
	CBSPVolumicOld bsp;
	CObject3D *ref=mesh->obj;
	CVector frustum[5];
	CVector L,T;
	CVertex *s,ds;
	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;
	CShortFace *faces;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	unsigned int id;
	CVector pu1,pu2;
	int * corres;

	mesh->Actualise(0);

	u=Pos-Lights[nLight].Pos;
	u.Normalise();

	N=u;

	U1.Base(u,0);
	U2.Base(u,1);

	U1*=sizex;
	U2*=sizey;

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;
	}

	L=Lights[nLight].Pos;
	faces=new CShortFace[50000];

	corres=(int*) malloc(50000);

	bsp.VerticesS=new CVertex[50000];

	


	A=Pos;

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1=U1;
	u2=U2;

	r=20;

	d=1.0f;

	frustum[0]=A;
	u=A+radius*(u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[1]=A+r*u;

	u=A+radius*(u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[2]=A+r*u;

	u=A+radius*(-1*u1-u2)/d;
	u=u-L;
	u.Normalise();
	frustum[3]=A+r*u;

	u=A+radius*(-1*u1+u2)/d;
	u=u-L;
	u.Normalise();
	frustum[4]=A+r*u;

	list=mesh->quad.FrustumGetGroupList(frustum,mesh->M);

	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	if (reset&COMMON_RESET)
	{
		anp=anf=np=nf=0;
		ShadowVertexBuffer->ListeSE.Free();
		ShadowVertexBuffer->Grouped=true;
	}
	else
	{
		anp=np=ShadowVertexBuffer->nVerticesActif;
		anf=nf=ShadowVertexBuffer->nIndicesActif/3;
		ShadowVertexBuffer->Grouped=true;
	}

	if (list->Length()>0)
	{	
		
		pg=list->GetFirst();
		while (pg)
		{
			id=(*pg)->nT;
			ntg=CM[id];
			ntg->nT=(*pg)->nT;
			ntg->ntgroup.Add((*pg));
			pg=list->GetNext();
		}

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{
				n0=*((*pg)->faces.GetFirst());

				Nf=ref->Faces[n0].NormCalc;

				a=ref->Faces[n0].NormCalc.x;
				b=ref->Faces[n0].NormCalc.y;
				c=ref->Faces[n0].NormCalc.z;
				d=-(ref->Faces[n0].NormCalc||ref->Faces[n0].v[0]->Calc);

				P=(ref->Faces[n0].v[0]->Calc+ref->Faces[n0].v[1]->Calc+ref->Faces[n0].v[2]->Calc)/3;
				P=P - Lights[nLight].Pos;

				if (((N||Nf)<-0.1f)&&((Nf||P)<-0.1f))
				{

					for (n=0;n<4;n++) bsp.VerticesS[n].tag=0;
					for (n=0;n<4;n++) bsp.VerticesS[n].tag2=1;

					bsp.BuildGroupVolume(ref,*pg,-1,1,Lights[nLight].Pos);

					if (reset&PROJECT_BASE_GROUND)
					{
						A=Pos-0*U1-U2;
						Am.Init(0,0);
						B=Pos+2*U1-U2;
						Bm.Init(1,0);
						C=Pos+2*U1+U2;
						Cm.Init(1,1);
						D=Pos-0*U1+U2;
						Dm.Init(0,1);
					}
					else
					{
						A=Pos-U1-U2;
						Am.Init(0,0);
						B=Pos+U1-U2;
						Bm.Init(1,0);
						C=Pos+U1+U2;
						Cm.Init(1,1);
						D=Pos-U1+U2;
						Dm.Init(0,1);
					}

					s1=a*L.x+b*L.y+c*L.z+d;
					//s1=a*Pos.x+b*Pos.y+c*Pos.z+d;

					int bob=0;

					for (n1=0;n1<=DEF;n1++)
						for (n2=0;n2<=DEF;n2++)
						{
							n=n1+n2*(DEF+1);
							u=A+((float)n1)*(B-A)/((float)DEF)+((float)n2)*(D-A)/((float)DEF);
							um0=Am+((float)n1)*(Bm-Am)/((float)DEF)+((float)n2)*(Dm-Am)/((float)DEF);
							bsp.VerticesS[n].Stok=u;
							bsp.VerticesS[n].Calc=u;
							bsp.VerticesS[n].Map=um0;
							s2=a*u.x+b*u.y+c*u.z+d;
							t=-s1/(s2-s1);
							if (t>K_t2) 
							{
								bob=1;
								bsp.VerticesS[n].tag2=1;
							}
							else
							{
								bsp.VerticesS[n].tag2=0;
								
							}
						}

					bsp.nVertices=(DEF+1)*(DEF+1);

			}

			if (reset&COMMON_MULTITEXTURE)
			{
				if (nf>anf)
				{
					nfo.vEnd=np;
					nfo.vStart=anp;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT=ntg->nT;
					nfo.nT2=aT;
					nfo.nL=-1;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
					anf=nf;
					anp=np;
				}
			}


			ntg=CM.GetNext();
		}
	}

	LoadWorldMatrix(M);

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	ShadowVertexBuffer->nVerticesActif=np;
	ShadowVertexBuffer->nIndicesActif=nf*3;
	
	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		SetParams(API3D_ZBIAS,4);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}
	else
	{
		if ((reset&COMMON_MULTITEXTURE)==0)
		{
			if (nf>0)
			{
				nfo.vEnd=np;
				nfo.vStart=anp;
				nfo.iStart=anf*3;
				nfo.ref=1;
				nfo.nLV=0;
				nfo.nT2=0;
				nfo.nT=aT;					
				nfo.nL=-1;
				nfo.iEnd=nf*3;
				ShadowVertexBuffer->ListeSE.Add(nfo);
			}
		}
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);
}
/**/


void C3DAPI::LightProjectMesh(int nLight,CObject3D *obj,CCollisionMesh * mesh,int reset)
{
    if (!Active) return;
    
	CMatrix Mobj,Mrot,M;
	CVector N,P,Nf;
	int n,nn;
	int np,nf;
	int anp,anf;
	CVector U1,U2,u1,u2,u,v1,v2;
	int n0;
	float a,b,c,d;
	CVector2 um0,um1,um2,um;
	int nbre;
	CPlane plan;
	CMatrix Mproj;
	CVector4 v4;
	CVector A,B,C,D;
	CVector2 Am,Bm,Cm,Dm;
	CBSPVolumicOld bsp;
	CVector L,T;
	CShortVertexMap *s,ds;
	CShortFaceMap *faces;
	int * corres;
	CObject3D * ref;
	CVector Pos=obj->Coo;
	CVector frustum[5];
	CIndicesSE nfo;
	CList <CFaceGROUP*> *list=NULL;
	CFaceGROUP ** pg;
	CMap <unsigned int,CnTGroup> CM;
	CnTGroup * ntg;
	unsigned int id;
	
	ref=mesh->obj;

	mesh->Actualise(0);

	M.Id();
#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((ref->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=ref->Proprietes.ActualMatrix();
		Mrot=ref->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(ref->Rot.x,ref->Rot.y,ref->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(ref->Coo.x,ref->Coo.y,ref->Coo.z);
	}

	for (n=0;n<ref->nVertices;n++)
	{
		ref->Vertices[n].Calc=ref->Vertices[n].Stok*Mobj;
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*Mrot;
	}

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].NormCalc=ref->Faces[n].Norm*Mrot;

#ifdef _DEFINES_API_CODE_PHYSIC_
	if ((obj->Output&PROPRIETES_PHYSIQUES)!=0)
	{
		Mobj=obj->Proprietes.ActualMatrix();
		Mrot=obj->Proprietes.ActualOrientationMatrix();
	}
	else
#endif
	{
		Mobj.Id();
		Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
		Mrot=Mobj;
		Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);
	}

	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Calc=obj->Vertices[n].Stok*Mobj;
		obj->Vertices[n].NormCalc=obj->Vertices[n].Norm*Mrot;
	}

	A=Pos;
	u=Pos-Lights[nLight].Pos;
	u.Normalise();
	N=u;

	U1.Base(u,0);
	U2.Base(u,1);

	L=Lights[nLight].Pos;
	u=L-A;
	u.Normalise();

	u1=U1;
	u2=U2;


	list=mesh->quad.GetGroupList();

	faces=new CShortFaceMap[NB_DATA_PROJECTED_SHADOWS];
	corres=(int*) malloc(NB_DATA_PROJECTED_SHADOWS);
	bsp.VerticesS=new CShortVertexMap[NB_DATA_PROJECTED_SHADOWS];

	for (n=0;n<obj->nVertices;n++)
	{
		bsp.VerticesS[n].Stok=obj->Vertices[n].Stok;
		bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
		bsp.VerticesS[n].Map=obj->Vertices[n].Map;
	}
	
	ShadowVertexBuffer->LockVertices();
	ShadowVertexBuffer->LockIndices();

	if (reset&COMMON_RESET)
	{
		anp=anf=np=nf=0;
		ShadowVertexBuffer->ListeSE.Free();
		ShadowVertexBuffer->Grouped=true;
	}
	else
	{
		anp=np=ShadowVertexBuffer->nVerticesActif;
		anf=nf=ShadowVertexBuffer->nIndicesActif/3;
		ShadowVertexBuffer->Grouped=true;
	}

	if (list->Length()>0)
	{			
		pg=list->GetFirst();
		while (pg)
		{
			id=(*pg)->nT;
			ntg=CM[id];
			ntg->nT=(*pg)->nT;
			ntg->ntgroup.Add((*pg));
			pg=list->GetNext();
		}

		id=0;
		ntg=CM.GetFirst();
		while (ntg)
		{
			pg=ntg->ntgroup.GetFirst();
			while (pg)
			{
				n0=*((*pg)->faces->GetFirst());

				Nf=ref->Faces[n0].NormCalc;

				a=ref->Faces[n0].NormCalc.x;
				b=ref->Faces[n0].NormCalc.y;
				c=ref->Faces[n0].NormCalc.z;
				d=-DOT(ref->Faces[n0].NormCalc,ref->Faces[n0].v[0]->Calc);

				VECTORADDDIV3(P,ref->Faces[n0].v[0]->Calc,ref->Faces[n0].v[1]->Calc,ref->Faces[n0].v[2]->Calc);
				VECTORSUB(P,P,Lights[nLight].Pos);

				if ((DOT(N,Nf)<-0.1f))
				{
					bsp.BuildGroupVolume(ref,*pg,-1,-1,Lights[nLight].Pos);

					for (n=0;n<obj->nVertices;n++)
					{
						bsp.VerticesS[n].Stok=obj->Vertices[n].Calc;
						bsp.VerticesS[n].Calc=obj->Vertices[n].Calc;
						bsp.VerticesS[n].Map=obj->Vertices[n].Map;
						bsp.VerticesS[n].tag=0;
						bsp.VerticesS[n].tag2=1;
					}

					bsp.nVertices=obj->nVertices;
					
					nn=0;
					for (n=0;n<obj->nFaces;n++)
					{
						faces[nn].n0=obj->Faces[n].v0;
						faces[nn].n1=obj->Faces[n].v1;
						faces[nn].n2=obj->Faces[n].v2;
						faces[nn].s0=&(bsp.VerticesS[faces[nn].n0]);
						faces[nn].s1=&(bsp.VerticesS[faces[nn].n1]);
						faces[nn].s2=&(bsp.VerticesS[faces[nn].n2]);
						nn++;
					}

					nbre=0;
					if (nn>0)
					{
						bsp.faces=faces;
						bsp.nfaces=nn;
						bsp.InOut(0,0,bsp.root,-1);
						nbre=bsp.nfaces;
					}
					
					if (nbre>0)
					{
						nn=0;
						for (n=0;n<nbre;n++)
						{
							if (faces[n].tag==-1)
							{
								faces[n].tag=1;
								faces[n].s0->tag=1;
								faces[n].s1->tag=1;
								faces[n].s2->tag=1;
								nn++;
							}
							else faces[n].tag=0;
						}

						if (nn>0)
						{
							float t1,t2,d1,d2,s1,s2,xm,ym;

							plan.Init(a,b,c,d);
							v4.Init(L);
							Mproj.ShadowProjection(v4,plan);

							if (reset&COMMON_MULTITEXTURE)
							{
								v1=ref->Faces[n0].v[1]->Calc-ref->Faces[n0].v[0]->Calc;
								v2=ref->Faces[n0].NormCalc;
								u1=v1^v2;
								u1.Normalise();

								v1=ref->Faces[n0].v[2]->Calc-ref->Faces[n0].v[0]->Calc;
								v2=ref->Faces[n0].NormCalc;
								u2=v1^v2;
								u2.Normalise();

								um0=ref->Faces[n0].v[0]->Map;
								um1=ref->Faces[n0].v[1]->Map;
								um2=ref->Faces[n0].v[2]->Map;
								
								v1.Base(ref->Faces[n0].NormCalc,0);
								v2.Base(ref->Faces[n0].NormCalc,1);

								for (n=0;n<bsp.nVertices;n++)
								{
									s=&(bsp.VerticesS[n]);
									if (s->tag==1)
									{
										v4.Init(s->Stok);
										v4=v4*Mproj;
										u.Init(v4.x,v4.y,v4.z);
										xm=ym=0;
										d1=-(u1||u);
										d2=-(u2||u);
										s1=(ref->Faces[n0].v[0]->Calc||u2) + d2;
										s2=(ref->Faces[n0].v[1]->Calc||u2) + d2;
										t1=-s1/(s2-s1);
										s1=(ref->Faces[n0].v[0]->Calc||u1) + d1;
										s2=(ref->Faces[n0].v[2]->Calc||u1) + d1;
										t2=-s1/(s2-s1);
										um=um0 + t1*(um1-um0) + t2*(um2-um0);
										xm=um.x;
										ym=um.y;
										if (np<(int)ShadowVertexBuffer->nVertices)
										{
											ShadowVertexBuffer->SetVertex(np,u);
											ShadowVertexBuffer->SetTexCoo(np,xm,ym);
											ShadowVertexBuffer->SetTexCoo2(np,s->Map);
											ShadowVertexBuffer->SetColor(np,1,1,1);
											corres[n]=np;
											np++;
										}
									}
								}

							}
							else
							{
								for (n=0;n<bsp.nVertices;n++)
								{
									s=&(bsp.VerticesS[n]);
									if (s->tag==1)
									{
										v4.Init(s->Stok);
										v4=v4*Mproj;
										u.Init(v4.x,v4.y,v4.z);
										if (np<(int)ShadowVertexBuffer->nVertices)
										{
											ShadowVertexBuffer->SetVertex(np,u);
											ShadowVertexBuffer->SetTexCoo(np,s->Map);
											ShadowVertexBuffer->SetColor(np,1,1,1);
											corres[n]=np;
											np++;
										}
									}
								}
							}

							for (n=0;n<nbre;n++)
							{
								if (faces[n].tag==1)
								{
									if (nf<(int)ShadowVertexBuffer->nIndices/3)
									{
										ShadowVertexBuffer->SetIndices(nf,corres[faces[n].n0],corres[faces[n].n2],corres[faces[n].n1]);
										nf++;
									}
								}
							}		
						}

						bsp.Plans.Free();
						bsp.Free();
					}
				}
				pg=ntg->ntgroup.GetNext();				
			}

			if (reset&COMMON_MULTITEXTURE)
			{
				if (nf>anf)
				{
					nfo.vEnd=np;
					nfo.vStart=anp;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT=ntg->nT;
					nfo.nT2=aT;
					nfo.nL=-1;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
					anf=nf;
					anp=np;
				}
			}
			else
			{
				if (nf>anf)
				{
					nfo.vEnd=np;
					nfo.vStart=anp;
					nfo.iStart=anf*3;
					nfo.ref=0;
					nfo.nLV=0;
					nfo.nT=((C3DAPIBASE*)ShadowVertexBuffer->capi)->aT;
					nfo.nT2=aT;
					nfo.nL=-1;
					nfo.iEnd=nf*3;
					ShadowVertexBuffer->ListeSE.Add(nfo);
					anf=nf;
					anp=np;
				}
			}

			ntg=CM.GetNext();
		}
	}

	LoadWorldMatrix(M);

    ShadowVertexBuffer->nVerticesActif=np;
    ShadowVertexBuffer->nIndicesActif=nf*3;

	ShadowVertexBuffer->UnlockVertices();
	ShadowVertexBuffer->UnlockIndices();

	if ((nf>0)&&(states(SHADOW_BUFFERING)))
	{
		SetVertexBuffer(ShadowVertexBuffer);
		SetParams(API3D_ZBIAS,4);
		DrawVertexBuffer();
		SetParams(API3D_ZBIAS,0);
	}
	else
	{
		if ((reset&COMMON_MULTITEXTURE)==0)
		{
			if (nf>anf)			
			{
				nfo.vEnd=np;
				nfo.vStart=anp;
				nfo.iStart=anf*3;
				nfo.ref=0;
				nfo.nLV=0;
				nfo.nT2=0;
				nfo.nT=aT;
				nfo.nL=-1;
				nfo.iEnd=nf*3;
				ShadowVertexBuffer->ListeSE.Add(nfo);
			}
		}
	}

	delete [] bsp.VerticesS;
	
	delete [] faces;
	free(corres);
}

#endif



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		set of face by face drawing

	 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::Tri_GouraudMapping(CFace *f)
{
    if (!Active) return;

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,1.0f);
	vbquad_xyzcolortex.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,1.0f);
	vbquad_xyzcolortex.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,1.0f);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Calc.x,-f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Calc.x,-f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Calc.x,-f->v[2]->Calc.y,f->v[2]->Calc.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,1-f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,1-f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,1-f->mp2.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
#endif
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,1);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,1);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,1);

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif


#ifdef API3D_OPENGL

#ifndef GLES
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor3f(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b);
        if (aT<0) glTexCoord2f(f->mp0.x,1-f->mp0.y); else glTexCoord2f(f->mp0.x,f->mp0.y);
		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);

		glColor3f(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b);
		if (aT<0) glTexCoord2f(f->mp1.x,1-f->mp1.y); else glTexCoord2f(f->mp1.x,f->mp1.y);
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);

		glColor3f(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b);
		if (aT<0) glTexCoord2f(f->mp2.x,1-f->mp2.y); else glTexCoord2f(f->mp2.x,f->mp2.y);
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);

	glEnd();	
#else

	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->v[0]->Diffuse.r;
	colorvertices[0*4+1]=f->v[0]->Diffuse.g;
	colorvertices[0*4+2]=f->v[0]->Diffuse.b;
	colorvertices[0*4+3]=1.0f;

	colorvertices[1*4+0]=f->v[1]->Diffuse.r;
	colorvertices[1*4+1]=f->v[1]->Diffuse.g;
	colorvertices[1*4+2]=f->v[1]->Diffuse.b;
	colorvertices[1*4+3]=1.0f;

	colorvertices[2*4+0]=f->v[2]->Diffuse.r;
	colorvertices[2*4+1]=f->v[2]->Diffuse.g;
	colorvertices[2*4+2]=f->v[2]->Diffuse.b;
	colorvertices[2*4+3]=1.0f;

    if (aT<0)
    {
        texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=1-f->mp0.y;
        texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=1-f->mp1.y;
        texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=1-f->mp2.y;
    }
    else
    {
        texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=f->mp0.y;
        texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=f->mp1.y;
        texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=f->mp2.y;
    }

	vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
	vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
	vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLES,0,3);

#endif

#endif
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX Vertices[3];

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,1);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,1);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,1);

	BYTE * pVertices;
	D3DVB_XYZCOLORTEX->Lock(0,3*sizeof(XYZCOLORTEX),(BYTE**)&pVertices,0);
	memcpy(pVertices,Vertices,3*sizeof(XYZCOLORTEX));
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX Vertices[3];

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,1);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,1);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,1);

	BYTE * pVertices;
	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&pVertices,0);
	memcpy(pVertices,Vertices,3*sizeof(XYZCOLORTEX));
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif


}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::Tri_Mapping(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
		glColor3f(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b);
		
		if (aT<0) glTexCoord2f(f->mp0.x,1-f->mp0.y); else glTexCoord2f(f->mp0.x,f->mp0.y);
		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);
		
		if (aT<0) glTexCoord2f(f->mp1.x,1-f->mp1.y); else glTexCoord2f(f->mp1.x,f->mp1.y);
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);

		if (aT<0) glTexCoord2f(f->mp2.x,1-f->mp2.y); else glTexCoord2f(f->mp2.x,f->mp2.y);
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);

	glEnd();	
#else
	static GLfloat vertices[4*3];
		static GLfloat texvertices[4*2];
		static GLfloat colorvertices[4*4];

		colorvertices[0*4+0]=f->Diffuse.r;
		colorvertices[0*4+1]=f->Diffuse.g;
		colorvertices[0*4+2]=f->Diffuse.b;
		colorvertices[0*4+3]=1.0f;

		colorvertices[1*4+0]=f->Diffuse.r;
		colorvertices[1*4+1]=f->Diffuse.g;
		colorvertices[1*4+2]=f->Diffuse.b;
		colorvertices[1*4+3]=1.0f;

		colorvertices[2*4+0]=f->Diffuse.r;
		colorvertices[2*4+1]=f->Diffuse.g;
		colorvertices[2*4+2]=f->Diffuse.b;
		colorvertices[2*4+3]=1.0f;
    
        if (aT<0)
        {
            texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=1-f->mp0.y;
            texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=1-f->mp1.y;
            texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=1-f->mp2.y;
        }
        else
        {
            texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=f->mp0.y;
            texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=f->mp1.y;
            texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=f->mp2.y;
        }

		vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
		vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
		vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

		glVertexPointer(3,GL_FLOAT,0,vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4,GL_FLOAT,0,colorvertices);
		glEnableClientState(GL_COLOR_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,texvertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLES,0,3);

#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,3*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=color;
	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=color;
	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------
	
	float r=f->Diffuse.r;
	float g=f->Diffuse.g;
	float b=f->Diffuse.b;
	float a=1.0f;

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,r,g,b,a);
	vbquad_xyzcolortex.SetColor(1,r,g,b,a);
	vbquad_xyzcolortex.SetColor(2,r,g,b,a);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Calc.x,f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Calc.x,f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Calc.x,f->v[2]->Calc.y,f->v[2]->Calc.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,1-f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,1-f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,1-f->mp2.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
#endif
    
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;
	Vertices[2].color=color;

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Tri_MappingStokF(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
		glColor3f(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b);
		
		if (aT<0) glTexCoord2f(f->mp0.x,1-f->mp0.y); else glTexCoord2f(f->mp0.x,f->mp0.y);
		glVertex3f(f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
		
		if (aT<0) glTexCoord2f(f->mp1.x,1-f->mp1.y); else glTexCoord2f(f->mp1.x,f->mp1.y);
		glVertex3f(f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);

		if (aT<0) glTexCoord2f(f->mp2.x,1-f->mp2.y); else glTexCoord2f(f->mp2.x,f->mp2.y);
		glVertex3f(f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

	glEnd();	
#else
	static GLfloat vertices[4*3];
		static GLfloat texvertices[4*2];
		static GLfloat colorvertices[4*4];

		colorvertices[0*4+0]=f->Diffuse.r;
		colorvertices[0*4+1]=f->Diffuse.g;
		colorvertices[0*4+2]=f->Diffuse.b;
		colorvertices[0*4+3]=1.0f;

		colorvertices[1*4+0]=f->Diffuse.r;
		colorvertices[1*4+1]=f->Diffuse.g;
		colorvertices[1*4+2]=f->Diffuse.b;
		colorvertices[1*4+3]=1.0f;

		colorvertices[2*4+0]=f->Diffuse.r;
		colorvertices[2*4+1]=f->Diffuse.g;
		colorvertices[2*4+2]=f->Diffuse.b;
		colorvertices[2*4+3]=1.0f;
    
        if (aT<0)
        {
            texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=1-f->mp0.y;
            texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=1-f->mp1.y;
            texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=1-f->mp2.y;
        }
        else
        {
            texvertices[2*0+0]=f->mp0.x; texvertices[2*0+1]=f->mp0.y;
            texvertices[2*1+0]=f->mp1.x; texvertices[2*1+1]=f->mp1.y;
            texvertices[2*2+0]=f->mp2.x; texvertices[2*2+1]=f->mp2.y;
        }

		vertices[3*0+0]=f->v[0]->Stok.x; vertices[3*0+1]=f->v[0]->Stok.y; vertices[3*0+2]=f->v[0]->Stok.z;
		vertices[3*1+0]=f->v[1]->Stok.x; vertices[3*1+1]=f->v[1]->Stok.y; vertices[3*1+2]=f->v[1]->Stok.z;
		vertices[3*2+0]=f->v[2]->Stok.x; vertices[3*2+1]=f->v[2]->Stok.y; vertices[3*2+2]=f->v[2]->Stok.z;

		glVertexPointer(3,GL_FLOAT,0,vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4,GL_FLOAT,0,colorvertices);
		glEnableClientState(GL_COLOR_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,texvertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,3*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=color;

	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;

	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;

	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;

	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;

	Vertices[2].color=color;
	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,1.0f);
	vbquad_xyzcolortex.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,1.0f);
	vbquad_xyzcolortex.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,1.0f);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);
#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,1-f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,1-f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,1-f->mp2.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
    }
#else
    vbquad_xyzcolortex.SetTexCoo(0,f->mp0.x,f->mp0.y);
    vbquad_xyzcolortex.SetTexCoo(1,f->mp1.x,f->mp1.y);
    vbquad_xyzcolortex.SetTexCoo(2,f->mp2.x,f->mp2.y);
#endif
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,1);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].xm=f->mp0.x;
	Vertices[0].ym=f->mp0.y;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].xm=f->mp1.x;
	Vertices[1].ym=f->mp1.y;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].xm=f->mp2.x;
	Vertices[2].ym=f->mp2.y;
	Vertices[2].color=color;

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;


	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Quad_MappingStok(CVertex * v0,CVertex * v1,CVertex * v2,CVertex * v3,float r,float g,float b,float a)
{
    if (!Active) return;

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------
	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,r,g,b,a);
	vbquad_xyzcolortex.SetColor(1,r,g,b,a);
	vbquad_xyzcolortex.SetColor(2,r,g,b,a);
	vbquad_xyzcolortex.SetColor(3,r,g,b,a);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,v0->Map.x,1-v0->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,v1->Map.x,1-v1->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,v2->Map.x,1-v2->Map.y);
        vbquad_xyzcolortex.SetTexCoo(3,v3->Map.x,1-v3->Map.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,v0->Map);
        vbquad_xyzcolortex.SetTexCoo(1,v1->Map);
        vbquad_xyzcolortex.SetTexCoo(2,v2->Map);
        vbquad_xyzcolortex.SetTexCoo(3,v3->Map);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,v0->Map);
	vbquad_xyzcolortex.SetTexCoo(1,v1->Map);
	vbquad_xyzcolortex.SetTexCoo(2,v2->Map);
	vbquad_xyzcolortex.SetTexCoo(3,v3->Map);
#endif
    
	vbquad_xyzcolortex.SetVertex(0,v0->Stok);
	vbquad_xyzcolortex.SetVertex(1,v1->Stok);
	vbquad_xyzcolortex.SetVertex(2,v2->Stok);
	vbquad_xyzcolortex.SetVertex(3,v3->Stok);

	vbquad_xyzcolortex.UnlockVertices();
	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	if (set_alphatest)
	{
		effect_hll_diffuse_alphatest_model.setFloat("ALPHA",val_alphatest);
		effect_hll_diffuse_alphatest_model.setTexture("Tex",aT);
		effect_hll_diffuse_alphatest_model.setMatrixWorldViewProj("MODEL");
		SetEffect(&effect_hll_diffuse_alphatest_model);
	}
	else
	{
		effect_hll_diffuse_model.setTexture("Tex",aT);
		effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
		SetEffect(&effect_hll_diffuse_model);
	}

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

#endif

#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glColor4f(r,g,b,a);
		if (aT<0) glTexCoord2f(v0->Map.x,1-v0->Map.y); else glTexCoord2f(v0->Map.x,v0->Map.y);
		glVertex3f(v0->Stok.x,v0->Stok.y,v0->Stok.z);
		if (aT<0) glTexCoord2f(v1->Map.x,1-v1->Map.y); else glTexCoord2f(v1->Map.x,v1->Map.y);
		glVertex3f(v1->Stok.x,v1->Stok.y,v1->Stok.z);
		if (aT<0) glTexCoord2f(v2->Map.x,1-v2->Map.y); else glTexCoord2f(v2->Map.x,v2->Map.y);
		glVertex3f(v2->Stok.x,v2->Stok.y,v2->Stok.z);
		if (aT<0) glTexCoord2f(v3->Map.x,1-v3->Map.y); else glTexCoord2f(v3->Map.x,v3->Map.y);
		glVertex3f(v3->Stok.x,v3->Stok.y,v3->Stok.z);
	glEnd();
#else
	static GLfloat vertices[4*3];
		static GLfloat texvertices[4*2];
		static GLfloat colorvertices[4*4];

		colorvertices[0*4+0]=r;
		colorvertices[0*4+1]=g;
		colorvertices[0*4+2]=b;
		colorvertices[0*4+3]=a;

		colorvertices[1*4+0]=r;
		colorvertices[1*4+1]=g;
		colorvertices[1*4+2]=b;
		colorvertices[1*4+3]=a;

		colorvertices[2*4+0]=r;
		colorvertices[2*4+1]=g;
		colorvertices[2*4+2]=b;
		colorvertices[2*4+3]=a;

		colorvertices[3*4+0]=r;
		colorvertices[3*4+1]=g;
		colorvertices[3*4+2]=b;
		colorvertices[3*4+3]=a;

        if (aT<0)
        {
            texvertices[2*0+0]=v0->Map.x; texvertices[2*0+1]=1-v0->Map.y;
            texvertices[2*1+0]=v1->Map.x; texvertices[2*1+1]=1-v1->Map.y;
            texvertices[2*2+0]=v3->Map.x; texvertices[2*2+1]=1-v3->Map.y;
            texvertices[2*3+0]=v2->Map.x; texvertices[2*3+1]=1-v2->Map.y;
        }
        else
        {
            texvertices[2*0+0]=v0->Map.x; texvertices[2*0+1]=v0->Map.y;
            texvertices[2*1+0]=v1->Map.x; texvertices[2*1+1]=v1->Map.y;
            texvertices[2*2+0]=v3->Map.x; texvertices[2*2+1]=v3->Map.y;
            texvertices[2*3+0]=v2->Map.x; texvertices[2*3+1]=v2->Map.y;
        }

		vertices[3*0+0]=v0->Stok.x; vertices[3*0+1]=v0->Stok.y; vertices[3*0+2]=v0->Stok.z;
		vertices[3*1+0]=v1->Stok.x; vertices[3*1+1]=v1->Stok.y; vertices[3*1+2]=v1->Stok.z;
		vertices[3*2+0]=v3->Stok.x; vertices[3*2+1]=v3->Stok.y; vertices[3*2+2]=v3->Stok.z;
		vertices[3*3+0]=v2->Stok.x; vertices[3*3+1]=v2->Stok.y; vertices[3*3+2]=v2->Stok.z;

		glVertexPointer(3,GL_FLOAT,0,vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4,GL_FLOAT,0,colorvertices);
		glEnableClientState(GL_COLOR_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
		glTexCoordPointer(2,GL_FLOAT,0,texvertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;
	color=D3DCOLOR_COLORVALUE(r,g,b,a);
	D3DVB_XYZCOLORTEX->Lock(0,4*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);
	Vertices[0].x=v0->Stok.x; Vertices[0].y=v0->Stok.y; Vertices[0].z=v0->Stok.z;
	Vertices[0].xm=v0->Map.x; Vertices[0].ym=v0->Map.y;
	Vertices[0].color=color;
	Vertices[1].x=v1->Stok.x; Vertices[1].y=v1->Stok.y; Vertices[1].z=v1->Stok.z;
	Vertices[1].xm=v1->Map.x; Vertices[1].ym=v1->Map.y;
	Vertices[1].color=color;
	Vertices[2].x=v2->Stok.x; Vertices[2].y=v2->Stok.y; Vertices[2].z=v2->Stok.z;
	Vertices[2].xm=v2->Map.x; Vertices[2].ym=v2->Map.y;
	Vertices[2].color=color;
	Vertices[3].x=v3->Stok.x; Vertices[3].y=v3->Stok.y; Vertices[3].z=v3->Stok.z;
	Vertices[3].xm=v3->Map.x; Vertices[3].ym=v3->Map.y;
	Vertices[3].color=color;
	D3DVB_XYZCOLORTEX->Unlock();
	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(struct XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
#endif

#ifdef API3D_DIRECT3D9
	DWORD color;
	struct XYZCOLORTEX * Vertices;
	color=D3DCOLOR_COLORVALUE(r,g,b,a);
	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);
	Vertices[0].x=v0->Stok.x; Vertices[0].y=v0->Stok.y;	Vertices[0].z=v0->Stok.z;
	Vertices[0].xm=v0->Map.x; Vertices[0].ym=v0->Map.y;
	Vertices[0].color=color;
	Vertices[1].x=v1->Stok.x; Vertices[1].y=v1->Stok.y;	Vertices[1].z=v1->Stok.z;
	Vertices[1].xm=v1->Map.x; Vertices[1].ym=v1->Map.y;
	Vertices[1].color=color;
	Vertices[2].x=v2->Stok.x; Vertices[2].y=v2->Stok.y; Vertices[2].z=v2->Stok.z;
	Vertices[2].xm=v2->Map.x; Vertices[2].ym=v2->Map.y;
	Vertices[2].color=color;
	Vertices[3].x=v3->Stok.x; Vertices[3].y=v3->Stok.y; Vertices[3].z=v3->Stok.z;
	Vertices[3].xm=v3->Map.x; Vertices[3].ym=v3->Map.y;
	Vertices[3].color=color;
	D3DVB_XYZCOLORTEX->Unlock();
	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, 0, sizeof(struct XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,0,2 );
#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	vbquad_xyzcolortex.LockVertices();
	Vertices=(struct XYZCOLORTEX *)vbquad_xyzcolortex.pVertices;

	color=D3DCOLOR_COLORVALUE(r,g,b,a);

	Vertices[0].x=v0->Stok.x; Vertices[0].y=v0->Stok.y;	Vertices[0].z=v0->Stok.z;
	Vertices[0].xm=v0->Map.x; Vertices[0].ym=v0->Map.y;
	Vertices[0].color=color;
	Vertices[1].x=v1->Stok.x; Vertices[1].y=v1->Stok.y;	Vertices[1].z=v1->Stok.z;
	Vertices[1].xm=v1->Map.x; Vertices[1].ym=v1->Map.y;
	Vertices[1].color=color;
	Vertices[2].x=v2->Stok.x; Vertices[2].y=v2->Stok.y; Vertices[2].z=v2->Stok.z;
	Vertices[2].xm=v2->Map.x; Vertices[2].ym=v2->Map.y;
	Vertices[2].color=color;
	Vertices[3].x=v3->Stok.x; Vertices[3].y=v3->Stok.y; Vertices[3].z=v3->Stok.z;
	Vertices[3].xm=v3->Map.x; Vertices[3].ym=v3->Map.y;
	Vertices[3].color=color;

	vbquad_xyzcolortex.UnlockVertices();
	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	if (set_alphatest)
	{
		effect_hll_diffuse_alphatest_model.setFloat("ALPHA",val_alphatest);
		effect_hll_diffuse_alphatest_model.setTexture("Tex",aT);
		effect_hll_diffuse_alphatest_model.setMatrixWorldViewProj("MODEL");
		SetEffect(&effect_hll_diffuse_alphatest_model);
	}
	else
	{
		effect_hll_diffuse_model.setTexture("Tex",aT);
		effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
		SetEffect(&effect_hll_diffuse_model);
	}
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

#endif
}

void C3DAPI::Tri_MappingStok(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
		glColor4f(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
		
		if (aT<0) glTexCoord2f(f->v[0]->Map.x,1-f->v[0]->Map.y); else glTexCoord2f(f->v[0]->Map.x,f->v[0]->Map.y);
		glVertex3f(f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
		
		if (aT<0) glTexCoord2f(f->v[1]->Map.x,1-f->v[1]->Map.y); else glTexCoord2f(f->v[1]->Map.x,f->v[1]->Map.y);
		glVertex3f(f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
		
		if (aT<0) glTexCoord2f(f->v[2]->Map.x,1-f->v[2]->Map.y); else glTexCoord2f(f->v[2]->Map.x,f->v[2]->Map.y);
		glVertex3f(f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

	glEnd();	
#else
	static GLfloat vertices[4*3];
		static GLfloat texvertices[4*2];
		static GLfloat colorvertices[4*4];

		colorvertices[0*4+0]=f->Diffuse.r;
		colorvertices[0*4+1]=f->Diffuse.g;
		colorvertices[0*4+2]=f->Diffuse.b;
		colorvertices[0*4+3]=1.0f;

		colorvertices[1*4+0]=f->Diffuse.r;
		colorvertices[1*4+1]=f->Diffuse.g;
		colorvertices[1*4+2]=f->Diffuse.b;
		colorvertices[1*4+3]=1.0f;

		colorvertices[2*4+0]=f->Diffuse.r;
		colorvertices[2*4+1]=f->Diffuse.g;
		colorvertices[2*4+2]=f->Diffuse.b;
		colorvertices[2*4+3]=1.0f;

        if (aT<0)
        {
            texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=1-f->v[0]->Map.y;
            texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=1-f->v[1]->Map.y;
            texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=1-f->v[2]->Map.y;
        }
        else
        {
            texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=f->v[0]->Map.y;
            texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=f->v[1]->Map.y;
            texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=f->v[2]->Map.y;
        }

		vertices[3*0+0]=f->v[0]->Stok.x; vertices[3*0+1]=f->v[0]->Stok.y; vertices[3*0+2]=f->v[0]->Stok.z;
		vertices[3*1+0]=f->v[1]->Stok.x; vertices[3*1+1]=f->v[1]->Stok.y; vertices[3*1+2]=f->v[1]->Stok.z;
		vertices[3*2+0]=f->v[2]->Stok.x; vertices[3*2+1]=f->v[2]->Stok.y; vertices[3*2+2]=f->v[2]->Stok.z;

		glVertexPointer(3,GL_FLOAT,0,vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4,GL_FLOAT,0,colorvertices);
		glEnableClientState(GL_COLOR_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,texvertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	D3DVB_XYZCOLORTEX->Lock(0,3*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;

	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;


	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;

	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;


	Vertices[2].color=color;

	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(struct XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif


#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLORTEX * Vertices;

	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;

	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;

	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;

	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;

	Vertices[2].color=color;

	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(struct XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------
	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolortex.SetColor(1,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolortex.SetColor(2,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,1-f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,1-f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,1-f->v[2]->Map.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
#endif
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=color;

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;


	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Triv_Mapping(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
		glColor3f(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b);
		
		if (aT<0) glTexCoord2f(f->v[0]->Map.x,1-f->v[0]->Map.y); else glTexCoord2f(f->v[0]->Map.x,f->v[0]->Map.y);
		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);
		
		if (aT<0) glTexCoord2f(f->v[1]->Map.x,1-f->v[1]->Map.y); else glTexCoord2f(f->v[1]->Map.x,f->v[1]->Map.y);
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);

		if (aT<0) glTexCoord2f(f->v[2]->Map.x,1-f->v[2]->Map.y); else glTexCoord2f(f->v[2]->Map.x,f->v[2]->Map.y);
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);

	glEnd();
#else
        static GLfloat vertices[4*3];
		static GLfloat texvertices[4*2];
		static GLfloat colorvertices[4*4];

		colorvertices[0*4+0]=f->Diffuse.r;
		colorvertices[0*4+1]=f->Diffuse.g;
		colorvertices[0*4+2]=f->Diffuse.b;
		colorvertices[0*4+3]=f->Diffuse.a;

		colorvertices[1*4+0]=f->Diffuse.r;
		colorvertices[1*4+1]=f->Diffuse.g;
		colorvertices[1*4+2]=f->Diffuse.b;
		colorvertices[1*4+3]=f->Diffuse.a;

		colorvertices[2*4+0]=f->Diffuse.r;
		colorvertices[2*4+1]=f->Diffuse.g;
		colorvertices[2*4+2]=f->Diffuse.b;
		colorvertices[2*4+3]=f->Diffuse.a;

        if (aT<0)
        {
            texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=1-f->v[0]->Map.y;
            texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=1-f->v[1]->Map.y;
            texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=1-f->v[2]->Map.y;
        }
        else
        {
            texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=f->v[0]->Map.y;
            texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=f->v[1]->Map.y;
            texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=f->v[2]->Map.y;
        }

		vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
		vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
		vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

		glVertexPointer(3,GL_FLOAT,0,vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4,GL_FLOAT,0,colorvertices);
		glEnableClientState(GL_COLOR_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,texvertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glEnable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;
	DWORD color;
	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	D3DVB_XYZCOLORTEX->Lock(0,3*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;

	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;

	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;

	Vertices[2].color=color;

	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;
	DWORD color;
	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=color;

	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolortex.SetColor(1,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolortex.SetColor(2,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Calc.x,f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Calc.x,f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Calc.x,f->v[2]->Calc.y,f->v[2]->Calc.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,1-f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,1-f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,1-f->v[2]->Map.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
#endif
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=color;

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;


	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void C3DAPI::Triv_GouraudMapping(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor4f(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[0]->Map.x,1-f->v[0]->Map.y); else glTexCoord2f(f->v[0]->Map.x,f->v[0]->Map.y);
		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);
		
		glColor4f(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[1]->Map.x,1-f->v[1]->Map.y); else glTexCoord2f(f->v[1]->Map.x,f->v[1]->Map.y);
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);

		glColor4f(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[2]->Map.x,1-f->v[2]->Map.y); else glTexCoord2f(f->v[2]->Map.x,f->v[2]->Map.y);
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);

	glEnd();
#else
	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->v[0]->Diffuse.r;
	colorvertices[0*4+1]=f->v[0]->Diffuse.g;
	colorvertices[0*4+2]=f->v[0]->Diffuse.b;
	colorvertices[0*4+3]=f->v[0]->Diffuse.a;
	colorvertices[1*4+0]=f->v[1]->Diffuse.r;
	colorvertices[1*4+1]=f->v[1]->Diffuse.g;
	colorvertices[1*4+2]=f->v[1]->Diffuse.b;
	colorvertices[1*4+3]=f->v[1]->Diffuse.a;
	colorvertices[2*4+0]=f->v[2]->Diffuse.r;
	colorvertices[2*4+1]=f->v[2]->Diffuse.g;
	colorvertices[2*4+2]=f->v[2]->Diffuse.b;
	colorvertices[2*4+3]=f->v[2]->Diffuse.a;

    if (aT<0)
    {
        texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=1-f->v[0]->Map.y;
        texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=1-f->v[1]->Map.y;
        texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=1-f->v[2]->Map.y;
    }
    else
    {
        texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=f->v[0]->Map.y;
        texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=f->v[1]->Map.y;
        texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=f->v[2]->Map.y;
    }

	vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
	vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
	vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;

	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;

	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;

	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
	
	D3DVB_XYZCOLORTEX->Unlock();	

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );

	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1 );
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;

	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;

	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;

	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;

	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;

	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;

	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );

	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1 );
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
	vbquad_xyzcolortex.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
	vbquad_xyzcolortex.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Calc.x,f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Calc.x,f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Calc.x,f->v[2]->Calc.y,f->v[2]->Calc.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,1-f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,1-f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,1-f->v[2]->Map.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
#endif
    
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Triv_GouraudMappingStok(CFace *f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor4f(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[0]->Map.x,1-f->v[0]->Map.y); else glTexCoord2f(f->v[0]->Map.x,f->v[0]->Map.y);
		glVertex3f(f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
		
		glColor4f(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[1]->Map.x,1-f->v[1]->Map.y); else glTexCoord2f(f->v[1]->Map.x,f->v[1]->Map.y);
		glVertex3f(f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);

		glColor4f(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
		if (aT<0) glTexCoord2f(f->v[2]->Map.x,1-f->v[2]->Map.y); else glTexCoord2f(f->v[2]->Map.x,f->v[2]->Map.y);
		glVertex3f(f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

	glEnd();
#else
	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->v[0]->Diffuse.r;
	colorvertices[0*4+1]=f->v[0]->Diffuse.g;
	colorvertices[0*4+2]=f->v[0]->Diffuse.b;
	colorvertices[0*4+3]=f->v[0]->Diffuse.a;
	colorvertices[1*4+0]=f->v[1]->Diffuse.r;
	colorvertices[1*4+1]=f->v[1]->Diffuse.g;
	colorvertices[1*4+2]=f->v[1]->Diffuse.b;
	colorvertices[1*4+3]=f->v[1]->Diffuse.a;
	colorvertices[2*4+0]=f->v[2]->Diffuse.r;
	colorvertices[2*4+1]=f->v[2]->Diffuse.g;
	colorvertices[2*4+2]=f->v[2]->Diffuse.b;
	colorvertices[2*4+3]=f->v[2]->Diffuse.a;
    
    if (aT<0)
    {
        texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=1-f->v[0]->Map.y;
        texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=1-f->v[1]->Map.y;
        texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=1-f->v[2]->Map.y;
    }
    else
    {
        texvertices[2*0+0]=f->v[0]->Map.x; texvertices[2*0+1]=f->v[0]->Map.y;
        texvertices[2*1+0]=f->v[1]->Map.x; texvertices[2*1+1]=f->v[1]->Map.y;
        texvertices[2*2+0]=f->v[2]->Map.x; texvertices[2*2+1]=f->v[2]->Map.y;
    }

	vertices[3*0+0]=f->v[0]->Stok.x; vertices[3*0+1]=f->v[0]->Stok.y; vertices[3*0+2]=f->v[0]->Stok.z;
	vertices[3*1+0]=f->v[1]->Stok.x; vertices[3*1+1]=f->v[1]->Stok.y; vertices[3*1+2]=f->v[1]->Stok.z;
	vertices[3*2+0]=f->v[2]->Stok.x; vertices[3*2+1]=f->v[2]->Stok.y; vertices[3*2+2]=f->v[2]->Stok.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX, sizeof(XYZCOLORTEX) );

	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1 );
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLORTEX * Vertices;

	D3DVB_XYZCOLORTEX->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
	
	D3DVB_XYZCOLORTEX->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLORTEX );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLORTEX,0, sizeof(XYZCOLORTEX) );

	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1 );
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolortex.LockVertices();

	vbquad_xyzcolortex.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
	vbquad_xyzcolortex.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
	vbquad_xyzcolortex.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolortex.SetVertex(0,f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
	vbquad_xyzcolortex.SetVertex(1,f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
	vbquad_xyzcolortex.SetVertex(2,f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

#if defined(API3D_OPENGL20)
    if (aT<0)
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,1-f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,1-f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,1-f->v[2]->Map.y);
    }
    else
    {
        vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
        vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
    }
#else
	vbquad_xyzcolortex.SetTexCoo(0,f->v[0]->Map.x,f->v[0]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(1,f->v[1]->Map.x,f->v[1]->Map.y);
	vbquad_xyzcolortex.SetTexCoo(2,f->v[2]->Map.x,f->v[2]->Map.y);
#endif
    
	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);
	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;
	vbquad_xyzcolortex.nIndicesActif=3*2;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolortex.LockVertices();
	struct XYZCOLORTEX * Vertices=(struct XYZCOLORTEX *) vbquad_xyzcolortex.pVertices;

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].xm=f->v[0]->Map.x;
	Vertices[0].ym=f->v[0]->Map.y;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].xm=f->v[1]->Map.x;
	Vertices[1].ym=f->v[1]->Map.y;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].xm=f->v[2]->Map.x;
	Vertices[2].ym=f->v[2]->Map.y;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolortex.UnlockVertices();

	vbquad_xyzcolortex.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_model.setTexture("Tex",aT);
	effect_hll_diffuse_model.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_model);

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolortex.nIndicesActif=3*2;
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Tri_Gouraud(CFace * f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor4f(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);
		
		glColor4f(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);
		
		glColor4f(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);
	glEnd();
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->v[0]->Diffuse.r;
	colorvertices[0*4+1]=f->v[0]->Diffuse.g;
	colorvertices[0*4+2]=f->v[0]->Diffuse.b;
	colorvertices[0*4+3]=f->v[0]->Diffuse.a;
    
	colorvertices[1*4+0]=f->v[1]->Diffuse.r;
	colorvertices[1*4+1]=f->v[1]->Diffuse.g;
	colorvertices[1*4+2]=f->v[1]->Diffuse.b;
	colorvertices[1*4+3]=f->v[1]->Diffuse.a;
    
	colorvertices[2*4+0]=f->v[2]->Diffuse.r;
	colorvertices[2*4+1]=f->v[2]->Diffuse.g;
	colorvertices[2*4+2]=f->v[2]->Diffuse.b;
	colorvertices[2*4+3]=f->v[2]->Diffuse.a;

	vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
	vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
	vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,3*sizeof(XYZCOLOR),(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
	vbquad_xyzcolor.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
	vbquad_xyzcolor.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolor.SetVertex(0,f->v[0]->Calc.x,f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolor.SetVertex(1,f->v[1]->Calc.x,f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolor.SetVertex(2,f->v[2]->Calc.x,f->v[2]->Calc.y,f->v[2]->Calc.z);

    vbquad_xyzcolor.nIndicesActif=3;
    
	vbquad_xyzcolor.UnlockVertices();

	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Tri_GouraudStok(CFace * f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor4f(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
		glVertex3f(f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
		
		glColor4f(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
		glVertex3f(f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
		
		glColor4f(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);
		glVertex3f(f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);
	glEnd();
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->v[0]->Diffuse.r;
	colorvertices[0*4+1]=f->v[0]->Diffuse.g;
	colorvertices[0*4+2]=f->v[0]->Diffuse.b;
	colorvertices[0*4+3]=f->v[0]->Diffuse.a;
	colorvertices[1*4+0]=f->v[1]->Diffuse.r;
	colorvertices[1*4+1]=f->v[1]->Diffuse.g;
	colorvertices[1*4+2]=f->v[1]->Diffuse.b;
	colorvertices[1*4+3]=f->v[1]->Diffuse.a;
	colorvertices[2*4+0]=f->v[2]->Diffuse.r;
	colorvertices[2*4+1]=f->v[2]->Diffuse.g;
	colorvertices[2*4+2]=f->v[2]->Diffuse.b;
	colorvertices[2*4+3]=f->v[2]->Diffuse.a;

	vertices[3*0+0]=f->v[0]->Stok.x; vertices[3*0+1]=f->v[0]->Stok.y; vertices[3*0+2]=f->v[0]->Stok.z;
	vertices[3*1+0]=f->v[1]->Stok.x; vertices[3*1+1]=f->v[1]->Stok.y; vertices[3*1+2]=f->v[1]->Stok.z;
	vertices[3*2+0]=f->v[2]->Stok.x; vertices[3*2+1]=f->v[2]->Stok.y; vertices[3*2+2]=f->v[2]->Stok.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLES,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,3*sizeof(XYZCOLOR),(BYTE**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif
#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);
	vbquad_xyzcolor.SetColor(1,f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);
	vbquad_xyzcolor.SetColor(2,f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolor.SetVertex(0,f->v[0]->Stok.x,f->v[0]->Stok.y,f->v[0]->Stok.z);
	vbquad_xyzcolor.SetVertex(1,f->v[1]->Stok.x,f->v[1]->Stok.y,f->v[1]->Stok.z);
	vbquad_xyzcolor.SetVertex(2,f->v[2]->Stok.x,f->v[2]->Stok.y,f->v[2]->Stok.z);

    vbquad_xyzcolor.nIndicesActif=3;
    
	vbquad_xyzcolor.UnlockVertices();
	
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	Vertices[0].x=f->v[0]->Stok.x;
	Vertices[0].y=f->v[0]->Stok.y;
	Vertices[0].z=f->v[0]->Stok.z;
	Vertices[0].color=D3DCOLOR_COLORVALUE(f->v[0]->Diffuse.r,f->v[0]->Diffuse.g,f->v[0]->Diffuse.b,f->v[0]->Diffuse.a);

	Vertices[1].x=f->v[1]->Stok.x;
	Vertices[1].y=f->v[1]->Stok.y;
	Vertices[1].z=f->v[1]->Stok.z;
	Vertices[1].color=D3DCOLOR_COLORVALUE(f->v[1]->Diffuse.r,f->v[1]->Diffuse.g,f->v[1]->Diffuse.b,f->v[1]->Diffuse.a);

	Vertices[2].x=f->v[2]->Stok.x;
	Vertices[2].y=f->v[2]->Stok.y;
	Vertices[2].z=f->v[2]->Stok.z;
	Vertices[2].color=D3DCOLOR_COLORVALUE(f->v[2]->Diffuse.r,f->v[2]->Diffuse.g,f->v[2]->Diffuse.b,f->v[2]->Diffuse.a);

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Tri_Flat(CFace * f)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor3f(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b);

		glVertex3f(f->v[0]->Calc.x,-f->v[0]->Calc.y,-f->v[0]->Calc.z);
		
		glVertex3f(f->v[1]->Calc.x,-f->v[1]->Calc.y,-f->v[1]->Calc.z);
		
		glVertex3f(f->v[2]->Calc.x,-f->v[2]->Calc.y,-f->v[2]->Calc.z);
	glEnd();
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=f->Diffuse.r;
	colorvertices[0*4+1]=f->Diffuse.g;
	colorvertices[0*4+2]=f->Diffuse.b;
	colorvertices[0*4+3]=1.0f;
	colorvertices[1*4+0]=f->Diffuse.r;
	colorvertices[1*4+1]=f->Diffuse.g;
	colorvertices[1*4+2]=f->Diffuse.b;
	colorvertices[1*4+3]=1.0f;
	colorvertices[2*4+0]=f->Diffuse.r;
	colorvertices[2*4+1]=f->Diffuse.g;
	colorvertices[2*4+2]=f->Diffuse.b;
	colorvertices[2*4+3]=1.0f;

	vertices[3*0+0]=f->v[0]->Calc.x; vertices[3*0+1]=-f->v[0]->Calc.y; vertices[3*0+2]=-f->v[0]->Calc.z;
	vertices[3*1+0]=f->v[1]->Calc.x; vertices[3*1+1]=-f->v[1]->Calc.y; vertices[3*1+2]=-f->v[1]->Calc.z;
	vertices[3*2+0]=f->v[2]->Calc.x; vertices[3*2+1]=-f->v[2]->Calc.y; vertices[3*2+2]=-f->v[2]->Calc.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP,0,3);
#endif
#endif
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,3*sizeof(XYZCOLOR),(BYTE**)&Vertices,0);

	DWORD color;
	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=color;
	
	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	DWORD color;
	color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=-f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=color;
	
	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=-f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=-f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolor.SetColor(1,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);
	vbquad_xyzcolor.SetColor(2,f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	vbquad_xyzcolor.SetVertex(0,f->v[0]->Calc.x,f->v[0]->Calc.y,f->v[0]->Calc.z);
	vbquad_xyzcolor.SetVertex(1,f->v[1]->Calc.x,f->v[1]->Calc.y,f->v[1]->Calc.z);
	vbquad_xyzcolor.SetVertex(2,f->v[2]->Calc.x,f->v[2]->Calc.y,f->v[2]->Calc.z);

    vbquad_xyzcolor.nIndicesActif=3;
    
	vbquad_xyzcolor.UnlockVertices();

	
	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	
	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(f->Diffuse.r,f->Diffuse.g,f->Diffuse.b,f->Diffuse.a);

	Vertices[0].x=f->v[0]->Calc.x;
	Vertices[0].y=f->v[0]->Calc.y;
	Vertices[0].z=f->v[0]->Calc.z;
	Vertices[0].color=color;

	Vertices[1].x=f->v[1]->Calc.x;
	Vertices[1].y=f->v[1]->Calc.y;
	Vertices[1].z=f->v[1]->Calc.z;
	Vertices[1].color=color;

	Vertices[2].x=f->v[2]->Calc.x;
	Vertices[2].y=f->v[2]->Calc.y;
	Vertices[2].z=f->v[2]->Calc.z;
	Vertices[2].color=color;

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;
#endif
}

void C3DAPI::Quad_Flat(CVector A,CVector B,CVector C,CVector D,float r,float g,float b)
{
    if (!Active) return;

	if (nMultiQuads>0) Flush();
	if (nMultiQuadsXYZ>0) Flush();
    
#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,r,g,b,1);
	vbquad_xyzcolor.SetColor(1,r,g,b,1);
	vbquad_xyzcolor.SetColor(2,r,g,b,1);
	vbquad_xyzcolor.SetColor(3,r,g,b,1);

	vbquad_xyzcolor.SetVertex(0,A);
	vbquad_xyzcolor.SetVertex(1,B);
	vbquad_xyzcolor.SetVertex(2,C);
	vbquad_xyzcolor.SetVertex(3,D);
    
    vbquad_xyzcolor.nIndicesActif=3*2;

	vbquad_xyzcolor.UnlockVertices();
	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

#endif

#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glColor4f(r,g,b,1);
		glVertex3f(A.x,-A.y,-A.z);
		glVertex3f(B.x,-B.y,-B.z);
		glVertex3f(C.x,-C.y,-C.z);
		glVertex3f(D.x,-D.y,-D.z);
	glEnd();	
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=1.0f;
	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=1.0f;
	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=1.0f;
	colorvertices[3*4+0]=r;
	colorvertices[3*4+1]=g;
	colorvertices[3*4+2]=b;
	colorvertices[3*4+3]=1.0f;

	vertices[3*0+0]=A.x; vertices[3*0+1]=-A.y; vertices[3*0+2]=-A.z;
	vertices[3*0+0]=B.x; vertices[3*0+1]=-B.y; vertices[3*0+2]=-B.z;
	vertices[3*0+0]=C.x; vertices[3*0+1]=-C.y; vertices[3*0+2]=-C.z;
	vertices[3*0+0]=D.x; vertices[3*0+1]=-D.y; vertices[3*0+2]=-D.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	struct XYZCOLOR * Vertices;

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	D3DVB_XYZCOLOR->Lock(0,4*sizeof(XYZCOLORTEX),(BYTE**)&Vertices,0);

	Vertices[0].x=A.x; Vertices[0].y=A.y; Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x; Vertices[1].y=B.y; Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x; Vertices[2].y=C.y; Vertices[2].z=C.z;
	Vertices[2].color=color;

	Vertices[3].x=D.x; Vertices[3].y=D.y; Vertices[3].z=D.z;
	Vertices[3].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(struct XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,0,2);

#endif

#ifdef API3D_DIRECT3D9
	DWORD color;
	struct XYZCOLOR * Vertices;

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	Vertices[0].x=A.x; Vertices[0].y=A.y; Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x; Vertices[1].y=B.y; Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x; Vertices[2].y=C.y; Vertices[2].z=C.z;
	Vertices[2].color=color;

	Vertices[3].x=D.x; Vertices[3].y=D.y; Vertices[3].z=D.z;
	Vertices[3].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(struct XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,0,2);
#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	DWORD color;
	struct XYZCOLOR * Vertices;

	vbquad_xyzcolor.LockVertices();
	Vertices=(struct XYZCOLOR *)vbquad_xyzcolor.pVertices;

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	Vertices[0].x=A.x; Vertices[0].y=A.y; Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x; Vertices[1].y=B.y; Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x; Vertices[2].y=C.y; Vertices[2].z=C.z;
	Vertices[2].color=color;

	Vertices[3].x=D.x; Vertices[3].y=D.y; Vertices[3].z=D.z;
	Vertices[3].color=color;

	vbquad_xyzcolor.UnlockVertices();
	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

#endif

}


void C3DAPI::Tri_Flat(CVector A,CVector B,CVector C,float r,float g,float b)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor3f(r,g,b);

		glVertex3f(A.x,-A.y,-A.z);		
		glVertex3f(B.x,-B.y,-B.z);
		glVertex3f(C.x,-C.y,-C.z);
	glEnd();
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=1.0f;
	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=1.0f;
	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=1.0f;

	vertices[3*0+0]=A.x; vertices[3*0+1]=-A.y; vertices[3*0+2]=-A.z;
	vertices[3*0+0]=B.x; vertices[3*0+1]=-B.y; vertices[3*0+2]=-B.z;
	vertices[3*0+0]=C.x; vertices[3*0+1]=-C.y; vertices[3*0+2]=-C.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP,0,3);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;
	DWORD color;

	D3DVB_XYZCOLOR->Lock(0,3*sizeof(XYZCOLOR),(BYTE**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;
	DWORD color;

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif
    
#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,r,g,b,1.0f);
	vbquad_xyzcolor.SetColor(1,r,g,b,1.0f);
	vbquad_xyzcolor.SetColor(2,r,g,b,1.0f);

	vbquad_xyzcolor.SetVertex(0,A);
	vbquad_xyzcolor.SetVertex(1,B);
	vbquad_xyzcolor.SetVertex(2,C);

    vbquad_xyzcolor.nIndicesActif=3;

	vbquad_xyzcolor.UnlockVertices();

	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	effect_hll_diffuse_nomap.setTexture("Tex",aT);
	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(r,g,b,1);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;
#endif

}

void C3DAPI::Tri_Flat(CVector A,CVector B,CVector C,float r,float g,float b,float a)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);

		glColor4f(r,g,b,a);

		glVertex3f(A.x,-A.y,-A.z);		
		glVertex3f(B.x,-B.y,-B.z);
		glVertex3f(C.x,-C.y,-C.z);
	glEnd();
#else
	glDisable(GL_TEXTURE_2D);
	static GLfloat vertices[4*3];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=a;
	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=a;
	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=a;

	vertices[3*0+0]=A.x; vertices[3*0+1]=-A.y; vertices[3*0+2]=-A.z;
	vertices[3*0+0]=B.x; vertices[3*0+1]=-B.y; vertices[3*0+2]=-B.z;
	vertices[3*0+0]=C.x; vertices[3*0+1]=-C.y; vertices[3*0+2]=-C.z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP,0,3);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;
	DWORD color;

	D3DVB_XYZCOLOR->Lock(0,3*sizeof(XYZCOLOR),(BYTE**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(r,g,b,a);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetVertexShader( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct XYZCOLOR * Vertices;
	DWORD color;

	D3DVB_XYZCOLOR->Lock(0,0,(void**)&Vertices,0);

	color=D3DCOLOR_COLORVALUE(r,g,b,a);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	D3DVB_XYZCOLOR->Unlock();

	D3DDevice->SetFVF( D3DFVF_XYZCOLOR );
	D3DDevice->SetStreamSource( 0, D3DVB_XYZCOLOR,0, sizeof(XYZCOLOR) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,1);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,r,g,b,a);
	vbquad_xyzcolor.SetColor(1,r,g,b,a);
	vbquad_xyzcolor.SetColor(2,r,g,b,a);

	vbquad_xyzcolor.SetVertex(0,A);
	vbquad_xyzcolor.SetVertex(1,B);
	vbquad_xyzcolor.SetVertex(2,C);

    vbquad_xyzcolor.nIndicesActif=3;

	vbquad_xyzcolor.UnlockVertices();

	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;
	
	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	DWORD color=D3DCOLOR_COLORVALUE(r,g,b,a);

	Vertices[0].x=A.x;
	Vertices[0].y=A.y;
	Vertices[0].z=A.z;
	Vertices[0].color=color;

	Vertices[1].x=B.x;
	Vertices[1].y=B.y;
	Vertices[1].z=B.z;
	Vertices[1].color=color;

	Vertices[2].x=C.x;
	Vertices[2].y=C.y;
	Vertices[2].z=C.z;
	Vertices[2].color=color;

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrixWorldViewProj("MODEL");
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;
#endif

}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Affichage triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Tri_Flat0(float x1,float y1,float x2,float y2,float x3,float y3)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#endif


#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct FVFFLAT Vertices[3];
	DWORD color;

	D3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);

	color=D3DCOLOR_COLORVALUE(1,0,0,0);

	Vertices[0].color=color;
	Vertices[0].x=x1;
	Vertices[0].y=y1;
	Vertices[0].rhw=(float) 1.0f/(Back);
	Vertices[0].z=0.99f;

	Vertices[1].color=color;
	Vertices[1].x=x2;
	Vertices[1].y=y2;
	Vertices[1].rhw=(float) 1.0f/(Back);
	Vertices[1].z=0.99f;
	
	Vertices[2].color=color;
	Vertices[2].x=x3;
	Vertices[2].y=y3;
	Vertices[2].rhw=(float) 1.0f/(Back);
	Vertices[2].z=0.99f;

	D3DDevice->SetVertexShader( D3DFVF_FVFFLAT );
	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST,1,Vertices ,3*sizeof(FVFFLAT));

#endif
#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	struct FVFFLAT Vertices[3];
	DWORD color;

	D3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);

	color=D3DCOLOR_COLORVALUE(1,0,0,0);

	Vertices[0].color=color;
	Vertices[0].x=x1;
	Vertices[0].y=y1;
	Vertices[0].rhw=(float) 1.0f/(Back);
	Vertices[0].z=0.99f;

	Vertices[1].color=color;
	Vertices[1].x=x2;
	Vertices[1].y=y2;
	Vertices[1].rhw=(float) 1.0f/(Back);
	Vertices[1].z=0.99f;
	
	Vertices[2].color=color;
	Vertices[2].x=x3;
	Vertices[2].y=y3;
	Vertices[2].rhw=(float) 1.0f/(Back);
	Vertices[2].z=0.99f;

	D3DDevice->SetFVF( D3DFVF_FVFFLAT );
	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST,1,Vertices ,3*sizeof(FVFFLAT));

#endif
}


void C3DAPI::Tri_Gouraud2D(float x1,float y1,float x2,float y2,float x3,float y3,CRGBA c1,CRGBA c2,CRGBA c3)
{
    if (!Active) return;

#if defined(API3D_OPENGL20)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------

	vbquad_xyzcolor.LockVertices();

	vbquad_xyzcolor.SetColor(0,c1.r,c1.g,c1.b,1);
	vbquad_xyzcolor.SetColor(1,c2.r,c2.g,c2.b,1);
	vbquad_xyzcolor.SetColor(2,c3.r,c3.g,c3.b,1);
	float zz=0.5f;
	vbquad_xyzcolor.SetVertex(0,x1-VIEWPORT.a[3][0],y1-VIEWPORT.a[3][1],zz);
	vbquad_xyzcolor.SetVertex(1,x2-VIEWPORT.a[3][0],y2-VIEWPORT.a[3][1],zz);
	vbquad_xyzcolor.SetVertex(2,x3-VIEWPORT.a[3][0],y3-VIEWPORT.a[3][1],zz);

    vbquad_xyzcolor.nIndicesActif=3;

	vbquad_xyzcolor.UnlockVertices();

	whileFlush=true;
	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

    effect_hll_diffuse_nomap.setMatrix("MODEL",ProjOrtho);
	SetEffect(&effect_hll_diffuse_nomap);
	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();
	ActualVertexBuffer=actual;
	ActualEffect=eff;
	whileFlush=false;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	vbquad_xyzcolor.LockVertices();
	struct XYZCOLOR * Vertices=(struct XYZCOLOR *) vbquad_xyzcolor.pVertices;

	Vertices[0].x=x1-VIEWPORT.a[3][0];
	Vertices[0].y=y1-VIEWPORT.a[3][1];
	Vertices[0].z=0.5f;
	Vertices[0].color=D3DCOLOR_COLORVALUE(c1.r,c1.g,c1.b,1);

	Vertices[1].x=x2-VIEWPORT.a[3][0];
	Vertices[1].y=y2-VIEWPORT.a[3][1];
	Vertices[1].z=0.5f;
	Vertices[1].color=D3DCOLOR_COLORVALUE(c2.r,c2.g,c2.b,1);

	Vertices[2].x=x3-VIEWPORT.a[3][0];
	Vertices[2].y=y3-VIEWPORT.a[3][1];
	Vertices[2].z=0.5f;
	Vertices[2].color=D3DCOLOR_COLORVALUE(c3.r,c3.g,c3.b,1);

	vbquad_xyzcolor.UnlockVertices();

	vbquad_xyzcolor.nIndicesActif=3;

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_diffuse_nomap.setMatrix("MODEL",ProjOrtho);
	SetEffect(&effect_hll_diffuse_nomap);

	SetVertexBuffer(&vbquad_xyzcolor);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolor.nIndicesActif=3*2;

#else

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL) && !defined(API3D_DIRECT3D12)
//-------------------------------------------------------------------------------------------DIRECT3D AND METAL -----	
	struct CUSTOMVERTEX01 Vertices[3];

	Vertices[0].color=D3DCOLOR_COLORVALUE(c1.r,c1.g,c1.b,1);
	Vertices[0].x=x1;
	Vertices[0].y=y1;
	Vertices[0].z=0.5f;
	Vertices[0].rhw=1.0f;

	Vertices[1].color=D3DCOLOR_COLORVALUE(c2.r,c2.g,c2.b,1);
	Vertices[1].x=x2;
	Vertices[1].y=y2;
	Vertices[1].z=0.5f;
	Vertices[1].rhw=1.0f;
	
	Vertices[2].color=D3DCOLOR_COLORVALUE(c3.r,c3.g,c3.b,1);
	Vertices[2].x=x3;
	Vertices[2].y=y3;
	Vertices[2].z=0.5f;
	Vertices[2].rhw=1.0f;

#ifdef API3D_DIRECT3D
	D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX01 );
#else
#endif

	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST,1,Vertices ,sizeof(struct CUSTOMVERTEX01));

#endif
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		Draw Quad

		void C3DAPIBASE::Quad(int x,int y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b)

		x,y : co in screen coord.
		sizex,sizey: lengths of quad
		x1,y1,x2,y2 : mapping coo
		r,g,b : color
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::Quad0(int x,int y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b)
{
    if (!Active) return;

	if (nMultiQuadsXYZ>0) Flush();
    
#if defined(API3D_METAL)||defined(API3D_DIRECT3D12)
    //--------------------------------------------------------------------------------------------DIRECT3D AND METAL -----
    
    float _x=decals_quads_x+x-VIEWPORT.a[3][0];
    float _y=decals_quads_y+y-VIEWPORT.a[3][1];
    float zz=1.0f;
    if (nMultiQuads>MAX_VBQUADS-1) Flush();
    
    int adr=nMultiQuads*4;
    
    vbquads.SetColor(adr+0,r,g,b,1);
    vbquads.SetColor(adr+1,r,g,b,1);
    vbquads.SetColor(adr+2,r,g,b,1);
    vbquads.SetColor(adr+3,r,g,b,1);
    
    vbquads.SetVertex(adr+0,_x,_y,zz);
    vbquads.SetVertex(adr+1,_x+sizex,_y,zz);
    vbquads.SetVertex(adr+2,_x+sizex,_y+sizey,zz);
    vbquads.SetVertex(adr+3,_x,_y+sizey,zz);
    
    vbquads.SetTexCoo(adr+0,x1,y1);
    vbquads.SetTexCoo(adr+1,x2,y1);
    vbquads.SetTexCoo(adr+2,x2,y2);
    vbquads.SetTexCoo(adr+3,x1,y2);
    
    nMultiQuads++;
    MultiQuadsZbuffer=1;
    
    CMatrix back=ProjOrtho;
    ProjOrtho.a[2][3]=1.0f; ProjOrtho.a[3][3]=0.0f;
    Flush();
    ProjOrtho=back;
#endif

#if defined(API3D_OPENGL20)
//-------------------------------------------------------------------------------------------OPEN GL AND METAL -------
	if (((SelectedRenderTarget>=0)||(aT<0))&&(!SecondarySurface)) return;

	float _x=decals_quads_x-VIEWPORT.a[3][0]+x;
	float _y=decals_quads_y-VIEWPORT.a[3][1]+y;
	float zz=0.9f;
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	vbquads.SetColor(adr+0,r,g,b,1.0f);
	vbquads.SetColor(adr+1,r,g,b,1.0f);
	vbquads.SetColor(adr+2,r,g,b,1.0f);
	vbquads.SetColor(adr+3,r,g,b,1.0f);

	vbquads.SetVertex(adr+0,_x,_y,zz);
	vbquads.SetVertex(adr+1,_x+sizex,_y,zz);
	vbquads.SetVertex(adr+2,_x+sizex,_y+sizey,zz);
	vbquads.SetVertex(adr+3,_x,_y+sizey,zz);

    if (aT<0)
    {
        vbquads.SetTexCoo(adr+0,x1,1-y1);
        vbquads.SetTexCoo(adr+1,x2,1-y1);
        vbquads.SetTexCoo(adr+2,x2,1-y2);
        vbquads.SetTexCoo(adr+3,x1,1-y2);
    }
    else
    {
        vbquads.SetTexCoo(adr+0,x1,y1);
        vbquads.SetTexCoo(adr+1,x2,y1);
        vbquads.SetTexCoo(adr+2,x2,y2);
        vbquads.SetTexCoo(adr+3,x1,y2);
    }

	nMultiQuads++;
	MultiQuadsZbuffer=1;
    
    Flush();
#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
	if (((SelectedRenderTarget>=0)||(aT<0))&&(!SecondarySurface)) return;

	float _x=decals_quads_x+x-VIEWPORT.a[3][0];
	float _y=decals_quads_y+y-VIEWPORT.a[3][1];
	float zz=0.999f;

	if (nMultiQuads>0) Flush();

	int adr=nMultiQuads<<2;

	DWORD c=D3DCOLOR_COLORVALUE(r,g,b,1.0f);

	MultiQuads[(adr)+0].color=c;
	MultiQuads[(adr)+0].x=(float) _x;
	MultiQuads[(adr)+0].y=(float) _y;
	MultiQuads[(adr)+0].xm=x1;
	MultiQuads[(adr)+0].ym=y1;
	MultiQuads[(adr)+0].z=zz;

	MultiQuads[(adr)+1].color=c;
	MultiQuads[(adr)+1].x=(float) (_x+sizex);
	MultiQuads[(adr)+1].y=(float) _y;
	MultiQuads[(adr)+1].xm=x2;
	MultiQuads[(adr)+1].ym=y1;
	MultiQuads[(adr)+1].z=zz;

	MultiQuads[(adr)+2].color=c;
	MultiQuads[(adr)+2].x=(float) (_x+sizex);
	MultiQuads[(adr)+2].y=(float) (_y+sizey);
	MultiQuads[(adr)+2].xm=x2;
	MultiQuads[(adr)+2].ym=y2;
	MultiQuads[(adr)+2].z=zz;

	MultiQuads[(adr)+3].color=c;
	MultiQuads[(adr)+3].x=(float) (_x);
	MultiQuads[(adr)+3].y=(float) (_y+sizey);
	MultiQuads[(adr)+3].xm=x1;
	MultiQuads[(adr)+3].ym=y2;
	MultiQuads[(adr)+3].z=zz;

	nMultiQuads++;

	MultiQuadsZbuffer=1;

	Flush();
#else

#ifdef API3D_OPENGL
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES
	if (((SelectedRenderTarget>=0)||(aT<0))&&(!SecondarySurface)) return;

	glEnable(GL_TEXTURE_2D);


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrtho(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrtho(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);

	glColor3f(r,g,b);
	if (aT<0)
	{
		if (SelectedRenderTarget>=0)
		{
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),1.0f);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),1.0f);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),1.0f);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),1.0f);
		}
		else
		{		
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-y),1.0f);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),1.0f);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),1.0f);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),1.0f);
		}	
	}
	else
	{
		if (SelectedRenderTarget>=0)
		{
			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),1.0f);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),1.0f);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),1.0f);

			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),1.0f);
		}
		else
		{		
			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-y),1.0f);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),1.0f);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),1.0f);

			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),1.0f);
		}
	}
	
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
#else
	if (((SelectedRenderTarget>=0)||(aT<0))&&(!SecondarySurface)) return;

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrthof(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrthof(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	static GLfloat vertices[4*2];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=1.0f;

	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=1.0f;

	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=1.0f;

	colorvertices[3*4+0]=r;
	colorvertices[3*4+1]=g;
	colorvertices[3*4+2]=b;
	colorvertices[3*4+3]=1.0f;

	texvertices[2*0+0]=x1; texvertices[2*0+1]=y1;
	vertices[2*0+0]=(float) x; vertices[2*0+1]=(float) (SCREEN_Y-y);

	texvertices[2*1+0]=x2; texvertices[2*1+1]=y1;
	vertices[2*1+0]=(float) (x+sizex); vertices[2*1+1]=(float) (SCREEN_Y-y);

	texvertices[2*2+0]=x1; texvertices[2*2+1]=y2;
	vertices[2*2+0]=(float) x; vertices[2*2+1]=(float) (SCREEN_Y-(y+sizey));

	texvertices[2*3+0]=x2; texvertices[2*3+1]=y2;
	vertices[2*3+0]=(float) (x+sizex); vertices[2*3+1]=(float) (SCREEN_Y-(y+sizey));

	glVertexPointer(2,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
#endif
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct CUSTOMVERTEX0 Vertices[4];

	Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,1);
	Vertices[1].color=Vertices[0].color;
	Vertices[2].color=Vertices[0].color;
	Vertices[3].color=Vertices[0].color;

	Vertices[0].x=(float) x;
	Vertices[0].y=(float) y;
	Vertices[0].xm=x1;
	Vertices[0].ym=y1;
	Vertices[0].rhw=(float) 1/(Back);
	Vertices[0].z=0.99999f;

	Vertices[1].x=(float) (x+sizex);
	Vertices[1].y=(float) y;
	Vertices[1].xm=x2;
	Vertices[1].ym=y1;
	Vertices[1].rhw=Vertices[0].rhw;
	Vertices[1].z=Vertices[0].z;

	Vertices[2].x=(float) (x+sizex);
	Vertices[2].y=(float) (y+sizey);
	Vertices[2].xm=x2;
	Vertices[2].ym=y2;
	Vertices[2].rhw=Vertices[0].rhw;
	Vertices[2].z=Vertices[0].z;
	
	Vertices[3].x=(float) (x);
	Vertices[3].y=(float) (y+sizey);
	Vertices[3].xm=x1;
	Vertices[3].ym=y2;
	Vertices[3].rhw=Vertices[0].rhw;
	Vertices[3].z=Vertices[0].z;
	
	D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX0 );
	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,2,Vertices ,sizeof(CUSTOMVERTEX0));
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	struct CUSTOMVERTEX0 Vertices[4];

	Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,1);
	Vertices[1].color=Vertices[0].color;
	Vertices[2].color=Vertices[0].color;
	Vertices[3].color=Vertices[0].color;

	Vertices[0].x=(float) x;
	Vertices[0].y=(float) y;
	Vertices[0].xm=x1;
	Vertices[0].ym=y1;
	Vertices[0].rhw=(float) 1/(Back);
	Vertices[0].z=0.99999f;

	Vertices[1].x=(float) (x+sizex);
	Vertices[1].y=(float) y;
	Vertices[1].xm=x2;
	Vertices[1].ym=y1;
	Vertices[1].rhw=Vertices[0].rhw;
	Vertices[1].z=Vertices[0].z;

	Vertices[2].x=(float) (x+sizex);
	Vertices[2].y=(float) (y+sizey);
	Vertices[2].xm=x2;
	Vertices[2].ym=y2;
	Vertices[2].rhw=Vertices[0].rhw;
	Vertices[2].z=Vertices[0].z;
	
	Vertices[3].x=(float) (x);
	Vertices[3].y=(float) (y+sizey);
	Vertices[3].xm=x1;
	Vertices[3].ym=y2;
	Vertices[3].rhw=Vertices[0].rhw;
	Vertices[3].z=Vertices[0].z;
	
	D3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX0 );
	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,2,Vertices ,sizeof(CUSTOMVERTEX0));

#endif
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		Draw Quad with ZBuffer

		void C3DAPIBASE::Quad(int x,int y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b)

		x,y : co in screen coord.
		sizex,sizey: lengths of quad
		x1,y1,x2,y2 : mapping coo
		r,g,b : color
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::QuadZ(float z,float w,float x0,float y0,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b)
{
    if (!Active) return;
    
    if (x0+sizex<X1base) return;
    if (y0+sizey<Y1base) return;
    if (x0>X2base) return;
    if (y0>Y2base) return;
    
    if (sizex==0) return;
    if (sizey==0) return;

	if (nMultiQuadsXYZ>0) Flush();

#ifdef API3D_OPENGL20
//--------------------------------------------------------------------------------------------------OPEN GL 2.0 ------
	float x=decals_quads_x-VIEWPORT.a[3][0]+x0;
    float _x=x+sizex;

	if (MultiQuadsZbuffer==0) Flush();
	else
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int n=nMultiQuads<<2;

	float zz=0.5f-z/2;

	register int adr=(n<<2);

	vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=1.0f;
	vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=1.0f;
	vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=1.0f;
	vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=1.0f;

	adr=(n<<1) +n;

	if (SelectedRenderTarget==-1)
    {
        float y=decals_quads_y-VIEWPORT.a[3][1]+y0;
        
		vbquads.BVertex_Array[adr++]=x;
		vbquads.BVertex_Array[adr++]=y;
		vbquads.BVertex_Array[adr++]=zz;
		
		vbquads.BVertex_Array[adr++]=_x;
		vbquads.BVertex_Array[adr++]=y;
		vbquads.BVertex_Array[adr++]=zz;
		
		vbquads.BVertex_Array[adr++]=_x;
		vbquads.BVertex_Array[adr++]=y+sizey;
		vbquads.BVertex_Array[adr++]=zz;
		
		vbquads.BVertex_Array[adr++]=x;
		vbquads.BVertex_Array[adr++]=y+sizey;
		vbquads.BVertex_Array[adr++]=zz;
    }
    else
    {
        
        float _y1=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-y0;
        float _y2=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-(y0+sizey);
        
		vbquads.BVertex_Array[adr++]=x;
		vbquads.BVertex_Array[adr++]=_y1;
		vbquads.BVertex_Array[adr++]=zz;
		
		vbquads.BVertex_Array[adr++]=_x;
		vbquads.BVertex_Array[adr++]=_y1;
		vbquads.BVertex_Array[adr++]=zz;
		
		vbquads.BVertex_Array[adr++]=_x;
		vbquads.BVertex_Array[adr++]=_y2;
		vbquads.BVertex_Array[adr++]=zz;
        
		vbquads.BVertex_Array[adr++]=x;
		vbquads.BVertex_Array[adr++]=_y2;
		vbquads.BVertex_Array[adr++]=zz;
    }

	adr=(n<<1);

    if (aT<0)
    {
		vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=1-y1;
		vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=1-y1;
		vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=1-y2;
		vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=1-y2;
    }
    else
    {
		vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=y1;
		vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=y1;
		vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=y2;
		vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=y2;
    }

	nMultiQuads++;
	MultiQuadsZbuffer=1;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	DWORD c=D3DCOLOR_COLORVALUE(r,g,b,1.0f);

	MultiQuads[(adr)+0].color=c;
	MultiQuads[(adr)+0].x=(float) x;
	MultiQuads[(adr)+0].y=(float) y;
	MultiQuads[(adr)+0].xm=x1;
	MultiQuads[(adr)+0].ym=y1;
	MultiQuads[(adr)+0].z=z;

	MultiQuads[(adr)+1].color=c;
	MultiQuads[(adr)+1].x=(float) (x+sizex);
	MultiQuads[(adr)+1].y=(float) y;
	MultiQuads[(adr)+1].xm=x2;
	MultiQuads[(adr)+1].ym=y1;
	MultiQuads[(adr)+1].z=z;

	MultiQuads[(adr)+2].color=c;
	MultiQuads[(adr)+2].x=(float) (x+sizex);
	MultiQuads[(adr)+2].y=(float) (y+sizey);
	MultiQuads[(adr)+2].xm=x2;
	MultiQuads[(adr)+2].ym=y2;
	MultiQuads[(adr)+2].z=z;

	MultiQuads[(adr)+3].color=c;
	MultiQuads[(adr)+3].x=(float) (x);
	MultiQuads[(adr)+3].y=(float) (y+sizey);
	MultiQuads[(adr)+3].xm=x1;
	MultiQuads[(adr)+3].ym=y2;
	MultiQuads[(adr)+3].z=z;

	nMultiQuads++;
	MultiQuadsZbuffer=1;
#endif

#if !defined(API3D_OPENGL20)&&!defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)

#ifdef API3D_OPENGL
	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrtho(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrtho(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);

	glColor4f(r,g,b,1.0f);
	if (aT<0)
	{
		if (SelectedRenderTarget>=0)
		{
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);
		}
		else
		{
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),z);
		}
	}
	else
	{
		if (SelectedRenderTarget>=0)
		{		
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);
		}
		else
		{			
			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),z);

			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),z);	
		}
	}
	
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#else

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=1.0f;

	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=1.0f;

	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=1.0f;

	colorvertices[3*4+0]=r;
	colorvertices[3*4+1]=g;
	colorvertices[3*4+2]=b;
	colorvertices[3*4+3]=1.0f;

	texvertices[2*0+0]=x1; texvertices[2*0+1]=y1;
	vertices[3*0+0]=(float) x; vertices[3*0+1]=(float) (SCREEN_Y-y); vertices[3*0+2]=z;

	texvertices[2*1+0]=x2; texvertices[2*1+1]=y1;
	vertices[3*1+0]=(float) (x+sizex); vertices[3*1+1]=(float) (SCREEN_Y-y); vertices[3*1+2]=z;

	texvertices[2*2+0]=x1; texvertices[2*2+1]=y2;
	vertices[3*2+0]=(float) x; vertices[3*2+1]=(float) (SCREEN_Y-(y+sizey)); vertices[3*2+2]=z;

	texvertices[2*3+0]=x2; texvertices[2*3+1]=y2;
	vertices[3*3+0]=(float) (x+sizex); vertices[3*3+1]=(float) (SCREEN_Y-(y+sizey)); vertices[3*3+2]=z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#endif
#endif	

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;

	if (MultiQuads)
	{
		if (nMultiQuads>MAX_VBQUADS-1) Flush();
		int adr=nMultiQuads*3;

		DWORD c=D3DCOLOR_COLORVALUE(r,g,b,1.0f);

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) x;
		MultiQuads[(adr)+0].y=(float) y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;

		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) (x+sizex);
		MultiQuads[(adr)+1].y=(float) y;
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y1;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) (x+sizex);
		MultiQuads[(adr)+2].y=(float) (y+sizey);
		MultiQuads[(adr)+2].xm=x2;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
		adr+=3;

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) x;
		MultiQuads[(adr)+0].y=(float) y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;

		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) (x+sizex);
		MultiQuads[(adr)+1].y=(float) (y+sizey);
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y2;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) (x);
		MultiQuads[(adr)+2].y=(float) (y+sizey);
		MultiQuads[(adr)+2].xm=x1;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
	}
	else
	{
		struct CUSTOMVERTEX0 Vertices[4];
		
		Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,1.0f);
		Vertices[1].color=Vertices[0].color;
		Vertices[2].color=Vertices[0].color;
		Vertices[3].color=Vertices[0].color;


		Vertices[0].x=(float) x;
		Vertices[0].y=(float) y;
		Vertices[0].xm=x1;
		Vertices[0].ym=y1;
		Vertices[0].rhw=w;
		Vertices[0].z=z;

		Vertices[1].x=(float) (x+sizex);
		Vertices[1].y=(float) y;
		Vertices[1].xm=x2;
		Vertices[1].ym=y1;
		Vertices[1].rhw=Vertices[0].rhw;
		Vertices[1].z=Vertices[0].z;

		Vertices[2].x=(float) (x+sizex);
		Vertices[2].y=(float) (y+sizey);
		Vertices[2].xm=x2;
		Vertices[2].ym=y2;
		Vertices[2].rhw=Vertices[0].rhw;
		Vertices[2].z=Vertices[0].z;

		Vertices[3].x=(float) (x);
		Vertices[3].y=(float) (y+sizey);
		Vertices[3].xm=x1;
		Vertices[3].ym=y2;
		Vertices[3].rhw=Vertices[0].rhw;
		Vertices[3].z=Vertices[0].z;

		D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX0 );
		D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,2,Vertices ,sizeof(CUSTOMVERTEX0));
	}
#endif

#if defined(API3D_DIRECT3D9)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//--------------------------------------------------------------------------------------------DIRECT3D AND METAL -----

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads*4;

	vbquads.SetColor(adr+0,r,g,b,1);
	vbquads.SetColor(adr+1,r,g,b,1);
	vbquads.SetColor(adr+2,r,g,b,1);
	vbquads.SetColor(adr+3,r,g,b,1);

	vbquads.SetVertex(adr+0,x,y,z);
	vbquads.SetVertex(adr+1,x+sizex,y,z);
	vbquads.SetVertex(adr+2,x+sizex,y+sizey,z);
	vbquads.SetVertex(adr+3,x,y+sizey,z);

	vbquads.SetTexCoo(adr+0,x1,y1);
	vbquads.SetTexCoo(adr+1,x2,y1);
	vbquads.SetTexCoo(adr+2,x2,y2);
	vbquads.SetTexCoo(adr+3,x1,y2);

	nMultiQuads++;
	MultiQuadsZbuffer=1;

#endif
#endif

}

void C3DAPI::QuadZ(float z,float w,float x0,float y0,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b,float a)
{
    if (!Active) return;
    
    if (x0+sizex<X1base) return;
    if (y0+sizey<Y1base) return;
    if (x0>X2base) return;
    if (y0>Y2base) return;
    if ((int)sizex==0) return;
    if ((int)sizey==0) return;

	if (nMultiQuadsXYZ>0) Flush();
    
#ifdef API3D_OPENGL20
//--------------------------------------------------------------------------------------------------OPEN GL 2.0 ------
	float x=decals_quads_x-VIEWPORT.a[3][0]+x0;
    float _x=x+sizex;

	if (MultiQuadsZbuffer==0) Flush();
	else
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int n=nMultiQuads<<2;

    float zz=0.5f-z/2;

    register int adr=(n<<2);
    
    vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=a;
    vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=a;
    vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=a;
    vbquads.BColor_Array[adr++]=r; vbquads.BColor_Array[adr++]=g; vbquads.BColor_Array[adr++]=b; vbquads.BColor_Array[adr++]=a;
    
    adr=(n<<1) +n;
    
    if (SelectedRenderTarget==-1)
    {
        
        float y=decals_quads_y-VIEWPORT.a[3][1]+y0;
        
        vbquads.BVertex_Array[adr++]=x;
        vbquads.BVertex_Array[adr++]=y;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=_x;
        vbquads.BVertex_Array[adr++]=y;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=_x;
        vbquads.BVertex_Array[adr++]=y+sizey;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=x;
        vbquads.BVertex_Array[adr++]=y+sizey;
        vbquads.BVertex_Array[adr++]=zz;
    }
    else
    {
        float _y1=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-y0;
        float _y2=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-(y0+sizey);
        
        vbquads.BVertex_Array[adr++]=x;
        vbquads.BVertex_Array[adr++]=_y1;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=x+sizex;
        vbquads.BVertex_Array[adr++]=_y1;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=x+sizex;
        vbquads.BVertex_Array[adr++]=_y2;
        vbquads.BVertex_Array[adr++]=zz;
        
        vbquads.BVertex_Array[adr++]=x;
        vbquads.BVertex_Array[adr++]=_y2;
        vbquads.BVertex_Array[adr++]=zz;
    }
    
    adr=(n<<1);
    
    if (aT<0)
    {
        vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=1-y1;
        vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=1-y1;
        vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=1-y2;
        vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=1-y2;
    }
    else
    {
        vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=y1;
        vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=y1;
        vbquads.BTexCoo_Array[adr++]=x2; vbquads.BTexCoo_Array[adr++]=y2;
        vbquads.BTexCoo_Array[adr++]=x1; vbquads.BTexCoo_Array[adr++]=y2;
    }


	nMultiQuads++;
	MultiQuadsZbuffer=1;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	DWORD c=D3DCOLOR_COLORVALUE(r,g,b,a);

	MultiQuads[(adr)+0].color=c;
	MultiQuads[(adr)+0].x=(float) x;
	MultiQuads[(adr)+0].y=(float) y;
	MultiQuads[(adr)+0].xm=x1;
	MultiQuads[(adr)+0].ym=y1;
	MultiQuads[(adr)+0].z=z;

	MultiQuads[(adr)+1].color=c;
	MultiQuads[(adr)+1].x=(float) (x+sizex);
	MultiQuads[(adr)+1].y=(float) y;
	MultiQuads[(adr)+1].xm=x2;
	MultiQuads[(adr)+1].ym=y1;
	MultiQuads[(adr)+1].z=z;

	MultiQuads[(adr)+2].color=c;
	MultiQuads[(adr)+2].x=(float) (x+sizex);
	MultiQuads[(adr)+2].y=(float) (y+sizey);
	MultiQuads[(adr)+2].xm=x2;
	MultiQuads[(adr)+2].ym=y2;
	MultiQuads[(adr)+2].z=z;

	MultiQuads[(adr)+3].color=c;
	MultiQuads[(adr)+3].x=(float) (x);
	MultiQuads[(adr)+3].y=(float) (y+sizey);
	MultiQuads[(adr)+3].xm=x1;
	MultiQuads[(adr)+3].ym=y2;
	MultiQuads[(adr)+3].z=z;

	nMultiQuads++;
	MultiQuadsZbuffer=1;
#endif

#if !defined(API3D_OPENGL20)&&!defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)

#ifdef API3D_OPENGL
	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrtho(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrtho(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);

	glColor4f(r,g,b,a);
	if (aT<0)
	{
		if (SelectedRenderTarget>=0)
		{
			
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);
		}
		else
		{
		
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),z);
		}
	}
	else
	{
		if (SelectedRenderTarget>=0)
		{		
			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-(y+sizey)),z);
		}
		else
		{			
			glTexCoord2f(x1,y1);
			glVertex3f((float) x,(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) (x+sizex),(float) (SCREEN_Y-(y+sizey)),z);

			glTexCoord2f(x1,y2);
			glVertex3f((float) x,(float) (SCREEN_Y-(y+sizey)),z);	
		}
	}
	
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#else

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=a;

	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=a;

	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=a;

	colorvertices[3*4+0]=r;
	colorvertices[3*4+1]=g;
	colorvertices[3*4+2]=b;
	colorvertices[3*4+3]=a;

	texvertices[2*0+0]=x1; texvertices[2*0+1]=y1;
	vertices[3*0+0]=(float) x; vertices[3*0+1]=(float) (SCREEN_Y-y); vertices[3*0+2]=z;

	texvertices[2*1+0]=x2; texvertices[2*1+1]=y1;
	vertices[3*1+0]=(float) (x+sizex); vertices[3*1+1]=(float) (SCREEN_Y-y); vertices[3*1+2]=z;

	texvertices[2*2+0]=x1; texvertices[2*2+1]=y2;
	vertices[3*2+0]=(float) x; vertices[3*2+1]=(float) (SCREEN_Y-(y+sizey)); vertices[3*2+2]=z;

	texvertices[2*3+0]=x2; texvertices[2*3+1]=y2;
	vertices[3*3+0]=(float) (x+sizex); vertices[3*3+1]=(float) (SCREEN_Y-(y+sizey)); vertices[3*3+2]=z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#endif
#endif	

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;

	if (MultiQuads)
	{
		if (nMultiQuads>MAX_VBQUADS-1) Flush();
		int adr=nMultiQuads*3;

		DWORD c=D3DCOLOR_COLORVALUE(r,g,b,a);

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) x;
		MultiQuads[(adr)+0].y=(float) y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;

		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) (x+sizex);
		MultiQuads[(adr)+1].y=(float) y;
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y1;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) (x+sizex);
		MultiQuads[(adr)+2].y=(float) (y+sizey);
		MultiQuads[(adr)+2].xm=x2;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
		adr+=3;

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) x;
		MultiQuads[(adr)+0].y=(float) y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;

		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) (x+sizex);
		MultiQuads[(adr)+1].y=(float) (y+sizey);
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y2;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) (x);
		MultiQuads[(adr)+2].y=(float) (y+sizey);
		MultiQuads[(adr)+2].xm=x1;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
	}
	else
	{
		struct CUSTOMVERTEX0 Vertices[4];
		
		Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,a);
		Vertices[1].color=Vertices[0].color;
		Vertices[2].color=Vertices[0].color;
		Vertices[3].color=Vertices[0].color;


		Vertices[0].x=(float) x;
		Vertices[0].y=(float) y;
		Vertices[0].xm=x1;
		Vertices[0].ym=y1;
		Vertices[0].rhw=w;
		Vertices[0].z=z;

		Vertices[1].x=(float) (x+sizex);
		Vertices[1].y=(float) y;
		Vertices[1].xm=x2;
		Vertices[1].ym=y1;
		Vertices[1].rhw=Vertices[0].rhw;
		Vertices[1].z=Vertices[0].z;

		Vertices[2].x=(float) (x+sizex);
		Vertices[2].y=(float) (y+sizey);
		Vertices[2].xm=x2;
		Vertices[2].ym=y2;
		Vertices[2].rhw=Vertices[0].rhw;
		Vertices[2].z=Vertices[0].z;

		Vertices[3].x=(float) (x);
		Vertices[3].y=(float) (y+sizey);
		Vertices[3].xm=x1;
		Vertices[3].ym=y2;
		Vertices[3].rhw=Vertices[0].rhw;
		Vertices[3].z=Vertices[0].z;

		D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX0 );
		D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,2,Vertices ,sizeof(CUSTOMVERTEX0));
	}
#endif

#if defined(API3D_DIRECT3D9)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//--------------------------------------------------------------------------------------------DIRECT3D AND METAL -----

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads*4;

	vbquads.SetColor(adr+0,r,g,b,a);
	vbquads.SetColor(adr+1,r,g,b,a);
	vbquads.SetColor(adr+2,r,g,b,a);
	vbquads.SetColor(adr+3,r,g,b,a);

	vbquads.SetVertex(adr+0,x,y,z);
	vbquads.SetVertex(adr+1,x+sizex,y,z);
	vbquads.SetVertex(adr+2,x+sizex,y+sizey,z);
	vbquads.SetVertex(adr+3,x,y+sizey,z);

	vbquads.SetTexCoo(adr+0,x1,y1);
	vbquads.SetTexCoo(adr+1,x2,y1);
	vbquads.SetTexCoo(adr+2,x2,y2);
	vbquads.SetTexCoo(adr+3,x1,y2);

	nMultiQuads++;
	MultiQuadsZbuffer=1;

#endif
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		Draw Quad with ZBuffer

		void C3DAPIBASE::Quad(int x,int y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b)

		x,y : co in screen coord.
		sizex,sizey: lengths of quad
		x1,y1,x2,y2 : mapping coo
		r,g,b : color
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::QuadXYZ(CVector &pos,float R,float z,float w,float x0,float y0,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b,float a)
{
    if (!Active) return;
    
    if (x0+sizex<X1base) return;
    if (y0+sizey<Y1base) return;
    if (x0>X2base) return;
    if (y0>Y2base) return;
    
    if (sizex==0) return;
    if (sizey==0) return;

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	if (nMultiQuads>0) Flush();

	if (nMultiQuadsXYZ>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuadsXYZ*4;

	vbquadsXYZ.SetColor(adr+0,r,g,b,a);
	vbquadsXYZ.SetColor(adr+1,r,g,b,a);
	vbquadsXYZ.SetColor(adr+2,r,g,b,a);
	vbquadsXYZ.SetColor(adr+3,r,g,b,a);
    
#ifdef API3D_OPENGL20
    float zz=0.5f-z/2;
    
    if (SelectedRenderTarget==-1)
    {
        vbquadsXYZ.SetVertex(adr+0,x,y,zz);
        vbquadsXYZ.SetVertex(adr+1,x+sizex,y,zz);
        vbquadsXYZ.SetVertex(adr+2,x+sizex,y+sizey,zz);
        vbquadsXYZ.SetVertex(adr+3,x,y+sizey,zz);
    }
    else
    {
        float _y1=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-y0;
        float _y2=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-(y0+sizey);
        
        vbquadsXYZ.SetVertex(adr+0,x,_y1,zz);
        vbquadsXYZ.SetVertex(adr+1,x+sizex,_y1,zz);
        vbquadsXYZ.SetVertex(adr+2,x+sizex,_y2,zz);
        vbquadsXYZ.SetVertex(adr+3,x,_y2,zz);
    }

#else
    float zz=z;
    
	vbquadsXYZ.SetVertex(adr+0,x,y,zz);
	vbquadsXYZ.SetVertex(adr+1,x+sizex,y,zz);
	vbquadsXYZ.SetVertex(adr+2,x+sizex,y+sizey,zz);
	vbquadsXYZ.SetVertex(adr+3,x,y+sizey,zz);
#endif
	vbquadsXYZ.SetTexCoo(adr+0,x1,y1);
	vbquadsXYZ.SetTexCoo(adr+1,x2,y1);
	vbquadsXYZ.SetTexCoo(adr+2,x2,y2);
	vbquadsXYZ.SetTexCoo(adr+3,x1,y2);

	vbquadsXYZ.SetTexCoo2(adr+0,pos.x,pos.y);
	vbquadsXYZ.SetTexCoo2(adr+1,pos.x,pos.y);
	vbquadsXYZ.SetTexCoo2(adr+2,pos.x,pos.y);
	vbquadsXYZ.SetTexCoo2(adr+3,pos.x,pos.y);

	vbquadsXYZ.SetTexCoo3(adr+0,pos.z,R);
	vbquadsXYZ.SetTexCoo3(adr+1,pos.z,R);
	vbquadsXYZ.SetTexCoo3(adr+2,pos.z,R);
	vbquadsXYZ.SetTexCoo3(adr+3,pos.z,R);

	nMultiQuadsXYZ++;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		Draw Quad in rotation with ZBuffer

		void C3DAPIBASE::QuadRotz(int x,int y,float sizex,float sizey,float ang,float x1,float y1,float x2,float y2,float r,float g,float b,float a)

		x,y : co in screen coord.
		sizex,sizey: lengths of quad
		ang : rotation angle
		x1,y1,x2,y2 : mapping coo
		r,g,b,a : color
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::QuadRotZ(float z,float w,float x0,float y0,float sizex,float sizey,float ang,float x1,float y1,float x2,float y2,float r,float g,float b,float a)
{
    if (!Active) return;

	if (nMultiQuadsXYZ>0) Flush();
    
#ifdef API3D_OPENGL20
//--------------------------------------------------------------------------------------------------OPEN GL 2.0 ------
	float s=sinf(ang*PI/180.0f);
	float c=cosf(ang*PI/180.0f);

	CVector2 p1,p2,p3,p4;
    
	p1.x=x0 - c*sizex - s*sizey;
	p1.y=y0 + s*sizex - c*sizey;
    
	p2.x=x0 + c*sizex - s*sizey;
	p2.y=y0 - s*sizex - c*sizey;
    
	p3.x=x0 + c*sizex + s*sizey;
	p3.y=y0 - s*sizex + c*sizey;
    
	p4.x=x0 - c*sizex + s*sizey;
	p4.y=y0 + s*sizex + c*sizey;
    
    if (SelectedRenderTarget==-1)
    {
        p1.x=decals_quads_x-VIEWPORT.a[3][0]+p1.x;
        p1.y=decals_quads_y-VIEWPORT.a[3][1]+p1.y;
        p2.x=decals_quads_x-VIEWPORT.a[3][0]+p2.x;
        p2.y=decals_quads_y-VIEWPORT.a[3][1]+p2.y;
        p3.x=decals_quads_x-VIEWPORT.a[3][0]+p3.x;
        p3.y=decals_quads_y-VIEWPORT.a[3][1]+p3.y;
        p4.x=decals_quads_x-VIEWPORT.a[3][0]+p4.x;
        p4.y=decals_quads_y-VIEWPORT.a[3][1]+p4.y;
    }
    else
    {
        p1.x=decals_quads_x-VIEWPORT.a[3][0]+p1.x;
        p1.y=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-p1.y;
        p2.x=decals_quads_x-VIEWPORT.a[3][0]+p2.x;
        p2.y=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-p2.y;
        p3.x=decals_quads_x-VIEWPORT.a[3][0]+p3.x;
        p3.y=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-p3.y;
        p4.x=decals_quads_x-VIEWPORT.a[3][0]+p4.x;
        p4.y=decals_quads_y-VIEWPORT.a[3][1]+VIEWPORT.a[1][1]*2-p4.y;
    }

	if (MultiQuadsZbuffer==0) Flush();
	else
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	float zz=0.5f-z/2;

	vbquads.SetColor(adr+0,r,g,b,a);
	vbquads.SetColor(adr+1,r,g,b,a);
	vbquads.SetColor(adr+2,r,g,b,a);
	vbquads.SetColor(adr+3,r,g,b,a);

	vbquads.SetVertex(adr+0,p1.x,p1.y,zz);
	vbquads.SetVertex(adr+1,p2.x,p2.y,zz);
	vbquads.SetVertex(adr+2,p3.x,p3.y,zz);
	vbquads.SetVertex(adr+3,p4.x,p4.y,zz);

	vbquads.SetTexCoo(adr+0,x1,y1);
	vbquads.SetTexCoo(adr+1,x2,y1);
	vbquads.SetTexCoo(adr+2,x2,y2);
	vbquads.SetTexCoo(adr+3,x1,y2);

	nMultiQuads++;
	MultiQuadsZbuffer=1;

#endif

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	float s=sinf(ang*PI/180.0f);
	float c=cosf(ang*PI/180.0f);

	CVector2 p1,p2,p3,p4;

	p1.x=x - c*sizex - s*sizey;
	p1.y=y + s*sizex - c*sizey;

	p2.x=x + c*sizex - s*sizey;
	p2.y=y - s*sizex - c*sizey;

	p3.x=x + c*sizex + s*sizey;
	p3.y=y - s*sizex + c*sizey;

	p4.x=x - c*sizex + s*sizey;
	p4.y=y + s*sizex + c*sizey;

	if (MultiQuadsZbuffer==0) Flush();
	else
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	DWORD cc=D3DCOLOR_COLORVALUE(r,g,b,a);

	MultiQuads[(adr)+0].color=cc;
	MultiQuads[(adr)+0].x=(float) p1.x;
	MultiQuads[(adr)+0].y=(float) p1.y;
	MultiQuads[(adr)+0].xm=x1;
	MultiQuads[(adr)+0].ym=y1;
	MultiQuads[(adr)+0].z=z;

	MultiQuads[(adr)+1].color=cc;
	MultiQuads[(adr)+1].x=(float) p2.x;
	MultiQuads[(adr)+1].y=(float) p2.y;
	MultiQuads[(adr)+1].xm=x2;
	MultiQuads[(adr)+1].ym=y1;
	MultiQuads[(adr)+1].z=z;

	MultiQuads[(adr)+2].color=cc;
	MultiQuads[(adr)+2].x=(float) p3.x;
	MultiQuads[(adr)+2].y=(float) p3.y;
	MultiQuads[(adr)+2].xm=x2;
	MultiQuads[(adr)+2].ym=y2;
	MultiQuads[(adr)+2].z=z;

	MultiQuads[(adr)+3].color=cc;
	MultiQuads[(adr)+3].x=(float) p4.x;
	MultiQuads[(adr)+3].y=(float) p4.y;
	MultiQuads[(adr)+3].xm=x1;
	MultiQuads[(adr)+3].ym=y2;
	MultiQuads[(adr)+3].z=z;

	nMultiQuads++;
	MultiQuadsZbuffer=1;
#endif
#if !defined(API3D_OPENGL20)&&!defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)

#ifdef API3D_OPENGL
	float s=sinf(ang*PI/180.0f);
	float c=cosf(ang*PI/180.0f);

	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;

	CVector2 p1,p2,p3,p4;

	p1.x=x - c*sizex - s*sizey;
	p1.y=y + s*sizex - c*sizey;

	p2.x=x + c*sizex - s*sizey;
	p2.y=y - s*sizex - c*sizey;

	p3.x=x + c*sizex + s*sizey;
	p3.y=y - s*sizex + c*sizey;

	p4.x=x - c*sizex + s*sizey;
	p4.y=y + s*sizex + c*sizey;
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#ifndef GLES

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrtho(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrtho(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);
		
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glColor4f(r,g,b,a);
	if (aT<0)
	{
		if (SelectedRenderTarget>=0)
		{			
			glTexCoord2f(x1,y2);
			glVertex3f((float) p1.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p1.y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) p2.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p2.y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) p3.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p3.y),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) p4.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p4.y),z);
		}
		else
		{		
			glTexCoord2f(x1,y2);
			glVertex3f((float) p1.x,(float) (SCREEN_Y-p1.y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) p2.x,(float) (SCREEN_Y-p2.y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) p3.x,(float) (SCREEN_Y-p3.y),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) p4.x,(float) (SCREEN_Y-p4.y),z);
		}
	}
	else
	{
		if (SelectedRenderTarget>=0)
		{		
			glTexCoord2f(x1,y2);
			glVertex3f((float) p1.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p1.y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) p2.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p2.y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) p3.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p3.y),z);

			glTexCoord2f(x1,y1);
			glVertex3f((float) p4.x,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-p4.y),z);
		}
		else
		{			
			glTexCoord2f(x1,y1);
			glVertex3f((float) p1.x,(float) (SCREEN_Y-p1.y),z);

			glTexCoord2f(x2,y1);
			glVertex3f((float) p2.x,(float) (SCREEN_Y-p2.y),z);

			glTexCoord2f(x2,y2);
			glVertex3f((float) p3.x,(float) (SCREEN_Y-p3.y),z);

			glTexCoord2f(x1,y2);
			glVertex3f((float) p4.x,(float) (SCREEN_Y-p4.y),z);			
		}
	}
	
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#else

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	static GLfloat vertices[4*3];
	static GLfloat texvertices[4*2];
	static GLfloat colorvertices[4*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=a;

	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=a;

	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=a;

	colorvertices[3*4+0]=r;
	colorvertices[3*4+1]=g;
	colorvertices[3*4+2]=b;
	colorvertices[3*4+3]=a;

	texvertices[2*0+0]=x1; texvertices[2*0+1]=y1;
	vertices[3*0+0]=(float) p1.x; vertices[3*0+1]=(float) (SCREEN_Y-p1.y); vertices[3*0+2]=z;

	texvertices[2*1+0]=x2; texvertices[2*1+1]=y1;
	vertices[3*1+0]=(float) p2.x; vertices[3*1+1]=(float) (SCREEN_Y-p2.y); vertices[3*1+2]=z;

	texvertices[2*2+0]=x1; texvertices[2*2+1]=y2;
	vertices[3*2+0]=(float) p3.x; vertices[3*2+1]=(float) (SCREEN_Y-p3.y); vertices[3*2+2]=z;

	texvertices[2*3+0]=x2; texvertices[2*3+1]=y2;
	vertices[3*3+0]=(float) p4.x; vertices[3*3+1]=(float) (SCREEN_Y-p4.y); vertices[3*3+2]=z;

	glVertexPointer(3,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
#endif
#endif	

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float s=sinf(ang*PI/180.0f);
	float c=cosf(ang*PI/180.0f);

	float x=decals_quads_x+x0;
	float y=decals_quads_y+y0;

	CVector2 p1,p2,p3,p4;

	p1.x=x - c*sizex - s*sizey;
	p1.y=y + s*sizex - c*sizey;

	p2.x=x + c*sizex - s*sizey;
	p2.y=y - s*sizex - c*sizey;

	p3.x=x + c*sizex + s*sizey;
	p3.y=y - s*sizex + c*sizey;

	p4.x=x - c*sizex + s*sizey;
	p4.y=y + s*sizex + c*sizey;

	if (MultiQuads)
	{
		if (nMultiQuads>MAX_VBQUADS-1) Flush();
		int adr=nMultiQuads*3;

		DWORD c=D3DCOLOR_COLORVALUE(r,g,b,a);

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) p1.x;
		MultiQuads[(adr)+0].y=(float) p1.y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;

		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) p2.x;
		MultiQuads[(adr)+1].y=(float) p2.y;
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y1;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) p3.x;
		MultiQuads[(adr)+2].y=(float) p3.y;
		MultiQuads[(adr)+2].xm=x2;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
		adr+=3;

		MultiQuads[(adr)+0].color=c;
		MultiQuads[(adr)+0].x=(float) p1.x;
		MultiQuads[(adr)+0].y=(float) p1.y;
		MultiQuads[(adr)+0].xm=x1;
		MultiQuads[(adr)+0].ym=y1;
		MultiQuads[(adr)+0].rhw=w;
		MultiQuads[(adr)+0].z=z;


		MultiQuads[(adr)+1].color=c;
		MultiQuads[(adr)+1].x=(float) p3.x;
		MultiQuads[(adr)+1].y=(float) p3.y;
		MultiQuads[(adr)+1].xm=x2;
		MultiQuads[(adr)+1].ym=y2;
		MultiQuads[(adr)+1].rhw=w;
		MultiQuads[(adr)+1].z=z;

		MultiQuads[(adr)+2].color=c;
		MultiQuads[(adr)+2].x=(float) p4.x;
		MultiQuads[(adr)+2].y=(float) p4.y;
		MultiQuads[(adr)+2].xm=x1;
		MultiQuads[(adr)+2].ym=y2;
		MultiQuads[(adr)+2].rhw=w;
		MultiQuads[(adr)+2].z=z;

		nMultiQuads++;
	}
	else
	{
		struct CUSTOMVERTEX0 Vertices[4];
		
		Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,a);
		Vertices[1].color=Vertices[0].color;
		Vertices[2].color=Vertices[0].color;
		Vertices[3].color=Vertices[0].color;

		Vertices[0].x=(float) p1.x;
		Vertices[0].y=(float) p1.y;
		Vertices[0].xm=x1;
		Vertices[0].ym=y1;
		Vertices[0].rhw=w;
		Vertices[0].z=z;

		Vertices[1].x=(float) p2.x;
		Vertices[1].y=(float) p2.y;
		Vertices[1].xm=x2;
		Vertices[1].ym=y1;
		Vertices[1].rhw=Vertices[0].rhw;
		Vertices[1].z=Vertices[0].z;

		Vertices[2].x=(float) p3.x;
		Vertices[2].y=(float) p3.y;
		Vertices[2].xm=x2;
		Vertices[2].ym=y2;
		Vertices[2].rhw=Vertices[0].rhw;
		Vertices[2].z=Vertices[0].z;

		Vertices[3].x=(float) p4.x;
		Vertices[3].y=(float) p4.y;
		Vertices[3].xm=x1;
		Vertices[3].ym=y2;
		Vertices[3].rhw=Vertices[0].rhw;
		Vertices[3].z=Vertices[0].z;

		D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX0 );
		D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,2,Vertices ,sizeof(CUSTOMVERTEX0));
	}
#endif

#if defined(API3D_DIRECT3D9)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
//--------------------------------------------------------------------------------------------DIRECT3D AND METAL -----

	float x=decals_quads_x+x0-VIEWPORT.a[3][0];
	float y=decals_quads_y+y0-VIEWPORT.a[3][1];

	float s=sinf(ang*PI/180.0f);
	float c=cosf(ang*PI/180.0f);

	CVector2 p1,p2,p3,p4;

	p1.x=x - c*sizex - s*sizey;
	p1.y=y + s*sizex - c*sizey;

	p2.x=x + c*sizex - s*sizey;
	p2.y=y - s*sizex - c*sizey;

	p3.x=x + c*sizex + s*sizey;
	p3.y=y - s*sizex + c*sizey;

	p4.x=x - c*sizex + s*sizey;
	p4.y=y + s*sizex + c*sizey;

	if (MultiQuadsZbuffer==0) Flush();
	else
	if (nMultiQuads>MAX_VBQUADS-1) Flush();

	int adr=nMultiQuads<<2;

	vbquads.SetColor(adr+0,r,g,b,a);
	vbquads.SetColor(adr+1,r,g,b,a);
	vbquads.SetColor(adr+2,r,g,b,a);
	vbquads.SetColor(adr+3,r,g,b,a);

	vbquads.SetVertex(adr+0,p1.x,p1.y,z);
	vbquads.SetVertex(adr+1,p2.x,p2.y,z);
	vbquads.SetVertex(adr+2,p3.x,p3.y,z);
	vbquads.SetVertex(adr+3,p4.x,p4.y,z);

	vbquads.SetTexCoo(adr+0,x1,y1);
	vbquads.SetTexCoo(adr+1,x2,y1);
	vbquads.SetTexCoo(adr+2,x2,y2);
	vbquads.SetTexCoo(adr+3,x1,y2);

	nMultiQuads++;
	MultiQuadsZbuffer=1;
#endif
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		cf cassebrique	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void C3DAPI::SetVertexBufferAlpha(float alpha,CObject3D * ref,CMatrix M)
{
    if (!Active) return;
    
	int n;
	float f;
	ActualVertexBuffer->LockVertices();
	for (n=0;n<(int)ActualVertexBuffer->nVertices;n++)
	{
		ref->Vertices[n].NormCalc=ref->Vertices[n].Norm*M;
		f=ref->Vertices[n].NormCalc.z;
		if (f<0.4f) f=0.4f;
		if (f>1.0f) f=1.0f;
		ActualVertexBuffer->SetColor(n,f,f,f,alpha);
	}
	ActualVertexBuffer->UnlockVertices();
	
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		coloured and textured triangle
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
#if defined(API3D_OPENGL20)||defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)
extern CVertexBuffer			effect_hll_quads_alphatest;
#endif

void C3DAPI::TriMC(float x0,float y0,float x1,float y1,float x2,float y2,float u0,float v0,float u1,float v1,float u2,float v2,float r,float g,float b,float a)
{
    if (!Active) return;
    
#ifdef API3D_OPENGL

#ifdef GLES
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	GLfloat vertices[3*2];
	GLfloat texvertices[3*2];
	GLfloat colorvertices[3*4];

	colorvertices[0*4+0]=r;
	colorvertices[0*4+1]=g;
	colorvertices[0*4+2]=b;
	colorvertices[0*4+3]=1.0f;

	colorvertices[1*4+0]=r;
	colorvertices[1*4+1]=g;
	colorvertices[1*4+2]=b;
	colorvertices[1*4+3]=1.0f;

	colorvertices[2*4+0]=r;
	colorvertices[2*4+1]=g;
	colorvertices[2*4+2]=b;
	colorvertices[2*4+3]=1.0f;


	texvertices[2*0+0]=u0; texvertices[2*0+1]=v0;
	vertices[2*0+0]=(float) (x0); vertices[2*0+1]=(float) SCREEN_Y-y0;

	texvertices[2*1+0]=u1; texvertices[2*1+1]=v1;
	vertices[2*1+0]=(float) (x1); vertices[2*1+1]=(float) SCREEN_Y-y1;

	texvertices[2*2+0]=u2; texvertices[2*2+1]=v2;
	vertices[2*2+0]=(float) (x2); vertices[2*2+1]=(float) SCREEN_Y-y2;
	
	glVertexPointer(2,GL_FLOAT,0,vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(4,GL_FLOAT,0,colorvertices);
	glEnableClientState(GL_COLOR_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2,GL_FLOAT,0,texvertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP,0,3);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);


#else
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	if (((SelectedRenderTarget>=0)||(aT<0))&&(!SecondarySurface)) return;

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (SelectedRenderTarget==-1)
		glOrtho(0.0f,SCREEN_X,0.0f,SCREEN_Y,-1.0f,1.0f);
	else
		glOrtho(0.0f,tile_secondarysurfacesW[SelectedRenderTarget],0.0f,tile_secondarysurfacesH[SelectedRenderTarget],-1.0f,1.0f);

	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	glBegin(GL_TRIANGLES);

	glColor3f(r,g,b);
	if (aT<0)
	{
		if (SelectedRenderTarget>=0)
		{
			glTexCoord2f(u0,v0);
			glVertex3f((float) x0,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y0),0);

			glTexCoord2f(u1,v1);
			glVertex3f((float) x1,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y1),0);

			glTexCoord2f(u2,v2);
			glVertex3f((float) x2,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y2),0);
		}
		else
		{		
			glTexCoord2f(u0,v0);
			glVertex3f((float) x0,(float) (SCREEN_Y-y0),0);

			glTexCoord2f(u1,v1);
			glVertex3f((float) x1,(float) (SCREEN_Y-y1),0);

			glTexCoord2f(u2,v2);
			glVertex3f((float) x2,(float) (SCREEN_Y-y2),0);
		}
	
	}
	else
	{

		if (SelectedRenderTarget>=0)
		{
			glTexCoord2f(u0,v0);
			glVertex3f((float) x0,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y0),0);

			glTexCoord2f(u1,v1);
			glVertex3f((float) x1,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y1),0);

			glTexCoord2f(u2,v2);
			glVertex3f((float) x2,(float) (tile_secondarysurfacesH[SelectedRenderTarget]-y2),0);
		}
		else
		{
			glTexCoord2f(u0,v0);
			glVertex3f((float) x0,(float) (SCREEN_Y-y0),0);

			glTexCoord2f(u1,v1);
			glVertex3f((float) x1,(float) (SCREEN_Y-y1),0);

			glTexCoord2f(u2,v2);
			glVertex3f((float) x2,(float) (SCREEN_Y-y2),0);
		}
	}
	
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
#endif
#endif

#if defined(API3D_OPENGL20)||defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_METAL)||defined(API3D_DIRECT3D12)

	vbquad_xyzcolortex.LockVertices();
	
	vbquad_xyzcolortex.SetColor(0,r,g,b,a);
	vbquad_xyzcolortex.SetColor(1,r,g,b,a);
	vbquad_xyzcolortex.SetColor(2,r,g,b,a);

	vbquad_xyzcolortex.SetVertex(0,x0-VIEWPORT.a[3][0],y0-VIEWPORT.a[3][1],0.5f);
	vbquad_xyzcolortex.SetVertex(1,x1-VIEWPORT.a[3][0],y1-VIEWPORT.a[3][1],0.5f);
	vbquad_xyzcolortex.SetVertex(2,x2-VIEWPORT.a[3][0],y2-VIEWPORT.a[3][1],0.5f);

	vbquad_xyzcolortex.SetTexCoo(0,u0,v0);
	vbquad_xyzcolortex.SetTexCoo(1,u1,v1);
	vbquad_xyzcolortex.SetTexCoo(2,u2,v2);

	vbquad_xyzcolortex.UnlockVertices();

	whileFlush=true;

	CVertexBuffer * actual=ActualVertexBuffer;
	CVertexBuffer * eff=ActualEffect;

	effect_hll_quads_alphatest.setTexture("Tex",aT);		

	effect_hll_quads_alphatest.setFloat("ALPHA",val_alphatest);
	effect_hll_quads_alphatest.setMatrix("PROJ",ProjOrtho);
	SetEffect(&effect_hll_quads_alphatest);

	vbquad_xyzcolortex.nIndicesActif=3;

	SetVertexBuffer(&vbquad_xyzcolortex);
	DrawVertexBuffer();

	ActualVertexBuffer=actual;
	ActualEffect=eff;

	whileFlush=false;

	vbquad_xyzcolortex.nIndicesActif=vbquad_xyzcolortex.nIndices;

#else

#if !defined(API3D_OPENGL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	float rhw=(float) Front+0.1f;
	float zz=0.01f;

	struct CUSTOMVERTEX0 Vertices[3];

	D3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,0);

	Vertices[0].color=D3DCOLOR_COLORVALUE(r,g,b,a);
	Vertices[1].color=Vertices[0].color;
	Vertices[2].color=Vertices[0].color;

	Vertices[0].x=x0;
	Vertices[0].y=y0;
	Vertices[0].xm=u0;
	Vertices[0].ym=v0;
	Vertices[0].rhw=(float) 2/(Front+Back);
	Vertices[0].z=zz;

	Vertices[1].x=x1;
	Vertices[1].y=y1;
	Vertices[1].xm=u1;
	Vertices[1].ym=v1;
	Vertices[1].rhw=Vertices[0].rhw;
	Vertices[1].z=zz;

	Vertices[2].x=x2;
	Vertices[2].y=y2;
	Vertices[2].xm=u2;
	Vertices[2].ym=v2;
	Vertices[2].rhw=Vertices[0].rhw;
	Vertices[2].z=zz;

#ifdef API3D_DIRECT3D9
	D3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX0 );
#else
	D3DDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX0 );
#endif
	D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,1,Vertices,sizeof(CUSTOMVERTEX0));

#endif
#endif
}
