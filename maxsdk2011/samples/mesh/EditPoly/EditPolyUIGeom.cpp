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
#include "EditPolyUI.h"

static ConstraintUIHandler theConstraintUIHandler;
ConstraintUIHandler *TheConstraintUIHandler () { return &theConstraintUIHandler; }

void ConstraintUIHandler::Initialize (HWND hWnd, EPolyMod *pMod) {
	if (!pMod) return;
	if (!pMod->getParamBlock()) return;
	int constraintType = pMod->getParamBlock()->GetInt (epm_constrain_type);
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


void ConstraintUIHandler::Update (HWND hWnd, EPolyMod *pMod) {
	if (!pMod) return;
	if (!pMod->getParamBlock()) return;
	int constraintType = pMod->getParamBlock()->GetInt (epm_constrain_type);
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

void ConstraintUIHandler::Set (HWND hWnd, EPolyMod *pMod, int type) {
	if (!pMod) return;
	if (!pMod->getParamBlock()) return;

	int constraintType = pMod->getParamBlock()->GetInt (epm_constrain_type);
	if (type == constraintType) return;

	EditPolyMod *epm = static_cast<EditPolyMod *>(pMod);

	epm->SetLastConstrainType(constraintType);

	int cType = pMod->getParamBlock()->GetInt (epm_constrain_type);
	
	switch(type)
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

	theHold.Begin ();
	pMod->getParamBlock()->SetValue (epm_constrain_type, 0, type);
	theHold.Accept (GetString (IDS_PARAMETERS));
}

PreserveMapsUIHandler* PreserveMapsUIHandler::mInstance = NULL;

PreserveMapsUIHandler* PreserveMapsUIHandler::GetInstance()
{
	if( mInstance == NULL ) {
		mInstance = new PreserveMapsUIHandler();
	}
	return mInstance;
}

void PreserveMapsUIHandler::DestroyInstance()
{
	if( mInstance != NULL ) {
		delete mInstance;
		mInstance = NULL;
	}
}

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

	// Find out which map channels are active in any of our meshes:
	mActive = MapBitArray(false);

	IObjParam *ip = mpEPoly->EpModGetIP();
	if (ip != NULL) {
		ModContextList list;
		INodeTab nodes;
		ip->GetModContexts(list, nodes);
		for (int i=0; i<list.Count(); i++) {
			if (list[i]->localData == NULL) continue;
			EditPolyData *pData = (EditPolyData *) list[i]->localData;
			MNMesh *pMesh = pData->GetMesh();
			if (pMesh == NULL) continue;
			for (int mp=-NUM_HIDDENMAPS; mp<pMesh->numm; mp++) {
				if (!pMesh->M(mp)->GetFlag(MN_DEAD)) mActive.Set (mp);
			}
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
	mpEPoly->EpModRefreshScreen ();
}

void PreserveMapsUIHandler::Toggle (int channel) {
	mSettings.Set (channel, !mSettings[channel]);
}

static INT_PTR CALLBACK PreserveMapsSettingsDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id;

	switch (msg) {
	case WM_INITDIALOG:
		ThePopupPosition()->Set (hWnd);
		PreserveMapsUIHandler::GetInstance()->Initialize (hWnd);
		break;

	case WM_COMMAND:
		id = LOWORD(wParam);
		if ((id>=IDC_PRESERVE_MAP_CHAN_MINUS_2) && (id<=IDC_PRESERVE_MAP_CHAN_99)) {
			PreserveMapsUIHandler::GetInstance()->Toggle (id - IDC_PRESERVE_MAP_CHAN_00);
			EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), true);
		} else {
			switch (LOWORD(wParam)) {
			case IDC_PRESERVE_MAPS_RESET:
				PreserveMapsUIHandler::GetInstance()->Reset (hWnd);
				EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), true);
				break;

			case IDC_PRESERVE_MAPS_APPLY:
				PreserveMapsUIHandler::GetInstance()->Commit ();
				EnableWindow (GetDlgItem (hWnd, IDC_PRESERVE_MAPS_APPLY), false);
				break;

			case IDOK:
				ThePopupPosition()->Scan (hWnd);
				PreserveMapsUIHandler::GetInstance()->Commit ();
				EndDialog(hWnd,1);
				break;

			case IDCANCEL:
				ThePopupPosition()->Scan (hWnd);
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
		mpEPoly->EpModGetIP()->GetMAXHWnd(), PreserveMapsSettingsDlgProc, NULL);
}

static EditPolyGeomDlgProc theGeomDlgProc;
EditPolyGeomDlgProc *TheGeomDlgProc () { return &theGeomDlgProc; }

void EditPolyGeomDlgProc::SetEnables (HWND hGeom) {
	int sl = mpMod->GetEPolySelLevel ();
	BOOL elem = (sl == EPM_SL_ELEMENT);
	BOOL obj = (sl == EPM_SL_OBJECT);
	bool edg = (meshSelLevel[sl]==MNM_SL_EDGE) ? true : false;

	bool animateMode = (mpMod->getParamBlock()->GetInt (epm_animation_mode) != 0);
#ifndef EDIT_POLY_DISABLE_IN_ANIMATE
	animateMode = false;
#endif

	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_CREATE));

	if (but) {
		but->Enable (!animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	if (but) {
		but->Enable (!animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH_LIST));
	if (but) {
		but->Enable (!animateMode);
		ReleaseICustButton (but);
	}

	// Detach active in subobj
	but = GetICustButton (GetDlgItem (hGeom, IDC_DETACH));
	if (but) {
		but->Enable (!obj && !animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_SETTINGS_DETACH));
	if (but) {
		but->Enable (!obj && !animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_COLLAPSE));
	if (but) {
		but->Enable (!obj && !elem);
		ReleaseICustButton (but);
	}

	// It would be nice if Slice Plane were always active, but we can't make it available
	// at the object level, since the transforms won't work.
	UpdateSliceUI (hGeom);

	but = GetICustButton (GetDlgItem (hGeom, IDC_CUT));
	if (but) {
		but->Enable (!animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_QUICKSLICE));
	if (but) {
		but->Enable (!animateMode);
		ReleaseICustButton (but);
	}

	// MSmooth, Tessellate always active.
	// Make Planar, Align View, Align Grid always active.

	but = GetICustButton (GetDlgItem (hGeom, IDC_HIDE_SELECTED));
	if (but) {
		but->Enable (!edg && !obj && !animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_HIDE_UNSELECTED));
	if (but) {
		but->Enable (!edg && !obj && !animateMode);
		ReleaseICustButton (but);
	}

	but = GetICustButton (GetDlgItem (hGeom, IDC_UNHIDEALL));
	if (but) {
		but->Enable (!edg && !obj && !animateMode);
		ReleaseICustButton (but);
	}

	EnableWindow (GetDlgItem (hGeom, IDC_EDITPOLY_DELETE_ISO_VERTS), (sl>EPM_SL_VERTEX));

	EnableWindow (GetDlgItem (hGeom, IDC_NAMEDSEL_LABEL), !obj);
	but = GetICustButton (GetDlgItem (hGeom, IDC_COPY_NS));
	if (but != NULL) {
		but->Enable (!obj);
		ReleaseICustButton (but);
	}
	but = GetICustButton (GetDlgItem (hGeom, IDC_PASTE_NS));
	if (but != NULL) {
		but->Enable (!obj && (GetMeshNamedSelClip (namedClipLevel[sl])));
		ReleaseICustButton(but);
	}
}

INT_PTR EditPolyGeomDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		theConstraintUIHandler.Initialize (hWnd, mpMod);

		// Set up the "depressed" color for the command-mode buttons
		but = GetICustButton(GetDlgItem(hWnd,IDC_CREATE));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(but);

		but = GetICustButton(GetDlgItem(hWnd,IDC_ATTACH));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_DETACH));
		if (but) {
			but->Disable ();
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SLICEPLANE));
		if (but) {
			but->SetType (CBT_CHECK);
			but->SetHighlightColor (GREEN_WASH);
			ReleaseICustButton (but);
		}

		but = GetICustButton(GetDlgItem(hWnd,IDC_CUT));
		if (but) {
			but->SetType(CBT_CHECK);
			but->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_QUICKSLICE));
		if (but) {
			but->SetType (CBT_CHECK);
			but->SetHighlightColor (GREEN_WASH);
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_ATTACH_LIST));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_ATTACH_LIST));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_DETACH));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_MESHSMOOTH));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_TESSELLATE));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_PRESERVE_MAPS));
		if (but)
		{
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton (but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_SETTINGS_RELAX));
		if (but) {
			but->SetImage (GetPolySelImageHandler()->LoadPlusMinus(), 4,4,5,5, 12, 12);
			but->SetTooltip (true, GetString (IDS_SETTINGS));
			ReleaseICustButton(but);
		}

		SetEnables(hWnd);
		UpdateSliceUI (hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_REPEAT_LAST:
			mpMod->EpModRepeatLast ();
			break;
		case IDC_CONSTRAINT_NONE:
			theConstraintUIHandler.Set (hWnd, mpMod,0);
			break;
		case IDC_CONSTRAINT_EDGE:
			theConstraintUIHandler.Set (hWnd, mpMod,1);
			break;
		case IDC_CONSTRAINT_FACE:
			theConstraintUIHandler.Set (hWnd, mpMod,2);
			break;
		case IDC_CONSTRAINT_NORMAL:
			theConstraintUIHandler.Set (hWnd, mpMod,3);
			break;
		case IDC_SETTINGS_PRESERVE_MAPS:
			PreserveMapsUIHandler::GetInstance()->SetEPoly (mpMod);
			PreserveMapsUIHandler::GetInstance()->DoDialog ();
			break;

		case IDC_CREATE:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModToggleCommandMode (ep_mode_create_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModToggleCommandMode (ep_mode_create_edge); break;
			default: mpMod->EpModToggleCommandMode (ep_mode_create_face); break;
			}
			break;

		case IDC_ATTACH:
			mpMod->EpModEnterPickMode (ep_mode_attach);
			break;

		case IDC_ATTACH_LIST:
			mpMod->EpModPopupDialog (ep_op_attach);
			break;

		case IDC_DETACH:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpMod->EpModButtonOp (ep_op_detach_vertex);
				break;
			case MNM_SL_EDGE:
			case MNM_SL_FACE:
				mpMod->EpModButtonOp (ep_op_detach_face);
				break;
			}
			break;

		case IDC_SETTINGS_DETACH:
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

		case IDC_COLLAPSE:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_collapse_vertex); break;
			case MNM_SL_EDGE: mpMod->EpModButtonOp (ep_op_collapse_edge); break;
			case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_collapse_face); break;
			}
			break;

		case IDC_CUT:
			mpMod->EpModToggleCommandMode (ep_mode_cut);
			break;

		case IDC_SLICEPLANE:
			mpMod->EpModToggleCommandMode (ep_mode_sliceplane);
			break;

		case IDC_SLICE:
			if (mpMod->EpInSliceMode()) {
				theHold.Begin ();
				mpMod->EpModCommitAndRepeat (mpMod->EpModGetIP()->GetTime());
				theHold.Accept (GetString (IDS_SLICE));
			} else {
				if (mpMod->GetMNSelLevel() == MNM_SL_FACE) mpMod->EpModButtonOp (ep_op_slice_face);
				else mpMod->EpModButtonOp (ep_op_slice);
			}
			break;

		case IDC_RESET_PLANE:
			mpMod->EpModButtonOp (ep_op_reset_plane);
			break;

		case IDC_QUICKSLICE:
			mpMod->EpModToggleCommandMode (ep_mode_quickslice);
			break;

		case IDC_MAKEPLANAR:
			mpMod->EpModButtonOp (ep_op_make_planar);
			break;

		case IDC_MAKE_PLANAR_X:
			mpMod->EpModButtonOp (ep_op_make_planar_x);
			break;

		case IDC_MAKE_PLANAR_Y:
			mpMod->EpModButtonOp (ep_op_make_planar_y);
			break;

		case IDC_MAKE_PLANAR_Z:
			mpMod->EpModButtonOp (ep_op_make_planar_z);
			break;

		case IDC_ALIGN_VIEW:
			theHold.Begin ();
			mpMod->getParamBlock()->SetValue (epm_align_type, t, 0);
			mpMod->UpdateAlignParameters (t);
			mpMod->EpModButtonOp (ep_op_align);
			theHold.Accept (GetString (IDS_EDITPOLY_ALIGN));
			break;

		case IDC_ALIGN_GRID:
			theHold.Begin ();
			mpMod->getParamBlock()->SetValue (epm_align_type, t, 1);
			mpMod->UpdateAlignParameters (t);
			mpMod->EpModButtonOp (ep_op_align);
			theHold.Accept (GetString (IDS_EDITPOLY_ALIGN));
			break;

		case IDC_RELAX:
			mpMod->EpModButtonOp (ep_op_relax);
			break;

		case IDC_SETTINGS_RELAX:
			mpMod->EpModPopupDialog (ep_op_relax);
			break;

		case IDC_MESHSMOOTH:
			mpMod->EpModButtonOp (ep_op_meshsmooth);
			break;

		case IDC_SETTINGS_MESHSMOOTH:
			mpMod->EpModPopupDialog (ep_op_meshsmooth);
			break;

		case IDC_TESSELLATE:
			mpMod->EpModButtonOp (ep_op_tessellate);
			break;

		case IDC_SETTINGS_TESSELLATE:
			mpMod->EpModPopupDialog (ep_op_tessellate);
			break;

		case IDC_HIDE_SELECTED:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_hide_vertex); break;
			case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_hide_face); break;
			}
			break;

		case IDC_UNHIDEALL:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_unhide_vertex); break;
			case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_unhide_face); break;
			}
			break;

		case IDC_HIDE_UNSELECTED:
			switch (mpMod->GetMNSelLevel()) {
			case MNM_SL_VERTEX: mpMod->EpModButtonOp (ep_op_hide_unsel_vertex); break;
			case MNM_SL_FACE: mpMod->EpModButtonOp (ep_op_hide_unsel_face); break;
			}
			break;

		case IDC_COPY_NS:
			mpMod->EpModButtonOp (ep_op_ns_copy);
			break;

		case IDC_PASTE_NS:
			mpMod->EpModButtonOp (ep_op_ns_paste);
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void EditPolyGeomDlgProc::UpdateSliceUI (HWND hGeom) {
	if (!hGeom) return;
	bool sliceMode = mpMod->EpInSliceMode ();
	bool inSlice = mpMod->EpInSlice ();
	bool animate = mpMod->getParamBlock()->GetInt (epm_animation_mode) != 0;
	bool subobj = (mpMod->GetEPolySelLevel()>0);

	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_SLICE));
	if (but != NULL)
	{
		if (animate) but->Enable (subobj);
		else but->Enable (sliceMode);
		ReleaseICustButton (but);
	}
	but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
	if (but != NULL)
	{
		if (animate) but->Enable (subobj);
		else but->Enable (sliceMode);
		ReleaseICustButton (but);
	}
	but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
	if (but != NULL)
	{
		but->SetCheck (sliceMode);
		if (animate) but->Enable (inSlice);
		else but->Enable (subobj);
		ReleaseICustButton (but);
	}
}

void EditPolyMod::UpdateSliceUI () {
	theGeomDlgProc.UpdateSliceUI (GetDlgHandle (ep_geom));
}




