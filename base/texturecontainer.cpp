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
//	@file texturecontainer.cpp
//	@date 2004
////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
//	TEXTURE API CONTAINER BASE
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "3d_api_base.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainerBase::init(void *apibase)
{
	api=apibase;
	name_group=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CTextureAPIContainerBase::is(int n)
{
	if (references) return (references[n]!=NULL);
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTextureAPI* CTextureAPIContainerBase::get(int n)
{
	if (references)
	{
		return references[n];
	}
	else return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainerBase::del(int n)
{
	if (references)
	{
		if (references[n])
		{
			references[n]->data.LQ.Free();
			references[n]->release();
			container.Delete(references[n]);
			references[n]=NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainerBase::delAll()
{
	int n;

	if (references)
	{
		if (name_group)
		{
			for (n=0;n<NBRE_MAX_TEXTURES;n++) 
				if (references[n])
				{
					if (references[n]->data.Group)
					if (strcmp(references[n]->data.Group,name_group)==0) del(n);
				}
		}
		else
		{
			for (n=0;n<NBRE_MAX_TEXTURES;n++) 
				if (references[n]) del(n);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainerBase::setGroup(char *name)
{
	name_group=name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
char* CTextureAPIContainerBase::getGroup()
{
	return name_group;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainerBase::clean()
{
	delAll();

	free(references);
	references=NULL;

	container.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTextureAPI* CTextureAPIContainerBase::addRef(int n)
{
	int i;

	if (!references)
	{
		references=(CTextureAPI**) malloc(sizeof(CTextureAPI*) * NBRE_MAX_TEXTURES);
		for (i=0;i<NBRE_MAX_TEXTURES;i++) references[i]=NULL;
	}

	if (references[n])
	{
		references[n]->release();
	}

	references[n]=container.InsertLast();

	references[n]->data.Group=name_group;
	references[n]->data.index=n;

#ifdef API3D_DIRECT3D
	references[n]->setAPI(((C3DAPIBASE *)api)->D3DDevice);
#endif

#ifdef API3D_DIRECT3D9
	references[n]->setAPI(((C3DAPIBASE *)api)->D3DDevice);
#endif

#ifdef API3D_DIRECT3D10
	references[n]->setAPI(((C3DAPIBASE *)api)->device);
#endif

#ifdef API3D_DIRECT3D11
	references[n]->setAPI(((C3DAPIBASE *)api)->device,((C3DAPIBASE *)api)->devicecontext);
#endif

#ifdef API3D_DIRECT3D12
	references[n]->setAPI(&((C3DAPIBASE *)api)->HandleContainer,((C3DAPIBASE *)api)->commandAllocator,((C3DAPIBASE *)api)->commandQueue);
#endif

	return references[n];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTextureAPI* CTextureAPIContainerBase::newRef()
{
	int n;

	if (!references)
	{
		references=(CTextureAPI**) malloc(sizeof(CTextureAPI*) * NBRE_MAX_TEXTURES);
		for (n=0;n<NBRE_MAX_TEXTURES;n++) references[n]=NULL;
	}

	n=0;
	while (references[n]) n++;

	references[n]=container.InsertLast();

	references[n]->data.Group=name_group;
	references[n]->data.index=n;

#ifdef API3D_DIRECT3D
	references[n]->setAPI(((C3DAPIBASE *)api)->D3DDevice);
#endif

#ifdef API3D_DIRECT3D9
	references[n]->setAPI(((C3DAPIBASE *)api)->D3DDevice);
#endif

#ifdef API3D_DIRECT3D10
	references[n]->setAPI(((C3DAPIBASE *)api)->device);
#endif

#ifdef API3D_DIRECT3D11
	references[n]->setAPI(((C3DAPIBASE *)api)->device,((C3DAPIBASE *)api)->devicecontext);
#endif

#ifdef API3D_DIRECT3D12
	references[n]->setAPI(&((C3DAPIBASE *)api)->HandleContainer,((C3DAPIBASE *)api)->commandAllocator,((C3DAPIBASE *)api)->commandQueue);
#endif

	return references[n];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	TEXTURE API CONTAINER
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::clone(int n_dest,int n_src)
{
	CTextureAPI *tex=addRef(n_dest);
	CTextureAPI *texsrc=get(n_src);

	tex->data=texsrc->data;
	tex->id=texsrc->id;
	tex->dup=true;

#ifdef API3D_DIRECT3D
	tex->D3DDevice=texsrc->D3DDevice;
#endif

#ifdef API3D_DIRECT3D9
	tex->D3DDevice=texsrc->D3DDevice;
#endif

#ifdef API3D_DIRECT3D10
	tex->idtex=texsrc->idtex;
	tex->device=texsrc->device;
#endif

#ifdef API3D_DIRECT3D11
	tex->idtex=texsrc->idtex;
	tex->device=texsrc->device;
	tex->devicecontext=texsrc->devicecontext;
#endif

#ifdef API3D_DIRECT3D12
	tex->texture=texsrc->texture;
	tex->cmdalloc=texsrc->cmdalloc;
	tex->queue=texsrc->queue;
	tex->dhhc=texsrc->dhhc;
#endif

}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CTextureAPIContainer::addclone(int n_src)
{
	CTextureAPI *tex=newRef();
	CTextureAPI *texsrc=get(n_src);

	tex->data=texsrc->data;
	tex->id=texsrc->id;
	tex->dup=true;

#ifdef API3D_DIRECT3D
	tex->D3DDevice=texsrc->D3DDevice;
#endif

#ifdef API3D_DIRECT3D9
	tex->D3DDevice=texsrc->D3DDevice;
#endif

#ifdef API3D_DIRECT3D10
	tex->idtex=texsrc->idtex;
	tex->device=texsrc->device;
#endif

#ifdef API3D_DIRECT3D11
	tex->idtex=texsrc->idtex;
	tex->device=texsrc->device;
	tex->devicecontext=texsrc->devicecontext;
#endif

#ifdef API3D_DIRECT3D12
	tex->texture=texsrc->texture;
	tex->cmdalloc=texsrc->cmdalloc;
	tex->queue=texsrc->queue;
	tex->dhhc=texsrc->dhhc;
#endif

	return tex->data.index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CTextureAPIContainer::add(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int flags)
{
	CTextureAPI *tex;
	int alpha,clamp,mipmap,dithering;

	if (flags&TC_ALPHA) alpha=1;
	else alpha=0;

	if (flags&TC_CLAMP) clamp=1;
	else clamp=0;

	if (flags&TC_MIPMAP) mipmap=1;
	else mipmap=0;

	if (flags&TC_DITHERING) dithering=1;
	else dithering=0;

	tex=newRef();

	if (((C3DAPIBASE *)api)->states(TEXTURES_32BITS)) tex->create32(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,alpha,clamp,mipmap,dithering);
	else tex->create(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,alpha,clamp,mipmap,dithering);

	return tex->data.index;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::create(int n,int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int flags)
{
	CTextureAPI *tex;
	int alpha,clamp,mipmap,dithering;

	if (flags&TC_ALPHA) alpha=1;
	else alpha=0;

	if (flags&TC_CLAMP) clamp=1;
	else clamp=0;

	if (flags&TC_MIPMAP) mipmap=1;
	else mipmap=0;

	if (flags&TC_DITHERING) dithering=1;
	else dithering=0;

	tex=addRef(n);
//	tex->create(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,alpha,clamp,mipmap,dithering);
	if (((C3DAPIBASE *)api)->states(TEXTURES_32BITS)) tex->create32(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,alpha,clamp,mipmap,dithering);
	else tex->create(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,alpha,clamp,mipmap,dithering);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::addBump(char * ptr,int sizex,int sizey,float scale)
{
	CTextureAPI *tex;
	tex=newRef();
	tex->createBump(ptr,sizex,sizey,scale);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::createBump(int n,char * ptr,int sizex,int sizey,float scale)
{
	CTextureAPI *tex;
	tex=addRef(n);
	tex->createBump(ptr,sizex,sizey,scale);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::modifyRGB(int n,int x,int y,int sizex,int sizey,char * ptr)
{
	CTextureAPI *tex;
	tex=get(n);
	tex->modifyRGB(x,y,sizex,sizey,ptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPIContainer::modifyRGBA(int n,int x,int y,int sizex,int sizey,char * ptr)
{
	CTextureAPI *tex;
	tex=get(n);
	tex->modifyRGBA(x,y,sizex,sizey,ptr);
}
