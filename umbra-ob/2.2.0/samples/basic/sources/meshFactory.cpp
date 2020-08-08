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
 * \brief     Mesh creation utility functions.
 * 
 *//*-------------------------------------------------------------------*/

// Application includes.
#include "meshFactory.hpp"
#include "plMath.hpp"

using namespace Platform;
using namespace BasicSample;

namespace BasicSample
{

/*-------------------------------------------------------------------*//*!
 *                                                                      
 * \brief   Subdivide sphere.
 *
 *          Used by MeshFactory::createGeoSphere.
 *
 *//*-------------------------------------------------------------------*/

static int subdivideGeoSphere(const Vector3& a, const Vector3& b, const Vector3& c, int sublevel, Vector3* vertices, int vi)
{
	if (sublevel == 0)
	{
		vertices[vi++] = a; 
		vertices[vi++] = b;  
		vertices[vi++] = c; 
		return vi; 
	}
	else
	{
		Vector3 ab = normalize(0.5f * (a + b));
		Vector3 ac = normalize(0.5f * (a + c));
		Vector3 bc = normalize(0.5f * (b + c));

		int vin;
		vin = subdivideGeoSphere(a, ab, ac, sublevel - 1, vertices, vi);
		vin = subdivideGeoSphere(ab, bc, ac, sublevel - 1, vertices, vin);
		vin = subdivideGeoSphere(ab, b, bc, sublevel - 1, vertices, vin);
		vin = subdivideGeoSphere(ac, bc, c, sublevel - 1, vertices, vin);
        return vin;
	}
}

} // namespace BasicSample
 
/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Creates mesh data for a torus shape.
 * 
 * \param   r           Mesh resolution value
 *
 * \param   p           Mesh resolution value
 *
 * \param   radius      The radius of the torus to be created.
 *
 * \param   thickness   The thickness of the torus to be creatd.
 *
 * \param   vertices    The function returns the allocated vertex
 *                      vertex data as a Vector3 array in this pointer 
 *                      reference.
 *
 * \param   normals     The function returns the allocated normal
 *                      data as a Vector3 array in this pointer 
 *                      reference.
 *
 * \param   triangles   The function returns the allocated index 
 *                      data as a Vector3i array in this pointer 
 *                      reference.
 *
 * \param   vertexCount The function returns the size of the vertex
 *                      and normal arrays in this reference.
 *
 * \param triangleCount The function returns the size of the index
 *                      array in this pointer reference.
 *
 *//*-------------------------------------------------------------------*/

void MeshFactory::createTorus(int r, 
                              int p, 
                              float radius, 
                              float thickness,
                              Vector3*& vertices,
                              Vector3*& normals,
                              Vector3i*& triangles,
                              int& vertexCount,
                              int& triangleCount)
{
	int*			pIndices;
	int				nIndices;

	struct Primitive
	{
		int			nIndices;
		int			indexBufferOffset;
		int			indexOffset;
		int			indexRange;
	};

	int			nPrimitives;
	Primitive*	pPrimitives;

	int i, j;

	vertices    = new Vector3[(r+1)*p];
	normals     = new Vector3[(r+1)*p];
	vertexCount = (r+1)*p;

	const float pi  = 3.141592f;
	double rstep    = 1.f/(float)r*2.f*pi;
	double rca      = std::cos(rstep);
	double rsa      = std::sin(rstep);

	float oy = thickness/2;
	float oz = 0.f;

	for(j=0; j<r+1; j++)
	{
		float x = 0.f;
		float y = oy + radius;
		float z = oz;

		float nx = 0.f;
		float ny = oy;
		float nz = oz;

		float pstep = 1.f/float(p) * float(pi)*2.0f;
		double pca = std::cos(pstep);
		double psa = std::sin(pstep);

		for(i=0; i<p; i++)
		{
			vertices[j*p + i].x = x;
			vertices[j*p + i].y = y;
			vertices[j*p + i].z = z;

			float bnx = nx;
			float bny = ny;
			float bnz = nz;

			double len = 1.0/std::sqrt(bnx*bnx+bny*bny+bnz*bnz);

			normals[j*p + i].x = float(bnx*len);
			normals[j*p + i].y = float(bny*len);
			normals[j*p + i].z = float(bnz*len);

			float tx = float(x * pca + y *-psa);
			float ty = float(x * psa + y * pca);
			x = tx;
			y = ty;

			tx = float(nx * pca + ny *-psa);
			ty = float(nx * psa + ny * pca);
			nx = tx;
			ny = ty;
		}

		float ty = float(oy * rca + oz *-rsa);
		float tz = float(oy * rsa + oz * rca);
		oy = ty;
		oz = tz;
	}

	nPrimitives = r;
	pPrimitives = new Primitive[r];

	nIndices = (p+1)*2;
	pIndices = new int[nIndices];

	int* pInd = pIndices;

	for(j=0; j<r; j++)
	{
		pPrimitives[j].nIndices = (p+1)*2;
		pPrimitives[j].indexBufferOffset = 0;
		int indexOffset = j*p;

		pPrimitives[j].indexOffset = indexOffset;
		pPrimitives[j].indexRange = (j+1)*p+p;
		pPrimitives[j].indexRange -= indexOffset;

		if(j == 0)
		{
			for(i=0; i<p; i++)
			{
				*pInd++ = j*p + i - indexOffset;
				*pInd++ = (j+1)*p + i - indexOffset;
			}
			*pInd++ = j*p - indexOffset;
			*pInd++ = (j+1)*p - indexOffset;
		}
	}

	triangles = new Vector3i[r*p*2];

	for(int prim=0; prim<nPrimitives; prim++)
	{
		Primitive& p = pPrimitives[prim];

		for(int triangle=0; triangle<p.nIndices-2; triangle++)
		{
			if(triangle & 1)
			{
				triangles[triangleCount] = Vector3i(
					pIndices[p.indexBufferOffset + triangle + 0] + p.indexOffset,
					pIndices[p.indexBufferOffset + triangle + 1] + p.indexOffset,
					pIndices[p.indexBufferOffset + triangle + 2] + p.indexOffset
				);
			}
			else
			{
				triangles[triangleCount] = Vector3i(
					pIndices[p.indexBufferOffset + triangle + 0] + p.indexOffset,
					pIndices[p.indexBufferOffset + triangle + 2] + p.indexOffset,
					pIndices[p.indexBufferOffset + triangle + 1] + p.indexOffset
				);
			}
			triangleCount++;
		}
	}

	delete[] pPrimitives;
}

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Creates mesh data for a sphere shape.
 * 
 * \param   level       The level of subdivision.
 *
 * \param   radius      The radius of the sphere to be created.
 *
 * \param   vertices    The function returns the allocated vertex
 *                      vertex data as a Vector3 array in this pointer 
 *                      reference.
 *
 * \param   normals     The function returns the allocated normal
 *                      data as a Vector3 array in this pointer 
 *                      reference.
 *
 * \param   triangles   The function returns the allocated index 
 *                      data as a Vector3i array in this pointer 
 *                      reference.
 *
 * \param  vertexCount  The function returns the size of the vertex
 *                      and normal arrays in this reference.
 *
 * \param triangleCount The function returns the size of the index
 *                      array in this pointer reference.
 *
 *//*-------------------------------------------------------------------*/

void MeshFactory::createGeoSphere(int level, 
                                  float radius, 
                                  Vector3*& vertices, 
                                  Vector3*& normals, 
                                  Vector3i*& triangles,
                                  int& vertexCount,
                                  int& triangleCount)
{
	const float t = 0.5f * (1 + (float)plSqrtf(5));
	const float s = 1.0f / (float)plSqrtf(1 + t*t);

	const Vector3 V[] = 
	{
		s * Vector3(t,1,0),
		s * Vector3(-t,1,0),
		s * Vector3(t,-1,0),
		s * Vector3(-t,-1,0),
		s * Vector3(1,0,t),
		s * Vector3(1,0,-t),
		s * Vector3(-1,0,t),
		s * Vector3(-1,0,-t),
		s * Vector3(0,t,1),
		s * Vector3(0,-t,1),
		s * Vector3(0,t,-1),
		s * Vector3(0,-t,-1)
	};

	const Vector3i T[] = 
	{
		Vector3i(0,8,4),
		Vector3i(1,10,7),
		Vector3i(2,9,11),
		Vector3i(7,3,1),
		Vector3i(0,5,10),
		Vector3i(3,9,6),
		Vector3i(3,11,9),
		Vector3i(8,6,4),
		Vector3i(2,4,9),
		Vector3i(3,7,11),
		Vector3i(4,2,0),
		Vector3i(9,4,6),
		Vector3i(2,11,5),
		Vector3i(0,10,8),
		Vector3i(5,0,2),
		Vector3i(10,5,7),
		Vector3i(1,6,8),
		Vector3i(1,8,10),
		Vector3i(6,1,3),
		Vector3i(11,7,5)
	};

	triangleCount   = 20 * ((level > 0) ? (2 << (2*level)) : 1); 
	vertexCount     = triangleCount * 3; 
	vertices        = new Vector3[vertexCount]; 
	normals         = new Vector3[vertexCount]; 
	triangles       = new Vector3i[triangleCount];

	int vi = 0; 
	for (int i = 0; i < 20; ++i)
        vi = subdivideGeoSphere(V[T[i].i], V[T[i].j], V[T[i].k], level, vertices, vi);

	for (int i = 0; i < vertexCount; ++i)
	{
		const Vector3 vertex = vertices[i]; 
		normals[i] = vertex;
		vertices[i] = radius * vertex; 
	}

	for (int i = 0; i < triangleCount; ++i)
		triangles[i] = Vector3i(3*i, 3*i + 1, 3*i + 2); 
}
