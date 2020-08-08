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
#include "renderer.hpp"

// Umbra includes.
#include "umbraObject.hpp"
#include "umbraModel.hpp"

// System includes.
#include <assert.h>

using namespace MultithreadedSample;

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
 * \param	renderer  	Pointer to the renderer, which is used to 
 *					  	initialize the object.
 *
 *//*-------------------------------------------------------------------*/

Object::Object(Model* model, Renderer* renderer) :
m_umbraObject(NULL),
m_rendererObject(NULL),
m_renderer(renderer)
{
    assert(model);
	assert(m_renderer);

    // Create a renderable object. Here RendererObject is used as a wrapper
	// for Platform::Object to prevent manipulation outside renderer.
    m_rendererObject = m_renderer->createObject(model->getPlatformMesh());
    assert(m_rendererObject);

    // Set color for the renderable object.
    m_renderer->setObjectColor(m_rendererObject, Platform::Vector4(0.3f, 0.5f, 0.7f, 1.0f));

    // Create an Umbra::Object.
	// Creating an Umbra::Object requires an Umbra::Model instance,
	// which is here obtained from the (application) Model.
    m_umbraObject	= Umbra::Object::create(model->getUmbraModel());
	assert(m_umbraObject);

    // Set user pointer to the renderer object.
	// Setting the user pointer allows the integration code to obtain
	// the related object class in e.g. commander, when only 
	// Umbra::Object is provided.
    m_umbraObject->setUserPointer(m_rendererObject);
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
    m_renderer->deleteObject(m_rendererObject);
    m_umbraObject->setCell(NULL);
	m_umbraObject->release();

	m_rendererObject = 0;
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

void Object::setObjectToCellMatrix(const Platform::Matrix4x4& mtx)
{
	assert(m_umbraObject && m_rendererObject);

	// Update local copy of the matrix
    m_objectToCell = mtx;
	
	// Update the matrix to Umbra::Object and Platform::Object
	m_umbraObject->setObjectToCellMatrix((const Umbra::Matrix4x4&)mtx);
	
	// Update object's matrix using the renderer
    m_renderer->setObjectMatrix(m_rendererObject, mtx);
}
