/**********************************************************************
 *<
	FILE: RefCheck.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __REFCHECK__H
#define __REFCHECK__H

#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include "istdplug.h"
#include "stdmat.h"
#include "bmmlib.h"
#include <io.h>

#include "iparamb2.h"
#include "iparamm2.h"
#include "IDxMaterial.h"
#include <ifnpub.h>

#define REFCHECK_CLASS_ID	Class_ID(0xa7d423ed, 0x64de98f9)
#define REFCHECK_FP_INTERFACE  Interface_ID(0x5bbf4ce8, 0x31c00a8)

extern ClassDesc*	GetRefCheckDesc();
extern HINSTANCE	hInstance;
extern TCHAR*		GetString(int id);

#define COPYWARN_YES		0x00
#define COPYWARN_YESTOALL	0x01
#define COPYWARN_NO			0x02
#define COPYWARN_NOTOALL	0x03

/**
 * The NameEnumCallBack used to find all Light Dist. files.
 */

class EnumLightDistFileCallBack : public AssetEnumCallback
{
public:

	NameTab fileList;
	int size;

	EnumLightDistFileCallBack() { size = 0; }

	void RecordAsset(const MaxSDK::AssetManagement::AssetUser&asset)
	{
		//convert to lower case
		TSTR fullPath = asset.GetFullFilePath();
		if(fullPath.isNull())
			fullPath = asset.GetFileName();
		//convert to lower case
		fullPath.toLower();
		// check for distribution types before appending.
		if(_tcsstr(fullPath, ".ies") || 
			_tcsstr(fullPath, ".cibse") ||
			_tcsstr(fullPath, ".ltli"))		{
			if(fileList.FindName(fullPath) < 0)	{
				fileList.AddName(fullPath);
				size++;
			}
		}
			
	}
};

//! \brief The pop-up dialog class of the Bitmap/photometric Path Editor utility.
class BitmapPathEditorDialog
{
	// Lingchun Li 8.12.2008
	/* This class is separated from the original class RefCheck. For now, the RefCheck
	class is only responsible for initialize/de-initialized the rollup-page for this
	module on the utility panel and it will call BitmapPathEditorDialog::DoDialog() 
	when user invoke the pop-up dialog(i.e,Bitmap/photometric Path Editor) and the 
	rest things	are all done in this class.*/
public:
	HWND hDialog;
	TSTR bitmapName;
	 // The core interface instance.
	Interface&  mInterface;	

	BitmapPathEditorDialog();
	~BitmapPathEditorDialog();

	// Main dialog resize functions
	void ResizeWindow(int x, int y);
	void SetMinDialogSize(int w, int h)	{ minDlgWidth = w; minDlgHeight = h; }
	int	 GetMinDialogWidth()			{ return minDlgWidth; }
	int  GetMinDialogHeight()			{ return minDlgHeight; }

	void CheckDependencies();
	void Update();
	void StripSelected();
	void StripAll();
	//! \brief Invoke the popup BitmapPathEditorDialog(i.e,Bitmap/photometric Path Editor). 
	/*! \param[in] BOOL ToCheckExternalMtlLibDependency - This variable states whether
	the BitmapPathEditorDialog should check the external material library resource
	(i.e.,material library/material editor) dependency.
	Set it true when the BitmapPathEditorDialog is invoked on the utility panel.
	Set it false otherwise.
	*/ 
	void DoDialog(bool includeMatLib, bool includeMedit);
	void DoSelection();
	void SetPath(const TCHAR* path, BitmapTex* map);
	void SetDxPath(MtlBase * mtl, TCHAR * path, TCHAR * newPath, TCHAR *ext);

	void StripMapName(TCHAR* path);
	void BrowseDirectory();
	BOOL ChooseDir(TCHAR *title, TCHAR *dir);
	void EnableEntry(HWND hWnd, BOOL bEnable, int numSel);
	void ShowInfo();
	void HandleInfoDlg(HWND dlg);

	BOOL GetIncludeMatLib();
	BOOL GetIncludeMedit();

	int  CopyWarningPrompt(HWND hParent, TCHAR* filename);
	void CopyMaps();
	void SetActualPath();
	void SelectMissing();

	BOOL FindMap(TCHAR* mapName, TCHAR* newName);

	BOOL GetCopyQuitFlag()	{ return bCopyQuitFlag; }
	void SetCopyQuitFlag(BOOL bStatus)	{ bCopyQuitFlag = bStatus; }
	void SetInfoTex(BitmapTex* b) { infoTex = b; }
	BitmapTex* GetInfoTex() { return infoTex; }
	void EnumerateNodes(INode *root);

	//! \brief ReInitialize the BitmapPathEditorDialog.
	/*! Reinitialize the DataIndex instance and clear the content of the internal file list.
	*/
	void ReInitialize();

private:
	BOOL		bCopyQuitFlag;
	BitmapTex*	infoTex;
	int			minDlgWidth;
	int			minDlgHeight;
	EnumLightDistFileCallBack distributionLister;
	
	// The internal control variable determines whether the BitmapPathEditorDialog
	// will check bitmap resources used by the material library.
	BOOL		mCheckMaterialLibraryDependency;
	// The internal control variable determines whether the BitmapPathEditorDialog
	// will check bitmap resources used by the material editor.
	BOOL		mCheckMaterialEditorDependency;
};

class RefCheck : public UtilityObj {
public:
	IUtil *iu;
	Interface *ip;
	HWND hPanel;
	BitmapPathEditorDialog mBitmapPathEditorDialog;

	RefCheck();

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void DeleteThis() {}

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);

	void DoDialog(BOOL ToCheckExternalMtlLibDependency = FALSE);

	bool GetIncludeMatLib();
	bool GetIncludeMedit();
	//PBITMAPINFO	pDib;
};

class FindNodesProc : public DependentEnumProc {
public:
	FindNodesProc(INodeTab* tab) {
		nodetab = tab;
	}
	int proc(ReferenceMaker *ref) {
		switch (ref->SuperClassID()) {
				case BASENODE_CLASS_ID:
					INode* n = (INode*)ref;
					nodetab->Append(1, &n, 5);
					return DEP_ENUM_SKIP;
					break;
		}
		return DEP_ENUM_CONTINUE;
	}
private:
	INodeTab* nodetab;
};
#endif // __REFCHECK__H
