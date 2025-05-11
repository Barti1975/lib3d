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
//	@file png.cpp
//	@date 2014-07-23
////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
	#pragma warning (disable:4244)
	#pragma warning (disable:4018)
#endif

#ifndef NOLIB3D
#include "../base/params.h"
#endif

#if !defined(WIN32)&&!defined(ANDROID)
#include <stdlib.h>
#endif
#include <memory.h>


#if defined(WIN32)||defined(LINUX)
#ifdef API3D_VR
#include "../../zlib/zlib.h"
#else
#ifdef API3D_SDL2
#include "../../zlib/zlib.h"
#else
#include "../zlib/zlib.h"
#endif
#endif
#else
#ifdef WINDOWS_STORE
#include "../../zlib/zlib.h"
#else
#ifdef API3D_SDL2
#include "../../zlib/zlib.h"
#else
#include "zlib.h"
#endif
#endif
#endif

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <memory.h>

#define KOEF_COMPRESSION 1.2f

#define LZ77HUFFMAN Z_HUFFMAN_ONLY  // Z_NO_COMPRESSION //Z_HUFFMAN_ONLY  //Z_BEST_SPEED

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(WINDOWS_STORE)
#define __FILE int
int __fopen(char *fname,char * prm);
void __fwrite(void *data,int len,int sz,int file);
int __fread(void *data,int len,int sz,int file);
void __fclose(int file);
int __fsize(int file);
#else
#define __FILE FILE *
#define __fopen fopen
#define __fwrite fwrite
#define __fread fread
#define __fclose fclose
#define __fscanf fscanf
#define __fprintf fprintf
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// toggle to disable crc checking
////////////////////////////////////////////////////////////////////////////////////////////////////////////
float PNG_valueofbeefsteack=0.2f;

#define USE_CRC_CHECK

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "lzcompress.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	FILTER_NONE			=	0,
	FILTER_SUB			=	1,
	FILTER_UP			=	2,
	FILTER_AVERAGE		=	3
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int B00(char Buf[4])
{
	unsigned int n;
	n=((unsigned int)(Buf[3]&255))+ ((unsigned int)(Buf[2]&255))*256 + ((unsigned int)(Buf[1]&255))*65536 + ((unsigned int)(Buf[0]&255))*65536*256;
	return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ib(unsigned int val)
{
	unsigned int n;
	char Buf[4];
	memcpy(Buf,(char*)&val,4);
	n=(unsigned int)(Buf[3]&255)+ ((unsigned int)(Buf[2]&255))*256 +((unsigned int) (Buf[1]&255))*65536 + ((unsigned int)(Buf[0]&255))*65536*256;
	return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------- CRC

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned long c;
	int n, k;
	if (crc_table_computed==0)
	for (n = 0; n < 256; n++)
	{
		c = (unsigned long) n;
		for (k = 0; k < 8; k++)
		{
			if (c & 1)
			c = 0xedb88320L ^ (c >> 1);
			else
			c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
  should be initialized to all 1's, and the transmitted value
  is the 1's complement of the final running CRC (see the
  crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf,int len)
{
	unsigned long c = crc;
	int n;

	if (!crc_table_computed) make_crc_table();
	for (n = 0; n < len; n++)
	{
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(char * ctype,unsigned char *buf, int len)
{
	unsigned long res;
	unsigned char * ptr;

	ptr=(unsigned char*) malloc(4+len);
	memcpy(ptr,ctype,4);
	if (len>0) memcpy(&ptr[4],buf,len);
	res=update_crc(0xffffffffL, ptr, len+4) ^ 0xffffffffL;

	free(ptr);

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct IMAGE_DATAS
{
	char * ptrImageDATAS;	// ptr
	unsigned int X,Y;		// dimension
	int	nTexture;			// gestion dynamique...
	int Alpha;				// 0-1
	int Grayscale;
};

struct HEADER
{
	char LEN_X[4];
	char LEN_Y[4];
	char Bit_Depth;
	char ColorType;
	char C_Method;
	char Filter;
	char Interlace;
	unsigned int X,Y;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int pi_abs(int x)
{
	if (x<0) return -x;
	else return x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int PAETH(unsigned int a,unsigned int b,unsigned int c)
{
	unsigned int p,pa,pb,pc;
	p=a+b-c;
	pa=pi_abs((int)p-(int)a);
	pb=pi_abs((int)p-(int)b);
	pc=pi_abs((int)p-(int)c);
	if ((pa<=pb)&&(pa<=pc)) return a;
	else if (pb<=pc) return b;
	else return c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILTRES - RGB
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSUB(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;

	adr=n*(HDR->X*3);

	ImageDATAF[adr+0]=((unsigned char*)ImageDATA)[adr+0];
	ImageDATAF[adr+1]=((unsigned char*)ImageDATA)[adr+1];
	ImageDATAF[adr+2]=((unsigned char*)ImageDATA)[adr+2];

	for (n1=3;n1<HDR->X*3;n1++)
	{
		ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr+n1-3])&0xFF;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterNONE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;
	adr=n*(HDR->X*3);
	for (n1=0;n1<HDR->X*3;n1++)
		ImageDATAF[adr+n1]=ImageDATA[adr+n1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterUP(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0;

	if (n>0)
	{
		adr=n*(HDR->X*3);
		adr0=(n-1)*(HDR->X*3);

		for (n1=0;n1<HDR->X*3;n1++)
		{
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr0+n1])&0xFF;
		}
	}
	else
	{
		for (n1=0;n1<HDR->X*3;n1++)
		{
			ImageDATAF[n1]=((unsigned char*)ImageDATA)[n1];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterAVERAGE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,prior,raw;

	if (n>0)
	{
		adr=n*(HDR->X*3);
		adr0=(n-1)*(HDR->X*3);

		for (n1=0;n1<3;n1++)
		{
			prior=((unsigned char*)ImageDATAF)[adr0+n1];
            raw=0;
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;
		}

        for (n1=3;n1<HDR->X*3;n1++)
        {
            prior=((unsigned char*)ImageDATAF)[adr0+n1];
            raw=((unsigned char*)ImageDATAF)[adr+n1-3];
            ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;
        }
    }
	else
	{
		adr=n*(HDR->X*3);
		for (n1=0;n1<3;n1++)
		{
            raw=0;
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255))&0xFF;
		}

        for (n1=3;n1<HDR->X*3;n1++)
        {
            raw=((unsigned char*)ImageDATAF)[adr+n1-3];
            ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255))&0xFF;
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FilterPAETH(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,left,above,upperleft;

	adr=n*HDR->X*3;
	adr0=(n-1)*HDR->X*3;

	for (n1=0;n1<3;n1++)
	{
        left=0;upperleft=0;
        if (n>0) above=((unsigned char*)ImageDATAF)[adr0+n1];
        else above=0;
	
		ImageDATAF[adr+n1]=(ImageDATA[adr+n1]+PAETH(left,above,upperleft))&0xFF;
	}

    for (n1=3;n1<HDR->X*3;n1++)
    {
        if (n>0)
        {
            left=((unsigned char*)ImageDATAF)[adr-3+n1];
            upperleft=((unsigned char*)ImageDATAF)[adr0-3+n1];
            above=((unsigned char*)ImageDATAF)[adr0+n1];
        }
        else
        {
            upperleft=0;
            above=0;
            left=((unsigned char*)ImageDATAF)[adr-3+n1];
        }
        ImageDATAF[adr+n1]=(ImageDATA[adr+n1]+PAETH(left,above,upperleft))&0xFF;
    }


}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILTRES - RGBA
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFilterSUB(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;

	adr=n*(HDR->X*4);

	ImageDATAF[adr+0]=((unsigned char*)ImageDATA)[adr+0];
	ImageDATAF[adr+1]=((unsigned char*)ImageDATA)[adr+1];
	ImageDATAF[adr+2]=((unsigned char*)ImageDATA)[adr+2];
	ImageDATAF[adr+3]=((unsigned char*)ImageDATA)[adr+3];

	for (n1=4;n1<HDR->X*4;n1++)
	{
		ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr+n1-4])&0xFF;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFilterNONE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;
	adr=n*(HDR->X*4);
	for (n1=0;n1<HDR->X*4;n1++)
		ImageDATAF[adr+n1]=ImageDATA[adr+n1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFilterUP(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0;

	if (n>0)
	{
		adr=n*(HDR->X*4);
		adr0=(n-1)*(HDR->X*4);

		for (n1=0;n1<HDR->X*4;n1++)
		{
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr0+n1])&0xFF;
		}
	}
	else
	{
		for (n1=0;n1<HDR->X*4;n1++)
		{
			ImageDATAF[n1]=((unsigned char*)ImageDATA)[n1];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFilterAVERAGE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,prior,raw;

	if (n>0)
	{
		adr=n*(HDR->X*4);
		adr0=(n-1)*(HDR->X*4);

		for (n1=0;n1<4;n1++)
		{
			prior=((unsigned char*)ImageDATAF)[adr0+n1];
            raw=0;
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;
		}

        for (n1=4;n1<HDR->X*4;n1++)
        {
            prior=((unsigned char*)ImageDATAF)[adr0+n1];
            raw=((unsigned char*)ImageDATAF)[adr+n1-4];
            ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;
        }
    }
	else
	{

		adr=n*(HDR->X*4);
		for (n1=0;n1<4;n1++)
		{
            raw=0;
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255))&0xFF;
		}

        for (n1=4;n1<HDR->X*4;n1++)
        {
            raw=((unsigned char*)ImageDATAF)[adr+n1-4];
            ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255))&0xFF;
        }

    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFilterPAETH(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,left,above,upperleft;

	adr=n*HDR->X*4;
	adr0=(n-1)*HDR->X*4;

	for (n1=0;n1<4;n1++)
	{
        left=0; upperleft=0;
        if (n>0) above=((unsigned char*)ImageDATAF)[adr0+n1];
        else above=0;
		ImageDATAF[adr+n1]=(ImageDATA[adr+n1]+PAETH(left,above,upperleft))&0xFF;
	}

    for (n1=4;n1<HDR->X*4;n1++)
    {
        if (n>0)
        {
            left=((unsigned char*)ImageDATAF)[adr-4+n1];
            upperleft=((unsigned char*)ImageDATAF)[adr0-4+n1];
            above=((unsigned char*)ImageDATAF)[adr0+n1];
        }
        else
        {
            upperleft=0;
            above=0;
            left=((unsigned char*)ImageDATAF)[adr-4+n1];
        }
        ImageDATAF[adr+n1]=(ImageDATA[adr+n1]+PAETH(left,above,upperleft))&0xFF;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FILTRES - GRAYSCALE
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void gFilterSUB(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;

	adr=n*(HDR->X);

	ImageDATAF[adr+0]=((unsigned char*)ImageDATA)[adr+0];

	for (n1=1;n1<HDR->X;n1++)
	{
		ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr+n1-1])&0xFF;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gFilterNONE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr;
	adr=n*(HDR->X);
	for (n1=0;n1<HDR->X;n1++)
		ImageDATAF[adr+n1]=ImageDATA[adr+n1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gFilterUP(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0;

	if (n>0)
	{
		adr=n*(HDR->X);
		adr0=(n-1)*(HDR->X);

		for (n1=0;n1<HDR->X;n1++)
		{
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1+0]+((unsigned char*)ImageDATAF)[adr0+n1])&0xFF;
		}
	}
	else
	{
		for (n1=0;n1<HDR->X;n1++)
		{
			ImageDATAF[n1]=((unsigned char*)ImageDATA)[n1];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gFilterAVERAGE(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,prior,raw;

	if (n>0)
	{
		adr=n*(HDR->X);
		adr0=(n-1)*(HDR->X);
        
        n1=0;
        prior=((unsigned char*)ImageDATAF)[adr0+n1];
        raw=0;
        ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;

		for (n1=1;n1<HDR->X;n1++)
		{
			prior=((unsigned char*)ImageDATAF)[adr0+n1];
            raw=((unsigned char*)ImageDATAF)[adr+n1-1];
			ImageDATAF[adr+n1]=(((unsigned char*)ImageDATA)[adr+n1]+(((raw+prior)>>1)&255))&0xFF;
		}
	}
	else
	{
		adr=n*(HDR->X);
        
        n1=0;
        raw=0;
        ImageDATAF[adr+n1]=((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255);
        
		for (n1=1;n1<HDR->X;n1++)
		{
            raw=ImageDATAF[adr+n1-1]&255;
			ImageDATAF[adr+n1]=((unsigned char*)ImageDATA)[adr+n1]+(((raw)>>1)&255);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gFilterPAETH(int n,char * ImageDATA,char * ImageDATAF,struct HEADER *HDR)
{
	unsigned int n1,adr,adr0,left,above,upperleft;

	adr=n*HDR->X;
	adr0=(n-1)*HDR->X;

    n1=0;
    left=0;upperleft=0;
    if (n>0) above=((unsigned char*)ImageDATAF)[adr0+n1];
    else above=0;
    
    ImageDATAF[adr+n1]=((int)((unsigned char*)ImageDATA)[adr+n1]+PAETH(left,above,upperleft))&0xFF;
    
	for (n1=1;n1<HDR->X;n1++)
	{
        if (n>0)
        {
            left=((unsigned char*)ImageDATAF)[adr-1+n1];
            upperleft=((unsigned char*)ImageDATAF)[adr0-1+n1];
            above=((unsigned char*)ImageDATAF)[adr0+n1];
        }
        else
        {
            upperleft=0;
            above=0;
            left=((unsigned char*)ImageDATAF)[adr-1+n1];
        }
		ImageDATAF[adr+n1]=((int)((unsigned char*)ImageDATA)[adr+n1]+PAETH(left,above,upperleft))&0xFF;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ALPHACOLORKEY 7
#define ALPHA1 0
#define ALPHA0 1
#define ALPHAINT 2
#define ALPHATRANSPARENT0 3
#define ALPHATRANSPARENT1 4
#define ALPHATRANSPARENT2 5
#define ALPHACHOISI 6
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PNG_Opacity=255;
int PNG_Color=0;
int PNG_r,PNG_g,PNG_b;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// + a partir d'un buffer...
float iabs(int n)
{
	if (n<0) return -n;
	else return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(ANDROID) || defined(SAVEPNG)
void COMPRESS_PNG_ALPHA(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SAVE_PNG(char * nomPNG,struct IMAGE_DATAS * im,int filter)
{
	struct HEADER HDR;
	unsigned int adr,size;
	char * ptr;
	unsigned char id[8]={137,80,78,71,13,10,26,10};
	char *sHDR="IHDR";
	char *sDAT="IDAT";
	char *sEND="IEND";
	int x,y;
	int r,g,b;
	int ar,ag,ab;
	int pr,pg,pb;
	unsigned int n;
	__FILE f;
	unsigned int zero=0;

	if (im->Alpha)
	{
		int sz;
		char *buf=(char*) malloc(im->X*im->Y*4*2);
		COMPRESS_PNG_ALPHA(im,FILTER_NONE,buf,&sz);
		f=__fopen(nomPNG,"wb");
		if(f==NULL) {
			return;
		}
		__fwrite(buf,sz,1,f);
		__fclose(f);
		free(buf);

		return;
	}

	if (im->Grayscale)
	{
		size=(im->X*im->Y) + im->Y;
		ptr=(char*) malloc(size);
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=0;
			adr++;
			for (x=0;x<(int) im->X;x++)
			{
				r=im->ptrImageDATAS[y*im->X + x]&255;
				ptr[adr++]=r;
			}
		}
		HDR.C_Method=0;
		HDR.Filter=0;
		HDR.Interlace=0;
		HDR.Bit_Depth=8;
		HDR.ColorType=0;
		x=ib(im->X);
		y=ib(im->Y);
		memcpy(HDR.LEN_X,&x,4);
		memcpy(HDR.LEN_Y,&y,4);

		uLongf sz=(int)(size*KOEF_COMPRESSION);
		char *zlib=(char*) malloc(sz);
		compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

		f=__fopen(nomPNG,"wb");

		__fwrite(id,8,1,f);
		//IHDR
		n=ib(13);
		__fwrite(&n,4,1,f);
		__fwrite(sHDR,4,1,f);
		__fwrite(&HDR,13,1,f);
		zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
		__fwrite(&zero,4,1,f);
		//IDAT
		n=ib(sz);
		__fwrite(&n,4,1,f);
		__fwrite(sDAT,4,1,f);
		__fwrite(zlib,sz,1,f);
		zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
		__fwrite(&zero,4,1,f);
		//IEND
		n=0;
		__fwrite(&n,4,1,f);
		__fwrite(sEND,4,1,f);
		zero=ib(crc(sEND,NULL,0));
		__fwrite(&zero,4,1,f);
		__fclose(f);
		free(zlib);
		free(ptr);
	}
	else
	{
		size=(im->X*im->Y*3) + im->Y;
		ptr=(char*) malloc(size);

		if (filter==FILTER_NONE)
		{
			adr=0;
			for (y=0;y<(int) im->Y;y++)
			{
				ptr[adr]=0;
				adr++;

				for (x=0;x<(int) im->X;x++)
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r;
					ptr[adr+1]=g;
					ptr[adr+2]=b;

					adr+=3;

				}
			}
		}

		if (filter==FILTER_SUB)
		{
			adr=0;
			for (y=0;y<(int) im->Y;y++)
			{
				ptr[adr]=1;
				adr++;

				for (x=0;x<(int) im->X;x++)
				{
					if (x==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r;
						ptr[adr+1]=g;
						ptr[adr+2]=b;
						ar=r;
						ag=g;
						ab=b;
					}
					else
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-ar;
						ptr[adr+1]=g-ag;
						ptr[adr+2]=b-ab;
						ar=r;
						ag=g;
						ab=b;
					}

					adr+=3;
				}
			}
		}

		if (filter==FILTER_UP)
		{
			//up
			adr=0;
			for (y=0;y<(int) im->Y;y++)
			{
				ptr[adr]=2;
				adr++;

				for (x=0;x<(int) im->X;x++)
				{
					if (y==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r;
						ptr[adr+1]=g;
						ptr[adr+2]=b;
					}
					else
					{
						pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
						pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
						pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-pr;
						ptr[adr+1]=g-pg;
						ptr[adr+2]=b-pb;
					}

					adr+=3;
				}
			}
		}

		if (filter==FILTER_AVERAGE)
		{
			// Average(x) = Raw(x) - floor((Raw(x-bpp)+Prior(x))/2)
			adr=0;
			for (y=0;y<(int) im->Y;y++)
			{
				ptr[adr]=3;
				adr++;

				for (x=0;x<(int) im->X;x++)
				{
					if (y==0)
					{
						if (x==0)
						{
							r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
							g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
							b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
							ptr[adr+0]=r;
							ptr[adr+1]=g;
							ptr[adr+2]=b;
							ar=r;
							ag=g;
							ab=b;

						}
						else
						{
							r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
							g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
							b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
							ptr[adr+0]=r-(ar>>1);
							ptr[adr+1]=g-(ag>>1);
							ptr[adr+2]=b-(ab>>1);
							ar=r;
							ag=g;
							ab=b;
						}
					}
					else
					{
						pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
						pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
						pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;

						if (x==0)
						{
							r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
							g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
							b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
							ptr[adr+0]=r-(pr>>1);
							ptr[adr+1]=g-(pg>>1);
							ptr[adr+2]=b-(pb>>1);
							ar=r;
							ag=g;
							ab=b;
						}
						else
						{
							r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
							g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
							b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
							ptr[adr+0]=r-((ar+pr)>>1);
							ptr[adr+1]=g-((ag+pg)>>1);
							ptr[adr+2]=b-((ab+pb)>>1);
							ar=r;
							ag=g;
							ab=b;

						}

					}

					adr+=3;
				}
			}
		}

		HDR.C_Method=0;
		HDR.Filter=0;
		HDR.Interlace=0;
		HDR.Bit_Depth=8;

		HDR.ColorType=2;

		x=ib(im->X);
		y=ib(im->Y);
		memcpy(HDR.LEN_X,&x,4);
		memcpy(HDR.LEN_Y,&y,4);

		uLongf sz=(int)(size*KOEF_COMPRESSION);
		char *zlib=(char*) malloc(sz);
		compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

		f=__fopen(nomPNG,"wb");
		__fwrite(id,8,1,f);
		//IHDR
		n=ib(13);
		__fwrite(&n,4,1,f);
		__fwrite(sHDR,4,1,f);
		__fwrite(&HDR,13,1,f);
		zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
		__fwrite(&zero,4,1,f);
		//IDAT
		n=ib(sz);
		__fwrite(&n,4,1,f);
		__fwrite(sDAT,4,1,f);
		__fwrite(zlib,sz,1,f);
		zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
		__fwrite(&zero,4,1,f);
		//IEND
		n=0;
		__fwrite(&n,4,1,f);
		__fwrite(sEND,4,1,f);
		zero=ib(crc(sEND,NULL,0));
		__fwrite(&zero,4,1,f);

		__fclose(f);

		free(zlib);
		free(ptr);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void QUICKSAVE_PNG(char * nomPNG,struct IMAGE_DATAS * im,int filter)
{
	struct HEADER HDR;
	unsigned int adr,size;
	char * ptr;
	unsigned char id[8]={137,80,78,71,13,10,26,10};
	char *sHDR="IHDR";
	char *sDAT="IDAT";
	char *sEND="IEND";
	int x,y;
	int r,g,b;
	int ar,ag,ab;
	int pr,pg,pb;
	unsigned int n;
	__FILE f;
	unsigned int zero=0;

	size=(im->X*im->Y*3) + im->Y;

	ptr=(char*) malloc(size);

	if (filter==FILTER_NONE)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=0;
			adr++;
			for (x=0;x<(int) im->X;x++)
			{
				r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
				g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
				b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
				ptr[adr+0]=r;
				ptr[adr+1]=g;
				ptr[adr+2]=b;
				adr+=3;
			}
		}
	}

	if (filter==FILTER_SUB)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=1;
			adr++;
			for (x=0;x<(int) im->X;x++)
			{
				if (x==0)
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r;
					ptr[adr+1]=g;
					ptr[adr+2]=b;
					ar=r;
					ag=g;
					ab=b;
				}
				else
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r-ar;
					ptr[adr+1]=g-ag;
					ptr[adr+2]=b-ab;
					ar=r;
					ag=g;
					ab=b;
				}
				adr+=3;
			}
		}
	}

	if (filter==FILTER_UP)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=2;
			adr++;
			for (x=0;x<(int) im->X;x++)
			{
				if (y==0)
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r;
					ptr[adr+1]=g;
					ptr[adr+2]=b;
				}
				else
				{
					pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
					pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
					pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r-pr;
					ptr[adr+1]=g-pg;
					ptr[adr+2]=b-pb;
				}
				adr+=3;
			}
		}
	}

	if (filter==FILTER_AVERAGE)
	{
		// Average(x) = Raw(x) - floor((Raw(x-bpp)+Prior(x))/2)
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=3;
			adr++;

			for (x=0;x<(int) im->X;x++)
			{
				if (y==0)
				{
					if (x==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r;
						ptr[adr+1]=g;
						ptr[adr+2]=b;
						ar=r;
						ag=g;
						ab=b;

					}
					else
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-(ar>>1);
						ptr[adr+1]=g-(ag>>1);
						ptr[adr+2]=b-(ab>>1);
						ar=r;
						ag=g;
						ab=b;

					}
				}
				else
				{
					pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
					pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
					pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;
					if (x==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-(pr>>1);
						ptr[adr+1]=g-(pg>>1);
						ptr[adr+2]=b-(pb>>1);
						ar=r;
						ag=g;
						ab=b;
					}
					else
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-((ar+pr)>>1);
						ptr[adr+1]=g-((ag+pg)>>1);
						ptr[adr+2]=b-((ab+pb)>>1);
						ar=r;
						ag=g;
						ab=b;
					}
				}
				adr+=3;
			}
		}
	}

	HDR.C_Method=0;
	HDR.Filter=0;
	HDR.Interlace=0;
	HDR.Bit_Depth=8;

	HDR.ColorType=2;

	x=ib(im->X);
	y=ib(im->Y);
	memcpy(HDR.LEN_X,&x,4);
	memcpy(HDR.LEN_Y,&y,4);

	uLongf sz=(int)(size*KOEF_COMPRESSION);
	char *zlib=(char*) malloc(sz);
	compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

	f=__fopen(nomPNG,"wb");
	__fwrite(id,8,1,f);
	//IHDR
	n=ib(13);
	__fwrite(&n,4,1,f);
	__fwrite(sHDR,4,1,f);
	__fwrite(&HDR,13,1,f);
	zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
	__fwrite(&zero,4,1,f);
	//IDAT
	n=ib(sz);
	__fwrite(&n,4,1,f);
	__fwrite(sDAT,4,1,f);
	__fwrite(zlib,sz,1,f);
	zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
	__fwrite(&zero,4,1,f);
	//IEND
	n=0;
	__fwrite(&n,4,1,f);
	__fwrite(sEND,4,1,f);
	zero=ib(crc(sEND,NULL,0));
	__fwrite(&zero,4,1,f);

	__fclose(f);

	free(zlib);
	free(ptr);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(ANDROID)||defined(READPNG)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void READ_PNG(char * nomPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha)
{
	struct HEADER HDR;
	char * ImageDATA;
	char * ImageDATAF;
	int ind;
	char * ImageDATAtmp;
	int grayscale;
	int piDATA;
	char * ptrbuf;
	int r,g,b,a;
	FILE *f;
	unsigned int adr,adr0;
	int n,nn;
	unsigned int n0,n1;
	unsigned int nnn;
	char BUF[512];
	char B3210[4];
	int filter[16384];
	int alpha;
	char sHDR[5];
	char sDAT[5];
	unsigned int crc_value;
	char *BLOCK;
	int palette;
	int prev_nnn;
	unsigned char rgb[256*3];
    
	f=fopen(nomPNG,"rb");
	if(f==NULL) {
		return;
	}
    
    fseek(f,0,SEEK_END);
    int sizepng=ftell(f);
    fseek(f,0,SEEK_SET);
    
	//LZCOMPRESS *LZ=new LZCOMPRESS;
    
	fread(&BUF[0],8,1,f);
	nn=0;
	for (n=0;n<8;n++)
	{
		nn+=BUF[n]&255;
	}

	// 425
	fread(&B3210[0],4,1,f);
	nnn=B00(B3210);

	fread(&sHDR[0],4,1,f);
	sHDR[4]='\0';

	fread(&HDR,13,1,f);

	HDR.X=B00(HDR.LEN_X);
	HDR.Y=B00(HDR.LEN_Y);

	alpha=0;
	grayscale=0;
	palette=0;
	if (HDR.ColorType==2) alpha=0;
	if (HDR.ColorType==6) alpha=1;
	if (HDR.ColorType==0) grayscale=1;
	if (HDR.ColorType==3) palette=1;

	if (palette==0)
	{
		if (grayscale==0)
		{
			ImageDATA=(char *) malloc(HDR.X*HDR.Y*(3+alpha));
			ImageDATAF=(char *) malloc(HDR.X*HDR.Y*(3+alpha));

			ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y*(3+alpha)+HDR.Y);

			IMDATAS->X=HDR.X;
			IMDATAS->Y=HDR.Y;
			IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y*4);

			IMDATAS->Grayscale=0;

//			LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y*(3+alpha)+HDR.Y;
		}
		else
		{

//			LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y+HDR.Y;

			ImageDATA=(char *) malloc(HDR.X*HDR.Y);
			ImageDATAF=(char *) malloc(HDR.X*HDR.Y);

			ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y+HDR.Y);

			IMDATAS->X=HDR.X;
			IMDATAS->Y=HDR.Y;
			IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y);
			IMDATAS->Grayscale=1;

		}
	}
	else
	{
//		LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y+HDR.Y;

		ImageDATA=(char *) malloc(HDR.X*HDR.Y);
		ImageDATAF=(char *) malloc(HDR.X*HDR.Y);

		ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y+HDR.Y);

		IMDATAS->X=HDR.X;
		IMDATAS->Y=HDR.Y;
		IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y*4);
		IMDATAS->Grayscale=0;
	}

	IMDATAS->Alpha=alpha;
	piDATA=0;

	fread(&crc_value,4,1,f);

    BLOCK=NULL;
    
#ifdef USE_CRC_CHECK
	if (crc_value==ib(crc(sHDR,(unsigned char*)&HDR,13)))
#endif
	{
		n0=0;
		n=0;
		nn=0;
		sDAT[0]='A';
		sDAT[1]='A';
		sDAT[2]='A';
		sDAT[3]='\0';
		
		prev_nnn=0;

		int pp=ftell(f);
		bool data=false;

		fread(&sDAT[0],4,1,f);
		while (!data)
		{
			if ((sDAT[0]=='I')&&(sDAT[1]=='D')&&(sDAT[2]=='A')&&(sDAT[3]=='T')) data=true;
			if ((sDAT[0]=='P')&&(sDAT[1]=='L')&&(sDAT[2]=='T')&&(sDAT[3]=='E')) data=true;
			if (!data)
			{
				pp++;
				fseek(f,pp,SEEK_SET);
				fread(&sDAT[0],4,1,f);
			}
		}

		pp-=4;
		fseek(f,pp,SEEK_SET);
        
		while (sDAT[3]!='D')
		{
			fread(&B3210[0],4,1,f);
			nnn=B00(B3210);
			fread(&sDAT[0],4,1,f);
			n0++;

			if ((sDAT[0]=='P')&&(sDAT[1]=='L')&&(sDAT[2]=='T')&&(sDAT[3]=='E'))
			{
				fread(rgb,nnn,1,f);
				fread(&crc_value,4,1,f);
			}
			else
			{
				if ((sDAT[0]=='I')&&(sDAT[1]=='D')&&(sDAT[2]=='A')&&(sDAT[3]=='T'))
				{
                    if (BLOCK==NULL)
                    {
                        BLOCK=(char*) malloc (sizepng);
                        memset(BLOCK,0,sizepng);
                    }

					fread(&BLOCK[prev_nnn],nnn,1,f);
					fread(&crc_value,4,1,f);
                    prev_nnn+=nnn;

#ifdef USE_CRC_CHECK
					//if (crc_value==ib(crc(sDAT,(unsigned char*) dataBLOCK,nnn)))
#endif
				}
				else
				{
                    ptrbuf=(char *) malloc(nnn);
					fread(ptrbuf,nnn,1,f);
                    free(ptrbuf);
					fread(&crc_value,4,1,f);
				}
			}
		}

		if (BLOCK)
		{
//			piDATA+=LZ->DEFLATE_zlibBlock(BLOCK,ImageDATAtmp,piDATA);
            
            uLongf tsize=HDR.X*HDR.Y*(3+alpha)+HDR.Y;
            uncompress((Bytef*)ImageDATAtmp, &tsize, (Bytef*)BLOCK, prev_nnn);
			free(BLOCK);
		}

	}
    
	fclose(f);
    
    if (BLOCK==NULL)
    {
        IMDATAS->X=HDR.X=16;
        IMDATAS->Y=HDR.Y=16;
        IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y*4);
        memset(IMDATAS->ptrImageDATAS,0,HDR.X*HDR.Y*4);
        IMDATAS->Grayscale=0;
        IMDATAS->Alpha=0;
        return;
    }

    //delete LZ;

	if (palette==1)
	{
		for (n0=0;n0<HDR.Y;n0++)
		{
			adr=1+n0*(HDR.X +1);
			adr0=n0*(HDR.X);

			filter[n0]=ImageDATAtmp[adr-1]&255;

			for (n1=0;n1<HDR.X;n1++)
			{
				ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
			}
		}

		for (n0=0;n0<HDR.Y;n0++)
		{
			n1=filter[n0];
			if (n1==0) gFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
			else
			{
				if (n1==1) gFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
				else
				{
					if (n1==2) gFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==3) gFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
						else gFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
					}
				}
			}
		}

		if (TypeAlpha==88)
		{
			for (n0=0;n0<HDR.X*HDR.Y;n0++)
			{
				ind=ImageDATAF[n0]&255;
				IMDATAS->ptrImageDATAS[n0+0]=ind;
			}
		}
		else
		{
			for (n0=0;n0<HDR.X*HDR.Y;n0++)
			{
				ind=ImageDATAF[n0]&255;
				IMDATAS->ptrImageDATAS[n0*4+0]=rgb[ind*3+0];
				IMDATAS->ptrImageDATAS[n0*4+1]=rgb[ind*3+1];
				IMDATAS->ptrImageDATAS[n0*4+2]=rgb[ind*3+2];
				IMDATAS->ptrImageDATAS[n0*4+3]=(unsigned char) 255;
			}
		}
	}
	else
	{
		if (grayscale==1)
		{
			for (n0=0;n0<HDR.Y;n0++)
			{
				adr=1+n0*(HDR.X +1);
				adr0=n0*(HDR.X);

				filter[n0]=ImageDATAtmp[adr-1]&255;

				for (n1=0;n1<HDR.X;n1++)
				{
					ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
				}
			}

			for (n0=0;n0<HDR.Y;n0++)
			{
				n1=filter[n0];
				if (n1==0) gFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
				else
				{
					if (n1==1) gFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==2) gFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==3) gFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
							else gFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
						}
					}
				}
			}

			for (n0=0;n0<HDR.X*HDR.Y;n0++)
			{
				IMDATAS->ptrImageDATAS[n0]=ImageDATAF[n0]&255;
			}
		}
		else
		{
			if (alpha==0)
			{
				for (n0=0;n0<HDR.Y;n0++)
				{

					adr=1+n0*(HDR.X*3 +1);
					adr0=n0*(HDR.X*3);

					filter[n0]=ImageDATAtmp[adr-1]&255;
					for (n1=0;n1<HDR.X*3;n1++)
					{
						ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
					}
				}

				for (n0=0;n0<HDR.Y;n0++)
				{
					n1=filter[n0];
					if (n1==0) FilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==1) FilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==2) FilterUP(n0,ImageDATA,ImageDATAF,&HDR);
							else
							{
								if (n1==3) FilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
								else FilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
							}
						}
					}
				}
			}
			else
			{
				for (n0=0;n0<HDR.Y;n0++)
				{
					adr=1+n0*(HDR.X*4 +1);
					adr0=n0*(HDR.X*4);
					filter[n0]=ImageDATAtmp[adr-1]&255;
					for (n1=0;n1<HDR.X;n1++)
					{
						ImageDATA[adr0+0]=ImageDATAtmp[adr+0]&255;
						ImageDATA[adr0+1]=ImageDATAtmp[adr+1]&255;
						ImageDATA[adr0+2]=ImageDATAtmp[adr+2]&255;
						ImageDATA[adr0+3]=ImageDATAtmp[adr+3]&255;
						adr0+=4;
						adr+=4;
					}
				}

				for (n0=0;n0<HDR.Y;n0++)
				{
					n1=filter[n0];
					if (n1==0) AFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==1) AFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==2) AFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
							else
							{
								if (n1==3) AFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
								else AFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
							}
						}
					}
				}

				adr0=adr=0;
				unsigned int adr2;
				adr2=0;
				for (n0=0;n0<HDR.X*HDR.Y;n0++)
				{
					IMDATAS->ptrImageDATAS[adr0+0]=ImageDATAF[adr+0]&255;
					IMDATAS->ptrImageDATAS[adr0+1]=ImageDATAF[adr+1]&255;
					IMDATAS->ptrImageDATAS[adr0+2]=ImageDATAF[adr+2]&255;
					IMDATAS->ptrImageDATAS[adr0+3]=ImageDATAF[adr+3]&255;

					adr0+=4;
					adr+=4;
					adr2++;
				}
			}

			if (alpha==0)
			{
				adr0=adr=0;
				for (n0=0;n0<HDR.X*HDR.Y;n0++)
				{
					r=ImageDATAF[adr+0]&255;
					IMDATAS->ptrImageDATAS[adr0+0]=r;
					g=ImageDATAF[adr+1]&255;
					IMDATAS->ptrImageDATAS[adr0+1]=g;
					b=ImageDATAF[adr+2]&255;
					IMDATAS->ptrImageDATAS[adr0+2]=b;

					if (TypeAlpha==ALPHA1)
					{
						IMDATAS->ptrImageDATAS[adr0+3]=(char) 255;
					}
					else
					{
						if (TypeAlpha==ALPHACOLORKEY)
						{
							if ((iabs(r-255)<3)&&(g<3)&&(iabs(b-255)<3))
							{
								IMDATAS->ptrImageDATAS[adr0+3]=(char) 0;
								IMDATAS->ptrImageDATAS[adr0+0]=(char) 0;
								IMDATAS->ptrImageDATAS[adr0+1]=(char) 0;
								IMDATAS->ptrImageDATAS[adr0+2]=(char) 0;
							}
							else
							{
								IMDATAS->ptrImageDATAS[adr0+3]=(char) PNG_Opacity;
							}
						}
						else
						if (TypeAlpha==ALPHA0)
						{
							if (r+g+b<10) IMDATAS->ptrImageDATAS[adr0+3]=(char) 0;
							else IMDATAS->ptrImageDATAS[adr0+3]=(char) PNG_Opacity;
						}
						else
						{
							if (TypeAlpha==ALPHAINT)
							{
								float intensite;
								float rr,gg,bb;

								rr=(float) r/255;
								gg=(float) g/255;
								bb=(float) b/255;

								intensite=0.7f*gg+0.2f*rr+0.1f*bb;

								if (intensite>0.7f) a=(char) 255;
								else
									if (intensite<PNG_valueofbeefsteack) a=0;
									else
										a=(char) ((intensite-PNG_valueofbeefsteack)*255/0.5f);

								IMDATAS->ptrImageDATAS[adr0+3]=a;

								if (PNG_Color==1)
								{
									IMDATAS->ptrImageDATAS[adr0+0]=PNG_r;
									IMDATAS->ptrImageDATAS[adr0+1]=PNG_g;
									IMDATAS->ptrImageDATAS[adr0+2]=PNG_b;
								}
							}
							else
							{
								if (TypeAlpha==ALPHATRANSPARENT0)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 128;
								else
								if (TypeAlpha==ALPHATRANSPARENT1)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 70;
								else
								if (TypeAlpha==ALPHATRANSPARENT2)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 150;
								else
								if (TypeAlpha==ALPHACHOISI)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) PNG_Opacity;
							}
						}
					}
					adr0+=4;
					adr+=3;
				}
			}
		}
	}

	free(ImageDATA);
	free(ImageDATAtmp);
	free(ImageDATAF);
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read png  from mem
class VirtFile
{
public:
	char * PTRREAD;
	int nREAD;

	VirtFile() {}

	int mtell()
	{
		return nREAD;
	}

	void mseek(int p)
	{
		nREAD=p;
	}

	void mread(char * ptr,int len)
	{
        memcpy(ptr,&PTRREAD[nREAD],len);
        nREAD+=len;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ALPHA_READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,int Opacity)
{
    struct HEADER HDR;
    VirtFile *F=new VirtFile;
    char BUF[512];
    
    F->PTRREAD=ptrFICHIERPNG;
    F->nREAD=0;

    F->mread(&BUF[0],8);
    F->mread(&BUF[0],4);
    F->mread(&BUF[0],4);
    F->mread((char*)&HDR,13);
    
    delete F;

    if (HDR.ColorType==6) return 1;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(_DEFINES_OPENGL_NO_SDL_INIT_)&&!defined(API3D_VR)
void _READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha,int Opacity)
#else
void READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha,int Opacity)
#endif
{
	struct HEADER HDR;
	char * ImageDATA;
	char * ImageDATAF;
    VirtFile *F=new VirtFile;
	char * ImageDATAtmp;
	int ind;
	int piDATA;
	char * ptrbuf;
	//LZCOMPRESS *LZ=new LZCOMPRESS;
	unsigned int adr,adr0;
	int n,nn;
	unsigned int n0,n1;
	unsigned int nnn,prev_nnn;
	char BUF[512];
	char B3210[4];
	int filter[16384];
	int alpha;
	char sHDR[5];
	char sDAT[5];
	unsigned int crc_value;
	char *BLOCK;
	int grayscale;
	int palette;
	unsigned char rgb[3*256];
    
	F->PTRREAD=ptrFICHIERPNG;
	F->nREAD=0;

	F->mread(&BUF[0],8);
	nn=0;
	for (n=0;n<8;n++)
	{
		nn+=BUF[n]&255;
	}
	// 425
	F->mread(&B3210[0],4);
	nnn=B00(B3210);

	F->mread(&sHDR[0],4);
	sHDR[4]='\0';

	F->mread((char*)&HDR,13);

	HDR.X=B00(HDR.LEN_X);
	HDR.Y=B00(HDR.LEN_Y);

	if ((HDR.X>1024*1024)||(HDR.Y>1024*1024))
	{
		IMDATAS->X=8;
		IMDATAS->Y=8;
		IMDATAS->ptrImageDATAS=(char *) malloc(8*8*4);
		for (n=0;n<8*8*4;n++) ((unsigned char*)IMDATAS->ptrImageDATAS)[n]=255;
		IMDATAS->Grayscale=0;
		IMDATAS->Alpha=1;
		return;
	}

	alpha=0;
	grayscale=0;
	palette=0;

	if (HDR.ColorType==2) alpha=0;
	if (HDR.ColorType==6) alpha=1;
	if (HDR.ColorType==0) grayscale=1;
	if (HDR.ColorType==3) palette=1;
    
    int sizepng=0;

	if (palette==0)
	{
		if (grayscale==0)
		{
			ImageDATA=(char *) malloc(HDR.X*HDR.Y*(3+alpha));
			ImageDATAF=(char *) malloc(HDR.X*HDR.Y*(3+alpha));
            
            sizepng=HDR.X*HDR.Y*(3+alpha)+HDR.Y;

			ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y*(3+alpha)+HDR.Y);

			IMDATAS->X=HDR.X;
			IMDATAS->Y=HDR.Y;
			IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y*4);

			IMDATAS->Grayscale=0;
			IMDATAS->Alpha=alpha;

			//LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y*(3+alpha)+HDR.Y;
		}
		else
		{
			//LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y+HDR.Y;
			ImageDATA=(char *) malloc(HDR.X*HDR.Y);
			ImageDATAF=(char *) malloc(HDR.X*HDR.Y);
            
            sizepng=HDR.X*HDR.Y+HDR.Y;
            
			ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y+HDR.Y);
			IMDATAS->X=HDR.X;
			IMDATAS->Y=HDR.Y;
			IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y);
			IMDATAS->Grayscale=1;
			IMDATAS->Alpha=0;
		}
	}
	else
	{
		//LZ->DEFLATE_LONGUEUR_deflated=HDR.X*HDR.Y+HDR.Y;

		ImageDATA=(char *) malloc(HDR.X*HDR.Y);
		ImageDATAF=(char *) malloc(HDR.X*HDR.Y);

        sizepng=HDR.X*HDR.Y+HDR.Y;
        
		ImageDATAtmp=(char *) malloc(HDR.X*HDR.Y+HDR.Y);
		IMDATAS->X=HDR.X;
		IMDATAS->Y=HDR.Y;
		IMDATAS->ptrImageDATAS=(char *) malloc(HDR.X*HDR.Y*4);
		IMDATAS->Grayscale=0;
		IMDATAS->Alpha=0;
	}

	piDATA=0;

	F->mread((char*) &crc_value,4);
    
	//if (crc_value==ib(crc(sHDR,(unsigned char*)&HDR,13)))
    if (sizepng>0)
	{
		BLOCK=NULL;
		prev_nnn=0;

		n0=0;
		n=0;
		nn=0;
		sDAT[0]='A';
		sDAT[1]='A';
		sDAT[2]='A';
		sDAT[3]='\0';

		int pp=F->mtell();
		bool data=false;

		F->mread(&sDAT[0],4);
		while (!data)
		{
			if ((sDAT[0]=='I')&&(sDAT[1]=='D')&&(sDAT[2]=='A')&&(sDAT[3]=='T')) data=true;
			if ((sDAT[0]=='P')&&(sDAT[1]=='L')&&(sDAT[2]=='T')&&(sDAT[3]=='E')) data=true;
			if (!data)
			{
				pp++;
				F->mseek(pp);
				F->mread(&sDAT[0],4);
			}
		}

		pp-=4;
		F->mseek(pp);
        
        BLOCK=(char*)malloc(sizepng*2);

		while (sDAT[3]!='D')
		{
			F->mread(&B3210[0],4);
			nnn=B00(B3210);
			F->mread(&sDAT[0],4);
			sDAT[4]='\0';

			n0++;

			if ((sDAT[0]=='P')&&(sDAT[1]=='L')&&(sDAT[2]=='T')&&(sDAT[3]=='E'))
			{
				F->mread((char*) &rgb[0],nnn);
				F->mread((char*)&crc_value,4);
			}
			else
			{
				if ((sDAT[0]=='I')&&(sDAT[1]=='D')&&(sDAT[2]=='A')&&(sDAT[3]=='T'))
				{
					F->mread(&BLOCK[prev_nnn],nnn);
					F->mread((char*)&crc_value,4);
                    prev_nnn+=nnn;
				}
				else
                {
                    ptrbuf=(char *) malloc(nnn+16);
					F->mread(ptrbuf,nnn);
                    free(ptrbuf);
					F->mread((char*) &crc_value,4);
				}
			}
		}

//		piDATA+=LZ->DEFLATE_zlibBlock(BLOCK,ImageDATAtmp,piDATA);
        
        uLongf tsize=HDR.X*HDR.Y*(3+alpha)+HDR.Y;
        uncompress((Bytef*)ImageDATAtmp, &tsize, (Bytef*)BLOCK, prev_nnn);

		free(BLOCK);
	}
    
//    delete LZ;
    delete F;

	if (palette==1)
	{
		for (n0=0;n0<HDR.Y;n0++)
		{
			adr=1+n0*(HDR.X +1);
			adr0=n0*(HDR.X);
			filter[n0]=ImageDATAtmp[adr-1]&255;
			for (n1=0;n1<HDR.X;n1++) ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
		}

		for (n0=0;n0<HDR.Y;n0++)
		{
			n1=filter[n0];
			if (n1==0) gFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
			else
			{
				if (n1==1) gFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
				else
				{
					if (n1==2) gFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==3) gFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
						else gFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
					}
				}
			}
		}

		for (n0=0;n0<HDR.X*HDR.Y;n0++)
		{
			ind=ImageDATAF[n0]&255;
			IMDATAS->ptrImageDATAS[n0*4+0]=rgb[ind*3+0];
			IMDATAS->ptrImageDATAS[n0*4+1]=rgb[ind*3+1];
			IMDATAS->ptrImageDATAS[n0*4+2]=rgb[ind*3+2];
			IMDATAS->ptrImageDATAS[n0*4+3]=(unsigned char)255;
		}
	}
	else
	{
		if (grayscale==1)
		{
			for (n0=0;n0<HDR.Y;n0++)
			{
				adr=1+n0*(HDR.X +1);
				adr0=n0*(HDR.X);
				filter[n0]=ImageDATAtmp[adr-1]&255;
				for (n1=0;n1<HDR.X;n1++) ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
			}

			for (n0=0;n0<HDR.Y;n0++)
			{
				n1=filter[n0];
				if (n1==0) gFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
				else
				{
					if (n1==1) gFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==2) gFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==3) gFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
							else gFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
						}
					}
				}
			}

			for (n0=0;n0<HDR.X*HDR.Y;n0++) IMDATAS->ptrImageDATAS[n0]=ImageDATAF[n0]&255;
		}
		else
		{
			if (alpha==0)
			{
				for (n0=0;n0<HDR.Y;n0++)
				{
					adr=1+n0*(HDR.X*3 +1);
					adr0=n0*(HDR.X*3);
					filter[n0]=ImageDATAtmp[adr-1]&255;
					for (n1=0;n1<HDR.X*3;n1++) ImageDATA[adr0+n1]=ImageDATAtmp[adr+n1];
				}

				for (n0=0;n0<HDR.Y;n0++)
				{
					n1=filter[n0];
					if (n1==0) FilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==1) FilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==2) FilterUP(n0,ImageDATA,ImageDATAF,&HDR);
							else
							{
								if (n1==3) FilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
								else FilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
							}
						}
					}
				}
			}
			else
			{
				for (n0=0;n0<HDR.Y;n0++)
				{
					adr=1+n0*(HDR.X*4 +1);
					adr0=n0*(HDR.X*4);
					filter[n0]=ImageDATAtmp[adr-1]&255;
					for (n1=0;n1<HDR.X;n1++)
					{
						ImageDATA[adr0+0]=ImageDATAtmp[adr+0]&255;
						ImageDATA[adr0+1]=ImageDATAtmp[adr+1]&255;
						ImageDATA[adr0+2]=ImageDATAtmp[adr+2]&255;
						ImageDATA[adr0+3]=ImageDATAtmp[adr+3]&255;
						adr0+=4;
						adr+=4;
					}
				}

				for (n0=0;n0<HDR.Y;n0++)
				{
					n1=filter[n0];
					if (n1==0) AFilterNONE(n0,ImageDATA,ImageDATAF,&HDR);
					else
					{
						if (n1==1) AFilterSUB(n0,ImageDATA,ImageDATAF,&HDR);
						else
						{
							if (n1==2) AFilterUP(n0,ImageDATA,ImageDATAF,&HDR);
							else
							{
								if (n1==3) AFilterAVERAGE(n0,ImageDATA,ImageDATAF,&HDR);
								else AFilterPAETH(n0,ImageDATA,ImageDATAF,&HDR);
							}
						}
					}
				}

				adr0=adr=0;
				unsigned int adr2;
				adr2=0;
				for (n0=0;n0<HDR.X*HDR.Y;n0++)
				{
					IMDATAS->ptrImageDATAS[adr0+0]=ImageDATAF[adr+0]&255;
					IMDATAS->ptrImageDATAS[adr0+1]=ImageDATAF[adr+1]&255;
					IMDATAS->ptrImageDATAS[adr0+2]=ImageDATAF[adr+2]&255;
					IMDATAS->ptrImageDATAS[adr0+3]=ImageDATAF[adr+3]&255;
					adr0+=4;
					adr+=4;
					adr2++;
				}
			}

			if (alpha==0)
			{
				adr0=adr=0;
				for (n0=0;n0<HDR.X*HDR.Y;n0++)
				{
					IMDATAS->ptrImageDATAS[adr0+0]=ImageDATAF[adr+0]&255;
					IMDATAS->ptrImageDATAS[adr0+1]=ImageDATAF[adr+1]&255;
					IMDATAS->ptrImageDATAS[adr0+2]=ImageDATAF[adr+2]&255;

					if (TypeAlpha==ALPHA1)
					{
						IMDATAS->ptrImageDATAS[adr0+3]=(char) 255;
					}
					else
					{
						if (TypeAlpha==ALPHA0)
						{
							int r,g,b;

							r=IMDATAS->ptrImageDATAS[adr0+0]&255;
							g=IMDATAS->ptrImageDATAS[adr0+1]&255;
							b=IMDATAS->ptrImageDATAS[adr0+2]&255;

							if (r+g+b<10)
								IMDATAS->ptrImageDATAS[adr0+3]=(char) 0;
							else
								IMDATAS->ptrImageDATAS[adr0+3]=(char) PNG_Opacity;
						}
						else
						{
							if (TypeAlpha==ALPHAINT)
							{
								int r,g,b;
								float intensite;
								float rr,gg,bb;

								r=IMDATAS->ptrImageDATAS[adr0+0]&255;
								g=IMDATAS->ptrImageDATAS[adr0+1]&255;
								b=IMDATAS->ptrImageDATAS[adr0+2]&255;

								rr=(float) r/255;
								gg=(float) g/255;
								bb=(float) b/255;

								intensite=0.7f*gg+0.2f*rr+0.1f*bb;

								if (intensite>0.7f) r=(char) 255;
								else
									r=(char) (intensite*255/0.7f);

								IMDATAS->ptrImageDATAS[adr0+3]=r;
							}
							else
							{
								if (TypeAlpha==ALPHATRANSPARENT0)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 128;
								else
								if (TypeAlpha==ALPHATRANSPARENT1)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 70;
								else
								if (TypeAlpha==ALPHATRANSPARENT2)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) 150;
								else
								if (TypeAlpha==ALPHACHOISI)
									IMDATAS->ptrImageDATAS[adr0+3]=(char) Opacity;
							}
						}
					}
					adr0+=4;
					adr+=3;
				}
			}
		}
	}
	free(ImageDATA);
	free(ImageDATAtmp);
	free(ImageDATAF);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void READ_PNG_FROM_MEM(char * ptrFICHIERPNG,struct IMAGE_DATAS * IMDATAS,char TypeAlpha)
{
#if defined(_DEFINES_OPENGL_NO_SDL_INIT_)&&!defined(API3D_VR)
	_READ_PNG_FROM_MEM(ptrFICHIERPNG,IMDATAS,TypeAlpha,PNG_Opacity);
#else
	READ_PNG_FROM_MEM(ptrFICHIERPNG,IMDATAS,TypeAlpha,PNG_Opacity);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void COMPRESS_PNG(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest)
{
	struct HEADER HDR;
	unsigned int adr,size;
	char * ptr;
	unsigned char id[8]={137,80,78,71,13,10,26,10};
	char *sHDR="IHDR";
	char *sDAT="IDAT";
	char *sEND="IEND";
	int x,y;
	int r,g,b;
	int ar,ag,ab;
	int pr,pg,pb;
	unsigned int n;
	unsigned int zero=0;

	size=(im->X*im->Y*3) + im->Y;
	ptr=(char*) malloc(size);

	if (filter==FILTER_NONE)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=0;
			adr++;
			for (x=0;x<(int) im->X;x++)
			{
				r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
				g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
				b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
				ptr[adr+0]=r;
				ptr[adr+1]=g;
				ptr[adr+2]=b;
				adr+=3;
			}
		}
	}

	if (filter==FILTER_SUB)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=1;
			adr++;

			for (x=0;x<(int) im->X;x++)
			{
				if (x==0)
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r;
					ptr[adr+1]=g;
					ptr[adr+2]=b;
					ar=r;
					ag=g;
					ab=b;
				}
				else
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r-ar;
					ptr[adr+1]=g-ag;
					ptr[adr+2]=b-ab;
					ar=r;
					ag=g;
					ab=b;
				}
				adr+=3;
			}
		}
	}

	if (filter==FILTER_UP)
	{
		//up
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr++]=2;

			for (x=0;x<(int) im->X;x++)
			{
				if (y==0)
				{
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r;
					ptr[adr+1]=g;
					ptr[adr+2]=b;
				}
				else
				{
					pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
					pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
					pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;
					r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
					g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
					b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
					ptr[adr+0]=r-pr;
					ptr[adr+1]=g-pg;
					ptr[adr+2]=b-pb;
				}
				adr+=3;
			}
		}
	}

	if (filter==FILTER_AVERAGE)
	{
		// Average(x) = Raw(x) - floor((Raw(x-bpp)+Prior(x))/2)

		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr++]=3;

			for (x=0;x<(int) im->X;x++)
			{
				if (y==0)
				{
					if (x==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r;
						ptr[adr+1]=g;
						ptr[adr+2]=b;
						ar=r;
						ag=g;
						ab=b;
					}
					else
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-(ar>>1);
						ptr[adr+1]=g-(ag>>1);
						ptr[adr+2]=b-(ab>>1);
						ar=r;
						ag=g;
						ab=b;
					}
				}
				else
				{
					pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
					pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
					pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;

					if (x==0)
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-(pr>>1);
						ptr[adr+1]=g-(pg>>1);
						ptr[adr+2]=b-(pb>>1);
						ar=r;
						ag=g;
						ab=b;
					}
					else
					{
						r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
						g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
						b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
						ptr[adr+0]=r-((ar+pr)>>1);
						ptr[adr+1]=g-((ag+pg)>>1);
						ptr[adr+2]=b-((ab+pb)>>1);
						ar=r;
						ag=g;
						ab=b;
					}
				}
				adr+=3;
			}
		}
	}

	HDR.C_Method=0;
	HDR.Filter=0;
	HDR.Interlace=0;
	HDR.Bit_Depth=8;

	HDR.ColorType=2;

	x=ib(im->X);
	y=ib(im->Y);
	memcpy(HDR.LEN_X,&x,4);
	memcpy(HDR.LEN_Y,&y,4);

	uLongf sz=(int)(size*KOEF_COMPRESSION);
	char *zlib=(char*) malloc(sz);
    
	compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

	int p=0;

#define WRITE_MEM_PNG(data,len) { memcpy(&dest[p],data,len);p+=len; }

	WRITE_MEM_PNG(&id,8);
	n=ib(13);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sHDR,4);
	WRITE_MEM_PNG(&HDR,13);
	zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
	WRITE_MEM_PNG(&zero,4);
	//IDAT
	n=ib(sz);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sDAT,4);
	WRITE_MEM_PNG(zlib,sz);
	zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
	WRITE_MEM_PNG(&zero,4);
	//IEND
	n=0;
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sEND,4);
	zero=ib(crc(sEND,NULL,0));
	WRITE_MEM_PNG(&zero,4);

	*size_dest=p;

	free(zlib);
	free(ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void COMPRESS_PNG_ALPHA(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest)
{
	struct HEADER HDR;
	unsigned int adr,size;
	char * ptr;
	unsigned char id[8]={137,80,78,71,13,10,26,10};
	char *sHDR="IHDR";
	char *sDAT="IDAT";
	char *sEND="IEND";
	int x,y;
	int r,g,b,a;
    int ar,ag,ab,aa;
    int pr,pg,pb,pa;
	unsigned int n;
	unsigned int zero=0;

	size=(im->X*im->Y*4) + im->Y;

	ptr=(char*) malloc(size);

	if (filter==FILTER_NONE)
	{
		adr=0;
		for (y=0;y<(int) im->Y;y++)
		{
			ptr[adr]=0;
			adr++;

			for (x=0;x<(int) im->X;x++)
			{
				r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
				g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
				b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
				a=im->ptrImageDATAS[4*(y*im->X + x) + 3]&255;
				ptr[adr+0]=r;
				ptr[adr+1]=g;
				ptr[adr+2]=b;
				ptr[adr+3]=a;
				adr+=4;
			}
		}
	}
    
    if (filter==FILTER_SUB)
    {
        adr=0;
        for (y=0;y<(int) im->Y;y++)
        {
            ptr[adr]=1;
            adr++;

            for (x=0;x<(int) im->X;x++)
            {
                if (x==0)
                {
                    r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
                    g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
                    b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
                    a=im->ptrImageDATAS[4*(y*im->X + x) + 3]&255;
                    ptr[adr+0]=r;
                    ptr[adr+1]=g;
                    ptr[adr+2]=b;
                    ptr[adr+3]=a;
                    ar=r;
                    ag=g;
                    ab=b;
                    aa=a;
                }
                else
                {
                    r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
                    g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
                    b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
                    a=im->ptrImageDATAS[4*(y*im->X + x) + 3]&255;
                    ptr[adr+0]=r-ar;
                    ptr[adr+1]=g-ag;
                    ptr[adr+2]=b-ab;
                    ptr[adr+3]=a-aa;
                    ar=r;
                    ag=g;
                    ab=b;
                    aa=a;
                }
                adr+=4;
            }
        }
    }

    
    if (filter==FILTER_UP)
    {
        //up
        adr=0;
        for (y=0;y<(int) im->Y;y++)
        {
            ptr[adr++]=2;

            for (x=0;x<(int) im->X;x++)
            {
                if (y==0)
                {
                    r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
                    g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
                    b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
                    a=im->ptrImageDATAS[4*(y*im->X + x) + 3]&255;
                    ptr[adr+0]=r;
                    ptr[adr+1]=g;
                    ptr[adr+2]=b;
                    ptr[adr+3]=a;
                }
                else
                {
                    pr=im->ptrImageDATAS[4*((y-1)*im->X + x) + 0]&255;
                    pg=im->ptrImageDATAS[4*((y-1)*im->X + x) + 1]&255;
                    pb=im->ptrImageDATAS[4*((y-1)*im->X + x) + 2]&255;
                    pa=im->ptrImageDATAS[4*((y-1)*im->X + x) + 3]&255;
                    r=im->ptrImageDATAS[4*(y*im->X + x) + 0]&255;
                    g=im->ptrImageDATAS[4*(y*im->X + x) + 1]&255;
                    b=im->ptrImageDATAS[4*(y*im->X + x) + 2]&255;
                    a=im->ptrImageDATAS[4*(y*im->X + x) + 3]&255;
                    ptr[adr+0]=r-pr;
                    ptr[adr+1]=g-pg;
                    ptr[adr+2]=b-pb;
                    ptr[adr+3]=a-pa;
                }
                adr+=4;
            }
        }
    }


	HDR.C_Method=0;
	HDR.Filter=0;
	HDR.Interlace=0;
	HDR.Bit_Depth=8;

	HDR.ColorType=6;

	x=ib(im->X);
	y=ib(im->Y);
	memcpy(HDR.LEN_X,&x,4);
	memcpy(HDR.LEN_Y,&y,4);

	uLongf sz=(int)(size*KOEF_COMPRESSION);
	char *zlib=(char*) malloc(sz);
	compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

	int p=0;

#define WRITE_MEM_PNG(data,len) { memcpy(&dest[p],data,len);p+=len; }

	WRITE_MEM_PNG(&id,8);
	n=ib(13);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sHDR,4);
	WRITE_MEM_PNG(&HDR,13);
	zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
	WRITE_MEM_PNG(&zero,4);
	//IDAT
	n=ib(sz);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sDAT,4);
	WRITE_MEM_PNG(zlib,sz);
	zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
	WRITE_MEM_PNG(&zero,4);
	//IEND
	n=0;
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sEND,4);
	zero=ib(crc(sEND,NULL,0));
	WRITE_MEM_PNG(&zero,4);

	*size_dest=p;
	free(zlib);
	free(ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void COMPRESS_PNG_GRAYSCALE(struct IMAGE_DATAS * im,int filter,char * dest,int *size_dest)
{
	struct HEADER HDR;
	unsigned int adr,size;
	char * ptr;
	unsigned char id[8]={137,80,78,71,13,10,26,10};
	char *sHDR="IHDR";
	char *sDAT="IDAT";
	char *sEND="IEND";
	int x,y;
	int r;
	unsigned int n;
	unsigned int zero=0;

	size=(im->X*im->Y) + im->Y;
	ptr=(char*) malloc(size);
	adr=0;
	for (y=0;y<(int) im->Y;y++)
	{
		ptr[adr]=0;
		adr++;
		for (x=0;x<(int) im->X;x++)
		{
			r=im->ptrImageDATAS[y*im->X + x]&255;
			ptr[adr++]=r;
		}
	}
	HDR.C_Method=0;
	HDR.Filter=0;
	HDR.Interlace=0;
	HDR.Bit_Depth=8;
	HDR.ColorType=0;
	x=ib(im->X);
	y=ib(im->Y);
	memcpy(HDR.LEN_X,&x,4);
	memcpy(HDR.LEN_Y,&y,4);

	uLongf sz=(int)(size*KOEF_COMPRESSION);
	char *zlib=(char*) malloc(sz);
	compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,LZ77HUFFMAN);

	int p=0;

#define WRITE_MEM_PNG(data,len) { memcpy(&dest[p],data,len);p+=len; }

	WRITE_MEM_PNG(&id,8);
	n=ib(13);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sHDR,4);
	WRITE_MEM_PNG(&HDR,13);
	zero=ib(crc(sHDR,(unsigned char*)&HDR,13));
	WRITE_MEM_PNG(&zero,4);
	//IDAT
	n=ib(sz);
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sDAT,4);
	WRITE_MEM_PNG(zlib,sz);
	zero=ib(crc(sDAT,(unsigned char*) zlib,sz));
	WRITE_MEM_PNG(&zero,4);
	//IEND
	n=0;
	WRITE_MEM_PNG(&n,4);
	WRITE_MEM_PNG(sEND,4);
	zero=ib(crc(sEND,NULL,0));
	WRITE_MEM_PNG(&zero,4);

	*size_dest=p;
	free(zlib);
	free(ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BILINEAR(struct IMAGE_DATAS *im,int xx,int yy,int &r,int &g,int &b,int &a)
{
    int r1,r2,r3,r4;
    int g1,g2,g3,g4;
    int b1,b2,b3,b4;
    int a1,a2,a3,a4;
	unsigned char *raw=(unsigned char *)im->ptrImageDATAS;

	int adr0=yy*im->X;
	int adr1=(1+yy)*im->X;
            
    r1=raw[((xx+0+adr0)<<2)+0];
    if (yy<(int)(im->Y-1)) r2=raw[((xx+0+adr1)<<2)+0];
    else r2=r1;
            
    if (xx<(int)(im->X-1))
    {
        r3=raw[((xx+1+adr0)<<2)+0];
        if (yy<(int)(im->Y-1)) r4=raw[((xx+1+adr1)<<2)+0];
        else r4=r3;
    }
    else { r3=r1; r4=r2; }
            
    g1=raw[((xx+0+adr0)<<2)+1];
    if (yy<(int)(im->Y-1)) g2=raw[((xx+0+adr1)<<2)+1];
    else g2=g1;
            
    if (xx<(int)(im->X-1))
    {
        g3=raw[((xx+1+adr0)<<2)+1];
        if (yy<(int)(im->Y-1)) g4=raw[((xx+1+adr1)<<2)+1];
        else g4=g3;
    }
    else { g3=g1; g4=g2; }
            
    b1=raw[((xx+0+adr0)<<2)+2];
    if (yy<(int)(im->Y-1)) b2=raw[((xx+0+adr1)<<2)+2];
    else b2=b1;
            
    if (xx<(int)(im->X-1))
    {
        b3=raw[((xx+1+adr0)<<2)+2];
        if (yy<(int)(im->Y-1)) b4=raw[((xx+1+adr1)<<2)+2];
        else b4=b3;
    }
    else { b3=b1; b4=b2; }
            
    a1=raw[((xx+0+adr0)<<2)+3];
    if (yy<(int)(im->Y-1)) a2=raw[((xx+0+adr1)<<2)+3];
    else a2=a1;
            
    if (xx<(int)(im->X-1))
    {
        a3=raw[((xx+1+adr0)<<2)+3];
        if (yy<(int)(im->Y-1)) a4=raw[((xx+1+adr1)<<2)+3];
        else a4=a3;
    }
    else { a3=a1; a4=a2; }
            
    r=(r1+r2+r3+r4)/4;
    g=(g1+g2+g3+g4)/4;
    b=(b1+b2+b3+b4)/4;
    a=(a1+a2+a3+a4)/4;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
