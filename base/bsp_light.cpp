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



////////////////////////////////////////////////////////////////////////
//	@file bsp_light.cpp 
//	@date 2003
////////////////////////////////////////////////////////////////////////

#include "params.h"

#ifdef _DEFINES_API_CODE_BSP_LIGHTS_

#include <stdio.h>
#include <float.h>
#include <math.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define TOTO_GRUGE
//#define ROOTSHADOWNEW
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool	TRIANGULARISATION_ERROR=false;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define COEF_CONVERTLINEAR_BSP (SMALLF*SMALLF)
#define COEF_CONVERTLINEAR_BSP_DIRECTIONNAL (SMALLF*SMALLF)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SMALLFBSPLIGHTROOT -SMALLF
#define SMALLFBSPLIGHTCOMBINE -SMALLF
#define SMALLFBSPLIGHT0 SMALLF
#define SMALLFBSPLIGHTPLANECLOSE SMALLF
#define ONEFBSPLIGHTPLANENORMAL 0.999f
#define SMALLFBSPLIGHT SMALLF
#define SMALLFBSPLIGHT2 SMALLF2
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "bsp_light.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OPTIMIZE=true;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void (*fonction_progress_bspapply_init)(int len)=NULL;
void (*fonction_progress_bspapply_step)()=NULL;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::RootShadowedQuad(CBSPLightNode * node,CVector &v0,CVector &v1,CVector &v2,CVector &v3)
{
	int res;
	int pas;
	int n1,n2;
	CVector v,a,b;
	float t;
	CVector u1,u2;
	float area;

	u1=v1-v0;
	u2=v3-v0;
	area=VECTORNORM(u1)*VECTORNORM(u2);
	pas=(int) (area*3.0f);
	if (pas>300) pas=300;
	t=((float) pas)/150.0f;
	t=t*t;
	pas=(int) (t*300);
	if (pas<16) pas=16;

	res=0;
	n1=0;
	while ((res==0)&&(n1<pas))
	{
		t=((float) n1)/((float) pas);
		VECTORINTERPOL(a,t,v3,v0);
		VECTORINTERPOL(b,t,v2,v1);
		n2=0;
		while ((res==0)&&(n2<pas))
		{
			t=((float) n2)/((float) pas);
			VECTORINTERPOL(v,t,b,a);
			if (!RootLight(node,v)) res++;
			n2++;
		}
		n1++;
	}

	return (res>0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::ObjectInShadow(CBSPLightNode * node,CObject3D *obj)
{
	bool res=false;
	CVector c,size;
	CVector Sommet[8];
	int n,nn;
	CVector v,a,b;
	float t;
	int pas=48;

	obj->Calculate();

	n=0;
	while ((!res)&&(n<obj->edges->nList))
	{
		a=obj->Vertices[obj->edges->List[n].a].Calc;
		b=obj->Vertices[obj->edges->List[n].b].Calc;
		VECTORSUB(v,b,a);
		pas=(int) (8*VECTORNORM(v));
		t=0.0f;
		float inct=1.0f/((float) pas);
		nn=0;
		while ((!res)&&(nn<=pas))
		{
			VECTORINTERPOL(v,t,a,b);
			if (!RootLight(node,v)) res=true;
			t=t+inct;
			nn++;
		}
		n++;
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::SaveBsp(CDataContainer *data)
{
	int n;
	CBSPLightNode **r;
	CBSPLightNode *res;

	data->storeWord(roots.Length());

	r=roots.GetFirst();
	while (r)
	{
		compteur=0;
		NodeIndice(*r);

		data->storeWord(compteur);

		for (n=0;n<compteur;n++)
		{
			getNode(*r,n,&res);

			if (res->type<0) data->storeChar((char) 0xFF);
			else data->storeChar(res->type);
			data->storeChar(res->tag);
			if (res->plus) data->storeWord(res->plus->index);
			else data->storeWord((unsigned short int) 0xFFFF);
			if (res->moins) data->storeWord(res->moins->index);
			else data->storeWord((unsigned short int) 0xFFFF);

			if ((res->plus)||(res->moins))
			{
				data->storeFloat(res->a);
				data->storeFloat(res->b);
				data->storeFloat(res->c);
				data->storeFloat(res->d);
			}
		}

		r=roots.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::SetTesselateValue(float v)
{
	tess_value=v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::LoadBsp(CDataReader * data)
{
	int n,nn;
	CBSPLightNode *array[30000];
	int nbre,nnb;
	int p,m;

	nbre=data->readWord();
	for (nn=0;nn<nbre;nn++)
	{
		nnb=data->readWord();
		for (n=0;n<nnb;n++) array[n]=new CBSPLightNode;
		for (n=0;n<nnb;n++)
		{
			array[n]->type=data->readChar();
			if (array[n]->type==-1) array[n]->type=-1;
			array[n]->tag=data->readChar();
			p=data->readWord();
			m=data->readWord();
			array[n]->index=n;
			if ((p!=0xFFFF)||(m!=0xFFFF))
			{
				array[n]->a=data->readFloat();
				array[n]->b=data->readFloat();
				array[n]->c=data->readFloat();
				array[n]->d=data->readFloat();
			}
			if (p!=0xFFFF) array[n]->plus=array[p];
			else array[n]->plus=NULL;
			if (m!=0xFFFF) array[n]->moins=array[m];
			else array[n]->moins=NULL;
		}
		if (nnb>0) roots.Add(array[0]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool __RootShadow(CBSPLightNode * n,CVector &u,CVector &v);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::TestIfShadows(CObject3D * obj,CBSPLightNode *r,CVector &L)
{
#ifdef ROOTSHADOWNEW
	static float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	CVector verts[8];

	for (int n=0;n<8;n++)
	{
		if (sss[n][0]>0) verts[n].x=objbase->x1;
		else verts[n].x=objbase->x0;
		if (sss[n][1]>0) verts[n].y=objbase->y1;
		else verts[n].y=objbase->y0;
		if (sss[n][2]>0) verts[n].z=objbase->z1;
		else verts[n].z=objbase->z0;
	}
	
	for (int i=0;i<8;i++)
		for (int j=7;j>i;j--)
			if (__RootShadow(r,verts[i],verts[j])) return true;

	return false;
#else
	int n;
	bool res=false;
	CVector u,N,P;
	CList <CShortFace> faces;
	CShortFace f;
	CVertex s;
	
	for (n=0;n<obj->nVertices;n++)
	{
		s.Calc=obj->Vertices[n].Stok;
		Vertices.Add(s);
	}

	for (n=0;n<obj->nFaces;n++)
	{
		f.n0=obj->Faces[n].v0;
		f.n1=obj->Faces[n].v1;
		f.n2=obj->Faces[n].v2;
		f.s0=Vertices[f.n0];
		f.s1=Vertices[f.n1];
		f.s2=Vertices[f.n2];
		faces.Add(f);
	}

	if (faces.Length()>0)
	{
		faces_in_shadows_tag=0;
		ApplyToFacesGruged(r,&faces);
		res=(faces_in_shadows_tag==1);
	}

	nVertices=0;
	faces.Free();
	Vertices.Free();

	return res;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::TestIfShadowsExt(CObject3D * obj,CBSPLightNode *r,CVector &Ldir)
{
#ifdef ROOTSHADOWNEW
	static float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	CVector verts[8];

	for (int n=0;n<8;n++)
	{
		if (sss[n][0]>0) verts[n].x=objbase->x1;
		else verts[n].x=objbase->x0;
		if (sss[n][1]>0) verts[n].y=objbase->y1;
		else verts[n].y=objbase->y0;
		if (sss[n][2]>0) verts[n].z=objbase->z1;
		else verts[n].z=objbase->z0;
	}

	for (int i=0;i<8;i++)
		for (int j=7;j>i;j--)
			if (__RootShadow(r,verts[i],verts[j])) return true;

	return false;
#else
	int n;
	bool res=false;
	CList <CShortFace> faces;
	CShortFace f;
	CVertex s;

	for (n=0;n<obj->nVertices;n++)
	{
		s.Calc=obj->Vertices[n].Stok;
		Vertices.Add(s);
	}

	for (n=0;n<obj->nFaces;n++)
	{
		f.n0=obj->Faces[n].v0;
		f.n1=obj->Faces[n].v1;
		f.n2=obj->Faces[n].v2;
		f.s0=Vertices[f.n0];
		f.s1=Vertices[f.n1];
		f.s2=Vertices[f.n2];
		faces.Add(f);
	}

	if (faces.Length()>0)
	{
		faces_in_shadows_tag=0;
		ApplyToFacesGruged(r,&faces);
		res=(faces_in_shadows_tag==1);
	}

	nVertices=0;
	faces.Free();
	Vertices.Free();

	return res;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPLight::BspApply(CObject3D * obj,int tag,CVector &L)
{
	static float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	CObject3D *tmp1,*tmp2,*tmp3,*tmp4;
	CBSPLightNode**r;
	int n;
	int count;
	int tmptag;
	CVector v;
	int nb=roots.Length();
	float inc=3.5f/nb;
	float value=0.0f;

	obj->Calculate();

	obj->x0=obj->x1=obj->Coo.x;
	obj->y0=obj->y1=obj->Coo.y;
	obj->z0=obj->z1=obj->Coo.z;

	for (n=0;n<obj->nVertices;n++)
	{
		v=obj->Vertices[n].Calc;
		if (v.x<obj->x0) obj->x0=v.x;
		if (v.x>obj->x1) obj->x1=v.x;
		if (v.y<obj->y0) obj->y0=v.y;
		if (v.y>obj->y1) obj->y1=v.y;
		if (v.z<obj->z0) obj->z0=v.z;
		if (v.z>obj->z1) obj->z1=v.z;
	}	

	CObject3D * objbase=new CObject3D;
	objbase->Init(8,12);

	for (n=0;n<8;n++)
	{
		if (sss[n][0]>0) objbase->Vertices[n].Stok.x=obj->x1;
		else objbase->Vertices[n].Stok.x=obj->x0;
		if (sss[n][1]>0) objbase->Vertices[n].Stok.y=obj->y1;
		else objbase->Vertices[n].Stok.y=obj->y0;
		if (sss[n][2]>0) objbase->Vertices[n].Stok.z=obj->z1;
		else objbase->Vertices[n].Stok.z=obj->z0;
	}

	objbase->Carre(0		,0,1,2,3);
	objbase->Carre(2		,3,2,6,7);
	objbase->Carre(4		,0,3,7,4);
	objbase->Carre(6		,1,0,4,5);
	objbase->Carre(8		,2,1,5,6);
	objbase->Carre(10		,4,7,6,5);


	tmp1=obj->Duplicate2();

	if (tag==1) { for (n=0;n<tmp1->nFaces;n++) tmp1->Faces[n].tag=tag; }
	else { for (n=0;n<tmp1->nFaces;n++) tmp1->Faces[n].tag+=tag; }

	count=0;
	n=0;
	r=roots.GetFirst();
	while (r)
	{
		tmp2=NULL;
		tmp3=NULL;

		if (TestIfShadows(objbase,*r,L))
		{
			tmp2=RootApply(*r,tmp1,tag,L);
			n++;
			tmptag=1;
			if (tag==1) tmptag=2;
			if (tag==2) tmptag=4;
			if (tag==4) tmptag=8;
			if (tag==8) tmptag=16;

			tmp4=tmp2->ConvertLinearMap2(COEF_CONVERTLINEAR_BSP);

			if (OPTIMIZE)
			{
				tmp3=tmp4->OptimizeMeshBooleanTag(tmptag,true);
				if (tmp3==NULL)
				{
					tmp3=tmp1->Duplicate2();
					TRIANGULARISATION_ERROR=true;
				}
			}
			else tmp3=tmp1->Duplicate2();

			if (r)
			{
				tmp4->Free();
				delete tmp4;

				tmp2->Free();
				delete tmp2;
				tmp2=NULL;

				tmp1->Free();
				delete tmp1;
				tmp1=NULL;
								
				tmp1=tmp3;

				r=roots.GetNext();
			}
		}
		else r=roots.GetNext();
	}

	objbase->Free();
	delete objbase;

	return tmp1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPLight::BspDirectionnalApply(CObject3D * obj,int tag,CVector &Ldir)
{
	static float sss[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	CObject3D *tmp1,*tmp2,*tmp3,*tmp4;
	CBSPLightNode**r;
	int n;
	int count;
	int tmptag;
	CVector v;
	CMatrix M;
	int nb=roots.Length();
	float inc=3.5f/nb;
	float value=0.0f;

	obj->Calculate();

	obj->x0=obj->x1=obj->Coo.x;
	obj->y0=obj->y1=obj->Coo.y;
	obj->z0=obj->z1=obj->Coo.z;

	for (n=0;n<obj->nVertices;n++)
	{
		v=obj->Vertices[n].Calc;
		if (v.x<obj->x0) obj->x0=v.x;
		if (v.x>obj->x1) obj->x1=v.x;
		if (v.y<obj->y0) obj->y0=v.y;
		if (v.y>obj->y1) obj->y1=v.y;
		if (v.z<obj->z0) obj->z0=v.z;
		if (v.z>obj->z1) obj->z1=v.z;
	}

	CObject3D * objbase=new CObject3D;
	objbase->Init(8,12);

	for (n=0;n<8;n++)
	{
		if (sss[n][0]>0) objbase->Vertices[n].Stok.x=obj->x1;
		else objbase->Vertices[n].Stok.x=obj->x0;
		if (sss[n][1]>0) objbase->Vertices[n].Stok.y=obj->y1;
		else objbase->Vertices[n].Stok.y=obj->y0;
		if (sss[n][2]>0) objbase->Vertices[n].Stok.z=obj->z1;
		else objbase->Vertices[n].Stok.z=obj->z0;
	}

	objbase->Carre(0		,0,1,2,3);
	objbase->Carre(2		,3,2,6,7);
	objbase->Carre(4		,0,3,7,4);
	objbase->Carre(6		,1,0,4,5);
	objbase->Carre(8		,2,1,5,6);
	objbase->Carre(10		,4,7,6,5);

	tmp1=obj->Duplicate2();

	if (tag==1)
	{
		for (n=0;n<tmp1->nFaces;n++) tmp1->Faces[n].tag=tag;
	}
	else
	{
		for (n=0;n<tmp1->nFaces;n++) tmp1->Faces[n].tag+=tag;
	}

	count=0;
	n=0;
	r=roots.GetFirst();
	while (r)
	{
		tmp2=NULL;
		tmp3=NULL;

		if (TestIfShadowsExt(objbase,*r,Ldir))
		{
			tmp2=RootApplyDirectionnal(*r,tmp1,tag,Ldir);
			n++;
			tmptag=1;
			if (tag==1) tmptag=2;
			if (tag==2) tmptag=4;
			if (tag==4) tmptag=8;
			if (tag==8) tmptag=16;

			tmp4=tmp2->ConvertLinearMap2(COEF_CONVERTLINEAR_BSP_DIRECTIONNAL);

			if (OPTIMIZE)
			{
				tmp3=tmp4->OptimizeMeshBooleanTag(tmptag,true);
				if (tmp3==NULL)
				{
					tmp3=tmp1->Duplicate2();
					TRIANGULARISATION_ERROR=true;
				}
			}
			else tmp3=tmp1->Duplicate2();

			if (r)
			{
				tmp4->Free();
				delete tmp4;

				tmp2->Free();
				delete tmp2;
				tmp2=NULL;

				tmp1->Free();
				delete tmp1;
				tmp1=NULL;

				tmp1=tmp3;

				r=roots.GetNext();
			}
		}
		else r=roots.GetNext();
	}

	objbase->Free();
	delete objbase;

	return tmp1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::PlaneList(CBSPLightNode * node)
{
	CPlane P,*p;
	bool present;
	if (node->type==NODE)
	{
		P.Init(node->a,node->b,node->c,node->d);
		P.Normalise();

		p=Plans.GetFirst();
		present=false;
		while ((p)&&(!present))
		{
			if (p->a*P.a+p->b*P.b+p->c*P.c>ONEFBSPLIGHTPLANENORMAL)
			{
				if (f_abs(P.d-p->d)<SMALLFBSPLIGHTPLANECLOSE) present=true;
			}

			if (p->a*P.a+p->b*P.b+p->c*P.c<-ONEFBSPLIGHTPLANENORMAL)
			{
				if (f_abs(P.d+p->d)<SMALLFBSPLIGHTPLANECLOSE) present=true; //0.0001f
			}

			p=Plans.GetNext();
		}

		if (!present) Plans.Add(P);

		PlaneList(node->plus);
		PlaneList(node->moins);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::Lighted(CVector &v)
{
	bool res;
	CBSPLightNode *n,**r;
	float s;

	if (roots.Length()==0) return true;

	res=true;
	r=roots.GetFirst();
	while ((r)&&(res))
	{
		n=*r;

		if (n!=NULL)
		{
			while (n->type==NODE)
			{
				s=n->a*v.x+ n->b*v.y + n->c*v.z + n->d;
				if (s>=-SMALLFBSPLIGHT) n=n->plus;
				else n=n->moins;
			}
			res=(n->type==LIGHT);
		}

		r=roots.GetNext();
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::RootLight(CBSPLightNode * node,CVector &v)
{
	bool res;
	CBSPLightNode *n;
	float s;

	res=false;
	n=node;
	if (n!=NULL)
	{
		while (n->type==NODE)
		{
			s=n->a*v.x+ n->b*v.y + n->c*v.z + n->d;
			if (s>=-SMALLFBSPLIGHT0) n=n->plus;
			else n=n->moins;
		}
		res=(n->type==LIGHT);
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::RootExtShadowVolume(CBSPLightNode * node,CVector &v)
{
	bool res=false;
	CBSPLightNode *n;
	float s;

	if (node!=NULL)
	{
		if ((node->type==NODE)&&(node->tag==SHADOWFACE))
		{
			s=node->a*v.x+ node->b*v.y + node->c*v.z + node->d;

			if (s>=-SMALLFBSPLIGHT) n=node->plus;
			else n=node->moins;

			res=RootExtShadowVolume(n,v);
		}
		else
		if ((node->type==NODE)&&(node->tag==NORMALFACE))
		{
			res=(RootExtShadowVolume(node->plus,v)||RootExtShadowVolume(node->moins,v));
		}
		else
		{
			res=(node->type==LIGHT);
		}
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::RootShadow(CBSPLightNode * node,CVector &v)
{
	bool res;
	CBSPLightNode *n;
	float s;

	res=true;
	n=node;
	if (n!=NULL)
	{
		while (n->type==NODE)
		{
			s=n->a*v.x+ n->b*v.y + n->c*v.z + n->d;
			if (s>=-SMALLFBSPLIGHT) n=n->plus;
			else n=n->moins;
		}
		res=(n->type==SHADOW);
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool __RootShadow(CBSPLightNode * n,CVector &u,CVector &v)
{
	float s0,s1;

	if (n->type==NODE)
	{
		s0=n->a*u.x+ n->b*u.y + n->c*u.z + n->d;
		s1=n->a*v.x+ n->b*v.y + n->c*v.z + n->d;

		if (s0*s1<0)
		{
			if (__RootShadow(n->plus,u,v)) return true;
			else return __RootShadow(n->moins,u,v);
		}
		else
		{
			if ((s0>SMALLFBSPLIGHTROOT)&&(s1>SMALLFBSPLIGHTROOT)) return __RootShadow(n->plus,u,v);
			else return __RootShadow(n->moins,u,v);
		}
	}
	else
	{
		if (n->type==SHADOW) return true;
		else return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::RootShadowedFace(CBSPLightNode * node,CShortFace * f)
{
	return true;
	if (__RootShadow(node,f->s0->Calc,f->s1->Calc)) return true;
	if (__RootShadow(node,f->s1->Calc,f->s2->Calc)) return true;
	if (__RootShadow(node,f->s2->Calc,f->s0->Calc)) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPLight::Apply(CObject3D * obj,int tag,CVector &L)
{
	CObject3D *res;
	int n;
	CVector u,N,P;
	CList <CShortFace> faces,facestoapply;
	CShortFace f,*ff;
	CVertex s,*ps;

	CBSPLightNode **r;
	CMatrix Mobj,Mrot;
	float ss;

	ss=1;
	if (obj->Status==1) ss=-1;

	Mobj.Id();
	Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	Mrot=Mobj;
	Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);

	res=new CObject3D;

	res->Rot=obj->Rot;
	res->Coo=obj->Coo;

	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Calc=obj->Vertices[n].Stok*Mobj;
		s.Stok=obj->Vertices[n].Stok;
		s.Calc=obj->Vertices[n].Calc;
		s.Map=obj->Vertices[n].Map;
		s.Map2=obj->Vertices[n].Map2;
		Vertices.Add(s);
	}

	for (n=0;n<obj->nFaces;n++)
	{
		P=(obj->Faces[n].v[0]->Calc + obj->Faces[n].v[1]->Calc + obj->Faces[n].v[2]->Calc)/3;
		f.n0=obj->Faces[n].v0;
		f.n1=obj->Faces[n].v1;
		f.n2=obj->Faces[n].v2;
		f.nT=obj->Faces[n].nT;
		f.nL=obj->Faces[n].nL;
		f.tag=obj->Faces[n].tag;
		f.s0=Vertices[f.n0];
		f.s1=Vertices[f.n1];
		f.s2=Vertices[f.n2];
		f.ref=obj->Faces[n].ref;

		VECTOR3MATRIX(obj->Faces[n].NormCalc,obj->Faces[n].Norm,Mrot);
		VECTORSUB(u,L,P);
		if (ss*DOT(u,obj->Faces[n].NormCalc)>0)
		{
			faces.Add(f);
		}
		else faces_inshadow.Add(f);
	}

	r=roots.GetFirst();
	while (r)
	{
#ifdef DECOUPAGEDABORD
		Plans.Free();
		PlaneList(*r);
		CutFaces(0,&faces);
#endif
		
		faces_inlight.Free();
		/*
		ff=faces.GetFirst();
		while (ff)
		{
			if (!RootShadowedFace(*r,ff))
			{
				faces_inlight.Add(*ff);
				ff=faces.DeleteAndGetNext();
			}
			else
			{
				ff=faces.GetNext();
			}
		}
		/**/
		if (faces.Length()>0)
		{
			ApplyToFaces((*r),&faces);
		}

		faces.Free();
		
		ff=faces_inlight.GetFirst();
		while (ff)
		{
			faces.Add(*ff);
			ff=faces_inlight.GetNext();
		}
		
		r=roots.GetNext();
	}

	nVertices=Vertices.Length();

	if ((faces_inlight.Length()==0)||(faces_inshadow.Length()==0))
	{
		res=obj->Duplicate2();

		if (faces_inshadow.Length()==0)
		{
			for (n=0;n<res->nFaces;n++)
				res->Faces[n].tag+=tag;
		}
	}
	else
	{
		res->Init(nVertices,faces_inshadow.Length()+faces_inlight.Length());

		n=0;
		ps=Vertices.GetFirst();
		while (ps)
		{
			res->Vertices[n].Stok=ps->Stok;
			res->Vertices[n].Map=ps->Map;
			res->Vertices[n].Map2=ps->Map2;
			n++;
			ps=Vertices.GetNext();
		}

		n=0;
		ff=faces_inshadow.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inshadow.GetNext();
			n++;
		}

		ff=faces_inlight.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag+tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inlight.GetNext();
			n++;
		}

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
	}

	res->SetFaces();
	res->CalculateNormals(-1);

	nVertices=0;
	faces_inlight.Free();
	faces_inshadow.Free();
	faces.Free();
	Vertices.Free();
	cutted_faces.Free();

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPLight::RootApplyDirectionnal(CBSPLightNode *thisroot,CObject3D * obj,int tag,CVector &Ldir)
{
	CObject3D *res;
	int n,nf;
	CVector u,N,P;
	CList <CShortFace> faces;
	CShortFace f,*ff;
	CVertex s,*ps;
	CBSPLightNode **r;
	CMatrix Mobj;
	float ss;	

	if (obj->Status!=0) ss=-1; else ss=1;

	Mobj.Id();
	Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);

	faces_inshadow.Free();
	faces.Free();
	Vertices.Free();
	nVertices=0;

	for (n=0;n<obj->nVertices;n++)
	{
		VECTOR3MATRIX(obj->Vertices[n].Calc,obj->Vertices[n].Stok,Mobj);
		s.Stok=obj->Vertices[n].Stok;
		s.Calc=obj->Vertices[n].Calc;
		s.Map=obj->Vertices[n].Map;
		s.Map2=obj->Vertices[n].Map2;
		Vertices.Add(s);
	}

	nf=0;
	for (n=0;n<obj->nFaces;n++)
	{
		VECTOR3MATRIX3X3(obj->Faces[n].NormCalc,obj->Faces[n].Norm,Mobj);
		f.n0=obj->Faces[n].v0;
		f.n1=obj->Faces[n].v1;
		f.n2=obj->Faces[n].v2;
		f.nT=obj->Faces[n].nT;
		f.nL=obj->Faces[n].nL;
		f.tag=obj->Faces[n].tag;
		f.s0=Vertices[f.n0];
		f.s1=Vertices[f.n1];
		f.s2=Vertices[f.n2];
		f.ref=obj->Faces[n].ref;

		if (VECTORNORM2(obj->Faces[n].Norm)<0.01f)
		{
			obj->Faces[n].tag2=-1;
            facing.Add(f);
            nf++;

		}
		else
		{
			if ((ss*DOT(Ldir,obj->Faces[n].NormCalc)<0)&&(obj->Faces[n].tag&tag))
			{
                facing.Add(f);
                nf++;

				obj->Faces[n].tag2=0;
			}
			else
			{
				if (f.tag&tag) f.tag-=tag;
				faces_inshadow.Add(f);
				obj->Faces[n].tag2=1;
			}
		}
	}

	r=&thisroot;
	faces_inlight.Free();

	if (nf>0)
	{
		Plans.Free();
		PlaneList(thisroot);
		CutFacesFast(0,&facing,nf,-1);
		ShadowTraitment(thisroot,&cutted_faces,1,tag);		
		cutted_faces.Free();
	}

	nVertices=Vertices.Length();

	if ((faces_inlight.Length()==0)||(faces_inshadow.Length()==0))
	{
		res=obj->Duplicate2();

		if (faces_inlight.Length()==0)
		{
			for (n=0;n<res->nFaces;n++)
				if (res->Faces[n].tag&tag) res->Faces[n].tag-=tag;
		}

	}
	else
	{

		res=new CObject3D;

		res->Rot=obj->Rot;
		res->Coo=obj->Coo;

		res->Init(nVertices,faces_inshadow.Length()+faces_inlight.Length());

		n=0;
		ps=Vertices.GetFirst();
		while (ps)
		{
			res->Vertices[n].Stok=ps->Stok;
			res->Vertices[n].Map=ps->Map;
			res->Vertices[n].Map2=ps->Map2;
			n++;
			ps=Vertices.GetNext();
		}

		n=0;
		ff=faces_inshadow.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inshadow.GetNext();
			n++;
		}

		ff=faces_inlight.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inlight.GetNext();
			n++;
		}
	}

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

	res->SetFaces();
	//res->CalculateNormals(-1);

	nVertices=0;
	faces_inlight.Free();
	faces_inshadow.Free();
	faces.Free();
    facing.Free();
	Vertices.Free();
	cutted_faces.Free();

	Plans.Free();

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPLight::RootApply(CBSPLightNode *thisroot,CObject3D * obj,int tag,CVector &L)
{
	CObject3D *res;
	int n,nf;
	CVector u,N,P;
	CList <CShortFace> faces;
	CShortFace f,*ff;
	CVertex s,*ps;
	CBSPLightNode **r;
	CMatrix Mobj;
	float ss=1;
	if (obj->Status!=0) ss=-1;

	Mobj.Id();
	Mobj.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
	Mobj.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);

    facing.Free();
	faces_inshadow.Free();
	faces.Free();
	Vertices.Free();
	nVertices=0;

	for (n=0;n<obj->nVertices;n++)
	{
		VECTOR3MATRIX(obj->Vertices[n].Calc,obj->Vertices[n].Stok,Mobj);
		s.Stok=obj->Vertices[n].Stok;
		s.Calc=obj->Vertices[n].Calc;
		s.Map=obj->Vertices[n].Map;
		s.Map2=obj->Vertices[n].Map2;
		Vertices.Add(s);
	}

	nf=0;
	for (n=0;n<obj->nFaces;n++)
	{
		VECTORADDDIV3(P,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[2]->Calc);
		f.n0=obj->Faces[n].v0;
		f.n1=obj->Faces[n].v1;
		f.n2=obj->Faces[n].v2;
		f.nT=obj->Faces[n].nT;
		f.nL=obj->Faces[n].nL;
		f.tag=obj->Faces[n].tag;
		f.s0=Vertices[f.n0];
		f.s1=Vertices[f.n1];
		f.s2=Vertices[f.n2];
		f.ref=obj->Faces[n].ref;

		VECTOR3MATRIX3X3(obj->Faces[n].NormCalc,obj->Faces[n].Norm,Mobj);
		VECTORSUB(u,L,P);

		if (VECTORNORM2(obj->Faces[n].Norm)<0.01f)
		{
			obj->Faces[n].tag2=-1;
            facing.Add(f);
            nf++;
		}
		else
		{
			if ((ss*DOT(u,obj->Faces[n].NormCalc)>0)&&(obj->Faces[n].tag&tag))
			{
                facing.Add(f);
                nf++;
				obj->Faces[n].tag2=0;
			}
			else
			{
				if (f.tag&tag) f.tag-=tag;
				faces_inshadow.Add(f);
				obj->Faces[n].tag2=1;
			}
		}
	}

	r=&thisroot;
	faces_inlight.Free();
	
	if (nf>0)
	{		
		Plans.Free();
		PlaneList(thisroot);
		CutFacesFast(0,&facing,nf,-1);
		ShadowTraitment(thisroot,&cutted_faces,-1,tag);
		cutted_faces.Free();
	}

	nVertices=Vertices.Length();

	if ((faces_inlight.Length()==0)||(faces_inshadow.Length()==0))
	{
		res=obj->Duplicate2();

		if (faces_inlight.Length()==0)
		{
			for (n=0;n<res->nFaces;n++)
				if (res->Faces[n].tag&tag) res->Faces[n].tag-=tag;
		}
	}
	else
	{
		res=new CObject3D;

		res->Rot=obj->Rot;
		res->Coo=obj->Coo;

		res->Init(nVertices,faces_inshadow.Length()+faces_inlight.Length());

		n=0;
		ps=Vertices.GetFirst();
		while (ps)
		{
			res->Vertices[n].Stok=ps->Stok;
			res->Vertices[n].Map=ps->Map;
			res->Vertices[n].Map2=ps->Map2;
			n++;
			ps=Vertices.GetNext();
		}

		n=0;
		ff=faces_inshadow.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inshadow.GetNext();
			n++;
		}

		ff=faces_inlight.GetFirst();
		while (ff)
		{
			res->Faces[n].v0=ff->n0;
			res->Faces[n].v1=ff->n1;
			res->Faces[n].v2=ff->n2;
			res->Faces[n].nT=ff->nT;
			res->Faces[n].nL=ff->nL;
			res->Faces[n].tag=ff->tag;
			res->Faces[n].ref=ff->ref;
			ff=faces_inlight.GetNext();
			n++;
		}
	}

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

	res->SetFaces();
	//res->CalculateNormals(-1);

	nVertices=0;
	faces_inlight.Free();
	faces_inshadow.Free();
	faces.Free();
    facing.Free();
	Vertices.Free();
	cutted_faces.Free();

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::ApplyToFaces(CBSPLightNode * node,CList <CShortFace> *faces)
{
	int res;
	int nx;
	CList <CShortFace> plus,moins;
	CShortFace *f;
	CShortFace tmp;
	CVertex sh;
	CVector I;
	CVertex *P1,*P2,*P3;
	CVertex *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int nAB,nAC;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int n;
	int * VerticesIndices;
	CVertex* *VerticesPointeurs;
	EdgeList2 Edges;

	nx=0;
	if (node->type==NODE)
	{
		Edges.Init(faces->Length()*3);
		f=faces->GetFirst();
		while (f)
		{
			Edges.AddEdge(f->n0,f->n1);
			Edges.AddEdge(f->n1,f->n2);
			Edges.AddEdge(f->n2,f->n0);
			f=faces->GetNext();
		}

		VerticesIndices=(int*) malloc(sizeof(int)*Edges.nList);
		VerticesPointeurs=(CVertex**) malloc(Edges.nList*sizeof(CVertex*));

		for (n=0;n<Edges.nList;n++) VerticesIndices[n]=-1;
		for (n=0;n<Edges.nList;n++) VerticesPointeurs[n]=NULL;

		f=faces->GetFirst();
		while (f)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->Calc.x*node->a +P1->Calc.y*node->b +P1->Calc.z*node->c + node->d;
			s2=P2->Calc.x*node->a +P2->Calc.y*node->b +P2->Calc.z*node->c + node->d;
			s3=P3->Calc.x*node->a +P3->Calc.y*node->b +P3->Calc.z*node->c + node->d;

			cc=(s1>=-SMALLFBSPLIGHT)+(s2>=-SMALLFBSPLIGHT)+(s3>=-SMALLFBSPLIGHT);

			if (cc==0)
			{
				moins.Add(*f);
			}

			if (cc==3)
			{
				plus.Add(*f);
			}

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;
				t1=t2=-1;
				res=0;

				if (s*s1>=-s*SMALLFBSPLIGHT)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
				}
				ss=s1;

				if (((s*s2>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s2>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;
				}

				if (((s*s3>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s3>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s3>s*ss)))
				{
					A=f->n2;B=f->n0;C=f->n1;
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3;
					PB=P1;
					PC=P2;
				}

				t1=-ss1/(ss2-ss1);
				t2=-ss1/(ss3-ss1);

				if (res==0)
				{

				}

				if ((t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
				{
					if (cc==1)
						moins.Add(*f);
					else
						plus.Add(*f);

					nx++;
				}
				else
				if ((1-t1<SMALLFBSPLIGHT2)&&(1-t2<SMALLFBSPLIGHT2))
				{
					if (cc==1)
						plus.Add(*f);
					else
						moins.Add(*f);

					nx++;
				}
				else
				{

					nAB=Edges.WitchEdge(A,B);
					nAC=Edges.WitchEdge(A,C);

					iAB=VerticesIndices[nAB];
					iAC=VerticesIndices[nAC];

					if (iAB==-1)
					{
						t=-ss1/(ss2-ss1);
						VECTORINTERPOL(sh.Stok,t,PB->Stok,PA->Stok);
						VECTORINTERPOL(sh.Calc,t,PB->Calc,PA->Calc);
						VECTOR2INTERPOL(sh.Map,t,PB->Map,PA->Map);
						VECTOR2INTERPOL(sh.Map2,t,PB->Map2,PA->Map2);
						nVertices=Vertices.Length();
						sAB=Vertices.Add(sh);
						VerticesIndices[nAB]=nVertices;
						iAB=nVertices;
						VerticesPointeurs[nAB]=sAB;
					}
					else sAB=VerticesPointeurs[nAB];


					if (iAC==-1)
					{
						t=-ss1/(ss3-ss1);
						VECTORINTERPOL(sh.Stok,t,PC->Stok,PA->Stok);
						VECTORINTERPOL(sh.Calc,t,PC->Calc,PA->Calc);
						VECTOR2INTERPOL(sh.Map,t,PC->Map,PA->Map);
						VECTOR2INTERPOL(sh.Map2,t,PC->Map2,PA->Map2);

						nVertices=Vertices.Length();
						sAC=Vertices.Add(sh);
						VerticesIndices[nAC]=nVertices;
						VerticesPointeurs[nAC]=sAC;
						iAC=nVertices;
					}
					else sAC=VerticesPointeurs[nAC];


					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.tag=f->tag;
					tmp.ref=f->ref;

					if (cc==1)
					{
						tmp.n0=A;
						tmp.n1=iAB;
						tmp.n2=iAC;
						tmp.s0=PA;
						tmp.s1=sAB;
						tmp.s2=sAC;
						plus.Add(tmp);

						tmp.n0=iAC;
						tmp.n1=iAB;
						tmp.n2=B;
						tmp.s0=sAC;
						tmp.s1=sAB;
						tmp.s2=PB;

						moins.Add(tmp);

						tmp.n0=iAC;
						tmp.n1=B;
						tmp.n2=C;
						tmp.s0=sAC;
						tmp.s1=PB;
						tmp.s2=PC;

						moins.Add(tmp);
					}
					else
					{
						tmp.n0=A;
						tmp.n1=iAB;
						tmp.n2=iAC;
						tmp.s0=PA;
						tmp.s1=sAB;
						tmp.s2=sAC;

						moins.Add(tmp);

						tmp.n0=iAC;
						tmp.n1=iAB;
						tmp.n2=B;
						tmp.s0=sAC;
						tmp.s1=sAB;
						tmp.s2=PB;

						plus.Add(tmp);

						tmp.n0=iAC;
						tmp.n1=B;
						tmp.n2=C;
						tmp.s0=sAC;
						tmp.s1=PB;
						tmp.s2=PC;

						plus.Add(tmp);
					}
				}
			}
			f=faces->GetNext();
		}

		free(VerticesIndices);
		free(VerticesPointeurs);
		Edges.Free();

		if (plus.Length()>0) ApplyToFaces(node->plus,&plus);
		plus.Free();

		if (moins.Length()>0) ApplyToFaces(node->moins,&moins);
		moins.Free();

	}
	else
	{
		if (node->type==SHADOW)
		{
			f=faces->GetFirst();
			while (f)
			{
				faces_inshadow.Add(*f);
				f=faces->GetNext();
			}
		}
		else
		{
			f=faces->GetFirst();
			while (f)
			{
				faces_inlight.Add(*f);
				f=faces->GetNext();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::ApplyToFacesGruged(CBSPLightNode * node,CList <CShortFace> *faces)
{
	int res;
	CList <CShortFace> plus,moins;
	CShortFace *f;
	CShortFace tmp;
	CVertex sh;
	CVector I;
	CVertex *P1,*P2,*P3;
	CVertex *PA,*PB,*PC,*sAB,*sAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t1,t2;
	int cc;

	if (faces_in_shadows_tag) return;

	if (node->type==NODE)
	{
		f=faces->GetFirst();
		while (f)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->Calc.x*node->a +P1->Calc.y*node->b +P1->Calc.z*node->c + node->d;
			s2=P2->Calc.x*node->a +P2->Calc.y*node->b +P2->Calc.z*node->c + node->d;
			s3=P3->Calc.x*node->a +P3->Calc.y*node->b +P3->Calc.z*node->c + node->d;

			cc=(s1>=-SMALLFBSPLIGHT)+(s2>=-SMALLFBSPLIGHT)+(s3>=-SMALLFBSPLIGHT);

			if (cc==0)
			{
				if (node->moins->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else moins.Add(*f);
			}

			if (cc==3)
			{
				if (node->plus->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else plus.Add(*f);
			}

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;
				t1=t2=-1;
				res=0;

				if (s*s1>=-s*SMALLFBSPLIGHT)
				{
					PA=P1; PB=P2; PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
				}
				ss=s1;

				if (((s*s2>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s2>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2; PB=P3; PC=P1;
				}

				if (((s*s3>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s3>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s3>s*ss)))
				{
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3; PB=P1; PC=P2;
				}

				t1=-ss1/(ss2-ss1);
				t2=-ss1/(ss3-ss1);

				if ((t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
				{
					if (cc==1)
					{
						if (node->moins->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else moins.Add(*f);
					}
					else
					{
						if (node->plus->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else plus.Add(*f);
					}
				}
				else
				if ((1-t1<SMALLFBSPLIGHT2)&&(1-t2<SMALLFBSPLIGHT2))
				{
					if (cc==2)
					{
						if (node->moins->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else moins.Add(*f);
					}
					else
					{
						if (node->plus->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; } else plus.Add(*f);
					}
				}
				else
				{
					if (node->moins->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; }
					if (node->plus->type==SHADOW) { faces_in_shadows_tag=1; plus.Free();moins.Free(); return; }

					VECTORINTERPOL(sh.Calc,t1,PB->Calc,PA->Calc);
					sAB=Vertices.Add(sh);
				
					VECTORINTERPOL(sh.Calc,t2,PC->Calc,PA->Calc);
					sAC=Vertices.Add(sh);

					if (cc==1)
					{
						tmp.s0=PA; tmp.s1=sAB; tmp.s2=sAC;
						plus.Add(tmp);

						tmp.s0=sAC; tmp.s1=sAB; tmp.s2=PB;
						moins.Add(tmp);

						tmp.s0=sAC; tmp.s1=PB; tmp.s2=PC;
						moins.Add(tmp);
					}
					else
					{
						tmp.s0=PA; tmp.s1=sAB; tmp.s2=sAC;
						moins.Add(tmp);

						tmp.s0=sAC; tmp.s1=sAB; tmp.s2=PB;
						plus.Add(tmp);

						tmp.s0=sAC; tmp.s1=PB; tmp.s2=PC;
						plus.Add(tmp);
					}
				}
			}
			f=faces->GetNext();
		}

		if (plus.Length()>0) ApplyToFacesGruged(node->plus,&plus);
		plus.Free();

		if (faces_in_shadows_tag==0)
		{
			if (moins.Length()>0) ApplyToFacesGruged(node->moins,&moins);			
		}
		moins.Free();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::ShadowTraitment(CBSPLightNode * node,CList <CShortFace> *faces,float mul0,int tag)
{
	CList <CShortFace> plus,moins;
	CShortFace *f;
	CShortFace ftmp;
	CVertex *P1,*P2,*P3;
	CVector P;
	float ss;
	float mul=-1;

	if (node->type==NODE)
	{
		f=faces->GetFirst();
		while (f)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			VECTORADDDIV3(P,P1->Calc,P2->Calc,P3->Calc);
			ss=P.x*node->a +P.y*node->b +P.z*node->c + node->d;

			if (ss>SMALLFBSPLIGHTCOMBINE) plus.Add(*f);
			else moins.Add(*f);

			f=faces->GetNext();
		}

		if (plus.Length()>0) ShadowTraitment(node->plus,&plus,mul,tag);
		plus.Free();

		if (moins.Length()>0) ShadowTraitment(node->moins,&moins,mul,tag);
		moins.Free();

	}
	else
	{
		if (node->type==SHADOW)
		{
			f=faces->GetFirst();
			while (f)
			{
				ftmp=*f;
				if (ftmp.tag&tag) ftmp.tag-=tag;

				faces_inshadow.Add(ftmp);
				f=faces->GetNext();
			}
		}
		else
		{
			f=faces->GetFirst();
			while (f)
			{
				faces_inlight.Add(*f);
				f=faces->GetNext();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPLight::FaceShadowTraitment(CBSPLightNode * node,CShortFace *f,float mul0,int tag)
{
	CVertex *P1,*P2,*P3;
	CVector P;
	float ss;
	float mul=-1;
	bool res=false;

	if (node->type==NODE)
	{
		P1=f->s0;
		P2=f->s1;
		P3=f->s2;
		VECTORADDDIV3(P,P1->Calc,P2->Calc,P3->Calc);
		ss=P.x*node->a +P.y*node->b +P.z*node->c + node->d;
		if (ss>SMALLFBSPLIGHTCOMBINE) res=FaceShadowTraitment(node->plus,f,mul,tag);
		else res=FaceShadowTraitment(node->moins,f,mul,tag);
	}
	else
	{
		if (node->type==SHADOW)
		{
			if (f->tag&tag) f->tag-=tag;
			res=true;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Valid(CShortFace * f)
{
	if ((f->n0==f->n1)||(f->n0==f->n2)||(f->n1==f->n2)) return false;
	else return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::CutFacesFast(int nP,CList <CShortFace> *faces,int NF,float mul)
{
	int res;
	int nx;
	CShortFace *f;
	CShortFace tmp;
	CVertex sh;
	float a,b,c,d;
	CVector I;
	CVertex *P1,*P2,*P3;
	CVertex *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int nAB,nAC;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t1,t2;
	int cc;
	int * VerticesIndices;
	CVertex* *VerticesPointeurs;
	EdgeList Edges;
	int nb,LEN0;
	int nf;
	int n;
	float mul0;

	mul0=-1;

	nf=NF;
	LEN0=NF;

	nx=0;
	if (nP<Plans.Length())
	{
		a=Plans[nP]->a;
		b=Plans[nP]->b;
		c=Plans[nP]->c;
		d=Plans[nP]->d;

		Edges.Init(LEN0*3);

        f=faces->GetFirst();
        while (f)
        {
			Edges.AddEdge2(f->n0,f->n1);
			Edges.AddEdge2(f->n1,f->n2);
			Edges.AddEdge2(f->n2,f->n0);
            f=faces->GetNext();
		}

		VerticesIndices=(int*) malloc(sizeof(int)*Edges.nList);
		VerticesPointeurs=(CVertex**) malloc(Edges.nList*sizeof(CVertex*));

		for (n=0;n<Edges.nList;n++) VerticesIndices[n]=-1;
		for (n=0;n<Edges.nList;n++) VerticesPointeurs[n]=NULL;

		CVertex * v=Vertices.GetFirst();
		while (v)
		{
			v->coef1=v->Calc.x*a +v->Calc.y*b +v->Calc.z*c + d;
			v=Vertices.GetNext();
		}

        f=faces->GetFirst();
		for (nb=0;nb<LEN0;nb++)
		{
			P1=f->s0; P2=f->s1; P3=f->s2;

			s1=P1->coef1; s2=P2->coef1; s3=P3->coef1;
			cc=(s1>=mul*SMALLFBSPLIGHT)+(s2>=mul*SMALLFBSPLIGHT)+(s3>=mul*SMALLFBSPLIGHT);

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;

				if (s*s1>=-s*SMALLFBSPLIGHT)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
				}
				ss=s1;

				if (((s*s2>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s2>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;
				}

				if (((s*s3>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s3>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s3>s*ss)))
				{
					A=f->n2;B=f->n0;C=f->n1;
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3;
					PB=P1;
					PC=P2;
				}

				t1=-ss1/(ss2-ss1);
				t2=-ss1/(ss3-ss1);

				if ((t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
				{
				}
				else
				if ((1.0f-t1<SMALLFBSPLIGHT2)&&(1.0f-t2<SMALLFBSPLIGHT2))
				{
				}
				else
				if ((t1<SMALLFBSPLIGHT2)&&(1.0f-t2<SMALLFBSPLIGHT2))
				{
				}
				else
				if ((1.0f-t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
				{
				}
				else
				{

					nAB=Edges.WitchEdge(A,B);
					nAC=Edges.WitchEdge(A,C);

					iAB=VerticesIndices[nAB];
					iAC=VerticesIndices[nAC];

					if (iAB==-1)
					{
						if (t1<SMALLFBSPLIGHT2)
						{
							sAB=PA;
							iAB=A;
							VerticesIndices[nAB]=iAB;
							VerticesPointeurs[nAB]=sAB;
						}
						else
						if (t1>1.0f-SMALLFBSPLIGHT2)
						{
							sAB=PB;
							iAB=B;
							VerticesIndices[nAB]=iAB;
							VerticesPointeurs[nAB]=sAB;
						}
						else
						{
							VECTORINTERPOL(sh.Stok,t1,PB->Stok,PA->Stok);
							VECTORINTERPOL(sh.Calc,t1,PB->Calc,PA->Calc);
							VECTOR2INTERPOL(sh.Map,t1,PB->Map,PA->Map);
							VECTOR2INTERPOL(sh.Map2,t1,PB->Map2,PA->Map2);

							nVertices=Vertices.Length();
							sAB=Vertices.Add(sh);
							VerticesIndices[nAB]=nVertices;
							iAB=nVertices;
							VerticesPointeurs[nAB]=sAB;
						}
					}
					else sAB=VerticesPointeurs[nAB];


					if (iAC==-1)
					{
						if (t2<SMALLFBSPLIGHT2)
						{
							sAC=PA;
							iAC=A;
							VerticesIndices[nAC]=iAC;
							VerticesPointeurs[nAC]=sAC;
						}
						else
						if (t2>1.0f-SMALLFBSPLIGHT2)
						{
							sAC=PC;
							iAC=C;
							VerticesIndices[nAC]=iAC;
							VerticesPointeurs[nAC]=sAC;
						}
						else
						{
							VECTORINTERPOL(sh.Stok,t2,PC->Stok,PA->Stok);
							VECTORINTERPOL(sh.Calc,t2,PC->Calc,PA->Calc);
							VECTOR2INTERPOL(sh.Map,t2,PC->Map,PA->Map);
							VECTOR2INTERPOL(sh.Map2,t2,PC->Map2,PA->Map2);

							nVertices=Vertices.Length();
							sAC=Vertices.Add(sh);
							VerticesIndices[nAC]=nVertices;
							VerticesPointeurs[nAC]=sAC;
							iAC=nVertices;
						}
					}
					else sAC=VerticesPointeurs[nAC];

					f->n0=A;
					f->n1=iAB;
					f->n2=iAC;
					f->s0=PA;
					f->s1=sAB;
					f->s2=sAC;
                    
                    CShortFace *f1=faces->InsertLast();
                    CShortFace *f2=faces->InsertLast();
                    
					f1->n0=iAC;
					f1->n1=iAB;
					f1->n2=B;
					f1->s0=sAC;
					f1->s1=sAB;
					f1->s2=PB;
					f1->nL=f->nL;
					f1->nT=f->nT;
					f1->tag=f->tag;
					f1->ref=f->ref;
					f1->N=f->N;

					f2->n0=iAC;
					f2->n1=B;
					f2->n2=C;
					f2->s0=sAC;
					f2->s1=PB;
					f2->s2=PC;
					f2->nL=f->nL;
					f2->nT=f->nT;
					f2->tag=f->tag;
					f2->ref=f->ref;
					f2->N=f->N;

					nf+=2;
				}
			}
            
            f=faces->GetNext();
		}

		free(VerticesIndices);
		free(VerticesPointeurs);
		Edges.Free();

		CutFacesFast(nP+1,faces,nf,mul);
	}
	else
	{
		cutted_faces.Free();
        f=faces->GetFirst();
        while (f)
        {
            cutted_faces.Add(*f);
            f=faces->GetNext();
        }
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::CutFaces(int nP,CList <CShortFace> *faces)
{
	int res;
	int nx;
	CList <CShortFace> newliste;
	CShortFace *f;
	CShortFace tmp;
	CVertex sh;
	float a,b,c,d;
	CVector I;
	CVertex *P1,*P2,*P3;
	CVertex *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int nAB,nAC;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int n;
	int * VerticesIndices;
	CVertex* *VerticesPointeurs;
	EdgeList2 Edges;

#define SMALLF0 SMALLFBSPLIGHT2

	nx=0;

	if (nP<Plans.Length())
	{

		Edges.Init(faces->Length()*3);
		f=faces->GetFirst();
		while (f)
		{
			Edges.AddEdge(f->n0,f->n1);
			Edges.AddEdge(f->n1,f->n2);
			Edges.AddEdge(f->n2,f->n0);
			f=faces->GetNext();
		}

		VerticesIndices=(int*) malloc(sizeof(int)*Edges.nList);
		VerticesPointeurs=(CVertex**) malloc(Edges.nList*sizeof(CVertex*));

		for (n=0;n<Edges.nList;n++) VerticesIndices[n]=-1;
		for (n=0;n<Edges.nList;n++) VerticesPointeurs[n]=NULL;

		a=Plans[nP]->a;
		b=Plans[nP]->b;
		c=Plans[nP]->c;
		d=Plans[nP]->d;

		f=faces->GetFirst();
		while (f)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->Calc.x*a +P1->Calc.y*b +P1->Calc.z*c + d;
			s2=P2->Calc.x*a +P2->Calc.y*b +P2->Calc.z*c + d;
			s3=P3->Calc.x*a +P3->Calc.y*b +P3->Calc.z*c + d;

			cc=(s1>=-SMALLFBSPLIGHT)+(s2>=-SMALLFBSPLIGHT)+(s3>=-SMALLFBSPLIGHT);


			if (cc==0)
			{
				newliste.Add(*f);
			}

			if (cc==3)
			{
				newliste.Add(*f);
			}

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;
				ss=0;

				if (s*s1>=-s*SMALLFBSPLIGHT)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
					ss=s1;
				}

				if (((s*s2>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s2>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;
				}

				if (((s*s3>=-s*SMALLFBSPLIGHT)&&(res==0))||((s*s3>=-s*SMALLFBSPLIGHT)&&(res==1)&&(s*s3>s*ss)))
				{
					A=f->n2;B=f->n0;C=f->n1;
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3;
					PB=P1;
					PC=P2;
				}

				t1=-ss1/(ss2-ss1);
				t2=-ss1/(ss3-ss1);

				if ((t1<SMALLF0)&&(t2<SMALLF0))
				{
					newliste.Add(*f);
					nx++;
				}
				else
				if ((1.0f-t1<SMALLF0)&&(1.0f-t2<SMALLF0))
				{
					newliste.Add(*f);
					nx++;
				}
				else
				if ((t1<SMALLF0)&&(1.0f-t2<SMALLF0))
				{
					newliste.Add(*f);
					nx++;
				}
				else
				if ((1.0f-t1<SMALLF0)&&(t2<SMALLF0))
				{
					newliste.Add(*f);
					nx++;
				}
				else
				{
					nAB=Edges.WitchEdge(A,B);
					nAC=Edges.WitchEdge(A,C);

					iAB=VerticesIndices[nAB];
					iAC=VerticesIndices[nAC];

					if (iAB==-1)
					{
						t=-ss1/(ss2-ss1);
						if (t<SMALLF0)
						{
							sAB=PA;
							iAB=A;
							VerticesIndices[nAB]=iAB;
							VerticesPointeurs[nAB]=sAB;
						}
						else
						if (t>1.0f-SMALLF0)
						{
							sAB=PB;
							iAB=B;
							VerticesIndices[nAB]=iAB;
							VerticesPointeurs[nAB]=sAB;
						}
						else
						{
							VECTORINTERPOL(sh.Stok,t,PB->Stok,PA->Stok);
							VECTORINTERPOL(sh.Calc,t,PB->Calc,PA->Calc);
							VECTOR2INTERPOL(sh.Map,t,PB->Map,PA->Map);
							VECTOR2INTERPOL(sh.Map2,t,PB->Map2,PA->Map2);

							nVertices=Vertices.Length();
							sAB=Vertices.Add(sh);
							VerticesIndices[nAB]=nVertices;
							iAB=nVertices;
							VerticesPointeurs[nAB]=sAB;
						}
					}
					else sAB=VerticesPointeurs[nAB];

					if (iAC==-1)
					{
						t=-ss1/(ss3-ss1);
						if (t<SMALLF0)
						{
							sAC=PA;
							iAC=A;
							VerticesIndices[nAC]=iAC;
							VerticesPointeurs[nAC]=sAC;
						}
						else
						if (t>1.0f-SMALLF0)
						{
							sAC=PC;
							iAC=C;
							VerticesIndices[nAC]=iAC;
							VerticesPointeurs[nAC]=sAC;
						}
						else
						{
							VECTORINTERPOL(sh.Stok,t,PC->Stok,PA->Stok);
							VECTORINTERPOL(sh.Calc,t,PC->Calc,PA->Calc);
							VECTOR2INTERPOL(sh.Map,t,PC->Map,PA->Map);
							VECTOR2INTERPOL(sh.Map2,t,PC->Map2,PA->Map2);

							nVertices=Vertices.Length();
							sAC=Vertices.Add(sh);
							VerticesIndices[nAC]=nVertices;
							VerticesPointeurs[nAC]=sAC;
							iAC=nVertices;
						}
					}
					else sAC=VerticesPointeurs[nAC];

					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.tag=f->tag;
					tmp.ref=f->ref;

					tmp.n0=A;
					tmp.n1=iAB;
					tmp.n2=iAC;
					tmp.s0=PA;
					tmp.s1=sAB;
					tmp.s2=sAC;
					newliste.Add(tmp);

					tmp.n0=iAC;
					tmp.n1=iAB;
					tmp.n2=B;
					tmp.s0=sAC;
					tmp.s1=sAB;
					tmp.s2=PB;
					newliste.Add(tmp);

					tmp.n0=iAC;
					tmp.n1=B;
					tmp.n2=C;
					tmp.s0=sAC;
					tmp.s1=PB;
					tmp.s2=PC;
					newliste.Add(tmp);
				}
			}
			f=faces->GetNext();
		}

		free(VerticesIndices);
		free(VerticesPointeurs);
		Edges.Free();

		CutFaces(nP+1,&newliste);
		newliste.Free();
	}
	else
	{
		f=faces->GetFirst();
		while (f)
		{
			cutted_faces.Add(*f);
			f=faces->GetNext();
		}

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PLUS 1
#define MOINS 2
#define PLUSMOINS 3
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TestSurface(CTriangles * first,CTriangles *s)
{
	int cc;
	float d;
	CTriangle * tri;
	int nb;

	INVDOTPRODUCT(d,first->Norm,first->Base);
	//d=-(first->Norm||first->Base);

	nb=0;
	cc=0;
	if (s->tag==NORMALFACE)
	{
		tri=s->List.GetFirst2();
		while (tri)
		{
			nb+=3;
			/*
			cc+=(((tri->A||first->Norm)+d)>-SMALLFBSPLIGHT);
			cc+=(((tri->B||first->Norm)+d)>-SMALLFBSPLIGHT);
			cc+=(((tri->C||first->Norm)+d)>-SMALLFBSPLIGHT);
			/**/
			cc+=((DOT(tri->A,first->Norm)+d)>=-SMALLFBSPLIGHT);
			cc+=((DOT(tri->B,first->Norm)+d)>=-SMALLFBSPLIGHT);
			cc+=((DOT(tri->C,first->Norm)+d)>=-SMALLFBSPLIGHT);

			tri=s->List.GetNext2();
		}
	}

	if (cc==0) return MOINS;
	else
		if (cc==nb) return PLUS;
		else
			return PLUSMOINS;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::Add(CList <CTriangles> * faces,CVector &L)
{
	roots.Add(BuildNode(faces,L));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::Build(CList <CTriangles> * faces,CVector &L)
{
	root=BuildNode(faces,L);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::FreeNode(CBSPLightNode * n)
{
	if (n!=NULL)
	{
		if (n->type==NODE)
		{
			FreeNode(n->plus);
			FreeNode(n->moins);
		}

		delete n;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::NodeIndice(CBSPLightNode * n)
{
	if (n!=NULL)
	{
		n->index=compteur;
		compteur++;
		if (n->type==NODE)
		{
			NodeIndice(n->plus);
			NodeIndice(n->moins);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::getNode(CBSPLightNode * n,int index,CBSPLightNode **result)
{
	if (n!=NULL)
	{
		if (n->index==index) *result=n;
		else
		{
			if (n->type==NODE)
			{
				getNode(n->plus,index,result);
				getNode(n->moins,index,result);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::Duplicate(CBSPLight * bsp)
{
	CBSPLightNode **r;

	if (bsp)
	{
		roots.Free();

		r=bsp->roots.GetFirst();
		while (r)
		{
			roots.Add(*r);	
			r=bsp->roots.GetNext();
		}

		dup=true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPLight::Free()
{
	CBSPLightNode **r;

	if (!dup)
	{
		r=roots.GetFirst();
		while (r)
		{
			FreeNode(*r);
			r=roots.GetNext();
		}
	}

	roots.Free();
    facing.Free();
	Vertices.Free();
	faces_inlight.Free();
	faces_inshadow.Free();
	cutted_faces.Free();
	Plans.Free();

	dup=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CotePlusTriangle(CTriangle * tri,float a,float b,float c,float d)
{
	CVector p;
	float s;

	VECTORADDDIV3(p,tri->A,tri->B,tri->C);

	s=a*p.x+b*p.y+c*p.z+d;

	if (s>-SMALLFBSPLIGHT) return true;
	else return false;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DecoupeTRIANGLE3(float mul,CTriangles * first,CTriangles * group,CTriangle * surf,float a,float b,float c,float d,
					  CList <CTriangle> *plus,CList <CTriangle> *moins)
{
	int res;
	float s,s1,s2,s3,ss;
	float t1,t2;
	CTriangle tri;
	CVector A,B,C,I,J;
	int cc;
	float mul2;
	t1=1;
	t2=1;

	mul2=-1;

	s1=surf->A.x*a +surf->A.y*b +surf->A.z*c +d;
	s2=surf->B.x*a +surf->B.y*b +surf->B.z*c +d;
	s3=surf->C.x*a +surf->C.y*b +surf->C.z*c +d;

	cc=(s1>=mul*SMALLFBSPLIGHT)+(s2>=mul*SMALLFBSPLIGHT)+(s3>=mul*SMALLFBSPLIGHT);

	if ((cc==0)||(cc==3))
	{
		if (cc==0)
		{
			moins->Add(*surf);
		}
		else
		{
#ifdef TOTO_GRUGE
			if ((first!=NULL)&&(group!=NULL))
			{
				if (TestSurface(group,first)==PLUS)
				{
					plus->Add(*surf);
				}
			}
			else
			{
				plus->Add(*surf);
			}
#else
			plus->Add(*surf);
#endif
		}
	}
	else
	{
		t1=t2=0.0f;
		res=0;
		if (cc==1) s=1; else s=-1;

		if (s*s1>=s*mul2*SMALLFBSPLIGHT)
		{
			res=1;
			A=surf->A;
			B=surf->B;
			C=surf->C;

			if (f_abs((float) (s2-s1))>0)	t1=-s1/(s2-s1);
			if (f_abs((float) (s3-s1))>0)	t2=-s1/(s3-s1);

		}
		ss=s1;

		if (((s*s2>=s*mul2*SMALLFBSPLIGHT)&&(res==0))||((s*s2>=s*mul2*SMALLFBSPLIGHT)&&(res==1)&&(s*s2>s*s1)))
		{
			A=surf->B;
			B=surf->C;
			C=surf->A;
			if (f_abs((float) (s3-s2))>0)	t1=-s2/(s3-s2);
			if (f_abs((float) (s1-s2))>0)	t2=-s2/(s1-s2);
			res=1;
			ss=s2;
		}

		if (((s*s3>=s*mul2*SMALLFBSPLIGHT)&&(res==0))||((s*s3>=s*mul2*SMALLFBSPLIGHT)&&(res==1)&&(s*s3>s*ss)))
		{
			res=1;
			A=surf->C;
			B=surf->A;
			C=surf->B;
			if (f_abs((float) (s1-s3))>0)	t1=-s3/(s1-s3);
			if (f_abs((float) (s2-s3))>0)	t2=-s3/(s2-s3);
		}

		if (t1>1.0f) t1=1.0f;
		if (t1<0.0f) t1=0.0f;
		if (t2>1.0f) t2=1.0f;
		if (t2<0.0f) t2=0.0f;

		if ((t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
		{
			if (CotePlusTriangle(surf,a,b,c,d)) plus->Add(*surf);
			else moins->Add(*surf);
		}
		else
		if ((1.0f-t1<SMALLFBSPLIGHT2)&&(1.0f-t2<SMALLFBSPLIGHT2))
		{
			if (CotePlusTriangle(surf,a,b,c,d)) plus->Add(*surf);
			else moins->Add(*surf);
		}
		else
		if ((1.0f-t1<SMALLFBSPLIGHT2)&&(t2<SMALLFBSPLIGHT2))
		{
			if (CotePlusTriangle(surf,a,b,c,d)) plus->Add(*surf);
			else moins->Add(*surf);
		}
		else
		if ((t1<SMALLFBSPLIGHT2)&&(1.0f-t2<SMALLFBSPLIGHT2))
		{
			if (CotePlusTriangle(surf,a,b,c,d)) plus->Add(*surf);
			else moins->Add(*surf);
		}
		else
		{
			VECTORINTERPOL(I,t1,B,A);
			VECTORINTERPOL(J,t2,C,A);
			//I=A+(float) t1*(B-A);
			//J=A+(float) t2*(C-A);

			tri.A=A;
			tri.B=I;
			tri.C=J;
			if (CotePlusTriangle(&tri,a,b,c,d))
			{
#ifdef TOTO_GRUGE
				if ((first!=NULL)&&(group!=NULL))
				{
					if (TestSurface(group,first)==PLUS)
					{
						plus->Add(tri);
					}
				}
				else
				{
					plus->Add(tri);
				}
#else
				plus->Add(tri);
#endif
			}

			else
			moins->Add(tri);

			tri.A=I;
			tri.B=B;
			tri.C=C;
			if (CotePlusTriangle(&tri,a,b,c,d))
			{
#ifdef TOTO_GRUGE
				if ((first!=NULL)&&(group!=NULL))
				{
					if (TestSurface(group,first)==PLUS)
					{
						plus->Add(tri);
					}
				}
				else
				{
					plus->Add(tri);
				}
#else
				plus->Add(tri);
#endif
			}
			else moins->Add(tri);

			tri.A=I;
			tri.B=C;
			tri.C=J;
			if (CotePlusTriangle(&tri,a,b,c,d))
			{
#ifdef TOTO_GRUGE
				if ((first!=NULL)&&(group!=NULL))
				{

					if (TestSurface(group,first)==PLUS)
					{
						plus->Add(tri);
					}
				}
				else
				{
					plus->Add(tri);
				}
#else
				plus->Add(tri);
#endif
			}
			else
			moins->Add(tri);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPLightNode * CBSPLight::BuildNode(CList <CTriangles> * faces,CVector &L)
{
	CBSPLightNode * node;
	CList <CTriangles> plus,moins;
	CTriangle * tri;
	int len;
	CTriangles *surf,*first,*t;
	CList <CTriangle> tp,tm;
	CVector N;
	int tag;
	int p,m;
	CVector pp;

	node=NULL;

	len=faces->Length();
	if (len>0)
	{
		first=faces->GetFirst();

		node=new CBSPLightNode;

		N=first->Norm;
		VECTORNORMALISE(N);
		node->type=NODE;
		node->a=N.x;
		node->b=N.y;
		node->c=N.z;
		pp=first->Base;
		node->d=-( N.x*pp.x +  N.y*pp.y +  N.z*pp.z);

		tag=first->tag;
		node->tag=tag;
		p=1;
		m=0;
		if (len>1)
		{
			surf=faces->GetNext();

			while (surf)
			{
				tri=surf->List.GetFirst();
				while (tri)
				{
					DecoupeTRIANGLE3(-1,NULL,surf,tri,node->a,node->b,node->c,node->d,&tp,&tm);
					tri=surf->List.GetNext();
				}

				if (tp.Length()>0)
				{
					t=plus.InsertLast();

					t->Base=surf->Base;
					t->Norm=surf->Norm;
					t->tag=surf->tag;
					tri=tp.GetFirst();
					while (tri)
					{
						t->List.Add(*tri);
						tri=tp.GetNext();
					}
				}

				if (tm.Length()>0)
				{
					t=moins.InsertLast();

					t->Base=surf->Base;
					t->Norm=surf->Norm;
					t->tag=surf->tag;
					tri=tm.GetFirst();
					while (tri)
					{
						t->List.Add(*tri);
						tri=tm.GetNext();
					}
				}

				tp.Free();
				tm.Free();

				surf=faces->GetNext();
			}

			if ((plus.Length()>0))
			{
				node->plus=BuildNode(&plus,L);
			}
			else
			{
				node->plus=new CBSPLightNode;
				node->plus->type=(p==0);
			}

			if (moins.Length()>0)
			{
				node->moins=BuildNode(&moins,L);
			}
			else
			{
				node->moins=new CBSPLightNode;
				node->moins->type=(m==0);
			}

			surf=plus.GetFirst();
			while (surf)
			{
				surf->List.Free();
				surf=plus.GetNext();
			}

			surf=moins.GetFirst();
			while (surf)
			{
				surf->List.Free();
				surf=moins.GetNext();
			}

			plus.Free();
			moins.Free();
		}
		else
		{
			node->plus=new CBSPLightNode;
			node->plus->type=(p==0);

			node->moins=new CBSPLightNode;
			node->moins->type=(m==0);

		}
	}
	return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
