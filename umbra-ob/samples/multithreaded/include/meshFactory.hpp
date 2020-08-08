#ifndef __MESHFACTORY_HPP
#define __MESHFACTORY_HPP

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

// Platformlib includes.
#include "platform.hpp"

namespace MultithreadedSample
{

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Utility class for creating vertex, index and normal
 *			data for different types of meshes.
 *
 *			MeshFactory in the multithreaded sample is identical to
 *          MeshFactory in the basic sample.
 *
 *//*-------------------------------------------------------------------*/

class MeshFactory
{

public:

	// Generates mesh data for a torus
    static void createTorus     (int r, 
                                 int p, 
                                 float radius, 
                                 float thickness,
                                 Platform::Vector3*& vertices,
                                 Platform::Vector3*& normals,
                                 Platform::Vector3i*& triangles,
                                 int& vertexCount,
                                 int& triangleCount);

	// Generates mesh data for a sphere
    static void createGeoSphere (int level, 
                                 float radius, 
                                 Platform::Vector3*& vertices, 
                                 Platform::Vector3*& normals, 
                                 Platform::Vector3i*& triangles,
                                 int& vertexCount,
                                 int& triangleCount);

private:

	MeshFactory();

}; // class MeshFactory

} // namespace MultithreadedSample

//------------------------------------------------------------------------
#endif // __MESHFACTORY_HPP
