
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

#ifndef _LZCOMPRESS_H_
#define _LZCOMPRESS_H_

#ifdef WIN32
#define FASTCONV __fastcall
#else
#define FASTCONV
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct _LZ_Node
{
	int value;
	unsigned int dec;
	int len;
	_LZ_Node *zero;
	_LZ_Node *un;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _LZ_Node LZ_Node;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct repetition
{
	unsigned int n_r;
	unsigned int codelen;
	unsigned int code;
	unsigned int n_r0;

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct lettre
{
	unsigned int codelen;
	unsigned int valeur;
	unsigned int valeurdec;
	unsigned int Bits[16];

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// THE CLASS
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class LZCOMPRESS
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	bool FORCE_PREDEFCODES;
	LZ_Node *tabNodes;
	int ntabNodes;
	LZ_Node *l_tree;
	LZ_Node *d_tree;
	unsigned int ptr_huffmanlz77bits;
	char * ptr_huffmanlz77;
	unsigned int lengthtmp;
	unsigned int resultatlz77;
	unsigned int DEFLATE_LONGUEUR_deflated;
	int TAG_NODEPACK;
	int alphabetcodeslenghtscodes[19];
	struct repetition TABretenu[500];
	int NbreBits;
	unsigned int GAINretenu;
	unsigned int LZ77retoffset;
	unsigned int LZ77distance;
	int TAG_HUFFMAN_CODES;
	int CodeLengths00[512];
	int CodeLengthsRetenus[512];
	unsigned int LEN_LZ77_Huffman;
	unsigned int increment;
	unsigned int DISTANCE;
	unsigned int __LENGTH;

	unsigned int ALPHABETLITTERALdec[290];
	unsigned int ALPHABETLITTERALlen[290];
	unsigned int ALPHABETDISTANCEdec[32];
	unsigned int ALPHABETDISTANCElen[32];

	unsigned int ALPHABETCODELENGTHSdec[32];
	unsigned int ALPHABETCODELENGTHSlen[32];

	struct lettre ALPHABETCODELENGTHS[19];
	struct lettre ALPHABETLITTERAL[290];
	struct lettre ALPHABETDISTANCE[32];

	/////////////////////////////////////////////////////////////////////////////////////////////

	LZCOMPRESS();
	~LZCOMPRESS();

	/////////////////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS

	void LZ_Init(LZ_Node **n);
	LZ_Node* FASTCONV NewNode();
	void FASTCONV LZ_TreeAdd(LZ_Node **root,int value,unsigned int dec0,int len);
	int FASTCONV LZ_GetValue(LZ_Node **root,unsigned int dec0);
	int FASTCONV LZ_GetValue_D(LZ_Node **root,unsigned int dec0);
	void LZ_Init_Tab_Nodes();
	void LZ_Free_Tab_Nodes();
	void LZ_Init_tree();
	void LZ_Free_tree();
	unsigned int FASTCONV decodeCODELENGTHS(char * pp,int p);
	unsigned int FASTCONV decodeLITTERAL_LENGTH(char * pp,int p);
	unsigned int FASTCONV decodeDISTANCE(char * pp,int p);
	void ALPHABETCODELENGTHS_();
	void ALPHABETLITTERAL_();
	void ALPHABETLITTERAL_fixed();
	void ALPHABETDISTANCE_();
	unsigned int DEFLATE_block(char * dataBLOCK,char * ImageDATA,unsigned int piDATA0);
	int DEFLATE_zlibBlock(char * dataBLOCK,char * ImageDATA,int piDATA);
	unsigned int FASTCONV CodeLength(unsigned int len);
	unsigned int FASTCONV CodeDistance(unsigned int dist);
	void ALPHABETLITTERAL_encode_fixed();
	void ALPHABETLITTERAL_encode();
	void ALPHABETDISTANCE_encode();
	void ALPHABETDISTANCE_encode_fixed();
	void ALPHABETCODELENGTHS_encode();
	void ALPHABETDISTANCE_fixed();
	unsigned int FASTCONV nbrerepetitionsids(struct repetition tab[],int p);
	int FASTCONV n_rdifferentzero(struct repetition tab[],int p);
	void FASTCONV recursCodeLengths(int NBits,struct repetition tab[],int lentab,int Niter,int p,int NbreBits);
	unsigned int FASTCONV CalculeGAIN(int len,struct repetition TAB[]);
	void FASTCONV PrepareCodeLengths();
	void FASTCONV RetiensCodeLengths(unsigned int GAIN,int etage);
	void FASTCONV StockeCodeLengths(int len);
	void FASTCONV recursCodelengths2(int nb,int etage,int N,struct repetition TAB[]);
	unsigned int CodageHuffman(unsigned int * datasLZ77,unsigned int LZ77offset,unsigned int len,char * c_blocks,unsigned int c_offset,unsigned int status);
	char * LZ77_Huffman0(unsigned char * ptrDATA,unsigned int len,int REPETITION,int DEPTH);
	char * QuickLZ77_Huffman(char * ptrDATA,unsigned int len);
	char * LZ77_Huffman(char * ptrDATA,unsigned int len);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STRUCT

struct Inflate
{
	unsigned int Type;
	unsigned int C_Size;
	unsigned int Size;
	char * BlockPtr;
};


#define INFLATE_NORMAL	0
#define INFLATE_WAV		1
#define INFLATE_SUB		2
#define INFLATE_SUB4	3
#define INFLATE_SUB3	4

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL

struct Inflate * InflateDatas(char * ptrDATA,unsigned int size,char Type);
char * DeflateDatas(struct Inflate * NFOS);
char * DeflateDatasZLIB(struct Inflate * NFOS);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INTERNAL 

struct Inflate * InflateNormal(char * ptrDATA,unsigned int size);
struct Inflate * InflateQuick(char * ptrDATA,unsigned int size);
struct Inflate * InflateWav(char * ptrDATA,unsigned int size);
struct Inflate * InflateSub(char * ptrDATA,unsigned int size);
struct Inflate * InflateSub4(char * ptrDATA,unsigned int size);
struct Inflate * InflateSub3(char * ptrDATA,unsigned int size);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern LZCOMPRESS TheClassForCompatibility;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
