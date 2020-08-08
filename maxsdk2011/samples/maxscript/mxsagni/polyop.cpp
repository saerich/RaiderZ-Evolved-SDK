#include "avg_maxver.h"

#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "Arrays.h"
#include "colorval.h"
#include <maxobj.h>
#include <istdplug.h>
#include "resource.h"

#include "modstack.h"
#include "iEPoly.h"
#include "MXSAgni.h"
#include "agnidefs.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

// ============================================================================
// <BitArray> getVertSelection <Poly poly>
Value*
polyop_getVertSelection_cf(Value** arg_list, int count)
{
	check_arg_count(getVertSelection, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getVertSelection);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue ();
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (msd) {
		vl.verts->bits=msd->GetVertSel();
	}
	return_value(vl.verts);
}

// ============================================================================
// setVertSelection <Poly poly> <vertlist>
Value*
polyop_setVertSelection_cf(Value** arg_list, int count)
{
	check_arg_count(setVertSelection, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertSelection);
	int nVerts = poly->VNum();
	BitArray verts(nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	IMeshSelect* ms = (owner) ? GetMeshSelectInterface(owner) : NULL;
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (ms && msd) {
		msd->SetVertSel(verts, ms, MAXScript_time());
		ms->LocalDataChanged();
	}
	return &ok;
}

// ============================================================================
// <BitArray> getEdgeSelection <Poly poly>
Value*
polyop_getEdgeSelection_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgeSelection, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getEdgeSelection);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue ();
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (msd)
		vl.edges->bits=msd->GetEdgeSel();
	return_value(vl.edges);
}

// ============================================================================
// setEdgeSelection <Poly poly> <edgelist>
Value*
polyop_setEdgeSelection_cf(Value** arg_list, int count)
{
	check_arg_count(setEdgeSelection, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setEdgeSelection);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	IMeshSelect* ms = (owner) ? GetMeshSelectInterface(owner) : NULL;
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (ms && msd) {
		msd->SetEdgeSel(edges, ms, MAXScript_time());
		ms->LocalDataChanged();
	}
	return &ok;
}

// ============================================================================
// <BitArray> getFaceSelection <Poly poly>
Value*
polyop_getFaceSelection_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceSelection, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getFaceSelection);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue ();
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (msd) {
		vl.faces->bits=msd->GetFaceSel();
	}
	return_value(vl.faces);
}

// ============================================================================
// setFaceSelection <Poly poly> <facelist>
Value*
polyop_setFaceSelection_cf(Value** arg_list, int count)
{
	check_arg_count(setFaceSelection, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceSelection);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	IMeshSelect* ms = (owner) ? GetMeshSelectInterface(owner) : NULL;
	IMeshSelectData* msd = (owner) ? GetMeshSelectDataInterface(owner) : NULL;
	if (ms && msd) {
		msd->SetFaceSel(faces, ms, MAXScript_time());
		ms->LocalDataChanged();
	}
	return &ok;
}

// ============================================================================
// <BitArray> getDeadVerts <Poly poly>
Value*
polyop_getDeadVerts_cf(Value** arg_list, int count)
{
	check_arg_count(getDeadVerts, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getDeadVerts);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpGetVerticesByFlag(vl.verts->bits, MN_DEAD);
	}
	return_value(vl.verts);
}

// ============================================================================
// <BitArray> getDeadEdges <Poly poly>
Value*
polyop_getDeadEdges_cf(Value** arg_list, int count)
{
	check_arg_count(getDeadEdges, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getDeadEdges);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpGetEdgesByFlag(vl.edges->bits, MN_DEAD);
	}
	return_value(vl.edges);
}

// ============================================================================
// <BitArray> getDeadFaces <Poly poly>
Value*
polyop_getDeadFaces_cf(Value** arg_list, int count)
{
	check_arg_count(getDeadFaces, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getDeadFaces);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpGetFacesByFlag(vl.faces->bits, MN_DEAD);
	}
	return_value(vl.faces);
}

// ============================================================================
// <BitArray> getVertsByFlag <Poly poly> <int flag> mask:<int flag>
Value*
polyop_getVertsByFlag_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getVertsByFlag, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getVertsByFlag);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	if (epi) {
		epi->EpGetVerticesByFlag(vl.verts->bits, arg_list[1]->to_int(), mask);
	}
	return_value(vl.verts);
}

// ============================================================================
// <int> getVertFlags <Poly poly> <int vert>
Value*
polyop_getVertFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getVertFlags, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertFlags);
	int nVerts = poly->VNum();
	int vertIndex=arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE));
	return Integer::intern(poly->V(vertIndex)->ExportFlags());
}

// ============================================================================
// <BitArray> setVertFlags <Poly poly> <vertlist> <int flag> mask:<int flag> undoable:<boolean>
Value*
polyop_setVertFlags_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setVertFlags, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertFlags);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly *epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	bool undoable = bool_key_arg(undoable, tmp, FALSE) == TRUE;
	int flag = arg_list[2]->to_int();
	if (flag & (MN_SEL | MN_DEAD)) undoable = true;
	if (mask & (MN_SEL | MN_DEAD)) undoable = true;
	if (epi)
	{	
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, mask, MNM_SL_VERTEX, mask, FALSE, FALSE, undoable);
		epi->EpSetVertexFlags(verts, flag, mask, undoable);
	}
	return &ok;
}

// ============================================================================
// <BitArray> getEdgesByFlag <Poly poly> <int flag> mask:<int flag>
Value*
polyop_getEdgesByFlag_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getEdgesByFlag, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getEdgesByFlag);
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	if (epi)
		epi->EpGetEdgesByFlag(vl.edges->bits, arg_list[1]->to_int(), mask);
	return_value(vl.edges);
}

// ============================================================================
// <int> getEdgeFlags <Poly poly> <int edge>
Value*
polyop_getEdgeFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgeFlags, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgeFlags);
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	return Integer::intern(poly->E(edgeIndex)->ExportFlags());
}

// ============================================================================
// <BitArray> setEdgeFlags <Poly poly> <edgelist> <int flag> mask:<int flag> undoable:<boolean>
Value*
polyop_setEdgeFlags_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setEdgeFlags, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setEdgeFlags);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi =  (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	bool undoable = bool_key_arg(undoable, tmp, FALSE) == TRUE;
	int flag = arg_list[2]->to_int();
	if (flag & (MN_SEL | MN_DEAD)) undoable = true;
	if (mask & (MN_SEL | MN_DEAD)) undoable = true;
	if (epi)
	{	epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, mask, MNM_SL_EDGE, mask, FALSE, FALSE, undoable);
		epi->EpSetEdgeFlags(edges, flag, mask, undoable);
	}
	return &ok;
}

// ============================================================================
// <BitArray> getFacesByFlag <Poly poly> <int flag> mask:<int flag>
Value*
polyop_getFacesByFlag_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getFacesByFlag, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getFacesByFlag);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	if (epi)
		epi->EpGetFacesByFlag(vl.faces->bits, arg_list[1]->to_int(), mask);
	return_value(vl.faces);
}

// ============================================================================
// <int> getFaceFlags <Poly poly> <int edge>
Value*
polyop_getFaceFlags_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceFlags, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceFlags);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	return Integer::intern(poly->F(faceIndex)->ExportFlags());
}

// ============================================================================
// <BitArray> setFaceFlags <Poly poly> <facelist> <int flag> mask:<int flag> undoable:<boolean>
Value*
polyop_setFaceFlags_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setFaceFlags, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceFlags);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	Value* tmp = NULL;
	int mask = int_key_arg(mask, tmp, 0);
	bool undoable = bool_key_arg(undoable, tmp, FALSE) == TRUE;
	int flag = arg_list[2]->to_int();
	if (flag & (MN_SEL | MN_DEAD)) undoable = true;
	if (mask & (MN_SEL | MN_DEAD)) undoable = true;
	if (epi)
	{	epi->EpfnPropagateComponentFlags(MNM_SL_FACE, mask, MNM_SL_FACE, mask, FALSE, FALSE, undoable);
		epi->EpSetFaceFlags(faces, flag, mask, undoable);
	}
	return &ok;
}

// ============================================================================
// <int> getNumVerts <Poly poly>
Value*
polyop_getNumVerts_cf(Value** arg_list, int count)
{
	check_arg_count(getNumVerts, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumVerts);
	return Integer::intern(poly->VNum());
}

// ============================================================================
// <int> getNumEdges <Poly poly>
Value*
polyop_getNumEdges_cf(Value** arg_list, int count)
{
	check_arg_count(getNumEdges, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumEdges);
	return Integer::intern(poly->ENum());
}

// ============================================================================
// <int> getNumFaces <Poly poly>
Value*
polyop_getNumFaces_cf(Value** arg_list, int count)
{
	check_arg_count(getNumFaces, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumFaces);
	return Integer::intern(poly->FNum());
}

// ============================================================================
// <int> GetHasDeadStructs <Poly poly>
Value*
polyop_getHasDeadStructs_cf(Value** arg_list, int count)
{
	check_arg_count(getHasDeadStructs, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getHasDeadStructs);
	int res=0;
	int i;
	for (i=0; i < poly->VNum(); i++) 
		if (poly->V(i)->GetFlag(MN_DEAD)) {
			res += 1;
			break;
		}
	for (i=0; i < poly->ENum(); i++) 
		if (poly->E(i)->GetFlag(MN_DEAD)) {
			res += 2;
			break;
		}
	for (i=0; i < poly->FNum(); i++) 
		if (poly->F(i)->GetFlag(MN_DEAD)) {
			res += 4;
			break;
		}
	return Integer::intern(res);
}


// ============================================================================
// <BitArray> getHiddenVerts <Poly poly>
Value*
polyop_getHiddenVerts_cf(Value** arg_list, int count)
{
	check_arg_count(getHiddenVerts, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getHiddenVerts);
	one_typed_value_local(BitArrayValue* verts);
	vl.verts = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpGetVerticesByFlag(vl.verts->bits, MN_HIDDEN);
	return_value(vl.verts);
}

// ============================================================================
//  setHiddenVerts <Poly poly> <vertlist>
Value*
polyop_setHiddenVerts_cf(Value** arg_list, int count)
{
	check_arg_count(setHiddenVerts, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setHiddenVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnHide(MNM_SL_VERTEX, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// <BitArray> getHiddenFaces <Poly poly>
Value*
polyop_getHiddenFaces_cf(Value** arg_list, int count)
{
	check_arg_count(getHiddenFaces, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, getHiddenFaces);
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue ();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi)
		epi->EpGetFacesByFlag(vl.faces->bits, MN_HIDDEN);
	return_value(vl.faces);
}

// ============================================================================
// setHiddenFaces <Poly poly> <facelist>
Value*
polyop_setHiddenFaces_cf(Value** arg_list, int count)
{
	check_arg_count(setHiddenFaces, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setHiddenFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnHide(MNM_SL_FACE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// unHideAllFaces <Poly poly>
Value*
polyop_unHideAllFaces_cf(Value** arg_list, int count)
{
	check_arg_count(unHideAllFaces, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, unHideAllFaces);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnUnhideAll(MNM_SL_FACE);
	return &ok;
}

// ============================================================================
// unHideAllVerts <Poly poly>
Value*
polyop_unHideAllVerts_cf(Value** arg_list, int count)
{
	check_arg_count(unHideAllVerts, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, unHideAllVerts);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnUnhideAll(MNM_SL_VERTEX);
	return &ok;
}


// ============================================================================
// <BitArray> getOpenEdges <Poly poly>
Value*
polyop_getOpenEdges_cf(Value** arg_list, int count)
{
	check_arg_count(getOpenEdges, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getOpenEdges);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges= poly->ENum();  // LAM: 2/7/01 - defect 275470 fix - was using VNum
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (nEdges);
	for (int i = 0; i < nEdges; i++) {
		MNEdge* e = poly->E(i);
		if (!e->GetFlag(MN_DEAD) && (e->f1 == -1 || e->f2 == -1)) vl.edges->bits.Set(i); // LAM - 6/13/02 - defect 311156 - can e->f1 == -1?
	}
	return_value(vl.edges);
}

// no setter for OpenEdges

// ============================================================================
// <BitArray> getBorderFromEdge <Poly poly> <int edge>
Value*
polyop_getBorderFromEdge_cf(Value** arg_list, int count)
{
	check_arg_count(getBorderFromEdge, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getBorderFromEdge);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue ();
	MNEdge* e = poly->E(edgeIndex);
	if (e->GetFlag(MN_DEAD)) return &undefined;
	if (e->f2 != -1) return_value(vl.edges);  // LAM: 2/7/01 - defect 275470 fix
	vl.edges->bits.SetSize(nEdges);
	poly->BorderFromEdge(edgeIndex,vl.edges->bits);
	return_value(vl.edges);
}

// ============================================================================
// <Array> getFaceVerts <Poly poly> <int face>
Value*
polyop_getFaceVerts_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceVerts, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceVerts);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	MNFace* f = poly->F(faceIndex);
	if (f->GetFlag(MN_DEAD)) return &undefined;
	one_typed_value_local(Array* verts);
	vl.verts = new Array (f->deg);
	for (int i = 0; i < f->deg; i++)
		vl.verts->append(Integer::intern(f->vtx[i]+1));
	return_value(vl.verts);
}

// ============================================================================
// <Array> getFaceEdges <Poly poly> <int face>
Value*
polyop_getFaceEdges_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceEdges, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceEdges);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	MNFace* f = poly->F(faceIndex);
	if (f->GetFlag(MN_DEAD)) return &undefined;
	one_typed_value_local(Array* edges);
	vl.edges = new Array (f->deg);
	for (int i = 0; i < f->deg; i++)
		vl.edges->append(Integer::intern(f->edg[i]+1));
	return_value(vl.edges);
}

// ============================================================================
// <Array> getEdgeVerts <Poly poly> <int edge>
Value*
polyop_getEdgeVerts_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgeVerts, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgeVerts);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	MNEdge* e = poly->E(edgeIndex);
	if (e->GetFlag(MN_DEAD)) return &undefined;
	one_typed_value_local(Array* verts);
	vl.verts = new Array (2);
	vl.verts->append(Integer::intern(e->v1+1));
	vl.verts->append(Integer::intern(e->v2+1));
	return_value(vl.verts);
}

// ============================================================================
// <boolean> isFaceDead <Poly poly> <int face>
Value*
polyop_isFaceDead_cf(Value** arg_list, int count)
{
	check_arg_count(isFaceDead, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, isFaceDead);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	return bool_result(poly->F(faceIndex)->GetFlag(MN_DEAD));
}

// ============================================================================
// <boolean> isEdgeDead <Poly poly> <int edge>
Value*
polyop_isEdgeDead_cf(Value** arg_list, int count)
{
	check_arg_count(isEdgeDead, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, isEdgeDead);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	return bool_result(poly->E(edgeIndex)->GetFlag(MN_DEAD));
}

// ============================================================================
// <boolean> isVertDead <Poly poly> <int vert>
Value*
polyop_isVertDead_cf(Value** arg_list, int count)
{
	check_arg_count(isVertDead, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, isVertDead);
	int nVerts = poly->VNum();
	int vertIndex=arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE)) 
	return bool_result(poly->V(vertIndex)->GetFlag(MN_DEAD));
}

// ============================================================================
// <Array> getEdgeFaces <Poly poly> <int edge>
Value*
polyop_getEdgeFaces_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgeFaces, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgeFaces);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	MNEdge* e = poly->E(edgeIndex);
	if (e->GetFlag(MN_DEAD)) return &undefined;
	one_typed_value_local(Array* faces);
	vl.faces = new Array (1);
	vl.faces->append(Integer::intern(e->f1+1));
	if (e->f2 != -1) 
		vl.faces->append(Integer::intern(e->f2+1));
	return_value(vl.faces);
}

// ============================================================================
// <int> getFaceDeg <Poly poly> <int face>
Value*
polyop_getFaceDeg_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceDeg, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceDeg);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	MNFace* f = poly->F(faceIndex);
	if (f->GetFlag(MN_DEAD)) return &undefined;
	return Integer::intern(f->deg);
}

// ============================================================================
// <int> getFaceMatID <Poly poly> <int face>
Value*
polyop_getFaceMatID_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceMatID, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceMatID);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	MNFace* f = poly->F(faceIndex);
	if (f->GetFlag(MN_DEAD)) return &undefined;
	return Integer::intern(f->material+1);
}

// ============================================================================
// setFaceMatID <Poly poly> <facelist> <int MatID>
Value*
polyop_setFaceMatID_cf(Value** arg_list, int count)
{
	check_arg_count(setFaceMatID, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceMatID);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	int matID = arg_list[2]->to_int()-1;
	if (matID < 0)
		throw RuntimeError (GetString(IDS_MATID_MUST_BE_VE_NUMBER_GOT), arg_list[2]);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, 0, false);
		epi->SetMatIndex(matID, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// CollapseDeadStructs <Poly poly>
Value*
polyop_collapseDeadStructs_cf(Value** arg_list, int count)
{
	check_arg_count(collapseDeadStructs, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseDeadStructs);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->CollapseDeadStructs();
	return &ok;
}

// ============================================================================
// <point3> getVert <Poly poly> <int vertex> node:<node>
Value*
polyop_getVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getVert, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVert);
	int vertIndex =  arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, poly->VNum(), GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE))

	Point3 p = poly->V(vertIndex)->p; // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
//	else world_to_current_coordsys(p);
	return new Point3Value(p);
}

// ============================================================================
// setVert <Poly poly> <vertlist> {<point3 pos> | <point3 pos_array>} node:<node>
Value*
polyop_setVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setVert, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVert);
	int nVerts=poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	Value* inNode = key_arg(node);

	Point3 pos;
	Array* posArray = NULL;
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
	}
	else
	{
		pos = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(pos);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(pos);
	}

	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
	{
		Point3 zero = Point3(0,0,0);
		Tab<Point3> delta;
		delta.SetCount (nVerts);
		for (int i = 0, j = 0; i < nVerts; i++) 
		{
			if (verts[i]) 
			{
				if (usingPosArray) pos = thePos[j++];
				delta[i] = pos - poly->V(i)->p;
			}
			else
				delta[i]=zero;
		}
		epi->ApplyDelta(delta, epi, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// moveVert <Poly poly> <vertlist> {<point3 offset> | <point3 offset_array>} node:<node> useSoftSel:<bool>
Value*
polyop_moveVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveVert, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveVert);
	int nVerts=poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	Value* inNode = key_arg(node);

	Point3 offset;
	Array* offsetArray = NULL;
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

	BOOL softSelSupport = poly->vDataSupport(0);
	Value* tmp = NULL;
	BOOL useSoftSel = bool_key_arg(useSoftSel, tmp, FALSE) && softSelSupport;
	float *softSelVal = (useSoftSel) ? poly->vertexFloat(0) : NULL;

	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		Point3 zero = Point3(0,0,0);
		Tab<Point3> delta;
		delta.SetCount (nVerts);
		Point3 voffset;
		for (int i = 0, j = 0; i < nVerts; i++) 
		{
			if (verts[i]) 
			{
				voffset = (usingOffsetArray) ? theOffset[j++] : offset;
				if (useSoftSel) voffset *= softSelVal[i];
				delta[i] = voffset;
			}
			else
				delta[i] = zero;
		}
		epi->ApplyDelta(delta, epi, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// <boolean> isMeshFilledIn <Poly poly>
Value*
polyop_isMeshFilledIn_cf(Value** arg_list, int count)
{
	check_arg_count(isMeshFilledIn, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, isMeshFilledIn);
	return bool_result(poly->GetFlag (MN_MESH_FILLED_IN));
}

// ============================================================================
// <bitarray> getEdgesUsingVert <Poly poly> <vertlist>
Value*
polyop_getEdgesUsingVert_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgesUsingVert, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgesUsingVert);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nVerts=poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	int nEdges=poly->ENum();
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (nEdges);
	for (int i = 0; i < nEdges; i++) {
		if (poly->E(i)->GetFlag (MN_DEAD)) continue;
		if (verts[poly->E(i)->v1] || verts[poly->E(i)->v2])
			vl.edges->bits.Set(i);
	}
	return_value(vl.edges);
}

// ============================================================================
// <bitarray> getFacesUsingVert <Poly poly> <vertlist>
Value*
polyop_getFacesUsingVert_cf(Value** arg_list, int count)
{
	check_arg_count(getFacesUsingVert, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFacesUsingVert);
	int nVerts=poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	int nFaces=poly->FNum();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<nFaces; i++) {
		MNFace* f = poly->F(i);
		if (f->GetFlag(MN_DEAD)) continue;
		for (int j=0; j<f->deg; j++) {
			if (verts[f->vtx[j]]) {
				vl.faces->bits.Set(i);
				break;
			}
		}
	}
	return_value(vl.faces);
}

// ============================================================================
// <bitarray> getVertsUsingEdge <Poly poly> <edgelist>
Value*
polyop_getVertsUsingEdge_cf(Value** arg_list, int count)
{
	check_arg_count(getVertsUsingEdge, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsingEdge);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	one_typed_value_local(BitArrayValue* verts);
	int nVerts=poly->VNum();
	vl.verts = new BitArrayValue (nVerts);
	for (int i = 0; i < nEdges; i++) {
		if (edges[i]) {  // LAM: 2/7/01 - defect 275471 fix
			MNEdge* e = poly->E(i);
			if (e->GetFlag(MN_DEAD)) continue;
			vl.verts->bits.Set(e->v1);  // LAM: 2/7/01 - defect 275471 fix
			vl.verts->bits.Set(e->v2);  // LAM: 2/7/01 - defect 275471 fix
		}
	}
	return_value(vl.verts);
}

// ============================================================================
// <bitarray> getFacesUsingEdge <Poly poly> <edgelist>
Value*
polyop_getFacesUsingEdge_cf(Value** arg_list, int count)
{
	check_arg_count(getFacesUsingEdge, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFacesUsingEdge);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	int nFaces=poly->FNum();
	one_typed_value_local(BitArrayValue* faces);
	vl.faces = new BitArrayValue (nFaces);
	for (int i=0; i<nFaces; i++) {
		MNFace* f = poly->F(i);
		if (f->GetFlag(MN_DEAD)) continue;
		for (int j=0; j<f->deg; j++) {
			if (edges[f->edg[j]]) {
				vl.faces->bits.Set(i);
				break;
			}
		}
	}
	return_value(vl.faces);
}

// ============================================================================
// <bitarray> getVertsUsingFace <Poly poly> <facelist>
Value*
polyop_getVertsUsingFace_cf(Value** arg_list, int count)
{
	check_arg_count(getVertsUsingFace, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsingFace);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	one_typed_value_local(BitArrayValue* verts);
	int nVerts=poly->VNum();
	vl.verts = new BitArrayValue (nVerts);
	for (int i=0; i<nFaces; i++) {
		if (faces[i]) {
			MNFace* f = poly->F(i);
			if (f->GetFlag(MN_DEAD)) continue;
			for (int j=0; j<f->deg; j++) 
				vl.verts->bits.Set(f->vtx[j]);
		}
	}
	return_value(vl.verts);
}

// ============================================================================
// <bitarray> getEdgesUsingFace <Poly poly> <facelist>
Value*
polyop_getEdgesUsingFace_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgesUsingFace, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgesUsingFace);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	int nEdges= poly->ENum();  // LAM: 2/7/01 - defect 279694 fix - was using VNum
	one_typed_value_local(BitArrayValue* edges);
	vl.edges = new BitArrayValue (nEdges);

	for (int i=0; i<nFaces; i++) {
		if (faces[i]) {
			MNFace* f = poly->F(i);
			if (f->GetFlag(MN_DEAD)) continue;
			for (int j=0; j<f->deg; j++) 
				vl.edges->bits.Set(f->edg[j]);
		}
	}
	return_value(vl.edges);
}

// ============================================================================
// <bitarray> getElementsUsingFace <Poly poly> <facelist> fence:<facelist>
Value*
polyop_getElementsUsingFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getElementsUsingFace, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getElementsUsingFace);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	BitArray fencefaces(nFaces);
	Value* fence = key_arg(fence);
	if (fence != &unsupplied) 
		ValueToBitArrayP(fence, fencefaces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	one_typed_value_local(BitArrayValue* elementfaces);
	vl.elementfaces = new BitArrayValue (nFaces);
	BitArray fromThisFace (nFaces);
	for (int i=0; i<nFaces; i++) {
		if (faces[i]) {
			if (poly->F(i)->GetFlag(MN_DEAD)) continue;
			fromThisFace=fencefaces;
			poly->ElementFromFace (i, fromThisFace);
			vl.elementfaces->bits |= fromThisFace;
		}
	}
	return_value(vl.elementfaces);
}

// ============================================================================
// <bitarray> getVertsUsedOnlyByFaces <Poly poly> <facelist>
Value*
polyop_getVertsUsedOnlyByFaces_cf(Value** arg_list, int count)
{
	check_arg_count(getVertsUsedOnlyByFaces, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsUsedOnlyByFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	one_typed_value_local(BitArrayValue* verts);
	int nVerts=poly->VNum();
	vl.verts = new BitArrayValue (nVerts);
	int i, j;
	// set all vertices used by specified faces
	for (i=0; i<nFaces; i++) {
		if (faces[i]) {
			MNFace* f = poly->F(i);
			if (f->GetFlag(MN_DEAD)) continue;
			for (j=0; j<f->deg; j++) 
				vl.verts->bits.Set(f->vtx[j]);
		}
	}
	// clear all vertices used by unspecified faces
	for (i=0; i<nFaces; i++) {
		if (!faces[i]) {
			MNFace* f = poly->F(i);
			if (f->GetFlag(MN_DEAD)) continue;
			for (j=0; j<f->deg; j++) 
				vl.verts->bits.Clear(f->vtx[j]);
		}
	}
	return_value(vl.verts);
}

// ============================================================================
// <point3> getFaceCenter <Poly poly> <int face> node:<node>
Value* 
polyop_getFaceCenter_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getFaceCenter, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceCenter);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	Point3 p;
	poly->ComputeCenter(faceIndex, p); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
//	else world_to_current_coordsys(p);
	return new Point3Value(p);
}

// ============================================================================
// <point3> getSafeFaceCenter <Poly poly> <int face> node:<node>
Value* 
polyop_getSafeFaceCenter_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getSafeFaceCenter, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getSafeFaceCenter);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	Point3 p;
	if (poly->ComputeSafeCenter(faceIndex, p)) {  // LOCAL COORDINATES!!!
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys(p);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys(p);
//		else world_to_current_coordsys(p);
		return new Point3Value(p);
	}
	return &undefined;
}

// ============================================================================
// <point3> getFaceNormal <Poly poly> <int face> node:<node>
Value* 
polyop_getFaceNormal_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getFaceNormal, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceNormal);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	Point3 p = poly->GetFaceNormal(faceIndex, true);// LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_to_current_coordsys_rotate(p);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_to_current_coordsys_rotate(p);
//	else world_to_current_coordsys(p);
	return new Point3Value(p);
}

// ============================================================================
// <float> getFaceArea <Poly poly> <int face>
Value* 
polyop_getFaceArea_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceArea, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceArea);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
		return Float::intern(Length(poly->GetFaceNormal(faceIndex))/2.0f);
}

// ============================================================================
// attach <Poly poly> <source> targetNode:<node> sourceNode:<node>
Value*
polyop_attach_cf(Value** arg_list, int count)
{
	check_arg_count(attach, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, attach);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	INode* targetNode=NULL;
	Value* inNode = key_arg(targetNode);
	if (is_node(inNode)) targetNode = inNode->to_node();
	else if (is_node(arg_list[0])) targetNode = arg_list[0]->to_node();
	if (targetNode == NULL)
		throw RuntimeError (GetString(IDS_POLY_ATTACH_REQUIRES_TARGET_NODE), arg_list[0]);
	INode* sourceNode=NULL;
	inNode = key_arg(sourceNode);
	if (is_node(inNode)) sourceNode = inNode->to_node();
	else if (is_node(arg_list[1])) sourceNode = arg_list[1]->to_node();
	if (sourceNode == NULL)
		throw RuntimeError (GetString(IDS_POLY_ATTACH_REQUIRES_SOURCE_NODE), arg_list[1]);

	if (epi) {
		bool canUndo = true;
		epi->EpfnAttach (sourceNode, canUndo, targetNode, MAXScript_time());
		if (!canUndo) {
			theHold.Release();
			HoldSuspend hs (theHold.Holding()); // LAM - 6/13/02 - defect 306957
			MAXScript_interface->FlushUndoBuffer();
		}
	}
	return &ok;
}

// ============================================================================
// deleteVerts <Poly poly> <vertlist>
Value*
polyop_deleteVerts_cf(Value** arg_list, int count)
{
	check_arg_count(deleteVerts, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnDelete(MNM_SL_VERTEX, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// deleteFaces <Poly poly> <facelist> delIsoVerts:<boolean=true>
Value*
polyop_deleteFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(deleteFaces, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool delIsoVerts = bool_key_arg(delIsoVerts, tmp, TRUE)==TRUE; 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnDelete(MNM_SL_FACE, MN_TARG, delIsoVerts);
	}
	return &ok;
}

// ============================================================================
// deleteEdges <Poly poly> <edgelist> delIsoVerts:<boolean=true>
Value*
polyop_deleteEdges_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(deleteEdges, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteEdges);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool delIsoVerts = bool_key_arg(delIsoVerts, tmp, TRUE) == TRUE; 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnDelete(MNM_SL_EDGE, MN_TARG, delIsoVerts);
	}
	return &ok;
}

// ============================================================================
// weldVertsByThreshold <Poly poly> <vertlist>
Value*
polyop_weldVertsByThreshold_cf(Value** arg_list, int count)
{
	check_arg_count(weldVertsByThreshold, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldVertsByThreshold);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnWeldFlaggedVerts(MN_TARG);
	}
	return &ok;
}

// ============================================================================
// weldVerts <Poly poly> <int vert1> <int vert2> <point3> node:<node>
Value*
polyop_weldVerts_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(weldVerts, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldVerts);
	int nVerts = poly->VNum();
	int v1=arg_list[1]->to_int()-1;
	range_check(v1+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE)) 
	int v2=arg_list[2]->to_int()-1;
	range_check(v2+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE)) 
	Point3 weldpoint= arg_list[3]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(weldpoint);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(weldpoint);
//	else world_from_current_coordsys(weldpoint);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi)
		epi->EpfnWeldVerts(v1, v2, weldpoint);
	return &ok;
}

// ============================================================================
// weldEdgesByThreshold <Poly poly> <edgelist>
Value*
polyop_weldEdgesByThreshold_cf(Value** arg_list, int count)
{
	check_arg_count(weldEdgesByThreshold, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldEdgesByThreshold);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnWeldFlaggedEdges(MN_TARG);
	}
	return &ok;
}

// ============================================================================
// weldEdges <Poly poly> <int edge1> <int edge2>
Value*
polyop_weldEdges_cf(Value** arg_list, int count)
{
	check_arg_count(weldEdges, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldEdges);
	int nEdges = poly->ENum();
	int e1=arg_list[1]->to_int()-1;
	range_check(e1+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	int e2=arg_list[2]->to_int()-1;
	range_check(e2+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi)
		epi->EpfnWeldEdges(e1, e2);
	return &ok;
}

// ============================================================================
// <int> createVert <Poly poly> <point3 pos> node:<node>
Value*
polyop_createVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(createVert, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, createVert);
	Point3 pos = arg_list[1]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(pos);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(pos);
//	else world_from_current_coordsys(pos);
	int vertIndex = -1;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		vertIndex = epi->EpfnCreateVertex(pos, true, false);
	return (vertIndex != -1) ? Integer::intern(vertIndex+1) : &undefined;
}


// ============================================================================
// <int> createEdge <Poly poly> <int vert1> <int vert2>
Value*
polyop_createEdge_cf(Value** arg_list, int count)
{
	check_arg_count(createEdge, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, createEdge);
	int nVerts = poly->VNum();
	int v1=arg_list[1]->to_int()-1;
	range_check(v1+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE)) 
	int v2=arg_list[2]->to_int()-1;
	range_check(v2+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE)) 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	int edgeIndex = -1;
	if (epi) {
		// LAM: 2/7/01 - defect 279709. Rules:
		// 1. Verts have to be used by the same face.
		// 2. Need to make sure not already an edge between verts. 
		// 3. v1 must not equal v2
		// Case 1 handled by EpfnCreateEdge, cases 2 and 3 not.
		// Seems like these should be handled in EpfnCreateEdge
		if (v1 != v2 && poly->FindEdgeFromVertToVert(v1,v2) == -1) 
			edgeIndex = epi->EpfnCreateEdge(v1, v2);
	}
	return (edgeIndex != -1) ? Integer::intern(edgeIndex+1) : &undefined;
}

// ============================================================================
// <int> createPolygon <Poly poly> <vertex array>
Value*
polyop_createPolygon_cf(Value** arg_list, int count)
{
	check_arg_count(createPolygon, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, createPolygon);
	int nVerts=poly->VNum();
	if (!is_array(arg_list[1]))
		throw RuntimeError (GetString(IDS_CREATEPOLYGON_VERTEX_LIST_MUST_BE_ARRAY_GOT), arg_list[1]);
	Array* vertArray = (Array*)arg_list[1];
	int degree = vertArray->size;
	int *v = new int[degree];
	for (int i = 0; i < degree; i++) {
		int vertIndex=vertArray->data[i]->to_int()-1;
		range_check(vertIndex+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE))
		v[i]=vertIndex;
	}
	int faceIndex = -1;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		faceIndex = epi->EpfnCreateFace(v, degree);
	delete [] v;
	return (faceIndex != -1) ? Integer::intern(faceIndex+1) : &undefined;
}

// ============================================================================
// deleteIsoVerts <Poly poly>
Value*
polyop_deleteIsoVerts_cf(Value** arg_list, int count)
{
	check_arg_count(deleteIsoVerts, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, deleteIsoVerts);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnDeleteIsoVerts();
	return &ok;
}

// ============================================================================
// autoSmooth <Poly poly>
Value*
polyop_autoSmooth_cf(Value** arg_list, int count)
{
	check_arg_count(autoSmooth, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, autoSmooth);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnAutoSmooth(MAXScript_time());
	return &ok;
}

// ============================================================================
// flipNormals <Poly poly> <facelist>
Value*
polyop_flipNormals_cf(Value** arg_list, int count)
{
	check_arg_count(flipNormals, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, flipNormals);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnFlipNormals(MN_TARG);
	}
	return &ok;
}

// ============================================================================
// retriangulate <Poly poly> <facelist>
Value*
polyop_retriangulate_cf(Value** arg_list, int count)
{
	check_arg_count(retriangulate, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, retriangulate);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnRetriangulate(MN_TARG);
	}
	return &ok;
}

// ============================================================================
// setDiagonal <Poly poly> <int face> <int face_vert1> <int face_vert2>
Value*
polyop_setDiagonal_cf(Value** arg_list, int count)
{
	check_arg_count(setDiagonal, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setDiagonal);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	MNFace* f = poly->F(faceIndex);
	int nVerts = f->deg;
	int v1=arg_list[2]->to_int()-1;
	range_check(v1+1, 1, nVerts, GetString(IDS_POLY_FACE_VERTEX_INDEX_OUT_OF_RANGE)) 
	int v2=arg_list[3]->to_int()-1;
	range_check(v2+1, 1, nVerts, GetString(IDS_POLY_FACE_VERTEX_INDEX_OUT_OF_RANGE)) 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnSetDiagonal(faceIndex, v1, v2);
	return &ok;
}

// ============================================================================
// forceSubdivision <Poly poly>
Value*
polyop_forceSubdivision_cf(Value** arg_list, int count)
{
	check_arg_count(forceSubdivision, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, forceSubdivision);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) 
		epi->EpfnForceSubdivision();
	return &ok;
}

// ============================================================================
// meshSmoothByVert <Poly poly> <vertlist>
Value*
polyop_meshSmoothByVert_cf(Value** arg_list, int count)
{
	check_arg_count(meshSmoothByVert, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, meshSmoothByVert);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnMeshSmooth (MNM_SL_VERTEX, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// meshSmoothByFace <Poly poly> <facelist>
Value*
polyop_meshSmoothByFace_cf(Value** arg_list, int count)
{
	check_arg_count(meshSmoothByFace, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, meshSmoothByFace);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnMeshSmooth (MNM_SL_FACE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// meshSmoothByEdge <Poly poly> <edgelist>
Value*
polyop_meshSmoothByEdge_cf(Value** arg_list, int count)
{
	check_arg_count(meshSmoothByEdge, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, meshSmoothByEdge);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnMeshSmooth (MNM_SL_EDGE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// setFaceSmoothGroup <Poly poly> <facelist> <int smoothing_group> add:<boolean>
Value*
polyop_setFaceSmoothGroup_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setFaceSmoothGroup, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceSmoothGroup);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool add = bool_key_arg(add, tmp, FALSE) == TRUE;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	int smg = arg_list[2]->to_int();
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		DWORD bitmask= (add) ? smg : 0xFFFFFFFF;
		epi->SetSmoothBits (smg, bitmask, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// <int> getFaceSmoothGroup <Poly poly> <int face>
Value*
polyop_getFaceSmoothGroup_cf(Value** arg_list, int count)
{
	check_arg_count(getFaceSmoothGroup, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getFaceSmoothGroup);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE));
	return Integer::intern(poly->F(faceIndex)->smGroup);
}

// ============================================================================
// breakVerts <Poly poly> <vertlist>
Value*
polyop_breakVerts_cf(Value** arg_list, int count)
{
	check_arg_count(breakVerts, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, breakVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnBreakVerts (MN_TARG);
	}
	return &ok;
}

// ============================================================================
// <int> divideEdge <Poly poly> <int edge> <float fraction>
Value*
polyop_divideEdge_cf(Value** arg_list, int count)
{
	check_arg_count(divideEdge, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideEdge);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	int vertIndex = -1;
	if (epi) 
		vertIndex = epi->EpfnDivideEdge(edgeIndex, arg_list[2]->to_float(), false);
	return (vertIndex != -1) ? Integer::intern(vertIndex+1) : &undefined;
}

// ============================================================================
// <int> divideFace <Poly poly> <int face> <point3 pos> node:<node>
Value*
polyop_divideFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(divideFace, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, divideFace);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) return &undefined;
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}
	int vertIndex = -1;
	if (epi) {
		Point3 pos =  arg_list[2]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
		Value* inNode = key_arg(node);
		if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys(pos);
		else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys(pos);
//		else world_from_current_coordsys(pos);
		Tab<float> bary;
		poly->FacePointBary (faceIndex, pos, bary);
		vertIndex = epi->EpfnDivideFace(faceIndex, bary, false);
	}
	return (vertIndex != -1) ? Integer::intern(vertIndex+1) : &undefined;
}

// ============================================================================
// collapseVerts <Poly poly> <vertlist>
Value*
polyop_collapseVerts_cf(Value** arg_list, int count)
{
	check_arg_count(collapseVerts, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnCollapse(MNM_SL_VERTEX, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// collapseFaces <Poly poly> <facelist>
Value*
polyop_collapseFaces_cf(Value** arg_list, int count)
{
	check_arg_count(collapseFaces, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnCollapse(MNM_SL_FACE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// collapseEdges <Poly poly> <edgelist>
Value*
polyop_collapseEdges_cf(Value** arg_list, int count)
{
	check_arg_count(collapseEdges, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, collapseEdges);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnCollapse(MNM_SL_EDGE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// splitEdges <Poly poly> <edgelist>
Value*
polyop_splitEdges_cf(Value** arg_list, int count)
{
	check_arg_count(splitEdges, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, splitEdges);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnSplitEdges(MN_TARG);
	}
	return &ok;
}

// ============================================================================
// <int> propagateFlags <Poly poly> <toSOLevel> <toFlag_int> <fromSOLevel> <fromFlag_int> ampersand:<boolean=false> 
//    set:<boolean=true> undoable:<boolean=true> 
//  toSOLevel/fromSOLevel = #object | #vertex | #edge | #face
Value*
polyop_propagateFlags_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(propagateFlags, 5, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, propagateFlags);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	def_poly_so_types();
	int slTo = GetID(polysoTypes, elements(polysoTypes), arg_list[1]);
	int flTo = arg_list[2]->to_int();
	int slFrom = GetID(polysoTypes, elements(polysoTypes), arg_list[3]);
	int flFrom = arg_list[4]->to_int();
	Value* tmp = NULL;
	bool ampersand = bool_key_arg(ampersand, tmp, FALSE) == TRUE;
	bool set = bool_key_arg(set, tmp, TRUE) == TRUE;
	bool undoable = bool_key_arg(undoable, tmp, TRUE) == TRUE;
	if (flTo & (MN_SEL | MN_DEAD)) undoable = true;
	if (epi) {
		int res=epi->EpfnPropagateComponentFlags(slTo, flTo, slFrom, flFrom, ampersand, set, undoable);
		return Integer::intern(res);
	}
	return &undefined;
}

// ============================================================================
// tessellateByVert <Poly poly> <vertlist>
Value*
polyop_tessellateByVert_cf(Value** arg_list, int count)
{
	check_arg_count(tessellateByVert, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, tessellateByVert);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnTessellate (MNM_SL_VERTEX, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// tessellateByFace <Poly poly> <facelist>
Value*
polyop_tessellateByFace_cf(Value** arg_list, int count)
{
	check_arg_count(tessellateByFace, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, tessellateByFace);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnTessellate (MNM_SL_FACE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// tessellateByEdge <Poly poly> <edgelist>
Value*
polyop_tessellateByEdge_cf(Value** arg_list, int count)
{
	check_arg_count(tessellateByEdge, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, tessellateByEdge);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnTessellate (MNM_SL_EDGE, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// detachFaces <Poly poly> <facelist> delete:<boolean=true> asNode:<boolean=false> name:<string="Object01"> node:<node=unsupplied>
Value* 
polyop_detachFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(detachFaces, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, detachFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool del = bool_key_arg(delete, tmp, TRUE) == TRUE; 
	BOOL asNode = bool_key_arg(asNode, tmp, FALSE); 
	TSTR name = _T("Object01");
	tmp = key_arg(name);
	if (tmp != &unsupplied)
		name = tmp->to_string();
	INode* myNode=NULL;
	Value* inNode = key_arg(node);
	if (is_node(inNode)) myNode = inNode->to_node();
	else if (is_node(arg_list[0])) myNode = arg_list[0]->to_node();
	if (asNode && (myNode == NULL))
		throw RuntimeError (GetString(IDS_POLY_DETACH_TO_NODE_REQUIRES_NODE));
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		if (asNode)
			res = epi->EpfnDetachToObject (name, MNM_SL_FACE, MN_TARG, !del, myNode, MAXScript_time());
		else
			res = epi->EpfnDetachToElement (MNM_SL_FACE, MN_TARG, !del);
	}
	return bool_result(res);
}

// ============================================================================
// <boolean> detachEdges <Poly poly> <edgelist> delete:<boolean=true> asNode:<boolean=false> name:<string="Object01"> node:<node=unsupplied>
Value* 
polyop_detachEdges_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(detachEdges, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, detachEdges);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool del = bool_key_arg(delete, tmp, TRUE) == TRUE; 
	BOOL asNode = bool_key_arg(asNode, tmp, FALSE); 
	TSTR name = _T("Object01");
	tmp = key_arg(name);
	if (tmp != &unsupplied)
		name = tmp->to_string();
	INode* myNode=NULL;
	Value* inNode = key_arg(node);
	if (is_node(inNode)) myNode = inNode->to_node();
	else if (is_node(arg_list[0])) myNode = arg_list[0]->to_node();
	if (asNode && (myNode == NULL))
		throw RuntimeError (GetString(IDS_POLY_DETACH_TO_NODE_REQUIRES_NODE));
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		if (asNode)
			res = epi->EpfnDetachToObject (name, MNM_SL_EDGE, MN_TARG, !del, myNode, MAXScript_time());
		else
			res = epi->EpfnDetachToElement (MNM_SL_EDGE, MN_TARG, !del);
	}
	return bool_result(res);
}

// ============================================================================
// <boolean> detachVerts <Poly poly> <vertlist> delete:<boolean=true> asNode:<boolean=false> name:<string="Object01"> node:<node=unsupplied>
Value* 
polyop_detachVerts_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(detachVerts, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, detachVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	Value* tmp = NULL;
	bool del = bool_key_arg(delete, tmp, TRUE) == TRUE; 
	BOOL asNode = bool_key_arg(asNode, tmp, FALSE); 
	TSTR name = _T("Object01");
	tmp = key_arg(name);
	if (tmp != &unsupplied)
		name = tmp->to_string();
	INode* myNode=NULL;
	Value* inNode = key_arg(node);
	if (is_node(inNode)) myNode = inNode->to_node();
	else if (is_node(arg_list[0])) myNode = arg_list[0]->to_node();
	if (asNode && (myNode == NULL))
		throw RuntimeError (GetString(IDS_POLY_DETACH_TO_NODE_REQUIRES_NODE));
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		if (asNode)
			res = epi->EpfnDetachToObject (name, MNM_SL_VERTEX, MN_TARG, !del, myNode, MAXScript_time());
		else
			res = epi->EpfnDetachToElement (MNM_SL_VERTEX, MN_TARG, !del);
	}
	return bool_result(res);
}

// ============================================================================
// fillInMesh <Poly poly>
Value*
polyop_fillInMesh_cf(Value** arg_list, int count)
{
	check_arg_count(fillInMesh, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, NULL, fillInMesh);
	if (!poly->GetFlag (MN_MESH_FILLED_IN)) {
		poly->FillInMesh ();
	}
	return &ok;
}

// ============================================================================
// resetSlicePlane <Poly poly>
Value*
polyop_resetSlicePlane_cf(Value** arg_list, int count)
{
	check_arg_count(resetSlicePlane, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetSlicePlane);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->EpResetSlicePlane();
	return &ok;
}

// ============================================================================
// <ray> getSlicePlane <Poly poly> size:<&size_float_var> node:<node>
Value*
polyop_getSlicePlane_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(getSlicePlane, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, getSlicePlane);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	Value* sizeValue = key_arg(size);
	Thunk* sizeThunk = NULL;
	if (sizeValue != &unsupplied) 
		sizeThunk = sizeValue->to_thunk();
	one_typed_value_local(RayValue* res);
	vl.res = new RayValue (Point3::Origin, Point3::Origin);
	float planeSize=0;
	if (epi) 
		epi->EpGetSlicePlane(vl.res->r.dir, vl.res->r.p, &planeSize); // LOCAL COORDINATES!!!
	if (sizeThunk)
		sizeThunk->assign(Float::intern(planeSize));
	Value* inNode = key_arg(node);
	if (is_node(inNode)) {
		((MAXNode*)inNode)->object_to_current_coordsys(vl.res->r.p);
		((MAXNode*)inNode)->object_to_current_coordsys_rotate(vl.res->r.dir);
	}
	else if (is_node(arg_list[0])) {
		((MAXNode*)arg_list[0])->object_to_current_coordsys(vl.res->r.p);
		((MAXNode*)arg_list[0])->object_to_current_coordsys_rotate(vl.res->r.dir);
	}
//	else {
//		world_to_current_coordsys(vl.res->r.p);
//		world_to_current_coordsys_rotate(vl.res->r.dir);
//	}
	return_value(vl.res);
}

// ============================================================================
// setSlicePlane <Poly poly> <ray plane_and_dir> <float size> node:<node>
Value*
polyop_setSlicePlane_cf(Value** arg_list, int count)
{
	check_arg_count(setSlicePlane, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setSlicePlane);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	Point3 planeNormal = arg_list[1]->to_ray().dir;
	Point3 planeCenter = arg_list[1]->to_ray().p;
	float planeSize = arg_list[2]->to_float();

	Value* inNode = key_arg(node);
	if (is_node(inNode)) {
		((MAXNode*)inNode)->object_from_current_coordsys(planeCenter);
		((MAXNode*)inNode)->object_from_current_coordsys_rotate(planeNormal);
	}
	else if (is_node(arg_list[0])) {
		((MAXNode*)arg_list[0])->object_from_current_coordsys(planeCenter);
		((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(planeNormal);
	}
//	else {
//		world_from_current_coordsys(planeCenter);
//		world_from_current_coordsys(planeNormal,NO_TRANSLATE + NO_SCALE);
//	}

	if (epi) 
		epi->EpSetSlicePlane(planeNormal, planeCenter, planeSize);
	return &ok;
}

// ============================================================================
// <boolean> slice <Poly poly> <facelist> <ray plane_and_dir> node:<node>
Value*
polyop_slice_cf(Value** arg_list, int count)
{
	check_arg_count(slice, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, slice);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	Point3 planeNormal = arg_list[2]->to_ray().dir;
	Point3 planeCenter = arg_list[2]->to_ray().p;

	Value* inNode = key_arg(node);
	if (is_node(inNode)) {
		((MAXNode*)inNode)->object_from_current_coordsys(planeCenter);
		((MAXNode*)inNode)->object_from_current_coordsys_rotate(planeNormal);
	}
	else if (is_node(arg_list[0])) {
		((MAXNode*)arg_list[0])->object_from_current_coordsys(planeCenter);
		((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(planeNormal);
	}
//	else {
//		world_from_current_coordsys(planeCenter);
//		world_from_current_coordsys(planeNormal,NO_TRANSLATE + NO_SCALE);
//	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		res = epi->EpfnSlice(planeNormal, planeCenter, true, MN_TARG);
	}
	return bool_result(res);
}

// ============================================================================
// <boolean> inSlicePlaneMode <Poly poly>
Value*
polyop_inSlicePlaneMode_cf(Value** arg_list, int count)
{
	check_arg_count(inSlicePlaneMode, 1, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, inSlicePlaneMode);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		return bool_result(epi->EpfnInSlicePlaneMode());
	return &undefined;
}


// ============================================================================
// <int> cutVert <Poly poly> <int_start_vert> <point3_destination> <point3_projdir> node:<node=unsupplied>
Value*
polyop_cutVert_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(cutVert, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, cutVert);
	int nVerts = poly->VNum();
	int vertIndex=arg_list[1]->to_int()-1;
	range_check(vertIndex+1, 1, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE));
	Point3 dest =  arg_list[2]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Point3 dir =   arg_list[3]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!

	Value* inNode = key_arg(node);
	if (is_node(inNode)) {
		((MAXNode*)inNode)->object_from_current_coordsys(dest);
		((MAXNode*)inNode)->object_from_current_coordsys_rotate(dir);
	}
	else if (is_node(arg_list[0])) {
		((MAXNode*)arg_list[0])->object_from_current_coordsys(dest);
		((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(dir);
	}
//	else {
//		world_from_current_coordsys(dest);
//		world_from_current_coordsys(dir,NO_TRANSLATE + NO_SCALE);
//	}

	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	int res=-1;
	if (epi) 
		res = epi->EpfnCutVertex(vertIndex, dest, dir);
	if (res == -1) return &undefined;
	return Integer::intern(res+1);
}

// ============================================================================
// <int> cutFace <Poly poly> <int face> <point3 start> <point3 dest> <point3 projdir> node:<node=unsupplied>
Value*
polyop_cutFace_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(cutFace, 5, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, cutFace);
	int nFaces = poly->FNum();
	int faceIndex =  arg_list[1]->to_int()-1;
	range_check(faceIndex+1, 1, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE))
	Point3 start = arg_list[2]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Point3 dest =  arg_list[3]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Point3 dir =   arg_list[4]->to_point3(); // CONVERT TO LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) {
		((MAXNode*)inNode)->object_from_current_coordsys(start);
		((MAXNode*)inNode)->object_from_current_coordsys(dest);
		((MAXNode*)inNode)->object_from_current_coordsys_rotate(dir);
	}
	else if (is_node(arg_list[0])) {
		((MAXNode*)arg_list[0])->object_from_current_coordsys(start);
		((MAXNode*)arg_list[0])->object_from_current_coordsys(dest);
		((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(dir);
	}
//	else {
//		world_from_current_coordsys(start);
//		world_from_current_coordsys(dest);
//		world_from_current_coordsys(dir,NO_TRANSLATE + NO_SCALE);
//	}
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	int res=-1;
	if (epi) 
		res = epi->EpfnCutFace(faceIndex, start, dest, dir);
	return Integer::intern(res+1);
}

// ============================================================================
// <int> cutEdge <Poly poly> <int edge1> <float prop1> <int edge2> <float prop2> <point3_projdir> node:<node=unsupplied>
Value*
polyop_cutEdge_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(cutEdge, 6, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, cutEdge);
	int edge1Index = arg_list[1]->to_int()-1;
	float prop1 =    arg_list[2]->to_float();
	int edge2Index = arg_list[3]->to_int()-1;
	float prop2 =    arg_list[4]->to_float();
	int nEdges = poly->ENum();
	range_check(edge1Index+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	range_check(edge2Index+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	Point3 dir =  arg_list[5]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(dir);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(dir);
//	else world_from_current_coordsys(dir,NO_TRANSLATE + NO_SCALE);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	int res=-1;
	if (epi) 
		res = epi->EpfnCutEdge(edge1Index, prop1, edge2Index, prop2, dir);
	return Integer::intern(res+1);
}

// ============================================================================
// <boolean> capHolesByVert <Poly poly> <vertlist>
Value*
polyop_capHolesByVert_cf(Value** arg_list, int count)
{
	check_arg_count(capHolesByVert, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, capHolesByVert);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		res = epi->EpfnCapHoles (MNM_SL_VERTEX, MN_TARG);
	}
	return bool_result(res);
}

// ============================================================================
// <boolean> capHolesByFace <Poly poly> <facelist>
Value*
polyop_capHolesByFace_cf(Value** arg_list, int count)
{
	check_arg_count(capHolesByFace, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, capHolesByFace);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		res = epi->EpfnCapHoles (MNM_SL_FACE, MN_TARG);
	}
	return bool_result(res);
}

// ============================================================================
// <boolean> capHolesByEdge <Poly poly> <edgelist>
Value*
polyop_capHolesByEdge_cf(Value** arg_list, int count)
{
	check_arg_count(capHolesByEdge, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, capHolesByEdge);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	bool res = false;
	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		res = epi->EpfnCapHoles (MNM_SL_EDGE, MN_TARG);
	}
	return bool_result(res);
}

// ============================================================================
// makeVertsPlanar <Poly poly> <vertlist>
Value*
polyop_makeVertsPlanar_cf(Value** arg_list, int count)
{
	check_arg_count(makeVertsPlanar, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeVertsPlanar);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnMakePlanar (MNM_SL_VERTEX, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// moveVertsToPlane <Poly poly> <vertlist> <Point3 planeNormal> <float planeOffset> node:<node>
Value*
polyop_moveVertsToPlane_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveVertsToPlane, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveVertsToPlane);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	Point3 normal = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	float offset =  arg_list[3]->to_float();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->EpfnMoveToPlane (normal, offset, MNM_SL_VERTEX, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// makeEdgesPlanar <Poly poly> <edgelist>
Value*
polyop_makeEdgesPlanar_cf(Value** arg_list, int count)
{
	check_arg_count(makeEdgesPlanar, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeEdgesPlanar);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnMakePlanar (MNM_SL_EDGE, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// moveEdgesToPlane <Poly poly> <edgelist> <Point3 planeNormal> <float planeOffset> node:<node=unsupplied>
Value*
polyop_moveEdgesToPlane_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveEdgesToPlane, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveEdgesToPlane);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	Point3 normal = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	float offset =  arg_list[3]->to_float();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnMoveToPlane (normal, offset, MNM_SL_EDGE, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// makeFacesPlanar <Poly poly> <facelist>
Value*
polyop_makeFacesPlanar_cf(Value** arg_list, int count)
{
	check_arg_count(makeFacesPlanar, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, makeFacesPlanar);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnMakePlanar (MNM_SL_FACE, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// moveFacesToPlane <Poly poly> <facelist> <Point3 planeNormal> <float planeOffset> node:<node=unsupplied>
Value*
polyop_moveFacesToPlane_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(moveFacesToPlane, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, moveFacesToPlane);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	Point3 normal = arg_list[2]->to_point3(); // LOCAL COORDINATES!!!
	Value* inNode = key_arg(node);
	if (is_node(inNode)) ((MAXNode*)inNode)->object_from_current_coordsys_rotate(normal);
	else if (is_node(arg_list[0])) ((MAXNode*)arg_list[0])->object_from_current_coordsys_rotate(normal);
//	else world_from_current_coordsys(normal,NO_TRANSLATE + NO_SCALE);
	float offset =  arg_list[3]->to_float();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnMoveToPlane (normal, offset, MNM_SL_FACE, MN_TARG, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// createShape <Poly poly> <edgelist> smooth:<boolean=false> name:<string="Shape01"> node:<node=unsupplied>
Value*
polyop_createShape_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(createShape, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, &owner, createShape);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	TSTR name = _T("Shape01");
	Value* tmp = NULL;
	bool smooth = bool_key_arg(smooth, tmp, FALSE) == TRUE; 
	tmp = key_arg(name);
	if (tmp != &unsupplied)
		name = tmp->to_string();
	INode* myNode = NULL;
	Value* inNode = key_arg(node);
	if (is_node(inNode)) myNode = inNode->to_node();
	else if (is_node(arg_list[0])) myNode = arg_list[0]->to_node();
	if (myNode == NULL)
		throw RuntimeError (GetString(IDS_POLY_CREATESHAPE_REQUIRES_NODE), arg_list[0]);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->EpfnCreateShape (name, smooth, myNode, MN_TARG);
	}
	return &ok;
}

// ============================================================================
// <boolean> getEdgeVis <Poly poly> <int edge>
Value*
polyop_getEdgeVis_cf(Value** arg_list, int count)
{
	check_arg_count(getEdgeVis, 2, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEdgeVis);
	int nEdges = poly->ENum();
	int edgeIndex=arg_list[1]->to_int()-1;
	range_check(edgeIndex+1, 1, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE)) 
	return bool_result(!poly->E(edgeIndex)->GetFlag (MN_EDGE_INVIS));
}

// ============================================================================
// setEdgeVis <Poly poly> <edgelist> <boolean>
Value*
polyop_setEdgeVis_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setEdgeVis, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setEdgeVis);
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	BOOL vis = arg_list[2]->to_bool();
	for (int i = 0; i < nEdges; i++)
	{
		if (edges[i]) poly->SetEdgeVis(i,vis);
	}
	if (epi)
		epi->LocalDataChanged (PART_DISPLAY);
	return &ok;
}

// ============================================================================
// extrudeFaces <Poly poly> <facelist> <float amount>
Value*
polyop_extrudeFaces_cf(Value** arg_list, int count)
{
	check_arg_count(extrudeFaces, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, extrudeFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->EpfnExtrudeFaces (arg_list[2]->to_float(), MN_TARG, MAXScript_time());
//		epi->EpSetFaceFlags(faces, MN_SEL, true);
//		epi->LocalDataChanged(PART_TOPO|PART_SELECT); // needed to force invalidation of temp data
//		epi->EpfnExtrudeFaces (arg_list[2]->to_float(), MN_SEL, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// bevelFaces <Poly poly> <facelist> <float height> <float outline>
Value*
polyop_bevelFaces_cf(Value** arg_list, int count)
{
	check_arg_count(bevelFaces, 4, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, bevelFaces);
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[1], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_SEL, MNM_SL_FACE, MN_SEL, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_SEL, true);
		epi->LocalDataChanged(PART_TOPO|PART_SELECT); // needed to force invalidation of temp data
		epi->EpfnBevelFaces (arg_list[2]->to_float(), arg_list[3]->to_float(), MN_SEL, MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// chamferVerts <Poly poly> <vertlist> <float amount> <bool amount>
Value*
polyop_chamferVerts_cf(Value** arg_list, int count)
{
	BOOL l_bopen	= false;
	bool l_open		= false;

	if ( count == 3 )
	{
		// old behaviour 
		check_arg_count(chamferVerts, 3, count);
	}
	else 
	{
		// new behaviour with open flag passed in 
		check_arg_count(chamferVerts, 4, count);
		l_bopen = arg_list[3]->to_bool();
	}

	l_open = l_bopen ? true:false;

	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, chamferVerts);
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[1], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_SEL, FALSE, FALSE, TRUE);
		epi->EpSetVertexFlags(verts, MN_SEL, true);
		epi->LocalDataChanged(PART_TOPO|PART_SELECT); // needed to force invalidation of temp data
		epi->EpfnChamferVerticesOpen(arg_list[2]->to_float(),l_open,  MAXScript_time());
	}
	return &ok;
}

// ============================================================================
// chamferEdges <Poly poly> <edgelist> <float amount> [bool amount]
Value*
polyop_chamferEdges_cf(Value** arg_list, int count)
{
	BOOL l_bopen = false;
	bool l_open = false;

	if ( count == 3 )
	{
		// old behavior
		check_arg_count(chamferEdges, 3, count);
	}
	else 
	{
		// new behaviour with open flag passed in 
		check_arg_count(chamferEdges, 4, count);
		l_bopen = arg_list[3]->to_bool();
	}

	l_open = l_bopen ? true:false;

	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, chamferEdges);
	DbgAssert(NULL != poly);
	int nEdges = poly->ENum();
	BitArray edges(nEdges);
	ValueToBitArrayP(arg_list[1], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_SEL, MNM_SL_EDGE, MN_SEL, FALSE, FALSE, TRUE);
		epi->EpSetEdgeFlags(edges, MN_SEL, true);
		epi->LocalDataChanged(PART_TOPO|PART_SELECT); // needed to force invalidation of temp data
		epi->EpfnChamferEdgesOpen(arg_list[2]->to_float(),l_open, MAXScript_time());
	}
	return &ok;
}

// ==================================================================================================
//           start of mapping section - channels 0-based
// ==================================================================================================

void checkMapChannel (MNMesh* poly, int channel) {
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (channel >= poly->numm)
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(channel));
	if (poly->M(channel)->GetFlag (MN_DEAD))
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(channel));
}

Value*
SetNumMapVertsP(Value* source, int channel, int count, bool keep, TCHAR* fn_name)
{	
// Note: can have map verts without having map faces
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(source, MESH_BASE_OBJ, &owner, fn_name);
	checkMapChannel (poly, channel);
	if (count < 0)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), Integer::intern(count));
	MNMap* mnmap=poly->M(channel);
	mnmap->VAlloc (count, keep);
	mnmap->numv = count;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	return &ok;
}

Value*
GetNumMapVertsP(Value* source, int channel, TCHAR* fn_name)
{	
// can have map verts without having map faces
	MNMesh* poly = get_polyForValue(source, MESH_READ_ACCESS, NULL, fn_name);
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	checkMapChannel (poly, channel);
	return Integer::intern(poly->M(channel)->numv);
}

// ============================================================================
// SetNumMaps <Poly poly> <Integer count> keep:<boolean=true>
Value* 
polyop_setNumMaps_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMaps, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumMaps);
	int numMaps=arg_list[1]->to_int();
	range_check(numMaps, 0, MAX_MESHMAPS, GetString(IDS_MESH_NUMMAPS_OUT_OF_RANGE));
	Value* tmp = NULL;
	bool keep = bool_key_arg(keep, tmp, TRUE) == TRUE; 
	if (count < poly->numm)
	{
		poly->MAlloc(numMaps, keep);
	}
	else 
	{
		poly->MShrink(numMaps);
		if (!keep)
		{
			for (int i = 0; i < numMaps; i++)
			{
				poly->InitMap(i);
			}
		}
	}

	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((numMaps<1 || !keep) ? PART_VERTCOLOR|PART_TEXMAP : PART_TEXMAP);
	return &ok;
}

// ============================================================================
// <Integer> GetNumMaps <Poly poly>
Value* 
polyop_getNumMaps_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMaps, 1, arg_count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumMaps);
	return Integer::intern(poly->numm);
}

// ============================================================================
// SetMapSupport <Poly poly> <Integer mapChannel> <Boolean support>
Value* 
polyop_setMapSupport_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setMapSupport, 3, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapSupport);
	DbgAssert(NULL != poly);
	int channel =  arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	bool support = arg_list[2]->to_bool() == TRUE;
	bool change=false;
	if (support) {
		if (channel >= poly->MNum()) {
			poly->SetMapNum(channel+1);
			change=true;
		}
		// The M accessor now accepts a value in the range -NUM_HIDDENMAPS to MNum().
		int polyNumMapChannels = poly->MNum();
		if ( channel >= -NUM_HIDDENMAPS && channel < polyNumMapChannels) {
			MNMap* myMap = poly->M(channel);
			if (myMap != NULL && myMap->GetFlag (MN_DEAD)) {
				poly->InitMap(channel);
				change=true;
			}
		}
	}
	else {
		if (channel < poly->numm && !poly->M(channel)->GetFlag (MN_DEAD)) {
			poly->ClearMap(channel);
			change=true;
		}
	}
	if (change) {
		EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

		if (epi) 
			epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}
	return &ok;
}

// ============================================================================
// <Boolean> GetMapSupport <Poly poly> <Integer mapChannel>
Value* 
polyop_getMapSupport_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapSupport, 2, arg_count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapSupport);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	return bool_result(channel < poly->numm && !poly->M(channel)->GetFlag (MN_DEAD));
}

// ============================================================================
// SetNumMapVerts <Poly poly> <Integer mapChannel> <Integer count> keep:<boolean=false>
Value* 
polyop_setNumMapVerts_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMapVerts, 3, count);
	Value* tmp = NULL;
	bool keep = bool_key_arg(keep, tmp, FALSE) == TRUE; 
	return SetNumMapVertsP(arg_list[0], arg_list[1]->to_int(), arg_list[2]->to_int(), keep, _T("setNumMapVerts"));
}

// ============================================================================
// <Integer> GetNumMapVerts <Poly poly> <Integer mapChannel>
Value* 
polyop_getNumMapVerts_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMapVerts, 2, arg_count);
	return GetNumMapVertsP(arg_list[0], arg_list[1]->to_int(), _T("getNumMapVerts"));
}

// ============================================================================
// SetNumMapFaces <Poly poly> <Integer mapChannel> <Integer count> keep:<boolean=false>
// dangerous with VC and TV (channel 0 and 1) since no bounds checking with those. 
Value* 
polyop_setNumMapFaces_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumMapFaces, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumMapFaces);
	int channel = arg_list[1]->to_int();
	int fcount =  arg_list[2]->to_int();
	Value* tmp = NULL;
	bool keep = bool_key_arg(keep, tmp, FALSE) == TRUE; 
	checkMapChannel (poly, channel);
	if (poly->M(channel)->numv == 0)
		throw RuntimeError (GetString(IDS_MUST_HAVE_AT_LEAST_ONE_MAP_VERTEX_TO_CREATE_MAP_FACE));
	if (fcount < 1)
		throw RuntimeError (GetString(IDS_COUNT_MUST_NOT_BE_NEG_NUMBER), arg_list[2]);
	MNMap* mnmap=poly->M(channel);
	mnmap->FAlloc (fcount, keep);
	mnmap->numf = fcount;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	return &ok;
}

// ============================================================================
// <Integer> GetNumMapFaces <Poly poly> <Integer mapChannel>
Value* 
polyop_getNumMapFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getNumMapFaces, 2, arg_count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumMapFaces);
	int channel = arg_list[1]->to_int();
	checkMapChannel (poly, channel);
	return Integer::intern(poly->M(channel)->numf);
}

// ============================================================================
// SetMapVert <Poly poly> <Integer mapChannel> <Integer index> <Point3 uvw>
Value* 
polyop_setMapVert_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setMapVert, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapVert);
		int channel = arg_list[1]->to_int();
	int index =   arg_list[2]->to_int()-1;
	Point3 uvw =  arg_list[3]->to_point3();
	checkMapChannel (poly, channel);
	range_check(index+1, 1, poly->M(channel)->numv, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
//	poly->MV(channel,index) = uvw; // poly->MV returns a UVVert = not a pointer or reference
	poly->M(channel)->v[index] = uvw;
	return &ok;
}

// ============================================================================
// <Point3> GetMapVert <Poly poly> <Integer mapChannel> <Integer index>
Value* 
polyop_getMapVert_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapVert, 3, arg_count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapVert);
	int channel = arg_list[1]->to_int();
	int index =   arg_list[2]->to_int()-1;
	checkMapChannel (poly, channel);
	range_check(index+1, 1, poly->M(channel)->numv, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE));
	return new Point3Value(poly->MV(channel,index));
}

// ============================================================================
// SetMapFace <Poly poly> <Integer mapChannel> <Integer map face index> <map vertex array>
Value* 
polyop_setMapFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setMapFace, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setMapFace);
	int channel =   arg_list[1]->to_int();
	checkMapChannel (poly, channel);
	int faceindex = arg_list[2]->to_int()-1;
	range_check(faceindex+1, 1, poly->M(channel)->numf, GetString(IDS_MESH_MAP_FACE_INDEX_OUT_OF_RANGE));
	if (!is_array(arg_list[3]))
		throw RuntimeError (GetString(IDS_SETMAPFACE_VERTEX_LIST_MUST_BE_ARRAY_GOT), arg_list[3]);
	Array* vertArray = (Array*)arg_list[3];
	int degree = vertArray->size;
	int *v = new int[degree];
	int nMapVerts = poly->M(channel)->numv;
	for (int i = 0; i < degree; i++) {
		int vertIndex=vertArray->data[i]->to_int()-1;
		range_check(vertIndex+1, 1, nMapVerts, GetString(IDS_MESH_MAP_VERTEX_INDEX_OUT_OF_RANGE))
		v[i]=vertIndex;
	}
	poly->MF(channel,faceindex)->MakePoly(degree, v);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	return &ok;
}

// ============================================================================
// <array> GetMapFace <Poly poly> <Integer mapChannel> <Integer index>
Value* 
polyop_getMapFace_cf(Value** arg_list, int arg_count)
{
	check_arg_count(getMapFace, 3, arg_count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getMapFace);
	int channel =   arg_list[1]->to_int();
	int faceindex = arg_list[2]->to_int()-1;
	checkMapChannel (poly, channel);
	range_check(faceindex+1, 1, poly->M(channel)->numf, GetString(IDS_MESH_MAP_FACE_INDEX_OUT_OF_RANGE));
	MNMapFace *fm = poly->MF(channel,faceindex);
	one_typed_value_local(Array* verts);
	vl.verts = new Array (fm->deg);
	for (int i = 0; i < fm->deg; i++)
		vl.verts->append(Integer::intern(fm->tv[i]+1));
	return_value(vl.verts);
}

// ============================================================================
// defaultMapFaces <Poly poly> <Integer mapChannel>
// turns on map support for specified channel
Value* 
polyop_defaultMapFaces_cf(Value** arg_list, int arg_count)
{
	check_arg_count(defaultMapFaces, 2, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, defaultMapFaces);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	if (channel >= poly->numm)
		poly->SetMapNum(channel+1);
	poly->InitMap(channel);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	return &ok;
}

// ============================================================================
// ApplyUVWMap <Poly poly>
//             <#planar | #cylindrical | #spherical | #ball | #box> | <#face>
//             utile:<float>   vtile:<float>   wtile:<float>
//             uflip:<boolean> vflip:<boolean> wflip:<boolean>
//             cap:<boolean> tm:<Matrix3> channel:<integer>
// turns on map support for specified channel
Value* 
polyop_applyUVWMap_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(applyUVWMap, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, applyUVWMap);
	def_uvwmap_types();
	int uvwmapType = GetID(uvwmapTypes, elements(uvwmapTypes), arg_list[1]);
	Value* tmp = NULL;
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
	if (channel >= poly->numm)
		poly->SetMapNum(channel+1);
	UVWMapper mapper(uvwmapType, tm, cap, uTile, vTile, wTile, uFlip, vFlip, wFlip);
	poly->ApplyMapper (mapper, channel);;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	return &ok;
}

// ============================================================================
// <BitArray> GetVertsByColor <Poly poly> <Color color> <Float red_thresh> <Float green_thresh> <Float blue_thresh>  channel:<Integer=0>
// <BitArray> GetVertsByColor <Poly poly> <Point3 uvw> <Float u_thresh> <Float v_thresh> <Float w_thresh>  channel:<Integer=0>
Value*
polyop_getVertsByColor_cf(Value** arg_list, int count)
{	
	check_arg_count_with_keys(getVertsByColor, 5, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVertsByColor);
	Point3 tst_color =  arg_list[1]->to_point3();
	Point3 tst_thresh = Point3 (arg_list[2]->to_float(), arg_list[3]->to_float(), arg_list[4]->to_float());
	if (is_color(arg_list[1])) {
		tst_color /= 255.0f;
		tst_thresh /= 255.0f;
	}		
	Value* tmp = NULL;
	int channel = int_key_arg(channel, tmp, 0); 
	if (channel >= poly->numm)
		throw RuntimeError (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL), Integer::intern(channel));
	if (poly->M(channel)->GetFlag (MN_DEAD)) {
		if (channel == 0)
			return new BitArrayValue();
		else
			throw MAXException (GetString(IDS_MAP_SUPPORT_NOT_ENABLED_FOR_CHANNEL));
	}
	one_typed_value_local(BitArrayValue* matches);
	vl.matches = new BitArrayValue (poly->VNum());
	for (int i=0; i < poly->FNum() && i< poly->M(channel)->numf; i++) {
		MNMapFace *fm = poly->MF(channel,i);
		for (int j=0; j < fm->deg; j++) {
			int iTVert=fm->tv[j];
			if (iTVert < 0 || iTVert >= poly->M(channel)->numv) continue;
			Point3 delta_color = poly->M(channel)->V(iTVert) - tst_color;
			if (fabs(delta_color.x) > tst_thresh.x) continue;
			if (fabs(delta_color.y) > tst_thresh.y) continue;
			if (fabs(delta_color.z) > tst_thresh.z) continue;
			vl.matches->bits.Set(poly->F(i)->vtx[j]);
		}
	}
	return_value(vl.matches);
}

// ============================================================================
// SetVertColor <Poly poly> <Integer mapChannel> <vertlist> <Color color>
Value* 
polyop_setVertColor_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setVertColor, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVertColor);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[2], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	Point3 color = arg_list[3]->to_point3()/255.0f;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->BeginVertexColorModify(channel);
		epi->SetVertexColor (color, channel, MN_TARG, MAXScript_time());
		epi->EndVertexColorModify(true);
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}
	return &ok;
}

// ============================================================================
// SetFaceColor <Poly poly> <Integer mapChannel> <facelist> <Color color>
Value* 
polyop_setFaceColor_cf(Value** arg_list, int arg_count)
{
// can have map verts without having map faces
	check_arg_count(setFaceColor, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setFaceColor);
	int channel = arg_list[1]->to_int();
	range_check(channel, -NUM_HIDDENMAPS, MAX_MESHMAPS-1, GetString(IDS_MESH_MAP_CHANNEL_OUT_OF_RANGE));
	int nFaces = poly->FNum();
	BitArray faces (nFaces);
	ValueToBitArrayP(arg_list[2], faces, nFaces, GetString(IDS_POLY_FACE_INDEX_OUT_OF_RANGE), MESH_FACESEL_ALLOWED, poly);
	Point3 color = arg_list[3]->to_point3()/255.0f;
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_FACE, MN_TARG, MNM_SL_FACE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetFaceFlags(faces, MN_TARG, false);
		epi->BeginVertexColorModify(channel);
		epi->SetFaceColor (color, channel, MN_TARG, MAXScript_time());
		epi->EndVertexColorModify(true);
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}
	return &ok;
}

// ============================================================================
// SetNumVDataChannels <Poly poly> <Integer count> keep:<boolean=true>
// Note: first ten channels for Discreet's use only.
// channel 1: Soft Selection
// channel 2  Vertex weights (for NURMS MeshSmooth)
// channel 3  Vertex Alpha values
// channel 4  Cornering values for subdivision use

Value* 
polyop_setNumVDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumVDataChannels, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumVDataChannels);
	int numChannels=arg_list[1]->to_int();
	range_check(numChannels, 0, MAX_VERTDATA, GetString(IDS_MESH_NUMVERTDATA_OUT_OF_RANGE));
	Value* tmp = NULL;
	bool keep = bool_key_arg(keep, tmp, TRUE) == TRUE; 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	poly->setNumVData(numChannels,keep);
	if (epi) 
		epi->LocalDataChanged (PART_GEOM);	
	return &ok;
}


// ============================================================================
// <Integer> GetNumVDataChannels <Poly poly>
Value* 
polyop_getNumVDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count(getNumVDataChannels, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumVDataChannels);
	return Integer::intern(poly->VDNum());
}

// ============================================================================
// SetVDataChannelSupport <Poly poly> <Integer vdChannel> <Boolean support>
Value* 
polyop_setVDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(setVDataChannelSupport, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVDataChannelSupport);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	BOOL support = arg_list[2]->to_bool();
	poly->setVDataSupport(channel,support);
	if (epi) 
		epi->LocalDataChanged (PART_GEOM);	
	return &ok;
}

// ============================================================================
// <Boolean> GetVDataChannelSupport <Poly poly> <Integer vdChannel>
Value* 
polyop_getVDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(getVDataChannelSupport, 2, count);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVDataChannelSupport);
	return bool_result(poly->vDataSupport(channel));
}

// ============================================================================
// <Float> GetVDataValue <Poly poly> <Integer vdChannel> <Integer vertex_index>
Value* 
polyop_getVDataValue_cf(Value** arg_list, int count)
{
	check_arg_count(getVDataValue, 3, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getVDataValue);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->vDataSupport(channel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int index=arg_list[2]->to_int()-1;
	range_check(index+1, 1, poly->vd[channel].Count(), GetString(IDS_MESH_VERTDATA_INDEX_OUT_OF_RANGE));
	return Float::intern(poly->vertexFloat(channel)[index]);
}

// ============================================================================
// setVDataValue <Poly poly> <Integer channel> <vertlist> <float>
Value* 
polyop_setVDataValue_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setVDataValue, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setVDataValue);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	int nVerts = poly->VNum();
	BitArray verts (nVerts);
	ValueToBitArrayP(arg_list[2], verts, nVerts, GetString(IDS_POLY_VERTEX_INDEX_OUT_OF_RANGE), MESH_VERTSEL_ALLOWED, poly);
	float val = arg_list[3]->to_float();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_VERTEX, MN_TARG, MNM_SL_VERTEX, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetVertexFlags(verts, MN_TARG, false);
		epi->BeginPerDataModify(MNM_SL_VERTEX,channel);
		epi->SetVertexDataValue (channel, val, MN_TARG, MAXScript_time());
		epi->EndPerDataModify(true);
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}
	return &ok;
}

// ============================================================================
// freeVData <Poly poly> <Integer vdChannel>
Value* 
polyop_freeVData_cf(Value** arg_list, int count)
{
	check_arg_count(freeVData, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeVData);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->vDataSupport(channel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	poly->freeVData(channel);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->LocalDataChanged (PART_GEOM);	
	}
	return &ok;
}

// ============================================================================
// resetVData <Poly poly> <Integer vdChannel>
Value* 
polyop_resetVData_cf(Value** arg_list, int count)
{
	check_arg_count(resetVData, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetVData);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_VERTDATA, GetString(IDS_MESH_VERTDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->vDataSupport(channel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->ResetVertexData (channel);	
	return &ok;
}


// ============================================================================
// SetNumEDataChannels <Poly poly> <Integer count> keep:<boolean=true>
// Note: first two channels for Discreet's use only.
// channel 1: edge knot data
// channel 2: edge crease data

Value* 
polyop_setNumEDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(setNumEDataChannels, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setNumEDataChannels);
	int numChannels=arg_list[1]->to_int();
	range_check(numChannels, 0, MAX_EDGEDATA, GetString(IDS_MESH_NUMEDGEDATA_OUT_OF_RANGE));
	Value* tmp = NULL;
	bool keep = bool_key_arg(keep, tmp, TRUE) == TRUE; 
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	poly->setNumEData(numChannels,keep);
	if (epi) 
		epi->LocalDataChanged (PART_GEOM);	
	return &ok;
}


// ============================================================================
// <Integer> GetNumEDataChannels <Poly poly>
Value* 
polyop_getNumEDataChannels_cf(Value** arg_list, int count)
{
	check_arg_count(getNumEDataChannels, 1, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getNumEDataChannels);
	return Integer::intern(poly->EDNum());
}

// ============================================================================
// SetEDataChannelSupport <Poly poly> <Integer vdChannel> <Boolean support>
Value* 
polyop_setEDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(setEDataChannelSupport, 3, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setEDataChannelSupport);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	BOOL support = arg_list[2]->to_bool();
	poly->setEDataSupport(channel,support);
	if (!support && epi) 
		epi->LocalDataChanged (PART_GEOM);	
	return &ok;
}

// ============================================================================
// <Boolean> GetEDataChannelSupport <Poly poly> <Integer vdChannel>
Value* 
polyop_getEDataChannelSupport_cf(Value** arg_list, int count)
{
	check_arg_count(getEDataChannelSupport, 2, count);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEDataChannelSupport);
	return bool_result(poly->eDataSupport(channel));
}

// ============================================================================
// <Float> GetEDataValue <Poly poly> <Integer vdChannel> <Integer edge_index>
Value* 
polyop_getEDataValue_cf(Value** arg_list, int count)
{
	check_arg_count(getEDataValue, 3, count);
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_READ_ACCESS, NULL, getEDataValue);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->eDataSupport(channel))
		throw RuntimeError (GetString(IDS_VERTEXDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	int index=arg_list[2]->to_int()-1;
	range_check(index+1, 1, poly->ed[channel].Count(), GetString(IDS_MESH_EDGEDATA_INDEX_OUT_OF_RANGE));
	return Float::intern(poly->edgeFloat(channel)[index]);
}

// ============================================================================
// setEDataValue <Poly poly> <Integer channel> <edgelist> <float>
Value* 
polyop_setEDataValue_cf(Value** arg_list, int arg_count)
{
	check_arg_count(setEDataValue, 4, arg_count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, setEDataValue);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	int nEdges = poly->ENum();
	BitArray edges (nEdges);
	ValueToBitArrayP(arg_list[2], edges, nEdges, GetString(IDS_POLY_EDGE_INDEX_OUT_OF_RANGE), MESH_EDGESEL_ALLOWED, poly);
	float val = arg_list[3]->to_float();
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) {
		epi->EpfnPropagateComponentFlags(MNM_SL_EDGE, MN_TARG, MNM_SL_EDGE, MN_TARG, FALSE, FALSE, FALSE);
		epi->EpSetEdgeFlags(edges, MN_TARG, false);
		epi->BeginPerDataModify(MNM_SL_EDGE,channel);
		epi->SetEdgeDataValue (channel, val, MN_TARG, MAXScript_time());
		epi->EndPerDataModify(true);
		epi->LocalDataChanged ((channel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	}
	return &ok;
}


// ============================================================================
// freeEData <Poly poly> <Integer vdChannel>
Value* 
polyop_freeEData_cf(Value** arg_list, int count)
{
	check_arg_count(freeEData, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, freeEData);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->eDataSupport(channel))
		throw RuntimeError (GetString(IDS_EDGEDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	poly->freeEData(channel);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->LocalDataChanged (PART_GEOM);	
	return &ok;
}

// ============================================================================
// resetEData <Poly poly> <Integer vdChannel>
Value* 
polyop_resetEData_cf(Value** arg_list, int count)
{
	check_arg_count(resetEData, 2, count);
	ReferenceTarget* owner = NULL;
	MNMesh* poly = get_polyForValue(arg_list[0], MESH_BASE_OBJ, &owner, resetEData);
	int channel = arg_list[1]->to_int()-1;
	range_check(channel+1, 1, MAX_EDGEDATA, GetString(IDS_MESH_EDGEDATA_CHANNEL_OUT_OF_RANGE));
	if (!poly->eDataSupport(channel))
		throw RuntimeError (GetString(IDS_EDGEDATA_SUPPORT_NOT_ENABLED_FOR_CHANNEL), arg_list[1]);
	EPoly* epi = NULL;
	if (owner) {
		epi = (EPoly*)((Animatable*)owner)->GetInterface(EPOLY_INTERFACE);
	}

	if (epi) 
		epi->ResetEdgeData (channel);	
	return &ok;
}

