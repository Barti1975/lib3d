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


#ifdef ANDROID
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\..\libzip\zip.h"

#define ZIPFILE zip_file

zip_file * ZIPfopen(char *filename,char *attr);

int ZIPfread(void *buffer,int len,int nb,zip_file *f);

int ZIPfread_(void *buffer,int len,int nb,zip_file *f);

unsigned int ZIPfsize(zip_file *f);

int ZIPfclose(zip_file * f);

int ZIPfeof(zip_file * f);

int ZIPFILEINIT();

int ZIPfgets(char * str, int len, zip_file *f);

int ZIPfscanf(zip_file *f,char * format, void * ptr);

bool ZIPfexist(char *filename);

char * RecupereCHAINE2(zip_file *f);

char * RecupereCHAINE(zip_file *f);

void TimeInit();

float TimeGet();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
