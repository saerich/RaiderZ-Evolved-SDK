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
 * outlined in signed agreement may result in irreparable harm to 
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief     Platform abstraction for Direct3D 10
 *
 *//*-------------------------------------------------------------------*/

#include <d3d10.h>
#include <d3dx10.h>
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
		VertexBuffer(ID3D10Buffer *value) 
			: platformBuffer(value) {}

		ID3D10Buffer* platformBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(ID3D10Buffer *value) 
			: platformBuffer(value) {}

		ID3D10Buffer* platformBuffer;
	};

	class Query
	{
	public:
		Query(ID3D10Query *value) 
			: platformQuery(value) {}

		ID3D10Query* platformQuery;
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

static int g_renderWidth;
static int g_renderHeight;

static Platform::State g_state;

ID3D10DepthStencilView*     g_depthStencilView = NULL;
ID3D10BlendState*           g_pBlendStateNoColorWrite = NULL;
ID3D10DepthStencilState*    g_depthStateWriteDepth = NULL;
ID3D10DepthStencilState*    g_depthStateTestOnly = NULL;
ID3D10Buffer*               g_vertexBuffer = NULL;
ID3D10Buffer*               g_indexBuffer = NULL;
ID3D10RasterizerState*      g_rasterizerSolid = NULL;
ID3D10RasterizerState*      g_rasterizerWireframe = NULL;

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
ID3DX10Font*				g_font			= NULL;       
// Font height in pixels
const int					g_fontHeight	= 20;

// Mouse button state
unsigned int mouseButtonBitmap	= 0;

// True if window is active
bool windowActive = false;

/*-------------------------------------------------------------------*//*!
 * Vertex shader for the depth pass.
 *//*-------------------------------------------------------------------*/

static ID3D10Buffer* g_constantBuffer;
static ID3D10Buffer* g_colorConstantBuffer = NULL;

static ID3D10VertexShader* g_depthVertexShader = NULL;

const char* g_strDepthVertexShaderProgram = 
" cbuffer WVP		: register(b0)              "
" {                                             "
" float4x4 matWVP;                              "
" }                                             "
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

static ID3D10VertexShader * g_queryVertexShader = NULL;
static ID3D10InputLayout* g_queryInputLayout = NULL;

const char* g_strQueryVertexShaderProgram = 
" cbuffer WVP		: register(b0)              "
" {                                             "
" float4x4 matWVP;                              "
" }                                             "
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

static ID3D10PixelShader * g_depthPixelShader = NULL;

const char* g_strDepthPixelShaderProgram = 
" struct PS_OUT_DEPTH                          "
" {                                            "
"     float4 Color : COLOR;                    "
"	  float3 pos		: POSITION;            "
" };                                           "
" struct PS_IN_COLOR                           "
" {                                            "
" 	float4 pos		: POSITION;                "
" };                                           "

"                                                       "
" PS_OUT_DEPTH PS_Main_Depth(PS_IN_COLOR IN) : COLOR    "
" {                                                     "
"   PS_OUT_DEPTH OUT;                                   "
"	OUT.Color = float4(1,1,1,1);                        "
"	return OUT;                                         "
" }                                                     ";

/*-------------------------------------------------------------------*//*!
 * Vertex shader for color pass.
 *//*-------------------------------------------------------------------*/

static ID3D10VertexShader* g_colorVertexShader = NULL;
static ID3D10InputLayout* g_colorInputLayout = NULL;

const char* g_strColorVertexShaderProgram = 
" cbuffer WVP		: register(b0)                          "
" {                                                         "
" float4x4 matWVP;                                          "
" }                                                         "
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
" 	float3 normal	: NORMAL;                              "
" };                                                        "
"                                                           "
" VS_OUT_COLOR VS_Main_Color(VS_IN_COLOR IN)                "
" {                                                         "
" 	VS_OUT_COLOR OUT;                                       "
" 	OUT.pos = mul(matWVP, float4(IN.pos, 1));               "
" 	OUT.normal = mul((float3x3)matWVP, IN.normal);			"
" 	return OUT;                                             "
" }                                                         ";

/*-------------------------------------------------------------------*//*!
 * Pixel shader for color pass.
 *//*-------------------------------------------------------------------*/

static ID3D10PixelShader* g_colorPixelShader = NULL;

const char* g_strColorPixelShaderProgram =
" cbuffer Color		: register(b0)									"
" {																	"
" float4 color;														"
" }																	"
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

static ID3D10PixelShader* g_flatColorPixelShader = NULL;

const char* g_strFlatColorPixelShaderProgram =
" cbuffer Color		: register(b0)									"
" {																	"
" float4 color;														"
" }																	"
"                                                                   "
" struct PS_IN_COLOR                                                "
" {                                                                 "
" 	float4 pos		: POSITION;                                     "
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
"                                                                   "
" 	OUT.color = color;                                              "
" 	return OUT;                                                     "
" }                                                                 ";

/*-------------------------------------------------------------------*//*!
 * DirectX globals.
 *//*-------------------------------------------------------------------*/

class DirectXGlobals
{
public:
    static ID3D10Device*                getDevice(void)                 { PLATFORM_ASSERT(s_device); return s_device; }
    static void                         setDevice(ID3D10Device* d)      { s_device = d; }

    static ID3D10RenderTargetView*      getRenderTargetView(void)                       { PLATFORM_ASSERT(s_renderTargetView); return s_renderTargetView; }
    static void                         setRenderTargetView(ID3D10RenderTargetView* d)  { s_renderTargetView = d; }

    static IDXGISwapChain*              getSwapChain(void)                 { PLATFORM_ASSERT(s_swapChain); return s_swapChain; }
    static void                         setSwapChain(IDXGISwapChain* d)    { s_swapChain = d; }

    static const Matrix4x4&             getViewMatrix (void)                    { return s_matView; }
    static void                         setViewMatrix (const Matrix4x4& matrix) { s_matView = matrix; } 

    static const Matrix4x4&             getProjMatrix (void)                    { return s_matProj; }
    static void                         setProjMatrix (const Matrix4x4& matrix) { s_matProj = matrix; } 

    static const Matrix4x4&             getWorldMatrix (void)                    { return s_matWorld; }
    static void                         setWorldMatrix (const Matrix4x4& matrix) { s_matWorld = matrix; } 

private:

	DirectXGlobals();

    static ID3D10Device*            s_device;
    static IDXGISwapChain*          s_swapChain;
    static ID3D10RenderTargetView*  s_renderTargetView;

    static Matrix4x4             s_matView;       // View matrix 
    static Matrix4x4             s_matProj;       // Projection matrix
    static Matrix4x4             s_matWorld;      // World matrix
};


ID3D10Device*           DirectXGlobals::s_device = NULL;
IDXGISwapChain*         DirectXGlobals::s_swapChain = NULL;
ID3D10RenderTargetView* DirectXGlobals::s_renderTargetView = NULL;

Matrix4x4          DirectXGlobals::s_matView;
Matrix4x4          DirectXGlobals::s_matProj;
Matrix4x4          DirectXGlobals::s_matWorld;

// Dynamic buffers used by D3D10drawUP implementation (rendering from a user pointers).
static VertexBuffer*		g_dynamicVB			= 0; 
static IndexBuffer*			g_dynamicIB			= 0; 
// D3D10drawUP is used for query rendering and debug lines. Allocate enough space to avoid
// flushing the buffers too many times during a frame.
static const unsigned int	g_dynamicVBSize		= 1024*10;// * sizeof(Vector3);
static const unsigned int	g_dynamicIBSize		= 1024*20;// * sizeof(unsigned int);
static unsigned int			g_dynamicVBPos		= 0;
static unsigned int			g_dynamicIBPos		= 0;

// Custom function which renders indexed/nonindexed geometry from user pointers
static void D3D10drawUP(const Vector3* vertices, int vertexCount, const unsigned int* indices, int indexCount, D3D10_PRIMITIVE_TOPOLOGY topology);

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Allocates an occlusion query.
 *
 *//*-------------------------------------------------------------------*/

Query* Platform::allocateQuery(void)
{
    ID3D10Device* D3D = DirectXGlobals::getDevice();
    ID3D10Query*  pOcclusionQuery = NULL;

    D3D10_QUERY_DESC desc;
    desc.Query = D3D10_QUERY_OCCLUSION;
    desc.MiscFlags = 0;

    HRESULT res = D3D->CreateQuery(&desc, &pOcclusionQuery);
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
 * \brief   Creates a DX10 vertex buffer.
 *
 * \param   size    The size of the vertex buffer in bytes.
 *
 *//*-------------------------------------------------------------------*/

VertexBuffer* Platform::createVertexBuffer(int size)
{
    PLATFORM_ASSERT(size > 0);

    ID3D10Device* D3D = DirectXGlobals::getDevice();
    PLATFORM_ASSERT(D3D);

    D3D10_BUFFER_DESC BufferDesc;
    BufferDesc.Usage             = D3D10_USAGE_DYNAMIC;
    BufferDesc.ByteWidth         = size;
    BufferDesc.BindFlags         = D3D10_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags    = D3D10_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags         = 0;

    ID3D10Buffer* vb;
    D3D->CreateBuffer(&BufferDesc,
                      NULL,
                      &vb);

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

    ULONG rv =  buffer->platformBuffer->Release();
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

void* Platform::lockVertexBuffer(VertexBuffer* buffer, int offset, int size)
{
    PLATFORM_ASSERT(buffer);
    PLATFORM_ASSERT(size > 0);
    (void)size;
    (void)offset;
    void* pData = NULL;

	HRESULT rv = buffer->platformBuffer->Map(D3D10_MAP_WRITE_DISCARD,
                         0,
                         &pData);
    PLATFORM_ASSERT(rv == S_OK);
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
    buffer->platformBuffer->Unmap();
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

    ID3D10Device* D3D = DirectXGlobals::getDevice();
    PLATFORM_ASSERT(D3D);

    D3D10_BUFFER_DESC BufferDesc;
    BufferDesc.Usage             = D3D10_USAGE_DYNAMIC;
    BufferDesc.ByteWidth         = size;
    BufferDesc.BindFlags         = D3D10_BIND_INDEX_BUFFER;
    BufferDesc.CPUAccessFlags    = D3D10_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags         = 0;

    ID3D10Buffer* ib;
    D3D->CreateBuffer(&BufferDesc,
                      NULL,
                      &ib);

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
    (void)size;
    (void)offset;
    void* pData = NULL;

	HRESULT rv = buffer->platformBuffer->Map(D3D10_MAP_WRITE_DISCARD,
                         0,
                         &pData);
    PLATFORM_ASSERT(rv == S_OK);
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
    buffer->platformBuffer->Unmap();
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief     The DirectX 10 specific implementation of the Mesh class
 *
 *//*-------------------------------------------------------------------*/

class D3D10Mesh : public Mesh
{
public:

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Constructor for D3D10 mesh.
	 *
	 *		   Creates vertex and index buffers to hold mesh data,
	 *		   and fills them with the supplied data.
	 *
	 *//*-------------------------------------------------------------------*/

	D3D10Mesh(const Vector3* vertices, const Vector3* normals, const Vector3i* triangles, int nVertices, int nTriangles) :
      m_numTriangles(nTriangles),
      m_numVertices (nVertices)
    {
        ID3D10Device* D3D = DirectXGlobals::getDevice();
        PLATFORM_ASSERT(D3D);

        int numv = nVertices;
        int nump = nTriangles;

		//--------------------------------------------------------------------
        // Create vertex buffer to hold mesh vertex and normal data
        //--------------------------------------------------------------------

        Vector3* vArray = new Vector3[numv*2];
        BYTE* pData = (BYTE*)vArray;

        for(int i = 0; i < numv; i++)
        {
            *vArray++ = vertices[i];
            *vArray++ = normals[i];
        }

        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.Usage             = D3D10_USAGE_DEFAULT;
        BufferDesc.ByteWidth         = numv*sizeof(Vector3)*2;
        BufferDesc.BindFlags         = D3D10_BIND_VERTEX_BUFFER;
        BufferDesc.CPUAccessFlags    = 0;
        BufferDesc.MiscFlags         = 0;

        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pData;
	    InitData.SysMemPitch = 0;
	    InitData.SysMemSlicePitch = 0;

        D3D->CreateBuffer(&BufferDesc,
                          &InitData,
                          &m_vertexBuffer);

        delete[] pData;

        if(!m_vertexBuffer)
            return;

        //--------------------------------------------------------------------
        // Create index buffer
        //--------------------------------------------------------------------

        unsigned short* iArray = new unsigned short[nump*3];
        pData = (BYTE*)iArray;

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

        BufferDesc.Usage             = D3D10_USAGE_DEFAULT;
        BufferDesc.ByteWidth         = nump*sizeof(short)*3;
        BufferDesc.BindFlags         = D3D10_BIND_INDEX_BUFFER;
        BufferDesc.CPUAccessFlags    = 0;
        BufferDesc.MiscFlags         = 0;

        InitData.pSysMem = pData;
	    InitData.SysMemPitch = 0;
	    InitData.SysMemSlicePitch = 0;

        D3D->CreateBuffer(&BufferDesc,
                          &InitData,
                          &m_indexBuffer);
        delete pData;
        
        if(!m_indexBuffer)
        {
            m_vertexBuffer->Release();
            return;
        }
    };

	/*-------------------------------------------------------------------*//*!
	 *
	 * \brief  Renders the mesh.
	 *
	 *//*-------------------------------------------------------------------*/

    virtual void draw (void) const 
    {
        ID3D10Device* D3D = DirectXGlobals::getDevice();

        //--------------------------------------------------------------------
        // Setup the drawing
        //--------------------------------------------------------------------

        UINT stride = sizeof(Vector3)*2;
        UINT offset = 0;

        D3D->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
        D3D->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

        //--------------------------------------------------------------------
        // Draw the mesh
        //--------------------------------------------------------------------

        D3D->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        D3D->DrawIndexed(  m_numTriangles*3, 0, 0 );
    }

private:

    ID3D10Buffer*       m_vertexBuffer;
    ID3D10Buffer*       m_indexBuffer;
    ID3D10InputLayout*  m_inputLayout;
    int                 m_numTriangles;
    int                 m_numVertices;
};

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
 * \param	p		Platform-specific parameter. In this case, HWND.
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

    g_renderWidth = mode.width;
    g_renderHeight = mode.height;

	// Create IDXGIFactory to be used to create device and swap chain
	IDXGIFactory* DXGIFactory;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&DXGIFactory) );
    
	if ( FAILED(hr) )
        return false;   

	// Get first adapter to create the device with
	IDXGIAdapter* DXGIAdapter;
	hr = DXGIFactory->EnumAdapters(0, &DXGIAdapter);

	if ( FAILED(hr) )
        return false;   

	ID3D10Device* dev;

	// Create D3D10 device
	hr = D3D10CreateDevice(
	  DXGIAdapter,
	  D3D10_DRIVER_TYPE_HARDWARE,
	  NULL,
	  /*D3D10_CREATE_DEVICE_DEBUG*/ 0,
	  D3D10_SDK_VERSION,
	  &dev
	);

	if ( FAILED(hr) )
        return false;  

	unsigned int numQualityLevels = 0, numDepthQualityLevels = 0;
	
	// Check number of quality levels available for color and depth buffers with
	// given multisample count
	hr = dev->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, mode.multisamples, &numQualityLevels);

	if ( FAILED(hr) )
        return false;  

	hr = dev->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, mode.multisamples, &numDepthQualityLevels);

	if ( FAILED(hr) )
        return false;  

	unsigned int sampleCount = mode.multisamples;
	unsigned int sampleQuality = 0;

	if(numQualityLevels > 0 && numDepthQualityLevels > 0)
	{
		// Choose automatically the best available quality 
		sampleQuality = min(numQualityLevels, numDepthQualityLevels) - 1;
	} else
	{
		// Zero output from CheckMultisampleQualityLevels indicates multisample mode is not supported
		sampleCount = 1;
		PLATFORM_ASSERT("Unsupported multisample mode" && false);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof(sd) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = g_renderWidth;
    sd.BufferDesc.Height = g_renderHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = sampleCount;
    sd.SampleDesc.Quality = sampleQuality;
    sd.Windowed = mode.windowed;

    IDXGISwapChain* swapChain;

	hr = DXGIFactory->CreateSwapChain(dev, &sd, &swapChain);

    if ( FAILED(hr) )
        return false; 

	DXGIFactory->Release();

     // Create a render target view

    ID3D10RenderTargetView* renderTargetView;

    ID3D10Texture2D *pBackBuffer;
    if( FAILED( swapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), (LPVOID*)&pBackBuffer ) ) )
        return FALSE;
    hr = dev->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return FALSE;
    
    ID3D10Texture2D* pDepthStencil = NULL;
    D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = 1280;
    descDepth.Height = 720;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = sampleCount;
    descDepth.SampleDesc.Quality = sampleQuality;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = dev->CreateTexture2D( &descDepth, NULL, &pDepthStencil );

    // Create the depth stencil view

    hr = dev->CreateDepthStencilView( pDepthStencil, // Depth stencil texture
                                      NULL, // Depth stencil desc
                                      &g_depthStencilView );  // [out] Depth stencil view


    dev->OMSetRenderTargets( 1, &renderTargetView, g_depthStencilView );

    //--------------------------------------------------------------------
    // Set the D3D device to the global container
    //--------------------------------------------------------------------

    DirectXGlobals::setDevice(dev);
    DirectXGlobals::setSwapChain(swapChain);
    DirectXGlobals::setRenderTargetView(renderTargetView);

    D3D10_VIEWPORT vp;
    vp.Width = 1280;
    vp.Height = 720;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    dev->RSSetViewports( 1, &vp );

    if( FAILED(hr) )
        return false;

    ID3D10Blob* pVertexShaderCode;
    ID3D10Blob* pVertexErrorMsg;

    //--------------------------------------------------------------------
    // Compile the depth pass vertex shader and create the vertex shader
    // Object
    //--------------------------------------------------------------------

    hr = D3DX10CompileFromMemory( g_strDepthVertexShaderProgram,
                             (UINT)strlen( g_strDepthVertexShaderProgram ),
                             NULL, NULL, NULL, 
                             "VS_Main_Depth", "vs_4_0", 0, 
                              0, NULL, &pVertexShaderCode, &pVertexErrorMsg, NULL );

    if( FAILED(hr) )
        return false;

    // Create the vertex shader
    hr = dev->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(),
        pVertexShaderCode->GetBufferSize(), &g_depthVertexShader );


    // Release the pointer to the compiled shader once you are done with it
    pVertexShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the query test shape vertex shader and create the vertex shader
    // Object
    //--------------------------------------------------------------------

    hr = D3DX10CompileFromMemory( g_strQueryVertexShaderProgram, 
                             (UINT)strlen( g_strQueryVertexShaderProgram ),
                             NULL, NULL, NULL, 
                             "VS_Main_Query", "vs_4_0", 0, 0, NULL,
                              &pVertexShaderCode, &pVertexErrorMsg, NULL );

    if( FAILED(hr) )
        return false;

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC queryLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
    };

    UINT numElements = sizeof(queryLayout)/sizeof(queryLayout[0]);

    hr = dev->CreateInputLayout(queryLayout, numElements,
          (DWORD*)pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(),
          &g_queryInputLayout
        );

    hr = dev->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(),
        pVertexShaderCode->GetBufferSize(), &g_queryVertexShader );

    pVertexShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the color pass vertex shader and create the vertex shader
    // Object
    //--------------------------------------------------------------------

    hr = D3D10CompileShader( g_strColorVertexShaderProgram, 
                         (UINT)strlen( g_strColorVertexShaderProgram ),
                         NULL, NULL, NULL, 
                         "VS_Main_Color", "vs_4_0", 0, 
                          &pVertexShaderCode, &pVertexErrorMsg );

    if( FAILED(hr) )
        return false;

    hr = dev->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(),
        pVertexShaderCode->GetBufferSize(), &g_colorVertexShader  );

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC colorLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };

    numElements = sizeof(colorLayout)/sizeof(colorLayout[0]);

    hr = dev->CreateInputLayout(colorLayout, numElements,
          (DWORD*)pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(),
          &g_colorInputLayout
        );


    // Release the pointer to the compiled shader once you are done with it
    pVertexShaderCode->Release();

    ID3D10Blob* pPixelShaderCode;
    ID3D10Blob* pPixelErrorMsg;

    //--------------------------------------------------------------------
    // Compile the depth pass pixel shader and create the pixel shader
    // Object
    //--------------------------------------------------------------------

    hr = D3D10CompileShader( g_strDepthPixelShaderProgram, 
                     (UINT)strlen( g_strDepthPixelShaderProgram ),
                     NULL, NULL, NULL, 
                     "PS_Main_Depth", "ps_4_0", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 
                      &pPixelShaderCode, &pPixelErrorMsg );

    if( FAILED(hr) )
    {
        char* str = (char*)pPixelErrorMsg->GetBufferPointer();
        (void)str;
        return false;
    }

    DirectXGlobals::getDevice()->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                                                    pPixelShaderCode->GetBufferSize(),
                                                   &g_depthPixelShader );

	pPixelShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the color pass pixel shader and create the pixel shader
    // Object
    //--------------------------------------------------------------------

    hr = D3D10CompileShader( g_strColorPixelShaderProgram, 
                 (UINT)strlen( g_strColorPixelShaderProgram ),
                 NULL, NULL, NULL, 
                 "PS_Main_Color", "ps_4_0", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 
                  &pPixelShaderCode, &pPixelErrorMsg );

    if( FAILED(hr) )
    {
        char* str = (char*)pPixelErrorMsg->GetBufferPointer();
        (void)str;
        return false;
    }

    DirectXGlobals::getDevice()->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                                                    pPixelShaderCode->GetBufferSize(),
                                                   &g_colorPixelShader );

	pPixelShaderCode->Release();

    //--------------------------------------------------------------------
    // Compile the debug line flat color pixel shader and create the pixel 
	// shader Object
    //--------------------------------------------------------------------

    hr = D3D10CompileShader( g_strFlatColorPixelShaderProgram, 
                 (UINT)strlen( g_strFlatColorPixelShaderProgram ),
                 NULL, NULL, NULL, 
                 "PS_Main_Color", "ps_4_0", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 
                  &pPixelShaderCode, &pPixelErrorMsg );

    if( FAILED(hr) )
    {
        char* str = (char*)pPixelErrorMsg->GetBufferPointer();
        (void)str;
        return false;
    }

    DirectXGlobals::getDevice()->CreatePixelShader((DWORD*)pPixelShaderCode->GetBufferPointer(),
                                                    pPixelShaderCode->GetBufferSize(),
                                                   &g_flatColorPixelShader );

	pPixelShaderCode->Release();

    D3D10_BUFFER_DESC BufferDesc;
    BufferDesc.Usage             = D3D10_USAGE_DEFAULT;
    BufferDesc.ByteWidth         = 16*sizeof(float);
    BufferDesc.BindFlags         = D3D10_BIND_CONSTANT_BUFFER;
    BufferDesc.CPUAccessFlags    = 0;
    BufferDesc.MiscFlags         = 0;

    dev->CreateBuffer(&BufferDesc,
                      NULL,
                      &g_constantBuffer);

    D3D10_BUFFER_DESC BufferDesc2;
    BufferDesc2.Usage             = D3D10_USAGE_DEFAULT;
    BufferDesc2.ByteWidth         = 4*sizeof(float);
    BufferDesc2.BindFlags         = D3D10_BIND_CONSTANT_BUFFER;
    BufferDesc2.CPUAccessFlags    = 0;
    BufferDesc2.MiscFlags         = 0;

    dev->CreateBuffer(&BufferDesc2,
                      NULL,
                      &g_colorConstantBuffer);

    D3D10_BLEND_DESC BlendStateDepthWrite;
    ZeroMemory(&BlendStateDepthWrite, sizeof(D3D10_BLEND_DESC));
    BlendStateDepthWrite.BlendEnable[0] = FALSE;
    BlendStateDepthWrite.RenderTargetWriteMask[0] = 0;

    dev->CreateBlendState(&BlendStateDepthWrite, &g_pBlendStateNoColorWrite);

	PLATFORM_ASSERT(hr == S_OK);

    D3D10_DEPTH_STENCIL_DESC dsDesc;

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D10_COMPARISON_LESS_EQUAL;

    // Stencil test parameters
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Create depth stencil states
    
    dev->CreateDepthStencilState(&dsDesc, &g_depthStateWriteDepth);

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D10_COMPARISON_LESS_EQUAL;

    dev->CreateDepthStencilState(&dsDesc, &g_depthStateTestOnly);


    // Create solid and wireframe rasterizer states
    D3D10_RASTERIZER_DESC rsDesc;
    rsDesc.FillMode = D3D10_FILL_SOLID;
    rsDesc.CullMode = D3D10_CULL_FRONT;
    rsDesc.FrontCounterClockwise = true;
    rsDesc.DepthBias = false;
    rsDesc.DepthBiasClamp = 0;
    rsDesc.SlopeScaledDepthBias = 0;
    rsDesc.DepthClipEnable = true;
    rsDesc.ScissorEnable = false;
    rsDesc.AntialiasedLineEnable = false;
	
	if(sampleCount > 1)
		rsDesc.MultisampleEnable = true;
	else
		rsDesc.MultisampleEnable = false;

    dev->CreateRasterizerState(&rsDesc, &g_rasterizerSolid);
    rsDesc.FillMode = D3D10_FILL_WIREFRAME;
    dev->CreateRasterizerState(&rsDesc, &g_rasterizerWireframe);

	D3D10_QUERY_DESC queryDesc;

	queryDesc.Query = D3D10_QUERY_EVENT;
	queryDesc.MiscFlags = 0;
	
	g_dynamicVB = Platform::createVertexBuffer( g_dynamicVBSize * sizeof(Vector3) );
	g_dynamicIB = Platform::createIndexBuffer(  g_dynamicIBSize * sizeof(unsigned int) );
    
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

	if( FAILED( hr = D3DX10CreateFont( dev, g_fontHeight, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
							           OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
									    L"Arial", &g_font ) ) )
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
 * \brief	Helper function template to release D3D10 objects.
 *//*-------------------------------------------------------------------*/

template<typename T>
void D3D10release(T& object)
{
	if(object)
	{
		object->Release();
		object = NULL;
	}
}

/*-------------------------------------------------------------------*//*!
 * \brief	Platform exit. Cleans up.
 *//*-------------------------------------------------------------------*/

void Platform::exit(void)
{
	if(g_dynamicVB)
	{
		deleteVertexBuffer(g_dynamicVB);
		g_dynamicVB = NULL;
	}

	if(g_dynamicIB) 
	{
		deleteIndexBuffer(g_dynamicIB);
		g_dynamicIB = NULL;
	}

	D3D10release(g_font);

	D3D10release(g_rasterizerWireframe);
	D3D10release(g_rasterizerSolid);
	
	D3D10release(g_depthStateTestOnly);
	D3D10release(g_depthStateWriteDepth);
	D3D10release(g_pBlendStateNoColorWrite);

	D3D10release(g_colorConstantBuffer);

	D3D10release(g_constantBuffer);

	D3D10release(g_flatColorPixelShader);
	D3D10release(g_colorPixelShader);
	D3D10release(g_depthPixelShader);
	D3D10release(g_flatColorPixelShader);

	D3D10release(g_colorVertexShader);
	D3D10release(g_queryVertexShader);
	D3D10release(g_depthVertexShader);

	D3D10release(g_colorInputLayout);
	D3D10release(g_queryInputLayout);
	
	D3D10release(g_depthStencilView);	

	if(DirectXGlobals::getDevice())
		DirectXGlobals::getDevice()->Release();
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

void Platform::readKeyboard(KeyboardState &keyboardState)
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
    for( DWORD i=0; i < XUSER_MAX_COUNT; i++ )
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
    qo->platformQuery->Begin();
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
    qo->platformQuery->End(); 
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

    UINT64 result = 0;

    if (waitForResult)
    {
        // loop until result is ready
        while (qo->platformQuery->GetData(&result, sizeof(UINT64), 0) != S_OK)
        {
        }
        availability = true;
    }
    else
    {
        // don't wait for the result if it's not ready
        if (qo->platformQuery->GetData(&result, sizeof(UINT64), 0) == S_OK)
            availability = true;
    }

    visibleSamples = (int)result;
	return availability;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets rendering state, depending on whether we're doing the
 *          depth pass, color pass or rendering occlusion queries.
 *//*-------------------------------------------------------------------*/

void Platform::setState(Platform::State s)
{
	if (g_state == s)
        return;

    ID3D10Device* D3D = DirectXGlobals::getDevice();

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
            D3D->OMSetDepthStencilState(g_depthStateTestOnly, 0);
            D3D->OMSetBlendState(g_pBlendStateNoColorWrite, 0, 0xffffffff);
            D3D->IASetInputLayout(g_queryInputLayout);
            D3D->VSSetShader(g_queryVertexShader);
            D3D->PSSetShader(g_depthPixelShader);
            D3D->VSSetConstantBuffers(0, 1, &g_constantBuffer);
			//D3D->PSSetConstantBuffers(0, 0, 0);
            D3D->RSSetState(g_rasterizerSolid);
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
            D3D->OMSetDepthStencilState(g_depthStateWriteDepth, 0);
            D3D->OMSetBlendState(g_pBlendStateNoColorWrite, 0, 0xffffffff);
            D3D->IASetInputLayout(g_colorInputLayout);
            D3D->VSSetShader(g_depthVertexShader);
            D3D->PSSetShader(g_depthPixelShader);
            D3D->VSSetConstantBuffers(0, 1, &g_constantBuffer);
			//D3D->PSSetConstantBuffers(0, 0, 0);
            D3D->RSSetState(g_rasterizerSolid);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering the color pass. 
    //--------------------------------------------------------------------
    case COLOR_DRAW_STATE:
        {
            D3D->OMSetDepthStencilState(g_depthStateWriteDepth, 0);
            D3D->OMSetBlendState(0, 0, 0xffffffff);
            D3D->IASetInputLayout(g_colorInputLayout);
            D3D->VSSetShader(g_colorVertexShader);
            D3D->PSSetShader(g_colorPixelShader);
            D3D->VSSetConstantBuffers(0, 1, &g_constantBuffer);
			D3D->PSSetConstantBuffers(0, 1, &g_colorConstantBuffer);

			D3D->RSSetState(g_rasterizerSolid);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering debug lines pass. Similiar to 
	// COLOR_DRAW_STATE, but uses the flat color pixel shader.
    //--------------------------------------------------------------------
    case DEBUGLINE_DRAW_STATE:
        {
            D3D->OMSetDepthStencilState(g_depthStateWriteDepth, 0);
            D3D->OMSetBlendState(0, 0, 0xffffffff);
            D3D->IASetInputLayout(g_queryInputLayout);
            D3D->VSSetShader(g_queryVertexShader);
            D3D->PSSetShader(g_flatColorPixelShader);
            D3D->VSSetConstantBuffers(0, 1, &g_constantBuffer);
			D3D->PSSetConstantBuffers(0, 1, &g_colorConstantBuffer);

            D3D->RSSetState(g_rasterizerSolid);
        }
        break;

	//--------------------------------------------------------------------
    // This mode is for rendering wireframe.
    //--------------------------------------------------------------------
    case WIREFRAME_DRAW_STATE:
        {
            D3D->OMSetDepthStencilState(g_depthStateWriteDepth, 0);
            D3D->OMSetBlendState(0, 0, 0xffffffff);
            D3D->IASetInputLayout(g_colorInputLayout);
            D3D->VSSetShader(g_colorVertexShader);
            D3D->PSSetShader(g_colorPixelShader);
            D3D->VSSetConstantBuffers(0, 1, &g_constantBuffer);
			D3D->PSSetConstantBuffers(0, 1, &g_colorConstantBuffer);

            D3D->RSSetState(g_rasterizerWireframe);
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
    ID3D10Device* D3D = DirectXGlobals::getDevice();

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* Object = objects[i];

        Matrix4x4 ObjectToCell = Object->getObjectToCellMatrix();
        DirectXGlobals::setWorldMatrix((Matrix4x4&)ObjectToCell);

        Matrix4x4 matWVP;
        getWVP((Matrix4x4&)matWVP);

        D3D->UpdateSubresource(g_constantBuffer,
                               0,
                               NULL,
                               &matWVP,16*4,
                               0);

        D3D->UpdateSubresource(g_colorConstantBuffer,
                               0,
                               NULL,
                               &Object->getColor(),
							   sizeof(Vector4),
                               0);

		Object->getMesh()->draw();
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Renders one or more objects with Shader.
 *
 * \param   objects     An array of objects to be drawn.
 * \param   objectCount Number of objects to draw.
 * \param   shader      Shader object to use when rendering
 *
 *//*-------------------------------------------------------------------*/

void Platform::draw(Object** objects, unsigned int objectCount, Shader* shader)
{
    ID3D10Device* D3D = DirectXGlobals::getDevice();

    for (unsigned int i = 0; i < objectCount; i++)
    {
        Object* Object = objects[i];

        Matrix4x4 ObjectToCell = Object->getObjectToCellMatrix();
        DirectXGlobals::setWorldMatrix((Matrix4x4&)ObjectToCell);

        Matrix4x4 matWVP;
        getWVP((Matrix4x4&)matWVP);

        D3D->UpdateSubresource(g_constantBuffer,
                               0,
                               NULL,
                               &matWVP,16*4,
                               0);

        D3D->UpdateSubresource(g_colorConstantBuffer,
                               0,
                               NULL,
                               &Object->getColor(),
							   sizeof(Vector4),
                               0);

        shader->preRender(Object);

		Object->getMesh()->draw();

        shader->postRender(Object);
    }
}

/*-------------------------------------------------------------------*//*!
 *
 * \brief	Renders 2D lines.
 *
 *//*-------------------------------------------------------------------*/

void Platform::drawLinesScreenSpace(const Vector3* lines, int nLines, const Vector4& color)
{
	// TODO: implement
    (void)lines;
    (void)nLines;
    (void)color;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders 3D lines.
 *//*-------------------------------------------------------------------*/

void Platform::drawLines(const Vector3* lines, int nLines, const Vector4& color)
{
	ID3D10Device* D3D = DirectXGlobals::getDevice();
    Matrix4x4 ident;
    DirectXGlobals::setWorldMatrix(ident);

    Matrix4x4 matWVP;
    getWVP((Matrix4x4&)matWVP);

    D3D->UpdateSubresource(
            g_constantBuffer,
            0,
            NULL,
            &matWVP,
            16*4,
            0
        );
 
    D3D->UpdateSubresource(g_colorConstantBuffer,
                           0,
                           NULL,
                           &color,
						   sizeof(Vector4),
                           0);

	D3D10drawUP(lines, nLines*2, 0, 0, D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets up indexed rendering. 
 *
 *			Configures the shaders before calling drawIndexed or 
 *			drawIndexedUP. 
 *//*-------------------------------------------------------------------*/

void Platform::setupQueryDraw()
{
    ID3D10Device* D3D = DirectXGlobals::getDevice();

    Matrix4x4 ident;
    DirectXGlobals::setWorldMatrix(ident);

    Matrix4x4 matWVP;
    getWVP((Matrix4x4&)matWVP);

    D3D->UpdateSubresource(
            g_constantBuffer,
            0,
            NULL,
            &matWVP,
            16*4,
            0
        );

    UINT stride = sizeof(Vector3);
    UINT VBOffset = 0;

    D3D->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, (UINT*)&VBOffset);
    D3D->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    D3D->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
    (void)start;
    (void)vertexCount;
    ID3D10Device* D3D = DirectXGlobals::getDevice();
    D3D->DrawIndexed(triangleCount*3, 0, vertexOffset);
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
	D3D10drawUP(vertices, vertexCount, (const unsigned int*)triangles, triangleCount*3, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
 * \brief	Clears the depth and stencil buffers.
 *//*-------------------------------------------------------------------*/

void Platform::clearDepthStencil(void)
{
    DirectXGlobals::getDevice()->ClearDepthStencilView(
            g_depthStencilView,
            D3D10_CLEAR_DEPTH ,
            1.f,
            0
        );
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
    return new D3D10Mesh(vertices, normals, triangles, nVertices, nTriangles);
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

	g_font->DrawTextA(NULL, str, -1, &rect, DT_LEFT|DT_TOP|DT_NOCLIP|DT_CALCRECT, D3DXCOLOR());

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Renders text. Currently not implemented (TODO).
 *//*-------------------------------------------------------------------*/

void Platform::fontPrint(
    int		        x,
    int	            y,
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

	DirectXGlobals::getDevice()->RSSetState(g_rasterizerSolid);
	
	D3DXCOLOR d3dxColor;
		
	d3dxColor.r = color.x;
	d3dxColor.g = color.y;
	d3dxColor.b = color.z;
	d3dxColor.a = color.w;

	g_font->DrawTextA(NULL, str, -1, &rect, DT_LEFT|DT_TOP|DT_NOCLIP, d3dxColor);

	// Disable geometry shader set by DrawText
	DirectXGlobals::getDevice()->GSSetShader(NULL);
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active index buffer.
 *
 * \param	indexBuffer		index buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setIndexBuffer(IndexBuffer* indexBuffer)
{
    g_indexBuffer = indexBuffer->platformBuffer;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the active vertex buffer.
 *
 * \param	vertexBuffer	vertex buffer to activate
 *
 *//*-------------------------------------------------------------------*/

void Platform::setVertexBuffer(VertexBuffer* vertexBuffer)
{
    g_vertexBuffer = vertexBuffer->platformBuffer;
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
    (void)stencil;
    float ClearColor[4] = { r, g, b, a }; // RGBA
    DirectXGlobals::getDevice()->ClearRenderTargetView( DirectXGlobals::getRenderTargetView(), ClearColor );

    DirectXGlobals::getDevice()->ClearDepthStencilView(
            g_depthStencilView,
            D3D10_CLEAR_DEPTH ,
            depth,
            0
        );
}

/*-------------------------------------------------------------------*//*!
 * \brief	Sets the current view port.
 *//*-------------------------------------------------------------------*/

void Platform::setViewport(int x, int y, int w, int h)
{
    D3D10_VIEWPORT vp;
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    vp.Width = w;
    vp.Height = h;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    DirectXGlobals::getDevice()->RSSetViewports(1, &vp);
}


/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame starts.
 *//*-------------------------------------------------------------------*/

bool Platform::beginFrame(void)
{

	void* data = 0;

	// discard the buffers used by D3D10drawUP implementation at 
	// the start of each frame by mapping them with D3D10_MAP_WRITE_DISCARD
	g_dynamicVB->platformBuffer->Map(D3D10_MAP_WRITE_DISCARD,
					 0, &data);
	g_dynamicVB->platformBuffer->Unmap();

	g_dynamicIB->platformBuffer->Map(D3D10_MAP_WRITE_DISCARD,
					 0, &data);
	g_dynamicIB->platformBuffer->Unmap();

	g_dynamicVBPos = 0;
	g_dynamicIBPos = 0;

	return true;
}

/*-------------------------------------------------------------------*//*!
 * \brief	Called when frame ends.
 *//*-------------------------------------------------------------------*/

void Platform::endFrame(void)
{
    DirectXGlobals::getSwapChain()->Present( 0, 0 );
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
 * \brief	Flush rendering.
 *//*-------------------------------------------------------------------*/
 
void Platform::flush(void)
{
    DirectXGlobals::getDevice()->Flush();
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/
 
void Platform::finish(void)
{
    // \todo
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
 * \brief	D3D10 rendering from a user pointer.
 *
 *			Implemented with big vertex and index buffers, which 
 *			are accessed using D3D10_MAP_WRITE_NO_OVERWRITE.
 *
 *	\param  vertices	Vertex data
 *	\param  vertexCount	Number of vertices
 *	\param  indices		Indicies, or NULL if no indices used
 *	\param  indexCount	Number of indices
 *	\param	topology	D3D10 constant, tells how vertices are rendered
 *						(e.g. D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
 *
 *//*-------------------------------------------------------------------*/

void D3D10drawUP(const Vector3* vertices, int vertexCount, const unsigned int* indices, int indexCount, D3D10_PRIMITIVE_TOPOLOGY topology)
{
	Vector3* vertexData = 0;
	unsigned int* indexData = 0;
	ID3D10Device* D3D = DirectXGlobals::getDevice();
	
	bool useIndices = indices != 0;

	// ensure that the buffers are big enough for the data
	if(vertexCount > g_dynamicVBSize || indexCount > g_dynamicIBSize)
	{
		PLATFORM_ASSERT("drawIndexedUP: vertex or index buffer not big enough to hold user data" && false);
		return;
	}

	// by default, map buffers in no overwrite mode, so we can
	// enter new data while old is being rendered
	D3D10_MAP MapModeVB = D3D10_MAP_WRITE_NO_OVERWRITE;
	D3D10_MAP MapModeIB = D3D10_MAP_WRITE_NO_OVERWRITE;

	// If the vertex data will not fit in the remaining vertex buffer ...
	if( g_dynamicVBSize - g_dynamicVBPos - 1 < (unsigned int)vertexCount )
	{
		// Discard the entire buffer, if not enough space.
		// Using this mode is safe, since DX10 will wait until GPU finishes using the buffer.
		MapModeVB = D3D10_MAP_WRITE_DISCARD;
		// start filling the buffer from beginning
		g_dynamicVBPos = 0;
	}

	// Similarly, check if index data fits in the remaining space
	if( useIndices && g_dynamicIBSize - g_dynamicIBPos - 1 < (unsigned int)indexCount )
	{
		// Discard the buffer, if not enough space.
		MapModeIB = D3D10_MAP_WRITE_DISCARD;
		// fill the buffer from beginning
		g_dynamicIBPos = 0;
	}

	// Map vertex buffer
    HRESULT rv = g_dynamicVB->platformBuffer->Map(MapModeVB,
							  0,
							  (void**)&vertexData);

	if(rv != S_OK)
	{
		PLATFORM_ASSERT("drawIndexedUP: couldn't map vertex buffer" && false);
		return;
	}

	memcpy(vertexData + g_dynamicVBPos, vertices, vertexCount * sizeof(Vector3));

	g_dynamicVB->platformBuffer->Unmap();

	if(useIndices)
	{
		// Map index buffer
		rv = g_dynamicIB->platformBuffer->Map(MapModeIB,
							  0,
							  (void**)&indexData);

		if(rv != S_OK)
		{
			PLATFORM_ASSERT("drawIndexedUP: couldn't map index buffer" && false);
			return;
		}

		memcpy(indexData + g_dynamicIBPos, indices, indexCount * sizeof(unsigned int));

		g_dynamicIB->platformBuffer->Unmap();
	}

	unsigned int stride = sizeof(Vector3);
	unsigned int VBOffset = 0;
	
	D3D->IASetVertexBuffers(0, 1, &g_dynamicVB->platformBuffer, &stride, &VBOffset);
    if(useIndices)
		D3D->IASetIndexBuffer(g_dynamicIB->platformBuffer, DXGI_FORMAT_R32_UINT, 0);
    D3D->IASetPrimitiveTopology(topology);

	if(useIndices)
		D3D->DrawIndexed( indexCount, g_dynamicIBPos, g_dynamicVBPos );
	else
		D3D->Draw( vertexCount, g_dynamicVBPos );

	g_dynamicVBPos += vertexCount;

	if(useIndices)
		g_dynamicIBPos += indexCount;
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

