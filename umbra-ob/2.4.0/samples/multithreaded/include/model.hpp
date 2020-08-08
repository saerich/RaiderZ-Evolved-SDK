#ifndef __MODEL_HPP
#define __MODEL_HPP

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

// Platfromlib includes.
#include "plMesh.hpp"

// Umbra includes.
#include "umbraModel.hpp"

namespace MultithreadedSample
{

class Renderer;

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Application specific model class.
 *
 *			This class is application's representation of renderable
 *			geometry. The class consists of Umbra::Model, which is
 *			a representation of a mesh for Occlusion Booster, and 
 *			Platform::Mesh, which is platform level model 
 *			representation.
 *
 *			Model of the multithreaded sample is identical to
 *          Model of the basic sample.
 *
 *			An Umbra::Model is required for presenting the geometry for
 *			Umbra Occlusion Booster. It's a representation of the 
 *			object's bounds. An Umbra::Model is also required for 
 *			creating an Umbra::Object.
 *
 *			This class represents 3D geometry, whereas Object
 *			represets an instance of the model in space. 
 *			
 *//*-------------------------------------------------------------------*/

class Model
{
public:

                        Model           (Platform::Vector3* vertices,
                                         Platform::Vector3* normals, 
                                         Platform::Vector3i* triangles,
                                         int vertexCount,
                                         int triangleCount, 
                                         Renderer* renderer);
                        ~Model          (void);

	// Getters
    Umbra::Model*       getUmbraModel   (void)  { return m_umbraModel; }
    Platform::Mesh*     getPlatformMesh (void)  { return m_platformMesh; }

private:

    Umbra::Model*       m_umbraModel;       /*!< Umbra::Model, "shape" of the model for Occlusion Booster. */
    Platform::Mesh*     m_platformMesh;     /*!< Platform specific mesh class, which can be rendered. */
    Renderer*           m_renderer;         /*!< Pointer to the Renderer */

};

} // namespace MultithreadedSample

//------------------------------------------------------------------------
#endif // __MODEL_HPP