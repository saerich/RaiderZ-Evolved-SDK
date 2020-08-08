//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: An expression-based controller
// AUTHOR 1: Don Brittain - created August.24.1995 (Windows 95 debut!)
// AUTHOR 2: Larry.Minton - major rewrite May.14.2005
//***************************************************************************/

#include "ctrl.h"
#include "units.h"
#include "exprlib.h"
#include "texutil.h"	// clamp
#include "notify.h"
#include <stdio.h>
#include "iexprctrl.h"
#include "INodeTransformMonitor.h"
#include "NodeTransformMonitor.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Numbers.h"
#include "MAXScrpt\MAXObj.h"
#include "MAXScrpt\Listener.h"

#include "3dsmaxport.h"

extern HINSTANCE hInstance;

#define EXPR_POS_CONTROL_CNAME		GetString(IDS_DB_POSITION_EXPR)
#define EXPR_P3_CONTROL_CNAME		GetString(IDS_DB_POINT3_EXPR)
#define EXPR_FLOAT_CONTROL_CNAME	GetString(IDS_DB_FLOAT_EXPR)
#define EXPR_SCALE_CONTROL_CNAME	GetString(IDS_DB_SCALE_EXPR)
#define EXPR_ROT_CONTROL_CNAME		GetString(IDS_DB_ROTATION_EXPR)
#define EXPR_CNAME					GetString(IDS_CNAME_EXPR)

#define EXPR_TICKS		0
#define EXPR_SECS		1
#define EXPR_FRAMES		2
#define EXPR_NTIME		3

#define WM_DISABLE_ACCEL WM_USER+3002  // message dialog posts to itself to tell it to disable accelerators

#define NUM_ALLOWED_RECURSIONS 32

class ExprControl;

// for the debug floater window
class ExprDebug : public TimeChangeCallback {
public:
	HWND hWnd;
	ExprControl *ec;
	TimeValue t;
	bool inUpdate;

	ExprDebug(HWND hParent, ExprControl *exprControl);
	~ExprDebug();

	void Invalidate();
	void SetTime(TimeValue tm)	{ t = tm; }
	void Update();
	void Init(HWND hWnd);

	void TimeChanged(TimeValue t)	{ SetTime(t); Update(); }
};

// scalar variables
class SVar {
public:
	SVar()	{ regNum = refID = subNum = -1; offset = 0; }
	TSTR	name;
	int		regNum;	// register number variable is assigned to
	int		refID;	// < 0 means constant, if >= 0 points at entry in the reference tab
	float	val;	// value, if constant
	int		subNum;	// < 0 means direct reference to object from reference tab, if >=0 the subAnim index in object
	int		offset;	// tick offset
};

class VVar {
public:
	VVar()	{ regNum = refID = subNum = -1; offset = 0; }
	TSTR	name;
	int		regNum;	// reg num this var is assigned to
	int		refID;	// < 0 means constant, if >= 0 points at entry in the reference tab
	Point3	val;	// value, if const
	int 	subNum;	// < 0 means direct reference to object from reference tab, if >=0 the subAnim index in object
	int		offset;	// tick offset
};

MakeTab(SVar);
MakeTab(VVar);

class VarRef {
public:
	VarRef()	{ client = NULL; refCt = 0; }
	VarRef(ReferenceTarget *c)	{ client = c; refCt = 1; }
	ReferenceTarget *client;	// the object referenced
	int				refCt;		// number of variables using this reference
};

MakeTab(VarRef);

static BYTE CurrentExprControlVersion = 1;

class ExprControl : public StdControl, public IExprCtrl
{
public:
	int			type;
	Expr		expr;
	int			timeSlots[4];
	int			sRegCt;
	int			vRegCt;
	int			curIndex;
	Point3		curPosVal;
	float		curFloatVal;
	Interval	ivalid;
	Interval	range;
	IObjParam *	ip;
	SVarTab		sVars;
	VVarTab		vVars;
	VarRefTab	refTab;
	TSTR		desc;
	static HFONT	hFixedFont;
	HWND		hDlg;
	ExprDebug	*edbg;

	bool		blockGetNodeName; // RB 3/23/99: See imp of getNodeName()
	bool		doDelayedUpdateVarList;
	bool		delayedUpdateVarListMaintainSelection;
	bool		disabledDueToRecursion;

	bool		bThrowOnError;

	Animatable	*this_client;
	int			this_index;

	TSTR		lastEditedExpr;
	INode		*nodeTMBeingEvaled;

	//flag to avoid recursion
	int			evaluating;
	TimeValue	evaluatingTime;

	void	updRegCt(int val, int type);
	BOOL	dfnVar(int type, TCHAR *buf, int slot, int offset = 0);
	int		getVarCount(int type) { return type == SCALAR_VAR ? sVars.Count() : vVars.Count(); }
	TCHAR * getVarName(int type, int i);
	int		getVarOffset(int type, int i);
	int		getRegNum(int type, int i);
	float	getScalarValue(int i, TimeValue t);
	Point3	getVectorValue(int i, TimeValue t);
	BOOL	assignScalarValue(int i, float val, int offset = TIME_NegInfinity, BOOL fromFPS = FALSE);
	BOOL	assignVectorValue(int i, Point3 &val, int offset = TIME_NegInfinity, BOOL fromFPS = FALSE);
	BOOL	assignController(int type, int i, ReferenceTarget *client, int subNum, int offset = TIME_NegInfinity, BOOL fromFPS = FALSE);
	void	deleteAllVars();
	void    getNodeName(ReferenceTarget *client,TSTR &name);
	void	updateVarList(bool maintainSelection = false, bool delayedUpdate = false);
	void	dropUnusedRefs();

	TSTR	getTargetAsString(int type, int i);

	static void NotifyPreReset(void* param, NotifyInfo*);

	// FPS method helper methods
	int		findIndex(int type, TSTR &name);
	BOOL	assignNode(INode* node, int varIndex, int offset = TIME_NegInfinity);
	BOOL	FindControl(Control* cntrl, ReferenceTarget *owner = NULL);

	// Published Functions
	virtual bool		GetThrowOnError();
	virtual void		SetThrowOnError(bool bOn);

	virtual TSTR		PrintDetails();
	virtual BOOL		SetExpression(TSTR &expression);
	virtual TSTR		GetExpression();
	virtual int			NumScalars();
	virtual int			NumVectors();

	virtual TSTR		GetDescription();
	virtual BOOL		SetDescription(TSTR &expression);

	virtual BOOL		AddScalarTarget(TSTR &name, Value* target, int offset = TIME_NegInfinity, ReferenceTarget *owner = NULL);
	virtual BOOL		AddVectorTarget(TSTR &name, Value* target, int offset = TIME_NegInfinity, ReferenceTarget *owner = NULL); 

	virtual BOOL		AddScalarConstant(TSTR &name, float	val); 
	virtual BOOL		AddVectorConstant(TSTR &name, Point3 point);

	virtual BOOL		AddVectorNode(TSTR &name, INode* node, int offset = TIME_NegInfinity);

	virtual BOOL		SetScalarTarget(Value* which, Value* target, ReferenceTarget *owner = NULL);
	virtual BOOL		SetVectorTarget(Value* which, Value* target, ReferenceTarget *owner = NULL); 
	virtual BOOL		SetVectorNode(Value* which, INode* node);
	virtual BOOL		SetScalarConstant(Value* which, float val); 
	virtual BOOL		SetVectorConstant(Value* which, Point3 point);

	virtual BOOL		DeleteVariable(TSTR &name);
	virtual BOOL		RenameVariable(TSTR &oldName, TSTR &newName);

	virtual BOOL		VariableExists(TSTR &name);
	virtual void		Update();
	virtual TimeValue	GetOffset(TSTR &name);
	virtual BOOL		SetOffset(TSTR &name, TimeValue tick);

	virtual float		GetScalarConstant(Value* which, TimeValue t);
	virtual Value*		GetScalarTarget(Value* which, BOOL asController);
	virtual Point3		GetVectorConstant(Value* which);
	virtual Value*		GetVectorTarget(Value* which, BOOL asController);
	virtual INode*		GetVectorNode(Value* which);

	virtual float		GetScalarValue(Value* which, TimeValue t);
	virtual Point3		GetVectorValue(Value* which, TimeValue t);
	virtual FPValue		GetVarValue(TSTR &name, TimeValue t);

	virtual int			GetScalarType(Value* which);
	virtual int			GetVectorType(Value* which);

	virtual int			GetValueType(TSTR &name);

	virtual TSTR		GetVectorName(int index);
	virtual TSTR		GetScalarName(int index);

	virtual int			GetScalarIndex(TSTR &name);
	virtual int			GetVectorIndex(TSTR &name);

	ExprControl(int type, ExprControl &ctrl, RemapDir& remap);
	ExprControl(int type, BOOL loading);
	~ExprControl();

	// Animatable methods
	int TrackParamsType() { return TRACKPARAMS_WHOLE; }
	
	void DeleteThis() { delete this; }
	int IsKeyable() { return 0; }		
	BOOL IsAnimated() {return TRUE;}
	Interval GetTimeRange(DWORD flags) { return range; }
	void EditTimeRange(Interval range,DWORD flags);
	void Hold();
	void MapKeys( TimeMap *map, DWORD flags );

	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );

	// Animatable's Schematic View methods
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
	bool SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);

	void EditTrackParams(
			TimeValue t,	// The horizontal position of where the user right clicked.
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);

	// Reference methods
	int NumRefs() { return StdControl::NumRefs() + refTab.Count(); }
	ReferenceTarget* GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	void RefDeleted();
	RefTargetHandle Clone(RemapDir& remap) { 
		DbgAssert(false); // should never be here. Should be in the Clone method of a derived class.
		ExprControl *ctrl = new ExprControl(this->type, *this, remap);
		CloneControl(ctrl,remap);
		BaseClone(this, ctrl, remap);
		return ctrl; 
	}		
	BOOL IsRealDependency(ReferenceTarget *rtarg)
	{
		int refID = FindRef(rtarg);
		if (refID < StdControl::NumRefs())
			return StdControl::IsRealDependency(rtarg);

		ReferenceTarget *client = refTab[refID - StdControl::NumRefs()].client;
		return (client && client->SuperClassID() == REF_TARGET_CLASS_ID && 
						  client->ClassID() == NODETRANSFORMMONITOR_CLASS_ID);
	};

	virtual BOOL ShouldPersistWeakRef(RefTargetHandle rtarg) { 
		int refID = FindRef(rtarg);
		if (refID < StdControl::NumRefs())
			return StdControl::ShouldPersistWeakRef(rtarg);

		return TRUE; 
	};

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Control methods
	void Copy(Control *from);
	
	BOOL IsLeaf() { return TRUE; }
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);
	void *CreateTempValue();
	void DeleteTempValue(void *val);
	void ApplyValue(void *val, void *delta);
	void MultiplyValue(void *val, float m);

	// StdControl methods
	void GetAbsoluteControlValue(INode *node,TimeValue t,Point3 *pt,Interval &iv);

	// FPMixinInterface
	///////////////////////////////////////////////////////////////////////////
	virtual BaseInterface* GetInterface(Interface_ID in_interfaceID) { 
		if (in_interfaceID == IID_EXPR_CONTROL) 
			return (IExprCtrl*)this; 
		else 
			return FPMixinInterface::GetInterface(in_interfaceID);
	}

};

HFONT ExprControl::hFixedFont = CreateFont(14,0,0,0,0,0,0,0,0,0,0,0, FIXED_PITCH | FF_MODERN, _T(""));


// Types
#define	VAR_UNKNOWN			-1
#define VAR_SCALAR_TARGET	0
#define	VAR_SCALAR_CONSTANT	1
#define	VAR_VECTOR_TARGET	2
#define	VAR_VECTOR_CONSTANT 3
#define VAR_VECTOR_NODE		4


///////////////////////////////////////////////////////////////////////////////
// Function Publishing descriptor for Expression control interface
///////////////////////////////////////////////////////////////////////////////

static FPInterfaceDesc exprctrl_interface(
    IID_EXPR_CONTROL, _T("IExprCtrl"), 0, NULL, FP_MIXIN,

	IExprCtrl::fnIdSetExpression,	_T("SetExpression"), 0, TYPE_BOOL, 0, 1,
					_T("Expression"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetExpression,	_T("GetExpression"), 0, TYPE_TSTR_BV, 0, 0,

	IExprCtrl::fnIdNumScalars, _T("NumScalars"), 0, TYPE_INT, 0, 0,

	IExprCtrl::fnIdNumVectors, _T("NumVectors"), 0, TYPE_INT, 0 , 0,

	IExprCtrl::fnIdAddScalarTarget, _T("AddScalarTarget"), 0, TYPE_BOOL, 0, 4,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Target"), 0, TYPE_VALUE,
					_T("Offset"), 0, TYPE_TIMEVALUE, f_keyArgDefault, 0,
					_T("Owner"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,

	IExprCtrl::fnIdAddVectorTarget, _T("AddVectorTarget"), 0, TYPE_BOOL, 0, 4,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Target"), 0, TYPE_VALUE,
					_T("Offset"), 0, TYPE_TIMEVALUE, f_keyArgDefault, 0,
					_T("Owner"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,

	IExprCtrl::fnIdAddVectorNode, _T("AddVectorNode"), 0, TYPE_BOOL, 0, 3,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Node"), 0, TYPE_INODE,
					_T("Offset"), 0, TYPE_TIMEVALUE, f_keyArgDefault, 0,

	IExprCtrl::fnIdAddScalarConstant, _T("AddScalarConstant"), 0, TYPE_BOOL, 0, 2,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Constant"), 0, TYPE_FLOAT,

	IExprCtrl::fnIdAddVectorConstant, _T("AddVectorConstant"), 0, TYPE_BOOL, 0, 2,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Constant"), 0, TYPE_POINT3,



	IExprCtrl::fnIdSetScalarTarget, _T("SetScalarTarget"), 0, TYPE_BOOL, 0, 3,
					_T("Which"), 0, TYPE_VALUE,
					_T("Target"), 0, TYPE_VALUE,
					_T("Owner"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,

	IExprCtrl::fnIdSetVectorTarget, _T("SetVectorTarget"), 0, TYPE_BOOL, 0, 3,
					_T("Which"), 0, TYPE_VALUE,
					_T("Target"), 0, TYPE_VALUE,
					_T("Owner"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,

	IExprCtrl::fnIdSetVectorNode, _T("SetVectorNode"), 0, TYPE_BOOL, 0, 2,
					_T("Which"), 0, TYPE_VALUE,
					_T("Node"), 0, TYPE_INODE,

	IExprCtrl::fnIdSetScalarConstant, _T("SetScalarConstant"), 0, TYPE_BOOL, 0, 2,
					_T("Which"), 0, TYPE_VALUE,
					_T("Constant"), 0, TYPE_FLOAT,

	IExprCtrl::fnIdSetVectorConstant, _T("SetVectorConstant"), 0, TYPE_BOOL, 0, 2,
					_T("Which"), 0, TYPE_VALUE,
					_T("Constant"), 0, TYPE_POINT3,



	IExprCtrl::fnIdDeleteVariable, _T("DeleteVariable"), 0, TYPE_BOOL, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdRenameVariable, _T("RenameVariable"), 0, TYPE_BOOL, 0, 2,
					_T("oldName"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("newName"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetDescription,	_T("GetDescription"), 0, TYPE_TSTR_BV, 0, 0,

	IExprCtrl::fnIdSetDescription, _T("SetDescription"), 0, TYPE_BOOL, 0, 1,
					_T("Description"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetOffset, _T("GetOffset"), 0, TYPE_TIMEVALUE, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdSetOffset, _T("SetOffset"), 0, TYPE_BOOL, 0, 2,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,
					_T("Offset"), 0, TYPE_TIMEVALUE,

	IExprCtrl::fnIdUpdate, _T("Update"), 0, TYPE_VOID, 0, 0,

	IExprCtrl::fnIdVariableExists, _T("VariableExists"), 0, TYPE_BOOL, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetScalarConstant, _T("GetScalarConstant"), 0, TYPE_FLOAT, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetScalarTarget, _T("GetScalarTarget"), 0, TYPE_VALUE, 0, 2,
					_T("Which"), 0, TYPE_VALUE,
					_T("asController"), 0, TYPE_BOOL, f_keyArgDefault, FALSE,

	IExprCtrl::fnIdGetVectorConstant, _T("GetVectorConstant"), 0, TYPE_POINT3_BV, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetVectorTarget, _T("GetVectorTarget"), 0, TYPE_VALUE, 0, 2,
					_T("Which"), 0, TYPE_VALUE,
					_T("asController"), 0, TYPE_BOOL, f_keyArgDefault, FALSE,

	IExprCtrl::fnIdGetVectorNode, _T("GetVectorNode"), 0, TYPE_INODE, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetScalarValue, _T("GetScalarValue"), 0, TYPE_FLOAT, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetVectorValue, _T("GetVectorValue"), 0, TYPE_POINT3_BV, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetValue, _T("GetValue"), 0, TYPE_FPVALUE, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetScalarType, _T("GetScalarType"), 0, TYPE_ENUM, IExprCtrl::enumValueType, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetVectorType, _T("GetVectorType"), 0, TYPE_ENUM, IExprCtrl::enumValueType, 0, 1,
					_T("Which"), 0, TYPE_VALUE,

	IExprCtrl::fnIdGetValueType, _T("GetValueType"), 0, TYPE_ENUM, IExprCtrl::enumValueType, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetScalarName, _T("GetScalarName"), 0, TYPE_TSTR_BV, 0, 1,
					_T("Index"), 0, TYPE_INDEX,

	IExprCtrl::fnIdGetVectorName, _T("GetVectorName"), 0, TYPE_TSTR_BV, 0, 1,
					_T("Index"), 0, TYPE_INDEX,

	IExprCtrl::fnIdGetScalarIndex, _T("GetScalarIndex"), 0, TYPE_INDEX, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdGetVectorIndex, _T("GetVectorIndex"), 0, TYPE_INDEX, 0, 1,
					_T("Name"), 0, TYPE_TSTR_BR, f_inOut, FPP_IN_PARAM,

	IExprCtrl::fnIdPrintDetails,	_T("PrintDetails"), 0, TYPE_TSTR_BV, 0, 0,


	properties,
	IExprCtrl::fnIdGetThrowOnError, IExprCtrl::fnIdSetThrowOnError, _T("ThrowOnError"), 0, TYPE_bool,

	enums,
	IExprCtrl::enumValueType, 6,
				_T("unknown"),			VAR_UNKNOWN,
				_T("scalarTarget"),		VAR_SCALAR_TARGET,
				_T("scalarConstant"),	VAR_SCALAR_CONSTANT,
				_T("vectorTarget"),		VAR_VECTOR_TARGET,
				_T("vectorConstant"),	VAR_VECTOR_CONSTANT,
				_T("vectorNode"),		VAR_VECTOR_NODE,
	end
);

///////////////////////////////////////////////////////////////////////////////
// Get Descriptor method for Mixin Interface
///////////////////////////////////////////////////////////////////////////////
FPInterfaceDesc* IExprCtrl::GetDesc()
{
    return &exprctrl_interface;
}

static Class_ID exprPosControlClassID(EXPR_POS_CONTROL_CLASS_ID,0); 
class ExprPosControl : public ExprControl 
{
public:
	ExprPosControl(ExprPosControl &ctrl, RemapDir& remap) : ExprControl(EXPR_POS_CONTROL_CLASS_ID, ctrl, remap) {}
	ExprPosControl(BOOL loading=FALSE) : ExprControl(EXPR_POS_CONTROL_CLASS_ID, loading) {}
	~ExprPosControl() {}
	RefTargetHandle Clone(RemapDir& remap) 
	{
		ExprPosControl *newob = new ExprPosControl(*this, remap); 
		CloneControl(newob,remap); 
		BaseClone(this, newob, remap);
		return(newob);
	}

	void GetClassName(TSTR& s) { s = EXPR_POS_CONTROL_CNAME; }
	Class_ID ClassID() { return exprPosControlClassID; }  
	SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  		
};

static Class_ID exprP3ControlClassID(EXPR_P3_CONTROL_CLASS_ID,0); 
class ExprP3Control : public ExprControl 
{
public:
	ExprP3Control(ExprP3Control &ctrl, RemapDir& remap) : ExprControl(EXPR_P3_CONTROL_CLASS_ID, ctrl, remap) {}
	ExprP3Control(BOOL loading=FALSE) : ExprControl(EXPR_P3_CONTROL_CLASS_ID, loading) {}
	~ExprP3Control() {}
	RefTargetHandle Clone(RemapDir& remap)
	{
		ExprP3Control *newob = new ExprP3Control(*this, remap);
		CloneControl(newob,remap);
		BaseClone(this, newob, remap);
		return(newob);
	}

	void GetClassName(TSTR& s) { s = EXPR_P3_CONTROL_CNAME; }
	Class_ID ClassID() { return exprP3ControlClassID; }  
	SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }  		
};

static Class_ID exprFloatControlClassID(EXPR_FLOAT_CONTROL_CLASS_ID,0); 
class ExprFloatControl : public ExprControl 
{
public:
	ExprFloatControl(ExprFloatControl &ctrl, RemapDir& remap) : ExprControl(EXPR_FLOAT_CONTROL_CLASS_ID, ctrl, remap) {}
	ExprFloatControl(BOOL loading=FALSE) : ExprControl(EXPR_FLOAT_CONTROL_CLASS_ID, loading) {}
	~ExprFloatControl() {}
	RefTargetHandle Clone(RemapDir& remap)
	{
		ExprFloatControl *newob = new ExprFloatControl(*this, remap); 
		CloneControl(newob,remap); 
		BaseClone(this, newob, remap); 
		return(newob);
	}

	void GetClassName(TSTR& s) { s = EXPR_FLOAT_CONTROL_CNAME; }
	Class_ID ClassID() { return exprFloatControlClassID; }  
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }  		
};

static Class_ID exprScaleControlClassID(EXPR_SCALE_CONTROL_CLASS_ID,0); 
class ExprScaleControl : public ExprControl 
{
public:
	ExprScaleControl(ExprScaleControl &ctrl, RemapDir& remap) : ExprControl(EXPR_SCALE_CONTROL_CLASS_ID, ctrl, remap) {}
	ExprScaleControl(BOOL loading=FALSE) : ExprControl(EXPR_SCALE_CONTROL_CLASS_ID, loading) {}
	~ExprScaleControl() {}
	RefTargetHandle Clone(RemapDir& remap)
	{
		ExprScaleControl *newob = new ExprScaleControl(*this, remap); 
		CloneControl(newob,remap); 
		BaseClone(this, newob, remap); 
		return(newob);
	}

	void GetClassName(TSTR& s) { s = EXPR_SCALE_CONTROL_CNAME; }
	Class_ID ClassID() { return exprScaleControlClassID; }  
	SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; }  		
};

static Class_ID exprRotControlClassID(EXPR_ROT_CONTROL_CLASS_ID,0); 
class ExprRotControl : public ExprControl 
{
public:
	ExprRotControl(ExprRotControl &ctrl, RemapDir& remap) : ExprControl(EXPR_ROT_CONTROL_CLASS_ID, ctrl, remap) {}
	ExprRotControl(BOOL loading=FALSE) : ExprControl(EXPR_ROT_CONTROL_CLASS_ID, loading) {}
	~ExprRotControl() {}
	RefTargetHandle Clone(RemapDir& remap)
	{
		ExprRotControl *newob = new ExprRotControl(*this, remap); 
		CloneControl(newob,remap); 
		BaseClone(this, newob, remap); 
		return(newob);
	}		

	void GetClassName(TSTR& s) { s = EXPR_ROT_CONTROL_CNAME; }
	Class_ID ClassID() { return Class_ID(EXPR_ROT_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		
};

//********************************************************
// EXPRESSION CONTROL
//********************************************************
class ExprPosClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&exprctrl_interface); return new ExprPosControl(loading); }
	const TCHAR *	ClassName() { return EXPR_POS_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return exprPosControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprPosClassDesc exprPosCD;
ClassDesc* GetExprPosCtrlDesc() {return &exprPosCD;}

class ExprP3ClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&exprctrl_interface); return new ExprP3Control(loading); }
	const TCHAR *	ClassName() { return EXPR_P3_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return exprP3ControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprP3ClassDesc exprP3CD;
ClassDesc* GetExprP3CtrlDesc() {return &exprP3CD;}

class ExprFloatClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&exprctrl_interface); return new ExprFloatControl(loading); }
	const TCHAR *	ClassName() { return EXPR_FLOAT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return exprFloatControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprFloatClassDesc exprFloatCD;
ClassDesc* GetExprFloatCtrlDesc() {return &exprFloatCD;}

class ExprScaleClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&exprctrl_interface); return new ExprScaleControl(loading); }
	const TCHAR *	ClassName() { return EXPR_SCALE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return exprScaleControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprScaleClassDesc exprScaleCD;
ClassDesc* GetExprScaleCtrlDesc() {return &exprScaleCD;}

class ExprRotClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { AddInterface(&exprctrl_interface); return new ExprRotControl(loading); }
	const TCHAR *	ClassName() { return EXPR_ROT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return exprRotControlClassID; }
	const TCHAR* 	Category() { return _T("");  }
};
static ExprFloatClassDesc exprRotCD;
ClassDesc* GetExprRotCtrlDesc() {return &exprRotCD;}

// This class is used to patch up references at the end of a clone operation
// an instance of this class is created and registered with the RemapDir if
// a target has not been cloned when the controller is cloned. The Proc
// will see if the target has been cloned at the end of the clone operation, 
// and replace the target ref with the clone ref if so.
class PostPatchClient : public PostPatchProc {
	ReferenceTarget* cont;
	ReferenceTarget* client;

public:
	PostPatchClient(ReferenceTarget* cont, ReferenceTarget* client) : cont(cont), client(client)  {}
	virtual int Proc(RemapDir&);
};

int PostPatchClient::Proc(RemapDir& remap)
{
	// Make sure we still hold a ref to the original target. In some cases we won't. For example,
	// if controller is on pos track, and client is part of object channel, and you shift-drag to 
	// clone, object ref is cloned by NodeSelectionProcessor::ChangeCloneType and it then remaps 
	// references, and then the PostPatchProcs are called. So by the time we are called, the original
	// client has been replaced with its clone.
	int refNum = cont->FindRef(client); 
	if (refNum >= 0)
	{
		ReferenceTarget *newClient = remap.FindMapping(client); // see if target was cloned
		if (newClient != NULL && client != newClient) // yep, update controller
			cont->ReplaceReference(refNum, newClient);
	}
	return TRUE;
}

static BOOL OKP3Control(SClass_ID cid) {
	if (cid==CTRL_POINT3_CLASS_ID || cid==CTRL_POS_CLASS_ID || cid==CTRL_POSITION_CLASS_ID )
		return 1;
	return 0;
}

ExprControl::ExprControl(int t, ExprControl &ctrl, RemapDir &remap)
{
	int i, j, ct, slot;
	TCHAR *cp;

	blockGetNodeName = false;
	doDelayedUpdateVarList = false;
	delayedUpdateVarListMaintainSelection = true;

	evaluating = 0;
	evaluatingTime = TIME_NegInfinity;
	nodeTMBeingEvaled = NULL;

	type = t;
	hDlg = NULL;
	edbg = NULL;

	range = ctrl.range;
	curPosVal = ctrl.curPosVal;
	curFloatVal = ctrl.curFloatVal;
	ivalid = ctrl.ivalid;
	sRegCt = vRegCt = 0;
	desc = ctrl.desc;
	disabledDueToRecursion = ctrl.disabledDueToRecursion;
	bThrowOnError = ctrl.bThrowOnError;

	ct = ctrl.expr.getVarCount(SCALAR_VAR);
	for(i = 0; i < ct; i++) {
		cp = ctrl.expr.getVarName(SCALAR_VAR, i);
		updRegCt(slot = expr.defVar(SCALAR_VAR, cp), SCALAR_VAR);
		if(slot == -1)	// variable already defined
			continue;
		if(_tcscmp(cp, _T("T")) == 0)
			timeSlots[EXPR_TICKS] = slot;
		else if(_tcscmp(cp, _T("S")) == 0)
			timeSlots[EXPR_SECS] = slot;
		else if(_tcscmp(cp, _T("F")) == 0)
			timeSlots[EXPR_FRAMES] = slot;
		else if(_tcscmp(cp, _T("NT")) == 0)
			timeSlots[EXPR_NTIME] = slot;
		else {
			dfnVar(SCALAR_VAR, cp, slot, ctrl.getVarOffset(SCALAR_VAR, j=i-4));

			if(ctrl.sVars[j].refID < 0)
				assignScalarValue(slot-4, ctrl.sVars[j].val);	// -4:  001205  --prs.
			else
			{
				ReferenceTarget *client = ctrl.refTab[ctrl.sVars[j].refID].client;
				if (client)
				{
					// see if client already cloned. If so, point at it. If not, we don't want to 
					// force a clone, so we will register a PostPatchProc to check again at the end
					// of the clone, and update our ref to the clone if present.
					ReferenceTarget *newClient = remap.FindMapping(client);
					if (newClient)
						client = newClient;
					else
						remap.AddPostPatchProc(new PostPatchClient(this, client), true);
				}
				assignController(SCALAR_VAR, slot-4, client, ctrl.sVars[j].subNum);
			}
		}
	}

	ct = ctrl.expr.getVarCount(VECTOR_VAR);
	for(i = 0; i < ct; i++) {
		cp = ctrl.expr.getVarName(VECTOR_VAR, i);
		updRegCt(slot = expr.defVar(VECTOR_VAR, cp), VECTOR_VAR);
		dfnVar(VECTOR_VAR, cp, slot, ctrl.getVarOffset(VECTOR_VAR, i));

		if(ctrl.vVars[i].refID < 0)
			assignVectorValue(slot, ctrl.vVars[i].val);
		else
		{
			ReferenceTarget *client = ctrl.refTab[ctrl.vVars[i].refID].client;
			// If client is a NodeTransformMonitor, always want to clone it. The NodeTransformMonitor
			// will take of handling whether the node it points to is cloned or not.
			if (client && client->SuperClassID() == REF_TARGET_CLASS_ID && client->ClassID() == NODETRANSFORMMONITOR_CLASS_ID)
				client = remap.CloneRef(client);
			else if (client)
			{
				// see if client already cloned. If so, point at it. If not, we don't want to 
				// force a clone, so we will register a PostPatchProc to check again at the end
				// of the clone, and update our ref to the clone if present.
				ReferenceTarget *newClient = remap.FindMapping(client);
				if (newClient)
					client = newClient;
				else
					remap.AddPostPatchProc(new PostPatchClient(this, client), true);
			}
			assignController(VECTOR_VAR, slot, client, ctrl.vVars[i].subNum);
	}
	}

	cp = ctrl.expr.getExprStr();
	expr.load(cp);

	if (ctrl.TestAFlag(A_ORT_DISABLED))
		SetAFlag(A_ORT_DISABLED);
}

ExprControl::ExprControl(int t, BOOL loading) 
{
	blockGetNodeName = false;
	doDelayedUpdateVarList = false;
	delayedUpdateVarListMaintainSelection = true;
	disabledDueToRecursion = false;
	bThrowOnError = true;

	evaluating = 0;
	evaluatingTime = TIME_NegInfinity;
	nodeTMBeingEvaled = NULL;

	type = t;
	range.Set(GetAnimStart(), GetAnimEnd());
	curPosVal = Point3(0,0,0);
	curFloatVal = 0.0f;
	ivalid.SetEmpty();
	sRegCt = vRegCt = 0;
	updRegCt(timeSlots[EXPR_TICKS] = expr.defVar(SCALAR_VAR, _T("T")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_SECS] = expr.defVar(SCALAR_VAR, _T("S")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_FRAMES] = expr.defVar(SCALAR_VAR, _T("F")), SCALAR_VAR);
	updRegCt(timeSlots[EXPR_NTIME] = expr.defVar(SCALAR_VAR, _T("NT")), SCALAR_VAR);
	switch(type = t) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		expr.load(_T("[ 0, 0, 0 ]"));
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		expr.load(_T("[ 1, 1, 1 ]"));
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		expr.load(_T("0"));
		break;
	case EXPR_ROT_CONTROL_CLASS_ID:
		expr.load(_T("{ [ 0, 0, 0 ], 0 }"));
		break;
	}
	hDlg = NULL;
	edbg = NULL;

	if (!loading && GetCOREInterface8()->GetControllerOverrideRangeDefault())
		SetAFlag(A_ORT_DISABLED);
}

ExprControl::~ExprControl()
{
	if (hDlg)
		DestroyWindow(hDlg);
	deleteAllVars();
	DeleteAllRefsFromMe();
}

void ExprControl::updRegCt(int val, int type)
{
	if(type == SCALAR_VAR) {
		if(val+1 > sRegCt)
			sRegCt = val+1;
	}
	else {
		if(val+1 > vRegCt)
			vRegCt = val+1;
	}
}

BOOL ExprControl::dfnVar(int type, TCHAR *buf, int slot, int offset)
{
	int i;
	if(type == SCALAR_VAR) {
		SVar sv;
		sv.regNum = slot;
		sv.offset = offset;
		sv.val = 0.0f;
		i = sVars.Append(1, &sv, 4);
		sVars[i].name = buf;
	}
	else {
		VVar vv;
		vv.regNum = slot;
		vv.offset = offset;
		vv.val.x =
		vv.val.y =
		vv.val.z = 0.0f;
		i = vVars.Append(1, &vv, 4);
		vVars[i].name = buf;
	}
	return TRUE;
}

void ExprControl::deleteAllVars()
{
	int i, ct;

	ct = sVars.Count();
	for(i = 0; i < ct; i++)
		delete sVars[i].name;
	sVars.SetCount(0);
	ct = vVars.Count();
	for(i = 0; i < ct; i++)
		delete vVars[i].name;
	vVars.SetCount(0);
	ct = refTab.Count();
	for(i = 0; i < ct; i++) 
		refTab[i].refCt = 0;
}

TCHAR *ExprControl::getVarName(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i >= 0 && i < sVars.Count())
			return sVars[i].name;
	}
	else {
		if(i >= 0 && i < vVars.Count())
			return vVars[i].name;
	}
	return NULL;
}

int ExprControl::getVarOffset(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i >= 0 && i < sVars.Count())
			return sVars[i].offset;
	}
	else {
		if(i >= 0 && i < vVars.Count())
			return vVars[i].offset;
	}
	return 0;
}

int	ExprControl::getRegNum(int type, int i)
{
	if(type == SCALAR_VAR) {
		if(i >= 0 && i < sVars.Count())
			return sVars[i].regNum;
	}
	else {
		if(i >= 0 && i < vVars.Count())
			return vVars[i].regNum;
	}
	return -1;
}

float ExprControl::getScalarValue(int i, TimeValue t)
{
	float fval = 0.0f;
	Interval iv;
	ReferenceTarget *client = NULL;

	if((i < sVars.Count()) && (i >= 0)) {
		if(sVars[i].refID < 0 || (client = refTab[sVars[i].refID].client) == NULL)
			fval = sVars[i].val;
		else {
			int subNum = sVars[i].subNum;
			ReferenceTarget *ref = (ReferenceTarget*)client->SubAnim(subNum);
			if (ref == NULL) {
				DbgAssert(false);
				fval = sVars[i].val;
				return fval;
			}
			TimeValue tEval = t+sVars[i].offset;
			if (ref->SuperClassID() == CTRL_USERTYPE_CLASS_ID) {
				if (client->SuperClassID()==PARAMETER_BLOCK_CLASS_ID) {
					IParamBlock *iparam = (IParamBlock*)client;
					int pi = iparam->AnimNumToParamNum(subNum);
					ParamType type = iparam->GetParameterType(pi);
					if (type == TYPE_FLOAT)
						iparam->GetValue(pi,tEval,fval,iv);
					else if (type == TYPE_INT || type == TYPE_BOOL) {
						int ival = 0;
						iparam->GetValue(pi,tEval,ival,iv);
						fval = (float)ival;
					}
					else {
						DbgAssert(false);
						fval = sVars[i].val;
					}
				} 
				else if (client->SuperClassID()==PARAMETER_BLOCK2_CLASS_ID) {
					IParamBlock2 *iparam = (IParamBlock2*)client;
					int tabIndex;
					int pi = iparam->AnimNumToParamNum(subNum, tabIndex);
					const ParamDef& pd = iparam->GetDesc()->paramdefs[pi];
					if (is_tab(pd.type) && tabIndex == -1) {
						DbgAssert(false);
						fval = sVars[i].val;
					}
					ParamType2 type = base_type(pd.type);
					if (type == TYPE_FLOAT || type == TYPE_ANGLE || type == TYPE_PCNT_FRAC || type == TYPE_WORLD || type == TYPE_COLOR_CHANNEL)
						iparam->GetValue(pd.ID, tEval, fval, iv, tabIndex);
					else if (type == TYPE_INT || type == TYPE_BOOL || type == TYPE_INDEX || type == TYPE_RADIOBTN_INDEX) {
						int ival = 0;
						iparam->GetValue(pd.ID, tEval, ival, iv, tabIndex);
						fval = (float)ival;
					}
					else {
						DbgAssert(false);
						fval = sVars[i].val;
					}
				}
			}
			else {
				Control *c = (Control*)ref;
				if(c&&(c->SuperClassID()==CTRL_FLOAT_CLASS_ID)) {
					c->GetValue(tEval, &fval, iv);
				}
				else {
					DbgAssert(false);
					fval = sVars[i].val;
				}
			}
		}
		sVars[i].val = fval;
	}
	return fval;
}

Point3 ExprControl::getVectorValue(int i, TimeValue t)
{
	Point3 vval;
	Interval iv;

	if((i < vVars.Count()) && (i >= 0)) {
		int subNum = vVars[i].subNum;
		TimeValue tEval = t+vVars[i].offset;
		ReferenceTarget *client = NULL;

		if(vVars[i].refID < 0 || (client = refTab[vVars[i].refID].client) == NULL)
			vval = vVars[i].val;
		else if (subNum < 0) {	// this is referencing a node, not a controller! DB 1/98
			INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
			INode *theNode = NULL;
			if (ntm) 
				theNode = ntm->GetNode();
			if (theNode == NULL) {
				DbgAssert(false);
				vval = vVars[i].val;
				return vval;
			}
			GetAbsoluteControlValue(theNode,tEval,&vval, iv);
		} 
		else {
			ReferenceTarget *ref = NULL;
			if (client) 
				ref = (ReferenceTarget*)client->SubAnim(subNum);
			if (ref == NULL) {
				DbgAssert(false);
				vval = vVars[i].val;
				return vval;
			}
			if (ref->SuperClassID() == CTRL_USERTYPE_CLASS_ID) {
				if (client->SuperClassID()==PARAMETER_BLOCK_CLASS_ID) {
					IParamBlock *iparam = (IParamBlock*)client;
					int pi = iparam->AnimNumToParamNum(subNum);
					ParamType type = iparam->GetParameterType(pi);
					if (type == TYPE_RGBA || type == TYPE_POINT3)
						iparam->GetValue(pi,tEval,vval,iv);
					else {
						DbgAssert(false);
						vval = vVars[i].val;
					}
				} 
				else if (client->SuperClassID()==PARAMETER_BLOCK2_CLASS_ID) {
					IParamBlock2 *iparam = (IParamBlock2*)client;
					int tabIndex;
					int pi = iparam->AnimNumToParamNum(subNum, tabIndex);
					const ParamDef& pd = iparam->GetDesc()->paramdefs[pi];
					if (is_tab(pd.type) && tabIndex == -1) {
						DbgAssert(false);
						vval = vVars[i].val;
					}
					ParamType2 type = base_type(pd.type);
					if (type == TYPE_RGBA || type == TYPE_POINT3 || type == TYPE_HSV)
						iparam->GetValue(pd.ID, tEval, vval, iv, tabIndex);
					else {
						DbgAssert(false);
						vval = vVars[i].val;
					}
				}
			}
			else {
				Control *c = (Control*)ref;
				if(c&&OKP3Control(c->SuperClassID()))
					c->GetValue(tEval, &vval, iv);
				else {
					DbgAssert(false);
					vval = vVars[i].val;
}
	}
}
		vVars[i].val = vval;
	}
	return vval;
}

class FloatFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client,int subNum)
	{ 
		SClass_ID cls;
		if (client && client->SuperClassID()==PARAMETER_BLOCK_CLASS_ID) {
			IParamBlock *iparam = (IParamBlock*)client;
			cls = iparam->GetAnimParamControlType(subNum);
		} 
		else if (client && client->SuperClassID()==PARAMETER_BLOCK2_CLASS_ID) {
			IParamBlock2 *iparam = (IParamBlock2*)client;
			cls = iparam->GetAnimParamControlType(subNum);
		}
		else cls = anim->SuperClassID();

		return cls == CTRL_FLOAT_CLASS_ID; 
	}
};

class VectorFilter : public TrackViewFilter {
public:
	BOOL proc(Animatable *anim, Animatable *client,int subNum)
			{ 
			if (anim->SuperClassID() == BASENODE_CLASS_ID) {
				INode *node = (INode*)anim;
				return !node->IsRootNode();
				}

				SClass_ID cls;
				if (client && client->SuperClassID()==PARAMETER_BLOCK_CLASS_ID) {
					IParamBlock *iparam = (IParamBlock*)client;
					cls = iparam->GetAnimParamControlType(subNum);
				} 
				else if (client && client->SuperClassID()==PARAMETER_BLOCK2_CLASS_ID) {
					IParamBlock2 *iparam = (IParamBlock2*)client;
					cls = iparam->GetAnimParamControlType(subNum);
				}
				else cls = anim->SuperClassID();

				return OKP3Control(cls); 
			}
};

BOOL ExprControl::assignController(int type, int varIndex, ReferenceTarget *client, int subNum, int offset, BOOL fromFPS)
{
	if (!client) return FALSE;

	if (client->SuperClassID() == BASENODE_CLASS_ID)
		return assignNode((INode*)client, varIndex, offset);

	int i, ct;

	ct = refTab.Count();
	for(i = 0; i < ct; i++) {
		if(refTab[i].client == client) {
			refTab[i].refCt++;
			break;
		}
	}
	if(i >= ct) {
		VarRef vr(NULL);
		i = refTab.Append(1, &vr, 4);
		if(ReplaceReference( StdControl::NumRefs()+i, client) != REF_SUCCEED) {
			refTab.Delete(i, 1);
			if (hDlg && !fromFPS) {
				MessageBox(hDlg, GetString(IDS_DB_CIRCULAR_DEPENDENCY), GetString(IDS_DB_CANT_ASSIGN), 
					MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_OK);
			}
			return FALSE;
		}
	}
	if(type == SCALAR_VAR) {
		if(varIndex >= sVars.Count()) {
			refTab[i].refCt--;
			return FALSE;
		}

		if (sVars[varIndex].refID != -1)
			refTab[sVars[varIndex].refID].refCt--;

		sVars[varIndex].refID = i;
		sVars[varIndex].subNum = subNum;
		if (offset != TIME_NegInfinity)
			sVars[varIndex].offset = offset;
		if (fromFPS && hDlg)
			updateVarList(true,true);
		dropUnusedRefs();
		return TRUE;
	}
	else if(type == VECTOR_VAR) {
		if(varIndex >= vVars.Count()) {
			refTab[i].refCt--;
			return FALSE;
		}

		if (vVars[varIndex].refID != -1)
			refTab[vVars[varIndex].refID].refCt--;

		vVars[varIndex].refID = i;
		vVars[varIndex].subNum = subNum;
		if (offset != TIME_NegInfinity)
			vVars[varIndex].offset = offset;
		if (fromFPS && hDlg)
			updateVarList(true,true);
		dropUnusedRefs();
		return TRUE;
	}
	return FALSE;
}

// This patch is here to convert number strings where the decimal point
// is a comma into numbers with a '.' for a decimal point.
// This is needed because the expression object only parses the '.' notation
TCHAR *UglyPatch(TCHAR *buf)
{
	TCHAR *cp = buf;
	while(*cp) {
		if( *cp == _T(','))
			*cp = _T('.');
		else if (*cp == _T(';'))
			*cp = _T(',');
		cp++;
	}
	return buf;
}

void ExprControl::Copy(Control *from)
{
	int i, j, ct, slot;
	TCHAR *cp;
	TCHAR buf[256];

	if (from == this || from == NULL) return;

	if (from->ClassID()==ClassID()) {
		ExprControl *ctrl = (ExprControl*)from;
		if(type != ctrl->type)
			goto dropOut;
		curPosVal = ctrl->curPosVal;
		curFloatVal = ctrl->curFloatVal;
		ivalid = ctrl->ivalid;
		range = ctrl->range;
		disabledDueToRecursion = ctrl->disabledDueToRecursion;

		desc = ctrl->desc;
//		sRegCt = vRegCt = 0;
		ct = ctrl->expr.getVarCount(SCALAR_VAR);
		for(i = 0; i < ct; i++) {
			cp = ctrl->expr.getVarName(SCALAR_VAR, i);
			updRegCt(slot = expr.defVar(SCALAR_VAR, cp), SCALAR_VAR);
			if(slot == -1)	// variable already defined
				continue;
			if(_tcscmp(cp, _T("T")) == 0)
				timeSlots[EXPR_TICKS] = slot;
			else if(_tcscmp(cp, _T("S")) == 0)
				timeSlots[EXPR_SECS] = slot;
			else if(_tcscmp(cp, _T("F")) == 0)
				timeSlots[EXPR_FRAMES] = slot;
			else if(_tcscmp(cp, _T("NT")) == 0)
				timeSlots[EXPR_NTIME] = slot;
			else {
				dfnVar(SCALAR_VAR, cp, slot, ctrl->getVarOffset(SCALAR_VAR, j=i-4));

				if(ctrl->sVars[j].refID < 0)
					assignScalarValue(slot, ctrl->sVars[j].val);
				else
					assignController(SCALAR_VAR, slot-4, ctrl->refTab[ctrl->sVars[j].refID].client, ctrl->sVars[j].subNum);
			}
		}
		ct = ctrl->expr.getVarCount(VECTOR_VAR);
		for(i = 0; i < ct; i++) {
			cp = ctrl->expr.getVarName(VECTOR_VAR, i);
			updRegCt(slot = expr.defVar(VECTOR_VAR, cp), VECTOR_VAR);
			dfnVar(VECTOR_VAR, cp, slot, ctrl->getVarOffset(VECTOR_VAR, i));

			if(ctrl->vVars[i].refID < 0)
				assignVectorValue(slot, ctrl->vVars[i].val);
			else
				assignController(VECTOR_VAR, slot, ctrl->refTab[ctrl->vVars[i].refID].client, ctrl->vVars[i].subNum);
		}
		expr.load(ctrl->expr.getExprStr());
	} 
	else {
dropOut:
		switch(type) {
		case EXPR_POS_CONTROL_CLASS_ID:
		case EXPR_P3_CONTROL_CLASS_ID:
			from->GetValue(0,&curPosVal,ivalid);
			_stprintf(buf, _T("[ %g; %g; %g ]"), curPosVal.x, curPosVal.y, curPosVal.z);
			expr.load(UglyPatch(buf));
			break;
		case EXPR_FLOAT_CONTROL_CLASS_ID:
			from->GetValue(0,&curFloatVal,ivalid);
			_stprintf(buf, _T("%g"), curFloatVal);
			expr.load(UglyPatch(buf));
			break;
		// should deal with SCALE and ROT types, but punt for now...
		}
	}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

void ExprControl::GetAbsoluteControlValue(
		INode *node,TimeValue t,Point3 *pt,Interval &iv)
{
	INode *oldNode = nodeTMBeingEvaled;
	nodeTMBeingEvaled = node;
	Matrix3 tm = node->GetNodeTM(t,&iv);
	*pt = tm.GetTrans();
	nodeTMBeingEvaled = oldNode;
}


void ExprControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	if (disabledDueToRecursion) 
	{
		if (method==CTRL_ABSOLUTE)
		{
			switch(type) {
			case EXPR_POS_CONTROL_CLASS_ID:
			case EXPR_P3_CONTROL_CLASS_ID:
				*((Point3*)val) = Point3::Origin;
				break;
			case EXPR_SCALE_CONTROL_CLASS_ID:
				*((ScaleValue *)val) = Point3(1,1,1);
				break;
			case EXPR_FLOAT_CONTROL_CLASS_ID:
				*((float *)val) = 0.0f;
				break;
				}			
			}
		return;
				}			

	// check for recursion 
	evaluating += 1;
	evaluatingTime = t;

	if((evaluating >= NUM_ALLOWED_RECURSIONS && t == evaluatingTime))
	{
		disabledDueToRecursion = true;
		TSTR msg; 
		if (nodeTMBeingEvaled)
			msg.printf(GetString(IDS_LAM_EXPR_ILLEGAL_SELF_REFERENCE_NODE),nodeTMBeingEvaled->GetName());
		else
			msg = GetString(IDS_LAM_EXPR_ILLEGAL_SELF_REFERENCE); 
		if (GetCOREInterface()->GetQuietMode()) 
			GetCOREInterface()->Log()->LogEntry(SYSLOG_WARN,NO_DIALOG,GetString(IDS_LAM_EXPR_ILLEGAL_CYCLE),msg);
		else
	{
			MessageBox(GetCOREInterface()->GetMAXHWnd(),msg,GetString(IDS_LAM_EXPR_ILLEGAL_CYCLE), 
				MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			EditTrackParams(t, NULL, NULL, GetCOREInterface()->GetMAXHWnd(), (IObjParam*)GetCOREInterface(),0);
		}
		GetValueLocalTime(t, val, valid, method);
		evaluating -= 1;
		return;
	}

	if (ivalid.InInterval(t))
	{
		switch(type) {
		case EXPR_POS_CONTROL_CLASS_ID:
			if (method==CTRL_RELATIVE) {
				Matrix3 *mat = (Matrix3*)val;
				mat->PreTranslate(curPosVal);
			}
			else
				*((Point3*)val) = curPosVal;
			break;
		case EXPR_P3_CONTROL_CLASS_ID:
			if (method==CTRL_RELATIVE) {
				Point3 *p = (Point3 *)val;
				*p += curPosVal;
			}
			else
				*((Point3*)val) = curPosVal;
			break;
		case EXPR_SCALE_CONTROL_CLASS_ID:
			if (method==CTRL_RELATIVE) {
				Matrix3 *mat = (Matrix3*)val;
				mat->Scale(curPosVal);
			} 
			else 
				*((ScaleValue *)val) = ScaleValue(curPosVal);
			break;
		case EXPR_FLOAT_CONTROL_CLASS_ID:
			if (method==CTRL_RELATIVE)
				*((float *)val) += curFloatVal;
			else
				*((float *)val) = curFloatVal;
			break;
	}
		evaluating -= 1;
		valid &= ivalid;
		return;
	}

	int i;
	float *sregs = new float[sRegCt];
	Point3 *vregs = new Point3[vRegCt];

	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
	case EXPR_SCALE_CONTROL_CLASS_ID:
		if(expr.getExprType() == VECTOR_EXPR) {
			sregs[timeSlots[EXPR_TICKS]] = (float)t;
			sregs[timeSlots[EXPR_SECS]] = (float)t/4800.0f;
			sregs[timeSlots[EXPR_FRAMES]] = (float)t/GetTicksPerFrame();
			float normalizedTime = (float)(t-range.Start()) / (float)(range.End()-range.Start());
			if (TestAFlag(A_ORT_DISABLED))
				normalizedTime = clamp(normalizedTime, 0.0f, 1.0f);
			sregs[timeSlots[EXPR_NTIME]] = normalizedTime;
			for(i = 0; i < sVars.Count(); i++) {
				float fval = getScalarValue(i, t);
				sregs[sVars[i].regNum] = fval;
			}
			for(i = 0; i < vVars.Count(); i++) {
				Point3 vval = getVectorValue(i, t);
				vregs[vVars[i].regNum] = vval;
			}
			if(disabledDueToRecursion || expr.eval((float *)&curPosVal, sRegCt, sregs, vRegCt, vregs))
				curPosVal = type == EXPR_SCALE_CONTROL_CLASS_ID ? Point3(1,1,1) : Point3(0,0,0);
		}
		else
			curPosVal = type == EXPR_SCALE_CONTROL_CLASS_ID ? Point3(1,1,1) : Point3(0,0,0);
		if (method==CTRL_RELATIVE) {
			if (disabledDueToRecursion)
				break;
			if(type == EXPR_POS_CONTROL_CLASS_ID) {
		  		Matrix3 *mat = (Matrix3*)val;
				mat->PreTranslate(curPosVal);  // DS 9/23/96
			}
			else if(type == EXPR_SCALE_CONTROL_CLASS_ID) {
		  		Matrix3 *mat = (Matrix3*)val;
				ApplyScaling(*mat, curPosVal);
			}
			else {
		  		Point3 *p = (Point3 *)val;
				*p += curPosVal;
			}
		} 
		else {
			if(type == EXPR_SCALE_CONTROL_CLASS_ID)
				*((ScaleValue *)val) = ScaleValue(curPosVal);
			else
				*((Point3*)val) = curPosVal;
		}
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		if(expr.getExprType() == SCALAR_EXPR) {
			sregs[timeSlots[EXPR_TICKS]] = (float)t;
			sregs[timeSlots[EXPR_SECS]] = (float)t/4800.0f;
			sregs[timeSlots[EXPR_FRAMES]] = (float)t/GetTicksPerFrame();
			float normalizedTime = (float)(t-range.Start()) / (float)(range.End()-range.Start());
			if (TestAFlag(A_ORT_DISABLED))
				normalizedTime = clamp(normalizedTime, 0.0f, 1.0f);
			sregs[timeSlots[EXPR_NTIME]] = normalizedTime;
			for(i = 0; i < sVars.Count(); i++) {
				float fval = getScalarValue(i, t);
				sregs[sVars[i].regNum] = fval;
			}
			for(i = 0; i < vVars.Count(); i++) {
				Point3 vval = getVectorValue(i, t);
				vregs[vVars[i].regNum] = vval;
				}
			if(disabledDueToRecursion || expr.eval((float *)&curFloatVal, sRegCt, sregs, vRegCt, vregs))
				curFloatVal = 0.0f;
		}
		else
			curFloatVal = 0.0f;
		if (method==CTRL_RELATIVE) {
			if (disabledDueToRecursion)
				break;
			*((float *)val) += curFloatVal;
		} 
		else {
			*((float *)val) = curFloatVal;
		}
	    break;
	}
	delete [] sregs;
	delete [] vregs;

	ivalid.SetInstant(t);
	valid &= ivalid;

	evaluating -= 1;
}

void *ExprControl::CreateTempValue()
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		return new Point3;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		return new ScaleValue;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		return new float;
	}
	return NULL;
}

void ExprControl::DeleteTempValue(void *val)
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		delete (Point3 *)val;
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		delete (ScaleValue *)val;
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		delete (float *)val;
		break;
	}
}

void ExprControl::ApplyValue(void *val, void *delta)
{
	Matrix3 *mat;
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
  		mat = (Matrix3*)val;
//		mat->SetTrans(mat->GetTrans()+*((Point3 *)delta));		
		mat->PreTranslate(*((Point3 *)delta));  //DS 9/23/96
		break;
	case EXPR_P3_CONTROL_CLASS_ID:
		*((Point3 *)val) += *((Point3 *)delta);
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		*((float *)val) += *((float *)delta);
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		Matrix3 *mat = (Matrix3*)val;
		ScaleValue *s = (ScaleValue*)delta;
		ApplyScaling(*mat,*s);
		break;
	}
}

void ExprControl::MultiplyValue(void *val, float m)
{
	switch(type) {
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		*((Point3 *)val) *= m;
		break;
	case EXPR_SCALE_CONTROL_CLASS_ID:
		*((ScaleValue *)val) *= m;
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		*((float *)val) *= m;
		break;
	}
}

void ExprControl::Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int etype)
{
	if(type == EXPR_FLOAT_CONTROL_CLASS_ID) {
		float fval0, fval1, fval2, res = 0.0f;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&fval0,valid);
				GetValueLocalTime(range.Start()+1,&fval1,valid);
				res = LinearExtrapolate(range.Start(),t,fval0,fval1,fval0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&fval0,valid);
				GetValueLocalTime(range.End(),&fval1,valid);
				res = LinearExtrapolate(range.End(),t,fval0,fval1,fval1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&fval0,valid);
				res = IdentityExtrapolate(range.Start(),t,fval0);
			} 
			else {
				GetValueLocalTime(range.End(),&fval0,valid);
				res = IdentityExtrapolate(range.End(),t,fval0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&fval0,valid);
			GetValueLocalTime(range.End(),&fval1,valid);
			GetValueLocalTime(CycleTime(range,t),&fval2,valid);
			res = RepeatExtrapolate(range,t,fval0,fval1,fval2);			
			break;
		}
		valid.Set(t,t);
		*((float*)val) = res;
	}
	else if(type == EXPR_SCALE_CONTROL_CLASS_ID) {
		ScaleValue val0, val1, val2, res;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} 
			else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
		valid.Set(t,t);
		*((ScaleValue *)val) = res;
	}
	else {
		Point3 val0, val1, val2, res;
		switch (etype) {
		case ORT_LINEAR:			
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				GetValueLocalTime(range.Start()+1,&val1,valid);
				res = LinearExtrapolate(range.Start(),t,val0,val1,val0);				
			} 
			else {
				GetValueLocalTime(range.End()-1,&val0,valid);
				GetValueLocalTime(range.End(),&val1,valid);
				res = LinearExtrapolate(range.End(),t,val0,val1,val1);
			}
			break;

		case ORT_IDENTITY:
			if (t<range.Start()) {
				GetValueLocalTime(range.Start(),&val0,valid);
				res = IdentityExtrapolate(range.Start(),t,val0);
			} 
			else {
				GetValueLocalTime(range.End(),&val0,valid);
				res = IdentityExtrapolate(range.End(),t,val0);
			}
			break;

		case ORT_RELATIVE_REPEAT:
			GetValueLocalTime(range.Start(),&val0,valid);
			GetValueLocalTime(range.End(),&val1,valid);
			GetValueLocalTime(CycleTime(range,t),&val2,valid);
			res = RepeatExtrapolate(range,t,val0,val1,val2);			
			break;
		}
		valid.Set(t,t);
		*((Point3 *)val) = res;
	}
}

ReferenceTarget* ExprControl::GetReference(int i) 
{
	int num;

	if(i < (num = StdControl::NumRefs()))
		return StdControl::GetReference(i);
	return (ReferenceTarget*)refTab[i-num].client;
}

void ExprControl::SetReference(int i, RefTargetHandle rtarg)
{
	int num;

	if(i < (num = StdControl::NumRefs()))
		StdControl::SetReference(i, rtarg);
	else
		refTab[i-num].client = rtarg;
}


// RB: When the last reference to an expression controller is
// deleted its parameter dialog needs to be closed.
void ExprControl::RefDeleted()
	{
	if (hDlg) {
		int c=0;
		RefListItem  *ptr = GetRefList().first;
		while (ptr) {
			if (ptr->maker!=NULL) {
				if (ptr->maker->SuperClassID()) {
					c++;
					break;
				}
				}
			ptr = ptr->next;
			}	
		if (!c) DestroyWindow(hDlg);	
	}
}

class ExprSVarChange : public RestoreObj {	
	SVar svarUndo, svarRedo;
	ExprControl* ec;
	int index;
public:
	ExprSVarChange(ExprControl* ec, int index) : ec(ec), index(index)
	{
		svarUndo.offset = ec->sVars[index].offset;
		svarUndo.val = ec->sVars[index].val;
	}
	void Restore(int isUndo) 
	{
		svarRedo.offset = ec->sVars[index].offset;
		svarRedo.val = ec->sVars[index].val;
		ec->sVars[index].offset = svarUndo.offset;
		ec->sVars[index].val = svarUndo.val;
		ec->updateVarList(true, true);
	}
	void Redo()
	{
		ec->sVars[index].offset = svarRedo.offset;
		ec->sVars[index].val = svarRedo.val;
		ec->updateVarList(true,true);
	}
	void EndHold() { }
};

class ExprVVarChange : public RestoreObj {	
	VVar vvarUndo, vvarRedo;
	ExprControl* ec;
	int index;
public:
	ExprVVarChange(ExprControl* ec, int index) : ec(ec), index(index)
	{
		vvarUndo.offset = ec->vVars[index].offset;
		vvarUndo.val = ec->vVars[index].val;
	}
	void Restore(int isUndo) 
	{
		vvarRedo.offset = ec->vVars[index].offset;
		vvarRedo.val = ec->vVars[index].val;
		ec->vVars[index].offset = vvarUndo.offset;
		ec->vVars[index].val = vvarUndo.val;
		ec->updateVarList(true,true);
	}
	void Redo()
	{
		ec->vVars[index].offset = vvarRedo.offset;
		ec->vVars[index].val = vvarRedo.val;
		ec->updateVarList(true,true);
	}
	void EndHold() { }
};

class ExprExprEditRestore : public RestoreObj {	
	TSTR exprUndo, exprRedo;
	ExprControl* ec;
public:
	ExprExprEditRestore(ExprControl* ec, TCHAR *expression) : ec(ec)
	{
		exprUndo = ec->lastEditedExpr;
		exprRedo = expression;
	}
	void Restore(int isUndo) 
	{
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_EXPR_EDIT, exprUndo);
	}
	void Redo()
	{
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_EXPR_EDIT, exprRedo);
	}
	void EndHold() { }
};

class ExprExprRestore : public RestoreObj {	
	TSTR exprUndo, exprRedo;
	ExprControl* ec;
public:
	ExprExprRestore(ExprControl* ec, TCHAR *expression = NULL) : ec(ec)
	{
		if (expression)
			exprUndo = expression;
		else
			exprUndo = ec->expr.getExprStr();
	}
	void Restore(int isUndo) 
	{
		if (isUndo)
			exprRedo = ec->expr.getExprStr();
		ec->disabledDueToRecursion = false;
		ec->expr.load(exprUndo);
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_EXPR_EDIT, exprUndo);
		if(ec->edbg)
			ec->edbg->Invalidate();
	}
	void Redo()
	{
		ec->disabledDueToRecursion = false;
		ec->expr.load(exprRedo);
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_EXPR_EDIT, exprRedo);
		if(ec->edbg)
			ec->edbg->Invalidate();
	}
	void EndHold() { }
};

class ExprDescRestore : public RestoreObj {	
	TSTR descUndo, descRedo;
	ExprControl* ec;
public:
	ExprDescRestore(ExprControl* ec) : ec(ec)
	{
		descUndo = ec->desc;
	}
	void Restore(int isUndo) 
	{
		if (isUndo)
			descRedo = ec->desc;
		ec->desc = descUndo;
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_DESCRIPTION, descUndo);
		}
	void Redo()
	{
		ec->desc = descRedo;
		if (ec->hDlg)
			SetDlgItemText(ec->hDlg, IDC_DESCRIPTION, descRedo);
	}
	void EndHold() { }
};

class ExprRefTabItemDeleteRestore : public RestoreObj {	
	int index;
	ExprControl* ec;
public:
	ExprRefTabItemDeleteRestore(ExprControl* ec, int index) : ec(ec), index(index) {}
	void Restore(int isUndo) 
	{
		VarRef vr;
		ec->refTab.Insert(index,1,&vr);
	}
	void Redo()
	{
		ec->refTab.Delete(index, 1);
	}
	void EndHold() { }
};

class ExprVarRestore : public RestoreObj {	
public:
	ExprControl *ec;
	SVarTab		sVarsUndo;
	VVarTab		vVarsUndo;
	VarRefTab	refTabUndo;
	ExprVarTab	varsUndo;
	SVarTab		sVarsRedo;
	VVarTab		vVarsRedo;
	VarRefTab	refTabRedo;
	ExprVarTab	varsRedo;
	bool		save_Expr_vars;
	bool		need_redo_data;

	~ExprVarRestore()
	{
		int i;
		for (i = 0; i < sVarsUndo.Count(); i++) delete sVarsUndo[i].name;
		for (i = 0; i < vVarsUndo.Count(); i++) delete vVarsUndo[i].name;
		for (i = 0; i < varsUndo.Count(); i++) delete varsUndo[i].name;
		for (i = 0; i < sVarsRedo.Count(); i++) delete sVarsRedo[i].name;
		for (i = 0; i < vVarsRedo.Count(); i++) delete vVarsRedo[i].name;
		for (i = 0; i < varsRedo.Count(); i++) delete varsRedo[i].name;

	}
	ExprVarRestore(ExprControl *ec, bool save_Expr_vars = false)
	{ 
		this->ec=ec;
		this->save_Expr_vars = save_Expr_vars;
		need_redo_data = true;

		SVar svar;
		VVar vvar;
		ExprVar evar;
		int i;

		for(i = 0; i < ec->sVars.Count(); i++) {
			sVarsUndo.Append(1,&svar,ec->sVars.Count()-1);
			sVarsUndo[i] = ec->sVars[i];
		}

		for(int i = 0; i < ec->vVars.Count(); i++) {
			vVarsUndo.Append(1,&vvar,ec->vVars.Count()-1);
			vVarsUndo[i] = ec->vVars[i];
		}

		refTabUndo.SetCount(ec->refTab.Count());
		for (int i = 0; i < ec->refTab.Count(); i++) 
			refTabUndo[i] = ec->refTab[i]; 

		if (save_Expr_vars)
		{
			for (i = 0; i < ec->expr.vars.Count(); i++) {
				varsUndo.Append(1,&evar,ec->expr.vars.Count()-1);
				varsUndo[i] = ec->expr.vars[i]; 
			}
		}
	}
	void Restore(int isUndo) 
	{
		SVar svar;
		VVar vvar;
		ExprVar evar;

		if (isUndo && need_redo_data) {
			need_redo_data = false;

			// save the redo data

			for(int i = 0; i < ec->sVars.Count(); i++) {
				sVarsRedo.Append(1,&svar,ec->sVars.Count()-1);
				sVarsRedo[i] = ec->sVars[i];
		}

			for(int i = 0; i < ec->vVars.Count(); i++) {
				vVarsRedo.Append(1,&vvar,ec->vVars.Count()-1);
				vVarsRedo[i] = ec->vVars[i];
		}

			refTabRedo.SetCount(ec->refTab.Count());
			for (int i = 0; i < ec->refTab.Count(); i++) 
				refTabRedo[i] = ec->refTab[i]; 

			if (save_Expr_vars)
			{
				for (int i = 0; i < ec->expr.vars.Count(); i++) {
					varsRedo.Append(1,&evar,ec->expr.vars.Count()-1);
					varsRedo[i] = ec->expr.vars[i]; 
				}
			}
		}

		// put the undo data back into the controller

		bool diffSize = ec->sVars.Count() != sVarsUndo.Count();
		if (diffSize) {
			for (int i = 0; i < ec->sVars.Count(); i++)
				delete ec->sVars[i].name;
			ec->sVars.SetCount(0);
		}
		for(int i = 0; i < sVarsUndo.Count(); i++) {
			if (diffSize)
				ec->sVars.Append(1,&svar,sVarsUndo.Count()-1);
			ec->sVars[i] = sVarsUndo[i];
		}

		diffSize = ec->vVars.Count() != vVarsUndo.Count();
		if (diffSize) {
			for (int i = 0; i < ec->vVars.Count(); i++)
				delete ec->vVars[i].name;
			ec->vVars.SetCount(0);
		}
		for(int i = 0; i < vVarsUndo.Count(); i++) {
			if (diffSize)
				ec->vVars.Append(1,&vvar,vVarsUndo.Count()-1);
			ec->vVars[i] = vVarsUndo[i];
		}

		ec->refTab.SetCount(refTabUndo.Count());
		for (int i = 0; i < ec->refTab.Count(); i++)
			ec->refTab[i] = refTabUndo[i]; 

		if (save_Expr_vars)
		{
			diffSize = ec->expr.vars.Count() != varsUndo.Count();
			if (diffSize) {
				for (int i = 0; i < ec->expr.vars.Count(); i++)
					delete ec->expr.vars[i].name;
				ec->expr.vars.SetCount(0);
			}
			for (int i = 0; i < varsUndo.Count(); i++) {
				if (diffSize)
					ec->expr.vars.Append(1,&evar,varsUndo.Count()-1);
				ec->expr.vars[i]= varsUndo[i]; 
			}
		}

		ec->disabledDueToRecursion = false;
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		ec->updateVarList(true, true);
	}
	void Redo()
	{
		SVar svar;
		VVar vvar;
		ExprVar evar;
		int i;

		// put the redo data back into the controller

		bool diffSize = ec->sVars.Count() != sVarsRedo.Count();
		if (diffSize) {
			for (i = 0; i < ec->sVars.Count(); i++)
				delete ec->sVars[i].name;
			ec->sVars.SetCount(0);
		}
		for(int i = 0; i < sVarsRedo.Count(); i++) {
			if (diffSize)
				ec->sVars.Append(1,&svar,sVarsRedo.Count()-1);
			ec->sVars[i] = sVarsRedo[i];
		}

		diffSize = ec->vVars.Count() != vVarsRedo.Count();
		if (diffSize) {
			for (int i = 0; i < ec->vVars.Count(); i++)
				delete ec->vVars[i].name;
			ec->vVars.SetCount(0);
		}
		for(int i = 0; i < vVarsRedo.Count(); i++) {
			if (diffSize)
				ec->vVars.Append(1,&vvar,vVarsRedo.Count()-1);
			ec->vVars[i] = vVarsRedo[i];
		}

		ec->refTab.SetCount(refTabRedo.Count());
		for (int i = 0; i < refTabRedo.Count(); i++)
			ec->refTab[i] = refTabRedo[i]; 

		if (save_Expr_vars)
		{
			diffSize = ec->expr.vars.Count() != varsRedo.Count();
			if (diffSize) {
				for (i = 0; i < ec->expr.vars.Count(); i++)
					delete ec->expr.vars[i].name;
				ec->expr.vars.SetCount(0);
			}
			for (i = 0; i < varsRedo.Count(); i++) {
				if (diffSize)
					ec->expr.vars.Append(1,&evar,varsRedo.Count()-1);
				ec->expr.vars[i]= varsRedo[i]; 
			}
		}

		ec->disabledDueToRecursion = false;
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		ec->updateVarList(true,true);
	}
	void EndHold() { }
};

class ExprInvalidateRestore : public RestoreObj {	
	ExprControl* ec;
public:
	ExprInvalidateRestore(ExprControl* ec) : ec(ec)
	{
	}
	void Restore(int isUndo) 
	{
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void Redo()
	{
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void EndHold() { }
};

RefResult ExprControl::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
{
	int i;

	switch (msg) {
	case REFMSG_GET_NODE_NAME:
		// RB 3/23/99: See comment at imp of getNodeName().
		if (blockGetNodeName) return REF_STOP;
		break;

	case REFMSG_CHANGE:
		ivalid.SetEmpty();
		if(edbg)
			edbg->Invalidate();
		break;

	case REFMSG_OBJECT_CACHE_DUMPED:
		return REF_STOP;
		break;

// DS: 5-21-97 -- removed the REFMSG_REF_DELETED. This would cause problems
// when multiple Expr controls referenced the same thing.  When one of the expr
// controls got deleted this message was sent to all the others, setting their
// refs to NULL even though the target still had backptr to them. This would cause
// crashes later.
//	case REFMSG_REF_DELETED:		// this is for when visibility tracks are deleted

	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		updateVarList(true,true);
		break;	

	case REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED: // this is for when nodes referenced via NodeTransformMonitor are deleted
	case REFMSG_TARGET_DELETED: {	
		if (theHold.Holding())
			theHold.Put(new ExprVarRestore(this, false));

		// get rid of any scalar or vector variable references
		for(i = 0; i < sVars.Count(); i++)
			if(sVars[i].refID >= 0 && refTab[sVars[i].refID].client == hTarg)
				sVars[i].refID = -1;
		for(i = 0; i < vVars.Count(); i++)
			if(vVars[i].refID >= 0 && refTab[vVars[i].refID].client == hTarg)
				vVars[i].refID = -1;

		// then get rid of the reference itself
		for (i = 0; i < refTab.Count(); i++)
			if (refTab[i].client == hTarg) 
				refTab[i].refCt = 0;

		// get rid of unused refs
		dropUnusedRefs();

		updateVarList(true,true);

		// REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED is sent with propogate off
		if (msg == REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED)
			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

		break;	
		}
	// jbw 11.7.00 - sent by objects whose parameters have changed subanim order
	case REFMSG_SUBANIM_NUMBER_CHANGED: {

		if (theHold.Holding())
			theHold.Put(new ExprVarRestore(this));

			// partID points to an Tab<ULONG> of renumberings, old num in LOWORD, new in HIWORD
			Tab<ULONG> renumberings = *reinterpret_cast<Tab<ULONG>*>(partID);  // take local copy so we can mark the ones used

			// update variables to point at new param subnum
			for(int i = 0; i < sVars.Count(); i++)
			if(sVars[i].refID >= 0 && refTab[sVars[i].refID].client == hTarg) {
				for (int j = 0; j < renumberings.Count(); j++) {
						short old_subNum = LOWORD(renumberings[j]);
						short new_subNum = HIWORD(renumberings[j]);
					if (sVars[i].subNum == old_subNum) {
						// found old subnum use, change to new subnum
						if (new_subNum < 0)
						{
							if (sVars[i].refID != -1)
								refTab[sVars[i].refID].refCt--;
								sVars[i].refID = -1;
						}
							else
								sVars[i].subNum = new_subNum;
						renumberings[j] = -1;  // mark renumbering as used
							break;
						}
					}
				}
			// same for vector vars
			for(int i = 0; i < vVars.Count(); i++)
			if(vVars[i].refID >= 0 && refTab[vVars[i].refID].client == hTarg) {
				for (int j = 0; j < renumberings.Count(); j++) {
						short old_subNum = LOWORD(renumberings[j]);
						short new_subNum = HIWORD(renumberings[j]);
					if (vVars[i].subNum == old_subNum) {
						// found old subnum use, change to new subnum
						if (new_subNum < 0)
						{
							if (vVars[i].refID != -1)
								refTab[vVars[i].refID].refCt--;
								vVars[i].refID = -1;
						}
							else
								vVars[i].subNum = new_subNum;
						renumberings[j] = -1;  // mark renumbering as used
							break;
						}
					}
				}
		updateVarList(true,true);
		dropUnusedRefs();
			break;
		}
	}
	return REF_SUCCEED;
}


class ExprControlRestore : public RestoreObj {	
public:
	ExprControl *ec;
	Interval undo, redo;
	ExprControlRestore(ExprControl *ec) { this->ec=ec; undo=ec->range; }
	void Restore(int isUndo) 
	{
		redo = ec->range;
		ec->range = undo;
		if(ec->edbg)
			ec->edbg->Invalidate();
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void Redo() 
	{
		ec->range = redo;
		if(ec->edbg)
			ec->edbg->Invalidate();
		ec->ivalid.SetEmpty();
		ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
	void EndHold() { ec->ClearAFlag(A_HELD); }
};

void ExprControl::Hold()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		theHold.Put(new ExprControlRestore(this));
		SetAFlag(A_HELD);
	}
}

void ExprControl::MapKeys( TimeMap *map, DWORD flags ) 
{
	Hold();
	range.Set(map->map(range.Start()), map->map(range.End())); 
	if(edbg)
		edbg->Invalidate();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

void ExprControl::EditTimeRange(Interval in_range, DWORD in_flags)
{
	if (in_flags == EDITRANGE_LINKTOKEYS)
		return;
	range = in_range;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void ExprControl::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{	
}

void ExprControl::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
}

TSTR ExprControl::SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker)
{
	return SvGetName(gom, gNodeMaker, false) + " -> " + gNodeTarger->GetAnim()->SvGetName(gom, gNodeTarger, false);
}

bool ExprControl::SvHandleRelDoubleClick(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker)
{
	return Control::SvHandleDoubleClick(gom, gNodeMaker);
}

SvGraphNodeReference ExprControl::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
{
	SvGraphNodeReference nodeRef = Control::SvTraverseAnimGraph( gom, owner, id, flags );

	if( nodeRef.stat == SVT_PROCEED ) {
		for( int i=0; i<refTab.Count(); i++ ) {
			gom->AddRelationship( nodeRef.gNode, refTab[i].client, i, RELTYPE_CONTROLLER );
		}
	}

	return nodeRef;
}

// RB 3/23/99: To solve 75139 (the problem where a node name is found for variables that 
// are not associated with nodes such as globabl tracks) we need to block the propogation
// of this message through our reference to the client of the variable we're referencing.
// In the expression controller's imp of NotifyRefChanged() we're going to block the get
// node name message if the blockGetNodeName variable is TRUE.
void ExprControl::getNodeName(ReferenceTarget *client, TSTR &name)
	{
	if (client) {
		if (client->SuperClassID() == BASENODE_CLASS_ID)
			name = ((INode*)client)->GetName();
		else {
			blockGetNodeName = true;
			client->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
			blockGetNodeName = false;
		}
	}
}

void ExprControl::updateVarList(bool maintainSelection, bool delayedUpdate)
{
	if (!hDlg) return;

	if (delayedUpdate)
	{
		doDelayedUpdateVarList = true;
		if (delayedUpdateVarListMaintainSelection)
			delayedUpdateVarListMaintainSelection = maintainSelection;
		InvalidateRect(hDlg, NULL, FALSE);
		return;
	}
	doDelayedUpdateVarList = false;
	delayedUpdateVarListMaintainSelection = true;

	if(edbg)
		edbg->Invalidate();

	int i, ct, sel = LB_ERR;
	TCHAR buf[100];

	if (maintainSelection) {
		sel = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCURSEL, 0, 0);
		if (sel != LB_ERR)
			SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, sel, (LPARAM)buf);
	}

	ct = getVarCount(SCALAR_VAR);
	SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_RESETCONTENT, 0, 0);
	for(i = 0; i < ct; i++)
		SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_ADDSTRING, 0, (LPARAM)getVarName(SCALAR_VAR, i));

	if (maintainSelection && sel != LB_ERR)
	{
		if (sel > ct) sel = ct;
		int j = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_FINDSTRINGEXACT, -1, (LPARAM)buf);
		if (j == LB_ERR) j = sel;
		SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_SETCURSEL, j, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
	}


	if (maintainSelection) {
		sel = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCURSEL, 0, 0);
		if (sel != LB_ERR)
			SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, sel, (LPARAM)buf);
	}

	ct = getVarCount(VECTOR_VAR);
	SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_RESETCONTENT, 0, 0);
	for(i = 0; i < ct; i++)
		SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_ADDSTRING, 0, (LPARAM)getVarName(VECTOR_VAR, i));

	if (maintainSelection && sel != LB_ERR)
	{
		if (sel > ct) sel = ct;
		int j = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_FINDSTRINGEXACT, -1, (LPARAM)buf);
		if (j == LB_ERR) j = sel;
		SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_SETCURSEL, j, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
	}
}

void ExprControl::dropUnusedRefs()
{
	int i, j;
	// get rid of unused refs
	for (j = refTab.Count()-1; j >= 0; j--)
	{
		if (refTab[j].refCt == 0 && refTab[j].client != NULL )
		{
			DeleteReference(j+StdControl::NumRefs());
			refTab[j].client = NULL;
		}
	}

	int oldRefTabCount = refTab.Count();

	for (j = refTab.Count()-1; j >= 0; j--)
	{
		if (refTab[j].client == NULL) {
			for(i = 0; i < sVars.Count(); i++)
				if(sVars[i].refID >= j)
					sVars[i].refID -= 1;
			for(i = 0; i < vVars.Count(); i++)
				if(vVars[i].refID >= j)
					vVars[i].refID -= 1;
			refTab.Delete(j,1);
			if (theHold.Holding())
				theHold.Put(new ExprRefTabItemDeleteRestore(this, j));
		}
	}
	disabledDueToRecursion = false;
}

void ExprControl::NotifyPreReset(void* param, NotifyInfo*)
{
	ExprControl* ec = (ExprControl*)param;
	if (ec->hDlg)
		SendMessage(ec->hDlg,WM_CLOSE,0,0);
	}

static void setupSpin(HWND hDlg, int spinID, int editID, float val)
{
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hDlg, spinID));
	spin->SetLimits( (float)-9e30, (float)9e30, FALSE );
	spin->SetScale( 1.0f );
	spin->LinkToEdit( GetDlgItem(hDlg, editID), EDITTYPE_FLOAT );
	spin->SetValue( val, FALSE );
	ReleaseISpinner( spin );
}

static float getSpinVal(HWND hDlg, int spinID)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg, spinID));
	float res = spin->GetFVal();
	ReleaseISpinner(spin);
	return res;
}

static INT_PTR CALLBACK ScalarAsgnDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ExprControl *ec = DLGetWindowLongPtr<ExprControl *>(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		DLSetWindowLongPtr(hDlg, lParam);
		ec = (ExprControl *)lParam;
		SetWindowText(hDlg, ec->getVarName(SCALAR_VAR, ec->curIndex));
		setupSpin(hDlg, IDC_EXPR_CONST_SPIN, IDC_EXPR_CONST, ec->getScalarValue(ec->curIndex, ec->ip->GetTime()));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		theHold.Begin();
		if (theHold.Holding()) {
			if (ec->sVars[ec->curIndex].refID == -1)
				theHold.Put(new ExprSVarChange(ec, ec->curIndex));
			else
				theHold.Put(new ExprVarRestore(ec));
		}
		ec->assignScalarValue(ec->curIndex, getSpinVal(hDlg, IDC_EXPR_CONST_SPIN));
		ec->dropUnusedRefs();
		theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));

		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}

static INT_PTR CALLBACK VectorAsgnDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Point3 pt;
	ExprControl *ec = DLGetWindowLongPtr<ExprControl *>(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		DLSetWindowLongPtr(hDlg, lParam);
		ec = (ExprControl *)lParam;
		SetWindowText(hDlg, ec->getVarName(VECTOR_VAR, ec->curIndex));
		pt = ec->getVectorValue(ec->curIndex, ec->ip->GetTime());
		setupSpin(hDlg, IDC_VEC_X_SPIN, IDC_VEC_X, pt.x);
		setupSpin(hDlg, IDC_VEC_Y_SPIN, IDC_VEC_Y, pt.y);
		setupSpin(hDlg, IDC_VEC_Z_SPIN, IDC_VEC_Z, pt.z);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		theHold.Begin();
		if (theHold.Holding()) {
			if (ec->vVars[ec->curIndex].refID == -1)
				theHold.Put(new ExprVVarChange(ec, ec->curIndex));
			else
				theHold.Put(new ExprVarRestore(ec));
		}
		pt.x = getSpinVal(hDlg, IDC_VEC_X_SPIN);
		pt.y = getSpinVal(hDlg, IDC_VEC_Y_SPIN);
		pt.z = getSpinVal(hDlg, IDC_VEC_Z_SPIN);
		ec->assignVectorValue(ec->curIndex, pt);
		ec->dropUnusedRefs();
		theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}

static int isEmpty(TCHAR *s)
{
	int c = 0;
	while((c = *s) != 0) {
		if(c != ' ' && c != '\t' && c != '\n' && c != '\r')
			return FALSE;
		s++;
	}
	return TRUE;
}

static TCHAR revertVal[4096];			// 001110  --prs.

static int updateExpr(HWND hDlg, ExprControl *ec, bool dismiss)
{
	TCHAR buf[4096];
	int exprType;
	GetDlgItemText(hDlg, IDC_EXPR_EDIT, buf, 4096);

	ec->disabledDueToRecursion = false;
	if(ec->expr.load(buf)) {
		if(isEmpty(buf)) {
			switch(ec->type) {
			case EXPR_POS_CONTROL_CLASS_ID:
			case EXPR_P3_CONTROL_CLASS_ID:
				_tcscpy(buf,_T("[ 0, 0, 0 ]"));
				break;
			case EXPR_SCALE_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("[ 1, 1, 1 ]"));
				break;
			case EXPR_FLOAT_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("0"));
				break;
			case EXPR_ROT_CONTROL_CLASS_ID:
				_tcscpy(buf, _T("{ [ 0, 0, 0 ], 0 }"));
				break;
			}
			SetDlgItemText(hDlg, IDC_EXPR_EDIT, buf);
			ec->expr.load(buf);
		}
		else if (dismiss) {		// this case 001110  --prs.
			TCHAR sbuf[4200];
			_stprintf(sbuf, GetString(IDS_PRS_EXPR_INVALID_REVERT), ec->expr.getProgressStr());
			int id = MessageBox(hDlg, sbuf, GetString(IDS_DB_EXPR_PARSE_ERROR),
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OKCANCEL);
			if (id == IDCANCEL)
				return FALSE;		// let user edit it again
			SetDlgItemText(hDlg, IDC_EXPR_EDIT, revertVal);	// else revert it
			ec->expr.load(revertVal);
			return TRUE;
		}
		else {
			MessageBox(hDlg, ec->expr.getProgressStr(), GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
	}
	exprType = ec->expr.getExprType();

	switch(ec->type) {
	case EXPR_SCALE_CONTROL_CLASS_ID:
	case EXPR_POS_CONTROL_CLASS_ID:
	case EXPR_P3_CONTROL_CLASS_ID:
		if(exprType != VECTOR_EXPR) {
			TSTR s = GetString(IDS_DB_NEED_VECTOR);
			MessageBox(hDlg, s, GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
		break;
	case EXPR_FLOAT_CONTROL_CLASS_ID:
		if(exprType != SCALAR_EXPR) {
			TSTR s = GetString(IDS_DB_NEED_SCALAR);
			MessageBox(hDlg, s, GetString(IDS_DB_EXPR_PARSE_ERROR), 
					MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			return FALSE;
		}
		break;
	}

	ec->ivalid.SetEmpty();
	ec->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	ec->NotifyDependents(FOREVER,0,REFMSG_MAXCONTROL_NOT_NEW,MAXSCRIPT_WRAPPER_CLASS_ID,FALSE,ec);
	ec->ip->RedrawViews(ec->ip->GetTime());
	return TRUE;
}

#define XPR	_T(".xpr")

static void FixExprFileExt(OPENFILENAME &ofn) 
{
   int l = static_cast<int>(_tcslen(ofn.lpstrFile));	// SR DCAST64: Downcast to 2G limit.
	int e = static_cast<int>(_tcslen(XPR));	// SR DCAST64: Downcast to 2G limit.
	if (e>l || _tcsicmp(ofn.lpstrFile+l-e, XPR))
		_tcscat(ofn.lpstrFile,XPR);	
}

static BOOL BrowseForExprFilename(TCHAR *name, TCHAR *dir, int save) 
{
	int tried = 0;
	FilterList filterList;
	HWND hWnd = GetActiveWindow();
	static int filterIndex = 1;
    OPENFILENAME	ofn;
    TCHAR			fname[256];

	_tcscpy(fname, name);

	filterList.Append( GetString(IDS_DB_XPR_FILES) );
	filterList.Append( _T("*.xpr"));

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize      = sizeof(OPENFILENAME);  // No OFN_ENABLEHOOK
    ofn.hwndOwner        = hWnd;
	ofn.hInstance        = DLGetWindowInstance(hWnd);

	ofn.nFilterIndex = filterIndex;
    ofn.lpstrFilter  = filterList;

    ofn.lpstrTitle   = GetString(save ? IDS_DB_SAVE_EXPR : IDS_DB_LOAD_EXPR);
    ofn.lpstrFile    = fname;
    ofn.nMaxFile     = sizeof(fname) / sizeof(TCHAR);      
	
	if(dir && dir[0])
	   	ofn.lpstrInitialDir = dir;
	else
	   	ofn.lpstrInitialDir = _T("");

	ofn.Flags = OFN_HIDEREADONLY | (save ? OFN_OVERWRITEPROMPT : (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST));
	ofn.FlagsEx = OFN_EX_NOPLACESBAR;

	if(save) {
		if(!GetSaveFileName(&ofn))
			return FALSE;
	}
	else {
		if(!GetOpenFileName(&ofn))
			return FALSE;
	}
	FixExprFileExt(ofn); // add ".xpr" if absent
	if(dir) {
		_tcsncpy(dir, ofn.lpstrFile, ofn.nFileOffset-1);
		dir[ofn.nFileOffset-1] = _T('\0');
	}
	_tcscpy(name, ofn.lpstrFile);
	return TRUE;
}

static TCHAR exprDir[512];

static void SaveExpression(HWND hDlg, ExprControl *ec)
{
	TCHAR buf[4096];
	int i, ct;
	FILE *fp;

	if(!exprDir[0])
		_tcscpy(exprDir, ec->ip->GetDir(APP_EXPRESSION_DIR));
	buf[0] = _T('\0');
	if(BrowseForExprFilename(buf, exprDir, TRUE))	{
		if((fp = _tfopen(buf, _T("w"))) == NULL)
			return;
		GetDlgItemText(hDlg, IDC_EXPR_EDIT, buf, 4096);
		TSTR outString = buf;
		if(!outString.isNull()) {
			Replace_CRLF_with_LF(outString);
			if (outString[outString.Length()-1] == _T('\n'))
				_ftprintf(fp, _T("%s"), outString.data());
			else
				_ftprintf(fp, _T("%s\n"), outString.data());
		}

		GetDlgItemText(hDlg, IDC_DESCRIPTION, buf, 4096);
		if(buf[0]) {
			TCHAR line[1024];
			TCHAR *cp = buf;
			int i;
			for(i = 0; *cp; i++, cp++) {
				line[i] = *cp;
				if(*cp == _T('\r')) {
					line[i] = 0;
					_ftprintf(fp, _T("d: %s\n"), line);
					line[0] = 0;
					cp++;	// skip over linefeed
					i = -1;	// reset i
				}
			}
			if(line[0]) {
				line[i] = 0;
				_ftprintf(fp, _T("d: %s\n"), line);
			}
		}
		ct = ec->getVarCount(SCALAR_VAR);
		for(i = 0; i < ct; i++)
			_ftprintf(fp, _T("s: %s:%d\n"), ec->getVarName(SCALAR_VAR, i), ec->sVars[i].offset);
		ct = ec->getVarCount(VECTOR_VAR);
		for(i = 0; i < ct; i++)
			_ftprintf(fp, _T("v: %s:%d\n"), ec->getVarName(VECTOR_VAR, i), ec->vVars[i].offset);
		fclose(fp);
	}
}

static void LoadExpression(HWND hDlg, ExprControl *ec)
{
	TCHAR buf[4096];
	TCHAR line[1024];
	TCHAR desc[4096];
	TSTR lineBuf;
	TSTR inString;
	int slot, i, ct;
	int type;
	int offset;
	TCHAR *cp;
	FILE *fp;

	if(!exprDir[0])
		_tcscpy(exprDir, ec->ip->GetDir(APP_EXPRESSION_DIR));
	buf[0] = _T('\0');
	if(BrowseForExprFilename(buf, exprDir, FALSE))	{
		if((fp = _tfopen(buf, _T("r"))) == NULL)
			return;

		theHold.Begin();
		if (theHold.Holding()) {
			theHold.Put(new ExprInvalidateRestore(ec));
			theHold.Put(new ExprVarRestore(ec, true));
			theHold.Put(new ExprExprRestore(ec));
			theHold.Put(new ExprDescRestore(ec));
		}

		desc[0] = _T('\0');
		SetDlgItemText(hDlg, IDC_DESCRIPTION, _T(""));		// empty-out description
		ct = ec->sVars.Count();								// empty-out vars
		for(i = 0; i < ct; i++) {	// leave pre-defined scalars
			ec->expr.deleteVar(ec->sVars[0].name);
			delete ec->sVars[0].name;
			ec->sVars.Delete(0, 1);
		}
		ct = ec->vVars.Count();
		for(i = 0; i < ct; i++) {
			ec->expr.deleteVar(ec->vVars[0].name);
			delete ec->vVars[0].name;
			ec->vVars.Delete(0, 1);
		}
		ct = ec->refTab.Count();
		for(i = 0; i < ct; i++) 
			ec->refTab[i].refCt = 0;

		ec->dropUnusedRefs();
		theHold.Accept(GetString(IDS_LAM_EXPR_LOAD));

		while(_fgetts(line, 1024, fp) && (line[1] != _T(':')))
		{
			lineBuf = line;
			inString.append(lineBuf);
		}
		Replace_LF_with_CRLF(inString);
		SetDlgItemText(hDlg, IDC_EXPR_EDIT, inString);
		ec->expr.load(lineBuf);

		if(line[1] == _T(':')) {
			do {
				if(line[_tcslen(line) - 1] == _T('\n'))
					line[_tcslen(line) - 1] = _T('\0');
				switch(line[0]) {
				case _T('d'):
					if(desc[0])
						_tcscat(desc, _T("\r\n"));
					_tcscat(desc, line+3);
					break;
				case _T('s'):
				case _T('v'):
					type = line[0] == _T('s') ? SCALAR_VAR : VECTOR_VAR;
					offset = 0;
					cp = line+3;
					while(cp[offset] != _T('\n') && cp[offset] != ':')
						offset++;
					cp[offset++] = _T('\0');
					ec->updRegCt(slot = ec->expr.defVar(type, cp), type);
					if(slot >= 0)
						ec->dfnVar(type, cp, slot, _ttoi(cp+offset));
					break;
				}
			} while(_fgetts(line, 1024, fp));
		}
		SetDlgItemText(hDlg, IDC_DESCRIPTION, desc);
		ec->updateVarList(false, true);
		ec->desc = desc;
		fclose(fp);
	}
}


BOOL invalidName(TCHAR *buf)
{
	TCHAR *cp = buf;
	if(_istdigit(*cp))
		return TRUE;
	while(*cp) {
		if(!_istalnum(*cp) && *cp != '_')
			return TRUE;
		cp++;
	}
	return FALSE;
}

static int fnListIDs[] = {
	IDS_DB_FN_SIN,
	IDS_DB_FN_NOISE,
	IDS_DB_FN_IF,
	IDS_DB_FN_VIF,
	IDS_DB_FN_MIN,
	IDS_DB_FN_MAX,
	IDS_DB_FN_POW,
	IDS_DB_FN_MOD,
	IDS_DB_FN_DEGTORAD,
	IDS_DB_FN_RADTODEG,
	IDS_DB_FN_COS,
	IDS_DB_FN_TAN,
	IDS_DB_FN_ASIN,
	IDS_DB_FN_ACOS,
	IDS_DB_FN_ATAN,
	IDS_DB_FN_SINH,
	IDS_DB_FN_COSH,
	IDS_DB_FN_TANH,
	IDS_DB_FN_LN,
	IDS_DB_FN_LOG,
	IDS_DB_FN_EXP,
	IDS_DB_FN_SQRT,
	IDS_DB_FN_ABS,
	IDS_DB_FN_CEIL,
	IDS_DB_FN_FLOOR,
	IDS_DB_FN_COMP,
	IDS_DB_FN_UNIT,
	IDS_DB_FN_LENGTH,
	IDS_DB_FN_PI,
	IDS_DB_FN_E,
	IDS_DB_FN_TPS
};

INT_PTR CALLBACK functionListProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 57;
    switch (message)  {
    case WM_INITDIALOG:
		CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
		SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_SETTABSTOPS, 1, (LPARAM)&i);
		for(i = 0; i < sizeof(fnListIDs) / sizeof(int); i++)
			SendDlgItemMessage(hDlg, IDC_FUNC_LIST, LB_ADDSTRING, 0, (LPARAM)GetString(fnListIDs[i]));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
		default:
			return FALSE;
		}
		break;
		
	case WM_CLOSE:
done:
		EndDialog(hDlg, TRUE);
		break;
    }
    return FALSE;
}

// init the resizing data for the expression dialog.Sets dynamic parameters 
// used for resizing expression and description dialog boxes and frames by the 
// ResizeDialog ( see below ) 
// lrr 04/04
void InitResizeData(HWND	hDlg,						
					int		&l_offset1,		
					int		&l_offset2,
					int		&l_orig_expr_height)
{		
	
	WINDOWPLACEMENT l_resizeWindowPlacement;
	l_resizeWindowPlacement.length = sizeof(WINDOWPLACEMENT);

	// get the  position of the Function button 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_FUNCTIONS), &l_resizeWindowPlacement);
	l_offset1 = l_offset2 = l_resizeWindowPlacement.rcNormalPosition.top;

	// get the offset of the bottom of the lower static control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_DESCR), &l_resizeWindowPlacement);
	l_offset1 -= l_resizeWindowPlacement.rcNormalPosition.bottom;

	// get the offset of the bottom of the lower edit control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_DESCRIPTION), &l_resizeWindowPlacement);
	l_offset2 -= l_resizeWindowPlacement.rcNormalPosition.bottom;

	// get the height of upper static control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_EXPR), &l_resizeWindowPlacement);
	l_orig_expr_height = l_resizeWindowPlacement.rcNormalPosition.bottom - l_resizeWindowPlacement.rcNormalPosition.top;
}

// Resizes the expression and description dialog boxes keeping their original proportion.
// lrr 05/05
void ResizeDialog(HWND	hDlg, 
				  int	in_offset1, 
				  int	in_offset2,
				  int	in_orig_expr_height)
{

	WINDOWPLACEMENT l_resizeWindowPlacement;
	l_resizeWindowPlacement.length = sizeof(WINDOWPLACEMENT);

	// get the  position of the Function button 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_FUNCTIONS), &l_resizeWindowPlacement);
	int l_P0_Y		= l_resizeWindowPlacement.rcNormalPosition.top; 

	// get the height of upper static control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_EXPR), &l_resizeWindowPlacement);
	int l_expr_height = l_resizeWindowPlacement.rcNormalPosition.bottom - l_resizeWindowPlacement.rcNormalPosition.top;
	int l_expr_height_offset = (l_expr_height - in_orig_expr_height)/2;

	// set the height of upper static control 
	l_resizeWindowPlacement.rcNormalPosition.bottom -= l_expr_height_offset;
	SetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_EXPR), &l_resizeWindowPlacement);

	// set the height of upper edit control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_EXPR_EDIT), &l_resizeWindowPlacement);
	l_resizeWindowPlacement.rcNormalPosition.bottom -= l_expr_height_offset;
	SetWindowPlacement(GetDlgItem(hDlg, IDC_EXPR_EDIT), &l_resizeWindowPlacement);

	// set the offset to the top and bottom of the lower static control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_DESCR), &l_resizeWindowPlacement);
	l_resizeWindowPlacement.rcNormalPosition.top -= l_expr_height_offset;
	l_resizeWindowPlacement.rcNormalPosition.bottom = l_P0_Y - in_offset1;
	SetWindowPlacement(GetDlgItem(hDlg, IDC_STATIC_DESCR), &l_resizeWindowPlacement);

	// set the offset to the top and bottom of the lower edit control 
	GetWindowPlacement(GetDlgItem(hDlg, IDC_DESCRIPTION), &l_resizeWindowPlacement);
	l_resizeWindowPlacement.rcNormalPosition.top -= l_expr_height_offset;
	l_resizeWindowPlacement.rcNormalPosition.bottom = l_P0_Y - in_offset2;
	SetWindowPlacement(GetDlgItem(hDlg,IDC_DESCRIPTION ), &l_resizeWindowPlacement);	
}

typedef struct
{
	ExprControl *ec;
	DialogResizer *resizer;
} ExprControlDialogData;

static UINT msg_count = 0; // for debugging. msg counter

static INT_PTR CALLBACK ExprParamsWndProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	if (msg == WM_COMMAND || msg == WM_ACTIVATE || msg == WM_NCACTIVATE || msg == WM_CAPTURECHANGED)
//		DebugPrint("ExprParamsWndProc (%X)- msg:%X; hDlg:%X; wParam:%X; lParam:%X\n", msg_count++, msg, hDlg, (int)wParam, (int)lParam);
	int type = 0, slot = 0, i, ct;
	Point3 pt;
	TCHAR buf[4096], buf2[257];
	TrackViewPick res;
	ISpinnerControl *spin;

	ExprControlDialogData *ecdata = DLGetWindowLongPtr<ExprControlDialogData *>(hDlg);
	ExprControl *ec = NULL;
	DialogResizer *resizer = NULL;
	if (ecdata)
	{
		ec = ecdata->ec;
		resizer = ecdata->resizer;
	}

	// ok for these to be shared across multiple dialogs
	static int				l_offset1	= 0;
	static int				l_offset2	= 0;
	static int				l_orig_expr_height	= 0;
	static bool				l_save_undo_rec			= false;

	switch (msg) {
	case WM_INITDIALOG:	

		ec = (ExprControl *)lParam;
		resizer = new DialogResizer;
		ecdata = new ExprControlDialogData;
		ecdata->ec = ec;
		ecdata->resizer = resizer;
		DLSetWindowLongPtr(hDlg, ecdata);

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, GetClassLongPtr(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));	// 020725  --prs.

		// resizer management at init 
		resizer->Initialize(hDlg); // use the current resource size as the minimum

		// lock controls to bottom right
		resizer->SetControlInfo(IDC_SAVE ,			DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_LOAD,			DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_EXPR_DEBUG,		DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_EXPR_EVAL,		DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDOK ,				DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_FUNCTIONS ,		DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_STATIC_TICKS,	DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_STATIC_SECS ,	DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_STATIC_FRAMES ,	DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_STATIC_NT ,		DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_RESIZE,			DialogResizer::kLockToBottomRight);

		// lock controls to bottom left
		resizer->SetControlInfo(IDC_ASGN_TO,			DialogResizer::kLockToBottomLeft);
		resizer->SetControlInfo(IDC_ASGN_CONST,		DialogResizer::kLockToBottomLeft);
		resizer->SetControlInfo(IDC_ASGN_CNTRL,		DialogResizer::kLockToBottomLeft);

		// lock to top left, resize height
		resizer->SetControlInfo(IDC_SCALAR_LIST, DialogResizer::kLockToTopLeft,
			DialogResizer::kHeightChangesWithDialog);
		resizer->SetControlInfo(IDC_VECTOR_LIST, DialogResizer::kLockToTopLeft,
			DialogResizer::kHeightChangesWithDialog);
		resizer->SetControlInfo(IDC_STATIC_SCAL, DialogResizer::kLockToTopLeft);
		resizer->SetControlInfo(IDC_STATIC_VEC, DialogResizer::kLockToTopLeft);

		// lock controls to bottom left, resize to width
		resizer->SetControlInfo(IDC_CUR_ASGN ,		DialogResizer::kLockToBottomLeft,
			DialogResizer::kWidthChangesWithDialog);

		// expression and description controls init
		resizer->SetControlInfo(IDC_STATIC_EXPR, DialogResizer::kLockToTopLeft,
			DialogResizer::kHeightChangesWithDialog | DialogResizer::kWidthChangesWithDialog);
		resizer->SetControlInfo(IDC_EXPR_EDIT, DialogResizer::kLockToTopLeft,
			DialogResizer::kHeightChangesWithDialog | DialogResizer::kWidthChangesWithDialog);

		resizer->SetControlInfo(IDC_STATIC_DESCR, DialogResizer::kLockToBottomLeft,
			DialogResizer::kHeightChangesWithDialog | DialogResizer::kWidthChangesWithDialog);
		resizer->SetControlInfo(IDC_DESCRIPTION, DialogResizer::kLockToBottomLeft,
			DialogResizer::kHeightChangesWithDialog | DialogResizer::kWidthChangesWithDialog);

		// expression and description controls init
		InitResizeData( hDlg, l_offset1, l_offset2, l_orig_expr_height);

		// load previously saved dialog position
		DialogResizer::LoadDlgPosition(hDlg, EXPR_CNAME);

		// proportional expression and description resiszing 
		ResizeDialog(hDlg, l_offset1, l_offset2, l_orig_expr_height);

		ec->hDlg = hDlg;
		spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
		spin->SetLimits( -1000000, 1000000, FALSE );
		spin->SetScale( 1.0f );
		spin->LinkToEdit( GetDlgItem(hDlg, IDC_OFFSET), EDITTYPE_INT );
		spin->SetValue( 0, FALSE );
		ReleaseISpinner( spin );
		SendDlgItemMessage(hDlg, IDC_EXPR_EDIT, WM_SETFONT, (WPARAM)ec->hFixedFont, TRUE);
		CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_SCALAR_RB);
		ec->updateVarList();
		EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 0);
		SetDlgItemText(hDlg, IDC_EXPR_EDIT, ec->expr.getExprStr());
		GetDlgItemText(hDlg, IDC_EXPR_EDIT, revertVal, 4096);	// 001110  --prs.
		SetDlgItemText(hDlg, IDC_DESCRIPTION, ec->desc);
		SetFocus(GetDlgItem(hDlg, IDC_EXPR_EDIT));
		RegisterNotification(ec->NotifyPreReset, ec, NOTIFY_SYSTEM_PRE_RESET);
		RegisterNotification(ec->NotifyPreReset, ec, NOTIFY_SYSTEM_PRE_NEW);
		RegisterNotification(ec->NotifyPreReset, ec, NOTIFY_FILE_PRE_OPEN);
		
		return FALSE;

	case CC_SPINNER_BUTTONDOWN:
		theHold.Begin();
		l_save_undo_rec = true;
		break;

	case CC_SPINNER_BUTTONUP:
		l_save_undo_rec = false;
		if (HIWORD(wParam))
		{
			theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
			if(ec->edbg)
				ec->edbg->Invalidate();
		}
		else
			theHold.Cancel();
		break;

	case CC_SPINNER_CHANGE:
		switch ( LOWORD(wParam) ) {
		case IDC_OFFSET_SPIN:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				ct = ec->sVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->sVars[i].name) == 0) {
						spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
						if (!HIWORD(wParam)) {
							theHold.Begin();
							if (theHold.Holding())
								theHold.Put(new ExprSVarChange(ec, i));
							theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
							if(ec->edbg)
								ec->edbg->Invalidate();
						}
						if (l_save_undo_rec) {
							l_save_undo_rec = false;
							if (theHold.Holding())
								theHold.Put(new ExprSVarChange(ec, i));
						}
						ec->sVars[i].offset = spin->GetIVal();
						ReleaseISpinner(spin);
						break;
					}
				}
				if(i < ct)
					break;

				ct = ec->vVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->vVars[i].name) == 0) {
						spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
						if (!HIWORD(wParam)) {
							theHold.Begin();
							if (theHold.Holding())
								theHold.Put(new ExprVVarChange(ec, i));
							theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
							if(ec->edbg)
								ec->edbg->Invalidate();
						}
						if (l_save_undo_rec) {
							l_save_undo_rec = false;
							if (theHold.Holding())
								theHold.Put(new ExprVVarChange(ec, i));
						}
						ec->vVars[i].offset = spin->GetIVal();
						ReleaseISpinner(spin);
						break;
					}
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXPR_DEBUG:
			if(ec->edbg)
				ShowWindow(ec->edbg->hWnd, SW_SHOWNORMAL);
			else
				ec->edbg = new ExprDebug(hDlg, ec);
			updateExpr(hDlg, ec, false);
			if(ec->edbg)			// 001019  --prs.
				ec->edbg->Invalidate();
			break;
		case IDC_FUNCTIONS:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_FUNC_LIST), hDlg, functionListProc);
			break;
		case IDC_CREATE_VAR:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				if(invalidName(buf)) {
					_tcscpy(buf2, GetString(IDS_DB_BAD_NAME));
					MessageBox(hDlg, buf2, GetString(IDS_DB_CANT_CREATE_VAR), 
							MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
					break;
				}
				theHold.Begin();
				if (theHold.Holding())
					theHold.Put(new ExprVarRestore(ec, true));
				type = IsDlgButtonChecked(hDlg, IDC_SCALAR_RB) ? SCALAR_VAR : VECTOR_VAR;
				ec->updRegCt(slot = ec->expr.defVar(type, buf), type);
				spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
				i = spin->GetIVal();
				ReleaseISpinner(spin);
				if(slot >= 0) {
					ec->dfnVar(type, buf, slot, i);
					theHold.Accept(GetString(IDS_LAM_EXPR_VAR_ADD));
				}
				else {
					_tcscpy(buf2, GetString(IDS_DB_DUPNAME));
					MessageBox(hDlg, buf2, GetString(IDS_DB_CANT_CREATE_VAR), 
							MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
					theHold.Cancel();
				}
			}
			ec->updateVarList();
			slot = type == SCALAR_VAR ? IDC_SCALAR_LIST : IDC_VECTOR_LIST;
			ct = SendDlgItemMessage(hDlg, slot, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++) {
				SendDlgItemMessage(hDlg, slot, LB_GETTEXT, i, (LPARAM)buf2);
				if(_tcscmp(buf, buf2) == 0) {
					SendDlgItemMessage(hDlg, slot, LB_SETCURSEL, i, 0);
					break;
				}
			}
			SendMessage(hDlg, WM_COMMAND, slot | (LBN_SELCHANGE << 16), 0); 
			break;
		case IDC_DELETE_VAR:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			theHold.Begin();
			if(buf[0]) {
				ct = ec->sVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->sVars[i].name) == 0) {
						if (theHold.Holding())
							theHold.Put(new ExprVarRestore(ec, true));
						ec->expr.deleteVar(buf);
						delete ec->sVars[i].name;
						if (ec->sVars[i].refID != -1)
							ec->refTab[ec->sVars[i].refID].refCt--;
						ec->sVars.Delete(i, 1);
						SetDlgItemText(hDlg, IDC_VAR_NAME, _T(""));
						break;
					}
				}
				if(i < ct) {
					SetDlgItemText(hDlg, IDC_CUR_ASGN, _T(""));
					ec->updateVarList();
					ec->dropUnusedRefs();
					theHold.Accept(GetString(IDS_LAM_EXPR_VAR_DELETE));
					break;
				}

				ct = ec->vVars.Count();
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->vVars[i].name) == 0) {
						if (theHold.Holding())
							theHold.Put(new ExprVarRestore(ec, true));
						ec->expr.deleteVar(buf);
						delete ec->vVars[i].name;
						if (ec->vVars[i].refID != -1)
							ec->refTab[ec->vVars[i].refID].refCt--;
						ec->vVars.Delete(i, 1);
						SetDlgItemText(hDlg, IDC_VAR_NAME, _T(""));
						break;
					}
				}
				if(i < ct) {
					SetDlgItemText(hDlg, IDC_CUR_ASGN, _T(""));
					ec->updateVarList();
					ec->dropUnusedRefs();
					theHold.Accept(GetString(IDS_LAM_EXPR_VAR_DELETE));
					break;
				}
				_stprintf(buf2, GetString(IDS_DB_CANTDELETE), buf);
				MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			}
			else {
				_tcscpy(buf2, GetString(IDS_DB_NOTHINGDEL));
				MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
			}
			theHold.Cancel();
			break;
		case IDC_RENAME_VAR:
			GetDlgItemText(hDlg, IDC_VAR_NAME, buf, 256);
			if(buf[0]) {
				TSTR oldName;
				TSTR newName = buf;
				int which = 0;
				bool found = false;
				i = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCURSEL, 0, 0);
				if (i != LB_ERR) {
					which = SCALAR_VAR;
					SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf2);
					oldName = buf2;
					slot = ec->findIndex(which,oldName);
					found = true;
				}
				if (!found) {
					i = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCURSEL, 0, 0);
					if (i != LB_ERR) {
						which = VECTOR_VAR;
						SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf2);
						oldName = buf2;
						slot = ec->findIndex(which,oldName);
						found = true;
					}
				}
				if (!found) {
					TSTR buf2 = GetString(IDS_LAM_EXPR_NOTHINGRENAME);
					MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
					break;
				}

				if(_tcscmp(buf, buf2) == 0) 
					break;

				if ((ec->findIndex(SCALAR_VAR, newName) != -1) || (ec->findIndex(VECTOR_VAR, newName) != -1)) {
					TSTR buf2;
					buf2.printf(GetString(IDS_LAM_EXPR_CANTRENAME_VAREXISTS), newName);
					MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);
					break;
				}

				if (which == SCALAR_VAR && slot <= 3) {
					TSTR buf2;
					buf2.printf(GetString(IDS_LAM_EXPR_CANTRENAME_READ_ONLY), oldName);
					MessageBox(hDlg, buf2, GetString(IDS_DB_EXPRCNTL), 
						MB_ICONEXCLAMATION | MB_APPLMODAL | MB_OK);

				}

				theHold.Begin();
				if (theHold.Holding())
				{
					theHold.Put(new ExprInvalidateRestore(ec));
					theHold.Put(new ExprVarRestore(ec, true));
				}
				if (which  == SCALAR_VAR)
					ec->sVars[slot-4].name = newName;
				else
					ec->vVars[slot].name = newName;

				for(int i = 0; i < ec->expr.vars.Count(); i++) {
					if(_tcscmp(ec->expr.vars[i].name, oldName) == 0) {
						ec->expr.vars[i].name = newName;
						ec->updateVarList(true,true);
					}
				}

				theHold.Accept(GetString(IDS_LAM_EXPR_VAR_RENAME));

			}
			break;
		case IDC_EXPR_EVAL:
			updateExpr(hDlg, ec, false);		// 3rd arg added 001110  --prs.
			if(ec->edbg)
				ec->edbg->Invalidate();
			break;
		case IDC_SCALAR_RB:
		case IDC_VECTOR_RB:
		case IDC_VAR_NAME:
		case IDC_OFFSET:
			break;

		case IDC_EXPR_EDIT:
			if(HIWORD(wParam) == EN_SETFOCUS) {
				HWND hEdit = GetDlgItem(hDlg, IDC_EXPR_EDIT);
				int bufLen = GetWindowTextLength(hEdit)+1;
				ec->lastEditedExpr.Resize(bufLen);
				GetWindowText(hEdit, ec->lastEditedExpr, bufLen);
			}
			else if(HIWORD(wParam) == EN_KILLFOCUS) {
				TSTR buf;
				HWND hEdit = GetDlgItem(hDlg, IDC_EXPR_EDIT);
				int bufLen = GetWindowTextLength(hEdit)+1;
				buf.Resize(bufLen);
				GetWindowText(hEdit, buf, bufLen);

				if (_tcscmp(buf, ec->lastEditedExpr) != 0) {
					theHold.Begin();
					if (theHold.Holding())
						theHold.Put(new ExprExprEditRestore(ec, buf));
					theHold.Accept(GetString(IDS_LAM_EXPR_EXPR_CHANGE));
				}
				ec->lastEditedExpr.Resize(0);
			}
			break;

		case IDC_DESCRIPTION:
			if(HIWORD(wParam) == EN_KILLFOCUS) {
					GetDlgItemText(hDlg, IDC_DESCRIPTION, buf, 4096);
				if (_tcscmp(buf, ec->desc) != 0) {
					theHold.Begin();
					if (theHold.Holding())
						theHold.Put(new ExprDescRestore(ec));
					theHold.Accept(GetString(IDS_LAM_EXPR_DESC_CHANGE));
					ec->desc = buf;
				}					
			}
			break;
		case IDC_ASGN_CNTRL:
			ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(SCALAR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
						break;
				}
				FloatFilter ff;
				if(ec->ip->TrackViewPickDlg(hDlg, &res, &ff)) {
					assert(res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID || res.anim->SuperClassID() == CTRL_USERTYPE_CLASS_ID);
					theHold.Begin();
					if (theHold.Holding())
						theHold.Put(new ExprVarRestore(ec));
					ec->assignController(SCALAR_VAR, i, res.client, res.subNum);
					theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
					SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
				}
				break;
			}
			ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(VECTOR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
						break;
				}
				VectorFilter vf;
				if(ec->ip->TrackViewPickDlg(hDlg, &res, &vf)) {
					assert(OKP3Control(res.anim->SuperClassID()) ||
						   res.anim->SuperClassID() == BASENODE_CLASS_ID ||
						   res.anim->SuperClassID() == CTRL_USERTYPE_CLASS_ID);
					theHold.Begin();
					if (theHold.Holding())
						theHold.Put(new ExprVarRestore(ec));
					if (res.anim->SuperClassID()==BASENODE_CLASS_ID) 
						ec->assignNode((INode*)res.anim, i);
					else
						ec->assignController(VECTOR_VAR, i, res.client, res.subNum);
					theHold.Accept(GetString(IDS_RB_PARAMETERCHANGE));
					SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
				}
				break;
			}
			break;
		case IDC_ASGN_CONST:
			ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(SCALAR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
						break;
				}
				ec->curIndex = i;
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SCALAR_ASGN),
							hDlg, ScalarAsgnDlgProc, (LPARAM)ec);
				SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
				break;
			}
			ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < ct; i++)
				if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
					break;
			if(i < ct) {
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
				ct = ec->getVarCount(VECTOR_VAR);
				for(i = 0; i < ct; i++) {
					if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
						break;
				}
				ec->curIndex = i;
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VECTOR_ASGN),
							hDlg, VectorAsgnDlgProc, (LPARAM)ec);
				SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
				break;
			}
			break;
		case IDC_SCALAR_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE) {
				SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
				ct = SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETCOUNT, 0, 0);
				for(i = 0; i < ct; i++)
					if(SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETSEL, i, 0))
						break;
				if(i < ct) {
					SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_GETTEXT, i, (LPARAM)buf);
					ct = ec->getVarCount(SCALAR_VAR);
					for(i = 0; i < ct; i++) 
						if(_tcscmp(buf, ec->getVarName(SCALAR_VAR, i)) == 0)
							break;
					SetDlgItemText(hDlg, IDC_VAR_NAME, buf);
					CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_SCALAR_RB);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
					spin->SetValue(ec->sVars[i].offset, FALSE);
					ReleaseISpinner(spin);
					ReferenceTarget *client = NULL;
					if(ec->sVars[i].refID < 0 || (client = ec->refTab[ec->sVars[i].refID].client) == NULL) {
						_stprintf(buf, "Constant: %g", ec->getScalarValue(i, ec->ip->GetTime()));
						UglyPatch(buf);
					}
					else {
						_tcscpy(buf, ec->getTargetAsString(SCALAR_VAR,i));
					}
					SetDlgItemText(hDlg, IDC_CUR_ASGN, buf);
				}
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 1);
			}
			break;
		case IDC_VECTOR_LIST:
			if(HIWORD(wParam) == LBN_SELCHANGE) {
				SendDlgItemMessage(hDlg, IDC_SCALAR_LIST, LB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
				ct = SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETCOUNT, 0, 0);
				for(i = 0; i < ct; i++)
					if(SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETSEL, i, 0))
						break;
				if(i < ct) {
					SendDlgItemMessage(hDlg, IDC_VECTOR_LIST, LB_GETTEXT, i, (LPARAM)buf);
					ct = ec->getVarCount(VECTOR_VAR);
					for(i = 0; i < ct; i++) 
						if(_tcscmp(buf, ec->getVarName(VECTOR_VAR, i)) == 0)
							break;
					SetDlgItemText(hDlg, IDC_VAR_NAME, buf);
					CheckRadioButton(hDlg, IDC_SCALAR_RB, IDC_VECTOR_RB, IDC_VECTOR_RB);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_OFFSET_SPIN));
					spin->SetValue(ec->vVars[i].offset, FALSE);
					ReleaseISpinner(spin);
					ReferenceTarget *client = NULL;
					if(ec->vVars[i].refID < 0 || (client = ec->refTab[ec->vVars[i].refID].client) == NULL) {
						pt = ec->getVectorValue(i, ec->ip->GetTime());
						_stprintf(buf, "Constant: [%g; %g; %g]", pt.x, pt.y, pt.z);
						UglyPatch(buf);
					}
					else {
						_tcscpy(buf, ec->getTargetAsString(VECTOR_VAR,i));
					}
					SetDlgItemText(hDlg, IDC_CUR_ASGN, buf);
				}
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CONST), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_ASGN_CNTRL), 1);
			}
			break;
		case IDOK:
		case IDCANCEL:
			if(updateExpr(hDlg, ec,true))		// 3rd argument added 001110  --prs.
				DestroyWindow(hDlg);
			break;

		case IDC_SAVE:
			SaveExpression(hDlg, ec);
			break;

		case IDC_LOAD:
			LoadExpression(hDlg, ec);
			break;
		}
		break;

	case WM_PAINT:
		if (ec->doDelayedUpdateVarList)
		{
			ec->doDelayedUpdateVarList = false;
			ec->updateVarList(ec->delayedUpdateVarListMaintainSelection);
		}
		return 0;

	case WM_DESTROY: 
		ec->hDlg = NULL;
		ec->lastEditedExpr.Resize(0);

		// save the last dialog position
		DialogResizer::SaveDlgPosition(hDlg, EXPR_CNAME);

		UnRegisterNotification(ec->NotifyPreReset, ec, NOTIFY_SYSTEM_PRE_RESET);
		UnRegisterNotification(ec->NotifyPreReset, ec, NOTIFY_SYSTEM_PRE_NEW);
		UnRegisterNotification(ec->NotifyPreReset, ec, NOTIFY_FILE_PRE_OPEN);

		delete ecdata;
		delete resizer;

		break;

	case WM_SIZING:
		resizer->Process_WM_SIZING(wParam, lParam);
		return TRUE;

	case WM_SIZE:
		resizer->Process_WM_SIZE(wParam, lParam);
		// description resizing 
		ResizeDialog(hDlg, l_offset1, l_offset2, l_orig_expr_height);
		return TRUE;

	case WM_CUSTEDIT_ENTER:
		if(wParam == IDC_OFFSET)
			DisableAccelerators();
		return FALSE;

	case WM_NCACTIVATE:
		if(wParam)
			PostMessage(hDlg,WM_DISABLE_ACCEL, 0, 0);
		else
			EnableAccelerators();
		return FALSE;

	case WM_CAPTURECHANGED:
	case WM_DISABLE_ACCEL:
		DisableAccelerators();
		return FALSE;

	default:
		return 0;			
	}
	return 1;
}


void ExprControl::EditTrackParams(
			TimeValue t,	// The horizontal position of where the user right clicked.
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags)
{
	if (!hDlg) {
		this->ip = ip;
		CreateDialogParam(
			hInstance,
			MAKEINTRESOURCE(IDD_EXPRPARAMS),
			hParent,
			ExprParamsWndProc,
			(LPARAM)this);
		TSTR title = TSTR(GetString(IDS_RB_EXPRESSIONCONTROLTITLE)) + TSTR(pname);
		SetWindowText(hDlg,title);

	} 
	else {
		SetActiveWindow(hDlg);
	}

}
#define EXPR_STR_CHUNK		0x5000
#define EXPR_RANGE_CHUNK	0x5001
#define EXPR_REFTAB_SIZE	0x5002
#define EXPR_SVAR_TABSIZE	0x5003
#define EXPR_VVAR_TABSIZE	0x5005
#define EXPR_VAR_NAME		0x5006
#define EXPR_VAR_REFID		0x5007
#define EXPR_VAR_FLOAT		0x5008
#define EXPR_VAR_POINT3		0x5009
#define EXPR_VAR_SUBNUM		0x500a
#define EXPR_DESCRIPTION	0x500b
#define EXPR_REFTAB_REFCT	0x500c
#define EXPR_VAR_OFFSET		0x500d
#define EXPR_RANGEEDITED_CHUNK	0x500e
#define EXPR_VERSION			0x500f
#define EXPR_THROW_ON_ERR_CHUNK	0x5010

#define EXPR_SVAR_ENTRY0	0x6000
#define EXPR_SVAR_ENTRYN	0x6fff
#define EXPR_VVAR_ENTRY0	0x7000
#define EXPR_VVAR_ENTRYN	0x7fff

IOResult ExprControl::Save(ISave *isave)
{
	ULONG 	nb;
	int		i, ct, intVar;

	Control::Save(isave); // RB: this will handle saving ORTs

	isave->BeginChunk(EXPR_VERSION);
	isave->Write(&CurrentExprControlVersion, sizeof(BYTE), &nb);
	isave->EndChunk();

	if(desc.Length()) {
	 	isave->BeginChunk(EXPR_DESCRIPTION);
		isave->WriteCString(desc);
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
 	isave->EndChunk();

 	isave->BeginChunk(EXPR_REFTAB_SIZE);
	intVar = refTab.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = refTab.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_REFTAB_REFCT);
		isave->Write(&i, sizeof(int), &nb);
		isave->Write(&refTab[i].refCt, sizeof(int), &nb);
		isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_SVAR_TABSIZE);
	intVar = sVars.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = sVars.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_SVAR_ENTRY0+i);
	 	 isave->BeginChunk(EXPR_VAR_NAME);
		 isave->WriteCString(sVars[i].name);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_REFID);
		 isave->Write(&sVars[i].refID, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_FLOAT);
		 isave->Write(&sVars[i].val, sizeof(float), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_SUBNUM);
		 isave->Write(&sVars[i].subNum, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_OFFSET);
		 isave->Write(&sVars[i].offset, sizeof(int), &nb);
 		 isave->EndChunk();
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_VVAR_TABSIZE);
	intVar = vVars.Count();
	isave->Write(&intVar, sizeof(intVar), &nb);
 	isave->EndChunk();

	ct = vVars.Count();
	for(i = 0; i < ct; i++) {
	 	isave->BeginChunk(EXPR_VVAR_ENTRY0+i);
	 	 isave->BeginChunk(EXPR_VAR_NAME);
		 isave->WriteCString(vVars[i].name);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_REFID);
		 isave->Write(&vVars[i].refID, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_POINT3);
		 isave->Write(&vVars[i].val, sizeof(Point3), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_SUBNUM);
		 isave->Write(&vVars[i].subNum, sizeof(int), &nb);
 		 isave->EndChunk();
	 	 isave->BeginChunk(EXPR_VAR_OFFSET);
		 isave->Write(&vVars[i].offset, sizeof(int), &nb);
 		 isave->EndChunk();
	 	isave->EndChunk();
	}

 	isave->BeginChunk(EXPR_STR_CHUNK);
	isave->WriteCString(expr.getExprStr());
 	isave->EndChunk();
	
	isave->BeginChunk(EXPR_THROW_ON_ERR_CHUNK);
	isave->Write(&bThrowOnError, sizeof(bool), &nb);
	isave->EndChunk();

	return IO_OK;
}

// RB 3/19/99: Check for refs to path controller. These must be refs to the percent track made from
// MAX2.5 or earlier. Convert to paramblock 2 refs.
class CheckForPathContRefsPLCB : public PostLoadCallback {
	public:
		ExprControl *cont;
		CheckForPathContRefsPLCB(ExprControl *c) {cont = c;}
		void proc(ILoad *iload) {
			for (int i=0; i<cont->refTab.Count(); i++) {
				ReferenceTarget* client = cont->refTab[i].client;
				if (!client) continue;
				if (client->SuperClassID()==CTRL_POSITION_CLASS_ID &&
					client->ClassID()==Class_ID(PATH_CONTROL_CLASS_ID,0)) {
										
					cont->ReplaceReference(i+cont->StdControl::NumRefs(),
						client->GetReference(PATHPOS_PBLOCK_REF));
					}
				}
			delete this;
			}
	};

class UpdateNodeRefsToWatchersPLCB : public PostLoadCallback {
public:
	ExprControl *cont;
	UpdateNodeRefsToWatchersPLCB(ExprControl *c) {cont = c;}
	int Priority() { return 6; }
	void proc(ILoad *iload) {
		for (int i=0; i<cont->refTab.Count(); i++) {
			ReferenceTarget* client = cont->refTab[i].client;
			if (!client) continue;
			if (client->SuperClassID()==BASENODE_CLASS_ID)
				cont->ReplaceReference(i+cont->StdControl::NumRefs(), new NodeTransformMonitor((INode*)client));
		}
		delete this;
	}
};

class CheckWatchersForNullPLCB : public PostLoadCallback {
public:
	ExprControl *cont;
	CheckWatchersForNullPLCB(ExprControl *c) {cont = c;}
	int Priority() { return 7; }
	void proc(ILoad *iload) {
		for (int i = cont->refTab.Count()-1; i >= 0; i--) {
			ReferenceTarget* client = cont->refTab[i].client;
			if (!client) continue;
			if (client->SuperClassID()==REF_TARGET_CLASS_ID &&
				client->ClassID()==NODETRANSFORMMONITOR_CLASS_ID) {

				INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
				if (ntm->GetNode() == NULL)
				{
					DbgAssert(false);  // as a dependent, the node should always have been saved and loaded
					client->DeleteThis();
				}
			}
		}
		delete this;
	}
};

// private namespace
namespace
{
	class FindPBOwnerDEP : public DependentEnumProc 
	{
	public:		
		ReferenceTarget*	rm;
		IParamBlock*		pb;
		FindPBOwnerDEP(IParamBlock* ipb) { rm = NULL; pb = ipb; }
		int proc(ReferenceMaker* rmaker)
		{
			if (rmaker == pb)
				return DEP_ENUM_CONTINUE;

			if( !rmaker->IsRealDependency(pb) ) 
				return DEP_ENUM_SKIP;

			SClass_ID sid = rmaker->SuperClassID();
			if (sid != MAKEREF_REST_CLASS_ID  && sid != MAXSCRIPT_WRAPPER_CLASS_ID && sid != DELREF_REST_CLASS_ID)
			{
				rm = (ReferenceTarget*)rmaker;
				return DEP_ENUM_HALT;
			}
			return DEP_ENUM_SKIP; // only need to look at immediate dependents
		}
	};
}

class RemoveUnownedPBPLCB : public PostLoadCallback {
public:
	ExprControl *cont;
	RemoveUnownedPBPLCB(ExprControl *c) {cont = c;}
	int Priority() { return 10; }
	void proc(ILoad *iload) {
		bool doUpdate = false;
		for (int i = cont->refTab.Count()-1; i >= 0; i--) {
			ReferenceTarget* client = cont->refTab[i].client;
			if (!client) 
			{
				doUpdate = true;
				continue;
			}
			if (client->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID && ((IParamBlock2*)client)->GetOwner() == NULL)
			{
				// unowned PB2 - delete 
				cont->DeleteReference(i+cont->StdControl::NumRefs());
				doUpdate = true;
			}
			else if (client->SuperClassID() == PARAMETER_BLOCK_CLASS_ID)
			{
				// look for an owner....
				FindPBOwnerDEP fpbo ((IParamBlock*)client);
				client->DoEnumDependents(&fpbo);
				if (fpbo.rm == NULL)
				{
					// unowned PB - delete
					cont->DeleteReference(i+cont->StdControl::NumRefs());
					doUpdate = true;
				}
			}
		}
		if (doUpdate)
			cont->dropUnusedRefs();
		delete this;
	}
};

IOResult ExprControl::Load(ILoad *iload)
{
	ULONG 	nb;
	TCHAR	*cp;
	int		id, i, varIndex, intVar;
	IOResult res;
	VarRef	dummyVarRef;

	BYTE	loadExprControlVersion = 0;

	Control::Load(iload); // RB: this will handle loading ORTs

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (id = iload->CurChunkID()) {
		case EXPR_VERSION:
			iload->Read(&loadExprControlVersion, sizeof(BYTE), &nb);
			break;
		case EXPR_STR_CHUNK:
			iload->ReadCStringChunk(&cp);
			expr.load(cp);
			break;
		case EXPR_DESCRIPTION:
			iload->ReadCStringChunk(&cp);
			desc = cp;
			break;
		case EXPR_RANGE_CHUNK:
			iload->Read(&range, sizeof(range), &nb);
			break;
		case EXPR_REFTAB_SIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			refTab.SetCount(intVar);
			for(i = 0; i < intVar; i++) {
				refTab[i].refCt = 0;
				refTab[i].client = NULL;
			}
			break;
		case EXPR_REFTAB_REFCT:
			iload->Read(&intVar, sizeof(int), &nb);
			if(intVar < refTab.Count())		// this should always be true!!!!
				iload->Read(&refTab[intVar].refCt, sizeof(int), &nb);
			break;
		case EXPR_SVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			sVars.SetCount(intVar);
			for(i = 0; i < intVar; i++)
				memset(&sVars[i], 0, sizeof(SVar));
			break;
		case EXPR_VVAR_TABSIZE:
			iload->Read(&intVar, sizeof(intVar), &nb);
			vVars.SetCount(intVar);
			for(i = 0; i < intVar; i++)
				memset(&vVars[i], 0, sizeof(VVar));
			break;
		case EXPR_THROW_ON_ERR_CHUNK:
			iload->Read(&bThrowOnError, sizeof(bool), &nb);
			break;
		}
		if(id >= EXPR_SVAR_ENTRY0 && id <= EXPR_SVAR_ENTRYN) {
			varIndex = id - EXPR_SVAR_ENTRY0;
			assert(varIndex < sVars.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case EXPR_VAR_NAME:
					iload->ReadCStringChunk(&cp);
					sVars[varIndex].name = cp;
					break;
				case EXPR_VAR_REFID:
					iload->Read(&sVars[varIndex].refID, sizeof(int), &nb);
					break;
				case EXPR_VAR_SUBNUM:
					iload->Read(&sVars[varIndex].subNum, sizeof(int), &nb);
					break;
				case EXPR_VAR_OFFSET:
					iload->Read(&sVars[varIndex].offset, sizeof(int), &nb);
					break;
				case EXPR_VAR_FLOAT:
					iload->Read(&sVars[varIndex].val, sizeof(float), &nb);
					break;
				}	
				iload->CloseChunk();
			}
			updRegCt(intVar = expr.defVar(SCALAR_VAR, sVars[varIndex].name), SCALAR_VAR);
			sVars[varIndex].regNum = intVar;
		}
		else if(id >= EXPR_VVAR_ENTRY0 && id <= EXPR_VVAR_ENTRYN) {
			varIndex = id - EXPR_VVAR_ENTRY0;
			assert(varIndex < vVars.Count());
			while (IO_OK == iload->OpenChunk()) {
				switch (iload->CurChunkID()) {
				case EXPR_VAR_NAME:
					iload->ReadCStringChunk(&cp);
					vVars[varIndex].name = cp;
					break;
				case EXPR_VAR_REFID:
					iload->Read(&vVars[varIndex].refID, sizeof(int), &nb);
					break;
				case EXPR_VAR_SUBNUM:
					iload->Read(&vVars[varIndex].subNum, sizeof(int), &nb);
					break;
				case EXPR_VAR_OFFSET:
					iload->Read(&vVars[varIndex].offset, sizeof(int), &nb);
					break;
				case EXPR_VAR_POINT3:
					iload->Read(&vVars[varIndex].val, sizeof(Point3), &nb);
					break;
				}	
				iload->CloseChunk();
			}
			updRegCt(intVar = expr.defVar(VECTOR_VAR, vVars[varIndex].name), VECTOR_VAR);
			vVars[varIndex].regNum = intVar;
		}
		iload->CloseChunk();
	}
	
	// RB 3/19/99: Refs to pathController\percent need to be changed to refer to the parameter block.
	iload->RegisterPostLoadCallback(new CheckForPathContRefsPLCB(this));

	if (loadExprControlVersion == 0)
		iload->RegisterPostLoadCallback(new UpdateNodeRefsToWatchersPLCB(this));

	iload->RegisterPostLoadCallback(new CheckWatchersForNullPLCB(this));
	iload->RegisterPostLoadCallback(new RemoveUnownedPBPLCB(this));

	return IO_OK;
}

TSTR ExprControl::getTargetAsString(int type, int i)
{
	TSTR out = _T("<< unknown >>");
	if((i < getVarCount(type)) && (i >= 0)) 
	{
		int subNum = type == SCALAR_VAR ? sVars[i].subNum : vVars[i].subNum;
		int	refID = type == SCALAR_VAR ? sVars[i].refID : vVars[i].refID;
		if(refID >= 0)
		{
			ReferenceTarget* client = refTab[refID].client;
			if (client)
			{
				INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
				if (ntm)
					client = ntm->GetNode();
			}
			Find_MXS_Name_For_Obj(client, out); 
			int nChars = out.Length();
			if (client && subNum >= 0)
			{
				out.append(_T("."));
				TSTR subAnimName = client->SubAnimName(subNum);
				bool hasPunc = false;
				int len = subAnimName.Length();
				for (int i = 0; i < len; i++)
					if (!_istalnum(subAnimName[i]))
					{
						hasPunc = true;
						break;
					}
					if (hasPunc)
						out.append(_T("'"));
					out.append(subAnimName);
					if (hasPunc)
						out.append(_T("'"));
			}
		}
	}
	return out;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


void ExprControl::SetThrowOnError(bool bOn)
{
	bThrowOnError = bOn;
}

bool ExprControl::GetThrowOnError()
{
	return bThrowOnError;
}

TSTR ExprControl::PrintDetails()
{
	int		i = 0, slot = 0;
	TSTR	pdString, ts_temp;
	ReferenceTarget *client = NULL;

	pdString = GetString(IDS_LAM_EXPR_PRINT_FMT1);

	ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT2), desc, expr.getExprStr());
	pdString += ts_temp;

	// Scalar
	int sCount = sVars.Count();
	ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT3), sCount);	pdString += ts_temp;
	for (i=0; i<sCount; i++) {
		if (sVars[i].refID < 0 || (client = refTab[sVars[i].refID].client) == NULL) {
			ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT4), i+1, sVars[i].name, sVars[i].val);
		} else {
			slot = sVars[i].refID;

			// Add Some error Checking
			if (slot >= refTab.Count()) {
				ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT5), i+1, sVars[i].name, sVars[i].offset);
			} else {
				TSTR nname;
				getNodeName(client,nname);
				TSTR pname;
				if (nname.Length())
					pname = nname + TSTR(_T("\\")) + client->SubAnimName(sVars[i].subNum);
				else 
					pname = client->SubAnimName(sVars[i].subNum);
				ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT6), i+1, sVars[i].name, pname, sVars[i].offset);
			}
		}
		pdString += ts_temp;
	}

	// Vector
	int vCount = vVars.Count();
	ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT8), vCount); pdString += ts_temp;
	for (i=0; i<vCount; i++) {
		if (vVars[i].refID < 0 || (client = refTab[vVars[i].refID].client) == NULL)
			ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT9), i+1, vVars[i].name, vVars[i].val.x, vVars[i].val.y, vVars[i].val.z);
		else {
			slot = vVars[i].refID;
			if (slot >= refTab.Count()) {
				ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT5), i+1, vVars[i].name, vVars[i].offset);
			} else {
				// Print Node Details
				if (vVars[i].subNum < 0) {
					INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
					INode *theNode = NULL;
					if (ntm) 
						theNode = ntm->GetNode();
					ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT7), i+1, vVars[i].name, (theNode) ? theNode->GetName() : "", vVars[i].offset);
				} else {
					TSTR nname;
					getNodeName(client,nname);
					TSTR pname;
					if (nname.Length())
						pname = nname + TSTR(_T("\\")) + client->SubAnimName(vVars[i].subNum);
					else 
						pname = client->SubAnimName(vVars[i].subNum);
					ts_temp.printf(GetString(IDS_LAM_EXPR_PRINT_FMT6), i+1, vVars[i].name, pname, vVars[i].offset);
				}
			}
		}
		pdString += ts_temp;
	}
	return pdString;
}


BOOL ExprControl::SetExpression(TSTR &expression)
{
	TSTR	ts_temp = expr.getExprStr();

	disabledDueToRecursion = false;
	if (expr.load(expression)) {
		TSTR progString = expr.getProgressStr();
		expr.load(ts_temp);
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETEXPR_PARSE_ERR), progString);
		return FALSE;
	}
	if (theHold.Holding())
	{
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprExprRestore(this, ts_temp));
	}

	if (hDlg)
	{
		SetDlgItemText(hDlg, IDC_EXPR_EDIT, expression);
		lastEditedExpr = expression;

	}

	if(edbg)
		edbg->Invalidate();

	// Update the scene!
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_MAXCONTROL_NOT_NEW,MAXSCRIPT_WRAPPER_CLASS_ID,FALSE,this);

	return TRUE;
}


TSTR ExprControl::GetExpression()
{
	return (_T(expr.getExprStr()));
}

int	ExprControl::NumScalars()
{
	return (sVars.Count()+4);
}

int	ExprControl::NumVectors()
{
	return	(vVars.Count());
}

BOOL ExprControl::assignScalarValue(int i, float val, int offset, BOOL fromFPS)
{
	if(i >= 0 && i < sVars.Count()) {
		if (sVars[i].refID != -1)
			refTab[sVars[i].refID].refCt--;
		sVars[i].refID = -1;
		sVars[i].val = val;
		if (offset != TIME_NegInfinity)
			sVars[i].offset = offset;
		if (fromFPS)
			updateVarList(true,true);
		return TRUE;
	}
	if  (fromFPS && bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_SCALAR_INDEX), Integer::intern(i+1));
	return FALSE;
}

BOOL ExprControl::assignVectorValue(int i, Point3 &val, int offset, BOOL fromFPS)
{
	if(i >= 0 && i < vVars.Count()) {
		if (vVars[i].refID != -1)
			refTab[vVars[i].refID].refCt--;
		vVars[i].refID = -1;
		vVars[i].val = val;
		if (offset != TIME_NegInfinity)
			vVars[i].offset = offset;
		if (fromFPS)
			updateVarList(true,true);
		return TRUE;
	}
	if  (fromFPS && bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VECTOR_INDEX), Integer::intern(i+1));
	return FALSE;
}

TSTR ExprControl::GetDescription()
{
	return desc;
}

BOOL ExprControl::SetDescription(TSTR &desc)
{
	if (theHold.Holding())
		theHold.Put(new ExprDescRestore(this));

	this->desc = desc;

	if (hDlg)
		SetDlgItemText(hDlg, IDC_DESCRIPTION, desc);
	return TRUE;
}

int	ExprControl::findIndex(int type, TSTR &name)
{
	int		i, ct;

	if (type == SCALAR_VAR) {
		ct = expr.getVarCount(SCALAR_VAR);
		for (i=0; i< ct; i++) {
			// if a match, return index
			if (!_tcscmp(name, expr.getVarName(SCALAR_VAR, i))) {
				return i;
			}
		}
	} else {
		ct = expr.getVarCount(VECTOR_VAR);
		for (i=0; i< ct; i++) {
			// if a match, return index
			if (!_tcscmp(name, expr.getVarName(VECTOR_VAR, i))) {
				return i;
			}
		}
	}

	return -1;
}

BOOL ExprControl::assignNode(INode* node, int varIndex, int offset)
{
	if (!node) return FALSE;

	int i, ct;
	// Indicate multiple references
	ct = refTab.Count();
	for(i = 0; i < ct; i++) {
		ReferenceTarget *client = refTab[i].client;
		if (client == NULL) continue;
		INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
		if(ntm && ntm->GetNode() == node) {
			refTab[i].refCt++;
			break;
		}
	}
	if(i >= ct) {
		NodeTransformMonitor *ntm = new NodeTransformMonitor(node);
		VarRef vr(NULL);
		i = refTab.Append(1, &vr, 4);
		ReplaceReference(StdControl::NumRefs()+i, ntm);
	}

	if(varIndex >= vVars.Count()) {
		refTab[i].refCt--;
		return FALSE;
	}

	if (vVars[varIndex].refID != -1)
		refTab[vVars[varIndex].refID].refCt--;

	vVars[varIndex].refID = i;
	vVars[varIndex].subNum = -1;
	if (offset != TIME_NegInfinity)
		vVars[varIndex].offset = offset;
	updateVarList(true,true);
	dropUnusedRefs();
	return TRUE;
}


BOOL ExprControl::SetVectorNode(Value* which, INode* node)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETVECNODE_VAR_NOT_EXIST), which);
		return FALSE;
	}

	return AddVectorNode(vVars[index].name, node, TIME_NegInfinity);
}

BOOL ExprControl::AddVectorNode(TSTR &name, INode* node, int offset)
{
	if(invalidName(name)) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VAR_NAME), name);
		return FALSE;
	}

	// removed circular dependency test for nodes - not needed since using NodeTransformMonitor
/*
	//check to make sure it is not circular
	node->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_CIRCULAR_DEPENDENCY),MAXClass::make_wrapper_for(node));
		return FALSE;
	}
*/

	if (theHold.Holding())
	{
		theHold.SuperBegin();
		theHold.Begin();
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprVarRestore(this, true));
	}

	int	slot	= expr.defVar(VECTOR_VAR, name);
	if (slot < 0) {
		slot = findIndex(VECTOR_VAR, name);
		if (slot >= 0) {
			BOOL res = assignNode(node, slot, offset);
			if (res && theHold.Holding())
			{
				theHold.Accept(NULL);
				theHold.SuperAccept(NULL);
			}
			else if (theHold.Holding())
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			return res;
		}
		if (theHold.Holding())
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECNODE_VARNAME_SCALAR), name);
		return FALSE;
	}
	updRegCt(slot, VECTOR_VAR);
	dfnVar(VECTOR_VAR, name, slot);

	slot = vVars.Count()-1;
	BOOL res = assignNode(node, slot, offset);
	if (!res) {
		expr.deleteVar(name);
		delete vVars[slot].name;
		vVars.Delete(slot, 1);
	}
	if (res && theHold.Holding())
	{
		theHold.Accept(NULL);
		theHold.SuperAccept(NULL);
	}
	else if (theHold.Holding())
	{
		theHold.Cancel();
		theHold.SuperCancel();
	}
	return res;
}

BOOL ExprControl::SetVectorTarget(Value* which, Value* target, ReferenceTarget *owner)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETVECTARG_VAR_NOT_EXIST), which);
		return FALSE;
	}

	return AddVectorTarget(vVars[index].name, target, TIME_NegInfinity, owner);
}

BOOL ExprControl::AddVectorTarget(TSTR &name, Value* target, int offset, ReferenceTarget* owner)
{
	if (is_controller(target)) {
		Control *cntrl = target->to_controller();

		// Check that the control is the proper type
		if (!OKP3Control(cntrl->SuperClassID())) {
			if  (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECTARG_NEED_POINT3_CTRL), target);
			return FALSE;
		}

		FindControl(cntrl, owner);
		if (this_client == NULL) {
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECTARG_CANNOT_FIND_TARG), target);
			return FALSE;
		}
	} 
	else if (is_subAnim(target)) {
		MAXSubAnim *targ = (MAXSubAnim*)target;
		deletion_check(targ);

		VectorFilter vf;
		if (!vf.proc(targ->ref->SubAnim(targ->subanim_num), targ->ref, targ->subanim_num)) {
			if (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECTARG_NEED_POINT3_SUBANIM), target);
			return FALSE;
		}

		this_client = targ->ref;
		this_index = targ->subanim_num;
	}
	else {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECTARG_TARG_INVALID), target);
		return FALSE;
	}

	if(invalidName(name)) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VAR_NAME), name);
		return FALSE;
	}

	//check to make sure it is not circular
	((ReferenceTarget *)this_client)->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (((ReferenceTarget *)this_client)->EndDependencyTest()) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_CIRCULAR_DEPENDENCY),target);
		return FALSE;
	}

	if (theHold.Holding())
	{
		theHold.SuperBegin();
		theHold.Begin();
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprVarRestore(this, true));
	}

	int	slot	= expr.defVar(VECTOR_VAR, name);
	if (slot < 0) {
		// find the slot so you can overwrite if necessary
		slot = findIndex(VECTOR_VAR, name);
		if (slot >= 0) {
			// We're replacing, so we should delete the reference
			BOOL res = assignController(VECTOR_VAR, slot, (ReferenceTarget *)this_client, this_index, offset, TRUE);
			if (res && theHold.Holding())
			{
				theHold.Accept(NULL);
				theHold.SuperAccept(NULL);
			}
			else if (theHold.Holding())
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			return res;
		}
		if (theHold.Holding())
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECTARG_VARNAME_SCALAR), name);
		return FALSE;
	}
	updRegCt(slot, VECTOR_VAR);
	dfnVar(VECTOR_VAR, name, slot);

	slot = vVars.Count()-1;
	BOOL res = assignController(VECTOR_VAR, slot, (ReferenceTarget *)this_client, this_index, offset, TRUE);
	if (!res) {
		expr.deleteVar(name);
		delete vVars[slot].name;
		vVars.Delete(slot, 1);
	}
	if (res && theHold.Holding())
	{
		theHold.Accept(NULL);
		theHold.SuperAccept(NULL);
	}
	else if (theHold.Holding())
	{
		theHold.Cancel();
		theHold.SuperCancel();
	}
	return res;
}


BOOL ExprControl::SetScalarTarget(Value* which, Value* target, ReferenceTarget *owner)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}
	if (index >= 0 && index <= 3) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETSCALARTARG_VAR_READONLY), which);
		return FALSE;
	}

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETSCALARTARG_VAR_NOT_EXIST), which);
		return FALSE;
	}

	return AddScalarTarget(sVars[index].name, target, TIME_NegInfinity, owner);
}

BOOL ExprControl::AddScalarTarget(TSTR &name, Value* target, int offset, ReferenceTarget *owner)
{
	if (is_controller(target)) {
		Control *cntrl = target->to_controller();

		// Check that the control is the proper type
		if (cntrl->SuperClassID() != CTRL_FLOAT_CLASS_ID) {
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_NEED_FLOAT_CTRL), target);
			return FALSE;
		}

		FindControl(cntrl, owner);
		if (this_client == NULL) {
			if  (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_CANNOT_FIND_TARG), target);
			return FALSE;
		}
	}
	else if (is_subAnim(target)) {
		MAXSubAnim *targ = (MAXSubAnim*)target;
		deletion_check(targ);

		FloatFilter ff;
		if (!ff.proc(targ->ref->SubAnim(targ->subanim_num), targ->ref, targ->subanim_num)) {
			if (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_NEED_FLOAT_SUBANIM), target);
			return FALSE;
		}

		this_client = targ->ref;
		this_index = targ->subanim_num;
	}
	else {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_TARG_INVALID), target);
		return FALSE;
	}


	if(invalidName(name)) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VAR_NAME), name);
		return FALSE;
	}

	//check to make sure it is not circular
	((ReferenceTarget *)this_client)->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (((ReferenceTarget *)this_client)->EndDependencyTest()) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_CIRCULAR_DEPENDENCY),target);
		return FALSE;
	}

	if (theHold.Holding())
	{
		theHold.SuperBegin();
		theHold.Begin();
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprVarRestore(this, true));
	}

	int	slot	= expr.defVar(SCALAR_VAR, name);
	if (slot < 0) {
		slot = findIndex(SCALAR_VAR, name);
		if (slot >= 0) {
			if (slot <= 3) {
				if (theHold.Holding())
				{
					theHold.Cancel();
					theHold.SuperCancel();
				}
				if  (bThrowOnError)
					throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_VAR_READONLY), name);
				return FALSE;
			}
			BOOL res = assignController(SCALAR_VAR, slot-4, (ReferenceTarget *)this_client, this_index, offset, TRUE);
			if (res && theHold.Holding())
			{
				theHold.Accept(NULL);
				theHold.SuperAccept(NULL);
			}
			else if (theHold.Holding())
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			return res;
		}
		if (theHold.Holding())
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARTARG_VARNAME_VECTOR), name);
		return FALSE;
	}
	updRegCt(slot, SCALAR_VAR);
	dfnVar(SCALAR_VAR, name, slot);

	slot = sVars.Count()-1;
	BOOL res = assignController(SCALAR_VAR, slot, (ReferenceTarget *)this_client, this_index, offset, TRUE);
	if (!res) {
		expr.deleteVar(name);
		delete sVars[slot].name;
		sVars.Delete(slot, 1);
	}
	if (res && theHold.Holding())
	{
		theHold.Accept(NULL);
		theHold.SuperAccept(NULL);
	}
	else if (theHold.Holding())
	{
		theHold.Cancel();
		theHold.SuperCancel();
	}
	return res;
}


BOOL ExprControl::SetScalarConstant(Value* which, float val)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}
	if (index >= 0 && index <= 3) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETSCALARCONST_VAR_READONLY), which);
		return FALSE;
	}

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETSCALARCONST_VAR_NOT_EXIST), which);
		return FALSE;
	}

	return AddScalarConstant(sVars[index].name, val);
}

BOOL ExprControl::AddScalarConstant(TSTR &name, float val)
{
	if(invalidName(name)) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VAR_NAME), name);
		return FALSE;
	}

	if (theHold.Holding())
	{
		theHold.SuperBegin();
		theHold.Begin();
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprVarRestore(this, true));
	}

	int	slot	= expr.defVar(SCALAR_VAR, name);
	if (slot < 0) {
		slot = findIndex(SCALAR_VAR, name);
		if (slot >= 0) {
			if (slot <= 3) {
				if (theHold.Holding())
				{
					theHold.Cancel();
					theHold.SuperCancel();
				}
				if  (bThrowOnError)
					throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARCONST_VAR_READONLY), name);
				return FALSE;
			}
			BOOL res = assignScalarValue(slot-4, val, 0, TRUE);
			if (res && theHold.Holding())
			{
				theHold.Accept(NULL);
				theHold.SuperAccept(NULL);
			}
			else if (theHold.Holding())
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			return res;
		}
		if (theHold.Holding())
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDSCALARCONST_VARNAME_VECTOR), name);
		return FALSE;
	}

	updRegCt(slot, SCALAR_VAR);
	dfnVar(SCALAR_VAR, name, slot);

	slot = sVars.Count()-1;
	BOOL res = assignScalarValue(slot, val, 0, TRUE);
	if (!res) {
		expr.deleteVar(name);
		delete sVars[slot].name;
		sVars.Delete(slot, 1);
	}
	if (res && theHold.Holding())
	{
		theHold.Accept(NULL);
		theHold.SuperAccept(NULL);
	}
	else if (theHold.Holding())
	{
		theHold.Cancel();
		theHold.SuperCancel();
	}
	return res;
}

BOOL ExprControl::SetVectorConstant(Value* which, Point3 point)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_SETVECCONST_VAR_NOT_EXIST), which);
		return FALSE;
	}

	return AddVectorConstant(vVars[index].name, point);
}

BOOL ExprControl::AddVectorConstant(TSTR &name, Point3 point)
{
	if(invalidName(name)) {
		if (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VAR_NAME), name);
		return FALSE;
	}

	if (theHold.Holding())
	{
		theHold.SuperBegin();
		theHold.Begin();
		theHold.Put(new ExprInvalidateRestore(this));
		theHold.Put(new ExprVarRestore(this, true));
	}

	int	slot	= expr.defVar(VECTOR_VAR, name);
	if (slot < 0) {
		slot = findIndex(VECTOR_VAR, name);
		if (slot >= 0) {
			BOOL res = assignVectorValue(slot, point, 0, TRUE);
			if (res && theHold.Holding())
			{
				theHold.Accept(NULL);
				theHold.SuperAccept(NULL);
			}
			else if (theHold.Holding())
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			return res;
		}
		if (theHold.Holding())
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_ADDVECCONST_VARNAME_SCALAR), name);
		return FALSE;
	}

	updRegCt(slot, VECTOR_VAR);
	dfnVar(VECTOR_VAR, name, slot);

	slot = vVars.Count()-1;
	BOOL res = assignVectorValue(vVars.Count()-1, point,0, TRUE);
	if (!res) {
		expr.deleteVar(name);
		delete vVars[slot].name;
		vVars.Delete(slot, 1);
	}
	if (res && theHold.Holding())
	{
		theHold.Accept(NULL);
		theHold.SuperAccept(NULL);
	}
	else if (theHold.Holding())
	{
		theHold.Cancel();
		theHold.SuperCancel();
	}
	return res;
}

BOOL ExprControl::DeleteVariable(TSTR &name)
{
	int i;
	if ((i = findIndex(SCALAR_VAR, name)) != -1) {
		if (i <= 3) {
			if  (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_DELETEVAR_VAR_READONLY), name);
			return FALSE;
		}
		i -= 4;
		if (theHold.Holding())
		{
			theHold.Put(new ExprInvalidateRestore(this));
			theHold.Put(new ExprVarRestore(this, true));
		}
		expr.deleteVar(name);
		delete sVars[i].name;
		if (sVars[i].refID != -1)
			refTab[sVars[i].refID].refCt--;
		sVars.Delete(i, 1);
		updateVarList(true,true);
		dropUnusedRefs();
		return TRUE;
	}
	if ((i=findIndex(VECTOR_VAR, name)) != -1) {
		if (theHold.Holding())
		{
			theHold.Put(new ExprInvalidateRestore(this));
			theHold.Put(new ExprVarRestore(this, true));
		}
		expr.deleteVar(name);
		delete vVars[i].name;
		if (vVars[i].refID != -1)
			refTab[vVars[i].refID].refCt--;
		vVars.Delete(i, 1);
		updateVarList(true,true);
		dropUnusedRefs();
		return TRUE;
	}
	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_DELETEVAR_VAR_NOT_EXIST), name);
	return FALSE;
}

BOOL ExprControl::RenameVariable(TSTR &oldName, TSTR &newName)
{
	if (_tcscmp(oldName, newName) == 0) return FALSE;

	int is = -1;
	int iv = -1;

	if ((findIndex(SCALAR_VAR, newName) != -1) || (findIndex(VECTOR_VAR, newName) != -1)) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_RENAMEVAR_VAR_EXISTS), newName);
		return FALSE;
	}

	if ((is = findIndex(SCALAR_VAR, oldName)) != -1) {
		if (is <= 3) {
			if  (bThrowOnError)
				throw RuntimeError(GetString(IDS_LAM_EXPR_RENAMEVAR_VAR_READONLY), oldName);
			return FALSE;
		}
		if (theHold.Holding())
		{
			theHold.Put(new ExprInvalidateRestore(this));
			theHold.Put(new ExprVarRestore(this, true));
		}
		sVars[is-4].name = newName;
	}
	else if ((iv = findIndex(VECTOR_VAR, oldName)) != -1) {
		if (theHold.Holding())
		{
			theHold.Put(new ExprInvalidateRestore(this));
			theHold.Put(new ExprVarRestore(this, true));
		}
		vVars[iv].name = newName;
	}

	if ((is != -1) || (iv != -1))  {
		for(int i = 0; i < expr.vars.Count(); i++) {
			if(_tcscmp(expr.vars[i].name, oldName) == 0) {
				expr.vars[i].name = newName;
				updateVarList(true,true);
				return TRUE;
			}
		}
		DbgAssert(false);
	}

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_RENAMEVAR_VAR_NOT_EXIST), oldName);
	return FALSE;
}

BOOL ExprControl::VariableExists(TSTR &name)
{
	if (findIndex(SCALAR_VAR, name) != -1)
		return TRUE;
	return (findIndex(VECTOR_VAR, name) != -1);
}

void ExprControl::Update()
{
	disabledDueToRecursion = false;
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

TimeValue ExprControl::GetOffset(TSTR &name)
{
	int i = findIndex(SCALAR_VAR, name);
	if (i != -1) {
		return (i >= 4) ? sVars[i-4].offset : 0;

	}
	else {
		i = findIndex(VECTOR_VAR, name);
		if (i != -1)
			return (vVars[i].offset);
	}

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_GETOFFSET_VAR_NOT_EXIST), name);
	return 0;
}


BOOL ExprControl::SetOffset(TSTR &name, TimeValue tick)
{
	int i = findIndex(SCALAR_VAR, name);
	if (i != -1) {
		if (i >= 4) {
			if (theHold.Holding())
			{
				theHold.Put(new ExprInvalidateRestore(this));
				theHold.Put(new ExprVarRestore(this));
			}
			sVars[i-4].offset = tick;
		}
		if (hDlg) 
			SendMessage(hDlg, WM_COMMAND, IDC_SCALAR_LIST | (LBN_SELCHANGE << 16), 0); 
		return TRUE;
		}
	else {
		i = findIndex(VECTOR_VAR, name);
		if (i != -1) {
			if (theHold.Holding())
			{
				theHold.Put(new ExprInvalidateRestore(this));
				theHold.Put(new ExprVarRestore(this));
			}
			vVars[i].offset = tick;
			if (hDlg)
				SendMessage(hDlg, WM_COMMAND, IDC_VECTOR_LIST | (LBN_SELCHANGE << 16), 0); 
			return TRUE;
		}
	}

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_SETOFFSET_VAR_NOT_EXIST), name);
	return FALSE;
}

float ExprControl::GetScalarConstant(Value* which, TimeValue t)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}

	if (index == 0) // 
		return (float)t;
	else if (index == 1)
		return (float)t/4800.0f;
	else if (index == 2)
		return (float)t/GetTicksPerFrame();
	else if (index == 3)
	{
		float normalizedTime = (float)(t-range.Start()) / (float)(range.End()-range.Start());
		if (TestAFlag(A_ORT_DISABLED))
			normalizedTime = clamp(normalizedTime, 0.0f, 1.0f);
		return normalizedTime;
	}

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARCONTS_VAR_NOT_EXIST), which);
		return 0.0f;
	}

	if (sVars[index].refID != -1) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARCONTS_VAR_NOT_CONST), which);
		return 0.0f;
	}

	return	(sVars[index].val);
}

Value* ExprControl::GetScalarTarget(Value* which, BOOL asController)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}

	if (index >= 0 && index <= 3) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARTARG_VAR_IS_CONST), which);
		return &undefined;
	}

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARTARG_VAR_NOT_EXIST), which);
		return &undefined;
	}

	if (sVars[index].refID == -1) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARTARG_VAR_IS_CONST), which);
		return &undefined;
	}

	ReferenceTarget *client = refTab[sVars[index].refID].client;
	if (client == NULL)
		return &undefined;

	int subNum = sVars[index].subNum;
	if (asController)
		return MAXControl::intern(((Control *)client->SubAnim(subNum)));
	else
		return MAXSubAnim::intern(client, subNum);
}

Point3 ExprControl::GetVectorConstant(Value* which)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECCONST_VAR_NOT_EXIST), which);
		return Point3(0.0f, 0.0f, 0.0f);
	}
	if (vVars[index].refID != -1) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECCONST_VAR_NOT_CONST), which);
		return Point3(0.0f, 0.0f, 0.0f);
	}

	return	(vVars[index].val);
}

Value* ExprControl::GetVectorTarget(Value* which, BOOL asController)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECTARG_VAR_NOT_EXIST), which);
		return &undefined;
	}

	if ((vVars[index].refID == -1) || (vVars[index].subNum == -1)) { // Node!
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECTARG_VAR_IS_CONST), which);
		return &undefined;
	}

	ReferenceTarget *client = refTab[vVars[index].refID].client;
	if (client == NULL)
		return &undefined;

	int	id = vVars[index].refID;
	int subNum = vVars[index].subNum;
	if (asController)
		return MAXControl::intern(((Control *)client->SubAnim(subNum)));
	else
		return MAXSubAnim::intern(client, subNum);
}

INode* ExprControl::GetVectorNode(Value* which)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECNODE_VAR_NOT_EXIST), which);
		return NULL;
	}
	if ((vVars[index].refID == -1) || (vVars[index].subNum != -1)) {	// Node!
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECNODE_VAR_IS_CONST), which);
		return NULL;
	}

	ReferenceTarget *client = refTab[vVars[index].refID].client;
	if (client == NULL)
		return NULL;
	INodeTransformMonitor *ntm = (INodeTransformMonitor*)client->GetInterface(IID_NODETRANSFORMMONITOR);
	if (ntm == NULL)
		return NULL;
	return ntm->GetNode();
}

float ExprControl::GetScalarValue(Value* which, TimeValue t)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}

	int i = index-4;
	if ((i < 0) || (i >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALAR_VAR_NOT_EXIST), which);
		return 0.0f;
	}

	float fval = 0.0f;
	if (index == 0)
		fval = (float)t;
	else if (index == 1)
		fval = (float)t/4800.0f;
	else if (index == 2)
		fval = (float)t/GetTicksPerFrame();
	else if (index == 3)
	{
		fval = (float)(t-range.Start()) / (float)(range.End()-range.Start());
		if (TestAFlag(A_ORT_DISABLED))
			fval = clamp(fval, 0.0f, 1.0f);
	}
	else
		fval = getScalarValue(index-4,t);
	return fval;
}

Point3 ExprControl::GetVectorValue(Value* which, TimeValue t)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVEC_VAR_NOT_EXIST), which);
		return Point3(0.0f, 0.0f, 0.0f);
	}

	return getVectorValue(index,t);
}

FPValue ExprControl::GetVarValue(TSTR &name, TimeValue t)
{
	int index = findIndex(SCALAR_VAR, name);
	if (index != -1) 
	{
		float fval = 0.0f;
		if (index == 0)
			fval = (float)t;
		else if (index == 1)
			fval = (float)t/4800.0f;
		else if (index == 2)
			fval = (float)t/GetTicksPerFrame();
		else if (index == 3)
		{
			fval = (float)(t-range.Start()) / (float)(range.End()-range.Start());
			if (TestAFlag(A_ORT_DISABLED))
				fval = clamp(fval, 0.0f, 1.0f);
		}
		else
			fval = getScalarValue(index-4,t);
		return FPValue (TYPE_FLOAT, fval);
	}
	else {
		index = findIndex(VECTOR_VAR, name);
		if (index != -1) 
			return FPValue (TYPE_POINT3_BV, getVectorValue(index,t));
	}

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_GETVAL_VAR_NOT_EXIST), name);
	return FPValue (TYPE_VALUE, &undefined); // undefined
}

int ExprControl::GetScalarType(Value* which)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(SCALAR_VAR, name);
	}

	if (index >= 0 && index <= 3) {
		return VAR_SCALAR_CONSTANT;
	}

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARTYPE_VAR_NOT_EXIST), which);
		return VAR_UNKNOWN;
	}

	if (sVars[index].refID == -1)
		return VAR_SCALAR_CONSTANT;

	return VAR_SCALAR_TARGET;
}

int	ExprControl::GetVectorType(Value* which)
{
	int index;
	if (is_number(which))
		index = which->to_int()-1;
	else
	{
		TSTR name = which->to_string();
		index = findIndex(VECTOR_VAR, name);
	}

	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECTYPE_VAR_NOT_EXIST), which);
		return VAR_UNKNOWN;
	}

	if (vVars[index].refID == -1)
		return VAR_VECTOR_CONSTANT;

	if (vVars[index].subNum == -1)
		return VAR_VECTOR_NODE;

	return VAR_VECTOR_TARGET;
}

int	ExprControl::GetValueType(TSTR &name)
{
	int i = findIndex(SCALAR_VAR, name);
	if (i != -1) {
		if ( i >= 4) {
			if (sVars[i-4].refID == -1)
				return VAR_SCALAR_CONSTANT;

			return VAR_SCALAR_TARGET;
		}
		return VAR_SCALAR_CONSTANT;
	}
	else {
		i = findIndex(VECTOR_VAR, name);
		if (i != -1) {
			if (vVars[i].refID == -1)
				return VAR_VECTOR_CONSTANT;

			if (vVars[i].subNum == -1)
				return VAR_VECTOR_NODE;

			return VAR_VECTOR_TARGET;
		}
	}

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_GETVALTYPE_VAR_NOT_EXIST), name);
	return VAR_UNKNOWN;
}

TSTR ExprControl::GetVectorName(int index)
{
	if ((index < 0) || (index >= vVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_VECTOR_INDEX), Integer::intern(index+1));
		return _T("");
	}
	return vVars[index].name;
}

TSTR ExprControl::GetScalarName(int index)
{
	if (index == 0) // 
		return _T("T");
	else if (index == 1)
		return _T("S");
	else if (index == 2)
		return _T("F");
	else if (index == 3)
		return _T("NT");

	index -= 4;
	if ((index < 0) || (index >= sVars.Count())) {
		if  (bThrowOnError)
			throw RuntimeError(GetString(IDS_LAM_EXPR_INVALID_SCALAR_INDEX), Integer::intern(index+1));
		return _T("");
	}
	return sVars[index].name;
}

int ExprControl::GetScalarIndex(TSTR &name)
{
	int i = findIndex(SCALAR_VAR, name);
	if (i >= 0)
		return i;

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_GETSCALARINDEX_VAR_NOT_EXIST), name);
	return -1;
}

int ExprControl::GetVectorIndex(TSTR &name)
{
	int i = findIndex(VECTOR_VAR, name);
	if (i != -1)
		return i;

	if  (bThrowOnError)
		throw RuntimeError(GetString(IDS_LAM_EXPR_GETVECINDEX_VAR_NOT_EXIST), name);
	return -1;
}

class ControlEnum : public AnimEnum {
public:	
	ControlEnum(Control *c) : AnimEnum(SCOPE_ALL)
	{
		ctrl = c;
		this_subNum = -1;
		this_client = NULL;
	}
	int proc(Animatable *anim, Animatable *client, int subNum)
	{
		if (anim == ctrl) {
			this_client = client;
			this_subNum = subNum;
			return ANIM_ENUM_ABORT;
		}
		return ANIM_ENUM_PROCEED;
	}

public:
	Control		*ctrl;
	Animatable	*this_client;
	int			this_subNum;
};

BOOL ExprControl::FindControl(Control* cntrl, ReferenceTarget *owner)
{
	if (owner == NULL) owner = GetCOREInterface()->GetScenePointer();
	ControlEnum ctrlEnum(cntrl);
	owner->EnumAnimTree(&ctrlEnum,NULL,0);
	this_client = ctrlEnum.this_client;
	this_index = ctrlEnum.this_subNum;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


static INT_PTR CALLBACK ExprDbgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ExprDebug::ExprDebug(HWND hParent, ExprControl *exprControl)
{
	ec = exprControl;
	CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_EXPR_DEBUG),
		hParent,
		ExprDbgWndProc,
		(LPARAM)this);	
}

ExprDebug::~ExprDebug()
{
	Rect rect;
	GetWindowRect(hWnd,&rect);
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	if(ec)
		ec->edbg = NULL;
}

void ExprDebug::Invalidate()
{
	InvalidateRect(hWnd,NULL,FALSE);
}

void ExprDebug::Update()
{
	if (inUpdate)
		DestroyWindow(hWnd);
	inUpdate = true;
	int i, ct;
	TCHAR buf[1024];
	Interval iv;
	HWND hList = GetDlgItem(hWnd, IDC_DEBUG_LIST);
	ExprControl* exprCtl = ec; 

	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	_stprintf(buf, _T("%s\t%g"), _T("T"), (float)t);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	_stprintf(buf, _T("%s\t%g"), _T("S"), (float)t/4800.0f);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	_stprintf(buf, _T("%s\t%g"), _T("F"), (float)t/GetTicksPerFrame());
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	float normalizedTime = (float)(t-ec->range.Start()) / (float)(ec->range.End()-ec->range.Start());
	if (ec->TestAFlag(A_ORT_DISABLED))
		normalizedTime = clamp(normalizedTime, 0.0f, 1.0f);
	_stprintf(buf, _T("%s\t%g"), _T("NT"), normalizedTime);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
 
	ct = ec->getVarCount(SCALAR_VAR);
	for(i = 0; i < ct; i++) {
		float fval = ec->getScalarValue(i, t);
		if (exprCtl->edbg == NULL) return;
		_stprintf(buf, _T("%s\t%.4f"), ec->getVarName(SCALAR_VAR, i), fval);
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	}
	ct = ec->getVarCount(VECTOR_VAR);
	for(i = 0; i < ct; i++) {
		Point3 vval = ec->getVectorValue(i, t);
		if (exprCtl->edbg == NULL) return;
		_stprintf(buf, _T("%s\t[%.4f, %.4f, %.4f]"), ec->getVarName(VECTOR_VAR, i), vval.x, vval.y, vval.z);
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
	}
	if(ec->expr.getExprType() == SCALAR_EXPR) {
		Interval iv;
		ec->GetValue(t, &ec->curFloatVal, iv);
		if (exprCtl->edbg == NULL) return;
		_stprintf(buf, _T("%g"), ec->curFloatVal);
	}
	else if(ec->type == EXPR_SCALE_CONTROL_CLASS_ID) {
		Interval iv;
		ScaleValue scaleVal;
		if (exprCtl->edbg == NULL) return;
		ec->GetValue(t, &scaleVal, iv);
		_stprintf(buf, _T("[%g, %g, %g]"), scaleVal.s.x, scaleVal.s.y, scaleVal.s.z);
	}
	else {
		Interval iv;
		ec->GetValue(t, &ec->curPosVal, iv);
		if (exprCtl->edbg == NULL) return;
		_stprintf(buf, _T("[%g, %g, %g]"), ec->curPosVal.x, ec->curPosVal.y, ec->curPosVal.z);
	}
	SetDlgItemText(hWnd, IDC_DEBUG_VALUE, buf);
	inUpdate = false;
}

static int tabs[] = { 75 };

void ExprDebug::Init(HWND hWnd)
{
	this->hWnd = hWnd;
	inUpdate = false;
	
		CenterWindow(hWnd,GetParent(hWnd));
	t = GetCOREInterface()->GetTime();

	SendMessage(GetDlgItem(hWnd, IDC_DEBUG_LIST), LB_SETTABSTOPS, 1, (LPARAM)tabs);
	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

typedef struct
{
	ExprDebug *ed;
	DialogResizer *resizer;
} ExprDebugDialogData;

static UINT dbg_msg_count = 0; // for debugging. msg counter

static INT_PTR CALLBACK ExprDbgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	if (msg == WM_COMMAND || msg == WM_ACTIVATE || msg == WM_NCACTIVATE || msg == WM_CAPTURECHANGED || msg == WM_SYSCOMMAND)
//		DebugPrint("ExprDbgWndProc (%X)- msg:%X; hWnd:%X; wParam:%X; lParam:%X\n", dbg_msg_count++, msg, hWnd, (int)wParam, (int)lParam);

	ExprDebugDialogData *eddata = DLGetWindowLongPtr<ExprDebugDialogData*>(hWnd);
	ExprDebug *ed = NULL;
	DialogResizer *resizer = NULL;
	if (eddata)
	{
		ed = eddata->ed;
		resizer = eddata->resizer;
	}

	switch (msg) {
	case WM_INITDIALOG:
//		SetWindowContextHelpId(hWnd,idh_dialog_xform_typein);

		ed = (ExprDebug*)lParam;
		resizer = new DialogResizer;
		eddata = new ExprDebugDialogData;
		eddata->ed = ed;
		eddata->resizer = resizer;
		DLSetWindowLongPtr(hWnd, eddata);

		SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLongPtr(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));
		ed->Init(hWnd);

		resizer->Initialize(hWnd); // use the current resource size as the minimum
		resizer->SetControlInfo(IDC_DEBUG_LIST, DialogResizer::kLockToTopLeft, DialogResizer::kWidthChangesWithDialog | DialogResizer::kHeightChangesWithDialog);
		resizer->SetControlInfo(IDC_DEBUG_VAL_LABEL, DialogResizer::kLockToBottomLeft);
		resizer->SetControlInfo(IDC_DEBUG_VALUE, DialogResizer::kLockToBottomLeft, DialogResizer::kWidthChangesWithDialog);
		resizer->SetControlInfo(IDCANCEL, DialogResizer::kLockToBottomRight);
		resizer->SetControlInfo(IDC_RESIZE,	DialogResizer::kLockToBottomRight);

		DialogResizer::LoadDlgPosition(hWnd, GetString(IDS_LAM_EXPR_DBG_CNAME));

		return FALSE;

	case WM_SYSCOMMAND:
//		if ((wParam & 0xfff0)==SC_CONTEXTHELP) 
//			DoHelp(HELP_CONTEXT,idh_dialog_xform_typein);				
		return 0;

	case WM_PAINT:
//		if (!ed->valid)
			ed->Update();
		return 0;
		
	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
			goto weAreDone;
		SendMessage(GetParent(hWnd), msg, wParam, lParam);						
		break;
	
	case WM_CLOSE:
weAreDone:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		DialogResizer::SaveDlgPosition(hWnd, GetString(IDS_LAM_EXPR_DBG_CNAME));
		delete eddata;
		delete ed;
		delete resizer;
		break;

	case WM_SIZING:
		resizer->Process_WM_SIZING(wParam, lParam);
		return TRUE;

	case WM_SIZE:
		resizer->Process_WM_SIZE(wParam, lParam);
		return TRUE;

	case WM_NCACTIVATE:
		if(wParam)
			PostMessage(hWnd,WM_DISABLE_ACCEL, 0, 0);
		else
			EnableAccelerators();
		return FALSE;

	case WM_CAPTURECHANGED:
	case WM_DISABLE_ACCEL:
		DisableAccelerators();
		return FALSE;


	default:
		return 0;
	}
	return 1;
}

