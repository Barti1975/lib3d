
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

#ifndef _MATHS_H_
#define _MATHS_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TESTS
#define API3D_STUB
#else
#include "defines.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265358979f

/*
#define SMALLF 0.00001f
#define SMALLF3 0.01f
#define SMALLF2 0.001f
*/

extern float SMALLF;
extern float SMALLF2;
extern float SMALLF3;

#define MULTIPLICATION_A_GAUCHE									// multiplication des CVectors � gauche
//#define MULTIPLICATION_A_DROITE								// multiplication des CVectors � droite	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Vector definition
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float f_sat(float f);
double d_sat(double d);
int i_abs(int n);
float f_abs(float f);
double d_abs(double d);

float _M_invsqrtf(float x);
float _M_sqrtf(float x);
float _M_cosf(float x);
float _M_sinf(float x);
float _M_acosf(float x);
float _M_asinf(float x);
float _M_tanf(float x);
#define _M_max(a,b) (a>=b?a:b)
#define _M_min(a,b) (a<=b?a:b)
float _M_atan2f(float y, float x);
float _M_atanf(float x);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class Vector
class API3D_STUB CVector
{
public:
    
	union
	{
		struct 
		{ 
			float x,y,z;
		};
		float v[3];
	};

	CVector() {}
	CVector(float X,float Y,float Z) { x=X;y=Y;z=Z;}

	bool isNan();

	//! normalise (sqrt(x�+y�+z�))
	void Normalise();
    float Normalise2();
	//! normalise sqrt(x�+y�+z�)
	float Norme();
	float Max();
	float Min();
	//! pseudo-normalise (x�+y�+z�)
	float Norme2();
	float NormeCarree();
	void Init(float X,float Y,float Z);											// charge CVector
	//! graam schmidt orthonormalisation
	void Base(CVector v,int tag);
	void neg();

    //! get string representation
    char * strlog();
    char * strlogf();

	//! permutation of coo.
	void permut(int cycle);
    
    bool nanish();
    
    //! misc
    CVector frac();
    //! misc
    CVector integer();
    //! rotation quick
    CVector rot_x(float a);
    CVector rot_y(float a);
    CVector rot_z(float a);
    //! misc
    void clamp(float min,float max);
    //! angles
    float angle(CVector& v);
    //! angles
    float angle2(CVector& v,CVector& w);
    //! rand
    CVector random();
    // ...
    void cross(CVector u,CVector v);
    float dot(CVector u);

	//! cross product
	inline CVector operator ^(CVector const&v) const
	{
		CVector vectoriel;

		vectoriel.x=y*v.z - z*v.y;
		vectoriel.y=z*v.x - x*v.z;
		vectoriel.z=x*v.y - y*v.x;

		return vectoriel;
	}


	//! dot product
	inline float operator ||(CVector const&v)
	{
		float s;
		s=(x*v.x + y*v.y + z*v.z);
		return s;
	}

	//! addition
	inline CVector operator +(CVector const&v) const
	{
		CVector somme;

		somme.x=x + v.x;
		somme.y=y + v.y;
		somme.z=z + v.z;

		return somme;
	}

	//! subtract
	inline CVector operator -(CVector const&v) const
	{
		CVector somme;

		somme.x=x - v.x;
		somme.y=y - v.y;
		somme.z=z - v.z;

		return somme;
	}

    //! scale
    inline CVector operator *(CVector const&v) const
    {
        CVector m;
        
        m.x=x * v.x;
        m.y=y * v.y;
        m.z=z * v.z;
        
        return m;
    }

    //! inv scale
    inline CVector operator /(CVector const&v) const
    {
        CVector m;
        
        m.x=x / v.x;
        m.y=y / v.y;
        m.z=z / v.z;
        
        return m;
    }
    
	inline void operator -=(CVector const&v)
	{
		x-=v.x;
		y-=v.y;
		z-=v.z;
	}

	inline void operator +=(CVector const&v)
	{
		x+=v.x;
		y+=v.y;
		z+=v.z;
	}
	
	//! multiplication
	inline CVector operator *(float f) const
	{
		CVector scaled;

		scaled.x=x*f;
		scaled.y=y*f;
		scaled.z=z*f;

		return scaled;
	}


	inline void operator /=(float f)
	{
		float surf;
		surf=1.0f/f;
		x*=surf;
		y*=surf;
		z*=surf;
	}

	inline void operator *=(float f)
	{
		x*=f;
		y*=f;
		z*=f;
	}

	//! scalar division
	inline CVector operator /(float f) const
	{
		CVector scaled;
		float surf;
		surf=1.0f/f;
		scaled.x=x*surf;
		scaled.y=y*surf;
		scaled.z=z*surf;
		return scaled;
	}


	//! scalar multiplication
	inline friend CVector operator *(float f,CVector const&v)
    {
    	CVector scaled;
		scaled.x=v.x*f;
		scaled.y=v.y*f;
		scaled.z=v.z*f;
	    return scaled;
	}

	// integer

	inline CVector operator *(int i) const
	{
		CVector scaled;

		scaled.x=x*((float)i);
		scaled.y=y*((float)i);
		scaled.z=z*((float)i);

		return scaled;
	}


	inline void operator /=(int i)
	{
		float surf;
		surf=1.0f/((float)i);
		x*=surf;
		y*=surf;
		z*=surf;
	}

	inline void operator *=(int i)
	{
		x*=((float)i);
		y*=((float)i);
		z*=((float)i);
	}

	inline CVector operator /(int i) const
	{
		CVector scaled;
		float surf;
		surf=1.0f/((float)i);
		scaled.x=x*surf;
		scaled.y=y*surf;
		scaled.z=z*surf;
		return scaled;
	}


	inline friend CVector operator *(int i,CVector const&v)
    {
    	CVector scaled;
		scaled.x=v.x*((float)i);
		scaled.y=v.y*((float)i);
		scaled.z=v.z*((float)i);
	    return scaled;
	}


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class Vector2 2d vectors
class API3D_STUB CVector2
{
public:
	float x,y;

	CVector2() {}
	CVector2(float X,float Y) { x=X;y=Y;}
	void Init(float X,float Y);
	void Mod1(float dx,float dy);
	float Norme();
	void Normalise();


	//! scalar multiplication
	inline CVector2 operator *(float f)	
	{
		CVector2 scale;
		scale.x=f*x;
		scale.y=f*y;
		return scale;
	}


	inline void operator /=(float f)
	{
		float surf;
		surf=1.0f/f;
		x*=surf;
		y*=surf;
	}

	inline void operator /=(int f)
	{
		float surf;
		surf=1.0f/(float) f;
		x*=surf;
		y*=surf;
	}

	inline void operator *=(int f)
	{
		x*=f;
		y*=f;
	}

	inline void operator *=(float f)
	{
		x*=f;
		y*=f;
	}

	inline void operator +=(CVector2 const&m)
	{
		x+=m.x;
		y+=m.y;
	}

	inline void operator -=(CVector2 const&m)
	{
		x-=m.x;
		y-=m.y;
	}

	//! scalar division
	inline CVector2 operator /(float f) const
	{
		CVector2 scale;
		scale.x=x/f;
		scale.y=y/f;
		return scale;
	}

	//! addition
	inline CVector2 operator +(CVector2 const&v) const
	{
		CVector2 somme;
		somme.x=x + v.x;
		somme.y=y + v.y;
		return somme;
	}

	//! subtract
	inline CVector2 operator -(CVector2 const&v) const
	{
		CVector2 diff;
		diff.x=x - v.x;
		diff.y=y - v.y;
		return diff;
	}

    inline CVector2 operator *(CVector2 const&v) const
    {
        CVector2 mm;
        mm.x=x * v.x;
        mm.y=y * v.y;
        return mm;
    }

	inline friend const CVector2 operator *(float f,CVector2 const&v)
    {
    	CVector2 scale;
		scale.x=f*v.x;
		scale.y=f*v.y;
		return scale;
	}

	//! dot product
	inline float operator ||(CVector2 const&v)
    {
		return x*v.x + y*v.y;
	}

    void Rotate(float ang);
    void RotateDeg(float ang);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class Vector4 4d vectors
class API3D_STUB CVector4
{
public:
	float x,y,z,w;

	CVector4() {}
	CVector4(float X,float Y,float Z,float W) { x=X;y=Y;z=Z;w=W;}
	void Init(float X,float Y,float Z,float W);
	void Init(CVector const&v);
    
    //! scalar multiplication
    inline CVector4 operator *(float f)
    {
        CVector4 scale;
        scale.x=f*x;
        scale.y=f*y;
        scale.z=f*z;
        scale.w=f*w;
        return scale;
    }

    inline void operator /=(float f)
    {
        float surf;
        surf=1.0f/f;
        x*=surf;
        y*=surf;
        z*=surf;
        w*=surf;
    }

    inline void operator /=(int f)
    {
        float surf;
        surf=1.0f/(float) f;
        x*=surf;
        y*=surf;
        z*=surf;
        w*=surf;
    }

    inline void operator *=(int f)
    {
        x*=f;
        y*=f;
        z*=f;
        w*=f;
    }

    inline void operator *=(float f)
    {
        x*=f;
        y*=f;
        z*=f;
        w*=f;
    }

    inline void operator +=(CVector4 const&m)
    {
        x+=m.x;
        y+=m.y;
        z+=m.z;
        w+=m.w;
    }

    inline void operator -=(CVector4 const&m)
    {
        x-=m.x;
        y-=m.y;
        z-=m.z;
        w-=m.w;
    }

    //! scalar division
    inline CVector4 operator /(float f) const
    {
        CVector4 scale;
        scale.x=x/f;
        scale.y=y/f;
        scale.z=z/f;
        scale.w=w/f;
        return scale;
    }

    //! addition
    inline CVector4 operator +(CVector4 const&v) const
    {
        CVector4 somme;
        somme.x=x + v.x;
        somme.y=y + v.y;
        somme.z=z + v.z;
        somme.w=w + v.w;
        return somme;
    }

    //! subtract
    inline CVector4 operator -(CVector4 const&v) const
    {
        CVector4 diff;
        diff.x=x - v.x;
        diff.y=y - v.y;
        diff.z=z - v.z;
        diff.w=w - v.w;
        return diff;
    }

    inline CVector4 operator *(CVector4 const&v) const
    {
        CVector4 mm;
        mm.x=x * v.x;
        mm.y=y * v.y;
        mm.z=z * v.z;
        mm.w=w * v.w;
        return mm;
    }

    inline friend const CVector4 operator *(float f,CVector4 const&v)
    {
        CVector4 scale;
        scale.x=f*v.x;
        scale.y=f*v.y;
        scale.z=f*v.z;
        scale.w=f*v.w;
        return scale;
    }

    //! dot product
    inline float operator ||(CVector4 const&v)
    {
        return x*v.x + y*v.y + z*v.z + w*v.w;
    }

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Plane definition
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Class Plane of equation a.x + b.y + c.z + d = 0
class API3D_STUB CPlane
{
public:
	float a,b,c,d;

	CPlane() {}
	CPlane(float A,float B,float C,float D) {	a=A;b=B;c=C;d=D; }

	void Normalise();

	//! distance to plane
	float Distance(CVector const&v);

	//! distance to plane
	float Distance_xyz(float x,float y,float z);

	void Init(float A,float B,float C,float D);

	//! dot product vector 4 / plan
	inline friend float operator ||(CVector4 const&v,CPlane const&p)
    {
    	float s;
		s=v.x*p.a+v.y*p.b+v.z*p.c+v.w*p.d;
		return s;
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class Plane of equation a.x + b.y + c.z + d = 0 (float vs double)
class API3D_STUB CPlane2
{
public:
	double a,b,c,d;
	CPlane2() {}
	void Init(double A,double B,double C,double D);
	void Normalise();
	double Distance(CVector const&v);
	double Distance_xyz(float x,float y,float z);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Quaternion
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMatrix;

//! Class Quaternion
class API3D_STUB CQuaternion
{
public:
	float s;
	CVector n;

	CQuaternion() {}

	void Id();
	//! initialisation
	void Init(float w,float x,float y,float z);
	//! norm value
	float Norme();
	//! normalise
	void Normalise();
	//! set orientation from axis and an angle
	void AxisAngle(CVector const&u,float angle);
	//! set orientation from axis and an angle
	void AxisAngleSC(CVector const&u,float angle);
	//! get orientation in form of an axis and an angle
	void GetAxisAngle(CVector *u,float *angle);
	//! set eulers orientation
	void SetEuler(float rx,float ry,float rz);
	//! conjugate
	CQuaternion Conjugate();
	//! inverse
	CQuaternion Inverse();
	//! conjugate
	CMatrix Transform();
	//! dot product
	float Dot(CQuaternion const&q);
	//! slerp interpolation (cf. stagiaires)
	void Slerp(CQuaternion *quat1,CQuaternion *quat2,float slerp);
    //! convert matrix
    void From(CMatrix &M);
    //! convert to eulers (radians)
    void GetEulers(float &rx,float &ry,float &rz);
    //! convert to eulers (degrees)
    void GetEulersDeg(float &rx,float &ry,float &rz);
    
    //! get string representation
    char * strlog();
    char * strlogf();

	//! addition
	inline CQuaternion operator +(CQuaternion const&q2) const
	{
		CQuaternion q;
		q.s=s+q2.s;
		q.n=n+q2.n;
		return q;
	}

	//! subtract
	inline CQuaternion operator -(CQuaternion const&q2) const
	{
		CQuaternion q;
		q.s=s-q2.s;
		q.n=n-q2.n;
		return q;
	}

	//! multiplication
	inline CQuaternion operator *(CQuaternion const&q2) const
	{
		CQuaternion q;
		
		q.s=s*q2.s - (n.x*q2.n.x + n.y*q2.n.y +n.z*q2.n.z);
		
		q.n.x=n.y*q2.n.z - n.z*q2.n.y +s*q2.n.x + q2.s*n.x;
		q.n.y=n.z*q2.n.x - n.x*q2.n.z +s*q2.n.y + q2.s*n.y;
		q.n.z=n.x*q2.n.y - n.y*q2.n.x +s*q2.n.z + q2.s*n.z;


		return q;
	}


	//! scalar multiplication
	inline CQuaternion operator *(float f) const
	{
		CQuaternion q;
		
		q.s=s*f;
		q.n.x=n.x*f;
		q.n.y=n.y*f;
		q.n.z=n.z*f;

		return q;
	}

	//! scalar division
	inline CQuaternion operator /(float f) const
	{
		CQuaternion q;
		
		q.s=s/f;
		q.n.x=n.x/f;
		q.n.y=n.y/f;
		q.n.z=n.z/f;

		return q;
	}

	inline CQuaternion operator *(int i) const
	{
		CQuaternion q;
		
		q.s=s*((float)i);
		q.n.x=n.x*((float)i);
		q.n.y=n.y*((float)i);
		q.n.z=n.z*((float)i);

		return q;
	}


	inline CQuaternion operator /(int i) const
	{
		CQuaternion q;
		
		q.s=s/((float)i);
		q.n.x=n.x/((float)i);
		q.n.y=n.y/((float)i);
		q.n.z=n.z/((float)i);

		return q;
	}


	inline CQuaternion operator *(CVector const&v) const
	{
		CQuaternion q,q0;

		q0.s=0;
		q0.n=v;
		q=(*this)*q0;

		return q;

	}

	inline friend const CQuaternion operator *(float f,CQuaternion const&q1)
    {
    	CQuaternion q;
	
    	q.s=q1.s*f;
		q.n.x=q1.n.x*f;
		q.n.y=q1.n.y*f;
		q.n.z=q1.n.z*f;

		return q;
	}

	inline friend CQuaternion operator *(int i,CQuaternion const&q1)
    {
    	CQuaternion q;
	
    	q.s=q1.s*((float)i);
		q.n.x=q1.n.x*((float)i);
		q.n.y=q1.n.y*((float)i);
		q.n.z=q1.n.z*((float)i);

		return q;
	}

	
	//! vector multiplication
    inline friend CQuaternion operator *(CVector const&v,CQuaternion const&q1)
    {
    	CQuaternion q,q0;

		q0.s=0;
		q0.n=v;

		q=q0*q1;

	    return q;
	}


};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Matrix
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class CMatrix
class API3D_STUB CMatrix
{
public:
	float a[4][4];

	CMatrix()
	{
		// init : CMatrix identit�
		a[0][0]=1;		a[1][0]=0;		a[2][0]=0;		a[3][0]=0;
		a[0][1]=0;		a[1][1]=1;		a[2][1]=0;		a[3][1]=0;
		a[0][2]=0;		a[1][2]=0;		a[2][2]=1;		a[3][2]=0;
		a[0][3]=0;		a[1][3]=0;		a[2][3]=0;		a[3][3]=1;
	}

	bool isNan();

	//! tranform from quaternion
	void OrientationQuaternion(CQuaternion const&q);
	//! tranform from quaternion
	void Mq(CQuaternion const&q);
	//! tranform : eulers radians
	//! multiply by CMatrix rotation
	void Rotation(float rx,float ry,float rz);
	//! tranform : eulers degree
	//! multiply by CMatrix rotation in degree
	void RotationDegre(float rx,float ry,float rz);
	void BaseRotationDegre(float rx,float ry,float rz);
	//! tranform : translation
	//! multiply by CMatrix translation
	void Translation(float x,float y,float z);
	//! tranform : scaling
	//! multiply by CMatrix scaling
	void Scaling(float x,float y,float z);

	//! tranform : eulers radians
	//! multiply by CMatrix rotation
	void Rotation(CVector const&r);
	//! tranform : eulers degree
	//! multiply by CMatrix rotation in degree
	void RotationDegre(CVector const&rd);
	//! tranform : translation
	//! multiply by CMatrix translation
	void Translation(CVector const&t);
	//! tranform : scaling
	//! multiply by CMatrix scaling
	void Scaling(CVector const&s);

	//! tranform : load identity
	void Id();
	//! affectation
	void Egal(CMatrix const&m);
    //! affectation
    void Mul(CMatrix const&m);
	//! load transpose matrix (m)
	void Transpose(CMatrix const&m);
	//! return transpose matrix (m)
	CMatrix Transpose();
	//! return orientation transform matrix (m)
	CMatrix Transform();
	//! return translation transform matrix (m)
	CVector Translation();
	//! transform : rotation axi/angle radians
	void RotationAngleVecteur(CVector const&u,float angle);
	//! transform : rotation axi/angle degree
	void RotationAngleVecteurDegre(CVector const&u,float angle);
		
	//! transform : fitting position avec vector orientation
	// X axe of object on axe u + pos translation
	void Orientation_x(CVector const&pos,CVector const&u);
	//! transform : fitting position avec vector orientation
	// Y axe of object on axe u + pos translation
	void Orientation_y(CVector const&pos,CVector const&u);
	//! transform : fitting position avec vector orientation
	// Z axe of object on axe u + pos translation
	void Orientation_z(CVector const&pos,CVector const&u);

	//! symetric
	void Symetric(CVector const&s);

	//! transform : view matrix
	void View(CVector const&pos,CVector const&direction,float roll);

	//! transform : base matrix
	void Base(CVector &T,CVector &N,CVector &B);

	//! load diagonal matrix
	void Diag(float a1,float a2,float a3,float a4);
	//! 3x3 determinant
	float Determinant3x3();
	//! load 3x3 inverse of M
	void Inverse3x3(CMatrix &M);
	
	//! 3x3 matrix trace
	float Trace3x3();
	//! 3x3 matrix norm
	float Norme3x3();
	//! jacobi algorithm
	void Jacobi(CMatrix &M);

	//! load 4x3 inverse of M
	void Inverse4x4(CMatrix &M);

	//! return 3x3 inverse of M
	CMatrix Inverse3x3();

	//! return 4x3 inverse of M
	CMatrix Inverse4x4();

    //! supper Scale transform
    CMatrix unScale();

    //! load translation vector
    CVector getTranslation();
    
    //! load scale vector
    CVector getScaling();

    //! load colum/row vector
    CVector column(int n);
    
    //! load colum/row vector
    CVector4 column4(int n);
    
    //! load colum/row vector
    CVector row(int n);
    
    //! load colum/row vector
    CVector4 row4(int n);
    
	//! load reflection matrix
	void Reflection(float aa,float bb,float cc,float dd);

	//! load projection matrix
	void ShadowProjection(CVector4 const&Light,CPlane const&P);

    //! D3D framing
    void CFrame(CMatrix const&M);
    
    //! Eulers
    void Eulers(float *x,float *y,float *z);
    
    //! Eulers
    void EulersDegrees(float *x,float *y,float *z);
    
    //! X,Y,Z
    void XYZ(CVector const&vX,CVector const&vY,CVector const&vZ);
    
    //! multiply only transform
    void mul3x3(CMatrix &M);

    //! get string representation
    char * strlog();

	// pr�voir calcul CMatrix inverse d�terminant etc.

	inline float& operator()(int n1,int n2) 
	{ 
		return a[n1][n2];
	}

	// multiplication matrix-matrix
    inline friend CMatrix operator *(CMatrix const&m1,CMatrix const&m2)
    {
	    CMatrix res;

		res.a[0][0]=m1.a[0][0]*m2.a[0][0] + m1.a[0][1]*m2.a[1][0] + m1.a[0][2]*m2.a[2][0] + m1.a[0][3]*m2.a[3][0];
		res.a[1][0]=m1.a[1][0]*m2.a[0][0] + m1.a[1][1]*m2.a[1][0] + m1.a[1][2]*m2.a[2][0] + m1.a[1][3]*m2.a[3][0];
		res.a[2][0]=m1.a[2][0]*m2.a[0][0] + m1.a[2][1]*m2.a[1][0] + m1.a[2][2]*m2.a[2][0] + m1.a[2][3]*m2.a[3][0];
		res.a[3][0]=m1.a[3][0]*m2.a[0][0] + m1.a[3][1]*m2.a[1][0] + m1.a[3][2]*m2.a[2][0] + m1.a[3][3]*m2.a[3][0];

		res.a[0][1]=m1.a[0][0]*m2.a[0][1] + m1.a[0][1]*m2.a[1][1] + m1.a[0][2]*m2.a[2][1] + m1.a[0][3]*m2.a[3][1];
		res.a[1][1]=m1.a[1][0]*m2.a[0][1] + m1.a[1][1]*m2.a[1][1] + m1.a[1][2]*m2.a[2][1] + m1.a[1][3]*m2.a[3][1];
		res.a[2][1]=m1.a[2][0]*m2.a[0][1] + m1.a[2][1]*m2.a[1][1] + m1.a[2][2]*m2.a[2][1] + m1.a[2][3]*m2.a[3][1];
		res.a[3][1]=m1.a[3][0]*m2.a[0][1] + m1.a[3][1]*m2.a[1][1] + m1.a[3][2]*m2.a[2][1] + m1.a[3][3]*m2.a[3][1];

		res.a[0][2]=m1.a[0][0]*m2.a[0][2] + m1.a[0][1]*m2.a[1][2] + m1.a[0][2]*m2.a[2][2] + m1.a[0][3]*m2.a[3][2];
		res.a[1][2]=m1.a[1][0]*m2.a[0][2] + m1.a[1][1]*m2.a[1][2] + m1.a[1][2]*m2.a[2][2] + m1.a[1][3]*m2.a[3][2];
		res.a[2][2]=m1.a[2][0]*m2.a[0][2] + m1.a[2][1]*m2.a[1][2] + m1.a[2][2]*m2.a[2][2] + m1.a[2][3]*m2.a[3][2];
		res.a[3][2]=m1.a[3][0]*m2.a[0][2] + m1.a[3][1]*m2.a[1][2] + m1.a[3][2]*m2.a[2][2] + m1.a[3][3]*m2.a[3][2];

		res.a[0][3]=m1.a[0][0]*m2.a[0][3] + m1.a[0][1]*m2.a[1][3] + m1.a[0][2]*m2.a[2][3] + m1.a[0][3]*m2.a[3][3];
		res.a[1][3]=m1.a[1][0]*m2.a[0][3] + m1.a[1][1]*m2.a[1][3] + m1.a[1][2]*m2.a[2][3] + m1.a[1][3]*m2.a[3][3];
		res.a[2][3]=m1.a[2][0]*m2.a[0][3] + m1.a[2][1]*m2.a[1][3] + m1.a[2][2]*m2.a[2][3] + m1.a[2][3]*m2.a[3][3];
		res.a[3][3]=m1.a[3][0]*m2.a[0][3] + m1.a[3][1]*m2.a[1][3] + m1.a[3][2]*m2.a[2][3] + m1.a[3][3]*m2.a[3][3];

	    return res;
	}


	// multiplication vector-matrix

    inline friend CVector operator *(CVector const&v,CMatrix const&m)
    {
    	CVector transformed;
	
    	transformed.x=v.x * m.a[0][0] + v.y * m.a[1][0] + v.z * m.a[2][0] + m.a[3][0];
		transformed.y=v.x * m.a[0][1] + v.y * m.a[1][1] + v.z * m.a[2][1] + m.a[3][1];
		transformed.z=v.x * m.a[0][2] + v.y * m.a[1][2] + v.z * m.a[2][2] + m.a[3][2];

		return transformed;
	}

	// multiplication vector4-matrix
    inline friend CVector4 operator *(CVector4 const&v,CMatrix const&m)
    {
    	CVector4 transformed;
	
    	transformed.x=v.x * m.a[0][0] + v.y * m.a[1][0] + v.z * m.a[2][0] + v.w*m.a[3][0];
		transformed.y=v.x * m.a[0][1] + v.y * m.a[1][1] + v.z * m.a[2][1] + v.w*m.a[3][1];
		transformed.z=v.x * m.a[0][2] + v.y * m.a[1][2] + v.z * m.a[2][2] + v.w*m.a[3][2];
		transformed.w=v.x * m.a[0][3] + v.y * m.a[1][3] + v.z * m.a[2][3] + v.w*m.a[3][3];

		transformed.x/=transformed.w;
		transformed.y/=transformed.w;
		transformed.z/=transformed.w;
		transformed.w=1.0;


		return transformed;
	}

    inline friend CVector operator *(CMatrix const&m,CVector const&v)
    {
    	CVector transformed;
	
    	transformed.x=v.x * m.a[0][0] + v.y * m.a[0][1] + v.z * m.a[0][2] + m.a[0][3];
		transformed.y=v.x * m.a[1][0] + v.y * m.a[1][1] + v.z * m.a[1][2] + m.a[1][3];
		transformed.z=v.x * m.a[2][0] + v.y * m.a[2][1] + v.z * m.a[2][2] + m.a[2][3];

		return transformed;
	}
	
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mnMatrix
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Matrix (m,n)
//! usefull for physics
class API3D_STUB CMatrix_MxN
{
public:
	float * a;
	int m,n;

    CMatrix_MxN();	
	~CMatrix_MxN(); 

	inline float& operator()(int l,int c) 
	{ 
		return a[l*n + c];
	}

	//! determinant
	float Determinant();
	//! initialisation (0..0)
	void Init(int M,int N);	// ligne,colonne
	//! load identity
	void Id();
	//! load zero matrix
	void Zero();
	//! multiplication scalar
	void Mul(float s);
	//! patch portion of matrix
	void Patch(int l,int c,CMatrix_MxN * M);
	//! patch portion of matrix
	void Patch(int l,int c,CMatrix_MxN * M,int sl,int sc,int dl,int dc);
	//! returns transpose
	CMatrix_MxN* Transpose();
	//! returns transpose
	CMatrix_MxN* TransposeSign();
	//! load a CMatrix
	void Load4x4(CMatrix *M);
	//! load a CMatrix
	void Load4x4(CMatrix const&M);
	//! swap lines
	void Swap(int l1,int l2);
	//! returns inverse matrix (for � matrix)
	CMatrix_MxN* Inverse();
	//! returns solve of (for � matrix)
	CMatrix_MxN* ResolveAXB(CMatrix_MxN * B);
	//! freeing
	void Free();
    //! jacobian holonom link matrix
    void Jacobian(CVector const&r,float sign);
    //! jacobian holonom spring matrix
    void JacobianSpring(CVector const&r,float sign,float raideur);
	//! jacobian holonom link matrix
	void Jacobian3(CVector const&r,float sign);
	//! jacobian holonom link matrix
	void JacobianPivot(CVector const&r,CVector const&x,float sign);
    void JacobianPivotFixe(CVector const&r,CVector const&x,float sign);
	//! jacobian holonom link matrix
	void JacobianContact(CVector const&G,CVector const&N,CVector const&p,float sign);

	//! jacobian holonom link matrix
	void Jacobian2D(float rx,float ry,float sign);
    void Jacobian2DSpring(float rx,float ry,float sign,float raideur);
    void Jacobian2DPivot(float rx,float ry,float sign);
    
	void InvExchangeLine(int line,CMatrix_MxN * M,CMatrix_MxN * iM,CMatrix_MxN * newLine,CMatrix_MxN * Mnew,CMatrix_MxN * iMnew);
	void InvExchangeColumn(int column,CMatrix_MxN * M,CMatrix_MxN * iM,CMatrix_MxN * newColumn,CMatrix_MxN * Mnew,CMatrix_MxN * iMnew);
	void LEn(int N,int n,float value);
	void CEn(int N,int n,float value);
	//! duplicate matrix
	void Duplicate(CMatrix_MxN *M);
	void Column(CMatrix_MxN *M,int c);
	void Line(CMatrix_MxN *M,int l);
	//! multiplication of two matrix
	void Mul(CMatrix_MxN *A,CMatrix_MxN *B);
	//! division of matrix by scalar
	void Div(CMatrix_MxN *A,float div);
	//! subtract of matrix
	void Sub(CMatrix_MxN *A,CMatrix_MxN *B);
	//! addition of matrix
	void Add(CMatrix_MxN *A,CMatrix_MxN *B);
	//! negate a matrix
	void Neg(CMatrix_MxN *A);
	//! resolve LCP problem
	void ResolveLCP(CMatrix_MxN *P,CMatrix_MxN *Q,CMatrix_MxN *R,CMatrix_MxN *S,CMatrix_MxN *u,CMatrix_MxN *v,CMatrix_MxN *px,CMatrix_MxN *pz,CMatrix_MxN *pw,bool drop);
	void Inverse(CMatrix_MxN *M);
    //! log
    char * strlog();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions g�n�rales
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Init_WorldUp(float x,float y,float z);

typedef CVector2 CLASS_NAME_2DVECTOR;
typedef CVector CLASS_NAME_VECTOR;
typedef CQuaternion CLASS_NAME_QUATERNION;
typedef CMatrix CLASS_NAME_MATRIX;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*?

	inlining for optimisation:

		VECTOR3MATRIX(u,v,m)
		VECTOR3PMATRIX(u,v,m)
		VECTOR4MATRIX(u,v,m)
		DOTPRODUCT(s,v1,v2)
		DOT(v1,v2)
		INVDOTPRODUCT(s,v1,v2)
		CROSSPRODUCT(u,v1,v2)
		VECTORADD(u,v1,v2)
		VECTORMAPBASE(u,base,t1,v1,t2,v2)
		VECTORINTERPOL(u,t,v1,v2)
		VECTOR4INTERPOL(u,t,v1,v2)
		VECTOR2INTERPOL(u,t,v1,v2)
		VECTORSUB(u,v1,v2)
		VECTORINIT(u,xx,yy,zz)
		VECTOR4INIT(u,v)
		VECTORMUL(u,v,f)
		VECTORNEG(u)
		VECTORDIV(u,v,f)
		VECTORNORMALISE(u)
		VECTORNORME(s,u)
		VECTORNORME2(s,u)
		VECTORNORM(u)
		CROSSQUAD(res,A1,B1,A2,B2)
		MATRIXMATRIX(res,m1,m2)
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern CVector ___tmp1,___tmp2;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define VECTOR3MATRIX(u,v,m)													\
{																				\
	float ___x=v.x;													\
	float ___y=v.y;													\
	float ___z=v.z;													\
   	u.x=___x*(m.a[0][0]) + ___y*(m.a[1][0]) + ___z*(m.a[2][0]) + (m.a[3][0]);	\
	u.y=___x*(m.a[0][1]) + ___y*(m.a[1][1]) + ___z*(m.a[2][1]) + (m.a[3][1]);	\
	u.z=___x*(m.a[0][2]) + ___y*(m.a[1][2]) + ___z*(m.a[2][2]) + (m.a[3][2]);	\
}																				\

#define VECTOR3MATRIX3X3(u,v,m)												\
{																			\
	float ___x=v.x;												\
	float ___y=v.y;												\
	float ___z=v.z;												\
   	u.x=___x*(m.a[0][0]) + ___y*(m.a[1][0]) + ___z*(m.a[2][0]);				\
	u.y=___x*(m.a[0][1]) + ___y*(m.a[1][1]) + ___z*(m.a[2][1]);				\
	u.z=___x*(m.a[0][2]) + ___y*(m.a[1][2]) + ___z*(m.a[2][2]);				\
}																			\

#define VECTOR3PMATRIX3X3(u,v,m)											\
{																			\
	float ___x=v.x;												\
	float ___y=v.y;												\
	float ___z=v.z;												\
   	u.x=___x*(m->a[0][0]) + ___y*(m->a[1][0]) + ___z*(m->a[2][0]);			\
	u.y=___x*(m->a[0][1]) + ___y*(m->a[1][1]) + ___z*(m->a[2][1]);			\
	u.z=___x*(m->a[0][2]) + ___y*(m->a[1][2]) + ___z*(m->a[2][2]);			\
}																			\

#define VECTOR3PMATRIX(u,v,m)														\
{																					\
	float ___x=v.x;														\
	float ___y=v.y;														\
	float ___z=v.z;														\
   	u.x=___x*(m->a[0][0]) + ___y*(m->a[1][0]) + ___z*(m->a[2][0]) + (m->a[3][0]);	\
	u.y=___x*(m->a[0][1]) + ___y*(m->a[1][1]) + ___z*(m->a[2][1]) + (m->a[3][1]);	\
	u.z=___x*(m->a[0][2]) + ___y*(m->a[1][2]) + ___z*(m->a[2][2]) + (m->a[3][2]);	\
}																					\


#define VECTOR4MATRIX(u,v,m)														\
{																					\
	float ___x=v.x;														\
	float ___y=v.y;														\
	float ___z=v.z;														\
	float ___w=v.w;														\
    u.x=___x * m.a[0][0] + ___y * m.a[1][0] + ___z * m.a[2][0] + ___w * m.a[3][0];	\
	u.y=___x * m.a[0][1] + ___y * m.a[1][1] + ___z * m.a[2][1] + ___w * m.a[3][1];	\
	u.z=___x * m.a[0][2] + ___y * m.a[1][2] + ___z * m.a[2][2] + ___w * m.a[3][2];	\
	u.w=___x * m.a[0][3] + ___y * m.a[1][3] + ___z * m.a[2][3] + ___w * m.a[3][3];	\
	u.x/=u.w;																		\
	u.y/=u.w;																		\
	u.z/=u.w;																		\
	u.w=1.0;																		\
}																					\

#define DOTPRODUCT(s,v1,v2)													\
{																			\
	s=v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;									\
}																			\

#define DOT(v1,v2) (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z)

#define INVDOTPRODUCT(s,v1,v2)												\
{																			\
	s=-(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);									\
}																			\


#define CROSSPRODUCT(u,v1,v2)												\
{																			\
	float ___x=v1.x;												\
	float ___y=v1.y;												\
	float ___z=v1.z;												\
	u.x=___y*v2.z - ___z*v2.y;												\
	u.y=___z*v2.x - ___x*v2.z;												\
	u.z=___x*v2.y - ___y*v2.x;												\
}																			\

#define VECTORADD(u,v1,v2)													\
{																			\
	u.x=v1.x+v2.x;															\
	u.y=v1.y+v2.y;															\
	u.z=v1.z+v2.z;															\
}																			\

#define VECTORMED(u,v1,v2)													\
{																			\
	u.x=(v1.x+v2.x)*0.5f;													\
	u.y=(v1.y+v2.y)*0.5f;													\
	u.z=(v1.z+v2.z)*0.5f;													\
}																			\

#define VECTORMAPBASE(u,base,t1,v1,t2,v2)									\
{																			\
	u.x=base.x + t1*v1.x+t2*v2.x;											\
	u.y=base.y + t1*v1.y+t2*v2.y;											\
	u.z=base.z + t1*v1.z+t2*v2.z;											\
}																			\

#define VECTORINTERPOL(u,t,v1,v2)											\
{																			\
	u.x=v2.x+t*(v1.x-v2.x);													\
	u.y=v2.y+t*(v1.y-v2.y);													\
	u.z=v2.z+t*(v1.z-v2.z);													\
}																			\

#define VECTOR4INTERPOL(u,t,v1,v2)											\
{																			\
	u.x=v2.x+t*(v1.x-v2.x);													\
	u.y=v2.y+t*(v1.y-v2.y);													\
	u.z=v2.z+t*(v1.z-v2.z);													\
	u.w=v2.w+t*(v1.w-v2.w);													\
}																			\


#define VECTOR2INTERPOL(u,t,v1,v2)											\
{																			\
	u.x=v2.x+t*(v1.x-v2.x);													\
	u.y=v2.y+t*(v1.y-v2.y);													\
}																			\


#define VECTORADDDIV3(u,v1,v2,v3)											\
{																			\
	u.x=(v1.x+v2.x+v3.x)*0.333333333f;										\
	u.y=(v1.y+v2.y+v3.y)*0.333333333f;										\
	u.z=(v1.z+v2.z+v3.z)*0.333333333f;										\
}																			\


#define VECTORSUB(u,v1,v2)													\
{																			\
	u.x=v1.x-v2.x;															\
	u.y=v1.y-v2.y;															\
	u.z=v1.z-v2.z;															\
}																			\

#define VECTOR2SUB(u,v1,v2)													\
{																			\
	u.x=v1.x-v2.x;															\
	u.y=v1.y-v2.y;															\
}																			\


#define VECTORINIT(u,xx,yy,zz)												\
{																			\
	u.x=xx;																	\
	u.y=yy;																	\
	u.z=zz;																	\
}																			\


#define VECTORZERO(u)														\
{																			\
	u.x=u.y=u.z=0.0f;														\
}																			\


#define VECTOR3INIT(u,v)													\
{																			\
	u.x=v.x;																\
	u.y=v.y;																\
	u.z=v.z;																\
}																			\

#define VECTOR3INITNEG(u,v)													\
{																			\
	u.x=-v.x;																\
	u.y=-v.y;																\
	u.z=-v.z;																\
}																			\


#define VECTOR4INIT(u,v)													\
{																			\
	u.x=v.x;																\
	u.y=v.y;																\
	u.z=v.z;																\
	u.w=1.0f;																\
}																			\

#define VECTORMUL(u,v,f)													\
{																			\
	u.x=v.x*f;																\
	u.y=v.y*f;																\
	u.z=v.z*f;																\
}																			\

#define VECTORNEG(u)              											\
{																			\
	u.x=-u.x;																\
	u.y=-u.y;																\
	u.z=-u.z;																\
}																			\

#define VECTORNEGATE(u,v)              										\
{																			\
	u.x=-v.x;																\
	u.y=-v.y;																\
	u.z=-v.z;																\
}																			\


#define VECTORDIV(u,v,f)													\
{																			\
	u.x=v.x/f;																\
	u.y=v.y/f;																\
	u.z=v.z/f;																\
}																			\


#define VECTORNORMALISE(u)													\
{																			\
	float __A_s=1.0f / sqrtf(u.x*u.x + u.y*u.y + u.z*u.z);			\
	u.x*=__A_s;																\
	u.y*=__A_s;																\
	u.z*=__A_s;																\
}																			\

#define VECTORNORME(s,u)													\
{																			\
	s=(float) sqrtf(u.x*u.x + u.y*u.y + u.z*u.z);							\
}																			\


#define VECTORNORME2(s,u)													\
{																			\
	s=(u.x*u.x + u.y*u.y + u.z*u.z);										\
}																			\

#define VECTORNORM2(u) ((float) (u.x*u.x + u.y*u.y + u.z*u.z))

#define VECTORNORM(u) ((float) sqrtf(u.x*u.x + u.y*u.y + u.z*u.z))

#define VECTORNORMZ(ret,u)                                                  \
{                                                                           \
    float fff;                                                     \
    if (u.x<0) ret=-u.x;                                                    \
    else ret=u.x;                                                           \
    if (u.y<0) fff=-u.y;                                                    \
    else fff=u.y;                                                           \
    if (fff>ret) ret=fff;                                                   \
    if (u.z<0) fff=-u.z;                                                    \
    else fff=u.z;                                                           \
    if (fff>ret) ret=fff;                                                   \
}                                                                         


#define CROSSQUAD(res,A1,B1,A2,B2)											\
{																			\
	VECTORSUB(___tmp1,B1,A1);												\
	VECTORSUB(___tmp2,B2,A2);												\
	CROSSPRODUCT(res,___tmp1,___tmp2);										\
}																			\


#define MATRIXMATRIX(res,m1,m2)																						\
{																													\
	res.a[0][0]=m1.a[0][0]*m2.a[0][0] + m1.a[0][1]*m2.a[1][0] + m1.a[0][2]*m2.a[2][0] + m1.a[0][3]*m2.a[3][0];		\
	res.a[1][0]=m1.a[1][0]*m2.a[0][0] + m1.a[1][1]*m2.a[1][0] + m1.a[1][2]*m2.a[2][0] + m1.a[1][3]*m2.a[3][0];		\
	res.a[2][0]=m1.a[2][0]*m2.a[0][0] + m1.a[2][1]*m2.a[1][0] + m1.a[2][2]*m2.a[2][0] + m1.a[2][3]*m2.a[3][0];		\
	res.a[3][0]=m1.a[3][0]*m2.a[0][0] + m1.a[3][1]*m2.a[1][0] + m1.a[3][2]*m2.a[2][0] + m1.a[3][3]*m2.a[3][0];		\
	res.a[0][1]=m1.a[0][0]*m2.a[0][1] + m1.a[0][1]*m2.a[1][1] + m1.a[0][2]*m2.a[2][1] + m1.a[0][3]*m2.a[3][1];		\
	res.a[1][1]=m1.a[1][0]*m2.a[0][1] + m1.a[1][1]*m2.a[1][1] + m1.a[1][2]*m2.a[2][1] + m1.a[1][3]*m2.a[3][1];		\
	res.a[2][1]=m1.a[2][0]*m2.a[0][1] + m1.a[2][1]*m2.a[1][1] + m1.a[2][2]*m2.a[2][1] + m1.a[2][3]*m2.a[3][1];		\
	res.a[3][1]=m1.a[3][0]*m2.a[0][1] + m1.a[3][1]*m2.a[1][1] + m1.a[3][2]*m2.a[2][1] + m1.a[3][3]*m2.a[3][1];		\
	res.a[0][2]=m1.a[0][0]*m2.a[0][2] + m1.a[0][1]*m2.a[1][2] + m1.a[0][2]*m2.a[2][2] + m1.a[0][3]*m2.a[3][2];		\
	res.a[1][2]=m1.a[1][0]*m2.a[0][2] + m1.a[1][1]*m2.a[1][2] + m1.a[1][2]*m2.a[2][2] + m1.a[1][3]*m2.a[3][2];		\
	res.a[2][2]=m1.a[2][0]*m2.a[0][2] + m1.a[2][1]*m2.a[1][2] + m1.a[2][2]*m2.a[2][2] + m1.a[2][3]*m2.a[3][2];		\
	res.a[3][2]=m1.a[3][0]*m2.a[0][2] + m1.a[3][1]*m2.a[1][2] + m1.a[3][2]*m2.a[2][2] + m1.a[3][3]*m2.a[3][2];		\
	res.a[0][3]=m1.a[0][0]*m2.a[0][3] + m1.a[0][1]*m2.a[1][3] + m1.a[0][2]*m2.a[2][3] + m1.a[0][3]*m2.a[3][3];		\
	res.a[1][3]=m1.a[1][0]*m2.a[0][3] + m1.a[1][1]*m2.a[1][3] + m1.a[1][2]*m2.a[2][3] + m1.a[1][3]*m2.a[3][3];		\
	res.a[2][3]=m1.a[2][0]*m2.a[0][3] + m1.a[2][1]*m2.a[1][3] + m1.a[2][2]*m2.a[2][3] + m1.a[2][3]*m2.a[3][3];		\
	res.a[3][3]=m1.a[3][0]*m2.a[0][3] + m1.a[3][1]*m2.a[1][3] + m1.a[3][2]*m2.a[2][3] + m1.a[3][3]*m2.a[3][3];		\
}																													\


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
