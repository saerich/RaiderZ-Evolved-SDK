/**********************************************************************
 *<
	FILE:			PAOperatorCopy_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "PAOperatorCopy_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

// Each plug-in should have one instance of the ClassDesc class
PAOperatorCopyDesc ThePAOperatorCopyDesc;

// Dialog Proc
PAOperatorCopyDlgProc ThePAOperatorCopyDlgProc;

// Value Operator param block
ParamBlockDesc2 copy_paramBlockDesc 
(
	// required block spec
		kCopy_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorCopyDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kCopy_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_COPY_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorCopyDlgProc,
	// required param specs
		// source data type
			kCopy_srcChannel,	_T("Source_Data_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SRCDATATYPE,
			// optional tagged param specs
					p_default,		kCopy_data_speed,
					p_ui,			TYPE_INTLISTBOX,	IDC_SRCDATATYPE,	kCopy_data_num,
												IDS_DATATYPE_AGE,			IDS_DATATYPE_BIRTHTIME,
												IDS_DATATYPE_CUSTOM,		IDS_DATATYPE_DEATHTIME,
												IDS_DATATYPE_LIFESPAN,		IDS_DATATYPE_MAPCHANNEL1,	
												IDS_DATATYPE_MAPCHANNEL2,	IDS_DATATYPE_MAPCHANNEL3,	
												IDS_DATATYPE_MAPCHANNEL4,	IDS_DATATYPE_MAPCHANNEL5,	
												IDS_DATATYPE_MTLINDEX,		IDS_DATATYPE_PARTICLEID,
												IDS_DATATYPE_POSITION,		
												IDS_DATATYPE_ROTATIONANGLE, IDS_DATATYPE_ROTATIONAXIS,	
												IDS_DATATYPE_EUCLIDEROTATION,IDS_DATATYPE_SCALE,			
												IDS_DATATYPE_SCRIPTFLOAT,	IDS_DATATYPE_SCRIPTINTEGER,
												IDS_DATATYPE_SCRIPTMATRIXX,	IDS_DATATYPE_SCRIPTMATRIXY,
												IDS_DATATYPE_SCRIPTMATRIXZ,	IDS_DATATYPE_SCRIPTMATRIXT,
												IDS_DATATYPE_SCRIPTVECTOR,	IDS_DATATYPE_SELECTED,
												IDS_DATATYPE_SIZE,			IDS_DATATYPE_SHAPEFACES,
												IDS_DATATYPE_SHAPESURFACE,	IDS_DATATYPE_SHAPEVERTICES,
												IDS_DATATYPE_SHAPEVOLUME,	IDS_DATATYPE_SPEED,	
												IDS_DATATYPE_SPINAXIS,		IDS_DATATYPE_SPINRATE,		
												IDS_DATATYPE_TIME,			IDS_DATATYPE_TIMEINEVENT,
					p_vals,			kCopy_data_age, 			kCopy_data_birthTime,
									kCopy_data_custom,			kCopy_data_deathTime,
									kCopy_data_lifespan,		kCopy_data_mapChannel1,	
									kCopy_data_mapChannel2,		kCopy_data_mapChannel3,	
									kCopy_data_mapChannel4,		kCopy_data_mapChannel5,	
									kCopy_data_mtlIndex,		kCopy_data_particleID,
									kCopy_data_position,		
									kCopy_data_rotationAngle,	kCopy_data_rotationAxis,	
									kCopy_data_euclideRotation,	kCopy_data_scale,
									kCopy_data_scriptFloat,		kCopy_data_scriptInteger,
									kCopy_data_scriptMatrixX,	kCopy_data_scriptMatrixY,
									kCopy_data_scriptMatrixZ,	kCopy_data_scriptMatrixT,
									kCopy_data_scriptVector,	kCopy_data_selected,
									kCopy_data_size,			kCopy_data_shapeFaces,		
									kCopy_data_shapeSurface,	kCopy_data_shapeVertices,
									kCopy_data_shapeVolume,		kCopy_data_speed,		
									kCopy_data_spinAxis,		kCopy_data_spinRate,		
									kCopy_data_localTime,		kCopy_data_timeInEvent,
			end,

		// channel name
			kCopy_srcName,		_T("Source_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_SRCCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_SRCCHANNELNAME,
			end,

		// data type
			kCopy_srcType,		_T("Source_Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SRCCHANNELTYPE,
			// optional tagged param specs
					p_default,		kCopy_type_worldVector,
					p_ui,			TYPE_INTLISTBOX,	IDC_SRCCHANNELTYPE,	kCopy_type_num,
												IDS_CUSTOMTYPE_ANGLE,			IDS_CUSTOMTYPE_BOOLEAN,
												IDS_CUSTOMTYPE_EUCLIDEROTATION,	IDS_CUSTOMTYPE_FLOAT,
												IDS_CUSTOMTYPE_FLOATVECTOR,		IDS_CUSTOMTYPE_INTEGER,
												IDS_CUSTOMTYPE_PERCENT,			IDS_CUSTOMTYPE_PERCENTVECTOR,
												IDS_CUSTOMTYPE_TIME,			IDS_CUSTOMTYPE_WORLDUNIT,
												IDS_CUSTOMTYPE_WORLDVECTOR,
					p_vals,			kCopy_type_angle,			kCopy_type_boolean,
									kCopy_type_angleVector,		kCopy_type_float,			
									kCopy_type_floatVector,		kCopy_type_integer,			
									kCopy_type_percent,			kCopy_type_percentVector,	
									kCopy_type_time,			kCopy_type_worldFloat,		
									kCopy_type_worldVector,
			end,

		// destination data type
			kCopy_dstChannel,	_T("Destination_Data_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DSTDATATYPE,
			// optional tagged param specs
					p_default,		kCopy_data_custom,
					p_ui,			TYPE_INTLISTBOX,	IDC_DSTDATATYPE,	kCopy_data_num - 8,
												IDS_DATATYPE_AGE,			
												IDS_DATATYPE_CUSTOM,		IDS_DATATYPE_DEATHTIME,
												IDS_DATATYPE_LIFESPAN,		IDS_DATATYPE_MAPCHANNEL1,	
												IDS_DATATYPE_MAPCHANNEL2,	IDS_DATATYPE_MAPCHANNEL3,	
												IDS_DATATYPE_MAPCHANNEL4,	IDS_DATATYPE_MAPCHANNEL5,	
												IDS_DATATYPE_MTLINDEX,		IDS_DATATYPE_POSITION,		
												IDS_DATATYPE_ROTATIONANGLE, IDS_DATATYPE_ROTATIONAXIS,	
												IDS_DATATYPE_EUCLIDEROTATION,IDS_DATATYPE_SCALE,			
												IDS_DATATYPE_SCRIPTFLOAT,	IDS_DATATYPE_SCRIPTINTEGER,
												IDS_DATATYPE_SCRIPTMATRIXX,	IDS_DATATYPE_SCRIPTMATRIXY,
												IDS_DATATYPE_SCRIPTMATRIXZ,	IDS_DATATYPE_SCRIPTMATRIXT,
												IDS_DATATYPE_SCRIPTVECTOR,	IDS_DATATYPE_SELECTED,
												IDS_DATATYPE_SPEED,			IDS_DATATYPE_SPINAXIS,		
												IDS_DATATYPE_SPINRATE,		IDS_DATATYPE_TIMEINEVENT,
					p_vals,			kCopy_data_age, 			
									kCopy_data_custom,			kCopy_data_deathTime,
									kCopy_data_lifespan,		kCopy_data_mapChannel1,	
									kCopy_data_mapChannel2,		kCopy_data_mapChannel3,	
									kCopy_data_mapChannel4,		kCopy_data_mapChannel5,	
									kCopy_data_mtlIndex,		kCopy_data_position,		
									kCopy_data_rotationAngle,	kCopy_data_rotationAxis,	
									kCopy_data_euclideRotation,	kCopy_data_scale,
									kCopy_data_scriptFloat,		kCopy_data_scriptInteger,
									kCopy_data_scriptMatrixX,	kCopy_data_scriptMatrixY,
									kCopy_data_scriptMatrixZ,	kCopy_data_scriptMatrixT,
									kCopy_data_scriptVector,	kCopy_data_selected,
									kCopy_data_speed,			kCopy_data_spinAxis,		
									kCopy_data_spinRate,		kCopy_data_timeInEvent,
			end,

		// channel name
			kCopy_dstName,		_T("Destination_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_DSTCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Current Speed"),
					p_ui,			TYPE_EDITBOX, IDC_DSTCHANNELNAME,
			end,

		// data type
			kCopy_dstType,		_T("Destination_Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DSTCHANNELTYPE,
			// optional tagged param specs
					p_default,		kCopy_type_worldVector,
					p_ui,			TYPE_INTLISTBOX,	IDC_DSTCHANNELTYPE,	kCopy_type_num,
												IDS_CUSTOMTYPE_ANGLE,			IDS_CUSTOMTYPE_BOOLEAN,
												IDS_CUSTOMTYPE_EUCLIDEROTATION,	IDS_CUSTOMTYPE_FLOAT,
												IDS_CUSTOMTYPE_FLOATVECTOR,		IDS_CUSTOMTYPE_INTEGER,
												IDS_CUSTOMTYPE_PERCENT,			IDS_CUSTOMTYPE_PERCENTVECTOR,
												IDS_CUSTOMTYPE_TIME,			IDS_CUSTOMTYPE_WORLDUNIT,
												IDS_CUSTOMTYPE_WORLDVECTOR,
					p_vals,			kCopy_type_angle,			kCopy_type_boolean,
									kCopy_type_angleVector,		kCopy_type_float,			
									kCopy_type_floatVector,		kCopy_type_integer,			
									kCopy_type_percent,			kCopy_type_percentVector,	
									kCopy_type_time,			kCopy_type_worldFloat,		
									kCopy_type_worldVector,
			end,

		// scope type: global or local
			kCopy_dstScope,		_T("Scope"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPE,
			// optional tagged param specs
					p_default,		kCopy_scope_local,
					p_range,		0, kCopy_scope_num-1,
					p_ui,			TYPE_RADIO, kCopy_scope_num, IDC_GLOBAL, IDC_LOCAL,
			end,

		// assign only once (to new particles only)
			kCopy_assignOnce, _T("Assign_Only_Once"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNONLYONCE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNONCE,
			end,

		// use condition filter
			kCopy_useCondition, _T("Use_Condition_Filter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECONDITIONFILTER,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECONDITION,
			end,

		// condition channel name
			kCopy_conditionName,	_T("Condition_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CONDITIONCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CONDITIONNAME,
			end,

		// scalar/vector conversion
			kCopy_scalarToVectorType, _T("Scalar/Vector Conversion"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCALARVECTORCONVERSION,
			// optional tagged param specs
					p_default,		kCopy_svType_x,
					p_range,		0, kCopy_svType_num-1,
					p_ui,			TYPE_RADIO, kCopy_svType_num, IDC_X, IDC_Y, IDC_Z, IDC_XYZ, IDC_LENGTH, IDC_AVERAGE, IDC_MINIMUM, IDC_MAXIMUM,
			end,

		// float/integer conversion
			kCopy_floatToIntegerType, _T("Float/Integer Conversion"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_FLOATINTEGERCONVERSION,
			// optional tagged param specs
					p_default,		kCopy_fiType_round,
					p_range,		0, kCopy_fiType_num-1,
					p_ui,			TYPE_RADIO, kCopy_fiType_num, IDC_UP, IDC_DOWN, IDC_ROUND,
			end,

		// float/time conversion
			kCopy_floatToTimeType, _T("Float/Time Conversion"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_FLOATTIMECONVERSION,
			// optional tagged param specs
					p_default,		kCopy_ftType_tick,
					p_range,		0, kCopy_ftType_num-1,
					p_ui,			TYPE_RADIO, kCopy_ftType_num, IDC_SEC, IDC_FRAME, IDC_TICK,
			end,

		// float/percent conversion
			kCopy_floatToPercentType, _T("Float/Percent Conversion"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_FLOATPERCENTCONVERSION,
			// optional tagged param specs
					p_default,		kCopy_fpType_factor100,
					p_range,		0, kCopy_fpType_num-1,
					p_ui,			TYPE_RADIO, kCopy_fpType_num, IDC_100_100, IDC_1_100,
			end,

		// float/angle conversion
			kCopy_floatToAngleType, _T("Float/Angle Conversion"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_FLOATANGLECONVERSION,
			// optional tagged param specs
					p_default,		kCopy_faType_radians,
					p_range,		0, kCopy_faType_num-1,
					p_ui,			TYPE_RADIO, kCopy_faType_num, IDC_DEGREES, IDC_RADIANS,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc copy_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorCopyDesc, FP_MIXIN, 
				
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

FPInterfaceDesc copy_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorCopyDesc, FP_MIXIN,

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

FPInterfaceDesc copy_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorCopyDesc, FP_MIXIN,

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

INT_PTR PAOperatorCopyDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
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
				case kCopy_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PAOperatorCopyDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int srcData = ppb->GetInt(kCopy_srcChannel, 0);
			int srcType = ppb->GetInt(kCopy_srcType, 0);
			int dstData = ppb->GetInt(kCopy_dstChannel, 0);
			int dstType = ppb->GetInt(kCopy_dstType, 0);

			switch(srcData) {
			case kCopy_data_birthTime:
			case kCopy_data_age:
			case kCopy_data_deathTime:
			case kCopy_data_lifespan:
			case kCopy_data_timeInEvent:
				srcType = kCopy_type_time;
				break;
			case kCopy_data_position:
			case kCopy_data_speed:
			case kCopy_data_size:
				srcType = kCopy_type_worldVector;
				break;
			case kCopy_data_scale:
				srcType = kCopy_type_percentVector;
				break;
			case kCopy_data_rotationAxis:
			case kCopy_data_spinAxis:
			case kCopy_data_mapChannel1:
			case kCopy_data_mapChannel2:
			case kCopy_data_mapChannel3:
			case kCopy_data_mapChannel4:
			case kCopy_data_mapChannel5:
			case kCopy_data_scriptVector:
			case kCopy_data_scriptMatrixX:
			case kCopy_data_scriptMatrixY:
			case kCopy_data_scriptMatrixZ:
			case kCopy_data_scriptMatrixT:
				srcType = kCopy_type_floatVector;
				break;
			case kCopy_data_rotationAngle:
			case kCopy_data_spinRate:
				srcType = kCopy_type_angle;
				break;
			case kCopy_data_euclideRotation:
				srcType = kCopy_type_angleVector;
				break;
			case kCopy_data_mtlIndex:
			case kCopy_data_shapeVertices:
			case kCopy_data_shapeFaces:
			case kCopy_data_scriptInteger:
			case kCopy_data_particleID:
				srcType = kCopy_type_integer;
				break;
			case kCopy_data_selected:
				srcType = kCopy_type_boolean;
				break;
			case kCopy_data_shapeSurface:
			case kCopy_data_shapeVolume:
			case kCopy_data_scriptFloat:
				srcType = kCopy_type_float;
				break;
			}

			switch(dstData) {
			case kCopy_data_birthTime:
			case kCopy_data_age:
			case kCopy_data_deathTime:
			case kCopy_data_lifespan:
			case kCopy_data_timeInEvent:
				dstType = kCopy_type_time;
				break;
			case kCopy_data_position:
			case kCopy_data_speed:
			case kCopy_data_size:
				dstType = kCopy_type_worldVector;
				break;
			case kCopy_data_scale:
				dstType = kCopy_type_percentVector;
				break;
			case kCopy_data_rotationAxis:
			case kCopy_data_spinAxis:
			case kCopy_data_mapChannel1:
			case kCopy_data_mapChannel2:
			case kCopy_data_mapChannel3:
			case kCopy_data_mapChannel4:
			case kCopy_data_mapChannel5:
			case kCopy_data_scriptVector:
			case kCopy_data_scriptMatrixX:
			case kCopy_data_scriptMatrixY:
			case kCopy_data_scriptMatrixZ:
			case kCopy_data_scriptMatrixT:
				dstType = kCopy_type_floatVector;
				break;
			case kCopy_data_rotationAngle:
			case kCopy_data_spinRate:
				dstType = kCopy_type_angle;
				break;
			case kCopy_data_euclideRotation:
				dstType = kCopy_type_angleVector;
				break;
			case kCopy_data_mtlIndex:
			case kCopy_data_shapeVertices:
			case kCopy_data_shapeFaces:
			case kCopy_data_scriptInteger:
				dstType = kCopy_type_integer;
				break;
			case kCopy_data_selected:
				dstType = kCopy_type_boolean;
				break;
			case kCopy_data_shapeSurface:
			case kCopy_data_shapeVolume:
			case kCopy_data_scriptFloat:
				dstType = kCopy_type_float;
				break;
			}

			bool isSrcCustom = (srcData == kCopy_data_custom);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_SRCCUSTOMNAME), isSrcCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_SRCCHANNELNAME), isSrcCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_SRCCUSTOMTYPE), isSrcCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_SRCCHANNELTYPE), isSrcCustom );

			bool isDstCustom = (dstData == kCopy_data_custom);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_DSTCUSTOMNAME), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_DSTCHANNELNAME), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_DSTCUSTOMTYPE), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_DSTCHANNELTYPE), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_SCOPE), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_GLOBAL), isDstCustom );
			EnableWindow( GetDlgItem(hWnd, IDC_LOCAL), isDstCustom );

			bool useCondition = (ppb->GetInt(kCopy_useCondition, 0) != 0);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CONDITIONNAME), useCondition );
			EnableWindow( GetDlgItem(hWnd, IDC_CONDITIONNAME), useCondition );

			bool isScalarToVector = ((srcType < kCopy_type_floatVector) && (dstType > kCopy_type_angle));
			bool isVectorToScalar = ((srcType > kCopy_type_angle) && (dstType < kCopy_type_floatVector));
			bool hasScalarVectorConversion = (isScalarToVector || isVectorToScalar);
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_SCALARVECTOR), hasScalarVectorConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_X), hasScalarVectorConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_Y), hasScalarVectorConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_Z), hasScalarVectorConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_XYZ), isScalarToVector);
			EnableWindow( GetDlgItem(hWnd, IDC_LENGTH), isVectorToScalar);
			EnableWindow( GetDlgItem(hWnd, IDC_AVERAGE), isVectorToScalar);
			EnableWindow( GetDlgItem(hWnd, IDC_MINIMUM), isVectorToScalar);
			EnableWindow( GetDlgItem(hWnd, IDC_MAXIMUM), isVectorToScalar);
		
			bool isFromInteger = ((srcType == kCopy_type_integer) || (srcType == kCopy_type_boolean));
			bool isToInteger = ((dstType == kCopy_type_integer) || (dstType == kCopy_type_boolean));
			bool hasIntegerConversion = (!isFromInteger && isToInteger);
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_FLOATINTEGER), hasIntegerConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_UP), hasIntegerConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_DOWN), hasIntegerConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_ROUND), hasIntegerConversion);

			bool hasTimeConversion = (((srcType == kCopy_type_time) && (dstType != kCopy_type_time)) ||
									  ((srcType != kCopy_type_time) && (dstType == kCopy_type_time)));
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_FLOATTIME), hasTimeConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_SEC), hasTimeConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME), hasTimeConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_TICK), hasTimeConversion);
			
			bool srcUsesPercent = ((srcType == kCopy_type_percent) || (srcType == kCopy_type_percentVector));
			bool dstUsesPercent = ((dstType == kCopy_type_percent) || (dstType == kCopy_type_percentVector));
			bool hasPercentConversion = ((srcUsesPercent && !dstUsesPercent) || (!srcUsesPercent && dstUsesPercent));
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_FLOATPERCENT), hasPercentConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_100_100), hasPercentConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_1_100), hasPercentConversion);

			bool srcUsesAngle = ((srcType == kCopy_type_angle) || (srcType == kCopy_type_angleVector));
			bool dstUsesAngle = ((dstType == kCopy_type_angle) || (dstType == kCopy_type_angleVector));
			bool hasAngleConversion = ((srcUsesAngle && !dstUsesAngle) || (!srcUsesAngle && dstUsesAngle));
			EnableWindow( GetDlgItem(hWnd, IDC_FRAME_FLOATANGLE), hasAngleConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_DEGREES), hasAngleConversion);
			EnableWindow( GetDlgItem(hWnd, IDC_RADIANS), hasAngleConversion);
		}
	}
}




} // end of namespace PartASM