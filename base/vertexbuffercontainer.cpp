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
//	@file vertexbuffercontainer.cpp
//	@date 2004
////////////////////////////////////////////////////////////////////////


#include "params.h"

extern int api3d_number_registered_vertexbuffers;
extern int api3d_number_registerednew_vertexbuffers;

#include <math.h>
#include <stdio.h>

#include "edges.h"
#include "vertexbuffercontainer.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__APPLE__)||defined(API3D_VR)
#define NOGETPROC
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef API3D_SDL2
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
#if !defined(NOGETPROC)&&!defined(LINUX)&&!defined(GLES)&&!defined(GLES20)
#ifdef OPENGL_MULTITEXTURING
extern PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2fARB;
extern PFNGLACTIVETEXTUREARBPROC				glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTextureARB;
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
#endif
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Container de vertex buffers GL/D3D
//	- reset
//	- lost device (D3D)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


extern unsigned int SizeVB(int TypeVB);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	bool exist=false;

	api3d_number_registerednew_vertexbuffers++;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb) exist=true;
		el=Container.GetNext();
	}

	if (!exist)
	{
		api3d_number_registered_vertexbuffers++;
		el=Container.InsertLast();
		el->vb=vb;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else
			el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free(LPDIRECT3DDEVICE8 device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnLostDevice(LPDIRECT3DDEVICE8 device)
{
	HRESULT h;
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
				if (el->vb->effect)
				{
					h=el->vb->effect->OnLostDevice();
				}
			}
		}
		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnResetDevice(LPDIRECT3DDEVICE8 device)
{
	VBElement *el;
	HRESULT h;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
				if (el->vb->effect)
				{
					h=el->vb->effect->OnResetDevice();
					//h=el->vb->effect->FindNextValidTechnique(NULL,&el->vb->tdesc);
					h=el->vb->effect->SetTechnique(el->vb->tdesc.Index);
				}
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release(LPDIRECT3DDEVICE8 device)
{

	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if (el->vb->pVertices) el->vb->UnlockVertices();
			if (el->vb->pIndices) el->vb->UnlockIndices();

			if (el->vb->D3D8INDEX) el->vb->D3D8INDEX->Release();
			if (el->vb->D3D8VB) el->vb->D3D8VB->Release();

			el->vb->D3D8INDEX=NULL;
			el->vb->D3D8VB=NULL;

			if (el->vb->nSTREAMS>0)
			{
				for (int n=0;n<el->vb->nSTREAMS;n++)
				{
					if (el->vb->STREAMS[n])
					{
						el->vb->STREAMS[n]->Release();
						el->vb->STREAMS[n]=NULL;
					}
				}
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VBDX_CreateVertexBuffer(int nv,LPDIRECT3DDEVICE8 D3DDevice,LPDIRECT3DVERTEXBUFFER8 *pVB,int blend,int temp,int hardware,int dyn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore(LPDIRECT3DDEVICE8 device)
{
	int nv,ni;
	VBElement *el;
	HRESULT h;
	int blend,temp;
	int tag=0;

	el=Container.GetFirst();
	while (el)
	{
	    tag=1;
		if (el->vb->D3DDevice==device)
		{
			nv=el->vb->nVertices;
			ni=el->vb->nIndices;

			if (el->vb->TypeVB>32) blend=1;
			else blend=0;

			temp=el->vb->TypeVB&0xF;

			if (VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->D3D8VB,blend,temp,1,el->vb->dynamic))
			{

                h=el->vb->D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_DEFAULT,&el->vb->D3D8INDEX);

                el->vb->LockVertices();
                memcpy(el->vb->pVertices,el->vb->Vertices,SizeVB(el->vb->TypeVB)*nv);
                el->vb->UnlockVertices();

                el->vb->LockIndices();
                memcpy(el->vb->pIndices,el->vb->Indices,sizeof(DWORD)*ni);
                el->vb->UnlockIndices();

                if (el->vb->nSTREAMS>0)
                {
                    for (int n=0;n<el->vb->nSTREAMS;n++)
                    {
                        VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->STREAMS[n],blend,temp,1,el->vb->dynamic);
                    }
                }
			}
			else tag=0;
		}
		if (tag==0) el=Container.DeleteAndGetNext();
		else el=Container.GetNext();
	}

}

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D9

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::UpdateDevice(LPDIRECT3DDEVICE9 device)
{
	VBElement *el=Container.GetFirst();
	while (el)
	{
		el->vb->D3DDevice=device;
		el=Container.GetNext();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	bool exist=false;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb) exist=true;
		el=Container.GetNext();
	}

	if (!exist)
	{
		el=Container.InsertLast();
		el->vb=vb;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnLostDevice(LPDIRECT3DDEVICE9 device)
{
	HRESULT h;
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
				if (el->vb->effect)
				{
					h=el->vb->effect->OnLostDevice();
				}
			}
		}
		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnResetDevice(LPDIRECT3DDEVICE9 device)
{
	VBElement *el;
	HRESULT h;
	D3DXHANDLE dh;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
				if (el->vb->effect)
				{
					h=el->vb->effect->OnResetDevice();
					h=el->vb->effect->FindNextValidTechnique(NULL,&dh);
					h=el->vb->effect->SetTechnique(dh);
				}
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release(LPDIRECT3DDEVICE9 device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if (el->vb->pVertices) el->vb->UnlockVertices();
			if (el->vb->pIndices) el->vb->UnlockIndices();
			if (el->vb->D3D9INDEX) el->vb->D3D9INDEX->Release();
			if (el->vb->D3D9VB) el->vb->D3D9VB->Release();
			el->vb->D3D9INDEX=NULL;
			el->vb->D3D9VB=NULL;

			if (el->vb->nSTREAMS>0)
			{
				for (int n=0;n<el->vb->nSTREAMS;n++)
				{
					if (el->vb->STREAMS[n])
					{
						el->vb->STREAMS[n]->Release();
						el->vb->STREAMS[n]=NULL;
					}
				}
			}
		}

		el=Container.GetNext();
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VBDX_CreateVertexBuffer(int nv,LPDIRECT3DDEVICE9 D3DDevice,LPDIRECT3DVERTEXBUFFER9 *pVB,int blend,int temp,int hardware,int dyn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore(LPDIRECT3DDEVICE9 device)
{
	int nv,ni;
	VBElement *el;
	HRESULT h;
	int blend;
	int temp;
	int n;
	int tag=0;

	el=Container.GetFirst();
	while (el)
	{
	    tag=1;
		if (el->vb->D3DDevice==device)
		{
			nv=el->vb->nVertices;
			ni=el->vb->nIndices;

			if ((el->vb->TypeVB)&32) blend=1;
			else blend=0;

			temp=(el->vb->TypeVB)&0xF;

			if (VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->D3D9VB,blend,temp,1,el->vb->dynamic))
			{
                h=el->vb->D3DDevice->CreateIndexBuffer(ni*sizeof(DWORD),0,D3DFMT_INDEX32,D3DPOOL_DEFAULT,&el->vb->D3D9INDEX,NULL);

                el->vb->LockVertices();
                if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->Vertices,SizeVB(el->vb->TypeVB)*nv);
                el->vb->UnlockVertices();

                el->vb->LockIndices();
                if (el->vb->Indices) memcpy(el->vb->pIndices,el->vb->Indices,sizeof(DWORD)*ni);
                el->vb->UnlockIndices();
                if (el->vb->nSTREAMS>0)
                {
                    for (n=0;n<el->vb->nSTREAMS;n++)
                    {
                        VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->STREAMS[n],blend,temp,1,el->vb->dynamic);
						el->vb->LockStream(n);
						if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->VerticesStream[n],SizeVB(el->vb->TypeVB)*nv);
						el->vb->UnlockStream();
                    }
                }
			}
			else tag=0;
		}
		if (tag==0) el=Container.DeleteAndGetNext();
		else el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free(LPDIRECT3DDEVICE9 device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_OPENGL)||defined(API3D_OPENGL20)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	el=Container.InsertLast();
	el->vb=vb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else
			el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release()
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->Type&API3D_VERTEXPROGRAM)
		{
#ifndef API3D_OPENGL20
#ifndef GLES
			glDeleteProgramsARB(el->vb->vp->get_pass_count(),el->vb->programs);
#endif
#endif
		}
		el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore()
{
	VBElement *el;
#ifndef API3D_OPENGL20
	int p;
	int len;
#endif
	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->Type&API3D_VERTEXPROGRAM)
		{
#ifndef API3D_OPENGL20
#ifndef GLES
			glGenProgramsARB(el->vb->vp->get_pass_count(),el->vb->programs);

			for (p=0;p<el->vb->vp->get_pass_count();p++)
			{

				glEnable(GL_VERTEX_PROGRAM_ARB);
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB,el->vb->programs[p]);
				el->vb->str_programs[p]=el->vb->vp->get_string_vsh(p);

				len=strlen(el->vb->str_programs[p]);
				glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,len,(const void*)el->vb->str_programs[p]);
				free(el->vb->str_programs[p]);

				glDisable(GL_VERTEX_PROGRAM_ARB);
			}
#endif
#endif
		}
		el=Container.GetNext();
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D10

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	bool exist=false;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb) exist=true;
		el=Container.GetNext();
	}

	if (!exist)
	{
		el=Container.InsertLast();
		el->vb=vb;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnLostDevice(ID3D10Device* device)
{
//	HRESULT h;
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
/*
				if (el->vb->effect)
				{
					h=el->vb->effect->OnLostDevice();
				}
/**/
			}
		}
		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnResetDevice(ID3D10Device* device)
{
	VBElement *el;
//	HRESULT h;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
/*
				if (el->vb->effect)
				{
					h=el->vb->effect->OnResetDevice();
					h=el->vb->effect->FindNextValidTechnique(NULL,&dh);
					h=el->vb->effect->SetTechnique(dh);
				}
/**/
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release(ID3D10Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if (el->vb->pVertices) el->vb->UnlockVertices();
			if (el->vb->pIndices) el->vb->UnlockIndices();
			if (el->vb->D3DINDEX) el->vb->D3DINDEX->Release();
			if (el->vb->D3DVB) el->vb->D3DVB->Release();
			el->vb->D3DINDEX=NULL;
			el->vb->D3DVB=NULL;

			if (el->vb->nSTREAMS>0)
			{
				for (int n=0;n<el->vb->nSTREAMS;n++)
				{
					if (el->vb->STREAMS[n])
					{
						el->vb->STREAMS[n]->Release();
						el->vb->STREAMS[n]=NULL;
					}
				}
			}
		}
		el=Container.GetNext();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VBDX_CreateVertexBuffer(int nv,ID3D10Device* D3DDevice,ID3D10Buffer **pVB,int blend,int temp,int hardware,int dyn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore(ID3D10Device* device)
{
	int nv,ni;
	VBElement *el;
//	HRESULT h;
	int blend;
	int temp;
	int n;
	int tag=0;

	el=Container.GetFirst();
	while (el)
	{
	    tag=1;
		if (el->vb->D3DDevice==device)
		{
			nv=el->vb->nVertices;
			ni=el->vb->nIndices;

			if ((el->vb->TypeVB)&32) blend=1;
			else blend=0;

			temp=(el->vb->TypeVB)&0xF;

			if (VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->D3DVB,blend,temp,1,el->vb->dynamic))
			{                
				D3D10_BUFFER_DESC indexBufferDesc;

				ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));
				indexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * ni;
				indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
				indexBufferDesc.MiscFlags = 0;

				el->vb->D3DDevice->CreateBuffer(&indexBufferDesc, NULL, &el->vb->D3DINDEX);

                el->vb->LockVertices();
                if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->Vertices,SizeVB(el->vb->TypeVB)*nv);
                el->vb->UnlockVertices();

                el->vb->LockIndices();
                if (el->vb->Indices) memcpy(el->vb->pIndices,el->vb->Indices,sizeof(DWORD)*ni);
                el->vb->UnlockIndices();
                if (el->vb->nSTREAMS>0)
                {
                    for (n=0;n<el->vb->nSTREAMS;n++)
                    {
                        VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->STREAMS[n],blend,temp,1,el->vb->dynamic);
						el->vb->LockStream(n);
						if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->VerticesStream[n],SizeVB(el->vb->TypeVB)*nv);
						el->vb->UnlockStream();
                    }
                }
			}
			else tag=0;
		}
		if (tag==0) el=Container.DeleteAndGetNext();
		else el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free(ID3D10Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D11

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	bool exist=false;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb) exist=true;
		el=Container.GetNext();
	}

	if (!exist)
	{
		el=Container.InsertLast();
		el->vb=vb;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnLostDevice(ID3D11Device* device)
{
//	HRESULT h;
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
			}
		}
		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnResetDevice(ID3D11Device* device)
{
	VBElement *el;
//	HRESULT h;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
/*
				if (el->vb->effect)
				{
					h=el->vb->effect->OnResetDevice();
					h=el->vb->effect->FindNextValidTechnique(NULL,&dh);
					h=el->vb->effect->SetTechnique(dh);
				}
/**/
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release(ID3D11Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if (el->vb->pVertices) el->vb->UnlockVertices();
			if (el->vb->pIndices) el->vb->UnlockIndices();
			if (el->vb->D3DINDEX) el->vb->D3DINDEX->Release();
			if (el->vb->D3DVB) el->vb->D3DVB->Release();
			el->vb->D3DINDEX=NULL;
			el->vb->D3DVB=NULL;

			if (el->vb->nSTREAMS>0)
			{
				for (int n=0;n<el->vb->nSTREAMS;n++)
				{
					if (el->vb->STREAMS[n])
					{
						el->vb->STREAMS[n]->Release();
						el->vb->STREAMS[n]=NULL;
					}
				}
			}
		}
		el=Container.GetNext();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VBDX_CreateVertexBuffer(int nv,ID3D11Device* D3DDevice,ID3D11Buffer **pVB,int blend,int temp,int hardware,int dyn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore(ID3D11Device* device)
{
	int nv,ni;
	VBElement *el;
//	HRESULT h;
	int blend;
	int temp;
	int n;
	int tag=0;

	el=Container.GetFirst();
	while (el)
	{
	    tag=1;
		if (el->vb->D3DDevice==device)
		{
			nv=el->vb->nVertices;
			ni=el->vb->nIndices;

			if ((el->vb->TypeVB)&32) blend=1;
			else blend=0;

			temp=(el->vb->TypeVB)&0xF;

			if (VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->D3DVB,blend,temp,1,el->vb->dynamic))
			{                
				D3D11_BUFFER_DESC indexBufferDesc;

				ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * ni;
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				indexBufferDesc.MiscFlags = 0;

				el->vb->D3DDevice->CreateBuffer(&indexBufferDesc, NULL, &el->vb->D3DINDEX);

                el->vb->LockVertices();
                if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->Vertices,SizeVB(el->vb->TypeVB)*nv);
                el->vb->UnlockVertices();

                el->vb->LockIndices();
                if (el->vb->Indices) memcpy(el->vb->pIndices,el->vb->Indices,sizeof(DWORD)*ni);
                el->vb->UnlockIndices();
                if (el->vb->nSTREAMS>0)
                {
                    for (n=0;n<el->vb->nSTREAMS;n++)
                    {
                        VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->STREAMS[n],blend,temp,1,el->vb->dynamic);
						el->vb->LockStream(n);
						if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->VerticesStream[n],SizeVB(el->vb->TypeVB)*nv);
						el->vb->UnlockStream();
                    }
                }
			}
			else tag=0;
		}
		if (tag==0) el=Container.DeleteAndGetNext();
		else el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free(ID3D11Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D12

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	bool exist=false;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb) exist=true;
		el=Container.GetNext();
	}

	if (!exist)
	{
		el=Container.InsertLast();
		el->vb=vb;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnLostDevice(ID3D12Device* device)
{
//	HRESULT h;
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
			}
		}
		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::OnResetDevice(ID3D12Device* device)
{
	VBElement *el;
//	HRESULT h;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if ((el->vb->Type&API3D_VERTEXPROGRAM)&&(el->vb->cloned==0))
			{
/*
				if (el->vb->effect)
				{
					h=el->vb->effect->OnResetDevice();
					h=el->vb->effect->FindNextValidTechnique(NULL,&dh);
					h=el->vb->effect->SetTechnique(dh);
				}
/**/
			}
		}

		el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release(ID3D12Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			if (el->vb->pVertices) el->vb->UnlockVertices();
			if (el->vb->pIndices) el->vb->UnlockIndices();
			if (el->vb->D3DINDEX) el->vb->D3DINDEX->Release();
			if (el->vb->D3DVB) el->vb->D3DVB->Release();
			el->vb->D3DINDEX=NULL;
			el->vb->D3DVB=NULL;

			if (el->vb->nSTREAMS>0)
			{
				for (int n=0;n<el->vb->nSTREAMS;n++)
				{
					if (el->vb->STREAMS[n])
					{
						el->vb->STREAMS[n]->Release();
						el->vb->STREAMS[n]=NULL;
					}
				}
			}
		}
		el=Container.GetNext();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//bool VBDX_CreateVertexBuffer(int nv,ID3D12Device* D3DDevice,ID3D11Buffer **pVB,int blend,int temp,int hardware,int dyn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore(ID3D12Device* device)
{
	int nv,ni;
	VBElement *el;
//	HRESULT h;
	int blend;
	int temp;
	int tag=0;

	el=Container.GetFirst();
	while (el)
	{
	    tag=1;
		if (el->vb->D3DDevice==device)
		{
			nv=el->vb->nVertices;
			ni=el->vb->nIndices;

			if ((el->vb->TypeVB)&32) blend=1;
			else blend=0;

			temp=(el->vb->TypeVB)&0xF;
			/*
			if (VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->D3DVB,blend,temp,1,el->vb->dynamic))
			{                
				D3D11_BUFFER_DESC indexBufferDesc;

				ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * ni;
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				indexBufferDesc.MiscFlags = 0;

				el->vb->D3DDevice->CreateBuffer(&indexBufferDesc, NULL, &el->vb->D3DINDEX);

                el->vb->LockVertices();
                if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->Vertices,SizeVB(el->vb->TypeVB)*nv);
                el->vb->UnlockVertices();

                el->vb->LockIndices();
                if (el->vb->Indices) memcpy(el->vb->pIndices,el->vb->Indices,sizeof(DWORD)*ni);
                el->vb->UnlockIndices();
                if (el->vb->nSTREAMS>0)
                {
                    for (n=0;n<el->vb->nSTREAMS;n++)
                    {
                        VBDX_CreateVertexBuffer(nv,el->vb->D3DDevice,&el->vb->STREAMS[n],blend,temp,1,el->vb->dynamic);
						el->vb->LockStream(n);
						if (el->vb->Vertices) memcpy(el->vb->pVertices,el->vb->VerticesStream[n],SizeVB(el->vb->TypeVB)*nv);
						el->vb->UnlockStream();
                    }
                }
			}
			else tag=0;
			/**/
		}
		if (tag==0) el=Container.DeleteAndGetNext();
		else el=Container.GetNext();
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free(ID3D12Device* device)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb->D3DDevice==device)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else el=Container.GetNext();
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_METAL)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Free()
{
	Container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::New(CVertexBuffer *vb)
{
	VBElement *el;
	el=Container.InsertLast();
	el->vb=vb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Del(CVertexBuffer *vb)
{
	VBElement *el;

	el=Container.GetFirst();
	while (el)
	{
		if (el->vb==vb)
		{
			el=Container.DeleteAndGetNext();
			el=NULL;
		}
		else
			el=Container.GetNext();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Release()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void VBContainer::Restore()
{
}

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VBContainer VertexBufferContainer;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
