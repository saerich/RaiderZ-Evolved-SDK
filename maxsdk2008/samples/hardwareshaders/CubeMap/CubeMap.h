/**********************************************************************
 *<
	FILE: CubeMap.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:		Neil Hazzard

	HISTORY: created 04/30/02
	Based on the code by Nikolai Sander from the r4 samples

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __CUBEMAP__H
#define __CUBEMAP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "ID3D9GraphicsWindow.h"
#include "IDX9PixelShader.h"
#include "IDX9VertexShader.h"

#include "IStdDualVS.h"
#include "reflect.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define SAFE_DELETE(p)			{ if (p) { delete (p);		(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete[] (p);	(p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release();	(p)=NULL; } }

class CubeMap;

class IDX9PixelShaderImp : public IDX9PixelShader 
{
	bool	initDone;

	LPDIRECT3DDEVICE9 pd3dDevice;
	LPDIRECT3DCUBETEXTURE9	pCubeTexture;

	LPDIRECT3DPIXELSHADER9	dwPixelShader;
	
	CubeMap *map;

public :
	IDX9PixelShaderImp(CubeMap *map);
	~IDX9PixelShaderImp();

	HRESULT ConfirmDevice(ID3D9GraphicsWindow *d3dgw);

	HRESULT	ConfirmVertexShader(IDX9VertexShader *pvs);

	HRESULT Initialize(Material *mtl, INode *node);

	int GetNumMultiPass();

	LPDIRECT3DPIXELSHADER9 GetPixelShaderHandle(int numPass) { return dwPixelShader; }

	HRESULT SetPixelShader(ID3D9GraphicsWindow *d3dgw, int numPass);
};

class IDX9VertexShaderCache : public VertexShaderCache
{
public:
		
};

class IDX9PixelVertexShaderImp : public IDX9VertexShader, public IStdDualVSCallback
{
	bool initDone;

	LPDIRECT3DDEVICE9 pd3dDevice;
	LPDIRECT3DCUBETEXTURE9	pCubeTexture;
	LPDIRECT3DVERTEXDECLARATION9 vdecl;
	// VertexShader Declarations
	Tab<DWORD> Declaration;

	// VertexShader Instructions
	LPD3DXBUFFER pCode;

	// VertexShader Constants
	Tab<D3DXVECTOR4> Constants;

		// VertexShader Handle
	LPDIRECT3DVERTEXSHADER9	dwVertexShader;
	
	IStdDualVS *stdDualVS;
	CubeMap *map;

public:
	IDX9PixelVertexShaderImp(CubeMap *map);
	~IDX9PixelVertexShaderImp();
	
	virtual HRESULT Initialize(Mesh *mesh, INode *node);
	virtual HRESULT Initialize(MNMesh *mnmesh, INode *node);
	// From FPInterface
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// From IVertexShader
	HRESULT ConfirmDevice(ID3D9GraphicsWindow *d3dgw);
	
	HRESULT ConfirmPixelShader(IDX9PixelShader *pps);

	bool CanTryStrips();

	int GetNumMultiPass();

	LPDIRECT3DVERTEXSHADER9 GetVertexShaderHandle(int numPass) { return dwVertexShader; }

	HRESULT SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass);
	
	// Draw 3D mesh as TriStrips
	bool	DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data);

	// Draw 3D mesh as wireframe
	bool	DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);

	// Draw 3D lines
	void	StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data);
	void	AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis);
	bool	DrawLines(ID3D9GraphicsWindow *d3dgw);
	void	EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// Draw 3D triangles
	void	StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data);
	void	AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis);
	bool	DrawTriangles(ID3D9GraphicsWindow *d3dgw);
	void	EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn);

	// from IStdDualVSCallback
	virtual ReferenceTarget *GetRefTarg();
	virtual VertexShaderCache *CreateVertexShaderCache();
	virtual HRESULT  InitValid(Mesh* mesh, INode *node);
	virtual HRESULT  InitValid(MNMesh* mnmesh, INode *node);
	HRESULT LoadAndCompileShader();
};

#endif // __CUBEMAP__H
