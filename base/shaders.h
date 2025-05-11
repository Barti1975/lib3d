/*?

<U><B> Vertex Programs </B></U>
<BR>
This implementation of vertex program and fragment program from openGL and vertex shaders 1.1 and pixel shaders 1.4 from directX, is developped to interpret and compile in assembly, pseudo code in a particular syntax.
The functionnalities of vertex program in the lib is designed to work on all cards even if graphic card driver doen't support it.(some divers doesn't have the open gl extension, some cards do not implement the ARB spefs even in software).
Here are some explanation on how to develop vertex shaders and pixel shaders and how to decline technique to fit destination graphic cards.
<BR>
<U><B>Exemples of vertex program that works with lib3d vertex programs</B></U>

<BR><HR><BR>

<U> Exemple1:</U> <B> Syntaxe </B>
<BR>
[VERTEXFORMAT]
	XYZ DIFFUSE TEX0			-> vertex buffer format

[DEFINES]
	MATRIX WORLD;				-> link variables, constants
	MATRIX VIEWPROJ;
	CONST zero=;
	CONST one=1.0,1.0,1.0,1.0;
	TEXTURE tex;
								-> the words following [TECHNIQUE] can be the following : 
								-> Shaders, Dot3, Multitexture, All Cards due to specification of the technique.
[TECHNIQUE]	All cards
[PASS0]
	[POSITION]					-> pseudo code
		u=iPos*4WORLD;
		oPos=u*4VIEWPROJ;
	[DIFFUSE]
		oDiffuse=one;
	[MAPPING0]
		oTexture0=iTexture0;
	[RENDER_STATE]				-> similar to directx
		Blend					False
		Texture[0]				tex
		ColorOp[0]				Modulate
		ColorArg1[0]			Texture 
		ColorArg2[0]			Diffuse
		AlphaOp[0]				Modulate
		AlphaArg1[0]			Texture 
		AlphaArg2[0]			Diffuse
		ColorOp[1]				Disable
		AlphaOp[1]				Disable
		Cull					None
		ZBuffer					True
		ZBufferWrite			True
		TextureCoordinate[0]	TexCoo0
[END]

<BR><HR><BR>

<U> Exemple2:</U> <B> Environment mapping exemple </B>
<BR>

[VERTEXFORMAT]
	XYZ NORMAL TEX0
[DEFINES]
	CONST zero;
	CONST one=1.0,1.0,1.0,1.0;
	CONST half=0.5;
	VECTOR alpha;
	VECTOR color;
	MATRIX WORLD;
	MATRIX VIEWPROJ;
	TEXTURE tex;
	TEXTURE envmap;

[TECHNIQUE]	All cards
[PASS0]
	[POSITION]
		tPos=iPos*4WORLD;
		tNorm=iNorm*3WORLD;
		oPos=tPos*4VIEWPROJ;
	[DIFFUSE]
		c=color;
		oDiffuse=alpha*c;
		oDiffuse.w=one.x;
	[MAPPING0]
		oTexture1.x=iTexture0.x;
		oTexture1.y=iTexture0.y;
	[MAPPING1]
		oTexture1.x=half.x+half.x*tNorm.x;
		oTexture1.y=half.x+half.x*tNorm.y;
	[RENDER_STATE]
		ShadeMode				Smooth
		Texture[0]				tex
		Texture[1]				envmap
		ColorOp[0]				Modulate
		ColorArg1[0]			Texture
		ColorArg2[0]			Diffuse
		AlphaOp[0]				SelectArg2
		AlphaArg2[0]			Diffuse
		ColorOp[1]				Modulate
		ColorArg1[1]			Texture
		ColorArg2[1]			Current
		AlphaOp[1]				Disable
		Cull					Ccw
		ZBuffer					True
		ZBufferWrite			True
		TextureCoordinate[0]	TexCoo0
[END]

<BR><HR><BR>

<U> Exemple3:</U> <B> Extrude silhouette volume of a degenerated edges vertex buffer </B>
<BR>

[VERTEXFORMAT]
	XYZ NORMAL TEX0

[DEFINES]
	MATRIX WORLD;
	MATRIX VIEWPROJ;
	VECTOR longueur;
	VECTOR Light;
	VECTOR Decal;
	CONST zero=;
	CONST one=1.0,1.0,1.0,1.0;
	TEXTURE tex;
	
[TECHNIQUE]	All cards
[PASS0]
	[POSITION]
		tPos=iPos*4WORLD;
		tNorm=iNorm*3WORLD;
		u=tPos-Light;
		s=sge((u|tNorm),zero);
		u1=tPos+longueur*s*u*rsq(u|u);
		oPos=u1*4VIEWPROJ;
	[DIFFUSE]
		oDiffuse.x=zero.x;
		oDiffuse.y=zero.y;
		oDiffuse.z=zero.z;
		oDiffuse.w=one.w-s.w;
	[MAPPING0]
		oTexture0=iTexture0+Decal;
	[RENDER_STATE]
		ShadeMode				Smooth
		Blend					True
		SrcBlend				SrcAlpha
		DstBlend				InvSrcAlpha
		Texture[0]				tex
		ColorOp[0]				SelectArg2
		ColorArg1[0]			Texture 
		ColorArg2[0]			Diffuse
		AlphaOp[0]				SelectArg2
		AlphaArg2[0]			Diffuse
		ColorOp[1]				Disable
		AlphaOp[1]				Disable
		Cull					Ccw
		ZBuffer					True
		ZBufferWrite			False
		TextureCoordinate[0]	TexCoo0

[PASS1]
	[POSITION]
		tPos=iPos*4WORLD;
		tNorm=iNorm*3WORLD;
		u=tPos-Light;
		s=sge((u|tNorm),zero);
		u1=tPos+longueur*s*u*rsq(u|u);
		oPos=u1*4VIEWPROJ;
	[DIFFUSE]
		oDiffuse.x=zero.x;
		oDiffuse.y=zero.y;
		oDiffuse.z=zero.z;
		oDiffuse.w=one.w-s.w;
	[MAPPING0]
		oTexture0=iTexture0+Decal;
	[RENDER_STATE]
		ShadeMode				Smooth
		Blend					True
		SrcBlend				SrcAlpha
		DstBlend				InvSrcAlpha
		Texture[0]				tex
		ColorOp[0]				SelectArg2
		ColorArg1[0]			Texture 
		ColorArg2[0]			Diffuse
		AlphaOp[0]				SelectArg2
		AlphaArg2[0]			Diffuse
		ColorOp[1]				Disable
		AlphaOp[1]				Disable
		Cull					Cw
		ZBuffer					True
		ZBufferWrite			False
		TextureCoordinate[0]	TexCoo0
[END]


<BR><HR><BR>

<U> Exemple4:</U> <B> Glass Effect with approximation of refraction and reflection </B>
<BR>
	 
[VERTEXFORMAT]
 	XYZ NORMAL TEX0				// vertex format
	 
 [DEFINES]

 -> constantes diverses

 	CONST zero;
 	CONST one=1.0,1.0,1.0,1.0;
 	CONST koef=2.0,2.0,2.0,20;
 	CONST half=0.5,0.5,0.5,0.0;
 	CONST cst=0.15,0.15,0.15,0.0;
 	CONST half2=0.35,0.5,0.5,0.0;
 	CONST up=0.0,-1.0,0.0;
	 
  -> texture variables

 	TEXTURE Tex;
 	TEXTURE Phong;

  -> variables: MATRIX,VECTOR

 	MATRIX LIGHT;
 	MATRIX WORLD;
 	MATRIX VIEWPROJ;
 	VECTOR Camera;
	VECTOR Color;


  -> rendering technique definition

  [TECHNIQUE]	AllCards

  -> the following strings to [TECHNIQUE] can be : DOT3,MULTITEXTURE,SHADERS
  -> the techniques are declined to the correct uses of devices
  -> for example DOT3 doesn't exist on ATI Rage	  

  [PASS0]
 
  -> the program is divided by output vertex format
  -> (position,diffuse,texcoord0)

 	[POSITION]

  -> tPos,tN,N are variables translated to asm registers
  -> the variable number is limited
 
  -> notes: m4x4 v',v,A => v'=v*4A
  ->        m3x3 v',v,A => v'=v*3A
  -> 		cross product: v ^ v'
  -> 		dot product: (v | v')
  -> 		vertex shader/program functions
  -> 		cos,sin,tan,abs,exp,log,acos,asin,...
  -> 		(!) cos et sin need more 15 asm instructions


 		tPos=iPos*4WORLD;
 		tN=iNorm*3WORLD;
 		N=tN*3VIEWPROJ;
 		oPos=tPos*4VIEWPROJ;
 	[DIFFUSE]
 		oDiffuse=Color*(one-max(zero,N.z)) - cst;
 		oDiffuse.w=half2.x;
 	[MAPPING0]
 		oTexture0=iTexture0;

-> render states similar to D3D ones

 	[RENDER_STATE]
 		ShadeMode				Smooth
 		Texture[0]				Tex
 		Blend					True
 		SrcBlend				DstColor
 		DstBlend				Zero
 		ColorOp[0]				Modulate
 		ColorArg1[0]			Diffuse
 		ColorArg2[0]			Texture
 		AlphaOp[0]				Disable
 		AlphaOp[1]				Disable
 		ColorOp[1]				Disable
 		ZBuffer					True
 		ZBufferWrite			True
 		TextureCoordinate[0]	TexCoo0

 [PASS1]
 	[POSITION]
 		tPos=iPos*4WORLD;
 		tN=iNorm*3WORLD;
 		oPos=tPos*4VIEWPROJ;
 	[DIFFUSE]
 		oDiffuse=one;
 	[MAPPING0]
 		v=normalise(tPos-Camera);
 		u=normalise(tN+tN-v);
 		u=u*3LIGHT;
 		oTexture0=half + half*u;
 	[RENDER_STATE]
 		ShadeMode				Smooth
 		Blend					True
 		SrcBlend				One
 		DstBlend				One
 		Texture[0]				Phong
 		ColorOp[0]				SelectArg1
 		ColorArg1[0]			Texture
 		AlphaOp[0]				Disable
 		ColorOp[1]				Disable
 		AlphaOp[1]				Disable
 		ZBuffer					True
 		ZBufferWrite			True
 		TextureCoordinate[0]	TexCoo0
 [END]
	 

<B>usage:</B>

	Render.SetParams(API3D_CULL,CCW);

	vb->setTexture("Phong",n° texture envmap);
	vb->setTexture("Tex",n° texture);
	vb->setMatrixViewProj("VIEWPROJ");
	vb->setMatrixWorld("WORLD");
	vb->setMatrix("LIGHT",ML);
	vb->setVector("Camera",Camera);
	vb->setFloat("Color",1.0f);
	
	Render.SetVertexBuffer(vb);
	Render.DrawVertexBuffer();

  -> pour un multigroup:

  -> init:

	vb=Render.CreateMultiGroupFrom(...);
	vb->SetVertexProgram(script_ref);
	vb->resetAssignation();
	vb->setTextureAssociation("Tex",TEXTURE_FROM_NT);
	vb->setTextureAssociation("Phong",TEXTURE_FROM_NT2);

  -> drawing:

	vb->setMatrixViewProj("VIEWPROJ");
	vb->setMatrixWorld("WORLD");
	vb->setMatrix("LIGHT",ML);
	vb->setVector("Camera",Camera);

	vb->setFloat("Color",1.0f);
	
	Render.SetVertexBuffer(vb);
	Render.DrawVertexBuffer();


<BR><HR><BR>

<U> Exemple5:</U> <B> Dynamic Bump mapping with pixels shaders </B>
<BR>

 [VERTEXFORMAT]
 	XYZ NORMAL TEX0

 
 [DEFINES]
 	CONST zero;
 	CONST one=1.0,1.0,1.0,1.0;
 	CONST half=0.5,0.5,0.5,0.0;
 	CONST PI2=1.57;
 	CONST up=0.0,-1.0,0.0;
 	CONST up2=1.0,0.0,0.0;
 	CONST SMALLF=0.9;
	CONST tt1=-1.0,0.0,0.0;
	CONST tt2=1.0,0.0,0.0;
	 
 	TEXTURE Tex;
 	TEXTURE Bump;

 	MATRIX WORLD;
 	MATRIX REFLEC;
 	MATRIX VIEWPROJ;
 	VECTOR Ambient;
 	VECTOR Camera;
 	VECTOR Specular;
 	VECTOR Light;
 	VECTOR Color;
 	VECTOR Attenuation;
 	VECTOR Light2;
 	VECTOR Color2;
 	VECTOR Attenuation2;
 	CONST color1=0.0,0.0,0.0,0.0;

  [MACROS]
  Tangente(v)
  {
	s = (up | v);
	temp = tgt(up , v);
	if (s.x>SMALLF.x)
	{
		temp = tt1;
	}
	if (s.x<SMALLF.y)
	{
		temp = tt2;
	}
	temp;
  }
 	 
 [TECHNIQUE]	Shaders

 [PASS0]
	 
 	[POSITION]
 		tmp=iPos*4WORLD;
 		N=iNorm*3WORLD;
 		u=tmp-Light;
 		u2=tmp-Light2;
 		tmp=tmp*4REFLEC;
 		oPos=tmp*4VIEWPROJ;
 		tmp.x=inv(u)*Attenuation;
 		tmp.y=inv(u2)*Attenuation2;
 		u=normalise(u);
 		u2=normalise(u2);
 		T=Tangente(N);
 		T=normalise(T);
 		w=zero;
 		w2=zero;
 		w.x=(u|T);
 		w2.x=(u2|T);
 		T=N^T;
		w.y=(u|T);
 		w2.y=(u2|T);
 		w.z=(u|N);
 		w2.z=(u2|N);
 	[DIFFUSE]
 		N=zero-N;
 		T=Color*max((u|N)*tmp.x,zero);
 		T=T+Color2*max((u2|N)*tmp.y,zero);
 		oDiffuse=Ambient+half*T;
 	[SPECULAR]
 		oSpecular=half+half*w;
 	[MAPPING0]
 		oTexture0=iTexture0;
 	[MAPPING1]
 		oTexture1=half+half*w2;

   [SHADER]
 		Out=sample(0,Tex0);
 		bump=sample(1,Tex0);
 		tmp=coord(Tex1);
 		Out=Out*Diffuse;
 		tmp=dotbx2(bump,tmp);
 		bump=dotbx2(bump,Specular);
 		tmp=tmp+bump;
 		tmp=tmp*tmp;
 		Out=Out*tmp;

 	[RENDER_STATE]
 		Blend					False
		Texture[0]				Tex
 		Texture[1]				Bump
 		ZBuffer					True
 		ZBufferWrite			True
 		Mip[0]					Linear
 		Mip[1]					Linear

 [TECHNIQUE]	Dot3

 [PASS0]
	 
	[POSITION]
		tPos=iPos*4WORLD;
		N=iNorm*3WORLD;
 		u=tPos-Light;
 		u2=tPos-Light2;
 		tmp.x=inv(u)*Attenuation;
 		tmp.y=inv(u2)*Attenuation2;
 		u=normalise(u);
 		u2=normalise(u2);
 		tPos=tPos*4REFLEC;
 		oPos=tPos*4VIEWPROJ;
	 
 	[DIFFUSE]
 		N=zero-N;
 		tmp2=Ambient+Color*max((u|N)*tmp.x,zero);
 		oDiffuse=tmp2+Color2*max((u2|N)*tmp.y,zero);
	 
 	[MAPPING0]
 		oTexture0=iTexture0;

 	[RENDER_STATE]
 		ShadeMode				Smooth
		Blend					False
 		Texture[0]				Tex
 		ColorOp[0]				Modulate
 		ColorArg1[0]			Texture
 		ColorArg2[0]			Diffuse
 		AlphaOp[0]				Disable
 		ColorOp[1]				Disable
 		AlphaOp[1]				Disable
 		ZBuffer					True
 		ZBufferWrite			True
		TextureCoordinate[0]	TexCoo0
		Mip[0]					Linear
	 

 [PASS1]
	 
	[POSITION]
		tPos=iPos*4WORLD;
		N=iNorm*3WORLD;
		ttPos=tPos*4REFLEC;
		oPos=ttPos*4VIEWPROJ;
		u=normalise(tPos-Light);
		T=Tangente(N);
		T=normalise(T);
		w=zero;
		w.x=(u|T);
		T=N^T;
		w.y=(u|T);
		w.z=(u|N);
	 
	[DIFFUSE]
		oDiffuse=half - half.x*w;
	
	[MAPPING0]
		oTexture0=iTexture0;

	[RENDER_STATE]\n"
		ShadeMode				Smooth
		Blend					True
		SrcBlend				DstColor
		DstBlend				One
		Texture[0]				Bump
		ColorOp[0]				Dot3
		ColorArg1[0]			Texture
		ColorArg2[0]			Diffuse
		AlphaOp[0]				Disable
		ColorOp[1]				Disable
		AlphaOp[1]				Disable
		ZBuffer					True
		ZBufferWrite			False
		TextureCoordinate[0]	TexCoo0
		Mip[0]					Linear

 [TECHNIQUE]	All cards

 [PASS0]
	 
 	[POSITION]
 		tPos=iPos*4WORLD;
 		N.xyz=iNorm.xyz;
 		N.w=zero.x;
 		N=N*3WORLD;
 		u=normalise(Light-tPos);
 		ttPos=tPos*4REFLEC;
 		oPos=ttPos*4VIEWPROJ;
	 
 	[DIFFUSE]
 		oDiffuse=Ambient+Color*max((u|N),zero);
	 
 	[MAPPING0]
 		oTexture0=iTexture0;

 	[RENDER_STATE]
 		ShadeMode				Smooth
 		Blend					False
 		Texture[0]				Tex
 		ColorOp[0]				Modulate
 		ColorArg1[0]			Texture
 		ColorArg2[0]			Diffuse
 		AlphaOp[0]				Disable
 		ColorOp[1]				Disable
 		AlphaOp[1]				Disable
 		ZBuffer					True
 		ZBufferWrite			True
 		TextureCoordinate[0]	TexCoo0
 		Mip[0]					Linear
 [END]

*/