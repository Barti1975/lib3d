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
//	@file bsp_ortho.cpp 
//	@date 2004
////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

#include "params.h"

#include <math.h>
#include "stdio.h"
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "bsp_volumic.h"
#include "edges.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool	BUILD_ORHOBSP_2D		=	false;
float	DIM_ORTHOBSP			=	0.5f;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFaceGROUP::CFaceGROUP()
{
	figures=NULL;
	faces=NULL;
	bsps=NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFaceGROUP::~CFaceGROUP() { 
#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_
	CBSPVolumicOld *bsp;
#endif
	if (faces)
	{
		faces->Free();
		delete faces;
		faces=NULL;
	}

	if (figures)
	{
		figures->Free();
		delete figures;
		figures=NULL;
	}

#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_
	if (bsps)
	{
		bsp=bsps->GetFirst();
		while (bsp)
		{
			bsp->Free();
			bsp=bsps->GetNext();
		}
		bsps->Free();
		delete bsps;
		bsps=NULL;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFaceGROUP::clean()
{
#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_
	CBSPVolumicOld *bsp;

	if (bsps)
	{
		bsp=bsps->GetFirst();
		while (bsp)
		{
			bsp->Free();
			bsp=bsps->GetNext();
		}
		bsps->Free();
		delete bsps;
		bsps=NULL;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPOrthoNode::CBSPOrthoNode()
{
	groups=NULL;faces=NULL;plus=moins=NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPOrthoNode::clean()
{
	if (groups)
	{
		CFaceGROUP * g=groups->GetFirst();
		while (g)
		{
			g->clean();
			g=groups->GetNext();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPOrtho::CBSPOrtho()
{ 
	root=NULL;ListeFaces=NULL;tmpGroups=NULL;Groups=NULL;
	for (int n=0;n<64;n++) ListeFacesTH[n]=NULL;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:


		CBSPOrthoNode * CBSPOrtho::BuildNode(int Niter,CList <int> *faces,CObject3D * obj,int axe)
		void CBSPOrtho::RecursFrustumGetFaceList(CBSPOrthoNode *node,CVector * frustum,CMatrix M)
		void CBSPOrtho::RecursFrustumGetGroupList(CBSPOrthoNode *node,CVector * frustum,CMatrix M)
		void CBSPOrtho::RecursGetFaceList(CBSPOrthoNode *node,CVector P,float R,CMatrix M)
		void CBSPOrtho::FreeNode(CBSPOrthoNode * node)
		CBSPOrthoNode * CBSPOrtho::BuildNodeWithFigures(int Niter,CList <CFaceGROUP> *groups,CObject3D * obj,int axe)

		

		interface :

		void CBSPOrtho::Build(int Niter,CObject3D * obj)
		void CBSPOrtho::BuildWithFigures(int Niter,CObject3D * obj)
		void CBSPOrtho::Free()
		CList <int> * CBSPOrtho::FrustumGetFaceList(CVector * frustum,CMatrix M)
		CList <CFaceGROUP*> * CBSPOrtho::FrustumGetGroupList(CVector * frustum,CMatrix M)
		CList <int> * CBSPOrtho::GetFaceList(CVector P,float R,CMatrix M)

		
	Usage:	

		frustum : array of 5 points
		P,R : sphere def

  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CBSPOrtho::Build(int Niter,CObject3D * obj)
{
	CList <int> faces;
	int n;
	
	faces.Free();
	for (n=0;n<obj->nFaces;n++) faces.Add(n);
	if (BUILD_ORHOBSP_2D) root=BuildNode2D(Niter,&faces,obj,AXE_x);
	else root=BuildNode(Niter,&faces,obj,AXE_x);
	faces.Free();
	//obj->SetTag(SET_FACES_COPLANAR_RECURS_TAG);
}


CBSPOrthoNode * CBSPOrtho::BuildNode(int Niter,CList <int> *faces,CObject3D * obj,int axe)
{
	CBSPOrthoNode *node=NULL;
	CList <int> plus;
	CList <int> moins;
	CList <int> *stok=NULL;
	float d;
	float s0,s1,s2;
	float dx,dy,dz;
	int cc;
	int * i;
	CVector N;
	CVector M;
	CVector *v,u;

	if (faces->Length()>0)
	{
		if ((faces->Length()<2)||(Niter==0))
		{
			stok=new CList <int>;
			i=faces->GetFirst();
			while (i)
			{
				stok->Add(*i);
				i=faces->GetNext();
			}
			node=new CBSPOrthoNode;
			node->type=ORTHOBSP_TERM;
			node->faces=stok;
		}
		else
		{
			switch (axe)
			{
			case 0:
				N.Init(1,0,0);
				break;
			case 1:
				N.Init(0,1,0);
				break;
			case 2:
				N.Init(0,0,1);
				break;
			};

			M.Init(0,0,0);
			i=faces->GetFirst();
			while (i)
			{
				M+=obj->Faces[*i].v[0]->Stok;
				M+=obj->Faces[*i].v[1]->Stok;
				M+=obj->Faces[*i].v[2]->Stok;
				i=faces->GetNext();
			}

			M=M/(float) (faces->Length()*3);
			dx=dy=dz=0;

			i=faces->GetFirst();
			while (i)
			{
				u=M-obj->Faces[*i].v[0]->Stok;
				if (f_abs(u.x)>dx) dx=f_abs(u.x);
				if (f_abs(u.y)>dy) dy=f_abs(u.y);
				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				u=M-obj->Faces[*i].v[1]->Stok;
				if (f_abs(u.x)>dx) dx=f_abs(u.x);
				if (f_abs(u.y)>dy) dy=f_abs(u.y);
				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				u=M-obj->Faces[*i].v[2]->Stok;
				if (f_abs(u.x)>dx) dx=f_abs(u.x);
				if (f_abs(u.y)>dy) dy=f_abs(u.y);
				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				i=faces->GetNext();
			}

			if (((axe==0)&&(dx<DIM_ORTHOBSP))||((axe==1)&&(dy<DIM_ORTHOBSP))||((axe==2)&&(dz<DIM_ORTHOBSP)))
			{
				if ((dx<DIM_ORTHOBSP)&&(dy<DIM_ORTHOBSP)&&(dz<DIM_ORTHOBSP))
				{
					stok=new CList <int>;
					i=faces->GetFirst();
					while (i)
					{
						stok->Add(*i);
						i=faces->GetNext();
					}
					node=new CBSPOrthoNode;
					node->type=ORTHOBSP_TERM;
					node->faces=stok;
				}
				else node=BuildNode(Niter-1,faces,obj,(axe+1)%3);
			}
			else
			{
				node=new CBSPOrthoNode;
				node->type=ORTHOBSP_NODE;

				node->a=N.x;
				node->b=N.y;
				node->c=N.z;
				node->d=d=-DOT(N,M);

				i=faces->GetFirst();
				while (i)
				{
					v=&(obj->Faces[*i].v[0]->Stok);
					s0=N.x*v->x+N.y*v->y+N.z*v->z+d;
					v=&(obj->Faces[*i].v[1]->Stok);
					s1=N.x*v->x+N.y*v->y+N.z*v->z+d;
					v=&(obj->Faces[*i].v[2]->Stok);
					s2=N.x*v->x+N.y*v->y+N.z*v->z+d;

					cc=(s0>0)+(s1>0)+(s2>0);

					if (cc==0) moins.Add(*i);
					else
					{
						if (cc==3) plus.Add(*i);
						else
						{
							moins.Add(*i);
							plus.Add(*i);
						}
					}
					i=faces->GetNext();
				}

				if (((plus.Length()==moins.Length()))&&(plus.Length()==faces->Length()))
				{
					delete node;

					plus.Free();
					moins.Free();
					node=BuildNode(Niter-1,faces,obj,(axe+1)%3);
				}
				else
				{
					if (plus.Length()>0)
					{
						node->plus=BuildNode(Niter-1,&plus,obj,(axe+1)%3);
					}
					else
					{
						node->plus=new CBSPOrthoNode;
						node->plus->type=ORTHOBSP_TERM;
						node->plus->faces=new CList <int>;
					}

					if (moins.Length()>0)
					{
						node->moins=BuildNode(Niter-1,&moins,obj,(axe+1)%3);
					}
					else
					{
						node->moins=new CBSPOrthoNode;
						node->moins->type=ORTHOBSP_TERM;
						node->moins->faces=new CList <int>;
					}

					plus.Free();
					moins.Free();
				}
			}
		}
	}

	return node;
}


CBSPOrthoNode * CBSPOrtho::BuildNode2D(int Niter,CList <int> *faces,CObject3D * obj,int axe)
{
	CBSPOrthoNode *node=NULL;
	CList <int> plus;
	CList <int> moins;
	CList <int> *stok=NULL;
	float d;
	float s0,s1,s2;
	float dx,dy,dz;
	int cc;
	int * i;
	CVector N;
	CVector M;
	CVector *v,u;

	if (faces->Length()>0)
	{
		if ((faces->Length()<2)||(Niter==0))
		{
			stok=new CList <int>;
			i=faces->GetFirst();
			while (i)
			{
				stok->Add(*i);
				i=faces->GetNext();
			}
			node=new CBSPOrthoNode;
			node->type=ORTHOBSP_TERM;
			node->faces=stok;
		}
		else
		{
			switch (axe)
			{
			case 0:
				N.Init(1,0,0);
				break;
			case 1:
				N.Init(0,0,1);
				break;
			};

			M.Init(0,0,0);
			i=faces->GetFirst();
			while (i)
			{
				VECTORADD(M,M,obj->Faces[*i].v[0]->Stok);
				VECTORADD(M,M,obj->Faces[*i].v[1]->Stok);
				VECTORADD(M,M,obj->Faces[*i].v[2]->Stok);
				i=faces->GetNext();
			}

			M=M/(float) (faces->Length()*3);
			dx=dy=dz=0;

			i=faces->GetFirst();
			while (i)
			{
				VECTORSUB(u,M,obj->Faces[*i].v[0]->Stok);
				if (f_abs(u.x)>dx) dx=f_abs(u.x);

				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				VECTORSUB(u,M,obj->Faces[*i].v[1]->Stok);
				if (f_abs(u.x)>dx) dx=f_abs(u.x);

				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				VECTORSUB(u,M,obj->Faces[*i].v[2]->Stok);
				if (f_abs(u.x)>dx) dx=f_abs(u.x);

				if (f_abs(u.z)>dz) dz=f_abs(u.z);
				i=faces->GetNext();
			}

			if (((axe==0)&&(dx<DIM_ORTHOBSP))||((axe==1)&&(dz<DIM_ORTHOBSP)))
			{
				if ((dx<DIM_ORTHOBSP)&&(dz<DIM_ORTHOBSP))
				{
					stok=new CList <int>;
					i=faces->GetFirst();
					while (i)
					{
						stok->Add(*i);
						i=faces->GetNext();
					}
					node=new CBSPOrthoNode;
					node->type=ORTHOBSP_TERM;
					node->faces=stok;
				}
				else node=BuildNode2D(Niter-1,faces,obj,(axe+1)%2);
			}
			else
			{
				node=new CBSPOrthoNode;
				node->type=ORTHOBSP_NODE;

				node->a=N.x;
				node->b=N.y;
				node->c=N.z;
				node->d=d=-DOT(N,M);
			
				i=faces->GetFirst();
				while (i)
				{
					v=&(obj->Faces[*i].v[0]->Stok);
					s0=N.x*v->x+N.y*v->y+N.z*v->z+d;
					v=&(obj->Faces[*i].v[1]->Stok);
					s1=N.x*v->x+N.y*v->y+N.z*v->z+d;
					v=&(obj->Faces[*i].v[2]->Stok);
					s2=N.x*v->x+N.y*v->y+N.z*v->z+d;

					cc=(s0>0)+(s1>0)+(s2>0);

					if (cc==0) moins.Add(*i);
					else
					{
						if (cc==3) plus.Add(*i);
						else
						{
							moins.Add(*i);
							plus.Add(*i);
						}
					}
					i=faces->GetNext();
				}

				if (((plus.Length()==moins.Length()))&&(plus.Length()==faces->Length()))
				{
					delete node;

					plus.Free();
					moins.Free();
					node=BuildNode2D(Niter-1,faces,obj,(axe+1)%2);
				}
				else
				{
					if (plus.Length()>0)
					{
						node->plus=BuildNode2D(Niter-1,&plus,obj,(axe+1)%2);
					}
					else
					{
						node->plus=new CBSPOrthoNode;
						node->plus->type=ORTHOBSP_TERM;
						node->plus->faces=new CList <int>;
					}

					if (moins.Length()>0)
					{
						node->moins=BuildNode2D(Niter-1,&moins,obj,(axe+1)%2);
					}
					else
					{
						node->moins=new CBSPOrthoNode;
						node->moins->type=ORTHOBSP_TERM;
						node->moins->faces=new CList <int>;
					}

					plus.Free();
					moins.Free();
				}
			}
		}
	}

	return node;
}




CList <int> * CBSPOrtho::GetFaceList(CVector &P,float R,CMatrix &M)
{
	CVector iP;
	CMatrix iM;

	if (ListeFaces)
	{
		ListeFaces->Free();
		delete ListeFaces;
	}

	iM.Inverse4x4(M);
	iP=P*iM;
	ListeFaces=new CList <int>;
	RecursGetFaceList(root,iP,R);

	return ListeFaces;
}

CList <int> * CBSPOrtho::GetFaceList(CVector &P,float R,CMatrix &M,int buf)
{
	CVector iP;
	CMatrix iM;

	if (ListeFacesTH[buf])
	{
		ListeFacesTH[buf]->Free();
		delete ListeFacesTH[buf];
	}

	ListeFacesTH[buf]=new CList <int>;

	iM.Inverse4x4(M);
	iP=P*iM;

	RecursGetFaceList(root,iP,R,buf);

	return ListeFacesTH[buf];
}

void CBSPOrtho::RecursFrustumGetFaceList(CBSPOrthoNode *node,CVector * frustum)
{
	float x1,x2;
	int n,*i,res;
	float a,b,c,d;

	if (node)
	{
		if (node->type==ORTHOBSP_NODE)
		{
			a=node->a;
			b=node->b;
			c=node->c;
			d=node->d;

			x1=a*frustum[0].x+b*frustum[0].y+c*frustum[0].z+d;
			res=1;
			for (n=1;n<5;n++)
			{
				x2=a*frustum[n].x+b*frustum[n].y+c*frustum[n].z+d;
				if (x1*x2<0) {res=0;break;}
			}

			if (res==1)
			{
				if (x1>0) RecursFrustumGetFaceList(node->plus,frustum);
				else RecursFrustumGetFaceList(node->moins,frustum);
			}
			else
			{
				if (x1<0)
				{
					RecursFrustumGetFaceList(node->moins,frustum);
					RecursFrustumGetFaceList(node->plus,frustum);
				}
				else
				{
					RecursFrustumGetFaceList(node->plus,frustum);
					RecursFrustumGetFaceList(node->moins,frustum);
				}
			}
		}
		else
		{
			bool tmp;
			int *ii;

			if (node->faces)
			{
				i=node->faces->GetFirst();
				while (i)
				{
					tmp=false;

					ii=ListeFaces->GetFirst();
					while ((ii)&&(!tmp))
					{
						if ((*ii)==(*i)) tmp=true;
						ii=ListeFaces->GetNext();
					}

					if (!tmp) ListeFaces->Add(*i);	
					i=node->faces->GetNext();
				}
			}

		}
	}
}


void CBSPOrtho::RecursFrustumGetGroupList(CBSPOrthoNode *node,CVector * frustum)
{
	float x1,x2;
	int n,res;
	float a,b,c,d;

	if (node)
	{
		if (node->type==ORTHOBSP_NODE)
		{

			a=node->a;
			b=node->b;
			c=node->c;
			d=node->d;

			x1=a*frustum[0].x+b*frustum[0].y+c*frustum[0].z+d;

			res=1;
			for (n=1;n<5;n++)
			{
				x2=a*frustum[n].x+b*frustum[n].y+c*frustum[n].z+d;
				if (x1*x2<0) {res=0;break;}
			}

			if (res==1)
			{
				if (x1>0)
				{
					RecursFrustumGetGroupList(node->plus,frustum);
				}
				else
				{
					RecursFrustumGetGroupList(node->moins,frustum);
				}
			}
			else
			{
				if (x1<0)
				{
					RecursFrustumGetGroupList(node->moins,frustum);
					RecursFrustumGetGroupList(node->plus,frustum);
				}
				else
				{
					RecursFrustumGetGroupList(node->plus,frustum);
					RecursFrustumGetGroupList(node->moins,frustum);
				}
			}
		}
		else
		{
			bool tmp;
			CFaceGROUP * g;
			CFaceGROUP ** gt;
			g=node->groups->GetFirst();
			while (g)
			{
				tmp=false;
				gt=tmpGroups->GetFirst();
				while ((gt)&&(!tmp))
				{
					if ((*gt)->id==g->id) tmp=true;
					gt=tmpGroups->GetNext();
				}

				if (!tmp) tmpGroups->Add(g);	
				g=node->groups->GetNext();
			}

		}
	}
}



CList <int> * CBSPOrtho::FrustumGetFaceList(CVector * frustum,CMatrix &M)
{
	int n;
	CMatrix iM;
	CVector fs[5];

	if (ListeFaces!=NULL)
	{
		ListeFaces->Free();
		delete ListeFaces;
	}

	ListeFaces=new CList <int>;

	iM.Inverse4x4(M);
	for (n=0;n<5;n++) fs[n]=frustum[n]*iM;

	RecursFrustumGetFaceList(root,fs);

	return ListeFaces;

}


CList <CFaceGROUP*> * CBSPOrtho::FrustumGetGroupList(CVector * frustum,CMatrix &M)
{
	int n;
	CMatrix iM;
	CVector fs[5];

	if (tmpGroups)
	{
		tmpGroups->Free();
		delete tmpGroups;
	}

	tmpGroups=new CList <CFaceGROUP*>;

	
	iM.Inverse4x4(M);
	for (n=0;n<5;n++) fs[n]=frustum[n]*iM;
	RecursFrustumGetGroupList(root,fs);

	return tmpGroups;

}


CList <CFaceGROUP*> * CBSPOrtho::GetGroupList()
{
	CFaceGROUP* pg;

	if (tmpGroups)
	{
		tmpGroups->Free();
		delete tmpGroups;
	}

	tmpGroups=new CList <CFaceGROUP*>;

	pg=Groups->GetFirst();
	while (pg)
	{
		tmpGroups->Add(pg);
		pg=Groups->GetNext();
	}
	
	return tmpGroups;
}


void CBSPOrtho::RecursGetFaceList(CBSPOrthoNode *node,CVector &P,float R)
{
	float a,b,c,d;
	float s;
	int *i;

	if (node)
	{
		if (node->type==ORTHOBSP_NODE)
		{
			a=node->a;
			b=node->b;
			c=node->c;
			d=node->d;

			s=a*P.x+b*P.y+c*P.z+d;

			if (f_abs(s)<=R)
			{
				// intersection
				RecursGetFaceList(node->plus,P,R);
				RecursGetFaceList(node->moins,P,R);
			}
			else
			{
				if (s<-R)
				{
					RecursGetFaceList(node->moins,P,R);

				}
				else
				{
					RecursGetFaceList(node->plus,P,R);
				}
			}
		}
		else
		{
			bool tmp;
			int *ii;
			i=node->faces->GetFirst();
			while (i)
			{
				tmp=false;

				ii=ListeFaces->GetFirst();
				while ((ii)&&(!tmp))
				{
					if ((*ii)==(*i)) tmp=true;
					ii=ListeFaces->GetNext();
				}

				if (!tmp) ListeFaces->Add(*i);	
				i=node->faces->GetNext();
			}
		}
	}
}

void CBSPOrtho::RecursGetFaceList(CBSPOrthoNode *node,CVector &P,float R,int buf)
{
	float a,b,c,d;
	float s;
	int i;

	if (node)
	{
		if (node->type==ORTHOBSP_NODE)
		{

			a=node->a;
			b=node->b;
			c=node->c;
			d=node->d;

			s=a*P.x+b*P.y+c*P.z+d;

			if (f_abs(s)<=R)
			{
				// intersection
				RecursGetFaceList(node->plus,P,R,buf);
				RecursGetFaceList(node->moins,P,R,buf);
			}
			else
			{
				if (s<-R)
				{
					RecursGetFaceList(node->moins,P,R,buf);

				}
				else
				{
					RecursGetFaceList(node->plus,P,R,buf);
				}
			}
		}
		else
		{
			bool tmp;
			int *ii;
			CElement <int> * pi=node->faces->First;			
			while (pi)
			{
				i=pi->data;
				tmp=false;

				ii=ListeFacesTH[buf]->GetFirst();
				while ((ii)&&(!tmp))
				{
					if ((*ii)==i) tmp=true;
					ii=ListeFacesTH[buf]->GetNext();
				}

				if (!tmp) ListeFacesTH[buf]->Add(i);	
				
				pi=pi->next;
			}
		}
	}
}

void CBSPOrtho::BuildWithFigures(int Niter,CObject3D * obj)
{
	int n,nn,N,n0,tag,nf;
	bool tmp,test;
	EdgeListD Edges;
	CFigure *f;
	CFaceGROUP * g;
	tag=1;

	tmp=true;

	Groups=new CList <CFaceGROUP>;

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag3=obj->Faces[n].tag;
	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;

	while (tmp)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag==0) nn=n;
			n++;
		}

		if (nn==-1) tmp=false;
		else
		{
			g=Groups->InsertLast();
			g->id=tag;
			g->nT=(unsigned short int)obj->Faces[nn].nT;
			g->tag=(unsigned short int)obj->Faces[nn].tag3;
			g->nL=(unsigned short int)(obj->Faces[nn].nLVectors-1);

			obj->TagCoplanarFaces_nTnLTag3(nn,nn,tag);
			obj->Faces[nn].tag=tag;

			nf=0;
			for (n=0;n<obj->nFaces;n++) if (obj->Faces[n].tag==tag) nf++;

			Edges.Init(nf*3);

			//NORM
			//g->Norm=obj->Faces[nn].Norm;

			g->faces=new CList <int>;
			for (n=0;n<obj->nFaces;n++) 
				if (obj->Faces[n].tag==tag) 
				{
					g->faces->Add(n);
					Edges.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
					Edges.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
					Edges.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
				}

			g->figures=new CList <CFigure>;
			test=true;
			while (test)
			{
				n=0;
				nn=-1;
				while ((n<Edges.nList)&&(nn==-1))
				{
					if (Edges.List[n].tag==0) nn=n;
					n++;
				}

				if (nn==-1) test=false;
				else
				{
					n0=nn;
					f=g->figures->InsertLast();
					f->List.Add(Edges.List[n0].a);
					Edges.List[n0].tag=1;
					n=Edges.Next(Edges.List[n0].b);
					Edges.List[n0].tag=0;
					N=0;
					while ((n!=n0)&&(N<500))
					{
						if (n!=-1)
						{
							Edges.List[n].tag=1;
							f->List.Add(Edges.List[n].a);
							n=Edges.Next(Edges.List[n].b);
						}
						N++;
					}
					Edges.List[n0].tag=1;
					f->OptimizeFrom(obj->Vertices);
				}
			}
			Edges.Free();
			tag++;
		}
	}

	if (BUILD_ORHOBSP_2D) root=BuildNodeWithFigures2D(Niter,Groups,obj,2);
	else root=BuildNodeWithFigures(Niter,Groups,obj,AXE_x);

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=obj->Faces[n].tag3;
}


void CBSPOrtho::BuildWithFigures2(int Niter,CObject3D * obj)
{
	int n,nn,N,n0,tag,nf;
	bool tmp,test;
	EdgeListD Edges;
	CFigure *f;

	CFaceGROUP * g;
	tag=1;

	tmp=true;

	Groups=new CList <CFaceGROUP>;

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag3=obj->Faces[n].tag;
	for (n=0;n<obj->nFaces;n++) 
	{
		if (obj->Faces[n].tag!=0) obj->Faces[n].tag=0;
		else obj->Faces[n].tag=-1;
	}

	while (tmp)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag==0) nn=n;
			n++;
		}

		if (nn==-1) tmp=false;
		else
		{
			g=Groups->InsertLast();
			g->id=tag;
			g->nT=obj->Faces[nn].nT;
			g->tag=obj->Faces[nn].tag3;
			g->nL=obj->Faces[nn].nLVectors-1;

			obj->TagCoplanarFaces_nTnLTag3(nn,nn,tag);
			obj->Faces[nn].tag=tag;

			nf=0;
			for (n=0;n<obj->nFaces;n++) if (obj->Faces[n].tag==tag) nf++;

			Edges.Init(nf*3);

			//NORM
			//g->Norm=obj->Faces[nn].Norm;

			g->faces=new CList <int>;
			for (n=0;n<obj->nFaces;n++) 
				if (obj->Faces[n].tag==tag) 
				{					
					g->faces->Add(n);
					Edges.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
					Edges.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
					Edges.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
				}

			g->figures=new CList <CFigure>;
			test=true;
			while (test)
			{
				n=0;
				nn=-1;
				while ((n<Edges.nList)&&(nn==-1))
				{
					if (Edges.List[n].tag==0) nn=n;
					n++;
				}

				if (nn==-1) test=false;
				else
				{
					n0=nn;
					f=g->figures->InsertLast();
					f->List.Add(Edges.List[n0].a);
					Edges.List[n0].tag=1;
					n=Edges.Next(Edges.List[n0].b);
					Edges.List[n0].tag=0;
					N=0;
					while ((n!=n0)&&(N<100))
					{
						if (n!=-1)
						{
							Edges.List[n].tag=1;
							f->List.Add(Edges.List[n].a);
							n=Edges.Next(Edges.List[n].b);
						}
						N++;
					}
					Edges.List[n0].tag=1;
					f->OptimizeFrom(obj->Vertices);
				}
			}
			Edges.Free();
			tag++;
		}
	}

	int NN=15;
	if (Groups->Length()<600) NN=12;
	if (Groups->Length()<400) NN=9;
	if (Groups->Length()<200) NN=6;
	if (Groups->Length()<100) NN=3;
	
	root=BuildNodeWithFigures(NN,Groups,obj,AXE_x);

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=obj->Faces[n].tag3;
}



void CopyGroup(CFaceGROUP *gdest,CFaceGROUP *gsrc)
{
	int * f;
	CFigure *fig;

	gdest->faces=new CList <int>;
	f=gsrc->faces->GetFirst();
	while (f)
	{
		gdest->faces->Add(*f);
		f=gsrc->faces->GetNext();
	}

	gdest->figures=new CList<CFigure>;
	fig=gsrc->figures->GetFirst();
	while (fig)
	{
		gdest->figures->InsertLast()->Copy(fig);
		fig=gsrc->figures->GetNext();
	}

	//NORM
	//gdest->Norm=gsrc->Norm;
	gdest->id=gsrc->id;
	gdest->nT=gsrc->nT;
	gdest->nL=gsrc->nL;
	gdest->tag=gsrc->tag;
}

void EdgesGroup(CFaceGROUP *g,CList <EdgeG> *edges,CVertex *vertices)
{
	CFigure *f;
	int a,b,i0,i1;
	int n;
	CVector A,B,C,D;
	CVector u1,u2;
	EdgeG *ed,newed,*ed_exist;
	
	f=g->figures->GetFirst();
	while (f)
	{
		for (n=0;n<f->List.Length();n++)
		{
			i0=n;
			i1=(n+1)%f->List.Length();

			a=*(f->List[i0]);
			b=*(f->List[i1]);

			A=vertices[a].Stok;
			B=vertices[b].Stok;

			ed_exist=NULL;

			ed=edges->GetFirst();
			while ((ed)&&(!ed_exist))
			{
				C=vertices[ed->a].Stok;
				D=vertices[ed->b].Stok;

				u1=C-A;
				u2=D-B;
				if ((u1.Norme()<SMALLF2)&&(u2.Norme()<SMALLF2)) ed_exist=ed;
				u1=C-B;
				u2=D-A;
				if ((u1.Norme()<SMALLF2)&&(u2.Norme()<SMALLF2)) ed_exist=ed;

				ed=edges->GetNext();
			}

			if (ed_exist)
			{
				ed_exist->cp=2;
				ed_exist->p2=(void*) f;
				ed_exist->num2=n;
			}
			else
			{
				newed.a=a;
				newed.b=b;
				newed.num1=n;
				newed.cp=1;
				newed.p1=(void*) f;
				edges->Add(newed);
			}
		}

		f=g->figures->GetNext();
	}
}


CBSPOrthoNode * CBSPOrtho::BuildNodeWithFigures(int Niter,CList <CFaceGROUP> *groups,CObject3D * obj,int axe)
{
	float dx,dy,dz;
	CBSPOrthoNode *node;
	CList <CFaceGROUP> plus;
	CList <CFaceGROUP> moins;
	float d;
	float s0,s1,s2;
	int cc;
	int nn,inf,sup;
	int * i;
	CVector N;
	CVector M,u;
	CVector *v;
	CFaceGROUP * g;

	node=NULL;

	if (groups->Length()>0)
	{

		if ((groups->Length()<3)||(Niter==0))
		{
			node=new CBSPOrthoNode;
			node->type=ORTHOBSP_TERM;
			node->faces=NULL;
			node->groups=new CList <CFaceGROUP>;

			g=groups->GetFirst();
			while (g)
			{
				CopyGroup(node->groups->InsertLast(),g);
				g=groups->GetNext();
			}

			node->faces=new CList <int>;

			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					node->faces->Add(*i);
					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

		}
		else
		{

			switch (axe)
			{
			case 0:
				N.Init(1,0,0);
				break;
			case 1:
				N.Init(0,1,0);
				break;
			case 2:
				N.Init(0,0,1);
				break;
			};

			M.Init(0,0,0);

			nn=0;
			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					M+=obj->Faces[*i].v[0]->Stok;
					M+=obj->Faces[*i].v[1]->Stok;
					M+=obj->Faces[*i].v[2]->Stok;
					nn++;
					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

			M=M/(float) (nn*3);

			dx=dy=dz=0;

			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					u=M-obj->Faces[*i].v[0]->Stok;
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.y)>dy) dy=f_abs(u.y);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);
					u=M-obj->Faces[*i].v[1]->Stok;
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.y)>dy) dy=f_abs(u.y);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);
					u=M-obj->Faces[*i].v[2]->Stok;
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.y)>dy) dy=f_abs(u.y);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);

					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

			if (((axe==0)&&(dx<DIM_ORTHOBSP))||((axe==1)&&(dy<DIM_ORTHOBSP))||((axe==2)&&(dz<DIM_ORTHOBSP)))
			{
				if ((dx<DIM_ORTHOBSP)&&(dy<DIM_ORTHOBSP)&&(dz<DIM_ORTHOBSP))
				{
					node=new CBSPOrthoNode;
					node->type=ORTHOBSP_TERM;
					node->faces=NULL;
					node->groups=new CList <CFaceGROUP>;

					g=groups->GetFirst();
					while (g)
					{
						CopyGroup(node->groups->InsertLast(),g);
						g=groups->GetNext();
					}

					node->faces=new CList <int>;

					g=groups->GetFirst();
					while (g)
					{
						i=g->faces->GetFirst();
						while (i)
						{
							node->faces->Add(*i);
							i=g->faces->GetNext();
						}
						g=groups->GetNext();
					}		
				}
				else node=BuildNodeWithFigures(Niter-1,groups,obj,(axe+1)%3);
			}
			else
			{
				node=new CBSPOrthoNode;
				node->type=ORTHOBSP_NODE;

				node->a=N.x;
				node->b=N.y;
				node->c=N.z;

				node->d=d=-DOT(N,M);

				nn=0;
				g=groups->GetFirst();				
				while (g)
				{
					inf=sup=0;
					i=g->faces->GetFirst();
					while (i)
					{
						v=&(obj->Faces[*i].v[0]->Stok);
						s0=N.x*v->x+N.y*v->y+N.z*v->z+d;
						v=&(obj->Faces[*i].v[1]->Stok);
						s1=N.x*v->x+N.y*v->y+N.z*v->z+d;
						v=&(obj->Faces[*i].v[2]->Stok);
						s2=N.x*v->x+N.y*v->y+N.z*v->z+d;

						cc=(s0>0)+(s1>0)+(s2>0);

						if (cc==0)
						{
							inf++;
						}
						
						if (cc==3)
						{
							sup++;
						}
						
						if ((cc==1)||(cc==2))
						{

							inf++;
							sup++;
						}
						i=g->faces->GetNext();
					}


					if ((sup>0)&&(inf==0))
					{
						CopyGroup(plus.InsertLast(),g);
					}

					if ((sup>0)&&(inf>0))
					{
						CopyGroup(plus.InsertLast(),g);
						CopyGroup(moins.InsertLast(),g);
					}

					if ((inf>0)&&(sup==0))
					{
						CopyGroup(moins.InsertLast(),g);
					}
					g=groups->GetNext();
				}
				
				if (plus.Length()>0)
				{
					node->plus=BuildNodeWithFigures(Niter-1,&plus,obj,(axe+1)%3);
				}
				else
				{
					node->plus=new CBSPOrthoNode;
					node->plus->type=ORTHOBSP_TERM;
				}

				if (moins.Length()>0)
				{
					node->moins=BuildNodeWithFigures(Niter-1,&moins,obj,(axe+1)%3);
				}
				else
				{
					node->moins=new CBSPOrthoNode;
					node->moins->type=ORTHOBSP_TERM;
				}

				plus.Free();
				moins.Free();
			}
		}			
	}

	return node;
}


CBSPOrthoNode * CBSPOrtho::BuildNodeWithFigures2D(int Niter,CList <CFaceGROUP> *groups,CObject3D * obj,int axe)
{
	float dx,dy,dz;
	CBSPOrthoNode *node;
	CList <CFaceGROUP> plus;
	CList <CFaceGROUP> moins;
	float d;
	float s0,s1,s2;
	int cc;
	int nn,inf,sup;
	int * i;
	CVector N;
	CVector M,u;
	CVector *v;
	CFaceGROUP * g;

	node=NULL;

	if (groups->Length()>0)
	{

		if ((groups->Length()<3)||(Niter==0))
		{
			node=new CBSPOrthoNode;
			node->type=ORTHOBSP_TERM;
			node->faces=NULL;
			node->groups=new CList <CFaceGROUP>;

			g=groups->GetFirst();
			while (g)
			{
				CopyGroup(node->groups->InsertLast(),g);
				g=groups->GetNext();
			}

			node->faces=new CList <int>;

			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					node->faces->Add(*i);
					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

		}
		else
		{

			switch (axe)
			{
			case 0:
				N.Init(1,0,0);
				break;
			case 1:
				N.Init(0,0,1);
				break;
			case 2:
				N.Init(0,1,0);  //Y
				break;
			};
			
			M.Init(0,0,0);

			nn=0;
			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					VECTORADD(M,M,obj->Faces[*i].v[0]->Stok);
					VECTORADD(M,M,obj->Faces[*i].v[1]->Stok);
					VECTORADD(M,M,obj->Faces[*i].v[2]->Stok);
					nn++;
					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

			M=M/(float) (nn*3);

			dx=dy=dz=0;

			g=groups->GetFirst();
			while (g)
			{
				i=g->faces->GetFirst();
				while (i)
				{
					VECTORSUB(u,M,obj->Faces[*i].v[0]->Stok);
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);

					VECTORSUB(u,M,obj->Faces[*i].v[1]->Stok);
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);

					VECTORSUB(u,M,obj->Faces[*i].v[2]->Stok);
					if (f_abs(u.x)>dx) dx=f_abs(u.x);
					if (f_abs(u.z)>dz) dz=f_abs(u.z);

					i=g->faces->GetNext();
				}
				g=groups->GetNext();
			}

			if (((axe==0)&&(dx<DIM_ORTHOBSP))||((axe==1)&&(dz<DIM_ORTHOBSP)))
			{
				if ((dx<DIM_ORTHOBSP)&&(dz<DIM_ORTHOBSP))
				{
					node=new CBSPOrthoNode;
					node->type=ORTHOBSP_TERM;
					node->faces=NULL;
					node->groups=new CList <CFaceGROUP>;

					g=groups->GetFirst();
					while (g)
					{
						CopyGroup(node->groups->InsertLast(),g);
						g=groups->GetNext();
					}

					node->faces=new CList <int>;

					g=groups->GetFirst();
					while (g)
					{
						i=g->faces->GetFirst();
						while (i)
						{
							node->faces->Add(*i);
							i=g->faces->GetNext();
						}
						g=groups->GetNext();
					}
				}
				else node=BuildNodeWithFigures2D(Niter-1,groups,obj,(axe+1)%2);
			}
			else
			{
				node=new CBSPOrthoNode;
				node->type=ORTHOBSP_NODE;

				node->a=N.x;
				node->b=N.y;
				node->c=N.z;

				node->d=d=-DOT(N,M);

				nn=0;
				g=groups->GetFirst();
				while (g)
				{
					inf=sup=0;
					i=g->faces->GetFirst();
					while (i)
					{
						v=&(obj->Faces[*i].v[0]->Stok);
						s0=N.x*v->x+N.y*v->y+N.z*v->z+d;
						v=&(obj->Faces[*i].v[1]->Stok);
						s1=N.x*v->x+N.y*v->y+N.z*v->z+d;
						v=&(obj->Faces[*i].v[2]->Stok);
						s2=N.x*v->x+N.y*v->y+N.z*v->z+d;

						cc=(s0>0)+(s1>0)+(s2>0);

						if (cc==0)
						{
							inf++;
						}

						if (cc==3)
						{
							sup++;
						}

						if ((cc==1)||(cc==2))
						{

							inf++;
							sup++;
						}
						i=g->faces->GetNext();
					}


					if ((sup>0)&&(inf==0))
					{
						CopyGroup(plus.InsertLast(),g);
					}

					if ((sup>0)&&(inf>0))
					{
						CopyGroup(plus.InsertLast(),g);
						CopyGroup(moins.InsertLast(),g);
					}

					if ((inf>0)&&(sup==0))
					{
						CopyGroup(moins.InsertLast(),g);
					}
					g=groups->GetNext();
				}

				if (plus.Length()>0)
				{
					node->plus=BuildNodeWithFigures2D(Niter-1,&plus,obj,(axe+1)%2);
				}
				else
				{
					node->plus=new CBSPOrthoNode;
					node->plus->type=ORTHOBSP_TERM;
				}

				if (moins.Length()>0)
				{
					node->moins=BuildNodeWithFigures2D(Niter-1,&moins,obj,(axe+1)%2);
				}
				else
				{
					node->moins=new CBSPOrthoNode;
					node->moins->type=ORTHOBSP_TERM;
				}

				plus.Free();
				moins.Free();
			}
		}
	}

	return node;
}


void CBSPOrtho::FreeNode(CBSPOrthoNode * node)
{
	if (node)
	{
		if (node->plus) FreeNode(node->plus);
		if (node->moins) FreeNode(node->moins);

		if (node->groups) 
		{
			node->groups->Free();
			delete (node->groups);
		}

		if (node->faces) 
		{
			node->faces->Free();
			delete (node->faces);
		}
		
		delete node;
	}
}



void CBSPOrtho::Free()
{
	if (Groups) 
	{
		Groups->Free();
		delete Groups;
	}
	Groups=NULL;

	if (tmpGroups)
	{
		tmpGroups->Free();
		delete tmpGroups;
	}
	tmpGroups=NULL;

	if (root)
	{
		FreeNode(root);
		root=NULL;
	}

	if (ListeFaces)
	{
		ListeFaces->Free();
		delete ListeFaces;
		ListeFaces=NULL;
	}

	for (int n=0;n<64;n++)
	{
		if (ListeFacesTH[n])
		{
			ListeFacesTH[n]->Free();
			delete ListeFacesTH[n];
			ListeFacesTH[n]=NULL;
		}
	}
}

void CBSPOrtho::cleanNode(CBSPOrthoNode * node)
{
	if (node->type==ORTHOBSP_NODE)
	{
		if (node->plus) cleanNode(node->plus);
		if (node->moins) cleanNode(node->moins);
	}
	node->clean();
}

void CBSPOrtho::clean()
{
	if (root) cleanNode(root);
}
