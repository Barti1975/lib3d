
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

#include "include.h"
#include "objects3d.h"
#include "vertexprogram.h"

#ifdef IOS
#define FNCALLCONVVB
#else
#ifdef ANDROID
#define FNCALLCONVVB
#else
#ifdef LINUX
#define FNCALLCONVVB
#else
#ifdef X64
#define FNCALLCONVVB
#else
#ifdef API3D_METAL
#define FNCALLCONVVB
#else
#define FNCALLCONVVB __fastcall
#endif
#endif
#endif
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Vertex Buffer creation parameters
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OFS(a) ((float*)((char*)NULL+(a)))

#define API3D_TRIANGLESLIST		1			// not supported at this time

#define API3D_VERTEXDATAS		1			// coord. of vertex, object space
#define API3D_COLORDATAS		2			// diffuse compoment
#define API3D_TEXCOODATAS		4			// one level of texture
#define API3D_NORMALDATAS		8			//normals
#define API3D_CALCMATRICE		16			//

#define API3D_COLORDATAS4		2

#define API3D_TEXCOODATAS2		64			// multitexturing 2 mapping coord.
#define API3D_TEXCOO2DATAS		64			// multitexturing 2 mapping coord.
#define API3D_TEXPERVERTEX 		128			// specifies to use Vertices[].Map values fot texturing instead of Faces[].mpx
#define API3D_VERTEXCALC 		256			// take the .Calc value instead of .Stok
#define API3D_TEXCOO2LIGHTMAP   512			// simple texture, lightmap

#define API3D_TEXCOO3BUMP		1024		// not supported in openGL, specifies 2 pass algo with texture, color lightmap,
											// and vector lightmap
#define	API3D_ENVMAPPING		2048		// approch to phong the vector L of the vb is use as center of env map
#define API3D_EDGESDG			4096
#define API3D_ENABLEUPDATE		8192

#define API3D_VERTEXPROGRAM		16384
#define API3D_BLENDING			32768
#define API3D_TEXTURE_NT2		65536

#define API3D_STYLEBUMP1		65536*2
#define API3D_TEXCOO4DATAS		65536*4
#define API3D_MORPH				65536*8
#define API3D_NOSECONDARY		65536*16
#define API3D_CONSTANT			65536*32
#define API3D_DEGMETHOD2		65536*64

#define API3D_NOREFTAGONE		65536*128

#define API3D_STREAMS			65536*256

#define API3D_FOURTEX			65536*512

#define API3D_DYNAMIC			65536*1024

#define API3D_DIFFUSEASSTOK0    65536*2048
#define API3D_COLORNORMALTEX2   65536*2048*2

#ifdef API3D_DIRECT3D
	#include "customvertex.h"
#endif

#ifdef API3D_DIRECT3D9
	#include "customvertex.h"
#endif

#ifdef API3D_DIRECT3D10
	#include "customvertex.h"
#endif

#ifdef API3D_DIRECT3D11
	#include "customvertex.h"
#endif

#ifdef API3D_DIRECT3D12
	#include "customvertex.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Vertex Buffer class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef API3D_DIRECT3D11

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#define D3DCOLOR_RGBA(r,g,b,a) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#endif

#ifdef API3D_DIRECT3D12

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#define D3DCOLOR_RGBA(r,g,b,a) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CIndicesSE
{
public:
	int iStart;
	int iEnd;
	int vStart;
	int vEnd;
    
    int iStartBase;
    int iEndBase;

	short int nT,nT2,nL;
	short int nLV;
	short int nSV;
	short int ref;
	short int tag;
    
    int flag;
    int strip;

    CIndicesSE() { iStartBase=iEndBase=vEnd=-1; flag=-1; strip=0; }
};


enum
{
	UNASSIGNED				=	0,
	TEXTURE_FROM_NT			=	1,
	TEXTURE_FROM_NT2		=	2,
	TEXTURE_FROM_NL			=	3,
	TEXTURE_FROM_NLVECTORS	=	4,
	TEXTURE_FROM_BUMP		=	5,
	TEXTURE_FROM_RENDER_0	=	6,
	TEXTURE_FROM_RENDER_1	=	7,
	TEXTURE_FROM_RENDER_2	=	8,
	TEXTURE_FROM_RENDER_3	=	9,
	TEXTURE_FROM_RENDER_4	=	10,
	TEXTURE_FROM_RENDER_5	=	11,
	TEXTURE_FROM_RENDER_6	=	12,
    TEXTURE_FROM_RENDER_7	=	13,
    TEXTURE_FROM_NTPLUSONE  =   14,
    TEXTURE_FROM_BUMPPLUSONE  =   15,
	TEXTURE_FROM_DISPLACE	=	16,
	TEXTURE_FROM_HORIZON	=	17
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderElement
{
public:
	int type;
	char name[64];
	float f;
	CVector4 v;
	CMatrix m;
	CVector4 vs[256];
	CMatrix ms[256];
	int vstexture;
	int texture;
	int indexmax;

	ShaderElement() { type=0; strcpy(name,""); indexmax=0; }

	void setFloat(float set) { type=1; f=set; }
	void setVector(CVector set) { type=2; v.x=set.x; v.y=set.y; v.z=set.z; v.w=0; }
	void setVector4(CVector4 set) { type=2; v=set; }
	void setMatrix(CMatrix set) { type=3; m=set; }
	void setVectorElement(int ndx,CVector set) { type=4; vs[ndx].x=set.x; vs[ndx].y=set.y; vs[ndx].z=set.z; vs[ndx].w=0; if (ndx>indexmax) indexmax=ndx;}
	void setMatrixElement(int ndx,CMatrix set) { type=5; ms[ndx]=set; if (ndx>indexmax) indexmax=ndx; }
	void setTexture(int set) { type=6; texture=set; }
	void setVSTexture(int set) { type=7; vstexture=set; }
	void setMatrixWorld() { type=8; }
	void setMatrixViewProj() { type=10; }
	void setMatrixViewProjTex() { type=11; }
	void setMatrixWorldViewProj() { type=12; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Data structure for object in pre-rendering
class API3D_STUB CVertexBuffer
{
public:
	char Name[32];
	unsigned int Type;
	int Mode;
	unsigned int nVertices;
	unsigned int nIndices;
	CVertexBuffer * secondary;
    CVertexBuffer * lod;
	CVertexBuffer * from;
	int _sizeVB;
	int _blend;
	int _blendN;
	int _temp;
	bool Grouped;
	unsigned int nVerticesActif;
	unsigned int nIndicesActif;
	int TypeVB;
	CList <CIndicesSE> ListeSE;
	CObject3D * ref;
	CVector L,Ldir;
	float alpha;
	int iValue1,iValue2;
	float fValue1,fValue2;
	int *CorresVertices;
	int *CorresVerticesNorm;
	CVertexProgram *vp;
	void * capi;
	char * AssignedTexture[18];
	int Lights[3];
	int stream_locked;
	float stream_interpolant;
	int cloned;
	int hasbeencloned;
	unsigned char tag;
	int dynamic;
	int secondary_pos;
	int secondary_vertex_pos;
	int stream1,stream2;
	bool HasRef;
	int reference;
	int context;
	bool modifparams;
	bool init;
    bool update;
    int flag;
    bool value_coef;
    int prop;
    int partial;
    CMatrix ViewProj;
	CList <ShaderElement> shadervariables;
	int fsr0,fsr1;
	int * sorting;
	int dirty;

#ifdef API3D_DIRECT3D

	LPD3DXEFFECT effect;
	D3DXTECHNIQUE_DESC tdesc;

	int pos_texture;
	LPDIRECT3DBASETEXTURE8	Texture[10];

	BYTE * pVertices;
	BYTE * pIndices;

	BYTE * Vertices;
	BYTE * Indices;

	LPDIRECT3DVERTEXBUFFER8 D3D8VB;
	LPDIRECT3DVERTEXBUFFER8 *STREAMS;
	int nSTREAMS;
	LPDIRECT3DINDEXBUFFER8 D3D8INDEX;
	LPDIRECT3DDEVICE8       D3DDevice;

	CVertexBuffer()
	{
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;
		
		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		HasRef=true;
		Grouped=false;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		_sizeVB=-1;
		tag=0;
		stream_locked=-1;
		pos_texture=0;

		reference=-666;

		Texture[0]=NULL;
		Texture[1]=NULL;
		Texture[2]=NULL;
		Texture[3]=NULL;
		Texture[4]=NULL;
		Texture[5]=NULL;
		Texture[6]=NULL;
		Texture[7]=NULL;
		Texture[8]=NULL;
		Texture[9]=NULL;

		capi=NULL;
		vp=NULL;
		effect=NULL;
		CorresVertices=NULL;
		CorresVerticesNorm=NULL;
		D3DDevice=NULL;
		D3D8VB=NULL;
		D3D8INDEX=NULL;

		Vertices=NULL;
		Indices=NULL;

		STREAMS=NULL;

		for (int n=0;n<18;n++) AssignedTexture[n]=NULL;

		Lights[0]=0;
		Lights[1]=0;
		Lights[2]=0;

		iValue1=-1;

		secondary=NULL;
		hasbeencloned=cloned=0;
		dynamic=0;
	}
#endif


#ifdef API3D_DIRECT3D9

	LPD3DXEFFECT effect;
	D3DXTECHNIQUE_DESC tdesc;

	int pos_texture;
	LPDIRECT3DBASETEXTURE9	Texture[10];

	BYTE * pVertices;
	BYTE * pIndices;

	BYTE * Vertices;
	BYTE * Indices;

	BYTE ** VerticesStream;

	LPDIRECT3DVERTEXBUFFER9 *STREAMS;
	int nSTREAMS;
	LPDIRECT3DVERTEXBUFFER9 D3D9VB;
	LPDIRECT3DINDEXBUFFER9 D3D9INDEX;
	LPDIRECT3DDEVICE9 D3DDevice;

	CVertexBuffer()
	{
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		HasRef=true;
		Grouped=false;
		VerticesStream=NULL;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;
		pos_texture=0;

		reference=-666;

		Texture[0]=NULL;
		Texture[1]=NULL;
		Texture[2]=NULL;
		Texture[3]=NULL;
		Texture[4]=NULL;
		Texture[5]=NULL;
		Texture[6]=NULL;
		Texture[7]=NULL;
		Texture[8]=NULL;
		Texture[9]=NULL;

		capi=NULL;
		vp=NULL;
		effect=NULL;
		CorresVertices=NULL;
		CorresVerticesNorm=NULL;
		D3DDevice=NULL;
		D3D9VB=NULL;
		D3D9INDEX=NULL;
		STREAMS=NULL;
		
		Vertices=NULL;
		Indices=NULL;

		for (int n=0;n<18;n++) AssignedTexture[n]=NULL;

		Lights[0]=0;
		Lights[1]=0;
		Lights[2]=0;

		iValue1=-1;

		secondary=NULL;
		hasbeencloned=cloned=0;
	}
#endif


#ifdef API3D_DIRECT3D10

	ID3D10Effect* effect;
	ID3D10EffectTechnique* tech;
	ID3D10InputLayout* layout;

	BYTE * pVertices;
	BYTE * pIndices;

	BYTE * Vertices;
	BYTE * Indices;

	BYTE ** VerticesStream;

	ID3D10Buffer * *STREAMS;
	int nSTREAMS;
	ID3D10Buffer * D3DVB;
	ID3D10Buffer * D3DINDEX;
#ifdef API3D_VERSION_DX10_1
	ID3D10Device1* D3DDevice;
#else
	ID3D10Device* D3DDevice;
#endif

	ID3D10EffectShaderResourceVariable* res[18];
	ID3D10EffectVectorVariable* vectors[256];
	ID3D10EffectMatrixVariable* matrices[256];

	CVertexBuffer()
	{
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		for (int n=0;n<256;n++)
		{
			vectors[n]=NULL;
			matrices[n]=NULL;
		}

		HasRef=true;
		Grouped=false;
		layout=NULL;
		VerticesStream=NULL;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;

		reference=-666;

		tech=NULL;
		capi=NULL;
		vp=NULL;
		effect=NULL;
		CorresVertices=NULL;
		CorresVerticesNorm=NULL;
		D3DDevice=NULL;
		D3DVB=NULL;
		D3DINDEX=NULL;
		STREAMS=NULL;
		
		Vertices=NULL;
		Indices=NULL;

		for (int n=0;n<18;n++)
		{
			AssignedTexture[n]=NULL;
			res[n]=NULL;
		}

		Lights[0]=0;
		Lights[1]=0;
		Lights[2]=0;

		iValue1=-1;

		secondary=NULL;
		hasbeencloned=cloned=0;
	}
#endif

#ifdef API3D_DIRECT3D11

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* tech;

	ID3D11InputLayout* layout;

	BYTE * pVertices;
	BYTE * pIndices;

	BYTE * Vertices;
	BYTE * Indices;

	BYTE ** VerticesStream;

	ID3D11Buffer * *STREAMS;
	int nSTREAMS;
	ID3D11Buffer * D3DVB;
	ID3D11Buffer * D3DINDEX;
#ifndef WINDOWS_PHONE
	ID3D11Device* D3DDevice;
	ID3D11DeviceContext* D3DDeviceContext;
#else
	ID3D11Device2* D3DDevice;
	ID3D11DeviceContext2* D3DDeviceContext;
#endif
	ID3DX11EffectShaderResourceVariable* res[18];
	ID3DX11EffectVectorVariable* vectors[256];
	ID3DX11EffectMatrixVariable* matrices[256];
	ID3DX11EffectMatrixVariable* palette;

	CVertexBuffer()
	{
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		for (int n=0;n<256;n++)
		{
			vectors[n]=NULL;
			matrices[n]=NULL;
		}

		palette=NULL;

		HasRef=true;
		Grouped=false;
		layout=NULL;
		VerticesStream=NULL;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;

		reference=-666;

		capi=NULL;
		vp=NULL;
		effect=NULL;
		CorresVertices=NULL;
		CorresVerticesNorm=NULL;
		D3DDevice=NULL;
		D3DDeviceContext=NULL;
		D3DVB=NULL;
		D3DINDEX=NULL;
		STREAMS=NULL;
		
		Vertices=NULL;
		Indices=NULL;

		for (int n=0;n<18;n++)
		{
			res[n]=NULL;
			AssignedTexture[n]=NULL;
		}

		Lights[0]=0;
		Lights[1]=0;
		Lights[2]=0;

		iValue1=-1;

		secondary=NULL;
		hasbeencloned=cloned=0;
	}
#endif

#ifdef API3D_DIRECT3D12

	ID3DFXEffect* effect;
	ID3DFXEffectTechnique* tech;

	ID3D12RootSignature* signature;
	ID3D12CommandSignature *commandsignature;

	CMap<int,ID3D12PipelineState*> pipelines;

	int actualcrc;

	D3D12_INPUT_ELEMENT_DESC* layout;
	int nlayout;

	BYTE * pVertices;
	BYTE * pIndices;

	BYTE * Vertices;
	BYTE * Indices;

	BYTE ** VerticesStream;

	ID3D12Resource * *STREAMS;
	ID3D12Resource * STREAMSTMP;
	D3D12_VERTEX_BUFFER_VIEW *STREAMSVIEW;
	int nSTREAMS;

	ID3D12Resource * D3DVB;
	ID3D12Resource * D3DVBTMP;
	D3D12_VERTEX_BUFFER_VIEW D3DVBVIEW;
	ID3D12Resource * D3DINDEX;
	ID3D12Resource * D3DINDEXTMP;
	D3D12_INDEX_BUFFER_VIEW D3DINDEXVIEW;

	ID3D12Device* D3DDevice;

	DescriptorHeapHandleContainer *dhhc;

	ID3DFXEffectShaderResourceVariable* res[18];
	ID3DFXEffectVectorVariable* vectors[256];
	ID3DFXEffectMatrixVariable* matrices[256];
	ID3DFXEffectMatrixVariable* palette;

	CVertexBuffer()
	{
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		for (int n=0;n<256;n++)
		{
			vectors[n]=NULL;
			matrices[n]=NULL;
		}

		palette=NULL;

		signature=NULL;
		commandsignature=NULL;

		HasRef=true;
		Grouped=false;
		layout=NULL;
		VerticesStream=NULL;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;

		reference=-666;

		capi=NULL;
		vp=NULL;
		effect=NULL;
		CorresVertices=NULL;
		CorresVerticesNorm=NULL;
		D3DDevice=NULL;

		D3DVB=NULL;
		D3DINDEX=NULL;
		STREAMS=NULL;
		
		D3DVBTMP=NULL;
		D3DINDEXTMP=NULL;

		Vertices=NULL;
		Indices=NULL;

		for (int n=0;n<18;n++)
		{
			//res[n]=NULL;
			AssignedTexture[n]=NULL;
		}

		Lights[0]=0;
		Lights[1]=0;
		Lights[2]=0;

		iValue1=-1;

		secondary=NULL;
		hasbeencloned=cloned=0;
	}
#endif


#if defined(API3D_METAL)

	int VB;
    int VBBase;
	int IB;
    int sVB[1024];
    
    unsigned char * data_metal;
    unsigned int size_data_metal;
    
	float** streams;
	float** streams_norms;
    
    bool modiftextures;
    
	int nstreams;
	unsigned int * Index_Array;


	float * Vertex_Array;

	float * Color_Array;
	float * Normal_Array;

	float * TexCoo_Array;
	float * TexCoo2_Array;
	float * TexCoo3_Array;
	float * TexCoo4_Array;
	
	float * Weights_Array;
	float * WeightsIndices_Array;
	unsigned char * Indices_Array;
    
    int FX;

    Env * roots[256];
	Env * palette;

	unsigned int numTexture[16];
	
    Env * res[18];

	CVertexBuffer() 
	{
        int n;

		nVertices=0;
		nIndices=0;

        data_metal=NULL;
        size_data_metal=0;
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;
        
		init=false;

		memset(Name,0,32);
        
		HasRef=true;
		Grouped=false;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;
        
        modiftextures=true;
        
        FX=-1;
        
        vp=NULL;

		reference=-666;

		for (n=0;n<18;n++)
        {
            AssignedTexture[n]=NULL;
            res[n]=NULL;
        }
        
        for (int n=0;n<256;n++) roots[n]=NULL;

		palette=NULL;
        
        capi=NULL;
		vp=NULL;
		CorresVertices=NULL;
		
		Index_Array=NULL;

		Color_Array=NULL;
		Vertex_Array=NULL;
		TexCoo_Array=NULL;
		TexCoo2_Array=NULL;
		TexCoo3_Array=NULL;
		TexCoo4_Array=NULL;
		Normal_Array=NULL; 
		Indices_Array=NULL;
		Weights_Array=NULL;
		WeightsIndices_Array=NULL;

		iValue1=-1;
		secondary=NULL;
		CorresVerticesNorm=NULL;
		
		streams=NULL;
		streams_norms=NULL;

		IB=VB=-1;

		hasbeencloned=cloned=0;
	}

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

#ifdef OPENGL_VBO
	GLuint VB;
	GLuint IB;
	bool vbo;
#endif

	float * temp_vertices;
	float * temp_normals;

	float** streams;
	float** streams_norms;

	float** Bstreams;
	float** Bstreams_norms;

	int nstreams;
#if defined(GLES)||defined(GLES20)
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
	unsigned int * Index_Array;
	unsigned int * BIndex_Array;

	unsigned short int * Index_Array16;
	unsigned short int * BIndex_Array16;
#else
	unsigned short int * Index_Array;
	unsigned short int * BIndex_Array;
#endif
#else
	GLuint * Index_Array;
	GLuint * BIndex_Array;
#endif
	float * Color_Array;
	float * Vertex_Array;
	float * TexCoo_Array;
	float * TexCoo2_Array;
	float * TexCoo3_Array;
	float * TexCoo4_Array;
	float * Normal_Array;
	float * Weights_Array;
	float * WeightsIndices_Array;
	unsigned char * Indices_Array;

	float * BColor_Array;
	float * BVertex_Array;
	float * BTexCoo_Array;
	float * BTexCoo2_Array;
	float * BTexCoo3_Array;
	float * BTexCoo4_Array;
	float * BNormal_Array;
	float * BWeights_Array;
	float * BWeightsIndices_Array;
	unsigned char * BIndices_Array;

    Env * roots[256];
	// temporary buffers

	CVector * Normal_Array_stok;
	float * Vertex2_Array;

	unsigned int numTexture[16];
	
	GLuint programs[16];	
	GLuint fprograms[16];
#if !defined(GLES) || defined(API3D_OPENGL20)
#if defined(ANDROID)||defined(IOS)
	GLuint glslprograms[16];
	GLuint vert_shader[16];
	GLuint frag_shader[16];
#else
#ifdef API3D_VR
	GLuint glslprograms[16];
	GLuint vert_shader[16];
	GLuint frag_shader[16];
#else
	GLhandleARB glslprograms[16];
	GLhandleARB vert_shader[16];
	GLhandleARB frag_shader[16];
#endif
#endif
#endif
	bool fp[16];
	char * str_programs[16];
	char * str_fprograms[16];
    Env * res[18];
    
	CVertexBuffer() 
	{ 
		init=false;

		nVertices=0;
		nIndices=0;

		memset(Name,0,32);
        
        lod=NULL;
		from=NULL;

		fsr0=fsr1=0;
		sorting=NULL;
		dirty=0;

		HasRef=true;
		vbo=false;
		Grouped=false;
		stream1=-1;
		stream2=0;
		stream_interpolant=0.0f;
		dynamic=0;
		tag=0;
		stream_locked=-1;

		reference=-666;

		for (int n=0;n<18;n++)
        {
            AssignedTexture[n]=NULL;
            res[n]=NULL;
        }
        
        for (int n=0;n<256;n++) roots[n]=NULL;
        
        capi=NULL;
		vp=NULL;
		CorresVertices=NULL;
		
		Index_Array=NULL;
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
		Index_Array16=NULL;
#endif
		Color_Array=NULL;
		Vertex_Array=NULL;
		TexCoo_Array=NULL;
		TexCoo2_Array=NULL;
		TexCoo3_Array=NULL;
		TexCoo4_Array=NULL;
		Normal_Array=NULL; 
		Indices_Array=NULL;
		Weights_Array=NULL;
		WeightsIndices_Array=NULL;

		BColor_Array=NULL;
		BVertex_Array=NULL;
		BTexCoo_Array=NULL;
		BTexCoo2_Array=NULL;
		BTexCoo3_Array=NULL;
		BTexCoo4_Array=NULL;
		BNormal_Array=NULL;
		BIndices_Array=NULL;
		BWeights_Array=NULL;
		BWeightsIndices_Array=NULL;

		Vertex2_Array=NULL;
		Normal_Array_stok=NULL;
		iValue1=-1;
		secondary=NULL;
		CorresVerticesNorm=NULL;
		
		streams=NULL;
		streams_norms=NULL;

		Bstreams=NULL;
		Bstreams_norms=NULL;

		hasbeencloned=cloned=0;
	}

#endif

	//? Vertex program access and functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! assignation for render.createmultigroupfrom created vertexbuffer
	void resetAssignation();
	//! Specifies a matrix value in an vertex program matrix array.
	void setMatrixArrayElement(char *name,int ndx,CMatrix const &M);
	//! vertex program spefs
	void setMatrix(char *name,CMatrix const &M);
	//! Specifies a vector value in an vertex program vector array.
	void setVectorArrayElement(char *name,int ndx,CVector const &v);
    //! Specifies a vector value in an vertex program vector array.
    void setVector4ArrayElement(char *name,int ndx,CVector4 const &v);
	//! Set a point
	void setPoint(char *name,CVector const &v);
	//! Set a vector4
	void setVector4(char *name,CVector4 const &v);
	//! Set a vector
	void setVector(char *name,CVector const &v);
	//! Set a color
	void setRGB(char *name,CRGBA const &col);
	//! Set a color
	void setRGB(char *name,float r,float g,float b);
    //! Set a color
    void setRGBA(char *name,float r,float g,float b,float a);
	//! vertex program spefs
	void setDword(char *name,unsigned int d);
	//! Set float value. (this is (f,f,f,f) vector)
	void setFloat(char *name,float f);
	//! Set a Matrix to C3DAPIBASE WORLD matrix.
	void setMatrixWorld(char *name);
	//! Set a Matrix to C3DAPIBASE VIEW*PROJ matrix.
	void setMatrixViewProj(char *name);
	//! Set a Matrix to C3DAPIBASE VIEW*PROJ matrix with no inversion of y in OPENGL.
	void setMatrixViewProjTex(char *name);
	//! Set a Matrix to C3DAPIBASE WORLD*VIEW*PROJ matrix.
	void setMatrixWorldViewProj(char *name);
	//! Set texture number.
	void setTexture(char *name,int n);
	//! Set texture number.
	void setDepthTexture(char *name,int n);
    //! Set texture number.
    void setTextureVS(char *name,int n);
	//! Set lightmap number.
	void setLightmap(char *name,int n);
	//! Set texture vertex buffer association.
	//! assignation for render.createmultigroupfrom created vertexbuffer
	//! @param assign : name
	//! @param value : can be one of the following (TEXTURE_FROM_NT,TEXTURE_FROM_NT2,TEXTURE_FROM_NL,TEXTURE_FROM_NLVECTORS,TEXTURE_FROM_BUMP)
	//!	This means that when defining a texture in VP like TEXTURE tex; you can associate this name with a faces[].flag of the Cobject3D derivate vertex buffer created by CreateMUltiGroupFrom() method from C3DAPIBASE.
	void setTextureAssociation(char * assign,int value);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! simple vertexbuffer to attach vertex program
	void BasicEffect();
	//! set vertexprogram of a basic effect or vertexbuffer
	//! see shaders informations
	char* SetVertexProgram(char * script,int ext);
    //! set vertexprogram of a basic effect or vertexbuffer
    //! see shaders informations
    char* SetVertexProgram(char * script);
	//! clone vertexprogram of a basic effect
	void CloneVertexProgram(CVertexBuffer *vb);
    //! clone vertexprogram of a basic effect
    void ResetVariablesVertexProgram();

	//! element hot patch
	ShaderElement *getName(char *str);
    
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//? Data acces and initialisation

	//! internal/multi device
	void SetAPI(void * api);
	//! locking/unlocking stream buffers
	void LockStream(int n);
	//! locking/unlocking stream buffers
	void UnlockStream();
	//! locking the buffer for writing
	void LockVertices();
	//! unlock
	void UnlockVertices();
	//! locking the buffer for writing
	void LockIndices();
	//! unlock
	void UnlockIndices();
	//! free ressources
	void Release();
	//! free ressources
	void Purge();
	//! set a float for interpolation
	//! For a API3D_MORPH vertex buffer, this represents the interpolant coefficient [0,1] of the two streams.
	void SetInterpolant(float t) { stream_interpolant=t; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! set lights internal value
	void SetLight(int n,int nl) { Lights[n]=nl; }
	//! set light internal value for ENVMAPPING
	//! For API3D_ENVMAPPING vertex buffers and for EDGES_DG vertex buffers this represent the renderer light number.
	void SetLight(int nl) { iValue1=nl; }
	//! set projection leght for EDGESDG vb (stencil shadowing)
	//! For EDGES_DG vertex buffers used as stencil shadow volumes drawing, this represent the length in current frame of the projection 	of vertices from the light.
	void SetProjectionLength(float d) { fValue1=d; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! fill the secondary stream for morph objects
	void FillMorphStream(int stream,CObject3D * obj,int key);


	//! fill streams for morph objects specially for edges dg vertex buffer
	void FillMorphStreamEDGESDG(int stream,CObject3D *obj);
	//! fill streams for morph objects specially for edges dg vertex buffer
	void FillMorphStreamEDGESDGCalc(int stream,CObject3D *obj);

	void GetVertex(int n,float*x,float*y,float*z);

	/*!
		Set vertex buffer type, as a combinaison of the following parameters.

		API3D_VERTEXDATAS,API3D_COLORDATAS,API3D_TEXCOODATAS,API3D_NORMALDATAS,
		API3D_CALCMATRICE,API3D_COLORDATAS4,API3D_TEXCOODATAS2,API3D_TEXCOO2DATAS,
		API3D_TEXPERVERTEX,API3D_VERTEXCALC,API3D_TEXCOO2LIGHTMAP,API3D_TEXCOO3BUMP,
		API3D_ENVMAPPING,API3D_EDGESG,API3D_ENABLEUPDATE,API3D_VERTEXPROGRAM,
		API3D_BLENDING,API3D_TEXTURE_NT2,API3D_TEXCOO4DATAS,API3D_MORPH,API3D_CONSTANT

		this has some restrictions:
		TEXCOO3DBUMP are allowed only with TEXCOO2LIGHTMAP flag without NORMALDATAS flag.
		An extension of this king of vertex buffer is TEXCOO4DATAS + NORMALDATAS
		Those two kind of vertex buffer are for bump mapping lightmap internal drawing.

		Examples:

		Color vertex buffer:

		API3D_VERTEXDATAS|API3D_COLORDATAS|API3D_TEXCOODATAS + API3D_TEXPERVERTEX



		Lighting Engine vertex buffer:(LIGHTINING==ON)

		API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX



		Environment mapping vertex buffer:

		API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_ENVMAPPING|API3D_TEXPERVERTEX

		several parameters have to be set when using this kind of vb; vb->SetLight() etc.



		Bump mapping:

		API3D_VERTEXDATAS|API3D_COLORDATAS|API3D_TEXCOODATAS|API3D_TEXCOO2LIGHTMAP|
		API3D_TEXCOO3BUMP*BUMP_MAPPING|API3D_TEXPERVERTEX|API3D_STYLEBUMP1*BUMP_MAPPING|
		DISPLACEMENT*(API3D_TEXCOO4DATAS|API3D_NORMALDATAS)



		All vertex buffer type can be completed by one of the following flag:

		API3D_ENABLEUPDATE	: saves main data informations in complementary array to allow update

		API3D_ENVMAPPING	: renderer speicfic environment mapping
		API3D_BLENDING	: creates a vertex buffer with weights and matrix indices
		API3D_TEXTURE_NT2	: fill .nt2 for multigroup vertex buffer
		API3D_STYLEBUMP1	: renderer specific for bump mapping

		API3D_VERTEXCALC	: get .Calc value of the object when creating vertex buffer with CreateFrom() methods.

		API3D_MORPH	: 2 streams vertex buffer
		API3D_CONSTANT	: vertex buffer are dynamic by default, specify this flag to enable drivers to locate vertex buffer directly in video RAM
	*/
	void SetType(int t) { Type=t; }
	//! Mode settings.*
	void SetMode(int m) { Mode=m; }

	//! initialisation
	void Init(int nv,int ni);
	//! initialisation
	//! @param nv : vertices
	//! @param ni : indices
	//! @param ns : secondary streams
	//! Note: nv vertices, ni indices ( = nfaces*3)
	void Init(int nv,int ni,int ns);

	//! Specifies rgb component of a vertex.
	void FNCALLCONVVB SetColor(int n,float r,float g,float b,float a);
	//! Specifies rgb component of a vertex.
	void FNCALLCONVVB SetColor(int n,float r,float g,float b);
	//! Specifies rgb component of a vertex.
	void FNCALLCONVVB SetColor(int n,unsigned int color);
	//! Specifies rgb component of a vertex.
	void FNCALLCONVVB SetColor(int n,CRGBA &col);
	//! Specifies vertex coordinates.
	void FNCALLCONVVB SetVertex(int n,float x,float y,float z);
	//! Specifies vertex texture coordinates. Stage 0.
	void FNCALLCONVVB SetTexCoo(int n,float x,float y);
	//! Specifies vertex texture coordinates. Stage 1.
	void FNCALLCONVVB SetTexCoo2(int n,float x,float y);
	//! Specifies vertex texture coordinates. Stage 2.
	void FNCALLCONVVB SetTexCoo3(int n,float x,float y);
	//! Specifies vertex texture coordinates. Stage 3.
	void FNCALLCONVVB SetTexCoo4(int n,float x,float y);
	//! Set Normal data
	void FNCALLCONVVB SetNormal(int n,float x,float y,float z);
	//! Specifies vertex coordinates.
	void FNCALLCONVVB SetVertex(int n,CVector &v);
	//! Set Normal data
	void FNCALLCONVVB SetNormal(int n,CVector &v);
	//!	Specifies vertex texture coordinates. Stage 0.
	void FNCALLCONVVB SetTexCoo(int n,CVector2 &map);
    //! Specifies vertex texture coordinates. Stage 0.
    void FNCALLCONVVB SetTexCoo(int n,UCVector2 &map);
	//! Specifies vertex texture coordinates. Stage 1.
	void FNCALLCONVVB SetTexCoo2(int n,CVector2 &map);
    //! Specifies vertex texture coordinates. Stage 1.
    void FNCALLCONVVB SetTexCoo2(int n,UCVector2 &map);
	//! Specifies vertex texture coordinates. Stage 2.
	void FNCALLCONVVB SetTexCoo3(int n,CVector2 &map);
	//! Specifies vertex texture coordinates. Stage 3.
	void FNCALLCONVVB SetTexCoo4(int n,CVector2 &map);
	//! Set matrix weights
	void FNCALLCONVVB SetWeights(int n,int w,float value);
	//! Set matrix indices
	void FNCALLCONVVB SetMatrixIndex(int n,int w,int index);
	//! Set matrix weights
	void FNCALLCONVVB InitWeights(int n,int w);
	//! Set matrix indices
	void FNCALLCONVVB InitMatrixIndex(int n,int w);
	//! Set indices of a vertex
	void FNCALLCONVVB SetIndices(int n,int n0,int n1,int n2);
    //! Set indices of a vertex
    void FNCALLCONVVB SetIndice(int n,int n0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! General creation functions

	//! automatic create fonctions
	//! create a VB from *obj of faces with [].tag==tag
	//! type specifies the data stored in vb : see VB PARAMETERS
	//! all the object will be VB
	void CreateFrom(CObject3D * obj,int tag,int type);
	//! automatic create fonctions
	void CreateFrom2(CObject3D * obj,int type);

	//! create a special VB to apply environment mapping
	void PhongVertexBuffer(CObject3D * obj);

	//! creates a vertex buffer dedicated to stencil shadow volume
	void CreateDegeneratedEdges(CObject3D * obj);
	//! creates a vertex buffer dedicated to stencil shadow volume
	void CreateDegeneratedEdges(CObject3D * obj,unsigned int flags);

	//! creates a vertex buffer dedicated to stencil shadow volume (specialy for carmack reverse)
	//! This create a EDGES_DG vertex buffer that can be modifed throught the arrays following :
	//! (This arrays contain indices from the object to the vertexbuffer data)
	//! int *CorresVertices;	->SetVertex(n,obj->Vertices[CorresVertices[n]].Stok
	//! int *CorresVerticesNorm;	->SetNormal(n,obj->Vertices[CorresVerticesNorm[n]].Stok
	void CreateDegeneratedEdgesDynamic(CObject3D * obj);
	//! creates a vertex buffer dedicated to stencil shadow volume
	void CreateDegeneratedEdgesDynamic(CObject3D * obj,unsigned int flags);
	//! creates a vertex buffer dedicated to stencil shadow volume (with closing form in secondary vertex buffer)
	void CreateDegeneratedEdgesDynamicFacing(CObject3D * obj,unsigned int flags);
	//! creates a vertex buffer dedicated to stencil shadow volume (with closing form in secondary vertex buffer)
	void CreateDegeneratedEdgesDynamicFacing(CObject3D * obj);
	//! creates a vertex buffer dedicated to stencil shadow volume (with closing form merged, specially for alternative method)
	void CreateDegeneratedEdgesDynamicMerged(CObject3D * obj);
	//! creates a vertex buffer dedicated to stencil shadow volume (with closing form merged, specially for alternative method)
	void CreateDegeneratedEdgesDynamicMerged(CObject3D * obj,unsigned int flags);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//? Simple effects (!) no full optimization

	//! simple effect for phongvertexbuffer created vb
	//! Apply basic environment mapping function to a phong vertex buffer with attenuation.
	//! This is an old function.
	void ApplyPhong(CMatrix const &M,CObject3D * obj);
	//! simple effect for phongvertexbuffer created vb
	//! Apply basic environment mapping function to a phong vertex buffer with attenuation.
	//! This is an old function.
	void ApplyPhong(CMatrix const &M,CObject3D * obj,float fog);
	//! Apply basic environment mapping function to a phong vertex buffer with white fading.
	//! This is an old function.
	void ApplyWhitePhong(CMatrix const &M,CObject3D * obj,float t);
	//! simple effect for phongvertexbuffer created vb
	//! Interpolates between to object and fill vertex buffer.
	//! This is an old function.
	void ApplyMorph(CObject3D * obj1,CObject3D * obj2,float t);
	//! simple effect for phongvertexbuffer created vb
	void ApplySimplePhong(CMatrix const &M,CObject3D * obj);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class for effect context parametrization

class CContextEffectParameters
{
public:
	int start,end;

	void (*function_texture)(CVertexBuffer *effect,char *name,int nt);
	void (*function_texture2)(CVertexBuffer *effect,char *name,int nt);
	void (*function_vectors)(CVertexBuffer *effect,char *name,int nt);
	void (*function_vectors2)(CVertexBuffer *effect,char *name,int nt);
	bool (*function_set)(CVertexBuffer *effect,int nT,int nT2,int tag);
	char *names[128];
	int nb;

	CContextEffectParameters()
	{
		reset();
	}

	//! reset contexts
	void reset();

	//! define the contextual function by effect parameter
	void setVectorFunction(void (*fn_vectors)(CVertexBuffer *effect,char *name,int nt));
	//! define the contextual function by effect parameter
	void setVectorFunction2(void (*fn_vectors)(CVertexBuffer *effect,char *name,int nt));
	//! define the contextual function by effect parameter
	void setTextureFunction(void (*fn_texture)(CVertexBuffer *effect,char *name,int nt));
	//! define the contextual function by effect parameter
	void setTextureFunction2(void (*fn_texture)(CVertexBuffer *effect,char *name,int nt));
	//! define the contextual function. set the effect values according to nT,nT2,tag.
	void setContextFunction(bool (*function_set)(CVertexBuffer *effect,int nT,int nT2,int tag));
	//! define a name parameter
	void registerName(char *name);
	//! internal. calls contextual functions
	bool update(CVertexBuffer *effect,int nT,int nT2,int tag);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool VertexProgramDefinedVar(CVertexBuffer *shd,char *name);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CShaderString
{
public:
    char * shader;
    CVertexBuffer *fx;
	CList<CVertexBuffer *> list;
    int size;
    
    CShaderString() { list.Free(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
