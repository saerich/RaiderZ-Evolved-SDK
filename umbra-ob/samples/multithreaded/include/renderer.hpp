#ifndef __RENDERER_HPP
#define __RENDERER_HPP

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
#include "plDataStream.hpp"
#include "platform.hpp"

// Umbra includes.
#include "umbraCamera.hpp"

namespace MultithreadedSample
{

class SampleCommander;
class SampleGraphicsServices;
class Renderer;
// Renderer hides Platform::Object behind this class
// to prevent direct manipulation
class RendererObject;

/*-------------------------------------------------------------------*//*!
 * \brief   Render command enumeration.
 *
 *			These commands are instructions sent to the rendering thread 
 *			by the application thread. 
 *
 *			Operations such as object matrix and color updates as well
 *			as object and camera deletions are performed in the
 *			rendering thread to avoid synchronization problems.
 *			
 *//*-------------------------------------------------------------------*/

enum RenderCommand
{
    CMD_DELETE_OBJECT,      /*!< Destroy object */
    CMD_SET_OBJECT_MATRIX,  /*!< Set object transform matrix. */
    CMD_SET_OBJECT_COLOR,   /*!< Set object color for rendering */
	CMD_RENDER_TEXT,		/*!< Render text */
    CMD_RENDER,             /*!< Start rendering */ 
    CMD_EXIT,               /*!< Exit render loop */
    CMD_BEGIN_FRAME,        /*!< Sent when frame begins */
    CMD_END_FRAME,          /*!< Sent when frame ends */
	CMD_FLUSH				/*!< Flush rendering thread commands */
};

/*-------------------------------------------------------------------*//*!
 * \brief   Application camera class, wrapper for Umbra::Camera.
 *
 *			Note that while the basic sample has no separate application
 *			level Camera class, the multithreaded sample requires one to
 *			allow managing viewports for split screen rendering.
 *//*-------------------------------------------------------------------*/

class Camera
{
public:

	// Constructor and destructor
    Camera(Renderer* renderer);
    ~Camera(void);

	/*---------------------------------------------------------------*//*!
	 * \brief   Camera viewport
	 *//*---------------------------------------------------------------*/
    struct Viewport
    {
        int x;
        int y;
        int w;
        int h;
    };

	/*---------------------------------------------------------------*//*!
	 * \brief   Get internal Umbra::Camera
	 *//*---------------------------------------------------------------*/
    Umbra::Camera*      getUmbraCamera(void) const  { return m_camera; }

	// Viewport management
	void setViewport    (const Viewport& vp)        { m_viewport = vp; }
    void getViewPort    (Viewport& vp)              { vp = m_viewport; }

private:

	// Renderer that was used to create 
	Renderer*		m_renderer;
    Umbra::Camera*  m_camera;
    int             m_padding[2];
    Viewport        m_viewport;
};


/*-------------------------------------------------------------------*//*!
 *
 * \brief   Renderer interface for application thread. 
 *
 *			Actual rendering is performed in a separate rendering 
 *			thread. This class exists to allowing the application thread
 *			to interface with the rendering thread.
 *
 *			Note that all operations are performed asynchronously in
 *			the rendering thread.
 *
 *//*-------------------------------------------------------------------*/

class Renderer
{
public:

                                Renderer        (Platform::DataStream*);
                                ~Renderer       (void);

	// Object and camera management
    RendererObject*             createObject    (Platform::Mesh* mesh);
    void                        deleteObject    (RendererObject* object);
    void                        setObjectMatrix (RendererObject* obj, const Platform::Matrix4x4& mtx);
    void                        setObjectColor  (RendererObject* obj, const Platform::Vector4& color);
    
	// Text printing
	void						fontPrint		(int x, int y, const Platform::Vector4& color, const char* text, ...);

	// Frame functions
    void                        beginFrame      (void);
    void                        render          (Camera* camera);
    void                        endFrame        (void);

	// Sync rendering thread with the caller thread
	void						finish			(void);

private:

	// Rendering callback loop, executed in a separate thread
	static void					renderLoop		(Platform::DataStream* stream);

	// Thread entry point
    friend void* startRenderThread(void* r);

    Platform::DataStreamWriter  m_dataStreamWriter;	/*!< Writer, which is used to communicate to the rendering thread */
	
	Platform::Thread*           m_renderingThread;	/*!< Thread object for the rendering thread */
    Platform::Semaphore*		m_semaphore;		/*!< Semaphore which controls frame synch with the rendering thread */

    SampleGraphicsServices*     m_graphicsServices; /*!< GraphicsServices, required for Umbra Occlusion Booster integration */
    SampleCommander*            m_commander;		/*!< Commander, also required for Umbra Occlusion Booster integration */

};

} // namespace MultithreadedSample

//------------------------------------------------------------------------

#endif // __RENDRER_HPP
