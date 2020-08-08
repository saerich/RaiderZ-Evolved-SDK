/**********************************************************************
 *<
	FILE: cubemap.cpp

	DESCRIPTION: Cubemap VertexShader

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 04/30/02
	Based on the code by Nikolai Sander from the r4 samples

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "CubeMap.h"
#include "IViewportManager.h"
#include "AssetManagement\IAssetAccessor.h"
#include "IPathConfigMgr.h"
#include "assetmanagement\AssetType.h"
#include "Assetmanagement\iassetmanager.h"
#include "IFileResolutionManager.h"

using namespace MaxSDK::AssetManagement;

#define CUBEMAP_CLASS_ID	Class_ID(0x1be91e49, 0x612a966c)


#define PBLOCK_REF	0
enum { cubemap_params };


//TODO: Add enums for various parameters
enum { 
	pb_cubemapfile,
};

// TCHAR *FindMapFile(TCHAR *file)
// {
// 	BOOL found = 0;
// 
// 	static TCHAR  filepath[MAX_PATH];
// 	TCHAR* fpart;
// 
// 	for (int i = 0; i < GetCOREInterface()->GetMapDirCount(); i++)
// 		if ((found = SearchPath(GetCOREInterface()->GetMapDir(i), file, NULL, MAX_PATH, filepath, &fpart)))
// 				break;
// 	if(found)
// 		return filepath;
// 	else
// 		return NULL;
// }

class CubeMap : public ReferenceTarget, public IDX9DataBridge {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0
		IDX9PixelShaderImp *pps;
		IDX9PixelVertexShaderImp *pvs;
		bool			initVS;
		bool			initPS;
		AssetUser cubeMapFileAsset;
		AssetUser vertexShaderFileAsset;
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return CUBEMAP_CLASS_ID;}		
		SClass_ID SuperClassID() { return REF_TARGET_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		int	NumParamBlocks() { return 1; }			
		IParamBlock2* GetParamBlock(int i) { if(i == 0) return pblock; else return NULL;} 
		IParamBlock2* GetParamBlockByID(short id) { if(id == cubemap_params ) return pblock; else return NULL;} 

		void DeleteThis() { delete this; }	
		int NumRefs(){return 1;}
		void SetReference(int i, RefTargetHandle  targ);
		ReferenceTarget * GetReference(int i);		
		ReferenceTarget * Clone(RemapDir &remap);

		void EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags);
			
		// From IDXDataBridge
		ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp);
		void DisableUI(){};
		void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl){};
		TCHAR * GetName(){return _T("Cubemap");}
		float GetDXVersion(){return 9.0f;}

		BaseInterface* GetInterface(Interface_ID id);

		void SetCubeMapFile(const AssetUser& file);
		const AssetUser& GetCubeMapFile();
		const AssetUser& GetVertexShaderFile();
		void SetVertexShaderFile(const AssetUser& file);
		void UpdateButtonText();

		bool GetInitVS(){ return initVS;}
		bool GetInitPS(){ return initPS;}

		void SetInitVS(bool init){ initVS = init;}
		void SetInitPS(bool init){ initPS = init;}

		CubeMap();
		~CubeMap();		

};


class CubeMapClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new CubeMap(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return CUBEMAP_CLASS_ID; }
	// The category is fixed so that the manager loads it as a Viewport Shader
	const TCHAR* 	Category() { return GetString(IDS_HW_CATEGORY); }
	const TCHAR*	InternalName() { return _T("CubeMap"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static CubeMapClassDesc CubeMapDesc;
ClassDesc2* GetCubeMapDesc() { return &CubeMapDesc; }






class PSCM_Accessor : public PBAccessor
{
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		CubeMap *map = (CubeMap*) owner;
		switch(id)
		{
		case pb_cubemapfile: 
			{
				IAssetManager* assetMgr = IAssetManager::GetInstance();
				if(assetMgr)
				{
					map->SetCubeMapFile(assetMgr->GetAsset(v.s,kBitmapAsset)); break;
				}
			}
		default: break;
		}
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}
};

static PSCM_Accessor pscm_accessor;

static ParamBlockDesc2 cubemap_param_blk ( cubemap_params, _T("params"),  0, &CubeMapDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0,NULL,
	// params
	pb_cubemapfile,		_T("cubeMapfile"),	TYPE_FILENAME,	0,	IDS_CUBEMAPFILENAME,
		p_ui,			TYPE_FILEOPENBUTTON, IDC_CUBEMAP,
		p_file_types,	IDS_CM_FILE_TYPES,
		p_caption,		IDS_CUBEMAPFILENAME,
		p_accessor,		&pscm_accessor,
		p_assetTypeID,	kBitmapAsset,
		end,

	end
	);



CubeMap::CubeMap()
{
	pblock = NULL;
	CubeMapDesc.MakeAutoParamBlocks(this);	
	pps = new IDX9PixelShaderImp(this);
	pvs = new IDX9PixelVertexShaderImp(this);
	initPS = true;
	initVS = true;
	IAssetManager* assetMgr = IAssetManager::GetInstance();
	if(assetMgr)
	{
		AssetUser assetCubeMap = assetMgr->GetAsset(_T("sky_cube_mipmap.dds"),kBitmapAsset);
		AssetUser assetVertexShader = assetMgr->GetAsset(_T("CubeMap.njv"),kBitmapAsset);
		SetVertexShaderFile(assetVertexShader);
		SetCubeMapFile(assetCubeMap);
	}

}

CubeMap::~CubeMap()
{
	if(pps)
		delete pps;
	if(pvs)
		delete pvs;

}

void CubeMap::SetCubeMapFile(const AssetUser& file)
{
	cubeMapFileAsset = file;
	initPS = true; 
	UpdateButtonText();

	static TCHAR cubemapFile[MAX_PATH];
	_tcscpy(cubemapFile,cubeMapFileAsset.GetFileName());

	cubemap_param_blk.ParamOption(pb_cubemapfile, p_init_file, cubemapFile);
}

const AssetUser& CubeMap::GetCubeMapFile()
{ 
	return cubeMapFileAsset;
}

const AssetUser& CubeMap::GetVertexShaderFile()
{ 
	return vertexShaderFileAsset;
}

void CubeMap::SetVertexShaderFile(const AssetUser& file)
{
	vertexShaderFileAsset = file;
}

void CubeMap::UpdateButtonText()
{
	IAutoMParamDlg *dlg = pblock->GetMParamDlg();
	IParamMap2 *map = dlg->GetMap();
	if(map)
	{
		TSTR p,f,e,name;
		
		ParamDef &mapdef = pblock->GetParamDef(pb_cubemapfile);	//Added to force a filename to be present	
		static TCHAR cubemapFile[MAX_PATH];
		_tcscpy(cubemapFile,cubeMapFileAsset.GetFileName());
		mapdef.init_file = cubemapFile;					//Added to force a filename to be present
		SplitFilename(TSTR(cubemapFile), &p, &f, &e);
		name = f+e;
		map->SetText(pb_cubemapfile, name);

	}
}


ParamDlg * CubeMap::CreateEffectDlg(HWND hWnd, IMtlParams * imp)
{
	ParamDlg * dlg = CubeMapDesc.CreateParamDlgs(hWnd, imp, this);
	UpdateButtonText();
	return dlg;
}

#define CUBEMAP_CHUNK 0x1000
#define CUBEMAP_ASSET_CHUNK 0x2000 

IOResult CubeMap::Load(ILoad *iload)
{
	TCHAR *buf;
	IOResult res = IO_OK;	
	
	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
		case CUBEMAP_CHUNK:
			{
				res = iload->ReadWStringChunk (&buf);
				IAssetManager* assetMgr = IAssetManager::GetInstance();
				if(assetMgr)
				{
					SetCubeMapFile(assetMgr->GetAsset(buf,kBitmapAsset));
				}
			};
			break;
		case CUBEMAP_ASSET_CHUNK:
			{
				IAssetManager* assetMgr = IAssetManager::GetInstance();	
				if(assetMgr)
					assetMgr->LoadAsset(iload,cubeMapFileAsset);
			};
			break;

		}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}				
		
	return IO_OK;
}



IOResult CubeMap::Save(ISave *isave)
{
	isave->BeginChunk(CUBEMAP_ASSET_CHUNK);
	cubeMapFileAsset.Save(isave);
	isave->EndChunk();
	
	return IO_OK;
}

void CubeMap::SetReference(int i, RefTargetHandle  targ)
{
	if(i==0)
		pblock = (IParamBlock2 *)targ;

}

RefTargetHandle CubeMap::GetReference(int i)
{
	if(i==0)
		return pblock;
	else
		return NULL;
}


RefResult CubeMap::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

ReferenceTarget *CubeMap::Clone(RemapDir &remap)
{
	CubeMap *pnew = new CubeMap;
	pnew->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, pnew, remap);
	return pnew;
}

BaseInterface *CubeMap::GetInterface(Interface_ID id)
{
	if (id == VIEWPORT_SHADER_CLIENT_INTERFACE) {
		return static_cast<IDXDataBridge*>(this);
	}

	else if (id == DX9_PIXEL_SHADER_INTERFACE_ID) {
		return (IDX9PixelShader *)pps;
	}
	else if (id == DX9_VERTEX_SHADER_INTERFACE_ID) {
		return (IDX9VertexShader *)pvs;
	}
	else if (id== VIEWPORT_SHADER9_CLIENT_INTERFACE)
	{
		return static_cast<IDX9DataBridge*>(this);
	}
	else {
		return BaseInterface::GetInterface(id);
	}
}

class CubeMapAccessor : public IAssetAccessor	{
public:

	CubeMapAccessor(CubeMap* aCubeMap) : mCubeMap(aCubeMap) {}

	virtual MaxSDK::AssetManagement::AssetType GetAssetType() const	{ return MaxSDK::AssetManagement::kBitmapAsset; }

protected:
	CubeMap* mCubeMap;
};

class CubeMapFileAccessor : public CubeMapAccessor	{
public:
	CubeMapFileAccessor(CubeMap* aCubeMap) : CubeMapAccessor(aCubeMap)	{ }

	// path accessor functions
	virtual MaxSDK::AssetManagement::AssetUser GetAsset() const ;
	virtual bool SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAsset) ;
};

MaxSDK::AssetManagement::AssetUser CubeMapFileAccessor::GetAsset() const	{
	return mCubeMap->GetCubeMapFile();
}

bool CubeMapFileAccessor::SetAsset(const MaxSDK::AssetManagement::AssetUser & aNewAsset)	{
	IAssetManager* assetMgr = IAssetManager::GetInstance();
	if(!assetMgr)
		return false;

	mCubeMap->SetCubeMapFile(aNewAsset);
	return true;
}

class CubeMapVertexAccessor : public CubeMapAccessor	{
public:
	CubeMapVertexAccessor(CubeMap* aCubeMap) : CubeMapAccessor(aCubeMap)	{ }

	// path accessor functions
	virtual MaxSDK::AssetManagement::AssetUser GetAsset() const;
	virtual bool SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAsset);
	// path is not writable, the file is locked and is found using the map path
	virtual bool IsAssetPathWritable() const	{ return false;  }
};

MaxSDK::AssetManagement::AssetUser CubeMapVertexAccessor::GetAsset() const	{
	return mCubeMap->GetVertexShaderFile();
}

bool CubeMapVertexAccessor::SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAsset)	{
	// asset path is not writable	
	return true;
}

// LAM - 4/21/03 - enumerate files for archiving
void CubeMap::EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags)
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return;

	if (GetCubeMapFile().GetId()!=kInvalidId)
	{
		if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
			IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
			callback->DeclareAsset(CubeMapFileAccessor(this));
		}
		else	{
			IPathConfigMgr::GetPathConfigMgr()->
				RecordInputAsset(GetCubeMapFile(), nameEnum, flags);
		}
		
	}
	
	if (GetCubeMapFile().GetId()!=kInvalidId)
	{
		if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
			IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
			callback->DeclareAsset(CubeMapVertexAccessor(this));
		}
		else	{
			IPathConfigMgr::GetPathConfigMgr()->
				RecordInputAsset(GetVertexShaderFile(), nameEnum, flags);
		}
	}
	
	ReferenceTarget::EnumAuxFiles(nameEnum, flags);
}

//--- IDX8PixelShaderImp-------------------------------------------------------

IDX9PixelShaderImp::IDX9PixelShaderImp(CubeMap *m)
  : pd3dDevice(NULL),
	dwPixelShader(NULL),
	pCubeTexture(NULL),
	map(m)
{
}

IDX9PixelShaderImp::~IDX9PixelShaderImp()
{
	SAFE_RELEASE(pCubeTexture);
	SAFE_RELEASE(dwPixelShader);
	SAFE_RELEASE(pd3dDevice);
}

HRESULT IDX9PixelShaderImp::ConfirmDevice(ID3D9GraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();

	D3DCAPS9 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
	{
		return E_FAIL;
	}

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT IDX9PixelShaderImp::ConfirmVertexShader(IDX9VertexShader *pvs)
{
	return S_OK;
}

HRESULT IDX9PixelShaderImp::Initialize(Material *mtl, INode *node)
{
	HRESULT hr = S_OK;

	if (map->GetInitPS()) {

		TSTR cubeMapPath;
		
		if(!map->GetCubeMapFile().GetFullFilePath(cubeMapPath))
			hr = E_FAIL;
		else
		{
			SAFE_RELEASE(pCubeTexture);
			hr = D3DXCreateCubeTextureFromFileEx(pd3dDevice,
				cubeMapPath,
				D3DX_DEFAULT,
				1,
				0,
				D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED,
				D3DX_FILTER_LINEAR,
				D3DX_FILTER_LINEAR,
				0,
				NULL,
				NULL,
				&pCubeTexture);
		}
		
		map->SetInitPS(false);
	}

	return hr;
}

int IDX9PixelShaderImp::GetNumMultiPass()
{
	return 1;
}

HRESULT IDX9PixelShaderImp::SetPixelShader(ID3D9GraphicsWindow *d3dgw, int numPass)
{
	HRESULT hr = S_OK;

	pd3dDevice->SetTexture(0, pCubeTexture);

	pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	return hr;
}

//--- IDX8VertexShaderImp -------------------------------------------------------

IDX9PixelVertexShaderImp::IDX9PixelVertexShaderImp(CubeMap *m) : 
	pd3dDevice(NULL),
	pCode(NULL),
	dwVertexShader(NULL),
	pCubeTexture(NULL),
	vdecl(NULL),
	map(m)
{
	stdDualVS = (IStdDualVS*) CreateInstance(REF_MAKER_CLASS_ID, STD_DUAL_VERTEX_SHADER);
	if(stdDualVS)
		stdDualVS->SetCallback((IStdDualVSCallback*)this);
	
}

IDX9PixelVertexShaderImp::~IDX9PixelVertexShaderImp()
{
	SAFE_RELEASE(pCode);
	SAFE_RELEASE(pCubeTexture);
	SAFE_RELEASE(dwVertexShader);
	SAFE_RELEASE(pd3dDevice);
	SAFE_RELEASE(vdecl);
	if(stdDualVS)
		stdDualVS->DeleteThis();
}

HRESULT IDX9PixelVertexShaderImp::Initialize(Mesh *mesh, INode *node)
{
	if(map->GetInitVS())
	{
		HRESULT hr = LoadAndCompileShader();
		if(FAILED(hr))
			return hr;

		map->SetInitVS(false);
	}	

	if(stdDualVS)
		return stdDualVS->Initialize(mesh, node);
	else
		return E_FAIL;
}

HRESULT IDX9PixelVertexShaderImp::Initialize(MNMesh *mnmesh, INode *node)
{
	if(map->GetInitVS())
	{
		HRESULT hr = LoadAndCompileShader();
		if(FAILED(hr))
			return hr;
		
		map->SetInitVS(false);
	}	

	if(stdDualVS)
		return stdDualVS->Initialize(mnmesh, node);
	else
		return E_FAIL;
}

HRESULT IDX9PixelVertexShaderImp::ConfirmDevice(ID3D9GraphicsWindow *d3dgw)
{
	pd3dDevice = d3dgw->GetDevice();
	pd3dDevice->AddRef();

	D3DCAPS9 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
	{
		return E_FAIL;
	}

	if (!(d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT IDX9PixelVertexShaderImp::ConfirmPixelShader(IDX9PixelShader *pps)
{
	return S_OK;
}

ReferenceTarget *IDX9PixelVertexShaderImp::GetRefTarg()
{
	return map;
}

VertexShaderCache *IDX9PixelVertexShaderImp::CreateVertexShaderCache()
{
	return new IDX9VertexShaderCache;
}

HRESULT IDX9PixelVertexShaderImp::InitValid(Mesh *mesh, INode *node)
{
	HRESULT hr = S_OK;

	DebugPrint(_T("DX8 Vertex Shader Init\n"));
		
	//Declaration.Append(4, &Decl[0]);
	
	LPD3DXBUFFER *ppCode = &pCode;
	
	// VertexShader Declarations
	// VertexShader Constants
	Constants.SetCount(20);
	
	Constants[CV_ZERO] = D3DXVECTOR4(0.0f,0.0f,0.0f,0.0f);
	Constants[CV_ONE] = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
	Constants[CV_HALF] = D3DXVECTOR4(0.5f,0.5f,0.5f,0.5f);
	
	
	pd3dDevice->SetVertexShader(dwVertexShader);
	
	pd3dDevice->SetVertexShaderConstantF(CV_ZERO, Constants[CV_ZERO], 1);
	pd3dDevice->SetVertexShaderConstantF(CV_ONE, Constants[CV_ONE], 1);
	pd3dDevice->SetVertexShaderConstantF(CV_HALF, Constants[CV_HALF], 1);
	map->SetInitVS(false);
	
	return hr;
}

HRESULT IDX9PixelVertexShaderImp::InitValid(MNMesh *mnmesh, INode *node)
{
	HRESULT hr = S_OK;

	DebugPrint(_T("DX8 Vertex Shader Init\n"));
	
	// VertexShader Constants
	Constants.SetCount(20);
	
	Constants[CV_ZERO] = D3DXVECTOR4(0.0f,0.0f,0.0f,0.0f);
	Constants[CV_ONE] = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
	Constants[CV_HALF] = D3DXVECTOR4(0.5f,0.5f,0.5f,0.5f);
	
	//		pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, true);
	pd3dDevice->SetVertexShader(dwVertexShader);
	
	pd3dDevice->SetVertexShaderConstantF(CV_ZERO, Constants[CV_ZERO], 1);
	pd3dDevice->SetVertexShaderConstantF(CV_ONE, Constants[CV_ONE], 1);
	pd3dDevice->SetVertexShaderConstantF(CV_HALF, Constants[CV_HALF], 1);
	
	map->SetInitVS(false);
	
	return hr;
}

bool IDX9PixelVertexShaderImp::CanTryStrips()
{
	return true;
}

int IDX9PixelVertexShaderImp::GetNumMultiPass()
{
	return 1;
}

HRESULT IDX9PixelVertexShaderImp::SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass)
{
	pd3dDevice->SetVertexShader(dwVertexShader);

	D3DXMATRIX matWorld = d3dgw->GetWorldXform();
	D3DXMATRIX matView = d3dgw->GetViewXform();
	D3DXMATRIX matProj = d3dgw->GetProjXform();
	D3DXMATRIX matTemp1;
	D3DXMATRIX matTemp2;
	D3DXMATRIX matTemp3;

	D3DXMatrixMultiply(&matTemp1, &matWorld, &matView);
	D3DXMatrixMultiply(&matTemp2, &matTemp1, &matProj);

	// Transform from Model Space to H-Clip space
	D3DXMatrixTranspose(&matTemp3, &matTemp2);
	pd3dDevice->SetVertexShaderConstantF(CV_WORLDVIEWPROJ_0, (float*)&matTemp3(0,0), 4);

	// Conversion from 3ds max coords to Direct3D coords:
	//
	// 3ds max:  (Up, Front, Right) == (+Z, +Y, +X)
	//
	// Direct3D: (Up, Front, Right) == (+Y, +Z, +X)
	//
	// Conversion from 3ds max to Direct3D coords:
	//
	// 3ds max * conversion matrix = Direct3D
	//
	// [ x y z w ] * | +1  0  0  0 | = [ X Y Z W ]
	//               |  0  0 +1  0 |
	//               |  0 +1  0  0 |
	//               |  0  0  0 +1 |
	//

	// Conversion matrix
	D3DXMatrixIdentity(&matTemp2);
	matTemp2.m[1][1] = 0.0f;
	matTemp2.m[1][2] = 1.0f;
	matTemp2.m[2][1] = 1.0f;
	matTemp2.m[2][2] = 0.0f;

	// Transform from Model Space to Direct3D World Space
	D3DXMatrixMultiply(&matTemp3, &matWorld, &matTemp2);

	D3DXMatrixTranspose(&matTemp1, &matTemp3);
	pd3dDevice->SetVertexShaderConstantF(CV_WORLD_0, (float*)&matTemp1(0,0), 4);

	// Use the Transpose of the Inverse of this Transform for the
	// Transformation of Normals from Model Space to Direct3D World Space
	D3DXMatrixInverse(&matTemp1, NULL, &matTemp3);

	// No Transpose necessary for setting the Shader Constants
	pd3dDevice->SetVertexShaderConstantF(CV_WORLDIT_0, (float*)&matTemp1(0,0), 3);

	// Camera position in Direct3D Camera Space
	D3DXVECTOR3 camPos, camWpos;
	camPos.x = camPos.y = camPos.z = 0.0f;

	// Inverse of View Transform to Transform Camera from Direct3D Camera Space
	// to 3ds max World Space
	D3DXMatrixInverse(&matTemp1, NULL, &matView);

	// Multiply in Transform to convert from 3ds max World Space
	// to Direct3D World Space
	D3DXMatrixMultiply(&matTemp3, &matTemp1, &matTemp2);

	// Find Camera position in Direct3D World Space
	D3DXVec3TransformCoord(&camWpos, &camPos, &matTemp3);
	pd3dDevice->SetVertexShaderConstantF(CV_EYE_WORLD, (float*)&camWpos, 1);
	pd3dDevice->SetVertexDeclaration(vdecl);
	return S_OK;
}

bool IDX9PixelVertexShaderImp::DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data)
{
	return false;
}

bool IDX9PixelVertexShaderImp::DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data)
{
	return false;
}

void IDX9PixelVertexShaderImp::StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data)
{
}

void IDX9PixelVertexShaderImp::AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis)
{
}

bool IDX9PixelVertexShaderImp::DrawLines(ID3D9GraphicsWindow *d3dgw)
{
	return false;
}

void IDX9PixelVertexShaderImp::EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}

void IDX9PixelVertexShaderImp::StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data)
{
}

void IDX9PixelVertexShaderImp::AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis)
{
}

bool IDX9PixelVertexShaderImp::DrawTriangles(ID3D9GraphicsWindow *d3dgw)
{
	return false;
}

void IDX9PixelVertexShaderImp::EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
}
HRESULT IDX9PixelVertexShaderImp::LoadAndCompileShader()
{

		HRESULT hr;
		TSTR vertexShaderPath;
			
		if(!map->GetVertexShaderFile().GetFullFilePath(vertexShaderPath))
		{
			return E_FAIL;
		}
		
		
		hr = D3DXAssembleShaderFromFile(vertexShaderPath,NULL, NULL, 0L, &pCode, NULL);
		
		SAFE_RELEASE(dwVertexShader);
		SAFE_RELEASE(vdecl);
		
		if(FAILED(hr))
		{
			return E_FAIL;
		}
		D3DVERTEXELEMENT9 dwDecl[] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
			D3DDECL_END()
		};

		hr = pd3dDevice->CreateVertexDeclaration(dwDecl, &vdecl);
		if (FAILED(hr)) {
			return hr;
		}
		

		hr = pd3dDevice->CreateVertexShader((LPDWORD)pCode->GetBufferPointer(), &dwVertexShader);

		return hr;

}
