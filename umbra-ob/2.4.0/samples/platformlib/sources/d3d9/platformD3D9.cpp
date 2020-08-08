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
 * \brief     Platform abstraction for Direct3D 9.
 * 
 *//*-------------------------------------------------------------------*/

#include <d3d9.h>
#include <D3DX9Shader.h>
#include <assert.h>
#include <map>
#include <windows.h>
#include <xinput.h>

#include <algorithm>
#include <sstream>

#include "platform.hpp"
#include "plObject.hpp"
#include "plMesh.hpp"

using namespace Platform;

typedef INT (*ATIQUERYMGPUCOUNT)();

__inline INT AtiMultiGPUAdapters()
{
	HINSTANCE lib = LoadLibrary(TEXT("ATIMGPUD.DLL"));
	if (!lib)
		return 1;

	ATIQUERYMGPUCOUNT AtiQueryMgpuCount;
	AtiQueryMgpuCount = (ATIQUERYMGPUCOUNT)GetProcAddress(lib, "AtiQueryMgpuCount");
	if (!AtiQueryMgpuCount)
		return 1;

	INT count = AtiQueryMgpuCount();
	if (count < 1) count = 1;

	FreeLibrary(lib);

	return count;
}

/*-------------------------------------------------------------------*//*!
 * Internal types.
 * In practice these classes are wrappers of the internal pointer types.
 *//*-------------------------------------------------------------------*/

namespace Platform 
{
	class VertexBuffer
	{
	public:
		VertexBuffer(IDirect3DVertexBuffer9 *value) 
			: platformBuffer(value) {}

		IDirect3DVertexBuffer9* platformBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(IDirect3DIndexBuffer9 *value) 
			: platformBuffer(value) {}

		IDirect3DIndexBuffer9* platformBuffer;
	};

	class Query
	{
	public:
		Query(IDirect3DQuery9 *value) 
			: platformQuery(value) {}

		IDirect3DQuery9* platformQuery;
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
 * Globals
 *//*-------------------------------------------------------------------*/

static int g_renderWidth = 0;
static int g_renderHeight = 0;

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
ID3DXFont*                  g_font			= NULL; 
// Font height in pixels
const int					g_fontHeight	= 20;

// Mouse button state
unsigned int mouseButtonBitmap	= 0;

// True if window is active
bool windowActive = false;

/*-------------------------------------------------------------------*//*!
 * Vertex shader for the depth pass.
 *//*-------------------------------------------------------------------*/

static IDirect3DVertexShader9* g_depthVertexShader = NULL;

const char* g_strDepthVertexShaderProgram = 
" float4x4 matWVP : register(c0);               "  
"                                               "  
" struct VS_IN_DEPTH                            "  
" {                                             " 
"     float3 pos	: POSITION;                 "  // Object space position 
"	  float3 normal	: NORMAL;                   "
" };                                            " 
"                                               " 
" struct VS_OUT_DEPTH                           " 
" {                                             " 
" 	float4 pos		: SV_POSITION;              "
" };                                            "  
"                                               "  
" VS_OUT_DEPTH VS_Main_Depth( VS_IN_DEPTH IN )  "  
" {                                             "  
"   VS_OUT_DEPTH OUT;                           "
" 	OUT.pos = mul(matWVP, float4(IN.pos, 1));   "
" 	return OUT;                                 "  // Transfer position
" };                                            ";

/*-------------------------------------------------------------------*//*!
 * Vertex shader for query shapes.
 *//*-------------------------------------------------------------------*/

static IDirect3DVertexShader9*      g_queryVertexShader = NULL;
static IDirect3DVertexDeclaration9* g_vertexDeclPosOnly = NULL;

const char* g_strQueryVertexShaderProgram = 
" float4x4 matWVP : register(c0);               "  
"                                               "  
" struct VS_IN_DEPTH                            "  
" {                                             " 
"     float3 pos	: POSITION;                 "
" };                                            " 
"                                               " 
" struct VS_OUT_DEPTH                           " 
" {                                             " 
"     float4 pos : SV_POSITION;                 "  // Projected space position 
" };                                            "  
"                                               "  
" VS_OUT_DEPTH VS_Main_Query( VS_IN_DEPTH IN )  "  
" {                                             "  
"   VS_OUT_DEPTH OUT;                           "
" 	OUT.pos = mul(matWVP, float4(IN.pos, 1));   "
" 	return OUT;                                 "  // Transfer position
" }                                             ";

/*-------------------------------------------------------------------*//*!
 * Pixel shader for depth pass.
 *//*-------------------------------------------------------------------*/

static IDirect3DPixelShader9 * g_depthPixelShader = NULL;

const char* g_strDepthPixelShaderProgram = 
" struct PS_OUT_DEPTH						    "
" {                                             "
"     float4 Color : COLOR;                     "
" };                                            "
"                                               "
" PS_OUT_DEPTH PS_Main_Depth() : COLOR          "
" {                                             "
"   PS_OUT_DEPTH OUT;                           "
"	OUT.Color = float4(1,1,1,1);                "
"	return OUT;                                 "
" }                                             ";

/*-------------------------------------------------------------------*//*!
 * Vertex shader for color pass.
 *//*-------------------------------------------------------------------*/

static IDirect3DVertexShader9*      g_colorVertexShader = NULL;
static IDirect3DVertexDeclaration9* g_colorInputLayout  = NULL;

const char* g_strColorVertexShaderProgram = 
" float4x4 matWVP : register(c0);                           "
"                                                           "
" struct VS_IN_COLOR                                        "
" {                                                         "
"	float3 pos		: POSITION;                             "
"	float3 normal	: NORMAL;                               "
" };                                                        "
"                                                           "
" struct VS_OUT_COLOR                                       "
" {                                                         "
" 	float4 pos		: SV_POSITION;                          "
" 	float3 normal	: NORMAL;                               "
" };                                                        "
"                                                           "
" VS_OUT_COLOR VS_Main_Color(VS_IN_COLOR IN)                "
" {                                                         "
" 	VS_OUT_COLOR OUT;                                       "
" 	OUT.pos = mul(matWVP, float4(IN.pos, 1));               "
" 	OUT.normal = mul((float3x3)matWVP, IN.normal); "
" 	return OUT;                                             "
" }                                                         ";

/*-------------------------------------------------------------------*//*!
 * Pixel shader for color pass.
 *//*-------------------------------------------------------------------*/

static IDirect3DPixelShader9* g_colorPixelShader = NULL;

const char* g_strColorPixelShaderProgram =
" float4 color      : register(c0);											"
"																			"
" struct PS_IN_COLOR														"
" {																			"
" 	float4 pos		: POSITION;												"
" 	float3 normal	: NORMAL;												"
" };																		"
"																			"
" struct PS_OUT_COLOR														"
" {																			"
" 	float4 color : COLOR;													"
" };																		"
"																			"
" PS_OUT_COLOR PS_Main_Color(PS_IN_COLOR IN)								"
" {																			"
"   float4 lightColor = float4(1.f,1.f,1.f,1.f);							"
"   float3 lightDir = float3(0.f,0.f,1.f);									"
" 	PS_OUT_COLOR OUT;														"
"																			"
"   float c	= max(0.0, dot(normalize(-lightDir), normalize(IN.normal)));	"
"	c = min(c, 1.0);														"
" 	if (IN.normal.x == 0 && IN.normal.y == 0 && IN.normal.z == 0)			"
"     c = 1.f;																"
" 																			"
" 	OUT.color		= color * c;											"
" 	return OUT;																"
" }																			";

/*-------------------------------------------------------------------*//*!
 * Flat color pixel shader for rendering debug lines.
 *//*-------------------------------------------------------------------*/

static IDirect3DPixelShader9* g_flatColorPixelShader = NULL;

const char* g_strFlatColorPixelShaderProgram =
" float4 color      : register(c0);                                 "
"                                                                   "
" struct PS_IN_COLOR                                                "
" {                                                                 "
" 	float4 pos		: POSITION;                                     "
" 	float3 normal	: NORMAL;                                       "
" };                                                                "
"                                                                   "
" struct PS_OUT_COLOR                                               "
" {                                                                 "
" 	float4 color : COLOR;                                           "
" };                                                                "
"                                                                   "
" PS_OUT_COLOR PS_Main_Color(PS_IN_COLOR IN)                        "
" {                                                                 "
" 	PS_OUT_COLOR OUT;                                               "
" 	OUT.color		= color;										"
" 	return OUT;                                                     "
" }                                                                 ";

/*-------------------------------------------------------------------*//*!
 * DirectX globals.
 *//*-------------------------------------------------------------------*/

class DirectXGlobals
{
public:

    static IDirect3DDevice9*    getDevice(void)                 { PLATFORM_ASSERT(s_device); return s_device; }
    static void                 setDevice(IDirect3DDevice9* d)  { s_device = d; }

    static IDirect3D9*          getDirect3D9(void)              { PLATFORM_ASSERT(s_direct3D9); return s_direct3D9; }
    static void                 setDirect3D9(IDirect3D9* d)     { s_direct3D9 = d; }

    static const Matrix4x4&             getViewMatrix (void)                    { return s_matView; }
    static void                         setViewMatrix (const Matrix4x4& matrix) { s_matView = matrix; } 

    static const Matrix4x4&             getProjMatrix (void)                    { return s_matProj; }
    static void                         setProjMatrix (const Matrix4x4& matrix) { s_matProj = matrix; } 

    static const Matrix4x4&             getWorldMatrix (void)                    { return s_matWorld; }
    static void                         setWorldMatrix (const Matrix4x4& matrix) { s_matWorld = matrix; } 

private:

	DirectXGlobals();

    static IDirect3D9*          s_direct3D9;
    static IDirect3DDevice9*    s_device;

    static Matrix4x4             s_matView;       // View matrix 
    static Matrix4x4             s_matProj;       // Projection matrix
    static Matrix4x4             s_matWorld;      // World matrix	
};

//--------------------------------------------------------------------
// Initializing the DirectX globals
//--------------------------------------------------------------------

IDirect3D9*        DirectXGlobals::s_direct3D9 = NULL;
IDirect3DDevice9*  DirectXGlobals::s_device = NULL;

Matrix4x4          DirectXGlobals::s_matView;
Matrix4x4          DirectXGlobals::s_matProj;
Matrix4x4          DirectXGlobals::s_matWorld;

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Allocates an occlusion query.
 *
 *//*-------------------------------------------------------------------*/

Query*  Platform::allocateQuery (void)
{
    IDirect3DDevice9* D3D             = DirectXGlobals::getDevice();
    IDirect3DQuery9*  pOcclusionQuery = NULL;

    HRESULT res = D3D->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery);
    if (res == S_OK)
        return new Query(pOcclusionQuery);
    else
        return NULL;
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
    query->platformQuery->Release();
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

    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();
    PLATFORM_ASSERT(D3D);

    IDirect3DVertexBuffer9* vb;
    D3D->CreateVertexBuffer(size,
                            D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
                            D3DFVF_XYZ,
                            D3DPOOL_DEFAULT,
                            &vb,
                            NULL);

    return new VertexBuffer(vb);
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

    ULONG rv = buffer->platformBuffer->Release();
    PLATFORM_ASSERT(rv == 0);
    (void)rv;

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

void*  Platform::lockVertexBuffer	(VertexBuffer* buffer, int offset, int size)
{
    PLATFORM_ASSERT(buffer);
    PLATFORM_ASSERT(size > 0);
    void* pData = NULL;
    HRESULT rv = buffer->platformBuffer->Lock(offset, size, &pData, D3DLOCK_NOOVERWRITE );
    PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
    return pData;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Unlocks a vertex buffer.
 *
 * \param	buffer	Vertex buffer to unlock
 *
 *//*-------------------------------------------------------------------*/

void Platform::unlockVertexBuffer(VertexBuffer* buffer)
{
    PLATFORM_ASSERT(buffer);
    HRESULT rv = buffer->platformBuffer->Unlock();
    PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
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

    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();
    PLATFORM_ASSERT(D3D);

    IDirect3DIndexBuffer9* ib;
    D3D->CreateIndexBuffer (size,
                            D3DUSAGE_WRITEONLY,
                            D3DFMT_INDEX32,
                            D3DPOOL_MANAGED,
                            &ib,
                            NULL);

    return new IndexBuffer(ib);
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
    
	ULONG rv = buffer->platformBuffer->Release();
    PLATFORM_ASSERT(rv == 0);
    (void)rv;

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

void* Platform::lockIndexBuffer(IndexBuffer* buffer, int offset, int size)
{
    PLATFORM_ASSERT(buffer);
    PLATFORM_ASSERT(size > 0);
    void* pData = NULL;
    HRESULT rv = buffer->platformBuffer->Lock(offset, size, &pData, 0);
    PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
    return pData;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Unlocks a index buffer.
 *
 * \param	buffer	Index buffer to unlock
 *
 *//*-------------------------------------------------------------------*/

void Platform::unlockIndexBuffer(IndexBuffer* buffer)
{
    PLATFORM_ASSERT(buffer);
    HRESULT rv = buffer->platformBuffer->Unlock();
    PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief  D3D9 implementation of mesh.
 *
 *//*-------------------------------------------------------------------*/

class D3D9Mesh : public Mesh
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

	D3D9Mesh(const Vector3* vertices, const Vector3* normals, const Vector3i* triangles, int nVertices, int nTriangles) :
      m_numTriangles(nTriangles),
      m_numVertices(nVertices)
    {
        IDirect3DDevice9* D3D = DirectXGlobals::getDevice();
        PLATFORM_ASSERT(D3D);

        int numv = nVertices;
        int nump = nTriangles;

		//--------------------------------------------------------------------
        // Create vertex buffer to hold mesh vertex and normal data
        //--------------------------------------------------------------------

        D3D->CreateVertexBuffer(numv*sizeof(Vector3)*2,
                                D3DUSAGE_WRITEONLY,
                                D3DFVF_XYZ|D3DFVF_NORMAL,
                                D3DPOOL_MANAGED,
                                &m_vertexBuffer,
                                NULL);
        if(!m_vertexBuffer)
            return;

		//--------------------------------------------------------------------
        // Crete index buffer to hold index data
        //--------------------------------------------------------------------

		D3D->CreateIndexBuffer (nump*sizeof(short)*3,
                                D3DUSAGE_WRITEONLY,
                                D3DFMT_INDEX16,
                                D3DPOOL_MANAGED,
                                &m_indexBuffer,
                                NULL);
        
        if(!m_indexBuffer)
        {
            m_vertexBuffer->Release();
            return;
        }

        //--------------------------------------------------------------------
        // Locks vertex buffer and writes vertices into it
        //--------------------------------------------------------------------

        BYTE* pData;
        m_vertexBuffer->Lock(0, 0, (void**)&pData, 0);
        Vector3* vArray = (Vector3*)pData;
        int i;
        for(i = 0; i < numv; i++)
        {
            *vArray++ = vertices[i];
            *vArray++ = normals[i];
        }
        
		m_vertexBuffer->Unlock();

        //--------------------------------------------------------------------
        // Locks index buffer and writes triangle indices into it
        //--------------------------------------------------------------------

        m_indexBuffer->Lock(0, 0, (void**)&pData, 0);
        unsigned short* iArray = (unsigned short*)pData;

        for(i = 0; i < nump; i++)
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

        m_indexBuffer->Unlock();
    };

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Destructor for D3D9 mesh.
	 *
	 *		   Releases the internal buffers.
	 *
	 *//*-------------------------------------------------------------------*/

    virtual ~D3D9Mesh() {

        if(m_vertexBuffer) 
		{
            ULONG refcount = m_vertexBuffer->Release();
            PLATFORM_ASSERT(refcount == 0);
            (void)refcount;
        }

        if(m_indexBuffer) 
		{
            ULONG refcount = m_indexBuffer->Release();
            PLATFORM_ASSERT(refcount == 0);
            (void)refcount;
        }
    }

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Renders the mesh.
	 *
	 *//*-------------------------------------------------------------------*/

    virtual void draw (void) const 
    {

        IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

        //--------------------------------------------------------------------
        // Setup the drawing
        //--------------------------------------------------------------------

        D3D->SetStreamSource        (0, m_vertexBuffer, 0, sizeof(Vector3)*2);
        D3D->SetIndices             (m_indexBuffer);

        //--------------------------------------------------------------------
        // Draw the mesh
        //--------------------------------------------------------------------

        D3D->DrawIndexedPrimitive   (D3DPT_TRIANGLELIST, 0, 0, m_numVertices, 0, m_numTriangles);

        //--------------------------------------------------------------------
        // Set the stream source to null
        //--------------------------------------------------------------------

        D3D->SetStreamSource        (0, NULL, 0, 0);
    }

private:

    IDirect3DVertexBuffer9*         m_vertexBuffer;
    IDirect3DIndexBuffer9*          m_indexBuffer;

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
    DirectXGlobals::getDevice()->SetPixelShaderConstantF( 0, (FLOAT*)&color, 1 );
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
	std::vector<std::string> &commandline = Platform::getCommandLine();
	// find argument "-printassert"
    std::vector<std::string>::iterator it = std::find(commandline.begin(), commandline.end(), "-printassert");

    if( it != commandline.end() )
    {
        // Set the assertions and abort() to print to the stderr instead of popping up a dialog
        const DWORD mode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
        SetErrorMode(mode | SEM_NOGPFAULTERRORBOX);
    }

	g_hWnd = (HWND)p;

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    if(!pD3D)
        return false;

    DirectXGlobals::setDirect3D9(pD3D);

    OutputDebugString(TEXT("D3DSAMPLE: Direct3D9 Created\n"));

	//--------------------------------------------------------------------
    // Create device
    //--------------------------------------------------------------------

    HRESULT result = pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, TRUE);

    PLATFORM_ASSERT(result == D3D_OK);

    g_renderWidth   = mode.width;
    g_renderHeight  = mode.height;

	D3DMULTISAMPLE_TYPE multiSampleType = D3DMULTISAMPLE_NONE;

	if(mode.multisamples < 2)
	{
		// use NONE if count is 0 or 1
		multiSampleType = D3DMULTISAMPLE_NONE;
	} else
	if(mode.multisamples >= 2 && mode.multisamples <= 16)
	{
		// Multisample counts from 2 to 16 map directly to enum value
		multiSampleType = (D3DMULTISAMPLE_TYPE)mode.multisamples;
	}
	else
	{
		// Otherwise unsupported
		PLATFORM_ASSERT("Unsupported multisample mode" && false);
		return false;
	}

    D3DPRESENT_PARAMETERS parms =
    {
        g_renderWidth, g_renderHeight,
        D3DFMT_X8R8G8B8,
        1,
        multiSampleType,
        0,
        D3DSWAPEFFECT_DISCARD,
        g_hWnd, // hDeviceWindow
        FALSE,
        TRUE,
        D3DFMT_D24S8,
        0,
        0,  //D3DPRESENT_RATE_DEFAULT,
        D3DPRESENT_INTERVAL_IMMEDIATE
    };

    parms.Windowed   = mode.windowed;

    IDirect3DDevice9* dev = NULL;

    result = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                D3DDEVTYPE_HAL, g_hWnd,
                                D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE|D3DCREATE_MULTITHREADED,
                                &parms,
                                &dev);

	PLATFORM_ASSERT(result == D3D_OK);
  
    DirectXGlobals::setDevice(dev);

    if(!DirectXGlobals::getDevice())
        return false;

    //--------------------------------------------------------------------
    // Create vertex declaration for query shapes
    //--------------------------------------------------------------------
    
	ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;

    D3DVERTEXELEMENT9 VertexElementsPositionOnly[] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        D3DDECL_END()
    };
    dev->CreateVertexDeclaration(VertexElementsPositionOnly,
                                 &g_vertexDeclPosOnly);

    //--------------------------------------------------------------------
    // Create vertex declaration for color pass
    //--------------------------------------------------------------------

    D3DVERTEXELEMENT9 VertexElementsColorPass[] =
    {
        { 0,  0,               D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0,  sizeof(float)*3, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        D3DDECL_END()
    };
    dev->CreateVertexDeclaration(VertexElementsColorPass,
                                 &g_colorInputLayout);

    //--------------------------------------------------------------------
    // Compile the depth pass vertex shader and create the vertex shader
    // object
    //--------------------------------------------------------------------

    HRESULT hr = D3DXCompileShader(g_strDepthVertexShaderProgram, 
                                   (UINT)strlen( g_strDepthVertexShaderProgram ),
                                   NULL, NULL, 
                                   "VS_Main_Depth", "vs_3_0", 0, 
                                   &pVertexShaderCode, &pVertexErrorMsg, NULL );

    if( FAILED(hr) )
        return false;

    dev->CreateVertexShader((DWORD*)pVertexShaderCode->GetBufferPointer(),
                            &g_depthVertexShader );

    // Release the pointer to the compiled shader once you are done with it
    pVertexShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the query test shape vertex shader and create the vertex shader
    // object
    //--------------------------------------------------------------------

    hr = D3DXCompileShader(g_strQueryVertexShaderProgram, 
                           (UINT)strlen( g_strQueryVertexShaderProgram ),
                           NULL, NULL, 
                           "VS_Main_Query", "vs_3_0", 0, 
                           &pVertexShaderCode, &pVertexErrorMsg, NULL );

    if( FAILED(hr) )
        return false;

    dev->CreateVertexShader((DWORD*)pVertexShaderCode->GetBufferPointer(),
                            &g_queryVertexShader );

    // Release the pointer to the compiled shader once you are done with it
    pVertexShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the color pass vertex shader and create the vertex shader
    // object
    //--------------------------------------------------------------------

    hr = D3DXCompileShader( g_strColorVertexShaderProgram, 
                            (UINT)strlen( g_strColorVertexShaderProgram ),
                            NULL, NULL, 
                            "VS_Main_Color", "vs_3_0", 
                            0, &pVertexShaderCode, &pVertexErrorMsg, NULL );

    if( FAILED(hr) )
        return false;

    dev->CreateVertexShader((DWORD*)pVertexShaderCode->GetBufferPointer(),
                            &g_colorVertexShader );

    // Release the pointer to the compiled shader once you are done with it
    pVertexShaderCode->Release();

    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;

    //--------------------------------------------------------------------
    // Compile the depth pass pixel shader and create the pixel shader
    // object
    //--------------------------------------------------------------------

    hr = D3DXCompileShader( g_strDepthPixelShaderProgram, 
                            (UINT)strlen( g_strDepthPixelShaderProgram ),
                            NULL, NULL, 
                            "PS_Main_Depth", "ps_3_0", 0, 
                            &pPixelShaderCode, &pPixelErrorMsg, NULL );
    if( FAILED(hr) )
    {
        char* str = (char*)pPixelErrorMsg->GetBufferPointer();
        (void)str;
        return false;
    }

    dev->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                           &g_depthPixelShader );

    //--------------------------------------------------------------------
    // Compile the color pass pixel shader and create the pixel shader
    // object
    //--------------------------------------------------------------------

    hr = D3DXCompileShader( g_strColorPixelShaderProgram, 
                            (UINT)strlen( g_strColorPixelShaderProgram ),
                            NULL, NULL, 
                            "PS_Main_Color", "ps_3_0", 0, 
                            &pPixelShaderCode, &pPixelErrorMsg, NULL );
    if( FAILED(hr) )
        return false;

    dev->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                           &g_colorPixelShader );

    //--------------------------------------------------------------------
    // Compile the flat color pixel shader and create the pixel shader
    // object
    //--------------------------------------------------------------------

    hr = D3DXCompileShader( g_strFlatColorPixelShaderProgram, 
                            (UINT)strlen( g_strFlatColorPixelShaderProgram ),
                            NULL, NULL, 
                            "PS_Main_Color", "ps_3_0", 0, 
                            &pPixelShaderCode, &pPixelErrorMsg, NULL );
    if( FAILED(hr) )
        return false;

    dev->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                           &g_flatColorPixelShader );


    //--------------------------------------------------------------------
    // Initialize the rest of the state
    //--------------------------------------------------------------------

    D3DVIEWPORT9 viewport =
    {
        0, 0, g_renderWidth, g_renderHeight, 0.f, 1.f
    };

    dev->SetViewport(&viewport);
    dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

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

    //--------------------------------------------------------------------
    // Create font
    //--------------------------------------------------------------------

    if( FAILED( hr = D3DXCreateFont( dev, g_fontHeight, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                                     TEXT( "Arial" ), &g_font ) ) )
		return false;

	return true;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Begins a debugging event. Not used on windows.
 *//*-------------------------------------------------------------------*/

void Platform::beginNamedEvent(const char* str)
{
    (void)str;
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

void Platform::setNamedCounter(float c, const char* str)
{
    (void)c;
    (void)str;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Platform exit. Cleans up.
 *//*-------------------------------------------------------------------*/

void Platform::exit(void)
{
    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

    D3D->SetVertexShader(0);
    D3D->SetPixelShader (0);
    D3D->SetVertexDeclaration(0);

	//--------------------------------------------------------------------
    // Release internal objects.
    //--------------------------------------------------------------------

	if(g_font)
	{
        ULONG refcount = g_font->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
	}

    if(g_depthVertexShader) 
	{
        ULONG refcount = g_depthVertexShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_queryVertexShader) 
	{
        ULONG refcount = g_queryVertexShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_vertexDeclPosOnly) 
	{
        ULONG refcount = g_vertexDeclPosOnly->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_depthPixelShader) 
	{
        ULONG refcount = g_depthPixelShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_colorVertexShader) 
	{
        ULONG refcount = g_colorVertexShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_colorInputLayout) 
	{
        ULONG refcount = g_colorInputLayout->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_colorPixelShader) 
	{
        ULONG refcount = g_colorPixelShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(g_flatColorPixelShader) 
	{
        ULONG refcount = g_flatColorPixelShader->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(DirectXGlobals::getDevice()) 
	{
		ULONG refcount = DirectXGlobals::getDevice()->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
    }

    if(DirectXGlobals::getDirect3D9()) 
	{
        ULONG refcount = DirectXGlobals::getDirect3D9()->Release();
        PLATFORM_ASSERT(refcount == 0);
        (void)refcount;
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
    qo->platformQuery->Issue(D3DISSUE_BEGIN); 
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Issue an occlusion query end.
 *
 * \param	qo	Query object obtained using allocateQuery
 *
 *//*-------------------------------------------------------------------*/

void Platform::endQuery(Query* qo)
{
    qo->platformQuery->Issue(D3DISSUE_END); 
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
    bool availability = false;

    if (waitForResult)
    {
        // loop until result is ready
        for (;;)
        {
            int ret = qo->platformQuery->GetData(&visibleSamples, sizeof(DWORD), D3DGETDATA_FLUSH);
            
            if (ret == S_OK)
            {
                availability = true;
                break;
            }
            if (ret == D3DERR_DEVICELOST)
            {
                availability = false;
                break;
            }          
        }        
    }
    else
    {
        // don't wait for the result if it's not ready
        if (qo->platformQuery->GetData(&visibleSamples, sizeof(DWORD), D3DGETDATA_FLUSH) == S_OK)
            availability = true;
    }

	return availability;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets rendering state, depending on whether we're doing the
 *          depth pass, color pass or rendering occlusion queries.
 *//*-------------------------------------------------------------------*/

void Platform::setState(Platform::State s)
{
	// Ignore redundant calls
	if (g_state == s)
        return;

    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

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
            //DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            D3D->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE);
            D3D->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            D3D->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
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
            //DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            D3D->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
            D3D->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            D3D->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            D3D->SetVertexShader(g_depthVertexShader);
            D3D->SetPixelShader(g_depthPixelShader);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering the color pass. 
    //--------------------------------------------------------------------
    case COLOR_DRAW_STATE:
        {
            //DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			//DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            D3D->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
            D3D->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
            D3D->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            D3D->SetVertexShader(g_colorVertexShader);
            D3D->SetPixelShader (g_colorPixelShader);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering debug lines pass. Similiar to 
	// COLOR_DRAW_STATE, but uses the flat color pixel shader.
    //--------------------------------------------------------------------
    case DEBUGLINE_DRAW_STATE:
        {
            //DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            D3D->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
            D3D->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
            D3D->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
            D3D->SetVertexShader(g_colorVertexShader);
            D3D->SetPixelShader (g_flatColorPixelShader);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering wireframe.
    //--------------------------------------------------------------------
    case WIREFRAME_DRAW_STATE:
        {
            //DirectXGlobals::getDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            D3D->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
            D3D->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
            D3D->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            D3D->SetVertexShader(g_colorVertexShader);
            D3D->SetPixelShader (g_flatColorPixelShader);
        }
    }

    g_state = s;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the view matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setViewMatrix(const Matrix4x4& matrix)
{
    DirectXGlobals::setViewMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the projection matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setProjectionMatrix(const Matrix4x4& matrix)
{
    DirectXGlobals::setProjMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the world matrix to be used when rendering
 *//*-------------------------------------------------------------------*/

void Platform::setWorldMatrix(const Matrix4x4& matrix)
{
    DirectXGlobals::setWorldMatrix((Matrix4x4&)matrix);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Get the current WVP-matrix
 *//*-------------------------------------------------------------------*/

static void getWVP(Matrix4x4& matrix)
{
    Matrix4x4 world      = DirectXGlobals::getWorldMatrix();
    Matrix4x4 view       = DirectXGlobals::getViewMatrix();
    Matrix4x4 projection = DirectXGlobals::getProjMatrix();

    matrix = ((world * view) * projection);
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
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

    D3D->SetVertexDeclaration(g_colorInputLayout);

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* object = objects[i];

        Matrix4x4 objectToCell = object->getObjectToCellMatrix();
        DirectXGlobals::setWorldMatrix(objectToCell);

        Matrix4x4 matWVP;
        getWVP((Matrix4x4&)matWVP);
        D3D->SetVertexShaderConstantF(0, (FLOAT*)&matWVP, 4);

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
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

    D3D->SetVertexDeclaration(g_colorInputLayout);

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* object = objects[i];

        shader->preRender(object);

        Matrix4x4 objectToCell = object->getObjectToCellMatrix();
        DirectXGlobals::setWorldMatrix(objectToCell);

        Matrix4x4 matWVP;
        getWVP((Matrix4x4&)matWVP);
        D3D->SetVertexShaderConstantF(0, (FLOAT*)&matWVP, 4);

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
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();            

    //--------------------------------------------------------------------
    // Setup the vertex shader
    //--------------------------------------------------------------------

    D3D->SetVertexShader(g_colorVertexShader);
    Matrix4x4 ident;

    D3D->SetVertexShaderConstantF( 0, (FLOAT*)&ident, 4 );

    //--------------------------------------------------------------------
    // Setup the pixel shader
    //--------------------------------------------------------------------

    D3D->SetPixelShader (g_colorPixelShader);
    setColor(color);

    D3D->SetFVF(D3DFVF_XYZ);
	// Render using DrawPrimitiveUP
    D3D->DrawPrimitiveUP(D3DPT_LINELIST, nLines, lines, sizeof(Vector3));
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders 3D lines.
 *//*-------------------------------------------------------------------*/

void Platform::drawLines(const Vector3* lines, int nLines, const Vector4& color)
{
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();            

    //--------------------------------------------------------------------
    // Setup the vertex shader
    //--------------------------------------------------------------------

    D3D->SetVertexShader(g_colorVertexShader);
    Matrix4x4 ident;
    DirectXGlobals::setWorldMatrix(ident);

    Matrix4x4 matWVP;
    getWVP((Matrix4x4&)matWVP);

    D3D->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );

    //--------------------------------------------------------------------
    // Setup the pixel shader
    //--------------------------------------------------------------------

    D3D->SetPixelShader (g_flatColorPixelShader);
    //setLight();
    setColor(color);

    //D3D->SetFVF(D3DFVF_XYZ);

	// Render using DrawPrimitiveUP
    D3D->DrawPrimitiveUP(D3DPT_LINELIST, nLines, lines, sizeof(Vector3));
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
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

    //--------------------------------------------------------------------
    // Setup the vertex shader
    //--------------------------------------------------------------------

    D3D->SetVertexDeclaration   (g_vertexDeclPosOnly);
    D3D->SetVertexShader        (g_queryVertexShader);

    Matrix4x4 ident;
    DirectXGlobals::setWorldMatrix(ident);

    Matrix4x4 matWVP;
    getWVP((Matrix4x4&)matWVP);
    D3D->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );

    //--------------------------------------------------------------------
    // Setup the pixel shader
    //--------------------------------------------------------------------

    D3D->SetPixelShader (g_depthPixelShader);
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

void Platform::drawIndexed(int vertexOffset, int vertexCount, int triangleCount, int start)
{
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();            
    D3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertexOffset, 0, vertexCount, start, triangleCount);
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

void Platform::drawIndexedUP(const Vector3* vertices, int vertexCount, const Vector3i* triangles, int triangleCount)
{
	IDirect3DDevice9* D3D = DirectXGlobals::getDevice();

	D3D->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST,	// PrimitiveType
		0,					// MinVertexIndex
		vertexCount,		// NumVertices
		triangleCount,		// PrimitiveCount
		triangles,			// pIndexData
		D3DFMT_INDEX32,		// IndexDataFormat
		vertices,			// pVertexStreamZeroData
		sizeof(Vector3)		// VertexStreamZeroStride
		); 
}

/*-------------------------------------------------------------------*//*!
 * \brief	Clears the depth and stencil buffers.
 *//*-------------------------------------------------------------------*/

void Platform::clearDepthStencil(void)
{
  DirectXGlobals::getDevice()->Clear(0, NULL, D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0, 1.f, 0);
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
    return new D3D9Mesh(vertices, normals, triangles, nVertices, nTriangles);
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

void Platform::fontMeasure(int &width, int &height, const char* text, ...)
{
    char str[256] = "";

	va_list args;
	va_start (args, text);
	vsprintf_s (str, 256, text, args);
	va_end (args);

	RECT			rect;

	rect.top = 0;
	rect.left = 0;
	rect.bottom = g_renderHeight;
	rect.right = g_renderWidth;

	PLATFORM_ASSERT(g_font);

	g_font->DrawTextA(NULL, str, -1, &rect, DT_LEFT|DT_TOP|DT_NOCLIP|DT_CALCRECT, 0);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
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
    char str[256] = "";

	va_list args;
	va_start (args, text);
	vsprintf_s (str, 256, text, args);
	va_end (args);

	RECT			rect;

	rect.top = y;
	rect.left = x;
	rect.bottom = g_renderHeight;
	rect.right = g_renderWidth;

	PLATFORM_ASSERT(g_font);
	
	D3DCOLOR d3dColor = 		
		D3DCOLOR_RGBA(
		(int)(color.x*255.f), 
		(int)(color.y*255.f), 
		(int)(color.z*255.f), 
		(int)(color.w*255.f)
		);

	g_font->DrawTextA(NULL, str, -1, &rect, DT_LEFT|DT_NOCLIP|DT_TOP, d3dColor);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active index buffer.
 *
 * \param	indexBuffer		index buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setIndexBuffer(IndexBuffer* indexBuffer)
{
    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();
	HRESULT rv = D3D->SetIndices(indexBuffer->platformBuffer);
	PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active vertex buffer.
 *
 * \param	vertexBuffer	vertex buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setVertexBuffer(VertexBuffer* vertexBuffer)
{
    IDirect3DDevice9* D3D = DirectXGlobals::getDevice();
    HRESULT rv = D3D->SetStreamSource(0, vertexBuffer->platformBuffer, 0, sizeof(Vector3));
    PLATFORM_ASSERT(rv == D3D_OK);
    (void)rv;
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
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void Platform::unbindGraphicsThread(void)
{
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
    (void)depth;
    (void)stencil;
    DirectXGlobals::getDevice()->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_COLORVALUE(r, g, b, a), 1.f, 0);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the current view port.
 *//*-------------------------------------------------------------------*/

void Platform::setViewport(int x, int y, int w, int h)
{
    D3DVIEWPORT9 vp;
    vp.X = x;
    vp.Y = y;
    vp.Width = w;
    vp.Height = h;
    vp.MinZ = 0.f;
    vp.MaxZ = 1.f;
    DirectXGlobals::getDevice()->SetViewport(&vp);
}


/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame starts.
 *//*-------------------------------------------------------------------*/

bool Platform::beginFrame(void)
{
    return DirectXGlobals::getDevice()->BeginScene() == D3D_OK;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame ends.
 *//*-------------------------------------------------------------------*/

void Platform::endFrame(void)
{
    DirectXGlobals::getDevice()->EndScene();
    DirectXGlobals::getDevice()->Present(NULL, NULL, NULL, NULL);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Flush rendering.
 *//*-------------------------------------------------------------------*/

void Platform::flush(void)
{
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void Platform::finish(void)
{
    // \todo implement!
    //DirectXGlobals::getDevice()->
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

    assert(DirectXGlobals::getDevice());

    bool success = doinit((void*)DirectXGlobals::getDevice());
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

