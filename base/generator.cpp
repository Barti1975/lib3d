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
//	@file generator.cpp 
////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

#include "params.h"

#include <math.h>
#include <stdio.h>
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "bsp_volumic.h"
#include "edges.h"
#include <string.h>

extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }

#ifdef _DEFINES_API_CODE_GENERATOR_

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		this reorder faces in temporary object of a generator

		void CGenerator::Tri(CVector L)

		L : base point to execute
				
	Usage:	
		
		
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::Tri(CVector L)
{
	int n,n1,n2;
	float f,ff;
	CVector u;
	CFace F;

	for (n=0;n<nFaces;n++)
	{
		u=Faces[n].v[0]->Stok - L;
		ff=u.Norme();
		u=Faces[n].v[1]->Stok - L;
		f=u.Norme();
		if (f>ff) ff=f;
		u=Faces[n].v[2]->Stok - L;
		f=u.Norme();
		if (f>ff) ff=f;
		Faces[n].coef1=ff;
	}


	for (n1=0;n1<nFaces;n1++)
		for (n2=n1+1;n2<nFaces;n2++)
		{
			if (Faces[n2].coef1<Faces[n1].coef1)
			{
				F=Faces[n1];
				Faces[n1]=Faces[n2];
				Faces[n2]=F;
			}
		}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		this reorder faces in temporary object of a generator

		void CGenerator::Tric(CVector L)

		L : base point to execute
				
	Usage:	
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::Tric(CVector L)
{
	int n,n1,n2;
	CVector u,p;
	CFace F;

	for (n=0;n<nFaces;n++)
	{
		p=(Faces[n].v[0]->Stok+Faces[n].v[1]->Stok+Faces[n].v[2]->Stok)/3;
		u=p-L;
		Faces[n].coef1=u.Norme();
	}

	for (n1=0;n1<nFaces;n1++)
		for (n2=n1+1;n2<nFaces;n2++)
		{
			if (Faces[n2].coef1<Faces[n1].coef1)
			{
				F=Faces[n1];
				Faces[n1]=Faces[n2];
				Faces[n2]=F;
			}
		}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		void CGenerator::TagCoplanarFaces(int nf,int ref,int TAG)

			set .tag of all faces coplanar with ref and with tag==0 to TAG

		void CGenerator::TagRecursNearToNear(int nf,int ref,int TAG)

			set .tag of near faces

		void CGenerator::TagObjetCoplanarFaces(CObject3D * obj,int nf,int ref,int TAG)

			set .tag of all faces coplanar with ref and with tag==0 to TAG
			obj extern


	Usage:	
		
		object traitment
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CGenerator::TagAllCoplanarFaces(int ref,int TAG,int limit)
{
	float ss;
	int n,cp;
	float d1,d2;

	DOTPRODUCT(d2,Faces[ref].Norm,Faces[ref].v[0]->Stok);

	cp=0;
	n=0;
	while ((n<nFaces)&&(cp<limit))
	{
		if ((n!=ref)&&(Faces[n].tag==0))
		{
			DOTPRODUCT(ss,Faces[n].Norm,Faces[ref].Norm);
			if (ss>1.0f-SMALLF2) // 0.995
			{
				DOTPRODUCT(d1,Faces[n].Norm,Faces[n].v[0]->Stok);
				if (f_abs(d2-d1)<SMALLF2)
				{
					Faces[n].tag=TAG;
					cp++;
				}
			}
			if (ss<-(1.0f-SMALLF2)) // 0.995
			{
				DOTPRODUCT(d1,Faces[n].Norm,Faces[n].v[0]->Stok);
				if (f_abs(d2+d1)<SMALLF2)
				{
					Faces[n].tag=TAG;
					cp++;
				}
			}
		}
		n++;
	}
	Faces[ref].tag=TAG;
}


void CGenerator::TagCoplanarFaces(int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;
	if (Faces[nf].tag==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm);
		if (ss>1.0f-SMALLF2)
		{
			Faces[nf].tag=TAG;
			f0=Face_01(nf);
			f1=Face_12(nf);
			f2=Face_20(nf);
			if (f0!=-1) TagCoplanarFaces(f0,ref,TAG);
			if (f1!=-1) TagCoplanarFaces(f1,ref,TAG);
			if (f2!=-1) TagCoplanarFaces(f2,ref,TAG);
		}
	}
}


void CGenerator::SetTopo()
{
	int n,nf;
	int res,tag;

	for (n=0;n<nFaces;n++)
	{
		Faces[n].f01=-2;
		Faces[n].f12=-2;
		Faces[n].f20=-2;
	}

	for (nf=0;nf<nFaces;nf++)
	{
		if (Faces[nf].f01==-2)
		{
			n=0;
			tag=-1;
			res=-1;
			while ((n<nFaces)&&(res==-1))
			{
				if (n!=nf)
				{
					if ((Faces[n].v0==Faces[nf].v1)&&(Faces[n].v1==Faces[nf].v0)) {res=n;tag=0;}
					if ((Faces[n].v1==Faces[nf].v1)&&(Faces[n].v2==Faces[nf].v0)) {res=n;tag=1;}
					if ((Faces[n].v2==Faces[nf].v1)&&(Faces[n].v0==Faces[nf].v0)) {res=n;tag=2;}
				}
				n++;
			}

			if (res>=0) 
			{
				Faces[nf].f01=res;
				if (tag==0) Faces[res].f01=nf;
				if (tag==1) Faces[res].f12=nf;
				if (tag==2) Faces[res].f20=nf;
			}
			else Faces[nf].f01=-1;
		}

		if (Faces[nf].f12==-2)
		{
			n=0;
			tag=-1;
			res=-1;
			while ((n<nFaces)&&(res==-1))
			{
				if (n!=nf)
				{
					if ((Faces[n].v0==Faces[nf].v2)&&(Faces[n].v1==Faces[nf].v1)) {res=n;tag=0;}
					if ((Faces[n].v1==Faces[nf].v2)&&(Faces[n].v2==Faces[nf].v1)) {res=n;tag=1;}
					if ((Faces[n].v2==Faces[nf].v2)&&(Faces[n].v0==Faces[nf].v1)) {res=n;tag=2;}
				}
				n++;
			}

			if (res>=0) 
			{
				Faces[nf].f12=res;
				if (tag==0) Faces[res].f01=nf;
				if (tag==1) Faces[res].f12=nf;
				if (tag==2) Faces[res].f20=nf;
			}
			else Faces[nf].f12=-1;
		}

		if (Faces[nf].f20==-2)
		{
			n=0;
			tag=-1;
			res=-1;
			while ((n<nFaces)&&(res==-1))
			{
				if (n!=nf)
				{
					if ((Faces[n].v0==Faces[nf].v0)&&(Faces[n].v1==Faces[nf].v2)) {res=n;tag=0;}
					if ((Faces[n].v1==Faces[nf].v0)&&(Faces[n].v2==Faces[nf].v2)) {res=n;tag=1;}
					if ((Faces[n].v2==Faces[nf].v0)&&(Faces[n].v0==Faces[nf].v2)) {res=n;tag=2;}
				}
				n++;
			}

			if (res>=0) 
			{
				Faces[nf].f20=res;
				if (tag==0) Faces[res].f01=nf;
				if (tag==1) Faces[res].f12=nf;
				if (tag==2) Faces[res].f20=nf;
			}
			else Faces[nf].f20=-1;
		}

	}
}

void CGenerator::TagCoplanarFacesTopo(int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;
	if (Faces[nf].tag==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm);
		if (ss>1.0f-SMALLF2)
		{
			Faces[nf].tag=TAG;
			f0=Faces[nf].f01;
			f1=Faces[nf].f12;
			f2=Faces[nf].f20;
			if (f0!=-1) TagCoplanarFacesTopo(f0,ref,TAG);
			if (f1!=-1) TagCoplanarFacesTopo(f1,ref,TAG);
			if (f2!=-1) TagCoplanarFacesTopo(f2,ref,TAG);
		}
	}
}


void CGenerator::TagALLRecCoplanarFaces(int nf,int ref,int TAG,int limit)
{
	float ss;
	int f0,f1,f2;

	if (Faces[nf].tag==0)
	{
		DOTPRODUCT(ss,Faces[nf].Norm,Faces[ref].Norm);
		if (ss>1.0f-SMALLF2)
		{
			Faces[nf].tag=TAG;
			f0=Face_01(nf);
			f1=Face_12(nf);
			f2=Face_20(nf);
			if (f0!=-1) TagALLRecCoplanarFaces(f0,ref,TAG,limit);
			if (f1!=-1) TagALLRecCoplanarFaces(f1,ref,TAG,limit);
			if (f2!=-1) TagALLRecCoplanarFaces(f2,ref,TAG,limit);
		}
	}
}


void CGenerator::TagObjetCoplanarFaces(CObject3D * obj,int nf,int ref,int TAG)
{
	float ss;
	int f0,f1,f2;

	

	if (obj->Faces[nf].tag==0)
	{
		DOTPRODUCT(ss,obj->Faces[nf].Norm,obj->Faces[ref].Norm);

		if (ss>1.0f-SMALLF2)
		{
			obj->Faces[nf].tag=TAG;
			f0=obj->Face_01(nf);
			f1=obj->Face_12(nf);
			f2=obj->Face_20(nf);

			if (f0!=-1) TagObjetCoplanarFaces(obj,f0,ref,TAG);
			if (f1!=-1) TagObjetCoplanarFaces(obj,f1,ref,TAG);
			if (f2!=-1) TagObjetCoplanarFaces(obj,f2,ref,TAG);
		}
	}
}



void CGenerator::TagRecursNearToNear(int nf,int ref,int TAG)
{
	int f0,f1,f2;

	if (Faces[nf].tag==0)
	{
		Faces[nf].tag=TAG;
		f0=Face_01(nf);
		f1=Face_12(nf);
		f2=Face_20(nf);

		if (f0!=-1) TagRecursNearToNear(f0,ref,TAG);
		if (f1!=-1) TagRecursNearToNear(f1,ref,TAG);
		if (f2!=-1) TagRecursNearToNear(f2,ref,TAG);
	}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		get the temporary object stored in generator
		
		CObject3D * CGenerator::pResObjet()
				
	Usage:	
		
		
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

CObject3D * CGenerator::pResObjet()
{
	CObject3D * duplicated;
	int n;

	duplicated=new CObject3D;
	duplicated->Init(nVertices,nFaces);

	for (n=0;n<nVertices;n++)
	{
		duplicated->Vertices[n].Stok=Vertices[n].Stok;
		duplicated->Vertices[n].Norm=Vertices[n].Norm;
        duplicated->Vertices[n].Map=Vertices[n].Map;
        //duplicated->Vertices[n].Map2=Vertices[n].Map2;
        duplicated->Vertices[n].tag=Vertices[n].tag;
        duplicated->Vertices[n].tag2=Vertices[n].tag2;
        duplicated->Vertices[n].temp=Vertices[n].temp;
		duplicated->Vertices[n].Diffuse=Vertices[n].Diffuse;
		duplicated->Vertices[n].Index[0]=Vertices[n].Index[0];
		duplicated->Vertices[n].Index[1]=Vertices[n].Index[1];
		duplicated->Vertices[n].Index[2]=Vertices[n].Index[2];
		duplicated->Vertices[n].Index[3]=Vertices[n].Index[3];
		duplicated->Vertices[n].Weight[0]=Vertices[n].Weight[0];
		duplicated->Vertices[n].Weight[1]=Vertices[n].Weight[1];
		duplicated->Vertices[n].Weight[2]=Vertices[n].Weight[2];
		duplicated->Vertices[n].Weight[3]=Vertices[n].Weight[3];
	}

	for (n=0;n<nFaces;n++)
	{
		duplicated->Faces[n].v0=Faces[n].v0;
		duplicated->Faces[n].v1=Faces[n].v1;
		duplicated->Faces[n].v2=Faces[n].v2;
		duplicated->Faces[n].Norm=Faces[n].Norm;
		duplicated->Faces[n].tag=Faces[n].tag;
		duplicated->Faces[n].tag2=Faces[n].tag2;
		duplicated->Faces[n].nT=Faces[n].nT;
		duplicated->Faces[n].nT2=Faces[n].nT2;
		duplicated->Faces[n].ref=Faces[n].ref;
		duplicated->Faces[n].nL=Faces[n].nL;
		duplicated->Faces[n].nLVectors=Faces[n].nLVectors;
		//duplicated->Faces[n].Diffuse=Faces[n].Diffuse;
		duplicated->Faces[n].mp0=Faces[n].mp0;
		duplicated->Faces[n].mp1=Faces[n].mp1;
		duplicated->Faces[n].mp2=Faces[n].mp2;
		duplicated->Faces[n].size_lightmap=Faces[n].size_lightmap;
	}

	duplicated->Coo.Init(0,0,0);
	duplicated->Rot.Init(0,0,0);
	duplicated->SetFaces();

	return duplicated;
}

CObject3D * CGenerator::pResObjetMap2()
{
    CObject3D * duplicated;
    int n;

    duplicated=new CObject3D;
    duplicated->Init(nVertices,nFaces);

    for (n=0;n<nVertices;n++)
    {
        duplicated->Vertices[n].Stok=Vertices[n].Stok;
        duplicated->Vertices[n].Norm=Vertices[n].Norm;
        duplicated->Vertices[n].Map=Vertices[n].Map;
        duplicated->Vertices[n].Map2=Vertices[n].Map2;
        duplicated->Vertices[n].tag=Vertices[n].tag;
        duplicated->Vertices[n].tag2=Vertices[n].tag2;
        duplicated->Vertices[n].temp=Vertices[n].temp;
        duplicated->Vertices[n].Diffuse=Vertices[n].Diffuse;
    }

    for (n=0;n<nFaces;n++)
    {
        duplicated->Faces[n].v0=Faces[n].v0;
        duplicated->Faces[n].v1=Faces[n].v1;
        duplicated->Faces[n].v2=Faces[n].v2;
        duplicated->Faces[n].Norm=Faces[n].Norm;
        duplicated->Faces[n].tag=Faces[n].tag;
        duplicated->Faces[n].tag2=Faces[n].tag2;
        duplicated->Faces[n].nT=Faces[n].nT;
        duplicated->Faces[n].nT2=Faces[n].nT2;
        duplicated->Faces[n].ref=Faces[n].ref;
        duplicated->Faces[n].nL=Faces[n].nL;
        duplicated->Faces[n].nLVectors=Faces[n].nLVectors;
        //duplicated->Faces[n].Diffuse=Faces[n].Diffuse;
        duplicated->Faces[n].mp0=Faces[n].mp0;
        duplicated->Faces[n].mp1=Faces[n].mp1;
        duplicated->Faces[n].mp2=Faces[n].mp2;
        duplicated->Faces[n].size_lightmap=Faces[n].size_lightmap;
    }

    duplicated->Coo.Init(0,0,0);
    duplicated->Rot.Init(0,0,0);
    duplicated->SetFaces();

    return duplicated;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

				
	Usage:	
		
		
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddWithOptimization(CObject3D * obj)
{
	int n,nn;
	int res,tag;

	
	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag2=0;

	res=0;
	tag=1;
	while (res==0)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag2==0) nn=n;
			n++;
		}

		if (nn==-1) res=1;
		else
		{
			obj->TagCoplanarFaces2(nn,nn,tag);
	
			// à faire





		}

		tag++;
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		retrieve face on side 01 12 20

		int CGenerator::Faces_01(int nf,...)
		int CGenerator::Faces_12(int nf,...)
		int CGenerator::Faces_20(int nf,...)
  
	Usage:	

		traitement
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
bool CGenerator::EdgeConfondue(int na1,int nb1,int na2,int nb2)
{
	int res;
	CVector u1,u2,i1,i2,u;
	float d;
	float sa1,sb1,sa2,sb2;
	float t1,t2;
	CVector a1=Vertices[na1].Stok;
	CVector b1=Vertices[nb1].Stok;
	CVector a2=Vertices[na2].Stok;
	CVector b2=Vertices[nb2].Stok;
	CVector p;

	res=0;
	u1=b1-a1;
	u2=b2-a2;
	u1.Normalise();
	u2.Normalise();
	if (f_abs(u1||u2)>1.0f-SMALLF2)
	{
		d=-(u1||a1);
		sa1=(u1||a1)+d;
		sb1=(u1||b1)+d;
		sa2=(u1||a2)+d;
		sb2=(u1||b2)+d;
		t1=-sa1/(sb1-sa1);
		t2=-sa2/(sb2-sa2);
		i1=a1+t1*(b1-a1);
		i2=a2+t2*(b2-a2);
		u=i2-i1;
		if (u.Norme()<SMALLF3) res=1;
	}

	return (res==1);
}


int CGenerator::Faces_01(int nf,int * faces_array,int *nb)
{
	int n,res;
	CFace * f;

	*nb=0;
	f=&Faces[nf];
	n=0;
	res=-1;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			res=-1;
			if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) res=n;
			if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) res=n;
			if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) res=n;

			if (EdgeConfondue(Faces[n].v0,Faces[n].v1,f->v1,f->v0)) res=n;
			if (EdgeConfondue(Faces[n].v1,Faces[n].v2,f->v1,f->v0)) res=n;
			if (EdgeConfondue(Faces[n].v2,Faces[n].v0,f->v1,f->v0)) res=n;

			if (res>=0)
			{
				faces_array[(*nb)]=res;
				(*nb)++;
			}
		}
		n++;
	}
	return res;
}

int CGenerator::Faces_12(int nf,int * faces_array,int *nb)
{
	int n,res;
	CFace * f;

	*nb=0;
	f=&Faces[nf];
	n=0;
	res=-1;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			res=-1;

			if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) res=n;
			if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) res=n;
			if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) res=n;

			if (EdgeConfondue(Faces[n].v0,Faces[n].v1,f->v2,f->v1)) res=n;
			if (EdgeConfondue(Faces[n].v1,Faces[n].v2,f->v2,f->v1)) res=n;
			if (EdgeConfondue(Faces[n].v2,Faces[n].v0,f->v2,f->v1)) res=n;
			
			if (res>=0)
			{
				faces_array[(*nb)]=res;
				(*nb)++;
			}
		}
		n++;
	}
	return res;
}

int CGenerator::Faces_20(int nf,int * faces_array,int *nb)
{
	int n,res;
	CFace * f;

	*nb=0;
	f=&Faces[nf];
	n=0;
	res=-1;
	while (n<nFaces)
	{
		if (n!=nf)
		{
			res=-1;

			if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) res=n;
			if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) res=n;
			if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) res=n;

			if (EdgeConfondue(Faces[n].v0,Faces[n].v1,f->v0,f->v2)) res=n;
			if (EdgeConfondue(Faces[n].v1,Faces[n].v2,f->v0,f->v2)) res=n;
			if (EdgeConfondue(Faces[n].v2,Faces[n].v0,f->v0,f->v2)) res=n;

			if (res>=0)
			{
				faces_array[(*nb)]=res;
				(*nb)++;
			}
		}
		n++;
	}
	return res;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		retrieve face on side 01 12 20

		int CGenerator::Face_01(int nf)
		int CGenerator::Face_12(int nf)
		int CGenerator::Face_20(int nf)
  
	Usage:	

		traitement
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


int CGenerator::Face_01(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n;
			if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n;
			if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n;
		}
		n++;
	}
	return -1;
}

int CGenerator::Face_12(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n;
			if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n;
			if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n;
		}
		n++;
	}
	return -1;
}

int CGenerator::Face_20(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	while (n<nFaces)
	{
		if (n!=nf)
		{
			if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n;
			if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n;
			if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n;
		}
		n++;
	}
	return -1;
}


int CGenerator::iFace_01(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	int nf0=Face_01(nf);
	while (n<nFaces)
	{
		if ((n!=nf)&&(n!=nf0))
		{
			if ((Faces[n].v0==f->v1)&&(Faces[n].v1==f->v0)) return n;
			if ((Faces[n].v1==f->v1)&&(Faces[n].v2==f->v0)) return n;
			if ((Faces[n].v2==f->v1)&&(Faces[n].v0==f->v0)) return n;
		}
		n++;
	}
	return -1;
}

int CGenerator::iFace_12(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	int nf0=Face_12(nf);
	while (n<nFaces)
	{
		if ((n!=nf)&&(n!=nf0))
		{
			if ((Faces[n].v0==f->v2)&&(Faces[n].v1==f->v1)) return n;
			if ((Faces[n].v1==f->v2)&&(Faces[n].v2==f->v1)) return n;
			if ((Faces[n].v2==f->v2)&&(Faces[n].v0==f->v1)) return n;
		}
		n++;
	}
	return -1;
}

int CGenerator::iFace_20(int nf)
{
	int n=0;
	CFace * f=&Faces[nf];
	int nf0=Face_20(nf);
	while (n<nFaces)
	{
		if ((n!=nf)&&(n!=nf0))
		{
			if ((Faces[n].v0==f->v0)&&(Faces[n].v1==f->v2)) return n;
			if ((Faces[n].v1==f->v0)&&(Faces[n].v2==f->v2)) return n;
			if ((Faces[n].v2==f->v0)&&(Faces[n].v0==f->v2)) return n;
		}
		n++;
	}
	return -1;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one with merge of close vertex
		
		void CGenerator::AddMinimizedVerticesIndexing(CObject3D * obj,float scale)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool FaceNormal(CFace * f,CVertex *vertices)
{
	CVector vec1,vec2,vec3;
	bool res=true;

	VECTORSUB(vec1,vertices[f->v1].Stok,vertices[f->v0].Stok);
	VECTORSUB(vec2,vertices[f->v2].Stok,vertices[f->v0].Stok);
	if (VECTORNORM(vec1)<0.001f) res=false;
	if (VECTORNORM(vec2)<0.001f) res=false;
	CROSSPRODUCT(vec3,vec2,vec1);
	VECTORNORMALISE(vec3);
	if (VECTORNORM(vec3)<0.001f) res=false;

	if (!res)
	{
		res=true;
		VECTORSUB(vec1,vertices[f->v2].Stok,vertices[f->v1].Stok);
		VECTORSUB(vec2,vertices[f->v0].Stok,vertices[f->v1].Stok);
		if (VECTORNORM(vec1)<0.001f) res=false;
		if (VECTORNORM(vec2)<0.001f) res=false;
		CROSSPRODUCT(vec3,vec2,vec1);
		VECTORNORMALISE(vec3);
		if (VECTORNORM(vec3)<0.001f) res=false;
	}

	if (!res)
	{
		res=true;
		VECTORSUB(vec1,vertices[f->v0].Stok,vertices[f->v2].Stok);
		VECTORSUB(vec2,vertices[f->v1].Stok,vertices[f->v2].Stok);
		if (VECTORNORM(vec1)<0.001f) res=false;
		if (VECTORNORM(vec2)<0.001f) res=false;
		CROSSPRODUCT(vec3,vec2,vec1);
		VECTORNORMALISE(vec3);
		if (VECTORNORM(vec3)<0.001f) res=false;
	}

	if (res) f->Norm=vec3;

	return res;
}

bool CGenerator::FacePresent(int v0,int v1,int v2)
{
	int n=0;
    int st=0;
    if (level>0) st=level;

    for (n=st;n<nFaces;n++)
	{
		if ( ((Faces[n].v0==v0)&&(Faces[n].v1==v1)&&(Faces[n].v2==v2)) || ((Faces[n].v0==v1)&&(Faces[n].v1==v2)&&(Faces[n].v2==v0))
			|| ((Faces[n].v0==v2)&&(Faces[n].v1==v0)&&(Faces[n].v2==v1)) ) return true;
	}
	return false;
}

int CGenerator::FaceWithEdges(int a,int b)
{
    int n;
    int st=0;
    if (level>0) st=level;

    for (n=st;n<nFaces;n++)
    {
        if ((Faces[n].v0==a)&&(Faces[n].v1==b)) return n;
        if ((Faces[n].v1==a)&&(Faces[n].v2==b)) return n;
        if ((Faces[n].v2==a)&&(Faces[n].v0==b)) return n;
    }
    
    return -1;
}

int CGenerator::nbFaceWithEdges(int a,int b)
{
    int n;
    int nb=0;
    int st=0;
    if (level>0) st=level;
    
    for (n=st;n<nFaces;n++)
    if (Faces[n].tag==1)
    {
        if ((Faces[n].v0==a)&&(Faces[n].v1==b)) nb++;
        if ((Faces[n].v1==a)&&(Faces[n].v2==b)) nb++;
        if ((Faces[n].v2==a)&&(Faces[n].v0==b)) nb++;
    }
    
    return nb;
}

int CGenerator::FaceWithEdges(int a,int b,int nf)
{
    int n;
    int st=0;
    if (level>0) st=level;

    for (n=st;n<nFaces;n++)
    if (n!=nf)
    {
        if ((Faces[n].v0==a)&&(Faces[n].v1==b)) return n;
        if ((Faces[n].v1==a)&&(Faces[n].v2==b)) return n;
        if ((Faces[n].v2==a)&&(Faces[n].v0==b)) return n;
    }
    
    return -1;
}


void CGenerator::AddMinimizedVerticesIndexing(CObject3D * obj,float scale)
{
	int n,nn;
	int c;
	int * corres;
	CVector u;
	CObject3D * tmp;

	obj->nKeys=0;

	tmp=obj->ConvertLinearSimple(scale/SMALLF);

	tmp->Calculate();

	corres=(int *) malloc(tmp->nVertices*4);

	for (n=0;n<tmp->nVertices;n++)
	{
		corres[n]=-1;
		for (nn=0;nn<nVertices;nn++)
		{
			VECTORSUB(u,tmp->Vertices[n].Calc,Vertices[nn].Stok);
			if (VECTORNORM(u)<scale)
			{
				corres[n]=nn;
			}
		}
	}

	nn=0;
	for (n=0;n<tmp->nVertices;n++)
	{
		if (corres[n]==-1)
		{
			Vertices[nVertices+nn].Stok=tmp->Vertices[n].Calc;
			Vertices[nVertices+nn].Norm=tmp->Vertices[n].NormCalc;
			Vertices[nVertices+nn].Map=tmp->Vertices[n].Map;
			Vertices[nVertices+nn].Map2=tmp->Vertices[n].Map2;
			Vertices[nVertices+nn].Diffuse=tmp->Vertices[n].Diffuse;
			tmp->Vertices[n].tag2=nVertices+nn;
			nn++;
		}

	}
	nVertices+=nn;


	nn=0;
	for (n=0;n<tmp->nFaces;n++)
	{
		c=tmp->Faces[n].v0;
		if (corres[c]==-1) Faces[nFaces+nn].v0=tmp->Faces[n].v[0]->tag2;
		else Faces[nFaces+nn].v0=corres[c];

		c=tmp->Faces[n].v1;
		if (corres[c]==-1) Faces[nFaces+nn].v1=tmp->Faces[n].v[1]->tag2;
		else Faces[nFaces+nn].v1=corres[c];

		c=tmp->Faces[n].v2;
		if (corres[c]==-1) Faces[nFaces+nn].v2=tmp->Faces[n].v[2]->tag2;
		else Faces[nFaces+nn].v2=corres[c];

		FaceNormal(&Faces[nFaces+nn],Vertices);
		
		if ((Faces[nFaces+nn].Norm||tmp->Faces[n].NormCalc)>=0)
		{
			if ((Faces[nFaces+nn].v0!=Faces[nFaces+nn].v1)&&(Faces[nFaces+nn].v0!=Faces[nFaces+nn].v2)&&(Faces[nFaces+nn].v1!=Faces[nFaces+nn].v2))
			{
				Faces[nFaces+nn].Norm=tmp->Faces[n].NormCalc;
				Faces[nFaces+nn].tag=0;
				Faces[nFaces+nn].nT=tmp->Faces[n].nT;
				//Faces[nFaces+nn].Diffuse=tmp->Faces[n].Diffuse;
				Faces[nFaces+nn].mp0.Init(0,0);
				Faces[nFaces+nn].mp1.Init(0,0);
				Faces[nFaces+nn].mp2.Init(0,0);
				nn++;
			}
		}
	}

	nFaces+=nn;		

	tmp->Free();
	delete tmp;

	/*
	for (n=0;n<obj->nVertices;n++)
	{
		corres[n]=-1;
		for (nn=0;nn<n;nn++)
		{
			u=obj->Vertices[n].Stok - obj->Vertices[nn].Stok;
			if (u.Norme()<scale)
			{
				corres[n]=nn;
				break;
			}
		}

		for (nn=0;nn<nVertices;nn++)
		{
			u=obj->Vertices[n].Calc - Vertices[nn].Stok;
			if (u.Norme()<scale)
			{
				corres[n]=65536+nn;
				break;
			}
		}
	}

	nn=0;
	for (n=0;n<obj->nVertices;n++)
	{
		if (corres[n]==-1)
		{
			Vertices[nVertices+nn].Stok=obj->Vertices[n].Calc;
			Vertices[nVertices+nn].Norm=obj->Vertices[n].NormCalc;
			Vertices[nVertices+nn].Map=obj->Vertices[n].Map;
			Vertices[nVertices+nn].Map2=obj->Vertices[n].Map2;
			Vertices[nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
			obj->Vertices[n].tag2=nVertices+nn;
			nn++;
		}

	}
	nVertices+=nn;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{

		c=obj->Faces[n].v0;
		if (corres[c]==-1)
		{
			Faces[nFaces+nn].v0=obj->Faces[n].v[0]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				Faces[nFaces+nn].v0=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					Faces[nFaces+nn].v0=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					Faces[nFaces+nn].v0=(cc&0xFFFF);
				}
			}
		}

		c=obj->Faces[n].v1;
		if (corres[c]==-1)
		{
			Faces[nFaces+nn].v1=obj->Faces[n].v[1]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				Faces[nFaces+nn].v1=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					Faces[nFaces+nn].v1=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					Faces[nFaces+nn].v1=(cc&0xFFFF);
				}
			}
		}

		c=obj->Faces[n].v2;
		if (corres[c]==-1)
		{
			Faces[nFaces+nn].v2=obj->Faces[n].v[2]->tag2;
		}
		else
		{
			if (corres[c]>=65536)
			{
				Faces[nFaces+nn].v2=(corres[c]&0xFFFF);
			}
			else
			{
				cc=corres[corres[c]];
				if (cc==-1)
				{
					Faces[nFaces+nn].v2=obj->Vertices[corres[c]].tag2;
				}
				else
				{
					Faces[nFaces+nn].v2=(cc&0xFFFF);
				}
			}
		}

		FaceNormal(&Faces[nFaces+nn],Vertices);
		
		if ((Faces[nFaces+nn].Norm||obj->Faces[n].NormCalc)>=0)
		{
			if ((Faces[nFaces+nn].v0!=Faces[nFaces+nn].v1)&&(Faces[nFaces+nn].v0!=Faces[nFaces+nn].v2)&&(Faces[nFaces+nn].v1!=Faces[nFaces+nn].v2))
			{
				Faces[nFaces+nn].Norm=obj->Faces[n].NormCalc;
				Faces[nFaces+nn].tag=0;
				Faces[nFaces+nn].nT=obj->Faces[n].nT;

				Faces[nFaces+nn].Diffuse=obj->Faces[n].Diffuse;
				Faces[nFaces+nn].mp0.Init(0,0);
				Faces[nFaces+nn].mp1.Init(0,0);
				Faces[nFaces+nn].mp2.Init(0,0);
				nn++;
			}
		}
	}

	nFaces+=nn;		
*/
	free(corres);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one with reallocation of vertex due to face mapping
		
		void CGenerator::AddMinimized(CObject3D * obj)

	Usage:	

		wavefront, etc
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CGenerator::AddMinimized(CObject3D * obj)
{
	int n,nn;
	CVector u;

	nn=0;
	for (n=0;n<obj->nVertices;n++)
	{
		Vertices[nVertices+nn].Stok=obj->Vertices[n].Calc;
		Vertices[nVertices+nn].Norm=obj->Vertices[n].NormCalc;
		Vertices[nVertices+nn].Map=obj->Vertices[n].Map;
		Vertices[nVertices+nn].Map2=obj->Vertices[n].Map2;
		Vertices[nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
		obj->Vertices[n].tag2=nVertices+nn;
		nn++;
	}
	nVertices+=nn;

	nn=0;
	for (n=0;n<obj->nFaces;n++)
	{
		Faces[nFaces+nn].v0=obj->Faces[n].v[0]->tag2;
		Faces[nFaces+nn].v1=obj->Faces[n].v[1]->tag2;
		Faces[nFaces+nn].v2=obj->Faces[n].v[2]->tag2;
		
		FaceNormal(&Faces[nFaces+nn],Vertices);
		
		if ((Faces[nFaces+nn].Norm||obj->Faces[n].NormCalc)>=0)
		{
			if ((Faces[nFaces+nn].v0!=Faces[nFaces+nn].v1)&&(Faces[nFaces+nn].v0!=Faces[nFaces+nn].v2)&&(Faces[nFaces+nn].v1!=Faces[nFaces+nn].v2))
			{
				Faces[nFaces+nn].Norm=obj->Faces[n].NormCalc;
				Faces[nFaces+nn].tag=0;
				Faces[nFaces+nn].nT=obj->Faces[n].nT;
				//Faces[nFaces+nn].Diffuse=obj->Faces[n].Diffuse;
				Faces[nFaces+nn].mp0.Init(0,0);
				Faces[nFaces+nn].mp1.Init(0,0);
				Faces[nFaces+nn].mp2.Init(0,0);
				nn++;
			}
		}
	}

	nFaces+=nn;		
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one with reallocation of vertex due to face mapping
		
		void CGenerator::AddConvertingFaceMappingToPerVertexMapping(CObject3D * obj)

	Usage:	

		wavefront, etc
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddConvertingFaceMappingToPerVertexMapping(CObject3D * obj)
{
	int n,nn,res,tag,d;

	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
	res=0;
	tag=1;

	obj->SetFaces();
	obj->CalculateNormals(-1);

	while (res==0)
	{
		n=0;
		d=-1;
		while ((n<obj->nFaces)&&(d==-1))
		{
			if (obj->Faces[n].tag==0) d=n;
			n++;
		}

		if (d==-1) res=1;
		else
		{
			TagObjetCoplanarFaces(obj,d,d,tag);

//			obj->Faces[d].tag=tag;

			for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					if (obj->Faces[n].v[0]->tag==0)
					{
						obj->Faces[n].v[0]->Map=obj->Faces[n].mp0;
						obj->Faces[n].v[0]->tag=1;
					}

					if (obj->Faces[n].v[1]->tag==0)
					{
						obj->Faces[n].v[1]->Map=obj->Faces[n].mp1;
						obj->Faces[n].v[1]->tag=1;
					}

					if (obj->Faces[n].v[2]->tag==0)
					{
						obj->Faces[n].v[2]->Map=obj->Faces[n].mp2;
						obj->Faces[n].v[2]->tag=1;
					}
				}
			}

			nn=0;

			for (n=0;n<obj->nVertices;n++)
			{
				if (obj->Vertices[n].tag==1)
				{
					Vertices[nVertices+nn].Stok=obj->Vertices[n].Stok;
					Vertices[nVertices+nn].Norm=obj->Vertices[n].Norm;
					Vertices[nVertices+nn].Map=obj->Vertices[n].Map;
					Vertices[nVertices+nn].Map2=obj->Vertices[n].Map2;
					Vertices[nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
					obj->Vertices[n].tag2=nVertices+nn;
					nn++;
				}
			}

			nVertices+=nn;

			nn=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag==tag)
				{
					Faces[nFaces+nn].v0=obj->Faces[n].v[0]->tag2;
					Faces[nFaces+nn].v1=obj->Faces[n].v[1]->tag2;
					Faces[nFaces+nn].v2=obj->Faces[n].v[2]->tag2;

					Faces[nFaces+nn].Norm=obj->Faces[n].Norm;
					Faces[nFaces+nn].tag=0;
					Faces[nFaces+nn].nT=obj->Faces[n].nT;

					//Faces[nFaces+nn].Diffuse=obj->Faces[n].Diffuse;
					Faces[nFaces+nn].mp0.Init(0,0);
					Faces[nFaces+nn].mp1.Init(0,0);
					Faces[nFaces+nn].mp2.Init(0,0);
					nn++;
				}
			}

			nFaces+=nn;
			tag++;
		}
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		
		void CGenerator::Add(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::Add(CObject3D * obj)
{
	int n;

	for (n=0;n<obj->nVertices;n++)
	{
        if (nVertices+n<maxnv)
        {
            Vertices[nVertices+n].Stok=obj->Vertices[n].Stok;
            Vertices[nVertices+n].Norm=obj->Vertices[n].Norm;
            Vertices[nVertices+n].Map=obj->Vertices[n].Map;
            Vertices[nVertices+n].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+n].tag=obj->Vertices[n].tag;
            Vertices[nVertices+n].tag2=obj->Vertices[n].tag2;
            Vertices[nVertices+n].temp=obj->Vertices[n].temp;
            Vertices[nVertices+n].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+n].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+n].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+n].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+n].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+n].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+n].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+n].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+n].Weight[3]=obj->Vertices[n].Weight[3];
        }
	}

	for (n=0;n<obj->nFaces;n++)
	{
        if (nFaces+n<maxnf)
        {
            Faces[nFaces+n].v0=obj->Faces[n].v0+nVertices;
            Faces[nFaces+n].v1=obj->Faces[n].v1+nVertices;
            Faces[nFaces+n].v2=obj->Faces[n].v2+nVertices;
            Faces[nFaces+n].Norm=obj->Faces[n].Norm;
            Faces[nFaces+n].tag=obj->Faces[n].tag;
            Faces[nFaces+n].nT=obj->Faces[n].nT;
            Faces[nFaces+n].nL=obj->Faces[n].nL;
            Faces[nFaces+n].nLVectors=obj->Faces[n].nLVectors;
            Faces[nFaces+n].nT2=obj->Faces[n].nT2;
            Faces[nFaces+n].ref=obj->Faces[n].ref;
            Faces[nFaces+n].size_lightmap=obj->Faces[n].size_lightmap;
            //Faces[nFaces+n].Diffuse=obj->Faces[n].Diffuse;
            Faces[nFaces+n].mp0=obj->Faces[n].mp0;
            Faces[nFaces+n].mp1=obj->Faces[n].mp1;
            Faces[nFaces+n].mp2=obj->Faces[n].mp2;
        }
	}

	nFaces+=obj->nFaces;
	nVertices+=obj->nVertices;
    
    if (nVertices>maxnv) nVertices=maxnv;
    if (nFaces>maxnf) nFaces=maxnf;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		
		void CGenerator::AddPlanar(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddPlanar(CObject3D * obj)
{
	int n,nn;
	int tag;
	bool test;
	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag2=0;

	tag=1;
	test=true;
	while (test)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag2==0) nn=n;
			else
				n++;
		}

		if (nn==-1) test=false;
		else
		{
			obj->Tag2CoplanarOnlyFaces(nn,tag);
			obj->Faces[nn].tag2=tag;

			for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag2==tag)
				{
					obj->Faces[n].v[0]->tag=1;
					obj->Faces[n].v[1]->tag=1;
					obj->Faces[n].v[2]->tag=1;
				}
			}

			for (n=0;n<obj->nVertices;n++)
			{
				if (obj->Vertices[n].tag)
				{
					Vertices[nVertices].Stok=obj->Vertices[n].Stok;
					Vertices[nVertices].Norm=obj->Vertices[n].Norm;
					Vertices[nVertices].Map=obj->Vertices[n].Map;
					Vertices[nVertices].Map2=obj->Vertices[n].Map2;
					Vertices[nVertices].Diffuse=obj->Vertices[n].Diffuse;
					obj->Vertices[n].tag2=nVertices;
					nVertices++;
				}
			}

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag2==tag)
				{
					Faces[nFaces]=obj->Faces[n];
					Faces[nFaces].v0=obj->Faces[n].v[0]->tag2;
					Faces[nFaces].v1=obj->Faces[n].v[1]->tag2;
					Faces[nFaces].v2=obj->Faces[n].v[2]->tag2;
					nFaces++;
				}
			}

			tag++;
			test=true;
		}
	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		
		void CGenerator::AddPlanarMod(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddPlanarMod(CObject3D * obj)
{
	int n,nn;
	int tag;
	bool test;
	float x,y;
	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag2=0;

	tag=1;
	test=true;
	while (test)
	{
		n=0;
		nn=-1;
		while ((n<obj->nFaces)&&(nn==-1))
		{
			if (obj->Faces[n].tag2==0) nn=n;
			else
				n++;
		}

		if (nn==-1) test=false;
		else
		{
			//obj->Tag2CoplanarOnlyFaces(nn,tag);
			obj->Faces[nn].tag2=tag;

			for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

			x=10000000;
			y=10000000;

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag2==tag)
				{
					obj->Faces[n].v[0]->tag=1;
					obj->Faces[n].v[1]->tag=1;
					obj->Faces[n].v[2]->tag=1;

					if (obj->Faces[n].v[0]->Map.x<x) x=obj->Faces[n].v[0]->Map.x;
					if (obj->Faces[n].v[0]->Map.y<y) y=obj->Faces[n].v[0]->Map.y;

					if (obj->Faces[n].v[1]->Map.x<x) x=obj->Faces[n].v[1]->Map.x;
					if (obj->Faces[n].v[1]->Map.y<y) y=obj->Faces[n].v[1]->Map.y;

					if (obj->Faces[n].v[2]->Map.x<x) x=obj->Faces[n].v[2]->Map.x;
					if (obj->Faces[n].v[2]->Map.y<y) y=obj->Faces[n].v[2]->Map.y;

				}
			}

			for (n=0;n<obj->nVertices;n++)
			{
				if (obj->Vertices[n].tag)
				{
					Vertices[nVertices].Stok=obj->Vertices[n].Stok;
					Vertices[nVertices].Norm=obj->Vertices[n].Norm;
					Vertices[nVertices].Map=obj->Vertices[n].Map;
					Vertices[nVertices].Map.x-=(int) x+1;
					Vertices[nVertices].Map.y-=(int) y+1;
					Vertices[nVertices].Map2=obj->Vertices[n].Map2;
					Vertices[nVertices].Diffuse=obj->Vertices[n].Diffuse;
					obj->Vertices[n].tag2=nVertices;
					nVertices++;
				}
			}

			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].tag2==tag)
				{
					Faces[nFaces]=obj->Faces[n];
					Faces[nFaces].v0=obj->Faces[n].v[0]->tag2;
					Faces[nFaces].v1=obj->Faces[n].v[1]->tag2;
					Faces[nFaces].v2=obj->Faces[n].v[2]->tag2;
					nFaces++;
				}
			}

			tag++;
			test=true;
		}
	}
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		vertex data are token from .Calc values
		
		void CGenerator::AddCalc(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddCalc(CObject3D * obj)
{
	int n;

	for (n=0;n<obj->nVertices;n++)
	{
        if (nVertices+n<maxnv)
        {
            Vertices[nVertices+n].Stok=obj->Vertices[n].Calc;
            Vertices[nVertices+n].Norm=obj->Vertices[n].NormCalc;
            Vertices[nVertices+n].Map=obj->Vertices[n].Map;
            Vertices[nVertices+n].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+n].tag=obj->Vertices[n].tag;
            Vertices[nVertices+n].tag2=obj->Vertices[n].tag2;
            Vertices[nVertices+n].temp=obj->Vertices[n].temp;
            Vertices[nVertices+n].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+n].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+n].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+n].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+n].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+n].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+n].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+n].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+n].Weight[3]=obj->Vertices[n].Weight[3];
        }
	}

	for (n=0;n<obj->nFaces;n++)
	{
        if (nFaces+n<maxnf)
        {
            Faces[nFaces+n].v0=obj->Faces[n].v0+nVertices;
            Faces[nFaces+n].v1=obj->Faces[n].v1+nVertices;
            Faces[nFaces+n].v2=obj->Faces[n].v2+nVertices;

            Faces[nFaces+n].Norm=obj->Faces[n].NormCalc;
            Faces[nFaces+n].tag=obj->Faces[n].tag;
            Faces[nFaces+n].tag2=obj->Faces[n].tag2;
            Faces[nFaces+n].ref=obj->Faces[n].ref;
            Faces[nFaces+n].nT=obj->Faces[n].nT;
            Faces[nFaces+n].nT2=obj->Faces[n].nT2;
            Faces[nFaces+n].nL=obj->Faces[n].nL;
            Faces[nFaces+n].nLVectors=obj->Faces[n].nLVectors;

            //Faces[nFaces+n].Diffuse=obj->Faces[n].Diffuse;
            Faces[nFaces+n].mp0=obj->Faces[n].mp0;
            Faces[nFaces+n].mp1=obj->Faces[n].mp1;
            Faces[nFaces+n].mp2=obj->Faces[n].mp2;

            Faces[nFaces+n].size_lightmap=obj->Faces[n].size_lightmap;
        }
	}

	nFaces+=obj->nFaces;
	nVertices+=obj->nVertices;

    if (nVertices>maxnv) nVertices=maxnv;
    if (nFaces>maxnf) nFaces=maxnf;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		vertex data are token from .Calc values
		
		void CGenerator::AddCalc(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CGenerator::AddCalcTess(CObject3D * objbase)
{
	int n;
	CObject3D *obj;
	CVector v,mini,maxi;
	float MAXI_BOUND=10000000.0f;

	maxi.Init(-MAXI_BOUND,-MAXI_BOUND,-MAXI_BOUND);
	mini.Init(MAXI_BOUND,MAXI_BOUND,MAXI_BOUND);
    
	for (n=0;n<objbase->nVertices;n++)
	{
		v=objbase->Vertices[n].Stok;
		if (v.x>maxi.x) maxi.x=v.x;
		if (v.y>maxi.y) maxi.y=v.y;
		if (v.z>maxi.z) maxi.z=v.z;
		if (v.x<mini.x) mini.x=v.x;
		if (v.y<mini.y) mini.y=v.y;
		if (v.z<mini.z) mini.z=v.z;
	}

	CVector bound=maxi-mini;
	if (bound.x<1) bound.x=1;
	if (bound.y<1) bound.y=1;
	if (bound.z<1) bound.z=1;
	float reso=bound.x*bound.y*bound.z/objbase->nFaces;

	if (reso<16.0f) obj=objbase->Tesselate2full();
	else obj=objbase->Duplicate2();

	for (n=0;n<obj->nVertices;n++)
	{
        if (nVertices+n<maxnv)
        {
            Vertices[nVertices+n].Stok=obj->Vertices[n].Calc;
            Vertices[nVertices+n].Norm=obj->Vertices[n].NormCalc;
            Vertices[nVertices+n].Map=obj->Vertices[n].Map;
            Vertices[nVertices+n].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+n].tag=obj->Vertices[n].tag;
            Vertices[nVertices+n].tag2=obj->Vertices[n].tag2;
            Vertices[nVertices+n].temp=obj->Vertices[n].temp;
            Vertices[nVertices+n].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+n].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+n].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+n].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+n].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+n].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+n].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+n].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+n].Weight[3]=obj->Vertices[n].Weight[3];
        }
	}

	for (n=0;n<obj->nFaces;n++)
	{
        if (nFaces+n<maxnf)
        {
            Faces[nFaces+n].v0=obj->Faces[n].v0+nVertices;
            Faces[nFaces+n].v1=obj->Faces[n].v1+nVertices;
            Faces[nFaces+n].v2=obj->Faces[n].v2+nVertices;

            Faces[nFaces+n].Norm=obj->Faces[n].NormCalc;
            Faces[nFaces+n].tag=obj->Faces[n].tag;
            Faces[nFaces+n].tag2=obj->Faces[n].tag2;
            Faces[nFaces+n].ref=obj->Faces[n].ref;
            Faces[nFaces+n].nT=obj->Faces[n].nT;
            Faces[nFaces+n].nT2=obj->Faces[n].nT2;
            Faces[nFaces+n].nL=obj->Faces[n].nL;
            Faces[nFaces+n].nLVectors=obj->Faces[n].nLVectors;

            //Faces[nFaces+n].Diffuse=obj->Faces[n].Diffuse;
            Faces[nFaces+n].mp0=obj->Faces[n].mp0;
            Faces[nFaces+n].mp1=obj->Faces[n].mp1;
            Faces[nFaces+n].mp2=obj->Faces[n].mp2;

            Faces[nFaces+n].size_lightmap=obj->Faces[n].size_lightmap;
        }
	}

	nFaces+=obj->nFaces;
	nVertices+=obj->nVertices;

    if (nVertices>maxnv) nVertices=maxnv;
    if (nFaces>maxnf) nFaces=maxnf;

	obj->Free();
	delete obj;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

     void CGenerator::AddTri(CVector &A,CVector &B,CVector &C)
     void CGenerator::AddTriNorm(CVector &A,CVector &NA,CVector &B,CVector &NB,CVector &C,CVector &NC)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CGenerator::AddTri(CVector &A,CVector &B,CVector &C)
{
    Vertices[nVertices+0].Stok=A;
    Vertices[nVertices+1].Stok=B;
    Vertices[nVertices+2].Stok=C;
    
    Faces[nFaces+0].v0=nVertices+0;
    Faces[nFaces+0].v1=nVertices+1;
    Faces[nFaces+0].v2=nVertices+2;
    
    nVertices+=3;
    nFaces++;
}

void CGenerator::AddTriNorm(CVector &A,CVector &NA,CVector &B,CVector &NB,CVector &C,CVector &NC)
{
    Vertices[nVertices+0].Stok=A;
    Vertices[nVertices+1].Stok=B;
    Vertices[nVertices+2].Stok=C;
    
    Vertices[nVertices+0].Norm=NA;
    Vertices[nVertices+1].Norm=NB;
    Vertices[nVertices+2].Norm=NC;
    
    Faces[nFaces+0].v0=nVertices+0;
    Faces[nFaces+0].v1=nVertices+1;
    Faces[nFaces+0].v2=nVertices+2;
    
    nVertices+=3;
    nFaces++;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

        add an object to the temporary one
        vertex data are token from .Calc values
        
        void CGenerator::AddMatrix(CObject3D * obj)

    Usage:
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddMatrix(CObject3D * obj,CMatrix M)
{
	int n;

	for (n=0;n<obj->nVertices;n++)
	{
		VECTOR3MATRIX(obj->Vertices[n].Calc,obj->Vertices[n].Stok,M);
		VECTOR3MATRIX3X3(obj->Vertices[n].NormCalc,obj->Vertices[n].Norm,M);
		obj->Vertices[n].NormCalc.Normalise();
	}

	for (n=0;n<obj->nVertices;n++)
	{
        if (nVertices+n<maxnv)
        {
            Vertices[nVertices+n].Stok=obj->Vertices[n].Calc;
            Vertices[nVertices+n].Norm=obj->Vertices[n].NormCalc;
            Vertices[nVertices+n].Map=obj->Vertices[n].Map;
            Vertices[nVertices+n].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+n].tag=obj->Vertices[n].tag;
            Vertices[nVertices+n].tag2=obj->Vertices[n].tag2;
            Vertices[nVertices+n].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+n].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+n].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+n].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+n].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+n].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+n].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+n].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+n].Weight[3]=obj->Vertices[n].Weight[3];
        }
    }
    
	for (n=0;n<obj->nFaces;n++)
	{
        if (nFaces+n<maxnf)
        {
            Faces[nFaces+n].v0=obj->Faces[n].v0+nVertices;
            Faces[nFaces+n].v1=obj->Faces[n].v1+nVertices;
            Faces[nFaces+n].v2=obj->Faces[n].v2+nVertices;

            Faces[nFaces+n].Norm=obj->Faces[n].NormCalc;
            Faces[nFaces+n].tag=obj->Faces[n].tag;
            Faces[nFaces+n].tag2=obj->Faces[n].tag2;
            Faces[nFaces+n].ref=obj->Faces[n].ref;
            Faces[nFaces+n].nT=obj->Faces[n].nT;
            Faces[nFaces+n].nT2=obj->Faces[n].nT2;
            Faces[nFaces+n].nL=obj->Faces[n].nL;
            Faces[nFaces+n].nLVectors=obj->Faces[n].nLVectors;

            //Faces[nFaces+n].Diffuse=obj->Faces[n].Diffuse;
            Faces[nFaces+n].mp0=obj->Faces[n].mp0;
            Faces[nFaces+n].mp1=obj->Faces[n].mp1;
            Faces[nFaces+n].mp2=obj->Faces[n].mp2;

            Faces[nFaces+n].size_lightmap=obj->Faces[n].size_lightmap;
        }
	}

	nFaces+=obj->nFaces;
	nVertices+=obj->nVertices;

    if (nVertices>maxnv) nVertices=maxnv;
    if (nFaces>maxnf) nFaces=maxnf;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		inversing orientation

		void CGenerator::AddInverse(CObject3D * obj)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddInverse(CObject3D * obj)
{
	int n;

	for (n=0;n<obj->nVertices;n++)
	{
        if (nVertices+n<maxnv)
        {
            Vertices[nVertices+n].Stok=obj->Vertices[n].Stok;
            Vertices[nVertices+n].Norm=obj->Vertices[n].Norm;
            Vertices[nVertices+n].Map=obj->Vertices[n].Map;
            Vertices[nVertices+n].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+n].Diffuse=obj->Vertices[n].Diffuse;
        }
	}

	for (n=0;n<obj->nFaces;n++)
	{
        if (nFaces+n<maxnf)
        {
            Faces[nFaces+n].v0=obj->Faces[n].v0+nVertices;
            Faces[nFaces+n].v1=obj->Faces[n].v2+nVertices;
            Faces[nFaces+n].v2=obj->Faces[n].v1+nVertices;
            Faces[nFaces+n].Norm=obj->Faces[n].Norm;
            Faces[nFaces+n].tag=obj->Faces[n].tag;
            Faces[nFaces+n].nT=obj->Faces[n].nT;
            Faces[nFaces+n].nL=obj->Faces[n].nL;
            Faces[nFaces+n].nLVectors=obj->Faces[n].nLVectors;
            Faces[nFaces+n].nT2=obj->Faces[n].nT2;
            Faces[nFaces+n].ref=obj->Faces[n].ref;
            Faces[nFaces+n].size_lightmap=obj->Faces[n].size_lightmap;
            //Faces[nFaces+n].Diffuse=obj->Faces[n].Diffuse;
            Faces[nFaces+n].mp0=obj->Faces[n].mp0;
            Faces[nFaces+n].mp1=obj->Faces[n].mp1;
            Faces[nFaces+n].mp2=obj->Faces[n].mp2;
        }
	}

	nFaces+=obj->nFaces;
	nVertices+=obj->nVertices;
    
    if (nVertices>maxnv) nVertices=maxnv;
    if (nFaces>maxnf) nFaces=maxnf;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		accordingly to .tag

		void CGenerator::AddSelected(CObject3D * obj,int tag)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CGenerator::AddSelected(CObject3D * obj,int tag)
{
	int * corres;
	int n,nn;
	int nf,np;

	corres=(int *) malloc(4*obj->nVertices);

	for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

	nf=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			obj->Vertices[obj->Faces[n].v0].tag=1;
			obj->Vertices[obj->Faces[n].v1].tag=1;
			obj->Vertices[obj->Faces[n].v2].tag=1;
			nf++;
		}
	}

	np=0;
	for (n=0;n<obj->nVertices;n++) 
	{
		corres[n]=-1;
		if (obj->Vertices[n].tag==1)
		{
			corres[n]=np;
			np++;
		}
	}

	
	nn=0;

	for (n=0;n<obj->nVertices;n++)
	{
		if (obj->Vertices[n].tag==1)
		{
			Vertices[nVertices+nn].Stok=obj->Vertices[n].Stok;
			Vertices[nVertices+nn].Norm=obj->Vertices[n].Norm;
			Vertices[nVertices+nn].Map=obj->Vertices[n].Map;
			Vertices[nVertices+nn].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+nn].tag=obj->Vertices[n].tag;
            Vertices[nVertices+nn].tag2=obj->Vertices[n].tag2;
			Vertices[nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+nn].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+nn].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+nn].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+nn].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+nn].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+nn].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+nn].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+nn].Weight[3]=obj->Vertices[n].Weight[3];
			nn++;
		}
	}


	nn=nFaces;
	
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			Faces[nn].v0=nVertices+corres[obj->Faces[n].v0];
			Faces[nn].v1=nVertices+corres[obj->Faces[n].v1];
			Faces[nn].v2=nVertices+corres[obj->Faces[n].v2];
			Faces[nn].Norm=obj->Faces[n].Norm;
			Faces[nn].tag=obj->Faces[n].tag;
			Faces[nn].nT=obj->Faces[n].nT;
			
			//Faces[nn].Diffuse=obj->Faces[n].Diffuse;
			Faces[nn].mp0=obj->Faces[n].mp0;
			Faces[nn].mp1=obj->Faces[n].mp1;
			Faces[nn].mp2=obj->Faces[n].mp2;
			nn++;
		}
	}

	nVertices+=np;
	nFaces+=nf;


	free(corres);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		add an object to the temporary one
		accordingly to .tag

		void CGenerator::AddSelectedCalc(CObject3D * obj,int tag)

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CGenerator::AddSelectedCalc(CObject3D * obj,int tag)
{
	int * corres;
	int n,nn;
	int nf,np;

	corres=(int *) malloc(4*obj->nVertices);

	for (n=0;n<obj->nVertices;n++) obj->Vertices[n].tag=0;

	nf=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			obj->Vertices[obj->Faces[n].v0].tag=1;
			obj->Vertices[obj->Faces[n].v1].tag=1;
			obj->Vertices[obj->Faces[n].v2].tag=1;
			nf++;
		}
	}

	np=0;
	for (n=0;n<obj->nVertices;n++) 
	{
		corres[n]=-1;
		if (obj->Vertices[n].tag==1)
		{
			corres[n]=np;
			np++;
		}
	}

	
	nn=0;

	for (n=0;n<obj->nVertices;n++)
	{
		if (obj->Vertices[n].tag==1)
		{
			Vertices[nVertices+nn].Stok=obj->Vertices[n].Calc;
			Vertices[nVertices+nn].Norm=obj->Vertices[n].NormCalc;
			Vertices[nVertices+nn].Map=obj->Vertices[n].Map;
			Vertices[nVertices+nn].Map2=obj->Vertices[n].Map2;
            Vertices[nVertices+nn].tag=obj->Vertices[n].tag;
            Vertices[nVertices+nn].tag2=obj->Vertices[n].tag2;
            Vertices[nVertices+nn].Diffuse=obj->Vertices[n].Diffuse;
            Vertices[nVertices+nn].Index[0]=obj->Vertices[n].Index[0];
            Vertices[nVertices+nn].Index[1]=obj->Vertices[n].Index[1];
            Vertices[nVertices+nn].Index[2]=obj->Vertices[n].Index[2];
            Vertices[nVertices+nn].Index[3]=obj->Vertices[n].Index[3];
            Vertices[nVertices+nn].Weight[0]=obj->Vertices[n].Weight[0];
            Vertices[nVertices+nn].Weight[1]=obj->Vertices[n].Weight[1];
            Vertices[nVertices+nn].Weight[2]=obj->Vertices[n].Weight[2];
            Vertices[nVertices+nn].Weight[3]=obj->Vertices[n].Weight[3];

			nn++;
		}
	}


	nn=nFaces;
	
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			Faces[nn].v0=nVertices+corres[obj->Faces[n].v0];
			Faces[nn].v1=nVertices+corres[obj->Faces[n].v1];
			Faces[nn].v2=nVertices+corres[obj->Faces[n].v2];
			Faces[nn].Norm=obj->Faces[n].Norm;
			Faces[nn].tag=obj->Faces[n].tag;
			Faces[nn].nT=obj->Faces[n].nT;
			
			//Faces[nn].Diffuse=obj->Faces[n].Diffuse;
			Faces[nn].mp0=obj->Faces[n].mp0;
			Faces[nn].mp1=obj->Faces[n].mp1;
			Faces[nn].mp2=obj->Faces[n].mp2;
			nn++;
		}
	}

	nVertices+=np;
	nFaces+=nf;


	free(corres);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:
		
		get temporary object from generator
		
		CObject3D CGenerator::ResObjet()

	Usage:	
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


CObject3D CGenerator::ResObjet()
{
	CObject3D duplicated;
	int n;

	duplicated.Init(nVertices,nFaces);

	for (n=0;n<nVertices;n++)
	{
		duplicated.Vertices[n].Stok=Vertices[n].Stok;
		duplicated.Vertices[n].Norm=Vertices[n].Norm;
		duplicated.Vertices[n].Map=Vertices[n].Map;
		duplicated.Vertices[n].Map2=Vertices[n].Map2;
		duplicated.Vertices[n].Diffuse=Vertices[n].Diffuse;
	}

	for (n=0;n<nFaces;n++)
	{
		duplicated.Faces[n].v0=Faces[n].v0;
		duplicated.Faces[n].v1=Faces[n].v1;
		duplicated.Faces[n].v2=Faces[n].v2;

		duplicated.Faces[n].Norm=Faces[n].Norm;
		duplicated.Faces[n].tag=Faces[n].tag;
		duplicated.Faces[n].nT=Faces[n].nT;

		//duplicated.Faces[n].Diffuse=Faces[n].Diffuse;
		duplicated.Faces[n].mp0=Faces[n].mp0;
		duplicated.Faces[n].mp1=Faces[n].mp1;
		duplicated.Faces[n].mp2=Faces[n].mp2;
	}

	duplicated.SetFaces();

	return duplicated;
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		
		
		void CGenerator::ReadScript(char * script)
		void CGenerator::ReadScript(char * script,CObject3D *base1)
		void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2)
		void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3)
		void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3,CObject3D *base4)


		base(x) : <=> OBJx
		
	Usage:	

	ex:

    char script[]="OBJ1 CUBE 10 10 10\n"
				"OBJ2 CUBE 5 5 5\n"
				"OBJ1 POS 0 0 0\n"
				"OBJ1 ROT 0 0 0\n"
				"OBJ2 POS 5 0 0\n"
				"OBJ2 ROT 0 0 0\n"
				"OBJ1 ADD OBJ1 OBJ2\n"

					"I RAYON OBJ\n"
					"OBJ DIFFUSE Y 0 I\n"

				"END\n";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

enum
{
	INT_A0	= 2019,
	INT_A1	= 2020,
	INT_A2	= 2021,
	INT_A3	= 2022,
	VECT_X	= 1111,
	VECT_Y	= 1112,
	VECT_Z	= 1113,

	RGB0	= 3001,
	RGB1	= 3002,
	RGB2	= 3003,
	RGB3	= 3004,
};

int WitchRes(char * str)
{
	int res=-1;

	if (strcmp(str,"OBJ1")==0) res=0;
	if (strcmp(str,"OBJ2")==0) res=1;
	if (strcmp(str,"OBJ3")==0) res=2;
	if (strcmp(str,"OBJ4")==0) res=3;

	if (strcmp(str,"A0")==0) res=INT_A0;
	if (strcmp(str,"A1")==0) res=INT_A1;
	if (strcmp(str,"A2")==0) res=INT_A2;
	if (strcmp(str,"A3")==0) res=INT_A3;

	if (strcmp(str,"X")==0) res=VECT_X;
	if (strcmp(str,"Y")==0) res=VECT_Y;
	if (strcmp(str,"Z")==0) res=VECT_Z;

	if (strcmp(str,"RGB0")==0) res=RGB0;
	if (strcmp(str,"RGB1")==0) res=RGB1;
	if (strcmp(str,"RGB2")==0) res=RGB2;
	if (strcmp(str,"RGB3")==0) res=RGB3;

	if (strcmp(str,"ADD")==0) res=5012;
	if (strcmp(str,"SUB")==0) res=5013;

	return res;
}

enum
{
	OP_SC_CUBE			=	1,
	OP_SC_SPHERE		=	2,
	OP_SC_PLANE			=	3,
	OP_SC_CYLINDER		=	4,

	OP_SC_CENTER		=	5,
	OP_SC_INVERSE		=	6,
	OP_SC_TESSELATE		=	7,
	OP_SC_DUPLICATE		=	8,
	OP_SC_ADD			=	9,
	OP_SC_ADDBOOLEEN	=	10,

	OP_SC_DUPLICATES	=	11,
	OP_SC_TAG_NT		=	12,
	OP_SC_POSITION		=	13,
	OP_SC_ROTATION		=	14,

	OP_SC_RESET			=	15,
	OP_SC_TEXTURE		=	16,
	OP_SC_TESSELATES	=	17,

	OP_SC_NORMALIZE		=	18,
	OP_SC_RAYON			=	19,
	OP_SC_DIFFUSE		=	20,


	OP_SC_NEG			=	21,
	OP_SC_MUL			=	22,
	OP_SC_DIV			=	23,
	OP_SC_SUB			=	24,
	OP_SC_MOV			=	25
};

int WitchOp(char * str)
{
	int op=0;

	// creation
	if (strcmp(str,"CUBE")==0) op=OP_SC_CUBE;
	if (strcmp(str,"SPHERE")==0) op=OP_SC_SPHERE;
	if (strcmp(str,"PLANE")==0) op=OP_SC_PLANE;
	if (strcmp(str,"CYLINDER")==0) op=OP_SC_CYLINDER;

	// opérations
	if (strcmp(str,"CENTER")==0) op=OP_SC_CENTER;
	if (strcmp(str,"RADIUS")==0) op=OP_SC_RAYON;
	if (strcmp(str,"TESSELATE")==0) op=OP_SC_TESSELATE;
	if (strcmp(str,"INVERSE")==0) op=OP_SC_INVERSE;
	if (strcmp(str,"DUPLICATE")==0) op=OP_SC_DUPLICATE;
	if (strcmp(str,"DUPLICATE_SELECTED")==0) op=OP_SC_DUPLICATES;
	if (strcmp(str,"TESSELATE_SELECTED")==0) op=OP_SC_TESSELATES;
	if (strcmp(str,"ADD")==0) op=OP_SC_ADD;
	if (strcmp(str,"TAG_NT")==0) op=OP_SC_TAG_NT;
	if (strcmp(str,"BOOLEEN")==0) op=OP_SC_ADDBOOLEEN;
	if (strcmp(str,"END")==0) op=-1;
	if (strcmp(str,"POS")==0) op=OP_SC_POSITION;
	if (strcmp(str,"ROT")==0) op=OP_SC_ROTATION;
	if (strcmp(str,"RESET")==0) op=OP_SC_RESET;
	if (strcmp(str,"TEXTURE")==0) op=OP_SC_TEXTURE;
	if (strcmp(str,"NORMALIZE")==0) op=OP_SC_NORMALIZE;
	if (strcmp(str,"DIFFUSE")==0) op=OP_SC_DIFFUSE;

	if (strcmp(str,"NEG")==0) op=OP_SC_NEG;
	if (strcmp(str,"MUL")==0) op=OP_SC_MUL;
	if (strcmp(str,"DIV")==0) op=OP_SC_DIV;
	if (strcmp(str,"SUB")==0) op=OP_SC_SUB;
	if (strcmp(str,"MOV")==0) op=OP_SC_MOV;

	return op;
}	

char TemporaryString[1024];

char * ParseScript(char * script,int *pos)
{
	int n,len;
	n=*pos;
	while ((script[n]!='\n')&&(script[n]!=' ')) n++;
	len=n-(*pos);
	memcpy(TemporaryString,&script[*pos],len);
	TemporaryString[len]='\0';
	*pos=n+1;

	return TemporaryString;
}


void CGenerator::ReadScript(char * script)
{
	ReadScript(script,NULL,NULL,NULL,NULL);
}

void CGenerator::ReadScript(char * script,CObject3D *base1)
{
	ReadScript(script,base1,NULL,NULL,NULL);
}

void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2)
{
	ReadScript(script,base1,base2,NULL,NULL);
}

void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3)
{
	ReadScript(script,base1,base2,base3,NULL);
}


void CGenerator::ReadScript(char * script,CObject3D *base1,CObject3D *base2,CObject3D *base3,CObject3D *base4)
{
	CGenerator GEN;
	int n;
	int pos=0;
	CVector u;
	bool end=false;
	char * command;
	char * dest;
	char * param;
	int res,res2,res3;
	CObject3D *objs[4];
	CObject3D *obj,*obj1,*obj2;
	CObject3D *robj1,*robj2;
	CObject3D **pobj1,**pobj2;
	CObject3D *tmp;
	float lx,ly,lz,R,L,DIM,x,y,z;
	int DEF,NT;
	int tag;
	float Values[4];
	float RGB[4][3];
	CBSPVolumic BSP;

	for (n=0;n<4;n++) objs[n]=NULL;
	obj=NULL;

	if (base1) objs[0]=base1->Duplicate2();
	if (base2) objs[1]=base2->Duplicate2();
	if (base3) objs[2]=base3->Duplicate2();
	if (base4) objs[3]=base4->Duplicate2();

	while (!end)
	{
		dest=ParseScript(script,&pos);




		if (WitchOp(dest)!=-1)
		{
			res=WitchRes(dest);

			command=ParseScript(script,&pos);

			if ((res>=INT_A0)&&(res<=INT_A3))
			{

//					"I RAYON OBJ\n"
//					"OBJ DIFFUSE Y 0 I\n"

				float v0,v1;

				switch (WitchOp(command))
				{

				case OP_SC_NEG:
					Values[res-INT_A0]=-Values[res-INT_A0];
					break;


				case OP_SC_ADD:
					

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v0);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v0=Values[res2-INT_A0];
					}

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v1);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v1=Values[res2-INT_A0];
					}
					Values[res-INT_A0]=v0+v1;
					break;

				case OP_SC_SUB:
					

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v0);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v0=Values[res2-INT_A0];
					}

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v1);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v1=Values[res2-INT_A0];
					}
					Values[res-INT_A0]=v0-v1;
					break;

				case OP_SC_MOV:
					

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v0);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v0=Values[res2-INT_A0];
					}

					Values[res-INT_A0]=v0;
					break;


				case OP_SC_MUL:
					

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v0);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v0=Values[res2-INT_A0];
					}

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v1);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v1=Values[res2-INT_A0];
					}
					Values[res-INT_A0]=v0*v1;
					break;

				case OP_SC_DIV:
					

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v0);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v0=Values[res2-INT_A0];
					}

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&v1);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) v1=Values[res2-INT_A0];
					}
					Values[res-INT_A0]=v0/v1;
					break;


				case OP_SC_RAYON:
					float r;
					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					
					if (res2>=0)
					{
						if (objs[res2]) r=objs[res2]->CalculateRadius(); 
							
					}
					else
					{
						if (obj) r=obj->CalculateRadius();
					}

					Values[res-INT_A0]=r;
					
					break;
				};

					

			}
			else
			if ((res>=RGB0)&&(res<=RGB3))
			{
				float r,g,b;
//				param=ParseScript(script,&pos);
				sscanf(command,"%f",&r);

				param=ParseScript(script,&pos);
				sscanf(param,"%f",&g);

				param=ParseScript(script,&pos);
				sscanf(param,"%f",&b);

				RGB[res-RGB0][0]=r;
				RGB[res-RGB0][1]=g;
				RGB[res-RGB0][2]=b;
			}
			else
			{


				switch (WitchOp(command))
				{
				case OP_SC_DIFFUSE:
					float _min,_max;
					int a1,a2;
					float r,g,b;
					float s;


					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==VECT_X) u.Init(1,0,0);
					if (res2==VECT_Y) u.Init(0,1,0);
					if (res2==VECT_Z) u.Init(0,0,1);

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&_min);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) _min=Values[res2-INT_A0];
					}

					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					if (res2==-1)
					{
						sscanf(param,"%f",&_max);
					}
					else
					{
						if ((res2>=INT_A0)&&(res2<=INT_A3)) _max=Values[res2-INT_A0];
					}


					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					a1=res2-RGB0;

					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					a2=res2-RGB0;

					if (res>=0)
					{
						tmp=objs[res];
					}
					else
					{
						tmp=obj;
					}

					for (n=0;n<tmp->nVertices;n++)
					{
						s=(tmp->Vertices[n].Stok||u);
						if (s<_min) { r=RGB[a1][0];g=RGB[a1][1];b=RGB[a1][2];}
						else
						if (s>_max) { r=RGB[a2][0];g=RGB[a2][1];b=RGB[a2][2];}
						else
						{
							r=RGB[a1][0]+(RGB[a2][0]-RGB[a1][0])*(s-_min)/(_max-_min);
							g=RGB[a1][1]+(RGB[a2][1]-RGB[a1][1])*(s-_min)/(_max-_min);
							b=RGB[a1][2]+(RGB[a2][2]-RGB[a1][2])*(s-_min)/(_max-_min);
						}

						tmp->Vertices[n].Diffuse.Init(r,g,b);

					}




					break;


				case OP_SC_CUBE:
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&lx);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&ly);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&lz);

					tmp=new CObject3D;
					tmp->Cube2(lx,ly,lz);


					if (res>=0)
					{
						if (objs[res]) 
						{
							objs[res]->Free();
							delete objs[res];
						}

						objs[res]=tmp;
					}
					else
					{
						if (obj) 
						{
							obj->Free();
							delete obj;
						}
						obj=tmp;
					}


					break;
				case OP_SC_SPHERE:
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&R);
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&DEF);

					tmp=new CObject3D;
					tmp->Sphere(R,DEF);


					if (res>=0)
					{
						if (objs[res]) 
						{
							objs[res]->Free();
							delete objs[res];
						}

						objs[res]=tmp;
					}
					else
					{
						if (obj) 
						{
							obj->Free();
							delete obj;
						}
						obj=tmp;
					}


					break;
				case OP_SC_PLANE:
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&DIM);
					
					tmp=new CObject3D;
					tmp->Plane(DIM);


					if (res>=0)
					{
						if (objs[res]) 
						{
							objs[res]->Free();
							delete objs[res];
						}

						objs[res]=tmp;
					}
					else
					{
						if (obj) 
						{
							obj->Free();
							delete obj;
						}
						obj=tmp;
					}

					break;
				case OP_SC_CYLINDER:
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&L);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&R);
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&DEF);

					tmp=new CObject3D;
					tmp->Cylindre(L,R,DEF);

					if (res>=0)
					{
						if (objs[res]) 
						{
							objs[res]->Free();
							delete objs[res];
						}

						objs[res]=tmp;
					}
					else
					{
						if (obj) 
						{
							obj->Free();
							delete obj;
						}
						obj=tmp;
					}

					break;
				case OP_SC_CENTER:

					if (res>=0)
					{
						if (objs[res]) objs[res]->Center();
					}
					else
					{
						if (obj) obj->Center();
					}
					break;
				case OP_SC_INVERSE:
					if (res>=0)
					{
						if (objs[res]) 
						{
							tmp=objs[res]->Inverse2();
							objs[res]->Free();
							delete objs[res];
							objs[res]=tmp;
						}
						
					}
					else
					{
						
						if (obj) 
						{
							tmp=obj->Inverse2();
							obj->Free();
							delete obj;
							obj=tmp;
						}
						
					}
					break;
				case OP_SC_TESSELATE:
					if (res>=0)
					{
						
						if (objs[res]) 
						{
							tmp=objs[res]->Tesselate2();
							objs[res]->Free();
							delete objs[res];
							objs[res]=tmp;
						}
						
					}
					else
					{
						
						if (obj) 
						{
							tmp=obj->Tesselate2();
							obj->Free();
							delete obj;
							obj=tmp;
						}
						
					}
					break;

				case OP_SC_NORMALIZE:

					if (res>=0)
					{
						
						if (objs[res]) 
						{
							tmp=objs[res]->Normalize2();
							objs[res]->Free();
							delete objs[res];
							objs[res]=tmp;
						}
						
					}
					else
					{
						
						if (obj) 
						{
							tmp=obj->Normalize2();
							obj->Free();
							delete obj;
							obj=tmp;
						}
						
					}
					break;


				case OP_SC_TESSELATES:
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&tag);

					if (res>=0)
					{
						if (objs[res]) 
						{
							tmp=objs[res]->TesselateSelected2(tag);
							objs[res]->Free();
							delete objs[res];
							objs[res]=tmp;
						}
					}
					else
					{
						if (obj) 
						{
							tmp=obj->TesselateSelected2(tag);
							obj->Free();
							delete obj;
							obj=tmp;
						}
					}
					break;

				case OP_SC_TEXTURE:
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&NT);

					if (res>=0)
					{
						if (objs[res]) objs[res]->FillTexture(NT); 
					}
					else
					{
						if (obj) obj->FillTexture(NT); 
					}
					break;

				case OP_SC_DUPLICATE:
					param=ParseScript(script,&pos);
					res2=WitchRes(param);

					tmp=NULL;
					if (res2>=0)
					{
						if (objs[res2]) tmp=objs[res2]->Duplicate2(); 
							
					}
					else
					{
						if (obj) tmp=obj->Duplicate2();
					}


					if (tmp)
					{
						if (res>=0)
						{
							if (objs[res]) 
							{
								objs[res]->Free();
								delete objs[res];
							}

							objs[res]=tmp;
							
						}
						else
						{
							if (obj) 
							{
								obj->Free();
								delete obj;
							}
							obj=tmp;
							
						}
					}


					break;
				case OP_SC_ADD:

					GEN.Reset();

					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					if (res2>=0)
					{
						if (objs[res2]) 
						{
							objs[res2]->Calculate();
							GEN.AddCalc(objs[res2]);
						}
					}
					else
					{
						if (obj) 
						{
							obj->Calculate();
							GEN.AddCalc(obj);
						}
					}
					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					if (res2>=0)
					{
						if (objs[res2]) 
						{
							objs[res2]->Calculate();
							GEN.AddCalc(objs[res2]);
						}
					}
					else
					{
						if (obj) 
						{
							obj->Calculate();
							GEN.AddCalc(obj);
						}
					}

					tmp=GEN.pResObjet();

					if (res>=0)
					{
						if (objs[res]) 
						{
							objs[res]->Free();
							delete objs[res];
						}

						objs[res]=tmp;
					}
					else
					{
						if (obj) 
						{
							obj->Free();
							delete obj;
						}
						obj=tmp;
					}


					break;
				case OP_SC_ADDBOOLEEN:


					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					param=ParseScript(script,&pos);
					res3=WitchRes(param);

#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

					tag=0;
					if (res2==-1)
					{
						if (obj) { obj1=obj;pobj1=&obj;tag++;}
					}
					else
					{
						if (objs[res2]) { obj1=objs[res2];pobj1=&objs[res2];tag++;}
					}
					if (res3==-1)
					{
						if (obj) { obj2=obj;pobj2=&obj;tag++;}
					}
					else
					{
						if (objs[res3]) { obj2=objs[res3];pobj2=&objs[res3];tag++;}
					}

					if ((res2!=res3)&&(tag==2))
					{
						if (res==5012)
						{
							if (obj1->CalculateRadius()>obj2->CalculateRadius())
							{
								robj1=BSP.Boolean(obj1,obj2,BOOLEAN_OUT,-1,-1,false);
								robj2=BSP.Boolean(obj2,obj1,BOOLEAN_OUT,-1,1,false);
							}
							else
							{
								robj1=BSP.Boolean(obj1,obj2,BOOLEAN_OUT,-1,1,false);
								robj2=BSP.Boolean(obj2,obj1,BOOLEAN_OUT,-1,-1,false);
							}
						}
						else
						{
							if (obj1->CalculateRadius()>obj2->CalculateRadius())
							{
								robj1=BSP.Boolean(obj1,obj2,BOOLEAN_OUT,1,-1,false);
								robj2=BSP.Boolean(obj2,obj1,BOOLEAN_OUT,1,1,false);
							}
							else
							{
								robj1=BSP.Boolean(obj1,obj2,BOOLEAN_OUT,1,1,false);
								robj2=BSP.Boolean(obj2,obj1,BOOLEAN_OUT,1,-1,false);
							}
						}

						obj1->Free();
						delete obj1;
						obj2->Free();
						delete obj2;


						if (robj1)
						{
							*pobj1=robj1->OptimizeMeshBoolean();
							robj1->Free();
							delete robj1;
						}
						else
						{
							*pobj1=NULL;
						}


						if (robj2)
						{
							*pobj2=robj2->OptimizeMeshBoolean();
							robj2->Free();
							delete robj2;
						}
						else
						{
							*pobj2=NULL;
						}
					}
#endif
					break;
				case OP_SC_DUPLICATES:

					param=ParseScript(script,&pos);
					res2=WitchRes(param);
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&tag);

					tmp=NULL;

					if (res2>=0)
					{
						if (objs[res2]) tmp=objs[res2]->DuplicateSelected2(tag); 
					}
					else
					{
						if (obj) tmp=obj->DuplicateSelected2(tag);
					}

					if (tmp)
					{
						if (res>=0)
						{
							if (objs[res]) 
							{
								objs[res]->Free();
								delete objs[res];
							}
							objs[res]=tmp;
						}
						else
						{
							if (obj) 
							{
								obj->Free();
								delete obj;
							}
							obj=tmp;
						}
					}

					break;
				case OP_SC_RESET:
					if (res>=0)
					{
						if (objs[res]) 
						{
							for (n=0;n<objs[res]->nFaces;n++) objs[res]->Faces[n].tag=0;
						}
					}
					else
					{
						if (obj) 
						{
							for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
						}
					}

					break;

				case OP_SC_TAG_NT:
					param=ParseScript(script,&pos);
					sscanf(param,"%d",&NT);

					if (res>=0)
					{
						if (objs[res]) 
						{
							for (n=0;n<objs[res]->nFaces;n++) if (objs[res]->Faces[n].nT==NT) objs[res]->Faces[n].tag=0;
						}
					}
					else
					{
						if (obj) 
						{
							for (n=0;n<obj->nFaces;n++) if (obj->Faces[n].nT==NT) obj->Faces[n].tag=0;
						}
					}


					break;

				case OP_SC_POSITION:

					param=ParseScript(script,&pos);
					sscanf(param,"%f",&x);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&y);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&z);


					if (res>=0)
					{
						if (objs[res]) objs[res]->Coo.Init(x,y,z);
					}
					else
					{
						if (obj) obj->Coo.Init(x,y,z);
					}


					break;

				case OP_SC_ROTATION:

					param=ParseScript(script,&pos);
					sscanf(param,"%f",&x);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&y);
					param=ParseScript(script,&pos);
					sscanf(param,"%f",&z);

					if (res>=0)
					{
						if (objs[res]) objs[res]->Rot.Init(x,y,z);
					}
					else
					{
						if (obj) obj->Rot.Init(x,y,z);
					}
					break;

				};
			}

		}
		else end=true;
	}


	for (n=0;n<4;n++) 
		if (objs[n])
		{
			objs[n]->Free();
			delete objs[n];
		}


	if (obj)
	{
		obj->Calculate();
		AddCalc(obj);
	}
}


void CGenerator::SetFaces()
{
	int n;
	for (n=0;n<nFaces;n++)
	{
		Faces[n].v[0]=&Vertices[Faces[n].v0];
		Faces[n].v[1]=&Vertices[Faces[n].v1];
		Faces[n].v[2]=&Vertices[Faces[n].v2];
	}
}

#endif
