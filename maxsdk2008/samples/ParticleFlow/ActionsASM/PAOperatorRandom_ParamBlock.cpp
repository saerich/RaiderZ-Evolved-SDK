/**********************************************************************
 *<
	FILE:			PAOperatorRandom_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "PAOperatorRandom_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

// Each plug-in should have one instance of the ClassDesc class
PAOperatorRandomDesc ThePAOperatorRandomDesc;

// Dialog Proc
PAOperatorRandomDlgProc ThePAOperatorRandomDlgProc;

// Value Operator param block
ParamBlockDesc2 random_paramBlockDesc 
(
	// required block spec
		kRandom_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorRandomDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kRandom_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_RANDOM_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorRandomDlgProc,
	// required param specs
		// channel name
			kRandom_name,	_T("Channel_Name"),
							TYPE_STRING,
							P_RESET_DEFAULT,
							IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_NAME,
			end,

		// data type
			kRandom_type,	_T("Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CHANNELTYPE,
			// optional tagged param specs
					p_default,		kRandom_type_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kRandom_type_num,
												IDS_CUSTOMTYPE_FLOAT,		
												IDS_CUSTOMTYPE_INTEGER,		
												IDS_CUSTOMTYPE_VECTOR,
					p_vals,			kRandom_type_float,
									kRandom_type_integer,
									kRandom_type_vector,
			end,

		// scope type: global or local
			kRandom_scope,		_T("Scope"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPE,
			// optional tagged param specs
					p_default,		kRandom_scope_local,
					p_range,		0, kRandom_scope_num-1,
					p_ui,			TYPE_RADIO, kRandom_scope_num, IDC_GLOBAL, IDC_LOCAL,
			end,

		// integer distribution type
			kRandom_integerType,	_T("Integer_Distribution_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_INTEGERDISTRIBUTIONTYPE,
			// optional tagged param specs
					p_default,		kRandom_integerType_uniform,
					p_ui,			TYPE_INTLISTBOX,	IDC_INTEGERTYPE,	kRandom_integerType_num,
												IDS_DISTRIBUTIONTYPE_UNIFORMDISCREET,		
					p_vals,			kRandom_integerType_uniform,
			end,

		// float distribution type
			kRandom_floatType,	_T("Float_Distribution_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_FLOATDISTRIBUTIONTYPE,
			// optional tagged param specs
					p_default,		kRandom_floatType_uniform,
					p_ui,			TYPE_INTLISTBOX,	IDC_FLOATTYPE,	kRandom_floatType_num,
												IDS_DISTRIBUTIONTYPE_EXPONENTIAL,
												IDS_DISTRIBUTIONTYPE_NORMAL,
												IDS_DISTRIBUTIONTYPE_TRIANGULAR,
												IDS_DISTRIBUTIONTYPE_UNIFORM,		
												IDS_DISTRIBUTIONTYPE_WEIBULL,
					p_vals,			kRandom_floatType_exponential,
									kRandom_floatType_normal,
									kRandom_floatType_triangular,
									kRandom_floatType_uniform,
									kRandom_floatType_weibull,
			end,

		// vector distribution type
			kRandom_vectorType,	_T("Vector_Distribution_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VECTORDISTRIBUTIONTYPE,
			// optional tagged param specs
					p_default,		kRandom_vectorType_uniform,
					p_ui,			TYPE_INTLISTBOX,	IDC_VECTORTYPE,	kRandom_vectorType_num,
												IDS_DISTRIBUTIONTYPE_NORMAL,
												IDS_DISTRIBUTIONTYPE_SPHERESURFACE,
												IDS_DISTRIBUTIONTYPE_SPHEREVOLUME,
												IDS_DISTRIBUTIONTYPE_UNIFORM,		
					p_vals,			kRandom_vectorType_normal,
									kRandom_vectorType_sphereSurface,
									kRandom_vectorType_sphereVolume,
									kRandom_vectorType_uniform,
			end,

		// param #1
			kRandom_param1,			_T("Parameter 1"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PARAMETER1,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PARAM1, IDC_PARAM1SPIN, SPIN_AUTOSCALE,
			end,

		// param #2
			kRandom_param2,			_T("Parameter 2"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PARAMETER2,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PARAM2, IDC_PARAM2SPIN, SPIN_AUTOSCALE,
			end,

		// param #3
			kRandom_param3,			_T("Parameter 3"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PARAMETER3,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_PARAM3, IDC_PARAM3SPIN, SPIN_AUTOSCALE,
			end,

		// positive param #1
			kRandom_posParam1,		_T("Positive Parameter 1"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_POSITIVEPARAMETER1,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_POSPARAM1, IDC_POSPARAM1SPIN, SPIN_AUTOSCALE,
			end,

		// positive param #2
			kRandom_posParam2,		_T("Positive Parameter 2"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_POSITIVEPARAMETER2,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.0f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_POSPARAM2, IDC_POSPARAM2SPIN, SPIN_AUTOSCALE,
			end,

		// positive param #3
			kRandom_posParam3,		_T("Positive Parameter 3"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_POSITIVEPARAMETER3,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.0f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_POSPARAM3, IDC_POSPARAM3SPIN, SPIN_AUTOSCALE,
			end,

		// integer param #1
			kRandom_intParam1,		_T("Integer Parameter 1"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERPARAMETER1,
			// optional tagged param specs
					p_default,		1,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_INTPARAM1, IDC_INTPARAM1SPIN, 1.0f,
			end,

		// integer param #2
			kRandom_intParam2,		_T("Integer Parameter 2"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERPARAMETER2,
			// optional tagged param specs
					p_default,		1,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_INTPARAM2, IDC_INTPARAM2SPIN, 1.0f,
			end,

		// integer param #3
			kRandom_intParam3,		_T("Integer Parameter 3"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_INTEGERPARAMETER3,
			// optional tagged param specs
					p_default,		10,
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_INT,
									IDC_INTPARAM3, IDC_INTPARAM3SPIN, 1.0f,
			end,

		// assign only once (to new particles only)
			kRandom_assignOnce, _T("Assign_Only_Once"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNONLYONCE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNONCE,
			end,

		// use condition filter
			kRandom_useCondition, _T("Use_Condition_Filter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECONDITIONFILTER,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECONDITION,
			end,

		// condition channel name
			kRandom_conditionName,	_T("Condition_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CONDITIONCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CONDITIONNAME,
			end,

		// random seed
			kRandom_seed,			_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT,
									IDC_SEED, IDC_SEEDSPIN, 1.0f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc random_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorRandomDesc, FP_MIXIN, 
				
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

FPInterfaceDesc random_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorRandomDesc, FP_MIXIN,

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

FPInterfaceDesc random_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorRandomDesc, FP_MIXIN,

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

INT_PTR PAOperatorRandomDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
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
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kRandom_RefMsg_NewRand);
					break;
				case kRandom_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PAOperatorRandomDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
//	BOOL bAxes, bDivergence, bRandom;
//	bool varAnimated, divAnimated;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int type = ppb->GetInt(kRandom_type, 0);
			bool useFilter = (ppb->GetInt(kRandom_useCondition, 0) != 0);
			int floatType = ppb->GetInt(kRandom_floatType, 0);
			int vectorType = ppb->GetInt(kRandom_vectorType, 0);

			bool showControl = (type == kRandom_type_integer);
			EnableWindow( GetDlgItem(hWnd, IDC_INTEGERTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_INTEGERTYPE), showControl ? SW_SHOW : SW_HIDE);

			showControl = (type == kRandom_type_float);
			EnableWindow( GetDlgItem(hWnd, IDC_FLOATTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_FLOATTYPE), showControl ? SW_SHOW : SW_HIDE);

			showControl = (type == kRandom_type_vector);
			EnableWindow( GetDlgItem(hWnd, IDC_VECTORTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_VECTORTYPE), showControl ? SW_SHOW : SW_HIDE);

			bool enable[3],positiveType[3];
			TSTR name[3];
			for(int i=0; i<3; i++) {
				enable[i] = false;
				positiveType[i] = false;
			}
			name[0] = GetString(IDS_PARAMETER1);
			name[1] = GetString(IDS_PARAMETER2);
			name[2] = GetString(IDS_PARAMETER3);

			switch(type) {
			case kRandom_type_integer:
				enable[0] = enable[2] = true;
				name[0] = GetString(IDS_MINIMUM);
				name[2] = GetString(IDS_MAXIMUM);
				break;

			case kRandom_type_float:
				switch(floatType) {
				case kRandom_floatType_triangular:
					enable[1] = true;
					name[1] = GetString(IDS_PEAK);
					// the break is omitted intentionally
				case kRandom_floatType_uniform:
					enable[0] = enable[2] = true;
					name[0] = GetString(IDS_MINIMUM);
					name[2] = GetString(IDS_MAXIMUM);
					break;
				case kRandom_floatType_normal:
					enable[0] = enable[1] = true;
					name[0] = GetString(IDS_MEAN);
					name[1] = GetString(IDS_VARIANCE);
					positiveType[1] = true;
					break;
				case kRandom_floatType_exponential:
					enable[0] = true;
					name[0] = GetString(IDS_MEAN);
					positiveType[0] = true;
					break;
				case kRandom_floatType_weibull:
					enable[0] = enable[1] = enable[2] = true;
					name[0] = GetString(IDS_ALPHA);
					name[1] = GetString(IDS_BETA);
					name[2] = GetString(IDS_SCALE);
					positiveType[0] = true;
					positiveType[1] = true;
					break;
				}
				break;

			case kRandom_type_vector:
				switch(vectorType) {
				case kRandom_vectorType_uniform:
					enable[0] = enable[2] = true;
					name[0] = GetString(IDS_MINIMUM);
					name[2] = GetString(IDS_MAXIMUM);
					break;
				case kRandom_vectorType_sphereSurface:
				case kRandom_vectorType_sphereVolume:
					enable[1] = true;
					name[1] = GetString(IDS_RADIUS);
					positiveType[1] = true;
					break;
				case kRandom_vectorType_normal:
					enable[1] = true;
					name[1] = GetString(IDS_VARIANCE);
					positiveType[1] = true;
					break;
				}
				break;
			}

			SetDlgItemText( hWnd, IDC_TEXT_PARAM1, name[0] );
			SetDlgItemText( hWnd, IDC_TEXT_PARAM2, name[1] );
			SetDlgItemText( hWnd, IDC_TEXT_PARAM3, name[2] );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_PARAM1), enable[0] );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_PARAM2), enable[1] );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_PARAM3), enable[2] );
			
			ISpinnerControl* spin = NULL;

			showControl = (type != kRandom_type_integer) && (!positiveType[0]);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM1), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM1SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PARAM1SPIN) );
			spin->Enable(showControl ? enable[0] : false);
			ReleaseISpinner(spin);
			
			showControl = (type != kRandom_type_integer) && (!positiveType[1]);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM2), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM2SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PARAM2SPIN) );
			spin->Enable(showControl ? enable[1] : false);
			ReleaseISpinner(spin);

			showControl = (type != kRandom_type_integer) && (!positiveType[2]);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM3), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_PARAM3SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_PARAM3SPIN) );
			spin->Enable(showControl ? enable[2] : false);
			ReleaseISpinner(spin);

			showControl = (type != kRandom_type_integer) && (positiveType[0]);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM1), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM1SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_POSPARAM1SPIN) );
			spin->Enable(showControl ? enable[0] : false);
			ReleaseISpinner(spin);

			showControl = (type != kRandom_type_integer) && (positiveType[1]);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM2), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM2SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_POSPARAM2SPIN) );
			spin->Enable(showControl ? enable[1] : false);
			ReleaseISpinner(spin);

			showControl = (type != kRandom_type_integer) && (positiveType[2]);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM3), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_POSPARAM3SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_POSPARAM3SPIN) );
			spin->Enable(showControl ? enable[2] : false);
			ReleaseISpinner(spin);

			showControl = (type == kRandom_type_integer);
			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM1), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM1SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_INTPARAM1SPIN) );
			spin->Enable(showControl ? enable[0] : false);
			ReleaseISpinner(spin);

			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM2), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM2SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_INTPARAM2SPIN) );
			spin->Enable(showControl ? enable[1] : false);
			ReleaseISpinner(spin);

			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM3), showControl ? SW_SHOW : SW_HIDE);
			ShowWindow( GetDlgItem( hWnd, IDC_INTPARAM3SPIN), showControl ? SW_SHOW : SW_HIDE);
			spin = GetISpinner( GetDlgItem( hWnd, IDC_INTPARAM3SPIN) );
			spin->Enable(showControl ? enable[2] : false);
			ReleaseISpinner(spin);

			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CONDITIONNAME), useFilter );
			EnableWindow( GetDlgItem(hWnd, IDC_CONDITIONNAME), useFilter );
		}
	}
}




} // end of namespace PartASM