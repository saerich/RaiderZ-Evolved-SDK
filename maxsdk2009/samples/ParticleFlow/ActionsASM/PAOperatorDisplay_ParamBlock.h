/**********************************************************************
 *<
	FILE:			PAOperatorDisplay_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Display Operator (declaration)
											 
	CREATED FOR:		Oleg Bayborodin

	HISTORY:		created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORDISPLAY_PARAMBLOCK_H_
#define  _PAOPERATORDISPLAY_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PAOperatorDisplayDesc.h"

namespace PartASM {

// block IDs
enum { kDisplay_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kDisplay_type, 
		kDisplay_visible,
		kDisplay_color,
		kDisplay_showParticles,
		kDisplay_showNumbering,
		kDisplay_showData,
		kDisplay_selectedOnly,
		kDisplay_dataChannel,
		kDisplay_channelName,
		kDisplay_customType,
		kDisplay_useColorCoding,
		kDisplay_useValue,
		kDisplay_gradientColor,
		kDisplay_iValue,
		kDisplay_fValue,
		kDisplay_tValue,
		kDisplay_pValue,
		kDisplay_wValue,
		kDisplay_gradientType,
		kDisplay_RType,
		kDisplay_GType,
		kDisplay_BType,
		kDisplay_RGBMinF,
		kDisplay_RGBMaxF,
		kDisplay_RGBMinP,
		kDisplay_RGBMaxP,
		kDisplay_RGBMinW,
		kDisplay_RGBMaxW,
		kDisplay_speedFactor,
		kDisplay_speedMode,
		kDisplay_precision
};

enum {	kDisplay_type_dots,
		kDisplay_type_ticks,
		kDisplay_type_circles,
		kDisplay_type_lines,
		kDisplay_type_boundingBoxes,
		kDisplay_type_geometry,
		kDisplay_type_diamonds,
		kDisplay_type_boxes,
		kDisplay_type_asterisks,
		kDisplay_type_triangles,
		kDisplay_type_Xs,			// not in UI, for empty geometry or boundingBoxes.
		kDisplay_type_num=10 };

enum {	kDisplay_data_custom,
		kDisplay_data_birthTime,
		kDisplay_data_age,
		kDisplay_data_deathTime,
		kDisplay_data_lifespan,
		kDisplay_data_timeInEvent,
		kDisplay_data_position,
		kDisplay_data_positionX,
		kDisplay_data_positionY,
		kDisplay_data_positionZ,
		kDisplay_data_speedVector,
		kDisplay_data_speedX,
		kDisplay_data_speedY,
		kDisplay_data_speedZ,
		kDisplay_data_speedMagnitude,
		kDisplay_data_scale,
		kDisplay_data_scaleX,
		kDisplay_data_scaleY,
		kDisplay_data_scaleZ,
		kDisplay_data_size,
		kDisplay_data_sizeX,
		kDisplay_data_sizeY,
		kDisplay_data_sizeZ,
		kDisplay_data_rotationAxis,
		kDisplay_data_rotationAngle,
		kDisplay_data_euclideRotation,
		kDisplay_data_spinAxis,
		kDisplay_data_spinRate,
		kDisplay_data_mtlIndex,
		kDisplay_data_selected,
		kDisplay_data_mapChannel1,
		kDisplay_data_mapChannel2,
		kDisplay_data_mapChannel3,
		kDisplay_data_mapChannel4,
		kDisplay_data_mapChannel5,
		kDisplay_data_verticesFaces,
		kDisplay_data_scriptInteger,
		kDisplay_data_scriptFloat,
		kDisplay_data_scriptVector,
		kDisplay_data_num,
		kDisplay_data_customWorldFloat, // this and below: special types for custom channel
		kDisplay_data_customPercent,
		kDisplay_data_customAngle,
		kDisplay_data_customAngleVector };

enum {	kDisplay_valueType_bool,
		kDisplay_valueType_time,
		kDisplay_valueType_integer,
		kDisplay_valueType_float,
		kDisplay_valueType_worldFloat,
		kDisplay_valueType_percent,
		kDisplay_valueType_angle,
		kDisplay_valueType_floatVector,
		kDisplay_valueType_worldVector,
		kDisplay_valueType_percentVector,
		kDisplay_valueType_angleVector,
		kDisplay_valueType_verticesFaces };

enum {	kDisplay_customType_bool,
		kDisplay_customType_time,
		kDisplay_customType_integer,
		kDisplay_customType_float,
		kDisplay_customType_worldFloat,
		kDisplay_customType_percent,
		kDisplay_customType_angle,
		kDisplay_customType_floatVector,
		kDisplay_customType_worldVector,
		kDisplay_customType_percentVector,
		kDisplay_customType_angleVector,
		kDisplay_customType_num };

enum {	kDisplay_gradientType_linear,
		kDisplay_gradientType_solid,
		kDisplay_gradientType_num };

enum {	kDisplay_RGBType_X,
		kDisplay_RGBType_Y,
		kDisplay_RGBType_Z,
		kDisplay_RGBType_num };

enum {	kDisplay_speedMode_leading,
		kDisplay_speedMode_centered,
		kDisplay_speedMode_trailing,
		kDisplay_speedMode_num };

enum { kNumGradientValues = 7 };

extern BOOL kUseGradientValue[kNumGradientValues];
extern Color kGradientColor[kNumGradientValues];
extern int kIntegerValues[kNumGradientValues];
extern TimeValue kTimeValues[kNumGradientValues];
extern float kFloatValues[kNumGradientValues];
extern float kPercentValues[kNumGradientValues];
extern float kWorldValues[kNumGradientValues];

enum { kNumRGBValues = 3 };
extern float kRGBFloatMinValues[kNumRGBValues];
extern float kRGBFloatMaxValues[kNumRGBValues];
extern float kRGBPercentMinValues[kNumRGBValues];
extern float kRGBPercentMaxValues[kNumRGBValues];
extern float kRGBWorldMinValues[kNumRGBValues];
extern float kRGBWorldMaxValues[kNumRGBValues];


extern PAOperatorDisplayDesc ThePAOperatorDisplayDesc;
extern ParamBlockDesc2 display_paramBlockDesc;
extern FPInterfaceDesc display_action_FPInterfaceDesc;
extern FPInterfaceDesc display_operator_FPInterfaceDesc;
extern FPInterfaceDesc display_PViewItem_FPInterfaceDesc;


// User Defined Reference Messages from PB
enum {	kDisplay_RefMsg_InitDlg = REFMSG_USER + 13279 };

// dialog messages
enum {	kDisplay_message_update = 100 };

class PAOperatorDisplayDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	void UpdateDlg( HWND hWnd, IParamBlock2* pblock );
};




} // end of namespace PartASM

#endif // _PAOPERATORDISPLAY_PARAMBLOCK_H_