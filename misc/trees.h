
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

#ifndef _TREES_H_
#define _TREES_H_

#ifdef _DEFINES_API_CODE_TREES_

#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
#include "../data/file_io_zip.h"
#define _Myfread ZIPfread_
#define _Myfopen ZIPfopen
#define _Myfclose ZIPfclose
//#define MyFILE ZIPFILE
#else
#define _Myfread fread
#define _Myfopen fopen
#define _Myfclose fclose
//#define MyFILE FILE
#endif

#ifdef CLASSNED
#include "../../ned/myfile.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TREES_VERSION_BETA		0000
#define TREES_VERSION_1_001		1001
#define TREES_VERSION_1_002		1002
#define TREES_VERSION_1_003		1003
#define TREES_VERSION_1_004     1004
#define TREES_VERSION_1_005     1005
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "../base/maths.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_NUMBER_OF_KEYS_NODE         1024
#define MAX_NUMBER_OF_KEYS_NODE4        256
#define MAX_NUMBER_OF_KEYS_NODE_OLD     128
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CNode
{
public:
	int id;
	CVector Pivot;
	CQuaternion Q;
	CVector Euler;
	CVector Scale;
	CVector Translate;
    CVector Alteration;
    CVector Alter;

	CVector _Pivot;
	CQuaternion _Q;
	CVector _Euler;
	CVector _Scale;
	CVector _Translate;
    
	char name[128];

	unsigned char scl;

	CQuaternion Qs[MAX_NUMBER_OF_KEYS_NODE];
	CVector Eulers[MAX_NUMBER_OF_KEYS_NODE];
	CVector Pivots[MAX_NUMBER_OF_KEYS_NODE];
	CVector Scales[MAX_NUMBER_OF_KEYS_NODE];

	CMatrix T,TLINK,PRE,ROT;

	CNode *prev;
	CNode *next[128];
#ifdef _DEFINES_API_CODE_PHYSIC_
	CPhysicJoint* joint[128];
#endif

	float Length;
	float sizex,sizey;
	float sizex2,sizey2;
	CVector dir,Calc;
	int nb;
	int tag;
	int added_num;
	CVector added_decal;
	CVector added_decal2;
	int externid;
    int speed;

	CNode()
	{
		int n;		

		id=0;

		scl=0;
		added_num=-1;
		for (n=0;n<128;n++) next[n]=NULL;
#ifdef _DEFINES_API_CODE_PHYSIC_
		for (n=0;n<128;n++) joint[n]=NULL;
#endif
		prev=NULL;
		nb=0;
		Euler.Init(0,0,0);
		Translate.Init(0,0,0);
        Pivot.Init(0,0,0);
        Alteration.Init(0,0,0);
        Alter.Init(0,0,0);
		Scale.Init(1,1,1);
		Q.Id();
        
        speed=1;

		_Euler.Init(0,0,0);
		_Translate.Init(0,0,0);
		_Pivot.Init(0,0,0);
		_Scale.Init(1,1,1);
		_Q.Id();
		tag=0;

		for (n=0;n<MAX_NUMBER_OF_KEYS_NODE;n++)
		{
			Qs[n].Id();
			Eulers[n].Init(0,0,0);
			Pivots[n].Init(0,0,0);
			Scales[n].Init(1,1,1);
		}

		memset(name,0,32);
	}

	CNode(CNode *father,int ID)
	{
	    int n;
		scl=0;
		id=ID;

		added_num=-1;
		for (n=0;n<128;n++) next[n]=NULL;
#ifdef _DEFINES_API_CODE_PHYSIC_
		for (n=0;n<128;n++) joint[n]=NULL;
#endif
		prev=father;
		nb=0;
		Euler.Init(0,0,0);
		Translate.Init(0,0,0);
		Pivot.Init(0,0,0);
        Alteration.Init(0,0,0);
        Alter.Init(0,0,0);
		Scale.Init(1,1,1);
		Q.Id();
        
        speed=1;
        
		_Euler.Init(0,0,0);
		_Translate.Init(0,0,0);
		_Pivot.Init(0,0,0);
		_Scale.Init(1,1,1);
		_Q.Id();

		for (n=0;n<MAX_NUMBER_OF_KEYS_NODE;n++)
		{
			Qs[n].Id();
			Eulers[n].Init(0,0,0);
			Pivots[n].Init(0,0,0);
			Scales[n].Init(1,1,1);
		}

		memset(name,0,32);
		sprintf(name,"node%d",ID);
		tag=0;
	}

	void SetPivot(float x,float y,float z);
	bool Add(CNode *child);
	void Del(CNode *child);
	void SetId(int i);
	void SetEuler(float rx,float ry,float rz);
	void SetTranslate(float tx,float ty,float tz);
	void SetScale(float sx,float sy,float sz);
	void SetQuaternion(CQuaternion &q);
    int IndexNode(CNode *child);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTree
{
public:
	CNode *root;
	CMatrix *Palette;
	CMatrix *Palette0;
	CVector Coo;
	CVector Rot;
	int tag;
	int mapping;
    int k1,k2;
    float kt;
    int l1,l2;
    float twot;
    float lt;
    bool two;
    int sane;
    int flags[4096];
	CVector Gs[MAX_NUMBER_OF_KEYS_NODE];
	CVector NZs[MAX_NUMBER_OF_KEYS_NODE];
	bool applyalteration;
	int nbentries;

    CTree() { root=NULL; tag=0; mapping=-1; sane=0; nbentries=-1; ResetFlags(); two=false; applyalteration=false; }

	void Quadriped();
	void Biped();
	void Free();
	void MatrixPalette();
	void CalcMatrix(CMatrix &fm,CNode *nod);

	void Centers();
    
    void ResetFlags();
    
    void Fix2Eulers();
    void Fix2Quats();
    void FixThePose();

	void FitAndWeights(CObject3D *obj);
	
    void Translate(CNode *nod,float px,float py,float pz);
	void Translate(float px,float py,float pz);
    
	void Scale(CNode *nod,float sx,float sy,float sz);
    void Scale(float sx,float sy,float sz);
    
    void ScaleY(float sx,float sy,float sz,float sxd,float syd,float szd);
    void ScaleY(CNode *nod,float sx,float sy,float sz,float sxd,float syd,float szd);

	void GetScale_x(CNode *nod,float *s);
	void GetScale_y(CNode *nod,float *s);
	void GetScale_z(CNode *nod,float *s);
#ifndef CLASSNED
	bool Load(FILE *f,int version);
#else
	bool Load(MyFILE *f,int version);
#endif
	void Save(FILE *f);

	void GetNbEntries(CNode *nod,int *ndx);
	void GetNodeById(CNode *nod,CNode **res,int id);
    void GetNodeByName(CNode *nod,CNode **res,char *name);
    
	void Apply(CMatrix &m,CNode *nod);

	void Reset(CNode *nod);
	void Fix(CNode *nod);
	void AddLengthBonesAfterNode(CNode *nod,float add);
    
    void ResetAlteration();

	void InterpolateKeys(int key1,int key2,float t);
    void InterpolateKeysTwoAnim(int fstkey1,int fstkey2,float t1,int sndkey1,int sndkey2,float t2,float t);

	void InterpolateKeys(CNode *nod,int key1,int key2,float t);
	void InterpolateKeys(CNode *nod,int key,float t);
    void InterpolateKeysTwoAnim(CNode *nod,int fstkey1,int fstkey2,float t1,int sndkey1,int sndkey2,float t2,float t);

	void GetMin_x(CNode *nod,float *s);
	void GetMax_x(CNode *nod,float *s);
	void GetMin_y(CNode *nod,float *s);
	void GetMax_y(CNode *nod,float *s);
	void GetMin_z(CNode *nod,float *s);
	void GetMax_z(CNode *nod,float *s);

	void CleanKey(int key);
	void StoreKey(int key);
	void ReadKey(int key);

	void RestoreKey();
	void BackupKey();

	int Store(char * buffer);
	bool Read(char *buffer,int version);
    
    bool Sanitize(int * relative);
    
    bool ValidNed();

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
#endif
