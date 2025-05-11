
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
float KD=2.0f;//2.0f*2.0f;
float KDa=4.0f; //4.0f*4.0f;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define LOG_PHYSIC
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "params.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "maths.h"
#include "objects3d.h"
#include "list.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEFINES_API_CODE_PHYSIC_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern float COEF_ENVELOPPE;
extern float CONSTANT_STATIC_VELOCITY;
extern float CONSTANT_STATIC_TORQUE;
extern float CONSTANT_STATIC_VELOCITY_SQUARE;
extern float CONSTANT_STATIC_TORQUE_SQUARE;
extern float CONSTANT_STATIC_SECONDARY;
extern float CONSTANT_STATIC_REPLACEMENT;
extern float CONSTANT_STATIC_WATER_MASS;
extern CVector DYNAMIC_WIND_FORCE;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SLIDING 0.1f
#define TEST_NORM 1.0f
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PhysicMultithreadToggle=true;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CList <char *> Log_Physic;
char __context[1024]={ '\0' };
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_Log(char *str,CQuaternion q)
{
	char temp[1024];
	char *s;
	sprintf(temp,"[%s]: %s (%3.3f,%3.3f,%3.3f,%3.3f)",__context,str,q.s,q.n.x,q.n.y,q.n.z);
	s=(char*) malloc(strlen(temp)+1);
	sprintf(s,"%s",temp);
	Log_Physic.Add(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_Log(char *str,CVector u)
{
	char temp[1024];
	char *s;
	sprintf(temp,"[%s]: %s (%3.3f,%3.3f,%3.3f)",__context,str,u.x,u.y,u.z);
	s=(char*) malloc(strlen(temp)+1);
	sprintf(s,"%s",temp);
	Log_Physic.Add(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_Log(char *str)
{
	char temp[1024];
	char *s;
	sprintf(temp,"[%s]: %s",__context,str);
	s=(char*) malloc(strlen(temp)+1);
	sprintf(s,"%s",temp);
	Log_Physic.Add(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_Log(char *str,float f)
{
	char temp[1024];
	char *s;
	sprintf(temp,"[%s]: %s (%3.3f)",__context,str,f);
	s=(char*) malloc(strlen(temp)+1);
	sprintf(s,"%s",temp);
	Log_Physic.Add(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_Log(char *str,int n)
{
	char temp[1024];
	char *s;
	sprintf(temp,"[%s]: %s (%d)",__context,str,n);
	s=(char*) malloc(strlen(temp)+1);
	sprintf(s,"%s",temp);
	Log_Physic.Add(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_SetContext(char *name)
{
	sprintf(__context,"%s",name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PH_WriteLog(char *fname)
{
	char **ps;
	FILE *f;

	f=fopen(fname,"w");
	if(f==NULL) {
	  return;
	}
	ps=Log_Physic.GetFirst();
	while (ps)		
	{
		fprintf(f,"%s\n",*ps);
		ps=Log_Physic.GetNext();
	}

	fclose(f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NORMAL STABILISATION AND CALCULUS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MULTITHREADED_PHYSIC
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
#define WAIT_THREAD() Sleep(0);
#define WAIT_THREAD_IN() Sleep(0);
#define WAIT_THREAD_INACTIVE() Sleep(1);
#else
#ifdef WIN32
#include <process.h>
#define WAIT_THREAD() Sleep(0);
#define WAIT_THREAD_IN() Sleep(0);
#define WAIT_THREAD_INACTIVE() Sleep(1);
#else
#include <pthread.h>
#include <sched.h>
#define WAIT_THREAD() sched_yield();
#define WAIT_THREAD_IN() sched_yield();
#define WAIT_THREAD_INACTIVE() usleep(1000);
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
#include <windows.h>
#ifdef WINDOWS_STORE
extern int WindowsPhoneProcessorNumber;
#else
extern int WindowsPhoneProcessorNumber;
#endif
#else
#ifdef _WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////
// Found on the web !
////////////////////////////////////////////////////////////////////////////////////////
int getNumThreads() {
#ifdef WINDOWS_PHONE
    return WindowsPhoneProcessorNumber-1;
#else
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif MACOS
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PhysicThread;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int index;
	PhysicThread *core;
} PHTH;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
DWORD WINAPI ThreadPhysic(LPVOID param);
#else
#ifdef WIN32
void ThreadPhysic(LPVOID param);
#else
void *ThreadPhysic(void * param);
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PhysicThread
{
public:
	int ph_num[NBTHREADS];
	int ph_num2[NBTHREADS];
	int ph_num3[NBTHREADS];
    int ph_num4[NBTHREADS];
    
	int PHTHREADS[NBTHREADS];
	int PHTHREADSRET[NBTHREADS];
	PHTH PHTHREADSindex[NBTHREADS];

	int PHTHREADSACTIVE;
	CPhysic *PH;
#ifdef WINDOWS_PHONE
	HANDLE phth[NBTHREADS];
	DWORD phthid[NBTHREADS];
#else
#ifdef WIN32
	HANDLE phth[NBTHREADS];
#else
	pthread_t phth[NBTHREADS];
#endif
#endif
	bool PhysicThreadson;
    
    int nbthreads;
    
    float dt,dt_int;
    
    int ns_ph[NBTHREADS][MAX_PHYSIC_OBJECTS_SCENE];
    int ns_ph2[NBTHREADS][MAX_PHYSIC_OBJECTS_SCENE];
    int ns_ph3[NBTHREADS][MAX_PHYSIC_OBJECTS_SCENE];
    int ns_ph4[NBTHREADS][MAX_PHYSIC_OBJECTS_SCENE];
    int phnb_num[NBTHREADS];
    int phnb_num2[NBTHREADS];
    int phnb_num3[NBTHREADS];
    int phnb_num4[NBTHREADS];
    
	PhysicThread() { PH=NULL; PHTHREADSACTIVE=0; PhysicThreadson=false; }

	int ph_full();
	int ph2_full();
	int ph3_full();
    int ph4_full();
	bool ph_busy();
	bool ph2_busy();
	bool ph3_busy();
    bool ph4_busy();

	void KillThreads();
	void InitThreads(CPhysic * ph);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicThread::InitThreads(CPhysic * ph)
{
	int n;

	if (ph->NumProcs<0)
	{
		ph->NumProcs=getNumThreads()/2;
        if (ph->NumProcs<2) ph->NumProcs=2;
        if (ph->NumProcs>MAX_BUF_PH) ph->NumProcs=MAX_BUF_PH;
	}
    
    nbthreads=ph->NumProcs;
    if (nbthreads<1) nbthreads=1;

	for (n=0;n<NBTHREADS;n++)
	{
		PHTHREADSindex[n].index=n;
		PHTHREADSindex[n].core=this;
	}

	PH=ph;
	for (n=0;n<NBTHREADS;n++) ph_num[n]=ph_num2[n]=ph_num3[n]=ph_num4[n]=-1;
    for (n=0;n<NBTHREADS;n++) phnb_num[n]=phnb_num2[n]=phnb_num3[n]=phnb_num4[n]=0;
	for (n=0;n<NBTHREADS;n++) PHTHREADS[n]=1;
	for (n=0;n<NBTHREADS;n++) PHTHREADSRET[n]=0;
    for (n=0;n<NBTHREADS;n++) phth[n]=0;
        
	if (!PhysicThreadson)
	{
		for (n=0;n<PH->NumProcs;n++)
		{
#ifdef WINDOWS_PHONE
			phth[n]=CreateThread(NULL,0,ThreadPhysic,&PHTHREADSindex[n],0,&phthid[n]);
			SetThreadPriority(phth[n],THREAD_PRIORITY_HIGHEST);
#else
#ifdef WIN32
			phth[n]=(HANDLE)_beginthread(ThreadPhysic, 0, &PHTHREADSindex[n]);
#else
            pthread_create(&phth[n],NULL,ThreadPhysic,&PHTHREADSindex[n]);
/*
            int policy;
            struct sched_param param;
            pthread_getschedparam(phth[n], &policy, &param);
            param.sched_priority = sched_get_priority_max(policy);
            pthread_setschedparam(phth[n], policy, &param);
/**/
#endif
#endif
		}
		PhysicThreadson=true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicThread::KillThreads()
{
    int n;
    
	for (n=0;n<PH->NumProcs;n++) PHTHREADS[n]=0;
#if	defined(WINDOWS_PHONE)
#else
    int NB=0;
    while (NB!=PH->NumProcs)
	{
        NB=0;
        for (n=0;n<PH->NumProcs;n++) NB+=PHTHREADSRET[n];
        WAIT_THREAD();
	}
#endif
	PhysicThreadson=false;
}

bool PhysicThread::ph_busy()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num[n]>=0) return true;
	return false;
}

bool PhysicThread::ph2_busy()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num2[n]>=0) return true;
	return false;
}

bool PhysicThread::ph3_busy()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num3[n]>=0) return true;
	return false;
}

bool PhysicThread::ph4_busy()
{
    int n;
    for (n=0;n<PH->NumProcs;n++) if (ph_num4[n]>=0) return true;
    return false;
}


int PhysicThread::ph_full()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num[n]<0) return n;
	return -1;
}

int PhysicThread::ph2_full()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num2[n]<0) return n;
	return -1;
}

int PhysicThread::ph3_full()
{
	int n;
	for (n=0;n<PH->NumProcs;n++) if (ph_num3[n]<0) return n;
	return -1;
}

int PhysicThread::ph4_full()
{
    int n;
    for (n=0;n<PH->NumProcs;n++) if (ph_num4[n]<0) return n;
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
DWORD WINAPI ThreadPhysic(LPVOID param)
#else
#ifdef WIN32
void ThreadPhysic(LPVOID param)
#else
void *ThreadPhysic(void * param)
#endif
#endif
{
	PHTH *th=(PHTH*)param;
	int K=th->index;
    int n;

	while (th->core->PHTHREADS[K])
	{
		if (th->core->PHTHREADSACTIVE)
		{
			if (th->core->ph_num[K]>0)
			{
                for (n=0;n<th->core->phnb_num[K];n++)
                    th->core->PH->ProcessBodyCollisions(th->core->ns_ph[K][n],K);
                
				th->core->ph_num[K]=-1;
			}
			else
#ifdef MULTITHREADED_PHYSIC_STATE
			if (th->core->ph_num2[K]>0)
			{
                for (n=0;n<th->core->phnb_num2[K];n++)
                    th->core->PH->ProcessBodyStateUpdate(th->core->ns_ph2[K][n],K);
                
				th->core->ph_num2[K]=-1;
			}
			else
#endif
			if (th->core->ph_num3[K]>0)
			{
                for (n=0;n<th->core->phnb_num3[K];n++)
                    th->core->PH->ProcessBodyTime(th->core->ns_ph3[K][n],K);

                th->core->ph_num3[K]=-1;
			}
            else
            if (th->core->ph_num4[K]>0)
            {
                for (n=0;n<th->core->phnb_num4[K];n++)
                    th->core->PH->ProcessBaseJoints(th->core->ns_ph4[K][n],th->core->dt,th->core->dt_int);
                
                th->core->ph_num4[K]=-1;
            }
			else WAIT_THREAD_IN();
		}
		else WAIT_THREAD_INACTIVE();
	}

    th->core->PHTHREADSRET[K]=1;

#ifdef WINDOWS_PHONE
	return 0;
#else
#ifdef WIN32
	_endthread();
#else
	pthread_exit(NULL);
#endif
#endif
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ProcessBodyStateUpdate(int n,int buf)
{
	int nn;
	CPhysicObject * op=Array[n];
 
	if (op->statique)
	for (nn=0;nn<nCollisionMeshes;nn++) 
	{ 
		if (CollisionMeshes[nn].obj->update)
		{
			if (IsObjCollidingMesh(n,&CollisionMeshes[nn],0,buf)==1) op->update=1;
		}				
	}				

	if (!op->statique)
	if (((VECTORNORM2(op->Vit)>CONSTANT_STATIC_VELOCITY_SQUARE)||(VECTORNORM2(op->Omega)>CONSTANT_STATIC_TORQUE_SQUARE))||(VECTORNORM2(op->PUSH)>SMALLF))
	{
		for (nn=0;nn<nArray;nn++)
		if (nn!=n)
		{
			unsigned int adr=n+(nn<<MAX_PHYSIC_OBJECTS_SCENE_SHL);

			if (Predict[adr])
			{
				if ((Array[nn]->statique==true)&&((op->Linked==false)||(Array[nn]->Linked==false)||(op->nGroup!=Array[nn]->nGroup)))
				{
					//if (IsPobClosing1CollidingPobClosing2(n,nn,Array[n]->pob,Array[nn]->pob,0,0)) Array[nn]->update=1;
					if (IsObj1CollidingObj2(n,nn,0,buf)) Array[nn]->update=1;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ProcessBodyCollisions(int n,int buf)
{
	int nn,t;
	int res,res2;
	float p,vrel;
	CPhysicObject * op;
	CPhysicObject * op2;
	CVector ua1,ua2,ub1,ub2;
	float friction;
	CVector ddOmega;
	CVector rQ,u,vG,v,dLg,T,Rn,Rt,u1,v1,u2,u3,v2,rQ1,rQ2,pN,N,I;
	float coef1,coef2,ss;
	unsigned int adr;
	STRUCT_EVENT_CONTACT *ev;
	int n_contact_objects,n_contact_env;

	op=Array[n];
	op->Collided=0;
	res=res2=0;

	Contacts[buf].Free();

	// collision moving objects
    // AYE
    if ((Array[n]->statique==false)&&(Array[n]->NoCollidingDynamic==0))
	for (int n_nn=0;n_nn<nArray;n_nn++)
	if (SortedIndices[n_nn].nobj!=n)
	{
		nn=SortedIndices[n_nn].nobj;
		adr=n+(nn<<MAX_PHYSIC_OBJECTS_SCENE_SHL);

		if ((Predict[adr])&&(NoCollision[adr]==0))
		{
			if ((Array[nn]->Linked==false)||(Array[n]->nGroup!=Array[nn]->nGroup)||((Array[n]->nGroup!=Array[nn]->nGroup)&&(Array[n]->LinkedCollision)&&(Array[nn]->LinkedCollision)))
			{
				if (IsObj1CollidingObj2(n,nn,1,buf)==1) res=1;
				else { NoCollision[adr]=NoCollision[nn+(n<<MAX_PHYSIC_OBJECTS_SCENE_SHL)]=1; }
			}
		}
	}

	if (res)
	{
		op->approx++;
		op->Collided+=2;
	}

	n_contact_objects=Contacts[buf].Length();

	// collisions with environnement

	if (op->collide_env)
	{
		if (Array[n]->statique==false)
		{
			for (nn=0;nn<nPlanes;nn++) 
			{ 
				if (Restitution[nn]>=0)
				{
					if (IsObjCollidingPlane(n,nn,1,buf)==1) res2=1; 
				}
			}
			for (nn=0;nn<nCollisionMeshes;nn++)
            {
                if (CollisionMeshes[nn].Restitution>0)
                    if (IsObjCollidingMesh(n,&CollisionMeshes[nn],1,buf)==1) res2=1;
            }
		}
	}

	if (res2) { op->Collided+=1;res=1; }

	n_contact_env=Contacts[buf].Length()-n_contact_objects;

	// collisions
			
	float STATIC_VREL_SEUIL=SMALLF;
	
	if (res)
	{
		vG=op->Vit;

		op->support=NULL;

		ContactStruct * cs=Contacts[buf].GetFirst();
		while (cs)
		{
			if (cs->Types==COLLISION_WITH_OBJECT)
			{
				t=cs->Num;
				N=cs->Normals;
				I=cs->Point;
				ss=DOT(op->Dep,N);
				if (ss<0) op->Dep-=ss*N;
				op2=Array[t];

				if (op2->Pos.y>op->Pos.y) op->support=op2;

				if (op2->tag)
				{
                    if (op2->Linked) op2->ext=1;
                    
					friction=op2->Friction;
					VECTORSUB(rQ1,I,op->Pos);
					VECTORSUB(rQ2,I,op2->Pos);
					CROSSPRODUCT(v1,op->Omega,rQ1);
					VECTORADD(u1,vG,v1);
					CROSSPRODUCT(v2,op2->Omega,rQ2);
					VECTORADD(u2,Array_vG[t],v2);
					VECTORSUB(u,u1,u2);
					DOTPRODUCT(vrel,u,N);
					if (vrel<-STATIC_VREL_SEUIL)
					{
						if (GetEventList)
						{
#ifdef MULTITHREADED_PHYSIC
							ev=EventsTH[buf].InsertLast();
#else
							ev=Events.InsertLast();
#endif
							ev->type=OBJECT_OBJECT;
							ev->p=I; ev->ndx1=n; ev->ndx2=t;
							ev->contact_type=CONTACT_IMPULSE;
							ev->vrel=vrel;
							if (vrel<SLIDING) ev->contact_type|=CONTACT_SLIDE;
						}
						u.x=u.x-vrel*N.x;
						u.y=u.y-vrel*N.y;
						u.z=u.z-vrel*N.z;
						if (VECTORNORM2(u)>TEST_NORM) VECTORNORMALISE(u);
						VECTORMUL(u,u,friction);
						VECTORSUB(ua2,N,u);
						VECTORNORMALISE(ua2);
						VECTORMUL(ua1,ua2,op->InvMass);
						VECTORMUL(ub1,N,op2->InvMass);
						CROSSPRODUCT(v,rQ1,N);
						VECTOR3MATRIX(v,v,op->IIt);
						CROSSPRODUCT(v,v,rQ1);
						coef1=DOT(N,v)+DOT(N,ua1);
						CROSSPRODUCT(v,rQ2,N);
						VECTOR3MATRIX(v,v,op2->IIt);
						CROSSPRODUCT(v,v,rQ2);
						coef2=DOT(ua2,v) + DOT(ua2,ub1);
						p=-(1+op2->Restitution)*vrel/(coef1+coef2);
						VECTORMUL(pN,ua2,p/(n_contact_objects*dt_int));
                        VECTORADD(op->F,op->F,pN);
						CROSSPRODUCT(dLg,rQ1,pN);
						VECTORADD(op->T,op->T,dLg);
					}
				}
				else
				{
					// quasi static object of infinite mass

					friction=op2->Friction;
					VECTORSUB(rQ,I,op->Pos);
					CROSSPRODUCT(v,op->Omega,rQ);
					VECTORADD(u,vG,v);
					DOTPRODUCT(vrel,u,N);
					if ((vrel>-STATIC_VREL_SEUIL)&&(vrel<0.0f))
					{
						op->F.Init(0,0,0);
						op->T.Init(0,0,0);
					}
					else
					if (vrel<-STATIC_VREL_SEUIL)
					{
						u.x=u.x-vrel*N.x;
						u.y=u.y-vrel*N.y;
						u.z=u.z-vrel*N.z;
						if (VECTORNORM2(u)>TEST_NORM) VECTORNORMALISE(u);
						VECTORMUL(u,u,friction);
						VECTORSUB(ua2,N,u);
						VECTORNORMALISE(ua2);
						VECTORMUL(ua1,ua2,op->InvMass);
						CROSSPRODUCT(v,rQ,N);
						VECTOR3MATRIX(v,v,op->IIt);
						CROSSPRODUCT(v,v,rQ);
						coef1=DOT(N,v)+DOT(N,ua1); // ua2 <=> N
						p=-(1+op2->Restitution)*vrel/coef1;
						if (GetEventList)
						{
#ifdef MULTITHREADED_PHYSIC
							ev=EventsTH[buf].InsertLast();
#else
							ev=Events.InsertLast();
#endif
							ev->type=OBJECT_OBJECT;
							ev->p=I; ev->ndx1=n; ev->ndx2=t;
							ev->contact_type=CONTACT_IMPULSE;
							ev->vrel=vrel;
							if (f_abs(vrel)<SLIDING) ev->contact_type|=CONTACT_SLIDE;
						}

						VECTORMUL(pN,ua2,p/(n_contact_objects*dt_int));
						VECTORADD(op->F,op->F,pN);
						CROSSPRODUCT(dLg,rQ,pN);
						VECTORADD(op->T,op->T,dLg);
					}
				}
			}
			else
			{
                int ok=1;
				if (cs->Types==COLLISION_WITH_PLANE)
                {
                    t=cs->Num;
                    if (Restitution[t]<0) ok=0;
                }
                else
                {
                    t=cs->Num + MAX_FACES_PER_OBJECT;
                    if (CollisionMeshes[(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT].Restitution<0) ok=0;
                }

                if (ok)
                {
                    N=cs->Normals;
                    I=cs->Point;
                    DOTPRODUCT(ss,op->Dep,N);
                    if (ss<0) op->Dep-=ss*N;
                    VECTORSUB(rQ,I,op->Pos);
                    CROSSPRODUCT(v,op->Omega,rQ);
                    VECTORADD(u,vG,v);
                    DOTPRODUCT(vrel,u,N);
                    if ((vrel>-STATIC_VREL_SEUIL)&&(vrel<0.0f))
                    {
        //				op->Vit.Init(0,0,0);
        //				op->Omega.Init(0,0,0);
                        op->F.Init(0,0,0);
                        op->T.Init(0,0,0);
                        if (GetEventList)
                        {
#ifdef MULTITHREADED_PHYSIC
                            ev=EventsTH[buf].InsertLast();
#else
                            ev=Events.InsertLast();
#endif
                            if (t<MAX_FACES_PER_OBJECT)
                            {
                                ev->type=OBJECT_PLANE;
                                ev->p=I; ev->ndx1=n; ev->ndx2=t;
                            }
                            else
                            {
                                ev->type=OBJECT_MESH;
                                ev->p=I; ev->ndx1=n;
                                ev->ndx2=(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT;
                            }

                            ev->contact_type=CONTACT_IMPULSE;
                            ev->vrel=vrel;
                            if (f_abs(vrel)<SLIDING) ev->contact_type|=CONTACT_SLIDE;
                        }
                    }
                    else
                    if (vrel<-STATIC_VREL_SEUIL)
                    {
                        if (t<MAX_FACES_PER_OBJECT) friction=Friction[t];
                        else friction=CollisionMeshes[(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT].Friction;
                        u.x=u.x-vrel*N.x;
                        u.y=u.y-vrel*N.y;
                        u.z=u.z-vrel*N.z;
                        if (VECTORNORM2(u)>TEST_NORM) VECTORNORMALISE(u);
                        VECTORMUL(u,u,friction);
                        VECTORSUB(ua2,N,u);
                        VECTORNORMALISE(ua2);
                        VECTORMUL(ua1,ua2,op->InvMass);
                        CROSSPRODUCT(v,rQ,N);
                        VECTOR3MATRIX(v,v,op->IIt);
                        CROSSPRODUCT(v,v,rQ);
                        coef1=DOT(N,v)+DOT(N,ua1);  // ua2 <=> N
                        if (t<MAX_FACES_PER_OBJECT)
                        {
                            p=-(1+Restitution[t])*vrel/coef1;
                            if (GetEventList)
                            {
#ifdef MULTITHREADED_PHYSIC
                                ev=EventsTH[buf].InsertLast();
#else
                                ev=Events.InsertLast();
#endif
                                ev->type=OBJECT_PLANE;
                                ev->p=I; ev->ndx1=n; ev->ndx2=t;
                                ev->contact_type=CONTACT_IMPULSE;
                                ev->vrel=vrel;
                                if (f_abs(vrel)<SLIDING) ev->contact_type|=CONTACT_SLIDE;
                            }
                        }
                        else
                        {
                            p=-(1+CollisionMeshes[(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT].Restitution)*vrel/coef1;
                            if (GetEventList)
                            {
#ifdef MULTITHREADED_PHYSIC
                                ev=EventsTH[buf].InsertLast();
#else
                                ev=Events.InsertLast();
#endif
                                ev->type=OBJECT_MESH;
                                ev->p=I; ev->ndx1=n;
                                ev->ndx2=(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT;
                                ev->vrel=vrel;
                                ev->contact_type=CONTACT_IMPULSE;
                                if (f_abs(vrel)<SLIDING) ev->contact_type|=CONTACT_SLIDE;
                            }
                        }

                        VECTORMUL(pN,ua2,p/(n_contact_env*dt_int));
                        VECTORADD(op->F,op->F,pN);
                        CROSSPRODUCT(dLg,rQ,pN);
                        VECTORADD(op->T,op->T,dLg);
                    }
				}
			}

			cs=Contacts[buf].GetNext();
		}
	}

	if ((op->fluid)&&(op->statique==false))
	{
		// interactions of fluid
		for (int n_nn=0;n_nn<nArray;n_nn++)
		if (SortedIndices[n_nn].nobj!=n)
		{
			nn=SortedIndices[n_nn].nobj;
			if (Array[nn]->fluid)
			{
				op2=Array[nn];
				VECTORSUB(u,op2->Pos,op->Pos);
				float pp=VECTORNORM(u);
				if ((pp<op->Radius*dist)&&(pp>op->Radius+op2->Radius))
				{
					VECTORDIV(u,u,pp);
					VECTORMUL(pN,u,0.035f*(op->viscosity/(pp*dt_int)));
					VECTORADD(op->F,op->F,pN);
				}
			}
		}

		// environment viscosity
		VECTORMUL(pN,op->Vit,(viscosity/dt_int));
		VECTORSUB(op->F,op->F,pN);

		// environment temperature
		u.Init(0,-0.1f,0);
		VECTORMUL(pN,u,((temperature-op->temperature)/dt_int));
		VECTORSUB(op->F,op->F,pN);

	}
	// archim�de
	
    if (op->statique==false)
    {
        for (nn=0;nn<nPlanes;nn++)
        {
            if (Restitution[nn]<0)
            {
                float visco=-Restitution[nn]/10.0f;
                if (visco>1.0f) visco=1.0f;
                float R=op->Radius;
                float ss=Planes[nn].a*op->Pos.x+Planes[nn].b*op->Pos.y+Planes[nn].c*op->Pos.z+Planes[nn].d;
                //ss+=R;
                if (ss>0)
                {
                    CVector ux=op->pob->Ux*op->MOActual;
                    CVector uy=op->pob->Uy*op->MOActual;
                    CVector uz=op->pob->Uz*op->MOActual;
                    float area=R*R;
                    if ((f_abs(ux.y)>f_abs(uy.y))&&(f_abs(ux.y)>f_abs(uz.y))) { area=op->pob->Ly*op->pob->Lz; rQ=uy+uz; }
                    if ((f_abs(uy.y)>f_abs(ux.y))&&(f_abs(uy.y)>f_abs(uz.y))) { area=op->pob->Lx*op->pob->Lz; rQ=ux+uz; }
                    if ((f_abs(uz.y)>f_abs(ux.y))&&(f_abs(uz.y)>f_abs(uy.y))) { area=op->pob->Ly*op->pob->Lx; rQ=ux+uy; }
                    rQ.Normalise();
                    if (ss>R) ss=R;
                    
                    float volum=ss*area*4;
                    VECTORINIT(pN,0,-volum/dt_int,0);
                    VECTORADD(op->F,op->F,pN);
                                    
                    if (op->Vit.Norme()<2) rQ*=op->Vit.Norme()*0.05f*ss/R;
                    else rQ*=0.1f*ss/R;
                    
                    CROSSPRODUCT(dLg,rQ,pN);
                    VECTORADD(op->T,op->T,dLg);
                    op->Omega*=0.999f-0.0075f*visco;
                    op->Vit*=0.999f-0.0075f*visco;
                }
            }
        }
        
        for (nn=0;nn<nCollisionMeshes;nn++)
        {
            if (CollisionMeshes[nn].Restitution<0)
            {
                float visco=-CollisionMeshes[nn].Restitution/10.0f;
                if (visco>1.0f) visco=1.0f;
                float R=op->Radius;
                
                CVector A=op->Pos-CVector(0,100,0);
                CVector B=op->Pos+CVector(0,100,0);
                                
                int coll=0;
                CVector C;
                
                CVector uuu;
                CObject3D *tmp=CollisionMeshes[nn].obj;

                int nf=0;
                while ((coll==0)&&(nf<tmp->nFaces))
                {
                    CFace *F=&tmp->Faces[nf];
                    
                    C.x=F->mp0.x; C.z=F->mp1.x;
                    float dx=F->mp1.y;
                    float dz=F->mp2.y;
                    
                    if ((f_abs(C.x-A.x)<dx)&&(f_abs(C.z-A.z)<dz))
                    if (tmp->IntersectFaceApprox(A,B,F))
                    {
                        tmp->FaceI=nf;
                        VECTORSUB(uuu,tmp->PointI,A)
                        if (VECTORNORM2(uuu)>0) coll=1;
                    }
                    nf++;
                }

                if (coll)
                //if (CollisionMeshes[nn].obj->IntersectObjetApprox(A,B,A))
                {
                    float ss=op->Pos.y-CollisionMeshes[nn].obj->PointI.y;
                    if (ss>0)
                    {
                        ss*=f_abs(CollisionMeshes[nn].obj->PointINorm.y);
                        
                        CVector ux=op->pob->Ux*op->MOActual;
                        CVector uy=op->pob->Uy*op->MOActual;
                        CVector uz=op->pob->Uz*op->MOActual;
                        float area=R*R;
                        if ((f_abs(ux.y)>f_abs(uy.y))&&(f_abs(ux.y)>f_abs(uz.y))) { area=op->pob->Ly*op->pob->Lz; rQ=uy+uz; }
                        if ((f_abs(uy.y)>f_abs(ux.y))&&(f_abs(uy.y)>f_abs(uz.y))) { area=op->pob->Lx*op->pob->Lz; rQ=ux+uz; }
                        if ((f_abs(uz.y)>f_abs(ux.y))&&(f_abs(uz.y)>f_abs(uy.y))) { area=op->pob->Ly*op->pob->Lx; rQ=ux+uy; }
                        rQ.Normalise();
                        if (ss>R) ss=R;
                        
                        float volum=ss*area;
                        //VECTORINIT(pN,0,-volum/dt_int,0);
                        pN=(volum/dt_int)*CollisionMeshes[nn].obj->PointINorm;
                        VECTORADD(op->F,op->F,pN);
                                        
                        if (op->Vit.Norme()<2) rQ*=op->Vit.Norme()*0.05f*ss/R;
                        else rQ*=0.1f*ss/R;
                        
                        CROSSPRODUCT(dLg,rQ,pN);
                        VECTORADD(op->T,op->T,dLg);
                        op->Omega*=0.999f-0.0075f*visco;
                        op->Vit*=0.999f-0.0075f*visco;

                    }
                }
            }
        }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ProcessBodyTime(int n,int buf)
{
	CVector N,u1;
	float dmax;
	float ddt,s1,s2;
	CPhysicObject * op;
	int nn;

	op=Array[n];

	// apply exact collision time to colliding objects
	if ((op->Collided&2)&&(!op->statique)&&(op->support))
	{				
		VECTORSUB(u1,op->Posdt,op->aPos);

		s2=VECTORNORM(u1);
		s1=VECTORNORM(op->Dep);
        
        if (s2<SMALLF2) ddt=0.3f;
        else ddt=s1/s2;
        if (ddt>1.0f) ddt=1.0f;
        
        if (ddt<SMALLF)
        {
            op->Vit.Init(0,0,0);
            op->Omega.Init(0,0,0);
        }
        else ddt=(2.0f+1.5f*ddt)/3.5f;

        if (ddt>=0.0f)
        {
            if (ddt<0.2f) ddt=0.2f;
            VECTORADD(op->Pos,op->aPos,ddt*op->Dep);
            op->Q.Slerp(&op->aQ,&op->Q_dt,ddt);
        }
	}

		// objects colliding environment
#define MULCP 0.05f

	if (Predict[n+(n<<MAX_PHYSIC_OBJECTS_SCENE_SHL)])
	{
		if (op->Collided&1)
		{
			float dmin=SMALLF;

			for (nn=0;nn<nPlanes;nn++)
			{				
				Contacts[buf].Free();
                if (Restitution[nn]>0)
				if (IsObjCollidingPlane(n,nn,1,buf)==1) 
				{
					VECTORINIT(N,0,0,0);
					dmax=0;
					ContactStruct * cs=Contacts[buf].GetFirst();
					while (cs)
					{
						dmax+=f_abs(cs->Distance);
						N.x+=cs->Distance*cs->Normals.x;
						N.y+=cs->Distance*cs->Normals.y;
						N.z+=cs->Distance*cs->Normals.z;

						cs=Contacts[buf].GetNext();
					}

					VECTORNORMALISE(N);
					if (dmax>dmin)
					{
						dmax=MULCP*dmax;
						op->Pos.x+=dmax*N.x;
						op->Pos.y+=dmax*N.y;
						op->Pos.z+=dmax*N.z;
					}
				}
			}

			for (nn=0;nn<nCollisionMeshes;nn++)
			{
				Contacts[buf].Free();
                if (CollisionMeshes[nn].Restitution>0)
				if (IsObjCollidingMesh(n,&CollisionMeshes[nn],1,buf)==1) 
				{
					VECTORINIT(N,0,0,0);
								
					dmax=0;
					ContactStruct * cs=Contacts[buf].GetFirst();
					while (cs)
					{
						dmax+=f_abs(cs->Distance);
						N.x+=cs->Distance*cs->Normals.x;
						N.y+=cs->Distance*cs->Normals.y;
						N.z+=cs->Distance*cs->Normals.z;
						cs=Contacts[buf].GetNext();
					}

					VECTORNORMALISE(N);
					if (dmax>dmin)
					{
						dmax=MULCP*dmax;
						op->Pos.x+=dmax*N.x;
						op->Pos.y+=dmax*N.y;
						op->Pos.z+=dmax*N.z;
					}
				}
			}

			Contacts[buf].Free();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::PrecalculateBaseJointsRelaxation()
{
	int n;
	CPhysicObject * op, * op2;
	CPhysicJoint *joint;
	CVector u;

	for (n=0;n<nBaseJoints;n++)
	{
		joint=BaseJoints[n];
		while (joint)
		{
			if ((joint->type==MOBIL_JOINT)||(joint->type==SPRING_JOINT)||(joint->type==BALL_JOINT)||(joint->type==PIVOT_JOINT))
			{
				if (joint->dbase<0)
				{
					op=joint->Object1;
					op2=joint->Object2;
					u=op->Pos-op2->Pos;
					joint->dbase=VECTORNORM(u);
				}
			}
            
			if ((joint->type==FIXED_JOINT)||(joint->type==FIXED_PIVOT)||(joint->type==FIXED_SPRING))
			{
				if (joint->dbase<0)
				{
					op=joint->Object1;
					u=joint->ptfixe-op->Pos;
					joint->dbase=VECTORNORM(u);
				}
			}
            
			joint=joint->NextJoint;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ProcessBaseJoints(int n,float dt,float dt_int)
{
    CQuaternion dQ;
    int nn;
    int res;
    CPhysicObject * op;
    CPhysicObject * op2;
    CMatrix M,Mt,tmp;
    CVector u,v;
    CMatrix_MxN *Big,*BigB,*resultvect;
    CVector twist_w,twist_v;
    CPhysicJoint *joint;
    int dimbig;

#ifdef LOG_PHYSIC
    PH_Log("Base Joint",n);
#endif
    //Jacobian=BaseJoints[n]->BigJacobian(-1);
    //tJacobian=Jacobian->TransposeSign();

    int proceed=0;
    int nbnb=BaseJoints[n]->NbreOp();
    
	Big=new CMatrix_MxN;
	BigB=new CMatrix_MxN;

    for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
    {
        op=BaseJoints[n]->Objs[nn];
        op->Lg=op->Omega;
        op->Lg.Normalise();
        //float f=VECTORNORM(op->Lg);
        //if (f>SMALLF2) { VECTORDIV(op->Lg,op->Lg,f); }
        //else op->Lg.Init(0,0,0);
        op->approx=0;
        if (!op->Fixe) proceed=1;
    }
        
    if (proceed)
    {
        dimbig=BaseJoints[n]->NbreOP*6 + BaseJoints[n]->total_len;

        Big->Init(dimbig,dimbig);
        BigB->Init(dimbig,1);
        
        if (BaseJoints[n]->relax)
        {
            for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
            {
                op=BaseJoints[n]->Objs[nn];
                
                if (!op->Fixe)
                {
                    op->aPos=op->Pos;
                    op->aVit=op->Vit;
                    op->aOmega=op->Omega;
                    op->aQ=op->Q;
                    
                    VECTORMUL(u,op->Lg,DOT(op->Omega,op->Lg));
                    VECTORSUB(v,op->Omega,u);
                    v=((1.0f - op->LinkFriction)*u +v);
                    twist_w=v*op->It + dt*op->T;
                    
                    if (op->wind_influence) twist_v=op->Mass*op->Vit + dt*(op->F+DYNAMIC_WIND_FORCE/dt_int);
                    else twist_v=op->Mass*op->Vit + dt*op->F;
                    
                    op->Pos+=dt*op->Vit;
                    op->Vit=twist_v*op->InvMass;
                    
                    op->Posdt=op->Pos;
                    dQ=(op->Omega*op->Q);
                    op->Q=op->Q+((dt*0.5f)*dQ);
                    op->Q.Normalise();
                    if (op->wait<=static_method) op->wait=static_method;

                    op->Omega=twist_w*op->IIt;
                    
                    op->Vit_dt=op->Vit;
                    op->Q_dt=op->Q;
                    op->Omega_dt=op->Omega;
                    op->Dep=op->Pos-op->aPos;
                }
            }
        }
        else
        {
            // Friction
            joint=BaseJoints[n];
            while (joint)
            {
                if (joint->angle_limit>0)
                {
                    op=joint->Object1;
                    op2=joint->Object2;
                    
                    CVector ang;
                    CVector vv1,vv2;
                    CVector _vv1,_vv2;
                    
                    VECTOR3MATRIX(vv1,joint->r1,op->MOActual);
                    VECTOR3MATRIX(vv2,joint->r2,op2->MOActual);
                    
                    CROSSPRODUCT(ang,vv2,vv1);
                    
                    VECTOR3INIT(_vv1,vv1);
                    VECTOR3INITNEG(_vv2,vv2);
                    
                    float nv1=VECTORNORM(_vv1);
                    float nv2=VECTORNORM(_vv2);
                    
                    if ((nv1>SMALLF)&&(nv2>SMALLF))
                    {
                        VECTORDIV(_vv1,_vv1,nv1);
                        VECTORDIV(_vv2,_vv2,nv2);
                        
                        float angle=-acosf(DOT(_vv1,_vv2));
                        
                        if (f_abs(angle)>joint->angle_limit)
                        {
                            VECTORNORMALISE(ang);
                            if (angle>0)
                            {
                                float alpha=joint->angle_limit-angle;
                                VECTORMUL(ang,ang,alpha);
                                VECTORSUB(op->Lg,op->Lg,ang);
                            }
                            else
                            {
                                float alpha=joint->angle_limit-f_abs(angle);
                                VECTORMUL(ang,ang,alpha);
                                VECTORADD(op->Lg,op->Lg,ang);
                            }
                        }
                    }
                }
                
                joint=joint->NextJoint;
            }
            
            for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
            {
                op=BaseJoints[n]->Objs[nn];
                
                int nn6=nn*6;			
                
                if (!op->Fixe)
                {
                    int __base=nn6+Big->n*nn6;
                    
                    Big->a[__base + 0]=op->It.a[0][0];
                    Big->a[__base + 1]=op->It.a[1][0];
                    Big->a[__base + 2]=op->It.a[2][0];
                    __base+=Big->n;
                    Big->a[__base + 0]=op->It.a[0][1];
                    Big->a[__base + 1]=op->It.a[1][1];
                    Big->a[__base + 2]=op->It.a[2][1];
                    __base+=Big->n;
                    Big->a[__base + 0]=op->It.a[0][2];
                    Big->a[__base + 1]=op->It.a[1][2];
                    Big->a[__base + 2]=op->It.a[2][2];
                    __base+=Big->n;
                    Big->a[__base + 3]=op->Mass;
                    __base+=Big->n;
                    Big->a[__base + 4]=op->Mass;
                    __base+=Big->n;
                    Big->a[__base + 5]=op->Mass;
                    
                    VECTORMUL(u,op->Lg,DOT(op->Omega,op->Lg));
                    VECTORSUB(v,op->Omega,u);
                    v=((1.0f - op->LinkFriction)*u +v);
                    twist_w=v*op->It + dt*op->T;
                    
                    if (op->wind_influence) twist_v=op->Mass*op->Vit + dt*(op->F+DYNAMIC_WIND_FORCE/dt_int);
                    else twist_v=op->Mass*op->Vit + dt*op->F;
                    
                    BigB->a[nn6 + 0]=twist_w.x;
                    BigB->a[nn6 + 1]=twist_w.y;
                    BigB->a[nn6 + 2]=twist_w.z;
                    BigB->a[nn6 + 3]=twist_v.x;
                    BigB->a[nn6 + 4]=twist_v.y;
                    BigB->a[nn6 + 5]=twist_v.z;
                }
                else
                {
                    int __base=nn6+Big->n*nn6;

                    Big->a[__base + 0]=1.0f;
                    __base+=Big->n;
                    Big->a[__base + 1]=1.0f;
                    __base+=Big->n;
                    Big->a[__base + 2]=1.0f;
                    __base+=Big->n;
                    Big->a[__base + 3]=1.0f;
                    __base+=Big->n;
                    Big->a[__base + 4]=1.0f;
                    __base+=Big->n;
                    Big->a[__base + 5]=1.0f;
                    
                    BigB->a[nn6 + 0]=0;
                    BigB->a[nn6 + 1]=0;
                    BigB->a[nn6 + 2]=0;
                    BigB->a[nn6 + 3]=0;
                    BigB->a[nn6 + 4]=0;
                    BigB->a[nn6 + 5]=0;
                }
            }
            
            BaseJoints[n]->BigJacobianPatch(Big,BaseJoints[n]->nObjs*6,-1);
            
            resultvect=Big->ResolveAXB(BigB);
            
            for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
            {
                int nn6=nn*6;
                op=BaseJoints[n]->Objs[nn];
                if (!op->Fixe)
                {
                    op->statique=false;
                    twist_w.x=resultvect->a[nn6 + 0];
                    twist_w.y=resultvect->a[nn6 + 1];
                    twist_w.z=resultvect->a[nn6 + 2];
                    twist_v.x=resultvect->a[nn6 + 3];
                    twist_v.y=resultvect->a[nn6 + 4];
                    twist_v.z=resultvect->a[nn6 + 5];
                    VECTORADD(op->Pos,op->Pos,dt*op->Vit);
                    dQ=(op->Omega*op->Q);
                    op->Q=op->Q + ((dt/2)*dQ);
                    op->Q.Normalise();
                    op->Omega=twist_w;
                    op->Vit=twist_v;
#ifdef LOG_PHYSIC
                    PH_Log("        Object",nn);
                    PH_Log("        Pos",op->Pos);
                    PH_Log("        Vit",op->Vit);
                    PH_Log("        W",op->Omega);
#endif
                }
            }

            delete resultvect;
        }
        
        BigB->Zero();
        
        for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
        {
            op=BaseJoints[n]->Objs[nn];
            if (!op->Fixe)
            {
                M=op->ActualOrientationMatrix();
                Mt.Transpose(M);
                op->MActual=op->MOActual=M;
                op->MActual.Translation(op->Pos.x,op->Pos.y,op->Pos.z);
                
                MATRIXMATRIX(tmp,Mt,op->Inv_I);
                MATRIXMATRIX(op->IIt,tmp,M);
                
                MATRIXMATRIX(tmp,Mt,op->I);
                MATRIXMATRIX(op->It,tmp,M);
            }
        }
        
		if (BaseJoints[n]->relaxation)
		{
			for (int it=0;it<16;it++)
			{
				float dbase,d;
				float K=0.1f;

				int nbj=0;
				joint=BaseJoints[n];
				while (joint)
				{
					if ((joint->type==MOBIL_JOINT)||(joint->type==SPRING_JOINT)||(joint->type==BALL_JOINT)||(joint->type==PIVOT_JOINT))
					{
						op=joint->Object1;
						op2=joint->Object2;                
						u=op2->Pos-op->Pos;
						d=VECTORNORM(u);
						dbase=joint->dbase;
						VECTORNORMALISE(u);
						op->Pos+=K*((d-dbase)/3)*u;
						op2->Pos-=K*((d-dbase)/3)*u;
					}
            
					if ((joint->type==FIXED_JOINT)||(joint->type==FIXED_PIVOT)||(joint->type==FIXED_SPRING))
					{
						op=joint->Object1;
						u=joint->ptfixe-op->Pos;
						d=VECTORNORM(u);
						dbase=joint->dbase;
						VECTORNORMALISE(u);
						op->Pos+=K*((d-dbase)/2)*u;
						nbj++;
					}
            
					joint=joint->NextJoint;
				}
			}
		}
		else
		{
			joint=BaseJoints[n];
			while (joint)
			{
				int offsetjoint=BaseJoints[n]->nObjs*6+BaseJoints[n]->jacobians_indices[joint->num];

				if (joint->type==MOBIL_JOINT)
				{
					op=joint->Object1;
					op2=joint->Object2;
                
					u=op->Pos+joint->r1*op->MOActual -(op2->Pos +joint->r2*op2->MOActual);

					BigB->a[offsetjoint+0]=-u.x;
					BigB->a[offsetjoint+1]=-u.y;
					BigB->a[offsetjoint+2]=-u.z;
                
					BigB->a[offsetjoint+3]=0;
				}
            
				if (joint->type==SPRING_JOINT)
				{
					op=joint->Object1;
					op2=joint->Object2;
                
					u=op->Pos+joint->r1*op->MOActual -(op2->Pos +joint->r2*op2->MOActual);
                
					float CC=f_abs(joint->raideur*joint->dL);
                
					BigB->a[offsetjoint+0]=-u.x*CC;
					BigB->a[offsetjoint+1]=-u.y*CC;
					BigB->a[offsetjoint+2]=-u.z*CC;
                
					BigB->a[offsetjoint+3]=0;
				}
            
				if (joint->type==BALL_JOINT)
				{
					op=joint->Object1;
					op2=joint->Object2;
                
					u=op->Pos+joint->r1*op->MOActual -(op2->Pos +joint->r2*op2->MOActual);
                
					BigB->a[offsetjoint+0]=-u.x;
					BigB->a[offsetjoint+1]=-u.y;
					BigB->a[offsetjoint+2]=-u.z;
				}
            
				if (joint->type==PIVOT_JOINT)
				{
					op=joint->Object1;
					op2=joint->Object2;
                
					u=op->Pos+joint->r1*op->MOActual -(op2->Pos +joint->r2*op2->MOActual);

					BigB->a[offsetjoint+0]=-u.x;
					BigB->a[offsetjoint+1]=-u.y;
					BigB->a[offsetjoint+2]=-u.z;
					//BigB->a[offsetjoint+3]=0;
					//BigB->a[offsetjoint+4]=0;
				}
            
				if (joint->type==FIXED_JOINT)
				{
					op=joint->Object1;
					u=joint->ptfixe -(op->Pos +joint->r1*op->MOActual);

					BigB->a[offsetjoint+0]=u.x;
					BigB->a[offsetjoint+1]=u.y;
					BigB->a[offsetjoint+2]=u.z;
				}
            
				if (joint->type==FIXED_PIVOT)
				{
					op=joint->Object1;
					u=joint->ptfixe -(op->Pos +joint->r1*op->MOActual);

					BigB->a[offsetjoint+0]=u.x;
					BigB->a[offsetjoint+1]=u.y;
					BigB->a[offsetjoint+2]=u.z;
				}
            
				if (joint->type==FIXED_SPRING)
				{
					op=joint->Object1;
					u=joint->ptfixe -(op->Pos +joint->r1*op->MOActual);
                
					float CC=0.02f*joint->raideur*joint->dL*(50.0f/N_ITERATIONS_tmp);

					BigB->a[offsetjoint+0]=-u.x*CC;
					BigB->a[offsetjoint+1]=-u.y*CC;
					BigB->a[offsetjoint+2]=-u.z*CC;
				}
            
				joint=joint->NextJoint;
			}

			//if (BaseJoints[n]->relax)
			Big->Zero();
			for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
			{
				op=BaseJoints[n]->Objs[nn];
            
				int nn6=nn*6;

				if (!op->Fixe)
				{
					int __base=nn6+Big->n*nn6;

					Big->a[__base + 0]=op->It.a[0][0];
					Big->a[__base + 1]=op->It.a[1][0];
					Big->a[__base + 2]=op->It.a[2][0];
					__base+=Big->n;
					Big->a[__base + 0]=op->It.a[0][1];
					Big->a[__base + 1]=op->It.a[1][1];
					Big->a[__base + 2]=op->It.a[2][1];
					__base+=Big->n;
					Big->a[__base + 0]=op->It.a[0][2];
					Big->a[__base + 1]=op->It.a[1][2];
					Big->a[__base + 2]=op->It.a[2][2];
					__base+=Big->n;
					Big->a[__base + 3]=op->Mass;
					__base+=Big->n;
					Big->a[__base + 4]=op->Mass;
					__base+=Big->n;
					Big->a[__base + 5]=op->Mass;
                
				}
				else
				{
					int __base=nn6+Big->n*nn6;
                
					Big->a[__base + 0]=1.0f;
					__base+=Big->n;
					Big->a[__base + 1]=1.0f;
					__base+=Big->n;
					Big->a[__base + 2]=1.0f;
					__base+=Big->n;
					Big->a[__base + 3]=1.0f;
					__base+=Big->n;
					Big->a[__base + 4]=1.0f;
					__base+=Big->n;
					Big->a[__base + 5]=1.0f;
				}
			}
        
			BaseJoints[n]->BigJacobianPatch(Big,BaseJoints[n]->nObjs*6,-1);

			resultvect=Big->ResolveAXB(BigB);
        
			for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
			{
				op=BaseJoints[n]->Objs[nn];
            
				int nn6=nn*6;
            
				if ((!op->Fixe)&&(op->approx==0))
				{
					twist_w.x=resultvect->a[nn6 + 0];
					twist_w.y=resultvect->a[nn6 + 1];
					twist_w.z=resultvect->a[nn6 + 2];
					twist_v.x=resultvect->a[nn6 + 3];
					twist_v.y=resultvect->a[nn6 + 4];
					twist_v.z=resultvect->a[nn6 + 5];
					VECTORADD(op->Pos,op->Pos,twist_v);
					dQ=((twist_w)*op->Q);
					op->Q=op->Q + (0.5f*dQ);
					op->Q.Normalise();
                
#ifdef LOG_PHYSIC
					PH_Log("        Object",nn);
					PH_Log("        Pos",op->Pos);
					PH_Log("        Vit",op->Vit);
					PH_Log("        W",op->Omega);
#endif
				}
			}
        
			delete resultvect;
		}        
        BigB->Free();
        Big->Free();
    }
    
    for (nn=0;nn<BaseJoints[n]->nObjs;nn++)
    {
        float K=SMALLF3;
        float K0=0.1f;
        float K1=0.1f;
        
        op=BaseJoints[n]->Objs[nn];
        op->ext=0;
        
        if (!op->Fixe)
        {
            res=0;
            if ((VECTORNORM(op->Vit)>0.01f)||(VECTORNORM(op->Omega)>0.01f)) res=1;
            
            if ((res==0)&&(op->wind_influence==false))
            {
                if (op->wait>0) op->wait--;
                else op->statique=true;
            }
        }
    }

	delete Big;
	delete BigB;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::Calculate(CPhysicObject *op,float dt)
{
	CQuaternion dQ;

	op->aPos=op->Pos;
	op->aVit=op->Vit;
	op->aOmega=op->Omega;
	op->aQ=op->Q;

	int res=0;

	if ((VECTORNORM2(CVector(op->Vit))>CONSTANT_STATIC_VELOCITY_SQUARE)||
		(VECTORNORM2(CVector(op->Omega))>CONSTANT_STATIC_TORQUE_SQUARE)||
		(VECTORNORM2(op->PUSH)>SMALLF)) res=1;

	if (res==0)
	{
		if (op->wait>0)
		{
			op->wait--;

			op->statique=false;
			op->Pos+=dt*op->Vit;
			op->Vit+=dt*op->F*op->InvMass;
			op->Posdt=op->Pos;
			dQ=(op->Omega*op->Q);
			op->Q=op->Q+((dt*0.5f)*dQ);
			op->Q.Normalise();

			VECTOR3MATRIX(op->dOmega,op->T,op->IIt);
			op->Omega+=(dt)*(op->dOmega);

		}
		else
		{
			op->F.Init(0,0,0);
			op->T.Init(0,0,0);
			op->Vit.Init(0,0,0);
			op->Omega.Init(0,0,0);
			op->Posdt=op->Pos;
			op->statique=true;
		}
	}
	else
	{
		if (op->statique)
		{
			op->F.Init(0,0,0);
			op->T.Init(0,0,0);
			op->Vit.Init(0,0,0);
			op->Omega.Init(0,0,0);

			op->Posdt=op->Pos;
			op->statique=false;
		}
		else
		{
			op->statique=false;
						
			op->Pos+=dt*op->Vit;
			op->Vit+=dt*op->F*op->InvMass;
			op->Posdt=op->Pos;
			dQ=(op->Omega*op->Q);
			op->Q=op->Q+((dt*0.5f)*dQ);
			op->Q.Normalise();
			if (op->wait<=static_method) op->wait=static_method;

			VECTOR3MATRIX(op->dOmega,op->T,op->IIt);
			op->Omega+=(dt)*(op->dOmega);
		}
	}
	op->Vit_dt=op->Vit;
	op->Q_dt=op->Q;
	op->Omega_dt=op->Omega;
	op->Dep=op->Pos-op->aPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::SetConstantsSPH(float radius)
{
	radiusSPH=radius;
	radiusSPH2=radius*radius;
	volumeSPH=1.0f/(PI*powf(radius,4)/24.0f);
	scaleSPH=24.0f/(powf(radius,4)*PI);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateDensityParticle(int n)
{
	float d=0.0f;

	int infx,supx;
	int infy,supy;
	int infz,supz;

	infx=ParticlesArray[n].xx-dcellx; supx=ParticlesArray[n].xx+dcellx+1;
	infy=ParticlesArray[n].yy-dcelly; supy=ParticlesArray[n].yy+dcelly+1;
	infz=ParticlesArray[n].zz-dcellz; supz=ParticlesArray[n].zz+dcellz+1;

	if (infx<0) infx=0;
	if (infy<0) infy=0;
	if (infz<0) infz=0;

	if (supx>particlesboundx) supx=particlesboundx;
	if (supy>particlesboundy) supy=particlesboundy;
	if (supz>particlesboundz) supz=particlesboundz;

	int ofsz=particlesboundy*infz;

	for (int z=infz;z<supz;z++)
	{		
		int ofsy=particlesboundx*(infy + ofsz);

		for (int y=infy;y<supy;y++)
		{
			for (int x=infx;x<supx;x++)
			{
				int ofs=x + ofsy;

				CElement <int> * el=particlesvoxel[ofs].First;
				while (el)
				{
					int pi=el->data;
					if (pi!=n)
					{
						float dx=ParticlesArray[n].Pos.x - ParticlesArray[pi].Pos.x;
						float dy=ParticlesArray[n].Pos.y - ParticlesArray[pi].Pos.y;
						float dz=ParticlesArray[n].Pos.z - ParticlesArray[pi].Pos.z;
						float dist=dx*dx+dy*dy+dz*dz;
						if (dist<radiusSPH2)
						{
							dist=sqrtf(dist);
							float influence=(radiusSPH-dist)*volumeSPH;
							d+=influence*ParticlesArray[n].Mass;
						}
					}
					el=el->next;
				}
			}
			ofsy+=particlesboundx;
		}
		ofsz+=particlesboundy;
	}

	ParticlesArray[n].density=d;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateGradientParticle(int n)
{
	CVector grad(0,0,0);
	CVector dir;

	int infx,supx;
	int infy,supy;
	int infz,supz;

	infx=ParticlesArray[n].xx-dcellx; supx=ParticlesArray[n].xx+dcellx+1;
	infy=ParticlesArray[n].yy-dcelly; supy=ParticlesArray[n].yy+dcelly+1;
	infz=ParticlesArray[n].zz-dcellz; supz=ParticlesArray[n].zz+dcellz+1;

	if (infx<0) infx=0;
	if (infy<0) infy=0;
	if (infz<0) infz=0;

	if (supx>particlesboundx) supx=particlesboundx;
	if (supy>particlesboundy) supy=particlesboundy;
	if (supz>particlesboundz) supz=particlesboundz;

	int ofsz=particlesboundy*infz;

	for (int z=infz;z<supz;z++)
	{		
		int ofsy=particlesboundx*(infy + ofsz);

		for (int y=infy;y<supy;y++)
		{
			for (int x=infx;x<supx;x++)
			{
				int ofs=x + ofsy;

				CElement <int> * el=particlesvoxel[ofs].First;
				while (el)
				{
					int pi=el->data;
					if (pi!=n)
					{
						VECTORSUB(dir,ParticlesArray[n].Pos,ParticlesArray[pi].Pos);
						float dist=VECTORNORM2(dir);
						if (dist<radiusSPH2)
						{
							dist=sqrtf(dist);
							float slope=(radiusSPH-dist)*scaleSPH/dist;
							if (ParticlesArray[pi].density>0) grad+=dir*slope*ParticlesArray[n].Mass/ParticlesArray[pi].density;
						}
					}
					el=el->next;
				}
			}
			ofsy+=particlesboundx;
		}
		ofsz+=particlesboundy;
	}

	ParticlesArray[n].grad=grad;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ConvertDensitytoPresure(density) ((density-targetdensity)*presuremul)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateViscosityParticle(int n)
{
	CVector f(0,0,0);
	CVector uu;

	int infx,supx;
	int infy,supy;
	int infz,supz;

	infx=ParticlesArray[n].xx-dcellx; supx=ParticlesArray[n].xx+dcellx+1;
	infy=ParticlesArray[n].yy-dcelly; supy=ParticlesArray[n].yy+dcelly+1;
	infz=ParticlesArray[n].zz-dcellz; supz=ParticlesArray[n].zz+dcellz+1;

	if (infx<0) infx=0;
	if (infy<0) infy=0;
	if (infz<0) infz=0;

	if (supx>particlesboundx) supx=particlesboundx;
	if (supy>particlesboundy) supy=particlesboundy;
	if (supz>particlesboundz) supz=particlesboundz;

	int ofsz=particlesboundy*infz;

	for (int z=infz;z<supz;z++)
	{		
		int ofsy=particlesboundx*(infy + ofsz);

		for (int y=infy;y<supy;y++)
		{
			for (int x=infx;x<supx;x++)
			{
				int ofs=x + ofsy;

				CElement <int> * el=particlesvoxel[ofs].First;
				while (el)
				{
					int pi=el->data;
					if (pi!=n)
					{
						float dx=ParticlesArray[n].Pos.x - ParticlesArray[pi].Pos.x;
						float dy=ParticlesArray[n].Pos.y - ParticlesArray[pi].Pos.y;
						float dz=ParticlesArray[n].Pos.z - ParticlesArray[pi].Pos.z;
						float dist=dx*dx+dy*dy+dz*dz;
						if (dist<radiusSPH2)
						{
							dist=sqrtf(dist);
							float influence=(radiusSPH-dist)*scaleSPH;
							VECTORSUB(uu,ParticlesArray[pi].Vit,ParticlesArray[n].Vit);
							f+=uu*influence;
						}
					}
					el=el->next;
				}
			}
			ofsy+=particlesboundx;
		}
		ofsz+=particlesboundy;
	}

	ParticlesArray[n].viscosity=f*ParticlesArray[n].strength;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MULTITHREADED_PHYSIC
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PARTICULES_NBTH 4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PARTICLES_DENSITY	    1
#define PARTICLES_GRADIENT      2
#define PARTICLES_VISCOSITY     3

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
DWORD WINAPI ThreadParticlesPhysic(LPVOID param)
#else
#ifdef WIN32
void ThreadParticlesPhysic(LPVOID param)
#else
void *ThreadParticlesPhysic(void * param)
#endif
#endif
{
	int n;
	dataparticulesthreads *th=(dataparticulesthreads*)param;

	while (th->phy->PARTICULES_ACT>=0)
	{
		if (th->phy->PARTICULES_ACT==1)
		{
			if (th->task==1)
			{
				for (n=th->start;n<th->end;n++) th->phy->CalculateDensityParticle(n);
			}
            else
            if (th->task==2)
            {
                for (n=th->start;n<th->end;n++) th->phy->CalculateGradientParticle(n);
            }
            else
            if (th->task==3)
            {
                for (n=th->start;n<th->end;n++) th->phy->CalculateViscosityParticle(n);
            }
			th->task=0;
		}
		else WAIT_THREAD();
	}

#ifdef WINDOWS_PHONE
	return 0;
#else
#ifdef WIN32
	_endthread();
#else
	pthread_exit(NULL);
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ParticlesThreadsEnd()
{
	PARTICULES_ACT=-1;
	PARTICULES_ACT2=-1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ParticlesDO(int task)
{
	int n;
	PARTICULES_ACT=1;

	for (n=0;n<PARTICULES_NBTH;n++) particlesthreaddata[n].task=task;

	int cc=0;
	for (n=0;n<PARTICULES_NBTH;n++) cc+=particlesthreaddata[n].task;
	while (cc>0)
	{
		WAIT_THREAD();
		cc=0;
		for (n=0;n<PARTICULES_NBTH;n++) cc+=particlesthreaddata[n].task;		
	}

	PARTICULES_ACT=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::ParticlesThreads()
{
#ifdef WINDOWS_PHONE
	HANDLE th;
	DWORD thid;
#else
#ifdef WIN32
	HANDLE th;
#else
	pthread_t th;
#endif
#endif
	
	PARTICULES_ACT=0;

	for (int n=0;n<PARTICULES_NBTH;n++)
	{
		int a=(n*nbParticlesArray)/PARTICULES_NBTH;
		int b=((n+1)*nbParticlesArray)/PARTICULES_NBTH;
		particlesthreaddata[n].start=a;
		particlesthreaddata[n].end=b;
		particlesthreaddata[n].task=666;
		particlesthreaddata[n].phy=this;

#ifdef WINDOWS_PHONE
		th=CreateThread(NULL,0,ThreadParticlesPhysic,&particlesthreaddata[n],0,&thid);
		SetThreadPriority(th,THREAD_PRIORITY_HIGHEST);
#else
#ifdef WIN32
		th=(HANDLE)_beginthread(ThreadParticlesPhysic, 0, &particlesthreaddata[n]);
#else
	    pthread_create(&th,NULL,ThreadParticlesPhysic,&particlesthreaddata[n]);
#endif
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::CalculateSceneNormal()
{
	CQuaternion dQ;
	int n;
	int nn,n1,n2,n_n;
	int t,n0;
	float dt;
	CPhysicObject * op;
	CPhysicObject * op2;
	CMatrix M,Mt,M0,tmp;
	float y,r,p,vrel;
	CVector dLg,u,u1,rQ,v,P,N,calc;
	CPhysicJoint *joint;
	CVector ua1,ua2,ub1,ub2;
	float friction;
	CVector ddOmega;
	CVector vG,T,Rn,Rt,v1,u2,u3,v2,rQ1,rQ2,pN,I;
	float coef1,coef2;

	PrecalculateBaseJointsRelaxation();

    CONSTANT_STATIC_VELOCITY_SQUARE=CONSTANT_STATIC_VELOCITY*CONSTANT_STATIC_VELOCITY;
    CONSTANT_STATIC_TORQUE_SQUARE=CONSTANT_STATIC_TORQUE*CONSTANT_STATIC_TORQUE;

	dt=DT/N_ITERATIONS;
	N_ITERATIONS_tmp=N_ITERATIONS;

    for (nn=0;nn<nCollisionMeshes;nn++)
    {
        ContactsParticles[nn].is=false;
        ContactsParticles[nn].iter=N_ITERATIONS_tmp;
        ContactsParticles[nn].nb=0;
        ContactsParticles[nn].mini.Init(100000,100000,100000);
        ContactsParticles[nn].maxi.Init(-100000,-100000,-100000);
    }
        
    memset(Predict,0,MAX_PHYSIC_OBJECTS_SCENE*MAX_PHYSIC_OBJECTS_SCENE);

	if (nbParticlesArray>0)
	{
		GPmin.Init(10000,10000,10000);
		GPmax.Init(-10000,-10000,-10000);

		if (particlesvoxel==NULL) particlesvoxel=new CList<int>[particlesboundx*particlesboundy*particlesboundz];

		for (n=0;n<nbParticlesArray;n++)
			if (ParticlesArray[n].active)
            {
                if (ParticlesArray[n].Pos.x>GPmax.x) GPmax.x=ParticlesArray[n].Pos.x;
                if (ParticlesArray[n].Pos.y>GPmax.y) GPmax.y=ParticlesArray[n].Pos.y;
                if (ParticlesArray[n].Pos.z>GPmax.z) GPmax.z=ParticlesArray[n].Pos.z;

                if (ParticlesArray[n].Pos.x<GPmin.x) GPmin.x=ParticlesArray[n].Pos.x;
                if (ParticlesArray[n].Pos.y<GPmin.y) GPmin.y=ParticlesArray[n].Pos.y;
                if (ParticlesArray[n].Pos.z<GPmin.z) GPmin.z=ParticlesArray[n].Pos.z;
            }
        
        float KG=4.0f;
        
        GPmax.x+=KG;
        GPmax.y+=KG;
        GPmax.z+=KG;
        
        GPmin.x-=KG;
        GPmin.y-=KG;
        GPmin.z-=KG;

		GPcell.x=(GPmax.x-GPmin.x)/particlesboundx;
		GPcell.y=(GPmax.y-GPmin.y)/particlesboundy;
		GPcell.z=(GPmax.z-GPmin.z)/particlesboundz;

		dcellx=(int)(radiusSPH/GPcell.x);
		dcelly=(int)(radiusSPH/GPcell.y);
		dcellz=(int)(radiusSPH/GPcell.z);

		float ddx=particlesboundx/(GPmax.x-GPmin.x);
		float ddy=particlesboundy/(GPmax.y-GPmin.y);
		float ddz=particlesboundz/(GPmax.z-GPmin.z);

		int dim3=particlesboundx*particlesboundy*particlesboundz;

		for (n=0;n<nbParticlesArray;n++)
		if (ParticlesArray[n].active)
		{
			int ofs=-1;
			ParticlesArray[n].xx=(int) ((ParticlesArray[n].Pos.x-GPmin.x)*ddx);
			ParticlesArray[n].yy=(int) ((ParticlesArray[n].Pos.y-GPmin.y)*ddy);
			ParticlesArray[n].zz=(int) ((ParticlesArray[n].Pos.z-GPmin.z)*ddz);

			ofs=ParticlesArray[n].xx + particlesboundx*(ParticlesArray[n].yy + particlesboundy*ParticlesArray[n].zz);

			if ((ofs>=0)&&(ofs<dim3)) particlesvoxel[ofs].Add(n);
					
			ParticlesArray[n].ofs=ofs;
		}
		else ParticlesArray[n].ofs=-1;

#ifdef MULTITHREADED_PHYSIC
		ParticlesThreads();
#endif
	}			

#ifdef LOG_PHYSIC
	PH_SetContext("Settings");
#endif
    y_max=-10000000;
	for (n=0;n<nCollisionMeshes;n++) 
	{
		if ((!CollisionMeshes[n].statique)||(nbParticlesArray>0)||(CollisionMeshes[n].boundsinit==false)) CollisionMeshes[n].Actualise(n+1);
		CollisionMeshes[n].bCoo=CollisionMeshes[n].obj->Coo;
		CollisionMeshes[n].bRot=CollisionMeshes[n].obj->Rot;
        if (y_max<CollisionMeshes[n].maxi.y) y_max=CollisionMeshes[n].maxi.y;
	}
    
    if (y_max<-10000) y_max=0;
    y_max+=20.0f;
    
	Events.Free();
#ifdef MULTITHREADED_PHYSIC

	if (PhysicMultithreadToggle)
		for (n=0;n<NBTHREADS;n++) EventsTH[n].Free();
#endif
/*
	if (scale_time)
	{
		int b=1;
		while ((b)&&(N_ITERATIONS_tmp<K_MUL_ITERATIONS*N_ITERATIONS))
		{
			b=0;
			for (n=0;n<nArray;n++)
			{ 
				if (Array[n]->Pos.y<y_max)
				{
					if (IsObjVelocityTooHigh(n,dt*N_ITERATIONS*0.9f,Array[n]->Vit,Array[n]->Omega)) b++;
				}
			}
			if (b) { N_ITERATIONS_tmp+=2; dt=DT/N_ITERATIONS_tmp; }
		}
	}
*/
	dt_int=dt;

#ifdef LOG_PHYSIC
	PH_Log("Physic Environment Calculation");
	PH_Log("Iterations",N_ITERATIONS);
	PH_Log("Iterations tmp",N_ITERATIONS_tmp);
	PH_Log("DT",DT);
	PH_Log("dt",dt);
	PH_Log("dt_int",dt_int);
#endif

	// colliding prediction
	float DTDT=DT*DT;

	for (n1=0;n1<nArray;n1++)
	{
		if (Array[n1]->Handled) Array[n1]->Posdt=Array[n1]->Pos;
		else Array[n1]->Posdt=Array[n1]->Pos+DT*Array[n1]->Vit;
		Array[n1]->f=DTDT*(VECTORNORM(Array[n1]->F))*Array[n1]->InvMass;
		Predict[n1+(n1<<MAX_PHYSIC_OBJECTS_SCENE_SHL)]=1;
		Array[n1]->approx=0;
	}

	for (n1=0;n1<nArray;n1++)
	{
		for (n2=n1+1;n2<nArray;n2++)
		{
			r=Array[n1]->Radius + Array[n2]->Radius + Array[n1]->f+ Array[n2]->f;
			VECTORSUB(u,Array[n1]->Posdt,Array[n2]->Posdt);
			if (((f_abs(u.x)<r)&&(f_abs(u.y)<r)&&(f_abs(u.z)<r))||(Array[n1]->Handled))
			{
                Predict[n1+(n2<<MAX_PHYSIC_OBJECTS_SCENE_SHL)]=1;
                Predict[n2+(n1<<MAX_PHYSIC_OBJECTS_SCENE_SHL)]=1;
			}
		}
	}

	// iterations
#ifdef LOG_PHYSIC
	PH_SetContext("Iterations");
#endif

	float invdt_int=1.0f/dt_int;

	for (n=0;n<nArray;n++)
	{
		op=Array[n];

#ifdef LOG_PHYSIC
		PH_Log("Object");
		PH_Log("	Pos",op->Pos);
		PH_Log("	PUSH",op->PUSH);
		PH_Log("	PUSH_POINT",op->PUSH_POINT);
#endif

		if (op->wind_influence)
		{
			u=op->PUSH;
			VECTORMUL(u,u,invdt_int);
			VECTORADD(op->F,op->F,u);
			VECTORSUB(rQ,op->PUSH_POINT,op->Pos);
			CROSSPRODUCT(dLg,rQ,u);
			VECTORADD(op->T,op->T,dLg);
		}
		else
		{
			VECTORMUL(op->F,op->PUSH,invdt_int);
			VECTORSUB(rQ,op->PUSH_POINT,op->Pos);
			CROSSPRODUCT(dLg,rQ,op->PUSH);
			VECTORMUL(op->T,dLg,invdt_int);
		}
	}

	// particles
	if (nbParticlesArray>0)
	{
		for (n=0;n<nbParticlesArray;n++)
		if (ParticlesArray[n].active)
		{
			if (ParticlesArray[n].wind_influence)
			{
				u=ParticlesArray[n].PUSH;
				VECTORMUL(u,u,invdt_int);
				VECTORADD(ParticlesArray[n].F,ParticlesArray[n].F,u);
			}
			else
			{
				VECTORMUL(ParticlesArray[n].F,ParticlesArray[n].PUSH,invdt_int);
				VECTORZERO(ParticlesArray[n].PUSH);
			}
		}
	}
    
	for (n=0;n<nBaseJoints;n++) BaseJoints[n]->setupJacobian();

	for (n=0;n<nArray;n++)
	{
		SortedIndices[n].nobj=n;
		SortedIndices[n].y=Array[n]->Pos.y;
	}

	// sort
	for (n1=0;n1<nArray;n1++)
		for (n2=n1+1;n2<nArray;n2++)
			if (SortedIndices[n2].y>SortedIndices[n1].y)
			{
				n=SortedIndices[n1].nobj;
				SortedIndices[n1].nobj=SortedIndices[n2].nobj;
				SortedIndices[n2].nobj=n;
				y=SortedIndices[n1].y;
				SortedIndices[n1].y=SortedIndices[n2].y;
				SortedIndices[n2].y=y;
			}

#ifdef MULTITHREADED_PHYSIC
	if (PhysicMultithreadToggle)
    {
        if (CORE==NULL)
        {
            CORE=new PhysicThread;
            CORE->InitThreads(this);
        }
        
        CORE->PHTHREADSACTIVE=1;
    }
#endif

	for (n0=0;n0<N_ITERATIONS_tmp;n0++)
	{	
		COEF_ENVELOPPE=0.0f;
        y_max=-10000000;
		for (n=0;n<nCollisionMeshes;n++) 
		{
			if (!CollisionMeshes[n].statique)
			{
				float tn=((float) n0)/N_ITERATIONS_tmp;
				CollisionMeshes[n].obj->Coo=tn*CollisionMeshes[n].bCoo + (1.0f-tn)*CollisionMeshes[n].aCoo;
				CollisionMeshes[n].obj->Rot=tn*CollisionMeshes[n].bRot + (1.0f-tn)*CollisionMeshes[n].aRot;
				CollisionMeshes[n].Actualise(n+1);
			}
            
            if (y_max<CollisionMeshes[n].maxi.y) y_max=CollisionMeshes[n].maxi.y;
		}
        
        if (y_max<-10000) y_max=0;
        y_max+=20.0f;

#ifdef LOG_PHYSIC
		PH_Log("Calcul");
		PH_Log("Object number",nArray);
#endif

		// stuffs
		memset(NoCollision,0,MAX_PHYSIC_OBJECTS_SCENE*MAX_PHYSIC_OBJECTS_SCENE);

		TagCollisionLinks();

		#define K3 (1.0f/3.0f)

		for (n=0;n<nArray;n++)
		{
			op=Array[n];

			M=op->ActualOrientationMatrix();
			Mt.Transpose(M);
			op->MActual=op->MOActual=M;
			op->MActual.Translation(op->Pos.x,op->Pos.y,op->Pos.z);
			
			MATRIXMATRIX(tmp,Mt,op->Inv_I);
			MATRIXMATRIX(op->IIt,tmp,M);
			
			MATRIXMATRIX(tmp,Mt,op->I);
			MATRIXMATRIX(op->It,tmp,M);
			
//			SortedIndices[n].nobj=n;
//			SortedIndices[n].y=op->Pos.y;

			if ((op->Type==TYPE_OBJECT)&&(op->statique==false))
			{ 
				if (op->OBJECT)
				{
					((CObject3D *)op->OBJECT)->Calculate(&op->MActual,&op->MOActual);

					CObject3D * obj=((CObject3D *)op->OBJECT);

					for (int k=0;k<obj->nFaces;k++)
					{
						VECTORADD(u,obj->Faces[k].v[0]->Calc,obj->Faces[k].v[1]->Calc);
						VECTORADD(u,u,obj->Faces[k].v[2]->Calc);
						VECTORMUL(u,u,K3);
						obj->Faces[k].mp0.x=u.x;
						obj->Faces[k].mp0.y=u.y;
						obj->Faces[k].mp1.x=u.z;
					}
				}

				if (op->SP)
				{
					for (int xx=0;xx<op->SP->nbx;xx++)
						for (int yy=0;yy<op->SP->nby;yy++)
							for (int zz=0;zz<op->SP->nbz;zz++)
							{
								nn=xx+4*(yy+4*zz);
								VECTORINIT(calc,op->SP->balls[nn].x,op->SP->balls[nn].y,op->SP->balls[nn].z);
								VECTOR3MATRIX(calc,calc,op->MActual);
								op->SP->balls[nn].calc=calc;
							}
				}

			}
		}

		// sort
/*
		for (n1=0;n1<nArray;n1++)
			for (n2=n1+1;n2<nArray;n2++)
				if (SortedIndices[n2].y>SortedIndices[n1].y)
				{
					n=SortedIndices[n1].nobj;
					SortedIndices[n1].nobj=SortedIndices[n2].nobj;
					SortedIndices[n2].nobj=n;
					y=SortedIndices[n1].y;
					SortedIndices[n1].y=SortedIndices[n2].y;
					SortedIndices[n2].y=y;
				}
/**/
		// euler equations
		for (n=0;n<nArray;n++)
		{
			op=Array[n];

#ifdef LOG_PHYSIC
			PH_Log("Object: ",n);
			PH_Log("	Pos",op->Pos);
			PH_Log("	Vit",op->Vit);
			PH_Log("	Q",op->Q);
			PH_Log("	W",op->Omega);
			PH_Log("	F",op->F);
			PH_Log("	T",op->T);
			PH_Log("	Mass",op->Mass);
#endif
			if (!op->Linked)
			{
				if (op->fluid)
				{
					// Process temperature
					float v=op->temperature-temperature;
					if (v<0) v=-v;
					if (v<SMALLF2) op->temperature=temperature;
					else
					{
						float k=1.0f/op->Radius;
						if (k>0.01f) k=0.01f;
						k=k/N_ITERATIONS_tmp;
						op->temperature=(1.0f-k)*op->temperature + k*temperature;
					}
				}

				Calculate(op,dt);
			}
            
            Array_vG[n]=op->Vit;
		}

		// particles
		if (nbParticlesArray>0)
		{
			if (step_particles>0)
			{                
				float pdt=dt/recurs_particles;

				for (int iter=0;iter<recurs_particles;iter++)
				{
                
					for (n=0;n<nbParticlesArray;n++)
					if (ParticlesArray[n].active)
					{
						ParticlesArray[n].Time+=dt;

						ParticlesArray[n].F+=ParticlesArray[n].PUSH;
						ParticlesArray[n].PUSH.Init(0,0,0);

						ParticlesArray[n].aPos=ParticlesArray[n].Pos;
						ParticlesArray[n].aVit=ParticlesArray[n].Vit;

						ParticlesArray[n].Pos+=pdt*ParticlesArray[n].Vit;
						ParticlesArray[n].Vit+=pdt*ParticlesArray[n].F/ParticlesArray[n].Mass;

						ParticlesArray[n].Posdt=ParticlesArray[n].Pos;

						VECTORSUB(ParticlesArray[n].dep,ParticlesArray[n].Pos,ParticlesArray[n].aPos);
				
						if (VECTORNORM2(ParticlesArray[n].Vit)<CONSTANT_STATIC_VELOCITY_SQUARE)
						{
							VECTORZERO(ParticlesArray[n].Vit);
							ParticlesArray[n].statique=true;
						}
						else
						{
							if (ParticlesArray[n].gravity) { VECTORINIT(ParticlesArray[n].F,0,ParticlesArray[n].Mass*Gravite,0); }
							else { VECTORZERO(ParticlesArray[n].F); }
						}
					}

#ifdef MULTITHREADED_PHYSIC
					//CalculateThreadedViscosityParticles();
					ParticlesDO(PARTICLES_DENSITY);
                    ParticlesDO(PARTICLES_GRADIENT);
                    ParticlesDO(PARTICLES_VISCOSITY);

					for (n=0;n<nbParticlesArray;n++)
					if (ParticlesArray[n].active)
					{
						float pr=ConvertDensitytoPresure(ParticlesArray[n].density);
						ParticlesArray[n].Vit+=ParticlesArray[n].grad*pr*pdt+ParticlesArray[n].viscosity*pdt;						
						ParticlesArray[n].Pos+=ParticlesArray[n].Vit*pdt;
					}

#else	
					for (n=0;n<nbParticlesArray;n++)
					if (ParticlesArray[n].active)
					{
						CalculateDensityParticle(n);
					}

					for (n=0;n<nbParticlesArray;n++)
					if (ParticlesArray[n].active)
					{
						CalculateGradientParticle(n);
					}

					for (n=0;n<nbParticlesArray;n++)
					if (ParticlesArray[n].active)
					{
						CalculateViscosityParticle(n);
						float pr=ConvertDensitytoPresure(ParticlesArray[n].density);
						ParticlesArray[n].Vit+=ParticlesArray[n].grad*pr*pdt+ParticlesArray[n].viscosity*pdt;						
						ParticlesArray[n].Pos+=ParticlesArray[n].Vit*pdt;
					}
#endif				
					
					for (n=0;n<nbParticlesArray;n++)
					{
						if (ParticlesArray[n].active)
						{
							int coll=0;

							for (nn=0;nn<nPlanes;nn++)
							{ 
								if (Restitution[nn]>=0)
									if (ParticleCollisionPlane(ParticlesArray[n].aPos,ParticlesArray[n].Pos,nn,1,0)==1) coll=1; 
							}

							for (nn=0;nn<nCollisionMeshes;nn++)
							{
								if (CollisionMeshes[nn].Restitution>0)
								if (ParticleCollisionMesh(ParticlesArray[n].aPos,ParticlesArray[n].Pos,nn,&CollisionMeshes[nn],1,0)==1)
								{
									coll=1;
									ContactsParticles[nn].nb++;
									ContactsParticles[nn].is=true;
									CVector ptc=(ParticlesArray[n].aPos+ParticlesArray[n].Pos)/2;
									if (ptc.x>ContactsParticles[nn].maxi.x) ContactsParticles[nn].maxi.x=ptc.x;
									if (ptc.y>ContactsParticles[nn].maxi.y) ContactsParticles[nn].maxi.y=ptc.y;
									if (ptc.z>ContactsParticles[nn].maxi.z) ContactsParticles[nn].maxi.z=ptc.z;
									if (ptc.x<ContactsParticles[nn].mini.x) ContactsParticles[nn].mini.x=ptc.x;
									if (ptc.y<ContactsParticles[nn].mini.y) ContactsParticles[nn].mini.y=ptc.y;
									if (ptc.z<ContactsParticles[nn].mini.z) ContactsParticles[nn].mini.z=ptc.z;
								}
							}

							if (coll==1)
							{
								CVector dep=ParticlesArray[n].Pos-ParticlesArray[n].Posdt;
								ContactStruct * cs=Contacts[0].GetFirst();
								while (cs)
								{

									float ss=DOT(dep,cs->Normals);
									if (ss<0) dep-=ss*cs->Normals;
									cs=Contacts[0].GetNext();
								}
								Contacts[0].Free();
								ParticlesArray[n].Pos=ParticlesArray[n].Posdt+dep;
							}
						}
					}
					/**/

					for (n=0;n<nbParticlesArray;n++)
					if ((ParticlesArray[n].active)&&(!ParticlesArray[n].statique))
					{
						int res2=0;
						ParticlesArray[n].Collided=0;

						for (nn=0;nn<nPlanes;nn++)
						{ 
							if (Restitution[nn]>=0)
								if (ParticleCollisionPlane(ParticlesArray[n].aPos,ParticlesArray[n].Pos,nn,1,0)==1) res2=1; 
						}

						for (nn=0;nn<nCollisionMeshes;nn++)
							if (CollisionMeshes[nn].Restitution>0) { if (ParticleCollisionMesh(ParticlesArray[n].aPos,ParticlesArray[n].Pos,nn,&CollisionMeshes[nn],1,0)==1) res2=1; }
				
						for (nn=0;nn<nArray;nn++)
						{
							if (ParticleCollisionBall(ParticlesArray[n].aPos,ParticlesArray[n].Pos,Array[nn]->Pos,Array[nn]->Radius))
								if (ParticleCollisionProp(ParticlesArray[n].aPos,ParticlesArray[n].Pos,nn,1,0))
								{
									Array[nn]->statique=false;
									Array[nn]->wait=static_method;
									res2=1;
								}
						}

						if (res2)
						{
							ParticlesArray[n].Collided=1;
                    
							float dt_int_p=dt_int*ParticlesArray[n].iter;

							vG=ParticlesArray[n].Vit;
							float STATIC_VREL_SEUIL=SMALLF;
				
							ContactStruct * cs=Contacts[0].GetFirst();
							while (cs)
							{
								if (cs->Types==COLLISION_WITH_OBJECT)
								{
									t=cs->Num;
									N=cs->Normals;
									I=cs->Point;
									op2=Array[t];

									float ss=DOT(ParticlesArray[n].dep,N);
									if (ss<0) ParticlesArray[n].dep-=ss*N;

									friction=op2->Friction;
									VECTORSUB(rQ2,I,op2->Pos);
									CROSSPRODUCT(v2,op2->Omega,rQ2);
									VECTORADD(u2,op2->Vit,v2);
									VECTORSUB(u,vG,u2);
									DOTPRODUCT(vrel,u,N);
									if (vrel<-STATIC_VREL_SEUIL)
									{
										u.x=u.x-vrel*N.x;
										u.y=u.y-vrel*N.y;
										u.z=u.z-vrel*N.z;
										if (VECTORNORM2(u)>TEST_NORM) VECTORNORMALISE(u);
										VECTORMUL(u,u,friction);
										VECTORSUB(ua2,N,u);
										VECTORNORMALISE(ua2);
										VECTORDIV(ua1,ua2,ParticlesArray[n].Mass);
										VECTORMUL(ub1,N,op2->InvMass);

										coef1=DOT(N,ua1);

										CROSSPRODUCT(v,rQ2,N);
										VECTOR3MATRIX(v,v,op2->IIt);
										CROSSPRODUCT(v,v,rQ2);
										coef2=DOT(ua2,v) + DOT(ua2,ub1);
										p=-(1+op2->Restitution)*vrel/(coef1+coef2);
										VECTORMUL(pN,ua2,p/(dt_int_p));
										VECTORADD(ParticlesArray[n].F,ParticlesArray[n].F,pN);

										VECTORSUB(op2->F,op2->F,pN);
										CROSSPRODUCT(dLg,rQ2,pN);
										VECTORSUB(op2->T,op2->T,dLg);
									}
								}
								else
								{
									if (cs->Types==COLLISION_WITH_PLANE) t=cs->Num; else t=cs->Num + MAX_FACES_PER_OBJECT;

									N=cs->Normals;
									I=cs->Point;

									float ss=DOT(ParticlesArray[n].dep,N);
									if (ss<0) ParticlesArray[n].dep-=ss*N;

									DOTPRODUCT(vrel,vG,N);
									if ((vrel>-STATIC_VREL_SEUIL)&&(vrel<0.0f))
									{
										ParticlesArray[n].F.Init(0,0,0);
									}
									else
									if (vrel<-STATIC_VREL_SEUIL)
									{
										if (t<MAX_FACES_PER_OBJECT) friction=Friction[t];
										else friction=CollisionMeshes[(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT].Friction;
										u.x=vG.x-vrel*N.x;
										u.y=vG.y-vrel*N.y;
										u.z=vG.z-vrel*N.z;
										if (VECTORNORM2(u)>TEST_NORM) VECTORNORMALISE(u);
										VECTORMUL(u,u,friction);
										VECTORSUB(ua2,N,u);
										VECTORNORMALISE(ua2);
										VECTORDIV(ua1,ua2,ParticlesArray[n].Mass);
										coef1=DOT(N,ua1);  // ua2 <=> N
										if (t<MAX_FACES_PER_OBJECT)
										{
											p=-(1+Restitution[t])*vrel/coef1;
										}
										else
										{
											p=-(1+CollisionMeshes[(t-MAX_FACES_PER_OBJECT)/MAX_FACES_PER_OBJECT].Restitution)*vrel/coef1;
										}

										VECTORMUL(pN,ua2,(p/dt_int_p));
										VECTORADD(ParticlesArray[n].F,ParticlesArray[n].F,pN);
									}
								}
								cs=Contacts[0].GetNext();
							}

							ParticlesArray[n].Pos=ParticlesArray[n].aPos+ParticlesArray[n].dep;
						}

						Contacts[0].Free();
					}
				}
			}
			// END PARTICLES
		}

		// constrained physic

		for (n=0;n<nArray;n++)
		{
			op=Array[n];
			if (op->Linked)
			{
				op->aPos=op->Pos;
				op->aVit=op->Vit;
				op->aOmega=op->Omega;
				op->aQ=op->Q;
			}
		}

		//if (nBaseJoints>0) MassMat.Init(6,6);

#ifdef MULTITHREADED_PHYSIC
        if (PhysicMultithreadToggle)
        {
            //CORE->PHTHREADSACTIVE=1;
            CORE->dt=dt;
            CORE->dt_int=dt_int;
            
            for (n=0;n<CORE->nbthreads;n++)
            {
                CORE->phnb_num[n]=0;
                CORE->phnb_num2[n]=0;
                CORE->phnb_num3[n]=0;
                CORE->phnb_num4[n]=0;
            }
        }
#endif
        

#ifdef MULTITHREADED_PHYSIC
        for (n=0;n<nBaseJoints;n++)
        {
            if (BaseJoints[n]->skelet)
            {
                if (PhysicMultithreadToggle)
                {
                    int th=n%CORE->nbthreads;
                    CORE->ns_ph4[th][CORE->phnb_num4[th]++]=n;
                }
                else ProcessBaseJoints(n,dt,dt_int);
            }
            else ProcessBaseJoints(n,dt,dt_int);
        }

        if (PhysicMultithreadToggle)
        {
            for (n=0;n<CORE->nbthreads;n++)
                if (CORE->phnb_num4[n]>0) CORE->ph_num4[n]=1;
            
            while (CORE->ph4_busy()) WAIT_THREAD();
        }

#else
        for (n=0;n<nBaseJoints;n++)
        {
            ProcessBaseJoints(n,dt,dt_int);
        }
#endif

		//if (nBaseJoints>0) MassMat.Free();

		for (n=0;n<nArray;n++)
		{
			op=Array[n];
			if (op->Linked)
			{
				op->Posdt=op->Pos;
				op->Vit_dt=op->Vit;
				op->Q_dt=op->Q;
				op->Omega_dt=op->Omega;
				VECTORSUB(op->Dep,op->Pos,op->aPos);
			}
		}

		// process influencies between objects and static objects
/*		
		for (n=0;n<nArray;n++)
			for (nn=n+1;nn<nArray;nn++)
			{
				int adr=n+(nn<<MAX_PHYSIC_OBJECTS_SCENE_SHL);
				if ((TestBit(adr)==1)&&(!(TestBitC(adr)==1)))
				{
					CVector D;
					int adr2=nn+(n<<MAX_PHYSIC_OBJECTS_SCENE_SHL);
					VECTORSUB(D,Array[n]->Pos,Array[nn]->Pos);
					r=Array[n]->Radius+Array[nn]->Radius;
					if ((f_abs(D.x)>r)||(f_abs(D.y)>r)||(f_abs(D.z)>r)) { SetBitC(adr); SetBitC(adr2); }
				}
			}
/**/

		if (static_method>=0)
		{		
			for (n=0;n<nArray;n++) Array[n]->update=0;

			COEF_ENVELOPPE=SMALLF2;

#ifdef MULTITHREADED_PHYSIC_STATE

            n=0;
			while (n<nArray)
			{
                if (Array[n]->Pos.y<y_max)
                {
					if (PhysicMultithreadToggle)
					{
						int th=n%CORE->nbthreads;
                        CORE->ns_ph2[th][CORE->phnb_num2[th]++]=n;
                        n++;
					}
					else ProcessBodyStateUpdate(n++,0);
                }
                else n++;
            }
            
            if (PhysicMultithreadToggle)
            {
                for (n=0;n<CORE->nbthreads;n++)
                    if (CORE->phnb_num2[n]>0) CORE->ph_num2[n]=1;

                while (CORE->ph2_busy()) WAIT_THREAD();
            }

#else
            n=0;
            while (n<nArray)
            {
                if (Array[n]->Pos.y<y_max)
                {
					ProcessBodyStateUpdate(n++,0);
                }
                else n++;
			}
#endif

			COEF_ENVELOPPE=0.0f;

			for (n=0;n<nArray;n++) 
			{
				if (Array[n]->update==1)
				{
					Array[n]->statique=false;
					Array[n]->wait=static_method+1;
				}
			}
		}

		for (n=0;n<nArray;n++)
		{
			Array_vG[n]=Array[n]->Vit;
			op=Array[n];
			VECTORZERO(op->T);
			VECTORZERO(op->F);
			if (op->statique==false) VECTORINIT(op->F,0,op->Mass*Gravite,0);
			VECTORZERO(op->PUSH);
			op->tag=0;
			if (((VECTORNORM2(op->Vit)>KD*CONSTANT_STATIC_VELOCITY_SQUARE)&&
				(VECTORNORM2(op->Omega)>KDa*CONSTANT_STATIC_TORQUE_SQUARE))) op->tag=1;
		}
        
        for (n=0;n<nBaseJoints;n++)
        {
            joint=BaseJoints[n];
            while (joint)
            {
                int offsetjoint=BaseJoints[n]->nObjs*6+BaseJoints[n]->jacobians_indices[joint->num];
                
                if (joint->type==SPRING_JOINT)
                {
                    op=joint->Object1;
                    op2=joint->Object2;
                    u=op->Pos-op2->Pos;
                    joint->Lt=u.Norme();
                    u.Normalise();
                    joint->dL=joint->Lt-joint->L;
                    float Ko=10.0f*N_ITERATIONS_tmp;
                    op->F-=Ko*op->Mass*joint->raideur*joint->dL*u;
                    op2->F+=Ko*op->Mass*joint->raideur*joint->dL*u;
                }
                
                if (joint->type==FIXED_SPRING)
                {
                    op=joint->Object1;
                    u=op->Pos-joint->ptfixe;
                    joint->Lt=u.Norme();
                    u.Normalise();
                    joint->dL=joint->Lt-joint->L;
                    float Ko=15.0f;
                    op->F-=Ko*op->Mass*joint->raideur*joint->dL*u;
                }
                
                joint=joint->NextJoint;
            }
        }

#ifdef MULTITHREADED_PHYSIC
        n_n=0;
        while (n_n<nArray)
        {
            n=SortedIndices[n_n].nobj;

            if (Array[n]->Pos.y<y_max)
            {
				if (PhysicMultithreadToggle)
				{
					int th=n%CORE->nbthreads;
                    CORE->ns_ph[th][CORE->phnb_num[th]++]=n;
                    n_n++;
				}
				else
                {
                    ProcessBodyCollisions(n,0);
                    n_n++;
                }
            }
            else n_n++;
        }
        
        if (PhysicMultithreadToggle)
        {
            for (n=0;n<CORE->nbthreads;n++)
                if (CORE->phnb_num[n]>0) CORE->ph_num[n]=1;

            while (CORE->ph_busy()) WAIT_THREAD();
        }

#else
        n_n=0;
        while (n_n<nArray)
        {
            n=SortedIndices[n_n].nobj;

            if (Array[n]->Pos.y<y_max)
            {
				ProcessBodyCollisions(n,0);
                n_n++;
            }
            else n_n++;
		}
#endif


#ifdef MULTITHREADED_PHYSIC
        n=0;
        while (n<nArray)
        {
            if (Array[n]->Pos.y<y_max)
            {
				if (PhysicMultithreadToggle)
				{
                    int th=n%CORE->nbthreads;
                    CORE->ns_ph3[th][CORE->phnb_num3[th]++]=n;
                    n++;
				}
				else ProcessBodyTime(n++,0);
            }
            else n++;
        }
        
        if (PhysicMultithreadToggle)
        {
            for (n=0;n<CORE->nbthreads;n++)
                if (CORE->phnb_num3[n]>0) CORE->ph_num3[n]=1;

            while (CORE->ph3_busy()) WAIT_THREAD();
        }
#else
        n=0;
        while (n<nArray)
        {
            if (Array[n]->Pos.y<y_max)
            {
				ProcessBodyTime(n++,0);
            }
            else n++;
        }
#endif

    } // iterations

#ifdef MULTITHREADED_PHYSIC
	if (nbParticlesArray>0) ParticlesThreadsEnd();
#endif	

#ifdef LOG_PHYSIC
	PH_SetContext("Iterations");
#endif

	if (nbParticlesArray>0)
		for (n=0;n<particlesboundx*particlesboundy*particlesboundz;n++) particlesvoxel[n].Free();


	for (n=0;n<nCollisionMeshes;n++) 
	{		
		CollisionMeshes[n].aCoo=CollisionMeshes[n].bCoo;
		CollisionMeshes[n].aRot=CollisionMeshes[n].bRot;
		CollisionMeshes[n].obj->update=false;
	}

#ifdef LOG_PHYSIC
	PH_WriteLog("physic.log");
#endif

#ifdef MULTITHREADED_PHYSIC
	//if (PhysicMultithreadToggle) CORE->KillThreads();
#endif

#ifdef MULTITHREADED_PHYSIC
	if (PhysicMultithreadToggle)
	{
        CORE->PHTHREADSACTIVE=0;
        
		for (n=0;n<NBTHREADS;n++)
		{
			STRUCT_EVENT_CONTACT *e=EventsTH[n].GetFirst();
			while (e)
			{
				Events.Add(*e);
				e=EventsTH[n].GetNext();
			}

			EventsTH[n].Free();
		}
	}
    else
    {
        STRUCT_EVENT_CONTACT *e=EventsTH[0].GetFirst();
        while (e)
        {
            Events.Add(*e);
            e=EventsTH[0].GetNext();
        }

        EventsTH[0].Free();
    }
#endif

    if (fix) FixProps();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::FixProps()
{
    for (int k=0;k<nArray;k++) Array[k]->Fix();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPhysic::KillTHs()
{
#ifdef MULTITHREADED_PHYSIC
    if (CORE)
    {
        CORE->KillThreads();
        delete CORE;
        CORE=NULL;
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
