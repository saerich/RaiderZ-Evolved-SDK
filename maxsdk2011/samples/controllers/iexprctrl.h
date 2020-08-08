/*****************************************************************************

FILE: IExprCtrl.h

DESCRIPTION: Describes all interfaces and constants about the expression 
controllers that the outside world should need to know.

CREATED BY:	Larry Minton

HISTORY:	April 11, 2005	Creation

Copyright (c) 2005, All Rights Reserved.
*****************************************************************************/

#ifndef __IEXPRCTRL__H
#define __IEXPRCTRL__H

#include "iFnPub.h"


/***************************************************************
Function Publishing System   
****************************************************************/

#define IID_EXPR_CONTROL Interface_ID(0x15b3e322, 0x6a176aa5)

///////////////////////////////////////////////////////////////////////////////

// Base Expression Controller Interface
class IExprCtrl: public FPMixinInterface
{
public:

	// Function Publishing System
	enum {
		fnIdSetExpression, fnIdGetExpression,
		fnIdNumScalars, fnIdNumVectors,
		fnIdAddScalarTarget, fnIdAddVectorTarget,
		fnIdAddScalarConstant, fnIdAddVectorConstant,
		fnIdDeleteVariable,
		fnIdSetDescription, fnIdGetDescription,
		fnIdVariableExists,
		fnIdAddVectorNode,
		fnIdSetScalarTarget, fnIdSetVectorTarget, fnIdSetVectorNode,
		fnIdSetScalarConstant, fnIdSetVectorConstant,
		fnIdUpdate,
		fnIdGetOffset, fnIdSetOffset,
		fnIdGetVariableType,
		fnIdGetScalarConstant,
		fnIdGetScalarTarget,
		fnIdGetVectorConstant,
		fnIdGetVectorTarget,
		fnIdGetVectorNode,
		fnIdGetScalarValue, fnIdGetVectorValue, fnIdGetValue,
		//
		fnIdGetScalarType,
		fnIdGetVectorType,
		//
		fnIdGetValueType,
		fnIdGetScalarName,
		fnIdGetVectorName,
		//
		fnIdGetScalarIndex,
		fnIdGetVectorIndex,

		fnIdPrintDetails,

		fnIdGetThrowOnError,
		fnIdSetThrowOnError,

		fnIdRenameVariable,

		enumValueType,
	};

	// Function Map For Mixin Interface
	///////////////////////////////////////////////////////////////////////////
	BEGIN_FUNCTION_MAP
		FN_0( fnIdPrintDetails, TYPE_TSTR_BV, PrintDetails);

		FN_1( fnIdSetExpression, TYPE_BOOL, SetExpression, TYPE_TSTR_BR);
		FN_0( fnIdGetExpression, TYPE_TSTR_BV, GetExpression);

		FN_0( fnIdNumScalars, TYPE_INT, NumScalars);
		FN_0( fnIdNumVectors, TYPE_INT, NumVectors);

		FN_4( fnIdAddScalarTarget, TYPE_BOOL, AddScalarTarget, TYPE_TSTR_BR, TYPE_VALUE, TYPE_TIMEVALUE, TYPE_REFTARG);
		FN_4( fnIdAddVectorTarget, TYPE_BOOL, AddVectorTarget, TYPE_TSTR_BR, TYPE_VALUE, TYPE_TIMEVALUE, TYPE_REFTARG);
		FN_3( fnIdAddVectorNode, TYPE_BOOL, AddVectorNode, TYPE_TSTR_BR, TYPE_INODE, TYPE_TIMEVALUE);

		FN_2( fnIdAddScalarConstant, TYPE_BOOL, AddScalarConstant, TYPE_TSTR_BR, TYPE_FLOAT); 
		FN_2( fnIdAddVectorConstant, TYPE_BOOL, AddVectorConstant, TYPE_TSTR_BR, TYPE_POINT3);

		FN_3( fnIdSetScalarTarget, TYPE_BOOL, SetScalarTarget, TYPE_VALUE, TYPE_VALUE, TYPE_REFTARG);
		FN_3( fnIdSetVectorTarget, TYPE_BOOL, SetVectorTarget, TYPE_VALUE, TYPE_VALUE, TYPE_REFTARG);
		FN_2( fnIdSetVectorNode, TYPE_BOOL, SetVectorNode, TYPE_VALUE, TYPE_INODE);

		FN_2( fnIdSetScalarConstant, TYPE_BOOL, SetScalarConstant, TYPE_VALUE, TYPE_FLOAT); 
		FN_2( fnIdSetVectorConstant, TYPE_BOOL, SetVectorConstant, TYPE_VALUE, TYPE_POINT3);

		FN_1 (fnIdDeleteVariable, TYPE_BOOL, DeleteVariable, TYPE_TSTR_BR);
		FN_2 (fnIdRenameVariable, TYPE_BOOL, RenameVariable, TYPE_TSTR_BR, TYPE_TSTR_BR);

		FN_0( fnIdGetDescription, TYPE_TSTR_BV, GetDescription);
		FN_1( fnIdSetDescription, TYPE_BOOL, SetDescription, TYPE_TSTR_BR);

		FN_1 (fnIdVariableExists, TYPE_BOOL, VariableExists, TYPE_TSTR_BR);

		VFN_0( fnIdUpdate, Update);

		FN_1 (fnIdGetOffset, TYPE_TIMEVALUE, GetOffset, TYPE_TSTR_BR);
		FN_2 (fnIdSetOffset, TYPE_BOOL, SetOffset, TYPE_TSTR_BR, TYPE_TIMEVALUE);

		FNT_1 (fnIdGetScalarConstant, TYPE_FLOAT, GetScalarConstant, TYPE_VALUE);
		FN_1 (fnIdGetVectorConstant, TYPE_POINT3_BV, GetVectorConstant, TYPE_VALUE);

		FN_2 (fnIdGetScalarTarget, TYPE_VALUE, GetScalarTarget, TYPE_VALUE, TYPE_BOOL);
		FN_2 (fnIdGetVectorTarget, TYPE_VALUE, GetVectorTarget, TYPE_VALUE, TYPE_BOOL);

		FN_1 (fnIdGetVectorNode, TYPE_INODE, GetVectorNode, TYPE_VALUE);

		FNT_1 (fnIdGetScalarValue, TYPE_FLOAT, GetScalarValue, TYPE_VALUE);
		FNT_1 (fnIdGetVectorValue, TYPE_POINT3_BV, GetVectorValue, TYPE_VALUE);
		FNT_1 (fnIdGetValue, TYPE_FPVALUE_BV, GetVarValue, TYPE_TSTR_BR);

		FN_1 (fnIdGetScalarType, TYPE_ENUM, GetScalarType, TYPE_VALUE);
		FN_1 (fnIdGetVectorType, TYPE_ENUM, GetVectorType, TYPE_VALUE);
		FN_1 (fnIdGetValueType, TYPE_ENUM, GetValueType, TYPE_TSTR_BR);

		FN_1 (fnIdGetScalarName, TYPE_TSTR_BV, GetScalarName, TYPE_INDEX);
		FN_1 (fnIdGetVectorName, TYPE_TSTR_BV, GetVectorName, TYPE_INDEX);
		//
		FN_1 (fnIdGetScalarIndex, TYPE_INDEX, GetScalarIndex, TYPE_TSTR_BR);
		FN_1 (fnIdGetVectorIndex, TYPE_INDEX, GetVectorIndex, TYPE_TSTR_BR);

		VFN_1 (fnIdSetThrowOnError, SetThrowOnError, TYPE_bool);
		FN_0 (fnIdGetThrowOnError, TYPE_bool, GetThrowOnError);
	END_FUNCTION_MAP

	FPInterfaceDesc* GetDesc(); 

	// Published Functions
	virtual bool		GetThrowOnError() = 0;
	virtual void		SetThrowOnError(bool bOn) = 0;

	virtual TSTR		PrintDetails() = 0;
	virtual BOOL		SetExpression(TSTR &expression) = 0;
	virtual TSTR		GetExpression() = 0;
	virtual int			NumScalars() = 0;
	virtual int			NumVectors() = 0;

	virtual TSTR		GetDescription() = 0;
	virtual BOOL		SetDescription(TSTR &expression) = 0;

	virtual BOOL		AddScalarTarget(TSTR &name, Value* target, int ticks, ReferenceTarget *owner = NULL) = 0;
	virtual BOOL		AddVectorTarget(TSTR &name, Value* target, int ticks, ReferenceTarget *owner = NULL) = 0; 

	virtual BOOL		AddScalarConstant(TSTR &name, float	val) = 0; 
	virtual BOOL		AddVectorConstant(TSTR &name, Point3 point) = 0;

	virtual BOOL		AddVectorNode(TSTR &name, INode* node, int ticks) = 0;

	virtual BOOL		SetScalarTarget(Value* which, Value* target, ReferenceTarget *owner = NULL) = 0;
	virtual BOOL		SetVectorTarget(Value* which, Value* target, ReferenceTarget *owner = NULL) = 0; 
	virtual BOOL		SetVectorNode(Value* which, INode* node) = 0;
	virtual BOOL		SetScalarConstant(Value* which, float val) = 0; 
	virtual BOOL		SetVectorConstant(Value* which, Point3 point) = 0;

	virtual BOOL		DeleteVariable(TSTR &name) = 0;
	virtual BOOL		RenameVariable(TSTR &oldName, TSTR &newName) = 0;

	virtual BOOL		VariableExists(TSTR &name) = 0;
	virtual void		Update() = 0;
	virtual TimeValue	GetOffset(TSTR &name) = 0;
	virtual BOOL		SetOffset(TSTR &name, TimeValue tick) = 0;

	virtual float		GetScalarConstant(Value* which, TimeValue t) = 0;
	virtual Value*		GetScalarTarget(Value* which, BOOL asController) = 0;
	virtual Point3		GetVectorConstant(Value* which) = 0;
	virtual Value*		GetVectorTarget(Value* which, BOOL asController) = 0;
	virtual INode*		GetVectorNode(Value* which) = 0;

	virtual float		GetScalarValue(Value* which, TimeValue t) = 0;
	virtual Point3		GetVectorValue(Value* which, TimeValue t) = 0;
	virtual FPValue		GetVarValue(TSTR &name, TimeValue t) = 0;

	virtual int			GetScalarType(Value* which) = 0;
	virtual int			GetVectorType(Value* which) = 0;

	virtual int			GetValueType(TSTR &name) = 0;

	virtual TSTR		GetVectorName(int index) = 0;
	virtual TSTR		GetScalarName(int index) = 0;

	virtual int			GetScalarIndex(TSTR &name) = 0;
	virtual int			GetVectorIndex(TSTR &name) = 0;
};

#endif __IEXPRCTRL__H