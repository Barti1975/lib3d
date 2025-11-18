
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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(API3D_DIRECT3D)||(API3D_DIRECT3D9)
#else
#define BC3_POSSIBLE
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	NO_ALPHA						=	0,
	ALPHA							=	1,

	TC_NO_ALPHA						=	0,
	TC_ALPHA						=	1,
	TC_NO_DITHERING					=	0,
	TC_DITHERING					=	2,
	TC_CLAMP    					=	4,
	TC_REPEAT    					=	8,
	TC_MIPMAP    					=	16,
	TC_NO_MIPMAP    				=	0
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TEXTURE565 0
#define TEXTURE1555 1
#define TEXTURE4444 2
////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CLightmap
{
public:
	int nt;
	int x,y;
	float * pVectors;
	char * pColors;
	CVector u0,u1,u2,N;
	int offx,offy,sizx,sizy;

	CLightmap() { pVectors=NULL; pColors=NULL; sizx=sizy=4; offx=offy=0; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CMaterial
{
public:
	float ambient[4];
	float diffuse[4];
	float specular[4];
	int power;

	CMaterial()
	{
		for (int n=0;n<4;n++)
		{
			ambient[n]=1.0f;
			diffuse[n]=1.0f;
			specular[n]=1.0f;
		}
		power=20;
	}

	void setPower(int power);
	void setAmbient(float r,float g,float b,float a);
	void setDiffuse(float r,float g,float b,float a);
	void setSpecular(float r,float g,float b,float a);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CQuad
{
public:
	int x1,y1,x2,y2;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTexture
{
public:
	int index;
	int ClampRepeat;
	int Format;
	int Tile;
	int Tilex;
	int Tiley;
	int MipMap;
	int LVL;
	char *Group;
	CMaterial *Material;
	CList <CQuad> LQ;
	unsigned char * ptr_texture;

	CTexture() {Group=NULL; ptr_texture=NULL;}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef API3D_DIRECT3D
	typedef LPDIRECT3DTEXTURE8 API_ID;
#endif

#ifdef API3D_DIRECT3D9
	typedef LPDIRECT3DTEXTURE9 API_ID;
#endif

#ifdef API3D_DIRECT3D10
	typedef ID3D10ShaderResourceView* API_ID;
#endif

#ifdef API3D_DIRECT3D11
	typedef ID3D11ShaderResourceView* API_ID;
#endif

#ifdef API3D_DIRECT3D12
	typedef DescriptorHeapHandle* API_ID;
#endif

#ifdef API3D_OPENGL
	typedef unsigned int API_ID;
#endif

#ifdef API3D_OPENGL20
	typedef unsigned int API_ID;
#endif

#ifdef API3D_METAL
	typedef int API_ID;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTextureAPI
{
public:

	CTexture				data;

	bool					dup;

#ifdef API3D_DIRECT3D
	LPDIRECT3DDEVICE8       D3DDevice; 
	LPDIRECT3DTEXTURE8		id;
#endif

#ifdef API3D_DIRECT3D10
	ID3D10Device*			device;
	ID3D10Texture2D *		idtex;
	ID3D10ShaderResourceView * id;
#endif

#ifdef API3D_DIRECT3D11
	ID3D11Device*			device;
	ID3D11DeviceContext*	devicecontext;
	ID3D11Texture2D *		idtex;
	ID3D11ShaderResourceView * id;
#endif

#ifdef API3D_DIRECT3D12
	DescriptorHeapHandleContainer*	dhhc;
	ID3D12CommandAllocator*			cmdalloc;
	ID3D12CommandQueue*				queue;
	ID3D12Resource*					texture;
	DescriptorHeapHandle			id;
	ID3D12Resource*					textureUpload;
#endif

#ifdef API3D_DIRECT3D9
	LPDIRECT3DDEVICE9       D3DDevice; 
	LPDIRECT3DTEXTURE9		id;
#endif

#ifdef API3D_OPENGL
	unsigned int			id;
#endif

#ifdef API3D_OPENGL20
	unsigned int			id;
#endif

#ifdef API3D_METAL
	unsigned int			id;
#endif

	CTextureAPI();
	~CTextureAPI();

		void					create(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith); 
		void					create16(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith);
		void					create32(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith);
#ifdef BC3_POSSIBLE
		void					createBC3(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith);
#endif
		void					createBump(char * ptr,int sizex,int sizey,float scale);
		void					modifyRGB(int x,int y,int sizex,int sizey,char * ptr);
		void					modifyRGBA(int x,int y,int sizex,int sizey,char * ptr);
		void					release();

#ifdef API3D_DIRECT3D
		void					setAPI(LPDIRECT3DDEVICE8 api);
		LPDIRECT3DTEXTURE8		getApiID();
#endif

#ifdef API3D_DIRECT3D9
		void					setAPI(LPDIRECT3DDEVICE9 api);
		LPDIRECT3DTEXTURE9		getApiID();
#endif

#ifdef API3D_DIRECT3D10
		void					setAPI(ID3D10Device* api);
		ID3D10ShaderResourceView*	getApiID();
#endif

#ifdef API3D_DIRECT3D11
		void					setAPI(ID3D11Device* api,ID3D11DeviceContext* cntxt);
		ID3D11ShaderResourceView*	getApiID();
#endif

#ifdef API3D_DIRECT3D12
		void					setAPI(DescriptorHeapHandleContainer* _dhhc,ID3D12CommandAllocator* _cmdalloc,ID3D12CommandQueue* _queue);
		DescriptorHeapHandle*	getApiID();
#endif

#ifdef API3D_OPENGL
		unsigned int			getApiID();
#endif

#ifdef API3D_OPENGL20
		unsigned int			getApiID();
#endif

#ifdef API3D_METAL
		int						getApiID();
#endif

		CTexture *				getTextureInfo();

		unsigned char*			get();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTextureAPIContainerBase
{
private:

	CList <CTextureAPI>		container;
	CTextureAPI **			references;
	char *					name_group;

public:	

	void *					api;
	
	CTextureAPIContainerBase() { references=NULL; name_group=NULL; }


		void					init(void *apibase);
		void					clean();
		CTextureAPI*			addRef(int n);
		CTextureAPI*			newRef();
		void					setGroup(char *name);
		char*					getGroup();
		void					del(int n);
		void					delAll();
		CTextureAPI*			get(int n);
		bool					is(int n);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CTextureAPIContainer : public CTextureAPIContainerBase
{
public:

	CTextureAPIContainer() {}
	
		// create a texture of dimmensions Sizex,Sizey from a RGBA bitmap at Sizeptrx,y : return texture index
		int add(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int flags);
		// create the texture number n of dimmensions Sizex,Sizey from a RGBA bitmap at Sizeptrx,y 
		void create(int n,int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int flags);
		// create bump map
		void addBump(char * ptr,int sizex,int sizey,float scale);
		// create bump map
		void createBump(int n,char * ptr,int sizex,int sizey,float scale);
		// modify texture
		void modifyRGB(int n,int x,int y,int sizex,int sizey,char * ptr);
		void modifyRGBA(int n,int x,int y,int sizex,int sizey,char * ptr);
		// misc
		void clone(int n_dest,int n_src);
		int addclone(int n_src);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB iCTextureAPIContainer
{
private:
	CTextureAPIContainer * APIContainer;
public:

	iCTextureAPIContainer() {}

	void setTCAPI(CTextureAPIContainer *tcapi)
	{
		APIContainer=tcapi;
	}

	inline CTexture* operator [](int index)
	{
		CTextureAPI *ref=APIContainer->get(index);

		if (ref) return &(ref->data);
		else return NULL;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB tCTextureAPIContainer 
{
private:
	CTextureAPIContainer * APIContainer;

public:

	tCTextureAPIContainer() {}

	void setTCAPI(CTextureAPIContainer *tcapi)
	{
		APIContainer=tcapi;
	}

	inline API_ID operator [](int index)
	{
		CTextureAPI *ref=APIContainer->get(index);
		if (ref) return ref->getApiID();
		else return 0;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB cCTextureAPIContainer 
{
private:
	CTextureAPIContainer * APIContainer;

public:

	cCTextureAPIContainer() {}

	void setTCAPI(CTextureAPIContainer *tcapi)
	{
		APIContainer=tcapi;
	}

	inline CTextureAPI* operator [](int index)
	{
		return APIContainer->get(index);
	}
};



#endif
