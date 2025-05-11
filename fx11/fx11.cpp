////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2017>
//	  Copyright (C) <2014>  <Laurent Cancé Francis, 10/08/1975>
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
//	@file fx11.cpp
//	@date 2014
////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef API3D_DIRECT3D11
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
#include "pch.h"
#else
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#endif
#include "fx11.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3D11SamplerState *sampler=NULL;
ID3D11SamplerState *samplerlod=NULL;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DFXConvention psvs;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DFXConvention::D3DFXConvention()
{
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
	strcpy(str_vs_convention,"vs_4_0_level_9_3");
	strcpy(str_ps_convention,"ps_4_0_level_9_3");
#else
	strcpy(str_vs_convention,"vs_5_0");
	strcpy(str_ps_convention,"ps_5_0");
#endif
	lowprofile=0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D3DFXCompileConventionLowProfile(int set)
{
	psvs.lowprofile=set;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D3DFXCompileConvention(char * version_level)
{
	sprintf(psvs.str_vs_convention,"vs_%s",version_level);
	sprintf(psvs.str_ps_convention,"ps_%s",version_level);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void strcrc(__cst &c)
{
	char *s=c.name;
	int n=0;
	int crc=0;
	while (s[n]!=0) crc+=s[n++];
	c.crc=crc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
int D3DFXCompileEffectFromVP( CVertexProgram *newvp, LPCSTR srcName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude,
										UINT HLSLFlags, UINT FXFlags,
										ID3D11Device2 *pDevice, ID3DFXEffect **ppEffect, ID3DBlob **ppErrors )
#else
int D3DFXCompileEffectFromVP( CVertexProgram *newvp, LPCSTR srcName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude,
										UINT HLSLFlags, UINT FXFlags,
										ID3D11Device *pDevice, ID3DFXEffect **ppEffect, ID3DBlob **ppErrors )
#endif
{
	int hr=S_OK;
	int ofs=0;
	HRESULT h;
	ID3DBlob* errors;
	char sEntryPoint[128];
	int n,p;

	ID3DFXEffect *fx=new ID3DFXEffect;
	fx->vp=newvp;

	fx->npass=newvp->get_pass_count();

	for (p=0;p<fx->npass;p++)
	{
		fx->vertexshaderscripts[p]=newvp->get_string_mega_vs(p);
		fx->pixelshaderscripts[p]=newvp->get_string_mega_ps(p);
	}

	UINT vsflags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef X64
	UINT psflags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;//|D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT psflags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR|D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	for (p=0;p<fx->npass;p++)
	{
		// vertex shader
		sprintf(sEntryPoint,"RenderPassVS%d",p);

		h=D3DCompile(fx->vertexshaderscripts[p],strlen(fx->vertexshaderscripts[p]), NULL, NULL,NULL, sEntryPoint, psvs.str_vs_convention, vsflags, 0, &fx->blobs_vs[p], &errors );
		if (FAILED(h)) { *ppErrors=errors; return E_FAIL; }

		h=pDevice->CreateVertexShader( fx->blobs_vs[p]->GetBufferPointer(),fx->blobs_vs[p]->GetBufferSize(), NULL, &fx->_vertexShader[p]);
		if (FAILED(h)) hr=E_FAIL;

		// pixel shader
		sprintf(sEntryPoint,"RenderPassPS%d",p);
		h=D3DCompile(fx->pixelshaderscripts[p],strlen(fx->pixelshaderscripts[p]), NULL, NULL,NULL, sEntryPoint, psvs.str_ps_convention, psflags, 0, &fx->blobs_ps[p], &errors );
		if (FAILED(h)) { *ppErrors=errors; return E_FAIL; }

		h=pDevice->CreatePixelShader( fx->blobs_ps[p]->GetBufferPointer(),fx->blobs_ps[p]->GetBufferSize(), NULL, &fx->_pixelShader[p]);
		if (FAILED(h)) hr=E_FAIL;

		fx->pass[p]=new ID3DFXEffectPass(p,fx);
	}

	fx->n_variables=0;

	ofs=0;
	fx->n_cst_vs=0;

    for (n=0;n<MAX_ENTRIES;n++)
    {
        if ((newvp->constants[n])&&(newvp->translate_constants[n]==NULL))
        {
            int nb=0;
            if (newvp->l_constants[n]>0xffff) nb=newvp->l_constants[n]>>16; else nb=1;
                    
            if (nb>1)
            {
                if ((newvp->l_constants[n]&0xffff)==_MATRIX)
				{
					strcpy(fx->cst_vs[fx->n_cst_vs].name,newvp->constants[n]);
					fx->cst_vs[fx->n_cst_vs].offset=ofs;
					fx->cst_vs[fx->n_cst_vs].size=4*4*nb;
					strcrc(fx->cst_vs[fx->n_cst_vs]);
					fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_vs[fx->n_cst_vs]);
					fx->cst_vs[fx->n_cst_vs].num=fx->n_variables++;
					fx->cst_vs[fx->n_cst_vs].vs_or_ps=__VS;
					fx->n_cst_vs++;
					ofs+=4*4*nb;
				}

                if ((newvp->l_constants[n]&0xffff)==_VECTOR)
				{
					strcpy(fx->cst_vs[fx->n_cst_vs].name,newvp->constants[n]);
					fx->cst_vs[fx->n_cst_vs].offset=ofs;
					fx->cst_vs[fx->n_cst_vs].size=4*nb;
					strcrc(fx->cst_vs[fx->n_cst_vs]);
					fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_vs[fx->n_cst_vs]);
					fx->cst_vs[fx->n_cst_vs].num=fx->n_variables++;
					fx->cst_vs[fx->n_cst_vs].vs_or_ps=__VS;
					fx->n_cst_vs++;
					ofs+=4*nb;
				}
            }
            else
            {
				if ((newvp->l_constants[n]&0xffff)==_MATRIX)
				{
					strcpy(fx->cst_vs[fx->n_cst_vs].name,newvp->constants[n]);
					fx->cst_vs[fx->n_cst_vs].offset=ofs;
					fx->cst_vs[fx->n_cst_vs].size=4*4;
					strcrc(fx->cst_vs[fx->n_cst_vs]);
					fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_vs[fx->n_cst_vs]);
					fx->cst_vs[fx->n_cst_vs].num=fx->n_variables++;
					fx->cst_vs[fx->n_cst_vs].vs_or_ps=__VS;
					fx->n_cst_vs++;
					ofs+=4*4; 
				}

                if ((newvp->l_constants[n]&0xffff)==_VECTOR)
				{
					strcpy(fx->cst_vs[fx->n_cst_vs].name,newvp->constants[n]);
					fx->cst_vs[fx->n_cst_vs].offset=ofs;
					fx->cst_vs[fx->n_cst_vs].size=4;
					strcrc(fx->cst_vs[fx->n_cst_vs]);
					fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_vs[fx->n_cst_vs]);
					fx->cst_vs[fx->n_cst_vs].num=fx->n_variables++;
					fx->cst_vs[fx->n_cst_vs].vs_or_ps=__VS;
					fx->n_cst_vs++;
					ofs+=4; 
				}
            }
        }
    }

	CD3D11_BUFFER_DESC constantBufferDesc(ofs*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
	pDevice->CreateBuffer( &constantBufferDesc,nullptr, &fx->_constants_vs );

	ofs=0;
	fx->n_cst_ps=0;
	for (n=0;n<newvp->npsvectors;n++)
	{
		strcpy(fx->cst_ps[fx->n_cst_ps].name,newvp->psvectors[n]);
		fx->cst_ps[fx->n_cst_ps].offset=ofs;
		fx->cst_ps[fx->n_cst_ps].size=4;
		strcrc(fx->cst_ps[fx->n_cst_ps]);
		fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_ps[fx->n_cst_ps]);
		fx->cst_ps[fx->n_cst_ps].num=fx->n_variables++;
		fx->cst_ps[fx->n_cst_ps].vs_or_ps=__PS;
		fx->n_cst_ps++;
		ofs+=4;
	}

	for (n=0;n<newvp->npsmatrices;n++)
	{
		strcpy(fx->cst_ps[fx->n_cst_ps].name,newvp->psmatrices[n]);
		fx->cst_ps[fx->n_cst_ps].offset=ofs;
		fx->cst_ps[fx->n_cst_ps].size=16;
		strcrc(fx->cst_ps[fx->n_cst_ps]);
		fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_ps[fx->n_cst_ps]);
		fx->cst_ps[fx->n_cst_ps].num=fx->n_variables++;
		fx->cst_ps[fx->n_cst_ps].vs_or_ps=__PS;
		fx->n_cst_ps++;
		ofs+=16;
	}

	if (ofs>0)
	{
		CD3D11_BUFFER_DESC constantBufferDesc2(ofs*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
		pDevice->CreateBuffer( &constantBufferDesc2,nullptr, &fx->_constants_ps );
	}	

	for (n=0;n<16;n++)
	{
		if (fx->vp->texture[n])
		{
			strcpy(fx->cst_tex[n].name,fx->vp->texture[n]);
			strcrc(fx->cst_tex[n]);
			fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_tex[n]);
			fx->variabletextures[n]=fx->variables[fx->n_variables]->AsShaderResource();
			fx->variabletextures[n]->tex=NULL;
			fx->cst_tex[n].num=fx->n_variables++;
		}
		else fx->cst_tex[n].num=-1;
	}

	for (n=0;n<16;n++)
	{
		if (fx->vp->texturevs[n])
		{
			strcpy(fx->cst_texvs[n].name,fx->vp->texturevs[n]);
			strcrc(fx->cst_texvs[n]);
			fx->variables[fx->n_variables]=new ID3DFXEffectVariable(fx,&fx->cst_texvs[n]);
			fx->variabletextures_vs[n]=fx->variables[fx->n_variables]->AsShaderResource();
			fx->variabletextures_vs[n]->tex=NULL;
			fx->cst_texvs[n].num=fx->n_variables++;
		}
		else fx->cst_texvs[n].num=-1;
	}

	*ppEffect=fx;

	if (sampler==NULL)
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	
	    samplerDesc.MaxAnisotropy = 0;

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;


		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;

        pDevice->CreateSamplerState( &samplerDesc, &sampler );
	}

	if (samplerlod==NULL)
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	
	    samplerDesc.MaxAnisotropy = 0;

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;


		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;

        pDevice->CreateSamplerState( &samplerDesc, &samplerlod );
	}

	fx->tech=new ID3DFXEffectTechnique;
	fx->tech->fx=fx;

	for (p=0;p<fx->npass;p++)
	{
		free(fx->vertexshaderscripts[p]);
		free(fx->pixelshaderscripts[p]);
		fx->vertexshaderscripts[p]=NULL;
		fx->pixelshaderscripts[p]=NULL;
	}

	newvp->cleanbuffs();

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffect::ID3DFXEffect()
{
	int n;

	vp=NULL;
	npass=0;
	n_variables=0;
		
	for (n=0;n<256;n++) variables[n]=NULL;

	for (n=0;n<16;n++) variabletextures[n]=NULL;
	for (n=0;n<16;n++) variabletextures_vs[n]=NULL;

	for (n=0;n<8;n++)
	{
		blobs_vs[n]=NULL;
		blobs_ps[n]=NULL;
		vertexshaderscripts[n]=NULL;
		pixelshaderscripts[n]=NULL;
		_vertexShader[n]=NULL;
		_pixelShader[n]=NULL;
		pass[n]=NULL;
	}

	tech=NULL;

	memset(data_vs,0,256*16*sizeof(float));
	memset(data_ps,0,64*16*sizeof(float));

	_constants_vs=NULL;
	_constants_ps=NULL;

	n_cst_vs=0;
	n_cst_ps=0;

	dirty_vs_constants=true;
	dirty_ps_constants=true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffect::~ID3DFXEffect()
{
	Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffect::Release()
{
	int n;

	vp=NULL;
	npass=0;

	for (n=0;n<8;n++)
	{
		if (vertexshaderscripts[n]) free(vertexshaderscripts[n]);
		vertexshaderscripts[n]=NULL;
		if (pixelshaderscripts[n]) free(pixelshaderscripts[n]);
		pixelshaderscripts[n]=NULL;

		SAFE_RELEASE(_vertexShader[n]);
		SAFE_RELEASE(_pixelShader[n]);

		SAFE_RELEASE(blobs_vs[n]);
		SAFE_RELEASE(blobs_ps[n]);

		if (pass[n]) delete pass[n];
		pass[n]=NULL;
	}

	if (tech) delete tech;
	tech=NULL;

	for (n=0;n<128;n++)
	{
		if (variables[n]) delete variables[n];
		variables[n]=NULL;
	}

	for (n=0;n<16;n++) variabletextures[n]=NULL;
	for (n=0;n<16;n++) variabletextures_vs[n]=NULL;

	SAFE_RELEASE(_constants_vs);
	SAFE_RELEASE(_constants_ps);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectTechnique * ID3DFXEffect::GetTechniqueByName(char *name)
{
	return tech;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectPass::GetDesc(D3DFX_PASS_DESC * pdesc)
{	
	pdesc->pIAInputSignature=fx->blobs_vs[num]->GetBufferPointer();
	pdesc->IAInputSignatureSize=(int)fx->blobs_vs[num]->GetBufferSize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
void ID3DFXEffect::Update(ID3D11DeviceContext2 * context)
#else
void ID3DFXEffect::Update(ID3D11DeviceContext * context)
#endif
{
	if (dirty_vs_constants)
	{
		if (_constants_vs) context->UpdateSubresource( _constants_vs,0,NULL,&data_vs,0,0);		
		dirty_vs_constants=false;
	}

	if (_constants_vs) context->VSSetConstantBuffers(0,1,&_constants_vs);
	
	if (dirty_ps_constants)
	{
		if (_constants_ps) context->UpdateSubresource( _constants_ps,0,NULL,&data_ps,0,0);		
		dirty_ps_constants=false;
	}

	if (_constants_ps) context->PSSetConstantBuffers(0,1,&_constants_ps);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WINDOWS_PHONE
void ID3DFXEffectPass::Apply(int val,ID3D11DeviceContext2 * context)
#else
void ID3DFXEffectPass::Apply(int val,ID3D11DeviceContext * context)
#endif
{
	context->VSSetShader(fx->_vertexShader[num],nullptr,0);
	context->PSSetShader(fx->_pixelShader[num],nullptr,0);	

	fx->Update(context);

	for (int n=0;n<16;n++)
		if (fx->variabletextures[n])
		{
			if (fx->variabletextures[n]->tex) context->PSSetShaderResources( n, 1, &fx->variabletextures[n]->tex );
		}

	context->PSSetSamplers(0, 1, &sampler);

	for (int n=0;n<16;n++)
		if (fx->variabletextures_vs[n])
		{
			if (fx->variabletextures_vs[n]->tex) context->VSSetShaderResources( n, 1, &fx->variabletextures_vs[n]->tex );
		}

	context->VSSetSamplers(0, 1, &samplerlod);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectPass * ID3DFXEffectTechnique::GetPassByIndex(int p)
{
	return fx->pass[p];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectTechnique::GetDesc(D3DFX_TECHNIQUE_DESC * desc)
{
	desc->Passes=fx->npass;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectVariable *ID3DFXEffect::GetVariableByName(char *name)
{
	int n=0;
//	int crc=0;
//	while (name[n]!=0) crc+=name[n++];

	for (n=0;n<16;n++)
	if (cst_tex[n].num>=0)
		if (strcmp(cst_tex[n].name,name)==0) return variables[cst_tex[n].num];

	for (n=0;n<16;n++)
	if (cst_texvs[n].num>=0)
		if (strcmp(cst_texvs[n].name,name)==0) return variables[cst_texvs[n].num];

	for (n=0;n<n_cst_vs;n++)
	if (cst_vs[n].name[0]==name[0])
		if (strcmp(cst_vs[n].name,name)==0) return variables[cst_vs[n].num];

	for (n=0;n<n_cst_ps;n++)
	if (cst_ps[n].name[0]==name[0])
		if (strcmp(cst_ps[n].name,name)==0) return variables[cst_ps[n].num];

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectMatrixVariable * ID3DFXEffectVariable::AsMatrix()
{
	return (ID3DFXEffectMatrixVariable *) this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectVectorVariable * ID3DFXEffectVariable::AsVector()
{
	return (ID3DFXEffectVectorVariable *) this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ID3DFXEffectShaderResourceVariable * ID3DFXEffectVariable::AsShaderResource()
{
	return (ID3DFXEffectShaderResourceVariable *)this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectVectorVariable::SetFloatVector(float * vect)
{
	if (this)
	{
		if (data->vs_or_ps==__VS)
		{
			memcpy(&fx->data_vs[data->offset],vect,4*sizeof(float));
			fx->dirty_vs_constants=true;
		}
		else
		{
			memcpy(&fx->data_ps[data->offset],vect,4*sizeof(float));
			fx->dirty_ps_constants=true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectVectorVariable::SetVectorArray(float * vects,int index,int nb)
{
	if (this)
	{
		if (data->vs_or_ps==__VS)
		{
			memcpy(&fx->data_vs[data->offset+index*4],vects,4*nb*sizeof(float));
			fx->dirty_vs_constants=true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectMatrixVariable::SetMatrix(float * mat)
{
	if (this)
	{
		if (data->vs_or_ps==__VS)
		{
			memcpy(&fx->data_vs[data->offset],mat,16*sizeof(float));
			fx->dirty_vs_constants=true;
		}
		else
		{
			memcpy(&fx->data_ps[data->offset],mat,16*sizeof(float));
			fx->dirty_vs_constants=true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectMatrixVariable::SetMatrixArray(float * mats,int index,int nb)
{
	if (this)
	{
		memcpy(&fx->data_vs[data->offset+index*16],mats,16*nb*sizeof(float));
		fx->dirty_vs_constants=true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3DFXEffectShaderResourceVariable::SetResource(ID3D11ShaderResourceView *res)
{
	if (this) tex=res;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
