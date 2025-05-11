
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

#include "defines.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define API3D_DIRECT3D							// initialise with directX8
//#define API3D_DIRECT3D9							// initialise with directX9
//#define API3D_VERSION_DX9c						// initialise with directX9c (combine with API3D_DIRECTX9)
//#define API3D_DIRECT3D10							// initialise with directX10
//#define API3D_VERSION_DX10_1						// initialise with directX10_1 (combine with API3D_DIRECTX9)
//#define API3D_DIRECT3D11							// initialise with directX11
//#define API3D_SDL_DIRECT3D						// initialise with directX8 and SDL
//#define API3D_SDL_OPENGL							// initialise with openGL and SDL
//#define API3D_OPENGL								// initialise with openGL only without video mode init (wxWidgets)
//#define API3D_OPENGL20							// full shader abstraction layer (GLES2)
//#define GLES										// android, iphone specific GLES1.1
//#define GLES20									// android, iphone specific GLES2.0
//#define API3D_SDL2								// inititalise with SDL 2
//#define API3D_GL_PROTOTYPES

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef API3D_STATIC
#	ifdef API3D_DIRECT3D11
#		pragma comment(lib,"d3d11.lib")
#		pragma comment(lib,"d3dcompiler.lib")
#	endif
#	ifdef API3D_DIRECT3D10
#		pragma comment(lib,"d3d10.lib")
#		pragma comment(lib,"d3dx10.lib")
#	endif
#	ifdef API3D_DIRECT3D9
#		pragma comment(lib,"d3d9.lib")
#		pragma comment(lib,"d3dx9.lib")
#	endif
#	ifdef API3D_DIRECT3D
#		pragma comment(lib,"d3d8.lib")
#		pragma comment(lib,"d3dx8.lib")
#	endif
#	ifdef API3D_SDL_OPENGL
#		pragma comment(lib,"glu32.lib")
#		pragma comment(lib,"opengl32.lib")
#		pragma comment(lib,"sdl.lib")
#		pragma comment(lib,"sdlmain.lib")
#	else
#		pragma comment(lib,"glu32.lib")
#		pragma comment(lib,"opengl32.lib")
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(WEBASM)||defined(API3D_SDL2)

#define _DEFINES_API_CODE_GENERATOR_

#define _DEFINES_API_CODE_BSP_LIGHTS_
#define _DEFINES_API_CODE_BSP_VOLUMIC_
#define _DEFINES_API_CODE_SOFTRENDERER_
#define _DEFINES_API_CODE_TREES_
#define _DEFINES_API_CODE_METABALLS_
#define _DEFINES_API_CODE_GENVEGETAUX_
#define _DEFINES_API_CODE_GENTEXTURE_
#define _DEFINES_API_CODE_PHYSIC_

#define _DEFINES_API_CODE_TEXTURE_MANAGER_
#define _DEFINES_API_CODE_MULTIPLE_SIZE_LIGHTMAP_

#ifdef API3D_SDL2
#define _DEFINES_OPENGL_NO_SDL_INIT_
#endif
#else

//#define _DEFINES_OPENGL_NO_SDL_INIT_
#define _DEFINES_API_CODE_GENERATOR_

//#define _DEFINES_API_CODE_BSP_LIGHTS_
#define _DEFINES_API_CODE_BSP_VOLUMIC_
//#define _DEFINES_API_CODE_SOFTRENDERER_
//#define _DEFINES_API_CODE_TREES_
//#define _DEFINES_API_CODE_METABALLS_
//#define _DEFINES_API_CODE_GENVEGETAUX_
//#define _DEFINES_API_CODE_GENTEXTURE_
//#define _DEFINES_API_CODE_PHYSIC_

#define _DEFINES_API_CODE_TEXTURE_MANAGER_
#define _DEFINES_API_CODE_MULTIPLE_SIZE_LIGHTMAP_
//#define _DEFINES_API_CODE_NO_LIGHTMAP_LEAKS_
//#define _DEFINES_API_CODE_GLITCH_
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NBRE_MAX_TEXTURES 4096
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

