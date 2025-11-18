
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include "params.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									   SDL/DIRECT3D
#ifdef API3D_SDL_DIRECT3D

	#include "SDL/SDL.h"
	#include "SDL/SDL_syswm.h"
	#define API3D_DIRECT3D9

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									  SDL/DIRECT3D8
#ifdef API3D_SDL_DIRECT3D8

	#include "SDL/SDL.h"
	#include "SDL/SDL_syswm.h"

	#define API3D_SDL_DIRECT3D
	#define API3D_DIRECT3D

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									       DIRECT3D
#ifdef API3D_DIRECT3D
	#include <windows.h>
	#include "d3dx8.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									      DIRECT3D9
#ifdef API3D_DIRECT3D9
	#include <windows.h>
	#include "d3dx9.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									     DIRECT3D10
#ifdef API3D_DIRECT3D10
	#include <windows.h>
	#include <d3d10_1.h>
	#include <d3dx10.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									     DIRECT3D11
#ifdef API3D_DIRECT3D11

#ifdef WINDOWS_PHONE
	#include <d3d11_2.h>
	#include <d3dcompiler.h>
	#include "../fx11/fx11.h"
	#define LIB3D_OWNED_FX11
#else
	#include <windows.h>
	#include <d3d11.h>
	#include <d3dcompiler.h>
	#include "../fx11/fx11.h"
	#define LIB3D_OWNED_FX11
#endif

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									     DIRECT3D12
#ifdef API3D_DIRECT3D12

	#include <windows.h>
	#include <dxgi1_4.h>
	#include <d3d12.h>
	#include <d3dcompiler.h>
	#include "../dx12/d3dx12.h"
	#include "../dx12/fx12.h"
	#include "../dx12/descriptorHeap.h"
	#define LIB3D_OWNED_FX11
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									     SDL/OPENGL
//																											 GLES20

#ifdef API3D_OPENGL20

	#if defined(__APPLE__)||defined(LINUX)
		#ifndef LINUX

			#ifdef IOS
				#include <OpenGLES/ES2/gl.h>
				#include <OpenGLES/ES2/glext.h>
			#else
				#include "OpenGL/gl.h"
				#include "OpenGL/glext.h"
				#include "OpenGL/glu.h"
			#endif

			#define OPENGL_MULTITEXTURING
			#define OPENGL_FRAMEBUFFERS
			#define OPENGL_GLSL
			#define OPENGL_VBO
		#else
			#include <GL/gl.h>
			#include <GL/glext.h>
			#include <GL/glu.h>

			#define OPENGL_MULTITEXTURING
			#define OPENGL_FRAMEBUFFERS
			#define OPENGL_GLSL
			#define OPENGL_VBO

		#endif
	#else
		#ifdef WIN32
			#include <windows.h>

#ifndef _DEFINES_OPENGL_NO_SDL_INIT_
			#include "SDL/SDL.h"
#endif
			#ifdef API3D_VR
				#ifdef OCULUS
					#include "GL/CAPI_GLE.h"
				#else
					#include <SDL.h>
					#include <GL/glew.h>
					#include <SDL_opengl.h>
					#include <GL/glu.h>
				#endif
			#else
				#ifndef API3D_SDL2
					#include <gl/gl.h>
					#include <gl/glext.h>
					#include <gl/glu.h>
					//#include <gl/wglext.h>
				#endif		
			#endif
			#ifdef API3D_SDL2
				#include <SDL.h>
				#include <SDL_opengl.h>
				#include <GL\GL.h>
				#include <GL\GLU.h>
			#endif	
			#define OPENGL_MULTITEXTURING
			#define OPENGL_FRAMEBUFFERS
			#define OPENGL_GLSL
			#define OPENGL_VBO
		#else

			#ifdef __APPLE__
				#include <SDL/SDL.h>
				#include <GL/gl.h>
				#include <GL/glu.h>
				#include <GL/glext.h>
				typedef wchar_t WCHAR;
				#define OPENGL_VBO
			#endif

			#ifdef ANDROID
				#include <EGL/egl.h>
				#if defined(GOOGLEVR)&&!defined(ES2)
					#include <GLES3/gl3.h>
					#include <GLES2/gl2ext.h>
					#include <GLES3/gl3ext.h>
				#else
				#ifdef API3D_VR
					#include <GLES3/gl3.h>
					#include <GLES2/gl2ext.h>
					#include <GLES3/gl3ext.h>
				#else
					#include <GLES2/gl2.h>
					#include <GLES2/gl2ext.h>
				#endif
				#endif
				typedef wchar_t WCHAR;
				#define OPENGL_MULTITEXTURING
				#define OPENGL_FRAMEBUFFERS
				#define OPENGL_GLSL
				#define OPENGL_VBO
			#endif

			#ifdef API3D_SDL2
				#include <SDL.h>
				#include <SDL_opengl.h>
				#include <GL/GL.h>
				#include <GL/GLU.h>

				#define OPENGL_MULTITEXTURING
				#define OPENGL_FRAMEBUFFERS
				#define OPENGL_GLSL
				#define OPENGL_VBO			
			#endif

			#ifdef WEBASM
				#include <functional>
				#include <emscripten.h>
				#include <SDL.h>
				#define GL_GLEXT_PROTOTYPES 1
				#include <SDL_opengles2.h>
				#define OPENGL_MULTITEXTURING
				#define OPENGL_FRAMEBUFFERS
				#define OPENGL_GLSL
				#define OPENGL_VBO
			#endif

			#ifdef IPHONE
				#include <OpenGLES/ES1/gl.h>  //TODO (GLES20)
				#include <OpenGLES/ES1/glext.h>
				#define OPENGL_MULTITEXTURING
				#define OPENGL_FRAMEBUFFERS
				#define OPENGL_GLSL
				#define OPENGL_VBO
			#endif

		#endif

	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																									     SDL/OPENGL
//																									         GLES11

#ifdef API3D_OPENGL
	
	#ifdef  __APPLE__
		#ifndef GLES

			#include "OpenGL/gl.h"
			#include "OpenGL/glext.h"
			#include "OpenGL/glu.h"

			#define OPENGL_MULTITEXTURING
			#define OPENGL_VERTEXPROGRAMS
			#define OPENGL_FRAMEBUFFERS
			#define OPENGL_GLSL
			#define OPENGL_VBO
		#endif
	#else

		#ifdef WIN32
			#include <windows.h>

#ifndef _DEFINES_OPENGL_NO_SDL_INIT_
			#include "SDL/SDL.h"
#endif
			#include <gl/gl.h>
			#include <gl/glext.h>
			#include <gl/glu.h>
			#include <gl/wglext.h>

			#define OPENGL_MULTITEXTURING
			#define OPENGL_VERTEXPROGRAMS
//			#define OPENGL_PBUFFERS
			#define OPENGL_FRAMEBUFFERS
			#define OPENGL_GLSL
			#define OPENGL_VBO
        #else

		#ifdef __APPLE__
			#include <SDL/SDL.h>
			#include <GL/gl.h>
			#include <GL/glu.h>
			#include <GL/glext.h>
			typedef wchar_t WCHAR;
			#define OPENGL_VBO
			//#include <GL/glx.h>
			//#include <GL/glxext.h>
		#endif

		#ifdef ANDROID
			#include <GLES/gl.h>
			#include <GLES/glext.h>
			typedef wchar_t WCHAR;
			#define OPENGL_MULTITEXTURING
			#define OPENGL_VBO
		#endif

		#ifdef IPHONE
			#include <OpenGLES/ES1/gl.h>
			#include <OpenGLES/ES1/glext.h>
			#define OPENGL_VBO
		#endif

	#endif
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#pragma warning (disable:4267)
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
