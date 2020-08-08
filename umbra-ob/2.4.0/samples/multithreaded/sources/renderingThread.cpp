/*-------------------------------------------------------------------*//*!
 *
 * Umbra
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
 * \brief     A simple Application using Umbra
 * 
 *//*-------------------------------------------------------------------*/

#include <assert.h>

#include "renderer.hpp"
#include "platform.hpp"
#include "commander.hpp"
#include "graphicsServices.hpp"
#include "plObject.hpp"

using namespace Platform;

namespace MultithreadedSample
{

/*-------------------------------------------------------------------*//*!
 * \brief   Rendering thread.
 *//*-------------------------------------------------------------------*/

void Renderer::renderLoop(DataStream* stream)
{
    bool exitThread = false;
    RenderCommand cmd;

	// Create DataStreamReader, which is used to read commands
	// from the stream
    DataStreamReader reader;
    reader.init(stream);

	Platform::Semaphore* semaphore;
    Umbra::Commander*	 commander;

	// Get synchronization semaphore and commander from the stream
    reader.get(semaphore);
    reader.get(commander);

    for (;;)
    {
		unsigned int commandValue = 0;

		// Read a command from the stream
		reader.get(commandValue);
		cmd = (RenderCommand)commandValue;
        
        {
            switch (cmd)
            {
            
			//------------------------------------------------------------
			// Set object's transformation matrix
			//------------------------------------------------------------
            case CMD_SET_OBJECT_MATRIX:
                {
                    // Get object from command queue.

                    Platform::Object* object = NULL;

                    reader.get((Platform::Object*&)object);

                    // Get object matrix from command queue.

                    Matrix4x4 mtx;
                    reader.get(mtx);
                    
					object->setObjectToCellMatrix(mtx);
					
                    break;
                }

			//------------------------------------------------------------
			// Set object's color
			//------------------------------------------------------------
            case CMD_SET_OBJECT_COLOR:
                {
                    // Get object from command queue.

                    Platform::Object* object = NULL;
                    Vector4 color;

                    reader.get((Platform::Object*&)object);
                    reader.get(color);
                   
                    object->setColor(color);

                    break;
                }

			//------------------------------------------------------------
			// Delete an object
			//------------------------------------------------------------
            case CMD_DELETE_OBJECT:
                {
                    // Get object from command queue.

                    Platform::Object* object = NULL;

                    reader.get((Platform::Object*&)object);
                   
                    delete object;

                    break;
                }

			//------------------------------------------------------------
			// Begins a frame.
			//------------------------------------------------------------
            case CMD_BEGIN_FRAME:
                {
					// Call platform, set viewport and clear.
                    Platform::beginFrame();
                    Platform::setViewport(0, 0, 1280, 720);
                    Platform::clear(0.25f, 0.25f, 0.25f, 0.f, 1.0f, 0.f);
                    break;
                }
                    
			//------------------------------------------------------------
			// Ends a frame. 
			//------------------------------------------------------------
            case CMD_END_FRAME:
                {
					// Call platform.
                    Platform::endFrame();
                    break;
                }

			//------------------------------------------------------------
			// Render scene from camera's viewpoint.
			//
			// No need to handle objects here: Occlusion Booster will
			// report visible objects via the commander, where also 
			// actual rendering takes place.
			//
			// (Note that passing camera and viewport here using
			// application Camera class doesn't make sense, since
			// a) It leads to synchronization problems when deleting
			//    cameras. Only purpose in having CMD_DELETE_CAMERA 
			//	  in rendering thread in the first place is to avoid
			//	  these problems.
			// b) The rendering thread only handles Platform and Umbra
			//    level objects elsewhere)
			//------------------------------------------------------------
            case CMD_RENDER:
                {
					// Camera and viewport
					Umbra::Camera*	camera;
                    int				x, y, w, h;

					// Read Camera object
					reader.get((Umbra::Camera*&)camera);

					// Read camera's viewport
					reader.get(x);
					reader.get(y);
					reader.get(w);
					reader.get(h);

					// Activate camera's viewport.
                    Platform::setViewport(x, y, w, h);

                   	// Do rendering. Calling processVisibility will make
					// Occlusion Booster issue rendering callbacks to
					// commander.
                    camera->processVisibility(commander);

					// Increment semaphore 
                    Platform::incSemaphore(semaphore); 

                    break;
                }

			//------------------------------------------------------------
			// Exit rendering thread
			//------------------------------------------------------------
            case CMD_EXIT:
                {
                    exitThread = true;
                    break;
                }

			//------------------------------------------------------------
			// Flush rendering thread, i.e. synchronize it with 
			// the application thread. 
			//------------------------------------------------------------
			case CMD_FLUSH:
				{
					Platform::Semaphore* semaphore = NULL;

					// Get synchronization semaphore
                    reader.get((Platform::Semaphore*&)semaphore);
                   
					// Increment the semaphore to signal that 
					// CMD_FLUSH was reached.
					Platform::incSemaphore(semaphore);

                    break;
				}

			//------------------------------------------------------------
			// Render text on screen
			//------------------------------------------------------------
			case CMD_RENDER_TEXT:
				{
					char *text;
					int x, y;
					Vector4 color;

					// Read parameters
                    reader.get(x);
					reader.get(y);
					reader.get(color);
					reader.get((char*&)text);

					// Do actual rendering
					Platform::fontPrint(x, y, color, text);

					// text can now be deleted (allocated in Renderer::fontPrint)
					delete [] text;

                    break;
				}

			//------------------------------------------------------------
			// Create a mesh.
			//------------------------------------------------------------
            case CMD_CREATE_MESH:
                {

                    Platform::Mesh** mesh        = NULL;
                    Vector3*         vertices    = NULL;
                    Vector3*         normals     = NULL;
                    Vector3i*        triangles   = NULL;
                    int nVertices                = 0;
                    int nTriangles               = 0;

                    // Get mesh data pointers from 
                    reader.get((Platform::Mesh**&)mesh);
                    reader.get((Vector3*&)vertices);
                    reader.get((Vector3*&)normals);
                    reader.get((Vector3i*&)triangles);
                    reader.get(nVertices);
                    reader.get(nTriangles);

                    assert(vertices && normals && triangles && nVertices > 0 && nTriangles > 0);

                    // Create a mesh. Note that Renderer::createMesh will use CMD_FLUSH to ensure 
                    // proper synchronization.
                    *mesh = Platform::createPlatformMesh(vertices, normals, triangles, nVertices, nTriangles);

                    break;
                }

			//------------------------------------------------------------
			// Destroy a mesh.
			//------------------------------------------------------------
            case CMD_DELETE_MESH:
                {
                    // Get object from command queue.

                    Platform::Mesh* mesh = NULL;

                    reader.get((Platform::Mesh*&)mesh);
                   
                    Platform::releasePlatformMesh(mesh);

                    break;
                }

			default:
                {
                    assert(false);
                    break;
                }
            };

			// If exit was requested
            if (exitThread)
                break;
        }
    }
}

} // namespace MultithreadedSample

//------------------------------------------------------------------------
