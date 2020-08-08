/**********************************************************************
 *<
	FILE:			MXSCurveCtl.h

	DESCRIPTION:	MaxScript Curve Control extension DLX

	CREATED BY:		Ravi Karra

	HISTORY:		Created on 5/12/99

 *>	Copyright © 1997, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "resource.h"
#include "icurvctl.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXObj.h"
#include "Funcs.h"
#include <maxscript\macros\local_class.h>
#include <maxscript\macros\generic_class.h>

#pragma push_macro("ScripterExport")

#undef ScripterExport
#define ScripterExport


extern	TCHAR* GetString(int id);
extern	HINSTANCE hInstance;
#define elements(_array) (sizeof(_array)/sizeof((_array)[0]))
typedef struct { int id; Value* val; } paramType;
int		GetID(paramType params[], int count, Value* val, int def_val=-1);
Value*	GetName(paramType params[], int count, int id, Value* def_val=NULL);

DECLARE_LOCAL_GENERIC_CLASS(CCRootClassValue, CurveCtlGeneric);

class MAXCurve;
class MAXCurveCtl;

/* -------------------- CCRootClassValue  ------------------- */
// base class for all classes declared in MXSCurveCtl
local_visible_class (CCRootClassValue)
class CCRootClassValue : public MAXWrapper
{
public:
						CCRootClassValue() { }

	ValueMetaClass*		local_base_class() { return class_tag(CCRootClassValue); }
						classof_methods (CCRootClassValue, Value);
	void				collect() { delete this; }
	void				sprin1(CharStream* s) { s->puts(_T("CCRootClassValue\n")); }
	void				gc_trace() { Value::gc_trace(); }
#	define				is_ccroot(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(CCRootClassValue))

#include <maxscript\macros\local_abstract_generic_functions.h>
#	include "curvepro.h"
};

/* -------------------- CurvePointValue  ------------------- */
local_applyable_class (CurvePointValue)
class CurvePointValue : public Value
{
public:
	CurvePoint	cp;
	ICurve		*curve;
	ICurveCtl*	cctl;
	int			pt_index;

				CurvePointValue(CurvePoint pt);
				CurvePointValue(ICurveCtl*	cctl, ICurve *crv, int ipt);
	static Value* intern(ICurveCtl*	cctl, ICurve *crv, int ipt);
	void		sprin1(CharStream* s);
				classof_methods(CurvePointValue, Value);
	void		collect() { delete this; }
#	define		is_curvepoint(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(CurvePointValue))
	CurvePoint	to_curvepoint() { return cp; }

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	//internal
	void		SetFlag(DWORD mask, Value* val) { 
					if(val->to_bool()) cp.flags|=(mask); else cp.flags &= ~(mask); }
};

/* -------------------- CurvePointsArray  ------------------- */
local_visible_class (CurvePointsArray)
class CurvePointsArray : public MAXWrapper
{
public:
	ICurve		*curve;
	ICurveCtl*	cctl;
				CurvePointsArray(ICurveCtl*	cctl, ICurve* crv);
	static Value* intern(ICurveCtl*	cctl, ICurve *crv);
				classof_methods (CurvePointsArray, Value);
	TCHAR*		class_name() { return _T("CurvePointsArray"); }
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s);	
	
	/* operations */
#include <maxscript\macros\local_implementations.h>
#	include <maxscript\protocols\arrays.h>
	Value*		map(node_map& m);

	/* built-in property accessors */

	def_property ( count );
};

/* -------------------- MAXCurve ------------------- */
local_visible_class (MAXCurve)
class MAXCurve : public CCRootClassValue // Should derive from MAXWrapper
{
public:
	ICurve*		curve;
	ICurveCtl*	cctl;

	// curve Properties
	COLORREF	color, dcolor;
	int			width, dwidth, style, dstyle;

				MAXCurve(ICurve* icurve, ICurveCtl* icctl=NULL);
	static Value* intern(ICurve* icurve, ICurveCtl* icctl=NULL);

	ValueMetaClass* local_base_class() { return class_tag(CCRootClassValue); } // local base class in this class's plug-in
				classof_methods(MAXCurve, MAXWrapper);
	TCHAR*		class_name() { return _T("MAXCurve"); }
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	void		gc_trace();
#	define		is_curve(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXCurve))

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		Update();

	// operations 
#include <maxscript\macros\local_implementations.h>
#	include "curvepro.h"
};

/* -------------------- MAXCurveCtl  ------------------- */
class MSResourceMakerCallback : public ResourceMakerCallback, public ReferenceMaker
{
	MAXCurveCtl *mcc;
	public:
				MSResourceMakerCallback(MAXCurveCtl *cc) { mcc = cc; }
		void	ResetCallback(int curvenum, ICurveCtl *pCCtl);
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
					PartID& partID, RefMessage message ) { return REF_SUCCEED; }
		void*	GetInterface(ULONG id) {		
					return (id==I_RESMAKER_INTERFACE) ? this : ReferenceMaker::GetInterface(id);
					}
};

local_visible_class (MAXCurveCtl)

class MAXCurveCtl : public RolloutControl
{
public:	
	ICurveCtl	*cctl;
	MSResourceMakerCallback *resource_cb;
	HWND		hFrame;
	DWORD		rcmFlags, uiFlags;
	bool		popup;
	float		xvalue;
	
				MAXCurveCtl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
				~MAXCurveCtl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new MAXCurveCtl (name, caption, keyparms, keyparm_count); }
	
	ValueMetaClass* local_base_class() { return class_tag(CCRootClassValue); } // local base class in this class's plug-in
	void		gc_trace();
				classof_methods (MAXCurveCtl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_T("MAXCurveCtl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR		get_control_class() { return _T(""); }
	void		compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	int			num_controls() { return 2; }

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
#include <maxscript\macros\local_implementations.h>
	use_local_generic(zoom,	"zoom");
};

#pragma pop_macro("ScripterExport")