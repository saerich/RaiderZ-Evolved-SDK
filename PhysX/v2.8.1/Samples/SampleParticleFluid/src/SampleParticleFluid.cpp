// ===============================================================================
//
//						   AGEIA PhysX SDK Sample Program
//
// Title: Particle Fluids
// Description: This sample program shows how to use particle fluids
//
// Originally written by: Simon Schirm (01.09.04)
//
// ===============================================================================

#define NOMINMAX
#ifdef WIN32 
#include <windows.h>
#elif LINUX
#endif

// Physics code
#undef random

#include "SampleParticleFluid.h"
#include "Utilities.h"

DebugRenderer		gDebugRenderer;
ErrorStream			gErrorStream;

// Physics SDK globals
NxPhysicsSDK*		gPhysicsSDK = NULL;
NxScene*			gScene = NULL;
NxVec3				gDefaultGravity(0,-9.8,0);
bool				gSceneRunning = false;

#ifdef NX_DISABLE_HARDWARE
bool				gHardwareSimulation = false;
#else
bool				gHardwareSimulation = true;
#endif

// Samples
unsigned			gSampleIndex = 0;
NxActor*			gSelectedActor = NULL;

NxActor*			gGroundPlane = NULL;
NxArray<SampleParticleFluid*> 
					gSamples;
NxArray<MyFluid*>	gMyFluids; 

// Time globals
NxReal				gTime;
NxReal				gLastTime;

// Display globals
int					gMainHandle;
bool				gHelp = false;
char				gDisplayString[512] = "";
char				gTitleString[512] = "";
char				gHelpString[512] = "";

// Camera globals
NxReal gCameraFov = 40.0f;
NxVec3 gCameraPos(0,5,15);
NxVec3 gCameraForward(0,0,-1);
NxVec3 gCameraRight(1,0,0);
const NxReal gCameraSpeed = 0.2;
int gViewWidth = 0;
int gViewHeight = 0;

// MouseGlobals
int mx = 0;
int my = 0;

bool gRotateCamera = false;
NxDistanceJoint* gMouseJoint = NULL;
NxActor* gMouseSphere = NULL;
NxReal gMouseDepth = 0.0f;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
bool gPause = false;
bool gShadows = true;
bool gWireframeMode = false;

// fps
int			gFrameCounter = 0;
float       gPreviousTime = getCurrentTime();

bool InitNx()
{
	// Initialize PhysicsSDK
	NxPhysicsSDKDesc desc;
	NxSDKCreateError errorCode = NXCE_NO_ERROR;
	gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, &gErrorStream, desc, &errorCode);
	if(gPhysicsSDK == NULL) 
	{
		printf("\nSDK create error (%d - %s).\nUnable to initialize the PhysX SDK, exiting the sample.\n\n", errorCode, getNxSDKCreateError(errorCode));
		return false;
	}

#ifndef NX_DISABLE_HARDWARE
	NxHWVersion hwCheck = gPhysicsSDK->getHWVersion();
	if (hwCheck == NX_HW_VERSION_NONE) 
	{
		printf("\nWarning: Unable to find a PhysX card, or PhysX card used by other application.");
		printf("\nThe fluids will be simulated in software.\n\n");
		gHardwareSimulation = false;
	}
#endif

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.005f);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity = gDefaultGravity;
    gScene = gPhysicsSDK->createScene(sceneDesc);
	if(gScene == NULL)
	{
		printf("\nError: Unable to create a PhysX scene, exiting the sample.\n\n");
		return false;
	}

	// Create the default material
	NxMaterialDesc       m; 
	m.restitution        = 0.5f;
	m.staticFriction     = 0.2f;
	m.dynamicFriction    = 0.2f;
	NxMaterial * mat = gScene->getMaterialFromIndex(0);
	mat->loadFromDesc(m); 

	gSamples[gSampleIndex]->setup();
	
	if (gScene->getNbActors() > 0)
		gSelectedActor = *gScene->getActors();
	else
		gSelectedActor = NULL;

	UpdateTime();

#if defined(_XBOX) | defined(__CELLOS_LV2__)
	glutRemapButtonExt(2, 'G', false);//Gravity
	glutRemapButtonExt(3, 'x', false);//Shadows
	glutRemapButtonExt(3, 'n', true);//Debug wireframe
#endif
	return true;
}

// ------------------------------------------------------------------------------------

void WaitForPhysics()
{
	if (gSceneRunning)
	{
		NxU32 error;
		gScene->flushStream();
		gScene->fetchResults(NX_RIGID_BODY_FINISHED, true, &error);
		assert(error == 0);
		gSceneRunning = false;
	}
}

// ------------------------------------------------------------------------------------
void ReleaseNx()
{
	WaitForPhysics();

	for (MyFluid **fluid = gMyFluids.begin(); fluid != gMyFluids.end(); fluid++)
		delete *fluid;
	gMyFluids.clear();

	if(gPhysicsSDK != NULL)
	{
		if(gScene != NULL) gPhysicsSDK->releaseScene(*gScene);
		gScene = NULL;
		NxReleasePhysicsSDK(gPhysicsSDK);
		gPhysicsSDK = NULL;
	}
}

// ------------------------------------------------------------------------------------
void ResetNx()
{
	WaitForPhysics();
	LetGoActor();
	ReleaseNx();
	if (!InitNx()) exit(0);
}

// ------------------------------------------------------------------------------------
NxReal UpdateTime()
{
    NxReal deltaTime;
    gTime = timeGetTime()*0.001f;  // Get current time in seconds
    deltaTime = gTime - gLastTime;
    gLastTime = gTime;
    return deltaTime;
}

// ------------------------------------------------------------------------------------

void RunPhysics()
{
	if (gSceneRunning)
		return;

	// Update the time step
	NxReal deltaTime = UpdateTime();

	deltaTime = 0.02f;	// todo: may go back to real time at some point

	// Run collision and dynamics for delta time since the last frame
	gScene->simulate(deltaTime);
	gSceneRunning = true;
	WaitForPhysics();

	gSamples[gSampleIndex]->update();

	gFrameCounter++;
}

// ------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	gSamples.pushBack(new SampleCreate());
	gSamples.pushBack(new SampleEmitter());
	gSamples.pushBack(new SampleCollision());
	gSamples.pushBack(new SampleForce());
	gSamples.pushBack(new SampleEvents());
	gSamples.pushBack(new SampleUserData());
	gSamples.pushBack(new SamplePacketData());

    InitGlut(argc, argv);
	// Initialize physics scene and start the application main loop if scene was created
	if (InitNx())
		glutMainLoop();
	else
		ReleaseNx();
}

// ------------------------------------------------------------------------------------

void DisplayText()
{
	sprintf(gDisplayString, "%s%s", gTitleString, gHelp?gHelpString:"");

	float y = 0.95f;
	int len = strlen(gDisplayString);
	len = (len < 1024)?len:1023;
	int start = 0;
	char textBuffer[1024];
	for(int i=0;i<len;i++)
	{
		if(gDisplayString[i] == '\n' || i == len-1)
		{
			int offset = i;
			if(i == len-1) offset= i+1;
			memcpy(textBuffer, gDisplayString+start, offset-start);
			textBuffer[offset-start]=0;
			GLFontRenderer::print(0.01, y, 0.03f, textBuffer);
			y -= 0.035f;
			start = offset+1;
		}
	}
}

// ------------------------------------------------------------------------------------

void SetTitleString(char *demoName)
{
	sprintf(gTitleString, "%s (%s)\nF1 for help\n", demoName, gHardwareSimulation ? "hardware" : "software");
}

// ------------------------------------------------------------------------------------

void SetHelpString(char *demoKeyString)
{
	char tempString[512];
	sprintf(gHelpString, "\nGeneral:\n");
	sprintf(tempString, "    1-%d: choose scene\n", gSamples.size());
	strcat(gHelpString, tempString);
	strcat(gHelpString, "    p: pause\n");
	strcat(gHelpString, "    o: single step\n");
#ifndef NX_DISABLE_HARDWARE
	strcat(gHelpString, "    h: hardware on/off\n");
#endif
	strcat(gHelpString, "    w,a,s,d: move/strafe\n");
	strcat(gHelpString, "    q,e: move up/down\n");
	strcat(gHelpString, "    mouse right: pick\n");
	strcat(gHelpString, "    space: shoot sphere\n");
	strcat(gHelpString, "    x: toggle shadows\n");
	strcat(gHelpString, "    n: toggle wireframe\n");
	strcat(gHelpString, "    F1: toggle help\n");
	strcat(gHelpString, "    F3,F4: pick push/pull\n");
	strcat(gHelpString, "    F5: toggle debug rendering\n");
	strcat(gHelpString, "    F9: toggle vsync\n");
	strcat(gHelpString, "    F10: Reset scene\n");

	if (demoKeyString)
	{
		strcat(gHelpString, "\nDemo specific:\n");
		strcat(gHelpString, demoKeyString);
	}
}

// ------------------------------------------------------------------------------------

void ProcessKeys()
{
	// Process keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Camera controls
			case 'w':{ gCameraPos += gCameraForward*gCameraSpeed; break; }
			case 's':{ gCameraPos -= gCameraForward*gCameraSpeed; break; }
			case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed; break; }
			case 'd':{ gCameraPos += gCameraRight*gCameraSpeed; break; }
			case 'e':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed; break; }
		}
	}
}

// ------------------------------------------------------------------------------------

void SetupCamera()
{
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(gCameraFov, ((float)glutGet(GLUT_WINDOW_WIDTH)/(float)glutGet(GLUT_WINDOW_HEIGHT)), 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



// ------------------------------------------------------------------------------------

void RenderScene(bool shadows)
{
    // Render all the actors in the scene
	int nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

		if (gWireframeMode)
			DrawWireActor(actor);
		else
			DrawActor(actor);

        // Handle shadows
        if (shadows && !gWireframeMode)
        {
			DrawActorShadow(actor);
        }
    }
    // Render all the fluids in the scene
	for (MyFluid **fluid = gMyFluids.begin(); fluid != gMyFluids.end(); fluid++)
	{
		(*fluid)->draw();
	}
}

// ------------------------------------------------------------------------------------

void RenderCallback()
{
	if (gScene && !gPause)
        RunPhysics();

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessKeys();
	SetupCamera();

	RenderScene(gShadows);
	
	gDebugRenderer.renderData(*(gScene->getDebugRenderable()));

	DisplayText();

    glutSwapBuffers();
}

// ------------------------------------------------------------------------------------

void ReshapeCallback(int width, int height)
{
	gViewWidth = width;
	gViewHeight = height;
    glViewport(0, 0, width, height);
}

// ------------------------------------------------------------------------------------

void IdleCallback()
{
    glutPostRedisplay();

	float time = getCurrentTime();
	float elapsedTime = time - gPreviousTime;
	
	if (elapsedTime > 1.0f) {
		char title[30];
		sprintf(title, "SampleParticleFluid %3.1f fps", (float)gFrameCounter / elapsedTime);
		glutSetWindowTitle(title);
		gPreviousTime = time;
		gFrameCounter = 0;
	}

	getElapsedTime();
}

// ------------------------------------------------------------------------------------

void KeyboardCallback(unsigned char key, int x, int y)
{
	gKeys[key] = true;
	if ('1' <= key && key <= '8') 
	{
		unsigned index = key - '0' - 1;
		if (index < gSamples.size())
		{
			gSamples[gSampleIndex]->cleanup();
			gSampleIndex = index;
			ResetNx();
		}
	}

	if (key == '0') 
	{
		gSamples[gSampleIndex]->cleanup();
		gSampleIndex = (gSampleIndex + 1) % gSamples.size();
		ResetNx();
	}

	switch (key)
	{
	case 'p': { gPause = !gPause; UpdateTime(); break; }
	case 'o': { if (!gPause) gPause = true; RunPhysics(); glutPostRedisplay(); break; }
	case 'x': { gShadows = !gShadows; break; }
	case 'n': { gWireframeMode = !gWireframeMode; break; }		
	case ' ': {
		NxActor *sphere = CreateSphere(gCameraPos, 1.0f, 1.0f);
		sphere->setLinearVelocity(gCameraForward * 20.0f);
		break; 
			  }
#ifndef NX_DISABLE_HARDWARE
	case 'h' : {
		gHardwareSimulation = !gHardwareSimulation;
		ResetNx();
		break;
			   }
#endif
	case 27 : { exit(0); break; }
	default : { break; }
	}

	gSamples[gSampleIndex]->onKeyPress(key, x, y);
}

// ------------------------------------------------------------------------------------

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

}

// ------------------------------------------------------------------------------------

void SpecialCallback(int key, int x, int y)
{
	switch (key)
    {
		// Reset PhysX
		case GLUT_KEY_F9: toggleVSync(); return;
		case GLUT_KEY_F10: ResetNx(); return; 
		case GLUT_KEY_F1: 
			gHelp = !gHelp;
			break;
		case GLUT_KEY_F3:
			gMouseDepth += 0.02f * (1.0f - gMouseDepth); 
			WaitForPhysics();
			MoveActor(mx,my);
			break;
		case GLUT_KEY_F4:
			gMouseDepth -= 0.02f * (1.0f - gMouseDepth);  
			if (gMouseDepth < 0.0f) gMouseDepth = 0.0f;
			WaitForPhysics();
			MoveActor(mx,my);
			break;
		case GLUT_KEY_F5:
			float debugRendering = gPhysicsSDK->getParameter(NX_VISUALIZATION_SCALE);
			debugRendering = 1.0f - debugRendering;
			WaitForPhysics();
			gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, debugRendering);
			break;
	}
}

// ------------------------------------------------------------------------------------

void MouseCallback(int button, int state, int x, int y)
{
    mx = x;
	my = y;

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
	{
#if !defined(_XBOX) && !defined (__CELLOS_LV2__) // no picking on consoles
		WaitForPhysics();
		PickActor(x,y);
#endif
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		gRotateCamera = true;
	}
	if (state == GLUT_UP) 
	{
		WaitForPhysics();
		LetGoActor();
		gRotateCamera = false;
	}

    mx = x;
	my = y;
}

// ------------------------------------------------------------------------------------

void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;
    
	if (gMouseJoint)
	{
		WaitForPhysics();
		MoveActor(x,y);
	}
	else if (gRotateCamera)
	{
		gCameraForward.normalize();
		gCameraRight.cross(gCameraForward,NxVec3(0,1,0));

		NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
		qx.rotate(gCameraForward);
		NxQuat qy(NxPiF32 * dy * 20 / 180.0f, gCameraRight);
		qy.rotate(gCameraForward);
	}

    mx = x;
    my = y;
}

// ------------------------------------------------------------------------------------

void InitGlut(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("Particle Fluid Sample");
	glutSetWindow(gMainHandle);
    glutDisplayFunc(RenderCallback);
    glutReshapeFunc(ReshapeCallback);
    glutIdleFunc(IdleCallback);
    glutKeyboardFunc(KeyboardCallback);
    glutKeyboardUpFunc(KeyboardUpCallback);
	glutSpecialFunc(SpecialCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);
    MotionCallback(0,0);

    // Setup default render states
	glClearColor(0.52f, 0.60f, 0.71f, 1.0f);  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
#ifdef WIN32
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
#endif

    // Setup lighting
    glEnable(GL_LIGHTING);
//    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
//    float DiffuseColor[]    = { 0.8f, 0.8f, 0.8f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
//    float SpecularColor[]   = { 0.7f, 0.7f, 0.7f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);

    float AmbientColor[]    = { 0.0f, 0.0f, 0.0f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.6f, 0.6f, 0.6f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.7f, 0.7f, 0.7f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);

	
	//    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    float Position[]        = { 10.0f, 200.0f, 15.0f, 0.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

// ------------------------------------------------------------------------------------

