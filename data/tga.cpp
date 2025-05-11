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
//	@file tga.cpp
//	@date 2004 update 2013
////////////////////////////////////////////////////////////////////////

#include "../base/params.h"

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

#if !defined(WIN32)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#define ALPHA1 0
#define ALPHA0 1
#define ALPHAINT 2
#define ALPHATRANSPARENT0 3
#define ALPHATRANSPARENT1 4
#define ALPHATRANSPARENT2 5
#define ALPHACHOISI 6
#define ALPHACOLORKEY 7


struct IMAGE_DATAS
{
	char * ptrImageDATAS;	// ptr
	unsigned int X,Y;		// dimension
	int	nTexture;			// gestion dynamique...
	int Alpha;				// 0-1
	int Grayscale;
};

int TGA_Opacity=255;
int TGA_r=0;
int TGA_g=0;
int TGA_b=0;

enum
{
	TYPE_NATIF	=	0,
	TYPE_OTHER	=	1
};

int TGA_TYPE_READWRITE=TYPE_NATIF;

void TGA_SetNativeReadWrite()
{
	TGA_TYPE_READWRITE=TYPE_NATIF;
}

void TGA_SetNonNativeReadWrite()
{
	TGA_TYPE_READWRITE=TYPE_OTHER;
}


void TGA_StoreInt(int n,FILE *f)
{
	unsigned char temp[4];
	if (TGA_TYPE_READWRITE!=TYPE_NATIF)
	{
		memcpy(temp,&n,4);
		fwrite(&temp[3],1,1,f);
		fwrite(&temp[2],1,1,f);
		fwrite(&temp[1],1,1,f);
		fwrite(&temp[0],1,1,f);
	}
	else
	{
		fwrite(&n,4,1,f);
	}
}

void TGA_StoreWord(short int w,FILE *f)
{
	unsigned char temp[2];
	if (TGA_TYPE_READWRITE!=TYPE_NATIF)
	{
		memcpy(temp,&w,2);
		fwrite(&temp[1],1,1,f);
		fwrite(&temp[0],1,1,f);
	}
	else
	{
		fwrite(&w,2,1,f);
	}
}

int TGA_GetInt(FILE *f)
{
	unsigned char temp[4];
	int n;
	if (TGA_TYPE_READWRITE!=TYPE_NATIF)
	{
		fread(&temp[3],1,1,f);
		fread(&temp[2],1,1,f);
		fread(&temp[1],1,1,f);
		fread(&temp[0],1,1,f);

		memcpy(&n,temp,4);
	}
	else
	{
		fread(&n,4,1,f);
	}
	return n;
}


int TGA_GetWord(FILE *f)
{
	unsigned char temp[2];
	short int n;
	if (TGA_TYPE_READWRITE!=TYPE_NATIF)
	{
		fread(&temp[1],1,1,f);
		fread(&temp[0],1,1,f);

		memcpy(&n,temp,2);
	}
	else
	{
		fread(&n,2,1,f);
	}

	return n;
}

void SAVE_TGA(char * name,struct IMAGE_DATAS *im)
{
	FILE * f;
	unsigned char * bits;
	unsigned char byteskip;
	short int shortskip;
	unsigned char imagetype;
	int colormode;
	unsigned char bitdepth;
	short int w,h;
	int n1,n2;

	imagetype=2;	
	byteskip=0;
	shortskip=0;
	w=im->X;
	h=im->Y;

	if (im->Alpha)
	{
		bits=(unsigned char *) malloc(w*h*4);
		bitdepth=32;
		colormode=4;

		unsigned int adr=0;
	    for (n2=0;n2<h;n2++)
		{
		    int adr2=(h-1-n2)*w*4;
		    for (n1=0;n1<w*4;n1+=4)
			{
				bits[adr+0]=im->ptrImageDATAS[n1+adr2+2]&255;
				bits[adr+1]=im->ptrImageDATAS[n1+adr2+1]&255;
				bits[adr+2]=im->ptrImageDATAS[n1+adr2+0]&255;
				bits[adr+3]=im->ptrImageDATAS[n1+adr2+3]&255;
				adr+=4;
			}
		}
	}
	else
	{
		bits=(unsigned char *) malloc(w*h*3);
		bitdepth=24;
		colormode=3;

        unsigned int adr=0;
		for (n2=0;n2<h;n2++)
		{
			int adr2=(h-1-n2)*w*4;
			for (n1=0;n1<w*4;n1+=4)
			{
				bits[adr+0]=im->ptrImageDATAS[n1+adr2+2]&255;
				bits[adr+1]=im->ptrImageDATAS[n1+adr2+1]&255;
				bits[adr+2]=im->ptrImageDATAS[n1+adr2+0]&255;
				adr+=3;
			}
		}
	}

	f=fopen(name,"wb");
	fwrite(&byteskip,1,1,f);
	fwrite(&byteskip,1,1,f);
	fwrite(&imagetype,1,1,f);
	fwrite(&shortskip,2,1,f);
	fwrite(&shortskip,2,1,f);
	fwrite(&byteskip,1,1,f);
	fwrite(&shortskip,2,1,f);
	fwrite(&shortskip,2,1,f);
	TGA_StoreWord(w,f);
	TGA_StoreWord(h,f);
	fwrite(&bitdepth,1,1,f);
	byteskip=0x10;
	fwrite(&byteskip,1,1,f);
	fwrite(bits,1,w*h*colormode,f);
	fclose(f);

	free(bits);
}


/////////////////////////////////////////////////////////////////////////////////////

#include <memory.h>
#define IMG_OK 0x1
#define IMG_ERR_NO_FILE 0x2
#define IMG_ERR_MEM_FAIL 0x4
#define IMG_ERR_BAD_FORMAT 0x8
#define IMG_ERR_UNSUPPORTED 0x40

class TGAImg
{
 public:
  TGAImg();
  ~TGAImg();
  int Load(char* szFilename);
  int LoadFromMem(char* buf,int size);
  int GetBPP();
  int GetWidth();
  int GetHeight();
  unsigned char* GetImg();       // Return a pointer to image data
  unsigned char* GetPalette();   // Return a pointer to VGA palette
 
  short int iWidth,iHeight,iBPP;
  unsigned long lImageSize;
  char bEnc;
  unsigned char *pImage, *pPalette, *pData;
  
  // Internal workers
  int ReadHeader();
  int LoadRawData();
  int LoadTgaRLEData();
  int LoadTgaPalette();
  void BGRtoRGB();
  void FlipImg();
};

TGAImg::TGAImg()
{ 
 pImage=pPalette=pData=NULL;
 iWidth=iHeight=iBPP=bEnc=0;
 lImageSize=0;
}

TGAImg::~TGAImg()
{
 if(pImage)
  {
   delete [] pImage;
   pImage=NULL;
  }
 if(pPalette)
  {
   delete [] pPalette;
   pPalette=NULL;
  }
 if(pData)
  {
   delete [] pData;
   pData=NULL;
  }
}

int TGAImg::Load(char* szFilename)
{
// using namespace std;
 FILE * fIn;
 unsigned long ulSize;
 int iRet;
 // Clear out any existing image and palette
  if(pImage)
   {
    delete [] pImage;
    pImage=NULL;
   }
  if(pPalette)
   {
    delete [] pPalette;
    pPalette=NULL;
   }
 // Open the specified file
 fIn=fopen(szFilename,"rb");
   
  if(fIn==NULL)
   return IMG_ERR_NO_FILE;
 // Get file size

  fseek(fIn,0,SEEK_END);
  ulSize=ftell(fIn);
  fseek(fIn,0,SEEK_SET);
 // Allocate some space
 // Check and clear pDat, just in case
  if(pData)
   delete [] pData; 
 pData=new unsigned char[ulSize];
  if(pData==NULL)
   {
    fclose(fIn);
    return IMG_ERR_MEM_FAIL;
   }
 // Read the file into memory
  fread((char*)pData,ulSize,1,fIn);
  fclose(fIn);
  // Process the header
 iRet=ReadHeader();
  if(iRet!=IMG_OK)
   return iRet;
  switch(bEnc)
   {
    case 1: // Raw Indexed
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0]+768)>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=1)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      // Load palette
      iRet=LoadTgaPalette();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }
    case 2: // Raw RGB
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0])>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      BGRtoRGB(); // Convert to RGB
      break;
     }
    case 3: // Raw RGB
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0])>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }
    case 9: // RLE Indexed
     {
      // Double check image type field
       if(pData[1]!=1)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadTgaRLEData();
       if(iRet!=IMG_OK)
        return iRet;
      // Load palette
      iRet=LoadTgaPalette();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }

    case 10: // RLE RGB
     {
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadTgaRLEData();
       if(iRet!=IMG_OK)
        return iRet;
      BGRtoRGB(); // Convert to RGB
      break;
     }
    default:
     return IMG_ERR_UNSUPPORTED;
   }
 // Check flip bit
  if((pData[17] & 0x10)) 
    FlipImg();
 // Release file memory
 delete [] pData;
 pData=NULL;
 return IMG_OK;
}


int TGAImg::LoadFromMem(char* buf,int size)
{
// using namespace std;
 unsigned long ulSize;
 int iRet;
 // Clear out any existing image and palette
  if(pImage)
   {
    delete [] pImage;
    pImage=NULL;
   }
  if(pPalette)
   {
    delete [] pPalette;
    pPalette=NULL;
   }
 // Load from mem
  ulSize=size;
 // Allocate some space
 // Check and clear pDat, just in case
  if(pData)
   delete [] pData; 
 pData=new unsigned char[ulSize];
  if(pData==NULL)
   {
    return IMG_ERR_MEM_FAIL;
   }
 // Read the file into memory
  memcpy((char*)pData,buf,ulSize);
  
  // Process the header
 iRet=ReadHeader();
  if(iRet!=IMG_OK)
   return iRet;
  switch(bEnc)
   {
    case 1: // Raw Indexed
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0]+768)>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=1)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      // Load palette
      iRet=LoadTgaPalette();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }
    case 2: // Raw RGB
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0])>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      BGRtoRGB(); // Convert to RGB
      break;
     }
    case 3: // Raw RGB
     {
      // Check filesize against header values
       if((lImageSize+18+pData[0])>ulSize)
        return IMG_ERR_BAD_FORMAT;
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadRawData();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }
    case 9: // RLE Indexed
     {
      // Double check image type field
       if(pData[1]!=1)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadTgaRLEData();
       if(iRet!=IMG_OK)
        return iRet;
      // Load palette
      iRet=LoadTgaPalette();
       if(iRet!=IMG_OK)
        return iRet;
      break;
     }

    case 10: // RLE RGB
     {
      // Double check image type field
       if(pData[1]!=0)
        return IMG_ERR_BAD_FORMAT;
      // Load image data
      iRet=LoadTgaRLEData();
       if(iRet!=IMG_OK)
        return iRet;
      BGRtoRGB(); // Convert to RGB
      break;
     }
    default:
     return IMG_ERR_UNSUPPORTED;
   }
 // Check flip bit
  if((pData[17] & 0x10)) 
    FlipImg();
 // Release file memory
 delete [] pData;
 pData=NULL;
 return IMG_OK;
}

int TGAImg::ReadHeader() // Examine the header and populate our class attributes
{
 short ColMapStart,ColMapLen;
 short x1,y1,x2,y2;
  if(pData==NULL)
   return IMG_ERR_NO_FILE;
  if(pData[1]>1)    // 0 (RGB) and 1 (Indexed) are the only types we know about
   return IMG_ERR_UNSUPPORTED;
  bEnc=pData[2];     // Encoding flag  1 = Raw indexed image
                     //                2 = Raw RGB
                     //                3 = Raw greyscale
                     //                9 = RLE indexed
                     //               10 = RLE RGB
                     //               11 = RLE greyscale
                     //               32 & 33 Other compression, indexed
   if(bEnc>11)       // We don't want 32 or 33
    return IMG_ERR_UNSUPPORTED;

 // Get palette info
 memcpy(&ColMapStart,&pData[3],2);
 memcpy(&ColMapLen,&pData[5],2);
 // Reject indexed images if not a VGA palette (256 entries with 24 bits per entry)
  if(pData[1]==1) // Indexed
   {
    if(ColMapStart!=0 || ColMapLen!=256 || pData[7]!=24)
     return IMG_ERR_UNSUPPORTED;
   }
 // Get image window and produce width & height values
 memcpy(&x1,&pData[8],2);
 memcpy(&y1,&pData[10],2);
 memcpy(&x2,&pData[12],2);
 memcpy(&y2,&pData[14],2);
 iWidth=(x2-x1);
 iHeight=(y2-y1);
  if(iWidth<1 || iHeight<1)
   return IMG_ERR_BAD_FORMAT;
 // Bits per Pixel
 iBPP=pData[16];
 // Check flip / interleave byte
  if(pData[17]>32) // Interleaved data
   return IMG_ERR_UNSUPPORTED;
 // Calculate image size
 lImageSize=(iWidth * iHeight * (iBPP/8));
 return IMG_OK;
}

int TGAImg::LoadRawData() // Load uncompressed image data
{
 short iOffset;

  if(pImage) // Clear old data if present
   delete [] pImage;
 pImage=new unsigned char[lImageSize];
  if(pImage==NULL)
   return IMG_ERR_MEM_FAIL;
 iOffset=pData[0]+18; // Add header to ident field size
  if(pData[1]==1) // Indexed images
   iOffset+=768;  // Add palette offset
  memcpy(pImage,&pData[iOffset],lImageSize);
 return IMG_OK;
}

int TGAImg::LoadTgaRLEData() // Load RLE compressed image data
{
 short iOffset,iPixelSize;
 unsigned char *pCur;
 unsigned long Index=0;
 unsigned char bLength,bLoop;
 // Calculate offset to image data
 iOffset=pData[0]+18;
 // Add palette offset for indexed images
  if(pData[1]==1)
   iOffset+=768; 
 // Get pixel size in bytes
 iPixelSize=iBPP/8;
 // Set our pointer to the beginning of the image data
 pCur=&pData[iOffset];
 // Allocate space for the image data
  if(pImage!=NULL)
   delete [] pImage;
 pImage=new unsigned char[lImageSize];
  if(pImage==NULL)
   return IMG_ERR_MEM_FAIL;
 // Decode
  while(Index<lImageSize) 
   {
     if(*pCur & 0x80) // Run length chunk (High bit = 1)
      {
       bLength=*pCur-127; // Get run length
       pCur++;            // Move to pixel data  
       // Repeat the next pixel bLength times
        for(bLoop=0;bLoop!=bLength;++bLoop,Index+=iPixelSize)
         memcpy(&pImage[Index],pCur,iPixelSize);
 
       pCur+=iPixelSize; // Move to the next descriptor chunk
      }
     else // Raw chunk
      {
       bLength=*pCur+1; // Get run length
       pCur++;          // Move to pixel data
       // Write the next bLength pixels directly
        for(bLoop=0;bLoop!=bLength;++bLoop,Index+=iPixelSize,pCur+=iPixelSize)
         memcpy(&pImage[Index],pCur,iPixelSize);
      }
   }

 return IMG_OK;
}

int TGAImg::LoadTgaPalette() // Load a 256 color palette
{
 unsigned char bTemp;
 short iIndex,iPalPtr;
 
  // Delete old palette if present
  if(pPalette)
   {
    delete [] pPalette;
    pPalette=NULL;
   }
 // Create space for new palette
 pPalette=new unsigned char[768];
  if(pPalette==NULL)
   return IMG_ERR_MEM_FAIL;
 // VGA palette is the 768 bytes following the header
 memcpy(pPalette,&pData[pData[0]+18],768);
 // Palette entries are BGR ordered so we have to convert to RGB
  for(iIndex=0,iPalPtr=0;iIndex!=256;++iIndex,iPalPtr+=3)
   {
    bTemp=pPalette[iPalPtr];               // Get Blue value
    pPalette[iPalPtr]=pPalette[iPalPtr+2]; // Copy Red to Blue
    pPalette[iPalPtr+2]=bTemp;             // Replace Blue at the end
   }
 return IMG_OK;
}

void TGAImg::BGRtoRGB() // Convert BGR to RGB (or back again)
{
 unsigned long Index,nPixels;
 unsigned char *bCur;
 unsigned char bTemp;
 short iPixelSize;
 // Set ptr to start of image
 bCur=pImage;
 // Calc number of pixels
 nPixels=iWidth*iHeight;
 // Get pixel size in bytes
 iPixelSize=iBPP/8;
  for(Index=0;Index!=nPixels;Index++)  // For each pixel
   {
    bTemp=*bCur;      // Get Blue value
    *bCur=*(bCur+2);  // Swap red value into first position
    *(bCur+2)=bTemp;  // Write back blue to last position
    bCur+=iPixelSize; // Jump to next pixel
   }
}

void TGAImg::FlipImg() // Flips the image vertically (Why store images upside down?)
{
 unsigned char bTemp;
 unsigned char *pLine1, *pLine2;
 int iLineLen,iIndex;

 iLineLen=iWidth*(iBPP/8);
 pLine1=pImage;
 pLine2=&pImage[iLineLen * (iHeight - 1)];
  for( ;pLine1<pLine2;pLine2-=(iLineLen*2))
   {
    for(iIndex=0;iIndex!=iLineLen;pLine1++,pLine2++,iIndex++)
     {
      bTemp=*pLine1;
      *pLine1=*pLine2;
      *pLine2=bTemp;       
     }
   } 
}

int TGAImg::GetBPP()
{
 return iBPP;
}

int TGAImg::GetWidth()
{
 return iWidth;
}

int TGAImg::GetHeight()
{
 return iHeight;
}

unsigned char* TGAImg::GetImg()
{
 return pImage;
}

unsigned char* TGAImg::GetPalette()
{
 return pPalette;
}

void READ_TGA(char * name,struct IMAGE_DATAS *im,unsigned char alpha)
{
	short int w,h;
	int x,y;
	int r,g,b,a,c;

	im->ptrImageDATAS=NULL;
	im->X=0;
	im->Y=0;

	TGAImg tga;

	tga.Load(name);
    tga.FlipImg();

	w=tga.GetWidth();
	h=tga.GetHeight();

	unsigned char * pal=tga.GetPalette();
	unsigned char * raw=tga.GetImg();

	im->Grayscale=0;

	if (tga.bEnc==3) { im->ptrImageDATAS=(char*) malloc(w*h*4);im->Grayscale=0;}
	else im->ptrImageDATAS=(char*) malloc(w*h*4);

	im->X=w;
	im->Y=h;
	im->Alpha=0;

	switch (tga.GetBPP())
	{
	case 8:
		if (tga.bEnc==3)
		{
			for (x=0;x<w;x++)
				for (y=0;y<h;y++)
				{
					c=raw[x+w*y];
					im->ptrImageDATAS[(x+w*y)]=c;
				}
		}
		else
		{
			for (x=0;x<w;x++)
				for (y=0;y<h;y++)
				{
					c=raw[x+w*y];
					im->ptrImageDATAS[4*(x+w*y)+0]=pal[3*c+0];
					im->ptrImageDATAS[4*(x+w*y)+1]=pal[3*c+1];
					im->ptrImageDATAS[4*(x+w*y)+2]=pal[3*c+2];
					im->ptrImageDATAS[4*(x+w*y)+3]=(char) 255;
				}
		}
		break;
	case 24:
		for (x=0;x<w;x++)
			for (y=0;y<h;y++)
			{
				r=raw[3*(x+w*y)+0];
				g=raw[3*(x+w*y)+1];
				b=raw[3*(x+w*y)+2];
				im->ptrImageDATAS[4*(x+w*y)+0]=r;
				im->ptrImageDATAS[4*(x+w*y)+1]=g;
				im->ptrImageDATAS[4*(x+w*y)+2]=b;
				im->ptrImageDATAS[4*(x+w*y)+3]=(char) 255;
			}
		break;
	case 32:
		for (x=0;x<w;x++)
			for (y=0;y<h;y++)
			{
				r=raw[4*(x+w*y)+0];
				g=raw[4*(x+w*y)+1];
				b=raw[4*(x+w*y)+2];
				a=raw[4*(x+w*y)+3];
				im->ptrImageDATAS[4*(x+w*y)+0]=r;
				im->ptrImageDATAS[4*(x+w*y)+1]=g;
				im->ptrImageDATAS[4*(x+w*y)+2]=b;
				im->ptrImageDATAS[4*(x+w*y)+3]=a;
			}
		im->Alpha=1;
		break;
	};
	
}

void READ_TGA_FROM_MEM(char * buf,int size,struct IMAGE_DATAS *im,unsigned char alpha)
{
	short int w,h;
	int x,y;
	int r,g,b,a,c;

	im->ptrImageDATAS=NULL;
	im->X=0;
	im->Y=0;

	TGAImg tga;

	tga.LoadFromMem(buf,size);
    tga.FlipImg();

	w=tga.GetWidth();
	h=tga.GetHeight();

	unsigned char * pal=tga.GetPalette();
	unsigned char * raw=tga.GetImg();

	im->Grayscale=0;

	if (tga.bEnc==3) { im->ptrImageDATAS=(char*) malloc(w*h*4);im->Grayscale=0;}
	else im->ptrImageDATAS=(char*) malloc(w*h*4);

	im->X=w;
	im->Y=h;
	im->Alpha=0;

	switch (tga.GetBPP())
	{
	case 8:
		if (tga.bEnc==3)
		{
			for (x=0;x<w;x++)
				for (y=0;y<h;y++)
				{
					c=raw[x+w*y];
					im->ptrImageDATAS[(x+w*y)]=c;
				}
		}
		else
		{
			for (x=0;x<w;x++)
				for (y=0;y<h;y++)
				{
					c=raw[x+w*y];
					im->ptrImageDATAS[4*(x+w*y)+0]=pal[3*c+0];
					im->ptrImageDATAS[4*(x+w*y)+1]=pal[3*c+1];
					im->ptrImageDATAS[4*(x+w*y)+2]=pal[3*c+2];
					im->ptrImageDATAS[4*(x+w*y)+3]=(char) 255;
				}
		}
		break;
	case 24:
		for (x=0;x<w;x++)
			for (y=0;y<h;y++)
			{
				r=raw[3*(x+w*y)+0];
				g=raw[3*(x+w*y)+1];
				b=raw[3*(x+w*y)+2];
				im->ptrImageDATAS[4*(x+w*y)+0]=r;
				im->ptrImageDATAS[4*(x+w*y)+1]=g;
				im->ptrImageDATAS[4*(x+w*y)+2]=b;
				im->ptrImageDATAS[4*(x+w*y)+3]=(char) 255;
			}
		break;
	case 32:
		for (x=0;x<w;x++)
			for (y=0;y<h;y++)
			{
				r=raw[4*(x+w*y)+0];
				g=raw[4*(x+w*y)+1];
				b=raw[4*(x+w*y)+2];
				a=raw[4*(x+w*y)+3];
				im->ptrImageDATAS[4*(x+w*y)+0]=r;
				im->ptrImageDATAS[4*(x+w*y)+1]=g;
				im->ptrImageDATAS[4*(x+w*y)+2]=b;
				im->ptrImageDATAS[4*(x+w*y)+3]=a;
			}
		im->Alpha=1;
		break;
	};
	
}

#define MINIMUM 0
#define MAXIMUM 1

void CNV_SQUARED(struct IMAGE_DATAS *im,int flags)
{
	char *ptr;
	int w,h;
	int x,y;
	int xp,yp;
	int r,g,b,a;

	if (im->X!=im->Y)
	{
		ptr=im->ptrImageDATAS;

		if (flags==MINIMUM)
		{
			if (im->X<im->Y) w=h=im->X;
			else w=h=im->Y;
		}
		else
		{
			if (im->X>im->Y) w=h=im->X;
			else w=h=im->Y;
		}

		im->ptrImageDATAS=(char*) malloc(w*h*4);

		for (x=0;x<w;x++)
			for (y=0;y<h;y++)
			{
				
				xp=(im->X*x)/w;
				yp=(im->Y*y)/h;


				r=ptr[4*(xp+yp*im->X)+0]&255;
				g=ptr[4*(xp+yp*im->X)+1]&255;
				b=ptr[4*(xp+yp*im->X)+2]&255;
				a=ptr[4*(xp+yp*im->X)+3]&255;

				im->ptrImageDATAS[4*(x+y*w)+0]=r;
				im->ptrImageDATAS[4*(x+y*w)+1]=g;
				im->ptrImageDATAS[4*(x+y*w)+2]=b;
				im->ptrImageDATAS[4*(x+y*w)+3]=a;

			}

		free(ptr);

		im->X=w;
		im->Y=h;
	}
}


void CNV_COMPONENT(struct IMAGE_DATAS *im,int cp)
{
	char * ptr=im->ptrImageDATAS;
	int x,y;
	int r,g,b;

	im->ptrImageDATAS=(char*) malloc(im->X*im->Y);
	im->Grayscale=1;
	im->Alpha=0;

	if (cp==4)
	{
		for (x=0;x<(int) im->X;x++)
			for (y=0;y<(int) im->Y;y++)
			{
				r=ptr[4*(x+y*im->Y)+0]&255;
				g=ptr[4*(x+y*im->Y)+1]&255;
				b=ptr[4*(x+y*im->Y)+2]&255;
				im->ptrImageDATAS[x+y*im->X]=(30*r + 59*g + 11*b)/100;
			}

	}
	else
	{
		for (x=0;x<(int) im->X;x++)
			for (y=0;y<(int) im->Y;y++)
			{
				im->ptrImageDATAS[x+y*im->X]=ptr[4*(x+y*im->Y)+cp];
			}
	}
	free(ptr);
}
