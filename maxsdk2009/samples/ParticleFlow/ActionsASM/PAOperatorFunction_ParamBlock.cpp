/**********************************************************************
 *<
	FILE:			PAOperatorFunction_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "PAOperatorFunction_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

// Each plug-in should have one instance of the ClassDesc class
PAOperatorFunctionDesc ThePAOperatorFunctionDesc;

// Dialog Proc
PAOperatorFunctionDlgProc ThePAOperatorFunctionDlgProc;

// Value Operator param block
ParamBlockDesc2 function_paramBlockDesc 
(
	// required block spec
		kFunction_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorFunctionDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kFunction_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_FUNCTION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorFunctionDlgProc,
	// required param specs
		// operand A channel name
			kFunction_opAName,	_T("Operand_A_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_OPERANDACHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_ANAME,
			end,

		// operand A data type
			kFunction_opAType,	_T("Operand_A_Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OPERANDACHANNELTYPE,
			// optional tagged param specs
					p_default,		kFunction_opAType_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_OPATYPE,		kFunction_opAType_num,
												IDS_CUSTOMTYPE_BOOLEAN,		IDS_CUSTOMTYPE_FLOAT,		
												IDS_CUSTOMTYPE_INTEGER,		IDS_CUSTOMTYPE_TIME,
												IDS_CUSTOMTYPE_VECTOR,		
					p_vals,			kFunction_opAType_boolean,	kFunction_opAType_float,			
									kFunction_opAType_integer,	kFunction_opAType_time,	
									kFunction_opAType_vector,	
			end,

		// operand A pre-factor
			kFunction_opAFactor,	_T("Operand_A_PreFactor"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AFACTOR, IDC_AFACTORSPIN, SPIN_AUTOSCALE,
			end,

		// use second operand
			kFunction_useOpB,		_T("Use_Second_Operand"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESECONDOPERAND,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESECONDOPERAND,
			end,

		// operand B channel name
			kFunction_opBName,	_T("Operand_B_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_OPERANDBCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_BNAME,
			end,

		// operand B data type
			kFunction_opBType,	_T("Operand_B_Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OPERANDBCHANNELTYPE,
			// optional tagged param specs
					p_default,		kFunction_opBType_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_OPBTYPE,		kFunction_opBType_num,
												IDS_CUSTOMTYPE_FLOAT,		IDS_CUSTOMTYPE_INTEGER,
												IDS_CUSTOMTYPE_VECTOR,		
					p_vals,			kFunction_opBType_float,			kFunction_opBType_integer,
									kFunction_opBType_vector,	
			end,

		// operand B pre-factor
			kFunction_opBFactor,	_T("Operand_B_PreFactor"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_BFACTOR, IDC_BFACTORSPIN, SPIN_AUTOSCALE,
			end,

		// boolean-boolean function type
			kFunction_bbType,		_T("BooleanBoolean_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_BOOLEANBOOLEANFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_bbType_and,
					p_ui,			TYPE_INTLISTBOX,	IDC_BBTYPE,		kFunction_bbType_num,
												IDS_FUNCTIONTYPE_AND,	IDS_FUNCTIONTYPE_OR,
												IDS_FUNCTIONTYPE_XOR,		
					p_vals,			kFunction_bbType_and,		kFunction_bbType_or,
									kFunction_bbType_xor,	
			end,

		// time-time function type
			kFunction_ttType,		_T("TimeTime_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TIMETIMEFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_ttType_add,
					p_ui,			TYPE_INTLISTBOX,	IDC_TTTYPE,		kFunction_ttType_num,
												IDS_FUNCTIONTYPE_ADDITION,	IDS_FUNCTIONTYPE_SUBTRACTION,
					p_vals,			kFunction_ttType_add,		kFunction_ttType_subtract,
			end,
			
		// float function type
			kFunction_fType,		_T("Float_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_FLOATFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_fType_abs,
					p_ui,			TYPE_INTLISTBOX,	IDC_FTYPE,		kFunction_fType_num,
												IDS_FUNCTIONTYPE_ABSOLUTE,			IDS_FUNCTIONTYPE_BINARYEXPONENT,
												IDS_FUNCTIONTYPE_BINARYLOGARITHM,	IDS_FUNCTIONTYPE_COSINE,
												IDS_FUNCTIONTYPE_COTANGENT,			IDS_FUNCTIONTYPE_CUBE,
												IDS_FUNCTIONTYPE_CUBICROOT,			IDS_FUNCTIONTYPE_DECIMALEXPONENT,
												IDS_FUNCTIONTYPE_DECIMALLOGARITHM,	IDS_IDENTITY,
												IDS_FUNCTIONTYPE_INVERSE,
												IDS_FUNCTIONTYPE_NATURALEXPONENT,	IDS_FUNCTIONTYPE_NATURALLOGARITHM,
												IDS_FUNCTIONTYPE_SINE,				IDS_FUNCTIONTYPE_SQUARE,
												IDS_FUNCTIONTYPE_SQUAREROOT,		IDS_FUNCTIONTYPE_TANGENT,
					p_vals,			kFunction_fType_abs,		kFunction_fType_bExp,
									kFunction_fType_bLog,		kFunction_fType_cos,
									kFunction_fType_ctg,		kFunction_fType_cube,
									kFunction_fType_cubicRoot,	kFunction_fType_dExp,
									kFunction_fType_dLog,		kFunction_fType_identity,
									kFunction_fType_inverse,
									kFunction_fType_nExp,		kFunction_fType_nLog,
									kFunction_fType_sin,		kFunction_fType_square,
									kFunction_fType_squareRoot,	kFunction_fType_tg,
			end,

		// float-float function type
			kFunction_ffType,		_T("FloatFloat_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_FLOATFLOATFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_ffType_add,
					p_ui,			TYPE_INTLISTBOX,	IDC_FFTYPE,		kFunction_ffType_num,
												IDS_FUNCTIONTYPE_ADDITION,			IDS_FUNCTIONTYPE_DIVISION,
												IDS_FUNCTIONTYPE_DIVISIONREMAINDER,	IDS_FUNCTIONTYPE_LOGARITHM,
												IDS_FUNCTIONTYPE_MAXIMUM,			IDS_FUNCTIONTYPE_MINIMUM,
												IDS_FUNCTIONTYPE_MULTIPLICATION,	IDS_FUNCTIONTYPE_POWER,
												IDS_FUNCTIONTYPE_SUBTRACTION,
					p_vals,			kFunction_ffType_add,		kFunction_ffType_divide,
									kFunction_ffType_mod,		kFunction_ffType_log,
									kFunction_ffType_max,		kFunction_ffType_min,
									kFunction_ffType_multiply,	kFunction_ffType_power,
									kFunction_ffType_subtract,
			end,

		// vector function type
			kFunction_vType,		_T("Vector_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VECTORFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_vType_length,
					p_ui,			TYPE_INTLISTBOX,	IDC_VTYPE,		kFunction_vType_num,
												IDS_IDENTITY, IDS_FUNCTIONTYPE_LENGTH,	IDS_FUNCTIONTYPE_NORMALIZE,
					p_vals,			kFunction_vType_identity, kFunction_vType_length, kFunction_vType_norm,
			end,

		// vector-float function type
			kFunction_vfType,		_T("VectorFloat_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VECTORFLOATFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_vfType_multiply,
					p_ui,			TYPE_INTLISTBOX,	IDC_VFTYPE,		kFunction_vfType_num,
												IDS_FUNCTIONTYPE_DEVIATE,		IDS_FUNCTIONTYPE_DIVISION,
												IDS_FUNCTIONTYPE_MULTIPLICATION,	
					p_vals,			kFunction_vfType_deviate,		kFunction_vfType_divide,
									kFunction_vfType_multiply,
			end,

		// vector-vector function type
			kFunction_vvType,		_T("VectorVector_Function_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VECTORVECTORFUNCTIONTYPE,
			// optional tagged param specs
					p_default,		kFunction_vvType_add,
					p_ui,			TYPE_INTLISTBOX,	IDC_VVTYPE,		kFunction_vvType_num,
												IDS_FUNCTIONTYPE_ADDITION,		
												IDS_FUNCTIONTYPE_ADDROTATION,
												IDS_FUNCTIONTYPE_DISTANCE,	
												IDS_FUNCTIONTYPE_NORMALCONFORM,
												IDS_FUNCTIONTYPE_ROTATEBY,
												IDS_FUNCTIONTYPE_SCALARPRODUCT,
												IDS_FUNCTIONTYPE_SCALEPRODUCT,
												IDS_FUNCTIONTYPE_SUBTRACTION,
												IDS_FUNCTIONTYPE_VECTORPRODUCT,
					p_vals,			kFunction_vvType_add,
									kFunction_vvType_addRotation,
									kFunction_vvType_distance,
									kFunction_vvType_conform,
									kFunction_vvType_rotateBy,
									kFunction_vvType_dot,
									kFunction_vvType_scaleProd,
									kFunction_vvType_subtract,
									kFunction_vvType_cross,
			end,

		// result channel name
			kFunction_resName,	_T("Result_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_RESULTCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_RNAME,
			end,

		// result post-factor
			kFunction_resFactor,	_T("Result_PostFactor"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_FLOATVALUE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_RFACTOR, IDC_RFACTORSPIN, SPIN_AUTOSCALE,
			end,

		// scope type: global or local
			kFunction_scope,		_T("Scope"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPE,
			// optional tagged param specs
					p_default,		kFunction_scope_local,
					p_range,		0, kFunction_scope_num-1,
					p_ui,			TYPE_RADIO, kFunction_scope_num, IDC_GLOBAL, IDC_LOCAL,
			end,

		// assign only once (to new particles only)
			kFunction_assignOnce, _T("Assign_Only_Once"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNONLYONCE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNONCE,
			end,

		// use condition filter
			kFunction_useCondition, _T("Use_Condition_Filter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECONDITIONFILTER,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECONDITION,
			end,

		// condition channel name
			kFunction_conditionName,	_T("Condition_Channel_Name"),
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
FPInterfaceDesc function_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorFunctionDesc, FP_MIXIN, 
				
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

FPInterfaceDesc function_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorFunctionDesc, FP_MIXIN,

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

FPInterfaceDesc function_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorFunctionDesc, FP_MIXIN,

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

INT_PTR PAOperatorFunctionDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
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
				case kFunction_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PAOperatorFunctionDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int opAType = ppb->GetInt(kFunction_opAType, 0);
			bool useOpB = (ppb->GetInt(kFunction_useOpB, 0) != 0);
			int opBType = ppb->GetInt(kFunction_opBType, 0);
			int bbType = ppb->GetInt(kFunction_bbType, 0);
			int ttType = ppb->GetInt(kFunction_ttType, 0);
			int fType = ppb->GetInt(kFunction_fType, 0);
			int ffType = ppb->GetInt(kFunction_ffType, 0);
			int vType = ppb->GetInt(kFunction_vType, 0);
			int vfType = ppb->GetInt(kFunction_vfType, 0);
			int vvType = ppb->GetInt(kFunction_vvType, 0);
			bool useFilter = (ppb->GetInt(kFunction_useCondition, 0) != 0);
			ISpinnerControl* spin = NULL;

			bool useAFactor = (opAType != kFunction_opAType_boolean);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_AFACTOR), useAFactor );
			spin = GetISpinner( GetDlgItem( hWnd, IDC_AFACTORSPIN) );
			spin->Enable(useAFactor);
			ReleaseISpinner(spin);
						
			if (opAType == kFunction_opAType_time) {
				useOpB = true;
				EnableWindow( GetDlgItem(hWnd, IDC_USESECONDOPERAND), false );
			} else {
				EnableWindow( GetDlgItem(hWnd, IDC_USESECONDOPERAND), true );
			}

			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_B), useOpB );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BNAME), useOpB );
			EnableWindow( GetDlgItem(hWnd, IDC_BNAME), useOpB );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BTYPE), useOpB );
			bool showControl = (opAType != kFunction_opAType_boolean) && (opAType != kFunction_opAType_time);
			EnableWindow( GetDlgItem(hWnd, IDC_OPBTYPE), showControl && useOpB);
			ShowWindow( GetDlgItem( hWnd, IDC_OPBTYPE), showControl ? SW_SHOW : SW_HIDE);
			TSTR opBTypeText = GetString(IDS_CUSTOMTYPE_BOOLEAN);
			if (opAType == kFunction_opAType_time)
				opBTypeText = GetString(IDS_CUSTOMTYPE_TIME);
			SetDlgItemText( hWnd, IDC_TEXT_BOOLEAN_OR_TIME, opBTypeText );
			ShowWindow( GetDlgItem( hWnd, IDC_TEXT_BOOLEAN_OR_TIME), showControl ? SW_HIDE : SW_SHOW);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BOOLEAN_OR_TIME), useOpB );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BFACTOR), useAFactor && useOpB );
			spin = GetISpinner( GetDlgItem( hWnd, IDC_BFACTORSPIN) );
			spin->Enable(useAFactor && useOpB);
			ReleaseISpinner(spin);

			showControl = ((opAType == kFunction_opAType_boolean) && (!useOpB));
			TSTR functionText = GetString(IDS_FUNCTIONTYPE_NOT);
			SetDlgItemText( hWnd, IDC_BTYPE, functionText );
			ShowWindow( GetDlgItem( hWnd, IDC_BTYPE), showControl ? SW_SHOW : SW_HIDE);
			TSTR resultText = GetString(IDS_CUSTOMTYPE_BOOLEAN);
			if (!showControl && useOpB) {
				showControl = (((opAType == kFunction_opAType_float) || (opAType == kFunction_opAType_integer)) && (opBType == kFunction_opBType_vector));
				if (showControl) {
					functionText = GetString(IDS_FUNCTIONTYPE_MULTIPLICATION);
					SetDlgItemText( hWnd, IDC_BTYPE, functionText );
					ShowWindow( GetDlgItem( hWnd, IDC_BTYPE), showControl ? SW_SHOW : SW_HIDE);
					resultText = GetString(IDS_CUSTOMTYPE_VECTOR);
				}
			}

			showControl = ((opAType == kFunction_opAType_boolean) && useOpB);
			EnableWindow( GetDlgItem(hWnd, IDC_BBTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_BBTYPE), showControl ? SW_SHOW : SW_HIDE);

			showControl = (opAType == kFunction_opAType_time);
			EnableWindow( GetDlgItem(hWnd, IDC_TTTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_TTTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) resultText = GetString(IDS_CUSTOMTYPE_TIME);

			showControl = (((opAType == kFunction_opAType_float) || (opAType == kFunction_opAType_integer)) && (!useOpB));
			EnableWindow( GetDlgItem(hWnd, IDC_FTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_FTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) resultText = GetString(IDS_CUSTOMTYPE_FLOAT);

			showControl = (useOpB && ((opAType == kFunction_opAType_float) || (opAType == kFunction_opAType_integer)) && ((opBType == kFunction_opBType_float) || (opBType == kFunction_opBType_integer)));
			EnableWindow( GetDlgItem(hWnd, IDC_FFTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_FFTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) resultText = GetString(IDS_CUSTOMTYPE_FLOAT);

			showControl = ((opAType == kFunction_opAType_vector) && (!useOpB));
			EnableWindow( GetDlgItem(hWnd, IDC_VTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_VTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) {
				resultText = GetString(IDS_CUSTOMTYPE_FLOAT);
				if (vType == kFunction_vType_norm)
					resultText = GetString(IDS_CUSTOMTYPE_VECTOR);
			}

			showControl = (useOpB && (opAType == kFunction_opAType_vector) && ((opBType == kFunction_opBType_float) || (opBType == kFunction_opBType_integer)));
			EnableWindow( GetDlgItem(hWnd, IDC_VFTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_VFTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) resultText = GetString(IDS_CUSTOMTYPE_VECTOR);

			showControl = (useOpB && (opAType == kFunction_opAType_vector) && (opBType == kFunction_opBType_vector));
			EnableWindow( GetDlgItem(hWnd, IDC_VVTYPE), showControl );
			ShowWindow( GetDlgItem( hWnd, IDC_VVTYPE), showControl ? SW_SHOW : SW_HIDE);
			if (showControl) {
				resultText = GetString(IDS_CUSTOMTYPE_VECTOR);
				switch(vvType) {
				case kFunction_vvType_distance:
				case kFunction_vvType_dot:
					resultText = GetString(IDS_CUSTOMTYPE_FLOAT);
					break;
				}
			}

			SetDlgItemText( hWnd, IDC_RTYPE, resultText );

			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_RFACTOR), useAFactor );
			spin = GetISpinner( GetDlgItem( hWnd, IDC_RFACTORSPIN) );
			spin->Enable(useAFactor);
			ReleaseISpinner(spin);

			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CONDITIONNAME), useFilter );
			EnableWindow( GetDlgItem(hWnd, IDC_CONDITIONNAME), useFilter );
		}
	}
}




} // end of namespace PartASM