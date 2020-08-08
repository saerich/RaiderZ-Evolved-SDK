#ifndef __OBJECT_HPP
#define __OBJECT_HPP

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

// Platfromlib includes.
#include "plMesh.hpp"
#include "plMath.hpp"

// Application includes
#include "model.hpp"

// Umbra includes.
#include "umbraObject.hpp"

namespace BasicSample
{

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Application specific object class.
 *
 *			This class represents an object, which is an instance of a
 *			model located in some position in space. This is 
 *			application's view of an object. 
 *
 *			Internally, this class consists of Umbra::Object, which is
 *			Occlusion Booster's object representation, and
 *			Platform::Object, which is the platform level object class.
 *
 *			Objects are integrated to Umbra Occlusion Booster by
 *			maintaining a corresponding Umbra::Object instance. The 
 *			Umbra::Object needs to be kept up-to-date on the object's 
 *			transformation.
 *
 *//*-------------------------------------------------------------------*/

class Object
{
public:
								Object                  (Model* model);
								~Object                 (void);

	// Set object's cell.
    void						setCell                 (Umbra::Cell*);

	// Set and get object's transformation matrix
    void						setObjectToCellMatrix   (const Platform::Matrix4x4&);
    const Platform::Matrix4x4&	getObjectToCellMatrix   (void) const            { return m_objectToCell; } 

	// Getters for Umbra and platform objects
    Platform::Object*			getObject               (void)                  { return m_platformObject;  }
    Umbra::Object*				getUmbraObject          (void)                  { return m_umbraObject; }

private:

	Platform::Matrix4x4			m_objectToCell;     /*!< Object's transformation matrix */

	Umbra::Object*				m_umbraObject;      /*!< Pointer to a Umbra::Object */
    Platform::Object*			m_platformObject;   /*!< Pointer to a platform specific renderable object */	
};

} // namespace BasicSample

//------------------------------------------------------------------------
#endif // __OBJECT_HPP
