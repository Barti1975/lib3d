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
//	@file metaballs.cpp 
//	@date 2009-01-07
////////////////////////////////////////////////////////////////////////

#include "metaballs.h"

#ifdef _DEFINES_API_CODE_METABALLS_

#include <math.h>
#include "../base/triangularisation.h"
#include "../data/png.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::AddMetaballTube(CVector a,CVector b,float pota,float potb)
{
	MetaballsTube[nMetaballsTube].a=a;
	MetaballsTube[nMetaballsTube].b=b;
	MetaballsTube[nMetaballsTube].pa=pota;
	MetaballsTube[nMetaballsTube].pb=potb;
	MetaballsTube[nMetaballsTube].tag=0;
	nMetaballsTube++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::SubMetaballTube(CVector a,CVector b,float pota,float potb)
{
	MetaballsTube[nMetaballsTube].a=a;
	MetaballsTube[nMetaballsTube].b=b;
	MetaballsTube[nMetaballsTube].pa=pota;
	MetaballsTube[nMetaballsTube].pb=potb;
	MetaballsTube[nMetaballsTube].tag=1;
	nMetaballsTube++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::AddMetaball(CVector pos,float pot)
{
	Metaballs[nMetaballs].v=pos;
	Metaballs[nMetaballs].p=pot;
	Metaballs[nMetaballs].tag=0;
	nMetaballs++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::SubMetaball(CVector pos,float pot)
{
	Metaballs[nMetaballs].v=pos;
	Metaballs[nMetaballs].p=pot;
	Metaballs[nMetaballs].tag=1;
	nMetaballs++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::SetEquipotentielle(float e,float precision)
{
	equi=e;
	small_float=precision*0.01f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CShortF::Init(int i1,int i2,int i3)
{
	n0=i1;
	n1=i2;
	n2=i3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float dist(CVector &P,CVector &O)
{
	CVector v;
	VECTORSUB(v,P,O);
	return VECTORNORM2(v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float DistancePointSegment(CVector &P,CVector &A,CVector &B)
{
	CVector v,w;
	VECTORSUB(v,B,A);
	VECTORSUB(w,P,A);
	float c1=DOT(w,v);
	if (c1<=0.0) return dist(P,A);
	float c2=DOT(v,v);
	if (c2<=c1) return dist(P,B);
	float b=c1/c2;
	CVector Pb=A+b*v;

	return dist(P,Pb);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float PotentielPointSegment(CVector &P,CVector &A,CVector &B,float pa,float pb)
{
	CVector v,w;
	VECTORSUB(v,B,A);
	VECTORSUB(w,P,A);
	float c1=DOT(w,v);
	if (c1<=0.0) return pa;
	float c2=DOT(v,v);
	if (c2<=c1) return pb;
	float b=c1/c2;
	float p=pa+b*(pb-pa);
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CMetaballs::Potentiel(CVector &u)
{
	float p;
	float f;
	float x,y,z;
	int n;

	p=0.0f;
	for (n=0;n<nMetaballs;n++)
	{
		x=Metaballs[n].v.x-u.x;
		y=Metaballs[n].v.y-u.y;
		z=Metaballs[n].v.z-u.z;
		f=1.0f/(x*x+y*y+z*z);
		if (Metaballs[n].tag==0) p+=Metaballs[n].p*f;
		else p-=Metaballs[n].p*f;
	}

	for (n=0;n<nMetaballsTube;n++)
	{
		f=1.0f/DistancePointSegment(u,MetaballsTube[n].a,MetaballsTube[n].b);
		if (MetaballsTube[n].tag==0) p+=PotentielPointSegment(u,MetaballsTube[n].a,MetaballsTube[n].b,MetaballsTube[n].pa,MetaballsTube[n].pb)*f;
		else p-=PotentielPointSegment(u,MetaballsTube[n].a,MetaballsTube[n].b,MetaballsTube[n].pa,MetaballsTube[n].pb)*f;
	}

	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DELTA 0.00001f
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMetaballs::Gradiant(CVector &A)
{
	CVector uu,u,v,u1,u2;
	float f;
	CVector d,pp;
	int n;
	float p0,pu;

	u.Init(0,0,0);
	p0=Potentiel(A);

	if (nMetaballsTube==0)
	{
		for (n=0;n<nMetaballs;n++)
		{
			VECTORSUB(d,A,Metaballs[n].v);
			f=(d.x*d.x+d.y*d.y+d.z*d.z);
			f=2*Metaballs[n].p/(f*f);
			VECTORMUL(uu,d,f);
			if (Metaballs[n].tag==0) { VECTORSUB(u,u,uu); }
			else { VECTORADD(u,u,uu); }
		}
		VECTORNORMALISE(u);
		VECTORMUL(pp,u,0.1f);
		VECTORADD(pp,A,pp);
		pu=Potentiel(pp);
		if (pu>p0) VECTORMUL(u,u,-1.0f);
	}
	else
	{
		CVector A0,A1;
		A0=A1=A;
		A1.x+=SMALLF2;
		A0.x-=SMALLF2;
		u.x=Potentiel(A1)-Potentiel(A0);
		A0=A1=A;
		A1.y+=SMALLF2;
		A0.y-=SMALLF2;
		u.y=Potentiel(A1)-Potentiel(A0);
		A0=A1=A;
		A1.z+=SMALLF2;
		A0.z-=SMALLF2;
		u.z=Potentiel(A1)-Potentiel(A0);

		VECTORNORMALISE(u);
		VECTORMUL(pp,u,0.1f);
		VECTORADD(pp,A,pp);
		pu=Potentiel(pp);
		if (pu>p0) VECTORMUL(u,u,-1.0f);
	}
	return u;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float ffabs(float x)
{
	if (x<0) return -x;
	return x;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMetaballs::EquiPotentielle(CVector &A,CVector &B,float pA0,float pB0)
{
	CVector I;
	CVector a,b;
	float p,t;
	int n;
	float pA;
	float pB;

	if (pA0>=equi) { a=A; b=B; pA=pA0; pB=pB0; }
	else { a=B; b=A; pA=pB0; pB=pA0;}

	if (((pA<equi)&&(pB>equi))||((pA>equi)&&(pB<equi)))
	{
		t=(equi-pA)/(pB-pA);
		VECTORINTERPOL(I,t,b,a);
		p=Potentiel(I);
		n=0;
		while ((n<K_ITERATIONS)&&(ffabs(p-equi)>SMALLF2))
		{
			if (p<equi) { b=I; pB=p; } else { a=I; pA=p; }
			t=(equi-pA)/(pB-pA);
			VECTORINTERPOL(I,t,b,a);
			p=Potentiel(I);
			n++;
		}

		if (p<equi) { b=I; pB=p; } else { a=I; pA=p; }
		t=(equi-pA)/(pB-pA);
		VECTORINTERPOL(I,t,b,a);
	}
	else I=a;

	return UploadVertices(I);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::Init()
{
	const int B[6][3] = { { -1,0,0 },{  1,0,0 },{  0,1,0 },{  0,-1,0 },{  0,0,1 },{  0,0,-1 } };
	CVector m1,m2;
	int n;
	CVector d;
	float x,y,z;
	float r;

	pnc0[0]=0; pnc1[0]=0; pnc2[0]=0;
	pnc0[1]=0; pnc1[1]=0; pnc2[1]=1;
	pnc0[2]=0; pnc1[2]=1; pnc2[2]=0;
	pnc0[3]=0; pnc1[3]=1; pnc2[3]=1;
	pnc0[4]=1; pnc1[4]=0; pnc2[4]=0;
	pnc0[5]=1; pnc1[5]=0; pnc2[5]=1;
	pnc0[6]=1; pnc1[6]=1; pnc2[6]=0;
	pnc0[7]=1; pnc1[7]=1; pnc2[7]=1;

	segm0[ 0]=0; segm1[ 0]=1;
	segm0[ 1]=1; segm1[ 1]=3;
	segm0[ 2]=2; segm1[ 2]=3;
	segm0[ 3]=0; segm1[ 3]=2;
	segm0[ 4]=0; segm1[ 4]=4;
	segm0[ 5]=4; segm1[ 5]=6;
	segm0[ 6]=2; segm1[ 6]=6;
	segm0[ 7]=1; segm1[ 7]=5;
	segm0[ 8]=4; segm1[ 8]=5;
	segm0[ 9]=3; segm1[ 9]=7;
	segm0[10]=5; segm1[10]=7;
	segm0[11]=6; segm1[11]=7;

	minx0=miny0=minz0=+LIMITEMETA;
	maxx0=maxy0=maxz0=-LIMITEMETA;

    if (equi>0)
	for (n=0;n<nMetaballs;n++)
	{
		x=Metaballs[n].v.x;
		y=Metaballs[n].v.y;
		z=Metaballs[n].v.z;
		r=Metaballs[n].p/equi;
		if (minx0>x-r) minx0=x-r;
		if (maxx0<x+r) maxx0=x+r;
		if (miny0>y-r) miny0=y-r;
		if (maxy0<y+r) maxy0=y+r;
		if (minz0>z-r) minz0=z-r;
		if (maxz0<z+r) maxz0=z+r;
	}

    if (equi>0)
	for (n=0;n<nMetaballsTube;n++)
	{
		x=MetaballsTube[n].a.x;
		y=MetaballsTube[n].a.y;
		z=MetaballsTube[n].a.z;
		r=2*MetaballsTube[n].pa/equi;
		if (minx0>x-r) minx0=x-r;
		if (maxx0<x+r) maxx0=x+r;
		if (miny0>y-r) miny0=y-r;
		if (maxy0<y+r) maxy0=y+r;
		if (minz0>z-r) minz0=z-r;
		if (maxz0<z+r) maxz0=z+r;

		x=MetaballsTube[n].b.x;
		y=MetaballsTube[n].b.y;
		z=MetaballsTube[n].b.z;
		r=MetaballsTube[n].pb/equi;
		if (minx0>x-r) minx0=x-r;
		if (maxx0<x+r) maxx0=x+r;
		if (miny0>y-r) miny0=y-r;
		if (maxy0<y+r) maxy0=y+r;
		if (minz0>z-r) minz0=z-r;
		if (maxz0<z+r) maxz0=z+r;
	}


#define K 1.0f

	minx0-=K;
	miny0-=K;
	minz0-=K;
	maxx0+=K;
	maxy0+=K;
	maxz0+=K;

    if (DIVISION>0)
    {
        minx=(int) (minx0/DIVISION);
        miny=(int) (miny0/DIVISION);
        minz=(int) (minz0/DIVISION);

        maxx=(int) (maxx0/DIVISION);
        maxy=(int) (maxy0/DIVISION);
        maxz=(int) (maxz0/DIVISION);
    }
    
	Orientation=1;

	nVertices=0;
	nFaces=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMetaballs::CalculateIntersectionSegment(int n0,int n1)
{
	int I;

	if (Segments[n0][n1]==-1)
	{
		I=EquiPotentielle(pts[n0],pts[n1],e[n0],e[n1]);
		Segments[n0][n1]=I;
		Segments[n1][n0]=I;
	}
	else
	{
		I=Segments[n0][n1];
	}

	return I;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMetaballs::UploadVertices(CVector &A)
{
	float x,y,z;
	
	for (int n=nVertices-1;n>=posVertices;n--)
	{
		short int d=actual-opt[n];
		if ((d<=1)&&(d>=0))
		{
			x=Vertices[n].x - A.x;
			y=Vertices[n].y - A.y;
			z=Vertices[n].z - A.z;
			
			if (x*x+y*y+z*z<SMALLF) return n;
		}
	}

	opt[nVertices]=actual;
	Vertices[nVertices++]=A;
	return nVertices-1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::UploadTriangle(int ref,int A,int B,int C)
{
	int sens;

	sens=0;
	if (e[ref]<equi) sens=1;
	if (impair==1) sens=(sens+1)&1;

	if (sens==Orientation)
	{
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=B;
		Faces[nFaces].n2=C;
		nFaces++;
	}
	else
	{
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=C;
		Faces[nFaces].n2=B;
		nFaces++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::UploadQuad(int ref,int A,int B,int C,int D)
{
	int sens;

	sens=0;
	if (e[ref]<equi) sens=1;
	if (impair==1) sens=(sens+1)&1;

	if (sens==Orientation)
	{
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=B;
		Faces[nFaces].n2=C;
		nFaces++;
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=C;
		Faces[nFaces].n2=D;
		nFaces++;
	}
	else
	{
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=C;
		Faces[nFaces].n2=B;
		nFaces++;
		Faces[nFaces].n0=A;
		Faces[nFaces].n1=D;
		Faces[nFaces].n2=C;
		nFaces++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::CalcTetrahedron(int n0,int n1,int n2,int n3)
{
	int e01,e02,e03,e00;
	int e12,e23,e31;
	int I1,I2,I3,I4;

	e01=e02=e03=0;

	if ((e[n0]>equi)&&(e[n1]<equi)) e01=1;
	else
	if ((e[n0]<equi)&&(e[n1]>equi)) e01=1;

	if ((e[n0]>equi)&&(e[n2]<equi)) e02=1;
	else
	if ((e[n0]<equi)&&(e[n2]>equi)) e02=1;

	if ((e[n0]>equi)&&(e[n3]<equi)) e03=1;
	else
	if ((e[n0]<equi)&&(e[n3]>equi)) e03=1;

	if (e01+e02+e03==3)
	{
		I1=CalculateIntersectionSegment(n0,n1);
		I2=CalculateIntersectionSegment(n0,n2);
		I3=CalculateIntersectionSegment(n0,n3);
		UploadTriangle(n0,I1,I2,I3);
	}

	if (e01+e02+e03==2)
	{
		if ((e01==1)&&(e02==1))
		{
			I1=CalculateIntersectionSegment(n0,n1);
			I2=CalculateIntersectionSegment(n0,n2);
		}
		else
		if ((e02==1)&&(e03==1))
		{
			I1=CalculateIntersectionSegment(n0,n2);
			I2=CalculateIntersectionSegment(n0,n3);
		}
		else
		if ((e03==1)&&(e01==1))
		{
			I1=CalculateIntersectionSegment(n0,n3);
			I2=CalculateIntersectionSegment(n0,n1);
		}

		e12=e23=e31=0;

		if ((e[n1]>equi)&&(e[n2]<equi)) e12=1;
		else
		if ((e[n1]<equi)&&(e[n2]>equi)) e12=1;

		if ((e[n2]>equi)&&(e[n3]<equi)) e23=1;
		else
		if ((e[n2]<equi)&&(e[n3]>equi)) e23=1;

		if ((e[n3]>equi)&&(e[n1]<equi)) e31=1;
		else
		if ((e[n3]<equi)&&(e[n1]>equi)) e31=1;

		e00=0;
		if ((e12==1)&&(e23==1))
		{
			e00=n2;
			I3=CalculateIntersectionSegment(n1,n2);
			I4=CalculateIntersectionSegment(n2,n3);
		}
		else
		if ((e31==1)&&(e23==1))
		{
			e00=n3;
			I3=CalculateIntersectionSegment(n2,n3);
			I4=CalculateIntersectionSegment(n3,n1);
		}
		else
		if ((e31==1)&&(e12==1))
		{
			e00=n1;
			I3=CalculateIntersectionSegment(n3,n1);
			I4=CalculateIntersectionSegment(n1,n2);
		}

		UploadQuad(e00,I1,I2,I3,I4);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::CalcTetrahedronTri(int n0,int n1,int n2,int n3)
{
	int e01,e02,e03;
	int I1,I2,I3;
	
	e01=e02=e03=0;

	if ((e[n0]>equi)&&(e[n1]<equi)) e01=1;
	else
	if ((e[n0]<equi)&&(e[n1]>equi)) e01=1;

	if ((e[n0]>equi)&&(e[n2]<equi)) e02=1;
	else
	if ((e[n0]<equi)&&(e[n2]>equi)) e02=1;

	if ((e[n0]>equi)&&(e[n3]<equi)) e03=1;
	else
	if ((e[n0]<equi)&&(e[n3]>equi)) e03=1;

	if (e01+e02+e03==3)
	{
		I1=CalculateIntersectionSegment(n0,n1);
		I2=CalculateIntersectionSegment(n0,n2);
		I3=CalculateIntersectionSegment(n0,n3);
		UploadTriangle(n0,I1,I2,I3);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void CMetaballs::chg(int n,int m)
{
	int xchg[3];
	xchg[0]=pnca0[n];
	xchg[1]=pnca1[n];
	xchg[2]=pnca2[n];
	pnca0[n]=pnca0[m];
	pnca1[n]=pnca1[m];
	pnca2[n]=pnca2[m];
	pnca0[m]=xchg[0];
	pnca1[m]=xchg[1];
	pnca2[m]=xchg[2];
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMetaballs::Calculate()
{
	const int T[5][4] = { { 4,1,7,2 },{ 0,1,4,2 },{ 5,7,4,1 },{ 3,1,2,7 },{ 6,2,4,7 } };	
	int l,n,m;
	int i;
	int ml,mm,mn,nn;
	CVector p,u;
	CVector2 Map;
	CVector G;
	int pos=0;
	int mx,my,mz,adr2,nrx,nry,nrz;
	int dimx,dimy,dimz;
	unsigned int adr;
	int ADD=3;
	float x,y,z;
	int poz[4096];

    if ((nMetaballs==0)&&(nMetaballsTube==0))
    {
        OBJ.nFaces=0;
        OBJ.nVertices=0;
        return -1;
    }
    
	if (vertexbuffer_update) K_ITERATIONS=1;

	Init();

	impair=0;

	minx-=ADD;
	miny-=ADD;
	minz-=ADD;

	maxx+=ADD;
	maxy+=ADD;
	maxz+=ADD;

	dimx=(maxx-minx);
	dimy=(maxy-miny);
	dimz=(maxz-minz);
    
    METABALLS_MAXDATA=dimx*dimy*dimz;

    Vertices=new CVector[METABALLS_MAXDATA];
    opt=new short int[METABALLS_MAXDATA];
    Faces=new CShortF[METABALLS_MAXDATA];

	int xx=dimx+1;
	int xy=xx*(dimy+1);

	potentiel_xyz=(float*) malloc((dimx+1)*(dimy+1)*(dimz+1)*sizeof(float));	

	for (n=0;n<(dimx+1)*(dimy+1)*(dimz+1);n++) potentiel_xyz[n]=-1.0f;

	float divinv=1.0f/DIVISION;

	for (nn=0;nn<nMetaballs;nn++)
	{
		p=Metaballs[nn].v;

		mx=(int)(p.x*divinv - minx);
		my=(int)(p.y*divinv - miny);
		mz=(int)(p.z*divinv - minz);

		#define NR 4

		mx-=NR;
		my-=NR;
		mz-=NR;

		nrz=nry=nrx=NR*2;

		if (mx<0) { nrx+=mx;mx=0;}
		if (my<0) { nry+=my;my=0;}
		if (mz<0) { nrz+=mz;mz=0;}

		if (mx+nrx>dimx) { nrx=dimx-mx;}
		if (my+nry>dimy) { nry=dimy-my;}
		if (mz+nrz>dimz) { nrz=dimz-mz;}

		adr=((dimy+1)*mz + my)*(dimx+1) +mx;

		for (l=0;l<nrz;l++)
		{
			adr2=adr;
			for (m=0;m<nry;m++)
			{
				for (n=0;n<nrx;n++) potentiel_xyz[adr+n]=0.0f;
				adr+=(dimx+1);
			}
			adr=adr2+xy;
		}
	}

	for (nn=0;nn<nMetaballsTube;nn++)
	{
		for (int k=0;k<20;k++)
		{
			p=MetaballsTube[nn].a + (k*(MetaballsTube[nn].b-MetaballsTube[nn].a))/20.0f;

			mx=(int)(p.x*divinv - minx);
			my=(int)(p.y*divinv - miny);
			mz=(int)(p.z*divinv - minz);

			#define NR 4

			mx-=NR;
			my-=NR;
			mz-=NR;

			nrz=nry=nrx=NR*2;

			if (mx<0) { nrx+=mx;mx=0;}
			if (my<0) { nry+=my;my=0;}
			if (mz<0) { nrz+=mz;mz=0;}

			if (mx+nrx>dimx) { nrx=dimx-mx;}
			if (my+nry>dimy) { nry=dimy-my;}
			if (mz+nrz>dimz) { nrz=dimz-mz;}

			adr=((dimy+1)*mz + my)*(dimx+1) +mx;

			for (l=0;l<nrz;l++)
			{
				adr2=adr;
				for (m=0;m<nry;m++)
				{
					for (n=0;n<nrx;n++) potentiel_xyz[adr+n]=0.0f;
					adr+=(dimx+1);
				}
				adr=adr2+xy;
			}
		}
	}

	x=minx*DIVISION;
	y=miny*DIVISION;
	z=minz*DIVISION;

	adr=0;
	p.x=x;
	for (l=minx;l<=maxx;l++)
	{
		p.y=y;
		for (m=miny;m<=maxy;m++)
		{
			adr=l-minx + (m-miny)*xx;
			p.z=z;
			for (n=0;n<=maxz-minz;n++)
			{
				if (potentiel_xyz[adr]==0.0f) potentiel_xyz[adr]=Potentiel(p);
				p.z+=DIVISION;
				adr+=xy;
			}
			p.y+=DIVISION;
		}
		p.x+=DIVISION;
	}

	posVertices=0;
	for (i=0;i<maxy-miny;i++) poz[i]=0;

	x=minx*DIVISION;
	for (l=minx;l<maxx;l++)
	{		
		y=miny*DIVISION;
		for (m=miny;m<maxy;m++)
		{
			z=minz*DIVISION;
			int adrr0=l-minx + (m-miny)*xx;

			posVertices=poz[m-miny];
			poz[m-miny]=nVertices;

			for (n=minz;n<maxz;n++)
			{
				actual=n-minz;

				ml=l&1;
				mm=m&1;
				mn=n&1;
				nn=0;
				for (i=0;i<8;i++)
				{
					pnca0[i]=pnc0[i];
					pnca1[i]=pnc1[i];
					pnca2[i]=pnc2[i];
				}

				impair=0;
				if (ml==1)
				{
					for (i=0;i<4;i++) chg(i,i+4);
					impair=(impair+1)&1;
				}

				if (mm==1)
				{
					impair=(impair+1)&1;
					chg(0,2);chg(1,3);chg(4,6);chg(5,7);
				}

				if (mn==1)
				{
					impair=(impair+1)&1;
					chg(0,1);chg(2,3);chg(4,5);chg(6,7);
				}

				for (i=0;i<8;i++)
				{
					int adrr=adrr0;
					if (pnca0[i]) { adrr++; pts[i].x=x+DIVISION; } else pts[i].x=x;
					if (pnca1[i]) { adrr+=xx; pts[i].y=y+DIVISION; } else pts[i].y=y;
					if (pnca2[i]) { adrr+=xy; pts[i].z=z+DIVISION; } else pts[i].z=z;
					e[i]=potentiel_xyz[adrr];
					if (e[i]>equi) nn++;
				}

				if ((nn>0)&&(nn<8))
				{
					for (int p=0;p<64;p++) ((int*)Segments)[p]=-1;

					for (i=0;i<5;i++)
					{
						CalcTetrahedron(T[i][0],T[i][1],T[i][2],T[i][3]);
						CalcTetrahedronTri(T[i][1],T[i][2],T[i][0],T[i][3]);
						CalcTetrahedronTri(T[i][2],T[i][3],T[i][0],T[i][1]);
						CalcTetrahedronTri(T[i][3],T[i][0],T[i][2],T[i][1]);
					}
				}
				z+=DIVISION;
				adrr0+=xy;
			}
			y+=DIVISION;
		}
		x+=DIVISION;
	}

	if (status==0)
	{
		OBJ.Init(nVertices,nFaces);
	}
	else
	{
		OBJ.Free();
		OBJ.Init(nVertices,nFaces);
	}

	status=1;

	G.Init(0,0,0);

	if (gravitycenter)
	{
		for (n=0;n<nVertices;n++) VECTORADD(G,G,Vertices[n]);
		VECTORDIV(G,G,(float) nVertices);
	}

	Pos=G;

	if (vertexbuffer_update)
	{
		if (VB.init) VB.Release();

		if (ENVMAPPING)
		{
			VB.SetType(API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX|API3D_ENVMAPPING);
			VB.Init(nVertices,nFaces*3);
		}
		else
		{
			VB.SetType(API3D_VERTEXDATAS|API3D_COLORDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
			VB.Init(nVertices,nFaces*3);
		}
	}

	// face indices
	if (vertexbuffer_update) VB.LockIndices();
	for (n=0;n<nFaces;n++)
	{
		if (vertexbuffer_update) VB.SetIndices(n,Faces[n].n0,Faces[n].n1,Faces[n].n2);
		OBJ.Faces[n].v0=Faces[n].n0;
		OBJ.Faces[n].v1=Faces[n].n1;
		OBJ.Faces[n].v2=Faces[n].n2;
	}
	if (vertexbuffer_update) VB.UnlockIndices();

	// vertex data
	if (vertexbuffer_update) VB.LockVertices();
	for (n=0;n<nVertices;n++)
	{
		if (gravitycenter) { VECTORSUB(OBJ.Vertices[n].Stok,Vertices[n],G); }
		else OBJ.Vertices[n].Stok=Vertices[n];
		if (vertexbuffer_update) VB.SetVertex(n,OBJ.Vertices[n].Stok);
	}

	OBJ.nFaces=nFaces;
	OBJ.nVertices=nVertices;
	OBJ.SetFaces();

	for (n=0;n<nVertices;n++)
	{	
		CVector N=Gradiant(Vertices[n]);
		OBJ.Vertices[n].Norm=N;
		if (ENVMAPPING)
		{
			if (vertexbuffer_update) VB.SetNormal(n,N);
			Map.x=0.5f+N.x/2;
			Map.y=0.0f+N.y/2;
			if (vertexbuffer_update) VB.SetTexCoo(n,Map);
		}
		else
		{
			Map.x=0.5f+N.x/4+N.z/4;
			Map.y=0.5f+N.y/2;
			if (vertexbuffer_update) VB.SetTexCoo(n,Map);
		}
		OBJ.Vertices[n].Map=Map;
	}

	if (vertexbuffer_update) VB.UnlockVertices();

	OBJ.Coo=G;

	sprintf(OBJ.Name,"Metaballs");

	OBJ.Radius=-1;
	if (vertexbuffer_update) VB.nVerticesActif=nVertices;
	if (vertexbuffer_update) VB.nIndicesActif=nFaces*3;

	free(potentiel_xyz);
    
    potentiel_xyz=NULL;

	delete [] Vertices;
	delete [] Faces;
	delete [] opt;

	Vertices=NULL;
	Faces=NULL;
	opt=NULL;

	return (maxx-minx)*(maxy-miny)*(maxz-minz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMetaballs::Free()
{
	status=0;
	if (vertexbuffer_update) VB.Release();
	OBJ.Free();
	potentiel_xyz=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
