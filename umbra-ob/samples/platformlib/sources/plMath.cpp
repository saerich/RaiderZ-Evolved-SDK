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

#include "plMath.hpp"

using namespace Platform;

void Matrix4x4::scale(const Vector3& scale)
{
    m[0].x *= scale.x;
    m[1].y *= scale.y;
    m[2].z *= scale.z;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Calculates inverse of a 4x4 matrix
 *
 * \param       src    source matrix
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::invert(const Matrix4x4& src)
{
    const float a4 = src[3][0];
    const float b4 = src[3][1];
    const float c4 = src[3][2];
    const float d4 = src[3][3];

    const float a1 = src[0][0];
    const float b1 = src[0][1];
    const float c1 = src[0][2];
    const float d1 = src[0][3];
    const float a2 = src[1][0];
    const float b2 = src[1][1];
    const float c2 = src[1][2];
    const float d2 = src[1][3];
    const float a3 = src[2][0];
    const float b3 = src[2][1];
    const float c3 = src[2][2];
    const float d3 = src[2][3];

    float a3b4_a4b3 = a3 * b4 - a4 * b3;
    float a3c4_a4c3 = a3 * c4 - a4 * c3;
    float a3d4_a4d3 = a3 * d4 - a4 * d3;
    float b3c4_b4c3 = b3 * c4 - b4 * c3;
    float b3d4_b4d3 = b3 * d4 - b4 * d3;
    float c3d4_c4d3 = c3 * d4 - c4 * d3;

    m[0][0]		= (b2 * c3d4_c4d3 - c2 * b3d4_b4d3 + d2 * b3c4_b4c3);
    m[1][0]		=-(a2 * c3d4_c4d3 - c2 * a3d4_a4d3 + d2 * a3c4_a4c3);
    m[2][0]		= (a2 * b3d4_b4d3 - b2 * a3d4_a4d3 + d2 * a3b4_a4b3);
    m[3][0]		=-(a2 * b3c4_b4c3 - b2 * a3c4_a4c3 + c2 * a3b4_a4b3);
    m[0][1]		=-(b1 * c3d4_c4d3 - c1 * b3d4_b4d3 + d1 * b3c4_b4c3);
    m[1][1]		= (a1 * c3d4_c4d3 - c1 * a3d4_a4d3 + d1 * a3c4_a4c3);
    m[2][1]		=-(a1 * b3d4_b4d3 - b1 * a3d4_a4d3 + d1 * a3b4_a4b3);
    m[3][1]		= (a1 * b3c4_b4c3 - b1 * a3c4_a4c3 + c1 * a3b4_a4b3);

    float a2d4_a4d2 = a2 * d4 - a4 * d2;
    float a2b4_a4b2 = a2 * b4 - a4 * b2;
    float a2c4_a4c2 = a2 * c4 - a4 * c2;
    float b2c4_b4c2 = b2 * c4 - b4 * c2;
    float b2d4_b4d2 = b2 * d4 - b4 * d2;
    float c2d4_c4d2 = c2 * d4 - c4 * d2;

    m[0][2]		= (b1 * c2d4_c4d2 - c1 * b2d4_b4d2 + d1 * b2c4_b4c2);
    m[1][2]		=-(a1 * c2d4_c4d2 - c1 * a2d4_a4d2 + d1 * a2c4_a4c2);
    m[2][2]		= (a1 * b2d4_b4d2 - b1 * a2d4_a4d2 + d1 * a2b4_a4b2);
    m[3][2]		=-(a1 * b2c4_b4c2 - b1 * a2c4_a4c2 + c1 * a2b4_a4b2);

    float a2b3_a3b2 = a2 * b3 - a3 * b2;
    float a2c3_a3c2 = a2 * c3 - a3 * c2;
    float a2d3_a3d2 = a2 * d3 - a3 * d2;
    float b2c3_b3c2 = b2 * c3 - b3 * c2;
    float b2d3_b3d2 = b2 * d3 - b3 * d2;
    float c2d3_c3d2 = c2 * d3 - c3 * d2;

    m[0][3]		=-(b1 * c2d3_c3d2 - c1 * b2d3_b3d2 + d1 * b2c3_b3c2);
    m[1][3]		= (a1 * c2d3_c3d2 - c1 * a2d3_a3d2 + d1 * a2c3_a3c2);
    m[2][3]		=-(a1 * b2d3_b3d2 - b1 * a2d3_a3d2 + d1 * a2b3_a3b2);
    m[3][3]		= (a1 * b2c3_b3c2 - b1 * a2c3_a3c2 + c1 * a2b3_a3b2);

    float  det		= a1 * m[0][0] + b1 * m[1][0] + c1 * m[2][0] + d1 * m[3][0];

    if (det != 1.0f)
    {
        det = 1.0f/det;
        m[0][0] = m[0][0]*det;
        m[0][1] = m[0][1]*det;
        m[0][2] = m[0][2]*det;
        m[0][3] = m[0][3]*det;
        m[1][0] = m[1][0]*det;
        m[1][1] = m[1][1]*det;
        m[1][2] = m[1][2]*det;
        m[1][3] = m[1][3]*det;
        m[2][0] = m[2][0]*det;
        m[2][1] = m[2][1]*det;
        m[2][2] = m[2][2]*det;
        m[2][3] = m[2][3]*det;
        m[3][0] = m[3][0]*det;
        m[3][1] = m[3][1]*det;
        m[3][2] = m[3][2]*det;
        m[3][3] = m[3][3]*det;
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Transposes a 4x4 Matrix
 *
 * \param       src    source matrix
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::transpose(const Matrix4x4& src)
{
    m[0].x = src[0].x;
    m[0].y = src[1].x;
    m[0].z = src[2].x;
    m[0].w = src[3].x;

    m[1].x = src[0].y;
    m[1].y = src[1].y;
    m[1].z = src[2].y;
    m[1].w = src[3].y;

    m[2].x = src[0].z;
    m[2].y = src[1].z;
    m[2].z = src[2].z;
    m[2].w = src[3].z;

    m[3].x = src[0].w;
    m[3].y = src[1].w;
    m[3].z = src[2].w;
    m[3].w = src[3].w;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Get a X-rotation matrix
 *
 * \param       angle    rotation angle
 *
 * \return      A matrix that can be used to rotate around X-axis
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::getRotateX(Matrix4x4& m, float angle)
{
    float s = (float)std::sin(angle);
    float c = (float)std::cos(angle);
    float o = 1.f;
    float z = 0.f;

    m.ident();
    m.setRight	(Vector3(o, z, z));
    m.setUp		(Vector3(z, c,-s));
    m.setDof	(Vector3(z, s, c));
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Get an Y-rotation matrix
 *
 * \param       angle    rotation angle
 *
 * \return      A matrix that can be used to rotate around Y-axis
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::getRotateY(Matrix4x4& m, float angle)
{
    float s = (float)std::sin(angle);
    float c = (float)std::cos(angle);
    float o = 1.f;
    float z = 0.f;

    m.ident();
    m.setRight	(Vector3(c, z, s));
    m.setUp		(Vector3(z, o, z));
    m.setDof	(Vector3(-s,z, c));
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Get an Z-rotation matrix
 *
 * \param       angle    rotation angle
 *
 * \return      A matrix that can be used to rotate around Z-axis
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::getRotateZ(Matrix4x4& m, float angle)
{
    float s = (float)std::sin(angle);
    float c = (float)std::cos(angle);
    float o = 1.f;
    float z = 0.f;

    m.ident();
    m.setRight	(Vector3(c, -s, z));
    m.setUp		(Vector3(s, c, z));
    m.setDof	(Vector3(z ,z, o));
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       Get a rotation matrix about a custom axis.
 *
 * \param       axis     rotation axis
 * \param       angle    rotation angle
 *
 * \return      A matrix that can be used for rotation 
 *              around a custom axis
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::getRotateAxis(Matrix4x4& m, const Vector3 &axis, float angle)
{
    m.ident();

    float s = (float)std::sin(angle);
    float c = (float)std::cos(angle);

	m[0][0] =           c + axis.x*axis.x*(1-c);
	m[0][1] =  axis.z * s + axis.y*axis.x*(1-c);
	m[0][2] = -axis.y * s + axis.z*axis.x*(1-c);
	m[1][0] = -axis.z * s + axis.x*axis.y*(1-c);
	m[1][1] =           c + axis.y*axis.y*(1-c);
	m[1][2] =  axis.x * s + axis.z*axis.y*(1-c);
	m[2][0] =  axis.y * s + axis.x*axis.z*(1-c);
	m[2][1] = -axis.x * s + axis.y*axis.z*(1-c);
	m[2][2] =           c + axis.z*axis.z*(1-c);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief       lookAt functions
 *
 * \param       m      Destination matrix
 * \param       loc    Location of the camera
 * \param       dest   Destination point
 * \param       inUp   Un-normalized up vector
 *
 * \return      A matrix that can be used to set the view matrix
 *
 *//*-------------------------------------------------------------------*/

void Matrix4x4::getLookAt(Matrix4x4& m, const Vector3 &loc, const Vector3 &dest, const Vector3 &inUp)
{
    const Vector3 up = normalize(inUp); 
    const Vector3 dof = normalize(dest - loc); 
    const Vector3 right = cross(up, dof); 

    m.ident(); 
    m.setUp(up); 
    m.setRight(right); 
    m.setDof(dof); 
    m.setTranslation(loc); 
}

//------------------------------------------------------------------------

