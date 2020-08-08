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
 * \brief     Platform abstraction for OpenGL.
 * 
 *//*-------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>
#include <assert.h>
#include <map>
#include <xinput.h>

#include <algorithm>
#include <sstream>

#include "platform.hpp"
#include "plObject.hpp"
#include "plMesh.hpp"

using namespace Platform;

/*-------------------------------------------------------------------*//*!
 * Internal types.
 * In practice these classes are wrappers of the internal pointer types.
 *//*-------------------------------------------------------------------*/

namespace Platform 
{
	class VertexBuffer
	{
	public:
		VertexBuffer(GLuint value) 
			: platformBuffer(value) {}

		GLuint platformBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(GLuint value) 
			: platformBuffer(value) {}

		GLuint platformBuffer;
	};

	class Query
	{
	public:
		Query(GLuint value) 
			: platformQuery(value) {}

		GLuint platformQuery;
	};

	class Thread
	{
	public:
		Thread(HANDLE value) 
			: handle(value) {}

		HANDLE handle;
	};

	class Mutex
	{
	public:
		Mutex() {}

		CRITICAL_SECTION criticalSection;
	};

	class Semaphore
	{
	public:
		Semaphore(HANDLE value)
			: handle(value) {}

		HANDLE handle;
	};
}

/*-------------------------------------------------------------------*//*!
 * Vertex shader for color pass.
 *//*-------------------------------------------------------------------*/

static GLuint   g_colorVertexShader  = 0;
static GLuint   g_colorShaderProgram = 0;

const char*     g_strColorVertexShaderProgram = 
"varying vec3 normal;\n                                     "
"                                                           "
"void main(void)                                            "
"{                                                          "  
"   normal = normalize(gl_NormalMatrix * gl_Normal);        "
"   gl_FrontColor = gl_Color;                               "
"	gl_Position   = ftransform();                           "
"}                                                          ";

/*-------------------------------------------------------------------*//*!
 * Fragment shader for color pass.
 *//*-------------------------------------------------------------------*/

static GLuint   g_colorFragmentShader = 0;

const char*     g_strColorFragmentShaderProgram =
"varying vec3 normal;\n                                     "
"                                                           "
"void main()                                                "
"{                                                          "
"   vec3 lightDir = vec3(0.f,0.f,-1.f);                     "
"                                                           "
"   float c;                                                "
"   c = max(0.0, dot(lightDir, normalize(normal)));         "	
"   c = min(c, 1.0);                                        "
"                                                           "
"   gl_FragColor = gl_Color * vec4(c, c, c, 1);             "
"}                                                          ";

/*-------------------------------------------------------------------*//*!
 * Globals
 *//*-------------------------------------------------------------------*/

static int  g_renderWidth  = 0;
static int  g_renderHeight = 0;
static bool g_windowed     = true;

static Platform::State g_state;

HWND g_hWnd;

static std::vector<std::string> g_args;

// Keyboard alphabet state
unsigned int keyboardAlphabetBitmap = 0;
// Keyboard symbol state
unsigned int keyboardSymbolsBitmap = 0;

// Mapping from windows virtual keycode to Platform symbol constant
std::map<unsigned int, Platform::KeyboardSymbol> symbolVirtualKeys;

// Mapping from windows virtual keycode to Platform alphabet constant
std::map<unsigned int, Platform::KeyboardAlphabet> alphabetVirtualKeys;

// Font for drawing text
GLuint                      g_font			= 0; 
// Font height in pixels
const int					g_fontHeight	= 15;

// Mouse button state
unsigned int mouseButtonBitmap	= 0;

// True if window is active
bool windowActive = false;

/*-------------------------------------------------------------------*//*!
 * DirectX globals.
 *//*-------------------------------------------------------------------*/

class OpenGLGlobals
{
public:

    // Occlusion Query interface

    typedef void        (WINAPI* PFNGLGENQUERIESARBPROC)            (GLsizei n, GLuint *ids);
    typedef void        (WINAPI* PFNGLDELETEQUERIESARBPROC)         (GLsizei n, const GLuint *ids);
    typedef GLboolean   (WINAPI* PFNGLISQUERYARBPROC)               (GLuint id);
    typedef void        (WINAPI* PFNGLBEGINQUERYARBPROC)            (GLenum target, GLuint id);
    typedef void        (WINAPI* PFNGLENDQUERYARBPROC)              (GLenum target);
    typedef void        (WINAPI* PFNGLGETQUERYIVARBPROC)            (GLenum target, GLenum pname, GLint *params);
    typedef void        (WINAPI* PFNGLGETQUERYOBJECTIVARBPROC)      (GLuint id, GLenum pname, GLint *params);
    typedef void        (WINAPI* PFNGLGETQUERYOBJECTUIVARBPROC)         (GLuint id, GLenum pname, GLuint *params);

    static              PFNGLGENQUERIESARBPROC                      glGenQueries;
    static              PFNGLDELETEQUERIESARBPROC                   glDeleteQueries;
    static              PFNGLISQUERYARBPROC                         glIsQuery;
    static              PFNGLBEGINQUERYARBPROC                      glBeginQuery;
    static              PFNGLENDQUERYARBPROC                        glEndQuery;
    static              PFNGLGETQUERYIVARBPROC                      glGetQueryiv;
    static              PFNGLGETQUERYOBJECTIVARBPROC                glGetQueryObjectiv;
    static              PFNGLGETQUERYOBJECTUIVARBPROC               glGetQueryObjectuiv;

    // Vertex buffer object interface

    typedef void        (WINAPI* PFNGLBINDBUFFERARBPROC)            (GLenum target, GLuint buffer);
    typedef void        (WINAPI* PFNGLDELETEBUFFERSARBPROC)         (GLsizei n, const GLuint *buffers);
    typedef void        (WINAPI* PFNGLGENBUFFERSARBPROC)            (GLsizei n, GLuint *buffers);
    typedef GLboolean   (WINAPI* PFNGLISBUFFERARBPROC)              (GLuint buffer);
    typedef void        (WINAPI* PFNGLBUFFERDATAARBPROC)            (GLenum target, ptrdiff_t size, const GLvoid *data, GLenum usage);
    typedef void        (WINAPI* PFNGLBUFFERSUBDATAARBPROC)         (GLenum target, ptrdiff_t offset, ptrdiff_t size, const GLvoid *data);
    typedef void        (WINAPI* PFNGLGETBUFFERSUBDATAARBPROC)      (GLenum target, ptrdiff_t offset, ptrdiff_t size, GLvoid *data);
    typedef GLvoid*     (WINAPI* PFNGLMAPBUFFERARBPROC)             (GLenum target, GLenum access);
    typedef GLboolean   (WINAPI* PFNGLUNMAPBUFFERARBPROC)           (GLenum target);
    typedef void        (WINAPI* PFNGLGETBUFFERPARAMETERIVARBPROC)  (GLenum target, GLenum pname, GLint *params);
    typedef void        (WINAPI* PFNGLGETBUFFERPOINTERVARBPROC)     (GLenum target, GLenum pname, GLvoid* *params);

    static              PFNGLBINDBUFFERARBPROC                      glBindBuffer;
    static              PFNGLDELETEBUFFERSARBPROC                   glDeleteBuffers;
    static              PFNGLGENBUFFERSARBPROC                      glGenBuffers;
    static              PFNGLISBUFFERARBPROC                        glIsBuffer;
    static              PFNGLBUFFERDATAARBPROC                      glBufferData;
    static              PFNGLBUFFERSUBDATAARBPROC                   glBufferSubData;
    static              PFNGLGETBUFFERSUBDATAARBPROC                glGetBufferSubData;
    static              PFNGLMAPBUFFERARBPROC                       glMapBuffer;
    static              PFNGLUNMAPBUFFERARBPROC                     glUnmapBuffer;
    static              PFNGLGETBUFFERPARAMETERIVARBPROC            glGetBufferParameteriv;
    static              PFNGLGETBUFFERPOINTERVARBPROC               glGetBufferPointerv;

    // Shader interface

    typedef void        (WINAPI* PFNGLDELETEOBJECTARBPROC)          (GLuint obj);
    typedef GLuint      (WINAPI* PFNGLGETHANDLEARBPROC)             (GLenum pname);
    typedef void        (WINAPI* PFNGLDETACHOBJECTARBPROC)          (GLuint containerObj, GLuint attachedObj);
    typedef GLuint      (WINAPI* PFNGLCREATESHADEROBJECTARBPROC)    (GLenum shaderType);
    typedef void        (WINAPI* PFNGLSHADERSOURCEARBPROC)          (GLuint shaderObj, GLsizei count, const char* *string, const GLint *length);
    typedef void        (WINAPI* PFNGLCOMPILESHADERARBPROC)         (GLuint shaderObj);
    typedef GLuint      (WINAPI* PFNGLCREATEPROGRAMOBJECTARBPROC)   (void);
    typedef void        (WINAPI* PFNGLATTACHOBJECTARBPROC)          (GLuint containerObj, GLuint obj);
    typedef void        (WINAPI* PFNGLLINKPROGRAMARBPROC)           (GLuint programObj);
    typedef void        (WINAPI* PFNGLUSEPROGRAMOBJECTARBPROC)      (GLuint programObj);
    typedef void        (WINAPI* PFNGLVALIDATEPROGRAMARBPROC)       (GLuint programObj);
    typedef void        (WINAPI* PFNGLGETOBJECTPARAMETERIVARBPROC)  (GLuint obj, GLenum pname, GLint *params);
    typedef void        (WINAPI* PFNGLGETINFOLOGARBPROC)            (GLuint obj, GLsizei maxLength, GLsizei *length, char *infoLog);

    static              PFNGLDELETEOBJECTARBPROC                    glDeleteObject;
    static              PFNGLGETHANDLEARBPROC                       glGetHandle;
    static              PFNGLDETACHOBJECTARBPROC                    glDetachObject;
    static              PFNGLCREATESHADEROBJECTARBPROC              glCreateShaderObject;
    static              PFNGLSHADERSOURCEARBPROC                    glShaderSource;
    static              PFNGLCOMPILESHADERARBPROC                   glCompileShader;
    static              PFNGLCREATEPROGRAMOBJECTARBPROC             glCreateProgramObject;
    static              PFNGLATTACHOBJECTARBPROC                    glAttachObject;
    static              PFNGLLINKPROGRAMARBPROC                     glLinkProgram;
    static              PFNGLUSEPROGRAMOBJECTARBPROC                glUseProgramObject;
    static              PFNGLGETOBJECTPARAMETERIVARBPROC            glGetObjectParameteriv;
    static              PFNGLGETINFOLOGARBPROC                      glGetInfoLog;

    // GL constants interface

    static const unsigned int                                       GL_FRAGMENT_SHADER              = 0x8B30;
    static const unsigned int                                       GL_VERTEX_SHADER                = 0x8B31;
    static const unsigned int                                       GL_STATIC_DRAW                  = 0x88E4;
    static const unsigned int                                       GL_ARRAY_BUFFER                 = 0x8892;
    static const unsigned int                                       GL_ELEMENT_ARRAY_BUFFER         = 0x8893;
    static const unsigned int                                       GL_WRITE_ONLY                   = 0x88B9;
    static const unsigned int                                       GL_SAMPLES_PASSED               = 0x8914;
    static const unsigned int                                       GL_QUERY_RESULT_AVAILABLE       = 0x8867;
    static const unsigned int                                       GL_QUERY_RESULT                 = 0x8866;
    static const unsigned int                                       GL_INFO_LOG_LENGTH              = 0x8B84;
    static const unsigned int                                       GL_OBJECT_COMPILE_STATUS        = 0x8B81;
    static const unsigned int                                       GL_OBJECT_LINK_STATUS           = 0x8B82;


    static HDC                          getDeviceContext(void)                      { PLATFORM_ASSERT(s_hDC); return s_hDC; }
    static void                         setDeviceContext(HDC hDC)                   { s_hDC = hDC; }

    static HGLRC                        getRenderingContext(void)                   { PLATFORM_ASSERT(s_hRC); return s_hRC; }
    static void                         setRenderingContext(HGLRC hRC)              { s_hRC = hRC; }

    static const Matrix4x4&             getViewMatrix (void)                        { return s_matView; }
    static void                         setViewMatrix (const Matrix4x4& matrix)     { s_matView = matrix; } 

    static const Matrix4x4&             getProjMatrix (void)                        { return s_matProj; }
    static void                         setProjMatrix (const Matrix4x4& matrix)     { s_matProj = matrix; } 

    static const Matrix4x4&             getWorldMatrix (void)                       { return s_matWorld; }
    static void                         setWorldMatrix (const Matrix4x4& matrix)    { s_matWorld = matrix; } 

    static bool                         getIndexBufferLocked (void)                 { return s_indexBufferLocked; }
    static void                         setIndexBufferLocked (bool state)           { s_indexBufferLocked = state; }

    static bool                         getVertexBufferLocked (void)                { return s_vertexBufferLocked; }
    static void                         setVertexBufferLocked (bool state)          { s_vertexBufferLocked = state; }

private:

	OpenGLGlobals();

    static Matrix4x4            s_matView;              // View matrix 
    static Matrix4x4            s_matProj;              // Projection matrix
    static Matrix4x4            s_matWorld;             // World matrix	

    static HDC				    s_hDC;	                // Device context handle
    static HGLRC		    	s_hRC;                  // Rendering context handle

    static bool                 s_vertexBufferLocked;   // True when vertex buffer is locked
    static bool                 s_indexBufferLocked;    // True when index buffer is locked

};

//--------------------------------------------------------------------
// Initializing the DirectX globals
//--------------------------------------------------------------------

Matrix4x4                       OpenGLGlobals::s_matView;
Matrix4x4                       OpenGLGlobals::s_matProj;
Matrix4x4                       OpenGLGlobals::s_matWorld;

OpenGLGlobals::PFNGLGENQUERIESARBPROC           OpenGLGlobals::glGenQueries             = NULL;
OpenGLGlobals::PFNGLDELETEQUERIESARBPROC        OpenGLGlobals::glDeleteQueries          = NULL;
OpenGLGlobals::PFNGLISQUERYARBPROC              OpenGLGlobals::glIsQuery                = NULL;
OpenGLGlobals::PFNGLBEGINQUERYARBPROC           OpenGLGlobals::glBeginQuery             = NULL;
OpenGLGlobals::PFNGLENDQUERYARBPROC             OpenGLGlobals::glEndQuery               = NULL;
OpenGLGlobals::PFNGLGETQUERYIVARBPROC           OpenGLGlobals::glGetQueryiv             = NULL;
OpenGLGlobals::PFNGLGETQUERYOBJECTIVARBPROC     OpenGLGlobals::glGetQueryObjectiv       = NULL;
OpenGLGlobals::PFNGLGETQUERYOBJECTUIVARBPROC    OpenGLGlobals::glGetQueryObjectuiv      = NULL;

OpenGLGlobals::PFNGLBINDBUFFERARBPROC           OpenGLGlobals::glBindBuffer             = NULL;
OpenGLGlobals::PFNGLDELETEBUFFERSARBPROC        OpenGLGlobals::glDeleteBuffers          = NULL;
OpenGLGlobals::PFNGLGENBUFFERSARBPROC           OpenGLGlobals::glGenBuffers             = NULL;
OpenGLGlobals::PFNGLISBUFFERARBPROC             OpenGLGlobals::glIsBuffer               = NULL;
OpenGLGlobals::PFNGLBUFFERDATAARBPROC           OpenGLGlobals::glBufferData             = NULL;
OpenGLGlobals::PFNGLBUFFERSUBDATAARBPROC        OpenGLGlobals::glBufferSubData          = NULL;
OpenGLGlobals::PFNGLGETBUFFERSUBDATAARBPROC     OpenGLGlobals::glGetBufferSubData       = NULL;
OpenGLGlobals::PFNGLMAPBUFFERARBPROC            OpenGLGlobals::glMapBuffer              = NULL;
OpenGLGlobals::PFNGLUNMAPBUFFERARBPROC          OpenGLGlobals::glUnmapBuffer            = NULL;
OpenGLGlobals::PFNGLGETBUFFERPARAMETERIVARBPROC OpenGLGlobals::glGetBufferParameteriv   = NULL;
OpenGLGlobals::PFNGLGETBUFFERPOINTERVARBPROC    OpenGLGlobals::glGetBufferPointerv      = NULL;

OpenGLGlobals::PFNGLDELETEOBJECTARBPROC         OpenGLGlobals::glDeleteObject           = NULL;
OpenGLGlobals::PFNGLGETHANDLEARBPROC            OpenGLGlobals::glGetHandle              = NULL;
OpenGLGlobals::PFNGLDETACHOBJECTARBPROC         OpenGLGlobals::glDetachObject           = NULL;
OpenGLGlobals::PFNGLCREATESHADEROBJECTARBPROC   OpenGLGlobals::glCreateShaderObject     = NULL;
OpenGLGlobals::PFNGLSHADERSOURCEARBPROC         OpenGLGlobals::glShaderSource           = NULL;
OpenGLGlobals::PFNGLCOMPILESHADERARBPROC        OpenGLGlobals::glCompileShader          = NULL;
OpenGLGlobals::PFNGLCREATEPROGRAMOBJECTARBPROC  OpenGLGlobals::glCreateProgramObject    = NULL;
OpenGLGlobals::PFNGLATTACHOBJECTARBPROC         OpenGLGlobals::glAttachObject           = NULL;
OpenGLGlobals::PFNGLLINKPROGRAMARBPROC          OpenGLGlobals::glLinkProgram            = NULL;
OpenGLGlobals::PFNGLUSEPROGRAMOBJECTARBPROC     OpenGLGlobals::glUseProgramObject       = NULL;
OpenGLGlobals::PFNGLGETOBJECTPARAMETERIVARBPROC OpenGLGlobals::glGetObjectParameteriv   = NULL;
OpenGLGlobals::PFNGLGETINFOLOGARBPROC           OpenGLGlobals::glGetInfoLog             = NULL;

HDC					                            OpenGLGlobals::s_hDC                    = NULL;
HGLRC				                            OpenGLGlobals::s_hRC                    = NULL;

bool                                            OpenGLGlobals::s_vertexBufferLocked     = false;
bool                                            OpenGLGlobals::s_indexBufferLocked      = false;

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Allocates an occlusion query.
 *
 *//*-------------------------------------------------------------------*/

Query* Platform::allocateQuery (void)
{
    GLuint queryID;

    OpenGLGlobals::glGenQueries(1, &queryID);

    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    return new Query(queryID);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Releases an occlusion query that was created with 
 *			allocateQuery.
 *
 * \param	query	Query object to be released.
 *
 *//*-------------------------------------------------------------------*/

void   Platform::releaseQuery  (Query* query)
{
    OpenGLGlobals::glDeleteQueries(1, &query->platformQuery);

    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

	delete query;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Creates a DX9 vertex buffer.
 *
 * \param   size    The size of the vertex buffer in bytes.
 *
 *//*-------------------------------------------------------------------*/

VertexBuffer* Platform::createVertexBuffer(int size)
{
    PLATFORM_ASSERT(size > 0);

    GLuint vbo;

    OpenGLGlobals::glGenBuffers (1, &vbo);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::glBindBuffer (OpenGLGlobals::GL_ARRAY_BUFFER, vbo);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::glBufferData (OpenGLGlobals::GL_ARRAY_BUFFER, size, NULL, OpenGLGlobals::GL_STATIC_DRAW);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    return new VertexBuffer(vbo);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Deletes a vertex buffer.
 *
 * \param	buffer	Vertex buffer to be deleted
 *
 *//*-------------------------------------------------------------------*/

void Platform::deleteVertexBuffer(VertexBuffer* buffer)
{
    PLATFORM_ASSERT(buffer);

    OpenGLGlobals::glDeleteBuffers (1, &buffer->platformBuffer);
	delete buffer;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Locks a vertex buffer.
 *
 * \param	buffer	Vertex buffer to lock
 * \param	offset	Byte offset to vertex data to be locked
 * \param	size	Byte size of vertex data to lock
 *
 *//*-------------------------------------------------------------------*/

void*  Platform::lockVertexBuffer	(VertexBuffer* buffer, int offset, int /* size */)
{
    PLATFORM_ASSERT(buffer);

    PLATFORM_ASSERT(
        OpenGLGlobals::getVertexBufferLocked() == false && 
        "Only one vertex buffer can be locked at once");

    OpenGLGlobals::glBindBuffer (OpenGLGlobals::GL_ARRAY_BUFFER, buffer->platformBuffer);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    void* data = OpenGLGlobals::glMapBuffer (OpenGLGlobals::GL_ARRAY_BUFFER, OpenGLGlobals::GL_WRITE_ONLY);
    
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );
    PLATFORM_ASSERT( data );

    OpenGLGlobals::setVertexBufferLocked(true);

    return (void*)((char*)data + offset);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Unlocks a vertex buffer.
 *
 * \param	buffer	Vertex buffer to unlock
 *
 *//*-------------------------------------------------------------------*/

void Platform::unlockVertexBuffer(VertexBuffer* /* buffer */)
{
    PLATFORM_ASSERT(OpenGLGlobals::getVertexBufferLocked() == true);
    
    OpenGLGlobals::glUnmapBuffer(OpenGLGlobals::GL_ARRAY_BUFFER);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::setVertexBufferLocked(false);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Creates a DX9 index buffer.
 *
 * \param   size    The size of the index buffer in bytes.
 *
 *//*-------------------------------------------------------------------*/

IndexBuffer* Platform::createIndexBuffer(int size)
{
    PLATFORM_ASSERT(size > 0);

    GLuint ibo;

    OpenGLGlobals::glGenBuffers(1, &ibo);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, ibo);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::glBufferData(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, size, NULL, OpenGLGlobals::GL_STATIC_DRAW);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    return new IndexBuffer(ibo);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Deletes an index buffer.
 *
 * \param	buffer	Index buffer to be deleted
 *
 *//*-------------------------------------------------------------------*/
void Platform::deleteIndexBuffer(IndexBuffer* buffer)
{
    PLATFORM_ASSERT(buffer);

    OpenGLGlobals::glDeleteBuffers(1, &buffer->platformBuffer);
	delete buffer;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Locks an index buffer.
 *
 * \param	buffer	Index buffer to lock
 * \param	offset	Byte offset to index data to be locked
 * \param	size	Byte size of index data to lock
 *
 *//*-------------------------------------------------------------------*/

void* Platform::lockIndexBuffer(IndexBuffer* buffer, int offset, int /* size */)
{
    PLATFORM_ASSERT(buffer);

    PLATFORM_ASSERT(
        OpenGLGlobals::getIndexBufferLocked() == false && 
        "Only one index buffer can be locked at once");

    OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, buffer->platformBuffer);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    void* data = OpenGLGlobals::glMapBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, OpenGLGlobals::GL_WRITE_ONLY);
    
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );
    PLATFORM_ASSERT( data );

    OpenGLGlobals::setIndexBufferLocked(true);

    return (void*)((char*)data + offset);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Unlocks a index buffer.
 *
 * \param	buffer	Index buffer to unlock
 *
 *//*-------------------------------------------------------------------*/

void Platform::unlockIndexBuffer(IndexBuffer* /* buffer */)
{
    PLATFORM_ASSERT(OpenGLGlobals::getIndexBufferLocked() == true);
    
    OpenGLGlobals::glUnmapBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

    OpenGLGlobals::setIndexBufferLocked(false);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief  D3D9 implementation of mesh.
 *
 *//*-------------------------------------------------------------------*/

class OpenGLMesh : public Mesh
{
public:

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Constructor for D3D9 mesh.
	 *
	 *		   Creates vertex and index buffers to hold mesh data,
	 *		   and fills them with the supplied data.
	 *
	 *//*-------------------------------------------------------------------*/

	OpenGLMesh(const Vector3* vertices, const Vector3* normals, const Vector3i* triangles, int nVertices, int nTriangles) :
      m_numTriangles(nTriangles),
      m_numVertices(nVertices)
    {
        int numv = nVertices;
        int nump = nTriangles;

        //--------------------------------------------------------------------
        // Create vertex and index buffer for holding mesh 
        // vertex, normal and index data
        //--------------------------------------------------------------------

        m_vertexBuffer = createVertexBuffer ( numv * sizeof(Vector3)        * 2 );
        m_indexBuffer  = createIndexBuffer  ( nump * sizeof(unsigned short) * 3 );

        PLATFORM_ASSERT( m_vertexBuffer && m_indexBuffer );

        //--------------------------------------------------------------------
        // Lock vertex buffer and write vertices into it
        //--------------------------------------------------------------------

        Vector3* vArray = (Vector3*)lockVertexBuffer(
            m_vertexBuffer,
            0, 0);

        for(int i = 0; i < numv; i++)
        {
            *vArray++ = normals[i];
            *vArray++ = vertices[i];
        }

        unlockVertexBuffer(m_vertexBuffer);

        //--------------------------------------------------------------------
        // Lock index buffer and write triangle indices into it
        //--------------------------------------------------------------------

        unsigned short* iArray = (unsigned short*)lockIndexBuffer(
            m_indexBuffer,
            0, 0);

        for(int i = 0; i < nump; i++)
        {
            //--------------------------------------------------------------------
            // Assert that none of the triangle indices exceed the 16-bit
            // integer range.
            //--------------------------------------------------------------------

            PLATFORM_ASSERT((triangles[i][0]) < 65536 && (triangles[i][1] < 65536) && (triangles[i][2] < 65536));
            iArray[i*3+0] = (unsigned short)triangles[i][0];
            iArray[i*3+1] = (unsigned short)triangles[i][1];
            iArray[i*3+2] = (unsigned short)triangles[i][2];
        }

        unlockIndexBuffer(m_indexBuffer);
    };

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Destructor for D3D9 mesh.
	 *
	 *		   Releases the internal buffers.
	 *
	 *//*-------------------------------------------------------------------*/

    virtual ~OpenGLMesh() {

        if(m_vertexBuffer) 
        {
            deleteVertexBuffer(m_vertexBuffer);
        }

        if(m_indexBuffer) 
        {
            deleteIndexBuffer(m_indexBuffer);
        }
    }

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Renders the mesh.
	 *
	 *//*-------------------------------------------------------------------*/

    virtual void draw (void) const 
    {

        OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ARRAY_BUFFER,         m_vertexBuffer->platformBuffer);
        OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->platformBuffer );

        glInterleavedArrays(GL_N3F_V3F, 0, 0);

        // draw 6 quads using offset of index array
        glDrawElements(GL_TRIANGLES, m_numTriangles*3, GL_UNSIGNED_SHORT, 0);

        OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ARRAY_BUFFER,         0);
        OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, 0);

    }

private:

    VertexBuffer*                   m_vertexBuffer;
    IndexBuffer*                    m_indexBuffer;

    int                             m_numTriangles;
    int                             m_numVertices;
};

/*-------------------------------------------------------------------*//*!
 *
 * \brief  Function for setting the color
 *
 * Sets a pixel shader constant
 *
 * \param   color   The color
 *
 *//*-------------------------------------------------------------------*/

static void setColor(const Vector4& color)
{
    glColor4f(
        color.x,
        color.y,
        color.z,
        color.w);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Allows command line arguments to be set.
 *
 * \param	args	Command line arguments.
 *
 *//*-------------------------------------------------------------------*/

void Platform::setCommandLine( const std::vector<std::string> &args )
{
	g_args.clear();
	g_args.resize(args.size());
	std::copy(args.begin(), args.end(), g_args.begin());
}

/*-------------------------------------------------------------------*//*!
 * \brief	Get a list of command line arguments
 *//*-------------------------------------------------------------------*/

std::vector<std::string>& Platform::getCommandLine(void)
{
	return g_args;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Extension helper. Same as wglGetProcAddress, but
*           with assert
 *//*-------------------------------------------------------------------*/

template<typename T>
T getExtensionProc(const char* name)
{
    T proc = (T)wglGetProcAddress(name);
    PLATFORM_ASSERT(proc);

    return proc;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Cleans shader log file.
 *//*-------------------------------------------------------------------*/

static void clearShaderObjectLog()
{
    FILE* f = fopen("shaderlog.txt", "w");
    fclose(f);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Helper function for printing shader errors.
 *//*-------------------------------------------------------------------*/

static void printShaderObjectLog(GLuint obj)
{

    int sizeOut = 0;
    int logSize = 0;

    OpenGLGlobals::glGetObjectParameteriv(obj, OpenGLGlobals::GL_INFO_LOG_LENGTH, &logSize);
			
	char* log = new char[logSize+1];
    log[0] = '\0';
	
    OpenGLGlobals::glGetInfoLog(obj, logSize, &sizeOut, log);
 
	if (sizeOut > 0)
    {
        FILE* f = fopen("shaderlog.txt", "a");
        fprintf(f, "-------------\n");
		fprintf(f, "%s", log);
        fclose(f);
        MessageBoxA(NULL, log, "shader compilation log", MB_OK);
    }

    delete [] log;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Initializes the platform.
 *
 * \param	mode	Videomode to be used. Only used under Windows. On
 *                  consoles video mode is configured by the system.
 * \param	p		Platform-specific handle. In this case, HWND.
 * \param	args	Command line arguments.
 *
 *//*-------------------------------------------------------------------*/

bool Platform::init(const VideoMode &mode, void* p)
{
    const int bitsPerPixel   = 32;
    const int depthPrecision = 24;

	std::vector<std::string> &commandline = Platform::getCommandLine();
	// find argument "-printassert"
    std::vector<std::string>::iterator it = std::find(commandline.begin(), commandline.end(), "-printassert");

    if( it != commandline.end() )
    {
        // Set the assertions and abort() to print to the stderr instead of popping up a dialog
        const DWORD mode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
        SetErrorMode(mode | SEM_NOGPFAULTERRORBOX);
    }

	g_hWnd          = (HWND)p;
    g_renderWidth   = mode.width;
    g_renderHeight  = mode.height;
    g_windowed      = mode.windowed;

    //--------------------------------------------------------------------
    // Fullscreen setup, if required
    //--------------------------------------------------------------------

    if( !g_windowed )
    {        
        // Change display resolution if fullscreen mode requested

        DEVMODE devMode;
	    memset(&devMode, 0, sizeof(DEVMODE));

	    devMode.dmSize			= sizeof(DEVMODE);
	    devMode.dmPelsWidth		= g_renderWidth;
	    devMode.dmPelsHeight	= g_renderHeight;
	    devMode.dmBitsPerPel	= bitsPerPixel;
	    devMode.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	    LONG result = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

        if (result != DISP_CHANGE_SUCCESSFUL)
            return false;

        // Adjust window styles for fullscreen

        // Remove window border
        SetWindowLong( g_hWnd, GWL_STYLE,   WS_POPUP );
        // Make window topmost
        SetWindowLong( g_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST );
        
        // Move window to origo. This is also needed for above SetWindowLongs to take effect.
        SetWindowPos(g_hWnd, HWND_TOP, 0, 0, g_renderWidth, g_renderHeight, SWP_DRAWFRAME | SWP_SHOWWINDOW);

        // Todo: multiple monitors?
    }

    HDC   hDC;
    HGLRC hRC;
    int pixelFormat;

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor =
	{
		sizeof (PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bitsPerPixel, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        depthPrecision, 
        0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

    hDC = GetDC (g_hWnd);

    if (!hDC)
        return false;

    pixelFormat = ChoosePixelFormat(hDC, &pixelFormatDescriptor);

    if(!pixelFormat)
    {
        ReleaseDC(g_hWnd, hDC);
        return false;
    }

    if (SetPixelFormat(hDC, pixelFormat, &pixelFormatDescriptor) == FALSE)
    {
        ReleaseDC(g_hWnd, hDC);
        return false;
    }

    hRC = wglCreateContext(hDC);
    
    if(!hRC)
    {
        ReleaseDC (g_hWnd, hDC);
        return false;
    }

    if (wglMakeCurrent(hDC, hRC) == FALSE)
	{
        wglDeleteContext(hRC);
        ReleaseDC(g_hWnd, hDC);
        return false;
    }

    OpenGLGlobals::setDeviceContext(hDC);
    OpenGLGlobals::setRenderingContext(hRC);

    typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
    PFNWGLSWAPINTERVALEXTPROC  wglSwapIntervalEXT = NULL;

    wglSwapIntervalEXT = getExtensionProc<PFNWGLSWAPINTERVALEXTPROC>("wglSwapIntervalEXT");
       
    if(wglSwapIntervalEXT)
        wglSwapIntervalEXT(1);

    glViewport(0, 0, (GLsizei)(mode.width), (GLsizei)(mode.height));	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 

	glDepthFunc (GL_LEQUAL);
	glEnable (GL_DEPTH_TEST);
	glShadeModel (GL_SMOOTH);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glDepthMask(true); 
	glColorMask(true, true, true, true); 

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    const char* extensions = NULL; 
    extensions = (const char*)glGetString (GL_EXTENSIONS); 
    
    bool occlusionQueriesSupported = (strstr(extensions, "GL_ARB_occlusion_query") != NULL);
    bool vboSupported              = (strstr(extensions, "GL_ARB_vertex_buffer_object") != NULL);
    bool shadersSupported          = (strstr(extensions, "GL_ARB_shader_objects") != NULL);

    if (!occlusionQueriesSupported || !vboSupported || !shadersSupported)
    {
        char msg[256] = "";
        sprintf_s(
            msg, 
            256, 
            "Required extension(s) not supported:\n%s%s%s",
            occlusionQueriesSupported ? "" : "GL_ARB_occlusion_query\n",
            vboSupported              ? "" : "GL_ARB_vertex_buffer_object\n",
            shadersSupported          ? "" : "GL_ARB_shader_objects\n"
            );

        MessageBoxA(g_hWnd, msg, "Error", MB_OK);

        PLATFORM_ASSERT(false);
        return false;
    }

    OpenGLGlobals::glGenQueries             = getExtensionProc<OpenGLGlobals::PFNGLGENQUERIESARBPROC>("glGenQueriesARB");
    OpenGLGlobals::glDeleteQueries          = getExtensionProc<OpenGLGlobals::PFNGLDELETEQUERIESARBPROC>("glDeleteQueries");
    OpenGLGlobals::glIsQuery                = getExtensionProc<OpenGLGlobals::PFNGLISQUERYARBPROC>("glIsQueryARB");
    OpenGLGlobals::glBeginQuery             = getExtensionProc<OpenGLGlobals::PFNGLBEGINQUERYARBPROC>("glBeginQueryARB");
    OpenGLGlobals::glEndQuery               = getExtensionProc<OpenGLGlobals::PFNGLENDQUERYARBPROC>("glEndQueryARB");
    OpenGLGlobals::glGetQueryiv             = getExtensionProc<OpenGLGlobals::PFNGLGETQUERYIVARBPROC>("glGetQueryivARB");
    OpenGLGlobals::glGetQueryObjectiv       = getExtensionProc<OpenGLGlobals::PFNGLGETQUERYOBJECTIVARBPROC>("glGetQueryObjectivARB");
    OpenGLGlobals::glGetQueryObjectuiv      = getExtensionProc<OpenGLGlobals::PFNGLGETQUERYOBJECTUIVARBPROC>("glGetQueryObjectuivARB");

    OpenGLGlobals::glBindBuffer             = getExtensionProc<OpenGLGlobals::PFNGLBINDBUFFERARBPROC>("glBindBufferARB");
    OpenGLGlobals::glDeleteBuffers          = getExtensionProc<OpenGLGlobals::PFNGLDELETEBUFFERSARBPROC>("glDeleteBuffersARB");
    OpenGLGlobals::glGenBuffers             = getExtensionProc<OpenGLGlobals::PFNGLGENBUFFERSARBPROC>("glGenBuffersARB");
    OpenGLGlobals::glIsBuffer               = getExtensionProc<OpenGLGlobals::PFNGLISBUFFERARBPROC>("glIsBufferARB");
    OpenGLGlobals::glBufferData             = getExtensionProc<OpenGLGlobals::PFNGLBUFFERDATAARBPROC>("glBufferDataARB");
    OpenGLGlobals::glBufferSubData          = getExtensionProc<OpenGLGlobals::PFNGLBUFFERSUBDATAARBPROC>("glBufferSubDataARB");
    OpenGLGlobals::glGetBufferSubData       = getExtensionProc<OpenGLGlobals::PFNGLGETBUFFERSUBDATAARBPROC>("glGetBufferSubDataARB");
    OpenGLGlobals::glMapBuffer              = getExtensionProc<OpenGLGlobals::PFNGLMAPBUFFERARBPROC>("glMapBufferARB");
    OpenGLGlobals::glUnmapBuffer            = getExtensionProc<OpenGLGlobals::PFNGLUNMAPBUFFERARBPROC>("glUnmapBufferARB");
    OpenGLGlobals::glGetBufferParameteriv   = getExtensionProc<OpenGLGlobals::PFNGLGETBUFFERPARAMETERIVARBPROC>("glGetBufferParameterivARB");
    OpenGLGlobals::glGetBufferPointerv      = getExtensionProc<OpenGLGlobals::PFNGLGETBUFFERPOINTERVARBPROC>("glGetBufferPointervARB");

    OpenGLGlobals::glDeleteObject           = getExtensionProc<OpenGLGlobals::PFNGLDELETEOBJECTARBPROC>("glDeleteObjectARB");
    OpenGLGlobals::glGetHandle              = getExtensionProc<OpenGLGlobals::PFNGLGETHANDLEARBPROC>("glGetHandleARB");
    OpenGLGlobals::glDetachObject           = getExtensionProc<OpenGLGlobals::PFNGLDETACHOBJECTARBPROC>("glDetachObjectARB");
    OpenGLGlobals::glCreateShaderObject     = getExtensionProc<OpenGLGlobals::PFNGLCREATESHADEROBJECTARBPROC>("glCreateShaderObjectARB");
    OpenGLGlobals::glShaderSource           = getExtensionProc<OpenGLGlobals::PFNGLSHADERSOURCEARBPROC>("glShaderSourceARB");
    OpenGLGlobals::glCompileShader          = getExtensionProc<OpenGLGlobals::PFNGLCOMPILESHADERARBPROC>("glCompileShaderARB");
    OpenGLGlobals::glCreateProgramObject    = getExtensionProc<OpenGLGlobals::PFNGLCREATEPROGRAMOBJECTARBPROC>("glCreateProgramObjectARB");
    OpenGLGlobals::glAttachObject           = getExtensionProc<OpenGLGlobals::PFNGLATTACHOBJECTARBPROC>("glAttachObjectARB");
    OpenGLGlobals::glLinkProgram            = getExtensionProc<OpenGLGlobals::PFNGLLINKPROGRAMARBPROC>("glLinkProgramARB");
    OpenGLGlobals::glUseProgramObject       = getExtensionProc<OpenGLGlobals::PFNGLUSEPROGRAMOBJECTARBPROC>("glUseProgramObjectARB");
    OpenGLGlobals::glGetObjectParameteriv   = getExtensionProc<OpenGLGlobals::PFNGLGETOBJECTPARAMETERIVARBPROC>("glGetObjectParameterivARB");
    OpenGLGlobals::glGetInfoLog             = getExtensionProc<OpenGLGlobals::PFNGLGETINFOLOGARBPROC>("glGetInfoLogARB");

	//--------------------------------------------------------------------
    // Initialize shaders.
    //--------------------------------------------------------------------

    bool logCleared = false;
    
    GLint shaderSuccess = 0;

    g_colorVertexShader =  OpenGLGlobals::glCreateShaderObject(OpenGLGlobals::GL_VERTEX_SHADER);
	OpenGLGlobals::glShaderSource(g_colorVertexShader, 1, &g_strColorVertexShaderProgram, NULL);	
    OpenGLGlobals::glCompileShader(g_colorVertexShader);
    // Query success
    OpenGLGlobals::glGetObjectParameteriv(g_colorVertexShader, OpenGLGlobals::GL_OBJECT_COMPILE_STATUS, &shaderSuccess);

    // Output logs if compilation did not succeed
    if( !shaderSuccess )
    {
        if(!logCleared)
        {
            clearShaderObjectLog();
            logCleared = true;
        }
        printShaderObjectLog(g_colorVertexShader);
    }

	g_colorFragmentShader = OpenGLGlobals::glCreateShaderObject(OpenGLGlobals::GL_FRAGMENT_SHADER);
	OpenGLGlobals::glShaderSource(g_colorFragmentShader, 1, &g_strColorFragmentShaderProgram, NULL);
	OpenGLGlobals::glCompileShader(g_colorFragmentShader);
    OpenGLGlobals::glGetObjectParameteriv(g_colorFragmentShader, OpenGLGlobals::GL_OBJECT_COMPILE_STATUS, &shaderSuccess);
    
    // Output logs if compilation did not succeed
    if( !shaderSuccess )
    {
        if(!logCleared)
        {
            clearShaderObjectLog();
            logCleared = true;
        }
        printShaderObjectLog(g_colorFragmentShader);
    }

	g_colorShaderProgram = OpenGLGlobals::glCreateProgramObject();
	OpenGLGlobals::glAttachObject(g_colorShaderProgram, g_colorVertexShader);
	OpenGLGlobals::glAttachObject(g_colorShaderProgram, g_colorFragmentShader);
	OpenGLGlobals::glLinkProgram(g_colorShaderProgram);
    OpenGLGlobals::glGetObjectParameteriv(g_colorShaderProgram, OpenGLGlobals::GL_OBJECT_LINK_STATUS, &shaderSuccess);

    // Output logs if link did not succeed
    if( !shaderSuccess )
    {
        if(!logCleared)
        {
            clearShaderObjectLog();
            logCleared = true;
        }
        printShaderObjectLog(g_colorShaderProgram);
    }

	//--------------------------------------------------------------------
    // Generate key maps from Windows virtual key codes to Platform enum 
	// constants.
    //--------------------------------------------------------------------

    // Map 0-9 to corresponding constants. The virtual key code values are quaranteed
    // to run from 0x30 to 0x39 (same as ASCII).
    for(int i = 0; i < 10; i++)
        symbolVirtualKeys[0x30+i] = (Platform::KeyboardSymbol)(1<<i);

    // Map some of the special keys to corresponding constants.
    symbolVirtualKeys[VK_RETURN]	= Platform::KEY_ENTER;
    symbolVirtualKeys[VK_SPACE]		= Platform::KEY_SPACE;
    symbolVirtualKeys[VK_ESCAPE]	= Platform::KEY_ESC;
    symbolVirtualKeys[VK_UP]		= Platform::KEY_UP;
    symbolVirtualKeys[VK_DOWN]		= Platform::KEY_DOWN;
    symbolVirtualKeys[VK_LEFT]		= Platform::KEY_LEFT;
    symbolVirtualKeys[VK_RIGHT]		= Platform::KEY_RIGHT;
	symbolVirtualKeys[VK_ADD]		= Platform::KEY_PLUS;
	symbolVirtualKeys[VK_SUBTRACT]	= Platform::KEY_MINUS;

    // Map A-Z to corresponding constants. The virtual key code values are quaranteed
    // to run from 0x41 to 0x5A (same as ASCII).
    for(int i = 0; i < 26; i++)
        alphabetVirtualKeys[0x41+i] = (Platform::KeyboardAlphabet)(1<<i);

	HFONT fontHandle = CreateFont(	
                        -g_fontHeight,
						0,
						0,
						0,
						FW_NORMAL,
						FALSE,
						FALSE,
						FALSE,
						ANSI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
						FF_DONTCARE|VARIABLE_PITCH,
						L"Courier New");

	g_font = glGenLists(256);
	
    // Select new font, store previous one
    HFONT prevFont = (HFONT)SelectObject(hDC, fontHandle);
	// Generate font display lists
    wglUseFontBitmaps(hDC, 0, 256, g_font);
    // Restore previous font
	SelectObject(hDC, prevFont);
    // Delete font now that we have the display lists
	DeleteObject(fontHandle);

	return true;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Begins a debugging event. Not used on windows.
 *//*-------------------------------------------------------------------*/

void Platform::beginNamedEvent(const char* /* str */)
{

}

/*-------------------------------------------------------------------*//*!
 * \brief	Ends a debugging event. Not used on windows.
 *//*-------------------------------------------------------------------*/

void Platform::endNamedEvent(void)
{

}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets a named counter value. Not used on windows.
 *//*-------------------------------------------------------------------*/

void Platform::setNamedCounter(float /* c */, const char* /* str */)
{
}

/*-------------------------------------------------------------------*//*!
 * \brief	Platform exit. Cleans up.
 *//*-------------------------------------------------------------------*/

void Platform::exit(void)
{
    // Delete font display lists
    glDeleteLists(g_font, 256);

    // Reset video mode if rendering fullscreen
    if( !g_windowed )
		ChangeDisplaySettings (NULL,0);

    // Unbind GL context
    wglMakeCurrent( OpenGLGlobals::getDeviceContext(), NULL );

    // Delete GL context
    if(OpenGLGlobals::getRenderingContext())
    {
        wglDeleteContext(OpenGLGlobals::getRenderingContext());
        OpenGLGlobals::setRenderingContext(NULL);
    }

    // Delete DC
    if(OpenGLGlobals::getDeviceContext())
    {
        ReleaseDC( g_hWnd, OpenGLGlobals::getDeviceContext() );
        OpenGLGlobals::setDeviceContext(NULL);
    }
}

/*-------------------------------------------------------------------*//*!
 * \brief   Read input from keyboard.
 *
 * \param   KeyboardState       Stores keyboard state, using two bitmaps
 *                              to tell which keys are pressed down.
 *
 * \note    See KeyboardSymbol and KeyboardAlphabet enums in platform.hpp
 *          for more info
 *  
 *//*-------------------------------------------------------------------*/

void Platform::readKeyboard(KeyboardState& keyboardState)
{
	keyboardState.alphabetBitmap = keyboardAlphabetBitmap;
	keyboardState.symbolsBitmap = keyboardSymbolsBitmap;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Read input from mouse.
 *
 * \param   MouseState       Stores mouse state as position and button
 *                           state.
 *
 * \note    See MouseState and MouseSymbol in platform.hpp for more
 *          info.
 *  
 *//*-------------------------------------------------------------------*/

void Platform::readMouse(MouseState& mouseState)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	mouseState.x = cursorPos.x;
	mouseState.y = cursorPos.y;
	mouseState.buttonBitmap = mouseButtonBitmap;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Centers mouse in window
 *
 * \param	MouseState object used to outputs new mouse coordinates.
 *  
 *//*-------------------------------------------------------------------*/

void Platform::centerMouse(MouseState& mouseState)
{
	if(!windowActive)
	{
		mouseState.x = 0;
		mouseState.y = 0;
		return;
	}

	RECT window;
	GetWindowRect(g_hWnd, &window);

	mouseState.x = (window.right + window.left)/2;
	mouseState.y = (window.bottom + window.top)/2;

	SetCursorPos( mouseState.x,
				  mouseState.y );
}

/*-------------------------------------------------------------------*//*!
 * Map analog reading from a pad thumb stick (0 - 32767) to a float 
 * value.
 *//*-------------------------------------------------------------------*/
inline float mapXInputThumbStickValue(int value, int deadzone) {
    
    if(value < deadzone && value > -deadzone)
        return 0.f;

    return (float)value/32767.f;
}

/*-------------------------------------------------------------------*//*!
 * Map analog reading from a pad trigger (0 - 32767) to a float value.
 *//*-------------------------------------------------------------------*/
inline float mapXInputTriggerValue(int value, int deadzone) {
    
    if(value < deadzone)
        return 0.f;

    return (float)value/256.f;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Read input from system.
 *//*-------------------------------------------------------------------*/

void Platform::readPad(PadState &padState)
{
    padState.keyBitmap = 0;
    padState.analogThumbRX = 0.f;
    padState.analogThumbRY = 0.f;
    padState.analogThumbLX = 0.f;
    padState.analogThumbLY = 0.f;

    padState.analogL2 = 0.f;
    padState.analogR2 = 0.f;

    // Iterate all gamepads.
    for( DWORD i=0; i < 4; i++ )
    {
        // Read the input state
        XINPUT_STATE InputState;
        BOOL connected = ( XInputGetState( i, &InputState ) == ERROR_SUCCESS ) ? TRUE : FALSE;

        // If not connected, try the next one.
        if(!connected)
            continue;

        // DPAD
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
            padState.keyBitmap |= Platform::BUTTON_LUP;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
            padState.keyBitmap |= Platform::BUTTON_LDOWN;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
            padState.keyBitmap |= Platform::BUTTON_LRIGHT;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
            padState.keyBitmap |= Platform::BUTTON_LLEFT;

        // Y,A,X,B
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
            padState.keyBitmap |= Platform::BUTTON_RUP;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_A)
            padState.keyBitmap |= Platform::BUTTON_RDOWN;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_X)
            padState.keyBitmap |= Platform::BUTTON_RRIGHT;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
            padState.keyBitmap |= Platform::BUTTON_RLEFT;

        // Start and back
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_START)
            padState.keyBitmap |= Platform::BUTTON_START;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
            padState.keyBitmap |= Platform::BUTTON_BACK;

        // LB, LT
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
            padState.keyBitmap |= Platform::BUTTON_L1;
        if(InputState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            padState.keyBitmap |= Platform::BUTTON_L2;

        // RB, RT
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
            padState.keyBitmap |= Platform::BUTTON_L1;
        if(InputState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            padState.keyBitmap |= Platform::BUTTON_R2;

        // Left thumb stick as digital value
        if(InputState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_UP;
        else if(InputState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_DOWN;
        
        if(InputState.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_RIGHT;
        else if(InputState.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_LEFT;

        // Right thumb stick as digital value
        if(InputState.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_UP;
        else if(InputState.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_DOWN;
        
        if(InputState.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_RIGHT;
        else if(InputState.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            padState.keyBitmap |= Platform::THUMBR_LEFT;

        // Left and right thumb sticks pressed down
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
            padState.keyBitmap |= Platform::BUTTON_THUMBL;
        if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
            padState.keyBitmap |= Platform::BUTTON_THUMBR;

        // Analog inputs
        // Compare short values to dead zones so that value will be > 1.0f or < -1.0f when it's out of the dead zone
        padState.analogThumbRX = mapXInputThumbStickValue(InputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        padState.analogThumbRY = -mapXInputThumbStickValue(InputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

        padState.analogThumbLX = mapXInputThumbStickValue(InputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        padState.analogThumbLY = -mapXInputThumbStickValue(InputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        // Trigger values, compare to trigger treshold so that 
        // value will be > 1.0f when triggered
        padState.analogL2 = mapXInputTriggerValue(InputState.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
        padState.analogR2 = mapXInputTriggerValue(InputState.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

        // Only read the first connected gamepad.
        return;
    }
}

/*-------------------------------------------------------------------*//*!
 * \brief	Isseue an occlusion query begin. 
 *
 *			Everything rendered between beginQuery and endQuery gets
 *			accounted for the query.
 *
 * \param	qo	Query object obtained using allocateQuery
 *
 *//*-------------------------------------------------------------------*/

void Platform::beginQuery(Query* qo)
{
    OpenGLGlobals::glBeginQuery(OpenGLGlobals::GL_SAMPLES_PASSED, qo->platformQuery);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Issue an occlusion query end.
 *
 * \param	qo	Query object obtained using allocateQuery
 *
 *//*-------------------------------------------------------------------*/

void Platform::endQuery(Query* /* qo */)
{
    OpenGLGlobals::glEndQuery(OpenGLGlobals::GL_SAMPLES_PASSED);
    
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Gets the results of an occlusion query.
 *
 * \param	visibleSamples	Receives the amount of visible pixels
 * \param	qo				Occlusion query of interest
 * \param	waitForResult	Should the call wait for results to be
 *							available?
 *
 * \return  True, if the result was obtained 
 *
 *//*-------------------------------------------------------------------*/

bool Platform::getQueryResult(int& visibleSamples, Query* qo, bool waitForResult)
{
    GLuint visiblePixels    = 0;
	GLint availability      = 0;

    if (waitForResult)
    {
		while (!availability)
			OpenGLGlobals::glGetQueryObjectiv(qo->platformQuery, OpenGLGlobals::GL_QUERY_RESULT_AVAILABLE, &availability);

		OpenGLGlobals::glGetQueryObjectuiv(qo->platformQuery, OpenGLGlobals::GL_QUERY_RESULT, &visiblePixels);
    }
    else
    {
		OpenGLGlobals::glGetQueryObjectiv(qo->platformQuery, OpenGLGlobals::GL_QUERY_RESULT_AVAILABLE, &availability);		
		if (availability)
		    OpenGLGlobals::glGetQueryObjectuiv(qo->platformQuery, OpenGLGlobals::GL_QUERY_RESULT, &visiblePixels);
    }

	visibleSamples = visiblePixels; 
	return availability > 0;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets rendering state, depending on whether we're doing the
 *          depth pass, color pass or rendering occlusion queries.
 *//*-------------------------------------------------------------------*/

void Platform::setState(Platform::State s)
{
    if (g_state == s)
        return;

    if (g_state == WIREFRAME_DRAW_STATE)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    if (g_state == COLOR_DRAW_STATE)
        OpenGLGlobals::glUseProgramObject(0);

    switch (s)
    {

	//--------------------------------------------------------------------
    // This mode exists for rendering occlusion queries. 
	// 
	// Depth and color writes are disabled, while depth testing is 
	// enabled, allowing us to test bounding geometry against the depth 
	// buffer.
    //--------------------------------------------------------------------
    case DEPTH_TEST_STATE:
        {
            // Disable depth and color writes
			glDepthMask(false); 
			glColorMask(false, false, false, false); 			
        }
        break; 

	//--------------------------------------------------------------------
    // This mode exists for rendering the depth pass. 
	// 
	// Color writing is disabled, while depth write and testing is 
	// enabled.
    //--------------------------------------------------------------------
    case DEPTH_DRAW_STATE:
        {
			glDepthMask(true); 
			glColorMask(false, false, false, false); 
        }
        break; 

	//--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    case DEBUGLINE_DRAW_STATE:
        {
			glDepthMask(true); 
			glColorMask(true, true, true, true);
        }
        break;

	//--------------------------------------------------------------------
    //
    //--------------------------------------------------------------------
    case COLOR_DRAW_STATE:
        {
            OpenGLGlobals::glUseProgramObject(g_colorShaderProgram);
			glDepthMask(true); 
			glColorMask(true, true, true, true);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering wireframe.
    //
    // Render with color and depth writes, but use different polygon 
    // mode.
    //--------------------------------------------------------------------
    case WIREFRAME_DRAW_STATE:
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glDepthMask(true); 
			glColorMask(true, true, true, true);

        }
        break;
    }

    g_state = s;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the view matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setViewMatrix(const Matrix4x4& matrix)
{
    OpenGLGlobals::setViewMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the projection matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setProjectionMatrix(const Matrix4x4& matrix)
{
    OpenGLGlobals::setProjMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the world matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setWorldMatrix(const Matrix4x4& matrix)
{
    OpenGLGlobals::setWorldMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Get the current WVP-matrix
 *//*-------------------------------------------------------------------*/

static void getWV(Matrix4x4& matrix)
{
    Matrix4x4 world      = OpenGLGlobals::getWorldMatrix();
    Matrix4x4 view       = OpenGLGlobals::getViewMatrix();

    matrix = (world * view);
}


/*-------------------------------------------------------------------*//*!
 *
 * \brief   Renders one or more objects.
 *
 * \param   objects     An array of objects to be drawn.
 * \param   objectCount Number of objects to draw.
 *
 *//*-------------------------------------------------------------------*/

void Platform::draw(Object** objects, unsigned int objectCount)
{
    glMatrixMode(GL_PROJECTION);  
    glLoadMatrixf( (float*)&OpenGLGlobals::getProjMatrix() );

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* object = objects[i];

        Matrix4x4 objectToCell = object->getObjectToCellMatrix();
        OpenGLGlobals::setWorldMatrix(objectToCell);

        Matrix4x4 matWV;
        getWV((Matrix4x4&)matWV);

        glMatrixMode(GL_MODELVIEW);         
        glLoadIdentity();
        glMultMatrixf( (float*)&matWV );

        if(g_state != WIREFRAME_DRAW_STATE)
            setColor(object->getColor());
        else
            setColor(Vector4(1,1,1,1));

		// Render through the object's mesh
        object->getMesh()->draw();
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Renders one or more objects with shader.
 *
 * \param   objects     An array of objects to be drawn.
 * \param   objectCount Number of objects to draw.
 * \param   shader      Shader object to use
 *
 *//*-------------------------------------------------------------------*/

void Platform::draw(Object** objects, unsigned int objectCount, Shader* shader)
{
    glMatrixMode(GL_PROJECTION);  
    glLoadMatrixf( (float*)&OpenGLGlobals::getProjMatrix() );

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* object = objects[i];

        shader->preRender(object);

        Matrix4x4 objectToCell = object->getObjectToCellMatrix();
        OpenGLGlobals::setWorldMatrix(objectToCell);

        Matrix4x4 matWV;
        getWV((Matrix4x4&)matWV);

        glMatrixMode(GL_MODELVIEW);    
        glLoadIdentity();
        glMultMatrixf( (float*)&matWV );

        if(g_state != WIREFRAME_DRAW_STATE)
            setColor(object->getColor());
        else
            setColor(Vector4(1,1,1,1));

		// Render through the object's mesh
        object->getMesh()->draw();

        shader->postRender(object);
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Renders 2D lines.
 *
 *//*-------------------------------------------------------------------*/

void Platform::drawLinesScreenSpace(const Vector3* lines, int nLines, const Vector4& color)
{
    if (g_state == WIREFRAME_DRAW_STATE)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    Matrix4x4 ident;
    OpenGLGlobals::setWorldMatrix(ident);

    Matrix4x4 matWV;
    getWV((Matrix4x4&)matWV);

    glMatrixMode(GL_MODELVIEW);         
    glLoadIdentity();
    glMultMatrixf( (float*)&matWV );

    setColor(color);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(float), (float*)lines);
    glDrawArrays(GL_LINES, 0, nLines*2);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (g_state == WIREFRAME_DRAW_STATE)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders 3D lines.
 *//*-------------------------------------------------------------------*/

void Platform::drawLines(const Vector3* lines, int nLines, const Vector4& color)
{
    glMatrixMode(GL_PROJECTION);  
    glLoadMatrixf( (float*)&OpenGLGlobals::getProjMatrix() );

    Matrix4x4 ident;
    OpenGLGlobals::setWorldMatrix(ident);

    Matrix4x4 matWV;
    getWV((Matrix4x4&)matWV);

    glMatrixMode(GL_MODELVIEW);         
    glLoadIdentity();
    glMultMatrixf( (float*)&matWV );

    setColor(color);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (float*)lines);
    glDrawArrays(GL_LINES, 0, nLines*2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets up query rendering. 
 *
 *			Configures the shaders, transformation and position only
 *			vertex declaration before drawIndexedUP is called to render
 *			query shapes.
 *//*-------------------------------------------------------------------*/

void Platform::setupQueryDraw()
{
    glMatrixMode(GL_PROJECTION);  
    glLoadMatrixf( (float*)&OpenGLGlobals::getProjMatrix() );

    Matrix4x4 ident;
    OpenGLGlobals::setWorldMatrix(ident);

    Matrix4x4 matWV;
    getWV((Matrix4x4&)matWV);

    glMatrixMode(GL_MODELVIEW);         
    glLoadIdentity();
    glMultMatrixf( (float*)&matWV );
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders triangle data from currently active index and 
 *			vertex buffers.
 *
 * \param	vertexOffset	index to first vertex
 * \param	vertexCount		number of vertices
 * \param	trianglecount	index buffer size
 * \param	start			index to first index
 *
 *//*-------------------------------------------------------------------*/

void Platform::drawIndexed(int vertexOffset, int /* vertexCount */, int triangleCount, int start)
{
	glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, (void*)(vertexOffset*sizeof(Vector3)));

    glDrawElements(GL_TRIANGLES,
                   triangleCount*3,
                   GL_UNSIGNED_INT,
                   (void*)(start * sizeof(int)));
        
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders triangle data from vertex and index buffers given 
 *			as user pointers.
 *
 * \param	vertices		pointer to vertex data
 * \param	vertexCount		number of vertices
 * \param	triangles		pointer to index data
 * \param	triangleCount	number of triangles (number of indices/3)
 *
 * *//*-------------------------------------------------------------------*/

void Platform::drawIndexedUP(const Vector3* vertices, int /* vertexCount */, const Vector3i* triangles, int triangleCount)
{
    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, (float*)vertices);
    glDrawElements(GL_TRIANGLES, triangleCount*3, GL_UNSIGNED_INT, (void*)triangles);

    glDisableClientState(GL_VERTEX_ARRAY);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Clears the depth and stencil buffers.
 *//*-------------------------------------------------------------------*/

void Platform::clearDepthStencil(void)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
}

/*-------------------------------------------------------------------*//*!
 * \brief   Creates platform specific Mesh object.
 *
 * \param   vertices    Pointer to an array with the vertex data.
 * \param   normals     Pointer to an array with the vertex normal data.
 * \param   triangles   Pointer to an array with the index data.
 * \param   nVertices   Number of vertices in the given array.
 * \param   nTriangles  Number of triangles in the given array.
 *
 *//*-------------------------------------------------------------------*/

Mesh* Platform::createPlatformMesh(const Vector3* vertices, 
                                  const Vector3* normals, 
                                  const Vector3i* triangles, 
                                  int nVertices, 
                                  int nTriangles)
{
    return new OpenGLMesh(vertices, normals, triangles, nVertices, nTriangles);
}

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Releases a platform specific mesh which was created with
 *          Platform::createPlatformMesh.
 *
 * \param   mesh    Pointer to the Mesh to be released.
 *
 *//*-------------------------------------------------------------------*/

void Platform::releasePlatformMesh(Mesh* mesh)
{
    delete mesh;
    mesh = 0;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Measure text size
 *//*-------------------------------------------------------------------*/

void Platform::fontMeasure(
    int&        width, 
    int&        height, 
    const char* text, 
    ...)
{
    // Disable any rendering, since we're just measuring text
    glDisable(GL_DEPTH_TEST);    
    glDepthMask(false);
    glColorMask(false, false, false, false);

    // Setup orthographic projection in screen dimensions
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_renderWidth, g_renderHeight, 0, -100.f, 100.f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    char str[256] = "";

    // Use sprint to get formatted text
	va_list args;
	va_start (args, text);
	vsprintf_s (str, 256, text, args);
	va_end (args);

    glListBase(g_font);

    char* current = str;
    char* next    = NULL;
    
    int numLines = 0;
    bool done = false;

    width = 0;

    // Counts number of lines in the text.
    do
    {
        next = strchr(current, '\n');

        // Count last line and occurences of '\n' as lines.
        // If '\n' is the last character, it's not counted
        // as a line.
        if(!next || *(next + 1))
            numLines++;

        // If no '\n' found, use end of string as next position
        if(!next)
        {
            next = current + strlen(current);
            // exit loop
            done = true;
        }

        // Set raster position to origo
        glRasterPos2f(0, 0);

        // Test render text from current to next
        GLsizei len = next - current;
        glCallLists(len, GL_UNSIGNED_BYTE, current);

        // Get raster x-position after rendering for
        // tracking max width
        float pos[4];
        glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
        if ((int)pos[0] > width)
            width = (int)pos[0];

        // Continue from next character after '\n'
        current = next + 1;

    // Stop looping if strchr doesn't find a '\n'
    } while(!done); 

    glListBase(0);

    glEnable(GL_DEPTH_TEST);    
    glDepthMask(true);
    glColorMask(true, true, true, true);

    // Calculate text height from line count and font height.
    height = numLines * g_fontHeight;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders text.
 *//*-------------------------------------------------------------------*/

void Platform::fontPrint(
    int             x,
    int             y,
    const Vector4&  color,
    const char*     text,
	...)
{
    // Render font with given color 
    glColor4f(color.x, color.y, color.z, color.w);

    // Disable shader, if currently in COLOR_DRAW_STATE
    if (g_state == COLOR_DRAW_STATE)
        OpenGLGlobals::glUseProgramObject(0);

    glDisable(GL_DEPTH_TEST);    
    glDepthMask(false);

    // Setup orthographic projection in screen dimensions
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_renderWidth, g_renderHeight, 0, -100.f, 100.f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    char str[256] = "";

    // Use sprint to get formatted text
	va_list args;
	va_start (args, text);
	vsprintf_s (str, 256, text, args);
	va_end (args);

	glListBase(g_font);

    //--------------------------------------------------------------------
    // Render text from str array line by line
    //--------------------------------------------------------------------

    // Current position in string
    char *current = str;
    // Current draw position
    GLfloat currentX = (GLfloat)x;
    GLfloat currentY = (GLfloat)y + (GLfloat)g_fontHeight;

    bool done = false;

    do 
    {
        // Start from current position
        glRasterPos2f((GLfloat)currentX, (GLfloat)currentY);
        
        // Find next '\n'
        char* next = strchr(current, '\n');

        // If no '\n' found, use end of string as next position
        if(!next)
        {
            next = current + strlen(current);
            // exit loop
            done = true;
        }

        // Draw text from current to next
        GLsizei len = next - current;
        glCallLists(len, GL_UNSIGNED_BYTE, current);

        // Continue from next character after '\n'
        current = next+1;

        // Set position to start of the next line
        currentX = (GLfloat)x;
        currentY += (GLfloat)g_fontHeight;
    } while(!done);

    glListBase(0); 

    /*float pos[4];
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
    pos[0] -= x;
    pos[1] -= y; */

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);    

    if (g_state == COLOR_DRAW_STATE)
        OpenGLGlobals::glUseProgramObject(g_colorShaderProgram);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active index buffer.
 *
 * \param	indexBuffer		index buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setIndexBuffer(IndexBuffer* indexBuffer)
{
    OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, indexBuffer->platformBuffer);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );

}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active vertex buffer.
 *
 * \param	vertexBuffer	vertex buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	glEnableClientState(GL_VERTEX_ARRAY);
    OpenGLGlobals::glBindBuffer(OpenGLGlobals::GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->platformBuffer);
    PLATFORM_ASSERT( glGetError() == GL_NO_ERROR );
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Return screen size as references.
 *
 * \param   width       Maximum screen width.
 * \param   height      Maximum screen height.
 * \param   windowed    If the platform supports windowing.
 *
 *//*-------------------------------------------------------------------*/

void Platform::getScreenSize(unsigned int& width, unsigned int& height, bool& windowed)
{
    windowed    = true;
    width       = g_renderWidth;
    height      = g_renderHeight;
}

/*-------------------------------------------------------------------*//*!
 * \brief	
 *//*-------------------------------------------------------------------*/

void Platform::bindGraphicsThread(void)
{
    wglMakeCurrent( OpenGLGlobals::getDeviceContext(), OpenGLGlobals::getRenderingContext() );
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void Platform::unbindGraphicsThread(void)
{
    wglMakeCurrent( OpenGLGlobals::getDeviceContext(), NULL );
}

/*-------------------------------------------------------------------*//*!
 * \brief	Clears the color, depth and stencil buffers.
 *//*-------------------------------------------------------------------*/

void Platform::clear(float r,
                     float g,
                     float b,
                     float a,
                     float depth,
                     float stencil)
{
    glClearColor(r, g, b, a); 
    glClearDepth(depth); 
    glClearStencil((int)stencil); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the current view port.
 *//*-------------------------------------------------------------------*/

void Platform::setViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame starts.
 *//*-------------------------------------------------------------------*/

bool Platform::beginFrame(void)
{
    return true;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame ends.
 *//*-------------------------------------------------------------------*/

void Platform::endFrame(void)
{
    SwapBuffers( OpenGLGlobals::getDeviceContext() );
}

/*-------------------------------------------------------------------*//*!
 * \brief	Flush rendering.
 *//*-------------------------------------------------------------------*/

void Platform::flush(void)
{
    glFlush();
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void Platform::finish(void)
{
}

/*-------------------------------------------------------------------*//*!
 * \brief	Get application time in milliseconds.
 *//*-------------------------------------------------------------------*/

float Platform::getTime ()
{
    static LARGE_INTEGER time, startTime;
    static bool initialized = false;
    static LARGE_INTEGER freq;

    if (!initialized)
    {
		QueryPerformanceCounter(&startTime);
        initialized = true;
        QueryPerformanceFrequency(&freq);
    }

	QueryPerformanceCounter(&time);
    LONGLONG d = (time.QuadPart - startTime.QuadPart);

    return (float)((double)d)/freq.QuadPart * 1000.f;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sleeps milliseconds.
 *//*-------------------------------------------------------------------*/

void Platform::sleep (int ms)
{
	Sleep(ms);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Starts a thread.
 *//*-------------------------------------------------------------------*/

Thread* Platform::beginThread(void* (*threadMainFunc)(void*), void* args)
{
    HANDLE h = CreateThread(NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)threadMainFunc,
                            args,
                            CREATE_SUSPENDED,
                            NULL);
	ResumeThread(h);
    return new Thread(h);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Waits for a thread to exit.
 *//*-------------------------------------------------------------------*/

void Platform::waitThreadToComplete(Thread* thread)
{
    WaitForSingleObject(thread->handle, INFINITE);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Creates a mutex.
 *//*-------------------------------------------------------------------*/

Mutex* Platform::createMutex()
{
	Mutex* mutex = new Mutex;
	InitializeCriticalSection(&mutex->criticalSection);

    return mutex;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Destroys a mutex.
 *//*-------------------------------------------------------------------*/

void Platform::destroyMutex(Mutex* mutex)
{
    DeleteCriticalSection(&mutex->criticalSection);
    delete mutex;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Reserves a mutex.
 *//*-------------------------------------------------------------------*/

void Platform::enterMutex(Mutex* mutex)
{
    EnterCriticalSection(&mutex->criticalSection);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Releases a mutex.
 *//*-------------------------------------------------------------------*/

void Platform::leaveMutex(Mutex* mutex)
{
    LeaveCriticalSection(&mutex->criticalSection);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Creates a semaphore.
 *//*-------------------------------------------------------------------*/

Semaphore* Platform::createSemaphore(int initialCount)
{
	HANDLE handle = 
	CreateSemaphore(
			NULL,
			initialCount,
			INT_MAX,
			NULL);

	return new Semaphore(handle);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Deletes a semaphore.
 *//*-------------------------------------------------------------------*/

void Platform::deleteSemaphore(Semaphore* sem)
{
    CloseHandle(sem->handle);
	delete sem;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Increases semaphore value.
 *//*-------------------------------------------------------------------*/

void Platform::incSemaphore(Semaphore* sem)
{
    ReleaseSemaphore(sem->handle, 1, NULL);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Decreases a semaphore value.
 *//*-------------------------------------------------------------------*/

bool Platform::decSemaphore(Semaphore* sem, bool wait)
{
    return WAIT_OBJECT_0 == WaitForSingleObject(sem->handle, wait ? INFINITE : 0);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Atomic add operation.
 *//*-------------------------------------------------------------------*/

int Platform::atomicAdd(volatile int* value, int a)
{
    return InterlockedExchangeAdd((volatile LONG*)value, a);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Atomic substract operation.
 *//*-------------------------------------------------------------------*/

int Platform::atomicSub(volatile int* value, int a)
{
    return InterlockedExchangeAdd((volatile LONG*)value, -a);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Atomic assignment.
 *//*-------------------------------------------------------------------*/

int Platform::atomicStore(volatile int* value, int a)
{
    return InterlockedExchange((volatile LONG*)value, a);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Atomic value read.
 *//*-------------------------------------------------------------------*/

int Platform::atomicGet(volatile int* value)
{
    return *value; // DEBUG DEBUG TODO!
}

/*-------------------------------------------------------------------*//*!
 * \brief	Shader constructor
 *//*-------------------------------------------------------------------*/

Platform::Shader::Shader()
: m_initialized(false)
{
}

/*-------------------------------------------------------------------*//*!
 * \brief	Shader init helper
 *//*-------------------------------------------------------------------*/

bool Platform::Shader::init()
{
    if(m_initialized)
        return false;

    bool success = doinit(NULL);
    if(success)
    {
        m_initialized = true;
        return true;
    } else
    {
        assert(false);
    }

    return false;
}

//-----------------------------------------------------------------------------

