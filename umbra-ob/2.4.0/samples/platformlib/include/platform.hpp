#ifndef __PLATFORM_HPP
#define __PLATFORM_HPP

/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief       Platform abstraction.
 * 
 *//*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <vector>

#if _XBOX_VER >= 200 
#	define UMBRA_PLATFORM_XBOX360
#elif defined( _WIN64 )
#	define UMBRA_PLATFORM_WIN64
#	define UMBRA_PLATFORM_WIN
#elif defined( _WIN32 )
#	define UMBRA_PLATFORM_WIN32
#	define UMBRA_PLATFORM_WIN
#elif defined( __CELLOS_LV2__ )
#	define UMBRA_PLATFORM_PS3
#elif defined(__APPLE__)
#	define UMBRA_PLATFORM_MAC
#else
#	error unknown platform
#endif

#if defined(UMBRA_PLATFORM_XBOX360)
typedef struct _D3DRECT D3DRECT;
#   define NOMINMAX
#endif

#if !defined(PLATFORM_ASSERT)
#   include <assert.h>
#   define PLATFORM_ASSERT assert
#endif

namespace Umbra
{
	class Vector3;
	class Vector3i;
}

/*-------------------------------------------------------------------*//*!
 * \brief           Platform API abstraction.
 *
 *					The platform library is a simple API abstraction
 *					that's implemented for DX9, DX10, Xbox360 and PS3.
 *					It implements e.g. rendering, vertex and index 
 *					buffers, occlusion queries, simple state management
 *					and system concepts such as mutexes, semaphores and
 *					threads.
 *					
 *					The platform library contains no Occlusion Booster
 *					integration code.
 *
 *//*-------------------------------------------------------------------*/

namespace Platform
{
	class Object;
	class Mesh;
	struct Vector3;
	struct Vector3i;
	struct Vector4;
	class Matrix4x4;

    typedef enum
    {
        EVENT_UPDATE_OBJECTS    = 0,
        EVENT_COLOR_PASS        = 1,
        EVENT_UPDATE_OBJECT     = 2
    } NamedEvent;

	/*---------------------------------------------------------------*//*!
	 * \brief          Video mode struct.
	 *//*---------------------------------------------------------------*/

	struct VideoMode 
	{
		VideoMode() :
			width(1280),
			height(720),
			windowed(true),
			multisamples(1)
		{}

		VideoMode(unsigned int w, unsigned int h, bool win = true, unsigned int m = 1) :
			width(w),
			height(h),
			windowed(win),
			multisamples(m)
		{}

		unsigned int	width;			/*!< Video mode width. */
		unsigned int	height;			/*!< Video mode height */
		bool			windowed;		/*!< True, if windowed. */
		unsigned int	multisamples;	/*!< Number of multisamples */

	};

  	/*---------------------------------------------------------------*//*!
	 * \brief          Shader abstraction
	 *//*---------------------------------------------------------------*/

    class Shader
    {
    public:
                        Shader          (void);
        virtual         ~Shader         (void) {}

                bool    init            (void);

        virtual void    preRender       (Object* object) = 0;
        virtual void    postRender      (Object* object) = 0;

    protected:

        virtual bool    doinit          (void* platformHandle) 
        {
            (void)platformHandle;
            return true;
        }
        
    private:

        bool            m_initialized;

    };

	// Platform specific types. 
	// The internals are not exposed to application code.

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform vertex buffer implementation.
	 *//*---------------------------------------------------------------*/
	class VertexBuffer;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform index buffer implementation.
	 *//*---------------------------------------------------------------*/
	class IndexBuffer;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform thread implementation.
	 *//*---------------------------------------------------------------*/
	class Thread;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform mutex implementation.
	 *//*---------------------------------------------------------------*/
	class Mutex;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform semaphore implementation.
	 *//*---------------------------------------------------------------*/
	class Semaphore;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform occlusion query implementation.
	 *//*---------------------------------------------------------------*/
	class Query;

	/*---------------------------------------------------------------*//*!
	 * \brief          Platform event implementation.
	 *//*---------------------------------------------------------------*/
	class Event;

#ifndef SPU

	// Initialization and deinitialization
	// Video mode parameter only used on PC
	bool    init                    (const VideoMode &mode, void* context);
    void    exit                    (void);

#if defined(UMBRA_PLATFORM_XBOX360)
	void			configureTiling	(const D3DRECT* tiles, int numTiles);
	
	const D3DRECT*	getTiles		();
	int				getNumTiles		();

	void			beginTile		(int tile, float red, float green, float blue, float alpha, float depth, float stencil);
	void			endTile			();

#endif

    void*   getSpursContext         (void);
    void*   getRenderContext        (void);

	std::vector<std::string>&
			getCommandLine		    (void);
	void	setCommandLine			(const std::vector<std::string> &args);

#endif

    // Input handling

	// Alphabet values for keyboard input handling.
	// These can be used to interpret KeyboardState::alphabetBitmap,
	// obtained by calling readKeyboard.
	enum KeyboardAlphabet
    {
        KEY_A       = (1<<0),
        KEY_B       = (1<<1),
        KEY_C       = (1<<2),
        KEY_D       = (1<<3),
        KEY_E       = (1<<4),
        KEY_F       = (1<<5),
        KEY_G       = (1<<6),
        KEY_H       = (1<<7),
        KEY_I       = (1<<8),
        KEY_J       = (1<<9),
        KEY_K       = (1<<10),
        KEY_L       = (1<<11),
        KEY_M       = (1<<12),
        KEY_N       = (1<<13),
        KEY_O       = (1<<14),
        KEY_P       = (1<<15),
        KEY_Q       = (1<<16),
        KEY_R       = (1<<17),
        KEY_S       = (1<<18),
        KEY_T       = (1<<19),
        KEY_U       = (1<<20),
        KEY_V       = (1<<21),
        KEY_W       = (1<<22),
        KEY_X       = (1<<23),
        KEY_Y       = (1<<24),
        KEY_Z       = (1<<25)
    };

	// Special values for keyboard input handling.
	// These can be used to interpret KeyboardState::symbolsBitmap,
	// obtained by calling readKeyboard.
    enum KeyboardSymbol
    {
        KEY_0       = (1<<0),
        KEY_1       = (1<<1),
        KEY_2       = (1<<2),
        KEY_3       = (1<<3),
        KEY_4       = (1<<4),
        KEY_5       = (1<<5),
        KEY_6       = (1<<6),
        KEY_7       = (1<<7),
        KEY_8       = (1<<8),
        KEY_9       = (1<<9),
        KEY_ENTER   = (1<<10),
        KEY_SPACE   = (1<<11),
        KEY_ESC     = (1<<12),
        KEY_UP      = (1<<13),
        KEY_DOWN    = (1<<14),
        KEY_LEFT    = (1<<15),
        KEY_RIGHT   = (1<<16),
        KEY_PLUS    = (1<<17),
        KEY_MINUS   = (1<<18)
    };

	// Special values for mouse button handling.
	// These can be used to interpret MouseState::buttonBitmap,
	// obtained by calling readMouse.
    enum MouseSymbol
	{
		MOUSE_LEFTBUTTON	= (1<<0),
		MOUSE_RIGHTBUTTON	= (1<<1)
	};

	// Button values for gamepad input handling.
	// These can be used to interpret PadState::keyBitmap,
	// obtained by calling readPad.
	enum ConsolePad
    {
        // DPAD buttons on PS3 and XBox360
        BUTTON_LUP          = (1<<1),
        BUTTON_LDOWN        = (1<<2),
        BUTTON_LLEFT        = (1<<3),
        BUTTON_LRIGHT       = (1<<4),

        // Y,A,X,B on XBox360
        // Triangle, cross, square, circle on PS3
        BUTTON_RUP          = (1<<5),
        BUTTON_RDOWN        = (1<<6),
        BUTTON_RLEFT        = (1<<7),
        BUTTON_RRIGHT       = (1<<8),

        // Start and back on XBox360
        // Start and select on PS3
        BUTTON_START        = (1<<9),
        BUTTON_BACK         = (1<<10),

        // LB, LT on XBox360
        // L1, L2 on PS3
        BUTTON_L1           = (1<<11),
        BUTTON_L2           = (1<<12),

        // RB, RT on XBox360
        // R1, R2 on PS3
        BUTTON_R1           = (1<<13),
        BUTTON_R2           = (1<<14),

        // Left thumb stick as digital value
        THUMBR_UP           = (1<<15),
        THUMBR_DOWN         = (1<<16),
        THUMBR_LEFT         = (1<<17),
        THUMBR_RIGHT        = (1<<18),

        // Right thumb stick as digital value
        THUMBL_UP           = (1<<19),
        THUMBL_DOWN         = (1<<20),
        THUMBL_LEFT         = (1<<21),
        THUMBL_RIGHT        = (1<<22),

        // Left and right thumb sticks pressed down
        BUTTON_THUMBR       = (1<<23),
        BUTTON_THUMBL       = (1<<24)
    };

	/*---------------------------------------------------------------*//*!
	 * \brief          Object to hold keyboard state.
	 *//*---------------------------------------------------------------*/
    struct KeyboardState {
        KeyboardState() :
            alphabetBitmap(0),
            symbolsBitmap(0) 
            {}

        unsigned int alphabetBitmap;
        unsigned int symbolsBitmap;
    };

	/*---------------------------------------------------------------*//*!
	 * \brief          Object to hold keyboard state.
	 *//*---------------------------------------------------------------*/
    struct MouseState {
        MouseState() :
            x(0),
            y(0),
			buttonBitmap(0)
            {}

		unsigned int x;
		unsigned int y;
        unsigned int buttonBitmap;
    };

	/*---------------------------------------------------------------*//*!
	 * \brief          Object to hold gamepad state.
	 *//*---------------------------------------------------------------*/
    struct PadState {
       PadState() :
            keyBitmap(0),
            analogThumbRX(0.f), 
            analogThumbRY(0.f),
            analogThumbLX(0.f),
            analogThumbLY(0.f),
            analogL2(0.f),
            analogR2(0.f)
            {}

        unsigned int keyBitmap;
        float analogThumbRX;
        float analogThumbRY;
        float analogThumbLX;
        float analogThumbLY;
        float analogL2;
        float analogR2;
    };

	// Reads keyboard state, populating a KeyboardState object.
	// Not supported on console platforms.
    void        readKeyboard            (KeyboardState& keyboardState);

	// Reads mouse mouse state, populating a MouseState object.
	// Not supported on console platforms.
    void        readMouse               (MouseState& mouseState);

	// Centers mouse position in the middle of window.
	// Outputs new coordinates using a MouseState object.
	// Not supported on console platforms.
	void        centerMouse             (MouseState& mouseState);

	// Reads gamepad state, populating a PadState object.
	// Supported on all platforms (including windows)
    void        readPad                 (PadState& padState);

    // Functions for occlusion queries

    Query*      allocateQuery           (void);
    void        releaseQuery            (Query*);
	void        beginQuery				(Query*); 
	void        endQuery                (Query*); 
	bool        getQueryResult          (int& visibleSamples, Query* queryObject, bool waitForResult); 

    // Matrix setter functions

	void 	    setViewMatrix			(const Matrix4x4& matrix);
	void 	    setProjectionMatrix		(const Matrix4x4& matrix); 
	void        setWorldMatrix          (const Matrix4x4& matrix);

    // Render state

    enum State
	{
		COLOR_DRAW_STATE,		/*!< State for color draw */
		DEPTH_DRAW_STATE,		/*!< State for depth draw */
		DEPTH_TEST_STATE,		/*!< State for depth test */
		WIREFRAME_DRAW_STATE,	/*!< State for wireframe draw */	
		DEBUGLINE_DRAW_STATE,	/*!< State for drawing debug lines */	
        INVALID_STATE
	};

	// Set rendering state
	void        setState                (State s);

    // Drawing functions

    void        clear                   (float red, float green, float blue, float alpha, float depth, float stencil);
	void        clearDepthStencil       (void); 

    void        setViewport             (int x, int y, int w, int h);

    void        draw                    (Object** objects, unsigned int objectCount);
    void        draw                    (Object** objects, unsigned int objectCount, Shader* shader);
    void        drawIndexed             (int vertexOffset, int vertexCount, int triangleCount, int start);  
	void		drawIndexedUP			(const Vector3* vertices, int vertexCount, const Vector3i* triangles, int triangleCount);
    void		drawLines				(const Vector3* lines, int nLines, const Vector4& color); 
    void		drawLinesScreenSpace	(const Vector3* lines, int nLines, const Vector4& color); 

    void        setupQueryDraw          (void);

    // Display functions

    void        getScreenSize           (unsigned int& width, unsigned int& height, bool& windowed);

    // Functions for platform mesh management

    Mesh*       createPlatformMesh      (const Vector3* vertices, const Vector3* normals, 
                                         const Vector3i* triangles, int nVertices, int nTriangles);
    void        releasePlatformMesh     (Mesh* mesh);

    // Text output

	void		fontMeasure				(int &width, int &height, const char* text, ...);
	void		fontPrint				(int x, int y, const Vector4& color, const char* text, ...);

    // Misc control functions

    bool        beginFrame              (void);
    void        endFrame                (void);
    void        flush                   (void);
    void        finish                  (void);

    // Profiling functions

    void        beginNamedEvent         (const char* str);
	void        endNamedEvent           (void);
    void        setNamedCounter         (float c, const char* str);

    // Threading functions

    float       getTime                 (void);		// in milliseconds
    void        sleep                   (int ms);

    Thread*     beginLocalThread        (int coreID, void* startAddress, void* arg1);

    Thread*     beginThread             (void* (*threadMainFunc)(void*), void* args); // \todo typedef
    void        waitThreadToComplete    (Thread* thread);

    int         getCurrentThreadID      (void);

    Mutex*      createMutex             (void);
    void        destroyMutex            (Mutex* mutex);
    void        enterMutex              (Mutex* mutex);
    void        leaveMutex              (Mutex* mutex);

    Semaphore*  createSemaphore         (int initialCount);
    void        deleteSemaphore         (Semaphore*);
    void        incSemaphore            (Semaphore*);
    bool        decSemaphore            (Semaphore*, bool wait);

    Event*      createEvent             (void);
    void        deleteEvent             (Event*);
    void        sendEvent               (Event*);
    void        waitEvent               (Event*);

    void        attachEventThread       (Event*);
    void        detachEventThread       (Event*);

    void        bindGraphicsThread      (void);
    void        unbindGraphicsThread    (void);

    int         atomicAdd               (volatile int* value, int a);
    int         atomicSub               (volatile int* value, int a);
    int         atomicStore             (volatile int* value, int a);
    int         atomicGet               (volatile int* value);

    void        putData                 (void* dst, const void* src, int s);
    void        putDataUnaligned        (void* dst, const void* src, int s);
    void        getDataUnaligned        (void* dst, const void* src, int s);

    void        putDataAlignedStart     (void* dst, const void* src, int s, int tag);

    void        getDataAlignedStart     (void* dst, const void* src, int s, int tag);
    void        getDataWait             (int tag);

    void        putDataWait             (int tag);
    
    // Vertex buffer operations
	VertexBuffer*	createVertexBuffer      (int size);
    void			deleteVertexBuffer      (VertexBuffer* buffer);
	void*			lockVertexBuffer	    (VertexBuffer* buffer, int offset, int size);
	void			unlockVertexBuffer      (VertexBuffer* buffer);

	// Index buffer operations
    IndexBuffer*	createIndexBuffer       (int size);
    void			deleteIndexBuffer       (IndexBuffer* buffer);
	void*		    lockIndexBuffer         (IndexBuffer* buffer, int offset, int size);
	void			unlockIndexBuffer       (IndexBuffer* buffer);

    void        setIndexBuffer          (IndexBuffer* indexBuffer);
    void        setVertexBuffer         (VertexBuffer* vertexBuffer);

#if defined(_MSC_VER)

	inline int safe_sprintf( char *buffer, std::size_t bufferSize, const char *format, ...)
	{
		va_list args;
		va_start( args, format );

		return vsprintf_s(buffer, bufferSize, format, args);
	}

	inline int safe_sscanf( char *buffer, std::size_t bufferSize, const char *format, ...)
	{
		va_list args;
		va_start( args, format );

		return vsprintf_s(buffer, bufferSize, format, args);
	}

#elif defined(__GNUC__)
	
	inline int safe_sprintf( char *buffer, std::size_t bufferSize, const char *format, ...)
	{
		va_list args;
		va_start( args, format );

		return vsnprintf(buffer, bufferSize, format, args);
	}

#else
	
	#error safe_sprintf not implemented for this platform

#endif

} // namespace Platform

// Application callback functions.

bool getVideoMode	   (Platform::VideoMode& videoMode);
bool initApplication   (void);
bool updateApplication (void);
void exitApplication   (void);

//------------------------------------------------------------------------
#endif

