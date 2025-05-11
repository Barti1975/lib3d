
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


#ifndef _DATA_CONTAINER_H_
#define _DATA_CONTAINER_H_

class CDataReader;
class CDataContainer;


#include "../base/params.h"
#include "../base/maths.h"
#include "../base/objects3d.h"
#include "lzcompress.h"

#if !defined(WIN32)&&!defined(ANDROID)
#include <stdlib.h>
#else
#include <malloc.h>
#endif


#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
#include "file_io_zip.h"
#endif

#include <memory.h>
#include <stdio.h>
#include <string.h>

#define MAX_BUF 4096

#ifdef CLASSNED
#include "../../ned/myfile.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBloc
{
public:
	char * data;
	unsigned int ptr;

	CBloc() { data=(char*) malloc(MAX_BUF); ptr=0;}

	void store(const char * ptr_data,int size);

	int fit(int size);

	int getLength();

	char * getData();

	void clean();

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDataReader
{
public:
	char * data;
	unsigned int ptr;
	unsigned int data_length;
    bool shrunk;
	
	CDataReader() { shrunk=true;data=NULL;ptr=0;data_length=0;}

	unsigned int getSize();
    
    void setRaw();
    
    void setNormal();

	char * getData();

	void clean();

	bool eod();

    inline CDataReader operator +(CDataReader cdr)
    {
        CDataReader * res=new CDataReader;

        res->data_length=getSize() + cdr.getSize();
        res->data=(char*) malloc(res->data_length);
        res->ptr=0;
        if (data) memcpy(res->data,data,getSize());
        if (cdr.data) memcpy(&(res->data[getSize()]),cdr.data,cdr.getSize());

        return (*res);
    }

	void get(char *raw,int size);

#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
   	void get(ZIPFILE *f,int size);

	void getLZ(ZIPFILE *f);

	struct Inflate * getLZnfo(ZIPFILE *f);
#else
	void get(FILE *f,int size);

	void getLZ(FILE *f);

	struct Inflate * getLZnfo(FILE *f);
#endif

#ifdef CLASSNED
	void getLZ(MyFILE *f);
	struct Inflate * getLZnfo(MyFILE *f);
#endif

	int getLZ(char * raw);

	int readInt();

	int readInt24();

	unsigned int readUInt24();

	unsigned int readUInt();

	int readChar();

	int readWord();

	float readFloat();

	char* readData(int len);

	void readData(void *dat,int len);

	CVector readVector();

	CQuaternion readQuaternion();

	CVector2 readMap();

	CRGBA readRGB();

	CRGBA readRGBA();

	char * readString();

	char * readString10();
    
    double readDouble();
    
    unsigned long int readLong();
    
    long int readLongInt();

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDataContainer
{
public:
	CList <CBloc> blocks;
	struct Inflate *nfo;
	char *result;
	unsigned char tmp_data[4];
    bool shrunk;

    CDataContainer() { shrunk=true; blocks.Free(); nfo=NULL; result=NULL; blocks.InsertLast();}

    ~CDataContainer()
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
        shrunk=true;
    }

	void clean();
    
    void setRaw();
    
    void setNormal();
    
	unsigned int getLength();

	char * getData();

	unsigned int getLZLength();

	char * getLZData();

	void compressFast();

	void compress();

	void storeData(const char *data,int len);

#if defined(_BIG_ENDIAN)
	unsigned char * bloc4(unsigned char * data);

	unsigned char * bloc3(unsigned char * data);

	unsigned char* bloc2(unsigned char * data);
#endif
	void storeInt(int param);

	void storeInt24(int param);

	void storeUInt24(unsigned int param);

	void storeUInt(unsigned int param);

	void storeChar(char param);

	void storeWord(short int param);

	void storeWord(int param);

	void storeWord(unsigned int param);

	void storeWord(unsigned short int param);

	void storeFloat(float param);
    
    void storeDouble(double param);
    
    void storeLong(unsigned long int param);

	void storeString(char * param);

	void storeVector(float x,float y,float z);

	void storeVector(CVector v);

	void storeMap(float x,float y);

	void storeMap(CVector2 v);

	void storeRGBA(float r,float g,float b,float a);

	void storeRGB(float r,float g,float b);

	// generic name

	void store(const void *data,int len);

	void store(int param);
    
    void store(short int param);

	void store(float param);

	void store(const char * param);

	void store(float x,float y,float z);

	void store(CVector v);

	void store(float x,float y);

	void store(CVector2 v);

	void store(float r,float g,float b,float a);

};


#endif
