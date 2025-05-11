const char MULTI00_SH[] = 

    "TEXTURE tex1;\n"
	"TEXTURE tex2;\n"
	"DWORD coord0;\n"
	"DWORD coord1;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "Texture[1] = <tex2>;\n"
			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"

			"TexCoordIndex[0] = <coord0>;\n"
			"TexCoordIndex[1] = <coord1>;\n"



            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
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
		"}\n"
    "}\n"


    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"TexCoordIndex[0] = <coord0>;\n"
		"}\n"


		"pass P1\n"
        "{\n"

			"Texture[0] = <tex2>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = Zero;\n"

			"TexCoordIndex[0] = <coord1>;\n"

		"}\n"
    "}\n";

const UINT len_MULTI00_SH = sizeof(MULTI00_SH)-1 ;



const char MULTI01_SH[] = 

    "TEXTURE tex1;\n"
	"TEXTURE tex2;\n"
	"DWORD coord0;\n"
	"DWORD coord1;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "Texture[1] = <tex2>;\n"
			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"TexCoordIndex[0] = <coord0>;\n"
			"TexCoordIndex[1] = <coord1>;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = AddSigned;\n"
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


    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"
			"TexCoordIndex[0] = <coord0>;\n"
		"}\n"


		"pass P1\n"
        "{\n"

			"Texture[0] = <tex2>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = One;\n"
			"DestBlend = One;\n"
			"TexCoordIndex[0] = <coord1>;\n"

		"}\n"
    "}\n";

const UINT len_MULTI01_SH = sizeof(MULTI01_SH)-1 ;


const char MULTI10_SH[] = 

    "TEXTURE tex1;\n"
	"TEXTURE tex2;\n"
	"DWORD coord0;\n"
	"DWORD coord1;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "Texture[1] = <tex2>;\n"
			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"TexCoordIndex[0] = <coord0>;\n"
			"TexCoordIndex[1] = <coord1>;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
			"ColorOp[0] = Add;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Modulate;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Add;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

		"}\n"
    "}\n"


    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
           
			"ColorOp[0] = Modulate;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Modulate;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"
			"TexCoordIndex[0] = <coord0>;\n"
		"}\n"


		"pass P1\n"
        "{\n"

			"Texture[0] = <tex2>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = DestColor;\n"
			"DestBlend = Zero;\n"
			"TexCoordIndex[0] = <coord1>;\n"

		"}\n"
    "}\n";

const UINT len_MULTI10_SH = sizeof(MULTI10_SH)-1 ;


const char MULTI11_SH[] = 

    "TEXTURE tex1;\n"
	"TEXTURE tex2;\n"
	"DWORD coord0;\n"
	"DWORD coord1;\n"

    "technique T0\n"
    "{\n"
        "pass P0\n"
        "{\n"

            "Texture[1] = <tex2>;\n"
			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
			"TexCoordIndex[0] = <coord0>;\n"
			"TexCoordIndex[1] = <coord1>;\n"

            "AddressU[1] = Wrap;\n"
            "AddressV[1] = Wrap;\n"
            "MinFilter[1] = Linear;\n"
            "MagFilter[1] = Linear;\n"
           
			"ColorOp[0] = Add;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"ColorOp[1] = Add;\n"
            "ColorArg1[1] = Texture;\n"
			"ColorArg2[1] = Current;\n"

			"AlphaOp[0]   = Add;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"

			"AlphaOp[1]   = Modulate;\n"
			"AlphaArg1[1] = Texture;\n"
			"AlphaArg2[1] = Current;\n"

		"}\n"
    "}\n"


    "technique T1\n"
    "{\n"
        "pass P0\n"
        "{\n"

			"Texture[0] = <tex1>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"
           
			"ColorOp[0] = Add;\n"
            "ColorArg1[0] = Texture;\n"
			"ColorArg2[0] = Diffuse;\n"

			"AlphaOp[0]   = Add;\n"
			"AlphaArg1[0] = Texture;\n"
			"AlphaArg2[0] = Diffuse;\n"
			"TexCoordIndex[0] = <coord0>;\n"
		"}\n"


		"pass P1\n"
        "{\n"

			"Texture[0] = <tex2>;\n"

            "AddressU[0] = Wrap;\n"
            "AddressV[0] = Wrap;\n"
            "MinFilter[0] = Linear;\n"
            "MagFilter[0] = Linear;\n"


			"ColorOp[0] = SelectArg1;\n"
            "ColorArg1[0] = Texture;\n"

			"AlphaOp[0]   = SelectArg1;\n"
			"AlphaArg1[0] = Diffuse;\n"

			"AlphaBlendEnable = true;\n"
			"SrcBlend = One;\n"
			"DestBlend = One;\n"
			"TexCoordIndex[0] = <coord1>;\n"

		"}\n"
    "}\n";

const UINT len_MULTI11_SH = sizeof(MULTI11_SH)-1 ;




