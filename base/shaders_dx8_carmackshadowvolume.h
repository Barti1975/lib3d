const char SHADOW_VOLUME_SH_CarmackReverse1[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"

    "technique T0\n"
    "{\n"


        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"
			"CullMode = Ccw;\n"
			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Incr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  // position
                    "float v3[3];\n"  // normal
					"float v7[2];\n"  // mapping
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"
                    "m4x4 r0, v0, c0\n"
                    "m3x3 r1, v3, c0\n"
					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"

					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"
				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[10] = <Len>;\n"

        "}\n"

        "pass P1\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Cw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Keep;\n"
			"StencilZFail = Decr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  // position
                    "float v3[3];\n"  // normal
					"float v7[2];\n"  // mapping
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"
                    "m4x4 r0, v0, c0\n"
                    "m3x3 r1, v3, c0\n"
					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"


				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			
        "}\n"
	"}\n";


const char SHADOW_VOLUME_SH_CarmackReverse2[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"

    "technique T0\n"
    "{\n"


		
        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Cw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"
			
			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Incr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  // position
                    "float v3[3];\n"  // normal
					"float v7[2];\n"  // mapping
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

                    "m4x4 r0, v0, c0\n"
                    "m3x3 r1, v3, c0\n"
					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c10\n"
					"add r5,c11,-r5\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c9\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = <Len>;\n"
			"VertexShaderConstant1[10] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[11] = (1.0f,1.0f,1.0f,1.0f);\n"
			
        "}\n"
		
        "pass P1\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Ccw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"
			
			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Decr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                    "float v0[3];\n"  // position
                    "float v3[3];\n"  // normal
					"float v7[2];\n"  // mapping
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

                    "m4x4 r0, v0, c0\n"
                    "m3x3 r1, v3, c0\n"
					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c10\n"
					"add r5,c11,-r5\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c9\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = <Len>;\n"
			"VertexShaderConstant1[10] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[11] = (1.0f,1.0f,1.0f,1.0f);\n"
			
        "}\n"

		
	"}\n";

	
const UINT len_SHADOW_VOLUME_SH_CarmackReverse1 = sizeof(SHADOW_VOLUME_SH_CarmackReverse1)-1 ;
const UINT len_SHADOW_VOLUME_SH_CarmackReverse2 = sizeof(SHADOW_VOLUME_SH_CarmackReverse2)-1 ;

const char SHADOW_VOLUME_SH_CarmackReverse1m[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	"VECTOR Interpolant;\n"

    "technique T0\n"
    "{\n"


        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"
			"CullMode = Ccw;\n"
			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Incr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
					"stream 0;\n"
                    "float v0[3];\n"  // position
                    "float v3[3];\n"  // normal
					"float v7[2];\n"  // mapping
					"stream 1;\n"
                    "float v1[3];\n"  // position
                    "float v4[3];\n"  // normal
					"float v8[2];\n"  // mapping
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"mov r3,c19\n"
					"sub r3,r3,c20\n"


                    "m4x4 r0,v0,c0\n"
					"m4x4 r2,v1,c0\n"
					"mul r0,r0,r3\n"
					"mad r0,r2,c20,r0\n"

                    "m3x3 r1,v3,c0\n"
					"mov r1.w,c21.x\n"
                    "m3x3 r2,v4,c0\n"
					"mov r2.w,c21.x\n"
					"mul r1,r1,r3\n"
					"mad r1,r2,c20,r1\n"


				
					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"

					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"
				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[10] = <Len>;\n"

			"VertexShaderConstant1[19] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[20] = <Interpolant>;\n"
			"VertexShaderConstant1[21] = (0.0f,0.0f,0.0f,0.0f);\n"

        "}\n"

        "pass P1\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Cw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Keep;\n"
			"StencilZFail = Decr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
					"stream 0;\n"
                    "float v0[3];\n"
                    "float v3[3];\n"
					"float v7[2];\n"
					"stream 1;\n"
                    "float v1[3];\n"
                    "float v4[3];\n"
					"float v8[2];\n"

                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"


					"mov r3,c19\n"
					"sub r3,r3,c20\n"



                    "m4x4 r0,v0,c0\n"
					"m4x4 r2,v1,c0\n"
					"mul r0,r0,r3\n"
					"mad r0,r2,c20,r0\n"

                    "m3x3 r1,v3,c0\n"
					"mov r1.w,c21.x\n"
                    "m3x3 r2,v4,c0\n"
					"mov r2.w,c21.x\n"
					"mul r1,r1,r3\n"
					"mad r1,r2,c20,r1\n"

					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"


				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[10] = <Len>;\n"

			"VertexShaderConstant1[19] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[20] = <Interpolant>;\n"
			"VertexShaderConstant1[21] = (0.0f,0.0f,0.0f,0.0f);\n"
			
        "}\n"
	"}\n";


const char SHADOW_VOLUME_SH_CarmackReverse2m[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
    "VECTOR Interpolant;\n"

    "technique T0\n"
    "{\n"


		
        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Cw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"
			
			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Incr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
					"stream 0;\n"
                    "float v0[3];\n"
                    "float v3[3];\n"
					"float v7[2];\n"
					"stream 1;\n"
                    "float v1[3];\n"
                    "float v4[3];\n"
					"float v8[2];\n"

                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"mov r3,c19\n"
					"sub r3,r3,c20\n"

                    "m4x4 r0,v0,c0\n"
					"m4x4 r2,v1,c0\n"
					"mul r0,r0,r3\n"
					"mad r0,r2,c20,r0\n"

                    "m3x3 r1,v3,c0\n"
					"mov r1.w,c21.x\n"
                    "m3x3 r2,v4,c0\n"
					"mov r2.w,c21.x\n"
					"mul r1,r1,r3\n"
					"mad r1,r2,c20,r1\n"




					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c10\n"
					"add r5,c11,-r5\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c9\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = <Len>;\n"
			"VertexShaderConstant1[10] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[11] = (1.0f,1.0f,1.0f,1.0f);\n"

			"VertexShaderConstant1[19] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[20] = <Interpolant>;\n"
			"VertexShaderConstant1[21] = (0.0f,0.0f,0.0f,0.0f);\n"
			
        "}\n"
		
        "pass P1\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"CullMode = Ccw;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"ZWriteEnable = False;\n"
			"ZEnable = True;\n"

			"ShadeMode = Flat;\n"
			
			"StencilEnable = True;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			
			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Keep;\n"
			"StencilZFail = Decr;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
					"stream 0;\n"
                    "float v0[3];\n"
                    "float v3[3];\n"
					"float v7[2];\n"
					"stream 1;\n"
                    "float v1[3];\n"
                    "float v4[3];\n"
					"float v8[2];\n"

                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"mov r3,c19\n"
					"sub r3,r3,c20\n"


                    "m4x4 r0,v0,c0\n"
					"m4x4 r2,v1,c0\n"
					"mul r0,r0,r3\n"
					"mad r0,r2,c20,r0\n"

                    "m3x3 r1,v3,c0\n"
					"mov r1.w,c21.x\n"
                    "m3x3 r2,v4,c0\n"
					"mov r2.w,c21.x\n"
					"mul r1,r1,r3\n"
					"mad r1,r2,c20,r1\n"


					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c10\n"
					"add r5,c11,-r5\n"
					"mul r7,r5,r2\n"
					"mul r7,r7,c9\n"
					"add r0,r7,r0\n"
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[9] = <Len>;\n"
			"VertexShaderConstant1[10] = (0.0f,0.0f,0.0f,0.0f);\n"
			"VertexShaderConstant1[11] = (1.0f,1.0f,1.0f,1.0f);\n"

			"VertexShaderConstant1[19] = (1.0f,1.0f,1.0f,1.0f);\n"
			"VertexShaderConstant1[20] = <Interpolant>;\n"
			"VertexShaderConstant1[21] = (0.0f,0.0f,0.0f,0.0f);\n"
			
        "}\n"

		
	"}\n";

	
const UINT len_SHADOW_VOLUME_SH_CarmackReverse1m = sizeof(SHADOW_VOLUME_SH_CarmackReverse1m)-1 ;
const UINT len_SHADOW_VOLUME_SH_CarmackReverse2m = sizeof(SHADOW_VOLUME_SH_CarmackReverse2m)-1 ;

