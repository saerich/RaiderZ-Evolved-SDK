/**********************************************************************
 *<
	FILE:			PATest_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Duration Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PATEST_PARAMBLOCK_H_
#define  _PATEST_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PATestDesc.h"

namespace PartASM {

// block IDs
enum { kTest_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kTest_channelName,
		kTest_channelType,
		kTest_conditionType,
		kTest_useValue2,
		kTest_iValue1,
		kTest_tValue1,
		kTest_fValue1,
		kTest_pValue1,
		kTest_wValue1,
		kTest_aValue1,
		kTest_iValue2,
		kTest_tValue2,
		kTest_fValue2,
		kTest_pValue2,
		kTest_wValue2,
		kTest_aValue2 };

enum {	kTest_channelType_boolean,
		kTest_channelType_time,
		kTest_channelType_integer,
		kTest_channelType_float,
		kTest_channelType_percent,
		kTest_channelType_worldFloat,
		kTest_channelType_angle,
		kTest_channelType_num };

enum {	kTest_conditionType_isLess,
		kTest_conditionType_isGreater,
		kTest_conditionType_num };

enum {	kTest_conditionType_isFalse,
		kTest_conditionType_isTrue };

// user messages
enum { kTest_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kTest_message_update = 100 };		// change in parameters

class PATestDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PATestDesc ThePATestDesc;
extern ParamBlockDesc2 test_paramBlockDesc;
extern FPInterfaceDesc test_action_FPInterfaceDesc;
extern FPInterfaceDesc test_test_FPInterfaceDesc;
extern FPInterfaceDesc test_PViewItem_FPInterfaceDesc;



} // end of namespace PartASM

#endif // _PATEST_PARAMBLOCK_H_