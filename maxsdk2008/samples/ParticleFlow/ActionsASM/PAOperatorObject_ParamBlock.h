/**********************************************************************
 *<
	FILE:			PAOperatorObject_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATOROBJECT_PARAMBLOCK_H_
#define  _PAOPERATOROBJECT_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorObjectDesc.h"

namespace PartASM {

// block IDs
enum { kObject_mainPBlockIndex };


// param IDs
enum {	kObject_channelName,
		kObject_channelType,
		kObject_channelScope,
		kObject_propertyInteger,
		kObject_propertyFloat,
		kObject_propertyVector,
		kObject_useSubMtl,
		kObject_subMtl,
		kObject_mappingChannel,
		kObject_lockClosestPoint, // not used
		kObject_objects,
		kObject_objectsMaxscript,
		kObject_useDesignate,
		kObject_designateName,
		kObject_sync,
		kObject_syncName,
		kObject_animated,
		kObject_subframeLocation,
		kObject_subframeGeometry,
		kObject_assignOnce,
		kObject_useCondition,
		kObject_conditionName,
		kObject_compensateIntegration };

enum {	kObject_channelType_integer,
		kObject_channelType_float,
		kObject_channelType_vector,
		kObject_channelType_num };

enum {	kObject_channelScope_global,
		kObject_channelScope_local,
		kObject_channelScope_num };

enum {  kObject_propertyInteger_objectIndexByPivot,
		kObject_propertyInteger_objectIndexByVertex,
		kObject_propertyInteger_objectIndexBySurface,
		kObject_propertyInteger_vertexIndex,
		kObject_propertyInteger_faceIndex,
		kObject_propertyInteger_materialIndex,
		kObject_propertyInteger_num };

enum {  kObject_propertyFloat_distanceToPivot,
		kObject_propertyFloat_distanceToVertex,
		kObject_propertyFloat_distanceToSurface,
		kObject_propertyFloat_objectSpinRate,
		kObject_propertyFloat_pointLuminance,
		kObject_propertyFloat_pointOpacity,
		kObject_propertyFloat_faceSquare,
		kObject_propertyFloat_softSelection,
		kObject_propertyFloat_num };

enum {  kObject_propertyVector_nearestSurfacePoint,
		kObject_propertyVector_nearestVertex,
		kObject_propertyVector_nearestNormal,
		kObject_propertyVector_objectPosition,
		kObject_propertyVector_objectSpeed,
		kObject_propertyVector_localSpeed,
		kObject_propertyVector_objectScale,
		kObject_propertyVector_objectEuclideRotation,
		kObject_propertyVector_nearestMapping,
		kObject_propertyVector_mappingUVector,
		kObject_propertyVector_mappingVVector,
		kObject_propertyVector_mappingWVector,
		kObject_propertyVector_nearestColor,
		kObject_propertyVector_objectSpinAxis,
		kObject_propertyVector_num };

enum {	kObject_sync_time,
		kObject_sync_age,
		kObject_sync_event,
		kObject_sync_channel,
		kObject_sync_num };
	
// user messages
enum { kObject_RefMsg_NewRand = REFMSG_USER + 13279,
		kObject_RefMsg_ListSelect, // add more nodes by select dialog
		kObject_RefMsg_ResetValidatorAction
};

// dialog messages
enum {	kObject_message_update = 100 };		// change in parameters

class PAOperatorObjectDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorObjectDesc ThePAOperatorObjectDesc;
extern ParamBlockDesc2 object_paramBlockDesc;
extern FPInterfaceDesc object_action_FPInterfaceDesc;
extern FPInterfaceDesc object_operator_FPInterfaceDesc;
extern FPInterfaceDesc object_PViewItem_FPInterfaceDesc;


} // end of namespace PartASM

#endif // _PAOPERATOROBJECT_PARAMBLOCK_H_