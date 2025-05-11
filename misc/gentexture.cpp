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
//	@file gentexture.cpp 
//	@created September 2004
////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <math.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
#include "../base/maths.h"
#include "../base/objects3d.h"
#include "../data/png.h"
#include "gentexture.h"
#include "../base/params.h"


#ifdef _DEFINES_API_CODE_GENTEXTURE_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
char * str_parse_char(char * script,int *pos,char c);
void str_upcase(char * str);
void str_clean(char * str);
void str_clean2(char * str);
bool str_strcmp_match(char * str,char * m);
int str_char(char * str,char c);
bool str_cmp_match(char * str,char * m);
char * str_parse(char * script,int *pos);
char * str_parse_virg(char * script,int *pos);
char * str_parse_rl(char * script,int *pos);
bool str_valid(char * str);
bool str_parentheses(char * str);
void str_cut(char * str,int pos,int len,char * s);
char * str_return_parentheses(char * str);
char * str_return_crochets(char * str);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
int str_parse_space(char *line,char args[16][64])
{
	char *tmp;
	int pos=0;
	int n=0;
	char ss[512];
	sprintf(ss,"%s",line);

	tmp=str_parse_char(ss,&pos,' ');
	while (tmp)
	{
		sprintf(args[n],"%s",tmp);
		n++;
		tmp=str_parse_char(ss,&pos,' ');
	}
	return n;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void str_parse_couplet(char *str,int *x,int *y)
{
	char *tmp=str_return_parentheses(str);
	sscanf(tmp,"%d,%d",x,y);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void str_parse_triplet(char *str,int *x,int *y,int *z)
{
	char *tmp=str_return_parentheses(str);
	sscanf(tmp,"%d,%d,%d",x,y,z);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
char * CTextureGeneration::loadScript(const char *name)
{
	char png_name[512];
	char ss[512];
	char rep[512];
	char *buffer;
	FILE *f;
	int n;

	sprintf(rep,"%s",name);
	n=(int)strlen(rep)-1;
	while ((n>0)&&(rep[n]!='/')&&(rep[n]!=92)) n--;
	rep[n]='\0';


	f=fopen(name,"r");
	if(f==NULL) {
	  return NULL;
	}
	fscanf(f,"%s",ss);
	fscanf(f,"%s",png_name);
	buffer=(char*) malloc(10000);

	n=0;
	while (strcmp(ss,"END")!=0)
	{
		fgets(ss,512,f);
		str_clean2(ss);
		memcpy(&buffer[n],ss,strlen(ss));
		n+=(int)strlen(ss);
		buffer[n]='\n';
		n++;
		str_clean(ss);
	}
	fclose(f);
	buffer[n]='\0';


	sprintf(ss,"%s/%s",rep,png_name);
	sprintf(mask_name,"%s",ss);

	READ_PNG(ss,&mask,ALPHA1);
	readScript((char*) buffer,mask.ptrImageDATAS);

	return buffer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void CTextureGeneration::smooth(unsigned char * ptr,int sx,int sy)
{
	int n1,n2;
	int x,y;
	unsigned int c,n;
	unsigned char * tmp=(unsigned char*) malloc(sx*sy);
	int p[7][7]={	{ 0, 0, 0, 0, 0, 0, 0},
					{ 0,0 ,0 ,0 ,0 ,0 , 0},
					{ 0,0 ,1 ,2 ,1 ,0 , 0},
					{ 0,0 ,2 ,3 ,2 ,0 , 0},
					{ 0,0 ,1 ,2 ,1 ,0 , 0},
					{ 0,0 ,0 ,0 ,0 ,0 , 0},
					{ 0, 0, 0, 0, 0, 0, 0 }
	};

	for (n2=0;n2<sy;n2++)
	{
		for (n1=0;n1<sx;n1++)
		{
			c=n=0;
			for (x=-3;x<=3;x++)
				for (y=-3;y<=3;y++)
				{
					c+=p[x+3][y+3]*((int)(ptr[((sx+n1+x)%sx)+(((sy+n2+y)%sy)*sx)]&255));
					n+=p[x+3][y+3];
				}
			tmp[n1+sx*n2]=c/n;
		}
	}

	memcpy(ptr,tmp,sx*sy);
	free(tmp);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void CTextureGeneration::readScript(char * script,char * image1,char * image2,char * image3,char * image4)
{

	int pos=0;
	char * line;
	char args[16][64];
	int nb,n,n1,n2,num,num2;
	int sizex,sizey;
	CRGBA gradiant[8][256];
	CRGBA rgb;
	char * images[4];
	float t,t2,tx,ty,hh;
	int x,y;
	int c,c1,c2,m;
	char *mask;
	int r,g,b;
	int r2,g2,b2;
	bool end=false;
	CRGBA rgb1,rgb2;
	char ss[512];
	
	line=str_parse_char(script,&pos,'\n');
	while ((line)&&(!end))
	{
		sprintf(ss,"%s",line);
		str_clean(ss);
		if (strcmp(ss,"END")==0) end=true;
		
		sprintf(args[0],"");
		sprintf(args[1],"");
		sprintf(args[2],"");
		sprintf(args[3],"");
		sprintf(args[4],"");
		sprintf(args[5],"");
		sprintf(args[6],"");
		sprintf(args[7],"");
		sprintf(args[8],"");
		sprintf(args[9],"");

		if (strlen(ss)>0)
		{
			nb=str_parse_space(line,args);

			if (nb>0)
			{
				if (strcmp(args[0],"RESULT")==0)
				{
					sscanf(args[6],"%d",&num2);
					sscanf(args[4],"%d",&num);
					sscanf(args[2],"%d",&m);

					num--;
					num2--;

					char * layer=(char*) malloc(sizex*sizey);
					memset(layer,0,sizex*sizey);

					char * layer2=(char*) malloc(sizex*sizey);
					memset(layer2,255,sizex*sizey);

					for (x=0;x<sizex;x++)
						for (y=0;y<sizey;y++)
						{
							if ((mask[x+y*sizex]&255)==m)
							{
								layer[x+y*sizex]=(char) 255;
							}
						}

					if ((strcmp(args[7],"SHADOW")==0)||(strcmp(args[8],"SHADOW")==0)||(strcmp(args[9],"SHADOW")==0))
					{
						for (x=0;x<sizex;x++)
							for (y=10;y<sizey;y++)
							{
								if ((mask[x+(y-10)*sizex]&255)==m)
								{
									layer2[x+y*sizex]=(char) 128;
								}
							}

						smooth((unsigned char*)layer2,sizex,sizey);
					}

					if ((strcmp(args[7],"SHADOW2")==0)||(strcmp(args[8],"SHADOW2")==0)||(strcmp(args[9],"SHADOW2")==0))
					{
						for (x=0;x<sizex;x++)
							for (y=10;y<sizey;y++)
							{
								if ((mask[x+(y-5)*sizex]&255)==m)
								{
									layer2[x+y*sizex]=(char) 128;
								}
							}

						smooth((unsigned char*)layer2,sizex,sizey);
					}

					if ((strcmp(args[7],"BEVEL")==0)||(strcmp(args[8],"BEVEL")==0)||(strcmp(args[9],"BEVEL")==0))
					{
						smooth((unsigned char*)layer,sizex,sizey);
						smooth((unsigned char*)layer,sizex,sizey);
					}

					if ((strcmp(args[7],"BEVEL2")==0)||(strcmp(args[8],"BEVEL2")==0)||(strcmp(args[9],"BEVEL2")==0))
					{
						smooth((unsigned char*)layer,sizex,sizey);
						smooth((unsigned char*)layer,sizex,sizey);
						smooth((unsigned char*)layer,sizex,sizey);
						smooth((unsigned char*)layer,sizex,sizey);
					}

					for (x=0;x<sizex;x++)
						for (y=0;y<sizey;y++)
						{

							t=((float)(layer[x+y*sizex]&255))/255.0f;
							t2=((float)(layer2[x+y*sizex]&255))/255.0f;

							r2=image[4*(x+y*sizex)+0]&255;
							g2=image[4*(x+y*sizex)+1]&255;
							b2=image[4*(x+y*sizex)+2]&255;

							r2=(int)(t2*r2);
							g2=(int)(t2*g2);
							b2=(int)(t2*b2);

							n=images[num][x+sizex*y]&255;
							r=(int)(255*gradiant[num2][n].r);
							g=(int)(255*gradiant[num2][n].g);
							b=(int)(255*gradiant[num2][n].b);

							
							image[4*(x+y*sizex)+0]=(unsigned char)(r*t +r2*(1-t));
							image[4*(x+y*sizex)+1]=(unsigned char)(g*t +g2*(1-t));
							image[4*(x+y*sizex)+2]=(unsigned char)(b*t +b2*(1-t));
							image[4*(x+y*sizex)+3]=(unsigned char)(255);
							
						}

					free(layer);
					free(layer2);

				}

				if (strcmp(args[0],"MASK")==0)
				{
					sscanf(args[2],"%d",&num);
					num--;
					mask=images[num];
				}

				if (strcmp(args[0],"FORMAT")==0)
				{
					str_parse_couplet(args[1],&sizex,&sizey);

					if (image1) images[0]=image1; else images[0]=(char*) malloc(sizex*sizey);
					if (image2) images[1]=image2; else images[1]=(char*) malloc(sizex*sizey);
					if (image3) images[2]=image3; else images[2]=(char*) malloc(sizex*sizey);
					if (image4) images[3]=image4; else images[3]=(char*) malloc(sizex*sizey);

					image=(unsigned char*) malloc(sizex*sizey*4);
					w=sizex;
					h=sizey;
					memset(image,0,w*h*4);
				}

				if (strcmp(args[0],"IMAGE")==0)
				{
					sscanf(args[1],"%d",&num);

					num--;

					if (strcmp(args[2],"BOIS")==0)
					{
						for (x=0;x<sizex;x++)
							for (y=0;y<sizey;y++)
							{
								tx=(float) x/sizex;
								ty=(float) y/sizey;
								hh=f_abs((float)(sin(8*tx*PI)*sin(2*ty*PI)*sin(2*(tx+ty)*PI)));
								images[num][x+y*sizex]=(char) ((int)(255*hh));
							}
					}

					if (strcmp(args[2],"SMOOTH")==0)
					{
						smooth((unsigned char*)images[num],sizex,sizey);
						smooth((unsigned char*)images[num],sizex,sizey);
					}

					if (strcmp(args[2],"IMAGE")==0)
					{
						sscanf(args[3],"%d",&num2);
						num2--;

						if (strcmp(args[4],"MUL")==0)
						{
							for (x=0;x<sizex;x++)
								for (y=0;y<sizey;y++)
								{
									c1=images[num][x+y*sizex]&255;
									c2=images[num2][x+y*sizex]&255;
									c=(c1*c2)/255;
									if (c>255) c=255;
									if (c<0) c=0;
									images[num][x+y*sizex]=c;
								}
						}

						if (strcmp(args[4],"ADD")==0)
						{
							for (x=0;x<sizex;x++)
								for (y=0;y<sizey;y++)
								{
									c1=images[num][x+y*sizex]&255;
									c2=images[num2][x+y*sizex]&255;
									c=c1+c2;
									if (c>255) c=255;
									if (c<0) c=0;
									images[num][x+y*sizex]=c;
								}
						}
					}

					if (strcmp(args[2],"RANDOM")==0)
					{
						int rnd;
						sscanf(args[3],"%d",&rnd);

						for (x=0;x<sizex;x++)
							for (y=0;y<sizey;y++)
							{
								images[num][x+y*sizex]=rand()%rnd;
							}
					}

					if (strcmp(args[2],"RAIN")==0)
					{
						int rnd,number;
						sscanf(args[3],"%d",&rnd);
						sscanf(args[4],"%d",&number);

						memset(images[num],0,sizex*sizey);

						for (n=0;n<number;n++)
						{
							x=rand()%sizex;
							y=rand()%sizey;

							c=rand()%rnd;
							

							for (n1=0;n1<40;n1++)
							{
								images[num][x+((y+n1)%sizey)*sizex]=c;
							}
						}

					}

					if (strcmp(args[2],"RAINX")==0)
					{
						int rnd,number;
						sscanf(args[3],"%d",&rnd);
						sscanf(args[4],"%d",&number);

						memset(images[num],0,sizex*sizey);

						for (n=0;n<number;n++)
						{
							x=rand()%sizex;
							y=rand()%sizey;

							c=rand()%rnd;
							

							for (n1=0;n1<40;n1++)
							{
								images[num][((x+n1)%sizex) + y*sizex]=c;
							}
						}

					}
				}

				if (strcmp(args[0],"GRADIANT")==0)
				{
					sscanf(args[1],"%d",&num);
					num--;

					if (strcmp(args[2],"RANDOMIZE")==0)
					{
						for (n1=0;n1<256;n1++)
							for (n2=0;n2<256;n2++)
							{
								if ((rand()%100)>50)
								{
									rgb=gradiant[num][n1];
									gradiant[num][n1]=gradiant[num][n2];
									gradiant[num][n2]=rgb;
	
								}
							}
					}

					if (strcmp(args[2],"DEGRADE")==0)
					{
						str_parse_triplet(args[3],&r,&g,&b);
						rgb1.Init((float) r/255.0f,(float) g/255.0f,(float) b/255.0f,1.0f);

						str_parse_triplet(args[4],&r,&g,&b);
						rgb2.Init((float) r/255.0f,(float) g/255.0f,(float) b/255.0f,1.0f);

						for (n=0;n<256;n++)
						{
							t=(float) n/256.0f;
							rgb=rgb1*(1-t) + rgb2*t;
							gradiant[num][n]=rgb;
						}
					}

				}

			}
		}
		line=str_parse_char(script,&pos,'\n');
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
char *CTextureGeneration::getMaskName()
{
	return mask_name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
struct IMAGE_DATAS * CTextureGeneration::getMask()
{
	return &mask;
}
#endif
