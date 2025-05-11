////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////
#ifndef _PHYSIC_OBJECT_H_
#define _PHYSIC_OBJECT_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if ((!defined(GLES))&&(!defined(GLES20)))||(defined(GLESFULL))
#define MAX_BUF_PH						32
#else
#define MAX_BUF_PH						1
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_FACES_PER_OBJECT			16384

#define MAX_PHYSIC_OBJECTS_SCENE		1024	// multiple de 4
#define MAX_PHYSIC_OBJECTS_SCENE_SHL	10	    // shl

#define MAX_PLANES_SCENE				128
#define MAX_NORMALS2					256

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Defines for physic engine
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// internal

#define TYPE_CUBE				1
#define TYPE_SPHERE				2
#define TYPE_POB				3
#define TYPE_SPHERETREE			4
#define TYPE_CYLINDER			5
#define TYPE_OBJECT				6


#define COLLISION_WITH_PLANE	1
#define COLLISION_WITH_OBJECT	2
#define COLLISION_WITH_MESH		3



#define POB_NODE				1
#define POB_CLOSING				2
#define POB_MULTINODECLOSING	3
#define POB_MULTINODE			4
#define POB_SPHERE				4
#define POB_CUBE				8


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Positionned Oriented Box
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class API3D_STUB CPOB
{
public:
	unsigned char	Type;
	unsigned char	ID;
	CVector			O;
	CVector			Ux,Uy,Uz;
	float			Lx,Ly,Lz;
	float			mLx,mLy,mLz;
	float			R;
	CPOB *			Next0;
	CPOB *			Next1;
	CPOB *			Next2;
	int				nPOB;

	CPOB() { Next0=Next1=Next2=NULL; }

	void InitCube(float Lx,float Ly,float Lz);

	void SetMulti() { ID=POB_MULTINODE; }
	void SetMultiClosing() { ID=POB_MULTINODECLOSING; }
	void SetClosing() { ID=POB_CLOSING; }
	void SetNode() { ID=POB_NODE; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	BALL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	float x,y,z,r;
	short int b0,b1,b2;
	char tag;
	char tags[MAX_BUF_PH];
} Ball;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Sphere tree
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API3D_STUB CSphereTree
{
public:
	int				nB;
	Ball *			Spheres;
	int				Nmax;
	char			str[512];

	CSphereTree() { nB=0;Spheres=NULL;}

	void Load(const char * str,float scale);
	void Save(const char * str);
	void Free();
	int Recurs(int posnB,int N);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Sphere Partitionnement
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	float			x,y,z,r;
	short int		b0,b1,b2;
	char			tag;
	unsigned char	tags[MAX_BUF_PH];
	CVector			calc;

} BallP;

class API3D_STUB CSpherePartition
{
public:
	int				nbx,nby,nbz;
	BallP			balls[64];
	unsigned char	tag[64];
	
	CSpherePartition() { nbx=nby=nbz=0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Data structure of a physic object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this is the class defined in a CObject3D as "Proprietes"

#ifdef _DEFINES_API_CODE_PHYSIC_

//! Data structure for primitives in order to be used by physic engine
class API3D_STUB CPhysicObject
{
public:
	// internal
	int				index;
	int				OrderInJacobian;
	int				nGroup;
	bool			Calc;
	bool			Linked;
	bool			LinkedCollision;
	bool			Fixe;
	CVector			Links[32];
	int				nLinks;
	bool			Handled;

	CVector			Pos0;
	CVector			Pos;
	CVector			Posdt;
	CVector			aPos;

	CVector			Vit;
	CVector			aVit;
	CVector			Vit_dt;

	bool			statique;
	float			ct,ct1,ct2;
	CVector			F,aF;

	CVector			dOmega;

	CQuaternion		Q;
	CQuaternion		Q_dt;
	CQuaternion		aQ;


	float			LinkFriction;
	float			dAlpha;
	CVector			T;

	CVector			PUSH;
	CVector			PUSH_POINT;
	CVector			Omega_dt;
	CVector			Omega;
	CVector			aOmega;
	CVector			Lg;
	CVector			aLg;
	CMatrix			I;
	CMatrix			Inv_I;
	CMatrix			IIt;
	CMatrix			It;

	CMatrix			R;
	CMatrix			aR;
	CMatrix			MActual;
	CMatrix			MOActual;
	CMatrix			MRef;
	CVector			u;

	CVector			decal;

	CVector			Dep;
	int				rep;
	float			f;
	float			Restitution;
	float			Friction;
	float			Alpha;
	float			Alphadt;
	float			aAlpha;
	int				Collided;
	CPhysicObject*	support;

	float			Mass;
	float			InvMass;

	// cube
	short int		Type;
	short int		NoCollidingDynamic;

	CVector			Ux;
	CVector			Uy;
	CVector			Uz;
	float			Lx,Ly,Lz;

	// sphere - cylindre

	float			Radius;
	CVector			U;
	float			L;

	// pob

	CPOB *			pob;
	CSphereTree *	STREE;
	CSpherePartition * SP;
	void *			OBJECT;

	int				wait;
	unsigned char	update;
	unsigned char	approx;
	unsigned char   value1;
	unsigned char   value2;

	bool			enh;
	bool			collide_env;
	bool			wind_influence;
	bool			fluid;
	unsigned char	collision_replace_type;
	unsigned char	ball_inertiatensor;
	int				tag;

	int				ndx;
	
	float			viscosity,temperature;
    
    int             ext;
    
    CMatrix         FixM,FixMo;

    CPhysicObject() { fluid=false; ext=0; NoCollidingDynamic=0; PUSH.Init(0,0,0); LinkedCollision=false; collision_replace_type=1; value1=0; support=NULL; wind_influence=false; collide_env=true; SP=NULL; Type=0; LinkFriction=0.0f; Fixe=Linked=false;nLinks=0;index=0;pob=NULL;STREE=NULL;wait=0;enh=false; OBJECT=NULL; Friction=0.3f; }

	~CPhysicObject()
	{
		if (pob) FreePob(pob);
		pob=NULL;

		if (STREE)
		{
			STREE->Free();
			STREE=NULL;
		}

		if (SP) 
		{ 
			delete SP; 
			SP=NULL;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//! retrieves matrix
		CMatrix ActualMatrix();
		//! retrieves matrix
		CMatrix ActualOrientationMatrix();
        //! set matrix
        void Fix();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//! initialisation fonctions (must be set)
		void InitRadius(float r);
		//! initialisation fonctions (obsolete)
		void InitCube_x(float lx,CVector ux);
		//! initialisation fonctions (obsolete)
		void InitCube_y(float ly,CVector uy);
		//! initialisation fonctions (obsolete)
		void InitCube_z(float lz,CVector uz);
		//! initialisation fonctions (obsolete)
		void IsACube();
		//! initialisation fonctions (cylinder)
		void IsACylinder();
		//! initialisation fonctions (pob, pobtree see POBGenerator)
		void IsAPob();
		//! initialisation fonctions (ball)
		void IsASphere();
		//! initialisation fonctions (approx by sphere trees)
		void IsASphereTree();
		//! initialisation fonctions (object-object are pob collision)
		void IsAMesh();
		//! initialisation fonctions (full mesh collision)
		void IsAFullMesh();
		//! initialisation fonctions (mesh collision, inertia of ball)
		void IsAMeshBall();

		//! mass of the object
		void SetMass(float m);
		//! collision restitution
		void SetRestitution(float k);
		//! initialisation fonctions
		void InitCylinder(float r,float l,CVector u);
		//! set the orientation quaternion
		void SetEulerOrientation(float rx,float ry,float rz);
		//! free ressource
		void Free();
		//! free pob ressource
		void FreePob(CPOB * pob);
		//! initialisation before physic using
		void Reset();
		//! initialisation before physic using without orientation reset
		void ResetWoQ();
		//! initialisation before physic using
		void InertiaTensor();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CParticle
{
public:
	// internal
	CVector			Pos;
	CVector			Posdt;
	CVector			aPos;
	CVector			Vit;
	CVector			aVit;
	CVector			Vit_dt;
	CVector			F;
	CVector			dep;
	CVector			PUSH;
	int				Collided;
	float			Mass;
	float			Time;
	bool			active;
	bool			wind_influence;
	bool			gravity;
    bool            statique;
	int				xx,yy,zz;
	int				ofs;
    CVector         moy,moy2;
    int             nbmoy,nbmoy2;
    float           iter;
    bool            yindependant;
	float			density;
	CVector			grad,viscosity;
	float			strength;
    
	CParticle()
	{
		Mass=1.0f;
		density=1.0f;
		strength=0.2f;
		viscosity.Init(0,0,0);
		grad.Init(0,0,0);
        iter=1;
		statique=false;
		Time=0.0f;
		PUSH.Init(0,0,0);
		wind_influence=false;
		gravity=true;
		active=false;
        yindependant=true;
	}

	~CParticle()
	{
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	MOBIL_JOINT	    =	1,
	FIXED_JOINT	    =	2,
	PIVOT_JOINT	    =	3,
	BALL_JOINT	    =	4,
    SPRING_JOINT    =   5,
    FIXED_PIVOT     =   6,
    FIXED_SPRING    =   7
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Data structure of physic joints
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API3D_STUB CPhysicJoint
{
public:
	int				type;
	int				num;
	int				NbreOP,NbreJoints;
	CPhysicObject *	Objs[32];	// utile qu'au basejoint
	int				nObjs;
	CVector			aG,dG,Gdt;
	CVector         ptfixe;
    CVector         axis1,axis2;
	CPhysicObject *	Object1;
	CPhysicObject *	Object2;
	CVector			r1,r2;
	CVector			u1,u2;
	CPhysicJoint *	NextJoint;
	int	*			jacobians_indices;
	int	*			jacobians_len;
	int				total_len;
    float           raideur;
    float           L1,dL1,L2,dL2;
    float           Lt,L,dL;
	float			angle_limit;
    bool            relax,skelet;
	int				relaxation;
	float			dbase;

	/////////////////////////////////////////////////////////////////
    CPhysicJoint() { jacobians_indices=jacobians_len=NULL; angle_limit=-1; raideur=1.0f; skelet=relax=false; nObjs=total_len=NbreJoints=NbreOP=num=0; relaxation=0; dbase=-1; }
	~CPhysicJoint() { if (jacobians_indices) { delete [] jacobians_indices; delete [] jacobians_len;} jacobians_indices=NULL; jacobians_len=NULL; }
	/////////////////////////////////////////////////////////////////
	CPhysicJoint *AddJointSpring(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,float K);
    CPhysicJoint *AddJointPivot(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,CVector axe1,CVector axe2);
	CPhysicJoint *AddJoint(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2);
	CPhysicJoint *AddJointBall(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2);
	CPhysicJoint *Fixe(CVector pos,CPhysicObject *op,CVector r);
    CPhysicJoint *FixePivot(CVector pos,CPhysicObject *op,CVector r,CVector axe);
    CPhysicJoint *FixeSpring(CVector pos,CPhysicObject *op,CVector r,float K);
	/////////////////////////////////////////////////////////////////
	int Nbre();
	int NbreOp();
	/////////////////////////////////////////////////////////////////
	CMatrix_MxN * BigJacobian();
	CMatrix_MxN * BigJacobian(int tag);
	/////////////////////////////////////////////////////////////////
	void BigJacobianPatch(CMatrix_MxN *Big,int ofs,int tag);
	int setupJacobian();

	void del(CPhysicObject *op);
};


//#define ObjetPhysique CPhysicObject



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Internal structure
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class API3D_STUB CCRC
{
public:
	int				nb;
	CVector			N;
	CVector			M;
};

class API3D_STUB CContactPoint
{
public:
	CVector Normals[MAX_NORMALS2];
	CVector ContactPoint[MAX_NORMALS2];
	int Num[MAX_NORMALS2];
	int nbre;

};


#endif






#endif
