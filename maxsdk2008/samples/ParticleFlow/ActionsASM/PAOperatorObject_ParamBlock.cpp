/**********************************************************************
 *<
	FILE:			PAOperatorObject_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED BY:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"
#include "PAOperatorObject_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

// Each plug-in should have one instance of the ClassDesc class
PAOperatorObjectDesc ThePAOperatorObjectDesc;

// Dialog Proc
PAOperatorObjectDlgProc ThePAOperatorObjectDlgProc;

// Value Operator param block
ParamBlockDesc2 object_paramBlockDesc 
(
	// required block spec
		kObject_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorObjectDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kObject_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_OBJECT_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorObjectDlgProc,
	// required param specs
		// channel name
			kObject_channelName,	_T("Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME,
			end,

		// data type
			kObject_channelType,	_T("Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CHANNELTYPE,
			// optional tagged param specs
					p_default,		kObject_channelType_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_CHANNELTYPE,	kObject_channelType_num,
												IDS_CUSTOMTYPE_FLOAT,		IDS_CUSTOMTYPE_INTEGER,		
												IDS_CUSTOMTYPE_VECTOR,
					p_vals,			kObject_channelType_float,			kObject_channelType_integer,
									kObject_channelType_vector,
			end,

		// scope type: global or local
			kObject_channelScope,	_T("Scope"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPE,
			// optional tagged param specs
					p_default,		kObject_channelScope_local,
					p_range,		0, kObject_channelScope_num-1,
					p_ui,			TYPE_RADIO, kObject_channelScope_num, IDC_GLOBAL, IDC_LOCAL,
			end,

		// float property type
			kObject_propertyFloat,	_T("Object_Property_Float_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OBJECTPROPERTYFLOATTYPE,
			// optional tagged param specs
					p_default,		kObject_propertyFloat_distanceToPivot,
					p_ui,			TYPE_INTLISTBOX,	IDC_FLOATTYPE,	kObject_propertyFloat_num,
												IDS_PROPERTYFLOAT_FACESQUARE,
												IDS_PROPERTYFLOAT_DISTANCETOPIVOT,		
												IDS_PROPERTYFLOAT_DISTANCETOSURFACE,		
												IDS_PROPERTYFLOAT_DISTANCETOVERTEX,
												IDS_PROPERTYFLOAT_OBJECTSPINRATE,
												IDS_PROPERTYFLOAT_POINTLUMINANCE,
												IDS_PROPERTYFLOAT_POINTOPACITY,
												IDS_PROPERTYFLOAT_SOFTSELECTION,
					p_vals,			kObject_propertyFloat_faceSquare,
									kObject_propertyFloat_distanceToPivot,
									kObject_propertyFloat_distanceToSurface,
									kObject_propertyFloat_distanceToVertex,
									kObject_propertyFloat_objectSpinRate,
									kObject_propertyFloat_pointLuminance,
									kObject_propertyFloat_pointOpacity,
									kObject_propertyFloat_softSelection,
									
			end,

		// integer property type
			kObject_propertyInteger,	_T("Object_Property_Integer_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OBJECTPROPERTYINTEGERTYPE,
			// optional tagged param specs
					p_default,		kObject_propertyInteger_vertexIndex,
					p_ui,			TYPE_INTLISTBOX,	IDC_INTTYPE,	kObject_propertyInteger_num,
												IDS_PROPERTYINTEGER_FACEINDEX,
												IDS_PROPERTYINTEGER_MATERIALINDEX,
												IDS_PROPERTYINTEGER_OBJECTBYPIVOT,		
												IDS_PROPERTYINTEGER_OBJECTBYSURFACE,
												IDS_PROPERTYINTEGER_OBJECTBYVERTEX,		
												IDS_PROPERTYINTEGER_VERTEXINDEX,
					p_vals,			kObject_propertyInteger_faceIndex,
									kObject_propertyInteger_materialIndex,
									kObject_propertyInteger_objectIndexByPivot,
									kObject_propertyInteger_objectIndexBySurface,
									kObject_propertyInteger_objectIndexByVertex,
									kObject_propertyInteger_vertexIndex,
			end,

		// vector property type
			kObject_propertyVector,	_T("Object_Property_Vector_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OBJECTPROPERTYVECTORTYPE,
			// optional tagged param specs
					p_default,		kObject_propertyVector_objectPosition,
					p_ui,			TYPE_INTLISTBOX,	IDC_VECTORTYPE2,	kObject_propertyVector_num,
												IDS_PROPERTYVECTOR_LOCALSPEED,
												IDS_PROPERTYVECTOR_MAPPINGUVECTOR,
												IDS_PROPERTYVECTOR_MAPPINGVVECTOR,
												IDS_PROPERTYVECTOR_MAPPINGWVECTOR,
												IDS_PROPERTYVECTOR_NEARESTCOLOR,
												IDS_PROPERTYVECTOR_NEARESTMAPPING,
												IDS_PROPERTYVECTOR_NEARESTNORMAL,
												IDS_PROPERTYVECTOR_NEARESTSURFACEPOINT,
												IDS_PROPERTYVECTOR_NEARESTVERTEX,
												IDS_PROPERTYVECTOR_OBJECTEUCLIDEROTATION,
												IDS_PROPERTYVECTOR_OBJECTPOSITION,
												IDS_PROPERTYVECTOR_OBJECTSCALE,
												IDS_PROPERTYVECTOR_OBJECTSPEED,
												IDS_PROPERTYVECTOR_OBJECTSPINAXIS,
					p_vals,			kObject_propertyVector_localSpeed,
									kObject_propertyVector_mappingUVector,
									kObject_propertyVector_mappingVVector,
									kObject_propertyVector_mappingWVector,
									kObject_propertyVector_nearestColor,
									kObject_propertyVector_nearestMapping,
									kObject_propertyVector_nearestNormal,
									kObject_propertyVector_nearestSurfacePoint,
									kObject_propertyVector_nearestVertex,
									kObject_propertyVector_objectEuclideRotation,
									kObject_propertyVector_objectPosition,
									kObject_propertyVector_objectScale,
									kObject_propertyVector_objectSpeed,
									kObject_propertyVector_objectSpinAxis,
			end,

		// use sub material
			kObject_useSubMtl, _T("Use_SubMaterial"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESUBMATERIAL,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESUBMATERIAL,
			end,

		// sub-material index
			kObject_subMtl,			_T("SubMaterial_Index"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SUBMATERIALINDEX,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, 64000,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT,
									IDC_SUBMATERIAL, IDC_SUBMATERIALSPIN, 0.25f,
			end,

		// mapping channel
			kObject_mappingChannel,	_T("Mapping_Channel_Index"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MAPPINGCHANNELINDEX,
			// optional tagged param specs
					p_default,		1,
					p_range,		0, MAX_MESHMAPS-1,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT,
									IDC_MAPPINGCHANNEL, IDC_MAPPINGCHANNELSPIN, 0.25f,
			end,

		// lock closest point // not used
/*			kObject_lockClosestPoint, _T("Lock_Closest_Point"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_LOCKCLOSESTPOINT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOCK,
			end,
*/
		// compensate integration lag
			kObject_compensateIntegration, _T("Compensate_Integration_Lag"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_COMPENSATEINTEGRATIONLAG,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_COMPENSATEINTEGRATION,
			end,

		// objects
			kObject_objects, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_OBJECTLIST,IDC_PICKNODE,0,IDC_REMOVENODE,
			end,
		// objects for maxscript manipulation
			kObject_objectsMaxscript, _T("Objects"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_OBJECTS,
			end,

		// use designate
			kObject_useDesignate, _T("Use_Integer_Channel_To_Designate_Objects"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USEINTEGERCHANNELTODESIGNATEOBJECTS,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_DESIGNATEOBJECTS,
			end,

		// designation channel name
			kObject_designateName,	_T("Designation_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME2,
			end,
		
		// sync type
			kObject_sync,	_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kObject_sync_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_TARGETSYNC,	kObject_sync_num,
												IDS_SYNCTYPE_TIME, IDS_SYNCTYPE_AGE, 
												IDS_SYNCTYPE_EVENT, IDS_SYNCTYPE_CHANNEL,
					p_vals,			kObject_sync_time, kObject_sync_age, 
									kObject_sync_event,	kObject_sync_channel,
			end,

		// sync channel name
			kObject_syncName,	_T("Sync_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME3,
			end,
		
		// animated shape
			kObject_animated, _T("Animated_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANIMATEDSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATED,
			end,

		// subframe sampling for location
			kObject_subframeLocation, _T("Subframe_Sampling_For_Location"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLINGFORLOCATION,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOCATION,
			end,

		// subframe sampling for geometry
			kObject_subframeGeometry, _T("Subframe_Sampling_For_Geometry"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLINGFORGEOMETRY,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_GEOMETRY,
			end,

		// assign only once (to new particles only)
			kObject_assignOnce, _T("Assign_Only_Once"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNONLYONCE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNONCE,
			end,

		// use condition filter
			kObject_useCondition, _T("Use_Condition_Filter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECONDITIONFILTER,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECONDITION,
			end,

		// condition channel name
			kObject_conditionName,	_T("Condition_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CONDITIONCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CONDITIONNAME2,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc object_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorObjectDesc, FP_MIXIN, 
				
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

FPInterfaceDesc object_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorObjectDesc, FP_MIXIN,

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

FPInterfaceDesc object_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorObjectDesc, FP_MIXIN,

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

INT_PTR PAOperatorObjectDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
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
				case IDC_PICKNODE:
					map->GetParamBlock()->NotifyDependents( FOREVER, 0, kObject_RefMsg_ResetValidatorAction );
					break;
				case IDC_BYLIST:
					map->GetParamBlock()->NotifyDependents( FOREVER, 0, kObject_RefMsg_ListSelect );
					break;
				case kObject_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PAOperatorObjectDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
//	BOOL bAxes, bDivergence, bRandom;
//	bool varAnimated, divAnimated;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			bool enableGeometry = true;
			bool enableSubMtl = false;
			bool enableMapping = false;
			bool enableLockClosest = false;
		
			int type = ppb->GetInt(kObject_channelType, 0);
			bool isFloatType = (type == kObject_channelType_float);
			EnableWindow( GetDlgItem(hWnd, IDC_FLOATTYPE), isFloatType );
			int showControl = isFloatType ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_FLOATTYPE), showControl);
			if (isFloatType) {
				switch(ppb->GetInt(kObject_propertyFloat, 0)) {
				case kObject_propertyFloat_distanceToPivot:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_DISTANCETOPIVOT));
					break;
				case kObject_propertyFloat_distanceToVertex:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_DISTANCETOVERTEX));
					break;
				case kObject_propertyFloat_distanceToSurface:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_DISTANCETOSURFACE));
					break;
				case kObject_propertyFloat_objectSpinRate:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTSPINRATE));
					break;
				case kObject_propertyFloat_pointLuminance:
					enableSubMtl = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_POINTLUMINANCE));
					break;
				case kObject_propertyFloat_pointOpacity:
					enableSubMtl = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_POINTOPACITY));
					break;
				case kObject_propertyFloat_faceSquare:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_FACESQUARE));
					break;
				case kObject_propertyFloat_softSelection:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_SOFTSELECTION));
					break;
				}
			}

			bool isIntType = (type == kObject_channelType_integer);
			EnableWindow( GetDlgItem(hWnd, IDC_INTTYPE), isIntType );
			showControl = isIntType ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_INTTYPE), showControl);
			if (isIntType) {
				switch (ppb->GetInt(kObject_propertyInteger, 0)) {
				case kObject_propertyInteger_objectIndexByPivot:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTINDEXBYPIVOT));
					break;
				case kObject_propertyInteger_objectIndexByVertex:
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTINDEXBYVERTEX));
					break;
				case kObject_propertyInteger_objectIndexBySurface:
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTINDEXBYSURFACE));
					break;
				case kObject_propertyInteger_vertexIndex:
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_VERTEXINDEX));
					break;
				case kObject_propertyInteger_faceIndex:
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_FACEINDEX));
					break;
				case kObject_propertyInteger_materialIndex:
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_MATERIALINDEX));
					break;
				}
			}

			bool isVectorType = (type == kObject_channelType_vector);
			EnableWindow( GetDlgItem(hWnd, IDC_VECTORTYPE2), isVectorType );
			showControl = isVectorType ? SW_SHOW : SW_HIDE;
			ShowWindow( GetDlgItem( hWnd, IDC_VECTORTYPE2), showControl);
			if (isVectorType) {
				int vectorType = ppb->GetInt(kObject_propertyVector, 0);
				switch(vectorType) {
				case kObject_propertyVector_objectPosition:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTPOSITION));
					break;
				case kObject_propertyVector_objectSpeed:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTSPEED));
					break;
				case kObject_propertyVector_objectScale:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTSCALE));
					break;
				case kObject_propertyVector_objectEuclideRotation:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTEUCLIDEROTATION));
					break;
				case kObject_propertyVector_objectSpinAxis:
					enableGeometry = false;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_OBJECTSPINAXIS));
					break;
				case kObject_propertyVector_nearestColor:
					enableSubMtl = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_NEARESTCOLOR));
					break;
				case kObject_propertyVector_nearestMapping:
					enableMapping = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_NEARESTMAPPING));
					break;
				case kObject_propertyVector_mappingUVector:
					enableMapping = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_MAPPINGUVECTOR));
					break;
				case kObject_propertyVector_mappingVVector:
					enableMapping = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_MAPPINGVVECTOR));
					break;
				case kObject_propertyVector_mappingWVector:
					enableMapping = true;
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_MAPPINGWVECTOR));
					break;
				case kObject_propertyVector_nearestSurfacePoint:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_NEARESTSURFACEPOINT));
					break;
				case kObject_propertyVector_nearestVertex:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_NEARESTVERTEX));
					break;
				case kObject_propertyVector_nearestNormal:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_NEARESTNORMAL));
					break;
				case kObject_propertyVector_localSpeed:
					enableLockClosest = true;
					SetDlgItemText( hWnd, IDC_TEXT_PROPERTY, GetString(IDS_DESCRIPTION_LOCALSPEED));
					break;
				}
			}
		
			ISpinnerControl* spin = NULL;
			bool useSubMtl = (ppb->GetInt(kObject_useSubMtl, 0) != 0);
			EnableWindow( GetDlgItem(hWnd, IDC_USESUBMATERIAL), enableSubMtl );
			spin = GetISpinner( GetDlgItem( hWnd, IDC_SUBMATERIALSPIN) );
			spin->Enable(enableSubMtl && useSubMtl);
			ReleaseISpinner(spin);
			
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_MAPPINGCHANNEL), enableMapping );
			spin = GetISpinner( GetDlgItem( hWnd, IDC_MAPPINGCHANNELSPIN) );
			spin->Enable(enableMapping);
			ReleaseISpinner(spin);

//			if (enableLockClosest) // lockClosest is not used at the moment
//				if (ppb->GetInt(kObject_assignOnce, 0) != 0) enableLockClosest = false;
//			EnableWindow( GetDlgItem(hWnd, IDC_LOCK), enableLockClosest );

			int numObjects = ppb->Count(kObject_objects);
			EnableWindow( GetDlgItem(hWnd, IDC_REMOVENODE), numObjects > 0 );

			bool useDesignate = (ppb->GetInt(kObject_useDesignate, 0) != 0);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CUSTOMNAME2), useDesignate);
			EnableWindow( GetDlgItem(hWnd, IDC_CHANNELNAME2), useDesignate);
			
			bool useSyncChannel = (ppb->GetInt(kObject_sync, 0) == kObject_sync_channel);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CUSTOMNAME3), useSyncChannel);
			EnableWindow( GetDlgItem(hWnd, IDC_CHANNELNAME3), useSyncChannel);

			EnableWindow( GetDlgItem(hWnd, IDC_ANIMATED), enableGeometry);
			bool animatedShape = (ppb->GetInt(kObject_animated, 0) != 0);
			EnableWindow( GetDlgItem(hWnd, IDC_GEOMETRY), enableGeometry && animatedShape);

			bool useFilter = (ppb->GetInt(kObject_useCondition, 0) != 0);
			EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CONDITIONNAME2), useFilter);
			EnableWindow( GetDlgItem(hWnd, IDC_CONDITIONNAME2), useFilter);
		}
	}
}




} // end of namespace PartASM