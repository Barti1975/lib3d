////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Canc� Francis, 10/08/1975>
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
#define FORCE_MODEL3
////////////////////////////////////////////////////////////////////////
//	@file vertexprogram.cpp
//	@date 2014
////////////////////////////////////////////////////////////////////////

//int __cdecl printf2(const char *format, ...);
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
#include "params.h"
#include <math.h>
#include "vertexprogram.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(GLES20)||defined(GLESFULL)
extern bool OPENGLES30HIGHP;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool STRICTSHADER=false;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
//#define LOG_DEBUG

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FSR HPP

#include "fsr.hpp"

#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int _sprintf(char *s, const char *format, ...)
{
	int done;
	va_list arg;
	char *sd;

	va_start (arg, format);
	char * first = (char*) va_arg( arg, char* );
	if (first==s)
	{
		if ((format[0]=='%')&&(format[1]=='s'))
		{
			sd=&s[strlen(s)];
			done = vsprintf (sd, &format[2], arg);
			va_end (arg);
		}
		else
		{
			sd=(char*)malloc(65536*4);
			va_start (arg, format);
			done = vsprintf (sd, format, arg);
			va_end (arg);
			strcpy(s,sd);
			free(sd);
		}
	}
	else
	{
		va_start (arg, format);
		done = vsprintf (s, format, arg);
		va_end (arg);
	}

	return done;

}

int _sprintf2(char *s, const char *format, ...)
{
	int done;
	va_list arg;
	char *sd;

	va_start (arg, format);
	char * first = (char*) va_arg( arg, char* );
	if (first==s)
	{
		if ((format[0]=='%')&&(format[1]=='s'))
		{
			sd=&s[strlen(s)];
			done = vsprintf (sd, &format[2], arg);
			va_end (arg);
		}
		else
		{
			sd=(char*)malloc(65536*4);
			va_start (arg, format);
			done = vsprintf (sd, format, arg);
			va_end (arg);
			strcpy(s,sd);
			free(sd);
		}
	}
	else
	{
		va_start (arg, format);
		done = vsprintf (s, format, arg);
		va_end (arg);
	}

	return done;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char strings[8192][256];
int nstrings=0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * newString()
{
	return strings[nstrings++];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char strings2[8192][256];
int nstrings2=0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * newString2()
{
	return strings2[nstrings2++];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char valueFSRRCASSample[32]={ 0 };
char value_bumpy_str[32]={ 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FSRCASSampleSharp(float x)
{
#ifdef API3D_OPENGL20
	if (x>555) _sprintf(valueFSRRCASSample,"0.0025");
	else _sprintf(valueFSRRCASSample,"%3.8f",x);
#else
	if (x>555) _sprintf(valueFSRRCASSample,"0.0025f");
	else _sprintf(valueFSRRCASSample,"%3.8ff",x);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SampleBumpy(float x)
{
#ifdef API3D_OPENGL20
	if (x>555) _sprintf(value_bumpy_str,"0.05");
	else _sprintf(value_bumpy_str,"%3.8f",x);
#else
	if (x>555) _sprintf(value_bumpy_str,"0.2");
	else _sprintf(value_bumpy_str,"%3.8f",x);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (disable:4703)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * VP_STR_ERROR[6]={ "SCRIPT_POSITION","SCRIPT_DIFFUSE","SCRIPT_SPECULAR","SCRIPT_MAPPING0","SCRIPT_MAPPING1","SCRIPT_MAPPING2"};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char RIF[8];
char RIFTMP[8];
char RIFTMP2[8];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_int(char * str)
{
    int len=(int) strlen(str);
	for (int n=0;n<len;n++)
        if ((str[n]<'0')||(str[n]>'9')) return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_upcase(char * str)
{
    int len=(int) strlen(str);
	for (int n=0;n<len;n++)
		if ((str[n]>='a')&&(str[n]<='z')) str[n]+='A'-'a';
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_get_color(char * str,float *r,float *g,float *b)
{
	int n=0;
	while (((str[n]<'0')||(str[n]>'9'))&&(str[n]!='-')&&(str[n]!='.')) n++;
	sscanf(&str[n],"%f %f %f",r,g,b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_clean(char * str)
{
	char ss[2048];
	int n,n0;
    int len=(int) strlen(str);
	n0=0;
	for (n=0;n<len;n++)
	{
		if (!((str[n]==' ')||(str[n]==9)||(str[n]==10)||(str[n]=='\n')||(str[n]==13)))
		{
			ss[n0]=str[n];
			n0++;
		}
	}
	ss[n0]='\0';
	n0++;
	for (n=0;n<n0;n++) str[n]=ss[n];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_clean2(char * str)
{
	char ss[2048];
	int n,n0;
    int len=(int) strlen(str);
    
	n0=0;
	for (n=0;n<len;n++)
	{
		if (!((str[n]==9)||(str[n]==10)||(str[n]=='\n')||(str[n]==13)))
		{
			ss[n0]=str[n];
			n0++;
		}
	}
	ss[n0]='\0';
	n0++;
	for (n=0;n<n0;n++) str[n]=ss[n];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_fixed_match(char * str,char * m)
{
    int lenm=(int) strlen(m);
    if ((int) strlen(str)<lenm) return false;

    int n=0;
	while (n<lenm)
	{
        if (str[n]!=m[n]) return false;
		n++;
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_strcmp_match(char * str,char * m)
{
	char ss[128];
	memcpy(ss,str,strlen(m));
	ss[strlen(m)]='\0';
	return (strcmp(ss,m)==0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_char(char * str, char c)		// positionnement du caract�re c / str
{
	int n=0;
    int len=(int)strlen(str);
	while (n<len)
	{
        if (str[n] == c) return n;
		n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_char_count(char * str, char c)		// positionnement du caract�re c / str
{
	int n=0;
	int res=0;
    int len=(int)strlen(str);
	while (n<len)
	{
		if (str[n] == c) res++;
		n++;
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_char_prt(char * str, char c)		// positionnement du caract�re c / str
{
	int level = 0;
	int n = 0;
    int len=(int)strlen(str);
    
	while (n<len)
	{
		if (str[n] == '(') level++;
		if (str[n] == ')') level--;
		if ((str[n] == c) && (level == 0)) return n;
		n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_last_char(char * str,char c)
{
	static char *cr="xyzwrgba";
    int len=(int) strlen(str);
	int n;

	if ((str[0]=='p')&&(str[1]=='i')&&(str[2]=='x')&&(str[3]=='e')&&(str[4]=='l'))
	{
		// pixel.vX.xyzw

		if (str[5]=='.')
		{
			if (str_char(cr,str[6])>=0) n=0;
			else n=6;
		}
		else n=6;
		
		while (n<len)
		{
			if (str[n]==c) return n;
			n++;
		}
	}

	if ((str[0]=='f')&&(str[1]=='l')&&(str[2]=='o')&&(str[3]=='a')&&(str[4]=='t'))
	{
		// floatX(indexed.vX).xyzw
		n=(int)strlen(str)-1;
		while ((str[n]!=c)&&(str[n]!=')')) n--;
		if (str[n]==c) return n;
	}

	if (str[0]=='-')
	{
		if ((str[1]=='i')||(str[1]=='o'))
		{
			if ((str[2]=='n')&&(strlen(str)>8))
			{
				// indexed.vX.xyzw

				if (str[8]=='.')
				{
					if (str_char(cr,str[9])>=0) n=0;
					else n=9;
				}
				else n=9;
		
				while (n<len)
				{
					if (str[n]==c) return n;
					n++;
				}
			}
			else
			{
				if (str[2]=='.')
				{
					if (str_char(cr,str[3])>=0) n=0;
					else n=3;
				}
				else n=3;
		
				while (n<len)
				{
					if (str[n]==c) return n;
					n++;
				}
			}
		}
		else
		{
			int level=0;
			n=0;

			while (n<len)
			{
				if (str[n]=='[') level++;
				if (str[n]==']') level--;
				if ((str[n]==c)&&(level==0)) return n;
				n++;
			}
		}
		return -1;
	}
	else
	if ((str[0]=='i')||(str[0]=='o'))
	{
		if ((str[1]=='n')&&(strlen(str)>7))
		{
			// indexed.vX.xyzw

			if (str[7]=='.')
			{
				if (str_char(cr,str[8])>=0) n=0;
				else n=8;
			}
			else n=8;
		
			while (n<len)
			{
				if (str[n]==c) return n;
				n++;
			}
		}
		else
		{
			if (str[1]=='.')
			{
				if (str_char(cr,str[2])>=0) n=0;
				else n=2;
			}
			else n=2;
		
			while (n<len)
			{
				if (str[n]==c) return n;
				n++;
			}
		}
	}
	else
	{
		int level=0;
		n=0;
		while (n<len)
		{
			if (str[n]=='[') level++;
			if (str[n]==']') level--;
			if ((str[n]==c)&&(level==0)) return n;
			n++;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_match(char * str,char * m)		// teste si la chaine m est presente dans la chaine str
{
	int n=0;
    int len=(int) strlen(str);
    if (len<(int) strlen(m)) return false;
    
	while (n<len)
	{
        if (str_fixed_match(&str[n],m)) return true;
		n++;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_match0(char * str,char * m)        // teste si la chaine m est presente dans la chaine str
{
    char mm[1024];
    _sprintf(mm,"%s(",m);
    if (str_fixed_match(&str[0],mm)) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_match00(char * str,char * m)        // teste si la chaine m est presente dans la chaine str
{
    if (str_fixed_match(&str[0],m)) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_cmp_match(char * str,char * m)
{
    int len=(int)strlen(str);
    int lenm=(int)strlen(m);
    
	if (len>=lenm)
	{
		int n=0;
		while (n<=len-lenm)
		{
            if (str_strcmp_match(&str[n],m)) return true;
			n++;
		}
	}
    return false;
}

int ii_str_cmp_match(char * str,char * m)
{
    int len=(int)strlen(str);
    int lenm=(int)strlen(m);
    
    if (len>=lenm)
    {
        int n=0;
        while (n<=len-lenm)
        {
            if (str_strcmp_match(&str[n],m)) return n;
            n++;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_mm(char *str,char *m)
{
    int len=(int)strlen(m);
    
	if ((int)strlen(str)>=len)
	{
		char ss[1024];
		
		memcpy(ss,str,len);
		ss[len]=0;
		return (strcmp(ss,m)==0);
	}
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char str_TemporaryString[32768];
char str_TemporaryStringprt[32768];
char str_TemporaryString2[32768];
char str_TemporaryStringb[32768];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int VPLINEINSCRIPT=0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse(char * script,int *pos)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		while (((script[*pos]=='\n')||(script[*pos]==' ')||(script[*pos]==9))&&(script[*pos]!='\0')) (*pos)++;
		n=*pos;
        if (script[n]=='\0') return NULL;
		while ((script[n]!='\n')&&(script[n]!=' ')&&(script[n]!=9)&&(script[n]!='\0')) n++;
		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryStringb,&script[*pos],len);
		str_TemporaryStringb[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryStringb;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_virg(char * script,int *pos)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!=';')&&(script[n]!='\0')) n++;
		len=n-(*pos);
        if ((len>SZ_LIMIT)||(script[n]==0))
        {
            *pos=n;
            return NULL;
        }
		memcpy(str_TemporaryString,&script[*pos],len);
		str_TemporaryString[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_virgkeep(char * script,int *pos)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!=';')&&(script[n]!=0xD)&&(script[n]!=0xA)&&(script[n]!='\0')) n++;
		if (script[n]==';') len=n-(*pos)+1;
		else len=n-(*pos);
        if ((len>SZ_LIMIT)||(script[n]==0))
        {
            *pos=n;
            return NULL;
        }

		memcpy(str_TemporaryString,&script[*pos],len);
		str_TemporaryString[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_virg2(char * script,int *pos)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!=';')&&(script[n]!='\0')) n++;
		len=n-(*pos);
        if ((len>SZ_LIMIT)||(script[n]==0))
        {
            *pos=n;
            return NULL;
        }

		memcpy(str_TemporaryString2,&script[*pos],len);
		str_TemporaryString2[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString2;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_rl(char * script,int *pos)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!='\n')&&(script[n]!='\0')) n++;
		VPLINEINSCRIPT++;
		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryString,&script[*pos],len);
		str_TemporaryString[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_rln2(char * script,int *pos)
{
    int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

    if (script[*pos]=='\0')
    {
        return NULL;
    }
    else
    {
        n=*pos;
        while ((script[n]!='\n')&&(script[n]!='\0')) n++;
        len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
        memcpy(str_TemporaryString,&script[*pos],len);
        str_TemporaryString[len]='\0';
        if (script[n]=='\0') *pos=n;
        else *pos=n+1;

        return str_TemporaryString;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_char_prt(char * script,int *pos,char c)
{
	int n,len;
	int level=0;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while (((script[n]!=c)&&(script[n]!='\0'))||(level>0))
		{
			if (script[n]=='(') level++;
			if (script[n]==')') level--;
			n++;
		}
		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryStringprt,&script[*pos],len);
		str_TemporaryStringprt[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryStringprt;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_char(char * script,int *pos,char c)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!=c)&&(script[n]!='\0')) n++;
		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryString,&script[*pos],len);
		str_TemporaryString[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_char_prths(char * script,int *pos,char c)
{
	int n,len;
	int level=0;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;

		while (!(((script[n]==c)&&(level==1))||(script[n]=='\0')))
		{
			if (script[n]=='(') level++;
			if (script[n]==')') level--;
			n++;
		}
		n++;

		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryString,&script[*pos],len);
		str_TemporaryString[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n;

		return str_TemporaryString;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_parse_char2(char * script,int *pos,char c)
{
	int n,len;
    
    int SZ_LIMIT=(int)strlen(&script[*pos]);
    if (SZ_LIMIT>256) SZ_LIMIT=256;

	if (script[*pos]=='\0')
	{
		return NULL;
	}
	else
	{
		n=*pos;
		while ((script[n]!=c)&&(script[n]!='\0')) n++;
		len=n-(*pos);
        if (len>SZ_LIMIT) return NULL;
		memcpy(str_TemporaryString2,&script[*pos],len);
		str_TemporaryString2[len]='\0';
		if (script[n]=='\0') *pos=n;
		else *pos=n+1;

		return str_TemporaryString2;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float temp_vector[4];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* str_get_vector(char *str)
{
	int p=0;
	char * tmp;
	temp_vector[0]=0.0f;
	temp_vector[1]=0.0f;
	temp_vector[2]=0.0f;
	temp_vector[3]=0.0f;
	tmp=str_parse_char(str,&p,',');
	if (tmp) sscanf(tmp,"%f",&temp_vector[0]);
	tmp=str_parse_char(str,&p,',');
	if (tmp) sscanf(tmp,"%f",&temp_vector[1]);
	tmp=str_parse_char(str,&p,',');
	if (tmp) sscanf(tmp,"%f",&temp_vector[2]);
	tmp=str_parse_char(str,&p,',');
	if (tmp) sscanf(tmp,"%f",&temp_vector[3]);

	return temp_vector;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float temp_mat[16];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float* str_get_matrix(char *str)
{
	int k;
	int p=0;
	char * tmp;

	for (k=0;k<16;k++) temp_mat[k]=0.0f;
	k=0;
	tmp=str_parse_char(str,&p,',');
	while (tmp)
	{
		sscanf(tmp,"%f",&temp_mat[k++]);
		tmp=str_parse_char(str,&p,',');
	}

	return temp_mat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_valid(char * str)
{
	int valid,cvalid;
	int n;
	int len=(int) strlen(str);

	if (len==0) return false;

	valid=0;
	cvalid=0;
	n=0;
	while (n<len)
	{
		if (str[n]=='(') valid++;
		if (str[n]==')') valid--;
		if (str[n]=='[') cvalid++;
		if (str[n]==']') cvalid--;
		n++;
	}
	return ((valid==0)&&(cvalid==0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_cut(char * str,int pos,int len,char * s)
{
	int n;
	if (len>0)
	for (n=0;n<len;n++) s[n]=str[pos+n];
	s[len]='\0';
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_parentheses(char * str)
{
	int n;
	int level;
	n=0;
	if (str[0]!='(') return false;
	else
	{
		n++;
		level=1;
        int len=(int) strlen(str);
		while (n<len)
		{
			if (str[n]=='(') level++;
			if (str[n]==')') level--;
            if ((level==0)&&(n!=len-1)) return false;
			n++;
		}
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char str_sname[65536];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_return_parentheses(char * str)  // pour test conditionnel (gere appel de fonction)
{
	int n,nn,n0;
	int res,level;
	int len;

	len=(int)strlen(str);

	n=0;
	while ((n<len)&&(str[n]!='(')) n++;
	if (n==len) return "";
	n++;

	nn=n;
	res=0;
	level=1;
	while ((nn<len)&&(res==0))
	{
		if (str[nn]=='(') level++;
		if (str[nn]==')') level--;

		if (level==0) res=1;
		else
			nn++;
	}

	if (nn==len) return "";

	for (n0=n;n0<nn;n0++) str_sname[n0-n]=str[n0];
	str_sname[nn-n]='\0';

	return str_sname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_apostrophes(char * str)  
{
	int n,nn,n0;
	int res,level;
	int len;

	len=(int)strlen(str);

	n=0;
	while ((n<len)&&(str[n]!='\'')) n++;
	if (n==len) return NULL;
	n++;

	nn=n;
	res=0;
	level=1;
	while ((nn<len)&&(res==0))
	{
		if (str[nn]=='\'') res=1;
		else
		nn++;
	}
	
    for (n0=n;n0<nn;n0++) str_sname[n0-n]=str[n0];
	str_sname[nn-n]='\0';

	return str_sname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_return_crochets(char * str)
{
	int n,nn,n0;
	int res,level;
	int len;

	len=(int)strlen(str);

	n=0;
	while ((n<len)&&(str[n]!='[')) n++;
	if (n==len) return NULL;
	n++;

	nn=n;
	res=0;
	level=1;
	while ((nn<len)&&(res==0))
	{
		if (str[nn]=='[') level++;
		if (str[nn]==']') level--;

		if (level==0) res=1;
		else
			nn++;
	}

	if (nn==len) return NULL;

    for (n0=n;n0<nn;n0++) str_sname[n0-n]=str[n0];
	str_sname[nn-n]='\0';

	return str_sname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void str_translate_format(char * instr,char * post,int *value)
{
	char temp[512];
	char temp2[512];

	if (str_char(instr,'[')==-1)
	{
		_sprintf(post,"%s[",instr);
		*value=0;
	}
	else
	{
		_sprintf(temp,"%s",str_return_crochets(instr));
		_sprintf(temp2,"%s",instr);
		temp2[str_char(temp2,'[')]='\0';

		if (str_int(temp))
		{
			sscanf(temp,"%d",value);
			_sprintf(post,"%s[",temp2);
		}
		else
		{
			*value=0;
			_sprintf(post,"%s[A0.x+",temp2);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool chiffre(char c)
{
	if ((c<'0')||(c>'9'))
	{
		if (c!='.') return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool numb(char *s)
{
	int n;
    
    if (s==NULL) return false;
    
	int len=(int)strlen(s);
	int f=str_char(s,'f');
	if (f==-1)
	{
		for (n=0;n<len;n++)
		{
			if (!chiffre(s[n]))
			{
				if (s[n]!='-') return false;
			}
		}
	}
	else
	{
		if (f!=len-1) return false;
		if (f==0) return false;
		for (n=0;n<len-1;n++)
		{
			if (!chiffre(s[n]))
			{
				if (s[n]!='-') return false;
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_simple(char *chaine)
{
    if (chaine==NULL) return true;
    
	if (numb(chaine)) return true;

	if (chaine[0]=='-')
	{
		if (numb(&chaine[1])) return true;
		return ((str_char(&chaine[1],'+')==-1)&&(str_char(&chaine[1],'-')==-1)&&(str_char(&chaine[1],'*')==-1)&&(str_char(&chaine[1],'/')==-1)&&(str_char(&chaine[1],'|')==-1)&&(str_char(&chaine[1],'(')==-1)&&(str_char(&chaine[1],'^')==-1)); 
	}
	else
	if ((chaine[0]=='1')&&(chaine[1]=='-'))
	{
		return ((str_char(&chaine[2],'+')==-1)&&(str_char(&chaine[2],'-')==-1)&&(str_char(&chaine[2],'*')==-1)&&(str_char(&chaine[2],'/')==-1)&&(str_char(&chaine[2],'|')==-1)&&(str_char(&chaine[2],'(')==-1)&&(str_char(&chaine[2],'^')==-1));
	}
	else return ((str_char(chaine,'+')==-1)&&(str_char(chaine,'-')==-1)&&(str_char(chaine,'*')==-1)&&(str_char(chaine,'/')==-1)&&(str_char(chaine,'|')==-1)&&(str_char(chaine,'(')==-1)&&(str_char(chaine,'^')==-1));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LENGTH_VAR[4]={ 0,4,1,1};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vpRenderState RS;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char * name;
	int value;
	char *d3ds;
} _TranslateValue;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
_TranslateValue vals[]={

	{"UNDEFINED",_UNDEFINED,""},
	{"MODULATE",_MODULATE,"Modulate"},
	{"BUMPENVMAPPING",_BUMPENVMAP,"BumpEnvMap"},
	{"ADD",_ADD,"Add"},
	{"DOT3",_DOT3,"DotProduct3"},
	{"DIFFUSE",_DIFFUSE,"Diffuse"},
	{"SPECULAR",_SPECULAR,"Specular"},
	{"TEXTURE",_TEXTURE,"Texture"},
	{"CURRENT",_CURRENT,"Current"},
	{"DISABLE",_DISABLE,"Disable"},
	{"TRUE",_TRUE,"True"},
	{"FALSE",_FALSE,"False"},
	{"ZERO",_ZERO,"Zero"},
	{"ONE",_ONE,"One"},
	{"INVSRCALPHA",_INVSRCALPHA,"InvSrcAlpha"},
	{"SRCALPHA",_SRCALPHA,"SrcAlpha"},
	{"INVDSTALPHA",_INVDSTALPHA,"InvDestAlpha"},
	{"DSTALPHA",_DSTALPHA,"DestAlpha"},
	{"INVSRCCOLOR",_INVSRCCOLOR,"InvSrcColor"},
	{"SRCCOLOR",_SRCCOLOR,"SrcColor"},
	{"INVDSTCOLOR",_INVDSTCOLOR,"invDestColor"},
	{"DSTCOLOR",_DSTCOLOR,"DestColor"},
	{"NONE",_NONE,"None"},
	{"CW",_CW,"Cw"},
	{"CCW",_CCW,"Ccw"},
	{"ADDSIGNED",_ADDSIGNED,"AddSigned"},
	{"ADDSIGNED2X",_ADDSIGNED2X,"AddSigned2x"},
	{"MODULATE2X",_MODULATE2X,"Modulate2x"},
	{"SUBTRACT",_SUBTRACT,"Subtract"},
	{"SELECTARG1",_SELECTARG1,"SelectArg1"},
	{"SELECTARG2",_SELECTARG2,"SelectArg2"},

	{"TEXCOO0",_TEXCOO0,"0"},
	{"TEXCOO1",_TEXCOO1,"1"},
	{"TEXCOO2",_TEXCOO2,"2"},
	{"TEXCOO3",_TEXCOO3,"3"},

	{"STENCIL",_STENCIL,""},
	{"TWOSIDED",_TWOSIDED,""},
	{"RENDER",_RENDER,""},
	{"BOTHZ",_BOTHZ,""},
	{"BOTH",_BOTH,""},	
	{"STENCIL_NOZTEST",_STENCIL_NO_ZTEST,""},
	{"ZBUFFER",_ZBUFFER,""},

	{"INCREMENT",_INCREMENT,""},
	{"DECREMENT",_DECREMENT,""},
	{"SET_ONE",_SET_ONE,""},
	{"EQUAL_ZERO",_EQUAL_ZERO,""},
	{"EQUAL_ONE",_EQUAL_ONE,""},
	{"ALWAYS",_ALWAYS,"Always"},
	{"NOT_ONE",_NOT_ONE,""},
	{"NOT_ZERO",_NOT_ZERO,""},
	{"LEQUAL_ONE",_LESSEQUAL_ONE,""},
	{"GEQUAL_ONE",_GREATEREQUAL_ONE,""},

	{"SMOOTH",_SMOOTH,"Gouraud"},
	{"FLAT",_FLAT,"Flat"},

	{"OFFSET_0",_OFFSET_0,"0"},
	{"OFFSET_1",_OFFSET_1,"1"},
	{"OFFSET_2",_OFFSET_2,"2"},
	{"OFFSET_3",_OFFSET_3,"3"},
	{"OFFSET_4",_OFFSET_4,"4"},
	{"OFFSET_5",_OFFSET_5,"5"},
	{"OFFSET_6",_OFFSET_6,"6"},
	{"OFFSET_7",_OFFSET_7,"7"},

	{"NEVER",_NEVER,"Never"},
	{"LESS",_LESS,"Less"},
	{"EQUAL",_EQUAL,"Equal"},
	{"LEQUAL",_LEQUAL,"LessEqual"},
	{"GREATER",_GREATER,"Greater"},
	{"NOTEQUAL",_NOTEQUAL,"NotEqual"},
	{"GEQUAL",_GEQUAL,"GreaterEqual"},

	{"CLAMP",_CLAMP,"Clamp"},
	{"REPEAT",_REPEAT,"Wrap"},
	{"LINEAR",_LINEAR,"Linear"},
	{"POINT",_POINT,"Point"},

	{NULL,0}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char * name;
	int * i_ptr;
	char *d3ds;
	bool shd3;
} _TranslateVar;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
_TranslateVar vars[]={

	{"MAG[0]",&RS.Mag[0],"MagFilter[0]",false},
	{"MAG[1]",&RS.Mag[1],"MagFilter[1]",false},
	{"MAG[2]",&RS.Mag[2],"MagFilter[2]",false},
	{"MAG[3]",&RS.Mag[3],"MagFilter[3]",false},
	{"MAG[4]",&RS.Mag[4],"MagFilter[4]",false},
	{"MAG[5]",&RS.Mag[5],"MagFilter[5]",false},
	{"MAG[6]",&RS.Mag[6],"MagFilter[6]",false},
	{"MAG[7]",&RS.Mag[7],"MagFilter[7]",false},
	{"MAG[8]",&RS.Mag[8],"MagFilter[8]",false},
	{"MAG[9]",&RS.Mag[9],"MagFilter[9]",false},
	{"MAG[10]",&RS.Mag[10],"MagFilter[10]",false},
	{"MAG[11]",&RS.Mag[11],"MagFilter[11]",false},
	{"MAG[12]",&RS.Mag[12],"MagFilter[12]",false},
	{"MAG[13]",&RS.Mag[13],"MagFilter[13]",false},
	{"MAG[14]",&RS.Mag[14],"MagFilter[14]",false},
	{"MAG[15]",&RS.Mag[15],"MagFilter[15]",false},

	{"MIN[0]",&RS.Min[0],"MinFilter[0]",false},
	{"MIN[1]",&RS.Min[1],"MinFilter[1]",false},
	{"MIN[2]",&RS.Min[2],"MinFilter[2]",false},
	{"MIN[3]",&RS.Min[3],"MinFilter[3]",false},
	{"MIN[4]",&RS.Min[4],"MinFilter[4]",false},
	{"MIN[5]",&RS.Min[5],"MinFilter[5]",false},
	{"MIN[6]",&RS.Min[6],"MinFilter[6]",false},
	{"MIN[7]",&RS.Min[7],"MinFilter[7]",false},
	{"MIN[8]",&RS.Min[8],"MinFilter[8]",false},
	{"MIN[9]",&RS.Min[9],"MinFilter[9]",false},
	{"MIN[10]",&RS.Min[10],"MinFilter[10]",false},
	{"MIN[11]",&RS.Min[11],"MinFilter[11]",false},
	{"MIN[12]",&RS.Min[12],"MinFilter[12]",false},
	{"MIN[13]",&RS.Min[13],"MinFilter[13]",false},
	{"MIN[14]",&RS.Min[14],"MinFilter[14]",false},
	{"MIN[15]",&RS.Min[15],"MinFilter[15]",false},

	{"MIP[0]",&RS.Mip[0],"MipFilter[0]",false},
	{"MIP[1]",&RS.Mip[1],"MipFilter[1]",false},
	{"MIP[2]",&RS.Mip[2],"MipFilter[2]",false},
	{"MIP[3]",&RS.Mip[3],"MipFilter[3]",false},
	{"MIP[4]",&RS.Mip[4],"MipFilter[4]",false},
	{"MIP[5]",&RS.Mip[5],"MipFilter[5]",false},
	{"MIP[6]",&RS.Mip[6],"MipFilter[6]",false},
	{"MIP[7]",&RS.Mip[7],"MipFilter[7]",false},
	{"MIP[8]",&RS.Mip[8],"MipFilter[8]",false},
	{"MIP[9]",&RS.Mip[9],"MipFilter[9]",false},
	{"MIP[10]",&RS.Mip[10],"MipFilter[10]",false},
	{"MIP[11]",&RS.Mip[11],"MipFilter[11]",false},
	{"MIP[12]",&RS.Mip[12],"MipFilter[12]",false},
	{"MIP[13]",&RS.Mip[13],"MipFilter[13]",false},
	{"MIP[14]",&RS.Mip[14],"MipFilter[14]",false},
	{"MIP[15]",&RS.Mip[15],"MipFilter[15]",false},

	{"ADDRESSU[0]",&RS.AddressU[0],"AddressU[0]",true},
	{"ADDRESSV[0]",&RS.AddressV[0],"AddressV[0]",true},

	{"ADDRESSU[1]",&RS.AddressU[1],"AddressU[1]",true},
	{"ADDRESSV[1]",&RS.AddressV[1],"AddressV[1]",true},

	{"ADDRESSU[2]",&RS.AddressU[2],"AddressU[2]",true},
	{"ADDRESSV[2]",&RS.AddressV[2],"AddressV[2]",true},

	{"ADDRESSU[3]",&RS.AddressU[3],"AddressU[3]",true},
	{"ADDRESSV[3]",&RS.AddressV[3],"AddressV[3]",true},

	{"ADDRESSU[4]",&RS.AddressU[4],"AddressU[4]",true},
	{"ADDRESSV[4]",&RS.AddressV[4],"AddressV[4]",true},

	{"ADDRESSU[5]",&RS.AddressU[5],"AddressU[5]",true},
	{"ADDRESSV[5]",&RS.AddressV[5],"AddressV[5]",true},

	{"ADDRESSU[6]",&RS.AddressU[6],"AddressU[6]",true},
	{"ADDRESSV[6]",&RS.AddressV[6],"AddressV[6]",true},

	{"ADDRESSU[7]",&RS.AddressU[7],"AddressU[7]",true},
	{"ADDRESSV[7]",&RS.AddressV[7],"AddressV[7]",true},

	{"ADDRESSU[8]",&RS.AddressU[8],"AddressU[8]",true},
	{"ADDRESSV[8]",&RS.AddressV[8],"AddressV[8]",true},

	{"ADDRESSU[9]",&RS.AddressU[9],"AddressU[9]",true},
	{"ADDRESSV[9]",&RS.AddressV[9],"AddressV[9]",true},

	{"ADDRESSU[10]",&RS.AddressU[10],"AddressU[10]",true},
	{"ADDRESSV[10]",&RS.AddressV[10],"AddressV[10]",true},

	{"ADDRESSU[11]",&RS.AddressU[11],"AddressU[11]",true},
	{"ADDRESSV[11]",&RS.AddressV[11],"AddressV[11]",true},

	{"ADDRESSU[12]",&RS.AddressU[12],"AddressU[12]",true},
	{"ADDRESSV[12]",&RS.AddressV[12],"AddressV[12]",true},

	{"ADDRESSU[13]",&RS.AddressU[13],"AddressU[13]",true},
	{"ADDRESSV[13]",&RS.AddressV[13],"AddressV[13]",true},

	{"ADDRESSU[14]",&RS.AddressU[14],"AddressU[14]",true},
	{"ADDRESSV[14]",&RS.AddressV[14],"AddressV[14]",true},

	{"ADDRESSU[15]",&RS.AddressU[15],"AddressU[15]",true},
	{"ADDRESSV[15]",&RS.AddressV[15],"AddressV[15]",true},

	{"COLOROP[0]",&RS.COp[0],"ColorOp[0]",true},
	{"COLORARG1[0]",&RS.CArg1[0],"ColorArg1[0]",true},
	{"COLORARG2[0]",&RS.CArg2[0],"ColorArg2[0]",true},
	{"ALPHAOP[0]",&RS.AOp[0],"AlphaOp[0]",true},
	{"ALPHAARG1[0]",&RS.AArg1[0],"AlphaArg1[0]",true},
	{"ALPHAARG2[0]",&RS.AArg2[0],"AlphaArg2[0]",true},

	{"COLOROP[1]",&RS.COp[1],"ColorOp[1]",true},
	{"COLORARG1[1]",&RS.CArg1[1],"ColorArg1[1]",true},
	{"COLORARG2[1]",&RS.CArg2[1],"ColorArg2[1]",true},
	{"ALPHAOP[1]",&RS.AOp[1],"AlphaOp[1]",true},
	{"ALPHAARG1[1]",&RS.AArg1[1],"AlphaArg1[1]",true},
	{"ALPHAARG2[1]",&RS.AArg2[1],"AlphaArg2[1]",true},

	{"COLOROP[2]",&RS.COp[2],"ColorOp[2]",true},
	{"COLORARG1[2]",&RS.CArg1[2],"ColorArg1[2]",true},
	{"COLORARG2[2]",&RS.CArg2[2],"ColorArg2[2]",true},
	{"ALPHAOP[2]",&RS.AOp[2],"AlphaOp[2]",true},
	{"ALPHAARG1[2]",&RS.AArg1[2],"AlphaArg1[2]",true},
	{"ALPHAARG2[2]",&RS.AArg2[2],"AlphaArg2[2]",true},

	{"COLOROP[3]",&RS.COp[3],"ColorOp[3]",true},
	{"COLORARG1[3]",&RS.CArg1[3],"ColorArg1[3]",true},
	{"COLORARG2[3]",&RS.CArg2[3],"ColorArg2[3]",true},
	{"ALPHAOP[3]",&RS.AOp[3],"AlphaOp[3]",true},
	{"ALPHAARG1[3]",&RS.AArg1[3],"AlphaArg1[3]",true},
	{"ALPHAARG2[3]",&RS.AArg2[3],"AlphaArg2[3]",true},

	{"TEXTURECOORDINATE[0]",&RS.TextCoord[0],"TexCoordIndex[0]",false},
	{"TEXTURECOORDINATE[1]",&RS.TextCoord[1],"TexCoordIndex[1]",false},
	{"TEXTURECOORDINATE[2]",&RS.TextCoord[2],"TexCoordIndex[2]",false},
	{"TEXTURECOORDINATE[3]",&RS.TextCoord[3],"TexCoordIndex[3]",false},

	{"BLEND",&RS.Blend,"AlphaBlendEnable",true},
	{"SRCBLEND",&RS.SrcBlend,"SrcBlend",true},
	{"DSTBLEND",&RS.DstBlend,"DestBlend",true},
	
	{"RENDERTARGET",&RS.RenderTarget,NULL,true},
	{"STENCIL",&RS.Stencil,NULL,true},
	
	{"SHADEMODE",&RS.ShadeMode,"ShadeMode",true},
	{"CULL",&RS.Cull,"CullMode",true},

	{"ZBUFFER",&RS.ZBuffer,"ZEnable",true},
	{"ZBUFFERWRITE",&RS.ZBufferWrite,"ZWriteEnable",true},
	{"ZBIAS",&RS.zbias,"Zbias",true},
	{"ZFUNC",&RS.Zfunc,"ZFunc",true},

	{"ALPHATEST",&RS.AlphaTest,"AlphaTestEnable",true},
	{"ALPHAREF",&RS.AlphaRef,"AlphaRef",true},
	{"ALPHAFUNC",&RS.AlphaFunc,"AlphaFunc",true},
	{"DITHER",&RS.Dither,"DitherEnable",true},

	{NULL,0},
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int* str_ptr_to_var(char *str)
{
	int n=0;
	while (vars[n].name)
	{
		if (strcmp(vars[n].name,str)==0) return vars[n].i_ptr;
		n++;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* str_d3d_translate_op(char *str)
{
	int n=0;
	bool tag=false;

	while (vars[n].name)
	{
		if (strcmp(vars[n].name,str)==0) return vars[n].d3ds;
		n++;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* str_d3d_translate_op3(char *str)
{
	int n=0;
	char *res=NULL;
	bool tag=false;

	while ((vars[n].name)&&(!res))
	{
		if (strcmp(vars[n].name,str)==0) { res=vars[n].d3ds; tag=vars[n].shd3; }
		n++;
	}

	if (!tag) return NULL;

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* str_d3d_translate_value(char *str)
{
	int n=0;
	while (vals[n].name)
	{
		if (strcmp(vals[n].name,str)==0) return vals[n].d3ds;
		n++;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char hexa_char(int h)
{
	char hexa[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	return hexa[h];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int char_hexa(char c)
{
	int res=0;

	switch (c)
	{
	case '0':
		res=0;
		break;
	case '1':
		res=1;
		break;
	case '2':
		res=2;
		break;
	case '3':
		res=3;
		break;
	case '4':
		res=4;
		break;
	case '5':
		res=5;
		break;
	case '6':
		res=6;
		break;
	case '7':
		res=7;
		break;
	case '8':
		res=8;
		break;
	case '9':
		res=9;
		break;
	case 'A':
		res=10;
		break;
	case 'B':
		res=11;
		break;
	case 'C':
		res=12;
		break;
	case 'D':
		res=13;
		break;
	case 'E':
		res=14;
		break;
	case 'F':
		res=15;
		break;
	};

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_int_value(char *str)
{
	int n=0;
	int a,b;
	int res=-1;

	if ((str[0]=='V')&&(str[1]=='A')&&(str[2]=='R')) res=-666;
	else
	{
		while ((vals[n].name)&&(res==-1))
		{
			if (strcmp(vals[n].name,str)==0) res=vals[n].value;
			n++;
		}

		if (res==-1)
		{
			if ((str[0]=='0')&&(str[1]=='X'))
			{
				a=char_hexa(str[2]);
				b=char_hexa(str[3]);
				res=a*16+b;
			}
			else
			if (numb(str))
			{
				sscanf(str,"%d",&res);
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* str_get_ptr_to_var_name(int * ptr)
{
	int n=0;

	while (vars[n].name)
	{
		if (ptr==vars[n].i_ptr) return vars[n].name;
		n++;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char shexa[5];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* str_get_int_value_name(int i)
{
	int n=0;

	if (i<256)
	{
		shexa[0]='0';
		shexa[1]='x';
		shexa[2]=hexa_char((i/16)&0xF);
		shexa[3]=hexa_char(i&0xF);
		shexa[4]='\0';
		return shexa;
	}
	else
	{
		while (vals[n].name)
		{
			if (vals[n].value==i) return vals[n].name;
			n++;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char shexa_tmp[9];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *str_convert_dec_to_hex(unsigned int dec)
{
	unsigned int n=0;
	unsigned int val=0;

	shexa_tmp[0]='0';
	shexa_tmp[1]='x';
	for (n=0;n<2;n++)
	{
		val=(dec>>(n*4))&0xF;
		shexa_tmp[3-n]=hexa_char(val);
	}
	shexa_tmp[4]='\0';
	return shexa_tmp;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RS_Reset(vpRenderState *RS)
{
	int *i;
	int l;
	int n;

	l=0;
	i=(int*) RS;

	while (l<(int) sizeof(vpRenderState))
	{
		(*i)=_UNDEFINED;
		l+=4;
		i++;
	}

	for (n=0;n<16;n++) RS->Texture[n]=NULL;
    for (n=0;n<16;n++) RS->TextureVS[n]=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::isValueCoef()
{
    int n;

    for (n=0;n<last_constant;n++)
    {
        if (constants[n])
        {
            if (translate_constants[n]==NULL)
            {
                if (strcmp(constants[n],"Range")==0) return false;
            }
        }
    }
    
    for (n=0;n<npsvectors;n++)
    {
        if (strcmp(psvectors[n],"Range")==0) return false;
    }

    
    for (n=0;n<last_constant;n++)
    {
        if (constants[n])
        {
            if (translate_constants[n]==NULL)
            {
                if (strcmp(constants[n],"value_coef")==0) return true;
            }
        }
    }
    
    for (n=0;n<npsvectors;n++)
    {
        if (strcmp(psvectors[n],"value_coef")==0) return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::cleanbuffs()
{
	vp.Free();
	vpoutput.Free();
	vpinput.Free();
	vpcst.Free();
	vptmp.Free();

	for (int n=0;n<8;n++)
	{
		s3vpmetal[n].Free();
		s3psvpmetal[n].Free();

		glvar[n].Free();
		glvp[n].Free();
		glps[n].Free();
		gltemp[n].Free();

		s3vp[n].Free();
		s3fnvp[n].Free();
		s3psvp[n].Free();
		s3psfnvp[n].Free();
		s3rs[n].Free();
	}

	glheader.Free();
	glheaderps.Free();
	glfn.Free();
	glfnps.Free();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::reset()
{
    reset(-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::reset(int ext)
{
	int n,p;

	for (p=0;p<8;p++)
	for (n=0;n<6;n++) trigocst[p][n]=0;

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

	globalvar=false;
	version300=0;
    
    pointcloud=0;

	FSRSample0=0;
	FSRSample1=0;

	SizingTexture0=0;
    
    FSR=0;
    isFSR=false;
    
    warp_normal_pos=ext;

	nmatrices=0;
	nfloats=0;
    
    NPASS=0;

	NoDepthOut=false;

	TYPEERROR=false;
	SYNTAXERROR=false;
    RSERROR=false;
	SYNTAXERRORFN=false;
	DEFINEERROR=false;
	UNUSEDERROR=false;
	PARAMERROR=false;
	REPERROR=false;
    IFERROR=false;

    ERRORSTR[0]=0;
    RSERRORSTR[0]=0;

	PRMERRORSTR[0]=0;
	ERRORSTRPARAM[0]=0;
	UNUSEDRRORSTR[0]=0;

	label=numvsfn=numpsfn=0;

	for (n=0;n<24;n++) vsfn[n].name[0]=0;
	for (n=0;n<24;n++) psfn[n].name[0]=0;

	for (p=0;p<8;p++)
	{
		for (n=0;n<24;n++) vsfncodeline[n][p]=0;
		for (n=0;n<24;n++) psfncodeline[n][p]=0;
		vscodeline[p]=0;
		pscodeline[p]=0;
		numbervsfn[p]=0;
		numberpsfn[p]=0;
	}

	clear_registers();
	Vars.Free();

	vp.Free();
	vpoutput.Free();
	vpinput.Free();
	vpcst.Free();
	vptmp.Free();

	for (n=0;n<8;n++)
	{
		s3vpmetal[n].Free();
		s3psvpmetal[n].Free();

		glvar[n].Free();
		glvp[n].Free();
		glps[n].Free();
		gltemp[n].Free();

		s3vp[n].Free();
		s3fnvp[n].Free();
		s3psvp[n].Free();
		s3psfnvp[n].Free();
		s3rs[n].Free();
	}

	glheader.Free();
	glheaderps.Free();
	glfn.Free();
	glfnps.Free();

	last_constant=0;
	last_iregister=0;
	last_oregister=0;
	last_o2register=0;
	last_register=0;
	last_constant_register=0;
	last_ps_constant_register=0;

	RS_Reset(&RS);
	for (n=0;n<4;n++)
	{
		RS_Reset(&RenderState[n][0]);
		RS_Reset(&RenderState[n][1]);
		RS_Reset(&RenderState[n][2]);
		RS_Reset(&RenderState[n][3]);
	}

	npsvectors=0;	
	for (n=0;n<MAX_ENTRIES;n++) psvectors[n]=NULL;
	npsmatrices=0;	
	for (n=0;n<MAX_ENTRIES;n++) psmatrices[n]=NULL;

	for (n=0;n<32;n++) if (registers[n]) free(registers[n]);
	for (n=0;n<32;n++) if (iregisters[n][0]) free(iregisters[n][0]);
	for (n=0;n<32;n++) if (oregisters[n][0]) free(oregisters[n][0]);
	for (n=0;n<32;n++) if (o2registers[n][0]) free(o2registers[n][0]);
	for (n=0;n<32;n++) if (iregisters[n][1]) free(iregisters[n][1]);
	for (n=0;n<32;n++) if (oregisters[n][1]) free(oregisters[n][1]);
	for (n=0;n<32;n++) if (o2registers[n][1]) free(o2registers[n][1]);

	for (n=0;n<MAX_ENTRIES;n++) if (constants[n]) free(constants[n]);
	for (n=0;n<MAX_ENTRIES;n++) l_constants[n]=0;
	for (n=0;n<MAX_ENTRIES;n++) if (translate_constants[n]) free(translate_constants[n]);

	for (n=0;n<MAX_ENTRIES;n++)  ps_constants[n]=NULL;
	for (n=0;n<MAX_ENTRIES;n++)  if (translate_ps_constants[n]) free(translate_ps_constants[n]);
	for (n=0;n<MAX_ENTRIES;n++)  translate_ps_constants[n]=NULL;

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

    for (n=0;n<16;n++)
    {
        if (texture[n]) free(texture[n]);
        texture[n]=NULL;
        if (texturevs[n]) free(texturevs[n]);
        texturevs[n]=NULL;
    }
    ntexture=ntexturevs=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::compile_render_state_pixelshader(int tech,int pass,CList <Code> *vp)
{
    // here it is
    
#ifdef API3D_METAL
    vpRenderState _RS;
    bool res=false;
    // SHADER MODEL 4 only

    memcpy(&_RS,&RenderState[tech][pass],sizeof(vpRenderState));

    if (_RS.COp[0]!=_UNDEFINED)
    {
        res=true;
        if (_RS.CArg1[0]==_TEXTURE) { _sprintf(c.str, "r1 = float4(%s.sample(texsampler0, pixel.t0.xy))", _RS.Texture[0]); AddInst(vp,c); }
        if (_RS.CArg2[0]==_TEXTURE) { _sprintf(c.str, "r2 = float4(%s.sample(texsampler0, pixel.t0.xy))", _RS.Texture[0]); AddInst(vp,c); }
        
        if (_RS.CArg1[0]==_DIFFUSE) { _sprintf(c.str, "r1 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg2[0]==_DIFFUSE) { _sprintf(c.str, "r2 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg1[0]==_SPECULAR) { _sprintf(c.str, "r1 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg2[0]==_SPECULAR) { _sprintf(c.str, "r2 = pixel.v1"); AddInst(vp,c); }

        switch (_RS.COp[0])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _MODULATE2X:
            _sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _DOT3:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED2X:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
            break;
        };

        switch (_RS.AOp[0])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
            break;
        case _DISABLE:
            _sprintf(c.str, "r0.w = 1.0"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
            break;
        };
    }

    if ((_RS.COp[1]!=_UNDEFINED)&&(res))
    {
        if (_RS.CArg1[1]==_TEXTURE) { _sprintf(c.str, "r1 = float4(%s.sample(texsampler0, pixel.t1.xy))", _RS.Texture[1]); AddInst(vp,c); }
        if (_RS.CArg2[1]==_TEXTURE) { _sprintf(c.str, "r2 = float4(%s.sample(texsampler0, pixel.t1.xy))", _RS.Texture[1]); AddInst(vp,c); }

        if (_RS.CArg1[1]==_DIFFUSE) { _sprintf(c.str, "r1 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg2[1]==_DIFFUSE) { _sprintf(c.str, "r2 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg1[1]==_SPECULAR) { _sprintf(c.str, "r1 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg2[1]==_SPECULAR) { _sprintf(c.str, "r2 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg1[1]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
        if (_RS.CArg2[1]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

        switch (_RS.COp[1])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _MODULATE2X:
            _sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _DOT3:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED2X:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
            break;
        };

        switch (_RS.AOp[1])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
            break;
        };
    }

    if ((_RS.COp[2]!=_UNDEFINED)&&(res))
    {
        if (_RS.CArg1[2]==_TEXTURE) { _sprintf(c.str, "r1 = float4(%s.sample(texsampler0, pixel.t2.xy))", _RS.Texture[2]); AddInst(vp,c); }
        if (_RS.CArg2[2]==_TEXTURE) { _sprintf(c.str, "r2 = float4(%s.sample(texsampler0, pixel.t2.xy))", _RS.Texture[2]); AddInst(vp,c); }

        if (_RS.CArg1[2]==_DIFFUSE) { _sprintf(c.str, "r1 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg2[2]==_DIFFUSE) { _sprintf(c.str, "r2 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg1[2]==_SPECULAR) { _sprintf(c.str, "r1 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg2[2]==_SPECULAR) { _sprintf(c.str, "r2 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg1[2]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
        if (_RS.CArg2[2]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

        switch (_RS.COp[2])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _MODULATE2X:
            _sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _DOT3:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED2X:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
            break;
        };

        switch (_RS.AOp[2])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
            break;
        };
    }

    if ((_RS.COp[3]!=_UNDEFINED)&&(res))
    {
        if (_RS.CArg1[3]==_TEXTURE) { _sprintf(c.str, "r1 = float4(%s.sample(texsampler0, pixel.t3.xy))", _RS.Texture[3]); AddInst(vp,c); }
        if (_RS.CArg2[3]==_TEXTURE) { _sprintf(c.str, "r2 = float4(%s.sample(texsampler0, pixel.t3.xy))", _RS.Texture[3]); AddInst(vp,c); }

        if (_RS.CArg1[3]==_DIFFUSE) { _sprintf(c.str, "r1 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg2[3]==_DIFFUSE) { _sprintf(c.str, "r2 = pixel.v0"); AddInst(vp,c); }
        if (_RS.CArg1[3]==_SPECULAR) { _sprintf(c.str, "r1 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg2[3]==_SPECULAR) { _sprintf(c.str, "r2 = pixel.v1"); AddInst(vp,c); }
        if (_RS.CArg1[3]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
        if (_RS.CArg2[3]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

        switch (_RS.COp[3])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _MODULATE2X:
            _sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
            break;
        case _DOT3:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
            break;
        case _ADDSIGNED2X:
            _sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
            _sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
            break;
        };

        switch (RS.AOp[3])
        {
        case _MODULATE:
            _sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
            break;
        case _ADD:
            _sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
            break;
        case _SUBTRACT:
            _sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
            break;
        case _SELECTARG1:
            _sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
            break;
        case _SELECTARG2:
            _sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
            break;
        };
    }

    if (!res)
    {
        res=true;
        _sprintf(c.str, "r0 = float4(1.0f,1.0f,1.0f,1.0f)"); AddInst(vp,c);
    }

    return res;

#endif
    
#if defined(API3D_DIRECT3D10)||defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)
	vpRenderState _RS;
	bool res=false;
	// SHADER MODEL 4 only

	memcpy(&_RS,&RenderState[tech][pass],sizeof(vpRenderState));

	if (_RS.COp[0]!=_UNDEFINED)
	{
		res=true;

        if (_RS.CArg1[0]==_TEXTURE) { _sprintf(c.str, "r1 = %s.Sample(smp, i.t0.xy)", _RS.Texture[0]); AddInst(vp,c); }
        if (_RS.CArg2[0]==_TEXTURE) { _sprintf(c.str, "r2 = %s.Sample(smp, i.t0.xy)", _RS.Texture[0]); AddInst(vp,c); }
		if (_RS.CArg1[0]==_DIFFUSE) { _sprintf(c.str, "r1 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_DIFFUSE) { _sprintf(c.str, "r2 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg1[0]==_SPECULAR) { _sprintf(c.str, "r1 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_SPECULAR) { _sprintf(c.str, "r2 = i.v1"); AddInst(vp,c); }

		switch (_RS.COp[0])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[0])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _DISABLE:
			_sprintf(c.str, "r0.w = 1.0"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[1]!=_UNDEFINED)&&(res))
	{
		if (_RS.CArg1[1]==_TEXTURE) { _sprintf(c.str, "r1 = %s.Sample(smp, i.t1.xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg2[1]==_TEXTURE) { _sprintf(c.str, "r2 = %s.Sample(smp, i.t1.xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg1[1]==_DIFFUSE) { _sprintf(c.str, "r1 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_DIFFUSE) { _sprintf(c.str, "r2 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_SPECULAR) { _sprintf(c.str, "r1 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_SPECULAR) { _sprintf(c.str, "r2 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

		switch (_RS.COp[1])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[1])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[2]!=_UNDEFINED)&&(res))
	{
		if (_RS.CArg1[2]==_TEXTURE) { _sprintf(c.str, "r1 = %s.Sample(smp, i.t2.xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg2[2]==_TEXTURE) { _sprintf(c.str, "r2 = %s.Sample(smp, i.t2.xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg1[2]==_DIFFUSE) { _sprintf(c.str, "r1 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_DIFFUSE) { _sprintf(c.str, "r2 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_SPECULAR) { _sprintf(c.str, "r1 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_SPECULAR) { _sprintf(c.str, "r2 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

		switch (_RS.COp[2])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[2])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[3]!=_UNDEFINED)&&(res))
	{
		if (_RS.CArg1[3]==_TEXTURE) { _sprintf(c.str, "r1 = %s.Sample(smp, i.t3.xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg2[3]==_TEXTURE) { _sprintf(c.str, "r2 = %s.Sample(smp, i.t3.xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg1[3]==_DIFFUSE) { _sprintf(c.str, "r1 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_DIFFUSE) { _sprintf(c.str, "r2 = i.v0"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_SPECULAR) { _sprintf(c.str, "r1 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_SPECULAR) { _sprintf(c.str, "r2 = i.v1"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }

		switch (_RS.COp[3])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - float4(0.5f,0.5f,0.5f,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (RS.AOp[3])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if (!res)
	{
		res=true;
		_sprintf(c.str, "r0 = float4(1.0f,1.0f,1.0f,1.0f)"); AddInst(vp,c);
	}

	return res;
#endif

#if defined(API3D_OPENGL20)||defined(API3D_OPENGL)
	vpRenderState _RS;
	bool res=false;
	// OPENGL "2.0" or OPENGLES 2.0

	memcpy(&_RS,&RenderState[tech][pass],sizeof(vpRenderState));

	if (_RS.COp[0]!=_UNDEFINED)
	{
		res=true;

#ifdef GLES20
		if (_RS.CArg1[0]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, myTexCoord[0].xy)", _RS.Texture[0]); AddInst(vp,c); }
		if (_RS.CArg2[0]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, myTexCoord[0].xy)", _RS.Texture[0]); AddInst(vp,c); }
		if (_RS.CArg1[0]==_DIFFUSE) { _sprintf(c.str, "r1 = myColor"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_DIFFUSE) { _sprintf(c.str, "r2 = myColor"); AddInst(vp,c); }
		if (_RS.CArg1[0]==_SPECULAR) { _sprintf(c.str, "r1 = mySecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_SPECULAR) { _sprintf(c.str, "r2 = mySecondaryColor"); AddInst(vp,c); }
#else
		if (_RS.CArg1[0]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, gl_TexCoord[0].xy)", _RS.Texture[0]); AddInst(vp,c); }
		if (_RS.CArg2[0]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, gl_TexCoord[0].xy)", _RS.Texture[0]); AddInst(vp,c); }
		if (_RS.CArg1[0]==_DIFFUSE) { _sprintf(c.str, "r1 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_DIFFUSE) { _sprintf(c.str, "r2 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg1[0]==_SPECULAR) { _sprintf(c.str, "r1 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[0]==_SPECULAR) { _sprintf(c.str, "r2 = gl_SecondaryColor"); AddInst(vp,c); }
#endif
		switch (_RS.COp[0])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[0])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _DISABLE:
			_sprintf(c.str, "r0.w = 1.0"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[1]!=_UNDEFINED)&&(res))
	{
#ifdef GLES20
		if (_RS.CArg1[1]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, myTexCoord[1].xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg2[1]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, myTexCoord[1].xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg1[1]==_DIFFUSE) { _sprintf(c.str, "r1 = myColor"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_DIFFUSE) { _sprintf(c.str, "r2 = myColor"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_SPECULAR) { _sprintf(c.str, "r1 = mySecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_SPECULAR) { _sprintf(c.str, "r2 = mySecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#else
		if (_RS.CArg1[1]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, gl_TexCoord[1].xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg2[1]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, gl_TexCoord[1].xy)", _RS.Texture[1]); AddInst(vp,c); }
		if (_RS.CArg1[1]==_DIFFUSE) { _sprintf(c.str, "r1 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_DIFFUSE) { _sprintf(c.str, "r2 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_SPECULAR) { _sprintf(c.str, "r1 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_SPECULAR) { _sprintf(c.str, "r2 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[1]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[1]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#endif
		switch (_RS.COp[1])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[1])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[2]!=_UNDEFINED)&&(res))
	{
#ifdef GLES20
		if (_RS.CArg1[2]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, myTexCoord[2].xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg2[2]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, myTexCoord[2].xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg1[2]==_DIFFUSE) { _sprintf(c.str, "r1 = myColor"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_DIFFUSE) { _sprintf(c.str, "r2 = myColor"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_SPECULAR) { _sprintf(c.str, "r1 = mySecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_SPECULAR) { _sprintf(c.str, "r2 = mySecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#else
		if (_RS.CArg1[2]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, gl_TexCoord[2].xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg2[2]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, gl_TexCoord[2].xy)", _RS.Texture[2]); AddInst(vp,c); }
		if (_RS.CArg1[2]==_DIFFUSE) { _sprintf(c.str, "r1 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_DIFFUSE) { _sprintf(c.str, "r2 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_SPECULAR) { _sprintf(c.str, "r1 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_SPECULAR) { _sprintf(c.str, "r2 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[2]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[2]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#endif
		switch (_RS.COp[2])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (_RS.AOp[2])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if ((_RS.COp[3]!=_UNDEFINED)&&(res))
	{
#ifdef GLES20
		if (_RS.CArg1[3]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, gl_TexCoord[3].xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg2[3]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, gl_TexCoord[3].xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg1[3]==_DIFFUSE) { _sprintf(c.str, "r1 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_DIFFUSE) { _sprintf(c.str, "r2 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_SPECULAR) { _sprintf(c.str, "r1 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_SPECULAR) { _sprintf(c.str, "r2 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#else
		if (_RS.CArg1[3]==_TEXTURE) { _sprintf(c.str, "r1 = texture2D(%s, gl_TexCoord[3].xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg2[3]==_TEXTURE) { _sprintf(c.str, "r2 = texture2D(%s, gl_TexCoord[3].xy)", _RS.Texture[3]); AddInst(vp,c); }
		if (_RS.CArg1[3]==_DIFFUSE) { _sprintf(c.str, "r1 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_DIFFUSE) { _sprintf(c.str, "r2 = gl_Color"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_SPECULAR) { _sprintf(c.str, "r1 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_SPECULAR) { _sprintf(c.str, "r2 = gl_SecondaryColor"); AddInst(vp,c); }
		if (_RS.CArg1[3]==_CURRENT) { _sprintf(c.str, "r1 = r0"); AddInst(vp,c); }
		if (_RS.CArg2[3]==_CURRENT) { _sprintf(c.str, "r2 = r0"); AddInst(vp,c); }
#endif

		switch (_RS.COp[3])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.xyz = r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _MODULATE2X:
			_sprintf(c.str, "r0.xyz = 2.0 * r1.xyz * r2.xyz"); AddInst(vp,c);
			break;
		case _DOT3:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.x = 2.0 * dot(r3.xyz,r4.xyz)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r0.xxx"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.xyz = r1.xyz + r2.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = r3.xyz + r4.xyz"); AddInst(vp,c);
			break;
		case _ADDSIGNED2X:
			_sprintf(c.str, "r3 = r1 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r4 = r2 - vec4(0.5,0.5,0.5,0.0)"); AddInst(vp,c);
			_sprintf(c.str, "r0.xyz = 2.0 * (r3.xyz + r4.xyz)"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.xyz = r1.xyz - r2.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.xyz = r1.xyz"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.xyz = r2.xyz"); AddInst(vp,c);
			break;
		};

		switch (RS.AOp[3])
		{
		case _MODULATE:
			_sprintf(c.str, "r0.w = r1.w * r2.w"); AddInst(vp,c);
			break;
		case _ADD:
			_sprintf(c.str, "r0.w = r1.w + r2.w"); AddInst(vp,c);
			break;
		case _SUBTRACT:
			_sprintf(c.str, "r0.w = r1.w - r2.w"); AddInst(vp,c);
			break;
		case _SELECTARG1:
			_sprintf(c.str, "r0.w = r1.w"); AddInst(vp,c);
			break;
		case _SELECTARG2:
			_sprintf(c.str, "r0.w = r2.w"); AddInst(vp,c);
			break;
		};
	}

	if (!res)
	{
		res=true;
		_sprintf(c.str, "r0 = vec4(1.0,1.0,1.0,1.0)"); AddInst(vp,c);
	}

	return res;
#endif

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::compile_render_state(int tech,int pass,CList <Code> *vp)
{
	char *op,*value;
	int *i;
	int l;
	int n;

	memcpy(&RS,&RenderState[tech][pass],sizeof(vpRenderState));

	if (shadermodel4) return;

	if (!shadermodel3)
	for (n=0;n<16;n++)
	{
		if (RS.Texture[n])
		{
			_sprintf(c.str,"Texture[%d] = <%s>;",n,RS.Texture[n]);
			vp->Add(c);
		}
	}

	l=0;
	i=(int*) &RS;

	while (l<(int)(sizeof(vpRenderState)-(16+16+2)*sizeof(int)))
	{
		if ((*i)!=_UNDEFINED)
		{
			if (!shadermodel3) op=str_d3d_translate_op(str_get_ptr_to_var_name(i));
			else op=str_d3d_translate_op3(str_get_ptr_to_var_name(i));
			
			if (*i<=255) value=str_convert_dec_to_hex(*i);
			else value=str_d3d_translate_value(str_get_int_value_name(*i));

			if (op)
			{
				_sprintf(c.str,"%s = %s;",op,value);
				vp->Add(c);
			}
		}
		l+=4;
		i++;
	}

	if (RS.Stencil!=_UNDEFINED)
	{
		switch (RS.Stencil)
		{
		case _INCREMENT:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilWriteMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Incr;");
			vp->Add(c);

			break;
		case _DECREMENT:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilWriteMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Decr;");
			vp->Add(c);

			break;
		case _SET_ONE:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilWriteMask = 0xffffffff;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _EQUAL_ZERO:
			_sprintf(c.str,"StencilRef = 0x0;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = Equal;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _EQUAL_ONE:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = Equal;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _ALWAYS:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = Always;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _NOT_ONE:
			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = NotEqual;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _NOT_ZERO:
			_sprintf(c.str,"StencilRef = 0x0;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = NotEqual;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _LESSEQUAL_ONE:

			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = LessEqual;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		case _GREATEREQUAL_ONE:

			_sprintf(c.str,"StencilRef = 0x1;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = GreaterEqual;");
			vp->Add(c);
			_sprintf(c.str,"StencilPass = Replace;");
			vp->Add(c);

			break;
		};

	}

	if (RS.RenderTarget!=_UNDEFINED)
	{
		switch(RS.RenderTarget)
		{
		case _STENCIL:
			_sprintf(c.str,"AlphaBlendEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"SrcBlend = Zero;");
			vp->Add(c);
			_sprintf(c.str,"DestBlend = One;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = Always;");
			vp->Add(c);
			_sprintf(c.str,"StencilZFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"StencilFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"StencilEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Flat;");
			vp->Add(c);
			break;

		case _TWOSIDED:
			_sprintf(c.str,"AlphaBlendEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"SrcBlend = Zero;");
			vp->Add(c);
			_sprintf(c.str,"DestBlend = One;");
			vp->Add(c);
			_sprintf(c.str,"StencilEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Flat;");
			vp->Add(c);

			_sprintf(c.str,"StencilRef = 0x1;"); vp->Add(c);
			_sprintf(c.str,"StencilMask = 0xffffffff;"); vp->Add(c);
			_sprintf(c.str,"StencilWriteMask = 0xffffffff;"); vp->Add(c);
			
			_sprintf(c.str,"TwoSidedStencilMode = True;"); vp->Add(c);
			
			_sprintf(c.str,"StencilPass = Incr;"); vp->Add(c);
			_sprintf(c.str,"StencilZFail = Keep;"); vp->Add(c);
			_sprintf(c.str,"StencilFunc = Always;"); vp->Add(c);
			_sprintf(c.str,"StencilFail = Keep;"); vp->Add(c);

			_sprintf(c.str,"Ccw_StencilPass = Decr;"); vp->Add(c);
			_sprintf(c.str,"Ccw_StencilZFail = Keep;"); vp->Add(c);
			_sprintf(c.str,"Ccw_StencilFunc = Always;"); vp->Add(c);
			_sprintf(c.str,"Ccw_StencilFail = Keep;"); vp->Add(c);

			_sprintf(c.str,"CullMode = None;\n"); vp->Add(c);
			break;

		case _RENDER:
			_sprintf(c.str,"StencilEnable = False;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Gouraud;");
			vp->Add(c);
			break;

		case _BOTH:
			_sprintf(c.str,"StencilEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"StencilZFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"StencilFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Gouraud;");
			vp->Add(c);
			break;

		case _BOTHZ:
			_sprintf(c.str,"StencilEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"StencilZFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"StencilFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Gouraud;");
			vp->Add(c);
			break;

		case _STENCIL_NO_ZTEST:
			_sprintf(c.str,"AlphaBlendEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"SrcBlend = Zero;");
			vp->Add(c);
			_sprintf(c.str,"DestBlend = One;");
			vp->Add(c);
			_sprintf(c.str,"StencilFunc = Always;");
			vp->Add(c);

			_sprintf(c.str,"StencilZFail = Replace;");
			vp->Add(c);
			_sprintf(c.str,"StencilFail = Keep;");
			vp->Add(c);
			_sprintf(c.str,"StencilEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"ShadeMode = Flat;");
			vp->Add(c);
			break;

		case _ZBUFFER:
			_sprintf(c.str,"AlphaBlendEnable = True;");
			vp->Add(c);
			_sprintf(c.str,"SrcBlend = Zero;");
			vp->Add(c);
			_sprintf(c.str,"DestBlend = One;");
			vp->Add(c);
			_sprintf(c.str,"ZWriteEnable = True;");
			vp->Add(c);
			break;

		};
	}

	c.str[0]='\0';
	vp->Add(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::new_constant_register()
{
	last_constant_register=actual_cst;
	last_constant_register+=actual_cst_size;
	return actual_cst;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::new_ps_constant_register()
{
	last_ps_constant_register=actual_cst;
	last_ps_constant_register+=1;
	return actual_cst;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_input_register(char *name,char * trans)
{
	iregisters[last_iregister][0]=name;
	iregisters[last_iregister++][1]=trans;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_output_register(char *name,char * trans)
{
	oregisters[last_oregister][0]=name;
	oregisters[last_oregister++][1]=trans;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_output2_register(char *name,char * trans)
{
	o2registers[last_o2register][0]=name;
	o2registers[last_o2register++][1]=trans;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_constant(char *name,int size)
{
	constants[last_constant]=name;
	l_constants[last_constant]=size;
	last_constant+=LENGTH_VAR[size];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_constant(char *name,int size,int nb)
{
	constants[last_constant]=name;
	l_constants[last_constant]=size + (nb<<16);
	last_constant+=LENGTH_VAR[size]*nb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVPVariable * CVertexProgram::Translate(char *str)
{
	CVPVariable *res,*v;
	if (str[0]!='r')
	{
		res=NULL;
		v=Vars.GetFirst();
		while (v)
		{
			if (strcmp(v->name,str)==0) res=v;
			v=Vars.GetNext();
		}
		return res;
	}
	else
	{
		res=VariableNulle;
		return res;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::set_translate_constant(char *cst,char *translate)
{
	if (is_constant(cst))
	{
		translate_constants[actual_cst]=translate;
	}
	return actual_cst;
}

// PIXELSHADER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_psvector(char *name)
{
	psvectors[npsvectors++]=name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::is_psvector(char *name)
{
	for (int n=0;n<npsvectors;n++)
		if (strcmp(psvectors[n],name)==0) return n;
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_psmatrix(char *name)
{
	psmatrices[npsmatrices++]=name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::is_psmatrix(char *name)
{
	for (int n=0;n<npsmatrices;n++)
		if (strcmp(psmatrices[n],name)==0) return n;
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_ps_constant(char *name)
{
	ps_constants[last_ps_constant]=name;
	ps_constants_type[last_ps_constant]=1;
	last_ps_constant++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::new_ps_matrix(char *name)
{
	ps_constants[last_ps_constant]=name;
	ps_constants_type[last_ps_constant]=4;
	last_ps_constant++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::set_translate_ps_constant(char *cst,char *translate)
{
	if (is_ps_constant(cst))
	{
		translate_ps_constants[actual_cst]=translate;
	}
	return actual_cst;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::set_translate_ps_matrix(char *cst,char *translate)
{
	if (is_ps_matrix(cst))
	{
		translate_ps_constants[actual_cst]=translate;
	}
	return actual_cst;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CVertexProgram::is_ps_constant(char *name)
{
	int n;
	char* s=NULL;

	for (n=0;n<last_ps_constant;n++)
	{
		if (ps_constants[n])
			if (strcmp(name,ps_constants[n])==0)
			{
				s=ps_constants[n];
				actual_cst=n;
				actual_cst_size=1;
			}
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CVertexProgram::is_ps_matrix(char *name)
{
	int n;
	char* s=NULL;

	for (n=0;n<last_ps_constant;n++)
	{
		if (ps_constants[n])
			if (strcmp(name,ps_constants[n])==0)
			{
				s=ps_constants[n];
				actual_cst=n;
				actual_cst_size=4;
			}
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::AddInst(CList <Code> *vp,Code &c)
{
	Code cc;

	if (addsemi)
	{
		if (api==1)
		{
			if (((str_match(c.str,"for"))||(str_match(c.str,"while"))||(str_match(c.str,"if"))||(str_match(c.str,"else"))||(str_match(c.str,"{"))||(str_match(c.str,"}")))) _sprintf(cc.str,"%s",c.str);
			else _sprintf(cc.str,"%s;",c.str);
		}
		else _sprintf(cc.str,"%s;",c.str);

		if (pixelshader)
		{
			vp->Add(cc);
			if (!str_match(c.str,"texld")) compteur_inst++;
		}
		else
		{
			vp->Add(cc);
		}
	}
	else
	{
		if (pixelshader)
		{
			vp->Add(c);
			if (!str_match(c.str,"texld")) compteur_inst++;
		}
		else
		{
			vp->Add(c);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::AddInst2(CList <Code> *vp,Code &c)
{
    Code cc;
    
    if (addsemi)
    {
        _sprintf(cc.str,"%s",c.str);
        
        if (pixelshader)
        {
            vp->Add(cc);
            if (!str_match(c.str,"texld")) compteur_inst++;
        }
        else
        {
            vp->Add(cc);
        }
    }
    else
    {
        if (pixelshader)
        {
            vp->Add(c);
            if (!str_match(c.str,"texld")) compteur_inst++;
        }
        else
        {
            vp->Add(c);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::AddInst00(CList <Code> *vp,Code &c)
{
	Code cc;

	if (addsemi)
	{
		_sprintf(cc.str,"%s",c.str);	

		if (pixelshader)
		{
			vp->Add(cc);
			if (!str_match(c.str,"texld")) compteur_inst++;
		}
		else
		{
			vp->Add(cc);
		}
	}
	else
	{
		if (pixelshader)
		{
			vp->Add(c);
			if (!str_match(c.str,"texld")) compteur_inst++;
		}
		else
		{
			vp->Add(c);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * signedvar(char *ss,int neg)
{
	char *res=newString();

	if (neg==0) strcpy(res,ss);
	if (neg==1) _sprintf(res,"-%s",ss);
	if (neg==2) _sprintf(res,"1-%s",ss);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * signedvarf(char *ss,int neg)
{
	char *res=newString();

	if (neg==0) _sprintf(res,"float(%s)",ss);
	if (neg==1) _sprintf(res,"-float(%s)",ss);
	if (neg==2) _sprintf(res,"1-float(%s)",ss);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::register_writeonly(char *str00)
{
	Chaine s;
	int neg;
	char str[128];

	if (str00[0]=='-') { strcpy(str,&str00[1]);neg=1;}
	else
		if ((str00[0]=='1')&&(str00[1]=='-')) { strcpy(str,&str00[1]);neg=2;}
		else {strcpy(str,str00);neg=0;}

	if ((str_char(str,'.')!=-1)&&(str_char(str,'[')==-1))
	{
		strcpy(s.str,str);
		s.str[str_char(str,'.')]='\0';
	}
	else
	{
		strcpy(s.str,str);
	}

	return (is_output(s.str)!=NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int nummodifiers(char *str)
{
	char modifier[32];
	if ((str_char(str,'.')!=-1)&&(str_char(str,'[')==-1))
	{
		strcpy(modifier,&str[str_char(str,'.')+1]);
	}
	else
	{		
		modifier[0]='\0';
	}
	return (int)strlen(modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::var(char * str00)
{
	return var(str00,0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int nametype(char *s)
{	
	if (str_match(s,"float")) return 1;
	if (str_match(s,"float2")) return 1;
	if (str_match(s,"float3")) return 1;
	if (str_match(s,"float4")) return 1;
	if (str_match(s,"vector2")) return 1;
	if (str_match(s,"vector3")) return 1;
	if (str_match(s,"vector4")) return 1;
	if (str_match(s,"vec2")) return 1;
	if (str_match(s,"vec3")) return 1;
	if (str_match(s,"vec4")) return 1;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::var(char * str00,int tag)
{
	Code cc;
	Chaine s;
	char *modifier;
	char *ss;
	char *ss2;
	int index;
	int adr_reg;
	int adr;
	char * temp;
	int p;
	int neg;
	char str[128];
    
    if (str00==NULL) return "";

	if (strcmp(str00,"trigo_mod")==0)  trigocst[actual_pass_trigocst][0]=1;
	if (strcmp(str00,"trigo_cst")==0)  trigocst[actual_pass_trigocst][1]=1;
	if (strcmp(str00,"trigo_cst2")==0)  trigocst[actual_pass_trigocst][2]=1;
	if (strcmp(str00,"trigo_cst3")==0)  trigocst[actual_pass_trigocst][3]=1;
	if (strcmp(str00,"trigo_cst4")==0)  trigocst[actual_pass_trigocst][4]=1;
	if (strcmp(str00,"zerocinq")==0) trigocst[actual_pass_trigocst][5]=1;

	ss=newString();

	if (pixelshader)
	{
		strcpy(str,str00);
		p=str_char(str,'.');
		if (p>=0) str[p]='\0';
        if (str[0]=='-')
        {
            if (is_psvector(&str[1])>=0)
            {
                strcpy(ss,str00);
                return ss;
            }
        }
        else
		if (is_psvector(str)>=0)
		{
			strcpy(ss,str00);
			return ss;
		}
        else
		if (is_psmatrix(str)>=0)
		{
			strcpy(ss,str00);
			return ss;
		}
	}

	modifier=newString();
	temp=newString();

	if (compile_expression)
	{ 
		if (str00[0]=='-')
		{
			if (isdefined(&str00[1])<0)
			{
				if (!DEFINEERROR) strcpy(ERRORSTRPARAM,str00);
				DEFINEERROR=true;
			}
		}
		else
		{
			if (isdefined(str00)<0)
			{
				if (!DEFINEERROR) strcpy(ERRORSTRPARAM,str00);
				DEFINEERROR=true;
			}
		}
	}

	if (str_char(str00,'[')>=0)
	{
		_sprintf(ss,str00);
		p=str_char(ss,'[');
		ss[p]='\0';

		for (p=0;p<nmatrices;p++)
		{
			if (strcmp(matrices[p],ss)==0) return str00;
		}
	}
	else
	{
		for (p=0;p<nmatrices;p++)
		{
			if (strcmp(matrices[p],str00)==0) return str00;
		}

		for (p=0;p<nfloats;p++)
		{
			if (strcmp(floats[p],str00)==0)
			{
				if (divprocessing) return str00;
				else
				if (floatprocessing) return str00;
				else
				{
					if (isfloat(str00))
					{
						return str00;
					}
					else
					{
						if (nbmodifiersprocessing==4)
						{
							if (api==0) _sprintf(temp,"float4( %s, %s, %s, %s )",str00,str00,str00,str00);
							else _sprintf(temp,"vec4( %s, %s, %s, %s )",str00,str00,str00,str00);
						}

						if (nbmodifiersprocessing==3)
						{
							if (api==0) _sprintf(temp,"float3( %s, %s, %s )",str00,str00,str00);
							else _sprintf(temp,"vec3( %s, %s, %s )",str00,str00,str00);
						}

						if (nbmodifiersprocessing==2)
						{
							if (api==0) _sprintf(temp,"float2( %s, %s )",str00,str00);
							else _sprintf(temp,"vec2( %s, %s )",str00,str00);
						}

						return temp;
					}
				}
			}
		}

	}

	if (str00[0]=='-')
	{ 
		strcpy(str,&str00[1]); neg=1;

		if (numb(str))
		{
			if (api==0)
			{
				if (strlen(str)>0)
				{
					if (str_char(str00,'.')!=-1) _sprintf(ss,"%sf",str00);
					else _sprintf(ss,"%s.0f",str00);
				}
				else strcpy(ss,str00);
			}
			else
			{
				if (str_char(str00,'.')!=-1) strcpy(ss,str00);
				else _sprintf(ss,"%s.0",str00);
			}
			return ss;
		}

		if (isfloat(str)) return str00;
	}
	else
	{
		if ((str00[0]=='1')&&(str00[1]=='-')) { strcpy(str,&str00[1]);neg=2;}
		else {strcpy(str,str00);neg=0;}
	}

	if (numb(str00))
	{
		if (api==0)
		{
			if (strlen(str00)>0)
			{
				if (str_char(str00,'.')!=-1) _sprintf(ss,"%sf",str00);
				else _sprintf(ss,"%s.0f",str00);
			}
			else strcpy(ss,str00);
		}
		else
		{
			if (str_char(str00,'.')!=-1) strcpy(ss,str00);
			else _sprintf(ss,"%s.0",str00);
		}
		return ss;
	}

	if (strcmp(str,"SCALAR_1DIV3")==0) { _sprintf(str,"trigo_cst3.x"); trigocst[actual_pass_trigocst][3]=1; }
	if (strcmp(str,"SCALAR_1DIV5")==0) { _sprintf(str,"trigo_cst4.y"); trigocst[actual_pass_trigocst][4]=1; }
	if (strcmp(str,"SCALAR_1DIV10")==0) { _sprintf(str,"trigo_cst3.w"); trigocst[actual_pass_trigocst][3]=1; }
	if (strcmp(str,"SCALAR_1DIV4")==0) { _sprintf(str,"trigo_cst4.w"); trigocst[actual_pass_trigocst][4]=1; }

	if (strcmp(str,"SCALAR_HALF")==0) { _sprintf(str,"trigo_cst2.w"); trigocst[actual_pass_trigocst][2]=1; }
	if (strcmp(str,"SCALAR_ZERO")==0) { _sprintf(str,"trigo_cst.x"); trigocst[actual_pass_trigocst][1]=1; }
	if (strcmp(str,"SCALAR_ONE")==0) { _sprintf(str,"trigo_cst.z"); trigocst[actual_pass_trigocst][1]=1; }
	if (strcmp(str,"SCALAR_PI")==0) { _sprintf(str,"trigo_cst.w"); trigocst[actual_pass_trigocst][1]=1; }

	if (strcmp(str,"SPRITEZVALUE")==0)
	{
		if (api==1) return "0.00002";
		else return "0.00002f";
	}

	if (strcmp(str,"ZSCALE")==0)
	{
		if (api==1) return "0.00002";
		else return "0.00002f";
	}

	if ((str[0]=='r')&&((str[1]>='0')&&(str[1]<='9')))
	{
		if ((str_char(str,'.')!=-1)&&(str_char(str,'[')==-1))
		{
			strcpy(s.str,str);
			s.str[str_char(str,'.')]='\0';
			_sprintf(modifier,"%s",&str[str_char(str,'.')]);
		}
		else
		{
			strcpy(s.str,str);
			modifier[0]='\0';
		}

		int reg=0;

		if (str[1] == '0') { ss = "r0"; reg=0; }
		if (str[1] == '1') { ss = "r1"; reg=1; }
		if (str[1] == '2') { ss = "r2"; reg=2; }
		if (str[1] == '3') { ss = "r3"; reg=3; }
		if (str[1] == '4') { ss = "r4"; reg=4; }
		if (str[1] == '5') { ss = "r5"; reg=5; }
		if (str[1] == '6') { ss = "r6"; reg=6; } 
		if (str[1] == '7') { ss = "r7"; reg=7; } 
		if (str[1] == '8') { ss = "r8"; reg=8; } 
		if (str[1] == '9') { ss = "r9"; reg=9; } 
		if ((str[1] == '1') && (str[2] == '0')) { ss = "r10"; reg=10; }
		if ((str[1] == '1') && (str[2] == '1')) { ss = "r11"; reg=11; }
		
		if (shadermodel)
		{
			if ((str[1] == '1') && (str[2] == '2')) { ss = "r12"; reg=12; }
			if ((str[1] == '1') && (str[2] == '3')) { ss = "r13"; reg=13; }
			if ((str[1] == '1') && (str[2] == '4')) { ss = "r14"; reg=14; }
			if ((str[1] == '1') && (str[2] == '5')) { ss = "r15"; reg=15; }
			if ((str[1] == '1') && (str[2] == '6')) { ss = "r16"; reg=16; }
		}

		if (shadermodel3)
		{
			if ((str[1] == '1') && (str[2] == '7')) { ss = "r17"; reg=17; }
			if ((str[1] == '1') && (str[2] == '8')) { ss = "r18"; reg=18; }
			if ((str[1] == '1') && (str[2] == '9')) { ss = "r19"; reg=19; }
			if ((str[1] == '2') && (str[2] == '0')) { ss = "r20"; reg=20; }
			if ((str[1] == '2') && (str[2] == '1')) { ss = "r21"; reg=21; }
			if ((str[1] == '2') && (str[2] == '2')) { ss = "r22"; reg=22; }
			if ((str[1] == '2') && (str[2] == '3')) { ss = "r23"; reg=23; }
			if ((str[1] == '2') && (str[2] == '4')) { ss = "r24"; reg=24; }
			if ((str[1] == '2') && (str[2] == '5')) { ss = "r25"; reg=25; }
			if ((str[1] == '2') && (str[2] == '6')) { ss = "r26"; reg=26; }
			if ((str[1] == '2') && (str[2] == '7')) { ss = "r27"; reg=27; }
			if ((str[1] == '2') && (str[2] == '8')) { ss = "r28"; reg=28; }
			if ((str[1] == '2') && (str[2] == '9')) { ss = "r29"; reg=29; }
			if ((str[1] == '3') && (str[2] == '0')) { ss = "r30"; reg=30; }
			if ((str[1] == '3') && (str[2] == '1')) { ss = "r31"; reg=31; }

		}

		if (reg<12)
		{
			if (!TAG_secondary_vars) tagregsave[reg] = tagreg[reg] = 1;
			else tagregsave[reg]=1;
		}
		else tagregsave[reg] = tagreg[reg] = 1;

		if (neg==1) _sprintf(temp,"-%s%s",ss,modifier);
		else
		if (neg==2) _sprintf(temp,"1 - %s%s",ss,modifier);
		else _sprintf(temp,"%s%s",ss,modifier);

		return temp;
	}

	CVPVariable * c;

	if ((str_char(str,'.')!=-1)&&(str_char(str,'[')==-1))
	{
		strcpy(s.str,str);
		s.str[str_char(str,'.')]='\0';
		_sprintf(modifier,"%s",&str[str_char(str,'.')]);
	}
	else
	{
		strcpy(s.str,str);
		modifier[0]='\0';
	}

	adr_reg=0;
	adr=0;

	if (str_char(s.str,'[')!=-1)
	{
		recursvar=1;

		strcpy(ss,str_return_crochets(s.str));

		if (!str_int(ss))
		{
			if ((api==1)&&(!shadermodel))
			{
				_sprintf(temp,"r%d",new_temp_register());
				ss2=compile(ss,temp,actual_vp);
				close_temp_register();

				_sprintf(cc.str,"ARL A0.x,%s",ss2);
				AddInst(actual_vp,cc);
				adr_reg=1;
				strcpy(ss,s.str);
				ss[str_char(ss,'[')]='\0';
                strcpy(s.str,ss);
			}

			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				adr_reg=1;
				_sprintf(temp,"r%d.x",new_temp_register());
				ss2=compile(ss,temp,actual_vp);
				close_temp_register();
                strcpy(ss,s.str);
				ss[str_char(ss,'[')]='\0';
				_sprintf(s.str,"%s[%s]",ss,ss2);
			}

			if ((api==0)&&(!shadermodel3))
			{
				_sprintf(temp,"r%d",new_temp_register());
				ss2=compile(ss,temp,actual_vp);
				close_temp_register();
				if (shadermodel) _sprintf(cc.str,"mova a0.x,%s",ss2);
				else _sprintf(cc.str,"mov a0.x,%s",ss2);
				AddInst(actual_vp,cc);
				adr_reg=1;
                strcpy(ss,s.str);
				ss[str_char(ss,'[')]='\0';
				_sprintf(s.str,"%s0",ss);
			}

			if (api==2)	// gl - my interpreter
			{
				_sprintf(temp,"r%d",new_temp_register());
				ss2=compile(ss,temp,actual_vp);
				close_temp_register();

				em->Add(OP_MOV,"a0.x",ss2,NULL);
				adr_reg=1;
                strcpy(ss,s.str);
				ss[str_char(ss,'[')]='\0';
				_sprintf(s.str,"%s0",ss);
			}
		}
		else
		{
			sscanf(ss,"%d",&index);
            strcpy(ss,s.str);
			ss[str_char(ss,'[')]='\0';

			if (!shadermodel3)
			{
				if ((api==0)||(api==2))
					_sprintf(s.str,"%s%d",ss,index);
				else
				{
                    strcpy(s.str,ss);
					adr=1;
				}
			}
			else
			{
				_sprintf(s.str,"%s[%d]",ss,index);
			}
		}

		p=str_char(str,']');
		if (str_char(&str[p],'.')!=-1)
		{
            strcpy(modifier,&str[p+str_char(&str[p],'.')]);
		}
		recursvar=0;
	}

	if (!TAG_secondary_vars)
	{
		c=Vars(s);

		if (!c)
		{
			if (pixelshader)
			{
				if (is_ps_constant(s.str))
				{
					if ((api==1)||(shadermodel3))
					{
                        strcpy(ss,s.str);
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;
						if (adr_reg) _sprintf(ss,"%s%s",s.str,modifier);
						//if (adr_reg) _sprintf(ss,"%s[A0.x]%s",ss,modifier);
						else
						{
							if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
							else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
						}
						return signedvar(ss,neg);
					}
					else
					{
						index=new_ps_constant_register();
						_sprintf(ss,"c%d",index);
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;
						if (adr_reg) _sprintf(ss,"c[%d+a0.x]%s",index,modifier);
						else _sprintf(ss,"%s%s",ss,modifier);

						return signedvar(ss,neg);
					}
				}

			}
			else
			{
                strcpy(ss,s.str);
				if (str_char(s.str,'[')!=-1)
				{
					ss[str_char(s.str,'[')]='\0';
				}

				if (is_constant(ss))
				{
					if ((api==1)&&(shadermodel))
					{
                        strcpy(ss,s.str);
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;
						if (adr_reg) _sprintf(ss,"%s%s",s.str,modifier);
						else
						{
							if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
							else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
						}
						return signedvar(ss,neg);
					}
					else
					if ((api==1)&&(!shadermodel))
					{
                        strcpy(ss,s.str);
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;
						if (adr_reg) _sprintf(ss,"%s[A0.x]%s",ss,modifier);
						else
						{
							if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
							else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
						}
						return signedvar(ss,neg);
					}
					else
					if ((api==0)&&(!shadermodel3))
					{
						index=new_constant_register();
						_sprintf(ss,"c%d",index);
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;
						if (adr_reg) _sprintf(ss,"c[%d+a0.x]%s",index,modifier);
						else _sprintf(ss,"%s%s",ss,modifier);

						return signedvar(ss,neg);
					}
					else
					{
						index=new_constant_register();

                        strcpy(ss,s.str);
						
						c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
						Vars.Add(s.str,*c);
						delete c;

						if (adr_reg) _sprintf(ss,"%s%s",s.str,modifier);
						else _sprintf(ss,"%s%s",ss,modifier);
					
						return signedvar(ss,neg);
					}
			
				}
			}
			
			if (is_output(s.str))
			{
                strcpy(ss,is_output(s.str)[1]);
				c=new CVPVariable(OREGISTER,ss,LASTLINESTR);
				c->tobeused=true;
				Vars.Add(s.str,*c);
				delete c;
				_sprintf(ss,"%s%s",ss,modifier);
				return signedvar(ss,neg);
			}
			else
			if (is_input(s.str))
			{
				if ((recursvar==0)&&(strcmp(s.str,"iWIndices")==0)&&(api==1))
				{
					strcpy(ss,is_input(s.str)[1]);
					c=new CVPVariable(IREGISTER,ss,LASTLINESTR);
					c->tobeused=true;
					Vars.Add(s.str,*c);
					delete c;
					_sprintf(ss,"%s%s",ss,modifier);

					return signedvarf(ss,neg);
				}
				else
				{
					strcpy(ss,is_input(s.str)[1]);
					c=new CVPVariable(IREGISTER,ss,LASTLINESTR);
					c->tobeused=true;
					Vars.Add(s.str,*c);
					delete c;
					_sprintf(ss,"%s%s",ss,modifier);

					return signedvar(ss,neg);
				}
			}
			else
			{
				if (shadermodel3)
				{
					if (strcmp(s.str, "r31") == 0)
					{
                        strcpy(ss, "r31");
						tagregsave[31] = 1;
						max_temporary_register = 32;
					}
					else
					if (strcmp(s.str, "r30") == 0)
					{
                        strcpy(ss, "r30");
						tagregsave[30] = 1;
						max_temporary_register = 32;
					}
					else
					if (strcmp(s.str, "r29") == 0)
					{
                        strcpy(ss, "r29");
						tagregsave[29] = 1;
						max_temporary_register = 32;
					}
					else
					{
						if (tag==0) SYNTAXERROR=true;
						_sprintf(ss, "r%d", new_register());
						last_register++;
					}
				}
				else
				{
					if (strcmp(s.str, "r11") == 0)
					{
                        strcpy(ss, "r11");
						tagregsave[11] = 1;
						max_temporary_register = 12;
					}
					else
					if (strcmp(s.str, "r10") == 0)
					{
                        strcpy(ss, "r10");
						tagregsave[10] = 1;
						max_temporary_register = 12;
					}
					else
					if (strcmp(s.str, "r9") == 0)
					{
                        strcpy(ss, "r9");
						tagregsave[9] = 1;
						max_temporary_register = 12;
					}
					else
					{
						_sprintf(ss, "r%d", new_register());
						last_register++;
					}
				}
				c=new CVPVariable(REGISTER,ss,LASTLINESTR);
				c->tobeused=true;
				Vars.Add(s.str,*c);
				delete c;
				_sprintf(ss,"%s%s",ss,modifier);

				return signedvar(ss,neg);
			}
		}
		else
		{
			if (compile_expression) c->use++;			

			if (adr_reg)
			{
				if (((shadermodel)&&(api==1))||(shadermodel3))
				{
                    strcpy(ss,s.str);
				}
				else
				{
					if (api==1) _sprintf(ss,"%s[A0.x]%s",c->name,modifier);
					else _sprintf(ss,"c[%s+a0.x]%s",&c->name[1],modifier);
				}
			}
			else
			{
				if (adr)
				{
					is_constant(s.str);
					_sprintf(ss,"%s[%d]%s",c->name,index*actual_cst_size,modifier);
				}
				else
				{
					_sprintf(ss,"%s%s",c->name,modifier);
					if ((recursvar==0)&&(strcmp(c->name,"WIndices")==0)&&(api==1)) return signedvarf(ss,neg);
				}
			}

			return signedvar(ss,neg);
		}
	}
	else
	{
		c=TempVars(s);
		if (!c)
		{
			c=Vars(s);

			if (!c)
			{
				if (pixelshader)
				{
					if (is_ps_constant(s.str))
					{
						if (((api==1)&&(shadermodel))||(shadermodel3))
						{
                            strcpy(ss,s.str);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"%s%s",s.str,modifier);
							else
							{
								if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
								else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
							}
							return signedvar(ss,neg);
						}
						else
						if ((api==1)&&(!shadermodel))
						{
                            strcpy(ss,s.str);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"%s[A0.x]%s",ss,modifier);
							else
							{
								if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
								else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
							}
							return signedvar(ss,neg);
						}
						else
						{
							index=new_ps_constant_register();
							_sprintf(ss,"c%d",index);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"c[%d+a0.x]%s",index,modifier);
							else _sprintf(ss,"%s%s",ss,modifier);

							return signedvar(ss,neg);
						}
					}

				}
				else
				{
					if (is_constant(s.str))
					{
						if (api==1)
						{
                            strcpy(ss,s.str);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"%s[A0.x]%s",ss,modifier);
							else
							{
								if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
								else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
							}

							return signedvar(ss,neg);
						}
						else
						if (shadermodel3)
						{
                            strcpy(ss,s.str);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"%s[a0.x]%s",ss,modifier);
							else
							{
								if (adr==0) _sprintf(ss,"%s%s",ss,modifier);
								else _sprintf(ss,"%s[%d]%s",ss,index,modifier);
							}

							return signedvar(ss,neg);
						}
						else
						{
							index=new_constant_register();
							_sprintf(ss,"c%d",index);
							c=new CVPVariable(CONSTANT,ss,LASTLINESTR);
							Vars.Add(s.str,*c);
							delete c;
							if (adr_reg) _sprintf(ss,"c[%d+a0.x]%s",index,modifier);
							else _sprintf(ss,"%s%s",ss,modifier);

							return signedvar(ss,neg);
						}
					}
				}

				if (is_output(s.str))
				{
                    strcpy(ss,is_output(s.str)[1]);
					c=new CVPVariable(OREGISTER,ss,LASTLINESTR);

					c->tobeused=true;
					Vars.Add(s.str,*c);
					delete c;
					_sprintf(ss,"%s%s",ss,modifier);
					return signedvar(ss,neg);

				}
				else
				if (is_input(s.str))
				{
                    strcpy(ss,is_input(s.str)[1]);
					c=new CVPVariable(IREGISTER,ss,LASTLINESTR);
					c->tobeused=true;
					Vars.Add(s.str,*c);
					delete c;
					_sprintf(ss,"%s%s",ss,modifier);
					return signedvar(ss,neg);
				}
				else
				{
					if (shadermodel3)
					{
						if (strcmp(s.str, "r31") == 0)
						{
                            strcpy(ss, "r31");
							tagregsave[31] = 1;
							max_temporary_register = 32;
						}
						else
						if (strcmp(s.str, "r30") == 0)
						{
                            strcpy(ss, "r30");
							tagregsave[30] = 1;
							max_temporary_register = 32;
						}
						else
						if (strcmp(s.str, "r29") == 0)
						{
                            strcpy(ss, "r29");
							tagregsave[29] = 1;
							max_temporary_register = 32;
						}
						else _sprintf(ss, "r%d", new_temp_register());
					}
					else
					{
						if (strcmp(s.str, "r11") == 0)
						{
                            strcpy(ss, "r11");
							tagregsave[11] = 1;
							max_temporary_register = 12;
						}
						else
						if (strcmp(s.str, "r10") == 0)
						{
                            strcpy(ss, "r10");
							tagregsave[10] = 1;
							max_temporary_register = 12;
						}
						else
						if (strcmp(s.str, "r9") == 0)
						{
                            strcpy(ss, "r9");
							tagregsave[9] = 1;
							max_temporary_register = 12;
						}
						else _sprintf(ss, "r%d", new_temp_register());

					}
					c = new CVPVariable(REGISTER, ss,LASTLINESTR);
					c->tobeused=true;
					TempVars.Add(s.str,*c);
					delete c;
					_sprintf(ss,"%s%s",ss,modifier);
					return signedvar(ss,neg);
				}
			}
			else
			{
				if (adr_reg)
				{
					if ((shadermodel)&&(api==1))
					{
                        strcpy(ss,s.str);
					}
					else
					{
						if (api==1) _sprintf(ss,"%s[A0.x]%s",c->name,modifier);
						else
						{
							if (shadermodel3) _sprintf(ss,"%s[a0.x]%s",c->name,modifier);
							else _sprintf(ss,"c[%s+a0.x]%s",&c->name[1],modifier);
						}
					}
				}
				else
				{
					if (adr)
					{
						is_constant(s.str);
						_sprintf(ss,"%s[%d]%s",c->name,index*actual_cst_size,modifier);
					}
					else
					{
						_sprintf(ss,"%s%s",c->name,modifier);
						if ((recursvar==0)&&(strcmp(c->name,"WIndices")==0)&&(api==1)) return signedvarf(ss,neg);
					}
				}
				return signedvar(ss,neg);
			}
		}
		else
		{

			if (adr_reg)
			{
				if ((shadermodel)&&(api==1))
				{
                    strcpy(ss,s.str);
				}
				else
				{
					if (api==1) _sprintf(ss,"%s[A0.x]%s",c->name,modifier);
					else _sprintf(ss,"c[%s+a0.x]%s",&c->name[1],modifier);
				}
			}
			else
			{
				if (adr)
				{
					is_constant(s.str);
					_sprintf(ss,"%s[%d]%s",c->name,index*actual_cst_size,modifier);
				}
				else
				{
					_sprintf(ss,"%s%s",c->name,modifier);
					if ((recursvar==0)&&(strcmp(c->name,"WIndices")==0)&&(api==1)) return signedvarf(ss,neg);
				}
			}
			return signedvar(ss,neg);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_close()
{
    strcpy(c.str,"};");
	vp.Add(c);
	c.str[0]='\0';
	vp.Add(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_close_psh()
{
    strcpy(c.str,"};");
	vp.Add(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_end()
{
	c.str[0]='\0';
	vp.Add(c);
    strcpy(c.str,"}");
	vp.Add(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_open_pass(int npass)
{
	_sprintf(c.str,"PASS P%d",npass);
	vp.Add(c);
    strcpy(c.str,"{");
	vp.Add(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::setIO(unsigned int flags)
{
	if (flags&METAL)
	{
        new_output_register("oPos","o.p0");
        new_output_register("oSize","o.PSize");
		new_output_register("oDiffuse","o.v0");
		new_output_register("oSpecular","o.v1");
		new_output_register("oTexture0","o.t0");
		new_output_register("oTexture1","o.t1");
		new_output_register("oTexture2","o.t2");
		new_output_register("oTexture3","o.t3");
		new_output_register("oTexture4","o.t4");
		new_output_register("oTexture5","o.t5");
		new_output_register("oTexture6","o.t6");
		new_output_register("oTexture7","o.t7");

		new_output_register("Position","pixel.p0");
		new_output_register("Diffuse","pixel.v0");
		new_output_register("Specular","pixel.v1");
		new_output_register("Tex0","pixel.t0");
		new_output_register("Tex1","pixel.t1");
		new_output_register("Tex2","pixel.t2");
		new_output_register("Tex3","pixel.t3");
		new_output_register("Tex4","pixel.t4");
		new_output_register("Tex5","pixel.t5");
		new_output_register("Tex6","pixel.t6");
		new_output_register("Tex7","pixel.t7");

		new_output2_register("Position","pixel.p0");
		new_output2_register("Diffuse","pixel.v0");
		new_output2_register("Specular","pixel.v1");
		new_output2_register("Tex0","pixel.t0");
		new_output2_register("Tex1","pixel.t1");
		new_output2_register("Tex2","pixel.t2");
		new_output2_register("Tex3","pixel.t3");
		new_output2_register("Tex4","pixel.t4");
		new_output2_register("Tex5","pixel.t5");
		new_output2_register("Tex6","pixel.t6");
		new_output2_register("Tex7","pixel.t7");

		new_input_register("iPos","v0");
		new_input_register("iPos2","v1");
		new_input_register("iWeights","v1");
		new_input_register("iWIndices","v2");
		new_input_register("iNorm","v3");
		new_input_register("iNorm2","v4");
		new_input_register("iDiffuse","v5");
		new_input_register("iSpecular","v6");
		new_input_register("iTexture0","v7");
		new_input_register("iTexture1","v8");
		new_input_register("iTexture2","v9");

		new_input_register("Address","a0.x");
	}


	if ((flags&DIRECT3D)||(flags&EMULATED))
	{
		if (shadermodel3)
		{
			new_output_register("oPos","o.p0");
			new_output_register("oDiffuse","o.v0");
			new_output_register("oSpecular","o.v1");
			new_output_register("oTexture0","o.t0");
			new_output_register("oTexture1","o.t1");
			new_output_register("oTexture2","o.t2");
			new_output_register("oTexture3","o.t3");
			new_output_register("oTexture4","o.t4");
			new_output_register("oTexture5","o.t5");
			new_output_register("oTexture6","o.t6");
			new_output_register("oTexture7","o.t7");

			// pixel shaders
			new_output_register("Position","i.p0");
			new_output_register("Diffuse","i.v0");
			new_output_register("Specular","i.v1");
			new_output_register("Tex0","i.t0");
			new_output_register("Tex1","i.t1");
			new_output_register("Tex2","i.t2");
			new_output_register("Tex3","i.t3");
			new_output_register("Tex4","i.t4");
			new_output_register("Tex5","i.t5");
			new_output_register("Tex6","i.t6");
			new_output_register("Tex7","i.t7");


			new_output2_register("Position","ip0");
			new_output2_register("Diffuse","iv0");
			new_output2_register("Specular","iv1");
			new_output2_register("Tex0","it0");
			new_output2_register("Tex1","it1");
			new_output2_register("Tex2","it2");
			new_output2_register("Tex3","it3");
			new_output2_register("Tex4","it4");
			new_output2_register("Tex5","it5");
			new_output2_register("Tex6","it6");
			new_output2_register("Tex7","it7");
		}
		else
		{

			new_output_register("oPos","oPos");
			new_output_register("oDiffuse","oD0");
			new_output_register("oSpecular","oD1");
			new_output_register("oTexture0","oT0");
			new_output_register("oTexture1","oT1");
			new_output_register("oTexture2","oT2");
			new_output_register("oTexture3","oT3");
			new_output_register("oTexture4","oT4");
			new_output_register("oTexture5","oT5");
			new_output_register("oTexture6","oT6");
			new_output_register("oTexture7","oT7");

			// pixel shaders
			new_output_register("Diffuse","v0");
			new_output_register("Specular","v1");
			new_output_register("Tex0","t0");
			new_output_register("Tex1","t1");
			new_output_register("Tex2","t2");
			new_output_register("Tex3","t3");
			new_output_register("Tex4","t4");
			new_output_register("Tex5","t5");
			new_output_register("Tex6","t6");
			new_output_register("Tex7","t7");
		}

		if (!shadermodel)
		{
			new_input_register("iPos","v0");
			new_input_register("iPos2","v1");
			new_input_register("iWeights","v1");
			new_input_register("iWIndices","v2");
			new_input_register("iNorm","v3");
			new_input_register("iNorm2","v4");
			new_input_register("iDiffuse","v5");
			new_input_register("iSpecular","v6");
			new_input_register("iTexture0","v7");
			new_input_register("iTexture1","v8");
			new_input_register("iTexture2","v9");
		}

		new_input_register("Address","a0.x");
	}

	if (flags&OPENGL)
	{
		if (!shadermodel)
		{
			new_input_register("iPos","iPos");
			new_input_register("iNorm","iNorm");
			new_input_register("iPos2","iPos2");
			new_input_register("iNorm2","iNorm2");
			new_input_register("iDiffuse","iDiffuse");
			new_input_register("iSpecular","iSpecular");
			new_input_register("iTexture0","iTexture0");
			new_input_register("iTexture1","iTexture1");
			new_input_register("iTexture2","iTexture2");
			new_input_register("iWeights","iWeights");
			new_input_register("iWIndices","iWIndices");

			new_input_register("Address","A0.x");

			new_output_register("oPos","oPos");
			new_output_register("oDiffuse","oDiffuse");
			new_output_register("oSpecular","oSpecular");
			new_output_register("oTexture0","oTexture0");
			new_output_register("oTexture1","oTexture1");
			new_output_register("oTexture2","oTexture2");
			new_output_register("oTexture3","oTexture3");
			new_output_register("oTexture4","oTexture4");

			new_output_register("Tex0","fragment.texcoord[0]");
			new_output_register("Tex1","fragment.texcoord[1]");
			new_output_register("Tex2","fragment.texcoord[2]");
			new_output_register("Tex3","fragment.texcoord[3]");
			new_output_register("Tex4","fragment.texcoord[4]");
			new_output_register("Diffuse","fragment.color");
			new_output_register("Specular","fragment.color.secondary");
		}
		else
		{
			new_input_register("iPos","VertexPosition");
			new_input_register("iNorm","VertexNormal");
			new_input_register("iPos2","VertexPosition2");
			new_input_register("iNorm2","VertexNormal2");
			new_input_register("iDiffuse","VertexColor");
			new_input_register("iSpecular","VertexSecondaryColor");
			new_input_register("iTexture0","MultiTexCoord0");
			new_input_register("iTexture1","MultiTexCoord1");
			new_input_register("iTexture2","MultiTexCoord2");
			new_input_register("iTexture3","MultiTexCoord3");
			new_input_register("iWeights","Weights");
			new_input_register("iWIndices","WIndices");

			new_input_register("Address","A0.x");

			new_output_register("oPos","gl_Position");

#ifdef GLES20
			new_output_register("oDiffuse","myColor");
			new_output_register("oSpecular","mySecondaryColor");

			new_output_register("oTexture0","myTexCoord[0]");
			new_output_register("oTexture1","myTexCoord[1]");
			new_output_register("oTexture2","myTexCoord[2]");
			new_output_register("oTexture3","myTexCoord[3]");
			new_output_register("oTexture4","myTexCoord[4]");
			new_output_register("oTexture5","myTexCoord[5]");
			new_output_register("oTexture6","myTexCoord[6]");
			new_output_register("oTexture7","myTexCoord[7]");

			new_output_register("Tex0","myTexCoord[0]");
			new_output_register("Tex1","myTexCoord[1]");
			new_output_register("Tex2","myTexCoord[2]");
			new_output_register("Tex3","myTexCoord[3]");
			new_output_register("Tex4","myTexCoord[4]");
			new_output_register("Tex5","myTexCoord[5]");
			new_output_register("Tex6","myTexCoord[6]");
			new_output_register("Tex7","myTexCoord[7]");

			new_output_register("Diffuse","myColor");
			new_output_register("Specular","mySecondaryColor");
#else
			new_output_register("oDiffuse","gl_FrontColor");
			new_output_register("oSpecular","gl_FrontSecondaryColor");

			new_output_register("oTexture0","gl_TexCoord[0]");
			new_output_register("oTexture1","gl_TexCoord[1]");
			new_output_register("oTexture2","gl_TexCoord[2]");
			new_output_register("oTexture3","gl_TexCoord[3]");
			new_output_register("oTexture4","gl_TexCoord[4]");
			new_output_register("oTexture5","gl_TexCoord[5]");
			new_output_register("oTexture6","gl_TexCoord[6]");
			new_output_register("oTexture7","gl_TexCoord[7]");

			new_output_register("Tex0","gl_TexCoord[0]");
			new_output_register("Tex1","gl_TexCoord[1]");
			new_output_register("Tex2","gl_TexCoord[2]");
			new_output_register("Tex3","gl_TexCoord[3]");
			new_output_register("Tex4","gl_TexCoord[4]");
			new_output_register("Tex5","gl_TexCoord[5]");
			new_output_register("Tex6","gl_TexCoord[6]");
			new_output_register("Tex7","gl_TexCoord[7]");

			new_output_register("Diffuse","gl_Color");
			new_output_register("Specular","gl_SecondaryColor");
#endif

			new_output_register("Position","gl_FragCoord");

#if defined(GLES20)||defined(GLESFULL)
			if (modifyZ) new_output_register("Depth","gl_FragDepthEXT");
#else
			if (modifyZ) new_output_register("Depth","gl_FragDepth");
#endif
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_open_vsh(int pass,unsigned int flags,unsigned int output)
{
	max_temporary_register=0;

	local_fvf_flags=flags;

	if ((flags&DIRECT3D)&&(!shadermodel3))
	{
		c.str[0]='\0';
		vp.Add(c);
		int NB=96;
		if (shadermodel) NB=256;

		for (int n=0;n<NB;n++)
		{
			if (constants[n])
			{
				if (translate_constants[n])
				{
					_sprintf(c.str,"VertexShaderConstant%d[%d] = { %s };",LENGTH_VAR[l_constants[n]&0xffff],n,translate_constants[n]);
				}
				else
				{
					_sprintf(c.str,"VertexShaderConstant%d[%d] = <%s>;",LENGTH_VAR[l_constants[n]&0xffff],n,constants[n]);
				}
				vp.Add(c);
			}
		}

		c.str[0]='\0';
		vp.Add(c);
        strcpy(c.str,"VertexShader =");
		vp.Add(c);
        strcpy(c.str,"decl");
		vp.Add(c);
        strcpy(c.str,"{");
		vp.Add(c);
		if ((flags&D3D9)==0)
		{
			if (flags&VPMORPH)
			{
                strcpy(c.str,"stream 0;");vp.Add(c);
                strcpy(c.str,"float v0[3];");vp.Add(c);
                strcpy(c.str,"float v3[3];");vp.Add(c);
                strcpy(c.str,"float v7[2];");vp.Add(c);
                strcpy(c.str,"stream 1;");vp.Add(c);
                strcpy(c.str,"float v1[3];");vp.Add(c);
                strcpy(c.str,"float v4[3];");vp.Add(c);
                strcpy(c.str,"float v8[2];");vp.Add(c);
			}
			else
			{
				// vertex format
				if (flags&XYZ) { strcpy(c.str,"float v0[3];");vp.Add(c); }
				if (flags&BLEND)
				{
                    strcpy(c.str,"float v1[4];");vp.Add(c);
                    strcpy(c.str,"ubyte v2[4];");vp.Add(c);
				}
				if (flags&NORMAL) { strcpy(c.str,"float v3[3];");vp.Add(c); }
				if (flags&DIFFUSE) { strcpy(c.str,"d3dcolor v5[1];");vp.Add(c); }
				if (flags&SPECULAR) { strcpy(c.str,"d3dcolor v6[1];");vp.Add(c); }
				if (flags&_TEX0) { strcpy(c.str,"float v7[2];");vp.Add(c); }
				if (flags&_TEX1) { strcpy(c.str,"float v8[2];");vp.Add(c); }
				if (flags&_TEX2) { strcpy(c.str,"float v9[2];");vp.Add(c); }
			}
		}

        strcpy(c.str,"}");
		vp.Add(c);
        strcpy(c.str,"asm");
		vp.Add(c);
        strcpy(c.str,"{");
		vp.Add(c);
		if (shadermodel) strcpy(c.str,"vs.2.x");
		else strcpy(c.str,"vs.1.1");
		vp.Add(c);

		if (flags&D3D9)
		{
			if (shadermodel)
			{
				last_iregister=0;

				if (flags&VPMORPH)
				{
					if (flags&BLEND)
					{
						strcpy(c.str,"dcl_position0 v0");vp.Add(c);
                        strcpy(c.str,"dcl_blendweight0 v1");vp.Add(c);
                        strcpy(c.str,"dcl_blendindices0 v2");vp.Add(c);
						strcpy(c.str,"dcl_normal0 v4");vp.Add(c);
						strcpy(c.str,"dcl_texcoord0 v7");vp.Add(c);

						strcpy(c.str,"dcl_position1 v3");vp.Add(c);
                        strcpy(c.str,"dcl_blendweight1 v6");vp.Add(c);
                        strcpy(c.str,"dcl_blendindices1 v8");vp.Add(c);
						strcpy(c.str,"dcl_normal1 v5");vp.Add(c);
						strcpy(c.str,"dcl_texcoord1 v9");vp.Add(c);


						new_input_register("iWeights","v1");
						new_input_register("iWIndices","v2");
						new_input_register("iPos","v0");
						new_input_register("iPos2","v3");
						new_input_register("iNorm","v4");
						new_input_register("iNorm2","v5");
						new_input_register("iTexture0","v7");
					}
					else
					{
						strcpy(c.str,"dcl_position0 v0");vp.Add(c);
						strcpy(c.str,"dcl_position1 v1");vp.Add(c);
						strcpy(c.str,"dcl_normal0 v2");vp.Add(c);
						strcpy(c.str,"dcl_normal1 v3");vp.Add(c);
						strcpy(c.str,"dcl_texcoord v4");vp.Add(c);

						new_input_register("iPos","v0");
						new_input_register("iPos2","v1");
						new_input_register("iNorm","v2");
						new_input_register("iNorm2","v3");
						new_input_register("iTexture0","v4");
					}
				}
				else
				{
					new_input_register("iPos","v0");
					if (flags&XYZ) { _sprintf(c.str,"dcl_position v0");vp.Add(c); }

					int v=1;

					if (flags&BLEND) {
                        strcpy(c.str,"dcl_blendweight v1");vp.Add(c);
                        strcpy(c.str,"dcl_blendindices v2");vp.Add(c);

						new_input_register("iWeights","v1");
						new_input_register("iWIndices","v2");

						v=3;
					}
					
					if (v==3)
					{
						if (flags&NORMAL) { strcpy(c.str,"dcl_normal v3");vp.Add(c); }
						if (flags&DIFFUSE) { strcpy(c.str,"dcl_color0 v3");vp.Add(c); }

						new_input_register("iNorm","v3");
						new_input_register("iDiffuse","v3");

						if (flags&_TEX0) { strcpy(c.str,"dcl_texcoord0 v4");vp.Add(c); }
						if (flags&_TEX1) { strcpy(c.str,"dcl_texcoord1 v5");vp.Add(c); }
						if (flags&_TEX2) { strcpy(c.str,"dcl_texcoord2 v6");vp.Add(c); }

						new_input_register("iTexture0","v4");
						new_input_register("iTexture1","v5");
						new_input_register("iTexture2","v6");

					}
					else
					{
						if (flags&NORMAL) { strcpy(c.str,"dcl_normal v1");vp.Add(c); }
						if (flags&DIFFUSE) { strcpy(c.str,"dcl_color0 v1");vp.Add(c); }

						new_input_register("iNorm","v1");
						new_input_register("iDiffuse","v1");

						if (flags&_TEX0) { strcpy(c.str,"dcl_texcoord0 v2");vp.Add(c); }
						if (flags&_TEX1) { strcpy(c.str,"dcl_texcoord1 v3");vp.Add(c); }
						if (flags&_TEX2) { strcpy(c.str,"dcl_texcoord2 v4");vp.Add(c); }

						new_input_register("iTexture0","v2");
						new_input_register("iTexture1","v3");
						new_input_register("iTexture2","v4");

					}
				
				}
			}
			else
			{

				if (flags&VPMORPH)
				{
                    strcpy(c.str,"dcl_position0 v0");vp.Add(c);
                    strcpy(c.str,"dcl_position1 v1");vp.Add(c);
                    strcpy(c.str,"dcl_normal0 v3");vp.Add(c);
                    strcpy(c.str,"dcl_normal1 v4");vp.Add(c);
                    strcpy(c.str,"dcl_texcoord v7");vp.Add(c);
				}
				else
				{
					if (flags&XYZ) { strcpy(c.str,"dcl_position v0");vp.Add(c); }
					if (flags&BLEND) {
                        strcpy(c.str,"dcl_blendweight v1");vp.Add(c);
                        strcpy(c.str,"dcl_blendindices v2");vp.Add(c);
					}
					if (flags&NORMAL) { strcpy(c.str,"dcl_normal v3");vp.Add(c); }
					if (flags&DIFFUSE) { strcpy(c.str,"dcl_color0 v5");vp.Add(c); }
					if (flags&SPECULAR) { strcpy(c.str,"dcl_color1 v6");vp.Add(c); }
					if (flags&_TEX0) { strcpy(c.str,"dcl_texcoord0 v7");vp.Add(c); }
					if (flags&_TEX1) { strcpy(c.str,"dcl_texcoord1 v8");vp.Add(c); }
					if (flags&_TEX2) { strcpy(c.str,"dcl_texcoord2 v9");vp.Add(c); }
				}
			}
		}
	}
	else
	if (metal==1)
	{
		int n;
		
		c.str[0]='\0'; vp.Add(c);

		int NB=96;
		if (shadermodel) NB=MAX_ENTRIES;

        strcpy(c.str,"#include <metal_stdlib>");vpcst.Add(c);
        strcpy(c.str,"#include <simd/simd.h>");vpcst.Add(c);

        strcpy(c.str,"using namespace metal;");vpcst.Add(c);
        strcpy(c.str,"");vpcst.Add(c);

        strcpy(c.str,"// VERTEXBUFFER FORMAT : ");

		if (flags&VPMORPH) _sprintf(c.str,"%s MORPH",c.str);
		else
		{
			if (flags&XYZ) _sprintf(c.str,"%s XYZ",c.str);
			if (flags&BLEND) _sprintf(c.str,"%s BLEND",c.str);
			if (flags&NORMAL) _sprintf(c.str,"%s NORMAL",c.str);
			if (flags&DIFFUSE) _sprintf(c.str,"%s DIFFUSE",c.str);
			if (flags&SPECULAR) _sprintf(c.str,"%s SPECULAR",c.str);

			if (flags&_TEX0) _sprintf(c.str,"%s TEX0",c.str);
			if (flags&_TEX1) _sprintf(c.str,"%s TEX1",c.str);
			if (flags&_TEX2) _sprintf(c.str,"%s TEX2",c.str);
			if (flags&_TEX3) _sprintf(c.str,"%s TEX3",c.str);
		}
		vpcst.Add(c);

        strcpy(c.str,"");vpcst.Add(c);

        strcpy(c.str,"struct constants_t");vpcst.Add(c);
        strcpy(c.str,"{");vpcst.Add(c);

		for (n=0;n<NB;n++)
		{
			if (constants[n])
			{
				if (translate_constants[n])
				{
				}
				else
				{
					if (LENGTH_VAR[l_constants[n]&0xffff]==1)
					{
						int nb=l_constants[n]>>16;
						if (nb==0) _sprintf(c.str,"simd::float4 %s;",constants[n]);
						else _sprintf(c.str,"simd::float4 %s[%d];",constants[n],nb);
					}

					if (LENGTH_VAR[l_constants[n]&0xffff]==4)
					{
						int nb=l_constants[n]>>16;
						if (nb==0) _sprintf(c.str,"simd::float4x4 %s;",constants[n]);
						else _sprintf(c.str,"simd::float4x4 %s[%d];",constants[n],nb);
					}
					vpcst.Add(c);
				}
			}
		}

        strcpy(c.str,"};");vpcst.Add(c);

		c.str[0]='\0'; vpcst.Add(c);

		if ((npsvectors>0)||(npsmatrices>0))
		{
            strcpy(c.str,"struct constants_ps");vpcst.Add(c);
            strcpy(c.str,"{");vpcst.Add(c);

			for (n=0;n<npsvectors;n++)
			{
				_sprintf(c.str,"simd::float4 %s;",psvectors[n]);vpcst.Add(c);
			}

			for (n=0;n<npsmatrices;n++)
			{
				_sprintf(c.str,"simd::float4x4 %s;",psmatrices[n]);vpcst.Add(c);
			}

            strcpy(c.str,"};");vpcst.Add(c);

			c.str[0]='\0'; vpcst.Add(c);
		}

		for (n=0;n<16;n++)
		{
			if (texture[n])
			{
				//_sprintf(c.str,"texture2d<uchar> %s [[texture(%d)]];",texture[n],n);
				//vpcst.Add(c);
			}
		}

		c.str[0]='\0'; vpcst.Add(c);

		for (n=0;n<NB;n++)
		{
			if (constants[n])
			{
				if (translate_constants[n])
				{
					int tt=1;
					if ((strcmp(constants[n],"trigo_mod")==0)&&(trigocst[pass][0]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst")==0)&&(trigocst[pass][1]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst2")==0)&&(trigocst[pass][2]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst3")==0)&&(trigocst[pass][3]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst4")==0)&&(trigocst[pass][4]==0)) tt=0;

					if (tt)
					{
						_sprintf(c.str,"constant float4 %s = { %s };",constants[n],translate_constants[n]);
						vpcst.Add(c);
					}
				}
			}
		}

		if (pass==0)
		{
			if (flags&VPMORPH)
			{
				//TODO
				if (flags&BLEND)
				{
					strcpy(c.str,"typedef struct");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v0;");vpinput.Add(c);
					strcpy(c.str,"packed_float4 v2;");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v3;");vpinput.Add(c);
					strcpy(c.str,"packed_float2 v7;");vpinput.Add(c);
					strcpy(c.str,"} VS_INPUT1;");vpinput.Add(c);

					strcpy(c.str,"");vpinput.Add(c);

					strcpy(c.str,"typedef struct");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v1;");vpinput.Add(c);
					strcpy(c.str,"packed_float4 v5;");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v4;");vpinput.Add(c);
					strcpy(c.str,"packed_float2 v8;");vpinput.Add(c);
					strcpy(c.str,"} VS_INPUT2;");vpinput.Add(c);
				}
				else
				{
					strcpy(c.str,"typedef struct");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v0;");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v3;");vpinput.Add(c);
					strcpy(c.str,"packed_float2 v7;");vpinput.Add(c);
					strcpy(c.str,"} VS_INPUT1;");vpinput.Add(c);

					strcpy(c.str,"");vpinput.Add(c);

					strcpy(c.str,"typedef struct");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v1;");vpinput.Add(c);
					strcpy(c.str,"packed_float3 v4;");vpinput.Add(c);
					strcpy(c.str,"packed_float2 v8;");vpinput.Add(c);
					strcpy(c.str,"} VS_INPUT2;");vpinput.Add(c);
				}
			}
			else
			{
                strcpy(c.str,"typedef struct");vpinput.Add(c);
                strcpy(c.str,"{");vpinput.Add(c);
				// vertex format
				if (flags&XYZ) { strcpy(c.str,"packed_float3 v0;");vpinput.Add(c); }  // METAL
				if (flags&BLEND)
				{
                    strcpy(c.str,"packed_float4 v1;");vpinput.Add(c);
				}
				if (flags&NORMAL) { strcpy(c.str,"packed_float3 v3;");vpinput.Add(c); }
				if (flags&DIFFUSE) { strcpy(c.str,"packed_float4 v5;");vpinput.Add(c); }
				if (flags&SPECULAR) { strcpy(c.str,"packed_float4 v6;");vpinput.Add(c); }

				if (flags&_TEX0) { strcpy(c.str,"packed_float2 v7;");vpinput.Add(c); }
				if (flags&_TEX1) { strcpy(c.str,"packed_float2 v8;");vpinput.Add(c); }
				if (flags&_TEX2) { strcpy(c.str,"packed_float2 v9;");vpinput.Add(c); }
				if (flags&_TEX3) { strcpy(c.str,"packed_float2 v10;");vpinput.Add(c); }

                strcpy(c.str,"} VS_INPUT;");vpinput.Add(c);
			}
		}

		last_iregister=0;

		if (flags&VPMORPH)
		{
			new_input_register("iPos","float4(float3(indexed.v0),1.0)");
			new_input_register("iPos2","float4(float3(indexed2.v1),1.0)");
			new_input_register("iNorm","float3(indexed.v3)");
			new_input_register("iNorm2","float3(indexed2.v4)");
			new_input_register("iTexture0","float2(indexed.v7)");
			if (flags&BLEND)
			{
				if (shadermodel4)
				{
					new_input_register("iWeights","Weights");
					new_input_register("iWIndices","WInd"); //ARRAY
				}
				else
				{
					new_input_register("iWeights","indexed.v2");
					new_input_register("iWIndices","WInd"); //ARRAY
				}
			}
		}
		else
		{
			new_input_register("iPos","float4(float3(indexed.v0),1.0)");
			int v=1;

			if (flags&BLEND) 
			{
				if (shadermodel4)
				{
					new_input_register("iWeights","Weights");
					new_input_register("iWIndices","WInd"); //ARRAY
					v=3;
				}
				else
				{
					new_input_register("iWeights","indexed.v1");
					new_input_register("iWIndices","WInd"); //ARRAY
					v=3;
				}
			}
					
			if (v==3)
			{
				new_input_register("iNorm","float3(indexed.v3)");
				new_input_register("iDiffuse","float4(indexed.v5)");
				new_input_register("iSpecular","float4(indexed.v6)");

				new_input_register("iTexture0","float2(indexed.v7)");
				new_input_register("iTexture1","float2(indexed.v8)");
				new_input_register("iTexture2","float2(indexed.v9)");
				new_input_register("iTexture3","float2(indexed.v10)");
			}
			else
			{
				new_input_register("iNorm","float3(indexed.v3)");
				new_input_register("iDiffuse","float4(indexed.v5)");
				new_input_register("iSpecular","float4(indexed.v6)");

				new_input_register("iTexture0","float2(indexed.v7)");
				new_input_register("iTexture1","float2(indexed.v8)");
				new_input_register("iTexture2","float2(indexed.v9)");
				new_input_register("iTexture3","float2(indexed.v10)");
			}				
		}
	}
	else
	if ((flags&DIRECT3D)&&(shadermodel3))
	{
		c.str[0]='\0';
		vp.Add(c);
		int NB=96;
		if (shadermodel) NB=MAX_ENTRIES;

		for (int n=0;n<16;n++)
		{
			if (texture[n])
			{
				if (shadermodel4) _sprintf(c.str,"Texture2D %s;",texture[n]);
				else _sprintf(c.str,"Texture %s;",texture[n]);
				vpcst.Add(c);
			}
		}

		c.str[0]='\0';
		vp.Add(c);

		for (int n=0;n<16;n++)
		{
			if (texturevs[n])
			{
				if (shadermodel4) _sprintf(c.str,"Texture2D %s;",texturevs[n]);
				else _sprintf(c.str,"Texture %s;",texturevs[n]);
				vpcst.Add(c);
			}
		}

		c.str[0]='\0';
		vp.Add(c);

		for (int n=0;n<NB;n++)
		{
			if (constants[n])
			{
				int tt=1;

				if (translate_constants[n])
				{
					if ((strcmp(constants[n],"trigo_mod")==0)&&(trigocst[pass][0]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst")==0)&&(trigocst[pass][1]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst2")==0)&&(trigocst[pass][2]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst3")==0)&&(trigocst[pass][3]==0)) tt=0;
					if ((strcmp(constants[n],"trigo_cst4")==0)&&(trigocst[pass][4]==0)) tt=0;
					
					_sprintf(c.str,"float4 %s = { %s };",constants[n],translate_constants[n]);
				}
				else
				{
					if (LENGTH_VAR[l_constants[n]&0xffff]==1)
					{
						int nb=l_constants[n]>>16;
						if (nb==0) _sprintf(c.str,"float4 %s;",constants[n]);
						else _sprintf(c.str,"float4 %s[%d];",constants[n],nb);
					}

					if (LENGTH_VAR[l_constants[n]&0xffff]==4)
					{
						int nb=l_constants[n]>>16;
						if (nb==0) _sprintf(c.str,"float4x4 %s;",constants[n]);
						else _sprintf(c.str,"float4x4 %s[%d];",constants[n],nb);
					}
				}

				if (tt) vpcst.Add(c);
			}
		}

		if (pass==0)
		{
			if (flags&VPMORPH)
			{
				if (weightsandindices)
				{
					strcpy(c.str,"struct VS_INPUT");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"float4 v0 : POSITION0;");vpinput.Add(c);
					strcpy(c.str,"float4 v1 : BLENDWEIGHT0;");vpinput.Add(c);
					strcpy(c.str,"float3 v3 : NORMAL0;");vpinput.Add(c);
					strcpy(c.str,"float2 v7 : TEXCOORD0;");vpinput.Add(c);

					strcpy(c.str,"float4 v2 : POSITION1;");vpinput.Add(c);
					strcpy(c.str,"float4 v4 : BLENDWEIGHT1;");vpinput.Add(c);
					strcpy(c.str,"float3 v5 : NORMAL1;");vpinput.Add(c);
					strcpy(c.str,"float2 v8 : TEXCOORD1;");vpinput.Add(c);
					strcpy(c.str,"};");vpinput.Add(c);
				}
				else
				{
					strcpy(c.str,"struct VS_INPUT");vpinput.Add(c);
					strcpy(c.str,"{");vpinput.Add(c);
					strcpy(c.str,"float4 v0 : POSITION0;");vpinput.Add(c);
					strcpy(c.str,"float3 v3 : NORMAL0;");vpinput.Add(c);
					strcpy(c.str,"float2 v7 : TEXCOORD0;");vpinput.Add(c);
					strcpy(c.str,"float4 v1 : POSITION1;");vpinput.Add(c);
					strcpy(c.str,"float3 v4 : NORMAL1;");vpinput.Add(c);
					strcpy(c.str,"float2 v8 : TEXCOORD1;");vpinput.Add(c);
					strcpy(c.str,"};");vpinput.Add(c);
				}
			}
			else
			{
                strcpy(c.str,"struct VS_INPUT");vpinput.Add(c);
                strcpy(c.str,"{");vpinput.Add(c);
				// vertex format
				if (flags&XYZ) { strcpy(c.str,"float4 v0 : POSITION;");vpinput.Add(c); }
				if (weightsandindices)
				{
                    strcpy(c.str,"float4 v1 : BLENDWEIGHT;");vpinput.Add(c);
					if (!shadermodel4) { strcpy(c.str,"DWORD v2 : BLENDINDICES;");vpinput.Add(c); }
				}
				if (flags&NORMAL) { strcpy(c.str,"float3 v3 : NORMAL;");vpinput.Add(c); }
				if (flags&DIFFUSE) { strcpy(c.str,"float4 v5 : COLOR0;");vpinput.Add(c); }
				if (flags&SPECULAR) { strcpy(c.str,"float4 v6 : COLOR1;");vpinput.Add(c); }

				if (flags&_TEX0) { strcpy(c.str,"float2 v7 : TEXCOORD0;");vpinput.Add(c); }
				if (flags&_TEX1) { strcpy(c.str,"float2 v8 : TEXCOORD1;");vpinput.Add(c); }
				if (flags&_TEX2) { strcpy(c.str,"float2 v9 : TEXCOORD2;");vpinput.Add(c); }
				if (flags&_TEX3) { strcpy(c.str,"float2 v10 : TEXCOORD3;");vpinput.Add(c); }

                strcpy(c.str,"};");vpinput.Add(c);
			}
		}

		last_iregister=0;

		if (flags&VPMORPH)
		{
			if (weightsandindices)
			{
				new_input_register("iWeights","Weights");
				new_input_register("iWIndices","WInd"); //ARRAY

				new_input_register("iPos","i.v0");
				new_input_register("iPos2","i.v2");
				new_input_register("iNorm","i.v3");
				new_input_register("iNorm2","i.v5");
				new_input_register("iTexture0","i.v7");
			}
			else
			{
				new_input_register("iPos","i.v0");
				new_input_register("iPos2","i.v1");
				new_input_register("iNorm","i.v3");
				new_input_register("iNorm2","i.v4");
				new_input_register("iTexture0","i.v7");
			}
		}
		else
		{
			new_input_register("iPos","i.v0");
			int v=1;

			if (flags&BLEND) 
			{
				if (shadermodel4)
				{
					new_input_register("iWeights","Weights");
					new_input_register("iWIndices","WInd"); //ARRAY
					v=3;
				}
				else
				{
					new_input_register("iWeights","i.v1");
					new_input_register("iWIndices","WInd"); //ARRAY
					v=3;
				}
			}
					
			if (v==3)
			{
				new_input_register("iNorm","i.v3");
				new_input_register("iDiffuse","i.v5");
				new_input_register("iSpecular","i.v6");

				new_input_register("iTexture0","i.v7");
				new_input_register("iTexture1","i.v8");
				new_input_register("iTexture2","i.v9");
				new_input_register("iTexture3","i.v10");
			}
			else
			{
				new_input_register("iNorm","i.v3");
				new_input_register("iDiffuse","i.v5");
				new_input_register("iSpecular","i.v6");

				new_input_register("iTexture0","i.v7");
				new_input_register("iTexture1","i.v8");
				new_input_register("iTexture2","i.v9");
				new_input_register("iTexture3","i.v10");
			}				
		}
	}
	else
	{
		if (!shadermodel)
		{
			if (flags&VPMORPH)
			{
                strcpy(c.str,"ATTRIB iPos = vertex.attrib[0]");glvp[pass].Add(c);
                strcpy(c.str,"ATTRIB iPos2 = vertex.attrib[1]");glvp[pass].Add(c);
                strcpy(c.str,"ATTRIB iNorm = vertex.attrib[2]");glvp[pass].Add(c);
                strcpy(c.str,"ATTRIB iNorm2 = vertex.attrib[3]");glvp[pass].Add(c);
                strcpy(c.str,"ATTRIB iTexture0 = vertex.attrib[8]");glvp[pass].Add(c);
			}
			else
			{
				if (flags&XYZ) { strcpy(c.str,"ATTRIB iPos = vertex.attrib[0]");glvp[pass].Add(c); }
				if (flags&BLEND)
				{
                    strcpy(c.str,"ATTRIB iWeights = vertex.attrib[1]");glvp[pass].Add(c);
                    strcpy(c.str,"ATTRIB iWIndices = vertex.attrib[6]");glvp[pass].Add(c);
				}
				if (flags&NORMAL) { strcpy(c.str,"ATTRIB iNorm = vertex.attrib[2]");glvp[pass].Add(c); }
				if (flags&DIFFUSE) { strcpy(c.str,"ATTRIB iDiffuse = vertex.attrib[3]");glvp[pass].Add(c); }
				if (flags&SPECULAR) { strcpy(c.str,"ATTRIB iSpecular = vertex.attrib[4]");glvp[pass].Add(c); }
				if (flags&_TEX0) { strcpy(c.str,"ATTRIB iTexture0 = vertex.attrib[8]");glvp[pass].Add(c); }
				if (flags&_TEX1) { strcpy(c.str,"ATTRIB iTexture1 = vertex.attrib[9]");glvp[pass].Add(c); }
				if (flags&_TEX2) { strcpy(c.str,"ATTRIB iTexture2 = vertex.attrib[10]");glvp[pass].Add(c); }
			}

			c.str[0]='\0';
			glvp[pass].Add(c);

			if (output&XYZ) { strcpy(c.str,"OUTPUT oPos = result.position");glvp[pass].Add(c); }
			if ((output&DIFFUSE)&&(output&SPECULAR)) { strcpy(c.str,"OUTPUT oDiffuse = result.color.primary");glvp[pass].Add(c); }
			if ((output&DIFFUSE)&&(!(output&SPECULAR))) { strcpy(c.str,"OUTPUT oDiffuse = result.color");glvp[pass].Add(c); }
			if (output&SPECULAR) { strcpy(c.str,"OUTPUT oSpecular = result.color.secondary");glvp[pass].Add(c); }
			if (output&_TEX0) { strcpy(c.str,"OUTPUT oTexture0 = result.texcoord[0]");glvp[pass].Add(c); }
			if (output&_TEX1) { strcpy(c.str,"OUTPUT oTexture1 = result.texcoord[1]");glvp[pass].Add(c); }
			if (output&_TEX2) { strcpy(c.str,"OUTPUT oTexture2 = result.texcoord[2]");glvp[pass].Add(c); }
			if (output&_TEX3) { strcpy(c.str,"OUTPUT oTexture3 = result.texcoord[3]");glvp[pass].Add(c); }
			if (output&_TEX4) { strcpy(c.str,"OUTPUT oTexture4 = result.texcoord[4]");glvp[pass].Add(c); }

			c.str[0]='\0';
			glvp[pass].Add(c);
            strcpy(c.str,"ADDRESS A0");glvp[pass].Add(c);
			c.str[0]='\0';
			glvp[pass].Add(c);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::header_open_psh(int pass,unsigned int flags,unsigned int output)
{
	int n;

	max_temporary_register=0;

	if (metal)
	{
		clear_registers();
		//PSH_OUTPUT=var("Out",1);
		tagregsave[0]=1;
		last_temp_register=last_register;
	
        new_output_register("Out","o.Out");
        
		if (modifyZ) new_output_register("Depth","o.Depth");
			
		phase=1;
		if (pass==0)
		for (n=0;n<MAX_ENTRIES;n++)
		{
			if (ps_constants[n])
			{
				if (translate_ps_constants[n])
				{
					if (ps_constants_type[n]==1)
                    {
                        _sprintf(c.str,"constant float4  %s = { %s };",ps_constants[n],translate_ps_constants[n]);
                        vpoutput.Add(c);
                    }
					else
                    {
                     
                        int mm=0;
                        _sprintf(c.str,"constant float4  %sA = { %3.4f,%3.4f,%3.4f,%3.4f };",ps_constants[n],float_translate_ps_constants[n][mm+0],float_translate_ps_constants[n][mm+1],float_translate_ps_constants[n][mm+2],float_translate_ps_constants[n][mm+3]);
                        vpoutput.Add(c);
                        mm=4;
                        _sprintf(c.str,"constant float4  %sB = { %3.4f,%3.4f,%3.4f,%3.4f };",ps_constants[n],float_translate_ps_constants[n][mm+0],float_translate_ps_constants[n][mm+1],float_translate_ps_constants[n][mm+2],float_translate_ps_constants[n][mm+3]);
                        vpoutput.Add(c);
                        mm=8;
                        _sprintf(c.str,"constant float4  %sC = { %3.4f,%3.4f,%3.4f,%3.4f };",ps_constants[n],float_translate_ps_constants[n][mm+0],float_translate_ps_constants[n][mm+1],float_translate_ps_constants[n][mm+2],float_translate_ps_constants[n][mm+3]);
                        vpoutput.Add(c);
                        mm=12;
                        _sprintf(c.str,"constant float4  %sD = { %3.4f,%3.4f,%3.4f,%3.4f };",ps_constants[n],float_translate_ps_constants[n][mm+0],float_translate_ps_constants[n][mm+1],float_translate_ps_constants[n][mm+2],float_translate_ps_constants[n][mm+3]);
                        vpoutput.Add(c);
                        
                        //_sprintf(c.str,"constant float4x4  %s = float4x4( %sA,%sB,%sC,%sD );",ps_constants[n],ps_constants[n],ps_constants[n],ps_constants[n],ps_constants[n]);
                        
                    }
				}
			}
		}

		c.str[0]='\0';
		vpoutput.Add(c);

		if (pass==0)
		{
            strcpy(c.str,"typedef struct");vpoutput.Add(c);
            strcpy(c.str,"{");vpoutput.Add(c);

			//_sprintf(c.str,"float4 position [[position]];"); vpoutput.Add(c);
			
			if (modifyZ)
			{ 
                strcpy(c.str,"float4 Out[[color(0)]];"); vpoutput.Add(c);
                strcpy(c.str,"float Depth[[depth(less)]];"); vpoutput.Add(c); // any greater less
			}
			else
			{
                strcpy(c.str,"float4 Out[[color(0)]];"); vpoutput.Add(c);
			}

			/*
			if (shadermodel4) { if (modifyZ) { _sprintf(c.str,"float Depth : SV_Depth;");vpoutput.Add(c); } 	}
			else { if (modifyZ) { _sprintf(c.str,"float Depth : DEPTH;");vpoutput.Add(c); } }
			/**/
            strcpy(c.str,"} PS_OUTPUT;");vpoutput.Add(c);
			c.str[0]='\0';
			vpoutput.Add(c);
		}

        strcpy(c.str,"typedef struct");vpoutput.Add(c);
        strcpy(c.str,"{");vpoutput.Add(c);

        strcpy(c.str,"float4 p0 [[position]];");vpoutput.Add(c);
        
        if (pointcloud) { strcpy(c.str,"float PSize[[point_size]];");vpoutput.Add(c); }
			
		if (output&DIFFUSE) { strcpy(c.str,"float4 v0;");vpoutput.Add(c); }
		if (output&SPECULAR) { strcpy(c.str,"float4 v1;");vpoutput.Add(c); }
        /*
         if (output&_TEX0) { _sprintf(c.str,"float4 t0 [[texturecoord0]];");vpoutput.Add(c); }
         if (output&_TEX1) { _sprintf(c.str,"float4 t1 [[texturecoord1]];");vpoutput.Add(c); }
         if (output&_TEX2) { _sprintf(c.str,"float4 t2 [[texturecoord2]];");vpoutput.Add(c); }
         if (output&_TEX3) { _sprintf(c.str,"float4 t3 [[texturecoord3]];");vpoutput.Add(c); }
         if (output&_TEX4) { _sprintf(c.str,"float4 t4 [[texturecoord4]];");vpoutput.Add(c); }
         if (output&_TEX5) { _sprintf(c.str,"float4 t5 [[texturecoord5]];");vpoutput.Add(c); }
         if (output&_TEX6) { _sprintf(c.str,"float4 t6 [[texturecoord6]];");vpoutput.Add(c); }
         if (output&_TEX7) { _sprintf(c.str,"float4 t7 [[texturecoord7]];");vpoutput.Add(c); }
         /**/
        
        if (output&_TEX0) { strcpy(c.str,"float4 t0;");vpoutput.Add(c); }  // [[ sample_no_perspective ]]
        if (output&_TEX1) { strcpy(c.str,"float4 t1;");vpoutput.Add(c); }
        if (output&_TEX2) { strcpy(c.str,"float4 t2;");vpoutput.Add(c); }
        if (output&_TEX3) { strcpy(c.str,"float4 t3;");vpoutput.Add(c); }
        if (output&_TEX4) { strcpy(c.str,"float4 t4;");vpoutput.Add(c); }
        if (output&_TEX5) { strcpy(c.str,"float4 t5;");vpoutput.Add(c); }
        if (output&_TEX6) { strcpy(c.str,"float4 t6;");vpoutput.Add(c); }
        if (output&_TEX7) { strcpy(c.str,"float4 t7;");vpoutput.Add(c); }
        
        
        
        _sprintf(c.str,"} VS_OUTPUT%d;",pass);vpoutput.Add(c);
		c.str[0]='\0';
		vpoutput.Add(c);

        /*
        _sprintf(c.str,"constexpr sampler s0(coord::normalized,address::repeat,min_filter::linear,mag_filter::linear,mip_filter::linear);");
        vpoutput.Add(c);
        
        _sprintf(c.str,"constexpr sampler s1(coord::normalized,address::repeat,min_filter::nearest,mag_filter::nearest,mip_filter::nearest);");
        vpoutput.Add(c);
        /**/
        //_sprintf(c.str,"constexpr sampler s0;");
        //vpoutput.Add(c);
        
        
        c.str[0]='\0';
		vpoutput.Add(c);
	}
	else
	if (flags&DIRECT3D)
	{
		if (shadermodel3)
		{			
			clear_registers();
			PSH_OUTPUT=var("Out",1);
			tagregsave[0]=1;
			last_temp_register=last_register;
	
			if (modifyZ) new_output_register("Depth","o.Depth");
			
			phase=1;
			if (pass==0)
			for (n=0;n<MAX_ENTRIES;n++)
			{
				if (ps_constants[n])
				{
					if (translate_ps_constants[n])
					{
						if (ps_constants_type[n]==1) _sprintf(c.str,"float4  %s = { %s };",ps_constants[n],translate_ps_constants[n]);
						else _sprintf(c.str,"float4x4  %s = { %s };",ps_constants[n],translate_ps_constants[n]);
						vpoutput.Add(c);
					}
				}
			}

			c.str[0]='\0';
			vpoutput.Add(c);

			for (n=0;n<npsvectors;n++)
			{
				_sprintf(c.str,"float4 %s;",psvectors[n]);
				vpoutput.Add(c);
			}

			for (n=0;n<npsmatrices;n++)
			{
				_sprintf(c.str,"float4x4 %s;",psmatrices[n]);
				vpoutput.Add(c);
			}

			c.str[0]='\0';
			vpoutput.Add(c);

			if (pass==0)
			{
                strcpy(c.str,"struct PS_OUTPUT");vpoutput.Add(c);
                strcpy(c.str,"{");vpoutput.Add(c);

				if (shadermodel4) { _sprintf(c.str,"float4 Out : SV_Target;");vpoutput.Add(c); }
				else { _sprintf(c.str,"float4 Out : COLOR0;");vpoutput.Add(c); }

				if (shadermodel4) { if (modifyZ) { _sprintf(c.str,"float Depth : SV_Depth;");vpoutput.Add(c); } 	}
				else { if (modifyZ) { _sprintf(c.str,"float Depth : DEPTH;");vpoutput.Add(c); } }
			
                strcpy(c.str,"};");vpoutput.Add(c);
				c.str[0]='\0';
				vpoutput.Add(c);
			}

            _sprintf(c.str,"struct VS_OUTPUT%d",pass);vpoutput.Add(c);
            strcpy(c.str,"{");vpoutput.Add(c);

			if (shadermodel4) { strcpy(c.str,"float4 p0 : SV_POSITION;");vpoutput.Add(c); }
			else { strcpy(c.str,"float4 p0 : POSITION;");vpoutput.Add(c); }
			
			if (output&DIFFUSE) { strcpy(c.str,"float4 v0 : COLOR0;");vpoutput.Add(c); }
			if (output&SPECULAR) { strcpy(c.str,"float4 v1 : COLOR1;");vpoutput.Add(c); }
			if (output&_TEX0) { strcpy(c.str,"float4 t0 : TEXCOORD0;");vpoutput.Add(c); }
			if (output&_TEX1) { strcpy(c.str,"float4 t1 : TEXCOORD1;");vpoutput.Add(c); }
			if (output&_TEX2) { strcpy(c.str,"float4 t2 : TEXCOORD2;");vpoutput.Add(c); }
			if (output&_TEX3) { strcpy(c.str,"float4 t3 : TEXCOORD3;");vpoutput.Add(c); }
			if (output&_TEX4) { strcpy(c.str,"float4 t4 : TEXCOORD4;");vpoutput.Add(c); }
			if (output&_TEX5) { strcpy(c.str,"float4 t5 : TEXCOORD5;");vpoutput.Add(c); }
			if (output&_TEX6) { strcpy(c.str,"float4 t6 : TEXCOORD6;");vpoutput.Add(c); }
			if (output&_TEX7) { strcpy(c.str,"float4 t7 : TEXCOORD7;");vpoutput.Add(c); }

            strcpy(c.str,"};");vpoutput.Add(c);
			c.str[0]='\0';
			vpoutput.Add(c);
			
			if (pass==0)
			{
				if (shadermodel4)
				{
                    strcpy(c.str,"SamplerState smp");vpoutput.Add(c);
                    strcpy(c.str,"{");vpoutput.Add(c);
                    strcpy(c.str,"Filter = MIN_MAG_MIP_LINEAR;");vpoutput.Add(c);
                    strcpy(c.str,"AddressU = Wrap;");vpoutput.Add(c);
                    strcpy(c.str,"AddressV = Wrap;");vpoutput.Add(c);
                    strcpy(c.str,"};");vpoutput.Add(c);

					c.str[0]='\0';
					vpoutput.Add(c);

                    strcpy(c.str,"SamplerState smpvs");vpoutput.Add(c);
                    strcpy(c.str,"{");vpoutput.Add(c);
                    strcpy(c.str,"Filter = MIN_MAG_LINEAR_MIP_POINT;");vpoutput.Add(c);
                    strcpy(c.str,"AddressU = Wrap;");vpoutput.Add(c);
                    strcpy(c.str,"AddressV = Wrap;");vpoutput.Add(c);
                    strcpy(c.str,"};");vpoutput.Add(c);
				}
				else
				{
					for (int i=0;i<ntexture;i++)
					{
						_sprintf(c.str,"sampler s%d = ",i);vpoutput.Add(c);
                        strcpy(c.str,"sampler_state");vpoutput.Add(c);
                        strcpy(c.str,"{");vpoutput.Add(c);
						_sprintf(c.str,"Texture = <%s>;",texture[i]);vpoutput.Add(c);
                        strcpy(c.str,"MipFilter = LINEAR;");vpoutput.Add(c);
                        strcpy(c.str,"MinFilter = LINEAR;");vpoutput.Add(c);
                        strcpy(c.str,"MagFilter = LINEAR;");vpoutput.Add(c);
                        strcpy(c.str,"};");vpoutput.Add(c);
					}

					c.str[0]='\0';
					vpoutput.Add(c);

					for (int i=0;i<ntexturevs;i++)
					{
						_sprintf(c.str,"sampler svs%d = ",i);vpoutput.Add(c);
                        strcpy(c.str,"sampler_state");vpoutput.Add(c);
                        strcpy(c.str,"{");vpoutput.Add(c);
						_sprintf(c.str,"Texture = <%s>;",texturevs[i]);vpoutput.Add(c);
                        strcpy(c.str,"MipFilter = POINT;");vpoutput.Add(c);
                        strcpy(c.str,"MinFilter = LINEAR;");vpoutput.Add(c);
                        strcpy(c.str,"MagFilter = LINEAR;");vpoutput.Add(c);
                        strcpy(c.str,"};");vpoutput.Add(c);
					}
				}
				c.str[0]='\0';
				vpoutput.Add(c);
#ifdef API3D_DIRECT3D9
				if (metal==0)
				{
					if (FSR==1)
					{
						char *sfsr=shader_functions_FSR_header;
						int ps=0;
						char *pstmp=str_parse_rln2(sfsr,&ps);
						while (pstmp)
						{
							if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
							pstmp=str_parse_rln2(sfsr,&ps);
						}
                                                    
						sfsr=shader_functions_FSR_EASU;
						ps=0;
						pstmp=str_parse_rln2(sfsr,&ps);
						while (pstmp)
						{
							if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
							pstmp=str_parse_rln2(sfsr,&ps);
						}
					}

					if (FSR==2)
					{
						char *sfsr=shader_functions_FSR_header;
						int ps=0;
						char *pstmp=str_parse_rln2(sfsr,&ps);
						while (pstmp)
						{
							if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
							pstmp=str_parse_rln2(sfsr,&ps);
						}
                                                    
						sfsr=shader_functions_FSR_RCAS;
						ps=0;
						pstmp=str_parse_rln2(sfsr,&ps);
						while (pstmp)
						{
							if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
							pstmp=str_parse_rln2(sfsr,&ps);
						}
					}
					/*
					if ((FSRSample0)||(FSRSample1))
					{
						char *sfsr=shader_functions_FSR_header_01;
						int ps=0;
						char *pstmp=str_parse_rln2(sfsr,&ps);
						while (pstmp)
						{
							if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
							pstmp=str_parse_rln2(sfsr,&ps);
						}

						if ((FSRSample0)&&(RenderState[0][0].Texture[0]))
						{
							sfsr=shader_functions_FSR_RCAS_0;
							ps=0;
							pstmp=str_parse_rln2(sfsr,&ps);
							while (pstmp)
							{
								if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
								pstmp=str_parse_rln2(sfsr,&ps);
							}
						}

						if ((FSRSample1)&&(RenderState[0][0].Texture[1]))
						{
							sfsr=shader_functions_FSR_RCAS_1;
							ps=0;
							pstmp=str_parse_rln2(sfsr,&ps);
							while (pstmp)
							{
								if (strlen(pstmp)>0) { strcpy(c.str,pstmp); vpoutput.Add(c); }
								pstmp=str_parse_rln2(sfsr,&ps);
							}
						}
					}
					/**/
				}
#endif
			}
		}
		else
		if (shadermodel)
		{
            strcpy(c.str,"PixelShader =");
			vp.Add(c);
            strcpy(c.str,"asm");
			vp.Add(c);
            strcpy(c.str,"{");
			vp.Add(c);
            strcpy(c.str,"ps.2.x");
			vp.Add(c);
			c.str[0]='\0';
			vp.Add(c);
			
			clear_registers();
			PSH_OUTPUT=var("Out",1);
			last_temp_register=last_register;
	
			new_output_register("Depth","oDepth");
			
			phase=1;
			for (n=0;n<MAX_ENTRIES;n++)
			{
				if (ps_constants[n])
				{
					if (translate_ps_constants[n])
					{
						_sprintf(c.str,"def  c%d, %s",n,translate_ps_constants[n]);
						vp.Add(c);
					}
				}
			}

			c.str[0]='\0';
			vp.Add(c);

			if (output&DIFFUSE) { strcpy(c.str,"dcl v0"); vp.Add(c); }
			if (output&SPECULAR) { strcpy(c.str,"dcl v1"); vp.Add(c); }
			if (output&_TEX0) { strcpy(c.str,"dcl t0"); vp.Add(c); }
			if (output&_TEX1) { strcpy(c.str,"dcl t1"); vp.Add(c); }
			if (output&_TEX2) { strcpy(c.str,"dcl t2"); vp.Add(c); }
			if (output&_TEX3) { strcpy(c.str,"dcl t3"); vp.Add(c); }
			if (output&_TEX4) { strcpy(c.str,"dcl t4"); vp.Add(c); }
			if (output&_TEX5) { strcpy(c.str,"dcl t5"); vp.Add(c); }
			if (output&_TEX6) { strcpy(c.str,"dcl t6"); vp.Add(c); }
			if (output&_TEX7) { strcpy(c.str,"dcl t7"); vp.Add(c); }

			c.str[0]='\0';
			vp.Add(c);

			for (int i=0;i<16;i++)
			{
				if (RS.Texture[i])
				{
					_sprintf(c.str,"dcl_2d s%d",i);
					vp.Add(c);
				}
			}

			c.str[0]='\0';
			vp.Add(c);
			
		}
		else
		{
            strcpy(c.str,"PixelShader =");
			vp.Add(c);
            strcpy(c.str,"asm");
			vp.Add(c);
            strcpy(c.str,"{");
			vp.Add(c);
            strcpy(c.str,"ps.1.4");
			vp.Add(c);
			c.str[0]='\0';
			vp.Add(c);
			clear_registers();
			PSH_OUTPUT=var("Out",1);
			last_temp_register=last_register;

			phase=0;
			for (n=0;n<8;n++)
			{
				if (ps_constants[n])
				{
					if (translate_ps_constants[n])
					{
						_sprintf(c.str,"def  c%d, %s",n,translate_ps_constants[n]);
						vp.Add(c);
					}
				}
			}

			c.str[0]='\0';
			vp.Add(c);
		}
	}
	else
	{
		
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CVertexProgram::is_constant(char *name)
{
	int n;

    for (n=0;n<last_constant;n++)
	{
		if (constants[n])
			if (strcmp(name,constants[n])==0)
			{
				char *s=constants[n];
				actual_cst=n;
				actual_cst_size=LENGTH_VAR[l_constants[n]&0xffff];
                return s;
			}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char** CVertexProgram::is_input(char *name)
{
	int n;
	char** s=NULL;

	for (n=0;n<last_iregister;n++)
	{
		if (strcmp(name,iregisters[n][0])==0) s=iregisters[n];
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char** CVertexProgram::is_output(char *name)
{
	int n;
	char** s=NULL;

	if ((func)&&(shadermodel3))
	{
		for (n=0;n<last_o2register;n++)
		{
			if (strcmp(name,o2registers[n][0])==0) { s=o2registers[n]; globalvar=true; }
		}
	}
	else
	{
		for (n=0;n<last_oregister;n++)
		{
			if (strcmp(name,oregisters[n][0])==0) s=oregisters[n];
		}
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::new_register()
{
	int n,res;
	res=-1;
	for (n=0;n<32;n++)
	{
		if ((tagreg[n]==0)&&(tagtempreg[n]!=666)) { res=n; break; }
	}

	tagreg[res]=1;
	tagregsave[res]=1;
	return res;

	//return last_register;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::close_temp_register()
{
	int n;
	last_temp_register=last_register;
	for (n=0;n<32;n++) if (tagtempreg[n]==1) tagtempreg[n]=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::close_temp_register(int last)
{
    int n;
    last_temp_register=last;
    for (n=last;n<32;n++) if (tagtempreg[n]==1) tagtempreg[n]=0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::new_temp_register()
{
	int n,res;

	res=-1;
	for (n=0;n<32;n++)
	{
		if ((tagreg[n]==0)&&(tagtempreg[n]==0)) { res=n; break; }
	}

	tagtempreg[res]=1;
	tagregsave[res]=1;
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char suppressparameters_str[1024];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *suppressparameters(char *str)
{
	char *ss=suppressparameters_str;
	strcpy(ss,str);
	int p=str_char(ss,'(');
	if (p>=0)
	{
		ss[p+1]='.';
		ss[p+2]='.';
		ss[p+3]='.';
		ss[p+4]=')';
		ss[p+5]='\0';
	}
	return suppressparameters_str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::unused_variables()
{
	CVPVariable *v=Vars.GetFirst();
	while (v)
	{
		if ((isbasicdefined(Vars.GetId().str)<0)&&(!isfloat(Vars.GetId().str)))
		{
			if (v->use==0)
			{
				int tag=1;
				if ((str_match(v->linestr,"FSRSample0"))&&(FSRSample0)) tag=0;
				if ((str_match(v->linestr,"FSRSample1"))&&(FSRSample1)) tag=0;

				if ((strlen(UNUSEDRRORSTR)==0)&&(tag))
				{
					strcpy(UNUSEDRRORSTR,v->linestr);
					strcpy(ERRORSTRPARAM,Vars.GetId().str);
				}
				if (tag) UNUSEDERROR=true;
			}
		}
		v=Vars.GetNext();
	}

	if (levelrepglobal>0) REPERROR=true;
	levelrepglobal=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::clear_registers()
{
	int n;
	Vars.Free();
	last_register=0;
	for (n=0;n<32;n++) tagreg[n]=0;
	for (n=0;n<32;n++) tagtempreg[n]=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::set_register(int n,char *name)
{
	registers[n]=(char*) malloc(strlen(name)+1);
	_sprintf(registers[n],"%s",name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int multiple_argument_parse_virg(char *str,char **strings);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::parse_inner_func_position(char * &script)
{
	
	char * vertex =	"		%s = iWeights.xxxx*%s*4%s[iWIndices.x] + iWeights.yyyy*%s*4%s[iWIndices.y];\n"
					"		%s = iWeights.zzzz*%s*4%s[iWIndices.z] + iWeights.wwww*%s*4%s[iWIndices.w];\n"
					"		%s += %s;\n";

	char * normal =	"		%s = iWeights.xxxx*%s*3%s[iWIndices.x]  + iWeights.yyyy*%s*3%s[iWIndices.y];\n"
					"		%s = iWeights.zzzz*%s*3%s[iWIndices.z]  + iWeights.wwww*%s*3%s[iWIndices.w];\n"
					"		%s+ = %s;\n";

	char * vertex0=	"		%s = iWeights.xxxx*%s*4%s[iWIndices.x] + iWeights.yyyy*%s*4%s[iWIndices.y];\n";

	char * normal0=	"		%s = iWeights.xxxx*%s*3%s[iWIndices.x]  + iWeights.yyyy*%s*3%s[iWIndices.y];\n";

	char * space =	"		%s = tgt(vec4(0,-1,0,0),%s);\n"
					"		%s = normalise(%s);\n"
					"		%s = %s ^ %s;\n";


	char strtmp[65536];
	char *res=(char*)malloc(strlen(script)+32768);

	bool update=false;
	int pos=0;
	int p=0;
	char *rns[8];
	char *tmp=str_parse(script,&p);
	while (tmp)
	{
		if (str_match(tmp,"VectorTangentSpace"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==2)
			{
				int pz=str_char(tmp,'=');
				tmp[pz]=0;

				int pp=0;
				_sprintf(strtmp,space,rns[0],tmp,rns[0],rns[0],rns[1],tmp,rns[0]);

				char * parsed=str_get_scriptfns(strtmp,&pp);

				_sprintf(&res[pos],"%s",parsed);
				pos+=(int)strlen(parsed);
				update=true;
			}
		}
		else
		if ((str_match(tmp,"weighted_normal_calculate_partial"))||(str_match(tmp,"WeightedNormalCalculatePartial")))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==2)
			{
				int pz=str_char(tmp,'=');
				tmp[pz]=0;

				int pp=0;
				_sprintf(strtmp,normal0,tmp,rns[0],rns[1],rns[0],rns[1]);

				char * parsed=str_get_scriptfns(strtmp,&pp);

				_sprintf(&res[pos],"%s",parsed);
				pos+=(int)strlen(parsed);
				update=true;
			}
		}
		else
		if ((str_match(tmp,"weighted_calculate_partial"))||(str_match(tmp,"WeightedCalculatePartial")))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==2)
			{
				int pz=str_char(tmp,'=');
				tmp[pz]=0;

				int pp=0;
				_sprintf(strtmp,vertex0,tmp,rns[0],rns[1],rns[0],rns[1]);

				char * parsed=str_get_scriptfns(strtmp,&pp);

				_sprintf(&res[pos],"%s",parsed);
				pos+=(int)strlen(parsed);
				update=true;
			}
		}
		else
		if ((str_match(tmp,"weighted_normal_calculate"))||(str_match(tmp,"WeightedNormalCalculate")))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==2)
			{
				int pz=str_char(tmp,'=');
				tmp[pz]=0;

				int pp=0;
				_sprintf(strtmp,normal,tmp,rns[0],rns[1],rns[0],rns[1],"tmp",rns[0],rns[1],rns[0],rns[1],tmp,"tmp");

				char * parsed=str_get_scriptfns(strtmp,&pp);

				_sprintf(&res[pos],"%s",parsed);
				pos+=(int)strlen(parsed);
				update=true;
			}
		}
		else
		if ((str_match(tmp,"weighted_calculate"))||(str_match(tmp,"WeightedCalculate")))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==2)
			{
				int pz=str_char(tmp,'=');
				tmp[pz]=0;

				int pp=0;
				_sprintf(strtmp,vertex,tmp,rns[0],rns[1],rns[0],rns[1],"tmp",rns[0],rns[1],rns[0],rns[1],tmp,"tmp");

				char * parsed=str_get_scriptfns(strtmp,&pp);

				_sprintf(&res[pos],"%s",parsed);
				pos+=(int)strlen(parsed);
				update=true;
			}
		}
		else
		{
			_sprintf(&res[pos],"%s\n",tmp);
			pos+=(int)strlen(tmp)+1;
		}

		tmp=str_parse(script,&p);
	}

	if (update)
	{
		free(script);
		script=res;
	}
	else free(res);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::parse_inner_func(char *script,char *&funcs)
{
	char strtmp[65536];

	char * shdsoft1 =	"float libsoftcalcshadowX%s(vec4 pos,float zval)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.r = zval - tmp.x;\n"
						"	tmp = cmp(tmp.r,psone,pszero);\n"
						"	tmp.x = BoundsViewAffect(pos);\n"
						"	tmp.x;\n"
						"};\n"
						"\n"
						"float libsoftcalcshadowdistX%s(vec4 pos)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.x;\n"
						"};\n"										// 4 %s
						"\n"
						"float libsoftcalcshadowpointX%s(vec4 pos,float zval,float dc,float zz)\n"
						"{\n"
						"	tmp.zw = pos.zw;\n"
						"	tmp.xy = pos.xy + vec2(dc,dc);\n"
						"	arte.x = libsoftcalcshadowdistX%s(tmp);\n"
						"	res.y = zval - arte.x;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x = res.y;\n"
						"	tmp.xy = pos.xy + vec2(dc,-dc);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	tmp.xy = pos.xy + vec2(-dc,dc);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	tmp.xy = pos.xy + vec2(-dc,-dc);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	arte.x = arte.x * 0.25;\n"
						"	arte.x = zval - arte.x;\n"
						"	arte = cmp(arte.x,psone,pszero);\n"
						"	if (zz>0.5) arte.x=1.0;\n"
						"	if (arte.x<0.1) res.x=0.0;\n"
						"	if (pos.x<0.05) res.x=0.0;\n"
						"	if (pos.y<0.05) res.x=0.0;\n"
						"	if (pos.x>0.95) res.x=0.0;\n"
						"	if (pos.y>0.95) res.x=0.0;\n"
						"	res.x;\n"
						"};\n"										// 5 %s
						"\n"
						"float libsoftcalcshadowpointXB%s(vec4 pos,float zval,float dc,float zz)\n"
						"{\n"
						"	tmp.zw = pos.zw;\n"
						"	tmp.xy = pos.xy + vec2(0,dc);\n"
						"	arte.x = libsoftcalcshadowdistX%s(tmp);\n"
						"	res.y = zval - arte.x;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x = res.y;\n"
						"	tmp.xy = pos.xy + vec2(0,-dc);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	tmp.xy = pos.xy + vec2(dc,0);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	tmp.xy = pos.xy + vec2(-dc,0);\n"
						"	arte.y = libsoftcalcshadowdistX%s(tmp);\n"
						"	arte.x += arte.y;\n"
						"	res.y = zval - arte.y;\n"
						"	res.y = cmp(res.y,1.0,0.0);\n"
						"	res.y = BoundsViewAffect(tmp);\n"
						"	res.x += res.y;\n"
						"	arte.x = arte.x * 0.25;\n"
						"	arte.x = zval - arte.x;\n"
						"	arte = cmp(arte.x,psone,pszero);\n"
						"	arte.x = BoundsViewAffect(pos);\n"
						"	if (zz>0.5) arte.x=1.0;\n"
						"	if (arte.x<0.1) res.x=0.0;\n"
						"	if (pos.x<0.05) res.x=0.0;\n"
						"	if (pos.y<0.05) res.x=0.0;\n"
						"	if (pos.x>0.95) res.x=0.0;\n"
						"	if (pos.y>0.95) res.x=0.0;\n"
						"	res.x;\n"
						"};\n"
						"\n"
						"float libcalcshadowsoftX%s(vec4 pos,float zval,float blur)\n"
						"{\n"
						"	ds.x = libsoftcalcshadowdistX%s(pos);\n"
						"	ds.x = zval - ds.x;\n"
						"	res = cmp(ds.x,psone,pszero);\n"
						"	res.y = BoundsViewAffect(pos);\n"
						"	ds.w = res.y;\n"
						"	res.x = 3.0 * res.y;\n"
						"	ds.y = 1.0;\n"
						"	ds.y = BoundsViewAffect(pos);\n"
						"	ds.x = 2000.0 * ds.x * ds.y;\n"
						"	if (ds.x<0.0) ds.x=1.0;\n"
						"	res.z = zval * zval * blur;\n"
						"	res.z = res.z * ds.x;\n"
						"	res.y =libsoftcalcshadowdistX%s(pos,zval,0.0005*res.z,ds.w);\n"
						"	res.x += 2.0 * res.y;\n"
						"	res.y =libsoftcalcshadowdistX%s(pos,zval,0.0005*res.z,ds.w);\n"
						"	res.x += 2.0 * res.y;\n"
						"	res.y =libsoftcalcshadowpointXB%s(pos,zval,0.0009*res.z,ds.w);\n"
						"	res.x += res.y;\n"
						"	res.y =libsoftcalcshadowdistX%s(pos,zval,0.0011*res.z,ds.w);\n"
						"	res.x += res.y;\n"
						"	res.y =libsoftcalcshadowdistX%s(pos,zval,0.0013*res.z,ds.w);\n"
						"	res.x += res.y;\n"
						"	res.y =libsoftcalcshadowpointXB%s(pos,zval,0.0017*res.z,ds.w);\n"
						"	res.x += 1.5 * res.y;\n"
						"	res.y =libsoftcalcshadowdistX%s(pos,zval,0.002*res.z,ds.w);\n"
						"	res.x += 2.0 * res.y;\n"
						"	res.x = 1.0 - 0.02222 * res.x;\n"
						"	res.x;\n"
						"};\n";										// 4 %s


	char * shdsoft4 =	"float libsoftcalcshadow%s(vec4 pos,float zval)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.r = zval - tmp.x;\n"
						"	tmp = cmp(tmp.r,psone,pszero);\n"
						"	tmp.x = BoundsViewAffect(pos);\n"
						"	tmp.x;\n"
						"};\n"
						"\n"
						"float libsoftcalcshadowdist%s(vec4 pos)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.x;\n"
						"};\n"										// 4 %s
						"\n"
						"float libsoftcalcshadowpoint%s(vec4 pos,float zval,float dc)\n"
						"{\n"
						"	tmp.zw = pos.zw;\n"
						"	tmp.xy = pos.xy + vec2(dc,dc);\n"
						"	res.x = libsoftcalcshadow%s(tmp,zval);\n"
						"	tmp.xy = pos.xy + vec2(dc,-dc);\n"
						"	res.x += libsoftcalcshadow%s(tmp,zval);\n"
						"	tmp.xy = pos.xy + vec2(-dc,dc);\n"
						"	res.x += libsoftcalcshadow%s(tmp,zval);\n"
						"	tmp.xy = pos.xy + vec2(-dc,-dc);\n"
						"	res.x += libsoftcalcshadow%s(tmp,zval);\n"
						"	res.x;\n"
						"};\n"										// 5 %s
						"\n"
						"float libcalcshadowsoft%s(vec4 pos,float zval)\n"
						"{\n"
						"	ds.x = libsoftcalcshadowdist%s(pos);\n"
						"	ds.x = zval - ds.x;\n"
						"	res = cmp(ds.x,psone,pszero);\n"
						"	res.x = BoundsViewAffect(pos);\n"
						"	ds.x = ds.x * 100.0;\n"
						"	if (ds.x<0.0) ds.x=1.0;\n"
						"	res.w = pos.w * ds.x;\n"
						"	res.x += libsoftcalcshadowpoint%s(pos,zval,0.001*res.w);\n"
						"	res.y = libsoftcalcshadowpoint%s(pos,zval,0.0045*res.w);\n"
						"	res.x += 2.0 * res.y;\n"
						"	res.x = 1.0 - 0.0769231 * res.x;\n"
						"	res.x;\n"
						"};\n";										// 4 %s

	char * shdmouss =   "vec3 libsimplemouss%s(vec3 pos,vec4 mouss)\n"
						"{\n"
						"	col = vec4(0,0,0,0);\n"
						"	if (mouss.w>1)\n"
						"	{\n"
						"		d.x = mouss.x-pos.x;\n"
						"		d.z = mouss.z-pos.z;\n"
						"		d.x = 0.5 + (0.5*d.x)/mouss.w;\n"
						"		d.y = 0.5 + (0.5*d.z)/mouss.w;\n"
						"		d.xy = clamp(d.xy,0,1);\n"
						"		h = sample(%s,d.yx);\n"
						"		col.rgb = h.rgb * h.aaa;\n"
						"	}\n"
						"	col.rgb;\n"
						"};\n";

	char * shdmouss2 =  "vec3 libmouss%s(vec3 pos,vec2 deform,vec4 mouss,vec2 time)\n"
						"{\n"
						"	col = vec4(0,0,0,0);\n"
						"	if (mouss.w>1)\n"
						"	{\n"
						"		d.x = mouss.x-pos.x;\n"
						"		d.z = mouss.z-pos.z;\n"
						"		d.x = 0.5 + (0.5*d.x)/mouss.w;\n"
						"		d.y = 0.5 + (0.5*d.z)/mouss.w;\n"
						"		d.xy += 0.01*deform.xy;\n"
						"		d.x += 0.01*sin(tmp.x+time.x);\n"
						"		d.y += 0.01*sin(tmp.z+time.y);\n"
						"		d.xy = clamp(d.xy,0,1);\n"
						"		h = sample(%s,d.yx);\n"
						"		col.rgb = h.rgb * h.aaa;\n"
						"	}\n"
						"	col.rgb;\n"
						"};\n";

	char * shdpoly =	"vec3 libpolynomial(vec3 col,vec4 koef)\n"
						"{\n"
						"	tmp.x = col.r;\n"
						"	tmp.y = tmp.x * tmp.x;\n"
						"	tmp.z = tmp.y * tmp.x;\n"
						"	tmp.w = tmp.y * tmp.y;\n"
						"	res.r = tmp.w + koef.x*tmp.z + koef.y*tmp.y + koef.z*tmp.x + koef.w;\n"
						"	tmp.x = col.g;\n"
						"	tmp.y = tmp.x * tmp.x;\n"
						"	tmp.z = tmp.y * tmp.x;\n"
						"	tmp.w = tmp.y * tmp.y;\n"
						"	res.g = tmp.w + koef.x*tmp.z + koef.y*tmp.y + koef.z*tmp.x + koef.w;\n"
						"	tmp.x = col.b;\n"
						"	tmp.y = tmp.x * tmp.x;\n"
						"	tmp.z = tmp.y * tmp.x;\n"
						"	tmp.w = tmp.y * tmp.y;\n"
						"	res.b = tmp.w + koef.x*tmp.z + koef.y*tmp.y + koef.z*tmp.x + koef.w;\n"
						"	res.rgb;\n"
						"};\n";

	char * shdbilinear=	"float libcalcshadow%s(vec3 pos,float zval)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.r = zval - tmp.x;\n"
						"	tmp.x = cmp(tmp.r,0.0,1.0);\n"
						"	tmp.x = BoundsViewIntegrate(pos);\n"
						"	tmp.x;\n"
						"};\n"
						"\n"
						"float libsinglebilinear%s(vec3 poz,float zval)\n"
						"{\n"
						"	map.xyz=poz;\n"
						"	pos.xy = poz.xy * %s.xx;\n"
						"	inter.xy = frac(pos.xy);\n"
						"	pos.xy = floor(pos.xy);\n"
						"	map.xy = pos.xy * %s.yy;\n"
						"	d.x = libcalcshadow%s(map.xyz,zval);\n"
						"	pos.x+=1.0;\n"
						"	map.xy = pos.xy * %s.yy;\n"
						"	d.y = libcalcshadow%s(map.xyz,zval);\n"
						"	pos.y+=1.0;\n"
						"	map.xy = pos.xy * %s.yy;\n"
						"	d.w = libcalcshadow%s(map.xyz,zval);\n"
						"	pos.x-=1.0;\n"
						"	map.xy = pos.xy * %s.yy;\n"
						"	d.z = libcalcshadow%s(map.xyz,zval);\n"
						"	a.x=d.x+inter.x*(d.y-d.x);\n"
						"	a.y=d.z+inter.x*(d.w-d.z);\n"
						"	d.x=a.x+inter.y*(a.y-a.x);\n"
						"	d.x;\n"
						"};\n"
						"\n"
						"float libshadowbilinear%s(vec4 poz,float zval)\n"
						"{\n"
						"	pos=poz;\n"
						"	d.x=libsinglebilinear%s(pos.xyz,zval);\n"
						"	pos.x+=2.0*%s.y;\n"
						"	d.x+=libsinglebilinear%s(pos.xyz,zval);\n"
						"	pos.y+=2.0*%s.y;\n"
						"	d.x+=libsinglebilinear%s(pos.xyz,zval);\n"
						"	pos.x-=2.0*%s.y;\n"
						"	d.x+=libsinglebilinear%s(pos.xyz,zval);\n"
						"	d.x=d.x*0.25;\n"
						"	d.x;\n"
						"};\n";

	char * shdsimple=	"float libcalcsimpleshadow%s(vec4 pos,float zval)\n"
						"{\n"
						"	tmp = sample(%s,pos.xy);\n"
						"	tmp = RGBToValue(tmp);\n"
						"	tmp.r = zval - tmp.x;\n"
						"	tmp.x = cmp(tmp.r,0.0,1.0);\n"
						"	tmp.x = BoundsViewIntegrate(pos);\n"
						"	tmp.x;\n"
						"};\n";

	int p=0;
	char *rns[8];
	char *tmp=str_parse(script,&p);
	while (tmp)
	{
		if (str_match(tmp,"ShadowMappingSoftExtended"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==4)
			{
				char *rn=rns[0];
				int n=0;
				sscanf(rn,"%d",&n);

				if (shadowssoft1[n]==0)
				{					
					_sprintf(strtmp,shdsoft1,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					shadowssoft1[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"ShadowMappingSoft"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==3)
			{
				char *rn=rns[0];
				int n=0;
				sscanf(rn,"%d",&n);

				if (shadowssoft4[n]==0)
				{					
					_sprintf(strtmp,shdsoft4,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					shadowssoft4[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"ShadowMappingBilinear"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==4)
			{
				char *rn=rns[0];
				char *rn4=rns[3];
				int n=0;
				sscanf(rn,"%d",&n);

				if (shadowsbilinear[n]==0)
				{					
					_sprintf(strtmp,shdbilinear,rn,rn,rn,rn4,rn4,rn,rn4,rn,rn4,rn,rn4,rn,rn,rn,rn4,rn,rn4,rn,rn4,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					shadowsbilinear[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"TextureSpaceSimple"))
		{
			char *str=str_return_parentheses(tmp);

			//vec3 libmouss(vec3 pos,vec4 mouss)

	        if (multiple_argument_parse_virg(str,rns)==3)
			{
				char *rn=rns[0];
				int n=0;
				sscanf(rn,"%d",&n);

				if (mouss1[n]==0)
				{					
					_sprintf(strtmp,shdmouss,rn,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					mouss1[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"TextureSpace"))
		{
			char *str=str_return_parentheses(tmp);

			//vec3 libmouss(vec3 pos,vec2 deform,vec4 mouss,vec2 time)

	        if (multiple_argument_parse_virg(str,rns)==5)
			{
				char *rn=rns[0];
				int n=0;
				sscanf(rn,"%d",&n);

				if (mouss2[n]==0)
				{					
					_sprintf(strtmp,shdmouss2,rn,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					mouss2[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"ShadowMappingSimple"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==3)
			{
				char *rn=rns[0];
				int n=0;
				sscanf(rn,"%d",&n);

				if (shadows[n]==0)
				{					
					_sprintf(strtmp,shdsimple,rn,rn);

					int pos=0;
					char * parsed=str_get_scriptfns(strtmp,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					shadows[n]=1;
				}
			}
		}
		else
		if (str_match(tmp,"Polynomial"))
		{
			char *str=str_return_parentheses(tmp);

	        if (multiple_argument_parse_virg(str,rns)==5)
			{
				if (polynomial==0)
				{					
					int pos=0;
					char * parsed=str_get_scriptfns(shdpoly,&pos);

					int lent=(int)strlen(parsed);
					if (funcs)
					{
						int len=(int)strlen(funcs);
						char * nouv=(char*)malloc(len+lent+1);
						_sprintf(nouv,"%s%s",funcs,parsed);
						free(funcs);
						funcs=nouv;
					}
					else
					{
						char * nouv=(char*)malloc(lent+1);
						_sprintf(nouv,"%s",parsed);
						funcs=nouv;
					}

					free(parsed);				
					polynomial=1;
				}
			}
		}

		tmp=str_parse(script,&p);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::compile_script(char *script,CList <Code> *vp)
{
	char temp[8192];
	char *tmp;
	int p;

	levelrep=0;
	levelifelse=0;

    localint=0;

	TAG_secondary_vars=false;
	actual_vp=vp;
	p=0;
	tmp=str_parse(script,&p);
	while (tmp)
	{
		nstrings=0;
		compile_init_variables(tmp,vp);
		tmp=str_parse(script,&p);
	}

	if ((api==1)&&(pixelshader))
	{
		p=0;
		tmp = str_parse(script, &p);
		while (tmp)
		{
			if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

			_sprintf(temp, "%s", tmp);
			tmp = str_parse(script, &p);
			nstrings = 0;
			if (tmp) compile_code(temp, vp, 0);
			else compile_code(temp, vp, 1);
		}
	}
	else
	{
		p=0;
		tmp=str_parse(script,&p);
		while (tmp)
		{
			if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

			nstrings=0;
			compile_code(tmp,vp,0);
			tmp=str_parse(script,&p);
		}
	}
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	_mul		=	0,
	_dp3		=	1,
	_dp4		=	2,
	_rsq		=	3,
	_rcp		=	4,
	_add		=	5,
	_sub		=	6,
	_max		=	7,
	_min		=	8,
	_mov		=	9,
	_sge		=	10,
	_exp2		=	11,
	_log2		=	12,
	_scalar_exp	=	13,
	_mad		=	14,
	_slt		=	15,
	_lit		=	16
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * Inst[][2]={

	{ "mul","MUL"},
	{ "dp3","DP3"},
	{ "dp4","DP4"},
	{ "rsq","RSQ"},
	{ "rcp","RCP"},
	{ "add","ADD"},
	{ "sub","SUB"},
	{ "max","MAX"},
	{ "min","MIN"},
	{ "mov","MOV"},
	{ "sge","SGE"},
	{ "exp","EX2"},
	{ "log","LG2"},
	{ "expp","EXP"},
	{ "mad","MAD"},
	{ "slt","SLT"},
	{ "lit","LIT"}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::compile_macro(Macro *m,char * param,CList <Code> *vp)
{
	char *tmp;
	char cmd[1024];
	int i,p,n;
	char *script=m->code;
	char *dst[16];
	char *src[16];
	char *output;

	close_temp_register();
	last_register_macro=last_temp_register;
	actual_vp=vp;

	TAG_secondary_vars = true;

	p = 0;
	tmp = str_parse(script, &p);
	while (tmp)
	{
		n = str_char(tmp, '=');
		if (n != -1)
		{
			_sprintf(cmd, tmp);
			cmd[n] = '\0';
			var(cmd);
		}
		tmp = str_parse(script, &p);
	}

	p = 0;
	n = 0;
	tmp = str_parse(m->def, &p);
	while (tmp)
	{
		dst[n] = var(tmp);
		n++;
		tmp = str_parse(m->def, &p);
	}

	p=0;
	n=0;
	tmp=str_parse(param,&p);
	while (tmp)
	{
		src[n]=var(tmp);
		n++;
		tmp=str_parse(param,&p);
	}

	for (i=0;i<n;i++)
	{
		if (api==2)
		{
			em->Add(OP_MOV,dst[i],src[i],NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = %s",dst[i],src[i]);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"%s %s,%s",Inst[_mov][api],dst[i],src[i]);
				AddInst(vp,c);
			}
		}
	}

	p=0;
	tmp = str_parse(script, &p);
	while (tmp)
	{
		n = str_char(tmp, '=');
		if (n != -1)
		{
			_sprintf(cmd, tmp);
			compile_code(cmd,vp,0);
		}
		else
		{
			if (strcmp(tmp,"")!=0) output=var(tmp);
		}
		tmp=str_parse(script,&p);
	}

	close_temp_register();

	TAG_secondary_vars=false;
	last_temp_register=last_register_macro;

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::compile_psfunction(Macro *fn,CList <Code> *vp)
{
	main=0;
	char *res=compile_function(fn,vp);
	main=1;
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::compile_function(Macro *fn,CList <Code> *vp)
{
	char *tmp;
	char ss[1024];
	int p,n;
	int r=8;
	char *script=fn->code;
	char *output;
	CVPVariable * ch;
	Chaine s;
	char *sv;
	char sfn[1024];
	int io=0;
	int levelrepfn=0;

	nmatrices=0;
	nfloats=0;

    strcpy(&paramsFn[0][0],"");
    strcpy(&paramsFn[1][0],"");
    strcpy(&paramsFn[2][0],"");
    strcpy(&paramsFn[3][0],"");

	defines[0]=fn->defs[0];
	defines[1]=fn->defs[1];
	defines[2]=fn->defs[2];
	defines[3]=fn->defs[3];

	levelrep=0;
	levelrepfn=0;
    localint=0;
    
	func=false;

	if ((api==0)&&(shadermodel3))
	{
		func=true;

		last_register=0; Vars.Free(); clear_registers();
		actual_vp=vp;
		last_register_macro=last_temp_register=last_register;
		p=0;
		tmp=str_parse(script,&p);
		while (tmp)
		{
			nstrings=0;
			compile_init_variables(tmp,vp);
			tmp=str_parse(script,&p);
		}

		nstrings=0;

		if (fn->ret==CALL_FLOAT) _sprintf(sfn,"float %s",fn->name);
		else
		if (fn->ret==CALL_VECTOR2) _sprintf(sfn,"float2 %s",fn->name);
		else
		if (fn->ret==CALL_VECTOR3) _sprintf(sfn,"float3 %s",fn->name);
		else _sprintf(sfn,"float4 %s",fn->name);

		TAG_secondary_vars=false;
		p=0;
		n=0;
		tmp=str_parse_char(fn->def,&p,',');
		while (tmp)
		{
			_sprintf(s.str,tmp);
			_sprintf(ss,tmp);
			io=0;
			sv="float4";
			if (fn->defs[n]==CALL_FLOAT) sv="float";
			if (fn->defs[n]==CALL_VECTOR2) sv="float2";
			if (fn->defs[n]==CALL_VECTOR3) sv="float3";
			if (fn->defs[n]==CALL_IOFLOAT) { sv="float"; io=1; }
			if (fn->defs[n]==CALL_IOVECTOR2) { sv="float2"; io=1; }
			if (fn->defs[n]==CALL_IOVECTOR3) { sv="float3"; io=1; }
			if (fn->defs[n]==CALL_IOVECTOR4) { sv="float4"; io=1; }
		
			defsFn[n]=fn->defs[n];

			if (io)
			{
				if (n>0) _sprintf(sfn,"%s,inout %s %s",sfn,sv,tmp);
				else _sprintf(sfn,"%s(inout %s %s",sfn,sv,tmp);
			}
			else
			{
				if (n>0) _sprintf(sfn,"%s,%s %s",sfn,sv,tmp);
				else _sprintf(sfn,"%s(%s %s",sfn,sv,tmp);
			}

			ch=new CVPVariable(REGISTER,ss,fn->entete);
			ch->tobeused=true;
			Vars.Add(s.str,*ch);
			delete ch;
			_sprintf(&paramsFn[n][0],ss);
			n++;
			tmp=str_parse_char(fn->def,&p,',');
		}	

		_sprintf(sfn,"%s)",sfn);
        strcpy(entete,sfn);

		if (!shadermodel3) addsemi=true;

		levelifelse=0;

		p=0;
		tmp=str_parse(script,&p);
		while (tmp)
		{
			if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

			n=str_char(tmp,'=');
			if (str_match(tmp,"#endif"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"break"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"discard"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"continue"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"#else"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"if("))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			if (str_match(tmp,"endrep"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
				levelrepfn--;
			}
			else
			if (str_match(tmp,"rep"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
				levelrepfn++;
			}
			else
			if (str_match(tmp,"endwhile"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
				levelrepfn--;
			}
			else
			if (str_match(tmp,"while"))
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
				levelrepfn++;
			}
			else
			if (n!=-1)
			{
				nstrings=0;
				compile_code(tmp,vp,0);
				close_temp_register();
			}
			else
			{
				if (strcmp(tmp,"")!=0)
				{
					if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

					if (str_char(tmp,'+')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'-')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'/')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'*')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'(')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,')')!=-1) SYNTAXERROR=true;
					if (str_char_count(tmp,'.')>1) SYNTAXERROR=true;

					compile_expression=true;
					_sprintf(c.str,"return %s",var(tmp));
					AddInst(vp,c);
					compile_expression=false;

					if (!shadermodel3) addsemi=false;

					if (!shadermodel3)
					{
                        strcpy(c.str,"}");
						AddInst(vp,c);
                        strcpy(c.str,"");
						AddInst(vp,c);
					}
				}
			}
			tmp=str_parse(script,&p);
		}

		if (!shadermodel3) addsemi=false;
		TAG_secondary_vars=false;
		TempVars.Free();
		last_register=last_temp_register=last_register_macro;
		close_temp_register();
		for (n=0;n<32;n++) if (tagtempreg[n]==666) tagtempreg[n]=0;
		func=false;

        strcpy(&paramsFn[0][0],"");
        strcpy(&paramsFn[1][0],"");
        strcpy(&paramsFn[2][0],"");
        strcpy(&paramsFn[3][0],"");

		if (levelrepfn>0) REPERROR=true;

		return fn->name;
	}

	nstrings=0;
	last_register_macro=last_temp_register=last_register;

	for (n=0;n<32;n++) tagregsave[n]=0;
	for (n=0;n<32;n++) tagreg[n]=0;
	for (n=0;n<32;n++) tagtempreg[n]=0;

	CList <Code> tmpvp;

	if (api==1)
	{ 
		last_register=0; Vars.Free(); clear_registers();
		actual_vp=&tmpvp;
		last_register_macro=last_temp_register=last_register;
		p=0;
		tmp=str_parse(script,&p);
		while (tmp)
		{
			nstrings=0;
			compile_init_variables(tmp,&tmpvp);
			tmp=str_parse(script,&p);
		}

		nstrings=0;
	}


	actual_vp=&tmpvp;

	if (fn->ret==CALL_FLOAT) _sprintf(sfn,"float %s",fn->name);
	else
	if (fn->ret==CALL_VECTOR2) _sprintf(sfn,"vec2 %s",fn->name);
	else
	if (fn->ret==CALL_VECTOR3) _sprintf(sfn,"vec3 %s",fn->name);
	else _sprintf(sfn,"vec4 %s",fn->name);

	TAG_secondary_vars=false;
	p=0;
	n=0;
	tmp=str_parse_char(fn->def,&p,',');
	while (tmp)
	{
		_sprintf(s.str,tmp);
		if (api==0) { _sprintf(ss,"r%d",fn->registers[n]); tagtempreg[fn->registers[n]]=666; }
		else _sprintf(ss,tmp);

		io=0;
		sv="vec4";
		if (fn->defs[n]==CALL_FLOAT) sv="float";
		if (fn->defs[n]==CALL_VECTOR2) sv="vec2";
		if (fn->defs[n]==CALL_VECTOR3) sv="vec3";
		if (fn->defs[n]==CALL_IOFLOAT) { sv="float"; io=1; }
		if (fn->defs[n]==CALL_IOVECTOR2) { sv="vec2"; io=1; }
		if (fn->defs[n]==CALL_IOVECTOR3) { sv="vec3"; io=1; }
		if (fn->defs[n]==CALL_IOVECTOR4) { sv="vec4"; io=1; }

		defsFn[n]=fn->defs[n];
		
		if (io)
		{
			if (n>0) _sprintf(sfn,"%s,inout %s %s",sfn,sv,tmp);
			else _sprintf(sfn,"%s(inout %s %s",sfn,sv,tmp);
		}
		else
		{
			if (n>0) _sprintf(sfn,"%s,%s %s",sfn,sv,tmp);
			else _sprintf(sfn,"%s(%s %s",sfn,sv,tmp);
		}

        strcpy(&paramsFn[n][0],ss);

		ch=new CVPVariable(REGISTER,ss,fn->entete);
		ch->tobeused=true;
		Vars.Add(s.str,*ch);
		delete ch;
		n++;
		tmp=str_parse_char(fn->def,&p,',');
	}

	_sprintf(sfn,"%s)",sfn);

	if (api==1) addsemi=true;

	levelifelse=0;
	levelrepfn=0;

	p=0;
	tmp=str_parse(script,&p);
	while (tmp)
	{
		if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

		n=str_char(tmp,'=');

		if (str_match(tmp,"#endif"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"break"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"discard"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"continue"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"#else"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"if("))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		if (str_match(tmp,"endrep"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
			levelrepfn--;
		}
		else
		if (str_match(tmp,"rep"))
		{
			strcpy(ERRORSTRREP,tmp);
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
			levelrepfn++;
		}
		else
		if (str_match(tmp,"endwhile"))
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
			levelrepfn--;
		}
		else
		if (str_match(tmp,"while"))
		{
			strcpy(ERRORSTRREP,tmp);
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
			levelrepfn++;
		}
		else
		if (n!=-1)
		{
			nstrings=0;
			compile_code(tmp,&tmpvp,0);
			close_temp_register();
		}
		else
		{
			if (strcmp(tmp,"")!=0)
			{
				if (api==0)
				{
					char *st=var(tmp);
					if (strcmp(st,"r11")!=0)
					{
						_sprintf(c.str,"mov r11,%s",st);
						AddInst(&tmpvp,c);
					}
					output="r11";
				}
				else
				{
					if (!((SYNTAXERROR)||(TYPEERROR)||(DEFINEERROR))) strcpy(ERRORSTR,tmp);

					if (str_char(tmp,'+')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'-')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'/')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'*')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,'(')!=-1) SYNTAXERROR=true;
					if (str_char(tmp,')')!=-1) SYNTAXERROR=true;
					if (str_char_count(tmp,'.')>1) SYNTAXERROR=true;

					compile_expression=true;
					_sprintf(c.str,"return %s",var(tmp));
					AddInst(&tmpvp,c);
					compile_expression=false;

					addsemi=false;

                    strcpy(c.str,"}");
					AddInst(&tmpvp,c);					
				}
			}
		}
		tmp=str_parse(script,&p);
	}

	if (api==1)
	{
		_sprintf(c.str,"%s",sfn);
		AddInst(vp,c);
        strcpy(c.str,"{");
		AddInst(vp,c);

		for (n=0;n<32;n++)
		if (tagregsave[n])
		{
			_sprintf(c.str,"vec4 r%d;",n);
			AddInst(vp,c);
		}

		_sprintf(c.str,"");
		AddInst(vp,c);

		Code * cc=tmpvp.GetFirst();
		while (cc)
		{
			strcpy(c.str,cc->str);
			AddInst(vp,c);
			cc=tmpvp.GetNext();
		}		
	}

	tmpvp.Free();

	addsemi=false;
	TAG_secondary_vars=false;
	TempVars.Free();
	last_register=last_temp_register=last_register_macro;
	close_temp_register();
	for (n=0;n<32;n++) if (tagtempreg[n]==666) tagtempreg[n]=0;

	func=false;

    strcpy(&paramsFn[0][0],"");
    strcpy(&paramsFn[1][0],"");
    strcpy(&paramsFn[2][0],"");
    strcpy(&paramsFn[3][0],"");

	if (levelrepfn>0) REPERROR=true;

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::floating(char *str)
{
	if (numb(str)) return true;

	if (strcmp(paramsFn[0],str)==0) { if (defines[0]==CALL_FLOAT) return true; }
	if (strcmp(paramsFn[1],str)==0) { if (defines[1]==CALL_FLOAT) return true; }
	if (strcmp(paramsFn[2],str)==0) { if (defines[2]==CALL_FLOAT) return true; }
	if (strcmp(paramsFn[3],str)==0) { if (defines[3]==CALL_FLOAT) return true; }

	return false;
}

//char definestr[MAX_ENTRIES][32];
//DEFINEERROR

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::isdefined(char *str)
{
    int n;
	char ss[512];
	strcpy(ss,str);

	if (numb(ss)) return 666;

	int p=str_char(ss,'.');
	if (p>=0) ss[p]='\0';

	p=str_char(ss,'[');
	if (p>=0) ss[p]='\0';

	for (n=0;n<ndefs;n++)
		if (strcmp(definestr[n],ss)==0) return n;
	
	if (strcmp(paramsFn[0],ss)==0) return 666;
	if (strcmp(paramsFn[1],ss)==0) return 666;
	if (strcmp(paramsFn[2],ss)==0) return 666;
	if (strcmp(paramsFn[3],ss)==0) return 666;

	if (is_constant(ss)) return 666;
	if (is_ps_constant(ss)) return 666;

	if (is_output(ss)) return 666;
	if (is_input(ss)) return 666;
    
    //for (n=0;n<npsvectors;n++)
    //    if (strcmp(psvectors[n],ss)==0) return 666;

	if (strcmp(ss,"Out")==0) return 666;

	if (strcmp(str,"SCALAR_1DIV3")==0) return 666;
	if (strcmp(str,"SCALAR_1DIV5")==0) return 666;
	if (strcmp(str,"SCALAR_1DIV10")==0) return 666;
	if (strcmp(str,"SCALAR_1DIV4")==0) return 666;

	if (strcmp(str,"SCALAR_HALF")==0) return 666;
	if (strcmp(str,"SCALAR_ZERO")==0) return 666;
	if (strcmp(str,"SCALAR_ONE")==0) return 666;
	if (strcmp(str,"SCALAR_PI")==0) return 666;

	if (strcmp(str,"SPRITEZVALUE")==0) return 666;
	if (strcmp(str,"ZSCALE")==0) return 666;

	if ((str[0]=='r')&&((str[1]>='0')&&(str[1]<='9'))) return 666;

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::isbasicdefined(char *str)
{
	char ss[512];
	strcpy(ss,str);

	if (numb(ss)) return 666;

	int p=str_char(ss,'.');
	if (p>=0) ss[p]='\0';

	p=str_char(ss,'[');
	if (p>=0) ss[p]='\0';

	if (strcmp(paramsFn[0],ss)==0) return 666;
	if (strcmp(paramsFn[1],ss)==0) return 666;
	if (strcmp(paramsFn[2],ss)==0) return 666;
	if (strcmp(paramsFn[3],ss)==0) return 666;

	if (is_constant(ss)) return 666;
	if (is_ps_constant(ss)) return 666;
    
    //for (int n=0;n<npsvectors;n++)
    //    if (strcmp(psvectors[n],ss)==0) return 666;

	if (is_output(ss)) return 666;
	if (is_input(ss)) return 666;

	if (strcmp(ss,"Out")==0) return 666;

    if (str[0]=='S')
    {
        if (strcmp(str,"SCALAR_1DIV3")==0) return 666;
        if (strcmp(str,"SCALAR_1DIV5")==0) return 666;
        if (strcmp(str,"SCALAR_1DIV10")==0) return 666;
        if (strcmp(str,"SCALAR_1DIV4")==0) return 666;

        if (strcmp(str,"SCALAR_HALF")==0) return 666;
        if (strcmp(str,"SCALAR_ZERO")==0) return 666;
        if (strcmp(str,"SCALAR_ONE")==0) return 666;
        if (strcmp(str,"SCALAR_PI")==0) return 666;

        if (strcmp(str,"SPRITEZVALUE")==0) return 666;
    }
	if (strcmp(str,"ZSCALE")==0) return 666;

	if ((str[0]=='r')&&((str[1]>='0')&&(str[1]<='9'))) return 666;

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::registerdefine(char *str)
{
	char ss[512];
	strcpy(ss,str);
	int p=str_char(ss,'.');
	if (p>=0) ss[p]='\0';
	strcpy(definestr[ndefs++],ss);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CorrectModifiers(char *exp)
{
    int c1,c2,c3,c4;
    char *tmp;
    int p=str_char(exp,'.');
    if (p<0) return true;
    
    tmp=&exp[p+1];
    
    c1=c2=c3=c4=0;
    
    for (int n=0;n<(int)strlen(tmp);n++)
    {
        if ((tmp[n]!='x')&&(tmp[n]!='y')&&(tmp[n]!='z')&&(tmp[n]!='w')&&
            (tmp[n]!='r')&&(tmp[n]!='g')&&(tmp[n]!='b')&&(tmp[n]!='a')) return false;
        
        if (tmp[n]=='x') c1++;
        if (tmp[n]=='y') c2++;
        if (tmp[n]=='z') c3++;
        if (tmp[n]=='w') c4++;

        if (tmp[n]=='r') c1++;
        if (tmp[n]=='g') c2++;
        if (tmp[n]=='b') c3++;
        if (tmp[n]=='a') c4++;
    }
    
    if (c1>1) return false;
    if (c2>1) return false;
    if (c3>1) return false;
    if (c4>1) return false;
    
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::result_parse_condition(char *str)
{
	int n=0;
	int p=0;
	int len=(int)strlen(str);

	while (n<len)
	{
		if ((str[n]=='(')||(str[n]==')')||(str[n]=='&')||(str[n]=='|')||(str[n]=='>')||(str[n]=='<')||(str[n]=='='))
		{
			tmpstrcond[p++]=str[n++];
		}
		else
		{
			int ii;
			int st=n;
			while ((!((str[n]=='(')||(str[n]==')')||(str[n]=='&')||(str[n]=='|')||(str[n]=='>')||(str[n]=='<')||(str[n]=='=')))&&(n<len)) n++;
			char svar[128];
			int s=0;
			for (ii=st;ii<n;ii++) svar[s++]=str[ii];
			svar[s]=0;
			char *cnv=var(svar);
			for (ii=0;ii<(int)strlen(cnv);ii++) tmpstrcond[p++]=cnv[ii];
		}
	}
	tmpstrcond[p]=0;

	return tmpstrcond;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::compile_code(char *str,CList <Code> *vp,int last_line_pixelshader)
{
    char chaine[8192];
    char tmp[8192];
    int p;
	char * res,*aff;
	int r;
	char ch[4]={ 'a','b','c','d' };
	bool b;	

	if ((str_match(str,"break"))&&(shadermodel3))
	{
        strcpy(c.str, "break");
		AddInst(vp, c);
	}
	else
	if ((str_match(str,"continue"))&&(shadermodel3))
	{
        strcpy(c.str, "continue");
		AddInst(vp, c);
	}
	else
	if ((str_match(str,"discard"))&&(shadermodel3))
	{
        strcpy(c.str, sDiscardAPI);
		AddInst(vp, c);
	}
	else
	if ((str_match(str,"#endif"))&&(levelifelse>0))
	{
        strcpy(c.str, "}");
		AddInst00(vp, c);
		levelifelse--;
	}
	else
	if (str_match(str,"#else"))
	{
        strcpy(c.str, "}");
		AddInst00(vp, c);
        strcpy(c.str, "else");
		AddInst00(vp, c);
        strcpy(c.str, "{");
		AddInst00(vp, c);
	}
	else
	if (str_match(str,"#if("))
	{
		compile_expression=true;
		floatprocessing=true;
		char *test = str_return_parentheses(str);

		if (shadermodel3)
		{
			_sprintf(c.str, "if (%s)", result_parse_condition(test));
			AddInst00(vp, c);
            strcpy(c.str, "{");
			AddInst00(vp, c);

			floatprocessing=false;
			compile_expression=false;

			levelifelse++;
		}
		else
		{
			if (str_char(test, '>') != -1)
			{
				p = str_char(test, '>');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s>=%s)", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s>%s)", var(test), var(&test[p + 1]));
			}
			else
			{
				p = str_char(test, '<');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s<=%s)", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s<%s)", var(test), var(&test[p + 1]));
			}

			_sprintf(c.str, "%s", tmp);
			AddInst00(vp, c);
            strcpy(c.str, "{");
			AddInst00(vp, c);

			floatprocessing=false;
			compile_expression=false;

			levelifelse++;
		}
	}
	else
	if (str_match(str,"if("))
	{
		
		if (str_match(str,"break"))
		{
			compile_expression=true;
			floatprocessing=true;
			char *test = str_return_parentheses(str);

			if (str_char(test, '>') != -1)
			{
				p = str_char(test, '>');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s>=%s) break", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s>%s) break", var(test), var(&test[p + 1]));
			}
			else
			{
				p = str_char(test, '<');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s<=%s) break", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s<%s) break", var(test), var(&test[p + 1]));
			}

			_sprintf(c.str, "%s", tmp);
			AddInst(vp, c);

			floatprocessing=false;
			compile_expression=false;
		}
		else
		if (str_match(str,"continue"))
		{
			floatprocessing=true;
			compile_expression=true;

			char *test = str_return_parentheses(str);

			if (str_char(test, '>') != -1)
			{
				p = str_char(test, '>');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s>=%s) continue", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s>%s) continue", var(test), var(&test[p + 1]));
			}
			else
			{
				p = str_char(test, '<');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s<=%s) continue", var(test), var(&test[p + 1]));
				}
				else _sprintf(tmp, "if (%s<%s) continue", var(test), var(&test[p + 1]));
			}

			_sprintf(c.str, "%s", tmp);
			AddInst(vp, c);

			floatprocessing=false;
			compile_expression=false;
		}
		else
		if (str_match(str,"discard"))
		{
			compile_expression=true;
			floatprocessing=true;
			char *test = str_return_parentheses(str);

			if (str_char(test, '>') != -1)
			{
				p = str_char(test, '>');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s>=%s) %s", var(test), var(&test[p + 1]),sDiscardAPI);
				}
				else _sprintf(tmp, "if (%s>%s) %s", var(test), var(&test[p + 1]),sDiscardAPI);
			}
			else
			{
				p = str_char(test, '<');
				test[p] = '\0';
				if (test[p + 1] == '=')
				{
					p++;
					_sprintf(tmp, "if (%s<=%s) %s", var(test), var(&test[p + 1]),sDiscardAPI);
				}
				else _sprintf(tmp, "if (%s<%s) %s", var(test), var(&test[p + 1]),sDiscardAPI);
			}

			_sprintf(c.str, "%s", tmp);
			AddInst(vp, c);

			floatprocessing=false;
			compile_expression=false;
		}
	}
	else
	if (str_match(str,"endrep"))
	{
		b=addsemi;
		addsemi=false;
        strcpy(c.str,"}");
		AddInst(vp,c);
		levelrep--;
		addsemi=b;

		levelrepglobal--;
	}
	else
	if (str_match(str,"rep"))
	{
		b=addsemi;
		addsemi=false;
		
		char *ss=str_return_parentheses(str);
		if (ss)
		{
			strcpy(ERRORSTRREP,str);
			levelrepglobal++;
			_sprintf(tmp,"%s",ss);
			sscanf(tmp,"%d",&r);
			if ((api==0)&&(metal==0)) _sprintf(c.str,"[loop] for (int %c%d=0;%c%d<%d;%c%d++)",ch[levelrep],levelrep,ch[levelrep],levelrep,r,ch[levelrep],levelrep); // [loop]
			else _sprintf(c.str,"for (int %c%d=0;%c%d<%d;%c%d++)",ch[levelrep],levelrep,ch[levelrep],levelrep,r,ch[levelrep],levelrep);
			AddInst(vp,c);
            strcpy(c.str,"{");
			AddInst(vp,c);
			levelrep++;
			addsemi=b;
		}
	}
	else
	if (str_match(str,"endwhile"))
	{
		b=addsemi;
		addsemi=false;
        strcpy(c.str,"}");
		AddInst(vp,c);
		levelrep--;
		addsemi=b;

		levelrepglobal--;
	}
	else
	if (str_match(str,"while"))
	{
		b=addsemi;
		addsemi=false;
		
		char *ss=str_return_parentheses(str);
		if (ss)
		{
			strcpy(ERRORSTRREP,str);
			levelrepglobal++;

			char test[256];
			strcpy(test,ss);
            
            if (shadermodel3)
            {
                _sprintf(tmp, "%s", result_parse_condition(test));
            }
            else
            {
                if (str_char(test, '>') != -1)
                {
                    p = str_char(test, '>');
                    test[p] = '\0';
                    if (test[p + 1] == '=')
                    {
                        p++;
                        _sprintf(tmp, "%s>=%s", var(test), var(&test[p + 1]));
                    }
                    else _sprintf(tmp, "%s>%s", var(test), var(&test[p + 1]));
                }
                else
                {
                    p = str_char(test, '<');
                    test[p] = '\0';
                    if (test[p + 1] == '=')
                    {
                        p++;
                        _sprintf(tmp, "%s<=%s", var(test), var(&test[p + 1]));
                    }
                    else _sprintf(tmp, "%s<%s", var(test), var(&test[p + 1]));
                }
            }

			if ((api==0)&&(metal==0)) _sprintf(c.str,"[loop] while (%s)",tmp); // [loop]
			else _sprintf(c.str,"while (%s)",tmp);
			AddInst(vp,c);

            strcpy(c.str,"{");
			AddInst(vp,c);

			levelrep++;
			addsemi=b;
		}
	}
	else
	if (str_char(str,'=')!=-1)
	{
		strcpy(LASTLINESTR,str);

		_sprintf(chaine,"%s",str);
		str_clean(chaine);

		p=str_char(chaine,'=');
		chaine[p]='\0';

		if ((str_match(&chaine[p+1],"mat"))&&(str_char_count(&chaine[p+1],'(')==1)&&(str_char_count(&chaine[p+1],')')==1)&&(!(str_match(&chaine[p+1],"_mat"))))
		{
			nbmodifiersprocessing=4;

			if (shadermodel)
			{
				int present=-1;
				for (int kk=0;kk<nmatrices;kk++)
					if (strcmp(matrices[kk],chaine)==0) present=kk;

				if (present<0) { present=nmatrices; strcpy(matrices[nmatrices++],chaine); }

				compile_expression=true;
				aff = compile(&chaine[p + 1], chaine, vp);
				compile_expression=false;

				if (isdefined(chaine)<0) registerdefine(chaine);
			}
		}
		else
		if ((str_match(&chaine[p+1],"float"))||((floating(&chaine[p+1]))&&(str_last_char(chaine,'.')==-1)))
		{
			nbmodifiersprocessing=1;

			if (shadermodel)
			{
				int present=-1;
				for (int kk=0;kk<nfloats;kk++)
					if (strcmp(floats[kk],chaine)==0) present=kk;

				floatprocessinginit=false;
				if (present<0)
				{ 
                    if (api==1) _sprintf(c.str, "float %s", chaine);
                    else _sprintf(c.str, "float %s;", chaine);
                    AddInst(vp, c);

					strcpy(floats[nfloats++],chaine);
					floatprocessinginit=true; 
				}
	
				if (floating(&chaine[p+1]))
                {
                    _sprintf(tmp,"%s",&chaine[p+1]);
                }
				else
                {
                    strcpy(tmp,&chaine[p+1]);
                }

                if (!TAG_secondary_vars) last_temp_register=last_register;
                else last_temp_register=last_register_macro;
                
                compile_expression=true;
                floatprocessing=true;
                aff = compile(tmp, chaine, vp);
                floatprocessing=false;
                floatprocessinginit=false;
                compile_expression=false;
                
                if (!TAG_secondary_vars) close_temp_register();
                
                if (strcmp(aff, chaine) != 0)
                {
                    _sprintf(c.str, "%s = %s", chaine, aff);
                    AddInst(vp, c);
                }

				if (isdefined(chaine)<0) registerdefine(chaine);
			}
		}
		else
		{
			tag_result_present=((str_cmp_match(&chaine[p+1],chaine))||(register_writeonly(chaine)));

			NoDepthOut=false;
			if (strcmp(chaine,"Out")==0) NoDepthOut=true;
            if (strcmp(chaine,"Depth")==0) NoDepthOut=true;
            
			if (isfloat(chaine)) floatprocessing=true;
            
            if (str_char(chaine,'.')>=0)
            {
                if (!CorrectModifiers(chaine)) TYPEERROR=true;
            }

			res = var(chaine);
			if (res[0]=='n') res=var(chaine);

			if (!TAG_secondary_vars) last_temp_register=last_register;
			else last_temp_register=last_register_macro;

			if ((last_line_pixelshader==0)||(shadermodel))
			{
				nbmodifiersprocessing=nb_modifiers(res);
				
				compile_expression=true;
				aff = compile(&chaine[p + 1], res, vp);
				compile_expression=false;


				if (!TAG_secondary_vars) close_temp_register();

				if (strcmp(aff, res) != 0)
				{
					//if (!((str_char(aff,'.')==-1)&&(str_char(res,'.')!=-1)))
					{
						if (api == 2)
						{
							em->Add(OP_MOV, res, aff, NULL);
						}
						else
						{
							if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
							{
								_sprintf(c.str, "%s = %s", res, aff);
								AddInst(vp, c);
							}
							else
							{
								_sprintf(c.str, "%s %s,%s", Inst[_mov][api], res, aff);
								AddInst(vp, c);
							}
						}
					}
				}

				if (isdefined(chaine)<0) registerdefine(chaine);
			}
			else
			{
				if (!shadermodel)
				{
					compile_expression=true;
					aff=compile(&chaine[p+1],"result.color",vp);
					compile_expression=false;
					close_temp_register();

					if (strcmp(aff,"result.color")!=0)
					{
						if (api!=2)
						{
							_sprintf(c.str,"%s %s,%s",Inst[_mov][api],"result.color",aff);
							AddInst(vp,c);
						}
					}
				}
			}

			floatprocessing=false;
		}
	}
	else
	{
		strcpy(ERRORSTR,str);
		SYNTAXERROR=true;
	}

	if (!shadermodel)
	{
		if ((api==0)&&(strcmp(str,"phase")==0))
		{
            strcpy(c.str,"phase");
			AddInst(vp,c);
			phase=1;
		}
	}
    
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::compile_init_variables(char *str,CList <Code> *vp)
{
	char chaine[1024];
	int p;
	char * res;
    
    if (str_mm(str,"#if")) return;
    if (str_mm(str,"#else")) return;
    if (str_mm(str,"#endif")) return;

	if (str_char(str,'=')!=-1)
	{
        strcpy(chaine,str);
		str_clean(chaine);

		p=str_char(chaine,'=');
		chaine[p]='\0';

		if (!str_match(&chaine[p+1],"mat"))
		{
            strcpy(chaine,str);
			str_clean(chaine);
			p=str_char(chaine,'=');
			if ((chaine[p - 1] == '-') || (chaine[p - 1] == '+')) p--;
			chaine[p]='\0';
			strcpy(LASTLINESTR,str);
			res=var(chaine,1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int str_operateur_moins(char * chaine,char ** str0,char **str1)
{
	int n,res;
	char *s0,*s1;

	s0=newString();
	s1=newString();

	*str0=s0;
	*str1=s1;

	n=0;
	res=0;
	while ((res==0)&&(n<(int) strlen(chaine)))
	{
		if (chaine[n]=='-')
		{
			str_cut(chaine,0,n,s0);
			str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

			if ((str_valid(s0))&&(str_valid(s1))) res=1;
			else n++;
		}
		else
		if (chaine[n]=='+')
		{
			str_cut(chaine,0,n,s0);
			str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

			if ((str_valid(s0))&&(str_valid(s1))) res=2;
			else n++;
		}
		else n++;
	}

	return res;
}

int multiple_argument_parse_virg(char *str,char **strings)
{
	char *tmp;
	int pos=0;
	int n=0;

	if (strlen(str)==0) return 0;

	tmp=str_parse_char_prt(str,&pos,',');
	while (tmp)
	{
		strings[n]=newString();
		_sprintf(strings[n++],"%s",tmp);
		tmp=str_parse_char_prt(str,&pos,',');
	}
    
    return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::test_force_register(char *str)
{
	int t;
	char chaine[1024];
	char *rn;
	char tmp[1024];
	int p;

	if (strcmp(str,"")==0) return NULL;
	else
	{
		if (str_parentheses(str)) _sprintf(chaine,"%s",str_return_parentheses(str));
		else _sprintf(chaine,"%s",str);

		if (str_match0(chaine,"sample"))
		{
			_sprintf(tmp,"%s",str_return_parentheses(chaine));
			p=str_char(tmp,',');
			tmp[p]='\0';

			sscanf(tmp,"%d",&t);
			if (api==0)
			{
				if (t==0) rn="r0";
				if (t==1) rn="r1";
				if (t==2) rn="r2";
				if (t==3) rn="r3";
				if (t==4) rn="r4";
				if (t==5) rn="r5";
			}
			else rn=NULL;

			return rn;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::nb_modifiers_syntax(char * tmp)
{
	if (!numb(tmp))
	{
		if (isfloat(tmp)) return 1;
		else
		{
			int sc = str_last_char(tmp, '.');
			if (sc != -1)
			{
				if (tmp[sc+1]!='v') return (int)strlen(&tmp[sc + 1]);
				else return 0;
			}
			else return 0;
		}
	}
	else return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::nb_modifiers(char * tmp)
{
	if (!numb(tmp))
	{
		int sc = str_last_char(tmp, '.');
		if (sc != -1)
		{
			if (tmp[sc+1]!='v') return (int)strlen(&tmp[sc + 1]);
			else return 4;
		}
		else return 4;
	}
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::swizzle(char *output, char *tmp4)
{
	if ((numberswz(output))!=numberswz(tmp4))
	{
		if (!numb(tmp4))
		{
			int scc = str_last_char(tmp4, '.');

			if (scc == -1)
			{
				int sc = str_last_char(output, '.');
				if (sc != -1)
				{
					if (output[sc+1]!='v')
					{
						switch ((int)strlen(&output[sc + 1]))
						{
						case 1:
							_sprintf(tmp4, "%s.x", tmp4);
							break;
						case 2:
							_sprintf(tmp4, "%s.xy", tmp4);
							break;
						case 3:
							_sprintf(tmp4, "%s.xyz", tmp4);
							break;
						case 4:
							//_sprintf(tmp4, "%s.xyzw", tmp4);
							break;
						};
					}
					//else _sprintf(tmp4, "%s.xyzw", tmp4);
				}
			}
			else
			{
				if ((int)strlen(&tmp4[scc + 1])==1)
				{
					char c=tmp4[scc+1];
					char tmp[1024];
					_sprintf(tmp,tmp4);
					tmp[scc]='\0';

					int sc = str_last_char(output, '.');
					if (sc != -1)
					{
						if (output[sc+1]!='v')
						{
							switch ((int)strlen(&output[sc + 1]))
							{
							case 1:
								_sprintf(tmp4, "%s.%c", tmp,c);
								break;
							case 2:
								_sprintf(tmp4, "%s.%c%c", tmp,c,c);
								break;
							case 3:
								_sprintf(tmp4, "%s.%c%c%c", tmp,c,c,c);
								break;
							case 4:
								_sprintf(tmp4, "%s.%c%c%c%c", tmp,c,c,c,c);
								break;
							};
						}
						else _sprintf(tmp4, "%s.%c%c%c%c", tmp,c,c,c,c);
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::resfield(char *output, char *tmp4)
{
	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(output, '.');
		if (sc != -1)
		{
			if (output[sc+1]=='v')
			{
			}
			else
			{
				switch ((int)strlen(&output[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				};
			}
		}
		else
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],output)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::outputfield1(char *tmp4, char *output, char *rn)
{
	_sprintf(tmp4, output);

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn, '.');
		if ((sc != -1) && (!numb(rn)))
		{
			if (rn[sc+1]=='v')
			{
				//_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}
	if ((str_last_char(tmp4, '.') == -1) && (numb(rn))) _sprintf(tmp4, "%s.x", tmp4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::outputfield2(char *tmp4, char *output, char *rn, char *rn2)
{
	_sprintf(tmp4, output);

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn, '.');
		if ((sc != -1) && (!numb(rn)))
		{
			if (rn[sc+1]=='v')
			{
				_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn2, '.');
		if ((sc != -1) && (!numb(rn2)))
		{
			if (rn2[sc+1]=='v')
			{
				//_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn2[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn2))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn2)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}
	if ((str_last_char(tmp4, '.') == -1) && (numb(rn))) _sprintf(tmp4, "%s.x", tmp4);
	if ((str_last_char(tmp4, '.') == -1) && (numb(rn2))) _sprintf(tmp4, "%s.x", tmp4);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::outputfield3(char *tmp4, char *output, char *rn, char *rn2,char *rn3)
{
	_sprintf(tmp4, output);

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn, '.');
		if ((sc != -1) && (!numb(rn)))
		{
			if (rn[sc+1]=='v')
			{
				//_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn2, '.');
		if ((sc != -1) && (!numb(rn2)))
		{
			if (rn2[sc+1]=='v')
			{
				//_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn2[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn2))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn2)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}

	}

	if (str_last_char(tmp4, '.') == -1)
	{
		int sc = str_last_char(rn3, '.');
		if ((sc != -1) && (!numb(rn3)))
		{
			if (rn3[sc+1]=='v')
			{
				_sprintf(tmp4, "%s.xyzw", tmp4);
			}
			else
			{
				switch ((int)strlen(&rn3[sc + 1]))
				{
				case 1:
					_sprintf(tmp4, "%s.x", tmp4);
					break;
				case 2:
					_sprintf(tmp4, "%s.xy", tmp4);
					break;
				case 3:
					_sprintf(tmp4, "%s.xyz", tmp4);
					break;
				case 4:
					//_sprintf(tmp4, "%s.xyzw", tmp4);
					break;
				};
			}
		}
		else
		if (!numb(rn3))
		{
			int present=-1;
			for (int kk=0;kk<nfloats;kk++)
				if (strcmp(floats[kk],rn3)==0) present=kk;

			if (present>=0) _sprintf(tmp4, "%s.x", tmp4);
		}
	}

	if ((str_last_char(tmp4, '.') == -1) && (numb(rn))) _sprintf(tmp4, "%s.x", tmp4);
	if ((str_last_char(tmp4, '.') == -1) && (numb(rn2))) _sprintf(tmp4, "%s.x", tmp4);
	if ((str_last_char(tmp4, '.') == -1) && (numb(rn3))) _sprintf(tmp4, "%s.x", tmp4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::isfloat(char * rn)
{
	if (!numb(rn))
	{
		int present=-1;
		for (int kk=0;kk<nfloats;kk++)
			if (strcmp(floats[kk],rn)==0) present=kk;

		if (present>=0) return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::numberswz(char *op)
{
	if (numb(op)) return 1;
	if (isfloat(op)) return 1;

	for (int n=0;n<4;n++)
	if (strcmp(&paramsFn[n][0],op)==0)
	{
		if (defsFn[n]==CALL_FLOAT) return 1;
		if (defsFn[n]==CALL_VECTOR2) return 2;
		if (defsFn[n]==CALL_VECTOR3) return 3;
		if (defsFn[n]==CALL_IOFLOAT) return 1;
		if (defsFn[n]==CALL_IOVECTOR2) return 2;
		if (defsFn[n]==CALL_IOVECTOR3) return 3;
		return 4;
	}

	int p=str_char(op,'(');
	if (p==-1)
	{	
		int sc=str_last_char(op,'.');
		if (op[sc+1]=='v') return 4;
		else
		{	
			if (sc>=0) return (int)strlen(&op[sc+1]);
			else return 4;
		}
	}
	else
	{
        if ((op[0]=='f')&&(op[1]=='l')&&(op[2]=='o')&&(op[3]=='a')&&(op[4]=='t')&&(op[5]=='(')&&(op[strlen(op)-1]==')'))
        {
            char tmpop[128];
            strcpy(tmpop,&op[p+1]);
            p=str_char(tmpop,')');
            tmpop[p]=0;
            int sc=str_last_char(tmpop,'.');
            if (tmpop[sc+1]=='v') return 4;
            else
            {
                if (sc>=0) return (int)strlen(&tmpop[sc+1]);
                else return 4;
            }
        }
        else
        {
            int sc=str_last_char(op,'.');
            if (op[sc+1]=='v') return 4;
            else
            {
                if (sc>=0) return (int)strlen(&op[sc+1]);
                else return 4;
            }
        }
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::getsmp(char *stex)
{
	for (int n=0;n<16;n++)
		if (strcmp(stex,texture[n])==0) return n;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::correct_modifier_texture2d(char * reg)
{
	if (isfloat(reg)) return true;

	int p=str_last_char(reg,'.');
	if (p==-1) return true;

	p++;
	int len=(int)strlen(&reg[p]);
	if (len!=2) return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::compile(char *str,char *outputbase,CList <Code> *vp)
{
	int n;
	int t;
	char *rns[8];
	char s0[8192];
	char s1[8192];
	char chaine[8192];
	char *tmp2;
	char *tmp3;
	char *tmp4;
	char *tmp5;
	char *rn,*rn2,*rn3,*rn4;
	char *rn1=s0;
	char *r0=s0;
	char *r1=s1;
	int p1,p2,p3;
	char *tmp;
	char *tmp0;
	int p,res;
	int value;
	const char ch[4]={ 'a','b','c','d' };
	char *output=outputbase;


	int lln=strlen(str);
	int pr1=0;
	int pr2=0;
	for (n=0;n<lln;n++)
	{
		if (str[n]=='(') pr1++;
		if (str[n]==')') pr2++;
	}

	if ((pr1!=pr2)||(lln==0)) SYNTAXERROR=true;

	if (floatprocessing)
		if ((!isfloat(output))&&(str_last_char(output,'.')<0)) _sprintf(output,"%s.x",output);

    if (nb_modifiers_syntax(output)>4) TYPEERROR=true;
    
	tmp=newString();
	tmp0=newString();
	tmp2=newString();
	tmp3=newString();
	tmp4=newString();
	tmp5=newString();

/*
	char ssz[1024];
	_sprintf(ssz,"%s : %s",output,str);
	LOG(ssz);
/**/
        
	if (strcmp(str,"")==0) return "null";
	else
	{
		if (str_char(str,'=')>=0) SYNTAXERROR=true;

		if (str_parentheses(str)) strcpy(chaine,str_return_parentheses(str));
		else strcpy(chaine,str);

		if ((str_char(chaine,'+')==-1)&&(str_char(chaine,'^')==-1)&&(str_char(chaine,'-')==-1)&&(str_char(chaine,'*')==-1)&&(str_char(chaine,'/')==-1)&&(str_char(chaine,'|')==-1))
		{
			goto _functions_test;
		}
		else
		{
			n=0;
			res=0;
			while ((res==0)&&(n<(int) strlen(chaine)))
			{
				if (chaine[n]=='-')
				{
					str_cut(chaine,0,n,s0);
					str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

					if ((str_valid(s0))&&(str_valid(s1))) res=1;
					else n++;
				}
				else
				if (chaine[n]=='+')
				{
					str_cut(chaine,0,n,s0);
					str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

					if ((str_valid(s0))&&(str_valid(s1))) res=2;
					else n++;
				}
				else n++;
			}

			if (res!=0)
			{
				if (str_simple(s1))
				{
					if (str_simple(s0)) rn=var(s0);
					else
					{
						if (str_last_char(output,'.')==-1) _sprintf(tmp,"r%d",new_temp_register());
						else
						{
							switch (nb_modifiers(output))
							{
							case 1:
								_sprintf(tmp,"r%d.x",new_temp_register());
								break;
							case 2:
								_sprintf(tmp,"r%d.xy",new_temp_register());
								break;
							case 3:
								_sprintf(tmp,"r%d.xyz",new_temp_register());
								break;
							case 4:
								_sprintf(tmp,"r%d",new_temp_register());
								break;
							};
						}
						rn=compile(s0,tmp,vp);
						t=1;
					}

					rn2=var(s1);

					if (numberswz(rn)!=numberswz(rn2)) TYPEERROR=true;
					
					_sprintf(tmp4,output);
					
					if (str_last_char(tmp4,'.')==-1)
					{
						int sc = str_last_char(rn, '.');
						if ((sc != -1) && (!numb(rn)) )
						{
							switch (nb_modifiers(rn))
							{
							case 1:
								_sprintf(tmp4, "%s.x", output);
								break;
							case 2:
								_sprintf(tmp4, "%s.xy", output);
								break;
							case 3:
								_sprintf(tmp4, "%s.xyz", output);
								break;
							case 4:
								_sprintf(tmp4, "%s", output);
								break;
							};
						}
					}

					if (str_last_char(tmp4,'.')==-1)
					{
						int sc=str_last_char(rn2,'.');
						if ((sc!=-1)&&(!numb(rn2)))
						{
							switch (nb_modifiers(rn2))
							{
							case 1:
								_sprintf(tmp4,"%s.x",output);
								break;
							case 2:
								_sprintf(tmp4,"%s.xy",output);
								break;
							case 3:
								_sprintf(tmp4,"%s.xyz",output);
								break;
							case 4:
								_sprintf(tmp4,"%s",output);
								break;
							};
						}						
					}
					
					if (floatprocessing)
						if (isfloat(output)) strcpy(tmp4,output);

					if (res==1)
					{					
						if (api==2)
						{
							em->Add(OP_SUB,tmp4,rn,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								_sprintf(c.str,"%s = %s - %s",tmp4,rn,rn2);
								AddInst(vp,c);
							}
							else
							{
								_sprintf(c.str,"%s %s,%s,%s",Inst[_sub][api],tmp4,rn,rn2);
								AddInst(vp,c);
							}
						}
					}
					else
					{
						if (api==2)
						{
							em->Add(OP_ADD,tmp,rn,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								_sprintf(c.str,"%s = %s + %s",tmp4,rn,rn2);
								AddInst(vp,c);
							}
							else
							{
							
								if ((pixelshader)&&(api==1))
								{
									_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_add][api],tmp4,rn,rn2);
									AddInst(vp,c);
								}
								else
								{	
									_sprintf(c.str,"%s %s,%s,%s",Inst[_add][api],tmp4,rn,rn2);
									AddInst(vp,c);
								}	
							}
						}
					}

					return tmp4;
				}
				else
				{

					if (tag_result_present==false)
					{
						if (str_last_char(output,'.')==-1)
						{
							_sprintf(tmp2,"r%d",new_temp_register());
							_sprintf(tmp3,"%s",output);
						}
						else
						{
							int sc=str_last_char(output,'.');
							if (sc!=-1)
							{
								switch (nb_modifiers(output))
								{
								case 1:
									_sprintf(tmp2,"r%d.x",new_temp_register());
									break;
								case 2:
									_sprintf(tmp2,"r%d.xy",new_temp_register());
									break;
								case 3:
									_sprintf(tmp2,"r%d.xyz",new_temp_register());
									break;
								case 4:
									_sprintf(tmp2,"r%d",new_temp_register());
									break;
								};
							}
							else _sprintf(tmp2,"r%d",new_temp_register());

							_sprintf(tmp3,"%s",output);
						}
						
					}
					else
					{
						if (str_last_char(output,'.')==-1)
						{
							_sprintf(tmp2,"r%d",new_temp_register());
							_sprintf(tmp3,"r%d",new_temp_register());
						}
						else
						{
							int sc=str_last_char(output,'.');
							if (sc!=-1)
							{
								switch (nb_modifiers(output))
								{
								case 1:
									_sprintf(tmp2,"r%d.x",new_temp_register());
									_sprintf(tmp3,"r%d.x",new_temp_register());
									break;
								case 2:
									_sprintf(tmp2,"r%d.xy",new_temp_register());
									_sprintf(tmp3,"r%d.xy",new_temp_register());
									break;
								case 3:
									_sprintf(tmp2,"r%d.xyz",new_temp_register());
									_sprintf(tmp3,"r%d.xyz",new_temp_register());
									break;
								case 4:
									_sprintf(tmp2,"r%d",new_temp_register());
									_sprintf(tmp3,"r%d",new_temp_register());
									break;
								};
							}
							else
							{
								_sprintf(tmp2,"r%d",new_temp_register());
								_sprintf(tmp3,"r%d",new_temp_register());
							}
						}
					}

					_sprintf(tmp4,"%s",output);

					if (floatprocessing)
					{
						if (isfloat(output)) strcpy(tmp4,output);

						if ((!isfloat(tmp2))&&(str_last_char(tmp2,'.')<0)) _sprintf(tmp2,"%s.x",tmp2);

						if ((!isfloat(tmp3))&&(str_last_char(tmp3,'.')<0)) _sprintf(tmp3,"%s.x",tmp3);
					}

					if (str_simple(s0))
					{
						rn=var(s0);
						if (api==2)
						{
							em->Add(OP_MOV,tmp3,rn,NULL);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								
								if ((numb(rn))&&(str_last_char(tmp3,'.')==-1)) _sprintf(tmp3, "%s.x", tmp3);
								_sprintf(c.str,"%s = %s",tmp3,rn);
								AddInst(vp,c);
							}
							else
							{
								_sprintf(c.str,"%s %s,%s",Inst[_mov][api],tmp3,rn);
								AddInst(vp,c);
							}
						}
					}
					else
					{
						rn=compile(s0,tmp3,vp);
						t=1;
					}

					int temp_regs=last_temp_register;

					int prev_res=res;

					char *stmp0,*stmp1;
					char *prev=s1;

					res=str_operateur_moins(s1,&stmp0,&stmp1);

					while (res!=0)
					{
						if (str_simple(stmp0)) rn=var(stmp0);
						else
						{
							rn=compile(stmp0,tmp2,vp);
						}

						last_temp_register=temp_regs;

						if (numberswz(rn)!=numberswz(tmp3)) TYPEERROR=true;

						if (prev_res==1)
						{
							if (api==2)
							{
								em->Add(OP_SUB,tmp3,tmp,rn);
							}
							else
							{
								if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
								{
									_sprintf(c.str,"%s = %s - %s",tmp3,tmp3,rn);
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"%s %s,%s,%s",Inst[_sub][api],tmp3,tmp3,rn);
									AddInst(vp,c);
								}
							}
						}
						else
						{
							if (api==2)
							{
								em->Add(OP_ADD,tmp3,tmp,rn);
							}
							else
							{
								if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
								{
									_sprintf(c.str,"%s = %s + %s",tmp3,tmp3,rn);
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"%s %s,%s,%s",Inst[_add][api],tmp3,tmp3,rn);
									AddInst(vp,c);
								}
							}
						}

						prev=stmp1;
						prev_res=res;
						res=str_operateur_moins(stmp1,&stmp0,&stmp1);
					}

					if (prev_res==1)
					{
						if (str_simple(prev)) rn2=var(prev);
						else
						{
							rn2=compile(prev,tmp2,vp);
						}

						last_temp_register=temp_regs;

						if (numberswz(rn2)!=numberswz(tmp3)) TYPEERROR=true;

						if (api==2)
						{
							em->Add(OP_SUB,tmp3,tmp3,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								_sprintf(c.str,"%s = %s - %s",tmp4,tmp3,rn2);
								AddInst(vp,c);
							}
							else
							{
								if ((pixelshader)&&(api==1))
								{
									_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_sub][api],tmp4,tmp3,rn2);
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"%s %s,%s,%s",Inst[_sub][api],tmp4,tmp3,rn2);
									AddInst(vp,c);
								}	
							}
						}
					}

					if (prev_res==2)
					{
						if (str_simple(prev)) rn2=var(prev);
						else
						{
							rn2=compile(prev,tmp2,vp);
						}

						last_temp_register=temp_regs;

						if (numberswz(rn2)!=numberswz(tmp3)) TYPEERROR=true;
						if (numberswz(output)!=numberswz(tmp3)) TYPEERROR=true;
						if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;

						if (api==2)
						{
							em->Add(OP_ADD,tmp3,tmp3,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								_sprintf(c.str,"%s = %s + %s",tmp4,tmp3,rn2);
								AddInst(vp,c);
							}
							else
							{	
								if ((pixelshader)&&(api==1))
								{
									_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_add][api],tmp4,tmp3,rn2);
									AddInst(vp,c);
								}
								else
								{	
									_sprintf(c.str,"%s %s,%s,%s",Inst[_add][api],tmp4,tmp3,rn2);
									AddInst(vp,c);
								}	
							}
						}
					}					

					return tmp4;
				}
			}
			else
			{
				n=0;
				res=0;
				while ((res==0)&&(n<(int) strlen(chaine)))
				{
					bool b4=((chaine[n]=='*')&&(chaine[n+1]=='4')&&(!chiffre(chaine[n+2]))&&(strlen(&chaine[n+2])>0)&&(chaine[n+2]!='*')&&(chaine[n+2]!='/'));
					bool b3=((chaine[n]=='*')&&(chaine[n+1]=='3')&&(!chiffre(chaine[n+2]))&&(strlen(&chaine[n+2])>0)&&(chaine[n+2]!='*')&&(chaine[n+2]!='/'));
					bool b2=((chaine[n]=='*')&&(chaine[n+1]=='2')&&(!chiffre(chaine[n+2]))&&(strlen(&chaine[n+2])>0)&&(chaine[n+2]!='*')&&(chaine[n+2]!='/'));

					if (chaine[n]=='/')
					{
						str_cut(chaine,0,n,s0);
						str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

						if ((str_valid(s0))&&(str_valid(s1))) res=1;
						else n++;
					}
					else
					if ((chaine[n]=='*')&&(!b2)&&(!b3)&&(!b4))
					{
						str_cut(chaine,0,n,s0);
						str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

						if ((str_valid(s0))&&(str_valid(s1))) res=2;
						else n++;

					}
					else
					if (chaine[n]=='|')
					{
						str_cut(chaine,0,n,s0);
						str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

						if ((str_valid(s0))&&(str_valid(s1))) res=3;
						else n++;
					}
					else
					if (chaine[n]=='^')
					{
						str_cut(chaine,0,n,s0);
						str_cut(chaine,n+1,(int)strlen(chaine)-(n+1),s1);

						if ((str_valid(s0))&&(str_valid(s1))) res=6;
						else n++;

					}
					else
					if (n<(int)strlen(chaine)-1)
					{
						if ((chaine[n]=='*')&&(chaine[n+1]=='4')&&(!chiffre(chaine[n+2])))
						{
							str_cut(chaine,0,n,s0);
							str_cut(chaine,n+2,(int)strlen(chaine)-(n+2),s1);

							if ((str_valid(s0))&&(str_valid(s1))) res=4;
							else n++;

						}
						else
						if ((chaine[n]=='*')&&(chaine[n+1]=='3')&&(!chiffre(chaine[n+2])))
						{
							str_cut(chaine,0,n,s0);
							str_cut(chaine,n+2,(int)strlen(chaine)-(n+2),s1);

							if ((str_valid(s0))&&(str_valid(s1))) res=5;
							else n++;

						}
						else
						if ((chaine[n]=='*')&&(chaine[n+1]=='2')&&(!chiffre(chaine[n+2])))
						{
							str_cut(chaine,0,n,s0);
							str_cut(chaine,n+2,(int)strlen(chaine)-(n+2),s1);

							if ((str_valid(s0))&&(str_valid(s1))) res=7;
							else n++;

						}
						else n++;
					}
					else n++;
				}

				if (res!=0)
				{
					if (res==3)
					{
						t=0;
						if (str_simple(s0))
						{
							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());

								rn2=compile(s1,tmp,vp);
								t=1;
							}
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());
								rn=compile(s0,tmp,vp);
								t=1;
							}

							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp2,"r%d",new_temp_register());
								rn2=compile(s1,tmp2,vp);
								t=1;
							}
						}

						if (api==2)
						{
							em->Add(OP_DP3,output,rn,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								if (str_last_char(output,'.')==-1)
								{
									_sprintf(tmp3,"r%d",new_temp_register());
									_sprintf(c.str,"%s.x = dot(%s.xyz,%s.xyz)",tmp3,rn,rn2);
									AddInst(vp,c);
									_sprintf(c.str,"%s = %s.xxxx",output,tmp3);
									AddInst(vp,c);

									return output;
								}
								else
								{
									_sprintf(c.str,"%s = dot(%s.xyz,%s.xyz)",output,rn,rn2);
									AddInst(vp,c);
								}
							}
							else
							{
								if ((pixelshader)&&(api==1))
								{
									_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_dp3][api],output,rn,rn2);
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"%s %s,%s,%s",Inst[_dp3][api],output,rn,rn2);
									AddInst(vp,c);
								}
							}
						}

						return output;
					}

					if (res==6)
					{
						t=0;
						if (str_simple(s0))
						{
							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());

								rn2=compile(s1,tmp,vp);
								t=1;
							}
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());

								rn=compile(s0,tmp,vp);
								t=1;
							}

							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp3,"r%d",new_temp_register());

								rn2=compile(s1,tmp3,vp);
								t=1;
							}
						}

						if ((strcmp(output,rn)!=0)&&(strcmp(output,rn2)!=0)) tmp0=output;
						else _sprintf(tmp0,"r%d",new_temp_register());

						_sprintf(tmp2,"r%d",new_temp_register());

						if (api==2)
						{
							em->Add(OP_CROSS,tmp,rn,rn2);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
								_sprintf(c.str,"%s.xyz = cross( %s.xyz , %s.xyz )",output,rn,rn2);
								AddInst(vp,c);
								_sprintf(c.str,"%s.w = 0.0",output);
								AddInst(vp,c);
								return output;
							}
							else
							{
								_sprintf(c.str,"%s %s,%s.yzxw,%s.zxyw",Inst[_mul][api],tmp2,rn,rn2);
								AddInst(vp,c);
								_sprintf(c.str,"%s %s,-%s.zxyw,%s.yzxw,%s",Inst[_mad][api],tmp0,rn,rn2,tmp2);
								AddInst(vp,c);
							}
						}

						return tmp0;
					}

					if (res==2)
					{
						int sco=str_last_char(output,'.');

						if (sco!=-1) _sprintf(tmp0,"%s",&output[sco]);
						else _sprintf(tmp0,"");
						
						t=0;
						if (str_simple(s0))
						{
							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp,"r%d%s",new_temp_register(),tmp0);
								rn2=compile(s1,tmp,vp);
								t=1;
							}
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								_sprintf(tmp,"r%d%s",new_temp_register(),tmp0);
								rn=compile(s0,tmp,vp);
								t=1;
							}

							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp2,"r%d%s",new_temp_register(),tmp0);
								rn2=compile(s1,tmp2,vp);
								t=1;
							}
						}

						int map=0;

						//MUL

						if (((str_last_char(rn,'.')>=0)&&(str_last_char(rn2,'.')>=0))&&(sco<0))
						{
							int sc0=str_last_char(rn,'.');
							int sc1=str_last_char(rn2,'.');
							int sz0=(int)strlen(&rn[sc0+1]);
							int sz1=(int)strlen(&rn2[sc1+1]);
							int sz=sz0;
							if (sz1>sz) sz=sz1;
							if (sz==1) map=1;
						}

						if (api==2) em->Add(OP_MUL,output,rn,rn2);
						else
						{
							bool bn=(str_last_char(rn,'.')==-1)||((str_last_char(rn,'.')!=-1)&&(numb(rn)));
							bool bn2=(str_last_char(rn2,'.')==-1)||((str_last_char(rn2,'.')!=-1)&&(numb(rn2)));

							if (bn&&bn2)
							{
								if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
								{
									_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
									AddInst(vp,c);
								}
								else
								{
									if (rn[0]=='c')
									{
										_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
										AddInst(vp,c);
									}
									else
									{
										if ((pixelshader)&&(api==1))
										{
											_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
											AddInst(vp,c);
										}
										else 
										{
											_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
											AddInst(vp,c);
										}	
									}
								}
								
								if (numberswz(rn)!=numberswz(rn2))
								{
									if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
									else
									{
										if (numberswz(rn)!=1)
											if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

										if (numberswz(rn2)!=1)
											if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
									}
								}

								return output;
							}
							else
							{
								if ((str_last_char(rn,'.')!=-1)&&(str_last_char(rn2,'.')!=-1)&&(str_last_char(output,'.')==-1))
								{
									if ((numb(rn))&&(numb(rn2)))
									{
										if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
										{
											_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
											AddInst(vp,c);
										}
										else
										{
											if (rn[0]=='c')
											{
												_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if ((pixelshader)&&(api==1))
												{
													_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
													AddInst(vp,c);
												}
												else 
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
													AddInst(vp,c);
												}	
											}
										}
										
										if (numberswz(rn)!=numberswz(rn2))
										{
											if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
											else
											{
												if (numberswz(rn)!=1)
													if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

												if (numberswz(rn2)!=1)
													if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
											}
										}

										return output;
									}
									else
									if (numb(rn))
									{
										if (str_last_char(output,'.')==-1)
										{
											int sc=str_last_char(rn2,'.');

											if (sc!=-1)
											{
												switch (nb_modifiers(rn2))
												{
												case 1:
													_sprintf(tmp4,"%s.x",output);
													break;
												case 2:
													_sprintf(tmp4,"%s.xy",output);
													break;
												case 3:
													_sprintf(tmp4,"%s.xyz",output);
													break;
												case 4:
													_sprintf(tmp4,"%s",output);
													break;
												};

											}
											else _sprintf(tmp4,"%s",output);


											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",tmp4,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(tmp4)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(tmp4)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											return tmp4;
										}
										else
										{
											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											return output;
										}
									}
									else
									if (numb(rn2))
									{
										if (str_last_char(output,'.')==-1)
										{
											int sc=str_last_char(rn,'.');

											if (sc!=-1)
											{
												switch (nb_modifiers(rn))
												{
												case 1:
													_sprintf(tmp4,"%s.x",output);
													break;
												case 2:
													_sprintf(tmp4,"%s.xy",output);
													break;
												case 3:
													_sprintf(tmp4,"%s.xyz",output);
													break;
												case 4:
													_sprintf(tmp4,"%s",output);
													break;
												};

											}
											else _sprintf(tmp4,"%s",output);


											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",tmp4,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(tmp4)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(tmp4)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											return tmp4;
										}
										else
										{

											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											return output;
										}
									}
									else
									{
										if (str_last_char(output,'.')==-1)
										{
											int sc=str_last_char(rn,'.');
											int sc2=str_last_char(rn2,'.');
												
											if (sc>=0) _sprintf(tmp2,"%s",&rn[sc]);
											else _sprintf(tmp2,"");

											if (sc2>=0) _sprintf(tmp3,"%s",&rn2[sc2]);
											else _sprintf(tmp3,"");

											if (map==0)
											{
												if (numberswz(rn)!=numberswz(rn2))
												{
													if (strlen(tmp2)>strlen(tmp3))
													{
														if (strlen(tmp2)<5) _sprintf(tmp4,"%s%s",output,tmp2);
														else _sprintf(tmp4,output);
													}
													else
													{
														if (strlen(tmp2)<5) _sprintf(tmp4,"%s%s",output,tmp3);
														else _sprintf(tmp4,output);
													}
												}
												else _sprintf(tmp4,output);
											}
											else _sprintf(tmp4,"%s.x",output);

											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",tmp4,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp4,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(tmp4)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(tmp4)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											if (map)
											{
												if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
												{
													_sprintf(c.str,"%s = %s.xxxx",output,output);
													AddInst(vp,c);
												}
												return output;
											}
											return tmp4;
										}
										else
										{
											if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
											{
												_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
												AddInst(vp,c);
											}
											else
											{
												if (rn[0]=='c')
												{
													_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
													AddInst(vp,c);
												}
												else
												{
													if ((pixelshader)&&(api==1))
													{
														_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}
													else 
													{
														_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
														AddInst(vp,c);
													}	
												}
											}

											if (numberswz(rn)!=numberswz(rn2))
											{
												if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
												else
												{
													if (numberswz(rn)!=1)
														if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

													if (numberswz(rn2)!=1)
														if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
												}
											}

											return output;
										}
									}
								}
								else
								{
									_sprintf(tmp3,rn);
									_sprintf(tmp4,rn2);
									if (!floating(tmp3)) swizzle(output,tmp3);
									if (!floating(tmp4)) swizzle(output,tmp4);

									if( ((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
									{
										_sprintf(c.str,"%s = %s * %s",output,tmp3,tmp4);
										AddInst(vp,c);
									}
									else
									{
										if (rn[0]=='c')
										{
											_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
											AddInst(vp,c);
										}
										else
										{
											if ((pixelshader)&&(api==1))
											{
												_sprintf(c.str,"%s_SAT %s,%s,%s",Inst[_mul][api],output,rn,rn2);
												AddInst(vp,c);
											}
											else 
											{
												_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn2);
												AddInst(vp,c);
											}	
										}
									}

									if (numberswz(rn)!=numberswz(rn2))
									{
										if ((numberswz(rn)!=1)&&(numberswz(rn2)!=1)) TYPEERROR=true;
										else
										{
											if (numberswz(rn)!=1)
												if (numberswz(output)!=numberswz(rn)) TYPEERROR=true;

											if (numberswz(rn2)!=1)
												if (numberswz(output)!=numberswz(rn2)) TYPEERROR=true;
										}
									}
									
									return output;
								}
							}
						}

						return output;
					}

					if (res==1)
					{
						divprocessing=true;

						t=0;
						if (str_simple(s0))
						{
							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp,"r%d.x",new_temp_register());
								/*
								if (str_last_char(output,'.')==-1) _sprintf(tmp,"r%d",new_temp_register());
								else
								{
									int sc=str_last_char(output,'.');
									if (sc!=-1)
									{
										switch (strlen(&output[sc+1]))
										{
										case 1:
											_sprintf(tmp,"r%d.x",new_temp_register());
											break;
										case 2:
											_sprintf(tmp,"r%d.xy",new_temp_register());
											break;
										case 3:
											_sprintf(tmp,"r%d.xyz",new_temp_register());
											break;
										case 4:
											_sprintf(tmp,"r%d",new_temp_register());
											break;
										};
									}
									else _sprintf(tmp,"r%d",new_temp_register());
								}
								/**/
								rn2=compile(s1,tmp,vp);
								t=1;
							}
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								if (str_last_char(output,'.')==-1) _sprintf(tmp,"r%d",new_temp_register());
								else
								{
									int sc=str_last_char(output,'.');
									if (sc!=-1)
									{
										switch (nb_modifiers(output))
										{
										case 1:
											_sprintf(tmp,"r%d.x",new_temp_register());
											break;
										case 2:
											_sprintf(tmp,"r%d.xy",new_temp_register());
											break;
										case 3:
											_sprintf(tmp,"r%d.xyz",new_temp_register());
											break;
										case 4:
											_sprintf(tmp,"r%d",new_temp_register());
											break;
										};
									}
									else _sprintf(tmp,"r%d",new_temp_register());
								}
								rn=compile(s0,tmp,vp);
								t=1;
							}

							if (str_simple(s1)) rn2=var(s1);
							else
							{
								_sprintf(tmp2,"r%d.x",new_temp_register());
								/*
								if (str_last_char(output,'.')==-1) _sprintf(tmp2,"r%d",new_temp_register());
								else
								{
									int sc=str_last_char(output,'.');
									if (sc!=-1)
									{
										switch (strlen(&output[sc+1]))
										{
										case 1:
											_sprintf(tmp2,"r%d.x",new_temp_register());
											break;
										case 2:
											_sprintf(tmp2,"r%d.xy",new_temp_register());
											break;
										case 3:
											_sprintf(tmp2,"r%d.xyz",new_temp_register());
											break;
										case 4:
											_sprintf(tmp2,"r%d",new_temp_register());
											break;
										};
									}
									else _sprintf(tmp2,"r%d",new_temp_register());
								}
								/**/
								rn2=compile(s1,tmp2,vp);
								t=1;
							}
						}

						divprocessing=false;

						if (api==2)
						{
							em->Add(OP_RCP,tmp,rn2,NULL);
							em->Add(OP_MUL,output,rn,tmp);
						}
						else
						{
							if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
							{
                                if (str_last_char(output,'.')==-1)
                                {
                                    if (str_last_char(rn,'.')==-1) _sprintf(tmp3,"%s",output);
                                    else
                                    {
                                        int sc=str_last_char(rn,'.');
                                        if (sc!=-1)
                                        {
                                            switch (strlen(&rn[sc+1]))
                                            {
                                            case 1:
                                                _sprintf(tmp3,"%s.x",output);
                                                break;
                                            case 2:
                                                _sprintf(tmp3,"%s.xy",output);
                                                break;
                                            case 3:
                                                _sprintf(tmp3,"%s.xyz",output);
                                                break;
                                            case 4:
                                                _sprintf(tmp3,"%s",output);
                                                break;
                                            };
                                        }
                                        else _sprintf(tmp3,"%s",output);
                                    }

                                    _sprintf(c.str,"%s = %s / %s",tmp3,rn,rn2);
                                    AddInst(vp,c);
                                    
                                    return tmp3;
                                }
                                else
                                {
                                    _sprintf(c.str,"%s = %s / %s",output,rn,rn2);
                                    AddInst(vp,c);
                                }
							}
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());
								_sprintf(c.str,"%s %s,%s.w",Inst[_rcp][api],tmp,rn2);
								AddInst(vp,c);
								_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,tmp);
								AddInst(vp,c);
							}
						}

						return output;
					}


					if (res==4)  // *4
					{
						
						t=0;
						if (str_simple(s0))
						{
							rn2=var(s1);
							rn=var(s0);
						}
						else
						{
							_sprintf(tmp,"r%d",new_temp_register());
							rn=compile(s0,tmp,vp);
							t=1;

							rn2=var(s1);
						}

						if (strcmp(output,rn)!=0) tmp=output;
						else _sprintf(tmp,"r%d",new_temp_register());

						if (api==2)
						{
							em->Add(OP_M4X4,tmp,rn,rn2);
						}
						else
						{

							if ((api==1)&&(shadermodel))
							{
								_sprintf(c.str,"%s = %s * %s",tmp,rn2,rn);
								AddInst(vp,c);
							}
							else
							if ((api==0)&&(shadermodel3))
							{
								if (metal==1)
								{
									if (strcmp(rn,"indexed.v0")==0) _sprintf(c.str,"%s = %s * float4(%s, 1.0)",tmp,rn2,rn);
									else _sprintf(c.str,"%s = %s * %s",tmp,rn2,rn);
								}
								else
								{
									_sprintf(c.str,"%s = mul(%s,%s)",tmp,rn,rn2);
								}
								AddInst(vp,c);
							}
							else
							{
								if (api)
								{
									str_translate_format(rn2,tmp2,&value);

									_sprintf(c.str,"DP4 %s.x,%s,%s%d]",tmp,rn,tmp2,value+0);
									AddInst(vp,c);
									_sprintf(c.str,"DP4 %s.y,%s,%s%d]",tmp,rn,tmp2,value+1);
									AddInst(vp,c);
									_sprintf(c.str,"DP4 %s.z,%s,%s%d]",tmp,rn,tmp2,value+2);
									AddInst(vp,c);
									_sprintf(c.str,"DP4 %s.w,%s,%s%d]",tmp,rn,tmp2,value+3);
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"m4x4 %s,%s,%s",tmp,rn,rn2);
									AddInst(vp,c);
								}
							}
						}

						return tmp;
					}

					if (res==5)	// *3
					{
						t=0;
						if (str_simple(s0))
						{
							rn2=var(s1);
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());
								rn=compile(s0,tmp,vp);
								t=1;
							}

							rn2=var(s1);
						}

						if (strcmp(output,rn)!=0) tmp=output;
						else _sprintf(tmp, "r%d", new_temp_register());

						if (api==2) em->Add(OP_M3X3,tmp,rn,rn2);
						else
						{
							if ((api==1)&&(shadermodel))
							{
								_sprintf(c.str,"%s = vec4(%s.xyz,0.0)",tmp,rn);
								AddInst(vp,c);
								_sprintf(c.str,"%s = %s  * %s",tmp,rn2,tmp);
								AddInst(vp,c);
								_sprintf(c.str,"%s.w = 0.0",tmp);
								AddInst(vp,c);
							}
							else
							if ((api==0)&&(shadermodel3))
							{
								if (metal==1)
								{
									_sprintf(c.str, "%s = float4( %s.xyz , 0.0f)", tmp,rn);
									AddInst(vp, c);
									_sprintf(c.str, "%s = %s * %s", tmp, rn2, tmp);
									AddInst(vp,c);
									_sprintf(c.str, "%s.w = 0.0f", tmp);
									AddInst(vp, c);
								}
								else
								{
									_sprintf(c.str, "%s = float4( %s.xyz , 0.0f)", tmp,rn);
									AddInst(vp, c);
									_sprintf(c.str, "%s = mul(%s,%s)", tmp, tmp, rn2);
									AddInst(vp,c);
									_sprintf(c.str, "%s.w = 0.0f", tmp);
									AddInst(vp, c);
								}
							}
							else
							{

								if (api)
								{
									str_translate_format(rn2,tmp2,&value);
									_sprintf(c.str,"DP3 %s.x,%s,%s%d]",tmp,rn,tmp2,value+0);
									AddInst(vp,c);
									_sprintf(c.str,"DP3 %s.y,%s,%s%d]",tmp,rn,tmp2,value+1);
									AddInst(vp,c);
									_sprintf(c.str,"DP3 %s.z,%s,%s%d]",tmp,rn,tmp2,value+2);
									AddInst(vp,c);
									_sprintf(c.str,"MOV %s.w,%s.x",tmp,var("trigo_cst"));
									AddInst(vp,c);
								}
								else
								{
									_sprintf(c.str,"m3x3 %s.xyz,%s,%s",tmp,rn,rn2);
									AddInst(vp,c);
									_sprintf(c.str,"mov %s.w,%s.x",tmp,var("trigo_cst"));
									AddInst(vp,c);
								}
							}
						}

						return tmp;
					}

					if (res==7)
					{
						t=0;
						if (str_simple(s0))
						{
							rn2=var(s1);
							rn=var(s0);
						}
						else
						{
							if (str_simple(s0)) rn=var(s0);
							else
							{
								_sprintf(tmp,"r%d",new_temp_register());
								rn=compile(s0,tmp,vp);
								t=1;
							}

							rn2=var(s1);
						}

						if (strcmp(output,rn)!=0) tmp=output;
						else _sprintf(tmp, "r%d", new_temp_register());

						if (api!=2)
						{
							if ((api==1)&&(shadermodel))
							{
								_sprintf(c.str,"%s.xy = %s.xy",tmp,rn);
								AddInst(vp,c);
								_sprintf(c.str,"%s.zw = vec2(0.0,0.0)",tmp);
								AddInst(vp,c);
								_sprintf(c.str,"%s = %s  * %s",tmp,rn2,tmp);
								AddInst(vp,c);
								_sprintf(c.str,"%s.zw = vec2(0.0,0.0)",tmp);
								AddInst(vp,c);
							}
							else
							if ((api==0)&&(shadermodel3))
							{
								_sprintf(tmp2, "r%d", new_temp_register());
								_sprintf(c.str, "%s.xy = %s.xy", tmp2,rn);
								AddInst(vp, c);
								_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", tmp2);
								AddInst(vp, c);
								_sprintf(c.str, "%s = mul(%s,%s)", tmp, tmp2, rn2);
								AddInst(vp,c);
								_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", tmp);
								AddInst(vp, c);
							}
						}

						return tmp;
					}

				}
				else
				{
					goto _functions_test;
				}
			}
			return "null";
		}
	}
	return "null";

_functions_test:

	int p0;
	Macro *m,*mac;

	p0=str_char(chaine,'(');

	if (p0!=-1)
	{
		mac=NULL;
		_sprintf(tmp,"%s",chaine);
		tmp[p0]='\0';
		_sprintf(tmp2,"%s",str_return_parentheses(chaine));
		m=ListMacros.GetFirst();
		while ((m)&&(!mac))
		{
			if (strcmp(m->name,tmp)==0) mac=m;
			m=ListMacros.GetNext();
		}

		if (mac)
		{			
			_sprintf(tmp,"%s",compile_macro(mac,tmp2,vp));
			return tmp;
		}
	}

	if (shadermodel)
	{
		int ps=psfnnum(chaine);
		if (ps>=0)
		{
			if ((api==0)&&(!shadermodel3))
			{
				affectparameters(chaine,&psfn[ps],vp);
				_sprintf(c.str,"call l%d",ps);
				AddInst(vp,c);
				close_temp_register();
				tagregsave[11]=1;
				return "r11";
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(tmp,"%s",str_return_parentheses(chaine));

				modifiersfn(&psfn[ps],tmp,vp);
				if (PARAMERROR)
				{
					if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
				}
				else
				{
					if (str_char(output,'.')==-1)
					{
						if (psfn[ps].ret==CALL_FLOAT) _sprintf(c.str,"%s.x = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR2) _sprintf(c.str,"%s.xy = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR3) _sprintf(c.str,"%s.xyz = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR4) _sprintf(c.str,"%s = %s(%s)",output,psfn[ps].name,tmp);
					}
					else _sprintf(c.str,"%s = %s(%s)",output,psfn[ps].name,tmp);
					AddInst(vp,c);
				}
				return output;
			}
			else
			{
				_sprintf(tmp,"%s",str_return_parentheses(chaine));

				modifiersfn(&psfn[ps],tmp,vp);
				if (PARAMERROR)
				{
					if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
				}
				else
				{
					if (str_char(output,'.')==-1)
					{
						if (psfn[ps].ret==CALL_FLOAT) _sprintf(c.str,"%s.x = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR2) _sprintf(c.str,"%s.xy = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR3) _sprintf(c.str,"%s.xyz = %s(%s)",output,psfn[ps].name,tmp);
						if (psfn[ps].ret==CALL_VECTOR4) _sprintf(c.str,"%s = %s(%s)",output,psfn[ps].name,tmp);
					}
					else _sprintf(c.str,"%s = %s(%s)",output,psfn[ps].name,tmp);
					AddInst(vp,c);
				}
				return output;
			}
		}

		int vs=vsfnnum(chaine);
		if (vs>=0)
		{			
			if ((api==0)&&(!shadermodel3))
			{
				affectparameters(chaine,&vsfn[vs],vp);
				_sprintf(c.str,"call l%d",vs);
				AddInst(vp,c);
				close_temp_register();
				tagregsave[11]=1;
				return "r11";
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(tmp,"%s",str_return_parentheses(chaine));
				modifiersfn(&vsfn[vs],tmp,vp);
				if (PARAMERROR)
				{
					if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
				}
				else
				{
					if (str_char(output,'.')==-1)
					{
						if (vsfn[vs].ret==CALL_FLOAT) _sprintf(c.str,"%s.x = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR2) _sprintf(c.str,"%s.xy = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR3) _sprintf(c.str,"%s.xyz = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR4) _sprintf(c.str,"%s = %s(%s)",output,vsfn[vs].name,tmp);
					}
					else _sprintf(c.str,"%s = %s(%s)",output,vsfn[vs].name,tmp);
					AddInst(vp,c);
				}
				return output;
			}
			else
			{
				_sprintf(tmp,"%s",str_return_parentheses(chaine));
				if (PARAMERROR)
				{
					if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
				}
				else
				{
					modifiersfn(&vsfn[vs],tmp,vp);
					if (str_char(output,'.')==-1)
					{
						if (vsfn[vs].ret==CALL_FLOAT) _sprintf(c.str,"%s.x = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR2) _sprintf(c.str,"%s.xy = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR3) _sprintf(c.str,"%s.xyz = %s(%s)",output,vsfn[vs].name,tmp);
						if (vsfn[vs].ret==CALL_VECTOR4) _sprintf(c.str,"%s = %s(%s)",output,vsfn[vs].name,tmp);
					}
					else _sprintf(c.str,"%s = %s(%s)",output,vsfn[vs].name,tmp);
					AddInst(vp,c);
				}
				return output;
			}
		}
	}

	if ((str_match0(chaine,"vec2"))||(str_match0(chaine,"vector2")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		bool bak=floatprocessing;
		floatprocessing=true;

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn=compile(&tmp[0],tmp2,vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn2=compile(&tmp[p+1],tmp2,vp);
		}

		if (str_char(output,'.')==-1) _sprintf(tmp0,"%s.xy",output);
		else _sprintf(tmp0,output);

		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s = vec2( %s, %s )",tmp0,rn,rn2);
			AddInst(vp,c);
		}
		else
		if ((api==0)&&(shadermodel3))
		{
			_sprintf(c.str,"%s = float2( %s, %s )",tmp0,rn,rn2);
			AddInst(vp,c);
		}

		floatprocessing=bak;
		return tmp0;
	}

	if ((str_match0(chaine,"vec3"))||(str_match0(chaine,"vector3")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		bool bak=floatprocessing;
		floatprocessing=true;

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn=compile(&tmp[0],tmp2,vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn2=compile(&tmp[p1+1],tmp2,vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp3,"r%d",new_temp_register());
			rn3=compile(&tmp[p2+1],tmp3,vp);
		}

		if (str_char(output,'.')==-1) _sprintf(tmp0,"%s.xyz",output);
		else _sprintf(tmp0,output);

		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s = vec3( %s, %s, %s )",tmp0,rn,rn2,rn3);
			AddInst(vp,c);
		}
		else
		if ((api==0)&&(shadermodel3))
		{
			_sprintf(c.str,"%s = float3( %s, %s, %s )",tmp0,rn,rn2,rn3);
			AddInst(vp,c);
		}

		floatprocessing=bak;

		return tmp0;
	}


	if ((str_match0(chaine,"vec4"))||(str_match0(chaine,"vector4")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		p3 = str_char_prt(&tmp[p2 + 1], ',') + p2 + 1;
        if (p3<0) { SYNTAXERROR=true; return ""; }
		tmp[p3]='\0';

		bool bak=floatprocessing;
		floatprocessing=true;

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn=compile(&tmp[0],tmp2,vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn2=compile(&tmp[p1+1],tmp2,vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp3,"r%d",new_temp_register());
			rn3=compile(&tmp[p2+1],tmp3,vp);
		}

		if (str_simple(&tmp[p3+1])) rn4=var(&tmp[p3+1]);
		else
		{
			_sprintf(tmp4,"r%d",new_temp_register());
			rn4=compile(&tmp[p3+1],tmp4,vp);
		}

		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s = vec4( %s, %s, %s,%s )",output,rn,rn2,rn3,rn4);
			AddInst(vp,c);
		}
		else
		if ((api==0)&&(shadermodel3))
		{
			_sprintf(c.str,"%s = float4( %s, %s, %s, %s )",output,rn,rn2,rn3,rn4);
			AddInst(vp,c);
		}

		floatprocessing=bak;

		return output;
	}


	if ((str_match0(chaine,"mat4"))||(str_match0(chaine,"matrix"))||(str_match0(chaine,"matrix4")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if ((api==1)&&(shadermodel)) _sprintf(c.str,"mat4 %s = mat4(",output);
		else
		if ((api==0)&&(shadermodel3)) _sprintf(c.str,"float4x4 %s = float4x4(",output);

		int nb=0;
		p=0;
		rn=str_parse_char(tmp,&p,',');
		while (rn)
		{			
			if (str_simple(rn)) rn=var(rn);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn=compile(rn,tmp2,vp);
			}

			if (nb==0)
			{
				if ((api==1)&&(shadermodel)) _sprintf(c.str,"%s %s",c.str,rn);
				else
				if ((api==0)&&(shadermodel3)) _sprintf(c.str,"%s %s",c.str,rn);
			}
			else
			{
				if ((api==1)&&(shadermodel)) _sprintf(c.str,"%s, %s",c.str,rn);
				else
				if ((api==0)&&(shadermodel3)) _sprintf(c.str,"%s, %s",c.str,rn);
			}

			nb++;
			rn=str_parse_char(tmp,&p,',');
		}

		for (int kl=nb;kl<16;kl++)
		{
			if (api==0)
			{
				if (kl==0) _sprintf(c.str,"%s 0.0f",c.str);
				else _sprintf(c.str,"%s, 0.0f",c.str);
			}
			else
			{
				if (kl==0) _sprintf(c.str,"%s 0.0",c.str);
				else _sprintf(c.str,"%s, 0.0",c.str);
			}
		}

		if ((api==1)&&(shadermodel)) _sprintf(c.str,"%s )",c.str);
		else
		if ((api==0)&&(shadermodel3)) _sprintf(c.str,"%s )",c.str);

		AddInst(vp,c);

		return output;
	}

	if (str_match0(chaine,"float"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if (!floatprocessinginit)
		{
			_sprintf(c.str,"%s = float(",output);

			rn=tmp;
			if (str_simple(rn)) rn=var(rn);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn=compile(rn,tmp2,vp);
			}

			_sprintf(c.str,"%s %s )",c.str,rn);

			AddInst(vp,c);
		}
		else
		{
			_sprintf(c.str,"float %s = float(",output);

			rn=tmp;
			if (str_simple(rn)) rn=var(rn);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn=compile(rn,tmp2,vp);
			}

			_sprintf(c.str,"%s %s )",c.str,rn);

			AddInst(vp,c);
		}

		return output;
	}


	if (str_match00(chaine,"phase"))
	{
		if (!shadermodel)
		if (api==0)
		{
			_sprintf(c.str,"phase");
			AddInst(vp,c);
			phase=1;
		}
	}
	
	if ((str_match0(chaine,"sample"))&&(!shadermodel))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		rn2=var(&tmp[p+1]);

		sscanf(tmp,"%d",&t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
		if (api==0)
		{
			if (t==0) rn="r0";
			if (t==1) rn="r1";
			if (t==2) rn="r2";
			if (t==3) rn="r3";
			if (t==4) rn="r4";
			if (t==5) rn="r5";

			if ((rn2[0]=='r')&&(phase==0))
			{
				phase=1;
				_sprintf(c.str,"phase");
				AddInst(vp,c);
			}

			_sprintf(c.str,"texld %s,%s",rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			rn=output;
			_sprintf(c.str,"TEX %s,%s,texture[%d],2D",output,rn2,t);
			AddInst(vp,c);
		}

		return rn;
	}

    if ((str_match00(chaine, "sampleVShalf.")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

		char ext[1024];
		_sprintf(ext, &chaine[str_char(chaine, '.')]);
		ext[str_char(ext, '(')] = '\0';
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s),0.0)", output, RS.TextureVS[t], rn2);
            AddInst(vp, c);

			if (strlen(ext)-1==1)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - 1.0", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==2)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec2(1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==3)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec3(1.0,1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==4)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec4(1.0,1.0,1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}

        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }

			if (strlen(ext)-1==1)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - 1.0f", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==2)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float2(1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==3)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float3(1.0f,1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==4)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float4(1.0f,1.0f,1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else    
    if ((str_match0(chaine, "samplesecondaryVShalf16")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);

            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s.xy),0.0)", output, RS.TextureVS[t], tmp3);
            AddInst(vp, c);

	        _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0*(255.0 * %s.z + %s.y)/255.0 - 1.0", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }

	        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0f*(255.0f * %s.z + %s.y)/255.0f - 1.0f", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
            AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else    
    if ((str_match0(chaine, "sampleVShalf16")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s),0.0)", output, RS.TextureVS[t], rn2);
            AddInst(vp, c);

	        _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0*(255.0 * %s.z + %s.y)/255.0 - 1.0", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }

	        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0f*(255.0f * %s.z + %s.y)/255.0f - 1.0f", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
            AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else    
    if ((str_match0(chaine, "samplehalf16")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2D(%s, vec2(%s))", output, RS.Texture[t], rn2);
            AddInst(vp, c);

	        _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0*(255.0 * %s.z + %s.y)/255.0 - 1.0", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }
            
	        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0f*(255.0f * %s.z + %s.y)/255.0f - 1.0f", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
            AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }

        return output;
    }
    else
	if ((str_match00(chaine, "samplehalf.")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

		char ext[1024];
		_sprintf(ext, &chaine[str_char(chaine, '.')]);
		ext[str_char(ext, '(')] = '\0';

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2D(%s, vec2(%s))", output, RS.Texture[t], rn2);
            AddInst(vp, c);

			if (strlen(ext)-1==1)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - 1.0", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==2)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec2(1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==3)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec3(1.0,1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==4)
			{
	            _sprintf(c.str, "%s%s = 2.0 * %s%s - vec4(1.0,1.0,1.0,1.0)", output,ext, output,ext);
		        AddInst(vp, c);
			}
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }
            
			if (strlen(ext)-1==1)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - 1.0f", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==2)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float2(1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==3)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float3(1.0f,1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}

			if (strlen(ext)-1==4)
			{
	            _sprintf(c.str, "%s%s = 2.0f * %s%s - float4(1.0f,1.0f,1.0f,1.0f)", output,ext, output,ext);
		        AddInst(vp, c);
			}
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }

        return output;
    }
    else
	if ((str_match00(chaine, "sample.")) && (shadermodel))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		rn2 = var(&tmp[p + 1]);
		if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn2 = compile(&tmp[p + 1], tmp2, vp);
		}
        
		if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

		_sprintf(tmp3, "r%d", new_temp_register());
		char ext[1024];

		_sprintf(ext, &chaine[str_char(chaine, '.')]);
		ext[str_char(ext, '(')] = '\0';
		
		_sprintf(tmp4, "%s%s", tmp3,ext);

		sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
		if ((api == 1) && (shadermodel))
		{
			_sprintf(c.str, "%s = texture2D(%s, vec2(%s))", tmp3, RS.Texture[t], rn2);
			AddInst(vp, c);
		}
		else
		if ((api == 0) && (shadermodel3))
		{
			if (shadermodel4)
			{
				if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
				{
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
					if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
					else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
					if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
					else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
					
#endif
					AddInst(vp, c);
				}
				else
				{
					if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", tmp3, RS.Texture[t], rn2);
					else _sprintf(c.str, "%s = %s.Sample(smp, %s)", tmp3, RS.Texture[t], rn2);
					AddInst(vp, c);
				}
			}
			else
			{
				int res=0;
				for (int k=0;k<ntexture;k++)
					if (RS.Texture[t])
						if (strcmp(texture[k],RS.Texture[t])==0) res=k;

				if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", tmp3, res, rn2);
				else _sprintf(c.str, "%s = tex2D(s%d, %s)", tmp3, res, rn2);
				AddInst(vp, c);
			}
		}

		return tmp4;
	}
    else
    if ((str_match0(chaine, "sampleLOD")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2D(%s, vec2(%s))", output, RS.Texture[t], rn2);
            AddInst(vp, c);
        }
        else
            if ((api == 0) && (shadermodel3))
            {
                if (shadermodel4)
                {
                    if (metal==1)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        AddInst(vp, c);
                    }
                    else
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                }
                else
                {
                    int res=0;
                    for (int k=0;k<ntexture;k++)
                        if (RS.Texture[t])
                            if (strcmp(texture[k],RS.Texture[t])==0) res=k;
                    
                    if (levelrep>0)
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                        else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                        else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                    }
                    AddInst(vp, c);
                }
            }
            else
                if (api == 0)
                {
                    _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
                    AddInst(vp, c);
                }
        return output;
    }
	else
	if((str_match0(chaine,"samplebumpy")) && (shadermodel))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp,',');
		if(p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1] = '\0';

		p2 = str_char_prt(&tmp[p1 + 1],',') + p1 + 1;
		if(p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2] = '\0';

		if(str_simple(&tmp[p1 + 1])) rn2 = var(&tmp[p1 + 1]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output,tmp2);
			rn2 = compile(&tmp[p1 + 1],tmp2,vp);
		}

		if(str_simple(&tmp[p2 + 1])) rn3 = var(&tmp[p2 + 1]);
		else
		{
			_sprintf(tmp3,"r%d",new_temp_register());
			resfield(output,tmp3);
			rn3 = compile(&tmp[p2 + 1],tmp3,vp);
		}

		if(!correct_modifier_texture2d(rn2)) TYPEERROR=true;

		sscanf(tmp,"%d",&t);
		if((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }

		_sprintf(tmp4,"r%d",new_temp_register());

		if((api == 1) && (shadermodel))
		{
			if ((SizingTexture0)&&(t==0)&&(main==1))
			{
				// Sample + neighboor
				_sprintf(c.str,"%s.x = 0.3 * %s.r + 0.6 * %s.g + 0.1 * %s.b",output,rn3,rn3,rn3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xy = %s.xy + vec2(SIZEDESTZ.z,0.0)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = 0.3 * %s.r + 0.6 * %s.g + 0.1 * %s.b",output,tmp4,tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xy = %s.xy + vec2(0.0,SIZEDESTZ.w)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = 0.3 * %s.r + 0.6 * %s.g + 0.1 * %s.b",output,tmp4,tmp4,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,output,output);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,tmp4);
				AddInst(vp,c);
			}
		}
		else
		if ((api == 0) && (shadermodel3))
		{
			if((SizingTexture0)&&(t==0)&&(main==1))
			{
				// Sample + neighboor
				_sprintf(c.str,"%s.x = 0.3f * %s.r + 0.6f * %s.g + 0.1f * %s.b",output,rn3,rn3,rn3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xy = %s.xy + float2(SIZEDESTZ.z,0.0)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = 0.3f * %s.r + 0.6f * %s.g + 0.1f * %s.b",output,tmp4,tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xy = %s.xy + float2(0.0,SIZEDESTZ.w)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = 0.3f * %s.r + 0.6f * %s.g + 0.1f * %s.b",output,tmp4,tmp4,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,output,output);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %sf",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,tmp4);
				AddInst(vp,c);
			}
		}

		return output;
	}
	else
	if((str_match0(chaine,"samplebump")) && (shadermodel))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if(str_simple(tmp)) rn = var(tmp);
		else SYNTAXERROR=true;

		if(!correct_modifier_texture2d(rn)) TYPEERROR=true;

		_sprintf(tmp2,"r%d",new_temp_register());
		_sprintf(tmp3,"r%d",new_temp_register());
		_sprintf(tmp4,"r%d",new_temp_register());

		if((api == 1) && (shadermodel))
		{
			if((SizingTexture0)&&(main==1))
			{
				_sprintf(c.str,"vec3 a00,a10,a11,a01");
				AddInst(vp,c);

                _sprintf(c.str,"%s.xy = ( floor(%s.xy * SIZEDESTZ.xy ) )* SIZEDESTZ.zw",tmp3,rn);
                AddInst(vp,c);

				// 0,0
                _sprintf(c.str,"%s.xy = %s.xy+vec2(-SIZEDESTZ.z,-SIZEDESTZ.w)",tmp4,tmp3);
                AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
				AddInst(vp,c);

                _sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,-SIZEDESTZ.w)",tmp4,tmp3);
                AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(-SIZEDESTZ.z,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"a00.xyz = normalize( %s.xyz )",tmp4);
				AddInst(vp,c);


				// 1,0
				_sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,-SIZEDESTZ.w)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(SIZEDESTZ.z,-SIZEDESTZ.w)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"a10.xyz = normalize( %s.xyz )",tmp4);
				AddInst(vp,c);

				// 0,1
				_sprintf(c.str,"%s.xy = %s.xy+vec2(-SIZEDESTZ.z,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(-SIZEDESTZ.z,SIZEDESTZ.w)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"a01.xyz = normalize( %s.xyz )",tmp4);
				AddInst(vp,c);

				// 1,1
				_sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(SIZEDESTZ.z,0.0)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xy = %s.xy+vec2(0.0,SIZEDESTZ.w)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
				AddInst(vp,c);

				// Normal
				_sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
				AddInst(vp,c);
				_sprintf(c.str,"a11.xyz = normalize( %s.xyz )",tmp4);
				AddInst(vp,c);

                // bilinear

                _sprintf(c.str,"%s.xy = floor(%s.xy * SIZEDESTZ.xy )",tmp3,rn);
                AddInst(vp,c);
				_sprintf(c.str,"%s.xy = %s.xy * SIZEDESTZ.xy - %s.xy",tmp4,rn,tmp3);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xyz = a00.xyz + %s.x*(a10.xyz - a00.xyz)",tmp2,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = a01.xyz + %s.x*(a11.xyz - a01.xyz)",tmp3,tmp4);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xyz = %s.xyz + %s.y*(%s.xyz - %s.xyz)",output,tmp2,tmp4,tmp3,tmp2);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xyz *= vec3(-1.0,-1.0,0.6)",output);
				AddInst(vp,c);

				_sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,output);
				AddInst(vp,c);

			}
		}
		else
			if((api == 0) && (shadermodel3))
			{
				if((SizingTexture0)&&(main==1))
				{
					_sprintf(c.str,"float3 a00,a10,a11,a01");
					AddInst(vp,c);

					_sprintf(c.str,"%s.xy = floor(%s.xy * SIZEDESTZ.xy) * SIZEDESTZ.zw",tmp3,rn);
					AddInst(vp,c);

                    // 0,0
                    _sprintf(c.str,"%s.xy = %s.xy+float2(-SIZEDESTZ.z,-SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,-SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(-SIZEDESTZ.z,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    // Normal
                    _sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
                    AddInst(vp,c);
                    _sprintf(c.str,"a00.xyz = normalize( %s.xyz )",tmp4);
                    AddInst(vp,c);


                    // 1,0
                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,-SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(SIZEDESTZ.z,-SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    // Normal
                    _sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
                    AddInst(vp,c);
                    _sprintf(c.str,"a10.xyz = normalize( %s.xyz )",tmp4);
                    AddInst(vp,c);

                    // 0,1
                    _sprintf(c.str,"%s.xy = %s.xy+float2(-SIZEDESTZ.z,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(-SIZEDESTZ.z,SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    // Normal
                    _sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s",tmp4,value_bumpy_str);
                    AddInst(vp,c);
                    _sprintf(c.str,"a01.xyz = normalize( %s.xyz )",tmp4);
                    AddInst(vp,c);

                    // 1,1
                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(SIZEDESTZ.z,0.0f)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.xy = %s.xy+float2(0.0f,SIZEDESTZ.w)",tmp4,tmp3);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = TiledSample0(%s.xy)",tmp4,tmp4);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %s.a",tmp2,tmp4);
                    AddInst(vp,c);

                    // Normal
                    _sprintf(c.str,"%s.xy = %s.yz - %s.xx",tmp4,tmp2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = %sf",tmp4,value_bumpy_str);
                    AddInst(vp,c);
                    _sprintf(c.str,"a11.xyz = normalize( %s.xyz )",tmp4);
                    AddInst(vp,c);

                    // bilinear

                    _sprintf(c.str,"%s.xy = floor(%s.xy * SIZEDESTZ.xy )",tmp3,rn);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xy = %s.xy * SIZEDESTZ.xy - %s.xy",tmp4,rn,tmp3);
                    AddInst(vp,c);

					_sprintf(c.str,"%s.xyz = a00.xyz + %s.x*(a10.xyz - a00.xyz)",tmp2,tmp4);
					AddInst(vp,c);
					_sprintf(c.str,"%s.xyz = a01.xyz + %s.x*(a11.xyz - a01.xyz)",tmp3,tmp4);
					AddInst(vp,c);

					_sprintf(c.str,"%s.xyz = %s.xyz + %s.y*(%s.xyz - %s.xyz)",output,tmp2,tmp4,tmp3,tmp2);
					AddInst(vp,c);

					_sprintf(c.str,"%s.xyz *= float3(-1.0f,-1.0f,0.2f)",output);
					AddInst(vp,c);

					_sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,output);
					AddInst(vp,c);
				}
			}

		return output;
	}
	else
    if ((str_match0(chaine, "samplehalf")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
			if ((FSRSample0)&&(t==0)&&(main==1))
			{
				_sprintf(c.str, "AU2 gxy = AU2(%s * SIZEDEST0.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF0(vec2(gxy),%s)", output, valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			if ((FSRSample1)&&(t==1)&&(main==1))
			{
				_sprintf(c.str, "AU2 bxy = AU2(%s * SIZEDEST1.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF1(vec2(bxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			{
				_sprintf(c.str, "%s = texture2D(%s, vec2(%s))", output, RS.Texture[t], rn2);
				AddInst(vp, c);
			}

            _sprintf(c.str, "%s.xyz = 2.0 * %s.xyz - vec3(1.0,1.0,1.0)", output, output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
			if ((FSRSample0)&&(t==0)&&(main==1))
			{
				_sprintf(c.str, "AU2 gxy = AU2(%s * SIZEDEST0.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF0(float2(gxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			if ((FSRSample1)&&(t==1)&&(main==1))
			{
				_sprintf(c.str, "AU2 bxy = AU2(%s * SIZEDEST1.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF1(float2(bxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }
            
            _sprintf(c.str, "%s.xyz = 2.0f * %s.xyz - float3(1.0f,1.0f,1.0f)", output, output);
            AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }

        return output;
    }
    else
    if ((str_match0(chaine, "sampleVSOffsetClamp")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));

        p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
        tmp[p1] = '\0';

        p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
        tmp[p2] = '\0';

        if (str_simple(&tmp[p1 + 1])) rn2 = var(&tmp[p1 + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p1 + 1], tmp2, vp);
        }

        if (str_simple(&tmp[p2 + 1])) rn3 = var(&tmp[p2 + 1]);
        else
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            resfield(output, tmp3);
            rn3 = compile(&tmp[p2 + 1], tmp3, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
                
        if ((api == 1) && (shadermodel))
        {
            if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }

            _sprintf(tmp3, "r%d", new_temp_register());
            _sprintf(c.str, "%s.xy = (%s+vec2(0.5,0.5)) * %s", tmp3, rn2, rn3);
            AddInst(vp, c);

            _sprintf(c.str, "%s.x = clamp(%s.x, 0.0, 1.0)", tmp3, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = clamp(%s.y, 0.0, 1.0)", tmp3, tmp3);
            AddInst(vp, c);

            _sprintf(c.str, "%s = texture2DLod(%s, %s.xy,0.0)", output, RS.TextureVS[t], tmp3);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    _sprintf(tmp3, "r%d", new_temp_register());
                    if (str_match(rn2,"pixel.")) _sprintf(c.str, "%s.xy = %s.xy * %s", tmp3, rn2, rn3);
                    else _sprintf(c.str, "%s.xy = %s * %s", tmp3, rn2, rn3);
                    AddInst(vp, c);
                    
                    if (str_last_char(rn3, '.') == -1)
                    {
                        SYNTAXERROR=PARAMERROR=true;
                        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                        return "";
                    }

                    _sprintf(c.str, "%s.x = clamp(%s.x, 0.0f, 1.0f)", tmp3, tmp3);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.y = clamp(%s.y, 0.0f, 1.0f)", tmp3, tmp3);
                    AddInst(vp, c);

                    _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], tmp3);
                    AddInst(vp, c);
                }
                else
                {
                    if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
                    {
                        SYNTAXERROR=PARAMERROR=true;
                        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                        return "";
                    }

                    _sprintf(tmp3, "r%d", new_temp_register());
                    _sprintf(c.str, "%s.xy = (%s+float2(0.5f,0.5f)) * %s", tmp3, rn2, rn3);
                    AddInst(vp, c);

                    _sprintf(c.str, "%s.x = clamp(%s.x, 0.0f, 1.0f)", tmp3, tmp3);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.y = clamp(%s.y, 0.0f, 1.0f)", tmp3, tmp3);
                    AddInst(vp, c);

#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], tmp3);
#else
                    _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], tmp3);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
                {
                    SYNTAXERROR=PARAMERROR=true;
                    if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                    return "";
                }

                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                _sprintf(tmp3, "r%d", new_temp_register());
                _sprintf(c.str, "%s.xy = (%s+float2(0.5f,0.5f)) * %s", tmp3, rn2, rn3);
                AddInst(vp, c);

                _sprintf(c.str, "%s.x = clamp(%s.x, 0.0f, 1.0f)", tmp3, tmp3);
                AddInst(vp, c);
                _sprintf(c.str, "%s.y = clamp(%s.y, 0.0f, 1.0f)", tmp3, tmp3);
                AddInst(vp, c);

                if (levelrep>0)
                {
                    _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, tmp3);
                }
                else
                {
                    _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, tmp3);
                }
                AddInst(vp, c);
            }
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "sampleVSOffset")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));

        p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
        tmp[p1] = '\0';

        p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
        tmp[p2] = '\0';

        if (str_simple(&tmp[p1 + 1])) rn2 = var(&tmp[p1 + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p1 + 1], tmp2, vp);
        }

        if (str_simple(&tmp[p2 + 1])) rn3 = var(&tmp[p2 + 1]);
        else
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            resfield(output, tmp3);
            rn3 = compile(&tmp[p2 + 1], tmp3, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
                
        if ((api == 1) && (shadermodel))
        {
            if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }

            _sprintf(tmp3, "r%d", new_temp_register());
            _sprintf(c.str, "%s.xy = (%s+vec2(0.5,0.5)) * %s", tmp3, rn2, rn3);
            AddInst(vp, c);

            _sprintf(c.str, "%s = texture2DLod(%s, %s.xy,0.0)", output, RS.TextureVS[t], tmp3);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    _sprintf(tmp3, "r%d", new_temp_register());
                    if (str_match(rn2,"pixel.")) _sprintf(c.str, "%s.xy = %s.xy * %s", tmp3, rn2, rn3);
                    else _sprintf(c.str, "%s.xy = %s * %s", tmp3, rn2, rn3);
                    AddInst(vp, c);
                    
                    if (str_last_char(rn3, '.') == -1)
                    {
                        SYNTAXERROR=PARAMERROR=true;
                        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                        return "";
                    }

                    _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], tmp3);
                    AddInst(vp, c);
                }
                else
                {
                    if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
                    {
                        SYNTAXERROR=PARAMERROR=true;
                        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                        return "";
                    }

                    _sprintf(tmp3, "r%d", new_temp_register());
                    _sprintf(c.str, "%s.xy = (%s+float2(0.5f,0.5f)) * %s", tmp3, rn2, rn3);
                    AddInst(vp, c);

#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], tmp3);
#else
                    _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], tmp3);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                if ((str_last_char(rn2, '.') == -1)||(str_last_char(rn3, '.') == -1))
                {
                    SYNTAXERROR=PARAMERROR=true;
                    if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                    return "";
                }

                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                _sprintf(tmp3, "r%d", new_temp_register());
                _sprintf(c.str, "%s.xy = (%s+float2(0.5f,0.5f)) * %s", tmp3, rn2, rn3);
                AddInst(vp, c);

                if (levelrep>0)
                {
                    _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, tmp3);
                }
                else
                {
                    _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, tmp3);
                }
                AddInst(vp, c);
            }
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "sampleVShalf")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s),0.0)", output, RS.TextureVS[t], rn2);
            AddInst(vp, c);
			_sprintf(c.str, "%s = 2.0 * %s.xyzw - vec4(1.0,1.0,1.0,1.0)",output,output);
			AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }

			_sprintf(c.str, "%s = 2.0f * %s - float4(1.0f,1.0f,1.0f,1.0f)",output,output);
			AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplesecondaryVShalf")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);

            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s.xy),0.0)", output, RS.TextureVS[t], tmp3);
            AddInst(vp, c);

			_sprintf(c.str, "%s = 2.0 * %s.xyzw - vec4(1.0,1.0,1.0,1.0)",output,output);
			AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }
			_sprintf(c.str, "%s = 2.0f * %s - float4(1.0f,1.0f,1.0f,1.0f)",output,output);
			AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplesecondaryVS")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);

            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s.xy),0.0)", output, RS.TextureVS[t], tmp3);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "sampleVS")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';
        
        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(c.str, "%s = texture2DLod(%s, vec2(%s),0.0)", output, RS.TextureVS[t], rn2);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.TextureVS[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.TextureVS[t], rn2);
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smpvs, %s.xy)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.Sample(smpvs, %s)", output, RS.TextureVS[t], rn2);
#else
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s.xy,0)", output, RS.TextureVS[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smpvs, %s,0)", output, RS.TextureVS[t], rn2);
#endif
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexturevs;k++)
                    if (RS.TextureVS[t])
                        if (strcmp(texture[k],RS.TextureVS[t])==0) res=k;
                
                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(svs%d, float4(%s,0,0))", output, res, rn2);
                }
                AddInst(vp, c);
            }
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplesecondaryhalf16")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);
            
            _sprintf(c.str, "%s = texture2D(%s, %s.xy)", output, RS.Texture[t], tmp3);
            AddInst(vp, c);

	        _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0*(255.0 * %s.z + %s.y)/255.0 - 1.0", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }

	        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
		    AddInst(vp, c);

            _sprintf(c.str, "%s.y = 2.0f*(255.0f * %s.z + %s.y)/255.0f - 1.0f", output, output, output);
            AddInst(vp, c);

			_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
            AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplesecondaryhalf")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);
            
            _sprintf(c.str, "%s = texture2D(%s, %s.xy)", output, RS.Texture[t], tmp3);
            AddInst(vp, c);

			_sprintf(c.str, "%s = 2.0 * %s.xyzw - vec4(1.0,1.0,1.0,1.0)",output,output);
			AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }

			_sprintf(c.str, "%s = 2.0f * %s - float4(1.0f,1.0f,1.0f,1.0f)",output,output);
			AddInst(vp, c);
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplesecondary")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s.xy = %s.xy", tmp3,rn2);
            else _sprintf(c.str, "%s.xy = %s", tmp3,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 1.0 - %s.y", tmp3,tmp3);
            AddInst(vp, c);
            
            _sprintf(c.str, "%s = texture2D(%s, %s.xy)", output, RS.Texture[t], tmp3);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        AddInst(vp, c);
                    }
                    else
                    {
                        if (levelrep>0)
                        {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
                        }
                        AddInst(vp, c);
                    }
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (levelrep>0)
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
                    else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
                }
                AddInst(vp, c);
            }
        }
        else
        if (api == 0)
        {
            _sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
            AddInst(vp, c);
        }
        return output;
    }
    else
    if ((str_match0(chaine, "samplebasedepth")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = texture2D(%s, %s.xy)", output, RS.Texture[t], rn2);
            else _sprintf(c.str, "%s = texture2D(%s, %s)", output, RS.Texture[t], rn2);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0).r", output, RS.Texture[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0).r", output, RS.Texture[t], rn2);
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0)).r", output, res, rn2);
                else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0)).r", output, res, rn2);
                AddInst(vp, c);
            }
        }

		return output;
    }
    else
    if ((str_match0(chaine, "sampledepth")) && (shadermodel))
    {
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {            
            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = texture2D(%s, %s.xy)", output, RS.Texture[t], rn2);
            else _sprintf(c.str, "%s = texture2D(%s, %s)", output, RS.Texture[t], rn2);
            AddInst(vp, c);
			_sprintf(c.str, "%s.x = 1.0 - 2.0 * %s.x", output, output);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
            if (shadermodel4)
            {
                if (metal==1)
                {
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                    }
                    AddInst(vp, c);
                }
                else
                {
                    if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0).r", output, RS.Texture[t], rn2);
                    else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0).r", output, RS.Texture[t], rn2);
                    AddInst(vp, c);
                }
            }
            else
            {
                int res=0;
                for (int k=0;k<ntexture;k++)
                    if (RS.Texture[t])
                        if (strcmp(texture[k],RS.Texture[t])==0) res=k;

                if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0)).r", output, res, rn2);
                else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0)).r", output, res, rn2);
                AddInst(vp, c);
            }
        }

		return output;
    }
    else
	if ((str_match0(chaine, "sample")) && (shadermodel))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		rn2 = var(&tmp[p + 1]);
		if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn2 = compile(&tmp[p + 1], tmp2, vp);
		}
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

		sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
		if ((api == 1) && (shadermodel))
		{
			if ((FSRSample0)&&(t==0)&&(main==1))
			{
				_sprintf(c.str, "AU2 gxy = AU2(%s * SIZEDEST0.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF0(vec2(gxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			if ((FSRSample1)&&(t==1)&&(main==1))
			{
				_sprintf(c.str, "AU2 bxy = AU2(%s * SIZEDEST1.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF1(vec2(bxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			{
				_sprintf(c.str, "%s = texture2D(%s, vec2(%s))", output, RS.Texture[t], rn2);
				AddInst(vp, c);
			}
		}
		else
		if ((api == 0) && (shadermodel3))
		{
			if ((FSRSample0)&&(t==0)&&(main==1))
			{
				_sprintf(c.str, "AU2 gxy = AU2(%s * SIZEDEST0.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF0(float2(gxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			if ((FSRSample1)&&(t==1)&&(main==1))
			{
				_sprintf(c.str, "AU2 bxy = AU2(%s * SIZEDEST1.xy)",rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s = FsrRcasF1(float2(bxy),%s)", output,valueFSRRCASSample);
				AddInst(vp, c);
			}
			else
			if (shadermodel4)
			{
				if (metal==1)
				{
                    if (RS.Mip[t]==_POINT)
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s1, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s1, %s))", output, RS.Texture[t], rn2);
                            
                        }
                    }
                    else
                    {
                        if (str_match(rn2,"pixel."))
                        {
                            _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                        }
                        else
                        {
                            if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = float4(%s.sample(s0, %s.xy))", output, RS.Texture[t], rn2);
                            else _sprintf(c.str, "%s = float4(%s.sample(s0, %s))", output, RS.Texture[t], rn2);
                            
                        }
                        
                    }
					AddInst(vp, c);
				}
				else
				{
                    if ((RS.Mip[t]==_POINT)||(shadertexturelevel))
                    {
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
						else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
						else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
						AddInst(vp, c);
					}
					else
					{
						if (levelrep>0)
						{
#if defined(WINDOWS_PHONE)&&!defined(WINDOWS_STORE)
							if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
							else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
#else
							if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.SampleLevel(smp, %s.xy,0)", output, RS.Texture[t], rn2);
							else _sprintf(c.str, "%s = %s.SampleLevel(smp, %s,0)", output, RS.Texture[t], rn2);
#endif
						}
						else
						{
							if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.Sample(smp, %s.xy)", output, RS.Texture[t], rn2);
							else _sprintf(c.str, "%s = %s.Sample(smp, %s)", output, RS.Texture[t], rn2);
						}
						AddInst(vp, c);
					}
				}
			}
			else
			{
				int res=0;
				for (int k=0;k<ntexture;k++)
					if (RS.Texture[t])
						if (strcmp(texture[k],RS.Texture[t])==0) res=k;

				if (levelrep>0)
				{
					if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s.xy,0,0))", output, res, rn2);
					else _sprintf(c.str, "%s = tex2Dlod(s%d, float4(%s,0,0))", output, res, rn2);
				}
				else
				{
					if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = tex2D(s%d, %s.xy)", output, res, rn2);
					else _sprintf(c.str, "%s = tex2D(s%d, %s)", output, res, rn2);
				}
				AddInst(vp, c);
			}
		}
		else
		if (api == 0)
		{
			_sprintf(c.str, "texld %s,%s,s%d", output, rn2, t);
			AddInst(vp, c);
		}
		return output;
	}
    
    //FSR(float2 InputSize,float2 OutputSize,float2 map)
    if ((str_match0(chaine, "fsrEASU")) && (shadermodel))
    {
        FSR=1;
        
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        
        rn2 = var(tmp);
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;
        
        _sprintf(c.str, "FsrEasuCon(SIZETEX.x,SIZETEX.y,SIZETEX.x,SIZETEX.y,SIZEDEST.x,SIZEDEST.y)");
        AddInst(vp, c);
        
        _sprintf(c.str, "AU2 gxy = AU2(%s.xy * SIZEDEST.xy)",rn2);
        AddInst(vp, c);
        _sprintf(c.str, "AF3 col = FsrEasuF(gxy)");
        AddInst(vp, c);
        if (api==1) _sprintf(c.str, "%s = vec4(col.xyz,1.0)", output);
        else _sprintf(c.str, "%s = float4(col.xyz,1.0f)", output);
        AddInst(vp, c);

        return output;
    }

    if ((str_match0(chaine, "fsrRCAS")) && (shadermodel))
    {
        FSR=2;
        
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }

        if (str_simple(tmp)) rn = var(tmp);
        else
        {
            _sprintf(tmp3, "r%d", new_temp_register());
            resfield(output, tmp3);
            rn = compile(tmp, tmp3, vp);
        }

        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        //float sh=0.0f;
        //sscanf(tmp, "%f", &sh);

        _sprintf(c.str, "FsrRcasCon(%s)",rn);
        AddInst(vp, c);
        
        _sprintf(c.str, "AU2 gxy = AU2(%s.xy * SIZEDEST.xy)",rn2);
        AddInst(vp, c);
        //_sprintf(c.str, "AF3 col = FsrRcasF(gxy,FILMGRAIN.xyz)");
        _sprintf(c.str, "AF3 col = FsrRcasF(gxy)");
        AddInst(vp, c);
        if (api==1) _sprintf(c.str, "%s = vec4(col.xyz,1.0)", output);
        else _sprintf(c.str, "%s = float4(col.xyz,1.0f)", output);
        AddInst(vp, c);

        return output;
    }

    if (((str_match0(chaine, "gatherRed"))||(str_match0(chaine, "gatherGreen"))||(str_match0(chaine, "gatherBlue"))) && (shadermodel))
    {
        int comp=-1;
        char *compo=NULL;
        char *gatherfn=NULL;
        char *swizl=NULL;
        if (str_match0(chaine, "gatherRed")) { comp=0; gatherfn="GatherRed"; compo="component::x"; swizl=".r"; }
        if (str_match0(chaine, "gatherGreen")) { comp=1; gatherfn="GatherGreen"; compo="component::y"; swizl=".g"; }
        if (str_match0(chaine, "gatherBlue")) { comp=2; gatherfn="GatherBlue"; compo="component::z"; swizl=".b"; }
        
        _sprintf(tmp, "%s", str_return_parentheses(chaine));
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p] = '\0';

        rn2 = var(&tmp[p + 1]);
        if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
        else
        {
            _sprintf(tmp2, "r%d", new_temp_register());
            resfield(output, tmp2);
            rn2 = compile(&tmp[p + 1], tmp2, vp);
        }
        
        if (!correct_modifier_texture2d(rn2)) TYPEERROR=true;

        sscanf(tmp, "%d", &t);
        if ((t<0)||(t>=16)) { t=0; SYNTAXERROR=true; }
        
        if ((api == 1) && (shadermodel))
        {
            //_sprintf(c.str, "ivec2 isize = textureSize(%s,0)", RS.Texture[t]);
            //AddInst(vp, c);
            //_sprintf(c.str, "%s = texturegather2D(%s, vec2(%s),ivec2(0),%d)", output, RS.Texture[t], rn2,comp);
            //AddInst(vp, c);

            _sprintf(c.str, "vec4 gth");
            AddInst(vp, c);

            // 0,0
            _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + vec2(-0.5,0.5)", rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.x = texture2D(%s, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t],swizl);
            AddInst(vp, c);

            // 1,0
            _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + vec2(0.5,0.5)", rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = texture2D(%s, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t],swizl);
            AddInst(vp, c);

            // 1,1
            _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + vec2(0.5,-0.5)", rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.z = texture2D(%s, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t],swizl);
            AddInst(vp, c);

            // 0,1
            _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + vec2(-0.5,-0.5)", rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.w = texture2D(%s, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t],swizl);
            AddInst(vp, c);
        }
        else
        if ((api == 0) && (shadermodel3))
        {
			int gath=0;
#if defined(API3D_DIRECT3D11)||defined(API3D_DIRECT3D12)||defined(API3D_METAL)
			if (shadermodel4) gath=1;
#endif
            if (gath)
            {
                if (metal==1)
                {
                    /*
                    if (str_match(rn2,"pixel."))
                    {
                        _sprintf(c.str, "%s = %s.gather(s0, %s.xy,int2(0),%s)", output, RS.Texture[t], rn2, compo);
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.gather(s0, %s.xy,int2(0),%s)", output, RS.Texture[t], rn2, compo);
                        else _sprintf(c.str, "%s = %s.gather(s0, %s,int2(0),%s)", output, RS.Texture[t], rn2, compo);
                        
                    }
                    AddInst(vp, c);
                    /**/
                    
                    _sprintf(c.str, "float4 gth");
                    AddInst(vp, c);

                    // 0,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + float2(-0.5,0.5)", rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.x = %s.sample(s1, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
                    AddInst(vp, c);

                    // 1,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + float2(0.5,0.5)", rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.y = %s.sample(s1, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
                    AddInst(vp, c);

                    // 1,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + float2(0.5,-0.5)", rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.sample(s1, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
                    AddInst(vp, c);

                    // 0,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) + float2(-0.5,-0.5)", rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.sample(s1, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
                    AddInst(vp, c);

                }
                else
                {
					_sprintf(c.str, "float4 gth");
					AddInst(vp, c);

					// 0,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(-0.5,0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 1,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(0.5,0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 1,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(0.5,-0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.z = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 0,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(-0.5,-0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.w = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);
					/*
                    if (levelrep>0)
                    {
#ifdef WINDOWS_PHONE
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.%s(smp, %s.xy,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
                        else _sprintf(c.str, "%s = %s.%s(smp, %s,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
#else
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.%s(smp, %s.xy,0,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
                        else _sprintf(c.str, "%s = %s.%s(smp, %s,0,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
#endif
                    }
                    else
                    {
                        if (str_last_char(rn2, '.') == -1) _sprintf(c.str, "%s = %s.%s(smp, %s.xy,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
                        else _sprintf(c.str, "%s = %s.%s(smp, %s,int2(0,0))", output, RS.Texture[t], gatherfn, rn2);
                    }
                    AddInst(vp, c);
					/**/
                }
            }
            else
            {
				if (shadermodel4)
				{
					_sprintf(c.str, "float4 gth");
					AddInst(vp, c);

					// 0,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(-0.5,0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 1,0
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(0.5,0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 1,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(0.5,-0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.z = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

					// 0,1
                    _sprintf(c.str, "gth.xy = floor(%s.xy * SIZETEX.xy) +float2(-0.5,-0.5)", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.w = %s.Sample(smp, gth.xy * SIZETEX.zw)%s", output, RS.Texture[t], swizl);
					AddInst(vp, c);

				}
				else
				{
					int res=0;
					for (int k=0;k<ntexture;k++)
						if (RS.Texture[t])
							if (strcmp(texture[k],RS.Texture[t])==0) res=k;

					_sprintf(c.str, "float4 gth");
					AddInst(vp, c);

					// 0,0
					_sprintf(c.str, "gth.x = floor(%s.x * SIZETEX.x) - 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "gth.y = floor(%s.y * SIZETEX.y) + 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.x = tex2D(s%d, gth.xy * SIZETEX.zw)%s", output, res,swizl);
					AddInst(vp, c);

					// 1,0
					_sprintf(c.str, "gth.x = floor(%s.x * SIZETEX.x) + 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "gth.y = floor(%s.y * SIZETEX.y) + 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.y = tex2D(s%d, gth.xy * SIZETEX.zw)%s", output, res,swizl);
					AddInst(vp, c);

					// 1,1
					_sprintf(c.str, "gth.x = floor(%s.x * SIZETEX.x) + 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "gth.y = floor(%s.y * SIZETEX.y) - 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.z = tex2D(s%d, gth.xy * SIZETEX.zw)%s", output, res,swizl);
					AddInst(vp, c);

					// 0,1
					_sprintf(c.str, "gth.x = floor(%s.x * SIZETEX.x) - 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "gth.y = floor(%s.y * SIZETEX.y) - 0.5", rn2);
					AddInst(vp, c);
					_sprintf(c.str, "%s.w = tex2D(s%d, gth.xy * SIZETEX.zw)%s", output, res,swizl);
					AddInst(vp, c);
				}
            }
        }
        return output;
    }

	if (str_match0(chaine,"mul_x2"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}

		if ((api == 0) && (!shadermodel3))
		{
			_sprintf(c.str,"mul_x2 %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = 2.0 * %s * %s",output,rn,rn2);
				AddInst(vp,c);
			}
			else
			{

				_sprintf(c.str,"ADD_SAT %s,%s,%s",rn,rn,rn);
				AddInst(vp,c);
				_sprintf(c.str,"MUL_SAT %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}
		return output;
	}


	if ( (str_match0(chaine,"reciprocal")) || (str_match0(chaine,"rcp")) )
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"rcp %s,%s.w",output,rn);
			AddInst(vp,c);
		}
		else
		{
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str,"%s = 1.0f / %s.w",output,rn);
				AddInst(vp,c);
			}
			else
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s = 1.0 / %s.w",output,rn);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"RCP %s,%s.w",output,rn);
				AddInst(vp,c);
			}
		}
		return output;
	}

	
	if (str_match0(chaine,"mul_x4"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"mul_x4 %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = 4.0 * %s * %s",output,rn,rn2);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"ADD_SAT %s,%s,%s",rn,rn,rn);
				AddInst(vp,c);
				_sprintf(c.str,"ADD_SAT %s,%s,%s",rn2,rn2,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"MUL_SAT %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}
		return output;
	}

	if (str_match0(chaine,"mul_sat"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}


		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"mul_sat %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s = clamp( %s * %s , 0.0, 1.0)",output,rn,rn2);
				AddInst(vp,c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str,"%s = clamp( %s * %s , 0.0f, 1.0f)",output,rn,rn2);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"MUL_SAT %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if (str_match0(chaine,"mul"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}
		
		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"mul %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = %s * %s",output,rn,rn2);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"MUL %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if (str_match0(chaine,"InsideBall"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(tmp, tmp2, vp);
		}

		_sprintf(tmp3, "r%d", new_temp_register());

		// pos=InsideBall(XYZ,R)

		if (api==1)
		{
			_sprintf(c.str,"if (%s.w>0.001)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"%s.w = length(%s.xyz)",tmp3,tmp3);
			AddInst(vp,c);
			_sprintf(c.str,"if (%s.w<%s.w) %s;",tmp3,rn,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}
		else
		{
			_sprintf(c.str,"if (%s.w>0.001f)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"%s.w = length(%s.xyz)",tmp3,tmp3);
			AddInst(vp,c);
			_sprintf(c.str,"if (%s.w<%s.w) %s",tmp3,rn,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}

		return output;
	}
	else
	if (str_match0(chaine,"InsideCube"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(tmp, tmp2, vp);
		}

		_sprintf(tmp3, "r%d", new_temp_register());

		// pos=InsideCube(XYZ,L)

		if (api==1)
		{
			_sprintf(c.str,"if (%s.w>0.001)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"if ((abs(%s.x)<%s.w)&&(abs(%s.y)<%s.w)&&(abs(%s.z)<%s.w)) %s;",tmp3,rn,tmp3,rn,tmp3,rn,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}
		else
		{
			_sprintf(c.str,"if (%s.w>0.001f)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"if ((abs(%s.x)<%s.w)&&(abs(%s.y)<%s.w)&&(abs(%s.z)<%s.w)) %s",tmp3,rn,tmp3,rn,tmp3,rn,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}

		return output;
	}
	else
	if (str_match0(chaine,"InsideBox"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn2 = compile(&tmp[p + 1], tmp4, vp);
		}

		_sprintf(tmp3, "r%d", new_temp_register());

		// pos=InsideBox(XYZ,Lxyz)

		if (api==1)
		{
			_sprintf(c.str,"if (%s.w>0.001)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"if ((abs(%s.x)<%s.x)&&(abs(%s.y)<%s.y)&&(abs(%s.z)<%s.z)) %s;",tmp3,rn2,tmp3,rn2,tmp3,rn2,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}
		else
		{
			_sprintf(c.str,"if (%s.w>0.001f)",rn);
			AddInst2(vp,c);
			_sprintf(c.str,"{");
			AddInst2(vp,c);
			_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,output);
			AddInst(vp,c);
			_sprintf(c.str,"if ((abs(%s.x)<%s.x)&&(abs(%s.y)<%s.y)&&(abs(%s.z)<%s.z)) %s",tmp3,rn2,tmp3,rn2,tmp3,rn2,sDiscardAPI);
			AddInst(vp,c);
			_sprintf(c.str,"}");
			AddInst2(vp,c);
		}

		return output;
	}

	if (str_match0(chaine,"Lighting"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=4)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            
            rn=var(rns[0]);
            rn2=var(rns[1]);
			rn3=var(rns[2]);
			rn4=var(rns[3]);

			_sprintf(tmp3, "r%d", new_temp_register());
			_sprintf(tmp4, "r%d", new_temp_register());

			// Lighting(pos,N,LIGHT,COLLIGHT)

			if (api==1)
			{
				_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,rn3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.w = length(%s.xyz)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = dot(%s.xyz,%s.xyz)",tmp4,tmp3,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = clamp(-%s.y,0.0,1.0)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.w/%s.w",tmp4,rn3,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s = vec4(0.0,0.0,0.0,0.0)",output);
				AddInst(vp,c);
				_sprintf(c.str,"if (%s.x<%s.w)",tmp4,rn4);
				AddInst2(vp,c);
				_sprintf(c.str,"{");
				AddInst2(vp,c);
				_sprintf(c.str,"%s.rgb=%s.rgb*%s.x*%s.y",output,rn4,tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"}");
				AddInst2(vp,c);
			}
			else
			if (api==0)
			{
				_sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp3,rn,rn3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.w = length(%s.xyz)",tmp4,tmp3);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = dot(%s.xyz,%s.xyz)",tmp4,tmp3,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.y = clamp(-%s.y,0.0f,1.0f)",tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = %s.w/%s.w",tmp4,rn3,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s = float4(0.0f,0.0f,0.0f,0.0f)",output);
				AddInst(vp,c);
				_sprintf(c.str,"if (%s.x<%s.w)",tmp4,rn4);
				AddInst2(vp,c);
				_sprintf(c.str,"{");
				AddInst2(vp,c);
				_sprintf(c.str,"%s.rgb=%s.rgb*%s.x*%s.y",output,rn4,tmp4,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"}");
				AddInst2(vp,c);
			}
		}
		return output;
	}	

	if (str_match0(chaine,"Polynomial"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=5)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=var(rns[0]);

			// Polynomial(col,a,b,c,d)
			// libpolynomial(vec3 col,float a,float b,float c,float d)

			if (api==1)
			{
				if ((str_last_char(output, '.') == -1)&&(str_last_char(rn, '.') == -1))
				{
					_sprintf(c.str,"%s.rgb = libpolynomial(%s.rgb,vec4(%s,%s,%s,%s))",output,rn,rns[1],rns[2],rns[3],rns[4]);
					AddInst(vp,c);
				}
				else
				{
					if ((str_last_char(output, '.') == -1)&&(str_last_char(rn, '.') != -1))
					{
						_sprintf(c.str,"%s.rgb = libpolynomial(%s,vec4(%s,%s,%s,%s))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
					else
					if ((str_last_char(output, '.') != -1)&&(str_last_char(rn, '.') == -1))
					{
						_sprintf(c.str,"%s = libpolynomial(%s.rgb,vec4(%s,%s,%s,%s))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
					else
					{
						_sprintf(c.str,"%s = libpolynomial(%s,vec4(%s,%s,%s,%s))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
				}
			}
			else
			{
				if ((str_last_char(output, '.') == -1)&&(str_last_char(rn, '.') == -1))
				{
					_sprintf(c.str,"%s.rgb = libpolynomial(%s.rgb,float4(%sf,%sf,%sf,%sf))",output,rn,rns[1],rns[2],rns[3],rns[4]);
					AddInst(vp,c);
				}
				else
				{
					if ((str_last_char(output, '.') == -1)&&(str_last_char(rn, '.') != -1))
					{
						_sprintf(c.str,"%s.rgb = libpolynomial(%s,float4(%sf,%sf,%sf,%sf))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
					else
					if ((str_last_char(output, '.') != -1)&&(str_last_char(rn, '.') == -1))
					{
						_sprintf(c.str,"%s = libpolynomial(%s.rgb,float4(%sf,%sf,%sf,%sf))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
					else
					{
						_sprintf(c.str,"%s = libpolynomial(%s,float4(%sf,%sf,%sf,%sf))",output,rn,rns[1],rns[2],rns[3],rns[4]);
						AddInst(vp,c);
					}
				}
			}
		}
		return output;
	}

	if (str_match0(chaine,"ShadowMappingSimple"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=3)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=rns[0];
            rn2=var(rns[1]);
			rn3=var(rns[2]);

			// ShadowSimple(tex,pos,zval)

			_sprintf(c.str,"%s = libcalcsimpleshadow%s(%s,%s)",output,rn,rn2,rn3);
			AddInst(vp,c);
		}
		return output;
	}

	if (str_match0(chaine,"TextureSpaceSimple"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=3)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
			rn=var(rns[1]);
			rn2=var(rns[2]);

			// TextureSpace(tex,pos,mouss)
			// "vec3 libmouss%s(vec3 pos,vec4 mouss)"

			if (str_last_char(rn2, '.') != -1)
			{
			    SYNTAXERROR=PARAMERROR=true;
		        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
	            return "";
			}

			if (str_last_char(output, '.') == -1)
			{
				if (str_last_char(rn, '.') == -1) _sprintf(c.str,"%s.rgb = libsimplemouss%s(%s.xyz,%s)",output,rns[0],rn,rn2);
				else _sprintf(c.str,"%s.rgb = libsimplemouss%s(%s,%s)",output,rns[0],rn,rn2);
			}
			else
			{
				if (str_last_char(rn, '.') == -1) _sprintf(c.str,"%s = libsimplemouss%s(%s.xyz,%s)",output,rns[0],rn,rn2);
				else _sprintf(c.str,"%s = libsimplemouss%s(%s,%s)",output,rns[0],rn,rn2);
			}
			AddInst(vp,c);
		}
		return output;
	}
	else
	if (str_match0(chaine,"TextureSpace"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=5)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
			rn=var(rns[1]);
			rn2=var(rns[2]);
			rn3=var(rns[3]);
			rn4=var(rns[4]);

			// TextureSpace(tex,pos,deform,mouss,time)
			// "vec3 libmouss%s(vec3 pos,vec2 deform,vec4 mouss,vec2 time)"

			if (str_last_char(rn3, '.') != -1)
			{
			    SYNTAXERROR=PARAMERROR=true;
		        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
	            return "";
			}

			if ((nb_modifiers(rn2)!=2)||(nb_modifiers(rn4)!=2))
			{
			    SYNTAXERROR=PARAMERROR=true;
		        if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
	            return "";
			}

			if (str_last_char(output, '.') == -1)
			{
				if (str_last_char(rn, '.') == -1) _sprintf(c.str,"%s.rgb = libmouss%s(%s.xyz,%s,%s,%s)",output,rns[0],rn,rn2,rn3,rn4);
				else _sprintf(c.str,"%s.rgb = libmouss%s(%s,%s,%s,%s)",output,rns[0],rn,rn2,rn3,rn4);
			}
			else
			{
				if (str_last_char(rn, '.') == -1) _sprintf(c.str,"%s = libmouss%s(%s.xyz,%s,%s,%s)",output,rns[0],rn,rn2,rn3,rn4);
				else _sprintf(c.str,"%s = libmouss%s(%s,%s,%s,%s)",output,rns[0],rn,rn2,rn3,rn4);
			}
			AddInst(vp,c);
		}
		return output;
	}

	if (str_match0(chaine,"ShadowMappingSoftExtended"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=4)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=rns[0];
            rn2=var(rns[1]);
			rn3=var(rns[2]);
			rn4=var(rns[3]);

			// ShadowSoft(tex,pos,zval,blur)

			_sprintf(c.str,"%s = libcalcshadowsoftX%s(%s,%s,%s)",output,rn,rn2,rn3,rn4);
			AddInst(vp,c);
		}
		return output;
	}

	if (str_match0(chaine,"ShadowMappingSoft"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=3)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=rns[0];
            rn2=var(rns[1]);
			rn3=var(rns[2]);

			// ShadowSoft(tex,pos,zval)

			_sprintf(c.str,"%s = libcalcshadowsoft%s(%s,%s)",output,rn,rn2,rn3);
			AddInst(vp,c);
		}
		return output;
	}

	if (str_match0(chaine,"ShadowMappingBilinear"))
	{
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=4)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=rns[0];
            rn2=var(rns[1]);
			rn3=var(rns[2]);
			rn4=rns[3];

			// ShadowBilinear(tex,pos,zval,SIZE)

			_sprintf(c.str,"%s = libshadowbilinear%s(%s,%s)",output,rn,rn2,rn3);
			AddInst(vp,c);
		}
		return output;
	}

	if (str_match0(chaine,"dot_sat"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"dp3_sat %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s.x = clamp( dot( %s.xyz , %s.xyz ) , 0.0, 1.0 )",output,rn,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = %s.xxxx",output,output);
				AddInst(vp,c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str,"%s.x = saturate( dot( %s.xyz , %s.xyz ) )",output,rn,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s = %s.xxxx",output,output);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"DP3_SAT %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if ((str_match0(chaine,"dotofbx2"))||(str_match0(chaine,"dotbx2")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp5, "r%d", new_temp_register());
			resfield(output, tmp5);
			rn2 = compile(&tmp[p + 1], tmp5, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			if (shadermodel)
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				_sprintf(tmp4,"r%d",new_temp_register());
				
				_sprintf(c.str,"sub %s,%s,%s",tmp3,rn,var("zerocinq"));
				AddInst(vp,c);
				_sprintf(c.str,"sub %s,%s,%s",tmp4,rn2,var("zerocinq"));
				AddInst(vp,c);
				_sprintf(c.str,"dp3 %s,%s,%s",output,tmp3,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"add %s,%s,%s",output,output,output);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"dp3_x2 %s,%s_bias,-%s_bias",output,rn,rn2);
				AddInst(vp,c);
			}
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				_sprintf(tmp4,"r%d",new_temp_register());

				_sprintf(c.str,"%s.xyz = %s.xyz - vec3( 0.5, 0.5, 0.5)",tmp3,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = %s.xyz - vec3( 0.5, 0.5, 0.5)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = dot( %s.xyz , %s.xyz )",output,tmp3,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s = 4.0 * %s.xxxx",output,output);
				AddInst(vp,c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				_sprintf(tmp4,"r%d",new_temp_register());

				_sprintf(c.str,"%s.xyz = %s.xyz - float3( 0.5f, 0.5f, 0.5f)",tmp3,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = %s.xyz - float3( 0.5f, 0.5f, 0.5f)",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.x = dot( %s.xyz , %s.xyz )",output,tmp3,tmp4);
				AddInst(vp,c);
				_sprintf(c.str,"%s = 4.0f * %s.xxxx",output,output);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				_sprintf(tmp4,"r%d",new_temp_register());
				_sprintf(c.str,"SUB %s,%s,zerocinq",tmp3,rn);
				AddInst(vp,c);
				_sprintf(c.str,"SUB %s,%s,zerocinq",tmp4,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"DP3_SAT %s,%s,%s",output,tmp3,tmp4);
				AddInst(vp,c);
			}
		}
		return output;
	}

	if (str_match0(chaine,"dot4"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"dp4 %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				if (str_char(rn,'.')==-1) _sprintf(tmp2,"%s",rn);
				else _sprintf(tmp2,"%s",rn);

				if (str_char(rn2,'.')==-1) _sprintf(tmp3,"%s",rn2);
				else _sprintf(tmp3,"%s",rn2);

				if (str_char(output,'.')!=-1)
				{	
					_sprintf(c.str,"%s = dot( %s , %s )",output,tmp2,tmp3);
					AddInst(vp,c);
				}
				else
				{	
					_sprintf(c.str,"%s.x = dot( %s , %s )",output,tmp2,tmp3);
					AddInst(vp,c);
					_sprintf(c.str,"%s = %s.xxxx",output,output);
					AddInst(vp,c);
				}
			}
			else
			{
				_sprintf(c.str,"DP4 %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}
	else
	if (str_match0(chaine,"dot"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			rn2 = compile(&tmp[p + 1], tmp3, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"dp3 %s,%s,%s",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				if (str_char(rn,'.')==-1) _sprintf(tmp2,"%s.xyz",rn);
				else _sprintf(tmp2,"%s",rn);

				if (str_char(rn2,'.')==-1) _sprintf(tmp3,"%s.xyz",rn2);
				else _sprintf(tmp3,"%s",rn2);

				if (str_char(output,'.')!=-1)
				{	
					_sprintf(c.str,"%s = dot( %s , %s )",output,tmp2,tmp3);
					AddInst(vp,c);
				}
				else
				{	
					_sprintf(c.str,"%s.x = dot( %s , %s )",output,tmp2,tmp3);
					AddInst(vp,c);
					_sprintf(c.str,"%s = %s.xxxx",output,output);
					AddInst(vp,c);
				}
			}
			else
			{
				_sprintf(c.str,"DP3 %s,%s,%s",output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

    if (str_match0(chaine,"scale"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
        tmp[p]='\0';
        
        if ((str_simple(&tmp[0]))&&(str_char(output,'.')==-1))
        {
            rn=var(&tmp[0]);
        
            if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
            else
            {
                _sprintf(tmp3, "r%d", new_temp_register());
                rn2 = compile(&tmp[p + 1], tmp3, vp);
            }
        
            if (api==1)
            {
                _sprintf(c.str, "%s.xyz = %s.xyz * %s", output, rn, rn2);
                AddInst(vp, c);
                _sprintf(c.str, "%s.w = 1.0", output);
                AddInst(vp, c);
                return output;
            }
            else    // api 0 = D3D or METAL
            {
                if (metal==1)
                {
                    _sprintf(c.str, "%s.xyz = %s.xyz * %s", output, rn, rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.w = 1.0", output);
                    AddInst(vp, c);
                    return output;
                }
                else
                {
                    _sprintf(c.str, "%s.xyz = %s.xyz * %s", output, rn, rn2);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.w = 1.0f", output);
                    AddInst(vp, c);
                    return output;
                }
            }
            return output;
        }
    }
    
    if (str_match0(chaine,"HalfScale"))
    {
        char *str=str_return_parentheses(chaine);
        
        _sprintf(tmp2,"r%d",new_temp_register());
        if (str_simple(str)) rn=var(str);
        else rn=compile(str,tmp2,vp);

        if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
        {
            if (api==0) _sprintf(c.str,"%s.xyz = %s.xyz * 2.0f - float3(1.0f,1.0f,1.0f)",output,rn);
            else _sprintf(c.str,"%s.xyz = %s.xyz * 2.0 - vec3(1.0,1.0,1.0)",output,rn);
            AddInst(vp,c);
            if (api==0) _sprintf(c.str,"%s.w = 0.0f",output);
            else _sprintf(c.str,"%s.w = 0.0",output);
            AddInst(vp,c);
        }
        
        return output;
    }

    if ((str_match0(chaine,"HalfScaleNormalize"))||(str_match0(chaine,"HalfScaleNorm")))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        _sprintf(tmp2,"r%d",new_temp_register());
        if (str_simple(str)) rn=var(str);
        else rn=compile(str,tmp2,vp);

        if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
        {
            if (api==0) _sprintf(c.str,"%s.xyz = %s.xyz - float3(0.5f,0.5f,0.5f)",output,rn);
            else _sprintf(c.str,"%s.xyz = %s.xyz - vec3(0.5,0.5,0.5)",output,rn);
            AddInst(vp,c);
            _sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,output);
            AddInst(vp,c);
            if (api==0) _sprintf(c.str,"%s.w = 0.0f",output);
            else _sprintf(c.str,"%s.w = 0.0",output);
            AddInst(vp,c);
        }
        
        return output;
    }

    if (str_match0(chaine,"FallOffRangeNorm"))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        
        _sprintf(tmp2,"r%d",new_temp_register());
        if (str_simple(str)) rn=var(str);
        else rn=compile(str,tmp2,vp);

        if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
        {
/*
            //val.w = norm( u.xyz );
            val.x = val.w - value_coef.y;
            val.y = val.x * value_coef.z;
            val.y = value_one.x - val.y;
            val.z = compare( val.x, val.y, value_one.x);
            val.x = val.w - value_coef.x;
            val.y = compare( val.x, value_zero.x, val.z);
            val.z = val.z * val.y;
/**/
            char *val=output;
            char *coef=rn;

            _sprintf(c.str, "%s.w = norm( %s.xyz )",val,val);
            AddInst(vp, c);
            _sprintf(c.str, "if (%s.w < %s.y)", val, coef);
            AddInst2(vp, c);
            _sprintf(c.str, "{");
            AddInst2(vp, c);
                if (api==1) _sprintf(c.str, "%s.z = 1.0",val); else _sprintf(c.str, "%s.z = 1.0f",val);
                AddInst(vp, c);
            _sprintf(c.str, "}");
            AddInst2(vp, c);
            _sprintf(c.str, "else");
            AddInst2(vp, c);
            _sprintf(c.str, "{");
            AddInst2(vp, c);

                _sprintf(c.str, "if (%s.w > %s.x)", val, coef);
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);
                    if (api==1) _sprintf(c.str, "%s.z = 0.0",val); else _sprintf(c.str, "%s.z = 0.0f",val);
                    AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst2(vp, c);
                _sprintf(c.str, "else");
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);

                    _sprintf(c.str,"%s.z = ( %s.x - %s.w ) * %s.z",val,coef,val,coef);
                    AddInst(vp,c);

                _sprintf(c.str, "}");
                AddInst2(vp, c);

            _sprintf(c.str, "}");
            AddInst2(vp, c);

/*
            _sprintf(c.str,"%s.x = %s.w - %s.y",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = %s.x * %s.z",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = 1.0 - %s.y",val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.z = ( %s.x >= 0.0 ) ? %s.y : 1.0", val,val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.x = %s.w - %s.x",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = ( %s.x >= 0.0 ) ? 0.0 : %s.z", val,val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.z = %s.z * %s.y",val,val,val);
            AddInst(vp,c);
/**/
        }

        return output;
    }
    else
    if (str_match0(chaine,"FallOffRangeValue"))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=2)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            
            rn2=var(rns[0]);
            rn=var(rns[1]);

//            if (str_simple(str)) rn=var(str);
//            else rn=compile(str,tmp2,vp);

            if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
            {
    /*
                //val.w = norm( u.xyz );
                val.x = val.w - value_coef.y;
                val.y = val.x * value_coef.z;
                val.y = value_one.x - val.y;
                val.z = compare( val.x, val.y, value_one.x);
                val.x = val.w - value_coef.x;
                val.y = compare( val.x, value_zero.x, val.z);
                val.z = val.z * val.y;
    /**/
                char *val=output;
                char *coef=rn;

                _sprintf(c.str, "if (%s < %s.y)", rn2, coef);
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);
                    if (api==1) _sprintf(c.str, "%s.z = 1.0",val); else _sprintf(c.str, "%s.z = 1.0f",val);
                    AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst2(vp, c);
                _sprintf(c.str, "else");
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);

                    _sprintf(c.str, "if (%s > %s.x)", rn2, coef);
                    AddInst2(vp, c);
                    _sprintf(c.str, "{");
                    AddInst2(vp, c);
                        if (api==1) _sprintf(c.str, "%s.z = 0.0",val); else _sprintf(c.str, "%s.z = 0.0f",val);
                        AddInst(vp, c);
                    _sprintf(c.str, "}");
                    AddInst2(vp, c);
                    _sprintf(c.str, "else");
                    AddInst2(vp, c);
                    _sprintf(c.str, "{");
                    AddInst2(vp, c);

                        _sprintf(c.str,"%s.z = ( %s.x - %s ) * %s.z",val,coef,rn2,coef);
                        AddInst(vp,c);

                    _sprintf(c.str, "}");
                    AddInst2(vp, c);

                _sprintf(c.str, "}");
                AddInst2(vp, c);

/*
                _sprintf(c.str,"%s.x = %s - %s.y",val,rn2,coef);
                AddInst(vp,c);
                _sprintf(c.str,"%s.y = %s.x * %s.z",val,val,coef);
                AddInst(vp,c);
                _sprintf(c.str,"%s.y = 1.0 - %s.y",val,val);
                AddInst(vp,c);
                _sprintf(c.str,"%s.z = ( %s.x >= 0.0 ) ? %s.y : 1.0", val,val,val);
                AddInst(vp,c);
                _sprintf(c.str,"%s.x = %s - %s.x",val,rn2,coef);
                AddInst(vp,c);
                _sprintf(c.str,"%s.y = ( %s.x >= 0.0 ) ? 0.0 : %s.z", val,val,val);
                AddInst(vp,c);
                _sprintf(c.str,"%s.z = %s.z * %s.y",val,val,val);
                AddInst(vp,c);
/**/
            }
        }
        return output;
    }
    else
    if (str_match0(chaine,"FallOffRange"))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        _sprintf(tmp2,"r%d",new_temp_register());
        if (str_simple(str)) rn=var(str);
        else rn=compile(str,tmp2,vp);

        if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
        {
/*
            //val.w = norm( u.xyz );
            val.x = val.w - value_coef.y;
            val.y = val.x * value_coef.z;
            val.y = value_one.x - val.y;
            val.z = compare( val.x, val.y, value_one.x);
            val.x = val.w - value_coef.x;
            val.y = compare( val.x, value_zero.x, val.z);
            val.z = val.z * val.y;
/**/
            char *val=output;
            char *coef=rn;

            _sprintf(c.str, "if (%s.w < %s.y)", val, coef);
            AddInst2(vp, c);
            _sprintf(c.str, "{");
            AddInst2(vp, c);
                if (api==1) _sprintf(c.str, "%s.z = 1.0",val); else _sprintf(c.str, "%s.z = 1.0f",val);
                AddInst(vp, c);
            _sprintf(c.str, "}");
            AddInst2(vp, c);
            _sprintf(c.str, "else");
            AddInst2(vp, c);
            _sprintf(c.str, "{");
            AddInst2(vp, c);

                _sprintf(c.str, "if (%s.w > %s.x)", val, coef);
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);
                    if (api==1) _sprintf(c.str, "%s.z = 0.0",val); else _sprintf(c.str, "%s.z = 0.0f",val);
                    AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst2(vp, c);
                _sprintf(c.str, "else");
                AddInst2(vp, c);
                _sprintf(c.str, "{");
                AddInst2(vp, c);

                    _sprintf(c.str,"%s.z = ( %s.x - %s.w ) * %s.z",val,coef,val,coef);
                    AddInst(vp,c);

                _sprintf(c.str, "}");
                AddInst2(vp, c);

            _sprintf(c.str, "}");
            AddInst2(vp, c);
            
/*
            _sprintf(c.str,"%s.x = %s.w - %s.y",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = %s.x * %s.z",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = 1.0 - %s.y",val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.z = ( %s.x >= 0.0 ) ? %s.y : 1.0", val,val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.x = %s.w - %s.x",val,val,coef);
            AddInst(vp,c);
            _sprintf(c.str,"%s.y = ( %s.x >= 0.0 ) ? 0.0 : %s.z", val,val,val);
            AddInst(vp,c);
            _sprintf(c.str,"%s.z = %s.z * %s.y",val,val,val);
            AddInst(vp,c);
/**/
        }

        return output;
    }

    if (str_match0(chaine,"CompleteFallOffRangeValue"))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=3)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn2=var(rns[0]);
            rn=var(rns[1]);
            char * rii=var(rns[2]);
            
            if (str_char(rii,'.')==-1)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
                {
                    //val.w = norm( u.xyz );
                    char *val=output;
                    char *coef=rn;

                    _sprintf(c.str, "if (%s > %s.y)", rn2, coef);
                    AddInst2(vp, c);
                    _sprintf(c.str, "{");
                    AddInst2(vp, c);

                        _sprintf(c.str, "if (%s > %s.x)", rn2, coef);
                        AddInst2(vp, c);
                        _sprintf(c.str, "{");
                        AddInst2(vp, c);
                            if (api==1) _sprintf(c.str, "%s = 0.0",rii); else _sprintf(c.str, "%s = 0.0f",rii);
                            AddInst(vp, c);
                        _sprintf(c.str, "}");
                        AddInst2(vp, c);
                        _sprintf(c.str, "else");
                        AddInst2(vp, c);
                        _sprintf(c.str, "{");
                        AddInst2(vp, c);

                            _sprintf(c.str,"%s *= ( %s.x - %s ) * %s.z",rii,coef,rn2,coef);
                            AddInst(vp,c);

                        _sprintf(c.str, "}");
                        AddInst2(vp, c);

                    _sprintf(c.str, "}");
                    AddInst2(vp, c);

                }
            }
        }
        return output;
    }
    else
    if (str_match0(chaine,"CompleteFallOffRange"))
    {
        char *str=str_return_parentheses(chaine);
        
        if (strlen(str)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (multiple_argument_parse_virg(str,rns)!=2)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            rn=var(rns[0]);
            char * rii=var(rns[1]);
            if (str_char(rii,'.')==-1)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {

                if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
                {
                    //val.w = norm( u.xyz );
                    char *val=output;
                    char *coef=rn;

                    _sprintf(c.str, "if (%s.w > %s.y)", val, coef);
                    AddInst2(vp, c);
                    _sprintf(c.str, "{");
                    AddInst2(vp, c);

                        _sprintf(c.str, "if (%s.w > %s.x)", val, coef);
                        AddInst2(vp, c);
                        _sprintf(c.str, "{");
                        AddInst2(vp, c);
                            if (api==1) _sprintf(c.str, "%s = 0.0",rii); else _sprintf(c.str, "%s = 0.0f",rii);
                            AddInst(vp, c);
                        _sprintf(c.str, "}");
                        AddInst2(vp, c);
                        _sprintf(c.str, "else");
                        AddInst2(vp, c);
                        _sprintf(c.str, "{");
                        AddInst2(vp, c);

                            _sprintf(c.str,"%s *= ( %s.x - %s.w ) * %s.z",rii,coef,val,coef);
                            AddInst(vp,c);

                        _sprintf(c.str, "}");
                        AddInst2(vp, c);

                    _sprintf(c.str, "}");
                    AddInst2(vp, c);
                }
            }
        }
        return output;
    }

    
    if (str_match0(chaine,"load"))
    {
        char *str=str_return_parentheses(chaine);
        
        if ((str_simple(str))&&(str_char(output,'.')==-1))
        {
            rn=var(str);
 
            if (api==1)
            {
                _sprintf(c.str, "%s.xyz = %s.xyz", output, rn);
                AddInst(vp, c);
                _sprintf(c.str, "%s.w = 1.0", output);
                AddInst(vp, c);
                return output;
            }
            else    // api 0 = D3D or METAL
            {
                if (metal==1)
                {
                    _sprintf(c.str, "%s.xyz = %s.xyz", output, rn);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.w = 1.0f", output);
                    AddInst(vp, c);
                    return output;
                }
                else
                {
                    _sprintf(c.str, "%s.xyz = %s.xyz", output, rn);
                    AddInst(vp, c);
                    _sprintf(c.str, "%s.w = 1.0f", output);
                    AddInst(vp, c);
                    return output;
                }
            }
            return output;
        }
    }

	if (str_match0(chaine,"frc"))
	{
		char *str=str_return_parentheses(chaine);
		
		_sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"frc %s,%s",output,rn);
			AddInst(vp,c);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str, "%s = %s", output, rn);
				AddInst(vp, c);
				return output;
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				if (shadermodel4)
				{
					_sprintf(c.str, "%s = %s", output, rn);
					AddInst(vp, c);
					return output;
				}
				else
				{
                    if (metal==1)
                    {
                        outputfield1(tmp4, output, rn);
                        _sprintf(c.str, "%s = fract( %s )", tmp4, rn);
                        AddInst(vp, c);
                        return tmp4;
                    }
                    else
                    {
                        outputfield1(tmp4, output, rn);
                        _sprintf(c.str, "%s = frac( %s )", tmp4, rn);
                        AddInst(vp, c);
                        return tmp4;
                    }
				}
			}
			else
			{
				_sprintf(c.str,"FRC %s,%s",output,rn);
				AddInst(vp,c);
			}
		}

		return output;
	}


	if ((str_match0(chaine,"fractionnal"))||(str_match0(chaine,"frac"))||(str_match0(chaine,"fract")))
	{
		char *str=str_return_parentheses(chaine);
		
		_sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"frc %s,%s",output,rn);
			AddInst(vp,c);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = fract( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				outputfield1(tmp4, output, rn);
				if (metal==1) _sprintf(c.str, "%s = fract( %s )", tmp4, rn);
				else _sprintf(c.str, "%s = frac( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str,"FRC %s,%s",output,rn);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if ((str_match0(chaine, "cmp")) || (str_match0(chaine, "compare")))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1] = '\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2] = '\0';

		if (str_simple(&tmp[0])) rn = var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p1 + 1])) rn2 = var(&tmp[p1 + 1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p1 + 1], tmp3, vp);
		}

		if (str_simple(&tmp[p2 + 1])) rn3 = var(&tmp[p2 + 1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn3 = compile(&tmp[p2 + 1], tmp4, vp);
		}

		if ((api == 0) && (!shadermodel3))
		{
			_sprintf(c.str, "cmp %s,%s,%s,%s", output, rn, rn2, rn3);
			AddInst(vp, c);
		}
		else
		{
			if ((api == 1) && (shadermodel))
			{
				int sc=str_last_char(rn,'.');
				if (sc==-1)
				{
					_sprintf(c.str, "%s.x = ( %s.x >= 0.0 ) ? %s.x : %s.x", output, rn, rn2, rn3);
					AddInst(vp, c);
					_sprintf(c.str, "%s.y = ( %s.y >= 0.0 ) ? %s.y : %s.y", output, rn, rn2, rn3);
					AddInst(vp, c);
					_sprintf(c.str, "%s.z = ( %s.z >= 0.0 ) ? %s.z : %s.z", output, rn, rn2, rn3);
					AddInst(vp, c);
					_sprintf(c.str, "%s.w = ( %s.w >= 0.0 ) ? %s.w : %s.w", output, rn, rn2, rn3);
					AddInst(vp, c);
				}
				else
				{				
					_sprintf(c.str, "%s = ( %s >= 0.0 ) ? %s : %s", output, rn, rn2, rn3);
					AddInst(vp, c);
				}
			}
			else
			if ((api == 0) && (shadermodel3))
			{
				if (metal==1)
				{
					int sc=str_last_char(rn,'.');
					if (sc==-1)
					{
						_sprintf(c.str, "%s.x = ( %s.x >= 0.0f ) ? %s.x : %s.x", output, rn, rn2, rn3);
						AddInst(vp, c);
						_sprintf(c.str, "%s.y = ( %s.y >= 0.0f ) ? %s.y : %s.y", output, rn, rn2, rn3);
						AddInst(vp, c);
						_sprintf(c.str, "%s.z = ( %s.z >= 0.0f ) ? %s.z : %s.z", output, rn, rn2, rn3);
						AddInst(vp, c);
						_sprintf(c.str, "%s.w = ( %s.w >= 0.0f ) ? %s.w : %s.w", output, rn, rn2, rn3);
						AddInst(vp, c);
					}
					else
					{				
						_sprintf(c.str, "%s = ( %s >= 0.0f ) ? %s : %s", output, rn, rn2, rn3);
						AddInst(vp, c);
					}
				}
				else
				{
					_sprintf(c.str, "%s = ( %s >= 0.0f ) ? %s : %s", output, rn, rn2, rn3);
					AddInst(vp, c);
				}
			}
			else
			{
				_sprintf(c.str, "CMP %s,%s,%s,%s", output, rn, rn2, rn3);
				AddInst(vp, c);
			}
		}

		return output;
	}

	if (str_match0(chaine, "refract"))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1] = '\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2] = '\0';

		if (str_simple(&tmp[0])) rn = var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p1 + 1])) rn2 = var(&tmp[p1 + 1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p1 + 1], tmp3, vp);
		}

		if (str_simple(&tmp[p2 + 1])) rn3 = var(&tmp[p2 + 1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn3 = compile(&tmp[p2 + 1], tmp4, vp);
		}

		if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
		{
			_sprintf(c.str, "%s = refract( %s , %s , %s)", output, rn, rn2, rn3);
			AddInst(vp, c);
		}

		return output;
	}

	if ((str_match0(chaine, "mix_sat"))||(str_match0(chaine, "lerp_sat")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p1 + 1], tmp3, vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn3 = compile(&tmp[p2 + 1], tmp4, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"lrp_sat %s,%s,%s,%s",output,rn3,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if ((api==0)&&(shadermodel3))
			{
				if (metal==1)
				{
					_sprintf(c.str,"%s = saturate( mix( %s , %s , %s) )",output,rn,rn2,rn3);
					AddInst(vp,c);
				}
				else
				{
					_sprintf(c.str,"%s = saturate( lerp( %s , %s , %s) )",output,rn,rn2,rn3);
					AddInst(vp,c);
				}
			}
			else
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s =  saturate( mix( %s , %s , %s  ) )",output,rn,rn2,rn3);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"LRP_SAT %s,%s,%s,%s",output,rn3,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}
	else
	if ((str_match0(chaine, "mix"))||(str_match0(chaine, "lerp")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p1 + 1], tmp3, vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn3 = compile(&tmp[p2 + 1], tmp4, vp);
		}

		if ((api==0)&&(!shadermodel3))
		{
			_sprintf(c.str,"lrp %s,%s,%s,%s",output,rn3,rn,rn2);
			AddInst(vp,c);
		}
		else
		{
			if ((api==0)&&(shadermodel3))
			{
				if (metal==1)
				{
					_sprintf(c.str,"%s =  mix( %s , %s , %s  )",output,rn,rn2,rn3);
					AddInst(vp,c);
				}
				else
				{
					_sprintf(c.str,"%s = lerp( %s , %s , %s)",output,rn,rn2,rn3);
					AddInst(vp,c);
				}
			}
			else
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s =  mix( %s , %s , %s  )",output,rn,rn2,rn3);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"LRP %s,%s,%s,%s",output,rn3,rn,rn2);
				AddInst(vp,c);
			}
		}
		
		return output;
	}


	if (str_match0(chaine,"smoothstep"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(&tmp[0], tmp2, vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp3, "r%d", new_temp_register());
			resfield(output, tmp3);
			rn2 = compile(&tmp[p1 + 1], tmp3, vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp4, "r%d", new_temp_register());
			resfield(output, tmp4);
			rn3 = compile(&tmp[p2 + 1], tmp4, vp);
		}

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			_sprintf(c.str,"%s = smoothstep( %s , %s , %s )",output,rn,rn2,rn3);
			AddInst(vp,c);
		}

		return output;
	}

	if (str_match0(chaine,"coord"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		rn=var(tmp);

		if (api==0)
		{
			if (!shadermodel)
			{
				if (str_char(output,'.')==-1) _sprintf(c.str,"texcrd %s.rgb,%s",output,rn);
				else _sprintf(c.str,"texcrd %s,%s",output,rn);
			}
			else
			{
				if (shadermodel3) _sprintf(c.str,"%s = %s",output,rn);
				else _sprintf(c.str,"mov %s,%s",output,rn);
			}
			AddInst(vp,c);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s = %s",output,rn);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"MOV %s,%s",output,rn);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if ((str_match0(chaine,"normalise"))||(str_match0(chaine,"normalize")))
	{
		if (str_simple(str_return_parentheses(chaine)))
		{
			rn=var(str_return_parentheses(chaine));
		}
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(str_return_parentheses(chaine), tmp2, vp);
		}

		_sprintf(rn1,"r%d",new_temp_register());

		if (api==2)
		{
			em->Add(OP_DP3,rn1,rn,rn);
			em->Add(OP_RSQ,rn1,rn1,NULL);
			em->Add(OP_MUL,output,rn,rn1);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s.xyz = normalize( %s.xyz )",output,rn);
				AddInst(vp,c);
				if (api==0) _sprintf(c.str,"%s.w = 0.0f",output);
				else _sprintf(c.str,"%s.w = 0.0",output);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_dp3][api],rn1,rn,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s.w",Inst[_rsq][api],rn1,rn1);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],output,rn,rn1);
				AddInst(vp,c);
			}
		}

		_sprintf(tmp,"%s",output);
		return tmp;
	}

	if (((str_match0(chaine,"norme"))||(str_match0(chaine,"norm")))&&(!str_match0(chaine,"normal")))
	{
		if (str_simple(str_return_parentheses(chaine)))
		{
			rn=var(str_return_parentheses(chaine));
		}
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(str_return_parentheses(chaine), tmp2, vp);
		}
		_sprintf(rn1,"r%d",new_temp_register());

		if (api==2)
		{
			em->Add(OP_DP3,output,rn,rn);
			em->Add(OP_RSQ,rn1,rn1,NULL);
			em->Add(OP_RCP,output,rn1,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = length( %s )",output,rn);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_dp3][api],rn1,rn,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s.w",Inst[_rsq][api],rn1,rn1);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s.w",Inst[_rcp][api],output,rn1);
				AddInst(vp,c);
			}
		}
		_sprintf(tmp,"%s",output);
		return tmp;
	}

	if (str_match0(chaine,"normecarree"))
	{
		if (str_simple(str_return_parentheses(chaine)))
		{
			rn=var(str_return_parentheses(chaine));
		}
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			rn = compile(str_return_parentheses(chaine), tmp2, vp);
		}

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			_sprintf(c.str,"%s = %s.x*%s.x + %s.y*%s.y + %s.z*%s.z ",output,rn,rn,rn,rn,rn,rn);
			AddInst(vp,c);
		}

		return output;
	}

	if (str_match0(chaine,"inv"))
	{
		char *str=str_return_parentheses(chaine);
		_sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		
		if (api==2)
		{
			_sprintf(rn1,"r%d",new_temp_register());
			em->Add(OP_DP3,rn1,rn,rn);
			em->Add(OP_RSQ,output,rn1,NULL);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s = 1.0 / length(%s.xyz)",output,rn);
				AddInst(vp,c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str,"%s = 1.0f / length(%s.xyz)",output,rn);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(rn1,"r%d",new_temp_register());
				_sprintf(c.str,"%s %s,%s,%s",Inst[_dp3][api],rn1,rn,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s.w",Inst[_rsq][api],output,rn1);
				AddInst(vp,c);
			}
		}
		_sprintf(tmp,"%s",output);
		return tmp;
	}


	if (str_match0(chaine,"lit"))
	{
		if (str_simple(str_return_parentheses(chaine)))
		{
			rn=var(str_return_parentheses(chaine));
		}
		else
		{
			_sprintf(tmp2, "r%d", new_temp_register());
			resfield(output, tmp2);
			rn = compile(str_return_parentheses(chaine), tmp2, vp);
		}

		if (api==2)
		{
			em->Add(OP_LIT,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = %s;",output,rn);  // TODO
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"%s %s,%s",Inst[_lit][api],output,rn);
				AddInst(vp,c);
			}
		}

		_sprintf(tmp,"%s",output);
		return tmp;
	}

	if (str_match0(chaine, "bx2"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api != 2)
		{
			if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
			{				
				int nb=nb_modifiers(output);

				if (api==0)
				{
					switch (nb)
					{
					case 1:
						_sprintf(tmp4,"0.5f");
						break;
					case 2:
						_sprintf(tmp4,"float2(0.5f,0.5f)");
						break;
					case 3:
						_sprintf(tmp4,"float3(0.5f,0.5f,0.5f)");
						break;
					case 4:
						_sprintf(tmp4,"float4(0.5f,0.5f,0.5f,0.5f)");
						break;
					};
				}
				else
				{
					switch (nb)
					{
					case 1:
						_sprintf(tmp4,"0.5");
						break;
					case 2:
						_sprintf(tmp4,"vec2(0.5,0.5)");
						break;
					case 3:
						_sprintf(tmp4,"vec3(0.5,0.5,0.5)");
						break;
					case 4:
						_sprintf(tmp4,"vec4(0.5,0.5,0.5,0.5)");
						break;
					};
				}

				if (nb>0)
				{
					if (api==1) _sprintf(c.str, "%s = 2.0 * (%s - %s)", output, rn, tmp4);
                    else _sprintf(c.str, "%s = 2.0f * (%s - %s)", output, rn, tmp4);
					AddInst(vp, c);
				}
			}
			else
			if ( api==0 )
			{
				_sprintf(c.str, "%s %s,%s_bx2", Inst[_mov][api], output, rn);
				AddInst(vp, c);
			}
		}

		return output;
	}

	if (str_match0(chaine, "log2"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api == 2)
		{
			em->Add(OP_LOG, output, rn, NULL);
		}
		else
		{
			if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = log2( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str, "%s %s,%s", Inst[_log2][api], output, rn);
				AddInst(vp, c);
			}
		}

		return output;
	}
	else
	if (str_match0(chaine, "log"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = log( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}

		return output;
	}

	if (str_match0(chaine, "exp2"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api == 2)
		{
			em->Add(OP_EXP, output, rn, NULL);
		}
		else
		{
			if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = exp2( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str, "%s %s,%s", Inst[_exp2][api], output, rn);
				AddInst(vp, c);
			}
		}

		return output;
	}
	else
	if (str_match0(chaine, "exp"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = exp( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}

		return output;
	}

	if (str_match0(chaine,"abs"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_ABS,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = abs( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,-%s",Inst[_max][api],output,rn,rn);
				AddInst(vp,c);
			}
		}

		return output;
	}


	if ((str_match0(chaine, "tgt"))||(str_match0(chaine, "tangent")))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		t = 0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t = 1;
			}
			rn = var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn = var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn = compile(&tmp[0], tmp2, vp);
				t = 1;
			}

			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				if (t == 0) _sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);

				rn2 = compile(&tmp[p + 1], tmp3, vp);
				t = 1;
			}
		}

		if (api == 2)
		{
			em->Add(OP_TGT, tmp2, rn, rn2);
		}
		else
		{
			if ((api == 1) && (shadermodel))
			{
				_sprintf(tmp, "r%d", new_temp_register());
				_sprintf(c.str,"%s.xyz = cross( -%s.xyz, %s.xyz )",output,rn,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = cross( vec3(0.0,%s.z,%s.y), %s.xyz )",tmp,rn,rn,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"if ( length(%s.xyz) < length(%s.xyz) )",output,tmp);
				AddInst(vp,c);
				_sprintf(c.str,"{");
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz=%s.xyz",output,tmp);
				AddInst(vp,c);
				_sprintf(c.str,"}");
				AddInst(vp,c);
				_sprintf(c.str,"%s.w = 0.0",output);
				AddInst(vp,c);
			}
			else
			if ((api == 0) && (shadermodel3))
			{
				_sprintf(tmp, "r%d", new_temp_register());
				_sprintf(c.str,"%s.xyz = cross( -%s.xyz, %s.xyz )",output,rn,rn2);
				AddInst(vp,c);
				_sprintf(c.str,"%s.xyz = cross( float3(0.0f,%s.z,%s.y), %s.xyz )",tmp,rn,rn,rn2);
				AddInst(vp,c);

				_sprintf(c.str,"if ( length(%s.xyz) < length(%s.xyz) ) %s.xyz=%s.xyz",output,tmp,output,tmp);
				AddInst(vp,c);
				_sprintf(c.str,"%s.w = 0.0f",output);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str, "%s %s,%s,%s", Inst[_dp3][api], output, rn, rn2);
				AddInst(vp, c);
				_sprintf(c.str, "%s %s,%s,%s,%s", Inst[_mad][api], output, output, rn2, rn);
				AddInst(vp, c);
			}
		}
		return output;
	}

	if (str_match0(chaine, "reflect"))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		t = 0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t = 1;
			}
			rn = var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn = var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn = compile(&tmp[0], tmp2, vp);
				t = 1;
			}

			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				if (t == 0) _sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);

				rn2 = compile(&tmp[p + 1], tmp3, vp);
				t = 1;
			}
		}

		if (((api == 1) && (shadermodel)) || ((api == 0) && (shadermodel3)))
		{
			_sprintf(c.str, "%s = reflect( %s , %s )", output, rn, rn2);
			AddInst(vp, c);
		}
		return output;
	}

	if (str_match0(chaine, "mod"))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		t = 0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t = 1;
			}
			rn = var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn = var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn = compile(&tmp[0], tmp2, vp);
				t = 1;
			}

			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				if (t == 0) _sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);

				rn2 = compile(&tmp[p + 1], tmp3, vp);
				t = 1;
			}
		}

		if ((api == 1) && (shadermodel))
		{
			_sprintf(c.str, "%s = mod( %s , %s )", output, rn, rn2);
			AddInst(vp, c);
		}
		else
		if ((api == 0) && (shadermodel3))
		{
			_sprintf(c.str, "%s = fmod( %s , %s )", output, rn, rn2);
			AddInst(vp, c);
		}

		return output;
	}

	if (str_match0(chaine,"iter"))
	{
		int vv;
		char *str=str_return_parentheses(chaine);
		sscanf(str,"%d",&vv);

		if ((((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))&&(vv>=0))
		{
			if (str_char(output,'.')!=-1)
			{
                if (api==1) _sprintf(c.str,"%s = float(%c%d)",output,ch[vv],vv);
                else _sprintf(c.str,"%s = (float) %c%d",output,ch[vv],vv);
                AddInst(vp,c);

				return output;
			}
			else
			{
				_sprintf(tmp2,"%s.x",output);
                if (api==1) _sprintf(c.str,"%s = float(%c%d)",tmp2,ch[vv],vv);
                else _sprintf(c.str,"%s = (float) %c%d",tmp2,ch[vv],vv);
                AddInst(vp,c);

				return tmp2;
			}
		}
	}

	if (str_match0(chaine,"floor"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;


		if (api==2)
		{
			em->Add(OP_FLOOR,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = floor( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(r0,"r%d",new_temp_register());

				if ((str_char(rn,'.')==-1)&&(str_char(output,'.')==-1))
				{
					_sprintf(c.str,"%s %s.y,%s.x",Inst[_scalar_exp][api],r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,-%s.y,%s.x",Inst[_add][api],output,r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.y",Inst[_scalar_exp][api],r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,-%s.y,%s.y",Inst[_add][api],output,r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.z",Inst[_scalar_exp][api],r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.z,-%s.y,%s.z",Inst[_add][api],output,r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.w",Inst[_scalar_exp][api],r0,rn);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.w,-%s.y,%s.w",Inst[_add][api],output,r0,rn);
					AddInst(vp,c);

				}
				else
				{
					if ((str_char(rn,'.')!=-1)&&(str_char(output,'.')!=-1))
					{
						_sprintf(c.str,"%s %s.y,%s",Inst[_scalar_exp][api],r0,rn);
						AddInst(vp,c);
						_sprintf(c.str,"%s %s,-%s.y,%s.x",Inst[_add][api],output,r0,rn);
						AddInst(vp,c);
					}

					if ((str_char(rn,'.')!=-1)&&(str_char(output,'.')==-1))
					{
						_sprintf(c.str,"%s %s.y,%s",Inst[_scalar_exp][api],r0,rn);
						AddInst(vp,c);
						_sprintf(c.str,"%s %s.x,-%s.y,%s.x",Inst[_add][api],output,r0,rn);
						AddInst(vp,c);
						_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_add][api],output,output);
						AddInst(vp,c);
					}
				}
			}
		}

		return output;
	}

	if (str_match0(chaine,"mirror"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		outputfield1(tmp4, output, rn);

		if ((api==1)&&(shadermodel))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = fract( %s * 0.5 )*2.0", tmp4, rn);
			AddInst(vp, c);

			char * len="1.0";
			if (numberswz(rn)==2) len="vec2(1.0,1.0)";
			if (numberswz(rn)==3) len="vec3(1.0,1.0,1.0)";
			if (numberswz(rn)==4) len="vec4(1.0,1.0,1.0,1.0)";

			_sprintf(c.str, "%s = %s - abs( %s - %s )", tmp4, len, tmp4, len);
			AddInst(vp, c);
		}
		else
		{
			outputfield1(tmp4, output, rn);
			if (metal==1) _sprintf(c.str, "%s = fract( %s * 0.5f ) * 2.0f", tmp4, rn);
			else _sprintf(c.str, "%s = frac( %s * 0.5f ) * 2.0f", tmp4, rn);
			AddInst(vp, c);

			char * len="1.0f";
			if (numberswz(rn)==2) len="float2(1.0f,1.0f)";
			if (numberswz(rn)==3) len="float3(1.0f,1.0f,1.0f)";
			if (numberswz(rn)==4) len="float4(1.0f,1.0f,1.0f,1.0f)";

			_sprintf(c.str, "%s = %s - abs( %s - %s )", tmp4, len, tmp4, len);
			AddInst(vp, c);
				
		}

		return tmp4;
	}

	if (str_match0(chaine,"trunc"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = trunc( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}

		return output;
	}

	if (str_match0(chaine,"round"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = round( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}

		return output;
	}

	if (str_match0(chaine,"degrees"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = degrees( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}
		return output;
	}


	if (str_match0(chaine,"radians"))
	{

		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = radians( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}
		return output;
	}

	if (str_match0(chaine, "rsq"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		_sprintf(rn1, "r%d", new_temp_register());

		if (api == 2)
		{
			em->Add(OP_RSQ, rn1, rn, NULL);
		}
		else
		{
			if ((api == 1) && (shadermodel))
			{				
				outputfield1(tmp4, rn1, rn);
				_sprintf(c.str, "%s = 1.0 / sqrt( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			if ((api == 0) && (shadermodel3))
			{
				outputfield1(tmp4, rn1, rn);
				_sprintf(c.str, "%s = 1.0f / sqrt( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str, "%s %s,%s.w", Inst[_rsq][api], rn1, rn);
				AddInst(vp, c);
			}
		}

		_sprintf(tmp, "%s", rn1);
		return tmp;
	}
	else
	if (str_match0(chaine,"rsqrt"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_FLOOR,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str, "%s = rsqrt( %s )", tmp4, rn);
				AddInst(vp, c);
				return tmp4;
			}
		}

		return output;
	}
	else
	if (str_match0(chaine,"sqrt"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
			outputfield1(tmp4, output, rn);
			_sprintf(c.str, "%s = sqrt( %s )", tmp4, rn);
			AddInst(vp, c);
			return tmp4;
		}

		return output;
	}

	if (str_match0(chaine,"atan2"))
	{
		_sprintf(tmp, "%s", str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p] = '\0';

		t = 0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t = 1;
			}
			rn = var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn = var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);

				rn = compile(&tmp[0], tmp2, vp);
				t = 1;
			}

			if (str_simple(&tmp[p + 1])) rn2 = var(&tmp[p + 1]);
			else
			{
				if (t == 0) _sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);

				rn2 = compile(&tmp[p + 1], tmp3, vp);
				t = 1;
			}
		}

		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s = atan( %s , %s )",output,rn,rn2);
			AddInst(vp,c);
		}
		else
		if ((api==0)&&(shadermodel3))
		{
			_sprintf(c.str,"%s = atan2( %s, %s )",output,rn,rn2);
			AddInst(vp,c);
		}
		return output;
	}
	else
	if (str_match0(chaine,"atan"))
	{
		char *str=str_return_parentheses(chaine);

		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_ATAN,output,rn,NULL);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str,"%s = atan( %s , 1.0 )",tmp4,rn);
				AddInst(vp,c);
				return tmp4;
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str,"%s = atan( %s )",output,rn);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(r0,"r%d",new_temp_register());

				if (str_char(rn,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.x",Inst[_mov][api],r0,rn);
					AddInst(vp,c);

				}
				else
				{
					_sprintf(c.str,"%s %s.x,%s",Inst[_mov][api],r0,rn);
					AddInst(vp,c);
				}

				_sprintf(c.str,"%s %s.y,%s.x,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.z,%s.y,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				// r0.z = t*t*t  r0.y=t*t r0.x=t

				_sprintf(c.str,"%s %s.w,%s.z,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				_sprintf(c.str,"%s %s.y,%s.w,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				// y= t^7 w=t^5 x=t z=t^3

				_sprintf(c.str,"%s %s.x,%s.x,%s.z,%s.x",Inst[_mad][api],r0,var("trigo_cst4"),r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.y,%s.w,%s.x",Inst[_mad][api],r0,var("trigo_cst4"),r0,r0);
				AddInst(vp,c);

				if (str_char(output,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst4"),r0,r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
					AddInst(vp,c);
				}
				else
				{
					_sprintf(c.str,"%s %s,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst4"),r0,r0);
					AddInst(vp,c);
				}
			}
		}
		return output;
	}
	else
	if (str_match0(chaine,"tanh"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api!=2)
		{
            if ((api==1)&&(shadermodel))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = (exp( %s ) - exp(-%s))/(exp( %s ) + exp(-%s))",tmp4,rn,rn,rn,rn);
                AddInst(vp,c);
                return tmp4;
            }
            
            if ((api==0)&&(shadermodel3))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = tanh( %s )",tmp4,rn);
                AddInst(vp,c);
                return tmp4;
            }
		}
		return output;
	}
	else
	if (str_match0(chaine,"tan"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_TAN,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str,"%s = tan( %s )",tmp4,rn);
				AddInst(vp,c);
				return tmp4;
			}
			else
			{
				_sprintf(r0,"r%d",new_temp_register());

				if (str_char(rn,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.x,%s.x",Inst[_add][api],r0,rn,var("trigo_mod"));
					AddInst(vp,c);
				}
				else
				{
					_sprintf(c.str,"%s %s.x,%s,%s.x",Inst[_add][api],r0,rn,var("trigo_mod"));
					AddInst(vp,c);
				}

				_sprintf(c.str,"%s %s.x,%s.x,%s.y",Inst[_mul][api],r0,r0,var("trigo_mod"));
				AddInst(vp,c);

				_sprintf(c.str,"%s %s.y,%s.x",Inst[_scalar_exp][api],r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.y,%s.z,-%s.x",Inst[_mad][api],r0,r0,var("trigo_mod"),var("trigo_mod"));
				AddInst(vp,c);

				// [-PI,PI]


				_sprintf(c.str,"%s %s.y,%s.x,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.z,%s.y,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				// r0.z = t*t*t  r0.y=t*t r0.x=t

				_sprintf(c.str,"%s %s.w,%s.z,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				_sprintf(c.str,"%s %s.y,%s.w,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				// y= t^7 w=t^5 x=t z=t^3

				_sprintf(c.str,"%s %s.x,%s.x,%s.z,%s.x",Inst[_mad][api],r0,var("trigo_cst3"),r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.y,%s.w,%s.x",Inst[_mad][api],r0,var("trigo_cst3"),r0,r0);
				AddInst(vp,c);

				if (str_char(output,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst3"),r0,r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
					AddInst(vp,c);

				}
				else
				{
					_sprintf(c.str,"%s %s,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst3"),r0,r0);
					AddInst(vp,c);
				}
			}
		}
		return output;
	}

	if (str_match0(chaine,"asin"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
        
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_ASIN,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = asin( %s )",tmp4,rn);
                AddInst(vp,c);
				return tmp4;
			}
			else
			{
				_sprintf(r0,"r%d",new_temp_register());

				if (str_char(rn,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.x",Inst[_mov][api],r0,rn);
					AddInst(vp,c);

				}
				else
				{
					_sprintf(c.str,"%s %s.x,%s",Inst[_mov][api],r0,rn);
					AddInst(vp,c);
				}

				_sprintf(c.str,"%s %s.y,%s.x,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.z,%s.y,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				// r0.z = t*t*t  r0.y=t*t r0.x=t

				_sprintf(c.str,"%s %s.w,%s.z,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				_sprintf(c.str,"%s %s.y,%s.w,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				// y= t^7 w=t^5 x=t z=t^3

				_sprintf(c.str,"%s %s.x,%s.x,%s.z,%s.x",Inst[_mad][api],r0,var("trigo_cst2"),r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.y,%s.w,%s.x",Inst[_mad][api],r0,var("trigo_cst2"),r0,r0);
				AddInst(vp,c);

				if (str_char(output,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst2"),r0,r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
					AddInst(vp,c);

				}
				else
				{
					_sprintf(c.str,"%s %s,%s.z,%s.y,%s.x",Inst[_mad][api],output,var("trigo_cst2"),r0,r0);
					AddInst(vp,c);
				}
			}
		}
		return output;
	}
	else
	if (str_match0(chaine,"sinh"))
	{
		char *str=str_return_parentheses(chaine);

		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());

		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api!=2)
		{
            if ((api==1)&&(shadermodel))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = (exp( %s ) - exp(-%s))/2.0",tmp4,rn,rn);
                AddInst(vp,c);
                return tmp4;
            }
            
            if ((api==0)&&(shadermodel3))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = sinh( %s )",tmp4,rn);
                AddInst(vp,c);
                return tmp4;
            }
		}
		return output;
	}
	else
	if (str_match0(chaine,"sin"))
	{
		char *str=str_return_parentheses(chaine);

		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());

		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_SIN,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str,"%s = sin( %s )",tmp4,rn);
				AddInst(vp,c);
				return tmp4;
			}
			else
			{
				if ((shadermodel)&&(pixelshader))
				{
					_sprintf(r0,"r%d",new_temp_register());
					if (str_char(rn,'.')==-1) _sprintf(c.str,"sincos %s.y,%s.x,%s,%s",r0,rn,var("sc1"),var("sc2"));
					else _sprintf(c.str,"sincos %s.y,%s,%s,%s",r0,rn,var("sc1"),var("sc2"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s,%s.y",Inst[_mov][api],output,r0);
					AddInst(vp,c);
				}
				else
				{

					_sprintf(r0,"r%d",new_temp_register());
					_sprintf(r1,"r%d",new_temp_register());

					if (str_char(rn,'.')==-1)
					{
						_sprintf(c.str,"%s %s.x,%s.x,%s.x",Inst[_add][api],r1,rn,var("trigo_mod"));
						AddInst(vp,c);
					}
					else
					{
						_sprintf(c.str,"%s %s.x,%s,%s.x",Inst[_add][api],r1,rn,var("trigo_mod"));
						AddInst(vp,c);
					}


					_sprintf(c.str,"%s %s.x,%s.x,%s.y",Inst[_mul][api],r1,r1,var("trigo_mod"));
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.y,%s.x",Inst[_scalar_exp][api],r0,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.w,%s.y,%s.z,-%s.x",Inst[_mad][api],r1,r0,var("trigo_mod"),var("trigo_mod"));
					AddInst(vp,c);

					// trigo_cst = 0,PI/2,1,PI
					// trigo_cst2 = 1/6,1/120

					_sprintf(c.str,"%s %s.x,%s.w,%s.x",Inst[_sge][api],r1,r1,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.z,-%s.x",Inst[_add][api],r0,var("trigo_cst"),r1);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.x,%s.w,%s.x",Inst[_mul][api],r0,r1,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,-%s.w,%s.y,%s.x",Inst[_mad][api],r0,r1,r0,r0);
					AddInst(vp,c);


					_sprintf(c.str,"%s %s.y,%s.x,%s.y",Inst[_sge][api],r0,r0,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.z,%s.w,-%s.x",Inst[_add][api],r0,var("trigo_cst"),r0);
					AddInst(vp,c);


					_sprintf(c.str,"%s %s.w,%s.z,-%s.y",Inst[_add][api],r0,var("trigo_cst"),r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.x,%s.w",Inst[_mul][api],r1,r0,r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.z,%s.y,%s.y",Inst[_mad][api],r1,r0,r0,r1);
					AddInst(vp,c);


					// signe
					_sprintf(c.str,"%s %s.z,%s.w,%s.x",Inst[_slt][api],r1,r1,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,%s.x,-%s.z",Inst[_add][api],r1,r1,r1);
					AddInst(vp,c);

					// calculs
					_sprintf(c.str,"%s %s.x,%s.y,%s.y",Inst[_mul][api],r0,r1,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.x,%s.y",Inst[_mul][api],r0,r0,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.z,%s.x,%s.y",Inst[_mul][api],r0,r0,r0);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.x,-%s.y,%s.x,%s.y",Inst[_mad][api],r0,r0,var("trigo_cst2"),r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],r0,r0,var("trigo_cst2"),r0);
					AddInst(vp,c);

					if (str_char(output,'.')==-1)
					{
						_sprintf(c.str,"%s %s.x,%s.x,%s.x",Inst[_mul][api],output,r1,r0);
						AddInst(vp,c);
						_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
						AddInst(vp,c);

					}
					else
					{
						_sprintf(c.str,"%s %s,%s.x,%s.x",Inst[_mul][api],output,r1,r0);
						AddInst(vp,c);

					}
				}
			}
		}
		return output;
	}

	if (str_match0(chaine,"acos"))
	{
		char *str=str_return_parentheses(chaine);
		
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());

		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_ACOS,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str,"%s = acos( %s )",tmp4,rn);
				AddInst(vp,c);
				return tmp4;
			}
			else
			{

				_sprintf(r0,"r%d",new_temp_register());

				if (str_char(rn,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.x",Inst[_mov][api],r0,rn);
					AddInst(vp,c);

				}
				else
				{
					_sprintf(c.str,"%s %s.x,%s",Inst[_mov][api],r0,rn);
					AddInst(vp,c);
				}

				_sprintf(c.str,"%s %s.y,%s.x,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.z,%s.y,%s.x",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);
				// r0.z = t*t*t  r0.y=t*t r0.x=t

				_sprintf(c.str,"%s %s.w,%s.z,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				_sprintf(c.str,"%s %s.y,%s.w,%s.y",Inst[_mul][api],r0,r0,r0);
				AddInst(vp,c);

				// y= t^7 w=t^5 x=t z=t^3

				_sprintf(c.str,"%s %s.x,%s.x,%s.z,%s.x",Inst[_mad][api],r0,var("trigo_cst2"),r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.y,%s.w,%s.x",Inst[_mad][api],r0,var("trigo_cst2"),r0,r0);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],r0,var("trigo_cst2"),r0,r0);
				AddInst(vp,c);

				if (str_char(output,'.')==-1)
				{
					_sprintf(c.str,"%s %s.x,%s.y,-%s.x",Inst[_add][api],output,var("trigo_cst"),r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
					AddInst(vp,c);
				}
				else
				{
					_sprintf(c.str,"%s %s,%s.y,-%s.x",Inst[_add][api],output,var("trigo_cst"),r0);
					AddInst(vp,c);
				}
			}
		}
		return output;
	}
	else
	if (str_match0(chaine,"cosh"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api!=2)
		{
            if ((api==1)&&(shadermodel))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = (exp( %s ) + exp(-%s))/2.0",tmp4,rn,rn);
                AddInst(vp,c);
                return tmp4;
            }
            
            if ((api==0)&&(shadermodel3))
            {
                outputfield1(tmp4, output, rn);
                _sprintf(c.str,"%s = cosh( %s )",tmp4,rn);
                AddInst(vp,c);
                return tmp4;
            }
		}
		return output;
	}
	else
	if (str_match0(chaine,"cos"))
	{
		char *str=str_return_parentheses(chaine);
		int sc=str_last_char(output,'.');
		if (sc!=-1)
		{
			switch (strlen(&output[sc+1]))
			{
			case 1:
				_sprintf(tmp2,"r%d.x",new_temp_register());
				break;
			case 2:
				_sprintf(tmp2,"r%d.xy",new_temp_register());
				break;
			case 3:
				_sprintf(tmp2,"r%d.xyz",new_temp_register());
				break;
			case 4:
				_sprintf(tmp2,"r%d",new_temp_register());
				break;
			};
		}
		else _sprintf(tmp2,"r%d",new_temp_register());
		if (str_simple(str)) rn=var(str);
		else rn=compile(str,tmp2,vp);
		if (numberswz(rn)>numberswz(output)) TYPEERROR=true;

		if (api==2)
		{
			em->Add(OP_COS,output,rn,NULL);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield1(tmp4, output, rn);
				_sprintf(c.str,"%s = cos( %s )",tmp4,rn);
				AddInst(vp,c);
				return tmp4;
			}
			else
			{
				if ((shadermodel)&&(pixelshader))
				{
					_sprintf(r0,"r%d",new_temp_register());
					if (str_char(rn,'.')==-1) _sprintf(c.str,"sincos %s.x,%s.x,%s,%s",r0,rn,var("sc1"),var("sc2"));
					else _sprintf(c.str,"sincos %s.x,%s,%s,%s",r0,rn,var("sc1"),var("sc2"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s,%s.x",Inst[_mov][api],output,r0);
					AddInst(vp,c);
				}
				else
				{

					_sprintf(r0,"r%d",new_temp_register());
					_sprintf(r1,"r%d",new_temp_register());

					if (str_char(rn,'.')==-1)
					{
						_sprintf(c.str,"%s %s.x,%s.x,%s.w",Inst[_add][api],r1,rn,var("trigo_mod"));
						AddInst(vp,c);
					}
					else
					{
						_sprintf(c.str,"%s %s.x,%s,%s.w",Inst[_add][api],r1,rn,var("trigo_mod"));
						AddInst(vp,c);
					}


					_sprintf(c.str,"%s %s.x,%s.x,%s.y",Inst[_mul][api],r1,r1,var("trigo_mod"));
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.y,%s.x",Inst[_scalar_exp][api],r0,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.w,%s.y,%s.z,-%s.x",Inst[_mad][api],r1,r0,var("trigo_mod"),var("trigo_mod"));
					AddInst(vp,c);

					// trigo_cst = 0,PI/2,1,PI
					// trigo_cst2 = 1/6,1/120

					_sprintf(c.str,"%s %s.x,%s.w,%s.x",Inst[_sge][api],r1,r1,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.z,-%s.x",Inst[_add][api],r0,var("trigo_cst"),r1);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.x,%s.w,%s.x",Inst[_mul][api],r0,r1,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,-%s.w,%s.y,%s.x",Inst[_mad][api],r0,r1,r0,r0);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.y,%s.x,%s.y",Inst[_sge][api],r0,r0,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.z,%s.w,-%s.x",Inst[_add][api],r0,var("trigo_cst"),r0);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.w,%s.z,-%s.y",Inst[_add][api],r0,var("trigo_cst"),r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.x,%s.w",Inst[_mul][api],r1,r0,r0);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.z,%s.y,%s.y",Inst[_mad][api],r1,r0,r0,r1);
					AddInst(vp,c);

					// signe
					_sprintf(c.str,"%s %s.z,%s.w,%s.x",Inst[_slt][api],r1,r1,var("trigo_cst"));
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,%s.x,-%s.z",Inst[_add][api],r1,r1,r1);
					AddInst(vp,c);

					// calculs
					_sprintf(c.str,"%s %s.x,%s.y,%s.y",Inst[_mul][api],r0,r1,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.y,%s.x,%s.y",Inst[_mul][api],r0,r0,r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.z,%s.x,%s.y",Inst[_mul][api],r0,r0,r0);
					AddInst(vp,c);

					_sprintf(c.str,"%s %s.x,-%s.y,%s.x,%s.y",Inst[_mad][api],r0,r0,var("trigo_cst2"),r1);
					AddInst(vp,c);
					_sprintf(c.str,"%s %s.x,%s.z,%s.y,%s.x",Inst[_mad][api],r0,r0,var("trigo_cst2"),r0);
					AddInst(vp,c);

					if (str_char(output,'.')==-1)
					{
						_sprintf(c.str,"%s %s.x,%s.x,%s.x",Inst[_mul][api],output,r1,r0);
						AddInst(vp,c);
						_sprintf(c.str,"%s %s.yzw,%s.x",Inst[_mov][api],output,output);
						AddInst(vp,c);
					}
					else
					{
						_sprintf(c.str,"%s %s,%s.x,%s.x",Inst[_mul][api],output,r1,r0);
						AddInst(vp,c);
					}
				}
			}
		}
		return output;
	}

	if ((str_match0(chaine,"normal_interpolation"))||(str_match0(chaine,"NormalInterpolation")))
	{
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=5)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                rns[0]=var(rns[0]);
                rns[1]=var(rns[1]);
                rns[2]=var(rns[2]);
                rns[3]=var(rns[3]);
                rns[4]=var(rns[4]);

                if (api==2)
                {
                    em->Add(OP_MUL,tmp2,rns[0],rns[2]);
                    em->Add(OP_MUL,output,rns[1],rns[3]);
                    em->Add(OP_ADD,tmp2,tmp2,output);
                    em->Add(OP_M3X3,output,tmp2,rns[4]);
                }
                else
                {
                    if (api)
                    {
                        str_translate_format(rns[4],s0,&value);
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz",tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            
                            _sprintf(c.str,"%s.x = dot( %s[0].xyz , %s.xyz )",output,rns[4],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.y = dot( %s[1].xyz , %s.xyz )",output,rns[4],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.z = dot( %s[2].xyz , %s.xyz )",output,rns[4],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 0.0",output);
                            AddInst(vp,c);
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_mul][api],tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.x,%s,%s%d]",output,tmp2,s0,value+0);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.y,%s,%s%d]",output,tmp2,s0,value+1);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.z,%s,%s%d]",output,tmp2,s0,value+2);
                            AddInst(vp,c);
                            _sprintf(c.str,"MOV %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz",tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);

                            _sprintf(c.str,"%s.w = 0.0f",tmp2);
                            AddInst(vp,c);

                            if (metal==0)
                            {
                                _sprintf(c.str,"%s = mul( %s , %s )",output,tmp2,rns[4]);
                                AddInst(vp,c);
                            }
                            else
                            {
                                _sprintf(c.str,"%s = %s * %s",output,rns[4],tmp2);
                                AddInst(vp,c);
                            }
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_mul][api],tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"m3x3 %s.xyz,%s,%s",output,tmp2,rns[4]);
                            AddInst(vp,c);
                            _sprintf(c.str,"mov %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
		return output;
	}
	else
	if ((str_match0(chaine,"interpolation"))||(str_match0(chaine,"Interpolation")))
	{
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=5)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                rns[0]=var(rns[0]);
                rns[1]=var(rns[1]);
                rns[2]=var(rns[2]);
                rns[3]=var(rns[3]);
                rns[4]=var(rns[4]);

                if (api==2)
                {
                    em->Add(OP_MUL,tmp2,rns[0],rns[2]);
                    em->Add(OP_MUL,output,rns[1],rns[3]);
                    em->Add(OP_ADD,tmp2,tmp2,output);
                    em->Add(OP_M4X4,output,tmp2,rns[4]);
                }
                else
                {
                    if (api)
                    {
                        str_translate_format(rns[4],s0,&value);
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz",tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0",tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s = %s * %s",output,rns[4],tmp2);
                            AddInst(vp,c);
                        }
                        else
                        {

                            _sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);

                            _sprintf(c.str,"DP4 %s.x,%s,%s%d]",output,tmp2,s0,value+0);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.y,%s,%s%d]",output,tmp2,s0,value+1);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.z,%s,%s%d]",output,tmp2,s0,value+2);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.w,%s,%s%d]",output,tmp2,s0,value+3);
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz",tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0f",tmp2);
                            AddInst(vp,c);
                            if (metal==0)
                            {
                                _sprintf(c.str,"%s = mul( %s ,  %s )",output,tmp2,rns[4]);
                                AddInst(vp,c);
                            }
                            else
                            {
                                _sprintf(c.str,"%s = %s * %s",output,rns[4],tmp2);
                                AddInst(vp,c);
                            }
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s,%s,%s",Inst[_mul][api],tmp2,rns[0],rns[2]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],tmp2,rns[1],rns[3],tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"m4x4 %s,%s,%s",output,tmp2,rns[4]);
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
		return output;
	}

    if ((str_match0(chaine,"normal_interpolate_base"))||(str_match0(chaine,"NormalInterpolateBase")))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=3)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                rns[0]=var(rns[0]);     // n1
                rns[1]=var(rns[1]);     // n2
                rns[2]=var(rns[2]);  // t
                //rns[3]=var(rns[3]);  // WORLD
                
                if (api==2)
                {
                    em->Add(OP_SUB,tmp2,rns[1],rns[0]);
                    em->Add(OP_MUL,output,tmp2,rns[2]);
                    em->Add(OP_ADD,output,rns[0],output);
                }
                else
                {
                    if (api)
                    {
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 0.0",output);
                            AddInst(vp,c);
                        }
                        else
                        {
                            //str_translate_format(rns[3],s0,&value);
                            
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"MOV %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 0.0f",output);
                            AddInst(vp,c);
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s",Inst[_mov][api],tmp2,rns[1]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"mov %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
        return output;
    }
    else
	if ((str_match0(chaine,"normal_interpolate"))||(str_match0(chaine,"NormalInterpolate")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=4)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                
                rns[0]=var(rns[0]);	 // n1
                rns[1]=var(rns[1]);	 // n2
                rns[2]=var(rns[2]);  // t
                rns[3]=var(rns[3]);  // WORLD
                
                if (api==2)
                {
                    em->Add(OP_SUB,tmp2,rns[1],rns[0]);
                    em->Add(OP_MUL,output,tmp2,rns[2]);
                    em->Add(OP_ADD,tmp2,rns[0],output);
                    em->Add(OP_M3X3,output,tmp2,rns[3]);
                }
                else
                {
                    if (api)
                    {
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 0.0",tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s = %s * %s",output,rns[3],tmp2);
                            AddInst(vp,c);
                        }
                        else
                        {
                            str_translate_format(rns[3],s0,&value);

                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.x,%s,%s%d]",output,tmp2,s0,value+0);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.y,%s,%s%d]",output,tmp2,s0,value+1);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP3 %s.z,%s,%s%d]",output,tmp2,s0,value+2);
                            AddInst(vp,c);
                            _sprintf(c.str,"MOV %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 0.0f",tmp2);
                            AddInst(vp,c);
                            if (metal==0)
                            {
                                _sprintf(c.str,"%s = mul( %s , %s)",output,tmp2,rns[3]);
                                AddInst(vp,c);
                            }
                            else
                            {
                                _sprintf(c.str,"%s = %s * %s",output,rns[3],tmp2);
                                AddInst(vp,c);
                            }
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s",Inst[_mov][api],tmp2,rns[1]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s,%s",Inst[_mad][api],tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"m3x3 %s.xyz,%s,%s",output,tmp2,rns[3]);
                            AddInst(vp,c);
                            _sprintf(c.str,"mov %s.w,%s.x",output,var("trigo_cst"));
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
		return output;
	}
    else
    if ((str_match0(chaine,"interpolate_base"))||(str_match0(chaine,"InterpolateBase")))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=3)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                rns[0]=var(rns[0]); //p1
                rns[1]=var(rns[1]); //p2
                rns[2]=var(rns[2]); // interpolant
                //rns[3]=var(rns[3]); // WORLD
                
                if (api==2)
                {
                    em->Add(OP_SUB,tmp2,rns[1],rns[0]);
                    em->Add(OP_MUL,output,tmp2,rns[2]);
                    em->Add(OP_ADD,output,rns[0],output);
                }
                else
                {
                    if (api)
                    {
                        //str_translate_format(rns[3],s0,&value);
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",output,rns[2],tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0",output);
                            AddInst(vp,c);
                        }
                        else
                        {
                            
                            _sprintf(c.str,"%s %s,%s,%s",Inst[_sub][api],tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",output,rns[2],tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0f",output);
                            AddInst(vp,c);
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s",Inst[_mov][api],tmp2,rns[1]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,tmp2,rns[0]);
                            AddInst(vp,c);
                            
                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],output,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
        return output;
    }
	else
	if ((str_match0(chaine,"interpolate"))||(str_match0(chaine,"Interpolate")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=4)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            else
            {
                _sprintf(tmp2,"r%d",new_temp_register());
                rns[0]=var(rns[0]); //p1
                rns[1]=var(rns[1]); //p2
                rns[2]=var(rns[2]); // interpolant
                rns[3]=var(rns[3]); // WORLD

                if (api==2)
                {
                    em->Add(OP_SUB,tmp2,rns[1],rns[0]);
                    em->Add(OP_MUL,output,tmp2,rns[2]);
                    em->Add(OP_ADD,tmp2,rns[0],output);
                    em->Add(OP_M4X4,output,tmp2,rns[3]);
                }
                else
                {
                    if (api)
                    {
                        str_translate_format(rns[3],s0,&value);
                        if ((api==1)&&(shadermodel))
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[2],tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0",tmp2);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s = %s * %s",output,rns[3],tmp2);
                            AddInst(vp,c);
                        }
                        else
                        {

                            _sprintf(c.str,"%s %s,%s,%s",Inst[_sub][api],tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);

                            _sprintf(c.str,"DP4 %s.x,%s,%s%d]",output,tmp2,s0,value+0);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.y,%s,%s%d]",output,tmp2,s0,value+1);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.z,%s,%s%d]",output,tmp2,s0,value+2);
                            AddInst(vp,c);
                            _sprintf(c.str,"DP4 %s.w,%s,%s%d]",output,tmp2,s0,value+3);
                            AddInst(vp,c);
                        }
                    }
                    else
                    {
                        if (shadermodel3)
                        {
                            _sprintf(c.str,"%s.xyz = %s.xyz - %s.xyz",tmp2,rns[1],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.xyz = %s.xyz * %s.xyz + %s.xyz",tmp2,rns[2],tmp2,rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s.w = 1.0f",tmp2);
                            AddInst(vp,c);

                            if (metal==0)
                            {
                                _sprintf(c.str,"%s = mul( %s , %s )",output,tmp2,rns[3]);
                                AddInst(vp,c);
                            }
                            else
                            {
                                _sprintf(c.str,"%s = %s * %s",output,rns[3],tmp2);
                                AddInst(vp,c);
                            }
                        }
                        else
                        {
                            _sprintf(c.str,"%s %s.xyz,%s",Inst[_mov][api],tmp2,rns[1]);
                            AddInst(vp,c);
                            _sprintf(c.str,"%s %s.xyz,%s,%s",Inst[_sub][api],tmp2,tmp2,rns[0]);
                            AddInst(vp,c);

                            _sprintf(c.str,"%s %s,%s,%s,%s",Inst[_mad][api],tmp2,tmp2,rns[2],rns[0]);
                            AddInst(vp,c);
                            _sprintf(c.str,"m4x4 %s,%s,%s",output,tmp2,rns[3]);
                            AddInst(vp,c);
                        }
                    }
                }
            }
        }
		return output;
	}

	if (str_match0(chaine,"getImpostorPosition"))
	{
		rn=var("iTexture1.xy");
		rn2=var("iTexture2.x");

        if ((api==0)&&(shadermodel3))
        {
			_sprintf(c.str,"%s.xy = %s",output,rn);
			AddInst(vp,c);
			_sprintf(c.str,"%s.z = %s",output,rn2);
			AddInst(vp,c);
			_sprintf(c.str,"%s.w = 1.0f",output);
			AddInst(vp,c);
		}
		else
		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s.xy = %s",output,rn);
			AddInst(vp,c);
			_sprintf(c.str,"%s.z = %s",output,rn2);
			AddInst(vp,c);
			_sprintf(c.str,"%s.w = 1.0",output);
			AddInst(vp,c);
		}

		return output;
	}

	if (str_match0(chaine,"getImpostorRadius"))
	{
		rn=var("iTexture2.y");

		int sc = str_last_char(output, '.');

		if (sc>=0) { SYNTAXERROR=true; return ""; }
		else
		{
            if ((api==0)&&(shadermodel3)) _sprintf(c.str,"%s = float4(%s,0.0f,0.0f,0.0f)",output,rn);
            else _sprintf(c.str,"%s = vec4(%s,0.0,0.0,0.0)",output,rn);
			AddInst(vp,c);
			return output;
		}
	}

	if (str_match0(chaine,"clamp"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p1 = str_char_prt(tmp, ',');
        if (p1<0) { SYNTAXERROR=true; return ""; }
		tmp[p1]='\0';

		p2 = str_char_prt(&tmp[p1 + 1], ',') + p1 + 1;
        if (p2<0) { SYNTAXERROR=true; return ""; }
		tmp[p2]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(&tmp[0],tmp2,vp);
		}

		if (str_simple(&tmp[p1+1])) rn2=var(&tmp[p1+1]);
		else
		{
			_sprintf(tmp3,"r%d",new_temp_register());
			resfield(output, tmp3);
			rn2=compile(&tmp[p1+1],tmp3,vp);
		}

		if (str_simple(&tmp[p2+1])) rn3=var(&tmp[p2+1]);
		else
		{
			_sprintf(tmp4,"r%d",new_temp_register());
			resfield(output, tmp4);
			rn3=compile(&tmp[p2+1],tmp4,vp);
		}

		outputfield1(tmp4, output, rn);
		_sprintf(c.str,"%s = clamp( %s , %s , %s )",tmp4,rn,rn2,rn3);
		AddInst(vp,c);

		return tmp4;
	}

    if (str_match0(chaine,"borderize"))
    {
        if ((api==0)&&(shadermodel3))
        {
            _sprintf(c.str,"%s = clamp( %s , 0.0001f, 0.9999f )",output,output);
        }
        else
        {
            _sprintf(c.str,"%s = clamp( %s , 0.0001, 0.9999 )",output,output);
        }
        AddInst(vp,c);

        return output;
    }

	if ((str_match0(chaine,"sat"))||(str_match0(chaine,"saturate")))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}

		outputfield1(tmp4, output, rn);

        if ((api==0)&&(shadermodel3))
        {
			_sprintf(c.str,"%s = saturate( %s )",tmp4,rn);
			AddInst(vp,c);
		}
		else
		if ((api==1)&&(shadermodel))
		{
			_sprintf(c.str,"%s = clamp( %s, 0.0, 1.0 )",tmp4,rn);
			AddInst(vp,c);
		}

		return tmp4;
	}

	////////////////////////////////////////////////////////////////  MISC

	if(str_match0(chaine,"RGBToIntensity"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		if(str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			rn=compile(tmp,tmp2,vp);
		}

		int sc = str_last_char(output,'.');

		if((api==1)&&(shadermodel))
		{
			if (sc==0) _sprintf(c.str,"%s.x = 0.3 * %s.r + 0.6 * %s.g + 0.1 * %s.b",output,rn,rn,rn);
			else _sprintf(c.str,"%s = 0.3 * %s.r + 0.6 * %s.g + 0.1 * %s.b",output,rn,rn,rn);
			AddInst(vp,c);
		}
		else
			if((api==0)&&(shadermodel3))
			{
				if (sc==0) _sprintf(c.str,"%s.x = 0.3f * %s.r + 0.6f * %s.g + 0.1f * %s.b",output,rn,rn,rn);
				else _sprintf(c.str,"%s = 0.3f * %s.r + 0.6f * %s.g + 0.1f * %s.b",output,rn,rn,rn);
				AddInst(vp,c);
			}

		return output;
	}

	////////////////////////////////////////////////////////////////  Signed Value RGBA 16 bits

	if (str_match0(chaine,"SignedValue16ToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}
        
		int sc = str_last_char(rn, '.');

        char sreg[32];
        
        if (sc>=0) _sprintf(sreg, "%s", rn);
        else _sprintf(sreg, "%s.x", rn);

		_sprintf(tmp3,"r%d.x",new_temp_register());

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s =  0.5 * (1.0 + %s)", tmp3, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.x = fract( 255.0 * %s )", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = floor( 255.0 * %s) / 255.0", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			if (metal==1)
			{
	            _sprintf(c.str, "%s =  0.5f * (1.0f + %s)", tmp3, sreg);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.x = fract( 255.0f * %s )", output, tmp3);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = floor( 255.0f * %s) / 255.0f", output, tmp3);
				AddInst(vp, c);
				_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
				AddInst(vp, c);
			}
			else
			{
	            _sprintf(c.str, "%s =  0.5f * (1.0f + %s)", tmp3, sreg);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.x = frac( 255.0f * %s )", output, tmp3);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = floor( 255.0f * %s) / 255.0f", output, tmp3);
				AddInst(vp, c);
				_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
				AddInst(vp, c);
			}
        }
	
        return output;
	}

    if (str_match0(chaine,"RGBToSignedValue16"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", output, rn, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s.x = (255.0f * %s.g + %s.r)/255.0f", output, rn, rn);
				AddInst(vp, c);
		        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
	            AddInst(vp, c);
				_sprintf(c.str, "%s = %s.xxxx", output, output);
				AddInst(vp, c);
            }
        
        return output;
    }


    if (str_match0(chaine,"RGBToTwoValue16"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", output, rn, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = (255.0 * %s.a + %s.b)/255.0", output, rn, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = 2.0 * %s.y - 1.0", output, output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s.x = (255.0f * %s.g + %s.r)/255.0f", output, rn, rn);
				AddInst(vp, c);
				_sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = (255.0f * %s.a + %s.b)/255.0f", output, rn, rn);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = 2.0f * %s.y - 1.0f", output, output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
				AddInst(vp, c);
			}
        
        return output;
    }

	////////////////////////////////////////////////////////////////  Value RGBA 16 bits

	if (str_match0(chaine,"TwoValue16ToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				resfield(output, tmp3);
				rn2=compile(&tmp[p+1],tmp3,vp);
				t=1;
			}
		}

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = 0.5 + 0.5 * (%s)", output,rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s.w = 0.5 + 0.5 * (%s)", output,rn2);
            AddInst(vp, c);

            _sprintf(c.str, "%s.y = floor( 255.0 * %s.x ) / 255.0", output,output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.x = fract( 255.0 * %s.x )", output,output);
            AddInst(vp, c);

			
			_sprintf(c.str, "%s.z = fract( 255.0 * %s.w )", output,output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.w = floor( 255.0 * %s.w ) / 255.0", output,output);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			if (metal==1)
			{
				_sprintf(c.str, "%s.x = 0.5f + 0.5f * (%s)", output,rn);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.5f + 0.5f * (%s)", output,rn2);
				AddInst(vp, c);

				_sprintf(c.str, "%s.y = floor( 255.0f * %s.x ) / 255.0f", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.x = fract( 255.0f * %s.x )", output,output);
				AddInst(vp, c);

			
				_sprintf(c.str, "%s.z = fract( 255.0f * %s.w )", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = floor( 255.0f * %s.w ) / 255.0f", output,output);
				AddInst(vp, c);
			}
			else
			{
				_sprintf(c.str, "%s.x = 0.5f + 0.5f * (%s)", output,rn);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.5f + 0.5f * (%s)", output,rn2);
				AddInst(vp, c);

				_sprintf(c.str, "%s.y = floor( 255.0f * %s.x ) / 255.0f", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.x = frac( 255.0f * %s.x )", output,output);
				AddInst(vp, c);

			
				_sprintf(c.str, "%s.z = frac( 255.0f * %s.w )", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = floor( 255.0f * %s.w ) / 255.0f", output,output);
				AddInst(vp, c);
			}
        }
	
        return output;
	}
	else
	if (str_match0(chaine,"halfValue16ToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				resfield(output, tmp3);
				rn2=compile(&tmp[p+1],tmp3,vp);
				t=1;
			}
		}

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = 0.5 + 0.5 * (%s)", output,rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s.w = 0.5 + 0.5 * (%s)", output,rn2);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = fract( 255.0 * %s.w )", output,output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.z = floor( 255.0 * %s.w ) / 255.0", output,output);
            AddInst(vp, c);
            _sprintf(c.str, "%s.w = 1.0", output);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			if (metal==1)
			{
	            _sprintf(c.str, "%s.x = 0.5f + 0.5f * (%s)", output,rn);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.5f + 0.5f * (%s)", output,rn2);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.y = fract( 255.0f * %s.w )", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.z = floor( 255.0f * %s.w ) / 255.0f", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 1.0f", output);
				AddInst(vp, c);
			}
			else
			{
	            _sprintf(c.str, "%s.x = 0.5f + 0.5f * (%s)", output,rn);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.5f + 0.5f * (%s)", output,rn2);
		        AddInst(vp, c);
				_sprintf(c.str, "%s.y = frac( 255.0f * %s.w )", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.z = floor( 255.0f * %s.w ) / 255.0f", output,output);
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 1.0f", output);
				AddInst(vp, c);
			}
        }
	
        return output;
	}
	else
	if (str_match00(chaine,"Value16ToRGB."))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}
        
		int sc = str_last_char(rn, '.');

        char sreg[32];
        
        if (sc>=0) _sprintf(sreg, "%s", rn);
        else _sprintf(sreg, "%s.x", rn);

		char ext[1024];
		_sprintf(ext, &chaine[str_char(chaine, '.')]);
		ext[str_char(ext, '(')] = '\0';

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.%c = fract( 255.0 * %s )", output,ext[1], sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.%c = floor( 255.0 * %s) / 255.0", output,ext[2], sreg);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			if (metal==1)
			{
				_sprintf(c.str, "%s.%c = fract( 255.0f * %s )", output,ext[1], sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.%c = floor( 255.0f * %s) / 255.0f", output,ext[2], sreg);
				AddInst(vp, c);
			}
			else
			{
				_sprintf(c.str, "%s.%c = frac( 255.0f * %s )", output,ext[1], sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.%c = floor( 255.0f * %s) / 255.0f", output,ext[2], sreg);
				AddInst(vp, c);
			}
        }
	
        return output;
	}
	else
	if (str_match0(chaine,"Value16ToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}
        
		int sc = str_last_char(rn, '.');

        char sreg[32];
        
        if (sc>=0) _sprintf(sreg, "%s", rn);
        else _sprintf(sreg, "%s.x", rn);

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = fract( 255.0 * %s )", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = floor( 255.0 * %s) / 255.0", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			if (metal==1)
			{
				_sprintf(c.str, "%s.x = fract( 255.0f * %s )", output, sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = floor( 255.0f * %s) / 255.0f", output, sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
				AddInst(vp, c);
			}
			else
			{
				_sprintf(c.str, "%s.x = frac( 255.0f * %s )", output, sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.y = floor( 255.0f * %s) / 255.0f", output, sreg);
				AddInst(vp, c);
				_sprintf(c.str, "%s.zw = float2(0.0f,0.0f)", output);
				AddInst(vp, c);
			}
        }
	
        return output;
	}

    if (str_match00(chaine,"isOpenGL"))
    {
		int scout = str_last_char(output, '.');
		if (scout==-1) { SYNTAXERROR=true; return ""; }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s = 1.0", output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s = 0.0f", output);
				AddInst(vp, c);
            }
        
        return output;
    }

    if (str_match00(chaine,"ZValueToZDepth"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }

		int sc = str_last_char(rn, '.');
		int scout = str_last_char(output, '.');

		if (sc==-1) { SYNTAXERROR=true; return ""; }
		if (scout==-1) { SYNTAXERROR=true; return ""; }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s = (1.0 + %s)*0.5", output, rn);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s = %s", output, rn);
				AddInst(vp, c);
            }
        
        return output;
    }

    if (str_match00(chaine,"ZDepthToZValue"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }

		int sc = str_last_char(rn, '.');
		int scout = str_last_char(output, '.');

		if (sc==-1) { SYNTAXERROR=true; return ""; }
		if (scout==-1) { SYNTAXERROR=true; return ""; }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s = 2.0 * %s - 1.0", output, rn);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s = %s", output, rn);
				AddInst(vp, c);
            }
        
        return output;
    }

    if (str_match00(chaine,"RGBToValue16."))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }

		char ext[1024];
		_sprintf(ext, &chaine[str_char(chaine, '.')]);
		ext[str_char(ext, '(')] = '\0';
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = (255.0 * %s.%c + %s.%c)/255.0", output, rn,ext[2], rn,ext[1]);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s.x = (255.0f * %s.%c + %s.%c)/255.0f", output, rn,ext[2], rn,ext[1]);
				AddInst(vp, c);
				_sprintf(c.str, "%s = %s.xxxx", output, output);
				AddInst(vp, c);
            }
        
        return output;
    }
	else
    if (str_match0(chaine,"RGBToValue16"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", output, rn, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
				_sprintf(c.str, "%s.x = (255.0f * %s.g + %s.r)/255.0f", output, rn, rn);
				AddInst(vp, c);
				_sprintf(c.str, "%s = %s.xxxx", output, output);
				AddInst(vp, c);
            }
        
        return output;
    }

////////////////////////////////////////////////////////////////  Signed Value RGBA

	if (str_match0(chaine,"SignedValueToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}
        
		int sc = str_last_char(rn, '.');

        char sreg[32];
        
        if (sc>=0) _sprintf(sreg, "%s", rn);
        else _sprintf(sreg, "%s.x", rn);


		_sprintf(tmp3,"r%d.x",new_temp_register());

        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s =  0.5 * (1.0 + %s)", tmp3, sreg);
            AddInst(vp, c);

#ifdef IOS
            _sprintf(c.str, "%s.x = fract( 255.0 * %s )", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = floor( 255.0 * %s) / 255.0", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
#else
            _sprintf(c.str, "%s = fract(%s * vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0))", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.yzw -= %s.xyz * vec3(1.0/256.0, 1.0/256.0, 1.0/256.0)", output, output);
            AddInst(vp, c);
#endif
        }
        else
        if ((api==0)&&(shadermodel3))
        {
            _sprintf(c.str, "%s =  0.5f * (1.0f + %s)", tmp3, sreg);
            AddInst(vp, c);

            if (metal==1) _sprintf(c.str, "%s = fract(%s * float4(256.0f*256.0f*256.0f, 256.0f*256.0f, 256.0f, 1.0f))", output, tmp3);
            else _sprintf(c.str, "%s = frac(%s * float4(256.0f*256.0f*256.0f, 256.0f*256.0f, 256.0f, 1.0f))", output, tmp3);
            AddInst(vp, c);
            _sprintf(c.str, "%s.yzw -= %s.xyz * float3(1.0f/256.0f, 1.0f/256.0f, 1.0f/256.0f)", output, output);
            AddInst(vp, c);
        }
	
        return output;
	}

    if (str_match0(chaine,"RGBToSignedValue"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
#ifdef IOS
            _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", output, rn, rn);
            AddInst(vp, c);
#else
            _sprintf(c.str, "%s.x = dot(%s , vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0))", output, rn);
            AddInst(vp, c);
#endif
            _sprintf(c.str, "%s.x = 2.0 * %s.x - 1.0", output, output);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
        }
        else
            if ((api==0)&&(shadermodel3))
            {
                _sprintf(c.str, "%s.x = dot(%s , float4(1.0f/(256.0f*256.0f*256.0f), 1.0f/(256.0f*256.0f), 1.0f/256.0f, 1.0f))", output, rn);
                AddInst(vp, c);
		        _sprintf(c.str, "%s.x = 2.0f * %s.x - 1.0f", output, output);
	            AddInst(vp, c);
                _sprintf(c.str, "%s = %s.xxxx", output, output);
                AddInst(vp, c);
            }
        
        return output;
    }


	////////////////////////////////////////////////////////////////  Value RGBA

	if (str_match0(chaine,"ValueToRGB"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		
		if (str_simple(tmp)) rn=var(tmp);
		else
		{
			_sprintf(tmp2,"r%d",new_temp_register());
			resfield(output, tmp2);
			rn=compile(tmp,tmp2,vp);
		}
        
		int sc = str_last_char(rn, '.');

        char sreg[32];
        
        if (sc>=0) _sprintf(sreg, "%s", rn);
        else _sprintf(sreg, "%s.x", rn);

        if ((api==1)&&(shadermodel))
        {
#ifdef IOS
            _sprintf(c.str, "%s.x = fract( 255.0 * %s )", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.y = floor( 255.0 * %s) / 255.0", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.zw = vec2(0.0,0.0)", output);
            AddInst(vp, c);
#else
            _sprintf(c.str, "%s = fract(%s * vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0))", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.yzw -= %s.xyz * vec3(1.0/256.0, 1.0/256.0, 1.0/256.0)", output, output);
            AddInst(vp, c);
#endif
        }
        else
        if ((api==0)&&(shadermodel3))
        {
            if (metal==1) _sprintf(c.str, "%s = fract(%s * float4(256.0f*256.0f*256.0f, 256.0f*256.0f, 256.0f, 1.0f))", output, sreg);
            else _sprintf(c.str, "%s = frac(%s * float4(256.0f*256.0f*256.0f, 256.0f*256.0f, 256.0f, 1.0f))", output, sreg);
            AddInst(vp, c);
            _sprintf(c.str, "%s.yzw -= %s.xyz * float3(1.0f/256.0f, 1.0f/256.0f, 1.0f/256.0f)", output, output);
            AddInst(vp, c);
        }
	
        return output;
	}

    if (str_match0(chaine,"RGBToValue"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
#ifdef IOS
            _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", output, rn, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
#else
            _sprintf(c.str, "%s.x = dot(%s , vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0))", output, rn);
            AddInst(vp, c);
            _sprintf(c.str, "%s = %s.xxxx", output, output);
            AddInst(vp, c);
#endif
        }
        else
            if ((api==0)&&(shadermodel3))
            {
                _sprintf(c.str, "%s.x = dot(%s , float4(1.0f/(256.0f*256.0f*256.0f), 1.0f/(256.0f*256.0f), 1.0f/256.0f, 1.0f))", output, rn);
                AddInst(vp, c);
                _sprintf(c.str, "%s = %s.xxxx", output, output);
                AddInst(vp, c);
            }
        
        return output;
    }


	////////////////////////////////////////////////////////////////

    if (str_match0(chaine,"ConvertClipZ"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        if ((api==1)&&(shadermodel))
        {
            _sprintf(c.str, "%s = 0.5 - (0.5 * %s)", output, rn);
            AddInst(vp, c);
        }
        else
        if ((api==0)&&(shadermodel3))
        {
            _sprintf(c.str, "%s = %s", output, rn);
            AddInst(vp, c);
        }
        
        return output;
    }
    

    if (str_match0(chaine,"QuarterInterpolateSin"))
    {
	    //val.y=QuarterInterpolateSin(val.x,val.z,koef,raw0,raw1,raw2,raw3);
		
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=7)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            
            rn=var(rns[0]); //x
            rn2=var(rns[1]); //y
			rn3=var(rns[2]); //y
            rns[0]=var(rns[3]); //rawy0
            rns[1]=var(rns[4]); //rawy1
            rns[2]=var(rns[5]); //rawy2
            rns[3]=var(rns[6]); //rawy3
            
            _sprintf(tmp2,"r%d",new_temp_register());

            int sc = str_last_char(output, '.');
            if (sc<0)
            {
            }
            else
            {
                if ((api==1)&&(shadermodel))
                {
                    _sprintf(c.str, "if (%s < 0.333)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s)/0.333",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    


                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.333)/0.333",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    


                    _sprintf(c.str, "if (%s >= 0.666)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.666)/0.334",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
                else
                if ((api==0)&&(shadermodel3))
                {
                    _sprintf(c.str, "if (%s < 0.333f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s)/0.333f",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);



                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.333f)/0.333f",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);



                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.666f)/0.334f",tmp2,rn2); AddInst(vp, c);
					_sprintf(c.str, "%s.y = %s.y - %s*sin(%s.y * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
					_sprintf(c.str, "%s.x = %s.x - %s*sin(%s.x * 6.2831853f)",tmp2,tmp2,rn3,tmp2); AddInst(vp, c);

                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
            }
        }
        return output;
    }
	else
    if (str_match0(chaine,"QuarterInterpolate"))
    {
	    //val.y=QuarterInterpolate(val.x,val.z,raw0,raw1,raw2,raw3);
		
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=6)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
            
            rn=var(rns[0]); //x
            rn2=var(rns[1]); //y
            rns[0]=var(rns[2]); //rawy0
            rns[1]=var(rns[3]); //rawy1
            rns[2]=var(rns[4]); //rawy2
            rns[3]=var(rns[5]); //rawy3
            
            _sprintf(tmp2,"r%d",new_temp_register());

            int sc = str_last_char(output, '.');
            if (sc<0)
            {
            }
            else
            {
                if ((api==1)&&(shadermodel))
                {
                    _sprintf(c.str, "if (%s < 0.333)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s)/0.333",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.333)/0.333",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.666)/0.334",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333)&&(%s < 0.666))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333)/0.333",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666)/0.334",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
                else
                if ((api==0)&&(shadermodel3))
                {
                    _sprintf(c.str, "if (%s < 0.333f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s)/0.333f",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[0],tmp2,rns[1],rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.333f)/0.333f",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[1],tmp2,rns[2],rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.y = (%s-0.666f)/0.334f",tmp2,rn2); AddInst(vp, c);
                    
                    _sprintf(c.str, "if (%s < 0.333f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.x + %s.y*(%s.x - %s.x)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.333f)&&(%s < 0.666f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.333f)/0.333f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.y + %s.y*(%s.y - %s.y)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.666f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s.x = (%s-0.666f)/0.334f",tmp2,rn); AddInst(vp, c);
                    _sprintf(c.str, "%s.z = %s.z + %s.y*(%s.z - %s.z)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s.w = %s.w + %s.y*(%s.w - %s.w)",tmp2,rns[2],tmp2,rns[3],rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "%s = %s.z + %s.x*(%s.w - %s.z)",output,tmp2,tmp2,tmp2,tmp2); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
            }
        }
        return output;
    }
    
    if (str_match0(chaine,"Quarter"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)!=6)
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }

            rn=var(rns[0]); //x
            rn2=var(rns[1]); //y
            rns[0]=var(rns[2]); //rawy0
            rns[1]=var(rns[3]); //rawy1
            rns[2]=var(rns[4]); //rawy2
            rns[3]=var(rns[5]); //rawy3
            
            int sc = str_last_char(output, '.');
            if (sc<0)
            {
            }
            else
            {
                if ((api==1)&&(shadermodel))
                {
                    _sprintf(c.str, "if (%s < 0.25)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25)&&(%s < 0.5))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5)&&(%s < 0.75))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.25)&&(%s < 0.5))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25)&&(%s < 0.5))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5)&&(%s < 0.75))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.5)&&(%s < 0.75))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25)&&(%s < 0.5))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5)&&(%s < 0.75))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.75)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25)&&(%s < 0.5))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5)&&(%s < 0.75))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
                else
                if ((api==0)&&(shadermodel3))
                {
                    _sprintf(c.str, "if (%s < 0.25f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25f)&&(%s < 0.5f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5f)&&(%s < 0.75f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[0]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.25f)&&(%s < 0.5f))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25f)&&(%s < 0.5f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5f)&&(%s < 0.75f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[1]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if ((%s >= 0.5f)&&(%s < 0.75f))",rn2,rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25f)&&(%s < 0.5f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5f)&&(%s < 0.75f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[2]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    
                    _sprintf(c.str, "if (%s >= 0.75f)",rn2); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s < 0.25f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.x",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.25f)&&(%s < 0.5f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.y",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if ((%s >= 0.5f)&&(%s < 0.75f))",rn,rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.z",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "if (%s >= 0.75f)",rn); AddInst2(vp, c);
                    _sprintf(c.str, "{"); AddInst2(vp, c);
                    _sprintf(c.str, "%s = %s.w",output,rns[3]); AddInst(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                    _sprintf(c.str, "}"); AddInst2(vp, c);
                }
            }
        }
        return output;
    }

    
    if (str_match0(chaine,"RelativeToNormal"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==2)
            {
                rn=var(rns[0]); // vect
                rn2=var(rns[1]); // normal
                
                if ((api == 1) && (shadermodel))  // OPEN GL
                {
                    _sprintf(tmp, "r%d", new_temp_register());
                    _sprintf(tmp2, "r%d", new_temp_register());

                    _sprintf(c.str,"%s.xyz = vec3( %s.z, 0.0, -%s.x)",tmp2,rn2,rn2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xyz = vec3( %s.y, -%s.x, 0.0)",tmp,rn2,rn2);
                    AddInst(vp,c);

//                    _sprintf(c.str,"%s.xyz = cross( vec3(0.0,1.0,0.0), %s.xyz )",tmp2,rn2);
//                    AddInst(vp,c);
//                    _sprintf(c.str,"%s.xyz = cross( vec3(0.0,0.0,-1.0), %s.xyz )",tmp,rn2);
//                    AddInst(vp,c);
                    _sprintf(c.str,"if ( length(%s.xyz) < length(%s.xyz) )",tmp2,tmp);
                    AddInst(vp,c);
                    _sprintf(c.str,"{");
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xyz=%s.xyz",tmp2,tmp);
                    AddInst(vp,c);
                    _sprintf(c.str,"}");
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0",tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = normalize( %s )",tmp2,tmp2);
                    AddInst(vp,c);
                    
                    // tmp2=T rn2=N
                    
                    _sprintf(c.str,"%s.x = dot(%s,%s)",output,rn,tmp2);
                    AddInst(vp,c);
                    
                    _sprintf(c.str,"%s.xyz = cross( %s.xyz , %s.xyz )",tmp,rn2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0",tmp);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.y = dot(%s,%s)",output,rn,tmp);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.z = dot(%s,%s)",output,rn,rn2);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.w = 0.0",output);
                    AddInst(vp,c);

                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
                    _sprintf(tmp, "r%d", new_temp_register());
                    _sprintf(tmp2, "r%d", new_temp_register());
                    
                    _sprintf(c.str,"%s.xyz = float3( %s.z, 0.0f, -%s.x)",tmp2,rn2,rn2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xyz = float3( %s.y, -%s.x, 0.0f)",tmp,rn2,rn2);
                    AddInst(vp,c);

//                    _sprintf(c.str,"%s.xyz = cross( float3(0.0f,1.0f,0.0f), %s.xyz )",tmp2,rn2);
//                    AddInst(vp,c);
//                    _sprintf(c.str,"%s.xyz = cross( float3(0.0f,0.0f,-1.0f), %s.xyz )",tmp,rn2);
//                    AddInst(vp,c);

                    _sprintf(c.str,"if ( length(%s.xyz) < length(%s.xyz) ) %s.xyz=%s.xyz",tmp2,tmp,tmp2,tmp);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0f",tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = normalize( %s )",tmp2,tmp2);
                    AddInst(vp,c);

                    // tmp2=T rn2=N
                    
                    _sprintf(c.str,"%s.x = dot(%s,%s)",output,rn,tmp2);
                    AddInst(vp,c);
                    
                    _sprintf(c.str,"%s.xyz = cross( %s.xyz , %s.xyz )",tmp,rn2,tmp2);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0f",tmp);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.y = dot(%s,%s)",output,rn,tmp);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.z = dot(%s,%s)",output,rn,rn2);
                    AddInst(vp,c);

                    _sprintf(c.str,"%s.w = 0.0f",output);
                    AddInst(vp,c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }

    if (str_match0(chaine,"NormalReflection"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==3)
            {
                rn=var(rns[0]); // pos
                rn2=var(rns[1]); // light
                rn3=var(rns[2]); // N
                
                //ur = Light - pos;
                //ur = normalise(ur);
                //ur = N+ N  - ur;
                //ur = normalise(ur);

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
                    _sprintf(c.str,"%s = normalize( %s - %s )",output,rn2,rn);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = normalize( 2.0 * %s - %s )",output,rn3,output);
                    AddInst(vp,c);
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
                    _sprintf(c.str,"%s = normalize( %s - %s )",output,rn2,rn);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = normalize( 2.0f * %s - %s )",output,rn3,output);
                    AddInst(vp,c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }


    if (str_match0(chaine,"ProcessShadowCoord"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==1)
            {
                rn=var(rns[0]); // posi
                
                _sprintf(tmp, "r%d", new_temp_register());
                
                //div.w = reciprocal( posi );
                //posi.xy = posi.xy * div.ww;
                //posi.xy = ScaleToSecondary(posi.xy);

                // tmp = div

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
                    _sprintf(c.str,"%s.w = 1.0 / %s.w",tmp,rn);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xy = %s.xy * %s.ww",output,rn,tmp);
                    AddInst(vp,c);
                    _sprintf(c.str, "%s.xy = vec2(0.5,0.5) + vec2(0.5,0.5) * %s.xy", output, output);
                    AddInst(vp, c);
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
                    _sprintf(c.str,"%s.w = 1.0f / %s.w",tmp,rn);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xy = %s.xy * %s.ww",output,rn,tmp);
                    AddInst(vp,c);
                    _sprintf(c.str, "%s.xy = float2(0.5f,0.5f) + float2(0.5f,-0.5f) * %s.xy", output, output);
                    AddInst(vp, c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }

    if (str_match0(chaine,"EyePositionRelative"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==3)
            {
                rn=var(rns[0]); // Eye
                rn2=var(rns[1]); // Pos
				rn3=var(rns[2]); // N
                
				_sprintf(tmp, "r%d", new_temp_register());
				_sprintf(tmp2, "r%d", new_temp_register());
				_sprintf(tmp3, "r%d", new_temp_register());

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
					//	up=0.0,-1.0,0.0,0.0;
					//	up2=0.0,0.0,-1.0,0.0;

                    _sprintf(c.str,"%s = vec4(-%s.z,0.0,%s.x,0.0)",tmp,rn3,rn3);		// r6 = ( up ^ tN )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = vec4(%s.y,-%s.x,0.0,0.0)",tmp2,rn3,rn3);	// r7 = ( up2 ^ tN )
                    AddInst(vp,c);

					_sprintf(c.str,"%s.xy = vec2(-%s.y,%s.y)",tmp3,rn3,rn3);			// r9.x = dot ( tN, up)  and r9.y = -r9.x
                    AddInst(vp,c);

					_sprintf(c.str,"if ( %s.x < 0.0 ) %s.x = %s.y;", tmp3,tmp3,tmp3);
					AddInst(vp, c);
/*
					_sprintf(c.str,"%s.z = ( %s.x < 0.0 ) ? 1.0 : 0.0", tmp3, tmp3);		// r9.z = slt(r9.x,0.0)
					AddInst(vp, c);
					_sprintf(c.str,"%s.w = ( %s.x >= 0.0 ) ? 1.0 : 0.0", tmp3, tmp3);	// r9.w = sge(r9.x,0.0)
					AddInst(vp, c);
                    _sprintf(c.str,"%s.x = %s.x * %s.w + %s.y * %s.z",tmp3,tmp3,tmp3,tmp3,tmp3);		// r9.x = r9.x*r9.w + r9.y*r9.z
                    AddInst(vp,c);
/**/
                    _sprintf(c.str,"%s.x = 1.0 - %s.x",tmp3,tmp3);	// r9.x = 1.0 - r9.x
                    AddInst(vp,c);

					_sprintf(c.str,"if ( %s.x < 0.025 ) %s = %s;", tmp3,tmp,tmp2);
					AddInst(vp, c);
/*
					_sprintf(c.str,"%s.z = ( %s.x < 0.025 ) ? 1.0 : 0.0", tmp3, tmp3);		// r9.z = slt(r9.x,0.025)
					AddInst(vp, c);
					_sprintf(c.str,"%s.w = ( %s.x >= 0.025 ) ? 1.0 : 0.0", tmp3, tmp3);		// r9.w = sge(r9.x,0.025)
					AddInst(vp, c);
                    _sprintf(c.str,"%s = %s * %s.wwww + %s * %s.zzzz",tmp,tmp,tmp3,tmp2,tmp3);	// r6 = r6 * r9.wwww + r7 * r9.zzzz
                    AddInst(vp,c);
/**/
					_sprintf(c.str,"%s.xyz = cross( %s.xyz , %s.xyz )",tmp2,rn3,tmp);		// r7 = tN ^ r6
					AddInst(vp,c);

					_sprintf(c.str,"%s = normalize( %s - %s )",tmp3,rn,rn2);			// r4 = normalise(Eye - tPos)
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = dot(%s.xyz , %s.xyz)",output,tmp3,rn3);	// u.x = ( r4 | tN )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = dot(%s.xyz , %s.xyz)",output,tmp3,tmp);	// u.y = ( r4 | r6 )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = dot(%s.xyz , %s.xyz)",output,tmp3,tmp2);	// u.z = ( r4 | r7 )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0",output);
                    AddInst(vp,c);
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
					//	up=0.0,-1.0,0.0,0.0;
					//	up2=0.0,0.0,-1.0,0.0;

                    _sprintf(c.str,"%s = float4(-%s.z,0.0f,%s.x,0.0f)",tmp,rn3,rn3);		// r6 = ( up ^ tN )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s = float4(%s.y,-%s.x,0.0f,0.0f)",tmp2,rn3,rn3);	// r7 = ( up2 ^ tN )
                    AddInst(vp,c);

					_sprintf(c.str,"%s.xy = float2(-%s.y,%s.y)",tmp3,rn3,rn3);			// r9.x = dot ( tN, up)  and r9.y = -r9.x
                    AddInst(vp,c);

					_sprintf(c.str,"if ( %s.x < 0.0f ) %s.x = %s.y", tmp3,tmp3,tmp3);
					AddInst(vp, c);

/*
					_sprintf(c.str,"%s.z = ( %s.x < 0.0f ) ? 1.0 : 0.0f", tmp3, tmp3);		// r9.z = slt(r9.x,0.0)
					AddInst(vp, c);
					_sprintf(c.str,"%s.w = ( %s.x >= 0.0f ) ? 1.0f : 0.0f", tmp3, tmp3);	// r9.w = sge(r9.x,0.0)
					AddInst(vp, c);
                    _sprintf(c.str,"%s.x = %s.x * %s.w + %s.y * %s.z",tmp3,tmp3,tmp3,tmp3,tmp3);		// r9.x = r9.x*r9.w + r9.y*r9.z
                    AddInst(vp,c);
/**/
                    _sprintf(c.str,"%s.x = 1.0f - %s.x",tmp3,tmp3);	// r9.x = 1.0 - r9.x
                    AddInst(vp,c);

					_sprintf(c.str,"if ( %s.x < 0.025f ) %s = %s", tmp3,tmp,tmp2);
					AddInst(vp, c);
/*
					_sprintf(c.str,"%s.z = ( %s.x < 0.025f ) ? 1.0f : 0.0f", tmp3, tmp3);		// r9.z = slt(r9.x,0.025)
					AddInst(vp, c);
					_sprintf(c.str,"%s.w = ( %s.x >= 0.025f ) ? 1.0f : 0.0f", tmp3, tmp3);		// r9.w = sge(r9.x,0.025)
					AddInst(vp, c);
                    _sprintf(c.str,"%s = %s * %s.wwww + %s * %s.zzzz",tmp,tmp,tmp3,tmp2,tmp3);	// r6 = r6 * r9.wwww + r7 * r9.zzzz
                    AddInst(vp,c);
/**/
					_sprintf(c.str,"%s.xyz = cross( %s.xyz , %s.xyz )",tmp2,rn3,tmp);		// r7 = tN ^ r6
					AddInst(vp,c);

					_sprintf(c.str,"%s = normalize( %s - %s )",tmp3,rn,rn2);			// r4 = normalise(Eye - tPos)
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.z = dot(%s.xyz , %s.xyz)",output,tmp3,rn3);	// u.x = ( r4 | tN )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x = dot(%s.xyz , %s.xyz)",output,tmp3,tmp);	// u.y = ( r4 | r6 )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y = dot(%s.xyz , %s.xyz)",output,tmp3,tmp2);	// u.z = ( r4 | r7 )
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.w = 0.0f",output);
                    AddInst(vp,c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }

    if (str_match0(chaine,"DisplacementOffset"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==2)
            {
                rn=var(rns[0]); // Displace.xyzw
                rn2=var(rns[1]); // Rel.xy
                
				_sprintf(tmp, "r%d", new_temp_register());

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
					//	r8 = Displace
					//	r4 = Rel
					//	r5 = UV
                    _sprintf(c.str,"%s.a = 0.08 * (%s.a - 0.5)",tmp,rn);  // r1.a = 0.04 * bx2(r8.a);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xy = %s.aa * %s.xy",output,tmp,rn2);  // r5.r += r1.a * r4.r;
                    AddInst(vp,c);
                    //_sprintf(c.str,"%s.y = %s.a * %s.y",output,tmp,rn2);  // r5.g -= r1.a * r4.g;
                    //AddInst(vp,c);
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
					//	r8 = Displace
					//	r4 = Rel
					//	r5 / output = UV
                    _sprintf(c.str,"%s.a = 0.08f * (%s.a - 0.5f)",tmp,rn);  // r1.a = 0.04 * bx2(r8.a);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.xy = %s.aa * %s.xy",output,tmp,rn2);  // r5.r += r1.a * r4.r;
                    AddInst(vp,c);
                    //_sprintf(c.str,"%s.y = %s.a * %s.y",output,tmp,rn2);  // r5.g -= r1.a * r4.g;
                    //AddInst(vp,c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }

    if (str_match0(chaine,"AddDisplacementOffset"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==2)
            {
                rn=var(rns[0]); // Displace.xyzw
                rn2=var(rns[1]); // Rel.xy
                
				_sprintf(tmp, "r%d", new_temp_register());

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
					//	r8 = Displace
					//	r4 = Rel
					//	r5 = UV
                    _sprintf(c.str,"%s.a = 0.08 * (%s.a - 0.5)",tmp,rn);  // r1.a = 0.04 * bx2(r8.a);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x += %s.a * %s.x",output,tmp,rn2);  // r5.r += r1.a * r4.r;
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y -= %s.a * %s.y",output,tmp,rn2);  // r5.g -= r1.a * r4.g;
                    AddInst(vp,c);
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
					//	r8 = Displace
					//	r4 = Rel
					//	r5 / output = UV
                    _sprintf(c.str,"%s.a = 0.08f * (%s.a - 0.5f)",tmp,rn);  // r1.a = 0.04 * bx2(r8.a);
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.x += %s.a * %s.x",output,tmp,rn2);  // r5.r += r1.a * r4.r;
                    AddInst(vp,c);
                    _sprintf(c.str,"%s.y -= %s.a * %s.y",output,tmp,rn2);  // r5.g -= r1.a * r4.g;
                    AddInst(vp,c);
                }
                return output;
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
    }

    if (str_match0(chaine,"CalculateShadowMappingIntegrate"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==3)
            {
                rn=var(rns[0]); // posi
                rn2=var(rns[1]); // zval
                
                // output.x = 0 or 1
                //rns[2]=nt
                
                _sprintf(tmp, "r%d", new_temp_register());
                int last_temp=last_temp_register;
        /*
                float calcshadow1(vec4 pos,float zval)
                {
                    tmp = sample(2,pos.xy);
                    tmp = RGBToValue(tmp);
                    tmp.r = zval - tmp.x;
                    tmp = cmp(tmp.r,psone,pszero);
                    tmp.x = BoundsViewAffect(pos);
                    tmp.x;
                };
        */
                // tmp = div

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
                    _sprintf(tmp2,"sample(%s,%s.xy)",rns[2],rn);
                    compile(tmp2,tmp,vp);
                    close_temp_register(last_temp);
#ifdef IOS
                    _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", tmp,tmp,tmp);
                    AddInst(vp, c);
#else
                    _sprintf(c.str, "%s.x = dot(%s , vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0))", tmp,tmp);
                    AddInst(vp, c);
#endif

                    //_sprintf(tmp2,"RGBToValue(%s)",tmp);
                    //compile(tmp2,tmp,vp);

                    _sprintf(c.str, "%s.x = ( %s >= %s.x ) ? 0.0 : 1.0", tmp, rn2, tmp);
                    AddInst(vp, c);

                    _sprintf(tmp2,"BoundsViewIntegrate(%s)",rn);
                    _sprintf(tmp3,"%s.x",tmp);
                    compile(tmp2,tmp3,vp);
                    close_temp_register(last_temp);

                    int sc = str_last_char(output, '.');
                    if (sc<0)
                    {
                        _sprintf(c.str,"%s.x = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                    else
                    {
                        _sprintf(c.str,"%s = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
                    _sprintf(tmp2,"sample(%s,%s.xy)",rns[2],rn);
                    compile(tmp2,tmp,vp);
                    close_temp_register(last_temp);
                    
//#ifdef IOS
//                    _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", tmp,tmp,tmp);
//                    AddInst(vp, c);
//#else
                    _sprintf(c.str, "%s.x = dot(%s , float4(1.0f/(256.0f*256.0f*256.0f), 1.0f/(256.0f*256.0f), 1.0f/256.0f, 1.0f))", tmp, tmp);
                    AddInst(vp, c);
//#endif

                    _sprintf(c.str, "%s.x = ( %s >= %s.x ) ? 0.0f : 1.0f", tmp, rn2, tmp);
                    AddInst(vp, c);

                    _sprintf(tmp2,"BoundsViewIntegrate(%s)",rn);
                    _sprintf(tmp3,"%s.x",tmp);
                    compile(tmp2,tmp3,vp);
                    close_temp_register(last_temp);

                    int sc = str_last_char(output, '.');
                    if (sc<0)
                    {
                        _sprintf(c.str,"%s.x = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                    else
                    {
                        _sprintf(c.str,"%s = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                }
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
        
        return output;
    }
    else
    if (str_match0(chaine,"CalculateShadowMapping"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }
        else
        {
            if (multiple_argument_parse_virg(tmp,rns)==3)
            {
                rn=var(rns[0]); // posi
                rn2=var(rns[1]); // zval
                
                // output.x = 0 or 1
                //rns[2]=nt
                
                _sprintf(tmp, "r%d", new_temp_register());
                int last_temp=last_temp_register;
        /*
                float calcshadow1(vec4 pos,float zval)
                {
                    tmp = sample(2,pos.xy);
                    tmp = RGBToValue(tmp);
                    tmp.r = zval - tmp.x;
                    tmp = cmp(tmp.r,psone,pszero);
                    tmp.x = BoundsViewAffect(pos);
                    tmp.x;
                };
        */
                // tmp = div

                if ((api == 1) && (shadermodel))  // OPEN GL
                {
                    _sprintf(tmp2,"sample(%s,%s.xy)",rns[2],rn);
                    compile(tmp2,tmp,vp);
                    close_temp_register(last_temp);
#ifdef IOS
                    _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", tmp,tmp,tmp);
                    AddInst(vp, c);
#else
                    _sprintf(c.str, "%s.x = dot(%s , vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0))", tmp,tmp);
                    AddInst(vp, c);
#endif

                    //_sprintf(tmp2,"RGBToValue(%s)",tmp);
                    //compile(tmp2,tmp,vp);

                    _sprintf(c.str, "%s.x = ( %s >= %s.x ) ? 1.0 : 0.0", tmp, rn2, tmp);
                    AddInst(vp, c);

                    _sprintf(tmp2,"BoundsViewAffect(%s)",rn);
                    _sprintf(tmp3,"%s.x",tmp);
                    compile(tmp2,tmp3,vp);
                    close_temp_register(last_temp);

                    int sc = str_last_char(output, '.');
                    if (sc<0)
                    {
                        _sprintf(c.str,"%s.x = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                    else
                    {
                        _sprintf(c.str,"%s = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                }
                else
                if ((api == 0) && (shadermodel3))   // D3D METAL 0.0f
                {
                    _sprintf(tmp2,"sample(%s,%s.xy)",rns[2],rn);
                    compile(tmp2,tmp,vp);
                    close_temp_register(last_temp);
                    
//#ifdef IOS
//                    _sprintf(c.str, "%s.x = (255.0 * %s.g + %s.r)/255.0", tmp,tmp,tmp);
//                    AddInst(vp, c);
//#else
                    _sprintf(c.str, "%s.x = dot(%s , float4(1.0f/(256.0f*256.0f*256.0f), 1.0f/(256.0f*256.0f), 1.0f/256.0f, 1.0f))", tmp, tmp);
                    AddInst(vp, c);
//#endif
                    _sprintf(c.str, "%s.x = ( %s >= %s.x ) ? 1.0f : 0.0f", tmp, rn2, tmp);
                    AddInst(vp, c);

                    _sprintf(tmp2,"BoundsViewAffect(%s)",rn);
                    _sprintf(tmp3,"%s.x",tmp);
                    compile(tmp2,tmp3,vp);
                    close_temp_register(last_temp);

                    int sc = str_last_char(output, '.');
                    if (sc<0)
                    {
                        _sprintf(c.str,"%s.x = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                    else
                    {
                        _sprintf(c.str,"%s = %s.x",output,tmp);
                        AddInst(vp,c);
                    }
                }
            }
            else
            {
                SYNTAXERROR=PARAMERROR=true;
                if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
                return "";
            }
        }
        
        return output;
    }

    char *_minimum="0.001";         // values for clipping inside shadow map
    char *_maximum="0.999";
    char *_znear="0.001";

    if (str_match0(chaine,"BoundsViewIntegrate"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            rn=compile(tmp,tmp2,vp);
        }
        
        int sc = str_last_char(output, '.');
        if (sc<0)
        {
        }
        else
        {
            if ((api==1)&&(shadermodel))
            {
                _sprintf(c.str, "if (%s.x < %s)", rn, _minimum);
                AddInst(vp, c);
                _sprintf(c.str, "{");
                AddInst(vp, c);
                _sprintf(c.str, "%s = 1.0",output);
                AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst(vp, c);
                
                _sprintf(c.str, "if (%s.y < %s)", rn, _minimum);
                AddInst(vp, c);
                _sprintf(c.str, "{");
                AddInst(vp, c);
                _sprintf(c.str, "%s = 1.0",output);
                AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst(vp, c);
                
                _sprintf(c.str, "if (%s.x > %s)", rn, _maximum);
                AddInst(vp, c);
                _sprintf(c.str, "{");
                AddInst(vp, c);
                _sprintf(c.str, "%s = 1.0",output);
                AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst(vp, c);
                
                _sprintf(c.str, "if (%s.y > %s)", rn, _maximum);
                AddInst(vp, c);
                _sprintf(c.str, "{");
                AddInst(vp, c);
                _sprintf(c.str, "%s = 1.0",output);
                AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst(vp, c);
                
                _sprintf(c.str, "if (%s.z < %s)", rn, _znear);
                AddInst(vp, c);
                _sprintf(c.str, "{");
                AddInst(vp, c);
                _sprintf(c.str, "%s = 1.0",output);
                AddInst(vp, c);
                _sprintf(c.str, "}");
                AddInst(vp, c);
            }
            else
                if ((api==0)&&(shadermodel3))
                {
                    _sprintf(c.str, "if (%s.x < %sf) %s = 1.0f", rn, _minimum, output);
                    AddInst(vp, c);
                    _sprintf(c.str, "if (%s.y < %sf) %s = 1.0f", rn, _minimum, output);
                    AddInst(vp, c);
                    _sprintf(c.str, "if (%s.x > %sf) %s = 1.0f", rn, _maximum, output);
                    AddInst(vp, c);
                    _sprintf(c.str, "if (%s.y > %sf) %s = 1.0f", rn, _maximum, output);
                    AddInst(vp, c);
                    _sprintf(c.str, "if (%s.z < %sf) %s = 1.0f", rn, _znear, output);
                    AddInst(vp, c);
                }
        }
        return output;
    }

    if (str_match0(chaine,"BoundsViewAffect"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            rn=compile(tmp,tmp2,vp);
        }

        int sc = str_last_char(output, '.');
        if (sc<0)
        {
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str, "if (%s.x < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.y < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.x > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.y > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.z < %s)", rn, _znear);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str, "if (%s.x < %sf) %s = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y < %sf) %s = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x > %sf) %s = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y > %sf) %s = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.z < %sf) %s = 0.0f", rn, _znear, output);
				AddInst(vp, c);
			}
		}
        return output;
    }

    if (str_match0(chaine,"BoundsView"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            rn=compile(tmp,tmp2,vp);
        }

        int sc = str_last_char(output, '.');
        if (sc<0)
        {
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str, "%s.w = 1.0", output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);


				_sprintf(c.str, "if (%s.y < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.x > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.y > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.z < %s)", rn, _znear);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s.w = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str, "%s.w = 1.0f", output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x < %sf) %s.w = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y < %sf) %s.w = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x > %sf) %s.w = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y > %sf) %s.w = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.z < %sf) %s.w = 0.0f", rn, _znear,output);
				AddInst(vp, c);
			}
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str, "%s = 1.0", output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.y < %s)", rn, _minimum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.x > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.y > %s)", rn, _maximum);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);

				_sprintf(c.str, "if (%s.z < %s)", rn, _znear);
				AddInst(vp, c);
				_sprintf(c.str, "{");
				AddInst(vp, c);
				_sprintf(c.str, "%s = 0.0",output);
				AddInst(vp, c);
				_sprintf(c.str, "}");
				AddInst(vp, c);
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				_sprintf(c.str, "%s = 1.0f", output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x < %sf) %s = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y < %sf) %s = 0.0f", rn, _minimum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.x > %sf) %s = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.y > %sf) %s = 0.0f", rn, _maximum, output);
				AddInst(vp, c);
				_sprintf(c.str, "if (%s.z < %sf) %s = 0.0f", rn, _znear, output);
				AddInst(vp, c);
			}
		}
        return output;
    }
    
    if (str_match0(chaine,"ScaleToSecondary"))
    {
        _sprintf(tmp,"%s",str_return_parentheses(chaine));
        if (strlen(tmp)==0)
        {
            SYNTAXERROR=PARAMERROR=true;
            if (PRMERRORSTR[0]==0) strcpy(PRMERRORSTR,chaine);
            return "";
        }

        if (str_simple(tmp)) rn=var(tmp);
        else
        {
            _sprintf(tmp2,"r%d",new_temp_register());
            resfield(output, tmp2);
            rn=compile(tmp,tmp2,vp);
        }
        
        int sc = str_last_char(rn, '.');
        if (sc>=0)
        {
        
            if ((api==1)&&(shadermodel))
            {
                _sprintf(c.str, "%s = vec2(0.5,0.5) + vec2(0.5,0.5) * %s", output, rn);
                AddInst(vp, c);
            }
            else
            if ((api==0)&&(shadermodel3))
            {
                _sprintf(c.str, "%s = float2(0.5f,0.5f) + float2(0.5f,-0.5f) * %s", output, rn);
                AddInst(vp, c);
            }
        }
        else
        {
            if ((api==1)&&(shadermodel))
            {
                _sprintf(c.str, "%s.xy = vec2(0.5,0.5) + vec2(0.5,0.5) * %s.xy", output, rn);
                AddInst(vp, c);
            }
            else
            if ((api==0)&&(shadermodel3))
            {
                _sprintf(c.str, "%s.xy = float2(0.5f,0.5f) + float2(0.5f,-0.5f) * %s.xy", output, rn);
                AddInst(vp, c);
            }
        }
        return output;
    }

    if (str_match00(chaine,"SecondarySign"))
    {        
        if ((api==1)&&(shadermodel))
        {
			// 1
			int sc=str_last_char(output,'.');
			if (sc!=-1)
			{
				switch (strlen(&output[sc+1]))
				{
				case 1:
					_sprintf(tmp,"1.0");
					break;
				case 2:
					_sprintf(tmp,"vec2(1.0,1.0)");
					break;
				case 3:
					_sprintf(tmp,"vec3(1.0,1.0,1.0)");
					break;
				case 4:
					_sprintf(tmp,"vec4(1.0,1.0,1.0,1.0)");
					break;
				};
			}
			else _sprintf(tmp,"vec4(1.0,1.0,1.0,1.0)");
        }
        else
        if ((api==0)&&(shadermodel3))
        {
			// -1
			int sc=str_last_char(output,'.');
			if (sc!=-1)
			{
				switch (strlen(&output[sc+1]))
				{
				case 1:
					_sprintf(tmp,"-1.0");
					break;
				case 2:
					_sprintf(tmp,"float2(-1.0,-1.0)");
					break;
				case 3:
					_sprintf(tmp,"float3(-1.0,-1.0,-1.0)");
					break;
				case 4:
					_sprintf(tmp,"float4(-1.0,-1.0,-1.0,-1.0)");
					break;
				};
			}
			else _sprintf(tmp,"float4(-1.0,-1.0,-1.0,-1.0)");
        }

        _sprintf(c.str, "%s = %s", output, tmp);
        AddInst(vp, c);

		return output;
    }

	if (str_match0(chaine,"DecalMappingSecondary"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				resfield(output, tmp3);
				rn2=compile(&tmp[p+1],tmp3,vp);
				t=1;
			}
		}

		if (api!=2)
		{
			if ((api==1)&&(shadermodel))
			{
				_sprintf(c.str,"%s = vec2( %s , -%s )",output,rn,rn2);
				AddInst(vp,c);
				return output;
			}
			else
			{
				_sprintf(c.str,"%s = float2( %s , %s )",output,rn,rn2);
				AddInst(vp,c);
				return output;
			}
		}

		return output;
	}


	if (str_match0(chaine,"max"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));
		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				resfield(output, tmp2);
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp3,"r%d",new_temp_register());
				resfield(output, tmp3);
				rn2=compile(&tmp[p+1],tmp3,vp);
				t=1;
			}
		}

		if (api==2)
		{
			em->Add(OP_MAX,tmp2,rn,rn2);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield2(tmp4, output, rn, rn2);
				_sprintf(c.str,"%s = max( %s , %s )",tmp4,rn,rn2);
				AddInst(vp,c);
				return tmp4;
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_max][api],output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}


	if (str_match0(chaine,"equation"))		// equation(normal,point)
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p=str_char_prt(tmp,',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		_sprintf(tmp2,"%s",output);
		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				if (t==0) _sprintf(tmp2,"r%d",new_temp_register());
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
		}

		if (t==0) tmp2=output;

		if (api==2)
		{
			em->Add(OP_EQPLANE,tmp2,rn,rn2);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = -dot( %s , %s )",tmp2,rn,rn2);
				AddInst(vp,c);
			}
			else
			{
				// a,b,c = normal
				// d = -(N|P)
				_sprintf(c.str,"%s %s,%s,-%s",Inst[_dp3][api],output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return tmp2;
	}

	if (str_match0(chaine,"plane"))		// plane(vector,plane_equation)
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p=str_char_prt(tmp,',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		_sprintf(tmp2,"%s",output);
		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2,"r%d",new_temp_register());
				rn=compile(&tmp[0],tmp2,vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				if (t==0) _sprintf(tmp2,"r%d",new_temp_register());
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
		}

		if (t==0) tmp2=output;

		_sprintf(tmp3,"r%d",new_temp_register());

		if (api==2)
		{
			em->Add(OP_PLANE,tmp2,rn,rn2);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				_sprintf(c.str,"%s = dot( %s , %s )",tmp2,rn,rn2);
				AddInst(vp,c);
			}
			else
			{
				_sprintf(c.str,"%s %s,%s",Inst[_mov][api],tmp3,rn);
				AddInst(vp,c);
				_sprintf(c.str,"%s %s.w,%s.z",Inst[_mov][api],tmp3,var("trigo_cst"));
				AddInst(vp,c);
				_sprintf(c.str,"%s %s,%s,%s",Inst[_dp4][api],tmp2,tmp3,rn2);
				AddInst(vp,c);
			}
		}

		return tmp2;
	}

	if (str_match0(chaine,"min"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p=str_char_prt(tmp,',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);
				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);
				rn = compile(&tmp[0], tmp2, vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);
				rn2=compile(&tmp[p+1],tmp2,vp);
				t=1;
			}
		}

		if (api==2)
		{
			em->Add(OP_MIN,tmp2,rn,rn2);
		}
		else
		{
			if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
			{
				outputfield2(tmp4, output, rn, rn2);
				_sprintf(c.str,"%s = min( %s , %s )",tmp4,rn,rn2);
				AddInst(vp,c);

				return tmp4;
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_min][api],output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if (str_match0(chaine,"pow"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p=str_char_prt(tmp,',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';
	
		t=0;
		if (str_simple(&tmp[0]))
		{
			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);
				rn2 = compile(&tmp[p + 1], tmp2, vp);
				t=1;
			}
			rn=var(&tmp[0]);
		}
		else
		{
			if (str_simple(&tmp[0])) rn=var(&tmp[0]);
			else
			{
				_sprintf(tmp2, "r%d", new_temp_register());
				resfield(output, tmp2);
				rn = compile(&tmp[0], tmp2, vp);
				t=1;
			}

			if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
			else
			{
				if (t==0) _sprintf(tmp3,"r%d",new_temp_register());
				_sprintf(tmp3, "r%d", new_temp_register());
				resfield(output, tmp3);
				rn2 = compile(&tmp[p + 1], tmp3, vp);
				t=1;
			}
		}

		if (((api==1)&&(shadermodel))||((api==0)&&(shadermodel3)))
		{
            int nbm=nb_modifiers_syntax(rn);
			int nbm2=nb_modifiers_syntax(rn2);
            if (nbm>4) TYPEERROR=true;
			if (nbm2>4) TYPEERROR=true;
            
            if ((nbm>1)&&(nbm2==-1))
            {
                int sc = str_last_char(output, '.');
                
                _sprintf(tmp3, output);
                if (sc>=0) tmp3[sc]='\0';

                sc = str_last_char(rn, '.');
                _sprintf(tmp4, rn);
                tmp4[sc]='\0';
                
                for (int ki=0;ki<nbm;ki++)
                {
                    _sprintf(c.str,"%s.%c = pow( %s.%c , %s )",tmp3,rn[sc+1+ki],tmp4,rn[sc+1+ki],rn2);
                    AddInst(vp,c);
                }
                
                return output;
            }
            else
            {                
                outputfield2(tmp4, output, rn, rn2);
                _sprintf(c.str,"%s = pow( abs(%s) , %s )",tmp4,rn,rn2);
                AddInst(vp,c);
            }
		}

		return tmp4;
	}

	if (str_match0(chaine,"sge"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else rn=compile(&tmp[0],output,vp);

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else rn2=compile(&tmp[p+1],output,vp);

		if (api==2)
		{
			em->Add(OP_SGE,output,rn,rn2);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				int sc=str_last_char(rn,'.');
				if (sc==-1)
				{
					int sc2=str_last_char(rn2,'.');
					if (sc2==-1)
					{
						_sprintf(c.str, "%s.x = ( %s.x >= %s.x ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.y = ( %s.y >= %s.y ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.z = ( %s.z >= %s.z ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.w = ( %s.w >= %s.w ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
					}
					else
					{
						_sprintf(c.str,"%s = ( %s >= %s ) ? 1.0 : 0.0",output,rn,rn2);
						AddInst(vp,c);
					}
				}
				else
				{
					_sprintf(c.str,"%s = ( %s >= %s ) ? 1.0 : 0.0",output,rn,rn2);
					AddInst(vp,c);
				}
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				if (metal==1)
				{
					int sc=str_last_char(rn,'.');
					if (sc==-1)
					{
						int sc2=str_last_char(rn2,'.');
						if (sc2==-1)
						{
							_sprintf(c.str, "%s.x = ( %s.x >= %s.x ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.y = ( %s.y >= %s.y ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.z = ( %s.z >= %s.z ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.w = ( %s.w >= %s.w ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
						}
						else
						{
							_sprintf(c.str,"%s = ( %s >= %s ) ? 1.0f : 0.0f",output,rn,rn2);
							AddInst(vp,c);
						}
					}
					else
					{
						_sprintf(c.str,"%s = ( %s >= %s ) ? 1.0f : 0.0f",output,rn,rn2);
						AddInst(vp,c);
					}
				}
				else
				{
					_sprintf(c.str,"%s = ( %s >= %s ) ? 1.0f : 0.0f",output,rn,rn2);
					AddInst(vp,c);
				}
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_sge][api],output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}


	if (str_match0(chaine,"slt"))
	{
		_sprintf(tmp,"%s",str_return_parentheses(chaine));

		p = str_char_prt(tmp, ',');
        if (p<0) { SYNTAXERROR=true; return ""; }
		tmp[p]='\0';

		if (str_simple(&tmp[0])) rn=var(&tmp[0]);
		else rn=compile(&tmp[0],output,vp);

		if (str_simple(&tmp[p+1])) rn2=var(&tmp[p+1]);
		else rn2=compile(&tmp[p+1],output,vp);

		if (api==2)
		{
			em->Add(OP_SLT,output,rn,rn2);
		}
		else
		{
			if ((api==1)&&(shadermodel))
			{
				int sc=str_last_char(rn,'.');
				if (sc==-1)
				{
					int sc2=str_last_char(rn2,'.');
					if (sc2==-1)
					{
						_sprintf(c.str, "%s.x = ( %s.x < %s.x ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.y = ( %s.y < %s.y ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.z = ( %s.z < %s.z ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
						_sprintf(c.str, "%s.w = ( %s.w < %s.w ) ? 1.0 : 0.0", output, rn, rn2);
						AddInst(vp, c);
					}
					else
					{
						_sprintf(c.str,"%s = ( %s < %s ) ? 1.0 : 0.0",output,rn,rn2);
						AddInst(vp,c);
					}
				}
				else
				{
					_sprintf(c.str,"%s = ( %s < %s ) ? 1.0 : 0.0",output,rn,rn2);
					AddInst(vp,c);
				}
			}
			else
			if ((api==0)&&(shadermodel3))
			{
				if (metal==1)
				{
					int sc=str_last_char(rn,'.');
					if (sc==-1)
					{
						int sc2=str_last_char(rn2,'.');
						if (sc2==-1)
						{
							_sprintf(c.str, "%s.x = ( %s.x < %s.x ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.y = ( %s.y < %s.y ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.z = ( %s.z < %s.z ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
							_sprintf(c.str, "%s.w = ( %s.w < %s.w ) ? 1.0f : 0.0f", output, rn, rn2);
							AddInst(vp, c);
						}
						else
						{
							_sprintf(c.str,"%s = ( %s < %s ) ? 1.0f : 0.0f",output,rn,rn2);
							AddInst(vp,c);
						}
					}
					else
					{
						_sprintf(c.str,"%s = ( %s < %s ) ? 1.0f : 0.0f",output,rn,rn2);
						AddInst(vp,c);
					}
				}
				else
				{
					_sprintf(c.str,"%s = ( %s < %s ) ? 1.0f : 0.0f",output,rn,rn2);
					AddInst(vp,c);
				}
			}
			else
			{
				_sprintf(c.str,"%s %s,%s,%s",Inst[_slt][api],output,rn,rn2);
				AddInst(vp,c);
			}
		}

		return output;
	}

	if (!NoDepthOut)
		if (numberswz(chaine)>numberswz(output))
		{
			TYPEERROR=true;
		}
	
    _sprintf(tmp,"%s",var(chaine));
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char s_tmp_error[1024];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_get_RS(char * script,int *pos,CVertexProgram *vp)
{
	char *tmp;
	int apos;
	int *iptr;
	char *str;
	char ss[256];
	int n,nn;
	bool success=true;
	int value;
    
	apos=*pos;
	tmp=str_parse(script,pos);
	while (tmp)
	{
		str_clean(tmp);
		str_upcase(tmp);

        if (strlen(tmp)>0)
        {
            if (tmp[0]!='[')
            {
                if (str_match(tmp,"TEXTUREVS["))
                {
                    str=NULL;
                    n=0;

                    while ((n<16)&&(!str))
                    {
                        _sprintf(ss,"TEXTUREVS[%d]",n);
                        if (strcmp(tmp,ss)==0)
                        {
                            tmp=str_parse(script,pos);
                            str_clean(tmp);
                            for (nn=0;nn<16;nn++)
                            {
                                if (vp->texturevs[nn])
                                {
                                    if (strcmp(vp->texturevs[nn],tmp)==0) str=vp->texturevs[nn];
                                }
                            }
                            RS.TextureVS[n]=str;
                        }
                        n++;
                    }

                    if (!str)
                    {
                        if (success) strcpy(s_tmp_error,tmp);
                        success=false;
                    }

                    apos=*pos;
                    tmp=str_parse(script,pos);

                }
                else
                if (str_match(tmp,"TEXTURE["))
                {
                    str=NULL;
                    n=0;

                    while ((n<16)&&(!str))
                    {
                        _sprintf(ss,"TEXTURE[%d]",n);
                        if (strcmp(tmp,ss)==0)
                        {
                            tmp=str_parse(script,pos);
                            str_clean(tmp);
                            for (nn=0;nn<16;nn++)
                            {
                                if (vp->texture[nn])
                                {
                                    if (strcmp(vp->texture[nn],tmp)==0) str=vp->texture[nn];
                                }
                            }
                            RS.Texture[n]=str;
                        }
                        n++;
                    }

                    if (!str)
                    {
                        if (success) strcpy(s_tmp_error,tmp);
                        success=false;
                    }

                    apos=*pos;
                    tmp=str_parse(script,pos);
                }
                else
                {
                    iptr=str_ptr_to_var(tmp);
                    if (iptr)
                    {
                        apos=*pos;
                        tmp=str_parse(script,pos);

                        str_clean(tmp);
                        str_upcase(tmp);

                        value=str_int_value(tmp);

                        if (value!=-1)
                        {
                            *iptr=value;
                        }
                        else
                        {
                            if (success) strcpy(s_tmp_error,tmp);
                            success=false;
                        }

                        apos=*pos;
                        tmp=str_parse(script,pos);
                    }
                    else
                    {
                        if (success) strcpy(s_tmp_error,tmp);
                        success=false;

                        apos=*pos;
                        tmp=str_parse(script,pos);
                    }
                }
            }
            else tmp=NULL;
        }
        else tmp=str_parse(script,pos);
	}

	*pos=apos;
    
    if (RS.Blend!=_UNDEFINED)
    {
        if (RS.SrcBlend==_UNDEFINED)
        {
            RS.SrcBlend=_SRCALPHA;
            RS.DstBlend=_INVSRCALPHA;
        }
    }

	return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IfElse
{
public:
	char *IF;
	char *ELSE;
	char *BREAK;
	char *CONTINUE;
	char *DISCARD;
	int tag;

	IfElse() { IF=ELSE=DISCARD=BREAK=CONTINUE=NULL; }

	~IfElse() {
		if (IF) free(IF);
		if (ELSE) free(ELSE);
		BREAK=NULL;
		CONTINUE=NULL;
		DISCARD=NULL;
	}

	void AddContinue()
	{
		CONTINUE="continue";
	}

	void AddBreak()
	{
		BREAK="break";
	}

	void AddDiscard()
	{
		DISCARD="discard";
	}

	void AddIf(char *str)
	{
		if (IF) free(IF);

		IF=(char*) malloc(strlen(str)+1);
		_sprintf(IF,"%s",str);
	}

	void AddElse(char *str)
	{
		if (ELSE) free(ELSE);

		ELSE=(char*) malloc(strlen(str)+1);
		_sprintf(ELSE,"%s",str);
	}

	void AddIfSM(char *str)
	{
		char ss[512];
		if (IF) _sprintf(ss,"%s%s\n",IF,str);
		else _sprintf(ss,"%s\n",str);
		if (IF) free(IF);
		IF=(char*) malloc(strlen(ss)+1);
		_sprintf(IF,"%s",ss);
	}

	void AddElseSM(char *str)
	{
		char ss[512];
		if (ELSE) _sprintf(ss,"%s%s\n",IF,str);
		else _sprintf(ss,"%s\n",str);
		if (ELSE) free(ELSE);
		ELSE=(char*) malloc(strlen(ss)+1);
		_sprintf(ELSE,"%s",ss);
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool str_pre_compile_expression(char * str,CMap <Chaine,IfElse> * cm,int tag)
{
	char chaine[1024];
	int p;
	IfElse * ie;
	Chaine s;

	if (str_match(str,"break"))
	{
		_sprintf(s.str,"bk%d",cm->Length());
		ie=(*cm)[s];
		ie->AddBreak();
		ie->tag=tag;
	}
	else
	if (str_match(str,"continue"))
	{
		_sprintf(s.str,"co%d",cm->Length());
		ie=(*cm)[s];
		ie->AddContinue();
		ie->tag=tag;
	}
	else
	if (str_match(str,"discard"))
	{
		_sprintf(s.str,"di%d",cm->Length());
		ie=(*cm)[s];
		ie->AddDiscard();
		ie->tag=tag;
	}
	else
	if ((str_char(str,'=')!=-1))
	{
		_sprintf(chaine,"%s",str);
		str_clean(chaine);

		p=str_char(chaine,'=');
        if (p<0) return false;
		chaine[p]='\0';

		_sprintf(s.str,"%s",chaine);

		ie=(*cm)[s];

		if (tag) ie->AddIf(&chaine[p+1]);
		else ie->AddElse(&chaine[p+1]);
	}
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * str_extend_c(char * script)
{
	int pos=0;
	int apos;
	char *tmp;
	char *buffer_tmp;
	bool b=false;
	int size;
	int level=0;
	buffer_tmp=(char*) malloc(65536*2);
	size=0;

	while (!b)
	{
		apos=pos;
		if (script[pos]==0) b=true;
		else
		{
			if (str_mm(&script[pos],"if("))
			{
				tmp=str_parse_char_prths(script,&pos,')');
                if (tmp)
                {
                    memcpy(&buffer_tmp[size],tmp,strlen(tmp));
                    size+=(int)strlen(tmp);

                    buffer_tmp[size++]='{';

                    if (script[pos]=='{') pos++;
                    else level=1;
                }
                else buffer_tmp[size++]=script[pos++];
			}
			else
			if (script[pos]==';')
			{
				buffer_tmp[size++]=';';
				if (level==1) buffer_tmp[size++]='}';
				level=0;
				pos++;
			}
			else
			if (str_mm(&script[pos],"}else{"))
			{
				buffer_tmp[size++]='}';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='l';
				buffer_tmp[size++]='s';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='{';

				pos+=6;
			}
			else
            if (str_mm(&script[pos],"}else"))
            {
                buffer_tmp[size++]='}';
                buffer_tmp[size++]='e';
                buffer_tmp[size++]='l';
                buffer_tmp[size++]='s';
                buffer_tmp[size++]='e';
                buffer_tmp[size++]='{';
                
                pos+=5;
            }
            else
			if (str_mm(&script[pos],"else{"))
			{
				if (level==1) buffer_tmp[size++]='}';
				level=0;
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='l';
				buffer_tmp[size++]='s';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='{';

				pos+=5;
			}
			else
			if (str_mm(&script[pos],"else"))
			{
				if (level==1) buffer_tmp[size++]='}';
				level=0;
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='l';
				buffer_tmp[size++]='s';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='{';
				level=1;
				pos+=4;
			}
			else
			if (script[pos]=='}')
			{
				buffer_tmp[size++]='}';
				pos++;
			}
			else
			{
				buffer_tmp[size++]=script[pos++];
			}
		}

		/*
		tmp=str_parse_virg(script,&pos);
		if (!tmp) b=true;
		else
		{			
			if ( (!str_match(tmp,"if(")) && (!str_match(tmp,"else")) )
			{
				memcpy(&buffer_tmp[size],tmp,strlen(tmp));
				size+=(int)strlen(tmp);
				buffer_tmp[size++]=';';
			}
			else
			if (str_match(tmp,"if("))
			{				
				printf("if():%s\n",tmp);
				int pp=str_char(tmp,'{');

				if (pp==-1)
				{
					char ss[1024];
					pos=apos;
					tmp=str_parse_char_prths(script,&pos,')');

					memcpy(&ss[0],tmp,strlen(tmp));
					int len=(int)strlen(tmp);
					//ss[len++]=')';
					ss[len++]='{';
					tmp=str_parse_char(script,&pos,';');

					memcpy(&ss[len],tmp,strlen(tmp));
					len+=(int)strlen(tmp);
					ss[len++]=';';
					ss[len++]='}';
					ss[len++]='\0';

					printf("-1:%s\n",ss);

					memcpy(&buffer_tmp[size],ss,strlen(ss));
					size+=(int)strlen(ss);

				}
				else
				{
					pos=apos;
					tmp=str_parse_char_prths(script,&pos,')');

					memcpy(&buffer_tmp[size],tmp,strlen(tmp));
					size+=(int)strlen(tmp);

					printf("...:%s\n",tmp);

					//buffer_tmp[size++]=')';
					buffer_tmp[size++]='{';
					tmp=str_parse_char(script,&pos,'{');
				}
			}
			else
			if (str_match(tmp,"else"))
			{
				printf("else:%s\n",tmp);

				if (tmp[0]=='}') buffer_tmp[size++]='}';

				pos=apos;
				while (!((script[pos+0]=='e')&&(script[pos+1]=='l')&&(script[pos+2]=='s')&&(script[pos+3]=='e'))) pos++;

				buffer_tmp[size++]='e';
				buffer_tmp[size++]='l';
				buffer_tmp[size++]='s';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='{';

				pos+=4;

				if (script[pos]=='{') pos++;
					
				tmp=str_parse_char(script,&pos,';');

				memcpy(&buffer_tmp[size],tmp,(int)strlen(tmp));
				size+=(int)strlen(tmp);
				buffer_tmp[size++]=';';
				buffer_tmp[size++]='}';
			}
		}
		/**/
	}

	buffer_tmp[size]='\0';

	return buffer_tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool continueorbreak(CMap <Chaine,IfElse> *cnv)
{
	bool res=true;
	IfElse * ie;

	ie=cnv->GetFirst();
	while (ie)
	{
		if (!((ie->CONTINUE)||(ie->BREAK)||(ie->DISCARD)))
		{
			res=false;
		}
		ie=cnv->GetNext();
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::str_if_else_virg_treatment(char * script00)
{
	int p;
	int pos=0;
	int apos;
	char *tmp;
	char *buffer, *buffer_tmp;
	bool b = false;
	char tmp1[2048];
	char tmp2[2048];
	char tmp666[2048];
	char temp[2048];
	char test[512];
	int pos1,pos2;
	int size;
	int level=0;
	buffer_tmp=(char*) malloc(65536*2);
	size=0;
	CMap <Chaine,IfElse> cnv;
	IfElse * ie;
	Chaine c;

	char *script=str_extend_c(script00);
	
	if (shadermodel3)
	{		
		int len=(int)strlen(script);
		int level=0;
		int n=0;
		while ((n<len)&&(!IFERROR))
		{
			if ((script[n]=='i')&&(script[n+1]=='f')&&(script[n+2]=='('))
			{
                int old=size;
				buffer_tmp[size++]='#';
				buffer_tmp[size++]='i';
				buffer_tmp[size++]='f';				
				n+=2;
                int op=0;
                int prths=0;
				while ((script[n]!='{')&&(n<len)&&(!IFERROR))
				{
                    if (script[n]=='>') op=1;
                    if (script[n]=='<') op=1;
                    if (script[n]=='=') op=1;
                    
                    if (script[n]=='(') prths++;
                    if (script[n]==')') prths--;
                    if (script[n]==';') IFERROR=true;
					buffer_tmp[size++]=script[n++];
				}
                
                if (op==0) IFERROR=true;
                if (prths>0) IFERROR=true;
                
                if (IFERROR)
                {
                    int nn=n-old;
                    if (nn>0)
                    {
                        memcpy(ERRORSTR,&script[old],nn);
                        ERRORSTR[nn]=0;
                    }
                    else ERRORSTR[0]=0;
                }
                
				if (n<len) n++;
				level++;
				buffer_tmp[size++]='\n';
			}
			else
			if ((script[n]=='}')&&(script[n+1]=='e')&&(script[n+2]=='l')&&(script[n+3]=='s')&&(script[n+4]=='e')&&(script[n+5]=='{'))
			{
				buffer_tmp[size++]='#';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='l';
				buffer_tmp[size++]='s';
				buffer_tmp[size++]='e';
				buffer_tmp[size++]='\n';
				n+=6;
			}
			else
			if (script[n]=='{')
			{
				buffer_tmp[size++]=script[n++];
			}
			else
			if (script[n]=='}')
			{
				if (level>0)
				{
					buffer_tmp[size++]='#';
					buffer_tmp[size++]='e';
					buffer_tmp[size++]='n';
					buffer_tmp[size++]='d';
					buffer_tmp[size++]='i';
					buffer_tmp[size++]='f';
					buffer_tmp[size++]='\n';
					level--;
					if (level<0) { SYNTAXERROR=true; return ""; }
					n++;
				}
				else
				{
					buffer_tmp[size++]=script[n++];
				}
			}
			else
			{
				if (script[n]==';')
				{
					buffer_tmp[size++]='\n';
					n++;
				}
				else buffer_tmp[size++]=script[n++];
			}
		}
        
        if (!SYNTAXERROR)
        {
            buffer=(char*) malloc(size+1);
            memcpy(buffer,buffer_tmp,size);
            buffer[size]='\0';
        }
        else buffer=NULL;

		free(buffer_tmp);
		free(script);

		return buffer;
	}

	while (!b)
	{
		apos=pos;
		tmp=str_parse_virg(script,&pos);
		if (!tmp) b=true;
		else
		{
			if (!str_match(tmp,"if("))
			{
				memcpy(&buffer_tmp[size],tmp,strlen(tmp));
				size+=(int)strlen(tmp);
				buffer_tmp[size]='\n';
				size++;
			}
			else
			{
				pos=apos;
				tmp=str_parse_char(script,&pos,'{');

				_sprintf(test,"%s",str_return_parentheses(tmp));

				pos=apos;
				_sprintf(tmp1,"%s",str_parse_char(script,&pos,'}'));

				pos1=str_char(tmp1,'{')+1;

				if (shadermodel3)
				{
					ie=cnv["test"];

					tmp=str_parse_virg(tmp1,&pos1);
					while (tmp)
					{
						ie->AddIfSM(tmp);
						tmp=str_parse_virg(tmp1,&pos1);
					}

					if ((script[pos+0]=='e')&&(script[pos+1]=='l')&&(script[pos+2]=='s')&&(script[pos+3]=='e'))
					{
						_sprintf(tmp2,"%s",str_parse_char(script,&pos,'}'));

						pos2=str_char(tmp2,'{')+1;
						tmp=str_parse_virg(tmp2,&pos2);

						while (tmp)
						{
							ie->AddElseSM(tmp);
							tmp=str_parse_virg(tmp2,&pos2);
						}
					}

				}
				else
				{
					tmp=str_parse_virg(tmp1,&pos1);

					while (tmp)
					{
                        if (!str_pre_compile_expression(tmp,&cnv,1)) { SYNTAXERROR=true; return ""; }
						tmp=str_parse_virg(tmp1,&pos1);
					}

					if ((script[pos+0]=='e')&&(script[pos+1]=='l')&&(script[pos+2]=='s')&&(script[pos+3]=='e'))
					{
						_sprintf(tmp2,"%s",str_parse_char(script,&pos,'}'));

						pos2=str_char(tmp2,'{')+1;
						tmp=str_parse_virg(tmp2,&pos2);

						while (tmp)
						{
							if (!str_pre_compile_expression(tmp,&cnv,0)) { SYNTAXERROR=true; return ""; }
							tmp=str_parse_virg(tmp2,&pos2);
						}
					}
				}
				tmp=temp;

				if ((!continueorbreak(&cnv))&&(!shadermodel3))
				{
					char sstmp[1024];
					strcpy(sstmp,test);

					if (str_char(test, '>') != -1)
					{
						p=str_char(test,'>');
						test[p]='\0';
						if (test[p + 1] == '=') p++;
						_sprintf(tmp, "%s.x=sge(%s,%s)", RIF, test, &test[p + 1]);
						_sprintf(tmp666, "%s.y=slt(%s,%s)", RIF, test, &test[p + 1]);
					}
					else
					{
						p=str_char(test,'<');
						test[p]='\0';
						if (test[p + 1] == '=') p++;
						_sprintf(tmp, "%s.x=sge(%s,%s)", RIF, &test[p + 1], test);
						_sprintf(tmp666, "%s.y=slt(%s,%s)", RIF, &test[p + 1], test);
					}

					memcpy(&buffer_tmp[size],tmp,strlen(tmp));
					size+=(int)strlen(tmp);
					buffer_tmp[size]='\n';
					size++;

					memcpy(&buffer_tmp[size],tmp666,strlen(tmp666));
					size+=(int)strlen(tmp666);
					buffer_tmp[size]='\n';
					size++;

					strcpy(test,sstmp);
				}

				ie=cnv.GetFirst();
				while (ie)
				{
					c=cnv.GetId();

					if (ie->CONTINUE)
					{
						if (str_char(test, '>') != -1)
						{
							p = str_char(test, '>');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s>=%s)continue;", test, &test[p + 1]);
							}
							else _sprintf(tmp, "if(%s>%s)continue;", test, &test[p + 1]);
						}
						else
						{
							p = str_char(test, '<');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s<=%s)continue;", test, &test[p + 1]);
							}
							else _sprintf(tmp, "if(%s<%s)continue;", test, &test[p + 1]);
						}
						memcpy(&buffer_tmp[size], tmp, strlen(tmp));
						size+=(int)strlen(tmp);
						buffer_tmp[size]='\n';
						size++;
						ie=NULL;
					}
					else
					if (ie->DISCARD)
					{
						if (str_char(test, '>') != -1)
						{
							p = str_char(test, '>');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s>=%s)%s;", test, &test[p + 1],sDiscardAPI);
							}
							else _sprintf(tmp, "if(%s>%s)%s;", test, &test[p + 1],sDiscardAPI);
						}
						else
						{
							p = str_char(test, '<');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s<=%s)%s;", test, &test[p + 1],sDiscardAPI);
							}
							else _sprintf(tmp, "if(%s<%s)%s;", test, &test[p + 1],sDiscardAPI);
						}
						memcpy(&buffer_tmp[size], tmp, (int)strlen(tmp));
						size+=(int)strlen(tmp);
						buffer_tmp[size]='\n';
						size++;
						ie=NULL;
					}
					else
					if (ie->BREAK)
					{
//						if (ie->tag == 0) _sprintf(tmp, "if(%s.x<0.5)break;", RIF);
//						else _sprintf(tmp, "if(%s.x>0.5)break;", RIF);
						if (str_char(test, '>') != -1)
						{
							p = str_char(test, '>');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s>=%s)break;", test, &test[p + 1]);
							}
							else _sprintf(tmp, "if(%s>%s)break;", test, &test[p + 1]);
						}
						else
						{
							p = str_char(test, '<');
							test[p] = '\0';
							if (test[p + 1] == '=')
							{
								p++;
								_sprintf(tmp, "if(%s<=%s)break;", test, &test[p + 1]);
							}
							else _sprintf(tmp, "if(%s<%s)break;", test, &test[p + 1]);
						}

						memcpy(&buffer_tmp[size],tmp,(int)strlen(tmp));
						size+=(int)strlen(tmp);
						buffer_tmp[size]='\n';
						size++;
						ie=NULL;
					}
					else
					{
						if (shadermodel3)
						{
							if (str_char(test, '>') != -1)
							{
								p=str_char(test,'>');
								test[p]='\0';
								if (test[p + 1] == '=')
								{
									p++;
									_sprintf(tmp, "#if(%s>=%s)", test, &test[p + 1]);
								}
								else _sprintf(tmp, "#if(%s>%s)", test, &test[p + 1]);
								memcpy(&buffer_tmp[size],tmp,strlen(tmp));
								size+=(int)strlen(tmp);
								buffer_tmp[size]='\n';
								size++;

								strcpy(tmp,ie->IF);
								memcpy(&buffer_tmp[size],tmp,strlen(tmp));
								size+=(int)strlen(tmp);
								buffer_tmp[size]='\n';
								size++;

								if (ie->ELSE)
								{
									_sprintf(tmp, "#else");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;

									strcpy(tmp,ie->ELSE);
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;

									_sprintf(tmp, "#endif");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;
								}
								else
								{
									_sprintf(tmp, "#endif");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;
								}
							}
							else
							{
								p=str_char(test,'<');
								test[p]='\0';
								if (test[p + 1] == '=')
								{
									p++;
									_sprintf(tmp, "#if(%s<=%s)", test, &test[p + 1]);
								}
								else _sprintf(tmp, "#if(%s<%s)", test, &test[p + 1]);
								memcpy(&buffer_tmp[size],tmp,strlen(tmp));
								size+=(int)strlen(tmp);
								buffer_tmp[size]='\n';
								size++;

								strcpy(tmp,ie->IF);
								memcpy(&buffer_tmp[size],tmp,strlen(tmp));
								size+=(int)strlen(tmp);
								buffer_tmp[size]='\n';
								size++;

								if (ie->ELSE)
								{
									_sprintf(tmp, "#else");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;

									strcpy(tmp,ie->ELSE);
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;

									_sprintf(tmp, "#endif");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;
								}
								else
								{
									_sprintf(tmp, "#endif");
									memcpy(&buffer_tmp[size],tmp,strlen(tmp));
									size+=(int)strlen(tmp);
									buffer_tmp[size]='\n';
									size++;
								}
							}

							buffer_tmp[size]=0;
						}
						else
						{
							char res[1024];
							char res2[1024];

							int sc = str_last_char(c.str, '.');
							if (sc != -1)
							{
								switch ((int)strlen(&c.str[sc + 1]))
								{
								case 1:
									_sprintf(res, "%s.z", RIF);
									_sprintf(res2, "%s.w", RIF);
									break;
								case 2:
									_sprintf(res, "%s.xy", RIFTMP);
									_sprintf(res2, "%s.zw", RIFTMP);
									break;
								case 3:
									_sprintf(res, "%s.xyz", RIFTMP);
									_sprintf(res2, "%s.xyz", RIFTMP2);
									break;
								case 4:
									_sprintf(res, "%s", RIFTMP);
									_sprintf(res2, "%s", RIFTMP2);
									break;
								};
							}
							else
							{
								_sprintf(res, "%s", RIFTMP);
								_sprintf(res2, "%s", RIFTMP2);
							}

							if (ie->ELSE)
							{
								//_sprintf(tmp, "%s=(%s)*%s.x+(%s)*%s.y", c.str, ie->IF, RIF, ie->ELSE, RIF);
								_sprintf(tmp1, "%s=(%s)*%s.x\n", res, ie->IF, RIF);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
								_sprintf(tmp1, "%s=(%s)*%s.y\n", res2, ie->ELSE, RIF);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
								_sprintf(tmp1, "%s=%s+%s\n", c.str, res, res2);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
							}
							else
							{
								//_sprintf(tmp1, "%s=(%s)*%s.x+(%s)*%s.y\n", res, ie->IF, RIF, c.str, RIF);
								_sprintf(tmp1, "%s=(%s)*%s.x\n", res, ie->IF, RIF);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
								_sprintf(tmp1, "%s=(%s)*%s.y\n", res2, c.str, RIF);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
								_sprintf(tmp1, "%s=%s+%s\n", c.str, res, res2);
								memcpy(&buffer_tmp[size], tmp1, (int)strlen(tmp1)); size += (int)strlen(tmp1);
							}
						}
						ie=cnv.GetNext();
					}
				}
				cnv.Free();
			}
		}
	}

	buffer=(char*) malloc(size+1);
	memcpy(buffer,buffer_tmp,size);
	buffer[size]='\0';

	free(buffer_tmp);
	free(script);
	return buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::str_parse_addsub(char * script)
{
	char *tmp;
	char *buffer, *buffer_tmp;
	bool b = false;
	char res[1024];
	char tmpnew[1024];
	char body[1024];
	int size;
	int p;
	int apos, pos;

	buffer_tmp = (char*)malloc(65536*2);
	size = 0;
    
	pos = 0;
	while (!b)
	{
		apos = pos;
		tmp = str_parse_virg(script, &pos);
		if (!tmp) b = true;
		else
		{
			if ((!str_match(tmp, "+=")) && (!str_match(tmp, "-=")) && (!str_match(tmp, "*=")) && (!str_match(tmp, "/=")))
			{
				memcpy(&buffer_tmp[size], tmp, strlen(tmp));
				size += (int)strlen(tmp);
				buffer_tmp[size] = ';';
				size++;
			}
			else
			{
                if (str_match(tmp, "if("))
                {
                    int n=0;
                    int lvl=0;
                    while ((!((tmp[n]==')')&&(lvl==1)))&&(n<(int)strlen(tmp)))
                    {
                        if (tmp[n]=='(') lvl++;
                        if (tmp[n]==')') lvl--;
                        n++;
                    }
                    
                    if (n<(int)strlen(tmp))
                    {
                        n++;
                        if (tmp[n]=='{') n++;
                        memcpy(&buffer_tmp[size], tmp, n);
                        size += n;
                        tmp=&tmp[n];
                    }
                    else IFERROR=true;
                }

                if (str_match(tmp, "}else{"))
                {
                    int n=6;
                    memcpy(&buffer_tmp[size], tmp, n);
                    size += n;
                    tmp=&tmp[n];
                }
                else
                if (str_match(tmp, "}else"))
                {
                    int n=5;
                    memcpy(&buffer_tmp[size], tmp, n);
                    size += n;
                    tmp=&tmp[n];
                }
                else
                if (str_match(tmp, "else{"))
                {
                    int n=5;
                    memcpy(&buffer_tmp[size], tmp, n);
                    size += n;
                    tmp=&tmp[n];
                }
                else
                if (str_match(tmp, "else"))
                {
                    int n=4;
                    memcpy(&buffer_tmp[size], tmp, n);
                    size += n;
                    tmp=&tmp[n];
                }
                
                if (str_match(tmp, "*="))
                {
                    _sprintf(res, "%s", tmp);
                    str_clean(res);
                    
                    p = str_char(res, '*');
                    res[p] = '\0';
                    
                    _sprintf(body, &res[p + 2]);
                    
                    int tag=0;
                    if (str_char(body,'+')>=0) tag=1;
                    if (str_char(body,'-')>=0) tag=1;
                    if (str_char(body,'*')>=0) tag=1;
                    if (str_char(body,'/')>=0) tag=1;
                    if (tag) _sprintf(tmpnew, "%s=%s*(%s);", res, res, body);
                    else _sprintf(tmpnew, "%s=%s*%s;", res, res, body);
                    
                    memcpy(&buffer_tmp[size], tmpnew, strlen(tmpnew));
                    size += (int)strlen(tmpnew);
                    //buffer_tmp[size] = ';';
                    //size++;
                }
                else
                if (str_match(tmp, "/="))
                {
                    _sprintf(res, "%s", tmp);
                    str_clean(res);
                    
                    p = str_char(res, '/');
                    res[p] = '\0';
                    
                    _sprintf(body, &res[p + 2]);
                    
                    _sprintf(tmpnew, "%s=%s/%s;", res, res, body);
                    
                    memcpy(&buffer_tmp[size], tmpnew, strlen(tmpnew));
                    size += (int)strlen(tmpnew);
                    //buffer_tmp[size] = ';';
                    //size++;
                }
                else
				if (str_match(tmp, "+="))
				{
					_sprintf(res, "%s", tmp);
					str_clean(res);

					p = str_char(res, '+');
					res[p] = '\0';

					_sprintf(body, &res[p + 2]);

					_sprintf(tmpnew, "%s=%s+%s;", res, res, body);

					memcpy(&buffer_tmp[size], tmpnew, strlen(tmpnew));
					size += (int)strlen(tmpnew);
					//buffer_tmp[size] = ';';
					//size++;
				}
				else
				if (str_match(tmp, "-="))
				{
					_sprintf(res, "%s", tmp);
					str_clean(res);

					p = str_char(res, '-');
					res[p] = '\0';

					_sprintf(body, &res[p + 2]);

					_sprintf(tmpnew, "%s=%s-%s;", res, res, body);

					memcpy(&buffer_tmp[size], tmpnew, strlen(tmpnew));
					size += (int)strlen(tmpnew);
					//buffer_tmp[size] = ';';
					//size++;
				}
			}
		}
	}

	buffer = (char*)malloc(size + 1);
	memcpy(buffer, buffer_tmp, size);
	buffer[size] = '\0';
	free(buffer_tmp);

	return buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char getfnname_str[1024];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * getfnname(char *script,int pos)
{
	char ss[4096];
	int p=pos;
	while ((script[p]!=')')&&(p>0)) p--;
	if (p>0)
	{
		int pp=p+1;
		while ((script[p]!='(')&&(p>0)) p--;
		while ((script[p]!=' ')&&(script[p]!='\t')&&(script[p]!='\n')&&(p>0)) p--;
		if (pp>p)
		{
			memcpy(ss,&script[p+1],pp-p);
			ss[pp-p]='\0';
			str_clean(ss);
			strcpy(getfnname_str,ss);
			return getfnname_str;
		}
		else return "<Unknown space-time>";
	}
	else return "<Unknown space-time>";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::str_get_scriptfns(char * script, int *pos)
{
	char * buffer_tmp;
	char * buffer,* temp;
	int size;
	char *tmp;
	int apos;
	int level=0;

	buffer_tmp = (char*)malloc(65536*2);
	size=0;

	apos=*pos;
	tmp=str_parse_virgkeep(script,pos);
	if (str_char(tmp,'{')>=0) strcpy(ERRORSTR,getfnname(script,apos));
	while (tmp)
	{
		if (str_char(tmp,'{')>=0) level++;
		if (str_char(tmp,'}')>=0) level--;
		if ((str_char(tmp,'}')>=0)&&(level==0)) { if (str_char(tmp,';')==-1) return NULL; }
		str_clean(tmp);
		if (tmp[0]!='[')
		{
			memcpy(&buffer_tmp[size],tmp,strlen(tmp));
			size+=(int)strlen(tmp);
			apos=*pos;
			char *rrr=str_parse_virgkeep(script,pos);
			if (rrr) { if ((str_char(rrr,'{')>=0)&&(level==0)) strcpy(ERRORSTR,getfnname(script,apos)); }
			tmp=rrr;
		}
		else tmp=NULL;
	}

	*pos=apos;

	buffer=(char*) malloc(size+1);
	memcpy(buffer,buffer_tmp,size);
	buffer[size]='\0';
	free(buffer_tmp);

	temp = str_parse_addsub(buffer);
	free(buffer);
	buffer = str_if_else_virg_treatment(temp);
	free(temp);

	return buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::str_get_script(char * script, int *pos)
{
	char * buffer_tmp;
	char * buffer,* temp;
	int size;
	char *tmp;
	int apos;

	buffer_tmp = (char*)malloc(65536*2);
	size=0;

	apos=*pos;
	tmp=str_parse_virg(script,pos);
	while (tmp)
	{
		str_clean(tmp);

		if (tmp[0]!='[')
		{
			memcpy(&buffer_tmp[size],tmp,strlen(tmp));
			size+=(int)strlen(tmp);
			buffer_tmp[size] = ';';
			size++;

			apos=*pos;
			tmp=str_parse_virg(script,pos);
		}
		else tmp=NULL;
	}

	*pos=apos;

	buffer=(char*) malloc(size+1);
	memcpy(buffer,buffer_tmp,size);
	buffer[size]='\0';
	free(buffer_tmp);

	temp = str_parse_addsub(buffer);
	free(buffer);
	buffer = str_if_else_virg_treatment(temp);
	free(temp);

	return buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVertexProgram::str_get_scriptm(char * script, int *pos)
{
	char * buffer_tmp;
	char * buffer;
	int size;
	char *tmp;
	int apos;

	buffer_tmp=(char*) malloc(65536*2);
	size=0;

	apos=*pos;
	tmp=str_parse_char(script,pos,'\n');
	//tmp=str_parse_virg(script,pos);
	while (tmp)
	{
		str_clean(tmp);
		if (tmp[0]!='[')
		{
			if (strlen(tmp)>0)
			{
				memcpy(&buffer_tmp[size],tmp,strlen(tmp));
				size+=(int)strlen(tmp);
				/*
				buffer_tmp[size]=';';
				size++;
				/**/
			}

			apos=*pos;

			tmp=str_parse_char(script,pos,'\n');
		}
		else tmp=NULL;
	}

	*pos=apos;

	buffer=(char*) malloc(size+1);
	memcpy(buffer,buffer_tmp,size);
	buffer[size]='\0';
	free(buffer_tmp);

	char *temp = str_if_else_virg_treatment(buffer);
	free(buffer);

	return temp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	TECH_DOT3			=	1,
	TECH_MULTITEXTURE	=	2,
	TECH_PIXELSHADER	=	4,
	TECH_SHADERMODEL	=	8,
	TECH_SHADERMODEL3	=	16
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::Translate_Macros(char * macros)
{
	Macro m;
	int n,level;
	int p,pp,len;
	int nn;
	char str[1024];
	char *str2;

	n=0;
	p=0;
	level=0;
	while (n<(int) strlen(macros))
	{
		if (macros[n]=='{')
		{
			if (level==0)
			{
				len=n-p;
				memcpy(str,&macros[p],len);
				str[len]='\0';

				pp=str_char(str,'(');
				str[pp]='\0';
				str2=&str[pp+1];
				str2[str_char(str2,')')]='\0';
				_sprintf(m.name,"%s",str);
				_sprintf(m.def,"%s",str2);
				level++;
				p=n+1;
			}
			else level++;
		}

		if (macros[n]=='}')
		{
			level--;
			if (level==0)
			{
				len=n-p;
				memcpy(m.code,&macros[p],len);
				m.code[len]='\0';
				for (nn=0;nn<len;nn++)
					if (m.code[nn]==';') m.code[nn]='\n';

				ListMacros.Add(m);
			}
		}
		n++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::vsfnnum(char *name)
{
	int n;
	char str[1024];
	int p=str_char(name,'(');
	if (p>=0)
	{
		_sprintf(str,name);
		str[p]='\0';
		for (n=0;n<24;n++) if (strcmp(str,vsfn[n].name)==0) return n;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CVertexProgram::psfnnum(char *name)
{
	int n;
	char str[1024];
	int p=str_char(name,'(');
	if (p>=0)
	{
		_sprintf(str,name);
		str[p]='\0';
		for (n=0;n<24;n++) if (strcmp(str,psfn[n].name)==0) return n;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::modifiersfn(Macro *fn,char *call,CList <Code> *vp)
{
	char remap[1024];
	char tmp[1024];
	char *rn;
	char *ss;
	char *def=fn->def;
	int k=0;

	_sprintf(remap,"");

	int p=0;
	int pp=0;
	ss=str_parse_char(def,&p,',');
	while (ss)
	{
		char *n=str_parse_char_prt(call,&pp,',');
		if (n)
		{
			compile_expression=true;
			if (str_simple(n))
			{
				if (isdefined(n)<0)
				{ 
					_sprintf(ERRORSTRPARAM,"unknown : \"%s\"",n);
					PARAMERROR=true;
					return; 
				}
				else rn=var(n);
			}
			else
			{
				_sprintf(tmp,"r%d",new_temp_register());
				rn=compile(n,tmp,vp);
			}
			compile_expression=false;

			if (k>0) _sprintf(remap,"%s,",remap);
			_sprintf(remap,"%s%s",remap,rn);
		}
		else
		{
			_sprintf(ERRORSTRPARAM,"forgetten : \"%s\"",ss);
			PARAMERROR=true;
			return;
		}
		k++;
		ss=str_parse_char(def,&p,',');
	}
	
	_sprintf(call,remap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::affectparameters(char *call,Macro *fn,CList <Code> *vp)
{	
	char str[512];
	char reg[512];
	char *def=fn->def;
	int k=0;

	fn->floats=0;

	int p=str_char(call,'(');
	if (p>=0)
	{		
		_sprintf(str,&call[p+1]);
		int pp=str_char(str,')');
		str[pp]='\0';
		p=pp=0;
		char *s=str_parse_char(def,&p,',');
		while (s)
		{
			char *n=str_parse_char_prt(str,&pp,',');
			int r=new_temp_register();
			_sprintf(reg,"r%d",r);
				
			fn->registers[k]=r;

			if (str_char(str,'.')==-1)
			{
				if (strcmp(reg,var(n))!=0)
				{
					if (fn->defs[k]==CALL_FLOAT) _sprintf(c.str,"mov %s.xyzw,%s.x",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR2) _sprintf(c.str,"mov %s.xy,%s.xy",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR3) _sprintf(c.str,"mov %s.xyz,%s.xyz",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR4) _sprintf(c.str,"mov %s,%s",reg,var(n));
					AddInst(vp,c);
				}
			}
			else
			{
				if (strcmp(reg,var(n))!=0)
				{					
					if (fn->defs[k]==CALL_FLOAT) _sprintf(c.str,"mov %s.xyzw,%s",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR2) _sprintf(c.str,"mov %s.xy,%s",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR3) _sprintf(c.str,"mov %s.xyz,%s",reg,var(n));
					if (fn->defs[k]==CALL_VECTOR4) _sprintf(c.str,"mov %s,%s",reg,var(n));
					AddInst(vp,c);
				}
			}				
			k++;
			s=str_parse_char(def,&p,',');
		}	
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int startname(char *s)
{
	int n=0;
	if (str_match(s,"float")) n=5;
	if (str_match(s,"float2")) n=6;
	if (str_match(s,"float3")) n=6;
	if (str_match(s,"float4")) n=6;
	if (str_match(s,"vector2")) n=7;
	if (str_match(s,"vector3")) n=7;
	if (str_match(s,"vector4")) n=7;
	if (str_match(s,"vec2")) n=4;
	if (str_match(s,"vec3")) n=4;
	if (str_match(s,"vec4")) n=4;
	if (str_match(s,"iofloat")) n=7;
	if (str_match(s,"iofloat2")) n=8;
	if (str_match(s,"iofloat3")) n=8;
	if (str_match(s,"iofloat4")) n=8;
	if (str_match(s,"iovector2")) n=9;
	if (str_match(s,"iovector3")) n=9;
	if (str_match(s,"iovector4")) n=9;
	if (str_match(s,"iovec2")) n=6;
	if (str_match(s,"iovec3")) n=6;
	if (str_match(s,"iovec4")) n=6;

	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVP_CnvDef(Macro *fn,char *def)
{
	int n=0;
	char str[1024];
	int p=0;
	_sprintf(str,"");
	char *s=str_parse_char(def,&p,',');
	while (s)
	{
		if (str_match(s,"float")) fn->defs[n]=CALL_FLOAT;
		if (str_match(s,"vector2")) fn->defs[n]=CALL_VECTOR2;
		if (str_match(s,"vector3")) fn->defs[n]=CALL_VECTOR3;
		if (str_match(s,"vector4")) fn->defs[n]=CALL_VECTOR4;
		if (str_match(s,"float2")) fn->defs[n]=CALL_VECTOR2;
		if (str_match(s,"float3")) fn->defs[n]=CALL_VECTOR3;
		if (str_match(s,"float4")) fn->defs[n]=CALL_VECTOR4;
		if (str_match(s,"vec2")) fn->defs[n]=CALL_VECTOR2;
		if (str_match(s,"vec3")) fn->defs[n]=CALL_VECTOR3;
		if (str_match(s,"vec4")) fn->defs[n]=CALL_VECTOR4;

		if (str_match(s,"iofloat")) fn->defs[n]=CALL_IOFLOAT;
		if (str_match(s,"iofloat2")) fn->defs[n]=CALL_IOVECTOR2;
		if (str_match(s,"iofloat3")) fn->defs[n]=CALL_IOVECTOR3;
		if (str_match(s,"iofloat4")) fn->defs[n]=CALL_IOVECTOR4;
		if (str_match(s,"iovector2")) fn->defs[n]=CALL_IOVECTOR2;
		if (str_match(s,"iovector3")) fn->defs[n]=CALL_IOVECTOR3;
		if (str_match(s,"iovector4")) fn->defs[n]=CALL_IOVECTOR4;
		if (str_match(s,"iovec2")) fn->defs[n]=CALL_IOVECTOR2;
		if (str_match(s,"iovec3")) fn->defs[n]=CALL_IOVECTOR3;
		if (str_match(s,"iovec4")) fn->defs[n]=CALL_IOVECTOR4;

		if (n>0) _sprintf(str,"%s,%s",str,&s[startname(s)]); else _sprintf(str,"%s",&s[startname(s)]);
		n++;
		s=str_parse_char(def,&p,',');
	}

	_sprintf(def,str);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVP_CnvRet(Macro *fn,char *s)
{
	char str[1024];

	fn->ret=CALL_VECTOR4;
	if (strlen(s)>0)
	{
		if (str_match(s,"float")) fn->ret=CALL_FLOAT;
		if (str_match(s,"vector2")) fn->ret=CALL_VECTOR2;
		if (str_match(s,"vector3")) fn->ret=CALL_VECTOR3;
		if (str_match(s,"float2")) fn->ret=CALL_VECTOR2;
		if (str_match(s,"float3")) fn->ret=CALL_VECTOR3;
		if (str_match(s,"vec2")) fn->ret=CALL_VECTOR2;
		if (str_match(s,"vec3")) fn->ret=CALL_VECTOR3;
		_sprintf(str,"%s",&s[startname(s)]);
		_sprintf(s,str);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::parse_script_vs_functions(char *script)
{
	Macro *fn;
	int n,level;
	int p,pp,len;
	int nn;
	char str[1024];
	char *str2;

	if (script==NULL) return;

	n=0;
	p=0;
	level=0;
	while (n<(int) strlen(script))
	{
		if (script[n]=='{')
		{
			if (level==0)
			{
				fn=&vsfn[numvsfn++];
				fn->floats=0;
				len=n-p;
				memcpy(str,&script[p],len);
				str[len]='\0';

				strcpy(fn->entete,str);

				pp=str_char(str,'(');
				str[pp]='\0';
				str2=&str[pp+1];
				str2[str_char(str2,')')]='\0';
				CVP_CnvRet(fn,str);
				_sprintf(fn->name,"%s",str);
				CVP_CnvDef(fn,str2);
				_sprintf(fn->def,"%s",str2);
				level++;
				p=n+1;
			}
			else level++;
		}

		if (script[n]=='}')
		{
			level--;
			if (level==0)
			{
				len=n-p;
				memcpy(fn->code,&script[p],len);
				fn->code[len]='\0';
				for (nn=0;nn<len;nn++)
					if (fn->code[nn]==';') fn->code[nn]='\n';

				p=n+2;
				n++;
			}
		}
		n++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVertexProgram::parse_script_ps_functions(char *script)
{
	Macro *fn;
	int n,level;
	int p,pp,len;
	int nn;
	char str[1024];
	char *str2;

	if (script==NULL) return;

	n=0;
	p=0;
	level=0;
	while (n<(int) strlen(script))
	{
		if (script[n]=='{')
		{
			if (level==0)
			{
				fn=&psfn[numpsfn++];
				fn->floats=0;
				len=n-p;
				memcpy(str,&script[p],len);
				str[len]='\0';

				strcpy(fn->entete,str);

				pp=str_char(str,'(');
				str[pp]='\0';
				str2=&str[pp+1];
				str2[str_char(str2,')')]='\0';
				CVP_CnvRet(fn,str);
				_sprintf(fn->name,"%s",str);
				CVP_CnvDef(fn,str2);
				_sprintf(fn->def,"%s",str2);
				level++;
				p=n+1;
			}
			else level++;
		}

		if (script[n]=='}')
		{
			level--;
			if (level==0)
			{
				len=n-p;
				memcpy(fn->code,&script[p],len);
				fn->code[len]='\0';
				for (nn=0;nn<len;nn++)
					if (fn->code[nn]==';') fn->code[nn]='\n';

				p=n+2; // "};"
				n++;
			}
		}
		n++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool matchouille(char *str,char *mtch)
{
    int len=(int)strlen(mtch);
    if ((int)strlen(str)<len) return false;
    char ss[1024];
    memcpy(ss,str,len);
    ss[len]=0;
    if (strcmp(ss,mtch)==0) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * DeleteCommentsAndWarps(char *str,int warp)
{
	char * script;
	char *buffer=(char*) malloc(65536*2);
	int size=0;
	int n=0;

	while (n<(int)strlen(str))
	{
		if ((str[n]!='/')&&(str[n]!='#'))
		{
			buffer[size++]=str[n++];
		}
		else
		{
			if (str[n]=='/') // comments with "//"
			{
				if (str[n+1]=='/')
				{
					n+=2;
					while ((str[n]!=0xA)&&(str[n]!=0xD)) n++;
					buffer[size++]=str[n++];
				}
				else
				{
					buffer[size++]=str[n++];
				}
			}
			else  // coments with "#"
			{
				n++;
				while ((str[n]!=0xA)&&(str[n]!=0xD)) n++;
				buffer[size++]=str[n++];
			}
		}
	}

	script=(char*) malloc(size+1);
	memcpy(script,buffer,size);
	script[size]='\0';
    
    // warp_normal_pos -1, 0 1 2 NORMAL, MORPH, TREE

    size=0;
    n=0;
    while (n<(int)strlen(script))
    {
        if (matchouille(&script[n],"GetPositionMatrix"))
        {
            int start=n;
            while ((script[n]!=0xA)&&(script[n]!=0xD)) n++;
            n--;
            int end=n;
            char func[1024];
            memcpy(func,&script[start],end-start);
            func[end-start]=0;
            
            char *prm=str_return_parentheses(func);
            if (strlen(prm)==0) return NULL;
            
            int p = str_char_prt(prm, ',');
            if (p<0) return NULL;
            prm[p]=0;
            char *mat=&prm[p+1];
            
            if (warp<=0) _sprintf(func,"%s = iPos *4 %s;\n",prm,mat);
            if (warp==1) _sprintf(func,"%s = interpolate(iPos,iPos2,Interpolant,%s);\n",prm,mat);
            if (warp==2) _sprintf(func,"%s = iWeights.xxxx*iPos*4PALETTE[iWIndices.x] + iWeights.yyyy*iPos*4PALETTE[iWIndices.y];\n",prm);
            
            for (int k=0;k<(int)strlen(func);k++) buffer[size++]=func[k];
            n++;
        }
        else
        if (matchouille(&script[n],"GetNormalMatrix"))
        {
            int start=n;
            while ((script[n]!=0xA)&&(script[n]!=0xD)) n++;
            n--;
            int end=n;
            char func[1024];
            memcpy(func,&script[start],end-start);
            func[end-start]=0;
            
            char *prm=str_return_parentheses(func);
            if (strlen(prm)==0) return NULL;
            
            int p = str_char_prt(prm, ',');
            if (p<0) return NULL;
            prm[p]=0;
            char *mat=&prm[p+1];

            if (warp<=0)
                _sprintf(func,"%s = iNorm *3 %s;\n",prm,mat);
            if (warp==1)
                _sprintf(func,"%s = normal_interpolate(iNorm,iNorm2,Interpolant,%s);\n\t%s = normalise(%s);\n",prm,mat,prm,prm);
            if (warp==2)
                _sprintf(func,"%s = iWeights.xxxx*iNorm*3PALETTE[iWIndices.x]  + iWeights.yyyy*iNorm*3PALETTE[iWIndices.y];\n\t%s = normalise(%s);\n",prm,prm,prm);
            
            for (int k=0;k<(int)strlen(func);k++) buffer[size++]=func[k];
            n++;
        }
        else
        if (matchouille(&script[n],"GetPosition"))
        {
            int start=n;
            while ((script[n]!=0xA)&&(script[n]!=0xD)) n++;
            n--;
            int end=n;
            char func[1024];
            memcpy(func,&script[start],end-start);
            func[end-start]=0;
            
            char *prm=str_return_parentheses(func);
            if (strlen(prm)==0) return NULL;
            
            if (warp<=0) _sprintf(func,"%s.xyz = iPos.xyz;\n\t%s.w=1.0;\n",prm,prm);
            if (warp==1) _sprintf(func,"%s = interpolate_base(iPos,iPos2,Interpolant);\n",prm);
            if (warp==2) _sprintf(func,"%s = iWeights.xxxx*iPos*4PALETTE[iWIndices.x] + iWeights.yyyy*iPos*4PALETTE[iWIndices.y];\n",prm);
            
            for (int k=0;k<(int)strlen(func);k++) buffer[size++]=func[k];
            n++;
        }
        else
        if (matchouille(&script[n],"GetNormal"))
        {
            int start=n;
            while ((script[n]!=0xA)&&(script[n]!=0xD)) n++;
            n--;
            int end=n;
            char func[1024];
            memcpy(func,&script[start],end-start);
            func[end-start]=0;
            
            char *prm=str_return_parentheses(func);
            if (strlen(prm)==0) return NULL;
            
            if (warp<=0)
                _sprintf(func,"%s.xyz = iNorm.xyz;\n\t%s.w=0.0;\n",prm,prm);
            if (warp==1)
                _sprintf(func,"%s = normal_interpolate_base(iNorm,iNorm2,Interpolant);\n\t%s = normalise(%s);\n",prm,prm,prm);
            if (warp==2)
                _sprintf(func,"%s = iWeights.xxxx*iNorm*3PALETTE[iWIndices.x]  + iWeights.yyyy*iNorm*3PALETTE[iWIndices.y];\n\t%s = normalise(%s);\n",prm,prm,prm);
            
            for (int k=0;k<(int)strlen(func);k++) buffer[size++]=func[k];
            n++;
        }
        else
        {
            buffer[size++]=script[n++];
        }
    }
    
    free(script);
    
    script=(char*) malloc(size+1);
    memcpy(script,buffer,size);
    script[size]='\0';
    
    free(buffer);
    
	return script;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::IsRegisterIn(int reg,CList<Code> &vpcode)
{
    char ss[128];
    char ssreg[128];
    _sprintf(ssreg,"r%d",reg);
    
    Code * cc=vpcode.GetFirst();
    while (cc)
    {
        int pc=str_char(cc->str,'=');
        int pc2=str_char(cc->str,'.');
        if ((pc>=0)&&((pc2==-1)||(pc2>pc)))
        {
            memcpy(ss,cc->str,pc);
            ss[pc]=0;
            str_clean(ss);
            if (strcmp(ss,ssreg)==0) return true;
            else cc=vpcode.GetNext();
        }
        else
        {
            if ((pc2>=0)&&(pc>pc2))
            {
                memcpy(ss,cc->str,pc2);
                ss[pc2]=0;
                str_clean(ss);
                if (strcmp(ss,ssreg)==0) return true;
                else cc=vpcode.GetNext();
            }
            else
            {
                cc=vpcode.GetNext();
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVertexProgram::read_shader(char *script0,unsigned int flags)
{
	char *str_chaine;
	int pos=0;
	int n,t,p,nb;
	int apos;
	char *tmp;
	static char chaine[1024];
	char *str;
	int tech,ntech;
	int tech_spefs[4];
	unsigned int fvf,output;
	float *vect;
	float *mat;
	int pass=0;
	int npass_max[4]={0,0,0,0};
	bool found;
	char * script_position[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_diffuse[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_specular[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping0[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping1[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping2[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping3[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping4[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping5[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping6[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_mapping7[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_shader[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_vsfunctions[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * script_psfunctions[4][4]={{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL}};
	char * macros=NULL;
	int nm,nv;
	bool success=true;
	int d3d9=0;
	bool model3=false;
	bool var3=false;
    bool defvp=false;
	char * script=DeleteCommentsAndWarps(script0,warp_normal_pos);

	if (strlen(valueFSRRCASSample)==0) FSRCASSampleSharp(666);

	if (strlen(value_bumpy_str)==0) SampleBumpy(666);

    if (script==NULL)
    {
        SYNTAXERROR=true;
        _sprintf(ERRORSTR,"Illegal call of function.");
        return false;
    }
    
	strcpy(sDiscardAPI,"discard");

	shadermodel=false;
	shadermodel3=false;
	flow=1;

	_sprintf(RIF, "r11");
	_sprintf(RIFTMP, "r10");
	_sprintf(RIFTMP2, "r9");

	metal=0;

	if (flags&DIRECT3D) api=0;
	if (flags&D3D9) d3d9=1;
	if (flags&OPENGL) api=1;
	if (flags&METAL) { api=0; d3d9=1; shadermodel4=true; metal=1; }
	if (flags&EMULATED) api=2;

	if (metal==1) strcpy(sDiscardAPI,"discard_fragment()");

	tech=0;
	ntech=0;
	last_ps_constant=0;

	if (flags&D3D10) shadermodel4=true;
	if (flags&D3D10) d3d9=1;
	if (flags&D3D11) shadermodel4=true;
	if (flags&D3D11) d3d9=1;

	if (shadermodel4)
	{
		shadermodel=shadermodel3=var3=true;
		_sprintf(RIF, "r31");
		_sprintf(RIFTMP, "r30");
		_sprintf(RIFTMP2, "r29");

		tmp=str_parse(script,&pos);
		while (tmp)
		{
			if (tmp[0]=='[')
			{
				if (strcmp(tmp,"[TECHNIQUE]")==0)
				{
					apos=pos;
					tmp=str_parse(script,&pos);
					while (tmp)
					{
						str_clean(tmp);
						str_upcase(tmp);
						if (tmp[0] != '[')
						{
							if (strcmp(tmp,"DEPTH")==0) modifyZ=1;
							apos=pos;
							tmp=str_parse(script,&pos);
						}
						else tmp=NULL;
					}
					pos=apos;
				}
			}
			tmp=str_parse(script,&pos);
		}
	}
	else
	{
		tmp=str_parse(script,&pos);
		while (tmp)
		{
			if (tmp[0]=='[')
			{
				if (strcmp(tmp,"[TECHNIQUE]")==0)
				{
					apos=pos;
					tmp=str_parse(script,&pos);
					while (tmp)
					{
						str_clean(tmp);
						str_upcase(tmp);
						if (tmp[0] != '[')
						{
                            if (strcmp(tmp,"POINTS")==0) pointcloud=1;

							if (strcmp(tmp,"TILED")==0) SizingTexture0=1;
                            
							if (strcmp(tmp,"MODEL3")==0) { shadermodel3=var3=true;_sprintf(RIF, "r31");_sprintf(RIFTMP, "r30");_sprintf(RIFTMP2, "r29");}
	#ifdef FORCE_MODEL3
							if (strcmp(tmp,"MODEL2X")==0) shadermodel3=var3=true;
	#endif
							if (strcmp(tmp, "MODEL3NOFLOW") == 0) { shadermodel3 = var3 = true; flow = 0; _sprintf(RIF, "r31"); _sprintf(RIFTMP, "r30"); _sprintf(RIFTMP2, "r29"); }
							if (strcmp(tmp,"DEPTH")==0) modifyZ=1;
                            if (strcmp(tmp,"VERSION300")==0) version300=1;
                            
							apos=pos;
							tmp=str_parse(script,&pos);
						}
						else tmp=NULL;
					}
					pos=apos;
				}
			}
			tmp=str_parse(script,&pos);
		}
	}

#ifdef FORCE_MODEL3
	shadermodel=true;
	model3=true;
	shadermodel3=var3=true;_sprintf(RIF, "r31");_sprintf(RIFTMP, "r30");_sprintf(RIFTMP2, "r29");
#endif

	apos=pos=0;
	tmp=str_parse(script,&pos);
	while (tmp)
	{
		if (ntech==0) tech=0;
		else tech=ntech-1;

		if (pass+1>npass_max[tech]) npass_max[tech]=pass+1;
		str_clean(tmp);
		str_upcase(tmp);

		if (tmp[0]=='[')
		{
			if (strcmp(tmp,"[TECHNIQUE]")==0)
			{
				ntech++;
				if (ntech==0) tech=0; else tech=ntech-1;

				tech_spefs[tech]=0;

				apos=pos;
				tmp=str_parse(script,&pos);
				while (tmp)
				{
					str_clean(tmp);
					str_upcase(tmp);
					if (tmp[0]!='[')
					{
						if (strcmp(tmp,"POINTS")==0) pointcloud=1;

						if (strcmp(tmp,"TILED")==0) SizingTexture0=1;
                        
						if (strcmp(tmp,"FSR")==0) { isFSR=true; shadermodel3=var3=true; tech_spefs[tech]|=TECH_SHADERMODEL|TECH_SHADERMODEL3; }

						if ((strcmp(tmp,"MULTITEXTURE")==0)&&(!(tech_spefs[tech]&TECH_MULTITEXTURE))) tech_spefs[tech]|=TECH_MULTITEXTURE;
						if ((strcmp(tmp,"DOT3")==0)&&(!(tech_spefs[tech]&TECH_DOT3))) tech_spefs[tech]|=TECH_DOT3;
						if ((strcmp(tmp,"SHADERS")==0)&&(!(tech_spefs[tech]&TECH_PIXELSHADER))) tech_spefs[tech]|=TECH_PIXELSHADER;
#ifdef FORCE_MODEL3
						if ((strcmp(tmp,"MODEL2X")==0)&&(!(tech_spefs[tech]&TECH_SHADERMODEL))) tech_spefs[tech]|=TECH_SHADERMODEL|TECH_SHADERMODEL3;
#else
						if ((strcmp(tmp,"MODEL2X")==0)&&(!(tech_spefs[tech]&TECH_SHADERMODEL))) tech_spefs[tech]|=TECH_SHADERMODEL;
#endif
						if ((strcmp(tmp,"MODEL3")==0)&&(!(tech_spefs[tech]&TECH_SHADERMODEL))) tech_spefs[tech]|=TECH_SHADERMODEL|TECH_SHADERMODEL3;
						if ((strcmp(tmp,"MODEL3NOFLOW")==0)&&(!(tech_spefs[tech]&TECH_SHADERMODEL))) tech_spefs[tech]|=TECH_SHADERMODEL|TECH_SHADERMODEL3;
						apos=pos;
						tmp=str_parse(script,&pos);
					}
					else tmp=NULL;
				}
				pos=apos;

#if defined(API3D_OPENGL20)
				if (api==1)
				{
					tech_spefs[tech]=TECH_SHADERMODEL|TECH_SHADERMODEL3;
					shadermodel=shadermodel3=var3=true;
					_sprintf(RIF, "r31");_sprintf(RIFTMP, "r30");_sprintf(RIFTMP2, "r29");
				}
#endif

				if (shadermodel4)
				{
					tech_spefs[tech]|=TECH_SHADERMODEL|TECH_SHADERMODEL3;
				}
			}
			else
			if (strcmp(tmp,"[PASS0]")==0)
			{
				pass=0;
			}
			else
			if (strcmp(tmp,"[PASS1]")==0)
			{
				pass=1;
			}
			else
			if (strcmp(tmp,"[PASS2]")==0)
			{
				pass=2;
			}
			else
			if (strcmp(tmp,"[PASS3]")==0)
			{
				pass=3;
			}
			else
			if (strcmp(tmp,"[VERTEXFORMAT]")==0)
			{
				fvf=0;
				apos=pos;
				tmp=str_parse(script,&pos);
				while (tmp)
				{
					str_clean(tmp);
					str_upcase(tmp);
					if (tmp[0]!='[')
					{
						if ((strcmp(tmp,"XYZ")==0)&&(!(fvf&XYZ))) fvf|=XYZ;
						if ((strcmp(tmp,"NORMAL")==0)&&(!(fvf&NORMAL))) fvf|=NORMAL;
						if ((strcmp(tmp,"DIFFUSE")==0)&&(!(fvf&DIFFUSE))) fvf|=DIFFUSE;
						if ((strcmp(tmp,"SPECULAR")==0)&&(!(fvf&SPECULAR))) fvf|=SPECULAR;
						if ((strcmp(tmp,"TEX0")==0)&&(!(fvf&_TEX0))) fvf|=_TEX0;
						if ((strcmp(tmp,"TEX1")==0)&&(!(fvf&_TEX1))) fvf|=_TEX1;
						if ((strcmp(tmp,"TEX2")==0)&&(!(fvf&_TEX2))) fvf|=_TEX2;

						if ((strcmp(tmp,"IMPOSTOR")==0)&&(!(fvf&_TEX1))) fvf|=_TEX1;
						if ((strcmp(tmp,"IMPOSTOR")==0)&&(!(fvf&_TEX2))) fvf|=_TEX2;						

						if ((strcmp(tmp,"TEX3")==0)&&(!(fvf&_TEX3))) fvf|=_TEX3;
						if ((strcmp(tmp,"BLEND")==0)&&(!(fvf&BLEND))) { fvf|=BLEND; weightsandindices=true; }
						if ((strcmp(tmp,"MORPH")==0)&&(!(fvf&VPMORPH))) { fvf|=VPMORPH; stream=true; }

						apos=pos;
						tmp=str_parse(script,&pos);
					}
					else tmp=NULL;
				}
                defvp=true;
                fvf_sm=fvf;
				pos=apos;
			}
			else
			if (strcmp(tmp,"[DEFINES]")==0)
			{
                if (!defvp) fvf_sm=fvf=XYZ|NORMAL|_TEX0;
                
				apos=pos;
				tmp=str_parse(script,&pos);
				while (tmp)
				{
					str_upcase(tmp);
					if (tmp[0]!='[')
					{
						if ((strcmp(tmp,"CONST")==0)||(strcmp(tmp,"PSCONST")==0)||(strcmp(tmp,"PSMATRIX")==0))
						{
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							if (str_char(tmp,'[')==-1)
							{
								int pp=str_char(tmp,'=');
								if (pp>=0)
								{
									char ss[1024];
									strcpy(ss,&tmp[pp+1]);
									for (int pn=0;pn<(int)strlen(ss);pn++)
									{
										if (!((ss[pn]==',')||(ss[pn]=='.')||(ss[pn]=='-')||((ss[pn]>='0')&&(ss[pn]<='9'))))
										{
											SYNTAXERROR=true;
											strcpy(ERRORSTR,tmp);
											return false;
										}
									}

									strcpy(ss,tmp);
									ss[pp]='\0';
									for (int pn=0;pn<(int)strlen(ss);pn++)
									{
										if (!((ss[pn]=='_')||(((ss[pn]>='A')&&(ss[pn]<='Z'))||((ss[pn]>='a')&&(ss[pn]<='z'))||((ss[pn]>='0')&&(ss[pn]<='9')))))
										{
											SYNTAXERROR=true;
											strcpy(ERRORSTR,tmp);
											return false;
										}
									}								
								}
							}
							pos=apos;
							tmp=str_parse(script,&pos);
							str_upcase(tmp);
						}

						if (strcmp(tmp,"MATRIX")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							if (str_char(tmp,'[')!=-1)
							{
								str=str_return_crochets(tmp);
								if (str==NULL) { SYNTAXERROR=true; return false; }

								tmp[str_char(tmp,'[')]='\0';

								sscanf(str,"%d",&nb);

#if defined(API3D_DIRECT3D9)||defined(WINDOWS_PHONE)||defined(WEBASM)||defined(RASPPI)
								if (nb>52) nb=52;
#else
#if defined(ANDROID)||defined(IOS)
								if (nb>54) nb=54;
#else
								if (nb>192) nb=192;
#endif
#endif

								if (api!=1)
								{
									if (var3)
									{
										str=(char*) malloc(strlen(tmp)+1);
										_sprintf(str,"%s",tmp);
										new_constant(str,_MATRIX,nb);
									}
									else
									{
										for (n=0;n<nb;n++)
										{
											str=(char*) malloc(strlen(tmp)+1+2);
											_sprintf(str,"%s%d",tmp,n);
											new_constant(str,_MATRIX);
										}
									}
								}
								else
								{
									str=(char*) malloc(strlen(tmp)+1);
									_sprintf(str,"%s",tmp);
									new_constant(str,_MATRIX,nb);
								}

							}
							else
							{
								str=(char*) malloc(strlen(tmp)+1);
								_sprintf(str,"%s",tmp);
								new_constant(str,_MATRIX);
							}
						}
						else
						if (strcmp(tmp,"VECTOR")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							if (str_char(tmp,'[')!=-1)
							{
								str=str_return_crochets(tmp);
								if (str==NULL) { SYNTAXERROR=true; return false; }

								tmp[str_char(tmp,'[')]='\0';

								sscanf(str,"%d",&nb);

								if (api!=1)
								{
									if (var3)
									{
										str=(char*) malloc(strlen(tmp)+1);
										_sprintf(str,"%s",tmp);
										new_constant(str,_VECTOR,nb);
									}
									else
									{
										for (n=0;n<nb;n++)
										{
											str=(char*) malloc(strlen(tmp)+1+2);
											_sprintf(str,"%s%d",tmp,n);
											new_constant(str,_VECTOR);
										}
									}
								}
								else
								{
									str=(char*) malloc(strlen(tmp)+1);
									_sprintf(str,"%s",tmp);
									new_constant(str,_VECTOR,nb);
								}

							}
							else
							{
								str=(char*) malloc(strlen(tmp)+1);
								_sprintf(str,"%s",tmp);
								new_constant(str,_VECTOR);
							}

						}
						else
						if ((strcmp(tmp,"PSVECTOR")==0)||(strcmp(tmp,"SHADERVECTOR")==0))
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							if (str_char(tmp,'[')!=-1)
							{
							}
							else
							{
								str=(char*) malloc(strlen(tmp)+1);
								_sprintf(str,"%s",tmp);
								new_psvector(str);
							}
						}
						else
						if ((strcmp(tmp,"MATRIXPS")==0)||(strcmp(tmp,"SHADERMATRIX")==0))
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							if (str_char(tmp,'[')!=-1)
							{
							}
							else
							{
								str=(char*) malloc(strlen(tmp)+1);
								_sprintf(str,"%s",tmp);
								new_psmatrix(str);
							}
						}
						else
						if (strcmp(tmp,"DWORD")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';
							str=(char*) malloc(strlen(tmp)+1);
							_sprintf(str,"%s",tmp);
							new_constant(str,_DWORD);
						}
						else
						if (strcmp(tmp,"CONST")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							_sprintf(chaine,"%s",tmp);

							p=str_char(chaine,'=');

							if (p!=-1) chaine[p]='\0';

							str=(char*) malloc(strlen(chaine)+1);
							_sprintf(str,"%s",chaine);
							new_constant(str,_VECTOR);

							vect=str_get_vector(&chaine[p+1]);

							if (flags&DIRECT3D)
							{
								//D3D

								str=(char*) malloc(256);
								_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
								set_translate_constant(chaine,str);

							}
							else
							{
								//GL

								str=(char*) malloc(256);
								_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);

								int tc=set_translate_constant(chaine,str);

								float_translate_constants[tc][0]=vect[0];
								float_translate_constants[tc][1]=vect[1];
								float_translate_constants[tc][2]=vect[2];
								float_translate_constants[tc][3]=vect[3];
							}
						}
						else
						if (strcmp(tmp,"PSCONST")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							_sprintf(chaine,"%s",tmp);

							p=str_char(chaine,'=');

							if (p!=-1) chaine[p]='\0';

							str=(char*) malloc(strlen(chaine)+1);
							_sprintf(str,"%s",chaine);
							new_ps_constant(str);

							vect=str_get_vector(&chaine[p+1]);

							if (flags&DIRECT3D)
							{
								//D3D

								str=(char*) malloc(256);
								_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
								set_translate_ps_constant(chaine,str);

							}
							else
							{
								//GL

								str=(char*) malloc(256);
								_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);

								int tc=set_translate_ps_constant(chaine,str);

								float_translate_ps_constants[tc][0]=vect[0];
								float_translate_ps_constants[tc][1]=vect[1];
								float_translate_ps_constants[tc][2]=vect[2];
								float_translate_ps_constants[tc][3]=vect[3];
							}
						}
						else
						if (strcmp(tmp,"PSMATRIX")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							_sprintf(chaine,"%s",tmp);

							p=str_char(chaine,'=');

							if (p!=-1) chaine[p]='\0';

							str=(char*) malloc(strlen(chaine)+1);
							_sprintf(str,"%s",chaine);
							new_ps_matrix(str);

							mat=str_get_matrix(&chaine[p+1]);

							if (flags&DIRECT3D)
							{
								//D3D
								str=(char*) malloc(1024);
								_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff,%3.8ff"
									,mat[0],mat[1],mat[2],mat[3]
									,mat[4],mat[5],mat[6],mat[7]
									,mat[8],mat[9],mat[10],mat[11]
									,mat[12],mat[13],mat[14],mat[15]);

								set_translate_ps_matrix(chaine,str);
							}
							else
							{
								//GL
								str=(char*) malloc(1024);
                                if (metal==1)
                                {
                                    _sprintf(str,"float4(%3.8f,%3.8f,%3.8f,%3.8f),float4(%3.8f,%3.8f,%3.8f,%3.8f),float4(%3.8f,%3.8f,%3.8f,%3.8f),float4(%3.8f,%3.8f,%3.8f,%3.8f)"
                                            ,mat[0],mat[1],mat[2],mat[3]
                                            ,mat[4],mat[5],mat[6],mat[7]
                                            ,mat[8],mat[9],mat[10],mat[11]
                                            ,mat[12],mat[13],mat[14],mat[15]);
                                    
                                }
                                else
                                {
                                    _sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f,%3.8f"
                                            ,mat[0],mat[1],mat[2],mat[3]
                                            ,mat[4],mat[5],mat[6],mat[7]
                                            ,mat[8],mat[9],mat[10],mat[11]
                                            ,mat[12],mat[13],mat[14],mat[15]);
                                    
                                }

								int tc=set_translate_ps_matrix(chaine,str);

								for (int kk=0;kk<16;kk++) float_translate_ps_constants[tc][kk]=mat[kk];
							}
						}
						else
						if (strcmp(tmp,"TEXTUREVS")==0)
						{
							apos=pos;
							tmp=str_parse_virg(script,&pos);
							str_clean(tmp);
							tmp[str_char(tmp,';')]='\0';

							_sprintf(chaine,"%s",tmp);
							str=(char*) malloc(strlen(chaine)+1);
							_sprintf(str,"%s",chaine);

							texturevs[ntexturevs]=str;
							ntexturevs++;
						}
						else
                        if (strcmp(tmp,"TEXTURE")==0)
                        {
                            apos=pos;
                            tmp=str_parse_virg(script,&pos);
                            str_clean(tmp);
                            tmp[str_char(tmp,';')]='\0';

                            _sprintf(chaine,"%s",tmp);
                            str=(char*) malloc(strlen(chaine)+1);
                            _sprintf(str,"%s",chaine);

                            texture[ntexture]=str;
                            ntexture++;
                        }
                        else
						{
							str_clean(tmp);
							if (strlen(tmp)>0)
							{
								SYNTAXERROR=true;
								strcpy(ERRORSTR,tmp);
								return false;
							}
						}

						apos=pos;
						tmp=str_parse(script,&pos);
					}
					else tmp=NULL;
				}

				pos=apos;
			}
			else
			if (strcmp(tmp,"[POSITION]")==0)
			{
				script_position[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[DIFFUSE]")==0)
			{
				script_diffuse[tech][pass]=str_get_script(script,&pos);
#ifdef GLES20
				char *fix="oDiffuse.r=clamp(oDiffuse.r,0.0,1.0);\n"
						"oDiffuse.g=clamp(oDiffuse.g,0.0,1.0);\n"
						"oDiffuse.b=clamp(oDiffuse.b,0.0,1.0);\n"
						"oDiffuse.a=clamp(oDiffuse.a,0.0,1.0);\n";
				char *Ascript=(char*) malloc(strlen(script_diffuse[tech][pass])+1+strlen(fix));

				memcpy(Ascript,script_diffuse[tech][pass],strlen(script_diffuse[tech][pass]));
				memcpy(&Ascript[strlen(script_diffuse[tech][pass])],fix,strlen(fix));
				Ascript[strlen(script_diffuse[tech][pass])+strlen(fix)]='\0';
				free(script_diffuse[tech][pass]);
				script_diffuse[tech][pass]=Ascript;
#endif
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[SPECULAR]")==0)
			{
				script_specular[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING0]")==0)
			{
				script_mapping0[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING1]")==0)
			{
				script_mapping1[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING2]")==0)
			{
				script_mapping2[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING3]")==0)
			{
				script_mapping3[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING4]")==0)
			{
				script_mapping4[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING5]")==0)
			{
				script_mapping5[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING6]")==0)
			{
				script_mapping6[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[MAPPING7]")==0)
			{
				script_mapping7[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[SHADER]")==0)
			{
				script_shader[tech][pass]=str_get_script(script,&pos);
                if (IFERROR) return false;
			}
			else
			if (strcmp(tmp,"[FUNCTIONS]")==0)
			{
				script_vsfunctions[tech][pass]=str_get_scriptfns(script,&pos);
                if (IFERROR) return false;
				if (script_vsfunctions[tech][pass]==NULL) { SYNTAXERRORFN=true; return false; }
			}
			else
			if (strcmp(tmp,"[PSFUNCTIONS]")==0)
			{
				script_psfunctions[tech][pass]=str_get_scriptfns(script,&pos);
                if (IFERROR) return false;
				if (script_psfunctions[tech][pass]==NULL) { SYNTAXERRORFN=true; return false; }
			}
			else
			if (strcmp(tmp,"[MACROS]")==0)
			{
				macros=str_get_scriptm(script,&pos);
				Translate_Macros(macros);
				free(macros);
			}
			else
			if (strcmp(tmp,"[RENDER_STATE]")==0)
			{
				RS_Reset(&RS);
				success&=str_get_RS(script,&pos,this);
				if (success)
					memcpy(&RenderState[tech][pass],&RS,sizeof(vpRenderState));
				else
                {
					_sprintf(RSERRORSTR,"%s",s_tmp_error);
                    RSERROR=true;
                    return false;                    
                }
			}
			else
			{
				if (strcmp(tmp,"[END]")!=0)
				{
					strcpy(ERRORSTR,tmp);
					SYNTAXERROR=true;
					return false;
				}
			}
		}
		tmp=str_parse(script,&pos);
	}

	if (pass==0)
	{
		// Alpha Test parametrable

		if (RenderState[0][0].AlphaRef==-666)
		{
			str=(char*)malloc(strlen("REFALPHA")+1);
			_sprintf(str,"REFALPHA");
			new_psvector(str);
		}
	
		// FSR Sampler 0 & 1 RCAS

		if ((SizingTexture0)&&(FSRSample0)&&(RenderState[0][0].Texture[0]))
		{
			str=(char*)malloc(strlen("SIZEDESTZ")+1);
			_sprintf(str,"SIZEDESTZ");
			new_psvector(str);
		}

		if ((SizingTexture0)&&(FSRSample0==0)&&(RenderState[0][0].Texture[0]))
		{
			str=(char*)malloc(strlen("SIZEDESTZ")+1);
			_sprintf(str,"SIZEDESTZ");
			new_psvector(str);

			char psfnbase0[4096];
			if(api==1) strcpy(psfnbase0,"TiledSample0(vec2 m)\n{\n c=sample(0,m.xy);\n c;\n};\n");
			else strcpy(psfnbase0,"TiledSample0(float2 m)\n{\n c=sample(0,m.xy);\n c;\n};\n");

			int p=0;
			char *psfn0=str_get_scriptfns(psfnbase0,&p);

			if(script_psfunctions[0][0])
			{
				char * buftmp=(char*)malloc(strlen(script_psfunctions[0][0])+strlen(psfn0)+4);
				_sprintf(buftmp,"%s%s",script_psfunctions[0][0],psfn0);
				free(script_psfunctions[0][0]);
				script_psfunctions[0][0]=buftmp;
			}
			else
			{
				char * buftmp=(char*)malloc(strlen(psfn0)+4);
				_sprintf(buftmp,"%s",psfn0);
				script_psfunctions[0][0]=buftmp;
			}
		}

		if ((FSRSample0)&&(RenderState[0][0].Texture[0]))
		{
			str=(char*) malloc(strlen("SIZEDEST0")+1);
			_sprintf(str,"SIZEDEST0");
			new_psvector(str);

			char psfnbase0[4096];
			if (api==1) strcpy(psfnbase0,"FSRSample0(vec2 m)\n{\n c=sample(0,m.xy);\n c;\n};\n");
			else strcpy(psfnbase0,"FSRSample0(float2 m)\n{\n c=sample(0,m.xy);\n c;\n};\n");

			int p=0;
			char *psfn0=str_get_scriptfns(psfnbase0,&p);

			if (script_psfunctions[0][0])
			{
				char * buftmp=(char*)malloc(strlen(script_psfunctions[0][0])+strlen(psfn0)+4);
				_sprintf(buftmp,"%s%s",script_psfunctions[0][0],psfn0);
				free(script_psfunctions[0][0]);
				script_psfunctions[0][0]=buftmp;
			}
			else
			{
				char * buftmp=(char*)malloc(strlen(psfn0)+4);
				_sprintf(buftmp,"%s",psfn0);
				script_psfunctions[0][0]=buftmp;
			}
		}

		if ((FSRSample1)&&(RenderState[0][0].Texture[1]))
		{
			str=(char*) malloc(strlen("SIZEDEST1")+1);
			_sprintf(str,"SIZEDEST1");
			new_psvector(str);

			char psfnbase1[4096];
			if (api==1) strcpy(psfnbase1,"FSRSample1(vec2 m)\n{\n c=sample(1,m.xy);\n c;\n};\n");
			else strcpy(psfnbase1,"FSRSample1(float2 m)\n{\n c=sample(1,m.xy);\n c;\n};\n");

			int p=0;
			char *psfn1=str_get_scriptfns(psfnbase1,&p);

			if (script_psfunctions[0][0])
			{
				char * buftmp=(char*)malloc(strlen(script_psfunctions[0][0])+strlen(psfn1)+4);
				_sprintf(buftmp,"%s%s",script_psfunctions[0][0],psfn1);
				free(script_psfunctions[0][0]);
				script_psfunctions[0][0]=buftmp;
			}
			else
			{
				char * buftmp=(char*)malloc(strlen(psfn1)+4);
				_sprintf(buftmp,"%s",psfn1);
				script_psfunctions[0][0]=buftmp;
			}
		}
	}

	// constantes pr�d�finies

	float tab[4];
	vect=tab;

	str_chaine=(char*) malloc(128);
	_sprintf(str_chaine,"trigo_mod");
	new_constant(str_chaine,_VECTOR);

	vect[0]=PI;
	vect[1]=1.0f/(2*PI);
	vect[2]=2*PI;
	vect[3]=3*PI/2;

	if (flags&DIRECT3D)
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
		set_translate_constant(str_chaine,str);

	}
	else
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);
		int tc=set_translate_constant(str_chaine,str);
		float_translate_constants[tc][0]=vect[0];
		float_translate_constants[tc][1]=vect[1];
		float_translate_constants[tc][2]=vect[2];
		float_translate_constants[tc][3]=vect[3];
	}

	// trigo_cst = 0,PI/2,1

	str_chaine=(char*) malloc(128);
	_sprintf(str_chaine,"trigo_cst");
	new_constant(str_chaine,_VECTOR);

	vect[0]=0;
	vect[1]=PI/2;
	vect[2]=1;
	vect[3]=PI;

	if (flags&DIRECT3D)
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
		set_translate_constant(str_chaine,str);

	}
	else
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);
		int tc=set_translate_constant(str_chaine,str);
		float_translate_constants[tc][0]=vect[0];
		float_translate_constants[tc][1]=vect[1];
		float_translate_constants[tc][2]=vect[2];
		float_translate_constants[tc][3]=vect[3];
	}

	str_chaine=(char*) malloc(128);
	_sprintf(str_chaine,"trigo_cst2");
	new_constant(str_chaine,_VECTOR);

	// trigo_cst2 = 1/6,1/120

	vect[0]=1.0f/6.0f;
	vect[1]=1.0f/120.0f;
	vect[2]=1.0f/5040.0f;
	vect[3]=0.5f;

	if (flags&DIRECT3D)
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
		set_translate_constant(str_chaine,str);

	}
	else
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);
		int tc=set_translate_constant(str_chaine,str);
		float_translate_constants[tc][0]=vect[0];
		float_translate_constants[tc][1]=vect[1];
		float_translate_constants[tc][2]=vect[2];
		float_translate_constants[tc][3]=vect[3];
	}


	str_chaine=(char*) malloc(128);
	_sprintf(str_chaine,"trigo_cst3");
	new_constant(str_chaine,_VECTOR);

	// trigo_cst3 = 1/3,2/15,17/315

	vect[0]=1.0f/3.0f;
	vect[1]=2.0f/15.0f;
	vect[2]=17.0f/315.0f;
	vect[3]=0.1f;


	if (flags&DIRECT3D)
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
		set_translate_constant(str_chaine,str);

	}
	else
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);
		int tc=set_translate_constant(str_chaine,str);
		float_translate_constants[tc][0]=vect[0];
		float_translate_constants[tc][1]=vect[1];
		float_translate_constants[tc][2]=vect[2];
		float_translate_constants[tc][3]=vect[3];
	}

	str_chaine=(char*) malloc(128);
	_sprintf(str_chaine,"trigo_cst4");
	new_constant(str_chaine,_VECTOR);

	// trigo_cst3 = 1/3,2/15,17/315

	vect[0]=-1.0f/3.0f;
	vect[1]=1.0f/5.0f;
	vect[2]=-1.0f/7.0f;
	vect[3]=0.25f;

	if (flags&DIRECT3D)
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8ff,%3.8ff,%3.8ff,%3.8ff",vect[0],vect[1],vect[2],vect[3]);
		set_translate_constant(str_chaine,str);

	}
	else
	{
		str=(char*) malloc(256);
		_sprintf(str,"%3.8f,%3.8f,%3.8f,%3.8f",vect[0],vect[1],vect[2],vect[3]);
		int tc=set_translate_constant(str_chaine,str);
		float_translate_constants[tc][0]=vect[0];
		float_translate_constants[tc][1]=vect[1];
		float_translate_constants[tc][2]=vect[2];
		float_translate_constants[tc][3]=vect[3];
	}


	if ((api==0)&&(!shadermodel3))
	{
		for (n=0;n<16;n++)
		{
			if (texture[n])
			{
				_sprintf(c.str,"TEXTURE %s;",texture[n]);
				vp.Add(c);
			}

		}
	}

	nm=0;
	nv=0;

	for (n=0;n<last_constant;n++)
	{
		if (api==0)
		{
			if (!translate_constants[n])
			{
				switch (l_constants[n]&0xffff)
				{
				case _MATRIX:
					_sprintf(c.str,"MATRIX %s;",constants[n]);
					vp.Add(c);
					break;
				case _VECTOR:
					_sprintf(c.str,"VECTOR %s;",constants[n]);
					vp.Add(c);
					break;
				case _DWORD:
					_sprintf(c.str,"DWORD %s;",constants[n]);
					vp.Add(c);
					break;
				};
			}
		}
	}


	found=false;
	for (t=0;t<ntech;t++)
	{
		if (!found)
		{
			if (((tech_spefs[t]&TECH_SHADERMODEL)&&(flags&PIXELSHADER2X_ENABLED))||(!(tech_spefs[t]&TECH_SHADERMODEL)))
			{
				if (((tech_spefs[t]&TECH_PIXELSHADER)&&(flags&PIXELSHADER_ENABLED))||(!(tech_spefs[t]&TECH_PIXELSHADER)))
				{
					if (((tech_spefs[t]&TECH_MULTITEXTURE)&&(flags&MULTITEXTURING_ENABLED))||(!(tech_spefs[t]&TECH_MULTITEXTURE)))
					{
						if (((tech_spefs[t]&TECH_DOT3)&&(flags&DOT3PRODUCT_ENABLED))||(!(tech_spefs[t]&TECH_DOT3)))
						{
							selected_tech=t;

							if (tech_spefs[selected_tech]&TECH_PIXELSHADER) selected_tech_with_ps=true;
							else selected_tech_with_ps=false;

							shadermodel=model3;
							if (tech_spefs[t]&TECH_SHADERMODEL)
							{
								shadermodel=true;
								if (tech_spefs[t]&TECH_SHADERMODEL3) shadermodel3=true;

								if (flags&DIRECT3D)
								{
									if (!shadermodel3)
									{
										str_chaine=(char*) malloc(128);
										_sprintf(str_chaine,"sc1");
										new_ps_constant(str_chaine);
										str=(char*) malloc(256);
										_sprintf(str,"-1.5500992e-006f, -2.1701389e-005f,  0.0026041667f, 0.00026041668f");
										set_translate_ps_constant(str_chaine,str);

										str_chaine=(char*) malloc(128);
										_sprintf(str_chaine,"sc2");
										new_ps_constant(str_chaine);
										str=(char*) malloc(256);
										_sprintf(str,"-0.020833334f, -0.12500000f, 1.0f, 0.50000000f");
										set_translate_ps_constant(str_chaine,str);
									}
									str_chaine=(char*) malloc(128);
									_sprintf(str_chaine,"zerocinq");
									new_ps_constant(str_chaine);
									str=(char*) malloc(256);
									_sprintf(str,"0.5f,0.5f,0.5f,0.0f");
									set_translate_ps_constant(str_chaine,str);
								}
							}

							if ((api==1)&&(shadermodel))
							{
                                for (n=0;n<16;n++)
                                {
                                    if (texturevs[n])
                                    {
                                        _sprintf(c.str,"uniform sampler2D %s;",texturevs[n]);
                                        glheader.Add(c);
                                    }
                                }

								for (n=0;n<last_constant;n++)
								{
									if (!translate_constants[n])
									{
										switch (l_constants[n]&0xffff)
										{
										case _MATRIX:
											if (l_constants[n]>0xffff)
											{
												nb=l_constants[n]>>16;

												_sprintf(c.str,"uniform mat4 %s[%d];",constants[n],nb);
												glheader.Add(c);
											}
											else
											{
												_sprintf(c.str,"uniform mat4 %s;",constants[n]);
												glheader.Add(c);
											}
											break;
										case _VECTOR:
                                            if (l_constants[n]>0xffff)
                                            {
                                                nb=l_constants[n]>>16;

                                                _sprintf(c.str,"uniform vec4 %s[%d];",constants[n],nb);
                                                glheader.Add(c);
                                            }
                                            else
                                            {
                                                _sprintf(c.str,"uniform vec4 %s;",constants[n]);
                                                glheader.Add(c);
                                            }
											break;
										case _DWORD:
											_sprintf(c.str,"uniform uint %s;",constants[n]);
											glheader.Add(c);
											break;
										};
									}
									else
									{
										_sprintf(c.str,"const vec4 %s = vec4( %s );",constants[n],translate_constants[n]);
										glheader.Add(c);
									}
									
								}
							}

							if ((api==1)&&(!shadermodel))
							{
								for (n=0;n<last_constant;n++)
								{
									switch (l_constants[n]&0xffff)
									{
									case _MATRIX:

										if (l_constants[n]>0xffff)
										{
											nb=l_constants[n]>>16;
											_sprintf(c.str,"PARAM %s[%d] = { program.env[%d..%d] };",constants[n],nb*4,nv,nv+nb*4-1);
											glheader.Add(c);
											nv+=nb*4;
										}
										else
										{
											_sprintf(c.str,"PARAM %s[4] = { state.matrix.program[%d] };",constants[n],nm);
											nm++;
											glheader.Add(c);
										}
										break;
									case _VECTOR:
										if (!translate_constants[n])
										{
											if (l_constants[n]<0xffff)
											{
												_sprintf(c.str,"PARAM %s = program.env[%d];",constants[n],nv);
												nv++;
											}
											else
											{
												nb=l_constants[n]>>16;
												_sprintf(c.str,"PARAM %s[%d] = { program.env[%d..%d] };",constants[n],nb,nv,nv+nb-1);
												nv+=nb;
											}
										}
										else
											_sprintf(c.str,"PARAM %s = { %s };",constants[n],translate_constants[n]);

										glheader.Add(c);
										break;
									case _DWORD:
										_sprintf(c.str,"PARAM %s;",constants[n]);
										glheader.Add(c);
										break;
									};
								}

							}
						

							if (api==1)
							{
								if (!shadermodel)
								{
									for (n=0;n<last_ps_constant;n++)
									{
										_sprintf(c.str,"PARAM %s = { %s };",ps_constants[n],translate_ps_constants[n]);
										glheaderps.Add(c);
									}

									_sprintf(c.str,"PARAM zerocinq = { 0.5,0.5,0.5,0.0 };");
									glheaderps.Add(c);
								}
								else
								{
									for (n=0;n<16;n++)
									{
										if (texture[n])
										{
											_sprintf(c.str,"uniform sampler2D %s;",texture[n]);
											glheaderps.Add(c);
										}
									}

									for (n=0;n<last_ps_constant;n++)
									{
										if (ps_constants_type[n]==1) _sprintf(c.str,"vec4 %s = vec4 ( %s );",ps_constants[n],translate_ps_constants[n]);
										else _sprintf(c.str,"mat4 %s = mat4 ( %s );",ps_constants[n],translate_ps_constants[n]);
										glheaderps.Add(c);
									}

									for (n=0;n<npsvectors;n++)
									{
										_sprintf(c.str,"uniform vec4 %s;",psvectors[n]);
										glheaderps.Add(c);
									}

									for (n=0;n<npsmatrices;n++)
									{
										_sprintf(c.str,"uniform mat4 %s;",psmatrices[n]);
										glheaderps.Add(c);
									}

								}
							}

							if (metal==1) setIO(METAL);
							else
							{
								if (api==0) setIO(DIRECT3D);
								if (api==1) setIO(OPENGL);
								if (api==2) setIO(EMULATED);
							}

							found=true;
							if (!api)
							{
								c.str[0]='\0';
								vp.Add(c);

								_sprintf(c.str,"TECHNIQUE T0");
								vp.Add(c);
								_sprintf(c.str,"{");
								vp.Add(c);
								NPASS=1;
							}
							else NPASS=npass_max[t];
							/*
							for (p=1;p<npass_max[t];p++)
							{
								if (!script_position[t][p]) script_position[t][p]=script_position[t][p-1];
								if (!script_diffuse[t][p]) script_diffuse[t][p]=script_diffuse[t][p-1];
								if (!script_specular[t][p]) script_specular[t][p]=script_specular[t][p-1];
								if (!script_mapping0[t][p]) script_mapping0[t][p]=script_mapping0[t][p-1];
								if (!script_mapping1[t][p]) script_mapping1[t][p]=script_mapping1[t][p-1];
								if (!script_mapping2[t][p]) script_mapping2[t][p]=script_mapping2[t][p-1];
								if (!script_mapping3[t][p]) script_mapping3[t][p]=script_mapping3[t][p-1];
								if (!script_mapping4[t][p]) script_mapping4[t][p]=script_mapping4[t][p-1];
								if (!script_vsfunctions[t][p]) script_vsfunctions[t][p]=script_vsfunctions[t][p-1];
							}
							/**/
							if ((api==0)&&(shadermodel3)) NPASS=npass_max[t];

							for (p=0;p<npass_max[t];p++)
							{
								actual_pass_trigocst=p;
								for (int tn=0;tn<6;tn++) trigocst[p][tn]=0;

								if (script_shader[t][p]) parse_inner_func(script_shader[t][p],script_psfunctions[t][p]);  // built-in functions
								parse_inner_func_position(script_position[t][p]);

								//printf("%s\n",script_shader[t][p]);
								//printf("FUNCS:\n");
								//printf("%s\n",script_psfunctions[t][p]);

								memcpy(&RS,&RenderState[selected_tech][p],sizeof(vpRenderState));
								
								nmatrices=0;
								nfloats=0;

								TempVars.Free();
								Vars.Free();
								clear_registers();

								if ((api==0)&&(!shadermodel3))
								{
									c.str[0]='\0';
									vp.Add(c);

									header_open_pass(p);
									compile_render_state(t,p,&vp);
								}

								output=0;

								if (script_position[t][p]) output|=XYZ;
								if (script_diffuse[t][p]) output|=DIFFUSE;
								if (script_specular[t][p]) output|=SPECULAR;
								if (script_mapping0[t][p]) output|=_TEX0;
								if (script_mapping1[t][p]) output|=_TEX1;
								if (script_mapping2[t][p]) output|=_TEX2;
								if (script_mapping3[t][p]) output|=_TEX3;
								if (script_mapping4[t][p]) output|=_TEX4;
								if (script_mapping5[t][p]) output|=_TEX5;
								if (script_mapping6[t][p]) output|=_TEX6;
								if (script_mapping7[t][p]) output|=_TEX7;

								if ((shadermodel3)&&(api==0))
								{
									if (p==0) header_open_vsh(p,(1-api)*DIRECT3D+api*OPENGL+d3d9*D3D9|fvf,output);
								}
								else
								{
									if (api!=2) header_open_vsh(p,(1-api)*DIRECT3D+api*OPENGL+d3d9*D3D9|fvf,output);
								}
								fvf_output[p]=output;

								if ((api==0)&&(!shadermodel3))
								{
									pixelshader=false;

									parse_script_vs_functions(script_vsfunctions[t][p]);

									ndefs=0;

									if (script_position[t][p]) compile_script(script_position[t][p],&vp);
									if (script_diffuse[t][p]) compile_script(script_diffuse[t][p],&vp);
									if (script_specular[t][p]) compile_script(script_specular[t][p],&vp);
									if (script_mapping0[t][p]) compile_script(script_mapping0[t][p],&vp);
									if (script_mapping1[t][p]) compile_script(script_mapping1[t][p],&vp);
									if (script_mapping2[t][p]) compile_script(script_mapping2[t][p],&vp);
									if (script_mapping3[t][p]) compile_script(script_mapping3[t][p],&vp);
									if (script_mapping4[t][p]) compile_script(script_mapping4[t][p],&vp);
									if (script_mapping5[t][p]) compile_script(script_mapping5[t][p],&vp);
									if (script_mapping6[t][p]) compile_script(script_mapping6[t][p],&vp);
									if (script_mapping7[t][p]) compile_script(script_mapping7[t][p],&vp);

									unused_variables();

									if ((script_vsfunctions[t][p])&&(shadermodel)&&(!shadermodel3))
									{
										_sprintf(c.str,"ret");
										vp.Add(c);
										// TODO
										for (int kk=0;kk<numvsfn;kk++)
										{
											_sprintf(c.str,"label l%d",kk);
											vp.Add(c);
											ndefs=0;
											compile_function(&vsfn[kk],&vp);
											unused_variables();
											_sprintf(c.str,"ret");
											vp.Add(c);
										}
									}

									header_close();									

									if (script_shader[t][p])
									{
										if (tech_spefs[t]&TECH_SHADERMODEL) selected_tech_with_ps=true;
										pixelshader=true;
										clear_registers();
										header_open_psh(p,DIRECT3D|fvf,output);

										parse_script_ps_functions(script_psfunctions[t][p]);

										ndefs=0;

										compile_script(script_shader[t][p],&vp);
										unused_variables();

										if (shadermodel)
										{
											_sprintf(c.str, "mov oC0,r0");
											vp.Add(c);

											if (script_psfunctions[t][p])
											{
												_sprintf(c.str,"ret");
												vp.Add(c);
												for (int kk=0;kk<numpsfn;kk++)
												{
													_sprintf(c.str,"label l%d",kk);
													vp.Add(c);													
													compile_psfunction(&psfn[kk],&vp);													
													unused_variables();
													_sprintf(c.str,"ret");
													vp.Add(c);
												}
											}
										}
				
										header_close_psh();
										pixelshader=false;
									}

									header_end();
								}
								else
								if ((api==0)&&(shadermodel3))
								{
									// RENDER
									// RS
									compile_render_state(t,p,&s3rs[p]);

									globalvar=false;

									pixelshader=false;
									addsemi=true;
									parse_script_vs_functions(script_vsfunctions[t][p]);

									for (n=0;n<32;n++) tagregsave[n]=0;

									ndefs=0;

									if (script_position[t][p]) compile_script(script_position[t][p],&s3vp[p]);
									if (script_diffuse[t][p]) compile_script(script_diffuse[t][p],&s3vp[p]);
									if (script_specular[t][p]) compile_script(script_specular[t][p],&s3vp[p]);
									if (script_mapping0[t][p]) compile_script(script_mapping0[t][p],&s3vp[p]);
									if (script_mapping1[t][p]) compile_script(script_mapping1[t][p],&s3vp[p]);
									if (script_mapping2[t][p]) compile_script(script_mapping2[t][p],&s3vp[p]);
									if (script_mapping3[t][p]) compile_script(script_mapping3[t][p],&s3vp[p]);
									if (script_mapping4[t][p]) compile_script(script_mapping4[t][p],&s3vp[p]);
									if (script_mapping5[t][p]) compile_script(script_mapping5[t][p],&s3vp[p]);
									if (script_mapping6[t][p]) compile_script(script_mapping6[t][p],&s3vp[p]);
									if (script_mapping7[t][p]) compile_script(script_mapping7[t][p],&s3vp[p]);

									unused_variables();

									_sprintf(c.str,"");
									s3vp[p].InsertFirst(c);

									if (weightsandindices)
									{
										if (metal==0)
										if (p==0) strcpy(c.str,"static int4 WInd;");vpcst.Add(c);
										
										if (shadermodel4)
										{
											if (metal==0)
											if (p==0) strcpy(c.str,"static float4 Weights;");vpcst.Add(c);

                                            strcpy(c.str,"Weights.x = IWeights.x - (float(WInd.x)*2.0);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.x  = int(IWeights.x/2.0);");s3vp[p].InsertFirst(c);
											
                                            strcpy(c.str,"Weights.y = IWeights.y - (float(WInd.y)*2.0);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.y  = int(IWeights.y/2.0);");s3vp[p].InsertFirst(c);
											
                                            strcpy(c.str,"Weights.z = IWeights.z - (float(WInd.z)*2.0);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.z  = int(IWeights.z/2.0);");s3vp[p].InsertFirst(c);
											
                                            strcpy(c.str,"Weights.w = IWeights.w - (float(WInd.w)*2.0);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.w  = int(IWeights.w/2.0);");s3vp[p].InsertFirst(c);
											if (stream)
											{
												if (metal==0) strcpy(c.str,"float4 IWeights = i.v1;");
												else strcpy(c.str,"float4 IWeights = indexed.v2;");
											}
											else
											{
												if (metal==0) strcpy(c.str,"float4 IWeights = i.v1;");
												else strcpy(c.str,"float4 IWeights = indexed.v1;");
											}											
											s3vp[p].InsertFirst(c);
											
                                            strcpy(c.str,"");s3vp[p].InsertFirst(c);

										}
										else
										{
                                            strcpy(c.str,"WInd.w  = ((twi/16777216));");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.z  = ((twi*256)/16777216);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.y  = ((twi*65536)/16777216);");s3vp[p].InsertFirst(c);
                                            strcpy(c.str,"WInd.x  = ((twi*16777216)/16777216);");s3vp[p].InsertFirst(c);

											strcpy(c.str,"int twi = i.v2;");s3vp[p].InsertFirst(c);
										}
										_sprintf(c.str,"");s3vp[p].InsertFirst(c);
									}
									
									_sprintf(c.str,"");
									s3vp[p].InsertFirst(c);
									if (metal==0)
                                    {
                                        if (output&_TEX7) { strcpy(c.str,"o.t7 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX6) { strcpy(c.str,"o.t6 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX5) { strcpy(c.str,"o.t5 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX4) { strcpy(c.str,"o.t4 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX3) { strcpy(c.str,"o.t3 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX2) { strcpy(c.str,"o.t2 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX1) { strcpy(c.str,"o.t1 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&_TEX0) { strcpy(c.str,"o.t0 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&SPECULAR) { strcpy(c.str,"o.v1 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                        if (output&DIFFUSE) { strcpy(c.str,"o.v0 = float4(0.0f,0.0f,0.0f,0.0f);"); s3vp[p].InsertFirst(c); }
                                    }
                                    
									for (n=31;n>=0;n--)
										if (tagregsave[n])
										{
                                            if (IsRegisterIn(n,s3vp[p]))
                                            {
                                                _sprintf(c.str,"float4 r%d;",n);
                                                s3vp[p].InsertFirst(c);
                                            }
										}
									
									_sprintf(c.str,"");
									s3vp[p].InsertFirst(c);
									_sprintf(c.str,"VS_OUTPUT%d o;",p);
									s3vp[p].InsertFirst(c);
                                    strcpy(c.str,"{");
									if ((numvsfn>0)||(metal==0)) s3vp[p].InsertFirst(c);

									if (metal==0)
									{
										_sprintf(c.str,"VS_OUTPUT%d RenderPassVS%d(VS_INPUT i)",p,p);
										s3vp[p].InsertFirst(c);
									}

                                    strcpy(c.str,"");
									s3vp[p].Add(c);

									// diffuse est toujours born�

									if (metal==0)
									{
										if (output&DIFFUSE)
										{
											_sprintf(c.str,"o.v0 = clamp( o.v0, 0.0f, 1.0f);");
											s3vp[p].Add(c);
										}
                                        strcpy(c.str,"return o;");
										s3vp[p].Add(c);
                                        strcpy(c.str,"}");
										s3vp[p].Add(c);
									}

									numbervsfn[p]=numvsfn;

									if (script_vsfunctions[t][p])
									{
										vsfncodeline[0][p]=0;
										for (int kk=0;kk<numvsfn;kk++)
										{
											for (n=0;n<32;n++) tagregsave[n]=0;
											for (n=0;n<32;n++) tagreg[n]=0;
											for (n=0;n<32;n++) tagtempreg[n]=0;

											_sprintf(&vsfnname[kk][p][0],compile_function(&vsfn[kk],&vptmp));
											unused_variables();

											for (n=31;n>=0;n--)
												if (tagregsave[n])
												{
                                                    if (IsRegisterIn(n,vptmp))
                                                    {
                                                        _sprintf(c.str,"float4 r%d;",n);
                                                        vptmp.InsertFirst(c);
                                                    }
												}

                                            strcpy(c.str,"{");
											vptmp.InsertFirst(c);
                                            strcpy(c.str,"}");
											vptmp.Add(c);
											_sprintf(c.str,entete);
											vptmp.InsertFirst(c);

											Code *wc=vptmp.GetFirst();
											while (wc)
											{
												s3fnvp[p].Add(*wc);
												wc=vptmp.GetNext();
											}

											vsfncodeline[kk+1][p]=vsfncodeline[kk][p]+vptmp.Length()+1;

											vptmp.Free();

                                            strcpy(c.str,"");
											s3fnvp[p].Add(c);
										}
									}

									if (metal==1)
									{
                                        // METAL VS
                                        
										int NB=96;
										if (shadermodel) NB=256;
										if (shadermodel3) NB=MAX_ENTRIES;

										if (numvsfn==0)
										{
											if (fvf&VPMORPH)
											{
												_sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT1 *vertex_array[[buffer(0)]],",p,p); s3vpmetal[p].Add(c);
												_sprintf(c.str,"                                const device VS_INPUT2 *vertex_array2[[ buffer(1) ]],"); s3vpmetal[p].Add(c);
												_sprintf(c.str,"                                constant constants_t& constants[[ buffer(2) ]],"); s3vpmetal[p].Add(c);
                                                
                                                
                                                int nt=0;
                                                for (n=0;n<16;n++)
                                                {
                                                    if (texturevs[n])
                                                    {
                                                        _sprintf(c.str,"                                texture2d<float> %s[[texture(%d)]],",texturevs[n],nt);
                                                        s3vpmetal[p].Add(c);
                                                        nt++;
                                                    }
                                                }
                                                    
                                                strcpy(c.str,"                                sampler s0[[sampler(0)]],");
                                                s3vpmetal[p].Add(c);
                                                
                                                strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"{"); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"#define indexed vertex_array[vid]"); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"#define indexed2 vertex_array2[vid]"); s3vpmetal[p].Add(c);
											}
											else
											{
												_sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT *vertex_array[[buffer(0)]],",p,p); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"                                constant constants_t& constants[[ buffer(1) ]],"); s3vpmetal[p].Add(c);
                                                
                                                int nt=0;
                                                for (n=0;n<16;n++)
                                                {
                                                    if (texturevs[n])
                                                    {
                                                        _sprintf(c.str,"                                texture2d<float> %s[[texture(%d)]],",texturevs[n],nt);
                                                        s3vpmetal[p].Add(c);
                                                        nt++;
                                                    }
                                                }
                                                    
                                                strcpy(c.str,"                                sampler s0[[sampler(0)]],");
                                                s3vpmetal[p].Add(c);

                                                strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"{"); s3vpmetal[p].Add(c);
                                                strcpy(c.str,"#define indexed vertex_array[vid]"); s3vpmetal[p].Add(c);
											}


											for (n=0;n<NB;n++)
											{
												if (constants[n])
												{
													if (translate_constants[n]==NULL)
													{
														if (LENGTH_VAR[l_constants[n]&0xffff]==1) _sprintf(c.str,"#define %s constants.%s",constants[n],constants[n]);
														if (LENGTH_VAR[l_constants[n]&0xffff]==4)
														{
															_sprintf(c.str,"#define %s constants.%s",constants[n],constants[n]);
														}
														s3vpmetal[p].Add(c);
													}
												}
											}

											_sprintf(c.str,"");
											s3vpmetal[p].Add(c);

											if (weightsandindices)
											{
                                                strcpy(c.str,"int4 WInd;");s3vpmetal[p].Add(c);
                                                strcpy(c.str,"float4 Weights;");s3vpmetal[p].Add(c);
											}

											for (n=s3vpmetal[p].Length()-1;n>=0;n--)
											{
												Code * cs=s3vpmetal[p][n];
												s3vp[p].InsertFirst(*cs);
											}

											if (output&DIFFUSE)
											{
                                                strcpy(c.str,"o.v0 = clamp( o.v0, 0.0f, 1.0f);");
												s3vp[p].Add(c);
											}

                                            strcpy(c.str,"return o;"); s3vp[p].Add(c);
                                            strcpy(c.str,"}"); s3vp[p].Add(c);

										}
										else
										{
										
											_sprintf(c.str,"struct VertexShader%d",p);s3vpmetal[p].Add(c);
                                            strcpy(c.str,"{");s3vpmetal[p].Add(c);

											int lasti=0;

                                            strcpy(c.str,"constant constants_t &cb;");
											s3vpmetal[p].Add(c);
                                            strcpy(c.str,"");
											s3vpmetal[p].Add(c);
                                            
                                            int nt=0;
                                            for (n=0;n<16;n++)
                                            {
                                                if (texturevs[n])
                                                {
                                                    _sprintf(c.str,"texture2d<float> %s;",texturevs[n]);
                                                    s3vpmetal[p].Add(c);
                                                    nt++;
                                                }
                                            }
                                                
                                            strcpy(c.str,"sampler s0;");
                                            s3vpmetal[p].Add(c);

											for (n=0;n<NB;n++)
											{
												if (constants[n])
												{
													if (translate_constants[n])
													{
													}
													else
													{
														if (LENGTH_VAR[l_constants[n]&0xffff]==1) _sprintf(c.str,"#define %s cb.%s",constants[n],constants[n]);
														if (LENGTH_VAR[l_constants[n]&0xffff]==4)
														{
															_sprintf(c.str,"#define %s cb.%s",constants[n],constants[n]);
														}
														lasti=n;
														s3vpmetal[p].Add(c);
													}
												}
											}

											_sprintf(c.str,"");
											s3vpmetal[p].Add(c);

											if (weightsandindices)
											{
                                                strcpy(c.str,"int4 WInd;");s3vpmetal[p].Add(c);
                                                strcpy(c.str,"float4 Weights;");s3vpmetal[p].Add(c);
											}

											if (fvf&VPMORPH)
											{
                                                strcpy(c.str,"thread VS_INPUT1 &indexed;");s3vpmetal[p].Add(c);
                                                strcpy(c.str,"thread VS_INPUT2 &indexed2;");s3vpmetal[p].Add(c);
											}
											else
											{
                                                strcpy(c.str,"thread VS_INPUT &indexed;");s3vpmetal[p].Add(c);
											}

											c.str[0]='\0'; s3vpmetal[p].Add(c);

                                            if (ntexturevs==0)
                                            {
                                                if (fvf&VPMORPH) _sprintf(c.str,"VertexShader%d(thread VS_INPUT1 &v,thread VS_INPUT2 &v2,constant constants_t& c) :",p);
                                                else _sprintf(c.str,"VertexShader%d(thread VS_INPUT &v,constant constants_t& c) :",p);
                                                s3vpmetal[p].Add(c);
                                            }
                                            else
                                            {
                                                if (fvf&VPMORPH)
                                                {
                                                    _sprintf(c.str,"VertexShader%d(thread VS_INPUT1 &v,thread VS_INPUT2 &v2,constant constants_t& c,",p);
                                                    s3vpmetal[p].Add(c);
                                                    
                                                    for (n=0;n<16;n++)
                                                    {
                                                        if (texturevs[n])
                                                        {
                                                            _sprintf(c.str,"thread texture2d<float>& tex%d,",n);
                                                            s3vpmetal[p].Add(c);
                                                        }
                                                    }

                                                    strcpy(c.str,"thread sampler& smp0) :");
                                                    s3vpmetal[p].Add(c);
                                                }
                                                else
                                                {
                                                    _sprintf(c.str,"VertexShader%d(thread VS_INPUT &v,constant constants_t& c,",p);
                                                    s3vpmetal[p].Add(c);

                                                    for (n=0;n<16;n++)
                                                    {
                                                        if (texturevs[n])
                                                        {
                                                            _sprintf(c.str,"thread texture2d<float>& tex%d,",n);
                                                            s3vpmetal[p].Add(c);
                                                        }
                                                    }

                                                    strcpy(c.str,"thread sampler& smp0) :");
                                                    s3vpmetal[p].Add(c);
                                                }

                                            }

                                            strcpy(c.str,"indexed(v),");s3vpmetal[p].Add(c);
											if (fvf&VPMORPH) { strcpy(c.str,"indexed2(v2),");s3vpmetal[p].Add(c); }

                                            if (ntexturevs==0)
                                            {
                                                strcpy(c.str,"cb(c)");s3vpmetal[p].Add(c);
                                            }
                                            else
                                            {
                                                strcpy(c.str,"cb(c),");s3vpmetal[p].Add(c);

                                                for (n=0;n<16;n++)
                                                {
                                                    if (texturevs[n])
                                                    {
                                                        _sprintf(c.str,"%s(tex%d),",texturevs[n],n);
                                                        s3vpmetal[p].Add(c);
                                                    }
                                                }

                                                strcpy(c.str,"s0(smp0)");s3vpmetal[p].Add(c);
                                            }

										
											Code *ccc=s3vpmetal[p].Last();
											if (ccc->str[strlen(ccc->str)-1]==',') ccc->str[strlen(ccc->str)-1]='\0';

                                            strcpy(c.str,"{");s3vpmetal[p].Add(c);
                                            strcpy(c.str,"};");s3vpmetal[p].Add(c);

											c.str[0]='\0'; s3vpmetal[p].Add(c);


											Code *cc=s3fnvp[p].GetFirst();
											while (cc)
											{
												s3vpmetal[p].Add(*cc);
												cc=s3fnvp[p].GetNext();
											}

											c.str[0]='\0'; s3vpmetal[p].Add(c);

											_sprintf(c.str,"VS_OUTPUT%d main()",p);s3vpmetal[p].Add(c);

											// UP

											for (n=s3vpmetal[p].Length()-1;n>=0;n--)
											{
												Code * cs=s3vpmetal[p][n];
												s3vp[p].InsertFirst(*cs);
											}

											if (output&DIFFUSE)
											{
                                                strcpy(c.str,"o.v0 = clamp( o.v0, 0.0f, 1.0f);");
												s3vp[p].Add(c);
											}

                                            strcpy(c.str,"return o;"); s3vp[p].Add(c);
                                            strcpy(c.str,"}"); s3vp[p].Add(c);
                                            strcpy(c.str,"};"); s3vp[p].Add(c);

											c.str[0]='\0'; s3vp[p].Add(c);
                                            
                                            if (ntexturevs==0)
                                            {
                                                if (fvf&VPMORPH)
                                                {
                                                    _sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT1 *vertex_array[[buffer(0)]],",p,p); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                const device VS_INPUT2 *vertex_array2[[ buffer(1) ]],"); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                constant constants_t& constants[[ buffer(2) ]],"); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vp[p].Add(c);
                                                    strcpy(c.str,"{"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT1 vert(vertex_array[vid]);"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT2 vert2(vertex_array2[vid]);"); s3vp[p].Add(c);
                                                    _sprintf(c.str,"VertexShader%d vs(vert,vert2,constants);",p); s3vp[p].Add(c);
                                                    _sprintf(c.str,"return vs.main();"); s3vp[p].Add(c);
                                                    _sprintf(c.str,"}"); s3vp[p].Add(c);
                                                }
                                                else
                                                {
                                                    _sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT *vertex_array[[buffer(0)]],",p,p); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                constant constants_t& constants[[ buffer(1) ]],"); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vp[p].Add(c);
                                                    strcpy(c.str,"{"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT vert(vertex_array[vid]);"); s3vp[p].Add(c);
                                                    _sprintf(c.str,"VertexShader%d vs(vert,constants);",p); s3vp[p].Add(c);
                                                    strcpy(c.str,"return vs.main();"); s3vp[p].Add(c);
                                                    strcpy(c.str,"}"); s3vp[p].Add(c);
                                                }
                                            }
                                            else
                                            {
                                                if (fvf&VPMORPH)
                                                {
                                                    _sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT1 *vertex_array[[buffer(0)]],",p,p); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                const device VS_INPUT2 *vertex_array2[[ buffer(1) ]],"); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                constant constants_t& constants[[ buffer(2) ]],"); s3vp[p].Add(c);
                                                    
                                                    
                                                    for (n=0;n<16;n++)
                                                    {
                                                        if (texturevs[n])
                                                        {
                                                            _sprintf(c.str,"                                texture2d<float> %s[[texture(%d)]],",texturevs[n],n); s3vp[p].Add(c);
                                                        }
                                                    }

                                                    strcpy(c.str,"                                sampler texsampler0[[sampler(1)]],"); s3vp[p].Add(c);
                                                    
                                                    strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vp[p].Add(c);
                                                    strcpy(c.str,"{"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT1 vert(vertex_array[vid]);"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT2 vert2(vertex_array2[vid]);"); s3vp[p].Add(c);
                                                    _sprintf(c.str,"VertexShader%d vs(vert,vert2,constants,",p); s3vp[p].Add(c);
                                                    
                                                    for (n=0;n<16;n++)
                                                        if (texturevs[n]) { _sprintf(c.str,"%s,",texturevs[n]); s3vp[p].Add(c); }
                                                    
                                                    strcpy(c.str,"texsampler0);"); s3vp[p].Add(c);
                                                    
                                                    strcpy(c.str,"return vs.main();"); s3vp[p].Add(c);
                                                    strcpy(c.str,"}"); s3vp[p].Add(c);
                                                }
                                                else
                                                {
                                                    _sprintf(c.str,"vertex VS_OUTPUT%d RenderPassVS%d(const device VS_INPUT *vertex_array[[buffer(0)]],",p,p); s3vp[p].Add(c);
                                                    strcpy(c.str,"                                constant constants_t& constants[[ buffer(1) ]],"); s3vp[p].Add(c);
                                                    
                                                    for (n=0;n<16;n++)
                                                    {
                                                        if (texturevs[n])
                                                        {
                                                            _sprintf(c.str,"                                texture2d<float> %s[[texture(%d)]],",texturevs[n],n); s3vp[p].Add(c);
                                                        }
                                                    }

                                                    strcpy(c.str,"                                sampler texsampler0[[sampler(1)]],"); s3vp[p].Add(c);

                                                    strcpy(c.str,"                                unsigned int vid[[ vertex_id ]])"); s3vp[p].Add(c);
                                                    strcpy(c.str,"{"); s3vp[p].Add(c);
                                                    strcpy(c.str,"VS_INPUT vert(vertex_array[vid]);"); s3vp[p].Add(c);
                                                    _sprintf(c.str,"VertexShader%d vs(vert,constants,",p); s3vp[p].Add(c);
                                                    
                                                    for (n=0;n<16;n++)
                                                        if (texturevs[n]) { _sprintf(c.str,"%s,",texturevs[n]); s3vp[p].Add(c); }

                                                    strcpy(c.str,"texsampler0);"); s3vp[p].Add(c);

                                                    strcpy(c.str,"return vs.main();"); s3vp[p].Add(c);
                                                    strcpy(c.str,"}"); s3vp[p].Add(c);
                                                }
                                            }
										}
									}

									if ((script_shader[t][p]==NULL)&&(shadermodel4))
									{
										ndefs=0;
										if (tech_spefs[t]&TECH_SHADERMODEL) selected_tech_with_ps=true;
										pixelshader=true;
										clear_registers();

										if (compile_render_state_pixelshader(t,p,&s3psvp[p]))
										{
											header_open_psh(p,DIRECT3D|fvf,output);
											if ((p==0)&&(metal==0))
											{
												if (globalvar)
												{
													if (output&_TEX7) { strcpy(c.str,"static float4 it7;"); vpcst.Add(c); }
													if (output&_TEX6) { strcpy(c.str,"static float4 it6;"); vpcst.Add(c); }
													if (output&_TEX5) { strcpy(c.str,"static float4 it5;"); vpcst.Add(c); }
													if (output&_TEX4) { strcpy(c.str,"static float4 it4;"); vpcst.Add(c); }
													if (output&_TEX3) { strcpy(c.str,"static float4 it3;"); vpcst.Add(c); }
													if (output&_TEX2) { strcpy(c.str,"static float4 it2;"); vpcst.Add(c); }
													if (output&_TEX1) { strcpy(c.str,"static float4 it1;"); vpcst.Add(c); }
													if (output&_TEX0) { strcpy(c.str,"static float4 it0;"); vpcst.Add(c); }
													if (output&SPECULAR) { strcpy(c.str,"static float4 iv1;"); vpcst.Add(c); }
													if (output&DIFFUSE) { strcpy(c.str,"static float4 iv0;"); vpcst.Add(c); }
												}
												if (modifyZ) { strcpy(c.str,"static float4 ip0;"); vpcst.Add(c); }
												
											}

											if (shadermodel4)
											{
                                                if (metal==1)
                                                {
                                                    if (RS.AlphaTest==_TRUE)
                                                    {
														float valrefalpha=16.0f/255.0f;
														if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;
                                                        
														_sprintf(c.str,"if (o.Out.a<%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (o.Out.a>=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (o.Out.a>%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (o.Out.a<=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (o.Out.a<%3.3ff)",valrefalpha);
                                                        
														s3psvp[p].Add(c);
                                                        
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);													
                                                    }
                                                }
                                                else
                                                {
                                                    if (RS.AlphaTest==_TRUE)
                                                    {
														float valrefalpha=16.0f/255.0f;
														if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;
                                                        
														_sprintf(c.str,"if (r0.a<%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (r0.a>=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (r0.a>%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (r0.a<=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (r0.a<%3.3ff)",valrefalpha);
                                                        
														s3psvp[p].Add(c);
                                                        
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);													
                                                    }                                                    
                                                }
											}

                                            strcpy(c.str,"o.Out = r0;");
											s3psvp[p].Add(c);

                                            strcpy(c.str,"");
											s3psvp[p].InsertFirst(c);

                                            strcpy(c.str,"float4 r4;");
                                            if (IsRegisterIn(4,s3psvp[p])) s3psvp[p].InsertFirst(c);
                                            strcpy(c.str,"float4 r3;");
                                            if (IsRegisterIn(3,s3psvp[p])) s3psvp[p].InsertFirst(c);
                                            strcpy(c.str,"float4 r2;");
                                            if (IsRegisterIn(2,s3psvp[p])) s3psvp[p].InsertFirst(c);
                                            strcpy(c.str,"float4 r1;");
                                            if (IsRegisterIn(1,s3psvp[p])) s3psvp[p].InsertFirst(c);
                                            strcpy(c.str,"float4 r0;");
                                            if (IsRegisterIn(0,s3psvp[p])) s3psvp[p].InsertFirst(c);

                                            strcpy(c.str,"");
											s3psvp[p].InsertFirst(c);

											if (metal==0)
											{
												if (globalvar)
												{
													if (output&_TEX7) { strcpy(c.str,"it7 = i.t7;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX6) { strcpy(c.str,"it6 = i.t6;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX5) { strcpy(c.str,"it5 = i.t5;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX4) { strcpy(c.str,"it4 = i.t4;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX3) { strcpy(c.str,"it3 = i.t3;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX2) { strcpy(c.str,"it2 = i.t2;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX1) { strcpy(c.str,"it1 = i.t1;"); s3psvp[p].InsertFirst(c); }
													if (output&_TEX0) { strcpy(c.str,"it0 = i.t0;"); s3psvp[p].InsertFirst(c); }
													if (output&SPECULAR) { strcpy(c.str,"iv1 = i.v1;"); s3psvp[p].InsertFirst(c); }
													if (output&DIFFUSE) { strcpy(c.str,"iv0 = i.v0;"); s3psvp[p].InsertFirst(c); }
												}
												if (modifyZ) { strcpy(c.str,"ip0 = i.p0;"); s3psvp[p].InsertFirst(c); }
											}
											//toto

											int lastnt=-1;

											if (metal==1)
											{
												_sprintf(c.str,"fragment PS_OUTPUT RenderPassPS%d(VS_OUTPUT%d pixel [[stage_in]],",p,p);
												s3psvpmetal[p].Add(c);
												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"                                constant constants_ps& constants[[ buffer(0) ]],");
													s3psvpmetal[p].Add(c);
												}

												int nt=0;
												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"                                texture2d<float> %s[[texture(%d)]],",texture[n],nt);
														s3psvpmetal[p].Add(c);
														nt++;
													}
												}
                                                    
                                                strcpy(c.str,"                                sampler texsampler0[[sampler(0)]],");
                                                s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"                                sampler texsampler1[[sampler(1)]])");
                                                s3psvpmetal[p].Add(c);
                                                    
												_sprintf(c.str,"{"); s3psvpmetal[p].Add(c);
                                                                                              
                                                for (n=0;n<MAX_ENTRIES;n++)
                                                {
                                                    if (ps_constants[n])
                                                    {
                                                        if (translate_ps_constants[n])
                                                        {
                                                            if (ps_constants_type[n]!=1)
                                                            {
                                                                _sprintf(c.str,"float4x4 %s=float4x4(%sA,%sB,%sC,%sD);",ps_constants[n],
                                                                        ps_constants[n],ps_constants[n],ps_constants[n],ps_constants[n]);
                                                                s3psvpmetal[p].Add(c);
                                                            }
                                                        }
                                                    }
                                                }
                                                                                              
                                                strcpy(c.str,"PS_OUTPUT o;");
												s3psvp[p].InsertFirst(c);

												for (n=s3psvpmetal[p].Length()-1;n>=0;n--)
												{
													Code * cs=s3psvpmetal[p][n];
													s3psvp[p].InsertFirst(*cs);
												}

                                                strcpy(c.str,"return o;");
												s3psvp[p].Add(c);

                                                strcpy(c.str,"}");
												s3psvp[p].Add(c);

												c.str[0]='\0'; s3psvp[p].Add(c);
											}
											else
											{
                                                strcpy(c.str,"PS_OUTPUT o;");
												s3psvp[p].InsertFirst(c);
                                                strcpy(c.str,"{");
												s3psvp[p].InsertFirst(c);
												_sprintf(c.str,"PS_OUTPUT RenderPassPS%d(VS_OUTPUT%d i)",p,p);
												s3psvp[p].InsertFirst(c);
                                                strcpy(c.str,"return o;");
												s3psvp[p].Add(c);
                                                strcpy(c.str,"}");
												s3psvp[p].Add(c);
											}
										}
										else
										{
											if (metal==1)
											{
												// GLIO

                                                strcpy(c.str,"typedef struct");vpoutput.Add(c);
                                                strcpy(c.str,"{");vpoutput.Add(c);

                                                strcpy(c.str,"float4 p0 [[position]];");vpoutput.Add(c);
                                                
                                                if (pointcloud) { strcpy(c.str,"float PSize[[point_size]];");vpoutput.Add(c); }
			
												if (output&DIFFUSE) { strcpy(c.str,"float4 v0;");vpoutput.Add(c); }
												if (output&SPECULAR) { strcpy(c.str,"float4 v1;");vpoutput.Add(c); }
                                                /*
                                                 if (output&_TEX0) { _sprintf(c.str,"float4 t0 [[texturecoord0]];");vpoutput.Add(c); }
                                                 if (output&_TEX1) { _sprintf(c.str,"float4 t1 [[texturecoord1]];");vpoutput.Add(c); }
                                                 if (output&_TEX2) { _sprintf(c.str,"float4 t2 [[texturecoord2]];");vpoutput.Add(c); }
                                                 if (output&_TEX3) { _sprintf(c.str,"float4 t3 [[texturecoord3]];");vpoutput.Add(c); }
                                                 if (output&_TEX4) { _sprintf(c.str,"float4 t4 [[texturecoord4]];");vpoutput.Add(c); }
                                                 if (output&_TEX5) { _sprintf(c.str,"float4 t5 [[texturecoord5]];");vpoutput.Add(c); }
                                                 if (output&_TEX6) { _sprintf(c.str,"float4 t6 [[texturecoord6]];");vpoutput.Add(c); }
                                                 if (output&_TEX7) { _sprintf(c.str,"float4 t7 [[texturecoord7]];");vpoutput.Add(c); }
                                                 
                                                 /**/
		
                                                if (output&_TEX0) { strcpy(c.str,"float4 t0;");vpoutput.Add(c); }
                                                if (output&_TEX1) { strcpy(c.str,"float4 t1;");vpoutput.Add(c); }
                                                if (output&_TEX2) { strcpy(c.str,"float4 t2;");vpoutput.Add(c); }
                                                if (output&_TEX3) { strcpy(c.str,"float4 t3;");vpoutput.Add(c); }
                                                if (output&_TEX4) { strcpy(c.str,"float4 t4;");vpoutput.Add(c); }
                                                if (output&_TEX5) { strcpy(c.str,"float4 t5;");vpoutput.Add(c); }
                                                if (output&_TEX6) { strcpy(c.str,"float4 t6;");vpoutput.Add(c); }
                                                if (output&_TEX7) { strcpy(c.str,"float4 t7;");vpoutput.Add(c); }
                                                
                                                _sprintf(c.str,"} VS_OUTPUT%d;",p);vpoutput.Add(c);
												c.str[0]='\0';
												vpoutput.Add(c);
											}
											else
											{
												_sprintf(c.str,"struct VS_OUTPUT%d",p);vpoutput.Add(c);
                                                strcpy(c.str,"{");vpoutput.Add(c);
												if (shadermodel4) { strcpy(c.str,"float4 p0 : SV_POSITION;");vpoutput.Add(c); }
												else { strcpy(c.str,"float4 p0 : POSITION;");vpoutput.Add(c); }
			
												if (output&DIFFUSE) { strcpy(c.str,"float4 v0 : COLOR0;");vpoutput.Add(c); }
												if (output&SPECULAR) { strcpy(c.str,"float4 v1 : COLOR1;");vpoutput.Add(c); }
												if (output&_TEX0) { strcpy(c.str,"float4 t0 : TEXCOORD0;");vpoutput.Add(c); }
												if (output&_TEX1) { strcpy(c.str,"float4 t1 : TEXCOORD1;");vpoutput.Add(c); }
												if (output&_TEX2) { strcpy(c.str,"float4 t2 : TEXCOORD2;");vpoutput.Add(c); }
												if (output&_TEX3) { strcpy(c.str,"float4 t3 : TEXCOORD3;");vpoutput.Add(c); }
												if (output&_TEX4) { strcpy(c.str,"float4 t4 : TEXCOORD4;");vpoutput.Add(c); }
												if (output&_TEX5) { strcpy(c.str,"float4 t5 : TEXCOORD5;");vpoutput.Add(c); }
												if (output&_TEX6) { strcpy(c.str,"float4 t6 : TEXCOORD6;");vpoutput.Add(c); }
												if (output&_TEX7) { strcpy(c.str,"float4 t7 : TEXCOORD7;");vpoutput.Add(c); }

                                                strcpy(c.str,"};");vpoutput.Add(c);
												c.str[0]='\0';
												vpoutput.Add(c);
											}
										}
									}

									globalvar=false;

									if (script_shader[t][p])
									{
										ndefs=0;

										if (tech_spefs[t]&TECH_SHADERMODEL) selected_tech_with_ps=true;
										pixelshader=true;
										clear_registers();
										for (n=0;n<32;n++) tagregsave[n]=0;

										header_open_psh(p,DIRECT3D|fvf,output);

										parse_script_ps_functions(script_psfunctions[t][p]);

										compile_script(script_shader[t][p],&s3psvp[p]);
										unused_variables();

										if (shadermodel4)
										{
											if (RS.AlphaTest==_TRUE)
											{
                                                if (metal==1)
                                                {
													if (RS.AlphaRef==-666)
													{
														_sprintf(c.str,"if (o.Out.a<REFALPHA.x)");
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (o.Out.a>=REFALPHA.x)");
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (o.Out.a>REFALPHA.x)");
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (o.Out.a<=REFALPHA.x)");
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (o.Out.a<REFALPHA.x)");
                                                        
														s3psvp[p].Add(c);
                                                        
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);
													}
													else
													{
														float valrefalpha=16.0f/255.0f;
														if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;
                                                    
														_sprintf(c.str,"if (o.Out.a<%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (o.Out.a>=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (o.Out.a>%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (o.Out.a<=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (o.Out.a<%3.3ff)",valrefalpha);
														s3psvp[p].Add(c);
                                                    
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);
													}
                                                }
                                                else
                                                {
													if (RS.AlphaRef==-666)
													{
														_sprintf(c.str,"if (r0.a<REFALPHA.x)");
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (r0.a>=REFALPHA.x)");
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (r0.a>REFALPHA.x)");
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (r0.a<=REFALPHA.x)");
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (r0.a<REFALPHA.x)");
                                                        
														s3psvp[p].Add(c);
                                                        
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);
													}
													else
													{
														float valrefalpha=16.0f/255.0f;
														if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;
                                                    
														_sprintf(c.str,"if (r0.a<%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (r0.a>=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (r0.a>%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (r0.a<=%3.3ff)",valrefalpha);
														if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (r0.a<%3.3ff)",valrefalpha);
														s3psvp[p].Add(c);
                                                    
														strcpy(c.str,"{"); s3psvp[p].Add(c);
														_sprintf(c.str,"%s;",sDiscardAPI); s3psvp[p].Add(c);
														strcpy(c.str,"}"); s3psvp[p].Add(c);
													}
                                                }
											}
										}

										if (metal==0)  // LOOK
										{
                                            strcpy(c.str,"o.Out = r0;");
											s3psvp[p].Add(c);
										}

										_sprintf(c.str,"");
										s3psvp[p].InsertFirst(c);

										for (n=31;n>=0;n--)
											if (tagregsave[n])
											{
                                                if (IsRegisterIn(n,s3psvp[p]))
                                                {
                                                    _sprintf(c.str,"float4 r%d;",n);
                                                    s3psvp[p].InsertFirst(c);
                                                }
											}

										_sprintf(c.str,"");
										s3psvp[p].InsertFirst(c);

										numberpsfn[p]=numpsfn;

										if (script_psfunctions[t][p])
										{
											psfncodeline[0][p]=0;
											for (int kk=0;kk<numpsfn;kk++)
											{
												for (n = 0; n<32; n++) tagregsave[n] = 0;
												for (n = 0; n<32; n++) tagreg[n] = 0;
												for (n = 0; n<32; n++) tagtempreg[n] = 0;

												_sprintf(&psfnname[kk][p][0],compile_psfunction(&psfn[kk], &vptmp));
												unused_variables();

												for (n=31;n>=0;n--)
													if (tagregsave[n])
													{
                                                        if (IsRegisterIn(n,vptmp))
                                                        {
                                                            _sprintf(c.str,"float4 r%d;",n);
                                                            vptmp.InsertFirst(c);
                                                        }
													}

                                                strcpy(c.str,"{");
												vptmp.InsertFirst(c);
                                                strcpy(c.str,"}");
												vptmp.Add(c);
												_sprintf(c.str,entete);
												vptmp.InsertFirst(c);

												Code *wc=vptmp.GetFirst();
												while (wc)
												{
													s3psfnvp[p].Add(*wc);
													wc=vptmp.GetNext();
												}

												psfncodeline[kk+1][p]=psfncodeline[kk][p]+vptmp.Length()+1;

												vptmp.Free();

                                                strcpy(c.str,"");
												s3psfnvp[p].Add(c);
											}
										}

										if (metal==0)
										{
                                            if (FSR==1)
                                            {
                                                char *sfsr=shader_functions_FSR_header;
                                                int ps=0;
                                                char *pstmp=str_parse_rln2(sfsr,&ps);
                                                while (pstmp)
                                                {
                                                    if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                }
                                                    
                                                sfsr=shader_functions_FSR_EASU;
                                                ps=0;
                                                pstmp=str_parse_rln2(sfsr,&ps);
                                                while (pstmp)
                                                {
                                                    if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                }
                                            }

                                            if (FSR==2)
                                            {
                                                char *sfsr=shader_functions_FSR_header;
                                                int ps=0;
                                                char *pstmp=str_parse_rln2(sfsr,&ps);
                                                while (pstmp)
                                                {
                                                    if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                }
                                                    
                                                sfsr=shader_functions_FSR_RCAS;
                                                ps=0;
                                                pstmp=str_parse_rln2(sfsr,&ps);
                                                while (pstmp)
                                                {
                                                    if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                }
                                            }

											if ((FSRSample0)||(FSRSample1))
											{
                                                char *sfsr=shader_functions_FSR_header_01;
                                                int ps=0;
                                                char *pstmp=str_parse_rln2(sfsr,&ps);
                                                while (pstmp)
                                                {
                                                    if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                }
                                                   
												if ((FSRSample0)&&(RenderState[0][0].Texture[0]))
												{
													sfsr=shader_functions_FSR_RCAS_0;
													ps=0;
													pstmp=str_parse_rln2(sfsr,&ps);
													while (pstmp)
													{
														if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
														pstmp=str_parse_rln2(sfsr,&ps);
													}
												}

												if ((SizingTexture0)&&(FSRSample0)&&(RenderState[0][0].Texture[0]))
												{
													char tempss[1024];
													_sprintf(tempss,shader_functions_FSR_RCAS_0_warp_samplebump,valueFSRRCASSample);
													sfsr=tempss;
													ps=0;
													pstmp=str_parse_rln2(sfsr,&ps);
													while (pstmp)
													{
														if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
														pstmp=str_parse_rln2(sfsr,&ps);
													}
												}

												if ((FSRSample1)&&(RenderState[0][0].Texture[1]))
												{
													sfsr=shader_functions_FSR_RCAS_1;
													ps=0;
													pstmp=str_parse_rln2(sfsr,&ps);
													while (pstmp)
													{
														if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psfnvp[p].Add(c); }
														pstmp=str_parse_rln2(sfsr,&ps);
													}
												}
											}
										}
				
										pixelshader=false;

										if ((p==0)&&(metal==0))
										{
											if (globalvar)
											{
												if (output&_TEX7) { strcpy(c.str,"static float4 it7;"); vpcst.Add(c); }
												if (output&_TEX6) { strcpy(c.str,"static float4 it6;"); vpcst.Add(c); }
												if (output&_TEX5) { strcpy(c.str,"static float4 it5;"); vpcst.Add(c); }
												if (output&_TEX4) { strcpy(c.str,"static float4 it4;"); vpcst.Add(c); }
												if (output&_TEX3) { strcpy(c.str,"static float4 it3;"); vpcst.Add(c); }
												if (output&_TEX2) { strcpy(c.str,"static float4 it2;"); vpcst.Add(c); }
												if (output&_TEX1) { strcpy(c.str,"static float4 it1;"); vpcst.Add(c); }
												if (output&_TEX0) { strcpy(c.str,"static float4 it0;"); vpcst.Add(c); }
												if (output&SPECULAR) { strcpy(c.str,"static float4 iv1;"); vpcst.Add(c); }
												if (output&DIFFUSE) { strcpy(c.str,"static float4 iv0;"); vpcst.Add(c); }
											}
											if (modifyZ) { strcpy(c.str,"static float4 ip0;"); vpcst.Add(c); }
										}

										if (metal==0)
										{
											if (globalvar)
											{
												if (output&_TEX7) { strcpy(c.str,"it7 = i.t7;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX6) { strcpy(c.str,"it6 = i.t6;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX5) { strcpy(c.str,"it5 = i.t5;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX4) { strcpy(c.str,"it4 = i.t4;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX3) { strcpy(c.str,"it3 = i.t3;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX2) { strcpy(c.str,"it2 = i.t2;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX1) { strcpy(c.str,"it1 = i.t1;"); s3psvp[p].InsertFirst(c); }
												if (output&_TEX0) { strcpy(c.str,"it0 = i.t0;"); s3psvp[p].InsertFirst(c); }
												if (output&SPECULAR) { strcpy(c.str,"iv1 = i.v1;"); s3psvp[p].InsertFirst(c); }
												if (output&DIFFUSE) { strcpy(c.str,"iv0 = i.v0;"); s3psvp[p].InsertFirst(c); }
											}

											if (modifyZ) { strcpy(c.str,"ip0 = i.p0;"); s3psvp[p].InsertFirst(c); }
										}

										int lastnt=-1;

										if (metal==1)
										{
											if ((numpsfn==0)&&(!isFSR))
											{
                                                _sprintf(c.str,"fragment float4 RenderPassPS%d(VS_OUTPUT%d pixel[[stage_in]],",p,p);
												if (modifyZ) _sprintf(c.str,"fragment PS_OUTPUT RenderPassPS%d(VS_OUTPUT%d pixel[[stage_in]],",p,p);
												s3psvpmetal[p].Add(c);
												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"                             constant constants_ps& constants[[ buffer(0) ]],");
													s3psvpmetal[p].Add(c);
												}

												int nt=0;
												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"                             texture2d<float> %s[[texture(%d)]],",texture[n],nt);
														s3psvpmetal[p].Add(c);
														nt++;
													}
												}

                                                strcpy(c.str,"                                sampler s0[[sampler(0)]],");
												s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"                                sampler s1[[sampler(1)]])");
												s3psvpmetal[p].Add(c);

												if ((npsvectors>0)||(npsmatrices>0))
												{
													for (n=0;n<npsvectors;n++)
													{
														_sprintf(c.str,"#define %s constants.%s",psvectors[n],psvectors[n]);
														s3psvpmetal[p].Add(c);
													}
													for (n=0;n<npsmatrices;n++)
													{
														_sprintf(c.str,"#define %s constants.%s",psmatrices[n],psmatrices[n]);
														s3psvpmetal[p].Add(c);
													}
												}

                                                strcpy(c.str,"{");s3psvpmetal[p].Add(c);

                                                strcpy(c.str,"PS_OUTPUT o;");
												s3psvpmetal[p].Add(c);
                                            
												for (n=0;n<MAX_ENTRIES;n++)
												{
													if (ps_constants[n])
													{
														if (translate_ps_constants[n])
														{
															if (ps_constants_type[n]!=1)
															{
																_sprintf(c.str,"%s=float4x4(%sA,%sB,%sC,%sD);",ps_constants[n],
																		ps_constants[n],ps_constants[n],ps_constants[n],ps_constants[n]);
																s3psvpmetal[p].Add(c);
															}
														}
													}
												}
                                            
												for (n=s3psvpmetal[p].Length()-1;n>=0;n--)
												{
													Code * cs=s3psvpmetal[p][n];
													s3psvp[p].InsertFirst(*cs);
												}

                                                strcpy(c.str,"return float4(o.Out);");
												if (modifyZ) strcpy(c.str,"return o;");
												s3psvp[p].Add(c);

                                                strcpy(c.str,"}");
												s3psvp[p].Add(c);

												c.str[0]='\0'; s3psvpmetal[p].Add(c);

											}
											else
											{
												_sprintf(c.str,"struct PixelShader%d",p);s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"{");s3psvpmetal[p].Add(c);

												_sprintf(c.str,"thread VS_OUTPUT%d &pixel;",p);s3psvpmetal[p].Add(c);

												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"constant constants_ps& ps;");
													s3psvpmetal[p].Add(c);
												}
                                            
												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"texture2d<float> %s;",texture[n]);
														s3psvpmetal[p].Add(c);
														lastnt=n;
													}
												}

												for (n=0;n<npsvectors;n++)
												{
													_sprintf(c.str,"#define %s ps.%s",psvectors[n],psvectors[n]);
													s3psvpmetal[p].Add(c);
												}

												for (n=0;n<npsmatrices;n++)
												{
													_sprintf(c.str,"#define %s ps.%s",psmatrices[n],psmatrices[n]);
													s3psvpmetal[p].Add(c);
												}

												strcpy(c.str,"sampler s0;");
												s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"sampler s1;");
												s3psvpmetal[p].Add(c);
                                            
												for (n=0;n<MAX_ENTRIES;n++)
												{
													if (ps_constants[n])
													{
														if (translate_ps_constants[n])
														{
															if (ps_constants_type[n]!=1)
															{
																_sprintf(c.str,"float4x4  %s;",ps_constants[n]);
																s3psvpmetal[p].Add(c);
															}
														}
													}
												}

												c.str[0]='\0'; s3psvpmetal[p].Add(c);

												_sprintf(c.str,"PixelShader%d(thread VS_OUTPUT%d& in,",p,p);
												s3psvpmetal[p].Add(c);

												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"constant constants_ps& c,");
													s3psvpmetal[p].Add(c);
												}

												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"thread texture2d<float>& tex%d,",n);
														s3psvpmetal[p].Add(c);
													}
												}

                                                strcpy(c.str,"thread sampler& smp0,");
												s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"thread sampler& smp1) :");
												s3psvpmetal[p].Add(c);

                                                strcpy(c.str,"pixel(in),");
												s3psvpmetal[p].Add(c);

												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"ps(c),");
													s3psvpmetal[p].Add(c);
												}

												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"%s(tex%d),",texture[n],n);
														s3psvpmetal[p].Add(c);
													}
												}

                                                strcpy(c.str,"s0(smp0),");
												s3psvpmetal[p].Add(c);
                                                strcpy(c.str,"s1(smp1)");
												s3psvpmetal[p].Add(c);

                                                strcpy(c.str,"{");s3psvpmetal[p].Add(c);
                                            
												for (n=0;n<MAX_ENTRIES;n++)
												{
													if (ps_constants[n])
													{
														if (translate_ps_constants[n])
														{
															if (ps_constants_type[n]!=1)
															{
																_sprintf(c.str,"%s=float4x4(%sA,%sB,%sC,%sD);",ps_constants[n],
																		ps_constants[n],ps_constants[n],ps_constants[n],ps_constants[n]);
																s3psvpmetal[p].Add(c);
															}
														}
													}
												}
                                            

                                                strcpy(c.str,"};");s3psvpmetal[p].Add(c);

												c.str[0]='\0'; s3psvpmetal[p].Add(c);

												Code *cc=s3psfnvp[p].GetFirst();
												while (cc)
												{
													s3psvpmetal[p].Add(*cc);
													cc=s3psfnvp[p].GetNext();
												}

                                                c.str[0]='\0'; s3psvpmetal[p].Add(c);
                                                
                                                if (FSR==1)
                                                {
                                                    char *sfsr=shader_functions_FSR_header;
                                                    int ps=0;
                                                    char *pstmp=str_parse_rln2(sfsr,&ps);
                                                    while (pstmp)
                                                    {
                                                        if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
                                                        pstmp=str_parse_rln2(sfsr,&ps);
                                                    }
                                                    
                                                    sfsr=shader_functions_FSR_EASU;
                                                    ps=0;
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                    while (pstmp)
                                                    {
                                                        if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
                                                        pstmp=str_parse_rln2(sfsr,&ps);
                                                    }
                                                }

                                                if (FSR==2)
                                                {
                                                    char *sfsr=shader_functions_FSR_header;
                                                    int ps=0;
                                                    char *pstmp=str_parse_rln2(sfsr,&ps);
                                                    while (pstmp)
                                                    {
                                                        if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
                                                        pstmp=str_parse_rln2(sfsr,&ps);
                                                    }
                                                    
                                                    sfsr=shader_functions_FSR_RCAS;
                                                    ps=0;
                                                    pstmp=str_parse_rln2(sfsr,&ps);
                                                    while (pstmp)
                                                    {
                                                        if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
                                                        pstmp=str_parse_rln2(sfsr,&ps);
                                                    }
                                                }

                                                if ((FSRSample0)||(FSRSample1))
                                                {
                                                    char *sfsr=shader_functions_FSR_header_01;
                                                    int ps=0;
                                                    char *pstmp=str_parse_rln2(sfsr,&ps);
                                                    while (pstmp)
                                                    {
                                                        if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
                                                        pstmp=str_parse_rln2(sfsr,&ps);
                                                    }
                                                    
													if ((FSRSample0)&&(RenderState[0][0].Texture[0]))
													{
														sfsr=shader_functions_FSR_RCAS_0;
														ps=0;
														pstmp=str_parse_rln2(sfsr,&ps);
														while (pstmp)
														{
															if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
															pstmp=str_parse_rln2(sfsr,&ps);
														}
													}

													if ((SizingTexture0)&&(FSRSample0)&&(RenderState[0][0].Texture[0]))
													{
														char tempss[1024];
														_sprintf(tempss,shader_functions_FSR_RCAS_0_warp_samplebump,valueFSRRCASSample);
														sfsr=tempss;
														ps=0;
														pstmp=str_parse_rln2(sfsr,&ps);
														while (pstmp)
														{
															if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
															pstmp=str_parse_rln2(sfsr,&ps);
														}
													}

													if ((FSRSample1)&&(RenderState[0][0].Texture[1]))
													{
														sfsr=shader_functions_FSR_RCAS_1;
														ps=0;
														pstmp=str_parse_rln2(sfsr,&ps);
														while (pstmp)
														{
															if (strlen(pstmp)>0) { strcpy(c.str,pstmp); s3psvpmetal[p].Add(c); }
															pstmp=str_parse_rln2(sfsr,&ps);
														}
													}
												}

												c.str[0]='\0'; s3psvpmetal[p].Add(c);

                                                strcpy(c.str,"PS_OUTPUT main()");s3vpmetal[p].Add(c);
												s3psvpmetal[p].Add(c);

                                                strcpy(c.str,"");
												s3psvp[p].InsertFirst(c);

                                                strcpy(c.str,"PS_OUTPUT o;");
												s3psvp[p].InsertFirst(c);
                                                strcpy(c.str,"{");
												s3psvp[p].InsertFirst(c);

												for (n=s3psvpmetal[p].Length()-1;n>=0;n--)
												{
													Code * cs=s3psvpmetal[p][n];
													s3psvp[p].InsertFirst(*cs);
												}

                                                strcpy(c.str,"return o;");
												s3psvp[p].Add(c);

                                                strcpy(c.str,"}");
												s3psvp[p].Add(c);

                                                strcpy(c.str,"};");
												s3psvp[p].Add(c);

												c.str[0]='\0'; s3psvp[p].Add(c);

                                                _sprintf(c.str,"fragment float4 RenderPassPS%d(VS_OUTPUT%d in [[stage_in]],",p,p);
                                                if (modifyZ) _sprintf(c.str,"fragment PS_OUTPUT RenderPassPS%d(VS_OUTPUT%d in [[stage_in]],",p,p);
                                                s3psvp[p].Add(c);
												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"                             constant constants_ps& constants[[ buffer(0) ]],");
													s3psvp[p].Add(c);
												}

												int nt=0;
												for (n=0;n<16;n++)
												{
													if (texture[n])
													{
														_sprintf(c.str,"                             texture2d<float> %s[[texture(%d)]],",texture[n],nt);
														s3psvp[p].Add(c);
														nt++;
													}
												}

                                                strcpy(c.str,"                                sampler texsampler0[[sampler(0)]],");
												s3psvp[p].Add(c);
                                                strcpy(c.str,"                                sampler texsampler1[[sampler(1)]])");
												s3psvp[p].Add(c);

                                                strcpy(c.str,"{"); s3psvp[p].Add(c);

												_sprintf(c.str,"PixelShader%d ps(in,",p); s3psvp[p].Add(c);

												if ((npsvectors>0)||(npsmatrices>0))
												{
                                                    strcpy(c.str,"constants,"); s3psvp[p].Add(c);
												}

												if (lastnt>=0)
												{
													for (n=0;n<16;n++)
													{
														if (texture[n])
														{
															_sprintf(c.str,"%s,",texture[n]);
															s3psvp[p].Add(c);
														}
													}
												}

                                                strcpy(c.str,"texsampler0,texsampler1);");
												s3psvp[p].Add(c);

												if (modifyZ)
                                                {
                                                    strcpy(c.str,"return ps.main();");
                                                    s3psvp[p].Add(c);
                                                }
                                                else
                                                {
                                                    strcpy(c.str,"PS_OUTPUT o=ps.main();");
                                                    s3psvp[p].Add(c);
                                                    
                                                    strcpy(c.str,"return float4(o.Out);");
                                                    s3psvp[p].Add(c);
                                                }
 
                                                strcpy(c.str,"}"); s3psvp[p].Add(c);
											}
										}
										else
										{
                                            strcpy(c.str,"return o;");
											s3psvp[p].Add(c);
                                            strcpy(c.str,"}");
											s3psvp[p].Add(c);

                                            strcpy(c.str,"PS_OUTPUT o;");
											s3psvp[p].InsertFirst(c);
                                            strcpy(c.str,"{");
											s3psvp[p].InsertFirst(c);
											_sprintf(c.str,"PS_OUTPUT RenderPassPS%d(VS_OUTPUT%d i)",p,p);
											s3psvp[p].InsertFirst(c);
										}
									}
									else
									if (!shadermodel4)
									{
										if (metal==1)
										{
                                            strcpy(c.str,"typedef struct");vpoutput.Add(c);
                                            strcpy(c.str,"{");vpoutput.Add(c);
                                            strcpy(c.str,"float4 p0;");vpoutput.Add(c);
			
											if (output&DIFFUSE) { strcpy(c.str,"float4 v0;");vpoutput.Add(c); }
											if (output&SPECULAR) { strcpy(c.str,"float4 v1;");vpoutput.Add(c); }
											if (output&_TEX0) { strcpy(c.str,"float4 t0;");vpoutput.Add(c); }
											if (output&_TEX1) { strcpy(c.str,"float4 t1;");vpoutput.Add(c); }
											if (output&_TEX2) { strcpy(c.str,"float4 t2;");vpoutput.Add(c); }
											if (output&_TEX3) { strcpy(c.str,"float4 t3;");vpoutput.Add(c); }
											if (output&_TEX4) { strcpy(c.str,"float4 t4;");vpoutput.Add(c); }
											if (output&_TEX5) { strcpy(c.str,"float4 t5;");vpoutput.Add(c); }
											if (output&_TEX6) { strcpy(c.str,"float4 t6;");vpoutput.Add(c); }
											if (output&_TEX7) { strcpy(c.str,"float4 t7;");vpoutput.Add(c); }

											_sprintf(c.str,"} VS_OUTPUT%d;",p);vpoutput.Add(c);
											c.str[0]='\0';
											vpoutput.Add(c);
										}
										else
										{
											_sprintf(c.str,"struct VS_OUTPUT%d",p);vpoutput.Add(c);
                                            strcpy(c.str,"{");vpoutput.Add(c);
											if (shadermodel4) { strcpy(c.str,"float4 p0 : SV_POSITION;");vpoutput.Add(c); }
											else { strcpy(c.str,"float4 p0 : POSITION;");vpoutput.Add(c); }
			
											if (output&DIFFUSE) { strcpy(c.str,"float4 v0 : COLOR0;");vpoutput.Add(c); }
											if (output&SPECULAR) { strcpy(c.str,"float4 v1 : COLOR1;");vpoutput.Add(c); }
											if (output&_TEX0) { strcpy(c.str,"float4 t0 : TEXCOORD0;");vpoutput.Add(c); }
											if (output&_TEX1) { strcpy(c.str,"float4 t1 : TEXCOORD1;");vpoutput.Add(c); }
											if (output&_TEX2) { strcpy(c.str,"float4 t2 : TEXCOORD2;");vpoutput.Add(c); }
											if (output&_TEX3) { strcpy(c.str,"float4 t3 : TEXCOORD3;");vpoutput.Add(c); }
											if (output&_TEX4) { strcpy(c.str,"float4 t4 : TEXCOORD4;");vpoutput.Add(c); }
											if (output&_TEX5) { strcpy(c.str,"float4 t5 : TEXCOORD5;");vpoutput.Add(c); }
											if (output&_TEX6) { strcpy(c.str,"float4 t6 : TEXCOORD6;");vpoutput.Add(c); }
											if (output&_TEX7) { strcpy(c.str,"float4 t7 : TEXCOORD7;");vpoutput.Add(c); }

                                            strcpy(c.str,"};");vpoutput.Add(c);
											c.str[0]='\0';
											vpoutput.Add(c);
										}
									}
								}
								else
								if (api==2)		// emulated
								{
									emvp[p]=new CEmulatedVertexProgram;
									emvp[p]->Init();
									em=emvp[p];

									for (int cn=0;cn<last_constant;cn++)
									{
										if (translate_constants[cn])
										{
											em->Set(cn,float_translate_constants[cn][0],float_translate_constants[cn][1],float_translate_constants[cn][2],float_translate_constants[cn][3]);
										}
										else
										{
											if (l_constants[cn]==_VECTOR) em->Set(cn,0,0,0,0);
										}
									}

									ndefs=0;

									if (script_position[t][p]) compile_script(script_position[t][p],&vp);
									if (script_diffuse[t][p]) compile_script(script_diffuse[t][p],&vp);
									if (script_specular[t][p]) compile_script(script_specular[t][p],&vp);
									if (script_mapping0[t][p]) compile_script(script_mapping0[t][p],&vp);
									if (script_mapping1[t][p]) compile_script(script_mapping1[t][p],&vp);
									if (script_mapping2[t][p]) compile_script(script_mapping2[t][p],&vp);
									if (script_mapping3[t][p]) compile_script(script_mapping3[t][p],&vp);
									if (script_mapping4[t][p]) compile_script(script_mapping4[t][p],&vp);
									if (script_mapping5[t][p]) compile_script(script_mapping5[t][p],&vp);
									if (script_mapping6[t][p]) compile_script(script_mapping6[t][p],&vp);
									if (script_mapping7[t][p]) compile_script(script_mapping7[t][p],&vp);

									unused_variables();

								}
								else
								{
									// API GL

									last_register=0;
									max_temporary_register=0;
                                    ndefs=0;

									parse_script_vs_functions(script_vsfunctions[t][p]);

									groups[t][p][0]=glvp[p].Length();
									if (script_position[t][p]) compile_script(script_position[t][p],&glvp[p]);
									groups[t][p][1]=glvp[p].Length();
									if (script_diffuse[t][p]) compile_script(script_diffuse[t][p],&glvp[p]);
									groups[t][p][2]=glvp[p].Length();
									if (script_specular[t][p]) compile_script(script_specular[t][p],&glvp[p]);
									groups[t][p][3]=glvp[p].Length();
									if (script_mapping0[t][p]) compile_script(script_mapping0[t][p],&glvp[p]);
									groups[t][p][4]=glvp[p].Length();
									if (script_mapping1[t][p]) compile_script(script_mapping1[t][p],&glvp[p]);
									groups[t][p][5]=glvp[p].Length();
									if (script_mapping2[t][p]) compile_script(script_mapping2[t][p],&glvp[p]);
									groups[t][p][6]=glvp[p].Length();
									if (script_mapping3[t][p]) compile_script(script_mapping3[t][p],&glvp[p]);
									groups[t][p][7]=glvp[p].Length();
									if (script_mapping4[t][p]) compile_script(script_mapping4[t][p],&glvp[p]);

									groups[t][p][8]=glvp[p].Length();
									if (script_mapping5[t][p]) compile_script(script_mapping5[t][p],&glvp[p]);
									groups[t][p][9]=glvp[p].Length();
									if (script_mapping6[t][p]) compile_script(script_mapping6[t][p],&glvp[p]);
									groups[t][p][10]=glvp[p].Length();
									if (script_mapping7[t][p]) compile_script(script_mapping7[t][p],&glvp[p]);

									unused_variables();

									tagregsave[29]=1;
									tagregsave[30]=1;
									tagregsave[31]=1;

									for (n=0;n<32;n++)
									if (tagregsave[n])
									{
										_sprintf(c.str,"vec4 r%d;",n);
										glvar[p].Add(c);
									}

									if ((script_vsfunctions[t][p])&&(shadermodel))
									{
                                        strcpy(c.str,"");
										glfn.Add(c);
										for (int kk=0;kk<numvsfn;kk++)
										{
											last_register=0;
											last_temp_register=0;
                                            ndefs=0;
                                            
											compile_function(&vsfn[kk],&glfn);
											unused_variables();

                                            strcpy(c.str,"");
											glfn.Add(c);
										}
									}

									for (n=0;n<12;n++)
									{
										if (!shadermodel)
										{
											_sprintf(c.str,"TEMP r%d",n);
											gltemp[p].Add(c);
										}
									}

									c.str[0]='\0';
									gltemp[p].Add(c);

									groups[t][p][5]=glvp[p].Length();
									if (script_shader[t][p])
									{
										ndefs=0;

										if (tech_spefs[t]&TECH_SHADERMODEL) selected_tech_with_ps=true;

										pixelshader=true;
										clear_registers();
										for (n=0;n<32;n++) tagregsave[n]=0;
										for (n=0;n<32;n++) tagreg[n]=0;
										for (n=0;n<32;n++) tagtempreg[n]=0;

										last_register=0;
										last_temp_register=0;
										max_temporary_register=0;

										parse_script_ps_functions(script_psfunctions[t][p]);

										compile_script(script_shader[t][p],&glps[p]);
										unused_variables();

										pixelshader=false;

										if (shadermodel)
										{
											if (RS.AlphaTest==_TRUE)
											{
												if (RS.AlphaRef==-666)
												{
													_sprintf(c.str,"if (%s.a<REFALPHA.x)",var("Out"));
													if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (%s.a>=REFALPHA.x)",var("Out"));
													if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (%s.a>REFALPHA.x)",var("Out"));
													if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (%s.a<=REFALPHA.x)",var("Out"));
													if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (%s.a<REFALPHA.x)",var("Out"));
												
													glps[p].Add(c);

													_sprintf(c.str,"{"); glps[p].Add(c);
													_sprintf(c.str,sDiscardAPI); glps[p].Add(c);
													_sprintf(c.str,"}"); glps[p].Add(c);
												}
												else
												{
													float valrefalpha=16.0f/255.0f;
													if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;

													_sprintf(c.str,"if (%s.a<%3.3f)",var("Out"),valrefalpha);
													if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (%s.a>=%3.3f)",var("Out"),valrefalpha);
													if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (%s.a>%3.3f)",var("Out"),valrefalpha);
													if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (%s.a<=%3.3f)",var("Out"),valrefalpha);
													if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (%s.a<%3.3f)",var("Out"),valrefalpha);
												
													glps[p].Add(c);

													_sprintf(c.str,"{"); glps[p].Add(c);
													_sprintf(c.str,sDiscardAPI); glps[p].Add(c);
													_sprintf(c.str,"}"); glps[p].Add(c);
												}
											}

											_sprintf(c.str,"gl_FragColor = %s",var("Out"));
											glps[p].Add(c);
										}
	
										if (last_temp_register>max_temporary_register) max_temporary_register=last_temp_register;

										if (last_register>max_temporary_register) max_temporary_register=last_register;

										_sprintf(c.str,"");
										glps[p].InsertFirst(c);

										for (n=32-1;n>=0;n--)
										if (tagregsave[n])
										{
											if (shadermodel)
											{
												_sprintf(c.str,"vec4 r%d",n);
												glps[p].InsertFirst(c);
											}
											else
											{
												if (n < 12)
												{
													_sprintf(c.str, "TEMP r%d", n);
													glps[p].InsertFirst(c);
												}
											}
										}

                                        strcpy(c.str,"");
										glps[p].InsertFirst(c);

										if ((script_psfunctions[t][p])&&(shadermodel))
										{
                                            strcpy(c.str,"");
											glfnps.Add(c);

											pixelshader=true;
											for (int kk=0;kk<numpsfn;kk++)
											{
												last_register=0;
												last_temp_register=0;
												ndefs=0;

												compile_psfunction(&psfn[kk],&glfnps);
												unused_variables();

                                                strcpy(c.str,"");
												glfnps.Add(c);
											}
											pixelshader=false;
										}
									}
#if defined(API3D_OPENGL20)||defined(API3D_OPENGL)
									else
									{
										// RS20
										selected_tech_with_ps=true;

										compile_render_state_pixelshader(t,p,&glps[p]);

										if (RS.AlphaTest==_TRUE)
										{
											float valrefalpha=16.0f/255.0f;
											if (RS.AlphaRef!=_UNDEFINED) valrefalpha=(float)RS.AlphaRef/255.0f;

											_sprintf(c.str,"if (%s.a<%3.3f)","r0",valrefalpha);
											if (RS.AlphaFunc==_LESS) _sprintf(c.str,"if (%s.a>=%3.3f)","r0",valrefalpha);
											if (RS.AlphaFunc==_LEQUAL) _sprintf(c.str,"if (%s.a>%3.3f)","r0",valrefalpha);
											if (RS.AlphaFunc==_GREATER) _sprintf(c.str,"if (%s.a<=%3.3f)","r0",valrefalpha);
											if (RS.AlphaFunc==_GEQUAL) _sprintf(c.str,"if (%s.a<%3.3f)","r0",valrefalpha);
												
											glps[p].Add(c);

                                            strcpy(c.str,"{"); glps[p].Add(c);
                                            strcpy(c.str,sDiscardAPI); glps[p].Add(c);
                                            strcpy(c.str,"}"); glps[p].Add(c);

										}

                                        strcpy(c.str,"gl_FragColor = r0");
										glps[p].Add(c);

                                        strcpy(c.str,"vec4 r4");
										glps[p].InsertFirst(c);
                                        strcpy(c.str,"vec4 r3");
										glps[p].InsertFirst(c);
                                        strcpy(c.str,"vec4 r2");
										glps[p].InsertFirst(c);
                                        strcpy(c.str,"vec4 r1");
										glps[p].InsertFirst(c);
                                        strcpy(c.str,"vec4 r0");
										glps[p].InsertFirst(c);
                                        strcpy(c.str,"");
										glps[p].InsertFirst(c);
									}
#endif
								}
							}

							if (api==0)
							{
                                strcpy(c.str,"}");
								vp.Add(c);
							}
						}
					}
				}
			}
		}
		else break;
	}

	c.str[0]='\0';
	if (api==1) glheader.Add(c);
	if (api==0) vp.Add(c);
	
	for (t=0;t<4;t++)
		for (p=0;p<4;p++)
		{
			if (script_position[t][p]) free(script_position[t][p]);
			if (script_diffuse[t][p]) free(script_diffuse[t][p]);
			if (script_specular[t][p]) free(script_specular[t][p]);
			if (script_mapping0[t][p]) free(script_mapping0[t][p]);
			if (script_mapping1[t][p]) free(script_mapping1[t][p]);
			if (script_mapping2[t][p]) free(script_mapping2[t][p]);
			if (script_mapping3[t][p]) free(script_mapping3[t][p]);
			if (script_mapping4[t][p]) free(script_mapping4[t][p]);
			if (script_mapping5[t][p]) free(script_mapping5[t][p]);
			if (script_mapping6[t][p]) free(script_mapping6[t][p]);
			if (script_mapping7[t][p]) free(script_mapping7[t][p]);
			if (script_shader[t][p]) free(script_shader[t][p]);
		}

	free(script);
/*
	if (REPERROR) printf("REP ERROR\n");
	if (TYPEERROR) printf("TYPE ERROR\n");
	if (SYNTAXERROR) printf("SYNTAX ERROR\n");
	if (DEFINEERROR) printf("DEFINE ERROR\n");
	if ((UNUSEDERROR)&&(STRICTSHADER)) printf("STRICT ERROR\n");
	if (PARAMERROR) printf("PARAM ERROR\n");
/**/

	if (REPERROR) return false;
	if (TYPEERROR) return false;
	if (SYNTAXERROR) return false;
	if (DEFINEERROR) return false;
	if ((UNUSEDERROR)&&(STRICTSHADER)) return false;
	if (PARAMERROR) return false;

	return success;
}


char* CVertexProgram::get_string_vsh(int pass)
{
	Code *c;
	Code cc;
	Code blank;
	unsigned int len,pos;
	char *buf;
	int level;
	int n,p;
	int res,cpt;
	unsigned int size;

	blank.str[0]='\0';

	if ((shadermodel3)&&(api==0))
	{
		int codeline=0;
		vp.Free();

		c=vpcst.GetFirst();
		while (c)
		{
			vp.Add(*c);
			codeline++;
			c=vpcst.GetNext();
		}

		vp.Add(blank);
		codeline++;

		c=vpoutput.GetFirst();
		while (c)
		{
			vp.Add(*c);
			codeline++;
			c=vpoutput.GetNext();
		}

		vp.Add(blank);
		codeline++;

		c=vpinput.GetFirst();
		while (c)
		{
			vp.Add(*c);
			codeline++;
			c=vpinput.GetNext();
		}

		vp.Add(blank);
		codeline++;

		for (p=0;p<get_pass_count();p++)
		{
			if (metal==0)
			{
				for (n=0;n<numvsfn;n++) vsfncodeline[n][p]+=codeline;

				c=s3fnvp[p].GetFirst();
				while (c)
				{
					vp.Add(*c);
					codeline++;
					c=s3fnvp[p].GetNext();
				}

				vp.Add(blank);
				codeline++;
			}

			c=s3vp[p].GetFirst();
			while (c)
			{
				vp.Add(*c);
				if (str_match(c->str,"RenderPassVS")) vscodeline[p]=codeline;
				codeline++;
				c=s3vp[p].GetNext();
			}

			vp.Add(blank);
			codeline++;

			if (metal==0)
			{
				for (n=0;n<numpsfn;n++) psfncodeline[n][p]+=codeline;

				c=s3psfnvp[p].GetFirst();
				while (c)
				{
					vp.Add(*c);
					codeline++;
					c=s3psfnvp[p].GetNext();
				}

				vp.Add(blank);
				codeline++;
			}

			c=s3psvp[p].GetFirst();
			while (c)
			{
				vp.Add(*c);
				if (str_match(c->str,"RenderPassPS")) pscodeline[p]=codeline;
				codeline++;
				c=s3psvp[p].GetNext();
			}

			vp.Add(blank);
			codeline++;
		}

		vp.Add(blank);

		if (metal==0)
		{
			if (shadermodel4) _sprintf(cc.str,"technique10 T0");
			else _sprintf(cc.str,"TECHNIQUE T0");
			vp.Add(cc);
			_sprintf(cc.str,"{");
			vp.Add(cc);
		}

		if (metal==0)
		{
			for (p=0;p<get_pass_count();p++)
			{
				vp.Add(blank);
				if (shadermodel4) _sprintf(cc.str,"pass P%d",p);
				else _sprintf(cc.str,"PASS P%d",p);
				vp.Add(cc);
				_sprintf(cc.str,"{");
				vp.Add(cc);
				vp.Add(blank);

				if (shadermodel4)
				{
					_sprintf(cc.str,"SetVertexShader(CompileShader(vs_4_0, RenderPassVS%d()));",p);
					vp.Add(cc);
					vp.Add(blank);
					if (s3psvp[p].Length()>0)
					{
						_sprintf(cc.str,"SetPixelShader(CompileShader(ps_4_0, RenderPassPS%d()));",p);
						vp.Add(cc);
						vp.Add(blank);
					}
					_sprintf(cc.str,"SetGeometryShader(NULL);");
					vp.Add(cc);
					vp.Add(blank);
				}
				else
				{
					_sprintf(cc.str,"VertexShader = compile vs_3_0 RenderPassVS%d();",p);
					vp.Add(cc);
					vp.Add(blank);
					if (s3psvp[p].Length()>0)
					{
						_sprintf(cc.str,"PixelShader  = compile ps_3_0 RenderPassPS%d();",p);
						vp.Add(cc);
						vp.Add(blank);
					}
				}
				c=s3rs[p].GetFirst();
				while (c)
				{
					vp.Add(*c);
					c=s3rs[p].GetNext();
				}

				vp.Add(blank);
				_sprintf(cc.str,"}");
				vp.Add(cc);
			
			}
			_sprintf(cc.str,"}");
			vp.Add(cc);

			vp.Add(blank);
		}
	}
	else get_vp(pass);

	level=0;
	len=0;
	cpt=0;
	c=vp.GetFirst();
	while (c)
	{
		size=(int)strlen(c->str)+1;

		if (c->str[0]=='{')
		{
			size+=level*2;
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			size+=level*2;
		}
		else
		{
			size+=level*2;
		}

		res=-1;
		for (n=0;n<=10;n++)
			if (vp_groups[n]==cpt) res=n;

		if (res!=-1) ptr_groups[res]=len;

		len+=size+1;
		cpt++;
		c=vp.GetNext();
	}

	buf=(char*) malloc(len+1);

	level=0;
	pos=0;
	c=vp.GetFirst();
	while (c)
	{
		if (c->str[0]=='{')
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}
		else
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}

		pos+=(int)strlen(c->str);
		buf[pos]='\n';
		pos++;
		c=vp.GetNext();
	}

	buf[pos]='\0';

	return buf;
}

char* CVertexProgram::get_string_mega_vs(int p)
{
	Code *c;
	Code cc;
	Code blank;
	unsigned int len,pos;
	char *buf;
	int level;
	int n;
	int res,cpt;
	unsigned int size;

	blank.str[0]='\0';

	vp.Free();

	int NB=96;
	if (shadermodel) NB=MAX_ENTRIES;

	_sprintf(cc.str,"cbuffer vs_constants : register(b0)");vp.Add(cc);
	_sprintf(cc.str,"{");vp.Add(cc);

	for (n=0;n<NB;n++)
	{
		if (constants[n])
		{
			if (translate_constants[n])
			{
			}
			else
			{
				if (LENGTH_VAR[l_constants[n]&0xffff]==1)
				{
					int nb=l_constants[n]>>16;
					if (nb==0) _sprintf(cc.str,"float4 %s;",constants[n]);
					else _sprintf(cc.str,"float4 %s[%d];",constants[n],nb);
				}
				if (LENGTH_VAR[l_constants[n]&0xffff]==4)
				{
					int nb=l_constants[n]>>16;
					if (nb==0) _sprintf(cc.str,"float4x4 %s;",constants[n]);
					else _sprintf(cc.str,"float4x4 %s[%d];",constants[n],nb);
				}
				vp.Add(cc);
			}
		}
	}

	_sprintf(cc.str,"};");vp.Add(cc);

	vp.Add(blank);

	int add=0;

	for (n=0;n<NB;n++)
	{
		if (constants[n])
		{
			int tt=1;

			if (translate_constants[n])
			{
				if ((strcmp(constants[n],"trigo_mod")==0)&&(trigocst[p][0]==0)) tt=0;
				if ((strcmp(constants[n],"trigo_cst")==0)&&(trigocst[p][1]==0)) tt=0;
				if ((strcmp(constants[n],"trigo_cst2")==0)&&(trigocst[p][2]==0)) tt=0;
				if ((strcmp(constants[n],"trigo_cst3")==0)&&(trigocst[p][3]==0)) tt=0;
				if ((strcmp(constants[n],"trigo_cst4")==0)&&(trigocst[p][4]==0)) tt=0;
					
				_sprintf(cc.str,"static const float4 %s = { %s };",constants[n],translate_constants[n]);
			}
			else
			{
				tt=0;
			}

			if (tt) { vp.Add(cc); add=1; }
		}
	}

	if (add) vp.Add(blank);

	int smp=0;

	for (n=0;n<16;n++)
	{
		if (texturevs[n])
		{
			_sprintf(cc.str,"Texture2D %s : register(t%d);",texturevs[n],n);
			vp.Add(cc);
			smp++;
		}
	}

	vp.Add(blank);

	if (smp)
	{
		_sprintf(cc.str,"SamplerState smpvs : register(s0);");vp.Add(cc);			
		vp.Add(blank);
	}

	vp.Add(blank);


	if (weightsandindices)
	{
		_sprintf(cc.str,"static int4 WInd;");vp.Add(cc);
		_sprintf(cc.str,"static float4 Weights;");vp.Add(cc);
		vp.Add(blank);
	}

	int flags=local_fvf_flags;

	if (flags&VPMORPH)
	{

		if (weightsandindices)
		{
			_sprintf(cc.str,"struct VS_INPUT");vp.Add(cc);
			_sprintf(cc.str,"{");vp.Add(cc);
			_sprintf(cc.str,"float4 v0 : POSITION0;");vp.Add(cc);
			_sprintf(cc.str,"float4 v1 : BLENDWEIGHT0;");vp.Add(cc);
			_sprintf(cc.str,"float3 v3 : NORMAL0;");vp.Add(cc);
			_sprintf(cc.str,"float2 v7 : TEXCOORD0;");vp.Add(cc);

			_sprintf(cc.str,"float4 v2 : POSITION1;");vp.Add(cc);
			_sprintf(cc.str,"float4 v4 : BLENDWEIGHT1;");vp.Add(cc);
			_sprintf(cc.str,"float3 v5 : NORMAL1;");vp.Add(cc);
			_sprintf(cc.str,"float2 v8 : TEXCOORD1;");vp.Add(cc);
			_sprintf(cc.str,"};");vp.Add(cc);

		}
		else
		{
			//TODO
			_sprintf(cc.str,"struct VS_INPUT");vp.Add(cc);
			_sprintf(cc.str,"{");vp.Add(cc);
			_sprintf(cc.str,"float4 v0 : POSITION0;");vp.Add(cc);
			_sprintf(cc.str,"float3 v3 : NORMAL0;");vp.Add(cc);
			_sprintf(cc.str,"float2 v7 : TEXCOORD0;");vp.Add(cc);
			_sprintf(cc.str,"float4 v1 : POSITION1;");vp.Add(cc);
			_sprintf(cc.str,"float3 v4 : NORMAL1;");vp.Add(cc);
			_sprintf(cc.str,"float2 v8 : TEXCOORD1;");vp.Add(cc);
			_sprintf(cc.str,"};");vp.Add(cc);
		}
	}
	else
	{
		_sprintf(cc.str,"struct VS_INPUT");vp.Add(cc);
		_sprintf(cc.str,"{");vp.Add(cc);
		// vertex format
		if (flags&XYZ) { _sprintf(cc.str,"float4 v0 : POSITION;");vp.Add(cc); }
		if (flags&BLEND)
		{
			_sprintf(cc.str,"float4 v1 : BLENDWEIGHT;");vp.Add(cc);
			if (!shadermodel4) { _sprintf(cc.str,"DWORD v2 : BLENDINDICES;");vp.Add(cc); }
		}
		if (flags&NORMAL) { _sprintf(cc.str,"float3 v3 : NORMAL;");vp.Add(cc); }
		if (flags&DIFFUSE) { _sprintf(cc.str,"float4 v5 : COLOR0;");vp.Add(cc); }
		if (flags&SPECULAR) { _sprintf(cc.str,"float4 v6 : COLOR1;");vp.Add(cc); }

		if (flags&_TEX0) { _sprintf(cc.str,"float2 v7 : TEXCOORD0;");vp.Add(cc); }
		if (flags&_TEX1) { _sprintf(cc.str,"float2 v8 : TEXCOORD1;");vp.Add(cc); }
		if (flags&_TEX2) { _sprintf(cc.str,"float2 v9 : TEXCOORD2;");vp.Add(cc); }
		if (flags&_TEX3) { _sprintf(cc.str,"float2 v10 : TEXCOORD3;");vp.Add(cc); }

		_sprintf(cc.str,"};");vp.Add(cc);
	}

	vp.Add(blank);

	unsigned int output=fvf_output[p];

	_sprintf(cc.str,"struct VS_OUTPUT%d",p);vp.Add(cc);
	_sprintf(cc.str,"{");vp.Add(cc);

	if (shadermodel4) { _sprintf(cc.str,"float4 p0 : SV_POSITION;");vp.Add(cc); }
	else { _sprintf(cc.str,"float4 p0 : POSITION;");vp.Add(cc); }
	if (output&DIFFUSE) { _sprintf(cc.str,"float4 v0 : COLOR0;");vp.Add(cc); }
	if (output&SPECULAR) { _sprintf(cc.str,"float4 v1 : COLOR1;");vp.Add(cc); }
	if (output&_TEX0) { _sprintf(cc.str,"float4 t0 : TEXCOORD0;");vp.Add(cc); }
	if (output&_TEX1) { _sprintf(cc.str,"float4 t1 : TEXCOORD1;");vp.Add(cc); }
	if (output&_TEX2) { _sprintf(cc.str,"float4 t2 : TEXCOORD2;");vp.Add(cc); }
	if (output&_TEX3) { _sprintf(cc.str,"float4 t3 : TEXCOORD3;");vp.Add(cc); }
	if (output&_TEX4) { _sprintf(cc.str,"float4 t4 : TEXCOORD4;");vp.Add(cc); }
	if (output&_TEX5) { _sprintf(cc.str,"float4 t5 : TEXCOORD5;");vp.Add(cc); }
	if (output&_TEX6) { _sprintf(cc.str,"float4 t6 : TEXCOORD6;");vp.Add(cc); }
	if (output&_TEX7) { _sprintf(cc.str,"float4 t7 : TEXCOORD7;");vp.Add(cc); }

	_sprintf(cc.str,"};");vp.Add(cc);

	vp.Add(blank);

	c=s3fnvp[p].GetFirst();
	while (c)
	{
		vp.Add(*c);
		c=s3fnvp[p].GetNext();
	}

	vp.Add(blank);

	c=s3vp[p].GetFirst();
	while (c)
	{
		vp.Add(*c);
		c=s3vp[p].GetNext();
	}

	vp.Add(blank);

	level=0;
	len=0;
	cpt=0;
	c=vp.GetFirst();
	while (c)
	{
		size=(int)strlen(c->str)+1;

		if (c->str[0]=='{')
		{
			size+=level*2;
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			size+=level*2;
		}
		else
		{
			size+=level*2;
		}

		res=-1;
		for (n=0;n<=10;n++)
			if (vp_groups[n]==cpt) res=n;

		if (res!=-1) ptr_groups[res]=len;

		len+=size+1;
		cpt++;
		c=vp.GetNext();
	}

	buf=(char*) malloc(len+1);

	level=0;
	pos=0;
	c=vp.GetFirst();
	while (c)
	{
		if (c->str[0]=='{')
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}
		else
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}

		pos+=(int)strlen(c->str);
		buf[pos]='\n';
		pos++;
		c=vp.GetNext();
	}

	buf[pos]='\0';

	return buf;
}


char* CVertexProgram::get_string_mega_ps(int p)
{
	Code *c;
	Code cc;
	Code blank;
	unsigned int len,pos;
	char *buf;
	int level;
	int n;
	int res,cpt;
	unsigned int size;

	blank.str[0]='\0';

	vp.Free();

	if ((npsvectors>0)||(npsmatrices>0))
	{
		_sprintf(cc.str,"cbuffer ps_constants : register(b0)");vp.Add(cc);
		_sprintf(cc.str,"{");vp.Add(cc);

		for (n=0;n<npsvectors;n++)
		{
			_sprintf(cc.str,"float4 %s;",psvectors[n]);vp.Add(cc);
		}

		for (n=0;n<npsmatrices;n++)
		{
			_sprintf(cc.str,"float4x4 %s;",psmatrices[n]);vp.Add(cc);
		}

		_sprintf(cc.str,"};");vp.Add(cc);
		cc.str[0]='\0'; vp.Add(cc);
	}

	int smp=0;

	for (n=0;n<16;n++)
	{
		if (texture[n])
		{
			_sprintf(cc.str,"Texture2D %s : register(t%d);",texture[n],n);
			vp.Add(cc);
			smp++;
		}
	}

	vp.Add(blank);

	_sprintf(cc.str,"struct PS_OUTPUT");vp.Add(cc);
	_sprintf(cc.str,"{");vp.Add(cc);
	_sprintf(cc.str,"float4 Out : SV_Target;");vp.Add(cc);	
	if (modifyZ) { _sprintf(cc.str,"float Depth : SV_Depth;");vp.Add(cc); }			
	_sprintf(cc.str,"};");vp.Add(cc);

	vp.Add(blank);

	unsigned int output=fvf_output[p];

	_sprintf(cc.str,"struct VS_OUTPUT%d",p);vp.Add(cc);
	_sprintf(cc.str,"{");vp.Add(cc);

	if (shadermodel4) { _sprintf(cc.str,"float4 p0 : SV_POSITION;");vp.Add(cc); }
	else { _sprintf(cc.str,"float4 p0 : POSITION;");vp.Add(cc); }
	if (output&DIFFUSE) { _sprintf(cc.str,"float4 v0 : COLOR0;");vp.Add(cc); }
	if (output&SPECULAR) { _sprintf(cc.str,"float4 v1 : COLOR1;");vp.Add(cc); }
	if (output&_TEX0) { _sprintf(cc.str,"float4 t0 : TEXCOORD0;");vp.Add(cc); }
	if (output&_TEX1) { _sprintf(cc.str,"float4 t1 : TEXCOORD1;");vp.Add(cc); }
	if (output&_TEX2) { _sprintf(cc.str,"float4 t2 : TEXCOORD2;");vp.Add(cc); }
	if (output&_TEX3) { _sprintf(cc.str,"float4 t3 : TEXCOORD3;");vp.Add(cc); }
	if (output&_TEX4) { _sprintf(cc.str,"float4 t4 : TEXCOORD4;");vp.Add(cc); }
	if (output&_TEX5) { _sprintf(cc.str,"float4 t5 : TEXCOORD5;");vp.Add(cc); }
	if (output&_TEX6) { _sprintf(cc.str,"float4 t6 : TEXCOORD6;");vp.Add(cc); }
	if (output&_TEX7) { _sprintf(cc.str,"float4 t7 : TEXCOORD7;");vp.Add(cc); }

	_sprintf(cc.str,"};");vp.Add(cc);

	vp.Add(blank);

	if (smp)
	{
		_sprintf(cc.str,"SamplerState smp : register(s0);");vp.Add(cc);			
		vp.Add(blank);
	}

	int add=0;

	for (n=0;n<MAX_ENTRIES;n++)
	{
		if (ps_constants[n])
		{
			if (translate_ps_constants[n])
			{
				int tt=1;
				if ((strcmp(ps_constants[n],"zerocinq")==0)&&(trigocst[p][5]==0)) tt=0;
				if (tt)
				{
					if (ps_constants_type[n]==1) _sprintf(cc.str,"static const float4  %s = { %s };",ps_constants[n],translate_ps_constants[n]);
					else _sprintf(cc.str,"static const float4x4  %s = { %s };",ps_constants[n],translate_ps_constants[n]);
					vp.Add(cc);
					add=1;
				}
			}
		}
	}

	if (add) vp.Add(blank);

	add=0;

	if (globalvar)
	{
		if (output&_TEX7) { _sprintf(cc.str,"static float4 it7;"); vp.Add(cc); add=1; }
		if (output&_TEX6) { _sprintf(cc.str,"static float4 it6;"); vp.Add(cc); add=1; }
		if (output&_TEX5) { _sprintf(cc.str,"static float4 it5;"); vp.Add(cc); add=1; }
		if (output&_TEX4) { _sprintf(cc.str,"static float4 it4;"); vp.Add(cc); add=1; }
		if (output&_TEX3) { _sprintf(cc.str,"static float4 it3;"); vp.Add(cc); add=1; }
		if (output&_TEX2) { _sprintf(cc.str,"static float4 it2;"); vp.Add(cc); add=1; }
		if (output&_TEX1) { _sprintf(cc.str,"static float4 it1;"); vp.Add(cc); add=1; }
		if (output&_TEX0) { _sprintf(cc.str,"static float4 it0;"); vp.Add(cc); add=1; }
		if (output&SPECULAR) { _sprintf(cc.str,"static float4 iv1;"); vp.Add(cc); add=1; }
		if (output&DIFFUSE) { _sprintf(cc.str,"static float4 iv0;"); vp.Add(cc); add=1; }
	}
	if (modifyZ) { _sprintf(cc.str,"static float4 ip0;"); vp.Add(cc); add=1; }

	if (add) vp.Add(blank);

	c=s3psfnvp[p].GetFirst();
	while (c)
	{
		vp.Add(*c);
		c=s3psfnvp[p].GetNext();
	}

	vp.Add(blank);

	c=s3psvp[p].GetFirst();
	while (c)
	{
		vp.Add(*c);
		c=s3psvp[p].GetNext();
	}

	vp.Add(blank);

	level=0;
	len=0;
	cpt=0;
	c=vp.GetFirst();
	while (c)
	{
		size=(int)strlen(c->str)+1;

		if (c->str[0]=='{')
		{
			size+=level*2;
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			size+=level*2;
		}
		else
		{
			size+=level*2;
		}

		res=-1;
		for (n=0;n<=10;n++)
			if (vp_groups[n]==cpt) res=n;

		if (res!=-1) ptr_groups[res]=len;

		len+=size+1;
		cpt++;
		c=vp.GetNext();
	}

	buf=(char*) malloc(len+1);

	level=0;
	pos=0;
	c=vp.GetFirst();
	while (c)
	{
		if (c->str[0]=='{')
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
			level++;
		}
		else
		if (c->str[0]=='}')
		{
			level--;
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}
		else
		{
			for (n=0;n<level*2;n++) buf[pos++]=' ';
			memcpy(&buf[pos],c->str,strlen(c->str));
		}

		pos+=(int)strlen(c->str);
		buf[pos]='\n';
		pos++;
		c=vp.GetNext();
	}

	buf[pos]='\0';

	return buf;
}



char* CVertexProgram::get_string_psh(int pass)
{
	Code *c;
	unsigned int len,pos;
	char *buf;
	int level;
	int n;
	int res,cpt;
	unsigned int size;

	if (selected_tech_with_ps)
	{
		get_ps(pass);

		if (vp.Length()>2)
		{
			level=0;
			len=0;
			cpt=0;
			c=vp.GetFirst();
			while (c)
			{
				size=(int)strlen(c->str)+1;

				if (c->str[0]=='{')
				{

					size+=level*2;
					level++;
				}
				else
				if (c->str[0]=='}')
				{
					level--;
					size+=level*2;

				}
				else
				{
					size+=level*2;

				}

				res=-1;
				for (n=0;n<=10;n++)
					if (vp_groups[n]==cpt) res=n;

				if (res!=-1) ptr_groups[res]=len;

				len+=size+1;
				cpt++;
				c=vp.GetNext();
			}

			buf=(char*) malloc(len+1);

			level=0;
			pos=0;
			c=vp.GetFirst();
			while (c)
			{
				if (c->str[0]=='{')
				{
					for (n=0;n<level*2;n++) buf[pos++]=' ';
					memcpy(&buf[pos],c->str,strlen(c->str));
					level++;
				}
				else
				if (c->str[0]=='}')
				{
					level--;
					for (n=0;n<level*2;n++) buf[pos++]=' ';
					memcpy(&buf[pos],c->str,strlen(c->str));
				}
				else
				{
					for (n=0;n<level*2;n++) buf[pos++]=' ';
					memcpy(&buf[pos],c->str,strlen(c->str));

				}

				pos+=(int)strlen(c->str);
				buf[pos]='\n';
				pos++;
				c=vp.GetNext();
			}

			buf[pos]='\0';

			return buf;
		}
		else return NULL;
	}
	else return NULL;
}

int CVertexProgram::get_pass_count()
{
	return NPASS;
}

void CVertexProgram::get_vp(int pass)
{
	Code *c;
	Code rc;
	int n,ndx;

	if (api)
	{
		for (n=0;n<=10;n++)
		{
			ndx=gltemp[pass].Length() + glheader.Length()+groups[selected_tech][pass][n];
			vp_groups[n]=ndx;
		}

		vp.Free();

		if (!shadermodel)
		{
			_sprintf(rc.str,"!!ARBvp1.0");
			vp.Add(rc);
		}
		else
		{
//			_sprintf(rc.str,"#version 140");
//			vp.Add(rc);

#if defined(GLES20)||defined(GLESFULL)
			_sprintf(rc.str,"precision highp float;");
			vp.Add(rc);
#endif
		}

		c=glheader.GetFirst();
		while (c)
		{
			vp.Add(*c);
			c=glheader.GetNext();
		}
		
		if (shadermodel)
		{
			_sprintf(rc.str,"attribute vec4 VertexPosition;");
			vp.Add(rc);
			if ((fvf_sm&NORMAL)||(stream))
			{
				_sprintf(rc.str,"attribute vec3 VertexNormal;");
				vp.Add(rc);
			}

			if (stream)
			{
				_sprintf(rc.str,"attribute vec4 VertexPosition2;");
				vp.Add(rc);
				_sprintf(rc.str,"attribute vec3 VertexNormal2;");
				vp.Add(rc);
			}

			if (fvf_sm&DIFFUSE)
			{
				_sprintf(rc.str,"attribute vec4 VertexColor;");
				vp.Add(rc);
			}

			if (fvf_sm&SPECULAR)
			{
				_sprintf(rc.str,"attribute vec4 VertexSecondaryColor;");
				vp.Add(rc);
			}

			if ((fvf_sm&_TEX0)||(stream))
			{
				_sprintf(rc.str,"attribute vec2 MultiTexCoord0;");
				vp.Add(rc);
			}

			if (fvf_sm&_TEX1)
			{
				_sprintf(rc.str,"attribute vec2 MultiTexCoord1;");
				vp.Add(rc);
			}

			if (fvf_sm&_TEX2)
			{
				_sprintf(rc.str,"attribute vec2 MultiTexCoord2;");
				vp.Add(rc);
			}

			if (fvf_sm&_TEX3)
			{
				_sprintf(rc.str,"attribute vec2 MultiTexCoord3;");
				vp.Add(rc);
			}

			if (weightsandindices)
			{
				_sprintf(rc.str,"attribute vec4 IWeights;");
				vp.Add(rc);
			}

			_sprintf(rc.str,"");
			vp.Add(rc);

			if (weightsandindices)
			{
				_sprintf(rc.str,"vec4 Weights;");
				vp.Add(rc);
				_sprintf(rc.str,"ivec4 WIndices;");
				vp.Add(rc);
				_sprintf(rc.str,"");
				vp.Add(rc);
			}

#ifdef GLES20
			_sprintf(rc.str,"");
			vp.Add(rc);
			_sprintf(rc.str,"varying vec4 myColor;");
			vp.Add(rc);
			if (fvf_output[pass]&SPECULAR)
			{
				_sprintf(rc.str,"varying vec4 mySecondaryColor;");
				vp.Add(rc);
			}
			if (fvf_output[pass]&_TEX7) { _sprintf(rc.str,"varying vec4 myTexCoord[8];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX6) { _sprintf(rc.str,"varying vec4 myTexCoord[7];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX5) { _sprintf(rc.str,"varying vec4 myTexCoord[6];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX4) { _sprintf(rc.str,"varying vec4 myTexCoord[5];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX3) { _sprintf(rc.str,"varying vec4 myTexCoord[4];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX2) { _sprintf(rc.str,"varying vec4 myTexCoord[3];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX1) { _sprintf(rc.str,"varying vec4 myTexCoord[2];"); vp.Add(rc); }
			else if (fvf_output[pass]&_TEX0) { _sprintf(rc.str,"varying vec4 myTexCoord[1];"); vp.Add(rc); }
#endif

			c=glfn.GetFirst();
			while (c)
			{
				vp.Add(*c);
				c=glfn.GetNext();
			}

			_sprintf(rc.str,"void main(void)");
			vp.Add(rc);
			_sprintf(rc.str,"{");
			vp.Add(rc);

			c=glvar[pass].GetFirst();
			while (c)
			{
				strcpy(rc.str,c->str);
				vp.Add(rc);
				c=glvar[pass].GetNext();
			}

			if (weightsandindices)
			{
				
				_sprintf(rc.str,"WIndices.x  = int(IWeights.x/2.0);");vp.Add(rc);
				_sprintf(rc.str,"Weights.x = IWeights.x - (float(WIndices.x)*2.0);");vp.Add(rc);
				
				_sprintf(rc.str,"WIndices.y  = int(IWeights.y/2.0);");vp.Add(rc);
				_sprintf(rc.str,"Weights.y = IWeights.y - (float(WIndices.y)*2.0);");vp.Add(rc);
				
				_sprintf(rc.str,"WIndices.z  = int(IWeights.z/2.0);");vp.Add(rc);
				_sprintf(rc.str,"Weights.z = IWeights.z - (float(WIndices.z)*2.0);");vp.Add(rc);

				_sprintf(rc.str,"WIndices.w  = int(IWeights.w/2.0);");vp.Add(rc);
				_sprintf(rc.str,"Weights.w = IWeights.w - (float(WIndices.w)*2.0);");vp.Add(rc);
				_sprintf(rc.str,"");vp.Add(rc);
				
			}
		}

		_sprintf(rc.str,"");
		vp.Add(rc);

		c=gltemp[pass].GetFirst();
		while (c)
		{
			if (strcmp(c->str,"")==0)
				_sprintf(rc.str,"%s",c->str);
			else
			{
				if (((str_match(c->str,"for"))||(str_match(c->str,"while"))||(str_match(c->str,"if"))||(str_match(c->str,"else"))||(str_match(c->str,"{"))||(str_match(c->str,"}")))) _sprintf(rc.str,"%s",c->str);
				else _sprintf(rc.str,"%s;",c->str);
			}
			vp.Add(rc);
			c=gltemp[pass].GetNext();
		}

		c=glvp[pass].GetFirst();
		while (c)
		{
			if (strcmp(c->str,"")==0) _sprintf(rc.str,"%s",c->str);
			else
			{
				if (((str_match(c->str,"for"))||(str_match(c->str,"while"))||(str_match(c->str,"if"))||(str_match(c->str,"else"))||(str_match(c->str,"{"))||(str_match(c->str,"}")))) _sprintf(rc.str,"%s",c->str);
				else _sprintf(rc.str,"%s;",c->str);
			}
			vp.Add(rc);
			c=glvp[pass].GetNext();
		}

		if (shadermodel)
		{
			_sprintf(rc.str,"}");
			vp.Add(rc);
		}
		else
		{
			_sprintf(rc.str,"END");
			vp.Add(rc);
		}
	}
}


void CVertexProgram::get_ps(int pass)
{
	Code *c;
	Code rc;
	int n,ndx;

	if (api)
	{
		for (n=0;n<=10;n++)
		{
			ndx=gltemp[pass].Length() + glheader.Length()+groups[selected_tech][pass][n];
			vp_groups[n]=ndx;
		}

		vp.Free();

		if (!shadermodel)
		{
			_sprintf(rc.str,"!!ARBfp1.0");
			vp.Add(rc);
		}
		else
		{
#if defined(GLES20)||defined(GLESFULL)
			if (modifyZ)
			{
				_sprintf(rc.str,"#extension GL_EXT_frag_depth : enable");
				vp.Add(rc);
			}

			if (OPENGLES30HIGHP) _sprintf(rc.str,"precision highp float;");
			else _sprintf(rc.str,"precision mediump float;");
			vp.Add(rc);
#endif
		}

		c=glheaderps.GetFirst();
		while (c)
		{
			vp.Add(*c);
			c=glheaderps.GetNext();
		}

#ifdef GLES20
		_sprintf(rc.str,"");
		vp.Add(rc);

		_sprintf(rc.str,"varying vec4 myColor;");
		vp.Add(rc);
		if (fvf_output[pass]&SPECULAR)
		{
			_sprintf(rc.str,"varying vec4 mySecondaryColor;");
			vp.Add(rc);
		}
		if (fvf_output[pass]&_TEX7) { _sprintf(rc.str,"varying vec4 myTexCoord[8];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX6) { _sprintf(rc.str,"varying vec4 myTexCoord[7];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX5) { _sprintf(rc.str,"varying vec4 myTexCoord[6];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX4) { _sprintf(rc.str,"varying vec4 myTexCoord[5];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX3) { _sprintf(rc.str,"varying vec4 myTexCoord[4];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX2) { _sprintf(rc.str,"varying vec4 myTexCoord[3];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX1) { _sprintf(rc.str,"varying vec4 myTexCoord[2];"); vp.Add(rc); }
		else if (fvf_output[pass]&_TEX0) { _sprintf(rc.str,"varying vec4 myTexCoord[1];"); vp.Add(rc); }
#endif

		c=glfnps.GetFirst();
		while (c)
		{
			vp.Add(*c);
			c=glfnps.GetNext();
		}

		_sprintf(rc.str,"");
		vp.Add(rc);

		if (shadermodel)
		{
            if (FSR==1)
            {
                char *sfsr=shader_functions_FSR_header;
                int ps=0;
                char *pstmp=str_parse_rln2(sfsr,&ps);
                while (pstmp)
                {
                    if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
                    pstmp=str_parse_rln2(sfsr,&ps);
                }
                
                sfsr=shader_functions_FSR_EASU;
                ps=0;
                pstmp=str_parse_rln2(sfsr,&ps);
                while (pstmp)
                {
                    if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
                    pstmp=str_parse_rln2(sfsr,&ps);
                }
            }

            if (FSR==2)
            {
                char *sfsr=shader_functions_FSR_header;
                int ps=0;
                char *pstmp=str_parse_rln2(sfsr,&ps);
                while (pstmp)
                {
                    if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
                    pstmp=str_parse_rln2(sfsr,&ps);
                }
                
                sfsr=shader_functions_FSR_RCAS;
                ps=0;
                pstmp=str_parse_rln2(sfsr,&ps);
                while (pstmp)
                {
                    if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
                    pstmp=str_parse_rln2(sfsr,&ps);
                }
            }

			if ((FSRSample0)||(FSRSample1))
			{
                char *sfsr=shader_functions_FSR_header_01;
                int ps=0;
                char *pstmp=str_parse_rln2(sfsr,&ps);
                while (pstmp)
                {
                    if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
                    pstmp=str_parse_rln2(sfsr,&ps);
                }
                
				if ((FSRSample0)&&(RenderState[0][0].Texture[0]))
				{
					sfsr=shader_functions_FSR_RCAS_0;
					ps=0;
					pstmp=str_parse_rln2(sfsr,&ps);
					while (pstmp)
					{
						if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
						pstmp=str_parse_rln2(sfsr,&ps);
					}
				}

				if ((SizingTexture0)&&(FSRSample0)&&(RenderState[0][0].Texture[0]))
				{
					char tempss[1024];
					_sprintf(tempss,shader_functions_FSR_RCAS_0_warp_samplebump,valueFSRRCASSample);
					sfsr=tempss;
					ps=0;
					pstmp=str_parse_rln2(sfsr,&ps);
					while (pstmp)
					{
						if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
						pstmp=str_parse_rln2(sfsr,&ps);
					}
				}

				if ((FSRSample1)&&(RenderState[0][0].Texture[1]))
				{
					sfsr=shader_functions_FSR_RCAS_1;
					ps=0;
					pstmp=str_parse_rln2(sfsr,&ps);
					while (pstmp)
					{
						if (strlen(pstmp)>0) { strcpy(rc.str,pstmp); vp.Add(rc); }
						pstmp=str_parse_rln2(sfsr,&ps);
					}
				}
			}

			_sprintf(rc.str,"");
			vp.Add(rc);
			_sprintf(rc.str,"void main(void)");
			vp.Add(rc);
			_sprintf(rc.str,"{");
			vp.Add(rc);
		}

		c=glps[pass].GetFirst();
		while (c)
		{
			if (strcmp(c->str,"")==0) _sprintf(rc.str,"%s",c->str);
			else
			{
				if (((str_match(c->str,"for"))||(str_match(c->str,"while"))||(str_match(c->str,"if"))||(str_match(c->str,"else"))||(str_match(c->str,"{"))||(str_match(c->str,"}")))) _sprintf(rc.str,"%s",c->str);
				else _sprintf(rc.str,"%s;",c->str);
			}
			vp.Add(rc);
			c=glps[pass].GetNext();
		}

		if (shadermodel)
		{
			_sprintf(rc.str,"}");
			vp.Add(rc);
		}
		else
		{
			_sprintf(rc.str,"END");
			vp.Add(rc);
		}
	}
}


void CVertexProgram::Free()
{
	int n;

	nmatrices=0;
	nfloats=0;

	for (n=0;n<16;n++)
	{
        if (texture[n]) free(texture[n]);
        texture[n]=NULL;
        if (texturevs[n]) free(texturevs[n]);
        texturevs[n]=NULL;
	}
    ntexture=ntexturevs=0;

	for (n=0;n<MAX_ENTRIES;n++)
	{
		if (constants[n]) free(constants[n]);
		if (translate_constants[n]) free(translate_constants[n]);

		constants[n]=NULL;
		translate_constants[n]=NULL;
	}

	for (n=0;n<last_ps_constant;n++)
	{
		free(ps_constants[n]);
		free(translate_ps_constants[n]);
		ps_constants[n]=NULL;
		translate_ps_constants[n]=NULL;
	}

	for (n=0;n<8;n++)
	{
		if (emvp[n])
		{
			emvp[n]->Free();
			delete emvp[n];
		}
	}

	vp.Free();
	vpoutput.Free();
	vpinput.Free();
	vpcst.Free();
	vptmp.Free();

	for (n=0;n<8;n++)
	{
		s3vpmetal[n].Free();
		s3psvpmetal[n].Free();

		glvar[n].Free();
		glvp[n].Free();
		glps[n].Free();
		gltemp[n].Free();

		s3vp[n].Free();
		s3fnvp[n].Free();
		s3psvp[n].Free();
		s3psfnvp[n].Free();
		s3rs[n].Free();
	}

	glheader.Free();
	glheaderps.Free();
	glfn.Free();
	glfnps.Free();


	ListMacros.Free();
	TempVars.Free();
	Vars.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Emulated Vertex Program
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEmulatedVertexProgram::Init()
{
	Vertex_Array=(float*) malloc(_EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_*3*sizeof(float));
	Color_Array=(float*) malloc(_EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_*4*sizeof(float));
	Color2_Array=(float*) malloc(_EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_*4*sizeof(float));
	TexCoo_Array=(float*) malloc(_EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_*2*sizeof(float));
	TexCoo2_Array=(float*) malloc(_EMULATED_VERTEX_PROGRAM_ARRAY_LENGTH_*2*sizeof(float));
	for (int n=0;n<MAX_ENTRIES;n++) vmc[n]=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Free()
{
	if (Vertex_Array) free(Vertex_Array);
	if (Color_Array) free(Color_Array);
	if (Color2_Array) free(Color2_Array);
	if (TexCoo_Array) free(TexCoo_Array);
	if (TexCoo2_Array) free(TexCoo2_Array);

	Vertex_Array=NULL;
	Color_Array=NULL;
	Color2_Array=NULL;
	TexCoo_Array=NULL;
	TexCoo2_Array=NULL;

	nCode=0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSVP_id,CSVP_number,CSVP_modifier;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSVP_GetIdentifier(char *str0)
{
	char str[512];
	char str2[512];
	int s,p,pp;

	CSVP_id=0;

	if (str0[0]=='-') {s=-1;_sprintf(str,"%s",&str0[1]);}
	else {s=1;_sprintf(str,"%s",str0);}

	pp=str_char(str,']');
	if (pp!=-1)
	{
		_sprintf(str2,"%s",&str[pp]);
	}
	else _sprintf(str2,"%s",str);

	p=str_char(str2,'.');
	if (p!=-1)
	{
		CSVP_modifier=0;
		if ((str_char(&str2[p+1],'x')!=-1)||(str_char(&str2[p+1],'X')!=-1)) CSVP_modifier|=C_X;
		if ((str_char(&str2[p+1],'y')!=-1)||(str_char(&str2[p+1],'Y')!=-1)) CSVP_modifier|=C_Y;
		if ((str_char(&str2[p+1],'z')!=-1)||(str_char(&str2[p+1],'Z')!=-1)) CSVP_modifier|=C_Z;
		if ((str_char(&str2[p+1],'w')!=-1)||(str_char(&str2[p+1],'W')!=-1)) CSVP_modifier|=C_W;

	}
	else CSVP_modifier=C_X|C_Y|C_Z|C_W;

	if (pp!=-1) str[str_char(str,'[')]='\0';
	p=str_char(str,'.');

	if (p!=-1) str[p]='\0';

	if (str[0]=='r')
	{
		CSVP_id=C_REG;
		sscanf(&str[1],"%d",&CSVP_number);
	}
	else
	if (str[0]=='c')
	{
		CSVP_id=C_CONST;
		if (pp==-1) sscanf(&str[1],"%d",&CSVP_number);
		else CSVP_number=0;
	}
	else
	if (str[0]=='o')
	{
		CSVP_number=0;
		if (strcmp(str,"oPos")==0) CSVP_id=C_OVERTEX;
		if (strcmp(str,"oD0")==0) CSVP_id=C_OCOLOR;
		if (strcmp(str,"oD1")==0) CSVP_id=C_OCOLOR2;
		if (strcmp(str,"oT0")==0) CSVP_id=C_OTEXCOO;
		if (strcmp(str,"oT1")==0) CSVP_id=C_OTEXCOO2;
		if (strcmp(str,"oT2")==0) CSVP_id=C_OTEXCOO3;
	}
	else
	if (str[0]=='v')
	{
		sscanf(&str[1],"%d",&CSVP_number);

		switch (CSVP_number)
		{
		case 0:
			CSVP_id=C_IVERTEX;
			break;
		case 1:
			CSVP_id=C_IWEIGHTS; // <=> C_IVERTEX2
			break;
		case 2:
			CSVP_id=C_IMATRIXINDEX;
			break;
		case 3:
			CSVP_id=C_INORMAL;
			break;
		case 4:
			CSVP_id=C_INORMAL2;
			break;
		case 5:
			CSVP_id=C_ICOLOR;
			break;
		case 6:
			CSVP_id=C_ICOLOR2;
			break;
		case 7:
			CSVP_id=C_ITEXCOO;
			break;
		case 8:
			CSVP_id=C_ITEXCOO2;
			break;
		case 9:
			CSVP_id=C_ITEXCOO3;
			break;
		}

		CSVP_number=0;

	}
	else
	if (str[0]=='a')
	{
		CSVP_number=0;
		CSVP_id=C_IADDRESS;
	}

	CSVP_id=s*CSVP_id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * op_str[]={
	"MOV",
	"ADD",
	"SUB",
	"MUL",
	"RCP",
	"RSQ",
	"SGE",
	"SLT",
	"M4X4",
	"M3X3",
	"CROSS",
	"MAX",
	"MIN",
	"DP3",
	"SIN",
	"COS",
	"TAN",
	"ASIN",
	"ACOS",
	"ATAN",
	"EXP",
	"LOG",
	"TGT",
	"FLOOR"

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* CEmulatedVertexProgram::str_Get(CVector4 * v)
{
	if (v==&A0) return "a0";

	if (v==&R[0]) return "r0";
	if (v==&R[1]) return "r1";
	if (v==&R[2]) return "r2";
	if (v==&R[3]) return "r3";
	if (v==&R[4]) return "r4";
	if (v==&R[5]) return "r5";
	if (v==&R[6]) return "r6";
	if (v==&R[7]) return "r7";
	if (v==&R[8]) return "r8";
	if (v==&R[9]) return "r9";
	if (v==&R[10]) return "r10";
	if (v==&R[11]) return "r11";

	if (v==&iPos) return "iPos";
	if (v==&iNorm) return "iNorm";
	if (v==&iNorm2) return "iNorm2";
	if (v==&iTexCoo) return "iTexCoo";
	if (v==&iTexCoo2) return "iTexCoo2";
	if (v==&iColor) return "iDiffuse";
	if (v==&iColor2) return "iSpecular";
	if (v==&iWeights) return "iWeights";
	if (v==&iWIndex) return "iWIndices";

	if (v==&oPos) return "oPos";
	if (v==&oNorm) return "oNorm";
	if (v==&oTexCoo) return "oTexCoo";
	if (v==&oTexCoo2) return "oTexCoo2";
	if (v==&oColor) return "oDiffuse";
	if (v==&oColor2) return "oSpecular";

	if (v==NULL) return "CstMatrix";

	return "Cst";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Set(int n,CVector4 v)
{
    vC[n].Init(v.x,v.y,v.z,v.w);
    vmc[n]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Set(int n,CVector v)
{
    vC[n].Init(v.x,v.y,v.z,0.0f);
    vmc[n]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Set(int n,float x,float y,float z,float w)
{
	vC[n].Init(x,y,z,w);
	vmc[n]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Set(int n,CMatrix m)
{
	mC[n]=m;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector4* CEmulatedVertexProgram::GetPrm(int id,int n)
{
	CVector4 *res=NULL;

	switch (id)
	{
	case C_REG:
		res=&R[n];
		break;
	case C_CONST:
		if (vmc[n]) res=&vC[n];
		break;
	case C_IVERTEX:
		res=&iPos;
		break;
	case C_INORMAL:
		res=&iNorm;
		break;
	case C_INORMAL2:
		res=&iNorm2;
		break;
	case C_ICOLOR:
		res=&iColor;
		break;
	case C_ICOLOR2:
		res=&iColor2;
		break;
	case C_ITEXCOO:
		res=&iTexCoo;
		break;
	case C_ITEXCOO2:
		res=&iTexCoo2;
		break;
	case C_ITEXCOO3:
		res=&iTexCoo3;
		break;
	case C_OVERTEX:
		res=&oPos;
		break;
	case C_ONORMAL:
		res=&oNorm;
		break;
	case C_OCOLOR:
		res=&oColor;
		break;
	case C_OCOLOR2:
		res=&oColor2;
		break;
	case C_OTEXCOO:
		res=&oTexCoo;
		break;
	case C_OTEXCOO2:
		res=&oTexCoo2;
		break;
	case C_OTEXCOO3:
		res=&oTexCoo3;
		break;

	case C_IWEIGHTS:
		res=&iWeights;
		break;

	case C_IMATRIXINDEX:
		res=&iWIndex;
		break;

	case C_IADDRESS:
		res=&A0;
		break;
	};

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix* CEmulatedVertexProgram::mGetPrm(int id,int n)
{
	return &mC[n];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float ff_abs(float f)
{
	if (f<0) return -f;
	else return f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Exec(int nb,float * Vertices,float * Normals,float * Vertices2,float * Normals2,float * Colors,float * Colors2,float * TexCoos,float * TexCoos2,int sizec,int sizec2,unsigned char * MatrixIndices,float * Weights,unsigned int fvf)
{
	int n;
	int cp;
	int n_x3,n_x4,n_x2;
	int adr;

#ifndef _EMULATED_VERTEX_PROGRAM_METHOD_FASTCALL_
	float a,r;
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CMatrix *M;
	CVector4 tres;
	float s;
	CVector v,v1,v2;
	CVector4 r1,r2;
	float power;
	const float MAXPOWER=127.9961f;

#endif

	for (n=0;n<nb;n++)
	{
		n_x4=n*4;
		n_x3=n*3;
		n_x2=n*2;

		if (Vertices) iPos.Init(Vertices[n_x3 +0],Vertices[n_x3 +1],Vertices[n_x3 +2],1.0f);
		if (Normals) iNorm.Init(Normals[n_x3 +0],Normals[n_x3 +1],Normals[n_x3 +2],0.0f);

		if (Vertices2) iWeights.Init(Vertices2[n_x3 +0],Vertices2[n_x3 +1],Vertices2[n_x3 +2],1.0f);
		if (Normals2) iNorm2.Init(Normals2[n_x3 +0],Normals2[n_x3 +1],Normals2[n_x3 +2],0.0f);

		if (sizec==3)
		{
			if (Colors) iColor.Init(Colors[n_x3 +0],Colors[n_x3 +1],Colors[n_x3 +2],0.0f);
		}
		else
		{
			if (Colors) iColor.Init(Colors[n_x4 +0],Colors[n_x4 +1],Colors[n_x4 +2],Colors[n_x4 +3]);
		}

		if (sizec==3)
		{
			if (Colors2) iColor2.Init(Colors2[n_x3 +0],Colors2[n_x3 +1],Colors2[n_x3 +2],0.0f);
		}
		else
		{
			if (Colors2) iColor2.Init(Colors2[n_x4 +0],Colors2[n_x4 +1],Colors2[n_x4 +2],Colors2[n_x4 +3]);
		}

		if (Weights) iWeights.Init(Weights[n_x4+0],Weights[n_x4+1],Weights[n_x4+2],Weights[n_x4+3]);

		if (MatrixIndices) iWIndex.Init(MatrixIndices[n_x4+0],MatrixIndices[n_x4+1],MatrixIndices[n_x4+2],MatrixIndices[n_x4+3]);

		if (TexCoos) iTexCoo.Init(TexCoos[n_x2 +0],TexCoos[n_x2 +1],0.0f,0.0f);
		if (TexCoos2) iTexCoo2.Init(TexCoos2[n_x2 +0],TexCoos2[n_x2 +1],0.0f,0.0f);

		for (cp=0;cp<nCode;cp++)
		{

#ifndef _EMULATED_VERTEX_PROGRAM_METHOD_FASTCALL_

			mr=Code[cp].m[0];
			m1=Code[cp].m[1];
			m2=Code[cp].m[2];

			res=Code[cp].dst;

			if (Code[cp].addressed==1)
			{
				adr=(int) A0.x;
				op1=&Code[cp].op1[adr];
			}
			else op1=Code[cp].op1;

			if (!Code[cp].op2)
			{
				if (Code[cp].addressed==2)
				{
					adr=(int) (A0.x/2);
					M=&Code[cp].mat[adr];
				}
				else
				{
					M=Code[cp].mat;
				}
			}
			else
			{
				if (Code[cp].addressed==2)
				{
					adr=(int) A0.x;
					op2=&Code[cp].op2[adr];
				}
				else op2=Code[cp].op2;
			}

			if (mr==15)
			{
				switch (Code[cp].op)
				{
				case OP_MOV:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}
					tres=r1;
					break;
				case OP_ADD:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}

					tres.x=r1.x+r2.x;
					tres.y=r1.y+r2.y;
					tres.z=r1.z+r2.z;
					tres.w=r1.w+r2.w;
					break;
				case OP_SUB:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}

					tres.x=r1.x-r2.x;
					tres.y=r1.y-r2.y;
					tres.z=r1.z-r2.z;
					tres.w=r1.w-r2.w;
					break;
				case OP_MUL:

					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}
					tres.x=r1.x*r2.x;
					tres.y=r1.y*r2.y;
					tres.z=r1.z*r2.z;
					tres.w=r1.w*r2.w;
					break;
				case OP_RCP:
					s=1.0f/op1->w;
					tres.x=s;
					tres.y=s;
					tres.z=s;
					tres.w=s;
					break;
				case OP_DP3:
					s=op1->x*op2->x+op1->y*op2->y+op1->z*op2->z;
					tres.x=s;
					tres.y=s;
					tres.z=s;
					tres.w=s;
					break;
				case OP_RSQ:
					s=1.0f/((float) sqrt(ff_abs(op1->w)));
					tres.x=s;
					tres.y=s;
					tres.z=s;
					tres.w=s;
					break;
				case OP_SGE:
					tres.x=(op1->x>op2->x);
					tres.y=(op1->y>op2->y);
					tres.z=(op1->z>op2->z);
					tres.w=(op1->w>op2->w);
					break;
				case OP_SLT:
					tres.x=(op1->x<op2->x);
					tres.y=(op1->y<op2->y);
					tres.z=(op1->z<op2->z);
					tres.w=(op1->w<op2->w);
					break;

				case OP_MAX:

					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}


					if (r1.x>r2.x) tres.x=r1.x; else tres.x=r2.x;
					if (r1.y>r2.y) tres.y=r1.y; else tres.y=r2.y;
					if (r1.z>r2.z) tres.z=r1.z; else tres.z=r2.z;
					if (r1.w>r2.w) tres.w=r1.w; else tres.w=r2.w;
					break;

				case OP_MIN:


					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}


					if (r1.x<r2.x) tres.x=r1.x; else tres.x=r2.x;
					if (r1.y<r2.y) tres.y=r1.y; else tres.y=r2.y;
					if (r1.z<r2.z) tres.z=r1.z; else tres.z=r2.z;
					if (r1.w<r2.w) tres.w=r1.w; else tres.w=r2.w;

					break;

				case OP_CROSS:
					v1.Init(op1->x,op1->y,op1->z);
					v2.Init(op2->x,op2->y,op2->z);
					CROSSPRODUCT(v,v1,v2);
					tres.x=v.x;
					tres.y=v.y;
					tres.z=v.z;
					tres.w=0.0f;
					break;

				case OP_M4X4:
					tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0]+op1->w*M->a[3][0];
					tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1]+op1->w*M->a[3][1];
					tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2]+op1->w*M->a[3][2];
					tres.w=op1->x*M->a[0][3]+op1->y*M->a[1][3]+op1->z*M->a[2][3]+op1->w*M->a[3][3];
					break;
				case OP_M3X3:
					tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0];
					tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1];
					tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2];
					tres.w=0.0f;
					break;

				case OP_LIT:
					tres.Init(1,0,0,1);

					power=op1->w;
					if (power<-MAXPOWER) power=-MAXPOWER;
					else if (power>MAXPOWER) power=MAXPOWER;

					if (op1->x>0)
					{
						tres.y=op1->x;
						if (op1->y>0) tres.z=(float) (pow(op1->y,power));
					}
					break;

				case OP_EQPLANE:
					{
						float d=-(op1->x*op2->x + op1->y*op2->y + op1->z*op2->z);
						tres.Init(op1->x,op1->y,op1->z,d);
					}
					break;

				case OP_ABS:
					tres.Init(f_abs(op1->x),f_abs(op1->y),f_abs(op1->z),f_abs(op1->w));
					break;

				case OP_PLANE:
					{
						float d=op1->x*op2->x + op1->y*op2->y + op1->z*op2->z + op2->w;
						tres.Init(d,d,d,d);
					}
					break;

				case OP_SIN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) sin(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;
				case OP_COS:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) cos(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_TAN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) tan(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_ASIN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) asin(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;
				case OP_ACOS:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) acos(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_ATAN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) atan(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;


				};

				(*res)=tres;
			}
			else
			{

				switch (Code[cp].op)
				{
				case OP_MOV:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					tres=r1;
					break;
				case OP_ADD:

					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}

					if (mr&C_X) tres.x=r1.x+r2.x;
					if (mr&C_Y) tres.y=r1.y+r2.y;
					if (mr&C_Z) tres.z=r1.z+r2.z;
					if (mr&C_W) tres.w=r1.w+r2.w;
					break;
				case OP_SUB:

					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}

					if (mr&C_X) tres.x=r1.x-r2.x;
					if (mr&C_Y) tres.y=r1.y-r2.y;
					if (mr&C_Z) tres.z=r1.z-r2.z;
					if (mr&C_W) tres.w=r1.w-r2.w;
					break;
				case OP_MUL:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}
					if (mr&C_X) tres.x=r1.x*r2.x;
					if (mr&C_Y) tres.y=r1.y*r2.y;
					if (mr&C_Z) tres.z=r1.z*r2.z;
					if (mr&C_W) tres.w=r1.w*r2.w;
					break;
				case OP_RCP:
					s=1.0f/op1->w;
					if (mr&C_X) tres.x=s;
					if (mr&C_Y) tres.y=s;
					if (mr&C_Z) tres.z=s;
					if (mr&C_W) tres.w=s;
					break;
				case OP_DP3:
					s=op1->x*op2->x+op1->y*op2->y+op1->z*op2->z;
					if (mr&C_X) tres.x=s;
					if (mr&C_Y) tres.y=s;
					if (mr&C_Z) tres.z=s;
					if (mr&C_W) tres.w=s;
					break;
				case OP_RSQ:
					s=1.0f/((float) sqrt(ff_abs(op1->w)));
					if (mr&C_X) tres.x=s;
					if (mr&C_Y) tres.y=s;
					if (mr&C_Z) tres.z=s;
					if (mr&C_W) tres.w=s;
					break;
				case OP_SGE:
					if (mr&C_X) tres.x=(op1->x>=op2->x);
					if (mr&C_Y) tres.y=(op1->y>=op2->y);
					if (mr&C_Z) tres.z=(op1->z>=op2->z);
					if (mr&C_W) tres.w=(op1->w>=op2->w);
					break;
				case OP_SLT:
					if (mr&C_X) tres.x=(op1->x<op2->x);
					if (mr&C_Y) tres.y=(op1->y<op2->y);
					if (mr&C_Z) tres.z=(op1->z<op2->z);
					if (mr&C_W) tres.w=(op1->w<op2->w);
					break;

				case OP_MAX:
					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}


					if (r1.x>r2.x) tres.x=r1.x; else tres.x=r2.x;
					if (r1.y>r2.y) tres.y=r1.y; else tres.y=r2.y;
					if (r1.z>r2.z) tres.z=r1.z; else tres.z=r2.z;
					if (r1.w>r2.w) tres.w=r1.w; else tres.w=r2.w;

					break;

				case OP_MIN:

					if (m1==15) r1=*op1;
					else
					{
						if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
						else
						{
							if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
							else
							{
								if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
								else
								{
									if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
									else r1=*op1;
								}
							}
						}
					}

					if (m2==15) r2=*op2;
					else
					{
						if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
						else
						{
							if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
							else
							{
								if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
								else
								{
									if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
									else r2=*op2;
								}
							}
						}
					}


					if (r1.x<r2.x) tres.x=r1.x; else tres.x=r2.x;
					if (r1.y<r2.y) tres.y=r1.y; else tres.y=r2.y;
					if (r1.z<r2.z) tres.z=r1.z; else tres.z=r2.z;
					if (r1.w<r2.w) tres.w=r1.w; else tres.w=r2.w;

					break;

				case OP_CROSS:
					v1.Init(op1->x,op1->y,op1->z);
					v2.Init(op2->x,op2->y,op2->z);
					CROSSPRODUCT(v,v1,v2);
					tres.x=v.x;
					tres.y=v.y;
					tres.z=v.z;
					tres.w=0.0f;
					break;

				case OP_M4X4:
					tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0]+op1->w*M->a[3][0];
					tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1]+op1->w*M->a[3][1];
					tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2]+op1->w*M->a[3][2];
					tres.w=op1->x*M->a[0][3]+op1->y*M->a[1][3]+op1->z*M->a[2][3]+op1->w*M->a[3][3];
					break;
				case OP_M3X3:
					tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0];
					tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1];
					tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2];
					tres.w=0.0f;
					break;


				case OP_SIN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) sin(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;
				case OP_COS:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) cos(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_TAN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) tan(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_ASIN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) asin(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;
				case OP_ACOS:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) acos(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				case OP_ATAN:
					if (m1==15)
					{
						a=op1->x;
					}
					else
					{

						if (m1&C_X) a=op1->x;
						if (m1&C_Y) a=op1->y;
						if (m1&C_Z) a=op1->z;
						if (m1&C_W) a=op1->w;
					}

					r=(float) atan(a);
					if (mr&C_X) tres.x=r;
					if (mr&C_Y) tres.y=r;
					if (mr&C_Z) tres.z=r;
					if (mr&C_W) tres.w=r;

					break;

				};

				if (mr&C_X) res->x=tres.x;
				if (mr&C_Y) res->y=tres.y;
				if (mr&C_Z) res->z=tres.z;
				if (mr&C_W) res->w=tres.w;
			}
#else

			Code[cp].function(&Code[cp]);
#endif
		}

		memcpy(&Vertex_Array[n*3],&oPos,3*sizeof(float));
		memcpy(&Color_Array[n*4],&oColor,4*sizeof(float));
		if (fvf&SPECULAR) memcpy(&Color2_Array[n*4],&oColor2,4*sizeof(float));
		if (fvf&_TEX0) memcpy(&TexCoo_Array[n*2],&oTexCoo,2*sizeof(float));
		if (fvf&_TEX1) memcpy(&TexCoo2_Array[n*2],&oTexCoo2,2*sizeof(float));

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Add(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	m1=c->m[1];
	m2=c->m[2];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;


	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}

	res->x=r1.x+r2.x;
	res->y=r1.y+r2.y;
	res->z=r1.z+r2.z;
	res->w=r1.w+r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Sub(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	m1=c->m[1];
	m2=c->m[2];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;


	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	res->x=r1.x-r2.x;
	res->y=r1.y-r2.y;
	res->z=r1.z-r2.z;
	res->w=r1.w-r2.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Mul(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	CVector4 r1,r2;
	int mr,m1,m2;

	mr=c->m[0];
	m1=c->m[1];
	m2=c->m[2];


	res=c->dst;
	op1=c->op1;
	op2=c->op2;


	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	res->x=r1.x*r2.x;
	res->y=r1.y*r2.y;
	res->z=r1.z*r2.z;
	res->w=r1.w*r2.w;


}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Dp3(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	float s;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	s=op1->x*op2->x;
	s+=op1->y*op2->y;
	s+=op1->z*op2->z;

	res->x=s;
	res->y=s;
	res->z=s;
	res->w=s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Rcp(CS_Code * c)
{
	CVector4 *res,*op1;
	float s;

	res=c->dst;
	op1=c->op1;
	s=1.0f/op1->w;
	res->x=s;
	res->y=s;
	res->z=s;
	res->w=s;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Rsq(CS_Code * c)
{
	CVector4 *res,*op1;
	float s;

	res=c->dst;
	op1=c->op1;
	s=1.0f/((float) sqrt(ff_abs(op1->w)));
	res->x=s;
	res->y=s;
	res->z=s;
	res->w=s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Max(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;



	m1=c->m[1];
	m2=c->m[2];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	res->x=(r1.x>r2.x) ? r1.x : r2.x;
	res->y=(r1.y>r2.y) ? r1.y : r2.y;
	res->z=(r1.z>r2.z) ? r1.z : r2.z;
	res->w=(r1.w>r2.w) ? r1.w : r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Min(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;



	m1=c->m[1];
	m2=c->m[2];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	res->x=(r1.x<r2.x) ? r1.x : r2.x;
	res->y=(r1.y<r2.y) ? r1.y : r2.y;
	res->z=(r1.z<r2.z) ? r1.z : r2.z;
	res->w=(r1.w<r2.w) ? r1.w : r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Sge(CS_Code * c)
{
	CVector4 *res,*op1,*op2;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	res->x=(op1->x>=op2->x) ? 1.0f : 0.0f;
	res->y=(op1->y>=op2->y) ? 1.0f : 0.0f;
	res->z=(op1->z>=op2->z) ? 1.0f : 0.0f;
	res->w=(op1->w>=op2->w) ? 1.0f : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Slt(CS_Code * c)
{
	CVector4 *res,*op1,*op2;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	res->x=(op1->x<op2->x) ? 1.0f : 0.0f;
	res->y=(op1->y<op2->y) ? 1.0f : 0.0f;
	res->z=(op1->z<op2->z) ? 1.0f : 0.0f;
	res->w=(op1->w<op2->w) ? 1.0f : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_M4x4(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 tres;
	CMatrix *M;

	res=c->dst;
	op1=c->op1;
	M=c->mat;

	tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0]+op1->w*M->a[3][0];
	tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1]+op1->w*M->a[3][1];
	tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2]+op1->w*M->a[3][2];
	tres.w=op1->x*M->a[0][3]+op1->y*M->a[1][3]+op1->z*M->a[2][3]+op1->w*M->a[3][3];

	res->x=tres.x;
	res->y=tres.y;
	res->z=tres.z;
	res->w=tres.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_M3x3(CS_Code * c)
{
	CVector4 *res,*op1,tres;
	CMatrix *M;

	res=c->dst;
	op1=c->op1;
	M=c->mat;

	tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0];
	tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1];
	tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2];
	tres.w=0.0f;

	res->x=tres.x;
	res->y=tres.y;
	res->z=tres.z;
	res->w=tres.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Cross(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	CVector v,v1,v2;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	v1.Init(op1->x,op1->y,op1->z);
	v2.Init(op2->x,op2->y,op2->z);
	CROSSPRODUCT(v,v1,v2);
	res->x=v.x;
	res->y=v.y;
	res->z=v.z;
	res->w=0.0f;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_Mov(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 r1;
	int m1;

	res=c->dst;
	op1=c->op1;

	m1=c->m[1];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	*res=r1;
}


// avec modifier

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Add(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	m1=c->m[1];
	m2=c->m[2];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;


	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}

	if (mr&C_X) res->x=r1.x+r2.x;
	if (mr&C_Y) res->y=r1.y+r2.y;
	if (mr&C_Z) res->z=r1.z+r2.z;
	if (mr&C_W) res->w=r1.w+r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Sub(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	m1=c->m[1];
	m2=c->m[2];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;


	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	if (mr&C_X) res->x=r1.x-r2.x;
	if (mr&C_Y) res->y=r1.y-r2.y;
	if (mr&C_Z) res->z=r1.z-r2.z;
	if (mr&C_W) res->w=r1.w-r2.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Mul(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	CVector4 r1,r2;
	int mr,m1,m2;

	mr=c->m[0];


	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	m1=c->m[1];
	m2=c->m[2];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}

	if (mr&C_X) res->x=r1.x*r2.x;
	if (mr&C_Y) res->y=r1.y*r2.y;
	if (mr&C_Z) res->z=r1.z*r2.z;
	if (mr&C_W) res->w=r1.w*r2.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Dp3(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	float s;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	s=op1->x*op2->x;
	s+=op1->y*op2->y;
	s+=op1->z*op2->z;

	if (mr&C_X) res->x=s;
	if (mr&C_Y) res->y=s;
	if (mr&C_Z) res->z=s;
	if (mr&C_W) res->w=s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Rcp(CS_Code * c)
{
	CVector4 *res,*op1;
	float s;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	s=1.0f/op1->w;
	if (mr&C_X) res->x=s;
	if (mr&C_Y) res->y=s;
	if (mr&C_Z) res->z=s;
	if (mr&C_W) res->w=s;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Rsq(CS_Code * c)
{
	CVector4 *res,*op1;
	float s;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	s=1.0f/((float) sqrt(ff_abs(op1->w)));
	if (mr&C_X) res->x=s;
	if (mr&C_Y) res->y=s;
	if (mr&C_Z) res->z=s;
	if (mr&C_W) res->w=s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Max(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;



	m1=c->m[1];
	m2=c->m[2];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	if (mr&C_X) res->x=(r1.x>r2.x) ? r1.x : r2.x;
	if (mr&C_Y) res->y=(r1.y>r2.y) ? r1.y : r2.y;
	if (mr&C_Z) res->z=(r1.z>r2.z) ? r1.z : r2.z;
	if (mr&C_W) res->w=(r1.w>r2.w) ? r1.w : r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Min(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr,m1,m2;
	CVector4 r1,r2;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	m1=c->m[1];
	m2=c->m[2];

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (m2==15) r2=*op2;
	else
	{
		if (m2==1) r2.Init(op2->x,op2->x,op2->x,op2->x);
		else
		{
			if (m2==2) r2.Init(op2->y,op2->y,op2->y,op2->y);
			else
			{
				if (m2==4) r2.Init(op2->z,op2->z,op2->z,op2->z);
				else
				{
					if (m2==8) r2.Init(op2->w,op2->w,op2->w,op2->w);
					else r2=*op2;
				}
			}
		}
	}


	if (mr&C_X) res->x=(r1.x<r2.x) ? r1.x : r2.x;
	if (mr&C_Y) res->y=(r1.y<r2.y) ? r1.y : r2.y;
	if (mr&C_Z) res->z=(r1.z<r2.z) ? r1.z : r2.z;
	if (mr&C_W) res->w=(r1.w<r2.w) ? r1.w : r2.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Sge(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	if (mr&C_X) res->x=(op1->x>=op2->x) ? 1.0f : 0.0f;
	if (mr&C_Y) res->y=(op1->y>=op2->y) ? 1.0f : 0.0f;
	if (mr&C_Z) res->z=(op1->z>=op2->z) ? 1.0f : 0.0f;
	if (mr&C_W) res->w=(op1->w>=op2->w) ? 1.0f : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Slt(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	if (mr&C_X) res->x=(op1->x<op2->x) ? 1.0f : 0.0f;
	if (mr&C_Y) res->y=(op1->y<op2->y) ? 1.0f : 0.0f;
	if (mr&C_Z) res->z=(op1->z<op2->z) ? 1.0f : 0.0f;
	if (mr&C_W) res->w=(op1->w<op2->w) ? 1.0f : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_M4x4(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 tres;
	CMatrix *M;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	M=c->mat;

	tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0]+op1->w*M->a[3][0];
	tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1]+op1->w*M->a[3][1];
	tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2]+op1->w*M->a[3][2];
	tres.w=op1->x*M->a[0][3]+op1->y*M->a[1][3]+op1->z*M->a[2][3]+op1->w*M->a[3][3];

	if (mr&C_X) res->x=tres.x;
	if (mr&C_Y) res->y=tres.y;
	if (mr&C_Z) res->z=tres.z;
	if (mr&C_W) res->w=tres.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_M3x3(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 tres;
	CMatrix *M;
	int mr;


	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	M=c->mat;

	tres.x=op1->x*M->a[0][0]+op1->y*M->a[1][0]+op1->z*M->a[2][0];
	tres.y=op1->x*M->a[0][1]+op1->y*M->a[1][1]+op1->z*M->a[2][1];
	tres.z=op1->x*M->a[0][2]+op1->y*M->a[1][2]+op1->z*M->a[2][2];
	tres.w=0.0f;

	if (mr&C_X) res->x=tres.x;
	if (mr&C_Y) res->y=tres.y;
	if (mr&C_Z) res->z=tres.z;
	if (mr&C_W) res->w=tres.w;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Cross(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	CVector v,v1,v2;
	int mr;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	v1.Init(op1->x,op1->y,op1->z);
	v2.Init(op2->x,op2->y,op2->z);
	CROSSPRODUCT(v,v1,v2);
	if (mr&C_X) res->x=v.x;
	if (mr&C_Y) res->y=v.y;
	if (mr&C_Z) res->z=v.z;
	if (mr&C_W) res->w=0.0f;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVPm_Mov(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 r1;
	int mr,m1;

	mr=c->m[0];
	m1=c->m[1];
	res=c->dst;
	op1=c->op1;

	if (m1==15) r1=*op1;
	else
	{
		if (m1==1) r1.Init(op1->x,op1->x,op1->x,op1->x);
		else
		{
			if (m1==2) r1.Init(op1->y,op1->y,op1->y,op1->y);
			else
			{
				if (m1==4) r1.Init(op1->z,op1->z,op1->z,op1->z);
				else
				{
					if (m1==8) r1.Init(op1->w,op1->w,op1->w,op1->w);
					else r1=*op1;
				}
			}
		}
	}

	if (mr&C_X) res->x=r1.x;
	if (mr&C_Y) res->y=r1.y;
	if (mr&C_Z) res->z=r1.z;
	if (mr&C_W) res->w=r1.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// trigo

void EVP_SIN(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;

	if (mr==15)
	{
		a=op1->x;
	}
	else
	{

		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}

	r=(float) sin(a);

	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

void EVP_ASIN(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;


	if (mr==15)
	{
		a=op1->x;
	}
	else
	{

		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}

	r=(float) asin(a);

	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_COS(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;


	if (mr==15)
	{
		a=op1->x;
	}
	else
	{

		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}


	r=(float) cos(a);

	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_ACOS(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;

	if (mr==15)
	{
		a=op1->x;
	}
	else
	{

		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}


	r=(float) acos(a);


	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_TAN(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;


	if (mr==15)
	{
		a=op1->x;
	}
	else
	{
		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}

	r=(float) tan(a);

	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

void EVP_ATAN(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[1];
	res=c->dst;
	op1=c->op1;

	if (mr==15)
	{
		a=op1->x;
	}
	else
	{

		if (mr&C_X) a=op1->x;
		if (mr&C_Y) a=op1->y;
		if (mr&C_Z) a=op1->z;
		if (mr&C_W) a=op1->w;
	}


	r=(float) atan(a);


	mr=c->m[0];

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_EXP(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;

	a=op1->w;

	r=(float) powf(2,a);

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_LOG(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	float a,r;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;

	a=op1->w;


	if (a!=0)
		r=(float) (logf(f_abs(a))/logf(2.0f));
	else
		r=-999999999.0f;

	if (mr&C_X) res->x=r;
	if (mr&C_Y) res->y=r;
	if (mr&C_Z) res->z=r;
	if (mr&C_W) res->w=r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_TGT(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	CVector up,view,v;
	int mr;
	float s;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	up.Init(op1->x,op1->y,op1->z);
	view.Init(op2->x,op2->y,op2->z);

	DOTPRODUCT(s,up,view);
	VECTORMUL(v,view,s);
	VECTORSUB(v,up,v);

	if (mr&C_X) res->x=v.x;
	if (mr&C_Y) res->y=v.y;
	if (mr&C_Z) res->z=v.z;
	if (mr&C_W) res->w=0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_FLOOR(CS_Code * c)
{
	CVector4 *res,*op1;
	int mr;
	int x,y,z,w;

	mr=c->m[0];
	res=c->dst;
	op1=c->op1;

	x=(int) op1->x;
	y=(int) op1->y;
	z=(int) op1->z;
	w=(int) op1->w;

	if (mr&C_X) res->x=(float)x;
	if (mr&C_Y) res->y=(float)y;
	if (mr&C_Z) res->z=(float)z;
	if (mr&C_W) res->w=(float)w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_LIT(CS_Code * c)
{
	CVector4 *res,*op1;
	float x,y,z,w;
	CVector4 dest;
	float power;
	const float MAXPOWER=127.9961f;


	res=c->dst;
	op1=c->op1;

	x=op1->x;	y=op1->y;	z=op1->z;	w=op1->w;

	dest.Init(1,0,0,1);

	power=w;
	if (power<-MAXPOWER) power=-MAXPOWER;
	else if (power>MAXPOWER) power=MAXPOWER;

	if (x>0)
	{
		dest.y=x;
		if (y>0) dest.z=(float) (pow(y,power));
	}

	res->x=dest.x;
	res->y=dest.y;
	res->z=dest.z;
	res->w=dest.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_PLANE(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	float d;
	CVector4 dest;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	d=op1->x*op2->x + op1->y*op2->y + op1->z*op2->z +op2->w;
	dest.Init(d,d,d,d);

	res->x=dest.x;
	res->y=dest.y;
	res->z=dest.z;
	res->w=dest.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_EQPLANE(CS_Code * c)
{
	CVector4 *res,*op1,*op2;
	float d;
	CVector4 dest;

	res=c->dst;
	op1=c->op1;
	op2=c->op2;

	d=-(op1->x*op2->x + op1->y*op2->y + op1->z*op2->z);
	dest.Init(op1->x,op1->y,op1->z,d);

	res->x=dest.x;
	res->y=dest.y;
	res->z=dest.z;
	res->w=dest.w;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EVP_ABS(CS_Code * c)
{
	CVector4 *res,*op1;
	CVector4 dest;

	res=c->dst;
	op1=c->op1;

	dest.Init(f_abs(op1->x),f_abs(op1->y),f_abs(op1->z),f_abs(op1->w));

	res->x=dest.x;
	res->y=dest.y;
	res->z=dest.z;
	res->w=dest.w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	void (*function_15)(CS_Code *c);
	void (*function_modifier)(CS_Code *c);

} data_functions_op;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
data_functions_op EVP_func[] = {

	{&EVP_Mov,&EVPm_Mov},
	{&EVP_Add,&EVPm_Add},
	{&EVP_Sub,&EVPm_Sub},
	{&EVP_Mul,&EVPm_Mul},
	{&EVP_Rcp,&EVPm_Rcp},
	{&EVP_Rsq,&EVPm_Rsq},
	{&EVP_Sge,&EVPm_Sge},
	{&EVP_Slt,&EVPm_Slt},
	{&EVP_M4x4,&EVPm_M4x4},
	{&EVP_M3x3,&EVPm_M3x3},
	{&EVP_Cross,&EVPm_Cross},
	{&EVP_Max,&EVPm_Max},
	{&EVP_Min,&EVPm_Min},
	{&EVP_Dp3,&EVPm_Dp3},
	{&EVP_SIN,&EVP_SIN},
	{&EVP_COS,&EVP_COS},
	{&EVP_TAN,&EVP_TAN},
	{&EVP_ASIN,&EVP_ASIN},
	{&EVP_ACOS,&EVP_ACOS},
	{&EVP_ATAN,&EVP_ATAN},
	{&EVP_EXP,&EVP_EXP},
	{&EVP_LOG,&EVP_LOG},
	{&EVP_TGT,&EVP_TGT},
	{&EVP_FLOOR,&EVP_FLOOR},
	{&EVP_LIT,&EVP_LIT},
	{&EVP_PLANE,&EVP_PLANE},
	{&EVP_EQPLANE,&EVP_EQPLANE},
	{&EVP_ABS,&EVP_ABS},
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmulatedVertexProgram::Add(int op,char * dst,char *op1,char *op2)
{
	char temp[128];
	char temp2[128];
	int index;


	Code[nCode].op=op;

	CSVP_GetIdentifier(dst);

	Code[nCode].dst=GetPrm(CSVP_id,CSVP_number);
	Code[nCode].m[0]=CSVP_modifier;

	Code[nCode].addressed=0;

	_sprintf(temp,"%s",op1);
	if (str_char(temp,'[')!=-1)
	{
		_sprintf(temp2,"%s",str_return_crochets(op1));

		CSVP_GetIdentifier(temp);

		temp2[str_char(temp2,'+')]='\0';

		sscanf(temp2,"%d",&index);

		Code[nCode].addressed=1;

		Code[nCode].op1=GetPrm(CSVP_id,CSVP_number) + index*sizeof(CVector4);
		Code[nCode].m[1]=CSVP_modifier;
	}
	else
	{
		CSVP_GetIdentifier(op1);
		Code[nCode].op1=GetPrm(CSVP_id,CSVP_number);
		Code[nCode].m[1]=CSVP_modifier;
	}



	if (op2)
	{
		_sprintf(temp,"%s",op2);
		if (str_char(temp,'[')!=-1)
		{
			Code[nCode].op2=NULL;
			Code[nCode].m[2]=15;


			_sprintf(temp2,"%s",str_return_crochets(op2));

			CSVP_GetIdentifier(temp);

			temp2[str_char(temp2,'+')]='\0';

			sscanf(temp2,"%d",&index);

			Code[nCode].addressed=2;

			if (GetPrm(CSVP_id,CSVP_number))
				Code[nCode].op2=GetPrm(CSVP_id,CSVP_number) + index*sizeof(CVector4);
			else
				Code[nCode].mat=mGetPrm(CSVP_id,CSVP_number) + index*sizeof(CMatrix);

			Code[nCode].m[2]=CSVP_modifier;
		}
		else
		{
			CSVP_GetIdentifier(op2);

			if (GetPrm(CSVP_id,CSVP_number))
				Code[nCode].op2=GetPrm(CSVP_id,CSVP_number);
			else
			{
				Code[nCode].op2=NULL;
				Code[nCode].mat=mGetPrm(CSVP_id,CSVP_number);
			}
			Code[nCode].m[2]=CSVP_modifier;
		}
	}


	if (Code[nCode].m[0]==15)
		Code[nCode].function=EVP_func[op].function_15;
	else
		Code[nCode].function=EVP_func[op].function_modifier;

	nCode++;
}
