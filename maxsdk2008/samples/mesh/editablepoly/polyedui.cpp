/**********************************************************************
 *<
	FILE: PolyEdUI.cpp

	DESCRIPTION: Editable Polygon Mesh Object / Edit Polygon Mesh Modifier UI code

	CREATED BY: Steve Anderson

	HISTORY: created Nov 9, 1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "MeshDLib.h"
#include "MaxIcon.h"
#include "macrorec.h"
#include "maxscrpt\MAXScrpt.h"
#include "maxscrpt\MAXObj.h"
#include "maxscrpt\3DMath.h"
#include "maxscrpt\msplugin.h"

#define USE_POPUP_DIALOG_ICON

// this max matches the MI max.
#define MAX_SUBDIV 7

// Class used to track the "current" position of the EPoly popup dialogs
static EPolyPopupPosition thePopupPosition;

// Luna task 748A - Preview dialog methods.
void EPolyPopupPosition::Scan (HWND hWnd) {
	if (!hWnd || !IsWindow(hWnd)) return;
	RECT rect;
	GetWindowRect (hWnd, &rect);
	mPosition[0] = rect.left;
	mPosition[1] = rect.top;
	mPosition[2] = rect.right - rect.left - 1;
	mPosition[3] = rect.bottom - rect.top;
	mPositionSet = true;
}

// Luna task 748A - Preview dialog methods.
void EPolyPopupPosition::Set (HWND hWnd) {
	if (!hWnd) return;
	if (mPositionSet) 
		SetWindowPos (hWnd, NULL, mPosition[0], mPosition[1],
			mPosition[2], mPosition[3], SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	else
		CenterWindow (hWnd, GetCOREInterface()->GetMAXHWnd());
}

// Luna task 748A - Preview dialog methods.
void EditPolyObject::EpfnClosePopupDialog () {
	if (!pOperationSettings) return;
	thePopupPosition.Scan (pOperationSettings->GetHWnd());
	DestroyModelessParamMap2 (pOperationSettings);
	pOperationSettings = NULL;

	if (EpPreviewOn ()) EpPreviewCancel ();

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.closePopupDialog"), 0, 0);
	macroRecorder->EmitScript ();
#endif
}

// Class Descriptor
// (NOTE: this must be in the same file as, and previous to, the ParamBlock2Desc declaration.)

static EditablePolyObjectClassDesc editPolyObjectDesc;
ClassDesc2* GetEditablePolyDesc() {return &editPolyObjectDesc;}

static FPInterfaceDesc epfi (
EPOLY_INTERFACE, _T("EditablePoly"), IDS_EDITABLE_POLY, &editPolyObjectDesc, FP_MIXIN,
	epfn_hide, _T("Hide"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_unhide_all, _T("unhideAll"), 0, TYPE_bool, 0, 1,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
	epfn_named_selection_copy, _T("namedSelCopy"), 0, TYPE_VOID, 0, 1,
		_T("Name"), 0, TYPE_STRING,
	epfn_named_selection_paste, _T("namedSelPaste"), 0, TYPE_VOID, 0, 1,
		_T("useRenameDialog"), 0, TYPE_bool,
	epfn_create_vertex, _T("createVertex"), 0, TYPE_INDEX, 0, 3,
		_T("point"), 0, TYPE_POINT3,
		_T("pointInLocalCoords"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_create_edge, _T("createEdge"), 0, TYPE_INDEX, 0, 3,
		_T("vertex1"), 0, TYPE_INDEX,
		_T("vertex2"), 0, TYPE_INDEX,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_create_face, _T("createFace"), 0, TYPE_INDEX, 0, 2,
		_T("vertexArray"), 0, TYPE_INDEX_TAB,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_cap_holes, _T("capHoles"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_delete, _T("delete"), 0, TYPE_bool, 0, 3,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
		_T("deleteIsoVerts"), 0, TYPE_bool, f_keyArgDefault, true,
	epfn_attach, _T("attach"), 0, TYPE_VOID, 0, 2,
		_T("nodeToAttach"), 0, TYPE_INODE,
		_T("myNode"), 0, TYPE_INODE,
	//epfn_multi_attach, _T("multiAttach"), 0, TYPE_VOID, 0, 2,
		//_T("nodeTable"), 0, TYPE_INODE_TAB_BR,
		//_T("myNode"), 0, TYPE_INODE,
	epfn_detach_to_element, _T("detachToElement"), 0, TYPE_bool, 0, 3,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
		_T("keepOriginal"), 0, TYPE_bool, f_keyArgDefault, false,
	//epfn_detach_to_object, _T("detachToObject"), 0, TYPE_bool, 0, 5,
		//_T("newNodeName"), 0, TYPE_STRING,
		//_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		//_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
		//_T("keepOriginal"), 0, TYPE_bool, f_keyArgDefault, false,
		//_T("myNode"), 0, TYPE_INODE,
	epfn_split_edges, _T("splitEdges"), 0, TYPE_bool, 0, 1,
		_T("edgeFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_break_verts, _T("breakVerts"), 0, TYPE_bool, 0, 1,
		_T("vertFlags"), 0, TYPE_DWORD,

	// keep version with old name for backward compatibility:
	epfn_divide_face, _T("divideFace"), 0, TYPE_INDEX, 0, 3,
		_T("faceID"), 0, TYPE_INDEX,
		_T("vertexCoefficients"), 0, TYPE_FLOAT_TAB_BR,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,

	// Add second version to match new name.
	epfn_divide_face, _T("insertVertexInFace"), 0, TYPE_INDEX, 0, 3,
		_T("faceID"), 0, TYPE_INDEX,
		_T("vertexCoefficients"), 0, TYPE_FLOAT_TAB_BR,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,

	// keep version with old name for backward compatibility:
	epfn_divide_edge, _T("divideEdge"), 0, TYPE_INDEX, 0, 3,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("proportion"), 0, TYPE_FLOAT,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,

	// Add second version to match new name.
	epfn_divide_edge, _T("insertVertexInEdge"), 0, TYPE_INDEX, 0, 3,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("proportion"), 0, TYPE_FLOAT,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_collapse, _T("collapse"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_extrude_faces, _T("extrudeFaces"), 0, TYPE_VOID, 0, 2,
		_T("amount"), 0, TYPE_FLOAT,
		_T("faceFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_bevel_faces, _T("bevelFaces"), 0, TYPE_VOID, 0, 3,
		_T("height"), 0, TYPE_FLOAT,
		_T("outline"), 0, TYPE_FLOAT,
		_T("faceFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_chamfer_vertices, _T("chamferVertices"), 0, TYPE_VOID, 0, 2,
		_T("amount"), 0, TYPE_FLOAT,
		_T("open"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_chamfer_edges, _T("chamferEdges"), 0, TYPE_VOID, 0, 2,
		_T("amount"), 0, TYPE_FLOAT,
		_T("open"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_slice, _T("slice"), 0, TYPE_bool, 0, 4,
		_T("slicePlaneNormal"), 0, TYPE_POINT3,
		_T("slicePlaneCenter"), 0, TYPE_POINT3,
		_T("flaggedFacesOnly"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("faceFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_in_slice_plane_mode, _T("inSlicePlaneMode"), 0, TYPE_bool, 0, 0,
	epfn_cut_vertex, _T("cutVertices"), 0, TYPE_INDEX, 0, 3,
		_T("startVertex"), 0, TYPE_INDEX,
		_T("endPosition"), 0, TYPE_POINT3,
		_T("viewDirection"), 0, TYPE_POINT3,
	epfn_cut_edge, _T("cutEdges"), 0, TYPE_INDEX, 0, 5,
		_T("startEdge"), 0, TYPE_INDEX,
		_T("startProportion"), 0, TYPE_FLOAT,
		_T("endEdge"), 0, TYPE_INDEX,
		_T("endProportion"), 0, TYPE_FLOAT,
		_T("viewDirection"), 0, TYPE_POINT3,
	epfn_cut_face, _T("cutFaces"), 0, TYPE_INDEX, 0, 4,
		_T("startFace"), 0, TYPE_INDEX,
		_T("startPosition"), 0, TYPE_POINT3,
		_T("endPosition"), 0, TYPE_POINT3,
		_T("viewDirection"), 0, TYPE_POINT3,
	epfn_weld_verts, _T("weldVerts"), 0, TYPE_bool, 0, 3,
		_T("vertex1"), 0, TYPE_INDEX,
		_T("vertex2"), 0, TYPE_INDEX,
		_T("destinationPoint"), 0, TYPE_POINT3,
	epfn_weld_edges, _T("weldEdges"), 0, TYPE_bool, 0, 2,
		_T("edge1"), 0, TYPE_INDEX,
		_T("edge2"), 0, TYPE_INDEX,
	epfn_weld_flagged_verts, _T("weldFlaggedVertices"), 0, TYPE_bool, 0, 1,
		_T("vertexFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_weld_flagged_edges, _T("weldFlaggedEdges"), 0, TYPE_bool, 0, 1,
		_T("edgeFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_create_shape, _T("createShape"), 0, TYPE_bool, 0, 4,
		_T("shapeName"), 0, TYPE_STRING,
		_T("curved"), 0, TYPE_bool,
		_T("myNode"), 0, TYPE_INODE,
		_T("edgeFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_make_planar, _T("makePlanar"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_move_to_plane, _T("moveToPlane"), 0, TYPE_bool, 0, 4,
		_T("planeNormal"), 0, TYPE_POINT3,
		_T("planeOffset"), 0, TYPE_FLOAT,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_align_to_grid, _T("alignToGrid"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_align_to_view, _T("alignToView"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_delete_iso_verts, _T("deleteIsoVerts"), 0, TYPE_bool, 0, 0,
	epfn_meshsmooth, _T("meshSmooth"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_tessellate, _T("tessellate"), 0, TYPE_bool, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_force_subdivision, _T("forceSubdivision"), 0, TYPE_VOID, 0, 0,
	epfn_set_diagonal, _T("setDiagonal"), 0, TYPE_VOID, 0, 3,
		_T("face"), 0, TYPE_INDEX,
		_T("corner1"), 0, TYPE_INDEX,
		_T("corner2"), 0, TYPE_INDEX,
	epfn_retriangulate, _T("retriangulate"), 0, TYPE_bool, 0, 1,
		_T("faceFlags"), 0, TYPE_DWORD,
	epfn_flip_normals, _T("flipNormals"), 0, TYPE_bool, 0, 1,
		_T("faceFlags"), 0, TYPE_DWORD,
	epfn_select_by_mat, _T("selectByMaterial"), 0, TYPE_VOID, 0, 2,
		_T("materialID"), 0, TYPE_INDEX,
		_T("clearCurrentSelection"), 0, TYPE_bool, f_keyArgDefault, true,
	epfn_select_by_smooth_group, _T("selectBySmoothGroup"), 0, TYPE_VOID, 0, 2,
		_T("smoothingGroups"), 0, TYPE_DWORD,
		_T("clearCurrentSelection"), 0, TYPE_bool, f_keyArgDefault, true,
	epfn_autosmooth, _T("autosmooth"), 0, TYPE_VOID, 0, 0,

	epfn_button_op, _T("buttonOp"), 0, TYPE_VOID, 0, 1,
		_T("buttonOpID"), 0, TYPE_ENUM, epolyEnumButtonOps,
	epfn_toggle_command_mode, _T("toggleCommandMode"), 0, TYPE_VOID, 0, 1,
		_T("commandModeID"), 0, TYPE_ENUM, epolyEnumCommandModes,
	epfn_enter_pick_mode, _T("enterPickMode"), 0, TYPE_VOID, 0, 1,
		_T("pickModeID"), 0, TYPE_ENUM, epolyEnumPickModes,
	epfn_exit_command_modes, _T("exitCommandModes"), 0, TYPE_VOID, 0, 0,

	// Flag Accessor methods:
	epfn_get_vertices_by_flag, _T("getVerticesByFlag"), 0, TYPE_bool, 0, 3,
		_T("vertexSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
	epfn_get_edges_by_flag, _T("getEdgesByFlag"), 0, TYPE_bool, 0, 3,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
	epfn_get_faces_by_flag, _T("getFacesByFlag"), 0, TYPE_bool, 0, 3,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,

	epfn_set_vertex_flags, _T("setVertexFlags"), 0, TYPE_VOID, 0, 4,
		_T("vertexSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,
	epfn_set_edge_flags, _T("setEdgeFlags"), 0, TYPE_VOID, 0, 4,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,
	epfn_set_face_flags, _T("setFaceFlags"), 0, TYPE_VOID, 0, 4,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,

	// Data accessor methods:
	epfn_reset_slice_plane, _T("resetSlicePlane"), 0, TYPE_VOID, 0, 0,
	epfn_get_slice_plane, _T("getSlicePlane"), 0, TYPE_VOID, 0, 3,
		_T("planeNormal"), 0, TYPE_POINT3_BR,
		_T("planeCenter"), 0, TYPE_POINT3_BR,
		_T("planeSize"), 0, TYPE_FLOAT_BP,
	epfn_set_slice_plane, _T("setSlicePlane"), 0, TYPE_VOID, 0, 3,
		_T("planeNormal"), 0, TYPE_POINT3_BR,
		_T("planeCenter"), 0, TYPE_POINT3_BR,
		_T("planeSize"), 0, TYPE_FLOAT,
	epfn_get_vertex_data, _T("getVertexData"), 0, TYPE_FLOAT, 0, 4,
		_T("vertexDataChannel"), 0, TYPE_INT,
		_T("numberSelected"), 0, TYPE_INT_BP,
		_T("uniformData"), 0, TYPE_bool_BP,
		_T("vertexFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_get_edge_data, _T("getEdgeData"), 0, TYPE_FLOAT, 0, 4,
		_T("edgeDataChannel"), 0, TYPE_INT,
		_T("numberSelected"), 0, TYPE_INT_BP,
		_T("uniformData"), 0, TYPE_bool_BP,
		_T("edgeFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_set_vertex_data, _T("setVertexData"), 0, TYPE_VOID, 0, 3,
		_T("vertexDataChannel"), 0, TYPE_INT,
		_T("value"), 0, TYPE_FLOAT,
		_T("vertexFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_set_edge_data, _T("setEdgeData"), 0, TYPE_VOID, 0, 3,
		_T("edgeDataChannel"), 0, TYPE_INT,
		_T("value"), 0, TYPE_FLOAT,
		_T("edgeFlags"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_reset_vertex_data, _T("resetVertexData"), 0, TYPE_VOID, 0, 1,
		_T("vertexDataChannel"), 0, TYPE_INT,
	epfn_reset_edge_data, _T("resetEdgeData"), 0, TYPE_VOID, 0, 1,
		_T("edgeDataChannel"), 0, TYPE_INT,
	epfn_begin_modify_perdata, _T("beginModifyPerData"), 0, TYPE_VOID, 0, 2,
		_T("mnSelLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("dataChannel"), 0, TYPE_INT,
	epfn_in_modify_perdata, _T("inModifyPerData"), 0, TYPE_bool, 0, 0,
	epfn_end_modify_perdata, _T("endModifyPerData"), 0, TYPE_VOID, 0, 1,
		_T("success"), 0, TYPE_bool,
	epfn_get_mat_index, _T("getMaterialIndex"), 0, TYPE_INDEX, 0, 1,
		_T("determined"), 0, TYPE_bool_BP,
	epfn_set_mat_index, _T("setMaterialIndex"), 0, TYPE_VOID, 0, 2,
		_T("index"), 0, TYPE_INDEX,
		_T("faceFlags"), 0, TYPE_DWORD,
	epfn_get_smoothing_groups, _T("getSmoothingGroups"), 0, TYPE_VOID, 0, 3,
		_T("faceFlag"), 0, TYPE_DWORD,
		_T("anyFaces"), 0, TYPE_DWORD_BP,
		_T("allFaces"), 0, TYPE_DWORD_BP,
	epfn_set_smoothing_groups, _T("setSmoothingGroups"), 0, TYPE_VOID, 0, 3,
		_T("bitValues"), 0, TYPE_DWORD,
		_T("bitMask"), 0, TYPE_DWORD, 
		_T("faceFlags"), 0, TYPE_DWORD,

	epfn_collapse_dead_structs, _T("collapeDeadStrctures"), 0, TYPE_VOID, 0, 0,
	epfn_collapse_dead_structs_spelled_right, _T("collapseDeadStructures"), 0, TYPE_VOID, 0, 0,
	epfn_propagate_component_flags, _T("propogateComponentFlags"), 0, TYPE_INT, 0, 7,
		_T("mnSelLevelTo"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flagSetTo"), 0, TYPE_DWORD,
		_T("mnSelLevelFrom"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("flagTestFrom"), 0, TYPE_DWORD,
		_T("allBitsMustMatch"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("set"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("undoable"), 0, TYPE_bool, f_keyArgDefault, false,

	epfn_preview_begin, _T("PreviewBegin"), 0, TYPE_VOID, 0, 1,
		_T("previewOperation"), 0, TYPE_ENUM, epolyEnumButtonOps,
	epfn_preview_cancel, _T("PreviewCancel"), 0, TYPE_VOID, 0, 0,
	epfn_preview_accept, _T("PreviewAccept"), 0, TYPE_VOID, 0, 0,
	epfn_preview_invalidate, _T("PreviewInvalidate"), 0, TYPE_VOID, 0, 0,
	epfn_preview_on, _T("PreviewOn"), 0, TYPE_bool, 0, 0,
	epfn_preview_set_dragging, _T("PreviewSetDragging"), 0, TYPE_VOID, 0, 1,
		_T("dragging"), 0, TYPE_bool,
	epfn_preview_get_dragging, _T("PreviewGetDragging"), 0, TYPE_bool, 0, 0,

	epfn_popup_dialog, _T("PopupDialog"), 0, TYPE_bool, 0, 1,
		_T("popupOperation"), 0, TYPE_ENUM, epolyEnumButtonOps,
	epfn_close_popup_dialog, _T("ClosePopupDialog"), 0, TYPE_VOID, 0, 0,

	epfn_repeat_last, _T("RepeatLastOperation"), 0, TYPE_VOID, 0, 0,

	epfn_grow_selection, _T("GrowSelection"), 0, TYPE_VOID, 0, 1,
		_T("selLevel"), 0, TYPE_ENUM, PMeshSelLevel, f_keyArgDefault, MNM_SL_CURRENT,
	epfn_shrink_selection, _T("ShrinkSelection"), 0, TYPE_VOID, 0, 1,
		_T("selLevel"), 0, TYPE_ENUM, PMeshSelLevel, f_keyArgDefault, MNM_SL_CURRENT,
	epfn_convert_selection, _T("ConvertSelection"), 0, TYPE_INT, 0, 3,
		_T("fromSelLevel"), 0, TYPE_ENUM, ePolySelLevel,
		_T("toSelLevel"), 0, TYPE_ENUM, ePolySelLevel,
		_T("requireAll"), 0, TYPE_bool, f_keyArgDefault, false,
	epfn_select_border, _T("SelectBorder"), 0, TYPE_VOID, 0,0,
	epfn_select_element, _T("SelectElement"), 0, TYPE_VOID, 0, 0,
	epfn_select_edge_loop, _T("SelectEdgeLoop"), 0, TYPE_VOID, 0, 0,
	epfn_select_edge_ring, _T("SelectEdgeRing"), 0, TYPE_VOID, 0, 0,
	epfn_remove, _T("Remove"), 0, TYPE_bool, 0, 2,
		_T("selLevel"), 0, TYPE_ENUM, PMeshSelLevel, f_keyArgDefault, MNM_SL_CURRENT,
		_T("flag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_delete_iso_map_verts, _T("DeleteIsoMapVerts"), 0, TYPE_bool, 0, 0,
	epfn_outline, _T("Outline"), 0, TYPE_bool, 0, 1,
		_T("faceFlag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_connect_edges, _T("ConnectEdges"), 0, TYPE_bool, 0, 1,
		_T("edgeFlag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_connect_vertices, _T("ConnectVertices"), 0, TYPE_bool, 0, 1,
		_T("vertexFlag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_extrude_along_spline, _T("ExtrudeAlongSpline"), 0, TYPE_bool, 0, 1,
		_T("faceFlag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_lift_from_edge, _T("HingeFromEdge"), 0, TYPE_bool, 0, 1,
		_T("faceFlag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_toggle_shaded_faces, _T("ToggleShadedFaces"), 0, TYPE_VOID, 0, 0,

	epfn_bridge, _T("Bridge"), 0, TYPE_bool, 0, 2,
		_T("selLevel"), 0, TYPE_ENUM, ePolySelLevel, f_keyArgDefault, EP_SL_CURRENT,
		_T("flag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_ready_to_bridge_selected, _T("ReadyToBridgeFlagged"), 0, TYPE_bool, 0, 2,
		_T("selLevel"), 0, TYPE_ENUM, ePolySelLevel, f_keyArgDefault, EP_SL_CURRENT,
		_T("flag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_turn_diagonal, _T("TurnDiagonal"), 0, TYPE_bool, 0, 2,
		_T("face"), 0, TYPE_INDEX,
		_T("diagonal"), 0, TYPE_INDEX,
	epfn_relax, _T("Relax"), 0, TYPE_bool, 0, 2,
		_T("selLevel"), 0, TYPE_ENUM, PMeshSelLevel, f_keyArgDefault, MNM_SL_CURRENT,
		_T("flag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,
	epfn_make_planar_in, _T("MakePlanarIn"), 0, TYPE_bool, 0, 3,
		_T("axis"), 0, TYPE_ENUM, axisEnum,
		_T("selLevel"), 0, TYPE_ENUM, PMeshSelLevel, f_keyArgDefault, MNM_SL_CURRENT,
		_T("flag"), 0, TYPE_DWORD, f_keyArgDefault, MN_SEL,

	epfn_get_epoly_sel_level, _T("GetEPolySelLevel"), 0, TYPE_ENUM, ePolySelLevel, 0, 0,
	epfn_get_mn_sel_level, _T("GetMeshSelLevel"), 0, TYPE_ENUM, PMeshSelLevel, 0, 0,

	epfn_get_selection, _T("GetSelection"), 0, TYPE_BITARRAY, 0, 1,
		_T("selectionLevel"), 0, TYPE_ENUM, PMeshSelLevel,
	epfn_set_selection, _T("SetSelection"), 0, TYPE_VOID, 0, 2,
		_T("selectionLevel"), 0, TYPE_ENUM, PMeshSelLevel,
		_T("selection"), 0, TYPE_BITARRAY,

	epfn_get_num_vertices, _T("GetNumVertices"), 0, TYPE_INT, 0, 0,
	epfn_get_vertex, _T("GetVertex"), 0, TYPE_POINT3_BV, 0, 1,
		_T("vertexID"), 0, TYPE_INDEX,
	epfn_get_vertex_face_count, _T("GetVertexFaceCount"), 0, TYPE_INT, 0, 1,
		_T("vertexID"), 0, TYPE_INDEX,
	epfn_get_vertex_face, _T("GetVertexFace"), 0, TYPE_INDEX, 0, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("face"), 0, TYPE_INDEX,
	epfn_get_vertex_edge_count, _T("GetVertexEdgeCount"), 0, TYPE_INT, 0, 1,
		_T("vertexID"), 0, TYPE_INDEX,
	epfn_get_vertex_edge, _T("GetVertexEdge"), 0, TYPE_INDEX, 0, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("edge"), 0, TYPE_INDEX,

	epfn_get_num_edges, _T("GetNumEdges"), 0, TYPE_INT, 0, 0,
	epfn_get_edge_vertex, _T("GetEdgeVertex"), 0, TYPE_INDEX, 0, 2,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("end"), 0, TYPE_INDEX,
	epfn_get_edge_face, _T("GetEdgeFace"), 0, TYPE_INDEX, 0, 2,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,

	epfn_get_num_faces, _T("GetNumFaces"), 0, TYPE_INT, 0, 0,
	epfn_get_face_degree, _T("GetFaceDegree"), 0, TYPE_INT, 0, 1,
		_T("faceID"), 0, TYPE_INDEX,
	epfn_get_face_vertex, _T("GetFaceVertex"), 0, TYPE_INDEX, 0, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
	epfn_get_face_edge, _T("GetFaceEdge"), 0, TYPE_INDEX, 0, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
	epfn_get_face_material, _T("GetFaceMaterial"), 0, TYPE_INDEX, 0, 1,
		_T("faceID"), 0, TYPE_INDEX,
	epfn_get_face_smoothing_group, _T("GetFaceSmoothingGroups"), 0, TYPE_DWORD, 0, 1,
		_T("faceID"), 0, TYPE_INDEX,

	epfn_get_num_map_channels, _T("GetNumMapChannels"), 0, TYPE_INT, 0, 0,
	epfn_get_map_channel_active, _T("GetMapChannelActive"), 0, TYPE_bool, 0, 1,
		_T("mapChannel"), 0, TYPE_INT,
	epfn_get_num_map_vertices, _T("GetNumMapVertices"), 0, TYPE_INT, 0, 1,
		_T("mapChannel"), 0, TYPE_INT,
	epfn_get_map_vertex, _T("GetMapVertex"), 0, TYPE_POINT3_BV, 0, 2,
		_T("mapChannel"), 0, TYPE_INT,
		_T("vertexID"), 0, TYPE_INDEX,
	epfn_get_map_face_vertex, _T("GetMapFaceVertex"), 0, TYPE_INT, 0, 3,
		_T("mapChannel"), 0, TYPE_INT,
		_T("faceID"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
	epfn_get_map_face_vert, _T("GetMapFaceVert"), 0, TYPE_INDEX, 0, 3,
		_T("mapChannel"), 0, TYPE_INT,
		_T("faceID"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
	epfn_set_ring_shift, _T("setRingShift"), 0, TYPE_VOID, 0, 3,
		_T("RingShiftValue"), 0, TYPE_INT,
		_T("MoveOnly"), 0, TYPE_bool,
		_T("Add"), 0, TYPE_bool,
	epfn_set_loop_shift, _T("setLoopShift"), 0, TYPE_VOID, 0, 3,
		_T("LoopShiftValue"), 0, TYPE_INT,
		_T("MoveOnly"), 0, TYPE_bool,
		_T("Add"), 0, TYPE_bool,
	epfn_convert_selection_to_border, _T("ConvertSelectionToBorder"), 0, TYPE_INT, 0, 2,
		_T("fromSelLevel"), 0, TYPE_ENUM, ePolySelLevel,
		_T("toSelLevel"), 0, TYPE_ENUM, ePolySelLevel,
	epfn_paintdeform_commit, _T("CommitPaintDeform"), 0, TYPE_VOID, 0, 0,
	epfn_paintdeform_cancel, _T("CancelPaintDeform"), 0, TYPE_VOID, 0, 0,


	epfn_get_cache_systemoff, _T("GetCache_SystemOn"), 0, TYPE_bool, 0, 0,
	epfn_set_cache_systemon, _T("SetCache_SystemOn"), 0, TYPE_VOID, 0, 1,
		_T("on"), 0, TYPE_bool,

	epfn_get_cache_suspend_dxcache, _T("GetCache_SuspendDXCache"), 0, TYPE_bool, 0, 0,
	epfn_set_cache_suspend_dxcache, _T("SetCache_SuspendDXCache"), 0, TYPE_VOID, 0, 1,
		_T("suspend"), 0, TYPE_bool,

	epfn_get_cache_cutoff_count, _T("GetCache_Cutoff"), 0, TYPE_INT, 0, 0,
	epfn_set_cache_cutoff_count, _T("SetCache_Cutoff"), 0, TYPE_VOID, 0, 1,
		_T("count"), 0, TYPE_INT,

	enums,
		epolyEnumButtonOps, 51,
			_T("GrowSelection"), epop_sel_grow,
			_T("ShrinkSelection"), epop_sel_shrink,
			_T("SelectEdgeLoop"), epop_select_loop,
			_T("SelectEdgeRing"), epop_select_ring,
			_T("HideSelection"), epop_hide,
			_T("HideUnselected"), epop_hide_unsel,
			_T("UnhideAll"), epop_unhide,
			_T("NamedSelectionCopy"), epop_ns_copy,
			_T("NamedSelectionPaste"), epop_ns_paste,
			_T("Cap"), epop_cap,
			_T("Delete"), epop_delete,
			_T("Remove"), epop_remove,
			_T("Detach"), epop_detach,
			_T("AttachList"), epop_attach_list,
			_T("SplitEdges"), epop_split,
			_T("BreakVertex"), epop_break,
			_T("Collapse"), epop_collapse,
			_T("ResetSlicePlane"), epop_reset_plane,
			_T("Slice"), epop_slice,
			_T("WeldSelected"), epop_weld_sel,
			_T("CreateShape"), epop_create_shape,
			_T("MakePlanar"), epop_make_planar,
			_T("AlignGrid"), epop_align_grid,
			_T("AlignView"), epop_align_view,
			_T("RemoveIsoVerts"), epop_remove_iso_verts,
			_T("MeshSmooth"), epop_meshsmooth,
			_T("Tessellate"), epop_tessellate,
			_T("Update"), epop_update,
			_T("SelectByVertexColor"), epop_selby_vc,
			_T("Retriangulate"), epop_retriangulate,
			_T("FlipNormals"), epop_flip_normals,
			_T("SelectByMatID"), epop_selby_matid,
			_T("SelectBySmoothingGroups"), epop_selby_smg,
			_T("Autosmooth"), epop_autosmooth,
			_T("ClearSmoothingGroups"), epop_clear_smg,
			_T("Extrude"), epop_extrude,
			_T("Bevel"), epop_bevel,
			_T("Inset"), epop_inset,
			_T("Outline"), epop_outline,
			_T("ExtrudeAlongSpline"), epop_extrude_along_spline,
			_T("HingeFromEdge"), epop_lift_from_edge,
			_T("ConnectEdges"), epop_connect_edges,
			_T("ConnectVertices"), epop_connect_vertices,
			_T("Chamfer"), epop_chamfer,
			_T("Cut"), epop_cut,
			_T("RemoveIsoMapVerts"), epop_remove_iso_map_verts,
			_T("ToggleShadedFaces"), epop_toggle_shaded_faces,
			_T("MakePlanarInX"), epop_make_planar_x,
			_T("MakePlanarInY"), epop_make_planar_y,
			_T("MakePlanarInZ"), epop_make_planar_z,
			_T("Relax"), epop_relax,

		epolyEnumCommandModes, 22,
			_T("CreateVertex"), epmode_create_vertex,
			_T("CreateEdge"), epmode_create_edge,
			_T("CreateFace"), epmode_create_face,
			_T("DivideEdge"), epmode_divide_edge,
			_T("DivideFace"), epmode_divide_face,
			_T("ExtrudeVertex"), epmode_extrude_vertex,
			_T("ExtrudeEdge"), epmode_extrude_edge,
			_T("ExtrudeFace"), epmode_extrude_face,
			_T("ChamferVertex"), epmode_chamfer_vertex,
			_T("ChamferEdge"), epmode_chamfer_edge,
			_T("Bevel"), epmode_bevel, 
			_T("SlicePlane"), epmode_sliceplane,
			_T("CutVertex"), epmode_cut_vertex,
			_T("CutEdge"), epmode_cut_edge,
			_T("CutFace"), epmode_cut_face,
			_T("Weld"), epmode_weld,
			_T("EditTriangulation"), epmode_edit_tri,
			_T("InsetFace"), epmode_inset_face,
			_T("QuickSlice"), epmode_quickslice,
			_T("HingeFromEdge"), epmode_lift_from_edge,
			_T("PickLiftEdge"), epmode_pick_lift_edge,
			_T("OutlineFace"), epmode_outline,

		epolyEnumPickModes, 2,
			_T("Attach"),	epmode_attach, 
			_T("PickShape"),	epmode_pick_shape,

		ePolySelLevel, 7,
			_T("Object"), EP_SL_OBJECT,
			_T("Vertex"), EP_SL_VERTEX,
			_T("Edge"), EP_SL_EDGE,
			_T("Border"), EP_SL_BORDER,
			_T("Face"), EP_SL_FACE,
			_T("Element"), EP_SL_ELEMENT,
			_T("CurrentLevel"), EP_SL_CURRENT,

		PMeshSelLevel, 5,
			_T("Object"), MNM_SL_OBJECT,
			_T("Vertex"), MNM_SL_VERTEX,
			_T("Edge"), MNM_SL_EDGE,
			_T("Face"), MNM_SL_FACE,
			_T("CurrentLevel"), MNM_SL_CURRENT,

		axisEnum, 3,
			_T("X"), 0,
			_T("Y"), 1,
			_T("Z"), 2,

	end
);

FPInterfaceDesc *EPoly::GetDesc () {
	return &epfi;
}

void *EditablePolyObjectClassDesc::Create (BOOL loading) {
	AddInterface (&epfi);
	return new EditPolyObject;
}

// This is very useful for macrorecording.

TCHAR *LookupOperationName (int opID) {
	int count = epfi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (epfi.enumerations[i]->ID == epolyEnumButtonOps) break;
	if (i==count) return NULL;

	FPEnum *fpenum = epfi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == opID) return fpenum->enumeration[i].name;
	}

	return NULL;
}

TCHAR *LookupEditablePolySelLevel (int selLevel) {
	int count = epfi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (epfi.enumerations[i]->ID == ePolySelLevel) break;
	if (i==count) return NULL;

	FPEnum *fpenum = epfi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == selLevel) return fpenum->enumeration[i].name;
	}

	return NULL;
}

TCHAR *LookupMNMeshSelLevel (int selLevel) {
	int count = epfi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (epfi.enumerations[i]->ID == PMeshSelLevel) break;
	if (i==count) return NULL;

	FPEnum *fpenum = epfi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == selLevel) return fpenum->enumeration[i].name;
	}

	return NULL;
}

class EditPolyObjectPBAccessor : public PBAccessor {
	public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
		{ ((EditPolyObject*)owner)->OnParamSet( v, id, tabIndex, t ); }
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
		{ ((EditPolyObject*)owner)->OnParamGet( v, id, tabIndex, t, valid ); }
};
static EditPolyObjectPBAccessor theEditPolyObjectPBAccessor;

class EditPolyConstraintValidatorClass : public PBValidator
{
	private:
		BOOL Validate(PB2Value &v)
		{
			if(v.i<0||v.i>3)
				return FALSE;
			return TRUE;
		};
	BOOL Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex)
	{ 
		if(v.i<0||v.i>3)
			return FALSE;
		int last;
		((EditPolyObject*)owner)->getParamBlock()->GetValue(ep_constrain_type, TimeValue(0), last, FOREVER);
		((EditPolyObject*)owner)->SetLastConstrainType(last);
		return this->Validate(v); 
	}

};

static EditPolyConstraintValidatorClass theEPConstraintValidator;


// Subobject levels for new (4.0) interface:
static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Edge(2);
static GenSubObjType SOT_Border(9);
static GenSubObjType SOT_Poly(4);
static GenSubObjType SOT_Element(5);

// Parameter block 2 contains all Editable parameters:
 
static ParamBlockDesc2 ep_param_block ( ep_block, _T("ePolyParameters"),
									   0, GetEditablePolyDesc(),
									   P_AUTO_CONSTRUCT|P_AUTO_UI|P_MULTIMAP,
									   0,
    // map rollups
	// (Note that modal or modeless floating dialogs are not included in this list.)
	// Task 748Y: UI redesign
	15,	ep_select, IDD_EP_SELECT, IDS_SELECT, 0, 0, NULL,
		ep_softsel, IDD_EP_SOFTSEL, IDS_SOFTSEL, 0, APPENDROLL_CLOSED, NULL,
		ep_geom, IDD_EP_GEOM, IDS_EDIT_GEOM, 0, 0, NULL,
		ep_geom_vertex, IDD_EP_GEOM_VERTEX, IDS_EDIT_VERTICES, 0, 0, NULL,
		ep_geom_edge, IDD_EP_GEOM_EDGE, IDS_EDIT_EDGES, 0, 0, NULL,
		ep_geom_border, IDD_EP_GEOM_BORDER, IDS_EDIT_BORDERS, 0, 0, NULL,
		ep_geom_face, IDD_EP_GEOM_FACE, IDS_EDIT_FACES, 0, 0, NULL,
		ep_geom_element, IDD_EP_GEOM_ELEMENT, IDS_EDIT_ELEMENTS, 0, 0, NULL,
		ep_subdivision, IDD_EP_MESHSMOOTH, IDS_SUBDIVISION_SURFACE, 0, 0, NULL,
		ep_displacement, IDD_EP_DISP_APPROX, IDS_DISPLACEMENT, 0, 0, NULL,
		ep_vertex, IDD_EP_SURF_VERT, IDS_VERTEX_PROPERTIES, 0, 0, NULL,
		ep_face, IDD_EP_SURF_FACE, IDS_EP_POLY_MATERIALIDS, 0, 0, NULL,
		ep_face_smoothing, IDD_EP_POLY_SMOOTHING, IDS_EP_POLY_SMOOTHING, 0, 0, NULL,
		ep_face_color, IDD_EP_POLY_COLOR , IDS_EP_POLY_COLOR, 0, 0, NULL,
		ep_paintdeform, IDD_EP_PAINTDEFORM, IDS_PAINTDEFORM, 0, 0, NULL,

	ep_by_vertex, _T("selByVertex"), TYPE_BOOL, P_TRANSIENT, IDS_SEL_BY_VERTEX,
		p_default, FALSE,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_SEL_BYVERT,
		end,

	ep_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_TRANSIENT, IDS_IGNORE_BACKFACING,
		p_default, FALSE,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_IGNORE_BACKFACES,
		end,

	ep_select_by_angle, _T("selectByAngle"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SELECT_BY_ANGLE,
		p_default, false,
		p_enable_ctrls, 1, ep_select_angle,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_SELECT_BY_ANGLE,
		end,

	ep_select_angle, _T("selectAngle"), TYPE_ANGLE, P_RESET_DEFAULT, IDS_SELECT_ANGLE,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_select, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SELECT_ANGLE, IDC_SELECT_ANGLE_SPIN, SPIN_AUTOSCALE,
		end,

	ep_ss_use, _T("useSoftSel"), TYPE_BOOL, 0, IDS_USE_SOFTSEL,
		p_default, FALSE,
		//p_enable_ctrls, 4, ep_ss_affect_back,
		//	ep_ss_falloff, ep_ss_pinch, ep_ss_bubble,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	ep_ss_edist_use, _T("ssUseEdgeDist"), TYPE_BOOL, 0, IDS_USE_EDIST,
		p_default, FALSE,
		p_enable_ctrls, 1, ep_ss_edist,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_USE_EDIST,
		end,

	ep_ss_edist, _T("ssEdgeDist"), TYPE_INT, 0, IDS_EDGE_DIST,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_EDIST, IDC_EDIST_SPIN, .2f,
		end,

	ep_ss_affect_back, _T("affectBackfacing"), TYPE_BOOL, 0, IDS_AFFECT_BACKFACING,
		p_default, TRUE,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_AFFECT_BACKFACING,
		end,

	ep_ss_falloff, _T("falloff"), TYPE_WORLD, P_ANIMATABLE, IDS_FALLOFF,
		p_default, 20.0f,
		p_range, 0.0f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_FALLOFF, IDC_FALLOFFSPIN, SPIN_AUTOSCALE,
		end,

	ep_ss_pinch, _T("pinch"), TYPE_WORLD, P_ANIMATABLE, IDS_PINCH,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PINCH, IDC_PINCHSPIN, SPIN_AUTOSCALE,
		end,

	ep_ss_bubble, _T("bubble"), TYPE_WORLD, P_ANIMATABLE, IDS_BUBBLE,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BUBBLE, IDC_BUBBLESPIN, SPIN_AUTOSCALE,
		end,

	ep_ss_lock, _T("lockSoftSel"), TYPE_BOOL, P_TRANSIENT, IDS_LOCK_SOFTSEL,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_LOCK_SOFTSEL,
		end,

	ep_paintsel_mode, _T("paintSelMode"), TYPE_INT, P_TRANSIENT, IDS_PAINTSEL_MODE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_default, PAINTMODE_OFF,
		end,

	ep_paintsel_value, _T("paintSelValue"), TYPE_FLOAT, 0, IDS_PAINTSEL_VALUE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_VALUE_EDIT, IDC_PAINTSEL_VALUE_SPIN, 0.01f,
		end,

	ep_paintsel_size, _T("paintSelSize"), TYPE_WORLD, P_TRANSIENT, IDS_PAINTSEL_SIZE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_range, 0.0f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTSEL_SIZE_EDIT, IDC_PAINTSEL_SIZE_SPIN, SPIN_AUTOSCALE,
		end,

	ep_paintsel_strength, _T("paintSelStrength"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTSEL_STRENGTH,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_range, 0.0f, 1.0f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_STRENGTH_EDIT, IDC_PAINTSEL_STRENGTH_SPIN, 0.01f,
		end,

	ep_constrain_type, _T("constrainType"), TYPE_INT, 0, IDS_CONSTRAIN_TYPE,
		p_default, 0,
		p_range,  0, 3,
		p_accessor, &theEditPolyObjectPBAccessor,
		p_validator, &theEPConstraintValidator,

		end,

	ep_delete_isolated_verts, _T("deleteIsolatedVerts"), TYPE_BOOL, P_TRANSIENT, IDS_DELETE_ISOLATED_VERTS,
		p_default, TRUE,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_DELETE_ISOLATED_VERTS,
		end,

	ep_interactive_full, _T("fullyInteractive"), TYPE_BOOL, 0, IDS_FULLY_INTERACTIVE,
		p_default, true,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_INTERACTIVE_FULL,
		end,

	ep_show_cage, _T("showCage"), TYPE_BOOL, 0, IDS_SHOW_CAGE,
		p_default, true,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_SHOW_CAGE,
		end,

	ep_split, _T("split"), TYPE_BOOL, P_TRANSIENT, IDS_SPLIT,
		p_default, FALSE,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_SPLIT,
		end,

	ep_preserve_maps, _T("preserveUVs"), TYPE_BOOL, 0, IDS_PRESERVE_MAPS,
		p_default, FALSE,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_PRESERVE_MAPS,
		end,

	ep_surf_subdivide, _T("surfSubdivide"), TYPE_BOOL, 0, IDS_SUBDIVIDE,
		p_default, FALSE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_USE_NURMS,
		end,

	ep_surf_subdiv_smooth, _T("subdivSmoothing"), TYPE_BOOL, 0, IDS_SMOOTH_SUBDIV,
		p_default, TRUE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_SMOOTH_SUBDIV,
		end,

	ep_surf_isoline_display, _T("isolineDisplay"), TYPE_BOOL, 0, IDS_ISOLINE_DISPLAY,	// CAL-03/20/03:
		p_default, TRUE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_ISOLINE_DISPLAY,
		end,

	ep_surf_iter, _T("iterations"), TYPE_INT, P_ANIMATABLE, IDS_ITER,
		p_default, 1,
		p_range, 0, 10,
		p_ui, ep_subdivision, TYPE_SPINNER, EDITTYPE_POS_INT, 
			IDC_ITER, IDC_ITERSPIN, .2f,
		end,

	ep_surf_thresh, _T("surfaceSmoothness"), TYPE_FLOAT, P_ANIMATABLE, IDS_SHARP,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_subdivision, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SHARP, IDC_SHARPSPIN, .002f,
		end,

#ifndef NO_OUTPUTRENDERER
	ep_surf_use_riter, _T("useRenderIterations"), TYPE_BOOL, 0, IDS_USE_RITER,
		p_default, FALSE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_USE_RITER,
		p_enable_ctrls, 1, ep_surf_riter,
		end,

	ep_surf_riter, _T("renderIterations"), TYPE_INT, P_ANIMATABLE, IDS_RITER,
		p_default, 0,
		p_range, 0, 10,
		p_ui, ep_subdivision, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_R_ITER, IDC_R_ITERSPIN, .2f,
		end,

	ep_surf_use_rthresh, _T("useRenderSmoothness"), TYPE_BOOL, 0, IDS_USE_RSHARP,
		p_default, FALSE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_USE_RSHARP,
		p_enable_ctrls, 1, ep_surf_rthresh,
		end,

	ep_surf_rthresh, _T("renderSmoothness"), TYPE_FLOAT, P_ANIMATABLE, IDS_RSHARP,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_subdivision, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_R_SHARP, IDC_R_SHARPSPIN, .002f,
		end,
#endif // NO_OUTPUTRENDERER

	ep_surf_sep_mat, _T("sepByMats"), TYPE_BOOL, 0, IDS_MS_SEP_BY_MATID,
		p_default, FALSE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_MATID,
		end,

	ep_surf_sep_smooth, _T("sepBySmGroups"), TYPE_BOOL, 0, IDS_MS_SEP_BY_SMOOTH,
		p_default, FALSE,
		p_ui, ep_subdivision, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_SMOOTH,
		end,

	ep_surf_update, _T("update"), TYPE_INT, 0, IDS_UPDATE_OPTIONS,
		p_default, 0, // Update Always.
		p_range, 0, 2,
		p_ui, ep_subdivision, TYPE_RADIO, 3,
			IDC_UPDATE_ALWAYS, IDC_UPDATE_RENDER, IDC_UPDATE_MANUAL,
		end,

	ep_vert_sel_color, _T("vertSelectionColor"), TYPE_RGBA, P_TRANSIENT, IDS_VERT_SELECTION_COLOR,
		p_default, Point3(1,1,1),
		p_ui, ep_vertex, TYPE_COLORSWATCH, IDC_VERT_SELCOLOR,
		end,

	ep_vert_color_selby, _T("vertSelectBy"), TYPE_INT, P_TRANSIENT, IDS_VERT_SELECT_BY,
		p_default, 0,
		p_ui, ep_vertex, TYPE_RADIO, 2, IDC_SEL_BY_COLOR, IDC_SEL_BY_ILLUM,
		end,

	ep_vert_selc_r, _T("vertSelectionRedRange"), TYPE_INT, P_TRANSIENT, IDS_VERT_SELECTION_RED_RANGE,
		p_default, 10,
		p_range, 0, 255,
		p_ui, ep_vertex, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_VERT_SELR, IDC_VERT_SELRSPIN, .5f,
		end,

	ep_vert_selc_g, _T("vertSelectionGreenRange"), TYPE_INT, P_TRANSIENT, IDS_VERT_SELECTION_GREEN_RANGE,
		p_default, 10,
		p_range, 0, 255,
		p_ui, ep_vertex, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_VERT_SELG, IDC_VERT_SELGSPIN, .5f,
		end,

	ep_vert_selc_b, _T("vertSelectionBlueRange"), TYPE_INT, P_TRANSIENT, IDS_VERT_SELECTION_BLUE_RANGE,
		p_default, 10,
		p_range, 0, 255,
		p_ui, ep_vertex, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_VERT_SELB, IDC_VERT_SELBSPIN, .5f,
		end,

	ep_face_smooth_thresh, _T("autoSmoothThreshold"), TYPE_ANGLE, P_TRANSIENT, IDS_FACE_AUTOSMOOTH_THRESH,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_face_smoothing, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SMOOTH_THRESH, IDC_SMOOTH_THRESHSPIN, SPIN_AUTOSCALE,
		end,

#ifndef WEBVERSION
	ep_sd_use, _T("useSubdivisionDisplacement"), TYPE_BOOL, 0, IDS_USE_DISPLACEMENT,
		p_default, false,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_ENGAGE,
		end,

	ep_sd_split_mesh, _T("displaceSplitMesh"), TYPE_BOOL, 0, IDS_SD_SPLITMESH,
		p_default, true,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_SPLITMESH,
		end,

	ep_sd_method, _T("displaceMethod"), TYPE_INT, 0, IDS_SD_METHOD,
		p_default, 3,
		p_ui, ep_displacement, TYPE_RADIO, 4, IDC_SD_TESS_REGULAR,
			IDC_SD_TESS_SPATIAL, IDC_SD_TESS_CURV, IDC_SD_TESS_LDA,
		end,

	ep_sd_tess_steps, _T("displaceSteps"), TYPE_INT, P_ANIMATABLE, IDS_SD_TESS_STEPS,
		p_default, 2,
		p_range, 1, 100,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_SD_TESS_U, IDC_SD_TESS_U_SPINNER, .5f,
		end,

	ep_sd_tess_edge, _T("displaceEdge"), TYPE_FLOAT, P_ANIMATABLE, IDS_SD_TESS_EDGE,
		p_default, 20.0f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_EDGE, IDC_SD_TESS_EDGE_SPINNER, .1f,
		end,

	ep_sd_tess_distance, _T("displaceDistance"), TYPE_FLOAT, P_ANIMATABLE, IDS_SD_TESS_DISTANCE,
		p_default, 20.0f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_DIST, IDC_SD_TESS_DIST_SPINNER, .1f,
		end,

	ep_sd_tess_angle, _T("displaceAngle"), TYPE_ANGLE, P_ANIMATABLE, IDS_SD_TESS_ANGLE,
		p_default, PI/18.0f,	// 10 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_ANG, IDC_SD_TESS_ANG_SPINNER, .1f,
		end,

	ep_sd_view_dependent, _T("viewDependent"), TYPE_BOOL, 0, IDS_SD_VIEW_DEPENDENT,
		p_default, false,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_VIEW_DEP,
		end,
#else // WEBVERSION
	ep_sd_use, _T(""), TYPE_BOOL, 0, IDS_USE_DISPLACEMENT,
		p_default, false,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_ENGAGE,
		end,

	ep_sd_split_mesh, _T(""), TYPE_BOOL, 0, IDS_SD_SPLITMESH,
		p_default, true,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_SPLITMESH,
		end,

	ep_sd_method, _T(""), TYPE_INT, 0, IDS_SD_METHOD,
		p_default, 3,
		p_ui, ep_displacement, TYPE_RADIO, 4, IDC_SD_TESS_REGULAR,
			IDC_SD_TESS_SPATIAL, IDC_SD_TESS_CURV, IDC_SD_TESS_LDA,
		end,

	ep_sd_tess_steps, _T(""), TYPE_INT, P_ANIMATABLE, IDS_SD_TESS_STEPS,
		p_default, 2,
		p_range, 0, 100,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_SD_TESS_U, IDC_SD_TESS_U_SPINNER, .5f,
		end,

	ep_sd_tess_edge, _T(""), TYPE_FLOAT, P_ANIMATABLE, IDS_SD_TESS_EDGE,
		p_default, 20.0f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_EDGE, IDC_SD_TESS_EDGE_SPINNER, .1f,
		end,

	ep_sd_tess_distance, _T(""), TYPE_FLOAT, P_ANIMATABLE, IDS_SD_TESS_DISTANCE,
		p_default, 20.0f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_DIST, IDC_SD_TESS_DIST_SPINNER, .1f,
		end,

	ep_sd_tess_angle, _T(""), TYPE_ANGLE, P_ANIMATABLE, IDS_SD_TESS_ANGLE,
		p_default, PI/18.0f,	// 10 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_displacement, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_SD_TESS_ANG, IDC_SD_TESS_ANG_SPINNER, .1f,
		end,

	ep_sd_view_dependent, _T(""), TYPE_BOOL, 0, IDS_SD_VIEW_DEPENDENT,
		p_default, false,
		p_ui, ep_displacement, TYPE_SINGLECHEKBOX, IDC_SD_VIEW_DEP,
		end,


#endif
	ep_asd_style, _T("displaceStyle"), TYPE_INT, 0, IDS_SD_DISPLACE_STYLE,
		p_default, 1,
		p_ui, ep_advanced_displacement, TYPE_RADIO, 3, IDC_ASD_GRID,
			IDC_ASD_TREE, IDC_ASD_DELAUNAY,
		end,

	ep_asd_min_iters, _T("displaceMinLevels"), TYPE_INT, 0, IDS_SD_MIN_LEVELS,
		p_default, 0,
		p_range, 0, MAX_SUBDIV,
		p_ui, ep_advanced_displacement, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_ASD_MIN_ITERS, IDC_ASD_MIN_ITERS_SPIN, .5f,
		end,

	ep_asd_max_iters, _T("displaceMaxLevels"), TYPE_INT, 0, IDS_SD_MAX_LEVELS,
		p_default, 2,
		p_range, 0, MAX_SUBDIV,
		p_ui, ep_advanced_displacement, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_ASD_MAX_ITERS, IDC_ASD_MAX_ITERS_SPIN, .5f,
		end,

	ep_asd_max_tris, _T("displaceMaxTris"), TYPE_INT, 0, IDS_SD_MAX_TRIS,
		p_default, 20000,
		p_range, 0, 99999999,
		p_ui, ep_advanced_displacement, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_ASD_MAX_TRIS, IDC_ASD_MAX_TRIS_SPIN, .5f,
		end,

	ep_extrusion_type, _T("extrusionType"), TYPE_INT, 0, IDS_EXTRUSION_TYPE,
		p_default, 0,	// Group normal
		p_ui, ep_settings, TYPE_RADIO, 3,
			IDC_EXTYPE_GROUP, IDC_EXTYPE_LOCAL, IDC_EXTYPE_BY_POLY,
		end,

	ep_bevel_type, _T("bevelType"), TYPE_INT, 0, IDS_BEVEL_TYPE,
		p_default, 0,	// Group normal
		p_ui, ep_settings, TYPE_RADIO, 3,
			IDC_BEVTYPE_GROUP, IDC_BEVTYPE_LOCAL, IDC_BEVTYPE_BY_POLY,
		end,

	ep_face_extrude_height, _T("faceExtrudeHeight"), TYPE_WORLD, P_TRANSIENT, IDS_FACE_EXTRUDE_HEIGHT,
		p_default, 10.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_FACE_EXTRUDE_HEIGHT, IDC_FACE_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	ep_vertex_extrude_height, _T("vertexExtrudeHeight"), TYPE_WORLD, P_TRANSIENT, IDS_VERTEX_EXTRUDE_HEIGHT,
		p_default, 10.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_VERTEX_EXTRUDE_HEIGHT, IDC_VERTEX_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	ep_edge_extrude_height, _T("edgeExtrudeHeight"), TYPE_WORLD, P_TRANSIENT, IDS_EDGE_EXTRUDE_HEIGHT,
		p_default, 10.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_EDGE_EXTRUDE_HEIGHT, IDC_EDGE_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	ep_vertex_extrude_width, _T("vertexExtrudeWidth"), TYPE_FLOAT, P_TRANSIENT, IDS_VERTEX_EXTRUDE_WIDTH,
		p_default, 3.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTEX_EXTRUDE_WIDTH, IDC_VERTEX_EXTRUDE_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	ep_edge_extrude_width, _T("edgeExtrudeWidth"), TYPE_FLOAT, P_TRANSIENT, IDS_EDGE_EXTRUDE_WIDTH,
		p_default, 3.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGE_EXTRUDE_WIDTH, IDC_EDGE_EXTRUDE_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	ep_bevel_height, _T("bevelHeight"), TYPE_WORLD, P_TRANSIENT, IDS_BEVEL_HEIGHT,
		p_default, 10.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_BEVEL_HEIGHT, IDC_BEVEL_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	ep_bevel_outline, _T("bevelOutline"), TYPE_WORLD, P_TRANSIENT, IDS_BEVEL_OUTLINE,
		p_default, -1.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_BEVEL_OUTLINE, IDC_BEVEL_OUTLINE_SPIN, SPIN_AUTOSCALE,
		end,

	ep_outline, _T("outlineAmount"), TYPE_WORLD, P_TRANSIENT, IDS_OUTLINE_AMOUNT,
		p_default, -1.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_OUTLINEAMOUNT, IDC_OUTLINESPINNER, SPIN_AUTOSCALE,
		end,

	ep_inset, _T("insetAmount"), TYPE_WORLD, P_TRANSIENT, IDS_INSET_AMOUNT,
		p_default, 1.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_INSETAMOUNT, IDC_INSETSPINNER, SPIN_AUTOSCALE,
		end,

	ep_inset_type, _T("insetType"), TYPE_INT, 0, IDS_INSET_TYPE,
		p_default, 0,	// Inset by cluster
		p_ui, ep_settings, TYPE_RADIO, 2,
			IDC_INSETTYPE_GROUP, IDC_INSETTYPE_BY_POLY,
		end,

	ep_vertex_chamfer, _T("vertexChamfer"), TYPE_WORLD, P_TRANSIENT, IDS_VERTEX_CHAMFER_AMOUNT,
		p_default, 1.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTEX_CHAMFER, IDC_VERTEX_CHAMFER_SPIN, SPIN_AUTOSCALE,
		end,

	ep_edge_chamfer, _T("edgeChamfer"), TYPE_WORLD, P_TRANSIENT, IDS_EDGE_CHAMFER_AMOUNT,
		p_default, 1.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGE_CHAMFER, IDC_EDGE_CHAMFER_SPIN, SPIN_AUTOSCALE,
		end,

	ep_edge_chamfer_segments, _T("edgeChamferSegments"), TYPE_INT, P_TRANSIENT, IDS_EDGE_CHAMFER_SEGMENTS,
		p_default, 1,
		p_range, 1, 999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
		IDC_EDGE_CHAMFER_SEGMENTS, IDC_EDGE_CHAMFER_SEGMENTS_SPIN, .5f,
		end,

	ep_vertex_break, _T("vertexBreak"), TYPE_WORLD, P_TRANSIENT, IDS_VERTEX_BREAK_AMOUNT,
		p_default, 0.0f,
		p_range, -1.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
		IDC_VERTEX_BREAK, IDC_VERTEX_BREAK_SPIN, SPIN_AUTOSCALE,
		end,

	ep_weld_threshold, _T("weldThreshold"), TYPE_WORLD, P_TRANSIENT, IDS_WELD_THRESHOLD,
		p_default, .1f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTWELD_THRESH, IDC_VERTWELD_THRESH_SPIN, SPIN_AUTOSCALE,
		end,

	ep_edge_weld_threshold, _T("edgeWeldThreshold"), TYPE_WORLD, P_TRANSIENT, IDS_EDGE_WELD_THRESHOLD,
		p_default, .1f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGEWELD_THRESH, IDC_EDGEWELD_THRESH_SPIN, SPIN_AUTOSCALE,
		end,

	ep_weld_pixels, _T("weldPixels"), TYPE_INT, P_TRANSIENT, IDS_WELD_PIXELS,
		p_default, 4,
		// Not currently used or included in UI.
		// Need to keep it here for 4.2 compatibility?
		//p_ui, ep_geom, TYPE_SPINNER, EDITTYPE_POS_INT,
			//IDC_T_THR, IDC_T_THR_SPIN, .5f,
		end,

	ep_ms_smoothness, _T("smoothness"), TYPE_FLOAT, P_TRANSIENT, IDS_MS_SMOOTHNESS,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_MS_SMOOTH, IDC_MS_SMOOTHSPIN, .001f,
		end,

	ep_ms_sep_smooth, _T("separateBySmoothing"), TYPE_BOOL, P_TRANSIENT, IDS_MS_SEP_BY_SMOOTH,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_SMOOTH,
		end,

	ep_ms_sep_mat, _T("separateByMaterial"), TYPE_BOOL, P_TRANSIENT, IDS_MS_SEP_BY_MATID,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_MATID,
		end,

	ep_tess_type, _T("tesselateBy"), TYPE_INT, P_TRANSIENT, IDS_TESS_BY,
		p_default, 0,
		p_ui, ep_settings, TYPE_RADIO, 2, IDC_TESS_EDGE, IDC_TESS_FACE,
		end,

	ep_tess_tension, _T("tessTension"), TYPE_FLOAT, P_TRANSIENT, IDS_TESS_TENSION,
		p_default, 0.0f,
		p_range, -9999999.9f, 9999999.9f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_TESS_TENSION, IDC_TESS_TENSIONSPIN, .01f,
		end,

	ep_connect_edge_segments, _T("connectEdgeSegments"), TYPE_INT, P_TRANSIENT, IDS_CONNECT_EDGE_SEGMENTS,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_CONNECT_EDGE_SEGMENTS, IDC_CONNECT_EDGE_SEGMENTS_SPIN, .5f,
		end,

	ep_extrude_spline_node, _T("extrudeAlongSplineNode"), TYPE_INODE, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_NODE,
		end,

	ep_extrude_spline_segments, _T("extrudeAlongSplineSegments"), TYPE_INT, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_SEGMENTS,
		p_default, 6,
		p_range, 1, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_EXTRUDE_SEGMENTS, IDC_EXTRUDE_SEGMENTS_SPIN, .01f,
		end,

	ep_extrude_spline_taper, _T("extrudeAlongSplineTaper"), TYPE_FLOAT, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_TAPER,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TAPER, IDC_EXTRUDE_TAPER_SPIN, .01f,
		end,
	
	ep_extrude_spline_taper_curve, _T("extrudeAlongSplineTaperCurve"), TYPE_FLOAT, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_TAPER_CURVE,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TAPER_CURVE, IDC_EXTRUDE_TAPER_CURVE_SPIN, .01f,
		end,
	
	ep_extrude_spline_twist, _T("extrudeAlongSplineTwist"), TYPE_ANGLE, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_TWIST,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -3600.0f, 3600.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TWIST, IDC_EXTRUDE_TWIST_SPIN, .5f,
		end,

	ep_extrude_spline_rotation, _T("extrudeAlongSplineRotation"), TYPE_ANGLE, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_ROTATION,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -360.f, 360.f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_ROTATION, IDC_EXTRUDE_ROTATION_SPIN, .5f,
		end,

	ep_extrude_spline_align, _T("extrudeAlongSplineAlign"), TYPE_BOOL, P_TRANSIENT, IDS_EXTRUDE_ALONG_SPLINE_ALIGN,
		p_default, 0,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_EXTRUDE_ALIGN_NORMAL,
		p_enable_ctrls, 1, ep_extrude_spline_rotation,
		end,

	ep_lift_angle, _T("hingeAngle"), TYPE_ANGLE, P_TRANSIENT, IDS_LIFT_FROM_EDGE_ANGLE,
		p_default, PI/6.0f,	// 30 degrees.
		p_dim, stdAngleDim,
		p_range, -360.0f, 360.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_LIFT_ANGLE, IDC_LIFT_ANGLE_SPIN, .5f,
		end,

	ep_lift_edge, _T("hinge"), TYPE_INDEX, P_TRANSIENT, IDS_LIFT_EDGE,
		p_default, -1,
		end,

	ep_lift_segments, _T("hingeSegments"), TYPE_INT, P_TRANSIENT, IDS_LIFT_SEGMENTS,
		p_default, 1,
		p_range, 1, 1000,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_LIFT_SEGMENTS, IDC_LIFT_SEGMENTS_SPIN, .5f,
		end,

	ep_paintdeform_mode, _T("paintDeformMode"), TYPE_INT, P_TRANSIENT, IDS_PAINTSEL_MODE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_default, PAINTMODE_OFF,
		end,

	ep_paintdeform_value, _T("paintDeformValue"), TYPE_FLOAT, 0, IDS_PAINTDEFORM_VALUE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_default, 10.0f,
		p_range, -99999999.0f, 99999999.0f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTDEFORM_VALUE_EDIT, IDC_PAINTDEFORM_VALUE_SPIN, SPIN_AUTOSCALE,
		end,

	ep_paintdeform_size, _T("paintDeformSize"), TYPE_WORLD, P_TRANSIENT, IDS_PAINTDEFORM_SIZE,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_range, 0.0f, 999999.f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTDEFORM_SIZE_EDIT, IDC_PAINTDEFORM_SIZE_SPIN, SPIN_AUTOSCALE,
		end,

	ep_paintdeform_strength, _T("paintDeformStrength"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTDEFORM_STRENGTH,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_range, 0.0f, 1.0f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTDEFORM_STRENGTH_EDIT, IDC_PAINTDEFORM_STRENGTH_SPIN, 0.01f,
		end,

	ep_paintdeform_axis, _T("paintDeformAxis"), TYPE_INT, 0, IDS_PAINTDEFORM_AXIS,
		p_accessor,		&theEditPolyObjectPBAccessor,
		p_default, PAINTAXIS_ORIGNORMS,
		p_ui, ep_paintdeform, TYPE_RADIO, 5, IDC_PAINTAXIS_ORIGNORMS, IDC_PAINTAXIS_DEFORMEDNORMS,
			IDC_PAINTAXIS_XFORM_X, IDC_PAINTAXIS_XFORM_Y, IDC_PAINTAXIS_XFORM_Z,
		p_vals, 	PAINTAXIS_ORIGNORMS, PAINTAXIS_DEFORMEDNORMS,
			PAINTAXIS_XFORM_X, PAINTAXIS_XFORM_Y, PAINTAXIS_XFORM_Z,
		end,

	// Parameters with no UI:
	ep_cut_start_level, _T("cutStartLevel"), TYPE_INT, P_TRANSIENT, IDS_CUT_START_LEVEL,
		p_default, MNM_SL_VERTEX,
		end,

	ep_cut_start_index, _T("cutStartIndex"), TYPE_INDEX, P_TRANSIENT, IDS_CUT_START_INDEX,
		p_default, 0,
		end,

	ep_cut_start_coords, _T("cutStartCoords"), TYPE_POINT3, P_TRANSIENT, IDS_CUT_START_COORDS,
		p_default, Point3(0.0f,0.0f,0.0f),
		end,

	ep_cut_end_coords, _T("cutEndCoords"), TYPE_POINT3, P_TRANSIENT, IDS_CUT_END_COORDS,
		p_default, Point3(0.0f,0.0f,0.0f),
		end,

	ep_cut_normal, _T("cutNormal"), TYPE_POINT3, P_TRANSIENT, IDS_CUT_NORMAL,
		p_default, Point3(0.0f,0.0f,0.0f),
		end,

	ep_bridge_segments, _T("bridgeSegments"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_SEGMENTS,
		p_default, 1,
		p_range, 1, 999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_BRIDGE_SEGMENTS, IDC_BRIDGE_SEGMENTS_SPIN, .5f,
		end,

	ep_bridge_taper, _T("bridgeTaper"), TYPE_FLOAT, P_TRANSIENT, IDS_BRIDGE_TAPER,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BRIDGE_TAPER, IDC_BRIDGE_TAPER_SPIN, .02f,
		end,

	ep_bridge_bias, _T("bridgeBias"), TYPE_FLOAT, P_TRANSIENT, IDS_BRIDGE_BIAS,
		p_default, 0.0f,
		p_range, -99.0f, 99.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_BRIDGE_BIAS, IDC_BRIDGE_BIAS_SPIN, 1.0f,
		end,

	ep_bridge_smooth_thresh, _T("bridgeSmoothThreshold"), TYPE_ANGLE, P_TRANSIENT, IDS_BRIDGE_SMOOTH_THRESHOLD,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
		IDC_BRIDGE_THRESH, IDC_BRIDGE_THRESH_SPIN, .5f,
		end,

	ep_bridge_selected, _T("bridgeSelected"), TYPE_INT, 0, IDS_BRIDGE_SELECTED,
		p_default, true,
		p_range, 0, 1,
		p_ui, ep_settings, TYPE_RADIO, 2, IDC_BRIDGE_SPECIFIC, IDC_BRIDGE_SELECTED,
		end,

	ep_bridge_twist_1, _T("bridgeTwist1"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_TWIST_1,
		p_default, 0,
		p_range, -9999999, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_BRIDGE_TWIST1, IDC_BRIDGE_TWIST1_SPIN, .5f,
		end,

	ep_bridge_twist_2, _T("bridgeTwist2"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_TWIST_2,
		p_default, 0,
		p_range, -9999999, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_BRIDGE_TWIST2, IDC_BRIDGE_TWIST2_SPIN, .5f,
		end,

	ep_bridge_target_1, _T("bridgeTarget1"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_TARGET_1,
		p_default, 0,
		end,

	ep_bridge_target_2, _T("bridgeTarget2"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_TARGET_2,
		p_default, 0,
		end,

	ep_relax_amount, _T("relaxAmount"), TYPE_FLOAT, P_TRANSIENT, IDS_RELAX_AMOUNT,
		p_default, 0.5f,
		p_range, -1.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_RELAX_AMOUNT, IDC_RELAX_AMOUNT_SPIN, .01f,
		end,

	ep_relax_iters, _T("relaxIterations"), TYPE_INT, P_TRANSIENT, IDS_RELAX_ITERATIONS,
		p_default, 1,
		p_range, 1, 99999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_RELAX_ITERATIONS, IDC_RELAX_ITERATIONS_SPIN, .5f,
		end,

	ep_relax_hold_boundary, _T("relaxHoldBoundaryPoints"), TYPE_BOOL, P_TRANSIENT, IDS_RELAX_HOLD_BOUNDARY_POINTS,
		p_default, true,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_RELAX_HOLD_BOUNDARY,
		end,

	ep_relax_hold_outer, _T("relaxHoldOuterPoints"), TYPE_BOOL, P_TRANSIENT, IDS_RELAX_HOLD_OUTER_POINTS,
		p_default, false,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_RELAX_HOLD_OUTER,
		end,

	ep_vertex_chamfer_open, _T("vertexChamferOpen"), TYPE_BOOL, P_TRANSIENT, IDS_OPEN_CHAMFER,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_VERTEX_CHAMFER_OPEN,
		end,

	ep_edge_chamfer_open, _T("edgeChamferOpen"), TYPE_BOOL, P_TRANSIENT, IDS_OPEN_CHAMFER,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_EDGE_CHAMFER_OPEN,
		end,

	ep_bridge_adjacent, _T("bridgeAdjacentAngle"), TYPE_ANGLE, P_RESET_DEFAULT, IDS_BRIDGE_ADJACENT,
		p_default, PI/4.0f,	// 45 degrees.,
		p_range, 0.0f, 180.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_BRIDGE_ADJACENT_THRESH, IDC_BRIDGE_ADJACENT_SPIN, 0.5f,
		end,

	ep_bridge_reverse_triangle, _T("bridgeReverseTriangle"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_REVERSETRI,
		p_default, false,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_REVERSETRI,
		end,

	ep_connect_edge_pinch, _T("connectEdgePinch"), TYPE_INT, P_TRANSIENT, IDS_CONNECT_EDGE_PINCH,
		p_default, 0,
		p_range, -100, 100,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
		IDC_CONNECT_EDGE_PINCH, IDC_CONNECT_EDGE_PINCH_SPIN, .5f,
		end,

	ep_connect_edge_slide, _T("connectEdgeSlide"), TYPE_INT, P_TRANSIENT, IDS_CONNECT_EDGE_SLIDE,
		p_default, 0,
		p_range, -99999, 99999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
		IDC_CONNECT_EDGE_SLIDE, IDC_CONNECT_EDGE_SLIDE_SPIN, .5f,
		end,

	ep_cage_color, _T("cageColor"), TYPE_POINT3, P_TRANSIENT, IDS_CAGE_COLOR,
		p_default, 		GetUIColor(COLOR_GIZMOS), 
		p_ui, ep_subdivision, TYPE_COLORSWATCH, IDC_CAGE_COLOR_SWATCH,
		end,

	ep_selected_cage_color, _T("selectedCageColor"), TYPE_POINT3, P_TRANSIENT,IDS_SELECTED_CAGE_COLOR,
		p_default, 		GetUIColor(COLOR_SEL_GIZMOS), 
		p_ui, ep_subdivision, TYPE_COLORSWATCH, IDC_SELECTED_CAGE_COLOR_SWATCH,
		end,

	ep_select_mode, _T("selectMode"),TYPE_INT,0, IDS_SELECT_MODE,
		p_default, 0, 
		p_ui, ep_select,TYPE_RADIO,3,
		IDC_SELECT_NONE,IDC_SELECT_SUBOBJ,IDC_SELECT_MULTI,
		p_accessor,		&theEditPolyObjectPBAccessor,
		end,


	end
);

// -- Misc. Window procs ----------------------------------------

static int createCurveType   = IDC_CURVE_SMOOTH;

static INT_PTR CALLBACK CurveNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);
		name = (TSTR*)lParam;
		SetWindowText (GetDlgItem(hWnd,IDC_CURVE_NAME), name->data());
		SendMessage(GetDlgItem(hWnd,IDC_CURVE_NAME), EM_SETSEL,0,-1);			
		CheckDlgButton(hWnd,createCurveType,TRUE);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize(GetWindowTextLength(GetDlgItem(hWnd,IDC_CURVE_NAME))+1);
			GetWindowText(GetDlgItem(hWnd,IDC_CURVE_NAME), name->data(), name->length()+1);
			if (IsDlgButtonChecked(hWnd,IDC_CURVE_SMOOTH)) createCurveType = IDC_CURVE_SMOOTH;
			else createCurveType = IDC_CURVE_LINEAR;
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,1);
			break;
		
		case IDCANCEL:
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

bool GetCreateShapeName (Interface *ip, TSTR &name, bool &ccSmooth) {
	HWND hMax = ip->GetMAXHWnd();
	name = GetString(IDS_SHAPE);
	ip->MakeNameUnique (name);
	bool ret = DialogBoxParam (hInstance,
		MAKEINTRESOURCE(IDD_CREATECURVE),
		hMax, CurveNameDlgProc, (LPARAM)&name) ? true : false;
	ccSmooth = (createCurveType == IDC_CURVE_LINEAR) ? false : true;
	return ret;
}

static BOOL detachToElem = FALSE;
static BOOL detachAsClone = FALSE;

static void SetDetachNameState(HWND hWnd) {
	if (detachToElem) {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),FALSE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),TRUE);
	}
}

static INT_PTR CALLBACK DetachDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);
		name = (TSTR*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_DETACH_NAME), name->data());
		SendMessage(GetDlgItem(hWnd,IDC_DETACH_NAME), EM_SETSEL,0,-1);
		CheckDlgButton (hWnd, IDC_DETACH_ELEM, detachToElem);
		CheckDlgButton (hWnd, IDC_DETACH_CLONE, detachAsClone);
		if (detachToElem) SetFocus (GetDlgItem (hWnd, IDOK));
		else SetFocus (GetDlgItem (hWnd, IDC_DETACH_NAME));
		SetDetachNameState(hWnd);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_DETACH_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_DETACH_NAME),
				name->data(), name->length()+1);
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,1);
			break;

		case IDC_DETACH_ELEM:
			detachToElem = IsDlgButtonChecked(hWnd,IDC_DETACH_ELEM);
			SetDetachNameState(hWnd);
			break;

		case IDC_DETACH_CLONE:
			detachAsClone = IsDlgButtonChecked (hWnd, IDC_DETACH_CLONE);
			break;

		case IDCANCEL:
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

bool GetDetachObjectName (Interface *ip, TSTR &name, bool &elem, bool &clone) {
	HWND hMax = ip->GetMAXHWnd();
	name = GetString(IDS_OBJECT);
	ip->MakeNameUnique (name);
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DETACH),
			hMax, DetachDlgProc, (LPARAM)&name)) {
		elem = detachToElem ? true : false;
		clone = detachAsClone ? true : false;
		return true;
	} else {
		return false;
	}
}

static int cloneTo = IDC_CLONE_ELEM;

static void SetCloneNameState(HWND hWnd) {
	switch (cloneTo) {
	case IDC_CLONE_ELEM:
		EnableWindow(GetDlgItem(hWnd,IDC_CLONE_NAME),FALSE);
		break;
	case IDC_CLONE_OBJ:
		EnableWindow(GetDlgItem(hWnd,IDC_CLONE_NAME),TRUE);
		break;
	}
}

static INT_PTR CALLBACK CloneDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static TSTR *name = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);
		name = (TSTR*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_CLONE_NAME), name->data());

		CheckRadioButton (hWnd, IDC_CLONE_OBJ, IDC_CLONE_ELEM, cloneTo);
		if (cloneTo == IDC_CLONE_OBJ) {
			SetFocus(GetDlgItem(hWnd,IDC_CLONE_NAME));
			SendMessage(GetDlgItem(hWnd,IDC_CLONE_NAME), EM_SETSEL,0,-1);
		} else SetFocus (GetDlgItem (hWnd, IDOK));
		SetCloneNameState(hWnd);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name->Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_CLONE_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_CLONE_NAME),
				name->data(), name->length()+1);
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			thePopupPosition.Scan (hWnd);
			EndDialog (hWnd, 0);
			break;

		case IDC_CLONE_ELEM:
		case IDC_CLONE_OBJ:
			cloneTo = LOWORD(wParam);
			SetCloneNameState(hWnd);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

BOOL GetCloneObjectName (Interface *ip, TSTR &name) {
	HWND hMax = ip->GetMAXHWnd();
	name = GetString(IDS_OBJECT);
	if (ip) ip->MakeNameUnique (name);
	DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_CLONE), hMax, CloneDlgProc, (LPARAM)&name);
	return (cloneTo==IDC_CLONE_OBJ);
}

class SelectDlgProc : public ParamMap2UserDlgProc {
	EditPolyObject* mpEPoly;
	bool updateNumSel;

public:
	SelectDlgProc () : mpEPoly(NULL), updateNumSel(true) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void RefreshSelType (HWND hWnd);
	void SetEnables (HWND hWnd);
	void SetNumSelLabel (HWND hWnd);
	void SetEPoly (EditPolyObject *e) { mpEPoly=e; }
	void UpdateNumSel () { updateNumSel=true; }
};

static SelectDlgProc theSelectDlgProc;

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE,
	IDC_SELBORDER, IDC_SELFACE, IDC_SELELEMENT };

void SelectDlgProc::RefreshSelType (HWND hWnd) {
	ICustToolbar *iToolbar = GetICustToolbar (GetDlgItem (hWnd, IDC_SELTYPE));
	if (iToolbar) {
		ICustButton* but = NULL;
		for (int i=1; i<6; i++) {
			but = iToolbar->GetICustButton (butIDs[i]);
			if (!but) continue;
			but->SetCheck (mpEPoly->GetEPolySelLevel()==i);
			ReleaseICustButton (but);
		}
	}
	ReleaseICustToolbar(iToolbar);
}

void SelectDlgProc::SetEnables (HWND hSel) {
	bool fac = (meshSelLevel[mpEPoly->GetEPolySelLevel()] == MNM_SL_FACE);
	bool edg = (meshSelLevel[mpEPoly->GetEPolySelLevel()] == MNM_SL_EDGE);
	bool edgNotBrdr = (mpEPoly->GetEPolySelLevel() == EP_SL_EDGE);
	bool obj = (mpEPoly->GetEPolySelLevel() == EP_SL_OBJECT);
	bool vtx = (mpEPoly->GetEPolySelLevel() == EP_SL_VERTEX);

	BOOL multiSelect = mpEPoly->getParamBlock()->GetInt (ep_select_mode, TimeValue(0)) == 2;
	BOOL highlightSelect = mpEPoly->getParamBlock()->GetInt (ep_select_mode, TimeValue(0)) == 1;


	EnableWindow (GetDlgItem (hSel, IDC_SEL_BYVERT), fac||edg&& ! multiSelect && ! highlightSelect);
	EnableWindow (GetDlgItem (hSel, IDC_IGNORE_BACKFACES), !obj);

	EnableWindow (GetDlgItem (hSel, IDC_SELECT_SUBOBJ), mpEPoly->GetEPolySelLevel()>0);
	EnableWindow (GetDlgItem (hSel, IDC_SELECT_MULTI), mpEPoly->GetEPolySelLevel()>0);


	ICustButton *but = GetICustButton (GetDlgItem (hSel, IDC_SELECTION_GROW));
	if (but) {
		but->Enable (!obj);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hSel, IDC_SELECTION_SHRINK));
	if (but) {
		but->Enable (!obj);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hSel, IDC_EDGE_RING_SEL));
	if (but) {
		but->Enable (edg);
		ReleaseICustButton(but);
	}

	but = GetICustButton (GetDlgItem (hSel, IDC_EDGE_LOOP_SEL));
	if (but) {
		but->Enable (edg);
		ReleaseICustButton(but);
	}

	bool facNotElem = (mpEPoly->GetEPolySelLevel() == EP_SL_FACE);
	if (mpEPoly->getParamBlock()->GetInt (ep_by_vertex)) facNotElem = false;
	EnableWindow (GetDlgItem (hSel, IDC_SELECT_BY_ANGLE), facNotElem);
	ISpinnerControl *spinner = GetISpinner (GetDlgItem (hSel, IDC_SELECT_ANGLE_SPIN));
	if (spinner) {
		bool useAngle = mpEPoly->getParamBlock()->GetInt (ep_select_by_angle) != 0;
		spinner->Enable (facNotElem && useAngle);
		ReleaseISpinner (spinner);
	}

	spinner = GetISpinner (GetDlgItem (hSel, IDC_EDGE_RING_SEL_SPIN));
	if (spinner )
	{
		spinner->Enable (edg);
		ReleaseISpinner (spinner);
	}

	spinner = GetISpinner (GetDlgItem (hSel, IDC_EDGE_LOOP_SEL_SPIN));
	if (spinner )
	{
		spinner->Enable (edg);
		ReleaseISpinner (spinner);
	}
}

void SelectDlgProc::SetNumSelLabel (HWND hWnd) {	
	static TSTR buf;
	if (!updateNumSel) {
		SetDlgItemText (hWnd, IDC_NUMSEL_LABEL, buf);
		return;
	}
	updateNumSel = false;

	int num = 0, j = 0, lastsel = 0;
	//better way would be to create a new GetEpolysellevel function.. this is a hack for mz prototype
	//so this can change depending upon final implementation...

	int tempSelLevel = mpEPoly->GetEPolySelLevel();

	if(0&&mpEPoly->GetSelectMode()==PS_MULTI_SEL)
	{
		if(mpEPoly->GetCalcMeshSelLevel(tempSelLevel)>0)
		{
	   
			switch(mpEPoly->GetCalcMeshSelLevel(tempSelLevel))
			{
			case MNM_SL_VERTEX:
				tempSelLevel = EP_SL_VERTEX;
				break;
			case MNM_SL_EDGE:
				tempSelLevel = EP_SL_EDGE;
				break;
			case MNM_SL_FACE:
				tempSelLevel = EP_SL_FACE;
				break;
			};

		}
	}


	switch (tempSelLevel) {
	case EP_SL_OBJECT:
		buf.printf (GetString (IDS_OBJECT_SEL));
		break;

	case EP_SL_VERTEX:
		num=0;
		for (j=0; j<mpEPoly->GetMeshPtr()->numv; j++) {
			if (!mpEPoly->GetMeshPtr()->v[j].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
			num++;
			lastsel=j;
		}
		if (num==1) {
			buf.printf (GetString(IDS_WHICHVERTSEL), lastsel+1);
		} else buf.printf (GetString(IDS_NUMVERTSELP), num);
		break;

	case EP_SL_FACE:
	case EP_SL_ELEMENT:
		num=0;
		for (j=0; j<mpEPoly->GetMeshPtr()->numf; j++) {
			if (!mpEPoly->GetMeshPtr()->f[j].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
			num++;
			lastsel=j;
		}
		if (num==1) {
			buf.printf (GetString(IDS_WHICHFACESEL), lastsel+1);
		} else buf.printf(GetString(IDS_NUMFACESELP),num);
		break;

	case EP_SL_EDGE:
	case EP_SL_BORDER:
		num=0;
		for (j=0; j<mpEPoly->GetMeshPtr()->nume; j++) {
			if (!mpEPoly->GetMeshPtr()->e[j].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
			num++;
			lastsel=j;
		}
		if (num==1) {
			buf.printf (GetString(IDS_WHICHEDGESEL), lastsel+1);
		} else buf.printf(GetString(IDS_NUMEDGESELP),num);
		break;
	}

	SetDlgItemText (hWnd, IDC_NUMSEL_LABEL, buf);
}

INT_PTR SelectDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							 UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustToolbar	*iToolbar	= NULL;
	ISpinnerControl *l_spin		= NULL;

	//ICustButton *iButton;
	BitArray nsel;
	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;

	switch (msg) {
	case WM_INITDIALOG:
		{
		
		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_SELTYPE));
		iToolbar->SetImage (GetPolySelImageHandler()->LoadImages());
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELBORDER));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);


		l_spin = SetupIntSpinner (hWnd, IDC_EDGE_RING_SEL_SPIN, IDC_EDGE_RING_SEL_FOR_SPIN, -9999999, 9999999, 0);
		if (l_spin) {
			l_spin->SetAutoScale(FALSE);
			ReleaseISpinner (l_spin);
			l_spin = NULL;
		}

		l_spin = SetupIntSpinner (hWnd, IDC_EDGE_LOOP_SEL_SPIN, IDC_EDGE_LOOP_SEL_FOR_SPIN, -9999999, 9999999, 0);
		if (l_spin) {
			l_spin->SetAutoScale(FALSE);
			ReleaseISpinner (l_spin);
			l_spin = NULL;
		}


		RefreshSelType (hWnd);
		SetEnables (hWnd);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELVERTEX:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_VERTEX)
				mpEPoly->SetEPolySelLevel (EP_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					pEditPoly->EpfnConvertSelection (EP_SL_CURRENT, EP_SL_VERTEX, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if ( GetKeyState (VK_SHIFT)<0)
				{
					// new with 8.0 convert it into the border vertices 
					theHold.Begin ();
					pEditPoly->EpfnConvertSelectionToBorder (EP_SL_CURRENT, EP_SL_VERTEX);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
				}
				mpEPoly->SetEPolySelLevel (EP_SL_VERTEX);
			}
			break;

		case IDC_SELEDGE:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_EDGE)
				mpEPoly->SetEPolySelLevel (EP_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					pEditPoly->EpfnConvertSelection (EP_SL_CURRENT, EP_SL_EDGE, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if ( GetKeyState (VK_SHIFT)<0)
				{
					// new with 8.0 convert it into the border vertices 
					theHold.Begin ();
					pEditPoly->EpfnConvertSelectionToBorder (EP_SL_CURRENT, EP_SL_EDGE);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
				}
				mpEPoly->SetEPolySelLevel (EP_SL_EDGE);
			}
			break;

		case IDC_SELBORDER:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_BORDER)
				mpEPoly->SetEPolySelLevel (EP_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					pEditPoly->EpfnConvertSelection (EP_SL_CURRENT, EP_SL_BORDER, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				mpEPoly->SetEPolySelLevel (EP_SL_BORDER);
			}
			break;

		case IDC_SELFACE:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_FACE)
				mpEPoly->SetEPolySelLevel (EP_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					pEditPoly->EpfnConvertSelection (EP_SL_CURRENT, EP_SL_FACE, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if ( GetKeyState (VK_SHIFT)<0)
				{
					// new with 8.0 convert it into the border vertices 
					theHold.Begin ();
					pEditPoly->EpfnConvertSelectionToBorder (EP_SL_CURRENT, EP_SL_FACE);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
				}

				mpEPoly->SetEPolySelLevel (EP_SL_FACE);
			}
			break;

		case IDC_SELELEMENT:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_ELEMENT)
				mpEPoly->SetEPolySelLevel (EP_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					pEditPoly->EpfnConvertSelection (EP_SL_CURRENT, EP_SL_ELEMENT, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}

				mpEPoly->SetEPolySelLevel (EP_SL_ELEMENT);
			}
			break;

		case IDC_SELECTION_GROW:
			mpEPoly->EpActionButtonOp (epop_sel_grow);
			break;

		case IDC_SELECTION_SHRINK:
			mpEPoly->EpActionButtonOp (epop_sel_shrink);
			break;

		case IDC_EDGE_RING_SEL:
			mpEPoly->EpActionButtonOp (epop_select_ring);
			break;

		case IDC_EDGE_LOOP_SEL:
			mpEPoly->EpActionButtonOp (epop_select_loop);
			break;
		}
		break;

	case WM_PAINT:
		if (updateNumSel) SetNumSelLabel (hWnd);
		return FALSE;

	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_EDGE);
			break;
		case IDC_SELBORDER:
			lpttt->lpszText = GetString(IDS_BORDER);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_FACE);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_ELEMENT);
			break;
		case IDC_SELECTION_SHRINK:
			lpttt->lpszText = GetString(IDS_SELECTION_SHRINK);
			break;
		case IDC_SELECTION_GROW:
			lpttt->lpszText = GetString(IDS_SELECTION_GROW);
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		{	
			switch (LOWORD(wParam)) 
			{
			case IDC_EDGE_LOOP_SEL_SPIN:
			case IDC_EDGE_RING_SEL_SPIN:

				theHold.Begin();  // (uses restore object in paramblock.)
				theHold.Put (new RingLoopRestore(pEditPoly));
				if ( wParam == IDC_EDGE_RING_SEL_SPIN)
					theHold.Accept(GetString(IDS_SELECT_EDGE_RING));
				else 
					theHold.Accept(GetString(IDS_SELECT_EDGE_LOOP));

				bool l_ctrl = (GetKeyState (VK_CONTROL)< 0) ? true : false;
				bool l_alt  = (GetKeyState (VK_MENU) < 0)?  true : false;
				if ( l_alt || l_ctrl )
				{
					l_spin = (ISpinnerControl*)lParam;
					if ( l_spin )
					{
						// enable the one click behaviour : this allow to do 1 click , while using the 
						// ctl or alt key. laurent r 05/05
						l_spin->Execute(I_EXEC_SPINNER_ONE_CLICK_ENABLE);
				
						if ( l_alt )
						{
							// disable the alt slow down. 
							l_spin->Execute(I_EXEC_SPINNER_ALT_DISABLE);
						}
					}
				}


				pEditPoly->setAltKey(l_alt);
				pEditPoly->setCtrlKey(l_ctrl);
				break;
			}
			break;


		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) 
		{
			case IDC_EDGE_LOOP_SEL_SPIN:
			case IDC_EDGE_RING_SEL_SPIN:
				l_spin = (ISpinnerControl*)lParam;
				if ( l_spin )
				{
					// reset all 
					l_spin->Execute(I_EXEC_SPINNER_ALT_ENABLE);
					l_spin->Execute(I_EXEC_SPINNER_ONE_CLICK_DISABLE);
				}
				break;
		}
		break;

	case CC_SPINNER_CHANGE:
		{
			switch (LOWORD(wParam)) 
			{
			case IDC_EDGE_LOOP_SEL_SPIN:
			case IDC_EDGE_RING_SEL_SPIN:
				l_spin = (ISpinnerControl*)lParam;
				if ( l_spin )
				{
					if ( LOWORD(wParam) == IDC_EDGE_LOOP_SEL_SPIN)
						pEditPoly->UpdateLoopEdgeSelection(l_spin->GetIVal());
					else 
						pEditPoly->UpdateRingEdgeSelection(l_spin->GetIVal());
				}
				break;
			}
		}
		break; 

	default:
		return FALSE;
	}
	return TRUE;
}

#define GRAPHSTEPS 20

class SoftselDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;

public:
	SoftselDlgProc () : mpEPoly(NULL) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DrawCurve (TimeValue t, HWND hWnd, HDC hdc);
	void DeleteThis() { }
	void SetEnables (HWND hSS);
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static SoftselDlgProc theSoftselDlgProc;

// Task 748Y: UI redesign
void SoftselDlgProc::DrawCurve (TimeValue t, HWND hWnd,HDC hdc) {
	float pinch, falloff, bubble;
	IParamBlock2 *pblock = mpEPoly->getParamBlock ();
	if (!pblock) return;

	pblock->GetValue (ep_ss_falloff, t, falloff, FOREVER);
	pblock->GetValue (ep_ss_pinch, t, pinch, FOREVER);
	pblock->GetValue (ep_ss_bubble, t, bubble, FOREVER);

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL), label);
	SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL), FormatUniverseValue (0.0f));
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL), label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_SS_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	rect.right  -= 3;
	rect.left   += 3;
	rect.top    += 20;
	rect.bottom -= 20;
	
	// Draw dotted lines for axes:
	SelectObject(hdc,CreatePen(PS_DOT,0,GetSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx (hdc, (rect.left+rect.right)/2, orect.top, NULL);
	LineTo (hdc, (rect.left+rect.right)/2, orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	
	// Draw selection curve:
	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float prop = fabsf(i-GRAPHSTEPS/2)/float(GRAPHSTEPS/2);
		float y = AffectRegionFunction(falloff*prop,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}
	
	/*
	// Draw tangent handles:
	// Tangent at distance=0 is (1, -3 * pinch).
	SelectObject(hdc,CreatePen(PS_SOLID,0,RGB(0,255,0)));
	MoveToEx (hdc, rect.left, rect.top, NULL);
	float radius = float(rect.right - rect.left)/3.f;
	float tangentY = -3.f * pinch;
	float tangentLength = Sqrt(1.0f + tangentY*tangentY);
	int ix = rect.left + int (radius/tangentLength);
	int iy = rect.top - int (radius * tangentY / tangentLength);
	LineTo (hdc, ix, iy);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

	// Tangent at distance=1 is (-1, 3*bubble).
	SelectObject(hdc,CreatePen(PS_SOLID,0,RGB(255,0,0)));
	MoveToEx (hdc, rect.right, rect.bottom, NULL);
	tangentY = 3.f*bubble;
	tangentLength = Sqrt(1.0f + tangentY*tangentY);
	ix = rect.right - int (radius/tangentLength);
	iy = rect.bottom - int(radius*tangentY/tangentLength);
	LineTo (hdc, ix, iy);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
*/

	WhiteRect3D(hdc,orect,TRUE);
}

void SoftselDlgProc::SetEnables (HWND hSS) {
	ISpinnerControl* spin = NULL;
	EnableWindow (GetDlgItem (hSS, IDC_USE_SOFTSEL), mpEPoly->GetEPolySelLevel());
	IParamBlock2 *pblock = mpEPoly->getParamBlock ();
	int softSel, useEdgeDist;
	pblock->GetValue (ep_ss_use, TimeValue(0), softSel, FOREVER);
	pblock->GetValue (ep_ss_edist_use, TimeValue(0), useEdgeDist, FOREVER);
	bool enable = (mpEPoly->GetEPolySelLevel() && softSel) ? TRUE : FALSE;
	EditPolyObject *pEditPoly = (EditPolyObject*)mpEPoly;
	BOOL isPainting = (pEditPoly->GetPaintMode()==PAINTMODE_PAINTSEL? TRUE:FALSE);
	BOOL isBlurring = (pEditPoly->GetPaintMode()==PAINTMODE_BLURSEL? TRUE:FALSE);
	BOOL isErasing  = (pEditPoly->GetPaintMode()==PAINTMODE_ERASESEL? TRUE:FALSE);

	ICustButton *but = GetICustButton (GetDlgItem (hSS, IDC_SHADED_FACE_TOGGLE));
	but->Enable (enable);
	ReleaseICustButton (but);
	EnableWindow (GetDlgItem (hSS, IDC_LOCK_SOFTSEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_BORDER), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_VALUE_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_SIZE_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_STRENGTH_LABEL), enable);
	but = GetICustButton (GetDlgItem (hSS, IDC_PAINTSEL_PAINT));
	but->SetCheck( isPainting );
	but->Enable (enable);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hSS, IDC_PAINTSEL_BLUR));
	but->SetCheck( isBlurring );
	but->Enable (enable);
	ReleaseICustButton (but);
	but = GetICustButton (GetDlgItem (hSS, IDC_PAINTSEL_REVERT));
	but->SetCheck( isErasing );
	but->Enable (enable);
	ReleaseICustButton (but);
	spin = GetISpinner (GetDlgItem (hSS, IDC_PAINTSEL_VALUE_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hSS, IDC_PAINTSEL_SIZE_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hSS, IDC_PAINTSEL_STRENGTH_SPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	but = GetICustButton (GetDlgItem (hSS, IDC_PAINTSEL_OPTIONS));
	but->Enable (enable);
	ReleaseICustButton (but);

	//If the selection is locked, disable controls (except the shaded face toggle)
	if( pblock->GetInt(ep_ss_lock, TimeValue(0)) )
		enable = FALSE;
	EnableWindow (GetDlgItem (hSS, IDC_USE_EDIST), enable);
	EnableWindow (GetDlgItem (hSS, IDC_AFFECT_BACKFACING), enable);
	spin = GetISpinner (GetDlgItem (hSS, IDC_EDIST_SPIN));
	spin->Enable (enable && useEdgeDist);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hSS, IDC_FALLOFFSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hSS, IDC_PINCHSPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	spin = GetISpinner (GetDlgItem (hSS, IDC_BUBBLESPIN));
	spin->Enable (enable);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hSS, IDC_FALLOFF_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PINCH_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_BUBBLE_LABEL), enable);
}

INT_PTR SoftselDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	Rect rect;
	PAINTSTRUCT ps;
	HDC hdc;
	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;

	switch (msg) {
	case WM_INITDIALOG: {
			ICustButton *btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_PAINT));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);
			btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_BLUR));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);
			btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_REVERT));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);
			SetEnables (hWnd);
		}
		break;
		
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&ps);
		DrawCurve (t, hWnd, hdc);
		EndPaint(hWnd,&ps);
		return FALSE;

	case CC_SPINNER_BUTTONDOWN:
		pEditPoly->EpPreviewSetDragging (true);
		break;

	case CC_SPINNER_BUTTONUP:
		pEditPoly->EpPreviewSetDragging (false);
		// If we haven't been interactively updating, we need to send an update now.
		int interactive;
		pEditPoly->getParamBlock()->GetValue (ep_interactive_full, TimeValue(0), interactive, FOREVER);
		if (!interactive) pEditPoly->EpPreviewInvalidate ();
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_FALLOFFSPIN:
		case IDC_PINCHSPIN:
		case IDC_BUBBLESPIN:
			GetClientRectP(GetDlgItem(hWnd,IDC_SS_GRAPH),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			mpEPoly->InvalidateSoftSelectionCache();
			break;
		case IDC_EDIST_SPIN:
			mpEPoly->InvalidateDistanceCache ();
			break;
		}
		map->RedrawViews (t, REDRAW_INTERACTIVE);
		break;

	

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USE_SOFTSEL:
			SetEnables (hWnd);
			mpEPoly->InvalidateDistanceCache ();
			map->RedrawViews (t);
			break;
		case IDC_USE_EDIST:
			SetEnables (hWnd);
			mpEPoly->InvalidateDistanceCache ();
			map->RedrawViews (t);
			break;

		case IDC_EDIST:
			mpEPoly->InvalidateDistanceCache ();
			map->RedrawViews (t);
			break;

		case IDC_AFFECT_BACKFACING:
			mpEPoly->InvalidateSoftSelectionCache ();
			map->RedrawViews (t);
			break;

		case IDC_SHADED_FACE_TOGGLE:
			mpEPoly->EpActionButtonOp (epop_toggle_shaded_faces);
			break;
		case IDC_LOCK_SOFTSEL:
			// When the soft selection becomes unlocked, end any active paint session
			//if( !(mpEPoly->getParamBlock ()->GetInt(ep_ss_lock,t)) ) {
			//	if( InPaintMode() ) EndPaint();
			//	mpEPoly->InvalidateDistanceCache ();
			//	map->RedrawViews (t);
			//}
			//SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_PAINT:
			pEditPoly->OnPaintBtn( PAINTMODE_PAINTSEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_BLUR:
			pEditPoly->OnPaintBtn( PAINTMODE_BLURSEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_REVERT:
			pEditPoly->OnPaintBtn( PAINTMODE_ERASESEL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTSEL_OPTIONS:
			GetMeshPaintMgr()->BringUpOptions();
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

// Luna task 748A - popup dialog
class PopupDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	EditPolyObject *mpEditPoly;
	int opPreview;

public:
	PopupDlgProc () : mpEPoly(NULL), mpEditPoly(NULL), opPreview(epop_null) { }
	void SetOperation (EPoly *pEPoly, int op) { mpEPoly=pEPoly; opPreview=op; mpEditPoly = (EditPolyObject *) pEPoly; }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void InvalidateUI (HWND hParams, int element);
	void DeleteThis() { }
};

static PopupDlgProc thePopupDlgProc;

// Luna task 748A - popup dialog
void PopupDlgProc::InvalidateUI (HWND hParams, int element) {
	HWND hDlgItem;
	TSTR buf;

	if (element<0) {
		// do all of them:
		InvalidateUI (hParams, ep_lift_edge);
		InvalidateUI (hParams, ep_tess_type);
		InvalidateUI (hParams, ep_extrude_spline_node);
		InvalidateUI (hParams, ep_bridge_selected);	// (this one does ep_bridge_target 1 and 2)
	}


	switch (element) {
	case ep_lift_edge:
		hDlgItem = GetDlgItem (hParams, IDC_LIFT_PICK_EDGE);
		if (hDlgItem) {
			ICustButton *ibut = GetICustButton (hDlgItem);
			if (ibut) {
				int liftEdge;
				mpEPoly->getParamBlock()->GetValue (ep_lift_edge, TimeValue(0), liftEdge, FOREVER);
				static TSTR buf;
				if (liftEdge<0) {	// undefined.
					buf.printf (GetString (IDS_PICK_LIFT_EDGE));
				} else {
					// Don't forget +1 for 1-based (rather than 0-based) indexing in UI:
					buf.printf (GetString (IDS_EDGE_NUMBER), liftEdge+1);
				}
				ibut->SetText (buf);
				ReleaseICustButton (ibut);
			}
		}
		break;

	case ep_tess_type:
		hDlgItem = GetDlgItem (hParams, IDC_TESS_TENSION_LABEL);
		if (hDlgItem) {
			int faceType;
			mpEPoly->getParamBlock()->GetValue (ep_tess_type, TimeValue(0), faceType, FOREVER);
			EnableWindow (hDlgItem, !faceType);
			ISpinnerControl *spin = GetISpinner (GetDlgItem (hParams, IDC_TESS_TENSIONSPIN));
			if (spin) {
				spin->Enable (!faceType);
				ReleaseISpinner (spin);
			}
		}
		break;

	case ep_extrude_spline_node:
		hDlgItem = GetDlgItem (hParams, IDC_EXTRUDE_PICK_SPLINE);
		if (hDlgItem) {
			INode* splineNode = NULL;
			mpEPoly->getParamBlock()->GetValue (ep_extrude_spline_node, TimeValue(0), splineNode, FOREVER);
			if (splineNode) SetDlgItemText (hParams, IDC_EXTRUDE_PICK_SPLINE, splineNode->GetName());
		}
		break;

	case ep_bridge_selected:
		hDlgItem = GetDlgItem (hParams, IDC_BRIDGE_PICK_TARG1);
		if (hDlgItem) {
			bool specific = (mpEPoly->getParamBlock()->GetInt(ep_bridge_selected) == 0);
			ICustButton *but = GetICustButton (hDlgItem);
			if (but) {
				but->Enable (specific);
				ReleaseICustButton (but);
			}
			but = GetICustButton (GetDlgItem (hParams, IDC_BRIDGE_PICK_TARG2));
			if (but) {
				but->Enable (specific);
				ReleaseICustButton (but);
			}
			//ISpinnerControl *spin = GetISpinner (GetDlgItem (hParams, IDC_BRIDGE_TWIST1_SPIN));
			//if (spin) {
			//	spin->Enable (specific);
			//	ReleaseISpinner (spin);
			//}
			//spin = GetISpinner (GetDlgItem (hParams, IDC_BRIDGE_TWIST2_SPIN));
			//if (spin) {
			//	spin->Enable (specific);
			//	ReleaseISpinner (spin);
			//}
			EnableWindow (GetDlgItem (hParams, IDC_BRIDGE_TARGET_1), specific);
			EnableWindow (GetDlgItem (hParams, IDC_BRIDGE_TARGET_2), specific);
			//EnableWindow (GetDlgItem (hParams, IDC_BRIDGE_TWIST1_LABEL), specific);
			//EnableWindow (GetDlgItem (hParams, IDC_BRIDGE_TWIST2_LABEL), specific);
			InvalidateUI (hParams, ep_bridge_target_1);
			InvalidateUI (hParams, ep_bridge_target_2);
		}
		break;

	case ep_bridge_target_1:
		hDlgItem = GetDlgItem (hParams, IDC_BRIDGE_PICK_TARG1);
		if (hDlgItem) {
			bool specific = (mpEPoly->getParamBlock()->GetInt(ep_bridge_selected) == 0);
			bool edgeOrBorder = (	mpEPoly->GetEPolySelLevel() == EP_SL_BORDER ||
									mpEPoly->GetEPolySelLevel() == EP_SL_EDGE )  ;
			int targ = mpEPoly->getParamBlock ()->GetInt (ep_bridge_target_1);
			if (targ==0) SetDlgItemText (hParams, IDC_BRIDGE_PICK_TARG1, GetString (
				edgeOrBorder ? IDS_BRIDGE_PICK_EDGE_1 : IDS_BRIDGE_PICK_POLYGON_1));
			else {
				buf.printf (GetString (edgeOrBorder ? IDS_BRIDGE_WHICH_EDGE : IDS_BRIDGE_WHICH_POLYGON), targ);
				SetDlgItemText (hParams, IDC_BRIDGE_PICK_TARG1, buf);
			}
		}
		break;

	case ep_bridge_target_2:
		hDlgItem = GetDlgItem (hParams, IDC_BRIDGE_PICK_TARG2);
		if (hDlgItem) {
			bool specific = (mpEPoly->getParamBlock()->GetInt(ep_bridge_selected) == 0);
			bool EdgeOrBorder = (	mpEPoly->GetEPolySelLevel() == EP_SL_BORDER ||
				mpEPoly->GetEPolySelLevel() == EP_SL_EDGE )  ;
			int targ = mpEPoly->getParamBlock ()->GetInt (ep_bridge_target_2);
			if (targ==0) SetDlgItemText (hParams, IDC_BRIDGE_PICK_TARG2, GetString (
				EdgeOrBorder ? IDS_BRIDGE_PICK_EDGE_2 : IDS_BRIDGE_PICK_POLYGON_2));
			else {
				buf.printf (GetString (EdgeOrBorder ? IDS_BRIDGE_WHICH_EDGE : IDS_BRIDGE_WHICH_POLYGON), targ);
				SetDlgItemText (hParams, IDC_BRIDGE_PICK_TARG2, buf);
			}
		}
		break;
	}
}

// Luna task 748A - popup dialog
INT_PTR PopupDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
										UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		// Place dialog in correct position:
		thePopupPosition.Set (hWnd);

		but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_PICK_SPLINE));
		if (but) {
			but->SetType (CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_LIFT_PICK_EDGE));
		if (but) {
			but->SetType (CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton (but);

			// Set the lift edge to -1 to make sure nothing happens till user picks edge.
			mpEPoly->getParamBlock()->SetValue (ep_lift_edge, TimeValue(0), -1);
		}

		if (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1)) {
			but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1));
			if (but) {
				but->SetType (CBT_CHECK);
				but->SetHighlightColor(GREEN_WASH);
				ReleaseICustButton(but);
			}

			but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG2));
			if (but) {
				but->SetType (CBT_CHECK);
				but->SetHighlightColor(GREEN_WASH);
				ReleaseICustButton(but);
			}

			bool Border = (mpEPoly->GetEPolySelLevel() == EP_SL_BORDER );
			bool Edge   = (mpEPoly->GetEPolySelLevel() == EP_SL_EDGE )  ;

			if (Border )
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_BORDERS) );
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_BORDER_SELECTION) );
			}
			else if (Edge )
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_EDGES ));
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_EDGE_SELECTION ));
			}
			else 
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_POLYGONS ));
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_POLYGON_SELECTION ));
			}


			SetDlgItemText (hWnd, IDC_BRIDGE_TARGET_1, GetString (
				(Edge || Border) ? IDS_BRIDGE_EDGE_1 : IDS_BRIDGE_POLYGON_1));
			SetDlgItemText (hWnd, IDC_BRIDGE_TARGET_2, GetString (
				(Edge || Border) ? IDS_BRIDGE_EDGE_2 : IDS_BRIDGE_POLYGON_2));
		}

		InvalidateUI (hWnd, -1);

		// The PB2 descriptor tells the system to enable or disable the rotation controls based on the align checkbox.
		// But there's no way to make it do the same for the label for the rotation control.
		// We handle that here:
		BOOL align;
		mpEPoly->getParamBlock()->GetValue (ep_extrude_spline_align, t, align, FOREVER);
		EnableWindow (GetDlgItem (hWnd, IDC_EXTRUDE_ROTATION_LABEL), align);

		// Start the preview itself:
		mpEditPoly->EpPreviewBegin (opPreview);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_APPLY:
			// Apply and don't close (but do clear selection).
			theHold.Begin ();	// so we can parcel the operation and the selection-clearing together.
			mpEditPoly->EpPreviewAccept ();
			theHold.Accept (GetString (IDS_APPLY));
			mpEditPoly->EpPreviewBegin (opPreview);
			break;

		case IDOK:	// Apply and close.
			// Make sure we're not in the pick edge mode:
			if (GetDlgItem (hWnd, IDC_LIFT_PICK_EDGE))
				mpEditPoly->ExitPickLiftEdgeMode ();
			mpEditPoly->EpPreviewAccept ();
			thePopupPosition.Scan (hWnd);
			DestroyModelessParamMap2(map);
			mpEditPoly->ClearOperationSettings ();

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
			macroRecorder->FunctionCall(_T("$.EditablePoly.closePopupDialog"), 0, 0);
			macroRecorder->EmitScript ();
#endif
			break;

		case IDCANCEL:
			// Make sure we're not still in any pick modes:
			if (GetDlgItem (hWnd, IDC_LIFT_PICK_EDGE))
				mpEditPoly->ExitPickLiftEdgeMode ();
			if (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1))
				mpEditPoly->ExitPickBridgeModes ();
			mpEditPoly->EpPreviewCancel ();
			thePopupPosition.Scan (hWnd);
			DestroyModelessParamMap2(map);
			mpEditPoly->ClearOperationSettings ();

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
			macroRecorder->FunctionCall(_T("$.EditablePoly.closePopupDialog"), 0, 0);
			macroRecorder->EmitScript ();
#endif
			break;

		case IDC_EXTRUDE_PICK_SPLINE:
			mpEPoly->EpActionEnterPickMode (epmode_pick_shape);
			break;
			
		case IDC_LIFT_PICK_EDGE:
			mpEPoly->EpActionToggleCommandMode (epmode_pick_lift_edge);
			break;

		case IDC_BRIDGE_PICK_TARG1:
			mpEPoly->EpActionToggleCommandMode (epmode_pick_bridge_1);
			break;

		case IDC_BRIDGE_PICK_TARG2:
			mpEPoly->EpActionToggleCommandMode (epmode_pick_bridge_2);
			break;

		case IDC_MS_SEP_BY_SMOOTH:
		case IDC_MS_SEP_BY_MATID:
		case IDC_TESS_EDGE:
		case IDC_TESS_FACE:
		case IDC_EXTYPE_GROUP:
		case IDC_EXTYPE_LOCAL:
		case IDC_EXTYPE_BY_POLY:
		case IDC_BEVTYPE_GROUP:
		case IDC_BEVTYPE_LOCAL:
		case IDC_BEVTYPE_BY_POLY:
		case IDC_INSETTYPE_GROUP:
		case IDC_INSETTYPE_BY_POLY:
		case IDC_RELAX_HOLD_BOUNDARY:
		case IDC_RELAX_HOLD_OUTER:
		case IDC_BRIDGE_SELECTED:
		case IDC_BRIDGE_SPECIFIC:
		case IDC_REVERSETRI:
		case IDC_VERTEX_CHAMFER_OPEN:
		case IDC_EDGE_CHAMFER_OPEN:
			// Following is necessary because Modeless ParamMap2's don't send such a message.
			mpEPoly->RefreshScreen ();
			break;

		case IDC_EXTRUDE_ALIGN_NORMAL:
			// The PB2 descriptor tells the system to enable or disable the rotation controls based on the align checkbox.
			// But there's no way to make it do the same for the label for the rotation control.
			// We handle that here:
			BOOL align;
			mpEPoly->getParamBlock()->GetValue (ep_extrude_spline_align, t, align, FOREVER);
			EnableWindow (GetDlgItem (hWnd, IDC_EXTRUDE_ROTATION_LABEL), align);
			// Following is necessary because Modeless ParamMap2's don't send such a message.
			mpEPoly->RefreshScreen ();
			break;
	}
		break;

	case CC_SPINNER_CHANGE:
		// Following is necessary because Modeless ParamMap2's don't send such a message:
		mpEPoly->RefreshScreen();
		break;

	case CC_SPINNER_BUTTONDOWN:
		mpEditPoly->EpPreviewSetDragging (true);
		break;

	case CC_SPINNER_BUTTONUP:
		mpEditPoly->EpPreviewSetDragging (false);
		mpEditPoly->EpPreviewInvalidate ();
		// Need to tell system something's changed:
		mpEditPoly->NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
		mpEPoly->RefreshScreen ();
		break;
	}
	return false;
}

// Luna task 748BB - repeat last
void EditPolyObject::EpSetLastOperation (int op) {
	ICustButton *pButton = NULL;
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) pButton = GetICustButton (GetDlgItem (hGeom, IDC_REPEAT_LAST));

	switch (op) {
	case epop_null:
	case epop_unhide:
	case epop_ns_copy:
	case epop_ns_paste:
	case epop_attach_list:
	case epop_reset_plane:
	case epop_remove_iso_verts:
	case epop_remove_iso_map_verts:
	case epop_update:
	case epop_selby_vc:
	case epop_selby_matid:
	case epop_selby_smg:
	case epop_cut:
	case epop_toggle_shaded_faces:
		// Don't modify button or mLastOperation.
		break;
	case epop_hide:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_HIDE));
		break;
	case epop_hide_unsel:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_HIDE_UNSELECTED));
		break;
	case epop_cap:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_CAP));
		break;
	case epop_delete:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_DELETE));
		break;
	case epop_remove:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_REMOVE));
		break;
	case epop_detach:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_DETACH));
		break;
	case epop_split:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SPLIT));
		break;
	case epop_break:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_BREAK));
		break;
	case epop_collapse:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_COLLAPSE));
		break;
	case epop_weld_sel:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_WELD_SEL));
		break;
	case epop_slice:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SLICE));
		break;
	case epop_create_shape:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_CREATE_SHAPE_FROM_EDGES));
		break;
	case epop_make_planar:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_MAKE_PLANAR));
		break;
	case epop_align_grid:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_ALIGN_TO_GRID));
		break;
	case epop_align_view:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_ALIGN_TO_VIEW));
		break;
	case epop_relax:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_RELAX));
		break;
	case epop_make_planar_x:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_MAKE_PLANAR_IN_X));
		break;
	case epop_make_planar_y:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_MAKE_PLANAR_IN_Y));
		break;
	case epop_make_planar_z:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_MAKE_PLANAR_IN_Z));
		break;
	case epop_meshsmooth:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_MESHSMOOTH));
		break;
	case epop_tessellate:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_TESSELLATE));
		break;
	case epop_flip_normals:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_FLIP_NORMALS));
		break;
	case epop_retriangulate:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_RETRIANGULATE));
		break;
	case epop_autosmooth:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_AUTOSMOOTH));
		break;
	case epop_clear_smg:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_ASSIGN_SMGROUP));
		break;
	case epop_extrude:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_EXTRUDE));
		break;
	case epop_bevel:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_BEVEL));
		break;
	case epop_chamfer:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_CHAMFER));
		break;
	case epop_inset:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_INSET));
		break;
	case epop_outline:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_OUTLINE));
		break;
	case epop_lift_from_edge:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_LIFT_FROM_EDGE));
		break;
	case epop_connect_edges:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_CONNECT_EDGES));
		break;
	case epop_connect_vertices:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_CONNECT_VERTICES));
		break;
	case epop_extrude_along_spline:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_EXTRUDE_ALONG_SPLINE));
		break;
	case epop_sel_grow:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECTION_GROW));
		break;
	case epop_sel_shrink:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECTION_SHRINK));
		break;
	case epop_select_loop:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECT_EDGE_LOOP));
		break;
	case epop_select_ring:
		mLastOperation = op;
		if (pButton) pButton->SetTooltip (true, GetString (IDS_SELECT_EDGE_RING));
		break;
	}

	if (pButton) ReleaseICustButton (pButton);
}

bool EditPolyObject::EpfnPopupDialog (int op) {
	ExitAllCommandModes (true, false);
	thePopupDlgProc.SetOperation (this, op);

	int msl = GetMNSelLevel();
	int dialogID;
	switch (op) {
	case epop_meshsmooth:
		dialogID = IDD_SETTINGS_MESHSMOOTH;
		break;
	case epop_tessellate:
		dialogID = IDD_SETTINGS_TESSELLATE;
		break;
	case epop_relax:
		dialogID = IDD_SETTINGS_RELAX;
		break;
	case epop_extrude:
		switch (msl) {
		case MNM_SL_VERTEX:
			dialogID = IDD_SETTINGS_EXTRUDE_VERTEX;
			break;
		case MNM_SL_EDGE:
			dialogID = IDD_SETTINGS_EXTRUDE_EDGE;
			break;
		case MNM_SL_FACE:
			dialogID = IDD_SETTINGS_EXTRUDE_FACE;
			break;
		default:
			return false;
		}
		break;
	case epop_bevel:
		if (msl != MNM_SL_FACE) return false;
		dialogID = IDD_SETTINGS_BEVEL;
		break;
	case epop_chamfer:
		if (msl == MNM_SL_FACE) return false;
		if (msl == MNM_SL_OBJECT) return false;
		if (msl == MNM_SL_VERTEX) dialogID = IDD_SETTINGS_CHAMFER_VERTEX;
		else dialogID = IDD_SETTINGS_CHAMFER_EDGE;
		break;
	case epop_outline:
		if (msl != MNM_SL_FACE) return false;
		dialogID = IDD_SETTINGS_OUTLINE;
		break;
	case epop_inset:
		if (msl != MNM_SL_FACE) return false;
		dialogID = IDD_SETTINGS_INSET;
		break;
	case epop_connect_edges:
		if (msl != MNM_SL_EDGE) return false;
		dialogID = IDD_SETTINGS_CONNECT_EDGES;
		break;
	case epop_lift_from_edge:
		if (msl != MNM_SL_FACE) return false;
		dialogID = IDD_SETTINGS_LIFT_FROM_EDGE;
		break;
	case epop_weld_sel:
		if (msl == MNM_SL_FACE) return false;
		if (msl == MNM_SL_OBJECT) return false;
		if (msl == MNM_SL_EDGE) dialogID = IDD_SETTINGS_WELD_EDGES;
		else dialogID = IDD_SETTINGS_WELD_VERTICES;
		break;
	case epop_extrude_along_spline:
		if (msl != MNM_SL_FACE) return false;
		dialogID = IDD_SETTINGS_EXTRUDE_ALONG_SPLINE;
		break;
	case epop_bridge_border:
		if (selLevel != EP_SL_BORDER) return false;
		dialogID = IDD_SETTINGS_BRIDGE_BORDERS;
		break;
	case epop_bridge_polygon:
		if (selLevel != EP_SL_FACE) return false;
		dialogID = IDD_SETTINGS_BRIDGE_POLYGONS;
		break;
	case epop_bridge_edge:
		if (selLevel != EP_SL_EDGE) return false;
		dialogID = IDD_SETTINGS_BRIDGE_EDGES;
		break;
	case epop_break:
		if (selLevel != EP_SL_VERTEX) return false;
		dialogID = IDD_SETTINGS_BREAK_VERTEX;
		break;
	default:
		return false;
	}
	TimeValue t = ip ? ip->GetTime() : GetCOREInterface()->GetTime();

	// Make the parammap.
	if (pOperationSettings) EpfnClosePopupDialog ();

	pOperationSettings = CreateModelessParamMap2 (
		ep_settings, pblock, t, hInstance, MAKEINTRESOURCE (dialogID),
		GetCOREInterface()->GetMAXHWnd(), &thePopupDlgProc);

	if (mPreviewOn) {
		EpPreviewInvalidate ();
		NotifyDependents (FOREVER, PART_ALL, REFMSG_CHANGE);
	}

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.PopupDialog"), 1, 0,
		mr_name, LookupOperationName(op));
	macroRecorder->EmitScript ();
#endif

	return true;
}

class PreserveMapsUIHandler {
private:
	EditPolyObject *mpEPoly;
	MapBitArray mActive, mSettings;

public:
	PreserveMapsUIHandler () : mpEPoly(NULL), mActive(false), mSettings(true,false) { }
	void SetEPoly (EditPolyObject *epoly) { mpEPoly = epoly; }
	void Initialize (HWND hWnd);
	void Update (HWND hWnd);
	void DoDialog ();
	void Reset (HWND hWnd);
	void Commit ();
	void Toggle (int channel);
};

static PreserveMapsUIHandler thePreserveMapsUIHandler;

void PreserveMapsUIHandler::Initialize (HWND hWnd) {
	// Make all our map buttons into check-type buttons:
	for (int chan=IDC_PRESERVE_MAP_CHAN_MINUS_2; chan<=IDC_PRESERVE_MAP_CHAN_99; chan++) {
		ICustButton *but = GetICustButton (GetDlgItem (hWnd, chan));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}
	}

	// Find out which map channels are active in our mesh:
	mActive = MapBitArray(false);

	MNMesh *pMesh = mpEPoly->GetMeshPtr();
	if (pMesh != NULL) {
		for (int chan=-NUM_HIDDENMAPS; chan<pMesh->numm; chan++) {
			if (!pMesh->M(chan)->GetFlag(MN_DEAD)) mActive.Set(chan);
		}
	}

	mSettings = mpEPoly->GetPreserveMapSettings();
	EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), false);

	Update (hWnd);
}

void PreserveMapsUIHandler::Update (HWND hWnd) {
	int maxActive = 0;
	for (int chan = -NUM_HIDDENMAPS; chan<100; chan++) {
		HWND hButton = GetDlgItem (hWnd, IDC_PRESERVE_MAP_CHAN_00 + chan);
		if (!hButton) continue;
		if (!mActive[chan]) ShowWindow (hButton, SW_HIDE);
		else {
			if (chan>0) maxActive = chan;
			ShowWindow (hButton, SW_NORMAL);
			ICustButton *but = GetICustButton (hButton);
			if (but) {
				but->SetCheck (mSettings[chan]);
				ReleaseICustButton (but);
			}
		}
	}

	// TODO: Shorten the dialog to something reasonable, based on maxActive.
}

void PreserveMapsUIHandler::Reset (HWND hWnd) {
	mSettings = MapBitArray(true, false);
	Update (hWnd);
}

void PreserveMapsUIHandler::Commit () {
	theHold.Begin ();
	mpEPoly->SetPreserveMapSettings (mSettings);
	theHold.Accept (GetString (IDS_PRESERVE_MAP_SETTINGS));
	mpEPoly->RefreshScreen ();
}

void PreserveMapsUIHandler::Toggle (int channel) {
	mSettings.Set (channel, !mSettings[channel]);
}

static INT_PTR CALLBACK PreserveMapsSettingsDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);
		thePreserveMapsUIHandler.Initialize (hWnd);
		break;

	case WM_COMMAND:
		id = LOWORD(wParam);
		if ((id>=IDC_PRESERVE_MAP_CHAN_MINUS_2) && (id<=IDC_PRESERVE_MAP_CHAN_99)) {
			thePreserveMapsUIHandler.Toggle (id - IDC_PRESERVE_MAP_CHAN_00);
			EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), true);
		} else {
			switch (LOWORD(wParam)) {
			case IDC_PRESERVE_MAPS_RESET:
				thePreserveMapsUIHandler.Reset (hWnd);
				EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), true);
				break;

			case IDC_PRESERVE_MAPS_APPLY:
				thePreserveMapsUIHandler.Commit ();
				EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), false);
				break;

			case IDOK:
				thePopupPosition.Scan (hWnd);
				thePreserveMapsUIHandler.Commit ();
				EndDialog(hWnd,1);
				break;

			case IDCANCEL:
				thePopupPosition.Scan (hWnd);
				EndDialog(hWnd,0);
				break;
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void PreserveMapsUIHandler::DoDialog () {
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS_PRESERVE_MAPS),
		mpEPoly->ip->GetMAXHWnd(), PreserveMapsSettingsDlgProc, NULL);
}

class GeomDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	EditPolyObject *mpEditPoly;	// Ideally replaced with EPoly2 interface or something?

public:
	GeomDlgProc () : mpEPoly(NULL), mpEditPoly(NULL) { }
	void SetEPoly (EPoly *e) { mpEPoly = e; mpEditPoly = (EditPolyObject *)e; }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void SetEnables (HWND hGeom);
	void UpdateCageCheckboxEnable (HWND hGeom);
	void DeleteThis() { }
	void UpdateConstraint (HWND hWnd);
};

static GeomDlgProc theGeomDlgProc;

void GeomDlgProc::SetEnables (HWND hGeom) {
	int sl = mpEPoly->GetEPolySelLevel ();
	BOOL elem = (sl == EP_SL_ELEMENT);
	BOOL obj = (sl == EP_SL_OBJECT);
	bool edg = (meshSelLevel[sl]==MNM_SL_EDGE) ? true : false;
	ICustButton *but = NULL;

	// Create always active.

	// Delete active in subobj.
	//but = GetICustButton (GetDlgItem (hGeom, IDC_DELETE));
	//but->Enable (!obj);
	//ReleaseICustButton (but);

	// Attach, attach list always active.

	// Detach active in subobj
	but = GetICustButton (GetDlgItem (hGeom, IDC_DETACH));
	but->Enable (!obj);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hGeom, IDC_COLLAPSE));
	but->Enable (!obj && !elem);
	ReleaseICustButton (but);

	// It would be nice if Slice Plane were always active, but we can't make it available
	// at the object level, since the transforms won't work.
	but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
	but->Enable (!obj);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hGeom, IDC_SLICE));
	but->Enable (mpEPoly->EpfnInSlicePlaneMode());
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
	but->Enable (mpEPoly->EpfnInSlicePlaneMode());
	ReleaseICustButton (but);

	// Cut always active
	// Quickslice also always active.

	// Make Planar, Make Planar in X/Y/Z, Align View, Align Grid always active.
	// MSmooth, Tessellate always active.

	but = GetICustButton (GetDlgItem (hGeom, IDC_HIDE_SELECTED));
	if (but) {
		but->Enable (!edg && !obj);
	ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_HIDE_UNSELECTED));
	if (but) {
		but->Enable (!edg && !obj);
	ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_UNHIDEALL));
	if (but) {
		but->Enable (!edg && !obj);
	ReleaseICustButton (but);
	}

	EnableWindow (GetDlgItem (hGeom, IDC_NAMEDSEL_LABEL), !obj);
	but = GetICustButton (GetDlgItem (hGeom, IDC_COPY_NS));
	if (but != NULL) {
	but->Enable (!obj);
	ReleaseICustButton (but);
	}
	but = GetICustButton (GetDlgItem (hGeom, IDC_PASTE_NS));
	if (but != NULL) {
		but->Enable (!obj && (GetMeshNamedSelClip (namedClipLevel[mpEPoly->GetEPolySelLevel()])));
		ReleaseICustButton(but);
	}

	EnableWindow (GetDlgItem (hGeom, IDC_DELETE_ISOLATED_VERTS), (sl>EP_SL_VERTEX));

	UpdateCageCheckboxEnable (hGeom);
}

void GeomDlgProc::UpdateCageCheckboxEnable (HWND hGeom) {
	EnableWindow (GetDlgItem (hGeom, IDC_SHOW_CAGE), mpEditPoly->ShowGizmoConditions ());
}

void EditPolyObject::UpdateCageCheckboxEnable () {
	HWND hWnd = GetDlgHandle (ep_geom);
	if (hWnd) {
		theGeomDlgProc.UpdateCageCheckboxEnable (hWnd);
	}
}

//OVERRIDES!!!!

//This class handles the subobject mode override

class SelectionModeOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mLastSelection;
	int mCurrentSelection;
	EditPolyObject *mpObj;
public:
	SelectionModeOverride(EditPolyObject *m,int selection):mActiveOverride(FALSE),mpObj(m),mCurrentSelection(selection){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL SelectionModeOverride ::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL SelectionModeOverride ::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mpObj->getParamBlock()->GetValue (ep_select_mode, TimeValue(0), mLastSelection, FOREVER);
	if(mLastSelection==mCurrentSelection && mCurrentSelection>0)
		mCurrentSelection = 0; //make it go to none if we are in subobj or highlight

	mpObj->getParamBlock()->SetValue (ep_select_mode, TimeValue(0), mCurrentSelection);
	return TRUE;
}

BOOL SelectionModeOverride ::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;

	mpObj->getParamBlock()->SetValue (ep_select_mode, TimeValue(0), mLastSelection);
	return TRUE;
}


//paint overrides
class PaintOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mPaint;
	int mPrevPaintMode;
	int mOldCommandMode;
	EditPolyObject *mpObj;
public:
	PaintOverride(EditPolyObject *m,int c):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mpObj(m),mPaint(c){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL PaintOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL PaintOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	mpObj->OnPaintBtn(mPaint,GetCOREInterface()->GetTime());
	return TRUE;
}

BOOL PaintOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	if(mPrevPaintMode!= PAINTMODE_OFF)
		mpObj->OnPaintBtn( mPrevPaintMode, GetCOREInterface()->GetTime());
	else if(mOldCommandMode!=-1)
		mpObj->EpActionToggleCommandMode(mOldCommandMode);
	else
		mpObj->OnPaintBtn( mPaint, GetCOREInterface()->GetTime());
	return TRUE;
}

		


//constraint overrides
class ConstraintOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mOverrideConstraint;
	int mPrevConstraint;
	EditPolyObject *mpObj;
public:
	ConstraintOverride(EditPolyObject *m,int c):mActiveOverride(FALSE),mpObj(m),mOverrideConstraint(c){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL ConstraintOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL ConstraintOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mpObj->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), mPrevConstraint, FOREVER);
	if(mPrevConstraint != mOverrideConstraint) //if they are the same then enter 0 else go to ti
		mpObj->getParamBlock()->SetValue (ep_constrain_type, TimeValue(0), mOverrideConstraint);
	else
		mpObj->getParamBlock()->SetValue (ep_constrain_type, TimeValue(0), 0);

	return TRUE;
}

BOOL ConstraintOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	mpObj->getParamBlock()->SetValue (ep_constrain_type, TimeValue(0), mPrevConstraint);
	return TRUE;
}

		


//This class handles the overrides for the subobject mode overrides that we support
//it simply saves the subobject mode and then returns it after the override is completed
class SubObjectModeOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mSubObjectMode;
	int mOldSubObjectMode;
	int mOldCommandMode;
	int mPrevPaintMode;
	EditPolyObject *mpObj;
public:
	SubObjectModeOverride(EditPolyObject *m,int id):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mSubObjectMode(id),mOldSubObjectMode(-1),mpObj(m){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL SubObjectModeOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL SubObjectModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	mOldSubObjectMode = mpObj->GetEPolySelLevel();
	mpObj->SetEPolySelLevel(mSubObjectMode);
	return TRUE;
}

BOOL SubObjectModeOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	mpObj->SetEPolySelLevel(mOldSubObjectMode);
	if(mOldCommandMode!=-1)
		mpObj->EpActionToggleCommandMode(mOldCommandMode);
	if(mPrevPaintMode!= PAINTMODE_OFF)
		mpObj->OnPaintBtn( mPrevPaintMode, GetCOREInterface()->GetTime());
	return TRUE;
}

//override for all of the command modes
class CommandModeOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mCommandMode;
	int mOldCommandMode;
	int mPrevPaintMode;
	EditPolyObject *mpObj;
public:
	CommandModeOverride(EditPolyObject *m,int id):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mCommandMode(id),mOldCommandMode(-1),mpObj(m){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL CommandModeOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL CommandModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	mpObj->EpActionToggleCommandMode(mCommandMode);
	return TRUE;
}

BOOL CommandModeOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	if(mOldCommandMode!=-1)
		mpObj->EpActionToggleCommandMode(mOldCommandMode);
	else
		mpObj->EpActionToggleCommandMode(mCommandMode);
	if(mPrevPaintMode!= PAINTMODE_OFF)
		mpObj->OnPaintBtn( mPrevPaintMode, GetCOREInterface()->GetTime());
	return TRUE;
}

//only does the override if in face mode
class FaceCommandModeOverride: public CommandModeOverride
{
public:
	FaceCommandModeOverride(EditPolyObject *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
	
};

BOOL FaceCommandModeOverride::StartOverride()
{
	if (mpObj->GetEPolySelLevel() != EP_SL_FACE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}



//only does the override if in less than eged mode
class LessEdgeCommandModeOverride: public CommandModeOverride
{
public:
	LessEdgeCommandModeOverride(EditPolyObject *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
};

BOOL LessEdgeCommandModeOverride::StartOverride()
{
	if (mpObj->GetEPolySelLevel() > EP_SL_EDGE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}


//only does the override if in less than eged mode
class GreaterEdgeCommandModeOverride: public CommandModeOverride
{
public:
	GreaterEdgeCommandModeOverride(EditPolyObject *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
};

BOOL GreaterEdgeCommandModeOverride::StartOverride()
{
	if (mpObj->GetEPolySelLevel() < EP_SL_EDGE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}


class ChamferModeOverride: public CommandModeOverride
{

public:
	ChamferModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL ChamferModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpObj->EpActionToggleCommandMode (epmode_chamfer_vertex);
			mCommandMode = epmode_chamfer_vertex;
			return TRUE;
		case MNM_SL_EDGE:
			mpObj->EpActionToggleCommandMode (epmode_chamfer_edge);
			mCommandMode = epmode_chamfer_edge;
			return TRUE;
		}

	return FALSE;
}


class CutModeOverride: public CommandModeOverride
{

public:
	CutModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL CutModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetMNSelLevel()) {
	case MNM_SL_VERTEX:
		mpObj->EpActionToggleCommandMode (epmode_cut_vertex);
		mCommandMode= epmode_cut_vertex;
		return TRUE;
	case MNM_SL_EDGE:
		mpObj->EpActionToggleCommandMode (epmode_cut_edge);
		mCommandMode= epmode_cut_edge;
		return TRUE;
	default:
		mpObj->EpActionToggleCommandMode (epmode_cut_face);
		mCommandMode = epmode_cut_face;
		return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}


class CreateModeOverride: public CommandModeOverride
{

public:
	CreateModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL CreateModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetMNSelLevel()) {
	case MNM_SL_VERTEX:
		mpObj->EpActionToggleCommandMode (epmode_create_vertex);
		mCommandMode= epmode_create_vertex;
		return TRUE;
	case MNM_SL_EDGE:
		mpObj->EpActionToggleCommandMode (epmode_create_edge);
		mCommandMode= epmode_create_edge;
		return TRUE;
	default:
		mpObj->EpActionToggleCommandMode (epmode_create_face);
		mCommandMode = epmode_create_face;
		return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}

class InsertModeOverride: public CommandModeOverride 
{
public:
	InsertModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL InsertModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetMNSelLevel()) {
	case MNM_SL_EDGE:
		mpObj->EpActionToggleCommandMode (epmode_divide_edge);
		mCommandMode = epmode_divide_edge;
		return TRUE;
	case MNM_SL_FACE:
		mpObj->EpActionToggleCommandMode (epmode_divide_face);
		mCommandMode = epmode_divide_face;
		return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}
		
class BridgeModeOverride: public CommandModeOverride
{
public:
	BridgeModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};
BOOL BridgeModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetEPolySelLevel()) {
		case EP_SL_BORDER:
			mpObj->EpActionToggleCommandMode (epmode_bridge_border);
			mCommandMode = epmode_bridge_border;
			return TRUE;
		case EP_SL_FACE:
			mpObj->EpActionToggleCommandMode (epmode_bridge_polygon);
			mCommandMode = epmode_bridge_polygon;
			return TRUE;
		case EP_SL_EDGE:
			mpObj->EpActionToggleCommandMode (epmode_bridge_edge);
			mCommandMode = epmode_bridge_edge;
			return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;	
}

class ExtrudeModeOverride: public CommandModeOverride 
{
public:
	ExtrudeModeOverride(EditPolyObject *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();
};

BOOL ExtrudeModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpObj->EpActionGetCommandMode();
	mPrevPaintMode = mpObj->GetPaintMode();
	switch (mpObj->GetEPolySelLevel()) {
		case EP_SL_VERTEX:
			mpObj->EpActionToggleCommandMode (epmode_extrude_vertex);
			mCommandMode= epmode_extrude_vertex;
			return TRUE;
		case EP_SL_EDGE:
		case EP_SL_BORDER:
			mpObj->EpActionToggleCommandMode (epmode_extrude_edge);
			mCommandMode= epmode_extrude_edge;
			return TRUE;
		case EP_SL_FACE:
			mpObj->EpActionToggleCommandMode (epmode_extrude_face);
			mCommandMode = epmode_extrude_face;
			return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}





void GeomDlgProc::UpdateConstraint (HWND hWnd) {
	if (!mpEPoly) return;
	if (!mpEPoly->getParamBlock()) return;

	int constraintType = mpEPoly->getParamBlock()->GetInt (ep_constrain_type);
	switch(constraintType)
	{
	case 0:
		CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_NONE);
		break;
	case 1:
		CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_EDGE);
		break;
	case 2:
		CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_FACE);
		break;
	case 3:
		CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_NORMAL);
		break;

	};

}

INT_PTR GeomDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;
	int constraintType = 0;

	EditPolyObject *polyObject = static_cast<EditPolyObject *>(mpEPoly);
	int constrainType;
	mpEPoly->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
			polyObject->SetLastConstrainType(constrainType);

	switch (msg) {
	case WM_INITDIALOG:
		// Set up the "depressed" color for the command-mode buttons
		but = GetICustButton(GetDlgItem(hWnd,IDC_CREATE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_ATTACH));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);					

		but = GetICustButton(GetDlgItem(hWnd,IDC_SLICEPLANE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		but->SetCheck (mpEPoly->EpfnInSlicePlaneMode());
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_CUT));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		// Luna task 748J
		but = GetICustButton(GetDlgItem(hWnd,IDC_QUICKSLICE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		constraintType = mpEPoly->getParamBlock()->GetInt (ep_constrain_type);
		switch(constraintType)
		{
		case 0:
			CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_NONE);
			break;
		case 1:
			CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_EDGE);
			break;
		case 2:
			CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_FACE);
			break;
		case 3:
			CheckRadioButton(hWnd, IDC_CONSTRAINT_NONE, IDC_CONSTRAINT_NORMAL, IDC_CONSTRAINT_NORMAL);
			break;

		};

		but = GetICustButton (GetDlgItem (hWnd, IDC_ATTACH_LIST));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_ATTACH_LIST));
		ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_MESHSMOOTH));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_TESSELLATE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_RELAX));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_PRESERVE_MAPS));
		if (but)
		{
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		SetEnables(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CONSTRAINT_NONE:
			theHold.Begin ();
			mpEPoly->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
			polyObject->SetLastConstrainType(constrainType);
			mpEPoly->getParamBlock()->SetValue (ep_constrain_type, 0, 0);
			theHold.Accept (GetString (IDS_PARAMETERS));
			break;
		case IDC_CONSTRAINT_EDGE:
			theHold.Begin ();
			mpEPoly->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
			polyObject->SetLastConstrainType(constrainType);
			mpEPoly->getParamBlock()->SetValue (ep_constrain_type, 0, 1);
			theHold.Accept (GetString (IDS_PARAMETERS));
			break;
		case IDC_CONSTRAINT_FACE:
			theHold.Begin ();
			mpEPoly->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
			polyObject->SetLastConstrainType(constrainType);
			mpEPoly->getParamBlock()->SetValue (ep_constrain_type, 0, 2);
			theHold.Accept (GetString (IDS_PARAMETERS));
			break;
		case IDC_CONSTRAINT_NORMAL:
			theHold.Begin ();
			mpEPoly->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
			polyObject->SetLastConstrainType(constrainType);
			mpEPoly->getParamBlock()->SetValue (ep_constrain_type, 0, 3);
			theHold.Accept (GetString (IDS_PARAMETERS));
			break;

		case IDC_SETTINGS_PRESERVE_MAPS:
			thePreserveMapsUIHandler.SetEPoly (mpEditPoly);
			thePreserveMapsUIHandler.DoDialog ();
			break;

		case IDC_CREATE:
			switch (mpEPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEPoly->EpActionToggleCommandMode (epmode_create_vertex);
				break;
			case MNM_SL_EDGE:
				mpEPoly->EpActionToggleCommandMode (epmode_create_edge);
				break;
			default:	// Luna task 748B - Create Polygon available from Object level.
				mpEPoly->EpActionToggleCommandMode (epmode_create_face);
				break;
			}
			break;

		//case IDC_DELETE:
			//mpEPoly->EpActionButtonOp (epop_delete);
			//break;

		case IDC_ATTACH:
			mpEPoly->EpActionEnterPickMode (epmode_attach);
		break;

		case IDC_DETACH:
			if (mpEPoly->GetMNSelLevel() != MNM_SL_OBJECT)
				mpEPoly->EpActionButtonOp (epop_detach);
			break;

		case IDC_ATTACH_LIST:
			mpEPoly->EpActionButtonOp (epop_attach_list);
				break;

		case IDC_COLLAPSE:
			mpEPoly->EpActionButtonOp (epop_collapse);
				break;

		case IDC_CUT:
			// Luna task 748D
			mpEPoly->EpActionToggleCommandMode (epmode_cut_vertex);
			break;

		// Luna task 748J
		case IDC_QUICKSLICE:
			mpEPoly->EpActionToggleCommandMode (epmode_quickslice);
		break;

		case IDC_SLICEPLANE:
			mpEPoly->EpActionToggleCommandMode (epmode_sliceplane);
			break;

		case IDC_SLICE:
			// Luna task 748A - preview mode
			if (mpEditPoly->EpPreviewOn()) {
				mpEditPoly->EpPreviewAccept ();
				mpEditPoly->EpPreviewBegin (epop_slice);
			}
			else mpEPoly->EpActionButtonOp (epop_slice);
			break;

		case IDC_RESET_PLANE:
			mpEPoly->EpActionButtonOp (epop_reset_plane);
			break;

		case IDC_MAKEPLANAR:
			mpEPoly->EpActionButtonOp (epop_make_planar);
			break;

		case IDC_ALIGN_VIEW:
			mpEPoly->EpActionButtonOp (epop_align_view);
			break;

		case IDC_ALIGN_GRID:
			mpEPoly->EpActionButtonOp (epop_align_grid);
			break;

		case IDC_RELAX:
			mpEPoly->EpActionButtonOp (epop_relax);
			break;

		case IDC_SETTINGS_RELAX:
			mpEditPoly->EpfnPopupDialog (epop_relax);
			break;

		case IDC_MAKE_PLANAR_X:
			mpEPoly->EpActionButtonOp (epop_make_planar_x);
			break;

		case IDC_MAKE_PLANAR_Y:
			mpEPoly->EpActionButtonOp (epop_make_planar_y);
			break;

		case IDC_MAKE_PLANAR_Z:
			mpEPoly->EpActionButtonOp (epop_make_planar_z);
			break;

		case IDC_MESHSMOOTH:
			mpEPoly->EpActionButtonOp (epop_meshsmooth);
			break;

		// Luna task 748A - popup dialog
		case IDC_SETTINGS_MESHSMOOTH:
			mpEditPoly->EpfnPopupDialog (epop_meshsmooth);
			break;

		case IDC_TESSELLATE:
			mpEPoly->EpActionButtonOp (epop_tessellate);
				break;

		// Luna task 748A - popup dialog
		case IDC_SETTINGS_TESSELLATE:
			mpEditPoly->EpfnPopupDialog (epop_tessellate);
			break;

		// Luna task 748BB
		case IDC_REPEAT_LAST:
			mpEditPoly->EpfnRepeatLastOperation ();
			break;

		case IDC_HIDE_SELECTED: mpEPoly->EpActionButtonOp (epop_hide); break;
		case IDC_UNHIDEALL: mpEPoly->EpActionButtonOp (epop_unhide); break;
		case IDC_HIDE_UNSELECTED: mpEPoly->EpActionButtonOp (epop_hide_unsel); break;
		case IDC_COPY_NS: mpEPoly->EpActionButtonOp (epop_ns_copy); break;
		case IDC_PASTE_NS: mpEPoly->EpActionButtonOp (epop_ns_paste); break;
		}
				break;

	default:
		return FALSE;
	}

	return TRUE;
}

class SubobjControlDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	EditPolyObject *mpEditPoly;
	bool mUIValid;

public:
	SubobjControlDlgProc () : mpEPoly(NULL), mpEditPoly(NULL), mUIValid(false) { }
	void SetEPoly (EPoly *e) { mpEPoly = e; mpEditPoly = (EditPolyObject *)e; }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void SetEnables (HWND hGeom);
	void UpdatePerDataDisplay (TimeValue t, int selLevel, int dataChannel, HWND hWnd);
	void Invalidate () { mUIValid=false; }
	void DeleteThis() { }
};

static SubobjControlDlgProc theSubobjControlDlgProc;

void SubobjControlDlgProc::SetEnables (HWND hGeom) {
	int sl = mpEPoly->GetEPolySelLevel ();
	int msl = meshSelLevel[sl];
	BOOL edg = (msl == MNM_SL_EDGE);
	BOOL vtx = (sl == EP_SL_VERTEX);
	BOOL fac = (msl == MNM_SL_FACE);
	BOOL brdr = (sl == EP_SL_BORDER);
	BOOL elem = (sl == EP_SL_ELEMENT);
	BOOL obj = (sl == EP_SL_OBJECT);
	ICustButton *but = NULL;

	but = GetICustButton (GetDlgItem (hGeom, IDC_REMOVE));
	if (but != NULL) {
		but->Enable (!brdr);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_CAP));
	if (but != NULL) {
		but->Enable (brdr);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_SPLIT));
	if (but != NULL) {
		but->Enable (!brdr);
		ReleaseICustButton (but);
	}

	// Insert Vertex always active.
	// Break vertex always active.

	// Extrude always active.
	// Bevel always active.
	// Inset always active
	// Outline always active
	// Chamfer always active

	// Target Weld not active at border level:
	but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	if (but != NULL) {
		but->Enable (!brdr);
		ReleaseICustButton (but);
	}

	// Remove Iso Verts  (& Map Verts) always active.
	// Create curve always active.
	// Luna task 748Q - Connect Edges - always active.
	// Luna task 748P - Lift from Edge - always active.
	// Luna task 748T - Extrude along Spline - always active.
}

void SubobjControlDlgProc::UpdatePerDataDisplay (TimeValue t, int selLevel, int dataChannel, HWND hWnd) {
	if (!mpEPoly) return;
	ISpinnerControl *spin=NULL;
	float defaultValue = 1.0f, value = 0.0f;
	int num = 0;
	bool uniform(true);

	switch (selLevel) {
			case EP_SL_VERTEX:
		switch (dataChannel) {
		case VDATA_WEIGHT:
			spin = GetISpinner (GetDlgItem (hWnd, IDC_VS_WEIGHTSPIN));
				break;
		}
		value = mpEPoly->GetVertexDataValue (dataChannel, &num, &uniform, MN_SEL, t);
				break;

	case EP_SL_EDGE:
			case EP_SL_BORDER:
		switch (dataChannel) {
		case EDATA_KNOT:
			spin = GetISpinner (GetDlgItem (hWnd, IDC_ES_WEIGHTSPIN));
				break;
		case EDATA_CREASE:
			spin = GetISpinner (GetDlgItem (hWnd, IDC_ES_CREASESPIN));
			defaultValue = 0.0f;
				break;
			}
		value = mpEPoly->GetEdgeDataValue (dataChannel, &num, &uniform, MN_SEL, t);
			break;
	}

	if (!spin) return;

	if (num == 0) {	// Nothing selected - use default.
		spin->SetValue (defaultValue, false);
		spin->SetIndeterminate (true);
	} else {
		if (!uniform) {	// Data not uniform
			spin->SetIndeterminate (TRUE);
		} else {
			// Set the readout.
			spin->SetIndeterminate(FALSE);
			spin->SetValue (value, FALSE);
		}
	}
	ReleaseISpinner(spin);
}

INT_PTR SubobjControlDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;
	ISpinnerControl* spin = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		// Set up the "depressed" color for the command-mode buttons
		but = GetICustButton(GetDlgItem(hWnd,IDC_INSERT_VERTEX));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_EXTRUDE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_BEVEL));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_OUTLINE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		// Luna task 748R
		but = GetICustButton (GetDlgItem (hWnd, IDC_INSET));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		// Luna task 748Y: Separate Bevel/Chamfer
		but = GetICustButton(GetDlgItem(hWnd,IDC_CHAMFER));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_TARGET_WELD));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		// Luna task 748P 
		but = GetICustButton (GetDlgItem (hWnd, IDC_LIFT_FROM_EDG));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		// Luna task 748T 
		but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_ALONG_SPLINE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		// Set up the weight spinner, if present:
		spin = SetupFloatSpinner (hWnd, IDC_VS_WEIGHTSPIN, IDC_VS_WEIGHT, 0.0f, 9999999.0f, 1.0f, .1f);
		if (spin) {
			spin->SetAutoScale(TRUE);
			ReleaseISpinner (spin);
			spin = NULL;
		}

		// Set up the edge data spinners, if present:
		spin = SetupFloatSpinner (hWnd, IDC_ES_WEIGHTSPIN, IDC_ES_WEIGHT, 0.0f, 9999999.0f, 1.0f, .1f);
		if (spin) {
			spin->SetAutoScale(TRUE);
			ReleaseISpinner (spin);
			spin = NULL;
		}

		spin = SetupFloatSpinner (hWnd, IDC_ES_CREASESPIN, IDC_ES_CREASE, 0.0f, 1.0f, 1.0f, .1f);
		if (spin) {
			spin->SetAutoScale(TRUE);
			ReleaseISpinner (spin);
			spin = NULL;
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_FS_EDIT_TRI));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			if (mpEPoly->GetMNSelLevel() == MNM_SL_EDGE)
				but->SetTooltip (true, GetString (IDS_EDIT_TRIANGULATION));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_TURN_EDGE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BRIDGE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_EXTRUDE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_WELD));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CHAMFER));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BREAK));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CONNECT_EDGES));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BREAK));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_OUTLINE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_INSET));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BEVEL));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_LIFT_FROM_EDGE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_EXTRUDE_ALONG_SPLINE));
		if (but) {
#ifdef USE_POPUP_DIALOG_ICON
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
#endif
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		SetEnables(hWnd);
		mUIValid = false;
				break;

	case WM_PAINT:
		if (mUIValid) return FALSE;
		UpdatePerDataDisplay (t, EP_SL_VERTEX, VDATA_WEIGHT, hWnd);
		UpdatePerDataDisplay (t, EP_SL_EDGE, EDATA_KNOT, hWnd);
		UpdatePerDataDisplay (t, EP_SL_EDGE, EDATA_CREASE, hWnd);
		mUIValid = true;
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_REMOVE:
			mpEPoly->EpActionButtonOp (epop_remove);
				break;

		case IDC_BREAK:
			mpEPoly->EpActionButtonOp (epop_break);
			break;

		case IDC_SETTINGS_BREAK :
			mpEditPoly->EpfnPopupDialog (epop_break);
			break;

		case IDC_SPLIT:
			mpEPoly->EpActionButtonOp (epop_split);
				break;

		case IDC_INSERT_VERTEX:
			switch (mpEPoly->GetMNSelLevel()) {
			case MNM_SL_EDGE:
				mpEPoly->EpActionToggleCommandMode (epmode_divide_edge);
				break;
			case MNM_SL_FACE:
				mpEPoly->EpActionToggleCommandMode (epmode_divide_face);
				break;
			}
			break;

		case IDC_CAP:
			if (mpEPoly->GetEPolySelLevel() == EP_SL_BORDER)
				mpEPoly->EpActionButtonOp (epop_cap);
			break;

		case IDC_EXTRUDE:
			switch (mpEPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEPoly->EpActionToggleCommandMode (epmode_extrude_vertex);
				break;
			case MNM_SL_EDGE:
				mpEPoly->EpActionToggleCommandMode (epmode_extrude_edge);
				break;
			case MNM_SL_FACE:
				mpEPoly->EpActionToggleCommandMode (epmode_extrude_face);
				break;
			}
			break;

		// Luna task 748A - New popup dialog management
		case IDC_SETTINGS_EXTRUDE:
			mpEditPoly->EpfnPopupDialog (epop_extrude);
			break;

		case IDC_BEVEL:
			// Luna task 748Y - separated Bevel and Chamfer
			mpEPoly->EpActionToggleCommandMode (epmode_bevel);
				break;

		// Luna task 748A - New popup dialog management
		case IDC_SETTINGS_BEVEL:
			mpEditPoly->EpfnPopupDialog (epop_bevel);
				break;

		// Luna task 748R
		case IDC_OUTLINE:
			mpEPoly->EpActionToggleCommandMode (epmode_outline);
				break;

		// Luna task 748R
		case IDC_SETTINGS_OUTLINE:
			mpEditPoly->EpfnPopupDialog (epop_outline);
			break;

		// Luna task 748R
		case IDC_INSET:
			mpEPoly->EpActionToggleCommandMode (epmode_inset_face);
			break;

		// Luna task 748R
		case IDC_SETTINGS_INSET:
			mpEditPoly->EpfnPopupDialog (epop_inset);
			break;

		case IDC_CHAMFER:
			switch (mpEPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEPoly->EpActionToggleCommandMode (epmode_chamfer_vertex);
				break;
			case MNM_SL_EDGE:
				mpEPoly->EpActionToggleCommandMode (epmode_chamfer_edge);
				break;
			}
			break;

		// Luna task 748A - New popup dialog management
		case IDC_SETTINGS_CHAMFER:
			mpEditPoly->EpfnPopupDialog (epop_chamfer);
			break;

		case IDC_WELD:
			mpEPoly->EpActionButtonOp (epop_weld_sel);
			break;

		// Luna task 748A - New popup dialog management
		case IDC_SETTINGS_WELD:
			mpEditPoly->EpfnPopupDialog (epop_weld_sel);
			break;

		case IDC_TARGET_WELD:
			mpEPoly->EpActionToggleCommandMode (epmode_weld);
			break;

		case IDC_REMOVE_ISO_VERTS:
			mpEPoly->EpActionButtonOp (epop_remove_iso_verts);
			break;

		case IDC_REMOVE_ISO_MAP_VERTS:
			mpEPoly->EpActionButtonOp (epop_remove_iso_map_verts);
			break;

		case IDC_CREATE_CURVE:
			mpEPoly->EpActionButtonOp (epop_create_shape);
			break;

		// Luna task 748Q
		case IDC_CONNECT_EDGES:
			mpEPoly->EpActionButtonOp (epop_connect_edges);
			break;

		case IDC_CONNECT_VERTICES:
			mpEPoly->EpActionButtonOp (epop_connect_vertices);
			break;

		case IDC_SETTINGS_CONNECT_EDGES:
			mpEditPoly->EpfnPopupDialog (epop_connect_edges);
			break;

		// Luna task 748P 
		case IDC_LIFT_FROM_EDG:
			mpEPoly->EpActionToggleCommandMode (epmode_lift_from_edge);
			break;

		case IDC_SETTINGS_LIFT_FROM_EDGE:
			mpEditPoly->EpfnPopupDialog (epop_lift_from_edge);
			break;

		//	Luna task 748T
		case IDC_EXTRUDE_ALONG_SPLINE:
			mpEPoly->EpActionEnterPickMode (epmode_pick_shape);
			break;

		case IDC_SETTINGS_EXTRUDE_ALONG_SPLINE:
			mpEditPoly->EpfnPopupDialog (epop_extrude_along_spline);
			break;

		case IDC_FS_EDIT_TRI:
			mpEPoly->EpActionToggleCommandMode (epmode_edit_tri);
			break;

		case IDC_TURN_EDGE:
			mpEPoly->EpActionToggleCommandMode (epmode_turn_edge);
			break;

		case IDC_BRIDGE:
			if (mpEditPoly->EpfnReadyToBridgeFlagged (EP_SL_CURRENT, MN_SEL)) 
			{
				theHold.Begin ();
				mpEPoly->getParamBlock()->SetValue (ep_bridge_selected, TimeValue(0), true);
				if (mpEPoly->GetEPolySelLevel() == EP_SL_BORDER) {
					mpEPoly->EpActionButtonOp (epop_bridge_border);
					theHold.Accept (GetString (IDS_BRIDGE_BORDERS));
				} else if (mpEPoly->GetEPolySelLevel() == EP_SL_FACE) {
					mpEPoly->EpActionButtonOp (epop_bridge_polygon);
					theHold.Accept (GetString (IDS_BRIDGE_POLYGONS));
				} else {
					// this is the edge bridging 
					mpEPoly->EpActionButtonOp (epop_bridge_edge);
					theHold.Accept (GetString (IDS_BRIDGE_EDGES));
				}

				// Don't leave the button pressed-in.
				but = GetICustButton(GetDlgItem(hWnd,IDC_BRIDGE));
				if (but) {
					but->SetCheck (false);
					ReleaseICustButton (but);
				}
			} 
			else 
			{
				switch (mpEPoly->GetEPolySelLevel()) 
				{
				case EP_SL_BORDER: 
					mpEPoly->EpActionToggleCommandMode (epmode_bridge_border); 
					break;
				case EP_SL_FACE: 
					mpEPoly->EpActionToggleCommandMode (epmode_bridge_polygon); 
					break;
				case EP_SL_EDGE: 
					mpEPoly->EpActionToggleCommandMode (epmode_bridge_edge); 
					break;
				}
			}
			break;

		case IDC_SETTINGS_BRIDGE:
			switch (mpEPoly->GetEPolySelLevel()) 
			{
				case EP_SL_BORDER: 
					mpEditPoly->EpfnPopupDialog (epop_bridge_border);
					break;
				case EP_SL_FACE: 
					mpEditPoly->EpfnPopupDialog (epop_bridge_polygon); 
					break;
				case EP_SL_EDGE: 
					mpEditPoly->EpfnPopupDialog (epop_bridge_edge); 
					break;
			}
			break;

		case IDC_FS_RETRIANGULATE:
			mpEPoly->EpActionButtonOp (epop_retriangulate);
			break;

		case IDC_FS_FLIP_NORMALS:
			mpEPoly->EpActionButtonOp (epop_flip_normals);
		break;

		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->numv) break;
			mpEPoly->BeginPerDataModify (MNM_SL_VERTEX, VDATA_WEIGHT);
			break;
		case IDC_ES_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			mpEPoly->BeginPerDataModify (MNM_SL_EDGE, EDATA_KNOT);
			break;
		case IDC_ES_CREASESPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			mpEPoly->BeginPerDataModify (MNM_SL_EDGE, EDATA_CREASE);
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHT:
			if (!mpEPoly->GetMeshPtr()->numv) break;
			theHold.Begin ();
			mpEPoly->EndPerDataModify (true);
			theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			break;

		case IDC_ES_WEIGHT:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			theHold.Begin ();
			mpEPoly->EndPerDataModify (true);
			theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			break;

		case IDC_ES_CREASE:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			theHold.Begin ();
			mpEPoly->EndPerDataModify (true);
			theHold.Accept (GetString(IDS_CHANGE_CREASE_VALS));
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->numv) break;
			pmap->RedrawViews (t, REDRAW_END);
			if (HIWORD(wParam)) {
				theHold.Begin ();
				mpEPoly->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			} else {
				mpEPoly->EndPerDataModify (false);
			}
			break;

		case IDC_ES_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			pmap->RedrawViews (t,REDRAW_END);
			if (HIWORD(wParam)) {
				theHold.Begin ();
				mpEPoly->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			} else {
				mpEPoly->EndPerDataModify (false);
			}
			break;

		case IDC_ES_CREASESPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			pmap->RedrawViews (t,REDRAW_END);
			if (HIWORD(wParam)) {
				theHold.Begin ();
				mpEPoly->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGE_CREASE_VALS));
			} else {
				mpEPoly->EndPerDataModify (false);
			}
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->numv) break;
			mpEPoly->SetVertexDataValue (VDATA_WEIGHT, spin->GetFVal(), MN_SEL, t);
			break;
		case IDC_ES_WEIGHTSPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			mpEPoly->SetEdgeDataValue (EDATA_KNOT, spin->GetFVal(), MN_SEL, t);
			break;
		case IDC_ES_CREASESPIN:
			if (!mpEPoly->GetMeshPtr()->nume) break;
			mpEPoly->SetEdgeDataValue (EDATA_CREASE, spin->GetFVal(), MN_SEL, t);
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// Luna task 748Y - UI redesign - eliminate Subdivide dialog proc; make "Surface" into "Subdivision" one.
class SubdivisionDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	bool mUIValid;

public:
	SubdivisionDlgProc () : mpEPoly(NULL), mUIValid(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void UpdateEnables (HWND hWnd, TimeValue t);
	void DeleteThis() { }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
	void InvalidateUI (HWND hWnd) { mUIValid = false; if (hWnd) InvalidateRect (hWnd, NULL, false); }
};

static SubdivisionDlgProc theSubdivisionDlgProc;

void SubdivisionDlgProc::UpdateEnables (HWND hWnd, TimeValue t) {
	IParamBlock2 *pblock = mpEPoly->getParamBlock ();
	if (pblock == NULL) return;
#ifndef NO_OUTPUTRENDERER
	int useRIter, useRSmooth;
	pblock->GetValue (ep_surf_use_riter, t, useRIter, FOREVER);
	pblock->GetValue (ep_surf_use_rthresh, t, useRSmooth, FOREVER);
	EnableWindow (GetDlgItem (hWnd, IDC_RENDER_ITER_LABEL), useRIter);
	EnableWindow (GetDlgItem (hWnd, IDC_RENDER_SMOOTHNESS_LABEL), useRSmooth);
#endif
	int updateType;
	pblock->GetValue (ep_surf_update, t, updateType, FOREVER);
	ICustButton *but = GetICustButton (GetDlgItem (hWnd, IDC_RECALC));
	but->Enable (updateType);
	ReleaseICustButton (but);
}

INT_PTR SubdivisionDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	IColorSwatch* iCol = NULL;

	switch (msg) {
	case WM_INITDIALOG:
			if (mpEPoly )
			{
				// init the color swatches 

				Point3 l_cageCol			= mpEPoly->getParamBlock()->GetPoint3(ep_cage_color,TimeValue(0) );
				Point3 l_selectedCageCol	= mpEPoly->getParamBlock()->GetPoint3(ep_selected_cage_color,TimeValue(0) );

				EditPolyObject* l_pEditPoly = (	EditPolyObject*) mpEPoly;

				l_pEditPoly->miColorCageSwatch			= GetIColorSwatch(GetDlgItem(hWnd, IDC_CAGE_COLOR_SWATCH));
				l_pEditPoly->miColorSelectedCageSwatch	= GetIColorSwatch(GetDlgItem(hWnd, IDC_SELECTED_CAGE_COLOR_SWATCH));

				l_pEditPoly->miColorCageSwatch->SetColor(l_cageCol, FALSE);

				l_pEditPoly->miColorSelectedCageSwatch->SetColor(l_selectedCageCol, FALSE);
				l_pEditPoly->miColorSelectedCageSwatch->SetColor(GetUIColor(COLOR_SEL_GIZMOS), FALSE);

				l_pEditPoly->miColorCageSwatch->SetModal();
				l_pEditPoly->miColorSelectedCageSwatch->SetModal();
#if 0 
				// tests for tool tip handling 
				ICustButton* btn = GetICustButton(GetDlgItem(hWnd,IDC_CAGE_COLOR_SWATCH));
				if ( btn )
				{
					TCHAR* tooltip = "test ";
					btn->SetTooltip( TRUE, tooltip );

				}
#endif 

			}
		break;
	case WM_CLOSE:
		{
			if (mpEPoly )
			{
				EditPolyObject* l_pEditPoly = (	EditPolyObject*) mpEPoly;

				ReleaseIColorSwatch(l_pEditPoly->miColorCageSwatch);
				l_pEditPoly->miColorCageSwatch = NULL;
				ReleaseIColorSwatch(l_pEditPoly->miColorSelectedCageSwatch);
				l_pEditPoly->miColorSelectedCageSwatch = NULL;
			}
			break;
		}
	case WM_PAINT:
		if (mUIValid) break;
		UpdateEnables (hWnd, t);
		mUIValid = true;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RECALC:
			mpEPoly->EpfnForceSubdivision ();
			break;
		case IDC_USE_RITER:
		case IDC_USE_RSHARP:
		case IDC_UPDATE_ALWAYS:
		case IDC_UPDATE_RENDER:
		case IDC_UPDATE_MANUAL:
			UpdateEnables (hWnd, t);
			break;
		}
#if 0 //for tooltip handling 
	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) 
		{
			case IDC_CAGE_COLOR_SWATCH:
				lpttt->lpszText = GetString (IDS_VERTEX);
				break;
			case IDC_SELECTED_CAGE_COLOR_SWATCH:
				lpttt->lpszText = GetString (IDS_EDGE);
				break;
		}
		break;
#endif 

	}
	return FALSE;
}

static void SetSmoothButtonState (HWND hWnd,DWORD bits,DWORD invalid,DWORD unused=0) {
	for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
		if ( (unused&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			ShowWindow(GetDlgItem(hWnd,i),SW_HIDE);
			continue;
		}

		if ( (invalid&(1<<(i-IDC_SMOOTH_GRP1))) ) {
			SetWindowText(GetDlgItem(hWnd,i),NULL);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,FALSE);
		} else {
			TSTR buf;
			buf.printf(_T("%d"),i-IDC_SMOOTH_GRP1+1);
			SetWindowText(GetDlgItem(hWnd,i),buf);
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_STATE,
				(bits&(1<<(i-IDC_SMOOTH_GRP1)))?TRUE:FALSE);
		}
		InvalidateRect(GetDlgItem(hWnd,i),NULL,TRUE);
	}
}

static DWORD selectBySmoothGroups = 0;
static bool selectBySmoothClear = true;

static INT_PTR CALLBACK SelectBySmoothDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static DWORD *param;
	ICustButton* iBut = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);
		param = (DWORD*)lParam;
		for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
		SetSmoothButtonState(hWnd,selectBySmoothGroups,0,param[0]);
		CheckDlgButton(hWnd,IDC_CLEARSELECTION, selectBySmoothClear);
		break;

	case WM_COMMAND: 
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
			if (iBut->IsChecked()) selectBySmoothGroups |= 1<<shift;
			else selectBySmoothGroups &= ~(1<<shift);
			ReleaseICustButton(iBut);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDOK:
			thePopupPosition.Scan (hWnd);
			selectBySmoothClear = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION) ? true : false;
			EndDialog(hWnd,1);					
			break;

		case IDCANCEL:
			thePopupPosition.Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;			

	default:
		return FALSE;
	}
	return TRUE;
}

bool GetSelectBySmoothParams (Interface *ip, DWORD usedBits, DWORD & smG, bool & clear) {
	DWORD buttonsToUse = ~usedBits;
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EP_SELECTBYSMOOTH),
		ip->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)&buttonsToUse)) {
		smG = selectBySmoothGroups;
		clear = selectBySmoothClear;
		return true;
	}
	return false;
}

static MtlID selectByMatID = 1;
static bool selectByMatClear = true;

bool GetSelectByMaterialParams (EditPolyObject *ep, MtlID & matId, bool & clear) {       
	HWND hWnd  = ep->GetDlgHandle(ep_face);
	if (hWnd){
		ISpinnerControl* spin = NULL;
		spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN_SEL));
		selectByMatID = spin->IsIndeterminate() ? -1 : (MtlID) spin->GetIVal();
		selectByMatClear = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION) ? true : false;
		ReleaseISpinner(spin);
		matId = selectByMatID;
		clear = selectByMatClear;
		return true;
	}
	return false;
}

class VertexDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	bool uiValid;

public:
	VertexDlgProc () : mpEPoly(NULL), uiValid(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void UpdateAlphaDisplay (HWND hWnd, TimeValue t);
	void Invalidate () { uiValid=false; }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static VertexDlgProc theVertexDlgProc;

void VertexDlgProc::UpdateAlphaDisplay (HWND hWnd, TimeValue t) {
	HWND hSpin = GetDlgItem (hWnd, IDC_VERT_ALPHASPIN);
	if (!hSpin) return;

	int num;
	bool uniform;
	float alpha;
	alpha = mpEPoly->GetVertexColor (&uniform, &num, MAP_ALPHA, MN_SEL, t).r;

	ISpinnerControl *spin = GetISpinner (hSpin);
	spin->SetIndeterminate (!uniform);
	spin->SetValue (alpha*100.0f, FALSE);
	ReleaseISpinner (spin);
}

INT_PTR VertexDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							 UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;
	IColorSwatch* iCol = NULL;
	BaseInterface* pInterface = NULL;
	Color selByColor;

	switch (msg) {
	case WM_INITDIALOG:
		spin = SetupFloatSpinner (hWnd, IDC_VERT_ALPHASPIN,
			IDC_VERT_ALPHA, 0.0f, 100.0f, 100.0f, .1f);
		ReleaseISpinner (spin);
		spin = NULL;
		// Cue an update based on the current selection
		uiValid = FALSE;
		break;

	case WM_PAINT:
		if (uiValid) return FALSE;
		iCol = GetIColorSwatch (GetDlgItem (hWnd,IDC_VERT_COLOR),
			mpEPoly->GetVertexColor (NULL, NULL, 0, MN_SEL, t),
			GetString (IDS_VERTEX_COLOR));
		ReleaseIColorSwatch(iCol);
		iCol = NULL;

		iCol = GetIColorSwatch (GetDlgItem (hWnd,IDC_VERT_ILLUM),
			mpEPoly->GetVertexColor (NULL, NULL, MAP_SHADING, MN_SEL, t),
			GetString (IDS_VERTEX_ILLUM));
		ReleaseIColorSwatch(iCol);
		iCol = NULL;
		
		UpdateAlphaDisplay (hWnd, t);

		int byIllum;
		mpEPoly->getParamBlock()->GetValue (ep_vert_sel_color, t, selByColor, FOREVER);
		mpEPoly->getParamBlock()->GetValue (ep_vert_color_selby, t, byIllum, FOREVER);
		iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_SELCOLOR), selByColor,
			GetString (byIllum ? IDS_SEL_BY_ILLUM : IDS_SEL_BY_COLOR));
		ReleaseIColorSwatch (iCol);
		iCol = NULL;
		uiValid = TRUE;
		return FALSE;

	case CC_COLOR_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			theHold.Begin ();
			mpEPoly->BeginVertexColorModify (0);
			break;
		case IDC_VERT_ILLUM:
			theHold.Begin ();
			mpEPoly->BeginVertexColorModify (MAP_SHADING);
			break;
		}
		break;

	case CC_COLOR_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			mpEPoly->EndVertexColorModify (HIWORD(wParam) ? true : false);
			if (HIWORD(wParam)) theHold.Accept (GetString(IDS_SET_VERT_COLOR));
			else theHold.Cancel ();
			break;
		case IDC_VERT_ILLUM:
			mpEPoly->EndVertexColorModify (HIWORD(wParam) ? true : false);
			if (HIWORD(wParam)) theHold.Accept (GetString(IDS_SET_VERT_ILLUM));
			else theHold.Cancel ();
			break;
		}
		break;

	case CC_COLOR_CHANGE:
		iCol = (IColorSwatch*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			mpEPoly->SetVertexColor (Color(iCol->GetColor()), 0, MN_SEL, t);
			break;
		case IDC_VERT_ILLUM:
			mpEPoly->SetVertexColor (Color(iCol->GetColor()), MAP_SHADING, MN_SEL, t);
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			mpEPoly->BeginPerDataModify (MNM_SL_VERTEX, VDATA_WEIGHT);
			break;
		case IDC_VERT_ALPHASPIN:
			theHold.Begin ();
			mpEPoly->BeginVertexColorModify (MAP_ALPHA);
		}
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHT:
			map->RedrawViews (t, REDRAW_END);
			theHold.Begin ();
			mpEPoly->EndPerDataModify (true);
			theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			break;

		case IDC_VERT_ALPHA:
			map->RedrawViews (t, REDRAW_END);
			mpEPoly->EndVertexColorModify (true);
			theHold.Accept (GetString (IDS_SET_VERT_ALPHA));
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			map->RedrawViews (t, REDRAW_END);
			if (HIWORD(wParam)) {
				theHold.Begin ();
				mpEPoly->EndPerDataModify (true);
				theHold.Accept (GetString(IDS_CHANGEWEIGHT));
			} else {
				mpEPoly->EndPerDataModify (false);
			}
			break;

		case IDC_VERT_ALPHASPIN:
			if (HIWORD(wParam)) {
				mpEPoly->EndVertexColorModify (true);
				theHold.Accept (GetString (IDS_SET_VERT_ALPHA));
			} else {
				mpEPoly->EndVertexColorModify (false);
				theHold.Cancel();
			}
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VS_WEIGHTSPIN:
			mpEPoly->SetVertexDataValue (VDATA_WEIGHT, spin->GetFVal(), MN_SEL, t);
			break;
		case IDC_VERT_ALPHASPIN:
			if (!mpEPoly->InVertexColorModify ()) {
				theHold.Begin ();
				mpEPoly->BeginVertexColorModify (MAP_ALPHA);
			}
			float alpha;
			alpha = spin->GetFVal()/100.0f;
			Color clr;
			clr = Color(alpha,alpha,alpha);
			mpEPoly->SetVertexColor (clr, MAP_ALPHA, MN_SEL, t);
			break;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == 1) return FALSE;	// not handling keyboard shortcuts here.

		switch (LOWORD(wParam)) {
		case IDC_VERT_SELBYCOLOR:
			mpEPoly->EpActionButtonOp (epop_selby_vc);
			break;
		case IDC_SEL_BY_COLOR:
		case IDC_SEL_BY_ILLUM:
			mpEPoly->getParamBlock()->GetValue (ep_vert_sel_color, t, selByColor, FOREVER);
			mpEPoly->getParamBlock()->GetValue (ep_vert_color_selby, t, byIllum, FOREVER);
			iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_SELCOLOR), selByColor,
				GetString (byIllum ? IDS_SEL_BY_ILLUM : IDS_SEL_BY_COLOR));
			pInterface = iCol->GetInterface (COLOR_SWATCH_RENAMER_INTERFACE_51);
			if (pInterface) {
				IColorSwatchRenamer *pRenamer = (IColorSwatchRenamer *) pInterface;
				pRenamer->SetName (GetString (byIllum ? IDS_SEL_BY_ILLUM : IDS_SEL_BY_COLOR));
			}
			ReleaseIColorSwatch (iCol);
			iCol = NULL;
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

class FaceDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	bool uiValid, klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel;

public:
	FaceDlgProc () : mpEPoly(NULL), uiValid(false), klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	// Luna task 748I - Flip Normals always available.
	void DeleteThis() { }
	void Invalidate () { uiValid=false; }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

class PolySmoothDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	bool uiValid, klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel;

public:
	PolySmoothDlgProc () : mpEPoly(NULL), uiValid(false), klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	// Luna task 748I - Flip Normals always available.
	void DeleteThis() { }
	void Invalidate () { uiValid=false; }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

class PolyColorDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
	bool uiValid, klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel;

public:
	PolyColorDlgProc () : mpEPoly(NULL), uiValid(false), klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	// Luna task 748I - Flip Normals always available.
	void DeleteThis() { }
	void UpdateAlphaDisplay (HWND hWnd, TimeValue t);
	void Invalidate () { uiValid=false; }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static FaceDlgProc theFaceDlgProc;
static PolySmoothDlgProc thePolySmoothDlgProc; //since we broke this dialog up we can still reuse the dlg proc for the 3 dialogs!! yeah!
static PolyColorDlgProc thePolyColorDlgProc;

void PolyColorDlgProc::UpdateAlphaDisplay (HWND hWnd, TimeValue t) {
	HWND hSpin = GetDlgItem (hWnd, IDC_VERT_ALPHASPIN);
	if (!hSpin) return;

	int num;
	bool uniform;
	float alpha;
	alpha = mpEPoly->GetFaceColor (&uniform, &num, MAP_ALPHA, MN_SEL, t).r;

	ISpinnerControl *spin = GetISpinner (hSpin);
	spin->SetIndeterminate (!uniform);
	spin->SetValue (alpha*100.0f, FALSE);
	ReleaseISpinner (spin);
}

INT_PTR FaceDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;
	IColorSwatch* iCol = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		spin = SetupIntSpinner (hWnd, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, 0);
		if(spin)
			ReleaseISpinner(spin);
		spin = NULL;
		spin = SetupIntSpinner (hWnd, IDC_MAT_IDSPIN_SEL, IDC_MAT_ID_SEL, 1, MAX_MATID, 0);     
		if(spin)
			ReleaseISpinner(spin);
		spin = NULL;
		CheckDlgButton(hWnd, IDC_CLEARSELECTION, 1);                                 
		SetupMtlSubNameCombo (hWnd, (EditPolyObject*)mpEPoly);           
		// Cue an update based on the current face selection.
		uiValid = FALSE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = true;
		break;

	case WM_PAINT:
		if (uiValid) return FALSE;
		// Display the correct material index:
		MtlID mat;
		bool determined;
		mat = MtlID(mpEPoly->GetMatIndex (&determined));
		spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
		if(spin)
		{
			if (!determined) {
				spin->SetIndeterminate(TRUE);
			} else {
				spin->SetIndeterminate(FALSE);
				spin->SetValue (int(mat+1), FALSE);
			}
			ReleaseISpinner(spin);
		}

		spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN_SEL)); 
		if(spin)
		{
			if (!determined) {
				spin->SetIndeterminate(TRUE);
			} else {
				spin->SetIndeterminate(FALSE);
				spin->SetValue (int(mat+1), FALSE);
			}
			ReleaseISpinner(spin);  
		}
		if (GetDlgItem (hWnd, IDC_MTLID_NAMES_COMBO)) { 
			ValidateUINameCombo(hWnd, (EditPolyObject*)mpEPoly);   
		}                             

		// Luna task 748I - Flip normals always available.

		uiValid = TRUE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = false;
		return FALSE;


	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			theHold.Begin ();
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_MAT_ID:
		case IDC_MAT_IDSPIN:
			// For some reason, there's a WM_CUSTEDIT_ENTER sent on IDC_MAT_ID
			// when we start this dialog up.  Use this variable to suppress its activity.
			if (klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel) break;
			mpEPoly->LocalDataChanged (PART_TOPO);
			mpEPoly->RefreshScreen ();
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				 theHold.Accept(GetString(IDS_ASSIGN_MATID));
			else theHold.Cancel();
			break;

			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			if (!theHold.Holding()) theHold.Begin();
			mpEPoly->SetMatIndex(spin->GetIVal()-1, MN_SEL);
			break;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == 1) return FALSE;	// not handling keyboard shortcuts here.
		
		switch (LOWORD(wParam)) {
		case IDC_SELECT_BYID:
			mpEPoly->EpActionButtonOp (epop_selby_matid);
			break;
		case IDC_MTLID_NAMES_COMBO:
			switch(HIWORD(wParam)){
			case CBN_SELENDOK:
				int index, val;
				index = SendMessage(GetDlgItem(hWnd,IDC_MTLID_NAMES_COMBO), CB_GETCURSEL, 0, 0);
				val = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
				if (index != CB_ERR){
					selectByMatID = val;
					selectByMatClear = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION) ? true : false;
					EditPolyObject *epo = (EditPolyObject *) mpEPoly;	
					theHold.Begin();
					epo->EpfnSelectByMat(selectByMatID, selectByMatClear, t);
					theHold.Accept(GetString(IDS_SELECT_BY_MATID));
					epo->RefreshScreen();
				}
				break;                                                    
			}
			break;
		
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}


INT_PTR PolySmoothDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;
	IColorSwatch* iCol = NULL;

	switch (msg) {
	case WM_INITDIALOG:
         
		// NOTE: the following requires that the smoothing group ID's be sequential!
		for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
			SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,
				CC_CMD_SET_TYPE,CBT_CHECK);
		}

		// Cue an update based on the current face selection.
		uiValid = FALSE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = true;
		break;

	case WM_PAINT:
		if (uiValid) return FALSE;
		// Display the correct smoothing groups for the current selection:
		DWORD invalid, bits;
		mpEPoly->GetSmoothingGroups (MN_SEL, &invalid, &bits);
		invalid -= bits;
		SetSmoothButtonState(hWnd,bits,invalid);
		// Luna task 748I - Flip normals always available.

		uiValid = TRUE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = false;
		return FALSE;

	
	case WM_COMMAND:
		if (HIWORD(wParam) == 1) return FALSE;	// not handling keyboard shortcuts here.
		if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
			LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));
			int bit = iBut->IsChecked() ? 1 : 0;
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
			// NOTE: Clumsy - what should really happen, is EPoly::SetSmoothBits should be type bool.
			EditPolyObject *epo = (EditPolyObject *) mpEPoly;
			if (!epo->LocalSetSmoothBits(bit<<shift, 1<<shift, MN_SEL))
				iBut->SetCheck (false);
			ReleaseICustButton(iBut);
			break;
		}
		switch (LOWORD(wParam)) {
		
		case IDC_SELECTBYSMOOTH:
			mpEPoly->EpActionButtonOp (epop_selby_smg);
			break;

		case IDC_SMOOTH_CLEAR:
			mpEPoly->EpActionButtonOp (epop_clear_smg);
			break;

		case IDC_SMOOTH_AUTO:
			mpEPoly->EpActionButtonOp (epop_autosmooth); break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}



INT_PTR PolyColorDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;
	IColorSwatch* iCol = NULL;
	int stringID = 0;

	switch (msg) {
	case WM_INITDIALOG:
		     
		spin = SetupFloatSpinner (hWnd, IDC_VERT_ALPHASPIN,
			IDC_VERT_ALPHA, 0.0f, 100.0f, 100.0f, .1f);
		if(spin)
			ReleaseISpinner (spin);
		spin = NULL;
		// Cue an update based on the current face selection.
		uiValid = FALSE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = true;
		break;

	case WM_PAINT:
		if (uiValid) return FALSE;
		       

		// Set the correct vertex color...
		iCol = GetIColorSwatch (GetDlgItem(hWnd,IDC_VERT_COLOR),
			mpEPoly->GetFaceColor (NULL, NULL, 0, MN_SEL, t),
			GetString(IDS_VERTEX_COLOR));
		if(iCol)
			ReleaseIColorSwatch(iCol);
		iCol = NULL;
		// ...and vertex illumination...
		iCol = GetIColorSwatch (GetDlgItem (hWnd,IDC_VERT_ILLUM),
			mpEPoly->GetFaceColor (NULL, NULL, MAP_SHADING, MN_SEL, t),
			GetString (IDS_VERTEX_ILLUM));
		if(iCol)
			ReleaseIColorSwatch(iCol);
		iCol = NULL;

		// ...and vertex alpha for the selected faces:
		UpdateAlphaDisplay (hWnd, t);

		// Luna task 748I - Flip normals always available.

		uiValid = TRUE;
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = false;
		return FALSE;

	case CC_COLOR_BUTTONDOWN:
		theHold.Begin ();
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR: mpEPoly->BeginVertexColorModify (0); break;
		case IDC_VERT_ILLUM: mpEPoly->BeginVertexColorModify (MAP_SHADING); break;\
		}
		break;

	case CC_COLOR_BUTTONUP:
		stringID = 0;
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			stringID = IDS_SET_VERT_COLOR;
			break;
		case IDC_VERT_ILLUM:
			stringID = IDS_SET_VERT_ILLUM;
			break;
		}
		mpEPoly->EndVertexColorModify (HIWORD(wParam) ? true : false);
		if (HIWORD(wParam)) theHold.Accept (GetString(stringID));
		else theHold.Cancel ();
		break;

	case CC_COLOR_CHANGE:
		iCol = (IColorSwatch*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_VERT_COLOR:
			mpEPoly->SetFaceColor (Color(iCol->GetColor()), 0, MN_SEL, t);
			break;
		case IDC_VERT_ILLUM:
			mpEPoly->SetFaceColor (Color(iCol->GetColor()), MAP_SHADING, MN_SEL, t);
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		
		case IDC_VERT_ALPHASPIN:
			theHold.Begin ();
			mpEPoly->BeginVertexColorModify (MAP_ALPHA);
		}
		break;

	case WM_CUSTEDIT_ENTER:
	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		

		case IDC_VERT_ALPHA:
		case IDC_VERT_ALPHASPIN:
			if (HIWORD(wParam) || (LOWORD(wParam)==IDC_VERT_ALPHA)) {
				mpEPoly->EndVertexColorModify (true);
				theHold.Accept (GetString (IDS_SET_VERT_ALPHA));
			} else {
				mpEPoly->EndVertexColorModify (false);
				theHold.Cancel();
			}
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		
		case IDC_VERT_ALPHASPIN:
			if (!mpEPoly->InVertexColorModify ()) {
				theHold.Begin ();
				mpEPoly->BeginVertexColorModify (MAP_ALPHA);
			}
			float alpha;
			alpha = spin->GetFVal()/100.0f;
			Color clr;
			clr = Color(alpha,alpha,alpha);
			mpEPoly->SetFaceColor (clr, MAP_ALPHA, MN_SEL, t);
			break;
		}
		break;


	default:
		return FALSE;
	}

	return TRUE;
}


class AdvancedDisplacementDlgProc : public ParamMap2UserDlgProc {
	EPoly *mpEPoly;
public:
	AdvancedDisplacementDlgProc () : mpEPoly(NULL) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void SetEnables (HWND hDisp);
	void DeleteThis() { }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static AdvancedDisplacementDlgProc theAdvancedDisplacementProc;

void AdvancedDisplacementDlgProc::SetEnables (HWND hDisp) {
	ISpinnerControl* spin = NULL;
	int style=0;
	if (mpEPoly && mpEPoly->getParamBlock())
		mpEPoly->getParamBlock()->GetValue (ep_asd_style, TimeValue(0), style, FOREVER);
	spin = GetISpinner (GetDlgItem (hDisp, IDC_ASD_MIN_ITERS_SPIN));
	spin->Enable (style<2);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_ASD_MIN_ITERS_LABEL), style<2);
	spin = GetISpinner (GetDlgItem (hDisp, IDC_ASD_MAX_ITERS_SPIN));
	spin->Enable (style<2);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_ASD_MAX_ITERS_LABEL), style<2);
	spin = GetISpinner (GetDlgItem (hDisp, IDC_ASD_MAX_TRIS_SPIN));
	spin->Enable (style==2);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_ASD_MAX_TRIS_LABEL), style==2);
}

INT_PTR AdvancedDisplacementDlgProc::DlgProc (TimeValue t, IParamMap2 *map,
						HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		thePopupPosition.Set (hWnd);

		SetEnables (hWnd);
		if (mpEPoly && mpEPoly->getParamBlock()) {
			int min, max;
			mpEPoly->getParamBlock()->GetValue (ep_asd_min_iters, t, min, FOREVER);
			mpEPoly->getParamBlock()->GetValue (ep_asd_max_iters, t, max, FOREVER);
			map->SetRange (ep_asd_min_iters, 0, max);
			map->SetRange (ep_asd_max_iters, min, MAX_SUBDIV);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ASD_GRID:
		case IDC_ASD_TREE:
		case IDC_ASD_DELAUNAY:
			SetEnables (hWnd);
			break;
		case IDCANCEL:
		case IDOK:
			thePopupPosition.Scan (hWnd);
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch ( LOWORD(wParam) ) {
		case IDC_ASD_MIN_ITERS_SPIN:
			map->SetRange (ep_asd_max_iters, spin->GetIVal(), MAX_SUBDIV);
			break;
		case IDC_ASD_MAX_ITERS_SPIN:
			map->SetRange (ep_asd_min_iters, 0, spin->GetIVal());
			break;
		}
	}
	return false;
}

class DisplacementDlgProc : public ParamMap2UserDlgProc {
	bool uiValid;
	EPoly *mpEPoly;

public:
	DisplacementDlgProc () : mpEPoly(NULL), uiValid(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void SetEnables (HWND hDisp);
	void DeleteThis() { }
	void InvalidateUI (HWND hWnd) { uiValid = false; if (hWnd) InvalidateRect (hWnd, NULL, false); }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static DisplacementDlgProc theDisplacementProc;

void DisplacementDlgProc::SetEnables (HWND hDisp) {
	BOOL useSD=false;
	int method=0;
	if (mpEPoly && mpEPoly->getParamBlock ()) {
		mpEPoly->getParamBlock()->GetValue (ep_sd_use, TimeValue(0), useSD, FOREVER);
		mpEPoly->getParamBlock()->GetValue (ep_sd_method, TimeValue(0), method, FOREVER);
	}

	// Enable buttons...
	EnableWindow (GetDlgItem (hDisp, IDC_SD_PRESET1), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_PRESET2), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_PRESET3), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_ADVANCED), useSD);

	// Enable grouping boxes:
	EnableWindow (GetDlgItem (hDisp, IDC_SD_PRESET_BOX), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_METHOD_BOX), useSD);

	// Enable radios, checkboxes:
	EnableWindow (GetDlgItem (hDisp, IDC_SD_SPLITMESH), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_TESS_REGULAR), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_TESS_SPATIAL), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_TESS_CURV), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_TESS_LDA), useSD);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_VIEW_DEP), useSD);

	// Enable spinners and their labels based both on useSD and method.
	bool useSteps = useSD && (method==0);
	ISpinnerControl* spin = NULL;
	spin = GetISpinner (GetDlgItem (hDisp, IDC_SD_TESS_U_SPINNER));
	spin->Enable (useSteps);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_STEPS_LABEL), useSteps);

	bool useEdge = useSD && ((method==1) || (method==3));
	spin = GetISpinner (GetDlgItem (hDisp, IDC_SD_TESS_EDGE_SPINNER));
	spin->Enable (useEdge);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_EDGE_LABEL), useEdge);

	bool useDist = useSD && (method>1);
	spin = GetISpinner (GetDlgItem (hDisp, IDC_SD_TESS_DIST_SPINNER));
	spin->Enable (useDist);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_DISTANCE_LABEL), useDist);

	spin = GetISpinner (GetDlgItem (hDisp, IDC_SD_TESS_ANG_SPINNER));
	spin->Enable (useDist);
	ReleaseISpinner (spin);
	EnableWindow (GetDlgItem (hDisp, IDC_SD_ANGLE_LABEL), useDist);

	uiValid = true;
}

class DispParamChangeRestore : public RestoreObj {
	IParamMap2 *map;
public:
	DispParamChangeRestore (IParamMap2 *m) : map(m) { }
	void Restore (int isUndo) { theDisplacementProc.InvalidateUI (map->GetHWnd()); }
	void Redo () { theDisplacementProc.InvalidateUI (map->GetHWnd()); }
};

INT_PTR DisplacementDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	BOOL ret;
	switch (msg) {
	case WM_INITDIALOG:
		uiValid = false;
		break;

	case WM_PAINT:
		if (uiValid) break;
		SetEnables (hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SD_ENGAGE:
			uiValid = false;
			break;
		case IDC_SD_PRESET1:
			theHold.Begin();  // (uses restore object in paramblock.)
			mpEPoly->UseDisplacementPreset (0);
			theHold.Put (new DispParamChangeRestore (pmap));
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			uiValid = false;
			break;
		case IDC_SD_PRESET2:
			theHold.Begin();  // (uses restore object in paramblock.)
			mpEPoly->UseDisplacementPreset (1);
			theHold.Put (new DispParamChangeRestore (pmap));
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			uiValid = false;
			break;
		case IDC_SD_PRESET3:
			theHold.Begin();  // (uses restore object in paramblock.)
			mpEPoly->UseDisplacementPreset (2);
			theHold.Put (new DispParamChangeRestore (pmap));
			theHold.Accept(GetString(IDS_DISP_APPROX_CHANGE));
			uiValid = false;
			break;
		case IDC_SD_ADVANCED:
			// Make sure we're up to date...
			mpEPoly->SetDisplacementParams ();
			theAdvancedDisplacementProc.SetEPoly(mpEPoly);
			ret = CreateModalParamMap2 (ep_advanced_displacement, mpEPoly->getParamBlock(),
				t, hInstance, MAKEINTRESOURCE (IDD_EP_DISP_APPROX_ADV),
				hWnd, &theAdvancedDisplacementProc);
			if (ret) {
				mpEPoly->SetDisplacementParams ();
			} else {
				// Otherwise, revert back to values in polyobject.
				mpEPoly->UpdateDisplacementParams ();
			}
			break;

		default:
			InvalidateUI (hWnd);
			break;
		}
	}
	return FALSE;
}

class PaintDeformDlgProc : public ParamMap2UserDlgProc {
	bool uiValid;
	EPoly *mpEPoly;

public:
	PaintDeformDlgProc () : mpEPoly(NULL), uiValid(false) { }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void SetEnables (HWND hWnd);
	void DeleteThis() { }
	void InvalidateUI (HWND hWnd) { uiValid = false; if (hWnd) InvalidateRect (hWnd, NULL, false); }
	void SetEPoly (EPoly *e) { mpEPoly=e; }
};

static PaintDeformDlgProc thePaintDeformDlgProc;

void PaintDeformDlgProc::SetEnables (HWND hWnd) {
	EditPolyObject *pEditPoly = (EditPolyObject*)mpEPoly;
	PaintModeID paintMode = pEditPoly->GetPaintMode();

	BOOL isPaintPushPull =	(paintMode==PAINTMODE_PAINTPUSHPULL? TRUE:FALSE);
	BOOL isPaintRelax =		(paintMode==PAINTMODE_PAINTRELAX? TRUE:FALSE);
	BOOL isErasing  =		(paintMode==PAINTMODE_ERASEDEFORM? TRUE:FALSE);
	BOOL isActive =			(pEditPoly->IsPaintDataActive(PAINTMODE_DEFORM)? TRUE:FALSE);

	ICustButton* but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_PUSHPULL));
	but->SetCheck( isPaintPushPull );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_RELAX));
	but->SetCheck( isPaintRelax );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_REVERT));
	but->Enable( isActive );
	but->SetCheck( isErasing );
	ReleaseICustButton (but);

	ISpinnerControl *spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTDEFORM_VALUE_SPIN));
	spin->Enable ((!isPaintRelax) && (!isErasing));
	ReleaseISpinner (spin);

	BOOL enablePushPull = ((!isPaintRelax) && (!isErasing)? TRUE:FALSE);
	HWND hDlgItem = GetDlgItem (hWnd, IDC_PAINTDEFORM_VALUE_LABEL);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_BORDER);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_ORIGNORMS);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_DEFORMEDNORMS);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_XFORM);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_XFORM_X);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_XFORM_Y);
	EnableWindow( hDlgItem, enablePushPull );
	hDlgItem = GetDlgItem (hWnd, IDC_PAINTAXIS_XFORM_Z);
	EnableWindow( hDlgItem, enablePushPull );

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_APPLY));
	but->Enable( isActive );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_CANCEL));
	but->Enable( isActive );
	ReleaseICustButton (but);
}

INT_PTR PaintDeformDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam)
{
	//BOOL ret;
	EditPolyObject *pEditPoly = (EditPolyObject*)mpEPoly;

	switch (msg) {
	case WM_INITDIALOG: {
			uiValid = false;
			ICustButton *btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTDEFORM_PUSHPULL));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);

			btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTDEFORM_RELAX));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);

			btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTDEFORM_REVERT));
			btn->SetType(CBT_CHECK);
			ReleaseICustButton(btn);
			break;
		}

	case WM_PAINT:
		if (uiValid) break;
		SetEnables (hWnd);
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_PAINTDEFORM_VALUE_SPIN:
		case IDC_PAINTDEFORM_SIZE_SPIN:
		case IDC_PAINTDEFORM_STRENGTH_SPIN:
			pEditPoly->UpdatePaintBrush();
			break;
		}
		break;

	case WM_COMMAND:
		//uiValid = false;
		switch (LOWORD(wParam)) {
		case IDC_PAINTDEFORM_PUSHPULL:
			pEditPoly->OnPaintBtn( PAINTMODE_PAINTPUSHPULL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_RELAX:
			pEditPoly->OnPaintBtn( PAINTMODE_PAINTRELAX, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_REVERT:
			pEditPoly->OnPaintBtn( PAINTMODE_ERASEDEFORM, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_APPLY:
			pEditPoly->EpfnPaintDeformCommit();
			break;
		case IDC_PAINTDEFORM_CANCEL:
			pEditPoly->EpfnPaintDeformCancel();
			break;
		case IDC_PAINTDEFORM_OPTIONS:
			GetMeshPaintMgr()->BringUpOptions();
			break;

		default:
			InvalidateUI (hWnd);
			break;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------
// EditPolyObject UI-related methods:

// Luna task 748Y
// Geometry subobject dialogs:
int geomIDs[] = { 0, ep_geom_vertex, ep_geom_edge, ep_geom_border, ep_geom_face, ep_geom_element };
int geomDlgs[] = { 0, IDD_EP_GEOM_VERTEX, IDD_EP_GEOM_EDGE, IDD_EP_GEOM_BORDER,
	IDD_EP_GEOM_FACE, IDD_EP_GEOM_ELEMENT };
int geomDlgNames[] = { 0, IDS_EDIT_VERTICES, IDS_EDIT_EDGES, IDS_EDIT_BORDERS,
	IDS_EDIT_FACES, IDS_EDIT_ELEMENTS };

// Luna task 748Y: UI Redesign
// Surface dialogs - vertex colors, smoothing groups, etc.
int surfIDs[] = {0, ep_vertex, 0, ep_face};
int surfDlgs[] = { 0, IDD_EP_SURF_VERT, 0, IDD_EP_SURF_FACE };
int surfDlgNames[] = { 0, IDS_VERTEX_PROPERTIES, 0, IDS_EP_POLY_MATERIALIDS };
ParamMap2UserDlgProc *surfProcs[] = { NULL, &theVertexDlgProc, NULL, &theFaceDlgProc };

// Luna task 748Y - UI redesign.
// Update all the UI controls to reflect the current selection level:
void EditPolyObject::UpdateUIToSelectionLevel (int oldSelLevel) {
	if (selLevel == oldSelLevel) return;

	HWND hWnd = GetDlgHandle (ep_select);

	if (hWnd != NULL) {
		theSelectDlgProc.RefreshSelType (hWnd);
		theSelectDlgProc.SetEnables (hWnd);
	}

	hWnd = GetDlgHandle (ep_softsel);
	if (hWnd != NULL) {
		//end paint session before invalidating UI
		if ( IsPaintSelMode(GetPaintMode()) ) EndPaintMode();
		theSoftselDlgProc.SetEnables (hWnd);
	}

	hWnd = GetDlgHandle (ep_geom);
	if (hWnd != NULL) {
		theGeomDlgProc.SetEnables (hWnd);
	}

	hWnd = GetDlgHandle (ep_paintdeform);
	if (hWnd != NULL) {
		//end paint session before invalidating UI
		if ( IsPaintDeformMode(GetPaintMode()) ) EndPaintMode();
		thePaintDeformDlgProc.SetEnables (hWnd);
	}

	int msl = meshSelLevel[selLevel];

	// Otherwise, we need to get rid of all remaining rollups and replace them.
	HWND hFocus = GetFocus ();

	// Destroy old versions:
	if (pSubobjControls) {
		rsSubobjControls = IsRollupPanelOpen (pSubobjControls->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pSubobjControls);
		pSubobjControls = NULL;
	}
	if (pSurface) {
		rsSurface = IsRollupPanelOpen (pSurface->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pSurface);
		pSurface = NULL;
	}
	if (pPolySmooth) {
		rsPolySmooth = IsRollupPanelOpen (pPolySmooth->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pPolySmooth);
		pPolySmooth = NULL;
	}
	if (pPolyColor) {
		rsPolyColor = IsRollupPanelOpen (pPolyColor->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pPolyColor);
		pPolyColor = NULL;
	}
	if (pGeom) {
		rsGeom = IsRollupPanelOpen (pGeom->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pGeom);
		pGeom = NULL;
	}
	if (pSubdivision) {
		rsSubdivision = IsRollupPanelOpen (pSubdivision->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pSubdivision);
		pSubdivision = NULL;
	}
	if (pDisplacement) {
		rsDisplacement = IsRollupPanelOpen (pDisplacement->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pDisplacement);
		pDisplacement = NULL;
	}
	if (pPaintDeform) {
		rsPaintDeform = IsRollupPanelOpen (pPaintDeform->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pPaintDeform);
		pPaintDeform = NULL;
	}

	// Create new versions:
	if (msl == MNM_SL_OBJECT) pSubobjControls = NULL;
	else {
		pSubobjControls = CreateCPParamMap2 (geomIDs[selLevel], pblock, ip, hInstance,
			MAKEINTRESOURCE (geomDlgs[selLevel]), GetString (geomDlgNames[selLevel]),
			rsSubobjControls ? APPENDROLL_CLOSED : 0, &theSubobjControlDlgProc);
	}
	pGeom = CreateCPParamMap2 (ep_geom, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_GEOM), GetString (IDS_EDIT_GEOM),
		rsGeom ? APPENDROLL_CLOSED : 0, &theGeomDlgProc);
	if (surfProcs[msl] == NULL) {pSurface = NULL;pPolySmooth=NULL;pPolyColor =NULL;}
	else {
		pSurface = CreateCPParamMap2 (surfIDs[msl], pblock, ip, hInstance,
			MAKEINTRESOURCE (surfDlgs[msl]), GetString (surfDlgNames[msl]),
			rsSurface ? APPENDROLL_CLOSED : 0, surfProcs[msl]);
		mtlref->hwnd = pSurface->GetHWnd();          
		noderef->hwnd = pSurface->GetHWnd(); 
		
		if(surfIDs[msl]==ep_face)
		{
			pPolySmooth = CreateCPParamMap2 (ep_face_smoothing, pblock, ip, hInstance,
				MAKEINTRESOURCE (IDD_EP_POLY_SMOOTHING), GetString (IDS_EP_POLY_SMOOTHING),
				rsPolySmooth ? APPENDROLL_CLOSED : 0, &thePolySmoothDlgProc);
			pPolyColor = CreateCPParamMap2 (ep_face_color, pblock, ip, hInstance,
				MAKEINTRESOURCE (IDD_EP_POLY_COLOR), GetString (IDS_EP_POLY_COLOR),
				rsPolyColor ? APPENDROLL_CLOSED : 0, &thePolyColorDlgProc);
		}
		else
		{
			pPolySmooth = NULL; pPolyColor = NULL;
		}
	}
	pSubdivision = CreateCPParamMap2 (ep_subdivision, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_MESHSMOOTH), GetString (IDS_SUBDIVISION_SURFACE),
		rsSurface ? APPENDROLL_CLOSED : 0, &theSubdivisionDlgProc);
	pDisplacement = CreateCPParamMap2 (ep_displacement, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_DISP_APPROX), GetString (IDS_SUBDIVISION_DISPLACEMENT),
		rsDisplacement ? APPENDROLL_CLOSED : 0, &theDisplacementProc);
	pPaintDeform = CreateCPParamMap2 (ep_paintdeform, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_PAINTDEFORM), GetString (IDS_PAINTDEFORM),
		rsPaintDeform ? APPENDROLL_CLOSED : 0, &thePaintDeformDlgProc);

	KillRefmsg kill(killRefmsg);
	SetFocus (hFocus);
}


void EditPolyObject::InvalidateNumberHighlighted (BOOL zeroOut)
{
	HWND hWnd = GetDlgHandle (ep_select);
	if (!hWnd) return;
	
	HWND item = GetDlgItem(hWnd,IDC_NUMHIGHLIGHTED_LABEL);
	InvalidateRect (item, NULL, FALSE);


	TSTR buf;
	if(zeroOut)
		mHighlightedItems.clear();
	int tempSelLevelOverride = mSelLevelOverride;
	if(GetSelectMode()==PS_SUBOBJ_SEL)
	{
		switch(selLevel)
		{

			case EP_SL_VERTEX:
				tempSelLevelOverride = MNM_SL_VERTEX;
					break;

			case EP_SL_EDGE:
			case EP_SL_BORDER:
				tempSelLevelOverride = MNM_SL_EDGE;
				break;

			case EP_SL_FACE:
			case EP_SL_ELEMENT:
				tempSelLevelOverride = MNM_SL_FACE;
				break;

		
		}
	}

	switch(tempSelLevelOverride)
	{
		case MNM_SL_VERTEX:
			if(mHighlightedItems.size()==1)
				buf.printf (GetString(IDS_EDITPOLY_WHICHVERTHIGHLIGHTED),*(mHighlightedItems.begin())+1);
			else buf.printf(GetString(IDS_EDITPOLY_NUMVERTHIGHLIGHTED),mHighlightedItems.size());
			break;
		case MNM_SL_EDGE:
			if(mHighlightedItems.size()==1)
				buf.printf (GetString(IDS_EDITPOLY_WHICHEDGEHIGHLIGHTED),*(mHighlightedItems.begin())+1);
			else buf.printf(GetString(IDS_EDITPOLY_NUMEDGEHIGHLIGHTED),mHighlightedItems.size());
			break;
		case MNM_SL_FACE:
			if(mHighlightedItems.size()==1)
				buf.printf (GetString(IDS_EDITPOLY_WHICHFACEHIGHLIGHTED),*(mHighlightedItems.begin())+1);
			else buf.printf(GetString(IDS_EDITPOLY_NUMFACEHIGHLIGHTED),mHighlightedItems.size());
			break;
		default:
			buf = TSTR("");
			break;
	}

	SetDlgItemText(hWnd,IDC_NUMHIGHLIGHTED_LABEL,buf);
}

void EditPolyObject::InvalidateNumberSelected () {
	HWND hWnd = GetDlgHandle (ep_select);
	if (!hWnd) return;
	InvalidateRect (hWnd, NULL, FALSE);
	theSelectDlgProc.UpdateNumSel();
}

void EditPolyObject::UpdateDisplacementParams () {
	// Make sure our EPoly displacement params are in alignment with the PolyObject data.
	// (This is vital after polyobjects are converted to EPolys.)

	// The first call we make to pblock->SetValue will overwrite all the PolyObject-level params,
	// so we need to cache copies.
	bool useDisp = GetDisplacement ();
	bool dispSplit = GetDisplacementSplit ();
	TessApprox dparams = DispParams ();

	// Make sure we don't set values that are already correct.
	TimeValue t = ip->GetTime();	// (none of these parameters should be animatable, but it's good form to use current time.)
	BOOL currentUseDisp, currentDispSplit;
	pblock->GetValue (ep_sd_use, t, currentUseDisp, FOREVER);
	if ((currentUseDisp && !useDisp) || (!currentUseDisp && useDisp))
		pblock->SetValue (ep_sd_use, t, useDisp);
	pblock->GetValue (ep_sd_split_mesh, t, currentDispSplit, FOREVER);
	if ((currentDispSplit && !dispSplit) || (!currentDispSplit && dispSplit))
		pblock->SetValue (ep_sd_split_mesh, t, dispSplit);

	int method = 0, currentMethod = 0;
	switch (dparams.type) {
	case TESS_REGULAR: method=0; break;
	case TESS_SPATIAL: method=1; break;
	case TESS_CURVE: method=2; break;
	case TESS_LDA: method=3; break;
	}
	pblock->GetValue (ep_sd_method, t, currentMethod, FOREVER);
	if (currentMethod != method) pblock->SetValue (ep_sd_method, t, method);

	int currentU, currentView;
	float currentEdge, currentDist, currentAngle;
	pblock->GetValue (ep_sd_tess_steps, t, currentU, FOREVER);
	pblock->GetValue (ep_sd_tess_edge, t, currentEdge, FOREVER);
	pblock->GetValue (ep_sd_tess_distance, t, currentDist, FOREVER);
	pblock->GetValue (ep_sd_tess_angle, t, currentAngle, FOREVER);
	pblock->GetValue (ep_sd_view_dependent, t, currentView, FOREVER);

	if (currentU != dparams.u) pblock->SetValue (ep_sd_tess_steps, t, dparams.u);
	if (currentEdge != dparams.edge) pblock->SetValue (ep_sd_tess_edge, t, dparams.edge);
	if (currentDist != dparams.dist) pblock->SetValue (ep_sd_tess_distance, t, dparams.dist);
	float dispAngle = dparams.ang*PI/180.0f;
	if (fabsf(currentAngle-dispAngle)>.00001f) pblock->SetValue (ep_sd_tess_angle, t, dispAngle);
	if (currentView != dparams.view) pblock->SetValue (ep_sd_view_dependent, t, dparams.view);

	int subdivStyle = 0, currentStyle = 0;
	switch (dparams.subdiv) {
	case SUBDIV_GRID: subdivStyle = 0; break;
	case SUBDIV_TREE: subdivStyle = 1; break;
	case SUBDIV_DELAUNAY: subdivStyle = 2; break;
	}
	pblock->GetValue (ep_asd_style, t, currentStyle, FOREVER);
	if (subdivStyle != currentStyle) pblock->SetValue (ep_asd_style, t, subdivStyle);

	int currentMinIters, currentMaxIters, currentMaxTris;
	pblock->GetValue (ep_asd_min_iters, t, currentMinIters, FOREVER);
	pblock->GetValue (ep_asd_max_iters, t, currentMaxIters, FOREVER);
	pblock->GetValue (ep_asd_max_tris, t, currentMaxTris, FOREVER);
	if (currentMinIters != dparams.minSub) pblock->SetValue (ep_asd_min_iters, t, dparams.minSub);
	if (currentMaxIters != dparams.maxSub) pblock->SetValue (ep_asd_max_iters, t, dparams.maxSub);
	if (currentMaxTris != dparams.maxTris) pblock->SetValue (ep_asd_max_tris, t, dparams.maxTris);
}

void EditPolyObject::SetDisplacementParams () {
	TimeValue t = ip ? ip->GetTime() : GetCOREInterface()->GetTime();
	SetDisplacementParams (t);
}

void EditPolyObject::SetDisplacementParams (TimeValue t) {
	displacementSettingsValid.SetInfinite ();
	int val;
	pblock->GetValue (ep_sd_use, t, val, displacementSettingsValid);
	SetDisplacement (val?true:false);
	if (!val) return;
	pblock->GetValue (ep_sd_split_mesh, t, val, displacementSettingsValid);
	SetDisplacementSplit (val?true:false);
	pblock->GetValue (ep_sd_method, t, val, displacementSettingsValid);
	switch (val) {
	case 0: DispParams().type=TESS_REGULAR; break;
	case 1: DispParams().type=TESS_SPATIAL; break;
	case 2: DispParams().type=TESS_CURVE; break;
	case 3: DispParams().type=TESS_LDA; break;
	}
	pblock->GetValue (ep_sd_tess_steps, t, DispParams().u, displacementSettingsValid);
	pblock->GetValue (ep_sd_tess_edge, t, DispParams().edge, displacementSettingsValid);
	pblock->GetValue (ep_sd_tess_distance, t, DispParams().dist, displacementSettingsValid);
	pblock->GetValue (ep_sd_tess_angle, t, DispParams().ang, displacementSettingsValid);
	DispParams().ang *= 180.0f/PI;
	pblock->GetValue (ep_sd_view_dependent, t, DispParams().view, displacementSettingsValid);
	int subdivStyle;
	pblock->GetValue (ep_asd_style, t, subdivStyle, displacementSettingsValid);
	switch (subdivStyle) {
	case 0: DispParams().subdiv = SUBDIV_GRID; break;
	case 1: DispParams().subdiv = SUBDIV_TREE; break;
	case 2: DispParams().subdiv = SUBDIV_DELAUNAY; break;
	}
	pblock->GetValue (ep_asd_min_iters, t, DispParams().minSub, displacementSettingsValid);
	pblock->GetValue (ep_asd_max_iters, t, DispParams().maxSub, displacementSettingsValid);
	pblock->GetValue (ep_asd_max_tris, t, DispParams().maxTris, displacementSettingsValid);
}

void EditPolyObject::UseDisplacementPreset (int presetNumber) {
	// Make sure PolyObject settings are up to date:
	SetDisplacementParams ();
	// Use PolyObject method to change PolyObject settings:
	SetDisplacementApproxToPreset (presetNumber);
	// Copy new PolyObject settings to parameter block:
	UpdateDisplacementParams ();
}

void EditPolyObject::InvalidateSurfaceUI() {
	if (pSubobjControls) {
		theSubobjControlDlgProc.Invalidate ();
		HWND hWnd = pSubobjControls->GetHWnd ();
		if (hWnd) InvalidateRect (hWnd, NULL, false);
	}
	if (!pSurface) return;
	switch (selLevel) {
	case EP_SL_OBJECT:
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		return;	// Nothing to invalidate.
	case EP_SL_VERTEX:
		theVertexDlgProc.Invalidate();
		break;
	case EP_SL_FACE:
	case EP_SL_ELEMENT:
		theFaceDlgProc.Invalidate();
		thePolySmoothDlgProc.Invalidate();
		thePolyColorDlgProc.Invalidate();
		break;
	}
	HWND hWnd = pSurface->GetHWnd();
	if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
	if(pPolySmooth)
	{
		hWnd = pPolySmooth->GetHWnd();
		if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
	}
	if(pPolyColor)
	{
		hWnd = pPolyColor->GetHWnd();
		if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
	}
}

void EditPolyObject::InvalidateSubdivisionUI () {
	if (!pSubdivision) return;
	HWND hWnd = pSubdivision->GetHWnd();
	theSubdivisionDlgProc.InvalidateUI (hWnd);
}

// --- Begin/End Edit Params ---------------------------------

static bool oldShowEnd;

void EditPolyObject::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;
	editObj = this;

	theSelectDlgProc.SetEPoly(this);
	theSoftselDlgProc.SetEPoly (this);
	theGeomDlgProc.SetEPoly (this);
	theSubobjControlDlgProc.SetEPoly (this);
	theSubdivisionDlgProc.SetEPoly (this);
	theDisplacementProc.SetEPoly (this);
	theVertexDlgProc.SetEPoly (this);
	theFaceDlgProc.SetEPoly (this);
	thePolySmoothDlgProc.SetEPoly(this);
	thePolyColorDlgProc.SetEPoly(this);
	thePaintDeformDlgProc.SetEPoly (this);

	int msl = meshSelLevel[selLevel];

	pSelect = CreateCPParamMap2 (ep_select, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_SELECT), GetString (IDS_SELECT),
		rsSelect ? APPENDROLL_CLOSED : 0, &theSelectDlgProc);
	pSoftsel = CreateCPParamMap2 (ep_softsel, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_SOFTSEL), GetString (IDS_SOFTSEL),
		rsSoftsel ? APPENDROLL_CLOSED : 0, &theSoftselDlgProc);
	if (selLevel == EP_SL_OBJECT) pSubobjControls = NULL;
	else {
		pSubobjControls = CreateCPParamMap2 (geomIDs[selLevel], pblock, ip, hInstance,
			MAKEINTRESOURCE (geomDlgs[selLevel]), GetString (geomDlgNames[selLevel]),
			rsSubobjControls ? APPENDROLL_CLOSED : 0, &theSubobjControlDlgProc);
	}
	pGeom = CreateCPParamMap2 (ep_geom, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_GEOM), GetString (IDS_EDIT_GEOM),
		rsGeom ? APPENDROLL_CLOSED : 0, &theGeomDlgProc);
	if (surfProcs[msl] == NULL){ pSurface = NULL;pPolySmooth =NULL;pPolyColor = NULL;    }
	else
	{
		pSurface = CreateCPParamMap2 (surfIDs[msl], pblock, ip, hInstance,
				MAKEINTRESOURCE (surfDlgs[msl]), GetString (surfDlgNames[msl]),
		rsSurface ? APPENDROLL_CLOSED : 0, surfProcs[msl]);

		if(surfIDs[msl]==ep_face)
		{
			pPolySmooth = CreateCPParamMap2 (ep_face_smoothing, pblock, ip, hInstance,
				MAKEINTRESOURCE (IDD_EP_POLY_SMOOTHING), GetString (IDS_EP_POLY_SMOOTHING),
				rsPolySmooth ? APPENDROLL_CLOSED : 0, &thePolySmoothDlgProc);
			pPolyColor = CreateCPParamMap2 (ep_face_color, pblock, ip, hInstance,
				MAKEINTRESOURCE (IDD_EP_POLY_COLOR), GetString (IDS_EP_POLY_COLOR),
				rsPolyColor ? APPENDROLL_CLOSED : 0, &thePolyColorDlgProc);
		}
		else
		{
			pPolySmooth = NULL; pPolyColor = NULL;
		}

	}
	pSubdivision = CreateCPParamMap2 (ep_subdivision, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_MESHSMOOTH), GetString (IDS_SUBDIVISION_SURFACE),
		rsSurface ? APPENDROLL_CLOSED : 0, &theSubdivisionDlgProc);
	pDisplacement = CreateCPParamMap2 (ep_displacement, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_DISP_APPROX), GetString (IDS_SUBDIVISION_DISPLACEMENT),
		rsDisplacement ? APPENDROLL_CLOSED : 0, &theDisplacementProc);
	pPaintDeform = CreateCPParamMap2 (ep_paintdeform, pblock, ip, hInstance,
		MAKEINTRESOURCE (IDD_EP_PAINTDEFORM), GetString (IDS_PAINTDEFORM),
		rsPaintDeform ? APPENDROLL_CLOSED : 0, &thePaintDeformDlgProc);

	InvalidateNumberSelected ();
	{
		KillRefmsg kill(killRefmsg);
	UpdateDisplacementParams ();
	}

	// Create sub object editing modes.
	moveMode       = new MoveModBoxCMode(this,ip);
	rotMode        = new RotateModBoxCMode(this,ip);
	uscaleMode     = new UScaleModBoxCMode(this,ip);
	nuscaleMode    = new NUScaleModBoxCMode(this,ip);
	squashMode     = new SquashModBoxCMode(this,ip);
	selectMode     = new SelectModBoxCMode(this,ip);

	// Add our command modes:
	createVertMode = new CreateVertCMode (this, ip);
	createEdgeMode = new CreateEdgeCMode (this, ip);
	createFaceMode = new CreateFaceCMode(this, ip);
	divideEdgeMode = new DivideEdgeCMode(this, ip);
	divideFaceMode = new DivideFaceCMode (this, ip);
	extrudeMode = new ExtrudeCMode (this, ip);
	chamferMode = new ChamferCMode (this, ip);
	bevelMode = new BevelCMode(this, ip);
	insetMode = new InsetCMode (this, ip);	// Luna task 748R
	outlineMode = new OutlineCMode (this, ip);
	extrudeVEMode = new ExtrudeVECMode (this, ip);	// Luna task 748F/G
	cutMode = new CutCMode (this, ip);	// Luna task 748D
	quickSliceMode = new QuickSliceCMode (this, ip);	// Luna task 748J
	weldMode = new WeldCMode (this, ip);
	liftFromEdgeMode = new LiftFromEdgeCMode (this, ip);	// Luna task 748P 
	pickLiftEdgeMode = new PickLiftEdgeCMode (this, ip);	// Luna task 748P 
	editTriMode = new EditTriCMode (this, ip);
	turnEdgeMode = new TurnEdgeCMode (this, ip);
	bridgeBorderMode = new BridgeBorderCMode (this, ip);
	bridgeEdgeMode = new BridgeEdgeCMode (this, ip);
	bridgePolyMode = new BridgePolygonCMode (this, ip);
	pickBridgeTarget1 = new PickBridge1CMode (this, ip);
	pickBridgeTarget2 = new PickBridge2CMode (this, ip);
	
	// Create reference for MultiMtl name support         
	noderef = new SingleRefMakerPolyNode;         //set ref to node
	INode* objNode = GetNode(this);
	if (objNode) {
		noderef->ep = this;
		noderef->SetRef(objNode);                 
	}

	mtlref = new SingleRefMakerPolyMtl;       //set ref for mtl
	mtlref->ep = this;
	if (objNode) {
		Mtl* nodeMtl = objNode->GetMtl();
		mtlref->SetRef(nodeMtl);                        
	}                 

	// And our pick modes:
	if (!attachPickMode) attachPickMode = new AttachPickMode;
	if (attachPickMode) attachPickMode->SetPolyObject (this, ip);
	// Luna task 748T
	if (!shapePickMode) shapePickMode = new ShapePickMode;
	if (shapePickMode) shapePickMode->SetPolyObject (this, ip);

	// Add our accelerator table (keyboard shortcuts)
	mpEPolyActions = new EPolyActionCB (this);
	ip->GetActionManager()->ActivateActionTable (mpEPolyActions, kEPolyActionID);

	//register all of the overrides with this modifier, first though unregister any still around for wthatever reason,though EndEditParams should handlethis
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
		mOverrides.ZeroCount();
	
		ActionTable *pTable = mpEPolyActions->GetTable();
		SubObjectModeOverride *subObjectModeOverride = new SubObjectModeOverride(this,EP_SL_OBJECT);
		ActionItem *item = pTable->GetAction(ID_EP_SEL_LEV_OBJ);
		mOverrides.Append(1,&item);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager
	/*  Kelcey said don't do these guys only the object.. leave in case she changes her mind..
		item = pTable->GetAction(ID_EP_SEL_LEV_VERTEX);
		mOverrides.Append(1,&item);
		subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_VERTEX);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager

		item = pTable->GetAction(ID_EP_SEL_LEV_EDGE);
		mOverrides.Append(1,&item);
		subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_EDGE);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager

		item = pTable->GetAction(ID_EP_SEL_LEV_BORDER);
		mOverrides.Append(1,&item);
		subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_BORDER);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager

		item = pTable->GetAction(ID_EP_SEL_LEV_FACE);
		mOverrides.Append(1,&item);
		subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_FACE);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager

		item = pTable->GetAction(ID_EP_SEL_LEV_ELEMENT);
		mOverrides.Append(1,&item);
		subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_ELEMENT);
		actionOverrideMan->ActivateActionItemOverride(item,subObjectModeOverride); //deletion of override handled by override manager
*/
		//command overrides	
		item = pTable->GetAction(ID_EP_EDIT_CUT);
		mOverrides.Append(1,&item);
		CutModeOverride *cutModeOverride = new CutModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,cutModeOverride);

		item = pTable->GetAction(ID_EP_EDIT_QUICKSLICE);
		mOverrides.Append(1,&item);
		CommandModeOverride *commandModeOverride = new CommandModeOverride(this,epmode_quickslice);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);
		
	/*	item = pTable->GetAction(ID_EP_EDIT_SLICEPLANE);
		mOverrides.Append(1,&item);
		commandModeOverride = new CommandModeOverride(this,ep_mode_sliceplane);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);
*/
		//face command mdoes
		item = pTable->GetAction(ID_EP_SUB_BEVEL);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,epmode_bevel);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_OUTLINE);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,epmode_outline);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_INSET);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,epmode_inset_face);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_LIFT);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,epmode_lift_from_edge);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//less than edge 
		item = pTable->GetAction(ID_EP_SUB_WELD);
		mOverrides.Append(1,&item);
		commandModeOverride = new LessEdgeCommandModeOverride(this,epmode_weld);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//greater edge
		item = pTable->GetAction(ID_EP_TURN);
		mOverrides.Append(1,&item);
		commandModeOverride = new GreaterEdgeCommandModeOverride(this,epmode_turn_edge);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		
		item = pTable->GetAction(ID_EP_SURF_EDITTRI);
		mOverrides.Append(1,&item);
		commandModeOverride = new GreaterEdgeCommandModeOverride(this,epmode_edit_tri);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//chamfer
		item = pTable->GetAction(ID_EP_SUB_CHAMFER);
		mOverrides.Append(1,&item);
		commandModeOverride = new ChamferModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//create
		item = pTable->GetAction(ID_EP_EDIT_CREATE);
		mOverrides.Append(1,&item);
		commandModeOverride = new CreateModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//insert
		item = pTable->GetAction(ID_EP_SUB_INSERTVERT);
		mOverrides.Append(1,&item);
		commandModeOverride = new InsertModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//bridge
		item = pTable->GetAction(ID_EP_SUB_BRIDGE);
		mOverrides.Append(1,&item);
		commandModeOverride = new BridgeModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//extrude
		item = pTable->GetAction(ID_EP_SUB_EXTRUDE);
		mOverrides.Append(1,&item);
		commandModeOverride = new ExtrudeModeOverride(this);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//normal constraint
		item = pTable->GetAction(ID_EP_NORMAL_CONSTRAINT);
		mOverrides.Append(1,&item);
		ConstraintOverride * constraintOverride = new ConstraintOverride(this,3);
		actionOverrideMan->ActivateActionItemOverride(item,constraintOverride);

		//edge constraint
		item = pTable->GetAction(ID_EP_EDGE_CONSTRAINT);
		mOverrides.Append(1,&item);
		constraintOverride = new ConstraintOverride(this,1);
		actionOverrideMan->ActivateActionItemOverride(item,constraintOverride);
		
		//face constraint
		item = pTable->GetAction(ID_EP_FACE_CONSTRAINT);
		mOverrides.Append(1,&item);
		constraintOverride = new ConstraintOverride(this,2);
		actionOverrideMan->ActivateActionItemOverride(item,constraintOverride);
		
		//paint
		item = pTable->GetAction(ID_EP_SS_PAINT);
		mOverrides.Append(1,&item);
		PaintOverride *paintOverride = new PaintOverride(this,PAINTMODE_PAINTSEL);
		actionOverrideMan->ActivateActionItemOverride(item,paintOverride);
	
		//paint pushpull
		item = pTable->GetAction(ID_EP_SS_PAINTPUSHPULL);
		mOverrides.Append(1,&item);
		paintOverride = new PaintOverride(this,PAINTMODE_PAINTPUSHPULL);
		actionOverrideMan->ActivateActionItemOverride(item,paintOverride);

		//paint relax
		item = pTable->GetAction(ID_EP_SS_PAINTRELAX);
		mOverrides.Append(1,&item);
		paintOverride = new PaintOverride(this,PAINTMODE_PAINTRELAX);
		actionOverrideMan->ActivateActionItemOverride(item,paintOverride);

}





	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult (GetFlag (EPOLY_DISP_RESULT));

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	// We want del key, backspace input if in geometry level
	if (selLevel != EP_SL_OBJECT) {
		ip->RegisterDeleteUser(this);
		backspacer.SetEPoly(this);
		backspaceRouter.Register(&backspacer);
	}

	InvalidateNumberHighlighted (TRUE);

	MeshPaintHandler::BeginEditParams();	

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);		
	SetAFlag(A_OBJ_BEING_EDITED);	


}

void EditPolyObject::EndEditParams (IObjParam *ip, ULONG flags,Animatable *next) {

	MeshPaintHandler::EndEditParams();	

	// Unregister del key, backspace notification
	if (selLevel != EP_SL_OBJECT) {
		ip->UnRegisterDeleteUser(this);
		backspacer.SetEPoly (NULL);
		backspaceRouter.UnRegister (&backspacer);
	}

	//unregister all of the overrides.  Need to do this before deactiviting the keyboard shortcuts.
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
	}
	mOverrides.ZeroCount();


	// Deactivate our keyboard shortcuts
	if (mpEPolyActions) {
		ip->GetActionManager()->DeactivateActionTable (mpEPolyActions, kEPolyActionID);
		delete mpEPolyActions;
		mpEPolyActions = NULL;
	}

	ExitAllCommandModes ();
	if (moveMode) delete moveMode;
	moveMode = NULL;
	if (rotMode) delete rotMode;
	rotMode = NULL;
	if (uscaleMode) delete uscaleMode;
	uscaleMode = NULL;
	if (nuscaleMode) delete nuscaleMode;
	nuscaleMode = NULL;
	if (squashMode) delete squashMode;
	squashMode = NULL;
	if (selectMode) delete selectMode;
	selectMode = NULL;

	if (createVertMode) delete createVertMode;
	createVertMode = NULL;
	if (createEdgeMode) delete createEdgeMode;
	createEdgeMode = NULL;
	if (createFaceMode) delete createFaceMode;
	createFaceMode = NULL;
	if (divideEdgeMode) delete divideEdgeMode;
	divideEdgeMode = NULL;
	if (divideFaceMode) delete divideFaceMode;
	divideFaceMode = NULL;
	if (extrudeMode) delete extrudeMode;
	extrudeMode = NULL;
	if (chamferMode) delete chamferMode;
	chamferMode = NULL;
	if (bevelMode) delete bevelMode;
	bevelMode = NULL;
	if (insetMode) delete insetMode;	// Luna task 748R
	insetMode = NULL;
	if (outlineMode) delete outlineMode;
	outlineMode = NULL;
	if (extrudeVEMode) delete extrudeVEMode;
	extrudeVEMode = NULL;
	if (cutMode) delete cutMode;	// Luna task 748D
	cutMode = NULL;
	if (quickSliceMode) delete quickSliceMode;	// Luna task 748J
	quickSliceMode = NULL;
	if (weldMode) delete weldMode;
	weldMode = NULL;
	if (liftFromEdgeMode) delete liftFromEdgeMode;	// Luna task 748P
	liftFromEdgeMode = NULL;
	if (pickLiftEdgeMode) delete pickLiftEdgeMode;	// Luna task 748P
	pickLiftEdgeMode = NULL;
	if (editTriMode) delete editTriMode;
	editTriMode = NULL;
	if (turnEdgeMode) delete turnEdgeMode;
	turnEdgeMode = NULL;
	if (bridgeBorderMode) delete bridgeBorderMode;
	bridgeBorderMode = NULL;
	if (bridgeEdgeMode) delete bridgeEdgeMode;
	bridgeEdgeMode = NULL;
	if (bridgePolyMode) delete bridgePolyMode;
	bridgePolyMode = NULL;
	if (pickBridgeTarget1) delete pickBridgeTarget1;
	pickBridgeTarget1 = NULL;
	if (pickBridgeTarget2) delete pickBridgeTarget2;
	pickBridgeTarget2 = NULL;

	if (attachPickMode) attachPickMode->ClearPolyObject ();
	// Luna task 748T
	if (shapePickMode) shapePickMode->ClearPolyObject ();

	if (tempData) {
		delete tempData;
		tempData = NULL;
	}
	if (tempMove) {
		delete tempMove;
		tempMove = NULL;
	}

	if (pSelect) {
		rsSelect = IsRollupPanelOpen (pSelect->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pSelect);
		pSelect = NULL;
	}
	if (pSoftsel) {
		rsSoftsel = IsRollupPanelOpen (pSoftsel->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pSoftsel);
		pSoftsel = NULL;
	}
	if (pGeom) {
		rsGeom = IsRollupPanelOpen (pGeom->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pGeom);
		pGeom = NULL;
	}
	if (pSubobjControls) {	// Luna task 748Y
		rsSubobjControls = IsRollupPanelOpen (pSubobjControls->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (pSubobjControls);
		pSubobjControls = NULL;
	}
	if (pSurface) {
		rsSurface = IsRollupPanelOpen (pSurface->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pSurface);
		pSurface = NULL;
	}
	if (pPolySmooth) {
		rsPolySmooth = IsRollupPanelOpen (pPolySmooth->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pPolySmooth);
		pPolySmooth = NULL;
	}
	if (pPolyColor) {
		rsPolyColor = IsRollupPanelOpen (pPolyColor->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pPolyColor);
		pPolyColor = NULL;
	}
	if (pSubdivision) {
		rsSubdivision = IsRollupPanelOpen (pSubdivision->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pSubdivision);
		pSubdivision = NULL;
	}
	if (pDisplacement) {
		rsDisplacement = IsRollupPanelOpen (pDisplacement->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pDisplacement);
		pDisplacement = NULL;
	}
	if (pPaintDeform) {
		rsPaintDeform = IsRollupPanelOpen (pPaintDeform->GetHWnd())?FALSE:TRUE;
		DestroyCPParamMap2 (pPaintDeform);
		pPaintDeform = NULL;
	}

	// Luna task 748A - close any popup settings dialog
	if (pOperationSettings) EpfnClosePopupDialog ();

	if (noderef) delete noderef;              
	noderef = NULL;                         
	if (mtlref) delete mtlref;             
	mtlref = NULL;                          

	this->ip = NULL;
	editObj = NULL;

	// Reset show end result
	// NOTE: since this causes a pipeline reevaluation, it needs to come last.
	SetFlag (EPOLY_DISP_RESULT, ip->GetShowEndResult());
	ip->SetShowEndResult(oldShowEnd);

	ClearAFlag(A_OBJ_BEING_EDITED);

}


void EditPolyObject::ExitPickLiftEdgeMode () {
	if (pickLiftEdgeMode) ip->DeleteMode (pickLiftEdgeMode);
}

void EditPolyObject::ExitPickBridgeModes () {
	if (pickBridgeTarget1) ip->DeleteMode (pickBridgeTarget1);
	if (pickBridgeTarget2) ip->DeleteMode (pickBridgeTarget2);
}

void EditPolyObject::ExitAllCommandModes (bool exSlice, bool exStandardModes) {
	if( InPaintMode() ) EndPaintMode();

	if (createVertMode) ip->DeleteMode (createVertMode);
	if (createEdgeMode) ip->DeleteMode (createEdgeMode);
	if (createFaceMode) ip->DeleteMode (createFaceMode);
	if (divideEdgeMode) ip->DeleteMode (divideEdgeMode);
	if (divideFaceMode) ip->DeleteMode (divideFaceMode);
	if (extrudeMode) ip->DeleteMode (extrudeMode);
	if (extrudeVEMode) ip->DeleteMode (extrudeVEMode);
	if (chamferMode) ip->DeleteMode (chamferMode);
	if (bevelMode) ip->DeleteMode (bevelMode);
	if (insetMode) ip->DeleteMode (insetMode);	// Luna task 748R
	if (outlineMode) ip->DeleteMode (outlineMode);
	if (extrudeVEMode) ip->DeleteMode (extrudeVEMode);
	if (cutMode) ip->DeleteMode (cutMode);	// Luna task 748D
	if (quickSliceMode) ip->DeleteMode (quickSliceMode);	// Luna task 748J
	if (weldMode) ip->DeleteMode (weldMode);
	if (liftFromEdgeMode) ip->DeleteMode (liftFromEdgeMode);	// Luna task 748P 
	if (pickLiftEdgeMode) ip->DeleteMode (pickLiftEdgeMode);	// Luna task 748P 
	if (editTriMode) ip->DeleteMode (editTriMode);
	if (turnEdgeMode) ip->DeleteMode (turnEdgeMode);
	if (bridgeBorderMode) ip->DeleteMode (bridgeBorderMode);
	if (bridgeEdgeMode) ip->DeleteMode (bridgeEdgeMode);
	if (bridgePolyMode) ip->DeleteMode (bridgePolyMode);
	if (pickBridgeTarget1) ip->DeleteMode (pickBridgeTarget1);
	if (pickBridgeTarget2) ip->DeleteMode (pickBridgeTarget2);

	if (exStandardModes) {
		// remove our SO versions of standardmodes:
		if (moveMode) ip->DeleteMode (moveMode);
		if (rotMode) ip->DeleteMode (rotMode);
		if (uscaleMode) ip->DeleteMode (uscaleMode);
		if (nuscaleMode) ip->DeleteMode (nuscaleMode);
		if (squashMode) ip->DeleteMode (squashMode);
		if (selectMode) ip->DeleteMode (selectMode);
	}

	if (sliceMode && exSlice) ExitSliceMode ();
	ip->ClearPickMode();
}

// Luna task 748Y - UI redesign
void EditPolyObject::InvalidateDialogElement (int elem) {
	// No convenient way to get parammap pointer from element id,
	// so we invalidate this element in all parammaps.  (Harmless - does
	// nothing if elem is not present in a given parammap.)
	if (pGeom) pGeom->Invalidate (elem);
	if (pSubobjControls) pSubobjControls->Invalidate(elem);
	if (pSelect) pSelect->Invalidate (elem);
	if (pSoftsel) {
		pSoftsel->Invalidate (elem);
		if ((elem == ep_ss_use) || (elem == ep_ss_lock))
			theSoftselDlgProc.SetEnables (pSoftsel->GetHWnd());
		if ((elem == ep_ss_falloff) || (elem == ep_ss_pinch) || (elem == ep_ss_bubble))
		{
			Rect rect;
			GetClientRectP(GetDlgItem(pSoftsel->GetHWnd(),IDC_SS_GRAPH),&rect);
			InvalidateRect(pSoftsel->GetHWnd(),&rect,FALSE);
		}
	}
	if (pSubdivision) pSubdivision->Invalidate (elem);
	if (pDisplacement) {
		pDisplacement->Invalidate (elem);
		if ((elem == ep_sd_use) || (elem == ep_sd_method)) {
			theDisplacementProc.InvalidateUI (pDisplacement->GetHWnd());
		}
	}
	if (pSurface) pSurface->Invalidate (elem);
	if (pPolySmooth) pPolySmooth->Invalidate(elem);
	if (pPolyColor) pPolyColor->Invalidate(elem);
	if (pOperationSettings) {
		pOperationSettings->Invalidate (elem);
		thePopupDlgProc.InvalidateUI (pOperationSettings->GetHWnd(), elem);
	}
	if ((elem == ep_constrain_type) && pGeom) {
		theGeomDlgProc.UpdateConstraint (pGeom->GetHWnd());
	}
	if (pPaintDeform) pPaintDeform->Invalidate (elem);
}

void EditPolyObject::UpdatePerDataDisplay (TimeValue t, int msl, int channel) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl != meshSelLevel[selLevel]) return;
	if (!pSubobjControls) return;
	HWND hWnd = pSubobjControls->GetHWnd();
	theSubobjControlDlgProc.UpdatePerDataDisplay (t, selLevel, channel, hWnd);
}

// Luna task 748Y - UI redesign
HWND EditPolyObject::GetDlgHandle (int dlgID) {
	if (!ip) return NULL;
	if (editObj != this) return NULL;
	if (dlgID<0) return NULL;
	IParamMap2 *pmap=NULL;
	switch (dlgID) {
	case ep_select:
		pmap = pSelect;
		break;
	case ep_softsel:
		pmap = pSoftsel;
		break;
	case ep_geom:
		pmap = pGeom;
		break;
	case ep_geom_vertex:
	case ep_geom_edge:
	case ep_geom_face:
	case ep_geom_border:
	case ep_geom_element:
		pmap = pSubobjControls;
		break;
	case ep_subdivision:
		pmap = pSubdivision;
		break;
	case ep_displacement:
		pmap = pDisplacement;
		break;
	case ep_vertex:
	case ep_face:
		pmap = pSurface; break;
	case ep_settings:
		pmap = pOperationSettings;
		break;
	case ep_face_smoothing:
		pmap = pPolySmooth;
		break;
	case ep_face_color:
		pmap = pPolyColor;
		break;
	case ep_paintdeform:
		pmap = pPaintDeform;
		break;
	}
	if (!pmap) return NULL;
	return pmap->GetHWnd();
}

int EditPolyObject::NumSubObjTypes() { return 5; }

ISubObjType *EditPolyObject::GetSubObjType(int i) {
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		SOT_Vertex.SetName(GetString(IDS_VERTEX));
		SOT_Edge.SetName(GetString(IDS_EDGE));
		SOT_Border.SetName(GetString(IDS_BORDER));
		SOT_Poly.SetName(GetString(IDS_FACE));
		SOT_Element.SetName(GetString(IDS_ELEMENT));
	}

	switch(i) {
	case -1:
		if (selLevel > 0) return GetSubObjType (selLevel-1);
		break;
	case 0: return &SOT_Vertex;
	case 1: return &SOT_Edge;
	case 2: return &SOT_Border;
	case 3: return &SOT_Poly;
	case 4: return &SOT_Element;
	}
	return NULL;
}


//az -  042503  MultiMtl sub/mtl name support
void GetMtlIDList(Mtl *mtl, NumList& mtlIDList)
{
	if (mtl != NULL && mtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0)) {
		int subs = mtl->NumSubMtls();
		if (subs <= 0)
			subs = 1;
		for (int i=0; i<subs;i++){
			if(mtl->GetSubMtl(i))
				mtlIDList.Add(i, TRUE);  

		}
	}
}

void GetEPolyMtlIDList(EditPolyObject *ep, NumList& mtlIDList)
{
	if (ep) {
		int c_numFaces = ep->mm.numf; 
		for (int i=0; i<c_numFaces; i++) {
			int mid = ep->mm.f[i].material; 
			if (mid != -1){
				mtlIDList.Add(mid, TRUE);
			}
		}
	}
}

INode* GetNode (EditPolyObject *ep){
	ModContextList mcList;
	INodeTab nodes;
	ep->ip->GetModContexts (mcList, nodes);
	INode* objnode = nodes.Count() == 1 ? nodes[0]->GetActualINode(): NULL;
	nodes.DisposeTemporary();
	return objnode;
}

BOOL SetupMtlSubNameCombo (HWND hWnd, EditPolyObject *ep) {
	INode* singleNode = NULL;
	Mtl *nodeMtl = NULL;
	
	if(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO)==NULL)
		return false;

	singleNode = GetNode(ep);
	if(singleNode){
		nodeMtl = singleNode->GetMtl();
		// check for scripted material
		if(nodeMtl){
			MSPlugin* plugin = (MSPlugin*)((ReferenceTarget*)nodeMtl)->GetInterface(I_MAXSCRIPTPLUGIN);
			if(plugin)
				nodeMtl = (Mtl*)plugin->get_delegate();
		}
	} 
	if(singleNode == NULL || nodeMtl == NULL || nodeMtl->ClassID() != Class_ID(MULTI_CLASS_ID, 0)) {    //no UI for cloned nodes, and not MultiMtl 
		SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), false);
		return false;
	}
	NumList mtlIDList;
	NumList mtlIDMeshList;
	GetMtlIDList(nodeMtl, mtlIDList);
	GetEPolyMtlIDList(ep, mtlIDMeshList);
	MultiMtl *nodeMulti = (MultiMtl*) nodeMtl;
	EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), true);
	SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);

	for (int i=0; i<mtlIDList.Count(); i++){
		TSTR idname, buf;
		if(mtlIDMeshList.Find(mtlIDList[i]) != -1) {
			nodeMulti->GetSubMtlName(mtlIDList[i], idname); 
			if (idname.isNull())
				idname = GetString(IDS_MTL_NONAME);                                 //az: 042503  - FIGS
			buf.printf(_T("%s - ( %d )"), idname.data(), mtlIDList[i]+1);
			int ith = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_ADDSTRING, 0, (LPARAM)buf.data());
			SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_SETITEMDATA, ith, (LPARAM)mtlIDList[i]);
		}
	}
	return true;
}

void UpdateNameCombo (HWND hWnd, ISpinnerControl *spin) {
	int cbcount, sval, cbval;
	sval = spin->GetIVal() - 1;          
	if (!spin->IsIndeterminate()){
		cbcount = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETCOUNT, 0, 0);
		if (cbcount > 0){
			for (int index=0; index<cbcount; index++){
				cbval = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
				if (sval == cbval) {
					SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)index, 0);
					return;
				}
			}
		}
	}
	SendMessage(GetDlgItem( hWnd, IDC_MTLID_NAMES_COMBO), CB_SETCURSEL, (WPARAM)-1, 0);	
}


void ValidateUINameCombo (HWND hWnd, EditPolyObject *ep) {
	SetupMtlSubNameCombo (hWnd, ep);
	ISpinnerControl* spin = NULL;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	if (spin)
	{
		UpdateNameCombo (hWnd, spin);
		ReleaseISpinner(spin);
	}
}

// From MeshPaintHandler
void EditPolyObject::GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& nodes ) {
	ModContextList _mcList;
	INodeTab _nodes;
	GetCOREInterface()->GetModContexts(_mcList,_nodes);
	DbgAssert( _nodes.Count()==1 );

	hosts.SetCount(1);
	nodes.SetCount(1);
	MeshPaintHost* host = this;
	hosts[0] = host;
	nodes[0] = _nodes[0];
}

float EditPolyObject::GetPaintValue(  PaintModeID paintMode  ) {
	if( (pblock!=NULL) && IsPaintSelMode(paintMode) )		return pblock->GetFloat(ep_paintsel_value);
	if( (pblock!=NULL) && IsPaintDeformMode(paintMode) )	return pblock->GetFloat(ep_paintdeform_value);
	return 0;
}
void EditPolyObject::SetPaintValue(  PaintModeID paintMode, float value  ) {
	if( (pblock!=NULL) && IsPaintSelMode(paintMode) )		pblock->SetValue(ep_paintsel_value, 0, value);
	if( (pblock!=NULL) && IsPaintDeformMode(paintMode) )	pblock->SetValue(ep_paintdeform_value, 0, value);
}

PaintAxisID EditPolyObject::GetPaintAxis( PaintModeID paintMode ) {
	if( (pblock!=NULL) && IsPaintDeformMode(paintMode) )
		return (PaintAxisID)pblock->GetInt(ep_paintdeform_axis);
	return PAINTAXIS_NONE;
}
void EditPolyObject::SetPaintAxis( PaintModeID paintMode, PaintAxisID axis ) {
	if( (pblock!=NULL) && IsPaintDeformMode(paintMode) )
		pblock->SetValue(ep_paintdeform_axis, 0, (int)axis );
}

MNMesh* EditPolyObject::GetPaintObject( MeshPaintHost* host )
	{ return GetMeshPtr(); }

void EditPolyObject::GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues ) {
	GenSoftSelData softSelData;
	GetSoftSelData( softSelData, ip->GetTime() );
	FloatTab* weights = TempData()->VSWeight(
		softSelData.useEdgeDist, softSelData.edgeIts, softSelData.ignoreBack,
		softSelData.falloff, softSelData.pinch, softSelData.bubble );

	if (!weights) {
		// can happen if num vertices == 0.
		selValues.ZeroCount();
		return;
	}
	selValues = *weights;
}

void EditPolyObject::GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos ) {
	MNMesh& mesh = GetMesh();
	int count = mesh.VNum();
	vertPos.SetCount( count );

	Point3* offsets = GetPaintDeformOffsets();
	if( offsets==NULL ) for( int i=0; i<count; i++ ) vertPos[i] = mesh.v[i].p;
	else				for( int i=0; i<count; i++ ) vertPos[i] = mesh.v[i].p - offsets[i];
}

void EditPolyObject::GetPaintMask( MeshPaintHost* host, BitArray& sel, FloatTab& softSel ) {
	MNMesh& mesh = GetMesh();
	sel = mesh.VertexTempSel ();
	float* weights = mesh.getVSelectionWeights ();
	int count = (weights==NULL? 0:mesh.VNum());
	softSel.SetCount(count);
	if( count>0 ) memcpy( softSel.Addr(0), weights, count*sizeof(float) );
}

void EditPolyObject::ApplyPaintSel( MeshPaintHost* host ) {
	arValid = NEVER;
	float* destWeights = GetMesh().getVSelectionWeights();
	float* srcWeights = GetPaintSelValues();
	int count = GetPaintSelCount();
	memcpy( destWeights, srcWeights, count * sizeof(float) );
	GetMesh().InvalidateHardwareMesh();
}

void EditPolyObject::ApplyPaintDeform( MeshPaintHost* host,  BitArray *partialRevert ) {
	localGeomValid = NEVER;
	Point3* offsets = GetPaintDeformOffsets();
	int count = GetPaintDeformCount();
	MNMesh& mesh = GetMesh();
	// Add in the offset values
	Point3 zero(0.0f,0.0f,0.0f);

	if (partialRevert)
	{
		IMNMeshUtilities8 *mnInterface = (IMNMeshUtilities8*) mesh.GetInterface(IMNMESHUTILITIES8_INTERFACE_ID);
		for( int i=0; i<count; i++ ) 
		{
			if ((*partialRevert)[i])
			{
				mnInterface->InvalidateVertexCache(i);
				mesh.v[i].p += offsets[i];
			}
		}
		//have to update the mesh since we're updating points directly - the geometry cache will be need to be updated
		mesh.SetFlag(MN_MESH_PARTIALCACHEINVALID);
	}
	else
	{
		for( int i=0; i<count; i++ ) 
		{
			mesh.v[i].p += offsets[i];
		}
		//have to update the mesh since we're updating points directly - the geometry cache will be need to be updated
		mesh.SetFlag(MN_CACHEINVALID);
	}
	mesh.InvalidateGeomCache();
}

void EditPolyObject::RevertPaintDeform( MeshPaintHost* host,  BitArray *partialRevert ) {
	localGeomValid = NEVER;
	Point3* offsets = GetPaintDeformOffsets();
	int count = GetPaintDeformCount();
	MNMesh& mesh = GetMesh();
	// Subtract out the offset values
	if (partialRevert)
	{
		for( int i=0; i<count; i++ ) 
		{
			if ((*partialRevert)[i])
				mesh.v[i].p -= offsets[i];
		}
		mesh.SetFlag(MN_MESH_PARTIALCACHEINVALID);
	}
	else
	{
		for( int i=0; i<count; i++ ) 
			mesh.v[i].p -= offsets[i];
		mesh.SetFlag(MN_CACHEINVALID);
	}
	//have to update the mesh since we're updating points directly - the geometry cache will be need to be updated
	mesh.InvalidateGeomCache();
}

void EditPolyObject::EpfnPaintDeformCommit() {
	if( InPaintMode() )
		EndPaintMode();
	if( IsPaintDataActive(PAINTMODE_DEFORM) )
		DeactivatePaintData( PAINTMODE_DEFORM, TRUE );
}

void EditPolyObject::EpfnPaintDeformCancel() {
	if( InPaintMode() )
		EndPaintMode();
	if( IsPaintDataActive(PAINTMODE_DEFORM) )
		DeactivatePaintData( PAINTMODE_DEFORM, FALSE );
}


void EditPolyObject::OnPaintDataRedraw( PaintModeID paintMode ) {
	if( IsPaintSelMode(paintMode) )		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if( IsPaintDeformMode(paintMode) )	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	if (ip) ip->RedrawViews( ip->GetTime() );
}

void EditPolyObject::OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode ) {
	// Call SetEnables() on appropriate dialog
	if( IsPaintSelMode(prevMode) || IsPaintSelMode(curMode) ) {
		HWND hWnd = GetDlgHandle (ep_softsel);
		if (hWnd) theSoftselDlgProc.SetEnables (hWnd);
	}
	if( IsPaintDeformMode(prevMode) || IsPaintDeformMode(curMode) ) {
		HWND hWnd = GetDlgHandle (ep_paintdeform);
		if (hWnd) thePaintDeformDlgProc.SetEnables (hWnd);
	}
}

void EditPolyObject::OnPaintDataActivate( PaintModeID paintMode, BOOL onOff ) {
	// Call SetEnables() on appropriate dialog
	if( IsPaintSelMode(paintMode) ) {
		HWND hWnd = GetDlgHandle (ep_softsel);
		if (hWnd) theSoftselDlgProc.SetEnables (hWnd);
	}
	if( IsPaintDeformMode(paintMode) ) {
		HWND hWnd = GetDlgHandle (ep_paintdeform);
		if (hWnd) thePaintDeformDlgProc.SetEnables (hWnd);
	}
}

void EditPolyObject::OnPaintBrushChanged() {
	InvalidateDialogElement(ep_paintsel_size);
	InvalidateDialogElement(ep_paintdeform_size);
	InvalidateDialogElement(ep_paintsel_strength);
	InvalidateDialogElement(ep_paintdeform_strength);
}


// Helper method
static BOOL IsButtonEnabled(HWND hParent, DWORD id) {
	if( hParent==NULL ) return FALSE;
	ICustButton* but = GetICustButton(GetDlgItem(hParent,id));
	BOOL retval = but->IsEnabled();
	ReleaseICustButton(but);
	return retval;
}

void EditPolyObject::OnParamSet(PB2Value& v, ParamID id, int tabIndex, TimeValue t) {
	switch( id ) {
	case ep_constrain_type:
		SetConstraintType(v.i);
		break;
	case ep_select_mode:
		SetSelectMode(v.i);
		break;
	case ep_ss_use:
		//When the soft selection is turned off, end any active paint session
		if( (!v.i) && InPaintMode() ) EndPaintMode();
		break;
	case ep_ss_lock:	{
			BOOL selActive = IsPaintDataActive(PAINTMODE_SEL);
			if( v.i && !selActive ) //Enable the selection lock; Activate the paintmesh selection
				ActivatePaintData(PAINTMODE_SEL);
			else if( (!v.i) && selActive ) { //Disable the selection lock; De-activate the paintmesh selection
				if( InPaintMode() ) EndPaintMode(); //end any active paint mode
				DeactivatePaintData(PAINTMODE_SEL);
			}
			//NOTE: SetEnables() is called by OnPaintDataActive() when activating or deactivating
			InvalidateDistanceCache ();
			IParamMap2 *pmap = GetEditablePolyDesc()->GetParamMap(&ep_param_block,ep_softsel);
			if (pmap) pmap->RedrawViews (t);
			break;
		}
	case ep_paintsel_size:
	case ep_paintdeform_size:
		SetPaintBrushSize(v.f);
		UpdatePaintBrush();
		break;
	case ep_paintsel_strength:
	case ep_paintdeform_strength:
		SetPaintBrushStrength(v.f);
		UpdatePaintBrush();
		break;
	case ep_paintsel_value:
	case ep_paintdeform_value:
	case ep_paintdeform_axis:
		UpdatePaintBrush(); break;
	case ep_paintsel_mode:
		if( v.i!=GetPaintMode() ) {
			BOOL valid = FALSE;
			HWND hwnd = (pSoftsel==NULL?  NULL : pSoftsel->GetHWnd());
			if( v.i==PAINTMODE_PAINTSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_PAINT );
			if( v.i==PAINTMODE_BLURSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_BLUR );
			if( v.i==PAINTMODE_ERASESEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_REVERT );
			if( v.i==PAINTMODE_OFF )		valid = TRUE;
			if( valid ) OnPaintBtn(v.i, t);
		}
		break;
	case ep_paintdeform_mode:
		if( v.i!=GetPaintMode() ) {
			BOOL valid = FALSE;
			HWND hwnd = (pPaintDeform==NULL?  NULL : pPaintDeform->GetHWnd());
			if( v.i==PAINTMODE_PAINTPUSHPULL )	valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_PUSHPULL );
			if( v.i==PAINTMODE_PAINTRELAX )		valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_RELAX );
			if( v.i==PAINTMODE_ERASEDEFORM )	valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_REVERT );
			if( v.i==PAINTMODE_OFF )			valid = TRUE;
			if( valid ) OnPaintBtn(v.i, t);
		}
		break;
	}
}
void EditPolyObject::OnParamGet(PB2Value& v, ParamID id, int tabIndex, TimeValue t, Interval &valid) {
	switch( id ) {
	//The selection lock is defined as being "on" anytime the paintmesh selection is active
	case ep_ss_lock: v.i = IsPaintDataActive(PAINTMODE_SEL); break;
	case ep_paintsel_size:
	case ep_paintdeform_size:
		v.f = GetPaintBrushSize(); break;
	case ep_paintsel_strength:
	case ep_paintdeform_strength:
		v.f = GetPaintBrushStrength(); break;
	case ep_paintsel_mode:
		v.i = GetPaintMode();
		if( !IsPaintSelMode(v.i) ) v.i=PAINTMODE_OFF;
		break;
	case ep_paintdeform_mode:
		v.i = GetPaintMode();
		if( !IsPaintDeformMode(v.i) ) v.i=PAINTMODE_OFF;
		break;
	}
}

void EditPolyObject::OnPaintBtn( int mode, TimeValue t ) {
	PaintModeID paintMode = (PaintModeID)mode;
	if( (mode==GetPaintMode()) || (mode==PAINTMODE_OFF) ) EndPaintMode();
	else if( IsPaintSelMode( paintMode ) ) {
		if( !pblock->GetInt(ep_ss_use) )	pblock->SetValue( ep_ss_use, t, TRUE );	//enable soft selection
		if( !pblock->GetInt(ep_ss_lock) )	pblock->SetValue( ep_ss_lock, t, TRUE );	//enable the soft selection lock
		// Exit any current command mode (like Bevel, Extrude, etc), if we're in one:
		ExitAllCommandModes (true, false);
		BeginPaintMode( paintMode );
	}
	else if( IsPaintDeformMode( paintMode ) ) {
		if( !IsPaintDataActive(paintMode) ) ActivatePaintData(paintMode);
		// Exit any current command mode, if we're in one:
		ExitAllCommandModes (true, false);
		BeginPaintMode( paintMode );
	}
}

TCHAR *EditPolyObject::GetPaintUndoString (PaintModeID mode) {
	if (IsPaintSelMode(mode)) return GetString (IDS_RESTOREOBJ_MESHPAINTSEL);
	return GetString (IDS_RESTOREOBJ_MESHPAINTDEFORM);
}


