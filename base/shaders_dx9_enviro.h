const char ENV_VSH[] = 

    "TEXTURE texMap;\n"
	"TEXTURE texEnvMap;\n"
    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    "MATRIX matLight;\n"
    "VECTOR vecLight;\n"
	"VECTOR posCamera;\n"

    "technique T0\n"
    "{\n"
        "pass P00\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"
		
            "Texture[1] = <texMap>;\n"
			"Texture[0] = <texEnvMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Add;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"

			"AlphaOp[1]   = SelectArg1;\n"
			"AlphaArg1[1] = Current;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"

                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					// r1 = N
                    "m3x3 r1, v3, c0\n"
					"mov r1.w,c12.w\n"
					// TEX1 = coo mapping
                    "mov oT1.xy, v7.xy\n"
					"mov r6,-r1\n"
					"m3x3 r8,r6,c8\n"
					"mov r8.w,c12.w\n"
					"mad r9.x,r8.x,c13.x,c13.w\n"
					"mad r9.y,r8.y,c13.x,c13.w\n"
					"mov oT0.xy, r9.xy\n"
					"dp3 r5,-r1,c16\n"
					"mov oD0,r5\n"
					"mov oD0.w,c12.x\n"
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n"
				
                "};\n"

        "}\n"
    "}\n"

    "technique T1\n"
    "{\n"

        "pass P0\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"

			"Texture[0] = <texMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"

			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"

			"AlphaOp[1]   = SelectArg1;\n"
			"AlphaArg1[1] = Current;\n"


			"AlphaBlendEnable = True;\n"        
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"
					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"

					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					// TEX1 = coo mapping
                    "mov oT0.xy, v7.xy\n"
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n" 
				
                "};\n"

        "}\n"



        "pass P1\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"

			"Texture[0] = <texEnvMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"


			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = True;\n"        
			"SrcBlend = One;\n"
			"DestBlend = One;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"
					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"


					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					// r1 = N
                    "m3x3 r1, v3, c0\n"
					"mov r1.w,c12.w\n"
					// TEX1 = coo mapping
//                    "mov oT1.xy, v7.xy\n"
					"mov r6,-r1\n"
					"m3x3 r8,r6,c8\n"
					"mov r8.w,c12.w\n"
					"mad r9.x,r8.x,c13.x,c13.w\n"
					"mad r9.y,r8.y,c13.x,c13.w\n"
					"mov oT0.xy, r9.xy\n"
					"dp3 r5,-r1,c16\n"
					"mov oD0,r5\n"
					"mov oD0.w,c12.x\n"
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n"
				
                "};\n"

        "}\n"

		
		
	"}\n";


const UINT len_ENV_VSH = sizeof(ENV_VSH)-1 ;




const char ENV2_VSH[] = 

    "TEXTURE texMap;\n"
	"TEXTURE texEnvMap;\n"
    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    "MATRIX matLight;\n"
    "VECTOR vecLight;\n"
	"VECTOR vecLight2;\n"
	"VECTOR posCamera;\n"
	"VECTOR ambient;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"
			"VertexShaderConstant1[17] = <ambient>;\n"
			"VertexShaderConstant1[19] = <vecLight2>;\n"
		
            "Texture[1] = <texEnvMap>;\n"
			"Texture[0] = <texMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Add;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"

			"AlphaOp[1]   = SelectArg1;\n"
			"AlphaArg1[1] = Current;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"
					"def c18,48.1f,1.0f,-1.0f,0.0f\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					// r1 = N
                    "m3x3 r1, v3, c0\n"
					"mov r1.w,c12.w\n"
					// TEX1 = coo mapping
                    "mov oT0.xy, v7.xy\n"
					
					"dp3 r7,-r1,c16\n"
					"dp3 r10,-r1,c19\n"

					
					
					"mov r2,c14\n"
					"add r2,r0,-r2\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3\n"
					"mul r2,r2,r3\n"

					"add r4,r2,-r1\n"
					"add r4,r4,-r1\n"



					"dp3 r5.w,r4,r4\n"
					"rsq r5,r5\n"
					"mul r9,r4,r5\n"
				
					"m3x3 r8,-r9,c8\n"
					"mov r8.w,c12.w\n"
					

					"min r8.x,r8.x,c18.y\n"
					"max r8.x,r8.x,c18.z\n"

					"min r8.y,r8.y,c18.y\n"
					"max r8.y,r8.y,c18.z\n"
					
					"mad r9.x,-r8.x,c13.x,c13.w\n"
					"mad r9.y,r8.y,c13.x,c13.w\n"
					"mov oT1.xy, r9.xy\n"
					
					"max r7,r7,c17\n"
					"max r10,r10,c17\n"
					"add r7,r10,r7\n"
					"min r7,r7,c15\n"
					"mov oD0,r7\n"
					"mov oD0.w,c12.x\n"
					
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n"
				
                "};\n"

			
        "}\n"
    "}\n"

    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"
			"VertexShaderConstant1[17] = <ambient>;\n"
			"VertexShaderConstant1[18] = <vecLight2>;\n"

			"Texture[0] = <texMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"

			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = Disable;\n"

			"AlphaBlendEnable = True;\n"        
			"SrcBlend = SrcAlpha;\n"
			"DestBlend = InvSrcAlpha;\n"



            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"

					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					"m3x3 r1, v3, c0\n"
					"mov r1.w,c12.w\n"
					// TEX1 = coo mapping
                    "mov oT0.xy, v7.xy\n"


					"dp3 r7,-r1,c16\n"
					"dp3 r10,-r1,c18\n"
					"max r7,r7,c17\n"
					"max r10,r10,c17\n"
					"add r7,r10,r7\n"
					"min r7,r7,c15\n"

					"mov oD0,r7\n"
					"mov oD0.w,c12.x\n"
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n" 
				
                "};\n"

			
        "}\n"


        "pass P1\n"
        "{\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant4[8] = <matLight>;\n"
			"VertexShaderConstant1[14] = <posCamera>;\n"
			"VertexShaderConstant1[16] = <vecLight>;\n"

			"Texture[0] = <texEnvMap>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"

			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Texture;\n"

			"AlphaOp[1]   = SelectArg1;\n"
			"AlphaArg1[1] = Current;\n"

			"AlphaBlendEnable = True;\n"        
			"SrcBlend = One;\n"
			"DestBlend = One;\n"


            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"
					"vs.1.1\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord  v7\n"

					"def c12,1.0f,0.0f,0.0f,0.0f\n"
					"def c13,0.5f,0.75f,0.0f,0.5f\n"
					"def c15,1.0f,1.0f,1.0f,1.0f\n"

					// r0=p*Mworld
                    "m4x4 r0, v0, c0\n"
					// r1 = N
                    "m3x3 r1, v3, c0\n"
					"mov r1.w,c12.w\n"
					// TEX1 = coo mapping
					
					"mov r2,c14\n"
					"add r2,r0,-r2\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3\n"
					"mul r2,r2,r3\n"

					"add r4,r2,-r1\n"
					"add r4,r4,-r1\n"

					"dp3 r5.w,r4,r4\n"
					"rsq r5,r5\n"
					"mul r9,r4,r5\n"


					"m3x3 r8,-r9,c8\n"
					"mov r8.w,c12.w\n"


					"mad r9.x,-r8.x,c13.x,c13.w\n"
					"mad r9.y,r8.y,c13.x,c13.w\n"

					"mov oT0.xy, r9.xy\n"
					// oPos = p*Mworld*Mviewproj
					"m4x4 oPos, r0, c4\n"
				
                "};\n"

        "}\n"
		
		
	"}\n";


const UINT len_ENV2_VSH = sizeof(ENV2_VSH)-1 ;


