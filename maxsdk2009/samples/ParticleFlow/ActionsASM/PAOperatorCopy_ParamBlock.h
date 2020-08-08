/**********************************************************************
 *<
	FILE:			PAOperatorCopy_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORCOPY_PARAMBLOCK_H_
#define  _PAOPERATORCOPY_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorCopyDesc.h"

namespace PartASM {

// block IDs
enum { kCopy_mainPBlockIndex };


// param IDs
enum {	kCopy_srcChannel,
		kCopy_srcName,
		kCopy_srcType,
		kCopy_dstChannel,
		kCopy_dstName,
		kCopy_dstType,
		kCopy_dstScope,
		kCopy_assignOnce,
		kCopy_useCondition,
		kCopy_conditionName,
		kCopy_scalarToVectorType,
		kCopy_floatToIntegerType,
		kCopy_floatToTimeType,
		kCopy_floatToPercentType,
		kCopy_floatToAngleType };

enum {	kCopy_data_custom,
		kCopy_data_localTime,
		kCopy_data_birthTime,
		kCopy_data_age,
		kCopy_data_deathTime,
		kCopy_data_lifespan,
		kCopy_data_timeInEvent,
		kCopy_data_position,
		kCopy_data_speed,
		kCopy_data_scale,
		kCopy_data_size,
		kCopy_data_rotationAxis,
		kCopy_data_rotationAngle,
		kCopy_data_euclideRotation,
		kCopy_data_spinAxis,
		kCopy_data_spinRate,
		kCopy_data_mtlIndex,
		kCopy_data_selected,
		kCopy_data_mapChannel1,
		kCopy_data_mapChannel2,
		kCopy_data_mapChannel3,
		kCopy_data_mapChannel4,
		kCopy_data_mapChannel5,
		kCopy_data_shapeVertices,
		kCopy_data_shapeFaces,
		kCopy_data_shapeSurface,
		kCopy_data_shapeVolume,
		kCopy_data_scriptInteger,
		kCopy_data_scriptFloat,
		kCopy_data_scriptVector,
		kCopy_data_scriptMatrixX,
		kCopy_data_scriptMatrixY,
		kCopy_data_scriptMatrixZ,
		kCopy_data_scriptMatrixT,
		kCopy_data_particleID,
		kCopy_data_num };

enum {	kCopy_type_boolean,
		kCopy_type_time,
		kCopy_type_integer,
		kCopy_type_float,
		kCopy_type_percent,
		kCopy_type_worldFloat,
		kCopy_type_angle,
		kCopy_type_floatVector,
		kCopy_type_percentVector,
		kCopy_type_worldVector,
		kCopy_type_angleVector,
		kCopy_type_num };

enum {	kCopy_scope_global,
		kCopy_scope_local,
		kCopy_scope_num };

enum {	kCopy_svType_x,
		kCopy_svType_y,
		kCopy_svType_z,
		kCopy_svType_xyz,
		kCopy_svType_length,
		kCopy_svType_average,
		kCopy_svType_minimum,
		kCopy_svType_maximum,
		kCopy_svType_num };

enum {	kCopy_fiType_up,
		kCopy_fiType_down,
		kCopy_fiType_round,
		kCopy_fiType_num };

enum {	kCopy_ftType_sec,
		kCopy_ftType_frame,
		kCopy_ftType_tick,
		kCopy_ftType_num };

enum {	kCopy_fpType_factor1,
		kCopy_fpType_factor100,
		kCopy_fpType_num };

enum {	kCopy_faType_degrees,
		kCopy_faType_radians,
		kCopy_faType_num };

	
// user messages
enum { kCopy_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kCopy_message_update = 100 };		// change in parameters

class PAOperatorCopyDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorCopyDesc ThePAOperatorCopyDesc;
extern ParamBlockDesc2 value_paramBlockDesc;
extern FPInterfaceDesc value_action_FPInterfaceDesc;
extern FPInterfaceDesc value_operator_FPInterfaceDesc;
extern FPInterfaceDesc value_PViewItem_FPInterfaceDesc;



} // end of namespace PartASM

#endif // _PAOPERATORCOPY_PARAMBLOCK_H_