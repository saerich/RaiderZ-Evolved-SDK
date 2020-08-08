/**********************************************************************
 *<
	FILE: le_tclasses.cpp

	DESCRIPTION: Contains the Implementation of the various types and the root class

	CREATED BY: Luis Estrada

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

//#include "pch.h"
#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"

#include "meshadj.h"
#include <patchobj.h>
#include <euler.h>
#include "resource.h"
#include "agnidefs.h"

#include "MXSAgni.h"
#include "ExtClass.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include	"le_wraps.h"

/*--------------------------------- TestInterface-----------------------------------*/

/*--------------------------------- Interface Class ------------------------------*/

Value*
ConfigureBitmapPaths_cf(Value** arg_list, int count)
{
	
	check_arg_count (ConfigureBitmapPaths,0,count);

	return MAXScript_interface->ConfigureBitmapPaths() ? &true_value : &false_value;
}


Value*
EditAtmosphere_cf(Value** arg_list, int count)
{
	if (count == 0)
		MAXScript_interface->EditAtmosphere(NULL);
	else
	{
		check_arg_count (EditAtmosphere,2,count);
		MAXScript_interface->EditAtmosphere(arg_list[0]->to_atmospheric(), arg_list[1]->to_node());
	}

	return &ok;
}


Value*
CheckForSave_cf(Value** arg_list, int count)
{
	//bool checkforsave
	check_arg_count (CheckForSave, 0, count);

	return MAXScript_interface->CheckForSave () ? &true_value : &false_value;

}

Value*
MatrixFromNormal_cf(Value** arg_list, int count)
{
	Matrix3 result_matrix;
	check_arg_count (MatrixFromNormal, 1, count);

	Point3 the_normal= arg_list[0]->to_point3();
	
	MatrixFromNormal (the_normal, result_matrix);

	return new Matrix3Value (result_matrix);
	
}



/*-------------------------------------------------- Object Class ---------------*/

/* Parameter needed are the time and the object) */
Value*
GetPolygonCount_cf(Value** arg_list, int count)
{
	int faces=0,vertnum=0;
	one_typed_value_local (Array* total) 

	check_arg_count (GetPolygonCount, 1, count);
	INode* node = arg_list[0]->to_node();

	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	GetPolygonCount (MAXScript_time(), refobj, faces, vertnum);
	
	vl.total = new Array (2);
	vl.total->append (Integer::intern(faces));
	vl.total->append (Integer::intern(vertnum));

	return_value (vl.total);	
}



Value*
GetTriMeshFaceCount_cf(Value** arg_list, int count)
{
	int faces=0,vertnum=0;
	one_typed_value_local (Array* total) 

	check_arg_count (GetTriMeshFaceCount, 1, count);
	INode* node = arg_list[0]->to_node();

	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	GetTriMeshFaceCount (MAXScript_time(), refobj, faces, vertnum);
	
	vl.total = new Array (2);
	vl.total->append (Integer::intern(faces));
	vl.total->append (Integer::intern(vertnum));

	return_value (vl.total);	
}


Value*
NumMapsUsed_cf(Value** arg_list, int count)
{
	check_arg_count (NumMapsUsed, 1, count);
	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	return Integer::intern (refobj->NumMapsUsed());
}


Value*
IsPointSelected_cf(Value** arg_list, int count)
{
	int pointindex;
	check_arg_count (IsPointSelected, 2, count);

	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	pointindex = arg_list[1]->to_int()-1;
	if (pointindex < 0)
		throw RuntimeError(GetString(IDS_RK_ISPOINTSELECTED_INDEX_OUT_OF_RANGE));

	return refobj->IsPointSelected(pointindex) ? &true_value : &false_value;
}


Value*
PointSelection_cf(Value** arg_list, int count)
{
	int pointindex;
	check_arg_count (PointSelection, 2, count);

	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	pointindex = arg_list[1]->to_int()-1;
	if (pointindex < 0)
		throw RuntimeError(GetString(IDS_RK_POINTSELECTION_INDEX_OUT_OF_RANGE));
	return Float::intern(refobj->PointSelection(pointindex));
}


Value*
IsShapeObject_cf(Value** arg_list, int count)
{
	check_arg_count (IsShapeObject, 1, count);
	
	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	return refobj->IsShapeObject () ? &true_value : &false_value;
}

Value* 
NumSurfaces_cf(Value** arg_list, int count)
{
	check_arg_count (NumSurfaces, 1, count);
	
	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;

	return Integer::intern(refobj->NumSurfaces (MAXScript_time()));

}

Value*
IsSurfaceUVClosed_cf (Value** arg_list, int count)
{
	check_arg_count (IsSurfaceUVClosed, 2, count);
	one_typed_value_local (Array* total);

	INode* node = arg_list[0]->to_node();
	ObjectState os = node->EvalWorldState(MAXScript_time());
	Object* refobj = os.obj;
	int inu;
	int inv;

	int surfnum = arg_list[1]->to_int();
	if (surfnum < 1 || surfnum > refobj->NumSurfaces(MAXScript_time())) 
		throw RuntimeError(GetString(IDS_RK_ISSURFACEUVCLOSED_INDEX_OUT_OF_RANGE));

	refobj->SurfaceClosed (MAXScript_time(), surfnum-1, inu, inv);
	
	vl.total = new Array (2);
	vl.total->append (inu ? &true_value : &false_value);
	vl.total->append (inv ? &true_value : &false_value);

	return_value (vl.total);	
}

/*------------------------------ Miscellaneous Global Functions -------------*/

Value*
DeselectHiddenEdges_cf(Value **arg_list, int count)
{
	Object *obj;

	check_arg_count (DeselectHiddenEdges, 1, count);

	INode* node = arg_list[0]->to_node();
	
	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		obj = (GeomObject*)os.obj;
		if (obj->ClassID() != GetEditTriObjDesc()->ClassID() &&
			obj->ClassID() != Class_ID(TRIOBJ_CLASS_ID, 0))		
			return &undefined;
	}
	else
		return &undefined;
	
	
	/* get the associated mesh and deselect hidden edges */
	Mesh &themesh = ((TriObject *) obj)->GetMesh();
	DeselectHiddenEdges ( themesh );

	return &ok;
}
	

Value*
DeselectHiddenFaces_cf(Value **arg_list, int count)
{
	Object *obj;
	check_arg_count (DeselectHiddenFaces, 1, count);

	INode* node = arg_list[0]->to_node();
	
	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		obj = (GeomObject*)os.obj;
		if (obj->ClassID() != GetEditTriObjDesc()->ClassID() &&
			obj->ClassID() != Class_ID(TRIOBJ_CLASS_ID, 0))		
			return &undefined;
	}
	else
		return &undefined;
	
	
	/* get the associated mesh and deselect hidden edges */
	Mesh &themesh = ((TriObject*) obj)->GetMesh();
	DeselectHiddenFaces ( themesh );

	return &ok;
}


Value*
AverageSelVertCenter_cf(Value** arg_list, int count)
{
	Object *obj;

	check_arg_count (AverageSelVertCenter, 1, count);

	INode* node = arg_list[0]->to_node();
	
	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		obj = (GeomObject*)os.obj;
		if (obj->ClassID() != GetEditTriObjDesc()->ClassID() &&
			obj->ClassID() != Class_ID(TRIOBJ_CLASS_ID, 0))		
			return &undefined;
	}
	else
		return &undefined;
	
	
	/* get the associated mesh and return the average center of the selected vertices */
	Mesh &themesh = ((TriObject *) obj)->GetMesh();
	return new Point3Value(AverageSelVertCenter ( themesh ) );

}



Value*
AverageSelVertNormal_cf(Value** arg_list, int count)
{
	Object *obj;

	check_arg_count (AverageSelVertNormal, 1, count);

	INode* node = arg_list[0]->to_node();
	
	// Get the object from the node
	ObjectState os = node->EvalWorldState(MAXScript_time());
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		obj = (GeomObject*)os.obj;
		if (obj->ClassID() != GetEditTriObjDesc()->ClassID() &&
			obj->ClassID() != Class_ID(TRIOBJ_CLASS_ID, 0))		
			return &undefined;
	}
	else
		return &undefined;
	
	
	/* get the associated mesh and return the average center of the selected vertices */
	Mesh &themesh = ((TriObject *) obj)->GetMesh();
	return new Point3Value(AverageSelVertNormal ( themesh ) );

}


/* -----------------------------------   Patch Properties ------------------------------- */

#ifndef NO_PATCHES
Value* GetPatchSteps_cf(Value** arg_list, int count)
{
	check_arg_count (GetPatchSteps, 1, count);

	// Get the object from the node
	INode* node = arg_list[0]->to_node();
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());

	if (obj->ClassID() != Class_ID(PATCHOBJ_CLASS_ID,0)) 
		throw RuntimeError (GetString(IDS_RK_PATCH_OPERATION_ON_NONMESH), obj->GetObjectName());

		//throw RuntimeError (_T("Patch operation on non-Patch: "), obj->GetObjectName());

	return Integer::intern( ((PatchObject*)obj)->GetMeshSteps());
}


Value* SetPatchSteps_cf (Value** arg_list, int count)
{
	check_arg_count (SetPatchSteps, 2, count);

	// Get the object from the node
	INode* node = arg_list[0]->to_node();
	Object* obj = Get_Object_Or_XRef_BaseObject(node->GetObjectRef());

	if (obj->ClassID() != Class_ID(PATCHOBJ_CLASS_ID,0))
		throw RuntimeError (GetString(IDS_RK_PATCH_OPERATION_ON_NONMESH), obj->GetObjectName());
	if (arg_list[1]->to_int() < 0 ) 
		throw RuntimeError (GetString(IDS_RK_SETPATCHSTEPS_INDEX_OUT_OF_RANGE));

	((PatchObject*)obj)->SetMeshSteps (arg_list[1]->to_int());
	needs_complete_redraw_set();

	if (MAXScript_interface->GetCommandPanelTaskMode() == TASK_MODE_MODIFY)
		if ((BaseObject*)obj == MAXScript_interface->GetCurEditObject())
			obj->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
			//MAXScript_interface->SetCommandPanelTaskMode (TASK_MODE_MODIFY);

	return &ok;
}
#endif // NO_PATCHES

/* --------------------------- Euler Angles -------------------------------- */

Value* GetEulerQuatAngleRatio_cf (Value** arg_list, int count)
{
	check_arg_count_with_keys(GetEulerQuatAngleRatio, 4, count);
	Quat quat1,quat2;
	Array *euler1, *euler2;
	int angle,size1,size2;

	def_euler_angles();
	Value* theangle = key_arg(angle)->eval();
	if (theangle == &unsupplied)
		angle = EULERTYPE_XYZ;
	else
		angle = GetID(eulerAngles, elements(eulerAngles), theangle); 

	quat1 = arg_list[0]->to_quat();
	quat2 = arg_list[1]->to_quat();
	type_check(arg_list[2], Array, "QuatAngleRatio");
	type_check(arg_list[3], Array, "QuatAngleRatio");
    euler1 = (Array*)arg_list[2];
	size1 = euler1->size;
	euler2 = (Array*)arg_list[3];
	size2 = euler2->size;
	if (!size1 || !size2)
		return &undefined;

	float* eulerarray1 = new float[size1];
    float* eulerarray2 = new float[size2];

	for (int i=0;i<size1;i++)
		eulerarray1[i] = euler1->data[i]->to_float();
	
	for (int i=0;i<size2;i++)
		eulerarray2[i] = euler2->data[i]->to_float();
	
	return Float::intern (GetEulerQuatAngleRatio (quat1, quat2, 
		eulerarray1, eulerarray2, angle));

}

Value* GetEulerMatAngleRatio_cf (Value** arg_list, int count)
{
	check_arg_count_with_keys(GetEulerMatAngleRatio, 4, count);
	Matrix3 mat1, mat2;
	Array *euler1, *euler2;
	int angle,size1,size2;

	def_euler_angles();
	Value* theangle = key_arg(angle)->eval(); 
	if (theangle == &unsupplied)
		angle = EULERTYPE_XYZ;
	else
		angle = GetID(eulerAngles, elements(eulerAngles), theangle); 

	mat1 = arg_list[0]->to_matrix3();
	mat2 = arg_list[1]->to_matrix3();
	type_check(arg_list[2], Array, "MatAngleRatio");
	type_check(arg_list[3], Array, "MatAngleRatio");
    euler1 = (Array*)arg_list[2];
	size1 = euler1->size;
	euler2 = (Array*)arg_list[3];
	size2 = euler2->size;
	if (!size1 || !size2)
		return &undefined;

	float* eulerarray1 = new float[size1];
    float* eulerarray2 = new float[size2];

	for (int i=0;i<size1;i++)
		eulerarray1[i] = euler1->data[i]->to_float();
	
	for (int i=0;i<size2;i++)
		eulerarray2[i] = euler2->data[i]->to_float();

	return Float::intern (GetEulerMatAngleRatio (mat1, mat2,
		eulerarray1, eulerarray2, angle)); 
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global vars here
void le_init()
{
//	#include "le_glbls.h"
}

