/**********************************************************************
 *<
	FILE: Common_Funcs.cpp

	DESCRIPTION: Utility functions for MXSAgni.dlx

	CREATED BY: Larry Minton, 3/30/000

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

//include mxsagni.h to use these functions and for interfacing macro definitions.

//methods/variables defined:
//	TCHAR* GetString(...)
//	int GetID(...)
//	Value* GetName(...)
//	void ValueToBitArray(...)
//	void FPValueToBitArray(...)
//	INode* _get_valid_node(...)
//	BezierShape* set_up_spline_access(...)
//	Mesh* _get_meshForValue(...)
//	Mesh* _get_meshForFPValue(...)
//	Mesh* getWorldStateMesh(...)
//	Mesh* _get_worldMeshForValue(...)
//	IMeshSelectData* get_IMeshSelData_for_mod(...)
//	MeshDeltaUser* get_MeshDeltaUser_for_mod(...)

#include "MAXScrpt.h"
#include "meshdelta.h"
#include "MAXObj.h"
#include "MeshSub.h"
#include "3DMath.h"

#include "mxsagni.h"
#include "modstack.h"
#include "SPLSHAPE.h"
#include "resource.h"

TCHAR*
GetString(int id)
{
	static TCHAR buf[256];

	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

int 
GetID(paramType params[], int count, Value* val, int def_val)
{
	for(int p = 0; p < count; p++)
		if(val == params[p].val)
			return params[p].id;
	if (def_val != -1 ) return def_val;

	// Throw an error showing valid args incase an invalid arg is passed
	TCHAR buf[2056]; buf[0] = '\0';
	
	_tcscat(buf, GetString(IDS_FUNC_NEED_ARG_OF_TYPE));	
	for (int i=0; i < count; i++) 
	{
		_tcscat(buf, params[i].val->to_string());
		if (i < count-1) _tcscat(buf, _T(" | #"));
	}	
	_tcscat(buf, GetString(IDS_FUNC_GOT_ARG_OF_TYPE));
	throw RuntimeError(buf, val); 
	return -1;
}

Value* 
GetName(paramType params[], int count, int id, Value* def_val)
{
	for(int p = 0; p < count; p++)
		if(id == params[p].id)
			return params[p].val;
	if (def_val) return def_val;
	DbgAssert(0);	// LAM - 12/6/01 - was returning NULL, which was giving nearly untraceable exceptions
	return &undefined;
}

#include "Numbers.h"
#include "Arrays.h"
#include "MAXObj.h"

void 
ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg, int selTypesAllowed) {
	if (inval->is_kind_of(class_tag(Array))) {
		Array* theArray = (Array*)inval;
		for (int k = 0; k < theArray->size; k++) {
			int index = theArray->data[k]->to_int();
			if (maxSize != -1) {
				range_check(index, 1, maxSize, errmsg);
			}
			else
				if (index > theBitArray.GetSize())
					theBitArray.SetSize(index, 1);
			theBitArray.Set(index-1);
		}
	}
	else if (inval->is_kind_of(class_tag(BitArrayValue))) {
		for (int k = 0; k < ((BitArrayValue*)inval)->bits.GetSize(); k++) {
			if ((((BitArrayValue*)inval)->bits)[k]) {
				if (maxSize != -1) {
					range_check(k+1, 1, maxSize, errmsg);
				}
				else
					if (k+1 > theBitArray.GetSize())
						theBitArray.SetSize(k+1, 1);
				theBitArray.Set(k);
			}
		}
	}
	else if (inval == n_all) {
		theBitArray.SetAll();
	}
	else if (inval == n_none) {
		theBitArray.ClearAll();
	}
	else if (is_number(inval)) {
		int index = inval->to_int();
		if (maxSize != -1) {
			range_check(index, 1, maxSize, errmsg);
		}
		else
			if (index > theBitArray.GetSize())
				theBitArray.SetSize(index, 1);
		theBitArray.Set(index-1);
	}
	else if ((selTypesAllowed == MESH_VERTSEL_ALLOWED && (is_vertselection(inval))) || 
	         (selTypesAllowed == MESH_FACESEL_ALLOWED && (is_faceselection(inval))) ||
	         (selTypesAllowed == MESH_EDGESEL_ALLOWED && (is_edgeselection(inval))))
			theBitArray = ((BitArrayValue*)((MeshSelection*)inval)->to_bitarrayValue())->bits;
	else 
		throw RuntimeError (GetString(IDS_CANNOT_CONVERT_TO_BITARRAY), inval);

}

void 
ValueToBitArrayM(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg, int selTypesAllowed, Mesh* mesh) {
	if (inval == n_selection) {
		switch (selTypesAllowed)
		{
			case MESH_VERTSEL_ALLOWED:
				theBitArray = mesh->vertSel;
				break;
			case MESH_FACESEL_ALLOWED:
				theBitArray = mesh->faceSel;
				break;
			case MESH_EDGESEL_ALLOWED:
				theBitArray = mesh->edgeSel;
		}

	}
	else
		ValueToBitArray(inval, theBitArray, maxSize, errmsg, selTypesAllowed);
}


INode*
_get_valid_node(MAXNode* _node, TCHAR* errmsg) {
	if (is_node(_node))
	{	
		if(deletion_check_test(_node))
			throw RuntimeError("Attempted to access to deleted object");
	}
	else
		throw RuntimeError (errmsg);
	return _node->to_node();
}

BezierShape*
set_up_spline_access(INode* node, int index)
{
	/* pick up base obj & make sure its a splineshape, then pass back splineshape pointer */
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());
	if (obj->ClassID() != splineShapeClassID && obj->ClassID() != Class_ID(SPLINE3D_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_SHAPE_OPERATION_ON_NONSPLINESHAPE), obj->GetObjectName());
	BezierShape* s = &((SplineShape*)obj)->shape;
	range_check(index, 1, s->SplineCount(), GetString(IDS_SHAPE_SPLINE_INDEX_OUT_OF_RANGE))
	return s;
}

// if value is a node, do normal mesh access for node.
// if value is an editable mesh (baseobject), get its mesh. Don't know the INode though
// if value is a MAXScript MeshValue, get its mesh
Mesh* 
_get_meshForValue(Value* value, int accessType, ReferenceTarget** owner, TCHAR* fn) {
	Mesh* mesh = NULL;
	if (owner) *owner = NULL;
	if (value->is_kind_of(class_tag(MAXNode))) {
		get_valid_node(value, fn);
		mesh = ((MAXNode*)value)->set_up_mesh_access(accessType, owner);
	}
	else if (value->tag == class_tag(MAXObject) &&
			 ((MAXObject*)value)->obj->ClassID() == EDITTRIOBJ_CLASSID) {
		mesh = &((TriObject*)((MAXObject*)value)->obj)->GetMesh();
		if (owner) *owner = ((MAXObject*)value)->obj;
	}
	else {
		mesh = ((MeshValue*)value)->to_mesh();
		if (owner) *owner = ((MeshValue*)value)->obj;
	}
/* 
// test block for seeing how to walk the reference tree
	if (owner && *owner) {
		RefList& theRefs = (*owner)->GetRefList();
		RefListItem* item;
		item = theRefs.FirstItem();
		while (item) {
			(item->maker)->SuperClassID();
			item=item->next;
		}

	}
*/
	return mesh;
}

class NullView: public View {
	public:
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};

// mesh returned is independent of the source, and is in world space coordinates
Mesh*
getWorldStateMesh(INode* node, BOOL renderMesh)
{
	const ObjectState& os = node->EvalWorldState(MAXScript_time());// Get the object that is referenced at the time
	Object* ob = os.obj;
	if (!ob || (ob->SuperClassID() != GEOMOBJECT_CLASS_ID && ob->SuperClassID() != SHAPE_CLASS_ID))
		throw RuntimeError (GetString(IDS_CANNOT_GET_MESH_FROM_OBJECT), node->GetName());
	Mesh* resMesh = NULL;

	if (renderMesh) {
		BOOL needDel;
		NullView nullView;	// Set up a null viewport so that we get the right mesh..
		Mesh *mesh = ((GeomObject*)ob)->GetRenderMesh(MAXScript_time(),node,nullView,needDel); // Type cast to a geometric object and then retriev
		if (!mesh) return NULL;
		resMesh = new Mesh (*mesh);
		if (needDel) delete mesh;
	}
	else {
		if (!ob->CanConvertToType(triObjectClassID)) return NULL;
		TriObject *tri = (TriObject *)ob->ConvertToType(MAXScript_time(), triObjectClassID);
		if (!tri) return NULL;
		resMesh = new Mesh (tri->GetMesh());
		if (ob != tri) tri->AutoDelete();
	}
	Matrix3 tm2 = node->GetObjTMAfterWSM(MAXScript_time());
	for (int i = 0; i < resMesh->getNumVerts(); i++)
		resMesh->verts[i] = resMesh->verts[i] * tm2;
/*
	if (node->GetObjectRef() == ob) {
		Matrix3 tm = node->GetObjectTM(MAXScript_time());
		for (int i = 0; i < resMesh->getNumVerts(); i++)
			resMesh->verts[i] = resMesh->verts[i] * tm;
	}

	if (os.GetTM()) {
		Matrix3 tm = *(os.GetTM());
		for (int i = 0; i < resMesh->getNumVerts(); i++)
			resMesh->verts[i] = resMesh->verts[i] * tm;
	}
*/
	return resMesh;
}


Mesh*
_get_worldMeshForValue(Value* value, TCHAR* fn, BOOL renderMesh) {
	Mesh* mesh = NULL;
	if (value->is_kind_of(class_tag(MAXNode))) {
		get_valid_node((MAXNode*)value, fn);
		mesh = getWorldStateMesh(((MAXNode*)value)->to_node(), renderMesh);
	}
	else if (value->tag == class_tag(MAXObject) &&
			 ((MAXObject*)value)->obj->ClassID() == EDITTRIOBJ_CLASSID) {
		mesh = &((TriObject*)((MAXObject*)value)->obj)->GetMesh();
	}
	else {
		mesh = ((MeshValue*)value)->to_mesh();
	}
	return mesh;
}

IMeshSelectData*
get_IMeshSelData_for_mod(Object* obj, Modifier*& target_mod, int mod_index)
{
	// hunt for the given modifier in this object's stack
	SClass_ID	sc;
	// if target_mod is NULL look for indexed sel mod
	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
		{
			for (int i = 0; i < dobj->NumModifiers(); i++)
			{
				IMeshSelectData* emd = NULL;
				LocalModData* lmd = NULL;
				Modifier* mod = dobj->GetModifier(i);
				lmd = dobj->GetModContext(i)->localData;
				if (target_mod && mod == target_mod && lmd != NULL) 
				{
					return GetMeshSelectDataInterface(lmd);
				}
				else if (lmd  != NULL &&
						 (emd = GetMeshSelectDataInterface(lmd)) != NULL &&
						 mod_index-- == 0)
				{
					target_mod = mod;
					return emd;
				}
			}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			sc = dobj->SuperClassID();
		}
	}
	return NULL;
}

MeshDeltaUser*
get_MeshDeltaUser_for_mod(Object* obj, Modifier*& target_mod, int mod_index)
{
	// hunt for the given modifier in this object's stack
	SClass_ID	sc;
	// if target_mod is NULL look for indexed sel mod
	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
		{
			for (int i = 0; i < dobj->NumModifiers(); i++)
			{
				Modifier* mod = dobj->GetModifier(i);
				if (target_mod && mod == target_mod)
					return GetMeshDeltaUserInterface(mod);
				else
				{
					MeshDeltaUser* mdu = GetMeshDeltaUserInterface(mod);
					if ((mdu != NULL) && (mod_index-- == 0))
					{
						target_mod = mod;
						return mdu;
					}
					
				}
			}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			sc = dobj->SuperClassID();
		}
	}
	return NULL;
}

void
world_to_current_coordsys(Point3& p, int mode)
{
	Value* coordsys = thread_local(current_coordsys);
	Matrix3 m (1);
	ViewExp* view = MAXScript_interface->GetActiveViewport();

	if (coordsys == n_local)
	{
		MAXScript_interface->ReleaseViewport(view); // LAM - 8/10/02 - defect 439650
		return;
	}
	else if (coordsys == n_screen)
		view->GetAffineTM(m);
	else if (coordsys == n_grid)
		view->GetConstructionTM(m);
	else if (is_node(coordsys))
		m = coordsys->to_node()->GetObjectTM(MAXScript_time());
	else if (is_matrix3(coordsys))
		m = coordsys->to_matrix3();

	MAXScript_interface->ReleaseViewport(view);
	if (mode & NO_TRANSLATE)
		m.NoTrans();
	if (mode & NO_SCALE)
		m.NoScale();
	// map to current
	p = p * Inverse(m);
}

void
world_from_current_coordsys(Point3& p, int mode)
{
	Value* coordsys = thread_local(current_coordsys);
	Matrix3 m (1);
	ViewExp* view = MAXScript_interface->GetActiveViewport();

	if (coordsys == n_local)
	{
		MAXScript_interface->ReleaseViewport(view); // LAM - 8/10/02 - defect 439650
		return;
	}
	else if (coordsys == n_screen)
		view->GetAffineTM(m);
	else if (coordsys == n_grid)
		view->GetConstructionTM(m);
	else if (is_node(coordsys))
		m = coordsys->to_node()->GetObjectTM(MAXScript_time());
	else if (is_matrix3(coordsys))
		m = coordsys->to_matrix3();

	MAXScript_interface->ReleaseViewport(view);
	if (mode & NO_TRANSLATE)
		m.NoTrans();
	if (mode & NO_SCALE)
		m.NoScale();
	// map from current into world coords
	p = p * m;
}

/*
void 
ValueToBitArrayP(Value* inval, MNMesh* poly, BitArray &theBitArray, int type, TCHAR* errmsg) {
	int nAlive=0;
	int i=0, j=0, outSize=0;
	BitArray userSel;
//	for (i = 0; i < poly->VNum(); i++) {
//		DWORD flag = poly->V(i)->ExportFlags();
//	}

	switch (type)
	{
		case POLY_VERTS:
			outSize=poly->VNum();
			for (i=0; i < outSize; i++) if (!poly->v[i].GetFlag(MN_DEAD)) ++nAlive;
			userSel.SetSize(nAlive);
			ValueToBitArray(inval, userSel, nAlive, errmsg);
			theBitArray.SetSize(outSize);
			for (i=0, j=0; i < outSize; i++) {
				if (poly->v[i].GetFlag(MN_DEAD)) continue;
				if (userSel[j++]) theBitArray.Set(i);
			}
			break;
		case POLY_FACES:
			outSize=poly->FNum();
			for (i=0; i < outSize; i++) if (!poly->f[i].GetFlag(MN_DEAD)) ++nAlive;
			userSel.SetSize(nAlive);
			ValueToBitArray(inval, userSel, nAlive, errmsg);
			theBitArray.SetSize(outSize);
			for (i=0, j=0; i < outSize; i++) {
				if (poly->f[i].GetFlag(MN_DEAD)) continue;
				if (userSel[j++]) theBitArray.Set(i);
			}
			break;
		case POLY_EDGES:
			outSize=poly->ENum();
			for (i=0; i < outSize; i++) if (!poly->e[i].GetFlag(MN_DEAD)) ++nAlive;
			userSel.SetSize(nAlive);
			ValueToBitArray(inval, userSel, nAlive, errmsg);
			theBitArray.SetSize(outSize);
			for (i=0, j=0; i < outSize; i++) {
				if (poly->e[i].GetFlag(MN_DEAD)) continue;
				if (userSel[j++]) theBitArray.Set(i);
			}
	}
}
*/
// if value is a node, do normal poly access for node.
// if value is an editable mesh (baseobject), get its poly. Don't know the INode though
MNMesh* 
_get_polyForValue(Value* value, int accessType, ReferenceTarget** owner, TCHAR* fn) {
	MNMesh* poly = NULL;
	if (owner) {
		*owner = NULL;
	}
	if (value->is_kind_of(class_tag(MAXNode))) {
		get_valid_node(value, fn);
		// pick up node world state mesh.  Complain if write access & object 
		// has a non-empty modifier stack
		Object* obj = ((MAXNode*)value)->to_node()->GetObjectRef();
		Object* base_obj = Get_Object_Or_XRef_BaseObject(obj);
		if (accessType == MESH_WRITE_ACCESS && obj != base_obj)
			throw RuntimeError (GetString(IDS_CANNOT_CHANGE_POLY_WITH_MODIFIERS_PRESENT), obj->GetObjectName());
		else if (accessType == MESH_BASE_OBJ || accessType == MESH_WRITE_ACCESS)
		{
			if (base_obj->ClassID() != EPOLYOBJ_CLASS_ID)
				throw RuntimeError (GetString(IDS_POLY_OPERATION_ON_NONPOLY), obj->GetObjectName());
			if (owner != NULL)
				*owner = base_obj;
			poly = &((PolyObject*)base_obj)->GetMesh();
		}
		else
		{
			ObjectState os = obj->Eval(MAXScript_time());
			obj = (Object*)os.obj;
			if (obj->ClassID() != EPOLYOBJ_CLASS_ID && obj->ClassID() != polyObjectClassID)
				throw RuntimeError (GetString(IDS_POLY_OPERATION_ON_NONPOLY), obj->GetObjectName());
			if (owner != NULL)
				*owner = obj;
			poly = &((PolyObject*)obj)->GetMesh();
		}
	}
	else if (value->tag == class_tag(MAXObject) &&
		    ((MAXObject*)value)->obj->ClassID() == EPOLYOBJ_CLASS_ID) 
	{
		poly = &((PolyObject*)((MAXObject*)value)->obj)->GetMesh();
		if (owner) *owner = ((MAXObject*)value)->obj;
	}
	else {
		throw RuntimeError (GetString(IDS_POLY_OPERATION_ON_NONPOLY), value);
	}
	return poly;
}

void 
ValueToBitArrayP(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg, int selTypesAllowed, MNMesh* poly) {
	if (inval == n_selection) {
		switch (selTypesAllowed)
		{
			case MESH_VERTSEL_ALLOWED:
				poly->getVertexSel(theBitArray);
				break;
			case MESH_FACESEL_ALLOWED:
				poly->getFaceSel(theBitArray);
				break;
			case MESH_EDGESEL_ALLOWED:
				if (poly->GetFlag (MN_MESH_FILLED_IN)) poly->getEdgeSel(theBitArray);
				break;
		}

	}
	else
		ValueToBitArray(inval, theBitArray, maxSize, errmsg, selTypesAllowed);
}

