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
//	@file vertexbuffer.cpp
//	@date 2004
////////////////////////////////////////////////////////////////////////

#include "params.h"

#include <math.h>
#include <stdio.h>

#include "maths.h"
#include "objects3d.h"
#include "3d_api_base.h"
#include "vertexbuffer.h"
#include "vertexbuffercontainer.h"
#include "edges.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  function:
//
//		Global function and variables for devices
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int api3d_number_vertexbuffers=0;
int api3d_number_registered_vertexbuffers=0;
int api3d_number_registerednew_vertexbuffers=0;

extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int							LIB3D_V2_MAIN_NOT_OPTIM		=		1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern	bool				HardwareVertexProcessing;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Duplicate Shaders Management
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CList<CShaderString>        AllocatedShaders;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CShaderString* IsSameShader(char *shader_string,int len)
{
    CShaderString *ss=AllocatedShaders.GetFirst();
    while (ss)
    {
        if (ss->size==len)
        {
            if (strcmp(ss->shader,shader_string)==0)
            {
                return ss;
            }
        }
        ss=AllocatedShaders.GetNext();
    }
    
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CShaderString* RegisterShader(char *shader_string,int len,CVertexBuffer *fx)
{
	CShaderString *ss=AllocatedShaders.InsertLast();
    ss->size=len;
    ss->shader=(char*)malloc(len+1);
    strcpy(ss->shader,shader_string);
    ss->fx=fx;
	return ss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UnregisterShader(CVertexBuffer *fx)
{
    CShaderString *ss=AllocatedShaders.GetFirst();
    while (ss)
    {
		if (ss->fx==fx->from)
        {
			CVertexBuffer **pfx=ss->list.GetFirst();
			while (pfx)
			{
				if (*pfx==fx)
				{
					ss->list.DeleteAndGetNext();
					return;
				}
				else pfx=ss->list.GetNext();
			}
            return;
        }
        else ss=AllocatedShaders.GetNext();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MigrateShader(CVertexBuffer *fx)
{
    CShaderString *ss=AllocatedShaders.GetFirst();
    while (ss)
    {
		if (ss->fx==fx)
        {
			if (ss->list.Length()==0)
			{
				free(ss->shader);
				AllocatedShaders.DeleteAndGetNext();
				return;
			}
			else
			{
				CVertexBuffer **pfx=ss->list.GetFirst();
				if (pfx)
				{
					ss->fx->hasbeencloned=0;
					ss->fx->cloned=1;

					ss->fx=*pfx;
					ss->list.DeleteFirst();

					ss->fx->cloned=0;
					ss->fx->hasbeencloned=1;

					if (ss->list.Length()>0)
					{
						pfx=ss->list.GetFirst();
						while (pfx)
						{
							(*pfx)->from=ss->fx;
							pfx=ss->list.GetNext();
						}
					}
					return;
				}
				else
				{
					free(ss->shader);
					AllocatedShaders.DeleteAndGetNext();
					return;
				}
			}
        }
        else ss=AllocatedShaders.GetNext();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SuppressShader(CVertexBuffer *fx)
{
    CShaderString *ss=AllocatedShaders.GetFirst();
    while (ss)
    {
		if (ss->fx==fx)
        {
			free(ss->shader);
			AllocatedShaders.DeleteAndGetNext();
			return;
        }
        else ss=AllocatedShaders.GetNext();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D

extern	LPDIRECT3DDEVICE8	D3DDevicePrincipal;
extern void	SetDEVICE(LPDIRECT3DDEVICE8 Device);

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D9

extern	LPDIRECT3DDEVICE9	D3DDevicePrincipal;
extern void	SetDEVICE(LPDIRECT3DDEVICE9 Device);

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D10

#ifdef API3D_VERSION_DX10_1
extern	ID3D10Device1*	D3DDevicePrincipal;
extern void	SetDEVICE(ID3D10Device1* Device);
#else
extern	ID3D10Device*	D3DDevicePrincipal;
extern void	SetDEVICE(ID3D10Device* Device);
#endif
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
#ifdef API3D_DIRECT3D11
#ifndef WINDOWS_PHONE
extern	ID3D11Device*			D3DDevicePrincipal;
extern	ID3D11DeviceContext*	D3DDeviceContextPrincipal; 

void SetDEVICE(ID3D11Device* Device,ID3D11DeviceContext *DeviceContext);
#else
extern	ID3D11Device2*			D3DDevicePrincipal;
extern	ID3D11DeviceContext2*	D3DDeviceContextPrincipal; 

void SetDEVICE(ID3D11Device2* Device,ID3D11DeviceContext2 *DeviceContext);
#endif

class D3DXMATRIX
{
public:
    union 
    {
        struct 
        {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
};

class D3DXVECTOR4
{
public:
    float x;
    float y;
    float z;
    float w;

	D3DXVECTOR4(float _x,float _y,float _z,float _w) { x=_x; x=_x; y=_y; z=_z; w=_w;}
};


#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D12
extern	ID3D12Device*			D3DDevicePrincipal;

void SetDEVICE(ID3D12Device* Device);

class D3DXMATRIX
{
public:
    union 
    {
        struct 
        {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
};

class D3DXVECTOR4
{
public:
    float x;
    float y;
    float z;
    float w;

	D3DXVECTOR4(float _x,float _y,float _z,float _w) { x=_x; x=_x; y=_y; z=_z; w=_w;}
};


#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern C3DAPIBASE *			TheClass3DAPI;
extern bool					G_Multitexture;
extern bool					G_Dot3;
extern bool					G_PS;
extern bool					G_PS2X;
extern bool					G_VertexProgram;
extern char					s_error[2048];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

#if defined(__APPLE__)||defined(API3D_VR)||defined(API3D_GL_PROTOTYPES)
#define NOGETPROC
#endif

#if defined(OCULUS)||defined(API3D_GL_PROTOTYPES)
#define glBindBufferARB	glBindBuffer
#define glBufferDataARB glBufferData
#define glDeleteBuffersARB	glDeleteBuffers
#define glGenBuffersARB	glGenBuffers
#define glBufferSubDataARB	glBufferSubData
#define	GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#define	GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
#define GLintptrARB GLintptr

#define glBindAttribLocationARB glBindAttribLocation
#define glGetAttribLocationARB glGetAttribLocation
#define glCreateProgramObjectARB glCreateProgram
#define glCreateShaderObjectARB glCreateShader
#define glShaderSourceARB glShaderSource
#define glCompileShaderARB glCompileShader
#define glGetObjectParameterivARB glGetShaderiv
#define glAttachObjectARB glAttachShader
#define glGetInfoLogARB glGetShaderInfoLog
#define glLinkProgramARB glLinkProgram
#define glUseProgramObjectARB glUseProgram
#define glGetUniformLocationARB glGetUniformLocation
#define glDetachObjectARB glDetachShader
#define glDeleteObjectARB glDeleteShader
#define glGetProgramivARB glGetProgramiv
#define glValidateProgramARB glValidateProgram
#define GLcharARB GLchar
#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
#endif

#if (!defined( NOGETPROC))&&(!defined(LINUX))||(defined(IOS))

#if !defined(GLES)&&!defined(GLES20)
#ifdef OPENGL_MULTITEXTURING
extern PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2fARB;
extern PFNGLACTIVETEXTUREARBPROC				glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTextureARB;
#endif
#endif

#ifdef OPENGL_VBO
#if !defined(GLES)&&!defined(GLES20)
extern PFNGLBINDBUFFERARBPROC					glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC					glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC					glGenBuffersARB;
extern PFNGLBUFFERSUBDATAARBPROC				glBufferSubDataARB;
#else
#define glBindBufferARB	glBindBuffer
#define glBufferDataARB glBufferData
#define glDeleteBuffersARB	glDeleteBuffers
#define glGenBuffersARB	glGenBuffers
#define glBufferSubDataARB	glBufferSubData
#define	GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#define	GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
#define GLintptrARB GLintptr
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OPENGL_GLSL
#ifdef GLES20
#define glBindAttribLocationARB glBindAttribLocation
#define glGetAttribLocationARB glGetAttribLocation
#define glCreateProgramObjectARB glCreateProgram
#define glCreateShaderObjectARB glCreateShader
#define glShaderSourceARB glShaderSource
#define glCompileShaderARB glCompileShader
#define glGetObjectParameterivARB glGetShaderiv
#define glAttachObjectARB glAttachShader
#define glGetInfoLogARB glGetShaderInfoLog
#define glLinkProgramARB glLinkProgram
#define glUseProgramObjectARB glUseProgram
#define glGetUniformLocationARB glGetUniformLocation
#define glDetachObjectARB glDetachShader
#define glDeleteObjectARB glDeleteShader
#define glGetProgramivARB glGetProgramiv
#define glValidateProgramARB glValidateProgram
#define GLcharARB GLchar
#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
#else
extern PFNGLCREATEPROGRAMOBJECTARBPROC			glCreateProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC			glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC					glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC				glCompileShaderARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC			glGetObjectParameterivARB;
extern PFNGLATTACHOBJECTARBPROC					glAttachObjectARB;
extern PFNGLGETINFOLOGARBPROC					glGetInfoLogARB;
extern PFNGLLINKPROGRAMARBPROC					glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC				glUseProgramObjectARB;
extern PFNGLDETACHOBJECTARBPROC					glDetachObjectARB;
extern PFNGLDELETEOBJECTARBPROC					glDeleteObjectARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC			glGetUniformLocationARB;
extern PFNGLUNIFORM4FARBPROC					glUniform4f;
extern PFNGLUNIFORMMATRIX4FVARBPROC				glUniformMatrix4fv;
extern PFNGLUNIFORM1IARBPROC					glUniform1i;
extern PFNGLGETPROGRAMIVARBPROC					glGetProgramivARB;
extern PFNGLVALIDATEPROGRAMARBPROC				glValidateProgramARB;
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OPENGL_VERTEXPROGRAMS
extern PFNGLPROGRAMSTRINGARBPROC				glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC					glBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC				glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC					glGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC		glProgramEnvParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC		glProgramLocalParameter4fARB;
extern PFNGLISPROGRAMARBPROC					glIsProgramARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointerARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC		glDisableVertexAttribArrayARB;
extern PFNGLGETPROGRAMIVARBPROC					glGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC				glGetProgramStringARB;
#endif

#endif
#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		return size of vertex buffer elements

		DWORD SizeVB(int TypeVB);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define memcopy1(dst,src) { *((char*)(dst))=*((char*)(src)); }
#define memcopy4(dst,src) { *((unsigned int*)(dst))=*((unsigned int*)(src)); }
#define memcopy8(dst,src) { ((unsigned int*)(dst))[0]=((unsigned int*)(src))[0];((unsigned int*)(dst))[1]=((unsigned int*)(src))[1]; }
#define memcopy12(dst,src) { ((unsigned int*)(dst))[0]=((unsigned int*)(src))[0];((unsigned int*)(dst))[1]=((unsigned int*)(src))[1];((unsigned int*)(dst))[2]=((unsigned int*)(src))[2]; }

#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
extern unsigned int LIB3D_TYPE_INDEX;
#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
    release vb
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FreeVB(CVertexBuffer *vb)
{
    vb->Release();
    delete vb;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 return size of vertex buffer elements
 
 DWORD SizeVB(int TypeVB);
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

unsigned int SizeVB(int TypeVB)
{
	unsigned int size;

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

	switch(TypeVB)
	{
	case 0:
		size=sizeof(struct XYZCOLORTEX);
		break;
	case 1:
		size=sizeof(struct XYZCOLORTEX);
		break;
	case 2:
		size=sizeof(struct XYZCOLOR);
		break;
	case 3:
		size=sizeof(struct XYZCOLORTEX);
		break;
	case 4:
		size=sizeof(struct XYZCOLORTEX2);
		break;
	case 5:
		size=sizeof(struct XYZNORMALTEX);
		break;
	case 6:
		size=sizeof(struct XYZCOLORTEX2);
		break;
	case 7:
		size=sizeof(struct XYZCOLORTEX3);
		break;
	case 8:
		size=sizeof(struct XYZNORMALTEX2);
		break;
	case 9:
		size=sizeof(struct XYZNORMALCOLORTEX4);
		break;
	case 10:
		size=sizeof(struct CUSTOMVERTEXBLUR);
		break;

	case 32:
		size=sizeof(struct BLENDXYZCOLORTEX);
		break;
	case 33:
		size=sizeof(struct BLENDXYZCOLORTEX);
		break;
	case 34:
		size=sizeof(struct BLENDXYZCOLOR);
		break;
	case 35:
		size=sizeof(struct BLENDXYZCOLORTEX);
		break;
	case 36:
		size=sizeof(struct BLENDXYZCOLORTEX2);
		break;
	case 37:
		size=sizeof(struct BLENDXYZNORMALTEX);
		break;
	case 38:
		size=sizeof(struct BLENDXYZCOLORTEX2);
		break;
	case 39:
		size=sizeof(struct BLENDXYZCOLORTEX3);
		break;
	case 40:
		size=sizeof(struct BLENDXYZNORMALTEX2);
		break;
	case 41:
		size=sizeof(struct BLENDXYZNORMALCOLORTEX4);
	};
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
	size=0;
#endif
	return size;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		int EDGEN_WFace(CObject3D *obj,int a,int b)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

inline int EDGEN_WFace(CObject3D *obj,int a,int b)
{
	int n=0;
	while (n<obj->nFaces)
	{
		int v0=obj->Faces[n].v0;
		int v1=obj->Faces[n].v1;		
		if ((v0==a)&&(v1==b)) return n;
		int v2=obj->Faces[n].v2;
		if ((v1==a)&&(v2==b)) return n;
		if ((v2==a)&&(v0==b)) return n;
		n++;
	}
	return -1;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

ShaderElement *CVertexBuffer::getName(char *str)
{
	ShaderElement *el=shadervariables.GetFirst();
	while (el)
	{
		if (strcmp(el->name,str)==0) return el;
		el=shadervariables.GetNext();
	}

	el=shadervariables.InsertLast();
	strcpy(el->name,str);
	return el;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexBuffer::SetAPI(void * api)
{
#ifdef API3D_DIRECT3D
	capi=api;
	D3DDevice=((C3DAPIBASE *)api)->D3DDevice;
#endif

#ifdef API3D_DIRECT3D9
	capi=api;
	D3DDevice=((C3DAPIBASE *)api)->D3DDevice;
#endif

#ifdef API3D_DIRECT3D10
	capi=api;
	D3DDevice=((C3DAPIBASE *)api)->device;
#endif

#ifdef API3D_DIRECT3D11
	capi=api;
	D3DDevice=((C3DAPIBASE *)api)->device;
	D3DDeviceContext=((C3DAPIBASE *)api)->devicecontext;
#endif

#ifdef API3D_DIRECT3D12
	capi=api;
	D3DDevice=((C3DAPIBASE *)api)->device;
	dhhc=((C3DAPIBASE *)api)->heap;
#endif

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D
bool VBDX_CreateVertexBuffer(int nv,LPDIRECT3DDEVICE8 D3DDevice,LPDIRECT3DVERTEXBUFFER8 *pVB,int blend,int temp,int hardware,int dyn)
{
	HRESULT h;
	DWORD tag=D3DUSAGE_WRITEONLY|(D3DUSAGE_DYNAMIC*dyn*hardware);

	if (hardware==0)
	{
		if (blend)
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLOR),tag ,D3DFVF_BLENDXYZCOLOR,D3DPOOL_MANAGED,pVB);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag ,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX),tag ,D3DFVF_BLENDXYZNORMALTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag ,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX3),tag ,D3DFVF_BLENDXYZCOLORTEX3,D3DPOOL_MANAGED,pVB);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX2),tag ,D3DFVF_BLENDXYZNORMALTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALCOLORTEX4),tag ,D3DFVF_BLENDXYZNORMALCOLORTEX4,D3DPOOL_MANAGED,pVB);
				break;
			};
		}
		else
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLOR),tag ,D3DFVF_XYZCOLOR,D3DPOOL_MANAGED,pVB);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag ,D3DFVF_XYZCOLORTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX),tag ,D3DFVF_XYZNORMALTEX,D3DPOOL_MANAGED,pVB);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag ,D3DFVF_XYZCOLORTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX3),tag ,D3DFVF_XYZCOLORTEX3,D3DPOOL_MANAGED,pVB);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX2),tag ,D3DFVF_XYZNORMALTEX2,D3DPOOL_MANAGED,pVB);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALCOLORTEX4),tag ,D3DFVF_XYZNORMALCOLORTEX4,D3DPOOL_MANAGED,pVB);
				break;
			};
		}
	}
	else
	{
		if (blend)
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLOR),tag ,D3DFVF_BLENDXYZCOLOR,D3DPOOL_DEFAULT,pVB);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag ,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag ,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX),tag ,D3DFVF_BLENDXYZNORMALTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag ,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX3),tag ,D3DFVF_BLENDXYZCOLORTEX3,D3DPOOL_DEFAULT,pVB);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX2),tag ,D3DFVF_BLENDXYZNORMALTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALCOLORTEX4),tag ,D3DFVF_BLENDXYZNORMALCOLORTEX4,D3DPOOL_DEFAULT,pVB);
				break;
			};
		}
		else
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLOR),tag ,D3DFVF_XYZCOLOR,D3DPOOL_DEFAULT,pVB);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag ,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag ,D3DFVF_XYZCOLORTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX),tag ,D3DFVF_XYZNORMALTEX,D3DPOOL_DEFAULT,pVB);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag ,D3DFVF_XYZCOLORTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX3),tag ,D3DFVF_XYZCOLORTEX3,D3DPOOL_DEFAULT,pVB);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX2),tag ,D3DFVF_XYZNORMALTEX2,D3DPOOL_DEFAULT,pVB);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALCOLORTEX4),tag ,D3DFVF_XYZNORMALCOLORTEX4,D3DPOOL_DEFAULT,pVB);
				break;
			};
		}
	}

	if (FAILED(h)) return false;
	else return true;

}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D9
bool VBDX_CreateVertexBuffer(int nv,LPDIRECT3DDEVICE9 D3DDevice,LPDIRECT3DVERTEXBUFFER9 *pVB,int blend,int temp,int hardware,int dyn)
{
	HRESULT h;
	DWORD tag=D3DUSAGE_WRITEONLY|(D3DUSAGE_DYNAMIC*dyn*hardware);

	if (hardware==0)
	{
		if (blend)
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLOR),tag,D3DFVF_BLENDXYZCOLOR,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX),tag,D3DFVF_BLENDXYZNORMALTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX3),tag,D3DFVF_BLENDXYZCOLORTEX3,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX2),tag,D3DFVF_BLENDXYZNORMALTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALCOLORTEX4),tag,D3DFVF_BLENDXYZNORMALCOLORTEX4,D3DPOOL_MANAGED,pVB,NULL);
				break;
			};
		}
		else
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 2:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLOR),tag,D3DFVF_XYZCOLOR,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 3:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 4:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag,D3DFVF_XYZCOLORTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 5:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX),tag,D3DFVF_XYZNORMALTEX,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 6:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag,D3DFVF_XYZCOLORTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 7:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX3),tag,D3DFVF_XYZCOLORTEX3,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 8:

				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX2),tag,D3DFVF_XYZNORMALTEX2,D3DPOOL_MANAGED,pVB,NULL);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALCOLORTEX4),tag,D3DFVF_XYZNORMALCOLORTEX4,D3DPOOL_MANAGED,pVB,NULL);
				break;
			};
		}
	}
	else
	{
		if (blend)
		{
			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLOR),tag,D3DFVF_BLENDXYZCOLOR,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX),tag,D3DFVF_BLENDXYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX),tag,D3DFVF_BLENDXYZNORMALTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX2),tag,D3DFVF_BLENDXYZCOLORTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZCOLORTEX3),tag,D3DFVF_BLENDXYZCOLORTEX3,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALTEX2),tag,D3DFVF_BLENDXYZNORMALTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct BLENDXYZNORMALCOLORTEX4),tag,D3DFVF_BLENDXYZNORMALCOLORTEX4,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			};
		}
		else
		{

			switch (temp)
			{
			case 0:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 1:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 2:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLOR),tag,D3DFVF_XYZCOLOR,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 3:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX),tag,D3DFVF_XYZCOLORTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 4:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag,D3DFVF_XYZCOLORTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 5:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX),tag,D3DFVF_XYZNORMALTEX,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 6:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX2),tag,D3DFVF_XYZCOLORTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 7:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZCOLORTEX3),tag,D3DFVF_XYZCOLORTEX3,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 8:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALTEX2),tag,D3DFVF_XYZNORMALTEX2,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			case 9:
				h=D3DDevice->CreateVertexBuffer(nv*sizeof(struct XYZNORMALCOLORTEX4),tag,D3DFVF_XYZNORMALCOLORTEX4,D3DPOOL_DEFAULT,pVB,NULL);
				break;
			};
		}
	}

	if (FAILED(h)) return false;
	else return true;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D10
bool VBDX_CreateVertexBuffer(int nv,ID3D10Device* D3DDevice,ID3D10Buffer **pVB,int blend,int temp,int hardware,int dyn)
{
	HRESULT h;
	D3D10_BUFFER_DESC vertexBufferDesc;

	ZeroMemory(&vertexBufferDesc,sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = SizeVB(blend*32+temp) * nv;
    vertexBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;

	h = D3DDevice->CreateBuffer(&vertexBufferDesc, NULL, pVB);

	if (FAILED(h)) return false;
	else return true;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D11
bool VBDX_CreateVertexBuffer(int nv,ID3D11Device* D3DDevice,ID3D11Buffer **pVB,int blend,int temp,int hardware,int dyn)
{
	HRESULT h;
	D3D11_BUFFER_DESC vertexBufferDesc;

	ZeroMemory(&vertexBufferDesc,sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = SizeVB(blend*32+temp) * nv;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;

	h = D3DDevice->CreateBuffer(&vertexBufferDesc, NULL, pVB);

	if (FAILED(h)) return false;
	else return true;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D12
bool VBDX_CreateVertexBuffer(int nv,ID3D12Device* D3DDevice,ID3D12Resource **pVB,int blend,int temp,int hardware,int dyn)
{
	D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(SizeVB(blend*32+temp) * nv),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(pVB));

	return true;
}

bool VBDX_CreateVertexBufferUpload(int nv,ID3D12Device* D3DDevice,ID3D12Resource **pVB,int blend,int temp,int hardware,int dyn)
{
	D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(SizeVB(blend*32+temp) * nv),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pVB));

	return true;
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_METAL)
extern void MTLCreateBuffer(CVertexBuffer *buf);

extern void MTLUpdateIndexBuffer(CVertexBuffer *buf);
extern void MTLUpdateVertexBuffer(CVertexBuffer *buf);
extern void MTLUpdateStreamVertexBuffer(CVertexBuffer *buf,int ns);
extern void MTLFreeBuffer(CVertexBuffer *buf);

int MTLSetProgramVertexBuffer(CVertexBuffer *buf);
void MTLFreeProgramVertexBuffer(CVertexBuffer *buf);

#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : INIT and "context" set

		nv : nb vertices
		ni : nb indices

	Note: Type must be specified before calling Init()

	old fonction compatibility

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::Init(int nv,int ni)
{
	Init(nv,ni,0);
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : INIT and "context" set

		nv : nb vertices
		ni : nb indices

	Note: Type must be specified before calling Init()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::Init(int nv,int ni,int ns)
{
	if (!capi) capi=TheClass3DAPI;

	init=true;

	dirty=0;
    
    partial=0;

	api3d_number_vertexbuffers++;

	if (Type&API3D_CONSTANT) dynamic=0;
	else dynamic=1;

	modifparams=false;

	stream1=stream2=0;
	stream_interpolant=0.0f;

#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int n;

	nstreams=ns;

	VertexBufferContainer.New(this);

	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	Color_Array=NULL;
	TexCoo_Array=NULL;
	Normal_Array=NULL;
	TexCoo2_Array=NULL;
	TexCoo3_Array=NULL;
	TexCoo4_Array=NULL;
	Vertex_Array=NULL;
	Indices_Array=NULL;
	Weights_Array=NULL;
	WeightsIndices_Array=NULL;

	if (ni>0) Index_Array=(unsigned int*) malloc(sizeof(unsigned int)*ni);

	TypeVB=0;
	Grouped=false;
    
    VB=-1;
    IB=-1;

	if ((Type&API3D_COLORDATAS)||(Type&API3D_COLORDATAS4)) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;
	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	if ((Type&API3D_COLORDATAS)||(Type&API3D_COLORDATAS4)) Color_Array=(float *) malloc(4*4*nv);

	if (Type&API3D_TEXCOODATAS) TexCoo_Array=(float *) malloc(4*2*nv);
	if (Type&API3D_NORMALDATAS)
	{
		Normal_Array=(float *) malloc(4*3*nv);
	}

	if (Type&API3D_VERTEXDATAS) Vertex_Array=(float *) malloc(4*3*nv);
	if (Type&API3D_TEXCOO2DATAS) TexCoo2_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_TEXCOO2LIGHTMAP) TexCoo2_Array=(float *) malloc(4*2*nv);
	if (Type&API3D_TEXCOO3BUMP) TexCoo3_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_TEXCOO4DATAS) TexCoo4_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_ENVMAPPING)
	{
		TexCoo2_Array=(float *) malloc(4*2*nv);
		Color_Array=(float *) malloc(4*4*nv);
	}

	if (Type&API3D_FOURTEX)
	{
		TypeVB=10;
		Vertex_Array=(float *) malloc(4*3*nv);
		Color_Array=(float *) malloc(4*4*nv);
		TexCoo_Array=(float *) malloc(4*2*nv);
		TexCoo2_Array=(float *) malloc(4*2*nv);
		TexCoo3_Array=(float *) malloc(4*2*nv);
		TexCoo4_Array=(float *) malloc(4*2*nv);
	}

	if (Type&API3D_MORPH)
	{
		streams=(float**) malloc(sizeof(float**)*ns);
		streams_norms=(float**) malloc(sizeof(float**)*ns);

		for (n=0;n<nstreams;n++)
		{
			streams[n]=(float*) malloc(4*3*nv);
			streams_norms[n]=(float *) malloc(4*3*nv);
            sVB[n]=-1;
		}
	}

	if (Type&API3D_BLENDING)
	{
		Indices_Array=(unsigned char*) malloc(4*nv);
		Weights_Array=(float*) malloc(sizeof(float)*4*nv);
		WeightsIndices_Array=(float*) malloc(sizeof(float)*4*nv);
	}

	MTLCreateBuffer(this);
    
	LockVertices();

	if (Type&API3D_BLENDING)
	{
		for (n=0;n<nv;n++)
		{
			InitMatrixIndex(n,0);
			InitMatrixIndex(n,1);
			InitMatrixIndex(n,2);
			InitMatrixIndex(n,3);

			InitWeights(n,0);
			InitWeights(n,1);
			InitWeights(n,2);
			InitWeights(n,3);
		}
	}

	if (Type&API3D_COLORDATAS)
		for (n=0;n<nVertices;n++) SetColor(n,1,1,1,1);

	UnlockVertices();

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n;

	nstreams=ns;

	VertexBufferContainer.New(this);

	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	Color_Array=NULL;
	TexCoo_Array=NULL;
	Normal_Array=NULL;
	TexCoo2_Array=NULL;
	TexCoo3_Array=NULL;
	TexCoo4_Array=NULL;
	Vertex_Array=NULL;
	Vertex2_Array=NULL;
	Indices_Array=NULL;
	Weights_Array=NULL;
#ifdef OPENGL_GLSL
	WeightsIndices_Array=NULL;
#endif

#if defined(GLES) || defined(GLES20)
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
    if (LIB3D_TYPE_INDEX==GL_UNSIGNED_INT)
    {
        Index_Array=(unsigned int*) malloc(sizeof(unsigned int)*ni);
        Index_Array16=NULL;
    }
    else
    {
        Index_Array=NULL;
        Index_Array16=(unsigned short int*) malloc(sizeof(unsigned short int)*ni);
    }
#else
	Index_Array=(unsigned short int*) malloc(sizeof(unsigned short int)*ni);
#endif
#else
	Index_Array=(GLuint*) malloc(sizeof(GLuint)*ni);
#endif

	BIndex_Array=Index_Array;
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
	BIndex_Array16=Index_Array16;
#endif

#if defined(GLES) || defined(GLES20) || defined(GLESFULL)
#ifdef GLESFULL
	if (Type&API3D_CONSTANT) vbo=true;
	else vbo=false;
#else
    vbo=false;
#endif
#else
	if (Type&API3D_CONSTANT) vbo=true;
	else vbo=false;
#endif

#ifdef API3D_VR
	// FORCE VBO FOR OPENVR
#ifndef OCULUS
	vbo=true;
#endif
#endif
    
#ifdef WEBASM
    vbo=true;
#endif

#ifdef RISCV
    vbo=true;
#endif

#ifdef RASPPI
    vbo=false;
#endif
    
	TypeVB=0;
	Grouped=false;

	if ((Type&API3D_COLORDATAS)||(Type&API3D_COLORDATAS4)) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;
	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	if ((Type&API3D_COLORDATAS)||(Type&API3D_COLORDATAS4)) Color_Array=(float *) malloc(4*4*nv);

	if (Type&API3D_TEXCOODATAS) TexCoo_Array=(float *) malloc(4*2*nv);
	if (Type&API3D_NORMALDATAS)
	{
		Normal_Array=(float *) malloc(4*3*nv);
#ifndef OPENGL20
		Normal_Array_stok=new CVector[nv];
#endif
	}

	if (Type&API3D_EDGESDG) Vertex2_Array=(float *) malloc(4*3*nv);
	if (Type&API3D_VERTEXDATAS) Vertex_Array=(float *) malloc(4*3*nv);
	if (Type&API3D_TEXCOO2DATAS) TexCoo2_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_TEXCOO2LIGHTMAP) TexCoo2_Array=(float *) malloc(4*2*nv);
	if (Type&API3D_TEXCOO3BUMP) TexCoo3_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_TEXCOO4DATAS) TexCoo4_Array=(float *) malloc(4*2*nv);

	if (Type&API3D_ENVMAPPING)
	{
		TexCoo2_Array=(float *) malloc(4*2*nv);
		Color_Array=(float *) malloc(4*4*nv);
	}

	if (Type&API3D_FOURTEX)
	{
		TypeVB=10;
		Vertex_Array=(float *) malloc(4*3*nv);
		Color_Array=(float *) malloc(4*4*nv);
		TexCoo_Array=(float *) malloc(4*2*nv);
		TexCoo2_Array=(float *) malloc(4*2*nv);
		TexCoo3_Array=(float *) malloc(4*2*nv);
		TexCoo4_Array=(float *) malloc(4*2*nv);
		vbo=false;
	}

	if (Type&API3D_MORPH)
	{
		streams=(float**) malloc(sizeof(float**)*ns);
		streams_norms=(float**) malloc(sizeof(float**)*ns);

		for (n=0;n<nstreams;n++)
		{
			streams[n]=(float*) malloc(4*3*nv);
			streams_norms[n]=(float *) malloc(4*3*nv);
		}

#ifndef API3D_OPENGL20
		temp_vertices=(float*) malloc(4*3*nv);
		temp_normals=(float *) malloc(4*3*nv);
#endif
	}

	if (Type&API3D_BLENDING)
	{
		Indices_Array=(unsigned char*) malloc(4*nv);
		Weights_Array=(float*) malloc(sizeof(float)*4*nv);
#ifdef OPENGL_GLSL
		WeightsIndices_Array=(float*) malloc(sizeof(float)*4*nv);
#endif
	}

	BColor_Array=Color_Array;
	BVertex_Array=Vertex_Array;
	BTexCoo_Array=TexCoo_Array;
	BTexCoo2_Array=TexCoo2_Array;
	BTexCoo3_Array=TexCoo3_Array;
	BTexCoo4_Array=TexCoo4_Array;
	BNormal_Array=Normal_Array;
	BIndices_Array=Indices_Array;
	BWeights_Array=Weights_Array;
	BWeightsIndices_Array=WeightsIndices_Array;

	if ((Type&API3D_MORPH)&&(vbo))
	{
		Bstreams=streams;
		Bstreams_norms=streams_norms;

		streams=(float**) malloc(sizeof(float**)*ns);
		streams_norms=(float**) malloc(sizeof(float**)*ns);
	}
	else
	{
		Bstreams=streams;
		Bstreams_norms=streams_norms;
	}

	if (vbo)
	{
		////////////////////////////////// VBO IBO

		Index_Array=NULL;
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
        Index_Array16=NULL;
#endif
		glGenBuffersARB(1,&IB);
		glGenBuffersARB(1,&VB);

		// IB
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,IB);
#if defined(GLES)||defined(GLES20)
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
        if (LIB3D_TYPE_INDEX==GL_UNSIGNED_INT)
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,nIndices*4,NULL,GL_STATIC_DRAW_ARB);
        else
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,nIndices*2,NULL,GL_STATIC_DRAW_ARB);
#else
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,nIndices*2,NULL,GL_STATIC_DRAW_ARB);
#endif
#else
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,nIndices*4,NULL,GL_STATIC_DRAW_ARB);
#endif
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);

		// VB
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,VB);

		if (Type&API3D_MORPH)
		{
			if (TypeVB==5)
			{
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*4+3*4+3*4+nstreams*(2*4*3)),NULL,GL_STATIC_DRAW);
				Vertex_Array=OFS(0);
				Normal_Array=OFS(3*4*nVertices);
				TexCoo_Array=OFS((3*4+3*4)*nVertices);
				unsigned int base=(2*4+3*4+3*4)*nVertices;
				for (n=0;n<nstreams;n++)
				{
					streams[n]=OFS(base+n*(2*4*3)*nVertices);
					streams_norms[n]=OFS(base+n*(2*4*3)*nVertices+3*4*nVertices);
				}
			}
			else vbo=false;
		}
		else
		{

			if (Type&API3D_BLENDING)
			{
				if (TypeVB==5)
				{
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*4+3*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					WeightsIndices_Array=OFS(3*4*nVertices);
					Normal_Array=OFS((4*4+3*4)*nVertices);
					TexCoo_Array=OFS((4*4+2*3*4)*nVertices);
				}
			}
			else
			{
				switch (TypeVB)
				{
				case 1:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					TexCoo_Array=OFS(3*4*nVertices);
					break;
				case 2:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS(3*4*nVertices);
					break;
				case 3:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS(3*4*nVertices);
					TexCoo_Array=OFS((4*4+3*4)*nVertices);
					break;
				case 4:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*2*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS(3*4*nVertices);
					TexCoo_Array=OFS((4*4+3*4)*nVertices);
					TexCoo2_Array=OFS((2*4+4*4+3*4)*nVertices);
					break;
				case 5:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*4+3*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Normal_Array=OFS(3*4*nVertices);
					TexCoo_Array=OFS((3*4+3*4)*nVertices);
					break;
				case 6:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*2*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS(3*4*nVertices);
					TexCoo_Array=OFS((4*4+3*4)*nVertices);
					TexCoo2_Array=OFS((2*4+4*4+3*4)*nVertices);
					break;
				case 7:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(3*2*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS((3*4)*nVertices);
					TexCoo_Array=OFS((4*4+3*4)*nVertices);
					TexCoo2_Array=OFS((2*4+4*4+3*4)*nVertices);
					TexCoo3_Array=OFS((2*2*4+4*4+3*4)*nVertices);
					break;
				case 8:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(2*2*4+4*4+3*4*2),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);										
					Normal_Array=OFS((3*4)*nVertices);
					Color_Array=OFS((3*4*2)*nVertices);
					TexCoo_Array=OFS((4*4+3*4*2)*nVertices);
					TexCoo2_Array=OFS((2*4+4*4+3*4*2)*nVertices);
					break;
				case 9:
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,nVertices*(4*2*4+3*4+4*4+3*4),NULL,GL_STATIC_DRAW);
					Vertex_Array=OFS(0);
					Color_Array=OFS(3*4*nVertices);
					Normal_Array=OFS((4*4+3*4)*nVertices);
					TexCoo_Array=OFS((4*4+3*4+3*4)*nVertices);
					TexCoo2_Array=OFS((2*4+4*4+3*4+3*4)*nVertices);
					TexCoo3_Array=OFS((2*4+2*4+4*4+3*4+3*4)*nVertices);
					TexCoo4_Array=OFS((2*4+2*4+2*4+4*4+3*4+3*4)*nVertices);
					break;
				};
			}
		}
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}

	if (Type&API3D_BLENDING)
	{
		LockVertices();
		for (n=0;n<nv;n++)
		{
			InitMatrixIndex(n,0);
			InitMatrixIndex(n,1);
			InitMatrixIndex(n,2);
			InitMatrixIndex(n,3);

			InitWeights(n,0);
			InitWeights(n,1);
			InitWeights(n,2);
			InitWeights(n,3);
		}
		UnlockVertices();
	}
	else
	if (Type&API3D_COLORDATAS)
	{
		LockVertices();
		for (n=0;n<nVertices;n++) SetColor(n,1,1,1,1);
		UnlockVertices();
	}

#endif
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int n;
	int temp;
	HRESULT h;
	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	nSTREAMS=ns;

	pVertices=NULL;
	pIndices=NULL;

	TypeVB=0;
	Grouped=false;


	if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

	if (Type&API3D_COLORDATAS) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;
	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	temp=TypeVB;

	if (Type&API3D_BLENDING) TypeVB+=32;

	_sizeVB=SizeVB(TypeVB);

	if (!HardwareVertexProcessing)
	{

		VertexBufferContainer.New(this);

		if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D8VB,1,temp,0,dynamic);
		else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D8VB,0,temp,0,dynamic);

		if (Type&API3D_MORPH)
		{
			STREAMS=(LPDIRECT3DVERTEXBUFFER8*) malloc(sizeof(LPDIRECT3DVERTEXBUFFER8)*ns);
			for (n=0;n<ns;n++)
			{
				if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,0,dynamic);
				else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,0,dynamic);
			}
		}

		h=D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_MANAGED,&D3D8INDEX);

		Vertices=Indices=NULL;
	}
	else
	{
		Vertices=(BYTE*) malloc(_sizeVB*nv);
		Indices=(BYTE*) malloc(sizeof(DWORD)*ni);

		VertexBufferContainer.New(this);

		if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D8VB,1,temp,1,dynamic);
		else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D8VB,0,temp,1,dynamic);

		if (Type&API3D_MORPH)
		{
			STREAMS=(LPDIRECT3DVERTEXBUFFER8*) malloc(sizeof(LPDIRECT3DVERTEXBUFFER8)*ns);
			for (n=0;n<ns;n++)
			{
				if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,1,dynamic);
				else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,1,dynamic);
			}
		}

		h=D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_DEFAULT,&D3D8INDEX);
	}

	_temp=TypeVB&0xF;
#ifdef API3D_DIRECT3D10
	if (TypeVB&32) _blend=4*4;
	else _blend=0;
#else
	if (TypeVB&32) _blend=5*4;
	else _blend=0;
#endif
	_blendN=_blend;
	if (_temp==9) _blend+=3*4;

	LockVertices();

	if (Type&API3D_BLENDING)
	{
		for (n=0;n<nv;n++)
		{
			SetMatrixIndex(n,0,0);
			SetMatrixIndex(n,1,0);
			SetMatrixIndex(n,2,0);
			SetMatrixIndex(n,3,0);

			SetWeights(n,0,0.0f);
			SetWeights(n,1,0.0f);
			SetWeights(n,2,0.0f);
			SetWeights(n,3,0.0f);
		}
	}

	if (TypeVB!=5)
	for (n=0;n<nv;n++) SetColor(n,1,1,1);
	UnlockVertices();
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D9 ----------
	int n;
	int temp;
	HRESULT h;
	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	pVertices=NULL;
	pIndices=NULL;

	TypeVB=0;
	Grouped=false;

	nSTREAMS=ns;

	if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

	if (Type&API3D_COLORDATAS) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;

	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	temp=TypeVB;

	if (Type&API3D_BLENDING) TypeVB+=32;

	_sizeVB=SizeVB(TypeVB);

	if (!HardwareVertexProcessing)
	{
		VertexBufferContainer.New(this);

		if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D9VB,1,temp,0,dynamic);
		else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D9VB,0,temp,0,dynamic);

		if (Type&API3D_MORPH)
		{
			STREAMS=(LPDIRECT3DVERTEXBUFFER9*) malloc(sizeof(LPDIRECT3DVERTEXBUFFER9)*ns);
			for (n=0;n<ns;n++)
			{
				if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,0,dynamic);
				else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,0,dynamic);
			}
		}

		h=D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_MANAGED,&D3D9INDEX,NULL);

		Vertices=Indices=NULL;
	}
	else
	{

		Vertices=(BYTE*) malloc(_sizeVB*nv);
		Indices=(BYTE*) malloc(sizeof(DWORD)*ni);

		VertexBufferContainer.New(this);

		if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D9VB,1,temp,1,dynamic);
		else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3D9VB,0,temp,1,dynamic);

		if (Type&API3D_MORPH)
		{
			VerticesStream=(BYTE**) malloc(sizeof(BYTE*)*ns);

			STREAMS=(LPDIRECT3DVERTEXBUFFER9*) malloc(sizeof(LPDIRECT3DVERTEXBUFFER9)*ns);
			for (n=0;n<ns;n++)
			{
				VerticesStream[n]=(BYTE*) malloc(_sizeVB*nv);
				if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,1,dynamic);
				else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,1,dynamic);
			}
		}

		h=D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_DEFAULT,&D3D9INDEX,NULL);
	}

	_temp=TypeVB&0xF;
#ifdef API3D_DIRECT3D10
	if (TypeVB&32) _blend=4*4;
	else _blend=0;
#else
	if (TypeVB&32) _blend=5*4;
	else _blend=0;
#endif
	_blendN=_blend;
	if (_temp==9) _blend+=3*4;

	if ((Type&API3D_FOURTEX)==0)
	{
		if (Type&API3D_BLENDING)
		{
			LockVertices();

			for (n=0;n<nv;n++)
			{
				InitMatrixIndex(n,0);
				InitMatrixIndex(n,1);
				InitMatrixIndex(n,2);
				InitMatrixIndex(n,3);

				InitWeights(n,0);
				InitWeights(n,1);
				InitWeights(n,2);
				InitWeights(n,3);
			}

			UnlockVertices();
		}
		else
		if ((TypeVB!=5)&&(TypeVB<32))
		{
			LockVertices();
			for (n=0;n<nv;n++) SetColor(n,1,1,1);
			UnlockVertices();
		}
	}

#endif


#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D9 ----------
	int temp,n;
	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	pVertices=NULL;
	pIndices=NULL;

	TypeVB=0;
	Grouped=false;

	nSTREAMS=ns;

	if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

	if (Type&API3D_COLORDATAS) TypeVB+=2;
	else if (Type&API3D_COLORDATAS4) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;

	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_FOURTEX) TypeVB=10;

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	temp=TypeVB;

	if (Type&API3D_BLENDING) TypeVB+=32;

	_sizeVB=SizeVB(TypeVB);


	Vertices=(BYTE*) malloc(_sizeVB*nv);
	Indices=(BYTE*) malloc(sizeof(DWORD)*ni);

	VertexBufferContainer.New(this);

	if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,1,temp,1,dynamic);
	else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,0,temp,1,dynamic);

	if (Type&API3D_MORPH)
	{
		VerticesStream=(BYTE**) malloc(sizeof(BYTE*)*ns);

		STREAMS=(ID3D10Buffer **) malloc(sizeof(ID3D10Buffer *)*ns);
		for (n=0;n<ns;n++)
		{
			VerticesStream[n]=(BYTE*) malloc(_sizeVB*nv);
			if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,1,dynamic);
			else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,1,dynamic);
		}
	}

	D3D10_BUFFER_DESC indexBufferDesc;

	ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D10_USAGE_DYNAMIC; 
    indexBufferDesc.ByteWidth = sizeof(unsigned int) * ni;
    indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    indexBufferDesc.MiscFlags = 0;

	D3DDevice->CreateBuffer(&indexBufferDesc, NULL, &D3DINDEX);

	_temp=TypeVB&0xF;
#ifdef API3D_DIRECT3D10
	if (TypeVB&32) _blend=4*4;
	else _blend=0;
#else
	if (TypeVB&32) _blend=5*4;
	else _blend=0;
#endif
	_blendN=_blend;
	if (_temp==9) _blend+=3*4;

	if ((Type&API3D_FOURTEX)==0)
	{
		if (Type&API3D_BLENDING)
		{
			LockVertices();

			for (n=0;n<nv;n++)
			{
				InitMatrixIndex(n,0);
				InitMatrixIndex(n,1);
				InitMatrixIndex(n,2);
				InitMatrixIndex(n,3);

				InitWeights(n,0);
				InitWeights(n,1);
				InitWeights(n,2);
				InitWeights(n,3);
			}

			UnlockVertices();
		}
		else
		if ((TypeVB!=5)&&(TypeVB<32))
		{
			LockVertices();
			for (n=0;n<nv;n++) SetColor(n,1,1,1);
			UnlockVertices();
		}
	}

#endif


#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D9 ----------
	int temp,n;
	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	pVertices=NULL;
	pIndices=NULL;

	TypeVB=0;
	Grouped=false;

	nSTREAMS=ns;

	if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;
	if (D3DDeviceContext==NULL) D3DDeviceContext=D3DDeviceContextPrincipal;

	if (Type&API3D_COLORDATAS) TypeVB+=2;
	else if (Type&API3D_COLORDATAS4) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;

	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_FOURTEX) TypeVB=10;

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	temp=TypeVB;

	if (Type&API3D_BLENDING) TypeVB+=32;

	_sizeVB=SizeVB(TypeVB);


	Vertices=(BYTE*) malloc(_sizeVB*nv);
	Indices=(BYTE*) malloc(sizeof(DWORD)*ni);

	VertexBufferContainer.New(this);

	if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,1,temp,1,dynamic);
	else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,0,temp,1,dynamic);

	if (Type&API3D_MORPH)
	{
		VerticesStream=(BYTE**) malloc(sizeof(BYTE*)*ns);

		STREAMS=(ID3D11Buffer **) malloc(sizeof(ID3D11Buffer *)*ns);
		for (n=0;n<ns;n++)
		{
			VerticesStream[n]=(BYTE*) malloc(_sizeVB*nv);
			if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,1,dynamic);
			else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,1,dynamic);
		}
	}

	D3D11_BUFFER_DESC indexBufferDesc;

	ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; 
    indexBufferDesc.ByteWidth = sizeof(unsigned int) * ni;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    indexBufferDesc.MiscFlags = 0;

	D3DDevice->CreateBuffer(&indexBufferDesc, NULL, &D3DINDEX);

	_temp=TypeVB&0xF;
	if (TypeVB&32) _blend=4*4;
	else _blend=0;

	_blendN=_blend;
	if (_temp==9) _blend+=3*4;

	if ((Type&API3D_FOURTEX)==0)
	{
		if (Type&API3D_BLENDING)
		{
			LockVertices();

			for (n=0;n<nv;n++)
			{
				InitMatrixIndex(n,0);
				InitMatrixIndex(n,1);
				InitMatrixIndex(n,2);
				InitMatrixIndex(n,3);

				InitWeights(n,0);
				InitWeights(n,1);
				InitWeights(n,2);
				InitWeights(n,3);
			}

			UnlockVertices();
		}
		else
		if ((TypeVB!=5)&&(TypeVB<32))
		{
			LockVertices();
			for (n=0;n<nv;n++) SetColor(n,1,1,1);
			UnlockVertices();
		}
	}
#endif	

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D9 ----------
	int temp,n;
	nVertices=nv;
	nIndices=ni;
	nVerticesActif=nv;
	nIndicesActif=ni;

	pVertices=NULL;
	pIndices=NULL;

	TypeVB=0;
	Grouped=false;

	nSTREAMS=ns;

	dhhc=((C3DAPIBASE*)capi)->heap;

	if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

	if (Type&API3D_COLORDATAS) TypeVB+=2;
	else if (Type&API3D_COLORDATAS4) TypeVB+=2;
	if (Type&API3D_TEXCOODATAS) TypeVB+=1;
	if (Type&API3D_TEXCOO2DATAS) TypeVB=4;
	if (Type&API3D_NORMALDATAS) TypeVB=5;
	if (Type&API3D_TEXCOO2LIGHTMAP) TypeVB=6;

	if (Type&API3D_TEXCOO3BUMP)
	{
		if ((Type&API3D_NORMALDATAS)&&(Type&API3D_TEXCOO4DATAS)) TypeVB=9;
		else TypeVB=7;
	}

	if (Type&API3D_FOURTEX) TypeVB=10;

	if (Type&API3D_COLORNORMALTEX2) TypeVB=8;

	temp=TypeVB;

	if (Type&API3D_BLENDING) TypeVB+=32;

	_sizeVB=SizeVB(TypeVB);


	Vertices=(BYTE*) malloc(_sizeVB*nv);
	Indices=(BYTE*) malloc(sizeof(DWORD)*ni);

	VertexBufferContainer.New(this);

	if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,1,temp,1,dynamic);
	else VBDX_CreateVertexBuffer(nv,D3DDevice,&D3DVB,0,temp,1,dynamic);

	D3DVBTMP=NULL;
	STREAMSTMP=NULL;

	if (Type&API3D_MORPH)
	{
		VerticesStream=(BYTE**) malloc(sizeof(BYTE*)*ns);

		STREAMS=(ID3D12Resource **) malloc(sizeof(ID3D12Resource *)*ns);
		
		STREAMSVIEW=(D3D12_VERTEX_BUFFER_VIEW*) malloc(sizeof(D3D12_VERTEX_BUFFER_VIEW)*ns);
		for (n=0;n<ns;n++)
		{
			VerticesStream[n]=(BYTE*) malloc(_sizeVB*nv);
			if (Type&API3D_BLENDING) VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],1,temp,1,dynamic);
			else VBDX_CreateVertexBuffer(nv,D3DDevice,&STREAMS[n],0,temp,1,dynamic);
		}
	}

	D3DINDEXTMP=NULL;

	D3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int) * ni),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&D3DINDEX));


	_temp=TypeVB&0xF;
	if (TypeVB&32) _blend=4*4;
	else _blend=0;

	_blendN=_blend;
	if (_temp==9) _blend+=3*4;

	if ((Type&API3D_FOURTEX)==0)
	{
		if (Type&API3D_BLENDING)
		{
			LockVertices();

			for (n=0;n<nv;n++)
			{
				InitMatrixIndex(n,0);
				InitMatrixIndex(n,1);
				InitMatrixIndex(n,2);
				InitMatrixIndex(n,3);

				InitWeights(n,0);
				InitWeights(n,1);
				InitWeights(n,2);
				InitWeights(n,3);
			}

			UnlockVertices();
		}
		else
		if ((TypeVB!=5)&&(TypeVB<32))
		{
			LockVertices();
			for (n=0;n<nv;n++) SetColor(n,1,1,1);
			UnlockVertices();
		}
	}
#endif	

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : lock vertices

		to be called before modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::LockVertices()
{
	stream_locked=-1;
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D8VB->Lock(0,nVertices*_sizeVB,(BYTE**)&pVertices,0);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((HardwareVertexProcessing)&&(dynamic))
	{
		D3D9VB->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD|D3DLOCK_NO_DIRTY_UPDATE);
		memcopy(pVertices,Vertices,_sizeVB*nVertices);
	}
	else
	{
		D3D9VB->Lock(0,0,(void**)&pVertices,0);
	}
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DVB->Map(D3D10_MAP_WRITE_DISCARD, 0,(void**)&pVertices);
	memcopy(pVertices,Vertices,_sizeVB*nVertices);
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D11_MAPPED_SUBRESOURCE mapped;
	D3DDeviceContext->Map(D3DVB,0,D3D11_MAP_WRITE_DISCARD,0,&mapped);
	pVertices=(BYTE*)mapped.pData;
	memcopy(pVertices,Vertices,_sizeVB*nVertices);
#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((Type&API3D_DYNAMIC))
	{
		pVertices=Vertices;
	}
	else
	if ((Type&API3D_CONSTANT)==0)
	{
		if (D3DVBTMP) ((C3DAPIBASE*)capi)->DynamicResources[((C3DAPIBASE*)capi)->frameIndex].Add(D3DVBTMP);
		VBDX_CreateVertexBufferUpload(nVertices,D3DDevice,&D3DVBTMP,0,TypeVB,1,dynamic);

		CD3DX12_RANGE readRange(0, 0);
		D3DVBTMP->Map(0, &readRange, reinterpret_cast<void**>(&pVertices));
		memcopy(pVertices,Vertices,_sizeVB*nVertices);
	}
	else
	{
		VBDX_CreateVertexBufferUpload(nVertices,D3DDevice,&D3DVBTMP,0,TypeVB,1,dynamic);

		CD3DX12_RANGE readRange(0, 0);
		D3DVBTMP->Map(0, &readRange, reinterpret_cast<void**>(&pVertices));
		memcopy(pVertices,Vertices,_sizeVB*nVertices);
	}
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : unlock vertices

		to be called after modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */



void CVertexBuffer::UnlockVertices()
{
    
#ifdef API3D_METAL
    MTLUpdateVertexBuffer(this);
#endif
    
#if defined(API3D_OPENGL20)||defined(API3D_OPENGL)
	if (vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,VB);

		if (Type&API3D_MORPH)
		{
			if (TypeVB==5)
			{
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
				/*
				for (n=0;n<nstreams;n++)
				{
					streams[n]=OFS(base+n*(2*4*3)*nVertices);
					streams_norms[n]=OFS(base+n*(2*4*3)*nVertices+3*4*nVertices);
				}
				/**/
			}
			else vbo=false;
		}
		else
		{

			if (Type&API3D_BLENDING)
			{
				if (TypeVB==5)
				{
                    if (partial)    // OPEN GL ES LACK OF PALETTE
                    {
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
                    }
                    else
                    {
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)WeightsIndices_Array,nVertices*4*4,BWeightsIndices_Array);
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
                        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
                    }
				}
			}
			else
			{
				switch (TypeVB)
				{
				case 1:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					break;
				case 2:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					break;
				case 3:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					break;
				case 4:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo2_Array,nVertices*2*4,BTexCoo2_Array);
					break;
				case 5:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					break;
				case 6:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo2_Array,nVertices*2*4,BTexCoo2_Array);
					break;
				case 7:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo2_Array,nVertices*2*4,BTexCoo2_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo3_Array,nVertices*2*4,BTexCoo3_Array);
					break;
                case 8:
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo2_Array,nVertices*2*4,BTexCoo2_Array);
                    break;
				case 9:
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Vertex_Array,nVertices*3*4,BVertex_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Color_Array,nVertices*4*4,BColor_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)Normal_Array,nVertices*3*4,BNormal_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo_Array,nVertices*2*4,BTexCoo_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo2_Array,nVertices*2*4,BTexCoo2_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo3_Array,nVertices*2*4,BTexCoo3_Array);
					glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)TexCoo4_Array,nVertices*2*4,BTexCoo4_Array);
					break;
				};
			}
		}
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}
#endif


#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D8VB->Unlock();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D9VB->Unlock();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DVB->Unmap();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DDeviceContext->Unmap(D3DVB,0);
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	
	if ((Type&API3D_DYNAMIC))
	{
		C3DAPIBASE *api=((C3DAPIBASE*)capi);

		int size = D3DVBVIEW.SizeInBytes = nVerticesActif*_sizeVB;
		int shr=(size+255)/256;
		int frame=api->frameIndex;

		ID3D12Resource * res;
		DescriptorHeapHandle id;
		UINT8* pdata;

		ID3D12Resource **pcst=(ID3D12Resource **)api->ConstantBuffers[frame][shr].Current;
		DescriptorHeapHandle *pid=(DescriptorHeapHandle *)api->ConstantBuffersId[frame][shr].Current;
		UINT8 **pptr=(UINT8 **)api->ConstantBuffersPtr[frame][shr].Current;
		if (pcst)
		{
			res=*pcst;		
			id=*pid;
			pdata=*pptr;
			api->ConstantBuffers[frame][shr].GetNext();
			api->ConstantBuffersId[frame][shr].GetNext();
			api->ConstantBuffersPtr[frame][shr].GetNext();
		}
		else
		{
			api->device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),D3D12_HEAP_FLAG_NONE,&CD3DX12_RESOURCE_DESC::Buffer(shr*256),D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr,IID_PPV_ARGS(&res));		
			api->ConstantBuffers[frame][shr].Add(res);

			id=api->heap->GetNewHeapHandle();
			api->ConstantBuffersId[frame][shr].Add(id);

			CD3DX12_RANGE readRange(0, 0);
			res->Map(0, &readRange, reinterpret_cast<void**>(&pdata));

			api->ConstantBuffersPtr[frame][shr].Add(pdata);
		}

		D3DVBVIEW.BufferLocation = res->GetGPUVirtualAddress();		
		//D3DVBVIEW.BufferLocation = D3DVBTMP->GetGPUVirtualAddress();
		D3DVBVIEW.StrideInBytes=_sizeVB;

		memcpy(pdata, pVertices, size);

		/*
		int size = D3DVBVIEW.SizeInBytes = nVerticesActif*_sizeVB;
		int shr=(size+255)/256;

		if (api->ofsConstantBuffers[api->frameIndex]+256*shr>HEAP_DYNAMIC_BUFFERS_SIZE)
		{
			((C3DAPIBASE*)capi)->CloseOpenedCommandListFence();
			api->ofsConstantBuffers[api->frameIndex]=0;
		}

		ID3D12Resource * res=api->heapConstantBuffers[api->frameIndex];
		D3DVBVIEW.BufferLocation = res->GetGPUVirtualAddress()+api->ofsConstantBuffers[api->frameIndex];
		
		//D3DVBVIEW.BufferLocation = D3DVBTMP->GetGPUVirtualAddress();
		D3DVBVIEW.StrideInBytes=_sizeVB;

		UINT8*pdata=api->pdataConstantBuffer[api->frameIndex]+api->ofsConstantBuffers[api->frameIndex];
		memcpy(pdata, pVertices, size);
		api->ofsConstantBuffers[api->frameIndex]+=256*shr;
		/**/
	}
	else
	if ((Type&API3D_CONSTANT)==0)
	{
		D3DVBTMP->Unmap(0, nullptr);
		D3DVBVIEW.BufferLocation = D3DVBTMP->GetGPUVirtualAddress();
		D3DVBVIEW.StrideInBytes=_sizeVB;
		D3DVBVIEW.SizeInBytes = nVerticesActif*_sizeVB;
	}
	else
	{
		D3DVBTMP->Unmap(0, nullptr);

		ID3D12GraphicsCommandList* cmd;

		((C3DAPIBASE*)capi)->CloseOpenedCommandList();

		dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ((C3DAPIBASE*)capi)->commandAllocator, nullptr, IID_PPV_ARGS(&cmd));

		D3D12_RESOURCE_BARRIER barrierDesc = {};

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = D3DVB;
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmd->ResourceBarrier(1, &barrierDesc);
		cmd->CopyResource(D3DVB,D3DVBTMP);

		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		cmd->ResourceBarrier(1, &barrierDesc); 

		cmd->Close();

		ID3D12CommandList* list[] = { cmd };
		((C3DAPIBASE*)capi)->commandQueue->ExecuteCommandLists(_countof(list), list);
		cmd->Release();

		D3DVBVIEW.BufferLocation = D3DVB->GetGPUVirtualAddress();
		D3DVBVIEW.StrideInBytes=_sizeVB;
		D3DVBVIEW.SizeInBytes = nVerticesActif*_sizeVB;

		((C3DAPIBASE*)capi)->DynamicResources[(((C3DAPIBASE*)capi)->frameIndex+1)&1].Add(D3DVBTMP);
		D3DVBTMP=NULL;
	}

	pVertices=NULL;
#endif
    
	stream_locked=-1;
    partial=0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : lock stream vertices

		to be called before modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::LockStream(int n)
{
    
#ifdef API3D_METAL
    stream_locked=n;
#endif
  
    
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
	stream_locked=n;
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (STREAMS)
	{
		STREAMS[n]->Lock(0,nVertices*_sizeVB,(BYTE**)&pVertices,0);
		stream_locked=n;
	}
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (STREAMS)
	{
		if ((HardwareVertexProcessing)&&(dynamic)) STREAMS[n]->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD|D3DLOCK_NO_DIRTY_UPDATE);
		else STREAMS[n]->Lock(0,0,(void**)&pVertices,0);
		stream_locked=n;
	}
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (STREAMS)
	{
		STREAMS[n]->Map(D3D10_MAP_WRITE_DISCARD, 0,(void**)&pVertices);
		stream_locked=n;
	}
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (STREAMS)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		D3DDeviceContext->Map(STREAMS[n],0,D3D11_MAP_WRITE_DISCARD,0,&mapped);
		pVertices=(BYTE*)mapped.pData;
		stream_locked=n;
	}
#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (STREAMS)
	{
		VBDX_CreateVertexBufferUpload(nVertices,D3DDevice,&STREAMSTMP,0,TypeVB,1,dynamic);

		CD3DX12_RANGE readRange(0, 0);
		STREAMSTMP->Map(0, &readRange, reinterpret_cast<void**>(&pVertices));
		memcopy(pVertices,Vertices,_sizeVB*nVertices);
		stream_locked=n;
	}
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : unlock stream

		to be called after modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */



void CVertexBuffer::UnlockStream()
{
#ifdef API3D_METAL
    MTLUpdateStreamVertexBuffer(this,stream_locked);
#endif

#if defined(API3D_OPENGL20)||defined(API3D_OPENGL)
	if ((vbo)&&(stream_locked>=0))
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,VB);
		if (Type&API3D_MORPH)
		{
			if (TypeVB==5)
			{
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)streams[stream_locked],nVertices*3*4,Bstreams[stream_locked]);
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(GLintptrARB)streams_norms[stream_locked],nVertices*3*4,Bstreams_norms[stream_locked]);
			}
		}
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (stream_locked>=0) STREAMS[stream_locked]->Unlock();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (stream_locked>=0) STREAMS[stream_locked]->Unlock();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (stream_locked>=0) STREAMS[stream_locked]->Unmap();
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (stream_locked>=0) D3DDeviceContext->Unmap(STREAMS[stream_locked],0);
	pVertices=NULL;
#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (stream_locked>=0)
	{
		STREAMSTMP->Unmap(0, nullptr);

		((C3DAPIBASE*)capi)->CloseOpenedCommandList();

		ID3D12GraphicsCommandList* cmd;

		dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ((C3DAPIBASE*)capi)->commandAllocator, nullptr, IID_PPV_ARGS(&cmd));

		D3D12_RESOURCE_BARRIER barrierDesc = {};

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = STREAMS[stream_locked];
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmd->ResourceBarrier(1, &barrierDesc);
		cmd->CopyResource(STREAMS[stream_locked],STREAMSTMP);

		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		cmd->ResourceBarrier(1, &barrierDesc); 

		cmd->Close();

		ID3D12CommandList* list[] = { cmd };
		((C3DAPIBASE*)capi)->commandQueue->ExecuteCommandLists(_countof(list), list);
		cmd->Release();


		STREAMSVIEW[stream_locked].BufferLocation = STREAMS[stream_locked]->GetGPUVirtualAddress();
		STREAMSVIEW[stream_locked].StrideInBytes=_sizeVB;
		STREAMSVIEW[stream_locked].SizeInBytes = nVerticesActif*_sizeVB;

		((C3DAPIBASE*)capi)->DynamicResources[(((C3DAPIBASE*)capi)->frameIndex+1)&1].Add(STREAMSTMP);
		STREAMSTMP=NULL;
	}
	pVertices=NULL;
#endif

	stream_locked=-1;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : lock indices

		to be called before modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::LockIndices()
{
#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D8INDEX->Lock(0,nIndices*4,(BYTE**)&pIndices,0);
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D9INDEX->Lock(0,0,(void**)&pIndices,0);
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DINDEX->Map(D3D10_MAP_WRITE_DISCARD, 0,(void**)&pIndices);
	memcpy(pIndices,Indices,4*nIndices);
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D11_MAPPED_SUBRESOURCE mapped;
	D3DDeviceContext->Map(D3DINDEX,0,D3D11_MAP_WRITE_DISCARD,0,&mapped);
	pIndices=(BYTE*)mapped.pData;
	memcpy(pIndices,Indices,4*nIndices);
#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	D3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int) * nIndices),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&D3DINDEXTMP));

	CD3DX12_RANGE readRangei(0, 0);
	D3DINDEXTMP->Map(0, &readRangei, reinterpret_cast<void**>(&pIndices));
	memcpy(pIndices,Indices,4*nIndices);
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex buffer : unlock indices

		to be called after modifying vertex buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::UnlockIndices()
{
    
#ifdef API3D_METAL
    MTLUpdateIndexBuffer(this);
#endif
    
#if defined(API3D_OPENGL)||defined(API3D_OPENGL20)
	if (vbo)
	{
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,IB);
#if defined(GLES)||defined(GLES20)
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
        if (LIB3D_TYPE_INDEX==GL_UNSIGNED_INT)
            glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0,nIndices*4,BIndex_Array);
        else
            glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0,nIndices*2,BIndex_Array16);
#else
		glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0,nIndices*2,BIndex_Array);
#endif
#else
		glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0,nIndices*4,BIndex_Array);
#endif
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D8INDEX->Unlock();
	pIndices=NULL;
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3D9INDEX->Unlock();
	pIndices=NULL;
#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DINDEX->Unmap();
	pIndices=NULL;
#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DDeviceContext->Unmap(D3DINDEX,0);
	pIndices=NULL;
#endif
#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	D3DINDEXTMP->Unmap(0,nullptr);
	pIndices=NULL;

	ID3D12GraphicsCommandList* cmd;
	((C3DAPIBASE*)capi)->CloseOpenedCommandList();

	dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ((C3DAPIBASE*)capi)->commandAllocator, nullptr, IID_PPV_ARGS(&cmd));

	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = D3DINDEX;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	cmd->ResourceBarrier(1, &barrierDesc);
	cmd->CopyResource(D3DINDEX,D3DINDEXTMP);

	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	cmd->ResourceBarrier(1, &barrierDesc); 

	cmd->Close();

	ID3D12CommandList* list[] = { cmd };
	((C3DAPIBASE*)capi)->commandQueue->ExecuteCommandLists(_countof(list), list);
	cmd->Release();

	D3DINDEXVIEW.BufferLocation = D3DINDEX->GetGPUVirtualAddress();
	D3DINDEXVIEW.Format=DXGI_FORMAT_R32_UINT;
	D3DINDEXVIEW.SizeInBytes = nIndicesActif*sizeof(int);

	((C3DAPIBASE*)capi)->DynamicResources[(((C3DAPIBASE*)capi)->frameIndex+1)&1].Add(D3DINDEXTMP);
	D3DINDEXTMP=NULL;
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions:

		Modifying vertex buffer data.

		void SetColor(int n,float r,float g,float b,float a);			// diffuse
		void SetColor(int n,float r,float g,float b);					// diffuse
		void SetVertex(int n,float x,float y,float z);					// coo.
		void SetTexCoo(int n,float x,float y);							// mapping coo.
		void SetTexCoo2(int n,float x,float y);							// mapping coo.
		void SetTexCoo3(int n,float x,float y);							// mapping coo.
		void SetNormal(int n,float x,float y,float z);					// normal vector
		void SetVertex(int n,CVector v);								// coo.
		void SetNormal(int n,CVector v);								// normal vector
		void SetTexCoo(int n,CVector2 map);								// mapping coo.
		void SetTexCoo2(int n,CVector2 map);							// mapping coo.
		void SetTexCoo3(int n,CVector2 map);							// mapping coo.
		void SetWeights(int n,int w,float value);						// up to 4 weight floats
		void SetMatrixIndex(int n,int w,int index);						// 4 matrix index

		void SetIndices(int n,int n0,int n1,int n2);					// triangle indices



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : set indices

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetIndices(int n,int n0,int n1,int n2)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1) +n;
	Index_Array[adr+0]=n0;
	Index_Array[adr+1]=n1;
	Index_Array[adr+2]=n2;
    
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1) +n;
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
    if (LIB3D_TYPE_INDEX==GL_UNSIGNED_INT)
    {
        BIndex_Array[adr+0]=n0;
        BIndex_Array[adr+1]=n1;
        BIndex_Array[adr+2]=n2;
    }
    else
    {
        BIndex_Array16[adr+0]=n0;
        BIndex_Array16[adr+1]=n1;
        BIndex_Array16[adr+2]=n2;
    }
#else
	BIndex_Array[adr+0]=n0;
	BIndex_Array[adr+1]=n1;
	BIndex_Array[adr+2]=n2;
#endif
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD i[3];

	i[0]=n0;
	i[1]=n1;
	i[2]=n2;

	memcopy12(pIndices+n*3*4,i)
	if (Indices) memcopy12(Indices+n*3*4,i)
#endif
}

void FNCALLCONVVB CVertexBuffer::SetIndice(int n,int n0)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
    Index_Array[n]=n0;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
    if (LIB3D_TYPE_INDEX==GL_UNSIGNED_INT)
    {
        BIndex_Array[n]=n0;
    }
    else
    {
        BIndex_Array16[n]=n0;
    }
#else
    BIndex_Array[n]=n0;
#endif
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
    DWORD i=n0;
    memcopy4(pIndices+n*4,&i)
    if (Indices) memcopy4(Indices+n*4,&i)
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : setcolor

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetColor(int n,float r,float g,float b)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2);

	Color_Array[adr+0]=r;
	Color_Array[adr+1]=g;
	Color_Array[adr+2]=b;
	Color_Array[adr+3]=1.0f;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2);

	BColor_Array[adr+0]=r;
	BColor_Array[adr+1]=g;
	BColor_Array[adr+2]=b;
	BColor_Array[adr+3]=1.0f;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	DWORD size=_sizeVB;

	color=D3DCOLOR_COLORVALUE(r,g,b,1);

	if (_temp==8)
	{
		memcopy4(pVertices+size*n+3*4+_blend+3*4,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+3*4+_blend,&color)
	}
	else
	if (_temp!=5)
	{
		memcopy4(pVertices+size*n+3*4+_blend,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+_blend,&color)
	}


#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : setcolor

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetColor(int n,CRGBA &col)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2);

	Color_Array[adr+0]=col.r;
	Color_Array[adr+1]=col.g;
	Color_Array[adr+2]=col.b;
	Color_Array[adr+3]=1.0f;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2);

	BColor_Array[adr+0]=col.r;
	BColor_Array[adr+1]=col.g;
	BColor_Array[adr+2]=col.b;
	BColor_Array[adr+3]=1.0f;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	DWORD size=_sizeVB;

	color=D3DCOLOR_COLORVALUE(col.r,col.g,col.b,1);

	if (_temp==8)
	{
		memcopy4(pVertices+size*n+3*4+_blend+3*4,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+3*4+_blend,&color)
	}
	else
	if (_temp!=5)
	{
		memcopy4(pVertices+size*n+3*4+_blend,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+_blend,&color)
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : setcolor

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetColor(int n,unsigned int color)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2);

	Color_Array[adr+0]=((color)&0xFF)/255.0f;
	Color_Array[adr+1]=((color>>8)&0xFF)/255.0f;
	Color_Array[adr+2]=((color>>16)&0xFF)/255.0f;
	Color_Array[adr+3]=1.0f;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2);

	BColor_Array[adr+0]=((color)&0xFF)/255.0f;
	BColor_Array[adr+1]=((color>>8)&0xFF)/255.0f;
	BColor_Array[adr+2]=((color>>16)&0xFF)/255.0f;
	BColor_Array[adr+3]=1.0f;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	DWORD size=_sizeVB;

	if (_temp==8)
	{
		memcopy4(pVertices+size*n+3*4+_blend+3*4,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+3*4+_blend,&color)
	}
	else
	if (_temp!=5)
	{
		memcopy4(pVertices+size*n+3*4+_blend,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+_blend,&color)
	}
#endif
}
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : set color

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::SetColor(int n,float r,float g,float b,float a)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2);

	Color_Array[adr+0]=r;
	Color_Array[adr+1]=g;
	Color_Array[adr+2]=b;
	Color_Array[adr+3]=a;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2);

	BColor_Array[adr+0]=r;
	BColor_Array[adr+1]=g;
	BColor_Array[adr+2]=b;
	BColor_Array[adr+3]=a;
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	DWORD color;
	DWORD size=_sizeVB;

	color=D3DCOLOR_COLORVALUE(r,g,b,a);
	
	if (_temp==8)
	{
		memcopy4(pVertices+size*n+3*4+_blend+3*4,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+2*3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+3*4+_blend,&color)
	}
	else
	if (_temp!=5)
	{
		memcopy4(pVertices+size*n+3*4+_blend,&color)
		if (stream_locked>=0) memcopy4(VerticesStream[stream_locked]+size*n+3*4+_blend,&color)
		else
		if (Vertices) memcopy4(Vertices+size*n+3*4+_blend,&color)
	}
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : set normal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetNormal(int n,float x,float y,float z)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		Normal_Array[adr+0]=x;
		Normal_Array[adr+1]=y;
		Normal_Array[adr+2]=z;
	}
	else
	{		
		streams_norms[stream_locked][adr+0]=x;
		streams_norms[stream_locked][adr+1]=y;
		streams_norms[stream_locked][adr+2]=z;
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
#ifndef OPENGL20
		Normal_Array_stok[n].x=x;
		Normal_Array_stok[n].y=y;
		Normal_Array_stok[n].z=z;
#endif
		BNormal_Array[adr+0]=x;
		BNormal_Array[adr+1]=y;
		BNormal_Array[adr+2]=z;
	}
	else
	{		
		Bstreams_norms[stream_locked][adr+0]=x;
		Bstreams_norms[stream_locked][adr+1]=y;
		Bstreams_norms[stream_locked][adr+2]=z;
	}
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	
	if (Type&API3D_NORMALDATAS)
	{
		DWORD size=_sizeVB;
		float *xyz;
		
		unsigned int adr=size*n+3*4+_blendN;

		xyz=(float*)(pVertices+adr);
		xyz[0]=x;
		xyz[1]=y;
		xyz[2]=z;

		if (stream_locked>=0)
		{
			xyz=(float*)(VerticesStream[stream_locked]+adr);
			xyz[0]=x;
			xyz[1]=y;
			xyz[2]=z;
		}
		else
		if (Vertices)
		{
			xyz=(float*)(Vertices+adr);
			xyz[0]=x;
			xyz[1]=y;
			xyz[2]=z;
		}
	}
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : set normal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetNormal(int n,CVector &v)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		Normal_Array[adr+0]=v.x;
		Normal_Array[adr+1]=v.y;
		Normal_Array[adr+2]=v.z;
	}
	else
	{
		streams_norms[stream_locked][adr+0]=v.x;
		streams_norms[stream_locked][adr+1]=v.y;
		streams_norms[stream_locked][adr+2]=v.z;
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
#ifndef OPENGL20
		Normal_Array_stok[n]=v;
#endif
		BNormal_Array[adr+0]=v.x;
		BNormal_Array[adr+1]=v.y;
		BNormal_Array[adr+2]=v.z;
	}
	else
	{
		Bstreams_norms[stream_locked][adr+0]=v.x;
		Bstreams_norms[stream_locked][adr+1]=v.y;
		Bstreams_norms[stream_locked][adr+2]=v.z;
	}

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (Type&API3D_NORMALDATAS)
	{
		DWORD size=_sizeVB;
		float *xyz;
		
		unsigned int adr=size*n+3*4+_blendN;

		xyz=(float*)(pVertices+adr);
		xyz[0]=v.x;
		xyz[1]=v.y;
		xyz[2]=v.z;
		if (stream_locked>=0)
		{
			xyz=(float*)(VerticesStream[stream_locked]+adr);
			xyz[0]=v.x;
			xyz[1]=v.y;
			xyz[2]=v.z;
		}
		else
		if (Vertices)
		{
			xyz=(float*)(Vertices+adr);
			xyz[0]=v.x;
			xyz[1]=v.y;
			xyz[2]=v.z;
		}
	}
	
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : weights

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetWeights(int n,int w,float value)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2) + w;

	Weights_Array[adr]=value;
	WeightsIndices_Array[adr]=((float)Indices_Array[adr])*2+Weights_Array[adr];
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2) + w;

	BWeights_Array[adr]=value;

#ifdef OPENGL_GLSL
	BWeightsIndices_Array[adr]=((float)BIndices_Array[adr])*2+BWeights_Array[adr];
#endif
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	DWORD size=_sizeVB;
	float val;

	memcopy4(&val,(Vertices+size*n+3*4+w*4))
	int ind=(int)(val/2);
	val=2*ind + value;
	memcopy4((pVertices+size*n+3*4+4*w),&val)
	if (Vertices) memcopy4((Vertices+size*n+3*4+4*w),&val)

#else
	DWORD size=_sizeVB;
	memcopy4((pVertices+size*n+3*4+w*4),&value)
	if (Vertices) memcopy4((Vertices+size*n+3*4+w*4),&value)
#endif
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : weights / matrix index

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::SetMatrixIndex(int n,int w,int index)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	unsigned int adr;
	unsigned int c;

	c=index&0xFF;
	adr=(n<<2) +w;
	Indices_Array[adr]=c;

	WeightsIndices_Array[adr]=((float)Indices_Array[adr])*2+Weights_Array[adr];
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	unsigned int adr;
	unsigned int c;

	c=index&0xFF;
	adr=(n<<2) +w;
	BIndices_Array[adr]=c;

#ifdef OPENGL_GLSL
	BWeightsIndices_Array[adr]=((float)BIndices_Array[adr])*2+BWeights_Array[adr];
#endif
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	DWORD size=_sizeVB;
	unsigned int c;
	c=index&0xFF;
	float value;

	memcopy4(&value,(Vertices+size*n+3*4+w*4))

	int ind=(int)(value/2.0f);
	value=value-2*ind;
	value=2*c+value;

	memcopy4((pVertices+size*n+3*4+4*w),&value)
	if (Vertices) memcopy4((Vertices+size*n+3*4+4*w),&value)
#else
	DWORD size=_sizeVB;
	unsigned char c;
	c=(index&0xFF);

	memcopy1((pVertices+size*n+3*4+4*4+w),&c)
	if (Vertices) memcopy1((Vertices+size*n+3*4+4*4+w),&c)
#endif
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : weights

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::InitWeights(int n,int w)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<2) + w;

	Weights_Array[adr]=0.0f;
	WeightsIndices_Array[adr]=0.0f;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<2) + w;

	BWeights_Array[adr]=0.0f;

#ifdef OPENGL_GLSL
	BWeightsIndices_Array[adr]=0.0f;
#endif
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	DWORD size=_sizeVB;
	float val=0.0f;
	memcopy4((pVertices+size*n+3*4+4*w),&val)
	if (Vertices) memcopy4((Vertices+size*n+3*4+4*w),&val)

#else
	DWORD size=_sizeVB;
	float value=0.0f;
	memcopy4((pVertices+size*n+3*4+w*4),&value)
	if (Vertices) memcopy4((Vertices+size*n+3*4+w*4),&value)
#endif
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : weights / matrix index

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::InitMatrixIndex(int n,int w)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	unsigned int adr;

	adr=(n<<2) +w;
	Indices_Array[adr]=0;
	WeightsIndices_Array[adr]=0.0f;
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	unsigned int adr;

	adr=(n<<2) +w;
	BIndices_Array[adr]=0;

#ifdef OPENGL_GLSL
	BWeightsIndices_Array[adr]=0.0f;
#endif
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	DWORD size=_sizeVB;
	float value=0.0f;
	memcopy4((pVertices+size*n+3*4+4*w),&value)
	if (Vertices) memcopy4((Vertices+size*n+3*4+4*w),&value)
#else
	DWORD size=_sizeVB;
	unsigned char c=0;
	memcopy1((pVertices+size*n+3*4+4*4+w),&c)
	if (Vertices) memcopy1((Vertices+size*n+3*4+4*4+w),&c)
#endif
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : coo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::SetVertex(int n,float x,float y,float z)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		Vertex_Array[adr+0]=x;
		Vertex_Array[adr+1]=y;
		Vertex_Array[adr+2]=z;
	}
	else
	{
		streams[stream_locked][adr+0]=x;
		streams[stream_locked][adr+1]=y;
		streams[stream_locked][adr+2]=z;
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		BVertex_Array[adr+0]=x;
		BVertex_Array[adr+1]=y;
		BVertex_Array[adr+2]=z;
	}
	else
	{
		Bstreams[stream_locked][adr+0]=x;
		Bstreams[stream_locked][adr+1]=y;
		Bstreams[stream_locked][adr+2]=z;
	}
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	/*
	float xyz[3];
	DWORD size=_sizeVB;

	xyz[0]=x;
	xyz[1]=y;
	xyz[2]=z;

	


	memcpy((pVertices+size*n),xyz,3*4);
	if (Vertices) memcpy((Vertices+size*n),xyz,3*4);
	/**/
	DWORD size=_sizeVB;
	float * xyz=(float*)(pVertices+size*n);
	xyz[0]=x;
	xyz[1]=y;
	xyz[2]=z;

	if (stream_locked>=0)
	{
		xyz=(float*)(VerticesStream[stream_locked]+size*n);
		xyz[0]=x;
		xyz[1]=y;
		xyz[2]=z;
	}
	else
	if (Vertices)
	{
		xyz=(float*)(Vertices+size*n);
		xyz[0]=x;
		xyz[1]=y;
		xyz[2]=z;
	}
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : coo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetVertex(int n,CVector &v)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		Vertex_Array[adr+0]=v.x;
		Vertex_Array[adr+1]=v.y;
		Vertex_Array[adr+2]=v.z;
	}
	else
	{
		streams[stream_locked][adr+0]=v.x;
		streams[stream_locked][adr+1]=v.y;
		streams[stream_locked][adr+2]=v.z;
	}

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1) +n;

	if (stream_locked==-1)
	{
		BVertex_Array[adr+0]=v.x;
		BVertex_Array[adr+1]=v.y;
		BVertex_Array[adr+2]=v.z;
	}
	else
	{
		Bstreams[stream_locked][adr+0]=v.x;
		Bstreams[stream_locked][adr+1]=v.y;
		Bstreams[stream_locked][adr+2]=v.z;
	}

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	/*
	float xyz[3];
	DWORD size=_sizeVB;

	xyz[0]=v.x;
	xyz[1]=v.y;
	xyz[2]=v.z;

	memcpy((pVertices+size*n),xyz,3*4);
	if (Vertices) memcpy((Vertices+size*n),xyz,3*4);
	/**/
	DWORD size=_sizeVB;
	float * xyz=(float*)(pVertices+size*n);
	xyz[0]=v.x;
	xyz[1]=v.y;
	xyz[2]=v.z;
	if (stream_locked>=0)
	{
		xyz=(float*)(VerticesStream[stream_locked]+size*n);
		xyz[0]=v.x;
		xyz[1]=v.y;
		xyz[2]=v.z;
	}
	else
	if (Vertices)
	{
		xyz=(float*)(Vertices+size*n);
		xyz[0]=v.x;
		xyz[1]=v.y;
		xyz[2]=v.z;
	}
#endif
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo(int n,float x,float y)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	if (stream_locked==-1)
	{
		int adr;
		adr=(n<<1);

		TexCoo_Array[adr+0]=x;
		TexCoo_Array[adr+1]=y;
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

	if (stream_locked==-1)
	{
		int adr;
		adr=(n<<1);

		BTexCoo_Array[adr+0]=x;
		BTexCoo_Array[adr+1]=y;
	}
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=x;
	xym[1]=y;

	if (_temp!=2)
	{
		if (_temp==5)
		{
			memcopy8((pVertices+size*n+_blend+3*4+3*4),xym)
			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+3*4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+3*4),xym)
		}
		else
		if (_temp==8)
		{
			memcopy8((pVertices+size*n+3*4+_blend+4+3*4),xym)

			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4+3*4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+3*4),xym)
		}
		else
		{
			memcopy8((pVertices+size*n+3*4+_blend+4),xym)
			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4),xym)
		}
	}

#endif
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo(int n,CVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	if (stream_locked==-1)
	{
		int adr;
		adr=(n<<1);
		TexCoo_Array[adr+0]=map.x;
		TexCoo_Array[adr+1]=map.y;
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	if (stream_locked==-1)
	{
		int adr;
		adr=(n<<1);
		BTexCoo_Array[adr+0]=map.x;
		BTexCoo_Array[adr+1]=map.y;
	}
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=map.x;
	xym[1]=map.y;

	if (_temp!=2)
	{
		if (_temp==5)
		{
			memcopy8((pVertices+size*n+_blend+3*4+3*4),xym)
			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+3*4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+3*4),xym)
		}
		else
		if (_temp==8)
		{
			memcopy8((pVertices+size*n+3*4+_blend+4+3*4),xym)

			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4+3*4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+3*4),xym)
		}
		else
		{
			memcopy8((pVertices+size*n+3*4+_blend+4),xym)
			if (stream_locked>=0)
			{
				memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4),xym)
			}
			else
			if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4),xym)
		}
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo(int n,UCVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
    if (stream_locked==-1)
    {
        int adr;
        adr=(n<<1);
        TexCoo_Array[adr+0]=map.x;
        TexCoo_Array[adr+1]=map.y;
    }
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
    if (stream_locked==-1)
    {
        int adr;
        adr=(n<<1);
        BTexCoo_Array[adr+0]=map.x;
        BTexCoo_Array[adr+1]=map.y;
    }
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
    float xym[2];
    DWORD size=_sizeVB;

    xym[0]=map.x;
    xym[1]=map.y;

    if (_temp!=2)
    {
        if (_temp==5)
        {
            memcopy8((pVertices+size*n+_blend+3*4+3*4),xym)
            if (stream_locked>=0)
            {
                memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+3*4),xym)
            }
            else
            if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+3*4),xym)
        }
        else
        if (_temp==8)
        {
            memcopy8((pVertices+size*n+3*4+_blend+4+3*4),xym)

            if (stream_locked>=0)
            {
                memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4+3*4),xym)
            }
            else
            if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+3*4),xym)
        }
        else
        {
            memcopy8((pVertices+size*n+3*4+_blend+4),xym)
            if (stream_locked>=0)
            {
                memcopy8((VerticesStream[stream_locked]+size*n+3*4+_blend+4),xym)
            }
            else
            if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4),xym)
        }
    }

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo3(int n,float x,float y)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1);
	TexCoo3_Array[adr+0]=x;
	TexCoo3_Array[adr+1]=y;

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

	int adr;
	adr=(n<<1);
	BTexCoo3_Array[adr+0]=x;
	BTexCoo3_Array[adr+1]=y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=x;
	xym[1]=y;

	if ((_temp==7)||(_temp==9))
	{
		memcopy8((pVertices+size*n+3*4+_blend+4+2*2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+2*2*4),xym)
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo3(int n,CVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------

	int adr;
	adr=(n<<1);
	TexCoo3_Array[adr+0]=map.x;
	TexCoo3_Array[adr+1]=map.y;

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

	int adr;
	adr=(n<<1);
	BTexCoo3_Array[adr+0]=map.x;
	BTexCoo3_Array[adr+1]=map.y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=map.x;
	xym[1]=map.y;

	if ((_temp==7)||(_temp==9))
	{		
		memcopy8((pVertices+size*n+4+3*4+_blend+2*2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+4+3*4+_blend+2*2*4),xym)
	}

#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo4(int n,float x,float y)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
    int adr;
    adr=(n<<1);
    TexCoo4_Array[adr+0]=x;
    TexCoo4_Array[adr+1]=y;
    
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

	int adr;
	adr=(n<<1);
	BTexCoo4_Array[adr+0]=x;
	BTexCoo4_Array[adr+1]=y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=x;
	xym[1]=y;

	if (_temp==9)
	{		
		memcopy8((pVertices+size*n+3*4+_blend+4+3*2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+3*2*4),xym)
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void FNCALLCONVVB CVertexBuffer::SetTexCoo4(int n,CVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------

	int adr;
	adr=(n<<1);
	TexCoo4_Array[adr+0]=map.x;
	TexCoo4_Array[adr+1]=map.y;

#endif


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

	int adr;
	adr=(n<<1);
	BTexCoo4_Array[adr+0]=map.x;
	BTexCoo4_Array[adr+1]=map.y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float xym[2];
	DWORD size=_sizeVB;
	
	xym[0]=map.x;
	xym[1]=map.y;

	if (_temp==9)
	{		
		memcopy8((pVertices+size*n+4+3*4+_blend+3*2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+4+3*4+_blend+3*2*4),xym)
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping 2

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo2(int n,float x,float y)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1);

	TexCoo2_Array[adr+0]=x;
	TexCoo2_Array[adr+1]=y;
#endif


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1);

	BTexCoo2_Array[adr+0]=x;
	BTexCoo2_Array[adr+1]=y;
#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=x;
	xym[1]=y;

	if (_temp==8)
	{		
		memcopy8((pVertices+size*n+3*4*2+_blend+4+2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4*2+_blend+4+2*4),xym)
	}
	else
	if (_temp!=2)
	{		
		memcopy8((pVertices+size*n+3*4+_blend+4+8),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+8),xym)
	}
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vertex buffer : mapping 2

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void FNCALLCONVVB CVertexBuffer::SetTexCoo2(int n,CVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int adr;
	adr=(n<<1);

	TexCoo2_Array[adr+0]=map.x;
	TexCoo2_Array[adr+1]=map.y;

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int adr;
	adr=(n<<1);

	BTexCoo2_Array[adr+0]=map.x;
	BTexCoo2_Array[adr+1]=map.y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	float xym[2];
	DWORD size=_sizeVB;

	xym[0]=map.x;
	xym[1]=map.y;

	if (_temp==8)
	{		
		memcopy8((pVertices+size*n+3*4*2+_blend+4+2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4*2+_blend+4+2*4),xym)
	}
	else
	if (_temp!=2)
	{		
		memcopy8((pVertices+size*n+3*4+_blend+4+8),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+8),xym)
	}
#endif
}

void FNCALLCONVVB CVertexBuffer::SetTexCoo2(int n,UCVector2 &map)
{
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
    int adr;
    adr=(n<<1);

    TexCoo2_Array[adr+0]=map.x;
    TexCoo2_Array[adr+1]=map.y;

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
    int adr;
    adr=(n<<1);

    BTexCoo2_Array[adr+0]=map.x;
    BTexCoo2_Array[adr+1]=map.y;

#endif
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
    float xym[2];
    DWORD size=_sizeVB;

    xym[0]=map.x;
    xym[1]=map.y;

	if (_temp==8)
	{		
		memcopy8((pVertices+size*n+3*4*2+_blend+4+2*4),xym)
		if (Vertices) memcopy8((Vertices+size*n+3*4*2+_blend+4+2*4),xym)
	}
	else
    if (_temp!=2)
    {
        memcopy8((pVertices+size*n+3*4+_blend+4+8),xym)
        if (Vertices) memcopy8((Vertices+size*n+3*4+_blend+4+8),xym)
    }
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  fucntion:

	Vertex buffer : PURGE

	free resources

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::Purge()
{
#define FR(ptr) { if (ptr) { free(ptr); ptr=NULL; } }

#if defined(API3D_METAL)

	FR(Index_Array)

	FR(Indices_Array)
	FR(Weights_Array)
	FR(WeightsIndices_Array)

	FR(Color_Array)
	FR(TexCoo_Array)
	FR(TexCoo2_Array)
	FR(TexCoo3_Array)
	FR(TexCoo4_Array)
	FR(Vertex_Array)
	FR(Normal_Array)

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

    if (vbo)
    {
        FR(BColor_Array)
        FR(BVertex_Array)
        FR(BTexCoo_Array)
        FR(BTexCoo2_Array)
        FR(BTexCoo3_Array)
        FR(BTexCoo4_Array)
        FR(BNormal_Array)
        FR(BIndices_Array)
        FR(BWeights_Array)
        FR(BWeightsIndices_Array)
        FR(BIndex_Array)
#ifdef ANDROID
        FR(BIndex_Array16)
#endif
    }

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  fucntion:

	Vertex buffer : RELEASE

	free resources

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#if !defined(WIN32)&&!defined(WINDOWS_STORE)
#include <unistd.h>
#endif

void CVertexBuffer::Release()
{
	while (dirty<0)
	{
#if defined(WIN32)||defined(WINDOWS_STORE)
		Sleep(1);
#else
		usleep(1000);
#endif
	}

	for (int n=0;n<18;n++)
	{
		if (AssignedTexture[n])
        {
            if (AssignedTexture[n][0]!=0) free(AssignedTexture[n]);
        }
		AssignedTexture[n]=NULL;
#if defined(API3D_DIRECT3D12)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D10)||defined(API3D_OPENGL20)
		res[n]=NULL;
#endif
	}

	shadervariables.Free();

    if (TheClass3DAPI->AutomaticallyCloneShaders)
    {
		if (Type&API3D_VERTEXPROGRAM)
		{
			if (hasbeencloned) MigrateShader(this);
			else
			{
				if (cloned==1) UnregisterShader(this);
				else SuppressShader(this);
			}
		}
    }
    
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------

	VertexBufferContainer.Del(this);

	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
        MTLFreeProgramVertexBuffer(this);
        
		if (vp->shadermodel)
		{
			for (int p=0;p<vp->get_pass_count();p++)
			{
			}
		}
		if (vp)
		{
			vp->Free();
			delete vp;
		}
    }

	vp=NULL;
	Type=0;

	MTLFreeBuffer(this);

	IB=VB=-1;

	if (Index_Array) free(Index_Array);

	if (Color_Array) free(Color_Array);
	if (TexCoo_Array) free(TexCoo_Array);
	if (TexCoo2_Array) free(TexCoo2_Array);
	if (TexCoo3_Array) free(TexCoo3_Array);
	if (TexCoo4_Array) free(TexCoo4_Array);
	if (Vertex_Array) free(Vertex_Array);

	if (Normal_Array) free(Normal_Array);

	if (Indices_Array)
	{
		free(Indices_Array);
		free(Weights_Array);
		free(WeightsIndices_Array);
	}

	Index_Array=NULL;

	Indices_Array=NULL;
	Weights_Array=NULL;
	WeightsIndices_Array=NULL;

	Color_Array=NULL;
	TexCoo_Array=NULL;
	TexCoo2_Array=NULL;
	TexCoo3_Array=NULL;
	TexCoo4_Array=NULL;
	Vertex_Array=NULL;
	Normal_Array=NULL;

	if (streams)
	{
		for (int n=0;n<nstreams;n++)
		{
			free(streams[n]);
			free(streams_norms[n]);
		}

		free(streams);
		free(streams_norms);	
		streams=NULL;
		streams_norms=NULL;
	}
#endif


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

	VertexBufferContainer.Del(this);

	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
#ifndef GLES
		if (vp->shadermodel)
		{
			for (int p=0;p<vp->get_pass_count();p++)
			{
				glDetachObjectARB(glslprograms[p],vert_shader[p]);
				glDetachObjectARB(glslprograms[p],frag_shader[p]);
				glDeleteObjectARB(vert_shader[p]);
				glDeleteObjectARB(frag_shader[p]);
				glDeleteObjectARB(glslprograms[p]);
			}
		}
#ifdef OPENGL_VERTEXPROGRAMS
		else glDeleteProgramsARB(vp->get_pass_count(),programs);
#endif
#endif
		if (vp)
		{
			vp->Free();
			delete vp;
		}
	}

	vp=NULL;
	Type=0;

	if (BIndex_Array) free(BIndex_Array);
#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
    if (BIndex_Array16) free(BIndex_Array16);
#endif

	if (BColor_Array) free(BColor_Array);
	if (BTexCoo_Array) free(BTexCoo_Array);
	if (BTexCoo2_Array) free(BTexCoo2_Array);
	if (BTexCoo3_Array) free(BTexCoo3_Array);
	if (BTexCoo4_Array) free(BTexCoo4_Array);
	if (BVertex_Array) free(BVertex_Array);

	if (Vertex2_Array) free(Vertex2_Array);

	if (BNormal_Array)
	{
		free(BNormal_Array);
#ifndef OPENGL20
		delete [] Normal_Array_stok;
#endif
	}

	if (BIndices_Array)
	{
		free(BIndices_Array);
		free(BWeights_Array);
#ifdef OPENGL_GLSL
		free(BWeightsIndices_Array);
#endif
	}

	Index_Array=NULL;
	BIndex_Array=NULL;

#if defined(ANDROID)&&!defined(UNSIGNEDSHORTINDICES)
	Index_Array16=NULL;
	BIndex_Array16=NULL;
#endif

	Indices_Array=NULL;
	Weights_Array=NULL;
	WeightsIndices_Array=NULL;

	Color_Array=NULL;
	TexCoo_Array=NULL;
	TexCoo2_Array=NULL;
	TexCoo3_Array=NULL;
	TexCoo4_Array=NULL;
	Vertex_Array=NULL;
	Normal_Array=NULL;

	BIndices_Array=NULL;
	BWeights_Array=NULL;
	BWeightsIndices_Array=NULL;

	BColor_Array=NULL;
	BTexCoo_Array=NULL;
	BTexCoo2_Array=NULL;
	BTexCoo3_Array=NULL;
	BTexCoo4_Array=NULL;
	BVertex_Array=NULL;
	BNormal_Array=NULL;

	if (vbo)
	{
		glDeleteBuffersARB(1,&VB);
		glDeleteBuffersARB(1,&IB);
	}

	if (streams)
	{
		for (int n=0;n<nstreams;n++)
		{
			free(Bstreams[n]);
			free(Bstreams_norms[n]);
		}

		free(Bstreams);
		free(Bstreams_norms);

		if (vbo)
		{
			free(streams);
			free(streams_norms);
		}

		streams=NULL;
		streams_norms=NULL;

		Bstreams=NULL;
		Bstreams_norms=NULL;

#ifndef API3D_OPENGL20		
		free(temp_vertices);
		free(temp_normals);
#endif
	}

#endif

#ifdef API3D_DIRECT3D

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
		if (effect) effect->Release();
		if (vp)
		{
			vp->Free();
			delete vp;
		}
	}

	vp=NULL;
	if (D3D8VB) D3D8VB->Release();
	if (D3D8INDEX) D3D8INDEX->Release();

	if (nSTREAMS>0)
	{
		for (int n=0;n<nSTREAMS;n++) STREAMS[n]->Release();
		free(STREAMS);
	}
	STREAMS=NULL;
	nSTREAMS=0;

	if (Vertices) free(Vertices);
	if (Indices) free(Indices);
	Vertices=Indices=NULL;

	D3D8VB=NULL;
	D3D8INDEX=NULL;

	VertexBufferContainer.Del(this);

#endif


#ifdef API3D_DIRECT3D9

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
		if (effect) effect->Release();
		if (vp)
		{
			vp->Free();
			delete vp;
		}
	}

	vp=NULL;
	effect=NULL;

	if (D3D9VB) D3D9VB->Release();
	if (D3D9INDEX) D3D9INDEX->Release();

	if (nSTREAMS>0)
	{
		for (int n=0;n<nSTREAMS;n++)
		{
			STREAMS[n]->Release();
			if (VerticesStream[n]) free(VerticesStream[n]);
		}
		free(VerticesStream);
		free(STREAMS);
	}
	STREAMS=NULL;
	VerticesStream=NULL;
	nSTREAMS=0;

	if (Vertices) free(Vertices);
	if (Indices) free(Indices);

	Vertices=Indices=NULL;

	D3D9VB=NULL;
	D3D9INDEX=NULL;

	VertexBufferContainer.Del(this);

#endif

#ifdef API3D_DIRECT3D10

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
		if (effect) effect->Release();
		
		if (vp)
		{
			vp->Free();
			delete vp;
		}
	}

	vp=NULL;
	effect=NULL;

	if (layout) layout->Release();
	if (D3DVB) D3DVB->Release();
	if (D3DINDEX) D3DINDEX->Release();

	if (nSTREAMS>0)
	{
		for (int n=0;n<nSTREAMS;n++)
		{
			STREAMS[n]->Release();
			if (VerticesStream[n]) free(VerticesStream[n]);
		}
		free(VerticesStream);
		free(STREAMS);
	}
	STREAMS=NULL;
	VerticesStream=NULL;
	nSTREAMS=0;

	if (Vertices) free(Vertices);
	if (Indices) free(Indices);

	Vertices=Indices=NULL;

	layout=NULL;
	D3DVB=NULL;
	D3DINDEX=NULL;

	VertexBufferContainer.Del(this);

#endif


#ifdef API3D_DIRECT3D11

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
		if (effect) effect->Release();
		
		if (vp)
		{
			vp->Free();
			delete vp;
		}
	}

	vp=NULL;
	effect=NULL;

	if (layout) layout->Release();
	if (D3DVB) D3DVB->Release();
	if (D3DINDEX) D3DINDEX->Release();

	if (nSTREAMS>0)
	{
		for (int n=0;n<nSTREAMS;n++)
		{
			STREAMS[n]->Release();
			if (VerticesStream[n]) free(VerticesStream[n]);
		}
		free(VerticesStream);
		free(STREAMS);
	}
	STREAMS=NULL;
	VerticesStream=NULL;
	nSTREAMS=0;

	if (Vertices) free(Vertices);
	if (Indices) free(Indices);

	Vertices=Indices=NULL;

	layout=NULL;
	D3DVB=NULL;
	D3DINDEX=NULL;

	VertexBufferContainer.Del(this);

#endif


#ifdef API3D_DIRECT3D12

//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	C3DAPIBASE *render=(C3DAPIBASE*) capi;

	if ((Type&API3D_VERTEXPROGRAM)&&(cloned==0))
	{
		if (effect) effect->Release();		// TODO
		
		if (vp)
		{
			vp->Free();
			delete vp;
		}

		render->VBVP.Delete(this);
	}

	ID3D12PipelineState** p_pipe=pipelines.GetFirst();
	while (p_pipe)
	{
		(*p_pipe)->Release();

		p_pipe=pipelines.DeleteAndGetNext();
	}

	vp=NULL;
	effect=NULL;

	if (layout)
	{
		delete [] layout;
		layout=NULL;
	}

	if (render)
	if (render->rendering)
	{		
		if (D3DVB) render->DynamicResources[render->frameIndex].Add(D3DVB);
		if (D3DVBTMP) render->DynamicResources[render->frameIndex].Add(D3DVBTMP);

		if (D3DINDEX) render->DynamicResources[render->frameIndex].Add(D3DINDEX);
		if (D3DINDEXTMP) render->DynamicResources[render->frameIndex].Add(D3DINDEXTMP);

		if (nSTREAMS>0)
		{
			for (int n=0;n<nSTREAMS;n++)
			{
				render->DynamicResources[render->frameIndex].Add(STREAMS[n]);			
				if (VerticesStream[n]) free(VerticesStream[n]);
			}
			free(VerticesStream);
			free(STREAMS);
			free(STREAMSVIEW);
		}
	}
	else
	{
		if (D3DVB) D3DVB->Release();
		if (D3DVBTMP) D3DVBTMP->Release();

		if (D3DINDEX) D3DINDEX->Release();
		if (D3DINDEXTMP) D3DINDEXTMP->Release();

		if (nSTREAMS>0)
		{
			for (int n=0;n<nSTREAMS;n++)
			{
				STREAMS[n]->Release();			
				if (VerticesStream[n]) free(VerticesStream[n]);
			}
			free(VerticesStream);
			free(STREAMS);
			free(STREAMSVIEW);
		}
	}
	

	if (STREAMSTMP) STREAMSTMP->Release();
	STREAMSTMP=NULL;

	STREAMS=NULL;
	STREAMSVIEW=NULL;
	VerticesStream=NULL;
	nSTREAMS=0;

	if (Vertices) free(Vertices);
	if (Indices) free(Indices);

	Vertices=Indices=NULL;

	if (signature) signature->Release();
	signature=NULL;
	if (commandsignature) commandsignature->Release();
	commandsignature=NULL;

	layout=NULL;
	D3DVB=NULL;
	D3DINDEX=NULL;
	D3DVBTMP=NULL;
	D3DINDEXTMP=NULL;

	VertexBufferContainer.Del(this);

#endif

	ListeSE.Free();
	if (CorresVertices) delete [] CorresVertices;
	if (CorresVerticesNorm) delete [] CorresVerticesNorm;

	if (secondary) secondary->Release();
    if (lod) lod->Release();
    
    lod=NULL;

	if (sorting) delete [] sorting;
	sorting=NULL;

	CorresVertices=NULL;
	CorresVerticesNorm=NULL;
	secondary=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexBuffer::FillMorphStreamEDGESDG(int stream,CObject3D *obj)
{
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20) || defined(API3D_METAL)
	int n;

	if (stream==0) LockVertices(); else LockStream(stream-1);

	for (n=0;n<nVertices;n++)
	{
		SetVertex(n,obj->Vertices[CorresVertices[n]].Stok);
		if (CorresVerticesNorm[n]>=0) SetNormal(n,obj->Faces[CorresVerticesNorm[n]].Norm);
		else SetNormal(n,-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.x,-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.y,-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.z);
	}

	if (stream==0) UnlockVertices(); else UnlockStream();
#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
	int n;
	int pos;
	float *p;
	DWORD size=_sizeVB;

	if (stream==0) LockVertices(); else LockStream(stream-1);
	pos=0;
	for (n=0;n<(int)nVertices;n++)
	{
		p=(float*) (&pVertices[pos]);

		p[0]=obj->Vertices[CorresVertices[n]].Stok.x;
		p[1]=obj->Vertices[CorresVertices[n]].Stok.y;
		p[2]=obj->Vertices[CorresVertices[n]].Stok.z;

		if (CorresVerticesNorm[n]>=0)
		{
			p[3]=obj->Faces[CorresVerticesNorm[n]].Norm.x;
			p[4]=obj->Faces[CorresVerticesNorm[n]].Norm.y;
			p[5]=obj->Faces[CorresVerticesNorm[n]].Norm.z;
		}
		else
		{
			p[3]=-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.x;
			p[4]=-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.y;
			p[5]=-obj->Faces[-(CorresVerticesNorm[n]+1)].Norm.z;
		}

		pos+=size;
	}

	if (stream==0) UnlockVertices(); else UnlockStream();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexBuffer::FillMorphStreamEDGESDGCalc(int stream,CObject3D *obj)
{
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20) || defined(API3D_METAL)
	int n;

	if (stream==0) LockVertices(); else LockStream(stream-1);

	for (n=0;n<nVertices;n++)
	{
		SetVertex(n,obj->Vertices[CorresVertices[n]].Calc);
		if (CorresVerticesNorm[n]>=0) SetNormal(n,obj->Faces[CorresVerticesNorm[n]].NormCalc);
		else SetNormal(n,-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.x,-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.y,-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.z);
	}

	if (stream==0) UnlockVertices(); else UnlockStream();

#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)

	int n;
	int pos;
	float *p;
	DWORD size=_sizeVB;

	if (stream==0) LockVertices(); else LockStream(stream-1);
	pos=0;
	for (n=0;n<(int)nVertices;n++)
	{
		p=(float*) (&pVertices[pos]);

		p[0]=obj->Vertices[CorresVertices[n]].Calc.x;
		p[1]=obj->Vertices[CorresVertices[n]].Calc.y;
		p[2]=obj->Vertices[CorresVertices[n]].Calc.z;

		if (CorresVerticesNorm[n]>=0)
		{
			p[3]=obj->Faces[CorresVerticesNorm[n]].NormCalc.x;
			p[4]=obj->Faces[CorresVerticesNorm[n]].NormCalc.y;
			p[5]=obj->Faces[CorresVerticesNorm[n]].NormCalc.z;
		}
		else
		{
			p[3]=-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.x;
			p[4]=-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.y;
			p[5]=-obj->Faces[-(CorresVerticesNorm[n]+1)].NormCalc.z;
		}

		pos+=size;
	}

	if (stream==0) UnlockVertices(); else UnlockStream();
#endif
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  fucntion:

	Vertex buffer : Morph vertex buffer

	free resources

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::FillMorphStream(int stream,CObject3D * obj,int key)
{
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20) || defined(API3D_METAL)

	int n;

	if (stream==0) LockVertices(); else LockStream(stream-1);

	if ((Type&API3D_NORMALDATAS)!=0)
	{
		for (n=0;n<obj->nVertices;n++)
		{
			SetVertex(n,obj->VerticesKeys[key][n].Stok);
			SetNormal(n,obj->VerticesKeys[key][n].Norm);
		}
	}
	else
	{
		for (n=0;n<obj->nVertices;n++)  SetVertex(n,obj->VerticesKeys[key][n].Stok);
	}

	if (stream==0) UnlockVertices(); else UnlockStream();

#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
	int n;
	int pos=0;
	float *p;
	DWORD size=_sizeVB;

	if (TypeVB&32)
	{
		if (stream==0) LockVertices();	else LockStream(stream-1);

		if ((Type&API3D_NORMALDATAS)!=0)
		{
			for (n=0;n<obj->nVertices;n++)
			{
				p=(float*) (&pVertices[pos]);
				p[0]=obj->VerticesKeys[key][n].Stok.x;
				p[1]=obj->VerticesKeys[key][n].Stok.y;
				p[2]=obj->VerticesKeys[key][n].Stok.z;
#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)
				p[3+4*4]=obj->VerticesKeys[key][n].Norm.x;
				p[4+4*4]=obj->VerticesKeys[key][n].Norm.y;
				p[5+4*4]=obj->VerticesKeys[key][n].Norm.z;
#else
				p[3+5*4]=obj->VerticesKeys[key][n].Norm.x;
				p[4+5*4]=obj->VerticesKeys[key][n].Norm.y;
				p[5+5*4]=obj->VerticesKeys[key][n].Norm.z;
#endif
				pos+=size;
			}
		}
		else
		{
			for (n=0;n<obj->nVertices;n++)
			{
				p=(float*) (&pVertices[pos]);
				p[0]=obj->VerticesKeys[key][n].Stok.x;
				p[1]=obj->VerticesKeys[key][n].Stok.y;
				p[2]=obj->VerticesKeys[key][n].Stok.z;
				pos+=size;
			}
		}

		if (stream==0) UnlockVertices(); else UnlockStream();
	}
	else
	{
		if (stream==0) LockVertices(); else LockStream(stream-1);

		if ((Type&API3D_NORMALDATAS)!=0)
		{
			for (n=0;n<obj->nVertices;n++)
			{
				p=(float*) (&pVertices[pos]);
				p[0]=obj->VerticesKeys[key][n].Stok.x;
				p[1]=obj->VerticesKeys[key][n].Stok.y;
				p[2]=obj->VerticesKeys[key][n].Stok.z;
				p[3]=obj->VerticesKeys[key][n].Norm.x;
				p[4]=obj->VerticesKeys[key][n].Norm.y;
				p[5]=obj->VerticesKeys[key][n].Norm.z;
				pos+=size;
			}
		}
		else
		{
			for (n=0;n<obj->nVertices;n++)
			{
				p=(float*) (&pVertices[pos]);
				p[0]=obj->VerticesKeys[key][n].Stok.x;
				p[1]=obj->VerticesKeys[key][n].Stok.y;
				p[2]=obj->VerticesKeys[key][n].Stok.z;
				pos+=size;
			}
		}

		if (stream==0) UnlockVertices(); else UnlockStream();
	}
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  fucntion:

	Vertex buffer : CREATE

	free resources

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::CreateFrom2(CObject3D * obj,int type)
{
	int n,nf,np;
	CFace * f;

	nf=obj->nFaces;
	np=obj->nVertices;

	Grouped=false;

	SetType(type);

    if (Type&API3D_STREAMS) Init(np,nf*3,obj->nKeys);
    else
    {
        if ((type&API3D_MORPH)&&(type&API3D_NORMALDATAS)) Init(np,nf*3,1);
        else Init(np,nf*3);
    }
    
	LockVertices();
	LockIndices();

    if ((Type&API3D_VERTEXCALC)!=0)
    {
        for (n=0;n<obj->nVertices;n++)
        {
            SetVertex(n,obj->Vertices[n].Calc);
            
            if ((Type&API3D_TEXPERVERTEX)!=0) SetTexCoo(n,obj->Vertices[n].Map);
            if (((Type&API3D_TEXPERVERTEX)!=0)&&(Type&API3D_TEXCOODATAS2)) SetTexCoo2(n,obj->Vertices[n].Map2);

            if ((Type&API3D_NORMALDATAS)!=0) SetNormal(n,obj->Vertices[n].NormCalc);

            if ((Type&API3D_DIFFUSEASSTOK0)!=0)
            {
                if ((Type&API3D_COLORDATAS4)!=0) SetColor(n,obj->Vertices[n].Stok0.x,obj->Vertices[n].Stok0.y,obj->Vertices[n].Stok0.z,obj->Vertices[n].coef1);
                else
                if ((Type&API3D_COLORDATAS)!=0) SetColor(n,obj->Vertices[n].Stok0.x,obj->Vertices[n].Stok0.y,obj->Vertices[n].Stok0.z);
            }
            else
            {
                if ((Type&API3D_COLORDATAS4)!=0) SetColor(n,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b,obj->Vertices[n].Diffuse.a);
                else
                if ((Type&API3D_COLORDATAS)!=0) SetColor(n,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b);
            }
            
            if (Type&API3D_BLENDING)
            {
                SetWeights(n,0,obj->Vertices[n].Weight[0]);
                SetWeights(n,1,obj->Vertices[n].Weight[1]);
                SetWeights(n,2,obj->Vertices[n].Weight[2]);
                SetWeights(n,3,obj->Vertices[n].Weight[3]);

                SetMatrixIndex(n,0,obj->Vertices[n].Index[0]);
                SetMatrixIndex(n,1,obj->Vertices[n].Index[1]);
                SetMatrixIndex(n,2,obj->Vertices[n].Index[2]);
                SetMatrixIndex(n,3,obj->Vertices[n].Index[3]);
            }
        }
    }
    else
    {
        for (n=0;n<obj->nVertices;n++)
        {
            SetVertex(n,obj->Vertices[n].Stok);

            if ((Type&API3D_TEXPERVERTEX)!=0) SetTexCoo(n,obj->Vertices[n].Map);
            if (((Type&API3D_TEXPERVERTEX)!=0)&&(Type&API3D_TEXCOODATAS2)) SetTexCoo2(n,obj->Vertices[n].Map2);

            if ((Type&API3D_NORMALDATAS)!=0) SetNormal(n,obj->Vertices[n].Norm);

            if ((Type&API3D_DIFFUSEASSTOK0)!=0)
            {
                if ((Type&API3D_COLORDATAS4)!=0) SetColor(n,obj->Vertices[n].Stok0.x,obj->Vertices[n].Stok0.y,obj->Vertices[n].Stok0.z,obj->Vertices[n].coef1);
                else
                if ((Type&API3D_COLORDATAS)!=0) SetColor(n,obj->Vertices[n].Stok0.x,obj->Vertices[n].Stok0.y,obj->Vertices[n].Stok0.z);
            }
            else
            {
                if ((Type&API3D_COLORDATAS4)!=0) SetColor(n,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b,obj->Vertices[n].Diffuse.a);
                else
                if ((Type&API3D_COLORDATAS)!=0) SetColor(n,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b);
            }
            
            if (Type&API3D_BLENDING)
            {
                SetWeights(n,0,obj->Vertices[n].Weight[0]);
                SetWeights(n,1,obj->Vertices[n].Weight[1]);
                SetWeights(n,2,obj->Vertices[n].Weight[2]);
                SetWeights(n,3,obj->Vertices[n].Weight[3]);

                SetMatrixIndex(n,0,obj->Vertices[n].Index[0]);
                SetMatrixIndex(n,1,obj->Vertices[n].Index[1]);
                SetMatrixIndex(n,2,obj->Vertices[n].Index[2]);
                SetMatrixIndex(n,3,obj->Vertices[n].Index[3]);
            }
        }
    }
        
    if (Type&API3D_STREAMS)
    {
        for (int k=0;k<obj->nKeys;k++)
        {
            LockStream(k);
            for (n=0;n<obj->nVertices;n++)
            {
                SetNormal(n,obj->VerticesKeys[k][n].Norm);
                SetVertex(n,obj->VerticesKeys[k][n].Stok);

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20)
                SetTexCoo(n,obj->Vertices[n].Map);
#endif
            }
            UnlockStream();
        }
    }

	for (n=0;n<obj->nFaces;n++)
	{
		f=&obj->Faces[n];
		SetIndices(n,f->v0,f->v1,f->v2);

		if ((Type&API3D_TEXPERVERTEX)==0)
		{
			if ((Type&API3D_TEXCOODATAS)!=0)
			{
				SetTexCoo(f->v0,f->mp0);
				SetTexCoo(f->v1,f->mp1);
				SetTexCoo(f->v2,f->mp2);
			}
		}
	}

	UnlockVertices();
	UnlockIndices();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	function:


		Vertex Buffer : Create from a CObject3D

		filter on Faces[].tag

		+API3D_VERTEXDATAS			get .Stok
		+
			API3D_COLORDATAS
			API3D_COLORDATAS4
		or
			API3D_NORMALDATAS
		+API3D_TEXCOODATAS
		+
			API3D_TEXCOODATAS2
		or
			API3D_TEXCOO2LIGHTMAP

		+API3D_TEXCOO3BUMP	(reserved)

		+API3D_TEXPERVERTEX
		+API3D_VERTEXCALC			get .Calc

		+API3D_ENVMAPPING			set environment mapping shader to render
		+API3D_EDGESDG				(reserved)
		+API3D_ENABLEUPDATE			generate index list to enable modification

		+API3D_VERTEXPROGRAM		specifies th vertex buffer TnL (reserved)
		+API3D_BLENDING				enable 5x4 weighting values


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::CreateFrom(CObject3D * obj,int tag,int type)
{
	int n,nf,np,nn;
	CFace * f;
	int * corres;

	for (n=0;n<obj->nVertices;n++) obj->Vertices[n].temp=0;

	corres=(int *) malloc(4*obj->nVertices);

	nf=0;
	for (n=0;n<obj->nFaces;n++)
	{
		if (obj->Faces[n].tag==tag)
		{
			nf++;
			obj->Faces[n].v[0]->temp=1;
			obj->Faces[n].v[1]->temp=1;
			obj->Faces[n].v[2]->temp=1;
		}
	}

	np=0;
	for (n=0;n<obj->nVertices;n++)
	{
		if (obj->Vertices[n].temp==1) np++;
	}

	if ((np>0)&&(nf>0))
	{
		SetType(type);
		Init(np,nf*3);
		LockVertices();
		LockIndices();

		nn=0;
		for (n=0;n<obj->nVertices;n++)
		{
			if (obj->Vertices[n].temp==1)
			{
				if ((Type&API3D_VERTEXCALC)!=0) SetVertex(nn,obj->Vertices[n].Calc);
				else SetVertex(nn,obj->Vertices[n].Stok);

				if ((Type&API3D_TEXPERVERTEX)!=0) SetTexCoo(nn,obj->Vertices[n].Map);
				if (((Type&API3D_TEXPERVERTEX)!=0)&&(Type&API3D_TEXCOODATAS2)) SetTexCoo2(nn,obj->Vertices[n].Map2);

				if ((Type&API3D_NORMALDATAS)!=0)
				{
					if ((Type&API3D_VERTEXCALC)==1) SetNormal(nn,obj->Vertices[n].NormCalc);
					else SetNormal(nn,obj->Vertices[n].Norm);
				}

				if ((Type&API3D_COLORDATAS4)!=0) SetColor(nn,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b,obj->Vertices[n].Diffuse.a);
				else
				if ((Type&API3D_COLORDATAS)!=0) SetColor(nn,obj->Vertices[n].Diffuse.r,obj->Vertices[n].Diffuse.g,obj->Vertices[n].Diffuse.b);
				
				if (Type&API3D_BLENDING)
				{
					SetWeights(nn,0,obj->Vertices[n].Weight[0]);
					SetWeights(nn,1,obj->Vertices[n].Weight[1]);
					SetWeights(nn,2,obj->Vertices[n].Weight[2]);
					SetWeights(nn,3,obj->Vertices[n].Weight[3]);

					SetMatrixIndex(nn,0,obj->Vertices[n].Index[0]);
					SetMatrixIndex(nn,1,obj->Vertices[n].Index[1]);
					SetMatrixIndex(nn,2,obj->Vertices[n].Index[2]);
					SetMatrixIndex(nn,3,obj->Vertices[n].Index[3]);
				}

				corres[n]=nn;
				nn++;
			}
		}

		nn=0;
		for (n=0;n<obj->nFaces;n++)
		{
			f=&obj->Faces[n];
			if (f->tag==tag)
			{
				SetIndices(nn,corres[f->v0],corres[f->v1],corres[f->v2]);

				if ((Type&API3D_TEXPERVERTEX)==0)
				{
					if ((Type&API3D_TEXCOODATAS)!=0)
					{
						SetTexCoo(corres[f->v0],f->mp0);
						SetTexCoo(corres[f->v1],f->mp1);
						SetTexCoo(corres[f->v2],f->mp2);
					}
				}
				nn++;
			}
		}
		UnlockVertices();
		UnlockIndices();
	}

	free(corres);
}



/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		create dynamic degenerated edges vertex buffer of an object

		CreateDegeneratedEdgesDynamic(CObject3D * obj)

			API3D_EDGESDG specifies that shadow volume on stencil buffer will be drawn
			CorresVertices filled to enable update.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::CreateDegeneratedEdgesDynamic(CObject3D * obj)
{
	CreateDegeneratedEdgesDynamic(obj,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_EDGESDG);
}

int CDED_WitchEdge(EdgeList *Edges,int *corres,int a,int b)
{
	int n,res;

	res=-1;
	if (a!=b)
	{
		n=0;
		while ((n<Edges->nList)&&(res==-1))
		{
			if (((corres[Edges->List[n].a]==corres[a])&&(corres[Edges->List[n].b]==corres[b]))
				||((corres[Edges->List[n].a]==corres[b])&&(corres[Edges->List[n].b]==corres[a]))) res=n;
			else n++;
		}
	}
	return res;
}

bool CDED_Valid(CObject3D * obj,int a,int b,int *corres)
{
	int nf1,nf2;
	int nn;
	CVector N1,N2;

	nf1=-1;
	nn=0;
	while ((nn<obj->nFaces)&&(nf1==-1))
	{
		if ((corres[obj->Faces[nn].v0]==a)&&(corres[obj->Faces[nn].v1]==b)) nf1=nn;
		if ((corres[obj->Faces[nn].v1]==a)&&(corres[obj->Faces[nn].v2]==b)) nf1=nn;
		if ((corres[obj->Faces[nn].v2]==a)&&(corres[obj->Faces[nn].v0]==b)) nf1=nn;
		nn++;
	}

	nf2=-1;
	nn=0;
	while ((nn<obj->nFaces)&&(nf2==-1))
	{
		if ((corres[obj->Faces[nn].v0]==b)&&(corres[obj->Faces[nn].v1]==a)) nf2=nn;
		if ((corres[obj->Faces[nn].v1]==b)&&(corres[obj->Faces[nn].v2]==a)) nf2=nn;
		if ((corres[obj->Faces[nn].v2]==b)&&(corres[obj->Faces[nn].v0]==a)) nf2=nn;
		nn++;
	}

	if ((nf1>=0)&&(nf2>=0))
	{

		N1=obj->Faces[nf1].Norm;
		N2=obj->Faces[nf2].Norm;

		if ((N1||N2)>1.0f-SMALLF2) return false;
		else return true;


	}
	else return false;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		create degenerated edges vertex buffer of an object

		CreateDegeneratedEdgesDynamic(CObject3D * obj)

			API3D_EDGESDG specifies that shadow volume on stencil buffer will be drawn



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::CreateDegeneratedEdgesDynamic(CObject3D * obj,unsigned int flags)
{
	EdgeList Edges;
	int n,nn,nf1,nf2;
	int *corres;
	int a,b;
	CVector u,N;

	N.Init(0,-1,0);
	Edges.Init(obj->nFaces*3);

	corres=new int[obj->nVertices];

	for (n=0;n<obj->nVertices;n++)
	{
		corres[n]=n;

		for (nn=0;nn<n;nn++)
		{
			if (corres[nn]==nn)
			{
				u=obj->Vertices[n].Stok - obj->Vertices[nn].Stok;
				if (u.Norme()<SMALLF)
				{
					corres[n]=nn;
				}
			}
		}
	}

	for (n=0;n<obj->nFaces;n++)
	{
		a=obj->Faces[n].v0;
		b=obj->Faces[n].v1;
		if (CDED_WitchEdge(&Edges,corres,a,b)==-1)
		{
			if (CDED_Valid(obj,corres[a],corres[b],corres))
			{
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.nList++;
			}
		}

		a=obj->Faces[n].v1;
		b=obj->Faces[n].v2;
		if (CDED_WitchEdge(&Edges,corres,a,b)==-1)
		{
			if (CDED_Valid(obj,corres[a],corres[b],corres))
			{
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.nList++;
			}
		}

		a=obj->Faces[n].v2;
		b=obj->Faces[n].v0;
		if (CDED_WitchEdge(&Edges,corres,a,b)==-1)
		{
			if (CDED_Valid(obj,corres[a],corres[b],corres))
			{
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.nList++;
			}
		}
	}


	SetType(flags);
	if (flags&API3D_MORPH) Init(Edges.nList*4,Edges.nList*2*3,1);
	else Init(Edges.nList*4,Edges.nList*2*3);
	Grouped=false;

	CorresVertices=new int[Edges.nList*4];
	CorresVerticesNorm=new int[Edges.nList*4];

	LockVertices();

	for (n=0;n<Edges.nList;n++)
	{


		a=corres[Edges.List[n].a];
		b=corres[Edges.List[n].b];

		nf1=-1;
		nn=0;
		while ((nn<obj->nFaces)&&(nf1==-1))
		{
			if ((corres[obj->Faces[nn].v0]==a)&&(corres[obj->Faces[nn].v1]==b)) nf1=nn;
			if ((corres[obj->Faces[nn].v1]==a)&&(corres[obj->Faces[nn].v2]==b)) nf1=nn;
			if ((corres[obj->Faces[nn].v2]==a)&&(corres[obj->Faces[nn].v0]==b)) nf1=nn;
			nn++;
		}


		nf2=-1;
		nn=0;
		while ((nn<obj->nFaces)&&(nf2==-1))
		{
			if ((corres[obj->Faces[nn].v0]==b)&&(corres[obj->Faces[nn].v1]==a)) nf2=nn;
			if ((corres[obj->Faces[nn].v1]==b)&&(corres[obj->Faces[nn].v2]==a)) nf2=nn;
			if ((corres[obj->Faces[nn].v2]==b)&&(corres[obj->Faces[nn].v0]==a)) nf2=nn;
			nn++;
		}


		CorresVertices[n*4 + 0] = Edges.List[n].a;
		CorresVertices[n*4 + 1] = Edges.List[n].b;
		CorresVertices[n*4 + 2] = Edges.List[n].b;
		CorresVertices[n*4 + 3] = Edges.List[n].a;


		if (nf1>=0)
		{
			CorresVerticesNorm[n*4 + 0] = nf1;
			CorresVerticesNorm[n*4 + 1] = nf1;
		}
		else
		{
			CorresVerticesNorm[n*4 + 0] = -1-nf2;
			CorresVerticesNorm[n*4 + 1] = -1-nf2;
		}


		if (nf2>=0)
		{
			CorresVerticesNorm[n*4 + 2] = nf2;
			CorresVerticesNorm[n*4 + 3] = nf2;
		}
		else
		{
			CorresVerticesNorm[n*4 + 2] = -1-nf1;
			CorresVerticesNorm[n*4 + 3] = -1-nf1;
		}


		SetVertex(n*4+0,obj->Vertices[Edges.List[n].a].Stok);
		SetVertex(n*4+1,obj->Vertices[Edges.List[n].b].Stok);
		SetVertex(n*4+2,obj->Vertices[Edges.List[n].b].Stok);
		SetVertex(n*4+3,obj->Vertices[Edges.List[n].a].Stok);


		if (nf2>=0) N=-1*obj->Faces[nf2].Norm;
		if (nf1!=-1)
		{
			SetNormal(n*4+0,obj->Faces[nf1].Norm);
			SetNormal(n*4+1,obj->Faces[nf1].Norm);
		}
		else
		{
			SetNormal(n*4+0,N);
			SetNormal(n*4+1,N);
		}


		if (nf1>=0) N=-1*obj->Faces[nf1].Norm;
		if (nf2!=-1)
		{
			SetNormal(n*4+2,obj->Faces[nf2].Norm);
			SetNormal(n*4+3,obj->Faces[nf2].Norm);
		}
		else
		{
			SetNormal(n*4+2,N);
			SetNormal(n*4+3,N);
		}


		SetTexCoo(n*4+0,0,0);
		SetTexCoo(n*4+1,1,0);
		SetTexCoo(n*4+2,1,1);
		SetTexCoo(n*4+3,0,1);
	}

	UnlockVertices();


	LockIndices();
	for (n=0;n<Edges.nList;n++)
	{
		SetIndices(n*2+0,n*4+0,n*4+1,n*4+2);
		SetIndices(n*2+1,n*4+0,n*4+2,n*4+3);
	}
	UnlockIndices();

	Edges.Free();
	delete [] corres;

}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		create dynamic degenerated edges vertex buffer of an object

		CreateDegeneratedEdgesDynamicFacing(CObject3D * obj)

			API3D_EDGESDG specifies that shadow volume on stencil buffer will be drawn
			CorresVertices filled to enable update.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::CreateDegeneratedEdgesDynamicFacing(CObject3D * obj)
{
	CreateDegeneratedEdgesDynamicFacing(obj,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_EDGESDG);
}


void CVertexBuffer::CreateDegeneratedEdgesDynamicFacing(CObject3D * obj,unsigned int flags0)
{
	EdgeList Edges;
	int n,nn,nf1,nf2;
	int a,b;
	int ab,ba;
	unsigned int flags;
	int nv=obj->nVertices;
	int * hash=(int *) malloc(nv*nv*sizeof(int));
	for (int tt=0;tt<nv*nv;tt++) hash[tt]=-1;

	if (flags0&API3D_NOSECONDARY) flags=flags0-API3D_NOSECONDARY;
	else flags=flags0;

	CVector u,N,u1,u2,u3;

	N.Init(0,-1,0);
	Edges.Init(obj->nFaces*3*2);

	for (n=0;n<obj->nFaces;n++)
	{
		if ((obj->Faces[n].v0!=obj->Faces[n].v1)&&(obj->Faces[n].v0!=obj->Faces[n].v2)&&(obj->Faces[n].v1!=obj->Faces[n].v2))
		{
			a=obj->Faces[n].v0;
			b=obj->Faces[n].v1;
			ab=a+nv*b;
			ba=b+nv*a;
			if ((hash[ab]<0)&&(hash[ba]<0))
			{
				hash[ab]=n;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].tag=0;
				Edges.nList++;
			}
			else { if (hash[ab]<0) hash[ab]=n; }

			a=obj->Faces[n].v1;
			b=obj->Faces[n].v2;
			ab=a+nv*b;
			ba=b+nv*a;
			if ((hash[ab]<0)&&(hash[ba]<0))
			{
				hash[ab]=n;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].tag=0;
				Edges.nList++;
			}
			else { if (hash[ab]<0) hash[ab]=n; }

			a=obj->Faces[n].v2;
			b=obj->Faces[n].v0;
			ab=a+nv*b;
			ba=b+nv*a;
			if ((hash[ab]<0)&&(hash[ba]<0))
			{
				hash[ab]=n;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].tag=0;
				Edges.nList++;
			}
			else { if (hash[ab]<0) hash[ab]=n; }
		}
	}

	SetType(flags);

	if (flags&API3D_MORPH) Init(Edges.nList*4,Edges.nList*2*3,1);
	else Init(Edges.nList*4,Edges.nList*2*3);

	Grouped=false;

	CorresVertices=new int[Edges.nList*4];
	CorresVerticesNorm=new int[Edges.nList*4];

	LockVertices();
	for (n=0;n<Edges.nList;n++)
	{
		a=Edges.List[n].a;
		b=Edges.List[n].b;

		nf1=hash[a+nv*b];
		nf2=hash[b+nv*a];

		CorresVertices[n*4 + 0] = Edges.List[n].a;
		CorresVertices[n*4 + 1] = Edges.List[n].b;
		CorresVertices[n*4 + 2] = Edges.List[n].b;
		CorresVertices[n*4 + 3] = Edges.List[n].a;

		if ((nf1>=0)&&(nf2>=0))
		{
			CorresVerticesNorm[n*4 + 0] = nf1;
			CorresVerticesNorm[n*4 + 1] = nf1;
			CorresVerticesNorm[n*4 + 2] = nf2;
			CorresVerticesNorm[n*4 + 3] = nf2;

			SetTexCoo(n*4+0,1,0);
			SetTexCoo(n*4+1,1,0);
			SetTexCoo(n*4+2,1,0);
			SetTexCoo(n*4+3,1,0);
		}
		else
		{
			if (nf1>=0)
			{
				CorresVerticesNorm[n*4 + 0] = nf1;
				CorresVerticesNorm[n*4 + 1] = nf1;
				CorresVerticesNorm[n*4 + 2] = nf1;
				CorresVerticesNorm[n*4 + 3] = nf1;

				SetTexCoo(n*4+0,1,0);
				SetTexCoo(n*4+1,1,0);
				SetTexCoo(n*4+2,0,0);
				SetTexCoo(n*4+3,0,0);
			}
			else
			if (nf2>=0)
			{
				CorresVerticesNorm[n*4 + 0] = -1-nf2;
				CorresVerticesNorm[n*4 + 1] = -1-nf2;
				CorresVerticesNorm[n*4 + 2] = -1-nf2;
				CorresVerticesNorm[n*4 + 3] = -1-nf2;

				SetTexCoo(n*4+0,1,0);
				SetTexCoo(n*4+1,1,0);
				SetTexCoo(n*4+2,0,0);
				SetTexCoo(n*4+3,0,0);
			}
			else
			{
				CorresVerticesNorm[n*4 + 0] = 0;
				CorresVerticesNorm[n*4 + 1] = 0;
				CorresVerticesNorm[n*4 + 2] = 0;
				CorresVerticesNorm[n*4 + 3] = 0;

				SetTexCoo(n*4+0,0,0);
				SetTexCoo(n*4+1,0,0);
				SetTexCoo(n*4+2,0,0);
				SetTexCoo(n*4+3,0,0);
			}
		}

		SetVertex(n*4+0,obj->Vertices[Edges.List[n].a].Stok);
		SetVertex(n*4+1,obj->Vertices[Edges.List[n].b].Stok);
		SetVertex(n*4+2,obj->Vertices[Edges.List[n].b].Stok);
		SetVertex(n*4+3,obj->Vertices[Edges.List[n].a].Stok);

		if (flags&API3D_BLENDING)
		{
			for (int k=0;k<4;k++)
				for (int p=0;p<4;p++)
				{
					switch (k)
					{
					case 0:
						nn=Edges.List[n].a;
						break;
					case 1:
						nn=Edges.List[n].b;
						break;
					case 2:
						nn=Edges.List[n].b;
						break;
					case 3:
						nn=Edges.List[n].a;
						break;
					};
					SetWeights(n*4+k,p,obj->Vertices[nn].Weight[p]);
					SetMatrixIndex(n*4+k,p,obj->Vertices[nn].Index[p]);
				}
		}

		if (nf1!=-1)
		{
			SetNormal(n*4+0,obj->Faces[nf1].Norm);
			SetNormal(n*4+1,obj->Faces[nf1].Norm);
		}
		else
		{
			SetNormal(n*4+0,-obj->Faces[nf2].Norm.x,-obj->Faces[nf2].Norm.y,-obj->Faces[nf2].Norm.z);
			SetNormal(n*4+1,-obj->Faces[nf2].Norm.x,-obj->Faces[nf2].Norm.y,-obj->Faces[nf2].Norm.z);		// le -1* rajout�
		}


		if (nf2!=-1)
		{
			SetNormal(n*4+2,obj->Faces[nf2].Norm);
			SetNormal(n*4+3,obj->Faces[nf2].Norm);
		}
		else
		{
			SetNormal(n*4+2,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);
			SetNormal(n*4+3,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);
		}

	}
	UnlockVertices();

	free(hash);

	LockIndices();
	for (n=0;n<Edges.nList;n++)
	{
		SetIndices(n*2+0,n*4+0,n*4+2,n*4+1);	//inv
		SetIndices(n*2+1,n*4+0,n*4+3,n*4+2);	//inv
	}
	UnlockIndices();

	Edges.Free();

	if ((flags0&API3D_NOSECONDARY)==0)
	{
		secondary=new CVertexBuffer;

		secondary->Grouped=false;
		secondary->SetType(flags);
		if (flags&API3D_MORPH) secondary->Init(obj->nFaces*3,obj->nFaces*3,1);
		else secondary->Init(obj->nFaces*3,obj->nFaces*3);

		secondary->LockVertices();

		secondary->CorresVertices=new int[obj->nFaces*3];
		secondary->CorresVerticesNorm=new int[obj->nFaces*3];

		for (n=0;n<obj->nFaces;n++)
		{
			secondary->CorresVertices[n*3+0]=obj->Faces[n].v0;
			secondary->CorresVertices[n*3+1]=obj->Faces[n].v1;
			secondary->CorresVertices[n*3+2]=obj->Faces[n].v2;

			secondary->CorresVerticesNorm[n*3+0]=n;
			secondary->CorresVerticesNorm[n*3+1]=n;
			secondary->CorresVerticesNorm[n*3+2]=n;

			secondary->SetVertex(n*3+0,obj->Faces[n].v[0]->Stok);
			secondary->SetVertex(n*3+1,obj->Faces[n].v[1]->Stok);
			secondary->SetVertex(n*3+2,obj->Faces[n].v[2]->Stok);

			secondary->SetNormal(n*3+0,obj->Faces[n].Norm);
			secondary->SetNormal(n*3+1,obj->Faces[n].Norm);
			secondary->SetNormal(n*3+2,obj->Faces[n].Norm);

			secondary->SetTexCoo(n*3+0,1,1);
			secondary->SetTexCoo(n*3+1,1,1);
			secondary->SetTexCoo(n*3+2,1,1);

			if (flags&API3D_BLENDING)
			{
				for (int k=0;k<3;k++)
					for (int p=0;p<4;p++)
					{
						switch (k)
						{
						case 0:
							nn=obj->Faces[n].v0;
							break;
						case 1:
							nn=obj->Faces[n].v1;
							break;
						case 2:
							nn=obj->Faces[n].v2;
							break;
						};
						secondary->SetWeights(n*3+k,p,obj->Vertices[nn].Weight[p]);
						secondary->SetMatrixIndex(n*3+k,p,obj->Vertices[nn].Index[p]);
					}
			}
		}
		secondary->UnlockVertices();

		secondary->LockIndices();
		for (n=0;n<obj->nFaces;n++) secondary->SetIndices(n,n*3+0,n*3+1,n*3+2);
		secondary->UnlockIndices();

	}
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		create degenerated edges vertex buffer of an object

		CreateDegeneratedEdges(CObject3D * obj)

			API3D_EDGESDG specifies that shadow volume on stencil buffer will be drawn



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::CreateDegeneratedEdges(CObject3D * obj)
{
	CreateDegeneratedEdges(obj,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_EDGESDG);
}

void CVertexBuffer::CreateDegeneratedEdges(CObject3D * obj,unsigned int flags)
{
	EdgeList Edges;
	int n,nf1,nf2;


	Edges.Init(obj->nFaces*3);

	for (n=0;n<obj->nFaces;n++)
	{
		Edges.AddEdge(obj->Faces[n].v0,obj->Faces[n].v1);
		Edges.AddEdge(obj->Faces[n].v1,obj->Faces[n].v2);
		Edges.AddEdge(obj->Faces[n].v2,obj->Faces[n].v0);
	}

	SetType(flags);


	Init(Edges.nList*4,Edges.nList*2*3);

	Grouped=false;

	LockVertices();
	for (n=0;n<Edges.nList;n++)
	{
		nf1=EDGEN_WFace(obj,Edges.List[n].a,Edges.List[n].b);
		nf2=EDGEN_WFace(obj,Edges.List[n].b,Edges.List[n].a);

		SetVertex(n*4+0,obj->Vertices[Edges.List[n].a].Stok);
		SetVertex(n*4+1,obj->Vertices[Edges.List[n].b].Stok);

		SetVertex(n*4+2,obj->Vertices[Edges.List[n].b].Stok);
		SetVertex(n*4+3,obj->Vertices[Edges.List[n].a].Stok);

		SetNormal(n*4+0,obj->Faces[nf1].Norm);
		SetNormal(n*4+1,obj->Faces[nf1].Norm);
		SetNormal(n*4+2,obj->Faces[nf2].Norm);
		SetNormal(n*4+3,obj->Faces[nf2].Norm);

		SetTexCoo(n*4+0,0,0);
		SetTexCoo(n*4+1,1,0);
		SetTexCoo(n*4+2,1,1);
		SetTexCoo(n*4+3,0,1);
	}
	UnlockVertices();

	LockIndices();
	for (n=0;n<Edges.nList;n++)
	{
		SetIndices(n*2+0,n*4+0,n*4+1,n*4+2);
		SetIndices(n*2+1,n*4+0,n*4+2,n*4+3);
	}
	UnlockIndices();
	Edges.Free();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		create dynamic degenerated edges vertex buffer of an object

		CreateDegeneratedEdgesDynamicFacing(CObject3D * obj)

			API3D_EDGESDG specifies that shadow volume on stencil buffer will be drawn
			CorresVertices filled to enable update.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool FaceNormalePrecalcBase(CFace * f,CObject3D *obj);

void CVertexBuffer::CreateDegeneratedEdgesDynamicMerged(CObject3D * obj)
{
	CreateDegeneratedEdgesDynamicMerged(obj,API3D_VERTEXDATAS|API3D_NORMALDATAS|API3D_TEXCOODATAS|API3D_EDGESDG);
}

void CVertexBuffer::CreateDegeneratedEdgesDynamicMerged(CObject3D * obj,unsigned int flags)
{
	EdgeList Edges;
	int n,nf1,nf2,n3,n4;
	int nv=obj->nVertices;
	int a,b;
	int ab,ba;
	int snd;
	CVector u,N,u1,u2,u3;
	short int * hash=NULL;
	int SEUILNV=16384;

	if (nv<=0) return;
	if (obj->nFaces<=0) return;

	if (nv<SEUILNV)
	{
		hash=(short int *) malloc(nv*nv*sizeof(short int));
		if (hash) for (int tt=0;tt<nv*nv/2;tt++) ((int*)hash)[tt]=-1;
	}

	if (flags&API3D_NOSECONDARY) snd=0; else snd=1;

	N.Init(0,-1,0);
	Edges.Init(obj->nFaces*3*2);

	if (hash)
	{
		for (n=0;n<obj->nFaces;n++)
		{
			if ((obj->Faces[n].v0!=obj->Faces[n].v1)&&(obj->Faces[n].v0!=obj->Faces[n].v2)&&(obj->Faces[n].v1!=obj->Faces[n].v2))
			{
				a=obj->Faces[n].v0;
				b=obj->Faces[n].v1;
				ab=a+nv*b;
				ba=b+nv*a;
				if ((hash[ab]<0)&&(hash[ba]<0))
				{
					hash[ab]=n;
					Edges.List[Edges.nList].a=a;
					Edges.List[Edges.nList].b=b;
					Edges.List[Edges.nList].tag=0;
					Edges.nList++;
				}
				else { if (hash[ab]<0) hash[ab]=n; }

				a=obj->Faces[n].v1;
				b=obj->Faces[n].v2;
				ab=a+nv*b;
				ba=b+nv*a;
				if ((hash[ab]<0)&&(hash[ba]<0))
				{
					hash[ab]=n;
					Edges.List[Edges.nList].a=a;
					Edges.List[Edges.nList].b=b;
					Edges.List[Edges.nList].tag=0;
					Edges.nList++;
				}
				else { if (hash[ab]<0) hash[ab]=n; }

				a=obj->Faces[n].v2;
				b=obj->Faces[n].v0;
				ab=a+nv*b;
				ba=b+nv*a;
				if ((hash[ab]<0)&&(hash[ba]<0))
				{
					hash[ab]=n;
					Edges.List[Edges.nList].a=a;
					Edges.List[Edges.nList].b=b;
					Edges.List[Edges.nList].tag=0;
					Edges.nList++;
				}
				else { if (hash[ab]<0) hash[ab]=n; }
			}
			
		}
	}
	else
	{
		
		for (n=0;n<obj->nFaces;n++)
		{
			int e;

			a=obj->Faces[n].v0; b=obj->Faces[n].v1;
			e=Edges.WitchEdge(a,b);

			if (e>=0) Edges.List[e].nf2=n;
			else
			{
				Edges.List[Edges.nList].ab=a+b;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].nf1=n;
				Edges.List[Edges.nList].nf2=-1;
				Edges.nList++;
			}

			a=obj->Faces[n].v1; b=obj->Faces[n].v2;
			e=Edges.WitchEdge(a,b);

			if (e>=0) Edges.List[e].nf2=n;
			else
			{
				Edges.List[Edges.nList].ab=a+b;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].nf1=n;
				Edges.List[Edges.nList].nf2=-1;
				Edges.nList++;
			}

			a=obj->Faces[n].v2; b=obj->Faces[n].v0;
			e=Edges.WitchEdge(a,b);

			if (e>=0) Edges.List[e].nf2=n;
			else
			{
				Edges.List[Edges.nList].ab=a+b;
				Edges.List[Edges.nList].a=a;
				Edges.List[Edges.nList].b=b;
				Edges.List[Edges.nList].nf1=n;
				Edges.List[Edges.nList].nf2=-1;
				Edges.nList++;
			}
		}

	}

	SetType(flags);
	if (flags&API3D_MORPH)
	{
		if (flags&API3D_STREAMS) Init(Edges.nList*4 + obj->nFaces*3*snd,(Edges.nList*2 + obj->nFaces*snd)*3,obj->nKeys);
		else Init(Edges.nList*4 + obj->nFaces*3*snd,(Edges.nList*2 + obj->nFaces*snd)*3,1);
	}
	else Init(Edges.nList*4 + obj->nFaces*3*snd,(Edges.nList*2 + obj->nFaces*snd)*3);

	Grouped=false;

	if (flags&API3D_ENABLEUPDATE)
	{
		CorresVertices=new int[Edges.nList*4 + obj->nFaces*3*snd];
		CorresVerticesNorm=new int[Edges.nList*4 + obj->nFaces*3*snd];
	}

	LockVertices();
	n4=0;
	for (n=0;n<Edges.nList;n++)
	{
		a=Edges.List[n].a;
		b=Edges.List[n].b;

		if (hash)
		{
			nf1=hash[a+nv*b];
			nf2=hash[b+nv*a];

			Edges.List[n].nf1=nf1;
			Edges.List[n].nf2=nf2;

		}
		else
		{
			/*
			nf1=EDGEN_WFace(obj,Edges.List[n].a,Edges.List[n].b);
			nf2=EDGEN_WFace(obj,Edges.List[n].b,Edges.List[n].a);
			/**/

			nf1=Edges.List[n].nf1;
			nf2=Edges.List[n].nf2;
		}

		if (flags&API3D_ENABLEUPDATE)
		{
			CorresVertices[n4 + 0] = a;
			CorresVertices[n4 + 1] = b;
			CorresVertices[n4 + 2] = b;
			CorresVertices[n4 + 3] = a;
		}

		if ((nf1>=0)&&(nf2>=0))
		{
			SetTexCoo(n4+0,1,0);
			SetTexCoo(n4+1,1,0);
			SetTexCoo(n4+2,1,0);
			SetTexCoo(n4+3,1,0);
		}
		else
		{
			SetTexCoo(n4+0,1,0);
			SetTexCoo(n4+1,1,0);
			SetTexCoo(n4+2,0,0);
			SetTexCoo(n4+3,0,0);
		}

		SetVertex(n4+0,obj->Vertices[a].Stok);
		SetVertex(n4+1,obj->Vertices[b].Stok);
		SetVertex(n4+2,obj->Vertices[b].Stok);
		SetVertex(n4+3,obj->Vertices[a].Stok);

		if (flags&API3D_BLENDING)
		{
			#define storeWI(k,p,i) {  SetWeights(n4+k,p,obj->Vertices[i].Weight[p]); SetMatrixIndex(n4+k,p,obj->Vertices[i].Index[p]); } 
			storeWI(0,0,a) storeWI(0,1,a) storeWI(0,2,a) storeWI(0,3,a)
			storeWI(1,0,b) storeWI(1,1,b) storeWI(1,2,b) storeWI(1,3,b)
			storeWI(2,0,b) storeWI(2,1,b) storeWI(2,2,b) storeWI(2,3,b)
			storeWI(3,0,a) storeWI(3,1,a) storeWI(3,2,a) storeWI(3,3,a)
		}

		if (flags&API3D_ENABLEUPDATE)
		{
			CorresVerticesNorm[n4 + 0] = nf1;
			CorresVerticesNorm[n4 + 1] = nf1;
		}
		SetNormal(n4+0,obj->Faces[nf1].Norm);
		SetNormal(n4+1,obj->Faces[nf1].Norm);

		if (nf2!=-1)
		{
			if (flags&API3D_ENABLEUPDATE)
			{
				CorresVerticesNorm[n4 + 2] = nf2;
				CorresVerticesNorm[n4 + 3] = nf2;
			}

			SetNormal(n4+2,obj->Faces[nf2].Norm);
			SetNormal(n4+3,obj->Faces[nf2].Norm);
		}
		else
		{
			if (flags&API3D_ENABLEUPDATE)
			{
				CorresVerticesNorm[n4 + 2] = -1-nf1;
				CorresVerticesNorm[n4 + 3] = -1-nf1;
			}

			SetNormal(n4+2,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);
			SetNormal(n4+3,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);

		}

		n4+=4;
	}

	int start=secondary_vertex_pos=Edges.nList*4;

	if (snd)
	{
		n3=0;
		for (n=0;n<obj->nFaces;n++)
		{
			if (flags&API3D_ENABLEUPDATE)
			{
				CorresVertices[start+n3+0]=obj->Faces[n].v0;
				CorresVertices[start+n3+1]=obj->Faces[n].v1;
				CorresVertices[start+n3+2]=obj->Faces[n].v2;

				CorresVerticesNorm[start+n3+0]=-1-n;
				CorresVerticesNorm[start+n3+1]=-1-n;
				CorresVerticesNorm[start+n3+2]=-1-n;
			}

			SetVertex(start+n3+0,obj->Faces[n].v[0]->Stok);
			SetVertex(start+n3+1,obj->Faces[n].v[1]->Stok);
			SetVertex(start+n3+2,obj->Faces[n].v[2]->Stok);

			SetNormal(start+n3+0,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);
			SetNormal(start+n3+1,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);
			SetNormal(start+n3+2,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);

			SetTexCoo(start+n3+0,1,1);
			SetTexCoo(start+n3+1,1,1);
			SetTexCoo(start+n3+2,1,1);

			if (flags&API3D_BLENDING)
			{
				#define storeWIF(k,p,i) {  SetWeights(start+n3+k,p,obj->Vertices[i].Weight[p]); SetMatrixIndex(start+n3+k,p,obj->Vertices[i].Index[p]); } 

				storeWIF(0,0,obj->Faces[n].v0) storeWIF(0,1,obj->Faces[n].v0) storeWIF(0,2,obj->Faces[n].v0) storeWIF(0,3,obj->Faces[n].v0)
				storeWIF(1,0,obj->Faces[n].v1) storeWIF(1,1,obj->Faces[n].v1) storeWIF(1,2,obj->Faces[n].v1) storeWIF(1,3,obj->Faces[n].v1)
				storeWIF(2,0,obj->Faces[n].v2) storeWIF(2,1,obj->Faces[n].v2) storeWIF(2,2,obj->Faces[n].v2) storeWIF(2,3,obj->Faces[n].v2)
			}

			n3+=3;
		}
	}

	UnlockVertices();

	if (flags&API3D_STREAMS)
	{
		for (n=0;n<obj->nVertices;n++)
		{
			obj->Vertices[n].Calc=obj->Vertices[n].Stok;
			obj->Vertices[n].NormCalc=obj->Vertices[n].Norm;
		}

		for (n=0;n<obj->nFaces;n++)
			obj->Faces[n].NormCalc=obj->Faces[n].Norm;

		for (int k=0;k<obj->nKeys;k++)
		{
			LockStream(k);

			for (n=0;n<obj->nVertices;n++)
				obj->Vertices[n].Stok=obj->VerticesKeys[k][n].Stok;

			for (n=0;n<obj->nFaces;n++) FaceNormalePrecalcBase(&obj->Faces[n],obj);

			n4=0;
			for (n=0;n<Edges.nList;n++)
			{
				a=Edges.List[n].a;
				b=Edges.List[n].b;

				nf1=Edges.List[n].nf1;
				nf2=Edges.List[n].nf2;

				SetVertex(n4+0,obj->Vertices[a].Stok);
				SetVertex(n4+1,obj->Vertices[b].Stok);
				SetVertex(n4+2,obj->Vertices[b].Stok);
				SetVertex(n4+3,obj->Vertices[a].Stok);
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20)
				if ((nf1>=0)&&(nf2>=0))
				{
					SetTexCoo(n4+0,1,0); SetTexCoo(n4+1,1,0); SetTexCoo(n4+2,1,0); SetTexCoo(n4+3,1,0);
				}
				else
				{
					SetTexCoo(n4+0,1,0); SetTexCoo(n4+1,1,0); SetTexCoo(n4+2,0,0); SetTexCoo(n4+3,0,0);
				}
#endif
				SetNormal(n4+0,obj->Faces[nf1].Norm);
				SetNormal(n4+1,obj->Faces[nf1].Norm);

				if (nf2!=-1)
				{
					SetNormal(n4+2,obj->Faces[nf2].Norm);
					SetNormal(n4+3,obj->Faces[nf2].Norm);
				}
				else
				{
					SetNormal(n4+2,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);
					SetNormal(n4+3,-obj->Faces[nf1].Norm.x,-obj->Faces[nf1].Norm.y,-obj->Faces[nf1].Norm.z);
				}
				n4+=4;
			}

			if (snd)
			{
				n3=0;
				for (n=0;n<obj->nFaces;n++)
				{
					SetVertex(start+n3+0,obj->Faces[n].v[0]->Stok);
					SetVertex(start+n3+1,obj->Faces[n].v[1]->Stok);
					SetVertex(start+n3+2,obj->Faces[n].v[2]->Stok);

					SetNormal(start+n3+0,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);
					SetNormal(start+n3+1,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);
					SetNormal(start+n3+2,-obj->Faces[n].Norm.x,-obj->Faces[n].Norm.y,-obj->Faces[n].Norm.z);
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20)
					SetTexCoo(start+n3+0,1,1);
					SetTexCoo(start+n3+1,1,1);
					SetTexCoo(start+n3+2,1,1);
#endif
					n3+=3;
				}
			}

			UnlockStream();
		}

		for (n=0;n<obj->nVertices;n++)
		{
			obj->Vertices[n].Stok=obj->Vertices[n].Calc;
			obj->Vertices[n].Norm=obj->Vertices[n].NormCalc;
		}

		for (n=0;n<obj->nFaces;n++)
			obj->Faces[n].Norm=obj->Faces[n].NormCalc;

	}

	if (hash) free(hash);

	LockIndices();
	for (n=0;n<Edges.nList;n++)
	{
		int n2=n<<1;
		int n4=n<<2;
		SetIndices(n2+0,n4+0,n4+2,n4+1);
		SetIndices(n2+1,n4+0,n4+3,n4+2);
	}

	if (snd)
	{
		int add=Edges.nList*2;
		secondary_pos=add;
		for (n=0;n<obj->nFaces;n++)
		{
			int n3=(n<<1) + n;
			SetIndices(add +n,start+n3+0,start+n3+2,start+n3+1);
		}
	}
	UnlockIndices();

	Edges.Free();
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

		Vertex Program Support

		loading:
			char* CVertexBuffer::SetVertexProgram(char * script)

			returns error

		dynamic variable set:

  			void setMatrixArrayElement(char *name,int ndx,CMatrix M);
			void setMatrix(char *name,CMatrix M);
			void setVectorArrayElement(char *name,int ndx,CVector v);
			void setVector(char *name,CVector v);
			void setDword(char *name,unsigned int d);
			void setFloat(char *name,float f);
			void setMatrixWorldViewProj(char *name);
			void setMatrixWorld(char *name);
			void setMatrixViewProj(char *name);
			void setTexture(char *name,int n);
			void setPoint(char *name,CVector v);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
#ifndef GLES
bool ProgramStringIsNative(GLenum target, GLenum format,GLsizei len, GLvoid *string)
{
#ifdef OPENGL_VERTEXPROGRAMS
	glProgramStringARB(target, format, len, string);

	GLint errorPos, isNative;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
	if ((errorPos == -1) && (isNative == 1)) return true;
	else return false;
#else
	return true;
#endif
}
#endif
#endif

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  functions: add line numbers to error message

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

char str_vb_TemporaryString[1024];

char * str_vb_parse(char * script, int *pos)
{
	int n, len;

	if (script[*pos] == '\0')
	{
		return NULL;
	}
	else
	{
		n = *pos;
		while ((script[n] != 0xa) && (script[n] != '\0')) n++;
		len = n - (*pos);
		memcpy(str_vb_TemporaryString, &script[*pos], len);
		str_vb_TemporaryString[len] = '\0';
		if (script[n] == '\0') *pos = n;
		else *pos = n + 1;

		return str_vb_TemporaryString;
	}
}

char * addlineinfo(char * text, int tag)
{
	int line = 1;
	char num[5];
	int size = 0;
	int p = 0;
	static char buf[32768*4];
	char *str = str_vb_parse(text, &p);
	while (str)
	{
		if (tag)
		{
			if (line < 10) sprintf(num, "000%d", line);
			else
			if (line < 100) sprintf(num, "00%d", line);
			else
			if (line < 1000) sprintf(num, "0%d", line);
			else sprintf(num, "%d", line);

			buf[size++] = num[0];
			buf[size++] = num[1];
			buf[size++] = num[2];
			buf[size++] = num[3];

			buf[size++] = '\t';
		}

		memcpy(&buf[size], str, strlen(str));
		size += strlen(str);
		buf[size++] = 0xd;
		buf[size++] = 0xa;

		line++;
		str = str_vb_parse(text, &p);
	}

	char * res = (char*)malloc(size + 1);
	memcpy(res, buf, size);
	res[size] = '\0';
	return res;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:	SetVertexProgram
	generate vs and ps 4.0 automatically for Direct3D10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_upcase(char * str);
void str_clean(char * str);
char * str_parse_rl(char * script,int *pos);
bool str_match(char * str,char * m);
char * str_apostrophes(char * str);
int str_char(char * str, char c);
char * str_apostrophes(char * str);
extern int VPLINEINSCRIPT;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlusMoinsEgalCase(char * error,char *error2)
{
	int n,p;
	char op1[1024];
	char op2[1024];
	int len=strlen(error);

	p=str_char(error,'=');
	if (p>0)
	{
		sprintf(op1,error);
		op1[p]='\0';
		n=0;
		while (((error[p+1+n]!='+')&&(error[p+1+n]!='-'))&&(n+p+1<len)) n++;
		sprintf(op2,&error[p+1]);
		op2[n]='\0';

		if (strcmp(op1,op2)==0)
		{
			sprintf(error2,"%s%c=%s",op1,error[p+1+n],&error[p+1+n+1]);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LineDebugReadShader(char * script,char * err)
{
	char two[1024];
	char prev[1024];
	char error[1024];
	char error2[1024];
	int pos=0;
	int rl=1;
	int p;

	VPLINEINSCRIPT=0;
	sprintf(error,"%s",err);
	sprintf(error2,"%s",err);
	str_upcase(error);
	str_upcase(error2);
	sprintf(prev,"");
	
	PlusMoinsEgalCase(error,error2);

	char *tmp=str_parse_rl(script,&pos);
	while (tmp)
	{
		str_clean(tmp);
		str_upcase(tmp);
		p=str_char(tmp,'#');
		if (p>=0) tmp[p]='\0';

		sprintf(two,"%s%s",prev,tmp);
		
		if (str_match(tmp,error)) return VPLINEINSCRIPT-1;
		if (str_match(tmp,error2)) return VPLINEINSCRIPT-1;
		if (str_match(two,error)) return VPLINEINSCRIPT-1-rl;
		if (strlen(tmp)>0) { sprintf(prev,tmp); rl=1; }
		else rl++;
		tmp=str_parse_rl(script,&pos);
	}

	return -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LineDebugReadShaderXX(char * script,char * err,int ln)
{
	char error[1024];
	char error2[1024];	
	int pos=0;
	int p;

	if (script==NULL) return -1;

	VPLINEINSCRIPT=0;
	sprintf(error,"%s",err);
	sprintf(error2,"%s",err);
	str_upcase(error);
	str_upcase(error2);
	
	PlusMoinsEgalCase(error,error2);

	char *tmp=str_parse_rl(script,&pos);
	while (tmp)
	{
		if (VPLINEINSCRIPT>ln)
		{
			str_clean(tmp);
			str_upcase(tmp);
			p=str_char(tmp,'#');
			if (p>=0) tmp[p]='\0';
			if (str_match(tmp,error)) return VPLINEINSCRIPT-1;
			if (str_match(tmp,error2)) return VPLINEINSCRIPT-1;
		}
		tmp=str_parse_rl(script,&pos);
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LineDebugReadShader2(char * script,char * err,int pass)
{
	char error[1024];
	char error2[1024];
	int pos=0;
	int p;
	int ps=-1;

	if (script==NULL) return -1;

	VPLINEINSCRIPT=0;
	sprintf(error,"%s",err);
	sprintf(error2,"%s",err);
	str_upcase(error);
	str_upcase(error2);

	PlusMoinsEgalCase(error,error2);

	char *tmp=str_parse_rl(script,&pos);
	while (tmp)
	{
		str_clean(tmp);
		str_upcase(tmp);
		if (str_match(tmp,"PASS")) ps++;
		p=str_char(tmp,'#');
		if (p>=0) tmp[p]='\0';
		p=str_char(tmp,'=');
		if (p>=0) tmp[p]='\0';
		if (ps==pass)
		{
			if (str_match(tmp,error)) return VPLINEINSCRIPT-1;
			if (str_match(tmp,error2)) return VPLINEINSCRIPT-1;
		}
		tmp=str_parse_rl(script,&pos);
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetNumberLines(char * script)
{
	int pos=0;
	int codeline=0;
	char *tmp=str_parse_rl(script,&pos);
	while (tmp)
	{
		codeline++;
		tmp=str_parse_rl(script,&pos);
	}
	return codeline;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetNumberLinesFrom(char * script,int poz)
{
	int pos=0;
	int codeline=0;
	char *tmp=str_parse_rl(script,&pos);
	while (tmp)
	{
		codeline++;
		if (codeline>poz)
		{
			if (str_char(tmp,'}')>=0) return codeline;
		}
		
		tmp=str_parse_rl(script,&pos);
	}
	return codeline;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LineBug(char *bug)
{
	int line=-1;
	int par=str_char(bug,'(');
	int virg=str_char(bug,',');

	if ((par>=0)&&(virg>=0))
	{
		if (virg>par)
		{
			char ss[128];
			memcpy(ss,&bug[par+1],virg-par-1);
			ss[virg-par]='\0';
			sscanf(ss,"%d",&line);
			return line;
		}
		else return -1;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LineDebugCompileVP(char * script,CVertexProgram * vp,int line,char *basescript)
{
	int res=-1;
	int linepass=0;
	char m[1024];
	int end=GetNumberLines(basescript);
	int poz;

	for (int p=0;p<vp->NPASS;p++)
	{
		vp->vsfncodeline[vp->numbervsfn[p]][p]=vp->vscodeline[p];
		vp->psfncodeline[vp->numberpsfn[p]][p]=vp->pscodeline[p];
	}

	if (vp->numbervsfn[0]==0) poz=vp->vscodeline[0];
	else poz=vp->vsfncodeline[0][0];

	for (int p=0;p<vp->NPASS;p++)
	{
		int endpass=0;
		if (p==vp->NPASS-1) endpass=end;
		else endpass=GetNumberLinesFrom(basescript,poz);

		if (vp->pscodeline[p]<vp->vscodeline[p]) vp->pscodeline[p]=endpass;
		poz=endpass;

		vp->vsfncodeline[vp->numbervsfn[p]][p]=vp->vscodeline[p];
		vp->psfncodeline[vp->numberpsfn[p]][p]=vp->pscodeline[p];


		if (line>vp->vsfncodeline[0][p])
		{
			if (line<vp->vscodeline[p])
			{
				// vs function pass p			
							
				for (int n=0;n<vp->numbervsfn[p];n++)
				{
					if ((line>vp->vsfncodeline[n][p])&&(line<vp->vsfncodeline[n+1][p]))
					{
						sprintf(m,vp->vsfnname[n][p]);
						return LineDebugReadShader2(script,m,p);
					}
				}
			}
			else
			if (line<vp->psfncodeline[0][p])
			{
				// vs pass p
				sprintf(m,"[PASS%d]",p);
				return LineDebugReadShader2(script,m,p);
			}
			else
			if (line<vp->pscodeline[p])
			{
				// ps function pass p
								
				for (int n=0;n<vp->numberpsfn[p];n++)
				{
					if ((line>vp->psfncodeline[n][p])&&(line<vp->psfncodeline[n+1][p]))
					{
						sprintf(m,vp->psfnname[n][p]);
						return LineDebugReadShader2(script,m,p);
					}
				}
			}
			if (line<endpass)
			{
				// ps pass p
				sprintf(m,"[SHADER]");
				return LineDebugReadShader2(script,m,p);
			}
		}
	}

	return res;
}

#if (defined(GLESFULL)&&(!defined(IOS))&&(!defined(WEBASM)))&&defined(NOGLSLBIN)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char NED_DIRECTORY_ANDROID[1024]={ 'N','E','D',0 };
#endif

#if (defined(GLESFULL)&&(!defined(IOS))&&(!defined(WEBASM)))&&!defined(NOGLSLBIN)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char NED_DIRECTORY_ANDROID[1024]={ 'N','E','D',0 };
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * nameGLSL=NULL;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void downcase(char *ss)
{
	for (int n=0;n<(int)strlen(ss);n++)
	{
		if ((ss[n]>='A')&&(ss[n]<='Z')) ss[n]=ss[n]+'a'-'A';
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLSLSetFileName(char *name)
{
	nameGLSL=name;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if (!defined(GOOGLEVR))||defined(ES2)
PFNGLGETPROGRAMBINARYOESPROC glGetProgramBinaryOES=NULL;
PFNGLPROGRAMBINARYOESPROC glProgramBinaryOES=NULL;
#else
#define glGetProgramBinaryOES glGetProgramBinary
#define glProgramBinaryOES glProgramBinary
#define GL_PROGRAM_BINARY_LENGTH_OES GL_PROGRAM_BINARY_LENGTH
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLSLSaveBinary(GLuint newProgram,int pass)
{
	GLint   binaryLength;
	void*   binary;
	FILE*   outfile;
	char 	fname[1024];
	GLenum	binaryFormat;
#if (!defined(GOOGLEVR))||defined(ES2)
	if (glGetProgramBinaryOES==NULL) glGetProgramBinaryOES=(PFNGLGETPROGRAMBINARYOESPROC) eglGetProcAddress("glGetProgramBinaryOES");
#endif
	if ((nameGLSL)&&(glGetProgramBinaryOES))
	{
		char tmp[1024];
		strcpy(tmp,nameGLSL);
		downcase(tmp);

		sprintf(fname,"%s/GLSL/%s_%d.compiled",NED_DIRECTORY_ANDROID,tmp,pass);

		glGetProgramiv(newProgram, GL_PROGRAM_BINARY_LENGTH_OES, &binaryLength);
		binary = (void*)malloc(binaryLength);
		glGetProgramBinaryOES(newProgram, binaryLength, &binaryLength, &binaryFormat, binary);

		outfile = fopen(fname, "wb");
		if (outfile)
		{
            fwrite(&binaryFormat,sizeof(GLenum),1,outfile);
            fwrite(&binaryLength,sizeof(GLint),1,outfile);
            fwrite(binary, binaryLength, 1, outfile);
            fclose(outfile);
		}
        free(binary);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GLSLLoadBinary(GLuint newProgram,int pass)
{
	GLint   binaryLength;
	void*   binary;
	GLint   success;
	FILE*   infile;
	char 	fname[1024];
	GLenum	binaryFormat;
#if (!defined(GOOGLEVR))||defined(ES2)
	if (glProgramBinaryOES==NULL) glProgramBinaryOES=(PFNGLPROGRAMBINARYOESPROC) eglGetProcAddress("glProgramBinaryOES");
#endif
	if ((nameGLSL)&&(glProgramBinaryOES))
	{
		char tmp[1024];
		strcpy(tmp,nameGLSL);
		downcase(tmp);

		sprintf(fname,"%s/GLSL/%s_%d.compiled",NED_DIRECTORY_ANDROID,tmp,pass);

		infile = fopen(fname, "rb");
		if (infile)
		{
			fread(&binaryFormat,sizeof(GLenum),1,infile);
			fread(&binaryLength,sizeof(GLint),1,infile);
			binary = (void*)malloc(binaryLength);
			fread(binary, binaryLength, 1, infile);
			fclose(infile);

			glProgramBinaryOES(newProgram, binaryFormat, binary, binaryLength);
			free(binary);

            GLint res;
            glGetProgramiv(newProgram,GL_LINK_STATUS,&res);
            if (res==GL_TRUE) return true;

			return false;
		}
	}

	return false;;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *suppressparameters(char *str);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ErrorManagement(CVertexProgram *vp,char *script,char *debug)
{
	if (vp->SYNTAXERRORFN)
	{
		vp->linedebug=LineDebugReadShader(script,vp->ERRORSTR);
		sprintf(debug,"Semi colon error in braces in line : %d : \"%s\"",vp->linedebug,suppressparameters(vp->ERRORSTR));
	}
	else
	if (vp->REPERROR)
	{
		vp->linedebug=LineDebugReadShader(script,vp->ERRORSTRREP);
		sprintf(debug,"Loop rep/endrep error in line : %d : \"%s\"",vp->linedebug,vp->ERRORSTRREP);
	}
	else
	if (vp->PARAMERROR)
	{
		vp->linedebug=LineDebugReadShader(script,vp->PRMERRORSTR);
		sprintf(debug,"Parameters error in line : %d : \"%s\" %s",vp->linedebug,vp->PRMERRORSTR,vp->ERRORSTRPARAM);
	}
    else
    if (vp->IFERROR)
    {
        vp->linedebug=LineDebugReadShader(script,vp->ERRORSTR);
        sprintf(debug,"Incorrect condition error in line : %d : %s",vp->linedebug,vp->ERRORSTR);
    }
	else
	{
        if (vp->RSERROR)
        {
            vp->linedebug=LineDebugReadShader(script,"RENDER_STATE");
            sprintf(debug,"Syntax error on '%s' render states in line : %d",vp->RSERRORSTR,vp->linedebug);
        }
        else
        {
            vp->linedebug=LineDebugReadShader(script,vp->ERRORSTR);
            if ((vp->SYNTAXERROR)&&(vp->DEFINEERROR))
            {
                if ((str_char(vp->ERRORSTRPARAM,'(')>=0)||(str_char(vp->ERRORSTRPARAM,')')>=0))
                {
                    char ss[256];
                    strcpy(ss,suppressparameters(vp->ERRORSTRPARAM));
                    sprintf(debug,"Undefined function in line : %d : \"%s\" unknown \"%s\"",vp->linedebug,vp->ERRORSTR,ss);
                }
                else
                {
                    if (str_char(vp->ERRORSTRPARAM,',')>=0) sprintf(debug,"Problem with parameters in line : %d : \"%s\" unknown \"%s\"",vp->linedebug,vp->ERRORSTR,vp->ERRORSTRPARAM);
                    else sprintf(debug,"Variable undefined in line : %d : \"%s\" unknown \"%s\"",vp->linedebug,vp->ERRORSTR,vp->ERRORSTRPARAM);
                }
            }
            else
            {
                if (vp->DEFINEERROR) sprintf(debug,"Something undefined in line : %d : \"%s\" unknown \"%s\"",vp->linedebug,vp->ERRORSTR,vp->ERRORSTRPARAM);
                else
                {
                    if (vp->UNUSEDERROR)
                    {
                        vp->linedebug=LineDebugReadShader(script,vp->UNUSEDRRORSTR);
                        sprintf(debug,"Deprecated unused variable in line : %d : \"%s\" obsolete : \"%s\"",vp->linedebug,suppressparameters(vp->UNUSEDRRORSTR),vp->ERRORSTRPARAM);
                    }
                    else
                    {
                        if (vp->SYNTAXERROR) sprintf(debug,"Syntax error in line : %d : \"%s\"",vp->linedebug,vp->ERRORSTR);
                        else sprintf(debug,"Syntax error on modifiers in line : %d : \"%s\"",vp->linedebug,vp->ERRORSTR);
                    }
                }
            }
        }
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexBuffer::ResetVariablesVertexProgram()
{
    int n,k;
    char *name;
    CVector4 v(0.0f,0.0f,0.0f,0.0f);
    CMatrix M;
    
    for (n=0;n<vp->last_constant;n++)
    {
        if (vp->constants[n])
        {
            if (vp->translate_constants[n]==NULL)
            {
                name=vp->constants[n];
                
                int nb=0;
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                
                if ((vp->l_constants[n]&0xffff)==_MATRIX)
                {
                    if (nb>1)
                    {
                        for (k=0;k<nb;k++) setMatrixArrayElement(name,k,M);
                    }
                    else setMatrix(name,M);
                }

                if ((vp->l_constants[n]&0xffff)==_VECTOR)
                {
                    if (nb>1)
                    {
                        for (k=0;k<nb;k++) setVector4ArrayElement(name,k,v);
                    }
                    else setVector4(name,v);
                }
            }
        }
    }
    
    for (n=0;n<vp->npsvectors;n++)
    {
        name=vp->psvectors[n];
        setVector4(name,v);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CVertexBuffer::SetVertexProgram(char * script)
{
    return SetVertexProgram(script,-1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CVertexBuffer::SetVertexProgram(char * script,int ext)
{
	unsigned int flags=0;
    int size_script;
    
    if (TheClass3DAPI->AutomaticallyCloneShaders)
    {
        size_script=strlen(script);
        CShaderString *ss=IsSameShader(script,size_script);
        if (ss)
        {
            CloneVertexProgram(ss->fx);
			ss->list.Add(this);
            return NULL;
        }
    }
    
#ifndef API3D_OPENGL20
	unsigned int len;
#endif
	if (G_Multitexture) flags|=MULTITEXTURING_ENABLED;
	if (G_Dot3) flags|=DOT3PRODUCT_ENABLED;
	if (G_PS) flags|=PIXELSHADER_ENABLED;
	if (G_PS2X) flags|=PIXELSHADER2X_ENABLED;

#ifdef API3D_METAL
    if (!vp)
    {
        vp=new CVertexProgram;
        vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;
        
        if (!vp->read_shader(script,METAL|MULTITEXTURING_ENABLED|DOT3PRODUCT_ENABLED|PIXELSHADER_ENABLED|PIXELSHADER2X_ENABLED))
        {
            char *debug;
            debug=(char*) malloc(128);
            ErrorManagement(vp,script,debug);
            LOG(debug);
            return debug;
        }

        FX=MTLSetProgramVertexBuffer(this);
        
        if (FX==-1)
        {
            LOG(script);
            return "ERROR SHADER";
        }
        
        value_coef=vp->isValueCoef();
        
        int n;
        Env *e;
        int tab[256];
        
        for (n=0;n<256;n++) { tab[n]=0; roots[n]=NULL; }
        
        for (n=0;n<vp->last_constant;n++)
        {
            if (vp->constants[n])
            {
                if (vp->translate_constants[n]==NULL)
                {
                    int c=vp->constants[n][0];
                    tab[c]++;
                }
            }
        }
        
        for (n=0;n<vp->npsvectors;n++)
        {
            int c=vp->psvectors[n][0];
            tab[c]++;
        }
        
        for (n=0;n<vp->last_constant;n++)
        {
            if (vp->constants[n])
            {
                int c=vp->constants[n][0];

                int nb=0;
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;

                if (nb>1)
                {
                    if ((vp->l_constants[n]&0xffff)==_MATRIX)
					{
	                    palette=vp->listenv.InsertLast();
		                strcpy(palette->name,vp->constants[n]);
						palette->type=TYPE_MATRIXLIST;
					}
				}

                if (tab[c]==1)
                {
                    roots[c]=e=vp->listenv.InsertLast();
                    strcpy(e->name,vp->constants[n]);
                                        
                    if (nb>1)
                    {
                        if ((vp->l_constants[n]&0xffff)==_MATRIX) e->type=TYPE_MATRIXLIST;
                        if ((vp->l_constants[n]&0xffff)==_VECTOR) e->type=TYPE_VECTORLIST;
                    }
                    else
                    {
                        if ((vp->l_constants[n]&0xffff)==_MATRIX) e->type=TYPE_MATRIX;
                        if ((vp->l_constants[n]&0xffff)==_VECTOR) e->type=TYPE_VECTOR;
                    }
                }
            }
        }
    }
    
    if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
    
#endif
    
#ifdef API3D_DIRECT3D
	char *str;
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (!vp)
	{
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		LOG("READ SHADER");
		vp->read_shader(script,DIRECT3D|flags);

		LOG("READ SHADER OK");

		str=vp->get_string_vsh(0);
		len=strlen(str);

		effect=NULL;

		//LPD3DXBUFFER errors;
		//if (FAILED(D3DXCreateEffect( D3DDevice, str, len, &effect, &errors) ) )
		if (FAILED(D3DXCreateEffect( D3DDevice, str, len, &effect, NULL) ) )
		{
			LOG("D3DXCE");
			LOG(str);
		//	LOG((char*)errors->GetBufferPointer());
			return "ERROR Create Effect";
		}

		if (effect)
		{
			effect->GetTechniqueDesc("T0",&tdesc);
			effect->SetTechnique(tdesc.Index);
		}
		else
		{
			LOG(str);
			LOG("ERROR effect=NULL");
			return "ERROR effect=NULL";
		}

		free(str);
	}
#endif

#ifdef API3D_DIRECT3D11
	char *str;
	char *debug;
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (vp==NULL)
	{
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		effect=NULL;

		flags=0;
		flags|=MULTITEXTURING_ENABLED;
		flags|=DOT3PRODUCT_ENABLED;
		flags|=PIXELSHADER_ENABLED;
		flags|=PIXELSHADER2X_ENABLED;

		if (!vp->read_shader(script,DIRECT3D|D3D11|flags))
		{
			debug=(char*) malloc(1024);
			ErrorManagement(vp,script,debug);
			LOG(debug);
			return debug;
		}
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (render->DEFER_VP) return NULL;
	}

	if (effect==NULL)
	{
#ifndef LIB3D_OWNED_FX11
		str=vp->get_string_vsh(0);
		len=strlen(str);

		ID3DBlob* errors=NULL;
		if (FAILED(D3DX11CompileEffectFromMemory(str,len,NULL,NULL,NULL,
			D3DCOMPILE_PREFER_FLOW_CONTROL*vp->flow|LIB3D_V2_MAIN_NOT_OPTIM*D3DCOMPILE_SKIP_OPTIMIZATION|D3DCOMPILE_PARTIAL_PRECISION*0,	// HLSL Flags
			0,	// FX Flags
			D3DDevice, &effect, &errors)))
#else
		str=vp->get_string_vsh(0);
		len=strlen(str);
		ID3DBlob* errors=NULL;

		if (D3DX11CompileEffectFromVP(vp,NULL,NULL,NULL,
			D3DCOMPILE_PREFER_FLOW_CONTROL*vp->flow|LIB3D_V2_MAIN_NOT_OPTIM*D3DCOMPILE_SKIP_OPTIMIZATION|D3DCOMPILE_PARTIAL_PRECISION*0,	// HLSL Flags
			0,	// FX Flags
			D3DDevice, &effect, &errors)!=S_OK)
#endif
		{
			debug = (char*)malloc(errors->GetBufferSize() + 1);
			memcpy(debug, (char*)errors->GetBufferPointer(), errors->GetBufferSize());
			debug[errors->GetBufferSize()] = '\0';

			if (str_match(debug,"subscript"))
			{
				char * err=str_apostrophes(debug);
				int ln=LineBug(debug);
				ln=LineDebugCompileVP(script,vp,ln,str);
				vp->linedebug=LineDebugReadShaderXX(script,err,ln);
			}
			else
			{
				int ln=LineBug(debug);
				vp->linedebug=LineDebugCompileVP(script,vp,ln,str);
			}

			char *tmp1;
			char *tmp2;

			if (debug) tmp1 = addlineinfo(debug, 0); else tmp1="";
			if (str) tmp2 = addlineinfo(str, 1); else tmp2="";

			free(debug);

			debug = (char*)malloc(strlen(tmp1) + strlen(tmp2) + 128);
			sprintf(debug,"Error in line (owner script) : %d\n",vp->linedebug);
			sprintf(debug,"%s%s\n", debug,tmp1);
			sprintf(debug,"%s%s\n",debug,tmp2);

			LOG(debug);

			if (debug) free(tmp1);
			if (str) free(tmp2);

			return debug;
		}

		if (effect)
		{
			tech=effect->GetTechniqueByName("T0");
		}
		else
		{
			debug=(char*)malloc(strlen("ERROR"));
			sprintf(debug,"ERROR");
			return debug;
		}
        
        value_coef=vp->isValueCoef();
		
		int n;
		int tab[256];

		for (n=0;n<256;n++) tab[n]=0;
                
		for (n=0;n<vp->last_constant;n++)
		{
			if (vp->constants[n])
			{
				if (vp->translate_constants[n]==NULL)
				{
					int c=vp->constants[n][0];
					tab[c]++;
				}
			}
		}

		for (n=0;n<vp->npsvectors;n++)
		{
			int c=vp->psvectors[n][0];
			tab[c]++;
		}

		for (n=0;n<vp->last_constant;n++)
		{
			if (vp->constants[n])
			{
				int c=vp->constants[n][0];
				if (tab[c]==1)
				{                            
					int nb=0;
					if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;

					if ((vp->l_constants[n]&0xffff)==_MATRIX) matrices[c]=effect->GetVariableByName(vp->constants[n])->AsMatrix();
					if ((vp->l_constants[n]&0xffff)==_VECTOR) vectors[c]=effect->GetVariableByName(vp->constants[n])->AsVector();
				}
			}
		}

		palette=effect->GetVariableByName("PALETTE")->AsMatrix();

		free(str);
        
        if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
	}
#endif

#ifdef API3D_DIRECT3D12
	char *str;
	char *debug;

	C3DAPIBASE *render=(C3DAPIBASE*) capi;
	
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (vp==NULL)
	{
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;
		effect=NULL;

		flags=0;
		flags|=MULTITEXTURING_ENABLED;
		flags|=DOT3PRODUCT_ENABLED;
		flags|=PIXELSHADER_ENABLED;
		flags|=PIXELSHADER2X_ENABLED;

		if (!vp->read_shader(script,DIRECT3D|D3D11|flags))
		{
			debug=(char*) malloc(1024);
			ErrorManagement(vp,script,debug);
			LOG(debug);
			return debug;
		}
		
		if (render->DEFER_VP) return NULL;
	}

	if (effect==NULL)
	{
		str=vp->get_string_vsh(0);
		len=strlen(str);
		ID3DBlob* errors=NULL;

		if (D3DFXCompileEffectFromVP(vp,NULL,NULL,NULL,
			0,	// HLSL Flags
			0,	// FX Flags
			dhhc, &effect, &errors)!=S_OK)
		{
			debug = (char*)malloc(errors->GetBufferSize() + 1);
			memcpy(debug, (char*)errors->GetBufferPointer(), errors->GetBufferSize());
			debug[errors->GetBufferSize()] = '\0';

			if (str_match(debug,"subscript"))
			{
				char * err=str_apostrophes(debug);
				int ln=LineBug(debug);
				ln=LineDebugCompileVP(script,vp,ln,str);
				vp->linedebug=LineDebugReadShaderXX(script,err,ln);
			}
			else
			{
				int ln=LineBug(debug);
				vp->linedebug=LineDebugCompileVP(script,vp,ln,str);
			}

			char *tmp1;
			char *tmp2;

			if (debug) tmp1 = addlineinfo(debug, 0); else tmp1="";
			if (str) tmp2 = addlineinfo(str, 1); else tmp2="";

			free(debug);

			debug = (char*)malloc(strlen(tmp1) + strlen(tmp2) + 128);
			sprintf(debug,"Error in line (owner script) : %d\n",vp->linedebug);
			sprintf(debug,"%s%s\n", debug,tmp1);
			sprintf(debug,"%s%s\n",debug,tmp2);

			LOG(debug);

			if (debug) free(tmp1);
			if (str) free(tmp2);

			return debug;
		}

		if (effect)
		{
			tech=effect->GetTechniqueByName("T0");

			render->VBVP.Add(this);
		}
		else
		{
			debug=(char*)malloc(strlen("ERROR"));
			sprintf(debug,"ERROR");
			return debug;
		}
        
        value_coef=vp->isValueCoef();
		
		int n;
		int tab[256];

		for (n=0;n<256;n++) tab[n]=0;
                
		for (n=0;n<vp->last_constant;n++)
		{
			if (vp->constants[n])
			{
				if (vp->translate_constants[n]==NULL)
				{
					int c=vp->constants[n][0];
					tab[c]++;
				}
			}
		}

		for (n=0;n<vp->npsvectors;n++)
		{
			int c=vp->psvectors[n][0];
			tab[c]++;
		}

		for (n=0;n<vp->last_constant;n++)
		{
			if (vp->constants[n])
			{
				int c=vp->constants[n][0];
				if (tab[c]==1)
				{                            
					int nb=0;
					if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;

					if ((vp->l_constants[n]&0xffff)==_MATRIX) matrices[c]=effect->GetVariableByName(vp->constants[n])->AsMatrix();
					if ((vp->l_constants[n]&0xffff)==_VECTOR) vectors[c]=effect->GetVariableByName(vp->constants[n])->AsVector();
				}
			}
		}

		palette=effect->GetVariableByName("PALETTE")->AsMatrix();

		free(str);
        
        if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
	}
#endif

#ifdef API3D_DIRECT3D10
	char *str;
	char *debug;
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (vp==NULL)
	{
		effect=NULL;

		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		flags=0;
		flags|=MULTITEXTURING_ENABLED;
		flags|=DOT3PRODUCT_ENABLED;
		flags|=PIXELSHADER_ENABLED;
		flags|=PIXELSHADER2X_ENABLED;

		if (!vp->read_shader(script,DIRECT3D|D3D10|flags))
		{
			debug=(char*) malloc(128);
			ErrorManagement(vp,script,debug);
			LOG(debug);
			return debug;
		}

		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (render->DEFER_VP) return NULL;
	}

	if (effect==NULL)
	{
		str=vp->get_string_vsh(0);
		len=strlen(str);
		
		ID3D10Blob* errors=NULL;
		if (FAILED(D3DX10CreateEffectFromMemory(str,len,NULL, NULL, NULL, "fx_4_0",
			D3D10_SHADER_PREFER_FLOW_CONTROL*vp->flow|D3D10_SHADER_PARTIAL_PRECISION*0|D3D10_SHADER_ENABLE_STRICTNESS,
			0, D3DDevice, NULL, NULL, &effect, &errors, NULL)))
		{
			debug = (char*)malloc(errors->GetBufferSize() + 1);
			memcpy(debug, (char*)errors->GetBufferPointer(), errors->GetBufferSize());
			debug[errors->GetBufferSize()] = '\0';

			if (str_match(debug,"subscript"))
			{
				char * err=str_apostrophes(debug);
				int ln=LineBug(debug);
				ln=LineDebugCompileVP(script,vp,ln,str);
				vp->linedebug=LineDebugReadShaderXX(script,err,ln);
			}
			else
			{
				int ln=LineBug(debug);
				vp->linedebug=LineDebugCompileVP(script,vp,ln,str);
			}

			char *tmp1;
			char *tmp2;

			if (debug) tmp1 = addlineinfo(debug, 0); else tmp1="";
			if (str) tmp2 = addlineinfo(str, 1); else tmp2="";

			free(debug);

			debug = (char*)malloc(strlen(tmp1) + strlen(tmp2) + 128);
			sprintf(debug,"Error in line (owner script) : %d\n",vp->linedebug);
			sprintf(debug,"%s%s\n", debug,tmp1);
			sprintf(debug,"%s%s\n",debug,tmp2);

			LOG(debug);

			if (debug) free(tmp1);
			if (str) free(tmp2);

			return debug;
		}

		if (effect)
		{
			tech=effect->GetTechniqueByName("T0");
		}
		else
		{
			debug=(char*)malloc(strlen("ERROR"));
			sprintf(debug,"ERROR");
			return debug;
		}

		free(str);
        
        value_coef=vp->isValueCoef();
        
        if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
	}
#endif

#ifdef API3D_DIRECT3D9
	char *str;
	D3DXHANDLE h;
	char *debug;
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (vp==NULL)
	{
		effect=NULL;
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		if (!vp->read_shader(script,DIRECT3D|D3D9|flags))
		{
			debug=(char*) malloc(128);
			ErrorManagement(vp,script,debug);
			LOG(debug);
			return debug;
		}

		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (render->DEFER_VP) return NULL;
	}

	if (effect==NULL)
	{
		str=vp->get_string_vsh(0);
		len=strlen(str);

		LPD3DXBUFFER errors=NULL;
		// D3DXSHADER_PREFER_FLOW_CONTROL*vp->flow
		// D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_SKIPVALIDATION
		// D3DXSHADER_AVOID_FLOW_CONTROL | D3DXSHADER_NO_PRESHADER | D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_SKIPVALIDATION
		if (FAILED(D3DXCreateEffect(D3DDevice, str, len, NULL, NULL, D3DXSHADER_PREFER_FLOW_CONTROL*vp->flow|D3DXSHADER_IEEE_STRICTNESS|D3DXSHADER_PARTIALPRECISION*0|D3DXSHADER_SKIPOPTIMIZATION, NULL, &effect, &errors)))
		{
			debug = (char*)malloc(errors->GetBufferSize() + 1);
			memcpy(debug, (char*)errors->GetBufferPointer(), errors->GetBufferSize());
			debug[errors->GetBufferSize()] = '\0';

			if (str_match(debug,"subscript"))
			{
				char * err=str_apostrophes(debug);
				int ln=LineBug(debug);
				ln=LineDebugCompileVP(script,vp,ln,str);
				vp->linedebug=LineDebugReadShaderXX(script,err,ln);
			}
			else
			{
				int ln=LineBug(debug);
				vp->linedebug=LineDebugCompileVP(script,vp,ln,str);
			}

			char *tmp1;
			char *tmp2;

			if (debug) tmp1 = addlineinfo(debug, 0); else tmp1="";
			if (str) tmp2 = addlineinfo(str, 1); else tmp2="";

			free(debug);

			debug = (char*)malloc(strlen(tmp1) + strlen(tmp2) + 128);
			sprintf(debug,"Error in line (owner script) : %d\n",vp->linedebug);
			sprintf(debug,"%s%s\n", debug,tmp1);
			sprintf(debug,"%s%s\n",debug,tmp2);

			LOG(debug);

			if (debug) free(tmp1);
			if (str) free(tmp2);

			return debug;
		}

		if (effect)
		{
			h=effect->GetTechniqueByName("T0");
			effect->SetTechnique(h);
		}
		else
		{
			debug=(char*)malloc(strlen("ERROR"));
			sprintf(debug,"ERROR");
			return debug;
		}
        
        value_coef=vp->isValueCoef();

		free(str);
        
        if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
	}
#endif

#if defined(API3D_OPENGL)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int p;
	GLint errorPos;
	bool error_fp=false;
	char *debug;
	char *s2="?";
	int n;

#ifdef OPENGL_GLSL
	flags=MULTITEXTURING_ENABLED|DOT3PRODUCT_ENABLED|PIXELSHADER_ENABLED|PIXELSHADER2X_ENABLED;
#else
	flags=0;
	if (G_Multitexture) flags|=MULTITEXTURING_ENABLED;
	if (G_Dot3) flags|=DOT3PRODUCT_ENABLED;

	if (G_VertexProgram)
	{
		if (G_PS) flags|=PIXELSHADER_ENABLED;
	}

	if (G_PS2X) flags|=PIXELSHADER2X_ENABLED;
#endif

	if (!vp)
	{
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		if (G_VertexProgram)
		{
			if (!vp->read_shader(script,OPENGL|flags))
			{
				debug=(char*) malloc(128);
				ErrorManagement(vp,script,debug);
				LOG(debug);
				return debug;
			}
			else
			{
				if (vp->shadermodel)
				{
#ifdef OPENGL_GLSL
					for (p=0;p<vp->get_pass_count();p++)
					{
						str_programs[p]=vp->get_string_vsh(p);
						str_fprograms[p]=vp->get_string_psh(p);
/*
						LOG("Vertex:");
						LOG(str_programs[p]);
						LOG("Fragment:");
						LOG(str_fprograms[p]);
/**/
						glslprograms[p] = glCreateProgramObjectARB();

						int ok=0;
						int ok2=0;

						vert_shader[p] = glCreateShaderObjectARB(GL_VERTEX_SHADER);
						glShaderSourceARB(vert_shader[p],1,(const GLcharARB**)&str_programs[p],0);
						glCompileShaderARB(vert_shader[p]);
						glGetObjectParameterivARB(vert_shader[p], GL_OBJECT_COMPILE_STATUS_ARB, &ok);
						if (ok) glAttachObjectARB(glslprograms[p],vert_shader[p]);
  
						if (str_fprograms[p])
						{
							frag_shader[p] = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
							glShaderSourceARB(frag_shader[p],1,(const GLcharARB**)&str_fprograms[p],0);
							glCompileShaderARB(frag_shader[p]);
							glGetObjectParameterivARB(frag_shader[p], GL_OBJECT_COMPILE_STATUS_ARB, &ok2);
							if (ok2) glAttachObjectARB(glslprograms[p],frag_shader[p]);
						}
						else ok2=1;
  
						glLinkProgramARB(glslprograms[p]);
						
						if ((!ok)||(!ok2))
						{
							char str1[8192];
							char str2[8192];
							char str3[8192];

							glGetInfoLogARB(glslprograms[p], sizeof(str1), 0, str1);
							glGetInfoLogARB(vert_shader[p], sizeof(str2), 0, str2);
							glGetInfoLogARB(frag_shader[p], sizeof(str3), 0, str3);

							char *tmp1;
							char *tmp2;

							if (str_programs[p]) tmp1 = addlineinfo(str_programs[p], 1); else tmp1="";
							if (str_fprograms[p]) tmp2 = addlineinfo(str_fprograms[p], 1); else tmp2="";

							debug = (char*)malloc(strlen(tmp1)+strlen(tmp2)+strlen(str1)+strlen(str2)+strlen(str3)+128);

							sprintf(debug, "Vertex:%c%c%s\n%s\nFragment:%c%c%s\n%s\nProgram:\n%s\n",13,10, tmp1, str2,13,10, tmp2, str3, str1);
							LOG(debug);

							if (str_programs[p]) free(tmp1);
							if (str_fprograms[p]) free(tmp2);

							return debug;

						}
						
						glValidateProgramARB(glslprograms[p]);
					}
#endif
				}
				else
				{
#ifdef OPENGL_VERTEXPROGRAMS
					glGenProgramsARB(vp->get_pass_count(),programs);

					for (p=0;p<vp->get_pass_count();p++)
					{
						fp[p]=false;

						glEnable(GL_VERTEX_PROGRAM_ARB);
						glBindProgramARB(GL_VERTEX_PROGRAM_ARB,programs[p]);

						str_programs[p]=vp->get_string_vsh(p);
						str_fprograms[p]=vp->get_string_psh(p);

						len=strlen(str_programs[p]);

						glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,len,(const void*)str_programs[p]);
						glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

						if (glGetError()==GL_INVALID_OPERATION)
						{
							glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
							for (n=0;n<6;n++)
							{
								if (errorPos>=vp->ptr_groups[n]) s2=VP_STR_ERROR[n];
							}

							LOG(str_programs[p]);
							sprintf(s_error,"Error: Technique:%d Pass:%d Group:%s",vp->selected_tech,p,s2);

							glDisable(GL_VERTEX_PROGRAM_ARB);
							return s_error;

						}
						else free(str_programs[p]);

						glDisable(GL_VERTEX_PROGRAM_ARB);

						if (str_fprograms[p])
						{
							
							glEnable(GL_FRAGMENT_PROGRAM_ARB);

							len=strlen(str_fprograms[p]);
							glGenProgramsARB(1,&fprograms[p]);
							glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fprograms[p]);
							len=strlen(str_fprograms[p]);

							glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,len,(const void*)str_fprograms[p]);
							GLint errorPos, isNative;
							glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
							glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);

							if ((errorPos == -1) && (isNative == 1)) fp[p]=true;
							else
							{
								LOG("ERROR FRAGMENT PROGRAM");
								LOG(str_fprograms[p]);

								error_fp=true;
								glDeleteProgramsARB(1,&fprograms[p]);

							}

							free(str_fprograms[p]);
							glDisable(GL_FRAGMENT_PROGRAM_ARB);
						}
					}
#endif
				}
			}
		}
		else
		{
			if (!vp->read_shader(script,EMULATED|flags)) return vp->str_error;
		}
	}

#endif

#if defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int p;
	bool error_fp=false;
	char * debug;
	char *s2="?";

	flags=MULTITEXTURING_ENABLED|DOT3PRODUCT_ENABLED|PIXELSHADER_ENABLED|PIXELSHADER2X_ENABLED;

	if (!vp)
	{
		vp=new CVertexProgram;
		vp->reset(ext);

		vp->FSRSample0=fsr0; vp->FSRSample1=fsr1;

		if (!vp->read_shader(script,OPENGL|flags))
		{
			debug=(char*) malloc(128);
			ErrorManagement(vp,script,debug);
			LOG(debug);
			return debug;
		}
		else
		{
			if (vp->shadermodel)
			{

#ifdef OPENGL_GLSL

				for (p=0;p<vp->get_pass_count();p++)
				{
					str_programs[p]=vp->get_string_vsh(p);
					str_fprograms[p]=vp->get_string_psh(p);
					glslprograms[p] = glCreateProgramObjectARB();
#if (defined(GLESFULL)&&(!defined(IOS))&&(!defined(WEBASM)))&&!defined(NOGLSLBIN)
					if (!GLSLLoadBinary(glslprograms[p],p))
					{
#endif

//						LOG(str_programs[p]);
//						LOG(str_fprograms[p]);

						int ok=0;
						int ok2=0;
						vert_shader[p] = glCreateShaderObjectARB(GL_VERTEX_SHADER);
						glShaderSourceARB(vert_shader[p],1,(const GLcharARB**)&str_programs[p],0);
						glCompileShaderARB(vert_shader[p]);
						glGetObjectParameterivARB(vert_shader[p], GL_OBJECT_COMPILE_STATUS_ARB, &ok);
						if (ok) glAttachObjectARB(glslprograms[p],vert_shader[p]);
	  
						if (str_fprograms[p])
						{
							frag_shader[p] = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
							glShaderSourceARB(frag_shader[p],1,(const GLcharARB**)&str_fprograms[p],0);
							glCompileShaderARB(frag_shader[p]);
							glGetObjectParameterivARB(frag_shader[p], GL_OBJECT_COMPILE_STATUS_ARB, &ok2);
							if (ok2) glAttachObjectARB(glslprograms[p],frag_shader[p]);
						}
						else ok2=1;
#ifdef IOS
                        int ok3=0;
                        glLinkProgramARB(glslprograms[p]);
                        GLint res;
                        glGetProgramiv(glslprograms[p],GL_LINK_STATUS,&res);
                        if (res==GL_TRUE) ok3=1;
#else
                        glLinkProgramARB(glslprograms[p]);
                        int ok3=1;
#endif
                        
						if ((!ok)||(!ok2)||(!ok3))
						{
							char str1[8192];
							char str2[8192];
							char str3[8192];
#ifdef IOS
                            if ((ok3==0)&&(ok)&&(ok2))
                            {
                                strcpy(str1,"LINK ERROR");

                                int len,maxlen;
                                glGetProgramiv(glslprograms[p],GL_INFO_LOG_LENGTH,&maxlen);
                                char *log=new char[maxlen];
                                glGetProgramInfoLog(glslprograms[p],maxlen,&len,log);
                                debug = (char*)malloc(strlen(str1)+len+128);
                                
                                sprintf(debug, "%s%c%c%s\n",str1,13,10, log);
                                LOG(debug);

                                delete [] log;
                            }
                            else
#endif
                            {
                                glGetInfoLogARB(glslprograms[p], sizeof(str1), 0, str1);
                                glGetInfoLogARB(vert_shader[p], sizeof(str2), 0, str2);
                                glGetInfoLogARB(frag_shader[p], sizeof(str3), 0, str3);

                                char *tmp1;
                                char *tmp2;
                                
                                if (str_programs[p]) tmp1 = addlineinfo(str_programs[p], 1); else tmp1="";
                                if (str_fprograms[p]) tmp2 = addlineinfo(str_fprograms[p], 1); else tmp2="";
                                
                                debug = (char*)malloc(strlen(tmp1)+strlen(tmp2)+strlen(str1)+strlen(str2)+strlen(str3)+128);
                                
                                sprintf(debug, "Vertex:%c%c%s\n%s\nFragment:%c%c%s\n%s\nProgram:\n%s\n",13,10, tmp1, str2,13,10, tmp2, str3, str1);
                                LOG(debug);
                                
                                if (str_programs[p]) free(tmp1);
                                if (str_fprograms[p]) free(tmp2);

                            }
	
							return debug;
						}
						
						glValidateProgramARB(glslprograms[p]);

#if (defined(GLESFULL)&&(!defined(IOS))&&(!defined(WEBASM)))&&!defined(NOGLSLBIN)
						GLSLSaveBinary(glslprograms[p],p);

					}
#endif
				}
                
                value_coef=vp->isValueCoef();
               
                int n;
                Env *e;
                int tab[256];
                
                for (n=0;n<256;n++) { tab[n]=0; roots[n]=NULL; }
                
                for (n=0;n<vp->last_constant;n++)
                {
                    if (vp->constants[n])
                    {
                        if (vp->translate_constants[n]==NULL)
                        {
                            int c=vp->constants[n][0];
                            tab[c]++;
                        }
                    }
                }

				for (n=0;n<vp->npsvectors;n++)
				{
					int c=vp->psvectors[n][0];
					tab[c]++;
				}

                for (n=0;n<vp->last_constant;n++)
                {
                    if (vp->constants[n])
                    {
                        int c=vp->constants[n][0];
                        if (tab[c]==1)
                        {
                            roots[c]=e=vp->listenv.InsertLast();
                            strcpy(e->name,vp->constants[n]);
                            
                            int nb=0;
                            if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;

                            if (nb>1)
                            {
                                if ((vp->l_constants[n]&0xffff)==_MATRIX) e->type=TYPE_MATRIXLIST;
                                if ((vp->l_constants[n]&0xffff)==_VECTOR) e->type=TYPE_VECTORLIST;
                            }
                            else
                            {
                                if ((vp->l_constants[n]&0xffff)==_MATRIX) e->type=TYPE_MATRIX;
                                if ((vp->l_constants[n]&0xffff)==_VECTOR) e->type=TYPE_VECTOR;
                            }
                        }
                    }
                }
                
                if (TheClass3DAPI->AutomaticallyCloneShaders) RegisterShader(script,size_script,this);
#endif
			}
		}
	}

#endif

	Type|=API3D_VERTEXPROGRAM;
	return NULL;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setMatrix(char *name,CMatrix const &M)
{
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);

#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);

#else
#ifdef API3D_DIRECT3D10
		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }
		ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
		D3DXMATRIX mat;
		mat._11=M.a[0][0];	mat._12=M.a[0][1];	mat._13=M.a[0][2];	mat._14=M.a[0][3];
		mat._21=M.a[1][0];	mat._22=M.a[1][1];	mat._23=M.a[1][2];	mat._24=M.a[1][3];
		mat._31=M.a[2][0];	mat._32=M.a[2][1];	mat._33=M.a[2][2];	mat._34=M.a[2][3];
		mat._41=M.a[3][0];	mat._42=M.a[3][1];	mat._43=M.a[3][2];	mat._44=M.a[3][3];
		effect->SetMatrix( name, &mat );
#endif
#endif
#endif
	}

#endif
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------
    int n;
    int i,j;
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        
        n=0;
        for (j=0;j<4;j++)
            for (i=0;i<4;i++)
                e->m[n++]=M.a[j][i];
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            n=0;
            for (j=0;j<4;j++)
                for (i=0;i<4;i++)
                    e->m[n++]=M.a[j][i];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_MATRIX;
        e->modif=true;
        for (n=0;n<16;n++) e->m[n]=M.a[n>>2][n&3];
    }
    
#endif
    
    
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int nn,res;
	GLfloat m[16];
	int n;
	int i,j;
	int nb;

	modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL		
        
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;

			n=0;
			for (j=0;j<4;j++)
				for (i=0;i<4;i++)
					 e->m[n++]=M.a[j][i];
            return;
        }
        
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				n=0;
				for (j=0;j<4;j++)
					for (i=0;i<4;i++)
						 e->m[n++]=M.a[j][i];
                return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIX;
			e->modif=true;
			for (n=0;n<16;n++) e->m[n]=m[n];
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]==_MATRIX) nn++;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			glMatrixMode(GL_MATRIX0_ARB+res);
			glLoadIdentity();
			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];
			glLoadMatrixf(m);
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,M);
		}
	}

#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


void CVertexBuffer::setMatrixArrayElement(char *name,int ndx,CMatrix const &M)
{
    
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int n,nn,res;
	float m[16];
	int i,j;
	int nb;

	modifparams=true;

	for (i=0;i<4;i++)
		for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];

	if (palette)
    {
        palette->modif=true;
        for (n=0;n<16;n++) palette->mm[ndx*16 + n]=m[n];
        if (palette->nb<ndx) palette->nb=ndx;
        return;
    }

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
        if (e->nb<ndx) e->nb=ndx;
        return;
    }

    e=vp->listenv.GetFirst();
	while (e)
	{
		if (strcmp(e->name,name)==0)
		{
			e->modif=true;
			for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
			if (e->nb<ndx) e->nb=ndx;
			return;
		}
		e=vp->listenv.GetNext();
	}

	if (!e)
	{		
		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->type=TYPE_MATRIXLIST;
		for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
		if (e->nb<ndx) e->nb=ndx;
		e->modif=true;
	}
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12

		int res,n,nb;

		if (palette)
		{
			palette->SetMatrixArray((float*)&M,ndx,1);
			return;
		}

		int c=name[0]&255;
		if (matrices[c])
		{
			matrices[c]->SetMatrixArray((float*)&M,ndx,1);
			return;
		}

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
			if (var) var->SetMatrixArray((float*)&M,ndx,1);
		}

#else
#ifdef API3D_DIRECT3D11

		int res,n,nb;

		if (palette)
		{
			palette->SetMatrixArray((float*)&M,ndx,1);
			return;
		}

		int c=name[0]&255;
		if (matrices[c])
		{
			matrices[c]->SetMatrixArray((float*)&M,ndx,1);
			return;
		}

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
			if (var) var->SetMatrixArray((float*)&M,ndx,1);
		}

#else
#ifdef API3D_DIRECT3D10
		int res,n,nb;

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrixArray((float*)&M,ndx,1); return; }

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
			if (var) var->SetMatrixArray((float*)&M,ndx,1);
		}
#else
		int res,n,nn,nb;
		char temp[128];
		static D3DXMATRIX mats[256];

		D3DXMATRIX mat;
		mat._11=M.a[0][0];	mat._12=M.a[0][1];	mat._13=M.a[0][2];	mat._14=M.a[0][3];
		mat._21=M.a[1][0];	mat._22=M.a[1][1];	mat._23=M.a[1][2];	mat._24=M.a[1][3];
		mat._31=M.a[2][0];	mat._32=M.a[2][1];	mat._33=M.a[2][2];	mat._34=M.a[2][3];
		mat._41=M.a[3][0];	mat._42=M.a[3][1];	mat._43=M.a[3][2];	mat._44=M.a[3][3];

		if (vp->shadermodel3)
		{
			static D3DXMATRIX mats[256];

			strcpy(temp,name);

			res=-1;
			n=0;
			nn=0;
			while ((n<vp->last_constant)&&(res==-1))
			{
				if (strcmp(name,vp->constants[n])==0) res=n;
				else
				{
					if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
					n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
				}
			}

			if (res>=0)
			{
				nb=vp->l_constants[res]>>16;
				effect->GetMatrixArray( temp, mats, nb );
				mats[ndx]=mat;
				effect->SetMatrixArray( temp, mats , nb);
			}
		}
		else
		{
			sprintf(temp,"%s%d",name,ndx);	
			effect->SetMatrix( temp, &mat );
		}
#endif
#endif
#endif
	}

#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	GLfloat m[16];
	int i,j;
	int nb;

	char temp[128];
	sprintf(temp,"%s%d",name,ndx);

	modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL		
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];

        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
            if (e->nb<ndx) e->nb=ndx;
            return;
        }

        e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
				if (e->nb<ndx) e->nb=ndx;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIXLIST;
			for (n=0;n<16;n++) e->mm[ndx*16 + n]=m[n];
			if (e->nb<ndx) e->nb=ndx;
			e->modif=true;
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;


			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;

			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;

			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;


		}

		if (res>=0)
		{

			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=M.a[i][j];

			if (res+4*ndx+4>vp->n_env) vp->n_env=res+4*ndx+4;

			for (i=0;i<4;i++)
				for (j=0;j<4;j++)
				vp->env[res+ndx*4+i][j]=m[i+j*4];

		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(temp,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,M);
		}
	}


#endif

}


void CVertexBuffer::setMatrixViewProj(char *name)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------
    int n;
    int i,j;
	
    C3DAPIBASE *render=(C3DAPIBASE*) capi;
	MATRIXMATRIX(ViewProj,render->viewMatrix,render->projectionMatrix);
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        
        n=0;
        for (j=0;j<4;j++)
            for (i=0;i<4;i++)
                e->m[n++]=ViewProj.a[j][i];
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            n=0;
            for (j=0;j<4;j++)
                for (i=0;i<4;i++)
                    e->m[n++]=ViewProj.a[j][i];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_MATRIX;
        e->modif=true;
        for (n=0;n<16;n++) e->m[n]=ViewProj.a[n>>2][n&3];
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12
		C3DAPIBASE *render=(C3DAPIBASE*) capi;

		MATRIXMATRIX(ViewProj,render->viewMatrix,render->projectionMatrix);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&ViewProj); return; }

		ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&ViewProj);
#else
#ifdef API3D_DIRECT3D11
		C3DAPIBASE *render=(C3DAPIBASE*) capi;

		MATRIXMATRIX(ViewProj,render->viewMatrix,render->projectionMatrix);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&ViewProj); return; }

		ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&ViewProj);
#else
#ifdef API3D_DIRECT3D10
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		D3DXMatrixMultiply( (D3DXMATRIX*)&ViewProj, &render->viewMatrix,&render->projectionMatrix );
        
		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&ViewProj); return; }
		ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&ViewProj);
#else

		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DXMATRIX matView,matProj;
		D3DDevice->GetTransform( D3DTS_VIEW, &matView );
		D3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
        D3DXMatrixMultiply( (D3DXMATRIX*)&ViewProj, &matView, &matProj );

		effect->SetMatrix( name, (D3DXMATRIX*)&ViewProj );
#endif
#endif
#endif
	}

#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	GLfloat mm[16],*m;
#ifndef API3D_OPENGL20
	GLfloat proj[16];
#endif
	C3DAPIBASE *render;
	int i,j,k;
	int nb;

	modifparams=true;
    
    m=(float*)&ViewProj;

	render=(C3DAPIBASE*) capi;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
#ifdef API3D_OPENGL
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) mm[i+4*j]=render->pVIEW.a[j][i];

		glGetFloatv(GL_PROJECTION_MATRIX, proj);

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=proj[i+4*k]*mm[k+4*j];
			}
#else
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) mm[i+4*j]=render->pVIEW.a[j][i];

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=render->projection[i+4*k]*mm[k+4*j];
			}
#endif
        
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            for (n=0;n<16;n++) e->m[n]=m[n];
            return;
        }
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				for (n=0;n<16;n++) e->m[n]=m[n];
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIX;
			for (n=0;n<16;n++) e->m[n]=m[n];
			e->modif=true;
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (vp->l_constants[n]==_MATRIX) nn++;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=render->pVIEW.a[j][i];

			glGetFloatv(GL_PROJECTION_MATRIX, proj);

			glMatrixMode(GL_MATRIX0_ARB+res);
			glLoadIdentity();
			glLoadMatrixf(proj);
			glMultMatrixf(m);
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,render->VIEW);
		}

	}

#endif

}


void CVertexBuffer::setMatrixViewProjTex(char *name)
{

#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------
    int n;
    int i,j;
	CMatrix M;

    C3DAPIBASE *render=(C3DAPIBASE*) capi;
	MATRIXMATRIX(M,render->viewMatrix,render->projectionMatrix);
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        
        n=0;
        for (j=0;j<4;j++)
            for (i=0;i<4;i++)
                e->m[n++]=M.a[j][i];
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            n=0;
            for (j=0;j<4;j++)
                for (i=0;i<4;i++)
                    e->m[n++]=M.a[j][i];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_MATRIX;
        e->modif=true;
        for (n=0;n<16;n++) e->m[n]=M.a[n>>2][n&3];
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		
		CMatrix M;
		MATRIXMATRIX(M,render->viewMatrix,render->projectionMatrix);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D11
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		
		CMatrix M;
		MATRIXMATRIX(M,render->viewMatrix,render->projectionMatrix);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D10
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		D3DXMATRIX matViewProj;
		D3DXMatrixMultiply( &matViewProj, &render->viewMatrix,&render->projectionMatrix );

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&matViewProj); return; }

		ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&matViewProj);
#else
		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DXMATRIX matView,matProj,matViewProj;
		D3DDevice->GetTransform( D3DTS_VIEW, &matView );
		D3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
        D3DXMatrixMultiply( &matViewProj, &matView, &matProj );

		effect->SetMatrix( name, &matViewProj );
#endif
#endif
#endif
	}

#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	GLfloat mm[16],m[16],proj[16];
	CMatrix M;
	C3DAPIBASE *render;
	int i,j,k;
	int nb;

    modifparams=true;
	render=(C3DAPIBASE*) capi;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
#ifdef API3D_OPENGL
		M=render->pVIEW;
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) mm[i+4*j]=M.a[j][i];

		glGetFloatv(GL_PROJECTION_MATRIX, proj);

		proj[1+4*0]=-proj[1+4*0]; 
		proj[1+4*1]=-proj[1+4*1]; 
		proj[1+4*2]=-proj[1+4*2]; 

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=proj[i+4*k]*mm[k+4*j];
			}
#else
		M=render->pVIEW;
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) mm[i+4*j]=M.a[j][i];

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) proj[i+4*j]=render->projection[i+4*j];

		proj[1+4*0]=-proj[1+4*0]; 
		proj[1+4*1]=-proj[1+4*1]; 
		proj[1+4*2]=-proj[1+4*2]; 

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=proj[i+4*k]*mm[k+4*j];
			}
#endif
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            for (n=0;n<16;n++) e->m[n]=m[n];
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				for (n=0;n<16;n++) e->m[n]=m[n];
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIX;
			for (n=0;n<16;n++) e->m[n]=m[n];
			e->modif=true;
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (vp->l_constants[n]==_MATRIX) nn++;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			M=render->pVIEW;
			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];

			glGetFloatv(GL_PROJECTION_MATRIX, proj);
			proj[1+4*0]=-proj[1+4*0]; 
			proj[1+4*1]=-proj[1+4*1]; 
			proj[1+4*2]=-proj[1+4*2]; 

			glMatrixMode(GL_MATRIX0_ARB+res);
			glLoadIdentity();
			glLoadMatrixf(proj);
			glMultMatrixf(m);
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,render->VIEW);
		}

	}

#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setMatrixWorldViewProj(char *name)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------
    int n;
    int i,j;
	CMatrix M,MM;
    
    C3DAPIBASE *render=(C3DAPIBASE*) capi;
	MATRIXMATRIX(MM,render->viewMatrix,render->projectionMatrix);
	MATRIXMATRIX(M,render->worldMatrix,MM);
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        
        n=0;
        for (j=0;j<4;j++)
            for (i=0;i<4;i++)
                e->m[n++]=M.a[j][i];
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            n=0;
            for (j=0;j<4;j++)
                for (i=0;i<4;i++)
                    e->m[n++]=M.a[j][i];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_MATRIX;
        e->modif=true;
        for (n=0;n<16;n++) e->m[n]=M.a[n>>2][n&3];
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		
		CMatrix M,MM;
		MATRIXMATRIX(MM,render->viewMatrix,render->projectionMatrix);
		MATRIXMATRIX(M,render->worldMatrix,MM);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D11
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		
		CMatrix M,MM;
		MATRIXMATRIX(MM,render->viewMatrix,render->projectionMatrix);
		MATRIXMATRIX(M,render->worldMatrix,MM);

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D10
		C3DAPIBASE *render=(C3DAPIBASE*) capi;

		D3DXMATRIX matViewProj,matWorldViewProj;
        D3DXMatrixMultiply( &matViewProj, &render->viewMatrix, &render->projectionMatrix );
		D3DXMatrixMultiply( &matWorldViewProj, &render->worldMatrix, &matViewProj );

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&matWorldViewProj); return; }

		ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&matWorldViewProj);
#else
		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DXMATRIX matView,matProj,matViewProj,matWorldViewProj,matWorld;
		D3DDevice->GetTransform( D3DTS_VIEW, &matView );
		D3DDevice->GetTransform( D3DTS_PROJECTION, &matProj );
		D3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
        D3DXMatrixMultiply( &matViewProj, &matView, &matProj );
		D3DXMatrixMultiply( &matWorldViewProj, &matWorld, &matViewProj );

		effect->SetMatrix( name, &matWorldViewProj );
#endif
#endif
#endif
	}

#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	GLfloat m[16],mm[16];
#ifndef API3D_OPENGL20
	GLfloat proj[16];
#endif
	CMatrix M;
	C3DAPIBASE *render;
	int i,j,k;
	int nb;

	modifparams=true;

	render=(C3DAPIBASE*) capi;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
		M=render->WORLD*render->pVIEW;
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) mm[i+4*j]=M.a[j][i];
#ifdef API3D_OPENGL
		glGetFloatv(GL_PROJECTION_MATRIX, proj);

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=proj[i+4*k]*mm[k+4*j];
			}
#else
		for (i=0;i<4;i++)
			for (j=0;j<4;j++) 
			{
				m[i+4*j]=0.0f;
				for (k=0;k<4;k++) m[i+4*j]+=render->projection[i+4*k]*mm[k+4*j];
			}
#endif
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            for (n=0;n<16;n++) e->m[n]=m[n];
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				for (n=0;n<16;n++) e->m[n]=m[n];
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIX;
			for (n=0;n<16;n++) e->m[n]=m[n];
			e->modif=true;
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (vp->l_constants[n]==_MATRIX) nn++;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}
		if (res>=0)
		{
			M=render->WORLD*render->pVIEW;
			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];

			glGetFloatv(GL_PROJECTION_MATRIX, proj);

			glMatrixMode(GL_MATRIX0_ARB+res);
			glLoadIdentity();
			glLoadMatrixf(proj);

			glMultMatrixf(m);
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}
		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,render->WORLD*render->VIEW);
		}

	}

#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setMatrixWorld(char *name)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------
    int n;
    int i,j;
    
    C3DAPIBASE *render=(C3DAPIBASE*) capi;
	CMatrix M=render->worldMatrix;
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        
        n=0;
        for (j=0;j<4;j++)
            for (i=0;i<4;i++)
                e->m[n++]=M.a[j][i];
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            n=0;
            for (j=0;j<4;j++)
                for (i=0;i<4;i++)
                    e->m[n++]=M.a[j][i];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_MATRIX;
        e->modif=true;
        for (n=0;n<16;n++) e->m[n]=M.a[n>>2][n&3];
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	if (effect)
	{
#ifdef API3D_DIRECT3D12
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		CMatrix M=render->worldMatrix;

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DFXEffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D11
		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		CMatrix M=render->worldMatrix;

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&M); return; }

		ID3DX11EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&M);
#else
#ifdef API3D_DIRECT3D10
		C3DAPIBASE *render=(C3DAPIBASE*) capi;

		int c=name[0]&255;
		if (matrices[c]) { matrices[c]->SetMatrix((float*)&render->worldMatrix); return; }

		ID3D10EffectMatrixVariable* var=effect->GetVariableByName(name)->AsMatrix();
		if (var) var->SetMatrix((float*)&render->worldMatrix);
#else
		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DXMATRIX matWorld;
		D3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
		effect->SetMatrix( name, &matWorld );
#endif
#endif
#endif
	}

#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	GLfloat m[16];
	CMatrix M;
	C3DAPIBASE *render;
	int i,j;
	int nb;

	modifparams=true;

	render=(C3DAPIBASE*) capi;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
		M=render->WORLD;

		for (i=0;i<4;i++)
			for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];

        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            for (n=0;n<16;n++) e->m[n]=m[n];
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				for (n=0;n<16;n++) e->m[n]=m[n];
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_MATRIX;
			for (n=0;n<16;n++) e->m[n]=m[n];
			e->modif=true;
		}

#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]==_MATRIX) nn++;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			M=render->WORLD;
			glMatrixMode(GL_MATRIX0_ARB+res);
			glLoadIdentity();
			for (i=0;i<4;i++)
				for (j=0;j<4;j++) m[i+4*j]=M.a[j][i];
			glLoadMatrixf(m);
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
		if (vp->emvp[n])
		{
			vp->emvp[n]->Set(res,render->WORLD);
		}
	}
#endif

}
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setVector(char *name,CVector const &v)
{
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(v.x,v.y,v.z,0);

#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else

		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif
    
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
    modifparams=true;
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;

	if (vp->shadermodel)
	{

#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=0.0f;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif
		
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=v.x;
			vp->env[res][1]=v.y;
			vp->env[res][2]=v.z;
			vp->env[res][3]=0.0f;
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				vp->emvp[n]->Set(res,v);
			}
	}

#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setRGB(char *name,CRGBA const &col)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(col.r,col.g,col.b,0);

#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else

		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;

	modifparams=true;

	if (vp->shadermodel)
	{

#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=col.r; e->v[1]=col.g; e->v[2]=col.b; e->v[3]=0.0f;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif
		
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=col.r;
			vp->env[res][1]=col.g;
			vp->env[res][2]=col.b;
			vp->env[res][3]=0.0f;
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				CVector v(col.r,col.g,col.b);
				vp->emvp[n]->Set(res,v);
			}
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setRGBA(char *name,float r,float g,float b,float a)
{
    
#ifdef API3D_METAL
    //------------------------------------------------------------------------------------------------ METAL -----------
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
    //------------------------------------------------------------------------------------------------ DIRECT3D ----------
    if (effect)
    {
        D3DXVECTOR4 vect(r,g,b,a);
        
#ifdef API3D_DIRECT3D12
        
        int c=name[0]&255;
        if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }
        
        ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
        if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11
        
        int c=name[0]&255;
        if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }
        
        ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
        if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10
        
        int c=name[0]&255;
        if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }
        
        ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
        if (var) var->SetFloatVector((float*)&vect);
#else
        
        effect->SetVector( name, &vect);
#endif
#endif
#endif
    }
#endif
    
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
    //------------------------------------------------------------------------------------------------ OPEN GL -----------
    int n,nn,res;
    int nb;
    
    modifparams=true;
    
    if (vp->shadermodel)
    {
        
#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
            return;
        }
        
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->modif=true;
                e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
                return;
            }
            e=vp->listenv.GetNext();
        }
        
        if (!e)
        {
            e=vp->listenv.InsertLast();
            strcpy(e->name,name);
            e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=a;
            e->type=TYPE_VECTOR;
            e->modif=true;
        }
#endif
    }
    else
        if (G_VertexProgram)
        {
#ifdef OPENGL_VERTEXPROGRAMS
            res=-1;
            n=0;
            nn=0;
            while ((n<vp->last_constant)&&(res==-1))
            {
                if (strcmp(name,vp->constants[n])==0) res=nn;
                
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
                if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
            }
            
            if (res>=0)
            {
                if (res+1>vp->n_env) vp->n_env=res+1;
                vp->env[res][0]=r;
                vp->env[res][1]=g;
                vp->env[res][2]=b;
                vp->env[res][3]=a;
            }
#endif
        }
        else
        {
            res=-1;
            n=0;
            nn=0;
            while ((n<vp->last_constant)&&(res==-1))
            {
                if (strcmp(name,vp->constants[n])==0) res=n;
                
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
                
            }
            
            if (res!=-1)
                for (n=0;n<8;n++)
                    if (vp->emvp[n])
                    {
                        vp->emvp[n]->Set(res,r,g,b,a);
                    }
        }
    
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setRGB(char *name,float r,float g,float b)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(r,g,b,0);

#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else

		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;

	modifparams=true;

	if (vp->shadermodel)
	{

#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=r; e->v[1]=g; e->v[2]=b; e->v[3]=0.0f;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif	
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=r;
			vp->env[res][1]=g;
			vp->env[res][2]=b;
			vp->env[res][3]=0.0f;
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				CVector v(r,g,b);
				vp->emvp[n]->Set(res,v);
			}
	}

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setPoint(char *name,CVector const &v)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(v.x,v.y,v.z,1.0f);

#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10
		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;

	modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=1.0f;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif		
	}
	else	
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=v.x;
			vp->env[res][1]=v.y;
			vp->env[res][2]=v.z;
			vp->env[res][3]=1.0f;

		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				vp->emvp[n]->Set(res,v.x,v.y,v.z,1.0f);
			}
	}

#endif
}

void CVertexBuffer::setVector4(char *name,CVector4 const &v)
{
    
#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -----------

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(v.x,v.y,v.z,v.w);
#ifdef API3D_DIRECT3D12

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11

		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10
		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		if (var) var->SetFloatVector((float*)&vect);
#else
		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;

	modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=v.x; e->v[1]=v.y; e->v[2]=v.z; e->v[3]=v.w;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif		
	}
	else	
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=v.x;
			vp->env[res][1]=v.y;
			vp->env[res][2]=v.z;
			vp->env[res][3]=v.w;

		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				vp->emvp[n]->Set(res,v.x,v.y,v.z,v.w);
			}
	}

#endif
}

void CVertexBuffer::setVectorArrayElement(char *name,int ndx,CVector const &v)
{
    
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
	int n,nn,res;
	int nb;

	modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
        
    if (e)
    {
        e->modif=true;
        e->vv[ndx*4 +0]=v.x;
        e->vv[ndx*4 +1]=v.y;
        e->vv[ndx*4 +2]=v.z;
        e->vv[ndx*4 +3]=0.0f;
        if (e->nb<ndx) e->nb=ndx;
        return;
    }

	e=vp->listenv.GetFirst();
	while (e)
	{
		if (strcmp(e->name,name)==0)
		{
			e->modif=true;
			e->vv[ndx*4 +0]=v.x;
			e->vv[ndx*4 +1]=v.y;
			e->vv[ndx*4 +2]=v.z;
			e->vv[ndx*4 +3]=0.0f;
			if (e->nb<ndx) e->nb=ndx;
			return;
		}
		e=vp->listenv.GetNext();
	}

	if (!e)
	{		
		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->type=TYPE_VECTORLIST;
		e->vv[ndx*4 +0]=v.x;
		e->vv[ndx*4 +1]=v.y;
		e->vv[ndx*4 +2]=v.z;
		e->vv[ndx*4 +3]=0.0f;
		if (e->nb<ndx) e->nb=ndx;
		e->modif=true;
	}
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ---------

	if (effect)
	{
		D3DXVECTOR4 vect(v.x,v.y,v.z,0);
#ifdef API3D_DIRECT3D12

		int res,n,nb;

		int c=name[0]&255;
		if (vectors[c])
		{
			vectors[c]->SetVectorArray((float*)&vect,ndx,1);
			return;
		}

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
			if (var) var->SetVectorArray((float*)&vect,ndx,1);
		}

#else
#ifdef API3D_DIRECT3D11

		int res,n,nb;

		int c=name[0]&255;
		if (vectors[c])
		{
			vectors[c]->SetVectorArray((float*)&vect,ndx,1);
			return;
		}

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
			if (var) var->SetVectorArray((float*)&vect,ndx,1);
		}

#else
#ifdef API3D_DIRECT3D10
		int res,n,nb,num;

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0)
			{
				res=n;
				if (vp->l_constants[n]>0xffff) num=vp->l_constants[n]>>16; else num=1;
			}
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
			static D3DXVECTOR4 vectlist[256];
			if (var)
			{
				var->GetFloatVectorArray((float*)&vectlist,0,num*4);
				vectlist[ndx]=vect;
				var->SetFloatVectorArray((float*)&vectlist,0,num*4);
			}
		}
#else
#ifdef API3D_DIRECT3D9
		int res,n,nb,num;

		res=-1;
		n=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0)
			{
				res=n;
				if (vp->l_constants[n]>0xffff) num=vp->l_constants[n]>>16; else num=1;
			}
			else
			{
				if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
				n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
			}
		}

		if (res>=0)
		{
			static D3DXVECTOR4 vectlist[256];
			effect->GetFloatArray(name,(float*)&vectlist,num*4);
			vectlist[ndx]=vect;
			effect->SetFloatArray(name,(float*)&vectlist,num*4);
		}
#else
		char temp[128];
		sprintf(temp,"%s%d",name,ndx);
		effect->SetVector( temp, &vect);
#endif
#endif
#endif
#endif
	}
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int n,nn,res;
	int nb;
	char temp[128];
	sprintf(temp,"%s%d",name,ndx);

	modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL		
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->vv[ndx*4 +0]=v.x;
            e->vv[ndx*4 +1]=v.y;
            e->vv[ndx*4 +2]=v.z;
            e->vv[ndx*4 +3]=0.0f;
            if (e->nb<ndx) e->nb=ndx;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->vv[ndx*4 +0]=v.x;
				e->vv[ndx*4 +1]=v.y;
				e->vv[ndx*4 +2]=v.z;
				e->vv[ndx*4 +3]=0.0f;
				if (e->nb<ndx) e->nb=ndx;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{		
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->type=TYPE_VECTORLIST;
			e->vv[ndx*4 +0]=v.x;
			e->vv[ndx*4 +1]=v.y;
			e->vv[ndx*4 +2]=v.z;
			e->vv[ndx*4 +3]=0.0f;
			if (e->nb<ndx) e->nb=ndx;
			e->modif=true;
		}
#endif
	}
	else
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
		}

		if (res>=0)
		{
			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res+ndx][0]=v.x;
			vp->env[res+ndx][1]=v.y;
			vp->env[res+ndx][2]=v.z;
			vp->env[res+ndx][3]=0.0f;

		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(temp,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}


		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				vp->emvp[n]->Set(res+ndx,v);
			}
	}

#endif

}

void CVertexBuffer::setVector4ArrayElement(char *name,int ndx,CVector4 const &v)
{
    
#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -----------
    int n,nn,res;
    int nb;

    modifparams=true;

    int c=name[0]&255;
    Env *e=roots[c];
        
    if (e)
    {
        e->modif=true;
        e->vv[ndx*4 +0]=v.x;
        e->vv[ndx*4 +1]=v.y;
        e->vv[ndx*4 +2]=v.z;
        e->vv[ndx*4 +3]=v.w;
        if (e->nb<ndx) e->nb=ndx;
        return;
    }

    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->vv[ndx*4 +0]=v.x;
            e->vv[ndx*4 +1]=v.y;
            e->vv[ndx*4 +2]=v.z;
            e->vv[ndx*4 +3]=v.w;
            if (e->nb<ndx) e->nb=ndx;
            return;
        }
        e=vp->listenv.GetNext();
    }

    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->type=TYPE_VECTORLIST;
        e->vv[ndx*4 +0]=v.x;
        e->vv[ndx*4 +1]=v.y;
        e->vv[ndx*4 +2]=v.z;
        e->vv[ndx*4 +3]=v.w;
        if (e->nb<ndx) e->nb=ndx;
        e->modif=true;
    }
#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

    if (effect)
    {
        D3DXVECTOR4 vect(v.x,v.y,v.z,v.w);
#ifdef API3D_DIRECT3D12

        int res,n,nb;

        int c=name[0]&255;
        if (vectors[c])
        {
            vectors[c]->SetVectorArray((float*)&vect,ndx,1);
            return;
        }

        res=-1;
        n=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(name,vp->constants[n])==0) res=n;
            else
            {
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
            }
        }

        if (res>=0)
        {
            ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
            if (var) var->SetVectorArray((float*)&vect,ndx,1);
        }

#else
#ifdef API3D_DIRECT3D11

        int res,n,nb;

        int c=name[0]&255;
        if (vectors[c])
        {
            vectors[c]->SetVectorArray((float*)&vect,ndx,1);
            return;
        }

        res=-1;
        n=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(name,vp->constants[n])==0) res=n;
            else
            {
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
            }
        }

        if (res>=0)
        {
            ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
            if (var) var->SetVectorArray((float*)&vect,ndx,1);
        }

#else
#ifdef API3D_DIRECT3D10
        int res,n,nb,num;

        res=-1;
        n=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(name,vp->constants[n])==0)
            {
                res=n;
                if (vp->l_constants[n]>0xffff) num=vp->l_constants[n]>>16; else num=1;
            }
            else
            {
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
            }
        }

        if (res>=0)
        {
            ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
            static D3DXVECTOR4 vectlist[256];
            if (var)
            {
                var->GetFloatVectorArray((float*)&vectlist,0,num*4);
                vectlist[ndx]=vect;
                var->SetFloatVectorArray((float*)&vectlist,0,num*4);
            }
        }
#else
#ifdef API3D_DIRECT3D9
        int res,n,nb,num;

        res=-1;
        n=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(name,vp->constants[n])==0)
            {
                res=n;
                if (vp->l_constants[n]>0xffff) num=vp->l_constants[n]>>16; else num=1;
            }
            else
            {
                if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
                n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
            }
        }

        if (res>=0)
        {
            static D3DXVECTOR4 vectlist[256];
            effect->GetFloatArray(name,(float*)&vectlist,num*4);
            vectlist[ndx]=vect;
            effect->SetFloatArray(name,(float*)&vectlist,num*4);
        }
#else
		char temp[128];
        sprintf(temp,"%s%d",name,ndx);
        effect->SetVector( temp, &vect);
#endif
#endif
#endif
#endif
    }
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
    int n,nn,res;
    int nb;
    char temp[128];
    sprintf(temp,"%s%d",name,ndx);

    modifparams=true;

    if (vp->shadermodel)
    {
#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->vv[ndx*4 +0]=v.x;
            e->vv[ndx*4 +1]=v.y;
            e->vv[ndx*4 +2]=v.z;
            e->vv[ndx*4 +3]=v.w;
            if (e->nb<ndx) e->nb=ndx;
            return;
        }

        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->modif=true;
                e->vv[ndx*4 +0]=v.x;
                e->vv[ndx*4 +1]=v.y;
                e->vv[ndx*4 +2]=v.z;
                e->vv[ndx*4 +3]=v.w;
                if (e->nb<ndx) e->nb=ndx;
                return;
            }
            e=vp->listenv.GetNext();
        }

        if (!e)
        {
            e=vp->listenv.InsertLast();
            strcpy(e->name,name);
            e->type=TYPE_VECTORLIST;
            e->vv[ndx*4 +0]=v.x;
            e->vv[ndx*4 +1]=v.y;
            e->vv[ndx*4 +2]=v.z;
            e->vv[ndx*4 +3]=v.w;
            if (e->nb<ndx) e->nb=ndx;
            e->modif=true;
        }
#endif
    }
    else
    if (G_VertexProgram)
    {
#ifdef OPENGL_VERTEXPROGRAMS
        res=-1;
        n=0;
        nn=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(name,vp->constants[n])==0) res=nn;

            if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
            if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
            if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
            n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;
        }

        if (res>=0)
        {
            if (res+1>vp->n_env) vp->n_env=res+1;
            vp->env[res+ndx][0]=v.x;
            vp->env[res+ndx][1]=v.y;
            vp->env[res+ndx][2]=v.z;
            vp->env[res+ndx][3]=v.w;

        }
#endif
    }
    else
    {
        res=-1;
        n=0;
        nn=0;
        while ((n<vp->last_constant)&&(res==-1))
        {
            if (strcmp(temp,vp->constants[n])==0) res=n;

            if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
            n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

        }


        if (res!=-1)
        for (n=0;n<8;n++)
            if (vp->emvp[n])
            {
                vp->emvp[n]->Set(res+ndx,v);
            }
    }

#endif

}

void CVertexBuffer::setFloat(char *name,float f)
{
    
#ifdef API3D_METAL
    //------------------------------------------------------------------------------------------------ METAL -----------
    
    modifparams=true;
    
    int c=name[0]&255;
    Env *e=roots[c];
    
    if (e)
    {
        e->modif=true;
        e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
        return;
    }
    
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->modif=true;
            e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    if (!e)
    {
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
        e->type=TYPE_VECTOR;
        e->modif=true;
    }
    
#endif
     
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
		D3DXVECTOR4 vect(f,f,f,0);
#ifdef API3D_DIRECT3D12
		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DFXEffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D11
		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3DX11EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		var->SetFloatVector((float*)&vect);
#else
#ifdef API3D_DIRECT3D10
		int c=name[0]&255;
		if (vectors[c]) { vectors[c]->SetFloatVector((float*)&vect); return; }

		ID3D10EffectVectorVariable* var=effect->GetVariableByName(name)->AsVector();
		var->SetFloatVector((float*)&vect);
#else
		effect->SetVector( name, &vect);
#endif
#endif
#endif
	}
#endif
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int	n,nn,res;
	int nb;

    modifparams=true;

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
        int c=name[0]&255;
        Env *e=roots[c];
        
        if (e)
        {
            e->modif=true;
            e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
            return;
        }

		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
			    e->modif=true;
				e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
				return;
			}
			e=vp->listenv.GetNext();
		}

		if (!e)
		{
			e=vp->listenv.InsertLast();
			strcpy(e->name,name);
			e->v[0]=f; e->v[1]=f; e->v[2]=f; e->v[3]=0.0f;
			e->type=TYPE_VECTOR;
			e->modif=true;
		}
#endif
	}
	else	
	if (G_VertexProgram)
	{
#ifdef OPENGL_VERTEXPROGRAMS
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=nn;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			if (((vp->l_constants[n]&0xffff)==_VECTOR)&&(!vp->translate_constants[n])) nn+=nb;
			if (((vp->l_constants[n]&0xffff)==_MATRIX)&&(vp->l_constants[n]>0xffff)) nn+=nb*4;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res>=0)
		{

			if (res+1>vp->n_env) vp->n_env=res+1;
			vp->env[res][0]=f;
			vp->env[res][1]=f;
			vp->env[res][2]=f;
			vp->env[res][3]=0.0f;
		}
#endif
	}
	else
	{
		res=-1;
		n=0;
		nn=0;
		while ((n<vp->last_constant)&&(res==-1))
		{
			if (strcmp(name,vp->constants[n])==0) res=n;

			if (vp->l_constants[n]>0xffff) nb=vp->l_constants[n]>>16; else nb=1;
			n+=LENGTH_VAR[vp->l_constants[n]&0xffff]*nb;

		}

		if (res!=-1)
		for (n=0;n<8;n++)
			if (vp->emvp[n])
			{
				vp->emvp[n]->Set(res,f,f,f,0);
			}
	}
#endif

}

void CVertexBuffer::setDword(char *name,unsigned int d)
{
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	if (effect)
	{
//		effect->SetDword(name,d);
	}
#else
//------------------------------------------------------------------------------------------------ OPEN GL -----------

#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setTextureAssociation(char * assign,int value)
{
	if (assign)
	{
		if (AssignedTexture[value]) free(AssignedTexture[value]);
		AssignedTexture[value]=(char*) malloc(strlen(assign)+1);
		strcpy(AssignedTexture[value],assign);
#if defined(API3D_DIRECT3D12)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D10)
		if (effect) res[value]=effect->GetVariableByName(assign)->AsShaderResource();
#endif
#if defined(API3D_OPENGL20)||defined(API3D_METAL)
        
        Env *e;
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,assign)==0)
            {
                res[value]=e;
                return;
            }
            e=vp->listenv.GetNext();
        }
        
        e=vp->listenv.InsertLast();
        sprintf(e->name,assign);
        e->tex=0;
        e->type=TYPE_TEXTURE;
#if defined(API3D_METAL)
        for (int n=0;n<16;n++)
            if (vp->texture[n])
                if (strcmp(vp->texture[n],assign)==0) e->loc=n;
#endif
        res[value]=e;
#endif
	}
}

void CVertexBuffer::setLightmap(char *name,int n)
{

#if defined(API3D_METAL)
    
    modiftextures=true;

	C3DAPIBASE *render=(C3DAPIBASE*) capi;
    
    Env *e;
    e=vp->listenv.GetFirst();
    while (e)
    {
        if (strcmp(e->name,name)==0)
        {
            e->tex=render->Lightmap[n];
            return;
        }
        e=vp->listenv.GetNext();
    }
    
    e=vp->listenv.InsertLast();
    strcpy(e->name,name);
    e->tex=render->Lightmap[n];
    for (int n=0;n<16;n++)
    if (vp->texture[n])
        if (strcmp(vp->texture[n],name)==0) e->loc=n;
    e->type=TYPE_TEXTURE;
    
#endif


#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

	C3DAPIBASE *render;

	if (effect)
	{
		render=(C3DAPIBASE*) capi;

#ifdef API3D_DIRECT3D12
		if (n>=0)
		{
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->Lightmap[n]);
		}
#else
#ifdef API3D_DIRECT3D11
		if (n>=0)
		{
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->Lightmap[n]);
		}
#else
#ifdef API3D_DIRECT3D10
		if (n>=0)
		{
			ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->Lightmap[n]);
		}
#else
		if (n>=0)
		{
			D3DDevice->SetTexture(0,render->Lightmap[n]);

#ifdef API3D_DIRECT3D9
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
#else
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
#endif
		}

		Texture[pos_texture]=NULL;
		D3DDevice->GetTexture(0,&Texture[pos_texture]);
		effect->SetTexture(name,Texture[pos_texture]);
		if (render->aT!=-555) render->SetTexture(render->aT);
		pos_texture++;
#endif
#endif
#endif
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
		int t=0;

		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (n>=0) t=render->Lightmap[n];

		Env *e;
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
				e->tex=t;
				return;
			}
			e=vp->listenv.GetNext();
		}

		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->tex=t;
		e->type=TYPE_TEXTURE;
#endif
	}
	else
	{
		int nn,res;

		res=-1;
		nn=0;
		while ((nn<16)&&(res==-1))
		{
			if (vp->texture[nn])
			{
				if (strcmp(vp->texture[nn],name)==0) res=nn;
			}
			nn++;
		}

		if (res!=-1) numTexture[res]=16384+n;
	}
#endif

}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setTexture(char *name,int n)
{
	int x,y;
	CVector4 vec;

	if ((n>=0)&&(vp))
	{
		if (vp->SizingTexture0)
		{
			C3DAPIBASE *rdr=(C3DAPIBASE*)capi;
			if(strcmp(vp->RenderState[0][0].Texture[0],name)==0)
			{
				x=rdr->cTexture[n]->data.Tilex;
				y=rdr->cTexture[n]->data.Tiley;
				vec.Init((float)x,(float)y,1.0f/x,1.0f/y);
				setVector4("SIZEDESTZ",vec);
			}
		}

		if (vp->FSRSample0)
		{
			C3DAPIBASE *rdr=(C3DAPIBASE*) capi;
			if (strcmp(vp->RenderState[0][0].Texture[0],name)==0)
			{
				x=rdr->cTexture[n]->data.Tilex;
				y=rdr->cTexture[n]->data.Tiley;
				vec.Init((float)x,(float)y,1.0f/x,1.0f/y);
				setVector4("SIZEDEST0",vec);
			}
		}

		if (vp->FSRSample1)
		{
			C3DAPIBASE *rdr=(C3DAPIBASE*) capi;
			if (strcmp(vp->RenderState[0][0].Texture[1],name)==0)
			{
				x=rdr->cTexture[n]->data.Tilex;
				y=rdr->cTexture[n]->data.Tiley;
				vec.Init((float)x,(float)y,1.0f/x,1.0f/y);
				setVector4("SIZEDEST1",vec);
			}
		}
	}

#if defined(API3D_METAL)

    modiftextures=true;
    
	C3DAPIBASE *render=(C3DAPIBASE*) capi;

	if (n>=0)
	{
        int aa=render->Texture[n];

        Env *e;
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->tex=aa;
                return;
            }
            e=vp->listenv.GetNext();
        }
        
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->tex=aa;
        for (int n=0;n<16;n++)
            if (vp->texture[n])
                if (strcmp(vp->texture[n],name)==0) e->loc=n;
        e->type=TYPE_TEXTURE;
    }
    else
    {
		Env *e;
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
				e->tex=n;
				return;
			}
			e=vp->listenv.GetNext();
		}
    
		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->tex=n;
		for (int n=0;n<16;n++)
		if (vp->texture[n])
			if (strcmp(vp->texture[n],name)==0) e->loc=n;
		e->type=TYPE_TEXTURE;
	}
    
#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	

	C3DAPIBASE *render;

	if (effect)
	{
		render=(C3DAPIBASE*) capi;

		if (n>=0)
		{
			if (render->IsActiveTexture(n))
			{

#ifdef API3D_DIRECT3D12
				ID3DFXEffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
				var->SetResource(render->Texture[n]);
#else
#ifdef API3D_DIRECT3D11
				ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
				var->SetResource(render->Texture[n]);
#else
#ifdef API3D_DIRECT3D10
				ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
				var->SetResource(render->Texture[n]);
#else
				D3DDevice->SetTexture(0,render->Texture[n]);
#ifdef API3D_DIRECT3D9
				if (render->iTexture[n]->ClampRepeat==0)
				{
					D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
					D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
				}
				else
				{
					D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
					D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
				}

				if (render->iTexture[n]->MipMap==1)
					D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);
				else
					D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
#else
				if (render->iTexture[n]->ClampRepeat==0)
				{
					D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
					D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
				}
				else
				{
					D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
					D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
				}

				if (render->iTexture[n]->MipMap==1)
					D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_LINEAR);
				else
					D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_NONE);

#endif
#endif
#endif
#endif
			}
		}
		else
		{

#ifdef API3D_DIRECT3D12
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(&render->shaderResourceView[-(n+1)]);
#else
#ifdef API3D_DIRECT3D11
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->shaderResourceView[-(n+1)]);
#else
#ifdef API3D_DIRECT3D10
			ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->shaderResourceView[-(n+1)]);
#else
			D3DDevice->SetTexture(0,render->D3DTextureSecondary[-(n+1)]);

#ifdef API3D_DIRECT3D9
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
			D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
			D3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			D3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
#else
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
			D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_NONE);
			D3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
			D3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
#endif
#endif
#endif
#endif
		}

#if !defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)&&!defined(API3D_DIRECT3D12)
		Texture[pos_texture]=NULL;
			
		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DDevice->GetTexture(0,&Texture[pos_texture]);

		effect->SetTexture(name,Texture[pos_texture]);
		
		pos_texture++;
#endif
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
		int t=0;

		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (n>=0)
		{
			if (render->IsActiveTexture(n)) t=render->Texture[n];
		}
		else
		{
#ifdef VR_SCREENPROJ
			if (n==-666) t=render->renderedTextureSPVR;
			else t=render->renderedTexture[-(n+1)];
#else
			t=render->renderedTexture[-(n+1)];
#endif
		}

		Env *e;
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
				e->tex=t;
				return;
			}
			e=vp->listenv.GetNext();
		}

		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->tex=t;
		e->type=TYPE_TEXTURE;
#endif
	}
	else
	{
		int nn,res;

		res=-1;
		nn=0;
		while ((nn<16)&&(res==-1))
		{
			if (vp->texture[nn])
			{
				if (strcmp(vp->texture[nn],name)==0) res=nn;
			}
			nn++;
		}

		if (res!=-1) numTexture[res]=n;
	}
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setDepthTexture(char *name,int n)
{
#if defined(API3D_METAL)

    modiftextures=true;
    
	C3DAPIBASE *render=(C3DAPIBASE*) capi;

	if (n<0)
    {
		Env *e;
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
				e->tex=n-6666;
				return;
			}
			e=vp->listenv.GetNext();
		}
    
		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->tex=n-6666;
		for (int n=0;n<16;n++)
		if (vp->texture[n])
			if (strcmp(vp->texture[n],name)==0) e->loc=n;
		e->type=TYPE_TEXTURE;
	}
    
#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	

	C3DAPIBASE *render;

	if (effect)
	{
		render=(C3DAPIBASE*) capi;

		if (n<0)
		{

#ifdef API3D_DIRECT3D12
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(&render->shaderResourceViewDepth[-(n+1)]);
#else
#ifdef API3D_DIRECT3D11
			ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->shaderResourceViewDepth[-(n+1)]);
#else
#ifdef API3D_DIRECT3D10
			ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
			var->SetResource(render->shaderResourceViewDepth[-(n+1)]);
#else
			D3DDevice->SetTexture(0,render->D3DTextureSecondary[-(n+1)]);

#ifdef API3D_DIRECT3D9
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
			D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
			D3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			D3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
#else
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
			D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
			D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_NONE);
			D3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
			D3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
#endif
#endif
#endif
#endif
		}

#if !defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)&&!defined(API3D_DIRECT3D12)
		Texture[pos_texture]=NULL;
			
		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

		D3DDevice->GetTexture(0,&Texture[pos_texture]);

		effect->SetTexture(name,Texture[pos_texture]);
		
		pos_texture++;
#endif
	}
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

	if (vp->shadermodel)
	{
#ifdef OPENGL_GLSL
		int t=0;

		C3DAPIBASE *render=(C3DAPIBASE*) capi;
		if (n<0)
		{
			t=render->depthrenderTexture[-(n+1)];
		}

		Env *e;
		e=vp->listenv.GetFirst();
		while (e)
		{
			if (strcmp(e->name,name)==0)
			{
				e->tex=t;
				return;
			}
			e=vp->listenv.GetNext();
		}

		e=vp->listenv.InsertLast();
		strcpy(e->name,name);
		e->tex=t;
		e->type=TYPE_TEXTURE;
#endif
	}
	else
	{
		int nn,res;

		res=-1;
		nn=0;
		while ((nn<16)&&(res==-1))
		{
			if (vp->texture[nn])
			{
				if (strcmp(vp->texture[nn],name)==0) res=nn;
			}
			nn++;
		}

		if (res!=-1) numTexture[res]=n;
	}
#endif
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::setTextureVS(char *name,int n)
{
#if defined(API3D_METAL)

    modiftextures=true;
    
    C3DAPIBASE *render=(C3DAPIBASE*) capi;

    if (n>=0)
    {
        int aa=render->Texture[n];

        Env *e;
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->tex=aa;
                return;
            }
            e=vp->listenv.GetNext();
        }
        
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->tex=aa;
        for (int n=0;n<16;n++)
            if (vp->texturevs[n])
                if (strcmp(vp->texturevs[n],name)==0) e->loc=n;
        e->type=TYPE_TEXTUREVS;
    }
    else
    {
        Env *e;
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->tex=n;
                return;
            }
            e=vp->listenv.GetNext();
        }
    
        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->tex=n;
        for (int n=0;n<16;n++)
        if (vp->texturevs[n])
            if (strcmp(vp->texturevs[n],name)==0) e->loc=n;
        e->type=TYPE_TEXTUREVS;
    }
    
#endif
    
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------

    C3DAPIBASE *render;

    if (effect)
    {

		render=(C3DAPIBASE*) capi;

        if (n>=0)
        {
            if (render->IsActiveTexture(n))
            {

#ifdef API3D_DIRECT3D12
                ID3DFXEffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
                var->SetResource(render->Texture[n]);
#else
#ifdef API3D_DIRECT3D11
                ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
                var->SetResource(render->Texture[n]);
#else
#ifdef API3D_DIRECT3D10
                ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
                var->SetResource(render->Texture[n]);
#else
                D3DDevice->SetTexture(0,render->Texture[n]);
#ifdef API3D_DIRECT3D9
                if (render->iTexture[n]->ClampRepeat==0)
                {
                    D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
                    D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
                }
                else
                {
                    D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
                    D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
                }

                if (render->iTexture[n]->MipMap==1)
                    D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);
                else
                    D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
#else
                if (render->iTexture[n]->ClampRepeat==0)
                {
                    D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
                    D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
                }
                else
                {
                    D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
                    D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
                }

                if (render->iTexture[n]->MipMap==1)
                    D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_LINEAR);
                else
                    D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_NONE);

#endif
#endif
#endif
#endif
            }
        }
        else
        {

#ifdef API3D_DIRECT3D12
            ID3DFXEffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
            var->SetResource(&render->shaderResourceView[-(n+1)]);
#else
#ifdef API3D_DIRECT3D11
            ID3DX11EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
            var->SetResource(render->shaderResourceView[-(n+1)]);
#else
#ifdef API3D_DIRECT3D10
            ID3D10EffectShaderResourceVariable* var=effect->GetVariableByName(name)->AsShaderResource();
            var->SetResource(render->shaderResourceView[-(n+1)]);
#else
            D3DDevice->SetTexture(0,render->D3DTextureSecondary[-(n+1)]);

#ifdef API3D_DIRECT3D9
            D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
            D3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
            D3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
            D3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
            D3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
#else
            D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP);
            D3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP);
            D3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_NONE);
            D3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
            D3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
#endif
#endif
#endif
#endif
        }

#if !defined(API3D_DIRECT3D10)&&!defined(API3D_DIRECT3D11)&&!defined(API3D_DIRECT3D12)
        Texture[pos_texture]=NULL;

		if (D3DDevice==NULL) D3DDevice=D3DDevicePrincipal;

        D3DDevice->GetTexture(0,&Texture[pos_texture]);

        effect->SetTexture(name,Texture[pos_texture]);
        
        pos_texture++;
#endif
    }
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------

    if (vp->shadermodel)
    {
#ifdef OPENGL_GLSL
        int t=0;

        C3DAPIBASE *render=(C3DAPIBASE*) capi;
        if (n>=0)
        {
            if (render->IsActiveTexture(n)) t=render->Texture[n];
        }
        else t=render->renderedTexture[-(n+1)];

        Env *e;
        e=vp->listenv.GetFirst();
        while (e)
        {
            if (strcmp(e->name,name)==0)
            {
                e->tex=t;
                return;
            }
            e=vp->listenv.GetNext();
        }

        e=vp->listenv.InsertLast();
        strcpy(e->name,name);
        e->tex=t;
        e->type=TYPE_TEXTUREVS;
#endif
    }
    else
    {
        int nn,res;

        res=-1;
        nn=0;
        while ((nn<16)&&(res==-1))
        {
            if (vp->texture[nn])
            {
                if (strcmp(vp->texture[nn],name)==0) res=nn;
            }
            nn++;
        }

        if (res!=-1) numTexture[res]=n;
    }
#endif
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::resetAssignation()
{
	for (int n=1;n<18;n++)
	{
		if (AssignedTexture[n]) free(AssignedTexture[n]);
		AssignedTexture[n]=NULL;
#if defined(API3D_DIRECT3D12)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D10)||defined(API3D_OPENGL20)
		res[n]=NULL;
#endif
    
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  functions:
//
//		simple effect functions / not optimized
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::PhongVertexBuffer(CObject3D * obj)
{
	int n;
	float f;

	for (n=0;n<obj->nVertices;n++)
	{
		obj->Vertices[n].Map2=obj->Vertices[n].Map;
		obj->Vertices[n].Map.x=0.5f+obj->Vertices[n].Norm.x/2;
		obj->Vertices[n].Map.y=0.5f+obj->Vertices[n].Norm.y/2;
		f=obj->Vertices[n].Norm.z;
		if (f<0) f=0;
		obj->Vertices[n].Diffuse.Init(f,f,f);
	}

	CreateFrom2(obj,API3D_VERTEXDATAS|API3D_COLORDATAS4|API3D_TEXCOODATAS|API3D_TEXCOO2DATAS|API3D_TEXPERVERTEX);
	ref=obj;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::ApplyPhong(CMatrix const &M,CObject3D * obj)
{
	int n;
	float f;
	float xym[2];

	LockVertices();
	for (n=0;n<(int)nVertices;n++)
	{
		obj->Vertices[n].NormCalc=obj->Vertices[n].Norm*M;
		xym[0]=0.5f+obj->Vertices[n].NormCalc.x/2;
		xym[1]=0.5f+obj->Vertices[n].NormCalc.y/2;
		SetTexCoo(n,xym[0],xym[1]);
		f=-obj->Vertices[n].NormCalc.z;
		if (f<0.5f) f=0.5f;
		SetColor(n,f,f,f,1);
	}
	UnlockVertices();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::ApplyPhong(CMatrix const &M,CObject3D * obj,float fog)
{
	int n;
	float f;
	float xym[2];

	LockVertices();
	for (n=0;n<(int)nVertices;n++)
	{
		obj->Vertices[n].NormCalc=obj->Vertices[n].Norm*M;
		xym[0]=0.5f+obj->Vertices[n].NormCalc.x/2;
		xym[1]=0.5f+obj->Vertices[n].NormCalc.y/2;
		SetTexCoo(n,xym[0],xym[1]);
		f=-obj->Vertices[n].NormCalc.z;
		if (f<0.5f) f=0.5f;
		f=f*fog;
		SetColor(n,f,f,f,1);
	}
	UnlockVertices();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::ApplyWhitePhong(CMatrix const &M,CObject3D * obj,float t)
{
	int n;
	float f;
	float xym[2];

	LockVertices();
	for (n=0;n<(int)nVertices;n++)
	{
		obj->Vertices[n].NormCalc=obj->Vertices[n].Norm*M;
		xym[0]=(0.5f+obj->Vertices[n].NormCalc.x/2)*(1-t) + 0.5f*t;
		xym[1]=(0.5f+obj->Vertices[n].NormCalc.y/2)*(1-t) + 0.5f*t;
		SetTexCoo(n,xym[0],xym[1]);
		f=-obj->Vertices[n].NormCalc.z;
		if (f<0.5f) f=0.5f;
		f=f*(1-t) + t*1;
		SetColor(n,f,f,f,1);
	}
	UnlockVertices();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::ApplyMorph(CObject3D * obj1,CObject3D * obj2,float t)
{
	int n;
	CVector v;

	LockVertices();
	for (n=0;n<(int)nVertices;n++)
	{
		v=obj1->Vertices[n].Stok + t*(obj2->Vertices[n].Stok - obj1->Vertices[n].Stok);
		SetVertex(n,v);
	}
	UnlockVertices();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////**

  function:

**/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::ApplySimplePhong(CMatrix const &M,CObject3D * obj)
{
	int n;
	float f;
	float xym[2];

	LockVertices();
	for (n=0;n<(int)nVertices;n++)
	{
		obj->Vertices[n].NormCalc=obj->Vertices[n].Norm*M;
		xym[0]=0.5f+obj->Vertices[n].NormCalc.x/2;
		xym[1]=0.5f+obj->Vertices[n].NormCalc.y/2;
		SetTexCoo(n,xym[0],xym[1]);
		f=-obj->Vertices[n].NormCalc.z;
		if (f<0.5f) f=0.5f;
		SetColor(n,f,f,f,1);
	}
	UnlockVertices();
}


void CVertexBuffer::CloneVertexProgram(CVertexBuffer *vb)
{
	cloned=1;
    
	from=vb;
	vb->hasbeencloned=1;

#if defined(API3D_METAL)

	FX=vb->FX;
    for (int n=0;n<256;n++) roots[n]=vb->roots[n];

#endif

#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
	effect=vb->effect;
#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	tech=vb->tech;
        
    if (effect)
    {
        for (int n=0;n<256;n++)
        {
            vectors[n]=vb->vectors[n];
            matrices[n]=vb->matrices[n];
        }
    }

#endif

#endif
    
	vp=vb->vp;
    
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
    
    for (int n=0;n<256;n++) roots[n]=vb->roots[n];
    
#ifndef GLES
	for (int p=0;p<vp->get_pass_count();p++)
	{
		if (vp->shadermodel)
		{
			glslprograms[p]=vb->glslprograms[p];
			vert_shader[p] = vb->vert_shader[p];
			frag_shader[p]=vb->frag_shader[p];
		}
		else
		{
			programs[p]=vb->programs[p];
			fprograms[p]=vb->fprograms[p];
			fp[p]=vb->fp[p];
		}
	}
#endif
#endif

	Type|=API3D_VERTEXPROGRAM;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CVertexBuffer::BasicEffect()
{
	CObject3D *obj;
	obj=new CObject3D;
	obj->Cube(1,1,1);
	CreateFrom2(obj,API3D_VERTEXDATAS|API3D_COLORDATAS4|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
	obj->Free();
	delete obj;
	//Type|=API3D_VERTEXPROGRAM;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::setContextFunction(bool (*fn_set)(CVertexBuffer *effect,int nT,int nT2,int tag))
{
	function_set=fn_set;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::setTextureFunction(void (*fn_texture)(CVertexBuffer *effect,char *name,int nt))
{
	function_texture=fn_texture;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::setTextureFunction2(void (*fn_texture)(CVertexBuffer *effect,char *name,int nt))
{
	function_texture2=fn_texture;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::setVectorFunction(void (*fn_vectors)(CVertexBuffer *effect,char *name,int nt))
{
	function_vectors=fn_vectors;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::setVectorFunction2(void (*fn_vectors)(CVertexBuffer *effect,char *name,int nt))
{
	function_vectors2=fn_vectors;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::registerName(char *name)
{
	names[nb++]=name;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

void CContextEffectParameters::reset()
{
	nb=0;
	start=end=-1;
	function_vectors=NULL;
	function_vectors2=NULL;
	function_texture=NULL;
	function_texture2=NULL;
	function_set=NULL;
	for (int n=0;n<128;n++) names[n]=NULL;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool CContextEffectParameters::update(CVertexBuffer *effect,int nT,int nT2,int tag)
{
	int n;

	for (n=0;n<nb;n++)
	{
		if (function_texture) function_texture(effect,names[n],nT);
		if (function_texture2) function_texture2(effect,names[n],nT2);
		if (function_vectors) function_vectors(effect,names[n],nT);
		if (function_vectors2) function_vectors2(effect,names[n],nT2);
	}

	if (function_set) return function_set(effect,nT,nT2,tag);
	else return true;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 function:
 
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

bool VertexProgramDefinedVar(CVertexBuffer *shd,char *name)
{
    int n;
    if (shd->vp)
    {
        if (shd->vp->is_constant(name)) return true;
        
        for (n=0;n<shd->vp->npsvectors;n++)
            if (strcmp(shd->vp->psvectors[n],name)==0) return true;
    }
    
    return false;
}

