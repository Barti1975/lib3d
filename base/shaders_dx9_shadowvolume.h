////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// normal
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SH[] = 

	"MATRIX matView;\n"
    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matProjNearPlane;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	"VECTOR ZNear;\n"

    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"			

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"
	
			"ShadeMode = Flat;\n"
			"ZFunc = Less;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"TwoSidedStencilMode = True;\n"
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"

			"Ccw_StencilPass = Decr;\n"
			"Ccw_StencilZFail = Keep;\n"
			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"

			"CullMode = None;\n"
			

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.005,0.005,0.005,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"
    "}\n"
	


    "technique T1\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

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
	
			"ShadeMode = Flat;\n"
			"ZFunc = Less;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.005,0.005,0.005,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

        "pass P1\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

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
			"ShadeMode = Flat;\n"
			"ZFunc = Less;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.005,0.005,0.005,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"

        "}\n"
	"}\n";


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// normal
// secondary vertex buffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SHc[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matView;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	"MATRIX matProjNearPlane;\n"

	"VECTOR ZNear;\n"


    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"
			

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"TwoSidedStencilMode = True;\n"
			
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"
			"Ccw_StencilPass = Decr;\n"
			"Ccw_StencilZFail = Keep;\n"

			"CullMode = None;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					
					"slt r5,r4,c9\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"
					
					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"


				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"
    "}\n"



    "technique T1\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

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
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					
					"slt r5,r4,c9\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"
					
					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"


				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

        "pass P1\n"
        "{\n"

			"ClipPlaneEnable = 0;\n"

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
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"


					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.1,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"

					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					
					"slt r5,r4,c9\n"

					"mul r7,r2,c10\n"
					"mul r7,r7,r5\n"
					"add r0,r7,r0\n"
			
					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"


				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

	"}\n";


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// normal - no artefact
// secondary vertex buffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SHcn[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matView;\n"
    "MATRIX matProjNearPlane;\n"
    "VECTOR Light;\n"
    "VECTOR Len;\n"

    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"
	

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"TwoSidedStencilMode = True;\n"

			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"
			"Ccw_StencilPass = Decr;\n"
			"Ccw_StencilZFail = Keep;\n"

			"CullMode = None;\n"			

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"

					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			
        "}\n"
    "}\n"

    "technique T1\n"
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
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"

					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			
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
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"


					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"slt r5,r4,c9\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"
					

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			
        "}\n"

	"}\n";



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// normal - no artefact
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SHn[] = 

	"MATRIX matView;\n"
    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matProjNearPlane;\n"
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	

    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"
			

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"TwoSidedStencilMode = True;\n"
			
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"
			"Ccw_StencilPass = Decr;\n"
			"Ccw_StencilZFail = Keep;\n"

			"CullMode = None;\n"			

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"
					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"


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
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			
        "}\n"
    "}\n"

    "technique T1\n"
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
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"
					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"


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
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			
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
			"ZFunc = Less;\n"


			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"
					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c13,0.0,0.0,0.0,0.0\n"
					"def c14,0.0,0.5,0.0,0.0\n"
					"def c15,0.01,0.01,0.01,0.0\n"

					"dcl_position v0\n"
					"dcl_normal v3\n"
					"dcl_texcoord v7\n"

					"m4x4 r0,v0,c0\n"
					"m3x3 r1,v3,c0\n"
					"mov r1.w,c9.x\n"

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
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"

        "}\n"
	"}\n";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// morphing
// secondary vertex buffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SHmc[] = 

    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matView;\n"
	"MATRIX matProjNearPlane;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	"VECTOR Interpolant;\n"

	"VECTOR ZNear;\n"

    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"
			

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"

			"TwoSidedStencilMode = True;\n"
			
			
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"
			"Ccw_StencilPass = Incr;\n"
			"Ccw_StencilZFail = Keep;\n"

			"CullMode = None;\n"			

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"

					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"



					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c9\n"

					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"					

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"
    "}\n"

    "technique T1\n"
    "{\n"

        "pass P0\n"
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
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"

					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"



					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c9\n"

					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"					

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

        "pass P1\n"
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
			"ZFunc = Less;\n"
	
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"

					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"


					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"
					"sge r5,r4,c9\n"

					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

	"}\n"
"\n";


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// morphing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char SHADOW_VOLUME_SHm[] = 

	"MATRIX matView;\n"
    "MATRIX matWorld;\n"
    "MATRIX matViewProj;\n"
	"MATRIX matProjNearPlane;\n"
    
    "VECTOR Light;\n"
    "VECTOR Len;\n"
	"VECTOR Interpolant;\n"
	"VECTOR ZNear;\n"

    "technique T0\n"
    "{\n"

        "pass P0\n"
        "{\n"

			"ColorOp[0] = Disable;\n"
			"AlphaOp[0] = Disable;\n"			

			"AlphaBlendEnable = True;\n"
			"SrcBlend = Zero;\n"
			"DestBlend = One;\n"

			"StencilEnable = True;\n"
			
			"ZWriteEnable = False;\n"

			"ZFunc = Less;\n"
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"TwoSidedStencilMode = True;\n"
			
			"StencilFunc = Always;\n"
			"StencilFail = Keep;\n"
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

			"Ccw_StencilFunc = Always;\n"
			"Ccw_StencilFail = Keep;\n"
			"Ccw_StencilPass = Incr;\n"
			"Ccw_StencilZFail = Keep;\n"

			"CullMode = None;\n"			

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"


					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"



					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"

					"sge r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"
					
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"
    "}\n"

    "technique T1\n"
    "{\n"

        "pass P0\n"
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

			"ZFunc = Less;\n"
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			
			"StencilPass = Decr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"


					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"



					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"

					"sge r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"
					
					"m4x4 oPos, r0, c4\n"

				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"
			
        "}\n"

        "pass P1\n"
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

			"ZFunc = Less;\n"
			"ZWriteEnable = False;\n"
			"ShadeMode = Flat;\n"

			"StencilRef = 0x1;\n"
			"StencilMask = 0xffffffff;\n"
			"StencilWriteMask = 0xffffffff;\n"
			
			"StencilPass = Incr;\n"
			"StencilZFail = Keep;\n"

            "VertexShader =\n"
                "decl\n"
                "{\n"
                "}\n"
                "asm\n"
                "{\n"

					"vs.1.1\n"

					"def c9,0.0,0.0,0.0,0.0\n"
					"def c11,1.0,1.0,1.0,1.0\n"
					"def c14,0.01,0.5,0.0,0.0\n"
					"def c15,0.001,0.001,0.001,0.0\n"

					"dcl_position0 v0\n"
					"dcl_position1 v1\n"
					"dcl_normal0 v3\n"
					"dcl_normal1 v4\n"
					"dcl_texcoord v7\n"

					"mov r3,c11\n"
					"sub r3,r3,c12\n"
					"mov r4,c12\n"

					"mul r1,v0,r3\n"
					"mad r1,v1,r4,r1\n"
					"m4x4 r0,r1,c0\n"

					"mul r2,v3,r3\n"
					"mad r2,v4,r4,r2\n"
					"m3x3 r1,r2,c0\n"
					"mov r1.w,c13.x\n"


					"add r2,r0,-c8\n"
					"dp3 r3.w,r2,r2\n"
					"rsq r3,r3.w\n"
					"mul r2,r2,r3\n"
					"dp3 r4,r2,r1\n"

					"sge r5,r4,c9\n"
					"mov r6,v7.x\n"
					"mul r5,r5,r6\n"


					"mul r7,r5,r2\n"
					"mul r7,r7,c10\n"
					"add r0,r7,r0\n"

					"m4x4 r2, r0, c16\n"

					"slt r8,r2,c13\n"
					"sge r9,r2,c13\n"

					"m4x4 r1,r0,c20\n"

					"mul r0,r0,r9.z\n"
					"mul r1,r1,r8.z\n"
					"add r0,r0,r1\n"

					
					"m4x4 oPos, r0, c4\n"
				"};\n"

            "VertexShaderConstant4[0] = <matWorld>;\n"
            "VertexShaderConstant4[4] = <matViewProj>;\n"
			"VertexShaderConstant1[8] = <Light>;\n"
			"VertexShaderConstant1[10] = <Len>;\n"
			"VertexShaderConstant1[13] = <ZNear>;\n"
			"VertexShaderConstant1[12] = <Interpolant>;\n"
			"VertexShaderConstant4[16] = <matView>;\n"
			"VertexShaderConstant4[20] = <matProjNearPlane>;\n"

        "}\n"
	"}\n"
	
"\n";



const UINT len_SHADOW_VOLUME_SH = sizeof(SHADOW_VOLUME_SH)-1 ;
const UINT len_SHADOW_VOLUME_SHc = sizeof(SHADOW_VOLUME_SHc)-1 ;

const UINT len_SHADOW_VOLUME_SHn = sizeof(SHADOW_VOLUME_SHn)-1 ;
const UINT len_SHADOW_VOLUME_SHcn = sizeof(SHADOW_VOLUME_SHcn)-1 ;

const UINT len_SHADOW_VOLUME_SHm = sizeof(SHADOW_VOLUME_SHm)-1 ;
const UINT len_SHADOW_VOLUME_SHmc = sizeof(SHADOW_VOLUME_SHmc)-1 ;
