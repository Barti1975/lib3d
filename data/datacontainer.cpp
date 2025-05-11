
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


#include "datacontainer.h"

#if !defined(WIN32)&&!defined(ANDROID)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
#include "file_io_zip.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBloc::store(const char * ptr_data,int size)
{
    memcpy(&data[ptr],ptr_data,size);
    ptr+=size;
}

int CBloc::fit(int size)
{
    if (ptr+size<MAX_BUF) return size;
    else return (MAX_BUF-ptr);
}

int CBloc::getLength()
{
    return ptr;
}

char * CBloc::getData()
{
    return data;
}

void CBloc::clean()
{
    free(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDataReader::setRaw()
{
    shrunk=false;
}

void CDataReader::setNormal()
{
    shrunk=true;
}

unsigned int CDataReader::getSize()
{
    return data_length;
}

char * CDataReader::getData()
{
    return data;
}

void CDataReader::clean()
{
    free(data);
    data=NULL;
    ptr=data_length=0;
}

bool CDataReader::eod()
{
    return (ptr>=data_length);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataReader::get(char *raw,int size)
{
    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }

    data=(char*) malloc(size);
    memcpy(data,raw,size);
    ptr=0;
    data_length=size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
void CDataReader::get(ZIPFILE *f,int size)
{
    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }

    data=(char*) malloc(size);
    ZIPfread(data,size,1,f);
    ptr=0;
    data_length=size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataReader::getLZ(ZIPFILE *f)
{
    struct Inflate nfo;
    unsigned int tmp;

    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }

#ifdef CLASSNED
	if (MyFILEBuffer)
	{
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo.C_Size=tmp;
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo.Size=tmp;
		if ((nfo.C_Size>0)&&(nfo.Size>0))
		{
            if (!shrunk)
            {
                data=(char*) malloc(nfo.Size);
                memcpy(data,MyFILEBuffer,nfo.Size);
                MyFILEBuffer+=nfo.C_Size;
            }
            else
            {
                nfo.BlockPtr=(char*) malloc(nfo.C_Size+16);
                nfo.BlockPtr[0]=120;
                nfo.BlockPtr[1]=-38;
                memcpy(&nfo.BlockPtr[2],MyFILEBuffer,nfo.C_Size);
                MyFILEBuffer+=nfo.C_Size;
                nfo.Type=INFLATE_NORMAL;
                data=DeflateDatasZLIB(&nfo);
                free(nfo.BlockPtr);
            }
		}
		data_length=nfo.Size;
	}
	else
	{
#endif
		ZIPfread(&tmp,4,1,f);
		nfo.C_Size=tmp;

		ZIPfread(&tmp,4,1,f);
		nfo.Size=tmp;

        if (!shrunk)
        {
            data=(char*) malloc(nfo.Size);
            ZIPfread(data,nfo.Size,1,f);
        }
        else
		if ((nfo.C_Size>0)&&(nfo.Size>0))
		{
			nfo.BlockPtr=(char*) malloc(nfo.C_Size+16);
            nfo.BlockPtr[0]=120;
            nfo.BlockPtr[1]=-38;
			ZIPfread(&nfo.BlockPtr[2],nfo.C_Size,1,f);
			nfo.Type=INFLATE_NORMAL;
			data=DeflateDatasZLIB(&nfo);
			free(nfo.BlockPtr);
		}
		data_length=nfo.Size;
#ifdef CLASSNED
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * CDataReader::getLZnfo(ZIPFILE *f)
{
    struct Inflate *nfo=(struct Inflate*)malloc(sizeof(struct Inflate));
    unsigned int tmp;

#ifdef CLASSNED
	if (MyFILEBuffer)
	{
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo->C_Size=tmp;
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo->Size=tmp;

        if ((nfo->C_Size>0)&&(nfo->Size>0))
		{
            nfo->BlockPtr=(char*) malloc(nfo->C_Size+16);
            nfo->BlockPtr[0]=120;
            nfo->BlockPtr[1]=-38;
            memcpy(&nfo->BlockPtr[2],MyFILEBuffer,nfo->C_Size);
            MyFILEBuffer+=nfo->C_Size;
            nfo->Type=INFLATE_NORMAL;
		}
	}
	else
	{
#endif
		ZIPfread(&tmp,4,1,f);
		nfo->C_Size=tmp;

		ZIPfread(&tmp,4,1,f);
		nfo->Size=tmp;

 		if ((nfo->C_Size>0)&&(nfo->Size>0))
		{
			nfo->BlockPtr=(char*) malloc(nfo->C_Size+16);
            nfo->BlockPtr[0]=120;
            nfo->BlockPtr[1]=-38;
			ZIPfread(&nfo->BlockPtr[2],nfo->C_Size,1,f);
			nfo->Type=INFLATE_NORMAL;
		}
#ifdef CLASSNED
	}
#endif

    return nfo;
}

#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataReader::get(FILE *f,int size)
{
    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }

    data=(char*) malloc(size);
    fread(data,size,1,f);
    ptr=0;
    data_length=size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDataReader::getLZ(FILE *f)
{
    struct Inflate nfo;
    unsigned int tmp;

    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }
#ifdef CLASSNED
	if (MyFILEBuffer)
	{
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo.C_Size=tmp;
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo.Size=tmp;
        
        if (!shrunk)
        {
            data=(char*) malloc(nfo.Size);
            memcpy(data,MyFILEBuffer,nfo.Size);
            MyFILEBuffer+=nfo.Size;
        }
        else
		if ((nfo.C_Size>0)&&(nfo.Size>0))
		{
			nfo.BlockPtr=(char*) malloc(nfo.C_Size+16);
            nfo.BlockPtr[0]=120;
            nfo.BlockPtr[1]=-38;
			memcpy(&nfo.BlockPtr[2],MyFILEBuffer,nfo.C_Size);
			MyFILEBuffer+=nfo.C_Size;
			nfo.Type=INFLATE_NORMAL;
			data=DeflateDatasZLIB(&nfo);
			free(nfo.BlockPtr);
		}
		data_length=nfo.Size;
	}
	else
	{
#endif
		fread(&tmp,4,1,f);
		nfo.C_Size=tmp;

		fread(&tmp,4,1,f);
		nfo.Size=tmp;

        if (!shrunk)
        {
            data=(char*) malloc(nfo.Size);
            fread(data,nfo.Size,1,f);
        }
        else
		if ((nfo.C_Size>0)&&(nfo.Size>0))
		{
			nfo.BlockPtr=(char*) malloc(nfo.C_Size+16);
            nfo.BlockPtr[0]=120;
            nfo.BlockPtr[1]=-38;
			fread(&nfo.BlockPtr[2],nfo.C_Size,1,f);
			nfo.Type=INFLATE_NORMAL;
			data=DeflateDatasZLIB(&nfo);
			free(nfo.BlockPtr);
		}
		data_length=nfo.Size;
#ifdef CLASSNED
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * CDataReader::getLZnfo(FILE *f)
{
    struct Inflate *nfo=(struct Inflate*)malloc(sizeof(struct Inflate));
    unsigned int tmp;

#ifdef CLASSNED
	if (MyFILEBuffer)
	{
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo->C_Size=tmp;
		memcpy(&tmp,MyFILEBuffer,4);
		MyFILEBuffer+=4;
		nfo->Size=tmp;
        
		if ((nfo->C_Size>0)&&(nfo->Size>0))
		{
			nfo->BlockPtr=(char*) malloc(nfo->C_Size+16);
            nfo->BlockPtr[0]=120;
            nfo->BlockPtr[1]=-38;
			memcpy(&nfo->BlockPtr[2],MyFILEBuffer,nfo->C_Size);
			MyFILEBuffer+=nfo->C_Size;
			nfo->Type=INFLATE_NORMAL;
		}
	}
	else
	{
#endif
		fread(&tmp,4,1,f);
		nfo->C_Size=tmp;

		fread(&tmp,4,1,f);
		nfo->Size=tmp;

		if ((nfo->C_Size>0)&&(nfo->Size>0))
		{
			nfo->BlockPtr=(char*) malloc(nfo->C_Size+16);
            nfo->BlockPtr[0]=120;
            nfo->BlockPtr[1]=-38;
			fread(&nfo->BlockPtr[2],nfo->C_Size,1,f);
			nfo->Type=INFLATE_NORMAL;
		}
#ifdef CLASSNED
	}
#endif

    return nfo;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CLASSNED
void CDataReader::getLZ(MyFILE *f)
{
    struct Inflate nfo;
    unsigned int tmp;

    if (data)
    {
        free(data);
        data=NULL;
        ptr=0;
    }

	Myfread(&tmp,4,1,f);
	nfo.C_Size=tmp;

	Myfread(&tmp,4,1,f);
	nfo.Size=tmp;

	if(!shrunk)
	{
		data=(char*)malloc(nfo.Size);
		Myfread(data,nfo.Size,1,f);
	}
	else
	if((nfo.C_Size>0) && (nfo.Size>0))
	{
		nfo.BlockPtr=(char*)malloc(nfo.C_Size + 16);
        nfo.BlockPtr[0]=120;
        nfo.BlockPtr[1]=-38;
		Myfread(&nfo.BlockPtr[2],nfo.C_Size,1,f);
		nfo.Type=INFLATE_NORMAL;
		data=DeflateDatasZLIB(&nfo);
		free(nfo.BlockPtr);
	}

	data_length=nfo.Size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * CDataReader::getLZnfo(MyFILE *f)
{
    struct Inflate *nfo=(struct Inflate*)malloc(sizeof(struct Inflate));
    unsigned int tmp;

	Myfread(&tmp,4,1,f);
	nfo->C_Size=tmp;

	Myfread(&tmp,4,1,f);
	nfo->Size=tmp;
	
	if ((nfo->C_Size>0)&&(nfo->Size>0))
	{
		nfo->BlockPtr=(char*) malloc(nfo->C_Size+16);
        nfo->BlockPtr[0]=120;
        nfo->BlockPtr[1]=-38;
		Myfread(&nfo->BlockPtr[2],nfo->C_Size,1,f);
		nfo->Type=INFLATE_NORMAL;
	}

    return nfo;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDataReader::getLZ(char * raw)
{
    struct Inflate nfo;

    if (data) { free(data); data=NULL; ptr=0; }

    nfo.C_Size=*(unsigned int*)&raw[0];
    nfo.Size=*(unsigned int*)&raw[4];
    
    if (!shrunk)
    {
        if ((nfo.C_Size>0)&&(nfo.Size>0))
        {
            data=(char*) malloc(nfo.Size);
            memcpy(data,&raw[8],nfo.Size);
        }
        data_length=nfo.Size;
        return (4+4+nfo.C_Size);
    }
    else
    {
        if ((nfo.C_Size>0)&&(nfo.Size>0))
        {
            nfo.BlockPtr=(char*) malloc(nfo.C_Size+16);
            nfo.BlockPtr[0]=120;
            nfo.BlockPtr[1]=-38;
            memcpy(&nfo.BlockPtr[2],&raw[8],nfo.C_Size);
            nfo.Type=INFLATE_NORMAL;
            data=DeflateDatasZLIB(&nfo);
            free(nfo.BlockPtr);
        }
        data_length=nfo.Size;
        
        return (4+4+nfo.C_Size);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDataReader::readInt()
{
    int len=sizeof(int);
    int n=*(int*) &data[ptr];
    ptr+=len;
    return n;
}

int CDataReader::readInt24()
{
    int len=3;
    int n=*(int*) &data[ptr];
    n=n&0x00FFFFFF;
    ptr+=len;
    return n;
}

unsigned int CDataReader::readUInt24()
{
    int len=3;
    unsigned int n=*(unsigned int*) &data[ptr];
    n=n&0x00FFFFFF;
    ptr+=len;
    return n;
}

unsigned int CDataReader::readUInt()
{
    int len=sizeof(unsigned int);
    unsigned int n=*(unsigned int*) &data[ptr];
    ptr+=len;
    return n;
}

int CDataReader::readChar()
{
    int len=1;
    int n=data[ptr]&255;
    ptr+=len;
    return n;
}

int CDataReader::readWord()
{
    int len=2;
    int n=*(unsigned short int*) &data[ptr];
    ptr+=len;
    return n;
}

float CDataReader::readFloat()
{
    int len=sizeof(float);
    float f=*((float*) (&data[ptr]));
    ptr+=len;
    return f;
}

double CDataReader::readDouble()
{
    int len=sizeof(double);
    double val=*((double*) (&data[ptr]));
    ptr+=len;
    return val;
}

unsigned long int CDataReader::readLong()
{
    int len=sizeof(unsigned long int);
    unsigned long int val=*((unsigned long int*) (&data[ptr]));
    ptr+=len;
    return val;
}

long int CDataReader::readLongInt()
{
    int len=sizeof(long int);
    long int val=*((long int*) (&data[ptr]));
    ptr+=len;
    return val;
}

char* CDataReader::readData(int len)
{
    char *dat=(char*) malloc(len);
    memcpy(dat,&data[ptr],len);
    ptr+=len;
    return dat;
}

void CDataReader::readData(void *dat,int len)
{
    memcpy((char*) dat,&data[ptr],len);
    ptr+=len;
}

CVector CDataReader::readVector()
{
    CVector v;
    v.x=readFloat();
    v.y=readFloat();
    v.z=readFloat();
    return v;
}

CQuaternion CDataReader::readQuaternion()
{
    CQuaternion v;
    v.n.x=readFloat();
    v.n.y=readFloat();
    v.n.z=readFloat();
    v.s=readFloat();
    return v;
}

CVector2 CDataReader::readMap()
{
    CVector2 v;
    v.x=readFloat();
    v.y=readFloat();
    return v;
}

CRGBA CDataReader::readRGB()
{
    CRGBA rgb;
    rgb.r=readFloat();
    rgb.g=readFloat();
    rgb.b=readFloat();
    rgb.a=1.0f;
    return rgb;
}

CRGBA CDataReader::readRGBA()
{
    CRGBA rgb;
    rgb.r=readFloat();
    rgb.g=readFloat();
    rgb.b=readFloat();
    rgb.a=readFloat();
    return rgb;
}

char * CDataReader::readString()
{
    int n;
    bool tmp;
    char strz[2048];
    char * res;
    n=0;
    tmp=false;
    while (!tmp)
    {
        strz[n]=data[ptr++];
        if (strz[n]==0) tmp=true;
        n++;
    }

    res=(char*) malloc(n);
    strcpy(res,strz);
    return res;

}

char * CDataReader::readString10()
{
    int n;
    bool tmp;
    char strz[2048];
    char * res;
    n=0;
    tmp=false;
    while (!tmp)
    {
        strz[n]=data[ptr++];
        if (strz[n]==0x0A) tmp=true;
        n++;
    }
    strz[n-1]='\0';

    res=(char*) malloc(n);
    strcpy(res,strz);
    return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDataContainer::clean()
{
    CBloc * bloc;
    bloc=blocks.GetFirst();
    while (bloc)
    {
        bloc->clean();
        bloc=blocks.GetNext();
    }
    blocks.Free();

    if (nfo)
    {
        free(nfo->BlockPtr);
        free(nfo);
    }

    if (result) free(result);
    result=NULL;
    nfo=NULL;

    blocks.InsertLast();
}

void CDataContainer::setRaw()
{
    shrunk=false;
}

void CDataContainer::setNormal()
{
    shrunk=true;
}

unsigned int CDataContainer::getLength()
{
    return ((blocks.Length()-1)*MAX_BUF + blocks.Last()->getLength());
}

char * CDataContainer::getData()
{
    CBloc * bloc;
    int n;

    if (result) free(result);
    result=(char*) malloc(getLength());
    n=0;
    bloc=blocks.GetFirst();
    while (bloc)
    {
        memcpy(&result[n*MAX_BUF],bloc->getData(),bloc->getLength());
        n++;
        bloc=blocks.GetNext();
    }

    return result;
}

unsigned int CDataContainer::getLZLength()
{
    if (!shrunk) return getLength();
    else
    {
        if (nfo) return (nfo->C_Size);
        else return 0xFFFFFFFF;
    }
}

char * CDataContainer::getLZData()
{
    if (!shrunk) return getData();
    else
    {
        if (nfo) return (nfo->BlockPtr);
        else return NULL;
    }
}

void CDataContainer::compressFast()
{
    if (shrunk) nfo=InflateQuick((char*)getData(),getLength());
}


void CDataContainer::compress()
{
    if (shrunk) nfo=InflateNormal((char*)getData(),getLength());
}

void CDataContainer::storeData(const char *data,int len)
{
	if (len<MAX_BUF)
	{
		CBloc *bloc=blocks.Last();
		int size=bloc->fit(len);

		bloc->store(data,size);

		if (size!=len)
		{
			bloc=blocks.InsertLast();
			bloc->store(&data[size],len-size);
		}
	}
	else
	{
		int p=0;
		int remain=len;
		CBloc *bloc=blocks.Last();
		int size=MAX_BUF-bloc->getLength();
		bloc->store(data,size);
		p+=size;
		remain-=size;
		while (remain>MAX_BUF)
		{
			bloc=blocks.InsertLast();
			bloc->store(&data[p],MAX_BUF);
			remain-=MAX_BUF;
			p+=MAX_BUF;
		}
		if (remain>0)
		{
			bloc=blocks.InsertLast();
			bloc->store(&data[p],remain);
		}
	}
}

#if defined(_BIG_ENDIAN)

unsigned char * CDataContainer::bloc4(unsigned char * data)
{
#if defined(_BIG_ENDIAN)
    tmp_data[0]=data[3];
    tmp_data[1]=data[2];
    tmp_data[2]=data[1];
    tmp_data[3]=data[0];
#else
    tmp_data[0]=data[0];
    tmp_data[1]=data[1];
    tmp_data[2]=data[2];
    tmp_data[3]=data[3];
#endif
    return &tmp_data[0];
}

unsigned char * CDataContainer::bloc3(unsigned char * data)
{
#if defined(_BIG_ENDIAN)
    tmp_data[0]=data[3];
    tmp_data[1]=data[2];
    tmp_data[2]=data[1];
#else
    tmp_data[0]=data[0];
    tmp_data[1]=data[1];
    tmp_data[2]=data[2];
#endif
    return &tmp_data[0];
}

unsigned char* CDataContainer::bloc2(unsigned char * data)
{
#if defined(_BIG_ENDIAN)
    tmp_data[0]=data[1];
    tmp_data[1]=data[0];
#else
    tmp_data[0]=data[0];
    tmp_data[1]=data[1];
#endif
    return &tmp_data[0];
}

#else

#define bloc4(ptr) ptr;
#define bloc3(ptr) ptr;
#define bloc2(ptr) ptr;

#endif

void CDataContainer::storeLong(unsigned long int param)
{
    int len=8;
    char * data=(char*) (&param);
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);
    
    bloc->store(data,size);
    
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}


void CDataContainer::storeInt(int param)
{
    int len=sizeof(int);
    char * data=(char*) bloc4((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeInt24(int param)
{
    int len=3;
    char * data=(char*) bloc3((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeUInt24(unsigned int param)
{
    int len=3;
    char * data=(char*) bloc3((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeUInt(unsigned int param)
{
    int len=sizeof(unsigned int);
    char * data=(char*) bloc4((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeChar(char param)
{
    int len=1;
    char * data=(char*) (&param);
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeWord(short int param)
{
    int len=2;
    char * data=(char*) bloc2((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeWord(int param)
{
    int len=2;
    char * data=(char*) bloc2((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeWord(unsigned int param)
{
    int len=2;
    char * data=(char*) bloc2((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeWord(unsigned short int param)
{
    int len=2;
    char * data=(char*) bloc2((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeFloat(float param)
{
    int len=sizeof(float);
    char * data=(char*) bloc4((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeDouble(double param)
{
    int len=sizeof(double);
    char * data=(char*) (&param);
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);
    
    bloc->store(data,size);
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeString(char * param)
{
    int len=(int) strlen(param)+1;
    char * data=(char*) (param);
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::storeVector(float x,float y,float z)
{
    storeFloat(x);
    storeFloat(y);
    storeFloat(z);
}

void CDataContainer::storeVector(CVector v)
{
    storeFloat(v.x);
    storeFloat(v.y);
    storeFloat(v.z);
}


void CDataContainer::storeMap(float x,float y)
{
    storeFloat(x);
    storeFloat(y);
}

void CDataContainer::storeMap(CVector2 v)
{
    storeFloat(v.x);
    storeFloat(v.y);
}

void CDataContainer::storeRGBA(float r,float g,float b,float a)
{
    storeFloat(r);
    storeFloat(g);
    storeFloat(b);
    storeFloat(a);
}

void CDataContainer::storeRGB(float r,float g,float b)
{
    storeFloat(r);
    storeFloat(g);
    storeFloat(b);
}


// generic name

void CDataContainer::store(const void *data,int len)
{
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store((char*)data,size);

    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&((char*)data)[size],len-size);
    }
}

void CDataContainer::store(int param)
{
    int len=sizeof(int);
    char * data=(char*) bloc4((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);
    
    bloc->store(data,size);
    
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::store(short int param)
{
    int len=sizeof(int);
    int nn=param;
    char * data=(char*) bloc4((unsigned char*) (&nn));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);
    
    bloc->store(data,size);
    
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::store(float param)
{
    int len=sizeof(float);
    char * data=(char*) bloc4((unsigned char*) (&param));
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::store(const char * param)
{
    int len=(int) strlen(param)+1;
    char * data=(char*) (param);
    CBloc *bloc=blocks.Last();
    int size=bloc->fit(len);

    bloc->store(data,size);
    if (size!=len)
    {
        bloc=blocks.InsertLast();
        bloc->store(&data[size],len-size);
    }
}

void CDataContainer::store(float x,float y,float z)
{
    storeFloat(x);
    storeFloat(y);
    storeFloat(z);
}

void CDataContainer::store(CVector v)
{
    storeFloat(v.x);
    storeFloat(v.y);
    storeFloat(v.z);
}

void CDataContainer::store(float x,float y)
{
    storeFloat(x);
    storeFloat(y);
}

void CDataContainer::store(CVector2 v)
{
    storeFloat(v.x);
    storeFloat(v.y);
}

void CDataContainer::store(float r,float g,float b,float a)
{
    storeFloat(r);
    storeFloat(g);
    storeFloat(b);
    storeFloat(a);
}

