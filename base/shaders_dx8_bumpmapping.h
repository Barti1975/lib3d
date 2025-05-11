const char BUMP_MAPPING_LIGHTMAP_SH[] = 

    "TEXTURE tex;\n"
	"TEXTURE lightmap;\n"
	"TEXTURE vectors;\n"
	"TEXTURE bump;\n"
	"DWORD transparent;\n"

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"
					"texld r0,t0\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r3,t0\n"
					"dp3 r5,r3_bx2,r2_bx2\n"
					"mul r0.rgb,r0,r1\n"
					"mul_x2 r0.rgb,r0,r5\n"
					"+mul r0.a,1-r0.a,r5.r\n"
				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
            "Texture[3] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = Linear;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

    
		"}\n"

    "}\n"



    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"
    
			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" //Linear

			"TexCoordIndex[0] = 0;\n"
			"TexCoordIndex[1] = 1;\n"

		
			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

//			"ColorOp[0] = Modulate;\n"
			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

//			"AlphaOp[0]   = Modulate;\n"
			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

		"}\n"

        "pass P1\n"
        "{\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"

			"ZFunc = Equal;\n"
			"Texture[0] = <vectors>;\n"
            "Texture[1] = <bump>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"
            
			"AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"            
			"MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = Linear;\n"

			"TexCoordIndex[0] = 2;\n"
			"TexCoordIndex[1] = 0;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"
			

			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = DotProduct3;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

		"}\n"

    "}\n"

    "technique T2\n"
    "{\n"
        "pass P0\n"
        "{\n"
			
			"Texture[0] = <lightmap>;\n"
			"Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n"

			"TexCoordIndex[0] = 0;\n"
			"TexCoordIndex[1] = 1;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"
		

//			"ColorOp[0] = Modulate;\n"
			"ColorOp[0] = SelectArg1;\n"

            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate2x;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

//			"AlphaOp[0]   = Modulate;\n"
			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

		"}\n"
	"}\n"


	    
	"technique T3\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"

			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"
		"}\n"


		"pass P1\n"
        "{\n"

		"ZFunc = Equal;\n"
			"Texture[0] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"

			"TexCoordIndex[0] = 1;\n"

		"}\n"
    "}\n"

	"technique T4\n"
    "{\n"
        "pass P0\n"
        "{\n"
	
			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"


//			"ColorOp[0] = Modulate;\n"
			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

//			"AlphaOp[0]   = Modulate;\n"
			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"

		"}\n"
    "}\n"


	"technique T5\n"
    "{\n"
        "pass P0\n"
        "{\n"
	
			"AlphaBlendEnable = true;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"
			"ShadeMode = Flat;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = SelectArg2;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Disable;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

		"}\n"
    "}\n"

"\n";


const UINT len_BUMP_MAPPING_LIGHTMAP_SH = sizeof(BUMP_MAPPING_LIGHTMAP_SH)-1 ;

const char BUMP_MAPPING_LIGHTMAP_SH_s[] = 

	"VECTOR vecLight1;\n"
	"VECTOR vecLight2;\n"
	"VECTOR vecLight3;\n"

	"VECTOR vecVar1;\n"
	"VECTOR vecVar2;\n"
	"VECTOR vecVar3;\n"

    "TEXTURE tex;\n"
	"TEXTURE lightmap;\n"
	"TEXTURE vectors;\n"
	"TEXTURE bump;\n"

	"DWORD transparent;\n"

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"

					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0.x,r1.x\n"
					"mov oD0.y,r1.x\n"
					"mov oD0.z,r1.x\n"
					"mov oD0.w,c14.x\n"
					"mov oT0,v7\n"
					"mov oT1,v8\n"
					"mov oT2,v9\n"
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"
					
					"def c0,1.0,0.0,0.0,0.0\n"

					"texld r0,t0\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r3,t0\n"
					"sub r4.a,c0.r,r0.a\n"
					"dp3 r5,r3_bx2,r2_bx2\n"
					"mul r0,r0,r1\n"
					"mul r0,r0,v0\n"
					"mul_x2 r0,r0,r5\n"
					"mul r0.a,r4.a,r5.r\n"
				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
            "Texture[3] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = Linear;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

		"}\n"

    "}\n"

    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"

					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0.x,r1.x\n"
					"mov oD0.y,r1.x\n"
					"mov oD0.z,r1.x\n"
					"mov oD0.w,c14.x\n"


					"mov oT0,v7\n"
					"mov oT1,v8\n"
				"};\n"


			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" //Linear

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"


            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

		"}\n"

        "pass P1\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m4x4 oPos,r0,c4\n"
					"mov oD0,c14\n"
					"mov oT0,v9\n"
					"mov oT1,v7\n"
				"};\n"

			"ZFunc = Equal;\n"
			"Texture[0] = <vectors>;\n"
            "Texture[1] = <bump>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"
            
			"AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = Linear;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"
			

			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = DotProduct3;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"


		"}\n"

    "}\n"


    "technique T2\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"

					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0.x,r1.x\n"
					"mov oD0.y,r1.x\n"
					"mov oD0.z,r1.x\n"
					"mov oD0.w,c14.x\n"


					"mov oT0,v7\n"
					"mov oT1,v8\n"
				"};\n"
		
			"Texture[0] = <lightmap>;\n"
			"Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n"


			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"
		

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate2x;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

			

		"}\n"
	"}\n"


	"technique T3\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"
		"}\n"


		"pass P1\n"
        "{\n"

			"ZFunc = Equal;\n"
			"Texture[0] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"

			"TexCoordIndex[0] = 1;\n"

		"}\n"
    "}\n"
    

	
	"technique T4\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"

		"}\n"
    "}\n"
	
"\n";



const UINT len_BUMP_MAPPING_LIGHTMAP_SH_s = sizeof(BUMP_MAPPING_LIGHTMAP_SH_s)-1 ;


const char BUMP_MAPPING_LIGHTMAP_SH_D0[] = 

	"VECTOR vecLight1;\n"
	"VECTOR vecLight2;\n"
	"VECTOR vecLight3;\n"

	"VECTOR vecVar1;\n"
	"VECTOR vecVar2;\n"
	"VECTOR vecVar3;\n"

    "TEXTURE tex;\n"
	"TEXTURE lightmap;\n"
	"TEXTURE vectors;\n"
	"TEXTURE bump;\n"
	"TEXTURE displace;\n"
	"TEXTURE horizon;\n"


	"DWORD transparent;\n"

	"VECTOR Eye;\n"

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r8,v3,c0\n"

					"mov r4,c15\n"
					"sub r4,r4,r0\n"
					
					"dp3 r5,r4,r4\n"
					"rsq r5.w,r5\n"
					"mul r4,r4,r5.w\n"				// r8=N
					
					"mov r1,c16\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r6,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up=T

					"mov r1,c20\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r7,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up2=T



					"dp3 r9.x,r8,c16\n"
					"mov r9.y,-r9.x\n"
					"slt r9.z,r9.x,c21.x\n"
					"sge r9.w,r9.x,c21.x\n"

					"mul r9.x,r9.x,r9.w\n"
					"mad r9.x,r9.y,r9.z,r9.x\n"

					"add r9.x,c21.z,-r9.x\n"

					"slt r9.z,r9.x,c21.y\n"
					"sge r9.w,r9.x,c21.y\n"
					

					"mul r6,r6,r9.w\n"
					"mad r6,r7,r9.z,r6\n"

					"mul r5,r8.yzxw,r6.zxyw\n"
					"mad r7,-r8.zxyw,r6.yzxw,r5\n" // r7=tgt^N=B

					"dp3 r5.x,r4,r8\n"
					"dp3 r5.y,r4,r6\n"
					"dp3 r5.z,r4,-r7\n"
					"mov r5.w,c18.x\n"

					"dp3 r10.x,r4,r6\n"
					"dp3 r10.y,r4,r7\n"
					"dp3 r10.z,r4,r8\n"
					"mov r10.w,c18.x\n"


					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0,r1\n"

					
					"mov oT0,v7\n"
					"mov oT1,v8\n"
					"mov oT2,v9\n"
					

					"mul r10.y,r10.y,v10.x\n"
					"mov oT4.xyz,r10.xyz\n"		// eye in local ref

					"mov r5.x,r5.y\n"
					"mov r5.y,r5.z\n"
					"mul r5.x,r5.x,v10.x\n"
					"mul r5.y,r5.y,v10.y\n"
					
					"mov oT3.xyz,r5.xyz\n"		// eye in local ref
					
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"

					"def c0,0.04,0.04,0.04,0.04\n"
					
					"def c1,0.5,0.5,0.5,0.5\n"
					
					
					"texld r3,t0\n"
					"texcrd r4.rgb,t3.xyz\n"
					"texcrd r2.rgb,t4.xyz\n"
					"texcrd r5.rgb,t0.xyz\n"

					"mul r1.a,r3_bx2.a,c0\n"
					"mad r5.rg,r1.a,r4,r5\n"

					"mov r3.rgb,r2\n"

					"phase\n"

					"texld r0,r5\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r4,r5\n"
					
					"mul r0,r0,r1\n"
					"mul r0,r0,v0\n"
					"mad_x2 r5.rgb,c1,-r2_bx2,r4_bx2\n"
					"dp3_sat r3.rgb,r3,r5\n"
					"mul r3.rgb,r3,r3\n"
					"dp3 r1,r4_bx2,r2_bx2\n"
					"mul_x2 r0.rgb,r0,r1\n"
					"+mul r0.a,1-r0.a,r1.r\n"
					"mad r0.rgb,r0,r3,r0\n"
					

				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
			"Texture[3] = <displace>;\n"
			"Texture[4] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = None;\n" 

            "AddressU[4] = Wrap;\n"
            "AddressV[4] = Wrap;\n"
            "MinFilter[4] = Linear;\n"
            "MagFilter[4] = Linear;\n"
			"MipFilter[4] = None;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[15] = <Eye>;\n"
			"VertexShaderConstant1[16] = (0.0f,-1.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[17] = (0.5f,0.5f,0.5f,0.5f);\n"
			"VertexShaderConstant1[18] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[19] = (0.05f,0.05f,0.05f,0.0f);\n"
			"VertexShaderConstant1[20] = (0.0f,0.0f,-1.0f,0.0f);\n"
			"VertexShaderConstant1[21] = (0.0f,0.025f,1.0f,0.0f);\n"
			"VertexShaderConstant1[22] = (0.125f,0.0f,0.0f,0.0f);\n"

		"}\n"




        "pass P1\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r8,v3,c0\n"

					"mov r4,c15\n"
					"sub r4,r4,r0\n"
					
					"dp3 r5,r4,r4\n"
					"rsq r5.w,r5\n"
					"mul r4,r4,r5.w\n"				// r8=N
					
					"mov r1,c16\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r6,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up=T

					"mov r1,c20\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r7,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up2=T



					"dp3 r9.x,r8,c16\n"
					"mov r9.y,-r9.x\n"
					"slt r9.z,r9.x,c21.x\n"
					"sge r9.w,r9.x,c21.x\n"

					"mul r9.x,r9.x,r9.w\n"
					"mad r9.x,r9.y,r9.z,r9.x\n"

					"add r9.x,c21.z,-r9.x\n"

					"slt r9.z,r9.x,c21.y\n"
					"sge r9.w,r9.x,c21.y\n"
					

					"mul r6,r6,r9.w\n"
					"mad r6,r7,r9.z,r6\n"

					"mul r5,r8.yzxw,r6.zxyw\n"
					"mad r7,-r8.zxyw,r6.yzxw,r5\n" // r7=tgt^N=B

					"dp3 r5.x,r4,r8\n"
					"dp3 r5.y,r4,r6\n"
					"dp3 r5.z,r4,-r7\n"
					"mov r5.w,c18.x\n"

					"m4x4 oPos,r0,c4\n"

					"mov r1,c14\n"
					"mov oD0,r1\n"

					
					"mov oT0,v7\n"
					"mov oT1,v9\n"
					
					
					"mov r5.x,r5.y\n"
					"mov r5.y,r5.z\n"
					
					"mul r5.x,r5.x,v10.x\n"
					"mul r5.y,r5.y,v10.y\n"
					
					
					"mov oT2.xyz,r5.xyz\n"		// eye in local ref
					
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"
					

					"def c0,0.04,0.04,0.04,0.04\n"
					
					//"def c1,0.25,0.25,0.25,0.25\n"
					"def c1,0.15,0.15,0.15,0.15\n"
					"def c2,0.0,0.0,0.0,0.0\n"
					"def c3,1.0,1.0,1.0,1.0\n"
					"def c4,0.5,0.5,0.5,0.5\n"
					"def c5,0.0,0.0,0.07,0.0\n"
					
					"texld r2,t0\n"
					"texld r1,t1\n"

					"texcrd r4.rgb,t2.xyz\n"
					"texcrd r5.rgb,t0.xyz\n"

					"mul r1.a,r2_bx2.a,c0\n"
					"mad r5.rg,r1.a,r4,r5\n"

					"mov r1,-r1_bx2\n"
					"mov r1.b,-r1.r\n"

					"phase\n"

					"texld r0,r5\n"

					"mov r1.a,-r1.g\n"
					"sub r3,r1,r0\n"
					"cmp r3,r3,c3,c2\n"
					"mov r0.rgb,c5\n"
					"dp4 r0.a,c1,r3\n"
					

				"};\n"

			"Texture[0] = <horizon>;\n"
			"Texture[1] = <vectors>;\n"
			"Texture[2] = <displace>;\n"
			


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = None;\n" 

            "AddressU[4] = Wrap;\n"
            "AddressV[4] = Wrap;\n"
            "MinFilter[4] = Linear;\n"
            "MagFilter[4] = Linear;\n"
			"MipFilter[4] = None;\n" 

			"AlphaBlendEnable = true;\n" 
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[15] = <Eye>;\n"
			"VertexShaderConstant1[16] = (0.0f,-1.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[17] = (0.5f,0.5f,0.5f,0.5f);\n"
			"VertexShaderConstant1[18] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[19] = (0.05f,0.05f,0.05f,0.0f);\n"
			"VertexShaderConstant1[20] = (0.0f,0.0f,-1.0f,0.0f);\n"
			"VertexShaderConstant1[21] = (0.0f,0.025f,1.0f,0.0f);\n"
			"VertexShaderConstant1[22] = (0.125f,0.0f,0.0f,0.0f);\n"

		"}\n"

    "}\n"
/**/
	"\n";



const char BUMP_MAPPING_LIGHTMAP_SH_D1[] =

     "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r8,v3,c0\n"

					"mov r4,c15\n"
					"sub r4,r4,r0\n"
					
					"dp3 r5,r4,r4\n"
					"rsq r5.w,r5\n"
					"mul r4,r4,r5.w\n"				// r8=N
					
					"mov r1,c16\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r6,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up=T

					"mov r1,c20\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r7,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up2=T



					"dp3 r9.x,r8,c16\n"
					"mov r9.y,-r9.x\n"
					"slt r9.z,r9.x,c21.x\n"
					"sge r9.w,r9.x,c21.x\n"

					"mul r9.x,r9.x,r9.w\n"
					"mad r9.x,r9.y,r9.z,r9.x\n"

					"add r9.x,c21.z,-r9.x\n"

					"slt r9.z,r9.x,c21.y\n"
					"sge r9.w,r9.x,c21.y\n"
					

					"mul r6,r6,r9.w\n"
					"mad r6,r7,r9.z,r6\n"

					"mul r5,r8.yzxw,r6.zxyw\n"
					"mad r7,-r8.zxyw,r6.yzxw,r5\n" // r7=tgt^N=B

					"dp3 r5.x,r4,r8\n"
					"dp3 r5.y,r4,r6\n"
					"dp3 r5.z,r4,-r7\n"
					"mov r5.w,c18.x\n"

					"dp3 r10.x,r4,r6\n"
					"dp3 r10.y,r4,r7\n"
					"dp3 r10.z,r4,r8\n"
					"mov r10.w,c18.x\n"


					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0,r1\n"

					
					"mov oT0,v7\n"
					"mov oT1,v8\n"
					"mov oT2,v9\n"
					

					"mul r10.y,r10.y,v10.x\n"
					//"mov oT4.xyz,r10.xyz\n"		// eye in local ref

					"mov r5.x,r5.y\n"
					"mov r5.y,r5.z\n"
					"mul r5.x,r5.x,v10.x\n"
					"mul r5.y,r5.y,v10.y\n"
					
					"mov oT3.xyz,r5.xyz\n"		// eye in local ref
					
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"
		
					"def c0,0.04,0.04,0.04,0.04\n"
					
					
					"texld r3,t0\n"
					"texcrd r4.rgb,t3.xyz\n"
					"texcrd r5.rgb,t0.xyz\n"

					"mul r1.a,r3_bx2.a,c0\n"
					"mad r5.rg,r1.a,r4,r5\n"

					"phase\n"

					"texld r0,r5\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r4,r5\n"
					
					"mul r0,r0,r1\n"
					"mul r0,r0,v0\n"
					"dp3 r1,r4_bx2,r2_bx2\n"
					"mul_x2 r0.rgb,r0,r1\n"
					"+mul r0.a,1-r0.a,r1.r\n"
					

				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
			"Texture[3] = <displace>;\n"
			"Texture[4] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = Linear;\n" 

            "AddressU[4] = Wrap;\n"
            "AddressV[4] = Wrap;\n"
            "MinFilter[4] = Linear;\n"
            "MagFilter[4] = Linear;\n"
			"MipFilter[4] = Linear;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[15] = <Eye>;\n"
			"VertexShaderConstant1[16] = (0.0f,-1.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[17] = (0.5f,0.5f,0.5f,0.5f);\n"
			"VertexShaderConstant1[18] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[19] = (0.05f,0.05f,0.05f,0.0f);\n"
			"VertexShaderConstant1[20] = (0.0f,0.0f,-1.0f,0.0f);\n"
			"VertexShaderConstant1[21] = (0.0f,0.025f,1.0f,0.0f);\n"
			"VertexShaderConstant1[22] = (0.125f,0.0f,0.0f,0.0f);\n"

		"}\n"




        "pass P1\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r8,v3,c0\n"

					"mov r4,c15\n"
					"sub r4,r4,r0\n"
					
					"dp3 r5,r4,r4\n"
					"rsq r5.w,r5\n"
					"mul r4,r4,r5.w\n"				// r8=N
					
					"mov r1,c16\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r6,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up=T

					"mov r1,c20\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r7,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up2=T



					"dp3 r9.x,r8,c16\n"
					"mov r9.y,-r9.x\n"
					"slt r9.z,r9.x,c21.x\n"
					"sge r9.w,r9.x,c21.x\n"

					"mul r9.x,r9.x,r9.w\n"
					"mad r9.x,r9.y,r9.z,r9.x\n"

					"add r9.x,c21.z,-r9.x\n"

					"slt r9.z,r9.x,c21.y\n"
					"sge r9.w,r9.x,c21.y\n"
					

					"mul r6,r6,r9.w\n"
					"mad r6,r7,r9.z,r6\n"

					"mul r5,r8.yzxw,r6.zxyw\n"
					"mad r7,-r8.zxyw,r6.yzxw,r5\n" // r7=tgt^N=B

					"dp3 r5.x,r4,r8\n"
					"dp3 r5.y,r4,r6\n"
					"dp3 r5.z,r4,-r7\n"
					"mov r5.w,c18.x\n"

					"m4x4 oPos,r0,c4\n"

					"mov r1,c14\n"
					"mov oD0,r1\n"

					
					"mov oT0,v7\n"
					"mov oT1,v9\n"
					
					
					"mov r5.x,r5.y\n"
					"mov r5.y,r5.z\n"
					
					"mul r5.x,r5.x,v10.x\n"
					"mul r5.y,r5.y,v10.y\n"
					
					
					"mov oT2.xyz,r5.xyz\n"		// eye in local ref
					
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"

					"def c0,0.04,0.04,0.04,0.04\n"
					
					"def c1,0.15,0.15,0.15,0.15\n"
					"def c2,0.0,0.0,0.08,0.0\n"
					"def c3,1.0,1.0,1.0,1.0\n"
					
					"texld r2,t0\n"
					"texld r1,t1\n"

					"texcrd r4.rgb,t2.xyz\n"
					"texcrd r5.rgb,t0.xyz\n"

					"mul r1.a,r2_bx2.a,c0\n"
					"mad r5.rg,r1.a,r4,r5\n"

					"mov r1,-r1_bx2\n"
					"mov r1.b,-r1.r\n"

					"phase\n"

					"texld r0,r5\n"

					"mov r1.a,-r1.g\n"
					"sub r3,r1,r0\n"
					"cmp r3,r3,c3,c2\n"
					"mov r0.rgb,c2\n"
					"dp4 r0.a,c1,r3\n"
					

				"};\n"

			"Texture[0] = <horizon>;\n"
			"Texture[1] = <vectors>;\n"
			"Texture[2] = <displace>;\n"
			


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = Linear;\n" 


			"AlphaBlendEnable = true;\n" 
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[15] = <Eye>;\n"
			"VertexShaderConstant1[16] = (0.0f,-1.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[17] = (0.5f,0.5f,0.5f,0.5f);\n"
			"VertexShaderConstant1[18] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[19] = (0.05f,0.05f,0.05f,0.0f);\n"
			"VertexShaderConstant1[20] = (0.0f,0.0f,-1.0f,0.0f);\n"
			"VertexShaderConstant1[21] = (0.0f,0.025f,1.0f,0.0f);\n"
			"VertexShaderConstant1[22] = (0.125f,0.0f,0.0f,0.0f);\n"

		"}\n"

    "}\n"
/**/
	"\n";

const char BUMP_MAPPING_LIGHTMAP_SH_D2[] = 

    "technique T2\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r8,v3,c0\n"

					"mov r4,c15\n"
					"sub r4,r4,r0\n"
					
					"dp3 r5,r4,r4\n"
					"rsq r5.w,r5\n"
					"mul r4,r4,r5.w\n"				// r8=N
					
					"mov r1,c16\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r6,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up=T

					"mov r1,c20\n"
					"mul r5,r1.yzxw,r8.zxyw\n"
					"mad r7,-r1.zxyw,r8.yzxw,r5\n" // r6=N^up2=T



					"dp3 r9.x,r8,c16\n"
					"mov r9.y,-r9.x\n"
					"slt r9.z,r9.x,c21.x\n"
					"sge r9.w,r9.x,c21.x\n"

					"mul r9.x,r9.x,r9.w\n"
					"mad r9.x,r9.y,r9.z,r9.x\n"

					"add r9.x,c21.z,-r9.x\n"

					"slt r9.z,r9.x,c21.y\n"
					"sge r9.w,r9.x,c21.y\n"
					

					"mul r6,r6,r9.w\n"
					"mad r6,r7,r9.z,r6\n"

					"mul r5,r8.yzxw,r6.zxyw\n"
					"mad r7,-r8.zxyw,r6.yzxw,r5\n" // r7=tgt^N=B

					"dp3 r5.x,r4,r8\n"
					"dp3 r5.y,r4,r6\n"
					"dp3 r5.z,r4,-r7\n"
					"mov r5.w,c18.x\n"

					"dp3 r10.x,r4,r6\n"
					"dp3 r10.y,r4,r7\n"
					"dp3 r10.z,r4,r8\n"
					"mov r10.w,c18.x\n"


					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0,r1\n"

					
					"mov oT0,v7\n"
					"mov oT1,v8\n"
					"mov oT2,v9\n"
					

					"mul r10.y,r10.y,v10.x\n"
					"mov oT4.xyz,r10.xyz\n"		// eye in local ref

					"mov r5.x,r5.y\n"
					"mov r5.y,r5.z\n"
					"mul r5.x,r5.x,v10.x\n"
					"mul r5.y,r5.y,v10.y\n"
					
					"mov oT3.xyz,r5.xyz\n"		// eye in local ref
					
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"

					"def c0,0.04,0.04,0.04,0.04\n"
					
					
					
					"texld r3,t0\n"
					"texcrd r4.rgb,t3.xyz\n"
					"texcrd r2.rgb,t4.xyz\n"
					"texcrd r5.rgb,t0.xyz\n"

					"mul r1.a,r3_bx2.a,c0\n"
					"mad r5.rg,r1.a,r4,r5\n"

					"phase\n"

					"texld r0,r5\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r4,r5\n"
					
					"mul r0,r0,r1\n"
					"mul r0,r0,v0\n"
					"dp3 r1,r4_bx2,r2_bx2\n"
					"mul_x2 r0.rgb,r0,r1\n"
					"+mul r0.a,1-r0.a,r1.r\n"
					

				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
			"Texture[3] = <displace>;\n"
			"Texture[4] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = Linear;\n" 

            "AddressU[4] = Wrap;\n"
            "AddressV[4] = Wrap;\n"
            "MinFilter[4] = Linear;\n"
            "MagFilter[4] = Linear;\n"
			"MipFilter[4] = Linear;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[15] = <Eye>;\n"
			"VertexShaderConstant1[16] = (0.0f,-1.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[17] = (0.5f,0.5f,0.5f,0.5f);\n"
			"VertexShaderConstant1[18] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[19] = (0.05f,0.05f,0.05f,0.0f);\n"
			"VertexShaderConstant1[20] = (0.0f,0.0f,-1.0f,0.0f);\n"
			"VertexShaderConstant1[21] = (0.0f,0.025f,1.0f,0.0f);\n"
			"VertexShaderConstant1[22] = (0.125f,0.0f,0.0f,0.0f);\n"

		"}\n"



    "}\n"

	"\n";

const char BUMP_MAPPING_LIGHTMAP_SH_D3[] = 

    "technique T3\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"

					"m4x4 r0,v0,c0\n"

					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0,r1\n"
					"mov oT0,v7\n"
					"mov oT1,v8\n"
					"mov oT2,v9\n"
				"};\n"

			"PixelShader =\n"
                "asm\n"
				"{\n"
					"ps.1.4\n"
					
					"def c0,1.0,0.0,0.0,0.0\n"

					"texld r0,t0\n"
					"texld r1,t1\n"
					"texld r2,t2\n"
					"texld r3,t0\n"
					"sub r4.a,c0.r,r0.a\n"
					"dp3 r5,r3_bx2,r2_bx2\n"
					"mul r0,r0,r1\n"
					"mul r0,r0,v0\n"
					"mul_x2 r0,r0,r5\n"
					"mul r0.a,r4.a,r5.r\n"
				"};\n"

			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"
			"Texture[2] = <vectors>;\n"
            "Texture[3] = <bump>;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" 


            "AddressU[2] = Wrap;\n"
            "AddressV[2] = Wrap;\n"
            "MinFilter[2] = Linear;\n"
            "MagFilter[2] = Linear;\n"
			"MipFilter[2] = None;\n" 


            "AddressU[3] = Wrap;\n"
            "AddressV[3] = Wrap;\n"
            "MinFilter[3] = Linear;\n"
            "MagFilter[3] = Linear;\n"
			"MipFilter[3] = Linear;\n" 

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = InvSrcAlpha;\n"
			"DestBlend = SrcAlpha;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

		"}\n"

    "}\n"


    "technique T4\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
				"{\n"
					"vs.1.1\n"
					"m4x4 r0,v0,c0\n"

					"add r1,c8,-r0\n"
					"add r2,c9,-r0\n"
					"add r3,c10,-r0\n"
					"dp3 r1,r1,r1\n"
					"dp3 r2,r2,r2\n"
					"dp3 r3,r3,r3\n"
					"rsq r1.w,r1\n"
					"rsq r2.w,r1\n"
					"rsq r3.w,r1\n"
					"mul r1,r1.w,c11\n"
					"mul r2,r2.w,c12\n"
					"mul r3,r3.w,c13\n"
					"add r1,r1,r2\n"
					"add r1,r1,r3\n"
					"add r1,c14,-r1\n"
					"mov r1.w,c14.x\n"

					"m4x4 oPos,r0,c4\n"
					"mov oD0,r1\n"
					"mov oT0,v7\n"
					"mov oT1,v8\n"
				"};\n"


			"Texture[0] = <lightmap>;\n"
            "Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n" //Linear

			"AlphaBlendEnable = <transparent>;\n" 
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"


            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

		"}\n"

        "pass P1\n"
        "{\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  
					"float v3[3];\n"  
					"d3dcolor v5[1];\n"
					"float v7[2];\n"  
					"float v8[2];\n"  
					"float v9[2];\n"  
					"float v10[2];\n"  
					
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"
					"m4x4 r0,v0,c0\n"
					"m4x4 oPos,r0,c4\n"
					"mov oD0,c14\n"
					"mov oT0,v9\n"
					"mov oT1,v7\n"
				"};\n"

			"ZFunc = Equal;\n"
			"Texture[0] = <vectors>;\n"
            "Texture[1] = <bump>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = None;\n"
            
			"AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = Linear;\n"	//Linear

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"
			

			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = DotProduct3;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"


		"}\n"

    "}\n"

    "technique T5\n"
    "{\n"
        "pass P0\n"
        "{\n"
            
			"Texture[0] = <lightmap>;\n"
			"Texture[1] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
			"MipFilter[1] = None;\n"

			"TexCoordIndex[0] = 0;\n"
			"TexCoordIndex[1] = 1;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"
		

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

			

		"}\n"
	"}\n"

    
	"technique T6\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"


            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"
			

		"}\n"


		"pass P1\n"
        "{\n"

			"ZFunc = Equal;\n"
			"Texture[0] = <tex>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = SrcColor;\n"

			"TexCoordIndex[0] = 1;\n"

            "VertexShaderConstant4[0]  = <matWorld>;\n"
            "VertexShaderConstant4[4]  = <matViewProj>;\n"
			"VertexShaderConstant1[8]  = <vecLight1>;\n"
			"VertexShaderConstant1[9]  = <vecLight2>;\n"
			"VertexShaderConstant1[10] = <vecLight3>;\n"
			"VertexShaderConstant1[11] = <vecVar1>;\n"
			"VertexShaderConstant1[12] = <vecVar2>;\n"
			"VertexShaderConstant1[13] = <vecVar3>;\n"
			"VertexShaderConstant1[14] = (1.0f,1.0f,1.0f,1.0f);\n"

		"}\n"
    "}\n"

	
	"technique T7\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <lightmap>;\n"

			"AlphaBlendEnable = <transparent>;\n"
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"


            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"MipFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = 0;\n"

		"}\n"
    "}\n"

"\n";
