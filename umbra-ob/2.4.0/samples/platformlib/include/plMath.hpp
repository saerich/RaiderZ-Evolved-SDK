#ifndef __PLMATH_HPP
#define __PLMATH_HPP

/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
 *
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief     Math utilities.
 * 
 *//*-------------------------------------------------------------------*/

#include "platform.hpp"

#include <cmath>

#if defined(UMBRA_PLATFORM_MAC) || defined(UMBRA_PLATFORM_PS3)
#   define ALIGN_16
#else
#   define ALIGN_16 __declspec(align(16))
#endif

#undef UNALIGNED
#if defined(_WIN64)
#   define UNALIGNED __unaligned
#else
#   define UNALIGNED
#endif

namespace Platform
{

const float PI = 3.14159265358979323846f;

inline double plSqrtf (double d)
{
#if defined(UMBRA_PLATFORM_MAC)
	return sqrt(d);
#else
	return std::sqrt(d);
#endif
}

/*-------------------------------------------------------------------*//*!
 * \brief           A simple 2-vector class
 *//*-------------------------------------------------------------------*/

struct Vector2
{
    float	x, y;

    Vector2     ()                                  { zero(); }
    Vector2(float a, float b)		                { x = a; y = b; }

    float&			operator[]  (int n)		        { float* a = &x; return a[n]; }
    const float&	operator[]  (int n) const       { const float* a = &x; return a[n]; }

    void            operator+=  (const Vector2& v)	{ x += v.x; y += v.y;  }
    void            operator-=  (const Vector2& v)	{ x -= v.x; y -= v.y; }
    void            normalize   ()				    {
                                                        float invNorm = 1.0f / (float)plSqrtf(x*x + y*y);
                                                        x *= invNorm;
                                                        y *= invNorm;
                                                    }

    void zero()								{ x = 0.0f; y = 0.0f; }
};


/*-------------------------------------------------------------------*//*!
 * \brief           A simple 3-vector class
 *//*-------------------------------------------------------------------*/

struct Vector3
{
    float	x, y, z;

    Vector3     ()                                  { zero(); }
    Vector3(float a, float b, float c)		        { x = a; y = b; z = c; }
	Vector3(const Vector3 &v)						{ x = v.x; y = v.y; z = v.z; }

    float&			operator[]  (int n)		        { float* a = &x; return a[n]; }
    const float&	operator[]  (int n) const       { const float* a = &x; return a[n]; }

	void            operator=   (const Vector3& v)	{ x = v.x; y = v.y; z = v.z; }
    void            operator+=  (const Vector3& v)	{ x += v.x; y += v.y; z += v.z; }
    void            operator-=  (const Vector3& v)	{ x -= v.x; y -= v.y; z -= v.z; }
    void            normalize   ()				    {
                                                        float invNorm = 1.0f / (float)plSqrtf(x*x + y*y + z*z);
                                                        x *= invNorm;
                                                        y *= invNorm;
                                                        z *= invNorm;
                                                    }

    void zero()								{ x = 0.0f; y = 0.0f; z = 0.0f; }
};

/*-------------------------------------------------------------------*//*!
 * \brief	Calculates length of a 3-vector.
 *//*-------------------------------------------------------------------*/

inline float length(const Vector3& v)
{
    return (float)plSqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Calculates norm of a 3-vector, i.e. length^2.
 *//*-------------------------------------------------------------------*/

inline float norm(const Vector3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector scaling operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 operator*(const Vector3& v, float s)
{
    return Vector3(v.x * s, v.y * s, v.z * s);
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector scaling operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 operator*(float s, const Vector3& v)
{
    return v*s;
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector add operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector substract operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector negating operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 operator-(const Vector3& v)
{
    return -1.0f * v;
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector cross product operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 cross(const Vector3& a, const Vector3& b)
{
    return Vector3(a.y * b.z - a.z * b.y, 
                   a.z * b.x - a.x * b.z, 
                   a.x * b.y - a.y * b.x);
}

/*-------------------------------------------------------------------*//*!
 * \brief	3-Vector normalization operator.
 *//*-------------------------------------------------------------------*/

inline Vector3 normalize(const Vector3& v)
{
    float invNorm = 1.0f / (float)plSqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return invNorm * v;
}

/*-------------------------------------------------------------------*//*!
 * \brief           A simple 4-vector class
 *//*-------------------------------------------------------------------*/

struct Vector4
{
    float	x, y, z, w;

    Vector4						(void) 									{ x = 0; y = 0; z = 0; w = 0;  }
    Vector4						(float a, float b, float c, float d)	{ x = a; y = b; z = c; w = d; }
    float&			operator[]	(int n)									{ float* a = &x; return a[n]; }
    const float&	operator[]	(int n) const 							{ const float* a = &x; return a[n]; }
};

/*-------------------------------------------------------------------*//*!
 * \brief           Integer 3-vector
 *//*-------------------------------------------------------------------*/

struct Vector3i
{
    int		i, j, k;

    Vector3i					(void) 					{ i = 0; j = 0; k = 0; }
    Vector3i					(int a, int b, int c)	{ i = a; j = b; k = c; }
    int&			operator[]	(int n)					{ int* a = &i; return a[n]; }
    const int&		operator[]	(int n) const 			{ const int* a = &i; return a[n]; }
};

/*-------------------------------------------------------------------*//*!
 * \brief           A simple 4x4-matrix class 
 * \note            Column major-layout -- e.g. the same as DirectX
 *//*-------------------------------------------------------------------*/

class Matrix4x4
{
public:
    // empty constructor
    inline Matrix4x4(void) 
    {
        ident();
    }

    // reset to identity 
    inline void	ident(void)
    {
        m[0].x = 1.0f; m[0].y = 0.0f; m[0].z = 0.0f; m[0].w = 0.0f; 
        m[1].x = 0.0f; m[1].y = 1.0f; m[1].z = 0.0f; m[1].w = 0.0f; 
        m[2].x = 0.0f; m[2].y = 0.0f; m[2].z = 1.0f; m[2].w = 0.0f; 
        m[3].x = 0.0f; m[3].y = 0.0f; m[3].z = 0.0f; m[3].w = 1.0f; 
    }

    inline void setTranslation(const Vector3& t)
    {
        m[3].x = t.x;
        m[3].y = t.y;
        m[3].z = t.z;
    }

    Vector4&			operator[]		(int n)					{ return m[n]; }
    const Vector4&		operator[]		(int n) const			{ return m[n]; }

    void				setRight		(const Vector3& t)		{ m[0] = Vector4(t.x, t.y, t.z, 0.f); }
    void				setUp			(const Vector3& t)		{ m[1] = Vector4(t.x, t.y, t.z, 0.f); }
    void				setDof			(const Vector3& t)		{ m[2] = Vector4(t.x, t.y, t.z, 0.f); }

    Vector3				getRight		(void) const			{ return Vector3(m[0].x, m[0].y, m[0].z); }
    Vector3				getUp			(void) const			{ return Vector3(m[1].x, m[1].y, m[1].z); }
    Vector3				getDof			(void) const			{ return Vector3(m[2].x, m[2].y, m[2].z); }
    Vector3				getTranslation	(void) const			{ return Vector3(m[3].x, m[3].y, m[3].z); } 

    void				scale			(const Vector3& scale);
    void				invert			(const Matrix4x4& src);
    void				transpose       (const Matrix4x4& src);

	// Static functions for generating special matrices
    static void			getRotateX		(Matrix4x4& m, float angle); 
    static void			getRotateY		(Matrix4x4& m, float angle);
    static void			getRotateZ		(Matrix4x4& m, float angle);
    static void			getRotateAxis   (Matrix4x4& m, const Vector3 &axis, float angle);
    static void			getLookAt		(Matrix4x4& m, const Vector3& loc, const Vector3& dest, const Vector3& up); 

private:
    // matrix data
    Vector4	m[4];
};

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Matrix4x4 multiplication operator
 *
 * \param       s    source A
 * \param       n    source B
 *
 *//*-------------------------------------------------------------------*/

inline Matrix4x4 operator*(const Matrix4x4& s, const Matrix4x4& n) // \todo
{
    Matrix4x4 m;
    m[0][0] = n[0][0]*s[0][0] + n[1][0]*s[0][1] + n[2][0]*s[0][2] + n[3][0]*s[0][3];
    m[1][0] = n[0][0]*s[1][0] + n[1][0]*s[1][1] + n[2][0]*s[1][2] + n[3][0]*s[1][3];
    m[2][0] = n[0][0]*s[2][0] + n[1][0]*s[2][1] + n[2][0]*s[2][2] + n[3][0]*s[2][3];
    m[3][0] = n[0][0]*s[3][0] + n[1][0]*s[3][1] + n[2][0]*s[3][2] + n[3][0]*s[3][3];
    m[0][1] = n[0][1]*s[0][0] + n[1][1]*s[0][1] + n[2][1]*s[0][2] + n[3][1]*s[0][3];
    m[1][1] = n[0][1]*s[1][0] + n[1][1]*s[1][1] + n[2][1]*s[1][2] + n[3][1]*s[1][3];
    m[2][1] = n[0][1]*s[2][0] + n[1][1]*s[2][1] + n[2][1]*s[2][2] + n[3][1]*s[2][3];
    m[3][1] = n[0][1]*s[3][0] + n[1][1]*s[3][1] + n[2][1]*s[3][2] + n[3][1]*s[3][3];
    m[0][2] = n[0][2]*s[0][0] + n[1][2]*s[0][1] + n[2][2]*s[0][2] + n[3][2]*s[0][3];
    m[1][2] = n[0][2]*s[1][0] + n[1][2]*s[1][1] + n[2][2]*s[1][2] + n[3][2]*s[1][3];
    m[2][2] = n[0][2]*s[2][0] + n[1][2]*s[2][1] + n[2][2]*s[2][2] + n[3][2]*s[2][3];
    m[3][2] = n[0][2]*s[3][0] + n[1][2]*s[3][1] + n[2][2]*s[3][2] + n[3][2]*s[3][3];
    m[0][3] = n[0][3]*s[0][0] + n[1][3]*s[0][1] + n[2][3]*s[0][2] + n[3][3]*s[0][3];
    m[1][3] = n[0][3]*s[1][0] + n[1][3]*s[1][1] + n[2][3]*s[1][2] + n[3][3]*s[1][3];
    m[2][3] = n[0][3]*s[2][0] + n[1][3]*s[2][1] + n[2][3]*s[2][2] + n[3][3]*s[2][3];
    m[3][3] = n[0][3]*s[3][0] + n[1][3]*s[3][1] + n[2][3]*s[3][2] + n[3][3]*s[3][3];
    return m;
}

} // namespace

//------------------------------------------------------------------------
#endif // __PLMATH_HPP
