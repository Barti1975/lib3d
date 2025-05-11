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
//	@file pob_generator.cpp
//	@date 2013-01-07
////////////////////////////////////////////////////////////////////////

  
#pragma warning (disable:4244)

#include "params.h"

#include <math.h>
#include <stdio.h>
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "physic_object.h"
#include "bsp_volumic.h"
#include "edges.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation
		matrix diagonalisation

		CMatrix CPOBGenerator::JacobiNew(CMatrix M0)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CMatrix CPOBGenerator::JacobiNew(CMatrix M0)
{
#define N_ITER_MAX 500
#define epsilon 0.00000001f
    int q,p,c;
    int res;
    float a,aa,theta;
    int n,n1,n2;
    float e;
    CMatrix Ak,Ak1,Bk,Dk,W,Wt,VP;
    
    res=1;
    
    VP.Id();
    Ak=M0;
    
    n=0;
    while ((res==1)&&(n<N_ITER_MAX))
    {        
        n++;
        a=0; p=q=0;
        for (n1=0;n1<3;n1++)
            for (n2=0;n2<3;n2++)
            {
                if (n1!=n2)
                {
                    aa=f_abs(Ak.a[n1][n2]);
                    if (a<aa)
                    {
                        a=aa;
                        p=n1;
                        q=n2;
                    }
                }
            }
        
        c=-1;
        //if (f_abs(f_abs(Ak.a[0][1])-a)<epsilon) {c=0;p=0;q=1;}
        //if (f_abs(f_abs(Ak.a[0][2])-a)<epsilon) {c=1;p=0;q=2;}
        //if (f_abs(f_abs(Ak.a[1][2])-a)<epsilon) {c=2;p=1;q=2;}
        
        if (f_abs(Ak.a[q][q]-Ak.a[p][p])>epsilon)
            theta=atanf((2*Ak.a[p][q])/(Ak.a[q][q]-Ak.a[p][p]))/2;
        else theta=0;
                
        W.Id();
        
        W.a[p][p]=cosf(theta);
        W.a[q][q]=W.a[p][p];
        W.a[q][p]=-sinf(theta);
        W.a[p][q]=-W.a[q][p];
                
        Wt.Transpose(W);
        
        VP=Wt*VP;
        
        Ak1=(Ak*W);
        Ak1=Wt*Ak1;
        Bk=Ak1;
        
        Bk.a[0][0]=0;
        Bk.a[1][1]=0;
        Bk.a[2][2]=0;
        
        e=Bk.Norme3x3();
        
        if (e<epsilon) res=0;
        
        Ak=Ak1;
    }
    return VP;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Jacobi

		CMatrix CPOBGenerator::Jacobi(CMatrix M0)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CMatrix CPOBGenerator::Jacobi(CMatrix M0)
{
#define N_ITER_MAX 500
#define epsilon 0.00000001f
	int q,p,c;
	int res;
	float a,aa,theta;
	int n,n1,n2;
	float e;
	CMatrix Ak,Ak1,Bk,Dk,W,Wt,VP;

	res=1;

	VP.Id();
	Ak=M0;

	n=0;
	while ((res==1)&&(n<N_ITER_MAX))
	{
		n++;
		a=0;
		for (n1=0;n1<3;n1++)
			for (n2=0;n2<3;n2++)
			{
				if (n1!=n2)
				{
					aa=f_abs(Ak.a[n1][n2]);
					if (a<aa) a=aa;
				}
			}

        p=q=-1;

		if (f_abs(f_abs(Ak.a[0][1])-a)<epsilon) {c=0;p=0;q=1;}
		if (f_abs(f_abs(Ak.a[0][2])-a)<epsilon) {c=1;p=0;q=2;}
		if (f_abs(f_abs(Ak.a[1][2])-a)<epsilon) {c=2;p=1;q=2;}

        if ((p>=0)&&(q>=0))
        {
		    if (f_abs(Ak.a[q][q]-Ak.a[p][p])>SMALLF)
			    theta=atanf((2*Ak.a[p][q])/(Ak.a[q][q]-Ak.a[p][p]))/2;
		    else theta=0;
        }
        else theta=0;

		W.Id();

        W.a[p][p]=cosf(theta);
        W.a[q][q]=W.a[p][p];
        W.a[q][p]=-sinf(theta);
        W.a[p][q]=-W.a[q][p];
		
		Wt.Transpose(W);

		VP=Wt*VP;

		Ak1=(Ak*W);
        Ak1=Wt*Ak1;      
		Bk=Ak1;

		Bk.a[0][0]=0;
		Bk.a[1][1]=0;
		Bk.a[2][2]=0;

		e=Bk.Norme3x3();

		if (e<epsilon) res=0;

		Ak=Ak1;
	}
	return VP;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFromEvictNT(CObject3D * obj,int tag,int NITER)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CPOB * CPOBGenerator::POBFromEvictNT(CObject3D * obj,int nt)
{
	int n,nn,j,k;
	CPOB * POB;
	float Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
	CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
	CMatrix C,C1;
	CVector X,Y,Z;

	X.Init(1,0,0);
	Y.Init(0,1,0);
	Z.Init(0,0,1);

	Mean.Init(0,0,0);

	for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].nT!=nt)
		{
			obj->Faces[n].v[0]->tag=1;
			obj->Faces[n].v[1]->tag=1;
			obj->Faces[n].v[2]->tag=1;

			Mean+=obj->Faces[n].v[0]->Calc;
			Mean+=obj->Faces[n].v[1]->Calc;
			Mean+=obj->Faces[n].v[2]->Calc;
			nn++;
		}
	}
	Mean=Mean/(float) (3*nn);

	M=O=Mean;
	
	for (j=0;j<3;j++)
		for (k=0;k<3;k++)
		{
			C.a[j][k]=0;

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].nT!=nt)
				{
					P=obj->Faces[n].v[0]->Calc;
					Q=obj->Faces[n].v[1]->Calc;
					R=obj->Faces[n].v[2]->Calc;				

					C.a[j][k]+=(P.v[j]-M.v[j])*(P.v[k]-M.v[k]) + (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]);			
				}
			}
			C.a[j][k]=C.a[j][k]/(float) (3*nn);
		}

	POB=new CPOB;
	POB->ID=POB_CLOSING;
	POB->Next1=0;
	POB->Next2=0;

	C1=Jacobi(C);

	Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
	Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
	Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;

	Ux.Normalise();
	Uy.Normalise();
	Uz.Normalise();
	
	POB->Ux=Ux;
	POB->Uy=Uy;
	POB->Uz=Uz;

	Lx=Ly=Lz=-1000;
	mLx=mLy=mLz=1000;
	sx=sy=sz=0;

	for (n=0;n<obj->nVertices;n++)
	if (obj->Vertices[n].tag)
	{
		vect=obj->Vertices[n].Calc-O;

		sx=(vect||Ux);
		if (sx>Lx) Lx=sx;
		if (sx<mLx) mLx=sx;
			
		sy=(vect||Uy);
		if (sy>Ly) Ly=sy;
		if (sy<mLy) mLy=sy;
			
		sz=(vect||Uz);
		if (sz>Lz) Lz=sz;
		if (sz<mLz) mLz=sz;
	}

	POB->Lx=Lx;
	POB->Ly=Ly;
	POB->Lz=Lz;
	POB->mLx=mLx;
	POB->mLy=mLy;
	POB->mLz=mLz;

	POB->O=O;

	return POB;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFrom2(CObject3D * obj,int tag,int NITER)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CPOB * CPOBGenerator::POBFrom2(CObject3D * obj,int tag,int NITER)
{
    int n,nn,j,k;
    CPOB * POB;
    float Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
    CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
    CMatrix C,C1;
    float s0,s1,s2;
    float a,b,c,d;
    CVector X,Y,Z;
    
    X.Init(1,0,0);
    Y.Init(0,1,0);
    Z.Init(0,0,1);
    
    if (tag==0) nPOB=0;
    
    Mean.Init(0,0,0);
    
    for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            obj->Faces[n].v[0]->tag=1;
            obj->Faces[n].v[1]->tag=1;
            obj->Faces[n].v[2]->tag=1;
        }
    }
    
    nn=0;
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            Mean+=obj->Faces[n].v[0]->Stok;
            Mean+=obj->Faces[n].v[1]->Stok;
            Mean+=obj->Faces[n].v[2]->Stok;
            nn++;
        }
    }
    
    Mean=Mean/(float) (3*nn);
    O=M=Mean;
    
    int err=0;
    
    for (j=0;j<3;j++)
        for (k=0;k<3;k++)
        {
            C.a[j][k]=0;
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                                        
                    C.a[j][k]+=(P.v[j]-M.v[j])*(P.v[k]-M.v[k]) + (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]);
                }
            }
            C.a[j][k]=C.a[j][k]/(float) (3*nn);
            if (nn==0) err=1;
        }    
    
    POB=new CPOB;
    POB->ID=POB_CLOSING;
    POB->Next1=0;
    POB->Next2=0;
    
    if (err==0) C1=Jacobi(C); else C1.Id();
    
    Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
    Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
    Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;
    
    Ux.Normalise();
    Uy.Normalise();
    Uz.Normalise();
        
    POB->Ux=Ux;
    POB->Uy=Uy;
    POB->Uz=Uz;
    
    Lx=Ly=Lz=-10000;
    mLx=mLy=mLz=10000;
    sx=sy=sz=0;
    
    for (n=0;n<obj->nVertices;n++)
    {
        if (obj->Vertices[n].tag==1)
        {
            vect=obj->Vertices[n].Stok-O;
            sx=(vect||Ux);
            if (sx>Lx) Lx=sx;
            if (sx<mLx) mLx=sx;
            
            sy=(vect||Uy);
            if (sy>Ly) Ly=sy;
            if (sy<mLy) mLy=sy;
            
            sz=(vect||Uz);
            if (sz>Lz) Lz=sz;
            if (sz<mLz) mLz=sz;            
        }
    }
    
    O+=((mLx+Lx)/2)*Ux +((mLy+Ly)/2)*Uy +((mLz+Lz)/2)*Uz;
    
    Lx=(Lx-mLx)/2;
    Ly=(Ly-mLy)/2;
    Lz=(Lz-mLz)/2;
    
    POB->Lx=Lx;
    POB->Ly=Ly;
    POB->Lz=Lz;
    POB->mLx=-Lx;
    POB->mLy=-Ly;
    POB->mLz=-Lz;
    
    POB->O=O;
    
    if (NITER>0)
    {
        POB->ID=POB_NODE;
        
        if ((Lx>Ly)&&(Lx>Lz)) N=Ux;
        if ((Ly>Lx)&&(Ly>Lz)) N=Uy;
        if ((Lz>Ly)&&(Lz>Lx)) N=Uz;
        
        a=N.x;
        b=N.y;
        c=N.z;
        d=-(a*Mean.x+b*Mean.y+c*Mean.z);
        
        for (n=0;n<obj->nFaces;n++)
        {
            if (obj->Faces[n].tag==tag)
            {
                P=obj->Faces[n].v[0]->Stok;
                Q=obj->Faces[n].v[1]->Stok;
                R=obj->Faces[n].v[2]->Stok;
                
                s0=a*P.x + b*P.y + c*P.z +d;
                s1=a*Q.x + b*Q.y + c*Q.z +d;
                s2=a*R.x + b*R.y + c*R.z +d;
                
                if (!((s0>0)&&(s1>0)&&(s2>0))) obj->Faces[n].tag++;                
            }
        }
        
        nn=0;
        for (n=0;n<obj->nFaces;n++)
            if (obj->Faces[n].tag==tag) nn++;
        
        if (nn<2)
        {
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
            
            POB->ID=POB_CLOSING;
            
            nPOB++;
            
            NITERATIONS=NITER;
        }
        else
        {            
            POB->Next1=POBFrom2(obj,tag+1,NITER-1);
            
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
                        
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                    
                    s0=a*P.x + b*P.y + c*P.z +d;
                    s1=a*Q.x + b*Q.y + c*Q.z +d;
                    s2=a*R.x + b*R.y + c*R.z +d;
                                        
                    if (!((s0<0)&&(s1<0)&&(s2<0))) obj->Faces[n].tag++;                    
                }
            }
            POB->Next2=POBFrom2(obj,tag+1,NITER-1);
            
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
        }
    }
    else
    {
        nPOB++;
    }
    
    if (tag==0) POB->nPOB=nPOB;
    return POB;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFrom2Topo(CObject3D * obj)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CPOB * CPOBGenerator::POBFrom2Topo(CObject3D * obj,int tag)
{
    int n,nn,j,k;
    CPOB * POB;
    float Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
    CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
    CMatrix C,C1;
    CVector X,Y,Z;
    
    X.Init(1,0,0);
    Y.Init(0,1,0);
    Z.Init(0,0,1);
    
    if (tag==0) nPOB=0;
    
    Mean.Init(0,0,0);
    
    for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            obj->Faces[n].v[0]->tag=1;
            obj->Faces[n].v[1]->tag=1;
            obj->Faces[n].v[2]->tag=1;
        }
    }
    
    nn=0;
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            Mean+=obj->Faces[n].v[0]->Stok;
            Mean+=obj->Faces[n].v[1]->Stok;
            Mean+=obj->Faces[n].v[2]->Stok;
            nn++;
        }
    }
    Mean=Mean/(float) (3*nn);
    
    M=Mean;
    
    O=Mean;
    
    int err=0;
    
    for (j=0;j<3;j++)
        for (k=0;k<3;k++)
        {
            C.a[j][k]=0;
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                                        
                    C.a[j][k]+=(P.v[j]-M.v[j])*(P.v[k]-M.v[k]) + (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]);
                }
            }
            C.a[j][k]=C.a[j][k]/(float) (3*nn);
            if (nn==0) err=1;
        }
        
    POB=new CPOB;
    POB->ID=POB_CLOSING;
    POB->Next1=0;
    POB->Next2=0;
    
    if (err==0) C1=Jacobi(C); else C1.Id();
    
    Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
    Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
    Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;
    
    Ux.Normalise();
    Uy.Normalise();
    Uz.Normalise();
    
    POB->Ux=Ux;
    POB->Uy=Uy;
    POB->Uz=Uz;
    
    Lx=Ly=Lz=-10000;
    mLx=mLy=mLz=10000;
    sx=sy=sz=0;
    
    for (n=0;n<obj->nVertices;n++)
    {
        if (obj->Vertices[n].tag==1)
        {
            vect=obj->Vertices[n].Stok-O;
            sx=(vect||Ux);
            if (sx>Lx) Lx=sx;
            if (sx<mLx) mLx=sx;
            
            sy=(vect||Uy);
            if (sy>Ly) Ly=sy;
            if (sy<mLy) mLy=sy;
            
            sz=(vect||Uz);
            if (sz>Lz) Lz=sz;
            if (sz<mLz) mLz=sz;
            
        }
    }
    
    O+=((mLx+Lx)/2)*Ux +((mLy+Ly)/2)*Uy +((mLz+Lz)/2)*Uz;
    
    POB->Lx=Lx;
    POB->Ly=Ly;
    POB->Lz=Lz;
    POB->mLx=-Lx;
    POB->mLy=-Ly;
    POB->mLz=-Lz;
    
    POB->O=O;
    
    return POB;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFrom2zero(CObject3D * obj)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CPOB * CPOBGenerator::POBFrom2zero(CObject3D * obj,int tag,int NITER)
{
    int n,nn,j,k;
    CPOB * POB;
    float Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
    CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
    CMatrix C,C1;
    float s0,s1,s2;
    float a,b,c,d;
    CVector X,Y,Z;
    
    X.Init(1,0,0);
    Y.Init(0,1,0);
    Z.Init(0,0,1);
    
    if (tag==0) nPOB=0;
    
    Mean.Init(0,0,0);
    
    for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            obj->Faces[n].v[0]->tag=1;
            obj->Faces[n].v[1]->tag=1;
            obj->Faces[n].v[2]->tag=1;
        }
    }
    
    nn=0;
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            Mean+=obj->Faces[n].v[0]->Stok;
            Mean+=obj->Faces[n].v[1]->Stok;
            Mean+=obj->Faces[n].v[2]->Stok;
            nn++;
        }
    }
    
    Mean=Mean/(float) (3*nn);
    
    M=Mean;
    O=Mean;
    
    int err=0;
    
    for (j=0;j<3;j++)
        for (k=0;k<3;k++)
        {
            C.a[j][k]=0;
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                    
                    C.a[j][k]+=(P.v[j]-M.v[j])*(P.v[k]-M.v[k]) + (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]);
                }
            }
            C.a[j][k]=C.a[j][k]/(float) (3*nn);
            if (nn==0) err=1;
        }
        
    POB=new CPOB;
    POB->ID=POB_CLOSING;
    POB->Next1=0;
    POB->Next2=0;
    
    if (err==0) C1=Jacobi(C); else C1.Id();
    
    Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
    Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
    Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;
    
    Ux.Normalise();
    Uy.Normalise();
    Uz.Normalise();
    
    POB->Ux=Ux;
    POB->Uy=Uy;
    POB->Uz=Uz;
    
    Lx=Ly=Lz=-10000;
    mLx=mLy=mLz=10000;
    sx=sy=sz=0;
    
    for (n=0;n<obj->nVertices;n++)
    {
        if (obj->Vertices[n].tag==1)
        {
            vect=obj->Vertices[n].Stok-O;
            sx=(vect||Ux);
            if (sx>Lx) Lx=sx;
            if (sx<mLx) mLx=sx;
            
            sy=(vect||Uy);
            if (sy>Ly) Ly=sy;
            if (sy<mLy) mLy=sy;
            
            sz=(vect||Uz);
            if (sz>Lz) Lz=sz;
            if (sz<mLz) mLz=sz;
        }
    }
    
    O+=((mLx+Lx)/2)*Ux +((mLy+Ly)/2)*Uy +((mLz+Lz)/2)*Uz;
    
    Lx=(Lx-mLx)/2;
    Ly=(Ly-mLy)/2;
    Lz=(Lz-mLz)/2;
    
    POB->Lx=Lx;
    POB->Ly=Ly;
    POB->Lz=Lz;
    POB->mLx=-Lx;
    POB->mLy=-Ly;
    POB->mLz=-Lz;
    
    POB->O=O;
    
    if (NITER>0)
    {
        POB->ID=POB_NODE;
        
        if ((Lx>Ly)&&(Lx>Lz)) N=Ux;
        if ((Ly>Lx)&&(Ly>Lz)) N=Uy;
        if ((Lz>Ly)&&(Lz>Lx)) N=Uz;
        
        a=N.x;
        b=N.y;
        c=N.z;
        d=-(a*Mean.x+b*Mean.y+c*Mean.z);
        d=-(a*O.x+b*O.y+c*O.z);
        
        for (n=0;n<obj->nFaces;n++)
        {
            if (obj->Faces[n].tag==tag)
            {
                P=obj->Faces[n].v[0]->Stok;
                Q=obj->Faces[n].v[1]->Stok;
                R=obj->Faces[n].v[2]->Stok;
                
                s0=a*P.x + b*P.y + c*P.z +d;
                s1=a*Q.x + b*Q.y + c*Q.z +d;
                s2=a*R.x + b*R.y + c*R.z +d;
                
                if (((s0>0)&&(s1>0)&&(s2>0))) obj->Faces[n].tag++;
            }
        }
        
        nn=0;
        for (n=0;n<obj->nFaces;n++)
            if (obj->Faces[n].tag==tag) nn++;
        
        if (nn<8)
        {
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
            
            POB->ID=POB_CLOSING;
            
            nPOB++;
            
            NITERATIONS=NITER;
        }
        else
        {            
            POB->Next2=POBFrom2zero(obj,tag+1,NITER-1);
            
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
                        
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                    
                    s0=a*P.x + b*P.y + c*P.z +d;
                    s1=a*Q.x + b*Q.y + c*Q.z +d;
                    s2=a*R.x + b*R.y + c*R.z +d;
                    
                    
                    if (((s0<0)&&(s1<0)&&(s2<0))) obj->Faces[n].tag++;
                    
                }
            }
            POB->Next1=POBFrom2zero(obj,tag+1,NITER-1);
            
            for (n=0;n<obj->nFaces;n++)
                if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
        }
    }
    else nPOB++;
    
    if (tag==0) POB->nPOB=nPOB;
    return POB;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFromPoints(CObject3D * obj)

		
	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CPOB * CPOBGenerator::POBFromPoints(CObject3D * obj,int tag)
{
	int n,nn,j,k;
	CPOB * POB;
	float Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
	CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
	CMatrix C,C1;
	CVector X,Y,Z;

	X.Init(1,0,0);
	Y.Init(0,1,0);
	Z.Init(0,0,1);

	Mean.Init(0,0,0);

	for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			obj->Faces[n].v[0]->tag=1;
			obj->Faces[n].v[1]->tag=1;
			obj->Faces[n].v[2]->tag=1;
		}
	}		

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			Mean+=obj->Faces[n].v[0]->Stok;
			Mean+=obj->Faces[n].v[1]->Stok;
			Mean+=obj->Faces[n].v[2]->Stok;
			nn++;
		}
	}

	Mean=Mean/(float) (3*nn);
	M=Mean;	
	O=Mean;
		
	for (j=0;j<3;j++)
		for (k=0;k<3;k++)
		{
			C.a[j][k]=0;
			float ss=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					float s=obj->Faces[n].Norm.y*0.5f+0.5f;
					P=obj->Faces[n].v[0]->Stok;
					Q=obj->Faces[n].v[1]->Stok;
					R=obj->Faces[n].v[2]->Stok;
					ss+=s;

					C.a[j][k]+=s*((P.v[j]-M.v[j])*(P.v[k]-M.v[k]) +  (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]));
				}
			}
			C.a[j][k]=C.a[j][k]/(float) (3*nn*ss);
		}

	POB=new CPOB;
	POB->ID=POB_CLOSING;
	POB->Next1=0;
	POB->Next2=0;

	C1=Jacobi(C);

	Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
	Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
	Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;

	Ux.Normalise();
	Uy.Normalise();
	Uz.Normalise();
	
	POB->Ux=Ux;
	POB->Uy=Uy;
	POB->Uz=Uz;

	Lx=Ly=Lz=-10;
	mLx=mLy=mLz=10;
	sx=sy=sz=0;
	
	for (n=0;n<obj->nVertices;n++)
	{
		if (obj->Vertices[n].tag==1)
		{
			vect=obj->Vertices[n].Stok-O;
			sx=(vect||Ux);
			if (sx>Lx) Lx=sx;
			if (sx<mLx) mLx=sx;
			
			sy=(vect||Uy);
			if (sy>Ly) Ly=sy;
			if (sy<mLy) mLy=sy;
			
			sz=(vect||Uz);
			if (sz>Lz) Lz=sz;
			if (sz<mLz) mLz=sz;
		}
	}

	O+=((mLx+Lx)/2)*Ux +((mLy+Ly)/2)*Uy +((mLz+Lz)/2)*Uz;

	Lx=(Lx-mLx)/2;
	Ly=(Ly-mLy)/2;
	Lz=(Lz-mLz)/2;

	POB->Lx=Lx;
	POB->Ly=Ly;
	POB->Lz=Lz;
	POB->mLx=-Lx;
	POB->mLy=-Ly;
	POB->mLz=-Lz;

	POB->O=O;

	return POB;
}

CPOB * CPOBGenerator::POBFrom2F(CObject3D * obj,int tag,int NITER)
{
    int n,nn,j,k;
    CPOB * POB;
    float L,Lx,Ly,Lz,mLx,mLy,mLz,sx,sy,sz;
    CVector Mean,P,Q,R,M,Ux,Uy,Uz,O,vect,N;
    CMatrix C,C1;
    float s0,s1,s2;
    float a,b,c,d;
    CVector X,Y,Z;
    
    X.Init(1,0,0);
    Y.Init(0,1,0);
    Z.Init(0,0,1);
    
    Mean.Init(0,0,0);
    
    for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;
    
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            obj->Faces[n].v[0]->tag=1;
            obj->Faces[n].v[1]->tag=1;
            obj->Faces[n].v[2]->tag=1;
        }
    }

    CVector mini(10000,10000,10000);
    CVector maxi(-10000,-10000,-10000);
    
    nn=0;
    for (n=0;n<obj->nFaces;n++)
    {
        if (obj->Faces[n].tag==tag)
        {
            for (k=0;k<3;k++)
            {
                if (obj->Faces[n].v[k]->Stok.x>maxi.x) maxi.x=obj->Faces[n].v[k]->Stok.x;
                if (obj->Faces[n].v[k]->Stok.y>maxi.y) maxi.y=obj->Faces[n].v[k]->Stok.y;
                if (obj->Faces[n].v[k]->Stok.z>maxi.z) maxi.z=obj->Faces[n].v[k]->Stok.z;
                
                if (obj->Faces[n].v[k]->Stok.x<mini.x) mini.x=obj->Faces[n].v[k]->Stok.x;
                if (obj->Faces[n].v[k]->Stok.y<mini.y) mini.y=obj->Faces[n].v[k]->Stok.y;
                if (obj->Faces[n].v[k]->Stok.z<mini.z) mini.z=obj->Faces[n].v[k]->Stok.z;
            }
            nn++;
        }
    }
    Mean=(mini+maxi)/2;
    
    O=M=Mean;
    
    for (j=0;j<3;j++)
        for (k=0;k<3;k++)
        {
            C.a[j][k]=0;
            for (n=0;n<obj->nFaces;n++)
            {
                if (obj->Faces[n].tag==tag)
                {
                    P=obj->Faces[n].v[0]->Stok;
                    Q=obj->Faces[n].v[1]->Stok;
                    R=obj->Faces[n].v[2]->Stok;
                                        
                    C.a[j][k]+=(P.v[j]-M.v[j])*(P.v[k]-M.v[k]) + (Q.v[j]-M.v[j])*(Q.v[k]-M.v[k]) + (R.v[j]-M.v[j])*(R.v[k]-M.v[k]);
                }
            }
            C.a[j][k]=C.a[j][k]/(float) (3*nn);
        }
        
    POB=new CPOB;
    POB->ID=POB_CLOSING;
    POB->Next1=0;
    POB->Next2=0;
    
    C1=Jacobi(C);
    
    Ux=C1.a[0][0]*X+C1.a[0][1]*Y+C1.a[0][2]*Z;
    Uy=C1.a[1][0]*X+C1.a[1][1]*Y+C1.a[1][2]*Z;
    Uz=C1.a[2][0]*X+C1.a[2][1]*Y+C1.a[2][2]*Z;
    
    Ux.Normalise();
    Uy.Normalise();
    Uz.Normalise();
    
    POB->Ux=Ux;
    POB->Uy=Uy;
    POB->Uz=Uz;
    
    Lx=Ly=Lz=-10;
    mLx=mLy=mLz=10;
    sx=sy=sz=0;
    
    for (n=0;n<obj->nVertices;n++)
    {
        if (obj->Vertices[n].tag==1)
        {
            vect=obj->Vertices[n].Stok-O;
            sx=(vect||Ux);
            if (sx>Lx) Lx=sx;
            if (sx<mLx) mLx=sx;
            
            sy=(vect||Uy);
            if (sy>Ly) Ly=sy;
            if (sy<mLy) mLy=sy;
            
            sz=(vect||Uz);
            if (sz>Lz) Lz=sz;
            if (sz<mLz) mLz=sz;
        }
    }
    
    O+=((mLx+Lx)/2)*Ux +((mLy+Ly)/2)*Uy +((mLz+Lz)/2)*Uz;
    
    Lx=(Lx-mLx)/2;
    Ly=(Ly-mLy)/2;
    Lz=(Lz-mLz)/2;
    
    POB->Lx=Lx;
    POB->Ly=Ly;
    POB->Lz=Lz;
    POB->mLx=-Lx;
    POB->mLy=-Ly;
    POB->mLz=-Lz;
    
    POB->O=O;
    
    if ((Lx>Ly)&&(Lx>Lz)) { N=Ux; L=Lx; }
    if ((Ly>Lx)&&(Ly>Lz)) { N=Uy; L=Ly; }
    if ((Lz>Ly)&&(Lz>Lx)) { N=Uz; L=Lz; }

    if (NITER>0)
    {
        POB->ID=POB_NODE;
        
        a=N.x;
        b=N.y;
        c=N.z;
        d=-(a*Mean.x+b*Mean.y+c*Mean.z);
        
        for (n=0;n<obj->nFaces;n++)
        {
            if (obj->Faces[n].tag==tag)
            {
                P=obj->Faces[n].v[0]->Stok;
                Q=obj->Faces[n].v[1]->Stok;
                R=obj->Faces[n].v[2]->Stok;
                
                s0=a*P.x + b*P.y + c*P.z +d;
                s1=a*Q.x + b*Q.y + c*Q.z +d;
                s2=a*R.x + b*R.y + c*R.z +d;
                
                if (!((s0>0)&&(s1>0)&&(s2>0))) obj->Faces[n].tag++;                
            }
        }
        
        POB->Next1=POBFrom2F(obj,tag+1,NITER-1);
        
        for (n=0;n<obj->nFaces;n++)
            if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
        
        for (n=0;n<obj->nFaces;n++)
        {
            if (obj->Faces[n].tag==tag)
            {
                P=obj->Faces[n].v[0]->Stok;
                Q=obj->Faces[n].v[1]->Stok;
                R=obj->Faces[n].v[2]->Stok;
                
                s0=a*P.x + b*P.y + c*P.z +d;
                s1=a*Q.x + b*Q.y + c*Q.z +d;
                s2=a*R.x + b*R.y + c*R.z +d;
                
                
                if (!((s0<0)&&(s1<0)&&(s2<0))) obj->Faces[n].tag++;
                
            }
        }
        POB->Next2=POBFrom2F(obj,tag+1,NITER-1);
        
        for (n=0;n<obj->nFaces;n++)
            if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;
    }
    else
    {
        POB->ID=POB_CLOSING;
        POB->nPOB=nPOB;
        
        NITERATIONS=NITER;
        nPOB++;
    }
    
    return POB;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
 
		POB Calculation

		CPOB * CPOBGenerator::POBFrom(CObject3D * obj,int NITER)

		
	Usage:	

		main function : calculate hierarchir of oriented bounding boxes
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CPOB * CPOBGenerator::POBFrom(CObject3D * obj,int NITER)
{
#define Ni 5
	int n;
	CPOB * pob;
	CObject3D objs[Ni];
	float f;

	f=obj->CalculateRadius()/(4*Ni);

	objs[0]=obj->TesselateLength4(f);

	for (n=1;n<Ni;n++) objs[n]=objs[n-1].TesselateLength4(f);

	for (n=0;n<objs[Ni-1].nFaces;n++) objs[Ni-1].Faces[n].tag=0;

	for (n=0;n<Ni-1;n++) objs[n].Free();

	pob=POBFrom2(&objs[Ni-1],0,NITER);
	
	objs[Ni-1].Free();

	return pob;
}


void CPOB::InitCube(float lx,float ly,float lz)
{
	Ux.Init(1,0,0);
	Uy.Init(0,1,0);
	Uz.Init(0,0,1);

	O.Init(0,0,0);

	Lx=lx/2;
	Ly=ly/2;
	Lz=lz/2;

	mLx=-Lx;
	mLy=-Ly;
	mLz=-Lz;

	Next1=0;
	Next2=0;

	ID=POB_CLOSING;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		POB Calculation

		CPOB * CPOBGenerator::POBFrom1(CObject3D * obj,int tag,int NITER)

		
	Usage:	

		main function : calculate hierarchir of oriented bounding boxes
		one level of hierarchy
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */



CPOB * CPOBGenerator::POBFrom1(CObject3D * obj,int tag,int NITER)
{
	int n,nn;
	CVector P,Q,R,M;
	CPOB * pob;

	

	pob=new CPOB;

	pob->ID=POB_CLOSING;
	pob->Ux.Init(1,0,0);
	pob->Uy.Init(0,1,0);
	pob->Uz.Init(0,0,1);

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			P=obj->Faces[n].v[0]->Stok;
			Q=obj->Faces[n].v[1]->Stok;
			R=obj->Faces[n].v[2]->Stok;
			M=P+Q+R;
			nn+=3;
		}
	}

	

	M=M/(float) nn;

	pob->O=M;

	pob->Lx=pob->Ly=pob->Lz=-10000;
	pob->mLx=pob->mLy=pob->mLz=10000;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			P=obj->Faces[n].v[0]->Stok - M;
			Q=obj->Faces[n].v[1]->Stok - M;
			R=obj->Faces[n].v[2]->Stok - M;

			if (P.x>pob->Lx) pob->Lx=P.x;
			if (P.y>pob->Ly) pob->Ly=P.y;
			if (P.z>pob->Lz) pob->Lz=P.z;

			if (Q.x>pob->Lx) pob->Lx=Q.x;
			if (Q.y>pob->Ly) pob->Ly=Q.y;
			if (Q.z>pob->Lz) pob->Lz=Q.z;

			if (R.x>pob->Lx) pob->Lx=R.x;
			if (R.y>pob->Ly) pob->Ly=R.y;
			if (R.z>pob->Lz) pob->Lz=R.z;

			if (P.x<pob->mLx) pob->mLx=P.x;
			if (P.y<pob->mLy) pob->mLy=P.y;
			if (P.z<pob->mLz) pob->mLz=P.z;

			if (Q.x<pob->mLx) pob->mLx=Q.x;
			if (Q.y<pob->mLy) pob->mLy=Q.y;
			if (Q.z<pob->mLz) pob->mLz=Q.z;

			if (R.x<pob->mLx) pob->mLx=R.x;
			if (R.y<pob->mLy) pob->mLy=R.y;
			if (R.z<pob->mLz) pob->mLz=R.z;
		}
	}

	pob->O+=((pob->mLx+pob->Lx)/2)*pob->Ux +((pob->mLy+pob->Ly)/2)*pob->Uy +((pob->mLz+pob->Lz)/2)*pob->Uz;

	pob->Lx=(pob->Lx-pob->mLx)/2;
	pob->Ly=(pob->Ly-pob->mLy)/2;
	pob->Lz=(pob->Lz-pob->mLz)/2;

	pob->mLx=-pob->Lx;
	pob->mLy=-pob->Ly;
	pob->mLz=-pob->Lz;

	pob->Next1=0;
	pob->Next2=0;


	
	if (NITER>0)
	{
		pob->ID=POB_NODE;


		int nx,ny,nz;
		float tx,ty,tz;

		nx=ny=nz=0;

		for (n=0;n<obj->nFaces;n++)
		{
			if (obj->Faces[n].tag==tag)
			{
				P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
				if (P.x>M.x) nx++;
				if (P.y>M.y) ny++;
				if (P.z>M.z) nz++;
			}
		}

		
		tx=(float) nx/(obj->nFaces -nx);
		ty=(float) ny/(obj->nFaces -ny);
		tz=(float) nz/(obj->nFaces -nz);

		int split;

		if ((tx>ty)&&(tx>tz)) split=0;
		if ((ty>tx)&&(ty>tz)) split=1;
		if ((tz>ty)&&(tz>tx)) split=2;


		switch (split)
		{
		case 0:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.x>M.x) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		case 1:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.y>M.y) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		case 2:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.z>M.z) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		};

		

		pob->Next1=POBFrom1(obj,tag+1,NITER-1);

		for (n=0;n<obj->nFaces;n++)
			if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;


		switch (split)
		{
		case 0:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.x<M.x) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		case 1:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.y<M.y) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		case 2:
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					P=(obj->Faces[n].v[0]->Stok +obj->Faces[n].v[1]->Stok + obj->Faces[n].v[2]->Stok)/3;
					if (P.z<M.z) obj->Faces[n].tag=tag+1;
				}
			}
			break;
		};

		pob->Next2=POBFrom1(obj,tag+1,NITER-1);

		for (n=0;n<obj->nFaces;n++)
			if (obj->Faces[n].tag==tag+1) obj->Faces[n].tag--;

	}


	return pob;
}
