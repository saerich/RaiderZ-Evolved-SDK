/*	Adds MAXScript access to controllers:
 *		Euler_XYZ
 *		Local_Euler_XYZ
 *		Attachment  -- hack, need to put in an interface
 *		Link
 *		List
 *  Larry Minton, 2000
 */

#include "avg_maxver.h"

#include "MAXScrpt.h"
#include "MAXclses.h"
#include "maxkeys.h"
#include "Numbers.h"
#include "3DMath.h"

#include "Numbers.h"
#include "Listener.h"

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "istdplug.h"

#include "MXSAgni.h"

#include "..\..\controllers\attach.h"  // needed for AKey

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
//	#include "lam_wraps.h"


//  =================================================
//  Link Controller
//  =================================================

Value*
getLinkCount_cf(Value** arg_list, int count)
{
	check_arg_count(getLinkCount, 1, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	return Integer::intern(((ILinkCtrl*)c)->GetNumTargets());
}

Value*
getLinkTime_cf(Value** arg_list, int count)
{
	check_arg_count(getLinkTime, 2, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	int i=arg_list[1]->to_int();
	range_check(i, 1, ((ILinkCtrl*)c)->GetNumTargets(), GetString(IDS_LINK_CONTROL_INDEX_OUT_OF_RANGE));
	return MSTime::intern(((ILinkCtrl*)c)->GetLinkTime(i-1));
}

Value*
setLinkTime_cf(Value** arg_list, int count)
{
	check_arg_count(setLinkTime, 3, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	int i=arg_list[1]->to_int();
	range_check(i, 1, ((ILinkCtrl*)c)->GetNumTargets(), GetString(IDS_LINK_CONTROL_INDEX_OUT_OF_RANGE));
	((ILinkCtrl*)c)->SetLinkTime(i-1,arg_list[2]->to_timevalue());
	needs_redraw_set();
	return &ok;
}

Value*
addLink_cf(Value** arg_list, int count)
{
	check_arg_count(addLink, 3, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	INode* node=get_valid_node(arg_list[1], addLink);
	((ILinkCtrl*)c)->AddNewLink(node,arg_list[2]->to_timevalue());
	needs_redraw_set();
	return &ok;
}

Value*
deleteLink_cf(Value** arg_list, int count)
{
	check_arg_count(deleteLink, 2, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	int i=arg_list[1]->to_int();
	range_check(i, 1, ((ILinkCtrl*)c)->GetNumTargets(), GetString(IDS_LINK_CONTROL_INDEX_OUT_OF_RANGE));
	((ILinkCtrl*)c)->DeleteTarget(i-1);
	needs_redraw_set();
	return &ok;
}

Value*
getLinkNode_cf(Value** arg_list, int count)
{
	check_arg_count(getLinkNode, 2, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	int i=arg_list[1]->to_int();
	range_check(i, 1, ((ILinkCtrl*)c)->GetNumTargets(), GetString(IDS_LINK_CONTROL_INDEX_OUT_OF_RANGE));
	return MAXNode ::intern((INode*)((RefTargetHandle)c)->GetReference(LINKCTRL_FIRSTPARENT_REF+i-1));
}

Value*
setLinkNode_cf(Value** arg_list, int count)
{
	check_arg_count(setLinkNode, 3, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->ClassID() != LINKCTRL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LINK_CONTROL), arg_list[0]);
	int i=arg_list[1]->to_int();
	range_check(i, 1, ((ILinkCtrl*)c)->GetNumTargets(), GetString(IDS_LINK_CONTROL_INDEX_OUT_OF_RANGE));
	INode* node=get_valid_node(arg_list[2], addLink);
	((ILinkCtrl*)c)->SetReference(LINKCTRL_FIRSTPARENT_REF+i-1, (RefTargetHandle)node);
// hack to get Motion panel to update:
	((ILinkCtrl*)c)->SetLinkTime(i-1,((ILinkCtrl*)c)->GetLinkTime(i-1));
	needs_redraw_set();
	return &ok;
}


//  =================================================
//  Attachment Controller
//  =================================================

#define ToTCBUI(a) (((a)+1.0f)*25.0f)
#define FromTCBUI(a) (((a)/25.0f)-1.0f)
#define ToEaseUI(a) ((a)*50.0f)
#define FromEaseUI(a) ((a)/50.0f)

#include "defimpfn.h"

// controller keyframe access classes

class MAXAKeyClass : public ValueMetaClass							\
{																	\
public:																\
		MAXAKeyClass(TCHAR* name) : ValueMetaClass (name) { } 		\
		void		collect() { delete this; }						\
};																	\

MAXAKeyClass MAXAKey_class (_T("MAXAKey"));

class MAXAKey : public Value
{
public:
	MAXControl*	controller;			// MAX-side controller						
	int			key_index;
	ENABLE_STACK_ALLOCATE(MAXAKey);
	MAXAKey (Control* icont, int ikey, ParamDimension* dim);
	MAXAKey	(Control* icont, int ikey);
	MAXAKey	(MAXControl* icont, int ikey);
	void		gc_trace();
	AKey*		setup_key_access();
	void		Invalidate();

//	ValueMetaClass* local_base_class() {return class_tag(MAXAKey);}  // Needed for ???
				classof_methods (MAXAKey, Value);  // Define classof, superclassof methods

	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	def_generic ( delete,		"delete");
	def_generic ( copy,			"copy");
	def_generic	( show_props,	"showProperties");
	def_generic ( get_props,	"getPropNames");

	def_property	(face);
	def_property	(coord);

	def_property	(time);
	def_property	(selected);
	def_property	(tension);
	def_property	(continuity);
	def_property	(bias);
	def_property	(easeTo);
	def_property	(easeFrom);
};

// -------------------- MAXAKey methods -------------------------------- 

#include "definsfn.h"

MAXAKey::MAXAKey(Control* icont, int ikey, ParamDimension* idim)
{
	tag = class_tag(MAXAKey);
	controller = NULL;
	one_value_local(_this);
	vl._this = this;
	controller = (MAXControl*)MAXControl::intern(icont,idim);
	key_index = ikey;
	pop_value_locals();
}

MAXAKey::MAXAKey(Control* icont, int ikey)
{
	tag = class_tag(MAXAKey);
	controller = NULL;
	one_value_local(_this);
	vl._this = this;
	controller = (MAXControl*)MAXControl::intern(icont,0);
	key_index = ikey;
	pop_value_locals();
}

MAXAKey::MAXAKey(MAXControl* icont, int ikey)
{
	tag = class_tag(MAXAKey);
	controller = icont;
	key_index = ikey;
}

void
MAXAKey::gc_trace()
{
	Value::gc_trace();
	if (controller && controller->is_not_marked())
		controller->gc_trace();
}

AKey*
MAXAKey::setup_key_access()
{
	deletion_check(controller);
	if (key_index >= controller->controller->NumKeys())
		throw RuntimeError (GetString(IDS_KEY_NO_LONGER_EXISTS_IN_CONTROLLER), controller);
	IAttachCtrl* ac = (IAttachCtrl*)controller->controller->GetInterface(I_ATTACHCTRL);
	if (ac)
		return ac->GetKey(key_index);
	else 
		throw RuntimeError (GetString(IDS_KEY_NO_LONGER_EXISTS_IN_CONTROLLER), controller);
}

void
MAXAKey::Invalidate()
{
	deletion_check(controller);
	IAttachCtrl* ac = (IAttachCtrl*)controller->controller->GetInterface(I_ATTACHCTRL);
	if (ac)
		return ac->Invalidate();
	needs_redraw_set();
}

// ------------ MAXAKey built-in properties  ------------- 

Value* 
MAXAKey::get_property(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	Value* prop = arg_list[0];
	if (prop == n_face) 
		return Integer::intern(k->face);
	else if (prop == n_coord) 
		return new Point2Value(k->u0, k->u1);
	else
		return Value::get_property(arg_list, count);
}

Value* 
MAXAKey::set_property(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	Value* prop = arg_list[1];
	Value* val = arg_list[0];
	BOOL processed = FALSE;
	if (prop == n_face) {
		k->face = val->to_int(); 
		processed = TRUE;
	}
	else if (prop == n_coord) {
		Point2 p = arg_list[0]->to_point2();
		k->u0= p.x;
		k->u1= p.y;
		processed = TRUE;
	}
	if (processed) {
		Invalidate();
		return val;	
	}
	else
		return Value::set_property(arg_list, count);
}

Value*
MAXAKey::get_tension(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	return Float::intern(ToTCBUI(k->tens));
}

Value*
MAXAKey::set_tension(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->tens = FromTCBUI(arg_list[0]->to_float());
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_bias(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	return Float::intern(ToTCBUI(k->bias));
}

Value*
MAXAKey::set_bias(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->bias = FromTCBUI(arg_list[0]->to_float());
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_continuity(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	return Float::intern(ToTCBUI(k->cont));
}

Value*
MAXAKey::set_continuity(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->cont = FromTCBUI(arg_list[0]->to_float());
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_easeTo(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	return Float::intern(ToEaseUI(k->easeIn));
}

Value*
MAXAKey::set_easeTo(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->easeIn = FromEaseUI(arg_list[0]->to_float());
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_easeFrom(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	return Float::intern(ToEaseUI(k->easeOut));
}

Value*
MAXAKey::set_easeFrom(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->easeOut = FromEaseUI(arg_list[0]->to_float());
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_time(Value** arg_list, int count)
{
	deletion_check(controller);
	return MSTime::intern(controller->controller->GetKeyTime(key_index));
}

Value*
MAXAKey::set_time(Value** arg_list, int count)
{
	AKey* k = setup_key_access();
	k->time = arg_list[0]->to_timevalue();
	Invalidate();
	return arg_list[0];
}

Value*
MAXAKey::get_selected(Value** arg_list, int count)
{
	deletion_check(controller);
	return controller->controller->IsKeySelected(key_index) ? &true_value : &false_value;
}

Value*
MAXAKey::set_selected(Value** arg_list, int count)
{
	deletion_check(controller);
	type_check(arg_list[0], Boolean, "<key>.selected");
	controller->controller->SelectKeyByIndex(key_index, arg_list[0] == &true_value);
	return arg_list[0];
}

Value*
MAXAKey::delete_vf(Value** arg_list, int count)
{
	//	delete <key>
	deletion_check(controller);
	check_arg_count(delete, 1, count + 1);
	if (key_index >= controller->controller->NumKeys())
		throw RuntimeError (GetString(IDS_KEY_NO_LONGER_EXISTS_IN_CONTROLLER), controller);

	controller->controller->DeleteKeyByIndex(key_index);
	needs_redraw_set();
	return &ok;
}


Value*
MAXAKey::copy_vf(Value** arg_list, int count)
{
	deletion_check(controller);
	return this;
}

void
MAXAKey::sprin1(CharStream* s)
{
	if (deletion_check_test(controller))
	{
		s->printf(_T("<Key for deleted controller>"));
		return;
	}

	TSTR cnm;
	controller->GetClassName(cnm);
	if (key_index >= controller->controller->NumKeys())
		s->printf(_T("#%s <deleted key>"), cnm);
	else
		s->printf(_T("#%s key(%d @ %gf)"), cnm, key_index + 1, controller->controller->GetKeyTime(key_index) / (float)GetTicksPerFrame());
}

Value*
MAXAKey::get_props_vf(Value** arg_list, int count)
{
	// getPropNames <max_object> [#dynamicOnly] -- returns array of prop names of wrapped obj
	one_typed_value_local(Array* result);
	vl.result = new Array (9);
	vl.result->append(n_face);
	vl.result->append(n_coord);
	vl.result->append(n_tension);
	vl.result->append(n_bias);
	vl.result->append(n_continuity);
	vl.result->append(n_easeTo);
	vl.result->append(n_easeFrom);
	vl.result->append(n_time);
	vl.result->append(n_selected);
	return_value(vl.result);
}

Value*
MAXAKey::show_props_vf(Value** arg_list, int count)
{
	// showProperties <max_object> [_T("prop_pat")] [to:<file>]   - applies to ref0 in the wrapper
	return &false_value;
}


//  =================================================
//  Attachment Controller
//  =================================================

Value* 
attach_get_node(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	INode* theNode = (ac) ? ac->GetObject() : NULL;
	if (theNode) return MAXNode::intern(theNode);
	return &undefined;
}

void
attach_set_node(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	INode* theNode=val->to_node();
	deletion_check((MAXNode*)val);
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	if (ac)
	{
		BOOL res=ac->SetObject(theNode);
		if (!res)
			throw RuntimeError(GetString(IDS_ATTACH_SET_NODE_FAILED), val);
		needs_redraw_set();
	}
}

Value* 
attach_get_align(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	if (ac)
		return ac->GetAlign() ? &true_value : &false_value;
	else
		return &undefined;
}

void
attach_set_align(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	if (ac)
	{
		ac->SetAlign(val->to_bool());
		needs_redraw_set();
	}
}

Value* 
attach_get_manupdate(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	if (ac)
		return ac->GetManualUpdate() ? &true_value : &false_value;
	else
		return &undefined;
}

void
attach_set_manupdate(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	IAttachCtrl* ac = (IAttachCtrl*)obj->GetInterface(I_ATTACHCTRL);
	if (ac)
	{
		ac->SetManualUpdate(val->to_bool());
		if (!val->to_bool()) needs_redraw_set();
	}
}


MAXClass attachment
	(_T("Attachment"), ATTACH_CONTROL_CLASS_ID, CTRL_POSITION_CLASS_ID, &position_controller, 0,
		accessors,
			fns,
				_T("node"),			attach_get_node,		attach_set_node,		TYPE_UNSPECIFIED,
				_T("align"),		attach_get_align,		attach_set_align,		TYPE_UNSPECIFIED,
				_T("manupdate"),	attach_get_manupdate,	attach_set_manupdate,	TYPE_UNSPECIFIED,
				end,
			end,
		end
	);

Value*
Attachment_GetKey_cf(Value** arg_list, int count)
{
	check_arg_count(getKey, 2, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->GetInterface(I_ATTACHCTRL) == NULL)
		throw RuntimeError (GetString(IDS_NOT_ATTACH_CONTROL), arg_list[0]);
	int	index = arg_list[1]->to_int();
	range_check(index, 1, c->NumKeys(), GetString(IDS_ATTACH_KEY_INDEX_OUT_OF_RANGE));
	return new MAXAKey(((MAXControl*)arg_list[0]), index - 1);  // array indexes are 1-based !!!
}

Value*
Attachment_AddNewKey_cf(Value** arg_list, int count)
{
	if (count < 2 || count > 3) check_arg_count(addNewKey, 2, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	if (c->GetInterface(I_ATTACHCTRL) == NULL)
		throw RuntimeError (GetString(IDS_NOT_ATTACH_CONTROL), arg_list[0]);
	TimeValue t = arg_list[1]->to_timevalue();
	DWORD flags = 0;
	if (count == 3)
	{	if (arg_list[2] == n_select)
			flags |= ADDKEY_SELECT;
		else 
			throw TypeError (GetString(IDS_UNRECOGNIZED_FLAG_FOR_ADDNEWKEY), arg_list[2]);
	}
	c->AddNewKey(t, flags);
	needs_redraw_set();
	return new MAXAKey(((MAXControl*)arg_list[0]), c->GetKeyIndex(t));
}

Value*
Attachment_Update_cf(Value** arg_list, int count)
{
	check_arg_count(update, 1, count);
	Control* c = arg_list[0]->to_controller();
	deletion_check(((MAXControl*)arg_list[0]));
	IAttachCtrl* ac = (IAttachCtrl*)c->GetInterface(I_ATTACHCTRL);
	if (ac)
	{
		c->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		ac->Invalidate(TRUE);
		needs_redraw_set();
	}
	return &ok;
}


//  =================================================
//  Euler_XYZ Controllers
//  =================================================

Value* 
euler_get_order(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	IEulerControl* ec = (IEulerControl*)obj->GetInterface(I_EULERCTRL);
	if (ec)
		return Integer::intern(ec->GetOrder()+1);
	else
		return &undefined;
}

void
euler_set_order(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	int order=val->to_int();
	range_check(order, 1, 9, GetString(IDS_AXIS_ORDER_INDEX_OUT_OF_RANGE));
	IEulerControl* ec = (IEulerControl*)obj->GetInterface(I_EULERCTRL);
	if (ec) ec->SetOrder(order-1);
	needs_redraw_set();
}

MAXClass euler_XYZ_rotation
	(_T("Euler_XYZ"), Class_ID(EULER_CONTROL_CLASS_ID, 0), CTRL_ROTATION_CLASS_ID, &rotation_controller, 0,
		accessors,
			fns,
				_T("axisOrder"),	euler_get_order,	euler_set_order,	TYPE_UNSPECIFIED,
				end,
			end,
		end
	);

MAXClass local_euler_XYZ_rotation
	(_T("Local_Euler_XYZ"), Class_ID(LOCAL_EULER_CONTROL_CLASS_ID, 0), CTRL_ROTATION_CLASS_ID, &rotation_controller, 0,
		accessors,
			fns,
				_T("axisOrder"),	euler_get_order,	euler_get_order,	TYPE_UNSPECIFIED,
				end,
			end,
		end
	);


//  =================================================
//  List Controllers
//  =================================================
#define FLOATLIST_CONTROL_CLASSID		Class_ID(FLOATLIST_CONTROL_CLASS_ID,0)
#define POINT3LIST_CONTROL_CLASSID		Class_ID(POINT3LIST_CONTROL_CLASS_ID,0)
#define POINT4LIST_CONTROL_CLASSID		Class_ID(POINT4LIST_CONTROL_CLASS_ID,0)
#define POSLIST_CONTROL_CLASSID			Class_ID(POSLIST_CONTROL_CLASS_ID,0)
#define ROTLIST_CONTROL_CLASSID			Class_ID(ROTLIST_CONTROL_CLASS_ID,0)
#define SCALELIST_CONTROL_CLASSID		Class_ID(SCALELIST_CONTROL_CLASS_ID,0)
#define DUMMY_CONTROL_CLASSID			Class_ID(DUMMY_CONTROL_CLASS_ID,0)

#include "definsfn.h"

Value*
ListController_GetName_cf(Value** arg_list, int count)
{
	check_arg_count(getName, 2, count);
	deletion_check(((MAXControl*)arg_list[0]));
	Control* c = arg_list[0]->to_controller();
	if (c->ClassID() != FLOATLIST_CONTROL_CLASSID && c->ClassID() != POINT3LIST_CONTROL_CLASSID &&
		c->ClassID() != POSLIST_CONTROL_CLASSID && c->ClassID() != ROTLIST_CONTROL_CLASSID &&
		c->ClassID() != SCALELIST_CONTROL_CLASSID && c->ClassID() != POINT4LIST_CONTROL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LIST_CONTROL), arg_list[0]);

	int index = arg_list[1]->to_int();
	range_check(index, 1, ((IListControl*)c)->GetListCount(), GetString(IDS_LIST_CONTROL_INDEX_OUT_OF_RANGE));
	TSTR name = ((IListControl*)c)->GetName(index-1);  // array indexes are 1-based !!!
	return new String(name);
}

Value*
ListController_SetName_cf(Value** arg_list, int count)
{
	check_arg_count(setName, 3, count);
	deletion_check(((MAXControl*)arg_list[0]));
	Control* c = arg_list[0]->to_controller();
	if (c->ClassID() != FLOATLIST_CONTROL_CLASSID && c->ClassID() != POINT3LIST_CONTROL_CLASSID &&
		c->ClassID() != POSLIST_CONTROL_CLASSID && c->ClassID() != ROTLIST_CONTROL_CLASSID &&
		c->ClassID() != SCALELIST_CONTROL_CLASSID && c->ClassID() != POINT4LIST_CONTROL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LIST_CONTROL), arg_list[0]);

	int index = arg_list[1]->to_int();
	IListControl* lc = (IListControl*)c;
	range_check(index, 1, lc->GetListCount(), GetString(IDS_LIST_CONTROL_INDEX_OUT_OF_RANGE));
	//if (!lc->names[index-1]) lc->names[index-1] = new TSTR;
	lc->SetName(index-1, arg_list[2]->to_string());
	//c->NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);
	//c->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return &ok;
}

Value*
ListController_GetActive_cf(Value** arg_list, int count)
{
	check_arg_count(getName, 1, count);
	deletion_check(((MAXControl*)arg_list[0]));
	Control* c = arg_list[0]->to_controller();
	if (c->ClassID() != FLOATLIST_CONTROL_CLASSID && c->ClassID() != POINT3LIST_CONTROL_CLASSID &&
		c->ClassID() != POSLIST_CONTROL_CLASSID && c->ClassID() != ROTLIST_CONTROL_CLASSID &&
		c->ClassID() != SCALELIST_CONTROL_CLASSID && c->ClassID() != POINT4LIST_CONTROL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LIST_CONTROL), arg_list[0]);

	return Integer::intern(((IListControl*)c)->GetActive()+1);  // array indexes are 1-based !!!
}

Value*
ListController_SetActive_cf(Value** arg_list, int count)
{
	check_arg_count(setName, 2, count);
	deletion_check(((MAXControl*)arg_list[0]));
	Control* c = arg_list[0]->to_controller();
	if (c->ClassID() != FLOATLIST_CONTROL_CLASSID && c->ClassID() != POINT3LIST_CONTROL_CLASSID &&
		c->ClassID() != POSLIST_CONTROL_CLASSID && c->ClassID() != ROTLIST_CONTROL_CLASSID &&
		c->ClassID() != SCALELIST_CONTROL_CLASSID && c->ClassID() != POINT4LIST_CONTROL_CLASSID)
		throw RuntimeError (GetString(IDS_NOT_LIST_CONTROL), arg_list[0]);

	int index = arg_list[1]->to_int();
	IListControl* lc = (IListControl*)c;
	range_check(index, 1, lc->GetListCount(), GetString(IDS_LIST_CONTROL_INDEX_OUT_OF_RANGE));
	lc->SetActive(index-1);
//	c->NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);
//	c->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return &ok;
}


