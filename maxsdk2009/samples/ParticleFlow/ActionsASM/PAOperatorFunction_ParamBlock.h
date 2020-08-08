/**********************************************************************
 *<
	FILE:			PAOperatorFunction_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORFUNCTION_PARAMBLOCK_H_
#define  _PAOPERATORFUNCTION_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorFunctionDesc.h"

namespace PartASM {

// block IDs
enum { kFunction_mainPBlockIndex };


// param IDs
enum {	kFunction_opAName,
		kFunction_opAType,
		kFunction_opAFactor,
		kFunction_useOpB,
		kFunction_opBName,
		kFunction_opBType,
		kFunction_opBFactor,
		kFunction_bbType,
		kFunction_ttType,
		kFunction_fType,
		kFunction_ffType,
		kFunction_vType,
		kFunction_vfType,
		kFunction_vvType,
		kFunction_resName,
		kFunction_resFactor,
		kFunction_scope,
		kFunction_assignOnce,
		kFunction_useCondition,
		kFunction_conditionName };

enum {	kFunction_opAType_boolean,
		kFunction_opAType_integer,
		kFunction_opAType_float,
		kFunction_opAType_time,
		kFunction_opAType_vector,
		kFunction_opAType_num };

enum {	kFunction_opBType_integer,
		kFunction_opBType_float,
		kFunction_opBType_vector,
		kFunction_opBType_num };

enum {	kFunction_opType_boolean,
		kFunction_opType_integer,
		kFunction_opType_float,
		kFunction_opType_time,
		kFunction_opType_vector,
		kFunction_opType_num };

enum {	kFunction_bbType_and,
		kFunction_bbType_or,
		kFunction_bbType_xor,
		kFunction_bbType_num };

enum {	kFunction_ttType_add,
		kFunction_ttType_subtract,
		kFunction_ttType_num };

enum {	kFunction_fType_abs,
		kFunction_fType_inverse,
		kFunction_fType_square,
		kFunction_fType_squareRoot,
		kFunction_fType_cube,
		kFunction_fType_cubicRoot,
		kFunction_fType_nExp,
		kFunction_fType_dExp,
		kFunction_fType_bExp,
		kFunction_fType_nLog,
		kFunction_fType_dLog,
		kFunction_fType_bLog,
		kFunction_fType_sin,
		kFunction_fType_cos,
		kFunction_fType_tg,
		kFunction_fType_ctg,
		kFunction_fType_identity,
		kFunction_fType_num };

enum {	kFunction_ffType_add,
		kFunction_ffType_subtract,
		kFunction_ffType_multiply,
		kFunction_ffType_divide,
		kFunction_ffType_mod,
		kFunction_ffType_power,
		kFunction_ffType_log,
		kFunction_ffType_min,
		kFunction_ffType_max,
		kFunction_ffType_num };

enum {	kFunction_vType_norm,
		kFunction_vType_length,
		kFunction_vType_identity,
		kFunction_vType_num };

enum {	kFunction_vfType_multiply,
		kFunction_vfType_divide,
		kFunction_vfType_deviate,
		kFunction_vfType_num };

enum {	kFunction_vvType_add,
		kFunction_vvType_subtract,
		kFunction_vvType_distance,
		kFunction_vvType_dot,
		kFunction_vvType_cross,
		kFunction_vvType_addRotation,
		kFunction_vvType_scaleProd,
		kFunction_vvType_conform,
		kFunction_vvType_rotateBy,
		kFunction_vvType_num };

enum {	kFunction_scope_global,
		kFunction_scope_local,
		kFunction_scope_num };
	
// user messages
enum { kFunction_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kFunction_message_update = 100 };		// change in parameters

class PAOperatorFunctionDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorFunctionDesc ThePAOperatorFunctionDesc;
extern ParamBlockDesc2 function_paramBlockDesc;
extern FPInterfaceDesc function_action_FPInterfaceDesc;
extern FPInterfaceDesc function_operator_FPInterfaceDesc;
extern FPInterfaceDesc function_PViewItem_FPInterfaceDesc;


} // end of namespace PartASM

#endif // _PAOPERATORFUNCTION_PARAMBLOCK_H_