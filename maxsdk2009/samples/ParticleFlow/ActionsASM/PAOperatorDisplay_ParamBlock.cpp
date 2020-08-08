/**********************************************************************
 *<
	FILE:			PAOperatorDisplay_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Display Operator (definition)
											 
	CREATED FOR:		Oleg Bayborodin

	HISTORY:		created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#include "StdAfx.h"

#include "PAOperatorDisplay_ParamBlock.h"

#include "PartASM_GlobalVariables.h"
#include "PartASM_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PartASM {

BOOL kUseGradientValue[kNumGradientValues] = { TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };
Color kGradientColor[kNumGradientValues] = { Color(0x0000FF), Color(0x00FFFF), Color(0x00FF00), Color(0xFFFF00),
											Color(0xFF0000), Color(0xFF00FF), Color(0x0000FF) };
int kIntegerValues[kNumGradientValues] = { 0, 10, 20, 30, 40, 50, 60 };
int kTimeValues[kNumGradientValues] = { 0, 800, 1600, 2400, 3200, 4000, 4800 };
float kFloatValues[kNumGradientValues] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
float kPercentValues[kNumGradientValues] = { 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f };
float kWorldValues[kNumGradientValues] = { 0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f };

float kRGBFloatMinValues[kNumRGBValues] = { 0.0f, 0.0f, 0.0f };
float kRGBFloatMaxValues[kNumRGBValues] = { 1.0f, 1.0f, 1.0f };
float kRGBPercentMinValues[kNumRGBValues] = { 0.0f, 0.0f, 0.0f };
float kRGBPercentMaxValues[kNumRGBValues] = { 1.0f, 1.0f, 1.0f };
float kRGBWorldMinValues[kNumRGBValues] = { 0.0f, 0.0f, 0.0f };
float kRGBWorldMaxValues[kNumRGBValues] = { 100.0f, 100.0f, 100.0f };

// Each plug-in should have one instance of the ClassDesc class
PAOperatorDisplayDesc		ThePAOperatorDisplayDesc;
// Dialog Proc
PAOperatorDisplayDlgProc	ThePAOperatorDisplayDlgProc;


// Display Operator param block
ParamBlockDesc2 display_paramBlockDesc 
(
	// required block spec
		kDisplay_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePAOperatorDisplayDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kDisplay_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_DISPLAY_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePAOperatorDisplayDlgProc,
	// required param specs
		// show particles
			kDisplay_showParticles, _T("Show_Particles"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWPARTICLES,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SHOWPARTICLES,
			end,
		// viewport color
			kDisplay_color, _T("Color"),
									TYPE_RGBA,
									P_RESET_DEFAULT,
									IDS_COLOR,
			// optional tagget param specs
					p_default,		Color(0, 0, 1),
					p_ui,			TYPE_COLORSWATCH, IDC_VIEWPORTCOLOR,
			end,
		// viewport type
			kDisplay_type,_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TYPE,
			// optional tagged param specs
					p_default,		kDisplay_type_ticks,
					p_ui,			TYPE_INTLISTBOX,	IDC_DISPLAYVIEWPORT,	kDisplay_type_num,
												IDS_DISPLAYTYPE_DOTS, IDS_DISPLAYTYPE_TICKS, 
												IDS_DISPLAYTYPE_CIRCLES, IDS_DISPLAYTYPE_LINES,
												IDS_DISPLAYTYPE_BBOXES, IDS_DISPLAYTYPE_GEOM,
												IDS_DISPLAYTYPE_DIAMONDS, IDS_DISPLAYTYPE_BOXES,
												IDS_DISPLAYTYPE_ASTERISKS, IDS_DISPLAYTYPE_TRIANGLES,
					p_vals,			kDisplay_type_dots,
									kDisplay_type_ticks,
									kDisplay_type_circles,
									kDisplay_type_lines,
									kDisplay_type_boundingBoxes,
									kDisplay_type_geometry,
									kDisplay_type_diamonds,
									kDisplay_type_boxes,
									kDisplay_type_asterisks,
									kDisplay_type_triangles,
			end,
		// visible % in viewport
			kDisplay_visible, _T("Visible"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_VISIBLE,
			// optional tagged param specs
					p_default,		1.0f,
					p_ms_default,	1.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_VISIBLEVIEWPORT, IDC_VISIBLEVIEWPORTSPIN, 1.0f,
			end,
		// show numbering
			kDisplay_showNumbering, _T("Show_Particle_IDs"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWNUMBERING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_NUMBER,
			end,
		// show data
			kDisplay_showData, _T("Show_Particle_Data"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWDATA,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_DATA,
			end,
		// show data for selected particles only
			kDisplay_selectedOnly, _T("For_Selected_Only"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_FORSELECTEDONLY,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SELECTEDONLY,
			end,
		// viewport type
			kDisplay_dataChannel,_T("Data_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DATATYPE,
			// optional tagged param specs
					p_default,		kDisplay_data_position,
					p_ui,			TYPE_INTLISTBOX,	IDC_DATATYPE,	kDisplay_data_num,
												IDS_DATATYPE_AGE,			IDS_DATATYPE_BIRTHTIME,
												IDS_DATATYPE_CUSTOM,		IDS_DATATYPE_DEATHTIME,
												IDS_DATATYPE_LIFESPAN,		IDS_DATATYPE_MAPCHANNEL1,	
												IDS_DATATYPE_MAPCHANNEL2,	IDS_DATATYPE_MAPCHANNEL3,	
												IDS_DATATYPE_MAPCHANNEL4,	IDS_DATATYPE_MAPCHANNEL5,	
												IDS_DATATYPE_MTLINDEX,		IDS_DATATYPE_POSITION,		
												IDS_DATATYPE_POSITIONX,		IDS_DATATYPE_POSITIONY,		
												IDS_DATATYPE_POSITIONZ,		IDS_DATATYPE_ROTATIONANGLE, 
												IDS_DATATYPE_ROTATIONAXIS,	IDS_DATATYPE_EUCLIDEROTATION,
												IDS_DATATYPE_SCALE,			IDS_DATATYPE_SCALEX,		
												IDS_DATATYPE_SCALEY,		IDS_DATATYPE_SCALEZ,		
												IDS_DATATYPE_SCRIPTFLOAT,	IDS_DATATYPE_SCRIPTINTEGER,
												IDS_DATATYPE_SCRIPTVECTOR,	IDS_DATATYPE_SELECTED,
												IDS_DATATYPE_SIZE,			IDS_DATATYPE_SIZEX,			
												IDS_DATATYPE_SIZEY,			IDS_DATATYPE_SIZEZ,			
												IDS_DATATYPE_SPEEDMAGNITUDE,IDS_DATATYPE_SPEEDVECTOR,	
												IDS_DATATYPE_SPEEDX,		IDS_DATATYPE_SPEEDY,		
												IDS_DATATYPE_SPEEDZ,		IDS_DATATYPE_SPINAXIS,		
												IDS_DATATYPE_SPINRATE,		IDS_DATATYPE_TIMEINEVENT,
												IDS_DATATYPE_VERTICESFACES,
					p_vals,			kDisplay_data_age, 			kDisplay_data_birthTime,
									kDisplay_data_custom,		kDisplay_data_deathTime,
									kDisplay_data_lifespan,		kDisplay_data_mapChannel1,	
									kDisplay_data_mapChannel2,	kDisplay_data_mapChannel3,	
									kDisplay_data_mapChannel4,	kDisplay_data_mapChannel5,	
									kDisplay_data_mtlIndex,		kDisplay_data_position,		
									kDisplay_data_positionX,	kDisplay_data_positionY,	
									kDisplay_data_positionZ,	kDisplay_data_rotationAngle,
									kDisplay_data_rotationAxis,	kDisplay_data_euclideRotation,
									kDisplay_data_scale,		kDisplay_data_scaleX,		
									kDisplay_data_scaleY,		kDisplay_data_scaleZ,		
									kDisplay_data_scriptFloat,	kDisplay_data_scriptInteger,
									kDisplay_data_scriptVector,	kDisplay_data_selected,
									kDisplay_data_size,			kDisplay_data_sizeX,		
									kDisplay_data_sizeY,		kDisplay_data_sizeZ,		
									kDisplay_data_speedMagnitude,kDisplay_data_speedVector,	
									kDisplay_data_speedX,		kDisplay_data_speedY,		
									kDisplay_data_speedZ,		kDisplay_data_spinAxis,		
									kDisplay_data_spinRate,		kDisplay_data_timeInEvent,
									kDisplay_data_verticesFaces,
			end,
		// channel name
			kDisplay_channelName,	_T("Custom_Channel_Name"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_CUSTOMCHANNELNAME,
			// optional tagged param specs
					p_default,		_T("Type Channel Name"),
					p_ui,			TYPE_EDITBOX, IDC_CHANNELNAME,
			end,


		// data type
			kDisplay_customType,_T("Custom_Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CUSTOMCHANNELTYPE,
			// optional tagged param specs
					p_default,		kDisplay_customType_float,
					p_ui,			TYPE_INTLISTBOX,	IDC_CUSTOMTYPE,	kDisplay_customType_num,
												IDS_CUSTOMTYPE_ANGLE,		IDS_CUSTOMTYPE_BOOL,		
												IDS_CUSTOMTYPE_EUCLIDEROTATION, IDS_CUSTOMTYPE_FLOAT,
												IDS_CUSTOMTYPE_FLOATVECTOR,	IDS_CUSTOMTYPE_INTEGER,		
												IDS_CUSTOMTYPE_PERCENT,		IDS_CUSTOMTYPE_PERCENTVECTOR,
												IDS_CUSTOMTYPE_TIME,		IDS_CUSTOMTYPE_WORLDUNIT,
												IDS_CUSTOMTYPE_WORLDVECTOR,
					p_vals,			kDisplay_customType_angle,		kDisplay_customType_bool, 		
									kDisplay_customType_angleVector,kDisplay_customType_float,
									kDisplay_customType_floatVector,kDisplay_customType_integer,
									kDisplay_customType_percent,	kDisplay_customType_percentVector,
									kDisplay_customType_time,		kDisplay_customType_worldFloat,
									kDisplay_customType_worldVector,
			end,
		// use color coding
			kDisplay_useColorCoding, _T("Use_Color_Coding"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECOLORCODING,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECOLORCODING,
			end,
		// use color coding
			kDisplay_useValue, _T("Use_Gradient_Value"),
									TYPE_BOOL_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_USEGRADIENTVALUE,
			// optional tagged param specs
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USE_NUM1, IDC_USE_NUM2,	IDC_USE_NUM3,
														IDC_USE_NUM4, IDC_USE_NUM5, IDC_USE_NUM6,
														IDC_USE_NUM7,
			end,
		// viewport color
			kDisplay_gradientColor, _T("Gradient_Color"),
									TYPE_RGBA_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_GRADIENTCOLOR,
			// optional tagget param specs
					p_ui,			TYPE_COLORSWATCH,	IDC_COLOR1, IDC_COLOR2, IDC_COLOR3,
														IDC_COLOR4, IDC_COLOR5, IDC_COLOR6,
														IDC_COLOR7,
			end,
		// integer values
			kDisplay_iValue,		_T("Integer_Values"),
									TYPE_INT_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_INTEGERVALUES,
			// optional tagged param specs
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	
									IDC_IVALUE1,	IDC_IVALUESPIN1,	
									IDC_IVALUE2,	IDC_IVALUESPIN2,	
									IDC_IVALUE3,	IDC_IVALUESPIN3,	
									IDC_IVALUE4,	IDC_IVALUESPIN4,	
									IDC_IVALUE5,	IDC_IVALUESPIN5,	
									IDC_IVALUE6,	IDC_IVALUESPIN6,	
									IDC_IVALUE7,	IDC_IVALUESPIN7,	
									0.5f,
			end,
		// time values
			kDisplay_tValue,		_T("Time_Values"),
									TYPE_INT_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_TIMEVALUES,
			// optional tagged param specs
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	
									IDC_TVALUE1,	IDC_TVALUESPIN1,	
									IDC_TVALUE2,	IDC_TVALUESPIN2,	
									IDC_TVALUE3,	IDC_TVALUESPIN3,	
									IDC_TVALUE4,	IDC_TVALUESPIN4,	
									IDC_TVALUE5,	IDC_TVALUESPIN5,	
									IDC_TVALUE6,	IDC_TVALUESPIN6,	
									IDC_TVALUE7,	IDC_TVALUESPIN7,	
									80.0f,
			end,
		// float values
			kDisplay_fValue,		_T("Float_Values"),
									TYPE_FLOAT_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_FLOATVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_FVALUE1,	IDC_FVALUESPIN1,	
									IDC_FVALUE2,	IDC_FVALUESPIN2,	
									IDC_FVALUE3,	IDC_FVALUESPIN3,	
									IDC_FVALUE4,	IDC_FVALUESPIN4,	
									IDC_FVALUE5,	IDC_FVALUESPIN5,	
									IDC_FVALUE6,	IDC_FVALUESPIN6,	
									IDC_FVALUE7,	IDC_FVALUESPIN7,	
									SPIN_AUTOSCALE,
			end,
		// percent values
			kDisplay_pValue,		_T("Percentage_Values"),
									TYPE_PCNT_FRAC_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_PERCENTAGEVALUES,
			// optional tagged param specs
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_PVALUE1,	IDC_PVALUESPIN1,	
									IDC_PVALUE2,	IDC_PVALUESPIN2,	
									IDC_PVALUE3,	IDC_PVALUESPIN3,	
									IDC_PVALUE4,	IDC_PVALUESPIN4,	
									IDC_PVALUE5,	IDC_PVALUESPIN5,	
									IDC_PVALUE6,	IDC_PVALUESPIN6,	
									IDC_PVALUE7,	IDC_PVALUESPIN7,	
									1.0f,
			end,
		// world unit values
			kDisplay_wValue,		_T("World_Unit_Values"),
									TYPE_WORLD_TAB, kNumGradientValues,
									P_RESET_DEFAULT,
									IDS_WORLDVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE,	
									IDC_WVALUE1,	IDC_WVALUESPIN1,	
									IDC_WVALUE2,	IDC_WVALUESPIN2,	
									IDC_WVALUE3,	IDC_WVALUESPIN3,	
									IDC_WVALUE4,	IDC_WVALUESPIN4,	
									IDC_WVALUE5,	IDC_WVALUESPIN5,	
									IDC_WVALUE6,	IDC_WVALUESPIN6,	
									IDC_WVALUE7,	IDC_WVALUESPIN7,	
									SPIN_AUTOSCALE,
			end,
		// gradient type: linear or solid
			kDisplay_gradientType, _T("Gradient_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_GRADIENTTYPE,
			// optional tagged param specs
					p_default,		kDisplay_gradientType_linear,
					p_range,		0, kDisplay_gradientType_num-1,
					p_ui,			TYPE_RADIO, kDisplay_gradientType_num, IDC_LINEAR, IDC_SOLID,
			end,
		// Red type
			kDisplay_RType,		_T("Red_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_REDTYPE,
			// optional tagged param specs
					p_default,		kDisplay_RGBType_X,
					p_range,		0, kDisplay_RGBType_num-1,
					p_ui,			TYPE_RADIO, kDisplay_RGBType_num, IDC_R_X, IDC_R_Y, IDC_R_Z,
			end,
		// Green type
			kDisplay_GType,		_T("Green_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_GREENTYPE,
			// optional tagged param specs
					p_default,		kDisplay_RGBType_Y,
					p_range,		0, kDisplay_RGBType_num-1,
					p_ui,			TYPE_RADIO, kDisplay_RGBType_num, IDC_G_X, IDC_G_Y, IDC_G_Z,
			end,
		// Blue type
			kDisplay_BType,		_T("Blue_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_BLUETYPE,
			// optional tagged param specs
					p_default,		kDisplay_RGBType_Z,
					p_range,		0, kDisplay_RGBType_num-1,
					p_ui,			TYPE_RADIO, kDisplay_RGBType_num, IDC_B_X, IDC_B_Y, IDC_B_Z,
			end,
		// RGB float min values
			kDisplay_RGBMinF,		_T("RGB_Float_Min_Values"),
									TYPE_FLOAT_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBFLOATMINVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_RFMIN,	IDC_RFMINSPIN,	
									IDC_GFMIN,	IDC_GFMINSPIN,	
									IDC_BFMIN,	IDC_BFMINSPIN,	
									SPIN_AUTOSCALE,
			end,
		// RGB float max values
			kDisplay_RGBMaxF,		_T("RGB_Float_Max_Values"),
									TYPE_FLOAT_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBFLOATMAXVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_RFMAX,	IDC_RFMAXSPIN,	
									IDC_GFMAX,	IDC_GFMAXSPIN,	
									IDC_BFMAX,	IDC_BFMAXSPIN,	
									SPIN_AUTOSCALE,
			end,
		// RGB percent min values
			kDisplay_RGBMinP,		_T("RGB_Percent_Min_Values"),
									TYPE_PCNT_FRAC_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBPERCENTMINVALUES,
			// optional tagged param specs
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_RPMIN,	IDC_RPMINSPIN,	
									IDC_GPMIN,	IDC_GPMINSPIN,	
									IDC_BPMIN,	IDC_BPMINSPIN,	
									1.0f,
			end,
		// RGB percent max values
			kDisplay_RGBMaxP,		_T("RGB_Percent_Max_Values"),
									TYPE_PCNT_FRAC_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBPERCENTMAXVALUES,
			// optional tagged param specs
					p_range,		-10000.0f, 10000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	
									IDC_RPMAX,	IDC_RPMAXSPIN,	
									IDC_GPMAX,	IDC_GPMAXSPIN,	
									IDC_BPMAX,	IDC_BPMAXSPIN,	
									1.0f,
			end,
		// RGB world min values
			kDisplay_RGBMinW,		_T("RGB_World_Min_Values"),
									TYPE_WORLD_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBWORLDMINVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE,	
									IDC_RWMIN,	IDC_RWMINSPIN,	
									IDC_GWMIN,	IDC_GWMINSPIN,	
									IDC_BWMIN,	IDC_BWMINSPIN,	
									SPIN_AUTOSCALE,
			end,
		// RGB world max values
			kDisplay_RGBMaxW,		_T("RGB_World_Max_Values"),
									TYPE_WORLD_TAB, kNumRGBValues,
									P_RESET_DEFAULT,
									IDS_RGBWORLDMAXVALUES,
			// optional tagged param specs
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE,	
									IDC_RWMAX,	IDC_RWMAXSPIN,	
									IDC_GWMAX,	IDC_GWMAXSPIN,	
									IDC_BWMAX,	IDC_BWMAXSPIN,	
									SPIN_AUTOSCALE,
			end,
		// speed factor to show lines
			kDisplay_speedFactor, _T("Speed_Scale_For_Lines"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_SPEEDSCALEFORLINES,
			// optional tagged param specs
					p_default,		1.0f,
					p_ms_default,	1.0f,
					p_range,		0.0f, 100000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_SPEEDFACTOR, IDC_SPEEDFACTORSPIN, 1.0f,
			end,
		// speed line placement
			kDisplay_speedMode,_T("Speed_Line_Placement"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SPEEDLINEPLACEMENT,
			// optional tagged param specs
					p_default,		kDisplay_speedMode_leading,
					p_ui,			TYPE_INTLISTBOX,	IDC_PLACEMENT,	kDisplay_speedMode_num,
												IDS_SPEEDMODE_LEADING,
												IDS_SPEEDMODE_CENTERED,
												IDS_SPEEDMODE_TRAILING,
					p_vals,			kDisplay_speedMode_leading,
									kDisplay_speedMode_centered,
									kDisplay_speedMode_trailing,
			end,
		// precision of the floating point values to show
			kDisplay_precision, _T("Precision"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_PRECISION,
			// optional tagged param specs
					p_default,		2,
					p_ms_default,	2,
					p_range,		0, 10,
					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_PRECISION, IDC_PRECISIONSPIN, 0.2f,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc display_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePAOperatorDisplayDesc, FP_MIXIN, 
				
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
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc display_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePAOperatorDisplayDesc, FP_MIXIN,

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

FPInterfaceDesc display_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePAOperatorDisplayDesc, FP_MIXIN,

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


//+--------------------------------------------------------------------------+
//|							Dialog Process									 |
//+--------------------------------------------------------------------------+

INT_PTR PAOperatorDisplayDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		pblock = map->GetParamBlock();
		if (pblock)
			pblock->NotifyDependents( FOREVER, (PartID)map, kDisplay_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case kDisplay_message_update:
			pblock = map->GetParamBlock();
			if (pblock)
				UpdateDlg( hWnd, pblock );
			break;
		}
		break;
	}
	return FALSE;
}

const int idcValueText[] = {IDC_TEXT_NUM1, IDC_TEXT_NUM2, IDC_TEXT_NUM3, IDC_TEXT_NUM4, IDC_TEXT_NUM5, IDC_TEXT_NUM6, IDC_TEXT_NUM7 };
const int idcValueUse[] = { IDC_USE_NUM1, IDC_USE_NUM2, IDC_USE_NUM3, IDC_USE_NUM4, IDC_USE_NUM5, IDC_USE_NUM6, IDC_USE_NUM7 };
const int idcValueColor[] = { IDC_COLOR1, IDC_COLOR2, IDC_COLOR3, IDC_COLOR4, IDC_COLOR5, IDC_COLOR6, IDC_COLOR7 };
const int idcValueF[] = { IDC_FVALUE1, IDC_FVALUE2, IDC_FVALUE3, IDC_FVALUE4, IDC_FVALUE5, IDC_FVALUE6, IDC_FVALUE7 };
const int idcValueFSpin[] = { IDC_FVALUESPIN1, IDC_FVALUESPIN2, IDC_FVALUESPIN3, IDC_FVALUESPIN4, IDC_FVALUESPIN5, IDC_FVALUESPIN6, IDC_FVALUESPIN7 };
const int idcValueI[] = { IDC_IVALUE1, IDC_IVALUE2, IDC_IVALUE3, IDC_IVALUE4, IDC_IVALUE5, IDC_IVALUE6, IDC_IVALUE7 };
const int idcValueISpin[] = { IDC_IVALUESPIN1, IDC_IVALUESPIN2, IDC_IVALUESPIN3, IDC_IVALUESPIN4, IDC_IVALUESPIN5, IDC_IVALUESPIN6, IDC_IVALUESPIN7 };
const int idcValueT[] = { IDC_TVALUE1, IDC_TVALUE2, IDC_TVALUE3, IDC_TVALUE4, IDC_TVALUE5, IDC_TVALUE6, IDC_TVALUE7 };
const int idcValueTSpin[] = { IDC_TVALUESPIN1, IDC_TVALUESPIN2, IDC_TVALUESPIN3, IDC_TVALUESPIN4, IDC_TVALUESPIN5, IDC_TVALUESPIN6, IDC_TVALUESPIN7 };
const int idcValueP[] = { IDC_PVALUE1, IDC_PVALUE2, IDC_PVALUE3, IDC_PVALUE4, IDC_PVALUE5, IDC_PVALUE6, IDC_PVALUE7 };
const int idcValuePSpin[] = { IDC_PVALUESPIN1, IDC_PVALUESPIN2, IDC_PVALUESPIN3, IDC_PVALUESPIN4, IDC_PVALUESPIN5, IDC_PVALUESPIN6, IDC_PVALUESPIN7 };
const int idcValueW[] = { IDC_WVALUE1, IDC_WVALUE2, IDC_WVALUE3, IDC_WVALUE4, IDC_WVALUE5, IDC_WVALUE6, IDC_WVALUE7 };
const int idcValueWSpin[] = { IDC_WVALUESPIN1, IDC_WVALUESPIN2, IDC_WVALUESPIN3, IDC_WVALUESPIN4, IDC_WVALUESPIN5, IDC_WVALUESPIN6, IDC_WVALUESPIN7 };
enum { kNumVectorValues = 6 };
const int idcValueRGBF[] = { IDC_RFMIN, IDC_RFMAX, IDC_GFMIN, IDC_GFMAX, IDC_BFMIN, IDC_BFMAX };
const int idcValueRGBFSpin[] = { IDC_RFMINSPIN, IDC_RFMAXSPIN, IDC_GFMINSPIN, IDC_GFMAXSPIN, IDC_BFMINSPIN, IDC_BFMAXSPIN };
const int idcValueRGBP[] = { IDC_RPMIN, IDC_RPMAX, IDC_GPMIN, IDC_GPMAX, IDC_BPMIN, IDC_BPMAX };
const int idcValueRGBPSpin[] = { IDC_RPMINSPIN, IDC_RPMAXSPIN, IDC_GPMINSPIN, IDC_GPMAXSPIN, IDC_BPMINSPIN, IDC_BPMAXSPIN };
const int idcValueRGBW[] = { IDC_RWMIN, IDC_RWMAX, IDC_GWMIN, IDC_GWMAX, IDC_BWMIN, IDC_BWMAX };
const int idcValueRGBWSpin[] = { IDC_RWMINSPIN, IDC_RWMAXSPIN, IDC_GWMINSPIN, IDC_GWMAXSPIN, IDC_BWMINSPIN, IDC_BWMAXSPIN };

void PAOperatorDisplayDlgProc::UpdateDlg( HWND hWnd, IParamBlock2* pblock )
{
	ISpinnerControl* spin;
	IColorSwatch* color;

	bool showParticles = (pblock->GetInt(kDisplay_showParticles, 0) != 0);
	bool showData = (pblock->GetInt(kDisplay_showData, 0) != 0);
	int dataChannel = pblock->GetInt(kDisplay_dataChannel, 0);
	int customType = pblock->GetInt(kDisplay_customType, 0);
	bool isCustomData = (dataChannel == kDisplay_data_custom);
	bool useColorCoding = (pblock->GetInt(kDisplay_useColorCoding, 0) != 0);
	bool useValue[7];
	int numUseValue = 0;
	for(int i=0; i<kNumGradientValues; i++) {
		useValue[i] = (pblock->GetInt(kDisplay_useValue, 0, i) != 0);
		if (useValue[i]) numUseValue++;
	}
	bool useSpeedFactor = showParticles;
	if (useSpeedFactor)
		useSpeedFactor = (pblock->GetInt(kDisplay_type, 0) == kDisplay_type_lines);

	EnableWindow( GetDlgItem(hWnd, IDC_TYPE_TEXT), showParticles );
	EnableWindow( GetDlgItem(hWnd, IDC_DISPLAYVIEWPORT), showParticles );
	
	EnableWindow( GetDlgItem(hWnd, IDC_SELECTEDONLY), showData);
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_DATA), showData || useColorCoding );
	EnableWindow( GetDlgItem(hWnd, IDC_DATATYPE), showData || useColorCoding );

	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CUSTOMNAME), (showData || useColorCoding) && isCustomData);
	EnableWindow( GetDlgItem(hWnd, IDC_CHANNELNAME), (showData || useColorCoding) && isCustomData);
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_CUSTOMTYPE), (showData || useColorCoding) && isCustomData);
	EnableWindow( GetDlgItem(hWnd, IDC_CUSTOMTYPE), (showData || useColorCoding) && isCustomData);

	int valueType = kDisplay_valueType_float;
	switch(dataChannel) {
	case kDisplay_data_custom:
		valueType = customType;
		break;
	case kDisplay_data_birthTime:
	case kDisplay_data_age:
	case kDisplay_data_deathTime:
	case kDisplay_data_lifespan:
	case kDisplay_data_timeInEvent:
		valueType = kDisplay_valueType_time;
		break;
	case kDisplay_data_position:
	case kDisplay_data_speedVector:
	case kDisplay_data_size:
		valueType = kDisplay_valueType_worldVector;
		break;
	case kDisplay_data_positionX:
	case kDisplay_data_positionY:
	case kDisplay_data_positionZ:
	case kDisplay_data_speedX:
	case kDisplay_data_speedY:
	case kDisplay_data_speedZ:
	case kDisplay_data_speedMagnitude:
	case kDisplay_data_sizeX:
	case kDisplay_data_sizeY:
	case kDisplay_data_sizeZ:
		valueType = kDisplay_valueType_worldFloat;
		break;
	case kDisplay_data_scale:
		valueType = kDisplay_valueType_percentVector;
		break;
	case kDisplay_data_scaleX:
	case kDisplay_data_scaleY:
	case kDisplay_data_scaleZ:
		valueType = kDisplay_valueType_percent;
		break;
	case kDisplay_data_euclideRotation:
		valueType = kDisplay_valueType_angleVector;
		break;
	case kDisplay_data_rotationAxis:
	case kDisplay_data_spinAxis:
	case kDisplay_data_mapChannel1:
	case kDisplay_data_mapChannel2:
	case kDisplay_data_mapChannel3:
	case kDisplay_data_mapChannel4:
	case kDisplay_data_mapChannel5:
	case kDisplay_data_scriptVector:
		valueType = kDisplay_valueType_floatVector;
		break;
	case kDisplay_data_rotationAngle:
	case kDisplay_data_spinRate:
		valueType = kDisplay_valueType_angle;
		break;
	case kDisplay_data_scriptFloat:
		valueType = kDisplay_valueType_float;
		break;
	case kDisplay_data_mtlIndex:
	case kDisplay_data_scriptInteger:
		valueType = kDisplay_valueType_integer;
		break;
	case kDisplay_data_selected:
		valueType = kDisplay_valueType_bool;
		break;
	case kDisplay_data_verticesFaces:
		valueType = kDisplay_valueType_verticesFaces;
		break;
	default: assert(0);
	}

	bool scalarType = true;
	bool vectorType = false;
	bool needPrecision = true;
	TSTR gradientFrameText = GetString(IDS_GRADIENTFORFLOATVALUES);
	switch(valueType) {
	case kDisplay_valueType_bool:
		gradientFrameText = GetString(IDS_GRADIENTFORBOOLVALUES);
		needPrecision = false;
		break;
	case kDisplay_valueType_time:
		gradientFrameText = GetString(IDS_GRADIENTFORTIMEVALUES);
		needPrecision = false;
		break;
	case kDisplay_valueType_integer:
		gradientFrameText = GetString(IDS_GRADIENTFORINTEGERVALUES);
		needPrecision = false;
		break;
	case kDisplay_valueType_float:
		gradientFrameText = GetString(IDS_GRADIENTFORFLOATVALUES);
		break;
	case kDisplay_valueType_percent:
		gradientFrameText = GetString(IDS_GRADIENTFORPERCENTVALUES);
		break;
	case kDisplay_valueType_worldFloat:
		gradientFrameText = GetString(IDS_GRADIENTFORWORLDVALUES);
		needPrecision = false;
		break;
	case kDisplay_valueType_angle:
		gradientFrameText = GetString(IDS_GRADIENTFORANGLEFALUES);
		break;
	case kDisplay_valueType_worldVector:
		needPrecision = false;
		// the break is omitted on purpose
	case kDisplay_valueType_floatVector:
	case kDisplay_valueType_percentVector:
	case kDisplay_valueType_angleVector:
		scalarType = false;
		vectorType = true;
		break;
	case kDisplay_valueType_verticesFaces:
		scalarType = false;
		needPrecision = false;
		break;
	default: assert(0);
	}

	EnableWindow( GetDlgItem(hWnd, IDC_USECOLORCODING), showParticles && (valueType != kDisplay_valueType_verticesFaces));
	bool doingColorCoding = showParticles && useColorCoding;
	bool doingScalar = scalarType && doingColorCoding;
	SetDlgItemText( hWnd, IDC_FRAME_GRADIENT, gradientFrameText );
	EnableWindow( GetDlgItem(hWnd, IDC_FRAME_GRADIENT), doingScalar );

	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_NUM1), doingScalar);
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_NUM2), doingScalar);
	color = GetIColorSwatch(GetDlgItem(hWnd, IDC_COLOR1));
	color->Enable(doingScalar && (useValue[0] || (valueType == kDisplay_valueType_bool)));
	ReleaseIColorSwatch(color);
	color = GetIColorSwatch(GetDlgItem(hWnd, IDC_COLOR2));
	color->Enable(doingScalar && (useValue[1] || (valueType == kDisplay_valueType_bool)));
	ReleaseIColorSwatch(color);
	for(int i=2; i<kNumGradientValues; i++) {
		EnableWindow( GetDlgItem(hWnd, idcValueText[i]), doingScalar && (valueType != kDisplay_valueType_bool));
		color = GetIColorSwatch(GetDlgItem(hWnd, idcValueColor[i]));
		color->Enable(doingScalar && (valueType != kDisplay_valueType_bool) && useValue[i]);
		ReleaseIColorSwatch(color);
	}
	int showControl;
	for(int i=0; i<kNumGradientValues; i++) {
		EnableWindow( GetDlgItem(hWnd, idcValueUse[i]), doingScalar && (valueType != kDisplay_valueType_bool));
		spin = GetISpinner( GetDlgItem( hWnd, idcValueFSpin[i]) );
		spin->Enable(((valueType == kDisplay_valueType_float) || (valueType == kDisplay_valueType_angle)) && useValue[i]);
		ReleaseISpinner(spin);
		showControl = (!scalarType || (valueType == kDisplay_valueType_float) || (valueType == kDisplay_valueType_angle)
										|| (valueType == kDisplay_valueType_bool)) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueF[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueFSpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValueISpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_integer) && useValue[i]);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_integer) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueI[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueISpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValueTSpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_time) && useValue[i]);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_time) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueT[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueTSpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValuePSpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_percent) && useValue[i]);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_percent) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueP[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValuePSpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValueWSpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_worldFloat) && useValue[i]);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_worldFloat) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueW[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueWSpin[i]), showControl);
	}
	if (valueType == kDisplay_valueType_bool) {
		ShowWindow(GetDlgItem( hWnd, IDC_FALSEVALUE_TEXT), SW_SHOW);
		ShowWindow(GetDlgItem( hWnd, IDC_TRUEVALUE_TEXT), SW_SHOW);
		ShowWindow(GetDlgItem( hWnd, IDC_FVALUE1), SW_HIDE);
		ShowWindow(GetDlgItem( hWnd, IDC_FVALUESPIN1), SW_HIDE);
		ShowWindow(GetDlgItem( hWnd, IDC_FVALUE2), SW_HIDE);
		ShowWindow(GetDlgItem( hWnd, IDC_FVALUESPIN2), SW_HIDE);
	} else {
		ShowWindow(GetDlgItem( hWnd, IDC_FALSEVALUE_TEXT), SW_HIDE);
		ShowWindow(GetDlgItem( hWnd, IDC_TRUEVALUE_TEXT), SW_HIDE);
	}
	bool enableControl = doingScalar && (valueType != kDisplay_valueType_bool) && (numUseValue > 1);
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_TYPE), enableControl );
	EnableWindow( GetDlgItem(hWnd, IDC_LINEAR), enableControl );
	EnableWindow( GetDlgItem(hWnd, IDC_SOLID), enableControl );

	bool doingVector = vectorType && doingColorCoding;
	EnableWindow( GetDlgItem(hWnd, IDC_FRAME_VECTOR), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_R), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_R_X), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_R_Y), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_R_Z), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_RMIN), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_RMAX), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_G), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_G_X), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_G_Y), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_G_Z), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_GMIN), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_GMAX), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_B), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_B_X), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_B_Y), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_B_Z), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BMIN), doingVector );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_BMAX), doingVector );
	for(int i=0; i<kNumVectorValues; i++) {
		spin = GetISpinner( GetDlgItem( hWnd, idcValueRGBFSpin[i]) );
		spin->Enable(((valueType == kDisplay_valueType_floatVector) || (valueType == kDisplay_valueType_angleVector)) && doingVector);
		ReleaseISpinner(spin);
		showControl = (!vectorType || (valueType == kDisplay_valueType_floatVector) || (valueType == kDisplay_valueType_angleVector)) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueRGBF[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueRGBFSpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValueRGBPSpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_percentVector) && doingVector);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_percentVector) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueRGBP[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueRGBPSpin[i]), showControl);
		spin = GetISpinner( GetDlgItem( hWnd, idcValueRGBWSpin[i]) );
		spin->Enable((valueType == kDisplay_valueType_worldVector)  && doingVector);
		ReleaseISpinner(spin);
		showControl = (valueType == kDisplay_valueType_worldVector) ? SW_SHOW : SW_HIDE;
		ShowWindow( GetDlgItem( hWnd, idcValueRGBW[i]), showControl);
		ShowWindow( GetDlgItem( hWnd, idcValueRGBWSpin[i]), showControl);
	}

	EnableWindow( GetDlgItem(hWnd, IDC_FRAME_LINES), useSpeedFactor );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_SPEEDFACTOR), useSpeedFactor );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SPEEDFACTORSPIN) );
	spin->Enable(useSpeedFactor);
	ReleaseISpinner(spin);
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_PLACEMENT), useSpeedFactor );
	EnableWindow( GetDlgItem(hWnd, IDC_PLACEMENT), useSpeedFactor );

	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_PRECISION), needPrecision );
	EnableWindow( GetDlgItem(hWnd, IDC_TEXT_DECIMALS), needPrecision );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_PRECISIONSPIN) );
	spin->Enable(needPrecision);
	ReleaseISpinner(spin);
}


} // end of namespace PartASM