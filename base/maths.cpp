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


////////////////////////////////////////////////////////////////////////
//	@file maths.cpp 
//	@date 2003
////////////////////////////////////////////////////////////////////////
  
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <math.h>
#include "maths.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool floatnanish(float x)
{
    return ((*(unsigned int *)&x) & 0x7fffffff) > 0x7f800000;
}

bool CVector::isNan()
{
	if (floatnanish(x)) return true;
	if (floatnanish(y)) return true;
	if (floatnanish(z)) return true;
	return false;
}

bool CMatrix::isNan()
{
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			if (floatnanish(a[i][j])) return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector up(0,-1,0);	// direction vers le haut 

CVector ___tmp1,___tmp2;

float _M_invsqrtf(float x)
{
   float xhalf = 0.5f * x;
   int i=*(int*)&x;
   i = 0x5f3759df - (i >> 1);
   x = *(float*)&i;
   x *= (1.5f - xhalf * x * x);
   x *= (1.5f - xhalf * x * x);
   return x;
}

float _M_sqrtf(float x)
{
	return 1.0f/_M_invsqrtf(x);
}

static float __FASTTRIGO_PI=3.1415926535897932384626433832795f;
static float __FASTTRIGO_PIsur2=-3.1415926535897932384626433832795f/2;
static float __FASTTRIGO_B = 4.0f/__FASTTRIGO_PI;
static float __FASTTRIGO_C = -4.0f/(__FASTTRIGO_PI*__FASTTRIGO_PI);
static float __FASTTRIGO_P = 0.225f;

float _M_cosf(float xx)
{
	float x=(xx-__FASTTRIGO_PIsur2)/__FASTTRIGO_PI;
	int ix=(int)x;
	x=(x-ix)*__FASTTRIGO_PI;
    float y = (__FASTTRIGO_B + __FASTTRIGO_C * f_abs(x))* x;
    if (ix&1) return -y;
	else return y;
}

float _M_sinf(float xx)
{
	float x=(xx)/__FASTTRIGO_PI;
	int ix=(int)x;
	x=(x-ix)*__FASTTRIGO_PI;
    float y = (__FASTTRIGO_B + __FASTTRIGO_C * f_abs(x))* x;
    if (ix&1) return -y;
	else return y;
}

float _M_acosf(float x)
{
    return _M_sqrtf(1.0f-x) * (1.5707963267f + x*(-0.213300989f + x*(0.077980478f + x*-0.02164095f)));
}

#define __M_PI_2 (PI*0.5f)

float _M_asinf(float x)
{
    return _M_sqrtf(1.0f-x) * (1.5707963267f + x*(-0.213300989f + x*(0.077980478f + x*-0.02164095f))) + (float)__M_PI_2;
}

float _M_tanf(float x)
{
	return _M_sinf(x)/_M_cosf(x);
}

float _M_atan2f(float y, float x)
{
	float xx=f_abs(x);
	float yy=f_abs(y);
	float a=_M_min(xx,yy)/_M_max(xx,yy);
	float s = a * a;
	float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
	if (yy>xx) r=1.57079637f - r;
	if (x < 0) r = 3.14159274f - r;
	if (y < 0) r = -r;
	return r;
}

float _M_atanf(float x)
{
	return _M_atan2f(x,1);
}


float _M_copysignf(float x,float y)
{
	if (x<0)
	{
		if (y<0) return x;
		else return -x;	
	}
	else
	{
		if (y<0) return -x;
		else return x;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables de pr�cision
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SMALLF=0.00001f;
float SMALLF3=0.01f;
float SMALLF2=0.001f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vectors
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector2::Norme()
{
	return (float) sqrtf(x*x+y*y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector2::Normalise()
{
	float n=(float) sqrtf(x*x+y*y);
	float sn;
	if (n>0.0f)
	{
		sn=1.0f/n;
		x*=sn;
		y*=sn;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector4::Init(CVector const&v)
{
	x=v.x;
	y=v.y;
	z=v.z;
	w=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlane2::Init(double A,double B,double C,double D)
{
	a=A;b=B;c=C;d=D;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlane2::Normalise()
{
	double n;
	n=sqrt(a*a+b*b+c*c);

	a/=n;
	b/=n;
	c/=n;
	d/=n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CPlane2::Distance(CVector const&v)
{
	return a*v.x+b*v.y+c*v.z+d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CPlane2::Distance_xyz(float x,float y,float z)
{
	return a*x+b*y+c*z+d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::neg()
{
	x=-x;
	y=-y;
	z=-z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::permut(int cycle)
{
	float xx,yy,zz;
	xx=x; yy=y; zz=z;
	if (cycle==1) { xx=y; yy=z; zz=x; }
	if (cycle==2) { xx=z; yy=x; zz=y; }
	x=xx; y=yy; z=zz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::Base(CVector v,int tag)
{
	CVector u,vect1,vect2,tmpup;
	float s;
	CVector up,iup;

	u=v;
	u.Normalise();

	up.Init(0,-1,0);

	iup=up;
	s=(iup||u);
	if (f_abs(s)>1.0f-SMALLF)
	{		
		if (s>0) vect1.Init(1,0,0);
		else vect1.Init(-1,0,0);
		vect1.Normalise();
	}
	else
	{
		vect1=iup - s*u;
		vect1.Normalise();
	}

	vect2=u^vect1;
	vect2.Normalise();

	if (tag==0) { x=vect1.x;y=vect1.y;z=vect1.z; }
	else { x=vect2.x;y=vect2.y;z=vect2.z; }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::Normalise()
{
	float n=sqrtf(x*x + y*y +z*z);
	float sn;
	if (n>0) 
	{
		sn=1.0f/n;
		x*=sn;
		y*=sn;
		z*=sn;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::Normalise2()
{
    float n=sqrtf(x*x + y*y +z*z);
    float sn;
    if (n>0)
    {
        sn=1.0f/n;
        x*=sn;
        y*=sn;
        z*=sn;
    }
    return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::Norme()
{
	float n;

	n=(float) sqrtf(x*x + y*y +z*z);

	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::Max()
{
	float nx=f_abs(x);
	float ny=f_abs(y);
	float nz=f_abs(z);
	if ((nx>ny)&&(nx>nz)) return nx;
	if ((ny>nx)&&(ny>nz)) return ny;
	return nz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::Min()
{
	float nx=f_abs(x);
	float ny=f_abs(y);
	float nz=f_abs(z);
	if ((nx<ny)&&(nx<nz)) return nx;
	if ((ny<nx)&&(ny<nz)) return ny;
	return nz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::Norme2()
{
	float n;

	n=(float) (x*x + y*y +z*z);

	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::NormeCarree()
{
	float n;

	n=(x*x + y*y +z*z);

	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::Init(float X,float Y,float Z)
{
	x=X;	y=Y;	z=Z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector4::Init(float X,float Y,float Z,float W)
{
	x=X;
	y=Y;
	z=Z;
	w=W;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector2::Init(float X,float Y)
{
	x=X;
	y=Y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector2::Mod1(float dx,float dy)
{
	x-=dx;
	y-=dy;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector2::Rotate(float ang)
{
    float c=cosf(ang);
    float s=sinf(ang);
    
    float _x=x;
    float _y=y;
    
    x=_x*c+_y*s;
    y=-_x*s+_y*c;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector2::RotateDeg(float ang)
{
    Rotate(ang*PI/180.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc function for Mesh Simplification Tutorial
// by Sven Forstmann in 2014
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float random_double_01(float a)
{
    float rnf=a*14.434252f+a*364.2343f+a*4213.45352f+a*2341.43255f+a*254341.43535f+a*223454341.3523534245f+23453.423412f;
    int rni=((int)rnf)%100000;
    return float(rni)/(100000.0f-1.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::random()
{
    x=(float)random_double_01(x);
    y=(float)random_double_01(y);
    z=(float)random_double_01(z);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::angle(CVector& v)
{
    CVector a = v , b = *this;
    float dot = v.x*x + v.y*y + v.z*z;
    float len = a.Norme() * b.Norme();
    
    if (len==0) len=0.00001f;
    
    float input = dot  / len;
    
    if (input<-1) input=-1;
    if (input>1) input=1;
    
    return acosf(input);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::angle2(CVector& v,CVector& w)
{
    CVector a = v , b= *this;
    float dot = a.x*b.x + a.y*b.y + a.z*b.z;
    float len = a.Norme() * b.Norme();
    
    if (len==0) len=1;
    
    CVector plane=b^w;
    
    if ( plane.x * a.x + plane.y * a.y + plane.z * a.z > 0 ) return -acosf(dot/len);
    return acosf(dot/len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::rot_x(float a)
{
    float c=cosf(a);
    float s=sinf(a);
    
    float yy= c * y + s * z;
    float zz= c * z - s * y;
    y = yy; z = zz;
    
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::rot_y(float a)
{
    float c=cosf(-a);
    float s=sinf(-a);

    float xx = c* x + s* z;
    float zz = c* z - s* x;
    x = xx; z = zz;
    
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::rot_z(float a)
{
    float c=cosf(a);
    float s=sinf(a);

    float yy = c* y + s* x;
    float xx = c* x - s* y;
    y = yy; x = xx;
    
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::cross(CVector u,CVector v)
{
    *this=u^v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CVector::dot(CVector u)
{
    return x*u.x+y*u.y+z*u.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVector::clamp(float min,float max)
{
    if (x<min) x=min;
    if (y<min) y=min;
    if (z<min) z=min;
    if (x>max) x=max;
    if (y>max) y=max;
    if (z>max) z=max;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::integer()
{
    return CVector(float(int(x)),float(int(y)),float(int(z)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CVector::frac()
{
    return CVector(x-float(int(x)),y-float(int(y)),z-float(int(z)));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plans
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlane::Normalise()
{
	CVector v;
	float n;

	v.Init(a,b,c);

	n=v.Norme();
	if (n>0.0f)
	{
		a/=n;
		b/=n;
		c/=n;
		d/=n;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CPlane::Distance(CVector const&v)
{
	return a*v.x+b*v.y+c*v.z+d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CPlane::Distance_xyz(float x,float y,float z)
{
	return a*x+b*y+c*z+d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlane::Init(float A,float B,float C,float D)
{
	a=A;
	b=B;
	c=C;
	d=D;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CQuaternion::Transform()
{
	CMatrix M;
	M.Mq(*this);
	return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::SetEuler(float rx,float ry,float rz)
{
	CVector w;
	CQuaternion Q,q1,q2,q3;
	CVector u1,u2,u3;

	u1.Init(1,0,0);
	u2.Init(0,1,0);
	u3.Init(0,0,1);

	q1.AxisAngle(u1,-PI*rx/180);
	q2.AxisAngle(u2,-PI*ry/180);
	q3.AxisAngle(u3,-PI*rz/180);

	Q.Id();
	Q=Q*q3;
	Q=Q*q2;
	Q=Q*q1;

	s=Q.s;
	n.x=Q.n.x;
	n.y=Q.n.y;
	n.z=Q.n.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::GetEulers(float &rx,float &ry,float &rz)
{
    // wikipedia
    float sinr_cosp = +2.0f * (s * n.x + n.y * n.z);
    float cosr_cosp = +1.0f - 2.0f * (n.x * n.x + n.y * n.y);
    rx = atan2f(sinr_cosp, cosr_cosp);
    
    float sinp = +2.0f * (s * n.y - n.z * n.x);
    if (fabsf(sinp) >= 1)
        ry = _M_copysignf(PI / 2, sinp);
    else
        ry = asinf(sinp);
    
    float siny_cosp = +2.0f * (s * n.z + n.x * n.y);
    float cosy_cosp = +1.0f - 2.0f * (n.y * n.y + n.z * n.z);
    rz = atan2f(siny_cosp, cosy_cosp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::GetEulersDeg(float &rx,float &ry,float &rz)
{
    float x,y,z;
    
    GetEulers(x,y,z);
    rx=x*180.0f/PI;
    ry=y*180.0f/PI;
    rz=z*180.0f/PI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::Init(float w,float x,float y,float z)
{	
	s=w;
	n.x=x;
	n.y=y;
	n.z=z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CQuaternion CQuaternion::Inverse()
{
	CQuaternion q;
	CQuaternion qc=Conjugate();
	q=(1.0f/Norme())*qc;
	return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::Normalise()
{
	float nn;
	nn=1.0f/Norme();
	s*=nn;
	n.x*=nn;
	n.y*=nn;
	n.z*=nn;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CQuaternion::Norme()
{
	return sqrtf(s*s + n.x*n.x+ n.y*n.y+ n.z*n.z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CQuaternion CQuaternion::Conjugate()
{
	CQuaternion q;
	q.s=s;
	q.n.x=-n.x;
	q.n.y=-n.y;
	q.n.z=-n.z;
	return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::From(CMatrix &M)
{
    float T=1.0f + M.a[0][0] + M.a[1][1] + M.a[2][2];
    
    if (T>0.0000001f)
    {
        float w=sqrtf(1.0f + M.a[0][0] + M.a[1][1] + M.a[2][2])*2.0f;
        s=0.25f*w;
        n.x=-(M.a[2][1]-M.a[1][2])/w;
        n.y=-(M.a[0][2]-M.a[2][0])/w;
        n.z=-(M.a[1][0]-M.a[0][1])/w;
    }
    else
    {
        if ((M.a[0][0]>M.a[1][1])&&(M.a[0][0]>M.a[2][2]))
        {
            float w=sqrtf(1.0f + M.a[0][0] - M.a[1][1] - M.a[2][2])*2.0f;
            n.x=0.25f*w;
            s=-(M.a[2][1]-M.a[1][2])/w;
            n.z=-(M.a[0][2]+M.a[2][0])/w;
            n.y=-(M.a[1][0]+M.a[0][1])/w;

        }
        else
        if (M.a[1][1]>M.a[2][2])
        {
            float w=sqrtf(1.0f - M.a[0][0] + M.a[1][1] - M.a[2][2])*2.0f;
            n.y=0.25f*w;
            n.z=-(M.a[2][1]+M.a[1][2])/w;
            s=-(M.a[0][2]-M.a[2][0])/w;
            n.x=-(M.a[1][0]+M.a[0][1])/w;

        }
        else
        {
            float w=sqrtf(1.0f - M.a[0][0] - M.a[1][1] + M.a[2][2])*2.0f;
            
            n.z=0.25f*w;
            n.y=-(M.a[2][1]+M.a[1][2])/w;
            n.x=-(M.a[0][2]+M.a[2][0])/w;
            s=-(M.a[1][0]-M.a[0][1])/w;

        }
    }
    /*
     
     >0
     S = sqrt(T) * 2;
     X = ( mat[9] - mat[6] ) / S;
     Y = ( mat[2] - mat[8] ) / S;
     Z = ( mat[4] - mat[1] ) / S;
     W = 0.25 * S;
     
     if ( mat[0] > mat[5] && mat[0] > mat[10] )  {    // Column 0:
     S  = sqrt( 1.0 + mat[0] - mat[5] - mat[10] ) * 2;
     X = 0.25 * S;
     Y = (mat[4] + mat[1] ) / S;
     Z = (mat[2] + mat[8] ) / S;
     W = (mat[9] - mat[6] ) / S;
     } else if ( mat[5] > mat[10] ) {            // Column 1:
     S  = sqrt( 1.0 + mat[5] - mat[0] - mat[10] ) * 2;
     X = (mat[4] + mat[1] ) / S;
     Y = 0.25 * S;
     Z = (mat[9] + mat[6] ) / S;
     W = (mat[2] - mat[8] ) / S;
     } else {                        // Column 2:
     S  = sqrt( 1.0 + mat[10] - mat[0] - mat[5] ) * 2;
     X = (mat[2] + mat[8] ) / S;
     Y = (mat[9] + mat[6] ) / S;
     Z = 0.25 * S;
     W = (mat[4] - mat[1] ) / S;
     }
     /**/
    
    Normalise();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::Slerp(CQuaternion *quat1,CQuaternion *quat2,float slerp)
{
    static float SMALLFW=0.0f;
    
    float omega,cosom,sinom,scale0,scale1;
    cosom = quat1->Dot(*quat2);

	if (cosom<0)
	{
		if ((1.0 + cosom) > SMALLFW)
		{
			if ((1.0 - cosom) > SMALLFW) {
				omega = acosf(cosom);
				sinom = sinf(omega);
				scale0 = sinf((1.0f - slerp) * omega) / sinom;
				scale1 = sinf(slerp * omega) / sinom;
			} else {
				scale0 = 1.0f - slerp;
				scale1 = slerp;
			}
			n.x = (float)( scale0 * quat1->n.x - scale1 * quat2->n.x );
			n.y = (float)( scale0 * quat1->n.y - scale1 * quat2->n.y );
			n.z = (float)( scale0 * quat1->n.z - scale1 * quat2->n.z );
			s = (float)( scale0 * quat1->s - scale1 * quat2->s );
		} else {
			n.x = -quat2->n.y;
			n.y = quat2->n.x;
			n.z = -quat2->s;
			s = quat2->n.z;
			scale0 = sinf((1.0f - slerp) * (float) PI/2);
			scale1 = sinf(slerp * (float) PI/2);
			n.x = (float)( scale0 * quat1->n.x - scale1 * n.x );
			n.y = (float)( scale0 * quat1->n.y - scale1 * n.y );
			n.z = (float)( scale0 * quat1->n.z - scale1 * n.z );
			s = (float)( scale0 * quat1->s - scale1 * s );
		}
	}
	else
	{
		if ((1.0 + cosom) > SMALLFW)
		{
			if ((1.0 - cosom) > SMALLFW) {
				omega = acosf(cosom);
				sinom = sinf(omega);
				scale0 = sinf((1.0f - slerp) * omega) / sinom;
				scale1 = sinf(slerp * omega) / sinom;
			} else {
				scale0 = 1.0f - slerp;
				scale1 = slerp;
			}
			n.x = (float)( scale0 * quat1->n.x + scale1 * quat2->n.x );
			n.y = (float)( scale0 * quat1->n.y + scale1 * quat2->n.y );
			n.z = (float)( scale0 * quat1->n.z + scale1 * quat2->n.z );
			s = (float)( scale0 * quat1->s + scale1 * quat2->s );
		} else {
			n.x = -quat2->n.y;
			n.y = quat2->n.x;
			n.z = -quat2->s;
			s = quat2->n.z;
			scale0 = sinf((1.0f - slerp) * (float) PI/2);
			scale1 = sinf(slerp * (float) PI/2);
			n.x = (float)( scale0 * quat1->n.x + scale1 * n.x );
			n.y = (float)( scale0 * quat1->n.y + scale1 * n.y );
			n.z = (float)( scale0 * quat1->n.z + scale1 * n.z );
			s = (float)( scale0 * quat1->s + scale1 * s );
		}
	}
	this->Normalise();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CQuaternion::Dot(CQuaternion const&q)
{
    return (n.x * q.n.x + n.y * q.n.y + n.z * q.n.z + s * q.s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::AxisAngle(CVector const&u,float angle)
{
	CVector uu;

	uu=u;
	uu.Normalise();

	s=(float) (cosf(angle/2));
	n.x=(float) (uu.x*sinf(angle/2));
	n.y=(float) (uu.y*sinf(angle/2));
	n.z=(float) (uu.z*sinf(angle/2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::AxisAngleSC(CVector const&u,float angle)
{
	CVector uu;

	uu=u;
	uu.Normalise();

	s=(float) (sinf(angle/2));
	n.x=(float) (uu.x*cosf(angle/2));
	n.y=(float) (uu.y*cosf(angle/2));
	n.z=(float) (uu.z*cosf(angle/2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::GetAxisAngle(CVector *u,float *angle)
{
	(*angle)=(float) (2*acosf(s));
	float sn=(float) sinf(*angle);
	if (f_abs(sn)>SMALLF)
	{
		*u=n/sn;
		u->Normalise();
	}
	else 
	{
		u->Init(1,0,0);
		*angle=0.0f;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CQuaternion::Id()
{
	s=1; n.x=n.y=n.z=0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrices
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Eulers(float *x,float *y,float *z)
{
    if (a[0][2]>0.9995f)
    {
        CMatrix MM=*this;
        MM.RotationDegre(0,90,0);
        *x=atan2f(-MM.a[1][2],MM.a[2][2]);
        *y=PI/2;
        *z=atan2f(-MM.a[0][1],MM.a[0][0]);
    }
    else
    if (a[0][2]<-0.9995f)
    {
        CMatrix MM=*this;
        MM.RotationDegre(0,90,0);
        *x=atan2f(-MM.a[1][2],MM.a[2][2]);
        *y=-PI/2;
        *z=atan2f(-MM.a[0][1],MM.a[0][0]);
    }
    else
    {
        *x=atan2f(-a[1][2],a[2][2]);
        *y=asinf(a[0][2]);
        *z=atan2f(-a[0][1],a[0][0]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::EulersDegrees(float *x,float *y,float *z)
{
    Eulers(x,y,z);
    *x*=180.0f/PI;
    *y*=180.0f/PI;
    *z*=180.0f/PI;
}

void CMatrix::Jacobi(CMatrix &M)
{
#define N_ITER_MAX 250
#define epsilon 0.0000000001f
	int q,p;
	int res;
	float a,aa,theta;
	int n,n1,n2;
	float e;
	CMatrix Ak,Ak1,Bk,Dk,W,Wt,VP;

	res=1;
	VP.Id();
	Ak=M;
	n=0;
	while ((res==1)&&(n<N_ITER_MAX))
	{
		n++;
		a=0; p=q=0;
		for (n1=0;n1<3;n1++)
			for (n2=0;n2<3;n2++)
			{
				if (n1!=n2)
				{
					aa=f_abs(Ak.a[n1][n2]);
					if (a<aa)
                    {
                        a=aa;
                        p=n1;
                        q=n2;
                    }
				}
			}

        /*
		c=p=q=0;
		if (f_abs(f_abs(Ak.a[0][1])-a)<epsilon) {c=0;p=0;q=1;}
		if (f_abs(f_abs(Ak.a[0][2])-a)<epsilon) {c=1;p=0;q=2;}
		if (f_abs(f_abs(Ak.a[1][2])-a)<epsilon) {c=2;p=1;q=2;}
         /**/

		if (f_abs(Ak.a[q][q]-Ak.a[p][p])>epsilon) theta=(float) atanf((2*Ak.a[p][q])/(Ak.a[q][q]-Ak.a[p][p]))/2;
		else theta=0;

		W.Id();
		W.a[p][p]=(float) cosf(theta);
		W.a[q][q]=(float) cosf(theta);
		W.a[q][p]=(float) -sinf(theta);
		W.a[p][q]=(float) sinf(theta);

		Wt.Transpose(W);
		VP=Wt*VP;
		Ak1=Ak*W;
		Ak1=Wt*Ak1;
		Bk=Ak1;
		Bk.a[0][0]=0; Bk.a[1][1]=0; Bk.a[2][2]=0;
		e=Bk.Norme3x3();
		if (e<epsilon) res=0;
		Ak=Ak1;
	}

	(*this)=VP;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Reflection(float aa,float bb,float cc,float dd)
{
	a[0][0]=1-2*aa*aa;
	a[1][0]=-2*aa*bb;
	a[2][0]=-2*aa*cc;
	a[3][0]=-2*aa*dd;

	a[0][1]=-2*aa*bb;
	a[1][1]=1-2*bb*bb;
	a[2][1]=-2*bb*cc;
	a[3][1]=-2*bb*dd;

	a[0][2]=-2*aa*cc;
	a[1][2]=-2*bb*cc;
	a[2][2]=1-2*cc*cc;
	a[3][2]=-2*cc*dd;

	a[0][3]=0;
	a[1][3]=0;
	a[2][3]=0;
	a[3][3]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::ShadowProjection(CVector4 const&Light,CPlane const&P)
{
	float dot=(Light||P);

	a[0][0]=dot-Light.x*P.a;
	a[1][0]= 0 -Light.x*P.b;
	a[2][0]= 0 -Light.x*P.c;
	a[3][0]= 0 -Light.x*P.d;

	a[0][1]= 0 -Light.y*P.a;
	a[1][1]=dot-Light.y*P.b;
	a[2][1]= 0 -Light.y*P.c;
	a[3][1]= 0 -Light.y*P.d;

	a[0][2]= 0 -Light.z*P.a;
	a[1][2]= 0 -Light.z*P.b;
	a[2][2]=dot-Light.z*P.c;
	a[3][2]= 0 -Light.z*P.d;

	a[0][3]= 0 -Light.w*P.a;
	a[1][3]= 0 -Light.w*P.b;
	a[2][3]= 0 -Light.w*P.c;
	a[3][3]=dot-Light.w*P.d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Base(CVector &T,CVector &N,CVector &B)
{
    Id();
    a[0][0]=T.x; a[0][1]=T.y; a[0][2]=T.z;
    a[1][0]=N.x; a[1][1]=N.y; a[1][2]=N.z;
    a[2][0]=B.x; a[2][1]=B.y; a[2][2]=B.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Id()
{
	a[0][0]=1;	a[1][0]=0;	a[2][0]=0;	a[3][0]=0;

	a[0][1]=0;	a[1][1]=1;	a[2][1]=0;	a[3][1]=0;

	a[0][2]=0;	a[1][2]=0;	a[2][2]=1;	a[3][2]=0;

	a[0][3]=0;	a[1][3]=0;	a[2][3]=0;	a[3][3]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Egal(CMatrix const&m)
{
	int n1,n2;

	for (n1=0;n1<4;n1++)
		for (n2=0;n2<4;n2++)
			a[n1][n2]=m.a[n1][n2];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Mul(CMatrix const&m)
{
    CMatrix res;
#ifdef MULTIPLICATION_A_GAUCHE
    res.a[0][0]=a[0][0]*m.a[0][0] + a[0][1]*m.a[1][0] + a[0][2]*m.a[2][0] + a[0][3]*m.a[3][0];
    res.a[1][0]=a[1][0]*m.a[0][0] + a[1][1]*m.a[1][0] + a[1][2]*m.a[2][0] + a[1][3]*m.a[3][0];
    res.a[2][0]=a[2][0]*m.a[0][0] + a[2][1]*m.a[1][0] + a[2][2]*m.a[2][0] + a[2][3]*m.a[3][0];
    res.a[3][0]=a[3][0]*m.a[0][0] + a[3][1]*m.a[1][0] + a[3][2]*m.a[2][0] + a[3][3]*m.a[3][0];
    
    res.a[0][1]=a[0][0]*m.a[0][1] + a[0][1]*m.a[1][1] + a[0][2]*m.a[2][1] + a[0][3]*m.a[3][1];
    res.a[1][1]=a[1][0]*m.a[0][1] + a[1][1]*m.a[1][1] + a[1][2]*m.a[2][1] + a[1][3]*m.a[3][1];
    res.a[2][1]=a[2][0]*m.a[0][1] + a[2][1]*m.a[1][1] + a[2][2]*m.a[2][1] + a[2][3]*m.a[3][1];
    res.a[3][1]=a[3][0]*m.a[0][1] + a[3][1]*m.a[1][1] + a[3][2]*m.a[2][1] + a[3][3]*m.a[3][1];
    
    res.a[0][2]=a[0][0]*m.a[0][2] + a[0][1]*m.a[1][2] + a[0][2]*m.a[2][2] + a[0][3]*m.a[3][2];
    res.a[1][2]=a[1][0]*m.a[0][2] + a[1][1]*m.a[1][2] + a[1][2]*m.a[2][2] + a[1][3]*m.a[3][2];
    res.a[2][2]=a[2][0]*m.a[0][2] + a[2][1]*m.a[1][2] + a[2][2]*m.a[2][2] + a[2][3]*m.a[3][2];
    res.a[3][2]=a[3][0]*m.a[0][2] + a[3][1]*m.a[1][2] + a[3][2]*m.a[2][2] + a[3][3]*m.a[3][2];
    
    res.a[0][3]=a[0][0]*m.a[0][3] + a[0][1]*m.a[1][3] + a[0][2]*m.a[2][3] + a[0][3]*m.a[3][3];
    res.a[1][3]=a[1][0]*m.a[0][3] + a[1][1]*m.a[1][3] + a[1][2]*m.a[2][3] + a[1][3]*m.a[3][3];
    res.a[2][3]=a[2][0]*m.a[0][3] + a[2][1]*m.a[1][3] + a[2][2]*m.a[2][3] + a[2][3]*m.a[3][3];
    res.a[3][3]=a[3][0]*m.a[0][3] + a[3][1]*m.a[1][3] + a[3][2]*m.a[2][3] + a[3][3]*m.a[3][3];

#else
    res.a[0][0]=m.a[0][0]*a[0][0] + m.a[0][1]*a[1][0] + m.a[0][2]*a[2][0] + m.a[0][3]*a[3][0];
    res.a[1][0]=m.a[1][0]*a[0][0] + m.a[1][1]*a[1][0] + m.a[1][2]*a[2][0] + m.a[1][3]*a[3][0];
    res.a[2][0]=m.a[2][0]*a[0][0] + m.a[2][1]*a[1][0] + m.a[2][2]*a[2][0] + m.a[2][3]*a[3][0];
    res.a[3][0]=m.a[3][0]*a[0][0] + m.a[3][1]*a[1][0] + m.a[3][2]*a[2][0] + m.a[3][3]*a[3][0];
    
    res.a[0][1]=m.a[0][0]*a[0][1] + m.a[0][1]*a[1][1] + m.a[0][2]*a[2][1] + m.a[0][3]*a[3][1];
    res.a[1][1]=m.a[1][0]*a[0][1] + m.a[1][1]*a[1][1] + m.a[1][2]*a[2][1] + m.a[1][3]*a[3][1];
    res.a[2][1]=m.a[2][0]*a[0][1] + m.a[2][1]*a[1][1] + m.a[2][2]*a[2][1] + m.a[2][3]*a[3][1];
    res.a[3][1]=m.a[3][0]*a[0][1] + m.a[3][1]*a[1][1] + m.a[3][2]*a[2][1] + m.a[3][3]*a[3][1];
    
    res.a[0][2]=m.a[0][0]*a[0][2] + m.a[0][1]*a[1][2] + m.a[0][2]*a[2][2] + m.a[0][3]*a[3][2];
    res.a[1][2]=m.a[1][0]*a[0][2] + m.a[1][1]*a[1][2] + m.a[1][2]*a[2][2] + m.a[1][3]*a[3][2];
    res.a[2][2]=m.a[2][0]*a[0][2] + m.a[2][1]*a[1][2] + m.a[2][2]*a[2][2] + m.a[2][3]*a[3][2];
    res.a[3][2]=m.a[3][0]*a[0][2] + m.a[3][1]*a[1][2] + m.a[3][2]*a[2][2] + m.a[3][3]*a[3][2];
    
    res.a[0][3]=m.a[0][0]*a[0][3] + m.a[0][1]*a[1][3] + m.a[0][2]*a[2][3] + m.a[0][3]*a[3][3];
    res.a[1][3]=m.a[1][0]*a[0][3] + m.a[1][1]*a[1][3] + m.a[1][2]*a[2][3] + m.a[1][3]*a[3][3];
    res.a[2][3]=m.a[2][0]*a[0][3] + m.a[2][1]*a[1][3] + m.a[2][2]*a[2][3] + m.a[2][3]*a[3][3];
    res.a[3][3]=m.a[3][0]*a[0][3] + m.a[3][1]*a[1][3] + m.a[3][2]*a[2][3] + m.a[3][3]*a[3][3];

#endif
    
    a[0][0]=res.a[0][0]; a[0][1]=res.a[0][1]; a[0][2]=res.a[0][2]; a[0][3]=res.a[0][3];
    a[1][0]=res.a[1][0]; a[1][1]=res.a[1][1]; a[1][2]=res.a[1][2]; a[1][3]=res.a[1][3];
    a[2][0]=res.a[2][0]; a[2][1]=res.a[2][1]; a[2][2]=res.a[2][2]; a[2][3]=res.a[2][3];
    a[3][0]=res.a[3][0]; a[3][1]=res.a[3][1]; a[3][2]=res.a[3][2]; a[3][3]=res.a[3][3];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Transpose(CMatrix const&m)
{
	int n1,n2;

	for (n1=0;n1<4;n1++)
		for (n2=0;n2<4;n2++)
			a[n1][n2]=m.a[n2][n1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CMatrix::Transpose()
{
	CMatrix M;
	M.Transpose(*this);
	return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMatrix::Translation()
{
	return CVector(a[3][0],a[3][1],a[3][2]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CMatrix::Transform()
{
	CMatrix M=*this;
	M.a[3][0]=M.a[3][1]=M.a[3][2]=0.0f;
	return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::RotationAngleVecteur(CVector const&u,float angle)
{
	CMatrix m;
	float c,s;

	c=(float) cosf(angle);
	s=(float) sinf(angle);

#ifdef MULTIPLICATION_A_GAUCHE

	m.a[0][0]=c+(1-c)*u.x*u.x;
	m.a[0][1]=(1-c)*u.x*u.y + s*u.z;
	m.a[0][2]=(1-c)*u.x*u.z - s*u.y;

	m.a[1][0]=(1-c)*u.y*u.x -s*u.z;
	m.a[1][1]=c+(1-c)*u.y*u.y;
	m.a[1][2]=(1-c)*u.y*u.z +s*u.x;

	m.a[2][0]=(1-c)*u.z*u.x + s*u.y;
	m.a[2][1]=(1-c)*u.z*u.y - s*u.x;
	m.a[2][2]=c+(1-c)*u.z*u.z;

#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0]=c+(1-c)*u.x*u.x;
	m.a[1][0]=(1-c)*u.x*u.y + s*u.z;
	m.a[2][0]=(1-c)*u.x*u.z - s*u.y;

	m.a[0][1]=(1-c)*u.y*u.x -s*u.z;
	m.a[1][1]=c+(1-c)*u.y*u.y;
	m.a[2][1]=(1-c)*u.y*u.z +s*u.x;

	m.a[0][2]=(1-c)*u.z*u.x + s*u.y;
	m.a[1][2]=(1-c)*u.z*u.y - s*u.x;
	m.a[2][2]=c+(1-c)*u.z*u.z;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::RotationAngleVecteurDegre(CVector const&u,float angle)
{
	CMatrix m;
	float c,s;

	c=(float) cosf(PI*angle/180);
	s=(float) sinf(PI*angle/180);

#ifdef MULTIPLICATION_A_GAUCHE

	m.a[0][0]=c+(1-c)*u.x*u.x;
	m.a[0][1]=(1-c)*u.x*u.y + s*u.z;
	m.a[0][2]=(1-c)*u.x*u.z - s*u.y;

	m.a[1][0]=(1-c)*u.y*u.x -s*u.z;
	m.a[1][1]=c+(1-c)*u.y*u.y;
	m.a[1][2]=(1-c)*u.y*u.z +s*u.x;

	m.a[2][0]=(1-c)*u.z*u.x + s*u.y;
	m.a[2][1]=(1-c)*u.z*u.y - s*u.x;
	m.a[2][2]=c+(1-c)*u.z*u.z;

#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0]=c+(1-c)*u.x*u.x;
	m.a[1][0]=(1-c)*u.x*u.y + s*u.z;
	m.a[2][0]=(1-c)*u.x*u.z - s*u.y;

	m.a[0][1]=(1-c)*u.y*u.x -s*u.z;
	m.a[1][1]=c+(1-c)*u.y*u.y;
	m.a[2][1]=(1-c)*u.y*u.z +s*u.x;

	m.a[0][2]=(1-c)*u.z*u.x + s*u.y;
	m.a[1][2]=(1-c)*u.z*u.y - s*u.x;
	m.a[2][2]=c+(1-c)*u.z*u.z;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Rotation(CVector const&r)
{
	CMatrix m;
	float cx,cy,cz,sx,sy,sz;

	cx=(float) cosf(r.x);
	sx=(float) sinf(r.x);
	cy=(float) cosf(r.y);
	sy=(float) sinf(r.y);
	cz=(float) cosf(r.z);
	sz=(float) sinf(r.z);

#ifdef MULTIPLICATION_A_GAUCHE
    
	m.a[0][0] = cy*cz;
	m.a[0][1] = -cy*sz;
	m.a[0][2] = sy;

	m.a[1][0] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[1][2] = -sx*cy;

	m.a[2][0] = sx*sz - cx*sy*cz;
	m.a[2][1] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif

#ifdef MULTIPLICATION_A_DROITE
    
	m.a[0][0] = cy*cz;
	m.a[1][0] = -cy*sz;
	m.a[2][0] = sy;

	m.a[0][1] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[2][1] = -sx*cy;

	m.a[0][2] = sx*sz - cx*sy*cz;
	m.a[1][2] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif
    
    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::RotationDegre(CVector const&rd)
{
	CMatrix m;
	float cx,cy,cz,sx,sy,sz;

	cx=(float) cosf(PI*rd.x/180);
	sx=(float) sinf(PI*rd.x/180);
	cy=(float) cosf(PI*rd.y/180);
	sy=(float) sinf(PI*rd.y/180);
	cz=(float) cosf(PI*rd.z/180);
	sz=(float) sinf(PI*rd.z/180);

#ifdef MULTIPLICATION_A_GAUCHE
    
	m.a[0][0] = cy*cz;
	m.a[0][1] = -cy*sz;
	m.a[0][2] = sy;

	m.a[1][0] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[1][2] = -sx*cy;

	m.a[2][0] = sx*sz - cx*sy*cz;
	m.a[2][1] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif

#ifdef MULTIPLICATION_A_DROITE
    
	m.a[0][0] = cy*cz;
	m.a[1][0] = -cy*sz;
	m.a[2][0] = sy;

	m.a[0][1] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[2][1] = -sx*cy;

	m.a[0][2] = sx*sz - cx*sy*cz;
	m.a[1][2] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif
    
    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Translation(CVector const&t)
{
    
#ifdef MULTIPLICATION_A_GAUCHE
	a[3][0]+=t.x;
	a[3][1]+=t.y;
	a[3][2]+=t.z;
#endif

#ifdef MULTIPLICATION_A_DROITE
	a[0][3]+=t.x;
	a[1][3]+=t.y;
	a[2][3]+=t.z;
#endif
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Scaling(CVector const&s)
{
	CMatrix m;

#ifdef MULTIPLICATION_A_GAUCHE
	m.a[0][0]=s.x;
	m.a[1][1]=s.y;
	m.a[2][2]=s.z;

#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0]=s.x;
	m.a[1][1]=s.y;
	m.a[2][2]=s.z;

#endif
    
    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Rotation(float rx,float ry,float rz)
{
	CMatrix m;
	float cx,cy,cz,sx,sy,sz;

	cx=(float) cosf(rx);
	sx=(float) sinf(rx);
	cy=(float) cosf(ry);
	sy=(float) sinf(ry);
	cz=(float) cosf(rz);
	sz=(float) sinf(rz);

#ifdef MULTIPLICATION_A_GAUCHE
    
	m.a[0][0] = cy*cz;
	m.a[0][1] = -cy*sz;
	m.a[0][2] = sy;

	m.a[1][0] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[1][2] = -sx*cy;

	m.a[2][0] = sx*sz - cx*sy*cz;
	m.a[2][1] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0] = cy*cz;
	m.a[1][0] = -cy*sz;
	m.a[2][0] = sy;

	m.a[0][1] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[2][1] = -sx*cy;

	m.a[0][2] = sx*sz - cx*sy*cz;
	m.a[1][2] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif
    
    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::RotationDegre(float rx,float ry,float rz)
{
	CMatrix m;
	float cx,cy,cz,sx,sy,sz;

	cx=(float) cosf(PI*rx/180);
	sx=(float) sinf(PI*rx/180);
	cy=(float) cosf(PI*ry/180);
	sy=(float) sinf(PI*ry/180);
	cz=(float) cosf(PI*rz/180);
	sz=(float) sinf(PI*rz/180);

#ifdef MULTIPLICATION_A_GAUCHE
    
	m.a[0][0] = cy*cz;
	m.a[0][1] = -cy*sz;
	m.a[0][2] = sy;

	m.a[1][0] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[1][2] = -sx*cy;

	m.a[2][0] = sx*sz - cx*sy*cz;
	m.a[2][1] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;
	
#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0] = cy*cz;
	m.a[1][0] = -cy*sz;
	m.a[2][0] = sy;

	m.a[0][1] = sx*sy*cz + cx*sz;
	m.a[1][1] = cx*cz - sx*sy*sz;
	m.a[2][1] = -sx*cy;

	m.a[0][2] = sx*sz - cx*sy*cz;
	m.a[1][2] = cx*sy*sz + sx*cz;
	m.a[2][2] = cx*cy;

#endif
    
    Mul(m);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::BaseRotationDegre(float rx,float ry,float rz)
{
	CMatrix m;
	float cx,cy,cz,sx,sy,sz;

	cx=(float) cosf(PI*rx/180);
	sx=(float) sinf(PI*rx/180);
	cy=(float) cosf(PI*ry/180);
	sy=(float) sinf(PI*ry/180);
	cz=(float) cosf(PI*rz/180);
	sz=(float) sinf(PI*rz/180);

#ifdef MULTIPLICATION_A_GAUCHE
    
	a[0][0] = cy*cz;
	a[0][1] = -cy*sz;
	a[0][2] = sy;

	a[1][0] = sx*sy*cz + cx*sz;
	a[1][1] = cx*cz - sx*sy*sz;
	a[1][2] = -sx*cy;

	a[2][0] = sx*sz - cx*sy*cz;
	a[2][1] = cx*sy*sz + sx*cz;
	a[2][2] = cx*cy;
	
#endif

#ifdef MULTIPLICATION_A_DROITE
	a[0][0] = cy*cz;
	a[1][0] = -cy*sz;
	a[2][0] = sy;

	a[0][1] = sx*sy*cz + cx*sz;
	a[1][1] = cx*cz - sx*sy*sz;
	a[2][1] = -sx*cy;

	a[0][2] = sx*sz - cx*sy*cz;
	a[1][2] = cx*sy*sz + sx*cz;
	a[2][2] = cx*cy;

#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Translation(float x,float y,float z)
{
#ifdef MULTIPLICATION_A_GAUCHE
	a[3][0]+=x;
	a[3][1]+=y;
	a[3][2]+=z;
#endif

#ifdef MULTIPLICATION_A_DROITE
	a[0][3]+=x;
	a[1][3]+=y;
	a[2][3]+=z;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Scaling(float x,float y,float z)
{
	CMatrix m;

#ifdef MULTIPLICATION_A_GAUCHE
	m.a[0][0]=x;
	m.a[1][1]=y;
	m.a[2][2]=z;
#endif

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0]=x;
	m.a[1][1]=y;
	m.a[2][2]=z;
#endif
    
    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Orientation_x(CVector const&pos,CVector const&u)
{
	CMatrix m;
	CVector tgt,vect1,vect2,tmp,view,iup;
	float s;

	iup=up;

	view.Init(u.x,u.y,u.z);
	view.Normalise();

	s=(iup||view);

	if (f_abs(s)>0.9999f)
	{
		iup.Init(up.z,up.x,up.y);
		s=(iup||view);
	}

	vect1=iup - s*view;
	vect1.Normalise();

	vect2=view^vect1;
	vect2.Normalise();


#ifdef MULTIPLICATION_A_GAUCHE

	m.a[0][0]=view.x;
	m.a[0][1]=view.y;
	m.a[0][2]=view.z;

	m.a[1][0]=vect1.x;
	m.a[1][1]=vect1.y;
	m.a[1][2]=vect1.z;

	m.a[2][0]=vect2.x;
	m.a[2][1]=vect2.y;
	m.a[2][2]=vect2.z;

	m.a[3][0]=pos.x;
	m.a[3][1]=pos.y;
	m.a[3][2]=pos.z;
	
#endif


#ifdef MULTIPLICATION_A_DROITE

	m.a[0][0]=view.x;
	m.a[1][0]=view.y;
	m.a[2][0]=view.z;

	m.a[0][1]=vect1.x;
	m.a[1][1]=vect1.y;
	m.a[2][1]=vect1.z;

	m.a[0][2]=vect2.x;
	m.a[1][2]=vect2.y;
	m.a[2][2]=vect2.z;

	m.a[0][3]=pos.x;
	m.a[1][3]=pos.y;
	m.a[2][3]=pos.z;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Orientation_y(CVector const&pos,CVector const&u)
{
	CMatrix m;
	CVector tgt,vect1,vect2,tmp,view,iup;
	float s;

	iup=up;

	view.Init(u.x,u.y,u.z);
	view.Normalise();

	s=(iup||view);

	if (f_abs(s)>0.9999f)
	{
		iup.Init(up.z,up.x,up.y);
		s=(iup||view);
	}

	vect1=iup - s*view;
	vect1.Normalise();

	vect2=view^vect1;
	vect2.Normalise();


#ifdef MULTIPLICATION_A_GAUCHE

	m.a[0][0]=vect2.x;
	m.a[0][1]=vect2.y;
	m.a[0][2]=vect2.z;

	m.a[1][0]=view.x;
	m.a[1][1]=view.y;
	m.a[1][2]=view.z;

	m.a[2][0]=vect1.x;
	m.a[2][1]=vect1.y;
	m.a[2][2]=vect1.z;

	m.a[3][0]=pos.x;
	m.a[3][1]=pos.y;
	m.a[3][2]=pos.z;

#endif


#ifdef MULTIPLICATION_A_DROITE

	m.a[0][0]=vect2.x;
	m.a[1][0]=vect2.y;
	m.a[2][0]=vect2.z;

	m.a[0][1]=view.x;
	m.a[1][1]=view.y;
	m.a[2][1]=view.z;

	m.a[0][2]=vect1.x;
	m.a[1][2]=vect1.y;
	m.a[2][2]=vect1.z;

	m.a[0][3]=pos.x;
	m.a[1][3]=pos.y;
	m.a[2][3]=pos.z;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Symetric(CVector const&s)
{
	CMatrix M,R;
	M.Id();
	M.Scaling(s.x,s.y,s.z);
	R=M*(*this);
	R=R*M;
	*this=R;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Orientation_z(CVector const&pos,CVector const&u)
{
	CMatrix m;
	CVector iup=up;
	CVector tgt,vect1,vect2,tmp,view;
	float s;

	view.Init(u.x,u.y,u.z);
	view.Normalise();

	s=(iup||view);

	if (f_abs(s)>0.9999f)
	{
		iup.Init(up.z,up.x,up.y);
		s=(iup||view);
	}

	vect1=iup - s*view;
	vect1.Normalise();

	vect2=view^vect1;
	vect2.Normalise();

#ifdef MULTIPLICATION_A_GAUCHE

	m.a[0][0]=vect1.x;
	m.a[0][1]=vect1.y;
	m.a[0][2]=vect1.z;

	m.a[1][0]=vect2.x;
	m.a[1][1]=vect2.y;
	m.a[1][2]=vect2.z;

	m.a[2][0]=view.x;
	m.a[2][1]=view.y;
	m.a[2][2]=view.z;

	m.a[3][0]=pos.x;
	m.a[3][1]=pos.y;
	m.a[3][2]=pos.z;
	
#endif


#ifdef MULTIPLICATION_A_DROITE

	m.a[0][0]=vect1.x;
	m.a[1][0]=vect1.y;
	m.a[2][0]=vect1.z;

	m.a[0][1]=vect2.x;
	m.a[1][1]=vect2.y;
	m.a[2][1]=vect2.z;

	m.a[0][2]=view.x;
	m.a[1][2]=view.y;
	m.a[2][2]=view.z;

	m.a[0][3]=pos.x;
	m.a[1][3]=pos.y;
	m.a[2][3]=pos.z;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::View(CVector const&pos,CVector const&direction,float roll)
{
	float r=roll;
	CMatrix m0,m2;
	CVector tgt,vect1,vect2,view,iup;
	float s;

	iup=up;

	m2.Id();
	m2.Translation(-pos.x,-pos.y,-pos.z);

	view.Init(direction.x,direction.y,direction.z);

	view.Normalise();

	s=(iup||view);

	if (f_abs(s)>1.0f-SMALLF)
	{		
		if (s>0) vect1.Init(0,0,-1);
		else vect1.Init(0,0,1);
		vect1.Normalise();
	}
	else
	{
		vect1=iup - s*view;
		vect1.Normalise();
	}

	vect2=view^vect1;
	vect2.Normalise();

#ifdef MULTIPLICATION_A_GAUCHE

	m0.a[0][0]=vect1.x;
	m0.a[1][0]=vect1.y;
	m0.a[2][0]=vect1.z;

	m0.a[0][1]=vect2.x;
	m0.a[1][1]=vect2.y;
	m0.a[2][1]=vect2.z;

	m0.a[0][2]=view.x;
	m0.a[1][2]=view.y;
	m0.a[2][2]=view.z;

	Egal(m2*m0);
#endif

#ifdef MULTIPLICATION_A_DROITE

	m0.a[0][0]=vect1.x;
	m0.a[0][1]=vect1.y;
	m0.a[0][2]=vect1.z;

	m0.a[1][0]=vect2.x;
	m0.a[1][1]=vect2.y;
	m0.a[1][2]=vect2.z;

	m0.a[2][0]=view.x;
	m0.a[2][1]=view.y;
	m0.a[2][2]=view.z;

	Egal(m0*m2);
#endif
    
	RotationDegre(0,0,r+90);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Mq(CQuaternion const&q)
{
	CMatrix m;

    float xs = q.n.x + q.n.x;   float ys = q.n.y + q.n.y;   float zs = q.n.z + q.n.z;
    float wx = q.s * xs;      float wy = q.s * ys;      float wz = q.s * zs;
    float xx = q.n.x * xs;    float xy = q.n.x * ys;    float xz = q.n.x * zs;
    float yy = q.n.y * ys;    float yz = q.n.y * zs;    float zz = q.n.z * zs;

	m.Id();

	m.a[0][0]=1.0f-yy-zz;
	m.a[1][0]=xy-wz;
	m.a[2][0]=xz+wy;

	m.a[0][1]=xy+wz;
	m.a[1][1]=1.0f-xx-zz;
	m.a[2][1]=yz-wx;

	m.a[0][2]=xz-wy;
	m.a[1][2]=yz+wx;
	m.a[2][2]=1.0f-xx-yy;

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::OrientationQuaternion(CQuaternion const&q)
{
	CMatrix m;

// faire multiplication droite et gauche

#ifdef MULTIPLICATION_A_DROITE
	m.a[0][0]=1-2*(q.n.y*q.n.y + q.n.z*q.n.z);
	m.a[0][1]=2*q.n.x*q.n.y - 2*q.s*q.n.z;
	m.a[0][2]=2*q.s*q.n.y+2*q.n.x*q.n.z;
	m.a[0][3]=0;

	m.a[1][0]=2*q.n.x*q.n.y+2*q.s*q.n.z;
	m.a[1][1]=1-2*(q.n.x*q.n.x + q.n.z*q.n.z);
	m.a[1][2]=-2*q.s*q.n.x + 2*q.n.y*q.n.z;
	m.a[1][3]=0;

	m.a[2][0]=-2*q.s*q.n.y + 2*q.n.x*q.n.z;
	m.a[2][1]=2*q.s*q.n.x+2*q.n.y*q.n.z;
	m.a[2][2]=1-2*(q.n.x*q.n.x + q.n.y*q.n.y);
	m.a[2][3]=0;

	m.a[3][0]=0;
	m.a[3][1]=0;
	m.a[3][2]=0;
	m.a[3][3]=1;

#endif

#ifdef MULTIPLICATION_A_GAUCHE
	m.a[0][0]=1-2*(q.n.y*q.n.y + q.n.z*q.n.z);
	m.a[1][0]=2*q.n.x*q.n.y - 2*q.s*q.n.z;
	m.a[2][0]=2*q.s*q.n.y+2*q.n.x*q.n.z;
	m.a[3][0]=0;

	m.a[0][1]=2*q.n.x*q.n.y+2*q.s*q.n.z;
	m.a[1][1]=1-2*(q.n.x*q.n.x + q.n.z*q.n.z);
	m.a[2][1]=-2*q.s*q.n.x + 2*q.n.y*q.n.z;
	m.a[3][1]=0;

	m.a[0][2]=-2*q.s*q.n.y + 2*q.n.x*q.n.z;
	m.a[1][2]=2*q.s*q.n.x+2*q.n.y*q.n.z;
	m.a[2][2]=1-2*(q.n.x*q.n.x + q.n.y*q.n.y);
	m.a[3][2]=0;

	m.a[0][3]=0;
	m.a[1][3]=0;
	m.a[2][3]=0;
	m.a[3][3]=1;

#endif

    Mul(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CMatrix::Trace3x3()
{
	return (a[0][0]+a[1][1]+a[2][2]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CMatrix::Norme3x3()
{
	CMatrix M;
	M.Transpose(*this);
	M=(*this)*M;
	return M.Trace3x3();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float CMatrix::Determinant3x3()
{
	return (
		a[0][0]*a[1][1]*a[2][2]+a[0][1]*a[1][2]*a[2][0]	+ a[0][2]*a[1][0]*a[2][1] 
		-a[2][0]*a[1][1]*a[0][2] - a[2][1]*a[1][2]*a[0][0] - a[2][2]*a[1][0]*a[0][1]
		);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CMatrix::Inverse3x3()
{
	CMatrix M;
	M.Inverse3x3(*this);
	return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CMatrix::Inverse4x4()
{
	CMatrix M;
	M.Inverse4x4(*this);
	return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Inverse3x3(CMatrix &M)
{
	float det=M.Determinant3x3();

    Id();
    
	a[0][0]=(M.a[1][1]*M.a[2][2] - M.a[2][1]*M.a[1][2])/det;
	a[0][1]=(M.a[1][2]*M.a[2][0] - M.a[2][2]*M.a[1][0])/det;
	a[0][2]=(M.a[1][0]*M.a[2][1] - M.a[2][0]*M.a[1][1])/det;

	a[1][0]=(M.a[2][1]*M.a[0][2] - M.a[0][1]*M.a[2][2])/det;
	a[1][1]=(M.a[2][2]*M.a[0][0] - M.a[0][2]*M.a[2][0])/det;
	a[1][2]=(M.a[2][0]*M.a[0][1] - M.a[0][0]*M.a[2][1])/det;

	a[2][0]=(M.a[0][1]*M.a[1][2] - M.a[1][1]*M.a[0][2])/det;
	a[2][1]=(M.a[0][2]*M.a[1][0] - M.a[1][2]*M.a[0][0])/det;
	a[2][2]=(M.a[0][0]*M.a[1][1] - M.a[1][0]*M.a[0][1])/det;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Inverse4x4(CMatrix &M)
{
	// proc�dure trouv�e sur internet : �l�gant!

	CMatrix tmp;
	int swap;
	int i,j,k;
	float aa;
	tmp=M;

	Id();
	for (i=0;i<4;i++)
	{
		swap=i;
		for (j=i+1;j<4;j++)
		{
			if (f_abs(tmp.a[j][i])>f_abs(tmp.a[i][i]))
			{
				swap=j;
			}
		}

		if (swap!=i)
		for (k=0;k<4;k++)
		{
			aa=tmp.a[swap][k];
			tmp.a[swap][k]=tmp.a[i][k];
			tmp.a[i][k]=aa;

			aa=a[swap][k];
			a[swap][k]=a[i][k];
			a[i][k]=aa;
		}

		aa=tmp.a[i][i];
		for (k=0;k<4;k++) 
		{
			tmp.a[i][k]/=aa;
			a[i][k]/=aa;
		}

		for (j=0;j<4;j++)
		{
			if (i!=j)
			{
				aa=tmp.a[j][i];
				for (k=0;k<4;k++) 
				{
					tmp.a[j][k]-=tmp.a[i][k]*aa;
					a[j][k]-=a[i][k]*aa;
				}
			
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMatrix::getTranslation()
{
    CVector t(a[3][0],a[3][1],a[3][2]);
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMatrix::getScaling()
{
    float s[3];
    for (int k=0;k<3;k++) s[k]=sqrtf(a[0][k]*a[0][k]+a[1][k]*a[1][k]+a[2][k]*a[2][k]);

    return CVector(s[0],s[1],s[2]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix CMatrix::unScale()
{
    CVector scale=getScaling();
    CMatrix M=*this;
    M.a[0][0]/=scale.x; M.a[1][0]/=scale.x; M.a[2][0]/=scale.x;
    M.a[0][1]/=scale.y; M.a[1][1]/=scale.y; M.a[2][1]/=scale.y;
    M.a[0][2]/=scale.z; M.a[1][2]/=scale.z; M.a[2][2]/=scale.z;
    return M;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMatrix::column(int n)
{
    CVector t(a[0][n],a[1][n],a[2][n]);
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector CMatrix::row(int n)
{
    CVector t(a[n][0],a[n][1],a[n][2]);
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector4 CMatrix::column4(int n)
{
    CVector4 t(a[0][n],a[1][n],a[2][n],a[3][n]);
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVector4 CMatrix::row4(int n)
{
    CVector4 t(a[n][0],a[n][1],a[n][2],a[n][3]);
    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MxN 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMatrix_MxN::CMatrix_MxN() { m=n=0; a=NULL; }
	
CMatrix_MxN::~CMatrix_MxN() 
{
	if ((m!=0)&&(n!=0))
	{
		if (a) delete [] a;
	}
        
    a=NULL;
	m=n=0;
}

float CMatrix_MxN::Determinant()
{
	CMatrix_MxN M;
	float res;
	int i,j,k;

	if (n==m)
	{
		if ((n==2)&&(m==2))
		{
			res=a[0+2*0]*a[1+2*1] - a[0+2*1]*a[1+2*0];
		}
		else
		{
			M.Init(m-1,n-1);

			res=0;

			for (k=0;k<n;k++)
			{
				for (i=0;i<k;i++)
					for (j=1;j<n;j++)
						M.a[i+(n-1)*(j-1)]=a[i+n*j];
				for (i=k+1;i<n;i++)
					for (j=1;j<n;j++)
						M.a[i-1+(n-1)*(j-1)]=a[i+n*j];

				if ((k&1)==0)
					res+=a[k]*M.Determinant();
				else
					res-=a[k]*M.Determinant();
			}
		
			M.Free();
		}
	}
	else
		res=0;

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Init(int M,int N)	// ligne,colonne
{
	if (M*N!=m*n)
	{
		Free();
		m=M;
		n=N;
		if ((m>0)&(n>0)) a=new float[m*n];
		else a=NULL;
	}
	
	for (int p=0;p<n*m;p++) a[p]=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Id()
{
	int n1,n2;

	if ((m!=0)&&(n!=0))
	{
		for (n1=0;n1<n;n1++)
			for (n2=0;n2<m;n2++)
				if (n1==n2)
					a[n1+n*n2]=1;
				else
					a[n1+n*n2]=0;

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Zero()
{
	int n1,n2;

	if ((m!=0)&&(n!=0))
	{
		for (n1=0;n1<n;n1++)
			for (n2=0;n2<m;n2++)
				a[n1+n*n2]=0;

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Mul(float s)
{
	int n1,n2;

	if ((m!=0)&&(n!=0))
	{
		for (n1=0;n1<n;n1++)
			for (n2=0;n2<m;n2++)
				a[n1+n*n2]*=s;

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Patch(int l,int c,CMatrix_MxN * M)
{
	int n1,n2;
	int adr=0;
	int pos;
	int pos0;

	if ((m!=0)&&(n!=0))
	{
		pos0=n*l + c;
		for (n2=0;n2<M->m;n2++)
		{
			pos=pos0;
			for (n1=0;n1<M->n;n1++) a[pos++]=M->a[adr++];
			pos0+=n;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Patch(int l,int c,CMatrix_MxN * M,int sl,int sc,int dl,int dc)
{
	int n1,n2;
	int adr=0;
	int pos;
	int pos0;

	if ((m!=0)&&(n!=0))
	{
		pos0=n*l + c;
		for (n2=0;n2<dl;n2++)
		{
			pos=pos0;
			for (n1=0;n1<dc;n1++) a[pos++]=M->a[sc+n1 + M->n*(sl+n2)];
			pos0+=n;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix_MxN* CMatrix_MxN::Transpose()	// optimisation MxN <4096
{
	CMatrix_MxN *res;
	int i,j,adr,adr2;

	res=new CMatrix_MxN;
	res->Init(n,m);

	adr=0;
	for (i=0;i<n;i++)
	{
		adr2=0;
		for (j=0;j<m;j++)
		{
			res->a[j+adr]=a[i+adr2];
			adr2+=n;
		}
		adr+=m;
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMatrix_MxN* CMatrix_MxN::TransposeSign()	// optimisation MxN <4096
{
	CMatrix_MxN *res;
	int i,j,adr,adr2;
	
	res=new CMatrix_MxN;
	res->Init(n,m);

	adr=0;
	for (i=0;i<n;i++)
	{
		adr2=0;
		for (j=0;j<m;j++)
		{
			res->a[j+adr]=-a[i+adr2];
			adr2+=n;
		}
		adr+=m;
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Jacobian2D(float rx,float ry,float sign)
{
    Init(2,3);
    a[0+0*3]=sign*ry;
    a[0+1*3]=-sign*rx;
    
    a[1+0*3]=sign;
    a[2+1*3]=sign;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Jacobian2DPivot(float rx,float ry,float sign)
{
    Init(2,3);
    a[0+0*3]=sign*ry*0;
    a[0+1*3]=-sign*rx*0;
    
    a[1+0*3]=sign;
    a[2+1*3]=sign;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Jacobian2DSpring(float rx,float ry,float sign,float raideur)
{
    Init(2,3);
    a[0+0*3]=sign*ry;
    a[0+1*3]=-sign*rx;
    
    a[1+0*3]=sign*raideur;
    a[2+1*3]=sign*raideur;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MOBIL_JOINT_DIM=4;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Jacobian(CVector const&r,float sign)
{
    Init(4,6);
    
    a[1+0*6]=sign*r.z;
    a[2+0*6]=-sign*r.y;
    a[3+0*6]=sign;
    
    a[0+1*6]=-sign*r.z;
    a[2+1*6]=sign*r.x;
    a[4+1*6]=sign;
    
    a[0+2*6]=sign*r.y;
    a[1+2*6]=-sign*r.x;
    a[5+2*6]=sign;
    
    a[0+3*6]=r.x;
    a[1+3*6]=r.y;
    a[2+3*6]=r.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int SPRING_JOINT_DIM=4;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::JacobianSpring(CVector const&r,float sign,float raideur)
{
    Init(4,6);
    
    a[1+0*6]=sign*r.z;
    a[2+0*6]=-sign*r.y;
    a[3+0*6]=sign*raideur;
    
    a[0+1*6]=-sign*r.z;
    a[2+1*6]=sign*r.x;
    a[4+1*6]=sign*raideur;
    
    a[0+2*6]=sign*r.y;
    a[1+2*6]=-sign*r.x;
    a[5+2*6]=sign*raideur;
    
    a[0+3*6]=r.x;
    a[1+3*6]=r.y;
    a[2+3*6]=r.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PIVOT_JOINT_DIM=6;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::JacobianPivot(CVector const&r,CVector const&x,float sign)
{
	Init(6,6);
    
    CVector u1,u2;

    u1.Base(x,0);
    u2.Base(x,1);
    
    a[1+0*6]=sign*r.z;
    a[2+0*6]=-sign*r.y;
    a[3+0*6]=sign;
    
    a[0+1*6]=-sign*r.z;
    a[2+1*6]=sign*r.x;
    a[4+1*6]=sign;
    
    a[0+2*6]=sign*r.y;
    a[1+2*6]=-sign*r.x;
    a[5+2*6]=sign;

	a[0+3*6]=u1.x;
	a[1+3*6]=u1.y;
	a[2+3*6]=u1.z;

    a[0+4*6]=u2.x;
    a[1+4*6]=u2.y;
    a[2+4*6]=u2.z;

    a[0+5*6]=r.x;
    a[1+5*6]=r.y;
    a[2+5*6]=r.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int FIXED_PIVOT_DIM=5;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::JacobianPivotFixe(CVector const&r,CVector const&x,float sign)
{
    Init(5,6);
    
    CVector u1,u2;
    
    u1.Base(x,0);
    u2.Base(x,1);
    
    a[1+0*6]=sign*r.z;
    a[2+0*6]=-sign*r.y;
    a[3+0*6]=sign;
    
    a[0+1*6]=-sign*r.z;
    a[2+1*6]=sign*r.x;
    a[4+1*6]=sign;
    
    a[0+2*6]=sign*r.y;
    a[1+2*6]=-sign*r.x;
    a[5+2*6]=sign;
    
    a[0+3*6]=u1.x;
    a[1+3*6]=u1.y;
    a[2+3*6]=u1.z;
    
    a[0+4*6]=u2.x;
    a[1+4*6]=u2.y;
    a[2+4*6]=u2.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int BALL_JOINT_DIM=3;
int FIXED_JOINT_DIM=4;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Jacobian3(CVector const&r,float sign)
{
	Init(3,6);

	a[1+0*6]=sign*r.z;
	a[2+0*6]=-sign*r.y;
	a[3+0*6]=sign;

	a[0+1*6]=-sign*r.z;
	a[2+1*6]=sign*r.x;
	a[4+1*6]=sign;

	a[0+2*6]=sign*r.y;
	a[1+2*6]=-sign*r.x;
	a[5+2*6]=sign;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::JacobianContact(CVector const&G,CVector const&N,CVector const&p,float sign)
{
	CVector u;
	Init(1,6);

	u=p-G;

	a[0]=sign*N.x;
	a[1]=sign*N.y;
	a[2]=sign*N.z;
	a[3]=sign*(N.y*u.z - N.z*u.y);
	a[4]=sign*(N.z*u.x - N.x*u.z);
	a[5]=sign*(N.x*u.y - N.y*u.x);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Load4x4(CMatrix * M)
{
	int i,j;
	Init(4,4);
	for (i=0;i<4;i++)
		for (j=0;j<4;j++)
			a[i+j*4]=M->a[j][i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Load4x4(CMatrix const&M)
{
	int i,j;
	Init(4,4);
	for (i=0;i<4;i++)
		for (j=0;j<4;j++)
			a[i+j*4]=M.a[j][i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Swap(int l1,int l2)
{
	int i;
	float aa;
	int adr1=l1*n;
	int adr2=l2*n;

	if (((m!=0)&&(n!=0))&&(l1!=l2))
	{
		for (i=0;i<n;i++)
		{
			aa=a[i+adr1];
			a[i+adr1]=a[i+adr2];
			a[i+adr2]=aa;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SWAP_LINES(mat,l1,l2)												\
{																			\
	float aa;														\
	int adr1=l1*mat->n;											\
	int adr2=l2*mat->n;											\
	for (i=0;i<mat->n;i++)													\
	{																		\
		aa=mat->a[adr1];													\
		mat->a[adr1++]=mat->a[adr2];										\
		mat->a[adr2++]=aa;													\
	}																		\
}																			\

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Inverse(CMatrix_MxN *M)
{
	CMatrix_MxN tmp,*t;

	tmp.Duplicate(M);
	t=tmp.Inverse();
	Duplicate(t);

	t->Free();
	delete t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// M-1.M = Id
CMatrix_MxN* CMatrix_MxN::Inverse()
{
	CMatrix_MxN M,*Result;
	int i,j,k,res;
	float aa,max;

	Result=0;

	if (m==n)
	{
		M.Init(n,n*2);

		// init

		for (i=0;i<n;i++)
			for (j=0;j<n;j++)
				M.a[i+(2*n)*j]=a[i+n*j];

		for (i=0;i<n;i++)
			for (j=0;j<n;j++)
				if (i==j)
					M.a[n+j+(2*n)*i]=1.0f;
				else
					M.a[n+j+(2*n)*i]=0.0f;

		// triangularisation

		for (k=0;k<n;k++)
		{
			res=k;
			max=f_abs(M.a[k+(n*2)*k]);
			for (i=k+1;i<n;i++)
			{
				if (f_abs(M.a[k+(n*2)*i])>max)
				{
					res=i;
					max=f_abs(M.a[k+(n*2)*i]);
				}
			}

			SWAP_LINES(this,res,k);

			//Swap(res,k);

			for (i=k+1;i<n;i++)
				for (j=k+1;j<2*n;j++)
				{
					M.a[j+(n*2)*i]=M.a[j+(n*2)*i] - (M.a[k+(n*2)*i]*M.a[j+(n*2)*k])/M.a[k+(n*2)*k];
				}

			for (i=k+1;i<n;i++)
					M.a[k+(n*2)*i]=0;
		}

		// remont�e

		Result=new CMatrix_MxN;
		Result->Init(n,n);

		for (k=n-1;k>=0;k--)
		{
			for (j=0;j<n;j++)
			{
				aa=M.a[n+j+k*(n*2)];

				for (i=k+1;i<n;i++)
				{
					aa-=M.a[i+k*(n*2)]*Result->a[j+n*i];
				}

				Result->a[j+n*k]=aa/M.a[k+(n*2)*k];
			}
		}

		M.Free();
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//static float floatbank[8192];
//static CMatrix_MxN tempmatrixmxn;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
CMatrix_MxN* CMatrix_MxN::ResolveAXB(CMatrix_MxN * B)
{
	CMatrix_MxN *Result;
	int i,j,k;
	int res;
	float aa,max;
	float *Ma=new float[(m+1)*(n+1)];

	Result=0;

	if (((B->m==m)&&(B->n==1))&&(m==n))
	{        
		// init
		int adr=0;
		int adr0=0;

		for (j=0;j<n;j++)
		{
			for (i=0;i<n;i++) Ma[adr0++]=a[adr++];
			adr0++;
		}

        adr0=0;
        for (i=0;i<n;i++) { Ma[n+adr0]=B->a[i]; adr0+=(n+1); }

		// triangularisation
		for (k=0;k<n;k++)
		{
			res=k;
			adr=k;
			max=f_abs(Ma[k+(n+1)*k]);
			for (i=k+1;i<n;i++)
			{
				float f=f_abs(Ma[adr]);
				if (f>max)
				{
					res=i;
					max=f;
				}
				adr+=n+1;
			}
			//Swap(res,k);
			SWAP_LINES(this,res,k);

            adr0=(n+1)*k;
            
			float ssk=1.0f/Ma[k+adr0];
            
			if (Ma[k+(n+1)*k]!=0.0f)
			for (j=k+1;j<n+1;j++)
			{
				adr=(k+1)*(n+1);
				for (i=k+1;i<n;i++)
				{
					Ma[j+adr]=Ma[j+adr] - (Ma[k+adr]*Ma[j+adr0])*ssk;
					adr+=n+1;
				}
			}
			adr=(k+1)*(n+1);
			for (i=k+1;i<n;i++) { Ma[k+adr]=0; adr+=(n+1); }
		}

        Result=new CMatrix_MxN;
        Result->Init(n,1);

		adr=(n-1)*(n+1);
		for (k=n-1;k>=0;k--)
		{
			aa=Ma[n+adr];
			for (i=k+1;i<n;i++) aa-=Ma[i+adr]*Result->a[i];
			if (Ma[k+adr]!=0.0f) Result->a[k]=aa/Ma[k+adr];
            else Result->a[k]=aa/SMALLF;
			adr-=(n+1);
		}
	}
    
    delete [] Ma;

	return Result;
}
/**/

CMatrix_MxN* CMatrix_MxN::ResolveAXB(CMatrix_MxN * B)
{
    CMatrix_MxN *Result;
    int i,j,k;
    float aa;
    float *Ma=new float[(m+1)*(n+1)];
    float *tmp=new float[1+n];
    int *ofs=new int[1+n];
    
    Result=0;
    
    if (((B->m==m)&&(B->n==1))&&(m==n))
    {
        // init
        int adr=0;
        int adr0=0;
        
        for (j=0;j<n;j++)
        {
            for (i=0;i<n;i++) Ma[adr0++]=a[adr++];
            adr0++;
        }
        
        adr0=n;
        for (i=0;i<n;i++) { Ma[adr0]=B->a[i]; adr0+=(n+1); }
        
        // triangularisation
        int adr00=n+1;
        adr0=0;
        for (k=0;k<n;k++)
        {
            adr=adr00;
            if (Ma[k+adr0]!=0.0f)
            {
                float ssk=1.0f/Ma[k+adr0];
                int nb=0;
                for (j=k+1;j<n+1;j++)
                {
                    if (Ma[k+adr]!=0)
                    {
                        tmp[nb]=Ma[k+adr]*ssk;
                        ofs[nb++]=adr;
                    }
                    adr+=n+1;
                }
                
                if (nb>0)
                {
                    for (j=k+1;j<n+1;j++)
                    {
                        for (i=0;i<nb;i++) Ma[ofs[i]+j]-=tmp[i]*Ma[j+adr0];
                    }
                }
            }
            
            adr00+=n+1;
            adr0+=n+1;
        }
        
        Result=new CMatrix_MxN;
        Result->Init(n,1);
        
        adr=(n-1)*(n+1);
        for (k=n-1;k>=0;k--)
        {
            aa=Ma[n+adr];
            for (i=k+1;i<n;i++) aa-=Ma[i+adr]*Result->a[i];
            if (Ma[k+adr]!=0.0f) Result->a[k]=aa/Ma[k+adr];
            else Result->a[k]=aa/SMALLF;
            adr-=(n+1);
        }
    }
    
    delete [] Ma;
    delete [] tmp;
    delete [] ofs;
    
    return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Free()
{
	if (((m>0)&&(n>0))&&(a)) delete [] a;
    a=NULL;
	m=n=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::CFrame(CMatrix const&M)
{
	a[0][0]=M.a[0][0];
	a[0][1]=-M.a[0][1];
	a[0][2]=-M.a[0][2];
	a[0][3]=M.a[0][3];
	
	a[1][0]=M.a[1][0];
	a[1][1]=-M.a[1][1];
	a[1][2]=-M.a[1][2];
	a[1][3]=M.a[1][3];
	
	a[2][0]=M.a[2][0];
	a[2][1]=-M.a[2][1];
	a[2][2]=-M.a[2][2];
	a[2][3]=M.a[2][3];

	a[3][0]=M.a[3][0];
	a[3][1]=-M.a[3][1];
	a[3][2]=-M.a[3][2];
	a[3][3]=M.a[3][3];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::XYZ(CVector const&vX,CVector const&vY,CVector const&vZ)
{
    a[0][0]=vX.x;
    a[1][0]=vX.y;
    a[2][0]=vX.z;

    a[0][1]=vY.x;
    a[1][1]=vY.y;
    a[2][1]=vY.z;

    a[0][2]=vZ.x;
    a[1][2]=vZ.y;
    a[2][2]=vZ.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::Diag(float a1,float a2,float a3,float a4)
{
	Id();
	a[0][0]=a1;
	a[1][1]=a2;
	a[2][2]=a3;
	a[3][3]=a4;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Div(CMatrix_MxN *M,float div)
{
	int i,j;
	Init(M->m,M->n);
	for (i=0;i<M->m;i++)
		for (j=0;j<M->n;j++) a[i*n +j]=M->a[i*n+j]/div;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Sub(CMatrix_MxN *A,CMatrix_MxN *B)
{
	int i,j;

	if ((A->n==B->n)&&(A->m==B->m))
	{
		Init(A->m,A->n);
		for (i=0;i<m;i++)
			for (j=0;j<n;j++) a[n*i+j]=A->a[n*i+j] - B->a[n*i+j];
	}
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Neg(CMatrix_MxN *A)
{
	int i,j;

	Init(A->m,A->n);
	for (i=0;i<m;i++)
		for (j=0;j<n;j++) a[n*i+j]=-A->a[n*i+j];
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Add(CMatrix_MxN *A,CMatrix_MxN *B)
{
	int i,j;

	if ((A->n==B->n)&&(A->m==B->m))
	{
		Init(A->m,A->n);
		for (i=0;i<m;i++)
			for (j=0;j<n;j++) a[n*i+j]=A->a[n*i+j] + B->a[n*i+j];
	}
}	


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Mul(CMatrix_MxN *A,CMatrix_MxN *B)
{
	int i,j,k;
	float c;

	if (A->n==B->m)
	{
		Init(A->m,B->n);
		for (i=0;i<m;i++)
			for (j=0;j<n;j++)
			{
				c=0.0f;
				for (k=0;k<A->n;k++) c+=A->a[A->n*i + k]*B->a[B->n*k+j];
				a[n*i +j]=c;
			}
	}
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::InvExchangeColumn(int column,CMatrix_MxN * M,CMatrix_MxN * iM,CMatrix_MxN * newColumn,CMatrix_MxN * Mnew,CMatrix_MxN * iMnew)
{
	int k;
	CMatrix_MxN Dv,t,tt,bT;
	Mnew->Free();
	iMnew->Free();
	Mnew->Duplicate(M);
	Mnew->Patch(0,column,newColumn);
	Dv.Init(newColumn->m,1);
	for (k=0;k<newColumn->m;k++) Dv.a[k]=newColumn->a[k] - M->a[k * M->n + column];
	bT.Line(iM,column);
	t.Mul(&bT,&Dv);
	float d=1.0f+t.a[0];
	t.Free();
	t.Mul(iM,&Dv);
	tt.Div(&t,d);
	t.Free();
	t.Mul(&tt,&bT);
	iMnew->Sub(iM,&t);
	bT.Free();
	Dv.Free();
	t.Free();
	tt.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::InvExchangeLine(int line,CMatrix_MxN * M,CMatrix_MxN * iM,CMatrix_MxN * newLine,CMatrix_MxN * Mnew,CMatrix_MxN * iMnew)
{
	int k;
	CMatrix_MxN Dv,t,tt,bT;
	Mnew->Free();
	iMnew->Free();
	Mnew->Duplicate(M);
	Mnew->Patch(line,0,newLine);
	Dv.Init(1,newLine->n);
	for (k=0;k<newLine->n;k++) Dv.a[k]=newLine->a[k] - M->a[line * M->n + k];
	bT.Column(iM,line);
	t.Mul(&Dv,&bT);
	float d=1.0f+t.a[0];
	t.Free();
	t.Mul(&Dv,iM);
	tt.Div(&t,d);
	t.Free();
	t.Mul(&bT,&tt);
	iMnew->Sub(iM,&t);
	bT.Free();
	Dv.Free();
	t.Free();
	tt.Free();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Duplicate(CMatrix_MxN *M)
{
	Init(M->m,M->n);
	for (int i=0;i<M->m;i++)
		for (int j=0;j<M->n;j++) a[M->n*i+j]=M->a[M->n*i+j];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Column(CMatrix_MxN *M,int c)
{
	Init(M->m,1);
	for (int k=0;k<m;k++) a[k]=M->a[k*M->n + c];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::Line(CMatrix_MxN *M,int l)
{
	Init(1,M->n);
	for (int k=0;k<n;k++) a[k]=M->a[l*M->n + k];
}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::LEn(int N,int n,float value)
{
	Init(1,N);
	Zero();
	a[n]=value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::CEn(int N,int n,float value)
{
	Init(N,1);
	Zero();
	a[n]=value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix_MxN::ResolveLCP(CMatrix_MxN *P,CMatrix_MxN *Q,CMatrix_MxN *R,CMatrix_MxN *S,CMatrix_MxN *u,CMatrix_MxN *v,CMatrix_MxN *px,CMatrix_MxN *pz,CMatrix_MxN *pw,bool drop)
{
	int *ind;
	CMatrix_MxN iP,iM,T1,T2,M,q,y,B,C,iB,I,En,ntrfc,tmpB,tmpiB;
	int trivial=1;
	int n,k,kz0;
	float z0;
	float min=1000000000;
	int dropping=0;
	int entering=0;
	int next_entering=0;

	iP.Inverse(P);
	T1.Mul(R,&iP); T2.Mul(&T1,Q); M.Sub(S,&T2);
	T2.Free();
	T2.Mul(&T1,u); 
	q.Sub(v,&T2);
	T1.Free();
	T2.Free();

	// pure LCP:  ( w - M.z = q )
	
	for (k=0;k<q.m;k++)	if (q.a[k]<0) trivial=0;
	
	if (trivial)
	{
		pw->Duplicate(&q);
		pz->Init(q.m,1);
		pz->Zero();
	}
	else
	{
		kz0=0;
		z0=10000;
		for (k=0;k<q.m;k++)	
			if (q.a[k]<z0) 
			{
				z0=q.a[k];
				kz0=k;
			}

		pz->Init(q.m,1);
		pz->Zero();
		pw->Init(q.m,1);
		pw->Zero();

		y.Init(q.m,1);
		y.Zero();

		ind=new int[q.m];

		for (k=0;k<q.m;k++) ind[k]=-1-k;
		ind[kz0]=0;

		// matrice identit�

		I.Init(q.m,q.m);
		I.Id();

		// vecteur qu'avec des -1
		En.Init(q.m,1);
		for (k=0;k<q.m;k++) En.a[k]=-1.0f;

		// B "feasible"
		B.Init(q.m,q.m);

		for (k=0;k<q.m;k++) 
		{
			if (ind[k]>0) 
			{
				T1.Column(&M,ind[k]-1);
				T2.Neg(&T1);
				B.Patch(0,k,&T2);
			}
			else
			if (ind[k]<0) 
			{
				T1.Column(&I,-(ind[k]+1));
				B.Patch(0,k,&T1);
			}
			else B.Patch(0,k,&En);
		}

		// inverse matrix de B
		iB.Inverse(&B);

		entering=0;
		next_entering=-1-kz0;
		dropping=-1;

		y.Mul(&iB,&q);
		// affectation de y aux diff�rents vecteurs associ�s par ind[]
		for (k=0;k<q.m;k++) 
		{
			if (ind[k]>0) pz->a[ind[k]-1]=y.a[k];
			else
			if (ind[k]<0) pw->a[-(ind[k]+1)]=y.a[k];
			else z0=y.a[k];
		}

		int N=0;
		
		while ((dropping!=kz0)&&(N<50))
		{
			entering=next_entering;			

			C.Init(q.m,1);
			if (entering>0) 
			{
				T1.Column(&M,entering-1);
				T2.Neg(&T1);
				C.Patch(0,0,&T2);
			}
			else
			if (entering<0) 
			{
				T1.Column(&I,-(entering+1));
				C.Patch(0,0,&T1);
			}
			else C.Patch(0,0,&En);

			T1.Mul(&iB,&q);
			T2.Mul(&iB,&C);

			dropping=-1;
			min=1000000000;
			for (k=0;k<q.m;k++) 
			{
				float f=T1.a[k]/T2.a[k];
				if ((f<min)&&(f>=0))
				{
					dropping=k;
					min=f;
				}
			}

			if (dropping>=0)
			{
				next_entering=-ind[dropping];
				ind[dropping]=entering;
				ntrfc.InvExchangeColumn(dropping,&B,&iB,&C,&tmpB,&tmpiB);
				B.Duplicate(&tmpB);
				iB.Duplicate(&tmpiB);
			}
			else
			{
				dropping=-1;
				n=0;
				while ((n<iB.n)&&(dropping==-1))
				{
					T1.Column(&iB,n);
					
					min=1000000000;
					for (k=0;k<q.m;k++) 
					{
						float f=T1.a[k]/T2.a[k];
						if ((f<min)&&(f>=0))
						{
							dropping=k;
							min=f;
						}
					}
					n++;
				}

				if (dropping>=0)
				{
					next_entering=-ind[dropping];
					ind[dropping]=entering;
					ntrfc.InvExchangeColumn(dropping,&B,&iB,&C,&tmpB,&tmpiB);
					B.Duplicate(&tmpB);
					iB.Duplicate(&tmpiB);
				}
				else N=50;
			}

			y.Mul(&iB,&q);

			// affectation de y aux diff�rents vecteurs associ�s par ind[]
			for (k=0;k<q.m;k++) 
			{
				if (ind[k]>0) pz->a[ind[k]-1]=y.a[k];
				else
				if (ind[k]<0) pw->a[-(ind[k]+1)]=y.a[k];
				else z0=y.a[k];
			}
			N++;
		}

		delete [] ind;
	}

	// calcul de x
	T1.Mul(Q,pz);
	T2.Add(u,&T1);
	T1.Mul(&iP,&T2);
	px->Neg(&T1);

	iP.Free();
	I.Free();
	C.Free();
	M.Free();
	q.Free();
	B.Free();
	iB.Free();
	En.Free();
	y.Free();
	T1.Free();
	T2.Free();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	G�n�ral
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Init_WorldUp(float x,float y,float z)
{
	up.Init(x,y,z);
}

float f_abs(float f)
{
    if (f<0) return -f;
    return f;
    //int x = ((*(int *)(&f)) & 0x7FFFFFFF);
    //return *(float *)(&x);
}

double d_abs(double d)
{
	if (d<0) return -d;
	else return d;
}

int i_abs(int n)
{
	if (n<0) return -n;
	return n;
}



float f_sat(float f)
{
	if (f<0) return 0;
	else
	{
		if (f>1) return 1;
		else return f;
	}
}

double d_sat(double d)
{
	if (d<0) return 0;
	else 
	{
		if (d>1) return 1;
		else return d;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char str_tmp_log_representation[1024];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CMatrix_MxN::strlog()
{
    char *s=str_tmp_log_representation;
    s[0]=0;
    for (int i=0;i<m;i++)
    {
        for (int j=0;j<n;j++)
        {
            if (j!=n-1) sprintf(s,"%s%3.5f ",s,a[i*n+j]);
            else sprintf(s,"%s%3.5f\n",s,a[i*n+j]);
        }
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CMatrix::strlog()
{
    char *s=str_tmp_log_representation;
    
    sprintf(s,"%3.5f %3.5f %3.5f %3.5f\n%3.5f %3.5f %3.5f %3.5f\n%3.5f %3.5f %3.5f %3.5f\n%3.5f %3.5f %3.5f %3.5f\n",
            a[0][0],a[0][1],a[0][2],a[0][3],
            a[1][0],a[1][1],a[1][2],a[1][3],
            a[2][0],a[2][1],a[2][2],a[2][3],
            a[3][0],a[3][1],a[3][2],a[3][3] );

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Nanouk(float value)
{
    return ((*(unsigned int *)&value) & 0x7fffffff) > 0x7f800000;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CVector::nanish()
{
    if (Nanouk(x)) return true;
    if (Nanouk(y)) return true;
    if (Nanouk(z)) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVector::strlog()
{
    char *s=str_tmp_log_representation;
    sprintf(s,"xyz: %3.5f %3.5f %3.5f",x,y,z);
    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CQuaternion::strlog()
{
    char *ss=str_tmp_log_representation;
    sprintf(ss,"s:%3.5f n:%3.5f %3.5f %3.5f",s,n.x,n.y,n.z);
    return ss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CVector::strlogf()
{
    char *s=str_tmp_log_representation;
    sprintf(s,"%3.5f %3.5f %3.5f",x,y,z);
    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char * CQuaternion::strlogf()
{
    char *ss=str_tmp_log_representation;
    sprintf(ss,"%3.5f %3.5f %3.5f %3.5f",s,n.x,n.y,n.z);
    return ss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMatrix::mul3x3(CMatrix &M)
{
    CMatrix TF=Transform()*M.Transform();
    a[0][0]=TF.a[0][0]; a[0][1]=TF.a[0][1]; a[0][2]=TF.a[0][2];
    a[1][0]=TF.a[1][0]; a[1][1]=TF.a[1][1]; a[1][2]=TF.a[1][2];
    a[2][0]=TF.a[2][0]; a[2][1]=TF.a[2][1]; a[2][2]=TF.a[2][2];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
