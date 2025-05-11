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

#ifndef _DEFINES_H_
#define _DEFINES_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)||defined(API3D_SDL2)
#pragma warning(disable:4005)
#pragma warning(disable:4703)
#pragma warning(disable:4251)
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#  ifndef __WIN32__
#    define __WIN32__
# endif
#endif
//#  define MAC_OS_X

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define API3D_STATIC

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DLL stuffs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WINDOWS_PHONE
#  define API3D_STUB
#endif

#ifdef WIN32
#ifdef API3D_STATIC
#  define API3D_STUB
#else
#  ifdef LIB3D_1_3_DLL_EXPORTS
#    define API3D_STUB __declspec(dllexport)
#  else
#    define API3D_STUB __declspec(dllimport)
#  endif
#endif
#endif

#ifdef __APPLE__
#define API3D_STATIC
#define API3D_STUB
#ifdef API3D_METAL
#else
#ifndef API3D_OPENGL20
#define API3D_OPENGL
#ifndef _DEFINES_OPENGL_NO_SDL_INIT_
#define API3D_SDL_OPENGL
#endif
#endif
#endif
#endif

#ifdef LINUX
#define API3D_STATIC
#define API3D_STUB
#ifndef API3D_OPENGL20
#define API3D_OPENGL
#endif
#ifndef _DEFINES_OPENGL_NO_SDL_INIT_
#define API3D_SDL_OPENGL
#endif
#endif

#ifdef ANDROID
#define API3D_STATIC
#define API3D_STUB
#ifdef API3D_OPENGL20
#define GLES20
#else
#define API3D_OPENGL
#define GLES
#endif
#define _DEFINES_OPENGL_NO_SDL_INIT_
#endif

#ifdef IOS
#define API3D_STATIC
#define API3D_STUB
#ifndef API3D_METAL
#ifdef API3D_OPENGL20
#define GLES20
#else
#define API3D_OPENGL
#define GLES
#endif
#endif
#define _DEFINES_OPENGL_NO_SDL_INIT_
#endif

#ifdef WEBASM
#define API3D_STATIC
#define API3D_STUB
#define GLES20
#define _DEFINES_OPENGL_NO_SDL_INIT_
#define GLhandleARB GLint
#endif

#ifdef API3D_SDL2
#define API3D_STUB
#define API3D_STATIC
#define _DEFINES_OPENGL_NO_SDL_INIT_
#endif

#ifndef WIN32
#	ifdef ANDROID
#		include <malloc.h>
#	else
#		include <stdlib.h>
#	endif
#else
#	include <malloc.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
