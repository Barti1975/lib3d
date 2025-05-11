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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	BASIC3D_SCENE.H
//
//	original form v1.0b
//
//	by Laurent CANCE 
//	April 2003
//
//	Read BASIC3D format exported from 3DSMAX by VoidExporter/Flexporter
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  

	/*?////////////////////////////////////////////////////////////////////////////////////////////////////////////
	<P>
	BASIC3D_MATERIAL
	data structure containing primary informations about materials
	<i>diffuseID</i> : BASIC3D_TEXTURE id for diffuse
	<i>specularID</i> : BASIC3D_TEXTURE id for specular
	<i>bumpID</i> : BASIC3D_TEXTURE id for bump
	<i>ar,ag,ab</i> : ambient color
	<i>r,g,b</i> : diffuse color
	<i>sr,sg,sb</i> : specular color
	</P>

	<P>
	BASIC3D_TEXTURE
	data structure</i> containing primary informations about textures
	<i>filename</i> : string name of the file to load
	<i>local_nt</i> : calling DoStuffs affect this value to a valid texture index
	<i>Alpha</i> : indicates if texture must be drawn blended
	<i>Tile(U,V),Offset(U,V),Scale(U,V),Transform</i> : mapping coordinate transform
	</P>

	<P>
	BASIC3D_Mesh
	basic structure englobing informations about meshes in scene
	this structure has some initialisation routines (SetKeys,Affect)

	<i>M,Mo,WM,WMo,iM,iMo,ifM,fM,fMo,fRoot</i> : matrices
	<i>obj</i> : CObject3D of the mesh
	<i>vb</i> : CVertexBuffer* place for rendering
	<i>name</i> : string name 
	<i>env_mapping</i> : the mesh has specular environment mapping
	<i>keys</i> : keyframing
	</P>

	<P>
	drawing:

		ComputeFrames(-1,0); initializes matrix with initial frame
		matrices are stored in m->ifM ou d3dframed in m->WM
		vertexbuffer allocated can be drawn with usual routines accessing the list of vertexbuffer
		in CList Meshes->vb or obj

	</P>
	////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


#ifndef _BASIC3D_SCENE_H_
#define _BASIC3D_SCENE_H_

#include <stdio.h>
#include "../base/params.h"
#include "../base/maths.h"
#include "../base/objects3d.h"
#include "../base/3d_api_base.h"
#ifdef _DEFINES_API_CODE_TREES_
#  include "../misc/trees.h"
#endif
#include "lzcompress.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	T_WRAP		=	1,
	T_MIRROR	=	2,
	T_CLAMP		=	4,

	SAVE3D_LIGHTCOMPRESS	=	1,
	SAVE3D_NOANIMS			=	2,
	SAVE3D_ANIMONLY			=	4
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int nobjs;
	int ncams;
	int nlights;
	int nanims;
	int nmat;
	int ntex;

} BASIC3D_HDR;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int framestart;
	int frameend;
	int frame_rate;

} BASIC3D_INFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int matID;
	int diffuseID;
	int specularID;
	int bumpID;
	float ar;
	float ag;
	float ab;
	float r;
	float g;
	float b;
	float sr;
	float sg;
	float sb;

} BASIC3D_MATERIAL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int texID;
	char * filename;
	int local_nt;

	bool Alpha;

	int TileU,TileV;
	float OffsetU,OffsetV;
	float ScaleU,ScaleV;

	CMatrix Transform;
    
    char idname[128];
    int tag;
    
    int active;

} BASIC3D_TEXTURE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum 
{
	MESH				=	1,
	SKELETON			=	2,
	SKIN				=	4,
	HELPER				=	8,
	CAMERA				=	16,
	FREE				=	32,
	TARGET				=	64,
	PARALLEL			=	128,
	ORTHOGRAPHIC		=	256,
	PERSPECTIVE			=	512,
	_LIGHT				=	1024,
	DIRECTIONNAL		=	2048,
	SPOTLIGHT			=	4096,
	OMNIDIRECTIONNAL	=	8192
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	ROTATION	=	1,
	POSITION	=	2,
	SCALING		=	4,
	LINEAR		=	8,
	TCB			=	16,
	BEZIER		=	32,
	MORPH		=	64
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BASIC3D_Keyframe
{
public:
	
	CQuaternion *q;
	CVector *p;

	CQuaternion *qs;
	CVector *ps;

	BASIC3D_Keyframe() { q=NULL;p=NULL;qs=NULL;ps=NULL; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BASIC3D_KeyframeTmp
{
public:
	
	CQuaternion q;
	CVector p;

	BASIC3D_KeyframeTmp() { q.Id(); p.Init(0,0,0); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float time;
	float value;
} ANM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int index;
	char name[1024];
	int nb;
	CMatrix T,TLINK,TLINKTR;
    long long id;
    int tl;

} BASIC3D_Cluster;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BASIC3D_Limb  // FBX
{
public:
	CVector t,r,s;
	CVector P;
	char name[1024];
	int skin;
	CMatrix T,TLINK,TLINKTR,POSE,REF;
	BASIC3D_Limb * next[1024];
	BASIC3D_Limb * prev;
    CVector nodepos;
    int nmodel;
	int nbnext;
	int index;
	int oldindex;
	int nb;
    int tl;
    long long id;
	CList <ANM> TX[32];
	CList <ANM> TY[32];
	CList <ANM> TZ[32];
	CList <ANM> RX[32];
	CList <ANM> RY[32];
	CList <ANM> RZ[32];
	CList <ANM> SX[32];
	CList <ANM> SY[32];
	CList <ANM> SZ[32];
	CVector base;
	int nANM;
	bool active;
	bool translate;
	bool reaffected;
    int noding;
    int depth;

	BASIC3D_Limb()
	{
        tl=0;
        nmodel=0;
		reaffected=false;
		translate=true;
		active=true;
		nb=0;
		nANM=0;
		nbnext=0;
		oldindex=index=-1;
		for (int n=0;n<16;n++) next[n]=NULL;
		prev=NULL;
		skin=-1;
		P.Init(0,0,0);
		t.Init(0,0,0); r.Init(0,0,0); s.Init(0,0,0);
	};

	~BASIC3D_Limb()
	{
		for (int n=0;n<32;n++)
		{
			SX[n].Free(); SY[n].Free(); SZ[n].Free();
			RX[n].Free(); RY[n].Free(); RZ[n].Free();
			TX[n].Free(); TY[n].Free(); TZ[n].Free();
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BASIC3D_Mesh
{
public:

	char * name;
	int attr;
	int ParentID;
	int topoID;
	CObject3D *obj;
	CVertexBuffer *vb;
	CVertexBuffer *edvb;
	CMap <unsigned int,BASIC3D_Keyframe> keys;
	BASIC3D_KeyframeTmp *animkeys;
	int nanimkeys;
	CMatrix M,Mo;
	CMatrix WM,WMo;
	CMatrix Mp,Mpo;
	CMatrix iM,iMo;
	CMatrix ifM,fM,fMo,fRoot;
	CVector p;
	CQuaternion q;
	CVector s;
	CVector pivot_p;
	CQuaternion pivot_q;
	CVector pivot_s;
	int fstart,fend;
	int tag;
	int sampling_rate;
	int nsamplekeys;
	bool env_mapping;
	long long longid;
	CList <BASIC3D_Limb*> limbs;
	CList <BASIC3D_Cluster> clusters;
	CVector _t,_s,_r,_pre;
	bool forapply;

#ifdef _DEFINES_API_CODE_TREES_
	CTree *tree;
	CNode * array[1024];
#endif

	BASIC3D_Mesh() 
	{ 
#ifdef _DEFINES_API_CODE_TREES_
		tree=NULL; 
#endif
		animkeys=NULL; obj=NULL;vb=NULL; ParentID=-1; keys.Free(); edvb=NULL; env_mapping=false; 
		_t.Init(0,0,0);
		_r.Init(0,0,0);
		_pre.Init(0,0,0);
		_s.Init(1,1,1);
	}

	void SetKeys();
	void Affect();
    BASIC3D_Limb * GetLimb(char *name);
    BASIC3D_Cluster * GetCluster(char *name);
    BASIC3D_Limb * GetLimbById(long long id);
    BASIC3D_Cluster * GetClusterById(long long id);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	LOAD_TEXTURES				=	0,
	APPLY_TEXTURE_TRANSFORM		=	1,
	APPLY_MATERIAL_COEFFICIENT	=	2,


	ACTIVATED					=	1,
	DESACTIVATED				=	0
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BASIC3D
{
public:

	bool SmgTyp;
	C3DAPIBASE * api;
	CList <BASIC3D_MATERIAL> Materials;
	CList <BASIC3D_TEXTURE> Textures;
	CMap <int,BASIC3D_Mesh> Meshes;
	CList <BASIC3D_Limb> Limbs;
	int keystart,keyend;
	int frame_rate;
	int blanc;
	unsigned char flags[8];
	char groupname[512];
	float TimeMax[32];
	int nANM;

	BASIC3D() { nANM=0;SmgTyp=false;flags[0]=1;flags[1]=1;flags[2]=1;flags[3]=0;flags[4]=0;flags[5]=0;flags[6]=0;flags[7]=0;groupname[0]='\0'; }

	//! get material
	BASIC3D_MATERIAL * GetMaterial(int id);
	//! get texture
	BASIC3D_TEXTURE * GetTexture(int id);
	//! get mesh by name
	BASIC3D_Mesh * GetMesh(char *name);
	//! get limb by name
	BASIC3D_Limb * GetLimb(char *name);
    //! get mesh by limb name
	BASIC3D_Mesh * GetMeshFromLimb(char *name);
    
    //! get limb by id
    BASIC3D_Limb * GetLimbById(long long id);
    //! get cluster by id
    BASIC3D_Cluster * GetClusterById(long long id);


	//! Setting options
	void SetFlag(int n,int value);
	//! Loading a scene
	void LoadBASIC3D(char *str);
	//! Loading an old engine object or a BASIC3D scene
	void Load(char * str);
	//! Saving of a scene
	void Save(char *name);
	//! Parametrisable saving of a scene
	void Save(char *name,int flags);
	//! Saving of a scene in .ASC
	void SaveASC(char *name);
	//! Saving of a scene in .X
	void SaveX(char *name);
	//! Saving of a scene in .3DS
	void Save3DS(char *name);
	//! Initialisation function
	void SetAPI(C3DAPIBASE * Api) { api=Api;}
	//! Calculate object transformations
	void ComputeFrame(int frame,float t);
	//! (obsolète)
	void CalculateVertexBuffers(unsigned int flags);
	//! (obsolète)
	void FreeVertexBuffers();
	//! (obsolète)
	void UpdateVertexBuffers();
	//! Killing
	void Free();
	//! Init Keyframmer
	void SetKeyframer();
	//! Smoothing group settings
	void ToggleSmoothing();
	//! Smoothing group settings
	void ToggleSmoothing(bool b);
	//! Preparing, mesh, textures, mapping coo.
	void DoStuffs(int nt_base,char * repertoire);
	//! Preparing, mesh, textures, mapping coo.
	void DoStuffs(char *group,char * repertoire);
	//! Cleaning
	void UndoStuffs();
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
