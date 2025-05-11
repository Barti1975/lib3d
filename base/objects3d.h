
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

#ifndef _OBJETS3D_H_
#define _OBJETS3D_H_

#include "params.h"
#include "list.h"
#include "edges.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

enum
{
	SET_FACES_COPLANAR_RECURS_TAG	=	(1<<24),
	SET_FACES_TAG					=	(2<<24),
	SET_REF							=	(4<<24),
	SET_FACES_TAG2					=	(8<<24),
	SET_VERTICES_TAG				=	(16<<24),
	SET_FACES_COPLANAR_RECURS_TAG2	=	(32<<24)
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define FASTCALLCONV __fastcall
#else
#define FASTCALLCONV
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API3D_STUB CRGBA
{
public:
    float r,g,b,a;
	void Init(float R,float G,float B);
	void Init(float R,float G,float B,float A);

	float Intensity() { return (0.3f*r+0.6f*g+0.1f*b); }

	void saturate()
	{
		float i=r;
		if (g>i) i=g;
		if (b>i) i=b;

		r=r/i;
		g=g/i;
		b=b/i;
	}

	inline CRGBA operator +(CRGBA rgba2)	
	{
		CRGBA somme;

		somme.r=r + rgba2.r;
		somme.g=g + rgba2.g;
		somme.b=b + rgba2.b;
		somme.a=a + rgba2.a;

		return somme;
	}

	inline CRGBA operator -(CRGBA rgba2)	
	{
		CRGBA diff;

		diff.r=r - rgba2.r;
		diff.g=g - rgba2.g;
		diff.b=b - rgba2.b;
		diff.a=a - rgba2.a;

		return diff;
	}

	inline CRGBA operator *(float f)
	{
		CRGBA res;

		res.r=r*f;
		res.g=g*f;
		res.b=b*f;
		res.a=a*f;

		return res;
	}

	inline CRGBA operator /(float f)
	{
		CRGBA res;

		res.r=r/f;
		res.g=g/f;
		res.b=b/f;
		res.a=a/f;

		return res;
	}

	inline CRGBA operator *(int f)
	{
		CRGBA res;

		res.r=r*f;
		res.g=g*f;
		res.b=b*f;
		res.a=a*f;

		return res;
	}

	inline CRGBA operator /(int f)
	{
		CRGBA res;

		res.r=r/f;
		res.g=g/f;
		res.b=b/f;
		res.a=a/f;

		return res;
	}

	inline void operator /=(float f)
	{
		float surf;
		surf=1.0f/f;
		r*=surf;
		g*=surf;
		b*=surf;
		a*=surf;
	}

	inline void operator *=(float f)
	{
		r*=f;
		g*=f;
		b*=f;
		a*=f;
	}

	inline void operator /=(int f)
	{
		float surf;
		surf=1.0f/f;
		r*=surf;
		g*=surf;
		b*=surf;
		a*=surf;
	}

	inline void operator *=(int f)
	{
		r*=f;
		g*=f;
		b*=f;
		a*=f;
	}

	inline void operator *=(CRGBA c)
	{
		r*=c.r;
		g*=c.g;
		b*=c.b;
		a*=c.a;
	}

	inline void operator +=(CRGBA c)
	{
		r+=c.r;
		g+=c.g;
		b+=c.b;
		a+=c.a;
	}

	inline void operator -=(CRGBA c)
	{
		r-=c.r;
		g-=c.g;
		b-=c.b;
		a-=c.a;
	}

	inline CRGBA operator *(CRGBA c)			
	{
		CRGBA res;

		res.r=r*c.r;
		res.g=g*c.g;
		res.b=b*c.b;
		res.a=a*c.a;

		return res;
	}


	inline friend CRGBA operator *(float f,CRGBA rgb)			
	{
		CRGBA res;

		res.r=rgb.r*f;
		res.g=rgb.g*f;
		res.b=rgb.b*f;
		res.a=rgb.a*f;

		return res;
	}

    inline void sat()
    {
        if (r>255) r=255;
        if (g>255) g=255;
        if (b>255) b=255;
        if (a>255) a=255;
    }

    inline void mini(CRGBA rgb)
    {
        if (r<rgb.r) r=rgb.r;
        if (g<rgb.g) g=rgb.g;
        if (b<rgb.b) b=rgb.b;
        if (a<rgb.a) a=rgb.a;
    }

    inline void maxi(CRGBA rgb)
    {
        if (r>rgb.r) r=rgb.r;
        if (g>rgb.g) g=rgb.g;
        if (b>rgb.b) b=rgb.b;
        if (a>rgb.a) a=rgb.a;
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PHYSIC OBJECT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "physic_object.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	VERTEX 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class API3D_STUB CVertexAdd
{
public:
	short int nbones;
	CVector offsets[32];
	float weigths[32];
	short int bonesID[32];
	short int bones[32];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UCVector
{
public:
    float x,y,z;
    
    operator CVector() const { return CVector((float)x,(float)y,(float)z); }
    
    inline void operator =(CVector v)
    {
        x=v.x;
        y=v.y;
        z=v.z;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UCVector2
{
public:
    float x,y;
    
    operator CVector2() const { return CVector2(x,y); }
    
    inline void operator =(CVector2 v)
    {
        x=v.x;
        y=v.y;
    }

    inline UCVector2 operator -(UCVector2 v)
    {
        UCVector2 tmp;
        tmp.x=x-v.x;
        tmp.y=y-v.y;
        return tmp;
    }

    inline UCVector2 operator -(CVector2 v)
    {
        UCVector2 tmp;
        tmp.x=x-v.x;
        tmp.y=y-v.y;
        return tmp;
    }

    inline UCVector2 operator +(UCVector2 v)
    {
        UCVector2 tmp;
        tmp.x=x+v.x;
        tmp.y=y+v.y;
        return tmp;
    }

    inline UCVector2 operator +(CVector2 v)
    {
        UCVector2 tmp;
        tmp.x=x+v.x;
        tmp.y=y+v.y;
        return tmp;
    }

    inline UCVector2 operator /(int d)
    {
        UCVector2 tmp;
        tmp.x=x/(float)d;
        tmp.y=y/(float)d;
        return tmp;
    }

    inline UCVector2 operator *(float f)
    {
        UCVector2 tmp;
        tmp.x=x*f;
        tmp.y=y*f;
        return tmp;
    }

    void Init(float xx,float yy)
    {
        x=xx;
        y=yy;
    }
    
    void Mod1(float dx,float dy)
    {
        x-=dx;
        y-=dy;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Vertex
class API3D_STUB CVertex
{
public:
	CVector Stok;											// coordinate values in object space
	CVector Stok0;
	CVector Calc;											// temporary calculated coo							
	CVector Norm;											// normal
	CVector NormCalc;					
	CVector2 Map;											// mapping coordinate
    
    union
    {
        struct {
            CRGBA Diffuse;                                  // diffuse
        };

		struct {
            float Weight[4];
        };
    };

    union
    {
        struct {
            UCVector2 Map2;
        };

		struct {
            unsigned int rgba;
			unsigned int misc;
        };
        
        struct {
            unsigned short int Index[4];
        };
    };

	int tag,tag2;											// integers, floats
	short int temp;
    union
    {
        float coef1;
        int value;
    };
	
	CVertex() { temp=0; }
	CVertex(float x,float y,float z) { Stok.Init(x,y,z);Norm.Init(0,0,0);tag=0;}
	CVertex(float x,float y,float z,float nx,float ny,float nz) { Stok.Init(x,y,z);Norm.Init(nx,ny,nz);tag=0;}

	void InitStok(float x,float y,float z) { Stok.Init(x,y,z);}
	void InitNorm(float x,float y,float z) { Norm.Init(x,y,z);}
	void SetDiffuse(float r,float g,float b,float a);
	
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Define a list of points cycling
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! definitions of the figure data structure to be processed by triangularisation, and others functions.
class API3D_STUB CFigure
{
public:
	CList <int> List;										// list of integers
	int a,b;
	int nF;
	int tag;
	int nT;

	CFigure() {}
	~CFigure() {List.Free();} 
	
	//! determining if two figures have to be linked for triangularisation
	bool Disjoint(CVertex *vertices,CVector N,CFigure *fig);
	//! determining if figure define volume or hole
	bool Sens(int ss,CVertex *vertices,CVector N);
	//! retreiving convex envelop of a figure
	void ConvexHull(CVertex *vertices,CVector N);
	//! optimise the list of points with colinear segments and nul ones
	void OptimizeFrom(CVertex * vertices);
    //! optimise the list of points with colinear segments and nul ones
    void OptimizeFromPreserve(CVertex * vertices);
	//! optimise the list of points with colinear segments and nul ones
	void preOptimizeFrom(CVertex * vertices);
	//! add two figures between one point in each figures
	void Add(int position,CFigure *figure,int index);
	//! duplicate
	void Copy(CFigure *figure);
	//! duplicate and reverse ordering
	void InvCopy(CFigure *figure);
	//! optimise the list of points with colinear segments and nul ones, large approx
	void OptimizeHugeFrom(CVertex * vertices);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  FACES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Face

class API3D_STUB CFace
{
public:
	int v0,v1,v2;										// vertex indices
	CVertex * v[3];										// vertex pointer
	CVector Norm;										// normal
	CVector NormCalc;									// temporary value of the normal
	int tag;											// integer, usefull to work with objects
    float coef1;
#ifndef LEVELEDIT_PLAYER
    CVector g;
    int f01,f12,f20;                                    // adjacent face indices
    CVector2 mp0,mp1,mp2;
#else
    union
    {
        struct
        {
            UCVector2 mp0,mp1,mp2;
        };
        
        struct
        {
            int f01,f12,f20;                                    // adjacent face indices
            UCVector g;
        };
    };
#endif
	short int nT,nT2;									// texture index
	unsigned char ref;									// transparency
	unsigned char var;
    
	union
	{
		struct 
		{ 
			int nL,nLVectors;							// lightmaps index
			short int tag2;
			short int tag4;
			//CRGBA Diffuse;								// diffuse
			unsigned char lock;
			unsigned char size_lightmap;
			unsigned short int tag3;
		};
        
        struct
        {
            CRGBA Diffuse;
        };
        
		struct
		{
			//int reserved1i,reserved2i;
			//short int reserved1si;
			//short int reserved2si;
            int i0,i1,i2,i3;
		};

        struct
        {
            int areserved1i,areserved2i;
            short int areserved1si;
            short int areserved2si;
            //float cx,cy,cz,r;
            int ii;
        };

		unsigned char tags[4*(2+4*0+1+1)];
	};

    CFace() {}

	// short fonctions to fill data
	void Init(int n0,int n1,int n2);
	void InitNorm(float x,float y,float z);
	void SetDiffuse(float r,float g,float b,float a);
    
    bool Inside(CVector &I);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Short structure of face ( specific to some functions)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API3D_STUB CShortFace
{
public:
	int n0,n1,n2;											// indices
	short int nT,nT2;
	int nL,nLVectors;									// texture
	short int tag;
	short int tag2;
	unsigned char lock;
	unsigned char ref;
	CVector N;												// normal
	CVertex *s0,*s1,*s2;									// pointers
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CShortVertexMap
{
public:
	CVector Stok;											// coo
	CVector Calc;											// calc
	CVector2 Map;
	CVector2 Map2;
	CRGBA Diffuse;
	int tag,tag2;
	float coef1;
    int ndx;

	CShortVertexMap() {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CShortFaceMap
{
public:
	int n0,n1,n2;											// indices
	union
	{
		struct 
		{ 
			CShortVertexMap *s0,*s1,*s2;							// pointers
		};
		CShortVertexMap * v[3];		
	};
	short int nT,nT2;
	int nL,nLVectors;									// texture
	short int tag;
	short int tag2;
	unsigned char lock;
	unsigned char ref;
	CVector N;												// normal
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Short structtures of vertex ( specific to some functions)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class API3D_STUB CShortVertex
{
public:
	CVector Stok;											// coo
	CVector Norm;											// normal

	CShortVertex() {}

	void InitStok(float x,float y,float z);
	void InitNorm(float x,float y,float z); 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Data class for a 3D object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../data/datacontainer.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	OBJECT3D_COMPRESSED_FORM	=	1,
    OBJECT3D_PACKED_FORM        =   2,
	OBJECT3D_NORMAL_FORM		=	0
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int size;
	char * raw;
} LMData;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! context

class CVertexBuffer;

class CEffectContext
{
public:
	CVertexBuffer * ambient;
	CVertexBuffer * light;
    CVertexBuffer * spotm;
	CVertexBuffer * daylight;
	CVertexBuffer * lightandamb;
	CVertexBuffer * lightwithshadowone;
	CVertexBuffer * lightwithshadowfourty;
    CVertexBuffer * lightwithshadowspotlight;
    CVertexBuffer * lightwithshadowspot;

    short int Iambient;
    short int Ilight;
    short int Ispotm;
    short int Idaylight;
    short int Ilightandamb;
    short int Ilightwithshadowone;
    short int Ilightwithshadowfourty;
    short int Ilightwithshadowspotlight;
    short int Ilightwithshadowspot;

    CEffectContext()
    {
        reset();
    }
    
    void reset()
    {
        ambient=NULL;
        light=NULL;
        spotm=NULL;
        daylight=NULL;
        lightandamb=NULL;
        lightwithshadowone=NULL;
        lightwithshadowfourty=NULL;
        lightwithshadowspotlight=NULL;
        lightwithshadowspot=NULL;
        
        Iambient=-1;
        Ilight=-1;
        Ispotm=-1;
        Idaylight=-1;
        Ilightandamb=-1;
        Ilightwithshadowone=-1;
        Ilightwithshadowfourty=-1;
        Ilightwithshadowspotlight=-1;
        Ilightwithshadowspot=-1;
    }

	int isInContext(CVertexBuffer *effprev)
	{
		if (effprev==ambient) return 0;
		if (effprev==light) return 1;
		if (effprev==spotm) return 2;
		if (effprev==daylight) return 3;
		if (effprev==lightandamb) return 4;
		if (effprev==lightwithshadowone) return 5;
		if (effprev==lightwithshadowfourty) return 6;
		if (effprev==lightwithshadowspotlight) return 7;
		if (effprev==lightwithshadowspot) return 8;
		return -1;
	}

	void replaceInContext(CVertexBuffer *effprev,CVertexBuffer *effnew)
	{
		if (effprev==ambient) ambient=effnew;
		if (effprev==light) light=effnew;
		if (effprev==spotm) spotm=effnew;
		if (effprev==daylight) daylight=effnew;
		if (effprev==lightandamb) lightandamb=effnew;
		if (effprev==lightwithshadowone) lightwithshadowone=effnew;
		if (effprev==lightwithshadowfourty) lightwithshadowfourty=effnew;
		if (effprev==lightwithshadowspotlight) lightwithshadowspotlight=effnew;
		if (effprev==lightwithshadowspot) lightwithshadowspot=effnew;
	}
};


class GridObject3D;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! main data structure in use in this lib
class API3D_STUB CObject3D
{
public:
	// main defs
	char Name[128];
	unsigned int Output;
	int nVertices;
	int nFaces;
	CVector Coo;												// position
	CVector Rot;												// euler angles
	CVertex *Vertices;											// list of vertices
	CFace *Faces;												// list of faces
	CVertexAdd *VerticesAdd;
	CMatrix M,MR;
	CVector aCoo;
	CVector aRot;
	// update cpt
	unsigned int CompteurFrames;	
	// radius,volum : can be calculed once
	float Radius;
	float Volum;
    
	// light attached params
    CVector light_pos;
    CRGBA light_color;
    float light_attenuation;
    float light_range;
    int light_spot;
    bool light_cast_env_shadows;
    unsigned char light_details;
    float light_falloff;
    float light_spot_radius;
    int cast_shadows;
    short int light_position;
    short int light_penumbra;

    CVector light_pos2;
    CRGBA light_color2;
    float light_attenuation2;
    float light_range2;
    int light_spot2;
    bool light_cast_env_shadows2;
    unsigned char light_details2;
    float light_falloff2;
    float light_spot_radius2;
    int cast_shadows2;
    short int light_position2;
    short int light_penumbra2;
    
    short int notself;
    
    short int precalc;
	short int after;
	short int nozbuf;
    
// edges
	EdgeListD * edges;
	// physic
    CPOB *AP;
#ifdef _DEFINES_API_CODE_PHYSIC_
	CPhysicObject Proprietes;								// physic 
#endif
	unsigned char *map_corres_vertices;
	// structure
	CObject3D * Double1;
	CObject3D * Double2;
    CObject3D * Silhouette;
	CObject3D * Ed;
	EdgeList *Edges;
	float * arraylengths;
	int Tab[16];
	int Tab2[16];
	CVector min,max;
	// morph keys
	CShortVertex *VerticesKeys[1024];						// frames
	int nKeys;
	int actual_key;
	int key1,key2;
	float t;
	int nT,nT2;
	int Status;
	int Status2;
	int Attribut;
	int Tag;
	int Tag2;
	int Tag3;
	int Tag4;
    int OldTag;
	int Affect;
	float x0,x1,y0,y1,z0,z1;
	CPlane P[6];
    float Pprm[6];
    int PNB[6];
	int nP;
    
    bool f012;
    
    int FaceI;
    CVector PointI;
    float PointI_t;
    CVector PointINorm;

    int _FaceI[32];
    CVector _PointI[32];
    float _PointI_t[32];
    CVector _PointINorm[32];

    CObject3D *lod;
    
    CQuaternion Q;
	bool update;
	bool active;
	bool env_mapping;
    bool fullnormals;
	int nurbs;
	int pvs;
	int identifier;
	char * anim_strings;
	float param;
	CVector *velocities;
	CVector *accelerations;
	char * form_compressed_buffer;
	unsigned char tag_stuffs_in_sleep_data;
	unsigned char tag_param;
	unsigned char tags[8];
	int * indices[4];
	CVector world_pos;
	float world_radius;
	int impostor;
	int navmesh;
	int drawn[3];
	int minnf,maxnf;
	CVector Pivot,G,A,B;
	int group;
	CMatrix M2,MR2;
	int poslightmaps;
	int ntag;
	int factor_envmap;
	int nt_envmap;
	CList <LMData> LightmapsData;
	CVector * ArrayLMData;
	CVector mini,maxi,medi;
	CVector boundmini,boundmaxi;
    CVertexBuffer *vbedges;
    CVertexBuffer *vbedges2;
	CEffectContext context;
    short int params_draw;
    short int nocollide;
    short int onlyshadowing;
    short int smoothingAll;
    
	int keyframe;
    float orientation;
    float RadiusCalc;
    
    CVector4 VSParams;
    CVector4 PSParams;
    
    CVertexBuffer *vbz,*vbzm,*vbzed;

#ifdef _DEFINES_API_CODE_PHYSIC_
	CPhysicJoint* joint;
#endif
    
    float lodparam;
    
    float noloddraw;
    int nbrow;
    int collmesh;
    short int texs[256];
    int nbtexs;
    
    int placed;
    
    CVector optimizedG;
    float optimizedRadius;
    
    GridObject3D *grid;
    int temporaryID;

	int state;
	int in_thread;
	int bumpinalpha;

	int virtualobject;

	int lightingdesactivated;

	float particularscale;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// constructor

	CObject3D() 
	{
        int n;

		lightingdesactivated=0;

		particularscale=1.0f;

		virtualobject=-1;
        
		in_thread=0;
		Status=0;
        collmesh=-1;
		bumpinalpha=0;
		state=0;        
        temporaryID=-1;        
        grid=NULL;        
        f012=false;        
        optimizedRadius=-1;        
        placed=0;
        
        lodparam=1.0f;        
        notself=0;        
        onlyshadowing=0;        
        nbtexs=-1;
        
#ifdef _DEFINES_API_CODE_PHYSIC_
		joint=NULL;
#endif
        AP=NULL;

		keyframe=0;
        nbrow=-1;        
        noloddraw=-1;        
        lod=NULL;        
        nocollide=0;
        
        vbz=vbzm=vbzed=NULL;
        
        VSParams.Init(0.0f,0.0f,0.0f,0.0f);
        PSParams.Init(0.0f,0.0f,0.0f,0.0f);
        
        orientation=1.0f;
        
		nurbs=0;
		fullnormals=false;
		navmesh=-1;
		impostor=-1;
		Ed=NULL;
		Status2=2;

        precalc=0;
		after=0;        
        params_draw=0;        
        smoothingAll=0;

		nozbuf=0;
        
        vbedges=NULL;
        vbedges2=NULL;

		context.ambient=NULL;
		context.light=NULL;
		context.daylight=NULL;
		context.lightandamb=NULL;
		context.lightwithshadowone=NULL;
		context.lightwithshadowfourty=NULL;
        context.lightwithshadowspotlight=NULL;
        context.lightwithshadowspot=NULL;

		context.Iambient=-1;
		context.Ilight=-1;
		context.Idaylight=-1;
		context.Ilightandamb=-1;
		context.Ilightwithshadowone=-1;
		context.Ilightwithshadowfourty=-1;
        context.Ilightwithshadowspotlight=-1;
        context.Ilightwithshadowspot=-1;

		tags[0]=0;
		tags[1]=0;
		tags[2]=0;
		tags[3]=0;
		tags[4]=0;
		tags[5]=0;
		tags[6]=0;
		tags[7]=0;
		velocities=NULL;
		accelerations=NULL;
		Attribut=0;
		active=false;
		tag_param=0;
		tag_stuffs_in_sleep_data=1;
		form_compressed_buffer=NULL;
		Output=0;
		Affect=0;
		param=-666.0f;
		cast_shadows=1;
		map_corres_vertices=NULL;
		identifier=-1;
		Coo.Init(0,0,0);
		Rot.Init(0,0,0);
		Radius=-1;Double2=NULL;Double1=NULL;CompteurFrames=0;Vertices=NULL;Faces=NULL;
        
        Silhouette=NULL;
        
		nVertices=0;
		nFaces=0;

		Vertices=NULL;
		Faces=NULL;

		update=true;
		env_mapping=false;
		factor_envmap=0;
		nt_envmap=0;
		nP=0;
		nKeys=0;
		anim_strings=NULL;
		edges=NULL;
		Edges=NULL;
		
        nT=0;
        nT2=0;
        
		group=0;
		key1=0;
		key2=0;
		t=0.0f;
        
        light_spot=0;
        light_spot2=0;
        light_attenuation=-1;
        light_attenuation2=-1;
        
        for (n=0;n<16;n++) { Tab[n]=-1; Tab2[n]=-1; }
        
        for (n=0;n<6;n++)
        {
            Pprm[n]=0;
            PNB[n]=0;
        }
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? storage functions

		//! storage form
		int SetForm(int flag);
		void store(CDataContainer *dat);
		void read(CDataReader *dat);
		//! form states
		int sleep();
        //! form states
        int pack();
        //! form states
		void awake();
    
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? bounds

		bool InsideBounds(CVector p);
		bool InsideBounds(CVector A,CVector B,CVector p);
		bool InsideBounds(CVector p,float r);

		bool InsideBounds(CVector p,CMatrix M);
		bool InsideBounds(CVector A,CVector B,CVector p,CMatrix M);
		bool InsideBounds(CVector p,float r,CMatrix M);

		void Bounds();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? raytrace functions

        //! for ray intersection
        void PrecomputeSides();
	
		//! ray intersection
		//! base refers to a near point, like camera position in raytracing
		bool IntersectObjet(CVector &A,CVector &B,CVector &Base);

		//! ray intersection
		//! base refers to a near point, like camera position in raytracing
		bool IntersectObjetApprox(CVector &A,CVector &B,CVector &Base);

		//! ray intersection
		//! face application
		bool IntersectFace(CVector &A,CVector &B,CFace * F);

		//! ray intersection
		//! face application
		bool IntersectFaceMatrix(CVector &A,CVector &B,CFace * F,CVector &N);

		//! ray intersection		
		//! face application
		bool IntersectFaceApprox(CVector &A,CVector &B,CFace * F);

		//! point test closed to face (must be in orthogonal volume of the triangle)
		bool FaceApprox(CVector &I,CFace * F);
    
        //! calculate mapping coo from point and face
        CVector2 CalculateMappingFromPointAndFace(int FACE,CVector &Pointer);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? main initializing functions

		//! initialise Vertices and Faces due to number of vertices and number of faces
		void Init(int nv,int nf);
		//! add texture value
		void Init2(int nv,int nf,int nT);
		//! calculate Faces[n].v[0..2] pointers to vertices : needed for all operations
		void SetFaces();

		//! invert normals
		void InvertNormals();
		//! smooth vertice and face normals
		void CalculateNormals(int tag);
		//! smooth vertice and face normals
		void CalculateNormalsQuick();
		//! face normals
		void CalculateFaceNormals();
		//! face normals calc
		void CalculateFaceNormalsCalc();

		//! calculate smoothing group normals due to Faces[n].tag
		void CalculateNormalsSmoothingGroups();
		//! calculate smoothing group normals due to Faces[n].tag
		void CalculateNormalsSmoothingGroups2();

		//! take care if vertices have different indices but are close
		void CalculateNormals2(int tag);
		//! take care if vertices have different indices but are close
		void CalculateNormalsBasic(int tag);
		//! take care if vertices have different indices but are close
		void CalculateNormalsSimple(int tag);
		//! take care if vertices have different indices but are close
		void CalculateNormals2a(int tag);

        float MinimalDistance();
        float MaximalDistance();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? lightmap coordinates calculation
	
		//! lightmap coordinates calculation
		CVector * CalculateLightmapCoordinates();
		//! lightmap coordinates calculation
		//! @param size : define lightmap size
		CVector * CalculateLightmapCoordinates(int size);
        //! lightmap coordinates calculation (average)
        //! @param size : define lightmap size
        CVector * CalculateLightmapCoordinatesAvg(int size);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? cutting due to x,y based square (used in pinball to allow 576x1050 texturing on all cards)	

		//! function to sperarate objects in part in order to apply 256x256 texture from a bigger one
		CObject3D PortionCarree(float x1,float y1,float x2,float y2,int nT,float dx,float dy);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? creation and fast simple operations

		//! returns an object with sharpen edges
		CObject3D ApplyBevel(float R);
		//! returns an object with sharpen edges
		CObject3D * ApplyBevel2(float R);
		//! returns an object with double faces
		CObject3D Dedouble();
		//! returns an object due to specification
		CObject3D Duplicate();
		//! returns an object due to specification
		CObject3D Regroupe();
        //! returns an object due to specification
        CObject3D DuplicateSelected(int tag);
        //! returns an object due to specification
        CObject3D DuplicateSelectedNot(int tag);
		//! returns an object due to specification
		CObject3D DuplicateSelected_nT(int nT);

		//! returns an object due to specification
		CObject3D* DuplicateVerticesSelected(int tag);

		//! returns an object due to specification
		CObject3D RTPatch(float sign);
		//! returns an object due to specification
		CObject3D* RTPatch2(float sign);
		//! returns an object due to specification
		CObject3D RTPatchSelected(int tag,float sign);
		//! returns an object due to specification
		CObject3D* RTPatchSelected2(int tag,float sign);
        //! returns an object due to specification
        CObject3D* RTPatchSelected2Basic(int tag,float sign);

		//! returns an object due to specification
		CObject3D Tesselate();
		//! returns an object due to specification
		CObject3D TesselateSelected(int tag);
		//! returns an object due to specification
		CObject3D Inverse();
		//! returns an object due to specification
		CObject3D TesselateLength(float len);
		//! returns an object due to specification
		CObject3D TesselateLengthSelected(int tag,float len);
		//! returns an object due to specification
		CObject3D TesselateLength4(float len);
		//! returns an object due to specification
		CObject3D TesselateLength3(float len);
		//! returns an object due to specification
		CObject3D* Tesselate3();
		//! returns an object due to specification
		CObject3D SphereDerivate();
		//! returns an object due to specification
		CObject3D TesselateInf(float Coef);
		//! returns an object due to specification
		CObject3D Normalize();

		//! returns an object with double faces
		CObject3D * Dedouble2();
		//! returns an object due to specification
		CObject3D* TesselateInf2(float Coef);

		//! returns an object due to specification
		CObject3D TesselateInfSelected(float Coef);
		//! returns an object due to specification
		CObject3D* TesselateInfSelected2(float Coef);

		//! returns an object due to specification
		CObject3D TesselateLengthNb(float len,int NB);

		//! returns an object due to specification
		CObject3D* TesselateLengthNb2(float len,int NB);
		//! returns an object due to specification
		CObject3D* TesselateLengthSelected2(int tag,float len);
		//! returns an object due to specification
		CObject3D *TesselateLength2(float len);
        //! returns an object due to specification
        CObject3D* TesselateLength2Map2(float len);
		//! returns an object due to specification
		CObject3D *TesselateLength42(float len);
		//! returns an object due to specification
		CObject3D *TesselateLength32(float len);
		//! a per face texture cube tesselated will become a sphere environment
		CObject3D *Normalize2();
		//! returns a duplicate
		CObject3D *Duplicate2();
		//! merge closest vertices
		CObject3D *Regroupe2();
		CObject3D* Regroupe2(float d);
		//! duplicate tagged faces
		CObject3D *DuplicateSelected2(int tag);
        //! duplicate tagged faces
        CObject3D *DuplicateSelectedNot2(int tag);
		//! duplicate faces from nT face value
		CObject3D* DuplicateSelected_nT2(int nT);
		//! tesselate th object
		CObject3D *Tesselate2();
		CObject3D *Tesselate2full();
		//! tesselate tagged faces
		CObject3D *TesselateSelected2(int tag);
		//! tesselate tagged faces
		CObject3D *TesselateSelected3(int tag);
		//! returns an object inversed
		CObject3D *Inverse2();
		//! returns an object of sphere derivated
		CObject3D *SphereDerivate2();
		//! ordering faces due to textures lightmap...
		CObject3D* ReArr();
		//! returns an object with edges selected splited
		CObject3D SplitEdges();
		//! returns an object with edges selected splited
		CObject3D *SplitEdges2();
		//! returns an object with faces linked due to mapping coordinates
		CObject3D * ReMeshByMapping();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//!	internals vertices map

		void MCVSet(int x,int y);
		int MCVGet(int x,int y);
		void CreateMCV();
		void FreeMCV();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? calculate, displace, set, etc.

		//! inverse face orientation (normals re-calculation)
		void InverseOrientation();
		//! inverse face orientation (normals re-calculation if tag!=0)
		void InverseOrientation(int tag);

        //! horz/vert mapping application
        void InvertXMap();
        void InvertYMap();

        void InvertXMapL();
        void InvertYMapL();

        //! rotate mapping application
        void RotateLeftMap();
        void RotateRightMap();

        void RotateLeftMapL();
        void RotateRightMapL();

		void RotateMapL(float deg);

        //! both scale mapping coo
        void MappingScale(float Kx,float Ky);
    
        //! both bound mapping application
        void MappingBoundsFixed(int tag);
    
		//! spherical mapping application
		void SphericalMapping();
		//! planar mapping application
		void PlanarMapping();
		//! planar mapping application
		void PlanarMappingRadial();
		//! planar mapping close application
		void PlanarMappingClose();
		//! planar mapping close application
		void PlanarMappingCloseRadial();
		//! cylindrical mapping application
		void CylindricalMapping();
		//! cylindrical mapping application (center)
		void CylindricalMappingCenter();
		//! cylindrical mapping application
		void CylindricalMappingOrtho();
		//! planar mapping application
		void PlanarMappingBound();
		//! planar mapping application
		void PlanarMappingBoundXYZ(CVector min,CVector max);

		//! planar mapping application
		void PlanarMappingBoundOrtho();

		//! planar mapping application (front/back)
		void PlanarMappingBound2();

		//! planar mapping application for painting
		void PlanarMappingBoundPainting(int texw,int texh);

        //! planar mapping application for painting
        char* PlanarMappingBoundPaintingAvg(int texw,int texh);

		//! mapping scale in x and y
		void ScaleMappingCoordinate(float sx,float sy);

		//! center th object
		CVector BoundingCenter();
		//! center th object
		void BoundingCenteredCoo();
		//! center th object
		CVector BoundingCentered();

        //! form states
        void OptimizedOcc();

        //! vertical value
        float Yup();
        //! vertical value
        float Ydown();
        //! center the object vertically
        CVector CenteredVertical(bool up);

		//! center the object
		CVector Centered();
		//! free object
		void Free();
		//! recalc stok and norm 
		void Apply(CMatrix &M,CMatrix &M2);
		//! recalc stok and norm due to Coo vector and Rot
		void Set();
		//! used to calculate an envelloppe
		void Displace(float d);
		// calculate center of the object
		CVector Center();
		// calculate center of the object
		CVector CenterCalc();
		//! all faces with the same texture
		void FillTexture(int NT);
		//! Calc & NormCalc due to Coo vector and Rot
		void Calculate();
		//! Calc & NormCalc due to Mobj & Mrot
		void Calculate(CMatrix *Mobj,CMatrix *Mrot);

        //! calculate center of faces
        void CalculateFaceCenter();
        //! calculate center of faces
        void CalculateFaceCenterCalc();

		//! bounding values
		float Bounding(int axe);
    
        //! Scaling
        void Scaling(float sc);
        //! Scaling
        void Scaling(float sx,float sy,float sz);
        //! Rotation
        void Rotation(float rx,float ry,float rz);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? primitive object generation

		//! object grid
		void Grid(float Lx,float Lz,int def);
		//! object disk
		void Disk(float R,int Rdef,int def);
		//! object dome
		void Dome(float R,int def);
		//! object cube
		void Cube(float Lx,float Ly,float Lz);
		//! object pyramid
		void Pyramid(float Lx,float Ly,float Lz);
		//! object cube
		void CubeWorld(float Ax,float Ay,float Az,float Bx,float By,float Bz);
		//! object cube
		void Cube2(float Lx,float Ly,float Lz);
		//! object cube (invert)
		void Cube2i(float Lx,float Ly,float Lz);
		//! object cylindre
		void Cylindre(float L,float R,int def);
		//! object cylindre
		void Cylindre2(float L,float R,int def);
		//! object cylindre
		void Cylindre2pair(float L,float R,int def);
		//! object cylindre
		void Cylindre3(float L,float R1,float R2,int def);

		//! object quad
		void Quad(CVector A,CVector B,CVector C,CVector D);
		void DoubleQuad(CVector A,CVector B,CVector C,CVector D);

		//! object sphere
		void Bone(float R);
		//! object sphere
		void Sphere(float R,int def);
		//! object sphere (for environment)
		void OpenSphere2(float R,int def);
		//! object sphere (for environment)
		void OpenSphere2(float R,int def,int tag);
        //! object sphere (for environment)
        void OpenSphere3(float R,int def);
        //! object sphere (for environment)
        void OpenSphere3(float R,int def,int tag);
		//! object sphere (for environment)
		void OpenSphere(float R,int def);
		//! object sphere (for environment)
		void OpenSphereTri(float R,int def);
		//! object plane
		void Plane(float Dim);
		//! object triangle
		void EquiTriangle(float Dim);
		//! object half-pipe
		void Arc(float r,float e,int def);
		//! object half-pipe
		void Arc0(float r,float e,int def);
		//! object half-pipe
		void Arc1(float r,float r2,float e,int def);
		//! object stairs
		void Escalier(float L,float H,float L2,float hmm,int def,float R,float dw);
		//! object turning stairs
		void Escalier2(float L,float H,float L2,float hmm,int def,float R,float dw);
		//! object tunnel
		void Tunnel(float L,float H,float L2,int def);
		//! object tunnel
		void Tunnel2(float L,float H,float L2,int def);
		//! object prism
		void Prism(float D,float L);
		//! object "tuy�re"
		void CorDeChasse(float L,float R1,float R2,int def);
		//! object tore
		void Tore(float R1,float R2,int def);
		//! object arrow
		void Arrow(float L,float R,int def);

		//! cone
		void Cone(float L,float R,int def);

		//! basic
		void Carre(int nf,int n0,int n1,int n2,int n3);
		//! basic
		void Carrei(int nf,int n3,int n2,int n1,int n0);
		//! basic
		void Triangle(int nf,int n0,int n1,int n2);
		//! basic
		void Carre2(int nf,int n0,int n1,int n2,int n3);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? object for vegetal generations

		//! internal
		void Cylindre3void(float L,float R1,float R2,int def);
		//! internal
		void CorDeChassevoid(float L,float R1,float R2,int def);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	//? misc function and topology

		// detach geometry selected
		void SmoothingGroupFaces();
    
        // tag with map coord
        void TagMappingCoordiantesFaces(int nf,int ref,int TAG);
        // check map coord
        bool ChecFaceMapCoord(int nf,CVector2 &map1,CVector2 &map0);

        // tag by groupq
        int TagGroupCloseToClose();

        // merge closest vertices due to their mapping coord.
        CObject3D * ConvertLinearFull(float scale,float scalem);
        // merge closest vertices due to their mapping coord.
        CObject3D * ConvertLinearFullNorm(float scale,float scalem);

        // merge closest vertices due to their mapping coord.
		CObject3D * ConvertLinearNTLV(float coef);
		// merge closest vertices due to their mapping coord.
		CObject3D * ConvertLinear(float coef);
        // merge closest vertices due to their mapping coord.
        CObject3D * ConvertLinearMap2(float coef);
		// merge closest vertices due to their mapping coord.
		CObject3D * ConvertLinearSimple(float coef);
		// merge closest vertices due to their mapping coord.
		CObject3D * ConvertLinearSimple_m(float coef,float coefm);

		//! edges are identical
		bool EdgeConfondue(CVector a1,CVector b1,CVector a2,CVector b2);

		//! objects have some same edges, same vertices
		//! x0,y0,z0 and x1,y1,z1 must be set to extrema Calc values
		bool Linked(CObject3D * obj);

		// see FILTER PARAMETERS

		//! initialize face edges links
		void SetF012();
		void SetF012Cull(float val);

        void SetWF012();
		void SetF012Near();
		void SetF012MapNear();

		void SetF012Inv();
		void SetF012NearInv();
    
        //! set face tag belong definition of topology
        float SetTagTopo();
    
		//! set face tag belong definition
		int SetTag(unsigned int Filtre);
		//! set face tag belong definition
		void TagCoplanarFaces(int nf,int ref,int TAG);
        //! set face tag belong definition
        void TagCoplanarFacesSet(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCoplanarFacesCool(int nf,int ref,int TAG);
        //! set face tag belong definition
        void TagCoplanarFacesCool2(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagHCoplanarFaces(int nf,int ref,int TAG);
		//! set face tag belong definition (set)
		void TagSetHCoplanarFaces(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCoplanarFaces2(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagNearToNearFaces(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCloseToCloseFaces(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCloseToCloseFaces2(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCloseToCloseFaces(int nf);
		//! set face tag belong definition
		void TagGroupFaces(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCoplanarOnlyFaces(int ref,int TAG);
		//! set face tag belong definition
		void Tag2CoplanarOnlyFaces(int ref,int TAG);
		//! set face tag belong definition
		void TagObjetCoplanarFacesTag(int nf,int ref,int TAG,int BASETAG);
		//! set face tag belong definition
		void TagCoplanarFaces_nT(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCoplanarFaces_nTnLTag3(int nf,int ref,int TAG);
		//! set face tag belong definition
		void TagCoplanarFacesMC(int nf,int ref,int TAG);
		//! set face tag belong definition
		int TagSelectingFacesClose();
		//! set face tag belong definition
		int TagSelectingFacesCloseNear();

		//! face close by edge
		int Face_20(int nf);
		//! face close by edge
		int Face_12(int nf);
		//! face close by edge
		int Face_01(int nf);
		
		//! face close by edge
		//int Face_20Q(int nf,int *references_face_012Q);
		//! face close by edge
		//int Face_12Q(int nf,int *references_face_012Q);
		//! face close by edge
		//int Face_01Q(int nf,int *references_face_012Q);

		//! face close by edge
		int Face_20QZ(int nf,int minf,int maxf);
		//! face close by edge
		int Face_12QZ(int nf,int minf,int maxf);
		//! face close by edge
		int Face_01QZ(int nf,int minf,int maxf);

		//! face close by edge (approx vertex position)
		int FaceClose_01(int nf);

		//! face close by edge (approx vertex position)
		int FaceClose_12(int nf);

		//! face close by edge (approx vertex position)
		int FaceClose_20(int nf);

        //! function to reorder vertically (/Y)
        int ReorderY();
        int ReorderYFaces();

		//! function to call before uploading object with type TYPE_OBJECT in physic props
		void CalculateSphereFaceOnCoef1();
		//! reset values
		void SetRefTexture();

		//! set texture (nT field)
		void SetTexture(int nt);

		//! set texture (nT2 field)
		void SetTexture2(int nt);

		//! set blend weight ('X', 'Y', 'Z')
		void SetBlend(int num,char c);

		//! apply blending to Stok coordinates.
		void ApplyBlending(int num,CMatrix M1,CMatrix M2);
    

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	//? modifying geometry
	
		//! take coplanar faces and retriangularize them (internal)
		CObject3D * OptimizeMeshBoolean_simple();
		//! take coplanar faces and retriangularize them; action on close vertices
		CObject3D * OptimizeMeshBoolean();
		//! take coplanar faces and retriangularize them (due to tag value)
		CObject3D * OptimizeMeshBooleanTag(int THISTAG,bool map2);
    
        //! take coplanar faces without simplifications
        CObject3D * PlanarOptimize();
		CObject3D * Triangles();
    
        //! take coplanar faces and retriangularize them (internal)
        CObject3D * OptimizeMeshBooleanPreserveTag2();

		//! add selected tag
		void AddSelected(CObject3D *obj,int tag);
        //! add selected tag
        void AddOnSelected(CObject3D *obj,int tag);
        //! add obj
        void Add(CObject3D *obj);
        //! add obj
        void AddFirst(CObject3D *obj);
    
        //! add face
        void AddTri(int a,int b,int c);
    
        //! add face
        void AddTriV(CVector &A,CVector &B,CVector &C);
    
        //! minimal data
        void Minimal();
    
        //! 3 vertices a face
        void Extend();

        //! mps
        void SetMappingVerticesFromFaces();
        //! mps
        void SetMappingFacesFromVertices();
    
    
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	//? physic modeling section

		void CalculateVertexMasses();
		void GravityCenter();

		//! calculate radius of the object
		float CalculateRadius();
        float CalculatePlanarRadius();
		//! calculate radius of the centered object
		float CalculateOptimalRadius();
		//! calculate volum of the object
		float CalculateVolum();
		//! calculate shortest radius of the object
		float CalculateMinimalRadius();
    
        //! calculate face lenth
        float CalculateAverageFaceLength();
        //! calculate face lenth
        float CalculateShortestFaceLength();
        //! calculate face lenth
        float CalculateLargestFaceLength();

#ifdef _DEFINES_API_CODE_PHYSIC_
		//! calulate spheretree of the object
		CSphereTree* CalculateSphereTree();
		CSpherePartition* CreateSpherePartition(int NB);
#endif		
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! grid data
#define GRID3DDEFOBJ 64
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GridObject3D
{
public:
    int nbx,nby,nbz;
    CList <int> grid[GRID3DDEFOBJ*GRID3DDEFOBJ*GRID3DDEFOBJ];
    CVector mini,maxi,bound;
    
    GridObject3D()
    {
 
    }

    ~GridObject3D()
    {
        for (int x=0;x<GRID3DDEFOBJ;x++)
            for (int y=0;y<GRID3DDEFOBJ;y++)
                for (int z=0;z<GRID3DDEFOBJ;z++)
                {
                    grid[x+GRID3DDEFOBJ*(y+GRID3DDEFOBJ*z)].Free();
                }
    }

	int GetList(int *list,CVector& _mini,CVector& _maxi)
	{
		CVector p;
		int x,y,z;
		int x1,y1,z1,x2,y2,z2;

		p=_mini;

		if (p.x<mini.x) p.x=mini.x;
		if (p.y<mini.y) p.y=mini.y;
		if (p.z<mini.z) p.z=mini.z;
		if (p.x>maxi.x) p.x=maxi.x;
		if (p.y>maxi.y) p.y=maxi.y;
		if (p.z>maxi.z) p.z=maxi.z;

		p.x=((nbx-1)*(p.x-mini.x))/bound.x;
        p.y=((nby-1)*(p.y-mini.y))/bound.y;
        p.z=((nbz-1)*(p.z-mini.z))/bound.z;

		x1=(int)p.x; y1=(int)p.y; z1=(int)p.z;
		
		p=_maxi;

		if (p.x<mini.x) p.x=mini.x;
		if (p.y<mini.y) p.y=mini.y;
		if (p.z<mini.z) p.z=mini.z;
		if (p.x>maxi.x) p.x=maxi.x;
		if (p.y>maxi.y) p.y=maxi.y;
		if (p.z>maxi.z) p.z=maxi.z;

		p.x=((nbx-1)*(p.x-mini.x))/bound.x;
        p.y=((nby-1)*(p.y-mini.y))/bound.y;
        p.z=((nbz-1)*(p.z-mini.z))/bound.z;

		x2=(int)p.x; y2=(int)p.y; z2=(int)p.z;

		int n=0;

		for (x=x1;x<=x2;x++)
			for (y=y1;y<=y2;y++)
				for (z=z1;z<=z2;z++)
				{
					int adr=x+nbx*(y+nby*z);
					CElement <int> * Current=grid[adr].First;
					int *pi;
					if (Current) pi=&(Current->data); else pi=NULL;
					while (pi)
					{
						list[n++]=*pi;
						Current=Current->next;
						if (Current) pi=&(Current->data); else pi=NULL;
					}
				}

		return n;
	}

	int GetList(int *list,CVector& C)
	{
		CVector p;
		int x,y,z;
		int x1,y1,z1,x2,y2,z2;
	
		p=C;

		if (p.x<mini.x) p.x=mini.x;
		if (p.y<mini.y) p.y=mini.y;
		if (p.z<mini.z) p.z=mini.z;
		if (p.x>maxi.x) p.x=maxi.x;
		if (p.y>maxi.y) p.y=maxi.y;
		if (p.z>maxi.z) p.z=maxi.z;

		p.x=((nbx-1)*(p.x-mini.x))/bound.x;
        p.y=((nby-1)*(p.y-mini.y))/bound.y;
        p.z=((nbz-1)*(p.z-mini.z))/bound.z;

		x1=(int)p.x-1; y1=(int)p.y-1; z1=(int)p.z-1;		
		x2=(int)p.x+1; y2=(int)p.y+1; z2=(int)p.z+1;

		if (x1<0) x1=0;
		if (y1<0) y1=0;
		if (z1<0) z1=0;

		if (x2>nbx-1) x2=nbx-1;
		if (y2>nby-1) y2=nby-1;
		if (z2>nbz-1) z2=nbz-1;

		int n=0;

		for (x=x1;x<=x2;x++)
			for (y=y1;y<=y2;y++)
				for (z=z1;z<=z2;z++)
				{
					int adr=x+nbx*(y+nby*z);
					CElement <int> * Current=grid[adr].First;
					int *pi;
					if (Current) pi=&(Current->data); else pi=NULL;
					while (pi)
					{
						list[n++]=*pi;
						Current=Current->next;
						if (Current) pi=&(Current->data); else pi=NULL;
					}
				}

		return n;
	}

	int GetListXZ(int *list,CVector& C)
	{
		CVector p;
		int x,z;
	
		p=C;

		if (p.x<mini.x) p.x=mini.x;
		if (p.z<mini.z) p.z=mini.z;
		if (p.x>maxi.x) p.x=maxi.x;
		if (p.z>maxi.z) p.z=maxi.z;

		p.x=((nbx-1)*(p.x-mini.x))/bound.x;
        p.z=((nbz-1)*(p.z-mini.z))/bound.z;

		x=(int) p.x;
		z=(int) p.z;

		int n=0;

		int adr=x+nbx*z;
		CElement <int> * Current=grid[adr].First;
		int *pi;
		if (Current) pi=&(Current->data); else pi=NULL;
		while (pi)
		{
			list[n++]=*pi;
			Current=Current->next;
			if (Current) pi=&(Current->data); else pi=NULL;
		}

		return n;
	}

    void Affect(CObject3D *obj,int opt)
    {
        int n;
        int nb=GRID3DDEFOBJ;
        int x,y,z;
        float f,fmoy;
        CVector p;
    
        mini.Init(100000,100000,100000);
        maxi.Init(-100000,-100000,-100000);

        for (n=0;n<obj->nVertices;n++)
        {
            p=obj->Vertices[n].Stok;
            if (p.x>maxi.x) maxi.x=p.x;
            if (p.y>maxi.y) maxi.y=p.y;
            if (p.z>maxi.z) maxi.z=p.z;
            if (p.x<mini.x) mini.x=p.x;
            if (p.y<mini.y) mini.y=p.y;
            if (p.z<mini.z) mini.z=p.z;
        }
        
        mini.x-=SMALLF3;
        mini.y-=SMALLF3;
        mini.z-=SMALLF3;

        maxi.x+=SMALLF3;
        maxi.y+=SMALLF3;
        maxi.z+=SMALLF3;

        bound=maxi-mini;
        
		if ((obj->nFaces>0)&&(opt!=666))
		{
			fmoy=0;
			for (n=0;n<obj->nFaces;n++)
			{
				p=obj->Faces[n].v[1]->Stok-obj->Faces[n].v[0]->Stok;
				f=p.Norme();
				fmoy+=f;
				p=obj->Faces[n].v[2]->Stok-obj->Faces[n].v[0]->Stok;
				f=p.Norme();
				fmoy+=f;
			}
        
			fmoy/=2*obj->nFaces;
        
			nb=(int)((bound.Norme()/fmoy)/4);
			if (nb>GRID3DDEFOBJ) nb=GRID3DDEFOBJ;
		}
		else nb=GRID3DDEFOBJ;
        
        if ((bound.x>bound.y)&&(bound.x>bound.z))
        {
            nbx=nb;
            nby=(int)(nbx*bound.y/bound.x);
            nbz=(int)(nbx*bound.z/bound.x);
        }

        if ((bound.y>bound.x)&&(bound.y>bound.z))
        {
            nby=nb;
            nbx=(int)(nby*bound.x/bound.y);
            nbz=(int)(nby*bound.z/bound.y);
        }

        if ((bound.z>bound.x)&&(bound.z>bound.y))
        {
            nbz=nb;
            nbx=(int)(nbz*bound.x/bound.z);
            nby=(int)(nbz*bound.y/bound.z);
        }

        for (n=0;n<obj->nVertices;n++)
        {
            p=obj->Vertices[n].Stok;
            p.x=((nbx-1)*(p.x-mini.x))/bound.x;
            p.y=((nby-1)*(p.y-mini.y))/bound.y;
            p.z=((nbz-1)*(p.z-mini.z))/bound.z;
            
            x=(int)p.x; y=(int)p.y; z=(int)p.z;
                        
            grid[x+nbx*(y+nby*z)].Add(n);

			if (opt==0)
			{
				if (p.x-x<0.05f)
					if (x>0) grid[x-1+nbx*(y+nby*z)].Add(n);
            
				if (p.y-y<0.05f)
					if (y>0) grid[x+nbx*(y-1+nby*z)].Add(n);
            
				if (p.z-z<0.05f)
					if (z>0) grid[x+nbx*(y+nby*(z-1))].Add(n);
            
            
				if (p.x-x>0.95f)
					if (x<nbx-1) grid[x+1+nbx*(y+nby*z)].Add(n);
            
				if (p.y-y>0.95f)
					if (y<nby-1) grid[x+nbx*(y+1+nby*z)].Add(n);
            
				if (p.z-z>0.95f)
					if (z<nbz-1) grid[x+nbx*(y+nby*(z+1))].Add(n);
			}
        }
    }

	void Affect(CObject3D *obj)
	{
		Affect(obj,0);
	}

    void AffectXZ(CObject3D *obj,int details)
    {
        int n;
        int nb=GRID3DDEFOBJ;
        int x,z;
        CVector p;
    
        mini.Init(100000,100000,100000);
        maxi.Init(-100000,-100000,-100000);

        for (n=0;n<obj->nVertices;n++)
        {
            p=obj->Vertices[n].Stok;
            if (p.x>maxi.x) maxi.x=p.x;
            if (p.y>maxi.y) maxi.y=p.y;
            if (p.z>maxi.z) maxi.z=p.z;
            if (p.x<mini.x) mini.x=p.x;
            if (p.y<mini.y) mini.y=p.y;
            if (p.z<mini.z) mini.z=p.z;
        }
        
        mini.x-=SMALLF3;
        mini.y-=SMALLF3;
        mini.z-=SMALLF3;

        maxi.x+=SMALLF3;
        maxi.y+=SMALLF3;
        maxi.z+=SMALLF3;

        bound=maxi-mini;
        
		nb=details;
                
        nbx=nb;
		nby=1;
        nbz=nb;

		char * tmp=new char[nb*nb];

        for (n=0;n<obj->nFaces;n++)
        {
			int def=8;

			for (int k=0;k<nb*nb;k++) tmp[k]=0;

			for (int a=0;a<=def;a++)
			{
				CVector A=obj->Faces[n].v[0]->Stok+a*(obj->Faces[n].v[1]->Stok-obj->Faces[n].v[0]->Stok)/def;
				CVector B=obj->Faces[n].v[0]->Stok+a*(obj->Faces[n].v[2]->Stok-obj->Faces[n].v[0]->Stok)/def;
				for (int b=0;b<=def;b++)
				{
					p=A+b*(B-A)/def;
					p.x=((nbx-1)*(p.x-mini.x))/bound.x;
					p.z=((nbz-1)*(p.z-mini.z))/bound.z;
            
		            x=(int)p.x; z=(int)p.z;
					tmp[x+nbx*z]=1;
				}
			}
            
			for (x=0;x<nb;x++)
				for (z=0;z<nb;z++)
				{
					if (tmp[x+nbx*z]) grid[x+nbx*z].Add(n);
				}
        }

		delete [] tmp;
    }

    void AffectCalc(CObject3D *obj,int opt)
    {
        int n;
        int nb=GRID3DDEFOBJ;
        int x,y,z;
        float f,fmoy;
        CVector p;
    
        mini.Init(100000,100000,100000);
        maxi.Init(-100000,-100000,-100000);

        for (n=0;n<obj->nVertices;n++)
        {
            p=obj->Vertices[n].Calc;
            if (p.x>maxi.x) maxi.x=p.x;
            if (p.y>maxi.y) maxi.y=p.y;
            if (p.z>maxi.z) maxi.z=p.z;
            if (p.x<mini.x) mini.x=p.x;
            if (p.y<mini.y) mini.y=p.y;
            if (p.z<mini.z) mini.z=p.z;
        }
        
        mini.x-=SMALLF3;
        mini.y-=SMALLF3;
        mini.z-=SMALLF3;

        maxi.x+=SMALLF3;
        maxi.y+=SMALLF3;
        maxi.z+=SMALLF3;

        bound=maxi-mini;
        
		if ((obj->nFaces>0)&&(opt!=666))
		{
			fmoy=0;
			for (n=0;n<obj->nFaces;n++)
			{
				p=obj->Faces[n].v[1]->Calc-obj->Faces[n].v[0]->Calc;
				f=p.Norme();
				fmoy+=f;
				p=obj->Faces[n].v[2]->Calc-obj->Faces[n].v[0]->Calc;
				f=p.Norme();
				fmoy+=f;
			}
        
			fmoy/=2*obj->nFaces;
        
			nb=(int)((bound.Norme()/fmoy)/4);
			if (nb>GRID3DDEFOBJ) nb=GRID3DDEFOBJ;
		}
		else nb=GRID3DDEFOBJ;
        
        if ((bound.x>bound.y)&&(bound.x>bound.z))
        {
            nbx=nb;
            nby=(int)(nbx*bound.y/bound.x);
            nbz=(int)(nbx*bound.z/bound.x);
        }

        if ((bound.y>bound.x)&&(bound.y>bound.z))
        {
            nby=nb;
            nbx=(int)(nby*bound.x/bound.y);
            nbz=(int)(nby*bound.z/bound.y);
        }

        if ((bound.z>bound.x)&&(bound.z>bound.y))
        {
            nbz=nb;
            nbx=(int)(nbz*bound.x/bound.z);
            nby=(int)(nbz*bound.y/bound.z);
        }

        for (n=0;n<obj->nVertices;n++)
        {
            p=obj->Vertices[n].Calc;
            p.x=((nbx-1)*(p.x-mini.x))/bound.x;
            p.y=((nby-1)*(p.y-mini.y))/bound.y;
            p.z=((nbz-1)*(p.z-mini.z))/bound.z;
            
            x=(int)p.x; y=(int)p.y; z=(int)p.z;
                        
            grid[x+nbx*(y+nby*z)].Add(n);

			if (opt==0)
			{
				if (p.x-x<0.05f)
					if (x>0) grid[x-1+nbx*(y+nby*z)].Add(n);
            
				if (p.y-y<0.05f)
					if (y>0) grid[x+nbx*(y-1+nby*z)].Add(n);
            
				if (p.z-z<0.05f)
					if (z>0) grid[x+nbx*(y+nby*(z-1))].Add(n);
            
            
				if (p.x-x>0.95f)
					if (x<nbx-1) grid[x+1+nbx*(y+nby*z)].Add(n);
            
				if (p.y-y>0.95f)
					if (y<nby-1) grid[x+nbx*(y+1+nby*z)].Add(n);
            
				if (p.z-z>0.95f)
					if (z<nbz-1) grid[x+nbx*(y+nby*(z+1))].Add(n);
			}
        }
    }

	void AffectCalc(CObject3D *obj)
	{
		AffectCalc(obj,0);
	}


    int Offset(CVector &p)
    {
        int x=(int)(((nbx-1)*(p.x-mini.x))/bound.x);
        int y=(int)(((nby-1)*(p.y-mini.y))/bound.y);
        int z=(int)(((nbz-1)*(p.z-mini.z))/bound.z);

        return x+nbx*(y+nby*z);
    }

};



/*?
<U><B> CObject3D Member Data Informations </B></U>

	Name of the object for referencement : 	char <B>Name[128]</B>;
	Variable for some function indicating transformation mode: unsigned int <B>Output</B>;
	Can be : PROPRIETES_PHYSIQUES,PROPRIETES_MATRICES, or 0
	Number of vertices : int <B>nVertices</B>;
	Number of faces : int <B>nFaces</B>;
	(Output==0) object position : CVector <B>Coo</B>;
	(Output==0) object euler orientation : CVector <B>Rot</B>;
	Vertices data : CVertex * <B>Vertices</B>;
	Faces data : CFace *<B>Faces</B>;
	Temporary, secondary buffer : CVertexAdd * <B>VerticesAdd</B>;
	(Output==PROP_MATRICES) position,orientation matrices : CMatrix <B>M,MR</B>;
	Var : unsigned int CompteurFrames;
	Temp Var : float <B>Radius</B>;
	Temp Var : float <B>Volum</B>;
	Temp Var (to attach some light to the object for example) : CVector <B>light_pos</B>;
	Temp Var : CRGBA <B>light_color</B>;
	Temp Var : float <B>light_attenuation</B>;
	Temp Var : float <B>light_range</B>;
	Edges data (can not be set): EdgeListD * <B>edges</B>;
	Physic data : CPhysicObject <B>Proprietes</B>;
	LOD objects : CObject3D * <B>Double1</B>;
	LOD objects : CObject3D * <B>Double2</B>;
	See definitions from edges.h : EdgeList *Edges</B>;
	Temp Var : float * arraylengths;
	Temp Var : int Tab[16];
	Temp Var : CVector min,max;
	Vertices keys for morphing data : CShortVertex *<B>VerticesKeys[512]</B>;
	Number of keys : int <B>nKeys</B>;
	Temp Var : int nT;
	Can be 0,1,2 for ClassNED managing : int <B>Status</B>;
	Temp Var : int Attribut;
	Temp Var : int Tag;
	Temp Var : int Tag2;
	Temp Var : int Affect;
	Bounding box : float <B>x0,x1,y0,y1,z0,z1</B>;
	Reflection Planes : CPlane <B>P[6]</B>;
	Number : int <B>nP</B>;
	Result from IntersectFace or IntersectObject : CVector <B>PointI</B>;
	idem : float <B>PointI_t</B>;
	idem : CVector <B>PointINorm</B>;
	Temp Var : CQuaternion Q;
	Temp Var : bool update;
	Temp Var : bool active;
	Temp Var : bool env_mapping;
	Temp Var : int pvs;
	Temp Var : int identifier;
	Attach script, infos to object : char * <B>anim_strings</B>;

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	Orthogonal BSP tree
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

#include "bsp_ortho.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	Data structure to upgrade mesh use 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

//! in order to speed functionnalit� of some procedures, we use orthogonal bsp trees to optimize calculations:
class API3D_STUB CCollisionMesh
{
public:
	CObject3D * obj;
	CVector aCoo,aRot;
	CVector bCoo,bRot;
	CBSPOrtho quad;
	CMatrix M;
	float Restitution;
	float Friction;
	bool initialised;
	bool ownermatrix;
	bool statique;
    CVector mini,maxi;
	CVector bounds;
	float Radius;
	bool boundsinit;

	CCollisionMesh() { initialised=false;ownermatrix=false;statique=false;boundsinit=false;}

	//! calculate a BSP and group with the object
	void Init(CObject3D * objet,float r,float f,int NITER);
	//! calculate a BSP and group with the object (this one for shadowing 2)
	void Init2(CObject3D * objet,float r,float f,int NITER);
	//! calculate a BSP and group with the object
	void Init3(CObject3D * objet,float r,float f,int NITER);
	//! actualise mesh etc.
	void Actualise(int tag);

	//! matrix of the object can be externaly specified
	void OwnerMatrix(CMatrix m);
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	Physic Engine
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

#ifdef _DEFINES_API_CODE_PHYSIC_
#include "physic.h"
#endif
#include "pob_generator.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	Generator : handles simple operations for 3D objects
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

#ifdef _DEFINES_API_CODE_GENERATOR_

#include "generator.h"

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CFigureCalc
{
public:
	CList <CVector> List;
	int tag;
	CFigure fig;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	General functions and datas for lightmaps,etc
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

void naming(char *dst,char *src);
 
extern void SetLightMapSize(int len);

extern int SIZE_LIGHTMAPS;

extern CObject3D * ConvertConnerie(CObject3D *obj);
extern CObject3D * VertexExtrude(CObject3D *obj);

extern int COUNT_TAG_FACEFEELING;

// sleep pack awake
void InitTemporaryFilesProtocol();
void FreeTemporaryFilesProtocol();
bool TemporaryFilesProtocol();


bool IntersectABCD(CVector &P,CVector &Q,CVector &A,CVector &B,CVector &C,CVector &D,CVector &I,CVector &N);
float IntersectionABCD(CVector &A,CVector &B,CVector &AA,CVector &BB,CVector &CC,CVector &DD,float &xm,float &ym);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
