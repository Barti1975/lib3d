////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2017>
//	  Copyright (C) <2014>  <Laurent Canc� Francis, 10/08/1975>
//	  laurentcance@free.fr
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
//	@file fx11.h
//	@date 2014
////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _FX11_H_
#define _FX11_H_

#ifdef API3D_DIRECT3D11

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\base\params.h"
#include "..\base\vertexprogram.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SAFE_RELEASE(p)       { if (p) { (p)->Release();  (p) = nullptr; } }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define __VS 0
#define __PS 1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class D3DFXConvention
{
public:
	char str_vs_convention[128];
	char str_ps_convention[128];
	int lowprofile;
	D3DFXConvention();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4_0_level_9_3
void D3DFXCompileConvention(char * version_level);
void D3DFXCompileConventionLowProfile(int set);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char name[40];
	int offset;
	int num;
	short int size;
	short int vs_or_ps;
	int crc;
} __cst;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ID3DFXEffectMatrixVariable;
class ID3DFXEffectVectorVariable;
class ID3DFXEffectShaderResourceVariable;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectVariable
{
public:
	ID3DFXEffect *fx;
	ID3D11ShaderResourceView* tex;
	__cst * data;

	ID3DFXEffectVariable(ID3DFXEffect * eff,__cst * val)
	{
		fx=eff;
		data=val;
	}

	ID3DFXEffectMatrixVariable * AsMatrix();
	ID3DFXEffectVectorVariable * AsVector();
	ID3DFXEffectShaderResourceVariable * AsShaderResource();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectMatrixVariable : public ID3DFXEffectVariable
{
public:
	void SetMatrix(float * mat);
	void SetMatrixArray(float * mats,int index,int nb);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectVectorVariable : public ID3DFXEffectVariable
{
public:
	void SetFloatVector(float * vect);
	void SetVectorArray(float * vects,int index,int nb);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectShaderResourceVariable : public ID3DFXEffectVariable
{
public:
	void SetResource(ID3D11ShaderResourceView *res);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class D3DFX_TECHNIQUE_DESC
{
public:
	int Passes;

	D3DFX_TECHNIQUE_DESC() { Passes=0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class D3DFX_PASS_DESC
{
public:
	void * pIAInputSignature;
	int IAInputSignatureSize;

	D3DFX_PASS_DESC()
	{
		pIAInputSignature=NULL;
		IAInputSignatureSize=0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectPass
{
public:
	ID3DFXEffect *fx;
	int num;

	ID3DFXEffectPass()
	{
		fx=NULL;
	}

	ID3DFXEffectPass(int p,ID3DFXEffect *eff)
	{
		fx=eff;
		num=p;
	}

#ifdef WINDOWS_PHONE
	void Apply(int val,ID3D11DeviceContext2 * context);
#else
	void Apply(int val,ID3D11DeviceContext * context);
#endif
	void GetDesc(D3DFX_PASS_DESC * pdesc);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffectTechnique
{
public:
	ID3DFXEffect *fx;

	ID3DFXEffectTechnique()
	{
		fx=NULL;
	}

	ID3DFXEffectPass * GetPassByIndex(int p);
	void GetDesc(D3DFX_TECHNIQUE_DESC * desc);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ID3DFXEffect
{
public:
	char name[128];
	CVertexProgram *vp;
	int npass;
	char * vertexshaderscripts[8];
	char * pixelshaderscripts[8];

	int n_variables;
	ID3DFXEffectVariable * variables[128];

	ID3DFXEffectShaderResourceVariable * variabletextures[16];
	ID3DFXEffectShaderResourceVariable * variabletextures_vs[16];

	ID3D11VertexShader* _vertexShader[8];
	ID3D11PixelShader* _pixelShader[8];
	ID3DFXEffectPass *pass[8];

	ID3DBlob *blobs_vs[8];
	ID3DBlob *blobs_ps[8];

	ID3DFXEffectTechnique * tech;

	ID3D11Buffer* _constants_vs;
	ID3D11Buffer* _constants_ps;

	bool dirty_vs_constants;
	bool dirty_ps_constants;

	float data_vs[256*16];
	float data_ps[64*16];

	int n_cst_vs;
	int n_cst_ps;
	__cst cst_vs[128];
	__cst cst_ps[128];
	__cst cst_tex[16];
	__cst cst_texvs[16];

	ID3DFXEffect();	
	~ID3DFXEffect();

	ID3DFXEffectTechnique * GetTechniqueByName(char *name);
	ID3DFXEffectVariable * GetVariableByName(char *name);

#ifdef WINDOWS_PHONE
	void Update(ID3D11DeviceContext2 * context);
#else
	void Update(ID3D11DeviceContext * context);
#endif

	void Release();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
int D3DFXCompileEffectFromVP( CVertexProgram *newvp, LPCSTR srcName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude,
										UINT HLSLFlags, UINT FXFlags,
										ID3D11Device2 *pDevice, ID3DFXEffect **ppEffect, ID3DBlob **ppErrors );
#else
int D3DFXCompileEffectFromVP( CVertexProgram *newvp, LPCSTR srcName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude,
										UINT HLSLFlags, UINT FXFlags,
										ID3D11Device *pDevice, ID3DFXEffect **ppEffect, ID3DBlob **ppErrors );
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define D3DX11FXConvention                       D3DFXConvention
#define ID3DX11EffectMatrixVariable              ID3DFXEffectMatrixVariable
#define ID3DX11EffectVectorVariable              ID3DFXEffectVectorVariable
#define ID3DX11EffectShaderResourceVariable      ID3DFXEffectShaderResourceVariable
#define ID3DX11EffectVariable                    ID3DFXEffectVariable
#define D3DX11_TECHNIQUE_DESC                    D3DFX_TECHNIQUE_DESC
#define D3DX11_PASS_DESC                         D3DFX_PASS_DESC
#define ID3DX11EffectPass                        ID3DFXEffectPass
#define ID3DX11EffectTechnique                   ID3DFXEffectTechnique
#define ID3DX11Effect                            ID3DFXEffect
#define D3DX11CompileEffectFromVP				 D3DFXCompileEffectFromVP

#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////