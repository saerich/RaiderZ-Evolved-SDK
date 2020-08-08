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
#include "IIndirectRefMaker.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "refs_wraps.h"

#include "agnidefs.h"
// -----------------------------------------------------------------------------------------

// 'refs' primitives

static Value* WrapObjectReference(ReferenceTarget *object, int index)
{
	ReferenceTarget *ref = object->GetReference(index);
	one_value_local(ref);
	try
	{
		// special case TVNodes. This is much faster than going through MAXClass::make_wrapper_for
		// since we already know the parent and index
		ITrackViewNode *tvnode = dynamic_cast<ITrackViewNode*>(ref);
		ITrackViewNode *parent = tvnode ? dynamic_cast<ITrackViewNode*>(object) : NULL;
		if (parent && index < parent->NumItems() && parent->GetNode(index) == tvnode)
			vl.ref = MAXTVNode::intern(parent,index);
		else
			vl.ref = MAXClass::make_wrapper_for(ref);
	}
	catch (...)
	{
		vl.ref = &undefined;
	}
	return_value(vl.ref);
}


Value*
refs_dependsOn_cf(Value** arg_list, int count)
{
	// refs.dependsOn <max_obj>
	check_arg_count(dependsOn, 1, count);
	if (arg_list[0]->is_kind_of(class_tag(MAXWrapper)))
	{
		MAXWrapper* mw = (MAXWrapper*)arg_list[0];
		ReferenceTarget* obj = (mw->NumRefs() > 0) ? mw->GetReference(0) : NULL;
		two_typed_value_locals(Array* result, Value* item;);
		vl.result = new Array (0);
		if (obj != NULL)
		{
			for (int i=0; i < obj->NumRefs(); i++)
			{
				vl.item = WrapObjectReference(obj, i);
				if (vl.item != &undefined)
					vl.result->append(vl.item);
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

		*item = MAXClass::make_wrapper_for((ReferenceTarget*)rmaker);
		if (*item != &undefined)
			result->append(*item);			

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
	check_arg_count_with_keys(dependents, 1, count);
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
refs_getnumrefs_cf(Value** arg_list, int count)
{
	// refs.getNumRefs <max_obj>
	check_arg_count(getNumRefs, 1, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	int numRefs = 0;
	if (refTarg != NULL)
		numRefs = refTarg->NumRefs();
	return_protected(Integer::intern(numRefs));
}

Value*
refs_getreference_cf(Value** arg_list, int count)
{
	// refs.getReference <max_obj> <int>
	check_arg_count(getReference, 2, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	if (refTarg != NULL)
	{
		int whichRef = arg_list[1]->to_int();
		range_check(whichRef, 1, refTarg->NumRefs(),_T("reference index"));
		return_protected(WrapObjectReference(refTarg, whichRef-1));
	}
	return &undefined;
}

Value*
refs_replacereference_cf(Value** arg_list, int count)
{
	// refs.replaceReference <max_obj> <int> {<max_obj> | undefined}
	check_arg_count(replaceReference, 3, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	if (refTarg != NULL)
	{
		int whichRef = arg_list[1]->to_int();
		range_check(whichRef, 1, refTarg->NumRefs(),_T("reference index"));
		ReferenceTarget* newRef = NULL;
		if (arg_list[2] != &undefined)
		{
			if( arg_list[2]->is_kind_of(class_tag(MAXNode)) )
				newRef = ((MAXNode*)arg_list[2])->node;
			else 
				newRef = arg_list[2]->to_reftarg();
		}
		RefResult res = refTarg->ReplaceReference(whichRef-1, newRef);
		if (res == REF_SUCCEED)
			return &true_value;
	}
	return &false_value;
}

Value*
refs_getnumIndirectrefs_cf(Value** arg_list, int count)
{
	// refs.getNumIndirectRefs <max_obj>
	check_arg_count(getNumIndirectRefs, 1, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	int numIndirectRefs = 0;
	if (refTarg != NULL)
	{
		IIndirectReferenceMaker* iirm = (IIndirectReferenceMaker*)refTarg->GetInterface(IID_IINDIRECTREFERENCEMAKER);
		if (iirm)
			numIndirectRefs = iirm->NumIndirectRefs();
	}
	return_protected(Integer::intern(numIndirectRefs));
}

Value*
refs_getIndirectreference_cf(Value** arg_list, int count)
{
	// refs.getIndirectReference <max_obj> <int>
	check_arg_count(getIndirectReference, 2, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	if (refTarg != NULL)
	{
		int whichRef = arg_list[1]->to_int();
		int numIndirectRefs = 0;
		IIndirectReferenceMaker* iirm = (IIndirectReferenceMaker*)refTarg->GetInterface(IID_IINDIRECTREFERENCEMAKER);
		if (iirm)
			numIndirectRefs = iirm->NumIndirectRefs();
		range_check(whichRef, 1, numIndirectRefs,_T("indirect reference index"));
		return_protected(MAXClass::make_wrapper_for(iirm->GetIndirectReference(whichRef-1)));
	}
	return &undefined;
}

Value*
refs_setIndirectreference_cf(Value** arg_list, int count)
{
	// refs.setIndirectReference <max_obj> <int> {<max_obj> | undefined}
	check_arg_count(setIndirectReference, 3, count);
	ReferenceTarget* refTarg = NULL;
	if( arg_list[0]->is_kind_of(class_tag(MAXNode)) )
		refTarg = ((MAXNode*)arg_list[0])->node;
	else 
		refTarg = arg_list[0]->to_reftarg();
	if (refTarg != NULL)
	{
		int whichRef = arg_list[1]->to_int();
		int numIndirectRefs = 0;
		IIndirectReferenceMaker* iirm = (IIndirectReferenceMaker*)refTarg->GetInterface(IID_IINDIRECTREFERENCEMAKER);
		if (iirm)
			numIndirectRefs = iirm->NumIndirectRefs();
		range_check(whichRef, 1, numIndirectRefs,_T("indirect reference index"));
		ReferenceTarget* newRef = NULL;
		if (arg_list[2] != &undefined)
		{
			if( arg_list[2]->is_kind_of(class_tag(MAXNode)) )
				newRef = ((MAXNode*)arg_list[2])->node;
			else 
				newRef = arg_list[2]->to_reftarg();
		}
		iirm->SetIndirectReference(whichRef-1, newRef);
	}
	return &ok;
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

