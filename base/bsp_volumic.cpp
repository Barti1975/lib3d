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


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
comments,adds:

  #define _API_BSP_VOLUMIC_ONLY_FULL_BOOLEAN_ force full boolean method
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define _DEFINES_API3D_CODE_BUILDGROUPVOLUMEFAST_
//#define _API_BSP_VOLUMIC_ONLY_FULL_BOOLEAN_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX(a,b) ((a<b)?b:a) 
#define NB_MAX_FACES (2*(3*(MAX(obj->nFaces,32)*MAX(ref->nFaces,32)*8+obj->nFaces*8+ref->nFaces*8)))
#define NB_MAX_VERTICES (4*(4*MAX(obj->nFaces,32)*MAX(ref->nFaces,32) + 8*(3*obj->nVertices + 2*ref->nVertices)))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Volumic BSP tree
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "params.h"

#include <math.h>
#include <stdio.h>
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "bsp_volumic.h"
#include "edges.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumic::Inside(CVector &p)
{
	CBSPNode * node;
	float s;

	node=root;

	if (node!=NULL)
	{
		while (node->type==BSP_NODE)
		{
			s=node->a*p.x+node->b*p.y+node->c*p.z+node->d;
			if (s>-SMALLF) node=node->plus;
			else node=node->moins;
		}

		if (node->type==BSP_IN) return true;
		else return false;
	}
	
	return false;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumic::Outside(CVector &p)
{
	CBSPNode * node;
	float s;

	node=root;

	if (node!=NULL)
	{
		while (node->type==BSP_NODE)
		{
			s=node->a*p.x+node->b*p.y+node->c*p.z+node->d;
			if (s>-SMALLF) node=node->plus;
			else node=node->moins;
		}

		if (node->type==BSP_IN) return false;
		else return true;
	}
	
	return false;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::Free()
{
	Plans.Free();
	if (root)
	{
		FreeNode(root);
		root=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::FreeNode(CBSPNode * node)
{
	if (node->type==BSP_NODE)
	{
		FreeNode(node->moins);
		FreeNode(node->plus);
	}
	delete node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::ListePlans(CBSPNode * node)
{
	CPlane P,*p;
	bool present;

	if (node)
	{
		if (node->type==BSP_NODE)
		{
			P.Init(node->a,node->b,node->c,node->d);
			P.Normalise();

			p=Plans.GetFirst();
			present=false;
			while ((p)&&(!present))
			{
				if (p->a*node->a+p->b*node->b+p->c*node->c>1.0f-SMALLF)
				{
					if (f_abs((node->d-p->d))<SMALLF) present=true;
				}

				if (p->a*node->a+p->b*node->b+p->c*node->c<-1.0f+SMALLF)
				{
					if (f_abs((node->d+p->d))<SMALLF) present=true;
				}
				p=Plans.GetNext();
			}

			if (!present)
				Plans.Add(P);


			ListePlans(node->plus);
			ListePlans(node->moins);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumic::FaceInside(CBSPNode *node,CFace *f,float mul)
{
	bool res=false;

	if (node)
	{
		if (node->type==BSP_NODE)
		{
			if (node==root)
			{
				_x=(f->v[0]->Calc.x+f->v[1]->Calc.x+f->v[2]->Calc.x)/3.0f;
				_y=(f->v[0]->Calc.y+f->v[1]->Calc.y+f->v[2]->Calc.y)/3.0f;
				_z=(f->v[0]->Calc.z+f->v[1]->Calc.z+f->v[2]->Calc.z)/3.0f;
			}
			float s=(float) (node->a*_x +node->b*_y +node->c*_z + node->d);

			if (s>mul*SMALLF) res=FaceInside(node->plus,f,mul);
			else res=FaceInside(node->moins,f,mul);
		}
		else
		{
			if (node->type==BSP_IN) res=true;
			else res=false;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumic::ShortFaceInside(CBSPNode *node,CShortFaceMap *f)
{
	float s1;//,s2,s3;
	//int cc;
	bool res;
	CVector u;

	res=false;
	if (node)
	{
		if (node->type==BSP_NODE)
		{
			u=(f->s0->Calc+f->s1->Calc+f->s2->Calc)/3;
			s1=(float) (node->a*u.x +node->b*u.y +node->c*u.z + node->d);

			if (s1>0) res=ShortFaceInside(node->plus,f);
			else res=ShortFaceInside(node->moins,f);
		}
		else
		{
			if (node->type==BSP_IN) res=true;
			else res=false;
		}
	}

	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void _Box(CShortFaceMap *F,CVector *min,CVector *max)
{
	min->Init(10000,10000,10000);
	max->Init(-10000,-10000,-10000);
	if (F->v[0]->Calc.x<min->x) min->x=F->v[0]->Calc.x;
	if (F->v[0]->Calc.y<min->y) min->y=F->v[0]->Calc.y;
	if (F->v[0]->Calc.z<min->z) min->z=F->v[0]->Calc.z;
	if (F->v[0]->Calc.x>max->x) max->x=F->v[0]->Calc.x;
	if (F->v[0]->Calc.y>max->y) max->y=F->v[0]->Calc.y;
	if (F->v[0]->Calc.z>max->z) max->z=F->v[0]->Calc.z;

	if (F->v[1]->Calc.x<min->x) min->x=F->v[1]->Calc.x;
	if (F->v[1]->Calc.y<min->y) min->y=F->v[1]->Calc.y;
	if (F->v[1]->Calc.z<min->z) min->z=F->v[1]->Calc.z;
	if (F->v[1]->Calc.x>max->x) max->x=F->v[1]->Calc.x;
	if (F->v[1]->Calc.y>max->y) max->y=F->v[1]->Calc.y;
	if (F->v[1]->Calc.z>max->z) max->z=F->v[1]->Calc.z;

	if (F->v[2]->Calc.x<min->x) min->x=F->v[2]->Calc.x;
	if (F->v[2]->Calc.y<min->y) min->y=F->v[2]->Calc.y;
	if (F->v[2]->Calc.z<min->z) min->z=F->v[2]->Calc.z;
	if (F->v[2]->Calc.x>max->x) max->x=F->v[2]->Calc.x;
	if (F->v[2]->Calc.y>max->y) max->y=F->v[2]->Calc.y;
	if (F->v[2]->Calc.z>max->z) max->z=F->v[2]->Calc.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool _BoxBox(CVector &am,CVector &bm,CVector &cm,CVector &dm)
{
	if ((cm.x>bm.x)&&(dm.x>bm.x)) return false;
	if ((cm.y>bm.y)&&(dm.y>bm.y)) return false;
	if ((cm.z>bm.z)&&(dm.z>bm.z)) return false;

	if ((cm.x<am.x)&&(dm.x<am.x)) return false;
	if ((cm.y<am.y)&&(dm.y<am.y)) return false;
	if ((cm.z<am.z)&&(dm.z<am.z)) return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CalcFacesCollisionTag(CFace *F1,CFace *F2,float K)
{
    CVector am,bm,cm,dm,aa;
    
    am.Init(10000,10000,10000);
    bm.Init(-10000,-10000,-10000);
    if (F1->v[0]->Calc.x<am.x) am.x=F1->v[0]->Calc.x;
    if (F1->v[0]->Calc.y<am.y) am.y=F1->v[0]->Calc.y;
    if (F1->v[0]->Calc.z<am.z) am.z=F1->v[0]->Calc.z;
    if (F1->v[0]->Calc.x>bm.x) bm.x=F1->v[0]->Calc.x;
    if (F1->v[0]->Calc.y>bm.y) bm.y=F1->v[0]->Calc.y;
    if (F1->v[0]->Calc.z>bm.z) bm.z=F1->v[0]->Calc.z;

    if (F1->v[1]->Calc.x<am.x) am.x=F1->v[1]->Calc.x;
    if (F1->v[1]->Calc.y<am.y) am.y=F1->v[1]->Calc.y;
    if (F1->v[1]->Calc.z<am.z) am.z=F1->v[1]->Calc.z;
    if (F1->v[1]->Calc.x>bm.x) bm.x=F1->v[1]->Calc.x;
    if (F1->v[1]->Calc.y>bm.y) bm.y=F1->v[1]->Calc.y;
    if (F1->v[1]->Calc.z>bm.z) bm.z=F1->v[1]->Calc.z;

    if (F1->v[2]->Calc.x<am.x) am.x=F1->v[2]->Calc.x;
    if (F1->v[2]->Calc.y<am.y) am.y=F1->v[2]->Calc.y;
    if (F1->v[2]->Calc.z<am.z) am.z=F1->v[2]->Calc.z;
    if (F1->v[2]->Calc.x>bm.x) bm.x=F1->v[2]->Calc.x;
    if (F1->v[2]->Calc.y>bm.y) bm.y=F1->v[2]->Calc.y;
    if (F1->v[2]->Calc.z>bm.z) bm.z=F1->v[2]->Calc.z;
    
    aa=(am+bm)/2;
    am=(am-aa)*K+aa;
    bm=(bm-aa)*K+aa;

    cm.Init(10000,10000,10000);
    dm.Init(-10000,-10000,-10000);
    if (F2->v[0]->Calc.x<cm.x) cm.x=F2->v[0]->Calc.x;
    if (F2->v[0]->Calc.y<cm.y) cm.y=F2->v[0]->Calc.y;
    if (F2->v[0]->Calc.z<cm.z) cm.z=F2->v[0]->Calc.z;
    if (F2->v[0]->Calc.x>dm.x) dm.x=F2->v[0]->Calc.x;
    if (F2->v[0]->Calc.y>dm.y) dm.y=F2->v[0]->Calc.y;
    if (F2->v[0]->Calc.z>dm.z) dm.z=F2->v[0]->Calc.z;

    if (F2->v[1]->Calc.x<cm.x) cm.x=F2->v[1]->Calc.x;
    if (F2->v[1]->Calc.y<cm.y) cm.y=F2->v[1]->Calc.y;
    if (F2->v[1]->Calc.z<cm.z) cm.z=F2->v[1]->Calc.z;
    if (F2->v[1]->Calc.x>dm.x) dm.x=F2->v[1]->Calc.x;
    if (F2->v[1]->Calc.y>dm.y) dm.y=F2->v[1]->Calc.y;
    if (F2->v[1]->Calc.z>dm.z) dm.z=F2->v[1]->Calc.z;

    if (F2->v[2]->Calc.x<cm.x) cm.x=F2->v[2]->Calc.x;
    if (F2->v[2]->Calc.y<cm.y) cm.y=F2->v[2]->Calc.y;
    if (F2->v[2]->Calc.z<cm.z) cm.z=F2->v[2]->Calc.z;
    if (F2->v[2]->Calc.x>dm.x) dm.x=F2->v[2]->Calc.x;
    if (F2->v[2]->Calc.y>dm.y) dm.y=F2->v[2]->Calc.y;
    if (F2->v[2]->Calc.z>dm.z) dm.z=F2->v[2]->Calc.z;
    
    aa=(cm+dm)/2;
    cm=(cm-aa)*K+aa;
    dm=(dm-aa)*K+aa;
    
    return _BoxBox(am,bm,cm,dm);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CalcFacesCollision(CFace *F1,CFace *F2)
{
	static int ArrFace[3][2]={{0,1},{1,2},{2,0}};
	int res=0;
	CVector N;
	float ss[3];
	int n,n1,n2;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	CVector u,A,B;
	float s,s1,s2,s3;
	float a,b,c,d,t;
	float SMALLFx=SMALLF2;
	float SMALLFz=SMALLF2;

	N=F2->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F2->v[0]->Calc.x+b*F2->v[0]->Calc.y+c*F2->v[0]->Calc.z);

	ss[0]=DOT(N,F1->v[0]->Calc) + d;
	ss[1]=DOT(N,F1->v[1]->Calc) + d;
	ss[2]=DOT(N,F1->v[2]->Calc) + d;	
	
	s=DOT(F1->NormCalc,F2->NormCalc);
	//if (f_abs(s)>1.0f - SMALLF) return true;

	for (n=0;n<3;n++)
	{
		n1=ArrFace[n][0];
		n2=ArrFace[n][1];

		A=F1->v[n1]->Calc;
		B=F1->v[n2]->Calc;

		t=-ss[n1]/(ss[n2]-ss[n1]);

		if ((t>-SMALLFz)&&(t<1.0f+SMALLFz))
		{
			VECTORINTERPOL(I,t,B,A);

			VECTORSUB(a1,F2->v[1]->Calc,F2->v[0]->Calc);
			VECTORSUB(b1,I,F2->v[0]->Calc);
			CROSSPRODUCT(c1,b1,a1);
			VECTORNORMALISE(c1);
			DOTPRODUCT(s1,F2->NormCalc,c1);
			if (s1>-SMALLFx)
			{
				VECTORSUB(a2,F2->v[2]->Calc,F2->v[1]->Calc);
				VECTORSUB(b2,I,F2->v[1]->Calc);
				CROSSPRODUCT(c2,b2,a2);
				VECTORNORMALISE(c2);
				DOTPRODUCT(s2,F2->NormCalc,c2);
				if (s2>-SMALLFx)
				{
					VECTORSUB(a3,F2->v[0]->Calc,F2->v[2]->Calc);
					VECTORSUB(b3,I,F2->v[2]->Calc);
					CROSSPRODUCT(c3,b3,a3);
					VECTORNORMALISE(c3);
					DOTPRODUCT(s3,F2->NormCalc,c3);
					if (s3>-SMALLFx)
					{
						//return CalcFacesCollisionTag(F1,F2,8);
						return true;
					}
				}
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumic::tagBoolean(CObject3D * obj,CObject3D * ref,float mul)
{
    int n;
    
    obj->Calculate();
    ref->Calculate();
    
    /*
    for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;
    
    for (i=0;i<obj->nFaces;i++)
    {
        int face=0;
        for (j=0;j<ref->nFaces;j++)
        {
			if (CalcFacesCollisionTag(&obj->Faces[i],&ref->Faces[j],1.5f)) { ref->Faces[j].tag=0; face=1; }
			else
            if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
            else
                if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
        }
        if (face==0) obj->Faces[i].tag=-1;
        else obj->Faces[i].tag=0;
    }
        
    int cnt=0;
    for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
    
    if (cnt==0) return false;
    /*
    if (ref->nFaces<2048)
    {
        for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
        for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
    }
    /**/

    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
    for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;

    CObject3D *ref0=ref->OptimizeMeshBoolean();
    Build(ref0,mul,VALUEBUILD);
    ref0->Free();
    delete ref0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==0)
        {
            int cc=0;
            if (Inside(obj->Faces[n].v[0]->Calc)) cc++;
            if (Inside(obj->Faces[n].v[1]->Calc)) cc++;
            if (Inside(obj->Faces[n].v[2]->Calc)) cc++;
            //if (FaceInside(root,&obj->Faces[n],1)) obj->Faces[n].tag=0;
            //else obj->Faces[n].tag=-1;
            if (cc==0) obj->Faces[n].tag=-1;
            else obj->Faces[n].tag=0;
        }
    }
    
    if (root) FreeNode(root);
    root=NULL;
    /**/
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumic::NumberOfPlanes(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2)
{
    CObject3D *obj,*ref;
    int n,i,j;

    obj=obj0->Duplicate2();
//    obj=obj0->OptimizeMeshBoolean();

    if (ref0->Double1) ref=ref0->Double1->OptimizeMeshBoolean();
    else ref=ref0->OptimizeMeshBoolean();

    obj->CalculateNormals(-1);
    ref->CalculateNormals(-1);

    obj->Calculate();
    ref->Calculate();

    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

    for (i=0;i<obj->nFaces;i++)
    {
        int face=0;
        for (j=0;j<ref->nFaces;j++)
        {
            if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
            else
            if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
        }
        if (face==0) obj->Faces[i].tag=-1;
        else obj->Faces[i].tag=0;
    }

    int cnt=0;
    for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
    
    if (cnt==0)
    {
        obj->Free(); delete obj;
        ref->Free(); delete ref;
        return 0;
    }

    Build(ref,mul,VALUEBUILD);
    
    ListePlans(root);
    int NB=Plans.Length();
    
    Free();
    
    if (root) FreeNode(root);
    root=NULL;

    obj->Free(); delete obj;
    ref->Free(); delete ref;

    return NB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * tagosBoolean(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2)
{
	CObject3D *obj,*ref;
	int n,i,j;

	obj=obj0->Duplicate2();
//	obj=obj0->OptimizeMeshBoolean();

	if (ref0->Double1) ref=ref0->Double1->OptimizeMeshBoolean();
	else ref=ref0->OptimizeMeshBoolean();

	obj->CalculateNormals(-1);
	ref->CalculateNormals(-1);

	obj->Calculate();
	ref->Calculate();

	ref->SetF012Near();

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

	for (i=0;i<obj->nFaces;i++)
	{		
		int face=0;
		for (j=0;j<ref->nFaces;j++)
		{
			if (CalcFacesCollisionTag(&obj->Faces[i],&ref->Faces[j],1.0f)) { ref->Faces[j].tag=0; face=1; }
			else
			if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
			else
			if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
		}
		if (face==0) obj->Faces[i].tag=-1;
		else obj->Faces[i].tag=0;
	}

	for (n=0;n<ref->nFaces;n++)
	{
		int ok=0;
		if (ref->Faces[n].tag<0)
		{
			if (ref->Faces[n].f01>=0) ok+=(ref->Faces[ref->Faces[n].f01].tag==0);
			if (ref->Faces[n].f12>=0) ok+=(ref->Faces[ref->Faces[n].f12].tag==0);
			if (ref->Faces[n].f20>=0) ok+=(ref->Faces[ref->Faces[n].f20].tag==0);
			if (ok>=2) ref->Faces[n].tag=0;
		}
	}

	for (n=0;n<ref->nFaces;n++)
		if (ref->Faces[n].tag==0) ref->Faces[n].tag2=1; else ref->Faces[n].tag2=0;


	return ref;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPVolumic::Boolean(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,bool TAG)
{
	CObject3D *res,*obj,*ref;
	int n,nn,nv,nf,i,j;

	obj=obj0->Duplicate2();
//	obj=obj0->OptimizeMeshBoolean();

	if (ref0->Double1) ref=ref0->Double1->OptimizeMeshBoolean();
	else ref=ref0->OptimizeMeshBoolean();

	obj->CalculateNormals(-1);
	ref->CalculateNormals(-1);

	obj->Calculate();
	ref->Calculate();

	ref->SetF012Near();

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

	for (i=0;i<obj->nFaces;i++)
	{		
		int face=0;
		for (j=0;j<ref->nFaces;j++)
		{
			if (!TAG)
			{
				if (CalcFacesCollisionTag(&obj->Faces[i],&ref->Faces[j],2.0f)) { ref->Faces[j].tag=0; face=1; }
			}
			else
			{
				if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
				else
				if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
			}
		}
		if (face==0) obj->Faces[i].tag=-1;
		else obj->Faces[i].tag=0;
	}
	
	if (!TAG)
	{
		for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag2=0;

		for (n=0;n<ref->nFaces;n++)
		{
			int ok=0;
			if (ref->Faces[n].tag<0)
			{
				if (ref->Faces[n].f01>=0) ok+=(ref->Faces[ref->Faces[n].f01].tag==0);
				if (ref->Faces[n].f12>=0) ok+=(ref->Faces[ref->Faces[n].f12].tag==0);
				if (ref->Faces[n].f20>=0) ok+=(ref->Faces[ref->Faces[n].f20].tag==0);
				if (ok>=2) ref->Faces[n].tag2=1;
			}
		}

		for (n=0;n<ref->nFaces;n++)
			if (ref->Faces[n].tag2) ref->Faces[n].tag=0;
	}

	int cnt=0;
	for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
	
	if (cnt==0)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		return NULL;
	}

	faces.Free();
	VerticesS.Free();

	CShortVertexMap ** verts=new CShortVertexMap*[obj->nVertices];

	for (n=0;n<obj->nVertices;n++)
	{
		CShortVertexMap *vt=VerticesS.InsertLast();
		verts[n]=vt;
		vt->Stok=obj->Vertices[n].Stok;
		vt->Calc=obj->Vertices[n].Calc;
        vt->Map=obj->Vertices[n].Map;
        vt->ndx=n;
	}

	int nbout=0;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			CShortFaceMap *fc=faces.InsertLast();
			fc->tag=0;
			fc->nT=obj->Faces[n].nT;
			fc->nT2=obj->Faces[n].nT2;
			fc->nL=obj->Faces[n].nL;
			fc->ref=obj->Faces[n].ref;
			fc->n0=obj->Faces[n].v0;
			fc->n1=obj->Faces[n].v1;
			fc->n2=obj->Faces[n].v2;
			fc->s0=verts[fc->n0];
			fc->s1=verts[fc->n1];
			fc->s2=verts[fc->n2];
			nn++;
		}
		else nbout++;
	}

	delete [] verts;

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag2=ref->Faces[n].tag;

	grouped=true;

	Build(ref,mul,VALUEBUILD);
	
	nVertices=obj->nVertices;
	nfaces=nn;

	if (full)
	{
		ListePlans(root);
		nfaces=CutFacesMap(0,faces,nfaces,mul2);
		Plans.Free();
	}
	else InOut(0,0,root,mul2);

	res=new CObject3D;
	res->Init(nVertices,nfaces+obj->nFaces);
	
	nv=0;
	
	CShortVertexMap *vt=VerticesS.GetFirst();
	while (vt)
	{
		res->Vertices[nv].Stok=vt->Stok;
		res->Vertices[nv].Calc=vt->Calc;
        res->Vertices[nv].Map=vt->Map;
        for (int i=0;i<4;i++)
        {
            res->Vertices[nv].Index[i]=obj->Vertices[vt->ndx].Index[i];
            res->Vertices[nv].Weight[i]=obj->Vertices[vt->ndx].Weight[i];
        }
		nv++;
		vt=VerticesS.GetNext();
	}	
	/*
	Free();
	//for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
	
	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag3=0;

	for (n=0;n<ref->nFaces;n++)
		if (ref->Faces[n].tag2==0)
		{
			ref->Faces[n].tag3=1;
			if (ref->Faces[n].f01>=0) ref->Faces[ref->Faces[n].f01].tag3=1;
			if (ref->Faces[n].f12>=0) ref->Faces[ref->Faces[n].f12].tag3=1;
			if (ref->Faces[n].f20>=0) ref->Faces[ref->Faces[n].f20].tag3=1;
		}

	for (n=0;n<ref->nFaces;n++)
		if (ref->Faces[n].tag3) ref->Faces[n].tag=0; else ref->Faces[n].tag=-1;

	Build(ref,mul,VALUEBUILD);
	/**/
	
	nf=0;
	
	CShortFaceMap *fc=faces.GetFirst();
	while (fc)
	{
		res->Faces[nf].Init(fc->n0,fc->n1,fc->n2);
		res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
		res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
		res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
		res->Faces[nf].nT=fc->nT;
		res->Faces[nf].nT2=fc->nT2;
		res->Faces[nf].nL=fc->nL;
		res->Faces[nf].ref=fc->ref;
		res->Faces[nf].tag=0;
		bool inc=false;
		if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		if (inc) nf++;

		fc=faces.GetNext();
	}

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==-1)
		{
			res->Faces[nf].v0=obj->Faces[n].v0;
			res->Faces[nf].v1=obj->Faces[n].v1;
			res->Faces[nf].v2=obj->Faces[n].v2;
			res->Faces[nf].nT=obj->Faces[n].nT;
			res->Faces[nf].nT2=obj->Faces[n].nT2;
			res->Faces[nf].nL=obj->Faces[n].nL;
			res->Faces[nf].tag=0;
			res->Faces[nf].ref=obj->Faces[n].ref;
			res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
			res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
			res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
			bool inc=false;
			if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			if (inc) nf++;
		}
	}

	if (nf==0)
	{
		if (root) FreeNode(root);
		root=NULL;
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		res->Free();
		delete res;
		faces.Free();
		VerticesS.Free();
		return NULL;
	}

	res->nFaces=nf;
	res->SetFaces();
	res->CalculateNormals(-1);

	res->Rot=obj->Rot;
	res->Coo=obj->Coo;
	for (n=0;n<6;n++) res->P[n]=obj->P[n];
	res->nP=obj->nP;
	for (n=0;n<16;n++) res->Tab[n]=obj->Tab[n];
	res->Status=obj->Status;
	res->Attribut=obj->Attribut;
	res->env_mapping=obj->env_mapping;
	res->Tag=obj->Tag;
	
	if (root) FreeNode(root);
	root=NULL;
	faces.Free();
	VerticesS.Free();

	obj->Free(); delete obj;
	ref->Free(); delete ref;

	obj=res->Duplicate2();
	res->Free();
	delete res;

	return obj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::Boolean2Parts(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,CObject3D **pobj1,CObject3D **pobj2)
{
	CObject3D *res,*res2,*obj,*ref;
	int n,nn,nv,nf,nf2,i,j;

	*pobj1=NULL;
	*pobj2=NULL;

	obj=obj0->Duplicate2();
	if (ref0->Double1) ref=ref0->Double1->Duplicate2();
	else ref=ref0->Duplicate2();

	obj->CalculateNormals(-1);
	ref->CalculateNormals(-1);

	obj->Calculate();
	ref->Calculate();

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

	for (i=0;i<obj->nFaces;i++)
	{
		int face=0;
		for (j=0;j<ref->nFaces;j++)	
		{
			if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
			else
			if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
		}

		if (face==0) obj->Faces[i].tag=-1;
		else obj->Faces[i].tag=0;
	}

	int cnt=0;
	for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
	
	if (cnt==0)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		*pobj1=NULL;
		*pobj2=NULL;
		return;	
	}

	faces.Free();
	VerticesS.Free();

	CShortVertexMap ** verts=new CShortVertexMap*[obj->nVertices];

	for (n=0;n<obj->nVertices;n++)
	{
		CShortVertexMap *vt=VerticesS.InsertLast();
		verts[n]=vt;
		vt->Stok=obj->Vertices[n].Stok;
		vt->Calc=obj->Vertices[n].Calc;
        vt->Map=obj->Vertices[n].Map;
        vt->ndx=n;
		vt->tag=1;
	}

	int nbout=0;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			CShortFaceMap *fc=faces.InsertLast();
			fc->tag=0;
			fc->nT=obj->Faces[n].nT;
			fc->nT2=obj->Faces[n].nT2;
			fc->nL=obj->Faces[n].nL;
			fc->ref=obj->Faces[n].ref;
			fc->n0=obj->Faces[n].v0;
			fc->n1=obj->Faces[n].v1;
			fc->n2=obj->Faces[n].v2;
			fc->s0=verts[fc->n0];
			fc->s1=verts[fc->n1];
			fc->s2=verts[fc->n2];
			nn++;
		}
		else nbout++;
	}

	delete [] verts;

	Build(ref,mul,VALUEBUILD);

	if (root==NULL)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;

		faces.Free();
		VerticesS.Free();
		return;
	}
	
	nVertices=obj->nVertices;
	nfaces=nn;
	if (full)
	{
		ListePlans(root);
		nfaces=CutFacesMap(0,faces,nfaces,mul2);
		Plans.Free();
	}
	else InOut(0,0,root,mul2);

	res=new CObject3D;
	res->Init(nVertices,nfaces+obj->nFaces);

	res2=new CObject3D;
	res2->Init(nVertices,nfaces+obj->nFaces);

	nv=0;
	CShortVertexMap *vt=VerticesS.GetFirst();
	while (vt)
	{
		res->Vertices[nv].Stok=vt->Stok;
		res->Vertices[nv].Calc=vt->Calc;
		res->Vertices[nv].Map=vt->Map;
		if (nv<obj->nVertices) res->Vertices[nv].tag2=0; else res->Vertices[nv].tag2=1;

		res2->Vertices[nv].Stok=vt->Stok;
		res2->Vertices[nv].Calc=vt->Calc;
		res2->Vertices[nv].Map=vt->Map;
		if (nv<obj->nVertices) res2->Vertices[nv].tag2=0; else res2->Vertices[nv].tag2=1;

        for (int i=0;i<4;i++)
        {
            res->Vertices[nv].Index[i]=obj->Vertices[vt->ndx].Index[i];
            res->Vertices[nv].Weight[i]=obj->Vertices[vt->ndx].Weight[i];

            res2->Vertices[nv].Index[i]=obj->Vertices[vt->ndx].Index[i];
            res2->Vertices[nv].Weight[i]=obj->Vertices[vt->ndx].Weight[i];
        }

		nv++;
		vt=VerticesS.GetNext();
	}
	
	Free();
	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
	Build(ref,mul,VALUEBUILD);

	if (root==NULL)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;

		faces.Free();
		VerticesS.Free();
		return;
	}
	
	nf=0;
	nf2=0;

	CShortFaceMap *fc=faces.GetFirst();
	while (fc)
	{
		res->Faces[nf].Init(fc->n0,fc->n1,fc->n2);
		res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
		res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
		res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
		res->Faces[nf].nT=fc->nT;
		res->Faces[nf].nT2=fc->nT2;
		res->Faces[nf].nL=fc->nL;
		res->Faces[nf].ref=fc->ref;
		res->Faces[nf].tag=0;		

		bool inc=false;
		if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		if (inc) nf++;
		else
		{
			res2->Faces[nf2].Init(fc->n0,fc->n1,fc->n2);
			res2->Faces[nf2].v[0]=&res2->Vertices[res2->Faces[nf2].v0];
			res2->Faces[nf2].v[1]=&res2->Vertices[res2->Faces[nf2].v1];
			res2->Faces[nf2].v[2]=&res2->Vertices[res2->Faces[nf2].v2];
			res2->Faces[nf2].nT=fc->nT;
			res2->Faces[nf2].nT2=fc->nT2;
			res2->Faces[nf2].nL=fc->nL;
			res2->Faces[nf2].ref=fc->ref;
			res2->Faces[nf2].tag=0;
			nf2++;
		}

		fc=faces.GetNext();
	}

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==-1)
		{
			res->Faces[nf].v0=obj->Faces[n].v0;
			res->Faces[nf].v1=obj->Faces[n].v1;
			res->Faces[nf].v2=obj->Faces[n].v2;
			res->Faces[nf].nT=obj->Faces[n].nT;
			res->Faces[nf].nT2=obj->Faces[n].nT2;
			res->Faces[nf].nL=obj->Faces[n].nL;
			res->Faces[nf].tag=0;
			res->Faces[nf].ref=obj->Faces[n].ref;
			res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
			res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
			res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
			bool inc=false;
			if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			if (inc) nf++;
			else
			{
				res2->Faces[nf2].v0=obj->Faces[n].v0;
				res2->Faces[nf2].v1=obj->Faces[n].v1;
				res2->Faces[nf2].v2=obj->Faces[n].v2;
				res2->Faces[nf2].nT=obj->Faces[n].nT;
				res2->Faces[nf2].nT2=obj->Faces[n].nT2;
				res2->Faces[nf2].nL=obj->Faces[n].nL;
				res2->Faces[nf2].tag=0;
				res2->Faces[nf2].ref=obj->Faces[n].ref;
				res2->Faces[nf2].v[0]=&res2->Vertices[res2->Faces[nf2].v0];
				res2->Faces[nf2].v[1]=&res2->Vertices[res2->Faces[nf2].v1];
				res2->Faces[nf2].v[2]=&res2->Vertices[res2->Faces[nf2].v2];

				nf2++;
			}
		}
	}

	if (nf==0)
	{
		if (root) FreeNode(root);
		root=NULL;
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		res->Free();
		delete res;
		res2->Free();
		delete res2;
		faces.Free();
		VerticesS.Free();
		return;	
	}

	res->nFaces=nf;
	res->SetFaces();
	res->CalculateNormals(-1);
	res->Rot=obj->Rot;
	res->Coo=obj->Coo;
	for (n=0;n<6;n++) res->P[n]=obj->P[n];
	res->nP=obj->nP;
	for (n=0;n<16;n++) res->Tab[n]=obj->Tab[n];
	res->Status=obj->Status;
	res->Attribut=obj->Attribut;
	res->env_mapping=obj->env_mapping;
	res->Tag=obj->Tag;

	res2->nFaces=nf2;
	res2->SetFaces();
	res2->CalculateNormals(-1);
	res2->Rot=obj->Rot;
	res2->Coo=obj->Coo;
	for (n=0;n<6;n++) res2->P[n]=obj->P[n];
	res2->nP=obj->nP;
	for (n=0;n<16;n++) res2->Tab[n]=obj->Tab[n];
	res2->Status=obj->Status;
	res2->Attribut=obj->Attribut;
	res2->env_mapping=obj->env_mapping;
	res2->Tag=obj->Tag;

	*pobj1=res->Duplicate2();
	*pobj2=res2->Duplicate2();

	res->Free(); delete res;
	res2->Free(); delete res2;

	if (root) FreeNode(root);
	root=NULL;
	faces.Free();
	VerticesS.Free();

	obj->Free(); delete obj;
	ref->Free(); delete ref;

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add CShortFaceMap *faces,int NF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::InOutNoMap(int mask,int N,CBSPNode * node,float mul)
{
	int res;
	int nm,np;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc,cc2;
	int n;
	float mul0=mul;
	float mul1=mul;

	nm=np=0;
	if (node)
	{
		if (node->type==BSP_NODE)
		{
			CShortVertexMap * vt=VerticesS.GetFirst();
			while (vt)
			{
				vt->coef1=(float) (vt->Calc.x*node->a +vt->Calc.y*node->b +vt->Calc.z*node->c + node->d);
				vt=VerticesS.GetNext();
			}

			int count=nfaces;

			f=faces.GetFirst();
			for (n=0;n<count;n++)
			{
				cc2=cc=f->tag;
				cc2=cc2&1;
				cc=cc>>1;

				if ((cc==N)&&(cc2==mask))
				{
					P1=f->s0;
					P2=f->s1;
					P3=f->s2;
					s1=P1->coef1;
					s2=P2->coef1;
					s3=P3->coef1;

					cc=(s1>mul1*SMALLF)+(s2>mul1*SMALLF)+(s3>mul1*SMALLF);

					if (cc==0) {f->tag=(N+1)*2 + 0;nm++;}
					if (cc==3) {f->tag=(N+1)*2 + 1;np++;}

					if ((cc==1)||(cc==2))
					{
						if (cc==1) s=1; else s=-1;

						t1=t2=-1;

						res=0;

						if (s*s1>s*mul0*SMALLF)
						{
							A=f->n0;B=f->n1;C=f->n2;
							PA=P1;
							PB=P2;
							PC=P3;
							ss1=s1;ss2=s2;ss3=s3;
							res=1;
						}
						ss=s1;

						if (((s*s2>s*mul0*SMALLF)&&(res==0))||((s*s2>s*mul0*SMALLF)&&(res==1)&&(s*s2>s*s1)))
						{
							ss=s2;
							A=f->n1;B=f->n2;C=f->n0;
							ss1=s2;ss2=s3;ss3=s1;
							res=1;
							PA=P2;
							PB=P3;
							PC=P1;
						}
						
						if (((s*s3>s*mul0*SMALLF)&&(res==0))||((s*s3>s*mul0*SMALLF)&&(res==1)&&(s*s3>s*ss)))
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


						if ((t1<SMALLF3)&&(t2<SMALLF3))
						{
							if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
							else {f->tag=(N+1)*2 + 1;np++;}
						}
						else
						if ((1-t1<SMALLF3)&&(1-t2<SMALLF3))
						{
							if (cc==1) { f->tag=(N+1)*2 + 1;np++;}
							else {f->tag=(N+1)*2 + 0;nm++;}
						}
						else
						{

							t=-ss1/(ss2-ss1);
							sAB=VerticesS.InsertLast();
							VECTORINTERPOL(sAB->Stok,t,PB->Stok,PA->Stok);
							VECTORINTERPOL(sAB->Calc,t,PB->Calc,PA->Calc);
							sAB->tag=0;
							iAB=nVertices;
							nVertices++;

							t=-ss1/(ss3-ss1);
							sAC=VerticesS.InsertLast();
							VECTORINTERPOL(sAC->Stok,t,PC->Stok,PA->Stok);
							VECTORINTERPOL(sAC->Calc,t,PC->Calc,PA->Calc);
							sAC->tag=0;
							iAC=nVertices;
							nVertices++;

							tmp.nL=f->nL;
							tmp.nT=f->nT;
							tmp.nT2=f->nT2;
							tmp.tag=f->tag;
							tmp.ref=f->ref;

							
							tmp.n0=A;
							tmp.n1=iAB;
							tmp.n2=iAC;
							tmp.s0=PA;
							tmp.s1=sAB;
							tmp.s2=sAC;
							tmp.lock=1;							
							*f=tmp;

							tmp.n0=iAC;
							tmp.n1=iAB;
							tmp.n2=B;
							tmp.s0=sAC;
							tmp.s1=sAB;
							tmp.s2=PB;
							tmp.lock=1;
							CShortFaceMap *fcnf=faces.Add(tmp);

							tmp.n0=iAC;
							tmp.n1=B;
							tmp.n2=C;
							tmp.s0=sAC;
							tmp.s1=PB;
							tmp.s2=PC;
							tmp.lock=1;
							CShortFaceMap *fcnf1=faces.Add(tmp);

							if (cc==1)
							{
								nm+=2;
								np+=1;
								f->tag=2*(N+1) + 1;
								fcnf->tag=2*(N+1) + 0;
								fcnf1->tag=2*(N+1) + 0;
							}
							else
							{
								nm+=1;
								np+=2;
								f->tag=2*(N+1) + 0;
								fcnf->tag=2*(N+1) + 1;
								fcnf1->tag=2*(N+1) + 1;
							}
							nfaces+=2;
						}
					}
				}

				f=faces.GetNext();
			}

			if (np>0) InOutNoMap(1,N+1,node->plus,mul);
			if (nm>0) InOutNoMap(0,N+1,node->moins,mul);
		}
		else
		{
			if (node->type==BSP_IN)
			{
				f=faces.GetFirst();
				while (f)
				{
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;

					if ((cc==N)&&(cc2==mask)) f->tag=-1;

					f=faces.GetNext();
				}
			}
			else
			{
				f=faces.GetFirst();
				while (f)
				{
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;

					if ((cc==N)&&(cc2==mask)) f->tag=-2;

					f=faces.GetNext();
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::InOut(int mask,int N,CBSPNode * node,float mul)
{
	#define SMALLFW SMALLF
	int res;
	int nm,np;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc,cc2;
	int n;

	nm=np=0;
	if (node)
	{
		if (node->type==BSP_NODE)
		{
			CShortVertexMap * vt=VerticesS.GetFirst();
			while (vt)
			{
				vt->tag=0;
				vt=VerticesS.GetNext();
			}

			int count=nfaces;

			f=faces.GetFirst();
			for (n=0;n<count;n++)
			{
				cc2=cc=f->tag;
				cc2=cc2&1;
				cc=cc>>1;

				if ((cc==N)&&(cc2==mask))
				{
					P1=f->s0;
					P2=f->s1;
					P3=f->s2;

					if (P1->tag) s1=P1->coef1; else { P1->tag=1; s1=P1->coef1=(float) (P1->Calc.x*node->a + P1->Calc.y*node->b + P1->Calc.z*node->c + node->d); }
					if (P2->tag) s2=P2->coef1; else { P2->tag=1; s2=P2->coef1=(float) (P2->Calc.x*node->a + P2->Calc.y*node->b + P2->Calc.z*node->c + node->d); }
					if (P3->tag) s3=P3->coef1; else { P3->tag=1; s3=P3->coef1=(float) (P3->Calc.x*node->a + P3->Calc.y*node->b + P3->Calc.z*node->c + node->d); }

					cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);
				
					if (cc==0) {f->tag=(N+1)*2 + 0;nm++;}
					if (cc==3) {f->tag=(N+1)*2 + 1;np++;}
				
					if ((cc==1)||(cc==2))
					{
						if (cc==1) s=1; else s=-1;
						t1=t2=-1;

						ss1=0;

						res=0;
						if (s*s1>=s*mul*SMALLF)
						{
							A=f->n0;B=f->n1;C=f->n2;
							PA=P1;
							PB=P2;
							PC=P3;
							ss1=s1;ss2=s2;ss3=s3;
							ss=s1;
							res=1;
						}
						else
						if (s*s2>=s*mul*SMALLF)
						{
							ss=s2;
							A=f->n1;B=f->n2;C=f->n0;
							ss1=s2;ss2=s3;ss3=s1;
							res=1;
							PA=P2;
							PB=P3;
							PC=P1;
						}
						else
						if (s*s3>=s*mul*SMALLF)
						{
							A=f->n2;B=f->n0;C=f->n1;
							ss1=s3;ss2=s1;ss3=s2;
							res=1;
							PA=P3;
							PB=P1;
							PC=P2;
						}

						if (res==1)
						{
							t1=-ss1/(ss2-ss1);
							t2=-ss1/(ss3-ss1);

							if ((t1<SMALLFW)&&(t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((t1<SMALLFW)&&(1.0f-t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((1.0f-t1<SMALLFW)&&(t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((1.0f-t1<SMALLFW)&&(1.0f-t2<SMALLFW))
							{
								if (cc==1) { f->tag=(N+1)*2 + 1;np++;}
								else {f->tag=(N+1)*2 + 0;nm++;}
							}
							else
							{
								t=t1;
								if (t<SMALLFW)
								{
									sAB=PA;
									iAB=A;
								}
								else
								if (t>1.0f-SMALLFW)
								{
									sAB=PB;
									iAB=B;
								}
								else
								{
									sAB=VerticesS.InsertLast();
									VECTORINTERPOL(sAB->Stok,t,PB->Stok,PA->Stok);
									VECTORINTERPOL(sAB->Calc,t,PB->Calc,PA->Calc);
									VECTOR2INTERPOL(sAB->Map,t,PB->Map,PA->Map);
                                    if (t>0.5f) sAB->ndx=PA->ndx; else sAB->ndx=PB->ndx;

									sAB->tag=0;
									iAB=nVertices;
									nVertices++;
								}

								t=t2;
								if (t<SMALLFW)
								{
									sAC=PA;
									iAC=A;
								}
								else
								if (t>1.0f-SMALLFW)
								{
									sAC=PC;
									iAC=C;
								}
								else
								{
									sAC=VerticesS.InsertLast();
									VECTORINTERPOL(sAC->Stok,t,PC->Stok,PA->Stok);
									VECTORINTERPOL(sAC->Calc,t,PC->Calc,PA->Calc);
									VECTOR2INTERPOL(sAC->Map,t,PC->Map,PA->Map);
                                    if (t>0.5f) sAC->ndx=PA->ndx; else sAC->ndx=PC->ndx;

									sAC->tag=0;
									iAC=nVertices;
									nVertices++;
								}

								tmp.nL=f->nL;
								tmp.nT=f->nT;
								tmp.nT2=f->nT2;
								tmp.tag=f->tag;
								tmp.ref=f->ref;

								tmp.n0=A;
								tmp.n1=iAB;
								tmp.n2=iAC;
								tmp.s0=PA;
								tmp.s1=sAB;
								tmp.s2=sAC;
								tmp.lock=1;
								*f=tmp;
							
								tmp.n0=iAC;
								tmp.n1=iAB;
								tmp.n2=B;
								tmp.s0=sAC;
								tmp.s1=sAB;
								tmp.s2=PB;
								tmp.lock=1;
								CShortFaceMap *fcnf=faces.Add(tmp);

								tmp.n0=iAC;
								tmp.n1=B;
								tmp.n2=C;
								tmp.s0=sAC;
								tmp.s1=PB;
								tmp.s2=PC;
								tmp.lock=1;
								CShortFaceMap *fcnf1=faces.Add(tmp);
				
								if (cc==1)
								{
									nm+=2;
									np+=1;
									f->tag=2*(N+1) + 1;
									fcnf->tag=2*(N+1) + 0;
									fcnf1->tag=2*(N+1) + 0;
								}
								else
								{
									nm+=1;
									np+=2;
									f->tag=2*(N+1) + 0;
									fcnf->tag=2*(N+1) + 1;
									fcnf1->tag=2*(N+1) + 1;
								}
								nfaces+=2;
							}
						}
					}
				}

				f=faces.GetNext();
			}

			if (np>0) InOut(1,N+1,node->plus,mul);
			if (nm>0) InOut(0,N+1,node->moins,mul);
		}
		else
		{
			if (node->type==BSP_IN)
			{
				f=faces.GetFirst();
				while (f)
				{
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;
					if ((cc==N)&&(cc2==mask)) f->tag=-1;
					f=faces.GetNext();
				}
			}
			else
			{
				f=faces.GetFirst();
				while (f)
				{
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;
					if ((cc==N)&&(cc2==mask)) f->tag=-2;
					f=faces.GetNext();
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumic::CutFaces(int nP,CList<CShortFaceMap> &faces,int NF,float mul)
{
	int res;
	int NBRE;
	int nx;
	//CList <CShortFaceMap> decoupees;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	CPlane *p;
	int A,B,C;
//	int nAB,nAC;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int nb,LEN;
	int nf;
	float mul0;

	mul0=-1;


	nf=NF;
	LEN=NF;

	nx=0;
	if (nP<Plans.Length())
	{
		p=Plans[nP];

		CShortVertexMap * vt=VerticesS.GetFirst();
		while (vt)
		{
			vt->coef1=(float) (vt->Calc.x*p->a +vt->Calc.y*p->b +vt->Calc.z*p->c + p->d);
			vt=VerticesS.GetNext();
		}

		f=faces.GetFirst();
		for (nb=0;nb<LEN;nb++)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->coef1;
			s2=P2->coef1;
			s3=P3->coef1;
			//s1=(float) (P1->Calc.x*p->a +P1->Calc.y*p->b +P1->Calc.z*p->c + p->d);
			//s2=(float) (P2->Calc.x*p->a +P2->Calc.y*p->b +P2->Calc.z*p->c + p->d);
			//s3=(float) (P3->Calc.x*p->a +P3->Calc.y*p->b +P3->Calc.z*p->c + p->d);

			cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;

				if (s*s1>s*mul0*SMALLF)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
				}
				ss=s1;
				
				if (((s*s2>s*mul0*SMALLF)&&(res==0))||((s*s2>s*mul0*SMALLF)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;

				}
				
				if (((s*s3>s*mul0*SMALLF)&&(res==0))||((s*s3>s*mul0*SMALLF)&&(res==1)&&(s*s3>s*ss)))
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
					// ERREUR
				}

				if ((t1<SMALLF3)&&(t2<SMALLF3))
				{
					//decoupees.Add(*f);
				}
				else
				if ((1-t1<SMALLF3)&&(1-t2<SMALLF3))
				{
					//decoupees.Add(*f);
				}
				else
				{
				//	t=-ss1/(ss2-ss1);
					t=t1;
					if (t<SMALLF2)
					{
						sAB=PA;
						iAB=A;
					}
					else
					if (t>1-SMALLF2)
					{
						sAB=PB;
						iAB=B;
					}
					else
					{
						sAB=VerticesS.InsertLast();
						sAB->Stok=(1-t)*(PA->Stok) + t*(PB->Stok);
						sAB->Calc=(1-t)*(PA->Calc) + t*(PB->Calc);
						sAB->tag=0;
						
						iAB=nVertices;
						nVertices++;						
					}

					t=t2;
					//t=-ss1/(ss3-ss1);
					if (t<SMALLF2)
					{
						sAC=PA;
						iAC=A;
					}
					else
					if (t>1-SMALLF2)
					{
						sAC=PC;
						iAC=C;
					}
					else
					{
						sAC=VerticesS.InsertLast();
						sAC->Stok=(1-t)*(PA->Stok) + t*(PC->Stok);
						sAC->Calc=(1-t)*(PA->Calc) + t*(PC->Calc);
						sAC->tag=0;
						
						iAC=nVertices;
						nVertices++;
					}

/*
					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.tag=f->tag;
					tmp.ref=f->ref;
					tmp.N=f->N;
*/					
					tmp.n0=A;
					tmp.n1=iAB;
					tmp.n2=iAC;
					tmp.s0=PA;
					tmp.s1=sAB;
					tmp.s2=sAC;
					*f=tmp;
					
					tmp.n0=iAC;
					tmp.n1=iAB;
					tmp.n2=B;
					tmp.s0=sAC;
					tmp.s1=sAB;
					tmp.s2=PB;
					faces.Add(tmp);


					tmp.n0=iAC;
					tmp.n1=B;
					tmp.n2=C;
					tmp.s0=sAC;
					tmp.s1=PB;
					tmp.s2=PC;
					faces.Add(tmp);
		
					nf+=2;				
				}
			}

			f=faces.GetNext();
		}

		NBRE=CutFaces(nP+1,faces,nf,mul);
	//	decoupees.Free();

	}
	else
	{
		NBRE=NF;
	}

	return NBRE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumic::CutFacesMap(int nP,CList<CShortFaceMap> &faces,int NF,float mul)
{
#define SMALLFWW SMALLF
	int res;
	int NBRE;
	int nx;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	CPlane *p;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int nb,LEN;
	int nf;

	nf=NF;
	LEN=NF;
	nx=0;
	if (nP<Plans.Length())
	{
		p=Plans[nP];

		CShortVertexMap * vt=VerticesS.GetFirst();
		while (vt)
		{
			vt->coef1=(float) (vt->Calc.x*p->a +vt->Calc.y*p->b +vt->Calc.z*p->c + p->d);
			vt=VerticesS.GetNext();
		}

		f=faces.GetFirst();
		for (nb=0;nb<LEN;nb++)
		{
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->coef1;
			s2=P2->coef1;
			s3=P3->coef1;
			//s1=(float) (P1->Calc.x*p->a +P1->Calc.y*p->b +P1->Calc.z*p->c + p->d);
			//s2=(float) (P2->Calc.x*p->a +P2->Calc.y*p->b +P2->Calc.z*p->c + p->d);
			//s3=(float) (P3->Calc.x*p->a +P3->Calc.y*p->b +P3->Calc.z*p->c + p->d);

			cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;
				if (s*s1>=s*mul*SMALLF)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					ss=s1;
					res=1;
				}
				else
				if (s*s2>=s*mul*SMALLF)
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;
				}
				else
				if (s*s3>=s*mul*SMALLF)
				{
					A=f->n2;B=f->n0;C=f->n1;
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3;
					PB=P1;
					PC=P2;
				}
				
				if (res==1) 
				{
					t1=-ss1/(ss2-ss1);
					t2=-ss1/(ss3-ss1);

					t=t1;
					if (t<SMALLFWW)
					{
						sAB=PA;
						iAB=A;
					}
					else
					if (t>1.0f-SMALLFWW)
					{
						sAB=PB;
						iAB=B;
					}
					else
					{
						sAB=VerticesS.InsertLast();
						sAB->Stok=(1-t)*(PA->Stok) + t*(PB->Stok);
						sAB->Calc=(1-t)*(PA->Calc) + t*(PB->Calc);
						sAB->Map=(1-t)*(PA->Map) + t*(PB->Map);
                        if (t>0.5f) sAB->ndx=PA->ndx; else sAB->ndx=PB->ndx;
						sAB->tag=0;
						sAB->tag2=0;
						
						iAB=nVertices;
						nVertices++;
					}

					t=t2;
					if (t<SMALLFWW)
					{
						sAC=PA;
						iAC=A;
					}
					else
					if (t>1.0f-SMALLFWW)
					{
						sAC=PC;
						iAC=C;
					}
					else
					{
						sAC=VerticesS.InsertLast();
						sAC->Stok=(1-t)*(PA->Stok) + t*(PC->Stok);
						sAC->Calc=(1-t)*(PA->Calc) + t*(PC->Calc);
						sAC->Map=(1-t)*(PA->Map) + t*(PC->Map);
                        if (t>0.5f) sAC->ndx=PA->ndx; else sAC->ndx=PC->ndx;
						sAC->tag=0;
						sAC->tag2=0;
						
						iAC=nVertices;
						nVertices++;
					}

					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.nT2=f->nT2;
					tmp.tag=f->tag;
					tmp.ref=f->ref;
					
					tmp.n0=A;
					tmp.n1=iAB;
					tmp.n2=iAC;
					tmp.s0=PA;
					tmp.s1=sAB;
					tmp.s2=sAC;
					*f=tmp;
					
					tmp.n0=iAC;
					tmp.n1=iAB;
					tmp.n2=B;
					tmp.s0=sAC;
					tmp.s1=sAB;
					tmp.s2=PB;
					faces.Add(tmp);

					tmp.n0=iAC;
					tmp.n1=B;
					tmp.n2=C;
					tmp.s0=sAC;
					tmp.s1=PB;
					tmp.s2=PC;
					faces.Add(tmp);
		
					nf+=2;					
				}
			}

			f=faces.GetNext();
		}

		NBRE=CutFacesMap(nP+1,faces,nf,mul);
	}
	else
	{
		NBRE=NF;
	}

	return NBRE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildVolumeFaceNormal(CObject3D *obj,int nf,float mul,float mul2)
{
    int n0,n1;
    CVector u;
    CBSPNode *node1,*node2,*node3,*nodeplus,*nodemoins;
    
    CVector N=obj->Faces[nf].NormCalc;
    
    node1=new CBSPNode;
    node2=new CBSPNode;
    node3=new CBSPNode;
    
    n0=obj->Faces[nf].v0;
    n1=obj->Faces[nf].v1;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node1->a=u.x;
    node1->b=u.y;
    node1->c=u.z;
    node1->d=-(u||obj->Vertices[n0].Calc);
    node1->type=BSP_NODE;
    
    n0=obj->Faces[nf].v1;
    n1=obj->Faces[nf].v2;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node2->a=u.x;
    node2->b=u.y;
    node2->c=u.z;
    node2->d=-(u||obj->Vertices[n0].Calc);
    node2->type=BSP_NODE;
    
    n0=obj->Faces[nf].v2;
    n1=obj->Faces[nf].v0;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node3->a=u.x;
    node3->b=u.y;
    node3->c=u.z;
    node3->d=-(u||obj->Vertices[n0].Calc);
    node3->type=BSP_NODE;
    
    root=node1;
    node1->moins=node2;
    node2->moins=node3;
    nodeplus=new CBSPNode;
    nodemoins=new CBSPNode;
    nodeplus->type=BSP_OUT;
    nodemoins->type=BSP_IN;
    node3->plus=nodeplus;
    node3->moins=nodemoins;
    
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node2->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node1->plus=nodeplus;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildVolumeQuadNormal(CVector &N,CVector &A,CVector &B,CVector &C,CVector &D,float mul,float mul2)
{
    CVector u;
    CBSPNode *node1,*node2,*node3,*node4,*nodeplus,*nodemoins;
    
    node1=new CBSPNode;
    node2=new CBSPNode;
    node3=new CBSPNode;
    node4=new CBSPNode;
    
    u=N^(B-A);
    u.Normalise();
    node1->a=u.x;
    node1->b=u.y;
    node1->c=u.z;
    node1->d=-(u||A);
    node1->type=BSP_NODE;
    
    u=N^(C-B);
    u.Normalise();
    node2->a=u.x;
    node2->b=u.y;
    node2->c=u.z;
    node2->d=-(u||B);
    node2->type=BSP_NODE;
    
    u=N^(D-C);
    u.Normalise();
    node3->a=u.x;
    node3->b=u.y;
    node3->c=u.z;
    node3->d=-(u||C);
    node3->type=BSP_NODE;

    u=N^(A-D);
    u.Normalise();
    node4->a=u.x;
    node4->b=u.y;
    node4->c=u.z;
    node4->d=-(u||D);
    node4->type=BSP_NODE;

    root=node1;
    node1->moins=node2;
    node2->moins=node3;
    node3->moins=node4;
    
    nodeplus=new CBSPNode;
    nodemoins=new CBSPNode;
    nodeplus->type=BSP_OUT;
    nodemoins->type=BSP_IN;
    node4->plus=nodeplus;
    node4->moins=nodemoins;
    
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node3->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node2->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node1->plus=nodeplus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildVolume(CObject3D *obj,int nf,float mul,float mul2,CVector L,int tag)
{
	int n,nn;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri1,tri2;
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	EdgeListD Edge;

	nn=0;
	for	(n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			nn++;
		}
	}

	Edge.Init(nn*3);

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			Edge.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
			Edge.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
			Edge.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
		}
	}

	//Edge.DelDoubleEdges();


	for (n=0;n<Edge.nList;n++)
	{
		sousliste=Triangles.InsertLast();

		A=(L+obj->Vertices[Edge.List[n].a].Calc)/2;
		B=(L+obj->Vertices[Edge.List[n].b].Calc)/2;
		C=obj->Vertices[Edge.List[n].b].Calc;
		D=obj->Vertices[Edge.List[n].a].Calc;
		tri1.A=A;
		tri1.B=B;
		tri1.C=C;
		tri2.A=A;
		tri2.B=C;
		tri2.C=D;
		u=(obj->Vertices[Edge.List[n].a].Calc-L)^(obj->Vertices[Edge.List[n].b].Calc -L);
		u.Normalise();
		sousliste->List.Add(tri1);
		sousliste->List.Add(tri2);
		sousliste->Norm=mul*u;
	}

	root=BuildNode(&Triangles,mul2);

	Edge.Free();

	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}

	Triangles.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildVolumeFace(CObject3D *obj,int nf,float mul,float mul2,CVector L)
{
	int n0,n1;
	CVector u;
	CMatrix Mobj,Mrot;
	CBSPNode *node1,*node2,*node3,*nodeplus,*nodemoins;

	node1=new CBSPNode;
	node2=new CBSPNode;
	node3=new CBSPNode;
	
	n0=obj->Faces[nf].v0;
	n1=obj->Faces[nf].v1;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node1->a=u.x;
	node1->b=u.y;
	node1->c=u.z;
	node1->d=-(u||(L));
	node1->type=BSP_NODE;

	n0=obj->Faces[nf].v1;
	n1=obj->Faces[nf].v2;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node2->a=u.x;
	node2->b=u.y;
	node2->c=u.z;
	node2->d=-(u||(L));
	node2->type=BSP_NODE;

	n0=obj->Faces[nf].v2;
	n1=obj->Faces[nf].v0;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node3->a=u.x;
	node3->b=u.y;
	node3->c=u.z;
	node3->d=-(u||(L));
	node3->type=BSP_NODE;

	root=node1;
	node1->moins=node2;
	node2->moins=node3;
	nodeplus=new CBSPNode;
	nodemoins=new CBSPNode;
	nodeplus->type=BSP_OUT;
	nodemoins->type=BSP_IN;
	node3->plus=nodeplus;
	node3->moins=nodemoins;

	nodeplus=new CBSPNode;
	nodeplus->type=BSP_OUT;
	node2->plus=nodeplus;
	nodeplus=new CBSPNode;
	nodeplus->type=BSP_OUT;
	node1->plus=nodeplus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DEFINES_API3D_CODE_BUILDGROUPVOLUMEFAST_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L)
{
	CShortFaceMapGROUP *g=(CShortFaceMapGROUP*) vg;
	int *i,n;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri;
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	CFigure * f;
	int array[512];
	int na;
	int a,b;


	f=g->figures->GetFirst();
	while (f)
	{
		n=0;
		i=f->List.GetFirst();
		while (i)
		{
			array[n]=*i;
			n++;
			i=f->List.GetNext();
		}
		na=f->List.Length();

		for (n=0;n<na;n++)
		{
			a=array[n];
			b=array[(n+1)%na];

			sousliste=Triangles.InsertLast();

			C=obj->Vertices[a].Calc;
			D=obj->Vertices[b].Calc;

			B.x=(L.x+C.x)*0.5f;
			B.y=(L.y+C.y)*0.5f;
			B.z=(L.z+C.z)*0.5f;

			A.x=(L.x+D.x)*0.5f;
			A.y=(L.y+D.y)*0.5f;
			A.z=(L.z+D.z)*0.5f;

			tri.A=A;
			tri.B=B;
			tri.C=C;
			sousliste->List.Add(tri);
			tri.A=A;
			tri.B=C;
			tri.C=D;
			sousliste->List.Add(tri);
			

			CVector u1,u2;
			VECTORSUB(u1,C,L);
			VECTORSUB(u2,D,L);
			CROSSPRODUCT(u,u1,u2);
			VECTORNORMALISE(u);
		
			sousliste->Norm=mul*u;
		}

		f=g->figures->GetNext();
	}

	root=BuildNode(&Triangles,mul2);


	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}

	Triangles.Free();
}
#else
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L)
{
	CFaceGROUP *g=(CFaceGROUP*) vg;
	int *i,n;
	static CTriangle2 Triangles[512];
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	CFigure * f;
	static int array[512];
	int na;
	int a,b;
	int nt=0;
	float K=SMALLF;

	f=g->figures->GetFirst();
	while (f)
	{
		n=0;
		i=f->List.GetFirst();
		while (i)
		{
			array[n]=*i;
			n++;
			i=f->List.GetNext();
		}
		na=f->List.Length();

		for (n=0;n<na;n++)
		{
			a=array[n];
			b=array[(n+1)%na];

			C=obj->Vertices[a].Calc;
			D=obj->Vertices[b].Calc;
			
			B.x=K*L.x+(1-K)*C.x;
			B.y=K*L.y+(1-K)*C.y;
			B.z=K*L.z+(1-K)*C.z;

			A.x=K*L.x+(1-K)*D.x;
			A.y=K*L.y+(1-K)*D.y;
			A.z=K*L.z+(1-K)*D.z;

			CVector u1,u2;
			VECTORSUB(u1,C,L);
			VECTORSUB(u2,D,L);
			CROSSPRODUCT(u,u1,u2);
			VECTORNORMALISE(u);
		
			Triangles[nt].A=A;
			Triangles[nt].B=C;
			Triangles[nt].C=D;
			Triangles[nt].Norm=mul*u;
			Triangles[nt].tag=0;
			nt++;
		}

		f=g->figures->GetNext();
	}

	root=BuildNode2(Triangles,nt,mul2,0,-1);

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::Build(CObject3D *obj,float mul,float mul2)
{
	int n,nn;
	bool test;
	int tag;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri;
	CMatrix Mobj,Mrot;

	#define SMALLFXYZ SMALLF2
	#define SMALLFSC SMALLF3

	obj->Calculate();
	obj->SetF012();

	if (!grouped)
	{
		for (n=0;n<obj->nFaces;n++)
			if (VECTORNORM2(obj->Faces[n].Norm)<0.25f) obj->Faces[n].tag=-1;
	}

	tag=1;
	test=true;
	nn=0;
	
	while (test)
	{
		n=nn;
		nn=-1;

		if (!grouped)
		{
			while ((n<obj->nFaces)&&(nn==-1))
			{
				if (obj->Faces[n].tag==0) nn=n;
				else n++;
			}
		}
		else
		{
			while ((n<obj->nFaces)&&(nn==-1))
			{
				if (VECTORNORM2(obj->Faces[n].Norm)>=0.25f)
				{
					if (obj->Faces[n].tag==0) nn=n;
					else n++;
				}
				else n++;
			}
		}

		if (nn==-1) test=false;
		else
		{
			sousliste=NULL;

			if (grouped)
			{
				obj->TagSetHCoplanarFaces(nn,nn,tag);
				obj->Faces[nn].tag=tag;

				for (n=0;n<obj->nFaces;n++)
				{
					if (obj->Faces[n].tag==tag)
					{
						CVector u1,u2,u3;

						VECTORSUB(u1,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[0]->Calc);
						VECTORSUB(u2,obj->Faces[n].v[2]->Calc,obj->Faces[n].v[1]->Calc);
						VECTORSUB(u3,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[2]->Calc);

						float f1=VECTORNORM(u1);
						float f2=VECTORNORM(u2);
						float f3=VECTORNORM(u3);

						if ((f1>SMALLFXYZ)&&(f2>SMALLFXYZ)&&(f3>SMALLFXYZ))
						{
							VECTORDIV(u1,u1,f1);
							VECTORDIV(u2,u2,f2);
							VECTORDIV(u3,u3,f3);

							if ((DOT(u1,u2)<1.0f-SMALLFSC)&&(DOT(u2,u3)<1.0f-SMALLFSC)&&(DOT(u1,u3)<1.0f-SMALLFSC))
							{
								if (mul<0)
								{
									tri.A=obj->Faces[n].v[0]->Calc;
									tri.B=obj->Faces[n].v[1]->Calc;
									tri.C=obj->Faces[n].v[2]->Calc;
								}
								else
								{
									tri.A=obj->Faces[n].v[0]->Calc;
									tri.B=obj->Faces[n].v[2]->Calc;
									tri.C=obj->Faces[n].v[1]->Calc;
								}
								if (sousliste==NULL) sousliste=Triangles.InsertLast();
								sousliste->List.Add(tri);
							}
						}
							
					}
				}
			}
			else
			{
				obj->Faces[nn].tag=tag;

				n=nn;

				CVector u1,u2,u3;

				VECTORSUB(u1,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[0]->Calc);
				VECTORSUB(u2,obj->Faces[n].v[2]->Calc,obj->Faces[n].v[1]->Calc);
				VECTORSUB(u3,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[2]->Calc);

				float f1=VECTORNORM(u1);
				float f2=VECTORNORM(u2);
				float f3=VECTORNORM(u3);

				if ((f1>SMALLFXYZ)&&(f2>SMALLFXYZ)&&(f3>SMALLFXYZ))
				{
					VECTORDIV(u1,u1,f1);
					VECTORDIV(u2,u2,f2);
					VECTORDIV(u3,u3,f3);

					if ((DOT(u1,u2)<1.0f-SMALLFSC)&&(DOT(u2,u3)<1.0f-SMALLFSC)&&(DOT(u1,u3)<1.0f-SMALLFSC))
					{
						if (mul<0)
						{
							tri.A=obj->Faces[n].v[0]->Calc;
							tri.B=obj->Faces[n].v[1]->Calc;
							tri.C=obj->Faces[n].v[2]->Calc;
						}
						else
						{
							tri.A=obj->Faces[n].v[0]->Calc;
							tri.B=obj->Faces[n].v[2]->Calc;
							tri.C=obj->Faces[n].v[1]->Calc;
						}
						if (sousliste==NULL) sousliste=Triangles.InsertLast();
						sousliste->List.Add(tri);
					}
				}
			}
	
			if (sousliste) sousliste->Norm=mul*obj->Faces[nn].NormCalc;

			tag++;
		}
	}

	if (Triangles.Length()>0)
	{
		root=BuildNode(&Triangles,mul2);

		sousliste=Triangles.GetFirst();
		while (sousliste)
		{
			sousliste->List.Free();
			sousliste=Triangles.GetNext();
		}

		Triangles.Free();
	}
	else root=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DecoupeTRIANGLE(float mul,CTriangle * surf,float a,float b,float c,float d,CList <CTriangle> *plus,CList <CTriangle> *moins)
{
	#define SMALLFD SMALLF
	#define SMALLFS SMALLF2
	int res;
	float s,s1,s2,s3,ss;
	float ss1,ss2,ss3;
	float t1,t2;
	CTriangle *tri;
	CVector A,B,C,I,J;
	int cc;

	t1=1;
	t2=1;

	A=surf->A;
	B=surf->B;
	C=surf->C;

	s1=surf->A.x*a +surf->A.y*b +surf->A.z*c +d;
	s2=surf->B.x*a +surf->B.y*b +surf->B.z*c +d;
	s3=surf->C.x*a +surf->C.y*b +surf->C.z*c +d;

	cc=(s1>mul*SMALLFD)+(s2>mul*SMALLFD)+(s3>mul*SMALLFD);

	if ((cc==0)||(cc==3))
	{
		if (cc==0) moins->Add(*surf);
		else plus->Add(*surf);
	}
	else
	{
		ss1=0;
		t1=t2=-1;
		res=0;
		if (cc==1) s=1; else s=-1;

		if (s*s1>s*mul*SMALLFD)
		{
			res=1;
			A=surf->A;
			B=surf->B;
			C=surf->C;

			ss1=s1;
			ss2=s2;
			ss3=s3;

			ss=s1;
		}
		else
		if (s*s2>s*mul*SMALLFD)
		{
			A=surf->B;
			B=surf->C;
			C=surf->A;

			ss1=s2;
			ss2=s3;
			ss3=s1;

			res=1;

			ss=s2;
		}
		else
		if (s*s3>s*mul*SMALLFD)
		{
			res=1;
			A=surf->C;
			B=surf->A;
			C=surf->B;

			ss1=s3;
			ss2=s1;
			ss3=s2;
		}

		if (res)
		{
			t1=-ss1/(ss2-ss1);
			t2=-ss1/(ss3-ss1);
				
			if ((t1<SMALLFS)&&(t2<SMALLFS))
			{
				if (cc==2) plus->Add(*surf);
				else moins->Add(*surf);
			}
			else			
			if ((1.0f-t1<SMALLFS)&&(t2<SMALLFS))
			{
				if (cc==2) plus->Add(*surf);
				else moins->Add(*surf);
			}
			else
			if ((t1<SMALLFS)&&(1.0f-t2<SMALLFS))
			{
				if (cc==2) plus->Add(*surf);
				else moins->Add(*surf);
			}
			else
			if ((1.0f-t1<SMALLFS)&&(1.0f-t2<SMALLFS))
			{
				if (cc==1) plus->Add(*surf);
				else moins->Add(*surf);
			}
			else
			{
				if (t1<SMALLFS) I=A;
				else
				if (t1>1.0f-SMALLFS) I=B;
				else VECTORINTERPOL(I,t1,B,A);

				if (t2<SMALLFS) J=A;
				else
				if (t2>1.0f-SMALLFS) J=C;
				else VECTORINTERPOL(J,t2,C,A);

				if (cc==1) tri=plus->InsertLast();
				else tri=moins->InsertLast();
				tri->A=A;
				tri->B=I;
				tri->C=J;

				if (cc==2) tri=plus->InsertLast();
				else tri=moins->InsertLast();
				tri->A=I;
				tri->B=B;
				tri->C=C;

				if (cc==2) tri=plus->InsertLast();
				else tri=moins->InsertLast();
				tri->A=I;
				tri->B=C;
				tri->C=J;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPNode * CBSPVolumic::BuildNode(CList <CTriangles> *Triangles,float mul)
{
	CBSPNode *node;
	CList <CTriangles> l_plus,l_moins;
	CTriangles *plus,*moins,*first,*tmp;
	CList <CTriangle> tmpplus,tmpmoins;
	CTriangle *tri;

	node=NULL;
	
	if (Triangles->Length()>0)
	{
		node=new CBSPNode;

		first=Triangles->GetFirst();

		node->a=first->Norm.x;
		node->b=first->Norm.y;
		node->c=first->Norm.z;
		node->d=-DOT((first->List.GetFirst())->A,first->Norm);
		node->type=BSP_NODE;

		if (Triangles->Length()>1)
		{
			tmp=Triangles->GetNext();
			while (tmp)
			{
				plus=moins=NULL;

				tri=tmp->List.GetFirst();
				while (tri)
				{				
					DecoupeTRIANGLE(mul,tri,node->a,node->b,node->c,node->d,&tmpplus,&tmpmoins);
					tri=tmp->List.GetNext();
				}

				if (tmpmoins.Length()>0)
				{					
					moins=l_moins.InsertLast();
					moins->Norm=tmp->Norm;

					tri=tmpmoins.GetFirst();
					while (tri)
					{
						moins->List.Add(*tri);
						tri=tmpmoins.GetNext();
					}
				}

				if (tmpplus.Length()>0)
				{					
					plus=l_plus.InsertLast();
					plus->Norm=tmp->Norm;

					tri=tmpplus.GetFirst();
					while (tri)
					{
						plus->List.Add(*tri);
						tri=tmpplus.GetNext();
					}
				}
				
				tmpmoins.Free();
				tmpplus.Free();
				tmp=Triangles->GetNext();
			}

			if (l_moins.Length()>0)
			{
				node->moins=BuildNode(&l_moins,mul);
			}
			else
			{
				node->moins=new CBSPNode;
				node->moins->type=BSP_IN;
			}

			if (l_plus.Length()>0)
			{
				node->plus=BuildNode(&l_plus,mul);
			}
			else
			{
				node->plus=new CBSPNode;
				node->plus->type=BSP_OUT;
			}

			tmp=l_moins.GetFirst();
			while (tmp)
			{
				tmp->List.Free();
				tmp=l_moins.GetNext();
			}

			tmp=l_plus.GetFirst();
			while (tmp)
			{
				tmp->List.Free();
				tmp=l_plus.GetNext();
			}

			l_moins.Free();
			l_plus.Free();
		}
		else
		{
			node->plus=new CBSPNode;
			node->plus->type=BSP_OUT;
			node->moins=new CBSPNode;
			node->moins->type=BSP_IN;
		}
	}
	return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool maskequiv(int mask,int N,int value)
{
	bool res=false;
	int val=2*N+mask;

	if ((mask==-1)&&(value>=0)) res=true;
	else
	{
		if (value>=0)
		{
			if (value==val) res=true;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DecoupeTRIANGLE_array(float mul,CTriangle2 * surf,float a,float b,float c,float d,CTriangle2 *list,int* NT,int N,int *nm,int *np)
{
	int res;
	float s,s1,s2,s3,ss;
	float t1,t2;
	CTriangle2 *tri;
	CVector A,B,C,I,J;
	int cc;
	float mul2;
	t1=1;
	t2=1;

	mul2=mul;

	s1=surf->A.x*a +surf->A.y*b +surf->A.z*c +d;
	s2=surf->B.x*a +surf->B.y*b +surf->B.z*c +d;
	s3=surf->C.x*a +surf->C.y*b +surf->C.z*c +d;

	cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);

	if ((cc==0)||(cc==3))
	{
		if (cc==0)
		{
			(*nm)++;
			surf->tag=2*(N+1) + 0;
		}
		else
		{
			surf->tag=2*(N+1) + 1;
			(*np)++;
		}
			/**/
	}
	else
	{
		t1=t2=-1;
		res=0;
		if (cc==1) s=1; else s=-1;

		if (s*s1>s*mul2*SMALLF)
		{
			res=1;
			A=surf->A;
			B=surf->B;
			C=surf->C;

			if (f_abs((float) (s2-s1))>0)	t1=-s1/(s2-s1);
			if (f_abs((float) (s3-s1))>0)	t2=-s1/(s3-s1);
		}
		ss=s1;
		
		if (((s*s2>s*mul2*SMALLF)&&(res==0))||((s*s2>s*mul2*SMALLF)&&(res==1)&&(s*s2>s*s1)))
		{
			A=surf->B;
			B=surf->C;
			C=surf->A;
			if (f_abs((float) (s3-s2))>0)	t1=-s2/(s3-s2);
			if (f_abs((float) (s1-s2))>0)	t2=-s2/(s1-s2);
			res=1;
		}
		
		if (((s*s3>mul2*SMALLF)&&(res==0))||((s*s3>s*mul2*SMALLF)&&(res==1)&&(s*s3>s*ss)))
		{
			res=1;
			A=surf->C;
			B=surf->A;
			C=surf->B;
			if (f_abs((float) (s1-s3))>0)	t1=-s3/(s1-s3);
			if (f_abs((float) (s2-s3))>0)	t2=-s3/(s2-s3);
		}

		if ((t1<SMALLF3)&&(t2<SMALLF3))
		{
			if (cc==1)
			{
				surf->tag=2*(N+1) + 0;
				(*nm)++;
			}
			else
			{
				surf->tag=2*(N+1) + 1;
				(*np)++;		
			}
		}
		else
		if ((1-t1<SMALLF3)&&(1-t2<SMALLF3))
		{
			
			if (cc==1)
			{
				surf->tag=2*(N+1) + 1;
				(*np)++;
			}
			else
			{
				surf->tag=2*(N+1) + 0;
				(*nm)++;
			}
		}
		else
		{
			VECTORINTERPOL(I,t1,B,A);
			VECTORINTERPOL(J,t2,C,A);

			if (cc==1)
			{
				
				surf->A=A;
				surf->B=I;
				surf->C=J;
				surf->tag=2*(N+1)+1;
				(*np)++;

				tri=&list[*NT];
				tri->A=I;
				tri->B=B;
				tri->C=C;
				tri->Norm=surf->Norm;
				tri->tag=2*(N+1)+0;
				(*NT)++;
				
				tri=&list[*NT];
				tri->A=I;
				tri->B=C;
				tri->C=J;
				tri->Norm=surf->Norm;
				tri->tag=2*(N+1)+0;
				(*NT)++;
				
				(*nm)++;
				(*nm)++;

			}
			else
			{

				surf->A=A;
				surf->B=I;
				surf->C=J;
				surf->tag=2*(N+1)+0;

				(*nm)++;

				tri=&list[*NT];
				tri->A=I;
				tri->B=B;
				tri->C=C;
				tri->Norm=surf->Norm;
				tri->tag=2*(N+1)+1;
				(*NT)++;
				
				tri=&list[*NT];
				tri->A=I;
				tri->B=C;
				tri->C=J;
				tri->Norm=surf->Norm;
				tri->tag=2*(N+1)+1;
				(*NT)++;

				(*np)++;
				(*np)++;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPNode * CBSPVolumic::BuildNode2(CTriangle2 *Triangles,int nt,float mul,int N,int mask)
{
	CBSPNode *node;
	CTriangle2 *first,*tmp;
	int n,nlevel;
	float a,b,c,d;
	int np,nm;
	int NT=nt;

	node=NULL;
	first=NULL;

	nlevel=0;
	for (n=0;n<nt;n++)
	{	
		if (maskequiv(mask,N,Triangles[n].tag))
		{
			if (first==NULL) 
			{
				first=&Triangles[n];
				Triangles[n].tag=-1;
			}
			nlevel++;
		}
	}

	if (nlevel>0)
	{
		node=new CBSPNode;

		a=node->a=first->Norm.x;
		b=node->b=first->Norm.y;
		c=node->c=first->Norm.z;
		d=node->d=-DOT(first->A,first->Norm);
		node->type=BSP_NODE;

		np=nm=0;

		if (nlevel>1)
		{
			for (n=0;n<nt;n++)
			{
				if (maskequiv(mask,N,Triangles[n].tag))
				{
					tmp=&Triangles[n];
					DecoupeTRIANGLE_array(mul,tmp,a,b,c,d,Triangles,&NT,N,&nm,&np);
				}
			}

			if (nm>0) node->moins=BuildNode2(Triangles,NT,mul,N+1,0);
			else
			{
				node->moins=new CBSPNode;
				node->moins->type=BSP_IN;
			}

			if (np>0) node->plus=BuildNode2(Triangles,NT,mul,N+1,1);
			else
			{
				node->plus=new CBSPNode;
				node->plus->type=BSP_OUT;
			}

		}
		else
		{
			node->plus=new CBSPNode;
			node->plus->type=BSP_OUT;
			node->moins=new CBSPNode;
			node->moins->type=BSP_IN;
		}
	}


	return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumic::BuildObjectVolume(CObject3D *obj,float mul,float mul2)
{
	bool test=false;
	int n,nn,N,n0;
	CTriangle tri;
	CList <CTriangles> Triangles;
	CTriangles *sousliste;
	int tag;

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
	N=0;
	tag=1;
	test=true;
	while (test)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag==0) nn=n;
			else
				n++;
		}

		if (nn==-1) test=false;
		else
		{
			obj->TagCoplanarOnlyFaces(nn,tag);
			obj->Faces[nn].tag=tag;
			sousliste=Triangles.InsertLast();
			n0=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					tri.A=obj->Faces[n].v[0]->Stok;
					tri.B=obj->Faces[n].v[1]->Stok;
					tri.C=obj->Faces[n].v[2]->Stok;
					sousliste->List.Add(tri);
					n0++;
				}
			}
			sousliste->Norm=mul*obj->Faces[nn].Norm;
			tag++;
		}
	}

	root=BuildNode(&Triangles,mul2);

	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}
	Triangles.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OLD
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumicOld::Inside(CVector &p)
{
	CBSPNode * node;
	float s;

	node=root;

	if (node!=NULL)
	{
		while (node->type==BSP_NODE)
		{
			s=node->a*p.x+node->b*p.y+node->c*p.z+node->d;
			if (s>-SMALLF) node=node->plus;
			else node=node->moins;
		}

		if (node->type==BSP_IN) return true;
		else return false;
	}
	
	return false;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumicOld::Outside(CVector &p)
{
	CBSPNode * node;
	float s;

	node=root;

	if (node!=NULL)
	{
		while (node->type==BSP_NODE)
		{
			s=node->a*p.x+node->b*p.y+node->c*p.z+node->d;
			if (s>-SMALLF) node=node->plus;
			else node=node->moins;
		}

		if (node->type==BSP_IN) return false;
		else return true;
	}
	
	return false;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::Free()
{
	Plans.Free();
	if (root)
	{
		FreeNode(root);
		root=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::FreeNode(CBSPNode * node)
{
	if (node->type==BSP_NODE)
	{
		FreeNode(node->moins);
		FreeNode(node->plus);
	}
	delete node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::ListePlans(CBSPNode * node)
{
	CPlane P,*p;
	bool present;

	if (node)
	{
		if (node->type==BSP_NODE)
		{
			P.Init(node->a,node->b,node->c,node->d);
			P.Normalise();

			p=Plans.GetFirst();
			present=false;
			while ((p)&&(!present))
			{
				if (p->a*node->a+p->b*node->b+p->c*node->c>1.0f-SMALLF)
				{
					if (f_abs((node->d-p->d))<SMALLF) present=true;
				}

				if (p->a*node->a+p->b*node->b+p->c*node->c<-1.0f+SMALLF)
				{
					if (f_abs((node->d+p->d))<SMALLF) present=true;
				}
				p=Plans.GetNext();
			}

			if (!present)
				Plans.Add(P);


			ListePlans(node->plus);
			ListePlans(node->moins);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumicOld::FaceInside(CBSPNode *node,CFace *f,float mul)
{
	bool res=false;

	if (node)
	{
		if (node->type==BSP_NODE)
		{
			if (node==root)
			{
				_x=(f->v[0]->Calc.x+f->v[1]->Calc.x+f->v[2]->Calc.x)/3.0f;
				_y=(f->v[0]->Calc.y+f->v[1]->Calc.y+f->v[2]->Calc.y)/3.0f;
				_z=(f->v[0]->Calc.z+f->v[1]->Calc.z+f->v[2]->Calc.z)/3.0f;
			}
			float s=(float) (node->a*_x +node->b*_y +node->c*_z + node->d);

			if (s>mul*SMALLF) res=FaceInside(node->plus,f,mul);
			else res=FaceInside(node->moins,f,mul);
		}
		else
		{
			if (node->type==BSP_IN) res=true;
			else res=false;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumicOld::ShortFaceInside(CBSPNode *node,CShortFaceMap *f)
{
	float s1;//,s2,s3;
	//int cc;
	bool res;
	CVector u;

	res=false;
	if (node)
	{
		if (node->type==BSP_NODE)
		{
			u=(f->s0->Calc+f->s1->Calc+f->s2->Calc)/3;
			s1=(float) (node->a*u.x +node->b*u.y +node->c*u.z + node->d);

			if (s1>0) res=ShortFaceInside(node->plus,f);
			else res=ShortFaceInside(node->moins,f);
		}
		else
		{
			if (node->type==BSP_IN) res=true;
			else res=false;
		}
	}

	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CBSPVolumicOld::tagBoolean(CObject3D * obj,CObject3D * ref,float mul)
{
    int n;
    
    obj->Calculate();
    ref->Calculate();
    
    /*
    for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;
    
    for (i=0;i<obj->nFaces;i++)
    {
        int face=0;
        for (j=0;j<ref->nFaces;j++)
        {
			if (CalcFacesCollisionTag(&obj->Faces[i],&ref->Faces[j],1.5f)) { ref->Faces[j].tag=0; face=1; }
			else
            if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
            else
                if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
        }
        if (face==0) obj->Faces[i].tag=-1;
        else obj->Faces[i].tag=0;
    }
        
    int cnt=0;
    for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
    
    if (cnt==0) return false;
    /*
    if (ref->nFaces<2048)
    {
        for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
        for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
    }
    /**/

    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
    for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;

    CObject3D *ref0=ref->OptimizeMeshBoolean();
    Build(ref0,mul,VALUEBUILD);
    ref0->Free();
    delete ref0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==0)
        {
            int cc=0;
            if (Inside(obj->Faces[n].v[0]->Calc)) cc++;
            if (Inside(obj->Faces[n].v[1]->Calc)) cc++;
            if (Inside(obj->Faces[n].v[2]->Calc)) cc++;
            //if (FaceInside(root,&obj->Faces[n],1)) obj->Faces[n].tag=0;
            //else obj->Faces[n].tag=-1;
            if (cc==0) obj->Faces[n].tag=-1;
            else obj->Faces[n].tag=0;
        }
    }
    
    if (root) FreeNode(root);
    root=NULL;
    /**/
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumicOld::NumberOfPlanes(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2)
{
    CObject3D *obj,*ref;
    int n,i,j;

    obj=obj0->Duplicate2();
//    obj=obj0->OptimizeMeshBoolean();

    if (ref0->Double1) ref=ref0->Double1->OptimizeMeshBoolean();
    else ref=ref0->OptimizeMeshBoolean();

    obj->CalculateNormals(-1);
    ref->CalculateNormals(-1);

    obj->Calculate();
    ref->Calculate();

    for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

    for (i=0;i<obj->nFaces;i++)
    {
        int face=0;
        for (j=0;j<ref->nFaces;j++)
        {
            if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
            else
            if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
        }
        if (face==0) obj->Faces[i].tag=-1;
        else obj->Faces[i].tag=0;
    }

    int cnt=0;
    for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
    
    if (cnt==0)
    {
        obj->Free(); delete obj;
        ref->Free(); delete ref;
        return 0;
    }

    Build(ref,mul,VALUEBUILD);
    
    ListePlans(root);
    int NB=Plans.Length();
    
    Free();
    
    if (root) FreeNode(root);
    root=NULL;

    obj->Free(); delete obj;
    ref->Free(); delete ref;

    return NB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CObject3D * CBSPVolumicOld::Boolean(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,bool TAG)
{
	CObject3D *res,*obj,*ref;
	int n,nn,nv,nf,i,j;

	obj=obj0->Duplicate2();
//	obj=obj0->OptimizeMeshBoolean();

	if (ref0->Double1) ref=ref0->Double1->OptimizeMeshBoolean();
	else ref=ref0->OptimizeMeshBoolean();

	obj->CalculateNormals(-1);
	ref->CalculateNormals(-1);

	obj->Calculate();
	ref->Calculate();

	ref->SetF012Near();

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

	for (i=0;i<obj->nFaces;i++)
	{		
		int face=0;
		for (j=0;j<ref->nFaces;j++)
		{
			if (!TAG)
			{
				if (CalcFacesCollisionTag(&obj->Faces[i],&ref->Faces[j],2.0f)) { ref->Faces[j].tag=0; face=1; }
			}
			else
			{
				if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
				else
				if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
			}
		}
		if (face==0) obj->Faces[i].tag=-1;
		else obj->Faces[i].tag=0;
	}
	
	if (!TAG)
	{
		for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag2=0;

		for (n=0;n<ref->nFaces;n++)
		{
			int ok=0;
			if (ref->Faces[n].tag<0)
			{
				if (ref->Faces[n].f01>=0) ok+=(ref->Faces[ref->Faces[n].f01].tag==0);
				if (ref->Faces[n].f12>=0) ok+=(ref->Faces[ref->Faces[n].f12].tag==0);
				if (ref->Faces[n].f20>=0) ok+=(ref->Faces[ref->Faces[n].f20].tag==0);
				if (ok>=2) ref->Faces[n].tag2=1;
			}
		}

		for (n=0;n<ref->nFaces;n++)
			if (ref->Faces[n].tag2) ref->Faces[n].tag=0;
	}

	int cnt=0;
	for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
	
	if (cnt==0)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		return NULL;
	}

	_max_nb_vertices=NB_MAX_VERTICES;
	_max_nb_faces=NB_MAX_FACES;

	VerticesS=new CShortVertexMap[_max_nb_vertices];
	faces=new CShortFaceMap[_max_nb_faces];

	for (n=0;n<obj->nVertices;n++)
	{
		VerticesS[n].Stok=obj->Vertices[n].Stok;
		VerticesS[n].Calc=obj->Vertices[n].Calc;
        VerticesS[n].Map=obj->Vertices[n].Map;
        VerticesS[n].ndx=n;
	}

	int nbout=0;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			faces[nn].tag=0;
			faces[nn].nT=obj->Faces[n].nT;
			faces[nn].nT2=obj->Faces[n].nT2;
			faces[nn].nL=obj->Faces[n].nL;
			faces[nn].ref=obj->Faces[n].ref;
			faces[nn].n0=obj->Faces[n].v0;
			faces[nn].n1=obj->Faces[n].v1;
			faces[nn].n2=obj->Faces[n].v2;
			faces[nn].s0=&(VerticesS[faces[nn].n0]);
			faces[nn].s1=&(VerticesS[faces[nn].n1]);
			faces[nn].s2=&(VerticesS[faces[nn].n2]);
			nn++;
		}
		else nbout++;
	}

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag2=ref->Faces[n].tag;

	grouped=true;

	Build(ref,mul,VALUEBUILD);
	
	nVertices=obj->nVertices;
	nfaces=nn;

	if (full)
	{
		ListePlans(root);
		nfaces=CutFacesMap(0,faces,nfaces,mul2);
		Plans.Free();
	}
	else InOut(0,0,root,mul2);

	res=new CObject3D;
	res->Init(nVertices,nfaces+obj->nFaces);
	
	nv=0;
	for (n=0;n<nVertices;n++)
	{
		res->Vertices[nv].Stok=VerticesS[n].Stok;
		res->Vertices[nv].Calc=VerticesS[n].Calc;
        res->Vertices[nv].Map=VerticesS[n].Map;
        for (int i=0;i<4;i++)
        {
            res->Vertices[nv].Index[i]=obj->Vertices[VerticesS[n].ndx].Index[i];
            res->Vertices[nv].Weight[i]=obj->Vertices[VerticesS[n].ndx].Weight[i];
        }
		nv++;
	}	
	/*
	Free();
	//for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
	
	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag3=0;

	for (n=0;n<ref->nFaces;n++)
		if (ref->Faces[n].tag2==0)
		{
			ref->Faces[n].tag3=1;
			if (ref->Faces[n].f01>=0) ref->Faces[ref->Faces[n].f01].tag3=1;
			if (ref->Faces[n].f12>=0) ref->Faces[ref->Faces[n].f12].tag3=1;
			if (ref->Faces[n].f20>=0) ref->Faces[ref->Faces[n].f20].tag3=1;
		}

	for (n=0;n<ref->nFaces;n++)
		if (ref->Faces[n].tag3) ref->Faces[n].tag=0; else ref->Faces[n].tag=-1;

	Build(ref,mul,VALUEBUILD);
	/**/
	
	nf=0;
	for (n=0;n<nfaces;n++)
	{
		res->Faces[nf].Init(faces[n].n0,faces[n].n1,faces[n].n2);
		res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
		res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
		res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
		res->Faces[nf].nT=faces[n].nT;
		res->Faces[nf].nT2=faces[n].nT2;
		res->Faces[nf].nL=faces[n].nL;
		res->Faces[nf].ref=faces[n].ref;
		res->Faces[nf].tag=0;
		bool inc=false;
		if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		if (inc) nf++;
	}

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==-1)
		{
			res->Faces[nf].v0=obj->Faces[n].v0;
			res->Faces[nf].v1=obj->Faces[n].v1;
			res->Faces[nf].v2=obj->Faces[n].v2;
			res->Faces[nf].nT=obj->Faces[n].nT;
			res->Faces[nf].nT2=obj->Faces[n].nT2;
			res->Faces[nf].nL=obj->Faces[n].nL;
			res->Faces[nf].tag=0;
			res->Faces[nf].ref=obj->Faces[n].ref;
			res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
			res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
			res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
			bool inc=false;
			if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			if (inc) nf++;
		}
	}

	if (nf==0)
	{
		if (root) FreeNode(root);
		root=NULL;
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		res->Free();
		delete res;
		delete [] faces;
		delete [] VerticesS;
		return NULL;
	}

	res->nFaces=nf;
	res->SetFaces();
	res->CalculateNormals(-1);

	res->Rot=obj->Rot;
	res->Coo=obj->Coo;
	for (n=0;n<6;n++) res->P[n]=obj->P[n];
	res->nP=obj->nP;
	for (n=0;n<16;n++) res->Tab[n]=obj->Tab[n];
	res->Status=obj->Status;
	res->Attribut=obj->Attribut;
	res->env_mapping=obj->env_mapping;
	res->Tag=obj->Tag;
	
	if (root) FreeNode(root);
	root=NULL;
	delete [] faces;
	delete [] VerticesS;

	obj->Free(); delete obj;
	ref->Free(); delete ref;

	obj=res->Duplicate2();
	res->Free();
	delete res;

	return obj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::Boolean2Parts(CObject3D * obj0,CObject3D * ref0,int tag,float mul,float mul2,CObject3D **pobj1,CObject3D **pobj2)
{
	CObject3D *res,*res2,*obj,*ref;
	int n,nn,nv,nf,nf2,i,j;

	*pobj1=NULL;
	*pobj2=NULL;

	obj=obj0->Duplicate2();
	if (ref0->Double1) ref=ref0->Double1->Duplicate2();
	else ref=ref0->Duplicate2();

	obj->CalculateNormals(-1);
	ref->CalculateNormals(-1);

	obj->Calculate();
	ref->Calculate();

	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=-1;

	for (i=0;i<obj->nFaces;i++)
	{
		int face=0;
		for (j=0;j<ref->nFaces;j++)	
		{
			if (CalcFacesCollision(&obj->Faces[i],&ref->Faces[j])) { ref->Faces[j].tag=0; face=1; }
			else
			if (CalcFacesCollision(&ref->Faces[j],&obj->Faces[i])) { ref->Faces[j].tag=0; face=1; }
		}

		if (face==0) obj->Faces[i].tag=-1;
		else obj->Faces[i].tag=0;
	}

	int cnt=0;
	for (n=0;n<ref->nFaces;n++) if (ref->Faces[n].tag==0) cnt++;
	
	if (cnt==0)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		*pobj1=NULL;
		*pobj2=NULL;
		return;	
	}

	_max_nb_vertices=NB_MAX_VERTICES;
	_max_nb_faces=NB_MAX_FACES;

	VerticesS=new CShortVertexMap[_max_nb_vertices];
	faces=new CShortFaceMap[_max_nb_faces];

	for (n=0;n<obj->nVertices;n++)
	{
		VerticesS[n].Stok=obj->Vertices[n].Stok;
		VerticesS[n].Calc=obj->Vertices[n].Calc;
		VerticesS[n].Map=obj->Vertices[n].Map;
        VerticesS[n].ndx=n;
		VerticesS[n].tag=1;
	}

	int nbout=0;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==0)
		{
			faces[nn].tag=0;
			faces[nn].nT=obj->Faces[n].nT;
			faces[nn].nT2=obj->Faces[n].nT2;
			faces[nn].nL=obj->Faces[n].nL;
			faces[nn].ref=obj->Faces[n].ref;
			faces[nn].n0=obj->Faces[n].v0;
			faces[nn].n1=obj->Faces[n].v1;
			faces[nn].n2=obj->Faces[n].v2;
			faces[nn].s0=&(VerticesS[faces[nn].n0]);
			faces[nn].s1=&(VerticesS[faces[nn].n1]);
			faces[nn].s2=&(VerticesS[faces[nn].n2]);
			nn++;
		}
		else nbout++;
	}

	Build(ref,mul,VALUEBUILD);

	if (root==NULL)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;

		delete [] faces;
		delete [] VerticesS;	
		return;
	}
	
	nVertices=obj->nVertices;
	nfaces=nn;
	if (full)
	{
		ListePlans(root);
		nfaces=CutFacesMap(0,faces,nfaces,mul2);
		Plans.Free();
	}
	else InOut(0,0,root,mul2);

	res=new CObject3D;
	res->Init(nVertices,nfaces+obj->nFaces);

	res2=new CObject3D;
	res2->Init(nVertices,nfaces+obj->nFaces);

	nv=0;
	for (n=0;n<nVertices;n++)
	{
		res->Vertices[nv].Stok=VerticesS[n].Stok;
		res->Vertices[nv].Calc=VerticesS[n].Calc;
		res->Vertices[nv].Map=VerticesS[n].Map;
		if (nv<obj->nVertices) res->Vertices[nv].tag2=0; else res->Vertices[nv].tag2=1;

		res2->Vertices[nv].Stok=VerticesS[n].Stok;
		res2->Vertices[nv].Calc=VerticesS[n].Calc;
		res2->Vertices[nv].Map=VerticesS[n].Map;
		if (nv<obj->nVertices) res2->Vertices[nv].tag2=0; else res2->Vertices[nv].tag2=1;

        for (int i=0;i<4;i++)
        {
            res->Vertices[nv].Index[i]=obj->Vertices[VerticesS[n].ndx].Index[i];
            res->Vertices[nv].Weight[i]=obj->Vertices[VerticesS[n].ndx].Weight[i];

            res2->Vertices[nv].Index[i]=obj->Vertices[VerticesS[n].ndx].Index[i];
            res2->Vertices[nv].Weight[i]=obj->Vertices[VerticesS[n].ndx].Weight[i];
        }

		nv++;
	}
	
	Free();
	for (n=0;n<ref->nFaces;n++) ref->Faces[n].tag=0;
	Build(ref,mul,VALUEBUILD);

	if (root==NULL)
	{
		obj->Free(); delete obj;
		ref->Free(); delete ref;

		delete [] faces;
		delete [] VerticesS;	
		return;
	}
	
	nf=0;
	nf2=0;

	for (n=0;n<nfaces;n++)
	{
		res->Faces[nf].Init(faces[n].n0,faces[n].n1,faces[n].n2);
		res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
		res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
		res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
		res->Faces[nf].nT=faces[n].nT;
		res->Faces[nf].nT2=faces[n].nT2;
		res->Faces[nf].nL=faces[n].nL;
		res->Faces[nf].ref=faces[n].ref;
		res->Faces[nf].tag=0;		

		bool inc=false;
		if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
		if (inc) nf++;
		else
		{
			res2->Faces[nf2].Init(faces[n].n0,faces[n].n1,faces[n].n2);
			res2->Faces[nf2].v[0]=&res2->Vertices[res2->Faces[nf2].v0];
			res2->Faces[nf2].v[1]=&res2->Vertices[res2->Faces[nf2].v1];
			res2->Faces[nf2].v[2]=&res2->Vertices[res2->Faces[nf2].v2];
			res2->Faces[nf2].nT=faces[n].nT;
			res2->Faces[nf2].nT2=faces[n].nT2;
			res2->Faces[nf2].nL=faces[n].nL;
			res2->Faces[nf2].ref=faces[n].ref;
			res2->Faces[nf2].tag=0;
			nf2++;
		}
	}

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==-1)
		{
			res->Faces[nf].v0=obj->Faces[n].v0;
			res->Faces[nf].v1=obj->Faces[n].v1;
			res->Faces[nf].v2=obj->Faces[n].v2;
			res->Faces[nf].nT=obj->Faces[n].nT;
			res->Faces[nf].nT2=obj->Faces[n].nT2;
			res->Faces[nf].nL=obj->Faces[n].nL;
			res->Faces[nf].tag=0;
			res->Faces[nf].ref=obj->Faces[n].ref;
			res->Faces[nf].v[0]=&res->Vertices[res->Faces[nf].v0];
			res->Faces[nf].v[1]=&res->Vertices[res->Faces[nf].v1];
			res->Faces[nf].v[2]=&res->Vertices[res->Faces[nf].v2];
			bool inc=false;
			if (tag==BOOLEAN_IN) { if (FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			else { if (!FaceInside(root,&res->Faces[nf],mul2)) inc=true; }
			if (inc) nf++;
			else
			{
				res2->Faces[nf2].v0=obj->Faces[n].v0;
				res2->Faces[nf2].v1=obj->Faces[n].v1;
				res2->Faces[nf2].v2=obj->Faces[n].v2;
				res2->Faces[nf2].nT=obj->Faces[n].nT;
				res2->Faces[nf2].nT2=obj->Faces[n].nT2;
				res2->Faces[nf2].nL=obj->Faces[n].nL;
				res2->Faces[nf2].tag=0;
				res2->Faces[nf2].ref=obj->Faces[n].ref;
				res2->Faces[nf2].v[0]=&res2->Vertices[res2->Faces[nf2].v0];
				res2->Faces[nf2].v[1]=&res2->Vertices[res2->Faces[nf2].v1];
				res2->Faces[nf2].v[2]=&res2->Vertices[res2->Faces[nf2].v2];

				nf2++;
			}
		}
	}

	if (nf==0)
	{
		if (root) FreeNode(root);
		root=NULL;
		obj->Free(); delete obj;
		ref->Free(); delete ref;
		res->Free();
		delete res;
		res2->Free();
		delete res2;
		delete [] faces;
		delete [] VerticesS;	
		return;	
	}

	res->nFaces=nf;
	res->SetFaces();
	res->CalculateNormals(-1);
	res->Rot=obj->Rot;
	res->Coo=obj->Coo;
	for (n=0;n<6;n++) res->P[n]=obj->P[n];
	res->nP=obj->nP;
	for (n=0;n<16;n++) res->Tab[n]=obj->Tab[n];
	res->Status=obj->Status;
	res->Attribut=obj->Attribut;
	res->env_mapping=obj->env_mapping;
	res->Tag=obj->Tag;

	res2->nFaces=nf2;
	res2->SetFaces();
	res2->CalculateNormals(-1);
	res2->Rot=obj->Rot;
	res2->Coo=obj->Coo;
	for (n=0;n<6;n++) res2->P[n]=obj->P[n];
	res2->nP=obj->nP;
	for (n=0;n<16;n++) res2->Tab[n]=obj->Tab[n];
	res2->Status=obj->Status;
	res2->Attribut=obj->Attribut;
	res2->env_mapping=obj->env_mapping;
	res2->Tag=obj->Tag;

	*pobj1=res->Duplicate2();
	*pobj2=res2->Duplicate2();

	res->Free(); delete res;
	res2->Free(); delete res2;

	if (root) FreeNode(root);
	root=NULL;
	delete [] faces;
	delete [] VerticesS;

	obj->Free(); delete obj;
	ref->Free(); delete ref;

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add CShortFaceMap *faces,int NF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::InOutNoMap(int mask,int N,CBSPNode * node,float mul)
{
	int res;
	int nm,np;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc,cc2;
	int n;
	float mul0=mul;
	float mul1=mul;

	nm=np=0;
	if (node)
	{
		if (node->type==BSP_NODE)
		{

			for (n=0;n<nVertices;n++) VerticesS[n].coef1=(float) (VerticesS[n].Calc.x*node->a +VerticesS[n].Calc.y*node->b +VerticesS[n].Calc.z*node->c + node->d);

			for (n=0;n<nfaces;n++)
			{
				f=&faces[n];

				cc2=cc=f->tag;
				cc2=cc2&1;
				cc=cc>>1;

				if ((cc==N)&&(cc2==mask))
				{
					P1=f->s0;
					P2=f->s1;
					P3=f->s2;
					s1=P1->coef1;
					s2=P2->coef1;
					s3=P3->coef1;

					cc=(s1>mul1*SMALLF)+(s2>mul1*SMALLF)+(s3>mul1*SMALLF);

					if (cc==0) {f->tag=(N+1)*2 + 0;nm++;}
					if (cc==3) {f->tag=(N+1)*2 + 1;np++;}

					if ((cc==1)||(cc==2))
					{
						if (cc==1) s=1; else s=-1;

						t1=t2=-1;

						res=0;

						if (s*s1>s*mul0*SMALLF)
						{
							A=f->n0;B=f->n1;C=f->n2;
							PA=P1;
							PB=P2;
							PC=P3;
							ss1=s1;ss2=s2;ss3=s3;
							res=1;
						}
						ss=s1;

						if (((s*s2>s*mul0*SMALLF)&&(res==0))||((s*s2>s*mul0*SMALLF)&&(res==1)&&(s*s2>s*s1)))
						{
							ss=s2;
							A=f->n1;B=f->n2;C=f->n0;
							ss1=s2;ss2=s3;ss3=s1;
							res=1;
							PA=P2;
							PB=P3;
							PC=P1;
						}
						
						if (((s*s3>s*mul0*SMALLF)&&(res==0))||((s*s3>s*mul0*SMALLF)&&(res==1)&&(s*s3>s*ss)))
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


						if ((t1<SMALLF3)&&(t2<SMALLF3))
						{
							if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
							else {f->tag=(N+1)*2 + 1;np++;}
						}
						else
						if ((1-t1<SMALLF3)&&(1-t2<SMALLF3))
						{
							if (cc==1) { f->tag=(N+1)*2 + 1;np++;}
							else {f->tag=(N+1)*2 + 0;nm++;}
						}
						else
						{

							t=-ss1/(ss2-ss1);
							sAB=&VerticesS[nVertices];
							VECTORINTERPOL(sAB->Stok,t,PB->Stok,PA->Stok);
							VECTORINTERPOL(sAB->Calc,t,PB->Calc,PA->Calc);
							sAB->tag=0;
							iAB=nVertices;
							nVertices++;

							t=-ss1/(ss3-ss1);
							sAC=&VerticesS[nVertices];
							VECTORINTERPOL(sAC->Stok,t,PC->Stok,PA->Stok);
							VECTORINTERPOL(sAC->Calc,t,PC->Calc,PA->Calc);
							sAC->tag=0;
							iAC=nVertices;
							nVertices++;

							tmp.nL=f->nL;
							tmp.nT=f->nT;
							tmp.nT2=f->nT2;
							tmp.tag=f->tag;
							tmp.ref=f->ref;

							tmp.n0=A;
							tmp.n1=iAB;
							tmp.n2=iAC;
							tmp.s0=PA;
							tmp.s1=sAB;
							tmp.s2=sAC;
							tmp.lock=1;
							faces[n]=tmp;


							tmp.n0=iAC;
							tmp.n1=iAB;
							tmp.n2=B;
							tmp.s0=sAC;
							tmp.s1=sAB;
							tmp.s2=PB;
							tmp.lock=1;

							faces[nfaces]=tmp;


							tmp.n0=iAC;
							tmp.n1=B;
							tmp.n2=C;
							tmp.s0=sAC;
							tmp.s1=PB;
							tmp.s2=PC;
							tmp.lock=1;
							faces[nfaces+1]=tmp;

							if (cc==1)
							{
								nm+=2;
								np+=1;
								faces[n].tag=2*(N+1) + 1;
								faces[nfaces].tag=2*(N+1) + 0;
								faces[nfaces+1].tag=2*(N+1) + 0;
							}
							else
							{
								nm+=1;
								np+=2;
								faces[n].tag=2*(N+1) + 0;
								faces[nfaces].tag=2*(N+1) + 1;
								faces[nfaces+1].tag=2*(N+1) + 1;
							}
							nfaces+=2;
						}
					}
				}
			}

			if (np>0) InOutNoMap(1,N+1,node->plus,mul);
			if (nm>0) InOutNoMap(0,N+1,node->moins,mul);
		}
		else
		{
			if (node->type==BSP_IN)
			{
				for (n=0;n<nfaces;n++)
				{
					f=&faces[n];
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;

					if ((cc==N)&&(cc2==mask)) f->tag=-1;
				}
			}
			else
			{
				for (n=0;n<nfaces;n++)
				{
					f=&faces[n];
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;

					if ((cc==N)&&(cc2==mask)) f->tag=-2;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::InOut(int mask,int N,CBSPNode * node,float mul)
{
	#define SMALLFW SMALLF
	int res;
	int nm,np;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc,cc2;
	int n;

	nm=np=0;
	if (node)
	{
		if (node->type==BSP_NODE)
		{
			for (n=0;n<nVertices;n++) VerticesS[n].tag=0;

			for (n=0;n<nfaces;n++)
			{
				f=&faces[n];

				cc2=cc=f->tag;
				cc2=cc2&1;
				cc=cc>>1;

				if ((cc==N)&&(cc2==mask))
				{
					P1=f->s0;
					P2=f->s1;
					P3=f->s2;

					if (P1->tag) s1=P1->coef1; else { P1->tag=1; s1=P1->coef1=(float) (P1->Calc.x*node->a + P1->Calc.y*node->b + P1->Calc.z*node->c + node->d); }
					if (P2->tag) s2=P2->coef1; else { P2->tag=1; s2=P2->coef1=(float) (P2->Calc.x*node->a + P2->Calc.y*node->b + P2->Calc.z*node->c + node->d); }
					if (P3->tag) s3=P3->coef1; else { P3->tag=1; s3=P3->coef1=(float) (P3->Calc.x*node->a + P3->Calc.y*node->b + P3->Calc.z*node->c + node->d); }

					cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);
				
					if (cc==0) {f->tag=(N+1)*2 + 0;nm++;}
					if (cc==3) {f->tag=(N+1)*2 + 1;np++;}
				
					if ((cc==1)||(cc==2))
					{
						if (cc==1) s=1; else s=-1;
						t1=t2=-1;

						ss1=0;

						res=0;
						if (s*s1>=s*mul*SMALLF)
						{
							A=f->n0;B=f->n1;C=f->n2;
							PA=P1;
							PB=P2;
							PC=P3;
							ss1=s1;ss2=s2;ss3=s3;
							ss=s1;
							res=1;
						}
						else
						if (s*s2>=s*mul*SMALLF)
						{
							ss=s2;
							A=f->n1;B=f->n2;C=f->n0;
							ss1=s2;ss2=s3;ss3=s1;
							res=1;
							PA=P2;
							PB=P3;
							PC=P1;
						}
						else
						if (s*s3>=s*mul*SMALLF)
						{
							A=f->n2;B=f->n0;C=f->n1;
							ss1=s3;ss2=s1;ss3=s2;
							res=1;
							PA=P3;
							PB=P1;
							PC=P2;
						}

						if (res==1)
						{
							t1=-ss1/(ss2-ss1);
							t2=-ss1/(ss3-ss1);

							if ((t1<SMALLFW)&&(t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((t1<SMALLFW)&&(1.0f-t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((1.0f-t1<SMALLFW)&&(t2<SMALLFW))
							{
								if (cc==1) {f->tag=(N+1)*2 + 0;nm++;}
								else {f->tag=(N+1)*2 + 1;np++;}
							}
							else
							if ((1.0f-t1<SMALLFW)&&(1.0f-t2<SMALLFW))
							{
								if (cc==1) { f->tag=(N+1)*2 + 1;np++;}
								else {f->tag=(N+1)*2 + 0;nm++;}
							}
							else
							{
								t=t1;
								if (t<SMALLFW)
								{
									sAB=PA;
									iAB=A;
								}
								else
								if (t>1.0f-SMALLFW)
								{
									sAB=PB;
									iAB=B;
								}
								else
								{
									sAB=&VerticesS[nVertices];
									VECTORINTERPOL(sAB->Stok,t,PB->Stok,PA->Stok);
									VECTORINTERPOL(sAB->Calc,t,PB->Calc,PA->Calc);
									VECTOR2INTERPOL(sAB->Map,t,PB->Map,PA->Map);
                                    if (t>0.5f) sAB->ndx=PA->ndx; else sAB->ndx=PB->ndx;

									sAB->tag=0;
									iAB=nVertices;
									nVertices++;
								}

								t=t2;
								if (t<SMALLFW)
								{
									sAC=PA;
									iAC=A;
								}
								else
								if (t>1.0f-SMALLFW)
								{
									sAC=PC;
									iAC=C;
								}
								else
								{
									sAC=&VerticesS[nVertices];
									VECTORINTERPOL(sAC->Stok,t,PC->Stok,PA->Stok);
									VECTORINTERPOL(sAC->Calc,t,PC->Calc,PA->Calc);
									VECTOR2INTERPOL(sAC->Map,t,PC->Map,PA->Map);
                                    if (t>0.5f) sAC->ndx=PA->ndx; else sAC->ndx=PC->ndx;

									sAC->tag=0;
									iAC=nVertices;
									nVertices++;
								}

								tmp.nL=f->nL;
								tmp.nT=f->nT;
								tmp.nT2=f->nT2;
								tmp.tag=f->tag;
								tmp.ref=f->ref;

								tmp.n0=A;
								tmp.n1=iAB;
								tmp.n2=iAC;
								tmp.s0=PA;
								tmp.s1=sAB;
								tmp.s2=sAC;
								tmp.lock=1;
								faces[n]=tmp;
							
								tmp.n0=iAC;
								tmp.n1=iAB;
								tmp.n2=B;
								tmp.s0=sAC;
								tmp.s1=sAB;
								tmp.s2=PB;
								tmp.lock=1;
								faces[nfaces]=tmp;

								tmp.n0=iAC;
								tmp.n1=B;
								tmp.n2=C;
								tmp.s0=sAC;
								tmp.s1=PB;
								tmp.s2=PC;
								tmp.lock=1;
								faces[nfaces+1]=tmp;
				
								if (cc==1)
								{
									nm+=2;
									np+=1;
									faces[n].tag=2*(N+1) + 1;
									faces[nfaces].tag=2*(N+1) + 0;
									faces[nfaces+1].tag=2*(N+1) + 0;
								}
								else
								{
									nm+=1;
									np+=2;
									faces[n].tag=2*(N+1) + 0;
									faces[nfaces].tag=2*(N+1) + 1;
									faces[nfaces+1].tag=2*(N+1) + 1;
								}
								nfaces+=2;
							}
						}
					}
				}
			}

			if (np>0) InOut(1,N+1,node->plus,mul);
			if (nm>0) InOut(0,N+1,node->moins,mul);
		}
		else
		{
			if (node->type==BSP_IN)
			{
				for (n=0;n<nfaces;n++)
				{
					f=&faces[n];
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;
					if ((cc==N)&&(cc2==mask)) f->tag=-1;
				}
			}
			else
			{
				for (n=0;n<nfaces;n++)
				{
					f=&faces[n];
					cc2=cc=f->tag;
					cc2=cc2&1;
					cc=cc>>1;
					if ((cc==N)&&(cc2==mask)) f->tag=-2;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumicOld::CutFaces(int nP,CShortFaceMap *faces,int NF,float mul)
{
	int res;
	int NBRE;
	int nx;
	//CList <CShortFaceMap> decoupees;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	CPlane *p;
	int A,B,C;
//	int nAB,nAC;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int n;
	int nb,LEN;
	int nf;
	float mul0;

	mul0=-1;


	nf=NF;
	LEN=NF;

	nx=0;
	if (nP<Plans.Length())
	{
		p=Plans[nP];

		for (n=0;n<nVertices;n++)
			VerticesS[n].coef1=(float) (VerticesS[n].Calc.x*p->a +VerticesS[n].Calc.y*p->b +VerticesS[n].Calc.z*p->c + p->d);

		for (nb=0;nb<LEN;nb++)
		{
			f=&faces[nb];
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->coef1;
			s2=P2->coef1;
			s3=P3->coef1;
			//s1=(float) (P1->Calc.x*p->a +P1->Calc.y*p->b +P1->Calc.z*p->c + p->d);
			//s2=(float) (P2->Calc.x*p->a +P2->Calc.y*p->b +P2->Calc.z*p->c + p->d);
			//s3=(float) (P3->Calc.x*p->a +P3->Calc.y*p->b +P3->Calc.z*p->c + p->d);

			cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;

				if (s*s1>s*mul0*SMALLF)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					res=1;
				}
				ss=s1;
				
				if (((s*s2>s*mul0*SMALLF)&&(res==0))||((s*s2>s*mul0*SMALLF)&&(res==1)&&(s*s2>s*s1)))
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;

				}
				
				if (((s*s3>s*mul0*SMALLF)&&(res==0))||((s*s3>s*mul0*SMALLF)&&(res==1)&&(s*s3>s*ss)))
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
					// ERREUR
				}

				if ((t1<SMALLF3)&&(t2<SMALLF3))
				{
					//decoupees.Add(*f);
				}
				else
				if ((1-t1<SMALLF3)&&(1-t2<SMALLF3))
				{
					//decoupees.Add(*f);
				}
				else
				{
				//	t=-ss1/(ss2-ss1);
					t=t1;
					if (t<SMALLF2)
					{
						sAB=PA;
						iAB=A;
					}
					else
					if (t>1-SMALLF2)
					{
						sAB=PB;
						iAB=B;
					}
					else
					{
						sAB=&VerticesS[nVertices];
						sAB->Stok=(1-t)*(PA->Stok) + t*(PB->Stok);
						sAB->Calc=(1-t)*(PA->Calc) + t*(PB->Calc);
						sAB->tag=0;
						
						iAB=nVertices;
						nVertices++;						
					}

					t=t2;
					//t=-ss1/(ss3-ss1);
					if (t<SMALLF2)
					{
						sAC=PA;
						iAC=A;
					}
					else
					if (t>1-SMALLF2)
					{
						sAC=PC;
						iAC=C;
					}
					else
					{
						sAC=&VerticesS[nVertices];
						sAC->Stok=(1-t)*(PA->Stok) + t*(PC->Stok);
						sAC->Calc=(1-t)*(PA->Calc) + t*(PC->Calc);
						sAC->tag=0;
						
						iAC=nVertices;
						nVertices++;
					}

/*
					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.tag=f->tag;
					tmp.ref=f->ref;
					tmp.N=f->N;
*/
					
					tmp.n0=A;
					tmp.n1=iAB;
					tmp.n2=iAC;
					tmp.s0=PA;
					tmp.s1=sAB;
					tmp.s2=sAC;
					faces[nb]=tmp;
					

					tmp.n0=iAC;
					tmp.n1=iAB;
					tmp.n2=B;
					tmp.s0=sAC;
					tmp.s1=sAB;
					tmp.s2=PB;

					faces[nf]=tmp;


					tmp.n0=iAC;
					tmp.n1=B;
					tmp.n2=C;
					tmp.s0=sAC;
					tmp.s1=PB;
					tmp.s2=PC;
					faces[nf+1]=tmp;
		
					nf+=2;
				}
			}
		}

		NBRE=CutFaces(nP+1,faces,nf,mul);
	//	decoupees.Free();
	}
	else
	{
		NBRE=NF;
	}

	return NBRE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CBSPVolumicOld::CutFacesMap(int nP,CShortFaceMap *faces,int NF,float mul)
{
#define SMALLFWW SMALLF
	int res;
	int NBRE;
	int nx;
	CShortFaceMap *f;
	CShortFaceMap tmp;
	CShortVertexMap sh;
	CVector I;
	CShortVertexMap *P1,*P2,*P3;
	CShortVertexMap *PA,*PB,*PC,*sAB,*sAC;
	CPlane *p;
	int A,B,C;
	int iAB,iAC;
	float s1,s2,s3,s,ss;
	float ss1,ss2,ss3;
	float t,t1,t2;
	int cc;
	int n;
	int nb,LEN;
	int nf;

	nf=NF;
	LEN=NF;
	nx=0;
	if (nP<Plans.Length())
	{
		p=Plans[nP];

		for (n=0;n<nVertices;n++)
			VerticesS[n].coef1=(float) (VerticesS[n].Calc.x*p->a +VerticesS[n].Calc.y*p->b +VerticesS[n].Calc.z*p->c + p->d);

		for (nb=0;nb<LEN;nb++)
		{
			f=&faces[nb];
			P1=f->s0;
			P2=f->s1;
			P3=f->s2;
			s1=P1->coef1;
			s2=P2->coef1;
			s3=P3->coef1;
			//s1=(float) (P1->Calc.x*p->a +P1->Calc.y*p->b +P1->Calc.z*p->c + p->d);
			//s2=(float) (P2->Calc.x*p->a +P2->Calc.y*p->b +P2->Calc.z*p->c + p->d);
			//s3=(float) (P3->Calc.x*p->a +P3->Calc.y*p->b +P3->Calc.z*p->c + p->d);

			cc=(s1>mul*SMALLF)+(s2>mul*SMALLF)+(s3>mul*SMALLF);

			if ((cc==1)||(cc==2))
			{
				if (cc==1) s=1; else s=-1;

				t1=t2=-1;

				res=0;
				if (s*s1>=s*mul*SMALLF)
				{
					A=f->n0;B=f->n1;C=f->n2;
					PA=P1;
					PB=P2;
					PC=P3;
					ss1=s1;ss2=s2;ss3=s3;
					ss=s1;
					res=1;
				}
				else
				if (s*s2>=s*mul*SMALLF)
				{
					ss=s2;
					A=f->n1;B=f->n2;C=f->n0;
					ss1=s2;ss2=s3;ss3=s1;
					res=1;
					PA=P2;
					PB=P3;
					PC=P1;
				}
				else
				if (s*s3>=s*mul*SMALLF)
				{
					A=f->n2;B=f->n0;C=f->n1;
					ss1=s3;ss2=s1;ss3=s2;
					res=1;
					PA=P3;
					PB=P1;
					PC=P2;
				}
				
				if (res==1) 
				{
					t1=-ss1/(ss2-ss1);
					t2=-ss1/(ss3-ss1);

					t=t1;
					if (t<SMALLFWW)
					{
						sAB=PA;
						iAB=A;
					}
					else
					if (t>1.0f-SMALLFWW)
					{
						sAB=PB;
						iAB=B;
					}
					else
					{
						sAB=&VerticesS[nVertices];
						sAB->Stok=(1-t)*(PA->Stok) + t*(PB->Stok);
						sAB->Calc=(1-t)*(PA->Calc) + t*(PB->Calc);
						sAB->Map=(1-t)*(PA->Map) + t*(PB->Map);
                        if (t>0.5f) sAB->ndx=PA->ndx; else sAB->ndx=PB->ndx;
						sAB->tag=0;
						sAB->tag2=0;
						
						iAB=nVertices;
						nVertices++;
					}

					t=t2;
					if (t<SMALLFWW)
					{
						sAC=PA;
						iAC=A;
					}
					else
					if (t>1.0f-SMALLFWW)
					{
						sAC=PC;
						iAC=C;
					}
					else
					{
						sAC=&VerticesS[nVertices];
						sAC->Stok=(1-t)*(PA->Stok) + t*(PC->Stok);
						sAC->Calc=(1-t)*(PA->Calc) + t*(PC->Calc);
						sAC->Map=(1-t)*(PA->Map) + t*(PC->Map);
                        if (t>0.5f) sAC->ndx=PA->ndx; else sAC->ndx=PC->ndx;
						sAC->tag=0;
						sAC->tag2=0;
						
						iAC=nVertices;
						nVertices++;
					}

					tmp.nL=f->nL;
					tmp.nT=f->nT;
					tmp.nT2=f->nT2;
					tmp.tag=f->tag;
					tmp.ref=f->ref;
					
					tmp.n0=A;
					tmp.n1=iAB;
					tmp.n2=iAC;
					tmp.s0=PA;
					tmp.s1=sAB;
					tmp.s2=sAC;
					faces[nb]=tmp;
					
					tmp.n0=iAC;
					tmp.n1=iAB;
					tmp.n2=B;
					tmp.s0=sAC;
					tmp.s1=sAB;
					tmp.s2=PB;
					faces[nf]=tmp;

					tmp.n0=iAC;
					tmp.n1=B;
					tmp.n2=C;
					tmp.s0=sAC;
					tmp.s1=PB;
					tmp.s2=PC;
					faces[nf+1]=tmp;
		
					nf+=2;					

				}
			}
		}

		NBRE=CutFacesMap(nP+1,faces,nf,mul);
	}
	else
	{
		NBRE=NF;
	}

	return NBRE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildVolumeFaceNormal(CObject3D *obj,int nf,float mul,float mul2)
{
    int n0,n1;
    CVector u;
    CBSPNode *node1,*node2,*node3,*nodeplus,*nodemoins;
    
    CVector N=obj->Faces[nf].NormCalc;
    
    node1=new CBSPNode;
    node2=new CBSPNode;
    node3=new CBSPNode;
    
    n0=obj->Faces[nf].v0;
    n1=obj->Faces[nf].v1;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node1->a=u.x;
    node1->b=u.y;
    node1->c=u.z;
    node1->d=-(u||obj->Vertices[n0].Calc);
    node1->type=BSP_NODE;
    
    n0=obj->Faces[nf].v1;
    n1=obj->Faces[nf].v2;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node2->a=u.x;
    node2->b=u.y;
    node2->c=u.z;
    node2->d=-(u||obj->Vertices[n0].Calc);
    node2->type=BSP_NODE;
    
    n0=obj->Faces[nf].v2;
    n1=obj->Faces[nf].v0;
    u=N^(obj->Vertices[n1].Calc-obj->Vertices[n0].Calc);
    u.Normalise();
    node3->a=u.x;
    node3->b=u.y;
    node3->c=u.z;
    node3->d=-(u||obj->Vertices[n0].Calc);
    node3->type=BSP_NODE;
    
    root=node1;
    node1->moins=node2;
    node2->moins=node3;
    nodeplus=new CBSPNode;
    nodemoins=new CBSPNode;
    nodeplus->type=BSP_OUT;
    nodemoins->type=BSP_IN;
    node3->plus=nodeplus;
    node3->moins=nodemoins;
    
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node2->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node1->plus=nodeplus;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildVolumeQuadNormal(CVector &N,CVector &A,CVector &B,CVector &C,CVector &D,float mul,float mul2)
{
    CVector u;
    CBSPNode *node1,*node2,*node3,*node4,*nodeplus,*nodemoins;
    
    node1=new CBSPNode;
    node2=new CBSPNode;
    node3=new CBSPNode;
    node4=new CBSPNode;
    
    u=N^(B-A);
    u.Normalise();
    node1->a=u.x;
    node1->b=u.y;
    node1->c=u.z;
    node1->d=-(u||A);
    node1->type=BSP_NODE;
    
    u=N^(C-B);
    u.Normalise();
    node2->a=u.x;
    node2->b=u.y;
    node2->c=u.z;
    node2->d=-(u||B);
    node2->type=BSP_NODE;
    
    u=N^(D-C);
    u.Normalise();
    node3->a=u.x;
    node3->b=u.y;
    node3->c=u.z;
    node3->d=-(u||C);
    node3->type=BSP_NODE;

    u=N^(A-D);
    u.Normalise();
    node4->a=u.x;
    node4->b=u.y;
    node4->c=u.z;
    node4->d=-(u||D);
    node4->type=BSP_NODE;

    root=node1;
    node1->moins=node2;
    node2->moins=node3;
    node3->moins=node4;
    
    nodeplus=new CBSPNode;
    nodemoins=new CBSPNode;
    nodeplus->type=BSP_OUT;
    nodemoins->type=BSP_IN;
    node4->plus=nodeplus;
    node4->moins=nodemoins;
    
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node3->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node2->plus=nodeplus;
    nodeplus=new CBSPNode;
    nodeplus->type=BSP_OUT;
    node1->plus=nodeplus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildVolume(CObject3D *obj,int nf,float mul,float mul2,CVector L,int tag)
{
	int n,nn;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri1,tri2;
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	EdgeListD Edge;

	nn=0;
	for	(n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			nn++;
		}
	}

	Edge.Init(nn*3);

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			Edge.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
			Edge.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
			Edge.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
		}
	}

	//Edge.DelDoubleEdges();


	for (n=0;n<Edge.nList;n++)
	{
		sousliste=Triangles.InsertLast();

		A=(L+obj->Vertices[Edge.List[n].a].Calc)/2;
		B=(L+obj->Vertices[Edge.List[n].b].Calc)/2;
		C=obj->Vertices[Edge.List[n].b].Calc;
		D=obj->Vertices[Edge.List[n].a].Calc;
		tri1.A=A;
		tri1.B=B;
		tri1.C=C;
		tri2.A=A;
		tri2.B=C;
		tri2.C=D;
		u=(obj->Vertices[Edge.List[n].a].Calc-L)^(obj->Vertices[Edge.List[n].b].Calc -L);
		u.Normalise();
		sousliste->List.Add(tri1);
		sousliste->List.Add(tri2);
		sousliste->Norm=mul*u;
	}

	root=BuildNode(&Triangles,mul2);

	Edge.Free();

	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}

	Triangles.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildVolumeFace(CObject3D *obj,int nf,float mul,float mul2,CVector L)
{
	int n0,n1;
	CVector u;
	CMatrix Mobj,Mrot;
	CBSPNode *node1,*node2,*node3,*nodeplus,*nodemoins;

	node1=new CBSPNode;
	node2=new CBSPNode;
	node3=new CBSPNode;
	
	n0=obj->Faces[nf].v0;
	n1=obj->Faces[nf].v1;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node1->a=u.x;
	node1->b=u.y;
	node1->c=u.z;
	node1->d=-(u||(L));
	node1->type=BSP_NODE;

	n0=obj->Faces[nf].v1;
	n1=obj->Faces[nf].v2;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node2->a=u.x;
	node2->b=u.y;
	node2->c=u.z;
	node2->d=-(u||(L));
	node2->type=BSP_NODE;

	n0=obj->Faces[nf].v2;
	n1=obj->Faces[nf].v0;
	u=(obj->Vertices[n1].Calc -L)^(obj->Vertices[n0].Calc-L);
	u.Normalise();
	node3->a=u.x;
	node3->b=u.y;
	node3->c=u.z;
	node3->d=-(u||(L));
	node3->type=BSP_NODE;

	root=node1;
	node1->moins=node2;
	node2->moins=node3;
	nodeplus=new CBSPNode;
	nodemoins=new CBSPNode;
	nodeplus->type=BSP_OUT;
	nodemoins->type=BSP_IN;
	node3->plus=nodeplus;
	node3->moins=nodemoins;

	nodeplus=new CBSPNode;
	nodeplus->type=BSP_OUT;
	node2->plus=nodeplus;
	nodeplus=new CBSPNode;
	nodeplus->type=BSP_OUT;
	node1->plus=nodeplus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DEFINES_API3D_CODE_BUILDGROUPVOLUMEFAST_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L)
{
	CShortFaceMapGROUP *g=(CShortFaceMapGROUP*) vg;
	int *i,n;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri;
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	CFigure * f;
	int array[512];
	int na;
	int a,b;


	f=g->figures->GetFirst();
	while (f)
	{
		n=0;
		i=f->List.GetFirst();
		while (i)
		{
			array[n]=*i;
			n++;
			i=f->List.GetNext();
		}
		na=f->List.Length();

		for (n=0;n<na;n++)
		{
			a=array[n];
			b=array[(n+1)%na];

			sousliste=Triangles.InsertLast();

			C=obj->Vertices[a].Calc;
			D=obj->Vertices[b].Calc;

			B.x=(L.x+C.x)*0.5f;
			B.y=(L.y+C.y)*0.5f;
			B.z=(L.z+C.z)*0.5f;

			A.x=(L.x+D.x)*0.5f;
			A.y=(L.y+D.y)*0.5f;
			A.z=(L.z+D.z)*0.5f;

			tri.A=A;
			tri.B=B;
			tri.C=C;
			sousliste->List.Add(tri);
			tri.A=A;
			tri.B=C;
			tri.C=D;
			sousliste->List.Add(tri);
			

			CVector u1,u2;
			VECTORSUB(u1,C,L);
			VECTORSUB(u2,D,L);
			CROSSPRODUCT(u,u1,u2);
			VECTORNORMALISE(u);
		
			sousliste->Norm=mul*u;
		}

		f=g->figures->GetNext();
	}

	root=BuildNode(&Triangles,mul2);


	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}

	Triangles.Free();
}
#else
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildGroupVolume(CObject3D *obj,void * vg,float mul,float mul2,CVector L)
{
	CFaceGROUP *g=(CFaceGROUP*) vg;
	int *i,n;
	static CTriangle2 Triangles[512];
	CVector u,A,B,C,D;
	CMatrix Mobj,Mrot;
	CFigure * f;
	static int array[512];
	int na;
	int a,b;
	int nt=0;
	float K=SMALLF;

	f=g->figures->GetFirst();
	while (f)
	{
		n=0;
		i=f->List.GetFirst();
		while (i)
		{
			array[n]=*i;
			n++;
			i=f->List.GetNext();
		}
		na=f->List.Length();

		for (n=0;n<na;n++)
		{
			a=array[n];
			b=array[(n+1)%na];

			C=obj->Vertices[a].Calc;
			D=obj->Vertices[b].Calc;
			
			B.x=K*L.x+(1-K)*C.x;
			B.y=K*L.y+(1-K)*C.y;
			B.z=K*L.z+(1-K)*C.z;

			A.x=K*L.x+(1-K)*D.x;
			A.y=K*L.y+(1-K)*D.y;
			A.z=K*L.z+(1-K)*D.z;

			CVector u1,u2;
			VECTORSUB(u1,C,L);
			VECTORSUB(u2,D,L);
			CROSSPRODUCT(u,u1,u2);
			VECTORNORMALISE(u);
		
			Triangles[nt].A=A;
			Triangles[nt].B=C;
			Triangles[nt].C=D;
			Triangles[nt].Norm=mul*u;
			Triangles[nt].tag=0;
			nt++;
		}

		f=g->figures->GetNext();
	}

	root=BuildNode2(Triangles,nt,mul2,0,-1);

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::Build(CObject3D *obj,float mul,float mul2)
{
	int n,nn;
	bool test;
	int tag;
	CList <CTriangles> Triangles;
	CTriangles * sousliste;
	CTriangle tri;
	CMatrix Mobj,Mrot;

	#define SMALLFXYZ SMALLF2
	#define SMALLFSC SMALLF3

	obj->Calculate();
	obj->SetF012();
	
	if (!grouped)
	{
		for (n=0;n<obj->nFaces;n++)
			if (VECTORNORM2(obj->Faces[n].Norm)<0.25f) obj->Faces[n].tag=-1;
	}

	tag=1;
	test=true;
	nn=0;
	
	while (test)
	{
		n=nn;
		nn=-1;

		if (!grouped)
		{
			while ((n<obj->nFaces)&&(nn==-1))
			{
				if (obj->Faces[n].tag==0) nn=n;
				else n++;
			}
		}
		else
		{
			while ((n<obj->nFaces)&&(nn==-1))
			{
				if (VECTORNORM2(obj->Faces[n].Norm)>=0.25f)
				{
					if (obj->Faces[n].tag==0) nn=n;
					else n++;
				}
				else n++;
			}
		}

		if (nn==-1) test=false;
		else
		{
			sousliste=NULL;

			if (grouped)
			{
				obj->TagSetHCoplanarFaces(nn,nn,tag);
				obj->Faces[nn].tag=tag;

				for (n=0;n<obj->nFaces;n++)
				{
					if (obj->Faces[n].tag==tag)
					{
						CVector u1,u2,u3;

						VECTORSUB(u1,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[0]->Calc);
						VECTORSUB(u2,obj->Faces[n].v[2]->Calc,obj->Faces[n].v[1]->Calc);
						VECTORSUB(u3,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[2]->Calc);

						float f1=VECTORNORM(u1);
						float f2=VECTORNORM(u2);
						float f3=VECTORNORM(u3);

						if ((f1>SMALLFXYZ)&&(f2>SMALLFXYZ)&&(f3>SMALLFXYZ))
						{
							VECTORDIV(u1,u1,f1);
							VECTORDIV(u2,u2,f2);
							VECTORDIV(u3,u3,f3);

							if ((DOT(u1,u2)<1.0f-SMALLFSC)&&(DOT(u2,u3)<1.0f-SMALLFSC)&&(DOT(u1,u3)<1.0f-SMALLFSC))
							{
								if (mul<0)
								{
									tri.A=obj->Faces[n].v[0]->Calc;
									tri.B=obj->Faces[n].v[1]->Calc;
									tri.C=obj->Faces[n].v[2]->Calc;
								}
								else
								{
									tri.A=obj->Faces[n].v[0]->Calc;
									tri.B=obj->Faces[n].v[2]->Calc;
									tri.C=obj->Faces[n].v[1]->Calc;
								}
								if (sousliste==NULL) sousliste=Triangles.InsertLast();
								sousliste->List.Add(tri);
							}
						}
							
					}
				}
			}
			else
			{
				obj->Faces[nn].tag=tag;

				n=nn;

				CVector u1,u2,u3;

				VECTORSUB(u1,obj->Faces[n].v[1]->Calc,obj->Faces[n].v[0]->Calc);
				VECTORSUB(u2,obj->Faces[n].v[2]->Calc,obj->Faces[n].v[1]->Calc);
				VECTORSUB(u3,obj->Faces[n].v[0]->Calc,obj->Faces[n].v[2]->Calc);

				float f1=VECTORNORM(u1);
				float f2=VECTORNORM(u2);
				float f3=VECTORNORM(u3);

				if ((f1>SMALLFXYZ)&&(f2>SMALLFXYZ)&&(f3>SMALLFXYZ))
				{
					VECTORDIV(u1,u1,f1);
					VECTORDIV(u2,u2,f2);
					VECTORDIV(u3,u3,f3);

					if ((DOT(u1,u2)<1.0f-SMALLFSC)&&(DOT(u2,u3)<1.0f-SMALLFSC)&&(DOT(u1,u3)<1.0f-SMALLFSC))
					{
						if (mul<0)
						{
							tri.A=obj->Faces[n].v[0]->Calc;
							tri.B=obj->Faces[n].v[1]->Calc;
							tri.C=obj->Faces[n].v[2]->Calc;
						}
						else
						{
							tri.A=obj->Faces[n].v[0]->Calc;
							tri.B=obj->Faces[n].v[2]->Calc;
							tri.C=obj->Faces[n].v[1]->Calc;
						}
						if (sousliste==NULL) sousliste=Triangles.InsertLast();
						sousliste->List.Add(tri);
					}
				}
			}
	
			if (sousliste) sousliste->Norm=mul*obj->Faces[nn].NormCalc;

			tag++;
		}
	}

	if (Triangles.Length()>0)
	{
		root=BuildNode(&Triangles,mul2);

		sousliste=Triangles.GetFirst();
		while (sousliste)
		{
			sousliste->List.Free();
			sousliste=Triangles.GetNext();
		}

		Triangles.Free();
	}
	else root=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPNode * CBSPVolumicOld::BuildNode(CList <CTriangles> *Triangles,float mul)
{
	CBSPNode *node;
	CList <CTriangles> l_plus,l_moins;
	CTriangles *plus,*moins,*first,*tmp;
	CList <CTriangle> tmpplus,tmpmoins;
	CTriangle *tri;

	node=NULL;
	
	if (Triangles->Length()>0)
	{
		node=new CBSPNode;

		first=Triangles->GetFirst();

		node->a=first->Norm.x;
		node->b=first->Norm.y;
		node->c=first->Norm.z;
		node->d=-DOT((first->List.GetFirst())->A,first->Norm);
		node->type=BSP_NODE;

		if (Triangles->Length()>1)
		{
			tmp=Triangles->GetNext();
			while (tmp)
			{
				plus=moins=NULL;

				tri=tmp->List.GetFirst();
				while (tri)
				{				
					DecoupeTRIANGLE(mul,tri,node->a,node->b,node->c,node->d,&tmpplus,&tmpmoins);
					tri=tmp->List.GetNext();
				}

				if (tmpmoins.Length()>0)
				{					
					moins=l_moins.InsertLast();
					moins->Norm=tmp->Norm;

					tri=tmpmoins.GetFirst();
					while (tri)
					{
						moins->List.Add(*tri);
						tri=tmpmoins.GetNext();
					}
				}

				if (tmpplus.Length()>0)
				{					
					plus=l_plus.InsertLast();
					plus->Norm=tmp->Norm;

					tri=tmpplus.GetFirst();
					while (tri)
					{
						plus->List.Add(*tri);
						tri=tmpplus.GetNext();
					}
				}
				
				tmpmoins.Free();
				tmpplus.Free();
				tmp=Triangles->GetNext();
			}

			if (l_moins.Length()>0)
			{
				node->moins=BuildNode(&l_moins,mul);
			}
			else
			{
				node->moins=new CBSPNode;
				node->moins->type=BSP_IN;
			}

			if (l_plus.Length()>0)
			{
				node->plus=BuildNode(&l_plus,mul);
			}
			else
			{
				node->plus=new CBSPNode;
				node->plus->type=BSP_OUT;
			}

			tmp=l_moins.GetFirst();
			while (tmp)
			{
				tmp->List.Free();
				tmp=l_moins.GetNext();
			}

			tmp=l_plus.GetFirst();
			while (tmp)
			{
				tmp->List.Free();
				tmp=l_plus.GetNext();
			}

			l_moins.Free();
			l_plus.Free();
		}
		else
		{
			node->plus=new CBSPNode;
			node->plus->type=BSP_OUT;
			node->moins=new CBSPNode;
			node->moins->type=BSP_IN;
		}
	}
	return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBSPNode * CBSPVolumicOld::BuildNode2(CTriangle2 *Triangles,int nt,float mul,int N,int mask)
{
	CBSPNode *node;
	CTriangle2 *first,*tmp;
	int n,nlevel;
	float a,b,c,d;
	int np,nm;
	int NT=nt;

	node=NULL;
	first=NULL;

	nlevel=0;
	for (n=0;n<nt;n++)
	{	
		if (maskequiv(mask,N,Triangles[n].tag))
		{
			if (first==NULL) 
			{
				first=&Triangles[n];
				Triangles[n].tag=-1;
			}
			nlevel++;
		}
	}

	if (nlevel>0)
	{
		node=new CBSPNode;

		a=node->a=first->Norm.x;
		b=node->b=first->Norm.y;
		c=node->c=first->Norm.z;
		d=node->d=-DOT(first->A,first->Norm);
		node->type=BSP_NODE;

		np=nm=0;

		if (nlevel>1)
		{
			for (n=0;n<nt;n++)
			{
				if (maskequiv(mask,N,Triangles[n].tag))
				{
					tmp=&Triangles[n];
					DecoupeTRIANGLE_array(mul,tmp,a,b,c,d,Triangles,&NT,N,&nm,&np);
				}
			}

			if (nm>0) node->moins=BuildNode2(Triangles,NT,mul,N+1,0);
			else
			{
				node->moins=new CBSPNode;
				node->moins->type=BSP_IN;
			}

			if (np>0) node->plus=BuildNode2(Triangles,NT,mul,N+1,1);
			else
			{
				node->plus=new CBSPNode;
				node->plus->type=BSP_OUT;
			}

		}
		else
		{
			node->plus=new CBSPNode;
			node->plus->type=BSP_OUT;
			node->moins=new CBSPNode;
			node->moins->type=BSP_IN;
		}
	}


	return node;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBSPVolumicOld::BuildObjectVolume(CObject3D *obj,float mul,float mul2)
{
	bool test=false;
	int n,nn,N,n0;
	CTriangle tri;
	CList <CTriangles> Triangles;
	CTriangles *sousliste;
	int tag;

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
	N=0;
	tag=1;
	test=true;
	while (test)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag==0) nn=n;
			else
				n++;
		}

		if (nn==-1) test=false;
		else
		{
			obj->TagCoplanarOnlyFaces(nn,tag);
			obj->Faces[nn].tag=tag;
			sousliste=Triangles.InsertLast();
			n0=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					tri.A=obj->Faces[n].v[0]->Stok;
					tri.B=obj->Faces[n].v[1]->Stok;
					tri.C=obj->Faces[n].v[2]->Stok;
					sousliste->List.Add(tri);
					n0++;
				}
			}
			sousliste->Norm=mul*obj->Faces[nn].Norm;
			tag++;
		}
	}

	root=BuildNode(&Triangles,mul2);

	sousliste=Triangles.GetFirst();
	while (sousliste)
	{
		sousliste->List.Free();
		sousliste=Triangles.GetNext();
	}
	Triangles.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
