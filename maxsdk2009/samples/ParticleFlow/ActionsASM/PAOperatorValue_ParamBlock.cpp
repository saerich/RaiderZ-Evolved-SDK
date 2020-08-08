/**********************************************************************
 *<
	FILE:			PAOperatorValue_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "PAOperatorValue_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

float kOperatorValueFloatValues[kNum3DVectorValues] = { 0.0f, 0.0f, 0.0f };
float kOperatorValuePercentValues[kNum3DVectorValues] = { 1.0f, 1.0f, 1.0f };
float kOperatorValueWorldValues[kNum3DVectorValues] = { 0.0f, 0.0f, 0.0f };
float kOperatorValueAngleValues[kNum3DVectorValues] = { 0.0f, 0.0f, 0.0f };

// Each plug-in should have one instance of the ClassDesc class
PAOperatorValueDesc ThePAOperatorValueDesc;

// Dialog Proc
PAOperatorValueDlgProc ThePAOperatorValueDlgProc;

// Value Operator param block
ParamBlockDesc2 value_paramBlockDesc 
(
	// required block spec
		kValue_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorValueDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kValue_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_VALUE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorValueDlgProc,
	// required param specs
		// channel name
			kValue_name,	_T("Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME,
			end,

		// data type
			kValue_type,	_T("Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CHANNELTYPE,
			// optional tagged param specs
					p_default,		kValue_type_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_CHANNELTYPE,	kValue_type_num,
												IDS_CUSTOMTYPE_ANGLE,		IDS_CUSTOMTYPE_EUCLIDEROTATION,
												IDS_CUSTOMTYPE_FLOAT,		IDS_CUSTOMTYPE_FLOATVECTOR,	
												IDS_CUSTOMTYPE_INTEGER,		IDS_CUSTOMTYPE_PERCENT,		
												IDS_CUSTOMTYPE_PERCENTVECTOR,IDS_CUSTOMTYPE_TIME,		
												IDS_CUSTOMTYPE_WORLDUNIT,	IDS_CUSTOMTYPE_WORLDVECTOR,
					p_vals,			kValue_type_angle,			kValue_type_angleVector,
									kValue_type_float,			kValue_type_floatVector,
									kValue_type_integer,		kValue_type_percent,	
									kValue_type_percentVector,	kValue_type_time,		
									kValue_type_worldFloat,		kValue_type_worldVector,
			end,

		// scope type: global or local
			kValue_scope,		_T("Scope"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPE,
			// optional tagged param specs
					p_default,		kValue_scope_local,
					p_range,		0, kValue_scope_num-1,
					p_ui,			TYPE_RADIO, kValue_scope_num, IDC_GLOBAL, IDC_LOCAL,
			end,

		// float value
			kValue_fValue,			_T("Float_Value"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_FVALUE, IDC_FVALUESPIN, SPIN_AUTOSCALE,
			end,

		// integer value
			kValue_iValue,			_T("Integer_Value"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERVALUE,
			// optional tagged param specs
					p_default,		0,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_IVALUE, IDC_IVALUESPIN, 1.0f,
			end,

		// time value
			kValue_tValue,			_T("Time_Value"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_TIMEVALUE,
			// optional tagged param specs
					p_default,		0,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_TIME,
									IDC_TVALUE, IDC_TVALUESPIN, 80.0f,
			end,

		// percent value
			kValue_pValue,			_T("Percent_Value"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PERCENTVALUE,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PVALUE, IDC_PVALUESPIN, 0.5f,
			end,

		// world value
			kValue_wValue,			_T("World_Unit_Value"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_WORLDUNITVALUE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_WVALUE, IDC_WVALUESPIN, SPIN_AUTOSCALE,
			end,

		// angle value
			kValue_aValue,			_T("Angle_Value"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVALUE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AVALUE, IDC_AVALUESPIN, 0.5f,
			end,

		// float vector 
			kValue_fVector,		_T("Float_Vector"),
									TYPE_FLOAT_TAB, kNum3DVectorValues,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVECTOR,
			// optional tagged param specs
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_FX, IDC_FXSPIN, 
									IDC_FY, IDC_FYSPIN, 
									IDC_FZ, IDC_FZSPIN, 
									SPIN_AUTOSCALE,
			end,

		// percent vector 
			kValue_pVector,		_T("Percent_Vector"),
									TYPE_PCNT_FRAC_TAB, kNum3DVectorValues,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PERCENTVECTOR,
			// optional tagged param specs
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PX, IDC_PXSPIN, 
									IDC_PY, IDC_PYSPIN, 
									IDC_PZ, IDC_PZSPIN, 
									1.0f,
			end,

		// world vector 
			kValue_wVector,		_T("World_Vector"),
									TYPE_WORLD_TAB, kNum3DVectorValues,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_WORLDVECTOR,
			// optional tagged param specs
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_WX, IDC_WXSPIN, 
									IDC_WY, IDC_WYSPIN, 
									IDC_WZ, IDC_WZSPIN, 
									SPIN_AUTOSCALE,
			end,

		// angle vector 
			kValue_aVector,		_T("Angle_Vector"),
									TYPE_ANGLE_TAB, kNum3DVectorValues,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVECTOR,
			// optional tagged param specs
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AX, IDC_AXSPIN, 
									IDC_AY, IDC_AYSPIN, 
									IDC_AZ, IDC_AZSPIN, 
									0.01745329f,
			end,

		// assign only once (to new particles only)
			kValue_assignOnce, _T("Assign_Only_Once"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNONLYONCE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNONCE,
			end,

		// sync type
			kValue_syncType,	_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kValue_sync_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kValue_sync_num,
												IDS_SYNCTYPE_TIME, IDS_SYNCTYPE_AGE, IDS_SYNCTYPE_EVENT,
					p_vals,			kValue_sync_time, kValue_sync_age, kValue_sync_event,
			end,

		// use condition filter
			kValue_useCondition, _T("Use_Condition_Filter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECONDITIONFILTER,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECONDITION,
			end,

		// condition channel name
			kValue_conditionName,	_T("Condition_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CONDITIONCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CONDITIONNAME,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc value_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorValueDesc, FP_MIXIN, 
				
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

FPInterfaceDesc value_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorValueDesc, FP_MIXIN,

			IPFOperator::kProceed, _T("proceed"), 0, TYPE_bool, 0, 7,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeStart"), 0, TYPE_TIMEVALUE,
				_T("timeEnd"), 0, TYPE_TIMEVALUE_BR,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,

		end
);

FPInterfaceDesc value_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorValueDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("actionNode"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,

		end
);

INT_PTR PAOperatorValueDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
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
				case kValue_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PAOperatorValueDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
//	BOOL bAxes, bDivergence, bRandom;
//	bool varAnimated, divAnimated;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int type = ppb->GetInt(kValue_type, 0);
			bool useFilter = (ppb->GetInt(kValue_useCondition, 0) != 0);
			bool scalarType = (type < kValue_type_floatVector);
			bool vectorType = (type >= kValue_type_floatVector);
			ISpinnerControl* spin = NULL;

			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_SCALAR), scalarType );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_VALUE), scalarType );

			spin = GetISpinner( GetDlgItem( hWnd, IDC_FVALUESPIN) );
			spin->Enable(type == kValue_type_float);
			ReleaseISpinner(spin);
			int showControl = (!scalarType || (type == kValue_type_float)) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FVALUESPIN), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_IVALUESPIN) );
			spin->Enable(type == kValue_type_integer);
			ReleaseISpinner(spin);
			showControl = (type == kValue_type_integer) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_IVALUESPIN), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_TVALUESPIN) );
			spin->Enable(type == kValue_type_time);
			ReleaseISpinner(spin);
			showControl = (type == kValue_type_time) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_TVALUESPIN), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_PVALUESPIN) );
			spin->Enable(type == kValue_type_percent);
			ReleaseISpinner(spin);
			showControl = (type == kValue_type_percent) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PVALUESPIN), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_WVALUESPIN) );
			spin->Enable(type == kValue_type_worldFloat);
			ReleaseISpinner(spin);
			showControl = (type == kValue_type_worldFloat) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WVALUESPIN), showControl);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_AVALUESPIN) );
			spin->Enable(type == kValue_type_angle);
			ReleaseISpinner(spin);
			showControl = (type == kValue_type_angle) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUE), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AVALUESPIN), showControl);

			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_VECTOR), vectorType );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_X), vectorType );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_Y), vectorType );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_Z), vectorType );

			bool enable = (type == kValue_type_floatVector);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_FXSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_FYSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_FZSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			showControl = (!vectorType || enable) ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_FX), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FXSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FY), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FYSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FZ), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_FZSPIN), showControl);

			enable = (type == kValue_type_worldVector);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_WXSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_WYSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_WZSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			showControl = enable ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_WX), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WXSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WY), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WYSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WZ), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_WZSPIN), showControl);

			enable = (type == kValue_type_percentVector);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PXSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PYSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PZSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			showControl = enable ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_PX), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PXSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PY), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PYSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PZ), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_PZSPIN), showControl);

			enable = (type == kValue_type_angleVector);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_AXSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_AYSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_AZSPIN) );
			spin->Enable(enable);
			ReleaseISpinner(spin);
			showControl = enable ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_AX), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AXSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AY), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AYSPIN), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AZ), showControl);
			ShowWindow( GetDlgItem( hWnd, IDC_AZSPIN), showControl);

			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CONDITIONNAME), useFilter );
			EnableWindow( GetDlgItem(hWnd, IDC_CONDITIONNAME), useFilter );
		}
	}
}




} // end of namespace PartASM