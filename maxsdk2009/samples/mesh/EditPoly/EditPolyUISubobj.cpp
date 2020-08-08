/**********************************************************************
 *<
	FILE: EditPolyUISubobj.cpp

	DESCRIPTION: UI code for Edit Subobject dialogs in Edit Poly Modifier

	CREATED BY: Steve Anderson

	HISTORY: created May 2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "epoly.h"
#include "EditPoly.h"
#include "EditPolyUI.h"

static EditPolySubobjControlDlgProc theSubobjControlDlgProc;
EditPolySubobjControlDlgProc *TheSubobjDlgProc () { return &theSubobjControlDlgProc; }

void EditPolySubobjControlDlgProc::SetEnables (HWND hWnd)
{
	// The only reason we disable things in this dialog is if they're not animatable,
	// and we're in animation mode.
	bool animateMode = mpMod->getParamBlock()->GetInt (epm_animation_mode) != 0;

#ifndef EDIT_POLY_DISABLE_IN_ANIMATE
	animateMode = false;
#endif

	ICustButton* but = NULL;
	but = GetICustButton (GetDlgItem (hWnd, IDC_INSERT_VERTEX));
	if (but) {
		if (animateMode) but->Disable();
		else but->Enable();
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hWnd, IDC_FS_EDIT_TRI));
	if (but) {
		if (animateMode) but->Disable();
		else but->Enable();
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hWnd, IDC_TURN_EDGE));
	if (but) {
		if (animateMode) but->Disable();
		else but->Enable();
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hWnd, IDC_CREATE_SHAPE));
	if (but) {
		if (animateMode) but->Disable();
		else but->Enable();
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CREATE_SHAPE));
	if (but) {
		if (animateMode) but->Disable();
		else but->Enable();
		ReleaseICustButton (but);
	}
}

INT_PTR EditPolySubobjControlDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		but = GetICustButton(GetDlgItem(hWnd,IDC_INSERT_VERTEX));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_BRIDGE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BRIDGE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_EXTRUDE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_EXTRUDE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_BEVEL));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BEVEL));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_OUTLINE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_OUTLINE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_INSET));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_INSET));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_CHAMFER));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CHAMFER));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BREAK));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_WELD));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_TARGET_WELD));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_HINGE_FROM_EDGE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_HINGE_FROM_EDGE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_ALONG_SPLINE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_EXTRUDE_ALONG_SPLINE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CONNECT_EDGES));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_CREATE_SHAPE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_FS_EDIT_TRI));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			if (mpMod->GetMNSelLevel() == MNM_SL_EDGE)
				but->SetTooltip (true, GetString (IDS_EDIT_TRIANGULATION));
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_TURN_EDGE));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_BRIDGE_EDGE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		SetEnables (hWnd);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_REMOVE:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_remove_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModButtonOp (ep_op_remove_edge); break;
			}
			break;

		case IDC_BREAK:
			mpMod->EpModButtonOp (ep_op_break);
			break;

		case IDC_SETTINGS_BREAK:
			mpMod->EpModPopupDialog (ep_op_break); 
			break;

		case IDC_SPLIT:
			mpMod->EpModButtonOp (ep_op_split);
			break;

		case IDC_INSERT_VERTEX:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_EDGE: mpMod->EpModToggleCommandMode (ep_mode_divide_edge); break;
			case MNM_SL_FACE: mpMod->EpModToggleCommandMode (ep_mode_divide_face); break;
			}
			break;

		case IDC_CAP:
			mpMod->EpModButtonOp (ep_op_cap);
			break;

		case IDC_EXTRUDE:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModToggleCommandMode (ep_mode_extrude_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModToggleCommandMode (ep_mode_extrude_edge); break;
			case MNM_SL_FACE: mpMod->EpModToggleCommandMode (ep_mode_extrude_face); break;
			}
			break;

		case IDC_SETTINGS_EXTRUDE:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModPopupDialog (ep_op_extrude_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModPopupDialog (ep_op_extrude_edge); break;
			case MNM_SL_FACE: mpMod->EpModPopupDialog (ep_op_extrude_face); break;
			}
			break;

		case IDC_BEVEL:
			mpMod->EpModToggleCommandMode (ep_mode_bevel);
			break;

		case IDC_SETTINGS_BEVEL:
			mpMod->EpModPopupDialog (ep_op_bevel);
			break;

		case IDC_OUTLINE:
			mpMod->EpModToggleCommandMode (ep_mode_outline);
			break;

		case IDC_SETTINGS_OUTLINE:
			mpMod->EpModPopupDialog (ep_op_outline);
			break;

		case IDC_INSET:
			mpMod->EpModToggleCommandMode (ep_mode_inset_face);
			break;

		case IDC_SETTINGS_INSET:
			mpMod->EpModPopupDialog (ep_op_inset);
			break;

		case IDC_CHAMFER:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpMod->EpModToggleCommandMode (ep_mode_chamfer_vertex);
				break;
			case MNM_SL_EDGE:
				mpMod->EpModToggleCommandMode (ep_mode_chamfer_edge);
				break;
			}
			break;

		case IDC_SETTINGS_CHAMFER:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModPopupDialog (ep_op_chamfer_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModPopupDialog (ep_op_chamfer_edge); break;
			}
			break;

		case IDC_WELD:
			switch (mpMod->GetEPolySelLevel()) {
			case EPM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_weld_vertex); break;
			case EPM_SL_EDGE: mpMod->EpModButtonOp (ep_op_weld_edge); break;
			}
			break;

		case IDC_SETTINGS_WELD:
			switch (mpMod->GetEPolySelLevel()) {
			case EPM_SL_VERTEX: mpMod->EpModPopupDialog (ep_op_weld_vertex); break;
			case EPM_SL_EDGE: mpMod->EpModPopupDialog (ep_op_weld_edge); break;
			}
			break;

		case IDC_TARGET_WELD:
			mpMod->EpModToggleCommandMode (ep_mode_weld);
			break;

		case IDC_REMOVE_ISO_VERTS:
			mpMod->EpModButtonOp (ep_op_remove_iso_verts);
			break;

		case IDC_REMOVE_ISO_MAP_VERTS:
			mpMod->EpModButtonOp (ep_op_remove_iso_map_verts);
			break;

		case IDC_CREATE_SHAPE:
			mpMod->EpModButtonOp (ep_op_create_shape);
			break;

		case IDC_SETTINGS_CREATE_SHAPE:
			mpMod->EpModPopupDialog (ep_op_create_shape);
			break;

		case IDC_CONNECT_EDGES:
			mpMod->EpModButtonOp (ep_op_connect_edge);
			break;

		case IDC_SETTINGS_CONNECT_EDGES:
			mpMod->EpModPopupDialog (ep_op_connect_edge);
			break;

		case IDC_CONNECT_VERTICES:
			mpMod->EpModButtonOp (ep_op_connect_vertex);
			break;

		case IDC_HINGE_FROM_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_hinge_from_edge);
			break;

		case IDC_SETTINGS_HINGE_FROM_EDGE:
			mpMod->EpModPopupDialog (ep_op_hinge_from_edge);
			break;

		case IDC_EXTRUDE_ALONG_SPLINE:
			mpMod->EpModEnterPickMode (ep_mode_pick_shape);
			break;

		case IDC_SETTINGS_EXTRUDE_ALONG_SPLINE:
			mpMod->EpModPopupDialog (ep_op_extrude_along_spline);
			break;

		case IDC_BRIDGE:
			int currentMode;
			currentMode = mpMod->EpModGetCommandMode ();
			if ((currentMode != ep_op_bridge_border) && 
				(currentMode != ep_op_bridge_polygon) &&
				(currentMode != ep_op_bridge_edge) &&
				mpMod->EpModReadyToBridgeSelected()) {

				if (mpMod->GetEPolySelLevel() == EPM_SL_BORDER) {
					mpMod->EpModButtonOp (ep_op_bridge_border);
				} else if (mpMod->GetEPolySelLevel() == EPM_SL_FACE) {
					mpMod->EpModButtonOp (ep_op_bridge_polygon);
				} else if (mpMod->GetEPolySelLevel() == EPM_SL_EDGE) {
					mpMod->EpModButtonOp (ep_op_bridge_edge);
				}
				// Don't leave the button pressed-in.
				but = GetICustButton(GetDlgItem(hWnd,IDC_BRIDGE));
				if (but) {
					but->SetCheck (false);
					ReleaseICustButton (but);
				}
			} else {
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
			}
			break;

		case IDC_SETTINGS_BRIDGE:
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

		case IDC_FS_EDIT_TRI:
			mpMod->EpModToggleCommandMode (ep_mode_edit_tri);
			break;

		case IDC_TURN_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_turn_edge);
			break;

		case IDC_FS_RETRIANGULATE:
			mpMod->EpModButtonOp (ep_op_retriangulate);
			break;

		case IDC_FS_FLIP_NORMALS:
			switch (mpMod->GetEPolySelLevel()) {
			case EPM_SL_FACE: mpMod->EpModButtonOp (ep_op_flip_face); break;
			case EPM_SL_ELEMENT: mpMod->EpModButtonOp (ep_op_flip_element); break;
			}
			break;

		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

