#include "avg_maxver.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Arrays.h"
#include "colorval.h"

#include "resource.h"

#include "modstack.h"
#include "meshadj.h"
#include "MeshDelta.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "MXSAgni.h"
#include "meshop_defs.h"
#include "agnidefs.h"

// ============================================================================

#include "defextfn.h"
#	include "namedefs.h"

int IndexedPoint3SortCmp(const void *elem1, const void *elem2 )
{
	IndexedPoint3SortVal* v1 = (IndexedPoint3SortVal*)elem1;
	IndexedPoint3SortVal* v2 = (IndexedPoint3SortVal*)elem2;
	return v1->index - v2->index;
}

// =========================================================================================
//                                      start meshop methods
// =========================================================================================

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// following can be getter/setters for properties
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ============================================================================
// <bitArray> getHiddenVerts <Mesh mesh>
Value*
meshop_getHiddenVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getHiddenVerts, 1, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getHiddenVerts);
	return new BitArrayValue(mesh->vertHide);
}

// ============================================================================
// setHiddenVerts <Mesh mesh> <vertlist>
Value*
meshop_setHiddenVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setHiddenVerts, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setHiddenVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->vertHide=verts;
	if (mdu) mdu->LocalDataChanged (PART_DISPLAY | SELECT_CHANNEL);
	return &ok;
}

// ============================================================================
// <bitArray> getHiddenFaces <Mesh mesh>
Value*
meshop_getHiddenFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getHiddenFaces, 1, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getHiddenFaces);
	int nFaces=mesh->getNumFaces();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int fi = 0; fi < nFaces; fi++) {
		if (mesh->faces[fi].Hidden()) 
			vl.faces->bits.Set(fi);
	}
	return_value(vl.faces);
}

// ============================================================================
// setHiddenFaces <Mesh mesh> <facelist>
Value*
meshop_setHiddenFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setHiddenFaces, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setHiddenFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
 	for (int fi = 0; fi < nFaces; fi++)
		mesh->faces[fi].SetHide(faces[fi]);
	if (mdu) mdu->LocalDataChanged (PART_DISPLAY  | SELECT_CHANNEL);
	return &ok;
}

// ============================================================================
// <bitArray> getOpenEdges <Mesh mesh>
Value*
meshop_getOpenEdges_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getOpenEdges, 1, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getOpenEdges);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (3*mesh->getNumFaces());
	mesh->FindOpenEdges(vl.edges->bits);
	return_value(vl.edges);
}

// no setter for OpenEdges

// ============================================================================
// <int> getNumVerts <Mesh mesh>
Value*
meshop_getNumVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumVerts, 1, arg_count);
	return Integer::intern(get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumVerts)->getNumVerts());
}

// ============================================================================
// setNumVerts <Mesh mesh> <int> 
Value*
meshop_setNumVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setNumVerts, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumVerts);
	int nNewVerts =               arg_list[1]->to_int();
	if (nNewVerts < 0)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	int nVerts=mesh->getNumVerts();
	if (nNewVerts > nVerts) {
		Point3 origin (0.0f, 0.0f, 0.0f);
		for (int i = nVerts; i < nNewVerts; i++) 
			tmd.VCreate (&origin);
	}
	else if (nNewVerts < nVerts) {
		BitArray verts(nVerts);
		for (int i = nVerts; i > nNewVerts; i--) 
			verts.Set(i-1);
		tmd.VDelete (verts);
	}
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// <int> getNumFaces <Mesh mesh>
Value*
meshop_getNumFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumFaces, 1, arg_count);
	return Integer::intern(get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumFaces)->getNumFaces());
}

// ============================================================================
// setNumFaces <Mesh mesh> <int> 
Value*
meshop_setNumFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setNumFaces, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumFaces);
	int nNewFaces =               arg_list[1]->to_int();
	if (nNewFaces < 0)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	if (mesh->getNumVerts() == 0)
		throw RuntimeError (GetString(IDS_MUST_HAVE_AT_LEAST_ONE_VERTEX_TO_CREATE_FACE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	int nFaces=mesh->getNumFaces();
	if (nNewFaces > nFaces) {
		Face v0;
		v0.v[0]=v0.v[1]=v0.v[2]=0;
		for (int i = nFaces; i < nNewFaces; i++) 
			tmd.FCreate (&v0);
	}
	else if (nNewFaces < nFaces) {
		BitArray faces(nFaces);
		for (int i = nFaces; i > nNewFaces; i--) 
			faces.Set(i-1);
		tmd.FDelete (faces);
	}
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

Value* GetNumMapVerts(Value* source, int channel, TCHAR* fn_name);
Value* SetNumMapVerts(Value* source, int channel, int count, bool keep, TCHAR* fn_name);

// ============================================================================
// <int> getNumTVerts <Mesh mesh>
Value* 
meshop_getNumTVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumTVerts, 1, arg_count);
	return GetNumMapVerts(arg_list[0], 1, _T("getNumTVerts"));
}

// ============================================================================
// setNumTVerts <Mesh mesh> <int> 
Value* 
meshop_setNumTVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setNumTVerts, 2, arg_count);
	return SetNumMapVerts(arg_list[0], 1, arg_list[1]->to_int(), TRUE, _T("setNumTVerts"));
}

// ============================================================================
// <int> getNumCPVVerts <Mesh mesh>
Value* 
meshop_getNumCPVVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumCPVVerts, 1, arg_count);
	return GetNumMapVerts(arg_list[0], 0, _T("getNumCPVVerts"));
}

// ============================================================================
// setNumCPVVerts <Mesh mesh> <int> 
Value* 
meshop_setNumCPVVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setNumCPVVerts, 2, arg_count);
	return SetNumMapVerts(arg_list[0], 0, arg_list[1]->to_int(), TRUE, _T("setNumCPVVerts"));
}

// ============================================================================
// <boolean> getDisplacementMapping <Mesh mesh>  // .useDisplacementMapping
Value* 
meshop_getDisplacementMapping_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getDisplacementMapping, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getDisplacementMapping);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return bool_result(owner->CanDoDisplacementMapping());
}

// ============================================================================
// setDisplacementMapping <Mesh mesh> <boolean> 
Value* 
meshop_setDisplacementMapping_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setDisplacementMapping, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setDisplacementMapping);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisableDisplacementMapping(!arg_list[1]->to_bool());
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <boolean> getSubdivisionDisplacement <Mesh mesh>   // .useSubdivisionDisplacement
Value* 
meshop_getSubdivisionDisplacement_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(getSubdivisionDisplacement, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionDisplacement);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return bool_result(owner->DoSubdivisionDisplacment());
}

// ============================================================================
// setSubdivisionDisplacement <Mesh mesh> <boolean> 
Value* 
meshop_setSubdivisionDisplacement_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionDisplacement, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionDisplacement);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DoSubdivisionDisplacment() = (arg_list[1]->to_bool() == TRUE);
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <boolean> getSplitMesh <Mesh mesh> // .displaceSplitMesh
Value* 
meshop_getSplitMesh_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSplitMesh, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSplitMesh);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return bool_result(owner->SplitMeshForDisplacement());
}

// ============================================================================
// setSplitMesh <Mesh mesh> <boolean> 
Value* 
meshop_setSplitMesh_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSplitMesh, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSplitMesh);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	if (arg_list[1]->to_bool())
	{	owner->DoSubdivisionDisplacment() = true;
	    owner->SplitMeshForDisplacement() = true;
	}
	else
		if (owner->DoSubdivisionDisplacment() )
			owner->SplitMeshForDisplacement() = false;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <name> getSubdivisionMethod <Mesh mesh>    // .displaceMethod
Value* 
meshop_getSubdivisionMethod_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionMethod, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionMethod);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	TessApprox& ta = owner->DisplacmentApprox();
	def_emesh_subdiv_method_types();
	return GetName(emeshsubdivmethodtypes, elements(emeshsubdivmethodtypes), owner->DisplacmentApprox().type);
}

// ============================================================================
// setSubdivisionMethod <Mesh mesh> {#regular | #spatial | #curvature | #spatialAndCurvature}
Value* 
meshop_setSubdivisionMethod_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionMethod, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionMethod);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	def_emesh_subdiv_method_types();
	int subdivMethodType = GetID(emeshsubdivmethodtypes, elements(emeshsubdivmethodtypes), arg_list[1]);
	owner->DisplacmentApprox().type = (TessType)subdivMethodType;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <name> getSubdivisionStyle <Mesh mesh>     // .displaceStyle
Value* 
meshop_getSubdivisionStyle_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionStyle, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionStyle);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	def_emesh_subdiv_style_types();
	return GetName(emeshsubdivstyletypes, elements(emeshsubdivstyletypes), owner->DisplacmentApprox().subdiv);
}

// ============================================================================
// setSubdivisionStyle <Mesh mesh> {#tree | #grid | #delaunay} 
Value* 
meshop_setSubdivisionStyle_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionStyle, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionStyle);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	def_emesh_subdiv_style_types();
	int subdivStyleType = GetID(emeshsubdivstyletypes, elements(emeshsubdivstyletypes), arg_list[1]);
	owner->DisplacmentApprox().subdiv = (TessSubdivStyle)subdivStyleType;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <boolean> getSubdivisionView <Mesh mesh>     // .viewDependent
Value* 
meshop_getSubdivisionView_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionView, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionView);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return bool_result(owner->DisplacmentApprox().view);
}

// ============================================================================
// setSubdivisionView <Mesh mesh> <boolean> 
Value* 
meshop_setSubdivisionView_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionView, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionView);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().view = arg_list[1]->to_bool();
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <int> getSubdivisionSteps <Mesh mesh>     // .displaceSteps
Value* 
meshop_getSubdivisionSteps_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionSteps, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionSteps);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Integer::intern(owner->DisplacmentApprox().u);
}

// ============================================================================
// setSubdivisionSteps <Mesh mesh> <int> 
Value* 
meshop_setSubdivisionSteps_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionSteps, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionSteps);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int steps = arg_list[1]->to_int();
	if (steps < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_STEPS_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().u = steps;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <float> getSubdivisionEdge <Mesh mesh>     // .displaceEdge
Value* 
meshop_getSubdivisionEdge_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionEdge, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionEdge);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Float::intern(owner->DisplacmentApprox().edge);
}

// ============================================================================
// setSubdivisionEdge <Mesh mesh> <float> 
Value* 
meshop_setSubdivisionEdge_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionEdge, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionEdge);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float edge = arg_list[1]->to_float();
	if (edge < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_EDGE_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().edge = edge;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <float> getSubdivisionDistance <Mesh mesh>     // .displaceDistance
Value* 
meshop_getSubdivisionDistance_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionDistance, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionDistance);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Float::intern(owner->DisplacmentApprox().dist);
}

// ============================================================================
// setSubdivisionDistance <Mesh mesh> <float> 
Value* 
meshop_setSubdivisionDistance_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionDistance, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionDistance);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float distance = arg_list[1]->to_float();
	if (distance < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_DISTANCE_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().dist = distance;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <float> getSubdivisionAngle <Mesh mesh>     // .displaceAngle
Value* 
meshop_getSubdivisionAngle_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionAngle, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionAngle);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Float::intern(owner->DisplacmentApprox().ang);
}

// ============================================================================
// setSubdivisionAngle <Mesh mesh> <float> 
Value* 
meshop_setSubdivisionAngle_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionAngle, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionAngle);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float angle = arg_list[1]->to_float();
	if (angle < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_ANGLE_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().ang = angle;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <int> getSubdivisionMinLevels <Mesh mesh>     // .displaceMinLevels
Value* 
meshop_getSubdivisionMinLevels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionMinLevels, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionMinLevels);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Integer::intern(owner->DisplacmentApprox().minSub);
}

// ============================================================================
// setSubdivisionMinLevels <Mesh mesh> <int> 
Value* 
meshop_setSubdivisionMinLevels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionMinLevels, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionMinLevels);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int levels = arg_list[1]->to_int();
	if (levels < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_LEVELS_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().minSub = levels;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <int> getSubdivisionMaxLevels <Mesh mesh>     // .displaceMaxLevels
Value* 
meshop_getSubdivisionMaxLevels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionMaxLevels, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionMaxLevels);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Integer::intern(owner->DisplacmentApprox().maxSub);
}

// ============================================================================
// setSubdivisionMaxLevels <Mesh mesh> <int> 
Value* 
meshop_setSubdivisionMaxLevels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionMaxLevels, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionMaxLevels);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int levels = arg_list[1]->to_int();
	if (levels < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_LEVELS_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().maxSub = levels;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}


// ============================================================================
// <int> getSubdivisionMaxTriangles <Mesh mesh>      // .displaceMaxTris
Value* 
meshop_getSubdivisionMaxTriangles_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSubdivisionMaxTriangles, 1, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, getSubdivisionMaxTriangles);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	return Integer::intern(owner->DisplacmentApprox().maxSub);
}

// ============================================================================
// setSubdivisionMaxTriangles <Mesh mesh> <int> 
Value* 
meshop_setSubdivisionMaxTriangles_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSubdivisionMaxTriangles, 2, arg_count);
	TriObject* owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, (ReferenceTarget**)&owner, setSubdivisionMaxTriangles);
	if (!owner)
		throw RuntimeError (GetString(IDS_DISPLACEMENT_MAPPING_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int maxTris = arg_list[1]->to_int();
	if (maxTris < 0)
		throw RuntimeError (GetString(IDS_SUBDIVISION_TRIANGLES_MUST_NOT_BE_NEG_NUMBER), arg_list[1]);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	owner->DisplacmentApprox().maxSub = maxTris;
	if (mdu) mdu->UpdateApproxUI();
	return &ok;
}

// ============================================================================
// <boolean> getSelByVertex <Mesh mesh> // selByVert
Value* 
meshop_getSelByVertex_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSelByVertex, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getSelByVertex);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiSelByVert, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setSelByVertex <Mesh mesh> <boolean> 
Value* 
meshop_setSelByVertex_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSelByVertex, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setSelByVertex);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiSelByVert, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <boolean> getIgnoreBackfacing <Mesh mesh> 
Value* 
meshop_getIgnoreBackfacing_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getIgnoreBackfacing, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getIgnoreBackfacing);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiIgBack, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setIgnoreBackfacing <Mesh mesh> <boolean> 
Value* 
meshop_setIgnoreBackfacing_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setIgnoreBackfacing, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setIgnoreBackfacing);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiIgBack, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <boolean> getIgnoreVisEdges <Mesh mesh> 
Value* 
meshop_getIgnoreVisEdges_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getIgnoreVisEdges, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getIgnoreVisEdges);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiIgnoreVis, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setIgnoreVisEdges <Mesh mesh> <boolean> 
Value* 
meshop_setIgnoreVisEdges_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setIgnoreVisEdges, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setIgnoreVisEdges);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiIgnoreVis, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <boolean> getSoftSel <Mesh mesh>  // .useSoftSel
Value* 
meshop_getSoftSel_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSoftSel, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getSoftSel);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiSoftSel, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setSoftSel <Mesh mesh> <boolean> 
Value* 
meshop_setSoftSel_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSoftSel, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setSoftSel);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiSoftSel, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <boolean> getSSUseEdgeDist <Mesh mesh>  // .ssUseEdgeDist
Value* 
meshop_getSSUseEdgeDist_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSSUseEdgeDist, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getSSUseEdgeDist);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiSSUseEDist, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setSSUseEdgeDist <Mesh mesh> <boolean> 
Value* 
meshop_setSSUseEdgeDist_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSSUseEdgeDist, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setSSUseEdgeDist);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiSSUseEDist, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <int> getSSEdgeDist <Mesh mesh>  // .ssEdgeDist
Value* 
meshop_getSSEdgeDist_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getSSEdgeDist, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getSSEdgeDist);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiSSEDist, paramVal);
	return Integer::intern(paramVal);
}

// ============================================================================
// setSSEdgeDist <Mesh mesh> <int> 
Value* 
meshop_setSSEdgeDist_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setSSEdgeDist, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setSSEdgeDist);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiSSEDist, arg_list[1]->to_int());
	return &ok;
}

// ============================================================================
// <boolean> getAffectBackfacing <Mesh mesh>  // .affectBackfacing
Value* 
meshop_getAffectBackfacing_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getAffectBackfacing, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getAffectBackfacing);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiSSBack, paramVal);
	return bool_result(!paramVal);
}

// ============================================================================
// setAffectBackfacing <Mesh mesh> <boolean> 
Value* 
meshop_setAffectBackfacing_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setAffectBackfacing, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setAffectBackfacing);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiSSBack, !(arg_list[1]->to_bool()));
	return &ok;
}

// ============================================================================
// <int> getWeldPixels <Mesh mesh>  // .weldPixels
Value* 
meshop_getWeldPixels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getWeldPixels, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getWeldPixels);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiWeldBoxSize, paramVal);
	return Integer::intern(paramVal);
}

// ============================================================================
// setWeldPixels <Mesh mesh> <int> 
Value* 
meshop_setWeldPixels_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setWeldPixels, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setWeldPixels);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiWeldBoxSize, arg_list[1]->to_int());
	return &ok;
}

// ============================================================================
// <int> getExtrusionType <Mesh mesh>  // .extrusionType
Value* 
meshop_getExtrusionType_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getExtrusionType, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getExtrusionType);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiExtrudeType, paramVal);
	return Integer::intern(paramVal);
}

// ============================================================================
// setExtrusionType <Mesh mesh> <int> 
Value* 
meshop_setExtrusionType_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setExtrusionType, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setExtrusionType);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiExtrudeType, arg_list[1]->to_int());
	return &ok;
}

// ============================================================================
// <boolean> getShowVNormals <Mesh mesh>  // .showVNormals
Value* 
meshop_getShowVNormals_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getShowVNormals, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getShowVNormals);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiShowVNormals, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setShowVNormals <Mesh mesh> <boolean> 
Value* 
meshop_setShowVNormals_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setShowVNormals, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setShowVNormals);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiShowVNormals, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <boolean> getShowFNormals <Mesh mesh>  // .showFNormals
Value* 
meshop_getShowFNormals_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getShowFNormals, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getShowFNormals);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	int paramVal;
	mdu->GetUIParam(MuiShowFNormals, paramVal);
	return bool_result(paramVal);
}

// ============================================================================
// setShowFNormals <Mesh mesh> <boolean> 
Value* 
meshop_setShowFNormals_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setShowFNormals, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setShowFNormals);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiShowFNormals, arg_list[1]->to_bool());
	return &ok;
}

// ============================================================================
// <float> getPlanarThreshold <Mesh mesh>  // .planarThreshold
Value* 
meshop_getPlanarThreshold_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getPlanarThreshold, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getPlanarThreshold);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiPolyThresh, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setPlanarThreshold <Mesh mesh> <float> 
Value* 
meshop_setPlanarThreshold_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setPlanarThreshold, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setPlanarThreshold);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiPolyThresh, arg_list[1]->to_float());
	return &ok;
}

// ============================================================================
// <float> getFalloff <Mesh mesh>  // .falloff
Value* 
meshop_getFalloff_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getFalloff, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getFalloff);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiFalloff, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setFalloff <Mesh mesh> <float> 
Value* 
meshop_setFalloff_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setFalloff, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFalloff);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiFalloff, arg_list[1]->to_float());
	return &ok;
}

// ============================================================================
// <float> getPinch <Mesh mesh>  // .pinch
Value* 
meshop_getPinch_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getPinch, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getPinch);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiPinch, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setPinch <Mesh mesh> <float> 
Value* 
meshop_setPinch_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setPinch, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setPinch);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiPinch, arg_list[1]->to_float());
	return &ok;
}

// ============================================================================
// <float> getBubble <Mesh mesh>  // .bubble
Value* 
meshop_getBubble_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getBubble, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getBubble);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiBubble, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setBubble <Mesh mesh> <float> 
Value* 
meshop_setBubble_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setBubble, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setBubble);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiBubble, arg_list[1]->to_float());
	return &ok;
}

// ============================================================================
// <float> getWeldThreshold <Mesh mesh>  // .WeldThreshold
Value* 
meshop_getWeldThreshold_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getWeldThreshold, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getWeldThreshold);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiWeldDist, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setWeldThreshold <Mesh mesh> <float> 
Value* 
meshop_setWeldThreshold_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setWeldThreshold, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setWeldThreshold);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiWeldDist, arg_list[1]->to_float());
	return &ok;
}

// ============================================================================
// <float> getNormalSize <Mesh mesh>  // .normalSize
Value* 
meshop_getNormalSize_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNormalSize, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getNormalSize);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	float paramVal;
	mdu->GetUIParam(MuiNormalSize, paramVal);
	return Float::intern(paramVal);
}

// ============================================================================
// setNormalSize <Mesh mesh> <float> 
Value* 
meshop_setNormalSize_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setNormalSize, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNormalSize);
	MeshDeltaUser *mdu = GetMeshDeltaUserInterface(owner);
	if (!mdu)
		throw RuntimeError (GetString(IDS_PROPERTY_NOT_APPLICABLE_TO_TRIMESH), arg_list[0]);
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mdu->SetUIParam(MuiNormalSize, arg_list[1]->to_float());
	return &ok;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`
// end of getter/setters for properties
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`


///////////////////////////////////////////////////////////////////////////////////////
//                           get SO bitarray based on SO bitarray
///////////////////////////////////////////////////////////////////////////////////////

// <bitarray> getEdgesUsingVert <Mesh mesh> <vertlist> 
Value*
meshop_getEdgesUsingVert_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getEdgesUsingVert, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgesUsingVert);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	int nFaces=mesh->getNumFaces();
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (3*nFaces);
	for (int i=0; i < nFaces; i++) {
		Face f = mesh->faces[i];
		for (int j=0; j < 3; j++) {
			if (verts[f.v[j]] || verts[f.v[(j+1)%3]]) 
				vl.edges->bits.Set(i*3+j);
		}
	}
	return_value(vl.edges);
}

// <bitarray> getFacesUsingVert <Mesh mesh> <vertlist> 
Value*
meshop_getFacesUsingVert_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getFacesUsingVert, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFacesUsingVert);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	int nFaces=mesh->getNumFaces();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<nFaces; i++) {
		for (int j=0; j<3; j++) {
			if (verts[mesh->faces[i].v[j]]) {
				vl.faces->bits.Set(i);
				break;
			}
		}
	}
	return_value(vl.faces);
}

// <bitarray> getPolysUsingVert <mesh> <vertlist> ignoreVisEdges:<boolean=false> threshhold:<float=45.> 
Value*
meshop_getPolysUsingVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getPolysUsingVert, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getPolysUsingVert);
	Value* tmp;
	BOOL igVis = bool_key_arg(ignoreVisEdges, tmp, FALSE); 
	float thresh = float_key_arg(threshhold, tmp, 45.0f); 
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	int nFaces=mesh->getNumFaces();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<mesh->getNumFaces(); i++) {
		if (verts[mesh->faces[i].v[0]] || verts[mesh->faces[i].v[1]] || verts[mesh->faces[i].v[2]])
			mesh->PolyFromFace (i, vl.faces->bits, DegToRad(thresh), igVis);
	}
	return_value(vl.faces);
}

// <bitarray> getVertsUsingEdge <Mesh mesh> <edgelist> 
Value*
meshop_getVertsUsingEdge_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getVertsUsingEdge, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsingEdge);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges(nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue (mesh->getNumVerts());
	for (int ei = 0; ei < nEdges; ei++) {
		if (edges[ei]) {
			int fi = ei/3;
			int vi = ei%3;
			vl.verts->bits.Set(mesh->faces[fi].v[vi]);
			vl.verts->bits.Set(mesh->faces[fi].v[(vi+1)%3]);
		}
	}
	return_value(vl.verts);
}

// <bitarray> getFacesUsingEdge <Mesh mesh> <edgelist> 
Value*
meshop_getFacesUsingEdge_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getFacesUsingEdge, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFacesUsingEdge);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges(nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (mesh->getNumFaces());
	for (int ei = 0; ei < nEdges; ei++) {
		if (edges[ei]) 
			vl.faces->bits.Set(ei/3);
	}
	return_value(vl.faces);
}

// <bitarray> getPolysUsingEdge <Mesh mesh> <edgelist> ignoreVisEdges:<boolean=false> threshhold:<float=45.>
Value*
meshop_getPolysUsingEdge_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getPolysUsingEdge, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getPolysUsingEdge);
	Value* tmp;
	BOOL igVis = bool_key_arg(ignoreVisEdges, tmp, FALSE); 
	float thresh = float_key_arg(threshhold, tmp, 45.0f); 
	int nEdges=3*mesh->getNumFaces();
	BitArray edges(nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	int nFaces=mesh->getNumFaces();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<nFaces; i++) {
		if (edges[i*3] || edges[i*3+1] || edges[i*3+2])
			mesh->PolyFromFace (i, vl.faces->bits, DegToRad(thresh), igVis);
	}
	return_value(vl.faces);
}

// <bitarray> getVertsUsingFace <Mesh mesh> <facelist> 
Value*
meshop_getVertsUsingFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getVertsUsingFace, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsingFace);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue (mesh->getNumVerts());
	for (int fi = 0; fi < nFaces; fi++) {
		if (faces[fi]) {
			for (int j=0; j<3; j++)
				vl.verts->bits.Set(mesh->faces[fi].v[j]);
		}
	}
	return_value(vl.verts);
}

// <bitarray> getEdgesUsingFace <Mesh mesh> <facelist> 
Value*
meshop_getEdgesUsingFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getEdgesUsingFace, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgesUsingFace);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (3*nFaces);
	for (int fi = 0; fi < nFaces; fi++) {
		if (faces[fi]) {
			for (int j=0; j<3; j++)
				vl.edges->bits.Set(3*fi+j);
		}
	}
	return_value(vl.edges);
}

// <bitarray> getPolysUsingFace <Mesh mesh> <facelist> ignoreVisEdges:<boolean=false> threshold:<float=45>
Value*
meshop_getPolysUsingFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getPolysUsingFace, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getPolysUsingFace);
	Value* tmp;
	BOOL igVis = bool_key_arg(ignoreVisEdges, tmp, FALSE); 
	float thresh = float_key_arg(threshhold, tmp, 45.0f); 
	int nFaces=mesh->getNumFaces();
	BitArray infaces(nFaces);
	ValueToBitArrayM(arg_list[1], infaces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<nFaces; i++) {
		if (infaces[i]) 
			mesh->PolyFromFace (i, vl.faces->bits, DegToRad(thresh), igVis);
	}
	return_value(vl.faces);
}

// <bitarray> getEdgesReverseEdge <Mesh mesh> <edgelist> 
Value*
meshop_getEdgesReverseEdge_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getEdgesReverseEdge, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgesReverseEdge);
	int nFaces=mesh->getNumFaces();
	BitArray inedges(3*nFaces);
	ValueToBitArrayM(arg_list[1], inedges, 3*nFaces, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (3*nFaces);
	AdjEdgeList el(*mesh);

// ?? redo by steping through el and using MEdge::Selected (which takes the selection bitarray as an argument)
// see EditMeshMod::AutoEdge for example...
	for (int i=0; i < nFaces; i++) {
		Face & f = mesh->faces[i];
		for (int j=0; j < 3; j++) {
			if (inedges[i*3+j]) {
				int me = el.FindEdge(f.v[j], f.v[(j+1)%3]);
				if (me < 0) break;
				MEdge e = el.edges[me];
				if (e.f[0] == UNDEFINED || e.f[1] == UNDEFINED) break;  // LAM - 8/23/03 - defect 477328
				int otherFace = e.f[0];
				if (otherFace == i) otherFace=e.f[1];
				int otherEdge = mesh->faces[otherFace].GetEdgeIndex(e.v[1],e.v[0]);
				vl.edges->bits.Set(otherFace*3+otherEdge);
			}
		}
	}
	return_value(vl.edges);
}

// <bitarray> getElementsUsingFace <Mesh mesh> <facelist> fence:<facelist=unsupplied>
Value*
meshop_getElementsUsingFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getElementsUsingFace, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getElementsUsingFace);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	BitArray fencefaces(nFaces);
	Value* fence = key_arg(fence);
	if (fence != &unsupplied) 
		ValueToBitArrayM(fence, fencefaces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	AdjEdgeList ae(*mesh);
	AdjFaceList af(*mesh, ae);
	one_typed_value_local(BitArrayValue* elementfaces);
	vl.elementfaces = new BitArrayValue (nFaces);
	BitArray fromThisFace (nFaces);
	for (int i=0; i<nFaces; i++) {
		if (faces[i]) {
			fromThisFace=fencefaces;
			mesh->ElementFromFace (i, fromThisFace, &af);
			vl.elementfaces->bits |= fromThisFace;
		}
	}
	return_value(vl.elementfaces);
}

// <bitarray> getVertsUsedOnlyByFaces <Mesh mesh> <facelist>
Value*
meshop_getVertsUsedOnlyByFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getVertsUsedOnlyByFaces, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsedOnlyByFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue (mesh->getNumVerts());
	mesh->FindVertsUsedOnlyByFaces(faces, vl.verts->bits);
	return_value(vl.verts);
}


///////////////////////////////////////////////////////////////////////////////////////
//                           info utilities
///////////////////////////////////////////////////////////////////////////////////////


// <array> getVertexAngles <Mesh mesh> <vertlist>
Value*
meshop_getVertexAngles_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getVertexAngles, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertexAngles);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	float* angles = new float [nVerts];
	mesh->FindVertexAngles(angles, &verts);
	one_typed_value_local(Array* result);
	vl.result = new Array (nVerts);
	for (int i = 0; i < nVerts; i++) {
		if (verts[i])
			vl.result->append(Float::intern(angles[i]));
		else
			vl.result->append(&undefined);
	}
	delete [] angles;
	return_value(vl.result);
}

// <float> getFaceArea <Mesh mesh> <facelist>
Value*
meshop_getFaceArea_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getFaceArea, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceArea);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float area=0.0f;
	for (int fi = 0; fi < nFaces; fi++) {
		if (faces[fi]) {
			area += Length(mesh->FaceNormal(fi));
		}
	}
	return Float::intern(area/2.0f);
}

// <point3> getFaceCenter <Mesh mesh> <int faceIndex> node:<node=unsupplied>
Value* 
meshop_getFaceCenter_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getFaceCenter, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceCenter);
	int faceIndex =               arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, mesh->getNumFaces(), GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE))
	Point3 p = mesh->FaceCenter(faceIndex); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
//	else world_to_current_coordsys(p);
	return new Point3Value(p);
}

// <point3> getBaryCoords <Mesh mesh> <int faceIndex> <point3 pos> node:<node=unsupplied>
Value* 
meshop_getBaryCoords_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getBaryCoords, 3, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getBaryCoords);
	int faceIndex =               arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, mesh->getNumFaces(), GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE))
	Point3 p =                    arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(p);
//	else world_from_current_coordsys(p);
	return new Point3Value(mesh->BaryCoords(faceIndex, p)); 
}

// <bitarray> getIsoVerts <Mesh mesh>
Value*
meshop_getIsoVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getIsoVerts, 1, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getIsoVerts);
	return new BitArrayValue(mesh->GetIsoVerts());
}


// <array> getFaceRNormals <Mesh mesh> <int faceIndex> node:<node=unsupplied>
Value* 
meshop_getFaceRNormals_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getFaceRNormals, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceRNormals);
	int faceIndex =               arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, mesh->getNumFaces(), GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE))

	one_typed_value_local(Array* result);
	vl.result = new Array (3);

	mesh->checkNormals(TRUE);
	MtlID fmtl = mesh->getFaceMtlIndex(faceIndex);
	Face *curf = &mesh->faces[faceIndex];
	DWORD *vp = mesh->faces[faceIndex].getAllVerts();
	DWORD smg = curf->getSmGroup();
	if (smg == 0)
	{
		for (int j=0; j<3; j++) vl.result->append (new Point3Value (mesh->getFaceNormal (faceIndex)));
		return_value (vl.result);
	}
	Point3 p;
	Value* inNode = key_arg(node);
	for(int j = 0; j < 3; j++) {
		DWORD cv = vp[j];
		RVertex* crv = &mesh->getRVert(cv);
		int numNor = (int)(crv->rFlags & NORCT_MASK);
		if (numNor == 1) {
			p = crv->rn.getNormal();
			if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys_rotate(p); // lam - 6/13/02 - defect 412295
			else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys_rotate(p); // lam - 6/13/02 - defect 412295
		//	else world_to_current_coordsys(p);
			vl.result->append(new Point3Value(p));
		}
		else {
			for (int j = 0; j < numNor; j++) {
				RNormal &rn = crv->ern[j];
				if ((smg & rn.getSmGroup()) && fmtl == rn.getMtlIndex()) { // lam - 6/13/02 - defect 311139
					p = rn.getNormal();
					if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys_rotate(p); // lam - 6/13/02 - defect 412295
					else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys_rotate(p); // lam - 6/13/02 - defect 412295
				//	else world_to_current_coordsys(p);
					vl.result->append(new Point3Value(p));
					break;
				}
			}
		}
	}
	return_value(vl.result);
}

// <float> minVertexDistanceFrom <Mesh mesh> <int vertIndex> <vertlist>
Value*
meshop_minVertexDistanceFrom_cf(Value** arg_list, int arg_count)
{
	check_arg_count(minVertexDistanceFrom, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, minVertexDistanceFrom);
	int vertIndex =  arg_list[1]->to_int()-1;
	int nVerts=mesh->getNumVerts();
	range_check(vertIndex+1, 1, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE))
	Point3 p = mesh->verts[vertIndex];
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[2], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	if (verts.IsEmpty()) return Float::intern(-1.0f);
	bool first = true;
	float minDist = -1.0f;
	for (int i = 0; i < nVerts; i++) {
		if (!verts[i]) continue;
		float tDist = LengthSquared(p - mesh->verts[i]);
		if (first) minDist = tDist, first=false;
		else if (tDist < minDist)  minDist = tDist;
	}
	return Float::intern(sqrt(minDist));
}

// <array> minVertexDistancesFrom <Mesh mesh> <vertlist> <int iterations>
Value*
meshop_minVertexDistancesFrom_cf(Value** arg_list, int arg_count)
{
	check_arg_count(minVertexsDistanceFrom, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, minVertexDistancesFrom);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	int iters =  arg_list[2]->to_int();
	BitArray oldSel (mesh->vertSel);
	mesh->vertSel = verts;
	float* selDist = new float [nVerts];
	SelectionDistance(*mesh, selDist, iters);
	mesh->vertSel = oldSel;
	one_typed_value_local(Array* result);
	vl.result = new Array (nVerts);
	for (int i = 0; i < nVerts; i++) 
		vl.result->append(Float::intern( selDist[i] ));
	delete [] selDist;
	return_value(vl.result);
}



///////////////////////////////////////////////////////////////////////////////////////
//                           operations: mesh level
///////////////////////////////////////////////////////////////////////////////////////


// meshop.attach $.baseobject $sphere02 targetNode:$
// meshop.attach $box01 $box02 attachMat:#IDToMat condenseMat:true deleteSourceNode:false
Value*
meshop_attach_cf(Value** arg_list, int count)
{	
// meshop.attach {<target_node> | <target_mesh>} {<source_node> | <source_mesh>} 
//               targetNode:<target_node> sourceNode:<source_node>
//               attachMat:{#neither | #MatToID | #IDToMat = #neither} condenseMat:<boolean = true>
//               deleteSourceNode:<boolean = true>
// 
	check_arg_count_with_keys(attach, 2, count);
	ReferenceTarget *owner;
	INode *node1, *node2;
	Value *tmp;
	bool sourceInLocalCoords = true;
	Mesh* targetMesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, attach);
	if (arg_list[0]->is_kind_of(class_tag(MAXNode)))
		node1 = arg_list[0]->to_node();
	else 
		node1 = node_key_arg(targetNode, tmp, NULL); 
	Mesh* sourceMesh = get_worldMeshForValue(arg_list[1], attach, false);
	if (arg_list[1]->is_kind_of(class_tag(MAXNode))) {
		sourceInLocalCoords = false;
		node2 = arg_list[1]->to_node();
	}
	else 
		node2 = node_key_arg(sourceNode, tmp, NULL); 
	bool bothAreNodes = node1 && node2;
	def_mtl_attach_types();
	int attachMat = GetID(mtlAttachTypes, elements(mtlAttachTypes), key_arg_or_default(attachMat, n_neither));
	BOOL condenseMat = bool_key_arg(condenseMat, tmp, TRUE);
	BOOL deleteSourceNode = bool_key_arg(deleteSourceNode, tmp, TRUE);

	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = node1 ? node1->GetMtl() : NULL;
	Mtl *m2 = node2 ? node2->GetMtl() : NULL;
	bool condenseMe = true;
	bool canUndo = true;
	if (bothAreNodes && m1 && m2 && (m1 != m2)) {
		if (attachMat == ATTACHMAT_IDTOMAT) {
			FitMeshIDsToMaterial (*targetMesh, m1);
			FitMeshIDsToMaterial (*sourceMesh, m2);
			if (condenseMat) condenseMe = true;
		}

		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		HoldSuspend theHoldSuspender (TRUE);
		if (attachMat == ATTACHMAT_MATTOID) {
			m1 = FitMaterialToMeshIDs (*targetMesh, m1);
			m2 = FitMaterialToMeshIDs (*sourceMesh, m2);
		}
		Mtl *multi = CombineMaterials(m1, m2, mat2Offset);
		theHoldSuspender.Resume();
		canUndo = false;	// Absolutely cannot undo material combinations.
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		// We can't be in face subobject mode, else we screw up the materials:
		if (mdu) {
			DWORD oldSL = mdu->GetEMeshSelLevel();
			if (oldSL>EM_SL_EDGE) mdu->SetEMeshSelLevel(EM_SL_OBJECT);
			node1->SetMtl(multi);
			if (oldSL>EM_SL_EDGE) mdu->SetEMeshSelLevel(oldSL);
		}
		else
			node1->SetMtl(multi);
		m1 = multi;
	}
	if (bothAreNodes && !m1 && m2) {
		// This material operation seems safe.
		// We can't be in face subobject mode, else we screw up the materials:
		if (mdu) {
			DWORD oldSL = mdu->GetEMeshSelLevel();
			if (oldSL>EM_SL_EDGE) mdu->SetEMeshSelLevel(EM_SL_OBJECT);
			node1->SetMtl(m2);
			if (oldSL>EM_SL_EDGE) mdu->SetEMeshSelLevel(oldSL);
		}
		else
			node1->SetMtl(m2);
		m1 = m2;
	}

	// Construct a transformation that takes a vertex out of the space of
	// the source node and puts it into the space of the destination node.
	// mesh of source is in world space coordinates
	Matrix3 node1tm = node1 ? node1->GetObjectTM(MAXScript_time()) : Matrix3(TRUE);
	Matrix3 node2tm = (sourceInLocalCoords && node2) ? node2->GetObjectTM(MAXScript_time()) : Matrix3(TRUE);
	Matrix3 tm = node2tm * Inverse(node1tm);

	MeshDelta tmd;
	tmd.AttachMesh (*targetMesh, *sourceMesh, tm, mat2Offset);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else {
		tmd.Apply (*targetMesh);
//		DWORD partsChanged = tmd.PartsChanged ();
//		if (partsChanged & PART_TOPO) targetMesh->InvalidateTopologyCache();
//		else if (partsChanged & PART_GEOM) targetMesh->InvalidateGeomCache ();
	}

	if (deleteSourceNode && node2) MAXScript_interface->DeleteNode (node2, FALSE);

	if (m1 && condenseMe) {
		// Following clears undo stack.
		m1 = CondenseMatAssignments (*targetMesh, m1);
	}
	if (!canUndo) {
		theHold.Release();
		HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
		MAXScript_interface->FlushUndoBuffer();
	}
	needs_redraw_set();
	return &ok;
}

// <boolean> removeDegenerateFaces <Mesh mesh>
Value* 
meshop_removeDegenerateFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(removeDegenerateFaces, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, removeDegenerateFaces);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	BOOL res = mesh->RemoveDegenerateFaces();
	if (res && mdu) {
		mdu->LocalDataChanged (ALL_CHANNELS);
		theHold.Release();
		HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
		MAXScript_interface->FlushUndoBuffer();
	}
	return bool_result(res);
}

// <boolean> removeIllegalFaces <Mesh mesh>
Value* 
meshop_removeIllegalFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(removeIllegalFaces, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, removeIllegalFaces);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	BOOL res = mesh->RemoveIllegalFaces();
	if (res && mdu) {
		mdu->LocalDataChanged (ALL_CHANNELS);
		theHold.Release();
		HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
		MAXScript_interface->FlushUndoBuffer();
	}
	return bool_result(res);
}

// deleteIsoVerts <Mesh mesh>
Value*
meshop_deleteIsoVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(deleteIsoVerts, 1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteIsoVerts);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.DeleteIsoVerts (*mesh);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// optimize <Mesh mesh> <float normalThreshold> <float edgeThreshold> <float bias> <float maxEdge> 
//          saveMatBoundries:<boolean=true> saveSmoothBoundries:<boolean=true> autoEdge:<boolean=true>
Value*
meshop_optimize_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(optimize, 5, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, optimize);
	float normThresh =   DegToRad(arg_list[1]->to_float());
	float edgeThresh =   DegToRad(arg_list[2]->to_float());
	float bias =                  arg_list[3]->to_float();
	float maxEdge =               arg_list[4]->to_float();
	Value* tmp;
	BOOL saveMatBoundries = bool_key_arg(saveMatBoundries, tmp, TRUE); 
	BOOL saveSmoothBoundries = bool_key_arg(saveSmoothBoundries, tmp, TRUE); 
	BOOL autoEdge = bool_key_arg(autoEdge, tmp, TRUE); 
	DWORD flags = 0;
	if (saveMatBoundries) flags += OPTIMIZE_SAVEMATBOUNDRIES;
	if (saveSmoothBoundries) flags += OPTIMIZE_SAVESMOOTHBOUNDRIES;
	if (autoEdge) flags += OPTIMIZE_AUTOEDGE;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->Optimize(normThresh, edgeThresh, bias, maxEdge, flags, NULL);
	if (mdu) {
		mdu->LocalDataChanged (ALL_CHANNELS);
		theHold.Release();
		HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
		MAXScript_interface->FlushUndoBuffer();
	}
	return &ok;
}

// cut <Mesh mesh> <int edge1Index> <float edge1f> <int edge2Index> <float edge2f> <point3 normal> 
//     fixNeighbors:<boolean=true> split:<boolean=true> node:<node=unsupplied>
Value* 
meshop_cut_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(cut, 6, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, cut);
	int edge1Index =              arg_list[1]->to_int()-1;
	float edge1f =                arg_list[2]->to_float();
	int edge2Index =              arg_list[3]->to_int()-1;
	float edge2f =                arg_list[4]->to_float();
	Point3 normal =               arg_list[5]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	int nEdges = 3*mesh->getNumFaces();
	range_check(edge1Index+1, 1, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE))
	range_check(edge2Index+1, 1, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE))
	Value* tmp;
	bool fixNeighbors = bool_key_arg(fixNeighbors, tmp, TRUE) == TRUE; 
	bool split = bool_key_arg(split, tmp, FALSE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.Cut (*mesh, edge1Index, edge1f, edge2Index, edge2f, normal, fixNeighbors, split);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// explodeAllFaces <Mesh mesh> <float threshold>
Value*
meshop_explodeAllFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(explodeAllFaces, 2, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, explodeAllFaces);
	float threshold =             arg_list[1]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.ExplodeFaces (*mesh, threshold);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}


///////////////////////////////////////////////////////////////////////////////////////
//                           operations: vertex level
///////////////////////////////////////////////////////////////////////////////////////

// <boolean> weldVertsByThreshold <Mesh mesh> <vertlist> <float tolerance>
Value*
meshop_weldVertsByThreshold_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(weldVertsByThreshold, 3, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldVertsByThreshold);
	int nVerts=mesh->getNumVerts();
	BitArray whichVerts (nVerts);
	ValueToBitArrayM(arg_list[1], whichVerts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	float tol =     arg_list[2]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	BOOL found = tmd.WeldByThreshold (*mesh, whichVerts, tol);
	if (found) {
		if (mdu) 
			GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
		else 
			tmd.Apply (*mesh);
	}
	return bool_result(found);
}

// weldVertSet <Mesh mesh> <vertlist> weldpoint:<point3=unsupplied> node:<node=unsupplied>
Value*
meshop_weldVertSet_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(weldVertSet, 2, count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldVertSet);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Value* tmp = key_arg(weldPoint);
	Point3* weldpoint= (is_point3(tmp)) ? &tmp->to_point3() : NULL;
	if (weldpoint) {
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(*weldpoint);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(*weldpoint);
	//	else world_from_current_coordsys(&weldpoint);
	}
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.WeldVertSet (*mesh, verts, weldpoint);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// deleteVerts <Mesh mesh> <vertlist>
Value*
meshop_deleteVerts_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(deleteVerts, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.DeleteVertSet (*mesh, verts);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// createPolygon <Mesh mesh> <vert index array> smGroup:<int=0> matID:<int=1>
Value*
meshop_createPolygon_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(createPolygon, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, createPolygon);
	int nVerts=mesh->getNumVerts();
	if (!is_array(arg_list[1]))
		throw RuntimeError (GetString(IDS_CREATEPOLYGON_VERTEX_LIST_MUST_BE_ARRAY_GOT), arg_list[1]);
	Array* vertArray = (Array*)arg_list[1];
	Value* tmp;
	int smGroup = int_key_arg(smGroup, tmp, 0); 
	int matID   = int_key_arg(matID, tmp, 1); 
	if (matID < 1)
		throw RuntimeError (GetString(IDS_MATID_MUST_BE_VE_NUMBER_GOT), Integer::intern(matID));
	int degree = vertArray->size;
	int *v = new int[degree];
	for (int i = 0; i < degree; i++) {
		int vertIndex=vertArray->data[i]->to_int()-1;
		range_check(vertIndex+1, 1, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE))
		v[i]=vertIndex;
	}
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.CreatePolygon (*mesh, degree, v, smGroup, matID);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	delete [] v;
	return &ok;
}

// breakVerts <Mesh mesh> <vertlist> 
Value*
meshop_breakVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(breakVerts, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, breakVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.BreakVerts (*mesh, verts);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// makeVertsPlanar <Mesh mesh> <vertlist> 
Value*
meshop_makeVertsPlanar_cf(Value** arg_list, int arg_count)
{
	check_arg_count(makeVertsPlanar, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeVertsPlanar);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.MakeSelVertsPlanar (*mesh, verts);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// moveVertsToPlane <Mesh mesh> <vertlist> <point3 normal> <float offset> node:<node=unsupplied>
Value*
meshop_moveVertsToPlane_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveVertsToPlane, 4, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveVertsToPlane);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Point3 normal = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	float offset =  arg_list[3]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.MoveVertsToPlane (*mesh, verts, normal, offset);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// cloneVerts <Mesh mesh> <vertlist> 
Value*
meshop_cloneVerts_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(cloneVerts, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, cloneVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.CloneVerts (*mesh, verts);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// <mesh> detachVerts <Mesh mesh> <vertlist> delete:<boolean=true> asMesh:<boolean=false> 
Value* 
meshop_detachVerts_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(detachVerts, 2, count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, detachVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Value* tmp;
	BOOL del = bool_key_arg(delete, tmp, TRUE); 
	BOOL asMesh = bool_key_arg(asMesh, tmp, FALSE); 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	MeshDelta tmd;
	Mesh* newMesh = NULL;
	if (asMesh)
		newMesh = new Mesh();
	tmd.Detach (*mesh, newMesh, verts, FALSE, del, !asMesh);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	Value* res = &ok;
	if (asMesh) res = new MeshValue(newMesh,TRUE);
	return res;
}

// chamferVerts <Mesh mesh> <vertlist> <float amount> 
Value*
meshop_chamferVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(chamferVerts, 3, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, chamferVerts);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	float amount  = arg_list[2]->to_float();
	mesh->vertSel = verts;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	MeshTempData TempData (mesh);
	MeshChamferData *mcd = TempData.ChamferData();
	AdjEdgeList *ae = TempData.AdjEList();
	tmd.ChamferVertices (*mesh, verts, *mcd, ae);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	nVerts=mesh->getNumVerts();
	if (nVerts == 0) return &ok;
// possibly add controllers for new verts
	if (mdu) {
		BitArray sel (nVerts);
		float *vmp = TempData.ChamferData()->vmax.Addr(0);
		for (int i = 0; i < nVerts; i++) if (vmp[i]) sel.Set(i);
		mdu->PlugControllersSel(MAXScript_time(),sel);
	}

	if (amount<=0) return &ok;
	TempData.Invalidate (tmd.PartsChanged ()); // LAM - 8/19/03 - defect 506171
	tmd.ClearAllOps();
	tmd.InitToMesh(*mesh);
	tmd.ChamferMove (*mesh, *TempData.ChamferData(), amount, TempData.AdjEList());
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// <point3> getVert <Mesh mesh> <int vertIndex> node:<node=unsupplied>
Value*
meshop_getVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getVert, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVert);
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, mesh->getNumVerts(), GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE))
	Point3 p = mesh->verts[vertIndex]; // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
//	else world_to_current_coordsys(p);
	return new Point3Value(p);
}

// setVert <Mesh mesh> <vertlist> {<point3 pos> | <point3 pos_array>} node:<node=unsupplied>
Value*
meshop_setVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setVert, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVert);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Value* inNode = key_arg(node);

	Point3 pos;
	Array* posArray;
	Tab<Point3> thePos;
	bool usingPosArray = false;

	if (arg_list[2]->is_kind_of(class_tag(Array)))
	{
		usingPosArray = true;
		posArray = (Array*)arg_list[2];
		int nVertsSpecified = verts.NumberSet();
		if (posArray->size != nVertsSpecified)
			throw RuntimeError (GetString(IDS_VERT_AND_POS_ARRAYS_NOT_SAME_SIZE));
		thePos.SetCount(nVertsSpecified);
		for (int i = 0; i < nVertsSpecified; i++)
		{
			pos = posArray->data[i]->to_point3(); // LOCAL COORDINATES!!!
			if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(pos);
			else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(pos);
			thePos[i] = pos;
		}
		// sort thePos based on vert order in vertArray
		if (is_array(arg_list[1]))
		{
			Array* vertArray = (Array*)arg_list[1];
			Tab<IndexedPoint3SortVal> sortVals;
			sortVals.SetCount(nVertsSpecified);
			for (int i = 0; i < nVertsSpecified; i++)
			{
				sortVals[i].index = vertArray->data[i]->to_int();
				sortVals[i].val = thePos[i];
			}

			sortVals.Sort(IndexedPoint3SortCmp);
			for (int i = 0; i < nVertsSpecified; i++)
				thePos[i] = sortVals[i].val;
		}
	}
	else
	{
		pos = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(pos);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(pos);
	}

	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	for (int vertIndex = 0, i = 0; vertIndex < nVerts; vertIndex++)
		if (verts[vertIndex]) 
		{
			if (usingPosArray) pos = thePos[i++];
			tmd.Move (vertIndex, pos-mesh->verts[vertIndex]);
		}
		if (mdu) 
			GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
		else 
			tmd.Apply (*mesh);
		return &ok;
}

// moveVert <Mesh mesh> <vertlist> {<point3 offset> | <point3 offset_array>} node:<node> useSoftSel:<bool>
Value*
meshop_moveVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveVert, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveVert);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Value* inNode = key_arg(node);

	Point3 offset;
	Array* offsetArray;
	Tab<Point3> theOffset;
	bool usingOffsetArray = false;

	if (arg_list[2]->is_kind_of(class_tag(Array)))
	{
		usingOffsetArray = true;
		offsetArray = (Array*)arg_list[2];
		int nVertsSpecified = verts.NumberSet();
		if (offsetArray->size != nVertsSpecified)
			throw RuntimeError (GetString(IDS_VERT_AND_POS_ARRAYS_NOT_SAME_SIZE));
		theOffset.SetCount(nVertsSpecified);
		for (int i = 0; i < nVertsSpecified; i++)
		{
			offset = offsetArray->data[i]->to_point3(); // LOCAL COORDINATES!!!
			if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_scaleRotate(offset);
			else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_scaleRotate(offset);
			theOffset[i] = offset;
		}
		// sort thePos based on vert order in vertArray
		if (is_array(arg_list[1]))
		{
			Array* vertArray = (Array*)arg_list[1];
			Tab<IndexedPoint3SortVal> sortVals;
			sortVals.SetCount(nVertsSpecified);
			for (int i = 0; i < nVertsSpecified; i++)
			{
				sortVals[i].index = vertArray->data[i]->to_int();
				sortVals[i].val = theOffset[i];
			}

			sortVals.Sort(IndexedPoint3SortCmp);
			for (int i = 0; i < nVertsSpecified; i++)
				theOffset[i] = sortVals[i].val;
		}
	}
	else
	{
		offset = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_scaleRotate(offset);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_scaleRotate(offset);
	}

	BOOL softSelSupport = mesh->vDataSupport(0);
	Value* tmp;
	BOOL useSoftSel = bool_key_arg(useSoftSel, tmp, FALSE) && softSelSupport;
	float *softSelVal = (useSoftSel) ? mesh->vertexFloat(0) : NULL;

	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	Point3 voffset;
	for (int vertIndex = 0, i = 0; vertIndex < nVerts; vertIndex++)
		if (verts[vertIndex]) 
		{
			voffset = (usingOffsetArray) ? theOffset[i++] : offset;
			if (useSoftSel) voffset *= softSelVal[vertIndex];
			tmd.Move (vertIndex, voffset);
		}
		if (mdu) 
			GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
		else 
			tmd.Apply (*mesh);
		return &ok;
}

///////////////////////////////////////////////////////////////////////////////////////
//                           operations: face level
///////////////////////////////////////////////////////////////////////////////////////


// autoSmooth <Mesh mesh> <facelist> <float threshold>
Value*
meshop_autoSmooth_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(autoSmooth, 3, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, autoSmooth);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float thresh = DegToRad (arg_list[2]->to_float());
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
// autoSmooth uses FaceClusterList, which operates on selected faces. Need to select the faces to make this work.
// store previous face selection and restore
	BitArray oldSel (mesh->faceSel);
	mesh->faceSel = faces; // .SetAll();
	MeshDelta tmd (*mesh);
	tmd.AutoSmooth (*mesh, faces, thresh);
	mesh->faceSel = oldSel;
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// unifyNormals <Mesh mesh> <facelist>
Value*
meshop_unifyNormals_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(unifyNormals, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, unifyNormals);
	int nFaces=mesh->getNumFaces();
	BitArray whichFaces (nFaces);
	ValueToBitArrayM(arg_list[1], whichFaces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.UnifyNormals (*mesh, whichFaces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// flipNormals <Mesh mesh> <facelist>
Value*
meshop_flipNormals_cf(Value** arg_list, int arg_count)
{
	check_arg_count(flipNormals,2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, flipNormals);
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd(*mesh);
	for (int index = 0; index < nFaces; index++) {
		if (faces[index]) 
			tmd.FlipNormal (*mesh, index);
	}
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}


// divideFaceByEdges <Mesh mesh> <int faceIndex> <int edge1Index> <float edge1f> <int edge2Index> <float edge2f>
//                   fixNeighbors:<boolean=true> split:<boolean=false>
Value* 
meshop_divideFaceByEdges_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(divideFaceByEdges, 6, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideFaceByEdges);
	int faceIndex =  arg_list[1]->to_int()-1;
	int edge1Index = arg_list[2]->to_int()-1;
	float edge1f =   arg_list[3]->to_float();
	int edge2Index = arg_list[4]->to_int()-1;
	float edge2f =   arg_list[5]->to_float();
	int nFaces = mesh->getNumFaces();
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE))
	range_check(edge1Index+1, 1, 3*nFaces, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE))
	range_check(edge2Index+1, 1, 3*nFaces, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE))
	Value* tmp;
	bool fixNeighbors = bool_key_arg(fixNeighbors, tmp, TRUE) == TRUE; 
	bool split = bool_key_arg(split, tmp, FALSE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->DivideFace(faceIndex, edge1Index, edge2Index, edge1f, edge2f, fixNeighbors, split);
	if (mdu) {
		mdu->LocalDataChanged (ALL_CHANNELS);
		theHold.Release();
		HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
		MAXScript_interface->FlushUndoBuffer();
	}
	return &ok;
}

// deleteFaces <Mesh mesh> <facelist> delIsoVerts:<boolean=true>
Value*
meshop_deleteFaces_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(deleteFaces, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	Value* tmp;
	BOOL delIsoVerts = bool_key_arg(delIsoVerts, tmp, TRUE); 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.DeleteFaceSet (*mesh, faces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	if (delIsoVerts) {
		tmd.ClearAllOps();
		tmd.InitToMesh(*mesh);
		tmd.DeleteIsoVerts (*mesh);
		if (mdu) 
			GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
		else 
			tmd.Apply (*mesh);
	}
	return &ok;
}

//bevelFaces <mesh> <facelist> <float height> <float outline> dir:<{<point3 dir> | #independent | #common}=#independent> node:<node=unsupplied>
Value*
meshop_bevelFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(bevelFaces, 4, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, bevelFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float height =  arg_list[2]->to_float();
	float outline = arg_list[3]->to_float();
	Value* dir = key_arg(dir);
	if (dir == &unsupplied)
		dir = n_independent;

// build vert list corresponding to specified faces
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	for (int i = 0; i < nFaces; i++) {
		if (!faces[i]) continue;
		Face *f = &mesh->faces[i];
		for (int j = 0; j < 3; j++) verts.Set(f->v[j]);
	}

	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshTempData TempData (mesh);
	MeshDelta tmd(*mesh);

// uses FaceClusterList, which operates on selected faces. Need to select the faces to make this work.
// store previous face selection and restore
	BitArray oldSel (mesh->faceSel);
	mesh->faceSel = faces; 
	Tab<Point3> *hdir = TempData.FaceExtDir(MESH_EXTRUDE_CLUSTER);
	int extType = MESH_EXTRUDE_CLUSTER;
	if (is_point3(dir)) 
	{	Point3 pdir = dir->to_point3(); // LOCAL COORDINATES!!!
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(pdir);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(pdir);
//		else world_from_current_coordsys(pdir,NO_TRANSLATE + NO_SCALE);
		for (int i=0; i< nVerts; i++) 
//			(*hdir)[i] = pdir;
			hdir->operator[](i) = pdir;
	}
	else if (dir == n_independent) {
		hdir = TempData.FaceExtDir(MESH_EXTRUDE_LOCAL);
		extType = MESH_EXTRUDE_LOCAL;
	}
	else if (dir != n_common)
		throw RuntimeError (GetString(IDS_BEVELFACE__INVALID_DIR_ARGUMENT), dir);
	tmd.Bevel (*mesh, verts, outline, TempData.OutlineDir(extType), height, hdir);
	mesh->faceSel = oldSel;
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// makeFacesPlanar <Mesh mesh> <facelist> 
Value*
meshop_makeFacesPlanar_cf(Value** arg_list, int arg_count)
{
	check_arg_count(makeFacesPlanar, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeFacesPlanar);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);

// MakeSelFacesPlanar uses the selected faces, not the bitArray passed in. Need to select the faces to make this work.
// store previous face selection and restore
	BitArray oldSel (mesh->faceSel);
	mesh->faceSel = faces; 
	tmd.MakeSelFacesPlanar (*mesh, faces);
	mesh->faceSel = oldSel;
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// cloneFaces <Mesh mesh> <facelist> 
Value*
meshop_cloneFaces_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(cloneFaces, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, cloneFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.CloneFaces (*mesh, faces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// collapseFaces <Mesh mesh> <facelist> 
Value*
meshop_collapseFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(collapseFaces, 2, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	for (int i=0; i<nFaces; i++) {
		if (!faces[i]) continue;
		Face f = mesh->faces[i];
		for (int k=0; k<3; k++) 
			verts.Set(f.v[k]);
	}
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.WeldVertSet (*mesh, verts);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// <mesh> detachFaces <Mesh mesh> <facelist> delete:<boolean=true> asMesh:<boolean=false> 
Value* 
meshop_detachFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(detachFaces, 2, count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, detachFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	Value* tmp;
	BOOL del = bool_key_arg(delete, tmp, TRUE); 
	BOOL asMesh = bool_key_arg(asMesh, tmp, FALSE); 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	Mesh* newMesh = NULL;
	if (asMesh)
		newMesh = new Mesh();
	tmd.Detach (*mesh, newMesh, faces, TRUE, del, !asMesh);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	Value* res = &ok;
	if (asMesh) res = new MeshValue(newMesh,TRUE);
	return res;
}

// divideFace <Mesh mesh> <int faceIndex> baryCoord:<point3=unsupplied>
Value*
meshop_divideFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(divideFace, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideFace);
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, mesh->getNumFaces(), GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE))
	float* pBaryCoord = NULL;
	Value* tmp = key_arg(baryCoord);
	if (is_point3(tmp)) {
		Point3 baryCoord = tmp->to_point3();
		pBaryCoord = new float [3];
		pBaryCoord[0]=baryCoord.x;
		pBaryCoord[1]=baryCoord.y;
		pBaryCoord[2]=baryCoord.z;
	}
	else if (tmp != &unsupplied)
		throw RuntimeError (GetString(IDS_BARYCOORD_NOT_POINT3), tmp);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.DivideFace (*mesh, faceIndex, pBaryCoord);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	if (pBaryCoord) delete [] pBaryCoord;
	return &ok;
}

// divideFaces <Mesh mesh> <facelist> 
Value*
meshop_divideFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(divideFaces, 2, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.DivideFaces (*mesh, faces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// explodeFaces <Mesh mesh> <facelist> <float threshold>
Value*
meshop_explodeFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(explodeFaces, 3, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, explodeFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float threshold = arg_list[2]->to_float();
	BitArray oldSel (mesh->faceSel);
	// need to set the face selection to faceList. Restore after operation
	mesh->faceSel = faces;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.ExplodeFaces (*mesh, threshold, TRUE);
	mesh->faceSel = oldSel;
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// extrudeFaces <mesh> <facelist> <float height> <float outline> dir:<{<point3 dir> | #independent | #common}=#independent> node:<node=unsupplied>
Value*
meshop_extrudeFaces_cf(Value** arg_list, int count)
{
	// sets the face selection to faceList. Check later to see if can restore
	check_arg_count_with_keys(extrudeFaces, 4, count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, extrudeFaces);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float height =  arg_list[2]->to_float();
	float outline = arg_list[3]->to_float();
	Value* dir = key_arg(dir);
	if (dir == &unsupplied)
		dir = n_independent;
	int extType = MESH_EXTRUDE_CLUSTER;
	if (dir == n_independent) 
		extType = MESH_EXTRUDE_LOCAL;
	else if (dir != n_common && ! is_point3(dir))
		throw RuntimeError (GetString(IDS_EXTRUDEFACE__INVALID_DIR_ARGUMENT), dir);
	mesh->faceSel = faces;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.ExtrudeFaces (*mesh, faces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
// get vertices to bevel
	int nVerts=mesh->getNumVerts();
	if (nVerts == 0) return &ok;
	BitArray sel (nVerts);
	for (int i = 0; i < mesh->getNumFaces(); i++) {
		if (!mesh->faceSel[i]) continue;
		Face *f = &mesh->faces[i];
		for (int j = 0; j < 3; j++) sel.Set(f->v[j]);
	}
// possibly add controllers for new verts
	if (mdu) 
		mdu->PlugControllersSel(MAXScript_time(),sel);

// start of bevel
	if (height == 0.f && outline == 0.0f) return &ok;
	tmd.ClearAllOps();
	tmd.InitToMesh(*mesh);
	MeshTempData TempData (mesh);
	Tab<Point3> *hdir = TempData.FaceExtDir(extType);
	if (is_point3(dir)) 
	{	Point3 pdir = dir->to_point3(); // LOCAL COORDINATES!!!
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(pdir);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(pdir);
//		else world_from_current_coordsys(pdir,NO_TRANSLATE + NO_SCALE);
		for (int i=0; i< nVerts; i++) 
//			(*hdir)[i] = pdir;
			hdir->operator[](i) = pdir;
	}
	tmd.Bevel (*mesh, sel, outline, TempData.OutlineDir(extType), height, hdir);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// slice <Mesh mesh> <facelist> <point3 normal> <float offset> separate:<boolean=false> delete:<boolean=false> node:<node=unsupplied>
Value*
meshop_slice_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(slice, 4, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, slice);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	Point3 normal = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	float offset  = arg_list[3]->to_float();
	Value* tmp;
	bool separate = bool_key_arg(separate, tmp, FALSE) == TRUE; 
	bool del =      bool_key_arg(delete, tmp, FALSE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.Slice (*mesh, normal, offset, separate, del, &faces);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// edgeTessellate <Mesh mesh> <facelist> <float tension>
Value*
meshop_edgeTessellate_cf(Value** arg_list, int arg_count)
{
	check_arg_count(edgeTessellate, 3, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, edgeTessellate);
	int nFaces=mesh->getNumFaces();
	BitArray faces (nFaces);
	ValueToBitArrayM(arg_list[1], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	float tension = arg_list[2]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.EdgeTessellate (*mesh, faces, tension);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}


///////////////////////////////////////////////////////////////////////////////////////
//                           operations: edge level
///////////////////////////////////////////////////////////////////////////////////////

// autoEdge <mesh> <edgelist> <float threshold> type:<{#SetClear | #Set | #Clear}=#SetClear>
Value*
meshop_autoEdge_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(autoEdge, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, autoEdge);
	int nEdges=3*mesh->getNumFaces();
	BitArray whichEdges (nEdges);
	ValueToBitArrayM(arg_list[1], whichEdges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	float thresh = DegToRad (arg_list[2]->to_float());
	def_autoedge_types();
	int type = GetID(autoedgeTypes, elements(autoedgeTypes), key_arg_or_default(type, n_setClear));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
// uses selected edges. Need to select all the edges in  to make this work.
// store previous edge selection and restore
	BitArray oldSel (mesh->edgeSel);
	mesh->edgeSel=whichEdges;
	AdjEdgeList ae(*mesh);
	AdjFaceList af(*mesh, ae);
	mesh->edgeSel=oldSel;
	Tab<MEdge> &edges = ae.edges;
	MeshDelta tmd(*mesh);
	for (int j=0; j<edges.Count(); j++) {
		if (!edges[j].Selected(mesh->faces, whichEdges)) continue;  // skip if edge not selected on either side
		BOOL vis = (thresh==0.0f) || mesh->AngleBetweenFaces(edges[j].f[0], edges[j].f[1]) > thresh;
		if ((type == AUTOEDGE_SET) && !vis) continue;
		if ((type == AUTOEDGE_CLEAR) && vis) continue;
		if (edges[j].f[0]!=UNDEFINED) {
			int e = mesh->faces[edges[j].f[0]].GetEdgeIndex(edges[j].v[0],edges[j].v[1]);
			tmd.SetSingleEdgeVis (*mesh, edges[j].f[0]*3+e, vis, &af);
			continue;
		}
		assert (edges[j].f[1]!=UNDEFINED);
		int e = mesh->faces[edges[j].f[1]].GetEdgeIndex(edges[j].v[0],edges[j].v[1]);
		tmd.SetSingleEdgeVis (*mesh, edges[j].f[1]*3+e, vis, &af);
	}

	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// turnEdge <mesh> <int edgeIndex>
Value*
meshop_turnEdge_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(turnEdge, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, turnEdge);
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, 3*mesh->getNumFaces(), GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE)) 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();

// turning multiple edges is problematic since edges can be renumbered.
// TurnEdge maintains edge selections, so maybe store incoming selection, reset selection to whichEdges,
// then run TurnEdge on each selected edge (set edge deselected immediately before) until there are
// no more selected edges. Restore initial edge selection. How would you determine new edge number
// of edge just turned?
// TurnEdge clears pending operations, so need to apply the MeshDelta after each TurnEdge

	MeshDelta tmd(*mesh);
	tmd.TurnEdge (*mesh, edgeIndex, NULL);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// deleteEdges <Mesh mesh> <edgelist> delIsoVerts:<boolean=true>
Value*
meshop_deleteEdges_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(deleteEdges, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges(nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	Value* tmp;
	BOOL delIsoVerts = bool_key_arg(delIsoVerts, tmp, TRUE); 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.DeleteEdgeSet (*mesh, edges);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	if (delIsoVerts) {
		tmd.DeleteIsoVerts (*mesh);
		if (mdu) 
			GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
		else 
			tmd.Apply (*mesh);
	}
	return &ok;
}

// cloneEdges <Mesh mesh> <edgelist>
Value*
meshop_cloneEdges_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(cloneEdges, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, cloneEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges(nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.ExtrudeEdges (*mesh, edges);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// collapseEdges <Mesh mesh> <edgelist>
Value*
meshop_collapseEdges_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(collapseEdges, 2, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges (nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.CollapseEdges (*mesh, edges);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// divideEdge <Mesh mesh> <int edgeIndex> <float edgef> visDiag1:<boolean=false> visDiag2:<boolean=false> 
//            fixNeighbors:<boolean=true> split:<boolean=false>
Value* 
meshop_divideEdge_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(divideEdge, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideEdge);
	int edgeIndex = arg_list[1]->to_int()-1;
	float edgef   = arg_list[2]->to_float();
	range_check(edgeIndex+1, 1, 3*mesh->getNumFaces(), GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE))
	Value* tmp;
	bool visDiag1     = bool_key_arg(visDiag1, tmp,     FALSE) == TRUE; 
	bool fixNeighbors = bool_key_arg(fixNeighbors, tmp, TRUE)  == TRUE; 
	bool visDiag2     = bool_key_arg(visDiag2, tmp,     FALSE) == TRUE; 
	bool split        = bool_key_arg(split, tmp,        FALSE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.DivideEdge (*mesh, edgeIndex, edgef, NULL, visDiag1, fixNeighbors, visDiag2, split);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// divideEdges <Mesh mesh> <edgelist>
Value*
meshop_divideEdges_cf(Value** arg_list, int arg_count)
{
	check_arg_count(divideEdges, 2, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges (nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	tmd.DivideEdges (*mesh, edges);
	if (owner && owner->ClassID() == EDITTRIOBJ_CLASSID) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// extrudeEdges <mesh> <edgelist> <float height> dir:<{<point3 dir> | #independent | #common}=#independent> node:<node=unsupplied>
Value*
meshop_extrudeEdges_cf(Value** arg_list, int count)
{
	// sets the edge selection to edgeList. Check later to see if can restore
	check_arg_count_with_keys(extrudeEdges, 3, count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, extrudeEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges (nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	float height =  arg_list[2]->to_float();
	float outline = 0; // arg_list[3]->to_float();
	Value* dir = key_arg(dir); //dir: <point3> | #independent | #common
	if (dir == &unsupplied)
		dir = n_independent;
	int extType = MESH_EXTRUDE_CLUSTER;
	if (dir == n_independent) 
		extType = MESH_EXTRUDE_LOCAL;
	else if (dir != n_common && ! is_point3(dir))
		throw RuntimeError (GetString(IDS_EXTRUDEEDGE__INVALID_DIR_ARGUMENT), dir);
	mesh->edgeSel = edges;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	Tab<Point3> edir;
	tmd.ExtrudeEdges (*mesh, edges, &edir);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	int nVerts=mesh->getNumVerts();
	if (nVerts == 0) return &ok;
	BitArray sel (nVerts);
	for (int i = 0; i < mesh->getNumFaces(); i++) {			
		Face *f = &mesh->faces[i];
		for (int j = 0; j < 3; j++) {
			if (!mesh->edgeSel[i*3+j]) continue;
			sel.Set(f->v[j]);
			sel.Set(f->v[(j+1)%3]);
		}			
	}
// possibly add controllers for new verts. 
	if (mdu) mdu->PlugControllersSel(MAXScript_time(),sel);
	if (height == 0.f && outline == 0.0f) return &ok;
	tmd.ClearAllOps();
	tmd.InitToMesh(*mesh);
	if (is_point3(dir)) 
	{	Point3 pdir = dir->to_point3(); // LOCAL COORDINATES!!!
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(pdir);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(pdir);
//		else world_from_current_coordsys(pdir,NO_TRANSLATE + NO_SCALE);
		for (int e=0; e< edir.Count(); e++) 
			edir[e] = pdir;
	}
	MeshTempData TempData (mesh);
	TempData.EdgeExtDir (&edir, extType);
	tmd.Bevel (*mesh, sel, outline, NULL, height, TempData.FaceExtDir (extType));
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// chamferEdges <Mesh mesh> <edgelist> <float amount>
Value*
meshop_chamferEdges_cf(Value** arg_list, int arg_count)
{
	// ChamferEdges sets the edge selection to a modified edgeList. Check later to see if can restore
	check_arg_count(chamferEdges, 3, arg_count);
	ReferenceTarget *owner = NULL;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, chamferEdges);
	int nEdges=3*mesh->getNumFaces();
	BitArray edges (nEdges);
	ValueToBitArrayM(arg_list[1], edges, nEdges, GetString(IDS_MESH_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, mesh);
	float amount  = arg_list[2]->to_float();
	mesh->edgeSel = edges;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd;
	MeshTempData TempData (mesh);
	MeshChamferData *mcd = TempData.ChamferData();
	AdjEdgeList *ae = TempData.AdjEList();
	tmd.ChamferEdges (*mesh, edges, *mcd, ae);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	int nVerts=mesh->getNumVerts();
	if (nVerts == 0) return &ok;
// possibly add controllers for new verts
	if (mdu) {
		BitArray sel (nVerts);
		float *vmp = TempData.ChamferData()->vmax.Addr(0);
		for (int i = 0; i < nVerts; i++) if (vmp[i]) sel.Set(i);
		mdu->PlugControllersSel(MAXScript_time(),sel);
	}

	if (amount<=0) return &ok;
	TempData.Invalidate (tmd.PartsChanged ()); // LAM - 8/19/03 - defect 506171
	tmd.ClearAllOps();
	tmd.InitToMesh(*mesh);
	tmd.ChamferMove (*mesh, *TempData.ChamferData(), amount, TempData.AdjEList());
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ==================================================================================================
//           start of mapping section - channels 0-based
// ==================================================================================================

inline int GetNumMapFaces(Mesh *mesh, int mp) {	return mesh->Map(mp).getNumFaces(); }

Value*
SetNumMapVerts(Value* source, int channel, int count, bool keep, TCHAR* fn_name)
{	
// Note: can have map verts without having map faces
	ReferenceTarget *owner;
	Mesh* mesh = _get_meshForValue(source, MESH_BASE_OBJ, &owner, fn_name);
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(channel));
	if (count < 0)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), Integer::intern(count));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setNumMapVerts(channel, count, keep);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

Value*
GetNumMapVerts(Value* source, int channel, TCHAR* fn_name)
{	
// can have map verts without having map faces
	Mesh* mesh = _get_meshForValue(source, MESH_READ_ACCESS, NULL, fn_name);
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(channel));
	return Integer::intern(mesh->getNumMapVerts(channel));
}

// ============================================================================
// setNumMaps <Mesh mesh> <int count> keep:<boolean=true>
Value* 
meshop_setNumMaps_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMaps, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumMaps);
	int numMaps=arg_list[1]->to_int();
	range_check(numMaps, 0, MAX_MESHMAPS, GetString(IDS_MESH_NUMMAPS_OUT_OF_RANGE));
	Value* tmp;
	bool keep = bool_key_arg(keep, tmp, TRUE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetMapNum (numMaps, keep);
	for (int i = 0; i < numMaps; i++)
		tmd.setMapSupport (i, true);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// <int> getNumMaps <Mesh mesh>
Value* 
meshop_getNumMaps_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMaps, 1, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumMaps);
	return Integer::intern(mesh->getNumMaps());
}

// ============================================================================
// setMapSupport <Mesh mesh> <int mapChannel> <boolean support>
Value* 
meshop_setMapSupport_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setMapSupport, 3, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapSupport);
	int channel =  arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, mesh->getNumMaps()-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	bool support = arg_list[2]->to_bool() == TRUE;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.setMapSupport(channel, support);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// <boolean> getMapSupport <Mesh mesh> <int mapChannel>
Value* 
meshop_getMapSupport_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapSupport, 2, arg_count);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapSupport);
	return bool_result(mesh->mapSupport(channel));
}

// ============================================================================
// setNumMapVerts <Mesh mesh> <int mapChannel> <int count> keep:<boolean=false>
Value* 
meshop_setNumMapVerts_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMapVerts, 3, count);
	Value* tmp;
	bool keep = bool_key_arg(keep, tmp, FALSE) == TRUE; 
	return SetNumMapVerts(arg_list[0], arg_list[1]->to_int(), arg_list[2]->to_int(), keep, _T("setNumMapVerts"));
}

// ============================================================================
// <int> getNumMapVerts <Mesh mesh> <int mapChannel>
Value* 
meshop_getNumMapVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMapVerts, 2, arg_count);
	return GetNumMapVerts(arg_list[0], arg_list[1]->to_int(), _T("getNumMapVerts"));
}

// ============================================================================
// setNumMapFaces <Mesh mesh> <int mapChannel> <int count> keep:<boolean=false> keepCount:<int=0>
// dangerous with VC and TV (channel 0 and 1) since no bounds checking with those. 
Value* 
meshop_setNumMapFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMapFaces, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumMapFaces);
	int channel = arg_list[1]->to_int();
	int fcount =  arg_list[2]->to_int();
	Value* tmp;
	BOOL keep =     bool_key_arg(keep,      tmp, FALSE); 
	int keepCount = int_key_arg (keepCount, tmp, 0); 
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	if (mesh->getNumMapVerts(channel) == 0)
		throw RuntimeError (GetString(IDS_MUST_HAVE_AT_LEAST_ONE_MAP_VERTEX_TO_CREATE_MAP_FACE));
	if (fcount < 1)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), arg_list[2]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setNumMapFaces(channel, fcount, keep, keepCount);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// <int> getNumMapFaces <Mesh mesh> <int mapChannel>
Value* 
meshop_getNumMapFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMapFaces, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumMapFaces);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	return Integer::intern(GetNumMapFaces(mesh, channel));
}

// ============================================================================
// setMapVert <Mesh mesh> <int mapChannel> <int index> <Point3 uvw>
Value* 
meshop_setMapVert_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setMapVert, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapVert);
	int channel = arg_list[1]->to_int();
	int index =   arg_list[2]->to_int()-1;
	Point3 uvw =  arg_list[3]->to_point3();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	range_check(index+1, 1, mesh->getNumMapVerts(channel), GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->mapVerts(channel)[index] = uvw;
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// <point3> getMapVert <Mesh mesh> <int mapChannel> <int index>
Value* 
meshop_getMapVert_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapVert, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapVert);
	int channel = arg_list[1]->to_int();
	int index =   arg_list[2]->to_int()-1;
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	range_check(index+1, 1, mesh->getNumMapVerts(channel), GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	return new Point3Value(mesh->mapVerts(channel)[index]);
}

// ============================================================================
// setMapFace <Mesh mesh> <int mapChannel> <int index> <point3 faceVerts>
Value* 
meshop_setMapFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setMapFace, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapFace);
	int channel =      arg_list[1]->to_int();
	int index =        arg_list[2]->to_int()-1;
	Point3 faceVerts = arg_list[3]->to_point3();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	range_check(index+1, 1, GetNumMapFaces(mesh, channel), GetString(IDS_MESH_MAP_FACE_INDEX_OUT_OF_RANGE));
	IPoint3 faceVertsI ((int)faceVerts.x-1, (int)faceVerts.y-1, (int)faceVerts.z-1);
	int nMapVerts = mesh->getNumMapVerts(channel);
	range_check(faceVertsI.x+1, 1, nMapVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	range_check(faceVertsI.y+1, 1, nMapVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	range_check(faceVertsI.z+1, 1, nMapVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->mapFaces(channel)[index] = TVFace(faceVertsI.x, faceVertsI.y, faceVertsI.z);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// <point3> getMapFace <Mesh mesh> <int mapChannel> <int index>
Value* 
meshop_getMapFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapFace, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapFace);
	int channel = arg_list[1]->to_int();
	int index   = arg_list[2]->to_int()-1;
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	range_check(index+1, 1, GetNumMapFaces(mesh, channel), GetString(IDS_MESH_MAP_FACE_INDEX_OUT_OF_RANGE));
	Point3 face(0,0,0);
	face.x = mesh->mapFaces(channel)[index].t[0] + 1;
	face.y = mesh->mapFaces(channel)[index].t[1] + 1;
	face.z = mesh->mapFaces(channel)[index].t[2] + 1;
	return new Point3Value(face);
}

// ============================================================================
// makeMapPlanar <Mesh mesh> <int mapChannel>
// turns on map support for specified channel
Value* 
meshop_makeMapPlanar_cf(Value** arg_list, int arg_count)
{
	check_arg_count(makeMapPlanar, 2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeMapPlanar);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setMapSupport(channel, TRUE);
	mesh->MakeMapPlanar(channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// applyUVWMap <TriMesh mesh> <#planar | #cylindrical | #spherical | #ball | #box> | <#face>
//             utile:<float=1.0>     vtile:<float=1.0>      wtile:<float=1.0>
//             uflip:<boolean=false> vflip:<boolean=false> wflip:<boolean=false>
//             cap:<boolean=true> tm:<Matrix3=identity matrix> channel:<int=1>
// turns on map support for specified channel
Value* 
meshop_applyUVWMap_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(applyUVWMap, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, applyUVWMap);
	def_uvwmap_types();
	int uvwmapType = GetID(uvwmapTypes, elements(uvwmapTypes), arg_list[1]);
	Value* tmp;
	float uTile = float_key_arg(uTile, tmp, 1.0f);
	float vTile = float_key_arg(vTile, tmp, 1.0f);
	float wTile = float_key_arg(wTile, tmp, 1.0f);
	BOOL uFlip =  bool_key_arg( uFlip, tmp, FALSE);
	BOOL vFlip =  bool_key_arg( vFlip, tmp, FALSE);
	BOOL wFlip =  bool_key_arg( wFlip, tmp, FALSE);
	BOOL cap =    bool_key_arg( cap,   tmp, TRUE);
	tmp =  key_arg(tm);
	Matrix3 tm;
	if (tmp == &unsupplied)
		tm = Matrix3(TRUE);
	else
		tm = tmp->to_matrix3();
	int channel = int_key_arg(channel, tmp, 1); 
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setMapSupport(channel, TRUE);
	mesh->ApplyUVWMap(uvwmapType, uTile, vTile, wTile, uFlip, vFlip, wFlip, cap, tm, channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// buildMapFaces <Mesh mesh> <int mapChannel> keep:<boolean=true>
// turns on map support for specified channel
Value*
meshop_buildMapFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(buildMapFaces, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, buildMapFaces);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	Value* tmp;
	BOOL keep = bool_key_arg(keep, tmp, TRUE);
	BOOL oldState = mesh->mapSupport(channel);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setMapSupport(channel, TRUE);
	if (mesh->getNumMapVerts(channel) == 0) {
		if (!oldState) mesh->setMapSupport(channel, FALSE);
		throw RuntimeError (GetString(IDS_BUILDMAPFACES_MESH_HAS_NO_MAP_VERTICES), arg_list[1]);
	}
	// set number of map faces to match mesh face count
	mesh->setNumMapFaces(channel, mesh->getNumFaces(), keep, GetNumMapFaces(mesh, channel));
	mesh->DeleteIsoMapVerts(channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// defaultMapFaces <Mesh mesh> <int mapChannel> 
// turns on map support for specified channel
Value*
meshop_defaultMapFaces_cf(Value** arg_list, int count)
{
	check_arg_count(defaultMapFaces, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, defaultMapFaces);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setMapSupport(channel, TRUE);
	// builds the map face array and initializes it to match the mesh's face array
	int nVerts = mesh->getNumVerts();
	int nFaces = mesh->getNumFaces();
	if (mesh->getNumMapVerts(channel) != nVerts)
		mesh->setNumMapVerts(channel, nVerts, TRUE);
	Box3 bbox = mesh->getBoundingBox();
	Point3 bboxMin = bbox.Min();
	Point3 bboxSize = bbox.Width();
	int i;
	for (i = 0; i < nVerts; i++)
		mesh->mapVerts(channel)[i] = (mesh->getVert(i)-bboxMin)/bboxSize;
	// set map faces to match mesh faces
	mesh->setNumMapFaces(channel, nFaces);
	for (i = 0; i < nFaces; i++)
	{	mesh->mapFaces(channel)[i].t[0] = mesh->faces[i].v[0];
		mesh->mapFaces(channel)[i].t[1] = mesh->faces[i].v[1];
		mesh->mapFaces(channel)[i].t[2] = mesh->faces[i].v[2];
	}
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// <bitarray> getIsoMapVerts <Mesh mesh> <int mapChannel>
Value* 
meshop_getIsoMapVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getIsoMapVerts, 2, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getIsoMapVerts);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	return new BitArrayValue(mesh->GetIsoMapVerts(channel));
}

// ============================================================================
// deleteIsoMapVerts <Mesh mesh> <int mapChannel>
Value*
meshop_deleteIsoMapVerts_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(deleteIsoMapVerts,2, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteIsoMapVerts);
	int channel = arg_list[1]->to_int();
	range_check(channel, 0, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->DeleteIsoMapVerts(channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// deleteIsoMapVertsAll <Mesh mesh>
Value*
meshop_deleteIsoMapVertsAll_cf(Value** arg_list, int arg_count)
{	
	check_arg_count(deleteIsoMapVertsAll,1, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteIsoMapVertsAll);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->DeleteIsoMapVerts();
	if (mdu) mdu->LocalDataChanged (OBJ_CHANNELS);
	return &ok;
}

// ============================================================================
// <bitarray> deleteMapVertSet <Mesh mesh> <int mapChannel> <mapVertlist>
Value* 
meshop_deleteMapVertSet_cf(Value** arg_list, int count)
{
	check_arg_count(deleteMapVertSet, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteMapVertSet);
	int channel=arg_list[1]->to_int();
	range_check(channel, 0, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int nVerts = mesh->getNumMapVerts(channel);
	BitArray verts (nVerts);
	ValueToBitArray(arg_list[2], verts, nVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	one_typed_value_local(BitArrayValue* affectedFaces);
	vl.affectedFaces = new BitArrayValue (GetNumMapFaces(mesh, channel));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->DeleteMapVertSet(channel, verts, &vl.affectedFaces->bits);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return_value(vl.affectedFaces);
}

// ============================================================================
// freeMapVerts <Mesh mesh> <int mapChannel>
Value* 
meshop_freeMapVerts_cf(Value** arg_list, int count)
{
	check_arg_count(freeMapVerts, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeMapVerts);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeMapVerts(channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// freeMapFaces <Mesh mesh> <int mapChannel>
Value* 
meshop_freeMapFaces_cf(Value** arg_list, int count)
{
	check_arg_count(freeMapFaces, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeMapFaces);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeMapFaces(channel);
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// freeMapChannel <Mesh mesh> <int mapChannel>
Value* 
meshop_freeMapChannel_cf(Value** arg_list, int count)
{
	check_arg_count(freeMapChannel, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeMapChannel);
	int channel=arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->Map(channel).Clear();
	if (mdu) mdu->LocalDataChanged (MapChannelID(channel));
	return &ok;
}

// ============================================================================
// <bitarray> getMapFacesUsingMapVert <Mesh mesh> <int mapChannel> <mapVertlist>
Value*
meshop_getMapFacesUsingMapVert_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapFacesUsingMapVert, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapFacesUsingMapVert);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int nMapVerts=mesh->getNumMapVerts(channel);
	BitArray mapVerts (nMapVerts);
	ValueToBitArray(arg_list[2], mapVerts, nMapVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	int nMapFaces=GetNumMapFaces(mesh, channel);
	one_typed_value_local(BitArrayValue* mapFaces);
	vl.mapFaces = new BitArrayValue (nMapFaces);
	TVFace *mapFaceData = mesh->mapFaces(channel);
	for (int i=0; i<nMapFaces; i++) {
		for (int j=0; j<3; j++) {
			if (mapVerts[mapFaceData[i].t[j]]) {
				vl.mapFaces->bits.Set(i);
				break;
			}
		}
	}
	return_value(vl.mapFaces);
}

// ============================================================================
// <bitarray> getMapVertsUsingMapFace <Mesh mesh> <int mapChannel> <mapFacelist>
Value*
meshop_getMapVertsUsingMapFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapVertsUsingMapFace, 3, arg_count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapVertsUsingMapFace);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (!mesh->mapSupport(channel))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int nMapFaces=GetNumMapFaces(mesh, channel);
	BitArray mapFaces(nMapFaces);
	ValueToBitArray(arg_list[2], mapFaces, nMapFaces, GetString(IDS_MESH_MAP_FACE_INDEX_OUT_OF_RANGE));
	one_typed_value_local(BitArrayValue* mapVerts);
	vl.mapVerts = new BitArrayValue (mesh->getNumMapVerts(channel));
	TVFace *mapFaceData = mesh->mapFaces(channel);
	for (int fi = 0; fi < nMapFaces; fi++) {
		if (mapFaces[fi]) {
			for (int j=0; j<3; j++)
				vl.mapVerts->bits.Set(mapFaceData[fi].t[j]);
		}
	}
	return_value(vl.mapVerts);
}

// ============================================================================
// <bitarray> getVertsByColor <Mesh mesh> <color color> <float red_thresh> <float green_thresh> <float blue_thresh>  channel:<int=0>
// <bitarray> getVertsByColor <Mesh mesh> <point3 uvw> <Float u_thresh> <Float v_thresh> <Float w_thresh>  channel:<int=0>
Value*
meshop_getVertsByColor_cf(Value** arg_list, int count)
{	
	check_arg_count(getVertsByColor, 5, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsByColor);
	Point3 tst_color =  arg_list[1]->to_point3();
	Point3 tst_thresh = Point3 (arg_list[2]->to_float(), arg_list[3]->to_float(), arg_list[4]->to_float());
	if (is_color(arg_list[1])) {
		tst_color /= 255.0f;
		tst_thresh /= 255.0f;
	}		
	Value* tmp;
	int channel = int_key_arg(channel, tmp, 0); 
	if (!mesh->mapSupport(channel)) {
		if (channel == 0)
			return new BitArrayValue();
		else
			throw MAXException (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL));
	}
	one_typed_value_local(BitArrayValue* matches);
	vl.matches = new BitArrayValue (mesh->getNumVerts());
	for (int i=0; i<mesh->getNumFaces() && i<mesh->Map(channel).getNumFaces(); i++) {
		TVFace tFace=mesh->mapFaces(channel)[i];
		for (int j=0; j<3; j++) {
			int iTVert=tFace.getTVert(j);
			if (iTVert < 0 || iTVert >= mesh->Map(channel).getNumVerts()) continue;
			Point3 delta_color = mesh->mapVerts(channel)[iTVert] - tst_color;
			if (fabs(delta_color.x) > tst_thresh.x) continue;
			if (fabs(delta_color.y) > tst_thresh.y) continue;
			if (fabs(delta_color.z) > tst_thresh.z) continue;
			vl.matches->bits.Set(mesh->faces[i].v[j]);
		}
	}
	return_value(vl.matches);
}

// ============================================================================
// setFaceAlpha <Mesh mesh> <int mapChannel> <facelist> <float alpha>
Value* 
meshop_setFaceAlpha_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setFaceAlpha, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceAlpha);
	int channel = arg_list[1]->to_int();
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[2], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	float alpha =  arg_list[3]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetFaceAlpha (*mesh, faces, alpha, channel);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// setVertAlpha <Mesh mesh> <int mapChannel> <vertlist> <float alpha>
Value* 
meshop_setVertAlpha_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setVertAlpha, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertAlpha);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[2], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	float alpha =  arg_list[3]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetVertAlpha (*mesh, verts, alpha, channel);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// setFaceColor <Mesh mesh> <int mapChannel> <facelist> <color color>
Value* 
meshop_setFaceColor_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setFaceColor, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceColor);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	int nFaces=mesh->getNumFaces();
	BitArray faces(nFaces);
	ValueToBitArrayM(arg_list[2], faces, nFaces, GetString(IDS_MESH_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	Point3 color = arg_list[3]->to_point3()/255.0f;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetFaceColors (*mesh, faces, color, channel);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// ============================================================================
// setVertColor <Mesh mesh> <int mapChannel> <vertlist> <color color>
Value* 
meshop_setVertColor_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setVertColor, 4, arg_count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertColor);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[2], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	Point3 color = arg_list[3]->to_point3()/255.0f;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetVertColors (*mesh, verts, color, channel);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}


// ==================================================================================================
//           start of vertex data section - channels 1-based
// ==================================================================================================

// VertWeight

// <float> getVertWeight <Mesh mesh> <int vertIndex>
Value*
meshop_getVertWeight_cf(Value** arg_list, int count)
{	
	check_arg_count(getVertWeight, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertWeight);
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, mesh->vData[VDATA_WEIGHT].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE))
	float *vw = mesh->getVertexWeights ();
	return (Float::intern((!vw) ? 1.0f : vw[vertIndex]));
}

// setVertWeight <Mesh mesh> <vertlist> <float weight>
Value*
meshop_setVertWeight_cf(Value** arg_list, int count)
{	
	check_arg_count(setVertWeight, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertWeight);
	if (!mesh->vDataSupport(VDATA_WEIGHT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_WEIGHT+1));
	float weight=arg_list[2]->to_float();
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	if (weight < MIN_WEIGHT) weight = MIN_WEIGHT;
	else if (weight > MAX_WEIGHT) weight = MAX_WEIGHT;
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetVertWeights (*mesh, verts, weight);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
//	tmd.Apply (*mesh);	// NOTE: not using ApplyMeshDelta; no undo support.
	return &ok;
}

// resetVertWeights <Mesh mesh>
Value*
meshop_resetVertWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(resetVertWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetVertWeights);
	if (!mesh->vDataSupport(VDATA_WEIGHT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_WEIGHT+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.ResetVertWeights (*mesh);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// freeVertWeights <Mesh mesh>
Value*
meshop_freeVertWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(freeVertWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVertWeights);
	if (!mesh->vDataSupport(VDATA_WEIGHT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_WEIGHT+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeVertexWeights();
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// supportVertWeights <Mesh mesh>
Value*
meshop_supportVertWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(supportVertWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, supportVertWeights);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->SupportVertexWeights();
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// VSelectWeight

// <float> getVSelectWeight <Mesh mesh> <int vertIndex>
Value*
meshop_getVSelectWeight_cf(Value** arg_list, int count)
{	
	check_arg_count(getVSelectWeight, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVSelectWeight);
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, mesh->vData[VDATA_SELECT].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE))
	float *vsw = mesh->getVSelectionWeights ();
	return (Float::intern((!vsw) ? 0.0f : vsw[vertIndex]));
}

// setVSelectWeight <Mesh mesh> <vertlist> <float weight>
Value*
meshop_setVSelectWeight_cf(Value** arg_list, int count)
{	
	check_arg_count(setVSelectWeight, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVSelectWeight);
	if (!mesh->vDataSupport(VDATA_SELECT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_SELECT+1));
	int nVerts=mesh->vData[VDATA_SELECT].Count();
	BitArray verts (nVerts);
	ValueToBitArray(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE));
	float weight =  arg_list[2]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	float *vsw = mesh->getVSelectionWeights ();
	for (int i=0; i < nVerts; i++)
		if (verts[i]) vsw[i] = weight;
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// resetVSelectWeights <Mesh mesh>
Value*
meshop_resetVSelectWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(resetVSelectWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetVSelectWeights);
	if (!mesh->vDataSupport(VDATA_SELECT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_SELECT+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	int nVerts=mesh->vData[VDATA_SELECT].Count();
	float *vsw = mesh->getVSelectionWeights ();
	float *defSelect = (float*)VertexDataDefault(VDATA_SELECT);
	for (int i=0; i < nVerts; i++)
		vsw[i] = *defSelect;
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// freeVSelectWeights <Mesh mesh>
Value*
meshop_freeVSelectWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(freeVSelectWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVSelectWeights);
	if (!mesh->vDataSupport(VDATA_SELECT))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_SELECT+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeVSelectionWeights();
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// supportVSelectWeights <Mesh mesh>
Value*
meshop_supportVSelectWeights_cf(Value** arg_list, int count)
{	
	check_arg_count(supportVSelectWeights, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, supportVSelectWeights);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->SupportVSelectionWeights();
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// VertCorner

// <float> getVertCorner <Mesh mesh> <int vertIndex>
Value*
meshop_getVertCorner_cf(Value** arg_list, int count)
{	
	check_arg_count(getVertCorner, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertCorner);
	if (!mesh->vDataSupport(VDATA_CORNER))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_CORNER+1));
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, mesh->vData[VDATA_CORNER].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE))
	return Float::intern(mesh->vertexFloat(VDATA_CORNER)[vertIndex]);
}

// setVertCorner <Mesh mesh> <vertlist> <float weight>
Value*
meshop_setVertCorner_cf(Value** arg_list, int count)
{	
	check_arg_count(setVertCorner, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertCorner);
	if (!mesh->vDataSupport(VDATA_CORNER))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_CORNER+1));
	int nVerts=mesh->vData[VDATA_CORNER].Count();
	BitArray verts (nVerts);
	ValueToBitArray(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE));
	float corner =  arg_list[2]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetVertCorners (*mesh, verts, corner);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// resetVertCorners <Mesh mesh>
Value*
meshop_resetVertCorners_cf(Value** arg_list, int count)
{	
	check_arg_count(resetVertCorners, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetVertCorners);
	if (!mesh->vDataSupport(VDATA_CORNER))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_CORNER+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.ResetVertCorners (*mesh);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}

// freeVertCorners <Mesh mesh>
Value*
meshop_freeVertCorners_cf(Value** arg_list, int count)
{	
	check_arg_count(freeVertCorners, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVertCorners);
	if (!mesh->vDataSupport(VDATA_CORNER))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_CORNER+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeVData(VDATA_CORNER);
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// supportVertCorners <Mesh mesh>
Value*
meshop_supportVertCorners_cf(Value** arg_list, int count)
{	
	check_arg_count(supportVertCorners, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, supportVertCorners);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setVDataSupport(VDATA_CORNER);
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// VertAlpha

// <float> getVAlpha <Mesh mesh> <int vertIndex>
Value*
meshop_getVAlpha_cf(Value** arg_list, int count)
{	
	check_arg_count(getVAlpha, 2, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVAlpha);
	if (!mesh->vDataSupport(VDATA_ALPHA))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_ALPHA+1));
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, mesh->vData[VDATA_ALPHA].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE))
	return Float::intern(mesh->vertexFloat(VDATA_ALPHA)[vertIndex]);
}

// setVAlpha <Mesh mesh> <vertlist> <float alpha>
Value*
meshop_setVAlpha_cf(Value** arg_list, int count)
{	
	check_arg_count(setVAlpha, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVAlpha);
	if (!mesh->vDataSupport(VDATA_ALPHA))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_ALPHA+1));
	int nVerts=mesh->vData[VDATA_ALPHA].Count();
	BitArray verts (nVerts);
	ValueToBitArray(arg_list[1], verts, nVerts, GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE));
	float weight =  arg_list[2]->to_float();
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	float *va = mesh->vertexFloat(VDATA_ALPHA);
	for (int i=0; i < nVerts; i++)
		if (verts[i]) va[i] = weight;
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// resetVAlphas <Mesh mesh>
Value*
meshop_resetVAlphas_cf(Value** arg_list, int count)
{	
	check_arg_count(resetVAlphas, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetVAlphas);
	if (!mesh->vDataSupport(VDATA_ALPHA))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_ALPHA+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	int nVerts=mesh->vData[VDATA_ALPHA].Count();
	float *va = mesh->vertexFloat(VDATA_ALPHA);
	float *defAlpha = (float*)VertexDataDefault(VDATA_ALPHA);
	for (int i=0; i < nVerts; i++)
		va[i] = *defAlpha;
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// freeVAlphas <Mesh mesh>
Value*
meshop_freeVAlphas_cf(Value** arg_list, int count)
{	
	check_arg_count(freeVAlphas, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVAlphas);
	if (!mesh->vDataSupport(VDATA_ALPHA))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(VDATA_ALPHA+1));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeVData(VDATA_ALPHA);
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// supportVAlphas <Mesh mesh>
Value*
meshop_supportVAlphas_cf(Value** arg_list, int count)
{	
	check_arg_count(supportVAlphas, 1, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, supportVAlphas);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setVDataSupport(VDATA_ALPHA);
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// ============================================================================
// setNumVDataChannels <Mesh mesh> <int count> keep:<boolean=true>
// Note: first ten channels for Discreet's use only.
// channel 1: Soft Selection
// channel 2  Vertex weights (for NURMS MeshSmooth)
// channel 3  Vertex Alpha values
// channel 4  Cornering values for subdivision use

Value* 
meshop_setNumVDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumVDataChannels, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumVDataChannels);
	int numChannels=arg_list[1]->to_int();
	range_check(numChannels, 0, MAX_VERTDATA, GetString(IDS_MESH_NUMVERTDATA_OUT_OF_RANGE));
	Value* tmp;
	bool keep = bool_key_arg(keep, tmp, TRUE) == TRUE; 
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	MeshDelta tmd (*mesh);
	tmd.SetVDataNum(numChannels, keep);
	if (mdu) 
		GetMeshDeltaUserDataInterface(owner)->ApplyMeshDelta (tmd, mdu, MAXScript_time());
	else 
		tmd.Apply (*mesh);
	return &ok;
}


// ============================================================================
// <int> getNumVDataChannels <Mesh mesh>
Value* 
meshop_getNumVDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count(getNumVDataChannels, 1, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumVDataChannels);
	return Integer::intern(mesh->getNumVData());
}

// ============================================================================
// setVDataChannelSupport <Mesh mesh> <int vdChannel> <boolean support>
Value* 
meshop_setVDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(setVDataChannelSupport, 3, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVDataChannelSupport);
	int vdChannel=arg_list[1]->to_int()-1;
	range_check(vdChannel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->setVDataSupport(vdChannel, arg_list[2]->to_bool());
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// ============================================================================
// <boolean> getVDataChannelSupport <Mesh mesh> <int vdChannel>
Value* 
meshop_getVDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(getVDataChannelSupport, 2, count);
	int vdChannel=arg_list[1]->to_int()-1;
	range_check(vdChannel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVDataChannelSupport);
	return bool_result(mesh->vDataSupport(vdChannel));
}

// ============================================================================
// <float> getVDataValue <Mesh mesh> <int vdChannel> <int vertIndex>
Value* 
meshop_getVDataValue_cf(Value** arg_list, int count)
{
	check_arg_count(getVDataValue, 3, count);
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVDataValue);
	int vdChannel=arg_list[1]->to_int()-1;
	range_check(vdChannel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!mesh->vDataSupport(vdChannel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int index=arg_list[2]->to_int()-1;
	range_check(index+1, 1, mesh->vData[vdChannel].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE));
	return Float::intern(mesh->vertexFloat(vdChannel)[index]);
}

// ============================================================================
// setVDataValue <Mesh mesh> <int vdChannel> <vertexlist> <float value>
Value* 
meshop_setVDataValue_cf(Value** arg_list, int count)
{
	check_arg_count(setVDataValue, 4, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVDataValue);
	int vdChannel=arg_list[1]->to_int()-1;
	range_check(vdChannel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!mesh->vDataSupport(vdChannel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int nVerts=mesh->getNumVerts();
	BitArray verts (nVerts);
	ValueToBitArrayM(arg_list[2], verts, nVerts, GetString(IDS_MESH_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, mesh);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	for (int i = 0; i < nVerts; i++)
		if (verts[i]) 
			mesh->vertexFloat(vdChannel)[i]=arg_list[3]->to_float();
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}

// ============================================================================
// freeVData <Mesh mesh> <int vdChannel>
Value* 
meshop_freeVData_cf(Value** arg_list, int count)
{
	check_arg_count(freeVData, 2, count);
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVData);
	int vdChannel=arg_list[1]->to_int()-1;
	range_check(vdChannel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!mesh->vDataSupport(vdChannel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	MeshDeltaUser *mdu = (owner) ? GetMeshDeltaUserInterface(owner) : NULL;
	if (mdu && mdu->Editing()) mdu->ExitCommandModes();
	mesh->freeVData(vdChannel);
	if (mdu) mdu->LocalDataChanged (GEOM_CHANNEL);
	return &ok;
}


///////////////////////////////////////////////////////////////////////////////////////
//                           UI Parameters
///////////////////////////////////////////////////////////////////////////////////////


Value*
meshop_getUIParam_cf(Value** arg_list, int count)
{	
	// meshop.getUIParam <node> [modifier_or_index] <param>
	// only applicable modifier is edit mesh. 
	// if mesh select, can get to falloff/pinch/bubble through paramblock
	INode* node;
	MeshDeltaUser* mdu = NULL;
	Value* whichParam;

	if (count_with_keys() == 3) {
		// modifier specified, must be a number or Edit Mesh
		node = get_valid_node(arg_list[0], getUIParam);
		Object*  obj = node->GetObjectRef();
		Modifier* target_mod = NULL;
		int mod_index = 0;
		if (is_number(arg_list[1]))
			mod_index = arg_list[1]->to_int() - 1;
		else
			target_mod = arg_list[1]->to_modifier();

		// find the specified modifier
		mdu = get_MeshDeltaUser_for_mod(obj, target_mod, mod_index);
		if (mdu == NULL)
			throw RuntimeError (GetString(IDS_MOD_NO_MESH_DELTA_I) , arg_list[1]);
		if (!mdu->Editing()) 
				throw RuntimeError (GetString(IDS_MOD_NOT_ACTIVE) , arg_list[1]);
		whichParam=arg_list[2];
	}
	else {
		// no modifier, get base editable mesh
		check_arg_count(getUIParam, 2, count);
		ReferenceTarget* owner = NULL;
		Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, getUIParam);
		if (owner != NULL) mdu = GetMeshDeltaUserInterface(owner);
		if (mdu == NULL)
			throw RuntimeError (GetString(IDS_BASEOBJECT_NO_MESHDELTA_I) , arg_list[0]);
		if (!mdu->Editing()) 
				throw RuntimeError (GetString(IDS_EDITABLEMESH_NOT_ACTIVE) , arg_list[0]);
		whichParam=arg_list[1];
	}
	def_MeshDeltaUserParam_types();
	meshUIParam param = GetID(MeshDeltaUserParamTypes, elements(MeshDeltaUserParamTypes), whichParam);
	BOOL isIntParam = ((param <= EM_MESHUIPARAM_LAST_INT) || (param==MuiDeleteIsolatedVerts));
	if (isIntParam) {
		int paramVal;
		mdu->GetUIParam(param, paramVal);
		return Integer::intern(paramVal);
	}
	else {
		float paramVal;
		mdu->GetUIParam(param, paramVal);
		return Float::intern(paramVal);
	}
}

Value*
meshop_setUIParam_cf(Value** arg_list, int count)
{	
	// meshop.setUIParam <node> [modifier_or_index] <param> <val>
	// only applicable modifier is edit mesh. 
	// if mesh select, can get to falloff/pinch/bubble through paramblock
	INode* node;
	MeshDeltaUser* mdu = NULL;
	Value* whichParam;
	Value* paramVal;

	if (count_with_keys() == 4) {
		// modifier specified, must be a number or Edit Mesh
		node = get_valid_node(arg_list[0], setUIParam);
		Object*  obj = node->GetObjectRef();
		Modifier* target_mod = NULL;
		int mod_index = 0;
		if (is_number(arg_list[1]))
			mod_index = arg_list[1]->to_int() - 1;
		else
			target_mod = arg_list[1]->to_modifier();

		// find the specified modifier
		mdu = get_MeshDeltaUser_for_mod(obj, target_mod, mod_index);
		if (mdu == NULL)
			throw RuntimeError (GetString(IDS_MOD_NO_MESH_DELTA_I) , arg_list[1]);
		if (!mdu->Editing()) 
				throw RuntimeError (GetString(IDS_MOD_NOT_ACTIVE) , arg_list[1]);
		whichParam=arg_list[2];
		paramVal=arg_list[3];
	}
	else {
		// no modifier, get base editable mesh
		check_arg_count(getUIParam, 3, count);
		ReferenceTarget* owner = NULL;
		Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, setUIParam);
		if (owner != NULL) mdu = GetMeshDeltaUserInterface(owner);
		if (mdu == NULL)
			throw RuntimeError (GetString(IDS_BASEOBJECT_NO_MESHDELTA_I) , arg_list[0]);
		if (!mdu->Editing()) 
				throw RuntimeError (GetString(IDS_EDITABLEMESH_NOT_ACTIVE) , arg_list[0]);
		whichParam=arg_list[1];
		paramVal=arg_list[2];
	}
	def_MeshDeltaUserParam_types();
	meshUIParam param = GetID(MeshDeltaUserParamTypes, elements(MeshDeltaUserParamTypes), whichParam);
	BOOL isIntParam = ((param <= EM_MESHUIPARAM_LAST_INT) || (param==MuiDeleteIsolatedVerts));
	if (isIntParam) {
		if (is_bool(paramVal))
			mdu->SetUIParam(param, paramVal->to_bool());
		else
			mdu->SetUIParam(param, paramVal->to_int());
	}
	else 
		mdu->SetUIParam(param, paramVal->to_float());
	return &ok;
}


// =========================================================================================
//                                      end meshop methods
// =========================================================================================
