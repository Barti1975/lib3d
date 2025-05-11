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


////////////////////////////////////////////////////////////////////////
//	@file lzcompress.cpp 
//	@created 2002
//  @date 2014-07
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LEN_CODAGEHUFFMAN 1024
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAXLENCODE 500
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOLIB3D
#  include "../base/params.h"
#endif


#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <math.h>
#include "lzcompress.h"

#ifdef _MSC_VER
	#pragma warning (disable:4101)
	#pragma warning (disable:4554)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FIXED 0
#define DYNAMIC 1

/////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int LENGTH_EXTRABITS[31]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0};
unsigned int LENGTH_LENGTH[31]={ 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258,259,0};

unsigned int DIST_EXTRABITS[31]={ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,0};
unsigned int DIST_DIST[31]={ 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

LZCOMPRESS TheClassForCompatibility;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
LZCOMPRESS::LZCOMPRESS()
{
	TAG_HUFFMAN_CODES=0;
	FORCE_PREDEFCODES=false;
	TAG_NODEPACK=0;
	l_tree=NULL;
	d_tree=NULL;
	ntabNodes=0;
    tabNodes=new LZ_Node[32768];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
LZCOMPRESS::~LZCOMPRESS()
{
	LZ_Free_tree();
    delete [] tabNodes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DECODAGE
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NBITSMAX 16
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBIT(pp,p)  (((((unsigned char*) pp)[(p>>3)])>>(p&7))&1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBITS(pp,p,nb) ((*((unsigned int*) &((unsigned char*) pp)[(p>>3)])>>(p&7))&((1<<nb)-1))
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBITS_D(pp,p,nb) ((*((unsigned int*) &((unsigned char*) pp)[(p>>3)])>>(p&7))&((1<<nb)-1))
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define bit_n(valeur,n) ((valeur>>n)&1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int masks_shift[32]=
{
	(1<<0)-1,(1<<1)-1,(1<<2)-1,(1<<3)-1,(1<<4)-1,(1<<5)-1,(1<<6)-1,(1<<7)-1,
	(1<<8)-1,(1<<9)-1,(1<<10)-1,(1<<11)-1,(1<<12)-1,(1<<13)-1,(1<<14)-1,(1<<15)-1,
	(1<<16)-1,(1<<17)-1,(1<<18)-1,(1<<19)-1,(1<<20)-1,(1<<21)-1,(1<<22)-1,(1<<23)-1,
	(1<<24)-1,(1<<25)-1,(1<<26)-1,(1<<27)-1,(1<<28)-1,(1<<29)-1,(1<<30)-1,0xFFFFFFFF
 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NBITSMAX 16
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBIT(pp,p)  (((((unsigned char*) pp)[(p>>3)])>>(p&7))&1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBITS(pp,p,nb) ((*((unsigned int*) &((unsigned char*) pp)[(p>>3)])>>(p&7))&masks_shift[nb])
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define READBITS_D(pp,p,nb) ((*((unsigned int*) &((unsigned char*) pp)[(p>>3)])>>(p&7))&masks_shift[nb])
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define bit_n(valeur,n) ((valeur>>n)&1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::decodeCODELENGTHS(char * pp,int p)
{
	unsigned int dec;
	unsigned int n;
	unsigned int mask,test;
	int res;

	dec=READBITS(pp,p,NBITSMAX);
	n=0;
	res=-1;
	while ((n<19)&&(res==-1))
	{
 		increment=ALPHABETCODELENGTHSlen[n];
		if (increment!=0)
		{
			mask=(1<<increment)-1;
			test=dec&mask;
			if (test==ALPHABETCODELENGTHSdec[n]) res=n; else n++;
		}
		else n++;
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define UI(x) ((unsigned int)x)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int masks_pow[32]=
{
	UI(1<<0),UI(1<<1),UI(1<<2),UI(1<<3),UI(1<<4),UI(1<<5),UI(1<<6),UI(1<<7),
	UI(1<<8),UI(1<<9),UI(1<<10),UI(1<<11),UI(1<<12),UI(1<<13),UI(1<<14),UI(1<<15),
	UI(1<<16),UI(1<<17),UI(1<<18),UI(1<<19),UI(1<<20),UI(1<<21),UI(1<<22),UI(1<<23),
	UI(1<<24),UI(1<<25),UI(1<<26),UI(1<<27),UI(1<<28),UI(1<<29),UI(1<<30),UI(1<<31)
 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::LZ_Init(LZ_Node **n)
{
	*n=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
LZ_Node* FASTCONV LZCOMPRESS::NewNode()
{
	LZ_Node *n;
	n=&(tabNodes[ntabNodes++]);
	n->un=NULL;
	n->zero=NULL;
	n->len=0;
	n->dec=0;
	n->value=0;
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::LZ_TreeAdd(LZ_Node **root,int value,unsigned int dec0,int len)
{
	int n,bit;
	LZ_Node *nod;
	unsigned int dec=dec0;

	if (*root) nod=*root;
	else
	{
		nod=NewNode();
		*root=nod;
	}

	for (n=0;n<len;n++)
	{
		bit=dec&masks_pow[n];

		if (bit)
		{
			if (!(nod->un)) nod->un=NewNode();
			nod=nod->un;
		}
		else
		{
			if (!(nod->zero)) nod->zero=NewNode();
			nod=nod->zero;
		}
	}

	nod->dec=dec;
	nod->value=value;
	nod->len=len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int FASTCONV LZCOMPRESS::LZ_GetValue(LZ_Node **root,unsigned int dec0)
{
	int n;
	int res;
	int bit;
	unsigned int dec=dec0;
	LZ_Node *node,*noderes;

	noderes=node=*root;

	n=0;
	while (node)
	{
		bit=dec&masks_pow[n];
		noderes=node;
		if (bit) node=node->un;
		else node=node->zero;
		n++;
	}
	if (noderes)
	{
		res=noderes->value;
		increment=noderes->len;
	}
	else res=0;

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int FASTCONV LZCOMPRESS::LZ_GetValue_D(LZ_Node **root,unsigned int dec0)
{
	int n,res;
	int bit;
	unsigned int dec=dec0;
	LZ_Node *node,*noderes;

	noderes=node=*root;
	n=0;
	while (node)
	{
		bit=dec&masks_pow[n];
		noderes=node;
		if (bit) node=node->un;
		else node=node->zero;
		n++;
	}

	if (noderes)
	{
		res=noderes->value;
		increment=noderes->len;
	}
	else res=0;

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::LZ_Init_Tab_Nodes()
{
	ntabNodes=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::LZ_Free_Tab_Nodes()
{
	ntabNodes=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::LZ_Init_tree()
{
	int n;

	l_tree=NULL;
	d_tree=NULL;

	ntabNodes=0;

	LZ_Init(&l_tree);
	LZ_Init(&d_tree);

	for (n=0;n<=285;n++)
		LZ_TreeAdd(&l_tree,n,ALPHABETLITTERALdec[n],ALPHABETLITTERALlen[n]);


	for (n=0;n<=29;n++)
		LZ_TreeAdd(&d_tree,n,ALPHABETDISTANCEdec[n],ALPHABETDISTANCElen[n]);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::LZ_Free_tree()
{
	l_tree=NULL;
	d_tree=NULL;

	ntabNodes=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::decodeLITTERAL_LENGTH(char * pp,int p)
{
	unsigned int EXTRABITS;
	unsigned int dec;
	unsigned int res;

	dec=READBITS(pp,p,NBITSMAX);

	res=LZ_GetValue(&l_tree,dec);

	int restmp=res-257;

	if ((restmp>=0)&&(res<=285))
	{
		// decode LENGTH
		EXTRABITS=LENGTH_EXTRABITS[restmp];

		if (EXTRABITS>0)
		{
			__LENGTH=READBITS(pp,p+increment,EXTRABITS)+ LENGTH_LENGTH[restmp];
			increment+=EXTRABITS;
		}
		else
		{
			__LENGTH=LENGTH_LENGTH[restmp];
		}
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::decodeDISTANCE(char * pp,int p)
{
	unsigned int EXTRABITS;
	unsigned int dec;
	unsigned int res;

	dec=READBITS_D(pp,p,NBITSMAX);

	if (d_tree) res=LZ_GetValue_D(&d_tree,dec);
	else res=0;

	EXTRABITS=DIST_EXTRABITS[res];
	if (EXTRABITS>0)
	{
		DISTANCE=READBITS_D(pp,p+increment,EXTRABITS)+ DIST_DIST[res];
		increment+=EXTRABITS;
	}
	else
	{
		DISTANCE=DIST_DIST[res];
	}


	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETCODELENGTHS_()
{
	unsigned int n,n0;
	unsigned int rr,rr0,rr1;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<19;n++)
	{
		nbres[ALPHABETCODELENGTHS[n].codelen]++;
	}

	nbres[0]=0;
	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}
	for (n=0;n<NBITSMAX;n++) nbres[n]=0;
	for (n=0;n<19;n++)
	{
		len=ALPHABETCODELENGTHS[n].codelen;
		if (len==0)
		{
			ALPHABETCODELENGTHS[n].valeur=0;
			ALPHABETCODELENGTHSlen[n]=0;
		}
		else
		{
			rr=valeurs[len]+nbres[len];
			ALPHABETCODELENGTHS[n].valeur=rr;
			ALPHABETCODELENGTHSlen[n]=len;
			rr1=rr;
			rr=0;
			for (n0=0;n0<len;n0++)
			{
				ALPHABETCODELENGTHS[n].Bits[n0]=bit_n(ALPHABETCODELENGTHS[n].valeur,len-n0-1);
				rr=rr<<1;
				rr0=rr1&1;
				rr+=rr0;
				rr1=rr1>>1;

			}
			ALPHABETCODELENGTHS[n].valeurdec=rr;
			ALPHABETCODELENGTHSdec[n]=rr;
			nbres[len]++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETLITTERAL_()
{
	unsigned int n,n0;
	unsigned int rr,rr0,rr1;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur;
	unsigned int bit;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;
	for (n=0;n<=285;n++)
	{
		nbres[ALPHABETLITTERAL[n].codelen]++;
	}
	nbres[0]=0;
	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=285;n++)
	{
		len=ALPHABETLITTERAL[n].codelen;
		if (len==0)
		{
			ALPHABETLITTERAL[n].valeur=0;
			ALPHABETLITTERALlen[n]=len;
		}
		else
		{
			rr=valeurs[len]+nbres[len];
			ALPHABETLITTERAL[n].valeur=rr;
			rr1=rr;
			rr=0;
			for (n0=0;n0<len;n0++)
			{
				bit=bit_n(rr,len-n0-1);
				ALPHABETLITTERAL[n].Bits[n0]=bit;
				rr=rr<<1;
				rr0=rr1&1;
				rr+=rr0;
				rr1=rr1>>1;
			}
			ALPHABETLITTERAL[n].valeurdec=rr;

			ALPHABETLITTERALdec[n]=rr;
			ALPHABETLITTERALlen[n]=len;


			nbres[len]++;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETLITTERAL_fixed()
{
	unsigned int n,n0;
	unsigned int rr,rr0,rr1;
	unsigned int len;

	for (n=0;n<=143;n++)
	{
		ALPHABETLITTERAL[n].codelen=8;
		ALPHABETLITTERAL[n].valeur=48+n;
	}
	for (n=144;n<=255;n++)
	{
		ALPHABETLITTERAL[n].codelen=9;
		ALPHABETLITTERAL[n].valeur=400+(n-144);
	}
	for (n=256;n<=279;n++)
	{
		ALPHABETLITTERAL[n].codelen=7;
		ALPHABETLITTERAL[n].valeur=(n-256);
	}
	for (n=280;n<=287;n++)
	{
		ALPHABETLITTERAL[n].codelen=8;
		ALPHABETLITTERAL[n].valeur=192+(n-280);
	}

	for (n=0;n<=285;n++)
	{
		len=ALPHABETLITTERAL[n].codelen;
		rr=ALPHABETLITTERAL[n].valeur;
		rr1=rr;
		rr=0;
		for (n0=0;n0<len;n0++)
		{
			ALPHABETLITTERAL[n].Bits[n0]=bit_n(rr,len-1-n0);
			rr=rr<<1;
			rr0=rr1&1;
			rr+=rr0;
			rr1=rr1>>1;
		}
		ALPHABETLITTERAL[n].valeurdec=rr;
		ALPHABETLITTERALdec[n]=rr;
		ALPHABETLITTERALlen[n]=len;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETDISTANCE_()
{
	unsigned int n,n0;
	unsigned int rr;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur,rr0,rr1;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=29;n++)
	{
		nbres[ALPHABETDISTANCE[n].codelen]++;
	}

	nbres[0]=0;

	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=29;n++)
	{
		len=ALPHABETDISTANCE[n].codelen;
		if (len==0)
		{
			ALPHABETDISTANCE[n].valeur=0;
			ALPHABETDISTANCElen[n]=0;
		}
		else
		{
			rr=valeurs[len]+nbres[len];
			ALPHABETDISTANCE[n].valeur=rr;

			rr1=rr;
			rr=0;
			for (n0=0;n0<len;n0++)
			{
				ALPHABETDISTANCE[n].Bits[n0]=bit_n(rr,len-n0-1);
				rr=rr<<1;
				rr0=rr1&1;
				rr+=rr0;

				rr1=rr1>>1;

			}
			ALPHABETDISTANCEdec[n]=rr;
			ALPHABETDISTANCElen[n]=len;

			ALPHABETDISTANCE[n].valeurdec=rr;

			nbres[len]++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETDISTANCE_fixed()
{
	unsigned int n,n0,rr,rr0,rr1;
	for (n=0;n<=29;n++)
	{
		ALPHABETDISTANCE[n].codelen=5;
		ALPHABETDISTANCE[n].valeur=n;
		rr=0;
		rr1=n;
		for (n0=0;n0<5;n0++)
		{
			ALPHABETDISTANCE[n].Bits[n0]=bit_n(n,5-n0-1);

			rr=rr<<1;
			rr0=rr1&1;
			rr+=rr0;

			rr1=rr1>>1;

		}
		ALPHABETDISTANCEdec[n]=rr;
		ALPHABETDISTANCElen[n]=5;


	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int LZCOMPRESS::DEFLATE_block(char * dataBLOCK,char * ImageDATA,unsigned int piDATA0)
{
	unsigned int piDATA;
	int r;
	unsigned int n;
	unsigned int LAST,ALGO,HLIT,HDIST,HCLEN;
	unsigned int tab[500];
	int nn;
	unsigned int nnn;
	int order[19]={ 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
	unsigned int p;
	unsigned int pp;
	unsigned int a;
	int nBLOCKS=0;
	unsigned int tempp;
	int back;

	tempp=0;

	piDATA=piDATA0;

	LAST=0;

	while ((LAST==0)&&(piDATA<DEFLATE_LONGUEUR_deflated))
	{
        nBLOCKS++;
        LAST=READBIT(dataBLOCK,tempp+0);		// HEADER
        ALGO=READBITS(dataBLOCK,tempp+1,2);
        HLIT=READBITS(dataBLOCK,tempp+3,5);
        HDIST=READBITS(dataBLOCK,tempp+8,5);
        HCLEN=READBITS(dataBLOCK,tempp+13,4);

        //17
        r=0;

        if (ALGO==2)
        {

            for (n=0;n<HCLEN+4;n++)
            {
                tab[n]=READBITS(dataBLOCK,tempp+17+3*n,3);
            }
            // 17 + (HCLEN+4)*3


            for (n=0;n<19;n++) ALPHABETCODELENGTHS[n].codelen=0;
            for (n=0;n<HCLEN+4;n++) ALPHABETCODELENGTHS[order[n]].codelen=tab[n];
            for (n=0;n<19;n++) tab[n]=0;

            ALPHABETCODELENGTHS_();

            p=tempp+17+(HCLEN+4)*3;
            pp=0;

            while (pp<HLIT+257+HDIST+1)
            {
                a=decodeCODELENGTHS(dataBLOCK,p);
                p+=increment;

                if (a<16)
                {
                    tab[pp]=a;
                    pp++;

                }
                else
                {
                    if (a==16)
                    {
                        nnn=READBITS(dataBLOCK,p,2) +3;
                        for (nn=0;nn<(int)nnn;nn++)
                        {
                            tab[pp+nn]=tab[pp-1];
                        }
                        pp+=nnn;
                        p+=2;

                    }
                    else
                    if (a==17)
                    {
                        nnn=READBITS(dataBLOCK,p,3) +3;
                        for (nn=0;nn<(int)nnn;nn++)
                        {
                            tab[pp+nn]=0;
                        }
                        pp+=nnn;
                        p+=3;

                    }
                    else
                    {
                        nnn=READBITS(dataBLOCK,p,7) +11;
                        for (nn=0;nn<(int)nnn;nn++)
                        {
                            tab[pp+nn]=0;
                        }
                        pp+=nnn;
                        p+=7;

                    }
                }
            }

            for (n=0;n<=285;n++)
                ALPHABETLITTERAL[n].codelen=0;

            for (n=0;n<HLIT+257;n++)
                ALPHABETLITTERAL[n].codelen=tab[n];

            for (n=0;n<32;n++)
                ALPHABETDISTANCE[n].codelen=0;

            for (n=0;n<HDIST+1;n++)
                ALPHABETDISTANCE[n].codelen=tab[n+HLIT+257];

            ALPHABETLITTERAL_();
            ALPHABETDISTANCE_();

        }
        else
        {
            if (ALGO==1)
            {
                p=tempp+3;
                ALPHABETLITTERAL_fixed();
                ALPHABETDISTANCE_fixed();
            }
            else
            {
                // 00
                unsigned int LEN,NLEN;
                int c1,c2;
                tempp+=3;
                tempp=(tempp+7)&(0xFFFFFFFF - 7);
                pp=tempp>>3;


                c1=dataBLOCK[pp]&255;
                c2=dataBLOCK[pp+1]&255;

                LEN=c1 + (c2<<8);
                NLEN=0;

                for (n=0;n<LEN;n++)
                {
                    ImageDATA[(unsigned int)piDATA]=dataBLOCK[(unsigned int)pp+4+n];
                    piDATA++;
                }

                p=tempp+(LEN+4)*8;
            }
        }

        a=0;

        if (ALGO!=0) LZ_Init_tree();

        while ((a!=256)&&(ALGO!=0)&&(piDATA<DEFLATE_LONGUEUR_deflated))
        {

            a=decodeLITTERAL_LENGTH(dataBLOCK,p);
            p+=increment;

            if (a<256)
            {
                if (piDATA<DEFLATE_LONGUEUR_deflated)
                {
                    ImageDATA[piDATA]=a;
                    piDATA++;
                }
            }
            else
            if (a!=256)
            {
                a=decodeDISTANCE(dataBLOCK,p);
                p+=increment;
                back=piDATA-DISTANCE;

                if (piDATA+__LENGTH>DEFLATE_LONGUEUR_deflated)
                    __LENGTH=DEFLATE_LONGUEUR_deflated-piDATA;

                for (nn=0;nn<(int)__LENGTH;nn++)
                {
                    ImageDATA[piDATA]=ImageDATA[back+nn];
                    piDATA++;
                }

            }

        }

        if (ALGO!=0) LZ_Free_tree();
        tempp=p;
	}

	return piDATA;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int LZCOMPRESS::DEFLATE_zlibBlock(char * dataBLOCK,char * ImageDATA,int piDATA)
{
	int CM,CINFO,FCHECK,FDICT,FLEVEL;
	char c;
	int n;

	c=dataBLOCK[0];
	CM=c&15;
	CINFO=(c>>4)&15;
	c=dataBLOCK[1];
	FCHECK=c&31;
	FDICT=(c>>5)&1;
	FLEVEL=(c>>6)&3;

	LZ_Init_Tab_Nodes();
	n=DEFLATE_block(&dataBLOCK[2],ImageDATA,piDATA);
	LZ_Free_Tab_Nodes();

	return n;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ENCODAGE
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::CodeLength(unsigned int len)
{
	unsigned int n;
	int res;
	n=0;
	res=-1;
	while ((n<29)&&(res==-1))
	{
		if ((len>=LENGTH_LENGTH[n])&&(len<LENGTH_LENGTH[n+1]))
			res=n;
		else n++;

	}
	n+=257;
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::CodeDistance(unsigned int dist)
{
	unsigned int n;
	int res;
	n=0;
	res=-1;
	while ((n<30)&&(res==-1))
	{
		if ((dist>=DIST_DIST[n])&&(dist<DIST_DIST[n+1]))
			res=n;
		else n++;

	}
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETLITTERAL_encode_fixed()
{
	unsigned int n,nn;
	unsigned int rr,rrr;
	unsigned int len;

	for (n=0;n<=143;n++)
	{
		ALPHABETLITTERALlen[n]=8;
		ALPHABETLITTERALdec[n]=48+n;
	}
	for (n=144;n<=255;n++)
	{
		ALPHABETLITTERALlen[n]=9;
		ALPHABETLITTERALdec[n]=400+(n-144);
	}
	for (n=256;n<=279;n++)
	{
		ALPHABETLITTERALlen[n]=7;
		ALPHABETLITTERALdec[n]=(n-256);
	}
	for (n=280;n<=287;n++)
	{
		ALPHABETLITTERALlen[n]=8;
		ALPHABETLITTERALdec[n]=192+(n-280);
	}

	for (n=0;n<=285;n++)
	{
		len=ALPHABETLITTERALlen[n];
		if (len==0)
		{
			ALPHABETLITTERALdec[n]=0;
		}
		else
		{
			rr=ALPHABETLITTERALdec[n];


			rrr=0;
			for (nn=0;nn<len;nn++)
			{
				rrr=rrr<<1;
				rrr+=(rr&1);
				rr=rr>>1;
			}
			ALPHABETLITTERALdec[n]=rrr;

		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETLITTERAL_encode()
{
	unsigned int n,nn;
	unsigned int rr,rrr;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;
	for (n=0;n<=285;n++)
	{
		nbres[ALPHABETLITTERALlen[n]]++;
	}
	nbres[0]=0;
	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=285;n++)
	{
		len=ALPHABETLITTERALlen[n];
		if (len==0)
		{
			ALPHABETLITTERALdec[n]=0;
		}
		else
		{
			rr=valeurs[len]+nbres[len];

			rrr=0;
			for (nn=0;nn<len;nn++)
			{
				rrr=rrr<<1;
				rrr+=(rr&1);
				rr=rr>>1;
			}

			ALPHABETLITTERALdec[n]=rrr;
			nbres[len]++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETDISTANCE_encode()
{
	unsigned int n,nn;
	unsigned int rr,rrr;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;
	for (n=0;n<=29;n++)
	{
		nbres[ALPHABETDISTANCElen[n]]++;
	}
	nbres[0]=0;
	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=29;n++)
	{

		len=ALPHABETDISTANCElen[n];
		if (len==0)
		{
			ALPHABETDISTANCEdec[n]=0;
		}
		else
		{
			rr=valeurs[len]+nbres[len];

			rrr=0;
			for (nn=0;nn<len;nn++)
			{
				rrr=rrr<<1;
				rrr+=(rr&1);
				rr=rr>>1;
			}


			ALPHABETDISTANCEdec[n]=rrr;
			nbres[len]++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETDISTANCE_encode_fixed()
{
	unsigned int n,nn;
	unsigned int rr,rrr;
	unsigned int len;


	for (n=0;n<=29;n++)
	{

		ALPHABETDISTANCElen[n]=5;
		len=ALPHABETDISTANCElen[n];
		if (len==0)
		{
			ALPHABETDISTANCEdec[n]=0;
		}
		else
		{
			rr=n;

			rrr=0;
			for (nn=0;nn<len;nn++)
			{
				rrr=rrr<<1;
				rrr+=(rr&1);
				rr=rr>>1;
			}

			ALPHABETDISTANCEdec[n]=rrr;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void LZCOMPRESS::ALPHABETCODELENGTHS_encode()
{
	unsigned int n,nn;
	unsigned int rr,rrr;
	unsigned int nbres[NBITSMAX];
	unsigned int valeurs[NBITSMAX];
	unsigned int len;
	unsigned int valeur;

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;
	for (n=0;n<=18;n++)
	{
		nbres[ALPHABETCODELENGTHSlen[n]]++;
	}
	nbres[0]=0;
	valeur=0;
	for (n=0;n<NBITSMAX;n++)
	{
		valeurs[n]=valeur;
		valeur=(valeur+nbres[n])<<1;
	}

	for (n=0;n<NBITSMAX;n++) nbres[n]=0;

	for (n=0;n<=18;n++)
	{

		len=ALPHABETCODELENGTHSlen[n];
		if (len==0)
		{
			ALPHABETCODELENGTHSdec[n]=0;
		}
		else
		{
			rr=valeurs[len]+nbres[len];
			alphabetcodeslenghtscodes[n]=rr;

			rrr=0;
			for (nn=0;nn<len;nn++)
			{
				rrr=rrr<<1;
				rrr+=(rr&1);
				rr=rr>>1;
			}

			ALPHABETCODELENGTHSdec[n]=rrr;
			nbres[len]++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::nbrerepetitionsids(struct repetition tab[],int p)
{
	int n;
	n=p-1;
	while (tab[n].n_r==tab[p].n_r) n--;
	return (p-n);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int FASTCONV LZCOMPRESS::n_rdifferentzero(struct repetition tab[],int p)
{
	int n;
	n=p;
	while (tab[n].n_r==0) n--;
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::recursCodeLengths(int NBits,struct repetition tab[],int lentab,int Niter,int p,int NbreBits)
{
	int nb;
	int n,n0,n1;
	int res;

	unsigned int gain;
	int valid;
	nb=nbrerepetitionsids(tab,p);
	if (Niter==0)
	{
		for (n=NbreBits;n>3;n--)
		{

			for (n0=p;n0>p-nb;n0--)
				tab[n0].codelen=n;

			recursCodeLengths(n,tab,lentab,Niter+1,p-nb,n);
			if (NbreBits>2)
			recursCodeLengths(n,tab,lentab,Niter+1,p-nb,n-1);

			if (NbreBits>2)
			for (n0=1;n0<nb;n0++)
			{
				for (n1=0;n1<n0;n1++)
					tab[p-n1].codelen=n;
				for (n1=n0;n1<nb;n1++)
					tab[p-n1].codelen=n-1;

				recursCodeLengths(n,tab,lentab,Niter+1,p-nb,n-1);
			}
		}
	}
	else
	{
		if (p>=0)
		{

			for (n0=p;n0>p-nb;n0--)
				tab[n0].codelen=NbreBits;



			recursCodeLengths(NBits,tab,lentab,Niter+1,p-nb,NbreBits);
			if (NbreBits>2)
			recursCodeLengths(NBits,tab,lentab,Niter+1,p-nb,NbreBits-1);

			if ((NbreBits>2)&&(nb>1))
			for (n0=1;n0<nb;n0++)
			{
				for (n1=0;n1<n0;n1++)
					tab[p-n1].codelen=NbreBits;
				for (n1=n0;n1<nb;n1++)
					tab[p-n1].codelen=NbreBits-1;


				recursCodeLengths(NBits,tab,lentab,Niter+1,p-nb,NbreBits-1);
			}

		}
		else
		{
			// calculs
			valid=(1<<NBits);

			n=0;
			res=tab[0].codelen;
			while ((n<lentab)&&(res!=0))
			{
				valid-=(1<<(NBits-res));
				n++;
				res=tab[n].codelen;
			}

			if (valid>0)
			{
				gain=0;
				n=0;
				res=tab[0].codelen;
				while ((n<lentab)&&(res!=0))
				{
					gain+=tab[n].n_r0*res;
					n++;
					res=tab[n].codelen;
				}

				if (gain<GAINretenu)
				{
					n=0;
					res=tab[0].codelen;
					while ((n<lentab))
					{

						TABretenu[n].code=tab[n].code;
						TABretenu[n].codelen=res;
						TABretenu[n].n_r=tab[n].n_r;
						n++;
						res=tab[n].codelen;
					}

					GAINretenu=gain;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define setnbits(c_block,p,valeur,nb) {*(unsigned int*)(&c_block[p>>3])+=(valeur<<(p&7))&0x00FFFFFF;}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int FASTCONV LZCOMPRESS::CalculeGAIN(int len,struct repetition TAB[])
{
	int n,nn,nnn;
	unsigned int gain;

	gain=0;
	nnn=0;
	for (n=0;n<len;n++)
	{
		if (CodeLengths00[n]!=0)
		for (nn=0;nn<CodeLengths00[n];nn++)
		{
			gain+=TAB[nnn].n_r0*n;
			nnn++;
		}		
	}

	return gain;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::PrepareCodeLengths()
{
	int n;
	for (n=0;n<MAXLENCODE;n++)
	{
		CodeLengths00[n]=0;
		CodeLengthsRetenus[n]=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::RetiensCodeLengths(unsigned int GAIN,int etage)
{
	int n;

	for (n=0;n<MAXLENCODE;n++)
		CodeLengthsRetenus[n]=0;

	for (n=0;n<etage;n++)
	{
		CodeLengthsRetenus[n]=CodeLengths00[n];
	}
	
	GAINretenu=GAIN;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::StockeCodeLengths(int len)
{
	int n,nn,nnn;
	unsigned int gain;

	gain=0;
	nnn=0;
	for (n=0;n<len;n++)
	{
		for (nn=0;nn<CodeLengthsRetenus[n];nn++)
		{
			TABretenu[nnn].codelen=n;
			nnn++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void FASTCONV LZCOMPRESS::recursCodelengths2(int nb,int etage,int N,struct repetition TAB[])
{
	int n,np;
	unsigned int GAIN;

	if (etage<NBITSMAX)
	{
		if (N>=nb)
		{
			CodeLengths00[etage]=nb;
			GAIN=CalculeGAIN(etage+1,TAB);
			if (GAIN<GAINretenu) RetiensCodeLengths(GAIN,etage+1);
		}
		else
		{
			if (etage<7)
			{	
				for (n=0;n<N-1;n++)
				{
					CodeLengths00[etage]=n;
					recursCodelengths2(nb-n,etage+1,(N-n)*2,TAB);
				}
			}
			else
			{
				n=N/2;
				CodeLengths00[etage]=n;
				recursCodelengths2(nb-n,etage+1,(N-n)*2,TAB);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int LZCOMPRESS::CodageHuffman(unsigned int * datasLZ77,unsigned int LZ77offset,unsigned int len,char * c_blocks,unsigned int c_offset,unsigned int status)
{
	int nbitsdepart;
	unsigned int n,nn,n0,n1,p;
	int dec,dist;
	unsigned int code,nr;
	struct repetition r_litteral[286];
	struct repetition r_dist[30];
	struct repetition r_codelen[20];
	int departn_r;
	int alphabetcodelength[500];

	int tab[500];
	int litteralcodelen[286];
	int distancecodelen[30];
	int codelengthscodelen[20];

	for (n=0;n<=285;n++)
	{
		r_litteral[n].code=n;
		r_litteral[n].codelen=0;
		r_litteral[n].n_r=0;
	}

	r_litteral[256].n_r=1;

	for (n=0;n<=29;n++)
	{
		r_dist[n].code=n;
		r_dist[n].codelen=0;
		r_dist[n].n_r=0;
	}

	nn=0;
	for (n=0;n<len;n++)
	{
		dec=datasLZ77[LZ77offset+nn];
		r_litteral[dec].n_r++;
		if (dec<257)
		{
			nn++;
			LZ77distance++;
		}
		else
		{
			dist=datasLZ77[LZ77offset+nn+1];
			r_dist[dist].n_r++;
			LZ77distance+=datasLZ77[LZ77offset+nn+2] + LENGTH_LENGTH[dec-257];
			nn+=4;
		}
	}

	LZ77retoffset=nn+LZ77offset;


	for (n0=0;n0<=285;n0++)
	{
		for (n1=n0+1;n1<=285;n1++)
		{
			if (r_litteral[n1].n_r>r_litteral[n0].n_r)
			{
				code=r_litteral[n1].code;
				r_litteral[n1].code=r_litteral[n0].code;
				r_litteral[n0].code=code;

				nr=r_litteral[n1].n_r;
				r_litteral[n1].n_r=r_litteral[n0].n_r;
				r_litteral[n0].n_r=nr;
			}
		}
	}


	for (n0=0;n0<=29;n0++)
	{
		for (n1=n0+1;n1<=29;n1++)
		{
			if (r_dist[n1].n_r>r_dist[n0].n_r)
			{
				code=r_dist[n1].code;
				r_dist[n1].code=r_dist[n0].code;
				r_dist[n0].code=code;

				nr=r_dist[n1].n_r;
				r_dist[n1].n_r=r_dist[n0].n_r;
				r_dist[n0].n_r=nr;
			}
		}
	}


	n0=n_rdifferentzero(r_litteral,285);

	int nb_litt,nb_dist,ng;

	nb_litt=0;
	for (ng=0;ng<=285;ng++)
		nb_litt+=r_litteral[ng].n_r;

	nb_dist=0;
	for (ng=0;ng<=29;ng++)
		nb_dist+=r_dist[ng].n_r;

	TAG_HUFFMAN_CODES=1;

	if ((nb_dist==0)||(nb_litt==0)) TAG_HUFFMAN_CODES=0;

	if (FORCE_PREDEFCODES)
	{
		TAG_HUFFMAN_CODES=0;
	}

	if (TAG_HUFFMAN_CODES==1)
	{
        for (n=0;n<=285;n++)
        {
            TABretenu[n].code=r_litteral[n].code;
            TABretenu[n].codelen=r_litteral[n].codelen;
            TABretenu[n].n_r=r_litteral[n].n_r;
        }

        departn_r=r_litteral[0].n_r;
        for (n=0;n<=285;n++)
        {
            r_litteral[n].n_r0=r_litteral[n].n_r;
        }

        departn_r=r_dist[0].n_r;
        for (n=0;n<=29;n++)
        {
            r_dist[n].n_r0=r_dist[n].n_r;
        }

        n=n_rdifferentzero(r_litteral,285);
        GAINretenu=0xFFFFFFFF;


        PrepareCodeLengths();
        recursCodelengths2(n+1,1,2,r_litteral);

        StockeCodeLengths(MAXLENCODE-1);
        nn=0;
        for (n=0;n<=285;n++)
        {
            n0=0;
            while (TABretenu[n0].code!=n) n0++;
            litteralcodelen[n]=TABretenu[n0].codelen;	
            tab[nn]=TABretenu[n0].codelen;
            nn++;
        }

        for (n=0;n<=29;n++)
        {
            TABretenu[n].code=r_dist[n].code;
            TABretenu[n].codelen=r_dist[n].codelen=0;
            TABretenu[n].n_r=r_dist[n].n_r;
        }

        n=n_rdifferentzero(r_dist,29);
        GAINretenu=0xFFFFFFFF;


        PrepareCodeLengths();
        recursCodelengths2(n+1,1,2,r_dist);
        StockeCodeLengths(MAXLENCODE-1);

        for (n=0;n<=29;n++)
        {
            n0=0;
            while (TABretenu[n0].code!=n) n0++;
            distancecodelen[n]=TABretenu[n0].codelen;
            tab[nn]=TABretenu[n0].codelen;
            nn++;
        }
        // RLE code lengths

        unsigned int LONGUEUR_CODELENGTHS;
        unsigned int LONGUEUR_RLE;

        LONGUEUR_CODELENGTHS=nn;

        n1=0;
        n=0;
        while (n<nn)
        {
            if (tab[n]==0)
            {
                n0=1;
                while ((n0<138)&&(tab[n+n0]==0)&&(n0+n<nn)) n0++;

                // zero de fin
                {
                    if (n0>=3)
                    {
                        if (n0>10)
                        {
                            alphabetcodelength[n1]=18+256*n0;
                            n1++;
                            n+=n0;
                        }
                        else
                        {
                            alphabetcodelength[n1]=17+256*n0;
                            n1++;
                            n+=n0;
                        }
                    }
                    else
                    {
                        alphabetcodelength[n1]=tab[n];
                        n1++;
                        n++;
                    }
                }
            }
            else
            {
				if (n>0)
				{
					n0=1;
					while ((n0<6)&&(tab[n+n0]==tab[n])) n0++;

					if ((tab[n]==tab[n-1]))
					{
						if (n0>=3)
						{
							alphabetcodelength[n1]=16+256*n0;
							n1++;
							n+=n0;
						}
						else
						{
							alphabetcodelength[n1]=tab[n];
							n1++;
							n++;

						}
					}
					else
					{
						if (n0-1>=3)
						{
							alphabetcodelength[n1]=tab[n];
							n1++;
							alphabetcodelength[n1]=16+256*(n0-1);
							n1++;
							n+=n0;
						}
						else
						{
							alphabetcodelength[n1]=tab[n];
							n1++;
							n++;
						}

					}
				}
				else
				{
					alphabetcodelength[n1]=tab[n];
					n1++;
					n++;
				}
            }
        }

        LONGUEUR_RLE=n1;
        nn=n1;

        for (n=0;n<19;n++)
        {
            r_codelen[n].code=n;
            r_codelen[n].codelen=0;
            r_codelen[n].n_r=0;
            r_codelen[n].n_r0=0;
        }

        for (n=0;n<n1;n++)
        {
            dec=alphabetcodelength[n]&255;
            r_codelen[dec].n_r++;
        }

        for (n0=0;n0<19;n0++)
        {
            for (n1=0;n1<19;n1++)
            {
                if (r_codelen[n1].n_r<r_codelen[n0].n_r)
                {
                    code=r_codelen[n1].code;
                    r_codelen[n1].code=r_codelen[n0].code;
                    r_codelen[n0].code=code;

                    nr=r_codelen[n1].n_r;
                    r_codelen[n1].n_r=r_codelen[n0].n_r;
                    r_codelen[n0].n_r=nr;
                }
            }
        }

        for (n=0;n<19;n++)
        {
            r_codelen[n].n_r0=r_codelen[n].n_r;
            TABretenu[n].code=r_codelen[n].code;
            TABretenu[n].codelen=0;
        }

        n=n_rdifferentzero(r_codelen,18);
        GAINretenu=0xFFFFFFFF;

        PrepareCodeLengths();
        recursCodelengths2(n+1,1,2,r_codelen);
        StockeCodeLengths(MAXLENCODE-1);
        for (n=0;n<=285;n++)
            ALPHABETLITTERALlen[n]=litteralcodelen[n];

        for (n=0;n<=29;n++)
            ALPHABETDISTANCElen[n]=distancecodelen[n];


        for (n=0;n<=18;n++)
        {
            n0=0;
            while (TABretenu[n0].code!=n) n0++;
            codelengthscodelen[n]=TABretenu[n0].codelen;
        }

        for (n=0;n<=18;n++)
            ALPHABETCODELENGTHSlen[n]=codelengthscodelen[n];

        ALPHABETDISTANCE_encode();
        ALPHABETLITTERAL_encode();
        ALPHABETCODELENGTHS_encode();

        if (TAG_HUFFMAN_CODES==1)
        {
            unsigned int HLIT,HDIST,HCLEN;

            HLIT=286-257;
            HDIST=30-1;

            int order[19]={ 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

            n=18;
            while (codelengthscodelen[order[n]]==0) n--;

            HCLEN=n+1-4;

            HLIT=286-257;

            n0=alphabetcodelength[LONGUEUR_RLE-1]&255;
            n1=alphabetcodelength[LONGUEUR_RLE-1]/256;


            if ((n0!=18)&&(n0!=17))
                HDIST=30-1;
            else
            {
                HDIST=30-1-n1;
                LONGUEUR_RLE--;
            }


            p=c_offset;

            // header block ----------------------------------------------------

            setnbits(c_blocks,p,status,1);
            p++;
            n=2;
            setnbits(c_blocks,p,n,2);
            p+=2;

            setnbits(c_blocks,p,HLIT,5);
            setnbits(c_blocks,p+5,HDIST,5);
            setnbits(c_blocks,p+10,HCLEN,4);
            p+=14;

            // code lengths lengths --------------------------------------------

            for (n=0;n<HCLEN+4;n++)
            {
                nn=codelengthscodelen[order[n]]&7;
                setnbits(c_blocks,p,nn,3);
                p+=3;
            }

            // RLE -------------------------------------------------------------

            for (n=0;n<LONGUEUR_RLE;n++)
            {
                n0=alphabetcodelength[n]&255;
                n1=alphabetcodelength[n]/256;

                if (n0<16)
                {
                    nn=ALPHABETCODELENGTHSdec[n0];
                    setnbits(c_blocks,p,nn,ALPHABETCODELENGTHSlen[n0]);
                    p+=ALPHABETCODELENGTHSlen[n0];
                }
                else
                {
                    if (n0==16)
                    {
                        nn=ALPHABETCODELENGTHSdec[n0];
                        setnbits(c_blocks,p,nn,ALPHABETCODELENGTHSlen[n0]);
                        p+=ALPHABETCODELENGTHSlen[n0];
                        n1=n1-3;
                        setnbits(c_blocks,p,n1,2);
                        p+=2;

                    }
                    else
                    if (n0==17)
                    {
                        nn=ALPHABETCODELENGTHSdec[n0];
                        setnbits(c_blocks,p,nn,ALPHABETCODELENGTHSlen[n0]);
                        p+=ALPHABETCODELENGTHSlen[n0];
                        n1=n1-3;
                        setnbits(c_blocks,p,n1,3);
                        p+=3;
                    }
                    else
                    {
                        nn=ALPHABETCODELENGTHSdec[n0];
                        setnbits(c_blocks,p,nn,ALPHABETCODELENGTHSlen[n0]);
                        p+=ALPHABETCODELENGTHSlen[n0];
                        n1=n1-11;
                        setnbits(c_blocks,p,n1,7);
                        p+=7;
                    }
                }
            }

        }
    }


    if (TAG_HUFFMAN_CODES==0)
    {

        ALPHABETDISTANCE_encode_fixed();
        ALPHABETLITTERAL_encode_fixed();

        p=c_offset;
        setnbits(c_blocks,p,status,1);
        p++;
        n=1;
        setnbits(c_blocks,p,n,2);
        p+=2;
    }

	// LZ77 encoding ---------------------------------------------------

	nn=0;
	for (n=0;n<len;n++)
	{
		dec=datasLZ77[LZ77offset+nn];
		if (dec<257)
		{
			n0=ALPHABETLITTERALdec[dec];
			setnbits(c_blocks,p,n0,ALPHABETLITTERALlen[dec]);
			p+=ALPHABETLITTERALlen[dec];
			nn++;
		}
		else
		{

			n0=ALPHABETLITTERALdec[dec];
			setnbits(c_blocks,p,n0,ALPHABETLITTERALlen[dec]);
			p+=ALPHABETLITTERALlen[dec];

  			if (LENGTH_EXTRABITS[dec-257]!=0)
			{
				n0=datasLZ77[LZ77offset+nn+2];
				setnbits(c_blocks,p,n0,LENGTH_EXTRABITS[dec-257]);
				p+=LENGTH_EXTRABITS[dec-257];
			}

			dist=datasLZ77[LZ77offset+nn+1];

			n0=ALPHABETDISTANCEdec[dist];
			setnbits(c_blocks,p,n0,ALPHABETDISTANCElen[dist]);
			p+=ALPHABETDISTANCElen[dist];

			if (DIST_EXTRABITS[dist]!=0)
			{
				n0=datasLZ77[LZ77offset+nn+3];
				setnbits(c_blocks,p,n0,DIST_EXTRABITS[dist]);
				p+=DIST_EXTRABITS[dist];
			}

			nn+=4;
		}
	}

	n0=ALPHABETLITTERALdec[256];
	setnbits(c_blocks,p,n0,ALPHABETLITTERALlen[256]);
	p+=ALPHABETLITTERALlen[256];

	return p;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define test_bit_ptr00(ptr,adr) (READBIT(ptr,adr))
/////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void set_bit_ptr00(char * ptr,unsigned int adr)
{
	if (READBIT(ptr,adr)==0) setnbits(ptr,adr,1,1);

	//(char) ptr[adr]=(char) 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LEN_PTRCHAINES 256*256*256
/////////////////////////////////////////////////////////////////////////////////////////////////////////
char * LZCOMPRESS::LZ77_Huffman0(unsigned char * ptrDATA,unsigned int len,int REPETITION,int DEPTH)
{
	char * ptrCHAINES;
	int res;
	unsigned int adr;
#ifndef C_ASM_COMPRESS
	unsigned int actual0,passe0;
#endif
	unsigned int n,nn,n0;
	unsigned int * block;
	unsigned int p;
	unsigned int a;
	unsigned int LEN_LZ77;
	unsigned int distance;
    int longueur;
	unsigned int check;
	unsigned int octet;
	char * blocklzhuffman;
	unsigned int octet2,octet3;
	unsigned int MAXBUFFER;
	unsigned int LONGUEUR_LZ77;

	ptrCHAINES=(char *) malloc (LEN_PTRCHAINES);
	check=0;
	MAXBUFFER=LEN_CODAGEHUFFMAN*8;
	
	LEN_LZ77=0;
	LZ77retoffset=0;

	block=(unsigned int *) malloc (MAXBUFFER*4);
	p=0;

	for (n=0;n<LEN_PTRCHAINES;n++) ptrCHAINES[n]=0;

	blocklzhuffman=(char *) malloc(len*4);
	for (n=0;n<len*4;n++) blocklzhuffman[n]=0;
	nn=0;

	n=0;
	while (n<len)
	{
		if ((n>4)&&(n+3<len))
		{
			octet=ptrDATA[n];
			octet2=ptrDATA[n+1];
			octet3=ptrDATA[n+2];

			adr=( octet<<16) + (octet2) + (octet3<<8);
			if (test_bit_ptr00(ptrCHAINES,adr)!=0) res=1; else res=0;

			if (res==1)
			{
				actual0=(*(unsigned int*)&(ptrDATA[n]))&0x00FFFFFF;
				n0=n-1;
				if (n>(unsigned int)DEPTH) a=n-DEPTH; else a=0;
				int longueur_max=-1;
				int distance_max,n0_max;
				int count=0;
				distance_max=0;
				n0_max=n0;
				longueur=-1;
				while ((n0>a)&&(count<REPETITION))
				{
					passe0=(*(unsigned int*)&(ptrDATA[n0]))&0x00FFFFFF;
					if (passe0==actual0)
					{
						longueur=3;
						while ((ptrDATA[n+longueur]==ptrDATA[n0+longueur])&&(longueur<257)&&(n+longueur+1<len))
							longueur++;
						distance=n-n0;

						if (longueur>longueur_max)
						{
							count++;
							longueur_max=longueur;
							distance_max=distance;
							n0_max=n0;
						}
					}
						
					n0--;
				}

				longueur=longueur_max;
				distance=distance_max;
				n0=n0_max;

				if (longueur!=-1)
				{
					octet=ptrDATA[n+longueur-2];
					octet2=ptrDATA[n+longueur-1];
					octet3=ptrDATA[n+longueur];
					adr=(octet<<16) + (octet2) + (octet3<<8);
					set_bit_ptr00(ptrCHAINES,adr);

					octet=ptrDATA[n+longueur-3];
					octet2=ptrDATA[n+longueur-2];
					octet3=ptrDATA[n+longueur-1];
					adr=(octet<<16) + (octet2) + (octet3<<8);
					set_bit_ptr00(ptrCHAINES,adr);
				}
				else // longueur == -1
				{
					longueur=-1;
					adr=( octet<<16) + (octet2) + (octet3<<8);
					set_bit_ptr00(ptrCHAINES,adr);
				}


			}
			else // res=0
			{
				longueur=-1;

				adr=( octet<<16) + (octet2) + (octet3<<8);
				set_bit_ptr00(ptrCHAINES,adr);
			}
		}	// n<4
		else
		{
			if (n+3<len)
			{
				octet=ptrDATA[n];
				octet2=ptrDATA[n+1];
				octet3=ptrDATA[n+2];
				adr=( octet<<16) + (octet2) + (octet3<<8);
				set_bit_ptr00(ptrCHAINES,adr);
			}

			longueur=-1;
		}

		if (longueur==-1)
		{
			// litteral
			if (p+1<MAXBUFFER)
			{
				block[p]=ptrDATA[n];
				p++;
			}
			n++;
		}
		else
		{
			// distance,longueur
			if (p+4<MAXBUFFER)
			{
				block[p]=CodeLength(longueur);
				block[p+1]=CodeDistance(distance);
				block[p+2]=longueur-LENGTH_LENGTH[block[p]-257];
				block[p+3]=distance-DIST_DIST[block[p+1]];
				p+=4;
			}
			n+=longueur;
		}

		LEN_LZ77++;

		if (LEN_LZ77==LEN_CODAGEHUFFMAN)
		{
			LONGUEUR_LZ77=LEN_LZ77;
			nn=CodageHuffman(block,0,LEN_CODAGEHUFFMAN,blocklzhuffman,nn,0);
			LEN_LZ77=0;
			p=0;
		}
	}

	if (LEN_LZ77>0)
	{
		LONGUEUR_LZ77=LEN_LZ77;
		nn=CodageHuffman(block,0,LEN_LZ77,blocklzhuffman,nn,0);
		LEN_LZ77=0;
		p=0;
	}

	LEN_LZ77_Huffman=(nn>>3) +1;
	free(ptrCHAINES);
	free(block);

	return blocklzhuffman;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
char * LZCOMPRESS::QuickLZ77_Huffman(char * ptrDATA,unsigned int len)
{
	FORCE_PREDEFCODES=true;
	return LZ77_Huffman0((unsigned char *)ptrDATA,len,2,256);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
char * LZCOMPRESS::LZ77_Huffman(char * ptrDATA,unsigned int len)
{
	FORCE_PREDEFCODES=false;
	return LZ77_Huffman0((unsigned char *)ptrDATA,len,12,8192);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MISC
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateWav(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	char * ptrDATA2;
	char * ptrDATA0;
	unsigned int n,nn;
	

	NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	ptrDATA2=(char *) malloc(size);
	ptrDATA0=(char *) malloc(size);

	ptrDATA2[0]=ptrDATA[0];
	ptrDATA2[1]=ptrDATA[1];
	for (n=2;n<size;n++)
	{
		nn=(ptrDATA[n]&255) - (ptrDATA[n-2]&255);
	    ptrDATA2[n]=nn&255;
	}

	nn=0;
	for (n=0;n<size/2;n++)
	{
		ptrDATA0[nn]=ptrDATA2[n*2]&255;
		nn++;
	}
	for (n=0;n<size/2;n++)
	{
		ptrDATA0[nn]=ptrDATA2[n*2+1]&255;
		nn++;
	}


	NFO->BlockPtr=TheClassForCompatibility.LZ77_Huffman(ptrDATA0,size);
	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Type=INFLATE_WAV;
	NFO->Size=size;

	free(ptrDATA2);
	free(ptrDATA0);

	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateSub(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	char * ptrDATA2;
	unsigned int n,nn;
	

	NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	ptrDATA2=(char *) malloc(size);

	ptrDATA2[0]=ptrDATA[0];
	for (n=1;n<size;n++)
	{
		nn=(ptrDATA[n]&255) - (ptrDATA[n-1]&255);
	    ptrDATA2[n]=nn&255;
	}

	NFO->BlockPtr=TheClassForCompatibility.QuickLZ77_Huffman(ptrDATA2,size);
	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Type=INFLATE_SUB;
	NFO->Size=size;

	free(ptrDATA2);

	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateSub4(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	char * ptrDATA2;
	unsigned int n,nn;
	

	NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	ptrDATA2=(char *) malloc(size);

	ptrDATA2[0]=ptrDATA[0];
	ptrDATA2[1]=ptrDATA[1];
	ptrDATA2[2]=ptrDATA[2];
	ptrDATA2[3]=ptrDATA[3];

	for (n=4;n<size;n++)
	{
		nn=(ptrDATA[n]&255) - (ptrDATA[n-4]&255);
	    ptrDATA2[n]=nn&255;
	}

	NFO->BlockPtr=TheClassForCompatibility.LZ77_Huffman(ptrDATA2,size);
	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Type=INFLATE_SUB4;
	NFO->Size=size;

	free(ptrDATA2);

	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateSub3(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	char * ptrDATA2;
	unsigned int n,nn;
	

	NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	ptrDATA2=(char *) malloc(size);

	ptrDATA2[0]=ptrDATA[0];
	ptrDATA2[1]=ptrDATA[1];
	ptrDATA2[2]=ptrDATA[2];

	for (n=3;n<size;n++)
	{
		nn=(ptrDATA[n]&255) - (ptrDATA[n-3]&255);
	    ptrDATA2[n]=nn&255;
	}

	NFO->BlockPtr=TheClassForCompatibility.LZ77_Huffman(ptrDATA2,size);
	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Type=INFLATE_SUB3;
	NFO->Size=size;

	free(ptrDATA2);

	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)||defined(LINUX)
#ifdef API3D_VR
#include "../../zlib/zlib.h"
#else
#ifdef API3D_SDL2
#include "../../zlib/zlib.h"
#else
#include "../zlib/zlib.h"
#endif
#endif
#else
#ifdef WINDOWS_STORE
#include "../../zlib/zlib.h"
#else
#ifdef API3D_SDL2
#include "../../zlib/zlib.h"
#else
#include "zlib.h"
#endif
#endif
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateNormal(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	
#ifdef LZCOMPRESS_CLASS
    NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	NFO->BlockPtr=TheClassForCompatibility.LZ77_Huffman(ptrDATA,size);

	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Size=size;
	NFO->Type=INFLATE_NORMAL;

#else
    NFO=(struct Inflate*) malloc(sizeof(struct Inflate));
    
    char *ptr=ptrDATA;
    
    uLongf sz=(int)(size*1.5f);
    char *zlib=(char*) malloc(sz);
    compress((Bytef*)zlib,&sz,(Bytef*)ptr,size);
    
    NFO->BlockPtr=(char*)malloc(sz-2);
    memcpy(NFO->BlockPtr,&zlib[2],sz-2);
    
    free(zlib);
    
    NFO->C_Size=sz-2;
    NFO->Size=size;
    NFO->Type=INFLATE_NORMAL;
#endif
    
	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateQuick(char * ptrDATA,unsigned int size)
{
	struct Inflate * NFO;
	
#ifdef LZCOMPRESS_CLASS
	NFO=(struct Inflate*) malloc(sizeof(struct Inflate));

	NFO->BlockPtr=TheClassForCompatibility.QuickLZ77_Huffman(ptrDATA,size);

	NFO->C_Size=TheClassForCompatibility.LEN_LZ77_Huffman;
	NFO->Size=size;
	NFO->Type=INFLATE_NORMAL;
#else
    NFO=(struct Inflate*) malloc(sizeof(struct Inflate));
    
    char *ptr=ptrDATA;
    
    uLongf sz=(int)(size*1.5f);
    char *zlib=(char*) malloc(sz);
    compress2((Bytef*)zlib,&sz,(Bytef*)ptr,size,Z_BEST_SPEED);
    
    NFO->BlockPtr=(char*)malloc(sz-2);
    memcpy(NFO->BlockPtr,&zlib[2],sz-2);
    
    free(zlib);
    
    NFO->C_Size=sz-2;
    NFO->Size=size;
    NFO->Type=INFLATE_NORMAL;
#endif

	return (NFO);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Inflate * InflateDatas(char * ptrDATA,unsigned int size,char Type)
{
	struct Inflate * NFOS;
	if (Type==INFLATE_NORMAL) NFOS=InflateNormal(ptrDATA,size);
	if (Type==INFLATE_WAV)	NFOS=InflateWav(ptrDATA,size);
	return (NFOS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
char * DeflateDatas(struct Inflate * NFOS)
{
	unsigned int len;
	char * DeflatedDATAS;
	char * DeflatedDATAS2;
	char * DeflatedDATAS0;
	unsigned int n,nn,nnn;
	
	DeflatedDATAS=(char *) malloc(NFOS->Size);
	DeflatedDATAS0=(char *) malloc(NFOS->Size);
	DeflatedDATAS2=(char *) malloc(NFOS->Size);
    
    LZCOMPRESS *lz=new LZCOMPRESS;
    
    lz->DEFLATE_LONGUEUR_deflated=NFOS->Size;
        
    lz->LZ_Init_Tab_Nodes();
    len=lz->DEFLATE_block(NFOS->BlockPtr,DeflatedDATAS,0);
    lz->LZ_Free_Tab_Nodes();
    
    delete lz;

    len=NFOS->Size;

	if (NFOS->Type==INFLATE_WAV)		// wav
	{

		nn=0;
		nnn=len/2;
		for (n=0;n<nnn;n++)
		{
			DeflatedDATAS0[nn]=DeflatedDATAS[n]&255;
			nn++;
			DeflatedDATAS0[nn]=DeflatedDATAS[n+nnn]&255;
			nn++;
		}

		DeflatedDATAS2[0]=DeflatedDATAS0[0]&255;
		DeflatedDATAS2[1]=DeflatedDATAS0[1]&255;
		for (n=2;n<len;n++)
		{
			nn=DeflatedDATAS0[n]&255;
			DeflatedDATAS2[n]=(nn + DeflatedDATAS2[n-2]&255)&255;
		}
	}


	if (NFOS->Type==INFLATE_SUB)
	{

		DeflatedDATAS2[0]=DeflatedDATAS[0]&255;
		for (n=1;n<len;n++)
		{
			nn=DeflatedDATAS[n]&255;
			DeflatedDATAS2[n]=(nn + DeflatedDATAS2[n-1]&255)&255;
		}
	}

	if (NFOS->Type==INFLATE_SUB4)
	{
		DeflatedDATAS2[0]=DeflatedDATAS[0]&255;
		DeflatedDATAS2[1]=DeflatedDATAS[1]&255;
		DeflatedDATAS2[2]=DeflatedDATAS[2]&255;
		DeflatedDATAS2[3]=DeflatedDATAS[3]&255;

		for (n=4;n<len;n++)
		{
			nn=DeflatedDATAS[n]&255;
			DeflatedDATAS2[n]=(nn + DeflatedDATAS2[n-4]&255)&255;
		}
	}

	if (NFOS->Type==INFLATE_SUB3)
	{
		DeflatedDATAS2[0]=DeflatedDATAS[0]&255;
		DeflatedDATAS2[1]=DeflatedDATAS[1]&255;
		DeflatedDATAS2[2]=DeflatedDATAS[2]&255;

		for (n=3;n<len;n++)
		{
			nn=DeflatedDATAS[n]&255;
			DeflatedDATAS2[n]=(nn + DeflatedDATAS2[n-3]&255)&255;
		}
	}

	if (NFOS->Type==INFLATE_NORMAL)		// normal
	{
		for (n=0;n<len;n++)
		{
			DeflatedDATAS2[n]=DeflatedDATAS[n]&255;
		}
	}

	free(DeflatedDATAS);
	free(DeflatedDATAS0);

	return DeflatedDATAS2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
char * DeflateDatasZLIB(struct Inflate * NFOS)
{
    unsigned int len;
    unsigned char * DeflatedDATAS;
    unsigned char * DeflatedDATAS2;
    unsigned char * DeflatedDATAS0;
    unsigned int n,nn,nnn;
    
    DeflatedDATAS=(unsigned char *) malloc(NFOS->Size);
    DeflatedDATAS0=(unsigned char *) malloc(NFOS->Size);
    DeflatedDATAS2=(unsigned char *) malloc(NFOS->Size);
    
    uLongf tsize=NFOS->Size;
    int ret=uncompress((Bytef*)DeflatedDATAS, &tsize, (Bytef*)NFOS->BlockPtr, NFOS->C_Size+2);

    if (ret!=Z_OK)
    {

		LZCOMPRESS *lz=new LZCOMPRESS;
    
		lz->DEFLATE_LONGUEUR_deflated=NFOS->Size;
        
		lz->LZ_Init_Tab_Nodes();
		len=lz->DEFLATE_block(&NFOS->BlockPtr[2],(char*)DeflatedDATAS,0);
		lz->LZ_Free_Tab_Nodes();
    
		delete lz;
/*
        TheClassForCompatibility.DEFLATE_LONGUEUR_deflated=NFOS->Size;
            
        TheClassForCompatibility.LZ_Init_Tab_Nodes();
        len=TheClassForCompatibility.DEFLATE_block(&NFOS->BlockPtr[2],(char*)DeflatedDATAS,0);
        TheClassForCompatibility.LZ_Free_Tab_Nodes();
/**/
    }
    
    len=NFOS->Size;

    if (NFOS->Type==INFLATE_WAV)        // wav
    {
        nn=0;
        nnn=len/2;
        for (n=0;n<nnn;n++)
        {
            DeflatedDATAS0[nn]=DeflatedDATAS[n];
            nn++;
            DeflatedDATAS0[nn]=DeflatedDATAS[n+nnn];
            nn++;
        }

        DeflatedDATAS2[0]=DeflatedDATAS0[0];
        DeflatedDATAS2[1]=DeflatedDATAS0[1];
        for (n=2;n<len;n++)
        {
            nn=DeflatedDATAS0[n];
            DeflatedDATAS2[n]=nn + DeflatedDATAS2[n-2];
        }
    }

    if (NFOS->Type==INFLATE_SUB)
    {
        DeflatedDATAS2[0]=DeflatedDATAS[0];
        for (n=1;n<len;n++)
        {
            nn=DeflatedDATAS[n];
            DeflatedDATAS2[n]=nn + DeflatedDATAS2[n-1];
        }
    }

    if (NFOS->Type==INFLATE_SUB4)
    {
        DeflatedDATAS2[0]=DeflatedDATAS[0];
        DeflatedDATAS2[1]=DeflatedDATAS[1];
        DeflatedDATAS2[2]=DeflatedDATAS[2];
        DeflatedDATAS2[3]=DeflatedDATAS[3];

        for (n=4;n<len;n++)
        {
            nn=DeflatedDATAS[n];
            DeflatedDATAS2[n]=nn + DeflatedDATAS2[n-4];
        }
    }

    if (NFOS->Type==INFLATE_SUB3)
    {
        DeflatedDATAS2[0]=DeflatedDATAS[0];
        DeflatedDATAS2[1]=DeflatedDATAS[1];
        DeflatedDATAS2[2]=DeflatedDATAS[2];

        for (n=3;n<len;n++)
        {
            nn=DeflatedDATAS[n];
            DeflatedDATAS2[n]=nn + DeflatedDATAS2[n-3];
        }
    }

    if (NFOS->Type==INFLATE_NORMAL)        // normal
    {
        memcpy(DeflatedDATAS2,DeflatedDATAS,len);
    }

    free(DeflatedDATAS);
    free(DeflatedDATAS0);

    return (char*)DeflatedDATAS2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
