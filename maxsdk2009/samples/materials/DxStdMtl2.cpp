/**********************************************************************
*<
FILE: DxStdMtl2.cpp

DESCRIPTION:	DirectX interface for the Standard Material

CREATED BY:		Neil Hazzard

HISTORY:		Created:  19/04/04


*>	Copyright (c) 2004, All Rights Reserved.
**********************************************************************/


#include "mtlhdr.h"
#include "mtlres.h"
#include "DxStdMtl2.h"
#include "custattrib.h"
#include "icustattribcontainer.h"
#include "IViewportManager.h"
#include "IMaterialViewportShading.h"

#include "rtmax.h"

#pragma comment( lib, "rtmax.lib" )
#pragma comment( lib, "mnmath.lib" ) 

#define VIEWPORTLOADER_CLASS_ID Class_ID(0x5a06293c, 0x30420c1e)

#define RENTRANT_COUNT 3

//#define FORCE_SMOKETEST

DxStdMtl2::DxStdMtl2(Animatable * pHost)
{
	m_pHostMaterial = pHost;
	m_pRTShaderNode = NULL;
	m_pRTShaderNode = IRTShaderManager::GetRTShaderManager()->AddShaderNode(m_pHostMaterial);
}

DxStdMtl2::~DxStdMtl2()
{
	IRTShaderManager::GetRTShaderManager()->RemoveShaderNode(m_pRTShaderNode);
}

void DxStdMtl2::Render()
{
	if(m_pRTShaderNode)
	{
		m_pRTShaderNode->DrawObjects();
	}
}

HRESULT DxStdMtl2::Initialize(Mesh *mesh, INode *node)
{
	if(m_pRTShaderNode)
	{
		m_pRTShaderNode->SetRenderObjects(node,mesh);
	}
	Render();
	return S_OK;
}

HRESULT DxStdMtl2::Initialize(MNMesh *mnmesh, INode *node)
{
	if(m_pRTShaderNode)
	{
		m_pRTShaderNode->SetRenderObjects(node,mnmesh);
	}
	Render();
	return S_OK;
}
HRESULT DxStdMtl2::ConfirmDevice(ID3D9GraphicsWindow *d3dgw)
{
	m_pRTShaderNode->RegisterGraphicsWindow(d3dgw);
	return S_OK;
}
HRESULT DxStdMtl2::ConfirmPixelShader(IDX9PixelShader *pps)
{
	return S_OK;
}
bool DxStdMtl2::CanTryStrips()
{
	return true;
}

int DxStdMtl2::GetNumMultiPass()
{
	return 0;
}

HRESULT DxStdMtl2::SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass)
{
	return S_OK;
}

bool DxStdMtl2::DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data)
{
	return true;
}

bool DxStdMtl2::DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data)
{
	return true;
}

bool DxStdMtl2::SaveEffectFile(TCHAR * fileName)
{
	FILE * effectStream = NULL;
	const TCHAR * buffer = IRTShaderManager2::GetRTShaderManager2()->GetShaderCode(m_pHostMaterial);
	if(buffer == NULL)
	{
		return false;
	}
	effectStream = _tfopen(fileName,"w");
	if(!effectStream)
	{
		return false;
	}
	_ftprintf(effectStream,"%s",buffer);
	fclose(effectStream);
	return true;
}