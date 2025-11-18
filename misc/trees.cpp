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
//	@file trees.cpp
//	@date 2003
////////////////////////////////////////////////////////////////////////

#include "../base/params.h"

#ifdef _DEFINES_API_CODE_TREES_

#include <math.h>
#include <stdio.h>

#if defined(ANDROID)&&!defined(GLESFULL)
#include "../data/file_io_zip.h"
#endif

#include "../base/maths.h"
#include "../base/list.h"
#include "../base/objects3d.h"
#include "../base/bsp_volumic.h"
#include "../base/edges.h"
#include "../base/triangularisation.h"
#include "trees.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }

#define MAX 1536

#ifndef CLASSNED

#define MyFILE FILE

//////////////////////////////////////////////////////////////////////////////////////////////////////
int Myfread(void * dest,int nb,int sz,MyFILE *f)
{
	return _Myfread(dest,nb,sz,f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
MyFILE *Myfopen(char *fname,char *rw)
{
	return _Myfopen(fname,rw);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void Myfclose(MyFILE *f)
{
	_Myfclose(f);
}
#else
extern char * MyFILEBuffer;
#endif


bool CNode::Add(CNode *child)
{
	int res=0;
	for (int n=0;n<nb;n++) if (next[n]==child) res=1;
	if ((res==0)&&(nb<128))
	{
		next[nb]=child;
		nb++;
		return true;
	}
	else return false;
}

int CNode::IndexNode(CNode *child)
{
    for (int n=0;n<nb;n++) if (next[n]==child) return n;
    return -1;
}

void CNode::Del(CNode *child)
{
	int n,nd;

	nd=-1;
	for (n=0;n<nb;n++) if (next[n]==child) nd=n;

	if (nd>=0)
	{
		nb--;
		next[nd]=next[nb];
	}
}

void CNode::SetId(int i)
{
	id=i;
}

void CNode::SetEuler(float rx,float ry,float rz)
{
	Euler.Init(rx,ry,rz);
}

void CNode::SetTranslate(float tx,float ty,float tz)
{
	Translate.Init(tx,ty,tz);
}

void CNode::SetScale(float sx,float sy,float sz)
{
	Scale.Init(sx,sy,sz);
}

void CNode::SetQuaternion(CQuaternion &q)
{
	Q=q;
}

void CNode::SetPivot(float x,float y,float z)
{
	Pivot.Init(x,y,z);
}

void CTree::Free()
{
    int n;
    int nb=0;
    GetNbEntries(root,&nb);
    
    CNode * tmp[1024];
    for (n=0;n<nb;n++)
    {
        CNode *nd=NULL;
        GetNodeById(root,&nd,n);
        tmp[n]=nd;
    }
    
    for (n=0;n<nb;n++)
        if (tmp[n]) delete tmp[n];
    
	root=NULL;
}

void CTree::CalcMatrix(CMatrix &fm,CNode *nod)
{
	CMatrix m;

    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
		/*
        m.Translation(-nod->Pivot.x,-nod->Pivot.y,-nod->Pivot.z);
        m.Scaling(nod->Scale.x,nod->Scale.y,nod->Scale.z);
        m.RotationDegre(nod->Euler.x,nod->Euler.y,nod->Euler.z);
        m.RotationDegre(nod->Alter.x,nod->Alter.y,nod->Alter.z);
        m.Mq(nod->Q);
        m.Translation(nod->Pivot.x,nod->Pivot.y,nod->Pivot.z);
        m.Translation(nod->Translate.x,nod->Translate.y,nod->Translate.z);

		/**/
		//m.a[3][0]=-nod->Pivot.x; m.a[3][1]=-nod->Pivot.y; m.a[3][2]=-nod->Pivot.z;
		//m.a[0][0]=nod->Scale.x; m.a[1][1]=nod->Scale.y; m.a[2][2]=nod->Scale.z;

        m.BaseRotationDegre(nod->Euler.x,nod->Euler.y,nod->Euler.z);

		CVector rel;
		VECTOR3MATRIX3X3(rel,nod->Pivot,m);
				
		if (nod->Scale.x!=1.0f) { m.a[3][0]=-rel.x*nod->Scale.x; m.a[0][0]*=nod->Scale.x; m.a[0][1]*=nod->Scale.x; m.a[0][2]*=nod->Scale.x; }
		else m.a[3][0]=-rel.x;
		if (nod->Scale.y!=1.0f) { m.a[3][1]=-rel.y*nod->Scale.y; m.a[1][0]*=nod->Scale.y; m.a[1][1]*=nod->Scale.y; m.a[1][2]*=nod->Scale.y; }
		else m.a[3][1]=-rel.y;
		if (nod->Scale.z!=1.0f) { m.a[3][2]=-rel.z*nod->Scale.z; m.a[2][0]*=nod->Scale.z; m.a[2][1]*=nod->Scale.z; m.a[2][2]*=nod->Scale.z; }
		else m.a[3][2]=-rel.z;

        if (applyalteration) m.RotationDegre(nod->Alter.x,nod->Alter.y,nod->Alter.z);
        m.Mq(nod->Q);

		m.a[3][0]+=nod->Pivot.x; m.a[3][1]+=nod->Pivot.y; m.a[3][2]+=nod->Pivot.z;
		m.a[3][0]+=nod->Translate.x; m.a[3][1]+=nod->Translate.y; m.a[3][2]+=nod->Translate.z;
		/**/

        Palette[nod->id]=m*fm;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) CalcMatrix(Palette[nod->id],nod->next[n]);
    }
}

void CTree::Apply(CMatrix &m,CNode *nod)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        nod->Pivot=nod->Pivot*m;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) Apply(m,nod->next[n]);
    }
}

void CTree::MatrixPalette()
{
	CMatrix m;
	CalcMatrix(m,root);
}


void CTree::Biped()
{
    /*
	int corres[17][4]={ 
		{-1,1,16,-1},
		{0,2,5,15},
		{1,3,-1,-1},
		{2,4,-1,-1},
		{3,-1,-1,-1},
		{1,6,-1,-1},
		{5,7,-1,-1},
		{6,-1,-1,-1},
		{0,9,12,-1},
		{8,10,-1,-1},
		{9,11,-1,-1},
		{10,-1,-1,-1},
		{8,13,-1,-1},
		{12,14,-1,-1},
		{13,-1,-1,-1},
		{1,-1,-1,-1},
		{0,8,-1,-1}
	};/**/

//	0					   
//	1					
//	2					O 15
//	3			    5	|1
//	4			   O----O----O 2
//	5			  /		|     \	
//	6			 /	    | 0/8  \
//	7			O 6		O		O 3
//	8			|		|		|
//	9			|		0 16
//	10			O 7		|		O 4
//	11			|		O 8/0	|
//	12				   / \
//	13				 /     \
//	14              O 9     O 12 
//	15              |       |
//					|       |
//					|       |
//					O 10    O 13
//					|       |
//					|       |
//					|       |
//					O 11    O 14
//				 __/         \__

    char * names[17]={
    "basin",
    "neck",
    "L_shoulder",
    "L_elbow",
    "L_hand",
    "R_shoulder",
    "R_elbow",
    "R_hand",
    "torso",
    "R_hip",
    "R_knee",
    "R_foot",
    "L_hip",
    "L_knee",
    "L_foot",
    "head",
    "abdomen"
    };
    
    int corres[17][5]={
        {-1,9,12,16,-1},
        {0,2,5,15,-1},
        {1,3,-1,-1,-1},
        {2,4,-1,-1,-1},
        {3,-1,-1,-1,-1},
        {1,6,-1,-1,-1},
        {5,7,-1,-1,-1},
        {6,-1,-1,-1,-1},
        {16,1,-1,-1,-1},
        {0,10,-1,-1,-1},   // 9
        {9,11,-1,-1,-1},
        {10,-1,-1,-1,-1},
        {0,13,-1,-1,-1},   // 12
        {12,14,-1,-1,-1},
        {13,-1,-1,-1,-1},
        {1,-1,-1,-1,-1},   // 15
        {0,8,-1,-1,-1}
    };

	float pos[17][3]={
		//{0,0,3},
        {0,5.7f,3},
		{0,-1.9f,0},
		
		{5,-2.8f,0},
		{8,1.7f,0},
		{11.5f,7.5f,0},
		
		{-5,-2.8f,0},
		{-8,1.7f,0},
		{-11.5f,7.5f,0},
		
		//{0,6.0f,3},
        {0,-0.5f,-1.5f},

		{-3.5,8.2f,2},
		{-4,19.5f,0.7f},
		{-4.5f,30,-1.0f},

		{3.5,8.2f,2},
		{4,19.5f,0.7f},
		{4.5f,30,-1.0f},

		{0,-4,0},
		{0,2.0f,3},
	};

	int n,i;
	CNode *array[17];

	for (n=0;n<17;n++)
    {
        array[n]=new CNode(NULL,n);
        strcpy(array[n]->name,names[n]);
    }

	for (n=0;n<17;n++)
	{
		if (corres[n][0]==-1)
		{
			root=array[n];
			root->prev=NULL;
		}
		else
		{
			array[n]->prev=array[corres[n][0]];
		}

		for (i=1;i<5;i++)
		{
			if (corres[n][i]!=-1) array[n]->Add(array[corres[n][i]]);
		}

		array[n]->SetPivot(pos[n][0],pos[n][1],pos[n][2]);
	}

	Palette=new CMatrix[17];
	Palette0=new CMatrix[17];
}


void CTree::Quadriped()
{
	int corres[17][4]={ 
		{-1,1,16,-1},
		{0,2,5,15},
		{1,3,-1,-1},
		{2,4,-1,-1},
		{3,-1,-1,-1},
		{1,6,-1,-1},
		{5,7,-1,-1},
		{6,-1,-1,-1},
		{0,9,12,-1},
		{8,10,-1,-1},
		{9,11,-1,-1},
		{10,-1,-1,-1},
		{8,13,-1,-1},
		{12,14,-1,-1},
		{13,-1,-1,-1},
		{1,-1,-1,-1},
		{0,8,-1,-1}
	};

//	0					   
//	1					
//	2					O 15
//	3			    5	|1
//	4			   O----O----O 2
//	5			  /		|     \	
//	6			 /	    | 0    \
//	7			O 6		O		O 3
//	8			|		|		|
//	9			|		0 16
//	10			O 7		|		O 4
//	11			|		O 8		|
//	12				   / \
//	13				 /     \
//	14              O 9     O 12 
//	15              |       |
//					|       |
//					|       |
//					O 10    O 13
//					|       |
//					|       |
//					|       |
//					O 11    O 14
//				 __/         \__



	float pos[17][3]={
		{0,6.0f,1},
		{0,6.0f,4.5f},
		
		{5,9.0f,6},
		{8,19.5f,6},
		{11.5f,30.0f,7.5f},
		
		{-5,9.0f,6},
		{-8,19.5f,6},
		{-11.5f,30.0f,7.5f},
		
		{0,6.0f,-5},

		{-3.5,9,-7},
		{-5,19.5f,-8},
		{-6.1f,30,-7},

		{3.5,9,-7},
		{5,19.5f,-8},
		{6.1f,30,-7},

		{0,5.0f,7},
		{0,4.5f,-3},
	};

	int n,i;
	CNode *array[17];

	for (n=0;n<17;n++) array[n]=new CNode(NULL,n);

	for (n=0;n<17;n++)
	{
		if (corres[n][0]==-1)
		{
			root=array[n];
			root->prev=NULL;
		}
		else
		{
			array[n]->prev=array[corres[n][0]];
		}

		for (i=1;i<4;i++)
		{
			if (corres[n][i]!=-1) array[n]->Add(array[corres[n][i]]);
		}

		array[n]->SetPivot(pos[n][0],pos[n][1],pos[n][2]);
	}

	Palette=new CMatrix[17];
	Palette0=new CMatrix[17];
}


void CTree::FitAndWeights(CObject3D *obj)
{

}

void CTree::Scale(CNode *nod,float sx,float sy,float sz)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        nod->Pivot.x*=sx;
        nod->Pivot.y*=sy;
        nod->Pivot.z*=sz;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) Scale(nod->next[n],sx,sy,sz);
    }
}

void CTree::Translate(CNode *nod,float px,float py,float pz)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        nod->Pivot.x+=px;
        nod->Pivot.y+=py;
        nod->Pivot.z+=pz;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) Translate(nod->next[n],px,py,pz);
    }
}

void CTree::Scale(float sx,float sy,float sz)
{
	Scale(root,sx,sy,sz);
}

void CTree::Translate(float px,float py,float pz)
{
	Translate(root,px,py,pz);
}

void CTree::ScaleY(CNode *nod,float sx,float sy,float sz,float sxd,float syd,float szd)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.y>0)
        {
            nod->Pivot.x*=sxd;
            nod->Pivot.y*=syd;
            nod->Pivot.z*=szd;
        }
        else
        {
            nod->Pivot.x*=sx;
            nod->Pivot.y*=sy;
            nod->Pivot.z*=sz;
        }
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) ScaleY(nod->next[n],sx,sy,sz,sxd,syd,szd);
    }
}

void CTree::ScaleY(float sx,float sy,float sz,float sxd,float syd,float szd)
{
    ScaleY(root,sx,sy,sz,sxd,syd,szd);
}


void CTree::GetScale_x(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (f_abs(nod->Pivot.x)>*s) *s=f_abs(nod->Pivot.x);
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetScale_x(nod->next[n],s);
    }
}

void CTree::GetScale_y(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (f_abs(nod->Pivot.y)>*s) *s=f_abs(nod->Pivot.y);
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetScale_y(nod->next[n],s);
    }
}

void CTree::GetScale_z(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (f_abs(nod->Pivot.z)>*s) *s=f_abs(nod->Pivot.z);
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetScale_z(nod->next[n],s);
    }
}

void CTree::GetMin_x(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.x<*s) *s=nod->Pivot.x;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMin_x(nod->next[n],s);
    }
}

void CTree::GetMax_x(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.x>*s) *s=nod->Pivot.x;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMax_x(nod->next[n],s);
    }
}

void CTree::GetMin_y(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.y<*s) *s=nod->Pivot.y;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMin_y(nod->next[n],s);
    }
}

void CTree::GetMax_y(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.y>*s) *s=nod->Pivot.y;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMax_y(nod->next[n],s);
    }
}

void CTree::GetMin_z(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.z<*s) *s=nod->Pivot.z;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMin_z(nod->next[n],s);
    }
}

void CTree::GetMax_z(CNode *nod,float *s)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->Pivot.z>*s) *s=nod->Pivot.z;
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetMax_z(nod->next[n],s);
    }
}

void CTree::GetNbEntries(CNode *nod,int *ndx)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->id+1>*ndx) *ndx=nod->id+1;
        
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) GetNbEntries(nod->next[n],ndx);
    }
}

void CTree::ResetFlags()
{
	if (nbentries<0)
	{
		for (int n=0;n<4096;n++) flags[n]=0;
	}
	else
	{
		for (int n=0;n<=nbentries;n++) flags[n]=0;
	}
}

void CTree::Reset(CNode *nod)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        nod->Euler.Init(0,0,0);
        nod->Q.Id();
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) Reset(nod->next[n]);
    }
}

void CTree::Fix(CNode *nod)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        nod->Pivot=nod->Pivot*Palette[nod->id];
        nod->Euler.Init(0,0,0);
        nod->Q.Id();
        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) Fix(nod->next[n]);
    }
}

void CTree::GetNodeById(CNode *nod,CNode **res,int id)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (nod->id==id) *res=nod;
        else
            for (int n=0;n<nod->nb;n++)
                if (nod->next[n]!=root) GetNodeById(nod->next[n],res,id);
    }
}

void CTree::GetNodeByName(CNode *nod,CNode **res,char *name)
{
    if (nod==root) ResetFlags();
    
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;
        
        if (strcmp(nod->name,name)==0) *res=nod;
        else
            for (int n=0;n<nod->nb;n++)
                if (nod->next[n]!=root) GetNodeByName(nod->next[n],res,name);
    }
}

void CTree::AddLengthBonesAfterNode(CNode *nod,float add)
{
	CVector u;
	if (nod->nb==1)
	{
		u=nod->next[0]->Pivot-nod->Pivot;
		u.Normalise();
		u*=add;
		Translate(nod->next[0],u.x,u.y,u.z);
	}
}

void CTree::CleanKey(int key)
{
	int n,nb;
	CNode *nod;
	nb=0;
	if (root) GetNbEntries(root,&nb);
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		nod->Eulers[key].Init(0,0,0);
		nod->Pivots[key].Init(0,0,0);
		nod->Scales[key].Init(1,1,1);
        nod->Euler.Init(0,0,0);
        nod->Translate.Init(0,0,0);
		nod->Qs[key].Id();
        nod->Q.Id();
	}
}

void CTree::StoreKey(int key)
{
	int n,nb;
	CNode *nod;
	nb=0;
	if (root) GetNbEntries(root,&nb);
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		nod->Eulers[key]=nod->Euler;
		nod->Scales[key]=nod->Scale;
		nod->Pivots[key]=nod->Translate;
		nod->Qs[key]=nod->Q;
	}
}


void CTree::ReadKey(int key)
{
	int n,nb;
	CNode *nod;
	nb=0;
	if (root) GetNbEntries(root,&nb);
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		nod->Euler=nod->Eulers[key];
		nod->Q=nod->Qs[key];
		nod->Scale=nod->Scales[key];
		nod->Translate=nod->Pivots[key];
	}
}

void CTree::BackupKey()
{
	int n,nb;
	CNode *nod;
	nb=0;
	if (root) GetNbEntries(root,&nb);
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		nod->_Euler=nod->Euler;
		nod->_Q=nod->Q;
		nod->_Scale=nod->Scale;
		nod->_Translate=nod->Translate;
	}
}


void CTree::RestoreKey()
{
	int n,nb;
	CNode *nod;
	nb=0;
	if (root) GetNbEntries(root,&nb);
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		nod->Euler=nod->_Euler;
		nod->Q=nod->_Q;
		nod->Scale=nod->_Scale;
		nod->Translate=nod->_Translate;
	}
}

void CTree::ResetAlteration()
{
    int n,nb;
    CNode *nod;
    nb=0;
    if (root) GetNbEntries(root,&nb);
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);
        nod->Alter.Init(0,0,0);
        nod->Alteration.Init(0,0,0);
        nod->speed=1;
    }
}

float mod1(float x)
{
    float r=0;
    
    if (x>0)
    {
        int n=(int)x;
        r=x-n;
    }
    else
    {
        int n=(int)x;
        r=1+x-n;
    }
    
    return r;
}

void CTree::InterpolateKeys(CNode *nod,int key1,int key2,float t)
{
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;

		nod->Euler=nod->Eulers[key1] + t*(nod->Eulers[key2] - nod->Eulers[key1]);
		nod->Translate=nod->Pivots[key1] + t*(nod->Pivots[key2] - nod->Pivots[key1]);
		nod->Scale=nod->Scales[key1] + t*(nod->Scales[key2] - nod->Scales[key1]);
		nod->Q.Slerp(&nod->Qs[key1],&nod->Qs[key2],t);

		float alpha=mod1((key1+t)/nod->speed)*PI;
		nod->Alter=sinf(alpha)*nod->Alteration;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) InterpolateKeys(nod->next[n],key1,key2,t);
    }
}

void CTree::InterpolateKeys(CNode *nod,int key,float t)
{
    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;

		nod->Euler=nod->Eulers[key];
		nod->Translate=nod->Pivots[key];
		nod->Scale=nod->Scales[key];
		nod->Q=nod->Qs[key];

		float alpha=mod1((key+t)/nod->speed)*PI;
		nod->Alter=sinf(alpha)*nod->Alteration;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) InterpolateKeys(nod->next[n],key,t);
    }
}

void CTree::InterpolateKeys(int key1,int key2,float t)
{
	ResetFlags();
	if (key1==key2) InterpolateKeys(root,key1,t);
	else InterpolateKeys(root,key1,key2,t);

	k1=key1;
	k2=key2;
	kt=t;
    
    two=false;
}

void CTree::InterpolateKeysTwoAnim(CNode *nod,int fstkey1,int fstkey2,float t1,int sndkey1,int sndkey2,float t2,float t)
{
    CVector Euler1,Translate1,Scale1;
    CQuaternion Q1;
    CVector Euler2,Translate2,Scale2;
    CQuaternion Q2;

    if (flags[nod->id]==0)
    {
        flags[nod->id]=1;

		Euler1=nod->Eulers[fstkey1] + t1*(nod->Eulers[fstkey2] - nod->Eulers[fstkey1]);
		Translate1=nod->Pivots[fstkey1] + t1*(nod->Pivots[fstkey2] - nod->Pivots[fstkey1]);
		Scale1=nod->Scales[fstkey1] + t1*(nod->Scales[fstkey2] - nod->Scales[fstkey1]);
		Q1.Slerp(&nod->Qs[fstkey1],&nod->Qs[fstkey2],t1);

		Euler2=nod->Eulers[sndkey1] + t2*(nod->Eulers[sndkey2] - nod->Eulers[sndkey1]);
		Translate2=nod->Pivots[sndkey1] + t2*(nod->Pivots[sndkey2] - nod->Pivots[sndkey1]);
		Scale2=nod->Scales[sndkey1] + t2*(nod->Scales[sndkey2] - nod->Scales[sndkey1]);
		Q2.Slerp(&nod->Qs[sndkey1],&nod->Qs[sndkey2],t2);

		nod->Euler=Euler1 + t*(Euler2 - Euler1);
		nod->Translate=Translate1 + t*(Translate2 - Translate1);
		nod->Scale=Scale1 + t*(Scale2 - Scale1);
		nod->Q.Slerp(&Q1,&Q2,t);
        
		float alpha=mod1((fstkey1+t1)/nod->speed)*PI;
		nod->Alter=sinf(alpha)*nod->Alteration;

        for (int n=0;n<nod->nb;n++)
            if (nod->next[n]!=root) InterpolateKeysTwoAnim(nod->next[n],fstkey1,fstkey2,t1,sndkey1,sndkey2,t2,t);
    }
}

void CTree::InterpolateKeysTwoAnim(int fstkey1,int fstkey2,float t1,int sndkey1,int sndkey2,float t2,float t)
{
	ResetFlags();
	InterpolateKeysTwoAnim(root,fstkey1,fstkey2,t1,sndkey1,sndkey2,t2,t);

    k1=fstkey1;
    k2=fstkey2;
    kt=t1;

    l1=sndkey1;
    l2=sndkey2;
    lt=t2;
    
    twot=t;
    
    two=true;
}

bool CTree::ValidNed()
{
    int n;
    CNode *nod;
    
    int nb=0;
    GetNbEntries(root,&nb);
    
    if (nb>MAX) return false;
    
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);
        if (nod->nb>128) return false;
    }
    
    return true;
}

#if defined(ANDROID)&&!defined(GLESFULLNOZIP)

void CTree::Save(FILE *f)
{
}

#else
void CTree::Save(FILE *f)
{
	int n,i,k;
	int nb=0;
	CVector *array;
	CVector *euler;
	CVector *translate;
	CVector *scale;
	CQuaternion *quat;
	CVector *eulers;
	CVector *trans;
	CVector *scales;
	CQuaternion *quats;
	CNode *nod;
	int *corres;
	
	GetNbEntries(root,&nb);

	array=new CVector[nb];

	euler=new CVector[nb];
	translate=new CVector[nb];
	scale=new CVector[nb];
	quat=new CQuaternion[nb];
	
	eulers=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	trans=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	scales=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	quats=new CQuaternion[nb*MAX_NUMBER_OF_KEYS_NODE];

	corres=new int[nb*130];

	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		array[n]=nod->Pivot;

		euler[n]=nod->Euler;
		translate[n]=nod->Translate;
		scale[n]=nod->Scale;
		quat[n]=nod->Q;

		for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
		{
			eulers[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Eulers[k];
			trans[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Pivots[k];
			scales[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Scales[k];
			quats[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Qs[k];
		}

		for (i=0;i<130;i++) corres[n*130+i]=-1;
		corres[n*130+0]=nod->nb;
		if (nod->prev) corres[n*130+1]=nod->prev->id;
		else corres[n*130+1]=-1;
		for (i=0;i<nod->nb;i++) corres[n*130+2+i]=nod->next[i]->id;
	}

	fwrite(&nb,sizeof(int),1,f);
	fwrite(&tag,sizeof(int),1,f);
	fwrite(&Coo,sizeof(CVector),1,f);
	fwrite(&Rot,sizeof(CVector),1,f);

	fwrite(array,nb*sizeof(CVector),1,f);
	
	fwrite(euler,nb*sizeof(CVector),1,f);
	fwrite(translate,nb*sizeof(CVector),1,f);
	fwrite(scale,nb*sizeof(CVector),1,f);
	fwrite(quat,nb*sizeof(CQuaternion),1,f);
	
	fwrite(corres,nb*130*sizeof(int),1,f);

	fwrite(eulers,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
	fwrite(trans,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
	fwrite(scales,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
	fwrite(quats,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CQuaternion),1,f);

	// 1004
	char strdata[MAX*32];
	memset(strdata,0,MAX*32);
	int pp=0;
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		char *s=nod->name;
		int p=0;
		while (s[p]!=0) strdata[pp++]=s[p++];
		strdata[pp++]=0;
	}

	fwrite(strdata,32*MAX,1,f);
	for (n=0;n<nb;n++) fwrite(&Palette0[n],sizeof(CMatrix),1,f);

	delete [] corres;
	delete [] array;
	delete [] euler;
	delete [] translate;
	delete [] scale;
	delete [] quat;
	delete [] eulers;
	delete [] scales;
	delete [] trans;
	delete [] quats;

}
#endif
#ifndef CLASSNED
#define Myfread fread
bool CTree::Load(FILE *f,int version)
#else
bool CTree::Load(MyFILE *f,int version)
#endif
{
	int n,i,k;
	int nb=0;
	CVector *array;
	CVector *euler;
	CVector *translate;
	CVector *scale;
	CQuaternion *quat;
	CNode *nodes[MAX];
	int *corres;
	CVector *eulers;
	CVector *trans;
	CVector *scales;
	CQuaternion *quats;

	Myfread(&nb,sizeof(int),1,f);
	Myfread(&tag,sizeof(int),1,f);
	Myfread(&Coo,sizeof(CVector),1,f);
	Myfread(&Rot,sizeof(CVector),1,f);

	array=new CVector[MAX];
	
	euler=new CVector[MAX];
	translate=new CVector[MAX];
	scale=new CVector[MAX];
	quat=new CQuaternion[MAX];

	if (version==1007) corres=new int[MAX*130];
	else corres=new int[MAX*18];
	
	eulers=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	trans=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	scales=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	quats=new CQuaternion[MAX*MAX_NUMBER_OF_KEYS_NODE];
    
    for (n=0;n<MAX_NUMBER_OF_KEYS_NODE;n++)
    {
        eulers[n].Init(0,0,0);
        trans[n].Init(0,0,0);
        scales[n].Init(0,0,0);
        quats[n].Id();
    }

	Myfread(array,nb*sizeof(CVector),1,f);
	Myfread(euler,nb*sizeof(CVector),1,f);
	if (version==1001)
	{
		for (int kk=0;kk<nb;kk++)
		{
			translate[kk].Init(0,0,0);
			scale[kk].Init(1,1,1);
		}
	}
	else
	{
		Myfread(translate,nb*sizeof(CVector),1,f);
		Myfread(scale,nb*sizeof(CVector),1,f);
	}
	Myfread(quat,nb*sizeof(CQuaternion),1,f);

	if (version==1007) Myfread(corres,nb*130*sizeof(int),1,f);
	else Myfread(corres,nb*18*sizeof(int),1,f);

	if (version==1001)
	{
		Myfread(eulers,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CVector),1,f);
		Myfread(quats,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CQuaternion),1,f);
		for (int kk=0;kk<nb*MAX_NUMBER_OF_KEYS_NODE_OLD;kk++)
		{
			trans[kk].Init(0,0,0);
			scales[kk].Init(1,1,1);
		}
	}
	else
	{
		if (version==1002)
		{
			Myfread(eulers,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CVector),1,f);
			Myfread(trans,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CVector),1,f);
			Myfread(scales,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CVector),1,f);
			Myfread(quats,nb*MAX_NUMBER_OF_KEYS_NODE_OLD*sizeof(CQuaternion),1,f);	
		}
		else
		{
            if ((version!=1005)&&(version!=1006)&&(version!=1007))
            {
                Myfread(eulers,nb*MAX_NUMBER_OF_KEYS_NODE4*sizeof(CVector),1,f);
                Myfread(trans,nb*MAX_NUMBER_OF_KEYS_NODE4*sizeof(CVector),1,f);
                Myfread(scales,nb*MAX_NUMBER_OF_KEYS_NODE4*sizeof(CVector),1,f);
                Myfread(quats,nb*MAX_NUMBER_OF_KEYS_NODE4*sizeof(CQuaternion),1,f);
            }
            else
            {
                Myfread(eulers,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
                Myfread(trans,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
                Myfread(scales,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,f);
                Myfread(quats,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CQuaternion),1,f);
            }
		}
	}
	
	for (n=0;n<nb;n++) nodes[n]=new CNode(NULL,n);

	if (version==1007)
	{
		for (n=0;n<nb;n++)
		{
			if (corres[n*130+1]==-1) root=nodes[n];
			else nodes[n]->prev=nodes[corres[n*130+1]];
			for (i=0;i<corres[n*130+0];i++) nodes[n]->Add(nodes[corres[n*130+2+i]]);

			nodes[n]->SetPivot(array[n].x,array[n].y,array[n].z);

			nodes[n]->SetEuler(euler[n].x,euler[n].y,euler[n].z);
			nodes[n]->SetTranslate(translate[n].x,translate[n].y,translate[n].z);
			nodes[n]->SetScale(scale[n].x,scale[n].y,scale[n].z);
			nodes[n]->SetQuaternion(quat[n]);

			for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
			{
				nodes[n]->Eulers[k]=eulers[n*MAX_NUMBER_OF_KEYS_NODE + k];
				nodes[n]->Pivots[k]=trans[n*MAX_NUMBER_OF_KEYS_NODE + k];
				nodes[n]->Scales[k]=scales[n*MAX_NUMBER_OF_KEYS_NODE + k];
				nodes[n]->Qs[k]=quats[n*MAX_NUMBER_OF_KEYS_NODE + k];
			}
		}
	}
	else
	{
		for (n=0;n<nb;n++)
		{
			if (corres[n*18+1]==-1) root=nodes[n];
			else nodes[n]->prev=nodes[corres[n*18+1]];
			for (i=0;i<corres[n*18+0];i++) nodes[n]->Add(nodes[corres[n*18+2+i]]);

			nodes[n]->SetPivot(array[n].x,array[n].y,array[n].z);

			nodes[n]->SetEuler(euler[n].x,euler[n].y,euler[n].z);
			nodes[n]->SetTranslate(translate[n].x,translate[n].y,translate[n].z);
			nodes[n]->SetScale(scale[n].x,scale[n].y,scale[n].z);
			nodes[n]->SetQuaternion(quat[n]);

			if ((version!=1003)&&(version!=1004)&&(version!=1005)&&(version!=1006))
			{
				for (k=0;k<MAX_NUMBER_OF_KEYS_NODE_OLD;k++)
				{
					nodes[n]->Eulers[k]=eulers[n*MAX_NUMBER_OF_KEYS_NODE_OLD + k];
					nodes[n]->Pivots[k]=trans[n*MAX_NUMBER_OF_KEYS_NODE_OLD + k];
					nodes[n]->Scales[k]=scales[n*MAX_NUMBER_OF_KEYS_NODE_OLD + k];
					nodes[n]->Qs[k]=quats[n*MAX_NUMBER_OF_KEYS_NODE_OLD + k];
				}
			}
			else
			{
				if ((version!=1005)&&(version!=1006))
				{
					for (k=0;k<MAX_NUMBER_OF_KEYS_NODE4;k++)
					{
						nodes[n]->Eulers[k]=eulers[n*MAX_NUMBER_OF_KEYS_NODE4 + k];
						nodes[n]->Pivots[k]=trans[n*MAX_NUMBER_OF_KEYS_NODE4 + k];
						nodes[n]->Scales[k]=scales[n*MAX_NUMBER_OF_KEYS_NODE4 + k];
						nodes[n]->Qs[k]=quats[n*MAX_NUMBER_OF_KEYS_NODE4 + k];
					}
				}
				else
				{
					for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
					{
						nodes[n]->Eulers[k]=eulers[n*MAX_NUMBER_OF_KEYS_NODE + k];
						nodes[n]->Pivots[k]=trans[n*MAX_NUMBER_OF_KEYS_NODE + k];
						nodes[n]->Scales[k]=scales[n*MAX_NUMBER_OF_KEYS_NODE + k];
						nodes[n]->Qs[k]=quats[n*MAX_NUMBER_OF_KEYS_NODE + k];
					}
				}
			}
		}
	}


	if ((version==1004)||(version==1005))
	{
		char strdata[128*32];
		Myfread(strdata,32*128,1,f);
		int pp=0;
		for (n=0;n<nb;n++)
		{
			char *s=nodes[n]->name;
			int p=0;
			while (strdata[pp]!=0)
			{
				s[p++]=strdata[pp];
				pp++;
			}
			pp++;
			s[p++]=0;
		}
	}

    if (version==1006)
    {
        char strdata[MAX*32];
        Myfread(strdata,32*192,1,f);
        int pp=0;
        for (n=0;n<nb;n++)
        {
            char *s=nodes[n]->name;
            int p=0;
            while (strdata[pp]!=0)
            {
                s[p++]=strdata[pp];
                pp++;
            }
            pp++;
            s[p++]=0;
        }
    }

	if (version==1007)
    {
        char strdata[MAX*32];
        Myfread(strdata,32*MAX,1,f);
        int pp=0;
        for (n=0;n<nb;n++)
        {
            char *s=nodes[n]->name;
            int p=0;
            while (strdata[pp]!=0)
            {
                s[p++]=strdata[pp];
                pp++;
            }
            pp++;
            s[p++]=0;
        }
    }
	Palette=new CMatrix[nb];
	Palette0=new CMatrix[nb];
	for (n=0;n<nb;n++) Myfread(&Palette0[n],sizeof(CMatrix),1,f);

	delete [] corres;
	delete [] array;
	delete [] euler;
	delete [] translate;
	delete [] scale;
	delete [] quat;
	delete [] eulers;
	delete [] trans;
	delete [] scales;
	delete [] quats;

	return (tag==1);
}

void memread(void * ptr,int size,int nb,char * file,int *pos)
{
	memcpy(ptr,file+(*pos),size*nb);
	(*pos)+=size*nb;
}

void memwrite(void * ptr,int size,int nb,char * file,int *pos)
{
	memcpy(file+(*pos),ptr,size*nb);
	(*pos)+=size*nb;
}

bool CTree::Sanitize(int * relative)
{
    int n;
    CNode *array[256];
    int p=0;
    int nb=0;
    CNode *nod;
    
    GetNbEntries(root,&nb);
    
    for (n=0;n<nb;n++)
    {
        nod=NULL;
        GetNodeById(root,&nod,n);
        if (nod)
        {
            relative[n]=p;
            array[n]=nod;
            p++;
        }
        else array[n]=NULL;
    }

    if (p==nb) return false;
    
    for (n=0;n<nb;n++)
    {
        nod=NULL;
        GetNodeById(root,&nod,n);
        if (nod)
        {
            for (int k=0;k<nod->nb;k++)
            {
                nod->next[k]=array[nod->next[k]->id];
            }
        }
    }

    for (n=0;n<nb;n++)
    {
        nod=NULL;
        GetNodeById(root,&nod,n);
        if (nod)
        {
            nod->id=relative[nod->id];
        }
    }

    return (p!=nb);
}

int CTree::Store(char * buffer)
{
	int n,i,k;
	int nb=0;
	CVector *array;
	CVector *euler;
	CVector *translate;
	CVector *scale;
	CQuaternion *quat;
	CVector *eulers;
	CVector *trans;
	CVector *scales;
	CQuaternion *quats;
	CNode *nod;
	int *corres;
	int pos=0;
    
	GetNbEntries(root,&nb);

	array=new CVector[nb];

	euler=new CVector[nb];
	translate=new CVector[nb];
	scale=new CVector[nb];
	quat=new CQuaternion[nb];

	eulers=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	trans=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	scales=new CVector[nb*MAX_NUMBER_OF_KEYS_NODE];
	quats=new CQuaternion[nb*MAX_NUMBER_OF_KEYS_NODE];

	corres=new int[nb*130];

	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		array[n]=nod->Pivot;
		euler[n]=nod->Euler;
		translate[n]=nod->Translate;
		scale[n]=nod->Scale;
		quat[n]=nod->Q;

		for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
		{
			eulers[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Eulers[k];
			trans[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Pivots[k];
			scales[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Scales[k];
			quats[n*MAX_NUMBER_OF_KEYS_NODE + k]=nod->Qs[k];
		}

		for (i=0;i<130;i++) corres[n*130+i]=-1;
		corres[n*130+0]=nod->nb;

		if (nod->prev) corres[n*130+1]=nod->prev->id;
		else corres[n*130+1]=-1;

		for (i=0;i<nod->nb;i++) corres[n*130+2+i]=nod->next[i]->id;
	}

	memwrite(&nb,sizeof(int),1,buffer,&pos);
	memwrite(&tag,sizeof(int),1,buffer,&pos);
	memwrite(&Coo,sizeof(CVector),1,buffer,&pos);
	memwrite(&Rot,sizeof(CVector),1,buffer,&pos);

	memwrite(array,nb*sizeof(CVector),1,buffer,&pos);

	memwrite(euler,nb*sizeof(CVector),1,buffer,&pos);
	memwrite(translate,nb*sizeof(CVector),1,buffer,&pos);
	memwrite(scale,nb*sizeof(CVector),1,buffer,&pos);
	memwrite(quat,nb*sizeof(CQuaternion),1,buffer,&pos);

	memwrite(corres,nb*130*sizeof(int),1,buffer,&pos);

	memwrite(eulers,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memwrite(trans,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memwrite(scales,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memwrite(quats,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CQuaternion),1,buffer,&pos);

	// 1004
	char strdata[MAX*32];
	memset(strdata,0,MAX*32);
	int pp=0;
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&nod,n);
		char *s=nod->name;
		int p=0;
		while (s[p]!=0) strdata[pp++]=s[p++];
		strdata[pp++]=0;
	}

	memwrite(strdata,32*MAX,1,buffer,&pos);

	for (n=0;n<nb;n++) memwrite(&Palette0[n],sizeof(CMatrix),1,buffer,&pos);

	delete [] corres;
	delete [] array;
	delete [] euler;
	delete [] translate;
	delete [] scale;
	delete [] quat;
	delete [] eulers;
	delete [] scales;
	delete [] trans;
	delete [] quats;

	return pos;
}

bool CTree::Read(char *buffer,int version)
{
	int n,i,k;
	int nb=0;
	CVector *array;
	CVector *euler;
	CVector *translate;
	CVector *scale;
	CQuaternion *quat;
	CNode *nodes[MAX];
	int *corres;
	CVector *eulers;
	CVector *trans;
	CVector *scales;
	CQuaternion *quats;
	int pos=0;
	
	memread(&nb,sizeof(int),1,buffer,&pos);
	memread(&tag,sizeof(int),1,buffer,&pos);
	memread(&Coo,sizeof(CVector),1,buffer,&pos);
	memread(&Rot,sizeof(CVector),1,buffer,&pos);

	array=new CVector[MAX];
	euler=new CVector[MAX];
	translate=new CVector[MAX];
	scale=new CVector[MAX];
	quat=new CQuaternion[MAX];
	corres=new int[MAX*130];
	
	eulers=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	trans=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	scales=new CVector[MAX*MAX_NUMBER_OF_KEYS_NODE];
	quats=new CQuaternion[MAX*MAX_NUMBER_OF_KEYS_NODE];

	memread(array,nb*sizeof(CVector),1,buffer,&pos);
	memread(euler,nb*sizeof(CVector),1,buffer,&pos);
	memread(translate,nb*sizeof(CVector),1,buffer,&pos);
	memread(scale,nb*sizeof(CVector),1,buffer,&pos);
	memread(quat,nb*sizeof(CQuaternion),1,buffer,&pos);
	memread(corres,nb*130*sizeof(int),1,buffer,&pos);

	memread(eulers,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memread(trans,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memread(scales,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CVector),1,buffer,&pos);
	memread(quats,nb*MAX_NUMBER_OF_KEYS_NODE*sizeof(CQuaternion),1,buffer,&pos);

	for (n=0;n<nb;n++) nodes[n]=new CNode(NULL,n);

	for (n=0;n<nb;n++)
	{
		if (corres[n*130+1]==-1) root=nodes[n];
		else nodes[n]->prev=nodes[corres[n*130+1]];
		for (i=0;i<corres[n*130+0];i++) nodes[n]->Add(nodes[corres[n*130+2+i]]);

		nodes[n]->SetPivot(array[n].x,array[n].y,array[n].z);
		nodes[n]->SetEuler(euler[n].x,euler[n].y,euler[n].z);
		nodes[n]->SetQuaternion(quat[n]);
		for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
		{
			nodes[n]->Eulers[k]=eulers[n*MAX_NUMBER_OF_KEYS_NODE + k];
			nodes[n]->Pivots[k]=trans[n*MAX_NUMBER_OF_KEYS_NODE + k];
			nodes[n]->Scales[k]=scales[n*MAX_NUMBER_OF_KEYS_NODE + k];
			nodes[n]->Qs[k]=quats[n*MAX_NUMBER_OF_KEYS_NODE + k];
		}
	}

	char strdata[MAX*32];
	memread(strdata,32*MAX,1,buffer,&pos);
	int pp=0;
	for (n=0;n<nb;n++)
	{
		char *s=nodes[n]->name;
		int p=0;
		while (strdata[pp]!=0)
		{
			s[p++]=strdata[pp];
			pp++;
		}
		pp++;
		s[p++]=0;
	}

	root=nodes[0];
	Palette=new CMatrix[nb];
	Palette0=new CMatrix[nb];
	for (n=0;n<nb;n++) memread(&Palette0[n],sizeof(CMatrix),1,buffer,&pos);

	delete [] corres;
	delete [] array;
	delete [] euler;
	delete [] translate;
	delete [] scale;
	delete [] quat;
	delete [] eulers;
	delete [] trans;
	delete [] scales;
	delete [] quats;

	return (tag==1);
}

void CNVEulers(CVector &r,CQuaternion &q,CVector *e)
{
    float x,y,z;
    CMatrix M;
    
    M.Id();
    M.RotationDegre(r);
    M.Mq(q);
    
    M.Eulers(&x,&y,&z);
    
    e->x=(180.0f*x/PI);
    e->y=(180.0f*y/PI);
    e->z=(180.0f*z/PI);
}

void CTree::Fix2Eulers()
{
    int n,k;
    CVector neweulers;
    CNode * nod;
    int nb=0;
    
    GetNbEntries(root,&nb);
    
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);
        
        CNVEulers(nod->Euler,nod->Q,&neweulers);
        nod->Euler=neweulers;
        nod->Q.Id();
        
        for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
        {
            CNVEulers(nod->Eulers[k],nod->Qs[k],&neweulers);
            nod->Eulers[k]=neweulers;
            nod->Qs[k].Id();
        }
    }
}

void CTree::Fix2Quats()
{
    int n,k;
    CQuaternion q;
    CNode * nod;
    int nb=0;
    
    GetNbEntries(root,&nb);
    
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);

        q.Id();
        q.SetEuler(nod->Euler.x,nod->Euler.y,nod->Euler.z);
        nod->Q=nod->Q*q;
        nod->Q.Normalise();
        nod->Euler.Init(0,0,0);

        for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
        {
            q.Id();
            q.SetEuler(nod->Eulers[k].x,nod->Eulers[k].y,nod->Eulers[k].z);
            nod->Qs[k]=nod->Qs[k]*q;
            nod->Qs[k].Normalise();
            nod->Eulers[k].Init(0,0,0);
        }
    }
}

void CNVEulers2(CMatrix &m,CMatrix &oldm,CVector *e)
{
    float x,y,z;
    CMatrix M,im;
    
    im.Inverse4x4(oldm);
    M=im*m;
    M.Eulers(&x,&y,&z);
    
    e->x=(180.0f*x/PI);
    e->y=(180.0f*y/PI);
    e->z=(180.0f*z/PI);
}


void CTree::Centers()
{
	float f,fmax;
	CVector u,tr;
	int n,k;
	int nb=0;
	CNode *node;
	GetNbEntries(root,&nb);

	ReadKey(0);
	MatrixPalette();

	fmax=0;
	for (n=0;n<nb;n++)
	{
		GetNodeById(root,&node,n);
		u=node->Pivot*Palette[node->id];
		f=VECTORNORM2(u);
		if (f>fmax) fmax=f;
	}

	for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
	{
		tr.Init(0,0,0);

		ReadKey(k);
		MatrixPalette();

		int nn=0;
		for (n=0;n<nb;n++)
		{
			GetNodeById(root,&node,n);
			u=node->Pivot*Palette[node->id];
			if (n==0) NZs[k]=u;
			f=VECTORNORM2(u);
			if (f<fmax*4)
			{
				tr+=u;
				nn++;
			}
		}

		tr.y*=0.25f;

		if (nn>0) Gs[k]=tr/nn;
		else Gs[k]=tr;
	}
}

void CTree::FixThePose()
{
    int nb=0;
    CNode *nod;
    CMatrix Ms[256];
    CVector Rs[256];
    CQuaternion Qs[256];
    CMatrix M;
    CMatrix Mo,oldMo;
    CVector zero(0,0,0);
    CVector neweulers,newtrans,piv;
    int n,k;
    
    M.Id();
    
    GetNbEntries(root,&nb);
    
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);
        Ms[nod->id]=Palette[nod->id];
        Rs[nod->id]=nod->Euler;
        Qs[nod->id]=nod->Q;
    }
    
    for (k=0;k<MAX_NUMBER_OF_KEYS_NODE;k++)
    {
        ReadKey(k);
        
        M.Id();
        CalcMatrix(M,root);
        
        for (n=0;n<nb;n++)
        {
            GetNodeById(root,&nod,n);
            piv=nod->Pivot*Ms[nod->id];
            
            Mo.Id();
            Mo.RotationDegre(nod->Eulers[k]);
            Mo.Mq(nod->Qs[k]);

            oldMo.Id();
            oldMo.RotationDegre(Rs[nod->id]);
            oldMo.Mq(Qs[nod->id]);

            CNVEulers2(Mo,oldMo,&neweulers);
            nod->Eulers[k]=neweulers;
            nod->Qs[k].Id();
        }
    }
    
    for (n=0;n<nb;n++)
    {
        GetNodeById(root,&nod,n);
        
        nod->Pivot=nod->Pivot*Ms[nod->id];
        nod->Euler.Init(0,0,0);
        nod->Q.Id();
    }
}

#endif
