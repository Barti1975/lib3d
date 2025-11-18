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
//	@file softrenderer.cpp
//	@date 2013-01-07
////////////////////////////////////////////////////////////////////////

#include "../base/params.h"

#ifdef _DEFINES_API_CODE_SOFTRENDERER_

#include "softrenderer.h"
#include "../base/objects3d.h"

#include "textfont.h"

const char *charconverttextfont=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~";
int lencharconverttextfont=-1;


int schr(char c)
{
    for (int n=0;n<lencharconverttextfont;n++)
        if (charconverttextfont[n]==c) return n;
    return 0;
}


void CSoftwareRenderer::Print(int xc,int yc,char *str,int r,int g,int b)
{
    unsigned int col=SR_ALPHA(255)+SR_COLOR(r,g,b);
    
    if (lencharconverttextfont<0) lencharconverttextfont=(int)strlen(charconverttextfont);
    
    for (int n=0;n<(int)strlen(str);n++)
    {
        int ofs=schr(str[n])*7*9;
        
        for (int y=0;y<9;y++)
        {
            for (int x=0;x<7;x++)
            {
                int c=softrenderbinaryfont[ofs+x+7*y]&255;
                if (c!=0) SetValue(xc+n*8+x,yc+y,col);
            }
        }
    }
}

void CSoftwareRenderer::Line(int x1,int y1,int x2,int y2,int r,int g,int b)
{
    unsigned int col=SR_ALPHA(255)+SR_COLOR(r,g,b);
    
    int dx=x1-x2;
    int dy=y1-y2;
    if (dx<0) dx=-dx;
    if (dy<0) dy=-dy;
    int len=dx;
    if (dy>len) len=dy;
    
    float incx=((float)(x2-x1))/len;
    float incy=((float)(y2-y1))/len;
    float xx=(float)x1;
    float yy=(float)y1;
    
    for (int n=0;n<len;n++)
    {
        SetValue((int)xx,(int)yy,col);
        xx+=incx;
        yy+=incy;
    }
}


void CSoftwareRenderer::SetParams(int q,int c)
{
	switch (q)
	{
	case CSR_ZBUFFER:
		zfunc=c;
		break;
	};
}

void CSoftwareRenderer::Free()
{
	int n;
	for (n=0;n<16;n++) 
	{
		if (surfaces[n]) free(surfaces[n]);

		surfaces[n]=NULL;
	}
	surface=NULL;
    
    if (surfacemap) delete [] surfacemap;
    surfacemap=NULL;
}


void CSoftwareRenderer::SetScreen(int x,int y)
{
	wx=x;
	wy=y;
}

void CSoftwareRenderer::SetRenderTarget(int n)
{
	surface=surfaces[n];
}

float CSoftwareRenderer::GetZ(int x,int y)
{
	float z=0;
	if (surface)
        if ((y>=0)&&(y<wy)&&(x>=0)&&(x<wx)) z=surface[x+y*wx].z;
	return z;
}

float CSoftwareRenderer::GetZaround(int x,int y)
{
	float z=-1;

	z=GetZ(x,y);
	if (z>back-1) z=GetZ(x+1,y);
	if (z>back-1) z=GetZ(x-1,y);
	if (z>back-1) z=GetZ(x,y+1);
	if (z>back-1) z=GetZ(x,y-1);
	if (z>back-1) z=GetZ(x+1,y+1);
	if (z>back-1) z=GetZ(x-1,y+1);
	if (z>back-1) z=GetZ(x+1,y-1);
	if (z>back-1) z=GetZ(x-1,y-1);
	if (z>back-1) z=GetZ(x+2,y);
	if (z>back-1) z=GetZ(x-2,y);
	if (z>back-1) z=GetZ(x,y+2);
	if (z>back-1) z=GetZ(x,y-2);

	return z;
}


CVector2 CSoftwareRenderer::GetCoo(int x,int y)
{
    CVector2 map(0,0);
    if (surfacemap)
        if ((y>=0)&&(y<wy)&&(x>=0)&&(x<wx))
        {
            map=surfacemap[x+y*wx];
        }
    return map;
}


unsigned int CSoftwareRenderer::GetValue(int x,int y)
{
	unsigned int c=0;
	if (surface)
        if ((y>=0)&&(y<wy)&&(x>=0)&&(x<wx)) c=surface[x+y*wx].value;
	return c;
}

void CSoftwareRenderer::SetValue(int x,int y,unsigned int c)
{
	if (surface)
	if ((y>=0)&&(y<wy)&&(x>=0)&&(x<wx)) surface[x+y*wx].value=c;
}


void CSoftwareRenderer::Fill(int line,int xd0,int xf0,int c,float z)
{
	CVector2 uv;
	uv.Init(0,0);
	Fill(line,xd0,xf0,c,c,z,z,1.0f,1.0f,uv,uv,uv,uv,uv,uv,0);
}

void CSoftwareRenderer::FillZ(int line,int xd0,int xf0,int cd,int cf,float zd,float zf,float izd,float izf,CVector2 &t0d,CVector2 &t0f,CVector2 &t1d,CVector2 &t1f,CVector2 &t2d,CVector2 &t2f,int flags)
{
	int n;
	int xd=xd0;
	int xf=xf0;
	int len;
	unsigned int adr;
	CVector2 inc0,inc1,inc2;
	CVector2 u0,u1,u2;
	CVector2 tex0,tex1,tex2;
	CVector2 u0d,u0f;
	CVector2 u1d,u1f;
	CVector2 u2d,u2f;
	float zzd,zzf;
	CRGBA c,colord,colorf,color,inccolor,bkc,tmp;
	float z,zz,iz,inc_iz,inc_zz;
	int x,y;

	if ((line<0)||(line>=wy)) return;

	if (flags&CSR_FILL_COLOR)
	{
		colord.Init(SR_GET_R(cd),SR_GET_G(cd),SR_GET_B(cd));
		colorf.Init(SR_GET_R(cf),SR_GET_G(cf),SR_GET_B(cf));
		colord.r*=izd; colord.g*=izd; colord.b*=izd;
		colorf.r*=izf; colorf.g*=izf; colorf.b*=izf;
	}

	if (flags&CSR_FILL_TEX0)
	{
		u0d=t0d*izd;
		u0f=t0f*izf;
	}

	if (flags&CSR_FILL_TEX1)
	{
		u1d=t1d*izd;
		u1f=t1f*izf;
	}

	if (flags&CSR_FILL_TEX2)
	{
		u2d=t2d*izd;
		u2f=t2f*izf;
	}

	if (flags&CSR_FILL_Z)
	{
		zzd=zd*izd;
		zzf=zf*izf;
	}

	if ((xd<wx)&&(xf>0))
	{
		if (xd<0) xd=0;
		if (xf>wx) xf=wx;
		len=xf-xd;
		float flen=(float) len;
		adr=line*wx+xd;

		iz=izd;
		zz=zzd;

		inc_iz=(izf-izd)/len;

		if (flags&CSR_FILL_COLOR) { inccolor=(colorf-colord)/flen; color=colord; }
		if (flags&CSR_FILL_TEX0) inc0=(u0f-u0d)/flen;
		if (flags&CSR_FILL_TEX1) inc1=(u1f-u1d)/flen;
		if (flags&CSR_FILL_TEX2) inc2=(u2f-u2d)/flen;
		if (flags&CSR_FILL_Z) inc_zz=(zzf-zzd)/flen; else z=zd;

		if (flags&CSR_FILL_TEX0) u0=u0d; 
		if (flags&CSR_FILL_TEX1) u1=u1d; 
		if (flags&CSR_FILL_TEX2) u2=u2d;

		if (flags&CSR_FILL_COLOR) c=color/iz;
		if (flags&CSR_FILL_TEX0) tex0=u0/iz;
		if (flags&CSR_FILL_TEX1) tex1=u1/iz;
		if (flags&CSR_FILL_TEX2) tex2=u2/iz;

		bkc.Init(0,0,0,0);
        
		for (n=0;n<len;n++)
		{
			c.a=1.0f;

			if (flags&CSR_FILL_Z) 
			{
				z=zz/iz;
				zz+=inc_zz;
			}

			if ((flags&CSR_FILL_Z)&&(flags&CSR_FILL_COLOR)) 
			{
				c=color/iz;
				color=color + inccolor;
			}

			if (flags&CSR_FILL_TEX0)
			{
				tex0= u0 / iz;
				u0= u0 + inc0;
                
                if ((flags&CSR_FILL_ALPHA)&&(tex_ptr1))
                {
                    x=((int) (tex0.x*szx1));
                    x=x%szx1;
                    y=((int) (tex0.y*szy1));
                    y=y%szy1;
                    if (x<0) x=szx1+x;
                    if (y<0) y=szy1+y;
                    c.a=((float) tex_ptr1[(x%szx1)+(y%szy1)*szx1])/255.0f;
                    c.r=1.0f;
                    c.g=1.0f;
                    c.b=1.0f;
                }
                
                if (((flags&CSR_FILL_ALPHA)==0)&&(tex_ptr1))
                {
                    x=((int) (tex0.x*szx1));
                    float tx=tex0.x*szx1-x;
                    x=x%szx1;
                    y=((int) (tex0.y*szy1));
                    float ty=tex0.y*szy1-y;
                    y=y%szy1;
                    if (x<0) x=szx1+x;
                    if (y<0) y=szy1+y;
                    c.a=1.0f;
                    
                    int r00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+0]);
                    int g00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+1]);
                    int b00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+2]);
                    int a00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+3]);

                    int r01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+0]);
                    int g01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+1]);
                    int b01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+2]);
                    int a01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+3]);

                    int r10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+0]);
                    int g10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+1]);
                    int b10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+2]);
                    int a10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+3]);

                    int r11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+0]);
                    int g11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+1]);
                    int b11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+2]);
                    int a11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+3]);
                    
                    int ra=(int)(r00+tx*(r01-r00));
                    int ga=(int)(g00+tx*(g01-g00));
                    int ba=(int)(b00+tx*(b01-b00));
                    int aa=(int)(a00+tx*(a01-a00));

                    int rb=(int)(r10+tx*(r11-r10));
                    int gb=(int)(g10+tx*(g11-g10));
                    int bb=(int)(b10+tx*(b11-b10));
                    int ab=(int)(a10+tx*(a11-a10));

                    int r=(int)(ra+ty*(rb-ra));
                    int g=(int)(ga+ty*(gb-ga));
                    int b=(int)(ba+ty*(bb-ba));
                    int a=(int)(aa+ty*(ab-aa));

                    c.r=((float) r)/255.0f;
                    c.g=((float) g)/255.0f;
                    c.b=((float) b)/255.0f;
                    c.a=((float) a)/255.0f;
                }
			}

			if (flags&CSR_FILL_TEX1)
			{
				tex1= u1 / iz;
				u1= u1 + inc1;
			}

			if (flags&CSR_FILL_TEX2)
			{
				tex2= u2 / iz;
				u2= u2 + inc2;
			}

            if (z>0)
            {
                if (flags&CSR_FILL_DZ)
                {
                    if ((z>=Zmin)&&(z<=Zmax))
			        if (((z<surface[adr+n].z)&&(zfunc==CSR_NORMAL))||((z>surface[adr+n].z)&&(zfunc==CSR_INVERSE))||(zfunc==CSR_DISABLED))
			        {
                        if (flags&CSR_FILL_GOURAUD)
                        {
                            tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                            tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                            tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                            tmp.a=1.0f;
                            unsigned int al=(unsigned int)(tmp.a*255);

                            surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                            surface[adr+n].z=z;
                        }
                        else
                        {
                            if ((surfacemap)&&(tex_ptr1))
                            {
                                surfacemap[adr+n].x=tex0.x;
                                surfacemap[adr+n].y=tex0.y;
                                surface[adr+n].z=z;
                            }
                            else
                            {
                                if (flags&CSR_FILL_ALPHA)
                                {
                                    if (c.a>SMALLF2)
                                    {
                                        bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                                        tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                        tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                        tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                        tmp.a=1.0f;
                                        surface[adr+n].value=SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                        surface[adr+n].z=z;
                                    }
                                }
                                else
                                {
                                    if (tex_ptr1)
                                    {
                                        bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                            
                                        if (flags&CSR_FILL_BLEND)
                                        {
                                            if ((surface[adr+n].value>>24)==0)
                                            {
                                                tmp.r=c.r;
                                                tmp.g=c.g;
                                                tmp.b=c.b;
                                            }
                                            else
                                            {
                                                tmp.r=0.5f*c.r + 0.5f*bkc.r;
                                                tmp.g=0.5f*c.g + 0.5f*bkc.g;
                                                tmp.b=0.5f*c.b + 0.5f*bkc.b;
                                            }
                                            tmp.a=c.a;
                                        }
                                        else
                                        {
                                            tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                            tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                            tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                            tmp.a=1.0f;
                                        }
                            
                                        unsigned int al=(unsigned int)(tmp.a*255);
                                        surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                    }
                                    else surface[adr+n].value=cd;
                        
                                    surface[adr+n].z=z;
                                }
                            }
                        }
                    }
                }
                else
			    if (((z<surface[adr+n].z)&&(zfunc==CSR_NORMAL))||((z>surface[adr+n].z)&&(zfunc==CSR_INVERSE))||(zfunc==CSR_DISABLED))
			    {
                    if ((surfacemap)&&(tex_ptr1))
                    {
                        surfacemap[adr+n].x=tex0.x;
                        surfacemap[adr+n].y=tex0.y;
                        surface[adr+n].z=z;
                    }
                    else
                    {
                        if (flags&CSR_FILL_ALPHA)
                        {
                            if (c.a>SMALLF2)
                            {
                                bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                                tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                tmp.a=1.0f;
                                surface[adr+n].value=SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                surface[adr+n].z=z;
                            }
                        }
                        else
                        {
                            if (tex_ptr1)
                            {
                                bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                            
                                if (flags&CSR_FILL_BLEND)
                                {
                                    if ((surface[adr+n].value>>24)==0)
                                    {
                                        tmp.r=c.r;
                                        tmp.g=c.g;
                                        tmp.b=c.b;
                                    }
                                    else
                                    {
                                        tmp.r=0.5f*c.r + 0.5f*bkc.r;
                                        tmp.g=0.5f*c.g + 0.5f*bkc.g;
                                        tmp.b=0.5f*c.b + 0.5f*bkc.b;
                                    }
                                    tmp.a=c.a;
                                }
                                else
                                {
                                    tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                    tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                    tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                    tmp.a=1.0f;
                                }
                            
                                unsigned int al=(unsigned int)(tmp.a*255);
                                surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                            }
                            else surface[adr+n].value=cd;
                        
                            surface[adr+n].z=z;
                        }
                    }
                }
			}

			iz+=inc_iz;
		}
	}
}

void CSoftwareRenderer::Fill(int line,int xd0,int xf0,int cd,int cf,float zd,float zf,float izd,float izf,CVector2 &t0d,CVector2 &t0f,CVector2 &t1d,CVector2 &t1f,CVector2 &t2d,CVector2 &t2f,int flags)
{
	int n;
	int xd=xd0;
	int xf=xf0;
	int len;
	unsigned int adr;
	CVector2 inc0,inc1,inc2;
	CVector2 u0,u1,u2;
	CVector2 tex0,tex1,tex2;
	CVector2 u0d,u0f;
	CVector2 u1d,u1f;
	CVector2 u2d,u2f;
	float zzd,zzf;
	CRGBA c,colord,colorf,color,inccolor,bkc,tmp;
	float z,zz,iz,inc_iz,inc_zz;
	int x,y;

	if ((line<0)||(line>=wy)) return;

	if (flags&CSR_FILL_COLOR)
	{
		colord.Init(SR_GET_R(cd),SR_GET_G(cd),SR_GET_B(cd));
		colorf.Init(SR_GET_R(cf),SR_GET_G(cf),SR_GET_B(cf));
		colord.r*=izd; colord.g*=izd; colord.b*=izd;
		colorf.r*=izf; colorf.g*=izf; colorf.b*=izf;
	}

	if (flags&CSR_FILL_TEX0)
	{
		u0d=t0d;
		u0f=t0f;
	}

	if (flags&CSR_FILL_TEX1)
	{
		u1d=t1d;
		u1f=t1f;
	}

	if (flags&CSR_FILL_TEX2)
	{
		u2d=t2d;
		u2f=t2f;
	}

	if (flags&CSR_FILL_Z)
	{
		zzd=zd*izd;
		zzf=zf*izf;
	}

	if ((xd<wx)&&(xf>0))
	{
		if (xd<0) xd=0;
		if (xf>wx) xf=wx;
		len=xf-xd;
		float flen=(float) len;
		adr=line*wx+xd;

		iz=izd;
		zz=zzd;

		inc_iz=(izf-izd)/len;

		if (flags&CSR_FILL_COLOR) { inccolor=(colorf-colord)/flen; color=colord; }
		if (flags&CSR_FILL_TEX0) inc0=(u0f-u0d)/flen;
		if (flags&CSR_FILL_TEX1) inc1=(u1f-u1d)/flen;
		if (flags&CSR_FILL_TEX2) inc2=(u2f-u2d)/flen;
		if (flags&CSR_FILL_Z) inc_zz=(zzf-zzd)/flen; else z=zd;

		if (flags&CSR_FILL_TEX0) u0=u0d; 
		if (flags&CSR_FILL_TEX1) u1=u1d; 
		if (flags&CSR_FILL_TEX2) u2=u2d;

		if (flags&CSR_FILL_COLOR) c=color;
		if (flags&CSR_FILL_TEX0) tex0=u0;
		if (flags&CSR_FILL_TEX1) tex1=u1;
		if (flags&CSR_FILL_TEX2) tex2=u2;

		bkc.Init(0,0,0,0);
        
		for (n=0;n<len;n++)
		{
			c.a=1.0f;

			if (flags&CSR_FILL_Z) 
			{
				z=zz/iz;
				zz+=inc_zz;
			}

			if ((flags&CSR_FILL_Z)&&(flags&CSR_FILL_COLOR)) 
			{
				c=color;
				color=color + inccolor;
			}

			if (flags&CSR_FILL_TEX0)
			{
				tex0= u0;
				u0= u0 + inc0;
                
                if ((flags&CSR_FILL_ALPHA)&&(tex_ptr1))
                {
                    x=((int) (tex0.x*szx1));
                    x=x%szx1;
                    y=((int) (tex0.y*szy1));
                    y=y%szy1;
                    if (x<0) x=szx1+x;
                    if (y<0) y=szy1+y;
                    c.a=((float) tex_ptr1[(x%szx1)+(y%szy1)*szx1])/255.0f;
                    c.r=1.0f;
                    c.g=1.0f;
                    c.b=1.0f;
                }
                
                if (((flags&CSR_FILL_ALPHA)==0)&&(tex_ptr1))
                {
                    x=((int) (tex0.x*szx1));
                    float tx=tex0.x*szx1-x;
                    x=x%szx1;
                    y=((int) (tex0.y*szy1));
                    float ty=tex0.y*szy1-y;
                    y=y%szy1;
                    if (x<0) x=szx1+x;
                    if (y<0) y=szy1+y;
                    c.a=1.0f;
                    
                    int r00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+0]);
                    int g00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+1]);
                    int b00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+2]);
                    int a00=((int) tex_ptr1[4*(((x+0)%szx1)+((y+0)%szy1)*szx1)+3]);

                    int r01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+0]);
                    int g01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+1]);
                    int b01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+2]);
                    int a01=((int) tex_ptr1[4*(((x+1)%szx1)+((y+0)%szy1)*szx1)+3]);

                    int r10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+0]);
                    int g10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+1]);
                    int b10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+2]);
                    int a10=((int) tex_ptr1[4*(((x+0)%szx1)+((y+1)%szy1)*szx1)+3]);

                    int r11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+0]);
                    int g11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+1]);
                    int b11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+2]);
                    int a11=((int) tex_ptr1[4*(((x+1)%szx1)+((y+1)%szy1)*szx1)+3]);
                    
                    int ra=(int)(r00+tx*(r01-r00));
                    int ga=(int)(g00+tx*(g01-g00));
                    int ba=(int)(b00+tx*(b01-b00));
                    int aa=(int)(a00+tx*(a01-a00));

                    int rb=(int)(r10+tx*(r11-r10));
                    int gb=(int)(g10+tx*(g11-g10));
                    int bb=(int)(b10+tx*(b11-b10));
                    int ab=(int)(a10+tx*(a11-a10));

                    int r=(int)(ra+ty*(rb-ra));
                    int g=(int)(ga+ty*(gb-ga));
                    int b=(int)(ba+ty*(bb-ba));
                    int a=(int)(aa+ty*(ab-aa));

                    c.r=((float) r)/255.0f;
                    c.g=((float) g)/255.0f;
                    c.b=((float) b)/255.0f;
                    c.a=((float) a)/255.0f;
                }
			}

			if (flags&CSR_FILL_TEX1)
			{
				tex1= u1;
				u1= u1 + inc1;
			}

			if (flags&CSR_FILL_TEX2)
			{
				tex2= u2;
				u2= u2 + inc2;
			}

            if (z>0)
            {
                if (flags&CSR_FILL_DZ)
                {
                    if ((z>=Zmin)&&(z<=Zmax))
			        if (((z<surface[adr+n].z)&&(zfunc==CSR_NORMAL))||((z>surface[adr+n].z)&&(zfunc==CSR_INVERSE))||(zfunc==CSR_DISABLED))
			        {
                        if (flags&CSR_FILL_GOURAUD)
                        {
                            tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                            tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                            tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                            tmp.a=1.0f;
                            unsigned int al=(unsigned int)(tmp.a*255);

                            surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                            surface[adr+n].z=z;
                        }
                        else
                        {
                            if ((surfacemap)&&(tex_ptr1))
                            {
                                surfacemap[adr+n].x=tex0.x;
                                surfacemap[adr+n].y=tex0.y;
                                surface[adr+n].z=z;
                            }
                            else
                            {
                                if (flags&CSR_FILL_ALPHA)
                                {
                                    if (c.a>SMALLF2)
                                    {
                                        bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                                        tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                        tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                        tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                        tmp.a=1.0f;
                                        surface[adr+n].value=SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                        surface[adr+n].z=z;
                                    }
                                }
                                else
                                {
                                    if (tex_ptr1)
                                    {
                                        bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                            
                                        if (flags&CSR_FILL_BLEND)
                                        {
                                            if ((surface[adr+n].value>>24)==0)
                                            {
                                                tmp.r=c.r;
                                                tmp.g=c.g;
                                                tmp.b=c.b;
                                            }
                                            else
                                            {
                                                tmp.r=0.5f*c.r + 0.5f*bkc.r;
                                                tmp.g=0.5f*c.g + 0.5f*bkc.g;
                                                tmp.b=0.5f*c.b + 0.5f*bkc.b;
                                            }
                                            tmp.a=c.a;
                                        }
                                        else
                                        {
                                            tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                            tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                            tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                            tmp.a=1.0f;
                                        }
                            
                                        unsigned int al=(unsigned int)(tmp.a*255);
                                        surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                    }
                                    else surface[adr+n].value=cd;
                        
                                    surface[adr+n].z=z;
                                }
                            }
                        }
                    }
                }
                else
			    if (((z<surface[adr+n].z)&&(zfunc==CSR_NORMAL))||((z>surface[adr+n].z)&&(zfunc==CSR_INVERSE))||(zfunc==CSR_DISABLED))
			    {
                    if ((surfacemap)&&(tex_ptr1))
                    {
                        surfacemap[adr+n].x=tex0.x;
                        surfacemap[adr+n].y=tex0.y;
                        surface[adr+n].z=z;
                    }
                    else
                    {
                        if (flags&CSR_FILL_ALPHA)
                        {
                            if (c.a>SMALLF2)
                            {
                                bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                                tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                tmp.a=1.0f;
                                surface[adr+n].value=SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                                surface[adr+n].z=z;
                            }
                        }
                        else
                        {
                            if (tex_ptr1)
                            {
                                bkc.Init(SR_GET_R(surface[adr+n].value),SR_GET_G(surface[adr+n].value),SR_GET_B(surface[adr+n].value));
                            
                                if (flags&CSR_FILL_BLEND)
                                {
                                    if ((surface[adr+n].value>>24)==0)
                                    {
                                        tmp.r=c.r;
                                        tmp.g=c.g;
                                        tmp.b=c.b;
                                    }
                                    else
                                    {
                                        tmp.r=0.5f*c.r + 0.5f*bkc.r;
                                        tmp.g=0.5f*c.g + 0.5f*bkc.g;
                                        tmp.b=0.5f*c.b + 0.5f*bkc.b;
                                    }
                                    tmp.a=c.a;
                                }
                                else
                                {
                                    tmp.r=c.a*c.r + (1.0f-c.a)*bkc.r;
                                    tmp.g=c.a*c.g + (1.0f-c.a)*bkc.g;
                                    tmp.b=c.a*c.b + (1.0f-c.a)*bkc.b;
                                    tmp.a=1.0f;
                                }
                            
                                unsigned int al=(unsigned int)(tmp.a*255);
                                surface[adr+n].value=(al<<24) + SR_COLOR((unsigned int)(tmp.r*255),(unsigned int)(tmp.g*255),(unsigned int)(tmp.b*255));
                            }
                            else surface[adr+n].value=cd;
                        
                            surface[adr+n].z=z;
                        }
                    }
                }
			}

			iz+=inc_iz;
		}
	}
}

void CSoftwareRenderer::Clear(int cc,float zz)
{
    int n;
    
	if (surface)
	for (n=0;n<wx*wy;n++)
	{
		surface[n].value=cc;
		surface[n].z=zz;
	}
    
    if (surfacemap)
        for (n=0;n<wx*wy;n++) surfacemap[n].Init(-666,-666);
}

int CSoftwareRenderer::AddSurface()
{
	int n=0;
	while (surfaces[n]) n++;
	surfaces[n]=(SRpixel*) malloc(wx*wy*sizeof(SRpixel));
	return n;
}

void CSoftwareRenderer::Flat(CVector2 &p0,CVector2 &p1,CVector2 &p2,int cc,float z)
{
	float AAA=0.15f;
	int i;
	float min,deltax1,deltax2,deltaxx;
	float dy1,dy2;
	float x,y,xx,fx,fy;
	float nz;
	int xdebut,xfin;
	CVector2 a,b,c;

	if (!surface) return;

	min=p0.y;i=0;
	if (min>p1.y) { min=p1.y;i=1;}
	if (min>p2.y) { min=p2.y;i=2;}

	switch (i)
	{
	case 0:
		a.Init((float) ((int) p0.x),(float) ((int) p0.y));
		b.Init((float) ((int) p1.x),(float) ((int) p1.y));
		c.Init((float) ((int) p2.x),(float) ((int) p2.y));
		break;
	case 1:
		a.Init((float) ((int) p1.x),(float) ((int) p1.y));
		b.Init((float) ((int) p2.x),(float) ((int) p2.y));
		c.Init((float) ((int) p0.x),(float) ((int) p0.y));
		break;
	case 2:
		a.Init((float) ((int) p2.x),(float) ((int) p2.y));
		b.Init((float) ((int) p0.x),(float) ((int) p0.y));
		c.Init((float) ((int) p1.x),(float) ((int) p1.y));
		break;
	};

	nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);


	if (nz>=0)
	{
		fx=x=a.x;
		y=a.y;

		if (b.y>c.y)
		{
			xx=c.x;
			fy=c.y;
			dy1=(c.y-a.y);
			dy2=(b.y-c.y);
			deltaxx=(b.x-a.x)/(b.y-a.y);
			deltax1=(c.x-a.x)/dy1;
			deltax2=(b.x-c.x)/dy2;
			if (dy1<1) deltax1=(c.x-a.x);
			if (dy2<1) deltax2=(b.x-c.x);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltax1*y;
						fx=fx-deltaxx*y;
						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,z);
						y=y+1;
						x=x+deltax1;
						fx=fx+deltaxx;
					}
				}
				else
				{
					x=x+deltax1*dy1;
					fx=fx+deltaxx*dy1;
				}
			}


			x=xx;
			y=fy;			

			if (y<wy)
			{

				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltax2*y;
						fx=fx-deltaxx*y;
						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,z);
						y=y+1;
						x=x+deltax2;
						fx=fx+deltaxx;

					}
				}
			}

		}
		else
		{
			xx=b.x;
			fy=b.y;
			dy1=(b.y-a.y);
			dy2=(c.y-b.y);
			deltax1=(b.x-a.x)/dy1;
			deltax2=(c.x-b.x)/dy2;
			deltaxx=(c.x-a.x)/(c.y-a.y);
			if (dy1<1) deltax1=(b.x-a.x);
			if (dy2<1) deltax2=(c.x-b.x);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax1*y;
						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,z);

						y=y+1;
						fx=fx+deltax1;
						x=x+deltaxx;
					}
				}
				else
				{
					fx=fx+deltax1*dy1;
					x=x+deltaxx*dy1;
				}
			}
			
			fx=xx;
			y=fy;

			if (y<wy)
			{
				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax2*y;
						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,z);
						y=y+1;
						fx=fx+deltax2;
						x=x+deltaxx;
					}
				}
			}
		}
		
	}
}


void CSoftwareRenderer::PrimitiveFlatZ(CVector &p0,CVector &p1,CVector &p2,int cc)
{
	float AAA=0.15f;
	int i;
	float min,deltax1,deltax2,deltaxx;
	float dy1,dy2;
	float x,y,xx,fx,fy;
	float nz;
	int xdebut,xfin;
	CVector2 a,b,c;
	float za,zb,zc,z1,z2;
	float deltazz,deltaz1,deltaz2;
	CVector2 zero;

	zero.Init(0,0);

	if (!surface) return;

	min=p0.y;i=0;
	if (min>p1.y) { min=p1.y;i=1;}
	if (min>p2.y) { min=p2.y;i=2;}

	switch (i)
	{
	case 0:
		a.Init((float) ((int) p0.x),(float) ((int) p0.y));
		b.Init((float) ((int) p1.x),(float) ((int) p1.y));
		c.Init((float) ((int) p2.x),(float) ((int) p2.y));
		za=p0.z;
		zb=p1.z;
		zc=p2.z;
		break;
	case 1:
		a.Init((float) ((int) p1.x),(float) ((int) p1.y));
		b.Init((float) ((int) p2.x),(float) ((int) p2.y));
		c.Init((float) ((int) p0.x),(float) ((int) p0.y));
		za=p1.z;
		zb=p2.z;
		zc=p0.z;
		break;
	case 2:
		a.Init((float) ((int) p2.x),(float) ((int) p2.y));
		b.Init((float) ((int) p0.x),(float) ((int) p0.y));
		c.Init((float) ((int) p1.x),(float) ((int) p1.y));
		za=p2.z;
		zb=p0.z;
		zc=p1.z;
		break;
	};

	nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

	if (nz>=0)
	{
		fx=x=a.x;
		y=a.y;
		z1=z2=za;


		if (b.y>c.y)
		{
			xx=c.x;
			fy=c.y;
			dy1=(c.y-a.y);
			dy2=(b.y-c.y);
			deltaxx=(b.x-a.x)/(b.y-a.y);
			deltax1=(c.x-a.x)/dy1;
			deltax2=(b.x-c.x)/dy2;
			if (dy1<1) deltax1=(c.x-a.x);
			if (dy2<1) deltax2=(b.x-c.x);

			deltazz=(zb-za)/(b.y-a.y);
			deltaz1=(zc-za)/dy1;
			deltaz2=(zb-zc)/dy2;
			if (dy1<1) deltaz1=(zc-za);
			if (dy2<1) deltaz2=(zb-zc);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltax1*y;
						fx=fx-deltaxx*y;

						z1-=deltaz1*y;
						z2-=deltazz*y;

						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						
						if ((z1>0)&&(z2>0))
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z);
						y=y+1;
						x=x+deltax1;
						fx=fx+deltaxx;
						z1+=deltaz1;
						z2+=deltazz;
					}
				}
				else
				{
					x=x+deltax1*dy1;
					fx=fx+deltaxx*dy1;

					z1+=deltaz1*dy1;
					z2+=deltazz*dy1;
				}
			}


			x=xx;
			z1=zc;
			y=fy;			

			if (y<wy)
			{

				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltax2*y;
						fx=fx-deltaxx*y;

						z1-=deltaz2*y;
						z2-=deltazz*y;

						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						if ((z1>0)&&(z2>0))
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z);
						y=y+1;
						x=x+deltax2;
						fx=fx+deltaxx;

						z1+=deltaz2;
						z2+=deltazz;
					}
				}
			}

		}
		else
		{
			xx=b.x;
			fy=b.y;
			dy1=(b.y-a.y);
			dy2=(c.y-b.y);
			deltax1=(b.x-a.x)/dy1;
			deltax2=(c.x-b.x)/dy2;
			deltaxx=(c.x-a.x)/(c.y-a.y);
			if (dy1<1) deltax1=(b.x-a.x);
			if (dy2<1) deltax2=(c.x-b.x);

			deltaz1=(zb-za)/dy1;
			deltaz2=(zc-zb)/dy2;
			deltazz=(zc-za)/(c.y-a.y);
			if (dy1<1) deltaz1=(zb-za);
			if (dy2<1) deltaz2=(zc-zb);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax1*y;

						z1-=deltazz*y;
						z2-=deltaz1*y;

						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						if ((z1>0)&&(z2>0))
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z);
						
						y=y+1;
						fx=fx+deltax1;
						x=x+deltaxx;

						z1+=deltazz;
						z2+=deltaz1;

					}
				}
				else
				{
					fx=fx+deltax1*dy1;
					x=x+deltaxx*dy1;

					z1+=deltazz*dy1;
					z2+=deltaz1*dy1;
				}
			}
			
			fx=xx;
			z2=zb;
			y=fy;

			if (y<wy)
			{
				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax2*y;

						z1-=deltazz*y;
						z2-=deltaz2*y;

						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						if ((z1>0)&&(z2>0))
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z);
						y=y+1;
						fx=fx+deltax2;
						x=x+deltaxx;

						z1+=deltazz;
						z2+=deltaz2;
					}
				}
			}
		}		
	}
}




void CSoftwareRenderer::PrimitiveFlatMapping(CVector &p0,CVector &p1,CVector &p2,CVector2 &tx0,CVector2 &tx1,CVector2 &tx2,int cc)
{
	float AAA=0.15f;
	int i;
	float min,deltax1,deltax2,deltaxx;
	float dy1,dy2;
	float x,y,xx,fx,fy;
	float nz;
	int xdebut,xfin;
	CVector2 a,b,c;
	float za,zb,zc,z1,z2;
	float deltazz,deltaz1,deltaz2;
	CVector2 ma,mb,mc,m1,m2;
	CVector2 deltamm,deltam1,deltam2;
	CVector2 zero;

	zero.Init(0,0);

	if (!surface) return;

	min=p0.y;i=0;
	if (min>p1.y) { min=p1.y;i=1;}
	if (min>p2.y) { min=p2.y;i=2;}

	switch (i)
	{
	case 0:
		a.Init((float) ((int) p0.x),(float) ((int) p0.y));
		b.Init((float) ((int) p1.x),(float) ((int) p1.y));
		c.Init((float) ((int) p2.x),(float) ((int) p2.y));
		za=p0.z;
		zb=p1.z;
		zc=p2.z;
		ma=tx0;
		mb=tx1;
		mc=tx2;
		break;
	case 1:
		a.Init((float) ((int) p1.x),(float) ((int) p1.y));
		b.Init((float) ((int) p2.x),(float) ((int) p2.y));
		c.Init((float) ((int) p0.x),(float) ((int) p0.y));
		za=p1.z;
		zb=p2.z;
		zc=p0.z;
		ma=tx1;
		mb=tx2;
		mc=tx0;
		break;
	case 2:
		a.Init((float) ((int) p2.x),(float) ((int) p2.y));
		b.Init((float) ((int) p0.x),(float) ((int) p0.y));
		c.Init((float) ((int) p1.x),(float) ((int) p1.y));
		za=p2.z;
		zb=p0.z;
		zc=p1.z;
		ma=tx2;
		mb=tx0;
		mc=tx1;
		break;
	};

	nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

	if (nz>=0)
	{
		fx=x=a.x;
		y=a.y;
		z1=z2=za;
		m1=m2=ma;


		if (b.y>c.y)
		{
			xx=c.x;
			fy=c.y;
			dy1=(c.y-a.y);
			dy2=(b.y-c.y);
			deltaxx=(b.x-a.x)/(b.y-a.y);
			deltax1=(c.x-a.x)/dy1;
			deltax2=(b.x-c.x)/dy2;
			if (dy1<1) deltax1=(c.x-a.x);
			if (dy2<1) deltax2=(b.x-c.x);

			deltazz=(zb-za)/(b.y-a.y);
			deltaz1=(zc-za)/dy1;
			deltaz2=(zb-zc)/dy2;
			if (dy1<1) deltaz1=(zc-za);
			if (dy2<1) deltaz2=(zb-zc);

			deltamm=(mb-ma)/(b.y-a.y);
			deltam1=(mc-ma)/dy1;
			deltam2=(mb-mc)/dy2;
			if (dy1<1) deltam1=(mc-ma);
			if (dy2<1) deltam2=(mb-mc);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltax1*y;
						fx=fx-deltaxx*y;

						z1-=deltaz1*y;
						z2-=deltazz*y;

						m1-=deltam1*y;
						m2-=deltamm*y;

						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_ALPHA);
						y=y+1;
						x=x+deltax1;
						fx=fx+deltaxx;
						z1+=deltaz1;
						z2+=deltazz;
						m1+=deltam1;
						m2+=deltamm;
					}
				}
				else
				{
					x=x+deltax1*dy1;
					fx=fx+deltaxx*dy1;

					z1+=deltaz1*dy1;
					z2+=deltazz*dy1;

					m1+=deltam1*dy1;
					m2+=deltamm*dy1;
				}
			}


			x=xx;
			z1=zc;
			m1=mc;
			y=fy;			

			if (y<wy)
			{

				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltax2*y;
						fx=fx-deltaxx*y;

						z1-=deltaz2*y;
						z2-=deltazz*y;

						m1-=deltam2*y;
						m2-=deltamm*y;

						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_ALPHA);
						y=y+1;
						x=x+deltax2;
						fx=fx+deltaxx;

						z1+=deltaz2;
						z2+=deltazz;

						m1+=deltam2;
						m2+=deltamm;
					}
				}
			}

		}
		else
		{
			xx=b.x;
			fy=b.y;
			dy1=(b.y-a.y);
			dy2=(c.y-b.y);
			deltax1=(b.x-a.x)/dy1;
			deltax2=(c.x-b.x)/dy2;
			deltaxx=(c.x-a.x)/(c.y-a.y);
			if (dy1<1) deltax1=(b.x-a.x);
			if (dy2<1) deltax2=(c.x-b.x);

			deltaz1=(zb-za)/dy1;
			deltaz2=(zc-zb)/dy2;
			deltazz=(zc-za)/(c.y-a.y);
			if (dy1<1) deltaz1=(zb-za);
			if (dy2<1) deltaz2=(zc-zb);

			deltam1=(mb-ma)/dy1;
			deltam2=(mc-mb)/dy2;
			deltamm=(mc-ma)/(c.y-a.y);
			if (dy1<1) deltam1=(mb-ma);
			if (dy2<1) deltam2=(mc-mb);

			if (y<wy)
			{
				if (y+dy1>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax1*y;

						m1-=deltamm*y;
						m2-=deltam1*y;

						dy1+=y;
						y=0;
					}

					if (y+dy1>wy) dy1=wy-y;

					if (dy1>0)
					for (i=0;i<dy1;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_ALPHA);
						
						y=y+1;
						fx=fx+deltax1;
						x=x+deltaxx;

						z1+=deltazz;
						z2+=deltaz1;

						m1+=deltamm;
						m2+=deltam1;

					}
				}
				else
				{
					fx=fx+deltax1*dy1;
					x=x+deltaxx*dy1;

					z1+=deltazz*dy1;
					z2+=deltaz1*dy1;

					m1+=deltamm*dy1;
					m2+=deltam1*dy1;
				}
			}
			
			fx=xx;
			z2=zb;
			m2=mb;
			y=fy;

			if (y<wy)
			{
				if (y+dy2>0)
				{
					if (y<0)
					{
						x=x-deltaxx*y;
						fx=fx-deltax2*y;

						z1-=deltazz*y;
						z2-=deltaz2*y;

						m1-=deltamm*y;
						m2-=deltam2*y;

						dy2+=y;
						y=0;
					}

					if (y+dy2>wy) dy2=wy-y;

					if (dy2>0)
					for (i=0;i<dy2;i++)
					{
						xdebut=(int) (x);
						xfin=(int) (fx+0.5f);
						Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_ALPHA);
						y=y+1;
						fx=fx+deltax2;
						x=x+deltaxx;

						z1+=deltazz;
						z2+=deltaz2;

						m1+=deltamm;
						m2+=deltam2;
					}
				}
			}
		}		
	}
}

void CSoftwareRenderer::PrimitiveFlatMappingRGB(CVector &p0,CVector &p1,CVector &p2,CVector2 &tx0,CVector2 &tx1,CVector2 &tx2,int cc)
{
    float AAA=0.15f;
    int i;
    float min,deltax1,deltax2,deltaxx;
    float dy1,dy2;
    float x,y,xx,fx,fy;
    float nz;
    int xdebut,xfin;
    CVector2 a,b,c;
    float za,zb,zc,z1,z2;
    float deltazz,deltaz1,deltaz2;
    CVector2 ma,mb,mc,m1,m2;
    CVector2 deltamm,deltam1,deltam2;
    CVector2 zero;

    zero.Init(0,0);

    if (!surface) return;

    min=p0.y;i=0;
    if (min>p1.y) { min=p1.y;i=1;}
    if (min>p2.y) { min=p2.y;i=2;}

    switch (i)
    {
    case 0:
        a.Init((float) ((int) p0.x),(float) ((int) p0.y));
        b.Init((float) ((int) p1.x),(float) ((int) p1.y));
        c.Init((float) ((int) p2.x),(float) ((int) p2.y));
        za=p0.z;
        zb=p1.z;
        zc=p2.z;
        ma=tx0;
        mb=tx1;
        mc=tx2;
        break;
    case 1:
        a.Init((float) ((int) p1.x),(float) ((int) p1.y));
        b.Init((float) ((int) p2.x),(float) ((int) p2.y));
        c.Init((float) ((int) p0.x),(float) ((int) p0.y));
        za=p1.z;
        zb=p2.z;
        zc=p0.z;
        ma=tx1;
        mb=tx2;
        mc=tx0;
        break;
    case 2:
        a.Init((float) ((int) p2.x),(float) ((int) p2.y));
        b.Init((float) ((int) p0.x),(float) ((int) p0.y));
        c.Init((float) ((int) p1.x),(float) ((int) p1.y));
        za=p2.z;
        zb=p0.z;
        zc=p1.z;
        ma=tx2;
        mb=tx0;
        mc=tx1;
        break;
    };
    
    nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

    if (nz>=0)
    {
        fx=x=a.x;
        y=a.y;
        z1=z2=za;
        m1=m2=ma;


        if (b.y>c.y)
        {
            xx=c.x;
            fy=c.y;
            dy1=(c.y-a.y);
            dy2=(b.y-c.y);
            deltaxx=(b.x-a.x)/(b.y-a.y);
            deltax1=(c.x-a.x)/dy1;
            deltax2=(b.x-c.x)/dy2;
            if (dy1<1) deltax1=(c.x-a.x);
            if (dy2<1) deltax2=(b.x-c.x);

            deltazz=(zb-za)/(b.y-a.y);
            deltaz1=(zc-za)/dy1;
            deltaz2=(zb-zc)/dy2;
            if (dy1<1) deltaz1=(zc-za);
            if (dy2<1) deltaz2=(zb-zc);

            deltamm=(mb-ma)/(b.y-a.y);
            deltam1=(mc-ma)/dy1;
            deltam2=(mb-mc)/dy2;
            if (dy1<1) deltam1=(mc-ma);
            if (dy2<1) deltam2=(mb-mc);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltax1*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz1*y;
                        z2-=deltazz*y;

                        m1-=deltam1*y;
                        m2-=deltamm*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0);
                        y=y+1;
                        x=x+deltax1;
                        fx=fx+deltaxx;
                        z1+=deltaz1;
                        z2+=deltazz;
                        m1+=deltam1;
                        m2+=deltamm;
                    }
                }
                else
                {
                    x=x+deltax1*dy1;
                    fx=fx+deltaxx*dy1;

                    z1+=deltaz1*dy1;
                    z2+=deltazz*dy1;

                    m1+=deltam1*dy1;
                    m2+=deltamm*dy1;
                }
            }


            x=xx;
            z1=zc;
            m1=mc;
            y=fy;

            if (y<wy)
            {

                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltax2*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz2*y;
                        z2-=deltazz*y;

                        m1-=deltam2*y;
                        m2-=deltamm*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0);
                        y=y+1;
                        x=x+deltax2;
                        fx=fx+deltaxx;

                        z1+=deltaz2;
                        z2+=deltazz;

                        m1+=deltam2;
                        m2+=deltamm;
                    }
                }
            }

        }
        else
        {
            xx=b.x;
            fy=b.y;
            dy1=(b.y-a.y);
            dy2=(c.y-b.y);
            deltax1=(b.x-a.x)/dy1;
            deltax2=(c.x-b.x)/dy2;
            deltaxx=(c.x-a.x)/(c.y-a.y);
            if (dy1<1) deltax1=(b.x-a.x);
            if (dy2<1) deltax2=(c.x-b.x);

            deltaz1=(zb-za)/dy1;
            deltaz2=(zc-zb)/dy2;
            deltazz=(zc-za)/(c.y-a.y);
            if (dy1<1) deltaz1=(zb-za);
            if (dy2<1) deltaz2=(zc-zb);

            deltam1=(mb-ma)/dy1;
            deltam2=(mc-mb)/dy2;
            deltamm=(mc-ma)/(c.y-a.y);
            if (dy1<1) deltam1=(mb-ma);
            if (dy2<1) deltam2=(mc-mb);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax1*y;

                        m1-=deltamm*y;
                        m2-=deltam1*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0);
                        
                        y=y+1;
                        fx=fx+deltax1;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz1;

                        m1+=deltamm;
                        m2+=deltam1;

                    }
                }
                else
                {
                    fx=fx+deltax1*dy1;
                    x=x+deltaxx*dy1;

                    z1+=deltazz*dy1;
                    z2+=deltaz1*dy1;

                    m1+=deltamm*dy1;
                    m2+=deltam1*dy1;
                }
            }
            
            fx=xx;
            z2=zb;
            m2=mb;
            y=fy;

            if (y<wy)
            {
                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax2*y;

                        z1-=deltazz*y;
                        z2-=deltaz2*y;

                        m1-=deltamm*y;
                        m2-=deltam2*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0);
                        y=y+1;
                        fx=fx+deltax2;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz2;

                        m1+=deltamm;
                        m2+=deltam2;
                    }
                }
            }
        }
    }
}


void CSoftwareRenderer::borders()
{
	int x,y;

	int adr=0;
	for (y=1;y<wy-1;y++)
	{
		for (x=1;x<wx-1;x++)
		{
			adr=x+y*wx;
			SRpixel& pix=surface[adr];
			if (pix.value==0)
			{
				SRpixel& up=surface[adr-wx];
				SRpixel& down=surface[adr+wx];
				SRpixel& left=surface[adr-1];
				SRpixel& right=surface[adr+1];
				if (up.value!=0) pix.value=up.value;
				if (down.value!=0) pix.value=down.value;
				if (left.value!=0) pix.value=left.value;
				if (right.value!=0) pix.value=right.value;
			}
		}
	}
}

void CSoftwareRenderer::PrimitiveFlatDot(CVector &p,unsigned int color,int size)
{
	int x,y;
	int x1,x2,y1,y2;
	int xc=(int)p.x;
	int yc=(int)p.y;
	float zc=p.z;

	if (!surface) return;
	
	if (zc>0)
	{		
		x1=(xc-size);
		y1=(yc-size);
		x2=(xc+size);
		y2=(yc+size);
		if (x2<0) return;
		if (y2<0) return;
		if (x1>=wx) return;
		if (y1>=wy) return;

		if (x1<0) x1=0;
		if (y1<0) y1=0;
		if (x2>wx) x2=wx;
		if (y2>wy) y2=wy;

		int dx=x2-x1;
		int dy=y2-y1;

		int adr=x1+wx*y1;

		#define medrgba(rgb1,rgb2) (((rgb1&0xFEFEFEFE)>>1)+((rgb2&0xFEFEFEFE)>>1))

		if (size>1)
		{
			for (y=0;y<dy;y++)
			{
				if ((y==0)||(y==dy-1))
				{
					SRpixel* pix=&surface[adr];
					for (x=1;x<dx-1;x++)
					{		
						if (zc<pix[x].z)
						{
							if (pix[x].value==0) pix[x].value=color;
							else pix[x].value=medrgba(pix[x].value,color);
							pix[x].z=zc;
						}
					}
				}
				else
				{
					SRpixel* pix=&surface[adr];
					for (x=0;x<dx;x++)
					{		
						if (zc<pix[x].z)
						{
							if (pix[x].value==0) pix[x].value=color;
							else pix[x].value=medrgba(pix[x].value,color);
							pix[x].z=zc;
						}
					}
				}
				adr+=wx;
			}
		}
		else
		{
			for (y=0;y<dy;y++)
			{
				SRpixel* pix=&surface[adr];
				for (x=0;x<dx;x++)
				{		
					if (zc<pix[x].z)
					{						
						if (pix[x].value==0) pix[x].value=color;
						else pix[x].value=medrgba(pix[x].value,color);
						pix[x].z=zc;
					}
				}

				adr+=wx;
			}
		}
	}
}

void CSoftwareRenderer::PrimitiveFlatMappingRGBBlend(CVector &p0,CVector &p1,CVector &p2,CVector2 &tx0,CVector2 &tx1,CVector2 &tx2,int cc)
{
    float AAA=0.15f;
    int i;
    float min,deltax1,deltax2,deltaxx;
    float dy1,dy2;
    float x,y,xx,fx,fy;
    float nz;
    int xdebut,xfin;
    CVector2 a,b,c;
    float za,zb,zc,z1,z2;
    float deltazz,deltaz1,deltaz2;
    CVector2 ma,mb,mc,m1,m2;
    CVector2 deltamm,deltam1,deltam2;
    CVector2 zero;

    zero.Init(0,0);

    if (!surface) return;

    min=p0.y;i=0;
    if (min>p1.y) { min=p1.y;i=1;}
    if (min>p2.y) { min=p2.y;i=2;}

    switch (i)
    {
    case 0:
        a.Init((float) ((int) p0.x),(float) ((int) p0.y));
        b.Init((float) ((int) p1.x),(float) ((int) p1.y));
        c.Init((float) ((int) p2.x),(float) ((int) p2.y));
        za=p0.z;
        zb=p1.z;
        zc=p2.z;
        ma=tx0;
        mb=tx1;
        mc=tx2;
        break;
    case 1:
        a.Init((float) ((int) p1.x),(float) ((int) p1.y));
        b.Init((float) ((int) p2.x),(float) ((int) p2.y));
        c.Init((float) ((int) p0.x),(float) ((int) p0.y));
        za=p1.z;
        zb=p2.z;
        zc=p0.z;
        ma=tx1;
        mb=tx2;
        mc=tx0;
        break;
    case 2:
        a.Init((float) ((int) p2.x),(float) ((int) p2.y));
        b.Init((float) ((int) p0.x),(float) ((int) p0.y));
        c.Init((float) ((int) p1.x),(float) ((int) p1.y));
        za=p2.z;
        zb=p0.z;
        zc=p1.z;
        ma=tx2;
        mb=tx0;
        mc=tx1;
        break;
    };

    nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

    if (nz>=0)
    {
        fx=x=a.x;
        y=a.y;
        z1=z2=za;
        m1=m2=ma;


        if (b.y>c.y)
        {
            xx=c.x;
            fy=c.y;
            dy1=(c.y-a.y);
            dy2=(b.y-c.y);
            deltaxx=(b.x-a.x)/(b.y-a.y);
            deltax1=(c.x-a.x)/dy1;
            deltax2=(b.x-c.x)/dy2;
            if (dy1<1) deltax1=(c.x-a.x);
            if (dy2<1) deltax2=(b.x-c.x);

            deltazz=(zb-za)/(b.y-a.y);
            deltaz1=(zc-za)/dy1;
            deltaz2=(zb-zc)/dy2;
            if (dy1<1) deltaz1=(zc-za);
            if (dy2<1) deltaz2=(zb-zc);

            deltamm=(mb-ma)/(b.y-a.y);
            deltam1=(mc-ma)/dy1;
            deltam2=(mb-mc)/dy2;
            if (dy1<1) deltam1=(mc-ma);
            if (dy2<1) deltam2=(mb-mc);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltax1*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz1*y;
                        z2-=deltazz*y;

                        m1-=deltam1*y;
                        m2-=deltamm*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND);
                        y=y+1;
                        x=x+deltax1;
                        fx=fx+deltaxx;
                        z1+=deltaz1;
                        z2+=deltazz;
                        m1+=deltam1;
                        m2+=deltamm;
                    }
                }
                else
                {
                    x=x+deltax1*dy1;
                    fx=fx+deltaxx*dy1;

                    z1+=deltaz1*dy1;
                    z2+=deltazz*dy1;

                    m1+=deltam1*dy1;
                    m2+=deltamm*dy1;
                }
            }


            x=xx;
            z1=zc;
            m1=mc;
            y=fy;

            if (y<wy)
            {

                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltax2*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz2*y;
                        z2-=deltazz*y;

                        m1-=deltam2*y;
                        m2-=deltamm*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND);
                        y=y+1;
                        x=x+deltax2;
                        fx=fx+deltaxx;

                        z1+=deltaz2;
                        z2+=deltazz;

                        m1+=deltam2;
                        m2+=deltamm;
                    }
                }
            }

        }
        else
        {
            xx=b.x;
            fy=b.y;
            dy1=(b.y-a.y);
            dy2=(c.y-b.y);
            deltax1=(b.x-a.x)/dy1;
            deltax2=(c.x-b.x)/dy2;
            deltaxx=(c.x-a.x)/(c.y-a.y);
            if (dy1<1) deltax1=(b.x-a.x);
            if (dy2<1) deltax2=(c.x-b.x);

            deltaz1=(zb-za)/dy1;
            deltaz2=(zc-zb)/dy2;
            deltazz=(zc-za)/(c.y-a.y);
            if (dy1<1) deltaz1=(zb-za);
            if (dy2<1) deltaz2=(zc-zb);

            deltam1=(mb-ma)/dy1;
            deltam2=(mc-mb)/dy2;
            deltamm=(mc-ma)/(c.y-a.y);
            if (dy1<1) deltam1=(mb-ma);
            if (dy2<1) deltam2=(mc-mb);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax1*y;

                        m1-=deltamm*y;
                        m2-=deltam1*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND);
                        
                        y=y+1;
                        fx=fx+deltax1;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz1;

                        m1+=deltamm;
                        m2+=deltam1;

                    }
                }
                else
                {
                    fx=fx+deltax1*dy1;
                    x=x+deltaxx*dy1;

                    z1+=deltazz*dy1;
                    z2+=deltaz1*dy1;

                    m1+=deltamm*dy1;
                    m2+=deltam1*dy1;
                }
            }
            
            fx=xx;
            z2=zb;
            m2=mb;
            y=fy;

            if (y<wy)
            {
                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax2*y;

                        z1-=deltazz*y;
                        z2-=deltaz2*y;

                        m1-=deltamm*y;
                        m2-=deltam2*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND);
                        y=y+1;
                        fx=fx+deltax2;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz2;

                        m1+=deltamm;
                        m2+=deltam2;
                    }
                }
            }
        }
    }
}

void CSoftwareRenderer::PrimitiveFlatMappingRGBBlendDZ(CVector &p0,CVector &p1,CVector &p2,CVector2 &tx0,CVector2 &tx1,CVector2 &tx2,int cc)
{
    float AAA=0.15f;
    int i;
    float min,deltax1,deltax2,deltaxx;
    float dy1,dy2;
    float x,y,xx,fx,fy;
    float nz;
    int xdebut,xfin;
    CVector2 a,b,c;
    float za,zb,zc,z1,z2;
    float deltazz,deltaz1,deltaz2;
    CVector2 ma,mb,mc,m1,m2;
    CVector2 deltamm,deltam1,deltam2;
    CVector2 zero;

    zero.Init(0,0);

    if (!surface) return;

    min=p0.y;i=0;
    if (min>p1.y) { min=p1.y;i=1;}
    if (min>p2.y) { min=p2.y;i=2;}

    switch (i)
    {
    case 0:
        a.Init((float) ((int) p0.x),(float) ((int) p0.y));
        b.Init((float) ((int) p1.x),(float) ((int) p1.y));
        c.Init((float) ((int) p2.x),(float) ((int) p2.y));
        za=p0.z;
        zb=p1.z;
        zc=p2.z;
        ma=tx0;
        mb=tx1;
        mc=tx2;
        break;
    case 1:
        a.Init((float) ((int) p1.x),(float) ((int) p1.y));
        b.Init((float) ((int) p2.x),(float) ((int) p2.y));
        c.Init((float) ((int) p0.x),(float) ((int) p0.y));
        za=p1.z;
        zb=p2.z;
        zc=p0.z;
        ma=tx1;
        mb=tx2;
        mc=tx0;
        break;
    case 2:
        a.Init((float) ((int) p2.x),(float) ((int) p2.y));
        b.Init((float) ((int) p0.x),(float) ((int) p0.y));
        c.Init((float) ((int) p1.x),(float) ((int) p1.y));
        za=p2.z;
        zb=p0.z;
        zc=p1.z;
        ma=tx2;
        mb=tx0;
        mc=tx1;
        break;
    };

    nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

    if (nz>=0)
    {
        fx=x=a.x;
        y=a.y;
        z1=z2=za;
        m1=m2=ma;


        if (b.y>c.y)
        {
            xx=c.x;
            fy=c.y;
            dy1=(c.y-a.y);
            dy2=(b.y-c.y);
            deltaxx=(b.x-a.x)/(b.y-a.y);
            deltax1=(c.x-a.x)/dy1;
            deltax2=(b.x-c.x)/dy2;
            if (dy1<1) deltax1=(c.x-a.x);
            if (dy2<1) deltax2=(b.x-c.x);

            deltazz=(zb-za)/(b.y-a.y);
            deltaz1=(zc-za)/dy1;
            deltaz2=(zb-zc)/dy2;
            if (dy1<1) deltaz1=(zc-za);
            if (dy2<1) deltaz2=(zb-zc);

            deltamm=(mb-ma)/(b.y-a.y);
            deltam1=(mc-ma)/dy1;
            deltam2=(mb-mc)/dy2;
            if (dy1<1) deltam1=(mc-ma);
            if (dy2<1) deltam2=(mb-mc);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltax1*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz1*y;
                        z2-=deltazz*y;

                        m1-=deltam1*y;
                        m2-=deltamm*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        x=x+deltax1;
                        fx=fx+deltaxx;
                        z1+=deltaz1;
                        z2+=deltazz;
                        m1+=deltam1;
                        m2+=deltamm;
                    }
                }
                else
                {
                    x=x+deltax1*dy1;
                    fx=fx+deltaxx*dy1;

                    z1+=deltaz1*dy1;
                    z2+=deltazz*dy1;

                    m1+=deltam1*dy1;
                    m2+=deltamm*dy1;
                }
            }


            x=xx;
            z1=zc;
            m1=mc;
            y=fy;

            if (y<wy)
            {

                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltax2*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz2*y;
                        z2-=deltazz*y;

                        m1-=deltam2*y;
                        m2-=deltamm*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        x=x+deltax2;
                        fx=fx+deltaxx;

                        z1+=deltaz2;
                        z2+=deltazz;

                        m1+=deltam2;
                        m2+=deltamm;
                    }
                }
            }

        }
        else
        {
            xx=b.x;
            fy=b.y;
            dy1=(b.y-a.y);
            dy2=(c.y-b.y);
            deltax1=(b.x-a.x)/dy1;
            deltax2=(c.x-b.x)/dy2;
            deltaxx=(c.x-a.x)/(c.y-a.y);
            if (dy1<1) deltax1=(b.x-a.x);
            if (dy2<1) deltax2=(c.x-b.x);

            deltaz1=(zb-za)/dy1;
            deltaz2=(zc-zb)/dy2;
            deltazz=(zc-za)/(c.y-a.y);
            if (dy1<1) deltaz1=(zb-za);
            if (dy2<1) deltaz2=(zc-zb);

            deltam1=(mb-ma)/dy1;
            deltam2=(mc-mb)/dy2;
            deltamm=(mc-ma)/(c.y-a.y);
            if (dy1<1) deltam1=(mb-ma);
            if (dy2<1) deltam2=(mc-mb);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax1*y;

                        m1-=deltamm*y;
                        m2-=deltam1*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND|CSR_FILL_DZ);
                        
                        y=y+1;
                        fx=fx+deltax1;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz1;

                        m1+=deltamm;
                        m2+=deltam1;

                    }
                }
                else
                {
                    fx=fx+deltax1*dy1;
                    x=x+deltaxx*dy1;

                    z1+=deltazz*dy1;
                    z2+=deltaz1*dy1;

                    m1+=deltamm*dy1;
                    m2+=deltam1*dy1;
                }
            }
            
            fx=xx;
            z2=zb;
            m2=mb;
            y=fy;

            if (y<wy)
            {
                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax2*y;

                        z1-=deltazz*y;
                        z2-=deltaz2*y;

                        m1-=deltamm*y;
                        m2-=deltam2*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,cc,cc,z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),m1,m2,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_TEX0|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        fx=fx+deltax2;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz2;

                        m1+=deltamm;
                        m2+=deltam2;
                    }
                }
            }
        }
    }
}


void CSoftwareRenderer::PrimitiveRGBBlendDZ(CVector &p0,CVector &p1,CVector &p2,CVector &c0,CVector &c1,CVector &c2,int cc)
{
    float AAA=0.15f;
    int i;
    float min,deltax1,deltax2,deltaxx;
    float dy1,dy2;
    float x,y,xx,fx,fy;
    float nz;
    int xdebut,xfin;
    CVector2 a,b,c;
    float za,zb,zc,z1,z2;
    float deltazz,deltaz1,deltaz2;
    CVector ma,mb,mc,m1,m2;
    CVector deltamm,deltam1,deltam2;
    CVector2 zero;

    zero.Init(0,0);

    if (!surface) return;

    min=p0.y;i=0;
    if (min>p1.y) { min=p1.y;i=1;}
    if (min>p2.y) { min=p2.y;i=2;}

    switch (i)
    {
    case 0:
        a.Init((float) ((int) p0.x),(float) ((int) p0.y));
        b.Init((float) ((int) p1.x),(float) ((int) p1.y));
        c.Init((float) ((int) p2.x),(float) ((int) p2.y));
        za=p0.z;
        zb=p1.z;
        zc=p2.z;
        ma=c0;
        mb=c1;
        mc=c2;
        break;
    case 1:
        a.Init((float) ((int) p1.x),(float) ((int) p1.y));
        b.Init((float) ((int) p2.x),(float) ((int) p2.y));
        c.Init((float) ((int) p0.x),(float) ((int) p0.y));
        za=p1.z;
        zb=p2.z;
        zc=p0.z;
        ma=c1;
        mb=c2;
        mc=c0;
        break;
    case 2:
        a.Init((float) ((int) p2.x),(float) ((int) p2.y));
        b.Init((float) ((int) p0.x),(float) ((int) p0.y));
        c.Init((float) ((int) p1.x),(float) ((int) p1.y));
        za=p2.z;
        zb=p0.z;
        zc=p1.z;
        ma=c2;
        mb=c0;
        mc=c1;
        break;
    };

    nz=(b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);

    if (nz>=0)
    {
        fx=x=a.x;
        y=a.y;
        z1=z2=za;
        m1=m2=ma;


        if (b.y>c.y)
        {
            xx=c.x;
            fy=c.y;
            dy1=(c.y-a.y);
            dy2=(b.y-c.y);
            deltaxx=(b.x-a.x)/(b.y-a.y);
            deltax1=(c.x-a.x)/dy1;
            deltax2=(b.x-c.x)/dy2;
            if (dy1<1) deltax1=(c.x-a.x);
            if (dy2<1) deltax2=(b.x-c.x);

            deltazz=(zb-za)/(b.y-a.y);
            deltaz1=(zc-za)/dy1;
            deltaz2=(zb-zc)/dy2;
            if (dy1<1) deltaz1=(zc-za);
            if (dy2<1) deltaz2=(zb-zc);

            deltamm=(mb-ma)/(b.y-a.y);
            deltam1=(mc-ma)/dy1;
            deltam2=(mb-mc)/dy2;
            if (dy1<1) deltam1=(mc-ma);
            if (dy2<1) deltam2=(mb-mc);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltax1*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz1*y;
                        z2-=deltazz*y;

                        m1-=deltam1*y;
                        m2-=deltamm*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,SR_COLORf(m1.x,m1.y,m1.z),SR_COLORf(m2.x,m2.y,m2.z),z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_GOURAUD|CSR_FILL_COLOR|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        x=x+deltax1;
                        fx=fx+deltaxx;
                        z1+=deltaz1;
                        z2+=deltazz;
                        m1+=deltam1;
                        m2+=deltamm;
                    }
                }
                else
                {
                    x=x+deltax1*dy1;
                    fx=fx+deltaxx*dy1;

                    z1+=deltaz1*dy1;
                    z2+=deltazz*dy1;

                    m1+=deltam1*dy1;
                    m2+=deltamm*dy1;
                }
            }


            x=xx;
            z1=zc;
            m1=mc;
            y=fy;

            if (y<wy)
            {

                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltax2*y;
                        fx=fx-deltaxx*y;

                        z1-=deltaz2*y;
                        z2-=deltazz*y;

                        m1-=deltam2*y;
                        m2-=deltamm*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,SR_COLORf(m1.x,m1.y,m1.z),SR_COLORf(m2.x,m2.y,m2.z),z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_GOURAUD|CSR_FILL_COLOR|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        x=x+deltax2;
                        fx=fx+deltaxx;

                        z1+=deltaz2;
                        z2+=deltazz;

                        m1+=deltam2;
                        m2+=deltamm;
                    }
                }
            }

        }
        else
        {
            xx=b.x;
            fy=b.y;
            dy1=(b.y-a.y);
            dy2=(c.y-b.y);
            deltax1=(b.x-a.x)/dy1;
            deltax2=(c.x-b.x)/dy2;
            deltaxx=(c.x-a.x)/(c.y-a.y);
            if (dy1<1) deltax1=(b.x-a.x);
            if (dy2<1) deltax2=(c.x-b.x);

            deltaz1=(zb-za)/dy1;
            deltaz2=(zc-zb)/dy2;
            deltazz=(zc-za)/(c.y-a.y);
            if (dy1<1) deltaz1=(zb-za);
            if (dy2<1) deltaz2=(zc-zb);

            deltam1=(mb-ma)/dy1;
            deltam2=(mc-mb)/dy2;
            deltamm=(mc-ma)/(c.y-a.y);
            if (dy1<1) deltam1=(mb-ma);
            if (dy2<1) deltam2=(mc-mb);

            if (y<wy)
            {
                if (y+dy1>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax1*y;

                        m1-=deltamm*y;
                        m2-=deltam1*y;

                        dy1+=y;
                        y=0;
                    }

                    if (y+dy1>wy) dy1=wy-y;

                    if (dy1>0)
                    for (i=0;i<dy1;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,SR_COLORf(m1.x,m1.y,m1.z),SR_COLORf(m2.x,m2.y,m2.z),z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_GOURAUD|CSR_FILL_COLOR|CSR_FILL_BLEND|CSR_FILL_DZ);
                        
                        y=y+1;
                        fx=fx+deltax1;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz1;

                        m1+=deltamm;
                        m2+=deltam1;

                    }
                }
                else
                {
                    fx=fx+deltax1*dy1;
                    x=x+deltaxx*dy1;

                    z1+=deltazz*dy1;
                    z2+=deltaz1*dy1;

                    m1+=deltamm*dy1;
                    m2+=deltam1*dy1;
                }
            }
            
            fx=xx;
            z2=zb;
            m2=mb;
            y=fy;

            if (y<wy)
            {
                if (y+dy2>0)
                {
                    if (y<0)
                    {
                        x=x-deltaxx*y;
                        fx=fx-deltax2*y;

                        z1-=deltazz*y;
                        z2-=deltaz2*y;

                        m1-=deltamm*y;
                        m2-=deltam2*y;

                        dy2+=y;
                        y=0;
                    }

                    if (y+dy2>wy) dy2=wy-y;

                    if (dy2>0)
                    for (i=0;i<dy2;i++)
                    {
                        xdebut=(int) (x);
                        xfin=(int) (fx+0.5f);
                        Fill((int) y,xdebut,xfin,SR_COLORf(m1.x,m1.y,m1.z),SR_COLORf(m2.x,m2.y,m2.z),z1,z2,((1/z1)-(1/front))/((1/back)-(1/front)),((1/z2)-(1/front))/((1/back)-(1/front)),zero,zero,zero,zero,zero,zero,CSR_FILL_Z|CSR_FILL_GOURAUD|CSR_FILL_COLOR|CSR_FILL_BLEND|CSR_FILL_DZ);
                        y=y+1;
                        fx=fx+deltax2;
                        x=x+deltaxx;

                        z1+=deltazz;
                        z2+=deltaz2;

                        m1+=deltamm;
                        m2+=deltam2;
                    }
                }
            }
        }
    }
}

#endif
