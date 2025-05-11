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
//	@file texture.cpp
//	@date 2004
////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////
//	TEXTURE API
///////////////////////////////////////////////////////////////////////////////////////////////////

// 	if (Tile>(int)MAX_TEXTURE_WIDTH) Tile=MAX_TEXTURE_WIDTH;

#include "3d_api_base.h"
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(API3D_METAL)
int MTLAddTexture(void *tex,int w,int h);
int MTLAddTextureMipMap(void *tex,int w,int h,int levels);
void MTLFreeTexture(int n);
void MTLUpdateTexture(int id,int x,int y,int sizex,int sizey,char *ptr);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
CTextureAPI::CTextureAPI()
{
#ifdef API3D_METAL
	id=-1;
#else
#ifndef API3D_DIRECT3D12
	id=0;
#else
	texture=NULL;
	textureUpload=NULL;
#endif
#ifdef API3D_DIRECT3D10
	idtex=0;
#endif
#ifdef API3D_DIRECT3D11
	idtex=0;
#endif
#endif
	dup=false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTextureAPI::~CTextureAPI()
{
	release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  function:
//		Create sub mipmap texture
///////////////////////////////////////////////////////////////////////////////////////////////////		
  
char dither[5][2][2]={ { 1,1,1,1},{1,1,0,1},{1,0,0,1},{1,0,0,0},{0,0,0,0}};

#define MIPMAPCOLOR(RGBA)											\
{																	\
	c1=((unsigned char*)ptrtex)[adr1+RGBA];							\
	c2=((unsigned char*)ptrtex)[adr2+RGBA];							\
	if (n1>0) c3=((unsigned char*)ptrtex)[adr3+RGBA]; else c3=c1;	\
	if (n2>0) c4=((unsigned char*)ptrtex)[adr4+RGBA]; else c4=c1;	\
	c5=((unsigned char*)ptrtex)[adr5+RGBA];							\
}


#define MIPMAPCOLORN1N2(RGBA)										\
{																	\
	c1=((unsigned char*)ptrtex)[adr1+RGBA];							\
	c2=((unsigned char*)ptrtex)[adr2+RGBA];							\
	c3=((unsigned char*)ptrtex)[adr3+RGBA];							\
	c4=((unsigned char*)ptrtex)[adr4+RGBA];							\
	c5=((unsigned char*)ptrtex)[adr5+RGBA];							\
}

#define MIPMAPCOLORM1N2(RGBA)										\
{																	\
	c1=((unsigned char*)ptrtex)[adr1+RGBA];							\
	c2=((unsigned char*)ptrtex)[adr2+RGBA];							\
	c3=c1;															\
	c4=((unsigned char*)ptrtex)[adr4+RGBA];							\
	c5=((unsigned char*)ptrtex)[adr5+RGBA];							\
}

#define MIPMAPCOLORN1M2(RGBA)										\
{																	\
	c1=((unsigned char*)ptrtex)[adr1+RGBA];							\
	c2=((unsigned char*)ptrtex)[adr2+RGBA];							\
	c3=((unsigned char*)ptrtex)[adr3+RGBA];							\
	c4=c1;															\
	c5=((unsigned char*)ptrtex)[adr5+RGBA];							\
}

#define MIPMAPCOLORM1M2(RGBA)										\
{																	\
	c1=((unsigned char*)ptrtex)[adr1+RGBA];							\
	c2=((unsigned char*)ptrtex)[adr2+RGBA];							\
	c3=c1;															\
	c4=c1;															\
	c5=((unsigned char*)ptrtex)[adr5+RGBA];							\
}


char * CreateMipMap(char * ptrtex,int tilex,int tiley,int rgba)
{
	int adr,adr0;
	int n1,n2;
	int c1,c2,c3,c4,c5;
	int tilex2;
	int tiley2;
	char *ptr2;
	int r,g,b,a;
	unsigned int adrs=0;
	unsigned int adr1;
	unsigned int adr2;
	unsigned int adr3;
	unsigned int adr4;
	unsigned int adr5;


	tilex2=tilex>>1;
	tiley2=tiley>>1;

	ptr2=(char*) malloc(tilex2*tiley2*rgba);

	n2=0;
	adr=(n2*tilex)<<1;
	adr0=n2*tilex2;

	n1=0;
	adr1=((adr+(n1<<1)+0)<<2);
	adr2=((adr+(n1<<1)+1)<<2);
	adr3=((adr+(n1<<1)-1)<<2);
	adr4=((adr+(n1<<1)-tilex)<<2);
	adr5=((adr+(n1<<1)+tilex)<<2);

	// rouge
	MIPMAPCOLORM1M2(0);
	r=(4*c1 +c2+c3+c4+c5)>>3;

	// vert
	MIPMAPCOLORM1M2(1);
	g=(4*c1 +c2+c3+c4+c5)>>3;

	// bleu
	MIPMAPCOLORM1M2(2);
	b=(4*c1 +c2+c3+c4+c5)>>3;

	// alpha
	MIPMAPCOLORM1M2(3);
	a=(4*c1 +c2+c3+c4+c5)>>3;

	ptr2[adrs+0]=r;
	ptr2[adrs+1]=g;
	ptr2[adrs+2]=b;
	ptr2[adrs+3]=a;

	adrs+=rgba;

	for (n1=1;n1<tilex2;n1++)
	{
		// rgba always equal to 4
		adr1=((adr+(n1<<1)+0)<<2);
		adr2=((adr+(n1<<1)+1)<<2);
		adr3=((adr+(n1<<1)-1)<<2);
		adr4=((adr+(n1<<1)-tilex)<<2);
		adr5=((adr+(n1<<1)+tilex)<<2);

		// rouge
		MIPMAPCOLORN1M2(0);
		r=(4*c1 +c2+c3+c4+c5)>>3;

		// vert
		MIPMAPCOLORN1M2(1);
		g=(4*c1 +c2+c3+c4+c5)>>3;

		// bleu
		MIPMAPCOLORN1M2(2);
		b=(4*c1 +c2+c3+c4+c5)>>3;

		// alpha
		MIPMAPCOLORN1M2(3);
		a=(4*c1 +c2+c3+c4+c5)>>3;

		ptr2[adrs+0]=r;
		ptr2[adrs+1]=g;
		ptr2[adrs+2]=b;
		ptr2[adrs+3]=a;

		adrs+=rgba;
	}


	for (n2=1;n2<tiley2;n2++)
	{
		adr=(n2*tilex)<<1;
		adr0=n2*tilex2;


		n1=0;
		// rgba always equal to 4
		adr1=((adr+(n1<<1)+0)<<2);
		adr2=((adr+(n1<<1)+1)<<2);
		adr3=((adr+(n1<<1)-1)<<2);
		adr4=((adr+(n1<<1)-tilex)<<2);
		adr5=((adr+(n1<<1)+tilex)<<2);

		// rouge
		MIPMAPCOLORM1N2(0);
		r=(4*c1 +c2+c3+c4+c5)>>3;

		// vert
		MIPMAPCOLORM1N2(1);
		g=(4*c1 +c2+c3+c4+c5)>>3;

		// bleu
		MIPMAPCOLORM1N2(2);
		b=(4*c1 +c2+c3+c4+c5)>>3;

		// alpha
		MIPMAPCOLORM1N2(3);
		a=(4*c1 +c2+c3+c4+c5)>>3;

		ptr2[adrs+0]=r;
		ptr2[adrs+1]=g;
		ptr2[adrs+2]=b;
		ptr2[adrs+3]=a;

		adrs+=rgba;

		for (n1=1;n1<tilex2;n1++)
		{
			// rgba always equal to 4
			adr1=((adr+(n1<<1)+0)<<2);
			adr2=adr1+4;
			adr3=adr1-4;
			adr4=adr2-(tilex<<2);
			adr5=adr2+(tilex<<2);

			// rouge
			MIPMAPCOLORN1N2(0);
			r=(4*c1 +c2+c3+c4+c5)>>3;

			// vert
			MIPMAPCOLORN1N2(1);
			g=(4*c1 +c2+c3+c4+c5)>>3;

			// bleu
			MIPMAPCOLORN1N2(2);
			b=(4*c1 +c2+c3+c4+c5)>>3;

			// alpha
			MIPMAPCOLORN1N2(3);
			a=(4*c1 +c2+c3+c4+c5)>>3;

			ptr2[adrs+0]=r;
			ptr2[adrs+1]=g;
			ptr2[adrs+2]=b;
			ptr2[adrs+3]=a;

			adrs+=rgba;
		}
	}

	return ptr2;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::create(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith)
{
	
#if defined(API3D_DIRECT3D12) || defined(API3D_DIRECT3D11) || defined(API3D_DIRECT3D10) || defined(API3D_OPENGL20) || defined(API3D_METAL)

	create32(Sizex,Sizey,ptrRGBA,Sizeptrx,Sizeptry,Alpha,Clamp,MipMap,dith);
	return;

#else

	data.ClampRepeat=Clamp;
	data.MipMap=MipMap;
	if (Sizex>Sizey) data.Tile=Sizex;
	else data.Tile=Sizey;
	data.Format=0;
	data.Material=NULL;
	data.Tilex=Sizex;
	data.Tiley=Sizey;

#if defined(API3D_OPENGL)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	int nn=0;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;

	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#if !defined(GLES)&&!defined(GLES20)
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
#endif
	if (Alpha==1)
	{
		ptrtex=(char*) malloc(Tilex*Tiley*4);

		if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
		{
			memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
		}
		else
		{
			adr2=0;
			for (n2=0;n2<Tiley;n2++)
			{
				y=(BASE_TILEy*n2)/Tiley;
				adr=y*BASE_TILEx;
				xx=0;
				incxx=(BASE_TILEx*256)/Tilex;
				for (n1=0;n1<Tilex;n1++)
				{
					x=(xx>>8);
					ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
					ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
					ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
					ptrtex[adr2+ 3]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
					xx+=incxx;
					adr2+=4;
				}
			}
		}

		id=0;

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1,(GLuint*)&id);
		glBindTexture(GL_TEXTURE_2D,id);

		if (MipMap==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}

#if defined(GLES)||defined(GLES20)
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifndef GLES20
		if (MipMap!=0) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
#else
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		if (MipMap!=0) glGenerateMipmap(GL_TEXTURE_2D);
#endif

#else
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,Tilex,Tiley,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		}
#endif
	}
	else
	{
		ptrtex=(char*) malloc(Tilex*Tiley*3);

		if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
		{
			unsigned char r,g,b;
			adr=0;
			adr2=0;
			for (n1=0;n1<Tilex*Tiley;n1++)
			{
				r=((unsigned char*)ptrRGBA)[adr + 0];
				g=((unsigned char*)ptrRGBA)[adr + 1];
				b=((unsigned char*)ptrRGBA)[adr + 2];

				ptrtex[adr2+0]=r;
				ptrtex[adr2+1]=g;
				ptrtex[adr2+2]=b;
				adr2+=3;
				adr+=4;
			}
		}
		else
		{
			adr2=0;
			for (n2=0;n2<Tiley;n2++)
			{
				y=(BASE_TILEy*n2)/Tiley;
				adr=y*BASE_TILEx;
				xx=0;
				incxx=(BASE_TILEx*256)/Tilex;
				for (n1=0;n1<Tilex;n1++)
				{
					x=(xx>>8);
					ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
					ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
					ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
					xx+=incxx;
					adr2+=3;
				}
			}
		}
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1,(GLuint*) &id);			
		glBindTexture(GL_TEXTURE_2D,id);

		if (MipMap==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}

#if defined(GLES)||defined(GLES20)

		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifndef GLES20
		if (MipMap!=0) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
#else
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		if (MipMap!=0) glGenerateMipmap(GL_TEXTURE_2D);
#endif

#else
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,Tilex,Tiley,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		}
#endif

	}

	free(ptrtex);
#endif


#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	D3DLOCKED_RECT Locked;
	int lvl,nn;
	unsigned int adr0,pitch;
	WORD * ptrdest;
	WORD cc;
	unsigned char r,g,b,a;

	D3DFORMAT Formats[3]={
					    D3DFMT_R5G6B5,      
						D3DFMT_X1R5G5B5,    
						D3DFMT_A4R4G4B4
						};
	D3DFORMAT Formats2[6]={
						D3DFMT_R8G8B8,    
						D3DFMT_X8R8G8B8,
					    D3DFMT_R5G6B5,      
						D3DFMT_X1R5G5B5,    
						D3DFMT_A8R8G8B8,    
						D3DFMT_A4R4G4B4
						};
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	int r0,g0,b0;
	char * Ptr2;
	char * Ptr3;
	int Tilex2;
	int Tiley2;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while (!((Tile>>LEVELS)&1)) LEVELS++;
		LEVELS++;
	}

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	
	adr2=0;
	for (n2=0;n2<Tiley;n2++)
	{
		y=(BASE_TILEy*n2)/Tiley;
		adr=y*BASE_TILEx;
		xx=0;
		incxx=(BASE_TILEx*256)/Tilex;
		for (n1=0;n1<Tilex;n1++)
		{
			x=(xx>>8);

			r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
			g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
			b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
			a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];

			if (dith==1)
			{
				r0=r&7;
				if (r0>4) r0=4;
				r0=dither[4-r0][x&1][y&1];
				r=((r>>3)+r0)<<3;

				if (r>255) r=255;


				g0=g&3;
				g0=dither[4-g0][x&1][y&1];
				g=((g>>2)+g0)<<2;

				if (g>255) g=255;

				b0=b&7;
				if (b0>4) b0=4;
				b0=dither[4-b0][x&1][y&1];
				b=((b>>3)+b0)<<3;
				if (b>255) b=255;
			}
			ptrtex[adr2+ 0]=r;
			ptrtex[adr2+ 1]=g;
			ptrtex[adr2+ 2]=b;
			ptrtex[adr2+ 3]=a;
			xx+=incxx;
			adr2+=4;
		}
	}

	

	if (Alpha==0) { nn=0;  }
	else { nn=2;  }

	while ((nn<3)&&(FAILED(D3DDevice->CreateTexture(Tilex,Tiley,LEVELS,0,Formats[nn],D3DPOOL_MANAGED,&id)))) nn++;

	if (nn<3)
	{	
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(WORD *) Locked.pBits;
		pitch=Locked.Pitch>>1;
		
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				switch (nn)
				{
				case 0:	//565
					cc=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
					break;
				case 1:	//555
					cc=((r>>3)<<10) +((g>>3)<<5) + (b>>3);
					break;
				case 2:	//4444
					cc=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
					break;
				}
				
				ptrdest[n1+adr]=cc;
			}
		}

		id->UnlockRect(0);

		if (MipMap==1)
		{
			Ptr2=CreateMipMap(ptrtex,Tilex,Tiley,4);

			for (lvl=1;lvl<LEVELS;lvl++)
			{
				id->LockRect(lvl,&Locked,NULL,0);
				ptrdest=(WORD *) Locked.pBits;
				pitch=Locked.Pitch>>1;
				Tilex2=Tilex>>1;
				Tiley2=Tiley>>1;

				adr=0;
				for (n2=0;n2<Tiley2;n2++)
				{
					adr0=n2*pitch;
					for (n1=0;n1<Tilex2;n1++)
					{
						r=((unsigned char*)Ptr2)[adr+0];
						g=((unsigned char*)Ptr2)[adr+1];
						b=((unsigned char*)Ptr2)[adr+2];
						a=((unsigned char*)Ptr2)[adr+3];

						switch (nn)
						{
						case 0:	//565
							cc=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
							break;
						case 1:	//555
							cc=((r>>3)<<10) +((g>>3)<<5) + (b>>3);
							break;
						case 2:	//4444
							cc=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
							break;
						};
						
						ptrdest[n1+adr0]=cc;
						adr+=4;
					}
				}

				id->UnlockRect(lvl);
				Ptr3=CreateMipMap(Ptr2,Tilex>>1,Tiley>>1,4);
				free(Ptr2);
				Ptr2=Ptr3;
				Tilex=Tilex>>1;
				Tiley=Tiley>>1;
			}
			free(Ptr2);
		}
	}
	free(ptrtex);

	data.Format=nn;
#endif


#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	D3DLOCKED_RECT Locked;
	int lvl,nn;
	unsigned int adr0,pitch;
	WORD * ptrdest;
	WORD cc;
	int r,g,b,a;

	D3DFORMAT Formats[3]={
					    D3DFMT_R5G6B5,      
						D3DFMT_X1R5G5B5,    
						D3DFMT_A4R4G4B4
						};
	D3DFORMAT Formats2[6]={
						D3DFMT_R8G8B8,    
						D3DFMT_X8R8G8B8,
					    D3DFMT_R5G6B5,      
						D3DFMT_X1R5G5B5,    
						D3DFMT_A8R8G8B8,    
						D3DFMT_A4R4G4B4
						};
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	int r0,g0,b0;
	char * Ptr2;
	char * Ptr3;
	int Tilex2;
	int Tiley2;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while (!((Tile>>LEVELS)&1)) LEVELS++;
		LEVELS++;
	}

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
	{
		memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
	}
	else
	{
		adr2=0;
		for (n2=0;n2<Tiley;n2++)
		{
			y=(BASE_TILEy*n2)/Tiley;
			adr=y*BASE_TILEx;
			xx=0;
			incxx=(BASE_TILEx*256)/Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				x=(xx>>8);

				r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
				g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
				b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
				a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];

				if (dith==1)
				{
					r0=r&7;
					if (r0>4) r0=4;
					r0=dither[4-r0][x&1][y&1];
					r=((r>>3)+r0)<<3;

					if (r>255) r=255;


					g0=g&3;
					g0=dither[4-g0][x&1][y&1];
					g=((g>>2)+g0)<<2;

					if (g>255) g=255;

					b0=b&7;
					if (b0>4) b0=4;
					b0=dither[4-b0][x&1][y&1];
					b=((b>>3)+b0)<<3;
					if (b>255) b=255;
				}
				ptrtex[adr2+ 0]=r;
				ptrtex[adr2+ 1]=g;
				ptrtex[adr2+ 2]=b;
				ptrtex[adr2+ 3]=a;
				xx+=incxx;
				adr2+=4;
			}
		}
	}

	if (Alpha==0) { nn=0;  }
	else { nn=2;  }

	while ((nn<3)&&(FAILED(D3DDevice->CreateTexture(Tilex,Tiley,LEVELS-1,0,Formats[nn],D3DPOOL_MANAGED,&id,NULL)))) nn++;

	if (nn<3)
	{

		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(WORD *) Locked.pBits;
		pitch=Locked.Pitch>>1;
		
		if (nn==0)
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				cc=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
				
				ptrdest[n1+adr]=cc;
			}
		}


		if (nn==1)
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				cc=((r>>3)<<10) +((g>>3)<<5) + (b>>3);				
				ptrdest[n1+adr]=cc;
			}
		}


		if (nn==2)
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				cc=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
				ptrdest[n1+adr]=cc;
			}
		}



		id->UnlockRect(0);

		if (MipMap==1)
		{
			Ptr2=CreateMipMap(ptrtex,Tilex,Tiley,4);

			for (lvl=1;lvl<LEVELS-1;lvl++)
			{
				id->LockRect(lvl,&Locked,NULL,0);
				ptrdest=(WORD *) Locked.pBits;
				pitch=Locked.Pitch>>1;
				Tilex2=Tilex>>1;
				Tiley2=Tiley>>1;

				adr=0;
				for (n2=0;n2<Tiley2;n2++)
				{
					adr0=n2*pitch;
					for (n1=0;n1<Tilex2;n1++)
					{
						r=((unsigned char*)Ptr2)[adr+0];
						g=((unsigned char*)Ptr2)[adr+1];
						b=((unsigned char*)Ptr2)[adr+2];
						a=((unsigned char*)Ptr2)[adr+3];

						switch (nn)
						{
						case 0:	//565
							cc=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
							break;
						case 1:	//555
							cc=((r>>3)<<10) +((g>>3)<<5) + (b>>3);
							break;
						case 2:	//4444
							cc=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
							break;
						};
						
						ptrdest[n1+adr0]=cc;
						adr+=4;
					}
				}

				id->UnlockRect(lvl);
				Ptr3=CreateMipMap(Ptr2,Tilex>>1,Tiley>>1,4);
				free(Ptr2);
				Ptr2=Ptr3;
				Tilex=Tilex>>1;
				Tiley=Tiley>>1;
			}
			free(Ptr2);
		}
	}
	free(ptrtex);

	data.Format=nn;
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::create32(int Sizex,int Sizey,char * ptrRGBA,int Sizeptrx,int Sizeptry,int Alpha,int Clamp,int MipMap,int dith)
{
	data.ClampRepeat=Clamp;
	data.MipMap=MipMap;
	if (Sizex>Sizey) data.Tile=Sizex;
	else data.Tile=Sizey;
	data.Format=0;
	data.Material=NULL;
	data.Tilex=Sizex;
	data.Tiley=Sizey;

#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -------------
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	int Tile;
	char * ptrtex;
	int LEVELS;
    int x,y,n1,n2,adr,adr2;
    int xx,incxx;
    
	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;

	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	if (Alpha==1)
	{
		ptrtex=(char*) malloc(Tilex*Tiley*4);
        
        if ((Tilex==BASE_TILEx)&&(Tiley==BASE_TILEy))
        {
            memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
        }
        else
        {
            adr2=0;
            for (n2=0;n2<Tiley;n2++)
            {
                y=(BASE_TILEy*n2)/Tiley;
                adr=y*BASE_TILEx;
                xx=0;
                incxx=(BASE_TILEx*256)/Tilex;
                for (n1=0;n1<Tilex;n1++)
                {
                    x=(xx>>8);
                    ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
                    ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
                    ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
                    ptrtex[adr2+ 3]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
                    xx+=incxx;
                    adr2+=4;
                }
            }
        }

		if (MipMap==0) id=MTLAddTexture(ptrtex,Tilex,Tiley);
		else id=MTLAddTextureMipMap(ptrtex,Tilex,Tiley,LEVELS);
	}
	else
	{
		ptrtex=(char*) malloc(Tilex*Tiley*4);

        if ((Tilex==BASE_TILEx)&&(Tiley==BASE_TILEy))
        {
            memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
            adr2=0;
            for (n1=0;n1<Tilex*Tiley;n1++)
            {
                ptrtex[adr2+3]=255;
                adr2+=4;
            }
        }
        else
        {
            adr2=0;
            for (n2=0;n2<Tiley;n2++)
            {
                y=(BASE_TILEy*n2)/Tiley;
                adr=y*BASE_TILEx;
                xx=0;
                incxx=(BASE_TILEx*256)/Tilex;
                for (n1=0;n1<Tilex;n1++)
                {
                    x=(xx>>8);
                    ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
                    ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
                    ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
                    ptrtex[adr2+ 3]=255;
                    xx+=incxx;
                    adr2+=4;
                }
            }
        }
		if (MipMap==0) id=MTLAddTexture(ptrtex,Tilex,Tiley);
		else id=MTLAddTextureMipMap(ptrtex,Tilex,Tiley,LEVELS);
	}

	free(ptrtex);
#endif


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	int Tile;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	int LEVELS;
	int nn;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	nn=0;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;

	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
#if !defined(GLES)&&!defined(GLES20)
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
#endif

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

#if !defined(GLES)&&!defined(GLES20)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, LEVELS);
#endif

	if (Alpha==1)
	{

		ptrtex=(char*) malloc(Tilex*Tiley*4);

        if ((Tilex==BASE_TILEx)&&(Tiley==BASE_TILEy))
        {
            memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
        }
        else
        {
            adr2=0;
            for (n2=0;n2<Tiley;n2++)
            {
                y=(BASE_TILEy*n2)/Tiley;
                adr=y*BASE_TILEx;
                xx=0;
                incxx=(BASE_TILEx*256)/Tilex;
                for (n1=0;n1<Tilex;n1++)
                {
                    x=(xx>>8);
                    ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
                    ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
                    ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
                    ptrtex[adr2+ 3]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
                    xx+=incxx;
                    adr2+=4;
                }
            }
        }
        
		id=0;

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1,(GLuint*)&id);
		glBindTexture(GL_TEXTURE_2D,id);

		if (MipMap==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}

#if defined(GLES)||defined(GLES20)
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifndef GLES20
		if (MipMap!=0) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
#else
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		if (MipMap!=0) glGenerateMipmap(GL_TEXTURE_2D);
#endif

#else
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifdef OCULUS
		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
#else
		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,Tilex,Tiley,GL_RGBA,GL_UNSIGNED_BYTE,ptrtex);
		}
#endif
#endif
	}
	else
	{
		ptrtex=(char*) malloc(Tilex*Tiley*3);
	
        if ((Tilex==BASE_TILEx)&&(Tiley==BASE_TILEy))
        {
            adr2=0;
            adr=0;
            for (n2=0;n2<Tilex*Tiley;n2++)
            {
                ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[adr + 0];
                ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[adr + 1];
                ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[adr + 2];
                adr2+=3;
                adr+=4;                
            }
        }
        else
        {
            adr2=0;
            for (n2=0;n2<Tiley;n2++)
            {
                y=(BASE_TILEy*n2)/Tiley;
                adr=y*BASE_TILEx;
                xx=0;
                incxx=(BASE_TILEx*256)/Tilex;
                for (n1=0;n1<Tilex;n1++)
                {
                    x=(xx>>8);
                    ptrtex[adr2+ 0]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
                    ptrtex[adr2+ 1]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
                    ptrtex[adr2+ 2]=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
                    xx+=incxx;
                    adr2+=3;
                }
            }
        }
        
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1,(GLuint*) &id);
		glBindTexture(GL_TEXTURE_2D,id);

		if (MipMap==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}

#if defined(GLES)||defined(GLES20)
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifndef GLES20
		if (MipMap!=0) glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
#else
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		if (MipMap!=0) glGenerateMipmap(GL_TEXTURE_2D);
#endif

#else
		if (Clamp==1)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

#ifdef OCULUS
		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
#else
		if (MipMap==0)
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Tilex,Tiley,0,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		else
		{
			gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,Tilex,Tiley,GL_RGB,GL_UNSIGNED_BYTE,ptrtex);
		}
#endif

#endif
	}

	free(ptrtex);
#endif


#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	D3DLOCKED_RECT Locked;
	int lvl,nn;
	unsigned int adr0,pitch;
	DWORD * ptrdest;
	DWORD cc;
	int r,g,b,a;

	D3DFORMAT Formats[2]={
						D3DFMT_X8R8G8B8,
						D3DFMT_A8R8G8B8,    
						};
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	char * Ptr2;
	char * Ptr3;
	int Tilex2;
	int Tiley2;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	ptrtex=(char*) malloc(Tilex*Tiley*4);
	
	adr2=0;
	for (n2=0;n2<Tiley;n2++)
	{
		y=(BASE_TILEy*n2)/Tiley;
		adr=y*BASE_TILEx;
		xx=0;
		incxx=(BASE_TILEx*256)/Tilex;
		for (n1=0;n1<Tilex;n1++)
		{
			x=(xx>>8);

			r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
			g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
			b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
			a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
			ptrtex[adr2+ 0]=r;
			ptrtex[adr2+ 1]=g;
			ptrtex[adr2+ 2]=b;
			ptrtex[adr2+ 3]=a;
			xx+=incxx;
			adr2+=4;
		}
	}


	if (Alpha==0) { nn=0;  }
	else { nn=1;  }

	while ((nn<2)&&(FAILED(D3DDevice->CreateTexture(Tilex,Tiley,LEVELS,0,Formats[nn],D3DPOOL_MANAGED,&id)))) nn++;

	if (nn<2)
	{
		
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(DWORD *) Locked.pBits;
		pitch=Locked.Pitch/4;
		
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				cc=(a<<24) + (r<<16) + (g<<8) +b;
			
				ptrdest[n1+adr]=cc;
			}
		}

		id->UnlockRect(0);

		if (MipMap==1)
		{
			Ptr2=CreateMipMap(ptrtex,Tilex,Tiley,4);

			for (lvl=1;lvl<LEVELS;lvl++)
			{
				id->LockRect(lvl,&Locked,NULL,0);
				ptrdest=(DWORD *) Locked.pBits;
				pitch=Locked.Pitch/4;
				Tilex2=Tilex>>1;
				Tiley2=Tiley>>1;

				adr=0;
				for (n2=0;n2<Tiley2;n2++)
				{
					adr0=n2*pitch;
					for (n1=0;n1<Tilex2;n1++)
					{
						r=((unsigned char*)Ptr2)[adr+0];
						g=((unsigned char*)Ptr2)[adr+1];
						b=((unsigned char*)Ptr2)[adr+2];
						a=((unsigned char*)Ptr2)[adr+3];

						cc=(a<<24) + (r<<16) + (g<<8) +b;

						ptrdest[n1+adr0]=cc;
						adr+=4;
					}
				}

				id->UnlockRect(lvl);
				Ptr3=CreateMipMap(Ptr2,Tilex>>1,Tiley>>1,4);
				free(Ptr2);
				Ptr2=Ptr3;
				Tilex=Tilex>>1;
				Tiley=Tiley>>1;
			}
			free(Ptr2);
		}
	}
	free(ptrtex);

	data.Format=-1-nn;
#endif


#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	D3DLOCKED_RECT Locked;
	int lvl,nn;
	unsigned int adr0,pitch;
	DWORD * ptrdest;
	DWORD cc;
	unsigned char r,g,b,a;

	D3DFORMAT Formats[6]={
						D3DFMT_X8R8G8B8,
						D3DFMT_A8R8G8B8,    
						};
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	char * Ptr2;
	char * Ptr3;
	int Tilex2;
	int Tiley2;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
	{
		memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
	}
	else
	{

		adr2=0;
		for (n2=0;n2<Tiley;n2++)
		{
			y=(BASE_TILEy*n2)/Tiley;
			adr=y*BASE_TILEx;
			xx=0;
			incxx=(BASE_TILEx*256)/Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				x=(xx>>8);

				r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
				g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
				b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
				a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
				ptrtex[adr2+ 0]=r;
				ptrtex[adr2+ 1]=g;
				ptrtex[adr2+ 2]=b;
				ptrtex[adr2+ 3]=a;
				xx+=incxx;
				adr2+=4;
			}
		}
	}

	if (Alpha==0) { nn=0; }
	else { nn=1;  }

	while ((nn<2)&&(FAILED(D3DDevice->CreateTexture(Tilex,Tiley,LEVELS,0,Formats[nn],D3DPOOL_MANAGED,&id,NULL)))) nn++;

	if (nn<2)
	{
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(DWORD *) Locked.pBits;
		pitch=Locked.Pitch/4;
		
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				r=((unsigned char*)ptrtex)[((n1+adr0)<<2)+0];
				g=((unsigned char*)ptrtex)[((n1+adr0)<<2)+1];
				b=((unsigned char*)ptrtex)[((n1+adr0)<<2)+2];
				a=((unsigned char*)ptrtex)[((n1+adr0)<<2)+3];

				cc=(a<<24) + (r<<16) + (g<<8) +b;
			
				ptrdest[n1+adr]=cc;
			}
		}

		id->UnlockRect(0);

		if (MipMap==1)
		{
			Ptr2=CreateMipMap(ptrtex,Tilex,Tiley,4);

			for (lvl=1;lvl<LEVELS;lvl++)
			{
				id->LockRect(lvl,&Locked,NULL,0);
				ptrdest=(DWORD *) Locked.pBits;
				pitch=Locked.Pitch/4;
				Tilex2=Tilex>>1;
				Tiley2=Tiley>>1;

				adr=0;
				for (n2=0;n2<Tiley2;n2++)
				{
					adr0=n2*pitch;
					for (n1=0;n1<Tilex2;n1++)
					{
						r=((unsigned char*)Ptr2)[adr+0];
						g=((unsigned char*)Ptr2)[adr+1];
						b=((unsigned char*)Ptr2)[adr+2];
						a=((unsigned char*)Ptr2)[adr+3];

						cc=(a<<24) + (r<<16) + (g<<8) +b;

						ptrdest[n1+adr0]=cc;
						adr+=4;
					}
				}

				id->UnlockRect(lvl);
				Ptr3=CreateMipMap(Ptr2,Tilex>>1,Tiley>>1,4);
				free(Ptr2);
				Ptr2=Ptr3;
				Tilex=Tilex>>1;
				Tiley=Tiley>>1;
			}
			free(Ptr2);
		}
	}
	free(ptrtex);

	data.Format=-1-nn;

#endif

#ifdef API3D_DIRECT3D10
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	unsigned int adr0,pitch;
	DWORD * ptrdest;
	unsigned char r,g,b,a;
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	char * Ptr2;
	char * Ptr3;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	data.LVL=LEVELS;

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
	{
		memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
	}
	else
	{
		adr2=0;
		for (n2=0;n2<Tiley;n2++)
		{
			y=(BASE_TILEy*n2)/Tiley;
			adr=y*BASE_TILEx;
			xx=0;
			incxx=(BASE_TILEx*256)/Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				x=(xx>>8);
				r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
				g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
				b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
				a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
				ptrtex[adr2+ 0]=r;
				ptrtex[adr2+ 1]=g;
				ptrtex[adr2+ 2]=b;
				ptrtex[adr2+ 3]=a;
				xx+=incxx;
				adr2+=4;
			}
		}
	}

	D3D10_TEXTURE2D_DESC desc;

	if (MipMap==0)
	{
		data.LVL=1;

		ZeroMemory(&desc,sizeof(desc));
		data.Tilex = desc.Width = Tilex;
		data.Tiley = desc.Height = Tiley;	
		desc.MipLevels = 1;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D10_USAGE_DYNAMIC;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		device->CreateTexture2D( &desc, NULL, &idtex );

		D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		data.Tilex=Tilex;
		data.Tiley=Tiley;
		data.ptr_texture=(unsigned char*) malloc(Tilex*Tiley*4);
		memcpy(data.ptr_texture,ptrtex,Tilex*Tiley*4);
		data.MipMap=0;

		D3D10_MAPPED_TEXTURE2D mappedTex;

		idtex->Map( D3D10CalcSubresource(0, 0, LEVELS), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );
		ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			unsigned int rgba;
			for (n1=0;n1<Tilex;n1++)
			{
				rgba=((unsigned int*)ptrtex)[adr0++];
				ptrdest[adr++]=rgba;
			}
		}

		idtex->Unmap( D3D10CalcSubresource(0, 0, LEVELS) );

		free(ptrtex);
	}
	else
	{
		data.LVL=LEVELS;

		data.Tilex=Tilex;
		data.Tiley=Tiley;
		data.ptr_texture=(unsigned char*) malloc(Tilex*Tiley*4);
		memcpy(data.ptr_texture,ptrtex,Tilex*Tiley*4);
		data.MipMap=1;

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		
		static D3D10_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=ptrtex;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=ptrtex;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=0;l<LEVELS;l++) free(ptrs[l]);
	}

	
	data.Format=0;

#endif

#ifdef API3D_DIRECT3D11
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	unsigned int adr0,pitch;
	DWORD * ptrdest;
	unsigned char r,g,b,a;
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	char * Ptr2;
	char * Ptr3;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	data.LVL=LEVELS;

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
	{
		memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
	}
	else
	{
		adr2=0;
		for (n2=0;n2<Tiley;n2++)
		{
			y=(BASE_TILEy*n2)/Tiley;
			adr=y*BASE_TILEx;
			xx=0;
			incxx=(BASE_TILEx*256)/Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				x=(xx>>8);
				r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
				g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
				b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
				a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
				ptrtex[adr2+ 0]=r;
				ptrtex[adr2+ 1]=g;
				ptrtex[adr2+ 2]=b;
				ptrtex[adr2+ 3]=a;
				xx+=incxx;
				adr2+=4;
			}
		}
	}

	D3D11_TEXTURE2D_DESC desc;

	if (MipMap==0)
	{
		data.LVL=1;

		ZeroMemory(&desc,sizeof(desc));
		data.Tilex = desc.Width = Tilex;
		data.Tiley = desc.Height = Tiley;	
		desc.MipLevels = 1;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		device->CreateTexture2D( &desc, NULL, &idtex );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		data.Tilex=Tilex;
		data.Tiley=Tiley;
		data.ptr_texture=(unsigned char*) malloc(Tilex*Tiley*4);
		memcpy(data.ptr_texture,ptrtex,Tilex*Tiley*4);
		data.MipMap=0;

		D3D11_MAPPED_SUBRESOURCE mappedTex;
		devicecontext->Map(idtex, D3D11CalcSubresource(0, 0, LEVELS), D3D11_MAP_WRITE_DISCARD, 0, &mappedTex );

		ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*Tilex;
			unsigned int rgba;
			for (n1=0;n1<Tilex;n1++)
			{
				rgba=((unsigned int*)ptrtex)[adr0++];
				ptrdest[adr++]=rgba;
			}
		}

		devicecontext->Unmap(idtex, D3D11CalcSubresource(0, 0, LEVELS) );

		free(ptrtex);
	}
	else
	{
		data.LVL=LEVELS;

		data.Tilex=Tilex;
		data.Tiley=Tiley;
		data.ptr_texture=(unsigned char*) malloc(Tilex*Tiley*4);
		memcpy(data.ptr_texture,ptrtex,Tilex*Tiley*4);
		data.MipMap=1;

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		static D3D11_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=ptrtex;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=ptrtex;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=0;l<LEVELS;l++) free(ptrs[l]);
	}

	data.Format=0;

#endif

#ifdef API3D_DIRECT3D12
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int LEVELS;
	int Tile;	
	unsigned char r,g,b,a;
	int BASE_TILEx;
	int Tilex;
	int BASE_TILEy;
	int Tiley;
	char * ptrtex;
	int x,y,n1,n2,adr,adr2;
	int xx,incxx;
	char * Ptr2;
	char * Ptr3;

	if (Sizex>Sizey) Tile=Sizey;
	else Tile=Sizex;

	BASE_TILEx=Sizeptrx;
	Tilex=Sizex;
	BASE_TILEy=Sizeptry;
	Tiley=Sizey;

	if (MipMap==0) LEVELS=1;
	else
	{
		LEVELS=1;
		while ((Tile>>LEVELS)>1) LEVELS++;
	}

	data.LVL=LEVELS;

	ptrtex=(char*) malloc(Tilex*Tiley*4);

	if ((BASE_TILEx==Tilex)&&(BASE_TILEy==Tiley))
	{
		memcpy(ptrtex,ptrRGBA,Tilex*Tiley*4);
	}
	else
	{
		adr2=0;
		for (n2=0;n2<Tiley;n2++)
		{
			y=(BASE_TILEy*n2)/Tiley;
			adr=y*BASE_TILEx;
			xx=0;
			incxx=(BASE_TILEx*256)/Tilex;
			for (n1=0;n1<Tilex;n1++)
			{
				x=(xx>>8);
				r=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 0];
				g=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 1];
				b=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 2];
				a=((unsigned char*)ptrRGBA)[((x+adr)<<2) + 3];
				ptrtex[adr2+ 0]=r;
				ptrtex[adr2+ 1]=g;
				ptrtex[adr2+ 2]=b;
				ptrtex[adr2+ 3]=a;
				xx+=incxx;
				adr2+=4;
			}
		}
	}

	((C3DAPIBASE*)dhhc->render)->CloseOpenedCommandList();

	cmdalloc=((C3DAPIBASE*)dhhc->render)->commandAllocators[((C3DAPIBASE*)dhhc->render)->frameIndex];

	ID3D12GraphicsCommandList* cmd;
	dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdalloc, nullptr, IID_PPV_ARGS(&cmd));

	D3D12_RESOURCE_DESC textureDesc = {};

	data.Tilex=Tilex;
	data.Tiley=Tiley;
	data.ptr_texture=(unsigned char*) malloc(Tilex*Tiley*4);
	memcpy(data.ptr_texture,ptrtex,Tilex*Tiley*4);

	D3D12_RESOURCE_BARRIER barrierDesc = {};

	if (MipMap==0)
	{
		data.LVL=1;
		data.MipMap=0;        

        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = data.Tilex = Tilex;
		textureDesc.Height = data.Tiley = Tiley;	
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        dhhc->d3ddevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),D3D12_HEAP_FLAG_NONE,&textureDesc,
										D3D12_RESOURCE_STATE_COMMON,nullptr,IID_PPV_ARGS(&texture));
	
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture, 0, 1);

        // Create the GPU upload buffer.
        dhhc->d3ddevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),D3D12_HEAP_FLAG_NONE,&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
										D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&textureUpload));



		D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = (void*)ptrtex;
        textureData.RowPitch = Tilex * 4;
        textureData.SlicePitch = textureData.RowPitch * Tiley;

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = texture;
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmd->ResourceBarrier(1, &barrierDesc);

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, 1, &textureData);
	}
	else
	{
		data.LVL=LEVELS;
		data.MipMap=1;

        textureDesc.MipLevels = LEVELS;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = data.Tilex = Tilex;
		textureDesc.Height = data.Tiley = Tiley;	
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        dhhc->d3ddevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),D3D12_HEAP_FLAG_NONE,&textureDesc,
										D3D12_RESOURCE_STATE_COMMON,nullptr,IID_PPV_ARGS(&texture));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture, 0, LEVELS);
		
        // Create the GPU upload buffer.
        dhhc->d3ddevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),D3D12_HEAP_FLAG_NONE,&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
										D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&textureUpload));

		D3D12_SUBRESOURCE_DATA textureData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				textureData[l].pData=ptrtex;
				textureData[l].RowPitch=Tilex*4;
				textureData[l].SlicePitch=Tilex*Tiley*4;
				Ptr3=ptrtex;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				textureData[l].pData=Ptr2;
				textureData[l].RowPitch=wx*4;
				textureData[l].SlicePitch=wx*wy*4;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)textureData[l].pData;
		}
		
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = texture;
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		cmd->ResourceBarrier(1, &barrierDesc);

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, LEVELS, textureData);
		for (int l=0;l<LEVELS;l++) free(ptrs[l]);
	}

	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	cmd->ResourceBarrier(1, &barrierDesc); 

	cmd->DiscardResource(textureUpload,nullptr);

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
	cmd->Close();

	ID3D12CommandList* list[] = { cmd };
	queue->ExecuteCommandLists(_countof(list), list);
	cmd->Release();

#ifdef CLASSNED
	if (data.ptr_texture) free(data.ptr_texture);
	data.ptr_texture=NULL;
#endif

	/*
	ID3D12Fence* fnce;
	dhhc->d3ddevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fnce));
	//auto nextFence=fnce->GetCompletedValue()+1;
	queue->Signal(fnce,1);

	queue->Wait(fnce,1);

	//textureUpload->Release();
	
	if (fnce->GetCompletedValue() < 1)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);  
		fnce->SetEventOnCompletion(1, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	/**/

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	if (MipMap==0) srvDesc.Texture2D.MipLevels = 1;
	else srvDesc.Texture2D.MipLevels = LEVELS;

	id=dhhc->GetNewHeapHandleNotRecycle();
	
	dhhc->d3ddevice->CreateShaderResourceView(texture, &srvDesc, id.GetCPUHandle());

	//((C3DAPIBASE*)dhhc->render)->TextureToShaders.Add(this);

	data.Format=0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::createBump(char * ptr,int sizex,int sizey,float scale)
{
	char * raw;
	int n1,n2;
	float dx,dy,i,ix,iy;
	unsigned char vr,vg,vb;
	CVector uub;
	unsigned int adr=0;

	raw=(char *) malloc(sizex*sizey*4);

	for (n2=0;n2<sizey;n2++)
		for (n1=0;n1<sizex;n1++)	
		{
			i=(float)(ptr[n1+ n2*sizex]&255);
			ix=(float)(ptr[(n1+1)%sizex+ n2*sizex]&255);
			iy=(float)(ptr[n1+ ((n2+1)%sizey)*sizex]&255);

			dx=(ix-i)/255.0f;
			dy=(iy-i)/255.0f;

			uub.x=-scale*dx;
			uub.y=-scale*dy;
			uub.z=-1;

			VECTORNORMALISE(uub);

			vr=(int)(128+uub.x*127);
			vg=(int)(128+uub.y*127);
			vb=(int)(128+uub.z*127);

			raw[adr+0]=vr;
			raw[adr+1]=vg;
			raw[adr+2]=vb;
			raw[adr+3]=(char)255;
			adr+=4;
		}

	create(sizex,sizey,raw,sizex,sizey,NO_ALPHA,0,0,0);
	
	free(raw);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::modifyRGB(int x,int y,int sizex,int sizey,char * ptr)
{

#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -------------
	char * modif=(char*) malloc(sizex*sizey*4);
	unsigned int adr=0;
	unsigned int adrto=0;
	for (int n=0;n<sizex*sizey;n++)
	{
		modif[adrto+0]=ptr[adr+0];
		modif[adrto+1]=ptr[adr+1];
		modif[adrto+2]=ptr[adr+2];
		modif[adrto+3]=255;
		adrto+=4;
		adr+=3;
	}

	MTLUpdateTexture(id,x,y,sizex,sizey,modif);
	free(modif);
#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,id);
	glTexSubImage2D(GL_TEXTURE_2D,0,x,y,sizex,sizey,GL_RGB,GL_UNSIGNED_BYTE,ptr);

#endif

#ifdef API3D_DIRECT3D12
	int n1,n2;
	unsigned int adr,adr0;
	unsigned int adry,adr0y;
	int r,g,b;
	char * Ptr2;
	char * Ptr3;
	
	if (data.ptr_texture==NULL) return;

	int add3sizex=3*sizex;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{
			r=((unsigned char*)ptr)[adr0+0];
			g=((unsigned char*)ptr)[adr0+1];
			b=((unsigned char*)ptr)[adr0+2];
			((unsigned int*)data.ptr_texture)[adr]=(255<<24)+(b<<16)+(g<<8)+r;
			adr0+=3;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add3sizex;
	}

	((C3DAPIBASE*)dhhc->render)->CloseOpenedCommandList();

	cmdalloc=((C3DAPIBASE*)dhhc->render)->commandAllocators[((C3DAPIBASE*)dhhc->render)->frameIndex];

	ID3D12GraphicsCommandList* cmd;
	dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdalloc, nullptr, IID_PPV_ARGS(&cmd));

	if (data.MipMap==0)
	{
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = (void*)data.ptr_texture;
        textureData.RowPitch = data.Tilex * 4;
        textureData.SlicePitch = textureData.RowPitch * data.Tiley;

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, 1, &textureData);
	}
	else
	{
		D3D12_SUBRESOURCE_DATA textureData[128];
		static char * ptrs[128];

		int wx=data.Tilex;
		int wy=data.Tiley;

		for (int l=0;l<data.LVL;l++)
		{
			if (l==0)
			{
				textureData[l].pData=data.ptr_texture;
				textureData[l].RowPitch=data.Tilex*4;
				textureData[l].SlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				textureData[l].pData=Ptr2;
				textureData[l].RowPitch=wx*4;
				textureData[l].SlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)textureData[l].pData;
		}

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, data.LVL, textureData);
        
		for (int l=1;l<data.LVL;l++) free(ptrs[l]);
	}

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	cmd->Close();
	ID3D12CommandList* list[] = { cmd };
    queue->ExecuteCommandLists(_countof(list), list);
	cmd->Release();

#else
#ifdef API3D_DIRECT3D11
	int n1,n2;
	unsigned int adr,adr0,pitch;
	unsigned int adry,adr0y;
	unsigned int rgb;
	int r,g,b;
	
	if (data.ptr_texture==NULL) return;

	int add3sizex=3*sizex;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{
			r=((unsigned char*)ptr)[adr0+0];
			g=((unsigned char*)ptr)[adr0+1];
			b=((unsigned char*)ptr)[adr0+2];
			((unsigned int*)data.ptr_texture)[adr]=(255<<24)+(b<<16)+(g<<8)+r;
			adr0+=3;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add3sizex;
	}

	if (data.MipMap>0)
	{
		int LEVELS=data.LVL;
		int Tilex=data.Tilex;
		int Tiley=data.Tiley;
		char *Ptr2,*Ptr3;
		D3D11_TEXTURE2D_DESC desc;

		idtex->Release();
		id->Release();

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		static D3D11_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=data.ptr_texture;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=1;l<LEVELS;l++) free(ptrs[l]);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedTex;
		devicecontext->Map(idtex, D3D11CalcSubresource(0, 0, data.LVL), D3D11_MAP_WRITE_DISCARD, 0, &mappedTex );

		DWORD *ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<data.Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*data.Tilex;
			for (n1=0;n1<data.Tilex;n1++)
			{
				rgb=((unsigned int*)data.ptr_texture)[adr0++];
				ptrdest[adr++]=rgb;
			}
		}

		devicecontext->Unmap( idtex,D3D11CalcSubresource(0, 0, data.LVL) );
	}
#else
#ifdef API3D_DIRECT3D10
	int n1,n2;
	unsigned int adr,adr0,pitch;
	unsigned int adry,adr0y;
	unsigned int rgb;
	int r,g,b;
	
	if (data.ptr_texture==NULL) return;

	int add3sizex=3*sizex;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{			
			r=((unsigned char*)ptr)[adr0+0];
			g=((unsigned char*)ptr)[adr0+1];
			b=((unsigned char*)ptr)[adr0+2];
			((unsigned int*)data.ptr_texture)[adr]=(255<<24)+(b<<16)+(g<<8)+r;
			//rgb=*((unsigned int*)(&ptr[adr0]))&0x00FFFFFF;
			//((unsigned int*)data.ptr_texture)[adr]=(255<<24)+rgb;
			adr0+=3;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add3sizex;
	}

	if (data.MipMap>0)
	{
		int LEVELS=data.LVL;
		int Tilex=data.Tilex;
		int Tiley=data.Tiley;
		char *Ptr2,*Ptr3;
		D3D10_TEXTURE2D_DESC desc;

		idtex->Release();
		id->Release();

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		
		static D3D10_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=data.ptr_texture;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=1;l<LEVELS;l++) free(ptrs[l]);
	}
	else
	{
		D3D10_MAPPED_TEXTURE2D mappedTex;
		idtex->Map( D3D10CalcSubresource(0, 0, data.LVL), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );
		DWORD *ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<data.Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*data.Tilex;
			for (n1=0;n1<data.Tilex;n1++)
			{
				rgb=((unsigned int*)data.ptr_texture)[adr0++];
				ptrdest[adr++]=rgb;
			}
		}

		idtex->Unmap( D3D10CalcSubresource(0, 0, data.LVL) );
	}
#else
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int r,g,b,a;
	unsigned int cc;
	int n1,n2;
	unsigned int adr,adr0,pitch;
	unsigned int adry,adr0y;
	WORD * ptrdest;
	DWORD * ptrdest2;
	D3DLOCKED_RECT Locked;
	RECT DirtyRect;
	CQuad q;
	int add3sizex=3*sizex;

	DirtyRect.top=y;
	DirtyRect.left=x;
	DirtyRect.right=x+sizex;
	DirtyRect.bottom=y+sizey;

	q.x1=x;
	q.y1=y;
	q.x2=x+sizex;
	q.y2=y+sizey;

	data.LQ.Add(q);

	if (data.Format<0) // 8888
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0

		ptrdest2=(DWORD *) Locked.pBits;

		pitch=Locked.Pitch/4;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];

				cc=(r<<16) + (g<<8) +b;

				ptrdest2[adr]=cc;
				adr0+=3;
				adr++;
			}
			adry+=pitch;
			adr0y+=add3sizex;
		}
		id->UnlockRect(0);
	}


	if (data.Format==0) // 565
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0

		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				ptrdest[adr]=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
				adr0+=3;
				adr++;
			}
			adry+=pitch;
			adr0y+=add3sizex;
		}
		id->UnlockRect(0);
	}


	if (data.Format==1) // 1555
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0
		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				ptrdest[adr]=((r>>3)<<10) +((g>>3)<<5) + (b>>3);
				adr0+=3;
				adr++;
			}
			adry+=pitch;
			adr0y+=add3sizex;
		}
		id->UnlockRect(0);
	}

	if (data.Format==2) // 4444
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0
		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				a=255;
				ptrdest[adr]=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
				adr0+=3;
				adr++;
			}
			adry+=pitch;
			adr0y+=add3sizex;
		}
		id->UnlockRect(0);

	}
#endif  
#endif
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::modifyRGBA(int x,int y,int sizex,int sizey,char * ptr)
{

#if defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ METAL -------------
	MTLUpdateTexture(id,x,y,sizex,sizey,ptr);
#endif


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,id);
	glTexSubImage2D(GL_TEXTURE_2D,0,x,y,sizex,sizey,GL_RGBA,GL_UNSIGNED_BYTE,ptr);

#endif

#ifdef API3D_DIRECT3D12

	unsigned int rgba;
	int n1,n2;
	unsigned int adr,adr0;
	unsigned int adry,adr0y;
	int add4sizex=4*sizex;
	char * Ptr2;
	char * Ptr3;

	if (data.ptr_texture==NULL) return;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{
			rgba=*((unsigned int*)(&ptr[adr0]));
			((unsigned int*)data.ptr_texture)[adr]=rgba;
			adr0+=4;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add4sizex;
	}

	((C3DAPIBASE*)dhhc->render)->CloseOpenedCommandList();

	cmdalloc=((C3DAPIBASE*)dhhc->render)->commandAllocators[((C3DAPIBASE*)dhhc->render)->frameIndex];

	ID3D12GraphicsCommandList* cmd;
	dhhc->d3ddevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdalloc, nullptr, IID_PPV_ARGS(&cmd));
	
	if (data.MipMap==0)
	{
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = (void*)data.ptr_texture;
        textureData.RowPitch = data.Tilex * 4;
        textureData.SlicePitch = textureData.RowPitch * data.Tiley;

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, 1, &textureData);
	}
	else
	{
		D3D12_SUBRESOURCE_DATA textureData[128];
		static char * ptrs[128];

		int wx=data.Tilex;
		int wy=data.Tiley;

		for (int l=0;l<data.LVL;l++)
		{
			if (l==0)
			{
				textureData[l].pData=data.ptr_texture;
				textureData[l].RowPitch=data.Tilex*4;
				textureData[l].SlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				textureData[l].pData=Ptr2;
				textureData[l].RowPitch=wx*4;
				textureData[l].SlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)textureData[l].pData;
		}

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

        UpdateSubresources(cmd, texture, textureUpload, 0, 0, data.LVL, textureData);        

		for (int l=1;l<data.LVL;l++) free(ptrs[l]);
	}

	cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	cmd->Close();
	ID3D12CommandList* list[] = { cmd };
    queue->ExecuteCommandLists(_countof(list), list);
	cmd->Release();

#else
#ifdef API3D_DIRECT3D11
	unsigned int rgba;
	int n1,n2;
	unsigned int adr,adr0;
	unsigned int adry,adr0y,pitch;
	int add4sizex=4*sizex;

	if (data.ptr_texture==NULL) return;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{
			rgba=*((unsigned int*)(&ptr[adr0]));
			((unsigned int*)data.ptr_texture)[adr]=rgba;
			adr0+=4;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add4sizex;
	}

	if (data.MipMap>0)
	{
		int LEVELS=data.LVL;
		int Tilex=data.Tilex;
		int Tiley=data.Tiley;
		char *Ptr2,*Ptr3;
		D3D11_TEXTURE2D_DESC desc;

		idtex->Release();
		id->Release();

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		static D3D11_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=data.ptr_texture;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=1;l<LEVELS;l++) free(ptrs[l]);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedTex;
		devicecontext->Map(idtex, D3D11CalcSubresource(0, 0, data.LVL), D3D11_MAP_WRITE_DISCARD, 0, &mappedTex );

		DWORD *ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<data.Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*data.Tilex;
			for (n1=0;n1<data.Tilex;n1++)
			{
				rgba=((unsigned int*)data.ptr_texture)[n1+adr0];
				ptrdest[n1+adr]=rgba;
			}
		}

		devicecontext->Unmap( idtex, D3D11CalcSubresource(0, 0, data.LVL) );
	}
#else
#ifdef API3D_DIRECT3D10
	unsigned int rgba;
	int n1,n2;
	unsigned int adr,adr0;
	unsigned int adry,adr0y,pitch;
	int add4sizex=4*sizex;

	if (data.ptr_texture==NULL) return;

	adry=x+y*data.Tilex;
	adr0y=0;
	for (n2=0;n2<sizey;n2++)
	{
		adr0=adr0y;
		adr=adry;
		for (n1=0;n1<sizex;n1++)
		{
			rgba=*((unsigned int*)(&ptr[adr0]));
			((unsigned int*)data.ptr_texture)[adr]=rgba;
			adr0+=4;
			adr++;
		}
		adry+=data.Tilex;
		adr0y+=add4sizex;
	}

	if (data.MipMap>0)
	{
		int LEVELS=data.LVL;
		int Tilex=data.Tilex;
		int Tiley=data.Tiley;
		char *Ptr2,*Ptr3;
		D3D10_TEXTURE2D_DESC desc;

		idtex->Release();
		id->Release();

		ZeroMemory(&desc,sizeof(desc));
		desc.Width = Tilex;
		desc.Height = Tiley;	
		desc.MipLevels = LEVELS;
		desc.ArraySize = 1;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		
		static D3D10_SUBRESOURCE_DATA InitData[128];
		static char * ptrs[128];

		int wx=Tilex;
		int wy=Tiley;

		for (int l=0;l<LEVELS;l++)
		{
			if (l==0)
			{
				InitData[l].pSysMem=data.ptr_texture;
				InitData[l].SysMemPitch=Tilex*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=(char*)data.ptr_texture;
			}
			else
			{
				Ptr2=CreateMipMap(Ptr3,wx,wy,4);
				wx=wx>>1;
				wy=wy>>1;
				InitData[l].pSysMem=Ptr2;
				InitData[l].SysMemPitch=wx*4;
				InitData[l].SysMemSlicePitch=0;
				Ptr3=Ptr2;
			}

			ptrs[l]=(char*)InitData[l].pSysMem;
		}

		device->CreateTexture2D( &desc, InitData, &idtex );

		D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

		idtex->GetDesc( &desc );
		
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ArraySize = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;

		device->CreateShaderResourceView( idtex, &srvDesc, &id );

		for (int l=1;l<LEVELS;l++) free(ptrs[l]);
	}
	else
	{
		D3D10_MAPPED_TEXTURE2D mappedTex;
		idtex->Map( D3D10CalcSubresource(0, 0, data.LVL), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );
		DWORD *ptrdest = (DWORD*)mappedTex.pData;
		pitch = mappedTex.RowPitch/4;
		
		for (n2=0;n2<data.Tiley;n2++)
		{
			adr=n2*pitch;
			adr0=n2*data.Tilex;
			for (n1=0;n1<data.Tilex;n1++)
			{
				rgba=((unsigned int*)data.ptr_texture)[n1+adr0];
				ptrdest[n1+adr]=rgba;
			}
		}

		idtex->Unmap( D3D10CalcSubresource(0, 0, data.LVL) );
	}
#else
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int r,g,b,a;
	int n1,n2;
	unsigned int adr,adr0;
	unsigned int adry,adr0y,pitch,cc;
	WORD * ptrdest;
	DWORD * ptrdest2;
	D3DLOCKED_RECT Locked;
	RECT DirtyRect;
	CQuad q;
	int add4sizex=4*sizex;

	DirtyRect.top=y;
	DirtyRect.left=x;
	DirtyRect.right=x+sizex;
	DirtyRect.bottom=y+sizey;

	q.x1=x;
	q.y1=y;
	q.x2=x+sizex;
	q.y2=y+sizey;

	data.LQ.Add(q);

	if (data.Format<0) // 8888
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0

		ptrdest2=(DWORD *) Locked.pBits;

		pitch=Locked.Pitch/4;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				a=((unsigned char*)ptr)[adr0+3];

				cc=(a<<24) + (r<<16) + (g<<8) +b;

				ptrdest2[adr]=cc;
				adr0+=4;
				adr++;
			}
			adry+=pitch;
			adr0y+=sizex<<2;
		}
		id->UnlockRect(0);
	}

	if (data.Format==0) // 565
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0

		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				ptrdest[adr]=((r>>3)<<11) +((g>>2)<<5) + (b>>3);
				adr0+=4;
				adr++;
			}
			adry+=pitch;
			adr0y+=add4sizex;
		}
		id->UnlockRect(0);
	}


	if (data.Format==1) // 1555
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0
		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				ptrdest[adr]=((r>>3)<<10) +((g>>3)<<5) + (b>>3);
				adr0+=4;
				adr++;
			}
			adry+=pitch;
			adr0y+=add4sizex;
		}
		id->UnlockRect(0);
	}

	if (data.Format==2) // 4444
	{
		id->LockRect(0,&Locked,&DirtyRect,D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK);	// level 0
		ptrdest=(WORD *) Locked.pBits;

		pitch=Locked.Pitch>>1;

		adry=0;
		adr0y=0;
		for (n2=0;n2<sizey;n2++)
		{
			adr0=adr0y;
			adr=adry;
			for (n1=0;n1<sizex;n1++)
			{
				r=((unsigned char*)ptr)[adr0+0];
				g=((unsigned char*)ptr)[adr0+1];
				b=((unsigned char*)ptr)[adr0+2];
				a=((unsigned char*)ptr)[adr0+3];
				ptrdest[adr]=((a>>4)<<12) + ((r>>4)<<8) +((g>>4)<<4) + (b>>4);
				adr0+=4;
				adr++;
			}
			adry+=pitch;
			adr0y+=add4sizex;
		}
		id->UnlockRect(0);

	}
#endif  
#endif
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::release()
{
	if (dup)
	{
#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
		id=0;
#endif

#ifdef API3D_DIRECT3D
		id=NULL;
#endif

#ifdef API3D_METAL
		id=-1;
#endif

#ifdef API3D_DIRECT3D9
		id=NULL;
#endif

#ifdef API3D_DIRECT3D10
		id=NULL;
		idtex=NULL;
		data.ptr_texture=NULL;
#endif

#ifdef API3D_DIRECT3D11
		id=NULL;
		idtex=NULL;
		data.ptr_texture=NULL;
#endif

#ifdef API3D_DIRECT3D12
		data.ptr_texture=NULL;
#endif
		data.Material=NULL;

		dup=false;
		return;
	}


#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)
//------------------------------------------------------------------------------------------------ OPEN GL -----------	
	glDeleteTextures(1,(GLuint*) &id);
	id=0;
#endif

#ifdef API3D_METAL
//------------------------------------------------------------------------------------------------ METAL -------------	
	if (id>=0) MTLFreeTexture(id);
	id=-1;
#endif

#ifdef API3D_DIRECT3D
//------------------------------------------------------------------------------------------------ DIRECT3D ----------	
	if (id) id->Release();
	id=NULL;
#endif

#ifdef API3D_DIRECT3D9
//------------------------------------------------------------------------------------------------ DIRECT3D9 ---------	
	if (id) id->Release();
	id=NULL;
#endif

#ifdef API3D_DIRECT3D10
	if (id) id->Release();
	id=NULL;
	if (idtex) idtex->Release();
	idtex=NULL;
	if (data.ptr_texture) free(data.ptr_texture);
	data.ptr_texture=NULL;

#endif

#ifdef API3D_DIRECT3D11
	if (id) id->Release();
	id=NULL;
	if (idtex) idtex->Release();
	idtex=NULL;
	if (data.ptr_texture) free(data.ptr_texture);
	data.ptr_texture=NULL;
#endif

#ifdef API3D_DIRECT3D12
	if (data.ptr_texture) free(data.ptr_texture);
	data.ptr_texture=NULL;
	if (texture)
	{
		int not=0;
		ID3D12Resource **res=((C3DAPIBASE*)dhhc->render)->TexturesFreeing.GetFirst();
		while (res)
		{
			if ((*res)==texture) not=1;
			res=((C3DAPIBASE*)dhhc->render)->TexturesFreeing.GetNext();
		}

		if (not==0) ((C3DAPIBASE*)dhhc->render)->TexturesFreeing.Add(texture);
	}

	if (textureUpload)
	{
		int not=0;
		ID3D12Resource **res=((C3DAPIBASE*)dhhc->render)->TexturesFreeing.GetFirst();
		while (res)
		{
			if ((*res)==textureUpload) not=1;
			res=((C3DAPIBASE*)dhhc->render)->TexturesFreeing.GetNext();
		}

		if (not==0) ((C3DAPIBASE*)dhhc->render)->TexturesFreeing.Add(textureUpload);
	}

	texture=NULL;
	textureUpload=NULL;
	dhhc->FreeHeapHandle(id);
#endif

	if (data.Material) delete data.Material;
	data.Material=NULL;
}

#ifdef API3D_DIRECT3D

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::setAPI(LPDIRECT3DDEVICE8 api)
{
	D3DDevice=api;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
LPDIRECT3DTEXTURE8 CTextureAPI::getApiID()
{
	return id;
}

#endif

#ifdef API3D_DIRECT3D9

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::setAPI(LPDIRECT3DDEVICE9 api)
{
	D3DDevice=api;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
LPDIRECT3DTEXTURE9 CTextureAPI::getApiID()
{
	return id;
}

#endif


#ifdef API3D_DIRECT3D10

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::setAPI(ID3D10Device* api)
{
	device=api;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ID3D10ShaderResourceView* CTextureAPI::getApiID()
{
	return id;
}

#endif

#ifdef API3D_DIRECT3D11

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTextureAPI::setAPI(ID3D11Device* api,ID3D11DeviceContext* cntxt)
{
	device=api;
	devicecontext=cntxt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ID3D11ShaderResourceView* CTextureAPI::getApiID()
{
	return id;
}

#endif

#ifdef API3D_DIRECT3D12

void CTextureAPI::setAPI(DescriptorHeapHandleContainer* _dhhc,ID3D12CommandAllocator* _cmdalloc,ID3D12CommandQueue* _queue)
{
	dhhc=_dhhc;
	cmdalloc=_cmdalloc;
	queue=_queue;
}

DescriptorHeapHandle* CTextureAPI::getApiID()
{
	return &id;
}

#endif

#if defined(API3D_OPENGL) || defined(API3D_OPENGL20)

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int	CTextureAPI::getApiID()
{
	return id;
}

#endif

#if defined(API3D_METAL)

///////////////////////////////////////////////////////////////////////////////////////////////////
int	CTextureAPI::getApiID()
{
	return id;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
CTexture * CTextureAPI::getTextureInfo()
{
	return &data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char* CTextureAPI::get()
{
	unsigned char *ptr;
	int sizex,sizey;

	sizex=data.Tilex;
	sizey=data.Tiley;
	ptr=(unsigned char*) malloc(sizex*sizey*4);
	memset(ptr,0,sizex*sizey*4);

#ifdef API3D_METAL
#else
#ifdef API3D_DIRECT3D12
#else
#ifdef API3D_DIRECT3D11
#else
#ifdef API3D_DIRECT3D10
#else
#if !defined(API3D_OPENGL) && !defined(API3D_OPENGL20) && !defined(API3D_METAL)
//------------------------------------------------------------------------------------------------ DIRECT3D ----------
	int r,g,b,a;
	int n1,n2;
	unsigned int adr,adr0,pitch;
	WORD * ptrdest;
	WORD rgba;
	DWORD * d_ptrdest;
	DWORD d_rgba;
	D3DLOCKED_RECT Locked;

	if (data.Format<0) // 888
	{
		id->LockRect(0,&Locked,NULL,0);	// level 0
		d_ptrdest=(DWORD *) Locked.pBits;
		pitch=Locked.Pitch/4;

		for (n2=0;n2<sizey;n2++)
		{
			adr0=4*(n2*sizex);
			adr=(n2)*pitch;
			for (n1=0;n1<sizex;n1++)
			{
				d_rgba=d_ptrdest[adr];

				r=(d_rgba>>16)&0xFF;
				g=(d_rgba>>8)&0xFF;
				b=(d_rgba)&0xFF;

				ptr[adr0+0]=r;
				ptr[adr0+1]=g;
				ptr[adr0+2]=b;
				ptr[adr0+3]=255;
				
				adr0+=4;
				adr++;
			}
		}
		id->UnlockRect(0);
	}


	if (data.Format==0) // 565
	{
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(WORD *) Locked.pBits;
		pitch=Locked.Pitch/2;

		for (n2=0;n2<sizey;n2++)
		{
			adr0=4*(n2*sizex);
			adr=(n2)*pitch;
			for (n1=0;n1<sizex;n1++)
			{
				rgba=ptrdest[adr];

				r=(rgba>>11)&31;
				g=(rgba>>5)&63;
				b=(rgba)&31;

				ptr[adr0+0]=r*8;
				ptr[adr0+1]=g*4;
				ptr[adr0+2]=b*8;
				ptr[adr0+3]=255;

				
				adr0+=4;
				adr++;
			}
		}
		id->UnlockRect(0);
	}


	if (data.Format==1) // 1555
	{
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(WORD *) Locked.pBits;
		pitch=Locked.Pitch/2;

		for (n2=0;n2<sizey;n2++)
		{
			adr0=4*(n2*sizex);
			adr=(n2)*pitch;
			for (n1=0;n1<sizex;n1++)
			{
				rgba=ptrdest[adr];

				r=(rgba>>10)&31;
				g=(rgba>>5)&31;
				b=(rgba)&31;

				ptr[adr0+0]=r*8;
				ptr[adr0+1]=g*8;
				ptr[adr0+2]=b*8;
				ptr[adr0+3]=255;

				adr0+=4;
				adr++;
			}
		}
		id->UnlockRect(0);
	}

	if (data.Format==2) // 4444
	{
		id->LockRect(0,&Locked,NULL,0);	// level 0
		ptrdest=(WORD *) Locked.pBits;
		pitch=Locked.Pitch/2;

		for (n2=0;n2<sizey;n2++)
		{
			adr0=4*(n2*sizex);
			adr=(n2)*pitch;
			for (n1=0;n1<sizex;n1++)
			{
				rgba=ptrdest[adr];

				a=(rgba>>12)&15;
				r=(rgba>>8)&15;
				g=(rgba>>4)&15;
				b=(rgba)&15;
				
				ptr[adr0+0]=r*16;
				ptr[adr0+1]=g*16;
				ptr[adr0+2]=b*16;
				ptr[adr0+3]=a*16;

				adr0+=4;
				adr++;
			}
		}
		id->UnlockRect(0);
	}
#endif  
#endif
#endif
#endif
#endif
	return ptr;
}
