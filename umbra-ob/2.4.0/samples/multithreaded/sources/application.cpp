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
 * \brief     Sample application.
 * 
 *//*-------------------------------------------------------------------*/

#include <stdio.h>

// Platformlib inludes.
#include "platform.hpp"
#include "plMesh.hpp"

// Application includes.
#include "commander.hpp"
#include "object.hpp"
#include "model.hpp"
#include "meshFactory.hpp"
#include "application.hpp"
#include "renderer.hpp"

// Umbra includes.
#include "umbra.hpp"

#if defined(UMBRA_EVALUATION)
// Include evaluation key from evaluation.hpp.
// Evaluation.hpp can be found directly under samples directory.
#include "evaluation.hpp"
#endif

using namespace Platform;
using namespace MultithreadedSample;

Application*    g_application      = 0;     /*!< Application instance */

const int       NUM_OBJECTS         = 1000;                         /*!< Number of static objects in the application. */
const int       NUM_DYNAMIC_OBJECTS = 10;                           /*!< Number of dynamic objects in the application. */
const float     STAT_UPDATE_TIME    = 150.f;                        /*!< Frequency of stat updates on screen. */	

#if _XBOX_VER >= 200 
	// Xbox360 controls
	#define CONTROLNAME_EXIT			 "Back"
	#define CONTROLNAME_HELP			 "Start"
	#define CONTROLNAME_ANIMATION		 "A"
	#define CONTROLNAME_OCCLUSIONCULLING "B"
	#define CONTROLNAME_LATENTQUERIES    "RB"
	#define CONTROLNAME_WIREFRAME	     "X"
	#define CONTROLNAME_DEBUGLINES       "Y"
	#define CONTROLNAME_MOVE             "Left control stick"
	#define CONTROLNAME_LOOK             "Right control stick"
#elif defined( _WIN32 )
	// PC controls
	#define CONTROLNAME_EXIT			 "ESC"
	#define CONTROLNAME_HELP			 "H"
	#define CONTROLNAME_ANIMATION		 "E"
	#define CONTROLNAME_OCCLUSIONCULLING "O"
	#define CONTROLNAME_LATENTQUERIES    "Q"
	#define CONTROLNAME_WIREFRAME	     "R"
	#define CONTROLNAME_DEBUGLINES       "L"
	#define CONTROLNAME_MOVE             "W/A/S/D"
	#define CONTROLNAME_LOOK             "Mouse/Arrow keys"
#else
	// PS3 controls
	#define CONTROLNAME_EXIT			 "Select"
	#define CONTROLNAME_HELP			 "Start"
	#define CONTROLNAME_ANIMATION		 "Cross"
	#define CONTROLNAME_OCCLUSIONCULLING "Circle"
	#define CONTROLNAME_LATENTQUERIES    "R1"
	#define CONTROLNAME_WIREFRAME	     "Square"
	#define CONTROLNAME_DEBUGLINES       "Triangle"
	#define CONTROLNAME_MOVE             "Left control stick"
	#define CONTROLNAME_LOOK             "Right control stick"
#endif

static const char*	g_onscreenHelpText =
CONTROLNAME_MOVE " - Move\n"
CONTROLNAME_LOOK " - Look\n"
"\n"
CONTROLNAME_ANIMATION " - Toggle animation\n"
CONTROLNAME_OCCLUSIONCULLING " - Toggle occlusion culling\n"
CONTROLNAME_LATENTQUERIES " - Toggle latent queries\n"
CONTROLNAME_WIREFRAME " - Toggle wireframe mode\n"
CONTROLNAME_DEBUGLINES " - Cycle debug lines\n"
"\n"
CONTROLNAME_HELP " - This help text\n"
CONTROLNAME_EXIT " - Exit\n";

static const char*	g_onscreenHelpPrompt =
"Press " CONTROLNAME_HELP " for help";

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Application constructor.
 *
 *          Initialized Umbra Occlusion Booster and creates objects 
 *          to render.
 *
 *//*-------------------------------------------------------------------*/

Application::Application(void) :
m_renderer(NULL),
m_cell(NULL),
m_sphereModel(NULL),
m_debugLineMode(0),
m_occlusionCulling(true),
m_latentQueries(false),
m_exit(false),
m_animation(false),
m_helpDisplayed(false),
m_helpTextX(0),
m_helpTextY(0),
m_helpPromptY(0),
m_time(0.f),
m_dTime(0.f),
m_animationStopTime(0.f),
m_animationSkipTime(0.f),
m_statTime(0.f),
m_statFrameSamples(0),
m_statFps(0.f),
m_statFrameTime(0.f),
m_screenWidth(0),
m_screenHeight(0),
m_windowed(false)
{
	// Get screen size.

    Platform::getScreenSize(m_screenWidth, m_screenHeight, m_windowed);

	// Initialize Umbra.
 
    umbraInit();

    // Create Renderer interface object for controlling the rendering thread.

    m_renderer = new Renderer(&m_renderStream);

    // Create Umbra::Cell. The cell is created first, so that cameras and objects 
	// can be later placed in it.
    
    createCell();

    // Create and initialize a Camera.
    
    createCamera();

	// Create scene objects
 
    createObjects(m_renderer);

	// Calculate positions for help text

	textInit();
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Application destructor.
 *
 *          Releases resources allocated by the Application class and
 *          deinitializes Umbra Occlusion Booster.
 *
 *//*-------------------------------------------------------------------*/

Application::~Application(void)
{
	// Flush to wait for the rendering thread to complete any pending 
	// commands before destroying anything

	m_renderer->finish();

    // Destroy objects and mesh.
 
    destroyObjects();
    
    // Delete Cameras.

    for (int i = 0; i < NUM_CAMERAS; i++) 
	{
		delete m_cameras[i];
		m_cameras[i] = NULL;
	}

    // Destroy Renderer

    delete m_renderer;

    // Deinitialize Umbra.

	umbraExit();
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Does frame to frame updates for the application.
 *
 * \return  Returns true if everything is ok. False if the application
 *          needs to exit.
 *
 *//*-------------------------------------------------------------------*/

bool Application::update(void)
{
    // Process keyboard input.
 
    handleInput();

    // Update camera parameters.

    updateCamera();

    // Animate the scene, if animation is enabled

    if (m_animation)
        animateObjects(m_renderer);

    // Render frame.

    render();

#if PRINT_STATISTICS
    if (count == 100)
    {
	    for (int i = Umbra::Library::STAT_TIMER_FIRST; i < Umbra::Library::STAT_TIMER_MAX; i++)
	    {
	        float t = Umbra::Library::getStatistic( (Umbra::LibraryDefs::Statistic)i );
	        std::printf("%s: %.3fms\n", Umbra::Library::getStatisticName((Umbra::LibraryDefs::Statistic)i), t*1000.f);
	    }
	}
	Umbra::Library::resetStatistics();
#endif

    // Return true if still running
    return !m_exit;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Creates an instance of Umbra::Cell.
 *
 *			Umbra::Cell defines a distinct space, where objects and cameras 
 *			reside.
 *
 *          This application uses a single cell. Umbra::Camera and all
 *          Umbra::Objects created later are placed in this cell.
 *
 *//*-------------------------------------------------------------------*/

void Application::createCell(void)
{
    m_cell = Umbra::Cell::create();
    assert(m_cell);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Create and initialize Cameras. This application supports
 *			multiple cameras with splitscreen.
 *
 *			Note that unlike the basic sample, the multithreaded sample
 *			doesn't directly handle Umbra::Camera objects. The 
 *			multithreaded sample's renderer implements a simple camera
 *			class as a wrapper for Umbra::Camera to allow splitscreen.
 *
 *          Umbra::Camera must be created and updated with the same 
 *			parameters as the application's camera (i.e. vf, viewport 
 *			and transformation matrix).
 *
 * \sa      Application::updateCamera(void)
 *
 *//*-------------------------------------------------------------------*/

void Application::createCamera(void)
{
     // Create cameras

    int c = 0;
    
    for (int j = 0; j < Y_CAMERAS; j++)
    for (int i = 0; i < X_CAMERAS; i++)
    {
	    m_cameras[c] = new Camera(m_renderer);

        Camera::Viewport vp;

        vp.x = i * (m_screenWidth/X_CAMERAS);
        vp.y = j * (m_screenHeight/Y_CAMERAS);
        vp.w = m_screenWidth/X_CAMERAS;
        vp.h = m_screenHeight/Y_CAMERAS;

        m_cameras[c]->setViewport(vp);

        c++;
    }

    Umbra::Frustum frustum;

	frustum.left    = -1.0f;
	frustum.right   =  1.0f;
	frustum.bottom  = -0.75f;
	frustum.top     =  0.75f;
	frustum.zNear   =  1.f;
	frustum.zFar    =  1000.f;
    
	// Configure cell, transformation and frustum for each camera
    for (int i = 0; i < NUM_CAMERAS; i++)
    {
        m_cameras[i]->getUmbraCamera()->setCell(m_cell);

   	    // Create a transformation matrix and set it to the 
   	    // Umbra::Camera as an initial camera to cell matrix.
        // This matrix must be the same as the application
        // view matrix.

	    m_cameraToCell.ident();
	    m_cameraToCell.setTranslation(Vector3(0.f, 0.f, -50.f));

		m_cameras[i]->getUmbraCamera()->setCameraToCellMatrix((Umbra::Matrix4x4&)m_cameraToCell[0][0]);

        // Set view frustum to Umbra::Camera.
    
        m_cameras[i]->getUmbraCamera()->setFrustum(frustum);
    }
}

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Umbra Occlusion Booster initialization.
 *
 *          Calls Umbra::Library::init. Creates SampleGraphicsServices,
 *          SampleCommander, Umbra::Cell and Umbra::Camera.
 *
 *//*-------------------------------------------------------------------*/

void Application::umbraInit(void)
{
	// Register evaluation key.
	// setEvaluationKey must be called before Umbra::Library::init.
	// See samples\evaluation.hpp for setting your evaluation key
	// for the samples.

	#if defined(UMBRA_EVALUATION)
	Umbra::Library::setEvaluationKey( UMBRA_EVALUATION_KEY );
	#endif

#ifdef __CELLOS_LV2__ // PS3
    int cores[2] = { 0, 1 };
    Umbra::Library::initThreads(Platform::getSpursContext(), 2, cores, 8);
#else
    int cores[2] = { 2, 3 };
    Umbra::Library::initThreads(NULL, 2, cores, 0);
#endif

    // Initialize Umbra
    
	Umbra::Library::init(Umbra::Library::COLUMN_MAJOR, NULL);
    
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Umbra Occlusion Booster deinitialization.
 *
 *          Releases Umbra:Cell.Calls Umbra::Library::exit.
 *
 *          Unlike with the Basic sample, the renderer owns 
 *	        SampleCommander and SampleGraphicsServices, and is 
 *			responsible for deleting them.
 *
 *//*-------------------------------------------------------------------*/

void Application::umbraExit(void)
{   
	// Release Umbra::Cell.
    
	m_cell->release();

    // Exit Umbra.

	Umbra::Library::exit();
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Camera update function. Configures application's 
 *			Cameras with current frame's properties and camera 
 *			matrix.
 *
 *          Umbra::Camera must have the same parameters as the
 *          application's camera (view frustum, viewport and 
 *          transformation matrix).
 *
 *//*-------------------------------------------------------------------*/

void Application::updateCamera(void)
{    
    // Setup camera properties.
    
    unsigned int properties = Umbra::Camera::VIEWFRUSTUM_CULLING;

	// If occlusion culling is enabled, turn on occlusion culling for the camera.

    if (m_occlusionCulling)
        properties |= Umbra::Camera::OCCLUSION_CULLING;
    
    // If latent queries are enabled, turn on latent queries for the camera.

	if (m_latentQueries)
		properties |= Umbra::Camera::LATENT_QUERIES;

    // Set view frustum and parameters for all cameras.
    
    for (int i = 0; i < NUM_CAMERAS; i++)
    {
		m_cameras[i]->getUmbraCamera()->setProperties(properties);

        // Set camera to cell matrix for Umbra::Camera.

        m_cameras[i]->getUmbraCamera()->setCameraToCellMatrix((Umbra::Matrix4x4&)m_cameraToCell[0][0]);
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Creates objects to render. 
 *
 *			This function handles application level objects and models.
 *          Umbra::Objects and Umbra::Models are created in Object and 
 *			Model class constructor. See the class constructors for more
 *			info on creating Umbra::Objects and Umbra::Models.
 *
 *//*-------------------------------------------------------------------*/

void Application::createObjects(Renderer* renderer)
{
    (void) renderer;
    Vector3*    vertices        = 0;
    Vector3*    normals         = 0;
    Vector3i*   triangles       = 0;
    int         vertexCount     = 0;
    int         triangleCount   = 0;

    // Use MeshFactory functions to create vertex, index and normal arrays for a sphere.

    MeshFactory::createGeoSphere(2, 2.35f, vertices, normals, triangles, vertexCount, triangleCount);

	assert("Invalid mesh data from MeshFactory" && vertices && normals && triangles && vertexCount > 0 && triangleCount > 0);

    // Use the data to create a Model from the sphere data.

    m_sphereModel = new Model(vertices, normals, triangles, vertexCount, triangleCount, m_renderer);
	assert(m_sphereModel);

	// Delete our copy of mesh data

	delete [] vertices;
	delete [] normals;
	delete [] triangles;

    // Create multiple instances of the sphere object.

    const int numOccludees = NUM_OBJECTS;
    const int numOccluders = NUM_DYNAMIC_OBJECTS;

    std::srand(0);

    // add small objects

	for (int i = 0; i < numOccludees; i++)
	{       
        // Create a new Object instance.
        
        Object* obj = new Object(m_sphereModel, m_renderer);
        assert(obj);
    
        // Insert the object in Umbra::Cell

        obj->setCell(m_cell);

        // Set object transformation

        Matrix4x4   objectToCell;

		// Give the object a random position
        objectToCell.setTranslation(Vector3( (float)std::rand() / (float)RAND_MAX * 30.f - 15.f, 
                                             (float)std::rand() / (float)RAND_MAX * 30.f - 15.f,
                                             (float)std::rand() / (float)RAND_MAX * 30.f - 15.f)); 

        obj->setObjectToCellMatrix(objectToCell);

        m_objects.push_back(obj);
	}

    // add bigger objects to act as good occluders

    for (int i = 0; i < numOccluders; i++)
	{       
        // Create a new Object instance.
        
        Object* obj = new Object(m_sphereModel, m_renderer);
        assert(obj);
    
        // Insert the object in Umbra::Cell

        obj->setCell(m_cell);

        // Set object transformation

        Matrix4x4   objectToCell;

        float d = (float)i/(float)numOccluders*PI*2;
		
        objectToCell.setTranslation(Vector3( std::sin(d)*30.f, 
                                             0.f,
                                             std::cos(d)*30.f)); 

        objectToCell.scale(Vector3(3, 3, 3));

        obj->setObjectToCellMatrix(objectToCell);

        m_objects.push_back(obj);
        m_dynamicObjects.push_back(obj); // these objects are dynamic
	}
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Releases all resources created in Application::createObjects.
 *
 *          Umbra::Objects and Umbra::Models are released when
 *          Object class instances are deleted. See Object
 *          class destructor for sample code on how to release
 *          Umbra::Models and Umbra::Objects.
 *
 *//*-------------------------------------------------------------------*/

void Application::destroyObjects(void)
{    
    // Delete all objects.
    
	for(size_t i = 0; i < m_objects.size(); i++)
		delete m_objects[i];

	m_objects.clear();

    // Delete sphere mesh.

    delete m_sphereModel;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Animates objects.
 *
 *			The function is called from update if animation is 
 *			enabled.
 *
 *          Updates dynamic objects.
 *
 *//*-------------------------------------------------------------------*/

void Application::animateObjects(Renderer* renderer)
{
    (void)renderer;
    Matrix4x4   objectToCell;
    Vector3     translation;
	float       ang           = (Platform::getTime() - m_animationSkipTime)/1000.f;

    int count = (int)m_dynamicObjects.size();

    for (int i = 0; i < count; i++)
	{               
        Object* obj = m_dynamicObjects[i];        

        // Set object transformation

        Matrix4x4 objectToCell = obj->getObjectToCellMatrix();

        float d = (float)i/(float)count*PI*2 + ang;
		
        objectToCell.setTranslation(Vector3( std::sin(d)*30.f, 
                                             0.f,
                                             std::cos(d)*30.f)); 

        obj->setObjectToCellMatrix(objectToCell);
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Moves an object away from camera.
 *
 * \param	objectTrans Object translation to operate on
 *
 *//*-------------------------------------------------------------------*/

void Application::dodgeCamera(Vector3& objectTrans)
{
	// Get camera's position
    Vector3 cameraTrans = m_cameraToCell.getTranslation();
	// Calculate a vector from camera position to object
    Vector3 direction   = objectTrans - cameraTrans;
	// Get length of the direction vector, object's distance from camera
    float   len         = length(direction);

	// If the object position too close to camera
    if (len < 3.f)
    {
		// Shift the object away from camera
        objectTrans.x += (direction.x * 0.5f)/len;
        objectTrans.y += (direction.y * 0.5f)/len;
        objectTrans.z += (direction.z * 0.5f)/len;
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Function for handling keyboard and console pad input.
 *
 *          Moves the camera in a 3D free flying style.
 *
 *//*-------------------------------------------------------------------*/

void Application::handleInput(void)
{
    Platform::KeyboardState keyboardState;
    Platform::PadState      padState;
	Platform::MouseState    mouseState;
    Matrix4x4               rotationX;
    Matrix4x4               rotationY;

    int                     multiplier      = 0;
    float                   move            = 0.f;
    float                   time            = 0.f;

	// Get keyboard and pad input
    Platform::readKeyboard(keyboardState);
    Platform::readPad(padState);
	Platform::readMouse(mouseState);

	// If no mouse state available, populate old mouse state automatically
	if(m_mouseState.x == 0 && m_mouseState.y == 0)
		m_mouseState = mouseState;

    time    = Platform::getTime();
    m_dTime = time - m_time;
    move    = m_dTime/50.f;
    m_time  = time;
    
    // Exit
    // ESC (Windows)
    // Select (PS3)
    // Back (Xbox360)
    
    if (keyboardState.symbolsBitmap & Platform::KEY_ESC || 
        padState.keyBitmap & Platform::BUTTON_BACK)
        m_exit = true;
    
    // Toggle help text
    // H (Windows)
    // Start (PS3)
    // Start (Xbox360)
    
    if (((keyboardState.alphabetBitmap & Platform::KEY_H) && 
        !(m_keyboardState.alphabetBitmap & Platform::KEY_H)) ||
        ((padState.keyBitmap & Platform::BUTTON_START) &&
        !(m_padState.keyBitmap & Platform::BUTTON_START)))
	{
		m_helpDisplayed = !m_helpDisplayed;
	}
   

	// Toggle animation
    // E (Windows)
    // Cross (PS3)
    // A (Xbox360)
    
    if (((keyboardState.alphabetBitmap & Platform::KEY_E) && 
        !(m_keyboardState.alphabetBitmap & Platform::KEY_E)) ||
        ((padState.keyBitmap & Platform::BUTTON_RDOWN) &&
        !(m_padState.keyBitmap & Platform::BUTTON_RDOWN)))
    {
        m_animation = !m_animation;

		if(!m_animation)
			// Remember when animation was stopped.
			m_animationStopTime = Platform::getTime();
		else {
			float delta = Platform::getTime() - m_animationStopTime;
			// Accumulate the amount of time the animation as stopped.
			m_animationSkipTime += delta;
		}
    }

    // Toggle occlusion culling
    // O (Windows)
    // Circle (PS3)
    // B (Xbox360)
    
    if (((keyboardState.alphabetBitmap & Platform::KEY_O) &&
        !(m_keyboardState.alphabetBitmap & Platform::KEY_O)) ||
        ((padState.keyBitmap & Platform::BUTTON_RRIGHT) &&
        !(m_padState.keyBitmap & Platform::BUTTON_RRIGHT)))
    {
        m_occlusionCulling = !m_occlusionCulling;
    }

    // Toggle latent queries
    // Q (Windows)
    // R1 (PS3)
    // RB (Xbox360)

	if (((keyboardState.alphabetBitmap & Platform::KEY_Q) && 
        !(m_keyboardState.alphabetBitmap & Platform::KEY_Q)) ||
		((padState.keyBitmap & Platform::BUTTON_R1) &&
        !(m_padState.keyBitmap & Platform::BUTTON_R1)))
    {
        m_latentQueries = !m_latentQueries;
    }

    // Toggle wireframe
    // R (Windows)
    // Square (PS3)
    // X (Xbox360)

    if (((keyboardState.alphabetBitmap & Platform::KEY_R) && 
        !(m_keyboardState.alphabetBitmap & Platform::KEY_R)) ||
        ((padState.keyBitmap & Platform::BUTTON_RLEFT) &&
        !(m_padState.keyBitmap & Platform::BUTTON_RLEFT)))
    {
        SampleCommander::m_wireframe = !SampleCommander::m_wireframe;
    }

    // Cycle debug lines
    // L (Windows)
    // Triangle (PS3)
    // Y (Xbox360)

    if (((keyboardState.alphabetBitmap & Platform::KEY_L) && 
        !(m_keyboardState.alphabetBitmap & Platform::KEY_L)) ||
        ((padState.keyBitmap & Platform::BUTTON_RUP) &&
        !(m_padState.keyBitmap & Platform::BUTTON_RUP)))
    {
		m_debugLineMode++;
		m_debugLineMode = m_debugLineMode % 5;

		Umbra::Library::clearFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OBJECT_BOUNDS); 
		Umbra::Library::clearFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_VOXELS);
		Umbra::Library::clearFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OBJECT_OBBS);
		Umbra::Library::clearFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OCCLUSION_QUERIES);

		if (m_debugLineMode == 1)
			Umbra::Library::setFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OBJECT_BOUNDS);
		else if (m_debugLineMode == 2)
			Umbra::Library::setFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_VOXELS);
		else if (m_debugLineMode == 3)
			Umbra::Library::setFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OBJECT_OBBS);
		else if (m_debugLineMode == 4)
			Umbra::Library::setFlags(Umbra::LibraryDefs::LINEDRAW, Umbra::LibraryDefs::LINE_OCCLUSION_QUERIES);
	}

    Vector3 dof     = m_cameraToCell.getDof();
    Vector3 right   = m_cameraToCell.getRight();
    Vector3 trans   = m_cameraToCell.getTranslation();

    // Console controller input.

    trans.x -= dof.x/length(dof) * padState.analogThumbLY * move;
    trans.y -= dof.y/length(dof) * padState.analogThumbLY * move;
    trans.z -= dof.z/length(dof) * padState.analogThumbLY * move;

    trans.x += right.x/length(right) * padState.analogThumbLX * move;
    trans.y += right.y/length(right) * padState.analogThumbLX * move;
    trans.z += right.z/length(right) * padState.analogThumbLX * move;

    Matrix4x4::getRotateY(rotationY, move * -0.1f * padState.analogThumbRX);
    Matrix4x4::getRotateX(rotationX, move * -0.1f * padState.analogThumbRY);

	// Mouse input

	int mouseXDelta = (int)mouseState.x - (int)m_mouseState.x;
	int mouseYDelta = (int)mouseState.y - (int)m_mouseState.y;

	if(mouseXDelta != 0 || mouseYDelta != 0)
	{
		Matrix4x4::getRotateY(rotationY, -0.005f * mouseXDelta);
		Matrix4x4::getRotateX(rotationX, -0.005f * mouseYDelta);
	}

    // Movement forward and backward

    if (keyboardState.alphabetBitmap & Platform::KEY_W)
        multiplier = 1;
    else if (keyboardState.alphabetBitmap & Platform::KEY_S)
        multiplier = -1;
    else
        multiplier = 0;

    trans.x += dof.x/length(dof) * move * multiplier;
    trans.y += dof.y/length(dof) * move * multiplier;
    trans.z += dof.z/length(dof) * move * multiplier;

    // Movement sideways

    if (keyboardState.alphabetBitmap & Platform::KEY_D)
        multiplier = 1;
    else if (keyboardState.alphabetBitmap & Platform::KEY_A)
        multiplier = -1;
    else
        multiplier = 0;

    trans.x += right.x/length(right) * move * multiplier;
    trans.y += right.y/length(right) * move * multiplier;
    trans.z += right.z/length(right) * move * multiplier;

	// Camera rotation up and down

    if (keyboardState.symbolsBitmap & Platform::KEY_UP)
        multiplier = 1;
    else if (keyboardState.symbolsBitmap & Platform::KEY_DOWN)
        multiplier = -1;
    else
        multiplier = 0;

    if (multiplier)
        Matrix4x4::getRotateX(rotationY, move * -0.05f * multiplier);

	// Camera rotation left and right

    if (keyboardState.symbolsBitmap & Platform::KEY_RIGHT)
        multiplier = 1;
    else if (keyboardState.symbolsBitmap & Platform::KEY_LEFT)
        multiplier = -1;
    else
        multiplier = 0;

    if (multiplier)
        Matrix4x4::getRotateY(rotationX, move * -0.05f * multiplier);

    // apply movement

    m_cameraToCell.setTranslation(trans);
    rotationX       = rotationX * rotationY;
    m_cameraToCell  = rotationX * m_cameraToCell;

	// Store old states for comparison

    m_keyboardState = keyboardState;
    m_padState      = padState;

	// Center the mouse in window and save the result in m_mouseState for comparison
	Platform::centerMouse(m_mouseState);	

}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Renders objects created in Application::createObjects.
 *
 *			Two calls to Occlusion Booster are required for rendering a
 *			frame: Camera::resolveVisibility and 
 *			Camera::processVisibility. The first prompts the library 
 *			to start the visibility query. The latter indicates commander 
 *			can be now invoked from the current thread.
 *
 *			This application uses a multithreaded renderer.
 *			Thus resolveVisibility is called from the application
 *			thread (called in Renderer::render when invoked below), but
 *			processVisibility is called from the rendering thread. This
 *			way occlusion booster callbacks are processed by the same
 *			thread which does the rendering.
 *
 *//*-------------------------------------------------------------------*/

void Application::render(void)
{
    // Begin frame.
 
    m_renderer->beginFrame();

	// Call render for all cameras
    
	for (int i = 0; i < NUM_CAMERAS; i++)
        m_renderer->render(m_cameras[i]);    

	// Measure and render statistics on screen

	renderStats();

	// Render on-screen help

	renderHelp();

    // End frame.

	m_renderer->endFrame();
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Measure and render statistics on screen.
 *
 *//*-------------------------------------------------------------------*/

void Application::renderStats(void)
{
	float time = Platform::getTime();
	m_statFrameSamples++;

	if( time - m_statTime > STAT_UPDATE_TIME )
	{
		m_statFrameTime = (time - m_statTime)/(float)m_statFrameSamples;
		m_statFps	    = 1000.f/m_statFrameTime;

		m_statFrameTime = ((int)(m_statFrameTime*100.f))/100.f;

		m_statFrameSamples = 0;
		m_statTime = time;
	}

	m_renderer->fontPrint(5, 5, Vector4(1,1,1,1), 
		"%g ms\n"
		"%d fps\n"
		"Occlusion culling: %s\n"
		"Latent occlusion: %s",
		m_statFrameTime, (int)m_statFps,
		m_occlusionCulling ? "on" : "off",
		m_latentQueries ? "on" : "off");
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Render help on screen.
 *
 *//*-------------------------------------------------------------------*/

void Application::renderHelp(void)
{
	if(m_helpDisplayed)
		m_renderer->fontPrint(m_helpTextX, m_helpTextY, Vector4(1,1,1,1), g_onscreenHelpText);
	else
		m_renderer->fontPrint(5, m_helpPromptY, Vector4(1,1,1,1), g_onscreenHelpPrompt);
}
/*-------------------------------------------------------------------*//*!
 *
 * \brief   Initializes text rendering. Calculates positions for 
 *			onscreen text.
 *
 *//*-------------------------------------------------------------------*/

void Application::textInit(void)
{
	// Measure size for help text

	int helpTextWidth = 0;
	int helpTextHeight = 0;
	Platform::fontMeasure(helpTextWidth, helpTextHeight, g_onscreenHelpText);

	// Calculate position for centered help text

	m_helpTextX = m_screenWidth/2 - helpTextWidth/2;
	m_helpTextY = m_screenHeight/2 - helpTextHeight/2;

	// Measure size for "Press X for help" - label
	
	int helpPromptWidth = 0;
	int helpPromptHeight = 0;
	Platform::fontMeasure(helpPromptWidth, helpPromptHeight, g_onscreenHelpPrompt);

	// Calculate position for the label

	m_helpPromptY = m_screenHeight - helpPromptHeight - 5;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Get video mode.
 *
 *			Allows application to decide the video mode.
 *
 *//*-------------------------------------------------------------------*/

bool getVideoMode(Platform::VideoMode& videoMode)
{
	videoMode.width        = 1280;
	videoMode.height	   = 720;
	videoMode.windowed	   = true;
	videoMode.multisamples = 1;

	return true;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Application initalization callback.
 *
 *//*-------------------------------------------------------------------*/

bool initApplication(void)
{
    // Create new application instance.

    g_application = new Application();

    if (g_application)
        return true;
    
	assert(false);
    return false;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Application update callback.
 *
 *//*-------------------------------------------------------------------*/

bool updateApplication(void)
{
	assert(g_application);
    return g_application->update();
}

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Application deinitialization callback.
 *
 *//*-------------------------------------------------------------------*/

void exitApplication(void)
{
    delete g_application;
	g_application = 0;
}
