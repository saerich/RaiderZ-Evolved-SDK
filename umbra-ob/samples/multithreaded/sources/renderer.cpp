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
 * \brief     A multithreded renderer 
 * 
 *//*-------------------------------------------------------------------*/

// Platformlib includes.
#include "platform.hpp"

#include "plObject.hpp"

// Application includes.
#include "renderer.hpp"
#include "graphicsServices.hpp"
#include "commander.hpp"

// System includes.
#include <assert.h>

namespace MultithreadedSample
{

/*-------------------------------------------------------------------*//*!
 * \brief   Renderer-exposed object. 
 *
 *			The purpose of this object is to hide the internal 
 *			Platform::Object to avoid direct manipulation. 
 *			RendererObjects are only manipulated trough the Renderer
 *			interface.
 *//*-------------------------------------------------------------------*/

	class RendererObject
{
public:
    Platform::Object* object;	/*!< Internal Platform Object */
};

} // namespace MultithreadedSample

using namespace Platform;
using namespace MultithreadedSample;

/*-------------------------------------------------------------------*//*!
 * \brief   Rendering thread entry point.
 *//*-------------------------------------------------------------------*/

void* MultithreadedSample::startRenderThread(void* r)
{
    DataStream* stream = static_cast<DataStream*>(r);
    assert(stream);

	// Reserve graphics thread
    Platform::bindGraphicsThread();

	Renderer::renderLoop(stream);

	// Release the graphics thread when exiting
	Platform::unbindGraphicsThread();

    return NULL;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renderer constructor. 
 *
 *			Initializes GraphicsServices and Commander. Starts the 
 *			rendering thread.
 *
 *//*-------------------------------------------------------------------*/

Renderer::Renderer(DataStream* outStream) :
    m_renderingThread(NULL)
{
    // Create data stream. This object is used to pass commands to the
    // rendering thread.

    m_dataStreamWriter.init(outStream);
    
	// This semaphore controls application thread and rendering thread
	// synchronization. Processes x frames before waiting.
    m_semaphore = Platform::createSemaphore(1);

    m_graphicsServices = new SampleGraphicsServices();
    m_commander = new SampleCommander(m_graphicsServices);

	// Put the semaphore and commander in the stream, so that the 
	// rendering thread may access them.
    m_dataStreamWriter.put(m_semaphore);
    m_dataStreamWriter.put(m_commander);

	// Release the graphics thread so that the rendering thread may 
	// reserve it.
	Platform::unbindGraphicsThread();

    // Spawn the rendering thread.
	// The thread is given the stream as parameter.
    m_renderingThread = Platform::beginThread(startRenderThread, (void*)outStream);
    assert(m_renderingThread);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Destructs Renderer.
 *//*-------------------------------------------------------------------*/

Renderer::~Renderer()
{
    // Exit the rendering thread.
    m_dataStreamWriter.put(CMD_EXIT);
    m_dataStreamWriter.flush();

    // Wait for the rendering thread to finish.
    assert(m_renderingThread);
    Platform::waitThreadToComplete(m_renderingThread);
    m_renderingThread = NULL;

	// Rendering thread now has released graphics ownership.
	// Bind it here.
	Platform::bindGraphicsThread();
    
	// Delete thread synchronization semaphore.
    Platform::deleteSemaphore(m_semaphore);

    delete m_commander;
    delete m_graphicsServices;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Set transformation matrix for a RendererObject.
 *//*-------------------------------------------------------------------*/

void Renderer::setObjectMatrix(RendererObject* obj, const Matrix4x4& mtx)
{
    m_dataStreamWriter.put(CMD_SET_OBJECT_MATRIX);
    m_dataStreamWriter.put((void*)obj);
    m_dataStreamWriter.put(mtx);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Create a RendererObject from a platform mesh.
 *//*-------------------------------------------------------------------*/

RendererObject* Renderer::createObject(Platform::Mesh* mesh)
{
    // Not streamed. The objects are safe to create outside
	// Rendering thread.
    return (RendererObject*)new Platform::Object(mesh);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Destroys a RendererObject. 
 *//*-------------------------------------------------------------------*/

void Renderer::deleteObject(RendererObject* obj)
{
    m_dataStreamWriter.put(CMD_DELETE_OBJECT);
    m_dataStreamWriter.put((void*)obj);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets an object's color.
 *//*-------------------------------------------------------------------*/

void Renderer::setObjectColor  (RendererObject* obj, const Vector4& mtx)
{
    m_dataStreamWriter.put(CMD_SET_OBJECT_COLOR);
    m_dataStreamWriter.put((void*)obj);
    m_dataStreamWriter.put(mtx);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Rendering text on screen.
 *//*-------------------------------------------------------------------*/

void Renderer::fontPrint(int x, int y, const Vector4& color, const char* text, ...)
{
	// Buffer for holding formatted text.
	// The buffer is deallocated by the rendering thread.
	char* str = new char[256];
	str[0] = 0;

	// Handle printf-style input
	va_list args;
	va_start (args, text);

	// Use vsprintf_s on microsoft compiler, vsnprintf otherwise
#if defined(_MSC_VER)
	vsprintf_s 
#else
	vsnprintf
#endif		
	(str, 256, text, args);

	va_end (args);

	// Command rendering thread to render the formatted text
    m_dataStreamWriter.put(CMD_RENDER_TEXT);
	m_dataStreamWriter.put(x);
	m_dataStreamWriter.put(y);
	m_dataStreamWriter.put(color);
    m_dataStreamWriter.put((void*)str);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Initiates visibility resolve for the camera and instructs 
 *			the rendering thread to render.
 *//*-------------------------------------------------------------------*/

void Renderer::render(Camera* camera)
{
    assert(camera);

    m_dataStreamWriter.flush();

    Platform::beginNamedEvent("resolveVisibility");
    
	// Launch resolve visibility for the camera.
	// Resolve visibility can be called in e.g. the application thread.
	// The rendering thread calls processVisibility once it's ready to
	// process Occlusion Booster callbacks in commander.
    camera->getUmbraCamera()->resolveVisibility();

    Platform::endNamedEvent();

	// Get camera's viewport

	Camera::Viewport viewport;
	camera->getViewPort(viewport);

	// Send rendering command.
	// Upon receiving this command, the rendering thread will call 
	// Camera::processVisibility to receive the frame's rendering
	// callbacks from Occlusion Booster.

    m_dataStreamWriter.put(CMD_RENDER);
	m_dataStreamWriter.put(camera->getUmbraCamera());	
	
	// Send viewport

	m_dataStreamWriter.put(viewport.x);
	m_dataStreamWriter.put(viewport.y);
	m_dataStreamWriter.put(viewport.w);
	m_dataStreamWriter.put(viewport.h);

    m_dataStreamWriter.flush();

    // Don't allow the main thread to get too many frames ahead,.
    // The amount of frames is controlled by the initial semaphore value.

    Platform::beginNamedEvent("wait for render");
    Platform::decSemaphore(m_semaphore, true); 
    Platform::endNamedEvent();
}

/*-------------------------------------------------------------------*//*!
 * \brief	Instructs the rendering thread to begin a frame.
 *//*-------------------------------------------------------------------*/

void Renderer::beginFrame(void)
{
    m_dataStreamWriter.put(CMD_BEGIN_FRAME);
    m_dataStreamWriter.flush();
}

/*-------------------------------------------------------------------*//*!
 * \brief	Instructs the rendering thread to end a frame.	
 *//*-------------------------------------------------------------------*/

void Renderer::endFrame(void)
{
    m_dataStreamWriter.put(CMD_END_FRAME);
    m_dataStreamWriter.flush();
}

/*-------------------------------------------------------------------*//*!
 * \brief	Synchronize caller thread with the rendering thread.
 *			
 *			Waits until the rendering thread has completed all commands.
 *//*-------------------------------------------------------------------*/

void Renderer::finish(void)
{
	// Use a semaphore to synchronize the thread
	Platform::Semaphore *semaphore = Platform::createSemaphore(0);

	// Upon receiving CMD_FLUSH, the rendering thread will increment
	// the semaphore
	m_dataStreamWriter.put(CMD_FLUSH);
	m_dataStreamWriter.put(semaphore);
    m_dataStreamWriter.flush();

	// Decrement the semaphore, effectively waiting until rendering 
	// thread reaches CMD_FLUSH and increments the semaphore
	Platform::decSemaphore(semaphore, true);

	// Now the semaphore can be deleted
	Platform::deleteSemaphore(semaphore);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Camera constructor. 
 *//*-------------------------------------------------------------------*/

Camera::Camera(Renderer* renderer)
	: m_renderer(renderer)
{
	unsigned int width    = 100;
	unsigned int height	  = 100;
	bool		 windowed = false;

	// Get screen size
	Platform::getScreenSize(width, height, windowed);

	// Configure default viewport to screen size
    m_viewport.x = 0;
    m_viewport.y = 0;
    m_viewport.w = width;
    m_viewport.h = height;

    m_camera = Umbra::Camera::create();
}

/*-------------------------------------------------------------------*//*!
 * \brief	Destruct the camera. 
 *
 *			Calls finish() to ensure deleting rendering thread does 
 *			not use the camera anymore.
 *
 *//*-------------------------------------------------------------------*/

Camera::~Camera(void)
{
	finish(); 
	m_camera->release(); 
}
