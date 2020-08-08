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
 * \brief     Application model class.
 * 
 *//*-------------------------------------------------------------------*/

// Platform includes
#include "platform.hpp"

// Application includes
#include "model.hpp"

// System includes
#include <assert.h>

using namespace Platform;
using namespace MultithreadedSample;

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Model constructor
 *
 *          The model is constructed from vertex and index data.
 *
 * \param   vertices        Vertex data of the model.
 * \param   normals         Normal data of the model.
 * \param   triangles       Triangle data of the model, i.e. indices
 *							to vertex and normal data.
 * \param   vertexCont      Number of vertices in the vertex array.
 * \param   triangleCount   Number of triangles.
 *
 *//*-------------------------------------------------------------------*/

Model::Model(Vector3* vertices, Vector3* normals, Vector3i* triangles, int vertexCount, int triangleCount) :
m_umbraModel(NULL),
m_platformMesh(NULL)
{
	assert(vertices && normals && triangles && vertexCount > 0 && triangleCount > 0);

	// Create platform mesh from the raw model data.
	// This is platform specific representation of a mesh, which can be rendered.
    m_platformMesh  = Platform::createPlatformMesh(vertices, normals, triangles, vertexCount, triangleCount);

	// Create Umbra::Model for Occlusion Booster. An Umbra::Model represents the geometry 
	// (or rather it's bounds) for Umbra Occlusion Booster.
	//
	// An Umbra::Model doesn't have a position/transformation in space. Instead, an 
	// Umbra::Object represents a model instance for Occlusion Booster, and has a position. 
	// Creating an Umbra::Object requires having a corresponding Umbra::Model. A single 
	// Umbra::Model can be shared between several Umbra::Objects.
	//
	// In this case the model is created from vertex data. Occlusion booster will 
	// automatically enumerate the vertices and construct a bounding box accordingly. In 
	// case there is a good, tight bounding box already available, it can be used directly. 
	// See Umbra::OBBModel.
	// 
    m_umbraModel    = Umbra::OBBModel::create((Umbra::Vector3*)vertices, vertexCount);

    assert(m_umbraModel);
    assert(m_platformMesh);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Model destructor
 *
 *          Releases all resources allocated and stored by the model
 *          class.
 *
 *//*-------------------------------------------------------------------*/

Model::~Model(void)
{
    Platform::releasePlatformMesh(m_platformMesh);
    m_umbraModel->release();

	m_platformMesh = NULL;
	m_umbraModel   = NULL;
}
