/**********************************************************************
 *<
	FILE: PolyAccel.cpp

	DESCRIPTION: Editable Polygon Mesh Object Accelerator table (keyboard shortcuts)

	CREATED BY: Steve Anderson

	HISTORY: created January 26, 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "EditPoly.h"


BOOL EditPolyActionCB::ExecuteAction (int id) {
	int checkboxValue;	// for toggles.
	int constrainType;
	int oldConstrainType;

	switch (id) {

	// Selection dialog shortcuts:

	case ID_EP_SEL_LEV_OBJ:
		mpMod->SetEPolySelLevel (EPM_SL_OBJECT);
		break;

	case ID_EP_SEL_LEV_VERTEX:
		mpMod->SetEPolySelLevel (EPM_SL_VERTEX);
		break;

	case ID_EP_SEL_LEV_EDGE:
		mpMod->SetEPolySelLevel (EPM_SL_EDGE);
		break;

	case ID_EP_SEL_LEV_BORDER:
		mpMod->SetEPolySelLevel (EPM_SL_BORDER);
		break;

	case ID_EP_SEL_LEV_FACE:
		mpMod->SetEPolySelLevel (EPM_SL_FACE);
		break;

	case ID_EP_SEL_LEV_ELEMENT:
		mpMod->SetEPolySelLevel (EPM_SL_ELEMENT);
		break;

	case ID_EP_SEL_BYVERT:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_by_vertex, TimeValue(0), checkboxValue, FOREVER);
		mpMod->getParamBlock()->SetValue (epm_by_vertex, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SEL_IGBACK:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_ignore_backfacing, TimeValue(0), checkboxValue, FOREVER);
		mpMod->getParamBlock()->SetValue (epm_ignore_backfacing, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_PREVIEW_SEL:
		mpMod->getParamBlock()->GetValue(epm_select_mode,TimeValue(0),checkboxValue,FOREVER);
		theHold.Begin();
		if(checkboxValue==PS_NONE_SEL)
		{
			mpMod->getParamBlock()->SetValue (epm_select_mode, TimeValue(0),PS_SUBOBJ_SEL);
		}
		else if(checkboxValue==PS_SUBOBJ_SEL)
		{
			mpMod->getParamBlock()->SetValue (epm_select_mode, TimeValue(0), PS_MULTI_SEL);
		}
		else if(checkboxValue==PS_MULTI_SEL)
		{
			mpMod->getParamBlock()->SetValue (epm_select_mode, TimeValue(0), PS_NONE_SEL);
		}
		theHold.Accept(GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SEL_GROW:
		mpMod->EpModButtonOp (ep_op_sel_grow);
		break;

	case ID_EP_SEL_SHRINK:
		mpMod->EpModButtonOp (ep_op_sel_shrink);
		break;

	case ID_EP_SEL_RING:
		if (mpMod->GetMNSelLevel() == MNM_SL_EDGE)
			mpMod->EpModButtonOp (ep_op_select_ring);
		break;

	case ID_EP_SEL_LOOP:
		if (mpMod->GetMNSelLevel() == MNM_SL_EDGE)
			mpMod->EpModButtonOp (ep_op_select_loop);
		break;

	case ID_EP_SEL_HIDE:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_hide_vertex); break;
		case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_hide_face); break;
		}
		break;

	case ID_EP_UNSEL_HIDE:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_hide_unsel_vertex); break;
		case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_hide_unsel_face); break;
		}
		break;

	case ID_EP_SEL_UNHIDE:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_unhide_vertex); break;
		case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_unhide_face); break;
		}
		break;

	// Soft Selection dialog shortcuts:

	case ID_EP_SS_ON:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_ss_use, TimeValue(0), checkboxValue, FOREVER);
		mpMod->getParamBlock()->SetValue (epm_ss_use, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SS_BACKFACE:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_ss_affect_back, TimeValue(0), checkboxValue, FOREVER);
		mpMod->getParamBlock()->SetValue (epm_ss_affect_back, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SS_SHADEDFACES:
		mpMod->EpModButtonOp (ep_op_toggle_shaded_faces);
		break;

	case ID_EP_SS_LOCK:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_ss_lock, TimeValue(0), checkboxValue, FOREVER);
		mpMod->getParamBlock()->SetValue (epm_ss_lock, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;
    //paint buttons...
	case ID_EP_SS_PAINT:
		theHold.Begin();
		mpMod->OnPaintBtn( PAINTMODE_PAINTSEL, GetCOREInterface()->GetTime() );
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;
	case ID_EP_SS_PAINTPUSHPULL:
		theHold.Begin();
		mpMod->OnPaintBtn( PAINTMODE_PAINTPUSHPULL,GetCOREInterface()->GetTime() );
	/*	removed this since the OnPaintBtn is more consistent MZ, leave in though in case defects pop up
		mpMod->getParamBlock()->GetValue(epm_paintdeform_mode,TimeValue(0),checkboxValue,FOREVER);
		if(checkboxValue==PAINTMODE_PAINTPUSHPULL)
			mpMod->getParamBlock()->SetValue(epm_paintdeform_mode,TimeValue(0),PAINTMODE_OFF);
		else
			mpMod->getParamBlock()->SetValue(epm_paintdeform_mode,TimeValue(0),PAINTMODE_PAINTPUSHPULL);
			*/
		theHold.Accept (GetString (IDS_PARAMETERS));

		break;
	case ID_EP_SS_PAINTRELAX:
		theHold.Begin();
		/*  removed this since the OnPaintBtn is more consistent MZ, leave in though in case defects pop up
		mpMod->getParamBlock()->GetValue(epm_paintdeform_mode,TimeValue(0),checkboxValue,FOREVER);
		if(checkboxValue==PAINTMODE_PAINTRELAX)
			mpMod->getParamBlock()->SetValue(epm_paintdeform_mode,TimeValue(0),PAINTMODE_OFF);
		else
			mpMod->getParamBlock()->SetValue(epm_paintdeform_mode,TimeValue(0),PAINTMODE_PAINTRELAX);
			*/
		mpMod->OnPaintBtn( PAINTMODE_PAINTRELAX,GetCOREInterface()->GetTime() );
		theHold.Accept (GetString (IDS_PARAMETERS));
	
		break;
	// Edit Geometry dialog shortcuts:

	case ID_EP_EDIT_CREATE:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpMod->EpModToggleCommandMode (ep_mode_create_vertex);
			break;
		case MNM_SL_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_create_edge);
			break;
		default:
			mpMod->EpModToggleCommandMode (ep_mode_create_face);
			break;
		}
		break;

	case ID_EP_EDIT_ATTACH:
		mpMod->EpModEnterPickMode (ep_mode_attach);
		break;

	case ID_EP_EDIT_ATTACHLIST:
		mpMod->EpModPopupDialog (ep_op_attach);
		break;

	case ID_EP_EDIT_DETACH:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpMod->EpModPopupDialog (ep_op_detach_vertex);
			break;
		case MNM_SL_EDGE:
		case MNM_SL_FACE:
			mpMod->EpModPopupDialog (ep_op_detach_face);
			break;
		}
		break;

	case ID_EP_EDIT_COLLAPSE:
		switch (mpMod->GetMNSelLevel ())
		{
		case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_collapse_vertex); break;
		case MNM_SL_EDGE: mpMod->EpModButtonOp (ep_op_collapse_edge); break;
		case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_collapse_face); break;
		}
		break;

	case ID_EP_EDIT_CUT:
		mpMod->EpModToggleCommandMode (ep_mode_cut);
		break;

	case ID_EP_EDIT_QUICKSLICE:
		mpMod->EpModToggleCommandMode (ep_mode_quickslice);
		break;

	case ID_EP_EDIT_SLICEPLANE:
		mpMod->EpModToggleCommandMode (ep_mode_sliceplane);
		break;

	case ID_EP_EDIT_SLICE:
		if (mpMod->EpInSliceMode()) {
			theHold.Begin ();
			mpMod->EpModCommitAndRepeat (mpMod->EpModGetIP()->GetTime());
			theHold.Accept (IDS_SLICE);
		} else {
			if (mpMod->GetMNSelLevel() == MNM_SL_FACE) mpMod->EpModButtonOp (ep_op_slice_face);
			else mpMod->EpModButtonOp (ep_op_slice);
		}
		break;

	case ID_EP_EDIT_RESETPLANE:
		mpMod->EpModButtonOp (ep_op_reset_plane);
		break;

	case ID_EP_EDIT_MAKEPLANAR:
		mpMod->EpModButtonOp (ep_op_make_planar);
		break;

	case ID_EP_EDIT_ALIGN_VIEW:
		theHold.Begin ();
		mpMod->getParamBlock()->SetValue (epm_align_type, 0, 0);
		mpMod->UpdateAlignParameters (0);
		mpMod->EpModButtonOp (ep_op_align);
		theHold.Accept (GetString (IDS_ALIGN_TO_VIEW));
		break;

	case ID_EP_EDIT_ALIGN_GRID:
		theHold.Begin ();
		mpMod->getParamBlock()->SetValue (epm_align_type, 0, 1);
		mpMod->UpdateAlignParameters (0);
		mpMod->EpModButtonOp (ep_op_align);
		theHold.Accept (GetString (IDS_ALIGN_TO_GRID));
		break;

	case ID_EP_EDIT_MESHSMOOTH:
		mpMod->EpModButtonOp (ep_op_meshsmooth);
		break;

	case ID_EP_EDIT_MESHSMOOTH_SETTINGS:
		mpMod->EpModPopupDialog (ep_op_meshsmooth);
		break;

	case ID_EP_EDIT_TESSELLATE:
		mpMod->EpModButtonOp (ep_op_tessellate);
		break;

	case ID_EP_EDIT_TESSELLATE_SETTINGS:
		mpMod->EpModPopupDialog (ep_op_tessellate);
		break;

	case ID_EP_EDIT_REPEAT:
		mpMod->EpModRepeatLast ();
		break;

	// Subobject-level editing dialog shortcuts:
	case ID_EP_SUB_REMOVE:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_remove_vertex); break;
		case MNM_SL_EDGE: mpMod->EpModButtonOp (ep_op_remove_edge); break;
		}
		break;

	case ID_EP_SUB_BREAK:
		mpMod->EpModButtonOp (ep_op_break);
		break;

	case ID_EP_SUB_SPLIT:
		mpMod->EpModButtonOp (ep_op_split);
		break;

	case ID_EP_SUB_INSERTVERT:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_divide_edge);
			break;
		case MNM_SL_FACE:
			mpMod->EpModToggleCommandMode (ep_mode_divide_face);
			break;
		}
		break;

	case ID_EP_SUB_CAP:
		mpMod->EpModButtonOp (ep_op_cap);
		break;

	case ID_EP_SUB_BRIDGE:
		switch (mpMod->GetEPolySelLevel()) {
			case EPM_SL_BORDER:
				mpMod->EpModToggleCommandMode (ep_mode_bridge_border);
				break;
			case EPM_SL_FACE:
				mpMod->EpModToggleCommandMode (ep_mode_bridge_polygon);
				break;
			case EPM_SL_EDGE:
				mpMod->EpModToggleCommandMode (ep_mode_bridge_edge);
				break;
		}
		break;

	case ID_EP_SUB_BRIDGE_SETTINGS:
		switch (mpMod->GetEPolySelLevel()) {
			case EPM_SL_BORDER:
				mpMod->EpModPopupDialog (ep_op_bridge_border);
				break;
			case EPM_SL_FACE:
				mpMod->EpModPopupDialog (ep_op_bridge_polygon);
				break;
			case EPM_SL_EDGE:
				mpMod->EpModPopupDialog (ep_op_bridge_edge);
				break;
		}
		break;

	case ID_EP_SUB_EXTRUDE:
		switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_VERTEX:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_vertex);
			break;
		case EPM_SL_EDGE:
		case EPM_SL_BORDER:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_edge);
			break;
		case EPM_SL_FACE:
			mpMod->EpModToggleCommandMode (ep_mode_extrude_face);
			break;
		}
		break;

	case ID_EP_SUB_EXTRUDE_SETTINGS:
		switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_VERTEX:
			mpMod->EpModPopupDialog (ep_op_extrude_vertex);
			break;
		case EPM_SL_EDGE:
		case EPM_SL_BORDER:
			mpMod->EpModPopupDialog (ep_op_extrude_edge);
			break;
		case EPM_SL_FACE:
			mpMod->EpModPopupDialog (ep_op_extrude_face);
			break;
		}
		break;

	case ID_EP_SUB_BEVEL:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModToggleCommandMode (ep_mode_bevel);
		break;

	case ID_EP_SUB_BEVEL_SETTINGS:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModPopupDialog (ep_op_bevel);
		break;

	case ID_EP_SUB_OUTLINE:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModToggleCommandMode (ep_mode_outline);
		break;

	case ID_EP_SUB_OUTLINE_SETTINGS:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModPopupDialog (ep_op_outline);
		break;

	case ID_EP_SUB_INSET:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModToggleCommandMode (ep_mode_inset_face);
		break;

	case ID_EP_SUB_INSET_SETTINGS:
		if (mpMod->GetEPolySelLevel() != EPM_SL_FACE) break;
		mpMod->EpModPopupDialog (ep_op_inset);
		break;

	case ID_EP_SUB_CHAMFER:
		switch (mpMod->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpMod->EpModToggleCommandMode (ep_mode_chamfer_vertex);
			break;
		case MNM_SL_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_chamfer_edge);
			break;
		}
		break;

	case ID_EP_SUB_CHAMFER_SETTINGS:
		switch (mpMod->GetMNSelLevel()) {
		case MNM_SL_VERTEX: mpMod->EpModPopupDialog (ep_op_chamfer_vertex); break;
		case MNM_SL_EDGE: mpMod->EpModPopupDialog (ep_op_chamfer_edge); break;
		}
		break;

	case ID_EP_SUB_WELD:
		if (mpMod->GetEPolySelLevel() > EPM_SL_EDGE) break;
		mpMod->EpModToggleCommandMode (ep_mode_weld);
		break;

	case ID_EP_SUB_WELD_SETTINGS:
		switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_VERTEX: mpMod->EpModPopupDialog (ep_op_weld_vertex); break;
		case EPM_SL_EDGE: mpMod->EpModPopupDialog (ep_op_weld_edge); break;
		}
		break;

	case ID_EP_SUB_REMOVE_ISO:
		mpMod->EpModButtonOp (ep_op_remove_iso_verts);
		break;

	case ID_EP_SUB_REMOVE_ISOMAP:
		mpMod->EpModButtonOp (ep_op_remove_iso_map_verts);
		break;

	case ID_EP_SUB_MAKESHAPE:
		if (mpMod->GetMNSelLevel() != MNM_SL_EDGE) break;
		mpMod->EpModPopupDialog (ep_op_create_shape);
		break;

	case ID_EP_SUB_CONNECT:
		switch (mpMod->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpMod->EpModButtonOp (ep_op_connect_vertex);
			break;
		case MNM_SL_EDGE:
			mpMod->EpModButtonOp (ep_op_connect_edge);
			break;
		}
		break;

	case ID_EP_SUB_CONNECT_SETTINGS:
		if (mpMod->GetMNSelLevel() != MNM_SL_EDGE) break;
		mpMod->EpModPopupDialog (ep_op_connect_edge);
		break;

	case ID_EP_SUB_HINGE:
		if (mpMod->GetEPolySelLevel () != EPM_SL_FACE) break;
		mpMod->EpModToggleCommandMode (ep_mode_hinge_from_edge);
		break;

	case ID_EP_SUB_HINGE_SETTINGS:
		if (mpMod->GetEPolySelLevel () != EPM_SL_FACE) break;
		mpMod->EpModPopupDialog (ep_op_hinge_from_edge);
		break;

	case ID_EP_SUB_EXSPLINE:
		if (mpMod->GetEPolySelLevel () != EPM_SL_FACE) break;
		mpMod->EpModEnterPickMode (ep_mode_pick_shape);
		break;

	case ID_EP_SUB_EXSPLINE_SETTINGS:
		if (mpMod->GetEPolySelLevel () != EPM_SL_FACE) break;
		mpMod->EpModPopupDialog (ep_op_extrude_along_spline);
		break;

	case ID_EP_TURN:
		if (mpMod->GetEPolySelLevel() < EPM_SL_EDGE) break;
		mpMod->EpModToggleCommandMode (ep_mode_turn_edge);
		break;

	// Surface dialog control shortcuts:

	case ID_EP_SURF_EDITTRI:
		if (mpMod->GetEPolySelLevel () < EPM_SL_EDGE) break;
		mpMod->EpModToggleCommandMode (ep_mode_edit_tri);
		break;

	case ID_EP_SURF_RETRI:
		mpMod->EpModButtonOp (ep_op_retriangulate);
		break;

	case ID_EP_SURF_FLIPNORM:
		switch (mpMod->GetEPolySelLevel()) {
		case EPM_SL_FACE: mpMod->EpModButtonOp (ep_op_flip_face); break;
		case EPM_SL_ELEMENT: mpMod->EpModButtonOp (ep_op_flip_element); break;
		}
		break;

	case ID_EP_SURF_AUTOSMOOTH:
		mpMod->EpModButtonOp (ep_op_autosmooth);
		break;

	case ID_EP_NONE_CONSTRAINT:
		mpMod->getParamBlock()->GetValue (epm_constrain_type, TimeValue(0), constrainType, FOREVER);
		oldConstrainType = constrainType;	
		if (constrainType == 0)
		{
			if(mpMod->GetLastConstrainType()!=0)
				constrainType = mpMod->GetLastConstrainType();
			else
				return FALSE;
		}
		mpMod->SetLastConstrainType(oldConstrainType);
		theHold.Begin ();
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_EDGE_CONSTRAINT:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_constrain_type, TimeValue(0), constrainType, FOREVER);
		if (constrainType == 1) constrainType = 0;
		else constrainType = 1;
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_FACE_CONSTRAINT:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_constrain_type, TimeValue(0), constrainType, FOREVER);
		if (constrainType == 2) constrainType = 0;
		else constrainType = 2;
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_NORMAL_CONSTRAINT:
		theHold.Begin ();
		mpMod->getParamBlock()->GetValue (epm_constrain_type, TimeValue(0), constrainType, FOREVER);
		if (constrainType == 3) constrainType = 0;
		else constrainType = 3;
		mpMod->getParamBlock()->SetValue (epm_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;



	case ID_EP_SEL_RING_UP:
		{
			RingLoopUpdateID l_change = RING_UP;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_RING_DOWN :
		{
			RingLoopUpdateID l_change = RING_DOWN;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_UP :
		{
			RingLoopUpdateID l_change = LOOP_UP;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_DOWN:
		{
			RingLoopUpdateID l_change = LOOP_DOWN;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_RING_UP_ADD:
		{
			RingLoopUpdateID l_change = RING_UP_ADD;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_RING_DOWN_ADD :
		{
			RingLoopUpdateID l_change = RING_DOWN_ADD;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_UP_ADD :
		{
			RingLoopUpdateID l_change = LOOP_UP_ADD;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_DOWN_ADD:
		{
			RingLoopUpdateID l_change = LOOP_DOWN_ADD;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_RING_UP_SUBTRACT:
		{
			RingLoopUpdateID l_change = RING_UP_SUBTRACT;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_RING_DOWN_SUBTRACT:
		{
			RingLoopUpdateID l_change = RING_DOWN_SUBTRACT;
			mpMod->EpModUpdateRingEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_UP_SUBTRACT :
		{
			RingLoopUpdateID l_change = LOOP_UP_SUBTRACT;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_SEL_LOOP_DOWN_SUBTRACT :
		{
			RingLoopUpdateID l_change = LOOP_DOWN_SUBTRACT;
			mpMod->EpModUpdateLoopEdgeSelection(l_change);
		}
		break;
	case ID_EP_EDIT_SS:
		mpMod->EpModToggleCommandMode (ep_mode_edit_ss);
		break;
	default:
		return false;
	}
	return true;
}

#ifndef RENDER_VER
const int kNumEPolyActions = 91;
#else
const int kNumEPolyActions = 0;
#endif

static ActionDescription epolyActions[] = {
#ifndef RENDER_VER
	ID_EP_SEL_LEV_OBJ,
		IDS_OBJECT_LEVEL,
		IDS_OBJECT_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LEV_VERTEX,
		IDS_VERTEX_LEVEL,
		IDS_VERTEX_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LEV_EDGE,
		IDS_EDGE_LEVEL,
		IDS_EDGE_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LEV_BORDER,
		IDS_BORDER_LEVEL,
		IDS_BORDER_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LEV_FACE,
		IDS_FACE_LEVEL,
		IDS_FACE_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LEV_ELEMENT,
		IDS_ELEMENT_LEVEL,
		IDS_ELEMENT_LEVEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_BYVERT,
		IDS_SEL_BY_VERTEX,
		IDS_SEL_BY_VERTEX,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_IGBACK,
		IDS_IGNORE_BACKFACING,
		IDS_IGNORE_BACKFACING,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_GROW,
		IDS_SELECTION_GROW,
		IDS_SELECTION_GROW,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_SHRINK,				//10
		IDS_SELECTION_SHRINK,
		IDS_SELECTION_SHRINK,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LOOP,
		IDS_SELECT_EDGE_LOOP,
		IDS_SELECT_EDGE_LOOP,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_RING,
		IDS_SELECT_EDGE_RING,
		IDS_SELECT_EDGE_RING,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_HIDE,
		IDS_HIDE,
		IDS_HIDE,
		IDS_EDIT_POLY_MOD,

	ID_EP_UNSEL_HIDE,
		IDS_HIDE_UNSELECTED,
		IDS_HIDE_UNSELECTED,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_UNHIDE,
		IDS_UNHIDE_ALL,
		IDS_UNHIDE_ALL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SS_BACKFACE,
		IDS_AFFECT_BACKFACING,
		IDS_AFFECT_BACKFACING,
		IDS_EDIT_POLY_MOD,

	ID_EP_SS_ON,
		IDS_USE_SOFTSEL,
		IDS_USE_SOFTSEL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SS_SHADEDFACES,
		IDS_SHADED_FACE_TOGGLE,
		IDS_SHADED_FACE_TOGGLE,
		IDS_EDIT_POLY_MOD,
	
	ID_EP_SS_PAINT,
		IDS_EP_SS_PAINT,
		IDS_EP_SS_PAINT,
		IDS_EDIT_POLY_MOD,
	
	ID_EP_SS_LOCK,				//20
		IDS_EP_SS_LOCK,
		IDS_EP_SS_LOCK,
		IDS_EDIT_POLY_MOD,

	ID_EP_SS_PAINTPUSHPULL,
		IDS_EP_SS_PAINTPUSHPULL,
		IDS_EP_SS_PAINTPUSHPULL,
		IDS_EDIT_POLY_MOD,

	ID_EP_SS_PAINTRELAX,
		IDS_EP_SS_PAINTRELAX,
		IDS_EP_SS_PAINTRELAX,
		IDS_EDIT_POLY_MOD,

	ID_EP_TURN,
		IDS_EP_TURN,
		IDS_EP_TURN,
		IDS_EDIT_POLY_MOD,


	ID_EP_EDIT_ALIGN_GRID,
		IDS_ALIGN_TO_GRID,
		IDS_ALIGN_TO_GRID,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_ALIGN_VIEW,
		IDS_ALIGN_TO_VIEW,
		IDS_ALIGN_TO_VIEW,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_ATTACH,
		IDS_ATTACH,
		IDS_ATTACH,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_ATTACHLIST,
		IDS_ATTACH_LIST,
		IDS_ATTACH_LIST,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_COLLAPSE,
		IDS_COLLAPSE,
		IDS_COLLAPSE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_CREATE,
		IDS_CREATE,
		IDS_CREATE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_CUT,			//30
		IDS_CUT,
		IDS_CUT,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_DETACH,
		IDS_DETACH,
		IDS_DETACH,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_MAKEPLANAR,
		IDS_MAKE_PLANAR,
		IDS_MAKE_PLANAR,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_MESHSMOOTH,
		IDS_MESHSMOOTH,
		IDS_MESHSMOOTH,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_MESHSMOOTH_SETTINGS,
		IDS_MESHSMOOTH_SETTINGS,
		IDS_MESHSMOOTH_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_QUICKSLICE,
		IDS_QUICKSLICE_MODE,
		IDS_QUICKSLICE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_REPEAT,
		IDS_REPEAT_LAST_OPERATION,
		IDS_REPEAT_LAST_OPERATION,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_RESETPLANE,
		IDS_RESET_SLICE_PLANE,
		IDS_RESET_SLICE_PLANE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_SLICE,
		IDS_SLICE,
		IDS_SLICE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_SLICEPLANE,
		IDS_SLICE_PLANE_MODE,
		IDS_SLICE_PLANE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_TESSELLATE,		//40
		IDS_TESSELLATE,
		IDS_TESSELLATE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDIT_TESSELLATE_SETTINGS,
		IDS_TESSELLATE_SETTINGS,
		IDS_TESSELLATE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_BEVEL,
		IDS_BEVEL_MODE,
		IDS_BEVEL_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_BEVEL_SETTINGS,
		IDS_BEVEL_SETTINGS,
		IDS_BEVEL_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_BREAK,
		IDS_BREAK,
		IDS_BREAK,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_CAP,
		IDS_CAP,
		IDS_CAP,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_CHAMFER,
		IDS_CHAMFER_MODE,
		IDS_CHAMFER_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_CHAMFER_SETTINGS,
		IDS_CHAMFER_SETTINGS,
		IDS_CHAMFER_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_CONNECT,
		IDS_CONNECT,
		IDS_CONNECT,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_CONNECT_SETTINGS,
		IDS_CONNECT_EDGE_SETTINGS,
		IDS_CONNECT_EDGE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_EXSPLINE,					//50
		IDS_EXTRUDE_ALONG_SPLINE_MODE,
		IDS_EXTRUDE_ALONG_SPLINE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_EXSPLINE_SETTINGS,
		IDS_EXTRUDE_ALONG_SPLINE_SETTINGS,
		IDS_EXTRUDE_ALONG_SPLINE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_EXTRUDE,
		IDS_EXTRUDE_MODE,
		IDS_EXTRUDE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_EXTRUDE_SETTINGS,
		IDS_EXTRUDE_SETTINGS,
		IDS_EXTRUDE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_INSERTVERT,
		IDS_INSERT_VERTEX_MODE,
		IDS_INSERT_VERTEX_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_INSET,
		IDS_INSET_MODE,
		IDS_INSET_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_INSET_SETTINGS,
		IDS_INSET_SETTINGS,
		IDS_INSET_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_HINGE,
		IDS_HINGE_FROM_EDGE_MODE,
		IDS_HINGE_FROM_EDGE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_HINGE_SETTINGS,
		IDS_HINGE_FROM_EDGE_SETTINGS,
		IDS_HINGE_FROM_EDGE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_MAKESHAPE,
		IDS_CREATE_SHAPE_FROM_EDGES,
		IDS_CREATE_SHAPE_FROM_EDGES,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_OUTLINE,			//60
		IDS_OUTLINE_MODE,
		IDS_OUTLINE_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_OUTLINE_SETTINGS,
		IDS_OUTLINE_SETTINGS,
		IDS_OUTLINE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_REMOVE,
		IDS_REMOVE,
		IDS_REMOVE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_REMOVE_ISO,
		IDS_REMOVE_ISOLATED_VERTICES,
		IDS_REMOVE_ISOLATED_VERTICES,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_REMOVE_ISOMAP,
		IDS_REMOVE_UNUSED_MAP_VERTICES,
		IDS_REMOVE_UNUSED_MAP_VERTICES,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_SPLIT,
		IDS_SPLIT_EDGES,
		IDS_SPLIT_EDGES,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_WELD,
		IDS_WELD_MODE,
		IDS_WELD_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_WELD_SETTINGS,
		IDS_WELD_SETTINGS,
		IDS_WELD_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SURF_AUTOSMOOTH,
		IDS_AUTOSMOOTH,
		IDS_AUTOSMOOTH,
		IDS_EDIT_POLY_MOD,

	ID_EP_SURF_EDITTRI,
		IDS_EDIT_TRIANGULATION_MODE,
		IDS_EDIT_TRIANGULATION_MODE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SURF_FLIPNORM,			//70
		IDS_FLIP_NORMALS,
		IDS_FLIP_NORMALS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SURF_RETRI,
		IDS_RETRIANGULATE,
		IDS_RETRIANGULATE,
		IDS_EDIT_POLY_MOD,

	ID_EP_NONE_CONSTRAINT,
		IDS_CONSTRAIN_TO_NONE,
		IDS_CONSTRAIN_TO_NONE,
		IDS_EDIT_POLY_MOD,

	ID_EP_EDGE_CONSTRAINT,
		IDS_CONSTRAIN_TO_EDGES,
		IDS_CONSTRAIN_TO_EDGES,
		IDS_EDIT_POLY_MOD,

	ID_EP_FACE_CONSTRAINT,
		IDS_CONSTRAIN_TO_FACES,
		IDS_CONSTRAIN_TO_FACES,
		IDS_EDIT_POLY_MOD,

	ID_EP_NORMAL_CONSTRAINT,
		IDS_CONSTRAIN_TO_NORMALS,
		IDS_CONSTRAIN_TO_NORMALS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_BRIDGE,
		IDS_BRIDGE,
		IDS_BRIDGE,
		IDS_EDIT_POLY_MOD,

	ID_EP_SUB_BRIDGE_SETTINGS,
		IDS_BRIDGE_SETTINGS,
		IDS_BRIDGE_SETTINGS,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_RING_UP,
		IDS_SELECT_EDGE_RING_UP,
		IDS_SELECT_EDGE_RING_UP,
		IDS_EDIT_POLY_MOD,
            
	ID_EP_SEL_RING_DOWN,
		IDS_SELECT_EDGE_RING_DOWN,
		IDS_SELECT_EDGE_RING_DOWN,
		IDS_EDIT_POLY_MOD,

	ID_EP_SEL_LOOP_UP,				//80
		IDS_SELECT_EDGE_LOOP_UP,
		IDS_SELECT_EDGE_LOOP_UP,
		IDS_EDIT_POLY_MOD,
            
	ID_EP_SEL_LOOP_DOWN,
		IDS_SELECT_EDGE_LOOP_DOWN,
		IDS_SELECT_EDGE_LOOP_DOWN,
		IDS_EDIT_POLY_MOD,
          
	ID_EP_SEL_RING_UP_ADD ,
		IDS_SELECT_EDGE_RING_UP_ADD,
		IDS_SELECT_EDGE_RING_UP_ADD,
		IDS_EDIT_POLY_MOD,
       
	ID_EP_SEL_RING_DOWN_ADD ,
		IDS_SELECT_EDGE_RING_DOWN_ADD,
		IDS_SELECT_EDGE_RING_DOWN_ADD,
		IDS_EDIT_POLY_MOD,
     
	ID_EP_SEL_LOOP_UP_ADD ,
		IDS_SELECT_EDGE_LOOP_UP_ADD,
		IDS_SELECT_EDGE_LOOP_UP_ADD,
		IDS_EDIT_POLY_MOD,
       
	ID_EP_SEL_LOOP_DOWN_ADD,
		IDS_SELECT_EDGE_LOOP_DOWN_ADD,
		IDS_SELECT_EDGE_LOOP_DOWN_ADD,
		IDS_EDIT_POLY_MOD,
      
	ID_EP_SEL_RING_UP_SUBTRACT,
		IDS_SELECT_EDGE_RING_UP_SUBTRACT,
		IDS_SELECT_EDGE_RING_UP_SUBTRACT,
		IDS_EDIT_POLY_MOD,
   
	ID_EP_SEL_RING_DOWN_SUBTRACT,
		IDS_SELECT_EDGE_RING_DOWN_SUBTRACT,
		IDS_SELECT_EDGE_RING_DOWN_SUBTRACT,
		IDS_EDIT_POLY_MOD,
 
	ID_EP_SEL_LOOP_UP_SUBTRACT,
		IDS_SELECT_EDGE_LOOP_UP_SUBTRACT,
		IDS_SELECT_EDGE_LOOP_UP_SUBTRACT,
		IDS_EDIT_POLY_MOD,
   
	ID_EP_SEL_LOOP_DOWN_SUBTRACT,
		IDS_SELECT_EDGE_LOOP_DOWN_SUBTRACT,
		IDS_SELECT_EDGE_LOOP_DOWN_SUBTRACT,
		IDS_EDIT_POLY_MOD,
 
	ID_EP_PREVIEW_SEL,			//90
		IDS_EP_PREVIEW_SEL,
		IDS_EP_PREVIEW_SEL,
		IDS_EDIT_POLY_MOD,
	ID_EP_EDIT_SS,
		IDS_EP_EDIT_SS,
		IDS_EP_EDIT_SS,
		IDS_EDIT_POLY_MOD
#else  // RENDER_VER
	0 // empty initializers are not allowed
#endif // RENDER_VER
};





ActionTable* GetEditPolyActions() {
	TSTR name = GetString (IDS_EDITPOLY_EDIT_POLY);
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_ACCEL_EPOLY));
	ActionTable* pTab = NULL;
	pTab = new ActionTable (kEditPolyActionID, kEditPolyActionID,
		name, hAccel, kNumEPolyActions, epolyActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext
		(kEditPolyActionID, name.data());

	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		//register the action items
		TSTR desc;
		ActionItem *item = pTab->GetAction(ID_EP_SEL_LEV_OBJ);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
	/*  Kelcey said don't do these guys only the object.. leave in case she changes her mind..
		item = pTab->GetAction(ID_EP_SEL_LEV_VERTEX);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		item = pTab->GetAction(ID_EP_SEL_LEV_EDGE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	
		item = pTab->GetAction(ID_EP_SEL_LEV_BORDER);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SEL_LEV_FACE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SEL_LEV_ELEMENT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	
	*/
		//command overrides	
		item = pTab->GetAction(ID_EP_EDIT_CUT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//new edit soft selection mode
		item = pTab->GetAction(ID_EP_EDIT_SS);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		item = pTab->GetAction(ID_EP_EDIT_QUICKSLICE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	
		/*
		item = pTab->GetAction(ID_EP_EDIT_SLICEPLANE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		*/

		//face command mdoes
		item = pTab->GetAction(ID_EP_SUB_BEVEL);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SUB_OUTLINE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SUB_INSET);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SUB_HINGE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//less than edge 
		item = pTab->GetAction(ID_EP_SUB_WELD);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//greater edge
		item = pTab->GetAction(ID_EP_TURN);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		item = pTab->GetAction(ID_EP_SURF_EDITTRI);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//chamfer
		item = pTab->GetAction(ID_EP_SUB_CHAMFER);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//create
		item = pTab->GetAction(ID_EP_EDIT_CREATE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//insert
		item = pTab->GetAction(ID_EP_SUB_INSERTVERT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//bridge
		item = pTab->GetAction(ID_EP_SUB_BRIDGE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	

		//extrude
		item = pTab->GetAction(ID_EP_SUB_EXTRUDE);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}	
		
		//none constrain
		item = pTab->GetAction(ID_EP_NONE_CONSTRAINT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		//normal constrain
		item = pTab->GetAction(ID_EP_NORMAL_CONSTRAINT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		//edge constrain
		item = pTab->GetAction(ID_EP_EDGE_CONSTRAINT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		//face constrain
		item = pTab->GetAction(ID_EP_FACE_CONSTRAINT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
		
		//paint 
		item = pTab->GetAction(ID_EP_SS_PAINT);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}

		//paint push/pull
		item = pTab->GetAction(ID_EP_SS_PAINTPUSHPULL);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}

		//paint relax
		item = pTab->GetAction(ID_EP_SS_PAINTRELAX);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}



	}
	return pTab;
}

ActionTable* EditPolyModClassDesc::GetActionTable(int i) {
	return GetEditPolyActions ();
}

