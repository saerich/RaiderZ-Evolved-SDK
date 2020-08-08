/**********************************************************************
 *<
	FILE:			PATest_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Duration Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"

#include "PATest_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

// Each plug-in should have one instance of the ClassDesc class
PATestDesc ThePATestDesc;

// custom update dialog process
PATestDlgProc ThePATestDlgProc;

// Test param block
ParamBlockDesc2 test_paramBlockDesc 
(
	// required block spec
		kTest_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePATestDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kTest_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_TEST_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePATestDlgProc,
	// required param specs
		// channel name
			kTest_channelName,	_T("Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME,
			end,

		// data type
			kTest_channelType,	_T("Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CHANNELTYPE,
			// optional tagged param specs
					p_default,		kTest_channelType_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_CHANNELTYPE,	kTest_channelType_num,
												IDS_CUSTOMTYPE_ANGLE,	IDS_CUSTOMTYPE_BOOLEAN,
												IDS_CUSTOMTYPE_FLOAT,		
												IDS_CUSTOMTYPE_INTEGER,	IDS_CUSTOMTYPE_PERCENT,		
												IDS_CUSTOMTYPE_TIME,	IDS_CUSTOMTYPE_WORLDUNIT,
					p_vals,			kTest_channelType_angle,	kTest_channelType_boolean,
									kTest_channelType_float,			
									kTest_channelType_integer,	kTest_channelType_percent,	
									kTest_channelType_time,	kTest_channelType_worldFloat,
			end,

		// condition type: less or greater
			kTest_conditionType, _T("Test_True_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_TESTTRUETYPE,
			// optional tagged param specs
					p_default,		kTest_conditionType_isGreater,
					p_range,		0, kTest_conditionType_num-1,
					p_ui,			TYPE_RADIO, kTest_conditionType_num, IDC_ISLESS, IDC_ISGREATER,
			end,

		// use second value
			kTest_useValue2, _T("Use_Value#2"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USEVALUE2,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANDOR,
			end,

		// float value
			kTest_fValue1,			_T("Float_Value#1"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE1,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_FVALUE_1, IDC_FVALUESPIN_1, SPIN_AUTOSCALE,
			end,

		// integer value
			kTest_iValue1,			_T("Integer_Value#1"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERVALUE1,
			// optional tagged param specs
					p_default,		0,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_IVALUE_1, IDC_IVALUESPIN_1, 1.0f,
			end,

		// time value
			kTest_tValue1,			_T("Time_Value#1"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_TIMEVALUE1,
			// optional tagged param specs
					p_default,		0,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_TIME,
									IDC_TVALUE_1, IDC_TVALUESPIN_1, 80.0f,
			end,

		// percent value
			kTest_pValue1,			_T("Percent_Value#1"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PERCENTVALUE1,
			// optional tagged param specs
					p_default,		0.5f,
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PVALUE_1, IDC_PVALUESPIN_1, 0.5f,
			end,

		// world value
			kTest_wValue1,			_T("World_Unit_Value#1"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_WORLDUNITVALUE1,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_WVALUE_1, IDC_WVALUESPIN_1, SPIN_AUTOSCALE,
			end,

		// angle value
			kTest_aValue1,			_T("Angle_Value#1"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVALUE1,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AVALUE_1, IDC_AVALUESPIN_1, 0.5f,
			end,

		// float value
			kTest_fValue2,			_T("Float_Value#2"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE2,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_FVALUE_2, IDC_FVALUESPIN_2, SPIN_AUTOSCALE,
			end,

		// integer value
			kTest_iValue2,			_T("Integer_Value#2"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERVALUE2,
			// optional tagged param specs
					p_default,		10,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_IVALUE_2, IDC_IVALUESPIN_2, 1.0f,
			end,

		// time value
			kTest_tValue2,			_T("Time_Value#2"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_TIMEVALUE2,
			// optional tagged param specs
					p_default,		4800,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_TIME,
									IDC_TVALUE_2, IDC_TVALUESPIN_2, 80.0f,
			end,

		// percent value
			kTest_pValue2,			_T("Percent_Value#2"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PERCENTVALUE2,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PVALUE_2, IDC_PVALUESPIN_2, 0.5f,
			end,

		// world value
			kTest_wValue2,			_T("World_Unit_Value#2"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_WORLDUNITVALUE2,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_WVALUE_2, IDC_WVALUESPIN_2, SPIN_AUTOSCALE,
			end,

		// angle value
			kTest_aValue2,			_T("Angle_Value#2"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVALUE2,
			// optional tagged param specs
					p_default,		1.5707963f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AVALUE_2, IDC_AVALUESPIN_2, 0.5f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc test_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePATestDesc, FP_MIXIN, 
				
			IPFAction::kInit,	_T("init"),		0,	TYPE_bool, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actions"), 0, TYPE_OBJECT_TAB_BR,
				_T("actionNodes"), 0, TYPE_INODE_TAB_BR, 
			IPFAction::kRelease, _T("release"), 0, TYPE_bool, 0, 1,
				_T("container"), 0, TYPE_IOBJECT,
			// reserved for future use
			//IPFAction::kChannelsUsed, _T("channelsUsed"), 0, TYPE_VOID, 0, 2,
			//	_T("interval"), 0, TYPE_INTERVAL_BR,
			//	_T("channels"), 0, TYPE_FPVALUE,
			IPFAction::kActivityInterval, _T("activityInterval"), 0, TYPE_INTERVAL_BV, 0, 0,
			IPFAction::kIsFertile, _T("isFertile"), 0, TYPE_bool, 0, 0,
			IPFAction::kIsNonExecutable, _T("isNonExecutable"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportRand, _T("supportRand"), 0, TYPE_bool, 0, 0,
			IPFAction::kGetRand, _T("getRand"), 0, TYPE_INT, 0, 0,
			IPFAction::kSetRand, _T("setRand"), 0, TYPE_VOID, 0, 1,
				_T("randomSeed"), 0, TYPE_INT,
			IPFAction::kNewRand, _T("newRand"), 0, TYPE_INT, 0, 0,
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc test_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePATestDesc, FP_MIXIN,

			IPFTest::kProceedStep1, _T("proceedStep1"), 0, TYPE_VOID, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,
			IPFTest::kProceedStep2, _T("proceedStep2"), 0, TYPE_bool, 0, 6,
				_T("timeStartTick"), 0, TYPE_TIMEVALUE,
				_T("timeStartFraction"), 0, TYPE_FLOAT,
				_T("timeEndTick"), 0, TYPE_TIMEVALUE_BR,
				_T("timeEndFraction"), 0, TYPE_FLOAT_BR,
				_T("testResult"), 0, TYPE_BITARRAY_BR,
				_T("testTime"), 0, TYPE_FLOAT_TAB_BR,
			IPFTest::kGetNextActionList, _T("getNextActionList"), 0, TYPE_INODE, 0, 2,
				_T("testNode"), 0, TYPE_INODE,
				_T("linkActive"), 0, TYPE_bool_BP,
			IPFTest::kSetNextActionList, _T("setNextActionList"), 0, TYPE_bool, 0, 2,
				_T("actionList"), 0, TYPE_INODE,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kSetLinkActive, _T("setLinkActive"), 0, TYPE_bool, 0, 2,
				_T("linkActive"), 0, TYPE_bool,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kClearNextActionList, _T("clearNextActionList"), 0, TYPE_bool, 0, 1,
				_T("testNode"), 0, TYPE_INODE,
		end
);

FPInterfaceDesc test_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePATestDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,
			IPViewItem::kGetWireExtension, _T("getWireExtension"), 0, TYPE_INT, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("wireHeight"), 0, TYPE_INT_BR,
			IPViewItem::kSetWireExtension, _T("setWireExtension"), 0, TYPE_VOID, 0, 3,
				_T("action"), 0, TYPE_INODE,
				_T("wireWidth"), 0, TYPE_INT,
				_T("wireHeight"), 0, TYPE_INT,
		end
);


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|					custom UI update for Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
INT_PTR PATestDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			UpdateDlg(map, hWnd);
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case kTest_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PATestDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int channelType = ppb->GetInt(kTest_channelType, 0);
			bool isLessType = (ppb->GetInt(kTest_conditionType, 0) == kTest_conditionType_isLess);
			bool useSecond = (ppb->GetInt(kTest_useValue2, 0) != 0);
			if (useSecond && (channelType == kTest_channelType_boolean))
				useSecond = false;
			ISpinnerControl* spin = NULL;

			if (channelType == kTest_channelType_boolean) {
				SetDlgItemText( hWnd, IDC_ISLESS, GetString(IDS_ISFALSE));
				SetDlgItemText( hWnd, IDC_ISGREATER, GetString(IDS_ISTRUE));
			} else {
				SetDlgItemText( hWnd, IDC_ISLESS, GetString(IDS_ISLESSTHANVALUE1));
				SetDlgItemText( hWnd, IDC_ISGREATER, GetString(IDS_ISGREATERTHANVALUE1));
			}
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_VALUE1), channelType != kTest_channelType_boolean );
			EnableWindow( GetDlgItem(hWnd, IDC_ANDOR), channelType != kTest_channelType_boolean );

			spin = GetISpinner( GetDlgItem( hWnd, IDC_FVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_float);
			ReleaseISpinner(spin);
			int showControl = ((channelType == kTest_channelType_float) || (channelType == kTest_channelType_boolean)) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_FVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_float) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUESPIN_2), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_IVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_integer);
			ReleaseISpinner(spin);
			showControl = (channelType == kTest_channelType_integer) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_IVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_integer) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUESPIN_2), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_TVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_time);
			ReleaseISpinner(spin);
			showControl = (channelType == kTest_channelType_time) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_TVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_time) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUESPIN_2), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_PVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_percent);
			ReleaseISpinner(spin);
			showControl = (channelType == kTest_channelType_percent) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_percent) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUESPIN_2), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_WVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_worldFloat);
			ReleaseISpinner(spin);
			showControl = (channelType == kTest_channelType_worldFloat) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_WVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_worldFloat) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUESPIN_2), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_AVALUESPIN_1) );
			spin->Enable(channelType == kTest_channelType_angle);
			ReleaseISpinner(spin);
			showControl = (channelType == kTest_channelType_angle) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUE_1), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUESPIN_1), showControl);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_AVALUESPIN_2) );
			spin->Enable((channelType == kTest_channelType_angle) && useSecond);
			ReleaseISpinner(spin);
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUE_2), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUESPIN_2), showControl);

			SetDlgItemText( hWnd, IDC_ANDOR, isLessType ? GetString(IDS_ORGREATERTHANVALUE2) : GetString(IDS_ANDLESSTHANVALUE2));
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_VALUE2), useSecond );
		}
	}
}



} // end of namespace PartASM
