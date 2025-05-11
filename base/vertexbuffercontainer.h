
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

#ifndef _VERTEXBUFFERCONTAINER_H_
#define _VERTEXBUFFERCONTAINER_H_  

#include "params.h"
#include "maths.h"
#include "objects3d.h"
#include "3d_api_base.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB VBElement
{
public:
	CVertexBuffer *vb;

	VBElement() { vb=NULL; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB VBContainer
{
public:
	CList <VBElement> Container;

	VBContainer() {}

	void Free();
	void New(CVertexBuffer *vb);
	void Del(CVertexBuffer *vb);

#ifdef API3D_DIRECT3D
	void OnLostDevice(LPDIRECT3DDEVICE8 device);
	void OnResetDevice(LPDIRECT3DDEVICE8 device);
	void Release(LPDIRECT3DDEVICE8 device);
	void Restore(LPDIRECT3DDEVICE8 device);
	void Free(LPDIRECT3DDEVICE8 device);
#endif

#ifdef API3D_DIRECT3D9
	void OnLostDevice(LPDIRECT3DDEVICE9 device);
	void OnResetDevice(LPDIRECT3DDEVICE9 device);
	void Release(LPDIRECT3DDEVICE9 device);
	void Restore(LPDIRECT3DDEVICE9 device);
	void Free(LPDIRECT3DDEVICE9 device);
	void UpdateDevice(LPDIRECT3DDEVICE9 device);
#endif

#ifdef API3D_DIRECT3D10
	void OnLostDevice(ID3D10Device* device);
	void OnResetDevice(ID3D10Device* device);
	void Release(ID3D10Device* device);
	void Restore(ID3D10Device* device);
	void Free(ID3D10Device* device);
#endif

#ifdef API3D_DIRECT3D11
	void OnLostDevice(ID3D11Device* device);
	void OnResetDevice(ID3D11Device* device);
	void Release(ID3D11Device* device);
	void Restore(ID3D11Device* device);
	void Free(ID3D11Device* device);
#endif

#ifdef API3D_DIRECT3D12
	void OnLostDevice(ID3D12Device* device);
	void OnResetDevice(ID3D12Device* device);
	void Release(ID3D12Device* device);
	void Restore(ID3D12Device* device);
	void Free(ID3D12Device* device);
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20) || defined(API3D_METAL)
	void Release();
	void Restore();
#endif

};

extern VBContainer VertexBufferContainer;
#endif
