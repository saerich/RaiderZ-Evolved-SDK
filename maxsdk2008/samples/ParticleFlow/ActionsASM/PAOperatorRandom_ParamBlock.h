/**********************************************************************
 *<
	FILE:			PAOperatorRandom_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED FOR:	Oleg Bayborodin

	HISTORY: created 05-17-03

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef  _PAOPERATORRANDOM_PARAMBLOCK_H_
#define  _PAOPERATORRANDOM_PARAMBLOCK_H_

#include "iparamm2.h"

#include "PartAsm_GlobalVariables.h"
#include "PAOperatorRandomDesc.h"

namespace PartASM {

// block IDs
enum { kRandom_mainPBlockIndex };


// param IDs
enum {	kRandom_name,
		kRandom_type,
		kRandom_scope,
		kRandom_integerType,
		kRandom_floatType,
		kRandom_vectorType,
		kRandom_param1,
		kRandom_param2,
		kRandom_param3,
		kRandom_posParam1,
		kRandom_posParam2,
		kRandom_posParam3,
		kRandom_intParam1,
		kRandom_intParam2,
		kRandom_intParam3,
		kRandom_assignOnce,
		kRandom_useCondition,
		kRandom_conditionName,
		kRandom_seed };

enum {	kRandom_type_integer,
		kRandom_type_float,
		kRandom_type_vector,
		kRandom_type_num };

enum {	kRandom_scope_global,
		kRandom_scope_local,
		kRandom_scope_num };

enum {  kRandom_integerType_uniform,
		kRandom_integerType_num };

enum {	kRandom_floatType_uniform,
		kRandom_floatType_triangular,
		kRandom_floatType_normal,
		kRandom_floatType_exponential,
		kRandom_floatType_weibull,
		kRandom_floatType_num };

enum {  kRandom_vectorType_uniform,
		kRandom_vectorType_sphereSurface,
		kRandom_vectorType_sphereVolume,
		kRandom_vectorType_normal,
		kRandom_vectorType_num };
	
// user messages
enum { kRandom_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kRandom_message_update = 100 };		// change in parameters

class PAOperatorRandomDlgProc : public ParamMap2UserDlgProc
{
public:
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

extern PAOperatorRandomDesc ThePAOperatorRandomDesc;
extern ParamBlockDesc2 random_paramBlockDesc;
extern FPInterfaceDesc random_action_FPInterfaceDesc;
extern FPInterfaceDesc random_operator_FPInterfaceDesc;
extern FPInterfaceDesc random_PViewItem_FPInterfaceDesc;




} // end of namespace PartASM

#endif // _PAOPERATORRANDOM_PARAMBLOCK_H_