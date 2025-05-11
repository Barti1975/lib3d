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
//	@file physics.cpp 
//	@created 2002
////////////////////////////////////////////////////////////////////////

#include "params.h"

#include <math.h>
#include <stdio.h>

#include "maths.h"
#include "objects3d.h"
#include "list.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// detection inside face pob/ball
#define SMALLFFACEINT SMALLF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern int PIVOT_JOINT_DIM;
extern int MOBIL_JOINT_DIM;
extern int BALL_JOINT_DIM;
extern int SPRING_JOINT_DIM;
extern int FIXED_JOINT_DIM;
extern int FIXED_PIVOT_DIM;

#ifdef _DEFINES_API_CODE_PHYSIC_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float COEF_ENVELOPPE=0.0f;
float CONSTANT_STATIC_VELOCITY=0.0f;
float CONSTANT_STATIC_TORQUE=0.0f;
float CONSTANT_STATIC_VELOCITY_SQUARE=0.0f;
float CONSTANT_STATIC_TORQUE_SQUARE=0.0f;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CONSTANT_STATIC_REPLACEMENT=0.1f;
float CONSTANT_STATIC_WATER_MASS=5.0f;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector DYNAMIC_WIND_FORCE;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SLIDING 0.1f
#define QUATERNIONS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetPhysicWindForce(CVector F)
{
	DYNAMIC_WIND_FORCE=F;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setScaleTimeMul(float sc)
{
	K_MUL_ITERATIONS=sc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetApproximateTimeCollision(bool value)
{
	ReplaceObjects=value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetApproximateTimeCollisionIterations(bool value)
{
	ReplaceObjectsIterations=value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::disableScaleTime()
{
	scale_time=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPhysic::enableScaleTime()
{
	scale_time=true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCollisionMesh *CPhysic::AddMesh(CObject3D * obj,float r,float f,int N)
{
	int n;
	CCollisionMesh *cm;

	obj->Calculate();

	CollisionMeshes[nCollisionMeshes].Init(obj,r,f,N);
	cm=&CollisionMeshes[nCollisionMeshes];	
	for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=MAX_FACES_PER_OBJECT*nCollisionMeshes;
	obj->update=false;

	for (n=0;n<obj->nVertices;n++)
	{
		float y=obj->Vertices[n].Calc.y+10.0f;
		if (y>y_max) y_max=y;
	}

	CollisionMeshes[nCollisionMeshes].aCoo=CollisionMeshes[nCollisionMeshes].bCoo=obj->Coo;
	CollisionMeshes[nCollisionMeshes].aRot=CollisionMeshes[nCollisionMeshes].bRot=obj->Rot;

	nCollisionMeshes++;

	return cm;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::NewObject(CPhysicObject * op)
{
	NewObject(op,false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::NewObject(CPhysicObject * op,bool link)
{
	int n;
	float rmax,r;

	Array[nArray]=op;
	op->index=nArray;
	op->Handled=false;
	op->Linked=link;
	op->support=NULL;
	nArray++;

	rmax=0.0f;

	for (n=0;n<nArray-1;n++) 
	{ 
		if (Array[n]->Linked==false)
		{
			if (IsObj1CollidingObj2(nArray-1,n,0,0)==1) 
			{
				r=Array[n]->Radius;
				if (r>rmax)
				{
					rmax=r;
					op->support=Array[n]; 
				}
			}
		}
	}

	float y=op->Pos.y+10.0f;
	if (y>y_max) y_max=y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseJoint(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2)
{
	BaseJoints[nBaseJoints]=new CPhysicJoint;
	op1->Linked=true;
	op2->Linked=true;

	op1->Links[op1->nLinks]=r1;
	VECTORNORMALISE(op1->Links[op1->nLinks]);
	op1->nLinks++;

	op2->Links[op2->nLinks]=r2;
	VECTORNORMALISE(op2->Links[op2->nLinks]);
	op2->nLinks++;

	BaseJoints[nBaseJoints]->type=MOBIL_JOINT;
	BaseJoints[nBaseJoints]->Object1=op1;
	BaseJoints[nBaseJoints]->Object2=op2;
	BaseJoints[nBaseJoints]->r1=r1;
	BaseJoints[nBaseJoints]->r2=r2;
	BaseJoints[nBaseJoints]->NextJoint=0;
	nBaseJoints++;
	return BaseJoints[nBaseJoints-1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseJointSpring(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,float K)
{
    BaseJoints[nBaseJoints]=new CPhysicJoint;
    op1->Linked=true;
    op2->Linked=true;
    
    op1->Links[op1->nLinks]=r1;
    VECTORNORMALISE(op1->Links[op1->nLinks]);
    op1->nLinks++;
    
    op2->Links[op2->nLinks]=r2;
    VECTORNORMALISE(op2->Links[op2->nLinks]);
    op2->nLinks++;
    
    BaseJoints[nBaseJoints]->type=SPRING_JOINT;
    BaseJoints[nBaseJoints]->Object1=op1;
    BaseJoints[nBaseJoints]->Object2=op2;
    BaseJoints[nBaseJoints]->r1=r1;
    BaseJoints[nBaseJoints]->r2=r2;
    BaseJoints[nBaseJoints]->L1=r1.Norme();
    BaseJoints[nBaseJoints]->L2=r2.Norme();
    CVector u=op1->Pos-op2->Pos;
    BaseJoints[nBaseJoints]->L=u.Norme();
    BaseJoints[nBaseJoints]->dL=0;
    BaseJoints[nBaseJoints]->raideur=K;
    BaseJoints[nBaseJoints]->NextJoint=0;
    nBaseJoints++;
    return BaseJoints[nBaseJoints-1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseJointBall(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2)
{
	BaseJoints[nBaseJoints]=new CPhysicJoint;
	op1->Linked=true;
	op2->Linked=true;

	op1->Links[op1->nLinks]=r1;
	VECTORNORMALISE(op1->Links[op1->nLinks]);
	op1->nLinks++;

	op2->Links[op2->nLinks]=r2;
	VECTORNORMALISE(op2->Links[op2->nLinks]);
	op2->nLinks++;

	BaseJoints[nBaseJoints]->type=BALL_JOINT;
	BaseJoints[nBaseJoints]->Object1=op1;
	BaseJoints[nBaseJoints]->Object2=op2;
	BaseJoints[nBaseJoints]->r1=r1;
	BaseJoints[nBaseJoints]->r2=r2;
	BaseJoints[nBaseJoints]->NextJoint=0;
	nBaseJoints++;
	return BaseJoints[nBaseJoints-1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseJointPivot(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,CVector axe1,CVector axe2)
{
	BaseJoints[nBaseJoints]=new CPhysicJoint;
	op1->Linked=true;
	op2->Linked=true;

	op1->Links[op1->nLinks]=r1;
	VECTORNORMALISE(op1->Links[op1->nLinks]);
	op1->nLinks++;

	op2->Links[op2->nLinks]=r2;
	VECTORNORMALISE(op2->Links[op2->nLinks]);
	op2->nLinks++;

	BaseJoints[nBaseJoints]->type=PIVOT_JOINT;
    BaseJoints[nBaseJoints]->Object1=op1;
    BaseJoints[nBaseJoints]->Object2=op2;
    BaseJoints[nBaseJoints]->axis1=axe1;
    BaseJoints[nBaseJoints]->axis2=axe2;
	BaseJoints[nBaseJoints]->r1=r1;
	BaseJoints[nBaseJoints]->r2=r2;
	BaseJoints[nBaseJoints]->NextJoint=0;
	nBaseJoints++;
	return BaseJoints[nBaseJoints-1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicObject FIXED;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseJointFixed(CVector pos,CPhysicObject *op,CVector r)
{
    CPhysicJoint *joint;
    
    joint=BaseJoints[nBaseJoints]=new CPhysicJoint;
    nBaseJoints++;
    
    op->Linked=true;
    joint->type=FIXED_JOINT;
    joint->Object1=op;
    op->Links[op->nLinks]=r;
    VECTORNORMALISE(op->Links[op->nLinks]);
    op->nLinks++;
    joint->Object2=&FIXED;
    joint->Object2->Fixe=true;
    joint->r1=r;
    joint->ptfixe=pos;
    joint->NextJoint=0;
    return joint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBasePivotFixed(CVector pos,CPhysicObject *op,CVector r,CVector axe)
{
    CPhysicJoint *joint;
    
    joint=BaseJoints[nBaseJoints]=new CPhysicJoint;
    nBaseJoints++;
    
    op->Linked=true;
    joint->type=FIXED_PIVOT;
    joint->Object1=op;
    op->Links[op->nLinks]=r;
    VECTORNORMALISE(op->Links[op->nLinks]);
    op->nLinks++;
    joint->Object2=&FIXED;
    joint->Object2->Fixe=true;
    joint->r1=r;
    joint->axis1=axe;
    joint->ptfixe=pos;
    joint->NextJoint=0;
    return joint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint * CPhysic::AddBaseSpringFixed(CVector pos,CPhysicObject *op,CVector r,float K)
{
    CPhysicJoint *joint;
    
    joint=BaseJoints[nBaseJoints]=new CPhysicJoint;
    nBaseJoints++;
    
    op->Linked=true;
    joint->type=FIXED_SPRING;
    joint->Object1=op;
    op->Links[op->nLinks]=r;
    VECTORNORMALISE(op->Links[op->nLinks]);
    op->nLinks++;
    joint->Object2=&FIXED;
    joint->Object2->Fixe=true;
    joint->r1=r;
    
    joint->L1=r.Norme();
    joint->L=r.Norme();
    joint->dL=0;
    joint->raideur=K;
    
    joint->ptfixe=pos;
    joint->NextJoint=0;
    return joint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetGravity(float g)
{
	Gravite=g;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::AddPlan(float a,float b,float c,float d,float k,float f)
{
	Restitution[nPlanes]=k;
	Friction[nPlanes]=f;
	Planes[nPlanes].Init(a,b,c,d);
	Planes[nPlanes].Normalise();
	nPlanes++;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::AddPlanWater(float a,float b,float c,float d,float k,float f)
{
	Restitution[nPlanes]=-k;
	Friction[nPlanes]=f;
	Planes[nPlanes].Init(a,b,c,d);
	Planes[nPlanes].Normalise();
	nPlanes++;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetIterations(int niter)
{
	N_ITERATIONS=niter;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ResetScenePlanes()
{
	nPlanes=0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IndexProp(CPhysicObject * prop)
{
	int n=0;
	int res=-1;
	while ((n<nArray)&&(res<0))
	{
		if (Array[n]==prop) res=n;
		n++;
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::TagCollisionLinks()
{
	int n,n1,n2;
	CPhysicJoint *joint;

	for (n=0;n<nBaseJoints;n++)
	{
		joint=BaseJoints[n];
		while (joint)
		{
			n1=n2=-1;
			if (joint->Object1) n1=IndexProp(joint->Object1);
			if (joint->Object2) n2=IndexProp(joint->Object2);

			if ((n1>=0)&&(n2>=0)) NoCollision[n1 + n2*MAX_PHYSIC_OBJECTS_SCENE]=NoCollision[n2 + n1*MAX_PHYSIC_OBJECTS_SCENE]=1;

			joint=joint->NextJoint;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ResetObjsArray()
{
	int n;
	CPhysicJoint *j,*joint;

	nArray=0;
	for (n=0;n<nBaseJoints;n++)
	{
		j=BaseJoints[n];
		while (j)
		{
			joint=j;
			j=j->NextJoint;
			if (joint->Object2->Fixe) delete joint->Object2;
			delete joint;
		}
	}
	nBaseJoints=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::Free()
{
	Reset();
	ParticlesFree();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::Reset()
{
	int n;
	CPhysicJoint *j,*joint;

	y_max=-100000;

	for (n=0;n<nCollisionMeshes;n++) 
	{
		CollisionMeshes[n].quad.clean();
		CollisionMeshes[n].quad.Free();
	}

	nCollisionMeshes=0;

	nArray=0;
	nPlanes=0;
	for (n=0;n<nBaseJoints;n++)
	{
		j=BaseJoints[n];
		while (j)
		{
			joint=j;
			j=j->NextJoint;

			if (joint->Object2!=&FIXED)
				if (joint->Object2->Fixe) delete joint->Object2;
			delete joint;
		}
	}
	nBaseJoints=0;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InitCube_x(float lx,CVector ux)
{
	Ux=ux;
	Ux.Normalise();
	Lx=lx;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InitCube_y(float ly,CVector uy)
{
	Uy=uy;
	Uy.Normalise();
	Ly=ly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InitCube_z(float lz,CVector uz)
{
	Uz=uz;
	Uz.Normalise();
	Lz=lz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsACube()
{
	Type=TYPE_CUBE;
	R.Id();
	enh=false;
	OBJECT=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsASphereTree()
{
	STREE=new CSphereTree;
	Type=TYPE_SPHERETREE;
	enh=false;
	OBJECT=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsAPob()
{
	Type=TYPE_POB;
	R.Id();
	enh=false;
	OBJECT=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsAMesh()
{
	Type=TYPE_OBJECT;
	R.Id();
	enh=false;
	ball_inertiatensor=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsAMeshBall()
{
	Type=TYPE_OBJECT;
	R.Id();
	enh=false;
	ball_inertiatensor=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsAFullMesh()
{
	Type=TYPE_OBJECT;
	enh=true;
	R.Id();
	ball_inertiatensor=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsASphere()
{
	Type=TYPE_SPHERE;
	R.Id();
	enh=false;
	OBJECT=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::SetMass(float m)
{
	Mass=m;
	InvMass=1.0f/m;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::SetRestitution(float k)
{
	Restitution=k;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InitRadius(float r)
{
	Radius=r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::Fix()
{
    FixM=ActualMatrix();
    FixMo=ActualOrientationMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CPhysicObject::ActualMatrix()
{
    CMatrix M;

#ifdef R_AND_ANGLEVECTEUR
    CVector u;
    u=Omega;
    u.Normalise();
    M=R;                                // orientation � t0
    M.RotationAngleVecteur(u,Alpha);    // mvt actuel autour de l'axe u
    M.Translation(Pos.x,Pos.y,Pos.z);    // translation
#endif

#ifdef QUATERNIONS

    M.OrientationQuaternion(Q);
    M.Translation(Pos.x,Pos.y,Pos.z);    // translation
    
#endif
    return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CPhysicObject::ActualOrientationMatrix()
{
    CMatrix M;

#ifdef R_AND_ANGLEVECTEUR
    CVector u;
    M=R;
    u=Omega;
    u.Normalise();
    M.RotationAngleVecteur(u,Alpha);
#endif

#ifdef QUATERNIONS
    M.OrientationQuaternion(Q);
#endif

    return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphere1CollidingSphere2(int n1,int n2,int Calculate,int buf)
{
	CVector D;
	float r,f;
	int res=0;

	VECTORSUB(D,Array[n2]->Pos,Array[n1]->Pos);
	f=VECTORNORM2(D);
	r=Array[n1]->Radius+Array[n2]->Radius;
	if (f<r*r)
	{
		res=1;
		if (Calculate==1)
		{
			f=_M_invsqrtf(f);
			VECTORMUL(D,D,f);
			ContactStruct * cs=Contacts[buf].InsertLast();
			cs->Normals.x=-D.x;
			cs->Normals.y=-D.y;
			cs->Normals.z=-D.z;
			cs->Point.x=Array[n1]->Pos.x+Array[n1]->Radius*D.x;
			cs->Point.y=Array[n1]->Pos.y+Array[n1]->Radius*D.y;
			cs->Point.z=Array[n1]->Pos.z+Array[n1]->Radius*D.z;
			cs->Types=COLLISION_WITH_OBJECT;
			cs->Num=n2;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBall1CollidingSphere2(int n1,int n2,CVector &P,float R,int Calculate,int buf)
{
	CVector D;
	float r,f;
	int res;

	res=0;
	VECTORSUB(D,Array[n2]->Pos,P);
	r=R+Array[n2]->Radius;
	f=VECTORNORM(D);
	if (f<r)
	{
		res=1;
		if (Calculate==1)
		{
			VECTORDIV(D,D,f);

			ContactStruct * cs=Contacts[buf].InsertLast();

			cs->Normals.x=-D.x;
			cs->Normals.y=-D.y;
			cs->Normals.z=-D.z;
			cs->Point.x=P.x+R*D.x;
			cs->Point.y=P.y+R*D.y;
			cs->Point.z=P.z+R*D.z;
			cs->Types=COLLISION_WITH_OBJECT;
			cs->Num=n2;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphere1CollidingBall2(int n1,int n2,CVector &P,float R,int Calculate,int buf)
{
	CVector D;
	float r,f;
	int res;

	res=0;
	VECTORSUB(D,P,Array[n1]->Pos);
	r=Array[n1]->Radius + R;
	f=VECTORNORM(D);
	if (f<r)
	{
		res=1;
		if (Calculate==1)
		{
			VECTORDIV(D,D,f);

			ContactStruct * cs=Contacts[buf].InsertLast();
			cs->Normals.x=-D.x;
			cs->Normals.y=-D.y;
			cs->Normals.z=-D.z;
			cs->Point.x=Array[n1]->Pos.x+Array[n1]->Radius*D.x;
			cs->Point.y=Array[n1]->Pos.y+Array[n1]->Radius*D.y;
			cs->Point.z=Array[n1]->Pos.z+Array[n1]->Radius*D.z;
			cs->Types=COLLISION_WITH_OBJECT;
			cs->Num=n2;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBall1CollidingBall2(int n1,int n2,CVector &P1,float R1,CVector &P2,float R2,int Calculate,int buf)
{
	CVector D;
	float r,f;
	int res;

	res=0;
	VECTORSUB(D,P2,P1);
	r=R2 + R2;
	f=VECTORNORM2(D);
	if (f<r*r)
	{
		res=1;
		if (Calculate==1)
		{
		    f=1.0f/sqrtf(f);
			VECTORMUL(D,D,f);

			ContactStruct * cs=Contacts[buf].InsertLast();
			cs->Normals.x=-D.x;
			cs->Normals.y=-D.y;
			cs->Normals.z=-D.z;
			cs->Point.x=P1.x+R1*D.x;
			cs->Point.y=P1.y+R1*D.y;
			cs->Point.z=P1.z+R1*D.z;
			cs->Types=COLLISION_WITH_OBJECT;
			cs->Num=n2;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector ProjectionPointSurAxe(CVector &P,CVector &O,CVector &u)
{
	CVector Pp;
	float s1,s2;
	CVector OP,A,B;
	float t;
	float a,b,c,d;

	a=u.x;
	b=u.y;
	c=u.z;
	d=-(a*P.x+b*P.y+c*P.z);
	A=O;
	B=O+5*u;
	s1=a*A.x + b*A.y +c*A.z +d;
	s2=a*B.x + b*B.y +c*B.z +d;
	t=-s1/(s2-s1);
	Pp=A+t*(B-A);

/*
	OP=P-O;
	ss=(OP||u);
	Pp=O+ss*u;
*/
	return Pp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResoudAttBtC(float a,float b,float c,float * t1,float * t2,float *D)
{
	*D=b*b - 4*a*c;
	if (*D>=0)
	{
		*t1=(-b+(float) sqrtf(*D))/(2*a);
		*t2=(-b-(float) sqrtf(*D))/(2*a);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBall1CollidingCylinder2(int n1,int n2,CVector &O,float R,int Calculate,int buf)
{
	int res;
	int np;

	np=Contacts[buf].Length();
	res=IsCylinder1CollidingBall2(n2,n1,O,R,Calculate,buf);

	if (Calculate)
	{
		ContactStruct * cs=Contacts[buf].SetCurrent(np);
		while (cs)
		{
			cs->Normals.x=-cs->Normals.x;
			cs->Normals.y=-cs->Normals.y;
			cs->Normals.z=-cs->Normals.z;
			cs=Contacts[buf].GetNext();
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinder1CollidingBall2(int n1,int n2,CVector &O,float R,int Calculate,int buf)
{
	int res=0;
	CVector I,H,P,u,v,HI,OI,OPp,Pp,N,OC;

	u=Array[n1]->U*Array[n1]->MOActual;
	VECTORNORMALISE(u);

	I=ProjectionPointSurAxe(O,Array[n1]->Pos,u);

	VECTORSUB(HI,I,O);

	VECTORSUB(N,Array[n1]->Pos,O);
	VECTORNORMALISE(N);

	VECTORSUB(OC,O,Array[n1]->Pos);

	if (VECTORNORM(HI)<Array[n1]->Radius+R)
	{
		if (f_abs(DOT(OC,u))<R+Array[n1]->L/2)
		{
			if (Calculate)
			{
				ContactStruct * cs=Contacts[buf].InsertLast();
				cs->Distance=0;
				cs->Normals=N;
				cs->Point.x=O.x+R*N.x;
				cs->Point.y=O.y+R*N.y;
				cs->Point.z=O.z+R*N.z;
				cs->Types=COLLISION_WITH_OBJECT;
				cs->Num=n2;
				
			}
			res=1;
		}
	}

	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinder1CollidingPobClosing2(int n1,int n2,CPOB *pob,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	const int corresfaces[6][4]={
		{1,3,2,4},
		{0,5,2,4},
		{0,5,1,3},
		{0,5,2,4},
		{0,5,1,3},
		{1,3,2,4} };

	CVector Opob;
	CVector Cube2[8+6];
	CVector Norm[6];
	CVector U1U2[6][2];
	CVector a1,a2,a1a2,a1I,iD,ni;
	float L1L2[6][3];
	float s1,s2;
	CVector ux,uy,uz,OM,v,u1;
	float t,t1,t2;
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,C,I,Pos1,Pos2;
	CMatrix M1,M2,MM1,MM2;
	float rA,rB,DL,rA1,rA2,rA3,rB1,rB2,rB3;
	int n,nn1,nn2,res;
	CVector Ux,Uy,Uz;
	CVector u,N,pA,pB,w1,w2,OI;
	float D,sA,sB;

	if (Array[n1]->pob) return IsPobClosing1CollidingPobClosing2(n1,n2,Array[n1]->pob,Array[n2]->pob,Calculate,buf);

	res=0;

	M1=Array[n1]->MOActual;
	M2=Array[n2]->MOActual;

	MM1=Array[n1]->MActual;
	MM2=Array[n2]->MActual;

	Pos1=Array[n1]->Pos;
	Pos2=pob->O*MM2;
	

	iD=Pos2-Pos1;
/*
	Ux.Init(1,0,0);
	Uy.Init(0,1,0);
	Uz.Init(0,0,1);
*/

	Uz=Array[n1]->U;
	Ux.Base(Uz,0);
	Uy.Base(Uz,1);

	VECTORNORMALISE(Ux);
	VECTORNORMALISE(Uy);
	VECTORNORMALISE(Uz);

	A1=(COEF_ENVELOPPE+Array[n1]->Radius)*(Ux*M1);
	A2=(COEF_ENVELOPPE+Array[n1]->Radius)*(Uy*M1);
	A3=(COEF_ENVELOPPE+Array[n1]->L/2)*(Uz*M1);

	B1=(COEF_ENVELOPPE+pob->Lx)*(pob->Ux*M2);
	B2=(COEF_ENVELOPPE+pob->Ly)*(pob->Uy*M2);
	B3=(COEF_ENVELOPPE+pob->Lz)*(pob->Uz*M2);
	
	Ls[0]=A1;
	Ls[1]=A2;
	Ls[2]=A3;

	Ls[3]=B1;
	Ls[4]=B2;
	Ls[5]=B3;

	Ls[6]=B1+B2;
	Ls[7]=B2+B3;
	Ls[8]=B3+B1;

	Ls[9]=B1+B2+B3;

	Ls[10]=A1+A2;
	Ls[11]=A2+A3;
	Ls[12]=A3+A1;

	Ls[13]=A1+A2+A3;

	for (n=0;n<13;n++) VECTORNORMALISE(Ls[n]);


	n=0;
	res=1;
	while ((res==1)&&(n<13))
	{
		L=Ls[n];
		rA1=DOT(A1,L);
		rA2=DOT(A2,L);
		rA3=DOT(A3,L);
		rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

		rB1=DOT(B1,L);
		rB2=DOT(B2,L);
		rB3=DOT(B3,L);
		rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

		DL=DOT(iD,L);
		DL=f_abs(DL);

		s1=rA+rB - DL;

		if (s1<0) res=0; else n++; 
	}


	if (res)
	{
		res=0;

		VECTOR3MATRIX(u1,Array[n1]->U,M1);
		VECTORNORMALISE(u1);

		VECTORSUB(N,Pos2,Pos1);
		VECTORNORMALISE(N);

		CROSSPRODUCT(ni,u1,N);
		CROSSPRODUCT(v,ni,u1);
		VECTORNORMALISE(v);
		if (f_abs(u1||N)>0.9f) v.Init(0,0,0);

		A=Array[n1]->Pos + (Array[n1]->L/2)*u1 + Array[n1]->Radius*v;
		B=Array[n1]->Pos - (Array[n1]->L/2)*u1 + Array[n1]->Radius*v;

		for (n=0;n<8+6;n++)
		{
			p=pob->O+Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
			Cube2[n]=p*MM2;
		}


		VECTOR3MATRIX(ux,pob->Ux,M2);
		VECTOR3MATRIX(uy,pob->Uy,M2);
		VECTOR3MATRIX(uz,pob->Uz,M2);

		
		Norm[0]=uz;
			U1U2[0][0]=ux;
			U1U2[0][1]=uy;
			L1L2[0][0]=pob->Lx;
			L1L2[0][1]=pob->Ly;
			L1L2[0][2]=pob->Lz;

		Norm[1]=-1*uy;
			U1U2[1][0]=ux;
			U1U2[1][1]=uz;
			L1L2[1][0]=pob->Lx;
			L1L2[1][1]=pob->Lz;
			L1L2[1][2]=pob->Ly;
		
		Norm[2]=-1*ux;
			U1U2[2][0]=uy;
			U1U2[2][1]=uz;
			L1L2[2][0]=pob->Ly;
			L1L2[2][1]=pob->Lz;
			L1L2[2][2]=pob->Lx;

		
		Norm[3]=uy;
			U1U2[3][0]=ux;
			U1U2[3][1]=uz;
			L1L2[3][0]=pob->Lx;
			L1L2[3][1]=pob->Lz;
			L1L2[3][2]=pob->Ly;

		
		Norm[4]=ux;
			U1U2[4][0]=uz;
			U1U2[4][1]=uy;
			L1L2[4][0]=pob->Lz;
			L1L2[4][1]=pob->Ly;
			L1L2[4][2]=pob->Lx;


		Norm[5]=-1*uz;
			U1U2[5][0]=ux;
			U1U2[5][1]=uy;
			L1L2[5][0]=pob->Lx;
			L1L2[5][1]=pob->Ly;
			L1L2[5][2]=pob->Lz;

	
		for (n=0;n<6;n++) VECTORNORMALISE(Norm[n]);
			
//		int nf,nn,toto;


		A=Array[n1]->Pos + (Array[n1]->L/2)*u1 + Array[n1]->Radius*v;
		B=Array[n1]->Pos - (Array[n1]->L/2)*u1 + Array[n1]->Radius*v;


		n=0;
		while (n<6)
		{
			a=Norm[n].x;
			b=Norm[n].y;
			c=Norm[n].z;
			d=-(a*Cube2[Faces[n][0]].x+b*Cube2[Faces[n][0]].y+c*Cube2[Faces[n][0]].z);

			sA=a*A.x+b*A.y+c*A.z+d;
			sB=a*B.x+b*B.y+c*B.z+d;
				
			t=-sA/(sB-sA);

			if ((t>0)&&(t<1)) 
			{
				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Pos2.x+b*Pos2.y+c*Pos2.z);

				sA=a*A.x+b*A.y+c*A.z+d;
				sB=a*B.x+b*B.y+c*B.z+d;

				p=(A+B)/2;

				if ((sA>0)&&(sB>0))
				{
					if (Calculate)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Distance=0;
						cs->Normals=-1*v;
						cs->Point=p;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
					}
					res=1;
				}
			}
			n++;
		}


		n=0;
		while (n<12+3)
		{
			nn1=Arretes[n][0];
			nn2=Arretes[n][1];

			A=Cube2[nn1];
			B=Cube2[nn2];

			pA=ProjectionPointSurAxe(A,Array[n1]->Pos,u1);
			pB=ProjectionPointSurAxe(B,Array[n1]->Pos,u1);


			w1=pA-A;
			w2=pB-B;

			
			
			if ((w1.Norme()<Array[n1]->Radius)&&(w2.Norme()<Array[n1]->Radius))
			{
				if (Calculate)
				{
					I=(A+B)/2;

					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Distance=0;
					cs->Normals=-1*N;
					cs->Point=I;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;
			}
			else
			{


				w2=(pB-pA - B+A);
				a=(w2||w2);
				b=2*(w1||w2);
				c=(w1||w1)-Array[n1]->Radius*Array[n1]->Radius;
				ResoudAttBtC(a,b,c,&t1,&t2,&D);

				if (D>=0)
				{

					if ((t1>-SMALLF)&&(t1<1+SMALLF))
					{
						I=A+t1*(B-A);

						OI=I-Array[n1]->Pos;

						if (f_abs(OI||u1)<Array[n1]->L/2)
						{

							u=-1*OI;
							u.Normalise();

							if (Calculate)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();
								cs->Distance=0;
								cs->Normals=u;
								cs->Point=I;
								cs->Types=COLLISION_WITH_OBJECT;
								cs->Num=n2;
								
							}
							res=1;
						}
					}

					if ((t2>-SMALLF)&&(t2<1+SMALLF))
					{
						I=A+t2*(B-A);

						OI=I-Array[n1]->Pos;

						if (f_abs(OI||u1)<Array[n1]->L/2)
						{

							u=-1*OI;
							u.Normalise();

							if (Calculate)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();
								cs->Distance=0;
								cs->Normals=u;
								cs->Point=I;
								cs->Types=COLLISION_WITH_OBJECT;
								cs->Num=n2;
								
							}
							res=1;
						}
					}
				}
			}
			/**/
			// faces dessus dessous

			VECTOR3MATRIX(u,Array[n1]->U,M1);

			VECTORNORMALISE(u);

			pA=Array[n1]->Pos + Array[n1]->L*u/2;
			pB=Array[n1]->Pos - Array[n1]->L*u/2;


			I=pA;

			d=-DOT(u,I);

			s1=DOT(u,A)+d;
			s2=DOT(u,B)+d;
			
			if (s1*s2<0)
			{
				t=-s1/(s2-s1);
				if ((t>-SMALLF)&&(t<1+SMALLF))
				{

					p=A+t*(B-A);

					VECTORSUB(OI,p,I);

					if (VECTORNORM(OI)<Array[n2]->Radius)
					{
						if (Calculate)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							cs->Normals=u;
							cs->Point=p;
							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
							
						}
						res=1;
					}
				}
			}


			u=-1*u;
			I=pB;


			d=-DOT(u,I);

			s1=DOT(u,A)+d;
			s2=DOT(u,B)+d;
			
			if (s1*s2<0)
			{
				t=-s1/(s2-s1);
				if ((t>-SMALLF)&&(t<1+SMALLF))
				{

					p=A+t*(B-A);

					VECTORSUB(OI,p,I);

					if (VECTORNORM(OI)<Array[n2]->Radius)
					{
						if (Calculate)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							cs->Normals=u;
							cs->Point=p;
							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
							
						}
						res=1;
					}
				}
			}

			n++;

		}


		/**/


	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingCylinder2(int n1,int n2,CPOB *pob,int Calculate,int buf)
{
	int res;
	int np;	

	np=Contacts[buf].Length();
	res=IsCylinder1CollidingPobClosing2(n2,n1,pob,Calculate,buf);

	if (Calculate)
	{
		ContactStruct * cs=Contacts[buf].SetCurrent(np);
		while (cs)
		{
			cs->Normals.x=-cs->Normals.x;
			cs->Normals.y=-cs->Normals.y;
			cs->Normals.z=-cs->Normals.z;
			cs=Contacts[buf].GetNext();
		}
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPob1CollidingCylinder2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int res=0;

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingCylinder2(n1,n2,pob,0,buf)==1)
			res=IsPob1CollidingCylinder2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingCylinder2(n1,n2,pob,Calculate,buf);
		res|=IsPob1CollidingCylinder2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingCylinder2(n1,n2,pob,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingCylinder2(n1,n2,pob,0,buf)==1)
		{
			res=IsPob1CollidingCylinder2(n1,n2,pob->Next1,Calculate,buf);
			res|=IsPob1CollidingCylinder2(n1,n2,pob->Next2,Calculate,buf);
		}
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinder1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int res=0;

	if (pob->ID==POB_MULTINODE)
	{
		if (IsCylinder1CollidingPobClosing2(n2,n1,pob,0,buf)==1)
			res=IsCylinder1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsCylinder1CollidingPobClosing2(n1,n2,pob,Calculate,buf);
		res|=IsCylinder1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsCylinder1CollidingPobClosing2(n2,n1,pob,Calculate,buf);		
	}
	else
	{
		if (IsCylinder1CollidingPobClosing2(n2,n1,pob,0,buf)==1)
		{
			res=IsCylinder1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
			res|=IsCylinder1CollidingPob2(n1,n2,pob->Next2,Calculate,buf);
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinder1CollidingCylinder2(int n1,int n2,int Calculate,int buf)
{
	CVector u1,u2,v1,v2,w1,w2,u,v,n,N,p;
	float a,b,c,d;
	float t1,t2,D,t,s1,s2;
	CVector A,B,pA,pB,I,OI,pC,PN;
	int res=0;
	CMatrix Mo1=Array[n1]->MOActual;
    CMatrix Mo2=Array[n2]->MOActual;

	u1=Array[n1]->U*Mo1;
	u2=Array[n2]->U*Mo2;

	u1.Normalise();
	u2.Normalise();


	if (f_abs(u1||u2)>0.9f)
	{

		VECTORSUB(N,Array[n1]->Pos,Array[n2]->Pos);
		d=VECTORNORM(N);
		VECTORDIV(N,N,d);
		
		A=Array[n2]->Pos + Array[n2]->L*u2/2;
		B=Array[n2]->Pos - Array[n2]->L*u2/2;

		pA=ProjectionPointSurAxe(A,Array[n1]->Pos,u1);
		pB=ProjectionPointSurAxe(B,Array[n1]->Pos,u1);

		PN=(A+B)/2;
		pC=ProjectionPointSurAxe(PN,Array[n1]->Pos,u1);

		u=pA-Array[n1]->Pos;

		if (VECTORNORM(u)<Array[n1]->L/2) 
		{
			if (d<Array[n1]->Radius+Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Distance=0;
					cs->Normals=N;
					cs->Point=Array[n2]->Pos+Array[n2]->Radius*N;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;

			}


		}


		VECTORSUB(u,pC,Array[n1]->Pos);

		if (VECTORNORM(u)<Array[n1]->L/2) 
		{
			if (d<Array[n1]->Radius+Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Distance=0;
					cs->Normals=N;
					cs->Point=Array[n2]->Pos+Array[n2]->Radius*N;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;

			}
		}


		VECTORSUB(u,pB,Array[n1]->Pos);
		
		if (VECTORNORM(u)<Array[n1]->L/2) 
		{
			if (d<Array[n1]->Radius+Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Distance=0;
					cs->Normals=N;
					cs->Point=Array[n2]->Pos+Array[n2]->Radius*N;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;


			}
		}
	}
	else
	{

		VECTORSUB(N,Array[n2]->Pos,Array[n1]->Pos);
		VECTORNORMALISE(N);

		CROSSPRODUCT(n,u2,N);
		CROSSPRODUCT(v,n,u2);
		VECTORNORMALISE(v);
		if (f_abs(u2||N)>1-SMALLF) v.Init(0,0,0);

		A=Array[n2]->Pos + Array[n2]->L*u2/2  - Array[n2]->Radius*v;
		B=Array[n2]->Pos - Array[n2]->L*u2/2  - Array[n2]->Radius*v;

		pA=ProjectionPointSurAxe(A,Array[n1]->Pos,u1);
		pB=ProjectionPointSurAxe(B,Array[n1]->Pos,u1);


		w1=pA-A;
		w2=pB-B;

		if ((w1.Norme()<Array[n1]->Radius)&&(w2.Norme()<Array[n1]->Radius))
		{
			if (Calculate)
			{
				I=(A+B)/2;

				ContactStruct * cs=Contacts[buf].InsertLast();
				cs->Distance=0;
				cs->Normals.x=-N.x;
				cs->Normals.y=-N.y;
				cs->Normals.z=-N.z;
				cs->Point=I;
				cs->Types=COLLISION_WITH_OBJECT;
				cs->Num=n2;
				
			}
			res=1;


		}
		else
		{


		w2=(pB-pA - B+A);
		a=DOT(w2,w2);
		b=2*DOT(w1,w2);

		c=DOT(w1,w1)-Array[n1]->Radius*Array[n1]->Radius;

		ResoudAttBtC(a,b,c,&t1,&t2,&D);

		if (D>=0)
		{

			if ((t1>-SMALLF)&&(t1<1+SMALLF))
			{
				I=A+t1*(B-A);

				VECTORSUB(OI,I,Array[n1]->Pos);

				if (f_abs(DOT(OI,u1))<Array[n1]->L/2)
				{

					u=-1*OI;
					VECTORNORMALISE(u);

					if (Calculate)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Distance=0;
						cs->Normals=u;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
					}
					res=1;
				}
			}



			if ((t2>-SMALLF)&&(t2<1+SMALLF))
			{
				I=A+t2*(B-A);

				OI=I-Array[n1]->Pos;

				if (f_abs(DOT(OI,u1))<Array[n1]->L/2)
				{

					u=-1*OI;
					VECTORNORMALISE(u);

					if (Calculate)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Distance=0;
						cs->Normals=u;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
					}
					res=1;
				}
			}

		}

	}

/*
		N=Array[n1]->Pos - Array[n2]->Pos;
		N.Normalise();

		n=u1^N;
		v=n^u1;
		v.Normalise();
		if (f_abs(u1||N)>1-SMALLF) v.Init(0,0,0);

		A=Array[n1]->Pos + Array[n1]->L*u2/2  - Array[n1]->Radius*v;
		B=Array[n1]->Pos - Array[n1]->L*u2/2  - Array[n1]->Radius*v;

		pA=ProjectionPointSurAxe(A,Array[n2]->Pos,u2);
		pB=ProjectionPointSurAxe(B,Array[n2]->Pos,u2);


		w1=pA-A;
		w2=pB-B;

		if ((w1.Norme()<Array[n2]->Radius)&&(w2.Norme()<Array[n2]->Radius))
		{

			if (Calculate)
			{

				I=(A+B)/2;
				Distance=0;
				Normals=N;
				Point=I;
				Types=COLLISION_WITH_OBJECT;
				Num=n2;
				
			}
			res=1;


		}
		else
		{


		w2=(pB-pA - B+A);
		a=(w2||w2);
		
		b=2*(w1||w2);

		c=(w1||w1)-Array[n2]->Radius*Array[n2]->Radius;

		ResoudAttBtC(a,b,c,&t1,&t2,&D);

		if (D>=0)
		{

			if ((t1>-SMALLF)&&(t1<1+SMALLF))
			{
				I=A+t1*(B-A);

				OI=I-Array[n2]->Pos;

				if (f_abs(OI||u2)<Array[n2]->L/2)
				{

					u=OI;
					u.Normalise();

					if (Calculate)
					{

						Distance=0;
						Normals=u;
						Point=I;
						Types=COLLISION_WITH_OBJECT;
						Num=n2;
						
					}
					res=1;
				}
			}



			if ((t2>-SMALLF)&&(t2<1+SMALLF))
			{
				I=A+t2*(B-A);

				OI=I-Array[n2]->Pos;

				if (f_abs(OI||u2)<Array[n2]->L/2)
				{

					u=OI;
					u.Normalise();

					if (Calculate)
					{
						Distance=0;
						Normals=u;
						Point=I;
						Types=COLLISION_WITH_OBJECT;
						Num=n2;
						
					}
					res=1;
				}
			}

		}
		}
		*/
	}



	// arrete objet 1


	N=Array[n1]->Pos - Array[n2]->Pos;
	N.Normalise();

	n=u1^N;
	v=n^u1;
	v.Normalise();


	pA=Array[n1]->Pos + Array[n1]->L*u2/2  - Array[n1]->Radius*v;
	pB=Array[n1]->Pos - Array[n1]->L*u2/2  - Array[n1]->Radius*v;



	// test par rapport aux faces de dessus et dessous de l'objet2






	u=Array[n2]->U*Mo2;

	VECTORNORMALISE(u);

	A=Array[n2]->Pos + Array[n2]->L*u/2;
	B=Array[n2]->Pos - Array[n2]->L*u/2;


	I=A;

	d=-DOT(u,I);

	s1=DOT(u,pA)+d;
	s2=DOT(u,pB)+d;
	
	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		if ((t>-SMALLF)&&(t<1+SMALLF))
		{

			p=pA+t*(pB-pA);

			VECTORSUB(OI,p,I);

			if (VECTORNORM(OI)<Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Normals=u;
					cs->Point=p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;
			}
		}
	}


	u=-1*u;
	I=B;


	d=-DOT(u,I);

	s1=DOT(u,pA)+d;
	s2=DOT(u,pB)+d;
	
	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		if ((t>-SMALLF)&&(t<1+SMALLF))
		{

			p=pA+t*(pB-pA);

			VECTORSUB(OI,p,I);

			if (VECTORNORM(OI)<Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Normals=u;
					cs->Point=p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;
			}
		}
	}

	// arrete objet 1


	VECTORSUB(N,Array[n2]->Pos,Array[n1]->Pos);
	VECTORNORMALISE(N);

	CROSSPRODUCT(n,u2,N);
	CROSSPRODUCT(v,n,u2);
	VECTORNORMALISE(v);
	//if (f_abs(u2||N)>1-SMALLF) v.Init(0,0,0);

	pA=Array[n2]->Pos + Array[n2]->L*u2/2  - Array[n2]->Radius*v;
	pB=Array[n2]->Pos - Array[n2]->L*u2/2  - Array[n2]->Radius*v;



	// test par rapport aux faces de dessus et dessous de l'objet2






	u=Array[n1]->U*Mo1;

	VECTORNORMALISE(u);

	A=Array[n1]->Pos + Array[n1]->L*u/2;
	B=Array[n1]->Pos - Array[n1]->L*u/2;


	I=A;

	d=-DOT(u,I);

	s1=DOT(u,pA)+d;
	s2=DOT(u,pB)+d;
	
	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		if ((t>-SMALLF)&&(t<1+SMALLF))
		{

			p=pA+t*(pB-pA);

			VECTORSUB(OI,p,I);

			if (VECTORNORM(OI)<Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Normals.x=-u.x;
					cs->Normals.y=-u.y;
					cs->Normals.z=-u.z;
					cs->Point=p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;
			}
		}
	}


	u=-1*u;
	I=B;


	d=-DOT(u,I);

	s1=DOT(u,pA)+d;
	s2=DOT(u,pB)+d;
	
	if (s1*s2<0)
	{
		t=-s1/(s2-s1);
		if ((t>-SMALLF)&&(t<1+SMALLF))
		{

			p=pA+t*(pB-pA);

			VECTORSUB(OI,p,I);

			if (VECTORNORM(OI)<Array[n2]->Radius)
			{
				if (Calculate)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();
					cs->Normals.x=-u.x;
					cs->Normals.y=-u.y;
					cs->Normals.z=-u.z;
					cs->Point=p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
				}
				res=1;
			}
		}
	}


	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingCube2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};


	float dmax;
	
	CVector Opob;
	
	int res3;
	CVector Cube1[8+6];
	CVector Cube2[8+6];
	CVector Norm[6];
	CVector U1U2[6][2];

	CVector a1,a2,a1a2,a1I;
	float L1L2[6][2];
	float s1,s2;
	CVector ux,uy,uz,OM;
	float ss[8+6];
	float t;
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I,Pos1,Pos2,axe;
	CMatrix M1,M2,MM1,MM2;
	float rA,rB,DL,rA1,rA2,rA3,rB1,rB2,rB3;
	int n,nn,nn1,nn2,res,res2;
	
	res=0;
	{

		M1=Array[n1]->MOActual;
		M2=Array[n2]->MOActual;

		MM1=Array[n1]->MActual;
		Opob=pob->O*MM1;

		Pos2=Array[n2]->Pos;
		Pos1=Opob;

		D=Pos2-Pos1;
		
		A1=(COEF_ENVELOPPE+pob->Lx)*(pob->Ux*M1);
		A2=(COEF_ENVELOPPE+pob->Ly)*(pob->Uy*M1);
		A3=(COEF_ENVELOPPE+pob->Lz)*(pob->Uz*M1);
		

		B1=(COEF_ENVELOPPE+Array[n2]->Lx)*(Array[n2]->Ux*M2);
		B2=(COEF_ENVELOPPE+Array[n2]->Ly)*(Array[n2]->Uy*M2);
		B3=(COEF_ENVELOPPE+Array[n2]->Lz)*(Array[n2]->Uz*M2);
		
		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;

		Ls[3]=B1;
		Ls[4]=B2;
		Ls[5]=B3;

		for (n=0;n<6;n++) VECTORNORMALISE(Ls[n]);


		dmax=VECTORNORM(D);

		n=0;
		res=1;
		while ((res==1)&&(n<6))
		{
			L=Ls[n];
			rA1=DOT(A1,L);
			rA2=DOT(A2,L);
			rA3=DOT(A3,L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

			rB1=DOT(B1,L);
			rB2=DOT(B2,L);
			rB3=DOT(B3,L);
			rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

			DL=DOT(D,L);
			DL=f_abs(DL);

			s1=rA+rB - DL;

			if (s1<0) res=0; 
			else 
			{
				n++;
				if (dmax>s1) 
				{
					dmax=s1;
					if (n<3)
						axe=L;
					else
						axe=-1*L;

				}

			}
		}
		
		if ((res==1)&&(pob->ID==POB_CLOSING)) { DMAX=dmax;Axe=axe;}
		if ((res==1)&&(pob->ID==POB_MULTINODECLOSING)) { DMAX=dmax;Axe=axe;}
		
		if ((res==1)&&(Calculate==1))	// collision
		{
			res=0;			
			MM2=Array[n2]->MActual;

			for (n=0;n<8+6;n++)
			{
				p=pob->O + Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
				VECTOR3MATRIX(Cube1[n],p,MM1);
			}

			for (n=0;n<8+6;n++)
			{
				p=Cube[n][0]*(COEF_ENVELOPPE+Array[n2]->Lx)*Array[n2]->Ux + Cube[n][1]*(COEF_ENVELOPPE+Array[n2]->Ly)*Array[n2]->Uy + Cube[n][2]*(COEF_ENVELOPPE+Array[n2]->Lz)*Array[n2]->Uz;
				VECTOR3MATRIX(Cube2[n],p,MM2);
			}

			VECTOR3MATRIX(ux,Array[n2]->Ux,M2);
			VECTOR3MATRIX(uy,Array[n2]->Uy,M2);
			VECTOR3MATRIX(uz,Array[n2]->Uz,M2);
			
			Norm[0]=uz;
				U1U2[0][0]=ux;
				U1U2[0][1]=uy;
				L1L2[0][0]=Array[n2]->Lx;
				L1L2[0][1]=Array[n2]->Ly;
			Norm[1]=-1*uy;
				U1U2[1][0]=ux;
				U1U2[1][1]=uz;
				L1L2[1][0]=Array[n2]->Lx;
				L1L2[1][1]=Array[n2]->Lz;
			
			Norm[2]=-1*ux;
				U1U2[2][0]=uy;
				U1U2[2][1]=uz;
				L1L2[2][0]=Array[n2]->Ly;
				L1L2[2][1]=Array[n2]->Lz;

			
			Norm[3]=uy;
				U1U2[3][0]=ux;
				U1U2[3][1]=uz;
				L1L2[3][0]=Array[n2]->Lx;
				L1L2[3][1]=Array[n2]->Lz;

			
			Norm[4]=ux;
				U1U2[4][0]=uz;
				U1U2[4][1]=uy;
				L1L2[4][0]=Array[n2]->Lz;
				L1L2[4][1]=Array[n2]->Ly;


			Norm[5]=-1*uz;
				U1U2[5][0]=ux;
				U1U2[5][1]=uy;
				L1L2[5][0]=Array[n2]->Lx;
				L1L2[5][1]=Array[n2]->Ly;

			n=0;
			res2=0;
			while ((n<6))//&&(res2==0))
			{
				VECTORNORMALISE(Norm[n]);

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube2[Faces[n][0]].x+b*Cube2[Faces[n][0]].y+c*Cube2[Faces[n][0]].z);

				for (nn=0;nn<8+6;nn++)
				{
					ss[nn]=a*Cube1[nn].x+b*Cube1[nn].y+c*Cube1[nn].z+d;
				}
			
				res3=0;
				nn=0;
				while ((nn<12+3))
				{
					nn1=Arretes[nn][0];
					nn2=Arretes[nn][1];

					if (ss[nn1]*ss[nn2]<0)
					{
						t=-ss[nn1]/(ss[nn2]-ss[nn1]);
						p=Cube1[nn1]+t*(Cube1[nn2]-Cube1[nn1]);

						VECTORSUB(OM,p,Pos2);

						s1=DOT(OM,U1U2[n][0]);
						s2=DOT(OM,U1U2[n][1]);

						if ((f_abs(s1)<L1L2[n][0])&&(f_abs(s2)<L1L2[n][1]))
						{

							ContactStruct * cs=Contacts[buf].InsertLast();

							if (res3==0) cs->Distance=1;
							else cs->Distance=-1;
							
							cs->Normals=Norm[n];
							cs->Point=p;

							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
						
							res2=1;
							res3=1;
							res=1;
						}
					}
					nn++;
				}	
				n++;

			}
			// inverse

			VECTOR3MATRIX(ux,pob->Ux,M1);
			VECTOR3MATRIX(uy,pob->Uy,M1);
			VECTOR3MATRIX(uz,pob->Uz,M1);
			
			Norm[0]=uz;
				U1U2[0][0]=ux;
				U1U2[0][1]=uy;
				L1L2[0][0]=pob->Lx;
				L1L2[0][1]=pob->Ly;
			Norm[1]=-1*uy;
				U1U2[1][0]=ux;
				U1U2[1][1]=uz;
				L1L2[1][0]=pob->Lx;
				L1L2[1][1]=pob->Lz;
			
			Norm[2]=-1*ux;
				U1U2[2][0]=uy;
				U1U2[2][1]=uz;
				L1L2[2][0]=pob->Ly;
				L1L2[2][1]=pob->Lz;

			
			Norm[3]=uy;
				U1U2[3][0]=ux;
				U1U2[3][1]=uz;
				L1L2[3][0]=pob->Lx;
				L1L2[3][1]=pob->Lz;

			
			Norm[4]=ux;
				U1U2[4][0]=uz;
				U1U2[4][1]=uy;
				L1L2[4][0]=pob->Lz;
				L1L2[4][1]=pob->Ly;


			Norm[5]=-1*uz;
				U1U2[5][0]=ux;
				U1U2[5][1]=uy;
				L1L2[5][0]=pob->Lx;
				L1L2[5][1]=pob->Ly;



			n=0;
			res2=0;
			while ((n<6))//&&(res2==0))
			{
				VECTORNORMALISE(Norm[n]);

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube1[Faces[n][0]].x+b*Cube1[Faces[n][0]].y+c*Cube1[Faces[n][0]].z);

				for (nn=0;nn<8+6;nn++)
				{
					ss[nn]=a*Cube2[nn].x+b*Cube2[nn].y+c*Cube2[nn].z+d;
				}

				res3=0;

				nn=0;
				while (nn<12+3)
				{
				
					nn1=Arretes[nn][0];
					nn2=Arretes[nn][1];

					if (ss[nn1]*ss[nn2]<0)
					{
						t=-ss[nn1]/(ss[nn2]-ss[nn1]);
						p=Cube2[nn1]+t*(Cube2[nn2]-Cube2[nn1]);

						VECTORSUB(OM,p,Pos1);

						s1=DOT(OM,U1U2[n][0]);
						s2=DOT(OM,U1U2[n][1]);

						if ((f_abs(s1)<L1L2[n][0])&&(f_abs(s2)<L1L2[n][1]))
						{
							ContactStruct * cs=Contacts[buf].InsertLast();

							if (res3==0) cs->Distance=1;
							else cs->Distance=-1;
							cs->Normals=-1*Norm[n];
							cs->Point=p;
							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
						
							res2=1;
							res3=1;
							res=1;
						}
					}
					nn++;
				}
				n++;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingSphere2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	float t1,t2;
	float x1,y1,z1,x2,y2,z2,xs,ys,zs;

	int toto;

	CVector u;
	CVector Cube1[8+6];
//	CVector Cube2[8+6];
	CVector Norm[6];
	

	float ss;
	CVector a1,a2,a1a2,a1I,Pos1,Pos2;
	
	float s1;
	CVector ux,uy,uz,OM;
	
	float r;
	float dmax;	
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I,Opob,axe;
	CMatrix M1,M2,MM1,MM2;
	float rA,DL,rA1,rA2,rA3;
	int n,nn,nn1,nn2,res,res2;
	res=0;

	MM1=Array[n1]->MActual;
	VECTOR3MATRIX(Opob,pob->O,MM1);

	Pos2=Array[n2]->Pos;

	Pos1=Opob;

	D=Pos2 - Pos1;
	r=Array[n1]->Radius+Array[n2]->Radius;
		
	M1=Array[n1]->MOActual;

	A1=(COEF_ENVELOPPE+pob->Lx)*pob->Ux;
	A1=A1*M1;
	A2=(COEF_ENVELOPPE+pob->Ly)*pob->Uy;
	A2=A2*M1;
	A3=(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
	A3=A3*M1;
	
	Ls[0]=A1;
	Ls[1]=A2;
	Ls[2]=A3;

	Ls[3]=A1+A2;
	Ls[4]=A1+A3;
	Ls[5]=A2+A3;

	Ls[6]=A1+A2+A3;

	for (n=0;n<7;n++) VECTORNORMALISE(Ls[n]);
	dmax=VECTORNORM(D);
	n=0;
	res2=1;
	while (((res2==1)||(Calculate==0))&&(n<7))
	{
		L=Ls[n];

		rA1=DOT(A1,L);
		rA2=DOT(A2,L);
		rA3=DOT(A3,L);
		rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

		DL=DOT(D,L);
		DL=f_abs(DL);

		ss=(COEF_ENVELOPPE+Array[n2]->Radius)+rA - DL;

		if (ss<0) { res2=0;n++;}
		else 
		{
			n++;

			if (dmax>ss) 
			{
				dmax=ss;
				axe=L;
			}
		}
	}

	if ((res2==1)&&(pob->ID==POB_CLOSING)) { DMAX=dmax;Axe=axe;}
	if ((res2==1)&&(pob->ID==POB_MULTINODECLOSING)) {DMAX=dmax;Axe=axe;}

	if ((res2==1)&&(Calculate==0)) res=1;

	if ((res2==1)&&(Calculate==1))
	{
		for (n=0;n<8+6;n++)
		{
			p=pob->O+Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
			VECTOR3MATRIX(Cube1[n],p,MM1);
		}

		r=(COEF_ENVELOPPE+Array[n2]->Radius);
		xs=Array[n2]->Pos.x;
		ys=Array[n2]->Pos.y;
		zs=Array[n2]->Pos.z;
//------------------------------------------------
		VECTOR3MATRIX(ux,pob->Ux,M1);
		VECTOR3MATRIX(uy,pob->Uy,M1);
		VECTOR3MATRIX(uz,pob->Uz,M1);

		
		Norm[0]=uz;
		Norm[1]=-1*uy;
		Norm[2]=-1*ux;
		Norm[3]=uy;
		Norm[4]=ux;
		Norm[5]=-1*uz;

		toto=0;

		for (n=0;n<6;n++)
		{
			VECTORNORMALISE(Norm[n]);

			a=Norm[n].x;
			b=Norm[n].y;
			c=Norm[n].z;
			d=-(a*Cube1[Faces[n][0]].x+b*Cube1[Faces[n][0]].y+c*Cube1[Faces[n][0]].z);

			s1=a*xs+b*ys+c*zs+d;

			if ((s1>0)&&(s1<r))	// collision possible avec face
			{

				ss=s1;
				p=-1*Norm[n];

				I=p*Array[n2]->Radius;
				I+=Array[n2]->Pos;

				res=1;

				nn=0;
				while ((nn<4)&&(res==1))
				{
					nn1=Faces[n][nn];
					nn2=Faces[n][(nn+1)%4];

					a1=Cube1[nn1];
					a2=Cube1[nn2];
					a1a2=a2-a1;

					a1I=I-a1;

					s1=((a1a2^a1I)||Norm[n]);

					if (s1>0)
					{
						res=0;
					}
					nn++;
				}

				if (res==1)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();

					cs->Normals=p;
					cs->Point=Array[n2]->Pos+ss*p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
					toto=1;
				}
			}
		}
//------------------------------------------------	

		if (toto==0)
		{
			n=0;
			while (n<12+3)
			{

				nn1=Arretes[n][0];
				nn2=Arretes[n][1];
			
				x1=Cube1[nn1].x;
				y1=Cube1[nn1].y;
				z1=Cube1[nn1].z;

				x2=Cube1[nn2].x;
				y2=Cube1[nn2].y;
				z2=Cube1[nn2].z;

				a=((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
				b=(x1-xs)*(x2-x1) + (y1-ys)*(y2-y1) + (z1-zs)*(z2-z1);
				c=(x1-xs)*(x1-xs) + (y1-ys)*(y1-ys) + (z1-zs)*(z1-zs) - r*r;
				
				d=b*b - a*c;

				if (d>=0)
				{
				
					t1=(-b +(float) sqrtf(d))/a;
					t2=(-b -(float) sqrtf(d))/a;


					if ((t1>=0)&&(t1<=1))
					{
						I.x=x1+t1*(x2-x1);
						I.y=y1+t1*(y2-y1);
						I.z=z1+t1*(z2-z1);
						p=I-Array[n2]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;
						toto=1;

					}
				
					if ((t2>=0)&&(t2<=1))
					{
						I.x=x1+t2*(x2-x1);
						I.y=y1+t2*(y2-y1);
						I.z=z1+t2*(z2-z1);
						p=I-Array[n2]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();
						
						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;
						toto=1;

					}
				}

				n++;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingBall2(int n1,int n2,CPOB * pob,CVector &P,float R,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	CVector u;
	float t1,t2;
	float x1,y1,z1,x2,y2,z2,xs,ys,zs;

	int toto;
	CVector Cube1[8+6];
	CVector Norm[6];

	float ss;
	CVector a1,a2,a1a2,a1I,Pos1,Pos2;
	
	float s1;
	CVector ux,uy,uz,OM;
	
	float r;
	float dmax;	
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I,Opob,axe;
	float rA,DL,rA1,rA2,rA3;
	int n,nn,nn1,nn2,res,res2;
	res=0;

	VECTOR3MATRIX(Opob,pob->O,Array[n1]->MActual);

	Pos2=P;

	Pos1=Opob;

	VECTORSUB(D,Pos2,Pos1);
	r=Array[n1]->Radius+R;

	A1=(COEF_ENVELOPPE+pob->Lx)*pob->Ux;
	VECTOR3MATRIX(A1,A1,Array[n1]->MOActual);
	A2=(COEF_ENVELOPPE+pob->Ly)*pob->Uy;
	VECTOR3MATRIX(A2,A2,Array[n1]->MOActual);
	A3=(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
	VECTOR3MATRIX(A3,A3,Array[n1]->MOActual);
	
	Ls[0]=A1;
	Ls[1]=A2;
	Ls[2]=A3;

	Ls[3]=A1+A2;
	Ls[4]=A1+A3;
	Ls[5]=A2+A3;

	Ls[6]=A1+A2+A3;

	for (n=0;n<7;n++) VECTORNORMALISE(Ls[n]);
	dmax=VECTORNORM(D);
	n=0;
	res2=1;
	while (((res2==1)||(Calculate==0))&&(n<7))
	{
		L=Ls[n];

		rA1=DOT(A1,L);
		rA2=DOT(A2,L);
		rA3=DOT(A3,L);
		rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

		DL=DOT(D,L);
		DL=f_abs(DL);

		ss=(R)+rA - DL;

		if (ss<0) { res2=0;n++;}
		else 
		{
			n++;

			if (dmax>ss) 
			{
				dmax=ss;
				axe=L;
			}
		}
	}

	if ((res2==1)&&(pob->ID==POB_CLOSING)) { DMAX=dmax;Axe=axe;}
	if ((res2==1)&&(pob->ID==POB_MULTINODECLOSING)) {DMAX=dmax;Axe=axe;}

	if ((res2==1)&&(Calculate==0)) res=1;

	if ((res2==1)&&(Calculate==1))
	{
		for (n=0;n<8+6;n++)
		{
			p=pob->O+Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
			Cube1[n]=p*Array[n1]->MActual;
		}

		r=(R);
		xs=P.x;
		ys=P.y;
		zs=P.z;
//------------------------------------------------
		VECTOR3MATRIX(ux,pob->Ux,Array[n1]->MOActual);
		VECTOR3MATRIX(uy,pob->Uy,Array[n1]->MOActual);
		VECTOR3MATRIX(uz,pob->Uz,Array[n1]->MOActual);

		
		Norm[0]=uz;
		Norm[1]=-1*uy;
		Norm[2]=-1*ux;
		Norm[3]=uy;
		Norm[4]=ux;
		Norm[5]=-1*uz;

		toto=0;

		for (n=0;n<6;n++)
		{
			VECTORNORMALISE(Norm[n]);

			a=Norm[n].x;
			b=Norm[n].y;
			c=Norm[n].z;
			d=-(a*Cube1[Faces[n][0]].x+b*Cube1[Faces[n][0]].y+c*Cube1[Faces[n][0]].z);

			s1=a*xs+b*ys+c*zs+d;

			if ((s1>0)&&(s1<r))	// collision possible avec face
			{

				ss=s1;
				p=-1*Norm[n];

				I=p*R;
				I+=P;

				res=1;

				nn=0;
				while ((nn<4)&&(res==1))
				{
					nn1=Faces[n][nn];
					nn2=Faces[n][(nn+1)%4];

					a1=Cube1[nn1];
					a2=Cube1[nn2];
					a1a2=a2-a1;

					a1I=I-a1;

					s1=((a1a2^a1I)||Norm[n]);

					if (s1>0)
					{
						res=0;
					}
					nn++;
				}


				if (res==1)
				{
					ContactStruct * cs=Contacts[buf].InsertLast();

					cs->Normals=p;
					cs->Point=P+ss*p;
					cs->Types=COLLISION_WITH_OBJECT;
					cs->Num=n2;
					
					toto=1;
				}


			}
		}


//------------------------------------------------
	

		if (toto==0)
		{
			n=0;
			while (n<12+3)
			{

				nn1=Arretes[n][0];
				nn2=Arretes[n][1];


			
				x1=Cube1[nn1].x;
				y1=Cube1[nn1].y;
				z1=Cube1[nn1].z;

				x2=Cube1[nn2].x;
				y2=Cube1[nn2].y;
				z2=Cube1[nn2].z;

				a=((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
				b=(x1-xs)*(x2-x1) + (y1-ys)*(y2-y1) + (z1-zs)*(z2-z1);
				c=(x1-xs)*(x1-xs) + (y1-ys)*(y1-ys) + (z1-zs)*(z1-zs) - r*r;


				
				d=b*b - a*c;

				if (d>=0)
				{
				
					t1=(-b +(float) sqrtf(d))/a;
					t2=(-b -(float) sqrtf(d))/a;


					if ((t1>=0)&&(t1<=1))
					{
						I.x=x1+t1*(x2-x1);
						I.y=y1+t1*(y2-y1);
						I.z=z1+t1*(z2-z1);
						p=I-P;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;
						toto=1;

					}
				
					if ((t2>=0)&&(t2<=1))
					{
						I.x=x1+t2*(x2-x1);
						I.y=y1+t2*(y2-y1);
						I.z=z1+t2*(z2-z1);
						p=I-P;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;
						toto=1;

					}


				}
				n++;
			}
		}
	}

	return res;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingPobClosing2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf)
{
//	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,1,-1},{-1,1,-1}, {-1,-1,1},{1,-1,1},{1,-1,-1},{-1,-1,-1}};
	const int Arretes[12+6*2][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{4,0},{5,1},{6,2},{7,3},
									{0,2},{2,5},{6,4},{7,0},{2,7},{5,0},
									{1,3},{1,6},{5,7},{4,3},{6,3},{1,4},
	};
	const int Faces[6][4]={{0,1,2,3},{2,1,5,6},{6,5,4,7},{7,4,0,3},{2,6,7,3},{5,1,0,4}};
	CVector Cube1[8];
	CVector Cube2[8];
	CVector Norm[6];
	CVector Norm2[6];
	CVector ux,uy,uz;
	float ss[6][8];
	float t;
	float a,b,c,d;
	CVector A1,A2,A3,A4,p,I,A,B,N,P;
	int n,nn,res;
	CVector a1,a2,a3,a4;
	CVector b1,b2,b3,b4;
	CVector c1,c2,c3,c4;
	float s1,s2,s3,s4;
	int aa,bb;
	int nf,cc;
	CVector X1,X2,X3;

	res=0;
	DMAX=0.0f;

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,-(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[0],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[1],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,-(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[2],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,-(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,-(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[3],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,-(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,-(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[4],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,-(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[5],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,-(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,-(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[6],p,Array[n1]->MActual);

	p=pob1->O;
	VECTORMUL(X1,pob1->Ux,-(COEF_ENVELOPPE+pob1->Lx));
	VECTORMUL(X2,pob1->Uy,-(COEF_ENVELOPPE+pob1->Ly));
	VECTORMUL(X3,pob1->Uz,-(COEF_ENVELOPPE+pob1->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube1[7],p,Array[n1]->MActual);


	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,-(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[0],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[1],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,-(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[2],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,-(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,-(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[3],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,-(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,-(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[4],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,-(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[5],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,-(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,-(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[6],p,Array[n2]->MActual);

	p=pob2->O;
	VECTORMUL(X1,pob2->Ux,-(COEF_ENVELOPPE+pob2->Lx));
	VECTORMUL(X2,pob2->Uy,-(COEF_ENVELOPPE+pob2->Ly));
	VECTORMUL(X3,pob2->Uz,-(COEF_ENVELOPPE+pob2->Lz));
	VECTORADD(p,p,X1);
	VECTORADD(p,p,X2);
	VECTORADD(p,p,X3);
	VECTOR3MATRIX(Cube2[7],p,Array[n2]->MActual);


	VECTOR3MATRIX(ux,pob1->Ux,Array[n1]->MOActual); VECTOR3MATRIX(uy,pob1->Uy,Array[n1]->MOActual); VECTOR3MATRIX(uz,pob1->Uz,Array[n1]->MOActual);
	Norm[0]=uy; Norm[1]=ux; VECTORNEGATE(Norm[2],uy); VECTORNEGATE(Norm[3],ux); VECTORNEGATE(Norm[4],uz); Norm[5]=uz;

	VECTOR3MATRIX(ux,pob2->Ux,Array[n2]->MOActual); VECTOR3MATRIX(uy,pob2->Uy,Array[n2]->MOActual); VECTOR3MATRIX(uz,pob2->Uz,Array[n2]->MOActual);
	Norm2[0]=uy; Norm2[1]=ux; VECTORNEGATE(Norm2[2],uy); VECTORNEGATE(Norm2[3],ux); VECTORNEGATE(Norm2[4],uz); Norm2[5]=uz;

	for (nf=0;nf<6;nf++)
	{
		N=Norm2[nf];

		nn=Faces[nf][3];
		P=Cube2[nn];

		a=N.x;
		b=N.y;
		c=N.z;
		d=-(a*P.x+b*P.y+c*P.z);

		cc=0;
		for (n=0;n<8;n++)
		{
			ss[nf][n]=a*Cube1[n].x+b*Cube1[n].y+c*Cube1[n].z+d;
			cc+=(ss[nf][n]>0);
		}

		if (cc==8)
		{
			res=-1;
			break;
		}
	}

	if (res==0)
	for (nf=0;nf<6;nf++)
	{
		N=Norm2[nf];
		
		nn=Faces[nf][3];
		A1=Cube2[nn];
		nn=Faces[nf][2];
		A2=Cube2[nn];
		nn=Faces[nf][1];
		A3=Cube2[nn];
		nn=Faces[nf][0];
		A4=Cube2[nn];

		for (n=0;n<12+6+6;n++)
		{
			aa=Arretes[n][0];
			bb=Arretes[n][1];

			A=Cube1[aa];
			B=Cube1[bb];

			if (ss[nf][aa]*ss[nf][bb]<0)
			{
				t=-ss[nf][aa]/(ss[nf][bb]-ss[nf][aa]);

				VECTORINTERPOL(I,t,B,A);

				VECTORSUB(a1,A2,A1);
				VECTORSUB(b1,I,A1);
				CROSSPRODUCT(c1,b1,a1);
				DOTPRODUCT(s1,N,c1);
				if (s1>=0)
				{
					VECTORSUB(a2,A3,A2);
					VECTORSUB(b2,I,A2);
					CROSSPRODUCT(c2,b2,a2);
					DOTPRODUCT(s2,N,c2);
					if (s2>=0)
					{
						VECTORSUB(a3,A4,A3);
						VECTORSUB(b3,I,A3);
						CROSSPRODUCT(c3,b3,a3);
						DOTPRODUCT(s3,N,c3);
						if (s3>=0)
						{
							VECTORSUB(a4,A1,A4);
							VECTORSUB(b4,I,A4);
							CROSSPRODUCT(c4,b4,a4);
							DOTPRODUCT(s4,N,c4);
							if (s4>=0)
							{
								if (Calculate==1)
								{
									ContactStruct * cs=Contacts[buf].InsertLast();
									cs->Distance=DMAX;
									cs->Normals=N;
									cs->Point=I;
									cs->Types=COLLISION_WITH_OBJECT;
									cs->Num=n2;
									
								}
								res=1;

							}
						}
					}
				}
			}
		}
	}

	for (nf=0;nf<6;nf++)
	{
		N=Norm[nf];
			
		nn=Faces[nf][3];
		P=Cube1[nn];

		a=N.x;
		b=N.y;
		c=N.z;
		d=-(a*P.x+b*P.y+c*P.z);

		cc=0;
		for (n=0;n<8;n++)
		{
			ss[nf][n]=a*Cube2[n].x+b*Cube2[n].y+c*Cube2[n].z+d;
			cc+=(ss[nf][n]>0);
		}

		if (cc==8)
		{
			res=-1;
			break;
		}
	}

	if (res>=0)
	for (nf=0;nf<6;nf++)
	{
		N=Norm[nf];
			
		nn=Faces[nf][3];
		A1=Cube1[nn];
		nn=Faces[nf][2];
		A2=Cube1[nn];
		nn=Faces[nf][1];
		A3=Cube1[nn];
		nn=Faces[nf][0];
		A4=Cube1[nn];

		for (n=0;n<12+6+6;n++)
		{
			aa=Arretes[n][0];
			bb=Arretes[n][1];

			A=Cube2[aa];
			B=Cube2[bb];

			if (ss[nf][aa]*ss[nf][bb]<0)
			{
				t=-ss[nf][aa]/(ss[nf][bb]-ss[nf][aa]);

				VECTORINTERPOL(I,t,B,A);

				VECTORSUB(a1,A2,A1);
				VECTORSUB(b1,I,A1);
				CROSSPRODUCT(c1,b1,a1);
				DOTPRODUCT(s1,N,c1);
				if (s1>=0)
				{
					VECTORSUB(a2,A3,A2);
					VECTORSUB(b2,I,A2);
					CROSSPRODUCT(c2,b2,a2);
					DOTPRODUCT(s2,N,c2);
					if (s2>=0)
					{
						VECTORSUB(a3,A4,A3);
						VECTORSUB(b3,I,A3);
						CROSSPRODUCT(c3,b3,a3);
						DOTPRODUCT(s3,N,c3);
						if (s3>=0)
						{
							VECTORSUB(a4,A1,A4);
							VECTORSUB(b4,I,A4);
							CROSSPRODUCT(c4,b4,a4);
							DOTPRODUCT(s4,N,c4);
							if (s4>=0)
							{
								if (Calculate==1)
								{
									ContactStruct * cs=Contacts[buf].InsertLast();

									cs->Distance=DMAX;
									cs->Normals=-1*N;
									cs->Point=I;
									cs->Types=COLLISION_WITH_OBJECT;
									cs->Num=n2;
									
								}
								res=1;

							}
						}
					}
				}
			}
		}
	}

	if (res<0) res=0;
			
	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pob type multi et arbre
int CPhysic::IsPobMulti1CollidingPobTree2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	int res,res2;
	CPOB * Pob;
	res=0;

	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_NODE))
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,0,buf)==1)
		{
			res=IsPobMulti1CollidingPobTree2(n1,n2,pob1,pob2->Next1,Calculate,buf,0);
			res2=IsPobMulti1CollidingPobTree2(n1,n2,pob1,pob2->Next2,Calculate,buf,0);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	
	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_CLOSING))
	{
		Pob=pob1;
		while ((Pob->ID!=POB_CLOSING)&&(res==0))
		{
			Pob=Pob->Next1;
			res=IsPobClosing1CollidingPobClosing2(n1,n2,Pob,pob2,Calculate,buf);
		}
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobTree1CollidingPobMulti2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	int res,res2;
	CPOB * Pob;
	res=0;

	if ((pob2->ID==POB_MULTINODE)&&(pob1->ID==POB_NODE))
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,0,buf)==1)
		{
			res=IsPobTree1CollidingPobMulti2(n1,n2,pob1->Next1,pob2,Calculate,buf,0);
			res2=IsPobTree1CollidingPobMulti2(n1,n2,pob1->Next2,pob2,Calculate,buf,0);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	
	if ((pob2->ID==POB_MULTINODE)&&(pob1->ID==POB_CLOSING))
	{
		Pob=pob2;
		while ((Pob->ID!=POB_CLOSING)&&(res==0))
		{
			Pob=Pob->Next1;
			res=IsPobClosing1CollidingPobClosing2(n1,n2,pob1,Pob,Calculate,buf);
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobMulti1CollidingPobMulti2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	int res;
	res=0;
	CPOB * Pob1,*Pob2;

	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_MULTINODE))
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,0,buf)==1)
		{
			Pob1=pob1;
			while ((Pob1->ID!=POB_CLOSING)&&(res==0))
			{
				Pob1=Pob1->Next1;

				Pob2=pob2;
				while ((Pob2->ID!=POB_CLOSING)&&(res==0))
				{
					Pob2=Pob2->Next1;

					res=IsPobClosing1CollidingPobClosing2(n1,n2,Pob1,Pob2,Calculate,buf);
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobTree1CollidingPobTree2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	int res,res2;
	int n;

	res=0;
	if ((pob1->ID==POB_CLOSING)&&(pob2->ID==POB_CLOSING))
	{
		res=IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,0,buf)==1)
		{
			n=0;
			if ((N%2)==0)
			{
				if (pob1->ID==POB_CLOSING) n=2;
				else n=1;
			}
			else
			{
				if (pob2->ID==POB_CLOSING) n=1;
				else n=2;
			}

			if (n==1)
			{
				res=IsPobTree1CollidingPobTree2(n1,n2,pob1->Next1,pob2,Calculate,buf,N+1);
				res2=IsPobTree1CollidingPobTree2(n1,n2,pob1->Next2,pob2,Calculate,buf,N+1);
				if ((res2==1)&&(res==0)) res=1;
			}
			else
			{
				res=IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2->Next1,Calculate,buf,N+1);
				res2=IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2->Next2,Calculate,buf,N+1);
				if ((res2==1)&&(res==0)) res=1;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPob1CollidingPob2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	if ((pob1->ID==POB_CLOSING)&&(pob2->ID==POB_CLOSING)) 
		return IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,Calculate,buf);

	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_CLOSING)) 
		return IsPobMulti1CollidingPobTree2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_CLOSING)&&(pob2->ID==POB_MULTINODE)) 
		return IsPobTree1CollidingPobMulti2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_NODE)&&(pob2->ID==POB_CLOSING)) 
		return IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_CLOSING)&&(pob2->ID==POB_NODE)) 
		return IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_NODE)&&(pob2->ID==POB_NODE)) 
		return IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_NODE)) 
		return IsPobMulti1CollidingPobTree2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_NODE)&&(pob2->ID==POB_MULTINODE)) 
		return IsPobTree1CollidingPobMulti2(n1,n2,pob1,pob2,Calculate,buf,N);

	if ((pob1->ID==POB_MULTINODE)&&(pob2->ID==POB_MULTINODE)) 
		return IsPobMulti1CollidingPobMulti2(n1,n2,pob1,pob2,Calculate,buf,N);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPob1CollidingCube2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int res,res2;
	res=0;

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingCube2(n1,n2,pob,0,buf)==1)
			res=IsPob1CollidingCube2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingCube2(n1,n2,pob,Calculate,buf);
		
		res2=IsPob1CollidingCube2(n1,n2,pob->Next1,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;

	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingCube2(n1,n2,pob,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingCube2(n1,n2,pob,0,buf)==1)
		{
			res=IsPob1CollidingCube2(n1,n2,pob->Next1,Calculate,buf);
			//if (res==0)
			res2=IsPob1CollidingCube2(n1,n2,pob->Next2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
			
		}
	}
	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCube1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int N;
	int res,res2;
	res=0;
	N=Contacts[buf].Length();

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingCube2(n2,n1,pob,0,buf)==1)
			res=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingCube2(n2,n1,pob,Calculate,buf);
		if (Calculate==1)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(N);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}
		}

		res2=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;

	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingCube2(n2,n1,pob,Calculate,buf);
		if (Calculate==1)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(N);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}
		}
	}
	else
	{
		if (IsPobClosing1CollidingCube2(n2,n1,pob,0,buf)==1)
		{
			res=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
			res2=IsCube1CollidingPob2(n1,n2,pob->Next2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPob1CollidingSphere2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int res,res2;
	res=0;

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingSphere2(n1,n2,pob,0,buf)==1)
			res=IsPob1CollidingSphere2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingSphere2(n1,n2,pob,Calculate,buf);
		res2=IsPob1CollidingSphere2(n1,n2,pob->Next1,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingSphere2(n1,n2,pob,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingSphere2(n1,n2,pob,0,buf)==1)
		{
			res=IsPob1CollidingSphere2(n1,n2,pob->Next1,Calculate,buf);
			res2=IsPob1CollidingSphere2(n1,n2,pob->Next2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphere1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int N;
	int res,res2;

	N=Contacts[buf].Length();
	res=0;

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingSphere2(n2,n1,pob,0,buf)==1)
			res=IsSphere1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingSphere2(n2,n1,pob,Calculate,buf);
		if (Calculate==1)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(N);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}

		}

		res2=IsSphere1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingSphere2(n2,n1,pob,Calculate,buf);
		if (Calculate==1)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(N);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}

		}
	}
	else
	{
		if (IsPobClosing1CollidingSphere2(n2,n1,pob,0,buf)==1)
		{
			res=IsSphere1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
			res2=IsSphere1CollidingPob2(n1,n2,pob->Next2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}

	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCube1CollidingCube2(int n1,int n2,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};
	
	int res3;
	CVector Cube1[8+6];
	CVector Cube2[8+6];
	CVector Norm[6];
	CVector U1U2[6][2];

	CVector a1,a2,a1a2,a1I;
	float L1L2[6][2];
	float s1,s2;
	CVector ux,uy,uz,OM;
	float ss[8+6];
	float r;
	float t;
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I;
	CMatrix M1,M2,MM1,MM2;
	float rA,rB,DL,rA1,rA2,rA3,rB1,rB2,rB3;
	int n,nn,nn1,nn2,res,res2;
	
	res=0;

	D=Array[n2]->Pos - Array[n1]->Pos;

	r=Array[n1]->Radius+Array[n2]->Radius;
	if (D.Norme()<r)
	{


		M1=Array[n1]->MOActual;
		M2=Array[n2]->MOActual;
		
		A1=Array[n1]->Lx*(Array[n1]->Ux*M1);
		A2=Array[n1]->Ly*(Array[n1]->Uy*M1);
		A3=Array[n1]->Lz*(Array[n1]->Uz*M1);
		
		B1=Array[n2]->Lx*(Array[n2]->Ux*M2);
		B2=Array[n2]->Ly*(Array[n2]->Uy*M2);
		B3=Array[n2]->Lz*(Array[n2]->Uz*M2);
		
		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;

		Ls[3]=B1;
		Ls[4]=B2;
		Ls[5]=B3;

		for (n=0;n<6;n++) Ls[n].Normalise();

		n=0;
		res=1;
		while ((res==1)&&(n<6))
		{
			L=Ls[n];
			rA1=(A1||L);
			rA2=(A2||L);
			rA3=(A3||L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

			rB1=(B1||L);
			rB2=(B2||L);
			rB3=(B3||L);
			rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

			DL=(D||L);
			DL=f_abs(DL);

			if (DL>rA+rB) res=0; else n++;
		}

		if ((res==1)&&(Calculate==1))	// collision
		{
			res=0;
            
			MM1=Array[n1]->MActual;
			MM2=Array[n2]->MActual;


			for (n=0;n<8+6;n++)
			{
				p=Cube[n][0]*Array[n1]->Lx*Array[n1]->Ux + Cube[n][1]*Array[n1]->Ly*Array[n1]->Uy + Cube[n][2]*Array[n1]->Lz*Array[n1]->Uz;
				Cube1[n]=p*MM1;
			}

			for (n=0;n<8+6;n++)
			{
				p=Cube[n][0]*Array[n2]->Lx*Array[n2]->Ux + Cube[n][1]*Array[n2]->Ly*Array[n2]->Uy + Cube[n][2]*Array[n2]->Lz*Array[n2]->Uz;
				Cube2[n]=p*MM2;
			}



			ux=Array[n2]->Ux*M2;
			uy=Array[n2]->Uy*M2;
			uz=Array[n2]->Uz*M2;

			
			Norm[0]=uz;
				U1U2[0][0]=ux;
				U1U2[0][1]=uy;
				L1L2[0][0]=Array[n2]->Lx;
				L1L2[0][1]=Array[n2]->Ly;
			Norm[1]=-1*uy;
				U1U2[1][0]=ux;
				U1U2[1][1]=uz;
				L1L2[1][0]=Array[n2]->Lx;
				L1L2[1][1]=Array[n2]->Lz;
			
			Norm[2]=-1*ux;
				U1U2[2][0]=uy;
				U1U2[2][1]=uz;
				L1L2[2][0]=Array[n2]->Ly;
				L1L2[2][1]=Array[n2]->Lz;

			
			Norm[3]=uy;
				U1U2[3][0]=ux;
				U1U2[3][1]=uz;
				L1L2[3][0]=Array[n2]->Lx;
				L1L2[3][1]=Array[n2]->Lz;

			
			Norm[4]=ux;
				U1U2[4][0]=uz;
				U1U2[4][1]=uy;
				L1L2[4][0]=Array[n2]->Lz;
				L1L2[4][1]=Array[n2]->Ly;


			Norm[5]=-1*uz;
				U1U2[5][0]=ux;
				U1U2[5][1]=uy;
				L1L2[5][0]=Array[n2]->Lx;
				L1L2[5][1]=Array[n2]->Ly;


			n=0;
			res2=0;
			while ((n<6))//&&(res2==0))
			{
				Norm[n].Normalise();

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube2[Faces[n][0]].x+b*Cube2[Faces[n][0]].y+c*Cube2[Faces[n][0]].z);

				for (nn=0;nn<8+6;nn++)
				{
					ss[nn]=a*Cube1[nn].x+b*Cube1[nn].y+c*Cube1[nn].z+d;
				}

				
			
				res3=0;
				nn=0;
				while ((nn<12+3))
				{
					nn1=Arretes[nn][0];
					nn2=Arretes[nn][1];

					

					if (ss[nn1]*ss[nn2]<0)
					{
						t=-ss[nn1]/(ss[nn2]-ss[nn1]);
						p=Cube1[nn1]+t*(Cube1[nn2]-Cube1[nn1]);

						//res=1;
	/*
						nnn=0;
						while ((nnn<4)&&(res==1))
						{
							nnn1=Faces[n][nnn];
							nnn2=Faces[n][(nnn+1)%4];

							a1=Cube1[nnn1];
							a2=Cube1[nnn2];
							a1a2=a2-a1;

							a1I=p-a1;

							s1=((a1a2^a1I)||Norm[n]);

							if (s1>0)
							{
								res=0;
							}
							nnn++;
							
						}
	*/


						

						OM=p-Array[n2]->Pos;

						s1=(OM||U1U2[n][0]);
						s2=(OM||U1U2[n][1]);

						if ((f_abs(s1)<L1L2[n][0])&&(f_abs(s2)<L1L2[n][1]))
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							if (res3==0)
								cs->Distance=1;
							else
								cs->Distance=-1;
							cs->Normals=Norm[n];
							cs->Point=p;
							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
						
							res2=1;
							res3=1;
							res=1;
						}
					}
					nn++;
				}
				
				n++;
			}

			// inverse

			ux=Array[n1]->Ux*M1;
			uy=Array[n1]->Uy*M1;
			uz=Array[n1]->Uz*M1;

			
			Norm[0]=uz;
				U1U2[0][0]=ux;
				U1U2[0][1]=uy;
				L1L2[0][0]=Array[n1]->Lx;
				L1L2[0][1]=Array[n1]->Ly;
			Norm[1]=-1*uy;
				U1U2[1][0]=ux;
				U1U2[1][1]=uz;
				L1L2[1][0]=Array[n1]->Lx;
				L1L2[1][1]=Array[n1]->Lz;
			
			Norm[2]=-1*ux;
				U1U2[2][0]=uy;
				U1U2[2][1]=uz;
				L1L2[2][0]=Array[n1]->Ly;
				L1L2[2][1]=Array[n1]->Lz;

			
			Norm[3]=uy;
				U1U2[3][0]=ux;
				U1U2[3][1]=uz;
				L1L2[3][0]=Array[n1]->Lx;
				L1L2[3][1]=Array[n1]->Lz;

			
			Norm[4]=ux;
				U1U2[4][0]=uz;
				U1U2[4][1]=uy;
				L1L2[4][0]=Array[n1]->Lz;
				L1L2[4][1]=Array[n1]->Ly;


			Norm[5]=-1*uz;
				U1U2[5][0]=ux;
				U1U2[5][1]=uy;
				L1L2[5][0]=Array[n1]->Lx;
				L1L2[5][1]=Array[n1]->Ly;



			n=0;
			res2=0;
			while ((n<6))//&&(res2==0))
			{
				Norm[n].Normalise();

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube1[Faces[n][0]].x+b*Cube1[Faces[n][0]].y+c*Cube1[Faces[n][0]].z);

				for (nn=0;nn<8+6;nn++)
				{
					ss[nn]=a*Cube2[nn].x+b*Cube2[nn].y+c*Cube2[nn].z+d;
				}

				
				
				res3=0;

				nn=0;
				while (nn<12+3)
				{
					nn1=Arretes[nn][0];
					nn2=Arretes[nn][1];

					if (ss[nn1]*ss[nn2]<0)
					{
						t=-ss[nn1]/(ss[nn2]-ss[nn1]);
						p=Cube2[nn1]+t*(Cube2[nn2]-Cube2[nn1]);

						OM=p-Array[n1]->Pos;

						s1=(OM||U1U2[n][0]);
						s2=(OM||U1U2[n][1]);

						if ((f_abs(s1)<L1L2[n][0])&&(f_abs(s2)<L1L2[n][1]))
						{

							ContactStruct * cs=Contacts[buf].InsertLast();

							if (res3==0) cs->Distance=1;
							else cs->Distance=-1;

							cs->Normals=-1*Norm[n];
							cs->Point=p;
							cs->Types=COLLISION_WITH_OBJECT;
							cs->Num=n2;
						
							res2=1;
							res3=1;
							res=1;
						}
					}
					nn++;
				}
				n++;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCube1CollidingSphere2(int n1,int n2,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	float t1,t2;
	float x1,y1,z1,x2,y2,z2,xs,ys,zs;
	
	
	CVector Cube1[8+6];
//	CVector Cube2[8+6];
	CVector Norm[6];
//	CVector U1U2[6][2];

	CVector a1,a2,a1a2,a1I;
	
	float s1;
	CVector ux,uy,uz,OM;

	float r;
	
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I;
	CMatrix M1,M2,MM1,MM2;
	float rA,DL,rA1,rA2,rA3;
	int n,nn,nn1,nn2,res,res2;
	

	res=0;


	D=Array[n2]->Pos - Array[n1]->Pos;
	r=Array[n1]->Radius+Array[n2]->Radius;
	if (D.Norme()<r)
	{
		M1=Array[n1]->MOActual;

		A1=Array[n1]->Lx*Array[n1]->Ux;
		A1=A1*M1;
		A2=Array[n1]->Ly*Array[n1]->Uy;
		A2=A2*M1;
		A3=Array[n1]->Lz*Array[n1]->Uz;
		A3=A3*M1;
		
		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;

		Ls[3]=A1+A2;
		Ls[4]=A1+A3;
		Ls[5]=A2+A3;

		Ls[6]=A1+A2+A3;

		for (n=0;n<7;n++) Ls[n].Normalise();
		n=0;
		res2=1;
		while ((res2==1)&&(n<7))
		{
			L=Ls[n];

			rA1=(A1||L);
			rA2=(A2||L);
			rA3=(A3||L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

			DL=(D||L);
			DL=f_abs(DL);

			if (DL>Array[n2]->Radius+rA) res2=0; else n++;
		}

		if ((res2==1)&&(Calculate==0)) res=1;

		if ((res2==1)&&(Calculate==1))
		{

			MM1=Array[n1]->MActual;

			for (n=0;n<8+6;n++)
			{
				p=Cube[n][0]*Array[n1]->Lx*Array[n1]->Ux + Cube[n][1]*Array[n1]->Ly*Array[n1]->Uy + Cube[n][2]*Array[n1]->Lz*Array[n1]->Uz;
				Cube1[n]=p*MM1;
			}


			r=Array[n2]->Radius;
			xs=Array[n2]->Pos.x;
			ys=Array[n2]->Pos.y;
			zs=Array[n2]->Pos.z;

			ux=Array[n1]->Ux*M1;
			uy=Array[n1]->Uy*M1;
			uz=Array[n1]->Uz*M1;

			
			Norm[0]=uz;
				
			Norm[1]=-1*uy;
				
			Norm[2]=-1*ux;
				
			Norm[3]=uy;
			Norm[4]=ux;

			Norm[5]=-1*uz;

			for (n=0;n<6;n++)
			{
				Norm[n].Normalise();

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube1[Faces[n][0]].x+b*Cube1[Faces[n][0]].y+c*Cube1[Faces[n][0]].z);

				s1=a*xs+b*ys+c*zs+d;

				if ((s1>0)&&(s1<r))	// collision possible avec face
				{

					p=-1*Norm[n];

					I=p*Array[n2]->Radius;
					I+=Array[n2]->Pos;

					res=1;

					nn=0;
					while ((nn<4)&&(res==1))
					{
						nn1=Faces[n][nn];
						nn2=Faces[n][(nn+1)%4];

						a1=Cube1[nn1];
						a2=Cube1[nn2];
						a1a2=a2-a1;

						a1I=I-a1;

						s1=((a1a2^a1I)||Norm[n]);

						if (s1>0)
						{
							res=0;
						}
						nn++;
						
					}


					if (res==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
					}


				}
			}

			n=0;
			while (n<12+3)
			{

				nn1=Arretes[n][0];
				nn2=Arretes[n][1];


			
				x1=Cube1[nn1].x;
				y1=Cube1[nn1].y;
				z1=Cube1[nn1].z;

				x2=Cube1[nn2].x;
				y2=Cube1[nn2].y;
				z2=Cube1[nn2].z;

				a=((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
				b=(x1-xs)*(x2-x1) + (y1-ys)*(y2-y1) + (z1-zs)*(z2-z1);
				c=(x1-xs)*(x1-xs) + (y1-ys)*(y1-ys) + (z1-zs)*(z1-zs) - r*r;


				
				d=b*b - a*c;

				if (d>=0)
				{
				
					t1=(-b +(float) sqrtf(d))/a;
					t2=(-b -(float) sqrtf(d))/a;


					if ((t1>=0)&&(t1<=1))
					{
						I.x=x1+t1*(x2-x1);
						I.y=y1+t1*(y2-y1);
						I.z=z1+t1*(z2-z1);
						p=I-Array[n2]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;

					}
				
					if ((t2>=0)&&(t2<=1))
					{
						I.x=x1+t2*(x2-x1);
						I.y=y1+t2*(y2-y1);
						I.z=z1+t2*(z2-z1);
						p=I-Array[n2]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;
					}
				}
				n++;
			}
		}
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphere1CollidingCube2(int n1,int n2,int Calculate,int buf)
{
	const float Cube[8+6][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
	{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
	};

	const int Arretes[12+3][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3},  {8,9},{10,11},{12,13}};
	
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	float t1,t2;
	float x1,y1,z1,x2,y2,z2,xs,ys,zs;
	

//	CVector Cube1[8+6];
	CVector Cube2[8+6];
	CVector Norm[6];
//	CVector U1U2[6][2];

	CVector a1,a2,a1a2,a1I;
	
	float s1;
	CVector ux,uy,uz,OM;
	
	float r;
	
	float a,b,c,d;
	CVector L,A1,A2,A3,B1,B2,B3,p,Ls[25],A,B,D,I;
	CMatrix M1,M2,MM1,MM2;
	float rB,DL,rB1,rB2,rB3;
	int n,nn,nn1,nn2,res,res2;
	

	res=0;


	D=Array[n2]->Pos - Array[n1]->Pos;
	r=Array[n1]->Radius+Array[n2]->Radius;
	if (D.Norme()<r)
	{
		M2=Array[n2]->MOActual;

		B1=Array[n2]->Lx*Array[n2]->Ux;
		B1=B1*M2;
		B2=Array[n2]->Ly*Array[n2]->Uy;
		B2=B2*M2;
		B3=Array[n2]->Lz*Array[n2]->Uz;
		B3=B3*M2;
		
		Ls[0]=B1;
		Ls[1]=B2;
		Ls[2]=B3;

		Ls[3]=B1+B2;
		Ls[4]=B1+B3;
		Ls[5]=B2+B3;

		Ls[6]=B1+B2+B3;

		for (n=0;n<7;n++) Ls[n].Normalise();
		n=0;
		res2=1;
		while ((res2==1)&&(n<3))
		{
			L=Ls[n];

			rB1=(B1||L);
			rB2=(B2||L);
			rB3=(B3||L);
			rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

			DL=(D||L);
			DL=f_abs(DL);

			if (DL>Array[n1]->Radius+rB) res2=0; else n++;
		}

		if ((res2==1)&&(Calculate==0)) res=1;

		if ((res2==1)&&(Calculate==1))
		{

			MM2=Array[n2]->MActual;

			for (n=0;n<8+6;n++)
			{
				p=Cube[n][0]*Array[n2]->Lx*Array[n2]->Ux + Cube[n][1]*Array[n2]->Ly*Array[n2]->Uy + Cube[n][2]*Array[n2]->Lz*Array[n2]->Uz;
				Cube2[n]=p*MM2;
			}


			r=Array[n1]->Radius;
			xs=Array[n1]->Pos.x;
			ys=Array[n1]->Pos.y;
			zs=Array[n1]->Pos.z;


			ux=Array[n2]->Ux*M2;
			uy=Array[n2]->Uy*M2;
			uz=Array[n2]->Uz*M2;

			
			Norm[0]=uz;
			Norm[1]=-1*uy;
			Norm[2]=-1*ux;
			Norm[3]=uy;
			Norm[4]=ux;
			Norm[5]=-1*uz;

			for (n=0;n<6;n++)
			{
				Norm[n].Normalise();

				a=Norm[n].x;
				b=Norm[n].y;
				c=Norm[n].z;
				d=-(a*Cube2[Faces[n][0]].x+b*Cube2[Faces[n][0]].y+c*Cube2[Faces[n][0]].z);

				s1=a*xs+b*ys+c*zs+d;

				if ((s1>0)&&(s1<r))	// collision possible avec face
				{

					p=-1*Norm[n];

					I=p*Array[n1]->Radius;
					I+=Array[n1]->Pos;

					res=1;

					nn=0;
					while ((nn<4)&&(res==1))
					{
						nn1=Faces[n][nn];
						nn2=Faces[n][(nn+1)%4];

						a1=Cube2[nn1];
						a2=Cube2[nn2];
						a1a2=a2-a1;

						a1I=I-a1;

						s1=((a1a2^a1I)||Norm[n]);

						if (s1>0)
						{
							res=0;
						}
						nn++;
						
					}

					if (res==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=-1*p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
					}
				}
			}


			n=0;
			while (n<12+3)
			{

				nn1=Arretes[n][0];
				nn2=Arretes[n][1];


			
				x1=Cube2[nn1].x;
				y1=Cube2[nn1].y;
				z1=Cube2[nn1].z;

				x2=Cube2[nn2].x;
				y2=Cube2[nn2].y;
				z2=Cube2[nn2].z;

				a=((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
				b=(x1-xs)*(x2-x1) + (y1-ys)*(y2-y1) + (z1-zs)*(z2-z1);
				c=(x1-xs)*(x1-xs) + (y1-ys)*(y1-ys) + (z1-zs)*(z1-zs) - r*r;

				d=b*b - a*c;

				if (d>=0)
				{
				
					t1=(-b +(float) sqrtf(d))/a;
					t2=(-b -(float) sqrtf(d))/a;



					if ((t1>=0)&&(t1<=1))
					{
						I.x=x1+t1*(x2-x1);
						I.y=y1+t1*(y2-y1);
						I.z=z1+t1*(z2-z1);
						p=I-Array[n1]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=-1*p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;

					}
					
					if ((t2>=0)&&(t2<=1))
					{
						I.x=x1+t2*(x2-x1);
						I.y=y1+t2*(y2-y1);
						I.z=z1+t2*(z2-z1);
						p=I-Array[n1]->Pos;
						p.Normalise();

						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=-1*p;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Num=n2;
						
						res=1;

					}
				}
				n++;
			}
		}
	}

	return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPHERE-TREE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTree1CollidingSphereTree2(int n1,int n2,int nB1,int nB2,int Calculate,int buf,int N)
{
	int n;
	int res,res2,res3;
	CVector P1,P2;
	float R1,R2;

	if (N==0) DMAX=0;

	P1.Init(Array[n1]->STREE->Spheres[nB1].x,Array[n1]->STREE->Spheres[nB1].y,Array[n1]->STREE->Spheres[nB1].z);
	P2.Init(Array[n2]->STREE->Spheres[nB2].x,Array[n2]->STREE->Spheres[nB2].y,Array[n2]->STREE->Spheres[nB2].z);
	R1=Array[n1]->STREE->Spheres[nB1].r;
	R2=Array[n2]->STREE->Spheres[nB2].r;

	VECTOR3MATRIX(P1,P1,Array[n1]->MActual);
	VECTOR3MATRIX(P2,P2,Array[n2]->MActual);

	res=0;
	res2=0;
	res3=0;

	if ((Array[n1]->STREE->Spheres[nB1].tag<LEVEL_INF_SPHERETREE)&&(Array[n2]->STREE->Spheres[nB2].tag<LEVEL_INF_SPHERETREE))
		res=IsBall1CollidingBall2(n1,n2,P1,R1,P2,R2,Calculate,buf);
	else
	{
		if (IsBall1CollidingBall2(n1,n2,P1,R1,P2,R2,0,buf))
		{
			if ((N&1)==0)
			{
				if (Array[n1]->STREE->Spheres[nB1].tag<LEVEL_INF_SPHERETREE) n=1;
				else n=0;
			}
			else
			{
				if (Array[n2]->STREE->Spheres[nB2].tag<LEVEL_INF_SPHERETREE) n=0;
				else n=1;
			}

			if (n==0)
			{
				if (Array[n1]->STREE->Spheres[nB1].b0!=-1)
					res=IsSphereTree1CollidingSphereTree2(n1,n2,Array[n1]->STREE->Spheres[nB1].b0,nB2,Calculate,buf,N+1);

				if (Array[n1]->STREE->Spheres[nB1].b1!=-1)
					res2=IsSphereTree1CollidingSphereTree2(n1,n2,Array[n1]->STREE->Spheres[nB1].b1,nB2,Calculate,buf,N+1);

				if (Array[n1]->STREE->Spheres[nB1].b2!=-1)
					res3=IsSphereTree1CollidingSphereTree2(n1,n2,Array[n1]->STREE->Spheres[nB1].b2,nB2,Calculate,buf,N+1);

				if ((res==1)||(res2==1)||(res3==1)) res=1;
			}
			else
			{
				if (Array[n2]->STREE->Spheres[nB2].b0!=-1)
					res=IsSphereTree1CollidingSphereTree2(n1,n2,nB1,Array[n2]->STREE->Spheres[nB2].b0,Calculate,buf,N+1);

				if (Array[n2]->STREE->Spheres[nB2].b1!=-1)
					res2=IsSphereTree1CollidingSphereTree2(n1,n2,nB1,Array[n2]->STREE->Spheres[nB2].b1,Calculate,buf,N+1);

				if (Array[n2]->STREE->Spheres[nB2].b2!=-1)
					res3=IsSphereTree1CollidingSphereTree2(n1,n2,nB1,Array[n2]->STREE->Spheres[nB2].b2,Calculate,buf,N+1);

				if ((res==1)||(res2==1)||(res3==1)) res=1;
			}
		}

	}

	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTree1CollidingSphere2(int n1,int n2,int nB1,int Calculate,int buf)
{
	int res,res2,res3;
	CVector P;
	float R;

	P.Init(Array[n1]->STREE->Spheres[nB1].x,Array[n1]->STREE->Spheres[nB1].y,Array[n1]->STREE->Spheres[nB1].z);
	R=Array[n1]->STREE->Spheres[nB1].r;
	VECTOR3MATRIX(P,P,Array[n1]->MActual);
	res=0;
	if (Array[n1]->STREE->Spheres[nB1].tag<LEVEL_INF_SPHERETREE)
	{
		res=IsBall1CollidingSphere2(n1,n2,P,R,Calculate,buf);
	}
	else
	{
		if (IsBall1CollidingSphere2(n1,n2,P,R,0,buf))
		{
			if (Array[n1]->STREE->Spheres[nB1].b0!=-1)
				res=IsSphereTree1CollidingSphere2(n1,n2,Array[n1]->STREE->Spheres[nB1].b0,Calculate,buf);
			if (Array[n1]->STREE->Spheres[nB1].b1!=-1)
				res2=IsSphereTree1CollidingSphere2(n1,n2,Array[n1]->STREE->Spheres[nB1].b1,Calculate,buf);
			if (Array[n1]->STREE->Spheres[nB1].b2!=-1)
				res3=IsSphereTree1CollidingSphere2(n1,n2,Array[n1]->STREE->Spheres[nB1].b2,Calculate,buf);

			if ((res==1)||(res2==1)||(res3==1)) res=1;
		}
	}
	
	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphere1CollidingSphereTree2(int n1,int n2,int nB2,int Calculate,int buf)
{
	int res,res2,res3;
	CVector P;
	float R;

	P.Init(Array[n2]->STREE->Spheres[nB2].x,Array[n2]->STREE->Spheres[nB2].y,Array[n2]->STREE->Spheres[nB2].z);
	R=Array[n2]->STREE->Spheres[nB2].r;
	VECTOR3MATRIX(P,P,Array[n2]->MActual);

	res=0;
	if (Array[n2]->STREE->Spheres[nB2].tag<LEVEL_INF_SPHERETREE)
	{
		res=IsSphere1CollidingBall2(n1,n2,P,R,Calculate,buf);
	}
	else
	{
		if (IsSphere1CollidingBall2(n1,n2,P,R,0,buf))
		{
			if (Array[n2]->STREE->Spheres[nB2].b0!=-1)
				res=IsSphere1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b0,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b1!=-1)
				res2=IsSphere1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b1,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b2!=-1)
				res3=IsSphere1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b2,Calculate,buf);

			if ((res==1)||(res2==1)||(res3==1)) res=1;
		}
	}
	
	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinder1CollidingSphereTree2(int n1,int n2,int nB2,int Calculate,int buf)
{
	int res,res2,res3;
	CVector P;
	float R;

	P.Init(Array[n2]->STREE->Spheres[nB2].x,Array[n2]->STREE->Spheres[nB2].y,Array[n2]->STREE->Spheres[nB2].z);
	R=Array[n2]->STREE->Spheres[nB2].r;

	VECTOR3MATRIX(P,P,Array[n2]->MActual);
	
	res=0;
	if (Array[n2]->STREE->Spheres[nB2].tag<LEVEL_INF_SPHERETREE)
	{
		res=IsCylinder1CollidingBall2(n1,n2,P,R,Calculate,buf);
	}
	else
	{
		if (IsCylinder1CollidingBall2(n1,n2,P,R,0,buf))
		{
			if (Array[n2]->STREE->Spheres[nB2].b0!=-1)
				res=IsCylinder1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b0,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b1!=-1)
				res2=IsCylinder1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b1,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b2!=-1)
				res3=IsCylinder1CollidingSphereTree2(n1,n2,Array[n2]->STREE->Spheres[nB2].b2,Calculate,buf);

			if ((res==1)||(res2==1)||(res3==1)) res=1;
		}
	}
	
	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTree1CollidingCylinder2(int n1,int n2,int nB1,int Calculate,int buf)
{
	int res,res2,res3;
	CVector P;
	float R;

	P.Init(Array[n1]->STREE->Spheres[nB1].x,Array[n1]->STREE->Spheres[nB1].y,Array[n1]->STREE->Spheres[nB1].z);
	R=Array[n1]->STREE->Spheres[nB1].r;
	VECTOR3MATRIX(P,P,Array[n1]->MActual);
	res=0;
	if (Array[n1]->STREE->Spheres[nB1].tag<LEVEL_INF_SPHERETREE)
	{
		res=IsBall1CollidingCylinder2(n1,n2,P,R,Calculate,buf);
	}
	else
	{
		if (IsBall1CollidingCylinder2(n1,n2,P,R,0,buf))
		{
			if (Array[n1]->STREE->Spheres[nB1].b0!=-1)
				res=IsSphereTree1CollidingCylinder2(n1,n2,Array[n1]->STREE->Spheres[nB1].b0,Calculate,buf);
			if (Array[n1]->STREE->Spheres[nB1].b1!=-1)
				res2=IsSphereTree1CollidingCylinder2(n1,n2,Array[n1]->STREE->Spheres[nB1].b1,Calculate,buf);
			if (Array[n1]->STREE->Spheres[nB1].b2!=-1)
				res3=IsSphereTree1CollidingCylinder2(n1,n2,Array[n1]->STREE->Spheres[nB1].b2,Calculate,buf);

			if ((res==1)||(res2==1)||(res3==1)) res=1;
		}
	}
	
	return res;

}


/*
int CPhysic::IsCube1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf)
{
	int N,n;
	int res,res2;
	res=0;
	N=nPoint[buf];

	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingCube2(n2,n1,pob,0)==1)
			res=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPobClosing1CollidingCube2(n2,n1,pob,Calculate,buf);

		if (Calculate==1)
		for (n=N;n<nPoint[buf];n++) Normals[n]=-1*Normals[n];

		
		res2=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;

	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingCube2(n2,n1,pob,Calculate,buf);

		if (Calculate==1)
		for (n=N;n<nPoint[buf];n++) Normals[n]=-1*Normals[n];

	}
	else
	{
		if (IsPobClosing1CollidingCube2(n2,n1,pob,0)==1)
		{

			res=IsCube1CollidingPob2(n1,n2,pob->Next1,Calculate,buf);
			//if (res==0)
			res2=IsCube1CollidingPob2(n1,n2,pob->Next2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	return res;
}
*/

//TOTO



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPob1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf)
{
	int res;

	res=0;

	if (pob1->ID==POB_CLOSING)
		res=IsPobClosing1CollidingSphereTree2(n1,n2,pob1,nB2,Calculate,buf);

	if (pob1->ID==POB_MULTINODE)
		res=IsPobMulti1CollidingSphereTree2(n1,n2,pob1,nB2,Calculate,buf);

	if (pob1->ID==POB_NODE)
		res=IsPobTree1CollidingSphereTree2(n1,n2,pob1,nB2,Calculate,buf);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTree1CollidingPob2(int n1,int n2,int nB1,CPOB * pob2,int Calculate,int buf)
{
	int res;
	int np;

	res=0;

	np=Contacts[buf].Length();

	if (pob2->ID==POB_CLOSING)
		res=IsPobClosing1CollidingSphereTree2(n2,n1,pob2,nB1,Calculate,buf);

	if (pob2->ID==POB_MULTINODE)
		res=IsPobMulti1CollidingSphereTree2(n2,n1,pob2,nB1,Calculate,buf);

	if (pob2->ID==POB_NODE)
		res=IsPobTree1CollidingSphereTree2(n2,n1,pob2,nB1,Calculate,buf);

	if (res==1)
	{
		if (Calculate==1)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(np);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobClosing1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf)
{
	int res,res2,res3;
	CVector P;
	float R;

	P.Init(Array[n2]->STREE->Spheres[nB2].x,Array[n2]->STREE->Spheres[nB2].y,Array[n2]->STREE->Spheres[nB2].z);
	R=Array[n2]->STREE->Spheres[nB2].r;

	VECTOR3MATRIX(P,P,Array[n2]->MActual);
	
	res=0;
	if (Array[n2]->STREE->Spheres[nB2].tag==1)
	{
		res=IsPobClosing1CollidingBall2(n1,n2,pob1,P,R,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingBall2(n1,n2,pob1,P,R,0,buf))
		{
			if (Array[n2]->STREE->Spheres[nB2].b0!=-1)
				res=IsPobClosing1CollidingSphereTree2(n1,n2,pob1,Array[n2]->STREE->Spheres[nB2].b0,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b1!=-1)
				res2=IsPobClosing1CollidingSphereTree2(n1,n2,pob1,Array[n2]->STREE->Spheres[nB2].b1,Calculate,buf);
			if (Array[n2]->STREE->Spheres[nB2].b2!=-1)
				res3=IsPobClosing1CollidingSphereTree2(n1,n2,pob1,Array[n2]->STREE->Spheres[nB2].b2,Calculate,buf);

			if ((res==1)||(res2==1)||(res3==1)) res=1;
		}
	}
	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobMulti1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf)
{
	int res;
	CPOB * Pob1;


	res=0;
	if (pob1->ID==POB_MULTINODE)
	{
		if (IsPobClosing1CollidingSphereTree2(n1,n2,pob1,nB2,0,buf)==1)
		{
			Pob1=pob1;
			while (Pob1->ID!=POB_CLOSING)
			{
				Pob1=Pob1->Next1;
				res+=IsPobClosing1CollidingSphereTree2(n1,n2,Pob1,nB2,Calculate,buf);
			}

			res=(res>0);
		}
	}
	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobTree1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf)
{
	int res,res2;

	res=0;
	if (pob1->ID==POB_CLOSING)
	{
		res=IsPobClosing1CollidingSphereTree2(n1,n2,pob1,nB2,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingSphereTree2(n1,n2,pob1,nB2,0,buf)==1)
		{

			res=IsPobTree1CollidingSphereTree2(n1,n2,pob1->Next1,nB2,Calculate,buf);
			res2=IsPobTree1CollidingSphereTree2(n1,n2,pob1->Next2,nB2,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}
	return res;
}


/*

int CPhysic::IsPobTree1CollidingPobTree2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N)
{
	int res,res2;
	int n;

	res=0;
	if ((pob1->ID==POB_CLOSING)&&(pob2->ID==POB_CLOSING))
	{
		res=IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,Calculate,buf);
	}
	else
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,pob1,pob2,0)==1)
		{

			n=0;
			if ((N%2)==0)
			{
				if (pob1->ID==POB_CLOSING) n=2;
				else n=1;
			}
			else
			{
				if (pob2->ID==POB_CLOSING) n=1;
				else n=2;
			}

			if (n==1)
			{
				res=IsPobTree1CollidingPobTree2(n1,n2,pob1->Next1,pob2,Calculate,buf,N+1);
				res2=IsPobTree1CollidingPobTree2(n1,n2,pob1->Next2,pob2,Calculate,buf,N+1);
				if ((res2==1)&&(res==0)) res=1;
			}
			else
			{
				res=IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2->Next1,Calculate,buf,N+1);
				res2=IsPobTree1CollidingPobTree2(n1,n2,pob1,pob2->Next2,Calculate,buf,N+1);
				if ((res2==1)&&(res==0)) res=1;
			}
		}
	}

	return res;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::numTemporary(CPhysicObject * op)
{
	int res,n;
	
	n=0;
	res=-1;
	while ((n<nArray)&&(res==-1))
	{
		if (Array[n]==op) res=n;
		n++;
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsThereCollision(CPhysicObject *op1,CPhysicObject *op2)
{
	int a,b;

	a=numTemporary(op1);
	b=numTemporary(op2);

	if ((a>=0)&&(b>=0))
	{

		op1->MActual=op1->ActualMatrix();
		op1->MOActual=op1->ActualOrientationMatrix();

		op2->MActual=op2->ActualMatrix();
		op2->MOActual=op2->ActualOrientationMatrix();

		return IsObj1CollidingObj2(a,b,0,0);
	}
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsObj1CollidingObj2(int n1,int n2,int Calculate,int buf)
{
	int res=0;
	int np;

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsSphere1CollidingSphere2(n1,n2,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_OBJECT)&&(Array[n2]->Type==TYPE_OBJECT))
	{
		//VECTORSUB(D,Array[n2]->Pos,Array[n1]->Pos);
		//r=Array[n1]->Radius+Array[n2]->Radius;
		//if (VECTORNORM2(D)<r*r)

		if ((!Array[n1]->enh)||(!Array[n2]->enh)) return IsPobClosing1CollidingPobClosing2(n1,n2,Array[n1]->pob,Array[n2]->pob,Calculate,buf);
		else
		{
			if (IsPobClosing1CollidingPobClosing2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,buf))
				return IsObject1CollidingObject2(n1,n2,Calculate,buf);
		}
	}

	if ((Array[n1]->Type==TYPE_OBJECT)&&(Array[n2]->Type==TYPE_POB))
	{
		return IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,Calculate,buf,0);
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_OBJECT))
	{
		return IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,Calculate,buf,0);
	}

	if ((Array[n1]->Type==TYPE_OBJECT)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsPob1CollidingSphere2(n1,n2,Array[n1]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_OBJECT))
	{
		return IsSphere1CollidingPob2(n1,n2,Array[n2]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsPob1CollidingSphere2(n1,n2,Array[n1]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_POB))
	{
		return IsSphere1CollidingPob2(n1,n2,Array[n2]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_POB))
	{
		return IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,Calculate,buf,0);
	}

	if ((Array[n1]->Type==TYPE_CYLINDER)&&(Array[n2]->Type==TYPE_CYLINDER))
	{
		return IsCylinder1CollidingCylinder2(n1,n2,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_CYLINDER)&&(Array[n2]->Type==TYPE_SPHERETREE))
	{
		return IsCylinder1CollidingSphereTree2(n1,n2,Array[n2]->STREE->nB-1,Calculate,buf);
	}

	if ((Array[n2]->Type==TYPE_CYLINDER)&&(Array[n1]->Type==TYPE_SPHERETREE))
	{
		return IsSphereTree1CollidingCylinder2(n1,n2,Array[n1]->STREE->nB-1,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_CYLINDER))
	{
		return IsPob1CollidingCylinder2(n1,n2,Array[n1]->pob,Calculate,buf);
	}

	if ((Array[n2]->Type==TYPE_POB)&&(Array[n1]->Type==TYPE_CYLINDER))
	{
		return IsCylinder1CollidingPob2(n1,n2,Array[n2]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_CYLINDER)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsCylinder1CollidingBall2(n1,n2,Array[n2]->Pos,Array[n2]->Radius,Calculate,buf);
	}

	if ((Array[n2]->Type==TYPE_CYLINDER)&&(Array[n1]->Type==TYPE_SPHERE))
	{
		np=Contacts[buf].Length();
		res=IsCylinder1CollidingBall2(n2,n1,Array[n1]->Pos,Array[n1]->Radius,Calculate,buf);

		if (Calculate)
		{
			ContactStruct * cs=Contacts[buf].SetCurrent(np);
			while (cs)
			{
				cs->Normals.x=-cs->Normals.x;
				cs->Normals.y=-cs->Normals.y;
				cs->Normals.z=-cs->Normals.z;
				cs=Contacts[buf].GetNext();
			}
		}

		return res;
	}

	if ((Array[n1]->Type==TYPE_SPHERETREE)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsSphereTree1CollidingSphere2(n1,n2,Array[n1]->STREE->nB-1,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_SPHERETREE))
	{
		return IsSphere1CollidingSphereTree2(n1,n2,Array[n2]->STREE->nB-1,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_SPHERETREE)&&(Array[n2]->Type==TYPE_SPHERETREE))
	{
		if (IsPobClosing1CollidingPobClosing2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,buf)==1)
			return IsSphereTree1CollidingSphereTree2(n1,n2,Array[n1]->STREE->nB-1,Array[n2]->STREE->nB-1,Calculate,buf,0);
	}

	if ((Array[n1]->Type==TYPE_SPHERETREE)&&(Array[n2]->Type==TYPE_POB))
	{
		return IsSphereTree1CollidingPob2(n1,n2,Array[n1]->STREE->nB-1,Array[n2]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_SPHERETREE))
	{
		return IsPob1CollidingSphereTree2(n1,n2,Array[n1]->pob,Array[n2]->STREE->nB-1,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_CUBE))
	{
		return IsSphere1CollidingCube2(n1,n2,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		return IsCube1CollidingSphere2(n1,n2,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_CUBE))
	{
		return IsCube1CollidingCube2(n1,n2,Calculate,buf);
	}

	// pob

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_CUBE))
	{
		return IsPob1CollidingCube2(n1,n2,Array[n1]->pob,Calculate,buf);
	}

	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_POB))
	{
		return IsCube1CollidingPob2(n1,n2,Array[n2]->pob,Calculate,buf);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ObjectFacesCollisionIntermediate(CFace *F1,CFace *F2,int nObj,int Calculate,int buf)
{
	const int ArrFace[3][2]={{0,1},{1,2},{2,0}};
	int res=0;
	CVector N;
	float ss[3];
	int n,n1,n2;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	CVector u,A,B;
	float s1,s2,s3;
	float a,b,c,d,t;
	int test;

	N=F2->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F2->v[0]->Calc.x+b*F2->v[0]->Calc.y+c*F2->v[0]->Calc.z);

	ss[0]=DOT(N,F1->v[0]->Calc) + d;
	ss[1]=DOT(N,F1->v[1]->Calc) + d;
	ss[2]=DOT(N,F1->v[2]->Calc) + d;

	if ((ss[0]>0)&&(ss[1]>0)&&(ss[2]>0)) return 0;
	if ((ss[0]<0)&&(ss[1]<0)&&(ss[2]<0)) return 0;

	DMAX=0;
		
	n=0;
	while (n<3)
	{
		
		if (n==0) test=F1->f01;
		if (n==1) test=F1->f12;
		if (n==2) test=F1->f20;

		if (test)
		{
			n1=ArrFace[n][0];
			n2=ArrFace[n][1];

			A=F1->v[n1]->Calc;
			B=F1->v[n2]->Calc;

			if (ss[n1]*ss[n2]<0)
			{
				t=-ss[n1]/(ss[n2]-ss[n1]);

				VECTORINTERPOL(I,t,B,A);

				VECTORSUB(a1,F2->v[1]->Calc,F2->v[0]->Calc);
				VECTORSUB(b1,I,F2->v[0]->Calc);
				CROSSPRODUCT(c1,b1,a1);
				DOTPRODUCT(s1,F2->NormCalc,c1);
				if (s1>=0)
				{
					VECTORSUB(a2,F2->v[2]->Calc,F2->v[1]->Calc);
					VECTORSUB(b2,I,F2->v[1]->Calc);
					CROSSPRODUCT(c2,b2,a2);
					DOTPRODUCT(s2,F2->NormCalc,c2);
					if (s2>=0)
					{
						VECTORSUB(a3,F2->v[0]->Calc,F2->v[2]->Calc);
						VECTORSUB(b3,I,F2->v[2]->Calc);
						CROSSPRODUCT(c3,b3,a3);			
						DOTPRODUCT(s3,F2->NormCalc,c3);
						if (s3>=0)
						{
							if (Calculate==1)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();
								cs->Distance=DMAX;
								cs->Normals=F2->NormCalc;
								cs->Point=I;
								cs->Types=COLLISION_WITH_OBJECT;
								cs->Num=nObj;
								
							}
							res=1;
						}
					}
				}
			}
		}
		n++;
	}
		
	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ObjectFacesCollisionIntermediate2(CFace *F1,CFace *F2,int nObj,int Calculate,int buf)
{
	const int ArrFace[3][2]={{0,1},{1,2},{2,0}};
	int res=0;
	CVector N;
	float ss[3];
	int n,n1,n2;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	CVector u,A,B;
	float s1,s2,s3;
	float a,b,c,d,t;
	int test;

	N=F2->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F2->v[0]->Calc.x+b*F2->v[0]->Calc.y+c*F2->v[0]->Calc.z);

	ss[0]=DOT(N,F1->v[0]->Calc) + d;
	ss[1]=DOT(N,F1->v[1]->Calc) + d;
	ss[2]=DOT(N,F1->v[2]->Calc) + d;

	if ((ss[0]>0)&&(ss[1]>0)&&(ss[2]>0)) return 0;
	if ((ss[0]<0)&&(ss[1]<0)&&(ss[2]<0)) return 0;

	DMAX=0;
		
	n=0;
	while (n<3)
	{

		if (n==0) test=F1->f01;
		if (n==1) test=F1->f12;
		if (n==2) test=F1->f20;

		if (test)
		{
			n1=ArrFace[n][0];
			n2=ArrFace[n][1];

			A=F1->v[n1]->Calc;
			B=F1->v[n2]->Calc;

			if (ss[n1]*ss[n2]<0)
			{
				t=-ss[n1]/(ss[n2]-ss[n1]);

				VECTORINTERPOL(I,t,B,A);

				VECTORSUB(a1,F2->v[1]->Calc,F2->v[0]->Calc);
				VECTORSUB(b1,I,F2->v[0]->Calc);
				CROSSPRODUCT(c1,b1,a1);
				DOTPRODUCT(s1,F2->NormCalc,c1);
				if (s1>=0)
				{
					VECTORSUB(a2,F2->v[2]->Calc,F2->v[1]->Calc);
					VECTORSUB(b2,I,F2->v[1]->Calc);
					CROSSPRODUCT(c2,b2,a2);
					DOTPRODUCT(s2,F2->NormCalc,c2);
					if (s2>=0)
					{
						VECTORSUB(a3,F2->v[0]->Calc,F2->v[2]->Calc);
						VECTORSUB(b3,I,F2->v[2]->Calc);
						CROSSPRODUCT(c3,b3,a3);
						DOTPRODUCT(s3,F2->NormCalc,c3);
						if (s3>=0)
						{
							if (Calculate==1)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();

								cs->Distance=DMAX;
								cs->Normals=-1*F2->NormCalc;
								cs->Point=I;
								cs->Types=COLLISION_WITH_OBJECT;
								cs->Num=nObj;
								
							}
							res=1;
						}
					}
				}
			}
		}
		n++;
	}
		
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsObject1CollidingObject2(int n1,int n2,int Calculate,int buf)
{
	int res=0;
	int n,_n;
	int i,j;
	int x,y,z;
	int _x,_y,_z;
	CVector A,B,F;
	CVector d,u,u1,u2,p1,p2,p,_p;
	float r1,r2;
	CObject3D *obj1,*obj2;
	float K3=1/3.0f;
	
	Axe.Init(0,0,0);
	DMAX=0;

	r1=Array[n1]->Radius;
	p1=Array[n1]->Pos;

	r2=Array[n2]->Radius;
	p2=Array[n2]->Pos;

	obj1=(CObject3D *) Array[n1]->OBJECT;
	obj2=(CObject3D *) Array[n2]->OBJECT;

	if ((obj1)&&(obj2))
	{	
		if ((Array[n1]->SP)&&(Array[n2]->SP))
		{
			for (n=0;n<64;n++)
			{
				Array[n1]->SP->balls[n].tags[buf]=0;
				Array[n2]->SP->balls[n].tags[buf]=0;
			}

			for (x=0;x<Array[n1]->SP->nbx;x++)
				for (y=0;y<Array[n1]->SP->nby;y++)
					for (z=0;z<Array[n1]->SP->nbz;z++)
					{
						n=x+((y+(z<<2))<<2);
						p=Array[n1]->SP->balls[n].calc;

						for (_x=0;_x<Array[n2]->SP->nbx;_x++)
							for (_y=0;_y<Array[n2]->SP->nby;_y++)
								for (_z=0;_z<Array[n2]->SP->nbz;_z++)
								{
									_n=_x+((_y+(_z<<2))<<2);
									_p=Array[n2]->SP->balls[_n].calc;

									if (IsBall1CollidingBall2(-1,-1,p,Array[n1]->SP->balls[n].r,_p,Array[n2]->SP->balls[_n].r,0,buf))
									{
										Array[n1]->SP->balls[n].tags[buf]=1;
										Array[n2]->SP->balls[_n].tags[buf]=1;
									}
								}

					}

		}
		else
		{
			if (Array[n1]->SP)
			{
				for (x=0;x<Array[n1]->SP->nbx;x++)
					for (y=0;y<Array[n1]->SP->nby;y++)
						for (z=0;z<Array[n1]->SP->nbz;z++)
						{
							n=x+((y+(z<<2))<<2);
							p=Array[n1]->SP->balls[n].calc;
							if (IsBall1CollidingBall2(-1,-1,p,Array[n1]->SP->balls[n].r,p2,r2,0,buf)) Array[n1]->SP->balls[n].tags[buf]=1;
							else Array[n1]->SP->balls[n].tags[buf]=0;
						}
			}

			if (Array[n2]->SP)
			{
				for (x=0;x<Array[n2]->SP->nbx;x++)
					for (y=0;y<Array[n2]->SP->nby;y++)
						for (z=0;z<Array[n2]->SP->nbz;z++)
						{
							n=x+((y+(z<<2))<<2);
							p=Array[n2]->SP->balls[n].calc;
							if (IsBall1CollidingBall2(-1,-1,p,Array[n2]->SP->balls[n].r,p1,r1,0,buf)) Array[n2]->SP->balls[n].tags[buf]=1;
							else Array[n2]->SP->balls[n].tags[buf]=0;
						}
			}
		}


		if (Array[n1]->SP)
		{			
			for (n=0;n<obj1->nFaces;n++)
			{
				obj1->Faces[n].tags[buf]=0;
				if ((Array[n1]->SP->balls[obj1->Faces[n].v[0]->temp].tags[buf])||(Array[n1]->SP->balls[obj1->Faces[n].v[1]->temp].tags[buf])||(Array[n1]->SP->balls[obj1->Faces[n].v[2]->temp].tags[buf]))
				{
					VECTORINIT(u,obj1->Faces[n].mp0.x,obj1->Faces[n].mp0.y,obj1->Faces[n].mp1.x);
					VECTORSUB(u,u,p2);
					float f=VECTORNORM(u);
					if (f<obj1->Faces[n].coef1+r2) obj1->Faces[n].tags[buf]=1;
				}
			}
		}
		else
		{
			for (n=0;n<obj1->nFaces;n++)
			{
				obj1->Faces[n].tags[buf]=0;
				VECTORINIT(u,obj1->Faces[n].mp0.x,obj1->Faces[n].mp0.y,obj1->Faces[n].mp1.x);
				VECTORSUB(u,u,p2);
				float f=VECTORNORM(u);
				if (f<obj1->Faces[n].coef1+r2) obj1->Faces[n].tags[buf]=1;
			}
		}

		if (Array[n2]->SP)
		{
			for (n=0;n<obj2->nFaces;n++)
			{
				obj2->Faces[n].tags[buf]=0;

				if ((Array[n2]->SP->balls[obj2->Faces[n].v[0]->temp].tags[buf])||(Array[n2]->SP->balls[obj2->Faces[n].v[1]->temp].tags[buf])||(Array[n2]->SP->balls[obj2->Faces[n].v[2]->temp].tags[buf]))
				{
					VECTORINIT(u,obj2->Faces[n].mp0.x,obj2->Faces[n].mp0.y,obj2->Faces[n].mp1.x);
					VECTORSUB(u,u,p1);
					float f=VECTORNORM(u);				
					if (f<obj2->Faces[n].coef1+r1) 	obj2->Faces[n].tags[buf]=1;
				}
			}
		}
		else
		{
			for (n=0;n<obj2->nFaces;n++)
			{
				obj2->Faces[n].tags[buf]=0;
				VECTORINIT(u,obj2->Faces[n].mp0.x,obj2->Faces[n].mp0.y,obj2->Faces[n].mp1.x);
				VECTORSUB(u,u,p1);
				float f=VECTORNORM(u);
				if (f<obj2->Faces[n].coef1+r1) obj2->Faces[n].tags[buf]=1;
			}
		}

		for (i=0;i<obj1->nFaces;i++)
		{
			if (obj1->Faces[i].tags[buf])
			for (j=0;j<obj2->nFaces;j++)
			{
				if (obj2->Faces[j].tags[buf])
				{
					res|=ObjectFacesCollisionIntermediate(&(obj1->Faces[i]),&(obj2->Faces[j]),n2,Calculate,buf);
					res|=ObjectFacesCollisionIntermediate2(&(obj2->Faces[j]),&(obj1->Faces[i]),n2,Calculate,buf);
				}		
			}
		}
	}

	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  COLLISION AVEC FACE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::FacesCollisionIntermediate(CFace *F1,CFace *F2,bool snd_is_mesh,int Calculate,int buf)
{
	const int ArrFace[3][2]={{0,1},{1,2},{2,0}};
	int res=0;
	CVector N;
	float ss[3];
	int n,n1,n2;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	float s1,s2,s3;
	float a,b,c,d,t;

	N=F2->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F2->v[0]->Calc.x+b*F2->v[0]->Calc.y+c*F2->v[0]->Calc.z);

	for (n=0;n<3;n++) ss[n]=DOT(N,F1->v[n]->Calc) + d;
		
	n=0;
	while (n<3)
	{
		n1=ArrFace[n][0];
		n2=ArrFace[n][1];

		if (ss[n1]*ss[n2]<0)
		{
			t=-ss[n1]/(ss[n2]-ss[n1]);

			VECTORINTERPOL(I,t,F1->v[n2]->Calc,F1->v[n1]->Calc);

			VECTORSUB(a1,F2->v[1]->Calc,F2->v[0]->Calc);
			VECTORSUB(b1,I,F2->v[0]->Calc);
			CROSSPRODUCT(c1,b1,a1);
			DOTPRODUCT(s1,F2->NormCalc,c1);
			if (s1>=0)
			{
				VECTORSUB(a2,F2->v[2]->Calc,F2->v[1]->Calc);
				VECTORSUB(b2,I,F2->v[1]->Calc);
				CROSSPRODUCT(c2,b2,a2);
				DOTPRODUCT(s2,F2->NormCalc,c2);
				if (s2>=0)
				{
					VECTORSUB(a3,F2->v[0]->Calc,F2->v[2]->Calc);
					VECTORSUB(b3,I,F2->v[2]->Calc);
					CROSSPRODUCT(c3,b3,a3);
					DOTPRODUCT(s3,F2->NormCalc,c3);
					if (s3>=0)
					{
						if (Calculate==1)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();

							if (snd_is_mesh)
							{
								cs->Normals=F2->NormCalc;
								cs->Point=I;
								cs->Types=COLLISION_WITH_MESH;
								if (ss[n1]<0) cs->Distance=-ss[n1];
								else cs->Distance=-ss[n2];

								cs->Num=F2->tag;
								
							}
							else
							{
								cs->Normals=F1->NormCalc;
								cs->Point=I;
								cs->Types=COLLISION_WITH_MESH;
								if (ss[n1]<0) cs->Distance=-ss[n1];
								else cs->Distance=-ss[n2];
								cs->Num=F1->tag;
								
							}
						}
						res=1;
					}
				}
			}
		}
		n++;
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::PropGenEdgeCollisionFace(int nObj,CFace *F,int type,int tag,int Calculate,int buf)
{
	int res=0;
	CVector u;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	float s1,s2,s3;
	float d,t;
	float sa,sb;
	CVector A,B;

	A=Array[nObj]->aPos;
	B=Array[nObj]->Posdt;

	VECTORSUB(u,B,A);
	VECTORNORMALISE(u);

	VECTORMUL(u,u,Array[nObj]->Radius);
	VECTORSUB(A,A,u);
	VECTORADD(B,B,u);

	d=-DOT(F->NormCalc,F->v[0]->Calc);
	sa=DOT(F->NormCalc,A) + d;
	sb=DOT(F->NormCalc,B) + d;

	if ((sa*sb<0)&&(sa>0))
	{
		t=-sa/(sb-sa);
		VECTORINTERPOL(I,t,B,A);

		VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc);
		VECTORSUB(b1,I,F->v[0]->Calc);
		CROSSPRODUCT(c1,b1,a1);
		DOTPRODUCT(s1,F->NormCalc,c1);
		if (s1>=0)
		{
			VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc);
			VECTORSUB(b2,I,F->v[1]->Calc);
			CROSSPRODUCT(c2,b2,a2);
			DOTPRODUCT(s2,F->NormCalc,c2);
			if (s2>=0)
			{
				VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc);
				VECTORSUB(b3,I,F->v[2]->Calc);
				CROSSPRODUCT(c3,b3,a3);
				DOTPRODUCT(s3,F->NormCalc,c3);
				if (s3>=0)
				{
					if (Calculate==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();

						Array[nObj]->Pos=I+Array[nObj]->Radius*F->NormCalc;
						cs->Normals=F->NormCalc;
						cs->Point=I;
						cs->Types=type;
						if (sa<0) cs->Distance=-sa;
						else cs->Distance=-sb;
						cs->Num=tag;
						
					}
					res=1;
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::EdgeCollisionFace(CObject3D *obj,int A,int B,CFace *F,int type,int tag,int Calculate,int buf)
{
	int res=0;
	CVector u;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	float s1,s2,s3;
	float a,b,c,d,t;
	float sa,sb;

	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	DOTPRODUCT(sa,F->NormCalc,obj->Vertices[A].Calc);
	sa+=d;
	DOTPRODUCT(sb,F->NormCalc,obj->Vertices[B].Calc);
	sb+=d;

	if (sa*sb<0)
	{
		t=-sa/(sb-sa);
		VECTORINTERPOL(I,t,obj->Vertices[B].Calc,obj->Vertices[A].Calc);

		VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc);
		VECTORSUB(b1,I,F->v[0]->Calc);
		CROSSPRODUCT(c1,b1,a1);
		DOTPRODUCT(s1,F->NormCalc,c1);
		if (s1>=0)
		{
			VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc);
			VECTORSUB(b2,I,F->v[1]->Calc);
			CROSSPRODUCT(c2,b2,a2);
			DOTPRODUCT(s2,F->NormCalc,c2);
			if (s2>=0)
			{
				VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc);
				VECTORSUB(b3,I,F->v[2]->Calc);
				CROSSPRODUCT(c3,b3,a3);
				DOTPRODUCT(s3,F->NormCalc,c3);
				if (s3>=0)
				{
					if (Calculate==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=F->NormCalc;
						cs->Point=I;
						cs->Types=type;
						if (sa<0) cs->Distance=-sa;
						else cs->Distance=-sb;
						cs->Num=tag;
						
					}
					res=1;
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::SegmentCollisionFace(CObject3D *obj,CVector &O,CVector &A,CFace *F,int type,int tag,int Calculate,int buf)
{
	int res=0;
	CVector N,u;
	CVector a1,a2,a3,I;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	float s1,s2,s3;
	float a,b,c,d,t;
	float sa,sb;

	N=F->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	sa=DOT(N,O) + d;
	sb=DOT(N,A) + d;

	if (sa*sb<0)
	{
		t=-sa/(sb-sa);

		VECTORINTERPOL(I,t,A,O);

		VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc);
		VECTORSUB(b1,I,F->v[0]->Calc);
		CROSSPRODUCT(c1,b1,a1);
		DOTPRODUCT(s1,F->NormCalc,c1);
		if (s1>=0)
		{
			VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc);
			VECTORSUB(b2,I,F->v[1]->Calc);
			CROSSPRODUCT(c2,b2,a2);
			DOTPRODUCT(s2,F->NormCalc,c2);
			if (s2>=0)
			{
				VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc);
				VECTORSUB(b3,I,F->v[2]->Calc);
				CROSSPRODUCT(c3,b3,a3);
				DOTPRODUCT(s3,F->NormCalc,c3);
				if (s3>=0)
				{
					if (Calculate==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();

						cs->Normals=F->NormCalc;
						cs->Point=I;
						cs->Types=type;
						if (sa<0) cs->Distance=-sa;
						else cs->Distance=-sb;
						cs->Num=tag;
						
					}
					res=1;
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsFaceCollidingFace(CFace *F,CFace *Fref,int Calculate,int buf)
{
	return FacesCollisionIntermediate(F,Fref,true,Calculate,buf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::TrivialTestPobFace(int nObj,CPOB *pob,CFace *F)
{
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	int n,nn;
	CVector _Cube[8];
	float a,b,c,d;
	float ss;
	CVector p;
	int cc=0;
	int cc2=0;

	for (n=0;n<8;n++)
	{
		p=pob->O + Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
		VECTOR3MATRIX(_Cube[n],p,Array[nObj]->MActual);
	}

	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
	
	for (nn=0;nn<8;nn++)
	{
		ss=a*_Cube[nn].x+b*_Cube[nn].y+c*_Cube[nn].z+d;
		cc+=(ss>0);
		cc2+=(ss<0);
	}

	if (cc==8) return 0;
	if (cc2==8) return 0;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CPhysic::IsPobClosingCollidingFace(int n0,CPOB * pob,CFace *F,int Calculate,int buf)
{
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	const int Arretes[12][2]={{0,1},{1,2},{2,3},{3,0},{5,6},{6,7},{7,4},{4,5},{6,2},{5,1},{4,0},{7,3}};
	CVector Cube1[8];
	float s0;
	CVector p,ux,uy,uz,OM;
	float ss[8];
	float t;
	float a,b,c,d;
	int n,nn,nn1,nn2,res;
	CVector uu1,uu2;

	res=0;

	for (n=0;n<8;n++)
	{
		p=pob->O + Cube[n][0]*(COEF_ENVELOPPE+pob->Lx)*pob->Ux + Cube[n][1]*(COEF_ENVELOPPE+pob->Ly)*pob->Uy + Cube[n][2]*(COEF_ENVELOPPE+pob->Lz)*pob->Uz;
		VECTOR3MATRIX(Cube1[n],p,Array[n0]->MActual);
	}

	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	int cc=0;
	for (nn=0;nn<8;nn++)
	{
		ss[nn]=a*Cube1[nn].x+b*Cube1[nn].y+c*Cube1[nn].z+d;
		cc+=(ss[nn]>0);
	}

	if (cc==8) return 0;

	nn=0;
	while (nn<12)
	{
		nn1=Arretes[nn][0];
		nn2=Arretes[nn][1];

		if (ss[nn1]*ss[nn2]<0)
		{
			t=-ss[nn1]/(ss[nn2]-ss[nn1]);
			p=Cube1[nn1]+t*(Cube1[nn2]-Cube1[nn1]);

			VECTORSUB(uu1,p,F->v[0]->Calc);
			VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
			CROSSPRODUCT(OM,uu1,uu2);
			s0=DOT(OM,F->NormCalc);
			if (s0>-SMALLFFACEINT)
			{
				VECTORSUB(uu1,p,F->v[1]->Calc);
				VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
				CROSSPRODUCT(OM,uu1,uu2);
				s0=DOT(OM,F->NormCalc);
				if (s0>-SMALLFFACEINT)
				{
					VECTORSUB(uu1,p,F->v[2]->Calc);
					VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
					CROSSPRODUCT(OM,uu1,uu2);						
					s0=DOT(OM,F->NormCalc);
					if (s0>-SMALLFFACEINT)
					{
						if (Calculate==1)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							cs->Normals=F->NormCalc;
							cs->Point=p;
							if (ss[nn1]<0) s0=-ss[nn1];
							else s0=-ss[nn2]; 
							cs->Types=COLLISION_WITH_MESH;
							cs->Num=F->tag;
							cs->Distance=s0;
							
						}
						res=1;
					}
				}
			}

		}
		nn++;
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBallCollidingFace(CVector &P,float R,CFace *F,int Calculate,int buf,float a,float b,float c,float d)
{
	float t,r;
	float s0,s1;
	CVector OM,p,u;
	CVector uu1,uu2;
	int res=0;
	CVector A,B,I;

	VECTORMUL(u,F->NormCalc,1000.0f);
	VECTORADD(A,P,u);
	VECTORSUB(B,P,u);

	s0=a*A.x + b*A.y + c*A.z +d;
	s1=a*B.x + b*B.y + c*B.z +d;

	t=-s0/(s1-s0);
	VECTORINTERPOL(I,t,B,A);
	VECTORSUB(u,I,P);
	r=VECTORNORM2(u);

	if (r<R*R)
	{
		VECTORSUB(uu1,I,F->v[0]->Calc);
		VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
		CROSSPRODUCT(OM,uu1,uu2);
		s0=DOT(OM,F->NormCalc);
		if (s0>0)
		{
			VECTORSUB(uu1,I,F->v[1]->Calc);
			VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
			CROSSPRODUCT(OM,uu1,uu2);
			s0=DOT(OM,F->NormCalc);
			if (s0>0)
			{
				VECTORSUB(uu1,I,F->v[2]->Calc);
				VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
				CROSSPRODUCT(OM,uu1,uu2);
				s0=DOT(OM,F->NormCalc);
				if (s0>0)
				{
					if (Calculate==1)
					{
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=F->NormCalc;
						cs->Point=I;
						cs->Types=COLLISION_WITH_MESH;
						cs->Num=F->tag;
						cs->Distance=R-sqrtf(r);
					}
					res=1;
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBallCollidingFace(CVector &P,float R,CFace *F,int Calculate,int buf)
{
	float t,r;
	float s0,s1;
	CVector OM,p,u;
	CVector uu1,uu2;
	int res;
	CVector A,B,I;

	res=0;

	float a=F->NormCalc.x;
	float b=F->NormCalc.y;
	float c=F->NormCalc.z;
	float d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	VECTORMUL(u,F->NormCalc,1000.0f);
	VECTORADD(A,P,u);
	VECTORSUB(B,P,u);

	s0=a*A.x + b*A.y + c*A.z +d;
	s1=a*B.x + b*B.y + c*B.z +d;

    if (s0*s1<0)
    {
        t=-s0/(s1-s0);
        VECTORINTERPOL(I,t,B,A);
        VECTORSUB(u,I,P);
        r=VECTORNORM(u);

        if (r<R)
        {
            VECTORSUB(uu1,I,F->v[0]->Calc);
            VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
            CROSSPRODUCT(OM,uu1,uu2);
            s0=DOT(OM,F->NormCalc);
            if (s0>-SMALLFFACEINT)
            {
                VECTORSUB(uu1,I,F->v[1]->Calc);
                VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
                CROSSPRODUCT(OM,uu1,uu2);
                s0=DOT(OM,F->NormCalc);
                if (s0>-SMALLFFACEINT)
                {
                    VECTORSUB(uu1,I,F->v[2]->Calc);
                    VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
                    CROSSPRODUCT(OM,uu1,uu2);
                    s0=DOT(OM,F->NormCalc);
                    if (s0>-SMALLFFACEINT)
                    {
                        if (Calculate==1)
                        {
                            ContactStruct * cs=Contacts[buf].InsertLast();
                            cs->Normals=F->NormCalc;
                            cs->Point=I;
                            cs->Types=COLLISION_WITH_MESH;
                            cs->Num=F->tag;
                            cs->Distance=R-r;
                        }
                        res=1;
                    }
                }
            }
        }
    }

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinderCollidingFace(int nObj,CFace *F,int Calculate,int buf)
{
	const int ArrFace[3][2]={{0,1},{1,2},{2,0}};
	CVector OM,I,N,v,ni,u,A,B,In,p;
	float sA,sB,t,s,s0;
	float ss[3];
	int res=0;
	float a,b,c,d;
	int nn,nn1,nn2;
	CVector uu1,uu2;
	int np=0;

	a=F->NormCalc.x;
	b=F->NormCalc.y;
	c=F->NormCalc.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	N.x=a;
	N.y=b;
	N.z=c;

	u=Array[nObj]->U*Array[nObj]->MOActual;

	u.Normalise();
	N.Normalise();

	ni=u^N;

	v=ni^u;

	v.Normalise();

	if (f_abs(u||N)>1-SMALLF) v.Init(0,0,0);

	A=Array[nObj]->Pos + Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
	B=Array[nObj]->Pos - Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
	
	sA=a*A.x+b*A.y+c*A.z+d;
	sB=a*B.x+b*B.y+c*B.z+d;

	s=Array[nObj]->Pos.x*a + Array[nObj]->Pos.y*b +Array[nObj]->Pos.z*c +d;

	if (s>0)
	{
		if (sA*sB<0)
		{
			t=-sA/(sB-sA);

			if ((t>-SMALLF)&&(t<1+SMALLF))
			{
				
				if (t<0) t=0;
				if (t>1) t=1;

				I=A+t*(B-A);

				VECTORSUB(uu1,I,F->v[0]->Calc);
				VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
				CROSSPRODUCT(OM,uu1,uu2);
				s0=DOT(OM,F->NormCalc);
				if (s0>-SMALLF)
				{
					VECTORSUB(uu1,I,F->v[1]->Calc);
					VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
					CROSSPRODUCT(OM,uu1,uu2);
					s0=DOT(OM,F->NormCalc);
					if (s0>-SMALLF)
					{
						VECTORSUB(uu1,I,F->v[2]->Calc);
						VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
						CROSSPRODUCT(OM,uu1,uu2);						
						s0=DOT(OM,F->NormCalc);
						if (s0>-SMALLF)
						{
							res=1;

							if (Calculate)
							{
								if (sB<0) s=sB;
								if (sA<0) s=sA;
								if (sB<0) In=B;
								if (sA<0) In=A;

								ContactStruct * cs=Contacts[buf].InsertLast();

								cs->Normals=F->NormCalc;
								cs->Types=COLLISION_WITH_MESH;
								cs->Num=F->tag;
								cs->Distance=-s*2;
								cs->Point=I;
								
							}
						}
					}
				}
			}
		}
		else
		{
			if (sA<0)
			{
				if (sA<sB) s=-sA; else s=-sB;

				if (res==0)
				{

					a=F->NormCalc.x;
					b=F->NormCalc.y;
					c=F->NormCalc.z;
					d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

					N.x=a;
					N.y=b;
					N.z=c;
					N.Normalise();
					
					I=(A+B)/2;

					s=a*I.x+b*I.y+c*I.z+d;

					I-=s*N;

					VECTORSUB(uu1,I,F->v[0]->Calc);
					VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
					CROSSPRODUCT(OM,uu1,uu2);
					s0=DOT(OM,F->NormCalc);
					if (s0>-SMALLF)
					{
						VECTORSUB(uu1,I,F->v[1]->Calc);
						VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
						CROSSPRODUCT(OM,uu1,uu2);
						s0=DOT(OM,F->NormCalc);
						if (s0>-SMALLF)
						{
							VECTORSUB(uu1,I,F->v[2]->Calc);
							VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
							CROSSPRODUCT(OM,uu1,uu2);						
							s0=DOT(OM,F->NormCalc);
							if (s0>-SMALLF)
							{					
								res=1;
								if (Calculate)
								{
									ContactStruct * cs=Contacts[buf].InsertLast();

									cs->Normals=F->NormCalc;
									cs->Types=COLLISION_WITH_MESH;
									cs->Num=F->tag;
									cs->Distance=-s;
									cs->Point=I;
									
								}
							}
						}
					}
				}		
			}
		}

		if (res==0)
		{

			v=(F->v[0]->Calc+F->v[1]->Calc+F->v[2]->Calc)/3 - Array[nObj]->Pos;
			v.Normalise();

			A=Array[nObj]->Pos + Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
			B=Array[nObj]->Pos - Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
			
			sA=a*A.x+b*A.y+c*A.z+d;
			sB=a*B.x+b*B.y+c*B.z+d;
			

			if (sA*sB<0)
			{

				t=-sA/(sB-sA);

				if ((t>-SMALLF)&&(t<1+SMALLF))
				{
					

					if (t<0) t=0;
					if (t>1) t=1;

					I=A+t*(B-A);

					VECTORSUB(uu1,I,F->v[0]->Calc);
					VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
					CROSSPRODUCT(OM,uu1,uu2);
					s0=DOT(OM,F->NormCalc);
					if (s0>-SMALLF)
					{
						VECTORSUB(uu1,I,F->v[1]->Calc);
						VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
						CROSSPRODUCT(OM,uu1,uu2);
						s0=DOT(OM,F->NormCalc);
						if (s0>-SMALLF)
						{
							VECTORSUB(uu1,I,F->v[2]->Calc);
							VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
							CROSSPRODUCT(OM,uu1,uu2);						
							s0=DOT(OM,F->NormCalc);
							if (s0>-SMALLF)
							{					

								res=1;
								if (Calculate)
								{
									if (sB<0) s=sB;
									if (sA<0) s=sA;
									if (sB<0) In=B;
									if (sA<0) In=A;

									ContactStruct * cs=Contacts[buf].InsertLast();

									cs->Normals=F->NormCalc;
									cs->Types=COLLISION_WITH_MESH;
									cs->Num=F->tag;
									cs->Distance=-s;
									cs->Point=I;
									
								}
							}
						}
					}
				}
			}

		}


		if (res==0)
		{
                 //Array[nObj]->ActualOrientationMatrix()
			u=Array[nObj]->U*Array[nObj]->MOActual;

			u.Normalise();

			A=Array[nObj]->Pos + Array[nObj]->L*u/2;
			B=Array[nObj]->Pos - Array[nObj]->L*u/2;

			I=A;

			d=-DOT(u,I);
			for (nn=0;nn<3;nn++)
				ss[nn]=DOT(u,F->v[nn]->Calc) + d;
			
			nn=0;
			while (nn<3)
			{
				nn1=ArrFace[nn][0];
				nn2=ArrFace[nn][1];

				if (ss[nn1]*ss[nn2]<0)
				{
					t=-ss[nn1]/(ss[nn2]-ss[nn1]);
					if ((t>-SMALLF)&&(t<1+SMALLF))
					{

						p=F->v[nn1]->Calc+t*(F->v[nn2]->Calc-F->v[nn1]->Calc);

						VECTORSUB(OM,p,I);

						if (VECTORNORM(OM)<Array[nObj]->Radius)
						{
							if (Calculate==1)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();

								cs->Normals=F->NormCalc;
								cs->Point=p;
								VECTORSUB(N,I,p);
								s=Array[nObj]->Radius-VECTORNORM(N);
								cs->Distance=0;
								cs->Types=COLLISION_WITH_MESH;
								cs->Num=F->tag;
								np++;
								
							}
							res=1;
						}
					}
				}
				nn++;
			}


			u=-1*u;
			I=B;

			d=-(u||I);
			for (nn=0;nn<3;nn++)
				ss[nn]=(u||F->v[nn]->Calc) + d;
			
			nn=0;
			while (nn<3)
			{
				nn1=ArrFace[nn][0];
				nn2=ArrFace[nn][1];

				if (ss[nn1]*ss[nn2]<0)
				{
					t=-ss[nn1]/(ss[nn2]-ss[nn1]);

					if ((t>-SMALLF)&&(t<1+SMALLF))
					{
						p=F->v[nn1]->Calc+t*(F->v[nn2]->Calc-F->v[nn1]->Calc);

						VECTORSUB(OM,p,I);

						if (VECTORNORM(OM)<Array[nObj]->Radius)
						{
							if (Calculate==1)
							{
								ContactStruct * cs=Contacts[buf].InsertLast();

								cs->Normals=F->NormCalc;
								cs->Point=p;
								VECTORSUB(N,I,p);
								s=Array[nObj]->Radius-VECTORNORM(N);
								cs->Distance=0;
								cs->Types=COLLISION_WITH_MESH;
								cs->Num=F->tag;
								np++;
								
							}
							res=1;
						}
					}
				}
				nn++;
			}
		}

	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPOBCollidingFace(int nObj,CPOB * pob,CFace *F,int Calculate,int buf)
{
	int res;

	res=0;

	if (pob->ID==POB_MULTINODECLOSING)
	{
		res+=IsPobClosingCollidingFace(nObj,pob,F,Calculate,buf);
		
		res+=IsPOBCollidingFace(nObj,pob->Next1,F,Calculate,buf);
		res=(res>0);
	}
	else
	if (pob->ID==POB_MULTINODE)
	{
		if (IsPobClosingCollidingFace(nObj,pob,F,0,buf)==1)
			res=IsPOBCollidingFace(nObj,pob->Next1,F,Calculate,buf);
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPobClosingCollidingFace(nObj,pob,F,Calculate,buf);
	}
	else
	{
		if (IsPobClosingCollidingFace(nObj,pob,F,0,buf)==1)
		{
			res+=IsPOBCollidingFace(nObj,pob->Next1,F,Calculate,buf);
			res+=IsPOBCollidingFace(nObj,pob->Next2,F,Calculate,buf);
			res=(res>0);
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTreeCollidingFace(int nObj,int n,CFace *F,int Calculate,int buf)
{
	Ball * B;
	int res;
	CVector P;
	float R;
	res=0;

	P.x=Array[nObj]->STREE->Spheres[n].x;
	P.y=Array[nObj]->STREE->Spheres[n].y;
	P.z=Array[nObj]->STREE->Spheres[n].z;
	R=Array[nObj]->STREE->Spheres[n].r;

	VECTOR3MATRIX(P,P,Array[nObj]->MActual);

	if (Array[nObj]->STREE->Spheres[n].tag<2)
	{
		res=IsBallCollidingFace(P,R,F,Calculate,buf);
	}
	else
	{
		
		if (IsBallCollidingFace(P,R,F,0,buf))
		{
			B=&(Array[nObj]->STREE->Spheres[n]);

			if (B->b0!=-1) res+=IsSphereTreeCollidingFace(nObj,Array[nObj]->STREE->Spheres[n].b0,F,Calculate,buf);
			if (B->b1!=-1) res+=IsSphereTreeCollidingFace(nObj,Array[nObj]->STREE->Spheres[n].b1,F,Calculate,buf);
			if (B->b2!=-1) res+=IsSphereTreeCollidingFace(nObj,Array[nObj]->STREE->Spheres[n].b2,F,Calculate,buf);
	
			res=(res>0);
		}		
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsObjCollidingMesh(int nObj,CCollisionMesh * mesh,int Calculate,int buf)
{
	int *i;
	int res=0;
	int x,y,z;
#define OCMunsur3 1.0f/3.0f;
	float a,b,c,d,s;
	CFace *F;
	CList <int> * list;
	CVector P,u,u1,u2,p;
	float R;
	CObject3D *obj;
	int n;

	P=Array[nObj]->Pos;
	R=Array[nObj]->Radius;

	if (IsPobSphere(P,R,mesh->bounds,mesh->M,mesh->Radius)==0) return 0;

#if ((!defined(GLES))&&(!defined(GLES20)))||(defined(GLESFULL))
	list=mesh->quad.GetFaceList(P,R,mesh->M,buf);
#else
	list=mesh->quad.GetFaceList(P,R,mesh->M);
#endif

	switch (Array[nObj]->Type)
	{
		case TYPE_SPHERE:
			i=list->GetFirst();
			while (i)
			{
				F=&(mesh->obj->Faces[*i]);
				a=F->NormCalc.x;
				b=F->NormCalc.y;
				c=F->NormCalc.z;
				d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
				if (a*P.x+b*P.y+c*P.z+d>0)
					res+=IsBallCollidingFace(P,R,F,Calculate,buf);
				i=list->GetNext();
			}
			break;

		case TYPE_POB:
			i=list->GetFirst();
			while (i)
			{
				F=&(mesh->obj->Faces[*i]);
				a=F->NormCalc.x;
				b=F->NormCalc.y;
				c=F->NormCalc.z;
				d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
				if (a*P.x+b*P.y+c*P.z+d>0) res+=IsPOBCollidingFace(nObj,Array[nObj]->pob,F,Calculate,buf);
				i=list->GetNext();
			}
			break;

		case TYPE_SPHERETREE:
			i=list->GetFirst();
			while (i)
			{
				F=&(mesh->obj->Faces[*i]);
				a=F->NormCalc.x;
				b=F->NormCalc.y;
				c=F->NormCalc.z;
				d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
				if (a*P.x+b*P.y+c*P.z+d>0) res+=IsSphereTreeCollidingFace(nObj,Array[nObj]->STREE->nB-1,&(mesh->obj->Faces[*i]),Calculate,buf);
				i=list->GetNext();
			}
			break;

		case TYPE_CYLINDER:
			i=list->GetFirst();
			while (i)
			{
				F=&(mesh->obj->Faces[*i]);
				a=F->NormCalc.x;
				b=F->NormCalc.y;
				c=F->NormCalc.z;
				d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
				if (a*P.x+b*P.y+c*P.z+d>0) res+=IsCylinderCollidingFace(nObj,&(mesh->obj->Faces[*i]),Calculate,buf);
				i=list->GetNext();
			}
			break;

		case TYPE_OBJECT:
			i=list->GetFirst();
			while (i)
			{
				F=&(mesh->obj->Faces[*i]);

				a=F->NormCalc.x;
				b=F->NormCalc.y;
				c=F->NormCalc.z;
				d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

				if (a*P.x+b*P.y+c*P.z+d>0)
				{
					CVector C,BB1,BB2;
					C.x=F->mp0.x;
					C.y=F->mp0.y;
					C.z=F->mp1.x;
					
					BB1.x=C.x-F->mp1.y;
					BB1.y=C.y-F->mp2.x;
					BB1.z=C.z-F->mp2.y;

					BB2.x=C.x+F->mp1.y;
					BB2.y=C.y+F->mp2.x;
					BB2.z=C.z+F->mp2.y;

					//float r=mesh->obj->Faces[*i].coef1;

					if (!((P.x+R<BB1.x)||(P.x-R>BB2.x)||(P.y+R<BB1.y)||(P.y-R>BB2.y)||(P.z+R<BB1.z)||(P.z-R>BB2.z)))
	#ifndef _DEFINES_API_CODE_GLITCH_
					if (TrivialTestPobFace(nObj,Array[nObj]->pob,F))
	#endif
					{
						obj=(CObject3D*) Array[nObj]->OBJECT;
						if (obj)
						{		
							//float rr=r*r;
							if (Array[nObj]->SP)
							{
								for (x=0;x<Array[nObj]->SP->nbx;x++)
									for (y=0;y<Array[nObj]->SP->nby;y++)
										for (z=0;z<Array[nObj]->SP->nbz;z++)
										{
											n=x+4*(y+4*z);
											Array[nObj]->SP->balls[n].tags[buf]=0;	
											p.Init(Array[nObj]->SP->balls[n].calc.x,Array[nObj]->SP->balls[n].calc.y,Array[nObj]->SP->balls[n].calc.z);
											s=a*p.x + b*p.y +c*p.z +d;
											if (f_abs(s)<Array[nObj]->SP->balls[n].r) Array[nObj]->SP->balls[n].tags[buf]=1;
										}

								float xxx;

								for (n=0;n<obj->edges->nList;n++) 
								{
									if ((Array[nObj]->SP->balls[obj->Vertices[obj->edges->List[n].a].temp].tags[buf])||(Array[nObj]->SP->balls[obj->Vertices[obj->edges->List[n].b].temp].tags[buf]))
									{
										u1=obj->Vertices[obj->edges->List[n].a].Calc;
										u2=obj->Vertices[obj->edges->List[n].b].Calc;

										if (u1.x>u2.x) { xxx=u2.x; u2.x=u1.x; u1.x=xxx; }
										if (u1.y>u2.y) { xxx=u2.y; u2.y=u1.y; u1.y=xxx; }
										if (u1.z>u2.z) { xxx=u2.z; u2.z=u1.z; u1.z=xxx; }

										if (!((u2.x<BB1.x)||(u1.x>BB2.x)||(u2.y<BB1.y)||(u1.y>BB2.y)||(u2.z<BB1.z)||(u1.z>BB2.z)))
											res+=EdgeCollisionFace(obj,obj->edges->List[n].a,obj->edges->List[n].b,F,COLLISION_WITH_MESH,F->tag,Calculate,buf);
										/*
										VECTORSUB(u1,obj->Vertices[obj->edges->List[n].a].Calc,C);
										VECTORSUB(u2,obj->Vertices[obj->edges->List[n].b].Calc,C);
										if ((VECTORNORM2(u1)<rr)||(VECTORNORM2(u2)<rr)) res+=EdgeCollisionFace(obj,obj->edges->List[n].a,obj->edges->List[n].b,F,COLLISION_WITH_MESH,F->tag,Calculate,buf);
										/**/
									}
								}
							}
							else
							{
								float xxx;

								for (n=0;n<obj->edges->nList;n++) 
								{
									u1=obj->Vertices[obj->edges->List[n].a].Calc;
									u2=obj->Vertices[obj->edges->List[n].b].Calc;

									if (u1.x>u2.x) { xxx=u2.x; u2.x=u1.x; u1.x=xxx; }
									if (u1.y>u2.y) { xxx=u2.y; u2.y=u1.y; u1.y=xxx; }
									if (u1.z>u2.z) { xxx=u2.z; u2.z=u1.z; u1.z=xxx; }

									if (!((u2.x<BB1.x)||(u1.x>BB2.x)||(u2.y<BB1.y)||(u1.y>BB2.y)||(u2.z<BB1.z)||(u1.z>BB2.z)))
										res+=EdgeCollisionFace(obj,obj->edges->List[n].a,obj->edges->List[n].b,F,COLLISION_WITH_MESH,F->tag,Calculate,buf);

									/*
									VECTORSUB(u1,obj->Vertices[obj->edges->List[n].a].Calc,C);
									VECTORSUB(u2,obj->Vertices[obj->edges->List[n].b].Calc,C);
									if ((VECTORNORM2(u1)<rr)||(VECTORNORM2(u2)<rr)) res+=EdgeCollisionFace(obj,obj->edges->List[n].a,obj->edges->List[n].b,F,COLLISION_WITH_MESH,F->tag,Calculate,buf);
									/**/
								}
							}
						}
					}
				}
				i=list->GetNext();
			}
			break;
	};

	res=(res>0);

	float vitesse=VECTORNORM(Array[nObj]->Vit);


	if ((res==0)&&(DT*vitesse>Array[nObj]->Radius))
	{
		i=list->GetFirst();
		while (i)
		{
			F=&(mesh->obj->Faces[*i]);
			if (PropGenEdgeCollisionFace(nObj,F,COLLISION_WITH_MESH,F->tag,Calculate,buf)) res=1;
			i=list->GetNext();
		}
	}

	return res;
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  COLLISION AVEC PLAN

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPOBClosingCollidingPlane(int nObj,CPOB * pob,int nPlan,int Calculate,int buf)
{
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	int n;
	CVector p,pc,N;
	float s;
	int res;

	res=0;
	n=0;

	while ((n<8)&&(!((Calculate==0)&&(res==1))))
	{
		p=pob->O+Cube[n][0]*pob->Lx*pob->Ux + Cube[n][1]*pob->Ly*pob->Uy + Cube[n][2]*pob->Lz*pob->Uz;
		pc=p*Array[nObj]->MActual;
		s=Planes[nPlan].Distance(pc);
		if (s<COEF_ENVELOPPE*Calculate)
		{
			res=1;

			if (Calculate==1)
			{
				N.x=Planes[nPlan].a;
				N.y=Planes[nPlan].b;
				N.z=Planes[nPlan].c;

				ContactStruct * cs=Contacts[buf].InsertLast();

				cs->Normals=N;
				cs->Types=COLLISION_WITH_PLANE;
				cs->Num=nPlan;
				cs->Distance=-s;
				cs->Point=pc-s*N;
				
			}
		}
		n++;
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPOBCollidingPlane(int nObj,CPOB * pob,int nPlan,int Calculate,int buf)
{
	int res,res2;

	res=0;

	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPOBClosingCollidingPlane(nObj,pob,nPlan,Calculate,buf);
		res2=IsPOBCollidingPlane(nObj,pob->Next1,nPlan,Calculate,buf);
		if ((res2==1)&&(res==0)) res=1;
	}
	else
	if (pob->ID==POB_MULTINODE)
	{
		if (IsPOBClosingCollidingPlane(nObj,pob,nPlan,0,buf)==1)
			res=IsPOBCollidingPlane(nObj,pob->Next1,nPlan,Calculate,buf);
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPOBClosingCollidingPlane(nObj,pob,nPlan,Calculate,buf);
	}
	else
	{
		if (IsPOBClosingCollidingPlane(nObj,pob,nPlan,0,buf)==1)
		{
			res=IsPOBCollidingPlane(nObj,pob->Next1,nPlan,Calculate,buf);
//			if (res==0)
			res2=IsPOBCollidingPlane(nObj,pob->Next2,nPlan,Calculate,buf);
			if ((res2==1)&&(res==0)) res=1;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsBallCollidingPlane(CVector &P,float r,int nPlan,int Calculate,int buf)
{
	CVector N;
	float s;
	int res;
	
	s=Planes[nPlan].Distance(P);

	if (s<r)
	{
		res=1;
		if (Calculate==1)
		{

			N.x=Planes[nPlan].a;
			N.y=Planes[nPlan].b;
			N.z=Planes[nPlan].c;
				
			ContactStruct * cs=Contacts[buf].InsertLast();

			cs->Normals=N;
			cs->Types=COLLISION_WITH_PLANE;
			cs->Num=nPlan;
			cs->Distance=r-s;
			cs->Point=P-r*N;
		}
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsCylinderCollidingPlane(int nObj,int nPlan,int Calculate,int buf)
{
	CVector N,v,n,u,A,B,In;
	float sA,sB,t,s;
	int res=0;
	CMatrix Mo=Array[nObj]->MOActual;

	N.x=Planes[nPlan].a;
	N.y=Planes[nPlan].b;
	N.z=Planes[nPlan].c;

	u=Array[nObj]->U*Mo;

	u.Normalise();
	N.Normalise();

	n=u^N;
	v=n^u;
	v.Normalise();

	if (f_abs(u||N)>1-SMALLF) v.Init(0,0,0);

	A=Array[nObj]->Pos + Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
	B=Array[nObj]->Pos - Array[nObj]->L*u/2  - Array[nObj]->Radius*v;
	

	sA=Planes[nPlan].Distance(A);
	sB=Planes[nPlan].Distance(B);

	if (sA*sB<0)
	{
		t=-sA/(sB-sA);

		if ((t>-SMALLF)&&(t<1+SMALLF))
		{
			res=1;

			if (t<0) t=0;
			if (t>1) t=1;

			if (Calculate)
			{
				if (sB<0) s=sB;
				if (sA<0) s=sA;
				if (sB<0) In=B;
				if (sA<0) In=A;

				ContactStruct * cs=Contacts[buf].InsertLast();

				cs->Normals=N;
				cs->Types=COLLISION_WITH_PLANE;
				cs->Num=nPlan;
				cs->Distance=-s;
				cs->Point=A+t*(B-A);
				
			}
		}
	}
	else
	{
		if (sA<0)
		{
			res=1;
			if (Calculate)
			{
				ContactStruct * cs=Contacts[buf].InsertLast();

				cs->Normals=N;
				cs->Types=COLLISION_WITH_PLANE;
				cs->Num=nPlan;
				cs->Distance=0;
				cs->Point=A;
				
				cs=Contacts[buf].InsertLast();
				cs->Normals=N;
				cs->Types=COLLISION_WITH_PLANE;
				cs->Num=nPlan;
				cs->Distance=0;
				cs->Point=B;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsSphereTreeCollidingPlane(int nObj,int n,int nPlan,int Calculate,int buf)
{
	Ball * B;
	int res;
	CVector P;
	float R;
	res=0;

	P.x=Array[nObj]->STREE->Spheres[n].x;
	P.y=Array[nObj]->STREE->Spheres[n].y;
	P.z=Array[nObj]->STREE->Spheres[n].z;
	R=Array[nObj]->STREE->Spheres[n].r;

	P=P*Array[nObj]->MActual;

	if (Array[nObj]->STREE->Spheres[n].tag<2)
	{	
		res=IsBallCollidingPlane(P,R,nPlan,Calculate,buf);
	}
	else
	{
		
		if (IsBallCollidingPlane(P,R,nPlan,0,buf))
		{
			B=&(Array[nObj]->STREE->Spheres[n]);

			if (B->b0!=-1)
				res+=IsSphereTreeCollidingPlane(nObj,Array[nObj]->STREE->Spheres[n].b0,nPlan,Calculate,buf);

			if (B->b1!=-1)
				res+=IsSphereTreeCollidingPlane(nObj,Array[nObj]->STREE->Spheres[n].b1,nPlan,Calculate,buf);
			
			if (B->b2!=-1)
				res+=IsSphereTreeCollidingPlane(nObj,Array[nObj]->STREE->Spheres[n].b2,nPlan,Calculate,buf);

			
			res=(res>0);
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CPhysic::VolumObjUnderPlane(int nObj,int nPlan)
{
	int n;
	float a,b,c,d;
	CMatrix M;
	CVector Pos,Ux,Uy,Uz,v;
	CPhysicObject *op=Array[nObj];
	CVector min,max;
	float s;
	float v1=0;
	float v2=0;
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };

	a=Planes[nPlan].a;
	b=Planes[nPlan].b;
	c=Planes[nPlan].c;
	d=Planes[nPlan].d;

	if (op->pob)
	{
		M=op->MOActual;
		Pos=op->Pos;
		Ux=op->pob->Lx*(op->pob->Ux*M);
		Uy=op->pob->Ly*(op->pob->Uy*M);
		Uz=op->pob->Lz*(op->pob->Uz*M);
		min.Init(100000,100000,100000);
		max.Init(-100000,-100000,-100000);
		v1=(float) (2.0f*(op->pob->Lx+op->pob->Ly+op->pob->Lz)/3.0f);
		for (n=0;n<8;n++)
		{
			v=Pos + Cube[n][0]*Ux + Cube[n][1]*Uy + Cube[n][2]*Uz;
			s=a*v.x + b*v.y + c*v.z +d;
			if (s<0)
			{
				if (v.x<min.x) min.x=v.x;
				if (v.y<min.y) min.y=v.y;
				if (v.z<min.z) min.z=v.z;
				if (v.x>max.x) max.x=v.x;
				if (v.y>max.y) max.y=v.y;
				if (v.z>max.z) max.z=v.z;
			}
		}		

		v2=(max.y-min.y)/v1;
	}

	return v2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsObjCollidingPlane(int nObj,int nPlan,int Calculate,int buf)
{
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	int n;
	CVector p,pc,N,Y,I;
	float s;
	int res;

	res=0;

	if (Array[nObj]->Type==TYPE_CYLINDER)
	{
		res=IsCylinderCollidingPlane(nObj,nPlan,Calculate,buf);
	}

	if (Array[nObj]->Type==TYPE_SPHERE)
	{
		p=Array[nObj]->Pos;
		s=Planes[nPlan].Distance(p);

		if (s<Array[nObj]->Radius+COEF_ENVELOPPE*Calculate)
		{
			res=1;
			if (Calculate==1)
			{
				Y.Init(0,1,0);

				N.x=Planes[nPlan].a;
				N.y=Planes[nPlan].b;
				N.z=Planes[nPlan].c;
					
				ContactStruct * cs=Contacts[buf].InsertLast();
				cs->Normals=N;
				cs->Point=p-Array[nObj]->Radius*N;		// inexact
				cs->Types=COLLISION_WITH_PLANE;
				cs->Num=nPlan;
				cs->Distance=Array[nObj]->Radius-s;
				
			}
		}
	}
    
    if (Array[nObj]->Type==TYPE_OBJECT)
    {
        p=Array[nObj]->Pos;
        s=Planes[nPlan].Distance(p);

        if (s<Array[nObj]->Radius+COEF_ENVELOPPE*Calculate)
        {
            CObject3D *obj=(CObject3D*) Array[nObj]->OBJECT;
            if (obj)
            {
                float a=Planes[nPlan].a;
                float b=Planes[nPlan].b;
                float c=Planes[nPlan].c;
                float d=Planes[nPlan].d;

                for (n=0;n<obj->edges->nList;n++)
                {
                    CVector ua=obj->Vertices[obj->edges->List[n].a].Calc;
                    CVector ub=obj->Vertices[obj->edges->List[n].b].Calc;
                    
                    float sa=a*ua.x+b*ua.y+c*ua.z+d;
                    float sb=a*ub.x+b*ub.y+c*ub.z+d;
                    
                    if (sa*sb<0)
                    {
                        res=1;
                        
                        float t=-sa/(sb-sa);
                        VECTORINTERPOL(I,t,ub,ua);
                        
                        if (Calculate==1)
                        {
                            N.x=Planes[nPlan].a;
                            N.y=Planes[nPlan].b;
                            N.z=Planes[nPlan].c;

                            ContactStruct * cs=Contacts[buf].InsertLast();

                            cs->Normals=N;
                            cs->Types=COLLISION_WITH_PLANE;
                            cs->Num=nPlan;
                            if (sa<sb) cs->Distance=f_abs(sa);
                            else cs->Distance=f_abs(sb);
                            cs->Point=I;
                        }
                    }
                }
            }
        }
    }

	if (Array[nObj]->Type==TYPE_SPHERETREE)
	{
		p=Array[nObj]->Pos;
		s=Planes[nPlan].Distance(p);

		if (s<Array[nObj]->Radius+COEF_ENVELOPPE*Calculate)
		{
			res=IsSphereTreeCollidingPlane(nObj,Array[nObj]->STREE->nB-1,nPlan,Calculate,buf);
		}
	}

	if (Array[nObj]->Type==TYPE_CUBE)
	{
		p=Array[nObj]->Pos;
		s=Planes[nPlan].Distance(p);

		if (s<Array[nObj]->Radius)
		{
			n=0;
			while ((n<8)&&(!((Calculate==0)&&(res==1))))
			{
				p=Cube[n][0]*Array[nObj]->Lx*Array[nObj]->Ux + Cube[n][1]*Array[nObj]->Ly*Array[nObj]->Uy + Cube[n][2]*Array[nObj]->Lz*Array[nObj]->Uz;
				pc=p*Array[nObj]->MActual;
				s=Planes[nPlan].Distance(pc);

				if (s<0)
				{
					res=1;

					if (Calculate==1)
					{
						N.x=Planes[nPlan].a;
						N.y=Planes[nPlan].b;
						N.z=Planes[nPlan].c;

						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=N;
						cs->Point=pc;		// inexact
						cs->Types=COLLISION_WITH_PLANE;
						cs->Num=nPlan;
						cs->Distance=-s;
					}
				}
				n++;
			}
		}
	}

	
	if (Array[nObj]->Type==TYPE_POB)
	{
		p=Array[nObj]->Pos;
		s=Planes[nPlan].Distance(p);

		if (s<Array[nObj]->Radius)
		{
			res=IsPOBCollidingPlane(nObj,Array[nObj]->pob,nPlan,Calculate,buf);		
		}
	}	
	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CPhysic::CalculateVolumePOB(CPOB * pob)
{
	float dV;
	dV=0;
	if (pob->ID==POB_CLOSING)
	{
		dV=pob->Lx*pob->Ly*pob->Lz*8;
	}
	else
	if (pob->ID==POB_MULTINODECLOSING)
	{
		dV=pob->Lx*pob->Ly*pob->Lz*8;
		dV+=CalculateVolumePOB(pob->Next1);
	}
	else
	if (pob->ID==POB_MULTINODE)
	{
		dV+=CalculateVolumePOB(pob->Next1);
	}
	else
	{
		dV+=CalculateVolumePOB(pob->Next1);
		dV+=CalculateVolumePOB(pob->Next2);
	}

	return dV;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateMatriceInertiePOB(CPOB * pob,CMatrix * M,float p)
{
	CMatrix I,R,Rt,I0;
	
	int n1,n2,n3;
	float lx,ly,lz,dV,t1,t2,t3;
	float x,y,z;
	float Ixx,Iyy,Izz,Ixy,Iyz,Ixz;
	CVector P;

#define NB 64

	if (pob->ID==POB_MULTINODECLOSING)
	{
		lx=(pob->Lx*2)/NB;
		ly=(pob->Ly*2)/NB;
		lz=(pob->Lz*2)/NB;

		dV=lx*ly*lz;


		Ixx=Iyy=Izz=0;
		Ixz=Ixy=Iyz=0;

		for (n1=0;n1<NB;n1++)
			for (n2=0;n2<NB;n2++)
				for (n3=0;n3<NB;n3++)
				{
					t1=(float) n1/NB;
					t2=(float) n2/NB;
					t3=(float) n3/NB;
					P=pob->O + (-pob->Lx + t1*pob->Lx*2)*pob->Ux
							 + (-pob->Ly + t2*pob->Ly*2)*pob->Uy
							 + (-pob->Lz + t3*pob->Lz*2)*pob->Uz;


					x=P.x;
					y=P.y;
					z=P.z;

					Ixx+=p*(y*y + z*z)*dV;
					Iyy+=p*(x*x + z*z)*dV;
					Izz+=p*(x*x + y*y)*dV;
/*
					Ixy+=p*(x*y)*dV;
					Ixz+=p*(x*z)*dV;
					Iyz+=p*(y*z)*dV;

  */

				}

		
		I.Id();
		I.a[0][0]=Ixx;
		I.a[1][1]=Iyy;
		I.a[2][2]=Izz;
		I.a[0][1]=-Ixy;
		I.a[1][0]=-Ixy;
		I.a[0][2]=-Ixz;
		I.a[2][0]=-Ixz;
		I.a[1][2]=-Iyz;
		I.a[2][1]=-Iyz;

		

		for (n1=0;n1<3;n1++)
			for (n2=0;n2<3;n2++)
			{
				M->a[n1][n2]+=I.a[n1][n2];
			}

			CalculateMatriceInertiePOB(pob->Next1,M,p);

	}
	else
	if (pob->ID==POB_CLOSING)
	{
/*		
		a=pob->Lx;
		b=pob->Ly;
		c=pob->Lz;
		
		r=(float) sqrtf(a*a+b*b+c*c);
		I0.Id();
		I0.a[0][0]=2*m*r*r/5;
		I0.a[1][1]=2*m*r*r/5;
		I0.a[2][2]=2*m*r*r/5;

		I=I0;

  */
		/*
		a=pob->Lx*2;
		b=pob->Ly*2;
		c=pob->Lz*2;
		I0.Id();
		I0.a[0][0]=m*(b*b + c*c)/12;
		I0.a[1][1]=m*(a*a + c*c)/12;
		I0.a[2][2]=m*(b*b + a*a)/12;


		R.a[0][0]=pob->Ux.x;
		R.a[1][0]=pob->Ux.y;
		R.a[2][0]=pob->Ux.z;

		R.a[0][1]=pob->Uy.x;
		R.a[1][1]=pob->Uy.y;
		R.a[2][1]=pob->Uy.z;

		R.a[0][2]=pob->Uz.x;
		R.a[1][2]=pob->Uz.y;
		R.a[2][2]=pob->Uz.z;


		Rt.Transpose(R);

		I=(Rt*I0)*R;
		


		
		dd=pob->O.y*pob->O.y + pob->O.z*pob->O.z;
		I.a[0][0]+=m*dd;

		dd=pob->O.x*pob->O.x + pob->O.z*pob->O.z;
		I.a[1][1]+=m*dd;

		dd=pob->O.x*pob->O.x + pob->O.y*pob->O.y;
		I.a[2][2]+=m*dd;
		*/


		lx=(pob->Lx*2)/NB;
		ly=(pob->Ly*2)/NB;
		lz=(pob->Lz*2)/NB;

		dV=lx*ly*lz;


		Ixx=Iyy=Izz=0;
		Ixz=Ixy=Iyz=0;

		for (n1=0;n1<NB;n1++)
			for (n2=0;n2<NB;n2++)
				for (n3=0;n3<NB;n3++)
				{
					t1=(float) n1/NB;
					t2=(float) n2/NB;
					t3=(float) n3/NB;
					P=pob->O + (-pob->Lx + t1*pob->Lx*2)*pob->Ux
							 + (-pob->Ly + t2*pob->Ly*2)*pob->Uy
							 + (-pob->Lz + t3*pob->Lz*2)*pob->Uz;


					x=P.x;
					y=P.y;
					z=P.z;

					Ixx+=p*(y*y + z*z)*dV;
					Iyy+=p*(x*x + z*z)*dV;
					Izz+=p*(x*x + y*y)*dV;
/*
					Ixy+=p*(x*y)*dV;
					Ixz+=p*(x*z)*dV;
					Iyz+=p*(y*z)*dV;

  */

				}

		
		I.Id();
		I.a[0][0]=Ixx;
		I.a[1][1]=Iyy;
		I.a[2][2]=Izz;
		I.a[0][1]=-Ixy;
		I.a[1][0]=-Ixy;
		I.a[0][2]=-Ixz;
		I.a[2][0]=-Ixz;
		I.a[1][2]=-Iyz;
		I.a[2][1]=-Iyz;

		

		for (n1=0;n1<3;n1++)
			for (n2=0;n2<3;n2++)
			{
				M->a[n1][n2]+=I.a[n1][n2];
			}

	}
	else
	{
		CalculateMatriceInertiePOB(pob->Next1,M,p);
		CalculateMatriceInertiePOB(pob->Next2,M,p);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InertiaTensor()
{
	float a,b,c;
	float m;

	switch (Type)
	{
	case TYPE_CUBE:
		//Radius=(float) sqrtf(Lx*Lx+Ly*Ly+Lz*Lz);
		m=Mass;
		a=Lx*2;
		b=Ly*2;
		c=Lz*2;
		I.Id();
		I.a[0][0]=m*(b*b + c*c)/12;
		I.a[1][1]=m*(a*a + c*c)/12;
		I.a[2][2]=m*(b*b + a*a)/12;
		break;

	case TYPE_POB:
		//Radius=(float) sqrtf(pob->Lx*pob->Lx +pob->Ly*pob->Ly +pob->Lz*pob->Lz);
		m=Mass;
		a=pob->Lx*2;
		b=pob->Ly*2;
		c=pob->Lz*2;
		I.Id();
		I.a[0][0]=m*(b*b + c*c)/12;
		I.a[1][1]=m*(a*a + c*c)/12;
		I.a[2][2]=m*(b*b + a*a)/12;
		break;

	case TYPE_SPHERE:
		m=Mass;
		I.Id();
		I.a[0][0]=2*m*Radius*Radius/5;
		I.a[1][1]=2*m*Radius*Radius/5;
		I.a[2][2]=2*m*Radius*Radius/5;
		break;

	case TYPE_SPHERETREE:
		m=Mass;
		a=pob->Lx*2;
		b=pob->Ly*2;
		c=pob->Lz*2;
		I.Id();
		I.a[0][0]=m*(b*b + c*c)/12;
		I.a[1][1]=m*(a*a + c*c)/12;
		I.a[2][2]=m*(b*b + a*a)/12;
		break;

	case TYPE_CYLINDER:
		m=Mass;
		I.Id();
		I.a[0][0]=m*(Radius*Radius + L*L/3)/4;
		I.a[1][1]=m*(Radius*Radius + L*L/3)/4;
		I.a[2][2]=m*Radius*Radius/2;
		break;

	case TYPE_OBJECT:
		if (ball_inertiatensor)
		{
			m=Mass;
			I.Id();
			I.a[0][0]=2*m*Radius*Radius/5;
			I.a[1][1]=2*m*Radius*Radius/5;
			I.a[2][2]=2*m*Radius*Radius/5;
		}
		else
		{
			//Radius=(float) sqrtf(pob->Lx*pob->Lx +pob->Ly*pob->Ly +pob->Lz*pob->Lz);
			m=Mass;
			a=pob->Lx*2;
			b=pob->Ly*2;
			c=pob->Lz*2;
			I.Id();
			I.a[0][0]=m*(b*b + c*c)/12;
			I.a[1][1]=m*(a*a + c*c)/12;
			I.a[2][2]=m*(b*b + a*a)/12;
		}
		break;
	};

	Inv_I.Id();
	Inv_I.a[0][0]=1.0f/I.a[0][0];
	Inv_I.a[1][1]=1.0f/I.a[1][1];
	Inv_I.a[2][2]=1.0f/I.a[2][2];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::Reset()
{
	Alpha=0;
	Omega.Init(0,0,0);
	F.Init(0,0,0);
	Vit.Init(0,0,0);
	T.Init(0,0,0);
	Lg.Init(0,0,0);
	aQ=Q;
	Q_dt=Q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::ResetWoQ()
{
	Alpha=0;
	Omega.Init(0,0,0);
	F.Init(0,0,0);
	Vit.Init(0,0,0);
	T.Init(0,0,0);
	Lg.Init(0,0,0);
	
	aQ=Q;
	Q_dt=Q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::Init()
{
	int n;

	for (n=0;n<nArray;n++)
	{
		Array[n]->Reset();
		Array[n]->InertiaTensor();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetBit(int n)
{
	Predict[n]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::TestBit(int n)
{
	return ((int) Predict[n]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetBitC(int n)
{
	NoCollision[n]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::TestBitC(int n)
{
	return ((int) NoCollision[n]);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ApproximateTimeCollision(int n1,int n2)
{
	CVector u,A1,A2,A3,B1,B2,B3,L,U,U0,N;
	CVector Ls[6];
	CMatrix M1,M2;
	int n,res;
	float ss,r,rA,rB,rA1,rA2,rA3,rB1,rB2,rB3,DL,dmax,d1,d2;
	float distanceminimale;
	float K0;
	float coefdiv=64;

	K0=coefdiv;
	DMAX=0;
	u=Array[n1]->Pos - Array[n2]->Pos;	

	d1=Array[n1]->Radius*0.00333f;
	d2=Array[n2]->Radius*0.00333f;

	if (d1<d2) distanceminimale=d1;
	else distanceminimale=d2;
	
	if ((Array[n1]->Type==TYPE_SPHERETREE)&&(Array[n2]->Type==TYPE_SPHERETREE))
	{
		// ...
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_SPHERE))
	{
		ss=u.Norme();
		r=Array[n1]->Radius + Array[n2]->Radius;
		if (ss<0.99f*r)
		{
			u.Normalise();
			Array[n2]->Pos-=u*(r-ss)/(coefdiv*4);
			Array[n1]->Pos+=u*(r-ss)/(coefdiv*4);
		}
	}

	if ((Array[n1]->Type==TYPE_CYLINDER)&&(Array[n2]->Type==TYPE_CYLINDER))
	{
		if (IsCylinder1CollidingCylinder2(n1,n2,0,0))
		{
			ss=u.Norme();
			r=Array[n1]->Radius + Array[n2]->Radius;
			if (ss<r)
			{
				u.Normalise();
				Array[n2]->Pos-=(r-ss)*u/K0;
				Array[n1]->Pos+=(r-ss)*u/K0;
			}
		}
	}


	if ((Array[n1]->Type==TYPE_CYLINDER)&&(Array[n2]->Type==TYPE_POB))
	{
		res=IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe1=(Axe1||u)*Axe1;
			Axe2=(Axe2||u)*Axe2;
			Axe1.Normalise();
			Axe2.Normalise();
			Array[n2]->Pos-=Axe1*DMAX/K0;
			Array[n1]->Pos+=Axe2*DMAX/K0;
		}
	}

	if ((Array[n2]->Type==TYPE_CYLINDER)&&(Array[n1]->Type==TYPE_POB))
	{
		res=IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe1=(Axe1||u)*Axe1;
			Axe2=(Axe2||u)*Axe2;
			Axe1.Normalise();
			Axe2.Normalise();
			Array[n2]->Pos-=Axe1*DMAX/K0;
			Array[n1]->Pos+=Axe2*DMAX/K0;
		}
	}



	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_CUBE))
	{
		M1=Array[n1]->MOActual;
        M2=Array[n2]->MOActual;
		
		A1=Array[n1]->Lx*(Array[n1]->Ux*M1);
		A2=Array[n1]->Ly*(Array[n1]->Uy*M1);
		A3=Array[n1]->Lz*(Array[n1]->Uz*M1);

		B1=Array[n2]->Lx*(Array[n2]->Ux*M2);
		B2=Array[n2]->Ly*(Array[n2]->Uy*M2);
		B3=Array[n2]->Lz*(Array[n2]->Uz*M2);		

		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;

		Ls[3]=B1;
		Ls[4]=B2;
		Ls[5]=B3;

		for (n=0;n<6;n++) Ls[n].Normalise();


		dmax=u.Norme();
		n=0;
		res=1;
		while ((res==1)&&(n<6))
		{
			L=Ls[n];
			rA1=(A1||L);
			rA2=(A2||L);
			rA3=(A3||L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

			rB1=(B1||L);
			rB2=(B2||L);
			rB3=(B3||L);
			rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

			DL=(u||L);
			DL=f_abs(DL);

			ss=(rA+rB)-DL;

			if (ss<0) res=0;
			else
			{
				if (dmax>ss) dmax=ss;
				n++;
			}

			
		}

		if (res==1)
		{
			u.Normalise();
			Array[n2]->Pos-=u*dmax/coefdiv;
			Array[n1]->Pos+=u*dmax/coefdiv;
		}

	}



	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_SPHERE))
	{
        M1=Array[n1]->MOActual;
			
		A1=Array[n1]->Lx*(Array[n1]->Ux*M1);
		A2=Array[n1]->Ly*(Array[n1]->Uy*M1);
		A3=Array[n1]->Lz*(Array[n1]->Uz*M1);
		
		rB=Array[n2]->Radius;

		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;

		for (n=0;n<3;n++) Ls[n].Normalise();

		dmax=u.Norme();
		n=0;
		res=1;
		while ((res==1)&&(n<3))
		{
			L=Ls[n];
			rA1=(A1||L);
			rA2=(A2||L);
			rA3=(A3||L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);

			DL=(u||L);
			DL=f_abs(DL);

			ss=(rA+rB)-DL;

			if (ss<0) res=0;
			else
			{
				n++;
				if (dmax>ss) dmax=ss;
			}
		}

		if (res==1)
		{

			u.Normalise();
			Array[n2]->Pos-=u*dmax/(coefdiv*2);
			Array[n1]->Pos+=u*dmax/(coefdiv*2);
		}

	}


	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_CUBE))
	{
        M2=Array[n2]->MOActual;
		
		B1=Array[n2]->Lx*(Array[n2]->Ux*M2);
		B2=Array[n2]->Ly*(Array[n2]->Uy*M2);
		B3=Array[n2]->Lz*(Array[n2]->Uz*M2);
		
		Ls[0]=B1;
		Ls[1]=B2;
		Ls[2]=B3;

		for (n=0;n<3;n++) Ls[n].Normalise();

		rA=Array[n1]->Radius;

		
		dmax=u.Norme();
		n=0;
		res=1;
		while ((res==1)&&(n<3))
		{
			L=Ls[n];

			rB1=(B1||L);
			rB2=(B2||L);
			rB3=(B3||L);
			rB=f_abs(rB1)+f_abs(rB2)+f_abs(rB3);

			DL=(u||L);
			DL=f_abs(DL);

			ss=(rA+rB)-DL;

			if (ss<0) res=0;
			else
			{
				n++;
				if (dmax>ss) dmax=ss;
			}

			
		}

		if (res==1)
		{
			u.Normalise();
			Array[n2]->Pos-=u*dmax/(coefdiv*2);
			Array[n1]->Pos+=u*dmax/(coefdiv*2);
		}

	}
	// pob

	DMAX=0;

	Axe.Init(0,0,0);
	Axe1.Init(0,0,0);
	Axe2.Init(0,0,0);

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_SPHERE))
	{
//		COEF_ENVELOPPE=0.015f;
		res=IsPob1CollidingSphere2(n1,n2,Array[n1]->pob,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe=(Axe||u)*Axe;
			Axe.Normalise();
			Array[n2]->Pos-=u*DMAX/(coefdiv*4);
			Array[n1]->Pos+=Axe*DMAX/(coefdiv*4);
		}
//		COEF_ENVELOPPE=0.0f;
	}

	if ((Array[n1]->Type==TYPE_SPHERE)&&(Array[n2]->Type==TYPE_POB))
	{
//		COEF_ENVELOPPE=0.015f;
		res=IsSphere1CollidingPob2(n1,n2,Array[n2]->pob,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe=(Axe||u)*Axe;
			Axe.Normalise();
			Array[n2]->Pos-=Axe*DMAX/(coefdiv*4);
			Array[n1]->Pos+=u*DMAX/(coefdiv*4);


		}
//		COEF_ENVELOPPE=0.0f;
	}


	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_CUBE))
	{
		res=IsPob1CollidingCube2(n1,n2,Array[n1]->pob,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe=(Axe||u)*Axe;
			Array[n2]->Pos-=u*DMAX/coefdiv;
			Array[n1]->Pos+=u*DMAX/coefdiv;
		}


	}

	if ((Array[n1]->Type==TYPE_CUBE)&&(Array[n2]->Type==TYPE_POB))
	{
		res=IsCube1CollidingPob2(n1,n2,Array[n2]->pob,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();

			Axe=(Axe||u)*Axe;

			Array[n2]->Pos-=u*DMAX/coefdiv;
			Array[n1]->Pos+=u*DMAX/coefdiv;
		}

	}


	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_POB))
	{
		res=IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe1=(Axe1||u)*Axe1;
			Axe2=(Axe2||u)*Axe2;
			Axe1.Normalise();
			Axe2.Normalise();
			if (Array[n2]->statique==false) Array[n2]->Pos-=Axe1*DMAX/K0;
			if (Array[n1]->statique==false) Array[n1]->Pos+=Axe2*DMAX/K0;
		}
	}

	if ((Array[n1]->Type==TYPE_OBJECT)&&(Array[n2]->Type==TYPE_OBJECT))
	{
		res=IsObject1CollidingObject2(n1,n2,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe.Normalise();
			Axe=(Axe||u)*Axe;
			Axe.Normalise();
			if (Array[n2]->statique==false) Array[n2]->Pos-=4*Axe*DMAX/K0;
			if (Array[n1]->statique==false) Array[n1]->Pos+=4*Axe*DMAX/K0;
		}
	}

	if ((Array[n1]->Type==TYPE_OBJECT)&&(Array[n2]->Type==TYPE_POB))
	{
		res=IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe1=(Axe1||u)*Axe1;
			Axe2=(Axe2||u)*Axe2;
			Axe1.Normalise();
			Axe2.Normalise();
			if (Array[n2]->statique==false) Array[n2]->Pos-=Axe1*DMAX/K0;
			if (Array[n1]->statique==false) Array[n1]->Pos+=Axe2*DMAX/K0;
		}
	}

	if ((Array[n1]->Type==TYPE_POB)&&(Array[n2]->Type==TYPE_OBJECT))
	{
		res=IsPob1CollidingPob2(n1,n2,Array[n1]->pob,Array[n2]->pob,0,0,0);
		if ((res==1)&&(DMAX>distanceminimale))
		{
			DMAX-=distanceminimale;
			u.Normalise();
			Axe1=(Axe1||u)*Axe1;
			Axe2=(Axe2||u)*Axe2;
			Axe1.Normalise();
			Axe2.Normalise();
			if (Array[n2]->statique==false) Array[n2]->Pos-=Axe1*DMAX/K0;
			if (Array[n1]->statique==false) Array[n1]->Pos+=Axe2*DMAX/K0;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetTimeInterval(float dt)
{
	DT=dt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::IsPOBClosingVelocityTooHigh(int nObj,CPOB * pob,float dt,CVector &V,CVector &W)
{
	float vitesse=dt*(VECTORNORM(V));
	if (vitesse>pob->Lx) return true;
	else
	if (vitesse>pob->Ly) return true;
	else
	if (vitesse>pob->Lz) return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::IsBallVelocityTooHigh(CVector &P,float R,float dt,CVector &V,CVector &W)
{
	float vitesse=VECTORNORM(V);

	if (dt*vitesse>R) return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::IsSphereTreeVelocityTooHigh(int nObj,int n,float dt,CVector &V,CVector &W)
{
	Ball * B;
	bool res;
	CVector P;
	float R;
	res=0;

	P.x=Array[nObj]->STREE->Spheres[n].x;
	P.y=Array[nObj]->STREE->Spheres[n].y;
	P.z=Array[nObj]->STREE->Spheres[n].z;
	R=Array[nObj]->STREE->Spheres[n].r;

	if (Array[nObj]->STREE->Spheres[n].tag<2)
	{
		res=IsBallVelocityTooHigh(P,R,dt,V,W);
	}
	else
	{
		B=&(Array[nObj]->STREE->Spheres[n]);
		res=false;
		
		if (B->b0!=-1) res|=IsSphereTreeVelocityTooHigh(nObj,Array[nObj]->STREE->Spheres[n].b0,dt,V,W);
		if (B->b1!=-1) res|=IsSphereTreeVelocityTooHigh(nObj,Array[nObj]->STREE->Spheres[n].b1,dt,V,W);
		if (B->b2!=-1) res|=IsSphereTreeVelocityTooHigh(nObj,Array[nObj]->STREE->Spheres[n].b2,dt,V,W);
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::IsPOBVelocityTooHigh(int nObj,CPOB * pob,float dt,CVector &V,CVector &W)
{
	bool res=false;

	if (pob->ID==POB_MULTINODECLOSING)
	{
		res=IsPOBClosingVelocityTooHigh(nObj,pob,dt,V,W);
		res|=IsPOBVelocityTooHigh(nObj,pob->Next1,dt,V,W);
	}
	else
	if (pob->ID==POB_MULTINODE)
	{
		res=IsPOBVelocityTooHigh(nObj,pob,dt,V,W);
			
	}
	else
	if (pob->ID==POB_CLOSING)
	{
		res=IsPOBClosingVelocityTooHigh(nObj,pob,dt,V,W);
	}
	else
	{
		res=IsPOBVelocityTooHigh(nObj,pob->Next1,dt,V,W);
		res|=IsPOBVelocityTooHigh(nObj,pob->Next2,dt,V,W);
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::IsObjVelocityTooHigh(int nObj,float dt,CVector &V,CVector& W)
{
	return IsBallVelocityTooHigh(Array[nObj]->Pos,Array[nObj]->Radius,dt,V,W);
/*
	if (Array[nObj]->Type==TYPE_SPHERE)
		res=IsBallVelocityTooHigh(Array[nObj]->Pos,Array[nObj]->Radius,dt,V,W);

	if (Array[nObj]->Type==TYPE_CYLINDER)
		res=IsBallVelocityTooHigh(0*Array[nObj]->Pos,Array[nObj]->Radius,dt,V,W);

	if (Array[nObj]->Type==TYPE_SPHERETREE)
		res=IsSphereTreeVelocityTooHigh(nObj,Array[nObj]->STREE->nB-1,dt,V,W);
	
	if (Array[nObj]->Type==TYPE_POB)
		res=IsPOBVelocityTooHigh(nObj,Array[nObj]->pob,dt,V,W);

	if (Array[nObj]->Type==TYPE_OBJECT)
		res=IsPOBClosingVelocityTooHigh(nObj,Array[nObj]->pob,dt,V,W);
/**/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::AddJoint(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2)
{
	CPhysicJoint *joint;

	joint=this;
	while (joint->NextJoint) joint=joint->NextJoint;

	op1->Linked=true;
	op2->Linked=true;

	op1->Links[op1->nLinks]=r1;
	op1->Links[op1->nLinks].Normalise();
	op1->nLinks++;
	op2->Links[op2->nLinks]=r2;
	op2->Links[op2->nLinks].Normalise();
	op2->nLinks++;


	joint->NextJoint=new CPhysicJoint;
	joint->NextJoint->type=MOBIL_JOINT;
	joint->NextJoint->Object1=op1;
	joint->NextJoint->Object2=op2;
	joint->NextJoint->r1=r1;
	joint->NextJoint->r2=r2;
	joint->NextJoint->NextJoint=0;

	return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::AddJointSpring(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,float K)
{
    CPhysicJoint *joint;
    
    joint=this;
    while (joint->NextJoint) joint=joint->NextJoint;
    
    op1->Linked=true;
    op2->Linked=true;
    
    op1->Links[op1->nLinks]=r1;
    op1->Links[op1->nLinks].Normalise();
    op1->nLinks++;
    op2->Links[op2->nLinks]=r2;
    op2->Links[op2->nLinks].Normalise();
    op2->nLinks++;
    
    joint->NextJoint=new CPhysicJoint;
    joint->NextJoint->type=SPRING_JOINT;
    joint->NextJoint->Object1=op1;
    joint->NextJoint->Object2=op2;
    joint->NextJoint->r1=r1;
    joint->NextJoint->r2=r2;
    joint->NextJoint->L1=r1.Norme();
    joint->NextJoint->L2=r2.Norme();
    CVector u=op1->Pos-op2->Pos;
    joint->NextJoint->L=u.Norme();
    joint->NextJoint->dL=0;
    joint->NextJoint->raideur=K;
    joint->NextJoint->NextJoint=0;
    
    return joint->NextJoint;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::AddJointBall(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2)
{
	CPhysicJoint *joint;

	joint=this;
	while (joint->NextJoint) joint=joint->NextJoint;

	op1->Linked=true;
	op2->Linked=true;

	op1->Links[op1->nLinks]=r1;
	op1->Links[op1->nLinks].Normalise();
	op1->nLinks++;
	op2->Links[op2->nLinks]=r2;
	op2->Links[op2->nLinks].Normalise();
	op2->nLinks++;

	joint->NextJoint=new CPhysicJoint;
	joint->NextJoint->type=BALL_JOINT;
	joint->NextJoint->Object1=op1;
	joint->NextJoint->Object2=op2;
	joint->NextJoint->r1=r1;
	joint->NextJoint->r2=r2;
	joint->NextJoint->NextJoint=0;

	return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::AddJointPivot(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,CVector axe1,CVector axe2)
{
	CPhysicJoint *joint;

	joint=this;
	while (joint->NextJoint) joint=joint->NextJoint;

	op1->Linked=true;
	op2->Linked=true;
	op1->Links[op1->nLinks]=r1;
	op1->Links[op1->nLinks].Normalise();
	op1->nLinks++;
	op2->Links[op2->nLinks]=r2;
	op2->Links[op2->nLinks].Normalise();
	op2->nLinks++;

	joint->NextJoint=new CPhysicJoint;
	joint->NextJoint->type=PIVOT_JOINT;
	joint->NextJoint->Object1=op1;
	joint->NextJoint->Object2=op2;
    joint->NextJoint->axis1=axe1;
    joint->NextJoint->axis2=axe2;
	joint->NextJoint->r1=r1;
	joint->NextJoint->r2=r2;
	joint->NextJoint->NextJoint=0;

	return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::Fixe(CVector pos,CPhysicObject *op,CVector r)
{
    CPhysicJoint *joint;
    
    joint=this;
    while (joint->NextJoint) joint=joint->NextJoint;
    
    op->Linked=true;
    
    joint->NextJoint=new CPhysicJoint;
    joint->NextJoint->type=FIXED_JOINT;
    joint->NextJoint->Object1=op;
    
    op->Links[op->nLinks]=r;
    op->Links[op->nLinks].Normalise();
    op->nLinks++;
    
    joint->NextJoint->Object2=&FIXED;
    joint->NextJoint->r1=r;
    joint->NextJoint->ptfixe=pos;
    joint->NextJoint->NextJoint=0;
    
    return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::FixePivot(CVector pos,CPhysicObject *op,CVector r,CVector axe)
{
    CPhysicJoint *joint;
    
    joint=this;
    while (joint->NextJoint) joint=joint->NextJoint;
    
    op->Linked=true;
    
    joint->NextJoint=new CPhysicJoint;
    joint->NextJoint->type=FIXED_PIVOT;
    joint->NextJoint->Object1=op;
    
    op->Links[op->nLinks]=r;
    op->Links[op->nLinks].Normalise();
    op->nLinks++;
    joint->NextJoint->Object2=&FIXED;
    joint->NextJoint->Object2->Fixe=true;
    joint->NextJoint->r1=r;
    joint->NextJoint->axis1=axe;
    joint->NextJoint->ptfixe=pos;
    joint->NextJoint->NextJoint=0;
    
    return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPhysicJoint *CPhysicJoint::FixeSpring(CVector pos,CPhysicObject *op,CVector r,float K)
{
    CPhysicJoint *joint;
    
    joint=this;
    while (joint->NextJoint) joint=joint->NextJoint;
    
    op->Linked=true;
    
    joint->NextJoint=new CPhysicJoint;
    joint->NextJoint->type=FIXED_SPRING;
    joint->NextJoint->Object1=op;
    
    op->Links[op->nLinks]=r;
    op->Links[op->nLinks].Normalise();
    op->nLinks++;
    
    joint->NextJoint->Object2=&FIXED;
    joint->NextJoint->r1=r;

    CVector u=op->Pos-pos;
    joint->L1=u.Norme();
    joint->L=u.Norme();
    joint->dL=0;
    joint->raideur=K;

    joint->NextJoint->ptfixe=pos;
    joint->NextJoint->NextJoint=0;
    
    return joint->NextJoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysicJoint::Nbre()
{
	CPhysicJoint *joint;
	int n=0;
	joint=this;
	while (joint)
	{
		joint->num=n;
		n++;
		joint=joint->NextJoint;
	}

	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysicJoint::NbreOp()
{
	int n,res;
	CPhysicJoint *joint;

	nObjs=0;

	joint=this;
	while (joint)
	{
		n=0;
		res=-1;
		while ((n<nObjs)&&(res==-1))
		{
			if (joint->Object1==Objs[n]) res=n;
			else n++;
		}

		if (res<0)
		{
			Objs[nObjs]=joint->Object1;
			joint->Object1->OrderInJacobian=nObjs;
			nObjs++;
		}
		else joint->Object1->OrderInJacobian=res;

		if ((joint->type!=FIXED_JOINT)&&(joint->type!=FIXED_SPRING)&&(joint->type!=FIXED_PIVOT))
		{
			n=0;
			res=-1;
			while ((n<nObjs)&&(res==-1))
			{
				if (joint->Object2==Objs[n]) res=n;
				else n++;
			}

			if (res<0)
			{
				Objs[nObjs]=joint->Object2;
				joint->Object2->OrderInJacobian=nObjs;
				nObjs++;
			}
			else joint->Object2->OrderInJacobian=res;
		}
		else
		{
			Objs[nObjs]=joint->Object2;
			joint->Object2->OrderInJacobian=nObjs;
			nObjs++;
		}

		joint=joint->NextJoint;
	}

	return nObjs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// s'applique aux basejoints
CMatrix_MxN * CPhysicJoint::BigJacobian()
{
	return BigJacobian(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix_MxN * CPhysicJoint::BigJacobian(int tag)	
{
	int n,nb,nbop;
	CMatrix_MxN *Big;
	CMatrix_MxN Js1,Js2;
	CPhysicJoint *joint;
	CVector r,r1,r2,x1,x2;
	CVector t,t1,t2;
	CVector u,u1,u2;
	int adr,len;
	CMatrix M;

	nb=Nbre();
	nbop=NbreOp();

	if (jacobians_indices==NULL)
	{
		jacobians_indices=new int[nb];
		jacobians_len=new int[nb];
	}

	adr=0;
	n=0;
	joint=this;
	while (joint)
	{
		jacobians_indices[n]=adr;

        if (joint->type==PIVOT_JOINT) len=PIVOT_JOINT_DIM;
        if (joint->type==SPRING_JOINT) len=SPRING_JOINT_DIM;
		if (joint->type==MOBIL_JOINT) len=MOBIL_JOINT_DIM;
		if (joint->type==BALL_JOINT) len=BALL_JOINT_DIM;
        
		if (joint->type==FIXED_JOINT) len=FIXED_JOINT_DIM;
        if (joint->type==FIXED_SPRING) len=SPRING_JOINT_DIM;
        if (joint->type==FIXED_PIVOT) len=FIXED_PIVOT_DIM;

		jacobians_len[n]=len;
		adr+=len;
		n++;
		joint=joint->NextJoint;
	}

	total_len=adr;

	NbreOP=nbop;
	NbreJoints=nb;

	// les matrices des OP doivent �tre initialis�es.
	Big=new CMatrix_MxN;
	Big->Init(total_len,nbop*6);
	
	joint=this;
	while (joint)
	{
		joint->Object1->approx=0;
		joint->Object2->approx=0;

		if (joint->type==PIVOT_JOINT)
		{			
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            x1=joint->axis1*joint->Object1->MOActual;
            x2=joint->axis2*joint->Object2->MOActual;
			Js1.JacobianPivot(r1,x1,1);
			Js2.JacobianPivot(r2,x2,-1);
			Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
			Big->Patch(jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
			Js1.Free();
			Js2.Free();
		}

        if (joint->type==MOBIL_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            Js1.Jacobian(r1,1);
            Js2.Jacobian(r2,-1);
            Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            Big->Patch(jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
            Js1.Free();
            Js2.Free();
        }
        
        if (joint->type==SPRING_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            Js1.JacobianSpring(r1,1,joint->raideur);
            Js2.JacobianSpring(r2,-1,joint->raideur);
            Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            Big->Patch(jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
            Js1.Free();
            Js2.Free();
        }
        
		if (joint->type==BALL_JOINT)
		{
			r1=joint->r1*joint->Object1->MOActual;
			r2=joint->r2*joint->Object2->MOActual;
			Js1.Jacobian3(r1,1);
			Js2.Jacobian3(r2,-1);
			Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
			Big->Patch(jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
			Js1.Free();
			Js2.Free();
		}

        if (joint->type==FIXED_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            Js1.Jacobian(r1,1);
            Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            Js1.Free();
        }
        
        if (joint->type==FIXED_SPRING)
        {
            r1=joint->r1*joint->Object1->MOActual;
            Js1.JacobianSpring(r1,1,joint->raideur);
            Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            Js1.Free();
        }
        
        if (joint->type==FIXED_PIVOT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            x1=joint->axis1*joint->Object1->MOActual;
            Js1.JacobianPivotFixe(r1,x1,1);
            Big->Patch(jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            Js1.Free();
        }
        
		joint=joint->NextJoint;
	}

	return Big;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicJoint::del(CPhysicObject *op)
{
	int n,k;

	n=0;
	while (n<nObjs)
	{
		if (Objs[n]==op)
		{
			for (k=n;k<nObjs;k++) Objs[k]=Objs[k+1];
			nObjs--;
		}
		else n++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysicJoint::setupJacobian()
{
	int adr=0;
	int n=0;
	int len;
	int nb=Nbre();
	int nbop=NbreOp();

	if (jacobians_indices)
	{
		delete [] jacobians_indices;
		delete [] jacobians_len;
	}
	jacobians_indices=new int[nb];
	jacobians_len=new int[nb];

	CPhysicJoint *joint=this;
	while (joint)
	{
		jacobians_indices[n]=adr;

        if (joint->type==PIVOT_JOINT) len=PIVOT_JOINT_DIM;
        if (joint->type==SPRING_JOINT) len=SPRING_JOINT_DIM;
        if (joint->type==MOBIL_JOINT) len=MOBIL_JOINT_DIM;
        if (joint->type==BALL_JOINT) len=BALL_JOINT_DIM;
        
        if (joint->type==FIXED_JOINT) len=FIXED_JOINT_DIM;
        if (joint->type==FIXED_SPRING) len=SPRING_JOINT_DIM;
        if (joint->type==FIXED_PIVOT) len=FIXED_PIVOT_DIM;

		jacobians_len[n]=len;
		adr+=len;
		n++;
		joint=joint->NextJoint;
	}

	total_len=adr;

	NbreOP=nbop;
	NbreJoints=nb;

	return total_len;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PatchMatandtMat(CMatrix_MxN *Big,int ofs,int l,int c,CMatrix_MxN *M)
{
	int n1,n2;
	int adr=0;
	int pos;
	int pos0;

	pos0=Big->n*(ofs+l) + c;
	for (n2=0;n2<M->m;n2++)
	{
		pos=pos0;
		for (n1=0;n1<M->n;n1++) Big->a[pos++]=M->a[adr++];
		pos0+=Big->n;
	}

	adr=0;
	pos0=c*Big->n + (l+ofs);
	for (n2=0;n2<M->m;n2++)
	{
		pos=pos0;
		for (n1=0;n1<M->n;n1++) { Big->a[pos]=-M->a[adr++]; pos+=Big->n; }
		pos0++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicJoint::BigJacobianPatch(CMatrix_MxN *Big,int ofs,int tag)	
{
	CMatrix_MxN Js1,Js2;
	CPhysicJoint *joint;
	CVector r,r1,r2,x1,x2;
	CVector t,t1,t2;

	joint=this;
	while (joint)
	{
		joint->Object1->approx=0;
		joint->Object2->approx=0;

		if (joint->type==PIVOT_JOINT)
		{			
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            x1=joint->axis1*joint->Object1->MOActual;
            x2=joint->axis2*joint->Object2->MOActual;
			Js1.JacobianPivot(r1,x1,1);
			Js2.JacobianPivot(r2,x2,-1);

			PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
			PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);

			Js1.Free();
			Js2.Free();
		}

        if (joint->type==MOBIL_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            Js1.Jacobian(r1,1);
            Js2.Jacobian(r2,-1);
            
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
            
            Js1.Free();
            Js2.Free();
        }
        
        if (joint->type==SPRING_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            r2=joint->r2*joint->Object2->MOActual;
            Js1.JacobianSpring(r1,1,joint->raideur);
            Js2.JacobianSpring(r2,-1,joint->raideur);
            
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);
            
            Js1.Free();
            Js2.Free();
        }
        
		if (joint->type==BALL_JOINT)
		{
			r1=joint->r1*joint->Object1->MOActual;
			r2=joint->r2*joint->Object2->MOActual;
			Js1.Jacobian3(r1,1);
			Js2.Jacobian3(r2,-1);

			PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
			PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object2->OrderInJacobian*6,&Js2);

			Js1.Free();
			Js2.Free();
		}

        if (joint->type==FIXED_JOINT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            Js1.Jacobian(r1,1);
            
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            
            Js1.Free();
        }
        
        if (joint->type==FIXED_SPRING)
        {
            r1=joint->r1*joint->Object1->MOActual;
            Js1.JacobianSpring(r1,1,joint->raideur);
            
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            
            Js1.Free();
        }
        
        if (joint->type==FIXED_PIVOT)
        {
            r1=joint->r1*joint->Object1->MOActual;
            x1=joint->axis1*joint->Object1->MOActual;
            Js1.JacobianPivotFixe(r1,x1,1);
            
            PatchMatandtMat(Big,ofs,jacobians_indices[joint->num],joint->Object1->OrderInJacobian*6,&Js1);
            
            Js1.Free();
        }
        
		joint=joint->NextJoint;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::enableEvents()
{
	GetEventList=true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::disableEvents()
{
	GetEventList=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setStaticDeterminationMethod(int method)
{
	static_method=method;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setAverageStabilization()
{
	POST_STABILIZATION_METHOD=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setNormalOldCodeStabilization()
{
	bancal_algo=1;
	POST_STABILIZATION_METHOD=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setNormalStabilization()
{
	bancal_algo=0;
	POST_STABILIZATION_METHOD=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::setLCPStabilization()
{
	POST_STABILIZATION_METHOD=2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::SetEulerOrientation(float rx,float ry,float rz)
{
	CVector w;
	CQuaternion q1,q2,q3;
	CVector u1,u2,u3;

	u1.Init(1,0,0);
	u2.Init(0,1,0);
	u3.Init(0,0,1);

	q1.AxisAngle(u1,-PI*rx/180);
	q2.AxisAngle(u2,-PI*ry/180);
	q3.AxisAngle(u3,-PI*rz/180);

	Q.Id();
	Q=Q*q3;
	Q=Q*q2;
	Q=Q*q1;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::IsACylinder()
{
	Type=TYPE_CYLINDER;
	OBJECT=NULL;
	enh=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::InitCylinder(float r,float l,CVector u)
{
	U=u;
	U.Normalise();
	L=l;
	Radius=r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::FreePob(CPOB * pob)
{
	if (pob->Next0) FreePob(pob->Next0);
	if (pob->Next1) FreePob(pob->Next1);
	if (pob->Next2) FreePob(pob->Next2);

	delete pob;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysicObject::Free()
{

	if (STREE) {
	
		STREE->Free();
		delete STREE;
	}

	if (pob) {

		FreePob(pob);
	}

	STREE=NULL;
	pob=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::addTemporary(CPhysicObject * op,bool link)
{
	NewObject(op,link);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::addTemporary(CPhysicObject * op)
{
	NewObject(op);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::delTemporary(CPhysicObject * op)
{
	int res,n;

	if (nArray>0)
	{
		n=0;
		res=-1;
		while ((n<nArray)&&(res==-1))
		{
			if (Array[n]==op) res=n;
			n++;
		}

		if (res>=0)
		{
			Array[res]=Array[nArray-1];
			nArray--;
			for (n=0;n<nArray;n++) { if (Array[n]->support==Array[res]) Array[n]->support=NULL;	}
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPhysic::isTemporary(CPhysicObject * op)
{
	int res,n;
	bool b=false;
	n=0;
	res=-1;
	while ((n<nArray)&&(res==-1))
	{
		if (Array[n]==op) { res=n; b=true; }
		n++;
	}

	return b;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateScene()
{
	CalculateSceneNormal();
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSphereTree::Free()
{
	nB=0;
	if (Spheres) free(Spheres);
	Spheres=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSphereTree::Load(const char * str,float scale)
{
	char ss[16];
	int N;
	int n;
	FILE * f;

	f=fopen(str,"rb");
	if(f==NULL) {
	  return;
	}
	fread(ss,8,1,f);
	fread(&nB,4,1,f);
	fread(&N,4,1,f);

	Spheres=(Ball *) malloc(nB*sizeof(Ball));
	fread(Spheres,nB*sizeof(Ball),1,f);
	fclose(f);

	for (n=0;n<nB;n++)
	{
		Spheres[n].x*=scale;
		Spheres[n].y*=scale;
		Spheres[n].z*=scale;
		Spheres[n].r*=scale;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSphereTree::Save(const char * str)
{
	const char * DEF="BALLSv1";
	FILE * f;

	f=fopen(str,"wb");
	if(f==NULL) {
	  return;
	}
	fwrite(DEF,8,1,f);
	fwrite(&nB,4,1,f);
	fwrite(&Nmax,4,1,f);
	fwrite(Spheres,nB*sizeof(Ball),1,f);
	fclose(f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSphereTree::Recurs(int posnB,int N)
{
	int n,nn,c;
	float somme;
	float x,y,z,r,rmax;
	int b0,b1;
	int pos;
	int ni;
	int res=-1;
	CVector A,B,u;

	c=0;
	for (n=posnB;n<nB;n++)
		if (Spheres[n].tag==0) 
		{
			
			if (Spheres[n].r<=0) Spheres[n].tag=-1;
			else c++;
		}

	pos=nB;

	if (c<=3)
	{
		res=N+1;
		if (c==1)
		{
			Spheres[posnB].tag=-1;
		}
		else
		{
			somme=0;
			x=y=z=0;

			if (c==2)
			{
				b0=posnB;
				b1=posnB+1;

				u.x=Spheres[b1].x-Spheres[b0].x;
				u.y=Spheres[b1].y-Spheres[b0].y;
				u.z=Spheres[b1].z-Spheres[b0].z;
				u.Normalise();

				A.x=Spheres[b0].x;
				A.y=Spheres[b0].y;
				A.z=Spheres[b0].z;

				A=A-Spheres[b0].r*u;

				B.x=Spheres[b1].x;
				B.y=Spheres[b1].y;
				B.z=Spheres[b1].z;

				B=B+Spheres[b1].r*u;

				u=(A+B)/2;

				x=u.x;
				y=u.y;
				z=u.z;

			}

			if (c==3)
			{
				for (n=posnB;n<nB;n++)
					if (Spheres[n].tag==0)
					{
						x+=Spheres[n].x*Spheres[n].r;
						y+=Spheres[n].y*Spheres[n].r;
						z+=Spheres[n].z*Spheres[n].r;
						somme+=Spheres[n].r;
					}

				x=x/somme;
				y=y/somme;
				z=z/somme;
			}
				
			rmax=0;
			for (n=posnB;n<nB;n++)
				if (Spheres[n].tag==0)
				{
					r=(float) sqrtf((Spheres[n].x-x)*(Spheres[n].x-x) + (Spheres[n].y-y)*(Spheres[n].y-y) + (Spheres[n].z-z)*(Spheres[n].z-z));
					r+=Spheres[n].r;
					if (r>rmax) rmax=r;
				}

			Spheres[nB].x=x;
			Spheres[nB].y=y;
			Spheres[nB].z=z;
			Spheres[nB].r=rmax;
			Spheres[nB].b0=posnB;
			Spheres[nB].b1=posnB+1;
			if (c==2)
				Spheres[nB].b2=-1;
			else
				Spheres[nB].b2=posnB+2;

			Spheres[nB].tag=N+1;
			nB++;
			for (n=posnB;n<nB;n++)
				if (Spheres[n].tag==0)
					Spheres[n].tag=N;
		
		}
	}
	else
	{

		n=posnB;
		while (n<pos)
		{

			if (Spheres[n].tag==0)
			{
				rmax=10000;
				b0=n;

				ni=-1;
				
				b1=-1;

				x=Spheres[b0].x;
				y=Spheres[b0].y;
				z=Spheres[b0].z;

				for (nn=posnB;nn<pos;nn++)
				if (n!=nn)
					if (Spheres[nn].tag==0)
					{
						r=(float) sqrtf((Spheres[nn].x-x)*(Spheres[nn].x-x) + (Spheres[nn].y-y)*(Spheres[nn].y-y) + (Spheres[nn].z-z)*(Spheres[nn].z-z));
						if ((r<rmax)&&(r<(Spheres[nn].r+Spheres[n].r)*2))
						{
							rmax=r;
							b1=nn;
							ni=nB;
						}
					}

				if (b1==-1)
				{

					for (nn=pos;nn<nB;nn++)
					{
						if (Spheres[nn].b2==-1)
						{
							r=(float) sqrtf((Spheres[nn].x-x)*(Spheres[nn].x-x) + (Spheres[nn].y-y)*(Spheres[nn].y-y) + (Spheres[nn].z-z)*(Spheres[nn].z-z));
							if (r<rmax)
							{
								rmax=r;
								b1=nn;
								ni=nn;
							}
						}
					}
				}

/*
				x=(Spheres[b0].x*Spheres[b0].r + Spheres[b1].x*Spheres[b1].r)/(Spheres[b0].r+Spheres[b1].r);
				y=(Spheres[b0].y*Spheres[b0].r + Spheres[b1].y*Spheres[b1].r)/(Spheres[b0].r+Spheres[b1].r);
				z=(Spheres[b0].z*Spheres[b0].r + Spheres[b1].z*Spheres[b1].r)/(Spheres[b0].r+Spheres[b1].r);
*/


				u.x=Spheres[b1].x-Spheres[b0].x;
				u.y=Spheres[b1].y-Spheres[b0].y;
				u.z=Spheres[b1].z-Spheres[b0].z;
				u.Normalise();

				A.x=Spheres[b0].x;
				A.y=Spheres[b0].y;
				A.z=Spheres[b0].z;

				A=A-Spheres[b0].r*u;

				B.x=Spheres[b1].x;
				B.y=Spheres[b1].y;
				B.z=Spheres[b1].z;

				B=B+Spheres[b1].r*u;

				u=(A+B)/2;

				x=u.x;
				y=u.y;
				z=u.z;


				rmax=0;
				r=(float) sqrtf((Spheres[b0].x-x)*(Spheres[b0].x-x) + (Spheres[b0].y-y)*(Spheres[b0].y-y) + (Spheres[b0].z-z)*(Spheres[b0].z-z));
				r+=Spheres[b0].r;

				if (r>rmax) rmax=r;

				r=(float) sqrtf((Spheres[b1].x-x)*(Spheres[b1].x-x) + (Spheres[b1].y-y)*(Spheres[b1].y-y) + (Spheres[b1].z-z)*(Spheres[b1].z-z));
				r+=Spheres[b1].r;
				if (r>rmax) rmax=r;

				if (ni==nB)
				{

					Spheres[ni].b0=b0;
					Spheres[ni].b1=b1;
					Spheres[ni].b2=-1;
					Spheres[ni].x=x;
					Spheres[ni].y=y;
					Spheres[ni].z=z;
					Spheres[ni].r=rmax;
					Spheres[ni].tag=0;
					nB++;

					Spheres[b0].tag=N;
					Spheres[b1].tag=N;
					
				}
				else
				{
					Spheres[ni].b2=b0;
					Spheres[ni].x=x;
					Spheres[ni].y=y;
					Spheres[ni].z=z;
					Spheres[ni].r=rmax;
					Spheres[ni].tag=0;

					Spheres[b0].tag=N;

				}
			}
			
			n++;
		}

		res=Recurs(pos,N+1);
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARTICLES SYSTEM COLLISIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEFINES_API_CODE_PHYSIC_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ParticlesInit(int nb)
{
	ParticlesArray=new CParticle[nb];
	nbParticlesArray=nb;
}

void CPhysic::ParticlesFree()
{
	if (nbParticlesArray>0)
	{
		delete [] ParticlesArray;
		nbParticlesArray=0;
	}
}

bool CPhysic::ParticlesAdd(CVector &pos,CVector &vit,float mass,bool replace)
{
	int n;

	for (n=0;n<nbParticlesArray;n++)
	if (!ParticlesArray[n].active)
	{
		ParticlesArray[n].Pos=pos;
			ParticlesArray[n].aPos=pos;
			ParticlesArray[n].Posdt=pos;
		ParticlesArray[n].Vit=vit;
			ParticlesArray[n].aVit=vit;
			ParticlesArray[n].Vit_dt=vit;

		ParticlesArray[n].Mass=mass;
		ParticlesArray[n].Time=0.0f;
        ParticlesArray[n].active=true;
        ParticlesArray[n].iter=iter_particles_local;
		return true;
	}

	if (replace)
	{
		float tm=0.0f;
		int res=-1;
		for (n=0;n<nbParticlesArray;n++)
			if (ParticlesArray[n].Time>tm) { tm=ParticlesArray[n].Time; res=n; }

		if (res>=0)
		{
			n=res;
			ParticlesArray[n].Pos=pos;
				ParticlesArray[n].aPos=pos;
				ParticlesArray[n].Posdt=pos;
			ParticlesArray[n].Vit=vit;
				ParticlesArray[n].aVit=vit;
				ParticlesArray[n].Vit_dt=vit;

			ParticlesArray[n].Mass=mass;
			ParticlesArray[n].Time=0.0f;
			ParticlesArray[n].active=true;
            ParticlesArray[n].iter=iter_particles_local;
			return true;
		}
	}

	return false;
}

bool CPhysic::ParticlesAdd(CVector &pos,CVector &vit,float mass,float visc,bool replace)
{
	int n;

	for (n=0;n<nbParticlesArray;n++)
	if (!ParticlesArray[n].active)
	{
		ParticlesArray[n].Pos=pos;
			ParticlesArray[n].aPos=pos;
			ParticlesArray[n].Posdt=pos;
		ParticlesArray[n].Vit=vit;
			ParticlesArray[n].aVit=vit;
			ParticlesArray[n].Vit_dt=vit;

		ParticlesArray[n].Mass=mass;
		ParticlesArray[n].strength=visc;
		ParticlesArray[n].Time=0.0f;
        ParticlesArray[n].active=true;
        ParticlesArray[n].iter=iter_particles_local;
		return true;
	}

	if (replace)
	{
		float tm=0.0f;
		int res=-1;
		for (n=0;n<nbParticlesArray;n++)
			if (ParticlesArray[n].Time>tm) { tm=ParticlesArray[n].Time; res=n; }

		if (res>=0)
		{
			n=res;
			ParticlesArray[n].Pos=pos;
				ParticlesArray[n].aPos=pos;
				ParticlesArray[n].Posdt=pos;
			ParticlesArray[n].Vit=vit;
				ParticlesArray[n].aVit=vit;
				ParticlesArray[n].Vit_dt=vit;

			ParticlesArray[n].Mass=mass;
			ParticlesArray[n].strength=visc;
			ParticlesArray[n].Time=0.0f;
			ParticlesArray[n].active=true;
            ParticlesArray[n].iter=iter_particles_local;
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionProp(CVector &A,CVector &B,int nObj,int Calculate,int buf)
{
	if (Array[nObj]->Type==TYPE_SPHERE) return ParticleCollisionSphere(A,B,nObj,Calculate,buf);
	if (Array[nObj]->Type==TYPE_POB) return ParticleCollisionPob(A,B,nObj,Array[nObj]->pob,Calculate,buf);
	if (Array[nObj]->Type==TYPE_CYLINDER) return ParticleCollisionCylinder(A,B,nObj,Calculate,buf);
	if (Array[nObj]->Type==TYPE_OBJECT) return ParticleCollisionObject(A,B,nObj,Calculate,buf);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionObject(CVector &A,CVector &B,int nObj,int Calculate,int buf)
{
	if (ParticleCollisionBall(A,B,Array[nObj]->Pos,Array[nObj]->Radius))
	{
		int x,y,z;
		int i,n;
		CVector u,p;
		CObject3D * obj=(CObject3D *) Array[nObj]->OBJECT;
		VECTORSUB(u,B,A);
		float r2=VECTORNORM(u);
		CVector p2=A;

		if (Array[nObj]->SP)
		{
			for (n=0;n<64;n++)
			{
				Array[nObj]->SP->balls[n].tags[buf]=0;
				Array[nObj]->SP->balls[n].tags[buf]=0;
			}

			for (x=0;x<Array[nObj]->SP->nbx;x++)
				for (y=0;y<Array[nObj]->SP->nby;y++)
					for (z=0;z<Array[nObj]->SP->nbz;z++)
					{
						int ofs=x+((y+(z<<2))<<2);
						p=Array[nObj]->SP->balls[ofs].calc;
						float R=Array[nObj]->SP->balls[ofs].r;
						if (ParticleCollisionBall(A,B,p,R)) Array[nObj]->SP->balls[ofs].tags[buf]=1; else Array[nObj]->SP->balls[ofs].tags[buf]=0;
					}
		}

		if (Array[nObj]->SP)
		{			
			for (n=0;n<obj->nFaces;n++)
			{
				obj->Faces[n].tags[buf]=0;
				if ((Array[nObj]->SP->balls[obj->Faces[n].v[0]->temp].tags[buf])||(Array[nObj]->SP->balls[obj->Faces[n].v[1]->temp].tags[buf])||(Array[nObj]->SP->balls[obj->Faces[n].v[2]->temp].tags[buf]))
				{
					VECTORINIT(u,obj->Faces[n].mp0.x,obj->Faces[n].mp0.y,obj->Faces[n].mp1.x);
					VECTORSUB(u,u,p2);
					float f=VECTORNORM(u);
					if (f<obj->Faces[n].coef1+r2) obj->Faces[n].tags[buf]=1;
				}
			}
		}
		else
		{
			for (n=0;n<obj->nFaces;n++)
			{
				obj->Faces[n].tags[buf]=0;
				VECTORINIT(u,obj->Faces[n].mp0.x,obj->Faces[n].mp0.y,obj->Faces[n].mp1.x);
				VECTORSUB(u,u,p2);
				float f=VECTORNORM(u);
				if (f<obj->Faces[n].coef1+r2) obj->Faces[n].tags[buf]=1;
			}
		}

		for (i=0;i<obj->nFaces;i++)
		{
			if (obj->Faces[i].tags[buf]) 
				if (ParticleCollisionFace(A,B,&(obj->Faces[i]),COLLISION_WITH_OBJECT,nObj,Calculate,buf)) return 1;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionPlane(CVector &A,CVector &B,int nPlan,int Calculate,int buf)
{
	int res=0;
	CVector N(Planes[nPlan].a,Planes[nPlan].b,Planes[nPlan].c);
	float d=Planes[nPlan].d;
	CVector I;
	float t;
	float sa=DOT(N,A) + d;
	float sb=DOT(N,B) + d;

	if (sa*sb<0)
	{
		if (Calculate==1)
		{
			t=-sa/(sb-sa);
			VECTORINTERPOL(I,t,B,A);
			ContactStruct * cs=Contacts[buf].InsertLast();
			cs->Normals=N;
			cs->Point=I;
			cs->Types=COLLISION_WITH_PLANE;
			cs->Num=nPlan;
			cs->Distance=0;
		}
		res=1;
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionMesh(CVector &A,CVector &B,int nObj,CCollisionMesh * mesh,int Calculate,int buf)
{
	int res=0;
	CVector u=B-A;
	float R=0.5f+VECTORNORM(u);
	CList <int> * list;

	if (IsPobSphere(A,R,mesh->bounds,mesh->M,mesh->Radius))
	{
#if ((!defined(GLES))&&(!defined(GLES20)))||(defined(GLESFULL))
		list=mesh->quad.GetFaceList(A,R,mesh->M,buf);
#else
		list=mesh->quad.GetFaceList(A,R,mesh->M);
#endif
		int *i=list->GetFirst();
		while (i)
		{
			if (ParticleCollisionFace(A,B,&(mesh->obj->Faces[*i]),COLLISION_WITH_MESH,nObj,Calculate,buf)) res=1;
			i=list->GetNext();
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionCylinder(CVector &A,CVector &B,int n,int Calculate,int buf)
{
	CVector Axe=Array[n]->U*Array[n]->MOActual;
	float L=Array[n]->L;
	float R=Array[n]->Radius;
	CVector OA=A-Array[n]->Pos;
	CVector OB=B-Array[n]->Pos;
	float r=OA.Norme2()-OA.Norme()*(OA||Axe);
	if (r<R)
	{
		CVector a,b;
		a.Base(Axe,0);
		b.Base(Axe,1);
		a.Normalise();
		b.Normalise();
		float xA=(a||OA);
		float yA=(b||OA);
		float xB=(a||OB);
		float yB=(b||OB);
		float dx=xB-xA;
		float dy=yB-yA;
		float al=dx*dx+dy*dy;
		float be=2*(dx*(xA-0)+dy*(yA-0));
		float ga=(xA-0)*(xA-0) + (yA-0)*(yA-0)-R*R;
		float D=be*be+4*al*ga;
		if (D>=0)
		{
			float t;
			float t1=(-be-sqrtf(D))/(2*al);
			float t2=(-be+sqrtf(D))/(2*al);
			if (t1>t2) t=t2; else t=t1;
			if ((t>=0)&&(t<=1))
			{
				CVector I=A+t*(B-A);			
				CVector OI=I-Array[n]->Pos;
				if (OI.Norme()<L/2)
				{
					if (Calculate==1)
					{
						CVector I=A+t*(B-A);
						CVector N=I-Array[n]->Pos;
						VECTORNORMALISE(N);
						ContactStruct * cs=Contacts[buf].InsertLast();
						cs->Normals=N;
						cs->Point=I;
						cs->Types=COLLISION_WITH_OBJECT;
						cs->Distance=0;
						cs->Num=n;
					}
					return 1;
				}			
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionPob(CVector &A,CVector &B,int n,CPOB *pob,int Calculate,int buf)
{
	const float Cube[8][3]={ {-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1} };
	const float Norm[6][3]={ {0,0,1},{0,-1,0},{-1,0,0},{0,1,0},{1,0,0},{0,0,-1} };
	const int Faces[6][4]={
		{0,1,2,3},
		{3,2,6,7},
		{0,3,7,4},
		{1,0,4,5},
		{2,1,5,6},
		{4,7,6,5}
	};

	CVector UX=pob->Ux*Array[n]->MOActual;
	CVector UY=pob->Uy*Array[n]->MOActual;
	CVector UZ=pob->Uz*Array[n]->MOActual;
	CVector O,QA,QB,QC,QD,N;
	int nf,pt;

	O=pob->O*Array[n]->MActual;

	for (nf=0;nf<6;nf++)
	{
		pt=Faces[nf][0];
		QA=O+Cube[pt][0]*UX*pob->Lx+Cube[pt][1]*UY*pob->Ly+Cube[pt][2]*UZ*pob->Lz;
		pt=Faces[nf][1];
		QB=O+Cube[pt][0]*UX*pob->Lx+Cube[pt][1]*UY*pob->Ly+Cube[pt][2]*UZ*pob->Lz;
		pt=Faces[nf][2];
		QC=O+Cube[pt][0]*UX*pob->Lx+Cube[pt][1]*UY*pob->Ly+Cube[pt][2]*UZ*pob->Lz;
		pt=Faces[nf][3];
		QD=O+Cube[pt][0]*UX*pob->Lx+Cube[pt][1]*UY*pob->Ly+Cube[pt][2]*UZ*pob->Lz;
		N=Norm[nf][0]*UX+Norm[nf][1]*UY+Norm[nf][2]*UZ;
		if (ParticleCollisionQuad(A,B,QA,QB,QC,QD,N,COLLISION_WITH_OBJECT,n,Calculate,buf)) return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionBall(CVector &A,CVector &B,CVector &P,float R)
{
	CVector uA=P-A;
	CVector uB=P-B;
	if (VECTORNORM(uA)<R) return 1;
	if (VECTORNORM(uB)<R) return 1;
	float x=P.x;
	float y=P.y;
	float z=P.z;
	float K2=R*R - x*x - y*y - z*z;
	float dx=(B.x - A.x);
	float dy=(B.y - A.y);
	float dz=(B.z - A.z);
	float a=dx*dx + dy*dy + dz*dz;
	float b=2*((dx*A.x) + (dy*A.y) + (dz*A.z)) - 2 *(x*dx + y*dy + z*dz);
	float c=(A.x*A.x) + (A.y*A.y) + (A.z*A.z) - K2 -2*(A.x*x + A.y*y + A.z*z);
	float D=b*b - 4*a*c;
	if (D>=0)
	{
		float t;
		float t1=(-b + sqrtf(D))/(2*a);
		float t2=(-b - sqrtf(D))/(2*a);
		if (t1>t2) t=t2; else t=t1;
		if ((t>=0)&&(t<=1)) return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionSphere(CVector &A,CVector &B,int n,int Calculate,int buf)
{
	int res=0;
	float x=Array[n]->Pos.x;
	float y=Array[n]->Pos.y;
	float z=Array[n]->Pos.z;
	float R=Array[n]->Radius;
	float K2=R*R - x*x - y*y - z*z;
	float dx=(B.x - A.x);
	float dy=(B.y - A.y);
	float dz=(B.z - A.z);
	float a=dx*dx + dy*dy + dz*dz;
	float b=2*((dx*A.x) + (dy*A.y) + (dz*A.z)) - 2 *(x*dx + y*dy + z*dz);
	float c=(A.x*A.x) + (A.y*A.y) + (A.z*A.z) - K2 -2*(A.x*x + A.y*y + A.z*z);
	float D=b*b - 4*a*c;
	if (D>=0)
	{
		float t;
		float t1=(-b + sqrtf(D))/(2*a);
		float t2=(-b - sqrtf(D))/(2*a);
		if (t1>t2) t=t2; else t=t1;
		if ((t>=0)&&(t<=1))
		{
			if (Calculate==1)
			{
				CVector I=A+t*(B-A);
				CVector N=I-Array[n]->Pos;
				VECTORNORMALISE(N);

				ContactStruct * cs=Contacts[buf].InsertLast();
				cs->Normals=N;
				cs->Point=I;
				cs->Types=COLLISION_WITH_OBJECT;
				cs->Distance=0;
				cs->Num=n;
			}
			res=1;
		}		
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionQuad(CVector &A,CVector &B,CVector &QA,CVector &QB,CVector &QC,CVector &QD,CVector &N,int type,int tag,int Calculate,int buf)
{
	int res=0;
	CVector u;
	CVector a1,a2,a3,a4,I;
	CVector b1,b2,b3,b4;
	CVector c1,c2,c3,c4;
	float s1,s2,s3,s4;
	float a,b,c,d,t;
	float sa,sb;

	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*QA.x+b*QA.y+c*QA.z);

	sa=DOT(N,A) + d;
	sb=DOT(N,B) + d;
	if (sa>0)
	if (sa*sb<0)
	{
		t=-sa/(sb-sa);

		VECTORINTERPOL(I,t,B,A);

		VECTORSUB(a1,QB,QA);
		VECTORSUB(b1,I,QA);
		CROSSPRODUCT(c1,b1,a1);
		DOTPRODUCT(s1,N,c1);
		if (s1>=0)
		{
			VECTORSUB(a2,QC,QB);
			VECTORSUB(b2,I,QB);
			CROSSPRODUCT(c2,b2,a2);
			DOTPRODUCT(s2,N,c2);
			if (s2>=0)
			{
				VECTORSUB(a3,QD,QC);
				VECTORSUB(b3,I,QC);
				CROSSPRODUCT(c3,b3,a3);
				DOTPRODUCT(s3,N,c3);
				if (s3>=0)
				{
					VECTORSUB(a4,QA,QD);
					VECTORSUB(b4,I,QD);
					CROSSPRODUCT(c4,b4,a4);
					DOTPRODUCT(s4,N,c4);
					if (s4>=0)
					{
						if (Calculate==1)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							cs->Normals=N;
							cs->Point=I;
							cs->Types=type;
							cs->Distance=0;
							cs->Num=tag;
						}
						res=1;
					}
				}
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::ParticleCollisionFace(CVector &A,CVector &B,CFace *F,int type,int tag,int Calculate,int buf)
{
    
    float r;
    float s0;
    CVector OM,p,N,u;
    CVector uu1,uu2;
    int res=0;
    CVector pA,pB,I;
    float a,b,c,d,t;
    float R=0.05f;
    CVector a1,a2,a3;
    CVector b1,b2,b3;
    CVector c1,c2,c3;
    float s1,s2,s3;
    float sa,sb;
    CVector P=A;
    
    N=F->NormCalc;
    a=N.x;
    b=N.y;
    c=N.z;
    d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);
    
    sa=DOT(N,A) + d;
    sb=DOT(N,B) + d;
    
    if (sa>0)
    {
        if (sa*sb<0)
        {
            t=-sa/(sb-sa);
            
            VECTORINTERPOL(I,t,B,A);
            
            VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc);
            VECTORSUB(b1,I,F->v[0]->Calc);
            CROSSPRODUCT(c1,b1,a1);
            DOTPRODUCT(s1,N,c1);
            if (s1>=0)
            {
                VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc);
                VECTORSUB(b2,I,F->v[1]->Calc);
                CROSSPRODUCT(c2,b2,a2);
                DOTPRODUCT(s2,N,c2);
                if (s2>=0)
                {
                    VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc);
                    VECTORSUB(b3,I,F->v[2]->Calc);
                    CROSSPRODUCT(c3,b3,a3);
                    DOTPRODUCT(s3,N,c3);
                    if (s3>=0)
                    {
                        if (Calculate==1)
                        {
                            ContactStruct * cs=Contacts[buf].InsertLast();
                            cs->Normals=F->NormCalc;
                            cs->Point=I;
                            cs->Types=type;
                            cs->Distance=0;
                            cs->Num=tag;
                        }
                        res=1;
                    }
                }
            }
        }
        
        if (res==0)
        {
            VECTORMUL(u,N,100.0f);
            VECTORADD(pA,P,u);
            VECTORSUB(pB,P,u);
            
            s0=a*pA.x + b*pA.y + c*pA.z +d;
            s1=a*pB.x + b*pB.y + c*pB.z +d;
            
            t=-s0/(s1-s0);
            VECTORINTERPOL(I,t,pB,pA);
            VECTORSUB(u,I,P);
            r=VECTORNORM2(u);
            
            if (r<R*R)
            {
                VECTORSUB(uu1,I,F->v[0]->Calc);
                VECTORSUB(uu2,F->v[1]->Calc,F->v[0]->Calc);
                CROSSPRODUCT(OM,uu1,uu2);
                s0=DOT(OM,N);
                if (s0>0)
                {
                    VECTORSUB(uu1,I,F->v[1]->Calc);
                    VECTORSUB(uu2,F->v[2]->Calc,F->v[1]->Calc);
                    CROSSPRODUCT(OM,uu1,uu2);
                    s0=DOT(OM,N);
                    if (s0>0)
                    {
                        VECTORSUB(uu1,I,F->v[2]->Calc);
                        VECTORSUB(uu2,F->v[0]->Calc,F->v[2]->Calc);
                        CROSSPRODUCT(OM,uu1,uu2);
                        s0=DOT(OM,N);
                        if (s0>0)
                        {
                            if (Calculate==1)
                            {
                                ContactStruct * cs=Contacts[buf].InsertLast();
                                cs->Normals=F->NormCalc;
                                cs->Point=I;
                                cs->Types=type;
                                cs->Distance=0;
                                cs->Num=tag;
                            }
                            res=1;
                        }
                    }
                }
            }
        }
    }
    
    return res;

    /*
	int res=0;
	CVector N,I;
	float a,b,c,d,t;
	CVector a1,a2,a3;
	CVector b1,b2,b3;
	CVector c1,c2,c3;
	float s1,s2,s3;
	float sa,sb;

	N=F->NormCalc;
	a=N.x;
	b=N.y;
	c=N.z;
	d=-(a*F->v[0]->Calc.x+b*F->v[0]->Calc.y+c*F->v[0]->Calc.z);

	sa=DOT(N,A) + d;
	sb=DOT(N,B) + d;

	if (sa>0)
	{
		if (sa*sb<0)
		{
			t=-sa/(sb-sa);

			VECTORINTERPOL(I,t,B,A);

			VECTORSUB(a1,F->v[1]->Calc,F->v[0]->Calc);
			VECTORSUB(b1,I,F->v[0]->Calc);
			CROSSPRODUCT(c1,b1,a1);
			DOTPRODUCT(s1,N,c1);
			if (s1>=0)
			{
				VECTORSUB(a2,F->v[2]->Calc,F->v[1]->Calc);
				VECTORSUB(b2,I,F->v[1]->Calc);
				CROSSPRODUCT(c2,b2,a2);
				DOTPRODUCT(s2,N,c2);
				if (s2>=0)
				{
					VECTORSUB(a3,F->v[0]->Calc,F->v[2]->Calc);
					VECTORSUB(b3,I,F->v[2]->Calc);
					CROSSPRODUCT(c3,b3,a3);
					DOTPRODUCT(s3,N,c3);
					if (s3>=0)
					{
						if (Calculate==1)
						{
							ContactStruct * cs=Contacts[buf].InsertLast();
							cs->Normals=F->NormCalc;
							cs->Point=I;
							cs->Types=type;
							cs->Distance=0;
							cs->Num=tag;
						}
						res=1;
					}
				}
			}
		}

	}

	return res;
    /**/
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPhysic::IsPobSphere(CVector &P,float R,CVector &bounds,CMatrix &M,float Radius)
{
	int n;
	float r,ss;
	CVector L,D,Ls[7];
	float rA,DL,rA1,rA2,rA3;
	CVector Pos=M.getTranslation();

	VECTORSUB(D,P,Pos);
	r = Radius + R;

	if (VECTORNORM(D)<r)
	{
		CVector A1(bounds.x*M.a[0][0],bounds.x*M.a[0][1],bounds.x*M.a[0][2]);
		CVector A2(bounds.y*M.a[1][0],bounds.y*M.a[1][1],bounds.y*M.a[1][2]);
		CVector A3(bounds.z*M.a[2][0],bounds.z*M.a[2][1],bounds.z*M.a[2][2]);

		Ls[0]=A1;
		Ls[1]=A2;
		Ls[2]=A3;
		Ls[3]=A1+A2;
		Ls[4]=A1+A3;
		Ls[5]=A2+A3;
		Ls[6]=A1+A2+A3;

		for (n=0;n<7;n++)
		{
			L=Ls[n];
			VECTORNORMALISE(L);
			rA1=DOT(A1,L);
			rA2=DOT(A2,L);
			rA3=DOT(A3,L);
			rA=f_abs(rA1)+f_abs(rA2)+f_abs(rA3);
			DL=DOT(D,L);
			DL=f_abs(DL);
			ss=R+rA - DL;
			if (ss<0) return 0;
		}

		return 1;
	}

	return 0;
}


#endif
