
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Cancé Francis, 10/08/1975>
//    laurent.francis.cance@outlook.fr
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

#ifndef _3D_API_H_
#define _3D_API_H_

#include "3d_api_base.h"

enum
{
	MMBL_LIGHT1				=	1,
	MMBL_LIGHT2				=	2,
	MMBL_LIGHT3				=	4,

	MMBL_RESET				=	16,


	COMMON_RESET			=	1,
	COMMON_MULTITEXTURE		=	2,
	PROJECT_BASE_GROUND		=	4
};

/*?///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	<i>Sample code:</i>

	<hr>

	<p><u>I. Rendering shadows in user buffer</u></p>

		CVector L;
		CCollisionMesh CollisionMeshShadowing;
		CVertexbuffer vb_shadows;


		CollisionMeshShadowing.Init2(MESHSHD,0,0,12);

		vb_shadows.SetAPI(&Render);
		vb_shadows.SetType(API3D_VERTEXDATAS|API3D_COLORDATAS|API3D_TEXCOODATAS|API3D_TEXPERVERTEX);
		vb_shadows.Init(2000,6000);
	
		
		<b>Static light (1)</b>

		Render.SetLightPoint(1,L,1,1,1,0,50);
		Render.DisableLight(1);							
		Render.SetParams(API3D_CULL,CCW);
		Render.SetParams(API3D_BLEND,OFF);

		ball.Output=PROPRIETES_MATRICES;
		ball.M=?
		ball.MR=?

		<b>Render to vertex buffer only (do not draw result to screen)</b>
		<b>Processing the shadows</b>


		Render.SetParams(API3D_SHADOWING,DONT_DRAW);
		Render.SetShadowVertexBuffer(&vb_shadows);
		Render.LightShadowsMeshMapping2b(1,&ball,&CollisionMeshShadowing);
		Render.SetParams(API3D_ZBIAS,8);
		

		<b>Rendering the shadows</b>


		M.Id();		
		Render.LoadWorldMatrix(M);

		if (vb_shadows[n].nIndices>0)
		{
			Render.SetVertexBuffer(&Game.vb_shd_Prop[n]);
			Render.DrawVertexBuffer();
		}

		<b>Free</b>


		CollisionMeshShadowing.quad.clean();
		CollisionMeshShadowing.quad.Free();
		vb_shadows.Release();


	<p><u>II. Rendering object projection in user buffer</u></p>

		<b>At first call</b>
		
		Render.LightProjectMesh(nl,obj,&meshouille,COMMON_RESET); 

		<b>At other call</b>
		
		Render.LightProjectMesh(nl,obj,&meshouille,0);







<hr>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													INTERFACES CLASS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

class API3D_STUB CGenericData
{
public:
	int idata[48];
	float fdata[16];
	int id;
	void *data;
	char * group;

	CGenericData() { data=NULL;group=NULL;id=-1; }
};

class API3D_STUB CGenericInterface
{
public:
	int type;
	int selected;
	void *api;
	CMap <unsigned int,CGenericData> localdata;
	char * actualnamegroup;

	CGenericInterface() { type=-1;selected=-1;api=NULL;actualnamegroup=NULL; }

	void	init();
	void	release(char *namegroup);
	void	release();
	int		create();
	void	create(int ref);
	void	free(int ref);
	void	update(void * data);
	void*	update(int ref,void * data);
	void	select(int ref);
	void	set(int quoi,int comment);
	void	set(int quoi,int comment,int modifier);
	void*	get();
	void	setgroup(char *namegroup);
};

/*

exemple1:

	int tex=Render.Interface(I_TEXTURES)->create();
	Render.Interface(I_TEXTURES)->set(API3D_SIZE,16);		// texture 16bits
	Render.Interface(I_TEXTURES)->set(API3D_WIDTH,256);
	Render.Interface(I_TEXTURES)->set(API3D_HEIGHT,256);
	Render.Interface(I_TEXTURES)->set(API3D_SRCWIDTH,214);
	Render.Interface(I_TEXTURES)->set(API3D_SRCHEIGHT,160);
	Render.Interface(I_TEXTURES)->set(API3D_ALPHA,1);
	Render.Interface(I_TEXTURES)->set(API3D_DATA,API3D_RGBA);
	Render.Interface(I_TEXTURES)->update(im.ptrImageDATAS);

	int tex2=Render.Interface(I_TEXTURES)->create();
	Render.Interface(I_TEXTURES)->set(API3D_SIZE,32);		// texture 32bits
	Render.Interface(I_TEXTURES)->set(API3D_WIDTH,256);		// src dim=dest dim
	Render.Interface(I_TEXTURES)->set(API3D_HEIGHT,256);
	Render.Interface(I_TEXTURES)->set(API3D_ALPHA,1);
	Render.Interface(I_TEXTURES)->set(API3D_DATA,API3D_RGBA);
	Render.Interface(I_TEXTURES)->update(im.ptrImageDATAS);

exemple2:

	CVertexBuffer *vb;
	CObject3D *obj;
	CVertexBuffer *vb2;
	CObject3D *obj2;

	Render.Interface(I_VERTEXBUFFERS)->setgroup("GROUP1");
	Render.Interface(I_VERTEXBUFFERS)->create();
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,1,API3D_TAG0);
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,0,API3D_TAG1);
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_TAG,0);
	
	vb=(CVertexBuffer*) Render.Interface(I_VERTEXBUFFERS)->update(obj);

	Render.Interface(I_VERTEXBUFFERS)->setgroup("GROUP2");
	Render.Interface(I_VERTEXBUFFERS)->create();
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,1,API3D_TAG0);
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,0,API3D_TAG1);
	Render.Interface(I_VERTEXBUFFERS)->set(API3D_TAG,0);
	
	vb2=(CVertexBuffer*) Render.Interface(I_VERTEXBUFFERS)->update(obj2);

	Render.Interface(I_VERTEXBUFFERS)->setgroup(NULL);
	Render.Interface(I_VERTEXBUFFERS)->release("GROUP1");


exemple3:

	Render.Interface(I_LIGHTMAPS)->init();
	Render.Interface(I_LIGHTMAPS)->select(I_SETTINGS);
	Render.Interface(I_LIGHTMAPS)->set(API3D_SIZE,32);
	...
	char *buf0;
	char *buf1;
	int ref0=Render.Interface(I_LIGHTMAPS)->create();
	int ref1=Render.Interface(I_LIGHTMAPS)->create();
	
	Render.Interface(I_LIGHTMAPS)->select(ref0);
	Render.Interface(I_LIGHTMAPS)->set(API3D_SRCWIDTH,64);
	Render.Interface(I_LIGHTMAPS)->set(API3D_SRCHEIGHT,64);
	Render.Interface(I_LIGHTMAPS)->set(API3D_SIZE,API3D_DIV4);
	Render.Interface(I_LIGHTMAPS)->update(buf0);
	
	Render.Interface(I_LIGHTMAPS)->select(ref1);					// src dim = API3D_SIZE dim; w=h=32
	Render.Interface(I_LIGHTMAPS)->set(API3D_SIZE,API3D_PLAIN);
	Render.Interface(I_LIGHTMAPS)->update(buf1);

exemple 4:

void function_draw_vb(void *data)
{
    CVertexBuffer * vb=(CVertexBuffer *) data;
   
    Render.SetVertexBuffer(vb);
    Render.DrawVertexBuffer();
}

    CVertexBuffer *vb;

    Render.Interface(I_VERTEXBUFFERS)->create();
    Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,1,API3D_TAG0);
    Render.Interface(I_VERTEXBUFFERS)->set(API3D_REF,0,API3D_TAG1);
    Render.Interface(I_VERTEXBUFFERS)->set(API3D_TAG,0);  
    vb=(CVertexBuffer*) Render.Interface(I_VERTEXBUFFERS)->update(obj);

    Render.Interface(I_VERTEXBUFFERS)->define("Draw",function_draw_vb);
    ...
    Render.Interface(I_VERTEXBUFFERS)->select(vb);
    Render.Interface(I_VERTEXBUFFERS)->do("Draw");


exemple 5:

    Render.Interface(I_LIGHTMAPS)->select(ref0);
    Render.Interface(I_LIGHTMAPS)->set(API3D_SRCWIDTH,64,API3D_SRCHEIGHT,64,API3D_SIZE,API3D_DIV4);
    Render.Interface(I_LIGHTMAPS)->update(buf0);


*/


enum
{
	API3D_SIZE				=	0,
	API3D_DATA,
	API3D_WIDTH,
	API3D_HEIGHT,
	API3D_SRCWIDTH,
	API3D_SRCHEIGHT,
	API3D_ALPHA,
	API3D_MIPMAP,

	API3D_REF				=	666,
	API3D_TAG,
	API3D_PLAIN,
	API3D_DIV2,
	API3D_DIV4,
	API3D_TAG0,
	API3D_TAG1,
	API3D_TAG2,
	API3D_TAG3,
	API3D_TAG4,
	API3D_TAG5,
	API3D_TAG6,
	API3D_TAG7,
	API3D_ON,
	API3D_OFF,
	API3D_RGB,
	API3D_RGBA,
	API3D_GRAYSCALE
};

enum
{
	I_SETTINGS				=	-1,
	I_LIGHTMAPS				=	0,
	I_TEXTURES,
	I_VERTEXBUFFERS
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													C3DAPI CLASS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//! Derivated Render class
class API3D_STUB C3DAPI : public C3DAPIBASE
{
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Primitives

	//? Triangle Drawing

		//! Draw triangles (rgb is took from vertices, Calc is the coo.)
		void	Tri_Gouraud(CFace *f);
		//! Draw triangles flat mode (Calc is the coo.)
		void	Tri_Flat(CFace *f);
		//! Draw triangles (rgb,map is took from face, Calc is the coo.)
		void	Tri_Mapping(CFace *f);
		//! Draw triangles (rgb,map is took from vertices, Stok is the coo.)
		void	Triv_Mapping(CFace *f);
		//! Draw triangles (Map took from face, rgb is took from vertices, Calc is the coo.)
		void	Tri_GouraudMapping(CFace *f);
		//! Draw triangles (Map took from vertices, rgb is took from vertices, Calc is the coo.)
		void	Triv_GouraudMapping(CFace *f);
		//! Draw triangles (rgb is took from vertices, Stok is the coo.)
		void	Tri_GouraudStok(CFace *f);
		//! Draw triangles (rgb is took from face, Stok is the coo.)
		void	Tri_MappingStok(CFace *f);
		//! Draw triangles (obsolete)
		void	Tri_Flat0(float x1,float y1,float x2,float y2,float x3,float y3);

		//! Draw triangles gouraud 2d
		void	Tri_Gouraud2D(float x1,float y1,float x2,float y2,float x3,float y3,CRGBA c1,CRGBA c2,CRGBA c3);

		//! Draw triangles 
		void	Tri_MappingStokF(CFace *f);
		//! Draw triangles (Map took from vertices, rgb is took from vertices, Stok is the coo.)
		void	Triv_GouraudMappingStok(CFace *f);
		//! Draw triangles in flat mode
		void	Tri_Flat(CVector A,CVector B,CVector C,float r,float g,float b);
		//! Draw triangles in flat mode with alpha value
		void	Tri_Flat(CVector A,CVector B,CVector C,float r,float g,float b,float a);

		//! Draw coloured textured triangle
		void	TriMC(float x0,float y0,float x1,float y1,float x2,float y2,float u0,float v0,float u1,float v1,float u2,float v2,float r,float g,float b,float a);

	//? Quad Drawing

		//! Rendering quad in flat mode (transformations are applied)
		void	Quad_Flat(CVector A,CVector B,CVector C,CVector D,float r,float g,float b);

		//! Rendering quad in flat mode (transformations are applied, mapping coordinate are took from .Map of Vertices)
		void	Quad_MappingStok(CVertex * v0,CVertex * v1,CVertex * v2,CVertex * v3,float r,float g,float b,float a);

		//! Draw a rectangular area (obsolete function)
		void	Quad0(int x,int y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b);

		//! Draw a rectangular area with depth testing, usefull for sprites
		void	QuadZ(float z,float w,float x,float y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b);

		//! Draw a rectangular area with depth testing, usefull for sprites (alpha component is implemented)
		// see quad() parameters
		void	QuadZ(float z,float w,float x,float y,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b,float a);

		//! Draw a rectangular area with depth testing, usefull for sprites
		void	QuadXYZ(CVector &pos,float R,float z,float w,float x0,float y0,float sizex,float sizey,float x1,float y1,float x2,float y2,float r,float g,float b,float a);

		//! Draw a rectangular area with depth testing, usefull for sprites (alpha component is implemented)
		// see quad() parameters
		void	QuadRotZ(float z,float w,float x,float y,float sizex,float sizey,float ang,float x1,float y1,float x2,float y2,float r,float g,float b,float a);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// shadows

		//! specifies witch vb to calculate shadow - specifying NULL invokes API vb for shadows
		// @param vb: vertex buffer to store processing
		void	SetShadowVertexBuffer(CVertexBuffer * vb) { if (vb) ShadowVertexBuffer=vb; else ShadowVertexBuffer=&SHADOW_MAPPING; }

	
	//? basic shadowing

		//! Draw a shadow from obj occluder of nLight lightsource on a plane (a,b,c,d)
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param a,b,c,d: plane equation
		void	LightShadowsPlaneFlat(int nLight,CObject3D * obj,float a,float b,float c,float d);

		
		//! Draw a shadow from obj occluder of nLight lightsource on a plane (a,b,c,d) with mapping coords specified
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param a,b,c,d: plane equation
		//! @param A,B,C,xm,ym: mapping minimal info
		void	LightShadowsPlaneMapping(int nLight,CObject3D * obj,float a,float b,float c,float d,
										 CVector A,CVector B,CVector C,
										 float xm,float ym);


#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_
	//? enhanced shadowing 


		// SetShadowVertexBuffer() specifies th vb to use for shadow 
		// parameters for shadowing are API3D_VERTEXDATAS|API3D_COLORDATAS|API3D_TEXCOODATAS

		//! object to object shadows - one texture only (obj is the occluder)
		//! Project obj to ref.
		//! the .Output of the object can be : 
		//! PROPRIETES_PHYSIQUES: get tranformation for objects in Cobject3D::Proprietes
		//! PROPRIETES_MATRICES: get tranformation for objects int Cobject3D::M and Mr matrices
		//! or 0: apply euler Rot tranformation and Coo translation to objects.
		//! Single texture.
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param ref: object to project to
		void	LightShadowsObjectMapping(int nLight,CObject3D * obj,CObject3D * ref);

		//! fast object to object shadows, but needs a ORTHOBSP - one texture only (obj is the occluder)
		//! collision mesh initialised with ::init()
		//! Project shadow of obj dur to light nLight or the renderer on mesh.
		//! The collision mesh have to be initialized with Init(obj,0,0,tree_depth_value);
		//! Single texture.
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param mesh: object to project to (collision mesh)
		void	LightShadowsMeshMapping(int nLight,CObject3D * obj,CCollisionMesh * mesh);

		//! fast object to object shadows, but needs a ORTHOBSP - full nT texturing management
		//! collision mesh initialised with ::init2()
		//! This function takes in consideration the Faces[n].nT value of ref.
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param mesh: object to project to (collision mesh)
		void	LightShadowsMeshMapping2(int nLight,CObject3D * objb,CCollisionMesh * mesh);

		//! fast object to object shadows, but needs a ORTHOBSP - full nT texturing management
		//! collision mesh initialised with ::init2()
		//! This function takes in consideration the Faces[n].nT value of ref.
		//! variation
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param mesh: object to project to (collision mesh)
		void	LightShadowsMeshMapping2b(int nLight,CObject3D * objb,CCollisionMesh * mesh);
		
		//! object to object shadows - full nT texturing management
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param ref: object to project to
		void	LightShadowsObjectMapping_nT(int nLight,CObject3D * obj,CObject3D * ref);

		//! object to object shadows - full nT,nL,NLVectors texturing management
		//! Project shadow of obj due to light nLight or the renderer on mesh on a bump mapping vertex buffer.
		//! The collision mesh have to be initialized with Init3(obj,0,0,tree_depth_value);
		//! @param nLight: light index
		//! @param obj: object to project
		//! @param mesh: object to project to (collision mesh)
		void	LightShadowsMeshMappingBL(int nLight,CObject3D * objb,CCollisionMesh * mesh,unsigned int flags);
#endif

		//! dynamic slow stencil buffer shadowing (this method is slow)
		//! @param nLight: light index
		//! @param obj: object to project
		void	DrawShadowVolume(int nL,CObject3D *obj);
		//! dynamic slow stencil buffer shadowing but usefull to prevent camera inside shadow volume
		//! @param nLight: light index
		//! @param obj: object to project
		void	DrawShadowVolumeExotic(int nL,CObject3D *obj,CMatrix M,CMatrix Mr,CVector Camera,float len);

		//! set vertex buffer alpha of vertices (apply of selected vertex buffer)
		//! @param alpha: alpha value
		//! @param ref: object
		//! @param M: matrix
		void	SetVertexBufferAlpha(float alpha,CObject3D * ref,CMatrix M);

#ifdef _DEFINES_API_CODE_BSP_VOLUMIC_

	//? projections

		//! project a texture on an object, (Pos define the center of the texture with sizex,sizey dimension, DEF is the subdivision depth)
		//! @param nLight: light index
		//! @param sizex,sizey: image dimension values
		//! @param ref: object to project to
		//! @param DEF: tesselation value
		void	LightProjectTexture(int nLight,CVector Pos,float sizex,float sizey,CObject3D * ref,int DEF);
		void	LightProjectTexture(int nLight,CVector Pos,float sizex,float sizey,CObject3D * ref,int DEF,int reset);
		//! project a texture on an object, (Pos define the center of the texture with sizex,sizey dimension, DEF is the subdivision depth)
		//! @param nLight: light index
		//! @param sizex,sizey: image dimension values
		//! @param ref: object to project to
		//! @param DEF: tesselation value
		// cf. below
		void	LightProjectTextureMesh(int nLight,CVector Pos,float sizex,float sizey,CCollisionMesh *mesh,int DEF,int reset);

		//! project an object(obj) on an object(ref)
		//! @param nLight: light index
		//! @param obj: object
		//! @param ref: object to project to
		void	LightProjectObject(int nLight,CObject3D *obj,CObject3D * ref);

		//! project an object(obj) on an object(collision mesh defined)
		//! @param nLight: light index
		//! @param obj: object
		//! @param mesh: object to project to (collision mesh)
		void	LightProjectMesh(int nLight,CObject3D *obj,CCollisionMesh * mesh,int reset);



#endif
};

/*?
	<B>Note:</B> The use of collision mesh speeds the processing by setting an orthogonal bsp of the reference object and by projecting only that is necessary.
*/


#endif
