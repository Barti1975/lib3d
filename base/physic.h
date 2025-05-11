
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PHYSIC.H
//
//	directX/OpenGL
//
//	original form v0.01
//
//	by Laurent CANCE 
//	September 2003
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  

#ifndef _PHYSIC_H_
#define _PHYSIC_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if ((!defined(GLES))&&(!defined(GLES20)))||(defined(GLESFULL))
#if !defined(WEBASM)&&!defined(API3D_SDL2)
#if defined(API3D_VR)||defined(GOOGLEVR)
#else
#define MULTITHREADED_PHYSIC
#define MULTITHREADED_PHYSIC_STATE
#define NBTHREADS MAX_BUF_PH
#endif
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Main task for physic
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEFINES_API_CODE_PHYSIC_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern float CONSTANT_STATIC_VELOCITY;
extern float CONSTANT_STATIC_TORQUE;
extern float CONSTANT_STATIC_WATER_MASS;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	unsigned char used;
	CVector p0;
	CVector p1;
	CVector p2;
	CVector N;
	CPhysicObject * op1;
	CPhysicObject * op2;
	int index_plane;
	int index_mesh;


} STRUCT_NAME_CONTACT;

#define MAX_CONTACT_POINTS_STAB 256

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	OBJECT_OBJECT		=	1,
	OBJECT_PLANE		=	2,
	OBJECT_MESH			=	3
};

enum
{
	CONTACT_IMPULSE		=	1,
	CONTACT_RESTING		=	2,
	CONTACT_SLIDE		=	4
};


typedef struct
{
	unsigned char type;
	unsigned char contact_type;
	CVector p;
	int ndx1,ndx2;	
	float vrel;
} STRUCT_EVENT_CONTACT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float y;
	int nobj;
} ZSort;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float Distance;
	CVector Normals;
	CVector Point;
	int Types;
	int Num;	

} ContactStruct;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ContactParticlesCollisionMesh
{
public:
    CVector mini,maxi;
    int nb,iter;
    bool is;
    ContactParticlesCollisionMesh() { is=false; nb=0; iter=1; }
};

#ifdef MULTITHREADED_PHYSIC
class PhysicThread;
class CPhysic;

typedef struct
{
	CPhysic *phy;
	int start,end;
	int task;

} dataparticulesthreads;

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Physic Engine
//! several method of calculation are avaiable
//! collision are made by primitives, or full objects definition
class API3D_STUB CPhysic
{
public:
	unsigned char Predict[MAX_PHYSIC_OBJECTS_SCENE*MAX_PHYSIC_OBJECTS_SCENE];
	unsigned char NoCollision[MAX_PHYSIC_OBJECTS_SCENE*MAX_PHYSIC_OBJECTS_SCENE];
	bool GetEventList;
	int LEVEL_INF_SPHERETREE;
	CList <STRUCT_EVENT_CONTACT> Events;
	CList <STRUCT_EVENT_CONTACT> EventsTH[MAX_BUF_PH];
	int POST_STABILIZATION_METHOD;
	CPhysicJoint *BaseJoints[2048];
	int nBaseJoints;
	CCollisionMesh CollisionMeshes[128];
    ContactParticlesCollisionMesh ContactsParticles[128];
	int nCollisionMeshes;
	CPhysicObject * Array[MAX_PHYSIC_OBJECTS_SCENE];
	int Sortn[MAX_PHYSIC_OBJECTS_SCENE];
	float Sorty[MAX_PHYSIC_OBJECTS_SCENE];
	CVector Array_vG[MAX_PHYSIC_OBJECTS_SCENE];
	int nArray;
	CPlane Planes[MAX_PLANES_SCENE];		// forme un volume convexe
	int nPlanes;
	float Restitution[MAX_PLANES_SCENE];
	float Friction[MAX_PLANES_SCENE];
	float Gravite;
	float DT;
	int N_ITERATIONS;
	bool ReplaceObjects;
	bool ReplaceObjectsIterations;
	float DMAX;
	CVector Axe,Axe1,Axe2;
	bool scale_time;
	CList <ContactStruct> Contacts[MAX_BUF_PH];
	CCRC CRC[MAX_PHYSIC_OBJECTS_SCENE];
	CCRC *CRC_f;
	int lCRC[MAX_PHYSIC_OBJECTS_SCENE];
	int nCRC;
	int static_method;
	unsigned char bancal_algo;
	float K_MUL_ITERATIONS;
	int NumProcs;
	int N_ITERATIONS_tmp;
	float dt_int;
	CParticle * ParticlesArray;
	int nbParticlesArray;
	ZSort SortedIndices[MAX_PHYSIC_OBJECTS_SCENE];

	CList <int> *particlesvoxel;

	float viscosity,temperature,dist;
	float y_max;

	float step_particles;
	float mul_particles;
    float mul_particles_limit;
	float viscosity_particles;
	float proche_particles;
	float proche_particles_ext;
	int recurs_particles;
    float iter_particles_local;

	int particlesboundx;
	int particlesboundy;
	int particlesboundz;
#ifdef MULTITHREADED_PHYSIC
    PhysicThread *CORE;
#endif
    
    bool processed;
    bool fix;

	//////////////////////// SPH

	float volumeSPH;
	float radiusSPH;
	float radiusSPH2;
	float scaleSPH;

    CVector GPmin,GPmax,GPcell;
	int dcellx,dcelly,dcellz;
	float targetdensity,presuremul;

	float * precalcdist;

#ifdef MULTITHREADED_PHYSIC
	CPhysic * thisone;
	dataparticulesthreads particlesthreaddata[64];

	int PARTICULES_ACT;
	int PARTICULES_ACT2;
#endif
    
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CPhysic() 
	{
		NumProcs=-1;
#ifdef MULTITHREADED_PHYSIC
        CORE=NULL;
		PARTICULES_ACT=0;
		PARTICULES_ACT2=-1;
#endif

		precalcdist=NULL;
		presuremul=1;
		targetdensity=1;
        
        fix=true;
        processed=true;
        
		particlesboundx=20;
		particlesboundy=20;
		particlesboundz=20;

		particlesvoxel=NULL;

        iter_particles_local=1.0f;
		mul_particles=-1.0f;
        mul_particles_limit=1000000.0f;
        
		step_particles=-1.0f;
		recurs_particles=4;
		viscosity_particles=0.999f;
		proche_particles=0.15f;
		proche_particles_ext=0.15f;

		temperature=25.0f;
		viscosity=0.0f;
		dist=5.0f;
		y_max=0;
		bancal_algo=0;
		LEVEL_INF_SPHERETREE=3;
		nPlanes=0;
		nArray=0;
		Gravite=9.81f;
		DT=1;
		nBaseJoints=0;
		nCollisionMeshes=0;
		GetEventList=false;
		ReplaceObjects=false;
		POST_STABILIZATION_METHOD=1;
		static_method=0;
		CRC_f=new CCRC[MAX_FACES_PER_OBJECT*(32+1)];
		ParticlesArray=NULL;
		nbParticlesArray=0;
		scale_time=true;
		K_MUL_ITERATIONS=16;
		ReplaceObjectsIterations=false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	~CPhysic()
	{
#ifdef MULTITHREADED_PHYSIC
        KillTHs();
#endif
		delete [] CRC_f;
		Events.Free();

		if (particlesvoxel) delete[] particlesvoxel;
		particlesvoxel=NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// initialisation

		//! scale time increase iterations for fast movements
		void        disableScaleTime();
		//! scale time increase iterations for fast movements
		void        enableScaleTime();
		//! index of props
		int			numTemporary(CPhysicObject * op);
		//! test if there props
		bool		isTemporary(CPhysicObject * op);
		//! add props
		void		addTemporary(CPhysicObject * op);
		//! add props
		void		addTemporary(CPhysicObject * op,bool link);
		//! del props
		void		delTemporary(CPhysicObject * op);
		//! set LOD for sphere tree
		void		setST_LOD(int n) {LEVEL_INF_SPHERETREE=n;}
		//! physic events to attach sound for exemple
		void		enableEvents();
		//! physic events to attach sound for exemple
		void		disableEvents();
		//! approximation of time collision		
		void		SetApproximateTimeCollisionIterations(bool value);
		//! parametring simulation
		void		setAverageStabilization();
		//! parametring simulation (best case)
		void		setNormalStabilization();
		//! parametring simulation (compatibility mode)
		void		setNormalOldCodeStabilization();
		//! fix this to one for a quick stable simulation combining with CONSTANT_STATIC_VELOCITY value
		void		setStaticDeterminationMethod(int method);

		void		setScaleTimeMul(float sc);
    
        void        FixProps();

		//! calculate
		void		Calculate(CPhysicObject *op,float dt);

		//! initialisation
		void		PrecalculateBaseJointsRelaxation();

		//! initialisation
		void		Init();

		//! free ressources
		void		Free();    
        void        KillTHs();
    
		//! add props
		void		NewObject(CPhysicObject * op);
		void		NewObject(CPhysicObject * op,bool link);

		//! add dynamic joint
		CPhysicJoint * AddBaseJoint(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2);
		//! add dynamic joint
		CPhysicJoint * AddBaseJointBall(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2);
		//! add generic joint
		CPhysicJoint * AddBaseJointFixed(CVector pos,CPhysicObject *op,CVector r);
        //! add dynamic pivot
        CPhysicJoint * AddBaseJointPivot(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,CVector axe1,CVector axe2);
        //! add dynamic spring
        CPhysicJoint * AddBaseJointSpring(CPhysicObject *op1,CPhysicObject *op2,CVector r1,CVector r2,float K);
        //! add generic joint
        CPhysicJoint * AddBaseSpringFixed(CVector pos,CPhysicObject *op,CVector r,float K);
        //! add generic joint
        CPhysicJoint * AddBasePivotFixed(CVector pos,CPhysicObject *op,CVector r,CVector axe);

		//! set gravity of environment
		void		SetGravity(float g);
		//! set subdivision interation number for integration
		void		SetIterations(int niter);
		//! reset planes
		void		ResetScenePlanes();
		//! reset props
		void		ResetObjsArray();
		//! reset 
		void		Reset();
		//! specifie if the simulation do approximation on object replacing
		void 		SetApproximateTimeCollision(bool value);

		//! add a pseudo moving mesh which simplier object can collide with
		CCollisionMesh *AddMesh(CObject3D * obj,float r,float f,int N);
		//! add a plane to environment
		void		AddPlan(float a,float b,float c,float d,float k,float f);

		//! add a plane of water to environment
		void		AddPlanWater(float a,float b,float c,float d,float k,float f);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// physic particules SPH

		void		SetConstantsSPH(float radius);

		void		CalculateDensityParticle(int n);
		void		CalculateGradientParticle(int n);
		void		CalculateViscosityParticle(int n);

#ifdef MULTITHREADED_PHYSIC
		void		ParticlesThreads();
		void		ParticlesThreadsEnd();
		void		ParticlesDO(int task);
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// physic processing

		//! set time interval for simulation
		void 		SetTimeInterval(float dt);
		//! calculate the physic of a scene
		void 		CalculateScene();

		
		void 		CalculateSceneNormal();
		
		int			IndexProp(CPhysicObject * prop);

		void		TagCollisionLinks();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

		//! (obsolete at this moment)
		void		setLCPStabilization();
		// (obsolete)
		void		CalculateMatriceInertiePOB(CPOB * pob,CMatrix * M,float p);
		// (obsolete)
		float		CalculateVolumePOB(CPOB * pob);
		
		void 		ApproximateTimeCollision(int n1,int n2);
		
		int 		TestBit(int n);
		
		void 		SetBit(int n);
		
		int 		TestBitC(int n);
		
		void 		SetBitC(int n);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// internals

		// apply collision response for an object
		void		ProcessBodyCollisions(int n,int buf);

		void		ProcessBodyStateUpdate(int n,int buf);

		void		ProcessBodyTime(int n,int buf);
		
		int 		IsObj1CollidingObj2(int n1,int n2,int Calculate,int buf);
    
        void        ProcessBaseJoints(int n,float dt,float dt_int);

		// objects

		
		int 		IsObject1CollidingObject2(int n1,int n2,int Calculate,int buf);

		// sphere tree

		
		int 		IsBallCollidingPlane(CVector &P,float r,int nPlan,int Calculate,int buf);
		
		int 		IsSphereTreeCollidingPlane(int nObj,int n,int nPlan,int Calculate,int buf);

		// pob closing

		
		int 		IsPobClosing1CollidingCube2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsCube1CollidingPobClosing2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsPobClosing1CollidingSphere2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsSphere1CollidingPobClosing2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsPobClosing1CollidingPobClosing2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf);
		

		// pob

		
		int 		IsPob1CollidingPob2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N);
		
		int 		IsPob1CollidingCube2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsCube1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsPob1CollidingSphere2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsSphere1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf);


		// cube sphere (obsolete)

		
		int 		IsCube1CollidingCube2(int n1,int n2,int Calculate,int buf);
		
		int 		IsSphere1CollidingSphere2(int n1,int n2,int Calculate,int buf);
		
		int 		IsSphere1CollidingCube2(int n1,int n2,int Calculate,int buf);
		
		int 		IsCube1CollidingSphere2(int n1,int n2,int Calculate,int buf);


		// pob - pob
		
		
		int 		IsPobTree1CollidingPobTree2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N);
		
		int 		IsPobMulti1CollidingPobMulti2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N);
		
		int 		IsPobTree1CollidingPobMulti2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N);
		
		int 		IsPobMulti1CollidingPobTree2(int n1,int n2,CPOB * pob1,CPOB * pob2,int Calculate,int buf,int N);


		// sphere tree vs others

		
		int 		IsPobClosing1CollidingBall2(int n1,int n2,CPOB * pob,CVector &P,float R,int Calculate,int buf);
		
		int 		IsSphere1CollidingBall2(int n1,int n2,CVector &P,float R,int Calculate,int buf);
		
		int 		IsBall1CollidingSphere2(int n1,int n2,CVector &P,float R,int Calculate,int buf);
		
		int 		IsBall1CollidingBall2(int n1,int n2,CVector &P1,float R1,CVector &P2,float R2,int Calculate,int buf);
		
		int 		IsSphereTree1CollidingSphereTree2(int n1,int n2,int nB1,int nB2,int Calculate,int buf,int N);
		
		int 		IsSphere1CollidingSphereTree2(int n1,int n2,int nB2,int Calculate,int buf);
		
		int 		IsSphereTree1CollidingSphere2(int n1,int n2,int nB1,int Calculate,int buf);
		
		int 		IsSphereTree1CollidingPob2(int n1,int n2,int nB1,CPOB * pob2,int Calculate,int buf);
		
		int 		IsPob1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf);
		
		int 		IsPobClosing1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf);
		
		int 		IsPobMulti1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf);
		
		int 		IsPobTree1CollidingSphereTree2(int n1,int n2,CPOB * pob1,int nB2,int Calculate,int buf);


		// plane

		
		int 		IsPOBClosingCollidingPlane(int nObj,CPOB * pob,int nPlan,int Calculate,int buf);
		
		int 		IsObjCollidingPlane(int nObj,int nPlan,int Calculate,int buf);
		
		int 		IsPOBCollidingPlane(int nObj,CPOB * pob,int nPlan,int Calculate,int buf);

		// faces-mesh

		
		int 		IsObjCollidingMesh(int nObj,CCollisionMesh * mesh,int Calculate,int buf);
		
		int 		IsBallCollidingFace(CVector &P,float R,CFace *F,int Calculate,int buf,float a,float b,float c,float d);

		int 		IsBallCollidingFace(CVector &P,float R,CFace *F,int Calculate,int buf);
		
		int 		IsPobClosingCollidingFace(int n,CPOB * pob,CFace *F,int Calculate,int buf);
		
		int 		IsSphereTreeCollidingFace(int nObj,int n,CFace *F,int Calculate,int buf);
		
		int 		IsPOBCollidingFace(int nObj,CPOB * pob,CFace *F,int Calculate,int buf);
		
		int 		IsFaceCollidingFace(CFace *F,CFace *Fref,int Calculate,int buf);


		// cylinders

		
		int 		IsCylinderCollidingPlane(int nObj,int nPlan,int Calculate,int buf);
		
		int 		IsCylinderCollidingFace(int nObj,CFace *F,int Calculate,int buf);
		
		int 		IsCylinder1CollidingCylinder2(int n1,int n2,int Calculate,int buf);
		
		int 		IsCylinder1CollidingBall2(int n1,int n2,CVector &O,float R,int Calculate,int buf);
		
		int 		IsSphereTree1CollidingCylinder2(int n1,int n2,int nB1,int Calculate,int buf);
		
		int 		IsCylinder1CollidingSphereTree2(int n1,int n2,int nB2,int Calculate,int buf);
		
		int 		IsBall1CollidingCylinder2(int n1,int n2,CVector &O,float R,int Calculate,int buf);
		
		int 		IsCylinder1CollidingPob2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsPob1CollidingCylinder2(int n1,int n2,CPOB * pob,int Calculate,int buf);
		
		int 		IsCylinder1CollidingPobClosing2(int n1,int n2,CPOB *pob,int Calculate,int buf);
		
		int 		IsPobClosing1CollidingCylinder2(int n1,int n2,CPOB *pob,int Calculate,int buf);

		// particles

		void		ParticlesInit(int nb);
		void		ParticlesFree();

		bool		ParticlesAdd(CVector &pos,CVector &vit,float mass,bool replace);
		bool		ParticlesAdd(CVector &pos,CVector &vit,float mass,float visc,bool replace);


		// primary
		int			ParticleCollisionFace(CVector &A,CVector &B,CFace *F,int type,int tag,int Calculate,int buf);
		int			ParticleCollisionQuad(CVector &A,CVector &B,CVector &QA,CVector &QB,CVector &QC,CVector &QD,CVector &N,int type,int tag,int Calculate,int buf);
		int			ParticleCollisionBall(CVector &A,CVector &B,CVector &P,float R);

		// physic objects
		int			ParticleCollisionPob(CVector &A,CVector &B,int n,CPOB *pob,int Calculate,int buf);
		int			ParticleCollisionSphere(CVector &A,CVector &B,int n,int Calculate,int buf);
		int			ParticleCollisionObject(CVector &A,CVector &B,int nObj,int Calculate,int buf);
		int			ParticleCollisionCylinder(CVector &A,CVector &B,int n,int Calculate,int buf);
		int			ParticleCollisionProp(CVector &A,CVector &B,int nObj,int Calculate,int buf);

		int			ParticleCollisionPlane(CVector &A,CVector &B,int nPlan,int Calculate,int buf);
		int			ParticleCollisionMesh(CVector &A,CVector &B,int nObj,CCollisionMesh * mesh,int Calculate,int buf);

		// tests sur la vitesse

		
		bool 		IsPOBClosingVelocityTooHigh(int nObj,CPOB * pob,float dt,CVector &V,CVector &W);
		
		bool 		IsBallVelocityTooHigh(CVector &P,float R,float dt,CVector &V,CVector &W);
		
		bool 		IsSphereTreeVelocityTooHigh(int nObj,int n,float dt,CVector &V,CVector &W);
		
		bool 		IsPOBVelocityTooHigh(int nObj,CPOB * pob,float dt,CVector &V,CVector &W);
		
		bool 		IsObjVelocityTooHigh(int nObj,float dt,CVector &V,CVector &W);

		// misc

		
		int 		FacesCollisionIntermediate(CFace *F1,CFace *F2,bool snd_is_mesh,int Calculate,int buf);
		
		int 		ObjectFacesCollisionIntermediate(CFace *F1,CFace *F2,int nObj,int Calculate,int buf);

		int 		ObjectFacesCollisionIntermediate2(CFace *F1,CFace *F2,int nObj,int Calculate,int buf);
		
		int 		IsThereCollision(CPhysicObject *op1,CPhysicObject *op2);
		
		int			SegmentCollisionFace(CObject3D *obj,CVector &O,CVector &A,CFace *F,int type,int tag,int Calculate,int buf);
		
		int 		EdgeCollisionFace(CObject3D *obj,int a,int b,CFace *F,int type,int tag,int Calculate,int buf);
		
		int 		PropGenEdgeCollisionFace(int nObj,CFace *F,int type,int tag,int Calculate,int buf);

		bool 		TrivialTestPobFace(int nObj,CPOB *pob,CFace *F);

		float		VolumObjUnderPlane(int nObj,int nPlan);

		int			IsPobSphere(CVector &P,float R,CVector &bounds,CMatrix &M,float Radius);
};

#endif

/*?

  <B><U><I>USAGE:</I></U></B>

	CPhysic Physique;
	CObject3D MESH1;
	CObject3D obj;

	obj.Sphere(1,16);  
	obj.Proprietes.IsASphere();

	objs[n].Proprietes.InitRayon(1);

	objs[n].Proprietes.SetMass(10);  // 10 kg
	objs[n].Proprietes.SetRestitution(0.1f);
	objs[n].Proprietes.Pos.Init(x0,y0,z0);

	Physique.Reset();
	Physique.NewObject(&obj.Proprietes);
	Physique.AddMesh(MESH1,0.1f,0.6f,18);

	Physique.SetIterations(7);
	Physique.SetGravity(9.81f*dim);

	Physique.Init();


	while (looping)
	{
		Physique.SetTimeInterval(0.035f*TIME_BETWEEN_TWO_FRAMES/FIXED_TIME_BETWEEN_TWO_FRAMES);
		Physique.CalculateScene();
	}

	///////////////////////////////////////////////////////////
	// Initialisation:
	///////////////////////////////////////////////////////////

	EnvPhysic=new CPhysic;
	EnvPhysic->Reset();
	EnvPhysic->AddMesh(Scene.WorldObject,ENV_RESPONSE,ENV_FRICTION,NITER_ENVPHYSIC);
	EnvPhysic->SetIterations(PHYSIC_ITERATIONS);
	EnvPhysic->SetGravity(9.81f);
	EnvPhysic->SetApproximateTimeCollision(PHYSIC_APPROXIMATION);

#ifdef NORMAL_STABILIZATION
	EnvPhysic->setNormalStabilization();
#else
	EnvPhysic->setAverageStabilization();
#endif

#ifdef DISABLE_SCALE_TIME
	EnvPhysic->disableScaleTime();
#else
	EnvPhysic->enableScaleTime();
	EnvPhysic->setScaleTimeMul(4);
#endif
	EnvPhysic->enableEvents();
	EnvPhysic->setStaticDeterminationMethod(STATIC_METHOD_VALUE);
	EnvPhysic->setST_LOD(PHYSIC_LOD);

	EnvPhysic->Init();       

	///////////////////////////////////////////////////////////
	// Comments:
	///////////////////////////////////////////////////////////

	PHYSIC_APPROXIMATION : bool (approximate positions of objetcs to avoid interpenetrations)
	Prop.wait / STATIC_METHOD_VALUE <=> frame number until stabilization
	DISABLE_SCALE_TIME : increase number of interval in physic eulers solve

	Prop.statique: value representing static/dynamic state of the object	

	Global variable
	CONSTANT_STATIC_VELOCITY=0.04f;
	set the thresold value to determine if object will be static	

	///////////////////////////////////////////////////////////
	// Initialisation of dynamic object:
	///////////////////////////////////////////////////////////

	* IsAMesh() , IsAFullMesh() , IsAPob():

	CObject3D *obj;
	CPhysicObject Props;

	if (Props.pob==NULL)
	{
		for (nn=0;nn<obj->nFaces;nn++) obj->Faces[nn].tag=0;
		pob=genPob.POBFrom2(obj,0,0);
	}
	else pob=Props.pob;

	Props.pob=pob;
	Props.IsAPob();

	if (bounding box)
	{
		Props.IsAPob();
	}
	else
	{
		Props.IsAMesh();
		(ou) Props.IsAFullMesh();

		if (Props.OBJECT==NULL) Props.OBJECT=obj;
		if (Props.SP==NULL)
		{
			Props.SP=((CObject3D*)Props.OBJECT)->CreateSpherePartition(4);
		}
	}

	Props.InitRadius(obj->Radius);
	Props.surobjet=NULL;
	Props.SetMass(xx);
	Props.SetRestitution(OBJ_RESPONSE);
	Props.Reset();
	Props.InertiaTensor();
	Props.Pos=obj->Coo;
	Props.R.RotationDegre(0,0,0);
	Props.Vit.Init(0,0,0);

*/

void SetPhysicWindForce(CVector F);

#endif
