/**********************************************************************
 *<
	FILE: EditPolyUI.cpp

	DESCRIPTION: UI code for Edit Poly Modifier

	CREATED BY: Steve Anderson,

	HISTORY: created March 2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "epoly.h"
#include "EditPoly.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"
#include "MeshDLib.h"
#include "EditPolyUI.h"

// Polymesh selection toolbar icons - used in select and edit tools.

#define IDC_SELVERTEX 0x3260
#define IDC_SELEDGE 0x3261
#define IDC_SELBORDER 0x3262
#define IDC_SELFACE 0x3263
#define IDC_SELELEMENT 0x3264

//--- ClassDescriptor and class vars ---------------------------------

static EditPolyModClassDesc emodDesc;
extern ClassDesc2* GetEditPolyDesc() {return &emodDesc;}

//---- Function Publishing Interface ---

enum epolyModEnumList { epolyModEnumButtonOps, epolyModEnumCommandModes, epolyModEnumPickModes,
	epolyModSelLevel, epolyModMeshSelLevel };

static const Matrix3 kMyStaticIdentityMatrix(true);

static FPInterfaceDesc editpolymod_fi (
EPOLY_MOD_INTERFACE, _T("EditPolyMod"), IDS_EDIT_POLY_MOD, &emodDesc, FP_MIXIN,
	epmod_get_sel_level, _T("GetEPolySelLevel"), 0, TYPE_ENUM, epolyModSelLevel, FP_NO_REDRAW, 0,
	epmod_get_mn_sel_level, _T("GetMeshSelLevel"), 0, TYPE_ENUM, epolyModMeshSelLevel, FP_NO_REDRAW, 0,
	epmod_set_sel_level, _T("SetEPolySelLevel"), 0, TYPE_VOID, 0, 1,
		_T("selectionLevel"), 0, TYPE_ENUM, epolyModSelLevel,

	epmod_convert_selection, _T("ConvertSelection"), 0, TYPE_INT, 0, 3,
		_T("fromLevel"), 0, TYPE_ENUM, epolyModSelLevel,
		_T("toLevel"), 0, TYPE_ENUM, epolyModSelLevel,
		_T("requireAll"), 0, TYPE_bool, f_keyArgDefault, false,

	epmod_get_operation, _T("GetOperation"), 0, TYPE_ENUM, epolyModEnumButtonOps, FP_NO_REDRAW, 0,
	epmod_set_operation, _T("SetOperation"), 0, TYPE_VOID, 0, 1,
		_T("operation"), 0, TYPE_ENUM, epolyModEnumButtonOps,
	epmod_popup_dialog, _T("PopupDialog"), 0, TYPE_VOID, 0, 1,
		_T("operation"), 0, TYPE_ENUM, epolyModEnumButtonOps,
	epmod_button_op, _T("ButtonOp"), 0, TYPE_VOID, 0, 1,
		_T("operation"), 0, TYPE_ENUM, epolyModEnumButtonOps,

	epmod_commit, _T("Commit"), 0, TYPE_VOID, 0, 0,
	epmod_commit_unless_animating, _T("CommitUnlessAnimating"), 0, TYPE_VOID, 0, 0,
	epmod_commit_and_repeat, _T("CommitAndRepeat"), 0, TYPE_VOID, 0, 0,
	epmod_cancel_operation, _T("Cancel"), 0, TYPE_VOID, 0, 0,

	epmod_get_selection, _T("GetSelection"), 0, TYPE_BITARRAY, FP_NO_REDRAW, 2,
		_T("meshSelLevel"), 0, TYPE_ENUM, epolyModMeshSelLevel,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_selection, _T("SetSelection"), 0, TYPE_bool, 0, 3,
		_T("meshSelLevel"), 0, TYPE_ENUM, epolyModMeshSelLevel,
		_T("newSelection"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_select, _T("Select"), 0, TYPE_bool, 0, 5,
		_T("meshSelLevel"), 0, TYPE_ENUM, epolyModMeshSelLevel,
		_T("newSelection"), 0, TYPE_BITARRAY_BR,
		_T("invert"), 0, TYPE_bool, f_keyArgDefault, false,
		_T("select"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_set_primary_node, _T("SetPrimaryNode"), 0, TYPE_VOID, 0, 1,
		_T("node"), 0, TYPE_INODE,

	epmod_toggle_command_mode, _T("ToggleCommandMode"), 0, TYPE_VOID, 0, 1,
		_T("commandMode"), 0, TYPE_ENUM, epolyModEnumCommandModes,
	epmod_enter_command_mode, _T("EnterCommandMode"), 0, TYPE_VOID, 0, 1,
		_T("commandMode"), 0, TYPE_ENUM, epolyModEnumCommandModes,
	epmod_enter_pick_mode, _T("EnterPickMode"), 0, TYPE_VOID, 0, 1,
		_T("commandMode"), 0, TYPE_ENUM, epolyModEnumPickModes,
	epmod_get_command_mode, _T("GetCommandMode"), 0, TYPE_ENUM, epolyModEnumCommandModes, 0, 0,
	epmod_get_pick_mode, _T("GetPickMode"), 0, TYPE_ENUM, epolyModEnumPickModes, 0, 0,

	epmod_repeat_last, _T("RepeatLastOperation"), 0, TYPE_VOID, 0, 0,

	epmod_move_selection, _T("MoveSelection"), 0, TYPE_VOID, 0, 4,
		_T("amount"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("localOrigin"), 0, TYPE_BOOL, f_keyArgDefault, false,
	epmod_rotate_selection, _T("RotateSelection"), 0, TYPE_VOID, 0, 4,
		_T("amount"), 0, TYPE_QUAT_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("localOrigin"), 0, TYPE_BOOL, f_keyArgDefault, false,
	epmod_scale_selection, _T("ScaleSelection"), 0, TYPE_VOID, 0, 4,
		_T("amount"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("localOrigin"), 0, TYPE_BOOL, f_keyArgDefault, false,

	epmod_move_slicer, _T("MoveSlicePlane"), 0, TYPE_VOID, 0, 3,
		_T("amount"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
	epmod_rotate_slicer, _T("RotateSlicePlane"), 0, TYPE_VOID, 0, 4,
		_T("amount"), 0, TYPE_QUAT_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("localOrigin"), 0, TYPE_BOOL, f_keyArgDefault, false,
	epmod_scale_slicer, _T("ScaleSlicePlane"), 0, TYPE_VOID, 0, 4,
		_T("amount"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("parent"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("axis"), 0, TYPE_MATRIX3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &kMyStaticIdentityMatrix,
		_T("localOrigin"), 0, TYPE_BOOL, f_keyArgDefault, false,

	epmod_in_slice_mode, _T("InSlicePlaneMode"), 0, TYPE_bool, FP_NO_REDRAW, 0,
	epmod_reset_slice_plane, _T("ResetSlicePlane"), 0, TYPE_VOID, 0, 0,
	epmod_get_slice_plane_tm, _T("GetSlicePlaneTM"), 0, TYPE_MATRIX3_BV, FP_NO_REDRAW, 0,
	epmod_set_slice_plane, _T("SetSlicePlane"), 0, TYPE_VOID, 0, 2,
		_T("planeNormal"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("planeCenter"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
	epmod_get_slice_plane, _T("GetSlicePlane"), 0, TYPE_VOID, 0, 2,
		_T("planeNormal"), 0, TYPE_POINT3_BR, f_inOut, FPP_OUT_PARAM,
		_T("planeCenter"), 0, TYPE_POINT3_BR, f_inOut, FPP_OUT_PARAM,

	epmod_create_vertex, _T("CreateVertex"), 0, TYPE_INDEX, 0, 2,
		_T("point"), 0, TYPE_POINT3,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_create_face, _T("CreateFace"), 0, TYPE_INDEX, 0, 2,
		_T("vertices"), 0, TYPE_INDEX_TAB,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_create_edge, _T("CreateEdge"), 0, TYPE_INDEX, 0, 3,
		_T("vertex1"), 0, TYPE_INDEX,
		_T("vertex2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_diagonal, _T("SetDiagonal"), 0, TYPE_VOID, 0, 3,
		_T("vertex1"), 0, TYPE_INDEX,
		_T("vertex2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_cut, _T("StartCut"), 0, TYPE_VOID, 0, 5,
		_T("level"), 0, TYPE_ENUM, epolyModMeshSelLevel,
		_T("index"), 0, TYPE_INDEX,
		_T("point"), 0, TYPE_POINT3,
		_T("normal"), 0, TYPE_POINT3,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_target_weld_vertex, _T("WeldVertices"), 0, TYPE_VOID, 0, 3,
		_T("vertex1"), 0, TYPE_INDEX,
		_T("vertex2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_target_weld_edge, _T("WeldEdges"), 0, TYPE_VOID, 0, 3,
		_T("edge1"), 0, TYPE_INDEX,
		_T("edge2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_bridge_borders, _T("BridgeBorders"), 0, TYPE_VOID, 0, 3,
		_T("edge1"), 0, TYPE_INDEX,
		_T("edge2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_bridge_polygons, _T("BridgePolygons"), 0, TYPE_VOID, 0, 3,
		_T("poly1"), 0, TYPE_INDEX,
		_T("poly2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_bridge_edges, _T("BridgeEdges"), 0, TYPE_VOID, 0, 3,
		_T("edge1"), 0, TYPE_INDEX,
		_T("edge2"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_bridge_node, _T("GetBridgeNode"), 0, TYPE_INODE, 0, 0,
	epmod_set_bridge_node, _T("SetBridgeNode"), 0, TYPE_VOID, 0, 1,
		_T("node"), 0, TYPE_INODE,

	epmod_get_preserve_map, _T("GetPreserveMap"), 0, TYPE_bool, 0, 1,
		_T("mapChannel"), 0, TYPE_INT,
	epmod_set_preserve_map, _T("SetPreserveMap"), 0, TYPE_VOID, 0, 2,
		_T("mapChannel"), 0, TYPE_INT,
		_T("preserve"), 0, TYPE_bool,

	epmod_attach_node, _T("Attach"), 0, TYPE_VOID, 0, 2,
		_T("nodeToAttach"), 0, TYPE_INODE,
		_T("editPolyNode"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_attach_nodes, _T("AttachList"), 0, TYPE_VOID, 0, 2,
		_T("nodesToAttach"), 0, TYPE_INODE_TAB_BR, f_inOut, FPP_IN_PARAM,
		_T("editPolyNode"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_detach_to_object, _T("DetachToObject"), 0, TYPE_VOID, 0, 1,
		_T("newObjectName"), 0, TYPE_TSTR_BR,
	epmod_create_shape, _T("CreateShape"), 0, TYPE_VOID, 0, 1,
		_T("newShapeName"), 0, TYPE_TSTR_BR,

	epmod_get_num_vertices, _T("GetNumVertices"), 0, TYPE_INT, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex, _T("GetVertex"), 0, TYPE_POINT3_BV,FP_NO_REDRAW, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex_face_count, _T("GetVertexFaceCount"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex_face, _T("GetVertexFace"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("face"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex_edge_count, _T("GetVertexEdgeCount"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex_edge, _T("GetVertexEdge"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("edge"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_get_num_edges, _T("GetNumEdges"), 0, TYPE_INT, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_edge_vertex, _T("GetEdgeVertex"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("end"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_edge_face, _T("GetEdgeFace"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_get_num_faces, _T("GetNumFaces"), 0, TYPE_INT, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_degree, _T("GetFaceDegree"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_vertex, _T("GetFaceVertex"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("faceID"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_edge, _T("GetFaceEdge"), 0, TYPE_INDEX, FP_NO_REDRAW, 3,
		_T("faceID"), 0, TYPE_INDEX,
		_T("side"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_material, _T("GetFaceMaterial"), 0, TYPE_INDEX, FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_smoothing_group, _T("GetFaceSmoothingGroups"), 0, TYPE_DWORD, FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_get_num_map_channels, _T("GetNumMapChannels"), 0, TYPE_INT, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_map_channel_active, _T("GetMapChannelActive"), 0, TYPE_bool, FP_NO_REDRAW, 2,
		_T("mapChannel"), 0, TYPE_INT,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_num_map_vertices, _T("GetNumMapVertices"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("mapChannel"), 0, TYPE_INT,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_map_vertex, _T("GetMapVertex"), 0, TYPE_POINT3_BV, FP_NO_REDRAW, 3,
		_T("mapChannel"), 0, TYPE_INT,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_map_face_vertex, _T("GetMapFaceVert"), 0, TYPE_INDEX, FP_NO_REDRAW, 4,
		_T("mapChannel"), 0, TYPE_INT,
		_T("faceID"), 0, TYPE_INDEX,
		_T("corner"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_list_operations, _T("List"), 0, TYPE_VOID, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_local_data_changed, _T("LocalDataChanged"), 0, TYPE_VOID, 0, 1,
		_T("parts"), 0, TYPE_DWORD,
	epmod_refresh_screen, _T("RefreshScreen"), 0, TYPE_VOID, 0, 0,
	epmod_in_slice, _T("InSlice"), 0, TYPE_bool, FP_NO_REDRAW, 0,
	epmod_show_operation_dialog, _T("ShowOperationDialog"), 0, TYPE_bool, FP_NO_REDRAW, 0,
	epmod_showing_operation_dialog, _T("ShowingOperationDialog"), 0, TYPE_bool, FP_NO_REDRAW, 0,
	epmod_close_operation_dialog, _T("CloseOperationDialog"), 0, TYPE_VOID, FP_NO_REDRAW, 0,
	epmod_get_primary_node, _T("GetPrimaryNode"), 0, TYPE_INODE, FP_NO_REDRAW, 0,
	epmod_get_node_tm, _T("GetNodeTM"), 0, TYPE_MATRIX3_BV, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_cut_end, _T("SetCutEnd"), 0, TYPE_VOID, 0, 2,
		_T("endPoint"), 0, TYPE_POINT3,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_last_cut_end, _T("GetLastCutEnd"), 0, TYPE_INDEX, FP_NO_REDRAW, 0,
	epmod_clear_last_cut_end, _T("ClearLastCutEnd"), 0, TYPE_VOID, FP_NO_REDRAW, 0,
	epmod_cut_cancel, _T("CutCancel"), 0, TYPE_VOID, 0, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_divide_edge, _T("DivideEdge"), 0, TYPE_INDEX, 0, 3,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("proportion"), 0, TYPE_FLOAT,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_divide_face, _T("DivideFace"), 0, TYPE_INDEX, 0, 3,
		_T("faceID"), 0, TYPE_INDEX,
		_T("vertexCoefficients"), 0, TYPE_FLOAT_TAB,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_ring_sel, _T("RingSelect"), 0, TYPE_VOID, 0, 3,
		_T("RingValue"), 0, TYPE_INT,
		_T("MoveOnly"), 0, TYPE_bool,
		_T("Add"), 0, TYPE_bool, 
	epmod_loop_sel, _T("LoopSelect"), 0, TYPE_VOID, 0, 3,
		_T("LoopValue"), 0, TYPE_INT,
		_T("MoveOnly"), 0, TYPE_bool,
		_T("Add"), 0, TYPE_bool, 
	epmod_convert_selection_to_border, _T("ConvertSelectionToBorder"), 0, TYPE_INT, 0, 2,
		_T("fromLevel"), 0, TYPE_ENUM, epolyModSelLevel,
		_T("toLevel"), 0, TYPE_ENUM, epolyModSelLevel,
	epmod_paintdeform_commit, _T("CommitPaintDeform"), 0, TYPE_VOID, 0, 0,
	epmod_paintdeform_cancel, _T("CancelPaintDeform"), 0, TYPE_VOID, 0, 0,

	epmod_get_face_normal, _T("GetFaceNormal"), 0, TYPE_POINT3_BV,FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_center, _T("GetFaceCenter"), 0, TYPE_POINT3_BV,FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_area, _T("GetFaceArea"), 0, TYPE_FLOAT, FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_open_edges, _T("GetOpenEdges"), 0, TYPE_BITARRAY, FP_NO_REDRAW, 1,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_get_verts_by_flag, _T("GetVerticesByFlag"), 0, TYPE_bool, FP_NO_REDRAW, 4,
		_T("vertexSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_edges_by_flag, _T("GetEdgesByFlag"), 0, TYPE_bool, FP_NO_REDRAW, 4,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_faces_by_flag, _T("GetFacesByFlag"), 0, TYPE_bool, FP_NO_REDRAW, 4,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsRequested"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_vertex_flags, _T("setVertexFlags"), 0, TYPE_VOID, 0, 5,
		_T("vertexSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_edge_flags, _T("setEdgeFlags"), 0, TYPE_VOID, 0, 5,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_set_face_flags, _T("setFaceFlags"), 0, TYPE_VOID, 0, 5,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("flagsToSet"), 0, TYPE_DWORD,
		_T("flagMask"), 0, TYPE_DWORD, f_keyArgDefault, 0,
		_T("generateUndoRecord"), 0, TYPE_bool, f_keyArgDefault, true,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_vertex_flags, _T("getVertexFlags"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("vertexID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_edge_flags, _T("getEdgeFlags"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("edgeID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_face_flags, _T("getFaceFlags"), 0, TYPE_INT, FP_NO_REDRAW, 2,
		_T("faceID"), 0, TYPE_INDEX,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_get_verts_using_edge, _T("getVertsUsingEdge"), 0, TYPE_VOID, FP_NO_REDRAW, 3,
		_T("vertSet"), 0, TYPE_BITARRAY_BR,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_edges_using_vert, _T("getEdgesUsingVert"), 0, TYPE_VOID, FP_NO_REDRAW, 3,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("vertSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_faces_using_edge, _T("getFacesUsingEdge"), 0, TYPE_VOID, FP_NO_REDRAW, 3,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("edgeSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_elements_using_face, _T("getElementsUsingFace"), 0, TYPE_VOID, FP_NO_REDRAW, 4,
		_T("elementSet"), 0, TYPE_BITARRAY_BR,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("fenceSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_faces_using_vert, _T("getFacesUsingVert"), 0, TYPE_VOID, FP_NO_REDRAW, 3,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("vertSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,
	epmod_get_verts_using_face, _T("getVertsUsingFace"), 0, TYPE_VOID, FP_NO_REDRAW, 3,
		_T("vertSet"), 0, TYPE_BITARRAY_BR,
		_T("faceSet"), 0, TYPE_BITARRAY_BR,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	epmod_set_vert, _T("setVert"), 0, TYPE_VOID, 0, 3,
		_T("vertSet"), 0, TYPE_BITARRAY_BR,
		_T("point"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM,
		_T("node"), 0, TYPE_INODE, f_keyArgDefault, NULL,

	enums,
		epolyModEnumButtonOps, 80,
			_T("GrowSelection"), ep_op_sel_grow,
			_T("ShrinkSelection"), ep_op_sel_shrink,
			_T("SelectEdgeLoop"), ep_op_select_loop,
			_T("SelectEdgeRing"), ep_op_select_ring,
			_T("GetStackSelection"), ep_op_get_stack_selection,
			_T("HideVertex"), ep_op_hide_vertex,
			_T("HideFace"), ep_op_hide_face,
			_T("HideUnselectedVertex"), ep_op_hide_unsel_vertex,
			_T("HideUnselectedFace"), ep_op_hide_unsel_face,
			_T("UnhideAllVertex"), ep_op_unhide_vertex,
			_T("UnhideAllFace"), ep_op_unhide_face,
			_T("NamedSelectionCopy"), ep_op_ns_copy,
			_T("NamedSelectionPaste"), ep_op_ns_paste,
			_T("Cap"), ep_op_cap,
			_T("DeleteVertex"), ep_op_delete_vertex,
			_T("DeleteEdge"), ep_op_delete_edge,
			_T("DeleteFace"), ep_op_delete_face,
			_T("RemoveVertex"), ep_op_remove_vertex,
			_T("RemoveEdge"), ep_op_remove_edge,
			_T("DetachVertex"), ep_op_detach_vertex,
			_T("DetachFace"), ep_op_detach_face,
			_T("SplitEdges"), ep_op_split,
			_T("BreakVertex"), ep_op_break,
			_T("CollapseVertex"), ep_op_collapse_vertex,
			_T("CollapseEdge"), ep_op_collapse_edge,
			_T("CollapseFace"), ep_op_collapse_face,
			_T("ResetSlicePlane"), ep_op_reset_plane,
			_T("Slice"), ep_op_slice,
			_T("SliceSelectedFaces"), ep_op_slice_face,
			_T("Cut"), ep_op_cut,
			_T("WeldVertex"), ep_op_weld_vertex,
			_T("WeldEdge"), ep_op_weld_edge,
			_T("CreateShape"), ep_op_create_shape,
			_T("MakePlanar"), ep_op_make_planar,
			_T("MakePlanarInX"), ep_op_make_planar_x,
			_T("MakePlanarInY"), ep_op_make_planar_y,
			_T("MakePlanarInZ"), ep_op_make_planar_z,
			_T("Align"), ep_op_align,
			_T("Relax"), ep_op_relax,
			_T("MeshSmooth"), ep_op_meshsmooth,
			_T("Tessellate"), ep_op_tessellate,
			_T("Retriangulate"), ep_op_retriangulate,
			_T("FlipFace"), ep_op_flip_face,
			_T("FlipElement"), ep_op_flip_element,
			_T("ExtrudeVertex"), ep_op_extrude_vertex,
			_T("ExtrudeEdge"), ep_op_extrude_edge,
			_T("ExtrudeFace"), ep_op_extrude_face,
			_T("Bevel"), ep_op_bevel,
			_T("Inset"), ep_op_inset,
			_T("Outline"), ep_op_outline,
			_T("ExtrudeAlongSpline"), ep_op_extrude_along_spline,
			_T("HingeFromEdge"), ep_op_hinge_from_edge,
			_T("ConnectVertices"), ep_op_connect_vertex,
			_T("ConnectEdges"), ep_op_connect_edge,
			_T("ChamferVertex"), ep_op_chamfer_vertex,
			_T("ChamferEdge"), ep_op_chamfer_edge,
			_T("RemoveIsoVerts"), ep_op_remove_iso_verts,
			_T("RemoveIsoMapVerts"), ep_op_remove_iso_map_verts,
			_T("ToggleShadedFaces"), ep_op_toggle_shaded_faces,
			_T("Transform"), ep_op_transform,
			_T("Create"), ep_op_create,
			_T("Attach"), ep_op_attach,
			_T("TargetWeldVertex"), ep_op_target_weld_vertex,
			_T("TargetWeldEdge"), ep_op_target_weld_edge,
			_T("EditTriangulation"), ep_op_edit_triangulation,
			_T("TurnEdge"), ep_op_turn_edge,
			_T("CreateEdge"), ep_op_create_edge,
			_T("CloneVertex"), ep_op_clone_vertex,
			_T("CloneEdge"), ep_op_clone_edge,
			_T("CloneFace"), ep_op_clone_face,
			_T("InsertVertexInEdge"), ep_op_insert_vertex_edge,
			_T("InsertVertexInFace"), ep_op_insert_vertex_face,
			_T("Autosmooth"), ep_op_autosmooth,
			_T("SetSmooth"), ep_op_smooth,
			_T("SetMaterial"), ep_op_set_material,
			_T("SelectBySmooth"), ep_op_select_by_smooth,
			_T("SelectByMaterial"), ep_op_select_by_material,
			_T("BridgeBorder"), ep_op_bridge_border,
			_T("BridgePolygon"), ep_op_bridge_polygon,
			_T("BridgeEdge"), ep_op_bridge_edge,

		epolyModEnumCommandModes, 25,
			_T("CreateVertex"), ep_mode_create_vertex,
			_T("CreateEdge"), ep_mode_create_edge,
			_T("CreateFace"), ep_mode_create_face,
			_T("DivideEdge"), ep_mode_divide_edge,
			_T("DivideFace"), ep_mode_divide_face,
			_T("ExtrudeVertex"), ep_mode_extrude_vertex,
			_T("ExtrudeEdge"), ep_mode_extrude_edge,
			_T("ExtrudeFace"), ep_mode_extrude_face,
			_T("ChamferVertex"), ep_mode_chamfer_vertex,
			_T("ChamferEdge"), ep_mode_chamfer_edge,
			_T("Bevel"), ep_mode_bevel, 
			_T("InsetFace"), ep_mode_inset_face,
			_T("OutlineFace"), ep_mode_outline,
			_T("SlicePlane"), ep_mode_sliceplane,
			_T("Cut"), ep_mode_cut,
			_T("Weld"), ep_mode_weld,
			_T("EditTriangulation"), ep_mode_edit_tri,
			_T("TurnEdge"), ep_mode_turn_edge,
			_T("QuickSlice"), ep_mode_quickslice,
			_T("HingeFromEdge"), ep_mode_hinge_from_edge,
			_T("PickHinge"), ep_mode_pick_hinge,
			_T("BridgeBorder"), ep_mode_bridge_border,
			_T("BridgePolygon"), ep_mode_bridge_polygon,
			_T("PickBridgeTarget1"), ep_mode_pick_bridge_1,
			_T("PickBridgeTarget2"), ep_mode_pick_bridge_2,

		epolyModEnumPickModes, 2,
			_T("Attach"),	ep_mode_attach, 
			_T("PickShape"),	ep_mode_pick_shape,

		epolyModSelLevel, 7,
			_T("Object"), EPM_SL_OBJECT,
			_T("Vertex"), EPM_SL_VERTEX,
			_T("Edge"), EPM_SL_EDGE,
			_T("Border"), EPM_SL_BORDER,
			_T("Face"), EPM_SL_FACE,
			_T("Element"), EPM_SL_ELEMENT,
			_T("CurrentLevel"), EPM_SL_CURRENT,

		epolyModMeshSelLevel, 5,
			_T("Object"), MNM_SL_OBJECT,
			_T("Vertex"), MNM_SL_VERTEX,
			_T("Edge"), MNM_SL_EDGE,
			_T("Face"), MNM_SL_FACE,
			_T("CurrentLevel"), MNM_SL_CURRENT,
	end
);

FPInterfaceDesc *EPolyMod::GetDesc () {
	return &editpolymod_fi;
}

FPInterfaceDesc *GetEPolyModFPInterface ()
{
	return &editpolymod_fi;
}

// This is very useful for macrorecording.

TCHAR *LookupOperationName (int opID) {
	int count = editpolymod_fi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (editpolymod_fi.enumerations[i]->ID == epolyModEnumButtonOps) break;
	if (i==count) return NULL;

	FPEnum *fpenum = editpolymod_fi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == opID) return fpenum->enumeration[i].name;
	}

	return NULL;
}

TCHAR *LookupEditPolySelLevel (int selLevel) {
	int count = editpolymod_fi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (editpolymod_fi.enumerations[i]->ID == epolyModSelLevel) break;
	if (i==count) return NULL;

	FPEnum *fpenum = editpolymod_fi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == selLevel) return fpenum->enumeration[i].name;
	}

	return NULL;
}

TCHAR *LookupMNMeshSelLevel (int selLevel) {
	int count = editpolymod_fi.enumerations.Count();
   int i;
	for (i=0; i<count; i++)
		if (editpolymod_fi.enumerations[i]->ID == epolyModMeshSelLevel) break;
	if (i==count) return NULL;

	FPEnum *fpenum = editpolymod_fi.enumerations[i];
	for (i=0; i<fpenum->enumeration.Count(); i++)
	{
		if (fpenum->enumeration[i].code == selLevel) return fpenum->enumeration[i].name;
	}

	return NULL;
}

void *EditPolyModClassDesc::Create (BOOL loading) {
	AddInterface (GetEPolyModFPInterface());
	return new EditPolyMod;
}

class EditPolyModPBAccessor : public PBAccessor {
	public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
		{ ((EditPolyMod*)owner)->OnParamSet( v, id, tabIndex, t ); }
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
		{ ((EditPolyMod*)owner)->OnParamGet( v, id, tabIndex, t, valid ); }
};
static EditPolyModPBAccessor theEditPolyModPBAccessor;

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
		((EditPolyMod*)owner)->getParamBlock()->GetValue(epm_constrain_type, TimeValue(0), last, FOREVER);
		((EditPolyMod*)owner)->SetLastConstrainType(last);
		return this->Validate(v); 
	}
};

static EditPolyConstraintValidatorClass theEPConstraintValidator;
//--- Parameter map/block descriptors -------------------------------

const Matrix3 kIdentityMatrix(true);

// Parameters
static ParamBlockDesc2 edit_mod_param_blk (kEditPolyParams, _T("Edit Poly Parameters"), 0, &emodDesc,
											   P_AUTO_CONSTRUCT | P_AUTO_UI | P_MULTIMAP, EDIT_PBLOCK_REF,
	// rollout descriptions:
	// (Leave out ep_settings)
	7,
	ep_animate, IDD_EDITPOLY_ANIMATE, IDS_EDITPOLY_EDIT_POLY, 0, 0, NULL,
	ep_select, IDD_EDITPOLY_SELECT, IDS_EDITPOLY_SELECTION, 0, 0, NULL,
	ep_softsel, IDD_EDITPOLY_SOFTSEL, IDS_SOFTSEL, 0, APPENDROLL_CLOSED, NULL,
	ep_geom, IDD_EDITPOLY_GEOM, IDS_EDIT_GEOM, 0, 0, NULL,
	ep_surface, IDD_EDITPOLY_SURF_FACE, IDS_EP_POLY_MATERIALIDS, 0, 0, NULL,
	ep_paintdeform, IDD_EDITPOLY_PAINTDEFORM, IDS_PAINTDEFORM, 0, 0, NULL,
	ep_face_smooth, IDD_EDITPOLY_POLY_SMOOTHING, IDS_EDITPOLY_POLY_SMOOTHING, 0 ,0, NULL,

	// Parameters
	// Type of edit:
	epm_current_operation, _T("currentOperation"), TYPE_INT, P_RESET_DEFAULT, IDS_EDITPOLY_EDIT_TYPE,
		p_default, ep_op_null,
		end,

	epm_animation_mode, _T("animationMode"), TYPE_INT, P_RESET_DEFAULT, IDS_EDITPOLY_ANIMATION_MODE,
		p_default, 0,
		p_ui, ep_animate, TYPE_RADIO, 2,
			IDC_EDITPOLY_MODELING_MODE, IDC_EDITPOLY_ANIMATION_MODE,
		end,

	epm_show_cage, _T("showCage"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SHOW_CAGE,
		p_default, false,
		p_ui, ep_animate, TYPE_SINGLECHEKBOX, IDC_SHOW_CAGE,
		end,

	epm_stack_selection, _T("useStackSelection"), TYPE_BOOL, P_RESET_DEFAULT, IDS_EDITPOLY_USE_STACK_SELECTION,
		p_default, 0,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_EDITPOLY_STACK_SELECTION,
		end,

	epm_by_vertex, _T("selectByVertex"), TYPE_BOOL, P_RESET_DEFAULT, IDS_EDITPOLY_SELECT_BY_VERTEX,
		p_default, 0,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_SEL_BYVERT,
		end,

	epm_ignore_backfacing, _T("ignoreBackfacing"), TYPE_BOOL, P_RESET_DEFAULT, IDS_EDITPOLY_IGNORE_BACKFACING,
		p_default, 0,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_IGNORE_BACKFACES,
		end,

	epm_select_by_angle, _T("selectByAngle"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SELECT_BY_ANGLE,
		p_default, false,
		p_enable_ctrls, 1, epm_select_angle,
		p_ui, ep_select, TYPE_SINGLECHEKBOX, IDC_SELECT_BY_ANGLE,
		end,

	epm_select_angle, _T("selectAngle"), TYPE_ANGLE, P_RESET_DEFAULT, IDS_SELECT_ANGLE,
		p_default, PI/4.0f,
		p_range, 0.0f, 180.0f,
		p_ui, ep_select, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
		IDC_SELECT_ANGLE, IDC_SELECT_ANGLE_SPIN, SPIN_AUTOSCALE,
		end,

	epm_ss_use, _T("useSoftSel"), TYPE_BOOL, 0, IDS_USE_SOFTSEL,
		p_default, FALSE,
		//p_enable_ctrls, 4, epm_ss_affect_back,
		//	epm_ss_falloff, epm_ss_pinch, epm_ss_bubble,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_USE_SOFTSEL,
		end,

	epm_ss_edist_use, _T("ssUseEdgeDist"), TYPE_BOOL, 0, IDS_USE_EDIST,
		p_default, FALSE,
		p_enable_ctrls, 1, epm_ss_edist,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_USE_EDIST,
		end,

	epm_ss_edist, _T("ssEdgeDist"), TYPE_INT, 0, IDS_EDGE_DIST,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_EDIST, IDC_EDIST_SPIN, .2f,
		end,

	epm_ss_affect_back, _T("affectBackfacing"), TYPE_BOOL, 0, IDS_AFFECT_BACKFACING,
		p_default, TRUE,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_AFFECT_BACKFACING,
		end,

	epm_ss_falloff, _T("falloff"), TYPE_WORLD, P_ANIMATABLE, IDS_FALLOFF,
		p_default, 20.0f,
		p_range, 0.0f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_FALLOFF, IDC_FALLOFFSPIN, SPIN_AUTOSCALE,
		end,

	epm_ss_pinch, _T("pinch"), TYPE_FLOAT, P_ANIMATABLE, IDS_PINCH,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PINCH, IDC_PINCHSPIN, SPIN_AUTOSCALE,
		end,

	epm_ss_bubble, _T("bubble"), TYPE_FLOAT, P_ANIMATABLE, IDS_BUBBLE,
		p_default, 0.0f,
		p_range, -999999.f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BUBBLE, IDC_BUBBLESPIN, SPIN_AUTOSCALE,
		end,

	epm_ss_lock, _T("lockSoftSel"), TYPE_BOOL, P_TRANSIENT, IDS_LOCK_SOFTSEL,
		p_accessor, &theEditPolyModPBAccessor,
		p_ui, ep_softsel, TYPE_SINGLECHEKBOX, IDC_LOCK_SOFTSEL,
		end,

	epm_paintsel_mode, _T("paintSelMode"), TYPE_INT, P_TRANSIENT, IDS_PAINTSEL_MODE,
		p_accessor,		&theEditPolyModPBAccessor,
		p_default, PAINTMODE_OFF,
		end,

	epm_paintsel_value, _T("paintSelValue"), TYPE_FLOAT, 0, IDS_PAINTSEL_VALUE,
		p_accessor, &theEditPolyModPBAccessor,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_VALUE_EDIT, IDC_PAINTSEL_VALUE_SPIN, 0.01f,
		end,

	epm_paintsel_size, _T("paintSelSize"), TYPE_WORLD, P_TRANSIENT, IDS_PAINTSEL_SIZE,
		p_accessor, &theEditPolyModPBAccessor,
		p_range, 0.0f, 999999.f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTSEL_SIZE_EDIT, IDC_PAINTSEL_SIZE_SPIN, SPIN_AUTOSCALE,
		end,

	epm_paintsel_strength, _T("paintSelStrength"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTSEL_STRENGTH,
		p_accessor, &theEditPolyModPBAccessor,
		p_range, 0.0f, 1.0f,
		p_ui, ep_softsel, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTSEL_STRENGTH_EDIT, IDC_PAINTSEL_STRENGTH_SPIN, 0.01f,
		end,

	epm_paintdeform_mode, _T("paintDeformMode"), TYPE_INT, P_TRANSIENT, IDS_PAINTDEFORM_MODE,
		p_accessor,		&theEditPolyModPBAccessor,
		p_default, PAINTMODE_OFF,
		end,

	epm_paintdeform_value, _T("paintDeformValue"), TYPE_FLOAT, 0, IDS_PAINTDEFORM_VALUE,
		p_accessor, &theEditPolyModPBAccessor,
		p_default, 10.0f,
		p_range, -99999999.0f, 99999999.0f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTDEFORM_VALUE_EDIT, IDC_PAINTDEFORM_VALUE_SPIN, SPIN_AUTOSCALE,
		end,

	epm_paintdeform_size, _T("paintDeformSize"), TYPE_WORLD, P_TRANSIENT, IDS_PAINTDEFORM_SIZE,
		p_accessor, &theEditPolyModPBAccessor,
		p_range, 0.0f, 999999.f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_PAINTDEFORM_SIZE_EDIT, IDC_PAINTDEFORM_SIZE_SPIN, SPIN_AUTOSCALE,
		end,

	epm_paintdeform_strength, _T("paintDeformStrength"), TYPE_FLOAT, P_TRANSIENT, IDS_PAINTDEFORM_STRENGTH,
		p_accessor, &theEditPolyModPBAccessor,
		p_range, 0.0f, 1.0f,
		p_ui, ep_paintdeform, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_PAINTDEFORM_STRENGTH_EDIT, IDC_PAINTDEFORM_STRENGTH_SPIN, 0.01f,
		end,

	epm_paintdeform_axis, _T("paintDeformAxis"), TYPE_INT, 0, IDS_PAINTDEFORM_AXIS,
		p_accessor, &theEditPolyModPBAccessor,
		p_default, PAINTAXIS_ORIGNORMS,
		p_ui, ep_paintdeform, TYPE_RADIO, 5, IDC_PAINTAXIS_ORIGNORMS, IDC_PAINTAXIS_DEFORMEDNORMS,
			IDC_PAINTAXIS_XFORM_X, IDC_PAINTAXIS_XFORM_Y, IDC_PAINTAXIS_XFORM_Z,
		p_vals, 	PAINTAXIS_ORIGNORMS, PAINTAXIS_DEFORMEDNORMS,
			PAINTAXIS_XFORM_X, PAINTAXIS_XFORM_Y, PAINTAXIS_XFORM_Z,
		end,

	epm_constrain_type, _T("constrainType"), TYPE_INT, 0, IDS_CONSTRAIN_TYPE,
		p_default, 0,
		p_range, 0, 3,
		p_accessor, &theEditPolyModPBAccessor,
		p_validator, &theEPConstraintValidator,
		end,

	epm_preserve_maps, _T("preserveUVs"), TYPE_BOOL, 0, IDS_PRESERVE_MAPS,
		p_default, FALSE,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_PRESERVE_MAPS,
		end,

	epm_split, _T("split"), TYPE_BOOL, 0, IDS_SPLIT,
		p_default, FALSE,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_SPLIT,
		end,

	// Individual edit type parameters
	epm_ms_smoothness, _T("smoothness"), TYPE_FLOAT, P_TRANSIENT, IDS_MS_SMOOTHNESS,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_MS_SMOOTH, IDC_MS_SMOOTHSPIN, .001f,
		end,

	epm_ms_sep_smooth, _T("separateBySmoothing"), TYPE_BOOL, P_TRANSIENT, IDS_MS_SEP_BY_SMOOTH,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_SMOOTH,
		end,

	epm_ms_sep_mat, _T("separateByMaterial"), TYPE_BOOL, P_TRANSIENT, IDS_MS_SEP_BY_MATID,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_MS_SEP_BY_MATID,
		end,

	epm_delete_isolated_verts, _T("deleteIsolatedVerts"), TYPE_BOOL, 0, IDS_DELETE_ISOLATED_VERTS,
		p_default, true,
		p_ui, ep_geom, TYPE_SINGLECHEKBOX, IDC_EDITPOLY_DELETE_ISO_VERTS,
		end,

	epm_extrude_face_type, _T("extrudeFaceType"), TYPE_INT, 0, IDS_EXTRUSION_TYPE,
		p_default, 0,	// Group normal
		p_ui, ep_settings, TYPE_RADIO, 3,
			IDC_EXTYPE_GROUP, IDC_EXTYPE_LOCAL, IDC_EXTYPE_BY_POLY,
		end,

	epm_extrude_face_height, _T("extrudeFaceHeight"), TYPE_WORLD, P_ANIMATABLE, IDS_FACE_EXTRUDE_HEIGHT,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_FACE_EXTRUDE_HEIGHT, IDC_FACE_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	epm_extrude_vertex_height, _T("extrudeVertexHeight"), TYPE_WORLD, P_ANIMATABLE, IDS_VERTEX_EXTRUDE_HEIGHT,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_VERTEX_EXTRUDE_HEIGHT, IDC_VERTEX_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	epm_extrude_edge_height, _T("extrudeEdgeHeight"), TYPE_WORLD, P_ANIMATABLE, IDS_EDGE_EXTRUDE_HEIGHT,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_EDGE_EXTRUDE_HEIGHT, IDC_EDGE_EXTRUDE_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	epm_extrude_vertex_width, _T("extrudeVertexWidth"), TYPE_WORLD, P_ANIMATABLE, IDS_VERTEX_EXTRUDE_WIDTH,
		p_default, 0.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTEX_EXTRUDE_WIDTH, IDC_VERTEX_EXTRUDE_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	epm_extrude_edge_width, _T("extrudeEdgeWidth"), TYPE_WORLD, P_ANIMATABLE, IDS_EDGE_EXTRUDE_WIDTH,
		p_default, 0.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGE_EXTRUDE_WIDTH, IDC_EDGE_EXTRUDE_WIDTH_SPIN, SPIN_AUTOSCALE,
		end,

	epm_bevel_type, _T("bevelType"), TYPE_INT, 0, IDS_BEVEL_TYPE,
		p_default, 0,	// Group normal
		p_ui, ep_settings, TYPE_RADIO, 3,
			IDC_BEVTYPE_GROUP, IDC_BEVTYPE_LOCAL, IDC_BEVTYPE_BY_POLY,
		end,

	epm_bevel_height, _T("bevelHeight"), TYPE_WORLD, P_ANIMATABLE, IDS_BEVEL_HEIGHT,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_BEVEL_HEIGHT, IDC_BEVEL_HEIGHT_SPIN, SPIN_AUTOSCALE,
		end,

	epm_bevel_outline, _T("bevelOutline"), TYPE_WORLD, P_ANIMATABLE, IDS_BEVEL_OUTLINE,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_BEVEL_OUTLINE, IDC_BEVEL_OUTLINE_SPIN, SPIN_AUTOSCALE,
		end,

	epm_outline, _T("outlineAmount"), TYPE_WORLD, P_ANIMATABLE, IDS_OUTLINE_AMOUNT,
		p_default, 0.0f,
		p_range, -9999999.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_UNIVERSE,
			IDC_OUTLINEAMOUNT, IDC_OUTLINESPINNER, SPIN_AUTOSCALE,
		end,

	epm_inset, _T("insetAmount"), TYPE_WORLD, P_ANIMATABLE, IDS_INSET_AMOUNT,
		p_default, 0.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_INSETAMOUNT, IDC_INSETSPINNER, SPIN_AUTOSCALE,
		end,

	epm_inset_type, _T("insetType"), TYPE_INT, 0, IDS_INSET_TYPE,
		p_default, 0,	// Inset by cluster
		p_ui, ep_settings, TYPE_RADIO, 2,
			IDC_INSETTYPE_GROUP, IDC_INSETTYPE_BY_POLY,
		end,

	epm_chamfer_vertex, _T("chamferVertexAmount"), TYPE_WORLD, P_ANIMATABLE, IDS_VERTEX_CHAMFER_AMOUNT,
		p_default, 0.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTEX_CHAMFER, IDC_VERTEX_CHAMFER_SPIN, SPIN_AUTOSCALE,
		end,

	epm_chamfer_vertex_open, _T("chamferVertexOpen"), TYPE_BOOL, P_TRANSIENT, IDS_OPEN_CHAMFER,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_CHAMFER_VERTEX_OPEN,
		end,

	epm_chamfer_edge, _T("chamferEdgeAmount"), TYPE_WORLD, P_ANIMATABLE, IDS_EDGE_CHAMFER_AMOUNT,
		p_default, 1.0f,
		p_range, 0.0f, 99999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGE_CHAMFER, IDC_EDGE_CHAMFER_SPIN, SPIN_AUTOSCALE,
		end,

	epm_edge_chamfer_segments, _T("edgeChamferSegments"), TYPE_INT, P_TRANSIENT, IDS_EDGE_CHAMFER_SEGMENTS,
		p_default, 1,
		p_range, 1, 999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
		IDC_EDGE_CHAMFER_SEGMENTS, IDC_EDGE_CHAMFER_SEGMENTS_SPIN, .5f,
		end,

	epm_chamfer_edge_open, _T("chamferEdgeOpen"), TYPE_BOOL, P_TRANSIENT, IDS_OPEN_CHAMFER,
		p_default, FALSE,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_EDGE_CHAMFER_OPEN,
		end,

	epm_weld_vertex_threshold, _T("weldVertexThreshold"), TYPE_WORLD, P_ANIMATABLE, IDS_WELD_THRESHOLD,
		p_default, .1f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_VERTWELD_THRESH, IDC_VERTWELD_THRESH_SPIN, SPIN_AUTOSCALE,
		end,

	epm_weld_edge_threshold, _T("weldEdgeThreshold"), TYPE_WORLD, P_ANIMATABLE, IDS_EDGE_WELD_THRESHOLD,
		p_default, .1f,
		p_range, 0.0f, 9999999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
			IDC_EDGEWELD_THRESH, IDC_EDGEWELD_THRESH_SPIN, SPIN_AUTOSCALE,
		end,

	epm_tess_type, _T("tessellateByFace"), TYPE_INT, P_ANIMATABLE, IDS_TESS_BY,
		p_default, 0,
		p_ui, ep_settings, TYPE_RADIO, 2, IDC_TESS_EDGE, IDC_TESS_FACE,
		end,

	epm_tess_tension, _T("tessTension"), TYPE_FLOAT, P_ANIMATABLE, IDS_TESS_TENSION,
		p_default, 0.0f,
		p_range, -9999999.9f, 9999999.9f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_TESS_TENSION, IDC_TESS_TENSIONSPIN, .01f,
		end,

	epm_connect_edge_segments, _T("connectEdgeSegments"), TYPE_INT, P_ANIMATABLE, IDS_CONNECT_EDGE_SEGMENTS,
		p_default, 1,
		p_range, 1, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_CONNECT_EDGE_SEGMENTS, IDC_CONNECT_EDGE_SEGMENTS_SPIN, .5f,
		end,

	epm_extrude_spline_node, _T("extrudeAlongSplineNode"), TYPE_INODE, 0, IDS_EXTRUDE_ALONG_SPLINE_NODE,
		end,

	epm_extrude_spline_segments, _T("extrudeAlongSplineSegments"), TYPE_INT, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_SEGMENTS,
		p_default, 6,
		p_range, 1, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_EXTRUDE_SEGMENTS, IDC_EXTRUDE_SEGMENTS_SPIN, .01f,
		end,

	epm_extrude_spline_taper, _T("extrudeAlongSplineTaper"), TYPE_FLOAT, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_TAPER,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TAPER, IDC_EXTRUDE_TAPER_SPIN, .01f,
		end,
	
	epm_extrude_spline_taper_curve, _T("extrudeAlongSplineTaperCurve"), TYPE_FLOAT, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_TAPER_CURVE,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TAPER_CURVE, IDC_EXTRUDE_TAPER_CURVE_SPIN, .01f,
		end,
	
	epm_extrude_spline_twist, _T("extrudeAlongSplineTwist"), TYPE_ANGLE, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_TWIST,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -3600.0f, 3600.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_TWIST, IDC_EXTRUDE_TWIST_SPIN, .5f,
		end,

	epm_extrude_spline_rotation, _T("extrudeAlongSplineRotation"), TYPE_ANGLE, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_ROTATION,
		p_default, 0.0f,
		p_dim, stdAngleDim,
		p_range, -360.f, 360.f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_EXTRUDE_ROTATION, IDC_EXTRUDE_ROTATION_SPIN, .5f,
		end,

	epm_extrude_spline_align, _T("extrudeAlongSplineAlign"), TYPE_BOOL, P_ANIMATABLE, IDS_EXTRUDE_ALONG_SPLINE_ALIGN,
		p_default, true,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_EXTRUDE_ALIGN_NORMAL,
		p_enable_ctrls, 1, epm_extrude_spline_rotation,
		end,

	epm_world_to_object_transform, _T("worldToObjectTransform"), TYPE_MATRIX3, 0, IDS_WORLD_TO_OBJECT_TRANSFORM,
		//p_default, &kIdentityMatrix,
		end,

	epm_hinge_angle, _T("hingeAngle"), TYPE_ANGLE, P_ANIMATABLE|P_RESET_DEFAULT, IDS_HINGE_FROM_EDGE_ANGLE,
		p_default, 0.0f,	// 30 degrees.
		p_dim, stdAngleDim,
		p_range, -360.0f, 360.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_HINGE_ANGLE, IDC_HINGE_ANGLE_SPIN, .5f,
		end,

	epm_hinge_base, _T("hingeBase"), TYPE_POINT3, 0, IDS_HINGE_FROM_EDGE_BASE,
		p_default, Point3(0,0,0),
		end,

	epm_hinge_dir, _T("hingeDirection"), TYPE_POINT3, 0, IDS_HINGE_FROM_EDGE_DIR,
		p_default, Point3(1,0,0),
		end,

	epm_hinge_segments, _T("hingeSegments"), TYPE_INT, P_ANIMATABLE, IDS_HINGE_SEGMENTS,
		p_default, 1,
		p_range, 1, 1000,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_HINGE_SEGMENTS, IDC_HINGE_SEGMENTS_SPIN, .5f,
		end,

	epm_align_type, _T("alignType"), TYPE_INT, P_RESET_DEFAULT, IDS_EDITPOLY_ALIGN_TYPE,
		p_default, 0,	// Group normal
		end,

	epm_align_plane_normal, _T("alignPlaneNormal"), TYPE_POINT3, P_ANIMATABLE, IDS_EDITPOLY_ALIGN_PLANE_NORMAL,
		p_default, Point3(0.0f, 0.0f, 0.0f),
		end,

	epm_align_plane_offset, _T("alignPlaneOffset"), TYPE_FLOAT, P_ANIMATABLE, IDS_EDITPOLY_ALIGN_PLANE_OFFSET,
		p_default, 0.0f,
		end,

	epm_autosmooth_threshold, _T("autoSmoothThreshold"), TYPE_ANGLE, P_ANIMATABLE, IDS_AUTOSMOOTH_THRESHOLD,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_face_smooth, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_AUTOSMOOTH_THRESHOLD, IDC_AUTOSMOOTH_THRESHOLD_SPIN, .5f,
		end,

		// These next two are DWORDs masquerading as INTs.
	epm_smooth_group_set, _T("smoothingGroupsToSet"), TYPE_INT, 0, IDS_SMOOTHING_GROUP_SET,
		p_default, 0,
		end,

	epm_smooth_group_clear, _T("smoothingGroupsToClear"), TYPE_INT, 0, IDS_SMOOTHING_GROUP_CLEAR,
		p_default, 0,
		end,

	epm_material, _T("materialIDToSet"), TYPE_INT, P_ANIMATABLE, IDS_MATERIAL_ID_SET, 
		p_default, 0,
		end,

	epm_material_selby, _T("selectByMaterialID"), TYPE_INT, 0, IDS_SELECT_BY_MATERIAL_ID,
		p_default, 0,
		end,

	epm_material_selby_clear, _T("selectByMaterialClear"), TYPE_BOOL, 0, IDS_SELECT_BY_MATERIAL_CLEAR,
		p_default, FALSE,
		p_ui, ep_surface, TYPE_SINGLECHEKBOX, IDC_CLEARSELECTION,
		end,

	epm_smoother_selby, _T("selectBySmoothingGroup"), TYPE_INT, 0, IDS_SELECT_BY_SMOOTHING_GROUP,
		p_default, 0,
		end,
		
	epm_smoother_selby_clear, _T("selectBySmoothingGroupClear"), TYPE_BOOL, 0, IDS_SELECT_BY_SMOOTHING_GROUP_CLEAR,
		p_default, 0,
		end,

	epm_bridge_segments, _T("bridgeSegments"), TYPE_INT, P_ANIMATABLE, IDS_BRIDGE_SEGMENTS,
		p_default, 1,
		p_range, 1, 999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_BRIDGE_SEGMENTS, IDC_BRIDGE_SEGMENTS_SPIN, .5f,
		end,

	epm_bridge_taper, _T("bridgeTaper"), TYPE_FLOAT, P_ANIMATABLE, IDS_BRIDGE_TAPER,
		p_default, 0.0f,
		p_range, -999.0f, 999.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BRIDGE_TAPER, IDC_BRIDGE_TAPER_SPIN, .02f,
		end,

	epm_bridge_bias, _T("bridgeBias"), TYPE_FLOAT, P_ANIMATABLE, IDS_BRIDGE_BIAS,
		p_default, 0.0f,
		p_range, -99.0f, 99.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_FLOAT,
			IDC_BRIDGE_BIAS, IDC_BRIDGE_BIAS_SPIN, 1.0f,
		end,

	epm_bridge_smooth_thresh, _T("bridgeSmoothThreshold"), TYPE_ANGLE, P_ANIMATABLE, IDS_BRIDGE_SMOOTH_THRESHOLD,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_BRIDGE_THRESH, IDC_BRIDGE_THRESH_SPIN, .5f,
		end,

	epm_bridge_selected, _T("bridgeSelected"), TYPE_INT, 0, IDS_BRIDGE_SELECTED,
		p_default, true,
		p_range, 0, 1,
		p_ui, ep_settings, TYPE_RADIO, 2, IDC_BRIDGE_SPECIFIC, IDC_BRIDGE_SELECTED,
		end,

	epm_bridge_twist_1, _T("bridgeTwist1"), TYPE_INT, 0, IDS_BRIDGE_TWIST_1,
		p_default, 0,
		p_range, -9999999, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_BRIDGE_TWIST1, IDC_BRIDGE_TWIST1_SPIN, .5f,
		end,

	epm_bridge_twist_2, _T("bridgeTwist2"), TYPE_INT, 0, IDS_BRIDGE_TWIST_2,
		p_default, 0,
		p_range, -9999999, 9999999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
			IDC_BRIDGE_TWIST2, IDC_BRIDGE_TWIST2_SPIN, .5f,
		end,

	epm_bridge_target_1, _T("bridgeTarget1"), TYPE_INT, 0, IDS_BRIDGE_TARGET_1,
		p_default, 0,
		end,

	epm_bridge_target_2, _T("bridgeTarget2"), TYPE_INT, 0, IDS_BRIDGE_TARGET_2,
		p_default, 0,
		end,

	epm_relax_amount, _T("relaxAmount"), TYPE_FLOAT, P_ANIMATABLE, IDS_RELAX_AMOUNT,
		p_default, 0.5f,
		p_range, -1.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
			IDC_RELAX_AMOUNT, IDC_RELAX_AMOUNT_SPIN, .01f,
		end,

	epm_relax_iters, _T("relaxIterations"), TYPE_INT, P_ANIMATABLE, IDS_RELAX_ITERATIONS,
		p_default, 1,
		p_range, 1, 99999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_INT,
			IDC_RELAX_ITERATIONS, IDC_RELAX_ITERATIONS_SPIN, .5f,
		end,

	epm_relax_hold_boundary, _T("relaxHoldBoundaryPoints"), TYPE_BOOL, 0, IDS_RELAX_HOLD_BOUNDARY_POINTS,
		p_default, true,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_RELAX_HOLD_BOUNDARY,
		end,

	epm_relax_hold_outer, _T("relaxHoldOuterPoints"), TYPE_BOOL, 0, IDS_RELAX_HOLD_OUTER_POINTS,
		p_default, false,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_RELAX_HOLD_OUTER,
		end,

	epm_bridge_adjacent, _T("bridgeEdgeAdjacent"), TYPE_ANGLE, P_ANIMATABLE, IDS_BRIDGE_ADJACENT,
		p_default, PI/4.0f,	// 45 degrees.
		p_range, 0.0f, 180.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
		IDC_BRIDGE_EDGE_ANGLE, IDC_BRIDGE_EDGE_ANGLE_SPIN, .5f,
		end,

	epm_bridge_reverse_triangle, _T("bridgeReverseTriangle"), TYPE_INT, P_TRANSIENT, IDS_BRIDGE_REVERSE_TRI,
		p_default, false,
		p_ui, ep_settings, TYPE_SINGLECHEKBOX, IDC_REVERSETRI,
		end,

	epm_connect_edge_pinch, _T("connectEdgePinch"), TYPE_INT, P_ANIMATABLE, IDS_CONNECT_EDGE_PINCH,
		p_default, 0,
		p_range, -100, 100,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
		IDC_CONNECT_EDGE_PINCH, IDC_CONNECT_EDGE_PINCH_SPIN, .5f,
		end,

	epm_connect_edge_slide, _T("connectEdgeSlide"), TYPE_INT, P_ANIMATABLE, IDS_CONNECT_EDGE_SLIDE,
		p_default, 0,
		p_range, -99999, 99999,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_INT,
		IDC_CONNECT_EDGE_SLIDE, IDC_CONNECT_EDGE_SLIDE_SPIN, .5f,
		end,

	epm_break_vertex_distance, _T("breakDistance"), TYPE_FLOAT, P_ANIMATABLE, IDS_RELAX_AMOUNT,
		p_default, 0.0f,
		p_range, -1.0f, 1.0f,
		p_ui, ep_settings, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
		IDC_BREAK_VERTEX, IDC_BREAK_VERTEX_SPIN, .01f,
		end,

	epm_remove_edge_ctrl, _T("remveEdgeCtrlKey"), TYPE_BOOL, 0, IDS_REMOVE_EDGE_CTRL_KEY,
		p_default, 0,
		end,


	epm_cage_color, _T("CageColor"), TYPE_POINT3, P_TRANSIENT,IDS_CAGE_COLOR,
		p_default, 		GetUIColor(COLOR_GIZMOS), 	
		p_ui, ep_animate, TYPE_COLORSWATCH, IDC_CAGE_COLOR_SWATCH,
		end,

	epm_selected_cage_color, _T("selectedCageColor"), TYPE_POINT3, P_TRANSIENT,IDS_SELECTED_CAGE_COLOR,
		p_default, 		GetUIColor(COLOR_SEL_GIZMOS), 
		p_ui, ep_animate, TYPE_COLORSWATCH, IDC_SELECTED_CAGE_COLOR_SWATCH,
		end,
	epm_select_mode, _T("selectMode"),TYPE_INT,0, IDS_SELECT_MODE,
		p_default, 0, 
		p_ui, ep_select,TYPE_RADIO,3,
		IDC_SELECT_NONE,IDC_SELECT_SUBOBJ,IDC_SELECT_MULTI,
		p_accessor,		&theEditPolyModPBAccessor,
		end,

	end
);

TCHAR *LookupParameterName (int paramID) {
	int index = edit_mod_param_blk.IDtoIndex (paramID);
	if (index<0) return NULL;
	return edit_mod_param_blk.paramdefs[index].int_name;
}

int LookupParameterType (int paramID) {
	int index = edit_mod_param_blk.IDtoIndex (paramID);
	if (index<0) return TYPE_INT;
	return edit_mod_param_blk.paramdefs[index].type;
}

//--- Our UI handlers ---------------------------------
class CurrentOperationProc : public ParamMap2UserDlgProc {
	EditPolyMod *mpEditPolyMod;

public:
	CurrentOperationProc () : mpEditPolyMod(NULL) { }
	void SetEditPolyMod (EditPolyMod*e) { mpEditPolyMod = e; }
	INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
		UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
	void UpdateCurrentOperation (HWND hWnd, TimeValue t);
	void SetEnables (HWND hWnd, TimeValue t);
};

static CurrentOperationProc theCurrentOperationProc;

void CurrentOperationProc::UpdateCurrentOperation (HWND hWnd, TimeValue t) {
	if (!mpEditPolyMod) return;
	if (!mpEditPolyMod->getParamBlock()) return;

	PolyOperation *pOp = mpEditPolyMod->GetPolyOperation();
	if (pOp) {
		SetDlgItemText (hWnd, IDC_EDITPOLY_CURRENT_OP_NAME, pOp->Name());
	} else {
		SetDlgItemText (hWnd, IDC_EDITPOLY_CURRENT_OP_NAME, GetString (IDS_EDITPOLY_NO_CURRENT_OP));
	}
}

void CurrentOperationProc::SetEnables (HWND hWnd, TimeValue t) {
	if (!mpEditPolyMod) return;
	IParamBlock2 *pblock = mpEditPolyMod->getParamBlock ();
	if (pblock==NULL) return;
	PolyOperation *pop = mpEditPolyMod->GetPolyOperation ();
	bool null = (pop==NULL) || (pop->OpID()==ep_op_null);
	bool hasPopup = null ? false : (pop->DialogID()>0);

	ICustButton *pBut = GetICustButton (GetDlgItem (hWnd, IDC_EDITPOLY_COMMIT));
	if (pBut)
	{
		pBut->Enable (!null);
		ReleaseICustButton (pBut);
	}
	pBut = GetICustButton (GetDlgItem (hWnd, IDC_EDITPOLY_SHOW_SETTINGS));
	if (pBut) {
		pBut->Enable (hasPopup);
		ReleaseICustButton (pBut);
	}
	pBut = GetICustButton (GetDlgItem (hWnd, IDC_EDITPOLY_CANCEL));
	if (pBut)
	{
		pBut->Enable (!null);
		ReleaseICustButton (pBut);
	}
}

INT_PTR CurrentOperationProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) 
{
	ICustButton		*pBut = NULL;
	IColorSwatch	*iCol = NULL;

	switch (msg) {
	case WM_INITDIALOG:	
		UpdateCurrentOperation (hWnd, t);

		pBut = GetICustButton (GetDlgItem (hWnd, IDC_EDITPOLY_SHOW_SETTINGS));
		if (pBut) {
			pBut->SetType (CBT_CHECK);
			pBut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(pBut);
		}

		if ( mpEditPolyMod )
		{
			// init the color swatches 

			Point3 l_cageCol			= mpEditPolyMod->getParamBlock()->GetPoint3(epm_cage_color,TimeValue(0) );
			Point3 l_selectedCageCol	= mpEditPolyMod->getParamBlock()->GetPoint3(epm_selected_cage_color,TimeValue(0) );

			mpEditPolyMod->m_iColorCageSwatch			= GetIColorSwatch(GetDlgItem(hWnd, IDC_CAGE_COLOR_SWATCH));
			mpEditPolyMod->m_iColorSelectedCageSwatch	= GetIColorSwatch(GetDlgItem(hWnd, IDC_SELECTED_CAGE_COLOR_SWATCH));

			mpEditPolyMod->m_iColorCageSwatch->SetColor(l_cageCol, FALSE );
			mpEditPolyMod->m_iColorSelectedCageSwatch->SetColor(l_selectedCageCol, FALSE);

			mpEditPolyMod->m_iColorCageSwatch->SetModal();
			mpEditPolyMod->m_iColorSelectedCageSwatch->SetModal();

			SetEnables (hWnd, t);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDITPOLY_COMMIT:
			theHold.Begin ();
			mpEditPolyMod->EpModCommit (t);
			theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
			break;

		case IDC_EDITPOLY_SHOW_SETTINGS:
			if (mpEditPolyMod->EpModShowingOperationDialog()) mpEditPolyMod->EpModCloseOperationDialog ();
			else mpEditPolyMod->EpModShowOperationDialog ();
			break;

		case IDC_EDITPOLY_CANCEL:
			theHold.Begin ();
			mpEditPolyMod->EpModCancel ();
			theHold.Accept (GetString (IDS_EDITPOLY_CANCEL));
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

class EditPolySelectProc : public ParamMap2UserDlgProc {
	EditPolyMod *mpMod;

public:
	EditPolySelectProc () : mpMod(NULL) { }
	void RefreshSelType (HWND hWnd);
	void SetEditPolyMod (EditPolyMod*e) { mpMod = e; }
	void SetEnables (HWND hWnd);
	void UpdateSelLevelDisplay (HWND hWnd);
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
};

static EditPolySelectProc theEditPolySelectProc;

static int butIDs[] = { 0, IDC_SELVERTEX, IDC_SELEDGE, IDC_SELBORDER, IDC_SELFACE, IDC_SELELEMENT };

void EditPolySelectProc::RefreshSelType (HWND hWnd) {
	ICustToolbar *iToolbar = GetICustToolbar (GetDlgItem (hWnd, IDC_SELTYPE));
	if (iToolbar) {
		ICustButton* but = NULL;
		for (int i=1; i<6; i++) {
			but = iToolbar->GetICustButton (butIDs[i]);
			if (!but) continue;
			but->SetCheck (mpMod->GetEPolySelLevel()==i);
			ReleaseICustButton (but);
		}
	}
	ReleaseICustToolbar(iToolbar);
}

void EditPolySelectProc::UpdateSelLevelDisplay (HWND hWnd) {
	if (!mpMod) return;
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_SELTYPE));
	ICustButton* but = NULL;
	for (int i=1; i<6; i++) {
		but = iToolbar->GetICustButton (butIDs[i]);
		but->SetCheck ((DWORD)i==mpMod->GetEPolySelLevel());
		ReleaseICustButton (but);
	}
	ReleaseICustToolbar (iToolbar);
	UpdateWindow(hWnd);
}

void EditPolySelectProc::SetEnables (HWND hParams) {
	if (!mpMod) return;
	int selLevel = mpMod->GetEPolySelLevel();
	bool soLevel = selLevel ? true : false;
	int stackSelect = mpMod->getParamBlock()->GetInt (epm_stack_selection, TimeValue(0)) ? true : false;
	BOOL multiSelect = mpMod->getParamBlock()->GetInt (epm_select_mode, TimeValue(0)) == 2;
	BOOL highlightSelect = mpMod->getParamBlock()->GetInt (epm_select_mode, TimeValue(0)) == 1;
	EnableWindow (GetDlgItem (hParams, IDC_EDITPOLY_STACK_SELECTION), soLevel && ! multiSelect && ! highlightSelect);
	EnableWindow (GetDlgItem (hParams, IDC_SEL_BYVERT), !stackSelect && ((selLevel>EPM_SL_VERTEX)?true:false) && ! multiSelect && ! highlightSelect);
	EnableWindow (GetDlgItem (hParams, IDC_IGNORE_BACKFACES), !stackSelect && soLevel);
	
	EnableWindow (GetDlgItem (hParams, IDC_SELECT_SUBOBJ), !stackSelect && soLevel);
	EnableWindow (GetDlgItem (hParams, IDC_SELECT_MULTI), !stackSelect && soLevel);
	ICustButton *pBut = GetICustButton (GetDlgItem (hParams, IDC_EDITPOLY_GET_STACK_SELECTION));
	if (pBut != NULL) {
		pBut->Enable (!stackSelect && soLevel);
		ReleaseICustButton (pBut);
	}

	pBut = GetICustButton (GetDlgItem (hParams, IDC_SELECTION_GROW));
	if (pBut != NULL) {
		pBut->Enable (!stackSelect && soLevel);
		ReleaseICustButton (pBut);
	}

	pBut = GetICustButton (GetDlgItem (hParams, IDC_SELECTION_SHRINK));
	if (pBut != NULL) {
		pBut->Enable (!stackSelect && soLevel);
		ReleaseICustButton (pBut);
	}

	pBut = GetICustButton (GetDlgItem (hParams, IDC_EDGE_RING_SEL));
	if (pBut != NULL) {
		pBut->Enable (!stackSelect && (selLevel == EPM_SL_EDGE));
		ReleaseICustButton (pBut);
	}

	pBut = GetICustButton (GetDlgItem (hParams, IDC_EDGE_LOOP_SEL));
	if (pBut != NULL) {
		pBut->Enable (!stackSelect && (selLevel == EPM_SL_EDGE));
		ReleaseICustButton (pBut);
	}

	bool facNotElem = (mpMod->GetEPolySelLevel() == EPM_SL_FACE);
	if (mpMod->getParamBlock()->GetInt (epm_by_vertex)) facNotElem = false;
	EnableWindow (GetDlgItem (hParams, IDC_SELECT_BY_ANGLE), facNotElem && !stackSelect);
	ISpinnerControl *spinner = GetISpinner (GetDlgItem (hParams, IDC_SELECT_ANGLE_SPIN));
	if (spinner) {
		bool useAngle = mpMod->getParamBlock()->GetInt (epm_select_by_angle) != 0;
		spinner->Enable (facNotElem && useAngle && !stackSelect);
		ReleaseISpinner (spinner);
	}

	ISpinnerControl *l_spin = SetupIntSpinner (hParams, IDC_EDGE_RING_SEL_SPIN, IDC_EDGE_RING_SEL_FOR_SPIN, -9999999, 9999999, 0);
	if (l_spin) {
		l_spin->SetAutoScale(FALSE);
		ReleaseISpinner (l_spin);
	}

	l_spin = SetupIntSpinner (hParams, IDC_EDGE_LOOP_SEL_SPIN, IDC_EDGE_LOOP_SEL_FOR_SPIN, -9999999, 9999999, 0);
	if (l_spin) {
		l_spin->SetAutoScale(FALSE);
		ReleaseISpinner (l_spin);
	}

}

INT_PTR EditPolySelectProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!mpMod) 
		return FALSE;

	ICustToolbar	*iToolbar	= NULL;
	ISpinnerControl *l_spin		= NULL;

	switch (msg) {
	case WM_INITDIALOG:
		iToolbar = GetICustToolbar(GetDlgItem(hWnd,IDC_SELTYPE));
		iToolbar->SetImage (GetPolySelImageHandler()->LoadImages());
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0,5,0,5,24,23,24,23,IDC_SELVERTEX));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,1,6,1,6,24,23,24,23,IDC_SELEDGE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2,7,2,7,24,23,24,23,IDC_SELBORDER));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,3,8,3,8,24,23,24,23,IDC_SELFACE));
		iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4,9,4,9,24,23,24,23,IDC_SELELEMENT));
		ReleaseICustToolbar(iToolbar);

		UpdateSelLevelDisplay (hWnd);
		SetEnables (hWnd);
		break;

	case WM_UPDATE_CACHE:
		mpMod->UpdateCache((TimeValue)wParam);
 		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELVERTEX:
			if (mpMod->GetEPolySelLevel() == EPM_SL_VERTEX)
				mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					mpMod->EpModConvertSelection (EPM_SL_CURRENT, EPM_SL_VERTEX, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if (GetKeyState (VK_SHIFT)<0)
				{
					// New with 8.0 - check for shift key, use it as a selection converter : 
					// convert it into border vertices  
					theHold.Begin ();
					mpMod->EpModConvertSelectionToBorder (EPM_SL_CURRENT, EPM_SL_VERTEX);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
			}
				mpMod->SetEPolySelLevel (EPM_SL_VERTEX);
			}
			break;

		case IDC_SELEDGE:
			if (mpMod->GetEPolySelLevel() == EPM_SL_EDGE)
				mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					mpMod->EpModConvertSelection (EPM_SL_CURRENT, EPM_SL_EDGE, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if (GetKeyState (VK_SHIFT)<0)
				{
					// New with 8.0 - check for shift key, use it as a selection converter : 
					// convert it into border edges 
					theHold.Begin ();
					mpMod->EpModConvertSelectionToBorder (EPM_SL_CURRENT, EPM_SL_EDGE);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
				}
				mpMod->SetEPolySelLevel (EPM_SL_EDGE);
			}
			break;

		case IDC_SELBORDER:
			if (mpMod->GetEPolySelLevel() == EPM_SL_BORDER)
				mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					mpMod->EpModConvertSelection (EPM_SL_CURRENT, EPM_SL_BORDER, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				mpMod->SetEPolySelLevel (EPM_SL_BORDER);
			}
			break;

		case IDC_SELFACE:
			if (mpMod->GetEPolySelLevel() == EPM_SL_FACE)
				mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					mpMod->EpModConvertSelection (EPM_SL_CURRENT, EPM_SL_FACE, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				else if (GetKeyState (VK_SHIFT)<0)
				{
					// New with 8.0 - check for shift key, use it as a selection converter : 
					// convert it into border faces 
					theHold.Begin ();
					mpMod->EpModConvertSelectionToBorder(EPM_SL_CURRENT, EPM_SL_FACE);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION_TO_BORDER));
				}
				mpMod->SetEPolySelLevel (EPM_SL_FACE);
			}
			break;

		case IDC_SELELEMENT:
			if (mpMod->GetEPolySelLevel() == EPM_SL_ELEMENT)
				mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
			else {
				// New with 5.0 - check for control key, use it as a selection converter.
				if (GetKeyState (VK_CONTROL)<0) {
					bool requireAll = (GetKeyState (VK_SHIFT)<0) ? true : false;
					theHold.Begin ();
					mpMod->EpModConvertSelection (EPM_SL_CURRENT, EPM_SL_ELEMENT, requireAll);
					theHold.Accept (GetString (IDS_CONVERT_SELECTION));
				}
				mpMod->SetEPolySelLevel (EPM_SL_ELEMENT);
			}
			break;

		case IDC_SELECTION_GROW:
			mpMod->EpModButtonOp (ep_op_sel_grow);
			break;

		case IDC_SELECTION_SHRINK:
			mpMod->EpModButtonOp (ep_op_sel_shrink);
			break;

		case IDC_EDGE_RING_SEL:
			mpMod->EpModButtonOp (ep_op_select_ring);
			break;

		case IDC_EDGE_LOOP_SEL:
			mpMod->EpModButtonOp (ep_op_select_loop);
			break;

		case IDC_EDITPOLY_GET_STACK_SELECTION:
			if (mpMod->GetEPolySelLevel() == EPM_SL_OBJECT) break;
			mpMod->EpModButtonOp (ep_op_get_stack_selection);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code != TTN_NEEDTEXT) break;
		LPTOOLTIPTEXT lpttt;
		lpttt = (LPTOOLTIPTEXT)lParam;				
		switch (lpttt->hdr.idFrom) {
		case IDC_SELVERTEX:
			lpttt->lpszText = GetString (IDS_EDITPOLY_VERTEX);
			break;
		case IDC_SELEDGE:
			lpttt->lpszText = GetString (IDS_EDITPOLY_EDGE);
			break;
		case IDC_SELBORDER:
			lpttt->lpszText = GetString(IDS_EDITPOLY_BORDER);
			break;
		case IDC_SELFACE:
			lpttt->lpszText = GetString(IDS_EDITPOLY_FACE);
			break;
		case IDC_SELELEMENT:
			lpttt->lpszText = GetString(IDS_EDITPOLY_ELEMENT);
			break;
		}
		break;

	case CC_SPINNER_BUTTONDOWN:
	{	
		switch (LOWORD(wParam))
		{
			case IDC_EDGE_RING_SEL_SPIN:
			case IDC_EDGE_LOOP_SEL_SPIN:
				{		
					bool l_ctrl = (GetKeyState (VK_CONTROL)< 0) ? true : false;
					bool l_alt  = (GetKeyState (VK_MENU) < 0)?  true : false;

					ModContextList	list;
					INodeTab		nodes;
					EditPolyData *l_polySelData = NULL ;

					mpMod->ip->GetModContexts(list,nodes);

					for (int m = 0; m < list.Count(); m++) 
					{
						l_polySelData = (EditPolyData*)list[m]->localData;

						if (!l_polySelData) 
							continue;

						theHold.Begin();  // (uses restore object in paramblock.)
						theHold.Put (new SelRingLoopRestore(mpMod,l_polySelData));

						if ( LOWORD(wParam) == IDC_EDGE_LOOP_SEL_SPIN)
							theHold.Accept(GetString(IDS_LOOP_EDGE_SEL));
						else 
							theHold.Accept(GetString(IDS_RING_EDGE_SEL));

					}

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

					mpMod->setAltKey(l_alt);
					mpMod->setCtrlKey(l_ctrl);
				}

			break;
		}
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
				l_spin->Execute(I_EXEC_SPINNER_ALT_ENABLE);
				l_spin->Execute(I_EXEC_SPINNER_ONE_CLICK_DISABLE);
			}
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) 
		{
			case IDC_EDGE_LOOP_SEL_SPIN:
				l_spin = (ISpinnerControl*)lParam;
				if (l_spin )
					mpMod->UpdateLoopEdgeSelection(l_spin->GetIVal());
				break;
			case IDC_EDGE_RING_SEL_SPIN:
				l_spin = (ISpinnerControl*)lParam;
				if (l_spin )
					mpMod->UpdateRingEdgeSelection(l_spin->GetIVal());
				break;
		}
		break;

	default: return FALSE;
	}
	return TRUE;
}

#define GRAPHSTEPS 20

static EditPolySoftselDlgProc theSoftselDlgProc;
EditPolySoftselDlgProc *TheSoftselDlgProc () { return &theSoftselDlgProc; }

void EditPolySoftselDlgProc::DrawCurve (TimeValue t, HWND hWnd,HDC hdc) {
	float pinch, falloff, bubble;
	IParamBlock2 *pblock = mpEPoly->getParamBlock ();
	if (!pblock) return;

	pblock->GetValue (epm_ss_falloff, t, falloff, FOREVER);
	pblock->GetValue (epm_ss_pinch, t, pinch, FOREVER);
	pblock->GetValue (epm_ss_bubble, t, bubble, FOREVER);

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

	WhiteRect3D(hdc,orect,TRUE);
}

void EditPolySoftselDlgProc::SetEnables (HWND hSS, TimeValue t) {
	IParamBlock2 *pblock = mpEPoly->getParamBlock ();
	int softSel, useEdgeDist, useStackSelection;
	softSel = pblock->GetInt (epm_ss_use, t);
	useEdgeDist = pblock->GetInt (epm_ss_edist_use, t);
	useStackSelection = pblock->GetInt (epm_stack_selection, t);

	ISpinnerControl* spin = NULL;
	EnableWindow (GetDlgItem (hSS, IDC_USE_SOFTSEL), mpEPoly->GetEPolySelLevel());// && !useStackSelection);
	bool enable = (mpEPoly->GetEPolySelLevel() && softSel /*&& !useStackSelection*/) ? true : false;

	EditPolyMod *pEditPoly = (EditPolyMod*)mpEPoly;
	BOOL isPainting = (pEditPoly->GetPaintMode()==PAINTMODE_PAINTSEL? TRUE:FALSE);
	BOOL isBlurring = (pEditPoly->GetPaintMode()==PAINTMODE_BLURSEL? TRUE:FALSE);
	BOOL isErasing  = (pEditPoly->GetPaintMode()==PAINTMODE_ERASESEL? TRUE:FALSE);

	EnableWindow (GetDlgItem (hSS, IDC_LOCK_SOFTSEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_BORDER), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_VALUE_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_SIZE_LABEL), enable);
	EnableWindow (GetDlgItem (hSS, IDC_PAINTSEL_STRENGTH_LABEL), enable);
	ICustButton *but = GetICustButton (GetDlgItem (hSS, IDC_PAINTSEL_PAINT));
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
	if (mpEPoly->getParamBlock()->GetInt(epm_ss_lock)) enable = false;

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

	but = GetICustButton (GetDlgItem (hSS, IDC_SHADED_FACE_TOGGLE));
	but->Enable (mpEPoly->GetEPolySelLevel() && softSel);
	ReleaseICustButton (but);
}

INT_PTR EditPolySoftselDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	Rect rect;
	PAINTSTRUCT ps;
	HDC hdc;
	EditPolyMod *pMod = (EditPolyMod *) mpEPoly;
	ICustButton* btn = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_PAINT));
		btn->SetType(CBT_CHECK);
		ReleaseICustButton(btn);

		btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_BLUR));
		btn->SetType(CBT_CHECK);
		ReleaseICustButton(btn);

		btn = GetICustButton(GetDlgItem(hWnd,IDC_PAINTSEL_REVERT));
		btn->SetType(CBT_CHECK);
		ReleaseICustButton(btn);

		SetEnables (hWnd, t);
		break;
		
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&ps);
		DrawCurve (t, hWnd, hdc);
		EndPaint(hWnd,&ps);
		return FALSE;

	case CC_SPINNER_BUTTONDOWN:
		break;

	case CC_SPINNER_BUTTONUP:
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam)) {
		case IDC_FALLOFFSPIN:
		case IDC_PINCHSPIN:
		case IDC_BUBBLESPIN:
			GetClientRectP(GetDlgItem(hWnd,IDC_SS_GRAPH),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			//mpEPoly->InvalidateSoftSelectionCache();	// handled by NotifyRefChanged.
			break;
		case IDC_EDIST_SPIN:
			//mpEPoly->InvalidateDistanceCache ();	// handled by NotifyRefChanged.
			break;
		case IDC_PAINTSEL_VALUE_SPIN:
		case IDC_PAINTSEL_SIZE_SPIN:
		case IDC_PAINTSEL_STRENGTH_SPIN:
			pMod->UpdatePaintBrush();
			break;
		}
		map->RedrawViews (t, REDRAW_INTERACTIVE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USE_SOFTSEL:
		case IDC_USE_EDIST:
			SetEnables (hWnd, t);
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
			mpEPoly->EpModButtonOp (ep_op_toggle_shaded_faces);
			break;

		case IDC_PAINTSEL_PAINT:
			pMod->OnPaintBtn( PAINTMODE_PAINTSEL, t );
			SetEnables (hWnd, t);
			break;

		case IDC_PAINTSEL_BLUR:
			pMod->OnPaintBtn( PAINTMODE_BLURSEL, t );
			SetEnables (hWnd, t);
			break;

		case IDC_PAINTSEL_REVERT:
			pMod->OnPaintBtn( PAINTMODE_ERASESEL, t );
			SetEnables (hWnd, t);
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


void EditPolyMod::UpdateCageCheckboxEnable () {
	HWND hWnd = GetDlgHandle (ep_animate);
	if (hWnd != NULL) {
		EnableWindow (GetDlgItem (hWnd, IDC_SHOW_CAGE), ShowGizmoConditions());
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
	EPolyMod *mpMod;
public:
	SelectionModeOverride(EPolyMod *m,int selection):mActiveOverride(FALSE),mpMod(m),mCurrentSelection(selection){}
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

	mpMod->getParamBlock()->GetValue (epm_select_mode, TimeValue(0), mLastSelection, FOREVER);
	if(mLastSelection==mCurrentSelection && mCurrentSelection>0)
		mCurrentSelection = 0; //make it go to none if we are in subobj or highlight

	mpMod->getParamBlock()->SetValue (epm_select_mode, TimeValue(0), mCurrentSelection);
	return TRUE;
}

BOOL SelectionModeOverride ::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;

	mpMod->getParamBlock()->SetValue (epm_select_mode, TimeValue(0), mLastSelection);
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
	EditPolyMod *mpMod;
public:
	PaintOverride(EditPolyMod *m,int c):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mpMod(m),mPaint(c){}
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

	mPrevPaintMode = mpMod->GetPaintMode();
	mOldCommandMode = mpMod->EpModGetCommandMode();

	mpMod->OnPaintBtn( mPaint, GetCOREInterface()->GetTime() );
	return TRUE;
}

BOOL PaintOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	if(mPrevPaintMode!= PAINTMODE_OFF)
		mpMod->OnPaintBtn( mPrevPaintMode, GetCOREInterface()->GetTime());
	else if(mOldCommandMode!=-1)
		mpMod->EpModToggleCommandMode(mOldCommandMode);
	else
		mpMod->OnPaintBtn( mPaint, GetCOREInterface()->GetTime());

	return TRUE;
}

		


//constraint overrides
class ConstraintOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	int mOverrideConstraint;
	int mPrevConstraint;
	EPolyMod *mpMod;
public:
	ConstraintOverride(EPolyMod *m,int c):mActiveOverride(FALSE),mpMod(m),mOverrideConstraint(c){}
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

	mpMod->getParamBlock()->GetValue (epm_constrain_type, TimeValue(0), mPrevConstraint, FOREVER);
	if(mPrevConstraint != mOverrideConstraint) //if they are the same then enter 0 else go to ti
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), mOverrideConstraint);
	else
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), 0);

	return TRUE;
}

BOOL ConstraintOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), mPrevConstraint);
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

	EditPolyMod *mpMod;
public:
	SubObjectModeOverride(EditPolyMod *m,int id):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mSubObjectMode(id),mOldSubObjectMode(-1),mpMod(m){}
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

	mOldCommandMode = mpMod->EpModGetCommandMode();
	mOldSubObjectMode = mpMod->GetEPolySelLevel();
	mPrevPaintMode = mpMod->GetPaintMode();
	mpMod->SetEPolySelLevel(mSubObjectMode);
	return TRUE;
}

BOOL SubObjectModeOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	mpMod->SetEPolySelLevel(mOldSubObjectMode);
	if(mOldCommandMode!=-1)
		mpMod->EpModToggleCommandMode(mOldCommandMode);
	if(mPrevPaintMode != PAINTMODE_OFF)
		mpMod->OnPaintBtn(mPrevPaintMode,GetCOREInterface()->GetTime());
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
	EditPolyMod *mpMod;
public:
	CommandModeOverride(EditPolyMod *m,int id):mPrevPaintMode(PAINTMODE_OFF),mActiveOverride(FALSE),mCommandMode(id),mOldCommandMode(-1),mpMod(m){}
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
	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	mpMod->EpModToggleCommandMode(mCommandMode);
	return TRUE;
}

BOOL CommandModeOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	if(mOldCommandMode!=-1)
		mpMod->EpModToggleCommandMode(mOldCommandMode);
	else
		mpMod->EpModToggleCommandMode(mCommandMode);
	if(mPrevPaintMode != PAINTMODE_OFF)
		mpMod->OnPaintBtn(mPrevPaintMode,GetCOREInterface()->GetTime());
	return TRUE;
}

//only does the override if in face mode
class FaceCommandModeOverride: public CommandModeOverride
{
public:
	FaceCommandModeOverride(EditPolyMod *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
	
};

BOOL FaceCommandModeOverride::StartOverride()
{
	if (mpMod->GetEPolySelLevel() != EPM_SL_FACE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}



//only does the override if in less than eged mode
class LessEdgeCommandModeOverride: public CommandModeOverride
{
public:
	LessEdgeCommandModeOverride(EditPolyMod *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
};

BOOL LessEdgeCommandModeOverride::StartOverride()
{
	if (mpMod->GetEPolySelLevel() > EPM_SL_EDGE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}


//only does the override if in less than eged mode
class GreaterEdgeCommandModeOverride: public CommandModeOverride
{
public:
	GreaterEdgeCommandModeOverride(EditPolyMod *m,int id):CommandModeOverride(m,id){}
	BOOL StartOverride();
};

BOOL GreaterEdgeCommandModeOverride::StartOverride()
{
	if (mpMod->GetEPolySelLevel() < EPM_SL_EDGE)
		return FALSE;
	else
		return CommandModeOverride::StartOverride();
}


class ChamferModeOverride: public CommandModeOverride
{

public:
	ChamferModeOverride(EditPolyMod *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL ChamferModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	switch (mpMod->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpMod->EpModToggleCommandMode (ep_mode_chamfer_vertex);
			mCommandMode = ep_mode_chamfer_vertex;
			return TRUE;
		case MNM_SL_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_chamfer_edge);
			mCommandMode = ep_mode_chamfer_edge;
			return TRUE;
		}

	return FALSE;
}


class CreateModeOverride: public CommandModeOverride
{

public:
	CreateModeOverride(EditPolyMod *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL CreateModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;
	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	switch (mpMod->GetMNSelLevel()) {
	case MNM_SL_VERTEX:
		mpMod->EpModToggleCommandMode (ep_mode_create_vertex);
		mCommandMode= ep_mode_create_vertex;
		return TRUE;
	case MNM_SL_EDGE:
		mpMod->EpModToggleCommandMode (ep_mode_create_edge);
		mCommandMode= ep_mode_create_edge;
		return TRUE;
	default:
		mpMod->EpModToggleCommandMode (ep_mode_create_face);
		mCommandMode = ep_mode_create_face;
		return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}

class InsertModeOverride: public CommandModeOverride 
{
public:
	InsertModeOverride(EditPolyMod *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};

BOOL InsertModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	switch (mpMod->GetMNSelLevel()) {
	case MNM_SL_EDGE:
		mpMod->EpModToggleCommandMode (ep_mode_divide_edge);
		mCommandMode = ep_mode_divide_edge;
		return TRUE;
	case MNM_SL_FACE:
		mpMod->EpModToggleCommandMode (ep_mode_divide_face);
		mCommandMode = ep_mode_divide_face;
		return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}
		
class BridgeModeOverride: public CommandModeOverride
{
public:
	BridgeModeOverride(EditPolyMod *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();

};
BOOL BridgeModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_BORDER:
			mpMod->EpModToggleCommandMode (ep_mode_bridge_border);
			mCommandMode = ep_mode_bridge_border;
			return TRUE;
		case EPM_SL_FACE:
			mpMod->EpModToggleCommandMode (ep_mode_bridge_polygon);
			mCommandMode = ep_mode_bridge_polygon;
			return TRUE;
		case EPM_SL_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_bridge_edge);
			mCommandMode = ep_mode_bridge_edge;
			return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;	
}

class ExtrudeModeOverride: public CommandModeOverride 
{
public:
	ExtrudeModeOverride(EditPolyMod *m):CommandModeOverride(m,-1){}
	BOOL StartOverride();
};

BOOL ExtrudeModeOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	mActiveOverride = TRUE;

	mOldCommandMode = mpMod->EpModGetCommandMode();
	mPrevPaintMode = mpMod->GetPaintMode();
	switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_VERTEX:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_vertex);
			mCommandMode= ep_mode_extrude_vertex;
			return TRUE;
		case EPM_SL_EDGE:
		case EPM_SL_BORDER:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_edge);
			mCommandMode= ep_mode_extrude_edge;
			return TRUE;
		case EPM_SL_FACE:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_face);
			mCommandMode = ep_mode_extrude_face;
			return TRUE;
	}
	mActiveOverride = FALSE;
	return FALSE;
}




//--- EditPolyMod methods -------------------------------

// Geometry subobject dialogs:
int editPolyGeomDlgs[] = { 0, IDD_EDITPOLY_GEOM_VERTEX, IDD_EDITPOLY_GEOM_EDGE, IDD_EDITPOLY_GEOM_BORDER,
	IDD_EDITPOLY_GEOM_FACE, IDD_EDITPOLY_GEOM_ELEMENT };
int editPolyGeomDlgNames[] = { 0, IDS_EDIT_VERTICES, IDS_EDIT_EDGES, IDS_EDIT_BORDERS,
	IDS_EDIT_FACES, IDS_EDIT_ELEMENTS };

void EditPolyMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;
	mpCurrentEditMod = this;

	// Set all our dialog procs to ourselves as modifier:
	theCurrentOperationProc.SetEditPolyMod (this);
	theEditPolySelectProc.SetEditPolyMod (this);
	TheOperationDlgProc()->SetEditPolyMod (this);
	theSoftselDlgProc.SetEPoly (this);
	TheGeomDlgProc()->SetEPoly (this);
	TheSubobjDlgProc()->SetEPoly (this);
	TheSurfaceDlgProc()->SetEPoly (this);
	TheFaceSmoothDlgProc()->SetEPoly (this);
	ThePaintDeformDlgProc()->SetEPoly (this);

	// Put up our dialogs:
	mpDialogAnimate = CreateCPParamMap2 (ep_animate, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_ANIMATE), GetString (IDS_EDITPOLY_MODE),
		mRollupAnimate ? APPENDROLL_CLOSED : 0, &theCurrentOperationProc);
	mpDialogSelect = CreateCPParamMap2 (ep_select, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_SELECT), GetString (IDS_EDITPOLY_SELECTION),
		mRollupSelect ? APPENDROLL_CLOSED : 0, &theEditPolySelectProc);
	mpDialogSoftsel = CreateCPParamMap2 (ep_softsel, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_SOFTSEL), GetString (IDS_SOFTSEL),
		mRollupSoftsel ? APPENDROLL_CLOSED : 0, &theSoftselDlgProc);

	if (selLevel != EPM_SL_OBJECT) {
		mpDialogSubobj = CreateCPParamMap2 (ep_subobj, mpParams, ip, hInstance,
			MAKEINTRESOURCE (editPolyGeomDlgs[selLevel]), GetString (editPolyGeomDlgNames[selLevel]),
			mRollupSubobj ? APPENDROLL_CLOSED : 0, TheSubobjDlgProc());
	} else mpDialogSubobj = NULL;

	mpDialogGeom = CreateCPParamMap2 (ep_geom, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_GEOM), GetString (IDS_EDIT_GEOM),
		mRollupGeom ? APPENDROLL_CLOSED : 0, TheGeomDlgProc());

	if (meshSelLevel[selLevel] == MNM_SL_FACE) {
		mpDialogSurface = CreateCPParamMap2 (ep_surface, mpParams, ip, hInstance,
			MAKEINTRESOURCE (IDD_EDITPOLY_SURF_FACE), GetString (IDS_EP_POLY_MATERIALIDS),
			mRollupSurface ? APPENDROLL_CLOSED : 0, TheSurfaceDlgProc());
		mpDialogFaceSmooth = CreateCPParamMap2 (ep_face_smooth, mpParams, ip, hInstance,
			MAKEINTRESOURCE (IDD_EDITPOLY_POLY_SMOOTHING), GetString (IDS_EDITPOLY_POLY_SMOOTHING),
			mRollupFaceSmooth ? APPENDROLL_CLOSED : 0, TheFaceSmoothDlgProc());

	} else {mpDialogSurface = NULL;mpDialogFaceSmooth=NULL;}

	mpDialogPaintDeform = CreateCPParamMap2 (ep_paintdeform, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_PAINTDEFORM), GetString (IDS_PAINTDEFORM),
		mRollupPaintDeform ? APPENDROLL_CLOSED : 0, ThePaintDeformDlgProc());

	selectMode	= new SelectModBoxCMode(this,ip);
	moveMode    = new MoveModBoxCMode(this,ip);
	rotMode     = new RotateModBoxCMode(this,ip);
	uscaleMode  = new UScaleModBoxCMode(this,ip);
	nuscaleMode = new NUScaleModBoxCMode(this,ip);
	squashMode  = new SquashModBoxCMode(this,ip);

	// Add our command modes:
	createVertMode = new EditPolyCreateVertCMode (this, ip);
	createEdgeMode = new EditPolyCreateEdgeCMode (this, ip);
	createFaceMode = new EditPolyCreateFaceCMode(this, ip);
	divideEdgeMode = new EditPolyDivideEdgeCMode(this, ip);
	divideFaceMode = new EditPolyDivideFaceCMode (this, ip);
	bridgeBorderMode = new EditPolyBridgeBorderCMode (this, ip);
	bridgePolyMode = new EditPolyBridgePolygonCMode (this, ip);
	bridgeEdgeMode = new EditPolyBridgeEdgeCMode (this, ip);
	extrudeMode = new EditPolyExtrudeCMode (this, ip);
	chamferMode = new EditPolyChamferCMode (this, ip);
	bevelMode = new EditPolyBevelCMode(this, ip);
	insetMode = new EditPolyInsetCMode (this, ip);
	outlineMode = new EditPolyOutlineCMode (this, ip);
	extrudeVEMode = new EditPolyExtrudeVECMode (this, ip);
	cutMode = new EditPolyCutCMode (this, ip);
	quickSliceMode = new EditPolyQuickSliceCMode (this, ip);
	weldMode = new EditPolyWeldCMode (this, ip);
	hingeFromEdgeMode = new EditPolyHingeFromEdgeCMode (this, ip);
	pickHingeMode = new EditPolyPickHingeCMode (this, ip);
	pickBridgeTarget1 = new EditPolyPickBridge1CMode (this, ip);
	pickBridgeTarget2 = new EditPolyPickBridge2CMode (this, ip);
	editTriMode = new EditPolyEditTriCMode (this, ip);
	turnEdgeMode = new EditPolyTurnEdgeCMode (this, ip);

	// And our pick modes:
	if (!attachPickMode) attachPickMode = new EditPolyAttachPickMode;
	if (attachPickMode) attachPickMode->SetPolyObject (this, ip);
	if (!mpShapePicker) mpShapePicker = new EditPolyShapePickMode;
	if (mpShapePicker) mpShapePicker->SetMod (this, ip);

	// Restore the selection level.
	ip->SetSubObjectLevel(selLevel);

	// Add our accelerator table (keyboard shortcuts)
	mpEPolyActions = new EditPolyActionCB (this);
	ip->GetActionManager()->ActivateActionTable (mpEPolyActions, kEditPolyActionID);

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
		SubObjectModeOverride *subObjectModeOverride = new SubObjectModeOverride(this,EPM_SL_OBJECT);
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
		CommandModeOverride *commandModeOverride = new CommandModeOverride(this,ep_mode_cut);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_EDIT_QUICKSLICE);
		mOverrides.Append(1,&item);
		commandModeOverride = new CommandModeOverride(this,ep_mode_quickslice);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);
		
	/*	item = pTable->GetAction(ID_EP_EDIT_SLICEPLANE);
		mOverrides.Append(1,&item);
		commandModeOverride = new CommandModeOverride(this,ep_mode_sliceplane);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);
*/
		//face command mdoes
		item = pTable->GetAction(ID_EP_SUB_BEVEL);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,ep_mode_bevel);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_OUTLINE);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,ep_mode_outline);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_INSET);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,ep_mode_inset_face);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		item = pTable->GetAction(ID_EP_SUB_HINGE);
		mOverrides.Append(1,&item);
		commandModeOverride = new FaceCommandModeOverride(this,ep_mode_hinge_from_edge);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//less than edge 
		item = pTable->GetAction(ID_EP_SUB_WELD);
		mOverrides.Append(1,&item);
		commandModeOverride = new LessEdgeCommandModeOverride(this,ep_mode_weld);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		//greater edge
		item = pTable->GetAction(ID_EP_TURN);
		mOverrides.Append(1,&item);
		commandModeOverride = new GreaterEdgeCommandModeOverride(this,ep_mode_turn_edge);
		actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

		
		item = pTable->GetAction(ID_EP_SURF_EDITTRI);
		mOverrides.Append(1,&item);
		commandModeOverride = new GreaterEdgeCommandModeOverride(this,ep_mode_edit_tri);
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


	// We want del key, backspace input if in geometry level
	if (selLevel != EPM_SL_OBJECT) {
		ip->RegisterDeleteUser(this);
		backspacer.SetEPoly(this);
		backspaceRouter.Register(&backspacer);
	}

	// Update the UI to match the selection level
	UpdateCageCheckboxEnable ();
	SetEnableStates ();
	UpdateSelLevelDisplay ();
	SetNumSelLabel();
	InvalidateNumberHighlighted (TRUE);

	MeshPaintHandler::BeginEditParams();

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

//	EpModCommit(t, false, true, false);  this was added to fix defect 833277.  It causes issues with edit poly modifier when animating so I removed so it can be fixed later.  PKW
}

void EditPolyMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {

	MeshPaintHandler::EndEditParams();

	// Unregister del key, backspace notification
	if (selLevel != EPM_SL_OBJECT) {
		ip->UnRegisterDeleteUser(this);
		backspacer.SetEPoly (NULL);
		backspaceRouter.UnRegister (&backspacer);
	}

	if (mpDialogAnimate) {
		mRollupAnimate = IsRollupPanelOpen (mpDialogAnimate->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogAnimate);
		mpDialogAnimate = NULL;
	}
	if (mpDialogSelect) {
		mRollupSelect = IsRollupPanelOpen (mpDialogSelect->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSelect);
		mpDialogSelect = NULL;
	}
	if (mpDialogSoftsel)
	{
		mRollupSoftsel = IsRollupPanelOpen (mpDialogSoftsel->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSoftsel);
		mpDialogSoftsel = NULL;
	}
	if (mpDialogGeom)
	{
		mRollupGeom = IsRollupPanelOpen (mpDialogGeom->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogGeom);
		mpDialogGeom = NULL;
	}
	if (mpDialogSubobj)
	{
		mRollupSubobj = IsRollupPanelOpen (mpDialogSubobj->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSubobj);
		mpDialogSubobj = NULL;
	}
	if (mpDialogSurface)
	{
		mRollupSurface = IsRollupPanelOpen (mpDialogSurface->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSurface);
		mpDialogSurface = NULL;
	}
	if (mpDialogFaceSmooth)
	{
		mRollupFaceSmooth = IsRollupPanelOpen (mpDialogFaceSmooth->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogFaceSmooth);
		mpDialogFaceSmooth = NULL;
	}
	if (mpDialogOperation) {
		DestroyModelessParamMap2 (mpDialogOperation);
		mpDialogOperation = NULL;
	}
	if (mpDialogPaintDeform) {
		mRollupPaintDeform = IsRollupPanelOpen (mpDialogPaintDeform->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogPaintDeform);
		mpDialogPaintDeform = NULL;
	}

	theCurrentOperationProc.SetEditPolyMod (NULL);
	theEditPolySelectProc.SetEditPolyMod (NULL);
	TheOperationDlgProc()->SetEditPolyMod (NULL);
	theSoftselDlgProc.SetEPoly (NULL);
	TheGeomDlgProc()->SetEPoly (NULL);
	TheSubobjDlgProc()->SetEPoly (NULL);
	TheSurfaceDlgProc()->SetEPoly (NULL);
	TheFaceSmoothDlgProc()->SetEPoly (NULL);


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
		ip->GetActionManager()->DeactivateActionTable (mpEPolyActions, kEditPolyActionID);
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
	if (bridgeBorderMode) delete bridgeBorderMode;
	bridgeBorderMode = NULL;
	if (bridgeEdgeMode) delete bridgeEdgeMode;
	bridgeEdgeMode = NULL;
	if (bridgePolyMode) delete bridgePolyMode;
	bridgePolyMode = NULL;
	if (extrudeMode) delete extrudeMode;
	extrudeMode = NULL;
	if (chamferMode) delete chamferMode;
	chamferMode = NULL;
	if (bevelMode) delete bevelMode;
	bevelMode = NULL;
	if (insetMode) delete insetMode;
	insetMode = NULL;
	if (outlineMode) delete outlineMode;
	outlineMode = NULL;
	if (extrudeVEMode) delete extrudeVEMode;
	extrudeVEMode = NULL;
	if (cutMode) delete cutMode;
	cutMode = NULL;
	if (quickSliceMode) delete quickSliceMode;
	quickSliceMode = NULL;
	if (weldMode) delete weldMode;
	weldMode = NULL;
	if (hingeFromEdgeMode) delete hingeFromEdgeMode;
	hingeFromEdgeMode = NULL;
	if (pickHingeMode) delete pickHingeMode;
	pickHingeMode = NULL;
	if (pickBridgeTarget1) delete pickBridgeTarget1;
	pickBridgeTarget1 = NULL;
	if (pickBridgeTarget2) delete pickBridgeTarget2;
	pickBridgeTarget2 = NULL;
	if (editTriMode) delete editTriMode;
	editTriMode = NULL;
	if (turnEdgeMode) delete turnEdgeMode;
	turnEdgeMode = NULL;

	if (attachPickMode) attachPickMode->ClearPolyObject ();
	if (mpShapePicker) mpShapePicker->ClearMod ();

	this->ip = NULL;
	mpCurrentEditMod = NULL;

	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

void EditPolyMod::UpdateSelLevelDisplay () {
	if (mpCurrentEditMod != this) return;
	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	theEditPolySelectProc.UpdateSelLevelDisplay (hWnd);
}

void EditPolyMod::SetEnableStates() {
	if (mpCurrentEditMod != this) return;

	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	theEditPolySelectProc.SetEnables (hWnd);
}

void EditPolyMod::SetNumSelLabel () {
	if (mpCurrentEditMod != this) return;
	TSTR buf;
	int num = 0, which = 0;
	if (!ip) return;

	if (!mpDialogSelect) return;
	HWND hWnd = mpDialogSelect->GetHWnd();
	if (!hWnd) return;

	//better way would be to create a new GetEpolysellevel function.. this is a hack for mz prototype
	//so this can change depending upon final implementation...

	int tempSelLevel = selLevel;
	if(0&&GetSelectMode()==PS_MULTI_SEL)
	{
		if(mSelLevelOverride>0)
		{
	   
			switch(mSelLevelOverride)
			{
			case MNM_SL_VERTEX:
				tempSelLevel = EPM_SL_VERTEX;
				break;
			case MNM_SL_EDGE:
				tempSelLevel = EPM_SL_EDGE;
				break;
			case MNM_SL_FACE:
				tempSelLevel = EPM_SL_FACE;
				break;
			};

		}
	}

	if (tempSelLevel > EPM_SL_OBJECT) {
		bool useStack = mpParams->GetInt (epm_stack_selection) != 0;

		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		nodes.DisposeTemporary();

		for (int i = 0; i < mcList.Count(); i++) {
			EditPolyData *pData = (EditPolyData *)mcList[i]->localData;
			if (!pData) continue;

			if (useStack) {
				MNMesh *pMesh = pData->GetMesh();
				if (!pMesh) continue;

				int j;
				switch (tempSelLevel) {
				case EPM_SL_VERTEX:
					for (j=0; j<pMesh->numv; j++) {
						if (!pMesh->v[j].GetFlag(MN_EDITPOLY_STACK_SELECT)) continue;
						num++;
						which=j;
					}
					break;
				case EPM_SL_EDGE:
				case EPM_SL_BORDER:
					for (j=0; j<pMesh->nume; j++) {
						if (!pMesh->e[j].GetFlag(MN_EDITPOLY_STACK_SELECT)) continue;
						num++;
						which=j;
					}
					break;
				case EPM_SL_FACE:
				case EPM_SL_ELEMENT:
					for (j=0; j<pMesh->numf; j++) {
						if (!pMesh->f[j].GetFlag(MN_EDITPOLY_STACK_SELECT)) continue;
						num++;
						which=j;
					}
					break;
				}
			} else {
				switch (tempSelLevel) {
				case EPM_SL_VERTEX:
					num += pData->mVertSel.NumberSet();
					if (pData->mVertSel.NumberSet() == 1) {
						for (which=0; which<pData->mVertSel.GetSize(); which++) if (pData->mVertSel[which]) break;
					}
					break;
				case EPM_SL_EDGE:
				case EPM_SL_BORDER:
					num += pData->mEdgeSel.NumberSet();
					if (pData->mEdgeSel.NumberSet() == 1) {
						for (which=0; which<pData->mEdgeSel.GetSize(); which++) if (pData->mEdgeSel[which]) break;
					}
					break;
				case EPM_SL_FACE:
				case EPM_SL_ELEMENT:
					num += pData->mFaceSel.NumberSet();
					if (pData->mFaceSel.NumberSet() == 1) {
						for (which=0; which<pData->mFaceSel.GetSize(); which++) if (pData->mFaceSel[which]) break;
					}
					break;
				}
			}
		}
	}

	switch (tempSelLevel) {
	case EPM_SL_VERTEX:
		if (num==1) buf.printf (GetString(IDS_EDITPOLY_WHICHVERTSEL), which+1);
		else buf.printf(GetString(IDS_EDITPOLY_NUMVERTSEL),num);
		break;

	case EPM_SL_EDGE:
	case EPM_SL_BORDER:
		if (num==1) buf.printf (GetString(IDS_EDITPOLY_WHICHEDGESEL), which+1);
		else buf.printf(GetString(IDS_EDITPOLY_NUMEDGESEL),num);
		break;

	case EPM_SL_FACE:
	case EPM_SL_ELEMENT:
		if (num==1) buf.printf (GetString(IDS_EDITPOLY_WHICHFACESEL), which+1);
		else buf.printf(GetString(IDS_EDITPOLY_NUMFACESEL),num);
		break;

	case EPM_SL_OBJECT:
		buf = GetString (IDS_OBJECT_SEL);
		break;
	}

	SetDlgItemText(hWnd,IDC_NUMSEL_LABEL,buf);
}

void EditPolyMod::UpdateOpChoice ()
{
	if (!mpDialogAnimate) return;
	HWND hWnd = mpDialogAnimate->GetHWnd();
	theCurrentOperationProc.UpdateCurrentOperation (hWnd, ip->GetTime());
}

void EditPolyMod::UpdateEnables (int paramPanelID) {
	if (!ip) return;
	HWND hWnd = GetDlgHandle (paramPanelID);
	switch (paramPanelID)
	{
	case ep_select:
		theEditPolySelectProc.SetEnables (hWnd);
		break;
	case ep_animate:
		theCurrentOperationProc.SetEnables (hWnd, ip->GetTime());
		break;
	case ep_geom:
		TheGeomDlgProc()->SetEnables (hWnd);
		break;
	case ep_softsel:
		theSoftselDlgProc.SetEnables (hWnd, ip->GetTime());
		break;
	case ep_subobj:
		TheSubobjDlgProc()->SetEnables (hWnd);
		break;
	}
}

void EditPolyMod::UpdateUIToSelectionLevel (int oldSelLevel) {
	if (selLevel == oldSelLevel) return;
	if (!ip) return;

	HWND hWnd = GetDlgHandle (ep_select);

	if (hWnd != NULL) {
		theEditPolySelectProc.RefreshSelType (hWnd);
		theEditPolySelectProc.SetEnables (hWnd);
	}

	hWnd = GetDlgHandle (ep_softsel);
	if (hWnd != NULL) {
		//end paint session before invalidating UI
		if ( IsPaintSelMode(GetPaintMode()) ) EndPaintMode();
		theSoftselDlgProc.SetEnables (hWnd, ip->GetTime());
	}
	hWnd = GetDlgHandle (ep_geom);
	if (hWnd != NULL) {
		TheGeomDlgProc()->SetEnables (hWnd);
	}

	hWnd = GetDlgHandle (ep_paintdeform);
	if (hWnd != NULL) {
		//end paint session before invalidating UI
		if ( IsPaintDeformMode(GetPaintMode()) ) EndPaintMode();
		ThePaintDeformDlgProc()->SetEnables (hWnd);
	}

	int msl = meshSelLevel[selLevel];

	// Otherwise, we need to get rid of all remaining rollups and replace them.
	HWND hFocus = GetFocus ();

	// Destroy old versions:
	if (mpDialogSurface) {
		mRollupSurface = IsRollupPanelOpen (mpDialogSurface->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSurface);
		mpDialogSurface = NULL;
	}
	if (mpDialogFaceSmooth) {
		mRollupFaceSmooth = IsRollupPanelOpen (mpDialogFaceSmooth->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogFaceSmooth);
		mpDialogFaceSmooth = NULL;
	}
	if (mpDialogGeom) {
		mRollupGeom = IsRollupPanelOpen (mpDialogGeom->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogGeom);
		mpDialogGeom = NULL;
	}
	if (mpDialogSubobj) {
		mRollupSubobj = IsRollupPanelOpen (mpDialogSubobj->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogSubobj);
		mpDialogSubobj = NULL;
	}
	if (mpDialogPaintDeform) {
		mRollupPaintDeform = IsRollupPanelOpen (mpDialogPaintDeform->GetHWnd()) ? false : true;
		DestroyCPParamMap2 (mpDialogPaintDeform);
		mpDialogPaintDeform = NULL;
	}

	// Create new versions:
	if (msl != MNM_SL_OBJECT) {
		mpDialogSubobj = CreateCPParamMap2 (ep_subobj, mpParams, ip, hInstance,
			MAKEINTRESOURCE (editPolyGeomDlgs[selLevel]), GetString (editPolyGeomDlgNames[selLevel]),
			mRollupSubobj ? APPENDROLL_CLOSED : 0, TheSubobjDlgProc());
	}

	mpDialogGeom = CreateCPParamMap2 (ep_geom, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_GEOM), GetString (IDS_EDIT_GEOM),
		mRollupGeom ? APPENDROLL_CLOSED : 0, TheGeomDlgProc());

	if (msl == MNM_SL_FACE) {
		mpDialogSurface = CreateCPParamMap2 (ep_surface, mpParams, ip, hInstance,
			MAKEINTRESOURCE (IDD_EDITPOLY_SURF_FACE), GetString (IDS_EP_POLY_MATERIALIDS),
			mRollupSurface ? APPENDROLL_CLOSED : 0, TheSurfaceDlgProc());
		mpDialogFaceSmooth = CreateCPParamMap2 (ep_face_smooth, mpParams, ip, hInstance,
			MAKEINTRESOURCE (IDD_EDITPOLY_POLY_SMOOTHING), GetString (IDS_EDITPOLY_POLY_SMOOTHING),
			mRollupFaceSmooth ? APPENDROLL_CLOSED : 0,TheFaceSmoothDlgProc());
	}

	mpDialogPaintDeform = CreateCPParamMap2 (ep_paintdeform, mpParams, ip, hInstance,
		MAKEINTRESOURCE (IDD_EDITPOLY_PAINTDEFORM), GetString (IDS_PAINTDEFORM),
		mRollupPaintDeform ? APPENDROLL_CLOSED : 0, ThePaintDeformDlgProc());

	KillRefmsg kill(killRefmsg);
	SetFocus (hFocus);
}

void EditPolyMod::InvalidateNumberHighlighted (BOOL zeroOut)
{
	HWND hWnd = GetDlgHandle (ep_select);
	if (!hWnd) return;
	if (mpCurrentEditMod != this) return;
	TSTR buf;
	int num = 0;
	if (!ip) return;

	if (!mpDialogSelect) return;

	HWND item = GetDlgItem(hWnd,IDC_NUMHIGHLIGHTED_LABEL);
	InvalidateRect (item, NULL, FALSE);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	nodes.DisposeTemporary();

	int tempSelLevelOverride = mSelLevelOverride;
	if(GetSelectMode()==PS_SUBOBJ_SEL)
	{
		switch(selLevel)
		{

			case EPM_SL_VERTEX:
				tempSelLevelOverride = MNM_SL_VERTEX;
					break;

			case EPM_SL_EDGE:
			case EPM_SL_BORDER:
				tempSelLevelOverride = MNM_SL_EDGE;
				break;

			case EPM_SL_FACE:
			case EPM_SL_ELEMENT:
				tempSelLevelOverride = MNM_SL_FACE;
				break;

		}
	}


	for (int i = 0; i < mcList.Count(); i++) {
		EditPolyData *pData = (EditPolyData *)mcList[i]->localData;
		if (!pData) continue;

		if(zeroOut)
			pData->mHighlightedItems.clear();
		
	
		switch(tempSelLevelOverride)
		{
			case MNM_SL_VERTEX:
				if(pData->mHighlightedItems.size()==1)
					buf.printf (GetString(IDS_EDITPOLY_WHICHVERTHIGHLIGHTED), *(pData->mHighlightedItems.begin())+1);
				else buf.printf(GetString(IDS_EDITPOLY_NUMVERTHIGHLIGHTED),pData->mHighlightedItems.size());
				break;
			case MNM_SL_EDGE:
				if(pData->mHighlightedItems.size()==1)
					buf.printf (GetString(IDS_EDITPOLY_WHICHEDGEHIGHLIGHTED), *(pData->mHighlightedItems.begin())+1);
				else buf.printf(GetString(IDS_EDITPOLY_NUMEDGEHIGHLIGHTED),pData->mHighlightedItems.size());
				break;
			case MNM_SL_FACE:
				if(pData->mHighlightedItems.size()==1)
					buf.printf (GetString(IDS_EDITPOLY_WHICHFACEHIGHLIGHTED),*(pData->mHighlightedItems.begin())+1);
				else buf.printf(GetString(IDS_EDITPOLY_NUMFACEHIGHLIGHTED),pData->mHighlightedItems.size());
				break;
			default:
		
				buf = TSTR("");
				break;
		}
	}

	SetDlgItemText(hWnd,IDC_NUMHIGHLIGHTED_LABEL,buf);
}

void EditPolyMod::InvalidateNumberSelected () {
	HWND hWnd = GetDlgHandle (ep_select);
	if (!hWnd) return;
	InvalidateRect (hWnd, NULL, FALSE);
	SetNumSelLabel ();
}


void EditPolyMod::ExitAllCommandModes (bool exSlice, bool exStandardModes) {
	if (createVertMode) ip->DeleteMode (createVertMode);
	if (createEdgeMode) ip->DeleteMode (createEdgeMode);
	if (createFaceMode) ip->DeleteMode (createFaceMode);
	if (divideEdgeMode) ip->DeleteMode (divideEdgeMode);
	if (divideFaceMode) ip->DeleteMode (divideFaceMode);
	if (bridgeBorderMode) ip->DeleteMode (bridgeBorderMode);
	if (bridgeEdgeMode) ip->DeleteMode (bridgeEdgeMode);
	if (bridgePolyMode) ip->DeleteMode (bridgePolyMode);
	if (extrudeMode) ip->DeleteMode (extrudeMode);
	if (extrudeVEMode) ip->DeleteMode (extrudeVEMode);
	if (chamferMode) ip->DeleteMode (chamferMode);
	if (bevelMode) ip->DeleteMode (bevelMode);
	if (insetMode) ip->DeleteMode (insetMode);
	if (outlineMode) ip->DeleteMode (outlineMode);
	if (extrudeVEMode) ip->DeleteMode (extrudeVEMode);
	if (cutMode) ip->DeleteMode (cutMode);
	if (quickSliceMode) ip->DeleteMode (quickSliceMode);
	if (weldMode) ip->DeleteMode (weldMode);
	if (hingeFromEdgeMode) ip->DeleteMode (hingeFromEdgeMode);
	if (pickHingeMode) ip->DeleteMode (pickHingeMode);
	if (pickBridgeTarget1) ip->DeleteMode (pickBridgeTarget1);
	if (pickBridgeTarget2) ip->DeleteMode (pickBridgeTarget2);
	if (editTriMode) ip->DeleteMode (editTriMode);
	if (turnEdgeMode) ip->DeleteMode (turnEdgeMode);

	if( InPaintMode() ) EndPaintMode();

	if (exStandardModes) {
		// remove our SO versions of standardmodes:
		if (moveMode) ip->DeleteMode (moveMode);
		if (rotMode) ip->DeleteMode (rotMode);
		if (uscaleMode) ip->DeleteMode (uscaleMode);
		if (nuscaleMode) ip->DeleteMode (nuscaleMode);
		if (squashMode) ip->DeleteMode (squashMode);
		if (selectMode) ip->DeleteMode (selectMode);
	}

	if (mSliceMode && exSlice) ExitSliceMode ();
	ip->ClearPickMode();
}

// Luna task 748Y - UI redesign
void EditPolyMod::InvalidateDialogElement (int elem) {
	// No convenient way to get parammap pointer from element id,
	// so we invalidate this element in all parammaps.  (Harmless - does
	// nothing if elem is not present in a given parammap.)
	if (mpDialogGeom) mpDialogGeom->Invalidate (elem);
	if (mpDialogSubobj) mpDialogSubobj->Invalidate(elem);
	if (mpDialogSurface)
	{
		mpDialogSurface->Invalidate (elem);
		switch (elem) {
			case epm_material:
				InvalidateSurfaceUI();
				break;
		}
	}
	if (mpDialogFaceSmooth)
	{
		mpDialogFaceSmooth->Invalidate (elem);
		switch (elem) {
			case epm_smooth_group_clear:
			case epm_smooth_group_set:
				InvalidateFaceSmoothUI();
				break;
		}
	}
	if (mpDialogSelect) mpDialogSelect->Invalidate (elem);
	if (mpDialogSoftsel) {
		mpDialogSoftsel->Invalidate (elem);
		if ((elem == epm_stack_selection) || (elem == epm_ss_use) || (elem == epm_ss_lock))
			theSoftselDlgProc.SetEnables (mpDialogSoftsel->GetHWnd(), ip ? ip->GetTime() : TimeValue(0));
		if ((elem == epm_ss_falloff) || (elem == epm_ss_pinch) || (elem == epm_ss_bubble))
		{
			Rect rect;
			GetClientRectP(GetDlgItem(mpDialogSoftsel->GetHWnd(),IDC_SS_GRAPH),&rect);
			InvalidateRect(mpDialogSoftsel->GetHWnd(),&rect,FALSE);
		}
	}
	if (mpDialogOperation) {
		mpDialogOperation->Invalidate (elem);
		TheOperationDlgProc()->UpdateUI (mpDialogOperation->GetHWnd(), ip->GetTime(), elem);
	}

	if (mpDialogPaintDeform) mpDialogPaintDeform->Invalidate (elem);

	if ((elem == epm_constrain_type) && mpDialogGeom) {
		TheConstraintUIHandler()->Update(mpDialogGeom->GetHWnd(), this);
	}
}

HWND EditPolyMod::GetDlgHandle (int dlgID) {
	if (!ip) return NULL;
	if (mpCurrentEditMod != this) return NULL;
	if (dlgID<0) return NULL;
	IParamMap2 *pmap=NULL;
	switch (dlgID) {
	case ep_animate:
		pmap = mpDialogAnimate;
		break;
	case ep_select:
		pmap = mpDialogSelect;
		break;
	case ep_softsel:
		pmap = mpDialogSoftsel;
		break;
	case ep_geom:
		pmap = mpDialogGeom;
		break;
	case ep_subobj:
		pmap = mpDialogSubobj;
		break;
	case ep_surface:
		pmap = mpDialogSurface;
		break;
	case ep_settings:
		pmap = mpDialogOperation;
		break;
	case ep_paintdeform:
		pmap = mpDialogPaintDeform;
		break;
	}
	if (!pmap) return NULL;
	return pmap->GetHWnd();
}

// -- Misc. Window procs ----------------------------------------

static INT_PTR CALLBACK ShapeNameDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static EditPolyMod *pMod;
	TSTR name;

	switch (msg) {
	case WM_INITDIALOG:
		ThePopupPosition()->Set (hWnd);
		pMod = (EditPolyMod *)lParam;
		SetWindowText (GetDlgItem(hWnd,IDC_SHAPE_NAME), pMod->GetShapeName().data());
		SendMessage (GetDlgItem (hWnd,IDC_SHAPE_NAME), EM_SETSEL,0,-1);
		CheckRadioButton (hWnd, IDC_SHAPE_SMOOTH, IDC_SHAPE_LINEAR, pMod->GetShapeType());
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name.Resize(GetWindowTextLength(GetDlgItem(hWnd,IDC_SHAPE_NAME))+1);
			GetWindowText(GetDlgItem(hWnd,IDC_SHAPE_NAME), name.data(), name.length()+1);
			pMod->SetShapeName (name);
			if (IsDlgButtonChecked(hWnd,IDC_SHAPE_SMOOTH)) pMod->SetShapeType (IDC_SHAPE_SMOOTH);
			else pMod->SetShapeType (IDC_SHAPE_LINEAR);
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,1);
			break;
		
		case IDCANCEL:
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

bool EditPolyMod::GetCreateShape () {
	HWND hMax = ip->GetMAXHWnd();
	mCreateShapeName = GetString(IDS_SHAPE);
	ip->MakeNameUnique (mCreateShapeName);
	return DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_CREATE_SHAPE),
		hMax, ShapeNameDlgProc, (LPARAM)this) ? true : false;
}

static void SetDetachNameState(HWND hWnd, int detachToElem) {
	if (detachToElem) {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),FALSE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAMELABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_DETACH_NAME),TRUE);
	}
}

static INT_PTR CALLBACK DetachDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TSTR name;
	static EditPolyMod *pMod;
	static int detachToElem, detachAsClone;

	switch (msg) {
	case WM_INITDIALOG:
		ThePopupPosition()->Set (hWnd);
		pMod = (EditPolyMod*)lParam;
		SetWindowText(GetDlgItem(hWnd,IDC_DETACH_NAME), pMod->GetDetachName().data());
		SendMessage(GetDlgItem(hWnd,IDC_DETACH_NAME), EM_SETSEL,0,-1);
		detachToElem = !pMod->GetDetachToObject();
		detachAsClone = pMod->GetDetachAsClone ();
		CheckDlgButton (hWnd, IDC_DETACH_TO_ELEMENT, detachToElem);
		CheckDlgButton (hWnd, IDC_DETACH_AS_CLONE, detachAsClone);
		if (detachToElem) SetFocus (GetDlgItem (hWnd, IDOK));
		else SetFocus (GetDlgItem (hWnd, IDC_DETACH_NAME));
		SetDetachNameState (hWnd, detachToElem);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name.Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_DETACH_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_DETACH_NAME),
				name.data(), name.length()+1);
			pMod->SetDetachName (name);
			if ((detachToElem == 0) != pMod->GetDetachToObject())
				pMod->SetDetachToObject (detachToElem == 0);
			if ((detachAsClone != 0) != pMod->GetDetachAsClone())
				pMod->SetDetachAsClone (detachAsClone != 0);
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,1);
			break;

		case IDC_DETACH_TO_ELEMENT:
			detachToElem = IsDlgButtonChecked(hWnd,IDC_DETACH_TO_ELEMENT);
			SetDetachNameState (hWnd, detachToElem);
			break;

		case IDC_DETACH_AS_CLONE:
			detachAsClone = IsDlgButtonChecked (hWnd, IDC_DETACH_AS_CLONE);
			break;

		case IDCANCEL:
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

bool EditPolyMod::GetDetachObject () {
	HWND hMax = ip->GetMAXHWnd();
	mDetachName = GetString(IDS_OBJECT);
	ip->MakeNameUnique (mDetachName);
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DETACH),
			hMax, DetachDlgProc, (LPARAM)this)) {
		return true;
	} else {
		return false;
	}
}

static void SetCloneNameState(HWND hWnd, int cloneTo) {
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
	static EditPolyMod *pMod = NULL;
	static int cloneTo;
	TSTR name;

	switch (msg) {
	case WM_INITDIALOG:
		ThePopupPosition()->Set (hWnd);
		pMod = (EditPolyMod *)lParam;
		cloneTo = pMod->GetCloneTo();
		SetWindowText(GetDlgItem(hWnd,IDC_CLONE_NAME), pMod->GetDetachName().data());

		CheckRadioButton (hWnd, IDC_CLONE_OBJ, IDC_CLONE_ELEM, cloneTo);
		if (cloneTo == IDC_CLONE_OBJ) {
			SetFocus(GetDlgItem(hWnd,IDC_CLONE_NAME));
			SendMessage(GetDlgItem(hWnd,IDC_CLONE_NAME), EM_SETSEL,0,-1);
		} else SetFocus (GetDlgItem (hWnd, IDOK));
		SetCloneNameState(hWnd, cloneTo);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name.Resize (GetWindowTextLength(GetDlgItem(hWnd,IDC_CLONE_NAME))+1);
			GetWindowText (GetDlgItem(hWnd,IDC_CLONE_NAME),
				name.data(), name.length()+1);
			pMod->SetDetachName (name);
			pMod->SetCloneTo (cloneTo);
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			ThePopupPosition()->Scan (hWnd);
			EndDialog (hWnd, 0);
			break;

		case IDC_CLONE_ELEM:
		case IDC_CLONE_OBJ:
			cloneTo = LOWORD(wParam);
			SetCloneNameState(hWnd, cloneTo);
			break;
		}
		break;

	default:
		return 0;
	}
	return 1;
}

bool EditPolyMod::GetDetachClonedObject () {
	HWND hMax = ip->GetMAXHWnd();
	mDetachName = GetString(IDS_OBJECT);
	ip->MakeNameUnique (mDetachName);
	DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_CLONE), hMax, CloneDlgProc, (LPARAM)this);
	return (mCloneTo==IDC_CLONE_OBJ);
}








