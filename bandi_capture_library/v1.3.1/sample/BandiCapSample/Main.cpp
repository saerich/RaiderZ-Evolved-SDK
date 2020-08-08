#include "StdAfx.h"
#include "Main.h"

struct Vertex
{
	float x, y, z; // Position of vertex in 3D space
    DWORD color;   // Color of vertex
};

Vertex g_lineList[] = 
{
    {-10.0f,  0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, 
    {10.0f,  0.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, 

    {0.0f,  -10.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, 
    {0.0f,  10.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, 

    {0.0f,  0.0f, -10.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, 
    {0.0f,  0.0f, 10.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, 
};
#define D3DFVF_MY_VERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )



#define SHADOWMAP_SIZE 512

#define HELPTEXTCOLOR D3DXCOLOR( 0.0f, 1.0f, 0.3f, 1.0f )

LPCWSTR g_aszMeshFile[] =
{
    L"data\\room.x",
    L"data\\airplane.x",
    L"data\\car.x",
    L"data\\sphere.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\arrow.x",
    L"data\\ring.x",
    L"data\\ring.x",
};

//#define NUM_OBJ = (sizeof(g_aszMeshFile)/sizeof(g_aszMeshFile[0]))

D3DXMATRIXA16 g_amInitObjWorld[NUM_OBJ] =
{
    D3DXMATRIXA16( 3.5f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f ),
    D3DXMATRIXA16( 0.43301f, 0.25f, 0.0f, 0.0f, -0.25f, 0.43301f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 5.0f, 1.33975f, 0.0f, 1.0f ),
    D3DXMATRIXA16( 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, -14.5f, -7.1f, 0.0f, 1.0f ),
    D3DXMATRIXA16( 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, -7.0f, 0.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, 5.0f, 0.2f, 5.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, 5.0f, 0.2f, -5.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, -5.0f, 0.2f, 5.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, -5.0f, 0.2f, -5.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, 14.0f, 0.2f, 14.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, 14.0f, 0.2f, -14.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, -14.0f, 0.2f, 14.0f, 1.0f ),
    D3DXMATRIXA16( 5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.5f, 0.0f, 0.0f, -9.0f, 0.0f, 0.0f, -14.0f, 0.2f, -14.0f, 1.0f ),
    D3DXMATRIXA16( 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, -14.5f, -9.0f, 0.0f, 1.0f ),
    D3DXMATRIXA16( 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.9f, 0.0f, 14.5f, -9.0f, 0.0f, 1.0f ),
};


D3DVERTEXELEMENT9 g_aVertDecl[] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};







CMain::CMain(void)
{
	m_pTxtHelper = NULL;
	m_pSprite9 = NULL;

	g_pTeapotMesh = NULL;
	ZeroMemory( &m_teapotMtrl, sizeof(D3DMATERIAL9) );
	m_teapotMtrl.Diffuse.r = 0.0f;
	m_teapotMtrl.Diffuse.g = 0.0f;
	m_teapotMtrl.Diffuse.b = 1.0f;
	m_teapotMtrl.Diffuse.a = 1.0f;

	g_fSpinX = 0;
	g_fSpinY = 0;
	g_pLineList_VB = 0;

	m_hWnd = NULL;
	m_bNowBeep = FALSE;
	m_hInstance = NULL;
	m_pBaCon1 = NULL;


	g_pFont = NULL;       
	g_pFontSmall = NULL;  
	g_pTextSprite = NULL; 
	g_pEffect = NULL;     
	g_bShowHelp = true;   
	g_pVertDecl = NULL;
	g_pTexDef = NULL;     
	g_pShadowMap = NULL;  
	g_pDSShadow = NULL;   
	g_bRightMouseDown = false;
	g_bCameraPerspective = true;
	g_bFreeLight = true;


    // Initialize the camera
    g_VCamera.SetScalers( 0.01f, 15.0f );
    g_LCamera.SetScalers( 0.01f, 8.0f );
    g_VCamera.SetRotateButtons( true, false, false );
    g_LCamera.SetRotateButtons( false, false, true );

    // Set up the view parameters for the camera
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 5.0f, -18.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
    g_VCamera.SetViewParams( &vFromPt, &vLookatPt );

    vFromPt = D3DXVECTOR3( 0.0f, 0.0f, -12.0f );
    vLookatPt = D3DXVECTOR3( 0.0f, -2.0f, 1.0f );
    g_LCamera.SetViewParams( &vFromPt, &vLookatPt );

    // Initialize the spot light
    g_fLightFov = D3DX_PI / 2.0f;

    g_Light.Diffuse.r = 1.0f;
    g_Light.Diffuse.g = 1.0f;
    g_Light.Diffuse.b = 1.0f;
    g_Light.Diffuse.a = 1.0f;
    g_Light.Position = D3DXVECTOR3( -8.0f, -8.0f, 0.0f );
    g_Light.Direction = D3DXVECTOR3( 1.0f, -1.0f, 0.0f );
    D3DXVec3Normalize( (D3DXVECTOR3*)&g_Light.Direction, (D3DXVECTOR3*)&g_Light.Direction );
    g_Light.Range = 10.0f;
    g_Light.Theta = g_fLightFov / 2.0f;
    g_Light.Phi = g_fLightFov / 2.0f;

}

CMain::~CMain(void)
{
}

void CMain::ExitApp()
{
	SAFE_RELEASE(m_pBaCon1);
	m_baFactory.Destory();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void CMain::InitApp(HINSTANCE hInstance)
{
	g_HUD.SetCallback( _OnGUIEvent ); int iY = 10;

    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22 );
    g_HUD.AddCheckBox( IDC_CHECKBOX, L"Display help text", 35, iY += 24, 125, 22, true, VK_F1 );
    g_HUD.AddCheckBox( IDC_LIGHTPERSPECTIVE, L"View from light's perspective", 0, iY += 24, 160, 22, false, L'V' );
    g_HUD.AddCheckBox( IDC_ATTACHLIGHTTOCAR, L"Attach light to car", 0, iY += 24, 160, 22, false, L'F' );

	g_HUD.AddButton( IDC_TEST, L"TEST(F12)", 35, iY += 24, 125, 22, VK_F12 );

	m_hInstance = hInstance;
}


void CMain::RenderText()
{
	/*
	m_pTxtHelper->Begin();
	m_pTxtHelper->SetInsertionPos( 5, 5 );
	m_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	m_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
	m_pTxtHelper->DrawTextLine( DXUTGetFrameStats() );

	m_pTxtHelper->DrawTextLine(_T("--------------------"));

	POSITION p = m_messages.GetHeadPosition();
	while(p)
	{
		m_pTxtHelper->DrawTextLine(m_messages.GetNext(p));
	}

	m_pTxtHelper->End();
	*/

    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats() );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*10 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls:" );

        txtHelper.SetInsertionPos( 15, pd3dsdBackBuffer->Height-15*9 );
        txtHelper.DrawFormattedTextLine(
            L"Rotate camera\nMove camera\n"
            L"Rotate light\nMove light\n"
            L"Change light mode (Current: %s)\nChange view reference (Current: %s)\n"
            L"Hidehelp\nQuit",
            g_bFreeLight ? L"Free" : L"Car-attached",
            g_bCameraPerspective ? L"Camera" : L"Light" );
        txtHelper.SetInsertionPos( 265, pd3dsdBackBuffer->Height-15*9 );
        txtHelper.DrawTextLine(
            L"Left drag mouse\nW,S,A,D,Q,E\n"
            L"Right drag mouse\nW,S,A,D,Q,E while holding right mouse\n"
            L"F\nV\nF1\nESC" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
    txtHelper.End();

}

extern CMain	g_main;
void CALLBACK CMain::_OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
{
	g_main.OnGUIEvent(nEvent, nControlID, pControl);
}

void CMain::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl)
{
	/*
	switch( nControlID )
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen(); break;
	case IDC_TOGGLEREF:
		DXUTToggleREF(); break;
	case IDC_CHANGEDEVICE:
		DXUTSetShowSettingsDialog( !DXUTGetShowSettingsDialog() );
		break;
	case IDC_TEST :
		DoTest();
		break;
	default :
		ASSERT(0);
	}
	*/

    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     DXUTSetShowSettingsDialog( !DXUTGetShowSettingsDialog() ); break;
        case IDC_CHECKBOX:
        {
            CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
            g_bShowHelp = pCheck->GetChecked();
            break;
        }
        case IDC_LIGHTPERSPECTIVE:
        {
            CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
            g_bCameraPerspective = !pCheck->GetChecked();
            if( g_bCameraPerspective )
            {
                g_VCamera.SetRotateButtons( true, false, false );
                g_LCamera.SetRotateButtons( false, false, true );
            } else
            {
                g_VCamera.SetRotateButtons( false, false, false );
                g_LCamera.SetRotateButtons( true, false, true );
            }

            break;
        }
        case IDC_ATTACHLIGHTTOCAR:
        {
            CDXUTCheckBox *pCheck = (CDXUTCheckBox *)pControl;
            g_bFreeLight = !pCheck->GetChecked();
            break;
        }
    }

}

HRESULT CMain::OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	/*
	HRESULT hr;


	pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

	// create light
	g_pLight.Type = D3DLIGHT_DIRECTIONAL;
	g_pLight.Direction = D3DXVECTOR3( 1.0f, 0.0f, 1.0f );

    g_pLight.Diffuse.r = 1.0f;
    g_pLight.Diffuse.g = 1.0f;
    g_pLight.Diffuse.b = 1.0f;
	g_pLight.Diffuse.a = 1.0f;

	g_pLight.Specular.r = 1.0f;
	g_pLight.Specular.g = 1.0f;
	g_pLight.Specular.b = 1.0f;
	g_pLight.Specular.a = 1.0f;


	SetTimer(m_hWnd, TIMER_BEEP, 3000, NULL);


	*/

	m_hWnd = DXUTGetHWND();
	SetTimer(m_hWnd, TIMER_PROCESSMESSAGE, 1000, NULL);

	// audio initialize
	if(m_pBaCon1==NULL)
	{
		if(m_baFactory.Create(BANDIAUDIO_RELEASE_DLL_FILE_NAME)==FALSE) ASSERT(0);
		m_pBaCon1 = m_baFactory.CreateBaCon();
		if(m_pBaCon1)
		{
			m_pBaCon1->Create(m_hInstance, m_hWnd);
			m_pBaCon1->Play(_T("sound\\sample01.ogg"));
			m_pBaCon1->SetVolume256(40);
			m_pBaCon1->SetOption(BAOPT_LOOP);
		}
		else ASSERT(0);
	}


    HRESULT hr;

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );
    V_RETURN( D3DXCreateFont( pd3dDevice, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFontSmall ) );

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = 0;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"data\\ShadowMap.fx" ) );

    // If this fails, there should be debug output as to 
    // they the .fx file failed to compile
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );

    // Create vertex declaration
    V_RETURN( pd3dDevice->CreateVertexDeclaration( g_aVertDecl, &g_pVertDecl ) );

    // Initialize the meshes
    for( int i = 0; i < NUM_OBJ; ++i )
    {
        V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, g_aszMeshFile[i] ) );
        if( FAILED( g_Obj[i].m_Mesh.Create( pd3dDevice, str ) ) )
            return DXUTERR_MEDIANOTFOUND;
        V_RETURN( g_Obj[i].m_Mesh.SetVertexDecl( pd3dDevice, g_aVertDecl ) );
        g_Obj[i].m_mWorld = g_amInitObjWorld[i];
    }

    // Initialize the light mesh
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"data\\spotlight.x" ) );
    if( FAILED( g_LightMesh.Create( pd3dDevice, str ) ) )
        return DXUTERR_MEDIANOTFOUND;
    V_RETURN( g_LightMesh.SetVertexDecl( pd3dDevice, g_aVertDecl ) );

    // World transform to identity
    D3DXMATRIXA16 mIdent;
    D3DXMatrixIdentity( &mIdent );
    V_RETURN( pd3dDevice->SetTransform( D3DTS_WORLD, &mIdent ) );

    return S_OK;
}

HRESULT CMain::OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	/*
	HRESULT hr;

//	V_RETURN( m_DialogResourceManager.OnD3D9ResetDevice() );
//	V_RETURN( m_SettingsDlg.OnD3D9ResetDevice() );

	V_RETURN( D3DXCreateSprite( pd3dDevice, &m_pSprite9 ) );

	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	g_HUD.SetSize( 170, 170 );

	// remember size
	m_nWidth = pBackBufferSurfaceDesc->Width;
	m_nHeight = pBackBufferSurfaceDesc->Height;

	// Load up the teapot mesh...
	//V (D3DXLoadMeshFromX( _T("airplane.x"), D3DXMESH_SYSTEMMEM, pd3dDevice, NULL, NULL, NULL, NULL, &g_pTeapotMesh ));
	hr = m_mesh1.Create(pd3dDevice, _T("airplane.x"));
	hr = m_mesh2.Create(pd3dDevice, _T("teapot.x"));

	// set light enable
    V (pd3dDevice->SetLight( 0, &g_pLight ));
    V (pd3dDevice->LightEnable( 0, TRUE ));
    V (pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) ));

	// line
	Vertex *pVertices = NULL;
	pd3dDevice->CreateVertexBuffer( 6*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pLineList_VB,
									  NULL );

	pVertices = NULL;
	g_pLineList_VB->Lock( 0, sizeof(g_lineList), (void**)&pVertices, 0 );
    memcpy( pVertices, g_lineList, sizeof(g_lineList) );
    g_pLineList_VB->Unlock();


	return S_OK;
	*/

    HRESULT hr;

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pFontSmall )
        V_RETURN( g_pFontSmall->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_VCamera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 100.0f );
    g_LCamera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 100.0f );

    // Create the default texture (used when a triangle does not use a texture)
    V_RETURN( pd3dDevice->CreateTexture( 1, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pTexDef, NULL ) );
    D3DLOCKED_RECT lr;
    V_RETURN( g_pTexDef->LockRect( 0, &lr, NULL, 0 ) );
    *(LPDWORD)lr.pBits = D3DCOLOR_RGBA( 255, 255, 255, 255 );
    V_RETURN( g_pTexDef->UnlockRect( 0 ) );

    // Restore the scene objects
    for( int i = 0; i < NUM_OBJ; ++i )
        V_RETURN( g_Obj[i].m_Mesh.RestoreDeviceObjects( pd3dDevice ) );
    V_RETURN( g_LightMesh.RestoreDeviceObjects( pd3dDevice ) );

    // Restore the effect variables
    V_RETURN( g_pEffect->SetVector( "g_vLightDiffuse", (D3DXVECTOR4 *)&g_Light.Diffuse ) );
    V_RETURN( g_pEffect->SetFloat( "g_fCosTheta", cosf( g_Light.Theta ) ) );

    // Create the shadow map texture
    V_RETURN( pd3dDevice->CreateTexture( SHADOWMAP_SIZE, SHADOWMAP_SIZE,
                                         1, D3DUSAGE_RENDERTARGET,
                                         D3DFMT_R32F,
                                         D3DPOOL_DEFAULT,
                                         &g_pShadowMap,
                                         NULL ) );

    // Create the depth-stencil buffer to be used with the shadow map
    // We do this to ensure that the depth-stencil buffer is large
    // enough and has correct multisample type/quality when rendering
    // the shadow map.  The default depth-stencil buffer created during
    // device creation will not be large enough if the user resizes the
    // window to a very small size.  Furthermore, if the device is created
    // with multisampling, the default depth-stencil buffer will not
    // work with the shadow map texture because texture render targets
    // do not support multisample.
    DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
    V_RETURN( pd3dDevice->CreateDepthStencilSurface( SHADOWMAP_SIZE,
                                                     SHADOWMAP_SIZE,
                                                     d3dSettings.pp.AutoDepthStencilFormat,
                                                     D3DMULTISAMPLE_NONE,
                                                     0,
                                                     TRUE,
                                                     &g_pDSShadow,
                                                     NULL ) );

    // Initialize the shadow projection matrix
    D3DXMatrixPerspectiveFovLH( &g_mShadowProj, g_fLightFov, 1, 0.01f, 100.0f);

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, pBackBufferSurfaceDesc->Height );
    CDXUTControl *pControl = g_HUD.GetControl( IDC_LIGHTPERSPECTIVE );
    if( pControl )
        pControl->SetLocation( 0, pBackBufferSurfaceDesc->Height - 50 );
    pControl = g_HUD.GetControl( IDC_ATTACHLIGHTTOCAR );
    if( pControl )
        pControl->SetLocation( 0, pBackBufferSurfaceDesc->Height - 25 );
    return S_OK;

}

void CMain::OnFrameMove( double fTime, float fElapsedTime)
{
//	g_fSpinY += (float)fElapsedTime*300;

    // Update the camera's position based on user input 
    g_VCamera.FrameMove( fElapsedTime );
    g_LCamera.FrameMove( fElapsedTime );

    // Animate the plane, car and sphere meshes
    D3DXMATRIXA16 m;

    D3DXMatrixRotationY( &m, D3DX_PI * fElapsedTime / 4.0f );
    D3DXMatrixMultiply( &g_Obj[1].m_mWorld, &g_Obj[1].m_mWorld, &m );
    D3DXMatrixRotationY( &m, -D3DX_PI * fElapsedTime / 4.0f );
    D3DXMatrixMultiply( &g_Obj[2].m_mWorld, &g_Obj[2].m_mWorld, &m );
    D3DXVECTOR3 vR( 0.1f, 1.0f, -0.2f );
    D3DXMatrixRotationAxis( &m, &vR, -D3DX_PI * fElapsedTime / 6.0f );
    D3DXMatrixMultiply( &g_Obj[3].m_mWorld, &m, &g_Obj[3].m_mWorld );

}

LRESULT CMain::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing)
{
	/*
	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// process timer
	if(uMsg==WM_TIMER)
	{
		OnTimer(wParam);
	}

	return 0;
	*/

    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all windows messages to camera and dialogs so they can respond to user input
    if( WM_KEYDOWN != uMsg || g_bRightMouseDown )
        g_LCamera.HandleMessages( hWnd, uMsg, wParam, lParam );

    if( WM_KEYDOWN != uMsg || !g_bRightMouseDown )
    {
        if( g_bCameraPerspective )
            g_VCamera.HandleMessages( hWnd, uMsg, wParam, lParam );
        else
            g_LCamera.HandleMessages( hWnd, uMsg, wParam, lParam );
    }

    return 0;

}

#define SPIN_STEP 5
void CMain::OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown)
{
	switch(nChar)
	{
	case VK_LEFT :	g_fSpinX += SPIN_STEP; break;
	case VK_RIGHT : g_fSpinX -= SPIN_STEP; break;
	case VK_UP :	g_fSpinY += SPIN_STEP; break;
	case VK_DOWN :	g_fSpinY -= SPIN_STEP; break;
	}

}

void CMain::OnD3D9LostDevice()
{
	/*
	SAFE_RELEASE( m_pSprite9 );
	SAFE_DELETE( m_pTxtHelper );
	SAFE_RELEASE(g_pTeapotMesh);
	SAFE_RELEASE(g_pLineList_VB);
	m_mesh1.Destroy();
	m_mesh2.Destroy();
	*/

    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pFontSmall )
        g_pFontSmall->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE(g_pTextSprite);

    SAFE_RELEASE( g_pDSShadow );
    SAFE_RELEASE( g_pShadowMap );
    SAFE_RELEASE( g_pTexDef );

    for( int i = 0; i < NUM_OBJ; ++i )
        g_Obj[i].m_Mesh.InvalidateDeviceObjects();
    g_LightMesh.InvalidateDeviceObjects();

}

void CMain::OnD3D9DestroyDevice()
{

    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pFontSmall );
    SAFE_RELEASE( g_pVertDecl );

    SAFE_RELEASE( g_pEffect );

    for( int i = 0; i < NUM_OBJ; ++i )
        g_Obj[i].m_Mesh.Destroy();
    g_LightMesh.Destroy();

}

void CMain::DoTest()
{
	Message(_T("Çï·Î: %d"), GetTickCount());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         message to scrren
/// @date   Thursday, August 07, 2008  3:26:06 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMain::Message(LPCTSTR szMsg, ...)
{
	CAtlString sMsg;
	va_list args;
	va_start(args, szMsg);
	sMsg.FormatV(szMsg, args);
	va_end(args);

	m_messages.AddHead(sMsg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         process timer
/// @date   Thursday, August 07, 2008  3:26:00 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void CMain::OnTimer(int nTimerID)
{
	if(nTimerID==TIMER_PROCESSMESSAGE)
	{
		if(m_messages.GetCount())
			m_messages.RemoveTail();
	}
	if(nTimerID==TIMER_BEEP)
	{
		m_bNowBeep = TRUE;
	}
}

void CMain::OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime)
{
	/*
	HRESULT hr;

	// Clear the render target and the zbuffer 
	V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

	// Render the scene
	if(!SUCCEEDED( pd3dDevice->BeginScene())){ASSERT(0);return;}

	// render text
	RenderText();

	// render buttons
	V( g_HUD.OnRender( fElapsedTime ) );

	//  projection
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ),  m_nWidth / (float)m_nHeight, 0.1f, 100.0f );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	D3DXMATRIX matWorld;
	D3DXMATRIX matRotation;
	D3DXMATRIX matTranslation;

	D3DXMATRIXA16 matTransform;
	float fScaleX = 1;
	float fScaleY = 1;
	D3DXMatrixScaling( &matTransform, fScaleX, fScaleY, 1.0f );

	D3DXMatrixRotationYawPitchRoll( &matRotation, D3DXToRadian(g_fSpinX), D3DXToRadian(g_fSpinY), 0.0f );
	D3DXMatrixTranslation( &matTranslation, 0.0f, 0.0f, 15.0f );
	matWorld = matRotation * matTranslation;
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );



	// xyz line
	pd3dDevice->SetStreamSource( 0, g_pLineList_VB, 0, sizeof(Vertex) );
	pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
	pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 3);



	// teapot
	hr = pd3dDevice->SetMaterial( &m_teapotMtrl );
	hr = m_mesh1.Render(pd3dDevice);
	hr = m_mesh2.Render(pd3dDevice);


	V( pd3dDevice->EndScene() );
	*/

    HRESULT hr;

    //
    // Compute the view matrix for the light
    // This changes depending on the light mode
    // (free movement or attached)
    //
    D3DXMATRIXA16 mLightView;
    if( g_bFreeLight )
        mLightView = *g_LCamera.GetViewMatrix();
    else
    {
        // Light attached to car.
        mLightView = g_Obj[2].m_mWorld;
        D3DXVECTOR3 vPos( mLightView._41, mLightView._42, mLightView._43 );  // Offset z by -2 so that it's closer to headlight
        D3DXVECTOR4 vDir = D3DXVECTOR4( 0.0f, 0.0f, -1.0f, 1.0f );  // In object space, car is facing -Z
        mLightView._41 = mLightView._42 = mLightView._43 = 0.0f;  // Remove the translation
        D3DXVec4Transform( &vDir, &vDir, &mLightView );  // Obtain direction in world space
        vDir.w = 0.0f;  // Set w 0 so that the translation part below doesn't come to play
        D3DXVec4Normalize( &vDir, &vDir );
        vPos.x += vDir.x * 4.0f;  // Offset the center by 4 so that it's closer to the headlight
        vPos.y += vDir.y * 4.0f;
        vPos.z += vDir.z * 4.0f;
        vDir.x += vPos.x;  // vDir denotes the look-at point
        vDir.y += vPos.y;
        vDir.z += vPos.z;
        D3DXVECTOR3 vUp( 0.0f, 1.0f, 0.0f );
        D3DXMatrixLookAtLH( &mLightView, &vPos, (D3DXVECTOR3*)&vDir, &vUp );
    }

    //
    // Render the shadow map
    //
    LPDIRECT3DSURFACE9 pOldRT = NULL;
    V( pd3dDevice->GetRenderTarget( 0, &pOldRT ) );
    LPDIRECT3DSURFACE9 pShadowSurf;
    if( SUCCEEDED( g_pShadowMap->GetSurfaceLevel( 0, &pShadowSurf ) ) )
    {
        pd3dDevice->SetRenderTarget( 0, pShadowSurf );
        SAFE_RELEASE( pShadowSurf );
    }
    LPDIRECT3DSURFACE9 pOldDS = NULL;
    if( SUCCEEDED( pd3dDevice->GetDepthStencilSurface( &pOldDS ) ) )
        pd3dDevice->SetDepthStencilSurface( g_pDSShadow );

    {
        CDXUTPerfEventGenerator g( DXUT_PERFEVENTCOLOR, L"Shadow Map" );
        RenderScene( pd3dDevice, true, fElapsedTime, &mLightView, &g_mShadowProj );
    }

    if( pOldDS )
    {
        pd3dDevice->SetDepthStencilSurface( pOldDS );
        pOldDS->Release();
    }
    pd3dDevice->SetRenderTarget( 0, pOldRT );
    SAFE_RELEASE( pOldRT );

    //
    // Now that we have the shadow map, render the scene.
    //
    const D3DXMATRIX *pmView = g_bCameraPerspective ? g_VCamera.GetViewMatrix() :
                                                      &mLightView;

    // Initialize required parameter
    V( g_pEffect->SetTexture( "g_txShadow", g_pShadowMap ) );
    // Compute the matrix to transform from view space to
    // light projection space.  This consists of
    // the inverse of view matrix * view matrix of light * light projection matrix
    D3DXMATRIXA16 mViewToLightProj;
    mViewToLightProj = *pmView;
    D3DXMatrixInverse( &mViewToLightProj, NULL, &mViewToLightProj );
    D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &mLightView );
    D3DXMatrixMultiply( &mViewToLightProj, &mViewToLightProj, &g_mShadowProj );
    V( g_pEffect->SetMatrix( "g_mViewToLightProj", &mViewToLightProj ) );

    {
        CDXUTPerfEventGenerator g( DXUT_PERFEVENTCOLOR, L"Scene" );
        RenderScene( pd3dDevice, false, fElapsedTime, pmView, g_VCamera.GetProjMatrix() );
    }
    g_pEffect->SetTexture( "g_txShadow", NULL );

}

void CMain::RenderScene( IDirect3DDevice9* pd3dDevice, bool bRenderShadow, float fElapsedTime, const D3DXMATRIX *pmView, const D3DXMATRIX *pmProj )
{
    HRESULT hr;

    // Set the projection matrix
    V( g_pEffect->SetMatrix( "g_mProj", pmProj ) );

    // Update the light parameters in the effect
    if( g_bFreeLight )
    {
        // Freely moveable light. Get light parameter
        // from the light camera.
        D3DXVECTOR3 v = *g_LCamera.GetEyePt();
        D3DXVECTOR4 v4;
        D3DXVec3Transform( &v4, &v, pmView );
        V( g_pEffect->SetVector( "g_vLightPos", &v4 ) );
        *(D3DXVECTOR3*)&v4 = *g_LCamera.GetWorldAhead();
        v4.w = 0.0f;  // Set w 0 so that the translation part doesn't come to play
        D3DXVec4Transform( &v4, &v4, pmView );  // Direction in view space
        D3DXVec3Normalize( (D3DXVECTOR3*)&v4, (D3DXVECTOR3*)&v4 );
        V( g_pEffect->SetVector( "g_vLightDir", &v4 ) );
    } else
    {
        // Light attached to car.  Get the car's world position and direction.
        D3DXMATRIXA16 m = g_Obj[2].m_mWorld;
        D3DXVECTOR3 v( m._41, m._42, m._43 );
        D3DXVECTOR4 vPos;
        D3DXVec3Transform( &vPos, &v, pmView );
        D3DXVECTOR4 v4( 0.0f, 0.0f, -1.0f, 1.0f );  // In object space, car is facing -Z
        m._41 = m._42 = m._43 = 0.0f;  // Remove the translation
        D3DXVec4Transform( &v4, &v4, &m );  // Obtain direction in world space
        v4.w = 0.0f;  // Set w 0 so that the translation part doesn't come to play
        D3DXVec4Transform( &v4, &v4, pmView );  // Direction in view space
        D3DXVec3Normalize( (D3DXVECTOR3*)&v4, (D3DXVECTOR3*)&v4 );
        V( g_pEffect->SetVector( "g_vLightDir", &v4 ) );
        vPos += v4 * 4.0f;  // Offset the center by 3 so that it's closer to the headlight.
        V( g_pEffect->SetVector( "g_vLightPos", &vPos ) );
    }

    // Clear the render buffers
    V( pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                          0x000000ff, 1.0f, 0L ) );

    if( bRenderShadow )
        V( g_pEffect->SetTechnique( "RenderShadow" ) );

    // Begin the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        if( !bRenderShadow )
            V( g_pEffect->SetTechnique( "RenderScene" ) );

        // Render the objects
        for( int obj = 0; obj < NUM_OBJ; ++obj )
        {
            D3DXMATRIXA16 mWorldView = g_Obj[obj].m_mWorld;
            D3DXMatrixMultiply( &mWorldView, &mWorldView, pmView );
            V( g_pEffect->SetMatrix( "g_mWorldView", &mWorldView ) );

            LPD3DXMESH pMesh = g_Obj[obj].m_Mesh.GetLocalMesh();
            UINT cPass;
            V( g_pEffect->Begin( &cPass, 0 ) );
            for( UINT p = 0; p < cPass; ++p )
            {
                V( g_pEffect->BeginPass( p ) );

                for( DWORD i = 0; i < g_Obj[obj].m_Mesh.m_dwNumMaterials; ++i )
                {
                    D3DXVECTOR4 vDif( g_Obj[obj].m_Mesh.m_pMaterials[i].Diffuse.r,
                                      g_Obj[obj].m_Mesh.m_pMaterials[i].Diffuse.g,
                                      g_Obj[obj].m_Mesh.m_pMaterials[i].Diffuse.b,
                                      g_Obj[obj].m_Mesh.m_pMaterials[i].Diffuse.a );
                    V( g_pEffect->SetVector( "g_vMaterial", &vDif ) );
                    if( g_Obj[obj].m_Mesh.m_pTextures[i] )
                        V( g_pEffect->SetTexture( "g_txScene", g_Obj[obj].m_Mesh.m_pTextures[i] ) )
                    else
                        V( g_pEffect->SetTexture( "g_txScene", g_pTexDef ) )
                    V( g_pEffect->CommitChanges() );
                    V( pMesh->DrawSubset( i ) );
                }
                V( g_pEffect->EndPass() );
            }
            V( g_pEffect->End() );
        }

        // Render light
        if( !bRenderShadow )
            V( g_pEffect->SetTechnique( "RenderLight" ) );

        D3DXMATRIXA16 mWorldView = *g_LCamera.GetWorldMatrix();
        D3DXMatrixMultiply( &mWorldView, &mWorldView, pmView );
        V( g_pEffect->SetMatrix( "g_mWorldView", &mWorldView ) );

        UINT cPass;
        LPD3DXMESH pMesh = g_LightMesh.GetLocalMesh();
        V( g_pEffect->Begin( &cPass, 0 ) );
        for( UINT p = 0; p < cPass; ++p )
        {
            V( g_pEffect->BeginPass( p ) );

            for( DWORD i = 0; i < g_LightMesh.m_dwNumMaterials; ++i )
            {                                     
                D3DXVECTOR4 vDif( g_LightMesh.m_pMaterials[i].Diffuse.r,
                                  g_LightMesh.m_pMaterials[i].Diffuse.g,
                                  g_LightMesh.m_pMaterials[i].Diffuse.b,
                                  g_LightMesh.m_pMaterials[i].Diffuse.a );
                V( g_pEffect->SetVector( "g_vMaterial", &vDif ) );
                V( g_pEffect->SetTexture( "g_txScene", g_LightMesh.m_pTextures[i] ) );
                V( g_pEffect->CommitChanges() );
                V( pMesh->DrawSubset( i ) );
            }
            V( g_pEffect->EndPass() );
        }
        V( g_pEffect->End() );

        if( !bRenderShadow )
            // Render stats and help text
            RenderText();

        // Render the UI elements
        if( !bRenderShadow )
            g_HUD.OnRender( fElapsedTime );

        V( pd3dDevice->EndScene() );
    }
}
