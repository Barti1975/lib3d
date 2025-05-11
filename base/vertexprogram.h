
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

#ifndef _VERTEXPROGRAM_H_
#define _VERTEXPROGRAM_H_

// if code generation settings are _fastcall_ for vertexprogram.cpp then define else not.
// _cdecl_ is too slow to be used with this method
//#define _EMULATED_VERTEX_PROGRAM_METHOD_FASTCALL_
// temporary vertex color,... array filled with emulated vertex program processed vertices
#define _EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_ 20000
#define MAX_ENTRIES 2048
#define MAX_ENTRIES_ENV 256

#include <string.h>
#include <stdio.h>
#ifdef WIN32
#include <malloc.h>
#endif
#include "list.h"
#include "maths.h"

extern char * VP_STR_ERROR[6];

enum
{
	OPENGL					=	1,
	DIRECT3D				=	2,
	XYZ						=	4,
	NORMAL					=	8,
	DIFFUSE					=	16,
	SPECULAR				=	32,
	_TEX0					=	64,
	_TEX1					=	128,
	_TEX2					=	256,
	_TEX3					=	512,
	_TEX4					=	1024,
	MULTITEXTURING_ENABLED	=	2048,
	DOT3PRODUCT_ENABLED		=	4096,
	PIXELSHADER_ENABLED		=	8192,
	EMULATED				=	16384,
	BLEND					=	32768,

	D3D9					=	65536,
	VPMORPH					=	2*65536,
	PIXELSHADER2X_ENABLED	=	4*65536,
	D3D10					=	8*65536,

	_TEX5					=	16*65536,
	_TEX6					=	32*65536,
	_TEX7					=	64*65536,

	D3D11					=	128*65536,

	METAL					=	256*65536,

	_MATRIX					=	1,
	_VECTOR					=	2,
	_DWORD					=	3
};

enum
{
	CONSTANT	=	1,
	REGISTER	=	2,
	IREGISTER	=	3,
	OREGISTER	=	4
};

class API3D_STUB CVPVariable
{
public:
	int type;
	char name[64];
	char linestr[256];
	bool tobeused;
	int use;

	CVPVariable() { use=0; linestr[0]=0; }
	CVPVariable(int t,char *str,char *code) { type=t; use=0; strcpy(linestr,code); if (str) sprintf(name,"%s",str); }
};

enum
{
	OP_MOV		=	0,
	OP_ADD		=	1,
	OP_SUB		=	2,
	OP_MUL		=	3,
	OP_RCP		=	4,
	OP_RSQ		=	5,
	OP_SGE		=	6,
	OP_SLT		=	7,
	OP_M4X4		=	8,
	OP_M3X3		=	9,
	OP_CROSS	=	10,
	OP_MAX		=	11,
	OP_MIN		=	12,
	OP_DP3		=	13,
	OP_SIN		=	14,
	OP_COS		=	15,
	OP_TAN		=	16,
	OP_ASIN		=	17,
	OP_ACOS		=	18,
	OP_ATAN		=	19,
	OP_EXP		=	20,
	OP_LOG		=	21,
	OP_TGT		=	22,
	OP_FLOOR	=	23,
	OP_LIT		=	24,
	OP_PLANE	=	25,
	OP_EQPLANE	=	26,
	OP_ABS		=	27
};

enum
{
	C_REG			=	1,
	C_CONST			=	2,
	C_IVERTEX		=	3,
	C_INORMAL		=	4,
	C_ICOLOR		=	5,
	C_ICOLOR2		=	6,
	C_ITEXCOO		=	7,
	C_ITEXCOO2		=	8,
	C_ITEXCOO3		=	9,
	C_OVERTEX		=	10,
	C_ONORMAL		=	11,
	C_OCOLOR		=	12,
	C_OCOLOR2		=	13,
	C_OTEXCOO		=	14,
	C_OTEXCOO2		=	15,
	C_OTEXCOO3		=	16,
	C_IWEIGHTS		=	17,
	C_IMATRIXINDEX	=	18,

	C_IADDRESS		=	19,

	C_INORMAL2		=	20,
	C_IVERTEX2		=	17,


};

enum
{
	C_X			=	1,
	C_Y			=	2,
	C_Z			=	4,
	C_W			=	8
};

class API3D_STUB CS_Code
{
public:
	int op;
	void (*function)(CS_Code *c);
	CVector4 *dst;
	CVector4 *op1;
	CVector4 *op2;
	CMatrix *mat;
	int m[3];
	unsigned char addressed;

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
class API3D_STUB CEmulatedVertexProgram
{
public:

	CS_Code Code[128];
	int nCode;

	CVector4 iPos,iNorm,iColor,iColor2,iTexCoo,iTexCoo2,iTexCoo3,iWeights,iWIndex,iNorm2;
	CVector4 oPos,oNorm,oColor,oColor2,oTexCoo,oTexCoo2,oTexCoo3;

	CVector4 A0;
	
	CMatrix mC[MAX_ENTRIES];
	CVector4 vC[MAX_ENTRIES];
	CVector4 R[12];
	unsigned char vmc[MAX_ENTRIES];


	float * Vertex_Array;
	float * Color_Array;
	float * Color2_Array;
	float * TexCoo_Array;
	float * TexCoo2_Array;

	CEmulatedVertexProgram() 
	{
		Vertex_Array=NULL;
		Color_Array=NULL;
		Color2_Array=NULL;
		TexCoo_Array=NULL;
		TexCoo2_Array=NULL;
		nCode=0;

		for (int n=0;n<MAX_ENTRIES;n++) vmc[n]=0;
	}

	void Init();
	void Free();
	void Add(int op,char * dst,char *op1,char *op2);
	void Exec(int nb,float * Vertices,float * Normals,float * Vertices2,float * Normals2,float * Colors,float * Colors2,float * TexCoos,float * TexCoos2,int sizec,int sizec2,unsigned char * MatrixIndices,float * Weights,unsigned int fvf);
	void Set(int n,CVector4 v);
    void Set(int n,CVector v);
	void Set(int n,CMatrix m);
	void Set(int n,float x,float y,float z,float w);
	CVector4* GetPrm(int id,int n);
	CMatrix* mGetPrm(int id,int n);
	char* str_Get(CVector4 * v);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
typedef struct
{
	int AOp[4];
	int AArg1[4];
	int AArg2[4];
	int COp[4];
	int CArg1[4];
	int CArg2[4];
	int TextCoord[4];
	int Mip[16];
	int Mag[16];
	int Min[16];
	int AddressU[16];
	int AddressV[16];

	int Blend;
	int SrcBlend;
	int DstBlend;
	int ZBuffer;
	int ZBufferWrite;

	int zbias;
	int ShadeMode;

	int Zfunc;
	int AlphaTest;
	int AlphaRef;
	int AlphaFunc;
	int Dither;
	int Cull;

	// lib3d owned

	int RenderTarget;
	int Stencil;

	char* Texture[16];
    char* TextureVS[16];
	
} vpRenderState;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

class API3D_STUB Code
{
public:
	char str[256];
};

class API3D_STUB Chaine
{
public:
	char str[512];

	Chaine() {}
	Chaine(char *s) { sprintf(str,"%s",s); }
};

inline bool operator ==(Chaine &c1,Chaine &c2)
{
	if (strcmp(c1.str,c2.str)==0) return true;
	else return false;
}

inline bool operator !=(Chaine &c1,Chaine &c2)
{
	if (strcmp(c1.str,c2.str)!=0) return true;
	else return false;
}


#define TYPE_VECTOR     0
#define TYPE_MATRIX     1
#define TYPE_MATRIXLIST 2
#define TYPE_VECTORLIST 3
#define TYPE_TEXTURE    4
#define TYPE_TEXTUREVS  5

class Env
{
public:
	float mm[MAX_ENTRIES_ENV * 16];
	float vv[MAX_ENTRIES_ENV * 4];
	float m[16];
	float v[4];
	int nb;
	char name[24];
	int tex;
	int type;
	int loc;
	bool modif;
    int offset;
    int offsetv;

    Env() { offsetv=offset=-1; nb=0; loc=-1; modif=false; }
};


#define CALL_FLOAT		1
#define CALL_VECTOR2	2
#define CALL_VECTOR3	3
#define CALL_VECTOR4	4
#define CALL_IOFLOAT	5
#define CALL_IOVECTOR2	6
#define CALL_IOVECTOR3	7
#define CALL_IOVECTOR4	8

typedef struct
{
	char name[1024];
	char def[1024];
	char code[8192];
	char output[128];
	int defs[4];
	int ret;
	int registers[4];
	int floats;
	char entete[1024];
} Macro;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
class API3D_STUB CVertexProgram
{
public:

	int api;
	int metal;
	int actual_cst;
	int actual_cst_size;
	char* registers[32];
	int last_register;
	int last_temp_register;
	int trigocst[8][6];
	unsigned int local_fvf_flags;
	int actual_pass_trigocst;
    Code c;
	CList <Code> vp;
	CList <Code> s3vpmetal[8];
	CList <Code> s3vp[8];
	CList <Code> s3fnvp[8];
	CList <Code> s3psvp[8];
	CList <Code> s3psvpmetal[8];
	CList <Code> s3psfnvp[8];
	CList <Code> s3rs[8];
	CList <Code> vpcst;	
	CList <Code> vptmp;
	CList <Code> vpoutput;
	CList <Code> vpinput;
	CList <Code> glvp[8];
	CList <Code> glps[8];
	int glnvp;
	CList <Code> glheader;
	CList <Code> glheaderps;
	CList <Code> glfn;
	CList <Code> glfnps;
	CList <Code> gltemp[8];
	CList <Code> glvar[8];
	int NPASS;
	int max_temporary_register;
	// emulated opengl vertex program
	CEmulatedVertexProgram *emvp[8];
	CEmulatedVertexProgram *em;
	unsigned int fvf_output[8];
	unsigned int fvf_sm;
	bool TAG_secondary_vars;
	char* ps_constants[MAX_ENTRIES];
	unsigned char ps_constants_type[MAX_ENTRIES];
	char* translate_ps_constants[MAX_ENTRIES];
	char* translate_constants[MAX_ENTRIES];
	float float_translate_ps_constants[MAX_ENTRIES][16];
	float float_translate_constants[MAX_ENTRIES][4];	
	char matrices[MAX_ENTRIES][32];
	int nmatrices;
	char floats[MAX_ENTRIES][32];
	int nfloats;
	char definestr[MAX_ENTRIES][32];
	int ndefs;
	char* constants[MAX_ENTRIES];
	int l_constants[MAX_ENTRIES];
	int last_constant_register;
	int last_constant;

	char* psvectors[MAX_ENTRIES];
	int npsvectors;
	char* psmatrices[MAX_ENTRIES];
	int npsmatrices;
    
    int localint;
    
    int pointcloud;

	int pop;
	char * texture[16];
	int ntexture;
    char * texturevs[16];
    int ntexturevs;
	char* iregisters[32][2];
	int last_iregister;
	char* oregisters[32][2];
	int last_oregister;
	char* o2registers[32][2];
	int last_o2register;
	CVPVariable *VariableNulle;
	CMap <Chaine,CVPVariable> Vars;
	CMap <Chaine,CVPVariable> TempVars;
	vpRenderState RenderState[4][4];
	CList <Code> *actual_vp;
	int selected_tech;
	float env[MAX_ENTRIES][4];
	int n_env;
	char str_error[2048];
	int groups[12][12][12];
	int vp_groups[12];
	int ptr_groups[12];
	CList <Macro> ListMacros;
	char *PSH_OUTPUT;
	int phase;
	bool pixelshader;
	int compteur_inst;
	int last_ps_constant_register;
	int last_ps_constant;
	int last_register_macro;
	bool selected_tech_with_ps;
	bool tag_result_present;	
	bool shadermodel;
	bool shadermodel3;
	CList <Env> listenv;
	bool weightsandindices;
	bool stream;
	Macro vsfn[24];
	Macro psfn[24];
	int numvsfn;
	int numpsfn;
	int numbervsfn[8];
	int numberpsfn[8];
	int label;
	bool addsemi;
	int tagreg[32];
	int tagtempreg[32];
	int tagregsave[32];
	char entete[1024];
	bool func;
	int levelrep;
	int flow;
	int modifyZ;
	int version300;
	bool shadermodel4;
	int shadertexturelevel;
	char ERRORSTR[256];
    char RSERRORSTR[256];
	char PRMERRORSTR[256];
	char LASTLINESTR[256];
		
	char UNUSEDRRORSTR[256];
	bool SYNTAXERROR;
    bool RSERROR;
	bool SYNTAXERRORFN;
	bool TYPEERROR;
	bool DEFINEERROR;
	bool UNUSEDERROR;
    bool IFERROR;
	bool PARAMERROR;
	bool REPERROR;
	char ERRORSTRREP[128];
	char ERRORSTRPARAM[128];

	int shadows[16];
	int shadowsbilinear[16];
	int polynomial;
	int mouss1[16],mouss2[16];
	int shadowssoft1[16];
	int shadowssoft4[16];

	int linedebug;
	int vsfncodeline[24][8];
	int psfncodeline[24][8];
	int vscodeline[8];
	int pscodeline[8];
	char vsfnname[24][8][64];
	char psfnname[24][8][64];
	char paramsFn[4][64];
	int defsFn[4];
	bool NoDepthOut;
	bool divprocessing;
	bool floatprocessing;
	bool floatprocessinginit;
	int nbmodifiersprocessing;
    int vertex[8];
    int vertex2[8];
    int color[8];
    int normal[8];
    int normal2[8];
    int weights[8];
    int texcoo0[8];
    int texcoo1[8];
    int texcoo2[8];
    int texcoo3[8];
	bool globalvar;
	int defines[4];
	bool compile_expression;
	int levelifelse;
	int levelrepglobal;
    int warp_normal_pos;
	char sDiscardAPI[32];
	char tmpstrcond[1024];
    int FSR;
    bool isFSR;

	int FSRSample0;
	int FSRSample1;

	int SizingTexture0;

	int main;

	int recursvar;

	~CVertexProgram() 
	{
		delete VariableNulle;
	}

	CVertexProgram() 
	{
		int n;

		main=1;
		recursvar=0;

		version300=0;
		globalvar=false;
        
        FSR=0;
        isFSR=false;

		FSRSample0=0;
		FSRSample1=0;
		SizingTexture0=0;
        
        warp_normal_pos=-1;
        
        pointcloud=0;

        for (n=0;n<16;n++)
        {
			shadows[n]=0;
			shadowsbilinear[n]=0;
			shadowssoft1[n]=0;
			shadowssoft4[n]=0;
			mouss1[n]=0;
			mouss2[n]=0;
		}

		polynomial=0;		

        for (n=0;n<8;n++)
        {
            vertex[n]=-666;
            vertex2[n]=-666;
            color[n]=-666;
            normal[n]=-666;
            normal2[n]=-666;
            weights[n]=-666;
            texcoo0[n]=-666;
            texcoo1[n]=-666;
            texcoo2[n]=-666;
            texcoo3[n]=-666;
        }

		compile_expression=false;
        
        localint=0;
        
		divprocessing=false;
		nbmodifiersprocessing=4;
		floatprocessinginit=floatprocessing=false;

		levelifelse=0;
		levelrepglobal=0;

		TYPEERROR=false;
		SYNTAXERROR=false;
		SYNTAXERRORFN=false;
		DEFINEERROR=false;
		UNUSEDERROR=false;
		PARAMERROR=false;
		REPERROR=false;
        RSERROR=false;
        IFERROR=false;
        
		ERRORSTRREP[0]=0;
		PRMERRORSTR[0]=0;

		shadertexturelevel=0;
		shadermodel4=false;
		modifyZ=0;
		flow=1;
		levelrep=0;
		func=false;

		for (n=0;n<32;n++) tagreg[n]=0;
		for (n=0;n<32;n++) tagtempreg[n]=0;

		addsemi=false;
		label=numvsfn=numpsfn=0;
		stream=false;
		weightsandindices=false;
		listenv.Free();
		shadermodel=false;
		shadermodel3=false;
		tag_result_present=false;
		compteur_inst=0;
		pixelshader=false;
		n_env=0;
		ntexture=0;
		
		for (n=0;n<24;n++) vsfn[n].name[0]=0;
		for (n=0;n<24;n++) psfn[n].name[0]=0;

		for (n=0;n<16;n++) texture[n]=NULL;
        for (n=0;n<16;n++) texturevs[n]=NULL;
        ntexture=ntexturevs=0;
        
		for (n=0;n<32;n++) registers[n]=NULL;
		for (n=0;n<32;n++) iregisters[n][0]=NULL;
		for (n=0;n<32;n++) oregisters[n][0]=NULL;
		for (n=0;n<32;n++) o2registers[n][0]=NULL;
		for (n=0;n<32;n++) iregisters[n][1]=NULL;
		for (n=0;n<32;n++) oregisters[n][1]=NULL;
		for (n=0;n<32;n++) o2registers[n][1]=NULL;
		for (n=0;n<MAX_ENTRIES;n++) constants[n]=NULL;
		for (n=0;n<MAX_ENTRIES;n++) l_constants[n]=0;
		for (n=0;n<MAX_ENTRIES;n++) translate_constants[n]=NULL;
		for (n=0;n<MAX_ENTRIES;n++) ps_constants[n]=NULL;
		for (n=0;n<MAX_ENTRIES;n++) translate_ps_constants[n]=NULL;

		npsvectors=0;
		for (n=0;n<MAX_ENTRIES;n++) psvectors[n]=NULL;

		npsmatrices=0;
		for (n=0;n<MAX_ENTRIES;n++) psmatrices[n]=NULL;

		VariableNulle=new CVPVariable(CONSTANT,"","");
		emvp[0]=NULL;
		emvp[1]=NULL;
		emvp[2]=NULL;
		emvp[3]=NULL;
		emvp[4]=NULL;
		emvp[5]=NULL;
		emvp[6]=NULL;
		emvp[7]=NULL;
		TAG_secondary_vars=false;
	}
    
    bool isValueCoef();

	void Translate_Macros(char * macros);
	void clear_registers();
	
    int new_register();
	int new_temp_register();
	
    void close_temp_register();
    void close_temp_register(int last);
    
	void set_register(int n,char *name);

	bool register_writeonly(char *str00);

	char* get_string_vsh(int pass);

    bool IsRegisterIn(int reg,CList<Code> &vpcode);

	char* get_string_mega_vs(int pass);
	char* get_string_mega_ps(int pass);

	char * compile_macro(Macro *m,char * param,CList <Code> *vp);
	void compile_code(char *str,CList <Code> *vp,int last_line_pixelshader);

	bool read_shader(char *script,unsigned int flags);

	int get_pass_count();
	char * result_parse_condition(char *str);


	char* is_ps_constant(char *name);
	char* is_ps_matrix(char *name);
	void new_ps_constant(char *name);
	void new_ps_matrix(char *name);
	int set_translate_ps_constant(char *cst,char *translate);
	int set_translate_ps_matrix(char *cst,char *translate);

	int set_translate_constant(char *cst,char *translate);
	
	void header_open_vsh(int pass,unsigned int flags,unsigned int output);
	void header_open_psh(int pass,unsigned int flags,unsigned int output);
	void header_close();
	void header_end();
	void header_close_psh();
	void header_open_pass(int npass);

	void compile_script(char *script,CList <Code> *vp);

	char * test_force_register(char *str);
	
	int numberswz(char *op);

	char** is_output(char *name);
	char** is_input(char *name);

	int new_ps_constant_register();
	int new_constant_register();
	CVPVariable * Translate(char *str);

	void compile_init_variables(char *str,CList <Code> *vp);
	void new_constant(char *name,int size);
	void new_constant(char *name,int size,int nb);

	void new_input_register(char *name,char * trans);
	void new_output_register(char *name,char * trans);
	void new_output2_register(char *name,char * trans);

	void new_psvector(char *name);
	int is_psvector(char *name);

	void new_psmatrix(char *name);
	int is_psmatrix(char *name);


	char * var(char * str);
	char * var(char * str,int tag);

	void AddInst(CList <Code> *vp,Code &c);
    void AddInst2(CList <Code> *vp,Code &c);
	void AddInst00(CList <Code> *vp,Code &c);
	
	char* is_constant(char *name);

	int getsmp(char *stex);

	void compile_render_state(int tech,int pass,CList <Code> *vp);
	bool compile_render_state_pixelshader(int tech,int pass,CList <Code> *vp);

	void get_vp(int pass);
	void get_ps(int pass);

	char* get_string_psh(int pass);

	void reset(int ext);
    void reset();
    
	void cleanbuffs();

	void setIO(unsigned int flags);

	char * compile(char *str,char *output,CList <Code> *vp);

	void Free();

	void parse_script_vs_functions(char *script);
	void parse_script_ps_functions(char *script);

	void parse_inner_func(char *script,char *&funcs);
	void parse_inner_func_position(char * &script_position);

	int vsfnnum(char *fnname);
	int psfnnum(char *fnname);

	void affectparameters(char *call,Macro *fn,CList <Code> *vp);

	char * compile_function(Macro *fn,CList <Code> *vp);
	char * compile_psfunction(Macro *fn,CList <Code> *vp);

	void modifiersfn(Macro *fn,char *s,CList <Code> *vp);

	char * str_if_else_virg_treatment(char * script00);
	char * str_get_script(char * script, int *pos);
	char * str_get_scriptfns(char * script, int *pos);
	char * str_get_scriptm(char * script, int *pos);

	char * str_parse_addsub(char * script);


	int isdefined(char *str);
	int isbasicdefined(char *str);
	void registerdefine(char *str);

	void unused_variables();


	// MISC

	bool correct_modifier_texture2d(char * reg);

	bool isfloat(char * rn);
	bool floating(char *str);
	int nb_modifiers_syntax(char * tmp);
	int nb_modifiers(char * tmp);
	void swizzle(char *output, char *tmp4);
	void resfield(char *output, char *tmp4);
	void outputfield1(char *tmp4, char *output, char *rn);
	void outputfield2(char *tmp4, char *output, char *rn, char *rn2);
	void outputfield3(char *tmp4, char *output, char *rn, char *rn2,char *rn3);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
extern int LENGTH_VAR[4];

enum
{

	_UNDEFINED			=	500,
	_MODULATE			=	501,
	_ADD				=	502,
	_DOT3				=	503,
	_DISABLE			=	504,
	_ADDSIGNED			=	505,
	_MODULATE2X			=	506,
	_SUBTRACT			=	507,

	_TRUE				=	508,
	_FALSE				=	509,

	_TEXTURE			=	510,
	_DIFFUSE			=	511,
	_SPECULAR			=	512,
	_CURRENT			=	513,

	_NONE				=	514,
	_CW					=	515,
	_CCW				=	516,

	_ZERO				=	517,
	_ONE				=	518,
	_SRCALPHA			=	519,
	_INVSRCALPHA		=	520,
	_SRCCOLOR			=	521,
	_INVSRCCOLOR		=	522,
	_DSTALPHA			=	523,
	_INVDSTALPHA		=	524,
	_DSTCOLOR			=	525,
	_INVDSTCOLOR		=	526,
	
	_TEXCOO0			=	527,
	_TEXCOO1			=	528,
	_TEXCOO2			=	529,


	_STENCIL			=	530,
	_RENDER				=	531,
	_BOTH				=	532,
	_STENCIL_NO_ZTEST	=	533,
	_ZBUFFER			=	534,

	_INCREMENT			=	535,
	_DECREMENT			=	536,		
	_SET_ONE			=	537,
	_EQUAL_ZERO			=	538,
	_EQUAL_ONE			=	539,
	_ALWAYS				=	540,
	_NOT_ONE			=	541,
	_NOT_ZERO			=	542,
	_LESSEQUAL_ONE		=	543,
	_GREATEREQUAL_ONE	=	544,
	_INCREMENTSAT		=	545,
	_DECREMENTSAT		=	546,

	_SMOOTH				=	547,
	_FLAT				=	548,

	_OFFSET_0			=	549,
	_OFFSET_1			=	550,
	_OFFSET_2			=	551,
	_OFFSET_3			=	552,
	_OFFSET_4			=	553,
	_OFFSET_5			=	554,
	_OFFSET_6			=	555,
	_OFFSET_7			=	556,

	_NEVER				=	557,
	_LESS				=	558,
	_EQUAL				=	559,
	_LEQUAL				=	560,
	_GREATER			=	561,
	_NOTEQUAL			=	562,
	_GEQUAL				=	563,

	_SELECTARG1			=	564,
	_SELECTARG2			=	565,

	_LINEAR				=	566,
	_BUMPENVMAP			=	567,

	_CLAMP				=	568,
	_REPEAT				=	569,

	_ADDSIGNED2X		=	570,
	_TEXCOO3			=	571,

	_POINT				=	572,
	_BOTHZ				=	573,

	_TWOSIDED			=	666,

};

void FSRCASSampleSharp(float x);
void SampleBumpy(float x);

#endif
