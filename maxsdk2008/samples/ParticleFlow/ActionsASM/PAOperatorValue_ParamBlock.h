/**********************************************************************
 *<
	FILE:			PAOperatorValue_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORVALUE_PARAMBLOCK_H_
#define  _PAOPERATORVALUE_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorValueDesc.h"

namespace PartASM {

// block IDs
enum { kValue_mainPBlockIndex };


// param IDs
enum {	kValue_name,
		kValue_type,
		kValue_scope,
		kValue_iValue,
		kValue_tValue,
		kValue_fValue,
		kValue_pValue,
		kValue_wValue,
		kValue_aValue,
		kValue_fVector,
		kValue_pVector,
		kValue_wVector,
		kValue_aVector,
		kValue_assignOnce,
		kValue_syncType,
		kValue_useCondition,
		kValue_conditionName };

enum {	kValue_type_time,
		kValue_type_integer,
		kValue_type_float,
		kValue_type_percent,
		kValue_type_worldFloat,
		kValue_type_angle,
		kValue_type_floatVector,
		kValue_type_percentVector,
		kValue_type_worldVector,
		kValue_type_angleVector,
		kValue_type_num };

enum {	kValue_scope_global,
		kValue_scope_local,
		kValue_scope_num };

enum {	kValue_sync_time,
		kValue_sync_age,
		kValue_sync_event,
		kValue_sync_num };
	
// user messages
enum { kValue_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kValue_message_update = 100 };		// change in parameters

class PAOperatorValueDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorValueDesc ThePAOperatorValueDesc;
extern ParamBlockDesc2 value_paramBlockDesc;
extern FPInterfaceDesc value_action_FPInterfaceDesc;
extern FPInterfaceDesc value_operator_FPInterfaceDesc;
extern FPInterfaceDesc value_PViewItem_FPInterfaceDesc;


extern float kOperatorValueFloatValues[kNum3DVectorValues];
extern float kOperatorValuePercentValues[kNum3DVectorValues];
extern float kOperatorValueWorldValues[kNum3DVectorValues];
extern float kOperatorValueAngleValues[kNum3DVectorValues];



} // end of namespace PartASM

#endif // _PAOPERATORVALUE_PARAMBLOCK_H_