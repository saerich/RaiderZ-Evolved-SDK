/**********************************************************************
*<
FILE: DxStdMtl2.h

DESCRIPTION:	DirectX Interface for the standard material

CREATED BY:		Neil Hazzard

HISTORY:		Created:  04/19/04
				The EffectElements class has been taken from the DxMaterial.  In the standard material case we are not
				hosting a UI, so it is much simpler.  We take care of converting textures to D3D versions, but all multi
				pass handling is done by the parser.
				Modified: 20-Sept-2008
				Updated to use the new IRTShaderNode for use with the shade tree tools


*>	Copyright (c) 2004, All Rights Reserved.
************************************************************************/

#ifndef	__DXSTDMTL2_H__
#define __DXSTDMTL2_H__

#include "max.h"
#include "ID3D9GraphicsWindow.h"
#ifndef IDX9_PIXELSHADER9_H
#include "idx9pixelshader.h"
#endif

#ifndef IDX9_VERTEXSHADER9_H
#include "idx9vertexshader.h"
#endif

#include <IRTShaderNode.h>
#include <IViewportManager.h>

#include "RTMax.h"
#include "stdmat.h"

class DxStdMtl2;


class DxStdMtl2 : public IDX9VertexShader,public IEffectFile
{
private:
	Animatable * m_pHostMaterial;
	IRTShaderNode * m_pRTShaderNode;
public:

	DxStdMtl2(Animatable * pHost);
	~DxStdMtl2();

	void Render();

	HRESULT Initialize(Mesh *mesh, INode *node);
	HRESULT Initialize(MNMesh *mnmesh, INode *node);

	// From IVertexShader
	HRESULT ConfirmDevice(ID3D9GraphicsWindow *d3dgw);
	HRESULT ConfirmPixelShader(IDX9PixelShader *pps);
	bool CanTryStrips();
	int GetNumMultiPass();

	LPDIRECT3DVERTEXSHADER9 GetVertexShaderHandle(int numPass) { return 0; }
	HRESULT SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass);

	// Draw 3D mesh as TriStrips
	bool	DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data);

	// Draw 3D mesh as wireframe
	bool	DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);

	// Draw 3D lines
	void	StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data){};
	void	AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis){};
	bool	DrawLines(ID3D9GraphicsWindow *d3dgw){return false;}
	void	EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn){};

	// Draw 3D triangles
	void	StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data){};
	void	AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis){};
	bool	DrawTriangles(ID3D9GraphicsWindow *d3dgw){return false;}
	void	EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn){};

	//IEffectFile
	bool SaveEffectFile(TCHAR * fileName);

};

#endif
