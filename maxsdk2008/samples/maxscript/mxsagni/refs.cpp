/**********************************************************************
*<
FILE: refs.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Strings.h"
#include "Arrays.h"
#include "maxclses.h"
#include "mscustattrib.h"
#include "MXSAgni.h"

#include "ICustAttribContainer.h"
#include "modstack.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

#include "definsfn.h"
#	include "refs_wraps.h"

#include "agnidefs.h"
// -----------------------------------------------------------------------------------------

// 'refs' primitives

Value*
refs_dependsOn_cf(Value** arg_list, int count)
{
	// refs.dependsOn <max_obj>
	check_arg_count(dependsOn, 1, count);
	if (arg_list[0]->is_kind_of(class_tag(MAXWrapper)))
	{
		MAXWrapper* mw = (MAXWrapper*)arg_list[0];
		ReferenceTarget* obj = (mw->NumRefs() > 0) ? mw->GetReference(0) : NULL;
		one_typed_value_local(Array* result);
		vl.result = new Array (0);
		if (obj != NULL)
		{
			for (int i=0; i < obj->NumRefs(); i++)
			{
				ReferenceTarget *ref = obj->GetReference(i);
				if (ref == NULL) continue;
				Value* item = NULL;
				try
				{
					item = MAXClass::make_wrapper_for(ref);
				}
				catch (...)
				{
					item = &undefined;
				}

				if (item != &undefined)
					vl.result->append(item);
			}
			ICustAttribContainer* cc = obj->GetCustAttribContainer();
			if (cc != NULL)
			{
				for (int i = 0; i < cc->GetNumCustAttribs(); i++)
				{
					CustAttrib* c = cc->GetCustAttrib(i);			
					if (c == NULL) continue;
					Value* item = NULL;
					try
					{
						item = MAXClass::make_wrapper_for(c);
					}
					catch (...)
					{
						item = &undefined;
					}

					if (item != &undefined)
						vl.result->append(item);
				}
			}
		}
		return_value(vl.result);
	}
	else
		return &undefined;
}

// reference enumerator class for refs.dependents() ...
// LAM - 11/03/02 - beefed up to handle cust attributes and rootnode
class GetDependentsProc : public DependentEnumProc
{
public:
	ReferenceTarget* obj;
	Array* result;
	Value** item;
	BOOL immediateOnly;
	GetDependentsProc(ReferenceTarget* obj, Array* result, Value** item, BOOL immediateOnly) 
	{ 
		this->obj = obj;
		this->result = result; 
		this->item = item; 
		this->immediateOnly = immediateOnly; 
	}
	int proc(ReferenceMaker *rmaker)
	{
		if (rmaker == (ReferenceMaker*)obj)
			return DEP_ENUM_CONTINUE;

		Class_ID cid = rmaker->ClassID();
		SClass_ID sid = rmaker->SuperClassID();
		if (sid == REF_MAKER_CLASS_ID && cid == CUSTATTRIB_CONTAINER_CLASS_ID)
		{
			Animatable *amaker = ((ICustAttribContainer*)rmaker)->GetOwner();
			if (amaker == NULL)
				return DEP_ENUM_CONTINUE;
			rmaker = (ReferenceMaker*)amaker->GetInterface(REFERENCE_MAKER_INTERFACE);
			if (rmaker == NULL)
				return DEP_ENUM_CONTINUE;
			cid = rmaker->ClassID();
			sid = rmaker->SuperClassID();
		}

		if (sid == MAKEREF_REST_CLASS_ID || sid == DELREF_REST_CLASS_ID || sid == MAXSCRIPT_WRAPPER_CLASS_ID)
			return DEP_ENUM_SKIP;

		MAXClass* cls = MAXClass::lookup_class(&cid, sid);
		MSCustAttrib* msCA;
		if (cls != NULL)
		{
			if (cls == &inode_object || cls == &rootnode_object)
				*item = MAXNode::intern((INode*)rmaker);
			else if (cls->superclass->superclass == &node_class)
				*item = MAXObject::intern((Object*)rmaker);
			else if ((msCA = (MSCustAttrib*)rmaker->GetInterface(I_SCRIPTEDCUSTATTRIB)) != NULL) // handle scripted CAs 
				*item = MAXCustAttrib::intern(msCA);
			else
				*item = (*cls->superclass->maker)(cls, (ReferenceTarget*)rmaker, NULL, 0);
			if (*item != &undefined)
				result->append(*item);			
		}
		return (immediateOnly) ? DEP_ENUM_HALT : DEP_ENUM_CONTINUE;
	}
};

class GetDependentNodesProc : public DependentEnumProc
{
public:
	ReferenceTarget* obj;
	Array* result;
	Value** item;
	BOOL firstOnly;
	BOOL baseObjOnly;
	bool isBaseObject;

	GetDependentNodesProc(ReferenceTarget* obj, Array* result, Value** item, BOOL firstOnly, BOOL baseObjOnly) 
	{ 
		this->obj = obj;
		this->result = result; 
		this->item = item; 
		this->firstOnly = firstOnly; 
		this->baseObjOnly = baseObjOnly; 
		isBaseObject = (GetObjectInterface(obj) != NULL);
	}
	int proc(ReferenceMaker *rmaker)
	{
		if (!isBaseObject && baseObjOnly)
			return DEP_ENUM_HALT;

		if (rmaker == (ReferenceMaker*)obj)
			return DEP_ENUM_CONTINUE;

		Class_ID cid = rmaker->ClassID();
		SClass_ID sid = rmaker->SuperClassID();
		if (sid == REF_MAKER_CLASS_ID && cid == CUSTATTRIB_CONTAINER_CLASS_ID)
		{
			Animatable *amaker = ((ICustAttribContainer*)rmaker)->GetOwner();
			if (amaker == NULL)
				return DEP_ENUM_CONTINUE;
			rmaker = (ReferenceMaker*)amaker->GetInterface(REFERENCE_MAKER_INTERFACE);
			if (rmaker == NULL)
				return DEP_ENUM_CONTINUE;
			cid = rmaker->ClassID();
			sid = rmaker->SuperClassID();
		}

		if (sid == MAKEREF_REST_CLASS_ID || sid == DELREF_REST_CLASS_ID || sid == MAXSCRIPT_WRAPPER_CLASS_ID)
			return DEP_ENUM_SKIP;

		MAXClass* cls = MAXClass::lookup_class(&cid, sid);
		if (cls == &inode_object)
		{
			Value *oldItem = *item;
			INode *node = (INode*)rmaker;
			*item = MAXNode::intern(node);
			if (firstOnly)
			{
				if (!isBaseObject && !baseObjOnly)
					return DEP_ENUM_HALT;
				Object* base_obj = node->GetObjectRef();
				if (base_obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
					base_obj = ((IDerivedObject*)base_obj)->FindBaseObject();
				if (base_obj == obj)
					return DEP_ENUM_HALT;
				else if (oldItem)
					*item = oldItem;
			}
			else
			{
				if (baseObjOnly)
				{
					Object* base_obj = node->GetObjectRef();
					if (base_obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
						base_obj = ((IDerivedObject*)base_obj)->FindBaseObject();
					if (base_obj == obj)
						result->append(*item);
				}
				else
					result->append(*item);
				*item = &undefined;
			}
			return DEP_ENUM_SKIP;
		}
		return DEP_ENUM_CONTINUE;
	}
};

Value*
refs_dependents_cf(Value** arg_list, int count)
{
	// refs.dependents <max_obj>
	check_arg_count_with_keys(dependsOn, 1, count);
	if (arg_list[0]->is_kind_of(class_tag(MAXWrapper)))
	{
		ReferenceTarget* obj = ((MAXWrapper*)arg_list[0])->GetReference(0);
		two_typed_value_locals(Array* result, Value* item);
		Value *tmp;
		BOOL immediateOnly = bool_key_arg(immediateOnly,tmp,FALSE);
		vl.result = new Array (0);
		if (obj != NULL)
		{
			GetDependentsProc gdp (obj, vl.result, &vl.item, immediateOnly);
			obj->DoEnumDependents(&gdp);
		}
		return_value(vl.result);
	}
	else
		return &undefined;
}

Value*
refs_dependentNodes_cf(Value** arg_list, int count)
{
	// refs.dependentNodes <max_obj> firstOnly:<bool> baseObjectOnly:<bool>
	check_arg_count_with_keys(dependentNodes, 1, count);
	if (arg_list[0]->is_kind_of(class_tag(MAXWrapper)))
	{
		ReferenceTarget* obj = ((MAXWrapper*)arg_list[0])->GetReference(0);
		two_typed_value_locals(Array* result, Value* item);
		Value *tmp;
		BOOL firstOnly = bool_key_arg(firstOnly,tmp,FALSE);
		BOOL baseObjectOnly = bool_key_arg(baseObjectOnly,tmp,FALSE);
		vl.item = &undefined;
		if (!firstOnly)
			vl.result = new Array (0);
		if (obj != NULL)
		{
			GetDependentNodesProc gdnp (obj, vl.result, &vl.item, firstOnly, baseObjectOnly);
			obj->DoEnumDependents(&gdnp);
			if (firstOnly)
				return_value(vl.item);
		}
		return_value(vl.result);
	}
	else
		return &undefined;
}

Value*
disableRefMsgs_cf(Value** arg_list, int count)
{
	check_arg_count(disableRefMsgs, 0, count);
	DisableRefMsgs();
	return &ok;
}

Value*
enableRefMsgs_cf(Value** arg_list, int count)
{
	check_arg_count(enableRefMsgs, 0, count);
	EnableRefMsgs();
	return &ok;
}

Value*
notifyDependents_cf(Value** arg_list, int count)
{
	// notifyDependents <MAXObject> partID:<name> msg:<int> propogate:<bool>
	check_arg_count_with_keys(notifyDependents, 1, count);
	ReferenceTarget *target = arg_list[0]->to_reftarg();
	def_refmsg_parts();
	Value* partIDVal = key_arg(partID); 
	int partID;
	if (is_int(partIDVal))
		partID = partIDVal->to_int();
	else
		partID = GetID(refmsgParts, elements(refmsgParts), partIDVal, PART_ALL);
	Value* msgVal = key_arg(msg); 
	int msg;
	if (is_int(msgVal))
		msg = msgVal->to_int();
	else
		msg = (partID == PART_PUT_IN_FG) ? REFMSG_FLAGDEPENDENTS : REFMSG_CHANGE;
	Value *tmp;
	BOOL propogate = bool_key_arg(propogate,tmp,TRUE);

	target->NotifyDependents(FOREVER,partID,msg,NOTIFY_ALL,propogate);
	needs_redraw_set();
	return &ok;
}

