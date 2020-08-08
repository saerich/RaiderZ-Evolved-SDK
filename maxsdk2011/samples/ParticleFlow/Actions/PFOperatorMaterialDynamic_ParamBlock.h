/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamic_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for MaterialDynamic Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

	*>	Copyright 2008 Autodesk, Inc.  All rights reserved.

	Use of this software is subject to the terms of the Autodesk license
	agreement provided at the time of installation or download, or which
	otherwise accompanies this software in either electronic or hard copy form.  

 **********************************************************************/

#ifndef  _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_
#define  _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorMaterialDynamicDesc.h"

namespace PFActions {

const int	kMaterialDynamic_minNumMtls = 0;
const float kMaterialDynamic_minRate = 0.00001f;
const float kMaterialDynamic_maxRate = 1000000.0f;


// block IDs
enum { kMaterialDynamic_mainPBlockIndex };


// param IDs
enum {	kMaterialDynamic_assignMaterial,
		kMaterialDynamic_material,
		kMaterialDynamic_assignID,
		kMaterialDynamic_showInViewport,
		kMaterialDynamic_type,
		kMaterialDynamic_resetAge,
		kMaterialDynamic_randomizeAge,
		kMaterialDynamic_maxAgeOffset,
		kMaterialDynamic_materialID,
		kMaterialDynamic_numSubMtls,
		kMaterialDynamic_rate,
		kMaterialDynamic_loop,
		kMaterialDynamic_sync,
		kMaterialDynamic_randomizeRotoscoping,
		kMaterialDynamic_maxRandOffset,
		kMaterialDynamic_randomSeed };

enum {	kMaterialDynamic_type_particleID,
		kMaterialDynamic_type_materialID,
		kMaterialDynamic_type_cycle,
		kMaterialDynamic_type_random,
		kMaterialDynamic_type_num=4 };

enum {	kMaterialDynamic_sync_time,
		kMaterialDynamic_sync_age,
		kMaterialDynamic_sync_event,
		kMaterialDynamic_sync_num=3 };

// User Defined Reference Messages from PB
enum {	kMaterialDynamic_RefMsg_InitDlg = REFMSG_USER + 13279,
		kMaterialDynamic_RefMsg_NewRand };

// dialog messages
enum {	kMaterialDynamic_message_assign = 100 };


class PFOperatorMaterialDynamicDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateDlg( HWND hWnd, IParamBlock2* pblock, TimeValue t);
};

extern PFOperatorMaterialDynamicDesc ThePFOperatorMaterialDynamicDesc;
extern ParamBlockDesc2 materialDynamic_paramBlockDesc;
extern FPInterfaceDesc materialDynamic_action_FPInterfaceDesc;
extern FPInterfaceDesc materialDynamic_operator_FPInterfaceDesc;
extern FPInterfaceDesc materialDynamic_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_