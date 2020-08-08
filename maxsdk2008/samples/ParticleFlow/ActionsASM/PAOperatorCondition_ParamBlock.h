/**********************************************************************
 *<
	FILE:			PAOperatorCondition_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORCONDITION_PARAMBLOCK_H_
#define  _PAOPERATORCONDITION_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorConditionDesc.h"

namespace PartASM {

// block IDs
enum { kCondition_mainPBlockIndex };


// param IDs
enum {	kCondition_channelName,
		kCondition_channelType,
		kCondition_conditionType,
		kCondition_useValue2,
		kCondition_iValue1,
		kCondition_tValue1,
		kCondition_fValue1,
		kCondition_pValue1,
		kCondition_wValue1,
		kCondition_aValue1,
		kCondition_iValue2,
		kCondition_tValue2,
		kCondition_fValue2,
		kCondition_pValue2,
		kCondition_wValue2,
		kCondition_aValue2,
		kCondition_assignOnce,
		kCondition_conditionName,
		kCondition_conditionScope };

enum {	kCondition_channelType_time,
		kCondition_channelType_integer,
		kCondition_channelType_float,
		kCondition_channelType_percent,
		kCondition_channelType_worldFloat,
		kCondition_channelType_angle,
		kCondition_channelType_num };

enum {	kCondition_conditionType_isLess,
		kCondition_conditionType_isGreater,
		kCondition_conditionType_num };

enum {	kCondition_conditionScope_global,
		kCondition_conditionScope_local,
		kCondition_conditionScope_num };

// user messages
enum { kCondition_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kCondition_message_update = 100 };		// change in parameters

class PAOperatorConditionDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorConditionDesc ThePAOperatorConditionDesc;
extern ParamBlockDesc2 condition_paramBlockDesc;
extern FPInterfaceDesc condition_action_FPInterfaceDesc;
extern FPInterfaceDesc condition_operator_FPInterfaceDesc;
extern FPInterfaceDesc condition_PViewItem_FPInterfaceDesc;

} // end of namespace PartASM

#endif // _PAOPERATORCONDITION_PARAMBLOCK_H_