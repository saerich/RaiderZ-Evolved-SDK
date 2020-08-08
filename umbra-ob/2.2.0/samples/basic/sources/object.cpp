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
 * \brief     Application object class.
 * 
 *//*-------------------------------------------------------------------*/

// Platformlib includes.
#include "plObject.hpp"
#include "platform.hpp"

// Application includes.
#include "object.hpp"
#include "model.hpp"

// Umbra includes.
#include "umbraObject.hpp"
#include "umbraModel.hpp"

// System includes.
#include <assert.h>

using Platform::Vector4;
using Platform::Matrix4x4;
using namespace BasicSample;

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Object constructor
 *
 *			The object is constructed from a model. The constructor 
 *			creates platform and Umbra objects, setting an user pointer
 *			for Umbra Object.
 *
 * \param   model   Pointer to the Model of which this Object is an 
 *					instance.
 *
 *//*-------------------------------------------------------------------*/

Object::Object(Model* model) :
m_umbraObject(NULL),
m_platformObject(NULL)
{
    assert(model);

    // Create a platform object.
	// The platform object is created from application model's Platform::Mesh.
    m_platformObject  = new Platform::Object(model->getPlatformMesh());
    assert(m_platformObject);

    // Set color for the platform object.
    m_platformObject->setColor(Vector4(0.3f, 0.5f, 0.7f, 1.0f));

    // Create an Umbra::Object.
	// Creating an Umbra::Object requires an Umbra::Model instance,
	// which is here obtained from the (application) Model.
    m_umbraObject	= Umbra::Object::create(model->getUmbraModel());
	assert(m_umbraObject);

    // Set user pointer to the renderer object.
	// Setting the user pointer allows the integration code to obtain
	// the related object class in Commander callbacks
    m_umbraObject->setUserPointer(m_platformObject);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Object destructor.
 *
 *          Release Umbra::Object and Umbra::Model. Delete 
 *          Platform::Object.
 *
 *//*-------------------------------------------------------------------*/

Object::~Object()
{
    delete m_platformObject;
	m_umbraObject->release();

	m_platformObject = 0;
	m_umbraObject = 0;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Sets the object into the given Umbra::Cell.
 *
 * \param   c   Pointer to the Umbra::Cell to be set for the 
 *              Umbra::Object member.
 *
 *          Setting an object to a cell adds the object in question 
 *          Umbra Occlusion Booster's internal representation of the
 *          3D scene. Objects that have not been set into cells
 *          are not processed during the visibility query.
 *
 *//*-------------------------------------------------------------------*/

void Object::setCell(Umbra::Cell* cell)
{
	assert(cell);
	m_umbraObject->setCell(cell);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Set the object's cell-transformation matrix.
 *
 * \param   mtx     Transformation matrix.
 *
 *          The cell-transformation matrix of the Umbra::Object
 *          is used to determine the object's position in Umbra Occlusion 
 *          Boosters internal renresentation of the 3D scene. The same
 *          matrix is used when rendering the object.
 *
 *//*-------------------------------------------------------------------*/

void Object::setObjectToCellMatrix(const Matrix4x4& mtx)
{
	assert(m_umbraObject && m_platformObject);

	// Update local copy of the matrix
    m_objectToCell = mtx;
	
	// Update the matrix to Umbra::Object and Platform::Object
	m_umbraObject->setObjectToCellMatrix((const Umbra::Matrix4x4&)mtx);
    m_platformObject->setObjectToCellMatrix(mtx);
}
