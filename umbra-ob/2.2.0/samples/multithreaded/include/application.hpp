#ifndef __APPLICATION_HPP
#define __APPLICATION_HPP

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
 * \subpage
 * \file
 * \brief       Application class.
 * 
 *//*-------------------------------------------------------------------*/

// Platformlib includes.
#include "plDataStream.hpp"

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Namespace for the multithreaded sample.
 * 
 *			This namespace contains all the basic multithreaded code,
 *			including code required for Umbra Occlusion Booster 
 *			integration.
 *
 *			The Application class is the main class of the sample,
 *			containing high level functionality. 
 *			SampleGraphicsServices and SampleCommander are classes
 *			required for the integration. Model and object are
 *			application-level classes for 3D meshes and mesh 
 *			instances.
 *
 *			Similiarly to the basic sample, this sample demonstrates
 *			the following steps to integrate Umbra Occlusion Booster:
 *				- Initializing the library
 *				- Commander implementation
 *				- GraphicsServices implementation
 *				- Creating a cell for objects
 *				- Creating an Umbra::Camera
 *				- Integrating models
 *				- Integrating objects
 *				- Performing visibility resolve
 *				- Deinitializing the library
 *
 *			In addition to these, the multithreaded sample differs from
 *			the basic sample by performing multithreaded rendering. A 
 *			separate renderer object is used for this. This demonstrates
 *			how the commander commands can be processed in different
 *			thread from where visibility resolve is initiated. 
 *			Furthermore, the multithreaded sample implements split
 *			screen rendering.
 *
 *//*-------------------------------------------------------------------*/

namespace MultithreadedSample
{

class Camera;

#define NUM_MAX_OBJECTS 1000        /*!< Number of objects in the application. */
#define PRINT_STATISTICS 0

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Main class of the multithreaded sample. 
 *			
 *			The application class initializes, updates and deinitializes 
 *			the sample. A number of animated sphere objects are created
 *			to demonstrate Umbra Occlusion Booster.
 *
 *			This class is very similiar to the application of the 
 *			basic sample. Unlike the basic sample however, here a 
 *			separate renderer object is used for multithreaded 
 *			rendering. This class also implements split screen, which
 *			can be enabled by adjusting X_CAMERAS and Y_CAMERAS 
 *			parameters.
 *
 *//*-------------------------------------------------------------------*/

class Application
{
public:

            Application     (void);
            ~Application    (void);

	// Application update method
    bool    update          (void);

private:

    static const int X_CAMERAS = 1;					/*!< Number of cameras/viewports vertically. */
    static const int Y_CAMERAS = 1;					/*!< Number of cameras/viewports horizontally. */
    static const int NUM_CAMERAS =					
		X_CAMERAS*Y_CAMERAS;						/*!< Total Number of cameras/viewports. */

	// Initialization/deinitialization methods
    void    createCell      (void);
    void    createCamera    (void);
    void    umbraInit       (void);
	void    textInit        (void);
    void    umbraExit       (void);
    void    createObjects   (Renderer*);
    void    destroyObjects  (void);

	// Runtime methods
    void    updateCamera    (void);
    void    animateObjects  (Renderer*);
    void    dodgeCamera     (Platform::Vector3& objectTrans);
    void    render          (void);
	void    renderStats     (void);
	void    renderHelp      (void);
    void    handleInput     (void);


    class Renderer*         m_renderer;				/*!< Rendering functionality separated as renderer to cope with multithreading. */
    Platform::DataStream    m_renderStream;			/*!< Datastream, which the renderer uses. */
    std::vector<Object*>    m_objects;              /*!< A vector containing all renderable objects. */

    std::vector<Object*>    m_dynamicObjects;       /*!< Vector containing dynamic objects. */

    Platform::Matrix4x4     m_cameraToCell;         /*!< Camera to cell matrix. Allows the application to keep track of camera movement. */

	Umbra::Cell*            m_cell;                 /*!< Application's Umbra::Cell instance. All objects and cameras belong in a cell. This application only uses a single cell. */
    Model*                  m_sphereModel;          /*!< Model object that holds 3D geometry for a sphere. */

	unsigned int			m_debugLineMode;		/*!< Which debug line is enabled */
    bool                    m_occlusionCulling;     /*!< True if occlusion culling is enabled. */
	bool                    m_latentQueries;		/*!< True if latent queries is enabled. */
    bool                    m_exit;                 /*!< True if application will exit on the next iteration. */
    bool                    m_animation;            /*!< True if animation is enabled. */

    Platform::KeyboardState m_keyboardState;        /*!< Keyboard state from the previous frame is stored here. */
    Platform::PadState      m_padState;             /*!< Control pad state from the previous frame is stored here. */
	Platform::MouseState    m_mouseState;           /*!< Mouse state from the previous frame is stored here. */

	bool					m_helpDisplayed;		/*!< Whether onscreen help is displayed. */
	int						m_helpTextX;			/*!< X-position for rendering help text. */
	int						m_helpTextY;			/*!< Y-position for rendering help text. */
	int						m_helpPromptY;			/*!< Y-position for rendering help text. */

    float                   m_time;                 /*!< Frame time on last update. */
    float                   m_dTime;                /*!< Timestep on current frame. */
	float					m_animationStopTime;	/*!< Time when animation was toggled */
	float					m_animationSkipTime;	/*!< Amout of time to skip due to stopped animation. */

	float					m_statTime;				/*!< Time of last stat update. */
	unsigned int			m_statFrameSamples;		/*!< Number of frames since last stat update. */
	float					m_statFps;				/*!< Latest measured fps. */
	float					m_statFrameTime;		/*!< Latest measured frame time. */

    unsigned int            m_screenWidth;          /*!< Maximum display width. */
    unsigned int            m_screenHeight;         /*!< Maximum display height. */
    bool                    m_windowed;             /*!< If the application is windowed. */

    Camera*                 m_cameras[NUM_CAMERAS]; /*!< Array of cameras. */
};

} // namespace MultithreadedSample

#endif // __APPLICATION_HPP
